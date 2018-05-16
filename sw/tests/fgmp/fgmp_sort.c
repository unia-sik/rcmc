/* Distributed sort with odd-even mergesort
 * 
 * Here, send, receive and merging of 2 elements are done simultaneously on
 * a per-element basis. The benchmark ``bitonic_sort'' is very similar, but the
 * whole block is first send, then received and then sorted.
 *
 * The communication pattern of the per-block solution is more regular than the
 * per-element one and therefore used in the WCET benchmark to simplify the
 * static timing analysis. Suprisingly, it is also much faster.
 * 
 * Nevertheless, the slower per-element solution solution is used here, because
 * it creates a much more complicated communication pattern that triggers more
 * special cases for this functional test.
 */

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "fgmp.h"
#include "debug.h"



#define BLOCK_SIZE 32           // integers per thread
#define CHUNK_SIZE 8            // small chunks are sorted with a different algorithm

uint64_t buf[3*BLOCK_SIZE];
unsigned long blocksize = BLOCK_SIZE;








static uint64_t xorshift128plus_next(uint64_t *seed) 
{
    uint64_t a = seed[0];
    uint64_t b = seed[1];
    a ^= a << 23;
    uint64_t c = a ^ b ^ (a >> 18) ^ (b >> 5);
    seed[0] = b;
    seed[1] = c;
    return c + b; 
}


static void xorshift128plus_jump(uint64_t *seed)
{
    uint64_t a = seed[0];
    uint64_t b = seed[1];
    uint64_t c = 0;
    uint64_t d = 0;
    uint64_t x = 0x8a5cd789635d2dff;

    for (unsigned i=0; i<128; i++) {
        if (i==64) x=0x121fd2155c472f96;
        if (x & 1) {
            c ^= a;
            d ^= b;
        }
        x >>= 1;
        uint64_t e = a ^ (a << 23);
        a = b;
        b = e ^ b ^ (e >> 18) ^ (b >> 5);
    }
    seed[0] = c;
    seed[1] = d;
}


// local sort of 8 elements with odd-even merge sorting
void chunk_sort_oddeven(uint64_t *buf)
{
    assert (CHUNK_SIZE == 8);
    uint64_t a0 = buf[0];
    uint64_t a1 = buf[1];
    uint64_t a2 = buf[2];
    uint64_t a3 = buf[3];
    uint64_t a4 = buf[4];
    uint64_t a5 = buf[5];
    uint64_t a6 = buf[6];
    uint64_t a7 = buf[7];

#define COMP(a, b) if (a>b) {uint64_t z=a; a=b; b=z; }
        COMP(a0, a1)        // sort 0
        COMP(a2, a3)
        COMP(a4, a5)
        COMP(a6, a7)

        COMP(a0, a2)        // sort 1
        COMP(a1, a3)
        COMP(a4, a6)
        COMP(a5, a7)

        COMP(a1, a2)        // merge 1 0
        COMP(a5, a6)

        COMP(a0, a4)        // sort 2
        COMP(a1, a5)
        COMP(a2, a6)
        COMP(a3, a7)

        COMP(a2, a4)        // merge 2 0
        COMP(a3, a5)

        COMP(a1, a2)        // merge 2 1
        COMP(a3, a4)
        COMP(a5, a6)
#undef COMP

    buf[0] = a0;
    buf[1] = a1;
    buf[2] = a2;
    buf[3] = a3;
    buf[4] = a4;
    buf[5] = a5;
    buf[6] = a6;
    buf[7] = a7;
}


// local sort of 8 elements with insert sort 
// currently unused
void chunk_sort_insert(uint64_t *p)
{
    unsigned i, j;
    for (i=1; i<CHUNK_SIZE; i++) {
        uint64_t x = p[i];
        j = i;
        while (j!=0 && p[j-1]>x) {
            p[j] = p[j-1];
            j--;
        }
        p[j] = x;
    }
}







