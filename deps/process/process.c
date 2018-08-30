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

#include <wait.h>
#include <errno.h>
#include <stdint.h>
#include <memory.h>
#include <assert.h>
#include <unistd.h>
#include <panic/panic.h>
#include "process.h"

#define MAGIC_NUMBER    0xdeadbeaf

#define expect(condition, ...) \
    __expect((__FILE__), (__LINE__), (condition), __VA_ARGS__)

#define ensure(condition) \
    expect((condition), (NULL))

#define die() \
    ensure(false)

/*
 * Miscellaneous
 */
static void __expect(const char *file, int line, bool condition, const char *format, ...)
__attribute__((__nonnull__(1), __format__(__printf__, 4, 5)));

static void moveFileDescriptor(int fd1, int fd2);

/*
 * Pipe
 */
union Pipe {
    int fileDescriptors[2];
    struct { int inputFileDescriptor, outputFileDescriptor; };
};

static void Pipe_open(union Pipe *self)
__attribute__((__nonnull__));

/*
 * Process
 */
const Error Process_UnableToFork = Error_new("Unable to fork");

static void Process_setExitInfoFromStatus(struct Process *self, int status)
__attribute__((__nonnull__));

Error Process_spawn(struct Process *const self, void (*const f)(void)) {
    assert(self);
    assert(f);
    pid_t pid;
    union Pipe pipeStderr, pipeStdout, pipeStdin;

    fflush(NULL);
    Pipe_open(&pipeStderr);
    Pipe_open(&pipeStdout);
    Pipe_open(&pipeStdin);

    switch (pid = fork()) {
        case -1: {  // error
            return Process_UnableToFork;
        }
        case 0: {   // child process
            fflush(NULL);
            const int stdinFileDescriptor = fileno(stdin);
            const int stderrFileDescriptor = fileno(stderr);
            const int stdoutFileDescriptor = fileno(stdout);

            ensure(close(stdinFileDescriptor) == 0);
            ensure(close(stderrFileDescriptor) == 0);
            ensure(close(stdoutFileDescriptor) == 0);

            ensure(close(pipeStdin.outputFileDescriptor) == 0);
            ensure(close(pipeStderr.inputFileDescriptor) == 0);
            ensure(close(pipeStdout.inputFileDescriptor) == 0);

            moveFileDescriptor(pipeStdin.inputFileDescriptor, stdinFileDescriptor);
            moveFileDescriptor(pipeStderr.outputFileDescriptor, stderrFileDescriptor);
            moveFileDescriptor(pipeStdout.outputFileDescriptor, stdoutFileDescriptor);

            fflush(NULL);
            f();
            fflush(NULL);
            _exit(0);
        }
        default: {  // parent process
            fflush(NULL);
            ensure(close(pipeStdin.inputFileDescriptor) == 0);
            ensure(close(pipeStderr.outputFileDescriptor) == 0);
            ensure(close(pipeStdout.outputFileDescriptor) == 0);
#ifndef NDEBUG
            self->_magicNumber = MAGIC_NUMBER;
#endif
            self->_id = pid;
            self->_inputFileDescriptor = pipeStdin.outputFileDescriptor;
            self->_errorFileDescriptor = pipeStderr.inputFileDescriptor;
            self->_outputFileDescriptor = pipeStdout.inputFileDescriptor;
            self->_exitValue = 0;
            self->_isAlive = true;
            self->_exitNormally = false;
            return Ok;
        }
    }
}

Error Process_wait(struct Process *const self, struct Process_ExitInfo *const out) {
    assert(self);
    assert(self->_magicNumber == MAGIC_NUMBER);

    if (self->_isAlive) {
        int status;
        ensure(waitpid(self->_id, &status, 0) == self->_id);

        Process_setExitInfoFromStatus(self, status);

        if (out) {
            const Error e = Process_exitInfo(self, out);
            assert(e == Ok);
            (void) e;
        }
        return Ok;
    } else {
        return IllegalState;
    }
}

