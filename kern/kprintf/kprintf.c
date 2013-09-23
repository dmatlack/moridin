/*
 * Mach Operating System
 * Copyright (c) 1993-1989 Carnegie Mellon University.
 * Copyright (c) 1994 The University of Utah and
 * the Computer Systems Laboratory (CSL).
 * All rights reserved.
 *
 * Permission to use, copy, modify and distribute this software and its
 * documentation is hereby granted, provided that both the copyright
 * notice and this permission notice appear in all copies of the
 * software, derivative works or modified versions, and any portions
 * thereof, and that both notices appear in supporting documentation.
 *
 * CARNEGIE MELLON, THE UNIVERSITY OF UTAH AND CSL ALLOW FREE USE OF
 * THIS SOFTWARE IN ITS "AS IS" CONDITION, AND DISCLAIM ANY LIABILITY
 * OF ANY KIND FOR ANY DAMAGES WHATSOEVER RESULTING FROM THE USE OF
 * THIS SOFTWARE.
 *
 * Carnegie Mellon requests users of this software to return to
 *
 *  Software Distribution Coordinator  or  Software.Distribution@CS.CMU.EDU
 *  School of Computer Science
 *  Carnegie Mellon University
 *  Pittsburgh PA 15213-3890
 *
 * any improvements or extensions that they make and grant Carnegie Mellon
 * the rights to redistribute these changes.
 */


/*****
 * 
 * Modified by David Matlack
 *
 *    Added 'k' prefix to datatypes and function names.
 *
 *****/


#include <stdarg.h>
#include <fmt/doprnt.h>
#include <dev/vga.h>

/* This version of printf is implemented in terms of putchar and puts.  */

#define	KPRINTF_BUFMAX	128

struct kprintf_state {
	char buf[KPRINTF_BUFMAX];
	unsigned int index;
};

int kputchar(int c)
{
	vga_putbyte(c);
  return c;
}

/* Simple puts() implementation that just uses putchar().
   Note that our printf() is implemented
   in terms of only puts() and putchar(), so that's all we need.
   The only reason the caller might want to replace this function
   is if putchar() has extremely high overhead for some reason.  */
int kputs(const char *s)
{
	while (*s)
	{
		kputchar(*s);
		s++;
	}
	kputchar('\n');
	return 0;
}

static void
kflush(struct kprintf_state *state)
{
	int i;

	for (i = 0; i < state->index; i++)
		kputchar(state->buf[i]);

	state->index = 0;
}

static void
kprintf_char(arg, c)
	char *arg;
	int c;
{
	struct kprintf_state *state = (struct kprintf_state *) arg;

	if (c == '\n')
	{
		state->buf[state->index] = 0;
		kputs(state->buf);
		state->index = 0;
	}
	else if ((c == 0) || (state->index >= KPRINTF_BUFMAX))
	{
		kflush(state);
		kputchar(c);
	}
	else
	{
		state->buf[state->index] = c;
		state->index++;
	}
}

/*
 * Printing (to console)
 */
int kvprintf(const char *fmt, va_list args)
{
	struct kprintf_state state;

	state.index = 0;
	_doprnt(fmt, args, 0, (void (*)())kprintf_char, (char *) &state);

	if (state.index != 0)
	    kflush(&state);

	/* _doprnt currently doesn't pass back error codes,
	   so just assume nothing bad happened.  */
	return 0;
}

int
kprintf(const char *fmt, ...)
{
	va_list	args;
	int err;

	va_start(args, fmt);
	err = kvprintf(fmt, args);
	va_end(args);

	return err;
}

