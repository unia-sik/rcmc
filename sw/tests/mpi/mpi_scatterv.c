/*
 * =====================================================================================
 *
 *       Filename:  gatherv.c
 *
 *    Description:  tests the functional correctness of mpi gatherv implementation
 *
 *        Version:  1.0
 *        Created:  24.09.2015 15:37:31
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Alexander Stegmeier
 *   Organization:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

#ifdef QUIET
#define printf(...)
#endif

#define satSize 20


int createSubCommunicator(int numParticipants, int *participantWorldRanks, MPI_Comm *subComm) {
    int err_value;
    MPI_Group __parop_group_world, __parop_subGroup;
    MPI_Comm_group(MPI_COMM_WORLD, &__parop_group_world);
    MPI_Group_incl(__parop_group_world, numParticipants, participantWorldRanks, &__parop_subGroup);
    err_value = MPI_Comm_create(MPI_COMM_WORLD, __parop_subGroup, subComm);
    if (__parop_subGroup != MPI_GROUP_NULL) {
    MPI_Group_free(&__parop_subGroup);
    }
    if (__parop_group_world != MPI_GROUP_NULL) {
    MPI_Group_free(&__parop_group_world);
    }
    return err_value;
}

int main(int argc, char *argv[]) {

    int my_rank;
    int max_rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    printf("start test of gatherv\n");

    int i;
    int numSatellites = max_rank;
    int satellite[satSize];
    int central[satSize*numSatellites];
    int displs[satSize];
    int cenSize[satSize];


    int participants[max_rank];


    for(i=0; i<max_rank; i++) {
        participants[i] = i;
    }
    if (my_rank == 0) {
        for(i=0; i<numSatellites*satSize; i++) {
            central[i] = i + 0.1;
        }

        for(i=0; i<numSatellites; i++) {
            displs[i] = i * satSize;
            cenSize[i] = satSize;

        }
    }

    MPI_Comm subComm;
    createSubCommunicator( max_rank, participants, &subComm);
    if (subComm != MPI_COMM_NULL) {
        MPI_Scatterv(central, cenSize, displs, MPI_INT, satellite, satSize, MPI_INT, 0, subComm);
        MPI_Comm_free(&subComm);
    }

    printf("Results:\n");
    for(i=0; i<satSize; i++) {
        printf("%6d.1 ", satellite[i]);
    }
    printf("\n");
    printf("\n");

    printf("finish test of gatherv\n");

    MPI_Finalize();
    return 0;
}
