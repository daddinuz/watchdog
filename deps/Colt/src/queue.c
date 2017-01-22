/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   November 20, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stdlib.h>
#include "internal.h"
#include "allocator.h"
#include "colt/list.h"
#include "colt/xor_list.h"
#include "colt/forward_list.h"
#include "colt/vector.h"
#include "colt/queue.h"


struct Queue_t {
    void *container;
    ContainerType_t type;
};

Queue_t *queue_new(ContainerType_t type) {
    colt_assert(LIST_CONTAINER == type || XOR_LIST_CONTAINER == type || FORWARD_LIST_CONTAINER == type ||
                VECTOR_CONTAINER == type, COLT_ERR_BASE_CONTAINER);
    Queue_t *queue = COLT_CALLOC(1, sizeof(Queue_t));
    switch (type) {
        case LIST_CONTAINER:
            queue->container = list_new();
            break;
        case XOR_LIST_CONTAINER:
            queue->container = xor_list_new();
            break;
        case FORWARD_LIST_CONTAINER:
            queue->container = forward_list_new();
            break;
        case VECTOR_CONTAINER:
            queue->container = vector_new();
            break;
        default:
            colt_die("Invalid underlying container"); /* LCOV_EXCL_LINE */
    }
    queue->type = type;
    return queue;
}

Queue_t *queue_copy(const Queue_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    Queue_t *queue = COLT_CALLOC(1, sizeof(Queue_t));
    switch (self->type) {
        case LIST_CONTAINER:
            queue->container = list_copy(self->container);
            break;
        case XOR_LIST_CONTAINER:
            queue->container = xor_list_copy(self->container);
            break;
        case FORWARD_LIST_CONTAINER:
            queue->container = forward_list_copy(self->container);
            break;
        case VECTOR_CONTAINER:
            queue->container = vector_copy(self->container);
            break;
        default:
            colt_die("Invalid underlying container"); /* LCOV_EXCL_LINE */
    }
    queue->type = self->type;
    return queue;
}

void queue_clear(Queue_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    switch (self->type) {
        case LIST_CONTAINER:
            list_clear(self->container);
            break;
        case XOR_LIST_CONTAINER:
            xor_list_clear(self->container);
            break;
        case FORWARD_LIST_CONTAINER:
            forward_list_clear(self->container);
            break;
        case VECTOR_CONTAINER:
            vector_clear(self->container);
            break;
        default:
            colt_die("Invalid underlying container"); /* LCOV_EXCL_LINE */
    }
}

void queue_delete_(Queue_t **ref) {
    colt_assert(NULL != ref, COLT_ERR_REF_IS_NULL);
    colt_assert(NULL != *ref, COLT_ERR_STAR_REF_IS_NULL);
    switch ((*ref)->type) {
        case LIST_CONTAINER:
            list_delete_(&*((List_t **) &(*ref)->container));
            break;
        case XOR_LIST_CONTAINER:
            xor_list_delete_(&*((XorList_t **) &(*ref)->container));
            break;
        case FORWARD_LIST_CONTAINER:
            forward_list_delete_(&*((ForwardList_t **) &(*ref)->container));
            break;
        case VECTOR_CONTAINER:
            vector_delete_(&*((Vector_t **) &(*ref)->container));
            break;
        default:
            colt_die("Invalid underlying container"); /* LCOV_EXCL_LINE */
    }
    free(*ref);
    *ref = NULL;
}

void queue_push(Queue_t *const self, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    switch (self->type) {
        case LIST_CONTAINER:
            list_push_back(self->container, data);
            break;
        case XOR_LIST_CONTAINER:
            xor_list_push_back(self->container, data);
            break;
        case FORWARD_LIST_CONTAINER:
            forward_list_push_back(self->container, data);
            break;
        case VECTOR_CONTAINER:
            vector_push_back(self->container, data);
            break;
        default:
            colt_die("Invalid underlying container"); /* LCOV_EXCL_LINE */
    }
}

void queue_extend(Queue_t *const self, const Queue_t *const q) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(NULL != q, "q must not be NULL");
    switch (self->type) {
        case LIST_CONTAINER:
            list_extend(self->container, q->container);
            break;
        case XOR_LIST_CONTAINER:
            xor_list_extend(self->container, q->container);
            break;
        case FORWARD_LIST_CONTAINER:
            forward_list_extend(self->container, q->container);
            break;
        case VECTOR_CONTAINER:
            vector_extend(self->container, q->container);
            break;
        default:
            colt_die("Invalid underlying container"); /* LCOV_EXCL_LINE */
    }
}

