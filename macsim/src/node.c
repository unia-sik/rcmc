/** 
 * $Id: node.h 346 2012-08-09 14:50:27Z mischejo $
 * Declare a node with all possible core implementations
 *
 * McSim project
 */
#include "node.h"
#include "memory.h"

//#include "tricore.h"
//#include "or32.h"
//#include "mips32.h"
#include "armv3.h"
#include "armv6m.h"
#include "riscv.h"
#include "rvmpb.h"
#include "trace.h"
#include "traffic.h"

#include "fixedlat.h"
#include "pnbe0.h"
#include "pnbe1.h"
#include "pnbe2.h"
#include "caerus.h"
#include "pnoa.h"
#include "pnaa.h"
//#include "paternoster.h"
//#include "gs_one_to_one.h"
//#include "gs_one_to_one_dyn.h"
#include "pnjm0.h"
#include "pnconfig.h"
#include "minbd.h"
#include "perfect.h"
#include "manhattan.h"






uint64_t xorshift128plus_next(uint64_t *seed) 
{
    uint64_t a = seed[0];
    uint64_t b = seed[1];
    a ^= a << 23;
    uint64_t c = a ^ b ^ (a >> 18) ^ (b >> 5);
    seed[0] = b;
    seed[1] = c;
    return c + b; 
}


void xorshift128plus_jump(uint64_t *seed)
{
    uint64_t a = seed[0];
    uint64_t b = seed[1];
    uint64_t c = 0;
    uint64_t d = 0;
    uint64_t x = 0x8a5cd789635d2dff;

    for (unsigned i=0; i<128; i++) {
        if (i==64) x=0x121fd2155c472f96;
        if (x & 1) {
            c ^= a;
            d ^= b;
        }
        x >>= 1;
        uint64_t e = a ^ (a << 23);
        a = b;
        b = e ^ b ^ (e >> 18) ^ (b >> 5);
    }
    seed[0] = c;
    seed[1] = d;
}


// Per-node random generator using the xorshift+ algorithm
uint64_t node_rand64(node_t *node)
{
    return xorshift128plus_next(node->seed);
}




// Init the memory
void core_init_context(node_t *node)
{
    node->bp_addr = NO_BREAKPOINT;

    switch (node->core_type) {
//        case CT_tricore:    tricore_init_context(node); break;
//        case CT_or32:       or32_init_context(node); break;
//        case CT_mips32:     mips32_init_context(node); break;
        case CT_armv3:          armv3_init_context(node); break;
        case CT_armv6m:         armv6m_init_context(node); break;
        case CT_netrace:        netrace_init_context(node); break;
        case CT_riscv:          riscv_init_context(node); break;
        case CT_rvmpb:          rvmpb_init_context(node); break;
        case CT_traffic:        traffic_init_context(node); break;
        default:                fatal("Unknown core type %d", node->core_type);
    }
}


// init all cores
void core_init_all(node_t *nodes[], rank_t max_rank)
{
    rank_t i;
    uint64_t s[2];

    s[0] = 0x9223372036854775; // inital seed
    s[1] = 0x8491185219873134;

    for (i=0; i<max_rank; i++) {
        core_init_context(nodes[i]);

        // each node gets an independent random sequence
        nodes[i]->seed[0] = s[0];
        nodes[i]->seed[1] = s[1];
        xorshift128plus_jump(s);
    }
}


// Remove context from memory and free memory blocks
void core_finish_context(node_t *node)
{
    switch (node->core_type) {
        case CT_armv3:          armv3_finish_context(node); break;
        case CT_armv6m:         armv6m_finish_context(node); break;
        case CT_netrace:        netrace_finish_context(node); break;
        case CT_riscv:          riscv_finish_context(node); break;
        case CT_rvmpb:          rvmpb_finish_context(node); break;
        case CT_traffic:        traffic_finish_context(node); break;
        default:                fatal("Unknown core type %d", node->core_type);
    }
}

