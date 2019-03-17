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


#include <stdint.h>
#include "fgmp.h"
#include "fgmp_block.h"
#include "fgmp_addr.h"
#include "random.h"
#include "mpi.h"


#define BLOCK_SIZE 32           // integers per thread
#define CHUNK_SIZE 8            // small chunks are sorted with a different algorithm

uint64_t buf[3*BLOCK_SIZE];
unsigned long blocksize = BLOCK_SIZE;

//needs to be signed (--> barrier)
#define cid_t int64_t 

// local sort of 8 elements with odd-even merge sorting
void chunk_sort_oddeven(uint64_t *buf)
{
//     assert (CHUNK_SIZE == 8);
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
void barrier(fgmp_info_t* info)
{   
    MPI_Barrier(info);
}

void merge_up(unsigned long ofs, cid_t other)
{
    unsigned long k;
    unsigned long dest = ofs ^ BLOCK_SIZE;

    // handshake    
    fgmp_srdy(other);
//     fgmp_bnr(other);
    
    fgmp_bsf();
    fgmp_snd(other, buf[ofs+blocksize-1]);    
    
    fgmp_bre();
    buf[2*BLOCK_SIZE] = fgmp_rcvp();
    
    fgmp_block_send_no_srdy(other, (blocksize - 1) * sizeof(uint64_t), buf + ofs);

    // merge sort bottom up and receive
    uint64_t *pa = &buf[ofs];
    uint64_t *pb = &buf[2*BLOCK_SIZE];
    uint64_t a = *pa++;
    uint64_t b = *pb++;
    for (k=1; k<blocksize; k++) {
        fgmp_bre();
        buf[2*BLOCK_SIZE+k] = fgmp_rcvp();
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

    // handshake
    fgmp_srdy(other);
//     fgmp_bnr(other);
    
    fgmp_bre();
    buf[2*BLOCK_SIZE+blocksize-1] = fgmp_rcvp();    
    
    fgmp_bsf();
    fgmp_snd(other, buf[ofs]);
    
    fgmp_block_recv_no_srdy(other, (blocksize - 1) * sizeof(uint64_t), buf + 2 * BLOCK_SIZE);
    
    // merge sort top down and send
    uint64_t *pa = &buf[ofs+blocksize-1];
    uint64_t *pb = &buf[2*BLOCK_SIZE+blocksize-1];
    uint64_t a = *pa--;
    uint64_t b = *pb--;
    for (k=1; k<blocksize; k++) {
        fgmp_bsf();
        fgmp_snd(other, buf[ofs+k]);
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




int main(int argc, char *argv[])
{
    long i, j, k; 
    fgmp_info_t info = fgmp_info();
    
    
    unsigned long log2p = 0;
    while ((2<<log2p) <= info.size) log2p++;

    unsigned long log2n = 0;
    while ((2<<log2n) <= blocksize) log2n++;
//     assert ((1<<log2n)==blocksize);
//     assert(blocksize <= BLOCK_SIZE);    
    
    // generate numbers
    random_seed_t seed;
    random_init_seed_default(&seed);
    
    for (i=0; i < fgmp_addr_end(&info); i = fgmp_addr_next_by_addr(i, &info)) {
        if (i==info.address) {
            for (k=0; k<blocksize; k++) {
                buf[k] = random_next_value(&seed);
            }
            // The loop can be canceled here, but by executing the superflous
            // xorshift jumps, each thread has nearly the same execution time
            // and no barrier is neccessary when timing starts afterwards.
        }
        random_balance(&seed);
    }

    // start timing
    unsigned long start0 = 0;//rdcycle();

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

    unsigned long start = 0;//rdcycle();

    // inter-node odd-even merge sorting
    for (i=0; i<log2p; i++) {

        cid_t other = info.rank ^ (1<<i);
        if (info.rank < other)
            merge_up(toggle, fgmp_addr_from_rank(other, &info));
        else
            merge_down(toggle, fgmp_addr_from_rank(other, &info));
        toggle ^= BLOCK_SIZE;

        j = i;
        while (j != 0) { // for j = i-1 ... 0 do
            j--;

            unsigned long width = 2<<i;
            unsigned long rel = info.rank & (width-1);
            unsigned long step = 1<<j;
            if (rel>=step && rel<(width-step)) {
                if ((rel>>j)&1)
                    merge_up(toggle, fgmp_addr_from_rank(info.rank+step, &info));
                else
                    merge_down(toggle, fgmp_addr_from_rank(info.rank-step, &info));
                toggle ^= BLOCK_SIZE;
            }
        }
    }
    
    barrier(&info);

    // stop timing
    unsigned long stop = 0;//rdcycle();

    // check local
    uint64_t v = 0, lower;
    for (k=0; k<blocksize; k++) {
        uint64_t x = buf[toggle+k];
        if (v > x) {
//             printf("core %ld: %lx > a[%ld]=%lx\n", my_cid, v, k, x);
            v = UINT64_MAX;
            break;
        }
        v = x;
    }

    // sync check
    if (info.address == 0) {
        lower = 0;
    } else {
        fgmp_srdy(fgmp_addr_prev(&info));
        fgmp_bre();
        lower = fgmp_rcvp();
    }
//     lower = (my_cid==0) ? 0 : fgmp_recv_flit(my_cid-1);
    if (lower > buf[toggle]) {
//         printf("first element %ld is lower than last element %ld of previous core\n",
//             buf[toggle], lower);
        return -1;
    }

    if (info.address < fgmp_addr_last(&info)) {
//         fgmp_bnr(fgmp_addr_next(&info)); 
        fgmp_bsf();
        fgmp_snd(fgmp_addr_next(&info), v);
        
        return 0;
    } else if (v != UINT64_MAX) {
//         printf("correctly sorted %lu elements in %lu cycles"
//             " ( %lu cycles/iteration/element) local sorting: %ld cycles\n",
//             blocksize*(1<<log2p), stop-start,
//             (stop-start)/((log2p+1)*log2p/2)/blocksize,
//             start-start0);
        return 0;
    }

    return -1;
}

