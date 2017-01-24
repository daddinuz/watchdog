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
#ifndef NDEBUG
#include "Watchdog/Watchdog.h"
#endif


int main(void) {
    /* no free, this will leak */
    double *number = malloc(sizeof(double));

    watchdog_dump();

    char *str = calloc(10, sizeof(char));
    str = realloc(str, 12);
    free(str);

    watchdog_collect();
    watchdog_dump();
    return EXIT_SUCCESS;
}
