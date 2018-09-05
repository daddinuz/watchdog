/*
Author: daddinuz
email:  daddinuz@gmail.com

Copyright (c) 2018 Davide Di Carlo

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
 */

#include "watchdog.h"

/*
 * Un-define overrides over stdlib.h
 */
#if WATCHDOG_HAS_C11_SUPPORT
#   undef aligned_alloc
#endif
#undef malloc
#undef calloc
#undef realloc
#undef free

#include <time.h>
#include <stdio.h>
#include <assert.h>
#include <panic/panic.h>
#include <process/process.h>

/*
 * Global variables
 */
static FILE *gStream = NULL;

/*
 * Watchdog
 */
static void Watchdog_report(const char *call, const char *file, const char *func, int line,
                            const void *relocated, const void *address, size_t size);

#if WATCHDOG_HAS_C11_SUPPORT

void *__Watchdog_aligned_alloc(const char *const file, const char *const func, const int line,
                               const size_t alignment, const size_t size) {
    assert(NULL != file);
    void *address = aligned_alloc(alignment, size);
    Watchdog_report("aligned_alloc", file, func, line, NULL, address, size);
    return address;
}

#endif

void *__Watchdog_malloc(const char *const file, const char *const func, const int line, const size_t size) {
    assert(NULL != file);
    void *address = malloc(size);
    Watchdog_report("malloc", file, func, line, NULL, address, size);
    return address;
}

void *__Watchdog_calloc(const char *const file, const char *const func, const int line,
                        const size_t numberOfMembers, const size_t memberSize) {
    assert(NULL != file);
    void *address = calloc(numberOfMembers, memberSize);
    Watchdog_report("calloc", file, func, line, NULL, address, numberOfMembers * memberSize);
    return address;
}

void *__Watchdog_realloc(const char *const file, const char *const func, const int line,
                         void *const memory, const size_t newSize) {
    assert(NULL != file);
    void *address = realloc(memory, newSize);
    Watchdog_report("realloc", file, func, line, memory, address, newSize);
    return address;
}

void __Watchdog_free(const char *const file, const char *const func, const int line, void *const memory) {
    assert(NULL != file);
    free(memory);
    Watchdog_report("free", file, func, line, NULL, memory, 0);
}

/*
 *
 */
static void Watchdog_onExit(void) {
    if (NULL != gStream) {
        fclose(gStream);
    }
}

void Watchdog_report(const char *const call, const char *const file, const char *const func, const int line,
                     const void *const relocated, const void *const address, const size_t size) {
    assert(NULL != call);
    assert(NULL != file);
    assert(NULL != func);
    assert(NULL != address);

    if (NULL == gStream) {
        char fileName[65] = "";
        snprintf(fileName, 64, ".watchdog-%d-%lu.jsonl", WATCHDOG_VERSION_HEX, time(NULL));
        gStream = fopen(fileName, "w");
        if (NULL == gStream) {
            Panic_terminate("Unable to open file: %s", fileName);
        }
        atexit(Watchdog_onExit);
    }

    const long PID = Process_getCurrentId(), parentPID = Process_getParentId(), timestamp = time(NULL);
    if (NULL != relocated) {
        fprintf(gStream,
                "{\"PID\": %ld, \"parentPID\": %ld, \"call\": \"%s\", \"file\": \"%s\", \"func\": \"%s\", \"line\": %d, \"address\": {\"from\": \"%p\", \"to\": \"%p\"}, \"size\": %zu, \"timestamp\": %lu}\n",
                PID, parentPID, call, file, func, line, relocated, address, size, timestamp);
    } else {
        fprintf(gStream,
                "{\"PID\": %ld, \"parentPID\": %ld, \"call\": \"%s\", \"file\": \"%s\", \"func\": \"%s\", \"line\": %d, \"address\": \"%p\", \"size\": %zu, \"timestamp\": %lu}\n",
                PID, parentPID, call, file, func, line, address, size, timestamp);
    }

    fflush(gStream);
}
