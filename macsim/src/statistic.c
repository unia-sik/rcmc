#include "statistic.h"

#include "share.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void statistic_init(statistic_t* stat)
{
    stat->instrs = NULL;
    stat->instrs_length = 0;
    stat->send_flits = NULL;
    stat->send_flits_length = 0;
    stat->recv_flits = NULL;
    stat->recv_flits_length = 0;
}

void statistic_free(statistic_t* stat)
{
    free(stat->instrs);
    stat->instrs_length = 0;
    free(stat->send_flits);
    stat->send_flits_length = 0;
    free(stat->recv_flits);
    stat->recv_flits_length = 0;
}

void statistic_insert_instr(statistic_t* stat, uint64_t addr, uint64_t cycles)
{
    if (stat->instrs_length <= addr) {
        stat->instrs = realloc(stat->instrs, addr * 2 * sizeof(*stat->instrs));
        memset(stat->instrs + stat->instrs_length, 0, (addr * 2 - stat->instrs_length) * sizeof(*stat->instrs));
        
        stat->instrs_length = addr * 2;
    }
    
    stat->instrs[addr].hits++;
    stat->instrs[addr].cycles += cycles;
}

void statistic_insert_send_flit(statistic_t* stat, uint64_t dest)
{    
    dest = (dest & 0xFFFF) + ((dest & 0xFFFF0000) >> 16) * conf_noc_width;
    if (stat->send_flits_length <= dest) {
        dest++;
        stat->send_flits = realloc(stat->send_flits, dest * 2 * sizeof(*stat->send_flits));
        memset(stat->send_flits + stat->send_flits_length, 0, (dest * 2 - stat->send_flits_length) * sizeof(*stat->send_flits));
        
        stat->send_flits_length = dest * 2;
        dest--;
    }
    
    stat->send_flits[dest]++;
}

void statistic_insert_recv_flit(statistic_t* stat, uint64_t dest)
{
    dest = (dest & 0xFFFF) + ((dest & 0xFFFF0000) >> 16) * conf_noc_width;
    if (stat->recv_flits_length <= dest) {
        dest++;
        stat->recv_flits = realloc(stat->recv_flits, dest * 2 * sizeof(*stat->recv_flits));
        memset(stat->recv_flits + stat->recv_flits_length, 0, (dest * 2 - stat->recv_flits_length) * sizeof(*stat->recv_flits));
        
        stat->recv_flits_length = dest * 2;
        dest--;
    }
    
    stat->recv_flits[dest]++;
}
