/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
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

#define ATH_MODULE_NAME hif
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

/**
 * hif_dev_rw_completion_handler() - Completion routine
 * for ALL HIF layer async I/O
 * @context: hif send context
 * @status: completion routine sync/async context
 *
 * Return: 0 for success and non-zero for failure
 */
QDF_STATUS hif_dev_rw_completion_handler(void *context, QDF_STATUS status)
{
	struct hif_sendContext *send_context =
				(struct hif_sendContext *)context;
	unsigned int transfer_id = send_context->transferID;
	struct hif_sdio_device *pdev = send_context->pDev;
	qdf_nbuf_t buf = send_context->netbuf;
	/* Fix Me: Do we need toeplitz_hash_result for SDIO */
	uint32_t toeplitz_hash_result = 0;

	if (send_context->bNewAlloc)
		qdf_mem_free((void *)send_context);
	else
		qdf_nbuf_pull_head(buf, send_context->head_data_len);
	if (pdev->hif_callbacks.txCompletionHandler)
		pdev->hif_callbacks.txCompletionHandler(pdev->hif_callbacks.
					Context, buf,
					transfer_id, toeplitz_hash_result);

	return QDF_STATUS_SUCCESS;
}

/**
 * hif_dev_send_buffer() - send buffer to sdio device
 * @pDev: sdio function
 * @transferID: transfer id
 * @pipe: ul/dl pipe
 * @nbytes: no of bytes to transfer
 * @buf: pointer to buffer
 *
 * Return: 0 for success and non-zero for failure
 */
QDF_STATUS hif_dev_send_buffer(struct hif_sdio_device *pdev,
			       unsigned int transfer_id,
			       uint8_t pipe, unsigned int nbytes,
			       qdf_nbuf_t buf)
{
	QDF_STATUS status;
	uint32_t padded_length;
	int frag_count = 0, i, head_data_len;
	struct hif_sendContext *send_context;
	unsigned char *pData;
	uint32_t request = HIF_WR_ASYNC_BLOCK_INC;
	uint8_t mbox_index = hif_dev_map_pipe_to_mail_box(pdev, pipe);

	if (mbox_index == INVALID_MAILBOX_NUMBER) {
		AR_DEBUG_PRINTF(ATH_DEBUG_ERR, ("pipe id(%d) invalid\n", pipe));
		return QDF_STATUS_E_FAILURE;
	}

	padded_length = DEV_CALC_SEND_PADDED_LEN(pdev, nbytes);
	A_ASSERT(padded_length - nbytes < HIF_DUMMY_SPACE_MASK + 1);
	/*
	 * two most significant bytes to save dummy data count
	 * data written into the dummy space will not put into
	 * the final mbox FIFO.
	 */
	request |= ((padded_length - nbytes) << 16);

	frag_count = qdf_nbuf_get_num_frags(buf);

	if (frag_count > 1) {
		/* header data length should be total sending length substract
		 * internal data length of netbuf
		 */
		head_data_len = sizeof(struct hif_sendContext) +
			(nbytes - qdf_nbuf_get_frag_len(buf, frag_count - 1));
	} else {
		/*
		 * | hif_sendContext | netbuf->data
		 */
		head_data_len = sizeof(struct hif_sendContext);
	}

	/* Check whether head room is enough to save extra head data */
	if ((head_data_len <= qdf_nbuf_headroom(buf)) &&
	    (qdf_nbuf_tailroom(buf) >= (padded_length - nbytes))) {
		send_context =
			(struct hif_sendContext *)qdf_nbuf_push_head(buf,
						     head_data_len);
		send_context->bNewAlloc = false;
	} else {
		send_context =
			(struct hif_sendContext *)
			qdf_mem_malloc(sizeof(struct hif_sendContext) +
				       padded_length);
		if (send_context) {
			send_context->bNewAlloc = true;
		} else {
			AR_DEBUG_PRINTF(ATH_DEBUG_ERR,
				("Allocate send context fail %d\n",
				sizeof(struct hif_sendContext) +
				padded_length));
			return QDF_STATUS_E_NOMEM;
		}
	}

	send_context->netbuf = buf;
	send_context->pDev = pdev;
	send_context->transferID = transfer_id;
	send_context->head_data_len = head_data_len;
	/*
	 * Copy data to head part of netbuf or head of allocated buffer.
	 * if buffer is new allocated, the last buffer should be copied also.
	 * It assume last fragment is internal buffer of netbuf
	 * sometime total length of fragments larger than nbytes
	 */
	pData = (unsigned char *)send_context + sizeof(struct hif_sendContext);
	for (i = 0; i < (send_context->bNewAlloc ? frag_count : frag_count - 1);
	     i++) {
		int frag_len = qdf_nbuf_get_frag_len(buf, i);
		unsigned char *frag_addr = qdf_nbuf_get_frag_vaddr(buf, i);

		if (frag_len > nbytes)
			frag_len = nbytes;
		memcpy(pData, frag_addr, frag_len);
		pData += frag_len;
		nbytes -= frag_len;
		if (nbytes <= 0)
			break;
	}

	/* Reset pData pointer and send_context out */
	pData = (unsigned char *)send_context + sizeof(struct hif_sendContext);
	status = hif_read_write(pdev->HIFDevice,
				pdev->MailBoxInfo.mbox_prop[mbox_index].
				extended_address, (char *)pData, padded_length,
				request, (void *)send_context);

	if (status == QDF_STATUS_E_PENDING)
		/*
		 * it will return QDF_STATUS_E_PENDING in native HIF
		 * implementation, which should be treated as successful
		 * result here.
		 */
		status = QDF_STATUS_SUCCESS;
	/* release buffer or move back data pointer when failed */
	if (status != QDF_STATUS_SUCCESS) {
		if (send_context->bNewAlloc)
			qdf_mem_free(send_context);
		else
			qdf_nbuf_pull_head(buf, head_data_len);
	}

	return status;
}
