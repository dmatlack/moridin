/* 
 * Copyright (c) 1995 The University of Utah and
 * the Computer Systems Laboratory at the University of Utah (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software is hereby
 * granted provided that (1) source code retains these copyright, permission,
 * and disclaimer notices, and (2) redistributions including binaries
 * reproduce the notices in supporting documentation, and (3) all advertising
 * materials mentioning features or use of this software display the following
 * acknowledgement: ``This product includes software developed by the
 * Computer Systems Laboratory at the University of Utah.''
 *
 * THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF THIS SOFTWARE IN ITS "AS
 * IS" CONDITION.  THE UNIVERSITY OF UTAH AND CSL DISCLAIM ANY LIABILITY OF
 * ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF THIS SOFTWARE.
 *
 * CSL requests users of this software to return to csl-dist@cs.utah.edu any
 * improvements that they make and grant CSL redistribution rights.
 */

#ifndef _410KERN_MALLOC_INTERNALS_H_
#define _410KERN_MALLOC_INTERNALS_H_

#include <types.h>
#include <lmm/lmm.h>

extern lmm_t malloc_lmm;

void *_malloc(size_t size);
void *_mustmalloc(size_t size);
void *_memalign(size_t alignment, size_t size);
void *_calloc(size_t nelt, size_t eltsize);
void *_mustcalloc(size_t nelt, size_t eltsize);
void *_realloc(void *buf, size_t new_size);
void _free(void *buf);

/* Alternate version of the standard malloc functions that expect the
   caller to keep track of the size of allocated chunks.  These
   versions are _much_ more memory-efficient when allocating many
   chunks naturally aligned to their (natural) size (e.g. allocating
   naturally-aligned pages or superpages), because normal memalign
   requires a prefix between each chunk which will create horrendous
   fragmentation and memory loss.  Chunks allocated with these
   functions must be freed with sfree() rather than the ordinary
   free(). */
void *_smalloc(size_t size);
void *_smemalign(size_t alignment, size_t size);
void _sfree(void *buf, size_t size);

#endif /* _410KERN_MALLOC_H_ */
