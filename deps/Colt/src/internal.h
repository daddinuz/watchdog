/*
 *  C Header File
 *
 *  Author: Davide Di Carlo
 *  Date:   December 07, 2016
 *  email:  daddinuz@gmail.com
 */

#include <stddef.h>
#include <stdbool.h>

#ifndef COLT_INTERNAL_H
#define COLT_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

extern void colt_swap(void **a, void **b);

extern void colt_die_(const char *error, const char *file, size_t line, const char *function);
#define colt_die(e)         colt_die_((e), __FILE__, __LINE__, __func__)

extern void colt_ensure_(bool assertion, const char *error, const char *file, size_t line, const char *function);
#define colt_ensure(a, e)   colt_ensure_((a), (e), __FILE__, __LINE__, __func__)

extern void colt_assert_(bool assertion, const char *error, const char *file, size_t line, const char *function);
#define colt_assert(a, e)   colt_assert_((a), (e), __FILE__, __LINE__, __func__)

#define COLT_ERR_SELF_IS_NULL       "self must not be NULL"
#define COLT_ERR_REF_IS_NULL        "ref must not be NULL"
#define COLT_ERR_STAR_REF_IS_NULL   "*ref must not be NULL"
#define COLT_ERR_COMPARATOR_IS_NULL "comparator must not be NULL"
#define COLT_ERR_FUNCTION_IS_NULL   "function must not be NULL"
#define COLT_ERR_BASE_CONTAINER     "invalid base container"
#define COLT_ERR_OUT_OF_RANGE       "out of range"
#define COLT_ERR_KEY_NOT_FOUND      "key not found"

#ifdef __cplusplus
}
#endif

#endif /* COLT_INTERNAL_H */
