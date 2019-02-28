/** 
 * $Id: memory.h 659 2014-03-13 10:35:46Z klugeflo $
 * Memory simulation
 *
 * MacSim project
 *
 */
#ifndef _MEMORY_H
#define _MEMORY_H

#include "node.h"
#include "share.h"
#include <byteswap.h>


// memory types
#define MT_PAGED_32BIT	1	// 3 staged pagetabele for full 32 bit adresses
#define MT_LINEAR	2	// linear memory from 0 to node_t->memory_size

// check if memory access is within memory
#define LINEAR_MEMORY_SIZE_CHECK\
    if (addr > node->memory_size)\
	fatal("Memory access to %x is beyond end at %x", addr, node->memory_size);


// Init memory
void memory_init(node_t *node, uint16_min_t mt, uint64_min_t ms);

// Finish memory, i.e. free allocated memory blocks
void memory_finish(node_t *node);

// Specialized read and write routines (for faster memory accesses)
void memory_read_var_p(node_t *node, addr_t addr, uint8_t *dest, uint32_min_t len);
void memory_write_var_p(node_t *node, addr_t addr, const uint8_t *src, uint32_min_t len);
void memory_read_var_l(node_t *node, addr_t addr, uint8_t *dest, uint32_min_t len);
void memory_write_var_l(node_t *node, addr_t addr, const uint8_t *src, uint32_min_t len);

// Read from memory
void memory_read(node_t *node, addr_t addr, uint8_t *dest, uint32_min_t len);

// Write to memory
void memory_write(node_t *node, addr_t addr, const uint8_t *src, uint32_min_t len);

// Print a memory dump
void memory_print_dump(node_t *node, addr_t from, addr_t to);

// General disassmbler that just prints a 32 bit dump
int no_disasm(node_t *node, addr_t pc, char *dstr);

// Detect file format and load into memory
void memory_load_file(node_t *node, const char *filename);



// -----------------------------------------------------------------------------
// Deal with host byte order
// -----------------------------------------------------------------------------

// The following macros can be uses in both directions, no matter if you read
// a value with a certain byte order to the host byte order or if you write from
// the host byte order to a certain byte order:
//   my = ENDIAN_16be(be)
//   be = ENDIAN_16be(my)
// no conversation is needed if you stay in the same endianess:
//   my = my
//   be = be

// byte order detection works for gcc and clang
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define ENDIAN_16le(x)		(x)
#define ENDIAN_32le(x)		(x)
#define ENDIAN_64le(x)		(x)
#define ENDIAN_16be(x)		bswap_16(x)
#define ENDIAN_32be(x)		bswap_32(x)
#define ENDIAN_64be(x)		bswap_64(x)
#else
#define ENDIAN_16le(x)		bswap_16(x)
#define ENDIAN_32le(x)		bswap_32(x)
#define ENDIAN_64le(x)		bswap_64(x)
#define ENDIAN_16be(x)		(x)
#define ENDIAN_32be(x)		(x)
#define ENDIAN_64be(x)		(x)
#endif
#define ENDIAN_8


/*
// optimized versions for linear memory
static inline uint8_t _memr_8l(node_t *node, addr_t addr)
{
    LINEAR_MEMORY_SIZE_CHECK
    return node->memory.u8[addr];
}

static inline uint16_t _memr_16l(node_t *node, addr_t addr)
{
    LINEAR_MEMORY_SIZE_CHECK
    return node->memory.u16[addr>>1];
}

static inline uint32_t _memr_32l(node_t *node, addr_t addr)
{
    LINEAR_MEMORY_SIZE_CHECK
    return node->memory.u32[addr>>2];
}

static inline uint64_t _memr_64l(node_t *node, addr_t addr)
{
    LINEAR_MEMORY_SIZE_CHECK
    return node->memory.u64[addr>>3];
}

static inline void _memw_8l(node_t *node, addr_t addr, uint8_t data)
{
    LINEAR_MEMORY_SIZE_CHECK
    node->memory.u8[addr] = data;
}

static inline void _memw_16l(node_t *node, addr_t addr, uint16_t data)
{
    LINEAR_MEMORY_SIZE_CHECK
    node->memory.u16[addr>>1] = data;
}

static inline void _memw_32l(node_t *node, addr_t addr, uint32_t data)
{
    LINEAR_MEMORY_SIZE_CHECK
    node->memory.u32[addr>>2] = data;
}

static inline void _memw_64l(node_t *node, addr_t addr, uint64_t data)
{
    LINEAR_MEMORY_SIZE_CHECK
    node->memory.u64[addr>>3] = data;
}
*/

// Helper macros to generate load/store macros
// Not for external usage!

#define IS_DEVS_ADDR(n, a)			\
  ( ((n)->device_segment.mask & (a)) == (n)->device_segment.base )


// memory access types
#define MA_8		2
#define MA_u8		3
#define MA_16le		4
#define MA_u16le	5
#define MA_16be		6
#define MA_u16be	7
#define MA_32le		8
#define MA_u32le	9
#define MA_32be		10
#define MA_u32be	11
#define MA_64le		12
#define MA_u64le	13
#define MA_64be		14
#define MA_u64be	15

static addr_t memory_access_len[] = {
    [MA_8]      = 1,
    [MA_u8]     = 1,
    [MA_16le]   = 2,
    [MA_u16le]  = 2,
    [MA_16be]   = 2,
    [MA_u16be]  = 2,
    [MA_32le]   = 4,
    [MA_u32le]  = 4,
    [MA_32be]   = 4,
    [MA_u32be]  = 4,
    [MA_64le]   = 8,
    [MA_u64le]  = 8,
    [MA_64be]   = 8,
    [MA_u64be]  = 8,
};

