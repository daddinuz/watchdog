/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   November 18, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stddef.h>
#include "common.h"

#ifndef COLT_VECTOR_H
#define COLT_VECTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#define _COLT_VECTOR_CHUNK_SIZE ((sizeof(long double) / sizeof(long long)) * sizeof(int))

typedef struct Vector_t Vector_t;

extern Vector_t *vector_new(void);
extern Vector_t *vector_copy(const Vector_t *const self);

extern void vector_clear(Vector_t *const self);
extern void vector_delete_(Vector_t **ref);
#define vector_delete(v) vector_delete_(&(v))

extern void vector_push_back(Vector_t *const self, void *const data);
extern void vector_push_front(Vector_t *const self, void *const data);
extern void vector_insert(Vector_t *const self, size_t index, void *const data);
extern void vector_extend(Vector_t *const self, const Vector_t *const v);

extern void *vector_pop_back(Vector_t *const self);
extern void *vector_pop_front(Vector_t *const self);
extern void *vector_remove(Vector_t *const self, size_t index);

extern void *vector_get(const Vector_t *const self, size_t index);
extern void vector_set(Vector_t *const self, size_t index, void *const data);

extern size_t vector_size(const Vector_t *const self);
extern void *vector_front(const Vector_t *const self);
extern void *vector_back(const Vector_t *const self);

extern void vector_reverse(Vector_t *const self);
extern void vector_sort(Vector_t *const self, Comparator_t comparator);

/*
 * vector specific
 */
extern Vector_t *vector_initialize(size_t capacity);
extern size_t vector_capacity(const Vector_t *const self);
extern void vector_shrink(Vector_t *const self);
extern void vector_reserve(Vector_t *const self, size_t size);
extern void **vector_raw(const Vector_t *const self); /* direct modifications of the raw data may invalidate the vector. */

#ifdef __cplusplus
}
#endif

#endif /* COLT_VECTOR_H **/
