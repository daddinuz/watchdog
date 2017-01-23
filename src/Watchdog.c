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


typedef enum action_t {
    ACTION_MALLOC,
    ACTION_CALLOC,
    ACTION_REALLOC,
    ACTION_FREE,
} action_t;

typedef struct memory_trace_t {
    char *file;
    size_t line;
    size_t size;
    action_t action;
} memory_trace_t;

typedef struct memory_info_t {
    XorList_t *trace_list;
    void *address;
    bool allocated;
} memory_info_t;

typedef struct memory_chunk_t {
    memory_info_t *info;
} memory_chunk_t;

/*
 * Global variables
 */
static FILE *__stream = NULL;
static XorList_t *__info_list = NULL, *__chunk_list = NULL; /* TODO : use xor list iterators (when Colt will implement them) ;) */
static size_t __allocations_counter = 0, __frees_counter = 0;

/*
 * Internal functions declaration
 */
static char *__watchdog_straction(action_t action);
static void *__watchdog_allocate(size_t size, bool clear, char *file, size_t line);
static void *__watchdog_reallocate(void *ptr, size_t size, char *file, size_t line);
static size_t __watchdog_free(void *ptr, char *file, size_t line);

/*
 * Public functions definitions
 */
void watchdog_initialize(const char *path) {
    __info_list = xor_list_new();
    __chunk_list = xor_list_new();
    if (NULL == path || strcmp(path, "<stderr>") == 0) {
        __stream = stderr;
    } else if (strcmp(path, "<stdout>") == 0) {
        __stream = stdout;
    } else {
        __stream = fopen(path, "w");
        if (NULL == __stream) {
            fprintf(stderr, "Watchdog: unable to open file '%s'\n", path);
            abort();
        }
    }
    fputs("[WATCHDOG] INFO: Watchdog initialized\n", __stream);
}

