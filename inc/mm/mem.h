/**
 * @file mm/mem.h
 *
 * @author David Matlack
 */
#ifndef __MM_MEM_H__
#define __MM_MEM_H__

#include <types.h>

//FIXME
//FIXME Is this bad using these as globals that just anyone can touch and
//FIXME edit? 
//FIXME

extern size_t kernel_image_start;
extern size_t kernel_image_end;

extern size_t kernel_mem_start;
extern size_t kernel_mem_end;

extern size_t user_mem_start;
extern size_t user_mem_end;

void mem_layout_dump(printf_f p);

#endif /* !__MM_MEM_H__ */
