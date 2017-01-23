/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   April 28th 2016
 *  email:  daddinuz@gmail.com
 */

#include "Watchdog/Watchdog.h"

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


typedef enum func_t {
    FUNC_MALLOC,
    FUNC_CALLOC,
    FUNC_REALLOC,
    FUNC_FREE,
} func_t;

typedef struct trace_t {
    char *file;
    size_t line;
    size_t size;
    func_t func;
} trace_t;

typedef struct info_t {
    XorList_t *trace_list;
    void *address;
    bool allocated;
} info_t;

typedef struct chunk_t {
    info_t *info;
} chunk_t;

/*
 * Global variables
 */
static FILE *__stream = NULL;
static XorList_t *__info_list = NULL, *__chunk_list = NULL; /* TODO use iterators (when Colt will implement them) */
static size_t __allocations_counter = 0, __frees_counter = 0;

/*
 * Internal functions declaration
 */
static char *__watchdog_func2string(func_t func);
static void *__watchdog_allocate(size_t size, bool clear, char *file, size_t line);
static void *__watchdog_reallocate(void *ptr, size_t size, char *file, size_t line);
static size_t __watchdog_free(void *ptr, char *file, size_t line);

/*
 * Public functions definitions
 */
void watchdog_initialize(FILE *stream) {
    __info_list = xor_list_new();
    __chunk_list = xor_list_new();
    __stream = stream ? stream : stderr;
    fputs("[WATCHDOG] INFO: Watchdog initialized\n", __stream);
}

void watchdog_terminate(void) {
    size_t bytes_allocated = 0, bytes_freed = 0, bytes_counter = 0;
    trace_t *current_trace = NULL;
    info_t *current_info = NULL;
    for (size_t i = xor_list_size(__info_list); i > 0; i--) {
        current_info = xor_list_pop_back(__info_list);
        for (size_t t = xor_list_size(current_info->trace_list); t > 0; t--) {
            current_trace = xor_list_pop_back(current_info->trace_list);
            bytes_counter += current_trace->size;
            free(current_trace);
        }
        bytes_allocated += bytes_counter;
        if (false == current_info->allocated) {
            bytes_freed += bytes_counter;
        }
        bytes_counter = 0;
        xor_list_delete(current_info->trace_list);
        free(current_info);
    }
    xor_list_delete(__info_list);
    xor_list_delete(__chunk_list);
    fputs("[WATCHDOG] INFO: Watchdog terminated\n", __stream);
    fprintf(__stream, "[WATCHDOG] %-5s %zu allocations, %zu frees\n", "", __allocations_counter, __frees_counter);
    fprintf(__stream, "[WATCHDOG] %-5s %zu bytes allocated, %zu bytes freed\n", "", bytes_allocated, bytes_freed);
    fclose(__stream);
}

/*
 * Protected functions definitions
 */
void _watchdog_dump(char *_file, size_t _line) {
    size_t bytes_allocated = 0, bytes_freed = 0, bytes_counter = 0;
    trace_t *current_trace = NULL;
    info_t *current_info = NULL;
    fprintf(__stream, "[WATCHDOG] INFO: summary at %s:%04zu\n", _file, _line);
    const size_t outer_size = xor_list_size(__info_list);
    for (size_t i = 0; i < outer_size; i++) {
        current_info = xor_list_get(__info_list, i);
        fprintf(__stream, "[WATCHDOG] %-8s address %p:\n", "", current_info->address);
        const size_t inner_size = xor_list_size(current_info->trace_list);
        for (size_t x = 0; x < inner_size - 1; x++) {
            current_trace = xor_list_get(current_info->trace_list, x);
            bytes_counter += current_trace->size;
            fprintf(__stream, "[WATCHDOG] %-16s %-7s at %s:%04zu | %2zu bytes were in use\n", "",
                    __watchdog_func2string(current_trace->func), current_trace->file, current_trace->line, current_trace->size);
        }
        current_trace = xor_list_back(current_info->trace_list);
        fprintf(__stream, "[WATCHDOG] %-16s %-7s at %s:%04zu | %2zu bytes currently allocated\n", "",
                __watchdog_func2string(current_trace->func), current_trace->file, current_trace->line, current_trace->size);
        bytes_counter += current_trace->size;
        bytes_allocated += bytes_counter;
        if (false == current_info->allocated) {
            bytes_freed += bytes_counter;
        }
        bytes_counter = 0;
    }
    fprintf(__stream, "[WATCHDOG] %-5s %zu allocations, %zu frees\n", "", __allocations_counter, __frees_counter);
    fprintf(__stream, "[WATCHDOG] %-5s %zu bytes allocated, %zu bytes freed\n", "", bytes_allocated, bytes_freed);
}

void _watchdog_collect(char *_file, size_t _line) {
    size_t bytes_collected = 0;
    trace_t *current_trace = NULL;
    info_t *current_info = NULL;
    fprintf(__stream, "[WATCHDOG] WARN: collect at %s:%04zu\n", _file, _line);
    const size_t info_list_size = xor_list_size(__info_list);
    for (size_t i = 0; i < info_list_size; i++) {
        current_info = xor_list_get(__info_list, i);
        current_trace = ((trace_t *) xor_list_back(current_info->trace_list));
        if (current_info->allocated) {
            fprintf(__stream, "[WATCHDOG] %-8s address %p:\n", "", current_info->address);
            fprintf(__stream, "[WATCHDOG] %-16s %-7s at %s:%04zu | %2zu bytes currently allocated\n", "",
                    __watchdog_func2string(current_trace->func), current_trace->file, current_trace->line, current_trace->size);
            __watchdog_free(current_info->address, "<garbage collector>", 0);
            bytes_collected += current_trace->size;
        }
    }
    fprintf(__stream, "[WATCHDOG] %-5s %zu bytes collected\n", "", bytes_collected);
}

