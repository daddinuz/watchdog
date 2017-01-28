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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include "colt/xor_list.h"


/*
 * Macro definitions
 */
#define UNUSED(x)  (void) x  /* quite compiler warnings where not needed*/

/*
 * Internal structures definition
 */
typedef enum call_t {
    CALL_MALLOC,
    CALL_CALLOC,
    CALL_REALLOC,
    CALL_FREE,
    CALL_EXIT,
    CALL_ABORT,
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
static FILE *__stream = NULL;
static bool __initialized = false;
static XorList_t *__info_list = NULL; /* TODO use iterators (when Colt will implement them) */
static size_t __allocations_counter = 0, __frees_counter = 0;
static size_t __bytes_allocated = 0, __bytes_freed = 0, __bytes_collected = 0;

/*
 * Internal functions declaration
 */
static void __watchdog_initialize(void);
static void __watchdog_terminate(void);
static void __watchdog_report(void);
static void __watchdog_collect(void);
static char *__watchdog_call_to_string(const call_t call);
static void __watchdog_log_call(const call_t call, const char *const file, const size_t line, ...);
static void *__watchdog_allocate(const size_t size, const bool clear, const char *const file, const size_t line);
static void *__watchdog_reallocate(void *ptr, const size_t size, const char *const file, const size_t line);
static size_t __watchdog_free(void *ptr, const char *const file, const size_t line);

/*
 * Protected functions definitions
 */
void *_watchdog_malloc(size_t size, char *_file, size_t _line) {
    __watchdog_initialize();
    void *data = __watchdog_allocate(size, false, _file, _line);
    __watchdog_log_call(CALL_MALLOC, _file, _line, size, data);
    return data;
}

void *_watchdog_calloc(size_t num, size_t size, char *_file, size_t _line) {
    __watchdog_initialize();
    const size_t actual_size = num * size;
    void *data = __watchdog_allocate(actual_size, true, _file, _line);
    __watchdog_log_call(CALL_CALLOC, _file, _line, actual_size, data);
    return data;
}

void *_watchdog_realloc(void *ptr, size_t size, char *_file, size_t _line) {
    __watchdog_initialize();
    void *data = __watchdog_reallocate(ptr, size, _file, _line);
    __watchdog_log_call(CALL_REALLOC, _file, _line, size, ptr, data);
    return data;
}

void _watchdog_free(void *ptr, char *_file, size_t _line) {
    __watchdog_initialize();
    const size_t bytes_freed = __watchdog_free(ptr, _file, _line);
    __watchdog_log_call(CALL_FREE, _file, _line, bytes_freed, ptr);
}

void _watchdog_exit(int status, char *_file, size_t _line) {
    __watchdog_initialize();
    __watchdog_log_call(CALL_EXIT, _file, _line, status);
    exit(status);
}

void _watchdog_abort(char *_file, size_t _line) {
    __watchdog_initialize();
    __watchdog_log_call(CALL_ABORT, _file, _line);
    abort();
}

/*
 * Internal functions definitions
 */
static void __watchdog_initialize(void) {
    if (__initialized) {
        return;
    }
    if (strcmp(WATCHDOG_OUTPUT, "<stdout>") == 0) {
        __stream = stdout;
    } else if (strcmp(WATCHDOG_OUTPUT, "<stderr>") == 0) {
        __stream = stderr;
    } else if (NULL == (__stream = fopen(WATCHDOG_OUTPUT, "w"))) {
        fprintf(stderr, "Watchdog: '%s' %s\n", WATCHDOG_OUTPUT, strerror(errno));
        abort();
    }
    __info_list = xor_list_new();
    atexit(__watchdog_terminate);
    atexit(__watchdog_report);
    atexit(__watchdog_collect);
    __initialized = true;
    fprintf(__stream, "[WATCHDOG] INFO: Watchdog Initialized\n");
}

static void __watchdog_terminate(void) {
    assert(__initialized);
    info_t *current_info = NULL;
    trace_t *current_trace = NULL;
    for (size_t i = xor_list_size(__info_list); i > 0; i--) {
        current_info = xor_list_pop_back(__info_list);
        for (size_t x = xor_list_size(current_info->trace_list); x > 0; x--) {
            current_trace = xor_list_pop_back(current_info->trace_list);
            free(current_trace);
        }
        xor_list_delete(current_info->trace_list);
        free(current_info);
    }
    xor_list_delete(__info_list);
    fprintf(__stream, "[WATCHDOG] INFO: Watchdog Terminated\n");
    if ((stdout != __stream) && (stderr != __stream)) {
        fclose(__stream);
    }
}

static void __watchdog_report(void) {
    assert(__initialized);
#if WATCHDOG_REPORT != 0
    info_t *current_info = NULL;
    trace_t *current_trace = NULL;
    fprintf(__stream, "[WATCHDOG] INFO: Report\n");
    const size_t outer_size = xor_list_size(__info_list);
    for (size_t i = 0; i < outer_size; i++) {
        current_info = xor_list_get(__info_list, i);
        fprintf(__stream, "[WATCHDOG] %-8s address %p:\n", "", current_info->address);
        const size_t inner_size = xor_list_size(current_info->trace_list);
        for (size_t x = 0; x < inner_size - 1; x++) {
            current_trace = xor_list_get(current_info->trace_list, x);
            fprintf(__stream, "[WATCHDOG] %-16s %-7s at %65s:%04zu | %2zu bytes were in use\n", "",
                    __watchdog_call_to_string(current_trace->func), current_trace->file, current_trace->line,
                    current_trace->size);
        }
        current_trace = xor_list_back(current_info->trace_list);
        fprintf(__stream, "[WATCHDOG] %-16s %-7s at %65s:%04zu | %2zu bytes currently allocated\n", "",
                __watchdog_call_to_string(current_trace->func), current_trace->file, current_trace->line,
                current_trace->size);
    }
    fprintf(__stream, "[WATCHDOG] %-5s %zu allocations, %zu frees\n", "", __allocations_counter, __frees_counter);
    fprintf(__stream, "[WATCHDOG] %-5s %zu bytes allocated, %zu bytes freed (whereof %zu bytes collected on exit)\n",
            "",
            __bytes_allocated, __bytes_freed, __bytes_collected);
#else
    UNUSED(__bytes_collected);
#endif
}

static void __watchdog_collect(void) {
    assert(__initialized);
#if WATCHDOG_GC != 0
    info_t *current_info = NULL;
    trace_t *current_trace = NULL;
    fprintf(__stream, "[WATCHDOG] WARN: Garbage Collector\n");
    const size_t info_list_size = xor_list_size(__info_list);
    for (size_t i = 0; i < info_list_size; i++) {
        current_info = xor_list_get(__info_list, i);
        current_trace = ((trace_t *) xor_list_back(current_info->trace_list));
        if (current_info->allocated) {
            fprintf(__stream, "[WATCHDOG] %-8s address %p:\n", "", current_info->address);
            fprintf(__stream, "[WATCHDOG] %-16s %-7s at %65s:%04zu | %2zu bytes still allocated\n", "",
                    __watchdog_call_to_string(current_trace->func), current_trace->file, current_trace->line,
                    current_trace->size);
            __watchdog_free(current_info->address, "<garbage collector>", 0);
            __bytes_collected += current_trace->size;
        }
    }
    fprintf(__stream, "[WATCHDOG] %-5s %zu bytes collected\n", "", __bytes_collected);
#else
    UNUSED(__bytes_collected);
#endif
}

static char *__watchdog_call_to_string(const call_t call) {
    switch (call) {
        case CALL_MALLOC:
            return "malloc";
        case CALL_CALLOC:
            return "calloc";
        case CALL_REALLOC:
            return "realloc";
        case CALL_FREE:
            return "free";
        case CALL_EXIT:
            return "exit";
        case CALL_ABORT:
            return "abort";
        default:
            abort();
            return "";
    }
}

static void __watchdog_log_call(const call_t call, const char *const file, const size_t line, ...) {
    assert(__initialized);
#if WATCHDOG_VERBOSE != 0
    va_list args;
    va_start(args, line);
    const char *const caller = __watchdog_call_to_string(call);
    switch (call) {
        case CALL_MALLOC: {
            const size_t size = va_arg(args, size_t);
            const void *const data = va_arg(args, void *);
            if (data) {
                fprintf(__stream, "[WATCHDOG] INFO: %-7s at %s:%04zu\n", caller, file, line);
                fprintf(__stream, "[WATCHDOG] %-5s %zu bytes allocated to address %p\n", "", size, data);
            } else {
                fprintf(__stream, "[WATCHDOG] ERROR: %-7s at %s:%04zu\n", caller, file, line);
                fprintf(__stream, "[WATCHDOG] %-5s failed to allocate %zu bytes\n", "", size);
            }
        }
            break;
        case CALL_CALLOC: {
            const size_t size = va_arg(args, size_t);
            const void *const data = va_arg(args, void *);
            if (data) {
                fprintf(__stream, "[WATCHDOG] INFO: %-7s at %s:%04zu\n", caller, file, line);
                fprintf(__stream, "[WATCHDOG] %-5s %zu bytes allocated to address %p\n", "", size, data);
            } else {
                fprintf(__stream, "[WATCHDOG] ERROR: %-7s at %s:%04zu\n", caller, file, line);
                fprintf(__stream, "[WATCHDOG] %-5s failed to allocate %zu bytes\n", "", size);
            }
        }
            break;
        case CALL_REALLOC: {
            const size_t size = va_arg(args, size_t);
            const void *const ptr = va_arg(args, void *);
            const void *const data = va_arg(args, void *);
            if (data) {
                fprintf(__stream, "[WATCHDOG] INFO: %-7s at %s:%04zu\n", caller, file, line);
                fprintf(__stream, "[WATCHDOG] %-5s %zu bytes reallocated from address %p to address %p\n", "", size,
                        ptr, data);
            } else {
                fprintf(__stream, "[WATCHDOG] ERROR: %-7s at %s:%04zu\n", caller, file, line);
                fprintf(__stream, "[WATCHDOG] %-5s failed to reallocate %zu bytes\n", "", size);
            }
        }
            break;
        case CALL_FREE: {
            const size_t size = va_arg(args, size_t);
            const void *const data = va_arg(args, void *);
            fprintf(__stream, "[WATCHDOG] INFO: %-7s at %s:%04zu\n", caller, file, line);
            fprintf(__stream, "[WATCHDOG] %-5s %zu bytes freed from address %p\n", "", size, data);
        }
            break;
        case CALL_EXIT: {
            const int exit_code = va_arg(args, int);
            fprintf(__stream, "[WATCHDOG] WARN: %-7s at %s:%04zu\n", caller, file, line);
            fprintf(__stream, "[WATCHDOG] %-5s exit code: %d\n", "", exit_code);
        }
            break;
        case CALL_ABORT: {
            fprintf(__stream, "[WATCHDOG] WARN: %-7s at %s:%04zu\n", caller, file, line);
        }
            break;
        default:
            abort();
            break;
    }
    va_end(args);
#else
    UNUSED(call);
    UNUSED(file);
    UNUSED(line);
#endif
}

static void *__watchdog_allocate(const size_t size, const bool clear, const char *const file, const size_t line) {
    chunk_t *chunk = clear ? calloc(1, sizeof(chunk_t) + size) : malloc(sizeof(chunk_t) + size);
    if (NULL == chunk) {
        return NULL;
    }
    info_t *info = malloc(sizeof(info_t));
    assert(NULL != info);
    info->trace_list = xor_list_new();
    info->address = (chunk + 1);
    info->allocated = true;
    trace_t *trace = malloc(sizeof(trace_t));
    assert(NULL != trace);
    trace->func = clear ? CALL_CALLOC : CALL_MALLOC;
    trace->file = (char *) file;
    trace->line = line;
    trace->size = size;
    xor_list_push_back(info->trace_list, trace);
    xor_list_push_back(__info_list, info);
    chunk->info = info;
    __bytes_allocated += size;
    __allocations_counter += 1;
    return chunk + 1;
}

static void *__watchdog_reallocate(void *ptr, const size_t size, const char *const file, const size_t line) {
    chunk_t *chunk = (chunk_t *) ptr - 1;
    info_t *info = chunk->info;
    const size_t old_size = ((trace_t *) xor_list_back(info->trace_list))->size;
    if (NULL == (chunk = realloc(chunk, sizeof(chunk_t) + size))) {
        return NULL;
    }
    chunk->info = info;
    info->address = (chunk + 1);
    info->allocated = true;
    trace_t *trace = malloc(sizeof(trace_t));
    assert(NULL != trace);
    trace->func = CALL_REALLOC;
    trace->file = (char *) file;
    trace->line = line;
    trace->size = size;
    xor_list_push_back(info->trace_list, trace);
    __bytes_allocated += size;
    __bytes_freed += old_size;
    return chunk + 1;
}

static size_t __watchdog_free(void *ptr, const char *const file, const size_t line) {
    chunk_t *chunk = (chunk_t *) ptr - 1;
    info_t *info = chunk->info;
    const size_t bytes_freed = ((trace_t *) xor_list_back(info->trace_list))->size;
    trace_t *trace = malloc(sizeof(trace_t));
    assert(NULL != trace);
    trace->func = CALL_FREE;
    trace->file = (char *) file;
    trace->line = line;
    trace->size = 0;
    info->allocated = false;
    xor_list_push_back(info->trace_list, trace);
    free(chunk);
    __frees_counter += 1;
    __bytes_freed += bytes_freed;
    return bytes_freed;
}
