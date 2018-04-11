#include <RCCE.h>
#include <mpi.h>
#include <assert.h>
#include <stddef.h>

int RCCE_init(int *argc, char ***argv)
{
    MPI_Init(argc, argv);
    RCCE_COMM_WORLD.mpicomm = MPI_COMM_WORLD;
    return RCCE_SUCCESS;
}

int RCCE_finalize()
{
    MPI_Finalize();
    return RCCE_SUCCESS;
}

double RCCE_wtime() {
#ifdef __riscv
    int r;
    asm volatile ("rdcycle %0" : "=r"(r));
    return r*0.000000001;
#else
    return 0.0;
#endif
}

int RCCE_ue()
{
    int r;
    MPI_Comm_rank(MPI_COMM_WORLD, &r);
    return r;
}

int RCCE_num_ues()
{
    int r;
    MPI_Comm_size(MPI_COMM_WORLD, &r);
    return r;
}

int RCCE_send(char *buf, size_t count, int dest)
{
    MPI_Send(buf, count, MPI_CHAR, dest, 0, MPI_COMM_WORLD);
    return RCCE_SUCCESS;
}

int RCCE_recv(char *buf, size_t max, int source)
{
    MPI_Status status;
    MPI_Recv(buf, max, MPI_CHAR, source, 0, MPI_COMM_WORLD, &status);
    return RCCE_SUCCESS;
}


/*
int    RCCE_recv_test(char *, size_t, int, int *);
*/

static int mpitype[] = { 
    [RCCE_INT   -RCCE_TYPE_BASE] = MPI_INT,
    [RCCE_LONG  -RCCE_TYPE_BASE] = MPI_INT64_T,
    [RCCE_FLOAT -RCCE_TYPE_BASE] = MPI_FLOAT,
    [RCCE_DOUBLE-RCCE_TYPE_BASE] = MPI_DOUBLE,
};
static int mpiop[] = { 
    [RCCE_SUM -RCCE_OP_BASE] = MPI_SUM,
    [RCCE_MIN -RCCE_OP_BASE] = MPI_MIN,
    [RCCE_MAX -RCCE_OP_BASE] = MPI_MAX,
    [RCCE_PROD-RCCE_OP_BASE] = MPI_PROD,
};

int RCCE_allreduce(
  char *inbuf,   // source buffer for reduction datan
  char *outbuf,  // target buffer for reduction data
  int num,       // number of data elements to be reduced
  int type,      // type of data elements
  int op,        // reduction operation
  RCCE_COMM comm // communication domain within which to reduce
  )
{
    assert( type<RCCE_TYPE_BASE+4 && op<RCCE_OP_BASE+RCCE_NUM_OPS);
    MPI_Allreduce(inbuf, outbuf, num, mpitype[type-RCCE_TYPE_BASE], 
      mpiop[op-RCCE_OP_BASE], comm.mpicomm);
    return RCCE_SUCCESS;
}

int RCCE_reduce(
  char *inbuf,   // source buffer for reduction datan
  char *outbuf,  // target buffer for reduction data
  int num,       // number of data elements to be reduced
  int type,      // type of data elements
  int op,        // reduction operation
  int root,      // member of "comm" receiving reduction results
  RCCE_COMM comm // communication domain within which to reduce
  )
{
    assert( type<RCCE_TYPE_BASE+4 && op<RCCE_OP_BASE+RCCE_NUM_OPS);
    MPI_Reduce(inbuf, outbuf, num, mpitype[type-RCCE_TYPE_BASE], 
      mpiop[op-RCCE_OP_BASE], root, comm.mpicomm);
    return RCCE_SUCCESS;
}


int    RCCE_bcast(char *buf, size_t count, int root, RCCE_COMM comm)
{
    MPI_Bcast(buf, count, MPI_CHAR, root, comm.mpicomm);
    return RCCE_SUCCESS;
}

/*
t_vcharp RCCE_shmalloc(size_t);
void     RCCE_shfree(t_vcharp);
void     RCCE_shflush(void);

int    RCCE_comm_split(int (*)(int, void *), void *, RCCE_COMM *);
int    RCCE_comm_free(RCCE_COMM *);
int    RCCE_comm_size(RCCE_COMM, int *);
int    RCCE_comm_rank(RCCE_COMM, int *);
void   RCCE_fence(void);
*/
int    RCCE_barrier(RCCE_COMM *comm)
{
    MPI_Barrier(comm->mpicomm);
    return RCCE_SUCCESS;
}
/*
int    RCCE_error_string(int, char *, int *);
int    RCCE_debug_set(int);
int    RCCE_debug_unset(int);
*/

RCCE_COMM    RCCE_COMM_WORLD;
