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

#include <time.h>
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

static const char *Watchdog_Chunk_explainStatus(const struct Watchdog_Chunk *self)
__attribute__((__warn_unused_result__, __nonnull__));

static void Watchdog_Chunk_delete(struct Watchdog_Chunk *self);

/*
 * Watchdog_Reporter
 */
struct Watchdog_Reporter;

static void Watchdog_Reporter_onEnter(struct Watchdog_Reporter *self)
__attribute__((__nonnull__));

static void Watchdog_Reporter_report(struct Watchdog_Reporter *self, const struct Watchdog_Chunk *chunk)
__attribute__((__nonnull__));

static void Watchdog_Reporter_onExit(struct Watchdog_Reporter *self)
__attribute__((__nonnull__));

static struct Watchdog_Reporter *Watchdog_Reporter_newJsonReporter(FILE *stream);

static struct Watchdog_Reporter *Watchdog_Reporter_newYamlReporter(FILE *stream);

static void Watchdog_Reporter_delete(struct Watchdog_Reporter *self);

/*
 * Watchdog_Dumper
 */
struct Watchdog_Dumper;

static void Watchdog_Dumper_onEnter(struct Watchdog_Dumper *self)
__attribute__((__nonnull__));

static void Watchdog_Dumper_dump(struct Watchdog_Dumper *self, const struct Watchdog_Chunk *chunk)
__attribute__((__nonnull__));

static void Watchdog_Dumper_onExit(struct Watchdog_Dumper *self)
__attribute__((__nonnull__));

static struct Watchdog_Dumper *Watchdog_Dumper_new(struct Watchdog_Reporter *reporter)
__attribute__((__nonnull__));

static void Watchdog_Dumper_delete(struct Watchdog_Dumper *self);

/*
 * Watchdog_Visit
 */
enum Watchdog_Visit {
    WATCHDOG_VISIT_NO_OP,
    WATCHDOG_VISIT_DUMP,
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

static enum Watchdog_Visit Watchdog_Visit_dumpAliveChunks(const struct Watchdog_Chunk *chunk)
__attribute__((__warn_unused_result__, __nonnull__));

static enum Watchdog_Visit Watchdog_Visit_collectAllChunks(const struct Watchdog_Chunk *chunk)
__attribute__((__warn_unused_result__, __nonnull__));

/*
 * Signal Handler
 */
static void Watchdog_signalHandler(int signo);

/*
 * Global variables
 */
static int gFrees = 0;
static bool gIsInitialized = false;
static struct Watchdog_Chunk *gChunksHead = NULL;

static struct Watchdog_Dumper *gDumper = NULL;
static struct Watchdog_Reporter *gReporter = NULL;

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
    Watchdog_Dumper_delete(gDumper);
    Watchdog_Reporter_delete(gReporter);
}

