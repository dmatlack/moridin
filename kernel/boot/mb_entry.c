/**
 * @file mbentry.c
 *
 * @brief The multiboot entry point of the kernel.
 *
 */
#include <boot/multiboot.h>
#include <stddef.h>
#include <assert.h>
#include <dev/vga.h>
#include <mm/memory.h>

extern size_t initrd_location;
extern void arch_startup(void);
extern void kernel_main(void);

/**
 * @brief The multiboot, C, entry-point to the kernel.
 *
 * This function should set up anything the kernel needs to run that is
 * dependent on this being a multiboot environment (and thus the presence
 * of the multiboot_info struct).
 *
 * @param mb_magic eax, magic value that confirms we are in multiboot
 * @param mb_info ebx, the multiboot_info struct 
 */
void mb_entry(unsigned int mb_magic, struct multiboot_info *mb_info)
{
	vga_init();

	ASSERT_EQUALS(mb_magic, MULTIBOOT_BOOTLOADER_MAGIC);

	/*
	 * Use the mb_info struct to learn about the physical memory layout
	 */
	mem_mb_init(mb_info);

	/*
	 * ASSUMPTION: initrd is the first mod loaded by GRUB.
	 */
	initrd_location = mb_mod_start(mb_info, 0);

	/*
	 * Do any architecture specific initialization routines before entering
	 * kernel_main.
	 */
	arch_startup();

	/*
	 * And finally enter the kernel
	 *
	 * TODO: GRUB/multiboot supports passing in a command line to the kernel
	 * (argv, envp). If we want to support that we should parse the cmdline
	 * field of the multiboot info struct and pass it into the kernel.
	 */
	kernel_main();
}
