/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   December 07, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stdlib.h>
#include "internal.h"
#include "allocator.h"
#include "colt/pair.h"


Pair_t pair_make(void *const first, void *const second) {
    return (Pair_t) {.first=first, .second=second};
}

Pair_t *pair_new(void *const first, void *const second) {
    Pair_t *pair = COLT_MALLOC(sizeof(Pair_t));
    pair->first = first;
    pair->second = second;
    return pair;
}

void pair_delete_(Pair_t **ref) {
    colt_assert(NULL != ref, COLT_ERR_REF_IS_NULL);
    colt_assert(NULL != *ref, COLT_ERR_STAR_REF_IS_NULL);
    free(*ref);
    *ref = NULL;
}