#define MA_8		2
#define MA_u8		3
#define MA_16le		4
#define MA_u16le	5
#define MA_16be		6
#define MA_u16be	7
#define MA_32le		8
#define MA_u32le	9
#define MA_32be		10
#define MA_u32be	11
#define MA_64le		12
#define MA_u64le	13
#define MA_64be		14
#define MA_u64be	15



// directly load from memory (no mapping)
static inline int memory_load_u64_direct(node_t *node, unsigned access_type, 
    addr_t addr, uint64_t *dest)
{
    switch (access_type)
    {

#define GEN_LOAD(u, w, e) \
    case MA_##u##w##e: \
    { \
	u##int##w##_t data; \
	memory_read(node, addr, (uint8_t *)&data, w>>3); \
	*dest = (u##int##w##_t)ENDIAN_##w##e(data); \
	break; \
    }
    GEN_LOAD( ,  8,   )
    GEN_LOAD(u,  8,   )
    GEN_LOAD( , 16, le)
    GEN_LOAD(u, 16, le)
    GEN_LOAD( , 32, le)
    GEN_LOAD(u, 32, le)
    GEN_LOAD( , 64, le)
    GEN_LOAD(u, 64, le)
    GEN_LOAD( , 16, be)
    GEN_LOAD(u, 16, be)
    GEN_LOAD( , 32, be)
    GEN_LOAD(u, 32, be)
    GEN_LOAD( , 64, be)
    GEN_LOAD(u, 64, be)
#undef GEN_LOAD

    default: fatal("Invalid memory access type %d", access_type);
    }
    return 1;
}


// directly store to memory (no mapping)
static inline int memory_store_direct(node_t *node, unsigned access_type, 
    addr_t addr, uint64_t data)
{
    switch (access_type)
    {

#define GEN_STORE(w, e) \
    case MA_##w##e: \
    { \
	uint##w##_t d = ENDIAN_##w##e(data); \
        memory_write(node, addr, (uint8_t *)&d, w>>3); \
	break; \
    }
    GEN_STORE( 8, )
    GEN_STORE(16, le)
    GEN_STORE(16, be)
    GEN_STORE(32, le)
    GEN_STORE(32, be)
    GEN_STORE(64, le)
    GEN_STORE(64, be)
#undef GEN_STORE
    default: fatal("Invalid memory access type %d", access_type);
    }
    return 1;
}


// Load from memory with check for memory mapped devices
// Maybe replaced by a core specific version with extended memory mapping
static inline int_fast16_t generic_memory_load_u64(node_t *node, unsigned access_type, 
    addr_t addr, uint64_t *dest)
{
    return memory_load_u64_direct(node, access_type, addr, dest);
}


// Store to memory with check for memory mapped devices
// Maybe replaced by a core specific version with extended memory mapping
static inline int_fast16_t generic_memory_store(node_t *node, unsigned access_type, 
    addr_t addr, uint64_t data)
{
    return memory_store_direct(node, access_type, addr, data);
}








// -----------------------------------------------------------------------------
// Deprecated
// -----------------------------------------------------------------------------


#define GEN_FETCH(w, e) \
static inline int memory_fetch_##w##e(node_t *node, addr_t addr, uint##w##_t *dest) \
{ \
    uint##w##_t data; \
    memory_read(node, addr, (uint8_t *)&data, w>>3); \
    *dest = ENDIAN_##w##e(data); \
    return 1; \
}

#define GEN_LOAD(u, w, e, t) \
static inline int memory_load_##u##w##e##_as_uint##t(node_t *node, addr_t addr, uint##t##_t *dest) \
{ \
    u##int##w##_t data; \
    memory_read(node, addr, (uint8_t *)&data, w>>3); \
    *dest = (u##int##w##_t)ENDIAN_##w##e(data); \
    return 1; \
}

#define GEN_STORE(w, e) \
static inline int memory_store_##w##e(node_t *node, addr_t addr, uint##w##_t data) \
{ \
    uint##w##_t d = ENDIAN_##w##e(data); \
    memory_write(node, addr, (uint8_t *)&d, w>>3); \
    return 1; \
}


//GEN_FETCH( 8, )
GEN_FETCH(16, le)
//GEN_FETCH(16, be)
GEN_FETCH(32, le)
//GEN_FETCH(32, be)
//GEN_FETCH(64, le)
//GEN_FETCH(64, be)

/*
GEN_LOAD( ,  8,   , 32)
GEN_LOAD(u,  8,   , 32)
GEN_LOAD( , 16, le, 32)
GEN_LOAD(u, 16, le, 32)
GEN_LOAD( , 32, le, 32)
GEN_LOAD(u, 32, le, 32)
GEN_LOAD( , 64, le, 32)
GEN_LOAD(u, 64, le, 32)
GEN_LOAD( , 16, be, 32)
GEN_LOAD(u, 16, be, 32)
GEN_LOAD( , 32, be, 32)
GEN_LOAD(u, 32, be, 32)
GEN_LOAD( , 64, be, 32)
GEN_LOAD(u, 64, be, 32)

GEN_STORE( 8, )
GEN_STORE(16, le)
GEN_STORE(16, be)
GEN_STORE(32, le)
GEN_STORE(32, be)
GEN_STORE(64, le)
GEN_STORE(64, be)
*/

#undef GEN_FETCH
#undef GEN_LOAD
#undef GEN_STORE




#endif // _MEMORY_H
