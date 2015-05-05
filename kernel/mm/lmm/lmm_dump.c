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
/*
 * Debugging routine:
 * dump an LMM memory pool and do a thorough sanity check on it.
 */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"

#include <assert.h>

#include <mm/lmm.h>
#include <mm/lmm_types.h>

void lmm_dump(lmm_t *lmm)
{
	struct lmm_region *reg;

	TRACE("lmm=%p", lmm);

	for (reg = lmm->regions; reg; reg = reg->next)
	{
		struct lmm_node *node;
		vm_size_t free_check;

		INFO(" region 0x%08lx-0x%08lx size=0x%08lx flags=0x%08x pri=%d free=0x%08lx",
			reg->min, reg->max, reg->max - reg->min,
			reg->flags, reg->pri, reg->free);

		ASSERT((reg->nodes == 0)
		       || (vm_offset_t)reg->nodes >= reg->min);
		ASSERT((reg->nodes ==0) || (vm_offset_t)reg->nodes < reg->max);
		ASSERT(reg->free >= 0);
		ASSERT(reg->free <= reg->max - reg->min);

		free_check = 0;
		for (node = reg->nodes; node; node = node->next)
		{
			INFO("  node %p-0x%08lx size=0x%08lx next=%p",
				node, (vm_offset_t)node + node->size, node->size, node->next);

			ASSERT(((vm_offset_t)node & ALIGN_MASK) == 0);
			ASSERT((node->size & ALIGN_MASK) == 0);
			ASSERT(node->size >= sizeof(*node));
			ASSERT((node->next == 0) || (node->next > node));
			ASSERT((vm_offset_t)node < reg->max);

			free_check += node->size;
		}

		INFO(" free_check=0x%08lx", free_check);
		ASSERT(reg->free == free_check);
	}

	INFO("lmm_dump done");
}

#pragma GCC diagnostic pop
