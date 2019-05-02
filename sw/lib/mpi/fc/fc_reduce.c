#include "fc_internal.h"
#include <string.h>

#define MAX_MIN_SUM_PROD_BIN(type) \
{ \
    type *p=d, *q=s; \
        switch (op) { \
        case MPI_MAX:   if (*p < *q) *p = *q; break; \
        case MPI_MIN:   if (*p > *q) *p = *q; break; \
        case MPI_SUM:   *p += *q; break; \
        case MPI_PROD:  *p *= *q; break; \
        case MPI_BAND:  *p &= *q; break; \
        case MPI_BOR:  *p |= *q; break; \
        case MPI_BXOR:  *p ^= *q; break; \
        } \
    break; \
}

#define MAX_MIN_SUM_PROD(type) \
{ \
    type *p=d, *q=s; \
        switch (op) { \
        case MPI_MAX:   if (*p < *q) *p = *q; break; \
        case MPI_MIN:   if (*p > *q) *p = *q; break; \
        case MPI_SUM:   *p += *q; break; \
        case MPI_PROD:  *p *= *q; break; \
        } \
    break; \
}

static void mpi_fc_Reduce_switch_op(void *d, void *s, MPI_Datatype type, MPI_Op op)
{
    switch (type) {
    case MPI_INT8_T:   MAX_MIN_SUM_PROD_BIN(int8_t)
    case MPI_INT16_T:  MAX_MIN_SUM_PROD_BIN(int16_t)
    case MPI_INT32_T:  MAX_MIN_SUM_PROD_BIN(int32_t)
    case MPI_INT64_T:  MAX_MIN_SUM_PROD_BIN(int64_t)
    case MPI_UINT8_T:  MAX_MIN_SUM_PROD_BIN(uint8_t)
    case MPI_UINT16_T: MAX_MIN_SUM_PROD_BIN(uint16_t)
    case MPI_UINT32_T: MAX_MIN_SUM_PROD_BIN(uint32_t)
    case MPI_UINT64_T: MAX_MIN_SUM_PROD_BIN(uint64_t)
    case MPI_FLOAT:    MAX_MIN_SUM_PROD(float)
    case MPI_DOUBLE:   MAX_MIN_SUM_PROD(double)
//     default: //assert(!"MPI_Datatype not supported for arithmetic reduction");
    }
}

void mpi_fc_Reduce_calc_op(void* data, int n, MPI_Datatype type, MPI_Op op) {
    for (int i = 0; i < n; i++) {
        mpi_fc_Reduce_switch_op(data + i * sizeof_mpi_datatype(type), data + (n + i) * sizeof_mpi_datatype(type), type, op);
    }
}

int mpi_fc_Reduce_fix_rank(int rank, int root, int size) {
    int result = rank - root;
    if (result < 0) {
        result += size;
    }
    return result;
}


#define MAP_RANK(x) (pnoo_addr_from_rank(((rank+comm->size+(x))%comm->size), comm) + comm->root)

// Reduces values on all processes to a single value 
int mpi_fc_Reduce(
    const void* sendbuf,         // address of send buffer
    void* recvbuf,               // address of receive buffer (significant only at root) 
    int count,                   // number of elements in send buffer
    MPI_Datatype datatype,       // data type of elements of send buffer 
    MPI_Op op,                   // reduce operation
    int root,                    // rank of root process 
    MPI_Comm comm                // communicator
)
{
    mpi_fc_Barrier(comm);
    int len = count * sizeof_mpi_datatype(datatype);
    char buffer[2 * len];

    int rank = comm->rank;
    int redrank = rank-root;
    if (redrank<0) redrank += comm->size;

    if ((redrank & 1) == 0 && redrank < comm->size - 1) {
        pnoo_srdy(MAP_RANK(+1));
    }

    memcpy(buffer, sendbuf, len);

    for (int i = 1; i < comm->size; i = i << 1) {
        if ((redrank & i) != 0) {
            mpi_transfer_send(MAP_RANK(-i), len, (void*)buffer);
            return MPI_SUCCESS;
        } else {
            if (redrank + i < comm->size) {
                mpi_transfer_recv(MAP_RANK(+1), len, buffer + len);
                if (redrank + (i << 1) < comm->size) {
                    pnoo_srdy(MAP_RANK(i<<1));
                }
                mpi_fc_Reduce_calc_op(buffer, count, datatype, op);
            }
        }
    }
    memcpy(recvbuf, buffer, len);
    return MPI_SUCCESS;
}
