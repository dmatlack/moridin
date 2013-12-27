/**
 * @file dev/dma/piix.h
 *
 * @brief The PIIX/PIIX3 contains two DMA Controllers (82C37), DMA1 and DMA2.
 * The DMA registers control the operation of the DMA controllers and are
 * accessible to the Host CPU via the PCI Bus Interface.
 *
 *
 *    Channel 0 ---+------.   .--- 4 ---+------.
 *                 |      |   |         |      |
 *    Channel 1 ---|      |   |    5 ---|      |
 *                 | DMA1 |---'         | DMA2 |
 *    Channel 2 ---|      |        6 ---|      |
 *                 |      |             |      |
 *    Channel 3 ---+------'        7 ---+------'
 *
 * Channels [3:0] are hardwired to 8-bit count-by-bytes transfers and Channels
 * [7:5] are hardwired to 16-bit count-by-words (address shifted) transfers.
 *
 *
 *
 * PIIX/3 DMA Signals Overview
 * ---------------------------
 *
 * DREQ       DMA REQUEST. The DREQ lines are used to request DMA service from
 *            the DMA controller.
 *
 * DACK       DMA ACKNOWLEDGE. The DACK output lines indicate that a request
 *            for DMA service has been granted. 
 *
 * TC         TERMINAL COUNT. The PIIX/3 asserts TC to DMA slaves as a terminal
 *            count indicator. When all the DMA channels are not in use, the TC
 *            is negated (low).
 *
 * REFRESH    As an output, this idicates a REFRESH cycle is in progress. As an
 *            input, REFRESH is drive by 16-bit ISA Bus masters to initiate
 *            refresh cycles.
 *
 * http://download.intel.com/design/intarch/datashts/29055002.pdf
 *
 * @author David Matlack
 *
 * @disclaimer I do not get paid to convert Intel's datasheets to ascii. I just
 * like it... :'(
 */
#ifndef __DEV_DMA_PIIX_H__
#define __DEV_DMA_PIIX_H__

/*
 * DCOM: DMA Command Register
 *
 * This 8bit register controls the configuration of the DMA.
 * 
 * 7      DACK Active Level: 1=Active high, 0=Active low
 * 6      DREQ Seset Alert Level: 1=Active low, 0=Active high
 * 4      DMA Group Arbitration Priority: 1=Rotating priority, 0=Fixed
 * 2      DMA Channel Group Enable: 1=Disable, 0=Enable
 *
 * Write-only
 */
#define DMA_DCOM_03 0x08
#define DMA_DCOM_47 0xD0

/*
 * DCM: DMA Channel Mode Register
 *
 * Each channel has a 6-bit DCM. The DCM provides control over DMA transfer R
 * type, transfer mode, address increment/decrement, and autoinitialization.
 *
 * 7:6    DMA Transfer Mode. 00=Demand Mode, 01=Single Mode, 10=Block Mode,
 *        11=Cascade Mode.
 * 5      Address Increment/Decrement Select: 0=Increment, 1=Decrement.
 * 4      Autoinitialize Enable: 1=Enable, 0=Disable
 * 3:2    DMA Transfer Type. 00=Verify Transfer, 01=Write Transfer, 10=Read
 *        Transfer, 11=Illegal
 * 1:0    DMA Channel Select. 00=Channel 0 (4), 01=Channel 1 (5), 10=Channel
 *        2 (6), 11=Channel 3 (7).
 *
 * Write-only
 */
#define DMA_DCM_03 0x0B
#define DMA_DCM_47 0xD6

/*
 * DR: DMA Request Register
 *
 * The Request Register is used by software to initiate a DMA request. For a
 * software request, the channel must be in Block Mode.
 *
 * 7:3    Reserver (0)
 * 2      DMA Channel Service Request. 0=Resets the individual software DMA 
 *        channel request bit. 1=Sets the request bit. Generation of a TC also
 *        sets this bit to 0.
 * 1:0    DMA Channel Select. (See DCM[1:0]).
 *
 * Write-only
 */
#define DMA_DR_03 0x09
#define DMA_DR_47 0xD2

/*
 * SMASK: Write Single Mask Bit
 *
 * A channel's mask bit is automatically set when the current byte/word count
 * register reaches terminal count. Setting the entire register disables all
 * DMA requests until a clear mask register instruction allows them to occur.
 * 
 * Bit    Meaning
 * 2      Channel Mask Select. 1=Disable DREQ for select channel, 0=Enable
 *        DREQ for the selected channel.
 * 1:0    DMA Channel Select. (see DCM[1:0]).
 *
 * Write-only
 */
#define DMA_SMASK_03 0x0A
#define DMA_SMASK_47 0xD4

/*
 * AMASK: Write All Mask Bits
 * 
 * 3:0    Channel Mask Bits. 1=Disable the corresponding DREQ(s). 0=Enable
 *        the corresponding DREQ(s).
 *        
 *        0001    Channel 0 (4)
 *        0010    Channel 1 (5)
 *        0100    Channel 2 (6)
 *        1000    Channel 3 (7)
 *
 * Read/Write
 */
#define DMA_AMASK_03 0x0F
#define DMA_AMASK_47 0xDE

/*
 * DS: DMA Status Register
 *
 * Each DMA controller has a read-only status regiser, that indicates which
 * channels have reached terminal count and which channels have a pending
 * DMA request.
 *
 * 7:4    Channel Request Status. When a valid DMA request is pending for a
 *        channel (on its DREQ signal line), the corresponding bit is set to
 *        1. When a DMA request is not pending for a channel, the bit is set
 *        to 0. The source of the DREQ may be hardware or software request.
 *        Note that channel 4 does not have a DREQ or DACK line, so the
 *        response reqach from DMA2 is irrelevent.
 *
 *        Bit     Channel
 *        4       0
 *        5       1/5
 *        6       2/6
 *        7       3/7
 * 3:0    Channel Termination Count Status. 1=TC is reached, 0=TC is not
 *        reached.
 *
 * Read-only
 */
#define DMA_STATUS_03 0x08
#define DMA_STATUS_47 0xD0

#define DMA_CBP_03  0x0C // DMA Clear Byte Pointer Register
#define DMA_CBP_47  0xD8
#define DMA_DMC_03  0x0D // DMA MAster Clear Register
#define DMA_DMC_47  0xDA
#define DMA_DCLM_03 0x0E // DMA Clear Mask Register
#define DMA_DCLM_47 0xDC

#endif /* !__DEV_DMA_PIIX_H__ */
