/**
 * @file kernel/irq.c
 *
 * @brief Hardware Interrupts
 *
 * @author David Matlack
 */
#include <kernel/irq.h>

#include <dev/vga.h>

#include <kernel.h>
#include <debug.h>
#include <assert.h>
#include <list.h>
#include <errno.h>

int __max_irqs;
struct irq_state *__irqs;

#ifdef ARCH_X86
typedef int irq_mask_t;
extern struct machine_irq_interface x86_irq_interface;
static struct machine_irq_interface *__machine_irq_system = &x86_irq_interface;
#endif

int irq_init(void) {
  struct machine_irq_info irq_info;
  int i;
  int ret;

  TRACE("");

  if (0 != (ret = __machine_irq_system->init(&irq_info))) {
    return ret;
  }

  __max_irqs = irq_info.max_irqs;

  __irqs = kmalloc(sizeof(struct irq_state) * __max_irqs);
  if (NULL == __irqs) {
    kprintf("NULL == __irqs\n");
    return ENOMEM;
  }
  
  for (i = 0; i < __max_irqs; i++) {
    struct irq_state *irq = __irqs + i;
    list_init(&irq->handlers);
    irq->count = 0;
    irq->in_irq = 0;
  }

  return 0;
}

void generate_irq(int irq) {
  __machine_irq_system->generate_irq(irq);
}

void acknowledge_irq(int irq) {
  __machine_irq_system->acknowledge_irq(irq);
}

void handle_irq(int irq) {
  struct irq_context context;
  struct irq_state *state;
  struct irq_handler *handler;

  state = &__irqs[irq];

  //FIXME atomic test and set
  ASSERT_EQUALS(0, state->in_irq);
  state->in_irq++;
  state->count++;

  context.irq = irq;

  list_foreach(handler, &state->handlers, link) {
    if (NULL != handler->top_handler) handler->top_handler(&context);
  }

  state->in_irq--;
  acknowledge_irq(irq);
}

void register_irq(int irq, struct irq_handler *new_handler) {
  ASSERT_GREATEREQ(irq, 0);
  ASSERT_LESSEQ(irq, 15);

  list_elem_init(new_handler, link);

  //TODO disable_interrupts spinlock

  list_insert_tail(&__irqs[irq].handlers, new_handler, link);

  //TODO restore_interrupts spinlock
}

void enable_irqs(void) {
  __machine_irq_system->enable_irqs();
}

void disable_irqs(void) {
  __machine_irq_system->disable_irqs();
}
