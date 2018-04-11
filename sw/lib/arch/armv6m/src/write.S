	.syntax unified
	.text
	.thumb
	.cpu cortex-m0




@ _write(int file, char *buf, int nbytes)
@
        .thumb_func
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






        .thumb_func
        .global _getpid
_getpid:
        bx lr

        .thumb_func
        .global _kill
_kill:
        bx lr

@        .thumb_func
@        .global _fini
@_fini:
@        bx lr


        .thumb_func
        .global _gettimeofday
_gettimeofday:
        bx lr


