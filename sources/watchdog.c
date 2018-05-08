/*
 * Author: daddinuz
 * email:  daddinuz@gmail.com
 *
 * Copyright (c) 2018 Davide Di Carlo
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "watchdog.h"

/*
 * Un-define overrides over stdlib.h
 */
#undef malloc
#undef calloc
#undef realloc
#undef free
#undef exit
#undef abort

#include <stdio.h>
#include <assert.h>
#include <memory.h>
#include <stdbool.h>
#include <panic/panic.h>

#define COLLECT_THRESHOLD   16
#define MAGIC_NUMBER        ((void *) 0xdeadbeaf)

#define _STR(x)             #x
#define STR(x)              _STR(x)

/*
 * Watchdog_Trace
 */
enum Watchdog_Trace_Call {
    WATCHDOG_TRACE_CALL_MALLOC,
    WATCHDOG_TRACE_CALL_CALLOC,
    WATCHDOG_TRACE_CALL_REALLOC,
    WATCHDOG_TRACE_CALL_FREE
};

static const char *Watchdog_Trace_Call_toString(enum Watchdog_Trace_Call call);

struct Watchdog_Trace {
    struct Watchdog_Trace *prev;
    const char *const file;
    const size_t size;
    const int line;
    const enum Watchdog_Trace_Call call;
};

static struct Watchdog_Trace *Watchdog_Trace_new(const char *file, int line, enum Watchdog_Trace_Call call, size_t size)
__attribute__((__warn_unused_result__, __nonnull__));

static struct Watchdog_Trace *Watchdog_Trace_add(struct Watchdog_Trace *self, struct Watchdog_Trace *trace)
__attribute__((__nonnull__(2)));

static void Watchdog_Trace_deleteAll(struct Watchdog_Trace *self);

/*
 * Watchdog_Chunk
 */
struct Watchdog_Chunk {
    struct Watchdog_Chunk *prev;
    struct Watchdog_Trace *trace;
    void *magicNumber;
    void *memory;
};

static struct Watchdog_Chunk *Watchdog_Chunk_fromMalloc(const char *file, int line, size_t size)
__attribute__((__warn_unused_result__, __nonnull__));

static struct Watchdog_Chunk *
Watchdog_Chunk_fromCalloc(const char *file, int line, size_t numberOfMembers, size_t memberSize)
__attribute__((__warn_unused_result__, __nonnull__));

static struct Watchdog_Chunk *Watchdog_Chunk_fromRealloc(const char *file, int line, void *ptr, size_t newSize)
__attribute__((__warn_unused_result__, __nonnull__(1)));

static void Watchdog_Chunk_markAsFreed(const char *file, int line, void *ptr)
__attribute__((__nonnull__(1)));

static struct Watchdog_Chunk *Watchdog_Chunk_add(struct Watchdog_Chunk *self, struct Watchdog_Chunk *chunk)
__attribute__((__warn_unused_result__, __nonnull__(2)));

static struct Watchdog_Chunk *Watchdog_Chunk_getFromMemory(const char *file, int line, const void *memory)
__attribute__((__warn_unused_result__, __nonnull__));

static void Watchdog_Chunk_report(const struct Watchdog_Chunk *self)
__attribute__((__nonnull__));

static void Watchdog_Chunk_delete(struct Watchdog_Chunk *self);

/*
 * Watchdog_Visit
 */
enum Watchdog_Visit {
    WATCHDOG_VISIT_NO_OP,
    WATCHDOG_VISIT_REPORT,
    WATCHDOG_VISIT_COLLECT,
    WATCHDOG_VISIT_REPORT_AND_COLLECT,
};

typedef enum Watchdog_Visit (*const Watchdog_Chunk_VisitFn)(const struct Watchdog_Chunk *const);

static struct Watchdog_Chunk *Watchdog_Visit_traverse(struct Watchdog_Chunk *start, Watchdog_Chunk_VisitFn visit)
__attribute__((__warn_unused_result__, __nonnull__));

static enum Watchdog_Visit Watchdog_Visit_reportAllAndCollectFreedChunks(const struct Watchdog_Chunk *chunk)
__attribute__((__warn_unused_result__, __nonnull__));

static enum Watchdog_Visit Watchdog_Visit_reportAndCollectFreedChunks(const struct Watchdog_Chunk *chunk)
__attribute__((__warn_unused_result__, __nonnull__));

