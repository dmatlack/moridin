/**
 * @file mem.c
 *
 * @brief The most basic memory manipulation functions and data.
 *
 * @author David Matlack
 */
#include <mm/mem.h>

/*
 * the kernel image is the kernels code (text region) as well as
 * all it's data regions, as it exists in memory. we need to know
 * this information so we don't accidentally overwrite it.
 */
size_t kernel_image_start;
size_t kernel_image_end;

/*
 * kernel memory is managed by kmalloc, the kernel's dynamic memory
 * allocator. it exists in ring 0 and is only used when running in
 * kernel mode.
 */
size_t kernel_mem_start;
size_t kernel_mem_end;

/*
 * user memory exists in ring 3 and is used to provide pages of memory
 * to user processes
 */
size_t user_mem_start;
size_t user_mem_end;

void mem_layout_dump(printf_f p) {
  p("=== Kernel Memory Layout ===\n");
  p("kernel_image_start:    0x%08x\n", kernel_image_start);
  p("kernel_image_end:      0x%08x\n", kernel_image_end);
  p("\n");
  p("kernel_mem_start:      0x%08x\n", kernel_mem_start);
  p("kernel_mem_end:        0x%08x\n", kernel_mem_end);
  p("\n");
  p("user_mem_start:        0x%08x\n", user_mem_start);
  p("user_mem_end:          0x%08x\n", user_mem_end);
}

