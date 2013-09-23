/* 
 * Copyright (c) 1996 The University of Utah and
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
/*
 * Alternate version of memalign
 * that expects the caller to keep track of the size of allocated chunks.
 * This version is _much_ more memory-efficient
 * when allocating many chunks naturally aligned to their (natural) size
 * (e.g. allocating naturally-aligned pages or superpages),
 * because normal memalign requires a prefix between each chunk
 * which will create horrendous fragmentation and memory loss.
 * Chunks allocated with this function must be freed with sfree().
 */

#include <stddef.h>
#include "malloc_internal.h"

void *_smemalign(size_t alignment, size_t size)
{
	unsigned shift;
	void *chunk;

	/* Find the alignment shift in bits.  XXX use proc_ops.h  */
	for (shift = 0; (1 << shift) < alignment; shift++);

	/*
	 * Allocate a chunk of LMM memory with the specified alignment shift
	 * and an offset such that the memory block we return will be aligned.
	 */
	if (!(chunk = lmm_alloc_aligned(&malloc_lmm, size, 0, shift, 0)))
		return NULL;

	return chunk;
}

