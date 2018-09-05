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

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "panic.h"

static Panic_Callback globalCallback = NULL;

static void terminate(const char *file, int line, const char *format, ...)
__attribute__((__noinline__, __noreturn__, __nonnull__(1, 3), __format__(__printf__, 3, 4)));

static void vterminate(const char *file, int line, const char *format, va_list args)
__attribute__((__noinline__, __noreturn__, __nonnull__(1, 3), __format__(__printf__, 3, 0)));

Panic_Callback Panic_registerCallback(const Panic_Callback callback) {
    const Panic_Callback backup = callback;
    globalCallback = callback;
    return backup;
}

void __Panic_terminate(const char *const file, const int line, const char *const format, ...) {
    assert(NULL != file);
    assert(NULL != format);
    va_list args;
    va_start(args, format);
    vterminate(file, line, format, args);
}

void __Panic_vterminate(const char *const file, const int line, const char *const format, va_list args) {
    assert(NULL != file);
    assert(NULL != format);
    vterminate(file, line, format, args);
}

void __Panic_when(const char *const file, const int line, const char *const message, const bool condition) {
    assert(NULL != file);
    assert(NULL != message);
    if (condition) {
        terminate(file, line, "(%s) evaluates to `true`", message);
    }
}

void __Panic_unless(const char *const file, const int line, const char *const message, const bool condition) {
    assert(NULL != file);
    assert(NULL != message);
    if (!condition) {
        terminate(file, line, "(%s) evaluates to `false`", message);
    }
}

/*
 *
 */
#define NEWLINE "\r\n"

static void doTerminate(const char *file, int line, const char *format, va_list args)
__attribute__((__noreturn__, __nonnull__(1, 3), __format__(__printf__, 3, 0)));

static void backtrace(FILE *stream)
__attribute__((__nonnull__));

void terminate(const char *file, int line, const char *format, ...) {
    assert(NULL != file);
    assert(NULL != format);
    va_list args;
    va_start(args, format);
    doTerminate(file, line, format, args);
}

void vterminate(const char *file, int line, const char *format, va_list args) {
    assert(NULL != file);
    assert(NULL != format);
    doTerminate(file, line, format, args);
}

void doTerminate(const char *file, int line, const char *format, va_list args) {
    assert(NULL != file);
    assert(NULL != format);
    fputs(NEWLINE, stderr);
    backtrace(stderr);
    fprintf(stderr, "   At: %s:%d" NEWLINE, file, line);
    if (0 != errno) {
        fprintf(stderr, "Error: (%d) %s" NEWLINE, errno, strerror(errno));
    }
    fputs("Cause: ", stderr);
    vfprintf(stderr, format, args);
    fputs(NEWLINE, stderr);
    va_end(args);
    if (NULL != globalCallback) {
        globalCallback();
    }
    abort();
}

#if !defined(PANIC_UNWIND_SUPPORT) && PANIC_UNWIND_SUPPORT == 0

void backtrace(FILE *const stream) {
    assert(NULL != stream);
    (void) stream;
}

#else

#define UNW_LOCAL_ONLY

#include <libunwind.h>

void backtrace(FILE *const stream) {
    assert(NULL != stream);
    const int previousError = errno;
    size_t size = 0;
    const size_t N_SIZE = 8, M_SIZE = 32;

    char buffer[N_SIZE][M_SIZE + 1];
    unw_cursor_t cursor;
    unw_context_t context;

    memset(&context, 0, sizeof(context));
    memset(&cursor, 0, sizeof(cursor));
    memset(buffer, 0, N_SIZE * M_SIZE);

    unw_getcontext(&context);
    unw_init_local(&cursor, &context);

    // skip: (v)terminate and __Panic_(v)terminate function calls
    for (size_t i = 0; i < 3; i++) {
        if (unw_step(&cursor) <= 0) {
            errno = previousError;  // restore errno
            return;                 // something wrong, exit
        }
    }

    for (size_t i = 0; i < N_SIZE && unw_step(&cursor) > 0; i++) {
        if (unw_get_proc_name(&cursor, buffer[i], M_SIZE, NULL) == 0) {
            size += 1;
            if (strcmp("main", buffer[i]) == 0) {
                break;
            }
        } else {
            break; // something wrong, stop unwinding
        }
    }

    if (0 == size) {
        errno = previousError;  // restore errno
        return;                 // something wrong, exit
    }

    fputs("Traceback (most recent call last):" NEWLINE, stream);
    if (strcmp("main", buffer[size - 1]) != 0) {
        fputs("  [ ]: (...)" NEWLINE, stream);
    }
    for (size_t i = 1; i < size; i++) {
        fprintf(stream, "  [%zu]: (%s)" NEWLINE, i - 1, buffer[size - i]);
    }
    fprintf(stream, "  ->-: (%s) current function" NEWLINE, buffer[0]);
    fputs(NEWLINE, stream);

    errno = previousError;  // restore errno
}

#endif
