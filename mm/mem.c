/**
 * @file mem.c
 *
 * @brief The most basic memory manipulation functions and data.
 *
 * @author David Matlack
 */
#include <mm/mem.h>

#include <kernel/kprintf.h> //remove me

#include <stddef.h>
#include <assert.h>

struct mem_map mem;

/**
 * @brief Initialize the memory constructs.
 *
 * We use a very simple scheme for memory:
 *
 *   +-----------------------------+ 0
 *   | 1 Megabyte (reserved)       |
 *   +-----------------------------+ 0x100000, kernel_image_start
 *   | kernel text/data/bss/etc    |
 *   +-----------------------------+ kernel_image_end
 *   |                             |
 *   | kernel memory               |
 *   |                             |
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
 *   mem_max
 *
 * @param mem_max The maximum available memory on the system.
 * @param page_size The size of memory pages on the system.
 * @param kimg_start The start address of the kernel image in memory.
 * @param kimg_end The end address of the kernel image in memory.
 */
int mem_init(size_t max_mem, size_t page_size,
              char kimg_start[], char kimg_end[]) {

  mem.max_mem = max_mem;

  mem.kernel_image_start = (size_t) kimg_start;
  mem.kernel_image_end = (size_t) kimg_end;
  
  mem.user_mem_start = 512 * MEGABYTE;
  mem.user_mem_end = max_mem; 

  mem.page_size = page_size;

  //FIXME should we move kernel_mem_start to after 16 MB?
  mem.kernel_mem_start = PAGE_ALIGN_UP(mem.kernel_image_end);
  mem.kernel_mem_end = PAGE_ALIGN_DOWN(mem.user_mem_start);

  assert(mem.user_mem_start < mem.user_mem_end);
  assert(mem.kernel_mem_start < mem.kernel_mem_end);
  assert(mem.kernel_mem_end - mem.kernel_mem_start > 32 * MEGABYTE);

  return 0;
}

void mem_layout_dump(printf_f p) {
  p("=== Kernel Memory Layout ===\n");
  p("kernel_image_start:    0x%08x\n", mem.kernel_image_start);
  p("kernel_image_end:      0x%08x\n", mem.kernel_image_end);
  p("\n");
  p("kernel_mem_start:      0x%08x\n", mem.kernel_mem_start);
  p("kernel_mem_end:        0x%08x\n", mem.kernel_mem_end);
  p("\n");
  p("user_mem_start:        0x%08x\n", mem.user_mem_start);
  p("user_mem_end:          0x%08x\n", mem.user_mem_end);
  p("\n");
  p("page_size:             0x%08x\n", mem.page_size);
  p("max_mem:               0x%08x\n", mem.max_mem);
}

