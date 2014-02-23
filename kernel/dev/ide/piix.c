/**
 * @file dev/ide/piix.c
 *
 * @brief This file implements a driver to control the Intel 82371FB (PIIX)
 * AND 82371SB (PIIX3) IDE controllers.
 *
 * See http://download.intel.com/design/intarch/datashts/29055002.pdf for the
 * data sheet.
 *
 * This IDE controller surfaces itself via the PCI bus interface, as a Mass
 * Storage Controller. The controller uses one base address register from
 * the PCI configuration space, BAR4, which indicates the Bus Master Interface
 * Base Address (BMIBA).
 */
#include <dev/pci.h>
#include <dev/ata.h>
#include <dev/ide.h>

#include <errno.h>
#include <debug.h>
#include <assert.h>
#include <kernel/io.h>

struct ide_device piix_pri_ide;
struct ide_device piix_sec_ide;

/**
 * @brief Global initializer for the PIIX IDE driver subsystem.
 *
 * @return 0
 */
int piix_ide_init(void) {
  TRACE();
  return 0;
}

/**
 * @brief Initialize new IDE device from a PCI device.
 *
 * @return
 *    EINVAL if the device and driver don't match
 *    EEXIST if the driver is already handling this device
 *    ENOMEM if the driver ran out of memory
 */
int piix_ide_device_init(struct pci_device *pci_d) {
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

  INFO("Initializing PIIX IDE PCI device: %02x:%02x.%02x (device=0x%04x)",
       pci_d->bus, pci_d->device, pci_d->func,
       pci_d->device_id);

  kprintf("PIIX IDE Controller:\n");

  /*
   * BAR 4 of the PCI configuration space holds the bus master base address
   */
  bm_base_addr = pci_config_inl(pci_d, PCI_BAR4) & ~MASK(2);

  /*
   * PIIX/3 has 2 IDE controllers: primary and secondary. We will initialize
   * both and look for drives in each.
   */

  ret = ide_init(&piix_pri_ide, bm_base_addr + 0x0, 14, 0x1F0, 0x3F4);
  if (ret) return ret;

  ret = ide_init(&piix_sec_ide, bm_base_addr + 0x8, 15, 0x170, 0x374);
  if (ret) goto cleanup_pri_ide;

  return 0;
cleanup_pri_ide:
  ide_destroy(&piix_pri_ide);
  return ret;
}

/*
 *
 * The PIIX/3 IDE PCI Device Driver
 *
 */
struct pci_device_driver piix_ide_driver = {
  .name       = "piix_ide",
  //TODO add support for the device id: 0x1230
  .id         = STRUCT_PCI_DEVICE_ID(0x8086, 0x7010, 0x1, 0x1),
  .init       = piix_ide_init,
  .new_device = piix_ide_device_init,
};
