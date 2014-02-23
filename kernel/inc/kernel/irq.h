/**
 * @file kernel/irq.h
 *
 * @author David Matlack
 */
#ifndef __KERNEL_IRQ_H__
#define __KERNEL_IRQ_H__

#include <list.h>
#include <arch/irq.h>

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

struct irq_state {
  /**
   * @brief The list of handlers that are register to receive this interrupt.
   */
  irq_handler_list_t handlers;
  /**
   * @brief The number of times this interrupt has occured.
   */
  int count;
  /**
   * @brief Incremented upon receiving an interrupt, decremented after all
   * handlers have been run.
   */
  int in_irq;
};

void irq_init(void);

void kernel_irq_handler(int irq);

void register_irq(int irq, struct irq_handler *new_handler);

void irq_status_bar(int bar_row);

#endif /* !__KERNEL_IRQ_H__ */
