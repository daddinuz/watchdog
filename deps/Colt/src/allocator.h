/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   November 20, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stddef.h>

#ifndef COLT_ALLOCATOR_H
#define COLT_ALLOCATOR_H

#ifdef __cplusplus
extern "C" {
#endif

extern void *colt_malloc_(size_t size, const char *file, size_t line);
#define colt_malloc(size)           colt_malloc_((size), __FILE__, __LINE__)

extern void *colt_calloc_(size_t nmemb, size_t size, const char *file, size_t line);
#define colt_calloc(nmemb, size)    colt_calloc_((nmemb), (size), __FILE__, __LINE__)

extern void *colt_realloc_(void *ptr, size_t size, const char *file, size_t line);
#define colt_realloc(ptr, size)    colt_realloc_((ptr), (size), __FILE__, __LINE__)

#define COLT_MALLOC(size)           colt_malloc(size)
#define COLT_CALLOC(nmemb, size)    colt_calloc(nmemb, size)
#define COLT_REALLOC(ptr, size)     colt_realloc(ptr, size)

#ifdef __cplusplus
}
#endif

#endif /* COLT_ALLOCATOR_H */
