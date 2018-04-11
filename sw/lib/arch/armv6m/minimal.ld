ENTRY(_start)
SECTIONS
{
    . = 0x00008000;
    .text : {
        *(.text)
        *(.rodata)
    }
    .bss : {
        *(.data)
        . = ALIGN(16);  /* clear function (crt.s) write 16 bytes at once */
        __bss_start__ = .;
        *(.bss)
        *(COMMON)
        . = ALIGN(16);
        __bss_end__ = .;
    }
    .sdata : { 
        *(.sdata) 
    }
    . = ALIGN(8);
    __end__ = .;        /* heap start for _sbrk */
    . = 0x00700000;     /* 6 MiB heap */
    __heap_top = .;     /* heap top for _sbrk */
    . = 0x00800000;     /* 1 MiB stack */
    __stack = .;        /* for crt0.S */
}
