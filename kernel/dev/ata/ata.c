/**
 * @file dev/ata/ata.c
 *
 * @brief Advanced Technology Attachment (ATA)
 *
 * @author David Matlack
 */
#include <dev/ata.h>

#include <kernel.h>
#include <errno.h>
#include <debug.h>
#include <string.h>

#include <arch/x86/io.h>

void ata_print_drive(struct ata_drive *drive) {
  if (!drive->exists) {
    INFO("ATA %s Drive: does not exist ",
         drive->select == ATA_SELECT_MASTER ? "Master" : "Slave");
  }
  else if (!drive->usable) {
    INFO("ATA %s Drive: unusable because of type %s", 
         drive->select == ATA_SELECT_MASTER ? "Master" : "Slave",
         drive_type_string(drive->type));
  }
  else {
    INFO("ATA %s Drive: %s",
         drive->select == ATA_SELECT_MASTER ? "Master" : "Slave",
         drive_type_string(drive->type));
  }

#ifdef KDEBUG
  if (drive->exists && drive->usable) {
    DEBUG("    Serial Number:      %s", drive->serial);
    DEBUG("    Firmware Version:   %s", drive->firmware);
    DEBUG("    Model Number:       %s", drive->model);
    DEBUG("    sectors:            %d", drive->sectors);
    DEBUG("    sectors / block:    %d", drive->sectors_per_block);
    DEBUG("    Supported DMA Mode: %d", drive->supported_dma_mode);
    DEBUG("    Selected DMA Mode:  %d", drive->dma_mode);
    DEBUG("    Supported PIO Mode: %d", drive->supported_pio_mode);
    DEBUG("    Major Version:      0x%04x", drive->major_version);
    DEBUG("    Minor Version:      0x%04x", drive->minor_version);
    DEBUG("    DMA Min Cylce Time: %d ns", drive->dma_min_nano);
    DEBUG("    DMA Cylce Time:     %d ns", drive->dma_nano);
  }
#endif
}

const char *ata_strerr(uint8_t error) {
  if (error & (1 << 2)) return "ABORT";
  else return "unknown";
}

/**
 * @brief Return true if this driver supports a given type of device. We
 * can use this to make sure we don't try to send commands and mess up
 * any unsopported devices.
 */
bool ata_is_supported(struct ata_drive *d) {
  switch (d->type) {
    case ATA_PATA: return true;
    default: return false;
  }
}

enum ata_drive_type get_drive_type(struct ata_signature *s) {
  if (s->lba_mid == 0x14 && s->lba_high == 0xEB) return ATA_PATAPI;
  if (s->lba_mid == 0x69 && s->lba_high == 0x96) return ATA_SATAPI;
  if (s->lba_mid == 0x00 && s->lba_high == 0x00) return ATA_PATA;
  if (s->lba_mid == 0x3c && s->lba_high == 0xc3) return ATA_SATA;
  return ATA_UNKNOWN;
}

bool does_ata_bus_exist(int cmd) {
  return inb(cmd + ATA_CMD_STATUS) != 0xff;
}

/**
 * @brief Select the provided ATA drive.
 */
void ata_select_drive(struct ata_drive *d) {
  outb(d->bus->cmd + ATA_CMD_DEVICE, d->select);
}

/**
 * @brief Send a command to an ATA drive.
 */
void ata_command(struct ata_drive *d, uint8_t command) {
  int i;

  ata_select_drive(d);

  outb(d->bus->cmd + ATA_CMD_COMMAND, command);

  for (i = 0; i < 4; i++) iodelay(); // spin for 400ns
}

#define RETURN_ERROR_IF_TIMEOUT(_count, _timeout, _culprit_string) \
  do { \
    DEBUG("  %s poll: %d cycles", _culprit_string, _count); \
    if ((_count) == (_timeout)) { \
      WARN("ATA drive timed out waiting for %s!", _culprit_string); \
      return ETIMEDOUT; \
    } \
  } while (0)

/**
 * @brief Issue the ATA IDENTIFY command to a drive in order to get some
 * information about it.
 *
 * For more information about WTF the IDENTIFY command returns, see section
 * 8.15 of http://www.t13.org/documents/UploadedDocuments/project/d1410r3b-ATA-ATAPI-6.pdf
 *
 * @return 
 *    0 if the drive exists AND reports data back via IDENTIFY
 *
 *    ENODEV if the drive does not exist
 *    ETIMEDOUT if the drive takes too long to respond to the IDENTIFY command
 *    EGENERIC if the drive responds with an error to the IDENTIFY command
 */
