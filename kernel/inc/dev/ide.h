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
 * An IDE can connect up to 4 drives, each of which may be:
 *   - ATA (Serial)       Most hard drives
 *   - ATA (Parallel)     Common among hard drives
 *   - ATAPI (Serial)     Most optical drives
 *   - ATAPI (Parallel)   Common among optical drives
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
