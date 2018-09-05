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

#define BUFFER_SIZE 64

void leaking(void) {
    char *buffer = calloc(BUFFER_SIZE + 1, sizeof(buffer[0]));
    snprintf(buffer, BUFFER_SIZE, "%d:%lu", Process_getCurrentId(), time(NULL));
    printf("%s", buffer);
    Process_sleep(5);
}

void nonLeaking(void) {
    char *buffer = calloc(BUFFER_SIZE / 3, sizeof(buffer[0]));
    buffer = realloc(buffer, (BUFFER_SIZE + 1) * sizeof(buffer[0]));
    snprintf(buffer, BUFFER_SIZE, "%d:%lu", Process_getCurrentId(), time(NULL));
    printf("%s", buffer);
    free(buffer);
    Process_sleep(2);
}

int main(void) {
    Error error;
    char *buffer = malloc((BUFFER_SIZE + 1) * sizeof(buffer[0]));
    struct Process leakingProcess, nonLeakingProcess;
    struct Process_ExitInfo info;
    long bytesRead;

    {   // spawn leaking process
        if (Process_spawn(&leakingProcess, leaking) != Ok) {
            Panic_terminate("Unable to fork");
        }
        printf("Spawned process: %d\n", Process_id(&leakingProcess));
    }

    {   // spawn non leaking process
        if (Process_spawn(&nonLeakingProcess, nonLeaking) != Ok) {
            Panic_terminate("Unable to fork");
        }
        printf("Spawned process: %d\n", Process_id(&nonLeakingProcess));
    }

    {   // wait leaking process
        error = Process_isAlive(&leakingProcess) ?
                Process_wait(&leakingProcess, &info) :
                Process_exitInfo(&leakingProcess, &info);
        if (error != Ok) {
            Panic_terminate("%s", Error_explain(error));
        }
        bytesRead = Process_readOutputStream(&leakingProcess, buffer, BUFFER_SIZE);
        if (bytesRead < 0) {
            Panic_terminate("Unexpected error while reading from output stream of process: %d",
                            Process_id(&leakingProcess));
        }
        printf("Process: %d exitNormally: %d exitValue: %2d output: %.*s\n",
               Process_id(&leakingProcess), info.exitNormally, info.exitValue, (int) bytesRead, buffer);
    }

    {   // wait non leaking process
        error = Process_isAlive(&nonLeakingProcess) ?
                Process_wait(&nonLeakingProcess, &info) :
                Process_exitInfo(&nonLeakingProcess, &info);
        if (error != Ok) {
            Panic_terminate("%s", Error_explain(error));
        }
        bytesRead = Process_readOutputStream(&nonLeakingProcess, buffer, BUFFER_SIZE);
        if (bytesRead < 0) {
            Panic_terminate("Unexpected error while reading from output stream of process: %d",
                            Process_id(&nonLeakingProcess));
        }
        printf("Process: %d exitNormally: %d exitValue: %2d output: %.*s\n",
               Process_id(&nonLeakingProcess), info.exitNormally, info.exitValue, (int) bytesRead, buffer);
    }

    Process_teardown(&nonLeakingProcess);
    Process_teardown(&leakingProcess);
    free(buffer);
    return 0;
}
