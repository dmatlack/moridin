/**
 * @file dev/ide.h
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
 * TO-READ: http://pdos.csail.mit.edu/6.828/2006/readings/hardware/IDE-BusMaster.pdf
 */
#ifndef __DEV_IDE_H__
#define __DEV_IDE_H__

#include <dev/pci.h>
#include <list.h>

/*
 * PCI Bus Master IDE I/O Registers
 *
 * This is a 16-byte I/O space, pointed to by the BMIBA. Each value below
 * is an offset from the BMIBA.
 */
#define BUS_MASTER_IDE_PRIMARY_CMD                0x00 // 1 byte
#define BUS_MASTER_IDE_PRIMARY_STATUS             0x02 // 1 byte
#define BUS_MASTER_IDE_PRIMARY_DESCRIPTOR_TABLE   0x04 // 4 bytes
#define BUS_MASTER_IDE_SECONDARY_CMD              0x08 // 1 byte
#define BUS_MASTER_IDE_SECONDARY_STATUS           0x0A // 1 byte
#define BUS_MASTER_IDE_SECONDARY_DESCRIPTOR_TABLE 0x0C // 4 bytes

struct ide_device {
  struct pci_device *pci_d;

  int irq;

  /*
   * Bus Master Interface Base Address
   */
  uint32_t BMIBA;

  list_link(struct ide_device) global_link;
};

list_typedef(struct ide_device) ide_device_list_t;
extern ide_device_list_t __ide_devices;

int ide_init(void);
int ide_device_init(struct pci_device *pci_d);

#endif /* ! __DEV_IDE_H__ */