static enum Watchdog_Visit Watchdog_Visit_collectAllChunks(const struct Watchdog_Chunk *chunk)
__attribute__((__warn_unused_result__, __nonnull__));

/*
 * Global variables
 */
static struct Watchdog_Chunk *gChunksHead = NULL;
static FILE *gStream = NULL;
static int gFrees = 0;
static bool gIsInitialized = false;

/*
 * Watchdog
 */
const char *Watchdog_version(void) {
    return STR(WATCHDOG_VERSION_MAJOR) "."
           STR(WATCHDOG_VERSION_MINOR) "."
           STR(WATCHDOG_VERSION_PATCH)
           WATCHDOG_VERSION_SUFFIX;
}

static void Watchdog_onExit(void) {
    gChunksHead = Watchdog_Visit_traverse(gChunksHead, Watchdog_Visit_reportAllAndCollectFreedChunks);
    gChunksHead = Watchdog_Visit_traverse(gChunksHead, Watchdog_Visit_collectAllChunks);
    assert(!gChunksHead);
    (void) gChunksHead;
    if (gStream && gStream != stdout && gStream != stderr) {
        fclose(gStream);
    }
}

static void Watchdog_initialize(void) {
    if (!gIsInitialized) {
        if (strcmp("<stdout>", WATCHDOG_OUTPUT) == 0) {
            gStream = stdout;
        } else if (strcmp("<stderr>", WATCHDOG_OUTPUT) == 0) {
            gStream = stderr;
        } else if (strcmp("<tempfile>", WATCHDOG_OUTPUT) == 0) {
            gStream = fopen(tmpnam(NULL), "w");
        } else {
            char command[1024] = "";
            const size_t commandSize = sizeof(command) - 1;
            const char *path = WATCHDOG_OUTPUT;
#if defined(_WIN32) || defined(_WIN64)
            snprintf(command, commandSize, "md %.*s", (int) (strrchr(path, '\\') - path), path);
#else
            snprintf(command, commandSize, "mkdir -p %.*s", (int) (strrchr(path, '/') - path), path);
#endif
            system(command);
            gStream = fopen(WATCHDOG_OUTPUT, "w");
        }
        if (!gStream) {
            Panic_terminate("Unable to open file: %s", WATCHDOG_OUTPUT);
        }
        atexit(Watchdog_onExit);
        gIsInitialized = true;
    }
}

static void Watchdog_collect(void) {
    if (gFrees >= COLLECT_THRESHOLD) {
        gChunksHead = Watchdog_Visit_traverse(gChunksHead, Watchdog_Visit_reportAndCollectFreedChunks);
        gFrees = 0;
    }
}

void *__Watchdog_malloc(const char *file, int line, size_t size) {
    assert(file);
    assert(line > 0);
    Watchdog_initialize();
    Watchdog_collect();
    gChunksHead = Watchdog_Chunk_add(gChunksHead, Watchdog_Chunk_fromMalloc(file, line, size));
    return gChunksHead->memory;
}

void *__Watchdog_calloc(const char *file, int line, size_t numberOfMembers, size_t memberSize) {
    assert(file);
    assert(line > 0);
    Watchdog_initialize();
    Watchdog_collect();
    gChunksHead = Watchdog_Chunk_add(gChunksHead, Watchdog_Chunk_fromCalloc(file, line, numberOfMembers, memberSize));
    return gChunksHead->memory;
}

void *__Watchdog_realloc(const char *file, int line, void *ptr, size_t newSize) {
    assert(file);
    assert(line > 0);
    Watchdog_initialize();
    Watchdog_collect();
    gFrees++;
    gChunksHead = Watchdog_Chunk_add(gChunksHead, Watchdog_Chunk_fromRealloc(file, line, ptr, newSize));
    return gChunksHead->memory;
}

void __Watchdog_free(const char *file, int line, void *ptr) {
    assert(file);
    assert(line > 0);
    Watchdog_initialize();
    Watchdog_collect();
    Watchdog_Chunk_markAsFreed(file, line, ptr);
    gFrees++;
}

void __Watchdog_exit(const char *file, int line, int status) {
    assert(file);
    assert(line > 0);
    (void) file;
    (void) line;
    exit(status);
}

void __Watchdog_abort(const char *file, int line) {
    assert(file);
    assert(line > 0);
    (void) file;
    (void) line;
    Watchdog_onExit();
    abort();
}

