/**
 * @file dev/ata.h
 *
 * @brief Advanced Technology Attachement 
 *
 * ATA-6 Spec:
 *   http://www.t13.org/documents/UploadedDocuments/project/d1410r3b-ATA-ATAPI-6.pdf
 *
 * Some useful definitions (and their sections) copied from the above spec:
 *
 * 3.1.14 CHS (cylinder-head-sector): This term defines an obsolete method of
 *        addressing the data on the device by cylinder number, head number,
 *        and sector number.
 *
 * 3.1.25 DMA (direct memory access) data transfer: A means of data transfer
 *        between device and host memory without host processor intervention.
 *
 * 3.1.30 LBA (logical block address): This term defines the addressing of data
 *        on the device by the linear mapping of sectors.
 *
 * 3.1.35 PIO (programmed input/output) data transfer: PIO data transfers are
 *        performed by the host processor utilizing accesses to the Data
 *        register.
 *
 * 3.1.41 sector: A uniquely addressable set of 256 words (512 bytes).
 *
 * @author David Matlack
 */
#ifndef __DEV_ATA_H__
#define __DEV_ATA_H__

#include <list.h>
#include <types.h>
#include <stdint.h>

#define ATA_BYTES_PER_SECTOR 512

/*
 * ATA COMMAND BLOCK
 */
#define ATA_CMD_DATA            0x00
#define ATA_CMD_ERROR           0x01
#define ATA_CMD_FEATURES        0x01
#define ATA_CMD_SECTOR_COUNT    0x02
#define ATA_CMD_LBA_LOW         0x03
#define ATA_CMD_LBA_MID         0x04
#define ATA_CMD_LBA_HIGH        0x05
#define ATA_CMD_HEAD            0x06
#define ATA_CMD_DEVICE          0x06
#define     ATA_SELECT_MASTER   0xA0
#define     ATA_SELECT_SLAVE    0xB0
#define ATA_CMD_STATUS          0x07
#define     ATA_ERR             (1 << 0)
#define     ATA_DRQ             (1 << 3)
#define     ATA_SRV             (1 << 4)
#define     ATA_DF              (1 << 5)
#define     ATA_RDY             (1 << 6)
#define     ATA_BSY             (1 << 7)
#define ATA_CMD_COMMAND         0x07
#define     ATA_READ_PIO        0x20
#define     ATA_READ_PIO_EXT    0x24
#define     ATA_READ_DMA        0xC8
#define     ATA_READ_DMA_EXT    0x25
#define     ATA_WRITE_PIO       0x30
#define     ATA_WRITE_PIO_EXT   0x34
#define     ATA_WRITE_DMA       0xCA
#define     ATA_WRITE_DMA_EXT   0x35
#define     ATA_CACHE_FLUSH     0xE7
#define     ATA_CACHE_FLUSH_EXT 0xEA
#define     ATA_PACKET          0xA0
#define     ATA_IDENTIFY_PACKET 0xA1
#define     ATA_IDENTIFY        0xEC

/*
 * ATA CONTROL BLOCK
 */
#define ATA_CTL_ALT_STATUS      0x02
#define     ATA_NIEN            (1 << 1) // stop interrupts
#define     ATA_SRST            (1 << 2) // software reset
#define     ATA_HOB             (1 << 7) // high order byte
#define ATA_CTL_DEVICE_CTL      0x02
#define ATA_CTL_TO_ISA          0x03

enum ata_drive_type {
  ATA_PATAPI,
  ATA_SATAPI,
  ATA_PATA,
  ATA_SATA,
  ATA_UNKNOWN
};
static inline const char *drive_type_string(enum ata_drive_type type) {
#define DT(_t) case (_t): return #_t
  switch (type) {
    DT(ATA_PATAPI);
    DT(ATA_SATAPI);
    DT(ATA_PATA);
    DT(ATA_SATA);
    DT(ATA_UNKNOWN);
    default: return "ATA_UNKNOWN";
  }
#undef DT
}

struct ata_drive;
list_typedef(struct ata_drive) ata_drive_list_t;

struct ata_signature {
  uint8_t sector_count;
  uint8_t lba_low;
  uint8_t lba_mid;
  uint8_t lba_high;
  uint8_t device;
};

struct ata_bus {
  /*
   * All the drives on an ATA bus share the same I/O ports, but the system
   * can choose between drives using the ATA_CMD_DRIVE command.
   */
  unsigned cmd_block;
  unsigned ctl_block;

  bool exists;

  ata_drive_list_t drives;
};

struct ata_drive {
  struct ata_signature sig;
  enum ata_drive_type type;
  uint8_t select; // ATA_SELECT_SLAVE or ATA_SELECT_MASTER

  /*
   * True if there exists a drive in this slot.
   */
  bool exists;
  /*
   * True if the drive is set up properly and ready to be used.
   */
  bool usable;

#define ATA_IDENTIFY_SERIAL_WORD_OFFSET 10
#define ATA_IDENTIFY_SERIAL_WORD_LENGTH 10
  char serial[ATA_IDENTIFY_SERIAL_WORD_LENGTH*2 + 1];

#define ATA_IDENTIFY_FIRMWARE_WORD_OFFSET 23
#define ATA_IDENTIFY_FIRMWARE_WORD_LENGTH 4
  char firmware[ATA_IDENTIFY_FIRMWARE_WORD_LENGTH*2 + 1];

#define ATA_IDENTIFY_MODEL_WORD_OFFSET 27
#define ATA_IDENTIFY_MODEL_WORD_LENGTH 20
  char model[ATA_IDENTIFY_MODEL_WORD_LENGTH*2 + 1];

  /*
   * One greater than the total number of user addressable sectors. Determined
   * via the IDENTIFY DEVICE command.
   */
  unsigned sectors;

#define ATA_PIO_NOT_SUPPORTED -1
  int supported_pio_mode;
#define ATA_DMA_NOT_SUPPORTED -1
  int supported_dma_mode;

  /*
   * number of sectors per block supported by READ/WRITE MULTIPLE command
   */
  unsigned sectors_per_block;

  uint16_t major_version;
  uint16_t minor_version;

  struct ata_bus *bus;
  list_link(struct ata_drive) ata_bus_link;
};

int ata_new_bus(struct ata_bus *busp, unsigned cmd_block, unsigned ctl_block);
void ata_print_drive(struct ata_drive *drive);

#endif /* !__DEV_ATA_H__ */
