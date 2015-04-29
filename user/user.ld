ENTRY(_start)

SECTIONS
{
  /*
   * All user programs should be loaded starting at 1GB because the kernel
   * occupies virtual addresses [0, 1GB).
   */
  . = 0x40000000;

  .text : ALIGN(4K)
  {
    *(.text)
  }
  .rodata : ALIGN(4K)
  {
    *(.rodata)
  }
  .data : ALIGN(4K)
  {
    *(.data)
  }
  .bss : ALIGN(4K)
  {
    *(.bss)
  }
  _end = .;
}