int ata_identify(struct ata_drive *drive, uint16_t data[256]) {
  uint8_t idstatus;
  const int timeout = 0x100000;
  int i;

  TRACE("drive=%p", drive);

  ata_select_drive(drive);

  outb(drive->bus->cmd + ATA_CMD_SECTOR_COUNT, 0);
  outb(drive->bus->cmd + ATA_CMD_LBA_LOW,      0);
  outb(drive->bus->cmd + ATA_CMD_LBA_MID,      0);
  outb(drive->bus->cmd + ATA_CMD_LBA_HIGH,     0);
  outb(drive->bus->cmd + ATA_CMD_DEVICE,       0);

  /*
   * IDENTIFY yourself, drive!
   */
  ata_command(drive, ATA_IDENTIFY);

  idstatus = inb(drive->bus->cmd + ATA_CMD_STATUS);
  if (0 == idstatus) return ENODEV;

  /*
   * The drive exists! Poll the BSY bit until it is cleared.
   */
  for (i = 0; i < timeout; i++) {
    idstatus = inb(drive->bus->cmd + ATA_CMD_STATUS);
    if (idstatus & ATA_ERR) break;
    if (!(idstatus & ATA_BSY)) break;
  }
  RETURN_ERROR_IF_TIMEOUT(i, timeout, "ATA_BSY");

  /*
   * Get the signature of the drive
   */
  drive->sig.sector_count = inb(drive->bus->cmd + ATA_CMD_SECTOR_COUNT);
  drive->sig.lba_low      = inb(drive->bus->cmd + ATA_CMD_LBA_LOW);
  drive->sig.lba_mid      = inb(drive->bus->cmd + ATA_CMD_LBA_MID);
  drive->sig.lba_high     = inb(drive->bus->cmd + ATA_CMD_LBA_HIGH);
  drive->sig.device       = inb(drive->bus->cmd + ATA_CMD_DEVICE);

  drive->type = get_drive_type(&drive->sig);

  /*
   * Continue polling until DRQ (ready to transfer data) is set or
   * ERR (error) is set.
   */
  for (i = 0; i < timeout; i++) {
    idstatus = inb(drive->bus->cmd + ATA_CMD_STATUS);
    if ((idstatus & ATA_DRQ) || (idstatus & ATA_ERR)) break;
  }
  RETURN_ERROR_IF_TIMEOUT(i, timeout, "ATA_DRQ");
  
  /*
   * If an error occurred, it's probably because this is an ATAPI device, and
   * it wants us to send it IDENTIFY PACKET DEVICE command intead. For now we
   * will ignore ATAPI devices and not deal with them.
   */
  if (idstatus & ATA_ERR) {
    uint8_t error;
    error = inb(drive->bus->cmd + ATA_CMD_ERROR);
    DEBUG("Error occured while waiting for ATA_DRQ after IDENTIFY: 0x%02x (%s)",
          error, ata_strerr(error));
    return EGENERIC;
  }

  /*
   * If all succeeded, we read in the 256 byte IDENTIFY data
   */
  for (i = 0; i < 256; i++) {
    data[i] = inw(drive->bus->cmd + ATA_CMD_DATA);
  }
  return 0;
}

/**
 * @brief Allocate and initialize a struct ata_drive.
 */
int ata_drive_create(struct ata_drive **drivep) {
  struct ata_drive *drive;

  *drivep = drive = kmalloc(sizeof(struct ata_drive));
  if (NULL == drive) {
    return ENOMEM;
  }

  memset(drive, 0, sizeof(struct ata_drive));

  drive->bus = NULL;
  drive->type = ATA_UNKNOWN;
  drive->exists = false;

  return 0;
}

/**
 * @brief Disable IRQs of a specific drive.
 */
void ata_disable_irqs(struct ata_drive *d) {
  ata_select_drive(d);
  outb(d->bus->ctl + ATA_CTL_ALT_STATUS, ATA_NIEN);
}

/**
 * @brief Copy a "string" (series of bytes) from the result of the IDENTIFY
 * DEVICE command (256 words / 512 bytes).
 *
 * This is useful because the result of IDENTIFY DEVICE contains strings
 * such as the firmware version and serial number of the device.
 */
static void ata_read_identify_string(uint16_t *data, char *buffer,
                                     unsigned offset, unsigned length) {
  uint16_t word;
  unsigned i;

  /*
   * WARNING: this function is a little weird because we are copying from an
   * array of words into an array of bytes. i indexes WORDS not BYTES here!
   */
  for (i = 0; i < length; i++) {
    word = data[offset + i];

    buffer[2*i + 0] = (word >> 8) & 0xff;
    buffer[2*i + 1] = (word >> 0) & 0xff;
  }

  buffer[length] = (char) 0;
}

/**
 * @brief Create a new ata_drive struct and add it to the provided ata_bus
 * struct.
 *
 * @return 
 *    ENOMEM if the kernel ran out of memory
 *    0 otherwise
 */
