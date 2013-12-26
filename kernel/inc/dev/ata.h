/**
 * @file dev/ata.h
 *
 * @brief Advanced Technology Attachement 
 *
 * ATA-6 Spec:
 *   http://www.t13.org/documents/UploadedDocuments/project/d1410r3b-ATA-ATAPI-6.pdf
 *
 * @author David Matlack
 */
#ifndef __DEV_ATA_H__
#define __DEV_ATA_H__

#include <list.h>
#include <types.h>
#include <stdint.h>

/*
 * ATA COMMAND BLOCK
 */
#define ATA_CMD_DATA           0x00
#define ATA_CMD_ERROR          0x01
#define ATA_CMD_FEATURES       0x01
#define ATA_CMD_SECTOR_COUNT   0x02
#define ATA_CMD_SECTOR_NUM     0x03
#define ATA_CMD_CYLINDER_LO    0x04
#define ATA_CMD_CYLINDER_HI    0x05
#define ATA_CMD_HEAD           0x06
#define ATA_CMD_DRIVE          0x06
#define     ATA_SELECT_MASTER  0xA0
#define     ATA_SELECT_SLAVE   0xB0
#define ATA_CMD_STATUS         0x07
#define     ATA_ERR            (1 << 0)
#define     ATA_DRQ            (1 << 3)
#define     ATA_SRV            (1 << 4)
#define     ATA_DF             (1 << 5)
#define     ATA_RDY            (1 << 6)
#define     ATA_BSY            (1 << 7)
#define ATA_CMD_COMMAND        0x07
#define     ATA_IDENTIFY       0xEC

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
  enum ata_drive_type type;
  bool exists;
  uint16_t identify[256];
  uint8_t select; // ATA_SELECT_SLAVE or ATA_SELECT_MASTER

  struct ata_bus *bus;

  list_link(struct ata_drive) ata_bus_link;
};

int ata_new_bus(struct ata_bus *busp, unsigned cmd_block, unsigned ctl_block);

#endif /* !__DEV_ATA_H__ */
