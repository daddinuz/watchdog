/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   November 19, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stddef.h>
#include "common.h"

#ifndef COLT_LIST_H
#define COLT_LIST_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct List_t List_t;

extern List_t *list_new(void);
extern List_t *list_copy(const List_t *const self);

extern void list_clear(List_t *const self);
extern void list_delete_(List_t **ref);
#define list_delete(l) list_delete_(&(l))

extern void list_push_back(List_t *const self, void *const data);
extern void list_push_front(List_t *const self, void *const data);
extern void list_insert(List_t *const self, size_t index, void *const data);
extern void list_extend(List_t *const self, const List_t *const l);

extern void *list_pop_back(List_t *const self);
extern void *list_pop_front(List_t *const self);
extern void *list_remove(List_t *const self, size_t index);

extern void *list_get(const List_t *const self, size_t index);
extern void list_set(List_t *const self, size_t index, void *const data);

extern size_t list_size(const List_t *const self);
extern void *list_front(const List_t *const self);
extern void *list_back(const List_t *const self);

extern void list_reverse(List_t *const self);
extern void list_sort(List_t *const self, Comparator_t comparator);

#ifdef __cplusplus
}
#endif

#endif /* COLT_LIST_H */
