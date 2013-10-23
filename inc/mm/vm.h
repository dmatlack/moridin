/**
 * @file mm/vm.h
 *
 * @author David Matlack
 */
#ifndef __MM_VM_H__
#define __MM_VM_H__

#include <stddef.h>
#include <list.h>

struct vm_object {
  void (*map)(size_t address, size_t size, int flags);
  void (*unmap)(size_t address, size_t size, int flags);
};

/**
 * @brief The vm_region struct represents a contiguous, page-aligned, region
 * of virtual memory that shares the same set of flags (read/write, permissions,
 * etc.).
 */
struct vm_region {
  int flags;
  size_t address;
  size_t size;
  struct vm_object *object;

  list_link(struct vm_region) region_link;
};

#define VM_FLAGS_RW_SHIFT 0
#define VM_FLAGS_US_SHIFT 1
#define VM_FLAGS_K_SHIFT  2

#define VM_FLAGS_READONLY   (0 << VM_FLAGS_RW_SHIFT)
#define VM_FLAGS_READWRITE  (1 << VM_FLAGS_RW_SHIFT)
#define VM_FLAGS_USER       (0 << VM_FLAGS_US_SHIFT)
#define VM_FLAGS_SUPERVISOR (1 << VM_FLAGS_US_SHIFT)
#define VM_FLAGS_KERNEL_MEM (1 << VM_FLAGS_K_SHIFT)

list_typedef(struct vm_region) vm_region_list_t;

/**
 * @brief There is one vm_space per user process, and it represents the 
 * virtual address space of that process.
 */
struct vm_space {
  vm_region_list_t regions;
};

int vm_init(void);
int vm_space_init(struct vm_space *vm);
struct vm_region *vm_add_region(struct vm_space *vm, size_t address,
                                size_t size, int flags);




#endif /* !__MM_VM_H__ */
