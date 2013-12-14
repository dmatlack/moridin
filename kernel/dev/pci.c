/**
 * @file dev/pci.c
 *
 * @brief Peripheral Component Interconnect
 */
#include <dev/pci.h>
#include <stdint.h>

#include <debug.h>
#include <kernel.h>
#include <arch/x86/io.h>

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

void pci_check_device(int bus, int device) {
  struct pci_device d;
  int func;
  int num_funcs = is_multifunc_device(bus, device) ? 8 : 1;

  for (func = 0; func < num_funcs; func++) {

    pci_parse_device(&d, bus, device, func);

    if (d.vendor_id == 0xffff) {
      // device does not exist
      return;
    }

    INFO("pci vendor=0x%x device=0x%x header_type=0x%x, class_code=0x%x (%s), subclass=0x%x",
        d.vendor_id, d.device_id, d.header_type, d.class_code, pci_class_code_desc(d.class_code), d.subclass);

    /*
     * If this device is a PCI-PCI bridge, then search the downstream bus
     */
    if (d.class_code == 0x06 && d.subclass == 0x04) {
      int secondary_bus = (pci_config_read(bus, device, 0, 0x18) >> 8) & 0xff;

      INFO("PCI-PCI Bridge found. Secondary Bus: %d", secondary_bus);
      pci_scan_bus(secondary_bus);
    }

  }
}

void pci_scan_bus(int bus) {
  int device;

  TRACE("bus=%d", bus);

  for (device = 0; device < 32; device++) {
    pci_check_device(bus, device);
  }
}

int pci_init(void) {

  pci_scan_bus(0);

  return 0;
}
