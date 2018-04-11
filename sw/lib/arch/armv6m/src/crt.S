	.syntax unified
	.text
	.thumb
	.cpu cortex-m0



@ _start
@
@ Program entry point: call main(), then exit()
@
        .thumb_func
        .global _start
_start:


        @ set stack pointer
        ldr     r0, .Lstack
        mov     sp, r0

        @ clear bss (size must be a multiple of 16)
        ldr     r0, .Lstart
        ldr     r1, .Lend
        movs    r2, #0
1:      str     r2, [r0, #0]
        str     r2, [r0, #4]
        str     r2, [r0, #8]
        str     r2, [r0, #12]
        adds    r0, #16
        cmp     r0, r1
        blt     1b

        @ call main
        movs    r0, #0
        movs    r1, #0
        bl      main
        @ fallthrough to exit


@ void exit(int status)
@
@ Exit from program: breakpoint 0
@
        .thumb_func
        .global _exit
_exit:
        bkpt    #0

                .align 2
.Lstack:        .word __stack           @ from linker file
.Lstart:        .word __bss_start__     @ from linker file
.Lend:          .word __bss_end__       @ from linker file
