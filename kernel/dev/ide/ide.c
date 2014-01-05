/**
 * @file dev/ide/ide.c
 */
#include <dev/ide.h>
#include <dev/ata.h>
#include <dev/pci.h>
#include <errno.h>

/**
 * @brief Initialize a new IDE controller with the following parameters.
 *
 * @param bm_offset The I/O address of the bus master I/O block.
 * @param irq The irq tied to this IDE.
 * @param ata_cmd The I/O address of the ATA command block.
 * @param ata_ctl The I/O address of the ATA control block.
 *
 * @return
 *    0 on success
 *    ENOMEM if the kernel runs out of memory
 */
int ide_init(struct ide_device *ide, unsigned bm_offset, int irq,
             unsigned ata_cmd, unsigned ata_ctl) {
  int ret;

  /*
   * Initialize the ata bus
   */
  ret = ata_init_bus(&ide->ata, irq, ata_cmd, ata_ctl);
  if (ret) goto cleanup_ata;

  /*
   * Initialize the bus master
   */
  ret = pci_init_bm(&ide->bm, bm_offset);
  if (ret) goto cleanup_bm;

  return 0;

cleanup_bm:
  ata_destroy_bus(&ide->ata);
cleanup_ata:
  return ret;
}

/**
 * @brief Free all memory helf by an ide_device struct. Do not free the
 * ide_device struct itself.
 */
void ide_destroy(struct ide_device *ide) {
  (void) ide;
}
