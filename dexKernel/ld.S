OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

ENTRY(_start)

SECTIONS
{
  . = 0xC8000000;
  .text       : { *(.text) }
  .bss        : { *(.bss) }
  .rodata     : { *(.rodata) }
  .data       : { *(.data) }
  __end__ = .;
}