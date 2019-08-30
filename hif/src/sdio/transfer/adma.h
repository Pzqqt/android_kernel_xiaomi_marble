/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#ifndef _ADMA_H_
#define _ADMA_H_

#include "hif_sdio_dev.h"
#include "htc_packet.h"
#include "htc_api.h"
#include "hif_internal.h"

/* This should align with the underlying transport layer */
#define HIF_DEFAULT_IO_BLOCK_SIZE    512
#define HIF_BLOCK_SIZE               HIF_DEFAULT_IO_BLOCK_SIZE
#define HIF_DUMMY_SPACE_MASK         0x0FFFFFFF

#define HIF_SDIO_MAX_AL_CHANNELS     2

struct devRegisters {
	uint32_t dummy;
};

#include "transfer.h"
#define DEV_REGISTERS_SIZE           sizeof(struct devRegisters)

uint8_t hif_dev_map_adma_chan_to_pipe(struct hif_sdio_device *pdev,
				      uint8_t chan, bool upload);

struct sdio_al_channel_handle *hif_dev_map_pipe_to_adma_chan
(
struct hif_sdio_device *pdev,
uint8_t pipeid
);

void dl_xfer_cb(struct sdio_al_channel_handle *ch_handle,
		struct sdio_al_xfer_result *result,
		void *ctx);
void ul_xfer_cb(struct sdio_al_channel_handle *ch_handle,
		struct sdio_al_xfer_result *result,
		void *ctx);

void dl_data_avail_cb(struct sdio_al_channel_handle *ch_handle,
		      unsigned int len);

void hif_sdio_rx_q_alloc(void *ctx);
#endif
