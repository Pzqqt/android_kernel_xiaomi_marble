/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _HIF_SDIO_COMMON_H_
#define _HIF_SDIO_COMMON_H_

/* SDIO manufacturer ID and Codes */
#define MANUFACTURER_ID_AR6002_BASE        0x200
#define MANUFACTURER_ID_AR6003_BASE        0x300
#define MANUFACTURER_ID_AR6004_BASE        0x400
#define MANUFACTURER_ID_AR6320_BASE        0x500
#define MANUFACTURER_ID_QCA9377_BASE       0x700
#define MANUFACTURER_ID_QCA9379_BASE       0x800
#define MANUFACTURER_ID_AR6K_BASE_MASK     0xFF00
#define MANUFACTURER_ID_AR6K_REV_MASK      0x00FF
#define FUNCTION_CLASS                     0x0
#define MANUFACTURER_CODE                  0x271

    /* Mailbox address in SDIO address space */
#if defined(SDIO_3_0)
#define HIF_MBOX_BASE_ADDR                 0x1000
#define HIF_MBOX_DUMMY_WIDTH               0x800
#else
#define HIF_MBOX_BASE_ADDR                 0x800
#define HIF_MBOX_DUMMY_WIDTH               0
#endif

#define HIF_MBOX_WIDTH                     0x800

#define HIF_MBOX_START_ADDR(mbox)               \
	(HIF_MBOX_BASE_ADDR + mbox * (HIF_MBOX_WIDTH + HIF_MBOX_DUMMY_WIDTH))

#define HIF_MBOX_END_ADDR(mbox)                 \
	(HIF_MBOX_START_ADDR(mbox) + HIF_MBOX_WIDTH - 1)

    /* extended MBOX address for larger MBOX writes to MBOX 0*/
#if defined(SDIO_3_0)
#define HIF_MBOX0_EXTENDED_BASE_ADDR       0x5000
#else
#define HIF_MBOX0_EXTENDED_BASE_ADDR       0x2800
#endif
#define HIF_MBOX0_EXTENDED_WIDTH_AR6002    (6*1024)
#define HIF_MBOX0_EXTENDED_WIDTH_AR6003    (18*1024)

    /* version 1 of the chip has only a 12K extended mbox range */
#define HIF_MBOX0_EXTENDED_BASE_ADDR_AR6003_V1  0x4000
#define HIF_MBOX0_EXTENDED_WIDTH_AR6003_V1      (12*1024)

#define HIF_MBOX0_EXTENDED_BASE_ADDR_AR6004     0x2800
#define HIF_MBOX0_EXTENDED_WIDTH_AR6004         (18*1024)


#if defined(SDIO_3_0)
#define HIF_MBOX0_EXTENDED_BASE_ADDR_AR6320     0x5000
#define HIF_MBOX0_EXTENDED_WIDTH_AR6320             (36*1024)
#define HIF_MBOX0_EXTENDED_WIDTH_AR6320_ROME_2_0    (56*1024)
#define HIF_MBOX1_EXTENDED_WIDTH_AR6320             (36*1024)
#define HIF_MBOX_DUMMY_SPACE_SIZE_AR6320        (2*1024)
#else
#define HIF_MBOX0_EXTENDED_BASE_ADDR_AR6320     0x2800
#define HIF_MBOX0_EXTENDED_WIDTH_AR6320             (24*1024)
#define HIF_MBOX1_EXTENDED_WIDTH_AR6320             (24*1024)
#define HIF_MBOX_DUMMY_SPACE_SIZE_AR6320        0
#endif


    /* GMBOX addresses */
#define HIF_GMBOX_BASE_ADDR                0x7000
#define HIF_GMBOX_WIDTH                    0x4000

/* for SDIO we recommend a 128-byte block size */
#if defined(WITH_BACKPORTS)
#define HIF_DEFAULT_IO_BLOCK_SIZE          128
#else
#define HIF_DEFAULT_IO_BLOCK_SIZE          256
#endif

#define FIFO_TIMEOUT_AND_CHIP_CONTROL 0x00000868
#define FIFO_TIMEOUT_AND_CHIP_CONTROL_DISABLE_SLEEP_OFF 0xFFFEFFFF
#define FIFO_TIMEOUT_AND_CHIP_CONTROL_DISABLE_SLEEP_ON 0x10000
/* In SDIO 2.0, asynchronous interrupt is not in SPEC
 * requirement, but AR6003 support it, so the register
 * is placed in vendor specific field 0xF0(bit0)
 * In SDIO 3.0, the register is defined in SPEC, and its
 * address is 0x16(bit1)
 */
/* interrupt mode register of AR6003 */
#define CCCR_SDIO_IRQ_MODE_REG_AR6003         0xF0
/* mode to enable special 4-bit interrupt assertion without clock */
#define SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6003   (1 << 0)
    /* interrupt mode register of AR6320 */
#define CCCR_SDIO_IRQ_MODE_REG_AR6320           0x16
/* mode to enable special 4-bit interrupt assertion without clock */
#define SDIO_IRQ_MODE_ASYNC_4BIT_IRQ_AR6320     (1 << 1)

#define CCCR_SDIO_ASYNC_INT_DELAY_ADDRESS       0xF0
#define CCCR_SDIO_ASYNC_INT_DELAY_LSB           0x06
#define CCCR_SDIO_ASYNC_INT_DELAY_MASK          0xC0

/* Vendor Specific Driver Strength Settings */
#define CCCR_SDIO_DRIVER_STRENGTH_ENABLE_ADDR   0xf2
#define CCCR_SDIO_DRIVER_STRENGTH_ENABLE_MASK   0x0e
#define CCCR_SDIO_DRIVER_STRENGTH_ENABLE_A      0x02
#define CCCR_SDIO_DRIVER_STRENGTH_ENABLE_C      0x04
#define CCCR_SDIO_DRIVER_STRENGTH_ENABLE_D      0x08

#endif /* _HIF_SDIO_COMMON_H_ */
