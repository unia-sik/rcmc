#ifndef _PAROP_PTHREAD_H
#define _PAROP_PTHREAD_H

// -----------------------------------------------------------------------------
//   pthread implementation
// -----------------------------------------------------------------------------

#ifndef PAROP_MAX_THREADS
#define PAROP_MAX_THREADS 32
#endif

#include <pthread.h>

#define PAROP_LOCAL     _Thread_local


int parop_max_rank;
_Thread_local int parop_my_rank;

typedef struct {
    int                 *colors;
    double              *data;
    double              **dptr;
    pthread_barrier_t   barrier;
} parop_collection_t;

pthread_t parop_thread_tab[PAROP_MAX_THREADS];
void *parop_memblock[PAROP_MAX_THREADS][PAROP_MAX_MEMBLOCK];
int parop_directions[PAROP_MAX_THREADS][PAROP_MAX_DIRECTIONS];
parop_collection_t parop_collections[PAROP_MAX_COLLECTIONS];



static inline void parop_fatal(char *s)
{
    machine_fatal(s);
}








// -----------------------------------------------------------------------------
// memory allocation
// -----------------------------------------------------------------------------



void *parop_def_memblock(int handle, size_t n)
{
    machine_assert(handle < PAROP_MAX_MEMBLOCK);
    void *p = machine_malloc(n);
    if (p==0) parop_fatal("Out of memory");
    parop_memblock[parop_my_rank][handle] = p;
    return p;
}





// -----------------------------------------------------------------------------
// collectives
// -----------------------------------------------------------------------------



void parop_def_collection(int handle, int *colors)
{
    machine_assert(handle < PAROP_MAX_COLLECTIONS);
    PAROP_ONCE {
        parop_collection_t *c = &parop_collections[handle];

        c->colors = machine_malloc(parop_max_rank*sizeof(unsigned));
        memcpy(c->colors, colors, parop_max_rank*sizeof(unsigned));
        c->data = machine_malloc(parop_max_rank*sizeof(double));
        c->dptr = machine_malloc(parop_max_rank*sizeof(double*));
        pthread_barrier_init(&c->barrier, NULL, parop_max_rank);
    }
}


static inline void parop_barrier(int handle)
{
    machine_assert(handle==0); // subset barriers not yet supported
    pthread_barrier_wait(&parop_collections[handle].barrier);
}


static inline void parop_allgather_int64(int handle, int64_t v, int64_t r[])
{
    parop_collection_t *c = &parop_collections[handle];
    c->data[parop_my_rank] = v;
    pthread_barrier_wait(&c->barrier);

    int i, j=0;
    unsigned my_color = c->colors[parop_my_rank];
    for (i=0; i<parop_max_rank; i++) {
        if (c->colors[i]==my_color) {
            r[j++] = c->data[i];
        }
    }
}


// so far only for the world communicator, because the determination of
// the root thread is difficult with sub communicators
static inline void parop_gather_int64_world(int64_t v, int64_t r[])
{
    parop_collection_t *c = &parop_collections[0];
    c->data[parop_my_rank] = v;
    pthread_barrier_wait(&c->barrier);

    int i;
    PAROP_ONCE {
        for (i=0; i<parop_max_rank; i++) {
            r[i] = c->data[i];
        }
    }
}



static inline double parop_allreduce_double(int handle, unsigned op, double v)
{
    int my_rank = parop_my_rank;
    parop_collection_t *c = &parop_collections[handle];

    c->data[my_rank] = v;
    pthread_barrier_wait(&c->barrier);

    unsigned r;
    unsigned my_color = c->colors[my_rank];
    double a = c->data[my_rank];
    for (r=0; r<parop_max_rank; r++) {
        if (c->colors[r]==my_color && r!=my_rank) {
            double b = c->data[r];
            switch (op) {
                case PAROP_OP_ADD: a += b; break;
                case PAROP_OP_MUL: a *= b; break;
                case PAROP_OP_MIN: if (a>b) a=b; break;
                case PAROP_OP_MAX: if (a<b) a=b; break;
                default: machine_fatal("Unknown collective operation");
            }
        }
    }

    // this second barrier is necessary to avoid race conditions when
    // two allrecudes are back to back and c->data is overwritten
    // before all threads have read it
    pthread_barrier_wait(&c->barrier);

    return a;
}


