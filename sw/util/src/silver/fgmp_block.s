#void fgmp_block_send(uint64_t dest, int n, void* data);
#void fgmp_block_send_no_srdy(uint64_t dest, int n, void* data);
.global fgmp_block_send
.global fgmp_block_send_no_srdy
fgmp_block_send:
    bnr a0, fgmp_block_send
fgmp_block_send_no_srdy:
    add t0, zero, a1
    add t2, zero, 256
send_block_loop:   
    sub t1, t2, t0
    add t0, t0, -256
    blt t1, zero, send_block_block_L0
    add t0, zero, zero
    
    srl t1, t1, 3 # number of flits to ignore
    sll t1, t1, 2 # each instr is 4 Byte    
    
    sub a2, a2, t1
    sub a2, a2, t1
    
    auipc t4, 0
    add t4, t4, t1 # jump over 3 * 4 * #ignored_flits
    add t4, t4, t1
    add t4, t4, t1
    jr 20(t4)
   
send_block_block_L0:
    ld t5, 0(a2)
send_block_block_L1:    bsf send_block_block_L1    
    snd a0, t5
    
    ld t5, 8(a2)
send_block_block_L2:    bsf send_block_block_L2  
    snd a0, t5
    
    ld t5, 16(a2)
send_block_block_L3:    bsf send_block_block_L3    
    snd a0, t5
    
    ld t5, 24(a2)
send_block_block_L4:    bsf send_block_block_L4    
    snd a0, t5
    
    ld t5, 32(a2)
send_block_block_L5:    bsf send_block_block_L5    
    snd a0, t5
    
    ld t5, 40(a2)
send_block_block_L6:    bsf send_block_block_L6    
    snd a0, t5
    
    ld t5, 48(a2)
send_block_block_L7:    bsf send_block_block_L7    
    snd a0, t5
    
    ld t5, 56(a2)
send_block_block_L8:    bsf send_block_block_L8    
    snd a0, t5
    
    ld t5, 64(a2)
send_block_block_L9:    bsf send_block_block_L9    
    snd a0, t5
    
    ld t5, 72(a2)
send_block_block_L10:    bsf send_block_block_L10    
    snd a0, t5
    
    ld t5, 80(a2)
send_block_block_L11:    bsf send_block_block_L11   
    snd a0, t5
    
    ld t5, 88(a2)
send_block_block_L12:    bsf send_block_block_L12    
    snd a0, t5
    
    ld t5, 96(a2)
send_block_block_L13:    bsf send_block_block_L13    
    snd a0, t5
    
    ld t5, 104(a2)
send_block_block_L14:    bsf send_block_block_L14 
    snd a0, t5
    
    ld t5, 112(a2)
send_block_block_L15:    bsf send_block_block_L15   
    snd a0, t5
    
    ld t5, 120(a2)
send_block_block_L16:    bsf send_block_block_L16    
    snd a0, t5
    
    ld t5, 128(a2)
send_block_block_L17:    bsf send_block_block_L17    
    snd a0, t5
    
    ld t5, 136(a2)
send_block_block_L18:    bsf send_block_block_L18    
    snd a0, t5
    
    ld t5, 144(a2)
send_block_block_L19:    bsf send_block_block_L19    
    snd a0, t5
    
    ld t5, 152(a2)
send_block_block_L20:    bsf send_block_block_L20    
    snd a0, t5
    
    ld t5, 160(a2)
send_block_block_L21:    bsf send_block_block_L21    
    snd a0, t5
    
    ld t5, 168(a2)
send_block_block_L22:    bsf send_block_block_L22    
    snd a0, t5
    
    ld t5, 176(a2)
send_block_block_L23:    bsf send_block_block_L23    
    snd a0, t5
    
    ld t5, 184(a2)
send_block_block_L24:    bsf send_block_block_L24    
    snd a0, t5
    
    ld t5, 192(a2)
send_block_block_L25:    bsf send_block_block_L25    
    snd a0, t5
    
    ld t5, 200(a2)
