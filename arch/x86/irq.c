/**
 * @file arch/x86/irq.c
 *
 * TODO move most of me out of arch/x86/ and into kernel/
 */
#include <arch/x86/irq.h>
#include <arch/x86/idt.h>
#include <arch/x86/pic.h>
#include <kernel/kprintf.h>
#include <debug.h>
#include <arch/x86/io.h>
#include <assert.h>
#include <list.h>

int spurious_irqs = 0;

static void spurious_irq(struct irq_context *c) {
  (void)c;
  spurious_irqs++;
}
static struct irq_handler spurious_irq_handler;

/*
 * For each IRQ, we keep a list of handlers that are registered to receive
 * said IRQ.
 */
static irq_handler_list_t irq_handlers[MAX_IRQS]; // FIXME magic numbers

int irq_init(void) {
  int i;

  for (i = 0; i < MAX_IRQS; i++) {
    list_init(&irq_handlers[i]);
  }

  /*
   * Add as handler for spurious irqs (7)
   */
  spurious_irq_handler.top_handler = spurious_irq;
  spurious_irq_handler.bottom_handler = NULL;
  list_elem_init(&spurious_irq_handler, link);
  list_insert_tail(&irq_handlers[7], &spurious_irq_handler, link);

  return 0;
}

void generate_irq(int irq) {
  if (irq < 8) {
    __int(IDT_PIC_MASTER_OFFSET + irq);
  }
  else if (irq < 16) {
    __int(IDT_PIC_SLAVE_OFFSET + irq);
  }
}

void handle_irq(int irq) {
  struct irq_handler *handler;
  struct irq_context context;

  context.irq = irq;

  list_foreach(handler, &(irq_handlers[irq]), link) {
    if (NULL != handler->top_handler) handler->top_handler(&context);
  }

  pic_eoi(irq);

  //TODO way in the future, delay this work using a high priority kernel thread
  // or something similar. At this moment we are still in interrupt context!!
  list_foreach(handler, &(irq_handlers[irq]), link) {
    if (NULL != handler->bottom_handler) handler->bottom_handler(&context);
  }
}

void register_irq(int irq, struct irq_handler *new_handler) {
  ASSERT(irq >= 0 && irq <= 15);

  //TODO disable_interrupts

  list_elem_init(new_handler, link);
  list_insert_tail(&(irq_handlers[irq]), new_handler, link);

  //TODO restore_interrupts
}
