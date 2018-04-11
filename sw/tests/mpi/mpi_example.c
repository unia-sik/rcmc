 /*
  "Hello World" MPI Test Program
 */
#include <stdio.h>
#include <string.h>
#include <mpi.h>

#ifdef QUIET
#define printf(...)
#endif

#define BUFSIZE 128

static int mpiw_max_rank = 0;
static int mpiw_rank = 0;

int mpiw_init(int *argc, char ***argv)
{
    MPI_Init(argc, argv);
    MPI_Comm_size(MPI_COMM_WORLD, &mpiw_max_rank);
    MPI_Comm_rank(MPI_COMM_WORLD, &mpiw_rank);
    return 1;
}

int mpiw_send(void *buf, int count, int dest)
{
    MPI_Send(buf, count, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
    return 1;
}

int mpiw_recv(void *buf, int max, int source)
{
    int count;
    MPI_Status status;
    MPI_Recv(buf, max, MPI_CHAR, source, 0, MPI_COMM_WORLD, &status);
    MPI_Get_count(&status, MPI_CHAR, &count);
    return count;
}

int mpiw_finalize()
{
    return MPI_Finalize();
}




 
int main(int argc, char *argv[])
{
   char idstr[BUFSIZE];
   char buff[BUFSIZE];
   int i;
 
   mpiw_init(&argc,&argv);
 
   /* At this point, all programs are running equivalently, the rank distinguishes
      the roles of the programs in the SPMD model, with rank 0 often used specially... */
   if(mpiw_rank == 0)
   {
     printf("%d: We have %d processors\n", mpiw_rank, mpiw_max_rank);
     for(i=1;i<mpiw_max_rank;i++)
     {
       sprintf(buff, "Hello %d! ", i);
//       strcpy(buff, "Hello X! "); buff[6] = i + '0';
       mpiw_send(buff, BUFSIZE, i);
     }
     for(i=1;i<mpiw_max_rank;i++)
     {
       mpiw_recv(buff, BUFSIZE, i);
       printf("%d: %s\n", mpiw_rank, buff);
     }
   }
   else
   {
     /* receive from rank 0: */
     mpiw_recv(buff, BUFSIZE, 0);
     sprintf(idstr, "Processor %d ", mpiw_rank);
//     strcpy(idstr, "Processor X "); idstr[10] = mpiw_rank + '0';
     strcat(buff, idstr);
     strcat(buff, "reporting for duty");
     /* send to rank 0: */
     mpiw_send(buff, BUFSIZE, 0);
     if (mpiw_rank==10) printf("%d: %s", mpiw_rank, buff);
   }
 

/*
    int j;
    for (i=0; i<10000000; i++)
	j = (i*3 ) ^ (j+5);
*/
    printf("%d: finished.\n", mpiw_rank);
 
   mpiw_finalize();
   return 0;
}
