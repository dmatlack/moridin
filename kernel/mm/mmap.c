/**
 * @file mm/mmap.c
 */
#include <mm/vm.h>
#include <kernel/proc.h>
#include <kernel/config.h>
#include <errno.h>
#include <arch/vm.h>
#include <kernel/debug.h>
#include <kernel/kmalloc.h>

#if CONFIG_KERNEL_VIRTUAL_START == 0 // f*** you gcc
  #define kernel_address(addr) \
     (addr <  CONFIG_KERNEL_VIRTUAL_TOP)
#else
  #define kernel_address(addr) \
    (addr >= CONFIG_KERNEL_VIRTUAL_START && \
     addr <  CONFIG_KERNEL_VIRTUAL_TOP)
#endif

/**
 * @return The mapping that contains <addr> or NULL if none exists.
 */
static struct vm_mapping *find_mapping(struct vm_space *space, unsigned long addr) {
  struct vm_mapping *m;

  list_foreach(m, &space->mappings, link) {
    /*
     * Found an overlapping region
     */
    if (m->address <= addr && (m->address + m->num_pages * PAGE_SIZE) > addr) {
      return m;
    }

    if (m->address > addr) break;
  }

  return NULL;
}

/**
 * @brief A page fault occurred on a mapping backed on a file. This function 
 * will map the page and fill it with the contents of the file.
 */
static int page_fault_file(struct vm_mapping *m, unsigned long addr) {
  unsigned long virt = PAGE_ALIGN_DOWN(addr);
  unsigned long voff = virt - m->address;
  int error;

  TRACE("mapping=%p, addr=0x%08x", m, addr);

  error = vm_map_page(m->space, virt, m->flags);
  if (error) {
    return ENOMEM;
  }

  /*
   * Read the page in from the file.
   */
  error = vfs_read_page(m->file, m->foff + voff, (char *) virt);
  if (error < 0) {
    vm_unmap_page(m->space, virt);
    return EFAULT;
  }

  /*
   * If a whole page wasn't read from the file (because it wasn't big
   * enough, copy 0's to the page.
   */
  if (error < PAGE_SIZE) {
    memset((void *) (virt + error), 0, PAGE_SIZE - error);
  }

  return 0;
}

/**
 * @breif A page fault occurred on an anonymous mapping.
 */
static int page_fault_anon(struct vm_mapping *m, unsigned long addr) {
  unsigned long virt = PAGE_ALIGN_DOWN(addr);
  int error;

  TRACE("mapping=%p, addr=0x%08x", m, addr);

  error = vm_map_page(m->space, virt, m->flags);
  if (error) {
    return ENOMEM;
  }

  memset((void *) virt, 0, PAGE_SIZE);

  return 0;
}

int vm_page_fault(unsigned long addr, int flags) {
  struct vm_mapping *mapping;
  struct vm_space *space = &CURRENT_PROC->space;
  int ret;

  TRACE("addr=0x%08x, flags=0x%x", addr, flags);

  if (kernel_address(addr)) {
    /*
     * User tried to access the kernel. Kill them!
     */
    if (flags & PF_USER) {
      DEBUG("User page faulted on kernel address 0x%08x.", addr);
      return EINVAL;
    }

    /*
     * Kernel faulted on kernel address... kernel bug probably
     */
    if (kernel_address(addr) && (flags & PF_SUPERVISOR)) {
      panic("Kernel faulted trying to %s to address 0x%08x!",
            flags & PF_READ ? "read" : "write", addr);
    }
  }

  /*
   * Otherwise the kernel page-faulted writing to a user page or the user
   * page-faulted writing to a user page. Both are expected.
   */
  mapping = find_mapping(space, addr);

  /*
   * SEGFAULT
   *
   * ... or maybe stack growth ... (FIXME)
   */
  if (!mapping) {
    return EFAULT;
  }

  if (mapping->file) {
    ret = page_fault_file(mapping, addr);
  }
  else {
    ret = page_fault_anon(mapping, addr);
  }

  return ret;
}

