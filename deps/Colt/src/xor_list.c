/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   January 13, 2017
 *  email:  daddinuz@gmail.com
 */

#include <stdlib.h>
#include <stdint.h>
#include "internal.h"
#include "allocator.h"
#include "colt/pair.h"
#include "colt/xor_list.h"


typedef struct Node_t {
    void *data;
    struct Node_t *link;
} Node_t;

struct XorList_t {
    size_t size;
    Node_t *front;
    Node_t *back;
};

static Node_t *__xor_list_node_xor(const Node_t *const p1, const Node_t *const p2) {
    return ((Node_t *) ((intptr_t) p1 ^ (intptr_t) p2));
}

static Node_t *__xor_list_node_new(void *data) {
    Node_t *node = COLT_CALLOC(1, sizeof(Node_t));
    node->data = data;
    return node;
}

static void *__xor_list_node_delete(Node_t **ref) {
    colt_assert(NULL != ref, COLT_ERR_REF_IS_NULL);
    colt_assert(NULL != *ref, COLT_ERR_STAR_REF_IS_NULL);
    void *const data = (*ref)->data;
    free(*ref);
    *ref = NULL;
    return data;
}

static Pair_t __xor_list_fetch(const XorList_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < self->size, COLT_ERR_OUT_OF_RANGE);
    const bool start_front = index < self->size / 2;
    Node_t *tmp = NULL, *predecessor = NULL, *node = NULL;
    if (start_front) {
        node = self->front;
        for (size_t i = 0; i < index; i++) {
            tmp = node;
            node = __xor_list_node_xor(predecessor, node->link);
            predecessor = tmp;
        }
    } else {
        node = self->back;
        for (size_t i = self->size - 1; i > index; i--) {
            tmp = node;
            node = __xor_list_node_xor(predecessor, node->link);
            predecessor = tmp;
        }
    }
    colt_assert(NULL != node, "node should not be NULL");
    return start_front ? pair_make(predecessor, node) : pair_make(__xor_list_node_xor(node->link, predecessor), node);
}

XorList_t *xor_list_new(void) {
    XorList_t *xor_list = COLT_CALLOC(1, sizeof(XorList_t));
    return xor_list;
}

XorList_t *xor_list_copy(const XorList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    XorList_t *copy = xor_list_new();
    Node_t *prev = NULL, *node = self->front, *next = NULL;
    while (NULL != node) {
        xor_list_push_back(copy, node->data);
        next = __xor_list_node_xor(prev, node->link);
        prev = node;
        node = next;
    }
    return copy;
}

void xor_list_clear(XorList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    for (size_t i = self->size; i > 0; i--) {
        xor_list_pop_front(self);
    }
    colt_assert(0 == self->size, "size should be 0");
    colt_assert(NULL == self->back, "back should be NULL");
    colt_assert(NULL == self->front, "front should be NULL");
}

void xor_list_delete_(XorList_t **ref) {
    colt_assert(NULL != ref, COLT_ERR_REF_IS_NULL);
    colt_assert(NULL != *ref, COLT_ERR_STAR_REF_IS_NULL);
    xor_list_clear(*ref);
    free(*ref);
    *ref = NULL;
}

void xor_list_push_back(XorList_t *const self, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    xor_list_insert(self, self->size, data);
}

void xor_list_push_front(XorList_t *const self, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    xor_list_insert(self, 0, data);
}

