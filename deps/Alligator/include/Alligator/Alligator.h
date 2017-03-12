/*
 * C Header File
 *
 * Author: Davide Di Carlo
 * Date:   February 05, 2017 
 * email:  daddinuz@gmal.com
 */

#include <stddef.h>

#ifndef __ALLIGATOR_H__
#define __ALLIGATOR_H__

#define ALLIGATOR_VERSION "0.1.3"

#ifdef __cplusplus
extern "C" {
#endif

extern void *alligator_malloc_(size_t size, const char *const __file, const size_t __line);
extern void *alligator_calloc_(size_t nmemb, size_t size, const char *const __file, const size_t __line);
extern void *alligator_realloc_(void *ptr, size_t size, const char *const __file, const size_t __line);
extern void alligator_free_(void *ptr, const char *const __file, const size_t __line);

#define alligator_malloc(size)          alligator_malloc_((size), __FILE__, __LINE__)
#define alligator_calloc(nmemb, size)   alligator_calloc_((nmemb), (size), __FILE__, __LINE__)
#define alligator_realloc(ptr, size)    alligator_realloc_((ptr), (size), __FILE__, __LINE__)
#define alligator_free(ptr)             alligator_free_((ptr), __FILE__, __LINE__)

#ifdef ALLIGATOR_WRAP_STDLIB
#define malloc(size)                    alligator_malloc(size)
#define calloc(nmemb, size)             alligator_calloc(nmemb, size)
#define realloc(ptr, size)              alligator_realloc(ptr, size)
#define free(ptr)                       alligator_free(ptr)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __ALLIGATOR_H__ */
