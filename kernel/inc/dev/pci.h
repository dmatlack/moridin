/**
 * @file dev/pci.h
 *
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
 *
 * Reference: http://wiki.osdev.org/PCI
 *            http://tldp.org/LDP/tlk/dd/pci.html
 *            http://www.openwatcom.org/index.php/PCI_Interrupt_Routing
 */
#ifndef __DEV_PCI_H__
#define __DEV_PCI_H__

#include <stdint.h>
#include <list.h>

/*
 * Write to this address to configure/select a device
 */
#define CONFIG_ADDRESS 0xCF8
/*
 * Read from this address to get the result of the configuration.
 */
#define CONFIG_DATA 0xCFC

/*
 * Functions to lookup string descriptions for devices. See pci_table.c.
 */
const char *pci_lookup_vendor(uint16_t vendor_id);
const char *pci_lookup_device(uint16_t vendor_id, uint16_t device_id);
const char *pci_lookup_classcode(uint8_t classcode);
const char *pci_lookup_subclass(uint8_t classcode, uint8_t subclass, uint8_t progif);

struct pci_device;
struct pci_bus;

list_typedef(struct pci_device) pci_device_list_t;
list_typedef(struct pci_bus) pci_bus_list_t;

/**
 * @brief A PCI bus can have multiple devices, and any of those devices can be
 * PCI-PCI bridge, which is an interconnect to another PCI bus. Thus a struct
 * pci_bus is a tree structure.
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
   * PCI configuration space selectors
   */
  int bus;
  int device;
  int func;

  /*
   * The PCI Device Configuration Space
   */
  struct {
    uint16_t vendor_id;
    uint16_t device_id;
    uint16_t command;
    uint16_t status;
    uint8_t  revision_id;
    uint8_t  progif;
    uint8_t  subclass;
    uint8_t  classcode;
    uint8_t  cache_line_size;
    uint8_t  latency_timer;
    uint8_t  header_type;
    uint8_t  bist;
    union {
      struct { /* header_type == 0x00 */
        uint32_t bar0;
        uint32_t bar1;
        uint32_t bar2;
        uint32_t bar3;
        uint32_t bar4;
        uint32_t bar5;
        uint32_t cardbus_cis_pointer;
        uint16_t subsystem_vendor_id;
        uint16_t subsystem_id;
        uint32_t expansion_rom;
        uint8_t  capabilities;
        /* interrupt_line does not affect the device in any way, it just
         * provides a way for BIOS or OS to provide information about the
         * device to the device driver. */
        uint8_t  interrupt_line;
        uint8_t  interrupt_pin;
        uint8_t  min_grant;
        uint8_t  max_latency;
      };
    };
  };

  /*
   * String descriptions of the device
   */
  const char *vendor_desc;
  const char *device_desc;
  const char *classcode_desc;
  const char *subclass_desc;

  /* 
   * list of all devices on the system 
   */
  list_link(struct pci_device) global_link;
  /* 
   * devices on the same bus as this device 
   */
  list_link(struct pci_device) bus_link;

#define PCI_DEVICE_MAX_DRIVERS 32
  struct pci_device_driver *drivers[PCI_DEVICE_MAX_DRIVERS];
  unsigned int num_drivers;
};

/*
 * Offsets into the pci configuration space
 */
#define PCI_VENDOR_ID       0x00
#define PCI_DEVICE_ID       0x02
#define PCI_COMMAND         0x04
#define PCI_STATUS          0x06
#define PCI_REVISION_ID     0x08
#define PCI_PROGIF          0x09
#define PCI_SUBCLASS        0x0A
#define PCI_CLASSCODE       0x0B
#define PCI_CACHE_LINE_SIZE 0x0C
#define PCI_LATENCY_TIMER   0x0D
#define PCI_HEADER_TYPE     0x0E
#define PCI_BIST            0x0F

#define PCI_BAR0                0x10
#define PCI_BAR1                0x14
#define PCI_BAR2                0x18
#define PCI_BAR3                0x1C
#define PCI_BAR4                0x20
#define PCI_BAR5                0x24
#define PCI_CARDBUS_CIS_POINTER 0x28
#define PCI_SUBSYSTEM_VENDOR_ID 0x2C
#define PCI_SUBSYSTEM_ID        0x2E
#define PCI_EXPANSION_ROM       0x30
#define PCI_CAPABILITIES        0x34
#define PCI_INTERRUPT_LINE      0x3C
#define PCI_INTERRUPT_PIN       0x3D
#define PCI_MIN_GRANT           0x3E
#define PCI_MAX_LATENCY         0x3F

uint32_t pci_config_ind(struct pci_device *d, int offset);
uint16_t pci_config_inw(struct pci_device *d, int offset);
uint8_t  pci_config_inb(struct pci_device *d, int offset);

void pci_config_outd(struct pci_device *d, int offset, uint32_t data);
void pci_config_outw(struct pci_device *d, int offset, uint16_t data);
void pci_config_outb(struct pci_device *d, int offset, uint8_t  data);

/*
 * A struct that can identify a set of devices. Used to match device drivers
 * to the devices they want to drive.
 */
struct pci_device_id {
  uint16_t vendor_id;
  uint16_t device_id;
  uint8_t classcode;
  uint8_t subclass;
};

#define PCI_VENDOR_ANY 0xFFFF
#define PCI_DEVICE_ANY 0xFFFF
#define PCI_CLASSCODE_ANY 0xFF
#define PCI_SUBCLASS_ANY 0xFF

#define STRUCT_PCI_DEVICE_ID(vid, did, cc, sc)\
  {\
    .vendor_id = vid,\
    .device_id = did,\
    .classcode = cc,\
    .subclass = sc,\
  }

list_typedef(struct pci_device_driver) pci_device_driver_list_t;

/**
 * @brief A pci_device_driver is a set of callbacks that will be invoked on
 * all devices that the driver requests to handle.
 */
struct pci_device_driver {
  const char *name;

  /*
   * The pci_device_id identifies the set of devices this driver will be
   * matched to
   */
  struct pci_device_id id;

  /**
   * @brief Called once on the device driver, to allow it to do whatever
   * initializations it may need.
   */
  int (*init)(void);
  /**
   * @brief Called with every device that matches this driver's pci_device_id
   * struct.
   */
  int (*new_device)(struct pci_device *pci_d);

  list_link(struct pci_device_driver) pci_link;
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

/*
 * A list of all pci device drivers registered in the system.
 */
extern pci_device_driver_list_t __pci_drivers;

int pci_init(void);
int pci_scan_bus(struct pci_bus *b);
void pci_register_driver(struct pci_device_driver *driver);

#endif /* !__DEV_PCI_H__ */
