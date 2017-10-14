/**
 * rvmpb.c
 * RISC-V ISA with message passing buffers
 *
 * MacSim project
 */

#include "riscv.h"
#include "rvmpb.h"
#include "memory.h"
#include <fenv.h>
#include <inttypes.h>
#include <math.h>
#include <string.h>


#define RVMPB_LATENCY_ADDR_CALC         0
#define RVMPB_LATENCY_INTERLOCK         1
#define RVMPB_LATENCY_MPB               6

#define RISCV_FLIT_LOAD 0x4711876543210000




// ---------------------------------------------------------------------
// memory access
// ---------------------------------------------------------------------

extern node_t *nodes[];
// ugly but effective




// Load from memory with check for memory mapped devices
// Maybe replaced by a core specific version with extended memory mapping
static inline int_fast16_t memory_load_u64(node_t *node, unsigned access_type, 
    addr_t addr, uint64_t *dest)
{
    if (addr>=0xc0000000) {
        if (addr<0xd8000000) {
            assert ((RISCV_LOADBUF_SIZE & (RISCV_LOADBUF_SIZE-1)) == 0);
                // must be a power of 2
            rank_t source = (addr>>16)-0xc000;
            if (source == node->rank) {
                return generic_memory_load_u64(node, access_type, 
                    0xd8000000+(addr&0xffff), dest);
            } else {
                if (source >= conf_max_rank) fatal("MPB access to unavailable core %xh", node);
                addr_t ofs = addr & (RISCV_LOADBUF_SIZE-1);
                addr_t base = addr & (0x00010000 - RISCV_LOADBUF_SIZE);
                addr_t len = memory_access_len[access_type];
                if ((source == node->core.riscv.loadbuf_rank)
                    && (base == node->core.riscv.loadbuf_base)
                    && (ofs+len <= RISCV_LOADBUF_SIZE))
                {
                    // hit
                    memcpy(dest, node->core.riscv.loadbuf+ofs, len);
                    return 1;
                } else {
                    node->core.riscv.loadbuf_rank = source;
                    node->core.riscv.loadbuf_base = base;

// fill load buffer here, if faster transfers with less flits should be simulated
//                    memory_read(nodes[source], 0xd8000000+base,
//                        node->core.riscv.loadbuf, RISCV_LOADBUF_SIZE);

                    if (node->core.riscv.mpb_send_pos < RISCV_LOADBUF_SIZE) {
                        // Don't start a request, as the node is busy with
                        // responding to a load request.
                        // By setting rank but not pos one_cycle() knows to 
                        // start the request directly after the respond is finished.
                        // Therefore it is guaranteed that a request does not
                        // interrupt a response.
                        node->core.riscv.loadbuf_pos = RISCV_LOADBUF_SIZE;
                    } else {
                        node->core.riscv.loadbuf_pos = 0;

                        // send a message and wait for the receive
                        if (!node->noc_send_flit(node, source, base | RISCV_FLIT_LOAD)) {
                            node->state = CS_SEND_BLOCKED;
                            return IC_BLOCKED;
                        }
                    }
                    node->state = CS_RECV_BLOCKED;
                    return IC_BLOCKED;
                }
            }
        } else if (addr>=0xd9000000) {
            fatal("#%lu °%d @%x: Memory load from address %lx", 
                node->cycle, node->rank, node->pc, addr);
        }
        // else fallthrough: private access to own MPB
    }
    return generic_memory_load_u64(node, access_type, addr, dest);
}




// Store to memory with check for memory mapped devices
// Maybe replaced by a core specific version with extended memory mapping
static inline int_fast16_t memory_store(node_t *node, unsigned access_type, 
    addr_t addr, uint64_t data)
{
    if (addr>=0xc0000000) {
        if (addr<0xd8000000) {
            rank_t dest = (addr>>16)-0xc000;
            if (dest >= conf_max_rank) 
                fatal("MPB access to unavailable core %xh", node);
            return RVMPB_LATENCY_MPB + generic_memory_store(
                nodes[dest],
                access_type, 
                (addr & 0x0000ffff) + 0xd8000000,
                data);
        } else if (addr>=0xd9000000) {
            fatal("Memory store to address %xh", addr);
        }
    }
    return generic_memory_store(node, access_type, addr, data);
}





