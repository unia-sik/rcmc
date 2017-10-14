/** 
 * $Id: node.h 346 2012-08-09 14:50:27Z mischejo $
 * Declare a node with all possible core implementations
 *
 * McSim project
 */
#ifndef _NODE_H
#define _NODE_H

#include "share.h"

// context structures for the cores
//#include "tricore_context.h"
//#include "or32_context.h"
//#include "mips32_context.h"
#include "armv3_context.h"
#include "armv6m_context.h"
#include "riscv_context.h"
#include "traffic_context.h"
#include "trace_context.h"

// NoC implementation
//#include "fixedlat_context.h"
#include "pnbe0_context.h"
#include "pnbe1_context.h"
#include "pnbe2_context.h"
#include "caerus_context.h"
#include "pnoa_context.h"
#include "pnaa_context.h"
//#include "paternoster_context.h"
//#include "gs_one_to_one_context.h"


// ---------------------------------------------------------------------
// Currently not core-dependent, but possible enhancement
// ---------------------------------------------------------------------

// Maximum length of a disassembled instruction
#define MAX_DISASM_STR	64

// Maximum length of an instruction in bytes
#define MAX_BYTES_PER_INSTRUCTION 4




// core types
#define CT_NONE         0
#define CT_traffic      1
#define CT_tricore      2
#define CT_or32         3
#define CT_mips32       4
#define CT_armv6m       5
#define CT_patmos       6
#define CT_riscv        7
#define CT_armv3        8
#define CT_netrace      9
#define CT_rvmpb        10


// synthetic traffic pattern type
#define TRAFFIC_NONE            '0'
#define TRAFFIC_UNIFORM         'U'
#define TRAFFIC_BITCOMP         'B'
#define TRAFFIC_TRANSPOSE       'T'
#define TRAFFIC_BITREV          'R'
#define TRAFFIC_SHUFFLE         'S'
#define TRAFFIC_TORNADO         'O'
#define TRAFFIC_NEIGHBOR        'N'
#define TRAFFIC_UPPERLEFT       'L'
//#define TRAFFIC_RANDPERM	'P'


// NoC types
#define NT_fixedlat     0
#define NT_manhattan    1
#define NT_pnbe0        2
#define NT_pnbe1        10
#define NT_pnbe2        9
#define NT_caerus       8
#define NT_pnoa         5
#define NT_pnaa         7
#define NT_pnjm0        20
#define NT_pnconfig     19
#define NT_minbd        18
#define NT_perfect      17


#define NT_paternoster_skeleton			1
#define NT_gs_one_to_one				3
#define NT_gs_one_to_one_dyn			4
#define NT_gs_all_to_all_simple			6

// local memory types
#define MT_PAGED_32BIT  1       // 3 staged pagetabele for full 32 bit addresses
#define MT_LINEAR       2       // Linear memory from 0 to node_t->memory_size


#define CI_NONE	        0
#define CI_WWABSTRACT   1

#define NO_BREAKPOINT    ((addr_t)-1)

// These fields provide the common interface to all core implementations.
// Therefore this macro must be the first entry in any core_t declaration.
typedef struct node_s {
    uint_fast32_t       state;
    cycle_t             cycle;
    cycle_t             retired;        // # retired instructions
    rank_t              rank;           // id of the core
    addr_t              pc;             // program counter
    addr_t              nextpc;         // pc after execution of the current instruction
    uint_fast32_t       instruction_word; // only set if unknown after execute

    // pseudo random number generator
    uint64_t            seed[2];

    // memory
    uint_fast16_t       memory_type;
    uint_fast64_t       memory_size;
    uint_fast32_t       memory_last_latency;
    union {
        uint8_t         ***map;
        uint8_t         *u8;
        uint16_t        *u16;
        uint32_t        *u32;
        uint64_t        *u64;
    } memory;

    // breakpoints
    addr_t              bp_addr;

    // network interface
    bool (*noc_send_flit)(struct node_s*, rank_t, flit_t);
    bool (*noc_recv_flit)(struct node_s*, rank_t, flit_t*);
    bool (*noc_sender_ready)(struct node_s*);
    bool (*noc_probe_rank)(struct node_s*, rank_t);
    rank_t (*noc_probe_any)(struct node_s*);
    void (*noc_route_one_cycle)(struct node_s*);

    // NoC
    uint_fast16_t       noc_type;
    void* noc_context;

    // cores
    uint_fast16_t       core_type;
    instruction_class_t (*one_cycle)(struct node_s*);
    union {
        // each supported core must be added here
//        tricore_context_t       tricore;
//        or32_context_t          or32;
//        mips32_context_t        mips32;
        armv3_context_t         armv3;
        armv6m_context_t        armv6m;
        riscv_context_t         riscv;
        traffic_context_t       traffic;
        netrace_context_t       netrace;
    } core;
} node_t;


// Per-node random generator using the xorshift+ algorithm
uint64_t node_rand64(node_t *node);

// Random number that is lower than max
// Only uniform, if max is a power of 2
static inline uint64_t node_randmax(node_t *node, uint64_t max)
{
    return node_rand64(node) % max;
}

// Init the memory
void core_init_context(node_t *node);

// Init all cores at once
void core_init_all(node_t *nodes[], rank_t max_rank);

// Remove context from memory and free memory blocks
void core_finish_context(node_t *node);

// Print a register dump of the ARM v6-M
void core_print_context(node_t *node);

// Store the current context to a file
void core_dump_context(const char *file, node_t *node);

// Disassemble one instruction
int core_disasm(node_t *node, addr_t pc, char *dstr);

// Init the whole NoC
void noc_init_all(node_t *nodes[], uint_fast16_t type, rank_t width, rank_t height);

// Destroy the whole NoC
void noc_destroy_all(node_t *nodes[], rank_t max_rank);

// Simulate one cycle of the whole NoC
void noc_route_all(node_t *nodes[], rank_t max_rank);

// Print the state of the NoC
void noc_print_context(node_t *nodes[], rank_t max_rank);

void noc_dump_context(const char *file, node_t *nodes[], rank_t max_rank);

// Print a logfile of all flit traffic in a cycle
void noc_log_traffic(const char *file, node_t *nodes[], rank_t max_rank);


#endif // _NODE_H


