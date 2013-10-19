/**
 * @file mm/mem.h
 *
 * @author David Matlack
 */
#ifndef __MM_MEM_H__
#define __MM_MEM_H__

#include <stddef.h>
#include <types.h>

struct mem_map {

  size_t kernel_image_start;
  size_t kernel_image_end;

  size_t kernel_mem_start;
  size_t kernel_mem_end;

  size_t user_mem_start;
  size_t user_mem_end;

  size_t page_size;

  size_t max_mem;

};

extern struct mem_map mem;
#define __KIMG_START   (mem.kernel_image_start)
#define __KIMG_END     (mem.kernel_image_end)
#define __KMEM_START   (mem.kernel_mem_start)
#define __KMEM_END     (mem.kernel_mem_end)
#define __UMEM_START   (mem.user_mem_start)
#define __UMEM_END     (mem.user_mem_end)
#define __PAGE_SIZE    (mem.page_size)
#define __MAX_MEM      (mem.max_mem)

#define PAGE_ALIGN_UP(n) \
  CEIL(mem.page_size, n)

#define PAGE_ALIGN_DOWN(n) \
  FLOOR(mem.page_size, n)

int mem_init(size_t max_mem, size_t page_size,
              char kimg_start[], char kimg_end[]);

void mem_layout_dump(printf_f p);

#endif /* !__MM_MEM_H__ */
