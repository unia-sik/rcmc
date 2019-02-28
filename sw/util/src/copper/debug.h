/* Debugging support */

#ifndef _DEBUG_H
#define _DEBUG_H

#include <assert.h>


#define read_csr(reg) ({ unsigned long __tmp; \
  asm volatile ("csrr %0, " #reg : "=r"(__tmp)); \
  __tmp; })

#define write_csr(reg, val) \
  asm volatile ("csrw " #reg ", %0" :: "r"(val))

#define swap_csr(reg, val) ({ long __tmp; \
  asm volatile ("csrrw %0, " #reg ", %1" : "=r"(__tmp) : "r"(val)); \
  __tmp; })

#define rdtime() read_csr(time)
#define rdcycle() read_csr(cycle)
#define rdinstret() read_csr(instret)



static inline void Debug(char ch)
{
    write_csr(0x782, ch);
}

inline void delay(uint64_t time)
{
    for (volatile int i = 0; i < time; i++) {}
}


#endif
