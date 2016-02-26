/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

/*
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef REMOVE_PKT_LOG
#include "qdf_mem.h"
#include "athdefs.h"
#include "pktlog_ac_i.h"
#include "cds_api.h"
#include "wma_types.h"
#include "htc.h"

wdi_event_subscribe PKTLOG_TX_SUBSCRIBER;
wdi_event_subscribe PKTLOG_RX_SUBSCRIBER;
wdi_event_subscribe PKTLOG_RX_REMOTE_SUBSCRIBER;
wdi_event_subscribe PKTLOG_RCFIND_SUBSCRIBER;
wdi_event_subscribe PKTLOG_RCUPDATE_SUBSCRIBER;
wdi_event_subscribe PKTLOG_SW_EVENT_SUBSCRIBER;

struct ol_pl_arch_dep_funcs ol_pl_funcs = {
	.pktlog_init = pktlog_init,
	.pktlog_enable = pktlog_enable,
	.pktlog_setsize = pktlog_setsize,
	.pktlog_disable = pktlog_disable,       /* valid for f/w disable */
};

struct ol_pktlog_dev_t ol_pl_dev = {
	.pl_funcs = &ol_pl_funcs,
};

void ol_pl_sethandle(ol_pktlog_dev_handle *pl_handle,
		     struct hif_opaque_softc *scn)
{
	ol_pl_dev.scn = (ol_ath_generic_softc_handle) scn;
	*pl_handle = &ol_pl_dev;
}

static A_STATUS pktlog_wma_post_msg(WMI_PKTLOG_EVENT event_types,
				    WMI_CMD_ID cmd_id, bool ini_triggered,
				    uint8_t user_triggered)
{
	cds_msg_t msg = { 0 };
	QDF_STATUS status;
	struct ath_pktlog_wmi_params *param;

	param = qdf_mem_malloc(sizeof(struct ath_pktlog_wmi_params));

	if (!param)
		return A_NO_MEMORY;

	param->cmd_id = cmd_id;
	param->pktlog_event = event_types;
	param->ini_triggered = ini_triggered;
	param->user_triggered = user_triggered;

	msg.type = WMA_PKTLOG_ENABLE_REQ;
	msg.bodyptr = param;
	msg.bodyval = 0;

	status = cds_mq_post_message(CDS_MQ_ID_WMA, &msg);

	if (status != QDF_STATUS_SUCCESS) {
		qdf_mem_free(param);
		return A_ERROR;
	}

	return A_OK;
}

static inline A_STATUS
pktlog_enable_tgt(struct hif_opaque_softc *_scn, uint32_t log_state,
		 bool ini_triggered, uint8_t user_triggered)
{
	uint32_t types = 0;

	if (log_state & ATH_PKTLOG_TX)
		types |= WMI_PKTLOG_EVENT_TX;

	if (log_state & ATH_PKTLOG_RX)
		types |= WMI_PKTLOG_EVENT_RX;

	if (log_state & ATH_PKTLOG_RCFIND)
		types |= WMI_PKTLOG_EVENT_RCF;

	if (log_state & ATH_PKTLOG_RCUPDATE)
		types |= WMI_PKTLOG_EVENT_RCU;

	if (log_state & ATH_PKTLOG_SW_EVENT)
		types |= WMI_PKTLOG_EVENT_SW;

	return pktlog_wma_post_msg(types, WMI_PDEV_PKTLOG_ENABLE_CMDID,
				   ini_triggered, user_triggered);
}

