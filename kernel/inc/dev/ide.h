/**
 * @file dev/ide.h
 */
#ifndef __DEV_IDE_H__
#define __DEV_IDE_H__

#include <dev/pci.h>
#include <dev/ata.h>

struct ide_device {
	struct ata_bus ata;
	struct pci_bus_master bm;
};

int ide_init(struct ide_device *ide, unsigned bm, int irq,
		unsigned ata_cmd, unsigned ata_ctl);

void ide_destroy(struct ide_device *ide);

#endif /* !__DEV_IDE_H__ */
