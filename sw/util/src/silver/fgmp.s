#void fgmp_snd(uint64_t dest, uint64_t data);
.global fgmp_snd
fgmp_snd:
    snd a0, a1
    ret
    
#uint64_t fgmp_rcvn();
.global fgmp_rcvn
fgmp_rcvn:
    rcvn a0
    ret
    
#uint64_t fgmp_rcvp();
.global fgmp_rcvp
fgmp_rcvp:
    rcvp a0
    ret

#void fgmp_bsf();
.global fgmp_bsf
fgmp_bsf:
    bsf fgmp_bsf
    ret

#void fgmp_bsnf();
.global fgmp_bsnf
fgmp_bsnf:
    bsnf fgmp_bsnf
    ret

#void fgmp_bre();
.global fgmp_bre
fgmp_bre:
    bre fgmp_bre
    ret
    
#void fgmp_brne();
.global fgmp_brne
fgmp_brne:
    brne fgmp_brne
    ret

#void fgmp_srdy(uint64_t dest);
.global fgmp_srdy
fgmp_srdy:
    srdy a0
    ret

#void fgmp_bnr(uint64_t dest);
.global fgmp_bnr
fgmp_bnr:
    bnr a0, fgmp_bnr
    ret

#int fgmp_flit_available();
.global fgmp_flit_available
fgmp_flit_available:
    add a0, zero, zero
    bre fgmp_flit_available_L1
    add a0, zero, 1
fgmp_flit_available_L1:
    ret
   
   
#int fgmp_send_buffer_free()
.global fgmp_send_buffer_free
fgmp_send_buffer_free:
    add a0, zero, zero
    bsf fgmp_send_buffer_free_L1
    add a0, zero, 1
fgmp_send_buffer_free_L1:
    ret

    
#int fgmp_is_ready(uint64_t dest)
.global fgmp_is_ready
fgmp_is_ready:
    add t0, zero, zero
    bnr a0, fgmp_is_ready_L1
    add t0, t0, 1
fgmp_is_ready_L1:
    mv a0, t0
    ret
    
.global fgmp_core_id
fgmp_core_id:
    csrr    a0,0xc75
    nop
    ret
    
.global fgmp_noc_size
fgmp_noc_size:
    csrr    a0, 0xC70
    nop
    ret
    
.global fgmp_noc_dimension
fgmp_noc_dimension:
    csrr    a0, 0xC72
    nop
    add t0, zero, 1
    sll t0, t0, 16
    add t0, t0, -1
    and a0, a0, t0
    ret









