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

int ide_device_init(struct pci_device *pci_d) {
  struct ide_device *ide_d;

  TRACE("pci_d=%p", pci_d);
  ASSERT_NOT_NULL(pci_d);

  ide_d = kmalloc(sizeof(struct ide_device));
  if (NULL == ide_d) {
    return ENOMEM;
  }

  list_elem_init(ide_d, global_link);
  ide_d->pci_d = pci_d;

  list_insert_tail(&__ide_devices, ide_d, global_link);

  return 0;
}
