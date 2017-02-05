/*
 * C Source File
 *
 * Author: Davide Di Carlo
 * Date:   February 05, 2017 
 * email:  daddinuz@gmal.com
 */

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include "Alligator/Alligator.h"

/*
 * Private functions declaration
 */
static void __die(const char *msg, const char *file, const size_t line);
static void __ensure(const bool assertion, const char *msg, const char *file, const size_t line);

/*
 * Protected functions definitions
 */
void *alligator_malloc_(size_t size, const char *__file, const size_t __line) {
    void *chunk = malloc(size);
    __ensure(NULL != chunk, strerror(errno), __file, __line);
    return chunk;
}

void *alligator_calloc_(size_t nmemb, size_t size, const char *__file, const size_t __line) {
    void *chunk = calloc(nmemb, size);
    __ensure(NULL != chunk, strerror(errno), __file, __line);
    return chunk;
}

void *alligator_realloc_(void *ptr, size_t size, const char *__file, const size_t __line) {
    void *chunk = realloc(ptr, size);
    __ensure(NULL != chunk, strerror(errno), __file, __line);
    return chunk;
}

void alligator_free_(void *ptr, const char *__file, const size_t __line) {
    /*
     * All standards compliant versions of the C library treat free(NULL) as a no-op,
     * but we want to be sure that NULL is never passed to free.
     */
    __ensure(NULL != ptr, "Freeing a NULL pointer", __file, __line);
    free(ptr);
}

/*
 * Private functions definition
 */
static void __die(const char *msg, const char *file, const size_t line) {
    fprintf(stderr, "At: %s:%lu\nError: %s\n", file, (long unsigned) line, msg);
    abort();
}

static void __ensure(const bool assertion, const char *msg, const char *file, const size_t line) {
    if (!assertion) {
        __die(msg, file, line);
    }
}
