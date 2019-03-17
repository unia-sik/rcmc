#pragma once

#include <stdint.h>
#include "fgmp.h"


typedef struct fgmp_info_s {
    int address;
    int rank;
    
    int size;
    int dimension;
    int width;
    int height;
    
    int root;
} fgmp_info_t;
    
inline fgmp_info_t fgmp_info() {
    fgmp_info_t result;
    result.address = fgmp_core_id();
    result.size = fgmp_noc_size();
    result.dimension = fgmp_noc_dimension();
    result.width = result.dimension & 0xFFFF;
    result.height = result.dimension >> 16;
    result.rank = (result.address >> 16) * result.width + (result.address & 0xFFFF);
    result.root = 0;  
    
    return result;
}

inline uint32_t fgmp_addr_gen(const uint32_t x, const uint32_t y) {
    return (y << 16) | x;
}

inline fgmp_info_t fgmp_info_virtual(const uint32_t rootAddr, const uint32_t width, const uint32_t height) {
    fgmp_info_t result;
    result.address = fgmp_core_id() - rootAddr;
    result.size = width * height;
    result.dimension = fgmp_addr_gen(width, height);
    result.width = width;
    result.height = height;
    result.rank = (result.address >> 16) * result.width + (result.address & 0xFFFF);
    result.root = rootAddr;
    
    return result;
}

inline uint32_t fgmp_addr_x(const uint64_t id) {
    return id & 0xFFFF;
}

inline uint32_t fgmp_addr_y(const uint64_t id) {
    return id >> 16;
}

inline uint32_t fgmp_addr_to_rank(const uint32_t addr, const fgmp_info_t* info) {
    return fgmp_addr_y(addr) * info->width + fgmp_addr_x(addr);
}

inline uint32_t fgmp_addr_from_rank(const uint32_t rank, const fgmp_info_t* info) {
    return ((rank / info->width) << 16) | (rank % info->width);
}


inline int fgmp_addr_next_by_addr(const int addr, const fgmp_info_t* info) {
    int x = addr & 0xFFFF;
    int y = addr >> 16;
    x++;
    
    if (x == (info->dimension & 0xFFFF)) {
        x = 0;
        y++;
    }
    
    return (y << 16) | x;
}

inline int fgmp_addr_next(const fgmp_info_t* info) {
    return fgmp_addr_next_by_addr(info->address, info);
}

inline int fgmp_addr_prev_by_addr(const int addr, const fgmp_info_t* info) {
    int x = addr & 0xFFFF;
    int y = addr >> 16;
    x--;
    
    if (x == -1) {
        x = (info->dimension & 0xFFFF) - 1;
        y--;
    }
    
    return (y << 16) | x;
}

inline int fgmp_addr_prev(const fgmp_info_t* info) {
    return fgmp_addr_prev_by_addr(info->address, info);
}

inline int fgmp_addr_end(const fgmp_info_t* info) {
    return info->dimension & 0xFFFF0000;
}

inline int fgmp_addr_last(const fgmp_info_t* info) {
    return info->dimension - 0x10001;
}
