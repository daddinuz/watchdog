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

#pragma once

#include <stdarg.h>
#include <stdbool.h>

#if !(defined(__GNUC__) || defined(__clang__))
__attribute__(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define PANIC_VERSION_MAJOR         1
#define PANIC_VERSION_MINOR         0
#define PANIC_VERSION_PATCH         0
#define PANIC_VERSION_SUFFIX        ""
#define PANIC_VERSION_IS_RELEASE    0
#define PANIC_VERSION_HEX           0x010000

/**
 * Type signature of the callback to be executed before terminating.
 */
typedef void (*Panic_Callback)(void);

/**
 * Registers a callback to execute before terminating.
 *
 * @param callback The callback to be executed, if NULL nothing will be executed.
 * @return The previous registered callback if any else NULL.
 */
extern Panic_Callback Panic_registerCallback(Panic_Callback callback);

/**
 * Reports the error and terminates execution.
 * Takes printf-like arguments.
 */
#define Panic_terminate(...) \
    __Panic_terminate((__FILE__), (__LINE__), __VA_ARGS__)

/**
 * Terminates execution if condition is `true`.
 */
#define Panic_when(condition) \
    __Panic_when((__FILE__), (__LINE__), (#condition), (condition))

/**
 * Terminates execution if condition is `false`.
 */
#define Panic_unless(condition) \
    __Panic_unless((__FILE__), (__LINE__), (#condition), (condition))

/**
 * @attention this function must be treated as opaque therefore should not be called directly.
 */
extern void __Panic_terminate(const char *file, int line, const char *format, ...)
__attribute__((__noinline__, __noreturn__, __nonnull__(1, 3), __format__(__printf__, 3, 4)));

/**
 * @attention this function must be treated as opaque therefore should not be called directly.
 */
extern void __Panic_vterminate(const char *file, int line, const char *format, va_list args)
__attribute__((__noinline__, __noreturn__, __nonnull__(1, 3), __format__(__printf__, 3, 0)));

/**
 * @attention this function must be treated as opaque therefore should not be called directly.
 */
extern void __Panic_when(const char *file, int line, const char *message, bool condition)
__attribute__((__noinline__, __nonnull__(1, 3)));

/**
 * @attention this function must be treated as opaque therefore should not be called directly.
 */
extern void __Panic_unless(const char *file, int line, const char *message, bool condition)
__attribute__((__noinline__, __nonnull__(1, 3)));

#ifdef __cplusplus
}
#endif
