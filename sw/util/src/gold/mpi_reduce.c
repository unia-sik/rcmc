#include "mpi.h"
#include "fgmp_block.h"

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

static void MPI_Reduce_switch_op(void *d, void *s, MPI_Datatype type, MPI_Op op)
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

void MPI_Reduce_calc_op(void* data, int n, MPI_Datatype type, MPI_Op op) {
    for (int i = 0; i < n; i++) {
        MPI_Reduce_switch_op(data + i * sizeof_mpi_datatype(type), data + (n + i) * sizeof_mpi_datatype(type), type, op);
    }
}

int MPI_Reduce_fix_rank(int rank, int root, int size) {
    int result = rank - root;
    if (result < 0) {
        result += size;
    }
    
    return result;
}

// Reduces values on all processes to a single value 
int MPI_Reduce(
    const void* sendbuf,         // address of send buffer
    void* recvbuf,               // address of receive buffer (significant only at root) 
    int count,                   // number of elements in send buffer
    MPI_Datatype datatype,       // data type of elements of send buffer 
    MPI_Op op,                   // reduce operation
    int root,                    // rank of root process 
    MPI_Comm comm                // communicator
)
{
    MPI_Barrier(comm);
    int local_size = count * sizeof_mpi_datatype(datatype);    
    char buffer[2 * local_size];
    
    if ((MPI_Reduce_fix_rank(comm->rank, root, comm->size) & 1) == 0 && MPI_Reduce_fix_rank(comm->rank, root, comm->size) < comm->size - 1) {
        fgmp_srdy(fgmp_addr_from_rank(MPI_Reduce_fix_rank(comm->rank, root, comm->size) + 1, comm) + comm->root);        
    }  
    
    for (int k = 0; k < local_size; k++) {
        ((char*)buffer)[k] = ((char*)sendbuf)[k];
    }
                    
    for (int i = 1; i < comm->size; i = i << 1) {
        if ((MPI_Reduce_fix_rank(comm->rank, root, comm->size) & i) != 0) {
            mpi_transfer_send(fgmp_addr_from_rank(MPI_Reduce_fix_rank(comm->rank, root, comm->size) - i, comm) + comm->root, local_size, (void*)buffer);
            return MPI_SUCCESS;
        } else {
            if (MPI_Reduce_fix_rank(comm->rank, root, comm->size) + i < comm->size) {
                mpi_transfer_recv(fgmp_addr_from_rank(MPI_Reduce_fix_rank(comm->rank, root, comm->size) + i, comm) + comm->root, local_size, buffer + local_size);                
                
                if (MPI_Reduce_fix_rank(comm->rank, root, comm->size) + (i << 1) < comm->size) {
                    fgmp_srdy(fgmp_addr_from_rank(MPI_Reduce_fix_rank(comm->rank, root, comm->size) + (i << 1), comm) + comm->root);                
                }
                
                MPI_Reduce_calc_op(buffer, count, datatype, op);  
            }
        }
    }
    
    for (int k = 0; k < local_size; k++) {
        ((char*)recvbuf)[k] = ((char*)buffer)[k];
    }
    
    return MPI_SUCCESS;
}
