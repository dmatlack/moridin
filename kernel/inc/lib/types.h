/**
 * @file lib/types.h
 */

#ifndef LIB_TYPES_H
#define LIB_TYPES_H

// POSIX sys/types.h
// http://pubs.opengroup.org/onlinepubs/9699919799/basedefs/sys_types.h.html

typedef unsigned int size_t;
typedef int          ssize_t;
typedef int          pid_t;


typedef unsigned long vm_offset_t; // needed by lmm
typedef unsigned long vm_size_t;   // needed by lmm

typedef char bool;
#define true  1
#define TRUE  1
#define false 0
#define FALSE 0

#define boolean_t bool

typedef int (*printf_f)(const char *fmt, ...);

#endif /* !LIB_TYPES_H */
