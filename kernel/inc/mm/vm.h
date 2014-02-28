/**
 * @file mm/vm.h
 *
 * @brief Virtual Memory Manager
 *
 * @author David Matlack
 */
#ifndef __MM_VM_H__
#define __MM_VM_H__

#include <string.h>
#include <stddef.h>
#include <list.h>

struct vm_mapping {
  struct vm_space *space;

  /*
   * The virtual address of the mapping.
   */
  size_t address;

  /*
   * The number of pages in the region.
   */
  unsigned long num_pages;

#define VM_R (1 << 0) // read
#define VM_W (1 << 1) // write
#define VM_X (1 << 2) // execute
#define VM_U (1 << 3) // user
#define VM_S (1 << 4) // supervisor
#define VM_G (1 << 5) // global
#define VM_P (1 << 6) // present (FIXME need to implement in arch/x86/vm.c)
  int flags;

  /*
   * The mapped file, NULL if this is an anonymous mapping.
   */
  struct vfs_file *file;
  unsigned long foff;

  list_link(struct vm_mapping) link;
};

list_typedef(struct vm_mapping) vm_mapping_list_t;

struct vm_space {
  void *object;
  vm_mapping_list_t mappings;
};

extern struct vm_space boot_vm_space;

void vm_init(void);
int  vm_space_init(struct vm_space *space);
void *__vm_space_switch(void *space_object);
int  vm_map(struct vm_space *space, size_t address, size_t size, int flags);
void vm_unmap(struct vm_space *space, size_t address, size_t size);

//TODO move to syscall header when that exists
#define PROT_EXEC   (1 << 0)
#define PROT_READ   (1 << 1)
#define PROT_WRITE  (1 << 2)
#define PROT_NONE   (1 << 3)

//TODO move to syscall header when that exists
#define MAP_SHARED    (1 << 0)
#define MAP_PRIVATE   (1 << 1)
#define MAP_ANONYMOUS (1 << 2)
#define MAP_LOCKED    (1 << 3)
#define MAP_FIXED     (1 << 4)

#include <fs/vfs.h>

unsigned long vm_mmap(unsigned long addr, unsigned long length,
                      int prot, int flags,
                      struct vfs_file *file, unsigned long off);

#define PF_READ       (1 << 0) // page fault was a read
#define PF_WRITE      (1 << 1) // page fault was a write
#define PF_USER       (1 << 2) // faulted while in user mode
#define PF_SUPERVISOR (1 << 3) // faulted while in kernel mode
int vm_page_fault(unsigned long address, int flags);

int vm_map_page(struct vm_space *space, unsigned long virt, int flags);
void vm_unmap_page(struct vm_space *space, unsigned long virt);

#endif /* !__MM_VM_H__ */