int ata_bus_add_drive(struct ata_bus *bus, uint8_t drive_select) {
  struct ata_drive *drive;
  uint16_t data[256];
  int ret, identify_ret;

  if (0 != (ret = ata_drive_create(&drive))) {
    return ret;
  }

  drive->select = drive_select;
  drive->bus = bus;

  if (ATA_SELECT_MASTER == drive_select) {
    bus->master = drive;
  }
  else {
    ASSERT_EQUALS(ATA_SELECT_SLAVE, drive_select);
    bus->slave = drive;
  }

  /*
   * We will be using polling at first, so disable IRQs for now
   */
  ata_disable_irqs(drive);

  /*
   * Execute the IDENTIFY DEVICE command to figure out if the drive exists,
   * and if we can support it
   */
  identify_ret = ata_identify(drive, data);

  drive->exists = (ENODEV != identify_ret);
  drive->usable = (0 == identify_ret);

  if (drive->exists && !ata_is_supported(drive)) {
    WARN("ATA device type not supported: %s (bus cmd: 0x%03x, drive select: "
         "0x%02x)", drive_type_string(drive->type), drive->bus->cmd,
          drive->select);
  }

  /*
   * Check the results of IDENTIFY DEVICE. See section 8.15 of the ATA-6 spec.
   */
  if (0 == identify_ret) {
    /* 
     * drives conforming to the ATA standard will clear bit 15
     */
    if (data[0] & (1 << 15)) drive->usable = false;
    /* 
     * if bit 2 is set, the IDENTIFY response is incomplete
     */
    if (data[0] & (1 << 2)) drive->usable = false;

    ata_read_identify_string(data, drive->serial,
                             ATA_IDENTIFY_SERIAL_WORD_OFFSET,
                             ATA_IDENTIFY_SERIAL_WORD_LENGTH);
    ata_read_identify_string(data, drive->firmware,
                             ATA_IDENTIFY_FIRMWARE_WORD_OFFSET,
                             ATA_IDENTIFY_FIRMWARE_WORD_LENGTH);
    ata_read_identify_string(data, drive->model,
                             ATA_IDENTIFY_MODEL_WORD_OFFSET,
                             ATA_IDENTIFY_MODEL_WORD_LENGTH);

    drive->sectors_per_block = data[47] & 0xff;

    /*
     * For 32-bit values, ATA transfers the lower order 16-bits first, then the
     * higher order 16-bits.
     */
    drive->sectors = data[61] << 16 | data[60];

    /*
     * Check if the drive supports PIO, DMA
     */
    drive->dma_nano = -1;
    drive->dma_min_nano = -1;
    drive->dma_mode = ATA_DMA_NOT_SUPPORTED;
    drive->supported_dma_mode = ATA_DMA_NOT_SUPPORTED;
    drive->supported_pio_mode = ATA_PIO_NOT_SUPPORTED;
    if ((data[53] & (1 << 1))) {
      if (data[63] & 1)         drive->supported_dma_mode = 0;
      if (data[63] & (1 << 1))  drive->supported_dma_mode = 1;
      if (data[63] & (1 << 2))  drive->supported_dma_mode = 2;
      if (data[63] & (1 << 8))  drive->dma_mode = 0;
      if (data[63] & (1 << 9))  drive->dma_mode = 1;
      if (data[63] & (1 << 10)) drive->dma_mode = 2;

      if (data[64] & 1)         drive->supported_pio_mode = 3;
      if (data[64] & (1 << 1))  drive->supported_pio_mode = 4;

      drive->dma_min_nano = data[65];
      drive->dma_nano = data[66];
    }

    drive->major_version = data[80];
    drive->minor_version = data[81];
  }

  return 0;
}

/**
 * @brief Free all memory held by an ata_drive.
 */
void ata_free_drive(struct ata_drive *d) {
  kfree(d, sizeof(struct ata_drive));
}

/**
 * @brief Probe an ATA bus for devices.
 *
 * This function is intended to be called by the IDE code. The IDE code knows
 * where the ATA I/O ports are (<cmd> and <ctl>). The IDE is 
 * expected to pass these values in for each ATA bus it knows about (2 in the
 * case of PIIX), and this function will probe the bus for devices.
 *
 * @return
 *    ENOMEM if there is not enough memory
 *
 *    0 success: otherwise (even if there is issues with the drives, we will
 *      still return success. the caller is expected to look at the ata_drive's
 *      and make sure they're all ok).
 */
int ata_new_bus(struct ata_bus *bus, int irq, int cmd, int ctl) {
  int ret;

  TRACE("bus=%p, cmd=0x%03x, ctl=0x%03x", bus, cmd, ctl);

  memset(bus, 0, sizeof(struct ata_bus));

  if (!does_ata_bus_exist(cmd)) {
    bus->exists = false;
    return 0;
  }

  bus->exists = true;
  bus->cmd = cmd;
  bus->ctl = ctl;
  bus->irq = irq;
  bus->master = NULL;
  bus->slave = NULL;

  if (0 != (ret = ata_bus_add_drive(bus, ATA_SELECT_MASTER))) {
    goto ata_new_bus_cleanup;
  }
  if (0 != (ret = ata_bus_add_drive(bus, ATA_SELECT_SLAVE))) {
    goto ata_new_bus_cleanup;
  }

  ata_print_drive(bus->master);
  ata_print_drive(bus->slave);

  return 0;

ata_new_bus_cleanup:
  if (bus->master) ata_free_drive(bus->master);
  if (bus->slave) ata_free_drive(bus->slave);
  return ret;
}
