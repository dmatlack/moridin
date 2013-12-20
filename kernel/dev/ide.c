/**
 * @file dev/ide.c
 */
#include <dev/ide.h>
#include <dev/pci.h>
#include <errno.h>
#include <debug.h>
#include <assert.h>
#include <kernel.h>

ide_device_list_t __ide_devices;

int ide_init(void) {
  TRACE();
  list_init(&__ide_devices);
  return 0;
}

/**
 * @brief Initialize an IDE device from a PCI device.
 */
int ide_device_init(struct pci_device *pci_d) {
  struct ide_device *ide_d;

  TRACE("pci_d=%p", pci_d);
  ASSERT_NOT_NULL(pci_d);

  if (pci_d->classcode != 0x1 || pci_d->subclass != 0x1) {
    return EINVAL;
  }

  ide_d = kmalloc(sizeof(struct ide_device));
  if (NULL == ide_d) {
    return ENOMEM;
  }

  list_elem_init(ide_d, global_link);
  ide_d->pci_d = pci_d;

  list_insert_tail(&__ide_devices, ide_d, global_link);

  kprintf("ide device: interrupt_line=0x%x, interrupt_pin=0x%x\n",
          pci_d->interrupt_line, pci_d->interrupt_pin);


  return 0;
}