void xor_list_insert(XorList_t *const self, size_t index, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index <= self->size, COLT_ERR_OUT_OF_RANGE);
    /*
     * A <-> B <-> C
     *   A <-X-> C
     *
     * X = A xor C
     * A = X xor C
     * C = X xor A
     */
    Node_t *new_node = __xor_list_node_new(data);
    if (0 == index) {                   // push front
        if (NULL == self->front) {
            self->front = self->back = new_node;
        } else {
            new_node->link = self->front;
            self->front->link = __xor_list_node_xor(self->front->link, new_node);
            self->front = new_node;
        }
    } else if (index == self->size) {   // push back
        new_node->link = self->back;
        self->back->link = __xor_list_node_xor(self->back->link, new_node);
        self->back = new_node;
    } else {                            // insert in the middle
        Pair_t pair = __xor_list_fetch(self, index);
        Node_t *next_node = pair.second;
        Node_t *prev_node = pair.first;
        prev_node->link = __xor_list_node_xor(__xor_list_node_xor(next_node, prev_node->link), new_node);
        new_node->link = __xor_list_node_xor(prev_node, next_node);
        next_node->link = __xor_list_node_xor(__xor_list_node_xor(prev_node, next_node->link), new_node);
    }
    self->size += 1;
}

void xor_list_extend(XorList_t *const self, const XorList_t *const l) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(NULL != l, "l must not be NULL");
    Node_t *prev = NULL, *node = l->front, *next = NULL;
    while (NULL != node) {
        xor_list_push_back(self, node->data);
        next = __xor_list_node_xor(prev, node->link);
        prev = node;
        node = next;
    }
}

void *xor_list_pop_back(XorList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return xor_list_remove(self, self->size - 1);
}

void *xor_list_pop_front(XorList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return xor_list_remove(self, 0);
}

void *xor_list_remove(XorList_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0 && index < self->size, COLT_ERR_OUT_OF_RANGE);
    Pair_t pair = __xor_list_fetch(self, index);
    Node_t *prev = pair.first, *node = pair.second, *next = __xor_list_node_xor(prev, node->link);
    if (prev == NULL) {
        self->front = next;
    } else {
        prev->link = __xor_list_node_xor(__xor_list_node_xor(prev->link, node), next);
    }
    if (next == NULL) {
        self->back = prev;
    } else {
        next->link = __xor_list_node_xor(__xor_list_node_xor(next->link, node), prev);
    }
    self->size -= 1;
    return __xor_list_node_delete(&node);
}

void *xor_list_get(const XorList_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < self->size, COLT_ERR_OUT_OF_RANGE);
    return ((Node_t *) __xor_list_fetch(self, index).second)->data;
}

void xor_list_set(XorList_t *const self, size_t index, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < self->size, COLT_ERR_OUT_OF_RANGE);
    ((Node_t *) __xor_list_fetch(self, index).second)->data = data;
}

size_t xor_list_size(const XorList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    return self->size;
}

void *xor_list_front(const XorList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return self->front->data;
}

void *xor_list_back(const XorList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return self->back->data;
}

void xor_list_reverse(XorList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    Node_t *tmp = NULL, *very_left = NULL, *left = self->front, *right = self->back, *very_right = NULL;
    for (size_t i = 0; i < self->size / 2; i++) {
        colt_swap(&left->data, &right->data);
        tmp = __xor_list_node_xor(very_left, left->link);
        very_left = left;
        left = tmp;
        tmp = __xor_list_node_xor(right->link, very_right);
        very_right = right;
        right = tmp;
    }
}

/*
 * ===> Selection Sort <==========================================
 *
 */
void xor_list_sort(XorList_t *const self, Comparator_t comparator) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(NULL != comparator, COLT_ERR_COMPARATOR_IS_NULL);
    Node_t *_current = NULL, *_next = NULL;
    Node_t *minimum = NULL, *traverse = NULL, *previous = NULL, *current = self->front, *next = NULL;
    while (current && (next = __xor_list_node_xor(previous, current->link))) {
        minimum = current;
        traverse = next;
        _current = current;
        _next = next;
        while (traverse) {
            /* Find minimum element from array */
            if (0 < comparator(minimum->data, traverse->data)) {
                minimum = traverse;
            }
            traverse = __xor_list_node_xor(_current, _next->link);
            _current = _next;
            _next = traverse;
        }
        /* Put minimum element on head location */
        colt_swap(&current->data, &minimum->data);
        previous = current;
        current = next;
    }
}
