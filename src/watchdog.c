/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   April 28th 2016
 *  email:  daddinuz@gmail.com
 */

#include "Watchdog/watchdog.h"

/*
 * Undefine overrides over stdlib.h
 */
#undef malloc
#undef calloc
#undef realloc
#undef free
#undef exit
#undef abort

/*
 * Re-include stdlib.h
 */
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "colt/xor_list.h"


/*
 * Struct to hold memory block's info
 */
typedef struct memory_trace_t {
    char *action;
    char *file;
    size_t line;
    size_t size;
} memory_trace_t;

typedef struct memory_chunk_t {
    XorList_t *history;
} memory_chunk_t;

static XorList_t *__memory = NULL;
static size_t __allocations = 0, __reallocations = 0, __frees = 0;
static FILE *__stream = NULL;

/* Check if a stream is set otherwise redirect to stderr */
#define STREAM ((NULL != __stream) ? __stream : stderr)

/*
 * Internal functions
 */
static memory_chunk_t *__watchdog_allocate(size_t size, char *file, size_t line, bool clear) {
    memory_chunk_t *new_chunk = malloc(sizeof(memory_chunk_t) + size);
    /** Do not try to handle allocation errors, malloc/calloc behaviour must not be altered **/
    if (NULL == new_chunk) {
        fprintf(STREAM, "[MEMORY] FAIL: %s(%zu) in (%s:%zu)\n", clear ? "calloc" : "malloc", size, file, line);
        return NULL;
    }
    new_chunk->history = xor_list_new();
    __allocations += 1;
    if (true == clear) {
        memset(new_chunk + 1, 0, size);
    }
    memory_trace_t *trace = malloc(sizeof(memory_trace_t));
    assert(NULL != trace);
    trace->action = clear ? "calloc" : "malloc";
    trace->file = file;
    trace->line = line;
    trace->size = size;
    xor_list_push_back(new_chunk->history, trace);
    return new_chunk;
}

static memory_chunk_t *__watchdog_reallocate(void *ptr, size_t size, char *file, size_t line) {
    memory_chunk_t *chunk = (memory_chunk_t *) ptr - 1;
    /** Do not try to handle realloc errors, realloc behaviour must not be altered **/
    if (NULL == realloc(chunk, sizeof(memory_chunk_t) + size)) {
        fprintf(STREAM, "[MEMORY] FAIL: realloc(%p, %zu) in (%s:%zu)\n", ptr, size, file, line);
        return NULL;
    }
    __reallocations += 1;
    memory_trace_t *trace = malloc(sizeof(memory_trace_t));
    assert(NULL != trace);
    trace->action = "realloc";
    trace->file = file;
    trace->line = line;
    trace->size = size;
    xor_list_push_back(chunk->history, trace);
    return chunk;
}

static void __watchdog_free(void *ptr, char *file, size_t line) {
    memory_chunk_t *chunk = (memory_chunk_t *) ptr - 1;
    /*
     * C standard, 7.20.3.2/2 from ISO-IEC 9899: 
     * If ptr is a null pointer, no action occurs.
     */
    if (NULL != ptr) {
        assert(NULL != realloc(chunk, sizeof(memory_chunk_t)));
    }
    __frees += 1;
    memory_trace_t *trace = malloc(sizeof(memory_trace_t));
    assert(NULL != trace);
    trace->action = "free";
    trace->file = file;
    trace->line = line;
    trace->size = 0;
    xor_list_push_back(chunk->history, trace);
}

/* If track is enabled log the call on the appropriate stream else do nothing */
#define TRACE(stream, format, ...) \
    do { \
        fprintf(STREAM, format, __VA_ARGS__); \
    } while (0)

/*
 * Public functions
 */
void watchdog_initialize(void) {
    __memory = xor_list_new();
}

void watchdog_terminate(void) {
    memory_trace_t *current_trace = NULL;
    memory_chunk_t *current_chunk = NULL;
    for (size_t c = xor_list_size(__memory); c > 0 ; c--) {
        current_chunk = xor_list_pop_back(__memory);
        for (size_t t = xor_list_size(current_chunk->history); t > 0; t--) {
            current_trace = xor_list_pop_back(current_chunk->history);
            free(current_trace);
        }
        xor_list_delete(current_chunk->history);
        free(current_chunk);
    }
    xor_list_delete(__memory);
}