static inline A_STATUS
wdi_pktlog_subscribe(struct ol_txrx_pdev_t *txrx_pdev, int32_t log_state)
{
	if (!txrx_pdev) {
		printk("Invalid pdev in %s\n", __func__);
		return A_ERROR;
	}
	if (log_state & ATH_PKTLOG_TX) {
		if (wdi_event_sub(txrx_pdev,
				  &PKTLOG_TX_SUBSCRIBER, WDI_EVENT_TX_STATUS)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RX) {
		if (wdi_event_sub(txrx_pdev,
				  &PKTLOG_RX_SUBSCRIBER, WDI_EVENT_RX_DESC)) {
			return A_ERROR;
		}
		if (wdi_event_sub(txrx_pdev,
				  &PKTLOG_RX_REMOTE_SUBSCRIBER,
				  WDI_EVENT_RX_DESC_REMOTE)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RCFIND) {
		if (wdi_event_sub(txrx_pdev,
				  &PKTLOG_RCFIND_SUBSCRIBER,
				  WDI_EVENT_RATE_FIND)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RCUPDATE) {
		if (wdi_event_sub(txrx_pdev,
				  &PKTLOG_RCUPDATE_SUBSCRIBER,
				  WDI_EVENT_RATE_UPDATE)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_SW_EVENT) {
		if (wdi_event_sub(txrx_pdev,
				  &PKTLOG_SW_EVENT_SUBSCRIBER,
				  WDI_EVENT_SW_EVENT)) {
			return A_ERROR;
		}
	}

	return A_OK;
}

void pktlog_callback(void *pdev, enum WDI_EVENT event, void *log_data)
{
	switch (event) {
	case WDI_EVENT_TX_STATUS:
	{
		/*
		 * process TX message
		 */
		if (process_tx_info(pdev, log_data)) {
			printk("Unable to process TX info\n");
			return;
		}
		break;
	}
	case WDI_EVENT_RX_DESC:
	{
		/*
		 * process RX message for local frames
		 */
		if (process_rx_info(pdev, log_data)) {
			printk("Unable to process RX info\n");
			return;
		}
		break;
	}
	case WDI_EVENT_RX_DESC_REMOTE:
	{
		/*
		 * process RX message for remote frames
		 */
		if (process_rx_info_remote(pdev, log_data)) {
			printk("Unable to process RX info\n");
			return;
		}
		break;
	}
	case WDI_EVENT_RATE_FIND:
	{
		/*
		 * process RATE_FIND message
		 */
		if (process_rate_find(pdev, log_data)) {
			printk("Unable to process RC_FIND info\n");
			return;
		}
		break;
	}
	case WDI_EVENT_RATE_UPDATE:
	{
		/*
		 * process RATE_UPDATE message
		 */
		if (process_rate_update(pdev, log_data)) {
			printk("Unable to process RC_UPDATE\n");
			return;
		}
		break;
	}
	case WDI_EVENT_SW_EVENT:
	{
		/*
		 * process SW EVENT message
		 */
		if (process_sw_event(pdev, log_data)) {
			printk("Unable to process SW_EVENT\n");
			return;
		}
		break;
	}
	default:
		break;
	}
}

A_STATUS
wdi_pktlog_unsubscribe(struct ol_txrx_pdev_t *txrx_pdev, uint32_t log_state)
{
	if (log_state & ATH_PKTLOG_TX) {
		if (wdi_event_unsub(txrx_pdev,
				    &PKTLOG_TX_SUBSCRIBER,
				    WDI_EVENT_TX_STATUS)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RX) {
		if (wdi_event_unsub(txrx_pdev,
				    &PKTLOG_RX_SUBSCRIBER, WDI_EVENT_RX_DESC)) {
			return A_ERROR;
		}
		if (wdi_event_unsub(txrx_pdev,
				    &PKTLOG_RX_REMOTE_SUBSCRIBER,
				    WDI_EVENT_RX_DESC_REMOTE)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RCFIND) {
		if (wdi_event_unsub(txrx_pdev,
				    &PKTLOG_RCFIND_SUBSCRIBER,
				    WDI_EVENT_RATE_FIND)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RCUPDATE) {
		if (wdi_event_unsub(txrx_pdev,
				    &PKTLOG_RCUPDATE_SUBSCRIBER,
				    WDI_EVENT_RATE_UPDATE)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RCUPDATE) {
		if (wdi_event_unsub(txrx_pdev,
				    &PKTLOG_SW_EVENT_SUBSCRIBER,
				    WDI_EVENT_SW_EVENT)) {
			return A_ERROR;
		}
	}
	return A_OK;
}

int pktlog_disable(struct hif_opaque_softc *scn)
{
	struct ol_txrx_pdev_t *txrx_pdev =
		cds_get_context(QDF_MODULE_ID_TXRX);
	struct ol_pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;

	if (txrx_pdev == NULL ||
			txrx_pdev->pl_dev == NULL ||
			txrx_pdev->pl_dev->pl_info == NULL)
		return -EFAULT;

	pl_dev = txrx_pdev->pl_dev;
	pl_info = pl_dev->pl_info;

	if (pktlog_wma_post_msg(0, WMI_PDEV_PKTLOG_DISABLE_CMDID, 0, 0)) {
		printk("Failed to disable pktlog in target\n");
		return -1;
	}

	if (pl_dev->is_pktlog_cb_subscribed &&
		wdi_pktlog_unsubscribe(txrx_pdev, pl_info->log_state)) {
		printk("Cannot unsubscribe pktlog from the WDI\n");
		return -1;
	}
	pl_dev->is_pktlog_cb_subscribed = false;
	return 0;
}

void pktlog_init(struct hif_opaque_softc *scn)
{
	struct ath_pktlog_info *pl_info;
	ol_txrx_pdev_handle pdev_txrx_handle;
	pdev_txrx_handle = cds_get_context(QDF_MODULE_ID_TXRX);

	if (pdev_txrx_handle == NULL ||
			pdev_txrx_handle->pl_dev == NULL ||
			pdev_txrx_handle->pl_dev->pl_info == NULL)
		return;

	pl_info = pdev_txrx_handle->pl_dev->pl_info;

	OS_MEMZERO(pl_info, sizeof(*pl_info));
	PKTLOG_LOCK_INIT(pl_info);

	pl_info->buf_size = PKTLOG_DEFAULT_BUFSIZE;
	pl_info->buf = NULL;
	pl_info->log_state = 0;
	pl_info->sack_thr = PKTLOG_DEFAULT_SACK_THR;
	pl_info->tail_length = PKTLOG_DEFAULT_TAIL_LENGTH;
	pl_info->thruput_thresh = PKTLOG_DEFAULT_THRUPUT_THRESH;
	pl_info->per_thresh = PKTLOG_DEFAULT_PER_THRESH;
	pl_info->phyerr_thresh = PKTLOG_DEFAULT_PHYERR_THRESH;
	pl_info->trigger_interval = PKTLOG_DEFAULT_TRIGGER_INTERVAL;
	pl_info->pktlen = 0;
	pl_info->start_time_thruput = 0;
	pl_info->start_time_per = 0;

	PKTLOG_TX_SUBSCRIBER.callback = pktlog_callback;
	PKTLOG_RX_SUBSCRIBER.callback = pktlog_callback;
	PKTLOG_RX_REMOTE_SUBSCRIBER.callback = pktlog_callback;
	PKTLOG_RCFIND_SUBSCRIBER.callback = pktlog_callback;
	PKTLOG_RCUPDATE_SUBSCRIBER.callback = pktlog_callback;
	PKTLOG_SW_EVENT_SUBSCRIBER.callback = pktlog_callback;
}

int pktlog_enable(struct hif_opaque_softc *scn, int32_t log_state,
		 bool ini_triggered, uint8_t user_triggered)
{
	struct ol_pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	struct ol_txrx_pdev_t *txrx_pdev;
	int error;

	if (!scn) {
		printk("%s: Invalid scn context\n", __func__);
		ASSERT(0);
		return -1;
	}

	txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!txrx_pdev) {
		printk("%s: Invalid txrx_pdev context\n", __func__);
		ASSERT(0);
		return -1;
	}

	pl_dev = txrx_pdev->pl_dev;
	if (!pl_dev) {
		printk("%s: Invalid pktlog context\n", __func__);
		ASSERT(0);
		return -1;
	}

	pl_info = pl_dev->pl_info;

	if (!pl_info)
		return 0;

	if (!pl_dev->tgt_pktlog_alloced) {
		if (pl_info->buf == NULL) {
			error = pktlog_alloc_buf(scn);

			if (error != 0)
				return error;

			if (!pl_info->buf) {
				printk("%s: pktlog buf alloc failed\n",
				       __func__);
				ASSERT(0);
				return -1;
			}

		}

		pl_info->buf->bufhdr.version = CUR_PKTLOG_VER;
		pl_info->buf->bufhdr.magic_num = PKTLOG_MAGIC_NUM;
		pl_info->buf->wr_offset = 0;
		pl_info->buf->rd_offset = -1;
		/* These below variables are used by per packet stats*/
		pl_info->buf->bytes_written = 0;
		pl_info->buf->msg_index = 1;
		pl_info->buf->offset = PKTLOG_READ_OFFSET;

		pl_info->start_time_thruput = os_get_timestamp();
		pl_info->start_time_per = pl_info->start_time_thruput;

		pl_dev->tgt_pktlog_alloced = true;
	}

	if (log_state != 0) {
		/* WDI subscribe */
		if ((!pl_dev->is_pktlog_cb_subscribed) &&
			wdi_pktlog_subscribe(txrx_pdev, log_state)) {
			printk("Unable to subscribe to the WDI %s\n", __func__);
			return -1;
		}
		pl_dev->is_pktlog_cb_subscribed = true;
		/* WMI command to enable pktlog on the firmware */
		if (pktlog_enable_tgt(scn, log_state, ini_triggered,
				user_triggered)) {
			printk("Device cannot be enabled, %s\n", __func__);
			return -1;
		}
	} else {
		pl_dev->pl_funcs->pktlog_disable(scn);
	}

	pl_info->log_state = log_state;
	return 0;
}

int pktlog_setsize(struct hif_opaque_softc *scn, int32_t size)
{
	ol_txrx_pdev_handle pdev_txrx_handle =
		cds_get_context(QDF_MODULE_ID_TXRX);
	struct ol_pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;

	if (pdev_txrx_handle == NULL ||
			pdev_txrx_handle->pl_dev == NULL ||
			pdev_txrx_handle->pl_dev->pl_info == NULL)
		return -EFAULT;

	pl_dev = pdev_txrx_handle->pl_dev;
	pl_info = pl_dev->pl_info;

	if (size < 0)
		return -EINVAL;

	if (size == pl_info->buf_size)
		return 0;

	if (pl_info->log_state) {
		printk
			("Logging should be disabled before changing bufer size\n");
		return -EINVAL;
	}

	if (pl_info->buf != NULL) {
		if (pl_dev->is_pktlog_cb_subscribed &&
			wdi_pktlog_unsubscribe(pdev_txrx_handle,
					 pl_info->log_state)) {
			printk("Cannot unsubscribe pktlog from the WDI\n");
			return -EFAULT;
		}
		pktlog_release_buf(scn);
		pl_dev->is_pktlog_cb_subscribed = false;
		pl_dev->tgt_pktlog_alloced = false;
	}

	if (size != 0)
		pl_info->buf_size = size;

	return 0;
}

/**
 * pktlog_process_fw_msg() - process packetlog message
 * @buff: buffer
 *
 * Return: None
 */
void pktlog_process_fw_msg(uint32_t *buff)
{
	uint32_t *pl_hdr;
	uint32_t log_type;
	struct ol_txrx_pdev_t *txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!txrx_pdev) {
		qdf_print("%s: txrx_pdev is NULL", __func__);
		return;
	}

	pl_hdr = buff;
	log_type =
		(*(pl_hdr + 1) & ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
		ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	if ((log_type == PKTLOG_TYPE_TX_CTRL)
		|| (log_type == PKTLOG_TYPE_TX_STAT)
		|| (log_type == PKTLOG_TYPE_TX_MSDU_ID)
		|| (log_type == PKTLOG_TYPE_TX_FRM_HDR)
		|| (log_type == PKTLOG_TYPE_TX_VIRT_ADDR))
		wdi_event_handler(WDI_EVENT_TX_STATUS,
				  txrx_pdev, pl_hdr);
	else if (log_type == PKTLOG_TYPE_RC_FIND)
		wdi_event_handler(WDI_EVENT_RATE_FIND,
				  txrx_pdev, pl_hdr);
	else if (log_type == PKTLOG_TYPE_RC_UPDATE)
		wdi_event_handler(WDI_EVENT_RATE_UPDATE,
				  txrx_pdev, pl_hdr);
	else if (log_type == PKTLOG_TYPE_RX_STAT)
		wdi_event_handler(WDI_EVENT_RX_DESC,
				  txrx_pdev, pl_hdr);
	else if (log_type == PKTLOG_TYPE_SW_EVENT)
		wdi_event_handler(WDI_EVENT_SW_EVENT,
				  txrx_pdev, pl_hdr);

}

/**
 * pktlog_t2h_msg_handler() - Target to host message handler
 * @context: pdev context
 * @pkt: HTC packet
 *
 * Return: None
 */
void pktlog_t2h_msg_handler(void *context, HTC_PACKET *pkt)
{
	struct ol_pktlog_dev_t *pdev = (struct ol_pktlog_dev_t *)context;
	qdf_nbuf_t pktlog_t2h_msg = (qdf_nbuf_t) pkt->pPktContext;
	uint32_t *msg_word;

	/* check for successful message reception */
	if (pkt->Status != A_OK) {
		if (pkt->Status != A_ECANCELED)
			pdev->htc_err_cnt++;
		qdf_nbuf_free(pktlog_t2h_msg);
		return;
	}

	/* confirm alignment */
	qdf_assert((((unsigned long)qdf_nbuf_data(pktlog_t2h_msg)) & 0x3) == 0);

	msg_word = (uint32_t *) qdf_nbuf_data(pktlog_t2h_msg);
	pktlog_process_fw_msg(msg_word);

	qdf_nbuf_free(pktlog_t2h_msg);
}

/**
 * pktlog_tx_resume_handler() - resume callback
 * @context: pdev context
 *
 * Return: None
 */
void pktlog_tx_resume_handler(void *context)
{
	qdf_print("%s: Not expected", __func__);
	qdf_assert(0);
}

/**
 * pktlog_h2t_send_complete() - send complete indication
 * @context: pdev context
 * @htc_pkt: HTC packet
 *
 * Return: None
 */
void pktlog_h2t_send_complete(void *context, HTC_PACKET *htc_pkt)
{
	qdf_print("%s: Not expected", __func__);
	qdf_assert(0);
}

/**
 * pktlog_h2t_full() - queue full indication
 * @context: pdev context
 * @pkt: HTC packet
 *
 * Return: HTC action
 */
HTC_SEND_FULL_ACTION pktlog_h2t_full(void *context, HTC_PACKET *pkt)
{
	return HTC_SEND_FULL_KEEP;
}

/**
 * pktlog_htc_connect_service() - create new endpoint for packetlog
 * @pdev - pktlog pdev
 *
 * Return: 0 for success/failure
 */
int pktlog_htc_connect_service(struct ol_pktlog_dev_t *pdev)
{
	HTC_SERVICE_CONNECT_REQ connect;
	HTC_SERVICE_CONNECT_RESP response;
	A_STATUS status;

	qdf_mem_set(&connect, sizeof(connect), 0);
	qdf_mem_set(&response, sizeof(response), 0);

	connect.pMetaData = NULL;
	connect.MetaDataLength = 0;
	connect.EpCallbacks.pContext = pdev;
	connect.EpCallbacks.EpTxComplete = pktlog_h2t_send_complete;
	connect.EpCallbacks.EpTxCompleteMultiple = NULL;
	connect.EpCallbacks.EpRecv = pktlog_t2h_msg_handler;
	connect.EpCallbacks.ep_resume_tx_queue = pktlog_tx_resume_handler;

	/* rx buffers currently are provided by HIF, not by EpRecvRefill */
	connect.EpCallbacks.EpRecvRefill = NULL;
	connect.EpCallbacks.RecvRefillWaterMark = 1;
	/* N/A, fill is done by HIF */

	connect.EpCallbacks.EpSendFull = pktlog_h2t_full;
	/*
	 * Specify how deep to let a queue get before htc_send_pkt will
	 * call the EpSendFull function due to excessive send queue depth.
	 */
	connect.MaxSendQueueDepth = PKTLOG_MAX_SEND_QUEUE_DEPTH;

	/* disable flow control for HTT data message service */
	connect.ConnectionFlags |= HTC_CONNECT_FLAGS_DISABLE_CREDIT_FLOW_CTRL;

	/* connect to control service */
	connect.service_id = PACKET_LOG_SVC;

	status = htc_connect_service(pdev->htc_pdev, &connect, &response);

	if (status != A_OK) {
		pdev->mt_pktlog_enabled = false;
		return -EIO;       /* failure */
	}

	pdev->htc_endpoint = response.Endpoint;
	pdev->mt_pktlog_enabled = true;

	return 0;               /* success */
}

#if defined(QCA_WIFI_3_0_ADRASTEA)
/**
 * pktlog_htc_attach() - attach pktlog HTC service
 *
 * Return: 0 for success/failure
 */
int pktlog_htc_attach(void)
{
	struct ol_txrx_pdev_t *txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	struct ol_pktlog_dev_t *pdev = NULL;
	void *htc_pdev = cds_get_context(QDF_MODULE_ID_HTC);

	if ((!txrx_pdev) || (!txrx_pdev->pl_dev) || (!htc_pdev))
		return -EINVAL;

	pdev = txrx_pdev->pl_dev;
	pdev->htc_pdev = htc_pdev;
	return pktlog_htc_connect_service(pdev);
}
#else
int pktlog_htc_attach(void)
{
	struct ol_txrx_pdev_t *txrx_pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	struct ol_pktlog_dev_t *pdev = NULL;

	if (!txrx_pdev)
		return -EINVAL;
	pdev = txrx_pdev->pl_dev;
	pdev->mt_pktlog_enabled = false;
	return 0;
}
#endif
#endif /* REMOVE_PKT_LOG */
