
//////////////////////////////////////////////////
// was fgmp.h for One-To-One router
/////////////////////////////////////////////////


#pragma once
#include <stdint.h>
#include "fgmp.h"

    static inline void pnoo_ibrr(uint64_t start, uint64_t end) {
        asm volatile (
            "mv a0, %0;"
            "mv a1, %1;" 
            ".word 0x00B5405B"                       
            : 
            : "r" (start), "r" (end) 
            : "a0", "a1"            
        );
    }
    
    static inline void pnoo_bbrr() {
        asm volatile (
            ".word 0x0000607b\n\t"
        );
    }
    
    static inline void pnoo_snd(uint64_t dest, uint64_t payload) {
        asm volatile (
            ".insn r 0x5b, 0, 0, x0, %0, %1\n\t"  // snd dest, payload
            : : "r" (dest), "r" (payload)   
        );
    }
    
    static inline void pnoo_srdy(uint64_t dest) {
        asm volatile (
            ".insn r 0x5b, 1, 0, x0, %0, x0\n\t" // srdy dest
            : : "r" (dest)
        );        
    }


#define pnoo_rcvn fgmp_recv_node    
#define pnoo_rcvp fgmp_recv_payload
/*
    static inline uint64_t pnoo_rcvn() {
        uint64_t result;
        asm volatile (
            "rcvn %0" 
            : "=r" (result) 
            :
        );         
        
        return result;
    }
    
    static inline uint64_t pnoo_rcvp() {        
        uint64_t result;
        asm volatile (
            "rcvp %0" 
            : "=r" (result) 
            :
        );           
        
        return result; 
    }
*/
    
    static inline void pnoo_bsf() {
        asm volatile (
            "1: .insn sb 0x7b, 0, x0, x0, 1b\n\t" // bsf $
        );
    }
    
    static inline void pnoo_bsnf() {
        asm volatile (
            "1: .insn sb 0x7b, 1, x0, x0, 1b\n\t" // bsnf $
        );        
    }
    
    static inline void pnoo_bre() {
        asm volatile (
            "1: .insn sb 0x7b, 2, x0, x0, 1b\n\t" // bre $
        );        
    }
    
    static inline void pnoo_brne() {
        asm volatile (
            "1: .insn sb 0x7b, 3, x0, x0, 1b\n\t" // brne $
        );
    }
    
    static inline void pnoo_bnr(uint64_t dest) {       
        asm volatile (
            "1: .insn sb 0x7b, 5, %0, x0, 1b\n\t" // bnr dest
            : : "r" (dest)
        );  
    }

    int pnoo_flit_available();
    //the inline version does not work... why?
    /*{ 
        int result = 0;
        asm volatile (
            "bre 1f;" 
            "add %0, %0, 1;"
            "1:"
            : "=r" (result)   
            :    
        );           
        
        return result;
    }*/
    
    static inline int pnoo_send_buffer_free() {
        int result = 0;
        asm volatile (
            ".insn sb 0x7b, 0, x0, x0, 1f\n\t" // bsf 1f
            "add %0, %0, 1\n\t"
            "1:\n\t"
            : "=r" (result)
        );
        return result;     
    }
    
    int pnoo_is_ready(uint64_t dest);
//     static inline int pnoo_is_ready(uint64_t dest) {       
//         int result = 0;
//         asm volatile (
//             "bnr %1 1f;" 
//             "add %0, %0, 1;"
//             "1:"
//             : "=r" (result)
//             : "r" (dest)      
//         );  
//     }
    
    static inline int pnoo_core_id() {
        int result;
        asm volatile (
            "csrr %0, 0xc75;"
            "nop"
            : "=r" (result)
        );
        
        return result;
    }
    
    static inline int pnoo_noc_size() {
        int result;
        asm volatile (
            "csrr %0, 0xc70;"
            "nop"
            : "=r" (result)
        );
        
        return result;        
    }
    
    static inline int pnoo_noc_dimension() {
        int result;
        asm volatile (
            "csrr %0, 0xc72;"
            "nop"
            : "=r" (result)
        );
        
        return result & 0xFFFFFFFF;   
    }





//////////////////////////////////////////////////
// was pnoo_addr.h for One-To-One router
//////////////////////////////////////////////////



typedef struct pnoo_info_s {
    int address;
    int rank;
    
    int size;
    int dimension;
    int width;
    int height;
    
    int root;
} pnoo_info_t;
    
static inline pnoo_info_t pnoo_info() {
    pnoo_info_t result;
    result.address = pnoo_core_id();
    result.size = pnoo_noc_size();
    result.dimension = pnoo_noc_dimension();
    result.width = result.dimension & 0xFFFF;
    result.height = result.dimension >> 16;
    result.rank = (result.address >> 16) * result.width + (result.address & 0xFFFF);
    result.root = 0;  
    
    return result;
}

