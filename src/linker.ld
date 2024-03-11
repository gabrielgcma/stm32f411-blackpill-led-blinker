ENTRY(reset_handler)
MEMORY{
  FLASH(rx): ORIGIN = 0x08000000, LENGTH =512K
  SRAM(rwx): ORIGIN = 0x20000000, LENGTH =128K
}

SECTIONS{
  .text :
  {
    KEEP(*(.isr_vectors))
    *(.text)
    . = ALIGN(4);
    *(.rodata)
    . = ALIGN(4);
    _etext = .;
    _la_data = _etext;
  }> FLASH

  .data : {
    _sdata = .; /* Location counter always tracks the VMA of the section, not the LMA */
    *(.data)
    . = ALIGN(4);
    _edata = .;
  }>SRAM AT> FLASH

  .bss : {
    _sbss = .;
    *(.bss)
    . = ALIGN(4);
    _ebss = .;
  }> SRAM
}

