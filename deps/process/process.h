/*
 * Author: daddinuz
 * email:  daddinuz@gmail.com
 *
 * Copyright (c) 2018 Davide Di Carlo
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdbool.h>
#include <error/error.h>

#if !(defined(__GNUC__) || defined(__clang__))
#define __attribute__(...)
#endif

#define PROCESS_VERSION_MAJOR        0
#define PROCESS_VERSION_MINOR        1
#define PROCESS_VERSION_PATCH        0
#define PROCESS_VERSION_SUFFIX       ""
#define PROCESS_VERSION_IS_RELEASE   0
#define PROCESS_VERSION_HEX          0x000100

extern const Error ProcessUnableToFork;
extern const Error ProcessInvalidState;

struct Process_ExitInfo {
    int exitValue;
    bool exitNormally;
};

struct Process {
    /* Do not access these members directly! */
#ifndef NDEBUG
    long _magicNumber;
#endif
    int _id;
    int _inputFileDescriptor;
    int _errorFileDescriptor;
    int _outputFileDescriptor;
    int _exitValue;
    bool _exitNormally;
    bool _isAlive;
};

extern ErrorOf(Ok, ProcessUnableToFork) Process_spawn(struct Process *self, void (*f)(void))
__attribute__((__warn_unused_result__, __nonnull__));

extern ErrorOf(Ok, ProcessInvalidState) Process_wait(struct Process *self, struct Process_ExitInfo *out)
__attribute__((__warn_unused_result__, __nonnull__(1)));

extern ErrorOf(Ok, ProcessInvalidState) Process_cancel(struct Process *self, struct Process_ExitInfo *out)
__attribute__((__warn_unused_result__, __nonnull__(1)));

extern ErrorOf(Ok, ProcessInvalidState) Process_exitInfo(const struct Process *self, struct Process_ExitInfo *out)
__attribute__((__warn_unused_result__, __nonnull__));

extern long Process_writeInputStream(struct Process *self, const char *buffer, size_t size)
__attribute__((__warn_unused_result__, __nonnull__));

extern long Process_readOutputStream(struct Process *self, char *buffer, size_t size)
__attribute__((__warn_unused_result__, __nonnull__));

extern long Process_readErrorStream(struct Process *self, char *buffer, size_t size)
__attribute__((__warn_unused_result__, __nonnull__));

extern int Process_id(const struct Process *self)
__attribute__((__warn_unused_result__, __nonnull__));

extern bool Process_isAlive(struct Process *self)
__attribute__((__warn_unused_result__, __nonnull__));

extern void Process_teardown(struct Process *self)
__attribute__((__nonnull__));

extern int Process_getCurrentId(void)
__attribute__((__warn_unused_result__));

extern int Process_getParentId(void)
__attribute__((__warn_unused_result__));

extern void Process_sleep(unsigned seconds);

#ifdef __cplusplus
}
#endif
