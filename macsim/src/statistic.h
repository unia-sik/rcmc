#pragma once

#include <stdint.h>


typedef struct statistic_instr {
    uint64_t hits;
    uint64_t cycles;
} statistic_instr_t;

typedef struct statistic {
    statistic_instr_t* instrs;
    uint64_t instrs_length;
    
    int* send_flits;
    int send_flits_length;
    
    int* recv_flits;
    int recv_flits_length;
} statistic_t;

void statistic_init(statistic_t* stat);
void statistic_free(statistic_t* stat);

void statistic_insert_instr(statistic_t* stat, uint64_t addr, uint64_t cycles);
void statistic_insert_send_flit(statistic_t* stat, uint64_t dest);
void statistic_insert_recv_flit(statistic_t* stat, uint64_t dest);



