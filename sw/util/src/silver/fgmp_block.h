#pragma once
#include <stdint.h>


void fgmp_block_send(uint64_t dest, int n, void* data);
void fgmp_block_recv(uint64_t src, int n, void* data);

void fgmp_block_send_no_srdy(uint64_t dest, int n, void* data);
void fgmp_block_recv_no_srdy(uint64_t src, int n, void* data);

void fgmp_block_send_recv(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);
void fgmp_block_send_recv_no_srdy(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);

void fgmp_block_send_32(uint64_t dest, int n, void* data);
void fgmp_block_recv_32(uint64_t src, int n, void* data);

void fgmp_block_send_no_srdy_32(uint64_t dest, int n, void* data);
void fgmp_block_recv_no_srdy_32(uint64_t src, int n, void* data);

void fgmp_block_send_recv_32(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);
void fgmp_block_send_recv_no_srdy_32(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);
