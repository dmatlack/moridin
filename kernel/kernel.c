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

#include <mm/mem.h>
#include <mm/vm.h>

#include <kernel/proc.h>

void kernel_main() {
  kprintf("Welcome to kernel_main()!\n");

  /* 
   * Initialize hardware interrupts by first telling the PIC where in the IDT it 
   * can find its interrupts handlers, and then installing the necessary interrupts 
   * handlers for each device connected to the PIC.
   */
  if (pic_init(IDT_PIC_MASTER_OFFSET, IDT_PIC_SLAVE_OFFSET)) {
    panic("Unable to initialize the PIC.\n");
  }

  if (kmalloc_init()) {
    panic("Unable to initialize the kernel dynamic memory allocator.\n");
  }

  if (vm_init()) {
    panic("Unable to initialize the virtual memory layer.\n");
  }

  mem_layout_dump(kprintf);

  while (1);
}
