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

void pci_check_device(int bus, int device) {

  struct pci_device d;

  pci_parse_device(&d, bus, device, 0);

  if (d.vendor_id == 0xffff) {
    // device does not exist
    return;
  }

  INFO("pci vendor=0x%x device=0x%x", d.vendor_id, d.device_id);
}

int pci_init(void) {
  int bus, device;

  for (bus = 0; bus < 32; bus++) {
    for (device = 0; device < 256; device++) {
      pci_check_device(bus, device);
    }
  }

  return 0;
}
