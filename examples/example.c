/*
 *  C Source File
 *
 *  Author: Davide Di Carlo
 *  Date:   April 28th 2016
 *  email:  daddinuz@gmail.com
 */

/**
 * WARNING: always include Watchdog.h after stdlib.h
 */
#include <stdlib.h>
#include "Watchdog/Watchdog.h"


/*
 * Print a memory summary, perform a garbage collection and terminate Watchdog.
 */
void terminate(void);


int main(void) {
    watchdog_initialize(stdout);    /* Initialize Watchdog and set stdout as output stream. */
    atexit(terminate);              /* Register a cleanup function to be called before exit. */

    watchdog_dump();

    /* no free, this will leak */
    double *number = malloc(sizeof(double));

    watchdog_dump();

    char *str = calloc(10, sizeof(char));
    str = realloc(str, 12);
    free(str);

    return EXIT_SUCCESS;            /* Will call terminate() */
}

void terminate(void) {
    watchdog_dump();
    watchdog_collect();
    watchdog_terminate();
}