void *queue_pop(Queue_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(queue_size(self) > 0, COLT_ERR_OUT_OF_RANGE);
    switch (self->type) {
        case LIST_CONTAINER:
            return list_pop_front(self->container);
        case XOR_LIST_CONTAINER:
            return xor_list_pop_front(self->container);
        case FORWARD_LIST_CONTAINER:
            return forward_list_pop_front(self->container);
        case VECTOR_CONTAINER:
            return vector_pop_front(self->container);
        default: /* LCOV_EXCL_START */
            colt_die("Invalid underlying container");
            return NULL; /* gcc is stupid: this line is needed to quite -Werror=return-type */
            /* LCOV_EXCL_STOP */
    }
}

void *queue_get(const Queue_t *const self, size_t index) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < queue_size(self), COLT_ERR_OUT_OF_RANGE);
    switch (self->type) {
        case LIST_CONTAINER:
            return list_get(self->container, index);
        case XOR_LIST_CONTAINER:
            return xor_list_get(self->container, index);
        case FORWARD_LIST_CONTAINER:
            return forward_list_get(self->container, index);
        case VECTOR_CONTAINER:
            return vector_get(self->container, index);
        default: /* LCOV_EXCL_START */
            colt_die("Invalid underlying container");
            return NULL; /* gcc is stupid: this line is needed to quite -Werror=return-type */
            /* LCOV_EXCL_STOP */
    }
}

void queue_set(Queue_t *const self, size_t index, void *const data) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(index < queue_size(self), COLT_ERR_OUT_OF_RANGE);
    switch (self->type) {
        case LIST_CONTAINER:
            list_set(self->container, index, data);
            break;
        case XOR_LIST_CONTAINER:
            xor_list_set(self->container, index, data);
            break;
        case FORWARD_LIST_CONTAINER:
            forward_list_set(self->container, index, data);
            break;
        case VECTOR_CONTAINER:
            vector_set(self->container, index, data);
            break;
        default:
            colt_die("Invalid underlying container"); /* LCOV_EXCL_LINE */
    }
}

size_t queue_size(const Queue_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    switch (self->type) {
        case LIST_CONTAINER:
            return list_size(self->container);
        case XOR_LIST_CONTAINER:
            return xor_list_size(self->container);
        case FORWARD_LIST_CONTAINER:
            return forward_list_size(self->container);
        case VECTOR_CONTAINER:
            return vector_size(self->container);
        default: /* LCOV_EXCL_START */
            colt_die("Invalid underlying container");
            return 0; /* gcc is stupid: this line is needed to quite -Werror=return-type */
            /* LCOV_EXCL_STOP */
    }
}

void *queue_front(const Queue_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(queue_size(self) > 0, COLT_ERR_OUT_OF_RANGE);
    switch (self->type) {
        case LIST_CONTAINER:
            return list_front(self->container);
        case XOR_LIST_CONTAINER:
            return xor_list_front(self->container);
        case FORWARD_LIST_CONTAINER:
            return forward_list_front(self->container);
        case VECTOR_CONTAINER:
            return vector_front(self->container);
        default: /* LCOV_EXCL_START */
            colt_die("Invalid underlying container");
            return 0; /* gcc is stupid: this line is needed to quite -Werror=return-type */
            /* LCOV_EXCL_STOP */
    }
}

void *queue_back(const Queue_t *const self) {
    colt_assert(NULL != self, COLT_ERR_SELF_IS_NULL);
    colt_assert(queue_size(self) > 0, COLT_ERR_OUT_OF_RANGE);
    switch (self->type) {
        case LIST_CONTAINER:
            return list_back(self->container);
        case XOR_LIST_CONTAINER:
            return xor_list_back(self->container);
        case FORWARD_LIST_CONTAINER:
            return forward_list_back(self->container);
        case VECTOR_CONTAINER:
            return vector_back(self->container);
        default: /* LCOV_EXCL_START */
            colt_die("Invalid underlying container");
            return 0; /* gcc is stupid: this line is needed to quite -Werror=return-type */
            /* LCOV_EXCL_STOP */
    }
}
