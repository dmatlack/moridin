/**
 * @file kernel.c
 *
 * Welcome. Have a look around.
 *
 */
#include <kernel/config.h>
#include <kernel/debug.h>
#include <kernel/init.h>
#include <kernel/irq.h>
#include <kernel/loader.h>
#include <kernel/proc.h>
#include <kernel/stack.h>
#include <kernel/sched.h>
#include <kernel/test.h>

#include <mm/kmalloc.h>
#include <mm/memory.h>
#include <mm/pages.h>
#include <mm/vm.h>
#include <mm/kmap.h>

#include <dev/vga.h>
#include <dev/serial.h>
#include <dev/pci.h>

#include <assert.h>
#include <errno.h>

#include <fs/initrd.h>
#include <fs/vfs.h>

/**
 * @brief This is main logical entry point for the kernel, not to be confused
 * with the actual entry point, _start. Also not to be confused the multiboot
 * entry point mb_entry. Ok so it's not the entry point, but it is an entry
 * point.
 */
void kernel_main()
{
	debug_init();

	/*
	 * Set up kmalloc to only allocate dynamic memory in the first 16 MB of
	 * memory. This will allow us to use kmalloc during early startup.
	 *
	 * NOTE: if we use a higher half kernel we'll have to offset these
	 * values
	 */
	kmalloc_early_init();

	pages_init();
	vm_init();
	kmap_init();
	initrd_init();
	pci_init();

	///////////////////////////////////////////////////////////////////////
	// Temporary hack to get to userspace with some test argv/arc
	//////////////////////////////////////////////////////////////////////
	{
		#define INIT "/fork_test"
		char *argv[4] = { INIT, "arg1", "arg2", ":)" };
		int argc = 4;

		// TODO: pass in the name of the init binary as a parameter via the
		// bootloader rather than hardcopying it.
		run_init(INIT, argc, argv );
	}
}
