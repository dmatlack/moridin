/**
 * @file kernel/kprintf.h
 *
 */
#ifndef __KERNEL_KPRINTF_H__
#define __KERNEL_KPRINTF_H__

/**
 * @brief Set the putchar method used by kprintf
 */
void kputchar_set(void (*pc)(char c));

/**
 * @brief Print a character to the screen.
 */
int kputchar(int c);

/**
 * @brief Formatted printing to the console.
 */
int kprintf(const char *fmt, ...);

#endif /* !__KERNEL_KPRINTF_H__ */
