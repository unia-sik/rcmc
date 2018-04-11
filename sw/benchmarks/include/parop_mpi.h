#ifndef _PAROP_MPI_H
#define _PAROP_MPI_H

// -----------------------------------------------------------------------------
//   MPI mplementation
// -----------------------------------------------------------------------------

#include <mpi.h>

#define PAROP_LOCAL


int parop_my_rank = 0;
int parop_max_rank = 0;

typedef struct {
    double		data;
} parop_collective_t;

void *parop_memblock[PAROP_MAX_MEMBLOCK];
int parop_directions[PAROP_MAX_DIRECTIONS];



static inline void parop_fatal(char *s)
{
    MPI_Abort(MPI_COMM_WORLD, 1);
    machine_fatal(s);
}







// -----------------------------------------------------------------------------
// memory allocation
// -----------------------------------------------------------------------------



void *parop_def_memblock(int handle, size_t n)
{
    machine_assert(handle < PAROP_MAX_MEMBLOCK);
    double *p = machine_malloc(n);
    if (p==0) parop_fatal("Out of memory");
    parop_memblock[handle] = p;
    return p;
}






// -----------------------------------------------------------------------------
// collectives
// -----------------------------------------------------------------------------


typedef struct {
    MPI_Comm    comm;
} parop_collection_t;

parop_collection_t parop_collections[PAROP_MAX_COLLECTIONS];

// MPI specific
MPI_Comm parop_mpi_comm_world;
    // communicator with all processes that are really used

/*
void parop_def_collection(int handle, int count, int *ranks)
{
    machine_assert(handle < PAROP_MAX_COLLECTIONS);

    MPI_Group world_group;
    MPI_Comm_group(parop_mpi_comm_world, &world_group);
    MPI_Group group;
    MPI_Group_incl(world_group, count, ranks, &group);
    MPI_Comm_create_group(parop_mpi_comm_world, group, 0,
        &parop_collections[handle].comm);
    MPI_Group_free(&group);
    MPI_Group_free(&world_group);
}
*/

void parop_def_collection(int handle, int *colors)
{
    machine_assert(handle < PAROP_MAX_COLLECTIONS);
#ifdef __riscv
    parop_collections[handle].comm = fgmp_def_collection(colors);
#else
    MPI_Comm_split(parop_mpi_comm_world, colors[parop_my_rank], 
        parop_my_rank, &parop_collections[handle].comm);
#endif
}


static inline void parop_barrier(int handle)
{
    MPI_Barrier(parop_collections[handle].comm);
}


static inline void parop_allgather_int64(int handle, int64_t v, int64_t r[])
{
    MPI_Allgather(&v, 1, MPI_INT64_T, r, 1, MPI_INT64_T,
        parop_collections[handle].comm);
}


// so far only for the world communicator, because the determination of
// the root thread is difficult with sub communicators
static inline void parop_gather_int64_world(int64_t v, int64_t r[])
{
    MPI_Gather(&v, 1, MPI_INT64_T, r, 1, MPI_INT64_T,
        0, parop_collections[0].comm);
}


static MPI_Op to_mpi_op[] = {MPI_SUM, MPI_PROD, MPI_MIN, MPI_MAX};

static inline double parop_allreduce_double(int handle, int op, double v)
{
    double r;
    MPI_Allreduce(&v, &r, 1, MPI_DOUBLE, to_mpi_op[op],
        parop_collections[handle].comm);
    return r;
}


static inline void parop_allreduce_multi_double(
    int handle, int op, double r[], int count, double v[])
{
    MPI_Allreduce(v, r, count, MPI_DOUBLE, to_mpi_op[op],
        parop_collections[handle].comm);
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
    parop_directions[handle] = next;
    parop_directions[handle+1] = prev;
}


void parop_forward_begin() {}