static inline uint32_t pnoo_addr_gen(const uint32_t x, const uint32_t y) {
    return (y << 16) | x;
}

static inline pnoo_info_t pnoo_info_virtual(const uint32_t rootAddr, const uint32_t width, const uint32_t height) {
    pnoo_info_t result;
    result.address = pnoo_core_id() - rootAddr;
    result.size = width * height;
    result.dimension = pnoo_addr_gen(width, height);
    result.width = width;
    result.height = height;
    result.rank = (result.address >> 16) * result.width + (result.address & 0xFFFF);
    result.root = rootAddr;
    
    return result;
}

static inline uint32_t pnoo_addr_x(const uint64_t id) {
    return id & 0xFFFF;
}

static inline uint32_t pnoo_addr_y(const uint64_t id) {
    return id >> 16;
}

static inline uint32_t pnoo_addr_to_rank(const uint32_t addr, const pnoo_info_t* info) {
    return pnoo_addr_y(addr) * info->width + pnoo_addr_x(addr);
}

static inline uint32_t pnoo_addr_from_rank(const uint32_t rank, const pnoo_info_t* info) {
    return ((rank / info->width) << 16) | (rank % info->width);
}


static inline int pnoo_addr_next_by_addr(const int addr, const pnoo_info_t* info) {
    int x = addr & 0xFFFF;
    int y = addr >> 16;
    x++;
    
    if (x == (info->dimension & 0xFFFF)) {
        x = 0;
        y++;
    }
    
    return (y << 16) | x;
}

static inline int pnoo_addr_next(const pnoo_info_t* info) {
    return pnoo_addr_next_by_addr(info->address, info);
}

static inline int pnoo_addr_prev_by_addr(const int addr, const pnoo_info_t* info) {
    int x = addr & 0xFFFF;
    int y = addr >> 16;
    x--;
    
    if (x == -1) {
        x = (info->dimension & 0xFFFF) - 1;
        y--;
    }
    
    return (y << 16) | x;
}

static inline int pnoo_addr_prev(const pnoo_info_t* info) {
    return pnoo_addr_prev_by_addr(info->address, info);
}

static inline int pnoo_addr_end(const pnoo_info_t* info) {
    return info->dimension & 0xFFFF0000;
}

static inline int pnoo_addr_last(const pnoo_info_t* info) {
    return info->dimension - 0x10001;
}




//////////////////////////////////////////////////
// was pnoo_block.h for One-To-One router
//////////////////////////////////////////////////






void pnoo_block_send(uint64_t dest, int n, void* data);
void pnoo_block_recv(uint64_t src, int n, void* data);

void pnoo_block_send_no_srdy(uint64_t dest, int n, void* data);
void pnoo_block_recv_no_srdy(uint64_t src, int n, void* data);

//void pnoo_block_send_recv(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);
//void pnoo_block_send_recv_no_srdy(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);

void pnoo_block_send_32(uint64_t dest, int n, void* data);
void pnoo_block_recv_32(uint64_t src, int n, void* data);

void pnoo_block_send_no_srdy_32(uint64_t dest, int n, void* data);
void pnoo_block_recv_no_srdy_32(uint64_t src, int n, void* data);

void pnoo_block_send_recv_32(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);
void pnoo_block_send_recv_no_srdy_32(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);



static inline void pnoo_block_send_recv_no_srdy(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv) {    
    
//     if (pnoo_core_id() < dest) {        
//         pnoo_block_send_no_srdy(dest, n, data_send);
//         pnoo_block_recv_no_srdy(src, n, data_recv); 
//     } else {
//         pnoo_block_recv_no_srdy(src, n, data_recv); 
//         pnoo_block_send_no_srdy(dest, n, data_send);
//     }
//     
//     return;
    while (n != 0) {
        if (n >= 256) {
//             pnoo_srdy(src);
//             pnoo_bnr(dest);
            pnoo_block_send_no_srdy(dest, 256, data_send);
            pnoo_block_recv_no_srdy(src, 256, data_recv);
            
            n -= 256;
            data_send += 256;
            data_recv += 256;
        } else {
//             pnoo_srdy(src);
//             pnoo_bnr(dest);
            pnoo_block_send_no_srdy(dest, n, data_send);
            pnoo_block_recv_no_srdy(src, n, data_recv);           
            
            n = 0;
        }
    }    
}

static inline void pnoo_block_send_recv(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv) {
    pnoo_srdy(src);
    pnoo_bnr(dest);
    pnoo_block_send_recv_no_srdy(dest, src, n, data_send, data_recv);
}

