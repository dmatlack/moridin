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

#endif /* !__MM_VM_H__ */