void merge_up(unsigned long ofs, cid_t other)
{
    unsigned long dest = ofs ^ BLOCK_SIZE;

    uint64_t *pa = &buf[ofs];
    uint64_t *pb = &buf[2*BLOCK_SIZE];
    uint64_t *ps = &buf[ofs+blocksize-1];
    uint64_t *es = &buf[ofs];
    uint64_t *pr = &buf[2*BLOCK_SIZE];
    uint64_t *er = &buf[2*BLOCK_SIZE+blocksize-1];
    uint64_t *pd = &buf[dest];
    uint64_t *ed = &buf[dest+blocksize-1];

    // handshake
    fgmp_send_flit(other, *ps--);
    *pr++ = fgmp_recv_flit(other);

    // merge sort bottom up and receive
    uint64_t a = *pa++;
    uint64_t b = *pb++;
    while (pd < ed) {
        if (!fgmp_cong() && ps>=es) 
            fgmp_send_flit(other, *ps--);
        while (fgmp_probe(other) && pr<=er)
            *pr++ = fgmp_recv_flit(other);
        if (a <= b) {
            *pd++ = a;
            a = *pa++;
        } else if (pb < pr) { // enough received?
            *pd++ = b;
            b = *pb++;
        }
    }
    *pd++ = (a<=b) ? a : b;

    // receive and send remainig data
    while (ps>=es || pr<=er) {
        if (!fgmp_cong() && ps>=es) 
            fgmp_send_flit(other, *ps--);
        while (fgmp_probe(other) && pr<=er)
            *pr++ = fgmp_recv_flit(other);
    }
}


void merge_down(unsigned long ofs, cid_t other)
{
    unsigned long dest = ofs ^ BLOCK_SIZE;

    uint64_t *pa = &buf[ofs+blocksize-1];
    uint64_t *pb = &buf[2*BLOCK_SIZE+blocksize-1];
    uint64_t *ps = &buf[ofs];
    uint64_t *es = &buf[ofs+blocksize-1];
    uint64_t *pr = &buf[2*BLOCK_SIZE+blocksize-1];
    uint64_t *er = &buf[2*BLOCK_SIZE];
    uint64_t *pd = &buf[dest+blocksize-1];
    uint64_t *ed = &buf[dest];

    // handshake
    *pr-- = fgmp_recv_flit(other);
    fgmp_send_flit(other, *ps++);

    // merge sort top down and send
    uint64_t a = *pa--;
    uint64_t b = *pb--;
    while (pd > ed) {
        while (fgmp_probe(other) && pr>=er)
            *pr-- = fgmp_recv_flit(other);
        if (!fgmp_cong() && ps<=es)
            fgmp_send_flit(other, *ps++);
        if (a >= b) {
            *pd-- = a;
            a = *pa--;
        } else if (pb > pr) { // enough received?
            *pd-- = b;
            b = *pb--;
        }
    }
    *pd-- = (a>=b) ? a : b;

    while (pr>=er || ps<=es) {
        while (fgmp_probe(other) && pr>=er)
            *pr-- = fgmp_recv_flit(other);
        if (!fgmp_cong() && ps<=es)
            fgmp_send_flit(other, *ps++);
    }
}








/***********************************************************************
 Blockwise send, reveive and sort: 
 Much faster than elementwise.
 Not used here, because the elementwise solution produces a more
 complicated traffic pattern.


void merge_up(unsigned long ofs, cid_t other)
{
    unsigned long k;
    unsigned long dest = ofs ^ BLOCK_SIZE;

    // handshake and send
    fgmp_send_flit(other, buf[ofs+blocksize-1]);
    buf[2*BLOCK_SIZE] = fgmp_recv_flit(other);
    for (k=1; k<blocksize; k++) {
        fgmp_send_flit(other, buf[ofs+blocksize-1-k]);
    }

    // merge sort bottom up and receive
    uint64_t *pa = &buf[ofs];
    uint64_t *pb = &buf[2*BLOCK_SIZE];
    uint64_t a = *pa++;
    uint64_t b = *pb++;
    for (k=1; k<blocksize; k++) {
        buf[2*BLOCK_SIZE+k] = fgmp_recv_flit(other);
        if (a <= b) {
            buf[dest+k-1] = a;
            a = *pa++;
        } else {
            buf[dest+k-1] = b;
            b = *pb++;
        }
    }
    buf[dest+blocksize-1] = (a<=b) ? a : b;
}


void merge_down(unsigned long ofs, cid_t other)
{
    unsigned long k;
    unsigned long dest = ofs ^ BLOCK_SIZE;

    // handshake and receive
    buf[2*BLOCK_SIZE+blocksize-1] = fgmp_recv_flit(other);
    fgmp_send_flit(other, buf[ofs]);
    for (k=1; k<blocksize; k++) {
        buf[2*BLOCK_SIZE+blocksize-1-k] = fgmp_recv_flit(other);
    }

    // merge sort top down and send
    uint64_t *pa = &buf[ofs+blocksize-1];
    uint64_t *pb = &buf[2*BLOCK_SIZE+blocksize-1];
    uint64_t a = *pa--;
    uint64_t b = *pb--;
    for (k=1; k<blocksize; k++) {
        fgmp_send_flit(other, buf[ofs+k]);
        if (a >= b) {
            buf[dest+blocksize-k] = a;
            a = *pa--;
        } else {
            buf[dest+blocksize-k] = b;
            b = *pb--;
        }
    }
    buf[dest] = (a>=b) ? a : b;
}

***********************************************************************/



