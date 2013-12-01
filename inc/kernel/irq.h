/**
 * @file kernel/irq.h
 *
 * @author David Matlack
 */
#ifndef __KERNEL_IRQ_H__
#define __KERNEL_IRQ_H__

#include <list.h>

struct irq_context {
  int irq;
};

typedef void (*irq_handler_f)(struct irq_context *context);

struct irq_handler {
  /**
   * @brief The top_handler, if registered, is executed in interrupt context,
   * while interrupts are disabled. It should be short and sweet.
   */
  irq_handler_f top_handler;
  /**
   * @brief The bottom_handler, if registered, is executed some time after
   * the interrupt has been acknowledged. It runs with interrupts enabled.
   */
  irq_handler_f bottom_handler;

  list_link(struct irq_handler) link;
};

list_typedef(struct irq_handler) irq_handler_list_t;

int irq_init(void);
void generate_irq(int irq);
void handle_irq(int irq);
void register_irq(int irq, struct irq_handler *new_handler);

#endif /* !__KERNEL_IRQ_H__ */
