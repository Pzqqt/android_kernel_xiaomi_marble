/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#include <osdep.h>
#include "a_types.h"
#include "athdefs.h"
#include "osapi_linux.h"
#include "targcfg.h"
#include "cdf_lock.h"
#include "cdf_status.h"
#include <cdf_atomic.h>         /* cdf_atomic_read */
#include <targaddrs.h>
#include <bmi_msg.h>
#include "hif_io32.h"
#include <hif.h>
#include "regtable.h"
#define ATH_MODULE_NAME hif
#include <a_debug.h>
#include "hif_main.h"
#include "ce_api.h"
#include "cdf_trace.h"
#include "cds_api.h"
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#else
#include "cnss_stub.h"
#endif
#include "epping_main.h"
#include "hif_debug.h"

/* Track a BMI transaction that is in progress */
#ifndef BIT
#define BIT(n) (1 << (n))
#endif

enum {
	BMI_REQ_SEND_DONE = BIT(0),   /* the bmi tx completion */
	BMI_RESP_RECV_DONE = BIT(1),  /* the bmi respond is received */
};

struct BMI_transaction {
	struct HIF_CE_state *hif_state;
	cdf_semaphore_t bmi_transaction_sem;
	uint8_t *bmi_request_host;        /* Req BMI msg in Host addr space */
	cdf_dma_addr_t bmi_request_CE;    /* Req BMI msg in CE addr space */
	uint32_t bmi_request_length;      /* Length of BMI request */
	uint8_t *bmi_response_host;       /* Rsp BMI msg in Host addr space */
	cdf_dma_addr_t bmi_response_CE;   /* Rsp BMI msg in CE addr space */
	unsigned int bmi_response_length; /* Length of received response */
	unsigned int bmi_timeout_ms;
	uint32_t bmi_transaction_flags;   /* flags for the transcation */
};

/*
 * send/recv completion functions for BMI.
 * NB: The "net_buf" parameter is actually just a
 * straight buffer, not an sk_buff.
 */
void hif_bmi_send_done(struct CE_handle *copyeng, void *ce_context,
		  void *transfer_context, cdf_dma_addr_t data,
		  unsigned int nbytes,
		  unsigned int transfer_id, unsigned int sw_index,
		  unsigned int hw_index, uint32_t toeplitz_hash_result)
{
	struct BMI_transaction *transaction =
		(struct BMI_transaction *)transfer_context;
	struct ol_softc *scn = transaction->hif_state->scn;

#ifdef BMI_RSP_POLLING
	/*
	 * Fix EV118783, Release a semaphore after sending
	 * no matter whether a response is been expecting now.
	 */
	cdf_semaphore_release(scn->cdf_dev,
			      &transaction->bmi_transaction_sem);
#else
	/*
	 * If a response is anticipated, we'll complete the
	 * transaction if the response has been received.
	 * If no response is anticipated, complete the
	 * transaction now.
	 */
	transaction->bmi_transaction_flags |= BMI_REQ_SEND_DONE;

	/* resp is't needed or has already been received,
	 * never assume resp comes later then this */
	if (!transaction->bmi_response_CE ||
	    (transaction->bmi_transaction_flags & BMI_RESP_RECV_DONE)) {
		cdf_semaphore_release(scn->cdf_dev,
				      &transaction->bmi_transaction_sem);
	}
#endif
}

#ifndef BMI_RSP_POLLING
void hif_bmi_recv_data(struct CE_handle *copyeng, void *ce_context,
		  void *transfer_context, cdf_dma_addr_t data,
		  unsigned int nbytes,
		  unsigned int transfer_id, unsigned int flags)
{
	struct BMI_transaction *transaction =
		(struct BMI_transaction *)transfer_context;
	struct ol_softc *scn = transaction->hif_state->scn;

	transaction->bmi_response_length = nbytes;
	transaction->bmi_transaction_flags |= BMI_RESP_RECV_DONE;

	/* when both send/recv are done, the sem can be released */
	if (transaction->bmi_transaction_flags & BMI_REQ_SEND_DONE) {
		cdf_semaphore_release(scn->cdf_dev,
				      &transaction->bmi_transaction_sem);
	}
}
#endif

