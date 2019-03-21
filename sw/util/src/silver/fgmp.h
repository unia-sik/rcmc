#pragma once
#include <stdint.h>

#define USE_INLINE

#ifndef USE_INLINE
    void fgmp_snd(uint64_t dest, uint64_t data);
    void fgmp_srdy(uint64_t dest);
    uint64_t fgmp_rcvn();
    uint64_t fgmp_rcvp();
    void fgmp_bsf();
    void fgmp_bsnf();
    void fgmp_bre();
    void fgmp_brne();
    void fgmp_bnr(uint64_t dest);

    int fgmp_flit_available();
    int fgmp_send_buffer_free();
    int fgmp_is_ready(uint64_t dest);
    
    int fgmp_core_id();
    int fgmp_noc_size();
    int fgmp_noc_dimension();
#else
    inline void fgmp_ibrr(uint64_t start, uint64_t end) {
        asm volatile (
            "mv a0, %0;"
            "mv a1, %1;" 
            ".word 0x00B5405B"                       
            : 
            : "r" (start), "r" (end) 
            : "a0", "a1"            
        );
    }
    
    inline void fgmp_bbrr() {
        asm volatile (
            ".word 0x0000607b\n\t"
        );
    }
    
    inline void fgmp_snd(uint64_t dest, uint64_t data) {
        asm volatile (
            "snd %0, %1" 
            : 
            : "r" (dest), "r" (data)             
        );
    }
    
    inline void fgmp_srdy(uint64_t dest) {
        asm volatile (
            "srdy %0" 
            : 
            : "r" (dest)      
        );        
    }
    
    inline uint64_t fgmp_rcvn() {
        uint64_t result;
        asm volatile (
            "rcvn %0" 
            : "=r" (result) 
            :
        );         
        
        return result;
    }
    
    inline uint64_t fgmp_rcvp() {        
        uint64_t result;
        asm volatile (
            "rcvp %0" 
            : "=r" (result) 
            :
        );           
        
        return result; 
    }
    
    inline void fgmp_bsf() {
        asm volatile (
            "1: bsf 1b\n\t"
        );
    }
    
    inline void fgmp_bsnf() {
        asm volatile (
            "1: bsnf 1b\n\t"
        );        
    }
    
    inline void fgmp_bre() {
        asm volatile (
            "1: bre 1b\n\t"
        );        
    }
    
    inline void fgmp_brne() {        
        asm volatile (
            "1: brne 1b\n\t"
        );
    }
    
    inline void fgmp_bnr(uint64_t dest) {       
        asm volatile (
            "1: bnr %0, 1b" 
            : 
            : "r" (dest)      
        );  
    }

    int fgmp_flit_available();
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
    
    inline int fgmp_send_buffer_free() {
        int result = 0;
        asm volatile (
            "bsf 1f;" 
            "add %0, %0, 1;"
            "1:"
            : "=r" (result)   
            :    
        );             
        
        return result;     
    }
    
    int fgmp_is_ready(uint64_t dest);
//     inline int fgmp_is_ready(uint64_t dest) {       
//         int result = 0;
//         asm volatile (
//             "bnr %1 1f;" 
//             "add %0, %0, 1;"
//             "1:"
//             : "=r" (result)
//             : "r" (dest)      
//         );  
//     }
    
    inline int fgmp_core_id() {
        int result;
        asm volatile (
            "csrr %0, 0xc75;"
            "nop"
            : "=r" (result)
        );
        
        return result;
    }
    
    inline int fgmp_noc_size() {
        int result;
        asm volatile (
            "csrr %0, 0xc70;"
            "nop"
            : "=r" (result)
        );
        
        return result;        
    }
    
    inline int fgmp_noc_dimension() {
        int result;
        asm volatile (
            "csrr %0, 0xc72;"
            "nop"
            : "=r" (result)
        );
        
        return result & 0xFFFFFFFF;   
    }
#endif