/*
 * Watchdog_Trace
 */
const char *Watchdog_Trace_Call_toString(const enum Watchdog_Trace_Call call) {
    switch (call) {
        case WATCHDOG_TRACE_CALL_MALLOC:
            return "malloc";
        case WATCHDOG_TRACE_CALL_CALLOC:
            return "calloc";
        case WATCHDOG_TRACE_CALL_REALLOC:
            return "realloc";
        case WATCHDOG_TRACE_CALL_FREE:
            return "free";
        default:
            Panic_terminate("Unknown value");
    }
}

struct Watchdog_Trace *
Watchdog_Trace_new(const char *const file, const int line, const enum Watchdog_Trace_Call call, const size_t size) {
    assert(file);
    assert(line > 0);
    assert((WATCHDOG_TRACE_CALL_FREE != call) || (0 == size));
    struct Watchdog_Trace *self = malloc(sizeof(*self));
    if (self) {
        struct Watchdog_Trace init = {.prev=NULL, .file=file, .size=size, .line=line, .call=call};
        memcpy(self, &init, sizeof(*self));
        return self;
    }
    Panic_terminate("Out of memory");
}

struct Watchdog_Trace *Watchdog_Trace_add(struct Watchdog_Trace *self, struct Watchdog_Trace *const trace) {
    assert(trace);
    trace->prev = self;
    self = trace;
    return self;
}

void Watchdog_Trace_deleteAll(struct Watchdog_Trace *self) {
    if (self) {
        for (struct Watchdog_Trace *prev = self->prev; prev; prev = self->prev) {
            self->prev = prev->prev;
            free(prev);
        }
        free(self);
    }
}

/*
 * Watchdog_Chunk
 */
struct Watchdog_Chunk *Watchdog_Chunk_fromMalloc(const char *const file, const int line, const size_t size) {
    assert(file);
    assert(line > 0);
    struct Watchdog_Chunk *self = malloc(sizeof(*self) + size);
    if (self) {
        self->prev = NULL;
        self->magicNumber = MAGIC_NUMBER;
        self->memory = self + 1;
        self->trace = Watchdog_Trace_new(file, line, WATCHDOG_TRACE_CALL_MALLOC, size);
        return self;
    }
    Panic_terminate("Out of memory");
}

struct Watchdog_Chunk *
Watchdog_Chunk_fromCalloc(const char *const file, const int line, const size_t numberOfMembers,
                          const size_t memberSize) {
    assert(file);
    assert(line > 0);
    const size_t actualSize = numberOfMembers * memberSize;
    struct Watchdog_Chunk *self = malloc(sizeof(*self) + actualSize);
    if (self) {
        self->prev = NULL;
        self->magicNumber = MAGIC_NUMBER;
        self->memory = self + 1;
        self->trace = Watchdog_Trace_new(file, line, WATCHDOG_TRACE_CALL_CALLOC, actualSize);
        memset(self->memory, 0, actualSize);
        return self;
    }
    Panic_terminate("Out of memory");
}

struct Watchdog_Chunk *
Watchdog_Chunk_fromRealloc(const char *const file, const int line, void *const ptr, const size_t newSize) {
    assert(file);
    assert(line > 0);
    struct Watchdog_Chunk *self;
    struct Watchdog_Trace *trace = Watchdog_Trace_new(file, line, WATCHDOG_TRACE_CALL_REALLOC, newSize);

    if (ptr) {
        self = Watchdog_Chunk_getFromMemory(file, line, ptr);
        trace = Watchdog_Trace_add(self->trace, trace);
        self->trace = NULL;
    }

    self = malloc(sizeof(*self) + newSize);
    if (self) {
        self->prev = NULL;
        self->magicNumber = MAGIC_NUMBER;
        self->memory = self + 1;
        self->trace = trace;
        return self;
    }

    Panic_terminate("Out of memory");
}

void Watchdog_Chunk_markAsFreed(const char *const file, const int line, void *const ptr) {
    assert(file);
    assert(line > 0);
    if (ptr) {
        struct Watchdog_Chunk *self = Watchdog_Chunk_getFromMemory(file, line, ptr);
        self->trace = Watchdog_Trace_add(self->trace, Watchdog_Trace_new(file, line, WATCHDOG_TRACE_CALL_FREE, 0));
    }
}

