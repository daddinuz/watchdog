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

extern void *_watchdog_malloc(size_t size, char *_file, size_t _line);
#define malloc(size)            _watchdog_malloc(size, __FILE__, __LINE__)

extern void *_watchdog_calloc(size_t num, size_t size, char *_file, size_t _line);
#define calloc(num, size)       _watchdog_calloc(num, size, __FILE__, __LINE__)

extern void *_watchdog_realloc(void *ptr, size_t size, char *_file, size_t _line);
#define realloc(ptr, size)      _watchdog_realloc(ptr, size, __FILE__, __LINE__)

extern void _watchdog_free(void *ptr, char *_file, size_t _line);
#define free(ptr)               _watchdog_free(ptr, __FILE__, __LINE__)

extern void _watchdog_exit(int status, char *_file, size_t _line);
#define exit(status)            _watchdog_exit(status, __FILE__, __LINE__)

extern void _watchdog_abort(char *_file, size_t _line);
#define abort()                 _watchdog_abort(__FILE__, __LINE__)

#endif /** __WATCHDOG_H__ **/
