// internal helper functions for MPI implementation

#ifndef _MPI_INTERNAL_H
#define _MPI_INTERNAL_H


#include "fgmp.h"
#include "hs_types.h"

#include <assert.h>
#include <stdbool.h>
#include <stddef.h>


typedef struct {
    int used; // ==0 if unused
    void *buf;
    int count;
    MPI_Datatype datatype;
    int source;
    int tag;
} mpi_request_t; 

#define MAX_REQUESTS 4               // maximum number of parallel Irecv

extern mpi_request_t request_slot[MAX_REQUESTS];


#define ACK_FLIT 0xbadeaffe


#define MAX_TAGS        32768           // maximum number of tags (MPI_TAG_UB+1)
                                        // DO NOT CHANGE:
                                        // hardwired in the following functions


static inline int tag_from_headflit(flit_t f)
{
    return (((f)>>(sizeof(flit_t)*8-15)) & 0x7fff);
}


static inline int len_from_headflit(flit_t f) 
{
    return ((f) & ((1L<<(sizeof(flit_t)*8-15))-1));
}


static inline flit_t headflit_from_tag_and_len(int tag, int len)
{
    return (((flit_t)(tag)<<(sizeof(flit_t)*8-15)) | (len));
}


static inline cid_t cid_from_comm(MPI_Comm comm, int rank)
{
    return (comm==MPI_COMM_WORLD) ? rank : comm->group.cids[rank];
}


// return the rank of the process in the current communicator
static inline int rank_in_comm(MPI_Comm comm, cid_t cid)
{
    if (comm==MPI_COMM_WORLD) return cid;
    int i, n=comm->group.size;
    for (i=0; i<n; n++)
        if (comm->group.cids[i]==cid)
            return i;
    return -1;
}


static inline size_t sizeof_mpi_datatype(MPI_Datatype datatype)
{
    return datatype & 0xff;
}


static inline void send_raw(cid_t dest, int len, const flit_t *buf)
{
    while (len>0) {
        pimp2_send_flit(dest, *buf++);
        len -= sizeof(flit_t);
    }
}


static inline void send_acked(cid_t dest, int len, const flit_t *buf)
{
    pimp2_send_flit(dest, *buf++);
    flit_t f = pimp2_recv_flit(dest);
    assert(f==ACK_FLIT);
    len -= sizeof(flit_t);
    while (len>0) {
        pimp2_send_flit(dest, *buf++);
        len -= sizeof(flit_t);
    }
}


// avoid unaligned memory access for little endian RISC-V
static inline flit_t unaligned_load_flit(flit_t *p)
{
    assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);
    ptrdiff_t addr = (ptrdiff_t)p;
    ptrdiff_t ofs = addr & (sizeof(flit_t)-1);
    if (ofs == 0) return *p;

    flit_t *aligned = (flit_t *)(addr & -(sizeof(flit_t)));
    // 0 1 2 3 4 5 6 7  8 9 a b c d e f
    //   left |      flit      | right
    ptrdiff_t left = 8*ofs;
    ptrdiff_t right = 8*sizeof(flit_t)-left;
    return (aligned[0] >> left) | (aligned[1] << right);
}


// avoid unaligned memory access for little endian RISC-V
static inline void unaligned_store_flit(flit_t *p, flit_t f)
{
    assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);
    ptrdiff_t addr = (ptrdiff_t)p;
    ptrdiff_t ofs = addr & (sizeof(flit_t)-1);
    if (ofs == 0) {
        *p = f;
    } else {
        flit_t *aligned = (flit_t *)(addr & -(sizeof(flit_t)));
        // 0 1 2 3 4 5 6 7  8 9 a b c d e f
        //   left |      flit      | right
        ptrdiff_t left = 8*ofs;
        ptrdiff_t right = 8*sizeof(flit_t)-left;
        aligned[0] = ((aligned[0] << right) >> right) | (f << left);
        aligned[1] = ((aligned[1] >> left) << left) | (f >> right);
    }
}


// store only a part of a flit in memory
// Use byte stores to avoid unaligned memory accesses.
static inline void store_flit_fraction(void *d, unsigned len, flit_t f)
{
    assert(__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__);
//    flit_t m = ((flit_t)(-2)<<(8*len-1));
//    unaligned_store_flit(d, ((unaligned_load_flit(d) ^ f) & m) ^ f);
    unsigned i;
    for (i=0; i<len; i++) {
        ((char *)d)[i] = f;
        f >>= 8;
    }
}


