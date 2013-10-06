/** @file lib/types.h
 *  @brief Defines some C standard types as well as
 *         wrapper types for some of the imported FLUX library code.
 *  @author matthewj S2008
 */

#ifndef LIB_TYPES_H
#define LIB_TYPES_H

typedef unsigned int size_t;
typedef unsigned long ptrdiff_t;

/* WRAPPERS */

typedef unsigned long vm_offset_t;
typedef unsigned long vm_size_t;

typedef char bool;
#define true  1
#define TRUE  1
#define false 0
#define FALSE 0

#define boolean_t bool

typedef int (*printf_f)(const char *fmt, ...);

#endif /* !LIB_TYPES_H */
