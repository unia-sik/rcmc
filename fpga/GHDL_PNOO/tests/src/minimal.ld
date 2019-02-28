OUTPUT_ARCH("riscv")
ENTRY(_start)
SECTIONS
{
    . = 0;
    .text : {
        *(.entry)
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