struct Watchdog_Chunk *Watchdog_Chunk_add(struct Watchdog_Chunk *self, struct Watchdog_Chunk *const chunk) {
    assert(chunk);
    chunk->prev = self;
    self = chunk;
    return self;
}

struct Watchdog_Chunk *Watchdog_Chunk_getFromMemory(const char *const file, const int line, const void *const memory) {
    assert(memory);
    struct Watchdog_Chunk *self = (struct Watchdog_Chunk *) memory - 1;
    if (MAGIC_NUMBER == self->magicNumber) {
        return self;
    }
    Panic_terminate("From %s:%d\nInvalid memory address: %p", file, line, memory);
}

void Watchdog_Chunk_report(const struct Watchdog_Chunk *const self) {
    assert(self);
    assert(self->trace);
    fprintf(gStream, "\"%p\":\n", self->memory);
    fprintf(gStream, "  status: %s\n", (WATCHDOG_TRACE_CALL_FREE == self->trace->call) ? "freed" : "leaked");
    fprintf(gStream, "  chunks:\n");
    for (struct Watchdog_Trace *trace = self->trace; trace; trace = trace->prev) {
        fprintf(gStream, "    - href: %s:%d\n", trace->file, trace->line);
        fprintf(gStream, "      file: %s\n", trace->file);
        fprintf(gStream, "      line: %d\n", trace->line);
        fprintf(gStream, "      size: %zu\n", trace->size);
        fprintf(gStream, "      call: %s\n", Watchdog_Trace_Call_toString(trace->call));
    }
    fprintf(gStream, "\n");
}

void Watchdog_Chunk_delete(struct Watchdog_Chunk *self) {
    if (self) {
        Watchdog_Trace_deleteAll(self->trace);
        free(self);
    }
}

/*
 * Watchdog_Visit
 */
struct Watchdog_Chunk *Watchdog_Visit_traverse(struct Watchdog_Chunk *start, Watchdog_Chunk_VisitFn visit) {
    for (struct Watchdog_Chunk *beforeMe = NULL, *current = start; current;) {
        switch (visit(current)) {
            case WATCHDOG_VISIT_NO_OP:
                beforeMe = current;
                current = current->prev;
                continue;
            case WATCHDOG_VISIT_REPORT:
                Watchdog_Chunk_report(current);
                beforeMe = current;
                current = current->prev;
                continue;
            case WATCHDOG_VISIT_COLLECT:
                if (current == start) {
                    start = current->prev;
                    Watchdog_Chunk_delete(current);
                    beforeMe = NULL;
                    current = start;
                } else {
                    beforeMe->prev = current->prev;
                    Watchdog_Chunk_delete(current);
                    current = beforeMe->prev;
                }
                continue;
            case WATCHDOG_VISIT_REPORT_AND_COLLECT:
                Watchdog_Chunk_report(current);
                if (current == start) {
                    start = current->prev;
                    Watchdog_Chunk_delete(current);
                    beforeMe = NULL;
                    current = start;
                } else {
                    beforeMe->prev = current->prev;
                    Watchdog_Chunk_delete(current);
                    current = beforeMe->prev;
                }
                continue;
            default:
                Panic_terminate("Unknown value");
        }
    }
    return start;
}

enum Watchdog_Visit Watchdog_Visit_reportAllAndCollectFreedChunks(const struct Watchdog_Chunk *const chunk) {
    assert(chunk);
    if (chunk->trace) {
        return WATCHDOG_TRACE_CALL_FREE == chunk->trace->call ? WATCHDOG_VISIT_REPORT_AND_COLLECT
                                                              : WATCHDOG_VISIT_REPORT;
    }
    return WATCHDOG_VISIT_COLLECT;
}

enum Watchdog_Visit Watchdog_Visit_reportAndCollectFreedChunks(const struct Watchdog_Chunk *const chunk) {
    assert(chunk);
    return (!chunk->trace || WATCHDOG_TRACE_CALL_FREE == chunk->trace->call) ? WATCHDOG_VISIT_REPORT_AND_COLLECT
                                                                             : WATCHDOG_VISIT_NO_OP;
}

enum Watchdog_Visit Watchdog_Visit_collectAllChunks(const struct Watchdog_Chunk *const chunk) {
    (void) chunk;
    assert(chunk);
    return WATCHDOG_VISIT_COLLECT;
}
