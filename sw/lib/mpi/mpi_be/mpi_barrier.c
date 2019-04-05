#include "mpi_internal.h"

int MPI_Barrier(MPI_Comm comm)
{
    assert(comm != MPI_COMM_NULL);

    int my_rank = comm->rank;
    if (my_rank==0) {
        if (comm->group.size!=1) {
            // send ack to next process
            pimp2_send_flit(cid_from_comm(comm, 1), 0);
            // wait for ack from last process
            flit_t f = pimp2_recv_flit(cid_from_comm(comm, comm->group.size-1));
            assert(f==(comm->group.size-1));
            // tell all
            broadcast_flit(comm, 42);
        }
    } else {
        // wait for ack from previous process
        flit_t f = pimp2_recv_flit(cid_from_comm(comm, my_rank-1));
        assert(f==(my_rank-1));
        // send ack to next process
        pimp2_send_flit(cid_from_comm(comm, (my_rank+1)%comm->group.size), my_rank);
        // wait for broadcast from process 0
        f = pimp2_recv_flit(cid_from_comm(comm, 0));
        assert(f==42);
    }
    return MPI_SUCCESS;
}

/* Alternative, centralised implementation
int MPI_Barrier(MPI_Comm comm)
{
    assert(comm != MPI_COMM_NULL);

    if (comm->rank!=0) {
        cid_t root = cid_from_comm(comm, 0);
        flit_t f = pimp2_recv_flit(root);
        assert(f==1);
        pimp2_send_flit(root, ACK_FLIT);
        f = pimp2_recv_flit(root);
        assert(f==2);
    } else {
        // tell other processes that root is ready
        broadcast_flit(comm, 1);
        wait_for_ack(comm);
        broadcast_flit(comm, 2);
    }
    return MPI_SUCCESS;
}
*/