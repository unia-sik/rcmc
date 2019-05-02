#ifndef _MPI_COMMON_TYPES_H
#define _MPI_COMMON_TYPES_H



////////////////////////////////////////////////////////////////////////
// Data structures
////////////////////////////////////////////////////////////////////////



// datatypes
// lowest 8 bits are equal to the size in bytes
#define MPI_INT8_T      0x001
#define MPI_INT16_T     0x002
#define MPI_INT32_T     0x004
#define MPI_INT64_T     0x008
#define MPI_UINT8_T     0x101
#define MPI_UINT16_T    0x102
#define MPI_UINT32_T    0x104
#define MPI_UINT64_T    0x108

#define MPI_BYTE        0x001
#define MPI_CHAR        0x001
#define MPI_SHORT       (sizeof(short)) // architecture dependent!
#define MPI_INT         (sizeof(int))   // architecture dependent!
#define MPI_FLOAT       0x204
#define MPI_DOUBLE      0x208


// reduction operations
#define MPI_MAX 0
#define MPI_MIN 1
#define MPI_SUM 2
#define MPI_PROD 3
#define MPI_BAND 5
#define MPI_BOR 7
#define MPI_BXOR 9
#define _IS_ARITH_OP(op)    ((op)<4)


#define MPI_UNDEFINED   (-1)
#define MPI_SUCCESS     0
#define MPI_COMM_NULL   0
#define MPI_GROUP_NULL  0
#define MPI_GROUP_EMPTY (void *)(-1)


typedef int MPI_Datatype;
typedef int MPI_Op;


typedef struct {
    int MPI_SOURCE;
    int MPI_TAG;
    int MPI_ERROR;
    int len; // in bytes
} MPI_Status;

typedef struct {
    int16_t     size; // number of processes
    int16_t     cids[1];
} mpi_group_t;
typedef mpi_group_t *MPI_Group;



#endif