// Print a register dump
void core_print_context(node_t *node)
{
    switch (node->core_type) {
//        case CT_tricore:    tricore_print_context(node); break;
//        case CT_or32:       or32_print_context(node); break;
//        case CT_mips32:     mips32_print_context(node); break;
        case CT_armv3:          armv3_print_context(node); break;
        case CT_armv6m:         armv6m_print_context(node); break;
        case CT_netrace:        netrace_print_context(node); break;
        case CT_riscv:          riscv_print_context(node); break;
        case CT_rvmpb:          riscv_print_context(node); break;
        case CT_traffic:        traffic_print_context(node); break;
        default:                fatal("Unknown core type %d", node->core_type);
    }
}

// Store the current context to a file
void core_dump_context(const char *file, node_t *node)
{
    switch (node->core_type) {
//        case CT_tricore:    tricore_print_context(node); break;
//        case CT_or32:       or32_print_context(node); break;
//        case CT_mips32:     mips32_print_context(node); break;
        case CT_armv3:      fatal("Not implemented for armv3"); break;
        case CT_armv6m:     fatal("Not implemented for armv6m"); break;
        case CT_riscv:      riscv_dump_context(file, node); break;
        case CT_rvmpb:      riscv_dump_context(file, node); break;
        default:            fatal("Unknown core type %d", node->core_type);
    }
}

// Disassemble one instruction
int core_disasm(node_t *node, addr_t pc, char *dstr)
{
    switch (node->core_type) {
//        case CT_tricore:    return no_disasm(node, pc, dstr);
//        case CT_or32:       return no_disasm(node, pc, dstr);
//        case CT_mips32:     return no_disasm(node, pc, dstr);
        case CT_armv3:          return armv3_disasm(node, pc, dstr);
        case CT_armv6m:         return armv6m_disasm(node, pc, dstr);
        case CT_riscv:          return riscv_disasm(node, pc, dstr);
        case CT_rvmpb:          return riscv_disasm(node, pc, dstr);
        case CT_traffic:        return no_disasm(node, pc, dstr);
        default:                fatal("Unknown core type %d", node->core_type);
    }
}

// Init the NoC
void noc_init_all(node_t *nodes[], uint_fast16_t type, rank_t width, rank_t height)
{
    // init each router
    rank_t i, x, y;
    for (i=0; i<height*width; i++) {
        node_t *n = nodes[i];
        n->noc_type = type;
        switch (type){
            case NT_fixedlat:   fixedlat_init(n); break;
            case NT_pnbe0:      pnbe0_init(n); break;
            case NT_pnbe1:      pnbe1_init(n); break;
            case NT_pnbe2:      pnbe2_init(n); break;
            case NT_caerus:     caerus_init(n); break;
            case NT_pnoa:       pnoa_init(n); break;
            case NT_pnaa:       pnaa_init(n); break;
//            case NT_paternoster_skeleton:       paternoster_init(node); break;
//            case NT_gs_one_to_one:              gs_one_to_one_init(node); break;
//            case NT_gs_one_to_one_dyn:          gs_one_to_one_dyn_init(node); break;
            case NT_pnjm0:      pnjm0_init(n); break;
            case NT_pnconfig:   pnconfig_init(n); break;
            case NT_minbd:      minbd_init(n); break;
            case NT_perfect:    perfect_init(n); break;
            case NT_manhattan:  manhattan_init(n); break;
            default:            fatal("Unknown NoC type %d", type);
        }
    }

    // connect router contexts
    // y
    // 2  16 17 18 19 ...
    // 1   8  9 10 11 ...
    // 0   0  1  2  3 ...
    //     0  1  2  3 x
    for (y=0; y<height; y++) {
        for (x=0; x<width; x++) {
            node_t *me = nodes[y*width+x];
            node_t *n = (y==0 
                ? nodes[(height-1)*width+x]
                : nodes[     (y-1)*width+x]);
            node_t *s = (y==height-1
                ? nodes[x]
                : nodes[(y+1)*width+x]);
            node_t *w = (x==0
                ? nodes[y*width+width-1]
                : nodes[y*width+x-1]);
            node_t *e = (x==width-1
                ? nodes[y*width]
                : nodes[y*width+x+1]);
            switch (type){
                case NT_pnjm0:
                    pnjm0_connect(me, x, y, n, s, w, e); break;
                case NT_pnconfig:
                    pnconfig_connect(me, x, y, n, s, w, e); break;
                case NT_minbd:
                    minbd_connect(me, x, y, n, s, w, e); break;
            }
        }
    }
}

