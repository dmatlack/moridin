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

size_t mb_mod_start(struct multiboot_info *mb_info, int index)
{
	return (size_t)
		(((multiboot_module_t *) mb_info->mods_addr) + index)->mod_start;
}

/**
 * @brief Dump the contents of the multiboot_info struct using the given
 * printf function.
 *
 * @param p The printf function to use to do printing.
 * @param mb_info The address of the multiboot info struct.
 */
void mb_dump(printf_f p, struct multiboot_info *mb_info)
{
	uint32_t flags = mb_info->flags;

	p("struct multiboot_info *: %p\n\n", mb_info);
	p("spec: http://www.gnu.org/software/grub/manual/multiboot/multiboot.txt\n");

#define IF_FLAGS(_flag) \
	p("%s: %d\n", #_flag, (flags & (_flag)) ? 1 : 0); \
	if (flags & (_flag)) 

	IF_FLAGS( MULTIBOOT_INFO_MEMORY ) {
		p("  mem_lower = 0x%08x\n", mb_info->mem_lower * 1024);
		p("  mem_upper = 0x%08x\n", mb_info->mem_upper * 1024);
	}

	IF_FLAGS( MULTIBOOT_INFO_BOOTDEV ) {
		p("  boot_device = 0x%08x\n", mb_info->boot_device);
	}

	IF_FLAGS( MULTIBOOT_INFO_CMDLINE ) {
		p("  cmd_line = %s\n", (char *) mb_info->cmdline);
	}

	IF_FLAGS( MULTIBOOT_INFO_MODS ) {
		unsigned i;

		p("  mods_count = %d\n", mb_info->mods_count);
		p("  mods_addr = 0x%08x\n", mb_info->mods_addr);

		for (i = 0; i < mb_info->mods_count; i++) {
			multiboot_module_t *mod;

			mod = ((multiboot_module_t *) mb_info->mods_addr) + i;
			p("    %d: start=0x%08x, end=0x%08x, size=0x%x, cmdline=%s\n",
					i, mod->mod_start, mod->mod_end, mod->mod_end - mod->mod_start,
					(char *) mod->cmdline);
		}
	}

	IF_FLAGS( MULTIBOOT_INFO_AOUT_SYMS ) {
		p("  aout_sym:\n");
		p("    tabsize = 0x%08x\n", mb_info->u.aout_sym.tabsize); 
		p("    strsize = 0x%08x\n", mb_info->u.aout_sym.strsize); 
		p("    addr = 0x%08x\n", mb_info->u.aout_sym.addr); 
		p("    reserved = 0x%08x\n", mb_info->u.aout_sym.reserved); 
	}

	IF_FLAGS( MULTIBOOT_INFO_ELF_SHDR ) {
		p("  elf_sec:\n");
		p("    num = 0x%08x\n", mb_info->u.elf_sec.num); 
		p("    size = 0x%08x\n", mb_info->u.elf_sec.size); 
		p("    addr = 0x%08x\n", mb_info->u.elf_sec.addr); 
		p("    shndx = 0x%08x\n", mb_info->u.elf_sec.shndx); 
	}

	IF_FLAGS( MULTIBOOT_INFO_MEM_MAP ) {
		struct multiboot_mmap_entry *e;

		p("  mmap_length = 0x%08x\n", mb_info->mmap_length);
		p("  mmap_addr = 0x%08x\n", mb_info->mmap_addr);

		e = (struct multiboot_mmap_entry *) mb_info->mmap_addr;
		while ((unsigned int) e < (mb_info->mmap_addr + mb_info->mmap_length)) {
			p("    start = 0x%08llx, end = 0x%08llx, len = 0x%08llx, type = %d (%s)\n",
					e->addr, e->addr + e->len, e->len, e->type,
					e->type == MULTIBOOT_MEMORY_AVAILABLE ? "available" : "reserved");
			e++;
		}
	}

	IF_FLAGS( MULTIBOOT_INFO_DRIVE_INFO ) {
		p("  (not implemented)\n");
	}

	IF_FLAGS( MULTIBOOT_INFO_CONFIG_TABLE ) {
		p("  (not implemented)\n");
	}

	IF_FLAGS( MULTIBOOT_INFO_BOOT_LOADER_NAME ) {
		p("  boot_loader_name = %s\n", mb_info->boot_loader_name);
	}

	IF_FLAGS( MULTIBOOT_INFO_APM_TABLE ) {
		p("  (not implemented)\n");
	}

	IF_FLAGS( MULTIBOOT_INFO_VIDEO_INFO ) {
		p("  (not implemented)\n");
	}

#undef IF_FLAGS
}

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
