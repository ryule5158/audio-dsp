ENTRY(main)

MEMORY
{
  FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 16M
  RAM   (rwx) : ORIGIN = 0x24000000, LENGTH = 16M
}

SECTIONS
{
  .text :
  {
    *(.text*)
    *(.rodata*)
    *(.glue_7*)
    *(.glue_7t*)
  } > FLASH

  .ARM.extab : { *(.ARM.extab* .gnu.linkonce.armextab.*) } > FLASH
  .ARM.exidx : { *(.ARM.exidx* .gnu.linkonce.armexidx.*) } > FLASH

  .data :
  {
    _sdata = .;
    *(.data*)
    _edata = .;
  } > RAM AT > FLASH

  .bss (NOLOAD) :
  {
    _sbss = .;
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM

  end = .;
  _end = .;
  _estack = ORIGIN(RAM) + LENGTH(RAM);
}
