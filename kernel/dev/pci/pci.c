/**
 * @file dev/pci.c
 *
 * @brief Peripheral Component Interconnect
 */
#include <dev/pci.h>
#include <string.h>
#include <stdint.h>
#include <debug.h>
#include <errno.h>
#include <kernel.h>
#include <arch/x86/io.h>
#include <types.h>

struct pci_bus *__pci_root;

pci_device_list_t __pci_devices;
pci_device_driver_list_t __pci_drivers;

extern struct pci_device_driver __ide_pci_driver;

/**
 * @brief Configure a PCI device.
 *
 * The CONFIG_ADDRESS register has the following structure:
 *
 * Bit   :  31      | 30-24    | 23-16 | 15-11  | 10-8     | 7-2      | 1-0
 * Value :  enable  | reserved | bus   | device | function | register | 00
 *
 * @param bus the bus number
 * @param device the device number
 * @param func the function number
 * @param offset the offset into the 256 byte configuration space to read. the
 * offset should be a multiple of 0x4 or you will get overlapping words.
 */
uint32_t pci_config_read(int bus, int device, int func, int offset) {
  uint32_t address;
  uint32_t data;

  address = (1 << 31)      |
            (bus << 16)    |
            (device << 11) |
            (func << 8)    |
            (offset & 0xfc);

  outd(CONFIG_ADDRESS, address);
  data = ind(CONFIG_DATA);

  return data;
}

bool is_multifunc_device(int bus, int device) {
  uint8_t header_type = (pci_config_read(bus, device, 0, 0x0C) >> 16) & 0xff;
  return (header_type & 0x80) != 0;
}

bool device_exists(int bus, int device, int func) {
  uint16_t vendor_id = pci_config_read(bus, device, func, 0x00) & 0xffff;
  return vendor_id != 0xffff;
}

void pci_parse_device(struct pci_device *d, int bus, int device, int func) {
  uint32_t dword;

  d->pci_config_bus = bus;
  d->pci_config_device = device;
  d->pci_config_func = func;

  dword = pci_config_read(bus, device, func, 0x00);
  d->device_id = (dword >> 16) & 0xffff;
  d->vendor_id = (dword) & 0xffff;

  dword = pci_config_read(bus, device, func, 0x04);
  d->status = (dword >> 16) & 0xffff;
  d->command = (dword) & 0xffff;

  dword = pci_config_read(bus, device, func, 0x08);
  d->classcode = (dword >> 24) & 0xff;
  d->subclass = (dword >> 16) & 0xff;
  d->progif = (dword >> 8) & 0xff;
  d->revision_id = (dword) & 0xff;

  dword = pci_config_read(bus, device, func, 0x0C);
  d->bist = (dword >> 24) & 0xff;
  d->header_type = (dword >> 16) & 0xff;
  d->latency_timer = (dword >> 8) & 0xff;
  d->cache_line_size = (dword) & 0xff;

  if (d->header_type == 0x00) {
    INFO("header_type==0x00 for device %p", d);
    dword = pci_config_read(bus, device, func, 0x10);
    d->bar0 = dword;
    dword = pci_config_read(bus, device, func, 0x14);
    d->bar1 = dword;
    dword = pci_config_read(bus, device, func, 0x18);
    d->bar2 = dword;
    dword = pci_config_read(bus, device, func, 0x1C);
    d->bar3 = dword;
    dword = pci_config_read(bus, device, func, 0x20);
    d->bar4 = dword;
    dword = pci_config_read(bus, device, func, 0x24);
    d->bar5 = dword;

    dword = pci_config_read(bus, device, func, 0x28);
    d->cardbus_cis_pointer = dword;

    dword = pci_config_read(bus, device, func, 0x2C);
    d->subsystem_id = (dword >> 16) & 0xffff;
    d->subsystem_vendor_id = dword & 0xffff;

    dword = pci_config_read(bus, device, func, 0x30);
    d->expansion_rom = dword;

    dword = pci_config_read(bus, device, func, 0x34);
    d->capabilities = dword & 0xff;

    dword = pci_config_read(bus, device, func, 0x3C);
    d->max_latency = (dword >> 24) & 0xff;
    d->min_grant = (dword >> 16) & 0xff;
    d->interrupt_pin = (dword >> 8) & 0xff;
    d->interrupt_line = (dword) & 0xff;
  }

  d->vendor_desc = pci_lookup_vendor(d->vendor_id);
  d->device_desc = pci_lookup_device(d->vendor_id, d->device_id);
  d->classcode_desc = pci_lookup_classcode(d->classcode);
  d->subclass_desc = pci_lookup_subclass(d->classcode, d->subclass, d->progif);
}


