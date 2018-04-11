#include <mpi.h>
#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef QUIET
#define printf(...)
#endif


#define ROOT 1

int main(int argc,char *argv[])
{
    int max_rank;
    int rank;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &max_rank);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int64_t sbuf = rank*10+1;
    int64_t max, min, sum, prod;
    MPI_Reduce(&sbuf, &max, 1, MPI_INT64_T, MPI_MAX, ROOT, MPI_COMM_WORLD);
    MPI_Reduce(&sbuf, &min, 1, MPI_INT64_T, MPI_MIN, ROOT, MPI_COMM_WORLD);
    MPI_Reduce(&sbuf, &sum, 1, MPI_INT64_T, MPI_SUM, ROOT, MPI_COMM_WORLD);
    MPI_Reduce(&sbuf, &prod, 1, MPI_INT64_T, MPI_PROD, ROOT, MPI_COMM_WORLD);

    if (rank==ROOT) {
        printf("max=%"PRId64" min=%"PRId64" sum=%"PRId64" prod=%"PRId64"\n",
            max, min, sum, prod);
    }

    MPI_Finalize();
    return 0;
}

