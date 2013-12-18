/**
 * @file dev/pci.c
 *
 * @brief Peripheral Component Interconnect
 */
#include <dev/pci.h>
#include <dev/ide.h>
#include <stdint.h>

#include <debug.h>
#include <errno.h>
#include <kernel.h>
#include <arch/x86/io.h>

struct pci_bus *__pci_root;
pci_device_list_t __pci_devices;

static inline const char *pci_class_code_desc(int class_code) {
  switch (class_code) {
    case 0x00: return "Device was built prior definition of the class code field";
    case 0x01: return "Mass Storage Controller";
    case 0x02: return "Network Controller";
    case 0x03: return "Display Controller";
    case 0x04: return "Multimedia Controller";
    case 0x05: return "Memory Controller";
    case 0x06: return "Bridge Device";
    case 0x07: return "Simple Communication Controllers";
    case 0x08: return "Base System Peripherals";
    case 0x09: return "Input Devices";
    case 0x0A: return "Docking Stations";
    case 0x0B: return "Processors";
    case 0x0C: return "Serial Bus Controllers";
    case 0x0D: return "Wireless Controllers";
    case 0x0E: return "Intelligent I/O Controllers";
    case 0x0F: return "Satellite Communication Controllers";
    case 0x10: return "Encryption/Decryption Controllers";
    case 0x11: return "Data Acquisition and Signal Processing Controllers";
    case 0xFF: return "Device does not fit any defined class.";
    default:   return "Reserved"; 
  }
}

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
  d->class_code = (dword >> 24) & 0xff;
  d->subclass = (dword >> 16) & 0xff;
  d->progif = (dword >> 8) & 0xff;
  d->revision_id = (dword) & 0xff;

  dword = pci_config_read(bus, device, func, 0x0C);
  d->bist = (dword >> 24) & 0xff;
  d->header_type = (dword >> 16) & 0xff;
  d->latency_timer = (dword >> 8) & 0xff;
  d->cache_line_size = (dword) & 0xff;
}

/**
 * @brief Return true if this is a multifunction device.
 */
bool is_multifunc_device(int bus, int device) {
  uint8_t header_type = (pci_config_read(bus, device, 0, 0x0C) >> 16) & 0xff;
  /*
   * bit 7 of the header type is set to 1 if this is a multifunction device
   */
  return (header_type & 0x80) != 0;
}

bool device_exists(int bus, int device, int func) {
  uint16_t vendor_id = pci_config_read(bus, device, func, 0x00) & 0xffff;
  /*
   * 0xffff is not a valid vendor id and indicates that this device does
   * not exist
   */
  return vendor_id != 0xffff;
}

int pci_device_create(struct pci_device **dp, int bus, int device, int func) {
  struct pci_device *d;

  *dp = d = kmalloc(sizeof(struct pci_device));
  if (NULL == d) {
    return ENOMEM;
  }

  list_elem_init(d, global_link);
  list_elem_init(d, bus_link);

  pci_parse_device(d, bus, device, func);

  return 0;
}

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
      if (d->class_code == 0x06 && d->subclass == 0x04) {
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

void __lspci(struct pci_bus *root, int depth) {
  struct pci_device *d;
  struct pci_bus *sb;

  INFO("%*sBus: %d", depth, "", root->bus);
  list_foreach(d, &root->devices, bus_link) {

    INFO("%*s[%d][%d] class=0x%x (%s), subclass=0x%x device-id=0x%x, "
         "vendor-id=0x%x", (depth+1)*2, "",
         d->pci_config_device, d->pci_config_func,
         d->class_code, pci_class_code_desc(d->class_code),
         d->subclass, d->device_id, d->vendor_id);

    list_foreach(sb, &root->buses, bus_link) {
      if (sb->self == d) __lspci(sb, depth + 1);
    }
  }
}

void lspci(void) {
  __lspci(__pci_root, 0);
}

int pci_init(void) {
  struct pci_device *d;
  int ret;

  TRACE();

  if (0 != (ret = ide_init())) {
    ERROR("Unable to initialize ide subsystem: %s", strerr(ret));
    return ret;
  }

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

  list_foreach(d, &__pci_devices, global_link) {
    if (0x1 == d->class_code && 0x1 == d->subclass) {
      if (0 != (ret = ide_device_init(d))) {
        WARN("Failed to initialize IDE device (bus=%d, device=%d, func=%d).",
             d->pci_config_bus, d->pci_config_device, d->pci_config_func);
      }
    }
  }
  
  lspci();

  return 0;
}