Error Process_cancel(struct Process *const self, struct Process_ExitInfo *const out) {
    assert(self);
    assert(self->_magicNumber == MAGIC_NUMBER);

    if (self->_isAlive) {
        int status;  // this must not be initialized
        const pid_t pid = self->_id;

        for (int i = 0; (i < 8) && (kill(pid, SIGTERM) != 0); i++) {}
        for (int i = 0; i < 2; i++) {
            const int w = waitpid(pid, &status, WNOHANG);
            if (w == pid) {
                Process_setExitInfoFromStatus(self, status);
                if (out) {
                    const Error e = Process_exitInfo(self, out);
                    assert(e == Ok);
                    (void) e;
                }
                return Ok;
            } else if (w == 0) {
                sleep(1);
            } else {
                die();
            }
        }

        for (int i = 0; (i < 8) && (kill(pid, SIGKILL) != 0); i++) {}
        ensure(waitpid(pid, &status, 0) == pid);
        Process_setExitInfoFromStatus(self, status);
        if (out) {
            const Error e = Process_exitInfo(self, out);
            assert(e == Ok);
            (void) e;
        }
        return Ok;
    } else {
        return IllegalState;
    }
}

Error Process_exitInfo(const struct Process *const self, struct Process_ExitInfo *const out) {
    assert(self);
    assert(self->_magicNumber == MAGIC_NUMBER);
    assert(out);

    if (self->_isAlive) {
        return IllegalState;
    } else {
        out->exitNormally = self->_exitNormally;
        out->exitValue = self->_exitValue;
        return Ok;
    }
}

long Process_writeInputStream(struct Process *const self, const char *const buffer, const size_t size) {
    assert(self);
    assert(self->_magicNumber == MAGIC_NUMBER);
    assert(buffer);
    assert(size != SIZE_MAX);
    return write(self->_inputFileDescriptor, buffer, size);
}

long Process_readOutputStream(struct Process *const self, char *const buffer, const size_t size) {
    assert(self);
    assert(self->_magicNumber == MAGIC_NUMBER);
    assert(buffer);
    assert(size != SIZE_MAX);
    return read(self->_outputFileDescriptor, buffer, size);
}

long Process_readErrorStream(struct Process *const self, char *const buffer, const size_t size) {
    assert(self);
    assert(self->_magicNumber == MAGIC_NUMBER);
    assert(buffer);
    assert(size != SIZE_MAX);
    return read(self->_errorFileDescriptor, buffer, size);
}

int Process_id(const struct Process *const self) {
    assert(self);
    assert(self->_magicNumber == MAGIC_NUMBER);
    return self->_id;
}

bool Process_isAlive(struct Process *const self) {
    assert(self);
    assert(self->_magicNumber == MAGIC_NUMBER);
    if (self->_isAlive) {
        int status;
        const pid_t pid = self->_id;

        const int w = waitpid(pid, &status, WNOHANG);
        ensure(w >= 0);
        if (w == pid) {
            Process_setExitInfoFromStatus(self, status);
        }
    }
    return self->_isAlive;
}

void Process_teardown(struct Process *const self) {
    assert(self);
    assert(self->_magicNumber == MAGIC_NUMBER);
    assert(!self->_isAlive);
    ensure(close(self->_inputFileDescriptor) == 0);
    ensure(close(self->_errorFileDescriptor) == 0);
    ensure(close(self->_outputFileDescriptor) == 0);
    memset(self, 0, sizeof(*self));
}

int Process_getCurrentId(void) {
    return getpid();
}

int Process_getParentId(void) {
    return getppid();
}

void Process_sleep(unsigned seconds) {
    sleep(seconds);
}

void Process_setExitInfoFromStatus(struct Process *const self, const int status) {
    assert(self);
    assert(self->_magicNumber == MAGIC_NUMBER);
    assert(self->_isAlive);
    if (WIFEXITED(status)) {
        self->_exitNormally = true;
        self->_exitValue = WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        self->_exitNormally = false;
        self->_exitValue = WTERMSIG(status);
    } else {
        die();
    }
    self->_isAlive = false;
}

/*
 * Miscellaneous
 */
void __expect(const char *const file, const int line, const bool condition, const char *const format, ...) {
    if (!condition) {
        if (format) {
            va_list args;
            va_start(args, format);
            __Panic_vterminate(file, line, format, args);
        } else {
            __Panic_terminate(file, line, "Unexpected error");
        }
    }
}

void moveFileDescriptor(const int fd1, const int fd2) {
    while (dup2(fd1, fd2) == -1) {
        ensure(EINTR == errno);
    }
    ensure(close(fd1) == 0);
}

/*
 * Pipe
 */
void Pipe_open(union Pipe *const self) {
    assert(self);
    expect(pipe(self->fileDescriptors) == 0, "Unable to open pipe");
}
