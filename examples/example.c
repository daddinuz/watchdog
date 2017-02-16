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
#define WATCHDOG_WRAP_STDLIB
#include "Watchdog.h"
#endif


/*
 *
 */
int main(void) {
    /* no free, this will leak */
    double *number = malloc(sizeof(double));
    /* free(number); */

    char *str = calloc(10, sizeof(char));
    str = realloc(str, 12);
    free(str);

    /* exit will appear on call logs */
    exit(EXIT_SUCCESS);
}
