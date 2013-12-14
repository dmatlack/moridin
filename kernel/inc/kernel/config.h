/**
 * @file kernel/config.h
 *
 * @brief All static kernel configuration parameters.
 */
#ifndef __KERNEL_CONFIG_H__
#define __KERNEL_CONFIG_H__

#include <stddef.h>

/*
 * The size of the kernel in the VIRTUAL address space.
 */
#define CONFIG_KERNEL_VM_SIZE           GB(1)

/*
 * The minimum amount of PHYSICAL memory needed for to boot.
 */
#define CONFIG_MIN_PHYSMEM_KERNEL       MB(32)
#define CONFIG_MIN_PHYSMEM_USER         MB(32)

/*
 * The default frequency (times per second) to receive hardware interrupts
 * from the timer.
 */
#define CONFIG_TIMER_HZ 100

#endif /* !__KERNEL_CONFIG_H__ */
