#ifndef __KERNEL_COMPILER_H__
#define __KERNEL_COMPILER_H__

#define __section(_s) __attribute__(( section( _s ) ))
#define __aligned(_a) __attribute__(( aligned( _a ) ))

#endif /* __KERNEL_COMPILER_H__ */
