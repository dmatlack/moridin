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
#include <kernel/io.h>

piix_ide_device_list_t __piix_ide_devices;

/**
 * @brief Global initializer for the PIIX IDE driver subsystem.
 *
 * @return 0
 */
int piix_ide_init(void) {
  TRACE();
  list_init(&__piix_ide_devices);
  return 0;
}


//TODO add support for the device id: 0x1230
struct pci_device_driver __piix_ide_driver = {
  .name       = "piix_ide",
  .id         = STRUCT_PCI_DEVICE_ID(0x8086, 0x7010, 0x1, 0x1),
  .init       = piix_ide_init,
  .new_device = piix_ide_device_init,
};

/**
 * @brief Initialize new IDE device from a PCI device.
 *
 * @return
 *    EINVAL if the device and driver don't match
 *    EEXIST if the driver is already handling this device
 *    ENOMEM if the driver ran out of memory
 */
int piix_ide_device_init(struct pci_device *pci_d) {
  struct piix_ide_device *piix_d;
  unsigned bm_base_addr;
  int ret;

  TRACE("pci_d=%p", pci_d);
  ASSERT_NOT_NULL(pci_d);

  if ((pci_d->progif & (1 << 7)) == 0) {
    ERROR("Suspected PIIX IDE device %02x:%02x.%02x does not support "
          "Bus Master IDE capabilities. (progif=0x%02x)",
          pci_d->bus, pci_d->device, pci_d->func, pci_d->progif);
    return EINVAL;
  }

  DEBUG("Initializing PIIX IDE PCI device: %02x:%02x.%02x (device=0x%04x)",
          pci_d->bus, pci_d->device, pci_d->func,
          pci_d->device_id);

  kprintf("PIIX IDE Controller:\n");

  /*
   * Make sure this device has not already been added
   */
  list_foreach(piix_d, &__piix_ide_devices, piix_link) {
    if (piix_d->pci_d == pci_d) {
      return EEXIST;
    }
  }

  piix_d = kmalloc(sizeof(struct piix_ide_device));
  if (NULL == piix_d) {
    return ENOMEM;
  }

  piix_d->pci_d = pci_d;

  /*
   * BAR 4 of the PCI configuration space holds the bus master base address
   */
  bm_base_addr = pci_config_inl(pci_d, PCI_BAR4) & ~MASK(2);

  /*
   * Initialize the primary IDE controller
   */
  piix_d->primary_bm.cmd     = bm_base_addr + PCI_BM_PRIMARY + PCI_BM_CMD;
  piix_d->primary_bm.status  = bm_base_addr + PCI_BM_PRIMARY + PCI_BM_STATUS;
  piix_d->primary_bm.pdtable = bm_base_addr + PCI_BM_PRIMARY + PCI_BM_PDTABLE;
  
  ret = ata_new_bus(&piix_d->primary_ata,
     PIIX_PRIMARY_IRQ, PIIX_PRIMARY_ATA_CMD, PIIX_PRIMARY_ATA_CTL);

  if (0 != ret) {
    ERROR("ata_new_bus error: %s", strerr(ret));
    goto piix_init_device_cleanup;
  }

  /*
   * Initialize the secondary IDE controller
   */
  piix_d->secondary_bm.cmd     = bm_base_addr + PCI_BM_SECONDARY + PCI_BM_CMD;
  piix_d->secondary_bm.status  = bm_base_addr + PCI_BM_SECONDARY + PCI_BM_STATUS;
  piix_d->secondary_bm.pdtable = bm_base_addr + PCI_BM_SECONDARY + PCI_BM_PDTABLE;

  ret = ata_new_bus(&piix_d->secondary_ata,
     PIIX_SECONDARY_IRQ, PIIX_SECONDARY_ATA_CMD, PIIX_SECONDARY_ATA_CTL);

  if (0 != ret) {
    ERROR("ata_new_bus error: %s", strerr(ret));
    goto piix_init_device_cleanup;
  }

  list_elem_init(piix_d, piix_link);
  list_insert_tail(&__piix_ide_devices, piix_d, piix_link);
  return 0;

piix_init_device_cleanup:
  kprintf("    error: %s", strerr(ret));
  kfree(piix_d, sizeof(struct piix_ide_device));
  return ret;
}
