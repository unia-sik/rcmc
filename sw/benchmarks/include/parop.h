#ifndef _PAROP_H
#define _PAROP_H

#include "machine.h"
#include "parop.h"

#define PAROP_MAX_MEMBLOCK      64
    // maximum number of allocated memory blocks per thread
#define PAROP_MAX_DIRECTIONS    64
    // maximum number of directions for parop_forward
#define PAROP_MAX_COLLECTIONS   4
    // maximum number of groups for collective operations


#define PAROP_OP_ADD    0
#define PAROP_OP_MUL    1
#define PAROP_OP_MIN    2
#define PAROP_OP_MAX    3

#define PAROP_ONCE      if (parop_my_rank==0)



#ifdef PAROP_IMPL_MPI
#include "parop_mpi.h"
#else
#include "parop_pthread.h"
#endif



static inline bool *parop_def_array_bool(int handle, size_t n)
{
    return (bool *)parop_def_memblock(handle, n*sizeof(bool));
}


static inline int32_t *parop_def_array_int32(int handle, size_t n)
{
    return (int32_t *)parop_def_memblock(handle, n*sizeof(int32_t));
}


static inline int64_t *parop_def_array_int64(int handle, size_t n)
{
    return (int64_t *)parop_def_memblock(handle, n*sizeof(int64_t));
}


static inline double *parop_def_array_double(int handle, size_t n)
{
    return parop_def_memblock(handle, n*sizeof(double));
}





#endif // _PAROP_H
