#ifndef _MACHINE_H
#define _MACHINE_H

// Platform dependent parts

// This is an generic implementation using standard libc functions

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> // for memcpy
#include <time.h>
#include <stdint.h>
#include <sys/time.h>


#define machine_malloc  malloc
#define machine_free    free
#define machine_assert  assert
#define machine_exit    exit
#define machine_putchar putchar



static inline void machine_putstr(const char *s)
{
    int i=0;
    while (s[i]) {
        machine_putchar(s[i]);
        i++;
    }
}


static inline void machine_fatal(char *s)
{
    machine_putstr("FATAL ERROR: ");
    machine_putstr(s);
    machine_putchar('\n');
    machine_exit(1);
}


#endif // _MACHINE_H