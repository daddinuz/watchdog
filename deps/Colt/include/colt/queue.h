/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   November 20, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stddef.h>
#include "common.h"

#ifndef COLT_QUEUE_H
#define COLT_QUEUE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Queue_t Queue_t;

extern Queue_t *queue_new(ContainerType_t type);
extern Queue_t *queue_copy(const Queue_t *const self);

extern void queue_clear(Queue_t *const self);
extern void queue_delete_(Queue_t **ref);
#define queue_delete(q) queue_delete_(&(q))

extern void queue_push(Queue_t *const self, void *const data);
extern void queue_extend(Queue_t *const self, const Queue_t *const q);

extern void *queue_pop(Queue_t *const self);

extern void *queue_get(const Queue_t *const self, size_t index);
extern void queue_set(Queue_t *const self, size_t index, void *const data);

extern size_t queue_size(const Queue_t *const self);
extern void *queue_front(const Queue_t *const self);
extern void *queue_back(const Queue_t *const self);

#ifdef __cplusplus
}
#endif

#endif /* COLT_QUEUE_H */
