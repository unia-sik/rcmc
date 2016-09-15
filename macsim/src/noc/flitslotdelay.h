#ifndef _FLITSLOTDLEAY_H
#define _FLITSLOTDLEAY_H

#include <stdlib.h>
#include <string.h>

#define RANK_EMPTY -1

typedef struct {
    uint_fast32_t       size;
    uint_fast32_t       count;
    flit_container2_t   *buf;
} flitslotdelay_t;

static inline bool flitslotdelay_step(flitslotdelay_t *delay, flit_container2_t *out)
{
    bool emit = false;
    for (int i = 0; i < delay->size; i++)
    {
        // Slot empty?
        if (delay->buf[i].src == RANK_EMPTY) continue;

        if (i == 0)
        {
          *out = delay->buf[i];
          delay->count--;
          emit = true;
        } else {
          delay->buf[i-1] = delay->buf[i];
        }
        delay->buf[i].src = RANK_EMPTY;
    }
    return emit;
}

static inline bool flitslotdelay_insert(flitslotdelay_t *delay, flit_container2_t *ref)
{
    if (delay->buf[delay->size-1].src != RANK_EMPTY)
    {
        return false;
    }
    delay->buf[delay->size-1] = *ref;
    delay->count++;
    return true;
}

static inline void flitslotdelay_init(flitslotdelay_t *delay, uint_fast32_t size) {
    delay->size = size;
    delay->count = 0;
    delay->buf = fatal_malloc(size*sizeof(flit_container2_t));
    for (int i = 0; i < delay->size; i++)
    {
        delay->buf[i].src = RANK_EMPTY;
    }
}

static inline void flitslotdelay_destroy(flitslotdelay_t *delay) {
    delay->count = 0;
    delay->size = 0;
    free(delay->buf);
}

#endif
