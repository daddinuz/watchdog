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


typedef struct memory_trace_t {
    char *action;
    char *file;
    size_t line;
    size_t size;
} memory_trace_t;

typedef struct memory_info_t {
    XorList_t *trace_list;
    void *address;
    bool allocated;
} memory_info_t;

typedef struct memory_chunk_t {
    memory_info_t *info;
} memory_chunk_t;

static FILE *__stream = NULL;
static XorList_t *__info_list = NULL, *__chunk_list = NULL; /* TODO : use skip lists instead of xor lists or use iterators */
static size_t __allocations = 0, __reallocations = 0, __frees = 0;

/* Check if a stream is set otherwise redirect to stderr */
#define STREAM ((NULL != __stream) ? __stream : stderr)

/*
 * Internal functions
 */
static void *__watchdog_allocate(size_t size, char *file, size_t line, bool clear) {
    memory_chunk_t *chunk = malloc(sizeof(memory_chunk_t) + size);
    /** Do not try to handle allocation errors, malloc/calloc behaviour must not be altered **/
    if (NULL == chunk) {
        return NULL;
    }
    if (clear) {
        memset(chunk + 1, 0, size);
    }
    __allocations += 1;
    xor_list_push_back(__chunk_list, chunk);
    memory_info_t *info = malloc(sizeof(memory_info_t));
    assert(NULL != info);
    info->trace_list = xor_list_new();
    info->address = (chunk + 1);
    info->allocated = true;
    memory_trace_t *trace = malloc(sizeof(memory_trace_t));
    assert(NULL != trace);
    trace->action = clear ? "calloc" : "malloc";
    trace->file = file;
    trace->line = line;
    trace->size = size;
    xor_list_push_back(info->trace_list, trace);
    xor_list_push_back(__info_list, info);
    chunk->info = info;
    return chunk + 1;
}

static void *__watchdog_reallocate(void *ptr, size_t size, char *file, size_t line) {
    memory_chunk_t *chunk = (memory_chunk_t *) ptr - 1;
    memory_info_t *info = chunk->info;
    /** Do not try to handle realloc errors, realloc behaviour must not be altered **/
    if (NULL == (chunk = realloc(chunk, sizeof(memory_chunk_t) + size))) {
        return NULL;
    }
    __reallocations += 1;
    xor_list_push_back(__chunk_list, chunk);
    chunk->info = info;
    info->address = (chunk + 1);
    info->allocated = true;
    memory_trace_t *trace = malloc(sizeof(memory_trace_t));
    assert(NULL != trace);
    trace->action = "realloc";
    trace->file = file;
    trace->line = line;
    trace->size = size;
    xor_list_push_back(info->trace_list, trace);
    return chunk + 1;
}

static void __watchdog_free(void *ptr, char *file, size_t line) {
    memory_chunk_t *chunk = (memory_chunk_t *) ptr - 1;
    memory_info_t *info = chunk->info;
    memory_trace_t *trace = malloc(sizeof(memory_trace_t));
    assert(NULL != trace);
    trace->action = "free";
    trace->file = file;
    trace->line = line;
    trace->size = 0;
    info->allocated = false;
    xor_list_push_back(info->trace_list, trace);
    free(chunk);
    __frees += 1;
}

/*
 * Public functions
 */
void watchdog_initialize(void) {
    __info_list = xor_list_new();
    __chunk_list = xor_list_new();
}

void watchdog_terminate(void) {
    memory_trace_t *current_trace = NULL;
    memory_info_t *current_info = NULL;
    for (size_t i = xor_list_size(__info_list); i > 0; i--) {
        current_info = xor_list_pop_back(__info_list);
        for (size_t t = xor_list_size(current_info->trace_list); t > 0; t--) {
            current_trace = xor_list_pop_back(current_info->trace_list);
            free(current_trace);
        }
        xor_list_delete(current_info->trace_list);
        free(current_info);
    }
    xor_list_delete(__info_list);
    xor_list_delete(__chunk_list);
}

void watchdog_set_output_stream(FILE *stream) {
    __stream = stream;
}

