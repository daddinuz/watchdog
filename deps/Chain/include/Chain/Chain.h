/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   January 28th 2017
 *  email:  daddinuz@gmail.com
 */

#include <stdbool.h>

#ifndef __CHAIN_H__
#define __CHAIN_H__

#define CHAIN_VERSION "0.1.3"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Chain_t Chain_t;

extern Chain_t *chain_new(void);
extern void chain_clear(Chain_t *const self);
extern void chain_delete_(Chain_t **ref);
#define chain_delete(c) chain_delete_(&(c))

extern void chain_push(Chain_t *const self, void *const data);
extern bool chain_pop(Chain_t *const self, void **out);

extern bool chain_front(const Chain_t *const self, void **out);
extern bool chain_back(const Chain_t *const self, void **out);

extern bool chain_empty(const Chain_t *const self);

typedef enum ChainBound_t {
    CHAIN_BEGIN,
    CHAIN_END
} ChainBound_t;

typedef struct ChainIterator_t ChainIterator_t;

extern ChainIterator_t *chain_iterator_new(Chain_t **ref, ChainBound_t bound);
extern void chain_iterator_rewind(ChainIterator_t *self, ChainBound_t bound);
extern void chain_iterator_delete_(ChainIterator_t **ref);
#define chain_iterator_delete(i) chain_iterator_delete_(&(i))

extern bool chain_iterator_next(ChainIterator_t *const self, void **out);
extern bool chain_iterator_prev(ChainIterator_t *const self, void **out);

#ifdef __cplusplus
}
#endif

#endif /* __CHAIN_H__ */
