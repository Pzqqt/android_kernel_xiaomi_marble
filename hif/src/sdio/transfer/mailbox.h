/*
 * Copyright (c) 2013-2014, 2016-2018 The Linux Foundation. All rights reserved.
 *
 *
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


#ifndef _MAILBOX_H_
#define _MAILBOX_H__

#include "a_debug.h"
#include "hif_sdio_dev.h"
#include "htc_packet.h"
#include "htc_api.h"
#include "hif_internal.h"

#define INVALID_MAILBOX_NUMBER 0xFF

#define OTHER_INTS_ENABLED (INT_STATUS_ENABLE_ERROR_MASK |	\
			    INT_STATUS_ENABLE_CPU_MASK   |	\
			    INT_STATUS_ENABLE_COUNTER_MASK)

/* HTC operational parameters */
#define HTC_TARGET_RESPONSE_TIMEOUT        2000 /* in ms */
#define HTC_TARGET_DEBUG_INTR_MASK         0x01
#define HTC_TARGET_CREDIT_INTR_MASK        0xF0

#define MAILBOX_COUNT 4
#define MAILBOX_FOR_BLOCK_SIZE 1
#define MAILBOX_USED_COUNT 2
#if defined(SDIO_3_0)
#define MAILBOX_LOOKAHEAD_SIZE_IN_WORD 2
#else
#define MAILBOX_LOOKAHEAD_SIZE_IN_WORD 1
#endif
#define AR6K_TARGET_DEBUG_INTR_MASK     0x01

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
#define HIF_MBOX0_EXTENDED_WIDTH_AR6002    (6 * 1024)
#define HIF_MBOX0_EXTENDED_WIDTH_AR6003    (18 * 1024)

/* version 1 of the chip has only a 12K extended mbox range */
#define HIF_MBOX0_EXTENDED_BASE_ADDR_AR6003_V1  0x4000
#define HIF_MBOX0_EXTENDED_WIDTH_AR6003_V1      (12 * 1024)

#define HIF_MBOX0_EXTENDED_BASE_ADDR_AR6004     0x2800
#define HIF_MBOX0_EXTENDED_WIDTH_AR6004         (18 * 1024)

#if defined(SDIO_3_0)
#define HIF_MBOX0_EXTENDED_BASE_ADDR_AR6320     0x5000
#define HIF_MBOX0_EXTENDED_WIDTH_AR6320             (36 * 1024)
#define HIF_MBOX0_EXTENDED_WIDTH_AR6320_ROME_2_0    (56 * 1024)
#define HIF_MBOX1_EXTENDED_WIDTH_AR6320             (36 * 1024)
#define HIF_MBOX_DUMMY_SPACE_SIZE_AR6320        (2 * 1024)
#else
#define HIF_MBOX0_EXTENDED_BASE_ADDR_AR6320     0x2800
#define HIF_MBOX0_EXTENDED_WIDTH_AR6320             (24 * 1024)
#define HIF_MBOX1_EXTENDED_WIDTH_AR6320             (24 * 1024)
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

#define HIF_BLOCK_SIZE                HIF_DEFAULT_IO_BLOCK_SIZE
#define HIF_MBOX0_BLOCK_SIZE          1
#define HIF_MBOX1_BLOCK_SIZE          HIF_BLOCK_SIZE
#define HIF_MBOX2_BLOCK_SIZE          HIF_BLOCK_SIZE
#define HIF_MBOX3_BLOCK_SIZE          HIF_BLOCK_SIZE

/*
 * data written into the dummy space will not put into the final mbox FIFO
 */
#define HIF_DUMMY_SPACE_MASK                   0xFFFF0000

/*
 * data written into the dummy space will not put into the final mbox FIFO
 */
#define HIF_DUMMY_SPACE_MASK                   0xFFFF0000

PREPACK struct MBOX_IRQ_PROC_REGISTERS {
	uint8_t host_int_status;
	uint8_t cpu_int_status;
	uint8_t error_int_status;
	uint8_t counter_int_status;
	uint8_t mbox_frame;
	uint8_t rx_lookahead_valid;
	uint8_t host_int_status2;
	uint8_t gmbox_rx_avail;
	uint32_t rx_lookahead[MAILBOX_LOOKAHEAD_SIZE_IN_WORD * MAILBOX_COUNT];
	uint32_t int_status_enable;
} POSTPACK;

PREPACK struct MBOX_IRQ_ENABLE_REGISTERS {
	uint8_t int_status_enable;
	uint8_t cpu_int_status_enable;
	uint8_t error_status_enable;
	uint8_t counter_int_status_enable;
} POSTPACK;

#define TOTAL_CREDIT_COUNTER_CNT 4

PREPACK struct MBOX_COUNTER_REGISTERS {
	uint32_t counter[TOTAL_CREDIT_COUNTER_CNT];
} POSTPACK;

struct devRegisters {
	struct MBOX_IRQ_PROC_REGISTERS IrqProcRegisters;
	struct MBOX_IRQ_ENABLE_REGISTERS IrqEnableRegisters;
	struct MBOX_COUNTER_REGISTERS MailBoxCounterRegisters;
};

#define mboxProcRegs(hdev)	hdev->devRegisters.IrqProcRegisters
#define mboxEnaRegs(hdev)	hdev->devRegisters.IrqEnableRegisters
#define mboxCountRegs(hdev)	hdev->devRegisters.MailBoxCounterRegisters

#define DEV_REGISTERS_SIZE	(sizeof(struct MBOX_IRQ_PROC_REGISTERS) + \
				 sizeof(struct MBOX_IRQ_ENABLE_REGISTERS) + \
				 sizeof(struct MBOX_COUNTER_REGISTERS))
#endif /* _MAILBOX_H_ */
