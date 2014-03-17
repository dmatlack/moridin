/**
 * @file mm/kmap.c
 */
#include <mm/pages.h>
#include <arch/vm.h>
#include <kernel/debug.h>
#include <assert.h>

char *kmap_start;
char *kmap_end;

extern struct vm_space kernel_space;

char  *kmap_bitmap;
size_t kmap_bitmap_size;

#define KMAP_VM_FLAGS (VM_S | VM_P | VM_R | VM_W)

void kmap_init(void) {
  TRACE();

  kmap_bitmap_size = ((size_t) kmap_end - (size_t) kmap_start) / PAGE_SIZE / 8;
  kprintf("0x%08x\n", kmap_bitmap_size);
  kmap_bitmap = kmalloc(kmap_bitmap_size);
  if (!kmap_bitmap) {
    panic("Failed to allocate the kmap_bitmap.");
  }

  memset(kmap_bitmap, 0, kmap_bitmap_size);
}

static inline void *kmap_address(char *block, int bit) {
  unsigned long pgnum;

  pgnum = (unsigned long) (block - kmap_bitmap) * 8;

  return (void *) ((unsigned long) kmap_start + (pgnum + bit)*PAGE_SIZE);
}

void *kmap_alloc_page(void) {
  char *block;

  for (block = kmap_bitmap; block < kmap_bitmap + kmap_bitmap_size; block++) {
    if (block) {
      int bit;

      for (bit = 0; ((*block >> bit) & 1) != 0; bit++) {
        ASSERT_NOTEQUALS(bit, 8);
      }

      *block |= (1 << bit);
      
      return kmap_address(block, bit);
    }
  }

  return NULL;
}

int kmap_map_page(void *virt, struct page *page) {
  int error;

  error = map_page(kernel_space.object, (unsigned long) virt, page, KMAP_VM_FLAGS);
  if (error) {
    return error;
  }

  tlb_invalidate((unsigned long) virt, PAGE_SIZE);
  return 0;
}

static inline char *kmap_block(void *virt) {
  unsigned long v = (unsigned long) virt;
  
  v -= (unsigned long) kmap_start;
  v /= PAGE_SIZE;
  v /= 8;

  return kmap_bitmap + v;
}

static inline int kmap_bit(void *virt) {
  unsigned long v = (unsigned long) virt;

  v -= (unsigned long) kmap_start;
  v /= PAGE_SIZE;

  return v % 8;
}

void kmap_free_page(void *virt) {
  char *block = kmap_block(virt);
  int bit = kmap_bit(virt);

  ASSERT(*block & (1 << bit));
  *block &= ~(1 << bit);
}

/**
 * @brief Create a mapping in kernel memory for the provided physical page.
 *
 * @return
 *    The virt address of the page on success
 *    0 on error
 */
void *kmap(struct page *page) {
  void *virt;
  int error;

  TRACE("page=%p (0x%08x)", page, page_address(page));

  virt = kmap_alloc_page();
  if (!virt) {
    return NULL;
  }

  error = kmap_map_page(virt, page);
  if (error) {
    kmap_free_page(virt);
    return NULL;
  }

  return virt;
}

/**
 * @brief Unmap a kernel mapping.
 */
void kunmap(void *virt) {
  TRACE("virt=%p", virt);

  unmap_page(kernel_space.object, (unsigned long) virt);
  tlb_invalidate((unsigned long) virt, PAGE_SIZE);
  kmap_free_page(virt);
}
