/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   April 28th 2016
 *  email:  daddinuz@gmail.com
 */

#include <stdio.h>
#include <stddef.h>

#ifndef __WATCHDOG_H__
#define __WATCHDOG_H__

/*
 * Public functions
 */
extern void watchdog_initialize(FILE *stream);
extern void watchdog_terminate(void);

/*
 * Protected functions / use macros instead
 */
extern void _watchdog_dump(char *_file, size_t _line);
#define watchdog_dump()             _watchdog_dump(__FILE__, __LINE__)

/* WARN: It's highly recommended to call this function only before program exits */
extern void _watchdog_collect(char *_file, size_t _line);
#define watchdog_collect()          _watchdog_collect(__FILE__, __LINE__)

extern void *_watchdog_malloc(size_t size, char *_file, size_t _line);
#define malloc(size)                _watchdog_malloc(size, __FILE__, __LINE__)

extern void *_watchdog_calloc(size_t num, size_t size, char *_file, size_t _line);
#define calloc(num, size)           _watchdog_calloc(num, size, __FILE__, __LINE__)

extern void *_watchdog_realloc(void *ptr, size_t size, char *_file, size_t _line);
#define realloc(ptr, size)          _watchdog_realloc(ptr, size, __FILE__, __LINE__)

extern void _watchdog_free(void *ptr, char *_file, size_t _line);
#define free(ptr)                   _watchdog_free(ptr, __FILE__, __LINE__)

extern void _watchdog_exit(int status, char *_file, size_t _line);
#define exit(status)                _watchdog_exit(status, __FILE__, __LINE__)

extern void _watchdog_abort(char *_file, size_t _line);
#define abort()                     _watchdog_abort(__FILE__, __LINE__)

#endif /** __WATCHDOG_H__ **/
