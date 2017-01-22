/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   November 20, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "internal.h"
#include "allocator.h"


void *colt_malloc_(size_t size, const char *file, size_t line) {
    void *memory = malloc(size);
    colt_ensure_(NULL != memory, strerror(errno), file, line, __func__);
    return memory;
}

void *colt_calloc_(size_t nmemb, size_t size, const char *file, size_t line) {
    void *memory = calloc(nmemb, size);
    colt_ensure_(NULL != memory, strerror(errno), file, line, __func__);
    return memory;
}

void *colt_realloc_(void *ptr, size_t size, const char *file, size_t line) {
    void *memory = realloc(ptr, size);
    colt_ensure_(NULL != memory, strerror(errno), file, line, __func__);
    return memory;
}
