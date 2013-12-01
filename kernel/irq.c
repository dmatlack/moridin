/**
 * @file kernel/irq.c
 *
 * @brief Hardware Interrupts
 *
 * @author David Matlack
 */
#include <kernel/irq.h>
#include <kernel/kprintf.h>
#include <debug.h>
#include <assert.h>
#include <list.h>

#ifdef ARCH_X86
#include <arch/x86/irq.h>
#endif

#define FIXME_NUM_IRQS 16

/*
 * For each IRQ, we keep a list of handlers that are registered to receive
 * said IRQ.
 */
static irq_handler_list_t irq_handlers[FIXME_NUM_IRQS];

int irq_init(void) {
  int i;

#ifdef ARCH_X86
  x86_init_irq();
#endif

  for (i = 0; i < FIXME_NUM_IRQS; i++) {
    list_init(&irq_handlers[i]);
  }

  return 0;
}

void generate_irq(int irq) {
#ifdef ARCH_X86
  x86_generate_irq(irq);
#endif
}

void acknowledge_irq(int irq) {
#ifdef ARCH_X86
  x86_acknowledge_irq(irq);
#endif
}


void handle_irq(int irq) {
  struct irq_handler *handler;
  struct irq_context context;

  context.irq = irq;

  list_foreach(handler, &(irq_handlers[irq]), link) {
    if (NULL != handler->top_handler) handler->top_handler(&context);
  }

  acknowledge_irq(irq);

  //FIXME move this out of interrupt context
  list_foreach(handler, &(irq_handlers[irq]), link) {
    if (NULL != handler->bottom_handler) handler->bottom_handler(&context);
  }
}

void register_irq(int irq, struct irq_handler *new_handler) {
  ASSERT(irq >= 0 && irq <= 15);

  list_elem_init(new_handler, link);

  //TODO disable_interrupts

  list_insert_tail(&(irq_handlers[irq]), new_handler, link);

  //TODO restore_interrupts
}
