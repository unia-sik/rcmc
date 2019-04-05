#include "mpi_internal.h"
#include <stdint.h>
#include <string.h>


#define MAX_MIN_SUM_PROD(type) \
{ \
    type *p=d, *q=s; \
    while (count != 0) { \
        switch (op) { \
        case MPI_MAX:   if (*p < *q) *p = *q; break; \
        case MPI_MIN:   if (*p > *q) *p = *q; break; \
        case MPI_SUM:   *p += *q; break; \
        case MPI_PROD:  *p *= *q; break; \
        } \
        p++; q++; count--;\
    } \
    break; \
}


static void arith_op(void *d, void *s, int count, MPI_Datatype type, MPI_Op op)
{
    switch (type) {
    case MPI_INT8_T:   MAX_MIN_SUM_PROD(int8_t)
    case MPI_INT16_T:  MAX_MIN_SUM_PROD(int16_t)
    case MPI_INT32_T:  MAX_MIN_SUM_PROD(int32_t)
    case MPI_INT64_T:  MAX_MIN_SUM_PROD(int64_t)
    case MPI_UINT8_T:  MAX_MIN_SUM_PROD(uint8_t)
    case MPI_UINT16_T: MAX_MIN_SUM_PROD(uint16_t)
    case MPI_UINT32_T: MAX_MIN_SUM_PROD(uint32_t)
    case MPI_UINT64_T: MAX_MIN_SUM_PROD(uint64_t)
    case MPI_FLOAT:    MAX_MIN_SUM_PROD(float)
    case MPI_DOUBLE:   MAX_MIN_SUM_PROD(double)
    default: assert(!"MPI_Datatype not supported for arithmetic reduction");
    }
}


static void bitwise_op(flit_t *d, flit_t s, MPI_Op op)
{
    switch (op) {
    case MPI_BAND: *d &= s; break;
    case MPI_BOR:  *d |= s; break;
    case MPI_BXOR: *d ^= s; break;
    default: assert(!"MPI_Op not supported");
    }
}


static void _mpi_reduce(const void *sbuf, void *rbuf, int count, MPI_Datatype type,
    MPI_Op op, int root, MPI_Comm comm) 
{
    cid_t i, n = comm->group.size;
    cid_t max_cid = mpi_comm_world.group.size;
    unsigned len = count * sizeof_mpi_datatype(type);
    unsigned num = (len+sizeof(flit_t)-1) / sizeof(flit_t); // flits per core
    flit_t buf[n*num];
    flit_t *ptrs[max_cid];
    unsigned lens[max_cid];

    // gather
    if (comm==MPI_COMM_WORLD) {
        for (i=0; i<max_cid; i++) {
            ptrs[i] = buf + i*num;
            lens[i] = len;
        }
        lens[root] = 0;
    } else {
        for (i=0; i<max_cid; i++)
            lens[i] = 0;
        for (i=0; i<n; i++) {
            if (i!=root) {
                ptrs[comm->group.cids[i]] = buf + i*num;
                lens[comm->group.cids[i]] = len;
            }
        }
    }
    gather(ptrs, lens, (n-1)*len);

    // reduce
    memcpy(rbuf, sbuf, num*sizeof(flit_t));
    for(i=0; i<n; i++) {
        if (i!=root) {
            if (_IS_ARITH_OP(op)) {
                arith_op(rbuf, buf+i*num, count, type, op);
            } else {
                unsigned j;
                for (j=0; j<num; j++)
                    bitwise_op((flit_t *)rbuf + j, buf[i*num+j], op);
            }
        }
    }
}


int MPI_Reduce(const void *sbuf, void *rbuf, int count, MPI_Datatype type, 
    MPI_Op op, int root, MPI_Comm comm)
{
    unsigned len = count * sizeof_mpi_datatype(type);

    if (comm->rank==root) {
        _mpi_reduce(sbuf, rbuf, count, type, op, root, comm);
    } else {
        cid_t root_cid = cid_from_comm(comm, root);
        flit_t f = pimp2_recv_flit(root_cid); // wait until root is ready
        assert(f==ACK_FLIT);
        send_raw(root_cid, len, sbuf);
    }
    return MPI_SUCCESS;

}


int MPI_Allreduce(const void *sbuf, void *rbuf, int count, MPI_Datatype type, 
    MPI_Op op, MPI_Comm comm)
{
    unsigned len = count * sizeof_mpi_datatype(type);

    if (comm->rank==0) {
        _mpi_reduce(sbuf, rbuf, count, type, op, 0, comm);

        // scatter
        int i, n = comm->group.size;
        for (i=1; i<n; i++) {
            send_raw(cid_from_comm(comm, i), len, rbuf);
        }

    } else {
        cid_t root_cid = cid_from_comm(comm, 0);
        flit_t f = pimp2_recv_flit(root_cid); // wait until root is ready
        assert(f==ACK_FLIT);
        send_raw(root_cid, len, sbuf);
        recv_raw(root_cid, len, rbuf); // scatter
    }
    return MPI_SUCCESS;

}
