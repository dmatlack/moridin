/* 
 * Copyright (c) 1996-1994 The University of Utah and
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

/****
 *
 * Modified by David Matlack:
 *  
 *    Moved the following out of stdio.h and into fmt.h in order to
 *    separate the print functionality and format functionality.
 *
 ****/

#ifndef _FMT_H_
#define _FMT_H_

int vprintf(const char *__format, va_list __vl);
int sprintf(char *__dest, const char *__format, ...)
            __attribute__((__format__ (__printf__, 2, 3)));
int snprintf(char *__dest, size_t __size, const char *__format, ...)
             __attribute__((__format__ (__printf__, 3, 4)));
int vsprintf(char *__dest, const char *__format, va_list __vl);
int vsnprintf(char *__dest, size_t __size, const char *__format, va_list __vl);
int sscanf(const char *__str, const char *__format, ...)
           __attribute__((__format__ (__scanf__, 2, 3)));

#endif /* _FLUX_MC_FMT_H_ */