/**
 * @brief Create a pci_device struct from the device identified from the tuple
 * (bus, device, func).
 */
int pci_device_create(struct pci_device **dp, int bus, int device, int func) {
  struct pci_device *d;

  *dp = d = kmalloc(sizeof(struct pci_device));
  if (NULL == d) {
    return ENOMEM;
  }
  memset(d, 0, sizeof(struct pci_device));

  list_elem_init(d, global_link);
  list_elem_init(d, bus_link);
  
  d->num_drivers = 0;

  pci_parse_device(d, bus, device, func);

  return 0;
}

/**
 * @brief Scan the provided bus for devices, adding those devices to the global
 * list of pci devices as well as the bus' list of devices. If a PCI-PCI bridge
 * device is found, recursively scan the downstream bus.
 */
int pci_scan_bus(struct pci_bus *b) {
  int ret, device, func;

  for (device = 0; device < 32; device++) {
    for (func = 0; func < 8; func++) {
      struct pci_device *d;

      if (!device_exists(b->bus, device, func)) continue;
      
      if (0 != (ret = pci_device_create(&d, b->bus, device, func))) {
        return ret;
      }

      list_insert_tail(&__pci_devices, d, global_link);
      list_insert_tail(&b->devices, d, bus_link);

      /*
       * If this device is a PCI-PCI bridge, then search the downstream bus
       */
      if (d->classcode == 0x06 && d->subclass == 0x04) {
        struct pci_bus *sb;
       
        sb = kmalloc(sizeof(struct pci_bus));
        if (NULL == b) {
          return ENOMEM;
        }

        list_init(&sb->devices);
        list_init(&sb->buses);
        list_elem_init(sb, bus_link);
        sb->bus = (pci_config_read(b->bus, device, 0, 0x18) >> 8) & 0xff;
        sb->self = d;
    
        INFO("PCI-PCI Bridge found. Secondary Bus: %d", sb->bus);

        pci_scan_bus(sb);
      }

      if (func == 0 && !is_multifunc_device(b->bus, device)) break;
    }
  }

  return 0;
}

void pci_print_device(struct pci_device *d) {
  INFO("PCI DEVICE (%p)\n"
    "vendor id:           0x%04x %s\n"
    "device id:           0x%04x %s\n"
    "classcode:           0x%02x   %s\n"
    "subclass:            0x%02x   %s\n"
    "status:              0x%04x\n"
    "command:             0x%04x\n"
    "progif:              0x%02x\n"
    "revision id:         0x%02x\n"
    "bist:                0x%02x\n"
    "header type:         0x%02x\n"
    "latency timer:       0x%02x\n"
    "cache line size:     0x%02x\n"
    "\n"
    "bar0:                0x%08x\n"
    "bar1:                0x%08x\n"
    "bar2:                0x%08x\n"
    "bar3:                0x%08x\n"
    "bar4:                0x%08x\n"
    "bar5:                0x%08x\n"
    "cardbus cis ptr:     0x%08x\n"
    "subsystem_id:        0x%04x\n"
    "subsystem_vendor_id: 0x%04x\n"
    "expansion_rom:       0x%08x\n"
    "capabilities:        0x%02x\n"
    "max_latency:         0x%02x\n"
    "min_grant:           0x%02x\n"
    "interrupt_pin:       0x%02x\n"
    "interrupt_line:      0x%02x\n"
    "\n",
    d,
    d->vendor_id, d->vendor_desc,
    d->device_id, d->device_desc,
    d->classcode, d->classcode_desc,
    d->subclass, d->subclass_desc,
    d->status,
    d->command,
    d->progif,
    d->revision_id,
    d->bist,
    d->header_type,
    d->latency_timer,
    d->cache_line_size,
    d->bar0,
    d->bar1,
    d->bar2,
    d->bar3,
    d->bar4,
    d->bar5,
    d->cardbus_cis_pointer,
    d->subsystem_id,
    d->subsystem_vendor_id,
    d->expansion_rom,
    d->capabilities,
    d->max_latency,
    d->min_grant,
    d->interrupt_pin,
    d->interrupt_line
  );
}

