#ifndef _MPI_H
#define _MPI_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>

#ifdef RCMC_MPILIB_pnoo
#include "pnoo.h"
#endif


// datatypes
// lowest 8 bits are equal to the size in bytes
#define MPI_INT8_T      0x001
#define MPI_INT16_T     0x002
#define MPI_INT32_T     0x004
#define MPI_INT64_T     0x008
#define MPI_UINT8_T     0x101
#define MPI_UINT16_T    0x102
#define MPI_UINT32_T    0x104
#define MPI_UINT64_T    0x108

#define MPI_BYTE        0x001
#define MPI_CHAR        0x001
#define MPI_SHORT       (sizeof(short)) // architecture dependent!
#define MPI_INT         (sizeof(int))   // architecture dependent!
#define MPI_FLOAT       0x204
#define MPI_DOUBLE      0x208


// reduction operations
#define MPI_MAX 0
#define MPI_MIN 1
#define MPI_SUM 2
#define MPI_PROD 3
#define MPI_BAND 5
#define MPI_BOR 7
#define MPI_BXOR 9
#define _IS_ARITH_OP(op)    ((op)<4)


#define MPI_UNDEFINED   (-1)
#define MPI_SUCCESS     0
#define MPI_COMM_NULL   0
#define MPI_GROUP_NULL  0
#define MPI_GROUP_EMPTY (void *)(-1)


typedef int MPI_Datatype;
typedef int MPI_Op;

#ifdef RCMC_MPILIB_pnoo
typedef struct {
    void* buf;                   // initial address of receive buffer
    int count;                   // maximum number of elements in receive buffer
    int source;                  // source-rank
    int tag;                     // tag
    MPI_Datatype datatype;       // datatype of each receive buffer element
} MPI_Request;
#else
typedef int MPI_Request;
#endif

typedef struct {
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
    int len; // in bytes
} MPI_Status;

typedef struct {
    int16_t     size; // number of processes
    int16_t     cids[1];
} mpi_group_t;
typedef mpi_group_t *MPI_Group;

#ifdef RCMC_MPILIB_pnoo
typedef pnoo_info_t mpi_communicator_t;
#else
typedef struct {
    int         rank;
    mpi_group_t group;
}  mpi_communicator_t;
#endif

typedef mpi_communicator_t *MPI_Comm;

extern mpi_communicator_t mpi_comm_world;
#define MPI_COMM_WORLD (&mpi_comm_world)
#define _MPI_GROUP_WORLD (&mpi_comm_world.group)



int MPI_Init(int *argc, char ***argv);
int MPI_Get_count(const MPI_Status *status, MPI_Datatype datatype, int *count);
int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, 
        MPI_Comm comm);
int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int src, int tag,
        MPI_Comm comm, MPI_Status *status);
int MPI_Irecv(void *buf, int count, MPI_Datatype datatype, int src, int tag,
        MPI_Comm comm, MPI_Request *request);
int MPI_Wait(MPI_Request *request, MPI_Status *status);
int MPI_Sendrecv(const void *sbuf, int scount, MPI_Datatype stype, int dest, int stag,
    void *rbuf, int rcount, MPI_Datatype rtype, int source, int rtag,
    MPI_Comm comm, MPI_Status *status);
int MPI_Sendrecv_replace(void *buf, int count, MPI_Datatype datatype, int dest,
    int stag, int source, int rtag, MPI_Comm comm, MPI_Status *status);

int MPI_Barrier(MPI_Comm comm);
int MPI_Bcast(void* buf, int count, MPI_Datatype datatype, int root, 
        MPI_Comm comm);
int MPI_Gather(const void* send_buf, int send_count, MPI_Datatype send_datatype,
        void* receive_buf, int receive_count, MPI_Datatype receive_datatype,
        int root, MPI_Comm comm);
