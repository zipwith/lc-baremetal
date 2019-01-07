OUTPUT_FORMAT(elf32-i386)
ENTRY(entry)

/* By default, mimgload is loaded at 2MB so that a proper kernel can be
 * loaded at 1MB, right at the start of high memory.  Of course, if the
 * kernel needs more than 1MB, then we will need to load mimgload at a
 * higher address.  Once the kernel has started, the kernel can use all
 * of high memory beyond the end of the kernel as a single block of
 * memory.
 */
SECTIONS {
  . = 0x200000; /* Load mimgload at 2MB */
  .text : {
    _text_start = .; *(.multiboot) *(.text) _text_end = .;
    _data_start = .; *(.rodata) *(.data)    _data_end = .;
    _bss_start  = .; *(COMMON) *(.bss)      _bss_end  = .;
  }
}
