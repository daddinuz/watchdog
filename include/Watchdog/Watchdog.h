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
extern void watchdog_initialize(const char *path);
extern void watchdog_terminate(void);
extern void watchdog_dump(void);
extern void watchdog_collect(void); /* WARN: It's highly recommended to call this function only before program exits */

/*
 * Protected functions / use the macros instead
 */
extern void *watchdog_malloc(size_t size, char *_file, size_t _line);
extern void *watchdog_calloc(size_t num, size_t size, char *_file, size_t _line);
extern void *watchdog_realloc(void *ptr, size_t size, char *_file, size_t _line);
extern void watchdog_free(void *ptr, char *_file, size_t _line);
extern void watchdog_exit(int status, char *_file, size_t _line);
extern void watchdog_abort(char *_file, size_t _line);

/*
 * Override stdlib.h macros
 */
#define malloc(size)       watchdog_malloc(size, __FILE__, __LINE__)
#define calloc(num, size)  watchdog_calloc(num, size, __FILE__, __LINE__)
#define realloc(ptr, size) watchdog_realloc(ptr, size, __FILE__, __LINE__)
#define free(ptr)          watchdog_free(ptr, __FILE__, __LINE__)
#define exit(status)       watchdog_exit(status, __FILE__, __LINE__)
#define abort()            watchdog_abort(__FILE__, __LINE__)

#endif /** __WATCHDOG_H__ **/
