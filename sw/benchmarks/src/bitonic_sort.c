// WCET-aware distributed sort with bitonic mergesort
//
// The algorithm is similar to the test program fgmp_sort.c, but merging is done
// en block after transfering the data, not in parallel and the bitonic sort net
// is used insted of odd-even merge sort. Both differences result in more uniform
// execution times.


#include "harness.h"





#ifndef DEFAULT_LOG2_P
#define DEFAULT_LOG2_P 2
#endif
#ifndef DEFAULT_PROBLEM_SIZE
#define DEFAULT_PROBLEM_SIZE 0
#endif


#define MB_BUF          1

#define DIR_MIRROR      0
#define DIR_STEP        2



// data that is send to every core
typedef struct params_data_s {
    long log2_nprocs;
    long log2_blocksize;
} params_data_t;


typedef struct percore_data_s {
    params_data_t params;
    uint64_t *buf;
    unsigned long toggle;       // start of sorted data within buf
} percore_data_t;




#define CHUNK_SIZE 8





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


static void local_sort(percore_data_t *percore)
{
    unsigned long log2_blocksize = percore->params.log2_blocksize;
    unsigned long blocksize = 1 << log2_blocksize;
    uint64_t *buf = percore->buf;
    unsigned long toggle = percore->toggle;
    unsigned long i, k;



    // local sort of 8 elements
    for (i=0; i<blocksize; i+=CHUNK_SIZE) {
        chunk_sort_oddeven(percore->buf+i);
        //chunk_sort_insert(percore->buf+i);
    }


    // local merge sort
    for (i=3; i<log2_blocksize; i++) {
        uint64_t *begin = &buf[toggle];
        toggle ^= blocksize;
        uint64_t *pd = &buf[toggle];


        uint64_t next_a = *begin;
        for(k=0; k<(1<<(log2_blocksize-1-i)); k++) {
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
    percore->toggle = toggle;
}





//  blockwise send, receive and sort: much faster than elementwise
static void merge(uint64_t *buf, unsigned long blocksize, unsigned long ofs, int dir, bool backward)
{
    unsigned long k;
    unsigned long dest = ofs ^ blocksize;

    // send buf[ofs] and receive to buf[2*blocksize]
    parop_forward_begin();
        parop_forward_int64(dir, blocksize,
            MB_BUF, 2*blocksize, 1,  // receive
            MB_BUF, ofs, 1);         // send
    parop_forward_end();

    if (!backward) {
        // merge sort bottom up
        uint64_t *pa = &buf[ofs];
        uint64_t *pb = &buf[2*blocksize];
        uint64_t a = *pa++;
        uint64_t b = *pb++;
        for (k=1; k<blocksize; k++) {
            if (a <= b) {
                buf[dest+k-1] = a;
                a = *pa++;
            } else {
                buf[dest+k-1] = b;
                b = *pb++;
            }
        }
        buf[dest+blocksize-1] = (a<=b) ? a : b;
    } else {
        // merge sort top down
        uint64_t *pa = &buf[ofs+blocksize-1];
        uint64_t *pb = &buf[2*blocksize+blocksize-1];
        uint64_t a = *pa--;
        uint64_t b = *pb--;
        for (k=1; k<blocksize; k++) {
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
}


static void distributed_sort(percore_data_t *percore)
{
    unsigned long log2_blocksize = percore->params.log2_blocksize;
    unsigned long log2_nprocs = percore->params.log2_nprocs;
    unsigned long blocksize = 1 << log2_blocksize;
    unsigned long toggle = percore->toggle;
    uint64_t *buf = percore->buf;
    int my_cid = parop_my_rank;

    // inter-node bitonic sort
    unsigned long i;
    for (i=0; i<log2_nprocs; i++) {

        unsigned long mask = (2 << i) - 1;
        unsigned long base = my_cid & ~mask;
        unsigned long ofs = my_cid & mask;
        int other = base + mask - ofs;
        merge(buf, blocksize, toggle, DIR_MIRROR+4*i, my_cid>other);
        toggle ^= blocksize;

        unsigned long j = i;
        while (j != 0) { // for j = i-1 ... 0 do
            j--;
            merge(buf, blocksize, toggle, DIR_STEP+4*j, (my_cid>>j)&1);
            toggle ^= blocksize;
        }
    }
    percore->toggle = toggle;
}


static void check(percore_data_t *percore)
{
    unsigned long blocksize = 1 << percore->params.log2_blocksize;
    unsigned long toggle = percore->toggle;
    uint64_t *buf = percore->buf;
    int my_cid = parop_my_rank;
    bool failed = false;

    // check local
    uint64_t v = 0;
    unsigned long i;
    for (i=0; i<blocksize; i++) {
        uint64_t x = buf[toggle+i];
        if (v > x) {
            printf("thread %d: %lx > a[%ld]=%lx\n", my_cid, v, i, x);
            failed = true;
            v = UINT64_MAX;
            break;
        }
        v = x;
    }

    unsigned long nprocs = 1 << percore->params.log2_nprocs;
    int64_t min[nprocs];
    int64_t max[nprocs];
    parop_gather_int64_world(failed ? UINT64_MAX : buf[toggle], min);
    parop_barrier(0);
    parop_gather_int64_world(failed ? 0 : buf[toggle+blocksize-1], max);

    PAROP_ONCE {
        unsigned long i;
        for (i=1; i<nprocs; i++) {
//            printf("%lu ... %lu, ", min[i], max[i]);
            if (max[i-1] > min[i]) failed = true;
        }
        if (!failed) {
            printf("verification successful\n");
        } else {
            printf("verification FAILED\n");
        }
    }
}


void init(percore_data_t *percore)
{
    unsigned long blocksize = 1 << percore->params.log2_blocksize;
    long i;
    int my_cid = parop_my_rank;

    percore->buf = (uint64_t *)parop_def_array_int64(MB_BUF, blocksize*3);
    percore->toggle = 0;

    for (i=0; i<percore->params.log2_nprocs; i++) {
        int mask = (2 << i) - 1;
        int base = my_cid & ~mask;
        int ofs = my_cid & mask;
        int other = base + mask - ofs;
        parop_def_direction(DIR_MIRROR+4*i, other, other);

        int step = 1<<i;
        other = ((my_cid>>i)&1) ? my_cid-step : my_cid+step;
        parop_def_direction(DIR_STEP+4*i, other, other);
    }

    uint64_t seed[2];
    seed[0] = 0x123456789abcdef0;
    seed[1] = 0xfedcba9876543210;


    // generate
    harness_rand_init(parop_my_rank, seed);
    for (i=0; i<blocksize; i++) {
        percore->buf[i] = harness_rand64(seed);
    }
}


int config(int argc, char *argv[], void *p, int available_threads)
{
    params_data_t *params = (params_data_t *)p;
    int nprocs;
    long problem_size;

    if (argc != 3) {
        harness_print_string("Usage: bitonic_sort <#threads> <problem size>\n"
            "  <#threads>     Number of threads. Must be a power of 2.\n"
            "  <problem size> Array size 2**(14+4*N) elements\n"
            "Invalid arguments, using default values instead: bitonic_sort ");

        // use default thread count if it is not given by the architecture
        nprocs = (available_threads == 0)
            ? (1 << DEFAULT_LOG2_P)
            : available_threads;
        problem_size = DEFAULT_PROBLEM_SIZE;

        harness_print_int32(nprocs);
        harness_print_string(" ");
        harness_print_int32(problem_size);
        harness_print_string("\n\n");
    } else {
        nprocs = harness_atoi(argv[1]);
        if (available_threads!=0 && nprocs > available_threads)
            nprocs = available_threads;
        problem_size = harness_atoi(argv[2]);
    }

    // nprocs must be a power of 2
    int log2_nprocs=0;
    while ((2<<log2_nprocs) <= nprocs) log2_nprocs++;
    params->log2_nprocs = log2_nprocs;

    if (log2_nprocs > 2*problem_size+13)
        harness_fatal("Must have at least 8 elements per thread");
    params->log2_blocksize = 2*problem_size+14 - log2_nprocs;

    harness_print_string("Parallel Benchmark BITONIC_SORT\n  size: ");
    harness_print_int64((uint64_t)1 << (4*problem_size + 14));
    harness_print_string("\n  using ");
    harness_print_int32(1 << log2_nprocs);
    harness_print_string(" of ");
    harness_print_int32(available_threads);
    harness_print_string(" threads\n");

    return 1 << log2_nprocs;
}


void parallel(void *p)
{
    params_data_t *params = p;
    percore_data_t *percore = machine_malloc(sizeof(percore_data_t));
    percore->params = *params;

    init(percore);
    local_sort(percore);
    distributed_sort(percore);
    check(percore);
}



int main(int argc, char **argv)
{
    return harness_spmd_main(argc, argv,
        sizeof(params_data_t),
        config,
        parallel);
}