void __lspci(struct pci_bus *root, int depth) {
  struct pci_device *d;
  struct pci_bus *sb;

  INFO("%*sBus: %d", depth, "", root->bus);
  list_foreach(d, &root->devices, bus_link) {

    pci_print_device(d);
    kprintf("%04x:%04x:%02x.%02x %s, %s, %s, %s\n", 
        d->vendor_id, d->device_id, d->classcode, d->subclass,
        d->vendor_desc, d->device_desc, d->classcode_desc, d->subclass_desc);

    list_foreach(sb, &root->buses, bus_link) {
      if (sb->self == d) __lspci(sb, depth + 1);
    }
  }
}

void lspci(void) {
  __lspci(__pci_root, 0);
}

/**
 * @brief Return true if the given device <d> matches the pci_device_id
 * struct <id>.
 */
bool pci_device_match(struct pci_device_id *id, struct pci_device *d) {
  if (id->vendor_id != d->vendor_id && id->vendor_id != PCI_VENDOR_ANY)
    return false;
  if (id->device_id != d->device_id && id->device_id != PCI_DEVICE_ANY)
    return false;
  if (id->classcode != d->classcode && id->classcode != PCI_CLASSCODE_ANY)
    return false;
  if (id->subclass != d->subclass && id->subclass != PCI_SUBCLASS_ANY)
    return false;
  return true;
}

void pci_device_add_driver(struct pci_device *d, 
                           struct pci_device_driver *driver) {
  int ret;

  if (d->num_drivers == PCI_DEVICE_MAX_DRIVERS) {
    WARN("Attempted to register more than %d drivers for device "
         "%04x:%04x:%02x.%02x", 
         PCI_DEVICE_MAX_DRIVERS, d->vendor_id, d->device_id, d->classcode,
         d->subclass);
    return;
  }

  if (0 != (ret = driver->new_device(d))) {
    WARN("Failed to add device %04x:%04x:%02x.%02x to driver %s: %s",
         d->vendor_id, d->device_id, d->classcode, d->subclass,
         driver->name, strerr(ret));
    return;
  }

  d->drivers[d->num_drivers++] = driver;
}

/**
 * @brief Initialize the PCI subsystem. This initialization will build
 * the PCI bus tree, and call driver initializers on all devices that have
 * matching drivers.
 */
int pci_init(void) {
  int ret;

  TRACE();

  list_init(&__pci_devices);
  list_init(&__pci_devices);

  __pci_root = kmalloc(sizeof(struct pci_bus));
  if (NULL == __pci_root) {
    return ENOMEM;
  }
  list_init(&__pci_root->devices);
  list_init(&__pci_root->buses);
  list_elem_init(__pci_root, bus_link);
  __pci_root->bus = 0;
  __pci_root->self = NULL;

  /*
   * construct the pci device tree
   */
  if (0 != (ret = pci_scan_bus(__pci_root))) {
    return ret;
  }

  lspci();

  /*
   * register the default drivers
   */
  pci_register_driver(&__ide_pci_driver);
  //TODO add more drivers ...

  return 0;
}

void pci_register_driver(struct pci_device_driver *driver) {
  struct pci_device *d;
  int ret;

  if (0 != (ret = driver->init())) {
    WARN("Failed to initalize the %s driver system: %s", driver->name,
         strerr(ret));
    return;
  }

  list_elem_init(driver, pci_link);
  list_insert_tail(&__pci_drivers, driver, pci_link);

  /*
   * invoke the driver on each matching device
   */
  list_foreach(d, &__pci_devices, global_link) {
    if (pci_device_match(&driver->id, d)) {
      pci_device_add_driver(d, driver);
    }
  }
}
