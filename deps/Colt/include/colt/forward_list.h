/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   November 27, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stddef.h>
#include "common.h"

#ifndef COLT_FORWARD_LIST_H
#define COLT_FORWARD_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ForwardList_t ForwardList_t;

extern ForwardList_t *forward_list_new(void);
extern ForwardList_t *forward_list_copy(const ForwardList_t *const self);

extern void forward_list_clear(ForwardList_t *const self);
extern void forward_list_delete_(ForwardList_t **ref);
#define forward_list_delete(l) forward_list_delete_(&(l))

extern void forward_list_push_back(ForwardList_t *const self, void *const data);
extern void forward_list_push_front(ForwardList_t *const self, void *const data);
extern void forward_list_insert(ForwardList_t *const self, size_t index, void *const data);
extern void forward_list_extend(ForwardList_t *const self, const ForwardList_t *const l);

extern void *forward_list_pop_back(ForwardList_t *const self);
extern void *forward_list_pop_front(ForwardList_t *const self);
extern void *forward_list_remove(ForwardList_t *const self, size_t index);

extern void *forward_list_get(const ForwardList_t *const self, size_t index);
extern void forward_list_set(ForwardList_t *const self, size_t index, void *const data);

extern size_t forward_list_size(const ForwardList_t *const self);
extern void *forward_list_front(const ForwardList_t *const self);
extern void *forward_list_back(const ForwardList_t *const self);

extern void forward_list_reverse(ForwardList_t *const self);
extern void forward_list_sort(ForwardList_t *const self, Comparator_t comparator);

#ifdef __cplusplus
}
#endif

#endif /* COLT_FORWARD_LIST_H */
