.section ".entry"
.global _start
_start:
    csrr a0,0xc71
    lui sp, 1
    
    add a0, zero, 0xAB
    sw a0, 0(sp)
    lw a0, 0(sp)    
    
    lui gp, 0xFFFF
    jal main
    add gp, zero, 0x00
    
.global exit
exit:
    csrw 0x782, 10
    csrw 0x780, 1

exit_loop:
    j exit_loop

.global assert
assert:
    beq a0, zero, assert_fail
    ret
assert_fail:
    add ra, ra, -4
    add gp, zero, ra
    jal exit
    
.global set_test_result
set_test_result:
    add gp, zero, a0
    ret
       
.global delay
delay:
    add t0, zero, a0
    add t1, zero, 0
delay_l1:
    beq t0, t1, delay_l2
    add t1, t1, 1
    j delay_l1
delay_l2:
    ret
    
.global clock
clock:
    rdcycle a0
    ret

.global get_cid
get_cid:
    csrr    a0,0xc71
    nop
    ret
    
.global get_max_cid
get_max_cid:
    csrr    a0, 0xC70
    nop
    ret
    
.global get_dim
get_dim:
    csrr    a0, 0xC72
    nop
    add t0, zero, 1
    sll t0, t0, 16
    add t0, t0, -1
    and a0, a0, t0
    ret
