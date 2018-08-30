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

#include <time.h>
#include <stddef.h>
#include <stdalign.h>
#include <watchdog.h>
#include <panic/panic.h>
#include <process/process.h>

void doSomething(void) {
    char *buffer = calloc(64, sizeof(buffer[0]));
    printf("%.*s", snprintf(buffer, 63, "%d:%lu", Process_getCurrentId(), time(NULL)), buffer);
    free(buffer);
}

int main(void) {
    char buffer[64] = "";
    const size_t bufferSize = sizeof(buffer) / sizeof(buffer[0]) - 1;
    struct Process_ExitInfo info;
    struct Process processesArray[2];
    const struct Process *processesArrayEnd = &processesArray[sizeof(processesArray) / sizeof(processesArray[0])];

    for (struct Process *process = processesArray; process < processesArrayEnd; process++) {
        if (Process_spawn(process, doSomething) != Ok) {
            Panic_terminate("Unable to fork");
        }
        printf("Spawned process: %d\n", Process_id(process));
    }

    for (struct Process *process = processesArray; process < processesArrayEnd; process++) {
        const Error e = Process_isAlive(process) ? Process_wait(process, &info) : Process_exitInfo(process, &info);
        if (e != Ok) {
            Panic_terminate("%s", Error_explain(e));
        }
        const long bytesRead = Process_readOutputStream(process, buffer, bufferSize);
        if (bytesRead < 0) {
            Panic_terminate("Unexpected error while reading from output stream of process: %d", Process_id(process));
        }
        printf("Process: %d exitNormally: %d exitValue: %2d output: %.*s\n",
               Process_id(process), info.exitNormally, info.exitValue, (int) bytesRead, buffer);
        Process_teardown(process);
    }

    void *a = aligned_alloc(alignof(max_align_t), 32);
    void *b = malloc(16);
    void *c = malloc(32);
    (void) a; // free(a);
    b = realloc(b, 32);
    (void) b; // free(b);
    free(c);
    return 0;
}
