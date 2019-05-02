#ifndef _MPI_H
#define _MPI_H


#include <stdint.h>



////////////////////////////////////////////////////////////////////////
// Choose implementation
////////////////////////////////////////////////////////////////////////

#if defined(RCMC_MPI_FLOWCONTROL)
#define _MPI_PREFIX(x) mpi_fc_ ## x

//#elif defined(RCMC_MPI_HANDSHAKE_EXPLICIT)
//# define _MPI_PREFIX(x) mpi_he_ ## x
//#elif defined(RCMC_MPI_HANDSHAKE_IMPLICIT)
//# define _MPI_PREFIX(x) mpi_hi_ ## x

//#elif defined(RCMC_MPI_HANDSHAKE_SOFTWARE)
#else
#define _MPI_PREFIX(x) mpi_hs_ ## x
#endif



#define MPI_Init                _MPI_PREFIX(Init)
#define MPI_Get_count           _MPI_PREFIX(Get_count)
#define MPI_Send                _MPI_PREFIX(Send)
#define MPI_Recv                _MPI_PREFIX(Recv)
#define MPI_Irecv               _MPI_PREFIX(Irecv)
#define MPI_Wait                _MPI_PREFIX(Wait)
#define MPI_Sendrecv            _MPI_PREFIX(Sendrecv)
#define MPI_Sendrecv_replace    _MPI_PREFIX(Sendrecv_replace)
#define MPI_Barrier             _MPI_PREFIX(Barrier)
#define MPI_Bcast               _MPI_PREFIX(Bcast)
#define MPI_Gather              _MPI_PREFIX(Gather)
#define MPI_Gatherv             _MPI_PREFIX(Gatherv)
#define MPI_Allgather           _MPI_PREFIX(Allgather)
#define MPI_Scatter             _MPI_PREFIX(Scatter)
#define MPI_Scatterv            _MPI_PREFIX(Scatterv)
#define MPI_Alltoall            _MPI_PREFIX(Alltoall)
#define MPI_Alltoallv           _MPI_PREFIX(Alltoallv)
#define MPI_Reduce              _MPI_PREFIX(Reduce)
#define MPI_Allreduce           _MPI_PREFIX(Allreduce)
#define MPI_Group_incl          _MPI_PREFIX(Group_incl)
#define MPI_Group_rank          _MPI_PREFIX(Group_rank)
#define MPI_Group_free          _MPI_PREFIX(Group_free)
#define MPI_Comm_create         _MPI_PREFIX(Comm_create)
#define MPI_Comm_free           _MPI_PREFIX(Comm_free)
#define MPI_Comm_rank           _MPI_PREFIX(Comm_rank)
#define MPI_Comm_size           _MPI_PREFIX(Comm_size)
#define MPI_Comm_group          _MPI_PREFIX(Comm_group)
#define MPI_Group_size          _MPI_PREFIX(Group_size)
#define MPI_Finalize            _MPI_PREFIX(Finalize)
#define MPI_Abort               _MPI_PREFIX(Abort)

#define parop_comm_split_consecutive    _MPI_PREFIX(parop_comm_split_consecutive)
#define fgmp_def_collection             _MPI_PREFIX(fgmp_def_collection)
#define fgmp_comm_world                 _MPI_PREFIX(fgmp_comm_world)





////////////////////////////////////////////////////////////////////////
// Data structures
////////////////////////////////////////////////////////////////////////


#if defined(RCMC_MPI_FLOWCONTROL)
#include "pnoo.h"
#include "../mpi/fc/fc_types.h"
#else
#include "../mpi/hs/hs_types.h"
#endif



////////////////////////////////////////////////////////////////////////
// Prototypes
////////////////////////////////////////////////////////////////////////


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


#ifdef RCMC_MPI_FLOWCONTROL

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



#endif // !_MPI_H
