OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS
{
    . = 0x00000000;
    .text : {
        *(.entry)
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
    . = 0x0001c000;     /* 64 KiB heap */
    __heap_top = .;     /* heap top for _sbrk */
    . = 0x00020000;     /* 16 KiB stack */
    __stack = .;        /* for crt0.S */
}
/*OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS
{
    . = 0;
    .text : {
        
        *(.text)
      }
    . = 0x80003000;

    .data : {
	*(.data)
    }
    .bss : {
        *(.bss)	
    }
}
*/
