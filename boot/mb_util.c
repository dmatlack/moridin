/**
 * @file mb_util.c
 *
 * @brief Utility functions for the multiboot start up code.
 *
 * @author David Matlack
 */
#include <boot/multiboot.h>
#include <debug.h>
#include <stddef.h>
#include <x86/page.h>
#include <stdint.h>
#include <assert.h>
#include <kernel/kprintf.h>

#include <mm/lmm.h>
#include <mm/lmm_types.h>

/*
 * the instance of lmm used in the kernel to dynamically allocate memory
 */
extern lmm_t kernel_lmm;

/*
 * the start and end address of the kernel image
 */
extern char *kernel_image_start;
extern char *kernel_image_end;

/*
 * the start address of physical memory dedicated to users
 */
extern char *user_mem_start;
extern char *user_mem_end;

/*
 * an "lmm_region_t" that covers the entire address space
 */
struct lmm_region global_region;

/**
 * @brief Initialize the kernel's base dynamic memory mangager, lmm.
 *
 * lmm needs to know in what regions of memory it can allocate memory.
 * We use a very simple scheme for determining what regions lmm can use.
 * The memory layout looks as follows:
 *
 *   +-----------------------------+ 0
 *   | 1 Megabyte (reserved)       |
 *   +-----------------------------+ 0x100000, kernel_image_start
 *   | kernel text/data/bss/etc    |
 *   +-----------------------------+ kernel_image_end
 *   |                             |
 *   | Available physical memory   |
 *   | for lmm/kernel use          |
 *   |                             |
 *   +-----------------------------+ user_mem_start
 *   |                             |
 *   | Physical memory reserved    |
 *   | for user processes          |
 *   |                             |
 *  ...                           ...
 *   |                             |
 *   |                             |
 *   +-----------------------------+ 0x100000 + (mb_info->mem_upper * 1024)
 *   (end of available memory)
 *
 * As we can see, there is a chunk of memory above the kernel image and below
 * user memory that we can give to lmm for use in dynamic memory allocation.
 *
 */ 
int mb_lmm_init(struct multiboot_info *mb_info) {
  size_t mem_upper_size = mb_info->mem_upper * 1024;
  size_t mem_upper_start = MEGABYTE;
  size_t mem_upper_end = mem_upper_start + mem_upper_size;
  size_t kimg_start = (size_t) kernel_image_start;
  size_t kimg_end = (size_t) kernel_image_end;
  size_t umem_start = (size_t) user_mem_start;

  /*
   * We will create a region of memory, (min, max), that lmm can use to
   * do all dynamic memory allocation.
   */
  size_t min = (size_t) 0;
  size_t max = (size_t) -1;

  /*
   * (min,max) must be within upper memory
   */
  if (min < mem_upper_start) {
    min = mem_upper_start;
  }
  if (max > mem_upper_end) {
    max = mem_upper_end;
  }

  /* 
   * (min,max) must not overlap the kernel image
   */
  if (min < kimg_end && max > kimg_end) {
    min = kimg_end;
  }
  if (max > kimg_start && min < kimg_start) {
    max = kimg_start;
  }

  /*
   * (min,max) must not overlap user memory
   */
  if (max > umem_start) {
    max = umem_start;
  }

  max = ALIGN_DOWN(max, PAGE_SIZE);
  min = ALIGN_UP(min, PAGE_SIZE);
  assert(max > min);

  lmm_init(&kernel_lmm);
  lmm_add_region(&kernel_lmm, &global_region, (size_t) 0, (size_t) -1, 0, 0);
  lmm_add_free(&kernel_lmm, (void *) min, (size_t) max - min);

  return 0;
}

/**
 * @brief Dump the contents of the multiboot_info struct using the given
 * printf function.
 *
 * @param p The printf function to use to do printing.
 * @param mb_info The address of the multiboot info struct.
 */
void mb_dump(printf_f p, struct multiboot_info *mb_info) {
  uint32_t flags = mb_info->flags;
  
  p("struct multiboot_info *: %p\n\n", mb_info);
  p("spec: http://www.gnu.org/software/grub/manual/multiboot/multiboot.txt\n");

#define F(n) ((flags >> (n)) & 0x1)
#define IF_FLAGS(n) \
  p("\nflags[%d] = %d\n", n, F(n)); \
  if (F(n)) 

  IF_FLAGS(0) {
    p("  mem_lower = 0x%08x\n", mb_info->mem_lower * 1024);
    p("  mem_upper = 0x%08x\n", mb_info->mem_upper * 1024);
  }

  IF_FLAGS(1) {
    p("  boot_device = 0x%08x\n", mb_info->boot_device);
  }

  IF_FLAGS(2) {
    p("  cmd_line = %s\n", (char *) mb_info->cmdline);
  }

  IF_FLAGS(3) {
    p("  mods_count = %d\n", mb_info->mods_count);
    p("  mods_addr = 0x%08x\n", mb_info->mods_addr);
  }

  IF_FLAGS(4) {
    p("  aout_sym:\n");
    p("    tabsize = 0x%08x\n", mb_info->u.aout_sym.tabsize); 
    p("    strsize = 0x%08x\n", mb_info->u.aout_sym.strsize); 
    p("    addr = 0x%08x\n", mb_info->u.aout_sym.addr); 
    p("    reserved = 0x%08x\n", mb_info->u.aout_sym.reserved); 
  }

  IF_FLAGS(5) {
    p("  elf_sec:\n");
    p("    num = 0x%08x\n", mb_info->u.elf_sec.num); 
    p("    size = 0x%08x\n", mb_info->u.elf_sec.size); 
    p("    addr = 0x%08x\n", mb_info->u.elf_sec.addr); 
    p("    shndx = 0x%08x\n", mb_info->u.elf_sec.shndx); 
  }

  IF_FLAGS(6) {
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

  //////
  //TODO eh are these really important?
  //////
  IF_FLAGS(7) {
    p("  TODO\n");
  }

  IF_FLAGS(8) {
    p("  TODO\n");
  }

  IF_FLAGS(9) {
    p("  boot_loader_name = %s\n", mb_info->boot_loader_name);
  }

  IF_FLAGS(10) {
    p("  TODO\n");
  }
  
  IF_FLAGS(11) {
    p("  TODO\n");
  }

#undef FLAGS
}
