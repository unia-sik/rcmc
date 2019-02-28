#include "mpi.h"
#include "debug.h"

void MPI_Barrier_send_row(MPI_Comm comm)
{    
    int x = fgmp_addr_x(comm->address);
    int y = fgmp_addr_y(comm->address);
    
    for (int i = 1; i < comm->width; i *= 2) {
        int src = fgmp_addr_gen((x + comm->width - i) % comm->width, y) + comm->root;
        int dest = fgmp_addr_gen((x + i) % comm->width, y) + comm->root;
        fgmp_srdy(src);      
        
        fgmp_bnr(dest);
        fgmp_bsf();        
        fgmp_snd(dest, 0xAC);
        fgmp_bre();
        fgmp_rcvp();
    }    
}

void MPI_Barrier_send_col(MPI_Comm comm)
{    
    int x = fgmp_addr_x(comm->address);
    int y = fgmp_addr_y(comm->address);
    
    for (int i = 1; i < comm->height; i *= 2) {        
        int src = fgmp_addr_gen(x, ((y + comm->height - i) % comm->height)) + comm->root;
        int dest = fgmp_addr_gen(x, ((y + i) % comm->height)) + comm->root;
        fgmp_srdy(src);      
        
        fgmp_bnr(dest);
        fgmp_bsf();
        fgmp_snd(dest, 0xAC);
        fgmp_bre();
        fgmp_rcvp();
    }    
}


// Blocks until all processes in the communicator have reached this routine. 
int MPI_Barrier(
    MPI_Comm comm                // communicator
)
{    
    fgmp_srdy(comm->address + comm->root);
    fgmp_ibrr(comm->root, fgmp_addr_gen(fgmp_addr_x(comm->root) + comm->width - 1, fgmp_addr_y(comm->root) + comm->height - 1));
    fgmp_bbrr();
    return MPI_SUCCESS;
    
    MPI_Barrier_send_row(comm);
    MPI_Barrier_send_col(comm);
    
    return MPI_SUCCESS;
}
