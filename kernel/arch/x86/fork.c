/**
 * @file arch/x86/fork.c
 *
 * @brief x86-specific functions for performing a process fork.
 */
#include <arch/vm.h>
#include <arch/syscall.h>
#include <kernel/proc.h>
#include <kernel/sched.h>
#include <stddef.h>

void fork_context(struct thread *new_thread)
{
	struct registers *cs_regs; /* context switch registers */
	u32 new_cr3 = (u32) new_thread->proc->space.mmu;
	u32 *esp = (u32 *) _KSTACK_END(new_thread);
	u32 *ebp;

	/*
	 * Copy the current (aka parent) thread's syscall registers to the
	 * top of the new thread's kernel stack. These registers will allow
	 * the new thread to call return_from_syscall and get to userspace.
	 */
	esp -= sizeof(struct registers) / sizeof(*esp);
	new_thread->regs = (struct registers *) esp;
	memcpy(new_thread->regs, CURRENT_THREAD->regs,
	       sizeof(struct registers));

	new_thread->regs->cr3 = new_cr3;

	 /* a fake return address */
	*(--esp) = 0xDEADBEEF;

	/* __context_switch returns to ... */
	*(--esp) = (u32) &child_return_from_fork;

	/*
	 * Next is the old frame pointer for returning from
	 * __context_switch.
	 */
	--esp;
	*(esp) = (u32) (esp + 1);
	ebp = esp;

	/*
	 * Finally put fake registers on the stack for __context_switch.
	 */
	esp -= sizeof(struct registers) / sizeof(*esp);
	cs_regs = (struct registers *) esp;
	memset(cs_regs, 0, sizeof(struct registers));

	cs_regs->cr3 = new_cr3;
	cs_regs->ebp = (u32) ebp;
	cs_regs->ds = SEGSEL_KERNEL_DS;
	cs_regs->es = SEGSEL_KERNEL_DS;
	cs_regs->fs = SEGSEL_KERNEL_DS;
	cs_regs->gs = SEGSEL_KERNEL_DS;

	new_thread->context = cs_regs;
}

static void fork_pde(entry_t *from_pde, entry_t *to_pde, unsigned long virt)
{
	struct entry_table *to_pt;
	struct entry_table *from_pt;
	unsigned j;

	from_pt = entry_pt(from_pde);

	to_pt = new_entry_table();
	if (!to_pt) {
		panic("TODO: unmap all pages.");
	}

	/*
	 * Copy the page directory entry.
	 */
	*to_pde = *from_pde;

	/*
	 * But use the address of the newly created page table.
	 */
	entry_set_addr(to_pde, __phys(to_pt));

	for (j = 0; j < ENTRY_TABLE_SIZE; j++) {
		entry_t *from_pte = from_pt->entries + j;
		entry_t *to_pte = to_pt->entries + j;

		/*
		 * Copy the page table entry.
		 */
		*to_pte = *from_pte;

		/*
		 * If the entry is mapped to a physical page, increase the
		 * reference count on that page by 1 as it is being used
		 * in the new address space.
		 */
		if (entry_is_present(from_pte)) {
			unsigned long phys = entry_phys(from_pte);

			/*
			 * Increase the reference counter on the page since
			 * another page table now points to it.
			 */
			page_get(page_struct(phys));

			/*
			 * Mark the page readonly in both page tables so
			 * both processes will page fault on a write and
			 * we can copy the page.
			 */
			entry_set_readonly(from_pte);
			entry_set_readonly(to_pte);

			tlb_invalidate(virt, PAGE_SIZE);
		}

		virt += PAGE_SIZE;
	}
}

int fork_address_space(struct entry_table *to_pd, struct entry_table *from_pd)
{
	unsigned i;

	TRACE("to_pd=0x%08x, from_pd=0x%08x", to_pd, from_pd);

	for (i = 0; i < ENTRY_TABLE_SIZE; i++) {
		unsigned long virt = i * (PAGE_SIZE * ENTRY_TABLE_SIZE);
		entry_t *from_pde = from_pd->entries + i;
		entry_t *to_pde = to_pd->entries + i;

		if (kernel_address(virt))
			/*
			 * All processes share the same page tables to map
			 * the kernel. So the only thing we need to copy for
			 * a kernel mapping is to copy the page directory
			 * entries.
			 */
			*to_pde = *from_pde;
		else if (entry_is_present(from_pde))
			/*
			 * If it's a user address, and the page directory
			 * entry is present, we assume that it's pointing
			 * to a page table and we make a copy of that page
			 * table instead.
			 *
			 * WARNING: using 4MB pages (PSE) will break this
			 * code.
			 */
			fork_pde(from_pde, to_pde, virt);
	}

	return 0;
}