send_block_block_L26:    bsf send_block_block_L26    
    snd a0, t5
    
    ld t5, 208(a2)
send_block_block_L27:    bsf send_block_block_L27    
    snd a0, t5
    
    ld t5, 216(a2)
send_block_block_L28:    bsf send_block_block_L28    
    snd a0, t5
    
    ld t5, 224(a2)
send_block_block_L29:    bsf send_block_block_L29    
    snd a0, t5
    
    ld t5, 232(a2)
send_block_block_L30:    bsf send_block_block_L30    
    snd a0, t5
    
    ld t5, 240(a2)
send_block_block_L31:    bsf send_block_block_L31    
    snd a0, t5
    
    ld t5, 248(a2)
send_block_block_L32:    bsf send_block_block_L32    
    snd a0, t5
    
    add a2, a2, 256

    bne t0, zero, send_block_loop
send_block_end:
    ret
    
#void fgmp_block_recv(uint64_t src, int n, void* data);
#void fgmp_block_recv_no_srdy(uint64_t src, int n, void* data);
.global fgmp_block_recv
.global fgmp_block_recv_no_srdy
fgmp_block_recv:
    srdy a0
fgmp_block_recv_no_srdy:
    add t0, zero, a1
    add t2, zero, 256
recv_block_loop:   
    sub t1, t2, t0
    add t0, t0, -256
    blt t1, zero, recv_block_block_L0
    add t0, zero, zero
    
    srl t1, t1, 3 # number of flits to ignore
    sll t1, t1, 2 # each instr is 4 Byte    
    
    sub a2, a2, t1
    sub a2, a2, t1
    
    auipc t4, 0
    add t4, t4, t1 # jump over 3 * 4 * #ignored_flits
    add t4, t4, t1
    add t4, t4, t1
    jr 20(t4)
   
recv_block_block_L0:
recv_block_block_L1:    bre recv_block_block_L1    
    rcvp a0
    sd a0, 0(a2)
    
recv_block_block_L2:    bre recv_block_block_L2  
    rcvp a0
    sd a0, 8(a2)
    
recv_block_block_L3:    bre recv_block_block_L3    
    rcvp a0
    sd a0, 16(a2)
    
recv_block_block_L4:    bre recv_block_block_L4    
    rcvp a0
    sd a0, 24(a2)
    
recv_block_block_L5:    bre recv_block_block_L5    
    rcvp a0
    sd a0, 32(a2)
    
recv_block_block_L6:    bre recv_block_block_L6    
    rcvp a0
    sd a0, 40(a2)
    
recv_block_block_L7:    bre recv_block_block_L7    
    rcvp a0
    sd a0, 48(a2)
    
recv_block_block_L8:    bre recv_block_block_L8    
    rcvp a0
    sd a0, 56(a2)
    
recv_block_block_L9:    bre recv_block_block_L9    
    rcvp a0
    sd a0, 64(a2)
    
recv_block_block_L10:    bre recv_block_block_L10    
    rcvp a0
    sd a0, 72(a2)
    
recv_block_block_L11:    bre recv_block_block_L11   
    rcvp a0
    sd a0, 80(a2)
    
recv_block_block_L12:    bre recv_block_block_L12    
    rcvp a0
    sd a0, 88(a2)
    
recv_block_block_L13:    bre recv_block_block_L13    
    rcvp a0
    sd a0, 96(a2)
    
recv_block_block_L14:    bre recv_block_block_L14 
    rcvp a0
    sd a0, 104(a2)
    
recv_block_block_L15:    bre recv_block_block_L15   
    rcvp a0
    sd a0, 112(a2)
    
recv_block_block_L16:    bre recv_block_block_L16    
    rcvp a0
    sd a0, 120(a2)
    
recv_block_block_L17:    bre recv_block_block_L17    
    rcvp a0
    sd a0, 128(a2)
    
recv_block_block_L18:    bre recv_block_block_L18    
    rcvp a0
    sd a0, 136(a2)
    