static void Watchdog_onEnter(void) {
    if (!gIsInitialized) {
        // Global reporter initialization
        FILE *stream = NULL;

        if (strcmp("<stdout>", WATCHDOG_REPORT_STREAM) == 0) {
            stream = stdout;
        } else if (strcmp("<stderr>", WATCHDOG_REPORT_STREAM) == 0) {
            stream = stderr;
        } else if (strcmp("<tempfile>", WATCHDOG_REPORT_STREAM) == 0) {
            stream = fopen(tmpnam(NULL), "w");
        } else {
            char command[1024] = "";
            const size_t commandSize = sizeof(command) - 1;
            const char *directory = WATCHDOG_REPORT_STREAM;
#if defined(_WIN32) || defined(_WIN64)
            snprintf(command, commandSize, "md %.*s", (int) (strrchr(directory, '\\') - directory), directory);
#else
            snprintf(command, commandSize, "mkdir -p %.*s", (int) (strrchr(directory, '/') - directory), directory);
#endif
            if (system(command) != 0) {
                Panic_terminate("Unable to create directory: %s", directory);
            }
            stream = fopen(WATCHDOG_REPORT_STREAM, "w");
        }
        if (!stream) {
            Panic_terminate("Unable to open file: %s", WATCHDOG_REPORT_STREAM);
        }

        if (strcmp("<json>", WATCHDOG_REPORT_FORMAT) == 0) {
            gReporter = Watchdog_Reporter_newJsonReporter(stream);
            gDumper = Watchdog_Dumper_new(Watchdog_Reporter_newJsonReporter(NULL));
        } else if (strcmp("<yaml>", WATCHDOG_REPORT_FORMAT) == 0) {
            gReporter = Watchdog_Reporter_newYamlReporter(stream);
            gDumper = Watchdog_Dumper_new(Watchdog_Reporter_newYamlReporter(NULL));
        } else {
            Panic_terminate("Unknown output format: %s", WATCHDOG_REPORT_FORMAT);
        }

        signal(WATCHDOG_SIGNAL, Watchdog_signalHandler);
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
    Watchdog_onEnter();
    Watchdog_collect();
    gChunksHead = Watchdog_Chunk_add(gChunksHead, Watchdog_Chunk_fromMalloc(file, line, size));
    return gChunksHead->memory;
}

void *__Watchdog_calloc(const char *file, int line, size_t numberOfMembers, size_t memberSize) {
    assert(file);
    assert(line > 0);
    Watchdog_onEnter();
    Watchdog_collect();
    gChunksHead = Watchdog_Chunk_add(gChunksHead, Watchdog_Chunk_fromCalloc(file, line, numberOfMembers, memberSize));
    return gChunksHead->memory;
}

void *__Watchdog_realloc(const char *file, int line, void *ptr, size_t newSize) {
    assert(file);
    assert(line > 0);
    Watchdog_onEnter();
    Watchdog_collect();
    gFrees++;
    gChunksHead = Watchdog_Chunk_add(gChunksHead, Watchdog_Chunk_fromRealloc(file, line, ptr, newSize));
    return gChunksHead->memory;
}

void __Watchdog_free(const char *file, int line, void *ptr) {
    assert(file);
    assert(line > 0);
    Watchdog_onEnter();
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

const char *Watchdog_Chunk_explainStatus(const struct Watchdog_Chunk *const self) {
    assert(self);
    return (self->trace && WATCHDOG_TRACE_CALL_FREE != self->trace->call) ? "in use" : "released";
}

void Watchdog_Chunk_delete(struct Watchdog_Chunk *self) {
    if (self) {
        Watchdog_Trace_deleteAll(self->trace);
        free(self);
    }
}

/*
 * Watchdog_Reporter
 */
struct Watchdog_Reporter {
    void (*onEnter)(struct Watchdog_Reporter *const);
    void (*report)(struct Watchdog_Reporter *const, const struct Watchdog_Chunk *const);
    void (*onExit)(struct Watchdog_Reporter *const);
    FILE *stream;
    void *context;
};

struct Watchdog_JsonReporter_Context {
    unsigned char needsHeader : 1;
    unsigned char needsFooter : 1;
    unsigned char hasReportedSomething : 1;
};

static void Watchdog_JsonReporter_onEnter(struct Watchdog_Reporter *self)
__attribute__((__nonnull__));

static void Watchdog_JsonReporter_report(struct Watchdog_Reporter *self, const struct Watchdog_Chunk *chunk)
__attribute__((__nonnull__));

static void Watchdog_JsonReporter_onExit(struct Watchdog_Reporter *)
__attribute__((__nonnull__));

static void Watchdog_YamlReporter_report(struct Watchdog_Reporter *, const struct Watchdog_Chunk *chunk)
__attribute__((__nonnull__));

void Watchdog_Reporter_onEnter(struct Watchdog_Reporter *const self) {
    assert(self);
    if (self->onEnter) {
        self->onEnter(self);
    }
}

void Watchdog_Reporter_report(struct Watchdog_Reporter *const self, const struct Watchdog_Chunk *const chunk) {
    assert(self);
    assert(self->stream);
    assert(self->report);
    self->report(self, chunk);
}

void Watchdog_Reporter_onExit(struct Watchdog_Reporter *const self) {
    assert(self);
    if (self->onExit) {
        self->onExit(self);
    }
}

struct Watchdog_Reporter *Watchdog_Reporter_newJsonReporter(FILE *stream) {
    struct Watchdog_Reporter *self = malloc(sizeof(*self) + sizeof(struct Watchdog_JsonReporter_Context));
    if (self) {
        self->onEnter = Watchdog_JsonReporter_onEnter;
        self->report = Watchdog_JsonReporter_report;
        self->onExit = Watchdog_JsonReporter_onExit;
        self->stream = stream;
        self->context = (self + 1);
        return self;
    }
    Panic_terminate("Out of memory");
}

struct Watchdog_Reporter *Watchdog_Reporter_newYamlReporter(FILE *stream) {
    struct Watchdog_Reporter *self = malloc(sizeof(*self));
    if (self) {
        self->stream = stream;
        self->report = Watchdog_YamlReporter_report;
        self->onEnter = self->onExit = self->context = NULL;
        return self;
    }
    Panic_terminate("Out of memory");
}

void Watchdog_Reporter_delete(struct Watchdog_Reporter *self) {
    if (self) {
        if (self->stream && self->stream != stdout && self->stream != stderr) {
            fclose(self->stream);
        }
        free(self);
    }
}

void Watchdog_JsonReporter_onEnter(struct Watchdog_Reporter *const self) {
    assert(self);
    assert(self->context);
    struct Watchdog_JsonReporter_Context *context = self->context;
    context->needsHeader = true;
    context->needsFooter = false;
    context->hasReportedSomething = false;
}

void Watchdog_JsonReporter_report(struct Watchdog_Reporter *const self, const struct Watchdog_Chunk *const chunk) {
    assert(self);
    assert(self->context);
    assert(chunk);
    assert(chunk->trace);
    struct Watchdog_JsonReporter_Context *context = self->context;
    if (context->needsHeader) {
        fprintf(self->stream, "{");
        context->needsHeader = false;
        context->needsFooter = true;
    }
    if (!context->hasReportedSomething) {
        context->hasReportedSomething = true;
    } else {
        fprintf(self->stream, ", ");
    }
    fprintf(self->stream, "\"%p\": {\"status\": \"%s\", \"chunks\": [", chunk->memory,
            Watchdog_Chunk_explainStatus(chunk));
    for (struct Watchdog_Trace *trace = chunk->trace; trace; trace = trace->prev) {
        fprintf(self->stream, "{\"href\": \"%s:%d\", ", trace->file, trace->line);
        fprintf(self->stream, "\"file\": \"%s\", ", trace->file);
        fprintf(self->stream, "\"line\": %d, ", trace->line);
        fprintf(self->stream, "\"size\": %zu, ", trace->size);
        fprintf(self->stream, "\"call\": \"%s\"}", Watchdog_Trace_Call_toString(trace->call));
        if (trace->prev) {
            fprintf(self->stream, ", ");
        }
    }
    fprintf(self->stream, "]}");
}

void Watchdog_JsonReporter_onExit(struct Watchdog_Reporter *const self) {
    assert(self);
    assert(self->context);
    struct Watchdog_JsonReporter_Context *context = self->context;
    if (context->needsFooter) {
        fprintf(self->stream, "}");
    }
}

void Watchdog_YamlReporter_report(struct Watchdog_Reporter *const self, const struct Watchdog_Chunk *const chunk) {
    assert(self);
    assert(chunk);
    assert(chunk->trace);
    fprintf(self->stream, "\"%p\":\n", chunk->memory);
    fprintf(self->stream, "  status: %s\n", Watchdog_Chunk_explainStatus(chunk));
    fprintf(self->stream, "  chunks:\n");
    for (struct Watchdog_Trace *trace = chunk->trace; trace; trace = trace->prev) {
        fprintf(self->stream, "    - href: %s:%d\n", trace->file, trace->line);
        fprintf(self->stream, "      file: %s\n", trace->file);
        fprintf(self->stream, "      line: %d\n", trace->line);
        fprintf(self->stream, "      size: %zu\n", trace->size);
        fprintf(self->stream, "      call: %s\n", Watchdog_Trace_Call_toString(trace->call));
    }
    fprintf(self->stream, "\n");
}

/*
 * Watchdog_Dumper
 */
struct Watchdog_Dumper {
    struct Watchdog_Reporter *reporter;
};

void Watchdog_Dumper_onEnter(struct Watchdog_Dumper *self) {
    assert(self);
    assert(self->reporter);
    self->reporter->stream = NULL;
}

void Watchdog_Dumper_dump(struct Watchdog_Dumper *const self, const struct Watchdog_Chunk *const chunk) {
    assert(self);
    assert(self->reporter);
    assert(chunk);
    if (!self->reporter->stream) {
        char buffer[1024] = "";
        snprintf(buffer, (sizeof(buffer) / sizeof(buffer[0])) - 1, "watchdog-%zu.dump", (size_t) time(NULL));
        self->reporter->stream = fopen(buffer, "w");
        if (!self->reporter->stream) {
            Panic_terminate("Unable to pen file: %s", buffer);
        }
    }
    Watchdog_Reporter_report(self->reporter, chunk);
}

void Watchdog_Dumper_onExit(struct Watchdog_Dumper *self) {
    assert(self);
    assert(self->reporter);
    if (self->reporter->stream) {
        assert(self->reporter->stream != stderr);
        assert(self->reporter->stream != stdout);
        fclose(self->reporter->stream);
    }
}

struct Watchdog_Dumper *Watchdog_Dumper_new(struct Watchdog_Reporter *reporter) {
    assert(reporter);
    assert(!reporter->stream);
    struct Watchdog_Dumper *self = malloc(sizeof(*self));
    if (self) {
        self->reporter = reporter;
        return self;
    }
    Panic_terminate("Out of memory");
}

void Watchdog_Dumper_delete(struct Watchdog_Dumper *self) {
    if (self) {
        assert(self->reporter);
        Watchdog_Reporter_delete(self->reporter);
        free(self);
    }
}

/*
 * Watchdog_Visit
 */
struct Watchdog_Chunk *Watchdog_Visit_traverse(struct Watchdog_Chunk *start, Watchdog_Chunk_VisitFn visit) {
    Watchdog_Dumper_onEnter(gDumper);
    Watchdog_Reporter_onEnter(gReporter);
    for (struct Watchdog_Chunk *beforeMe = NULL, *current = start; current;) {
        switch (visit(current)) {
            case WATCHDOG_VISIT_NO_OP:
                beforeMe = current;
                current = current->prev;
                continue;
            case WATCHDOG_VISIT_DUMP:
                Watchdog_Dumper_dump(gDumper, current);
                beforeMe = current;
                current = current->prev;
                continue;
            case WATCHDOG_VISIT_REPORT:
                Watchdog_Reporter_report(gReporter, current);
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
                Watchdog_Reporter_report(gReporter, current);
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
    Watchdog_Reporter_onExit(gReporter);
    Watchdog_Dumper_onExit(gDumper);
    return start;
}

enum Watchdog_Visit Watchdog_Visit_reportAllAndCollectFreedChunks(const struct Watchdog_Chunk *const chunk) {
    assert(chunk);
    if (chunk->trace) {
        return (WATCHDOG_TRACE_CALL_FREE == chunk->trace->call) ? WATCHDOG_VISIT_REPORT_AND_COLLECT
                                                                : WATCHDOG_VISIT_REPORT;
    }
    return WATCHDOG_VISIT_COLLECT;
}

enum Watchdog_Visit Watchdog_Visit_dumpAliveChunks(const struct Watchdog_Chunk *const chunk) {
    assert(chunk);
    if (chunk->trace && WATCHDOG_TRACE_CALL_FREE != chunk->trace->call) {
        return WATCHDOG_VISIT_DUMP;
    }
    return WATCHDOG_VISIT_NO_OP;
}

enum Watchdog_Visit Watchdog_Visit_reportAndCollectFreedChunks(const struct Watchdog_Chunk *const chunk) {
    assert(chunk);
    if (chunk->trace) {
        return (WATCHDOG_TRACE_CALL_FREE == chunk->trace->call) ? WATCHDOG_VISIT_REPORT_AND_COLLECT
                                                                : WATCHDOG_VISIT_NO_OP;
    }
    return WATCHDOG_VISIT_COLLECT;
}

enum Watchdog_Visit Watchdog_Visit_collectAllChunks(const struct Watchdog_Chunk *const chunk) {
    (void) chunk;
    assert(chunk);
    return WATCHDOG_VISIT_COLLECT;
}

void Watchdog_signalHandler(int signo) {
    (void) signo;
    assert(WATCHDOG_SIGNAL == signo);
    gChunksHead = Watchdog_Visit_traverse(gChunksHead, Watchdog_Visit_dumpAliveChunks);
}
