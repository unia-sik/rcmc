#void fgmp_block_send_16(uint64_t dest, int n, void* data);
#void fgmp_block_send_no_srdy_16(uint64_t dest, int n, void* data);
.global fgmp_block_send_16
.global fgmp_block_send_no_srdy_16
fgmp_block_send_16:
    bnr a0, fgmp_block_send_16
fgmp_block_send_no_srdy_16:
    add t0, zero, a1
    add t2, zero, 256
send_block_loop_16:   
    sub t1, t2, t0
    add t0, t0, -256
    blt t1, zero, send_block_block_L0_16
    add t0, zero, zero
    
    srl t1, t1, 3 # number of flits to ignore
    sll t1, t1, 2 # each instr is 4 Byte    
    
    sub a2, a2, t1
    sub a2, a2, t1
    
    auipc t4, 0
    add t4, t4, t1 # jump over 6 * 4 * #ignored_flits
    add t4, t4, t1
    add t4, t4, t1
    
    add t4, t4, t1 
    add t4, t4, t1
    add t4, t4, t1
    jr 32(t4)
   
send_block_block_L0_16:
    lh t5, 0(a2)
    sll t5, t5, 48
    or t6, t5, t5
    lh t5, 2(a2)
    sll t5, t5, 32
    or t6, t5, t5
    lh t5, 4(a2)
    sll t5, t5, 16
    or t6, t5, t5
    lh t5, 6(a2)
    or t6, t5, t5
