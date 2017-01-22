/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   December 07, 2016
 *  email:  daddinuz@gmail.com
 */

#ifndef COLT_PAIR_H
#define COLT_PAIR_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Pair_t {
    void *first;
    void *second;
} Pair_t;

extern Pair_t pair_make(void *const first, void *const second);

extern Pair_t *pair_new(void *const first, void *const second);
extern void pair_delete_(Pair_t **ref);
#define pair_delete(p)  pair_delete_(&(p))

#ifdef __cplusplus
}
#endif

#endif /* COLT_PAIR_H */
