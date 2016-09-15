/*
 * flitfifo.h
 * Management of FIFOs with flits
 *
 * RC/MC project
 */

#ifndef _FLITFIFO_H
#define _FLITFIFO_H

#include <stdlib.h>
#include <string.h>

// OPTIMISE: the size of this container should be aligned to a power of 2
typedef struct {
    rank_t      src;
    rank_t      dest;
    flit_t      flit;
} flit_container2_t;

typedef struct {
    uint_fast32_t       size;
    uint_fast32_t       first;
    uint_fast32_t       count;
    flit_container2_t   *buf;
} flitfifo_t;



static const char letter[64] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz$&";

static inline void print_flit(bool valid, flit_container2_t fc)
{
    if (!valid) user_printf(" . ");
    else user_printf("%c%c%c", letter[(fc.src%conf_max_rank)&63],
        letter[(fc.src/conf_max_rank)%63], letter[fc.dest%63]);
}



static inline void flitfifo_print(flitfifo_t *fifo)
{
    int i, j;
    printf("1st=%lu %lu/%lu ", fifo->first, fifo->count, fifo->size);
    if (fifo->first+fifo->count > fifo->size) {
        for (i=0; i<fifo->first+fifo->count-fifo->size; i++)
            print_flit(true, fifo->buf[i]);
//            putchar('a' + i + fifo->size - fifo->first);
//            printf("%lu", fifo->buf[i].src);
        for (; i < fifo->first; i++) putchar('.');
        j='a'-i;
        for (; i < fifo->size; i++) 
            print_flit(true, fifo->buf[i]);
//            putchar(j+i);
//            putchar('0'+fifo->buf[i].src);
    } else {
        for (i=0; i<fifo->first; i++) putchar('.');
        for (j=0; j<fifo->count; j++,i++) //putchar('a'+j);
            print_flit(true, fifo->buf[i]);
//            putchar('0'+fifo->buf[i].src);
//            printf("%lu", fifo->buf[i].src);
        for (;i<fifo->size; i++) putchar('.');
    }
}


static inline void flitfifo_verbose(flitfifo_t *fifo)
{
    int i, j;
    printf("1st=%lu %lu/%lu", fifo->first, fifo->count, fifo->size);
    if (fifo->first+fifo->count > fifo->size) {
        for (i=0; i<fifo->first+fifo->count-fifo->size; i++)
            printf(" %lx", fifo->buf[i].src);
        for (; i < fifo->first; i++) putchar('.');
        j='a'-i;
        for (; i < fifo->size; i++) 
            printf(" %lx", fifo->buf[i].src);
    } else {
        for (i=0; i<fifo->first; i++) putchar('.');
        for (j=0; j<fifo->count; j++,i++) //putchar('a'+j);
            printf(" %lx", fifo->buf[i].src);
        for (;i<fifo->size; i++) putchar('.');
    }
}



static inline bool flitfifo_empty(flitfifo_t *fifo)
{
    return fifo->count == 0;
}

static inline bool flitfifo_full(flitfifo_t *fifo)
{
    return fifo->count >= fifo->size;
}

static inline bool flitfifo_insert(flitfifo_t *fifo, flit_container2_t *fc)
{
    if (fifo->count >= fifo->size) return false;
    fifo->buf[(fifo->first+fifo->count) % fifo->size] = *fc;
    fifo->count++;
//if (fifo->size>10) {flitfifo_print(fifo); printf("FFINS\n");}
    return true;
}

static inline bool flitfifo_remove(flitfifo_t *fifo, flit_container2_t *fc)
{
    if (fifo->count == 0) return false;
    fifo->count--;
    uint_fast32_t i = fifo->first;
    *fc = fifo->buf[i];
    fifo->first = (i+1==fifo->size) ? 0 : i+1;
//if (fifo->size>10) {flitfifo_print(fifo); printf("FFRM\n");}
    return true;
}

static inline int_fast32_t flitfifo_find_rank(flitfifo_t *fifo, rank_t src)
{
    int_fast32_t i = fifo->first;
    uint_fast32_t remain = fifo->count;
    while (remain>0) {
        if (fifo->buf[i].src == src) return i;
        i++;
        if (i==fifo->size) i=0;
        remain--;
    }
    return -1;
}

static inline bool flitfifo_remove_by_rank(flitfifo_t *fifo, rank_t src,
    flit_container2_t *fc)
{
    uint32_t first = fifo->first;
    uint32_t remain = fifo->count;

    uint32_t i = first;
    while (remain>0) {
        if (fifo->buf[i].src == src) {
            *fc = fifo->buf[i];
            if (i >= first) {
                // found entry is at a higher address than the first entry
                // => move preceeding entries one up
                memmove(&fifo->buf[first+1], &fifo->buf[first], 
                    (i-first)*sizeof(flit_container2_t));
                fifo->first = (first+1==fifo->size) ? 0 : first+1;
            } else {
                // found entry is at a lower address than the first entry
                // => move succeeding entries one down
                memmove(&fifo->buf[i], &fifo->buf[i+1], 
                    (remain-1)*sizeof(flit_container2_t));
            }
            fifo->count--;
//if (fifo->size>10) {flitfifo_print(fifo); printf("FFRR\n");}
            return true;
        }
        i++;
        if (i==fifo->size) i=0;
        remain--;
    }
    return false;
}

static inline rank_t flitfifo_first_rank(flitfifo_t *fifo)
{
    return (fifo->count==0) ? (rank_t)-1 : fifo->buf[fifo->first].src;
}

static inline void flitfifo_init(flitfifo_t *fifo, uint_fast32_t size)
{
    fifo->count = 0;
    fifo->first = 0;
    fifo->size = size;
    fifo->buf = fatal_malloc(size*sizeof(flit_container2_t));
}

static inline void flitfifo_destroy(flitfifo_t *fifo)
{
    fifo->size = 0;
    free(fifo->buf);
}

#endif
