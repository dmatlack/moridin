/**
 * @file kernel/irq.c
 *
 * @brief Hardware Interrupts
 *
 * @author David Matlack
 */
#include <kernel/irq.h>
#include <kernel/atomic.h>
#include <kernel/kmalloc.h>

#include <kernel/debug.h>
#include <assert.h>
#include <list.h>
#include <errno.h>

#include <dev/vga.h>
#include <string.h>

int __max_irqs;
struct irq_state *__irqs;

#ifdef ARCH_X86
typedef int irq_mask_t;
extern struct machine_irq_interface x86_irq_interface;
static struct machine_irq_interface *__machine_irq_system = &x86_irq_interface;
#endif

void irq_init(void) {
  struct machine_irq_info irq_info;
  int i;
  int ret;

  TRACE();
  ASSERT_NOT_NULL(__machine_irq_system);
  ASSERT_NOT_NULL(__machine_irq_system->init);

  if (0 != (ret = __machine_irq_system->init(&irq_info))) {
    panic("Couldn't get architecture irq information: %d/%s", ret, strerr(ret));
  }

  __max_irqs = irq_info.max_irqs;

  ASSERT_EQUALS(__max_irqs, 16);

  __irqs = kmalloc(sizeof(struct irq_state) * __max_irqs);
  if (NULL == __irqs) {
    panic("Not enough memory to allocate the irq list.");
  }
  
  for (i = 0; i < __max_irqs; i++) {
    struct irq_state *irq = __irqs + i;
    list_init(&irq->handlers);
    irq->count = 0;
    irq->in_irq = 0;
  }
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
  int prev_in_irq;

  state = &__irqs[irq];
  atomic_add(&state->count, 1);

  prev_in_irq = atomic_add(&state->in_irq, 1);
  ASSERT_EQUALS(prev_in_irq, 0);
  
  context.irq = irq;
  list_foreach(handler, &state->handlers, link) {
    if (handler->top_handler) handler->top_handler(&context);
  }

  atomic_add(&state->in_irq, -1);

  acknowledge_irq(irq);
}

void register_irq(int irq, struct irq_handler *new_handler) {
  ASSERT_GREATEREQ(irq, 0);
  ASSERT_LESS(irq, __max_irqs);

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

/**
 * @brief Print bar of text accross a row of the console with some useful
 * information about IRQs.
 *
 * @param bar_row The row to draw the bar.
 */
void irq_status_bar(int bar_row) {
  int old_row, old_col;
  char old_color;
  int i;

  vga_get_cursor(&old_row, &old_col);
  old_color = vga_get_color();

  vga_set_color(VGA_COLOR(VGA_WHITE, VGA_BLUE));
  
  vga_set_cursor(bar_row, 0);
  kprintf("IRQs: ");
  for (i = 0; i < __max_irqs; i++) {
    if (__irqs[i].count > 0)
      kprintf("%d:%d ", i, __irqs[i].count);
  }

  vga_set_cursor(old_row, old_col);
  vga_set_color(old_color);
}
