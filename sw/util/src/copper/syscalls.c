#include <sys/stat.h>
#include <sys/time.h>
#include <errno.h>

#define SYS_write       64
#define SYS_exit        93


/*
static long syscall(long num, long arg0, long arg1, long arg2)
{
  register long a7 asm("a7") = num;
  register long a0 asm("a0") = arg0;
  register long a1 asm("a1") = arg1;
  register long a2 asm("a2") = arg2;
  asm volatile ("scall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a7));
  return a0;
}
*/

#define write_csr(reg, val) \
  asm volatile ("csrw " #reg ", %0" :: "r"(val))

int errno;

// return the adress of errno (important for reentrant functions)
int *__errno()
{
    return &errno;
}



int close(int file) 
{
    return -1; 
}


/*
int _fini(int file) 
{
    return 0; 
}
*/

int fstat(int file, struct stat *st) 
{
    return 0;
}

long getpid()
{
    return 0;
}


int gettimeofday(struct timeval *p, void *z)
{
    return 0;
}


int isatty(int file) 
{
    return 1; 
}


long kill(int pid, int sig)
{
    return 0;
}


int lseek(int file, int ptr, int dir) 
{
    return 0;
}


/*
int _open(const char *name, int flags, int mode)
{
    return -1;
}
*/

int read(int file, char *ptr, int len) 
{
    return 0;
}


char *heap_end = 0;
char *heap_top = 0;
extern char __end__; // from linker
extern char __heap_top; // from linker

caddr_t sbrk(int incr) 
{
    if (heap_end == 0) heap_end = &__end__;

    if (heap_end + incr > &__heap_top) {

        //errno = ENOMEM;
        // If the above line is uncommented, gcc generates an
        // infinite loop here. Don't know why

        return (caddr_t)-1; // collision of heap and stack
    }

    caddr_t r = (caddr_t)heap_end;
    heap_end += incr;
    return r;
}


long write(int file, char *buf, long nbytes)
{
    int i;
    for (i=0; i<nbytes; i++)
        write_csr(0x782, buf[i]);
    return nbytes;
//    syscall(SYS_write, file, (long)buf, nbytes);
}

