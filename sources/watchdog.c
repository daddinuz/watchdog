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
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || (defined(__cplusplus) && __cplusplus >= 201103L)
#undef aligned_alloc
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
static void
Watchdog_report(const char *call, const char *file, const char *func, int line,
                const void *relocated, const void *address, size_t size);


#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || (defined(__cplusplus) && __cplusplus >= 201103L)

void *__Watchdog_aligned_alloc(const char *file, const char *func, int line, size_t alignment, size_t size) {
    assert(file);
    assert(line > 0);
    void *address = aligned_alloc(alignment, size);
    Watchdog_report("aligned_alloc", file, func, line, NULL, address, size);
    return address;
}

#endif


void *__Watchdog_malloc(const char *file, const char *func, int line, const size_t size) {
    assert(file);
    assert(line > 0);
    void *address = malloc(size);
    Watchdog_report("malloc", file, func, line, NULL, address, size);
    return address;
}

void *
__Watchdog_calloc(const char *file, const char *func, int line, const size_t numberOfMembers, const size_t memberSize) {
    assert(file);
    assert(line > 0);
    void *address = calloc(numberOfMembers, memberSize);
    Watchdog_report("calloc", file, func, line, NULL, address, numberOfMembers * memberSize);
    return address;
}

void *__Watchdog_realloc(const char *file, const char *func, int line, void *const memory, const size_t newSize) {
    assert(file);
    assert(line > 0);
    void *address = realloc(memory, newSize);
    Watchdog_report("realloc", file, func, line, memory, address, newSize);
    return address;
}

void __Watchdog_free(const char *file, const char *func, int line, void *const memory) {
    assert(file);
    assert(line > 0);
    free(memory);
    Watchdog_report("free", file, func, line, NULL, memory, 0);
}

/*
 *
 */
static void Watchdog_onExit(void) {
    if (gStream) {
        fclose(gStream);
    }
}

void Watchdog_report(const char *const call, const char *file, const char *func, int line,
                     const void *const relocated, const void *const address, const size_t size) {
    assert(call);
    assert(file);
    assert(line > 0);
    assert(address);

    if (!gStream) {
        char buffer[64] = "";
        snprintf(buffer, 63, "watchdog-%lu.jsonl", time(NULL));
        gStream = fopen(buffer, "a");
        if (!gStream) {
            Panic_terminate("Unable to open file: %s", buffer);
        }
        atexit(Watchdog_onExit);
    }

    const long PID = Process_getCurrentId(), parentPID = Process_getParentId(), timestamp = time(NULL);
    if (relocated) {
        fprintf(gStream,
                "{\"PID\": %ld, \"parentPID\": %ld, \"call\": \"%s\", \"file\": \"%s\", \"func\": \"%s\", \"line\": %d, \"address\": {\"from\": \"%p\", \"to\": \"%p\"}, \"size\": %zu, \"timestamp\": %lu}\n",
                PID, parentPID, call, file, func, line, relocated, address, size, timestamp);
    } else {
        fprintf(gStream,
                "{\"PID\": %ld, \"parentPID\": %ld, \"call\": \"%s\", \"file\": \"%s\", \"func\": \"%s\", \"line\": %d, \"address\": \"%p\", \"size\": %zu, \"timestamp\": %lu}\n",
                PID, parentPID, call, file, func, line, address, size, timestamp);
    }
}