recv_block_block_L19:    bre recv_block_block_L19    
    rcvp a0
    sd a0, 144(a2)
    
recv_block_block_L20:    bre recv_block_block_L20    
    rcvp a0
    sd a0, 152(a2)
    
recv_block_block_L21:    bre recv_block_block_L21    
    rcvp a0
    sd a0, 160(a2)
    
recv_block_block_L22:    bre recv_block_block_L22    
    rcvp a0
    sd a0, 168(a2)
    
recv_block_block_L23:    bre recv_block_block_L23    
    rcvp a0
    sd a0, 176(a2)
    
recv_block_block_L24:    bre recv_block_block_L24    
    rcvp a0
    sd a0, 184(a2)
    
recv_block_block_L25:    bre recv_block_block_L25    
    rcvp a0
    sd a0, 192(a2)
    
recv_block_block_L26:    bre recv_block_block_L26    
    rcvp a0
    sd a0, 200(a2)
    
recv_block_block_L27:    bre recv_block_block_L27    
    rcvp a0
    sd a0, 208(a2)
    
recv_block_block_L28:    bre recv_block_block_L28    
    rcvp a0
    sd a0, 216(a2)
    
recv_block_block_L29:    bre recv_block_block_L29    
    rcvp a0
    sd a0, 224(a2)
    
recv_block_block_L30:    bre recv_block_block_L30    
    rcvp a0
    sd a0, 232(a2)
    
recv_block_block_L31:    bre recv_block_block_L31    
    rcvp a0
    sd a0, 240(a2)
    
recv_block_block_L32:    bre recv_block_block_L32    
    rcvp a0
    sd a0, 248(a2)    
    
    add a2, a2, 256

    bne t0, zero, recv_block_loop
recv_block_end:
    ret
    
#void fgmp_block_send_recv(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);
#void fgmp_block_send_recv_no_srdy(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);
#.global fgmp_block_send_recv
#.global fgmp_block_send_recv_no_srdy
fgmp_block_send_recv_old:
    srdy a1
send_recv_block_srdy_old:    bnr a0, send_recv_block_srdy_old
fgmp_block_send_recv_no_srdy_old:
    add t0, zero, a2
    add t2, zero, 256
send_recv_block_loop:   
    sub t1, t2, t0
    add t0, t0, -256
    blt t1, zero, send_recv_block_block_L0
    add t0, zero, zero
    
    srl t1, t1, 3 # number of flits to ignore
    sll t1, t1, 2 # each instr is 4 Byte    
    
    sub a3, a3, t1
    sub a3, a3, t1
    
    sub a4, a4, t1
    sub a4, a4, t1
    
    auipc t4, 0
    add t4, t4, t1 # jump over 6 * 4 * #ignored_flits
    add t4, t4, t1
    add t4, t4, t1
    add t4, t4, t1
    add t4, t4, t1
    add t4, t4, t1
    jr 32(t4)

send_recv_block_block_L0:
    ld t5, 0(a3)
send_recv_block_block_L1_send:    bsf send_recv_block_block_L1_send    
    snd a0, t5
send_recv_block_block_L1_recv:    bre send_recv_block_block_L1_recv    
    rcvp t5
    sd t5, 0(a4)    

    ld t5, 8(a3)
send_recv_block_block_L2_send:    bsf send_recv_block_block_L2_send    
    snd a0, t5
send_recv_block_block_L2_recv:    bre send_recv_block_block_L2_recv    
    rcvp t5
    sd t5, 8(a4)   

    ld t5, 16(a3)
send_recv_block_block_L3_send:    bsf send_recv_block_block_L3_send    
    snd a0, t5
send_recv_block_block_L3_recv:    bre send_recv_block_block_L3_recv    
    rcvp t5
    sd t5, 16(a4)   

    ld t5, 24(a3)
send_recv_block_block_L4_send:    bsf send_recv_block_block_L4_send    
    snd a0, t5
send_recv_block_block_L4_recv:    bre send_recv_block_block_L4_recv    
    rcvp t5
    sd t5, 24(a4)   

    ld t5, 32(a3)
