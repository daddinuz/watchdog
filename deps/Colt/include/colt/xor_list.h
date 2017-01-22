/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   January 13, 2017
 *  email:  daddinuz@gmail.com
 */

#include <stddef.h>
#include "common.h"

#ifndef COLT_XOR_LIST_H
#define COLT_XOR_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct XorList_t XorList_t;

extern XorList_t *xor_list_new(void);
extern XorList_t *xor_list_copy(const XorList_t *const self);

extern void xor_list_clear(XorList_t *const self);
extern void xor_list_delete_(XorList_t **ref);
#define xor_list_delete(l) xor_list_delete_(&(l))

extern void xor_list_push_back(XorList_t *const self, void *const data);
extern void xor_list_push_front(XorList_t *const self, void *const data);
extern void xor_list_insert(XorList_t *const self, size_t index, void *const data);
extern void xor_list_extend(XorList_t *const self, const XorList_t *const l);

extern void *xor_list_pop_back(XorList_t *const self);
extern void *xor_list_pop_front(XorList_t *const self);
extern void *xor_list_remove(XorList_t *const self, size_t index);

extern void *xor_list_get(const XorList_t *const self, size_t index);
extern void xor_list_set(XorList_t *const self, size_t index, void *const data);

extern size_t xor_list_size(const XorList_t *const self);
extern void *xor_list_front(const XorList_t *const self);
extern void *xor_list_back(const XorList_t *const self);

extern void xor_list_reverse(XorList_t *const self);
extern void xor_list_sort(XorList_t *const self, Comparator_t comparator);

#ifdef __cplusplus
}
#endif

#endif /* COLT_xor_list_H */
