/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   April 28th 2016
 *  email:  daddinuz@gmail.com
 */

#include <stddef.h>

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

#define WATCHDOG_VERSION "0.1.5"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef WATCHDOG_GC         /* Garbage collection on exit (0: off, every non-zero: on) */
#define WATCHDOG_GC         1
#endif

#ifndef WATCHDOG_REPORT     /* Memory usage report on exit (0: off, every non-zero: on) */
#define WATCHDOG_REPORT     1
#endif

#ifndef WATCHDOG_VERBOSE    /* Log every call (0: off, every non-zero: on) */
#define WATCHDOG_VERBOSE    1
#endif

#if WATCHDOG_GC == 0 && WATCHDOG_REPORT == 0 && WATCHDOG_VERBOSE == 0
#error "Watchdog: all features are disabled" /* Do you really need Watchdog? */
#endif

#ifndef WATCHDOG_OUTPUT     /* Where to redirect Watchdog output to ("<stderr>" | "<stdout>" | "/path/to/file.log") */
#define WATCHDOG_OUTPUT     "<stderr>"
#endif

extern void *watchdog_malloc_(size_t size, const char *const __file, const size_t __line);
extern void *watchdog_calloc_(size_t nmemb, size_t size, const char *const __file, const size_t __line);
extern void *watchdog_realloc_(void *ptr, size_t size, const char *const __file, const size_t __line);
extern void watchdog_free_(void *ptr, const char *const __file, const size_t __line);
extern void watchdog_exit_(int status, const char *const __file, const size_t __line);
extern void watchdog_abort_(const char *const __file, const size_t __line);

#define watchdog_malloc(size)           watchdog_malloc_((size), __FILE__, __LINE__)
#define watchdog_calloc(nmemb, size)    watchdog_calloc_((nmemb), (size), __FILE__, __LINE__)
#define watchdog_realloc(ptr, size)     watchdog_realloc_((ptr), (size), __FILE__, __LINE__)
#define watchdog_free(ptr)              watchdog_free_((ptr), __FILE__, __LINE__)
#define watchdog_exit(status)           watchdog_exit_((status), __FILE__, __LINE__)
#define watchdog_abort()                watchdog_abort_(__FILE__, __LINE__)

#ifdef WATCHDOG_WRAP_STDLIB
#undef malloc
#undef calloc
#undef realloc
#undef free
#undef exit
#undef abort
#define malloc(size)                    watchdog_malloc(size)
#define calloc(nmemb, size)             watchdog_calloc(nmemb, size)
#define realloc(ptr, size)              watchdog_realloc(ptr, size)
#define free(ptr)                       watchdog_free(ptr)
#define exit(status)                    watchdog_exit(status)
#define abort()                         watchdog_abort()
#endif

#ifdef __cplusplus
}
#endif

#endif /** __WATCHDOG_H__ **/
