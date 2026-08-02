OUTPUT_FORMAT("elf32-i386")
ENTRY(_start)

USER_BASE = 0x01000000;
LSP_HDR = 0x10;

SECTIONS
{
    . = USER_BASE + LSP_HDR;

    .text : {
        *(.text*)
    }
    .rodata : {
        *(.rodata*)
    }
    .data : {
        *(.data*)
    }
    .bss : {
        __bss_start = .;
        *(COMMON)
        *(.bss*)
        __bss_end = .;
    }

    /DISCARD/ : {
        *(.note*)
        *(.comment*)
    }
}
