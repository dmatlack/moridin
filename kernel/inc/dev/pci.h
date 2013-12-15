/**
 * @file dev/pci.h
 *
 * Reference: http://wiki.osdev.org/PCI
 *            http://tldp.org/LDP/tlk/dd/pci.html
 */
#ifndef __DEV_PCI_H__
#define __DEV_PCI_H__

#include <stdint.h>
#include <list.h>

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

struct pci_device;
struct pci_bus;

list_typedef(struct pci_device) pci_device_list_t;
list_typedef(struct pci_bus) pci_bus_list_t;

/*
 * A PCI bus can have multiple devices, and any of those devices can be
 * PCI-PCI bridge, which is an interconnect to another PCI bus. Thus a
 * struct pci_bus is a tree structure.
 */
struct pci_bus {
  /*
   * A pointer to the pci_device struct for this bus. The device will
   * be a PCI-PCI bridge device, of course.
   */
  struct pci_device *self;

  /*
   * The bus number of this bus.
   */
  int bus;

  /*
   * A list of devices connected to this bus.
   */
  pci_device_list_t devices;

  /*
   * The PCI-PCI bridges connected to this bus.
   */
  pci_bus_list_t buses;
  list_link(struct pci_bus) bus_link;
};

struct pci_device {
  /* 
   * identifies a device, where valid IDs are allocated by the vendor 
   */
  int device_id;

  /* 
   * identifies a manufacturer. vendor ids are allocated by PCI-SIG and
   * 0xFFFF indicates a non-existent device 
   */
  int vendor_id;

  /* 
   * a register used to record status information 
   */
  int status;

  /* 
   * provides control of a device's ability to generate and respond to
   * pci cycles. the only functionality guarenteed to be supported by 
   * all devices is to diconnect from the pci bus for all accesses except
   * the configuration space access. command = 0 for this function. 
   */
  int command;

  /* 
   * a read-only register that specifies the type of function the device
   * supports 
   */
  int class_code;

  /* 
   * a read-only register that specifies the _specific_ function the device
   * performs 
   */
  int subclass;

  /* 
   * A read-only register that specifies a register level programming 
   * interface the device has, if it has any at all 
   */
  int progif;

  /* 
   * specifies a revision identifies (vendor allocated) 
   */
  int revision_id;

  /* 
   * represents the status and allows control of a device's "built-in self
   * test" 
   */
  int bist;

  /* 
   * Identifies the layout of the reast of the header beginning at byte 0x10
   * of the header and also specifies whether or not the device has multiple
   * functions (bit 7). 
   */
  int header_type;

  /* 
   * specifies the latency timer in units of pci bus clocks 
   */
  int latency_timer;

  /* 
   * specifies the system cache line size in 32-bit units (DWORD) 
   */
  int cache_line_size;


  /* 
   * list of all devices on the system 
   */
  list_link(struct pci_device) global_link;
  /* 
   * devices on the same bus as this device 
   */
  list_link(struct pci_device) bus_link;
};

/*
 * The root of the pci device list tree.
 */
extern struct pci_bus *__pci_root;

/*
 * A list of all pci devices on the system. This is just a flat version
 * of the pci_bus tree.
 */
extern pci_device_list_t __pci_devices;

int pci_scan_bus(struct pci_bus *b);
int pci_init(void);

#endif /* !__DEV_PCI_H__ */
