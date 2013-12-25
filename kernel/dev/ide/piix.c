/**
 * @file dev/ide/piix.c
 */
#include <dev/ide/piix.h>
#include <dev/pci.h>
#include <dev/ata.h>

#include <errno.h>
#include <debug.h>
#include <assert.h>
#include <kernel.h>
#include <arch/x86/io.h>

piix_ide_device_list_t __piix_ide_devices;

//TODO add support for the device id: 0x1230
struct pci_device_driver __piix_ide_driver = {
  .name       = "piix_ide",
  .id         = STRUCT_PCI_DEVICE_ID(0x8086, 0x7010, 0x1, 0x1),
  .init       = piix_ide_init,
  .new_device = piix_ide_device_init,
};


int piix_ide_init(void) {
  TRACE();
  list_init(&__piix_ide_devices);
  return 0;
}

/**
 * @brief Initialize new IDE device from a PCI device.
 */
int piix_ide_device_init(struct pci_device *pci_d) {
  struct piix_ide_device *ide_d;
  int ret;

  TRACE("pci_d=%p", pci_d);
  ASSERT_NOT_NULL(pci_d);

  if ((pci_d->progif & (1 << 7)) == 0) {
    ERROR("Suspected PIIX IDE device %02x:%02x.%02x does not support "
          "Bus Master IDE capabilities. (progif=0x%02x)",
          pci_d->bus, pci_d->device, pci_d->func, pci_d->progif);
    return EINVAL;
  }

  INFO("Initializing PIIX IDE PCI device: %02x:%02x.%02x (device=0x%04x)",
          pci_d->bus, pci_d->device, pci_d->func,
          pci_d->device_id);

  /*
   * Make sure this device has not already been added
   */
  list_foreach(ide_d, &__piix_ide_devices, piix_link) {
    if (ide_d->pci_d == pci_d) return 0;
  }

  ide_d = kmalloc(sizeof(struct piix_ide_device));
  if (NULL == ide_d) {
    return ENOMEM;
  }

  ide_d->pci_d = pci_d;

  list_elem_init(ide_d, piix_link);
  list_insert_tail(&__piix_ide_devices, ide_d, piix_link);

  /*
   * BAR 4 of the PCI configuration space holds the Bus Master Interface
   * Base Address (BMIBA).
   */
  ide_d->BMIBA = pci_config_ind(pci_d, PCI_BAR4) & ~MASK(2);
  INFO("Bus Master Base Address: 0x%08x", ide_d->BMIBA);

  /*
   * PIIX has 2 ATA buses: Primary and Secondary
   */
  if (0 != (ret = ata_new_bus(ide_d->ata + 0, 
                              PRIMARY_CMD_BLOCK_OFFSET, 
                              PRIMARY_CTL_BLOCK_OFFSET))) {
    WARN("ata_new_bus error: %s", strerr(ret));
  }
  if (0 != (ret = ata_new_bus(ide_d->ata + 1, 
                              SECONDARY_CMD_BLOCK_OFFSET, 
                              SECONDARY_CTL_BLOCK_OFFSET))) {
    WARN("ata_new_bus error: %s", strerr(ret));
  }

  return 0;
}
