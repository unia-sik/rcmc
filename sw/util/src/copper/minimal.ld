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
        __bss_start__ = .;
        *(.bss) 
        *(.tbss) 
        __bss_end__ = .;
    }
    .sdata : { 
        *(.sdata) 
    }
    . = ALIGN(8);
    __end__ = .;        /* heap start for _sbrk */
    . = 0x0001c000;     /* 64 KiB heap */
    __heap_top = .;     /* heap top for _sbrk */
    . = 0x00020000;     /* 16 KiB stack */
    __stack = .;        /* for crt0.S */
}