void *_watchdog_malloc(size_t size, char *_file, size_t _line) {
    void *data = __watchdog_allocate(size, false, _file, _line);
    if (data) {
        fprintf(__stream, "[WATCHDOG] INFO: malloc  at %s:%04zu\n", _file, _line);
        fprintf(__stream, "[WATCHDOG] %-5s %zu bytes allocated to address %p\n", "", size, data);
    } else {
        fprintf(__stream, "[WATCHDOG] ERROR: malloc at %s:%04zu\n", _file, _line);
        fprintf(__stream, "[WATCHDOG] %-5s failed to allocate %zu bytes\n", "", size);
    }
    return data;
}

void *_watchdog_calloc(size_t num, size_t size, char *_file, size_t _line) {
    const size_t actual_size = num * size;
    void *data = __watchdog_allocate(actual_size, true, _file, _line);
    if (data) {
        fprintf(__stream, "[WATCHDOG] INFO: calloc  at %s:%04zu\n", _file, _line);
        fprintf(__stream, "[WATCHDOG] %-5s %zu bytes allocated to address %p\n", "", actual_size, data);
    } else {
        fprintf(__stream, "[WATCHDOG] ERROR: calloc at %s:%04zu\n", _file, _line);
        fprintf(__stream, "[WATCHDOG] %-5s failed to allocate %zu bytes\n", "", actual_size);
    }
    return data;
}

void *_watchdog_realloc(void *ptr, size_t size, char *_file, size_t _line) {
    void *data = __watchdog_reallocate(ptr, size, _file, _line);
    if (data) {
        fprintf(__stream, "[WATCHDOG] INFO: realloc at %s:%04zu\n", _file, _line);
        fprintf(__stream, "[WATCHDOG] %-5s %zu bytes reallocated from address %p to address %p\n", "", size, ptr, data);
    } else {
        fprintf(__stream, "[WATCHDOG] ERROR: realloc at %s:%04zu\n", _file, _line);
        fprintf(__stream, "[WATCHDOG] %-5s failed to reallocate %zu bytes\n", "", size);
    }
    return data;
}

void _watchdog_free(void *ptr, char *_file, size_t _line) {
    const size_t bytes_freed = __watchdog_free(ptr, _file, _line);
    fprintf(__stream, "[WATCHDOG] INFO: free    at %s:%04zu\n", _file, _line);
    fprintf(__stream, "[WATCHDOG] %-5s %zu bytes freed from address %p\n", "", bytes_freed, ptr);
}

void _watchdog_exit(int status, char *_file, size_t _line) {
    fprintf(__stream, "[WATCHDOG] INFO: exit    at %s:%04zu\n", _file, _line);
    fprintf(__stream, "[WATCHDOG] %-5s exit code: %d\n", "", status);
    exit(status);
}

void _watchdog_abort(char *_file, size_t _line) {
    fprintf(__stream, "[WATCHDOG] WARN: abort   at %s:%04zu\n", _file, _line);
    abort();
}

/*
 * Internal functions definitions
 */
static char *__watchdog_func2string(func_t func) {
    switch (func) {
        case FUNC_MALLOC:
            return "malloc";
        case FUNC_CALLOC:
            return "calloc";
        case FUNC_REALLOC:
            return "realloc";
        case FUNC_FREE:
            return "free";
        default:
            return "unknown";
    }
}

static void *__watchdog_allocate(size_t size, bool clear, char *file, size_t line) {
    chunk_t *chunk = clear ? calloc(1, sizeof(chunk_t) + size) : malloc(sizeof(chunk_t) + size);
    if (NULL == chunk) {
        return NULL;
    }
    __allocations_counter += 1;
    xor_list_push_back(__chunk_list, chunk);
    info_t *info = malloc(sizeof(info_t));
    assert(NULL != info);
    info->trace_list = xor_list_new();
    info->address = (chunk + 1);
    info->allocated = true;
    trace_t *trace = malloc(sizeof(trace_t));
    assert(NULL != trace);
    trace->func = clear ? FUNC_CALLOC : FUNC_MALLOC;
    trace->file = file;
    trace->line = line;
    trace->size = size;
    xor_list_push_back(info->trace_list, trace);
    xor_list_push_back(__info_list, info);
    chunk->info = info;
    return chunk + 1;
}

static void *__watchdog_reallocate(void *ptr, size_t size, char *file, size_t line) {
    chunk_t *chunk = (chunk_t *) ptr - 1;
    info_t *info = chunk->info;
    if (NULL == (chunk = realloc(chunk, sizeof(chunk_t) + size))) {
        return NULL;
    }
    xor_list_push_back(__chunk_list, chunk);
    chunk->info = info;
    info->address = (chunk + 1);
    info->allocated = true;
    trace_t *trace = malloc(sizeof(trace_t));
    assert(NULL != trace);
    trace->func = FUNC_REALLOC;
    trace->file = file;
    trace->line = line;
    trace->size = size;
    xor_list_push_back(info->trace_list, trace);
    return chunk + 1;
}

static size_t __watchdog_free(void *ptr, char *file, size_t line) {
    chunk_t *chunk = (chunk_t *) ptr - 1;
    info_t *info = chunk->info;
    const size_t bytes_freed = ((trace_t *) xor_list_back(info->trace_list))->size;
    trace_t *trace = malloc(sizeof(trace_t));
    assert(NULL != trace);
    trace->func = FUNC_FREE;
    trace->file = file;
    trace->line = line;
    trace->size = 0;
    info->allocated = false;
    xor_list_push_back(info->trace_list, trace);
    free(chunk);
    __frees_counter += 1;
    return bytes_freed;
}
