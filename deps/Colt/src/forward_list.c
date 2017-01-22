/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   November 27, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stdlib.h>
#include <colt/pair.h>
#include "internal.h"
#include "allocator.h"
#include "colt/forward_list.h"


typedef struct Node_t {
    void *data;
    struct Node_t *next;
} Node_t;

struct ForwardList_t {
    size_t size;
    Node_t *front;
    Node_t *back;
};

static Node_t *__forward_list_node_new(void *data) {
    Node_t *node = COLT_CALLOC(1, sizeof(Node_t));
    node->data = data;
    return node;
}

static void *__forward_list_node_delete(Node_t **ref) {
    colt_assert(NULL != ref, COLT_ERR_REF_IS_NULL);
    colt_assert(NULL != *ref, COLT_ERR_STAR_REF_IS_NULL);
    void *const data = (*ref)->data;
    free(*ref);
    *ref = NULL;
    return data;
}

static Pair_t __forward_list_fetch(const ForwardList_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < self->size, COLT_ERR_OUT_OF_RANGE);
    Pair_t pair = {.first=NULL, .second=self->front};
    for (size_t i = 0; i < index; i += 1) {
        pair.first = pair.second;
        pair.second = ((Node_t *) pair.second)->next;
    }
    colt_assert(NULL != pair.second, "node should not be NULL");
    return pair;
}

ForwardList_t *forward_list_new(void) {
    ForwardList_t *forward_list = COLT_CALLOC(1, sizeof(ForwardList_t));
    forward_list->size = 0;
    return forward_list;
}

ForwardList_t *forward_list_copy(const ForwardList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    ForwardList_t *copy = forward_list_new();
    for (Node_t *node = self->front; NULL != node; node = node->next) {
        forward_list_push_back(copy, node->data);
    }
    return copy;
}

void forward_list_clear(ForwardList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    Pair_t pair = {.first=NULL, .second=self->front};
    while (NULL != pair.second) {
        pair.first = pair.second;
        pair.second = ((Node_t *) pair.second)->next;
        __forward_list_node_delete((Node_t **) &pair.first);
    }
    self->size = 0;
    self->back = NULL;
    self->front = NULL;
}

void forward_list_delete_(ForwardList_t **ref) {
    colt_assert(NULL != ref, COLT_ERR_REF_IS_NULL);
    colt_assert(NULL != *ref, COLT_ERR_STAR_REF_IS_NULL);
    forward_list_clear(*ref);
    free(*ref);
    *ref = NULL;
}

void forward_list_push_back(ForwardList_t *const self, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    forward_list_insert(self, self->size, data);
}

void forward_list_push_front(ForwardList_t *const self, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    forward_list_insert(self, 0, data);
}

void forward_list_insert(ForwardList_t *const self, size_t index, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index <= self->size, COLT_ERR_OUT_OF_RANGE);
    Node_t *new_node = __forward_list_node_new(data);
    if (0 == index) {
        if (NULL == self->front) {
            self->front = self->back = new_node;
        } else {
            Node_t *tmp = self->front;
            self->front = new_node;
            self->front->next = tmp;
        }
    } else if (index == self->size) {
        self->back->next = new_node;
        self->back = new_node;
    } else {
        Pair_t pair = __forward_list_fetch(self, index);
        new_node->data = data;
        new_node->next = pair.second;
        ((Node_t *) pair.first)->next = new_node;
    }
    self->size += 1;
}

void forward_list_extend(ForwardList_t *const self, const ForwardList_t *const l) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(NULL != l, "l must not be NULL");
    for (Node_t *node = l->front; NULL != node; node = node->next) {
        forward_list_push_back(self, node->data);
    }
}

void *forward_list_pop_back(ForwardList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return forward_list_remove(self, self->size - 1);
}

void *forward_list_pop_front(ForwardList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return forward_list_remove(self, 0);
}

void *forward_list_remove(ForwardList_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0 && index < self->size, COLT_ERR_OUT_OF_RANGE);
    Pair_t pair = __forward_list_fetch(self, index);
    if (NULL == pair.first) {
        self->front = ((Node_t *) pair.second)->next;
    } else {
        ((Node_t *) pair.first)->next = ((Node_t *) pair.second)->next;
    }
    if (NULL == ((Node_t *) pair.second)->next) {
        self->back = pair.first;
    }
    self->size -= 1;
    return __forward_list_node_delete((Node_t **) &pair.second);
}

void *forward_list_get(const ForwardList_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < self->size, COLT_ERR_OUT_OF_RANGE);
    return ((Node_t *) __forward_list_fetch(self, index).second)->data;
}

void forward_list_set(ForwardList_t *const self, size_t index, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < self->size, COLT_ERR_OUT_OF_RANGE);
    ((Node_t *) __forward_list_fetch(self, index).second)->data = data;
}

size_t forward_list_size(const ForwardList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    return self->size;
}

void *forward_list_front(const ForwardList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return self->front->data;
}

void *forward_list_back(const ForwardList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return self->back->data;
}

void forward_list_reverse(ForwardList_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    Node_t *prev = NULL, *current = self->front, *next = NULL;
    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    self->back = self->front;
    self->front = prev;
}

/*
 * ===> Selection Sort <==========================================
 *
 */
void forward_list_sort(ForwardList_t *const self, Comparator_t comparator) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(NULL != comparator, COLT_ERR_COMPARATOR_IS_NULL);
    Node_t *traverse = NULL, *minimum = NULL, *head = self->front;
    while (head && head->next) {
        minimum = head;
        traverse = head->next;
        while (traverse) {
            /* Find minimum element from array */
            if (0 < comparator(minimum->data, traverse->data)) {
                minimum = traverse;
            }
            traverse = traverse->next;
        }
        /* Put minimum element on heading location */
        colt_swap(&head->data, &minimum->data);
        head = head->next;
    }
}
