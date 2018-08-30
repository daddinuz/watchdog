/*
Author: daddinuz
email:  daddinuz@gmail.com

Copyright (c) 2018 Davide Di Carlo

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#if !(defined(__GNUC__) || defined(__clang__))
__attribute__(...)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define ERROR_VERSION_MAJOR         0
#define ERROR_VERSION_MINOR         3
#define ERROR_VERSION_PATCH         0
#define ERROR_VERSION_SUFFIX        ""
#define ERROR_VERSION_IS_RELEASE    0
#define ERROR_VERSION_HEX           0x000300

/**
 * Represents errors that may occur at runtime.
 * Every error instance must be a tack allocated singleton.
 * The lifetime of every error is the whole program duration, in order to check if two errors are equal a simple
 * comparison between pointers can be done.
 *
 * @attention this struct must be treated as opaque therefore its members must not be accessed directly.
 */
typedef struct __Error {
    const char *const __message;
} const *Error;

/**
 * An helper macro used for type hinting, useful when writing interfaces.
 * By convention the annotations are the errors that may be returned.
 */
#define ErrorOf(...) \
    Error

/**
 * Helper macro to create new errors.
 *
 * @code
 * Error CustomError = Error_new("Custom error explanation");
 * @endcode
 */
#define Error_new(message) \
     ((Error) &((const struct __Error) {.__message=(message)}))

/**
 * Gets the error message explanation.
 *
 * @attention self must not be `NULL`.
 */
extern const char *Error_explain(Error self)
__attribute__((__warn_unused_result__, __nonnull__));

/**
 * Built-in errors
 */
extern Error Ok;                // Notifies a successful execution.
extern Error DomainError;       // Indicates that a function has been passed illegal or inappropriate arguments
extern Error IllegalState;      // A function has been invoked at an illegal or inappropriate time
extern Error LookupError;       // A key or index used on a mapping or sequence is invalid
extern Error MathError;         // Arithmetic errors e.g. zero division
extern Error MemoryError;       // Memory related error, e.g. overlapping memory. Note: should not be used to notify OOM
extern Error OutOfMemory;       // The app ran out of memory
extern Error SystemError;       // System-related errors e.g. file not found
extern Error StopIteration;     // Indicates that the end of a sequence has been reached

#ifdef __cplusplus
}
#endif
