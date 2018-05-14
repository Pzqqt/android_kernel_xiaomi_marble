/*
 * Copyright (c) 2013-2014, 2016-2018 The Linux Foundation. All rights reserved.
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

#ifndef _HIF_SDIO_INTERNAL_H_
#define _HIF_SDIO_INTERNAL_H_

#include "a_debug.h"
#include "hif_sdio_dev.h"
#include "htc_packet.h"
#include "htc_api.h"
#include "hif_internal.h"

#if defined(CONFIG_SDIO_TRANSFER_MAILBOX)
#include <transfer/mailbox.h>
#elif defined(CONFIG_SDIO_TRANSFER_ADMA)
#error "Error - Not implemented yet"
#else
#error "Error - Invalid transfer method"
#endif

#define INVALID_MAILBOX_NUMBER 0xFF

#define HIF_SDIO_RX_BUFFER_SIZE            1792
#define HIF_SDIO_RX_DATA_OFFSET            64

/* TODO: print output level and mask control */
#define ATH_DEBUG_IRQ  ATH_DEBUG_MAKE_MODULE_MASK(4)
#define ATH_DEBUG_XMIT ATH_DEBUG_MAKE_MODULE_MASK(5)
#define ATH_DEBUG_RECV ATH_DEBUG_MAKE_MODULE_MASK(6)

#define ATH_DEBUG_MAX_MASK 32

#define SDIO_NUM_DATA_RX_BUFFERS  64
#define SDIO_DATA_RX_SIZE         1664

struct hif_sdio_device {
	struct hif_sdio_dev *HIFDevice;
	qdf_spinlock_t Lock;
	qdf_spinlock_t TxLock;
	qdf_spinlock_t RxLock;
	struct hif_msg_callbacks hif_callbacks;
	struct hif_device_mbox_info MailBoxInfo;
	uint32_t BlockSize;
	uint32_t BlockMask;
	enum hif_device_irq_mode HifIRQProcessingMode;
	struct hif_device_irq_yield_params HifIRQYieldParams;
	bool DSRCanYield;
	HIF_MASK_UNMASK_RECV_EVENT HifMaskUmaskRecvEvent;
	int CurrentDSRRecvCount;
	int RecheckIRQStatusCnt;
	uint32_t RecvStateFlags;
	void *pTarget;
	bool swap_mailbox;
	struct devRegisters devRegisters;
};

#define LOCK_HIF_DEV(device)    qdf_spin_lock(&(device)->Lock)
#define UNLOCK_HIF_DEV(device)  qdf_spin_unlock(&(device)->Lock)
#define LOCK_HIF_DEV_RX(t)      qdf_spin_lock(&(t)->RxLock)
#define UNLOCK_HIF_DEV_RX(t)    qdf_spin_unlock(&(t)->RxLock)
#define LOCK_HIF_DEV_TX(t)      qdf_spin_lock(&(t)->TxLock)
#define UNLOCK_HIF_DEV_TX(t)    qdf_spin_unlock(&(t)->TxLock)

#define DEV_CALC_RECV_PADDED_LEN(pDev, length) \
		(((length) + (pDev)->BlockMask) & (~((pDev)->BlockMask)))
#define DEV_CALC_SEND_PADDED_LEN(pDev, length) \
		DEV_CALC_RECV_PADDED_LEN(pDev, length)
#define DEV_IS_LEN_BLOCK_ALIGNED(pDev, length) \
		(((length) % (pDev)->BlockSize) == 0)

#define HTC_RECV_WAIT_BUFFERS        (1 << 0)
#define HTC_OP_STATE_STOPPING        (1 << 0)

#define HTC_RX_PKT_IGNORE_LOOKAHEAD      (1 << 0)
#define HTC_RX_PKT_REFRESH_HDR           (1 << 1)
#define HTC_RX_PKT_PART_OF_BUNDLE        (1 << 2)
#define HTC_RX_PKT_NO_RECYCLE            (1 << 3)
#define HTC_RX_PKT_LAST_BUNDLED_PKT_HAS_ADDTIONAL_BLOCK     (1 << 4)

#define IS_DEV_IRQ_PROCESSING_ASYNC_ALLOWED(pDev) \
		((pDev)->HifIRQProcessingMode != HIF_DEVICE_IRQ_SYNC_ONLY)

/* hif_sdio_dev.c */
HTC_PACKET *hif_dev_alloc_rx_buffer(struct hif_sdio_device *pDev);

/* hif_sdio_recv.c */
QDF_STATUS hif_dev_rw_completion_handler(void *context, QDF_STATUS status);
QDF_STATUS hif_dev_dsr_handler(void *context);

#endif /* _HIF_SDIO_INTERNAL_H_ */
