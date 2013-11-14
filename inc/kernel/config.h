/**
 * @file kernel/config.h
 *
 * @brief All static kernel configuration parameters.
 */
#ifndef __KERNEL_CONFIG_H__
#define __KERNEL_CONFIG_H__

#include <stddef.h>

#define CONFIG_MAX_KERNEL_MEM GB(1)
#define CONFIG_MIN_KERNEL_MEM MB(128)
#define CONFIG_MIN_USER_MEM   MB(256)

#endif /* !__KERNEL_CONFIG_H__ */
