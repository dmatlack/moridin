/**
 * @file mm/vm.h
 *
 * @author David Matlack
 */
#ifndef __MM_VM_H__
#define __MM_VM_H__

#include <stddef.h>
#include <list.h>

struct vm_zone {
  size_t address;
  size_t size;
};

extern struct vm_zone __vm_zone_kernel;
extern struct vm_zone __vm_zone_user;

#define VM_ZONE_KERNEL (&(__vm_zone_kernel))
#define VM_ZONE_USER   (&(__vm_zone_user))

typedef int vm_flags_t;

#define VM_FLAGS_RW_SHIFT   0
#define VM_FLAGS_READONLY   (0 << VM_FLAGS_RW_SHIFT)
#define VM_FLAGS_READWRITE  (1 << VM_FLAGS_RW_SHIFT)

#define VM_FLAGS_US_SHIFT   1
#define VM_FLAGS_USER       (0 << VM_FLAGS_US_SHIFT)
#define VM_FLAGS_SUPERVISOR (1 << VM_FLAGS_US_SHIFT)

#define VM_IS_READWRITE(flags)  (flags & VM_FLAGS_READWRITE)
#define VM_IS_READONLY(flags)   (!VM_FLAGS_READWRITE(flags))
#define VM_IS_SUPERVISOR(flags) (flags & VM_FLAGS_SUPERVISOR)
#define VM_IS_USER(flags)       (!VM_FLAGS_SUPERVISOR(flags))


/*
 * vm_machine_object is an opaque object that the machine dependent
 * vm code can define to keep track of an address space.
 */
struct vm_machine_object;

/*
 * vm_machine_interface is an interface that must be implemented by
 * the machine dependent vm code.
 */
struct vm_machine_interface {

  /**
   * @brief This function is expected to do any initialization necessary to
   * bootstrap the machine dependent virtual memory system. After this function
   * return, virtual memory is ON.
   */
  int (*bootstrap)(size_t page_size);

  /**
   * @brief This function is called *after* bootstrap and *after* kernel
   * dynamic memory facilities have been initialized. It must do any necessary
   * initialization that required dynamic memory.
   */
  int (*init)(void);

  /**
   * @brief Allocate and initialize a new vm_machine_object.
   */
  int (*init_object)(struct vm_machine_object **object);

  /**
   * @brief Create mappings between the virtual and physical pages in the 
   * provided address space.
   */
  int (*map)(struct vm_machine_object *object,
             size_t *vpages,
             size_t *ppages,
             int num_pages,
             vm_flags_t flags);

  /**
   * @brief Unmap all of the provided virtual pages in the address space. In
   * other words, after unmap is called, accessing any of the specified virtual
   * addresses should result in a page fault.
   */
  int (*unmap)(struct vm_machine_object *object,
               size_t *vpages,
               int num_pages,
               vm_flags_t flags);

};

struct vm_region {
  size_t address;
  size_t size;
  vm_flags_t flags;
  list_link(struct vm_region) link;
  const char *name;
};

#define INITIALIZE_VM_REGION(region_ptr, address, size, flags, name) \
  do { \
    ASSERT(NULL != (region_ptr)); \
    ASSERT(FLOOR(PAGE_SIZE, (size)) == size); \
    (region_ptr)->address = (address); \
    (region_ptr)->size    = (size); \
    (region_ptr)->flags   = (flags); \
    (region_ptr)->name    = name; \
    list_elem_init(region_ptr, link); \
  } while (0)

list_typedef(struct vm_region) vm_region_list_t;

struct vm_address_space {
  vm_region_list_t            region_list;
  struct vm_machine_object   *object;
};

int vm_bootstrap(void);

int vm_address_space_init(struct vm_address_space *vm);
int vm_address_space_create(struct vm_address_space **vmp);
int vm_map_region(struct vm_address_space *vm, struct vm_region *new_region);

#endif /* !__MM_VM_H__ */
