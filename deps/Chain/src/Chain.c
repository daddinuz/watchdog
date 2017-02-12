/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   January 28th 2017
 *  email:  daddinuz@gmail.com
 */

#include <stdint.h>
#include <assert.h>

#define ALLIGATOR_WRAP_STDLIB

#include "Alligator.h"
#include "Chain.h"


/**
 * Node internal structure
 */
typedef struct Node_t {
    void *data;
    struct Node_t *link;
} Node_t;

/**
 * Node private functions declaration
 */
static Node_t *__node_new(void *data);
static Node_t *__node_xor(const Node_t *const p1, const Node_t *const p2);
static void *__node_delete(Node_t **ref);

/**
 * Chain internal structure
 */
struct Chain_t {
    size_t size;
    Node_t *front;
    Node_t *back;
};

/**
 * Chain public functions definition
 */
Chain_t *chain_new(void) {
    Chain_t *chain = calloc(1, sizeof(Chain_t));
    return chain;
}

void chain_clear(Chain_t *const self) {
    assert(NULL != self);
    while (NULL != self->back) {
        chain_pop(self);
    }
    self->size = 0;
}

void chain_delete_(Chain_t **ref) {
    assert(NULL != ref);
    assert(NULL != *ref);
    chain_clear(*ref);
    free(*ref);
    *ref = NULL;
}

void chain_push(Chain_t *const self, void *const data) {
    assert(NULL != self);
    Node_t *new_node = __node_new(data);
    if (NULL == self->back) {
        self->front = self->back = new_node;
    } else {
        new_node->link = self->back;
        self->back->link = __node_xor(self->back->link, new_node);
        self->back = new_node;
    }
    self->size += 1;
}

void *chain_pop(Chain_t *const self) {
    assert(NULL != self);
    assert(NULL != self->back);
    Node_t *left = __node_xor(self->back->link, NULL), *node = self->back, *right = NULL;
    if (left == NULL) {
        self->front = right;
    } else {
        left->link = __node_xor(__node_xor(left->link, node), right);
    }
    self->back = left;
    self->size -= 1;
    return __node_delete(&node);
}

void *chain_front(const Chain_t *const self) {
    assert(NULL != self);
    assert(NULL != self->front);
    return self->front->data;
}

void *chain_back(const Chain_t *const self) {
    assert(NULL != self);
    assert(NULL != self->back);
    return self->back->data;
}

bool chain_empty(const Chain_t *const self) {
    assert(NULL != self);
    return 0 == self->size;
}

size_t chain_size(const Chain_t *const self) {
    assert(NULL != self);
    return self->size;
}

/**
 * Chain Iterator internal structure
 */
struct ChainIterator_t {
    Chain_t **ref;
    Node_t *left, *node, *right;
};

/**
 * Chain Iterator public functions definition
 */
ChainIterator_t *chain_iterator_new(Chain_t **ref, ChainBound_t bound) {
    assert(NULL != ref);
    assert(NULL != *ref);
    ChainIterator_t *iterator = calloc(1, sizeof(ChainIterator_t));
    iterator->ref = ref;
    if (CHAIN_BEGIN == bound) {
        iterator->left = NULL;
        iterator->node = (*ref)->front;
        iterator->right = __node_xor(iterator->node->link, iterator->left);
    } else {
        iterator->right = NULL;
        iterator->node = (*ref)->back;
        iterator->left = __node_xor(iterator->node->link, iterator->right);
    }
    return iterator;
}

void chain_iterator_rewind(ChainIterator_t *self, ChainBound_t bound) {
    assert(NULL != self);
    assert(NULL != self->ref);
    assert(NULL != *self->ref);
    if (CHAIN_BEGIN == bound) {
        self->left = NULL;
        self->node = (*self->ref)->front;
        self->right = __node_xor(self->node->link, self->left);
    } else {
        self->right = NULL;
        self->node = (*self->ref)->back;
        self->left = __node_xor(self->node->link, self->right);
    }
}

void chain_iterator_delete_(ChainIterator_t **ref) {
    assert(NULL != ref);
    assert(NULL != *ref);
    free(*ref);
    *ref = NULL;
}

bool chain_iterator_next(ChainIterator_t *const self, void **out) {
    assert(NULL != self);
    if (NULL != *self->ref && NULL != self->node) {
        if (NULL != out) {
            *out = self->node->data;
        }
        self->left = self->node;
        self->node = self->right;
        self->right = self->node ? __node_xor(self->node->link, self->left) : NULL;
        return true;
    }
    return false;
}

bool chain_iterator_prev(ChainIterator_t *const self, void **out) {
    assert(NULL != self);
    if (NULL != *self->ref && NULL != self->node) {
        if (NULL != out) {
            *out = self->node->data;
        }
        self->right = self->node;
        self->node = self->left;
        self->left = self->node ? __node_xor(self->node->link, self->right) : NULL;
        return true;
    }
    return false;
}

/**
 * Node private functions definition
 */
Node_t *__node_new(void *data) {
    Node_t *node = calloc(1, sizeof(Node_t));
    node->data = data;
    return node;
}

Node_t *__node_xor(const Node_t *const p1, const Node_t *const p2) {
    return ((Node_t *) ((intptr_t) p1 ^ (intptr_t) p2));
}

void *__node_delete(Node_t **ref) {
    assert(NULL != ref);
    assert(NULL != *ref);
    void *const data = (*ref)->data;
    free(*ref);
    *ref = NULL;
    return data;
}
