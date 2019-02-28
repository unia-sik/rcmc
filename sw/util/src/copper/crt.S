        .section .text.crt
        .align 2
        .global _start
_start:
        # set stack pointer
        la      a0, __stack
        mv      sp, a0

        # clear bss (maybe 31 bytes too much)
        li      a0, 0

# FIXME: dont know why this does not work
#        la      a1, __bss_start__
#        la      a2, __bss_end__
        lw      a1, 3f
        lw      a2, 4f

1:

#ifdef __risc64
        sd      a0, (a1)
        sd      a0, 8(a1)
        sd      a0, 16(a1)
        sd      a0, 24(a1)
#else
        sw      a0, (a1)
        sw      a0, 4(a1)
        sw      a0, 8(a1)
        sw      a0, 12(a1)
        sw      a0, 16(a1)
        sw      a0, 20(a1)
        sw      a0, 24(a1)
        sw      a0, 28(a1)
#endif

        add     a1, a1, 32
        blt     a1, a2, 1b


        # call main(0, NULL)
        li      a0, 0
        li      a1, 0
        jal     main

        # fallthrough to _exit


        # terminate program
/*         .global _exit */
_exit:
        csrw    0x780, 1
2:      j       2b

3:	.word __bss_start__
4:	.word __bss_end__