int MPI_Gatherv(const void* sbuf, int scount, MPI_Datatype stype, void* rbuf,
        const int rcounts[], const int rdisps[], MPI_Datatype rtype, int root,
        MPI_Comm comm);
int MPI_Allgather(const void* send_buf, int send_count, MPI_Datatype send_datatype,
        void* receive_buf, int receive_count, MPI_Datatype receive_datatype,
        MPI_Comm comm); // not yet implemented, only to avoid warnings
int MPI_Scatter(const void* send_buf, int send_count, MPI_Datatype send_datatype,
        void* receive_buf, int receive_count, MPI_Datatype receive_datatype,
        int root, MPI_Comm comm);
int MPI_Scatterv(const void *sbuf, const int scounts[], const int sdisps[],
        MPI_Datatype stype, void *rbuf, int rcount, MPI_Datatype rtype, int root,
        MPI_Comm comm) ;
int MPI_Alltoall(const void* send_buf, int send_count, MPI_Datatype send_datatype, 
        void *receive_buf, int receive_count, MPI_Datatype receive_datatype,
        MPI_Comm comm);
int MPI_Alltoallv(const void *sbuf, const int *scounts, const int *sdisps, MPI_Datatype sdtype,
        void *rbuf, const int *rcounts, const int *rdisps, MPI_Datatype rdtype, 
        MPI_Comm comm);
int MPI_Reduce(const void *sbuf, void *rbuf, int count, MPI_Datatype type,
        MPI_Op op, int root, MPI_Comm comm);
int MPI_Allreduce(const void *sbuf, void *rbuf, int count, MPI_Datatype type,
        MPI_Op op, MPI_Comm comm);



// create a new group with the given ranks
int MPI_Group_incl(MPI_Group group, int n, const int ranks[], MPI_Group *new);

// search current process in group
int MPI_Group_rank(MPI_Group group, int *rank);

// free the memory that was allocated for the group
int MPI_Group_free(MPI_Group *group);

// create a new communicator and copy the group list, if the current process is a member of it
int MPI_Comm_create(MPI_Comm comm, MPI_Group group, MPI_Comm *new);

// free the memory that was allocated for the group
int MPI_Comm_free(MPI_Comm *comm);


static inline int MPI_Comm_rank(MPI_Comm comm, int *rank)
{
    *rank = comm->rank;
    return MPI_SUCCESS;
}


#ifdef RCMC_MPILIB_pnoo

static inline int MPI_Comm_size(MPI_Comm comm, int *size)
{
    *size = comm->size;
    return MPI_SUCCESS;
}

mpi_communicator_t MPI_New_Comm(
    const uint32_t root,         // The root of the new rectangle (root = lower left corner)
    const uint32_t width,        // width of the rectangle
    const uint32_t height,       // height of the rectangle
    MPI_Comm comm                // current communicator
);

// TODO: not implemented yet
static inline int MPI_Comm_group(MPI_Comm comm, MPI_Group *group)
{
    return MPI_SUCCESS;
}


#else

static inline int MPI_Comm_size(MPI_Comm comm, int *size)
{
    *size = comm->group.size;
    return MPI_SUCCESS;
}


// get a pointer to the group of the communicator
static inline int MPI_Comm_group(MPI_Comm comm, MPI_Group *group)
{
    *group = &comm->group;
    return MPI_SUCCESS;
}

#endif


static inline int MPI_Group_size(MPI_Group group, int *size)
{
    *size = group->size;
    return MPI_SUCCESS;
}


static inline int MPI_Finalize()
{
    return MPI_SUCCESS;
}


static inline int MPI_Abort(MPI_Comm comm, int errno)
{
    return MPI_SUCCESS;
}




void parop_comm_split_consecutive(MPI_Comm all, unsigned n, MPI_Comm *subset);
MPI_Comm fgmp_def_collection(int *colors);
MPI_Comm fgmp_comm_world(int n);



#ifdef __cplusplus
}
#endif

#endif // !_MPI_H
