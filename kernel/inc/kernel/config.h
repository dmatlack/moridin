/**
 * @file kernel/config.h
 *
 * @brief All static kernel configuration parameters.
 */
#ifndef __KERNEL_CONFIG_H__
#define __KERNEL_CONFIG_H__

/*
 * The kernel's virtual address space.
 */
#define CONFIG_KERNEL_VIRTUAL_START           0x00000000
#define CONFIG_KERNEL_VIRTUAL_END             0x40000000
#define CONFIG_KERNEL_VIRTUAL_SIZE            (CONFIG_KERNEL_VIRTUAL_END - CONFIG_KERNEL_VIRTUAL_START)

#define CONFIG_KMAP_MIN_SIZE                  MB(128)
#define CONFIG_KHEAP_MAX_END                  (CONFIG_KERNEL_VIRTUAL_END - CONFIG_KMAP_MIN_SIZE)

/*
 * The user's virtual address space.
 */
#define CONFIG_USER_VIRTUAL_START             0x40000000
#define CONFIG_USER_VIRTUAL_END               0xFFFFF000
#define CONFIG_USER_VIRTUAL_SIZE              (CONFIG_USER_VIRTUAL_END - CONFIG_USER_VIRTUAL_START)

/*
 * The default frequency (times per second) to receive hardware interrupts
 * from the timer.
 */
#define CONFIG_TIMER_HZ 100

#define LOG_ERROR 0
#define LOG_WARN  1
#define LOG_INFO  2
#define LOG_DEBUG 3
#define CONFIG_LOG_LEVEL LOG_INFO

#endif /* !__KERNEL_CONFIG_H__ */
