/* ld file with very much memory for NBP benchmarks (e.g. DT BH A) */
OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS
{
    . = 0x00000000;
    .text : {
        *(.text.crt)
        *(.text)
      }
    .bss : {
        *(.data)
        . = ALIGN(32);  /* clear function writes 32 bytes at once */
        __bss_start__ = .;
        *(.bss) 
        *(.tbss) 
        . = ALIGN(32);
        __bss_end__ = .;
    }
    .sdata : { 
        *(.sdata) 
    }
    . = ALIGN(8);
    __end__ = .;        /* heap start for _sbrk */
    . = 0x70000000;     /* 1.5 GiB heap */
    __heap_top = .;     /* heap top for _sbrk */
    . = 0x7f000000;     /* 250 MiB stack */
    __stack = .;        /* for crt0.S */
}