void watchdog_dump(void) {
    size_t bytes_allocated = 0;
    memory_trace_t *current_trace = NULL;
    memory_info_t *current_info = NULL;
    fprintf(STREAM, "[WATCHDOG] DUMP: %zu allocations, %zu reallocations, %zu frees\n",
            __allocations, __reallocations, __frees);
    for (size_t c = 0; c < xor_list_size(__info_list); c++) {
        current_info = xor_list_get(__info_list, c);
        fprintf(STREAM, "[WATCHDOG] %-5s address %p:\n", "", current_info->address);
        for (size_t t = 0; t < xor_list_size(current_info->trace_list); t++) {
            current_trace = xor_list_get(current_info->trace_list, t);
            fprintf(STREAM, "[WATCHDOG] %-10s %-10s at (%s:%zu): %4zu bytes allocated\n", "",
                    current_trace->action, current_trace->file, current_trace->line, current_trace->size
            );
        }
        bytes_allocated += ((memory_trace_t *) xor_list_back(current_info->trace_list))->size;
    }
    fprintf(STREAM, "[WATCHDOG] %-5s ---- %zu bytes currently allocated\n", "", bytes_allocated);
}

void watchdog_collect(void) {
    size_t bytes_collected = 0;
    memory_trace_t *current_trace = NULL;
    memory_info_t *current_info = NULL;
    fprintf(STREAM, "[WATCHDOG] WARN: running garbage collection\n");
    for (size_t c = 0; c < xor_list_size(__info_list); c++) {
        current_info = xor_list_get(__info_list, c);
        fprintf(STREAM, "[WATCHDOG] %-5s address %p:\n", "", current_info->address);
        current_trace = ((memory_trace_t *) xor_list_back(current_info->trace_list));
        bytes_collected += current_trace->size;
        fprintf(STREAM, "[WATCHDOG] %-10s collecting %3zu bytes\n", "", current_trace->size
        );
        if (current_info->allocated) {
            __watchdog_free(current_info->address, "<garbage collector>", 0);
        }
    }
    fprintf(STREAM, "[WATCHDOG] %-5s ---- %zu bytes collected\n", "", bytes_collected);
}

/*
 * Protected functions
 */
void *watchdog_malloc(size_t size, char *_file, size_t _line) {
    fprintf(STREAM, "[WATCHDOG] CALL: at (%s:%zu) -- malloc  : %zu bytes requested\n", _file, _line, size);
    void *data = __watchdog_allocate(size, _file, _line, false);
    if (data) {
        fprintf(STREAM, "[WATCHDOG] DONE: at (%s:%zu) -- malloc  : %zu bytes allocated to address %p\n", _file, _line, size, data);
    } else {
        fprintf(STREAM, "[WATCHDOG] FAIL: at (%s:%zu) -- malloc  : unable to allocate %zu bytes\n", _file, _line, size);
    }
    return data;
}

void *watchdog_calloc(size_t num, size_t size, char *_file, size_t _line) {
    const size_t actual_size = num * size;
    fprintf(STREAM, "[WATCHDOG] CALL: at (%s:%zu) -- calloc  : %zu bytes requested\n", _file, _line, actual_size);
    void *data = __watchdog_allocate(actual_size, _file, _line, true);
    if (data) {
        fprintf(STREAM, "[WATCHDOG] DONE: at (%s:%zu) -- calloc  : %zu bytes allocated to address %p\n", _file, _line, actual_size, data);
    } else {
        fprintf(STREAM, "[WATCHDOG] FAIL: at (%s:%zu) -- calloc  : unable to allocate %zu bytes\n", _file, _line, actual_size);
    }
    return data;
}

void *watchdog_realloc(void *ptr, size_t size, char *_file, size_t _line) {
    fprintf(STREAM, "[WATCHDOG] CALL: at (%s:%zu) -- realloc : %zu bytes requested\n", _file, _line, size);
    void *data = __watchdog_reallocate(ptr, size, _file, _line);
    if (data) {
        fprintf(STREAM, "[WATCHDOG] DONE: at (%s:%zu) -- realloc : %zu bytes reallocated from address %p to address %p\n", _file, _line, size, ptr, data);
    } else {
        fprintf(STREAM, "[WATCHDOG] FAIL: at (%s:%zu) -- realloc : unable to allocate %zu bytes\n", _file, _line, size);
    }
    return data;
}

void watchdog_free(void *ptr, char *_file, size_t _line) {
    fprintf(STREAM, "[WATCHDOG] CALL: at (%s:%zu) -- free    : address %p\n", _file, _line, ptr);
    __watchdog_free(ptr, _file, _line);
}

void watchdog_exit(int status, char *_file, size_t _line) {
    fprintf(STREAM, "[WATCHDOG] CALL: at (%s:%zu) -- exit    : status %d\n", _file, _line, status);
    exit(status);
}

void watchdog_abort(char *_file, size_t _line) {
    fprintf(STREAM, "[WATCHDOG] CALL: at (%s:%zu) -- abort\n", _file, _line);
    abort();
}