void watchdog_terminate(void) {
    size_t bytes_allocated = 0, bytes_freed = 0, bytes_counter = 0;
    memory_trace_t *current_trace = NULL;
    memory_info_t *current_info = NULL;
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

void watchdog_dump(void) {
    size_t bytes_allocated = 0;
    memory_trace_t *current_trace = NULL;
    memory_info_t *current_info = NULL;
    fprintf(__stream, "[WATCHDOG] INFO: %zu allocations, %zu frees\n", __allocations_counter, __frees_counter);
    const size_t outer_size = xor_list_size(__info_list);
    for (size_t i = 0; i < outer_size; i++) {
        current_info = xor_list_get(__info_list, i);
        fprintf(__stream, "[WATCHDOG] %-5s address %p:\n", "", current_info->address);
        const size_t inner_size = xor_list_size(current_info->trace_list);
        for (size_t x = 0; x < inner_size - 1; x++) {
            current_trace = xor_list_get(current_info->trace_list, x);
            fprintf(__stream, "[WATCHDOG] %-10s at (%s:%zu) -- %-7s : %3zu bytes were in use\n", "",
                    current_trace->file, current_trace->line, __watchdog_straction(current_trace->action), current_trace->size);
        }
        current_trace = xor_list_back(current_info->trace_list);
        fprintf(__stream, "[WATCHDOG] %-10s at (%s:%zu) -- %-7s : %3zu bytes currently allocated\n", "",
                current_trace->file, current_trace->line, __watchdog_straction(current_trace->action), current_trace->size);
        bytes_allocated += ((memory_trace_t *) xor_list_back(current_info->trace_list))->size;
    }
    fprintf(__stream, "[WATCHDOG] %-5s %zu bytes currently allocated\n", "", bytes_allocated);
}

void watchdog_collect(void) {
    size_t bytes_collected = 0;
    memory_trace_t *current_trace = NULL;
    memory_info_t *current_info = NULL;
    fprintf(__stream, "[WATCHDOG] WARN: running garbage collection\n");
    for (size_t c = 0; c < xor_list_size(__info_list); c++) {
        current_info = xor_list_get(__info_list, c);
        current_trace = ((memory_trace_t *) xor_list_back(current_info->trace_list));
        if (current_info->allocated) {
            fprintf(__stream, "[WATCHDOG] %-5s address %p:\n", "", current_info->address);
            fprintf(__stream, "[WATCHDOG] %-10s %zu bytes currently allocated\n", "", current_trace->size);
            __watchdog_free(current_info->address, "<garbage collector>", 0);
            bytes_collected += current_trace->size;
        }
    }
    fprintf(__stream, "[WATCHDOG] %-5s %zu bytes collected\n", "", bytes_collected);
}

/*
 * Protected functions definitions
 */
void *watchdog_malloc(size_t size, char *_file, size_t _line) {
    void *data = __watchdog_allocate(size, false, _file, _line);
    if (data) {
        fprintf(__stream, "[WATCHDOG] INFO: at (%s:%zu) -- malloc  : %3zu bytes allocated to address %p\n", _file, _line, size, data);
    } else {
        fprintf(__stream, "[WATCHDOG] FATAL: at (%s:%zu) -- malloc  : unable to allocate %zu bytes\n", _file, _line, size);
    }
    return data;
}

void *watchdog_calloc(size_t num, size_t size, char *_file, size_t _line) {
    const size_t actual_size = num * size;
    void *data = __watchdog_allocate(actual_size, true, _file, _line);
    if (data) {
        fprintf(__stream, "[WATCHDOG] INFO: at (%s:%zu) -- calloc  : %3zu bytes allocated to address %p\n", _file, _line, actual_size, data);
    } else {
        fprintf(__stream, "[WATCHDOG] FATAL: at (%s:%zu) -- calloc  : unable to allocate %zu bytes\n", _file, _line, actual_size);
    }
    return data;
}

void *watchdog_realloc(void *ptr, size_t size, char *_file, size_t _line) {
    void *data = __watchdog_reallocate(ptr, size, _file, _line);
    if (data) {
        fprintf(__stream, "[WATCHDOG] INFO: at (%s:%zu) -- realloc : %3zu bytes reallocated from address %p to address %p\n", _file, _line, size, ptr, data);
    } else {
        fprintf(__stream, "[WATCHDOG] FATAL: at (%s:%zu) -- realloc : unable to allocate %zu bytes\n", _file, _line, size);
    }
    return data;
}

void watchdog_free(void *ptr, char *_file, size_t _line) {
    const size_t freed = __watchdog_free(ptr, _file, _line);
    fprintf(__stream, "[WATCHDOG] INFO: at (%s:%zu) -- free    : %3zu bytes freed from address %p\n",
            _file, _line, freed, ptr);
}

void watchdog_exit(int status, char *_file, size_t _line) {
    fprintf(__stream, "[WATCHDOG] INFO: at (%s:%zu) -- exit    : status %d\n", _file, _line, status);
    exit(status);
}

void watchdog_abort(char *_file, size_t _line) {
    fprintf(__stream, "[WATCHDOG] INFO: at (%s:%zu) -- abort\n", _file, _line);
    abort();
}

/*
 * Internal functions definitions
 */
static char *__watchdog_straction(action_t action) {
    switch (action) {
        case ACTION_MALLOC:
            return "malloc";
        case ACTION_CALLOC:
            return "calloc";
        case ACTION_REALLOC:
            return "realloc";
        case ACTION_FREE:
            return "free";
        default:
            return "unknown";
    }
}

static void *__watchdog_allocate(size_t size, bool clear, char *file, size_t line) {
    memory_chunk_t *chunk = clear ? calloc(1, sizeof(memory_chunk_t) + size) : malloc(sizeof(memory_chunk_t) + size);
    /** Do not try to handle allocation errors, malloc/calloc behaviour must not be altered **/
    if (NULL == chunk) {
        return NULL;
    }
    __allocations_counter += 1;
    xor_list_push_back(__chunk_list, chunk);
    memory_info_t *info = malloc(sizeof(memory_info_t));
    assert(NULL != info);
    info->trace_list = xor_list_new();
    info->address = (chunk + 1);
    info->allocated = true;
    memory_trace_t *trace = malloc(sizeof(memory_trace_t));
    assert(NULL != trace);
    trace->action = clear ? ACTION_CALLOC : ACTION_MALLOC;
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
    xor_list_push_back(__chunk_list, chunk);
    chunk->info = info;
    info->address = (chunk + 1);
    info->allocated = true;
    memory_trace_t *trace = malloc(sizeof(memory_trace_t));
    assert(NULL != trace);
    trace->action = ACTION_REALLOC;
    trace->file = file;
    trace->line = line;
    trace->size = size;
    xor_list_push_back(info->trace_list, trace);
    return chunk + 1;
}

static size_t __watchdog_free(void *ptr, char *file, size_t line) {
    memory_chunk_t *chunk = (memory_chunk_t *) ptr - 1;
    memory_info_t *info = chunk->info;
    const size_t freed = ((memory_trace_t *) xor_list_back(info->trace_list))->size;
    memory_trace_t *trace = malloc(sizeof(memory_trace_t));
    assert(NULL != trace);
    trace->action = ACTION_FREE;
    trace->file = file;
    trace->line = line;
    trace->size = 0;
    info->allocated = false;
    xor_list_push_back(info->trace_list, trace);
    free(chunk);
    __frees_counter += 1;
    return freed;
}
