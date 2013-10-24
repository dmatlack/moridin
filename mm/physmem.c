/**
 * @file physmem.c
 *
 * @author David Matlack
 */
#include <mm/physmem.h>

#include <kernel/kprintf.h> //remove me
#include <debug.h>

#include <stddef.h>
#include <assert.h>

struct pmem_map __pmem;

/**
 * @brief Initialize the memory constructs.
 *
 * @param mem_max The maximum available memory on the system.
 * @param page_size The size of memory pages on the system.
 * @param kimg_start The start address of the kernel image in memory.
 * @param kimg_end The end address of the kernel image in memory.
 */
int mem_init(size_t max_mem, size_t page_size,
             char kimg_start[], char kimg_end[]) {
  size_t kmem_size;
  size_t umem_size;
  size_t mem_avail;

  __pmem.max_mem = max_mem;
  __pmem.page_size = page_size;

  __pmem.kernel_image_start = PAGE_ALIGN_DOWN((size_t) kimg_start);
  __pmem.kernel_image_end = PAGE_ALIGN_UP((size_t) kimg_end);
  
  ZONE_BIOS->address = 0;
  ZONE_BIOS->size = MB(1);
  ZONE_BIOS->dbgstr = "ZONE_BIOS";
  dprintf("ZONE_BIOS: start = 0x%08x, size = 0x%08x\n", ZONE_BIOS->address, ZONE_BIOS->size);

  ZONE_DMA->address = MB(1);
  ZONE_DMA->size = MB(15);
  ZONE_DMA->dbgstr = "ZONE_DMA";

  /*
   * We need to divide up the remaining memory between the user and the 
   * kernel without giving too much memory to the user such that the kernel
   * can't do very much and not giving too much to the kernel such that only
   * few processes can run.
   */
  mem_avail = max_mem - MB(16);
  assert(mem_avail >= CONFIG_MIN_USER_MEM + CONFIG_MIN_KERNEL_MEM);

  //FIXME hack
  umem_size = PAGE_ALIGN_DOWN(mem_avail / 4 * 3);
  kmem_size = PAGE_ALIGN_DOWN(mem_avail - umem_size);

  if (kmem_size > CONFIG_MAX_KERNEL_MEM) {
    kmem_size = CONFIG_MAX_KERNEL_MEM;
    umem_size = mem_avail - kmem_size;
  }

  assert(kmem_size > CONFIG_MIN_KERNEL_MEM);
  assert(umem_size > CONFIG_MIN_USER_MEM);

  ZONE_USER->address = MB(16);
  ZONE_USER->size = umem_size;
  ZONE_USER->dbgstr = "ZONE_USER";

  ZONE_KERNEL->address = ZONE_USER->address + ZONE_USER->size;
  ZONE_KERNEL->size = kmem_size;
  ZONE_KERNEL->dbgstr = "ZONE_KERNEL";

  return 0;
}

void mem_layout_dump(printf_f p) {
  int i;

#define PADDR(a) p("0x%08x (%d MB)\n", (a), (a) / MB(1))

  p("=== Physical Memory Layout ===\n");
  p("max_mem:               "); PADDR(__pmem.max_mem);
  p("page_size:             0x%08x\n", __pmem.page_size);
  p("\n");
  p("kernel_image_start:    0x%08x\n", __pmem.kernel_image_start);
  p("kernel_image_end:      0x%08x\n", __pmem.kernel_image_end);
  p("\n");
  for (i = 0; i < PMEM_NUM_ZONES; i++) {
    struct pmem_zone *z = __pmem.zones + i;
    p("  [%d] %s\n", i, z->dbgstr);
    p("     start = "); PADDR(z->address);
    p("     end   = "); PADDR(z->address + z->size);
    p("     size  = "); PADDR(z->size);
    p("\n");
  }
}

