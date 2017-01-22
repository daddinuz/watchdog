/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   December 07, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stdlib.h>
#include <stdio.h>
#include "internal.h"

#ifndef NDEBUG
#define NDEBUG 1
#endif

void colt_swap(void **a, void **b) {
    void *tmp = *a;
    *a = *b;
    *b = tmp;
}

void colt_die_(const char *error, const char *file, size_t line, const char *function) {
    fprintf(stderr, "Traceback:\n  File: \"%s\", line %zu, in <%s>\nError: %s\n", file, line, function, error);
    abort();
}

void colt_ensure_(bool assertion, const char *error, const char *file, size_t line, const char *function) {
    if (!assertion) {
        colt_die_(error, file, line, function);
    }
}

void colt_assert_(bool assertion, const char *error, const char *file, size_t line, const char *function) {
    if (NDEBUG) {
        colt_ensure_(assertion, error, file, line, function);
    }
}