static inline void parop_allreduce_multi_double(
    int handle, unsigned op, double result[], unsigned len, double v[])
{
    parop_collection_t *c = &parop_collections[handle];

    c->dptr[parop_my_rank] = v;
    pthread_barrier_wait(&c->barrier);

    unsigned i;
    for (i=0; i<len; i++) result[i] = 0.0;

    unsigned r;
    unsigned my_color = c->colors[parop_my_rank];
    for (r=0; r<parop_max_rank; r++) {
        if (c->colors[r]==my_color) {
            double *p=c->dptr[r];
            for (i=0; i<len; i++) {
                double a = result[i];
                double b = p[i];
                switch (op) {
                    case PAROP_OP_ADD: a += b; break;
                    case PAROP_OP_MUL: a *= b; break;
                    case PAROP_OP_MAX: if (a>b) a=b; break;
                    case PAROP_OP_MIN: if (a<b) a=b; break;
                    default: parop_fatal("Unknown collective operation");
                }
                result[i] = a;
            }
        }
    }

    // this second barrier is necessary to avoid race conditions when
    // v is freed immediately after this function.
    // Especially critical if v is on the stack of the calling routine
    pthread_barrier_wait(&c->barrier);
}






// -----------------------------------------------------------------------------
// point-to-point data transfers
// -----------------------------------------------------------------------------


// Two succeding entries in parop_directions belong together.
// The first on is the next node in this direction, the second
// one the node in the reverse direction (the node that sends to me)
// The direction handle must be a multiple of 2.
// To reverse the direction, the direction is xored with 1.
void parop_def_direction(int handle, int next, int prev)
{
    machine_assert(handle < PAROP_MAX_DIRECTIONS && (handle&1)==0);
    parop_directions[parop_my_rank][handle] = next;
    parop_directions[parop_my_rank][handle+1] = prev;
}


static inline void parop_forward_begin()
{
    parop_barrier(0);
}


void parop_forward_double(int direction, int len,
    int dmb, int di, int dstride,
    int smb, int si, int sstride)
{
    int rank = parop_my_rank;
    int prev = parop_directions[rank][direction^1];

    if (prev >= 0) {
        int i;
        double *p = ((double *)parop_memblock[rank][dmb]) + di;
        double *q = ((double *)parop_memblock[prev][smb]) + si;
        for (i=0; i<len; i++) {
            *p = *q;
            p += dstride;
            q += sstride;
        }
    }
}


void parop_forward_int64(int direction, int len,
    int dmb, int di, int dstride,
    int smb, int si, int sstride)
{
    int rank = parop_my_rank;
    int prev = parop_directions[rank][direction^1];

    if (prev >= 0) {
        int i;
        int64_t *p = ((int64_t *)parop_memblock[rank][dmb]) + di;
        int64_t *q = ((int64_t *)parop_memblock[prev][smb]) + si;
        for (i=0; i<len; i++) {
            *p = *q;
            p += dstride;
            q += sstride;
        }
    }
}


void parop_forward_end() {}





// -----------------------------------------------------------------------------
// broadcast data transfers
// -----------------------------------------------------------------------------

void parop_broadcast_data(int mb, int i, int len, void *data, int senderRank){
  // if this Process is not the sender nothing has to be done.
  // Sender Rank is only really necessarry for distributed memory
  // computing models like with MPI
  if(parop_my_rank != senderRank){
    return;
  }
  else{
    char *source = (char *)data;
    char *target = 0;
    int currentRank;
    int currentByte;
    for(currentRank = 0; currentRank < parop_max_rank; currentRank++){
      // target now points to the start of the correct memblock
      target = (char *)parop_memblock[currentRank][mb]; 
      // target now points to the correct index within the memblock
      target += i;
      for(currentByte = 0; currentByte < len; currentByte++){
        target[currentByte] = source[currentByte];
      }
    }
  }
}







// -----------------------------------------------------------------------------
// main application structure 
// -----------------------------------------------------------------------------


typedef struct {
    int rank;
    void (*entry)(void *);
    void *params;
} internal_pthread_args_t;


void internal_pthread_thread(internal_pthread_args_t *args)
{
    parop_my_rank = args->rank;
    args->entry(args->params);
}


int harness_spmd_main(int argc, char **argv,
    int params_data_size,
    int call_config(int, char **, void *, int),
    void call_parallel(void *))
{
    int r;
    char params[params_data_size];
    parop_my_rank = 0;
    parop_max_rank = call_config(argc, argv, params, PAROP_MAX_THREADS);

    int colors[parop_max_rank];
    for (r=0; r<parop_max_rank; r++) colors[r] = 0;
    parop_def_collection(0, colors);

    for (r=1; r<parop_max_rank; r++) {
        internal_pthread_args_t *args = 
            machine_malloc(sizeof(internal_pthread_args_t));
        args->rank = r;
        args->entry = call_parallel;
        args->params = params;
        int e = pthread_create(&parop_thread_tab[r], NULL,
            (void * (*)(void *))(internal_pthread_thread), args);
        if (e!=0) machine_fatal("Error in pthread_create()");
    }

    call_parallel(params);

    for (r=1; r<parop_max_rank; r++) {
        int e = pthread_join(parop_thread_tab[r], NULL);
        if (e!=0) machine_fatal("Error in pthread_join()");
    }
    return 0;
}





#endif // _PAROP_PTHREAD_H
