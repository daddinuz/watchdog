/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   November 19, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stddef.h>
#include <stdbool.h>

#ifndef COLT_COMMON_H
#define COLT_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Underlying containers for adaptors
 */
typedef enum ContainerType_t {
    LIST_CONTAINER = 0,
    XOR_LIST_CONTAINER,
    FORWARD_LIST_CONTAINER,
    VECTOR_CONTAINER
} ContainerType_t;

/*
 *  < 0 : The element pointed to by a goes before the element pointed to by b
 *    0 : The element pointed to by a is equivalent to the element pointed to by b
 *  > 0 : The element pointed to by a goes after the element pointed to by b
 */
typedef int (*const Comparator_t)(const void *const a, const void *const b);

#ifdef __cplusplus
}
#endif

#endif /* COLT_COMMON_H */
