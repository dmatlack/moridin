/**
 * @file dev/pci.h
 *
 * Reference: http://wiki.osdev.org/PCI
 */
#ifndef __DEV_PCI_H__
#define __DEV_PCI_H__

#include <stdint.h>

/************
 * PCI: Peripheral Component Interconnect
 *
 * Configuration Space
 * ----------------------------------------------------------------------------
 * The PCI spec allows a totally software driven initialization of each device
 * via a separate "Configuration Address Space". All PCI devices, are required
 * to provide 256 bytes of configuration registers for this purpose.
 * 
 * Configuration read/write cycles are used to access the Configuration Space
 * of each target (device). A target is selected via IDSEL, which acts as a 
 * "chip select" signal.
 */

/*
 * Write to this address to configure/select a device
 */
#define CONFIG_ADDRESS 0xCF8
/*
 * Read from this address to get the result of the configuration.
 */
#define CONFIG_DATA 0xCFC

struct pci_device {
  /* identifies a device, where valid IDs are allocated by the vendor */
  int device_id;

  /* identifies a manufacturer. vendor ids are allocated by PCI-SIG and
   * 0xFFFF indicates a non-existent device */
  int vendor_id;

  /* a register used to record status information */
  int status;

  /* provides control of a device's ability to generate and respond to
   * pci cycles. the only functionality guarenteed to be supported by 
   * all devices is to diconnect from the pci bus for all accesses except
   * the configuration space access. command = 0 for this function. */
  int command;

  /* a read-only register that specifies the type of function the device
   * supports */
  int class_code;

  /* a read-only register that specifies the _specific_ function the device
   * performs */
  int subclass;

  /* A read-only register that specifies a register level programming 
   * interface the device has, if it has any at all */
  int progif;

  /* specifies a revision identifies (vendor allocated) */
  int revision_id;

  /* represents the status and allows control of a device's "built-in self
   * test" */
  int bist;

  /* Identifies the layout of the reast of the header beginning at byte 0x10
   * of the header and also specifies whether or not the device has multiple
   * functions (bit 7). */
  int header_type;

  /* specifies the latency timer in units of pci bus clocks */
  int latency_timer;

  /* specifies the system cache line size in 32-bit units (DWORD) */
  int cache_line_size;
};

int pci_init(void);

#endif /* !__DEV_PCI_H__ */