// ---------------------------------------------------------------------
// floating point math
// ---------------------------------------------------------------------

typedef union {
    int32_t i;
    float f;
} if32_t;

typedef union {
    uint64_t u;
    double f;
} uf64_t;


#define BRu(h,l)	(((iw)>>(l))&((1<<((h)-(l)+1))-1))
#define REG_S           node->core.riscv.reg[BRu(19, 15)]
#define REG_Du          (*((uint64_t *)&node->core.riscv.reg[BRu(11, 7)]))
#define REG_Tu          (*((uint64_t *)&node->core.riscv.reg[BRu(24, 20)]))
#define SREG_D          (*((float *)&node->core.riscv.freg[BRu(11, 7)]))
#define SREG_T          (*((float *)&node->core.riscv.freg[BRu(24, 20)]))
#define DREG_D          node->core.riscv.freg[BRu(11, 7)]
#define DREG_T          node->core.riscv.freg[BRu(24, 20)]
#define IMM_I           ((int64_t)(int32_t)iw>>20)
#define IMM_S           ((((int64_t)(int32_t)iw>>20)&0xffffffffffffffe0) \
                        | ((iw>>7) &0x0000001f))
#define nBRu(h,l)	(((next_iw)>>(l))&((1<<((h)-(l)+1))-1))


// ---------------------------------------------------------------------
// Main switch for decoding and execution
// ---------------------------------------------------------------------


instruction_class_t rvmpb_execute_iw(node_t *node, uint_fast32_t iw, uint_fast32_t next_iw)
{
    if32_t x32;
    uf64_t x64;

    node->core.riscv.reg[0] = 0;

    if ((iw&0x5b)==0x03) switch (iw & 0x7f)
    {
        case 0x03:
        {
            addr_t addr = REG_S + IMM_I;
            instruction_class_t latency = RVMPB_LATENCY_ADDR_CALC;

            // next_instruction.REG_S = REG_D and next_instruction uses REG_S
            if (riscv_instruction_uses_reg_s(next_iw) && nBRu(19, 15) == BRu(11, 7)) {
                latency = RVMPB_LATENCY_INTERLOCK;
            }
            // next_instruction.REG_T = REG_D and next_instruction uses REG_T
            if (riscv_instruction_uses_reg_t(next_iw) && nBRu(24, 20) == BRu(11, 7)) {
                latency = RVMPB_LATENCY_INTERLOCK;
            }

            uint_fast16_t op = (iw >> 12) & 7;
            if (op!=7) {
                unsigned access_from_opcode[8] =
                    {MA_8, MA_16le, MA_32le, MA_64le, MA_u8, MA_u16le, MA_u32le, 0};
                instruction_class_t ic = 
                    memory_load_u64(node, access_from_opcode[op], addr, &REG_Du);
                return (ic<0) ? ic : (ic+latency);
            }
            break;
        }

        case 0x23:
        {
            addr_t addr = REG_S + IMM_S;
            switch (iw & 0x7000) {
            case 0x0000: return RVMPB_LATENCY_ADDR_CALC
                + memory_store(node, MA_8, addr, REG_Tu); // sb
            case 0x1000: return RVMPB_LATENCY_ADDR_CALC
                + memory_store(node, MA_16le, addr, REG_Tu); // sh
            case 0x2000: return RVMPB_LATENCY_ADDR_CALC
                + memory_store(node, MA_32le, addr, REG_Tu); // sw
            case 0x3000: return RVMPB_LATENCY_ADDR_CALC
                + memory_store(node, MA_64le, addr, REG_Tu); // sd
            }
            break;
        }

        case 0x07:
        {
            addr_t addr = REG_S + IMM_I;
            switch (iw & 0x7000) {
            case 0x2000:  // flw
            {
                uint64_t u;
                uint16_t lat = memory_load_u64(node, MA_32le, addr, &u);
                x32.i = u;
                SREG_D = x32.f;
                return RVMPB_LATENCY_ADDR_CALC+lat;
            }
            case 0x3000:  // fld
            {
                uint16_t lat = memory_load_u64(node, MA_64le, addr, &x64.u);
                DREG_D = x64.f;
                return RVMPB_LATENCY_ADDR_CALC+lat;
            }
            }
            break;
        }

        case 0x27:
        {
            addr_t addr = REG_S + IMM_S;
            switch (iw & 0x7000) {
            case 0x2000:  // fsw
                x32.f = SREG_T;
                return RVMPB_LATENCY_ADDR_CALC
                    + memory_store(node, MA_32le, addr, x32.i);
            case 0x3000:  // fsd
                x64.f = DREG_T;
                return RVMPB_LATENCY_ADDR_CALC
                    + memory_store(node, MA_64le, addr, x64.u);
            }
            break;
        }
    }

    if ((iw&0x707f)==0x206b) {
        // invmpb
        node->core.riscv.loadbuf_rank = conf_max_rank;
        return 1;
    }

    return riscv_execute_iw(node, iw, next_iw);
}


