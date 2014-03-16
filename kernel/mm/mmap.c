/**
 * @file mm/mmap.c
 */
#include <mm/vm.h>

#include <kernel/config.h>
#include <kernel/debug.h>
#include <mm/kmalloc.h>
#include <kernel/log.h>
#include <kernel/proc.h>

#include <arch/vm.h>

#include <errno.h>
#include <math.h>

/**
 * @return The mapping that contains <addr> or NULL if none exists.
 */
static struct vm_mapping *find_mapping(struct vm_space *space, unsigned long addr) {
  struct vm_mapping *m;

  list_foreach(m, &space->mappings, link) {
    /*
     * Found an overlapping region
     */
    if (m->address <= addr && M_END(m) > addr) {
      return m;
    }

    if (m->address > addr) break;
  }

  return NULL;
}

/**
 * @brief Return the first mapping in space that overlaps with [addr, addr + length).
 */
static struct vm_mapping *find_first_overlapping(struct vm_space *space, unsigned long addr,
                                                 unsigned long length) {
  struct vm_mapping *m;

  list_foreach(m, &space->mappings, link) {
    if (check_overlap(addr, length, m->address, M_LENGTH(m))) {
      return m;
    }
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
      panic("Kernel faulted trying to %s kernel address 0x%08x!",
            flags & PF_READ ? "read" : "write to", addr);
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
  struct vm_mapping *m, *prev = NULL;
  int vmflags = 0;
  bool found = false;

  if (prot & PROT_EXEC)     vmflags |= VM_X;
  if (prot & PROT_READ)     vmflags |= VM_R;
  if (prot & PROT_WRITE)    vmflags |= VM_W;
  if (!(prot & PROT_NONE))  vmflags |= VM_P;
  if (kernel_address(addr)) vmflags |= VM_S;
  else                      vmflags |= VM_U;

  // Steps:
  //  1. Find an overlapping mapping to extend or create a new mapping.
  list_foreach(m, &space->mappings, link) {
    /*
     * Found an overlapping region
     */
    if (check_overlap(addr, length, m->address, M_LENGTH(m))) {
      DEBUG("OVERLAP?? (0x%08x, 0x%08x) (0x%08x, 0x%08x)",
            addr, length, m->address, M_LENGTH(m));
      found = true;
      break;
    }

    /*
     * Found the first region AFTER the region we're trying to add.
     */
    if (m->address > addr) {
      ASSERT_GREATEREQ(m->address, addr + length);
      break;
    }

    prev = m;
  }

  if (found) {
    //TODO: Probably need to handle this case for growing the heap and stack...
    panic("Found an overlapping mapping!! Handling the case is not implemented "
          "yet...");
  }
  else {
    m = (struct vm_mapping *) kmalloc(sizeof(struct vm_mapping));
    if (!m) {
      return ENOMEM;
    }

    m->space = space;
    m->address = addr;
    m->num_pages = length / PAGE_SIZE;
    m->flags = vmflags;
    if (flags & MAP_ANONYMOUS) {
      m->file = NULL;
      m->foff = 0;
    }
    else {
      m->file = file;
      m->foff = off;
    }

    if (prev) {
      list_insert_after(&space->mappings, prev, m, link);
    }
    else {
      list_insert_head(&space->mappings, m, link);
    }
  }

  return addr;
}

/**
 * @return The page aligned address of the region that was mapped on success.
 * On error return an error code < PAGE_SIZE.
 */
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

  // If addr is NULL then we should pick the address to map for the process.
  if (!addr) {
    panic("NULL addr not implemented");
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

/**
 * @return 0 on success, non-0 error code otherwise
 *
 * It is not an error to unmap a region of memory that contains no mappings.
 */
int vm_munmap(unsigned long addr, unsigned long length) {
  struct vm_mapping *next;
  struct vm_space *space = &CURRENT_PROC->space;
  struct vm_mapping *m;
  unsigned long virt;

  TRACE("addr=0x%08x, length=0x%x", addr, length);

  // hmm... nobody should be unmapping the kernel...
  if (kernel_address(addr)) {
    panic("Attempt to unmap kernel virtual address 0x%08x", addr);
  }

  if (!IS_PAGE_ALIGNED(addr)) {
    DEBUG("addr not page aligned: 0x%x", addr);
    return EINVAL;
  }

  if (addr + length < addr) {
    DEBUG("Overflow: 0x%08x + 0x%08x = 0x%08x", addr, length, addr + length);
    return EINVAL;
  }

  length = PAGE_ALIGN_UP(length);

  m = find_first_overlapping(space, addr, length);
  if (!m) {
    return 0;
  }
  
  /*
   * If we are trying to unmap in the middle of a mapping we will split it
   * in two.
   */
  if (addr > m->address && (addr + length) < M_END(m)) {
    next = kmalloc(sizeof(struct vm_mapping));
    if (!next) {
      return ENOMEM;
    }

    next->space = space;
    next->address = addr + length;
    next->num_pages = (M_END(m) - next->address) / PAGE_SIZE;
    next->flags = m->flags;
    next->file = m->file; //TODO: increase file->refs?
    /*
     * The file offset has to change because the start address of the mapping
     * changed.
     */
    if (m->file) next->foff = m->foff + (next->address - m->address);

    list_insert_after(&space->mappings, m, next, link);

    m->num_pages = (addr - m->address) / PAGE_SIZE;

    /*
     * Unmap the pages in the hardware virtual memory management.
     */
    for (virt = addr; virt < addr + length; virt += PAGE_SIZE) {
      vm_unmap_page(space, virt);
    }
  }
  /*
   * In the normal case we are unmapping some set of mappings.
   */
  else {
    unsigned long unmap_start, unmap_end;

    do {
      /*
       * Only unmap the end of this mapping.
       */
      if (m->address < addr) {
        unmap_start = addr;
        unmap_end = M_END(m);

        m->num_pages -= (unmap_end - unmap_start) / PAGE_SIZE;
        m = list_next(m, link);
      }
      /*
       * Only unmap the beginning of this mapping.
       */
      else if (M_END(m) > addr + length) {
        unmap_start = m->address;
        unmap_end = addr + length;

        if (m->file) m->foff += (unmap_end - unmap_start);
        m->num_pages -= (unmap_end - unmap_start) / PAGE_SIZE;
        m->address = unmap_end;
        m = list_next(m, link);
      }
      /*
       * Unmap the entire mapping.
       */
      else {
        unmap_start = m->address;
        unmap_end = M_END(m);

        next = list_next(m, link);
        list_remove(&space->mappings, m, link);
        kfree(m, sizeof(struct vm_mapping)); //TODO: decrease file->refs?
        m = next;
      }

      /*
       * Unmap the pages in the hardware virtual memory management.
       */
      for (virt = unmap_start; virt < unmap_end; virt += PAGE_SIZE) {
        vm_unmap_page(space, virt);
      }
    } while (m && check_overlap(addr, length, m->address, M_LENGTH(m)));
  }

    return 0;
}

#if 0
void test_munmap(void) {
  int prot = PROT_READ | PROT_WRITE;
  int flags = MAP_PRIVATE | MAP_FIXED | MAP_ANONYMOUS;
  int error;

  TRACE();

#define _MAP(_addr, _length) \
  do { \
    error = vm_mmap(_addr, _length, prot, flags, NULL, 0); \
    ASSERT(!(error % PAGE_SIZE)); \
  } while (0)

#define _UNMAP(_start, _end) \
  do { \
    error = vm_munmap(_start, (_end) - (_start)); \
    ASSERT(!error); \
    vm_dump_maps(__log, &CURRENT_PROC->space); \
  } while (0)

  _MAP(0x80000000, 3 * PAGE_SIZE);
  _MAP(0x80010000, 10 * PAGE_SIZE);
  _MAP(0x90000000, 3 * PAGE_SIZE);

  _UNMAP(0x80000000 + PAGE_SIZE, 0x90000000 + PAGE_SIZE);

  /*
   * unmap everything
   */
  _UNMAP(0x80000000 - PAGE_SIZE, 0xA0000000 - PAGE_SIZE);

#undef _MAP
#undef _UNMAP
}
#endif
