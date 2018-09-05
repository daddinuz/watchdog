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

#include <stdlib.h>

#if !(defined(__GNUC__) || defined(__clang__))
__attribute__(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define WATCHDOG_VERSION_MAJOR          1
#define WATCHDOG_VERSION_MINOR          0
#define WATCHDOG_VERSION_PATCH          0
#define WATCHDOG_VERSION_SUFFIX         ""
#define WATCHDOG_VERSION_IS_RELEASE     0
#define WATCHDOG_VERSION_HEX            0x010000

#if (defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L) || (defined(__cplusplus) && __cplusplus >= 201103L)
#   define WATCHDOG_HAS_C11_SUPPORT 1
#else
#   define WATCHDOG_HAS_C11_SUPPORT 0
#endif

#if WATCHDOG_HAS_C11_SUPPORT

/**
 * Same as aligned_alloc from <stdlib.h> since C11
 *
 * @attention this function must be treated as opaque therefore should not be called directly, use the macro below instead.
 */
extern void *__Watchdog_aligned_alloc(const char *file, const char *func, int line, size_t alignment, size_t size)
__attribute__((__warn_unused_result__, __nonnull__(1)));

#   define Watchdog_aligned_alloc(alignment, size) \
        __Watchdog_aligned_alloc((__FILE__), (__func__), (__LINE__), (alignment), (size))

#endif

/**
 * Same as malloc from <stdlib.h>
 *
 * @attention this function must be treated as opaque therefore should not be called directly, use the macro below instead.
 */
extern void *__Watchdog_malloc(const char *file, const char *func, int line, size_t size)
__attribute__((__warn_unused_result__, __nonnull__(1)));

#define Watchdog_malloc(size) \
    __Watchdog_malloc((__FILE__), (__func__), (__LINE__), (size))

/**
 * Same as calloc from <stdlib.h>
 *
 * @attention this function must be treated as opaque therefore should not be called directly, use the macro below instead.
 */
extern void *__Watchdog_calloc(const char *file, const char *func, int line, size_t numberOfMembers, size_t memberSize)
__attribute__((__warn_unused_result__, __nonnull__(1)));

#define Watchdog_calloc(numberOfMembers, memberSize) \
    __Watchdog_calloc((__FILE__), (__func__), (__LINE__), (numberOfMembers), (memberSize))

/**
 * Same as realloc from <stdlib.h>
 *
 * @attention this function must be treated as opaque therefore should not be called directly, use the macro below instead.
 */
extern void *__Watchdog_realloc(const char *file, const char *func, int line, void *memory, size_t newSize)
__attribute__((__warn_unused_result__, __nonnull__(1)));

#define Watchdog_realloc(memory, newSize) \
    __Watchdog_realloc((__FILE__), (__func__), (__LINE__), (memory), (newSize))

/**
 * Same as free from <stdlib.h>
 *
 * @attention this function must be treated as opaque therefore should not be called directly, use the macro below instead.
 */
extern void __Watchdog_free(const char *file, const char *func, int line, void *memory)
__attribute__((__nonnull__(1)));

#define Watchdog_free(memory) \
    __Watchdog_free((__FILE__), (__func__), (__LINE__), (memory))

/*
 * Macros
 */
#if WATCHDOG_FORCE_OVERRIDE || !defined(NDEBUG)
#   if WATCHDOG_HAS_C11_SUPPORT
#       undef aligned_alloc
#       define aligned_alloc(alignment, size)   Watchdog_aligned_alloc((alignment), (size))
#   endif
#   undef malloc
#   define malloc(size)                         Watchdog_malloc((size))
#   undef calloc
#   define calloc(numberOfMembers, memberSize)  Watchdog_calloc((numberOfMembers), (memberSize))
#   undef realloc
#   define realloc(memory, newSize)             Watchdog_realloc((memory), (newSize))
#   undef free
#   define free(memory)                         Watchdog_free((memory))
#endif

#ifdef __cplusplus
}
#endif