send_block_block_L1_16:    bsf send_block_block_L1_16    
    snd a0, t5
    
    lw t5, 8(a2)
    lw t6, 12(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L2_16:    bsf send_block_block_L2_16  
    snd a0, t5
    
    lw t5, 16(a2)
    lw t6, 20(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L3_16:    bsf send_block_block_L3_16    
    snd a0, t5
    
    lw t5, 24(a2)
    lw t6, 28(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L4_16:    bsf send_block_block_L4_16    
    snd a0, t5
    
    lw t5, 32(a2)
    lw t6, 36(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L5_16:    bsf send_block_block_L5_16    
    snd a0, t5
    
    lw t5, 40(a2)
    lw t6, 44(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L6_16:    bsf send_block_block_L6_16    
    snd a0, t5
    
    lw t5, 48(a2)
    lw t6, 52(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L7_16:    bsf send_block_block_L7_16    
    snd a0, t5
    
    lw t5, 56(a2)
    lw t6, 60(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L8_16:    bsf send_block_block_L8_16    
    snd a0, t5
    
    lw t5, 64(a2)
    lw t6, 68(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L9_16:    bsf send_block_block_L9_16    
    snd a0, t5
    
    lw t5, 72(a2)
    lw t6, 76(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L10_16:    bsf send_block_block_L10_16    
    snd a0, t5
    
    lw t5, 80(a2)
    lw t6, 84(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L11_16:    bsf send_block_block_L11_16   
    snd a0, t5
    
    lw t5, 88(a2)
    lw t6, 92(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L12_16:    bsf send_block_block_L12_16    
    snd a0, t5
    
    lw t5, 96(a2)
    lw t6, 100(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L13_16:    bsf send_block_block_L13_16    
    snd a0, t5
    
    lw t5, 104(a2)
    lw t6, 108(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L14_16:    bsf send_block_block_L14_16 
    snd a0, t5
    
    lw t5, 112(a2)
    lw t6, 116(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L15_16:    bsf send_block_block_L15_16   
    snd a0, t5
    
    lw t5, 120(a2)
    lw t6, 124(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L16_16:    bsf send_block_block_L16_16    
    snd a0, t5
    
    lw t5, 128(a2)
    lw t6, 132(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L17_16:    bsf send_block_block_L17_16    
    snd a0, t5
    
    lw t5, 136(a2)
    lw t6, 140(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L18_16:    bsf send_block_block_L18_16    
    snd a0, t5
    
    lw t5, 144(a2)
    lw t6, 148(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L19_16:    bsf send_block_block_L19_16    
    snd a0, t5
    
    lw t5, 152(a2)
    lw t6, 156(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L20_16:    bsf send_block_block_L20_16    
    snd a0, t5
    
    lw t5, 160(a2)
    lw t6, 164(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L21_16:    bsf send_block_block_L21_16    
    snd a0, t5
    
    lw t5, 168(a2)
    lw t6, 172(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L22_16:    bsf send_block_block_L22_16    
    snd a0, t5
    
    lw t5, 176(a2)
    lw t6, 180(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L23_16:    bsf send_block_block_L23_16    
    snd a0, t5
    
    lw t5, 184(a2)
    lw t6, 188(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L24_16:    bsf send_block_block_L24_16    
    snd a0, t5
    
    lw t5, 192(a2)
    lw t6, 196(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L25_16:    bsf send_block_block_L25_16    
    snd a0, t5
    
    lw t5, 200(a2)
    lw t6, 204(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L26_16:    bsf send_block_block_L26_16    
    snd a0, t5
    
    lw t5, 208(a2)
    lw t6, 212(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L27_16:    bsf send_block_block_L27_16    
    snd a0, t5
    
    lw t5, 216(a2)
    lw t6, 220(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L28_16:    bsf send_block_block_L28_16    
    snd a0, t5
    
    lw t5, 224(a2)
    lw t6, 228(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L29_16:    bsf send_block_block_L29_16    
    snd a0, t5
    
    lw t5, 232(a2)
    lw t6, 236(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L30_16:    bsf send_block_block_L30_16    
    snd a0, t5
    
    lw t5, 240(a2)
    lw t6, 244(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L31_16:    bsf send_block_block_L31_16    
    snd a0, t5
    
    lw t5, 248(a2)
    lw t6, 252(a2)
    sll t5, t5, 32
    or t5, t5, t6
send_block_block_L32_16:    bsf send_block_block_L32_16    
    snd a0, t5
    
    add a2, a2, 256

    bne t0, zero, send_block_loop_16
send_block_end_16:
    ret
    
#void fgmp_block_recv_16(uint64_t src, int n, void* data);
#void fgmp_block_recv_no_srdy_16(uint64_t src, int n, void* data);
.global fgmp_block_recv_16
.global fgmp_block_recv_no_srdy_16
fgmp_block_recv_16:
    srdy a0
fgmp_block_recv_no_srdy_16:
    add t0, zero, a1
    add t2, zero, 256
recv_block_loop_16:   
    sub t1, t2, t0
    add t0, t0, -256
    blt t1, zero, recv_block_block_L0_16
    add t0, zero, zero
    
    srl t1, t1, 3 # number of flits to ignore
    sll t1, t1, 2 # each instr is 4 Byte    
    
    sub a2, a2, t1
    sub a2, a2, t1
    
    auipc t4, 0
    add t4, t4, t1 # jump over 5 * 4 * #ignored_flits
    add t4, t4, t1
    add t4, t4, t1
    
    add t4, t4, t1
    add t4, t4, t1
    jr 28(t4)
   
recv_block_block_L0_16:
recv_block_block_L1_16:    bre recv_block_block_L1_16    
    rcvp a0
    sw a0, 4(a2)
    srl a0, a0, 32
    sw a0, 0(a2)
    
recv_block_block_L2_16:    bre recv_block_block_L2_16
    rcvp a0
    sw a0, 12(a2)
    srl a0, a0, 32
    sw a0, 8(a2)
    
recv_block_block_L3_16:    bre recv_block_block_L3_16    
    rcvp a0
    sw a0, 20(a2)
    srl a0, a0, 32
    sw a0, 16(a2)
    
recv_block_block_L4_16:    bre recv_block_block_L4_16    
    rcvp a0
    sw a0, 28(a2)
    srl a0, a0, 32
    sw a0, 24(a2)
    
recv_block_block_L5_16:    bre recv_block_block_L5_16    
    rcvp a0
    sw a0, 36(a2)
    srl a0, a0, 32
    sw a0, 32(a2)
    
recv_block_block_L6_16:    bre recv_block_block_L6_16    
    rcvp a0
    sw a0, 44(a2)
    srl a0, a0, 32
    sw a0, 40(a2)
    
recv_block_block_L7_16:    bre recv_block_block_L7_16    
    rcvp a0
    sw a0, 52(a2)
    srl a0, a0, 32
    sw a0, 48(a2)
    
recv_block_block_L8_16:    bre recv_block_block_L8_16    
    rcvp a0
    sw a0, 60(a2)
    srl a0, a0, 32
    sw a0, 56(a2)
    
recv_block_block_L9_16:    bre recv_block_block_L9_16    
    rcvp a0
    sw a0, 68(a2)
    srl a0, a0, 32
    sw a0, 64(a2)
    
recv_block_block_L10_16:    bre recv_block_block_L10_16    
    rcvp a0
    sw a0, 76(a2)
    srl a0, a0, 32
    sw a0, 72(a2)
    
recv_block_block_L11_16:    bre recv_block_block_L11_16   
    rcvp a0
    sw a0, 84(a2)
    srl a0, a0, 32
    sw a0, 80(a2)
    
recv_block_block_L12_16:    bre recv_block_block_L12_16    
    rcvp a0
    sw a0, 92(a2)
    srl a0, a0, 32
    sw a0, 88(a2)
    
recv_block_block_L13_16:    bre recv_block_block_L13_16    
    rcvp a0
    sw a0, 100(a2)
    srl a0, a0, 32
    sw a0, 96(a2)
    
recv_block_block_L14_16:    bre recv_block_block_L14_16 
    rcvp a0
    sw a0, 108(a2)
    srl a0, a0, 32
    sw a0, 104(a2)
    
recv_block_block_L15_16:    bre recv_block_block_L15_16   
    rcvp a0
    sw a0, 116(a2)
    srl a0, a0, 32
    sw a0, 112(a2)
    
recv_block_block_L16_16:    bre recv_block_block_L16_16    
    rcvp a0
    sw a0, 124(a2)
    srl a0, a0, 32
    sw a0, 120(a2)
    
recv_block_block_L17_16:    bre recv_block_block_L17_16    
    rcvp a0
    sw a0, 132(a2)
    srl a0, a0, 32
    sw a0, 128(a2)
    
recv_block_block_L18_16:    bre recv_block_block_L18_16    
    rcvp a0
    sw a0, 140(a2)
    srl a0, a0, 32
    sw a0, 136(a2)
    
recv_block_block_L19_16:    bre recv_block_block_L19_16    
    rcvp a0
    sw a0, 148(a2)
    srl a0, a0, 32
    sw a0, 144(a2)
    
recv_block_block_L20_16:    bre recv_block_block_L20_16    
    rcvp a0
    sw a0, 156(a2)
    srl a0, a0, 32
    sw a0, 152(a2)
    
recv_block_block_L21_16:    bre recv_block_block_L21_16    
    rcvp a0
    sw a0, 164(a2)
    srl a0, a0, 32
    sw a0, 160(a2)
    
recv_block_block_L22_16:    bre recv_block_block_L22_16    
    rcvp a0
    sw a0, 172(a2)
    srl a0, a0, 32
    sw a0, 168(a2)
    
recv_block_block_L23_16:    bre recv_block_block_L23_16    
    rcvp a0
    sw a0, 180(a2)
    srl a0, a0, 32
    sw a0, 176(a2)
    
recv_block_block_L24_16:    bre recv_block_block_L24_16    
    rcvp a0
    sw a0, 188(a2)
    srl a0, a0, 32
    sw a0, 184(a2)
    
recv_block_block_L25_16:    bre recv_block_block_L25_16    
    rcvp a0
    sw a0, 196(a2)
    srl a0, a0, 32
    sw a0, 192(a2)
    
recv_block_block_L26_16:    bre recv_block_block_L26_16    
    rcvp a0
    sw a0, 204(a2)
    srl a0, a0, 32
    sw a0, 200(a2)
    
recv_block_block_L27_16:    bre recv_block_block_L27_16    
    rcvp a0
    sw a0, 212(a2)
    srl a0, a0, 32
    sw a0, 208(a2)
    
recv_block_block_L28_16:    bre recv_block_block_L28_16    
    rcvp a0
    sw a0, 220(a2)
    srl a0, a0, 32
    sw a0, 216(a2)
    
recv_block_block_L29_16:    bre recv_block_block_L29_16    
    rcvp a0
    sw a0, 228(a2)
    srl a0, a0, 32
    sw a0, 224(a2)
    
recv_block_block_L30_16:    bre recv_block_block_L30_16    
    rcvp a0
    sw a0, 236(a2)
    srl a0, a0, 32
    sw a0, 232(a2)
    
recv_block_block_L31_16:    bre recv_block_block_L31_16    
    rcvp a0
    sw a0, 244(a2)
    srl a0, a0, 32
    sw a0, 240(a2)
    
recv_block_block_L32_16:    bre recv_block_block_L32_16    
    rcvp a0
    sw a0, 252(a2)
    srl a0, a0, 32
    sw a0, 248(a2)    
    
    add a2, a2, 256

    bne t0, zero, recv_block_loop_16
recv_block_end_16_16:
    ret
    
#void fgmp_block_send_recv(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);
#void fgmp_block_send_recv_no_srdy(uint64_t dest, uint64_t src, int n, void* data_send, void* data_recv);
.global fgmp_block_send_recv_16
.global fgmp_block_send_recv_no_srdy_16
fgmp_block_send_recv_16:
    srdy a1
send_recv_block_srdy_16:    bnr a0, send_recv_block_srdy_16
fgmp_block_send_recv_no_srdy_16:
    add t0, zero, a2
    add t2, zero, 256
send_recv_block_loop_16:   
    sub t1, t2, t0
    add t0, t0, -256
    blt t1, zero, send_recv_block_block_L0_16
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

send_recv_block_block_L0_16:
    lw t5, 0(a3)
    lw t6, 4(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L1_send_16:    bsf send_recv_block_block_L1_send_16
    snd a0, t5
send_recv_block_block_L1_recv_16:    bre send_recv_block_block_L1_recv_16
    rcvp t5
    sw t5, 4(a4)
    srl t5, t5, 32
    sw t5, 0(a4)    

    lw t5, 8(a3)
    lw t6, 12(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L2_send_16:    bsf send_recv_block_block_L2_send_16
    snd a0, t5
send_recv_block_block_L2_recv_16:    bre send_recv_block_block_L2_recv_16
    rcvp t5
    sw t5, 12(a4)
    srl t5, t5, 32
    sw t5, 8(a4)   

    lw t5, 16(a3)
    lw t6, 20(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L3_send_16:    bsf send_recv_block_block_L3_send_16
    snd a0, t5
send_recv_block_block_L3_recv_16:    bre send_recv_block_block_L3_recv_16
    rcvp t5
    sw t5, 20(a4)
    srl t5, t5, 32
    sw t5, 16(a4)   

    lw t5, 24(a3)
    lw t6, 28(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L4_send_16:    bsf send_recv_block_block_L4_send_16
    snd a0, t5
send_recv_block_block_L4_recv_16:    bre send_recv_block_block_L4_recv_16
    rcvp t5
    sw t5, 28(a4)
    srl t5, t5, 32
    sw t5, 24(a4)   

    lw t5, 32(a3)
    lw t6, 36(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L5_send_16:    bsf send_recv_block_block_L5_send_16
    snd a0, t5
send_recv_block_block_L5_recv_16:    bre send_recv_block_block_L5_recv_16
    rcvp t5
    sw t5, 36(a4)
    srl t5, t5, 32
    sw t5, 32(a4)   

    lw t5, 40(a3)
    lw t6, 44(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L6_send_16:    bsf send_recv_block_block_L6_send_16
    snd a0, t5
send_recv_block_block_L6_recv_16:    bre send_recv_block_block_L6_recv_16
    rcvp t5
    sw t5, 44(a4)
    srl t5, t5, 32
    sw t5, 40(a4)   

    lw t5, 48(a3)
    lw t6, 52(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L7_send_16:    bsf send_recv_block_block_L7_send_16
    snd a0, t5
send_recv_block_block_L7_recv_16:    bre send_recv_block_block_L7_recv_16
    rcvp t5
    sw t5, 52(a4)
    srl t5, t5, 32
    sw t5, 48(a4)   

    lw t5, 56(a3)
    lw t6, 60(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L8_send_16:    bsf send_recv_block_block_L8_send_16
    snd a0, t5
send_recv_block_block_L8_recv_16:    bre send_recv_block_block_L8_recv_16
    rcvp t5
    sw t5, 60(a4)
    srl t5, t5, 32
    sw t5, 56(a4)   

    lw t5, 64(a3)
    lw t6, 68(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L9_send_16:    bsf send_recv_block_block_L9_send_16
    snd a0, t5
send_recv_block_block_L9_recv_16:    bre send_recv_block_block_L9_recv_16
    rcvp t5
    sw t5, 68(a4)
    srl t5, t5, 32
    sw t5, 64(a4)   

    lw t5, 72(a3)
    lw t6, 76(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L10_send_16:    bsf send_recv_block_block_L10_send_16
    snd a0, t5
send_recv_block_block_L10_recv_16:    bre send_recv_block_block_L10_recv_16
    rcvp t5
    sw t5, 76(a4)
    srl t5, t5, 32
    sw t5, 72(a4)   

    lw t5, 80(a3)
    lw t6, 84(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L11_send_16:    bsf send_recv_block_block_L11_send_16
    snd a0, t5
send_recv_block_block_L11_recv_16:    bre send_recv_block_block_L11_recv_16
    rcvp t5
    sw t5, 84(a4)
    srl t5, t5, 32
    sw t5, 80(a4)   

    lw t5, 88(a3)
    lw t6, 92(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L12_send_16:    bsf send_recv_block_block_L12_send_16
    snd a0, t5
send_recv_block_block_L12_recv_16:    bre send_recv_block_block_L12_recv_16
    rcvp t5
    sw t5, 92(a4)
    srl t5, t5, 32
    sw t5, 88(a4)   

    lw t5, 96(a3)
    lw t6, 100(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L13_send_16:    bsf send_recv_block_block_L13_send_16
    snd a0, t5
send_recv_block_block_L13_recv_16:    bre send_recv_block_block_L13_recv_16
    rcvp t5
    sw t5, 100(a4)
    srl t5, t5, 32
    sw t5, 96(a4)   

    lw t5, 104(a3)
    lw t6, 108(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L14_send_16:    bsf send_recv_block_block_L14_send_16
    snd a0, t5
send_recv_block_block_L14_recv_16:    bre send_recv_block_block_L14_recv_16
    rcvp t5
    sw t5, 108(a4)
    srl t5, t5, 32
    sw t5, 104(a4)   

    lw t5, 112(a3)
    lw t6, 116(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L15_send_16:    bsf send_recv_block_block_L15_send_16
    snd a0, t5
send_recv_block_block_L15_recv_16:    bre send_recv_block_block_L15_recv_16
    rcvp t5
    sw t5, 116(a4)
    srl t5, t5, 32
    sw t5, 112(a4)   

    lw t5, 120(a3)
    lw t6, 124(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L16_send_16:    bsf send_recv_block_block_L16_send_16
    snd a0, t5
send_recv_block_block_L16_recv_16:    bre send_recv_block_block_L16_recv_16
    rcvp t5
    sw t5, 124(a4)
    srl t5, t5, 32
    sw t5, 120(a4)   

    lw t5, 128(a3)
    lw t6, 132(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L17_send_16:    bsf send_recv_block_block_L17_send_16
    snd a0, t5
send_recv_block_block_L17_recv_16:    bre send_recv_block_block_L17_recv_16
    rcvp t5
    sw t5, 132(a4)
    srl t5, t5, 32
    sw t5, 128(a4)   

    lw t5, 136(a3)
    lw t6, 140(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L18_send_16:    bsf send_recv_block_block_L18_send_16
    snd a0, t5
send_recv_block_block_L18_recv_16:    bre send_recv_block_block_L18_recv_16
    rcvp t5
    sw t5, 140(a4)
    srl t5, t5, 32
    sw t5, 136(a4)   

    lw t5, 144(a3)
    lw t6, 148(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L19_send_16:    bsf send_recv_block_block_L19_send_16
    snd a0, t5
send_recv_block_block_L19_recv_16:    bre send_recv_block_block_L19_recv_16
    rcvp t5
    sw t5, 148(a4)
    srl t5, t5, 32
    sw t5, 144(a4)   

    lw t5, 152(a3)
    lw t6, 156(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L20_send_16:    bsf send_recv_block_block_L20_send_16
    snd a0, t5
send_recv_block_block_L20_recv_16:    bre send_recv_block_block_L20_recv_16
    rcvp t5
    sw t5, 156(a4)
    srl t5, t5, 32
    sw t5, 152(a4)   

    lw t5, 160(a3)
    lw t6, 164(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L21_send_16:    bsf send_recv_block_block_L21_send_16
    snd a0, t5
send_recv_block_block_L21_recv_16:    bre send_recv_block_block_L21_recv_16
    rcvp t5
    sw t5, 164(a4)
    srl t5, t5, 32
    sw t5, 160(a4)   

    lw t5, 168(a3)
    lw t6, 172(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L22_send_16:    bsf send_recv_block_block_L22_send_16
    snd a0, t5
send_recv_block_block_L22_recv_16:    bre send_recv_block_block_L22_recv_16
    rcvp t5
    sw t5, 172(a4)
    srl t5, t5, 32
    sw t5, 168(a4)   

    lw t5, 176(a3)
    lw t6, 180(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L23_send_16:    bsf send_recv_block_block_L23_send_16
    snd a0, t5
send_recv_block_block_L23_recv_16:    bre send_recv_block_block_L23_recv_16
    rcvp t5
    sw t5, 180(a4)
    srl t5, t5, 32
    sw t5, 176(a4)   

    lw t5, 184(a3)
    lw t6, 188(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L24_send_16:    bsf send_recv_block_block_L24_send_16
    snd a0, t5
send_recv_block_block_L24_recv_16:    bre send_recv_block_block_L24_recv_16
    rcvp t5
    sw t5, 188(a4)
    srl t5, t5, 32
    sw t5, 184(a4)   

    lw t5, 192(a3)
    lw t6, 196(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L25_send_16:    bsf send_recv_block_block_L25_send_16
    snd a0, t5
send_recv_block_block_L25_recv_16:    bre send_recv_block_block_L25_recv_16
    rcvp t5
    sw t5, 196(a4)
    srl t5, t5, 32
    sw t5, 192(a4)   

    lw t5, 200(a3)
    lw t6, 204(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L26_send_16:    bsf send_recv_block_block_L26_send_16
    snd a0, t5
send_recv_block_block_L26_recv_16:    bre send_recv_block_block_L26_recv_16
    rcvp t5
    sw t5, 204(a4)
    srl t5, t5, 32
    sw t5, 200(a4)   

    lw t5, 208(a3)
    lw t6, 212(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L27_send_16:    bsf send_recv_block_block_L27_send_16
    snd a0, t5
send_recv_block_block_L27_recv_16:    bre send_recv_block_block_L27_recv_16
    rcvp t5
    sw t5, 212(a4)
    srl t5, t5, 32
    sw t5, 208(a4)   

    lw t5, 216(a3)
    lw t6, 220(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L28_send_16:    bsf send_recv_block_block_L28_send_16
    snd a0, t5
send_recv_block_block_L28_recv_16:    bre send_recv_block_block_L28_recv_16
    rcvp t5
    sw t5, 220(a4)
    srl t5, t5, 32
    sw t5, 216(a4)   

    lw t5, 224(a3)
    lw t6, 228(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L29_send_16:    bsf send_recv_block_block_L29_send_16
    snd a0, t5
send_recv_block_block_L29_recv_16:    bre send_recv_block_block_L29_recv_16
    rcvp t5
    sw t5, 228(a4)
    srl t5, t5, 32
    sw t5, 224(a4)   

    lw t5, 232(a3)
    lw t6, 236(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L30_send_16:    bsf send_recv_block_block_L30_send_16
    snd a0, t5
send_recv_block_block_L30_recv_16:    bre send_recv_block_block_L30_recv_16
    rcvp t5
    sw t5, 236(a4)
    srl t5, t5, 32
    sw t5, 232(a4)   

    lw t5, 240(a3)
    lw t6, 244(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L31_send_16:    bsf send_recv_block_block_L31_send_16
    snd a0, t5
send_recv_block_block_L31_recv_16:    bre send_recv_block_block_L31_recv_16
    rcvp t5
    sw t5, 244(a4)
    srl t5, t5, 32
    sw t5, 240(a4)   

    lw t5, 248(a3)
    lw t6, 252(a3)
    sll t5, t5, 32
    or t5, t5, t6
send_recv_block_block_L32_send_16:    bsf send_recv_block_block_L32_send_16
    snd a0, t5
send_recv_block_block_L32_recv_16:    bre send_recv_block_block_L32_recv_16
    rcvp t5
    sw t5, 252(a4)
    srl t5, t5, 32
    sw t5, 248(a4)   
    
    add a3, a3, 256
    add a4, a4, 256

    bne t0, zero, send_recv_block_loop_16
send_recv_block_end_16_16:
    ret




































