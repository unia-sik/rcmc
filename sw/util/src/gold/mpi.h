#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "fgmp_addr.h"
#include "mpi_transfer.h"
    
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
// typedef int MPI_Request;
typedef int MPI_Op;
typedef fgmp_info_t mpi_communicator_t;
typedef mpi_communicator_t* MPI_Comm;

extern mpi_communicator_t mpi_comm_world;
#define MPI_COMM_WORLD (&mpi_comm_world)
#define MPI_ALIGN  __attribute__ ((aligned (8)))
typedef struct {
    void* buf;                   // initial address of receive buffer
    int count;                   // maximum number of elements in receive buffer
    int source;                  // source-rank
    int tag;                     // tag
    MPI_Datatype datatype;       // datatype of each receive buffer element
} MPI_Request;


typedef struct {
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
    int len; // in bytes
} MPI_Status;
/*
typedef struct {
    int16_t     size; // number of processes
    int16_t     cids[1];
} mpi_group_t;
typedef mpi_group_t* MPI_Group;*/

// typedef struct {
//     fgmp_info_t info;
// }  mpi_communicator_t;
// typedef mpi_communicator_t* MPI_Comm;





static inline uint64_t sizeof_mpi_datatype(MPI_Datatype datatype)
{
    return datatype & 0xff;
}

int MPI_Init(
    int* argc,
    char** *argv
);

mpi_communicator_t MPI_New_Comm(
    const uint32_t root,         // The root of the new rectangle (root = lower left corner)
    const uint32_t width,        // width of the rectangle
    const uint32_t height,       // height of the rectangle
    MPI_Comm comm                // current communicator
);

// Performs a blocking send
int MPI_Send(
    const void* buf,             // initial address of send buffer
    int count,                   // number of elements in send buffer (nonnegative integer)
    MPI_Datatype datatype,       // datatype of each send buffer element (handle)
    int dest,                    // rank of destination
    int tag,                     // message tag
    MPI_Comm comm                // communicator
);

// Blocking receive for a message
int MPI_Recv(
    void* buf,                   // initial address of receive buffer
    int count,                   // maximum number of elements in receive buffer
    MPI_Datatype datatype,       // datatype of each receive buffer element
    int source,                  // rank of source
    int tag,                     // message tag
    MPI_Comm comm,               // communicator
    MPI_Status* status           // status object
);

// Gets the number of "top level" elements
int MPI_Get_count(
    const MPI_Status* status,    // return status of receive operation (Status)
    MPI_Datatype datatype,       // datatype of each receive buffer element (handle)
    int* count                   // number of received elements (integer)
);

// Begins a nonblocking receive
int MPI_Irecv(
    void* buf,                   // initial address of receive buffer
    int count,                   // maximum number of elements in receive buffer
    MPI_Datatype datatype,       // datatype of each receive buffer element
    int source,                  // rank of source
    int tag,                     // message tag
    MPI_Comm comm,               // communicator
    MPI_Request* request         // communication request
);

// Waits for an MPI request to complete
int MPI_Wait(
    MPI_Request* request,        // request
    MPI_Status* status           // status object (Status). May be MPI_STATUS_IGNORE (= NULL for now)
);

// Sends and receives a message
int MPI_Sendrecv(
    const void* sendbuf,         // initial address of send buffer
    int sendcount,               // number of elements in send buffer
    MPI_Datatype sendtype,       // type of elements in send buffer (handle)
    int dest,                    // rank of destination
    int sendtag,                 // send tag
    void* recvbuf,               // initial address of receive buffer
    int recvcount,               // number of elements in receive buffer
    MPI_Datatype recvtype,       // type of elements in receive buffer
    int source,                  // rank of source
    int recvtag,                 // receive tag
    MPI_Comm comm,               // communicator
    MPI_Status* status           // status object. This refers to the receive operation.
);

// Sends and receives using a single buffer
int MPI_Sendrecv_replace(
    void* buf,                   // initial address of send and receive buffer
    int count,                   // number of elements in send and receive buffer
    MPI_Datatype datatype,       // type of elements in send and receive buffer
    int dest,                    // rank of destination 
    int sendtag,                 // send message tag
    int source,                  // rank of source 
    int recvtag,                 // receive message tag
    MPI_Comm comm,               // communicator
    MPI_Status* status           // status object
);