// Destroy the NoC
// Must preserve the Noc Type !
void noc_destroy_all(node_t *nodes[], rank_t max_rank)
{
    rank_t i;
    unsigned nt = nodes[0]->noc_type;
    for (i=0; i<max_rank; i++) {
        node_t *n = nodes[i];
        switch(n->noc_type){
            case NT_fixedlat:   fixedlat_destroy(n); break;
            case NT_pnbe0:      pnbe0_destroy(n); break;
            case NT_pnbe1:      pnbe1_destroy(n); break;
            case NT_pnbe2:      pnbe2_destroy(n); break;
            case NT_caerus:     caerus_destroy(n); break;
            case NT_pnoa:       pnoa_destroy(n); break;
            case NT_pnaa:       pnaa_destroy(n); break;
//            case NT_paternoster_skeleton:       paternoster_destroy(n); break;
//            case NT_gs_one_to_one:              gs_one_to_one_destroy(n); break;
//            case NT_gs_one_to_one_dyn:          gs_one_to_one_dyn_destroy(n); break;
            case NT_pnjm0:      pnjm0_destroy(n); break;
            case NT_pnconfig:   pnconfig_destroy(n); break;
            case NT_minbd:      minbd_destroy(n); break;
            case NT_perfect:    perfect_destroy(n); break;
            case NT_manhattan:  manhattan_destroy(n); break;
            default:            fatal("Unknown NoC type %d", n->noc_type);
        }
    }

    // print statistics
    switch(nt) {
        case NT_pnjm0:          pnjm0_print_stat(); break;
        case NT_pnconfig:       pnconfig_print_stat(); break;
        case NT_minbd:          minbd_print_stat(); break;
    }
}


// simulate one cycle of the whole NoC
void noc_route_all(node_t *nodes[], rank_t max_rank)
{
    rank_t r;

    // router type must be identical for all nodes
    switch(nodes[0]->noc_type) {
        case NT_pnjm0:          pnjm0_route_all(nodes, max_rank); break;
        case NT_pnconfig:       pnconfig_route_all(nodes, max_rank); break;
        case NT_minbd:          minbd_route_all(nodes, max_rank); break;
        case NT_perfect:        break;

        default:
            for (r=0; r<conf_max_rank; r++)
                nodes[r]->noc_route_one_cycle(nodes[r]);
    }
}


// print the state of the NoC
void noc_print_context(node_t *nodes[], rank_t max_rank)
{
    // router type must be identical for all nodes
    switch(nodes[0]->noc_type) {
        case NT_pnjm0:          pnjm0_print_context(nodes, max_rank); break;
        case NT_pnconfig:       pnconfig_print_context(nodes, max_rank); break;
        case NT_minbd:          minbd_print_context(nodes, max_rank); break;
        case NT_manhattan:      manhattan_print_context(nodes, max_rank); break;

        default:
            user_printf("Not supported for this NoC\n");
    }
}

void noc_dump_context(const char *file, node_t *nodes[], rank_t max_rank)
{
    switch(nodes[0]->noc_type) {
        case NT_pnconfig:       pnconfig_dump_context(file, nodes, max_rank); break;

        default:
            user_printf("Not supported for this NoC\n");
    }
}

void noc_log_traffic(const char *file, node_t *nodes[], rank_t max_rank)
{
    switch(nodes[0]->noc_type){
        case NT_pnconfig:       pnconfig_log_traffic(file, nodes, max_rank); break;

        default:
            user_printf("log_traffic is not supported for this NoC variant\n");
    }
}