// Simulate one cycle
instruction_class_t rvmpb_one_cycle(node_t *node)
{
    uint32_t iw;
    uint32_t next_iw;
    uint_fast32_t pc = node->pc;


    // 1st: receive load responds
    if (node->core.riscv.loadbuf_pos < RISCV_LOADBUF_SIZE) {
        assert (node->state == CS_RECV_BLOCKED);
        flit_t flit;

        if (!node->noc_recv_flit(node, node->core.riscv.loadbuf_rank, &flit)) {
            node->state = CS_RECV_BLOCKED;
            return IC_BLOCKED;
        }

        // FIXME: byte ordering?
        *(flit_t *)&node->core.riscv.loadbuf[node->core.riscv.loadbuf_pos] = flit;
        node->core.riscv.loadbuf_pos += FLIT_LEN;
        if (node->core.riscv.loadbuf_pos < RISCV_LOADBUF_SIZE) {
            node->state = CS_RECV_BLOCKED;
            return IC_BLOCKED;
        }
    }


    // 2nd: send load responds
    if (node->core.riscv.mpb_send_pos < RISCV_LOADBUF_SIZE) {
        if (node->noc_sender_ready(node)) {
            flit_t flit;
            generic_memory_load_u64(
                node, 
                MA_64le, 
                0xd8000000 + node->core.riscv.mpb_send_base + node->core.riscv.mpb_send_pos,
                 &flit);
            if (node->noc_send_flit(node, node->core.riscv.mpb_send_rank, flit)) {
                node->core.riscv.mpb_send_pos += FLIT_LEN;
                if (node->core.riscv.mpb_send_pos >= RISCV_LOADBUF_SIZE
                    && node->core.riscv.loadbuf_rank < conf_max_rank) 
                {
                    // pending load respond: activate it
                    node->core.riscv.loadbuf_pos = 0;
                }
            }
        }
    } else {


        // 3rd: check if new load request (only if none is pending)
        rank_t source = node->noc_probe_any(node);
        if (source>=0) {
            flit_t flit;
            if (node->noc_recv_flit(node, source, &flit)) {
                assert ((flit & 0xffffffffffff0000)==RISCV_FLIT_LOAD);
                node->core.riscv.mpb_send_rank = source;
                node->core.riscv.mpb_send_base = flit & 0xffff;
                node->core.riscv.mpb_send_pos = 0;
            }
            // begin sending in next cycle
        }
    }


    memory_fetch_32le(node, pc, &iw);
    node->nextpc = pc + 4;
    memory_fetch_32le(node, node->nextpc, &next_iw);
    node->retired++;
    return riscv_execute_iw(node, iw, next_iw);
}


// Init context
void rvmpb_init_context(node_t *node)
{
    riscv_init_context(node);

    node->core_type = CT_riscv;
    node->one_cycle = &rvmpb_one_cycle;

    node->core.riscv.loadbuf_rank = conf_max_rank;
    node->core.riscv.loadbuf_pos = RISCV_LOADBUF_SIZE;
    node->core.riscv.mpb_send_pos = RISCV_LOADBUF_SIZE;
}


// Remove context from memory and free memory blocks
void rvmpb_finish_context(node_t *node)
{
    riscv_finish_context(node);
}