send_recv_block_block_L5_send:    bsf send_recv_block_block_L5_send    
    snd a0, t5
send_recv_block_block_L5_recv:    bre send_recv_block_block_L5_recv    
    rcvp t5
    sd t5, 32(a4)   

    ld t5, 40(a3)
send_recv_block_block_L6_send:    bsf send_recv_block_block_L6_send    
    snd a0, t5
send_recv_block_block_L6_recv:    bre send_recv_block_block_L6_recv    
    rcvp t5
    sd t5, 40(a4)   

    ld t5, 48(a3)
send_recv_block_block_L7_send:    bsf send_recv_block_block_L7_send    
    snd a0, t5
send_recv_block_block_L7_recv:    bre send_recv_block_block_L7_recv    
    rcvp t5
    sd t5, 48(a4)   

    ld t5, 56(a3)
send_recv_block_block_L8_send:    bsf send_recv_block_block_L8_send    
    snd a0, t5
send_recv_block_block_L8_recv:    bre send_recv_block_block_L8_recv    
    rcvp t5
    sd t5, 56(a4)   

    ld t5, 64(a3)
send_recv_block_block_L9_send:    bsf send_recv_block_block_L9_send    
    snd a0, t5
send_recv_block_block_L9_recv:    bre send_recv_block_block_L9_recv    
    rcvp t5
    sd t5, 64(a4)   

    ld t5, 72(a3)
send_recv_block_block_L10_send:    bsf send_recv_block_block_L10_send    
    snd a0, t5
send_recv_block_block_L10_recv:    bre send_recv_block_block_L10_recv    
    rcvp t5
    sd t5, 72(a4)   

    ld t5, 80(a3)
send_recv_block_block_L11_send:    bsf send_recv_block_block_L11_send    
    snd a0, t5
send_recv_block_block_L11_recv:    bre send_recv_block_block_L11_recv    
    rcvp t5
    sd t5, 80(a4)   

    ld t5, 88(a3)
send_recv_block_block_L12_send:    bsf send_recv_block_block_L12_send    
    snd a0, t5
send_recv_block_block_L12_recv:    bre send_recv_block_block_L12_recv    
    rcvp t5
    sd t5, 88(a4)   

    ld t5, 96(a3)
send_recv_block_block_L13_send:    bsf send_recv_block_block_L13_send    
    snd a0, t5
send_recv_block_block_L13_recv:    bre send_recv_block_block_L13_recv    
    rcvp t5
    sd t5, 96(a4)   

    ld t5, 104(a3)
send_recv_block_block_L14_send:    bsf send_recv_block_block_L14_send    
    snd a0, t5
send_recv_block_block_L14_recv:    bre send_recv_block_block_L14_recv    
    rcvp t5
    sd t5, 104(a4)   

    ld t5, 112(a3)
send_recv_block_block_L15_send:    bsf send_recv_block_block_L15_send    
    snd a0, t5
send_recv_block_block_L15_recv:    bre send_recv_block_block_L15_recv    
    rcvp t5
    sd t5, 112(a4)   

    ld t5, 120(a3)
send_recv_block_block_L16_send:    bsf send_recv_block_block_L16_send    
    snd a0, t5
send_recv_block_block_L16_recv:    bre send_recv_block_block_L16_recv    
    rcvp t5
    sd t5, 120(a4)   

    ld t5, 128(a3)
send_recv_block_block_L17_send:    bsf send_recv_block_block_L17_send    
    snd a0, t5
send_recv_block_block_L17_recv:    bre send_recv_block_block_L17_recv    
    rcvp t5
    sd t5, 128(a4)   

    ld t5, 136(a3)
send_recv_block_block_L18_send:    bsf send_recv_block_block_L18_send    
    snd a0, t5
send_recv_block_block_L18_recv:    bre send_recv_block_block_L18_recv    
    rcvp t5
    sd t5, 136(a4)   

    ld t5, 144(a3)
