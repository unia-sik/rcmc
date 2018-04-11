	.syntax unified
	.text
	.cpu cortex-a15




@ _write(int file, char *buf, int nbytes)
@
        .global _write
_write:
        mov     r3, r2
        b       2f
1:
        ldrb    r0, [r1]
        svc     #1
        adds    r1, #1
        subs    r2, #1
2:
        cmp     r2, #0
        bhi     1b
        mov     r0, r3
        bx lr






        .global _getpid
_getpid:
        bx lr

        .global _kill
_kill:
        bx lr

@        .global _fini
@_fini:
@        bx lr


        .global _gettimeofday
_gettimeofday:
        bx lr


