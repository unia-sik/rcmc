#include <sys/stat.h>

int _close(int file) 
{
    return -1; 
}


int _fini(int file) 
{
    return 0; 
}


int _fstat(int file, struct stat *st) 
{
    return 0;
}


int _isatty(int file) 
{
    return 1; 
}


int _lseek(int file, int ptr, int dir) 
{
    return 0;
}


int _open(const char *name, int flags, int mode)
{
    return -1;
}


int _read(int file, char *ptr, int len) 
{
    return 0;
}


char *heap_end = 0;
extern char __end__; // from linker
extern char __heap_top; // from linker

caddr_t _sbrk(int incr) 
{
    if (heap_end == 0) heap_end = &__end__;

    if (heap_end + incr > &__heap_top)
        return (caddr_t)0; // collision of heap and stack

    caddr_t r = (caddr_t)heap_end;
    heap_end += incr;
    return r;
}
