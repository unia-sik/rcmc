#include "mpi_internal.h"

void MPI_Barrier_send_row(MPI_Comm comm)
{    
    int x = pnoo_addr_x(comm->address);
    int y = pnoo_addr_y(comm->address);
    
    for (int i = 1; i < comm->width; i *= 2) {
        int src = pnoo_addr_gen((x + comm->width - i) % comm->width, y) + comm->root;
        int dest = pnoo_addr_gen((x + i) % comm->width, y) + comm->root;
        pnoo_srdy(src);      
        
        pnoo_bnr(dest);
        pnoo_bsf();        
        pnoo_snd(dest, 0xAC);
        pnoo_bre();
        pnoo_rcvp();
    }    
}

void MPI_Barrier_send_col(MPI_Comm comm)
{    
    int x = pnoo_addr_x(comm->address);
    int y = pnoo_addr_y(comm->address);
    
    for (int i = 1; i < comm->height; i *= 2) {        
        int src = pnoo_addr_gen(x, ((y + comm->height - i) % comm->height)) + comm->root;
        int dest = pnoo_addr_gen(x, ((y + i) % comm->height)) + comm->root;
        pnoo_srdy(src);      
        
        pnoo_bnr(dest);
        pnoo_bsf();
        pnoo_snd(dest, 0xAC);
        pnoo_bre();
        pnoo_rcvp();
    }    
}


// Blocks until all processes in the communicator have reached this routine. 
int MPI_Barrier(
    MPI_Comm comm                // communicator
)
{    
    pnoo_srdy(comm->address + comm->root);
    pnoo_ibrr(comm->root, pnoo_addr_gen(pnoo_addr_x(comm->root) + comm->width - 1, pnoo_addr_y(comm->root) + comm->height - 1));
    pnoo_bbrr();
    return MPI_SUCCESS;
    
    MPI_Barrier_send_row(comm);
    MPI_Barrier_send_col(comm);
    
    return MPI_SUCCESS;
}
