/* Debugging support */

#ifndef _DEBUG_H
#define _DEBUG_H

#include <assert.h>


static inline void Debug(char ch)
{
    asm volatile (
	"mov r0, %0\n\t"
	"svc 1\n\t"
	: : "r" (ch) : "r0");
}

static inline void delay(long time)
{
    while (time>0) {
        time--;
        asm volatile ("nop");
    }
}


#endif // !_DEBUG_H
