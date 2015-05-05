/**
 * @file kernel.c
 */
#include <kernel/config.h>
#include <kernel/init.h>
#include <kernel/irq.h>
#include <kernel/loader.h>
#include <kernel/proc.h>
#include <kernel/stack.h>
#include <kernel/sched.h>
#include <kernel/symbols.h>
#include <kernel/test.h>
#include <kernel/compiler.h>

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

extern void arch_startup(void);

void kernel_main()
{
	arch_startup();

	log_init();

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
