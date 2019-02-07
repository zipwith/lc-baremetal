OUTPUT_FORMAT(elf32-i386)
ENTRY(entry)

SECTIONS {
  . = 0x500000;
  .text ALIGN(0x1000) : {
    _text_start = .; *(.text) _text_end = .;
    *(.rodata)
    *(.data)
    _start_bss = .; *(COMMON) *(.bss) _end_bss = .;
  }
}