send_recv_block_block_L19_send:    bsf send_recv_block_block_L19_send    
    snd a0, t5
send_recv_block_block_L19_recv:    bre send_recv_block_block_L19_recv    
    rcvp t5
    sd t5, 144(a4)   

    ld t5, 152(a3)
send_recv_block_block_L20_send:    bsf send_recv_block_block_L20_send    
    snd a0, t5
send_recv_block_block_L20_recv:    bre send_recv_block_block_L20_recv    
    rcvp t5
    sd t5, 152(a4)   

    ld t5, 160(a3)
send_recv_block_block_L21_send:    bsf send_recv_block_block_L21_send    
    snd a0, t5
send_recv_block_block_L21_recv:    bre send_recv_block_block_L21_recv    
    rcvp t5
    sd t5, 160(a4)   

    ld t5, 168(a3)
send_recv_block_block_L22_send:    bsf send_recv_block_block_L22_send    
    snd a0, t5
send_recv_block_block_L22_recv:    bre send_recv_block_block_L22_recv    
    rcvp t5
    sd t5, 168(a4)   

    ld t5, 176(a3)
send_recv_block_block_L23_send:    bsf send_recv_block_block_L23_send    
    snd a0, t5
send_recv_block_block_L23_recv:    bre send_recv_block_block_L23_recv    
    rcvp t5
    sd t5, 176(a4)   

    ld t5, 184(a3)
send_recv_block_block_L24_send:    bsf send_recv_block_block_L24_send    
    snd a0, t5
send_recv_block_block_L24_recv:    bre send_recv_block_block_L24_recv    
    rcvp t5
    sd t5, 184(a4)   

    ld t5, 192(a3)
send_recv_block_block_L25_send:    bsf send_recv_block_block_L25_send    
    snd a0, t5
send_recv_block_block_L25_recv:    bre send_recv_block_block_L25_recv    
    rcvp t5
    sd t5, 192(a4)   

    ld t5, 200(a3)
send_recv_block_block_L26_send:    bsf send_recv_block_block_L26_send    
    snd a0, t5
send_recv_block_block_L26_recv:    bre send_recv_block_block_L26_recv    
    rcvp t5
    sd t5, 200(a4)   

    ld t5, 208(a3)
send_recv_block_block_L27_send:    bsf send_recv_block_block_L27_send    
    snd a0, t5
send_recv_block_block_L27_recv:    bre send_recv_block_block_L27_recv    
    rcvp t5
    sd t5, 208(a4)   

    ld t5, 216(a3)
send_recv_block_block_L28_send:    bsf send_recv_block_block_L28_send    
    snd a0, t5
send_recv_block_block_L28_recv:    bre send_recv_block_block_L28_recv    
    rcvp t5
    sd t5, 216(a4)   

    ld t5, 224(a3)
send_recv_block_block_L29_send:    bsf send_recv_block_block_L29_send    
    snd a0, t5
send_recv_block_block_L29_recv:    bre send_recv_block_block_L29_recv    
    rcvp t5
    sd t5, 224(a4)   

    ld t5, 232(a3)
send_recv_block_block_L30_send:    bsf send_recv_block_block_L30_send    
    snd a0, t5
send_recv_block_block_L30_recv:    bre send_recv_block_block_L30_recv    
    rcvp t5
    sd t5, 232(a4)   

    ld t5, 240(a3)
send_recv_block_block_L31_send:    bsf send_recv_block_block_L31_send    
    snd a0, t5
send_recv_block_block_L31_recv:    bre send_recv_block_block_L31_recv    
    rcvp t5
    sd t5, 240(a4)   

    ld t5, 248(a3)
send_recv_block_block_L32_send:    bsf send_recv_block_block_L32_send    
    snd a0, t5
send_recv_block_block_L32_recv:    bre send_recv_block_block_L32_recv    
    rcvp t5
    sd t5, 248(a4)   
    
    add a3, a3, 256
    add a4, a4, 256

    bne t0, zero, send_recv_block_loop
send_recv_block_end:
    ret


































