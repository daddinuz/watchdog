/*
 * Author: daddinuz
 * email:  daddinuz@gmail.com
 *
 * Copyright (c) 2018 Davide Di Carlo
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdlib.h>

#if !(defined(__GNUC__) || defined(__clang__))
#define __attribute__(...)
#endif

#define WATCHDOG_VERSION_MAJOR       0
#define WATCHDOG_VERSION_MINOR       2
#define WATCHDOG_VERSION_PATCH       1
#define WATCHDOG_VERSION_SUFFIX      ""
#define WATCHDOG_VERSION_IS_RELEASE  0
#define WATCHDOG_VERSION_HEX         0x000201

#ifndef WATCHDOG_OUTPUT             /* Where to redirect output ("<stderr>" | "<stdout>" | "<tempfile>" | "/path/to/file.yml") */
#define WATCHDOG_OUTPUT             "<stderr>"
#endif

/**
 * @return The semantic versioning string of the package.
 */
extern const char *Watchdog_version(void)
__attribute__((__warn_unused_result__));

/**
 * Same as malloc from stdlib.h
 *
 * @attention Do not use this function directly use the macro instead.
 */
extern void *__Watchdog_malloc(const char *file, int line, size_t size)
__attribute__((__warn_unused_result__, __nonnull__(1)));

#define Watchdog_malloc(size) \
    __Watchdog_malloc(__FILE__, __LINE__, (size))

/**
 * Same as calloc from stdlib.h
 *
 * @attention Do not use this function directly use the macro instead.
 */
extern void *__Watchdog_calloc(const char *file, int line, size_t numberOfMembers, size_t memberSize)
__attribute__((__warn_unused_result__, __nonnull__(1)));

#define Watchdog_calloc(numberOfMembers, memberSize) \
    __Watchdog_calloc(__FILE__, __LINE__, (numberOfMembers), (memberSize))

/**
 * Same as realloc from stdlib.h
 *
 * @attention Do not use this function directly use the macro instead.
 */
extern void *__Watchdog_realloc(const char *file, int line, void *ptr, size_t newSize)
__attribute__((__warn_unused_result__, __nonnull__(1)));

#define Watchdog_realloc(ptr, newSize) \
    __Watchdog_realloc(__FILE__, __LINE__, (ptr), (newSize))

/**
 * Same as free from stdlib.h
 *
 * @attention Do not use this function directly use the macro instead.
 */
extern void __Watchdog_free(const char *file, int line, void *ptr)
__attribute__((__nonnull__(1)));

#define Watchdog_free(ptr) \
    __Watchdog_free(__FILE__, __LINE__, (ptr))

/**
 * Same as exit from stdlib.h
 *
 * @attention Do not use this function directly use the macro instead.
 */
extern void __Watchdog_exit(const char *file, int line, int status)
__attribute__((__noreturn__, __nonnull__(1)));

#define Watchdog_exit(status) \
    __Watchdog_exit(__FILE__, __LINE__, (status))

/**
 * Same as abort from stdlib.h
 *
 * @attention Do not use this function directly use the macro instead.
 */
extern void __Watchdog_abort(const char *file, int line)
__attribute__((__noreturn__, __nonnull__(1)));

#define Watchdog_abort() \
    __Watchdog_abort(__FILE__, __LINE__)


#if !defined(NDEBUG) || defined(WATCHDOG_DISABLED)

#undef malloc
#define malloc(size) \
    Watchdog_malloc((size))

#undef calloc
#define calloc(numberOfMembers, memberSize) \
    Watchdog_calloc((numberOfMembers), (memberSize))

#undef realloc
#define realloc(ptr, newSize) \
    Watchdog_realloc((ptr), (newSize))

#undef free
#define free(ptr) \
    Watchdog_free((ptr))

#undef exit
#define exit(status) \
    Watchdog_exit((status))

#undef abort
#define abort() \
    Watchdog_abort()

#endif

#ifdef __cplusplus
}
#endif
