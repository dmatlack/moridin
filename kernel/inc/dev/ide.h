/**
 * @file dev/ide.h
 */
#ifndef __DEV_IDE_H__
#define __DEV_IDE_H__

#include <dev/pci.h>
#include <list.h>

/*************
 * IDE: the electrical specification of the cables which connect ATA drives
 * (like hard drives) to another device. Alternatively, an IDE cable can
 * terminate at an IDE card connect to a PCI (Peripheral Component
 * Interconnect).
 * 
 * An IDE device surfaces itself as a pci device with class code 0x1 (Mass
 * Storage Controller) and subclass 0x1 (IDE). The IDE device only uses 5
 * of the 6 BARs (Base Address Register).
 *
 *  BAR0 - Base address of primary channel (IO space), 0x1F0
 *  BAR1 - Base address of primary channel control port (IO space), 0x365
 *  BAR2 - Base address of secondary channel, 0x170
 *  BAR3 - Base address of secondary channel control port, 0x376
 *  BAR4 - Bus Master IDE (refers to the base of the IO range consisting of
 *         16 ports. each 8 ports controls DMA on the primary and secondary
 *         channel respectively).
 */

struct ide_device {
  struct pci_device *pci_d;

  list_link(struct ide_device) global_link;
};

list_typedef(struct ide_device) ide_device_list_t;
extern ide_device_list_t __ide_devices;

int ide_init(void);
int ide_device_init(struct pci_device *pci_d);

#endif /* ! __DEV_IDE_H__ */
