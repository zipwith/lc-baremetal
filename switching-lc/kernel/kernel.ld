OUTPUT_FORMAT(elf32-i386)
ENTRY(entry)

SECTIONS {
  . = 0x100000;
  .text ALIGN(0x1000) : {
    _text_start = .; *(.multiboot) *(.text) _text_end = .;
    *(.rodata)
    *(.data)
    _start_bss = .; *(COMMON) *(.bss) _end_bss = .;
  }

  .init ALIGN(0x1000) : {
    *(.initdata)
  }
}
