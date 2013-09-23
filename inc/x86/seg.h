/** @file x86/seg.h
 *  @brief Predefined segment selectors
 *
 *  @note Must match other course-specific code (e.g., head.S).
 *
 *  @author matthewj S2008
 */

#ifndef X86_SEG_H
#define X86_SEG_H

#define SEGSEL_KERNEL_TSS_IDX   1
#define SEGSEL_KERNEL_CS_IDX    2
#define SEGSEL_KERNEL_DS_IDX    3
#define SEGSEL_USER_CS_IDX      4
#define SEGSEL_USER_DS_IDX      5

#define SEGSEL_TSS         0x08      /**< Task Segment Selector */
#define SEGSEL_KERNEL_CS   0x10      /**< Kernel Code Segment */
#define SEGSEL_KERNEL_DS   0x18      /**< Kernel Data Segment */
#define SEGSEL_USER_CS     0x23      /**< User Code Segment */
#define SEGSEL_USER_DS     0x2b      /**< User Data Segment */

#endif /* !X86_SEG_H */
