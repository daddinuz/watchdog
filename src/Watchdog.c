/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   April 28th 2016
 *  email:  daddinuz@gmail.com
 */

#include "Watchdog/Watchdog.h"

/*
 * Un-define overrides over stdlib.h
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
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "colt/xor_list.h"


typedef enum call_t {
    CALL_MALLOC,
    CALL_CALLOC,
    CALL_REALLOC,
    CALL_FREE,
} call_t;

typedef struct trace_t {
    char *file;
    size_t line;
    size_t size;
    call_t func;
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
static bool __initialized = false;
static XorList_t *__info_list = NULL, *__chunk_list = NULL; /* TODO use iterators (when Colt will implement them) */
static size_t __bytes_allocated = 0, __bytes_freed = 0, __bytes_collected = 0;
static size_t __allocations_counter = 0, __frees_counter = 0;

/*
 * Internal functions declaration
 */
static void __watchdog_initialize(void);
static void __watchdog_terminate(void);
static void __watchdog_dump(void);
static void __watchdog_collect(void);
static char *__watchdog_call_to_string(call_t call);
static void *__watchdog_allocate(size_t size, bool clear, char *file, size_t line);
static void *__watchdog_reallocate(void *ptr, size_t size, char *file, size_t line);
static size_t __watchdog_free(void *ptr, char *file, size_t line);

/*
 * Protected functions definitions
 */
void *_watchdog_malloc(size_t size, char *_file, size_t _line) {
    __watchdog_initialize();
    void *data = __watchdog_allocate(size, false, _file, _line);
    if (data) {
        printf("[WATCHDOG] INFO: malloc  at %s:%04zu\n", _file, _line);
        printf("[WATCHDOG] %-5s %zu bytes allocated to address %p\n", "", size, data);
    } else {
        printf("[WATCHDOG] ERROR: malloc at %s:%04zu\n", _file, _line);
        printf("[WATCHDOG] %-5s failed to allocate %zu bytes\n", "", size);
    }
    return data;
}

void *_watchdog_calloc(size_t num, size_t size, char *_file, size_t _line) {
    __watchdog_initialize();
    const size_t actual_size = num * size;
    void *data = __watchdog_allocate(actual_size, true, _file, _line);
    if (data) {
        printf("[WATCHDOG] INFO: calloc  at %s:%04zu\n", _file, _line);
        printf("[WATCHDOG] %-5s %zu bytes allocated to address %p\n", "", actual_size, data);
    } else {
        printf("[WATCHDOG] ERROR: calloc at %s:%04zu\n", _file, _line);
        printf("[WATCHDOG] %-5s failed to allocate %zu bytes\n", "", actual_size);
    }
    return data;
}

void *_watchdog_realloc(void *ptr, size_t size, char *_file, size_t _line) {
    __watchdog_initialize();
    void *data = __watchdog_reallocate(ptr, size, _file, _line);
    if (data) {
        printf("[WATCHDOG] INFO: realloc at %s:%04zu\n", _file, _line);
        printf("[WATCHDOG] %-5s %zu bytes reallocated from address %p to address %p\n", "", size, ptr, data);
    } else {
        printf("[WATCHDOG] ERROR: realloc at %s:%04zu\n", _file, _line);
        printf("[WATCHDOG] %-5s failed to reallocate %zu bytes\n", "", size);
    }
    return data;
}

void _watchdog_free(void *ptr, char *_file, size_t _line) {
    __watchdog_initialize();
    const size_t bytes_freed = __watchdog_free(ptr, _file, _line);
    printf("[WATCHDOG] INFO: free    at %s:%04zu\n", _file, _line);
    printf("[WATCHDOG] %-5s %zu bytes freed from address %p\n", "", bytes_freed, ptr);
}

void _watchdog_exit(int status, char *_file, size_t _line) {
    __watchdog_initialize();
    printf("[WATCHDOG] INFO: exit    at %s:%04zu\n", _file, _line);
    printf("[WATCHDOG] %-5s exit code: %d\n", "", status);
    exit(status);
}

void _watchdog_abort(char *_file, size_t _line) {
    __watchdog_initialize();
    printf("[WATCHDOG] WARN: abort   at %s:%04zu\n", _file, _line);
    abort();
}

/*
 * Internal functions definitions
 */
void __watchdog_initialize(void) {
    if (__initialized) {
        return;
    }
    __info_list = xor_list_new();
    __chunk_list = xor_list_new();
    atexit(__watchdog_terminate);
    atexit(__watchdog_dump);
    atexit(__watchdog_collect);
    __initialized = true;
    printf("[WATCHDOG] INFO: Watchdog Initialized\n");
}

void __watchdog_terminate(void) {
    assert(__initialized);
    info_t *current_info = NULL;
    trace_t *current_trace = NULL;
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
    printf("[WATCHDOG] INFO: Watchdog Terminated\n");
    __initialized = false;
    __info_list = __chunk_list = NULL;
    __allocations_counter = __frees_counter = 0;
}

