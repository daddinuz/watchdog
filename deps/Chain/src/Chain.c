/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   January 28th 2017
 *  email:  daddinuz@gmail.com
 */

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include "Chain/Chain.h"


/**
 * Node internal structure
 */
typedef struct Node_t {
    void *data;
    struct Node_t *link;
} Node_t;

/*
 * Node private functions declaration
 */
static Node_t *__node_new(void *data);
static Node_t *__node_xor(const Node_t *const p1, const Node_t *const p2);
static void __node_delete(Node_t **ref, void **out);

/**
 * Chain internal structure
 */
struct Chain_t {
    Node_t *front;
    Node_t *back;
};

/*
 * Chain public functions definition
 */
Chain_t *chain_new(void) {
    Chain_t *chain = calloc(1, sizeof(Chain_t));
    assert(NULL != chain);
    return chain;
}

void chain_clear(Chain_t *const self) {
    assert(NULL != self);
    while (NULL != self->back) {
        assert(NULL != self->front);
        chain_pop(self, NULL);
    }
    assert(NULL == self->back);
    assert(NULL == self->front);
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
        assert(NULL == self->front);
        self->front = self->back = new_node;
    } else {
        assert(NULL != self->front);
        new_node->link = self->back;
        self->back->link = __node_xor(self->back->link, new_node);
        self->back = new_node;
    }
}

bool chain_pop(Chain_t *const self, void **out) {
    assert(NULL != self);
    if (NULL == self->back) {
        assert(NULL == self->front);
        return false;
    }
    assert(NULL != self->front);
    Node_t *left = __node_xor(self->back->link, NULL), *node = self->back, *right = NULL;
    if (left == NULL) {
        self->front = right;
    } else {
        left->link = __node_xor(__node_xor(left->link, node), right);
    }
    self->back = left;
    __node_delete(&node, out);
    return true;
}

bool chain_front(const Chain_t *const self, void **out) {
    assert(NULL != self);
    assert(NULL != out);
    if (NULL == self->front) {
        assert(NULL == self->back);
        return false;
    }
    assert(NULL != self->back);
    *out = self->front->data;
    return true;
}

bool chain_back(const Chain_t *const self, void **out) {
    assert(NULL != self);
    assert(NULL != out);
    if (NULL == self->back) {
        assert(NULL == self->front);
        return false;
    }
    assert(NULL != self->front);
    *out = self->back->data;
    return true;
}

bool chain_empty(const Chain_t *const self) {
    assert(NULL != self);
    if (NULL == self->back) {
        assert(NULL == self->front);
        return false;
    }
    assert(NULL != self->front);
    return true;
}

/**
 * Chain Iterator internal structure
 */
struct ChainIterator_t {
    Chain_t **chain;
    Node_t *left, *node, *right;
};

/*
 * Chain Iterator public functions definition
 */
ChainIterator_t *chain_iterator_new(Chain_t **ref, ChainBound_t bound) {
    assert(NULL != ref);
    assert(NULL != *ref);
    ChainIterator_t *iterator = calloc(1, sizeof(ChainIterator_t));
    assert(NULL != iterator);
    iterator->chain = ref;
    if (CHAIN_FRONT == bound) {
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

void chain_iterator_reset(ChainIterator_t *self, ChainBound_t bound) {
    assert(NULL != self);
    assert(NULL != self->chain);
    assert(NULL != *self->chain);
    if (CHAIN_FRONT == bound) {
        self->left = NULL;
        self->node = (*self->chain)->front;
        self->right = __node_xor(self->node->link, self->left);
    } else {
        self->right = NULL;
        self->node = (*self->chain)->back;
        self->left = __node_xor(self->node->link, self->right);
    }
}

void chain_iterator_delete_(ChainIterator_t **ref) {
    assert(NULL != ref);
    assert(NULL != *ref);
    free(*ref);
    *ref = NULL;
}

bool chain_iterator_right(ChainIterator_t *const self, void **out) {
    assert(NULL != self);
    if (NULL != *self->chain && NULL != self->node) {
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

bool chain_iterator_left(ChainIterator_t *const self, void **out) {
    assert(NULL != self);
    if (NULL != *self->chain && NULL != self->node) {
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
static Node_t *__node_new(void *data) {
    Node_t *node = calloc(1, sizeof(Node_t));
    assert(node);
    node->data = data;
    return node;
}

static Node_t *__node_xor(const Node_t *const p1, const Node_t *const p2) {
    return ((Node_t *) ((intptr_t) p1 ^ (intptr_t) p2));
}

static void __node_delete(Node_t **ref, void **out) {
    assert(NULL != ref);
    assert(NULL != *ref);
    if (out) {
        *out = (*ref)->data;
    }
    free(*ref);
    *ref = NULL;
}
