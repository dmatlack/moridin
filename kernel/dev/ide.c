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

struct pci_device_driver __ide_pci_driver = {
  .name = "IDE",
  .id = PCI_DEVICE_ID(PCI_VENDOR_ANY, PCI_DEVICE_ANY, 0x1, 0x1),
  .init = ide_init,
  .new_device = ide_device_init,
};

int ide_init(void) {
  TRACE();
  list_init(&__ide_devices);
  return 0;
}

/**
 * @brief Initialize new IDE device from a PCI device.
 */
int ide_device_init(struct pci_device *pci_d) {
  struct ide_device *ide_d;

  TRACE("pci_d=%p", pci_d);
  ASSERT_NOT_NULL(pci_d);

  if (pci_d->classcode != 0x1 || pci_d->subclass != 0x1) {
    return EINVAL;
  }

  /*
   * Make sure this device has not already been added
   */
  list_foreach(ide_d, &__ide_devices, global_link) {
    if (ide_d->pci_d == pci_d) return 0;
  }

  ide_d = kmalloc(sizeof(struct ide_device));
  if (NULL == ide_d) {
    return ENOMEM;
  }

  list_elem_init(ide_d, global_link);
  ide_d->pci_d = pci_d;

  list_insert_tail(&__ide_devices, ide_d, global_link);

  return ENOMEM;
}