void __watchdog_dump(void) {
    assert(__initialized);
    info_t *current_info = NULL;
    trace_t *current_trace = NULL;
    printf("[WATCHDOG] INFO: Summary\n");
    const size_t outer_size = xor_list_size(__info_list);
    for (size_t i = 0; i < outer_size; i++) {
        current_info = xor_list_get(__info_list, i);
        printf("[WATCHDOG] %-8s address %p:\n", "", current_info->address);
        const size_t inner_size = xor_list_size(current_info->trace_list);
        for (size_t x = 0; x < inner_size - 1; x++) {
            current_trace = xor_list_get(current_info->trace_list, x);
            printf("[WATCHDOG] %-16s %-7s at %65s:%04zu | %2zu bytes were in use\n", "",
                   __watchdog_call_to_string(current_trace->func), current_trace->file, current_trace->line,
                   current_trace->size);
        }
        current_trace = xor_list_back(current_info->trace_list);
        printf("[WATCHDOG] %-16s %-7s at %65s:%04zu | %2zu bytes currently allocated\n", "",
               __watchdog_call_to_string(current_trace->func), current_trace->file, current_trace->line,
               current_trace->size);
    }
    printf("[WATCHDOG] %-5s %zu allocations, %zu frees\n", "", __allocations_counter, __frees_counter);
    printf("[WATCHDOG] %-5s %zu bytes allocated, %zu bytes freed (whereof %zu bytes collected on exit)\n", "",
           __bytes_allocated, __bytes_freed, __bytes_collected);
}

void __watchdog_collect(void) {
    assert(__initialized);
    info_t *current_info = NULL;
    trace_t *current_trace = NULL;
    printf("[WATCHDOG] WARN: Garbage Collector\n");
    const size_t info_list_size = xor_list_size(__info_list);
    for (size_t i = 0; i < info_list_size; i++) {
        current_info = xor_list_get(__info_list, i);
        current_trace = ((trace_t *) xor_list_back(current_info->trace_list));
        if (current_info->allocated) {
            printf("[WATCHDOG] %-8s address %p:\n", "", current_info->address);
            printf("[WATCHDOG] %-16s %-7s at %65s:%04zu | %2zu bytes still allocated\n", "",
                   __watchdog_call_to_string(current_trace->func), current_trace->file, current_trace->line,
                   current_trace->size);
            __watchdog_free(current_info->address, "<garbage collector>", 0);
            __bytes_collected += current_trace->size;
        }
    }
    printf("[WATCHDOG] %-5s %zu bytes collected\n", "", __bytes_collected);
}

static char *__watchdog_call_to_string(call_t call) {
    switch (call) {
        case CALL_MALLOC:
            return "malloc";
        case CALL_CALLOC:
            return "calloc";
        case CALL_REALLOC:
            return "realloc";
        case CALL_FREE:
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
    xor_list_push_back(__chunk_list, chunk);
    info_t *info = malloc(sizeof(info_t));
    assert(NULL != info);
    info->trace_list = xor_list_new();
    info->address = (chunk + 1);
    info->allocated = true;
    trace_t *trace = malloc(sizeof(trace_t));
    assert(NULL != trace);
    trace->func = clear ? CALL_CALLOC : CALL_MALLOC;
    trace->file = file;
    trace->line = line;
    trace->size = size;
    xor_list_push_back(info->trace_list, trace);
    xor_list_push_back(__info_list, info);
    chunk->info = info;
    __bytes_allocated += size;
    __allocations_counter += 1;
    return chunk + 1;
}

static void *__watchdog_reallocate(void *ptr, size_t size, char *file, size_t line) {
    chunk_t *chunk = (chunk_t *) ptr - 1;
    info_t *info = chunk->info;
    const size_t old_size = ((trace_t *) xor_list_back(info->trace_list))->size;
    if (NULL == (chunk = realloc(chunk, sizeof(chunk_t) + size))) {
        return NULL;
    }
    xor_list_push_back(__chunk_list, chunk);
    chunk->info = info;
    info->address = (chunk + 1);
    info->allocated = true;
    trace_t *trace = malloc(sizeof(trace_t));
    assert(NULL != trace);
    trace->func = CALL_REALLOC;
    trace->file = file;
    trace->line = line;
    trace->size = size;
    xor_list_push_back(info->trace_list, trace);
    __bytes_allocated += size;
    __bytes_freed += old_size;
    return chunk + 1;
}

static size_t __watchdog_free(void *ptr, char *file, size_t line) {
    chunk_t *chunk = (chunk_t *) ptr - 1;
    info_t *info = chunk->info;
    const size_t bytes_freed = ((trace_t *) xor_list_back(info->trace_list))->size;
    trace_t *trace = malloc(sizeof(trace_t));
    assert(NULL != trace);
    trace->func = CALL_FREE;
    trace->file = file;
    trace->line = line;
    trace->size = 0;
    info->allocated = false;
    xor_list_push_back(info->trace_list, trace);
    free(chunk);
    __frees_counter += 1;
    __bytes_freed += bytes_freed;
    return bytes_freed;
}