// Blocks until all processes in the communicator have reached this routine. 
int MPI_Barrier(
    MPI_Comm comm                // communicator
);

// Broadcasts a message from the process with rank "root" to all other processes of the communicator 
int MPI_Bcast(
    void* buffer,                // starting address of buffer
    int count,                   // number of entries in buffer
    MPI_Datatype datatype,       // data type of buffer
    int root,                    // rank of broadcast root
    MPI_Comm comm                // communicator
);

// Gathers together values from a group of processes 
int MPI_Gather(
    const void* sendbuf,         // starting address of send buffer       
    int sendcount,               // number of elements in send buffer 
    MPI_Datatype sendtype,       // data type of send buffer elements 
    void* recvbuf,               // address of receive buffer (significant NOT only at root) 
    int recvcount,               // number of elements for any single receive (significant NOT only at root) 
    MPI_Datatype recvtype,       // data type of recv buffer elements (significant NOT only at root)
    int root,                    // rank of receiving process 
    MPI_Comm comm                // communicator
);

// Gathers into specified locations from all processes in a group 
int MPI_Gatherv(
    const void* sendbuf,         // starting address of send buffer
    int sendcount,               // number of elements in send buffer
    MPI_Datatype sendtype,       // data type of send buffer elements
    void* recvbuf,               // address of receive buffer (significant only at root) 
    const int* recvcounts,       // integer array (of length group size) containing the number of elements that are received from each process (significant only at root) 
    const int* displs,           // integer array (of length group size). Entry i specifies the displacement relative to recvbuf at which to place the incoming data from process i (significant only at root) 
    MPI_Datatype recvtype,       // data type of recv buffer elements (significant only at root) 
    int root,                    // rank of receiving process 
    MPI_Comm comm                // communicator
);

// Sends data from one process to all other processes in a communicator 
int MPI_Scatter(
    const void* sendbuf,         // address of send buffer (significant only at root) 
    int sendcount,               // number of elements sent to each process (significant only at root) 
    MPI_Datatype sendtype,       // data type of send buffer elements (significant only at root) 
    void* recvbuf,               // address of receive buffer 
    int recvcount,               // number of elements in receive buffer
    MPI_Datatype recvtype,       // data type of receive buffer elements
    int root,                    // rank of sending process
    MPI_Comm comm                // communicator
);

// Scatters a buffer in parts to all processes in a communicator 
int MPI_Scatterv(
    const void* sendbuf,         // address of send buffer (significant only at root) 
    const int* sendcounts,       // integer array (of length group size) specifying the number of elements to send to each processor 
    const int* sdispls,          // integer array (of length group size). Entry i specifies the displacement (relative to sendbuf from which to take the outgoing data to process i
    MPI_Datatype sendtype,       // data type of send buffer elements
    void* recvbuf,               // address of receive buffer
    int recvcount,               // number of elements in receive buffer
    MPI_Datatype recvtype,       // data type of receive buffer elements
    int root,                    // rank of sending process
    MPI_Comm comm                // communicator
);

// Sends data from all to all processes 
int MPI_Alltoall(
    const void* sendbuf,         // starting address of send buffer
    int sendcount,               // number of elements to send to each process
    MPI_Datatype sendtype,       // data type of send buffer elements
    void* recvbuf,               // address of receive buffer 
    int recvcount,               // number of elements received from any process
    MPI_Datatype recvtype,       // data type of receive buffer elements
    MPI_Comm comm                // communicator
);

// Sends data from all to all processes; each process may send a different amount of data and provide displacements for the input and output data. 
int MPI_Alltoallv(
    const void* sendbuf,         // starting address of send buffer
    const int* sendcounts,       // integer array equal to the group size specifying the number of elements to send to each processor 
    const int* sdispls,          // integer array (of length group size). Entry j specifies the displacement (relative to sendbuf from which to take the outgoing data destined for process j
    MPI_Datatype sendtype,       // data type of send buffer elements
    void* recvbuf,               // address of receive buffer
    const int* recvcounts,       // integer array equal to the group size specifying the maximum number of elements that can be received from each processor 
    const int* rdispls,          // integer array (of length group size). Entry i specifies the displacement (relative to recvbuf at which to place the incoming data from process i
    MPI_Datatype recvtype,       // data type of receive buffer elements (handle) 
    MPI_Comm comm                // communicator
);

