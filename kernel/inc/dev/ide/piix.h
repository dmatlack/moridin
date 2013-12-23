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

#define IDE_PRIMARY_IRQ   14
#define IDE_SECONDARY_IRQ 15

/******************************************************************************
 *
 * PCI Bus Master IDE I/O Registers
 *
 * This is a 16-byte I/O space, pointed to by the BMIBA. Each value below
 * is an offset from the BMIBA. All bus master IDE I/O space registers can
 * be accessed as byte, word, or dword quantities.
 *
 * There are 2 sets of the same registers: primary and secondary. The way to
 * access each is to use the offsets. e.g.:
 *
 *   BMI Primary Command:   BMICOM + BMI_PRIMARY
 *   BMI Secondary Command: BMICOM + BMI_SECONDARY
 *
 ******************************************************************************/
#define BMI_PRIMARY   0x00
#define BMI_SECONDARY 0x08

/*
 * Bus Master IDE Primary Command (BMICOM)
 *
 *  7 - 4    | 3     | 2 - 1    | 0
 *  RESERVED | RWCON | RESERVED | SSBM
 *
 *    RCWON: Bus Master Read/Write Control. 0=Reads, 1=Writes. This bit must
 *      not be changed when the bus master function is active.
 *
 *    SSBM: Start/Stop Bus Master. 0=Stop, 1=Start. When this bit is set to 1,
 *      the bus master operation starts. The controller transfers data between
 *      the IDE device and memory only when this bit is set. Bust master can 
 *      be stopped by setting this bit to 1 but all information will be lost.
 *
 *      If this bit is set to 0 while the bus master operation is still active
 *      (i.e. bit 0 is 1 in the status register) and the drive has not yet
 *      finished the data transfer (bit 2 is 0 in status register), the bus
 *      master command is aborted. This bit is intended to be set to 0 after the
 *      data transfer is competed, as indicated by either bit 0 or bit 2 being set
 *      in the status register.
 *
 * NOTE: The register is READ/WRITE.
 */
#define BMICOM 0x00

/*
 * Bus Master IDE Status Register (BMISTA)
 *
 *  7        | 6       | 5       | 4 - 3    | 2          | 1         | 0
 *  RESERVED | DMA1CAP | DMA0CAP | RESERVED | IRQ STATUS | DMA ERROR | BMIDEA
 *    
 *    DMA{0,1}CAP: Drive X DMA Capable. 1=Drive X is capable of DMA transfers.
 *      This bit is sofware controlled (R/W) and does not affect hardware
 *      operation.
 *
 *    IRQ STATUS: This bit, when set to 1, indicates when an IDE has asserted
 *      its interrupt line. When bit 2=1 and bit 0=0 (BMIDEA), all read data
 *      from the IDE device has been transferred to main memory and all write
 *      data has been transferred to the IDE device.
 *
 *      Software sets this bit to 0 by writing a 1 to it. (?????????????)
 *
 *      If this bit is set to 0 (by writing a 1), it will stay 0 until the
 *      next interrupt arrives.
 *
 *    DMA ERROR: 1=PIIX/PIIX3 encountered a target abort or master abort while
 *      transferring data on the PCI Bus.
 *
 *      Software set this bit to 0 by writing a 1 to it.
 *    
 *    BMIDEA: Bus Master IDE Active (Read-Only). The PIIX/PIIX3 sets this bit
 *      to 1 when bit 0 on the BMICOM register is set to 1. The PIIX/PIIX3 sets
 *      this bit to 0 when the laster transfer for a region is performed (where
 *      EOT for that region is set in the region descriptor). The PIIX/3 also
 *      sets this bit to 0 when bit 0 of the BMICOM register is set to 0 (FORCE
 *      ABORT) or when the bit 1 of this register is set to 1 (DMA ERROR).
 *
 * Interrupt/Activity Scenarios
 *
 *    IRQ STATUS | BMIDEA | Description
 *    -------------------------------------------------------------------------
 *      0           1       DMA transfer is in progress. No interrupt.
 *
 *      1           0       IDE device generated an interrupt and the physical
 *                          region descriptors exhausted. This indicates normal
 *                          completion where the size of the physical memory
 *                          regions is equal to the IDE device transfer size.
 *
 *      1           1       IDE device generated an interrupt. The controller
 *                          has not reached the end of the end of the physical
 *                          memory regions. This is valid completion when the
 *                          size of the memroy regions is larger than the IDE
 *                          device transfer size.
 *
 *      0           0       Error condition. If the IDE DMA error bit is set to
 *                          1, there is a problem.
 */
#define BMISTA 0x02

/*
 * Bus Master IDE Descriptor Table Pointer Register (BMIDTP)
 *
 * This register provides the base memory address of the descriptor table. The
 * descriptor table must be dword aligned and not cross a 4 KB boundary in
 * memory.
 *
 * 31 - 2                         | 1 - 0
 * Descriptor table base address  | RESERVED
 *
 */
#define BMIDTP 0x04

struct piix_ide_device {
  struct pci_device *pci_d;

  int irq;

  /*
   * Bus Master Interface Base Address
   */
  uint32_t BMIBA;

  list_link(struct piix_ide_device) piix_link;
};

list_typedef(struct piix_ide_device) piix_ide_device_list_t;

int piix_ide_init(void);
int piix_ide_device_init(struct pci_device *pci_d);

#endif /* ! __DEV_IDE_H__ */
