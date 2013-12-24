/**
 * @file dev/ide/piix.c
 */
#include <dev/ide/piix.h>
#include <dev/pci.h>

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

const char *ata_device_str(uint8_t clo, uint8_t chi) {
  if (clo == 0x14 && chi == 0xEB) return "PATAPI";
  if (clo == 0x69 && chi == 0x96) return "SATAPI";
  if (clo == 0x00 && chi == 0x00) return "PATA";
  if (clo == 0x3c && chi == 0xc3) return "SATA";
  return "unknown";
}

static void piix_ide_identify(int block_offset, int select_drive) {
  uint8_t data[256];
  uint8_t idstatus;

  TRACE("block_offset=0x%04x, select_drive=0x%04x", block_offset, select_drive);

  if (inb(block_offset + IDEIO_CMD_STATUS) == 0xff) {
    INFO("There are no drives attached to this bus!"); 
    return;
  }

  /*
   * Select the appropriate drive
   */
  outb(block_offset + IDEIO_CMD_DRIVE, select_drive);

  /*
   * Set all the info registers (sector count, cylinders lo/hi, etc... to 0
   */ 
  outb(block_offset + IDEIO_CMD_SECTOR_COUNT, 0);
  outb(block_offset + IDEIO_CMD_SECTOR_NUM,   0);
  outb(block_offset + IDEIO_CMD_CYLINDER_LO,  0);
  outb(block_offset + IDEIO_CMD_CYLINDER_HI,  0);

  /*
   * IDENTIFY yourself, drive!
   */
  outb(block_offset + IDEIO_CMD_COMMAND, IDEIO_IDENTIFY);
  idstatus = inb(block_offset + IDEIO_CMD_STATUS);

  if (0 == idstatus) {
    INFO("Drive does not exist!");
  }
  else {
    uint8_t clo, chi;

    INFO("Drive exists. status=0x%04x (BSY=%d)", idstatus, idstatus & IDEIO_BSY);
    /*
     * The drive exists! Poll the BSY bit until it is cleared.
     */
    do {
      idstatus = inb(block_offset + IDEIO_CMD_STATUS);
      
      /*
       * ATAPI and SATA devices *should* report error in response to an identify.
       */
      if (idstatus & IDEIO_ERR) break;
    } while ((idstatus & IDEIO_BSY) != 0);
    INFO("Done waiting for BSY.");

    /*
     * If CYLINDER_LO or CYLINDER_HI are non-zero, this is not an ATA drive
     */
    clo = inb(block_offset + IDEIO_CMD_CYLINDER_LO);
    chi = inb(block_offset + IDEIO_CMD_CYLINDER_HI);
    kprintf("clo=0x%02x, chi=0x%02x (%s)\n", clo, chi, ata_device_str(clo, chi));

    if (clo != 0 || chi != 0) {
      INFO("Drive is not ATA! (clo=0x%02x, chi=0x%02x)", clo, chi);
    }
    else {
      /*
       * Continue polling until DRQ (ready to transfer data) is set or
       * ERR (error) is set.
       */
      do {
        idstatus = inb(block_offset + IDEIO_CMD_STATUS);
      } while (((idstatus & IDEIO_DRQ) == 0) && ((idstatus & IDEIO_ERR) == 0));
      INFO("Done waiting for DRQ/ERR");
      
      if ((idstatus & IDEIO_ERR) != 0) {
        INFO("Error occured while waiting!");
      }
      else {
        int i;

        INFO("IDENTIFY succeeded!");
        for (i = 0; i < 256; i++) {
          data[i] = inb(IDEIO_CMD_DATA);
        }
        (void)data;
      }
    }
  }
}

/**
 * @brief Initialize new IDE device from a PCI device.
 */
int piix_ide_device_init(struct pci_device *pci_d) {
  struct piix_ide_device *ide_d;

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

  piix_ide_identify(PRIMARY_CMD_BLOCK_OFFSET, IDEIO_SELECT_MASTER);
  piix_ide_identify(PRIMARY_CMD_BLOCK_OFFSET, IDEIO_SELECT_SLAVE);
  piix_ide_identify(SECONDARY_CMD_BLOCK_OFFSET, IDEIO_SELECT_MASTER);
  piix_ide_identify(SECONDARY_CMD_BLOCK_OFFSET, IDEIO_SELECT_SLAVE);

  return 0;
}
