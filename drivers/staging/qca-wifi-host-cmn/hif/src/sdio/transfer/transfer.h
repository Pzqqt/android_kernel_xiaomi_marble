/*
 * Copyright (c) 2013-2020 The Linux Foundation. All rights reserved.
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

#ifndef __TRANSFER_H_
#define __TRANSFER_H_

#include <qdf_types.h>
#include <qdf_status.h>
#include <qdf_timer.h>
#include <qdf_time.h>
#include <qdf_lock.h>
#include <qdf_mem.h>
#include <qdf_util.h>
#include <qdf_defer.h>
#include <qdf_atomic.h>
#include <qdf_nbuf.h>
#include <athdefs.h>
#include <qdf_net_types.h>
#include <a_types.h>
#include <athdefs.h>
#include <a_osapi.h>
#include <hif.h>
#include <htc_services.h>
#include <a_debug.h>
#include "hif_sdio_internal.h"

#if defined(CONFIG_SDIO_TRANSFER_MAILBOX) && defined(CONFIG_SDIO_TRANSFER_ADMA)
#error "-----------------------------------------------"
#error "Error - Both transfer methods cannot be enabled"
#error "-----------------------------------------------"
#endif

#define NBUF_ALLOC_FAIL_WAIT_TIME 100
/* high nibble */
#define BUNDLE_COUNT_HIGH(f) (((f) & 0x0C) << 2)
/* low nibble */
#define BUNDLE_COUNT_LOW(f)  (((f) & 0xF0) >> 4)
#define GET_RECV_BUNDLE_COUNT(f) (BUNDLE_COUNT_HIGH(f) + BUNDLE_COUNT_LOW(f))

/*
 * Data structure to record required sending context data
 */
struct hif_sendContext {
	bool bNewAlloc;
	struct hif_sdio_device *pDev;
	qdf_nbuf_t netbuf;
	unsigned int transferID;
	unsigned int head_data_len;
};

int hif_get_send_address(struct hif_sdio_device *pdev,
			 uint8_t pipe, unsigned long *addr);

QDF_STATUS hif_dev_alloc_and_prepare_rx_packets(struct hif_sdio_device *pdev,
						uint32_t look_aheads[],
						int messages,
						HTC_PACKET_QUEUE *queue);

QDF_STATUS hif_dev_process_trailer(struct hif_sdio_device *pdev,
				   uint8_t *buffer, int length,
				   uint32_t *next_look_aheads,
				   int *num_look_aheads,
				   HTC_ENDPOINT_ID from_endpoint);

void hif_dev_free_recv_pkt_queue(HTC_PACKET_QUEUE *recv_pkt_queue);

QDF_STATUS hif_dev_process_recv_header(struct hif_sdio_device *pdev,
				       HTC_PACKET *packet,
				       uint32_t *next_look_aheads,
				       int *num_look_aheads);
void hif_fixup_write_param(struct hif_sdio_dev *pdev, uint32_t req,
			   uint32_t *length, uint32_t *addr);

#ifdef CONFIG_SDIO_TRANSFER_MAILBOX
static inline uint32_t hif_get_send_buffer_flags(struct hif_sdio_device *pdev)
{
	if (pdev)
		return (uint32_t)HIF_WR_ASYNC_BLOCK_INC;

	hif_err("hif obj is null. Not populating xfer flags");

	return 0;
}

static inline int hif_sdio_bus_configure(struct hif_softc *hif_sc)
{
	return 0;
}

#elif defined(CONFIG_SDIO_TRANSFER_ADMA)
static inline uint32_t hif_get_send_buffer_flags(struct hif_sdio_device *pdev)
{
	/* ADAM-TODO */
	return (uint32_t)HIF_WR_ASYNC_BLOCK_FIX;
}

int hif_sdio_bus_configure(struct hif_softc *hif_sc);
#endif

#endif /* __TRANSFER_H__ */
