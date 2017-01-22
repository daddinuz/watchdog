/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   November 19, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stdlib.h>
#include "internal.h"
#include "allocator.h"
#include "colt/list.h"


typedef struct Node_t {
    void *data;
    struct Node_t *prev;
    struct Node_t *next;
} Node_t;

struct List_t {
    size_t size;
    Node_t *front;
    Node_t *back;
};

static Node_t *__list_node_new(void *data) {
    Node_t *node = COLT_CALLOC(1, sizeof(Node_t));
    node->data = data;
    return node;
}

static void *__list_node_delete(Node_t **ref) {
    colt_assert(NULL != ref, COLT_ERR_REF_IS_NULL);
    colt_assert(NULL != *ref, COLT_ERR_STAR_REF_IS_NULL);
    void *const data = (*ref)->data;
    free(*ref);
    *ref = NULL;
    return data;
}

static Node_t *__list_fetch(const List_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < self->size, COLT_ERR_OUT_OF_RANGE);
    Node_t *node = NULL;
    if (index < self->size / 2) {
        node = self->front;
        for (size_t i = 0; i < index; i++) {
            node = node->next;
        }
    } else {
        node = self->back;
        for (size_t i = self->size - 1; i > index; i--) {
            node = node->prev;
        }
    }
    colt_assert(NULL != node, "node should not be NULL");
    return node;
}

List_t *list_new(void) {
    List_t *list = COLT_CALLOC(1, sizeof(List_t));
    return list;
}

List_t *list_copy(const List_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    List_t *copy = list_new();
    for (Node_t *node = self->front; NULL != node; node = node->next) {
        list_push_back(copy, node->data);
    }
    return copy;
}

void list_clear(List_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    Node_t *node = self->front;
    Node_t *next = NULL;
    while (NULL != node) {
        next = node->next;
        __list_node_delete(&node);
        node = next;
    }
    self->size = 0;
    self->back = NULL;
    self->front = NULL;
}

void list_delete_(List_t **ref) {
    colt_assert(NULL != ref, COLT_ERR_REF_IS_NULL);
    colt_assert(NULL != *ref, COLT_ERR_STAR_REF_IS_NULL);
    list_clear(*ref);
    free(*ref);
    *ref = NULL;
}

void list_push_back(List_t *const self, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    list_insert(self, self->size, data);
}

void list_push_front(List_t *const self, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    list_insert(self, 0, data);
}

void list_insert(List_t *const self, size_t index, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index <= self->size, COLT_ERR_OUT_OF_RANGE);
    Node_t *new_node = __list_node_new(data);
    if (0 == index) {                   // push front
        if (NULL == self->front) {
            self->front = self->back = new_node;
        } else {
            self->front->prev = new_node;
            new_node->next = self->front;
            self->front = new_node;
        }
    } else if (index == self->size) {   // push back
        new_node->prev = self->back;
        self->back->next = new_node;
        self->back = new_node;
    } else {                            // insert in the middle
        Node_t *next_node = __list_fetch(self, index);
        new_node->next = next_node;
        new_node->prev = next_node->prev;
        next_node->prev->next = new_node;
        next_node->prev = new_node;
    }
    self->size += 1;
}

void list_extend(List_t *const self, const List_t *const l) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(NULL != l, "l must not be NULL");
    for (Node_t *node = l->front; NULL != node; node = node->next) {
        list_push_back(self, node->data);
    }
}

void *list_pop_back(List_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return list_remove(self, self->size - 1);
}

void *list_pop_front(List_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return list_remove(self, 0);
}

void *list_remove(List_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0 && index < self->size, COLT_ERR_OUT_OF_RANGE);
    Node_t *node = __list_fetch(self, index);
    if (node->prev == NULL) {
        self->front = node->next;
    } else {
        node->prev->next = node->next;
    }
    if (node->next == NULL) {
        self->back = node->prev;
    } else {
        node->next->prev = node->prev;
    }
    self->size -= 1;
    return __list_node_delete(&node);
}

void *list_get(const List_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < self->size, COLT_ERR_OUT_OF_RANGE);
    return __list_fetch(self, index)->data;
}

void list_set(List_t *const self, size_t index, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < self->size, COLT_ERR_OUT_OF_RANGE);
    __list_fetch(self, index)->data = data;
}

size_t list_size(const List_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    return self->size;
}

void *list_front(const List_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return self->front->data;
}

void *list_back(const List_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(self->size > 0, COLT_ERR_OUT_OF_RANGE);
    return self->back->data;
}

void list_reverse(List_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    Node_t *left = self->front, *right = self->back;
    for (size_t i = 0; i < self->size / 2; i++) {
        colt_swap(&left->data, &right->data);
        left = left->next;
        right = right->prev;
    }
}

/*
 * ===> Quick Sort <==========================================
 *
 */
static Node_t *__list_quick_sort_partition(Node_t *front, Node_t *back, Comparator_t comparator) {
    void *pivot = back->data;
    Node_t *i = front->prev;
    Node_t *j = front;
    while (j != back) {
        if (0 > comparator(j->data, pivot)) {
            i = (NULL == i) ? front : i->next;
            colt_swap(&i->data, &j->data);
        }
        j = j->next;
    }
    i = (NULL == i) ? front : i->next;
    colt_swap(&i->data, &j->data);
    return i;
}

static void __list_quick_sort(Node_t *front, Node_t *back, Comparator_t comparator) {
    if (NULL != back && front != back && front != back->next) {
        Node_t *pivot = __list_quick_sort_partition(front, back, comparator);
        __list_quick_sort(front, pivot->prev, comparator);
        __list_quick_sort(pivot->next, back, comparator);
    }
}

void list_sort(List_t *const self, Comparator_t comparator) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(NULL != comparator, COLT_ERR_COMPARATOR_IS_NULL);
    __list_quick_sort(self->front, self->back, comparator);
}
