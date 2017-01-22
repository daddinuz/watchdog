/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   December 03, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stddef.h>
#include "common.h"

#ifndef COLT_STACK_H
#define COLT_STACK_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Stack_t Stack_t;

extern Stack_t *stack_new(ContainerType_t type);
extern Stack_t *stack_copy(const Stack_t *const self);

extern void stack_clear(Stack_t *const self);
extern void stack_delete_(Stack_t **ref);
#define stack_delete(s) stack_delete_(&(s))

extern void stack_push(Stack_t *const self, void *const data);
extern void stack_extend(Stack_t *const self, const Stack_t *const s);

extern void *stack_pop(Stack_t *const self);

extern void *stack_get(const Stack_t *const self, size_t index);
extern void stack_set(Stack_t *const self, size_t index, void *const data);

extern size_t stack_size(const Stack_t *const self);
extern void *stack_top(const Stack_t *const self);

#ifdef __cplusplus
}
#endif

#endif /* COLT_STACK_H */
