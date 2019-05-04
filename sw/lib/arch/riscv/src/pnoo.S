
.macro  snd     rs1, rs2
        .insn   r 0x5b, 0, 0, x0, \rs1, \rs2
.endm

.macro  srdy    rs1
        .insn   r 0x5b, 1, 0, x0, \rs1, x0
.endm

.macro  rcvn    rd
        .insn   r 0x5b, 2, 0, \rd, x0, x0
.endm

.macro  rcvp    rd
        .insn   r 0x5b, 3, 0, \rd, x0, x0
.endm

.macro  bsf     label
        .insn   sb 0x7b, 0, x0, x0, \label
.endm

.macro  bsnf    label
        .insn   sb 0x7b, 1, x0, x0, \label
.endm

.macro  bre     label
        .insn   sb 0x7b, 2, x0, x0, \label
.endm

.macro  brne    label
        .insn   sb 0x7b, 3, x0, x0, \label
.endm

.macro  br      rs1, label
        .insn   sb 0x7b, 4, \rs1, x0, \label
.endm

.macro  bnr     rs1, label
        .insn   sb 0x7b, 5, \rs1, x0, \label
.endm








#void pnoo_block_send(uint64_t dest, int n, void* data);
#void pnoo_block_send_no_srdy(uint64_t dest, int n, void* data);
.global pnoo_block_send
.global pnoo_block_send_no_srdy
pnoo_block_send:
    bnr a0, pnoo_block_send
pnoo_block_send_no_srdy:
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
    
#void pnoo_block_recv(uint64_t src, int n, void* data);
#void pnoo_block_recv_no_srdy(uint64_t src, int n, void* data);
.global pnoo_block_recv
.global pnoo_block_recv_no_srdy
pnoo_block_recv:
    srdy a0
pnoo_block_recv_no_srdy:
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
    
