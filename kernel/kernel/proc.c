/**
 * @file kernel/proc.c
 *
 * @brief Process and Thread management.
 *
 * @author David Matlack
 */
#include <kernel.h>
#include <kernel/proc.h>
#include <kernel/atomic.h>

#include <mm/vm.h>
#include <string.h>

#include <errno.h>
#include <debug.h>
#include <assert.h>

//static int next_pid = 0;
//
//static int get_next_pid() {
//  return atomic_add(&next_pid, 1);
//}
