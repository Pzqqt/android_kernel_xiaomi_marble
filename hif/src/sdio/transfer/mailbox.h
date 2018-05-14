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