int main(int argc, char *argv[])
{
    long i, j, k;

    cid_t my_cid = fgmp_get_cid();
    cid_t max_cid = fgmp_get_max_cid();
    unsigned long log2p = 0;
    while ((2<<log2p) <= max_cid) log2p++;

    unsigned long log2n = 0;
    while ((2<<log2n) <= blocksize) log2n++;
    assert ((1<<log2n)==blocksize);
    assert(blocksize <= BLOCK_SIZE);

    // generate numbers
    uint64_t seed[2];
    seed[0] = 0x123456789abcdef0;
    seed[1] = 0xfedcba9876543210;
    for (i=0; i<max_cid; i++) {
        if (i==my_cid) {
            for (k=0; k<blocksize; k++) {
                buf[k] = xorshift128plus_next(seed);
            }
            // The loop can be canceled here, but by executing the superflous
            // xorshift jumps, each thread has nearly the same execution time
            // and no barrier is neccessary when timing starts afterwards.
        }
        xorshift128plus_jump(seed);
    }

    // local sort of 8 consecutive elements
    for (k=0; k<blocksize; k+=CHUNK_SIZE) {
        chunk_sort_oddeven(buf+k);
        //chunk_sort_insert(buf+k);
    }

    // local merge sort
    unsigned long toggle = 0; 
    for (i=3; i<log2n; i++) {
        uint64_t *begin = &buf[toggle];
        toggle ^= BLOCK_SIZE;
        uint64_t *pd = &buf[toggle];


        uint64_t next_a = *begin;
        for(k=0; k<(1<<(log2n-1-i)); k++) {
            uint64_t *ed = pd + (2<<i);
            uint64_t *pa = begin;
            uint64_t *pb = begin + (1<<i);
            uint64_t a = next_a; //*pa++;
            pa++;
            uint64_t b = *pb++;

            // one iteration to compute the value of the memory location,  where
            // the end marker in the last merge may be put (begin+(2<<i) == pd)
            if (a <= b) {
                *pd++ = a;
                a = *pa++;
            } else {
                *pd++ = b;
                b = *pb++;
            }

            // put end markers to avoid additional index comparison
            begin[1<<i] = UINT64_MAX;
            next_a = begin[2<<i];
            begin[2<<i] = UINT64_MAX;

            while (pd < ed) {
                if (a <= b) {
                    *pd++ = a;
                    a = *pa++;
                } else {
                    *pd++ = b;
                    b = *pb++;
                }
            }
            begin += 2<<i;
        }
        *begin = next_a;
    }

    // inter-node odd-even merge sorting
    for (i=0; i<log2p; i++) {

        cid_t other = my_cid ^ (1<<i);
        if (my_cid < other)
            merge_up(toggle, other);
        else
            merge_down(toggle, other);
        toggle ^= BLOCK_SIZE;

        j = i;
        while (j != 0) { // for j = i-1 ... 0 do
            j--;

            unsigned long width = 2<<i;
            unsigned long rel = my_cid & (width-1);
            unsigned long step = 1<<j;
            if (rel>=step && rel<(width-step)) {
                if ((rel>>j)&1)
                    merge_up(toggle, my_cid+step);
                else
                    merge_down(toggle, my_cid-step);
                toggle ^= BLOCK_SIZE;
            }
        }
    }

    // simple barrier
    if (my_cid==0) {
        fgmp_send_flit(1, 0);
        fgmp_recv_flit(max_cid-1);
    } else {
        fgmp_recv_flit(my_cid-1);
        fgmp_send_flit((my_cid+1) % max_cid, 0);
    }

    // check local
    uint64_t v = 0, lower;
    for (k=0; k<blocksize; k++) {
        uint64_t x = buf[toggle+k];
        if (v > x) {
            putchar('U'); // unordered within one core
            v = UINT64_MAX;
            break;
        }
        v = x;
    }

    // sync check
    lower = (my_cid==0) ? 0 : fgmp_recv_flit(my_cid-1);
    if (lower > buf[toggle]) {
        // first element is lower than last element of previous core
        putchar('L');
    }

    fgmp_send_flit((my_cid<max_cid-1) ? my_cid+1 : 0, v);

    if (my_cid==0) {
        v = fgmp_recv_flit(max_cid-1);
        if (v != UINT64_MAX) {
            putchar('k'); // successful
        } else {
            putchar('F'); // result not ordered
        }
        putchar('\n');
    }
    return 0;
}