static inline void recv_raw(cid_t source, unsigned len, flit_t *buf)
{
    //assert(len > 0);
    if (len==0) return;
    while (len>sizeof(flit_t)) {
        *buf++ = pimp2_recv_flit(source);
        len -= sizeof(flit_t);
    }
    store_flit_fraction(buf, len, pimp2_recv_flit(source));
}


static inline void recv_acked(cid_t source, unsigned len, flit_t *buf)
{
    flit_t f = pimp2_recv_flit(source);
    pimp2_send_flit(source, ACK_FLIT);
    if (len > sizeof(flit_t)) {
        *buf++ = f;
        len -= sizeof(flit_t);
        while (len>sizeof(flit_t)) {
            *buf++ = pimp2_recv_flit(source);
            len -= sizeof(flit_t);
        }
        f = pimp2_recv_flit(source);
    }
    store_flit_fraction(buf, len, f);
}


// send a flit to all other processes
static inline void broadcast_flit(MPI_Comm comm, flit_t f)
{
    int n = comm->group.size;
    int i;

    if (comm==MPI_COMM_WORLD) {
        for (i=0; i<n; i++)
            if (i!=mpi_comm_world.rank)
                pimp2_send_flit(i, f);
    } else {
        for (i=0; i<n; i++)
            if (i!=comm->rank)
                pimp2_send_flit(comm->group.cids[i], f);
    }
}

static inline void wait_for_ack(MPI_Comm comm)
{
    int n = comm->group.size;
    int counter = n-1; // #processes we are waiting for
    int i = 0;
    bool ready[n];
    for (i=0; i<n; i++) ready[i] = false;
    ready[comm->rank] = true;

    while (counter!=0) {
        i++;
        if (i>=n) i = 0;
        if (!ready[i]) {
            cid_t c = cid_from_comm(comm, i);
            if (pimp2_probe(c)) {
                flit_t f = pimp2_recv_flit(c);
                assert(f==ACK_FLIT);
                ready[i] = true;
                counter--;
            }
        }
    }
}


static inline void gather(flit_t *buf_per_process[], unsigned lens[], unsigned total_len)
{
    cid_t max_cid = mpi_comm_world.group.size;
    cid_t ack=0;

    while (total_len>0) {

#ifndef OLD_PIMP2
        if (fgmp_recv_empty()) {
            // Tell every node in group to start by sending an ack flit.
            // Must be interleaved with receiving to avoid deadlock.
            if (ack<max_cid) {
                if (lens[ack]==0) ack++;
                else if (!fgmp_cong()) {
                    fgmp_send_flit(fgmp_xyz_from_cid(ack), ACK_FLIT);
                    ack++;
                }
            }
        } else {
            cid_t i = fgmp_cid_from_xyz(fgmp_recv_node());
            flit_t f = fgmp_recv_payload();
            if (lens[i]<sizeof(flit_t)) {
                store_flit_fraction(buf_per_process[i], lens[i], f);
                total_len -= lens[i];
                lens[i] = 0;
            } else {
                *buf_per_process[i]++ = f;
                total_len -= sizeof(flit_t);
                lens[i] -= sizeof(flit_t);
            }
        }
#else
        cid_t i=0;
        while (lens[i]==0 || pimp2_probe(i)==false) {
            i++;
            if (i>=max_cid) i=0;

            // Tell every node in group to start by sending an ack flit.
            // Must be interleaved with receiving to avoid deadlock.
            if (ack<max_cid) {
                if (lens[ack]==0) ack++;
                else if (!pimp2_cong()) {
                    pimp2_send_flit(ack, ACK_FLIT);
                    ack++;
                }
            }
        }
        flit_t f = pimp2_recv_flit(i);
        if (lens[i]<sizeof(flit_t)) {
            store_flit_fraction(buf_per_process[i], lens[i], f);
            total_len -= lens[i];
            lens[i] = 0;
        } else {
            *buf_per_process[i]++ = f;
            total_len -= sizeof(flit_t);
            lens[i] -= sizeof(flit_t);
        }
#endif

    }
}




#endif