// Reduces values on all processes to a single value 
int MPI_Reduce(
    const void* sendbuf,         // address of send buffer
    void* recvbuf,               // address of receive buffer (significant only at root) 
    int count,                   // number of elements in send buffer
    MPI_Datatype datatype,       // data type of elements of send buffer 
    MPI_Op op,                   // reduce operation
    int root,                    // rank of root process 
    MPI_Comm comm                // communicator
);

// Combines values from all processes and distributes the result back to all processes 
int MPI_Allreduce(
    const void* sendbuf,         // starting address of send buffer
    void* recvbuf,               // starting address of receive buffer
    int count,                   // number of elements in send buffer 
    MPI_Datatype datatype,       // data type of elements of send buffer
    MPI_Op op,                   // operation
    MPI_Comm comm                // communicator
);

// Gathers data from all tasks and distribute the combined data to all tasks 
int MPI_Allgather(
    const void *sendbuf,         // starting address of send buffer
    int sendcount,               // number of elements in send buffer
    MPI_Datatype sendtype,       // data type of send buffer elements
    void *recvbuf,               // address of receive buffer
    int recvcount,               // number of elements received from any process
    MPI_Datatype recvtype,       // data type of receive buffer elements
    MPI_Comm comm                // communicator
);

// // Produces a group by reordering an existing group and taking only listed members 
// int MPI_Group_incl(
//     MPI_Group group,             // group
//     int n,                       // number of elements in array ranks (and size of newgroup ) 
//     const int ranks[],           // ranks of processes in group to appear in newgroup (array of integers) 
//     MPI_Group* newgroup          // new group derived from above, in the order defined by ranks
// );
// 
// // Returns the rank of this process in the given group 
// int MPI_Group_rank(
//     MPI_Group group,             // group
//     int* rank                    // rank of the calling process in group, or MPI_UNDEFINED if the process is not a member 
// );
// 
// // Frees a group 
// int MPI_Group_free(
//     MPI_Group* group             // group to free
// );
// 
// // Creates a new communicator 
// int MPI_Comm_create(
//     MPI_Comm comm,               // communicator
//     MPI_Group group,             // group, which is a subset of the group of comm
//     MPI_Comm* newcomm            // new communicator
// );
// 
// // Marks the communicator object for deallocation 
// int MPI_Comm_free(
//     MPI_Comm* comm               // Communicator to be destroyed
// );
// 
// // Determines the size of the group associated with a communicator 
// static inline int MPI_Comm_size(
//     MPI_Comm comm,               // communicator
//     int* size                    // number of processes in the group of comm
// )
// {
//     *size = comm->group.size;
//     return MPI_SUCCESS;
// }
// 
// // Determines the rank of the calling process in the communicator 
// static inline int MPI_Comm_rank(
//     MPI_Comm comm,               // communicator
//     int* rank                    // rank of the calling process in the group of comm
// )
// {
//     *rank = comm->info.rank;
//     return MPI_SUCCESS;
// }


// // Accesses the group associated with given communicator 
// static inline int MPI_Comm_group(
//     MPI_Comm comm,               // Communicator
//     MPI_Group* group             // Group in communicator
// )
// {
//     *group = &comm->group;
//     return MPI_SUCCESS;
// }

// // Returns the size of a group 
// static inline int MPI_Group_size(
//     MPI_Group group,             // group
//     int* size                    // number of processes in the group
// )
// {
//     *size = group->size;
//     return MPI_SUCCESS;
// }

// Terminates MPI execution environment 
static inline int MPI_Finalize()
{
    return MPI_SUCCESS;
}


// Terminates MPI execution environment 
static inline int MPI_Abort(
    MPI_Comm comm,               // communicator of tasks to abort 
    int errorcode                // error code to return to invoking environment 
)
{
    return MPI_SUCCESS;
}




#ifdef __cplusplus
}
#endif