void parop_forward_double(int direction, int len,
    int dmb, int di, int dstride,
    int smb, int si, int sstride)
{
    int i;
    double buf[len];
    double *p = (double *)parop_memblock[smb] + si;
    for (i=0; i<len; i++) {
        buf[i] = *p; 
        p+=sstride;
    }

    MPI_Status status;
    int next = parop_directions[direction];
    int prev = parop_directions[direction^1];

    if (prev<0) {
        if (next>=0) {
            MPI_Send(buf, len, MPI_DOUBLE, next, 0, MPI_COMM_WORLD);
        }
    } else {
        if (next<0) {
            MPI_Recv(buf, len, MPI_DOUBLE, prev, 0, MPI_COMM_WORLD, &status);
        } else {
            MPI_Sendrecv_replace(buf, len, MPI_DOUBLE, 
                next, 0, prev, 0, MPI_COMM_WORLD, &status);
        }
        p = (double *)parop_memblock[dmb] + di;
        for (i=0; i<len; i++) {
            *p = buf[i];
            p+=dstride;
        }
    }
}


void parop_forward_int64(int direction, int len,
    int dmb, int di, int dstride,
    int smb, int si, int sstride)
{
    int i;
    int64_t buf[len];
    int64_t *p = (int64_t *)parop_memblock[smb] + si;
    for (i=0; i<len; i++) {
        buf[i] = *p; 
        p+=sstride;
    }

    MPI_Status status;
    int next = parop_directions[direction];
    int prev = parop_directions[direction^1];

    if (prev<0) {
        if (next>=0) {
            MPI_Send(buf, len, MPI_INT64_T, next, 0, MPI_COMM_WORLD);
        }
    } else {
        if (next<0) {
            MPI_Recv(buf, len, MPI_INT64_T, prev, 0, MPI_COMM_WORLD, &status);
        } else {
            MPI_Sendrecv_replace(buf, len, MPI_INT64_T,
                next, 0, prev, 0, MPI_COMM_WORLD, &status);
        }
        p = (int64_t *)parop_memblock[dmb] + di;
        for (i=0; i<len; i++) {
            *p = buf[i];
            p+=dstride;
        }
    }
}


void parop_forward_end() {}




// -----------------------------------------------------------------------------
// broadcast data transfers
// -----------------------------------------------------------------------------

void parop_broadcast_data(int mb, int i, int len, void *data, int senderRank){
  //void *pointerToStartOfData = parop_memblock[parop_my_rank] + i;
  void *pointerToStartOfData = data;
  //printf("core:%d sender:%d\r\n", parop_my_rank, senderRank);
  MPI_Bcast(pointerToStartOfData, len, MPI_BYTE, senderRank, parop_mpi_comm_world);
}



// -----------------------------------------------------------------------------
// main application structure 
// -----------------------------------------------------------------------------


int harness_spmd_main(int argc, char **argv,
    int params_data_size,
    int call_config(int, char **, void *, int),
    void call_parallel(void *))
{
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &parop_my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &parop_max_rank);

    char params[params_data_size];
    if (parop_my_rank == 0) {
        parop_max_rank = call_config(argc, argv, params, parop_max_rank);
    }

    MPI_Bcast(&parop_max_rank, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(params, params_data_size, MPI_CHAR, 0, MPI_COMM_WORLD);

    // splitting must be done before MPI_Finalize()
#ifdef __riscv
    parop_mpi_comm_world = fgmp_comm_world(parop_max_rank);
#else
    MPI_Comm_split(MPI_COMM_WORLD,
        parop_my_rank >= parop_max_rank ? 1 : 0,
        parop_my_rank,
        &parop_mpi_comm_world);
#endif
    parop_collections[0].comm = parop_mpi_comm_world;

    // if there are too many threads, they don't do anything
    if (parop_my_rank < parop_max_rank) {
        call_parallel(params);
    }

    MPI_Finalize();
    return 0;
}






#endif // _PAROP_MPI_H
