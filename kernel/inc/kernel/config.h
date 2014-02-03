/**
 * @file kernel/config.h
 *
 * @brief All static kernel configuration parameters.
 */
#ifndef __KERNEL_CONFIG_H__
#define __KERNEL_CONFIG_H__

/*
 * The start address of the kernel's virtual address space.
 */
#define CONFIG_KERNEL_VIRTUAL_START 0x00000000

#define CONFIG_USER_VIRTUAL_TOP 0xFFFFF000

/*
 * The default frequency (times per second) to receive hardware interrupts
 * from the timer.
 */
#define CONFIG_TIMER_HZ 100

#endif /* !__KERNEL_CONFIG_H__ */