/*
 * addr, length, off assumed to be page aligned!
 */
unsigned long __vm_mmap(unsigned long addr, unsigned long length, int prot, int flags,
                        struct vfs_file *file, unsigned long off) {
  struct vm_space *space = &CURRENT_PROC->space;
  struct vm_mapping *mapping, *prev = NULL;
  int error, vmflags = 0;
  bool found = false;

  if (prot & PROT_EXEC)     vmflags |= VM_X;
  if (prot & PROT_READ)     vmflags |= VM_R;
  if (prot & PROT_WRITE)    vmflags |= VM_W;
  if (!(prot & PROT_NONE))  vmflags |= VM_P;
  if (kernel_address(addr)) vmflags |= VM_S;
  else                      vmflags |= VM_U;

  // Steps:
  //  1. Find an overlapping mapping to extend or create a new mapping.
  list_foreach(mapping, &space->mappings, link) {
    unsigned long start, end;

    start = mapping->address;
    end = start + mapping->num_pages * PAGE_SIZE;

    /*
     * Found an overlapping region
     */
    if ((start <= addr && end > addr) ||
        (start <= addr + length && end > addr + length)) {
      found = true;
      break;
    }

    prev = mapping;

    /*
     * Found the first region AFTER the region we're trying to add.
     */
    if (start > addr) {
      ASSERT_GREATEREQ(start, addr + length);
      break;
    }
  }

  if (found) {
    //TODO: Probably need to handle this case for growing the heap and stack...
    panic("Found an overlapping mapping!! Handling the case is not implemented "
          "yet...");
  }
  else {
    mapping = (struct vm_mapping *) kmalloc(sizeof(struct vm_mapping));
    if (!mapping) {
      return ENOMEM;
    }

    mapping->space = space;
    mapping->address = addr;
    mapping->num_pages = length / PAGE_SIZE;
    mapping->flags = vmflags;
    if (flags & MAP_ANONYMOUS) {
      mapping->file = NULL;
      mapping->foff = 0;
    }
    else {
      mapping->file = file;
      mapping->foff = off;
    }

    if (prev) {
      list_insert_after(&space->mappings, prev, mapping, link);
    }
    else {
      list_insert_head(&space->mappings, mapping, link);
    }
  }

  //TODO: maybe reserve num_pages pages from the physical page allocator
  // so we don't run out of memory during a page fault.

  /*
   * OK, eventually this code here will be triggered by the page fault
   * handler upon access. But for now let's just pretend we page faulted
   * and map the pages.
   */
  {
    unsigned fault_addr;

    for (fault_addr = addr; fault_addr < addr + length; fault_addr += PAGE_SIZE) {
      // simulate a user read page fault
      error = vm_page_fault(fault_addr, PF_USER | PF_READ);
      ASSERT_EQUALS(0, error);
    }
  }

  return addr;
}

unsigned long vm_mmap(unsigned long addr, unsigned long length, int prot, int flags,
                      struct vfs_file *file, unsigned long off) {

  TRACE("addr=0x%08x, length=0x%x, prot=0x%x, flags=0x%x, file=%p, off=0x%x",
        addr, length, prot, flags, file, off);
  
  if (flags & MAP_SHARED) {
    panic("MAP_SHARED not implemented");
  }

  if (flags & MAP_LOCKED) {
    panic("MAP_LOCKED not implemented");
  }

  if ((flags & MAP_PRIVATE) && (flags & MAP_SHARED)) {
    return EINVAL;
  }

  if (!IS_PAGE_ALIGNED(addr)) {
    DEBUG("addr not page aligned: 0x%x", addr);
    return EINVAL;
  }

  if (!IS_PAGE_ALIGNED(off)) {
    DEBUG("off not page aligned: 0x%x", off);
    return EINVAL;
  }

  length = PAGE_ALIGN_UP(length);

  return __vm_mmap(addr, length, prot, flags, file, off);
}
