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

#include <types.h>
#include <stdint.h>
#include <dev/ide/lba.h>

#define ATA_BYTES_PER_SECTOR 512

/*
 * ATA COMMAND BLOCK
 */
#define ATA_CMD_DATA            0x00
#define ATA_CMD_ERROR           0x01
#define     ATA_NM              (1 << 1)
#define     ATA_ABRT            (1 << 2)
#define     ATA_MCR             (1 << 3)
#define     ATA_IDNF            (1 << 4)
#define     ATA_MC              (1 << 5)
#define     ATA_UNC             (1 << 6)
#define     ATA_WP              (1 << 6)
#define     ATA_ICRC            (1 << 7)
#define ATA_CMD_FEATURES        0x01
#define ATA_CMD_SECTOR_COUNT    0x02
#define ATA_CMD_LBA_LOW         0x03
#define ATA_CMD_LBA_MID         0x04
#define ATA_CMD_LBA_HIGH        0x05
#define ATA_CMD_HEAD            0x06
#define ATA_CMD_DEVICE          0x06
#define     ATA_SELECT_MASTER   (0 << 4)
#define     ATA_SELECT_SLAVE    (1 << 4)
#define     ATA_DEVICE_LBA      (1 << 6)
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
#define     ATA_SET_FEATURES    0xEF

/* 
 * SET FEATURES
 */
#define ATA_TRANSFER_MODE_SUBCMD 0x03
#define ATA_DMA_MODE(_mode) (1 << 5) | (_mode)


/*
 * ATA CONTROL BLOCK
 */
#define ATA_CTL_ALT_STATUS      0x02
#define     ATA_NIEN            (1 << 1) // stop interrupts
#define     ATA_SRST            (1 << 2) // software reset
#define     ATA_HOB             (1 << 7) // high order byte
#define ATA_CTL_DEVICE_CTL      0x02
#define ATA_CTL_TO_ISA          0x03

#define ATA_WARN(_drive, fmt, ...) \
  WARN("ATA %s Drive: "fmt, \
       (_drive)->select & ATA_SELECT_MASTER ? "Master" : "Slave", \
       ##__VA_ARGS__)

#define ATA_MAX_TIMEOUT 0x1000
#define ATA_WAIT(_cmd, _status, _timeout, _error, _fault) \
do { \
  _timeout = 0; \
  _error = 0; \
  _fault = 0; \
  while (_timeout++ < ATA_MAX_TIMEOUT) { \
    _status = inb(_cmd + ATA_CMD_STATUS); \
     \
    if (_status & ATA_ERR) { \
      _error = inb(_cmd + ATA_CMD_ERROR); \
      _timeout = 0; \
      break; \
    } \
    if (_status & ATA_DF) { \
      _fault = 1; \
      _timeout = 0; \
      break; \
    } \
    if (!(_status & ATA_BSY) && (_status & ATA_RDY) && !(_status & ATA_DRQ)) { \
      _timeout = 0; \
      break; \
    } \
  } \
} while (0)

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

struct ata_signature {
  uint8_t sector_count;
  uint8_t lba_low;
  uint8_t lba_mid;
  uint8_t lba_high;
  uint8_t device;
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
  int dma_mode;
  int dma_min_nano;
  int dma_nano;

  /*
   * number of sectors per block supported by READ/WRITE MULTIPLE command
   */
  unsigned sectors_per_block;

  uint16_t major_version;
  uint16_t minor_version;

  struct ata_bus *bus;
};

struct ata_bus {
  bool exists;

  int irq;
  unsigned cmd;
  unsigned ctl;

  struct ata_drive master;
  struct ata_drive slave;
};

int  ata_init_bus(struct ata_bus *bus, int irq, int cmd, int ctl);
void ata_destroy_bus(struct ata_bus *bus);

/*
 * DMA reqests to ATA drives
 */
void ata_read_dma(struct ata_drive *drive, lba28_t lba, uint8_t sectors);
void ata_write_dma(struct ata_drive *d, lba28_t lba, uint8_t sectors);
int  ata_dma_done(struct ata_drive *drive);

#endif /* !__DEV_ATA_H__ */
