/* @file lib/stdint.h
 * @author matthewj S2008
 * @brief Standard integer type definitions
 */

#ifndef LIB_STDINT_H
#define LIB_STDINT_H

#ifndef ASSEMBLER

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned long uint32_t;
typedef unsigned long long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed long int32_t;
typedef signed long long int64_t;

typedef int intptr_t;
typedef unsigned int uintptr_t;

#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define s8 int8_t
#define s16 int16_t
#define s32 int32_t
#define s64 int64_t

#endif /* !ASSEMBLER */

#endif /* !LIB_STDINT_H */