CDF_STATUS hif_exchange_bmi_msg(struct ol_softc *scn,
		     uint8_t *bmi_request,
		     uint32_t request_length,
		     uint8_t *bmi_response,
		     uint32_t *bmi_response_lengthp, uint32_t TimeoutMS)
{
	struct HIF_CE_state *hif_state = (struct HIF_CE_state *)scn->hif_hdl;
	struct HIF_CE_pipe_info *send_pipe_info =
		&(hif_state->pipe_info[BMI_CE_NUM_TO_TARG]);
	struct CE_handle *ce_send_hdl = send_pipe_info->ce_hdl;
	cdf_dma_addr_t CE_request, CE_response = 0;
	struct BMI_transaction *transaction = NULL;
	int status = CDF_STATUS_SUCCESS;
	struct HIF_CE_pipe_info *recv_pipe_info =
		&(hif_state->pipe_info[BMI_CE_NUM_TO_HOST]);
	struct CE_handle *ce_recv = recv_pipe_info->ce_hdl;
	unsigned int mux_id = 0;
	unsigned int transaction_id = 0xffff;
	unsigned int user_flags = 0;
	struct bmi_info *info = hif_get_bmi_ctx(scn);
#ifdef BMI_RSP_POLLING
	cdf_dma_addr_t buf;
	unsigned int completed_nbytes, id, flags;
	int i;
#endif

	transaction =
		(struct BMI_transaction *)cdf_mem_malloc(sizeof(*transaction));
	if (unlikely(!transaction)) {
		HIF_ERROR("%s: no memory", __func__);
		return CDF_STATUS_E_NOMEM;
	}
	transaction_id = (mux_id & MUX_ID_MASK) |
		(transaction_id & TRANSACTION_ID_MASK);
#ifdef QCA_WIFI_3_0
	user_flags &= DESC_DATA_FLAG_MASK;
#endif
	A_TARGET_ACCESS_LIKELY(scn);

	/* Initialize bmi_transaction_sem to block */
	cdf_semaphore_init(&transaction->bmi_transaction_sem);
	cdf_semaphore_acquire(scn->cdf_dev, &transaction->bmi_transaction_sem);

	transaction->hif_state = hif_state;
	transaction->bmi_request_host = bmi_request;
	transaction->bmi_request_length = request_length;
	transaction->bmi_response_length = 0;
	transaction->bmi_timeout_ms = TimeoutMS;
	transaction->bmi_transaction_flags = 0;

	/*
	 * CE_request = dma_map_single(dev,
	 * (void *)bmi_request, request_length, DMA_TO_DEVICE);
	 */
	CE_request = info->bmi_cmd_da;
	transaction->bmi_request_CE = CE_request;

	if (bmi_response) {

		/*
		 * CE_response = dma_map_single(dev, bmi_response,
		 * BMI_DATASZ_MAX, DMA_FROM_DEVICE);
		 */
		CE_response = info->bmi_rsp_da;
		transaction->bmi_response_host = bmi_response;
		transaction->bmi_response_CE = CE_response;
		/* dma_cache_sync(dev, bmi_response,
		    BMI_DATASZ_MAX, DMA_FROM_DEVICE); */
		cdf_os_mem_dma_sync_single_for_device(scn->cdf_dev,
					       CE_response,
					       BMI_DATASZ_MAX,
					       DMA_FROM_DEVICE);
		ce_recv_buf_enqueue(ce_recv, transaction,
				    transaction->bmi_response_CE);
		/* NB: see HIF_BMI_recv_done */
	} else {
		transaction->bmi_response_host = NULL;
		transaction->bmi_response_CE = 0;
	}

	/* dma_cache_sync(dev, bmi_request, request_length, DMA_TO_DEVICE); */
	cdf_os_mem_dma_sync_single_for_device(scn->cdf_dev, CE_request,
				       request_length, DMA_TO_DEVICE);

	status =
		ce_send(ce_send_hdl, transaction,
			CE_request, request_length,
			transaction_id, 0, user_flags);
	ASSERT(status == CDF_STATUS_SUCCESS);
	/* NB: see hif_bmi_send_done */

	/* TBDXXX: handle timeout */

	/* Wait for BMI request/response transaction to complete */
	/* Always just wait for BMI request here if
	 * BMI_RSP_POLLING is defined */
	while (cdf_semaphore_acquire
		       (scn->cdf_dev, &transaction->bmi_transaction_sem)) {
		/*need some break out condition(time out?) */
	}

	if (bmi_response) {
#ifdef BMI_RSP_POLLING
		/* Fix EV118783, do not wait a semaphore for the BMI response
		 * since the relative interruption may be lost.
		 * poll the BMI response instead.
		 */
		i = 0;
		while (ce_completed_recv_next(
			    ce_recv, NULL, NULL, &buf,
			    &completed_nbytes, &id,
			    &flags) != CDF_STATUS_SUCCESS) {
			if (i++ > BMI_RSP_TO_MILLISEC) {
				HIF_ERROR("%s:error, can't get bmi response\n",
					__func__);
				status = CDF_STATUS_E_BUSY;
				break;
			}
			OS_DELAY(1000);
		}

		if ((status == CDF_STATUS_SUCCESS) && bmi_response_lengthp)
			*bmi_response_lengthp = completed_nbytes;
#else
		if ((status == CDF_STATUS_SUCCESS) && bmi_response_lengthp) {
			*bmi_response_lengthp =
				transaction->bmi_response_length;
		}
#endif

	}

	/* dma_unmap_single(dev, transaction->bmi_request_CE,
		request_length, DMA_TO_DEVICE); */
	/* bus_unmap_single(scn->sc_osdev,
		 transaction->bmi_request_CE,
		request_length, BUS_DMA_TODEVICE); */

	if (status != CDF_STATUS_SUCCESS) {
		cdf_dma_addr_t unused_buffer;
		unsigned int unused_nbytes;
		unsigned int unused_id;
		unsigned int toeplitz_hash_result;

		ce_cancel_send_next(ce_send_hdl,
			NULL, NULL, &unused_buffer,
			&unused_nbytes, &unused_id,
			&toeplitz_hash_result);
	}

	A_TARGET_ACCESS_UNLIKELY(scn);
	cdf_mem_free(transaction);
	return status;
}
