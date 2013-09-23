/* 
 * Copyright (c) 1995-1994 The University of Utah and
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

#include <assert.h>
#include <stddef.h>
#include <malloc/malloc_internal.h>
#include <string.h>

void *
_calloc(size_t nelt, size_t eltsize)
{
	size_t allocsize = nelt * eltsize;

	void *ptr = _malloc(allocsize);
	if (!ptr)
		return NULL;

	memset(ptr, 0, allocsize);

	return ptr;
}

void *
_mustcalloc(size_t nelt, size_t eltsize)
{
	void *buf;

	buf = _calloc(nelt, eltsize);
	assert(buf);

	return buf;
}
