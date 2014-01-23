/**
 * @file mm/vm.h
 *
 * @brief Virtual Memory Manager
 *
 * @author David Matlack
 */
#ifndef __MM_VM_H__
#define __MM_VM_H__

#include <stddef.h>
#include <list.h>

typedef int vm_flags_t;

#define VM_R (1 << 0) // read
#define VM_W (1 << 1) // write
#define VM_X (1 << 2) // execute
#define VM_U (1 << 3) // user
#define VM_S (1 << 4) // supervisor

struct vm_region {
  size_t address;
  size_t size;
  vm_flags_t flags;
  list_link(struct vm_region) link;
};

list_typedef(struct vm_region) vm_region_list_t;

struct vm_space {
  void *object;
};

extern struct vm_space boot_vm_space;

int  vm_init(void);
int  vm_space_init(struct vm_space *space);
void vm_space_switch(struct vm_space *space);
int  vm_map(struct vm_space *space, size_t address, size_t size, vm_flags_t flags);
int  __vm_map(struct vm_space *space, size_t address, size_t size, size_t *ppages, vm_flags_t flags);
void vm_unmap(struct vm_space *space, size_t address, size_t size);

#endif /* !__MM_VM_H__ */
