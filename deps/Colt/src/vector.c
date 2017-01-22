/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   November 18, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stdlib.h>
#include <string.h>
#include "internal.h"
#include "allocator.h"
#include "colt/vector.h"


static size_t __max(size_t a, size_t b) {
    return a > b ? a : b;
}

static size_t __round_up(size_t n, size_t f) {
    return (n / f + 1) * f;
}

struct Vector_t {
    void **raw;
    size_t size;
    size_t capacity;
};

static Vector_t *__vector_initialize(size_t capacity) {
    colt_assert(capacity > 0, "capacity must be greater than 0");
    Vector_t *v = COLT_CALLOC(1, sizeof(Vector_t));
    v->raw = COLT_CALLOC(capacity, sizeof(void *));
    v->capacity = capacity;
    return v;
}

Vector_t *vector_new(void) {
    return __vector_initialize(_COLT_VECTOR_CHUNK_SIZE);
}

Vector_t *vector_copy(const Vector_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    Vector_t *copy = __vector_initialize(__round_up(self->size, _COLT_VECTOR_CHUNK_SIZE));
    memcpy(copy->raw, self->raw, sizeof(void *) * self->size);
    copy->size = self->size;
    return copy;
}

void vector_clear(Vector_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    free(self->raw);
    self->raw = COLT_CALLOC(_COLT_VECTOR_CHUNK_SIZE, sizeof(void *));
    self->capacity = _COLT_VECTOR_CHUNK_SIZE;
    self->size = 0;
}

void vector_delete_(Vector_t **ref) {
    colt_assert(NULL != ref, COLT_ERR_REF_IS_NULL);
    colt_assert(NULL != *ref, COLT_ERR_STAR_REF_IS_NULL);
    free((*ref)->raw);
    free(*ref);
    *ref = NULL;
}

void vector_push_back(Vector_t *const self, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    vector_insert(self, self->size, data);
}

void vector_push_front(Vector_t *const self, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    vector_insert(self, 0, data);
}

void vector_insert(Vector_t *const self, size_t index, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index <= self->size, COLT_ERR_OUT_OF_RANGE);
    vector_reserve(self, self->size + 1);
    size_t shift = (self->size - index) * sizeof(void *);
    memmove(&(self->raw[index + 1]), &(self->raw[index]), shift);
    self->raw[index] = data;
    self->size += 1;
}

void vector_extend(Vector_t *const self, const Vector_t *const v) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(NULL != v, "v must not be NULL");
    vector_reserve(self, self->size + v->size);
    memcpy(self->raw + self->size, v->raw, sizeof(void *) * v->size);
    self->size += v->size;
}

void *vector_pop_back(Vector_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return vector_remove(self, self->size - 1);
}

void *vector_pop_front(Vector_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return vector_remove(self, 0);
}

void *vector_remove(Vector_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0 && index < self->size, COLT_ERR_OUT_OF_RANGE);
    void *const data = self->raw[index];
    size_t delta = (self->size - 1 - index) * sizeof(void *);
    memmove(&(self->raw[index]), &(self->raw[index + 1]), delta);
    self->size -= 1;
    return data;
}

void *vector_get(const Vector_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < self->size, COLT_ERR_OUT_OF_RANGE);
    return self->raw[index];
}

void vector_set(Vector_t *const self, size_t index, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < self->size, COLT_ERR_OUT_OF_RANGE);
    self->raw[index] = data;
}

size_t vector_size(const Vector_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    return self->size;
}

void *vector_front(const Vector_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return self->raw[0];
}

void *vector_back(const Vector_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return self->raw[self->size - 1];
}

void vector_reverse(Vector_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    for (size_t i = 0; i < self->size / 2; i++) {
        colt_swap(&self->raw[i], &self->raw[self->size - 1 - i]);
    }
}

/*
 * ===> Quick Sort <==========================================
 *
 */
static size_t
__vector_quick_sort_partition(Vector_t *const self, Comparator_t comparator, size_t left, size_t pivot, size_t right) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(NULL != comparator, COLT_ERR_COMPARATOR_IS_NULL);
    size_t index = left;
    void *pivot_value = self->raw[pivot];
    colt_swap(&self->raw[pivot], &self->raw[right]);
    for (size_t i = left; i < right; i++) {
        if (0 > comparator(self->raw[i], pivot_value)) {
            colt_swap(&self->raw[i], &self->raw[index]);
            ++index;
        }
    }
    colt_swap(&self->raw[index], &self->raw[right]);
    return index;
}

static void __vector_quick_sort(Vector_t *const self, Comparator_t comparator, size_t left, size_t right) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(NULL != comparator, COLT_ERR_COMPARATOR_IS_NULL);
    size_t pivot;
    if (left >= right) {
        return;
    }
    pivot = (left + right) / 2;
    pivot = __vector_quick_sort_partition(self, comparator, left, pivot, right);
    __vector_quick_sort(self, comparator, left, ((pivot > 1) ? pivot - 1 : 0));
    __vector_quick_sort(self, comparator, pivot + 1, right);
}

void vector_sort(Vector_t *const self, Comparator_t comparator) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(NULL != comparator, COLT_ERR_COMPARATOR_IS_NULL);
    __vector_quick_sort(self, comparator, 0, self->size ? self->size - 1 : 0);
}

/*
 * vector specific
 */
Vector_t *vector_initialize(size_t capacity) {
    return __vector_initialize(capacity);
}

size_t vector_capacity(const Vector_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    return self->capacity;
}

void vector_shrink(Vector_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    if (self->size == self->capacity) {
        return;
    }
    size_t new_capacity = __max(self->size, _COLT_VECTOR_CHUNK_SIZE);
    void *raw = COLT_CALLOC(new_capacity, sizeof(void *));
    memcpy(raw, self->raw, sizeof(void *) * self->size);
    free(self->raw);
    self->raw = raw;
    self->capacity = new_capacity;
}

void vector_reserve(Vector_t *const self, size_t requested) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    if (requested <= self->capacity) {
        return;
    }
    size_t new_capacity = __round_up(requested, _COLT_VECTOR_CHUNK_SIZE);
    void *raw = COLT_CALLOC(new_capacity, sizeof(void *));
    memcpy(raw, self->raw, sizeof(void *) * self->size);
    free(self->raw);
    self->raw = raw;
    self->capacity = new_capacity;
}

void **vector_raw(const Vector_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    return self->raw;
}
