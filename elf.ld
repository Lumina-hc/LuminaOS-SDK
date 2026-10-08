OUTPUT_FORMAT("elf64-x86-64")
ENTRY(_start)

USER_BASE = 0x08048000;

SECTIONS
{
    . = USER_BASE;

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
