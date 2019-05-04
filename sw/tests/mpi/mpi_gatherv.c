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

#ifndef SAT_SIZE
#define SAT_SIZE 20  // 5 for shorter runtime
#endif

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

    if (my_rank == 0) {
        printf("start test of gatherv\n");
    }
    
    int i;
    int numSatellites = max_rank; //4;
    int satellite[SAT_SIZE];
    int central[SAT_SIZE*numSatellites];
    int displs[SAT_SIZE];
    int cenSize[SAT_SIZE];;



    for(i=0; i<SAT_SIZE; i++) {
        satellite[i] = (100 * my_rank) + i;
    }

    int participants[max_rank];
    for(i=0; i<max_rank; i++) {
        participants[i] = i;
    }
    int numParticipants = max_rank;
    if (my_rank == 0) {
        for(i=0; i<numSatellites; i++) {
            displs[i] = i * SAT_SIZE;
            cenSize[i] = SAT_SIZE;

        }
    }

//    MPI_Comm subComm;
//    createSubCommunicator( numParticipants, participants, &subComm);
//    if (subComm != MPI_COMM_NULL) {
        MPI_Gatherv(satellite, SAT_SIZE, MPI_INT,
            central, cenSize, displs, MPI_INT, 0, MPI_COMM_WORLD);
//        MPI_Comm_free(&subComm);
//    }

    if (my_rank == 0) {
        printf("Results:\n");
        for(i=0; i<numSatellites*SAT_SIZE; i++) {
            printf("%6d.0 ", central[i]);
        }
        printf("\n\nfinish test of gatherv\n");
    }

    MPI_Finalize();
    return 0;
}
