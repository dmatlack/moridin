/**
 * @file dev/ide/piix.h
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
 *
 * TO READ: http://pdos.csail.mit.edu/6.828/2006/readings/hardware/IDE-BusMaster.pdf
 */
#ifndef __DEV_IDE_H__
#define __DEV_IDE_H__

#include <dev/pci.h>
#include <list.h>
#include <dev/ata.h>

#define PIIX_PRIMARY_IRQ        14
#define PIIX_PRIMARY_ATA_CMD    0x1F0
#define PIIX_PRIMARY_ATA_CTL    0x3F4

#define PIIX_SECONDARY_IRQ      15
#define PIIX_SECONDARY_ATA_CMD  0x170
#define PIIX_SECONDARY_ATA_CTL  0x374

struct piix_ide_device {
  struct pci_device *pci_d;
  
  struct ata_bus        primary_ata;
  struct pci_bus_master primary_bm;

  struct ata_bus        secondary_ata;
  struct pci_bus_master secondary_bm;

  list_link(struct piix_ide_device) piix_link;
};

list_typedef(struct piix_ide_device) piix_ide_device_list_t;

int piix_ide_init(void);
int piix_ide_device_init(struct pci_device *pci_d);

#endif /* ! __DEV_IDE_H__ */
