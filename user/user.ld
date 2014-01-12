ENTRY(_start)

SECTIONS
{
  /*
   * All user programs should be loaded starting at 1GB because the kernel
   * occupies virtual addresses [0, 1GB).
   */
  . = 0x40000000;

  .text :
  {
    *(.text)
  }
  .rodata :
  {
    *(.rodata)
  }
  .data :
  {
    *(.data)
  }
  .bss :
  {
    *(.bss)
  }
}
