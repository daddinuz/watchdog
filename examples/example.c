/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   April 28th 2016
 *  email:  daddinuz@gmail.com
 */

/**
 * WARNING: always include watchdog.h after stdlib.h
 */
#include <stdlib.h>
#include <stdio.h>
#include "Watchdog/watchdog.h"

/*
 * File to redirect watchdog.h outputs
 */
FILE *file = NULL;

/*
 * Terminate will be called before exit, will print
 * out a memory report and perform a garbage collect
 */
void terminate(void);

/*
 *
 */
int main(void) {
    watchdog_initialize();

    /*
     * Register terminate() to be called when the program exits normally
     */
    atexit(terminate);

    file = fopen("watchdog.log", "w");
    if (NULL == file) {
        fprintf(stderr, "Unable to open file: 'watchdog.log'\n");
        abort();            /** Will not call terminate **/
    }
    watchdog_set_output_stream(file);
    watchdog_dump();

    char *foo = malloc(6 * sizeof(char));
    char *moo = calloc(8, sizeof(char));

    watchdog_dump();

    foo = realloc(foo, 10 * sizeof(char));

    watchdog_dump();

    int *data = calloc(5, sizeof(int));
    free(data);

    watchdog_dump();

    return EXIT_SUCCESS;    /** Will call terminate **/
}

void terminate(void) {
    fclose(file);
    watchdog_set_output_stream(stderr);
    watchdog_dump();
    watchdog_collect();
    watchdog_terminate();
}
