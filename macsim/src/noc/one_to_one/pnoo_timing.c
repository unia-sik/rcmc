#include "pnoo_timing.h"
#include "pnoo_event_queue.h"
#include "pnoo_context.h"
#include "pnoo_helper.h"

#include "math.h"

#define PNOO_TIMING_RDY_FLIT_INJECTION_TIME ((uint64_t)(floor(((double)(conf_noc_width * conf_noc_width)) / 2)))

cycle_t pnoo_timing_calc_access_time(const node_t *node)
{
    return node->cycle + 2;
}

uint64_t pnoo_timing_calc_next_send_round(const cycle_t clk, const bool delayed)
{    
    uint64_t i = clk % (conf_noc_width);
    if (i == 0 && delayed) {
        return 0;
    } else {
        return (conf_noc_width) - i;
    }    
}

uint64_t pnoo_timing_calc_next_flit_round(const cycle_t clk, const bool delayed)
{
    uint64_t i = clk % (PNOO_TIMING_RDY_FLIT_INJECTION_TIME);    
    if (i == 0 && delayed) {
        return 0;
    } else {
        return (PNOO_TIMING_RDY_FLIT_INJECTION_TIME) - i;
    }    
}

uint64_t pnoo_timing_calc_ready_distance(const rank_t src, const rank_t dest) {
    rank_t correctedSrc = src;
    rank_t correctedDest = dest;
    uint64_t size = conf_noc_width * conf_noc_width;
    
    if (((src / conf_noc_width) & 0x1) == 1) {
        correctedSrc -= (correctedSrc % conf_noc_width);
        correctedSrc += ((conf_noc_width - 1) - (src % conf_noc_width));
    }
    
    if (((dest / conf_noc_width) & 0x1) == 1) {
        correctedDest -= (correctedDest % conf_noc_width);
        correctedDest += ((conf_noc_width - 1) - (dest % conf_noc_width));
    }
    
    uint64_t result = (size + correctedDest - correctedSrc) % (size);
        
    return min(result, size - result);;
}

cycle_t pnoo_timing_calc_next_event_time(const cycle_t clk, const pnoo_events_t event, const rank_t src, const rank_t dest, const bool delayed)
{    
    switch (event) {
        case PNOO_EVENT_SEND_BUFFER_REMOVE:
        {            
            uint64_t sendClkOffset = pnoo_timing_calc_next_send_round(clk, delayed);            
            return clk  + sendClkOffset + 1;
        }
        case PNOO_EVENT_SEND_BUFFER_DECREMENT:
        {
            return clk + 1;
        }
        case PNOO_EVENT_RECV_BUFFER_INSERT:
        {             
            return clk + (2 * conf_noc_width);
        }
        case PNOO_EVENT_RECV_BUFFER_INCREMENT:
        {
            return clk + (2 * conf_noc_width) - 2;
        }
        case PNOO_EVENT_RDY_RECV:
        {
//             return clk + 1;
            uint64_t sendClkOffset = pnoo_timing_calc_next_flit_round(clk, false);  
            uint64_t distance = pnoo_timing_calc_ready_distance(src, dest);
            return clk  + sendClkOffset + 1 + distance;
        }
        case PNOO_EVENT_RDY_RECV_COMMIT:
        {
//             return clk + 1;
            uint64_t sendClkOffset = pnoo_timing_calc_next_flit_round(clk, delayed);        
            cycle_t recvTime = clk  + sendClkOffset;
            recvTime += PNOO_TIMING_RDY_FLIT_INJECTION_TIME + 1;
            return recvTime;
        }
        case PNOO_EVENT_RDY_RECV_RELEASE:
        {
//             return clk + 1;
            uint64_t sendClkOffset = pnoo_timing_calc_next_flit_round(clk, delayed);         
            cycle_t recvTime = clk  + sendClkOffset;
            recvTime += PNOO_TIMING_RDY_FLIT_INJECTION_TIME + 1;
            return recvTime;
        }
        case PNOO_EVENT_BARRIER_SET:
        {
//             return clk + 1;
            uint64_t sendClkOffset = pnoo_timing_calc_next_flit_round(clk, delayed);              
            int64_t distance = pnoo_timing_calc_ready_distance(src, dest);
            return clk  + sendClkOffset + 1 + distance - 1;
        }
        case PNOO_EVENT_BARRIER_ENABLE:
        {
//             return clk + 1;
            return clk + pnoo_timing_calc_next_flit_round(clk, true);        
        }
        case PNOO_EVENT_BARRIER_CLEAR:
        {
//             return clk + 1;
            return clk + pnoo_timing_calc_next_flit_round(clk, false);        
        }            
        default: fatal("Invalid event in pnoo\n");        
    };    
}

