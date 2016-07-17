/*
 * error.c
 * Rie¹enie IJC-DU1, 21.3.2011
 * Autor: Peter Lacko, FIT
 * Prelo¾ené: GCC 4.5.2
 */

#include<stdarg.h>
#include<stdio.h>
#include<stdlib.h>
#include"error.h"

// vypise chybovu hlasku na obrazovku
void Error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "CHYBA: ");
    vfprintf(stderr, fmt, args);
    va_end(args);
    exit(EXIT_FAILURE);
}
