/**
 * @file kernel.c
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <debug.h>

#include <dev/vga.h>

#include <x86/exn.h>
#include <x86/pic.h>
#include <x86/io.h>

#include <mm/mem.h>

void kernel_main() {

  /* 
   * initialize the x86 exception handling facilities and install the kernel's
   * exception handler
   */
  if (x86_exn_init(kernel_x86_exn_handler)) {
    panic("Unable to install x86 exception handlers.\n");
  }

  /* 
   * Initialize hardware interrupts by first telling the PIC where in the IDT it 
   * can find its interrupts handlers, and then installing the necessary interrupts 
   * handlers for each device connected to the PIC.
   */
  if (pic_init(IDT_PIC_MASTER_OFFSET, IDT_PIC_SLAVE_OFFSET)) {
    panic("Unable to initialize the PIC.\n");
  }

  // debug printing
  dprintf("Hello debug console, this is the kernel!\n");
  kprintf("Hello vga console, this is the kernel!\n");

  kprintf("Allocating chunk of size %d: %p\n", 1024, kmalloc(1024));
  kprintf("Allocating chunk of size %d: %p\n", 1024, kmalloc(1024));
  kprintf("Allocating chunk of size %d: %p\n", 1024, kmalloc(1024));
  kprintf("Allocating chunk of size %d: %p\n", 1024, kmalloc(1024));
  mem_layout_dump(kprintf);

  while (1) iodelay();
}