void watchdog_set_output_stream(FILE *stream) {
    __stream = stream;
}

void watchdog_dump(void) {
    size_t bytes_allocated = 0;
    memory_trace_t *current_trace = NULL;
    memory_chunk_t *current_chunk = NULL;
    fprintf(STREAM, "[MEMORY] DUMP: %zu allocations, %zu reallocations, %zu frees\n",
            __allocations, __reallocations, __frees);
    for (size_t c = 0; c < xor_list_size(__memory); c++) {
        current_chunk = xor_list_get(__memory, c);
        fprintf(STREAM, "[MEMORY] address %p:\n", (void *) (current_chunk + 1));
        for (size_t t = 0; t < xor_list_size(current_chunk->history); t++) {
            current_trace = xor_list_get(current_chunk->history, t);
            fprintf(STREAM, "[MEMORY] %*c %s at (%s:%zu): %4zu bytes allocated\n",
                    8, ' ', current_trace->action, current_trace->file, current_trace->line, current_trace->size
            );
        }
        bytes_allocated += ((memory_trace_t *) xor_list_back(current_chunk->history))->size;
    }
    fprintf(STREAM, "[MEMORY] %*c HEAP: %zu bytes currently allocated\n", 5, ' ', bytes_allocated);
}

void watchdog_collect(void) {
    size_t bytes_collected = 0;
    memory_trace_t *current_trace = NULL;
    memory_chunk_t *current_chunk = NULL;
    fprintf(STREAM, "[MEMORY] WARN: running garbage collection\n");
    for (size_t c = 0; c < xor_list_size(__memory); c++) {
        current_chunk = xor_list_get(__memory, c);
        fprintf(STREAM, "[MEMORY] address %p:\n", (void *) (current_chunk + 1));
        current_trace = ((memory_trace_t *) xor_list_back(current_chunk->history));
        bytes_collected += current_trace->size;
        fprintf(STREAM, "[MEMORY] %*c collecting %3zu bytes\n",
                8, ' ', current_trace->size
        );
        __watchdog_free(current_chunk + 1, "<garbage collector>", 0);
    }
    fprintf(STREAM, "[MEMORY] %*c HEAP: %zu bytes collected\n", 5, ' ', bytes_collected);
}

/*
 * Protected functions
 */
void *watchdog_malloc(size_t size, char *_file, size_t _line) {
    TRACE(STREAM, "[MEMORY] CALL: malloc(%zu) in (%s:%zu)\n", size, _file, _line);
    memory_chunk_t *chunk = __watchdog_allocate(size, _file, _line, false);
    if (NULL == chunk) {
        return NULL;
    }
    xor_list_push_back(__memory, chunk);
    return chunk + 1;
}

void *watchdog_calloc(size_t num, size_t size, char *_file, size_t _line) {
    TRACE(STREAM, "[MEMORY] CALL: calloc(%zu, %zu) in (%s:%zu)\n", num, size, _file, _line);
    memory_chunk_t *chunk = __watchdog_allocate(num * size, _file, _line, true);
    if (NULL == chunk) {
        return NULL;
    }
    xor_list_push_back(__memory, chunk);
    return chunk + 1;
}

void *watchdog_realloc(void *ptr, size_t size, char *_file, size_t _line) {
    TRACE(STREAM, "[MEMORY] CALL: realloc(%p, %zu) in (%s:%zu)\n", ptr, size, _file, _line);
    memory_chunk_t *chunk = __watchdog_reallocate(ptr, size, _file, _line);
    if (NULL == chunk) {
        return NULL;
    }
    return chunk + 1;
}

void watchdog_free(void *ptr, char *_file, size_t _line) {
    TRACE(STREAM, "[MEMORY] CALL: free(%p) in (%s:%zu)\n", ptr, _file, _line);
    __watchdog_free(ptr, _file, _line);
}

void watchdog_exit(int status, char *_file, size_t _line) {
    TRACE(STREAM, "[MEMORY] CALL: exit(%d) in (%s:%zu)\n", status, _file, _line);
    exit(status);
}

void watchdog_abort(char *_file, size_t _line) {
    TRACE(STREAM, "[MEMORY] CALL: abort() in (%s:%zu)\n", _file, _line);
    abort();
}
