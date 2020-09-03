/*
 * Copyright (c) 2012-2020 The Linux Foundation. All rights reserved.
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
#include <cdp_txrx_cmn_struct.h>
#include <cdp_txrx_ctrl.h>
#ifdef PKTLOG_LEGACY
#include "pktlog_wifi2.h"
#else
#include "pktlog_wifi3.h"
#endif /* PKTLOG_LEGACY */

wdi_event_subscribe PKTLOG_TX_SUBSCRIBER;
wdi_event_subscribe PKTLOG_RX_SUBSCRIBER;
wdi_event_subscribe PKTLOG_RX_REMOTE_SUBSCRIBER;
wdi_event_subscribe PKTLOG_RCFIND_SUBSCRIBER;
wdi_event_subscribe PKTLOG_RCUPDATE_SUBSCRIBER;
wdi_event_subscribe PKTLOG_SW_EVENT_SUBSCRIBER;
wdi_event_subscribe PKTLOG_LITE_T2H_SUBSCRIBER;
wdi_event_subscribe PKTLOG_LITE_RX_SUBSCRIBER;
wdi_event_subscribe PKTLOG_OFFLOAD_SUBSCRIBER;

struct ol_pl_arch_dep_funcs ol_pl_funcs = {
	.pktlog_init = pktlog_init,
	.pktlog_enable = pktlog_enable,
	.pktlog_setsize = pktlog_setsize,
	.pktlog_disable = pktlog_disable,       /* valid for f/w disable */
};

struct pktlog_dev_t pl_dev = {
	.pl_funcs = &ol_pl_funcs,
};

void pktlog_sethandle(struct pktlog_dev_t **pl_handle,
		     struct hif_opaque_softc *scn)
{
	pl_dev.scn = (ol_ath_generic_softc_handle) scn;
	*pl_handle = &pl_dev;
}

void pktlog_set_pdev_id(struct pktlog_dev_t *pl_dev, uint8_t pdev_id)
{
	pl_dev->pdev_id = pdev_id;
}

void pktlog_set_callback_regtype(
		enum pktlog_callback_regtype callback_type)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		qdf_print("Invalid pl_dev");
		return;
	}

	pl_dev->callback_type = callback_type;
}

struct pktlog_dev_t *get_pktlog_handle(void)
{
	uint8_t pdev_id = WMI_PDEV_ID_SOC;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	return cdp_get_pldev(soc, pdev_id);
}

static A_STATUS pktlog_wma_post_msg(WMI_PKTLOG_EVENT event_types,
				    WMI_CMD_ID cmd_id, bool ini_triggered,
				    uint8_t user_triggered)
{
	struct scheduler_msg msg = { 0 };
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

	status = scheduler_post_message(QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &msg);

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

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Pktlog events: %d", __func__, types);

	return pktlog_wma_post_msg(types, WMI_PDEV_PKTLOG_ENABLE_CMDID,
				   ini_triggered, user_triggered);
}

#ifdef PKTLOG_LEGACY
/**
 * wdi_pktlog_subscribe() - Subscribe pktlog callbacks
 * @pdev_id: pdev id
 * @log_state: Pktlog registration
 *
 * Return: zero on success, non-zero on failure
 */
static inline A_STATUS
wdi_pktlog_subscribe(uint8_t pdev_id, int32_t log_state)
{
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (pdev_id < 0) {
		qdf_print("Invalid pdev");
		return A_ERROR;
	}

	if (log_state & ATH_PKTLOG_TX) {
		if (cdp_wdi_event_sub(soc, pdev_id, &PKTLOG_TX_SUBSCRIBER,
				      WDI_EVENT_TX_STATUS)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RX) {
		if (cdp_wdi_event_sub(soc, pdev_id, &PKTLOG_RX_SUBSCRIBER,
				      WDI_EVENT_RX_DESC)) {
			return A_ERROR;
		}
		if (cdp_wdi_event_sub(soc, pdev_id,
				      &PKTLOG_RX_REMOTE_SUBSCRIBER,
				      WDI_EVENT_RX_DESC_REMOTE)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RCFIND) {
		if (cdp_wdi_event_sub(soc, pdev_id,
				      &PKTLOG_RCFIND_SUBSCRIBER,
				      WDI_EVENT_RATE_FIND)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RCUPDATE) {
		if (cdp_wdi_event_sub(soc, pdev_id,
				      &PKTLOG_RCUPDATE_SUBSCRIBER,
				      WDI_EVENT_RATE_UPDATE)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_SW_EVENT) {
		if (cdp_wdi_event_sub(soc, pdev_id,
				      &PKTLOG_SW_EVENT_SUBSCRIBER,
				      WDI_EVENT_SW_EVENT)) {
			return A_ERROR;
		}
	}

	return A_OK;
}
#else
static inline A_STATUS
wdi_pktlog_subscribe(uint8_t pdev_id, int32_t log_state)
{
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if (pdev_id < 0) {
		qdf_print("Invalid pdev");
		return A_ERROR;
	}

	if ((log_state & ATH_PKTLOG_TX) ||
	    (log_state  & ATH_PKTLOG_RCFIND) ||
	    (log_state & ATH_PKTLOG_RCUPDATE) ||
	    (log_state & ATH_PKTLOG_SW_EVENT)) {
		if (cdp_wdi_event_sub(soc,
				      pdev_id,
				      &PKTLOG_OFFLOAD_SUBSCRIBER,
				      WDI_EVENT_OFFLOAD_ALL)) {
			return A_ERROR;
		}
	}

	if (log_state & ATH_PKTLOG_RX) {
		if (cdp_wdi_event_sub(soc, pdev_id,
				      &PKTLOG_RX_SUBSCRIBER,
				      WDI_EVENT_RX_DESC)) {
			return A_ERROR;
		}
	}

	if (log_state & ATH_PKTLOG_SW_EVENT) {
		if (cdp_wdi_event_sub(soc, pdev_id,
				      &PKTLOG_SW_EVENT_SUBSCRIBER,
				      WDI_EVENT_SW_EVENT)) {
			return A_ERROR;
		}
	}

	if (log_state & ATH_PKTLOG_LITE_T2H) {
		if (cdp_wdi_event_sub(soc, pdev_id,
				      &PKTLOG_LITE_T2H_SUBSCRIBER,
				      WDI_EVENT_LITE_T2H)) {
			return A_ERROR;
		}
	}

	if (log_state & ATH_PKTLOG_LITE_RX) {
		if (cdp_wdi_event_sub(soc, pdev_id,
				      &PKTLOG_LITE_RX_SUBSCRIBER,
				      WDI_EVENT_LITE_RX)) {
			return A_ERROR;
		}
	}

	return A_OK;
}
#endif

void pktlog_callback(void *pdev, enum WDI_EVENT event, void *log_data,
		u_int16_t peer_id, uint32_t status)
{
	switch (event) {
	case WDI_EVENT_OFFLOAD_ALL:
	{
		if (process_offload_pktlog_wifi3(pdev, log_data)) {
			qdf_print("Unable to process offload info");
			return;
		}
		break;
	}
	case WDI_EVENT_TX_STATUS:
	{
		/*
		 * process TX message
		 */
		if (process_tx_info(pdev, log_data)) {
			qdf_print("Unable to process TX info");
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
			qdf_print("Unable to process RX info");
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
			qdf_print("Unable to process RX info");
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
			qdf_print("Unable to process RC_FIND info");
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
			qdf_print("Unable to process RC_UPDATE");
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
			qdf_print("Unable to process SW_EVENT");
			return;
		}
		break;
	}
	default:
		break;
	}
}

void
lit_pktlog_callback(void *context, enum WDI_EVENT event, void *log_data,
			u_int16_t peer_id, uint32_t status)
{
	switch (event) {
	case WDI_EVENT_RX_DESC:
	{
		if (process_rx_desc_remote_wifi3(context, log_data)) {
			qdf_print("Unable to process RX info");
			return;
		}
		break;
	}
	case WDI_EVENT_LITE_T2H:
	{
		if (process_pktlog_lite_wifi3(context, log_data,
					      PKTLOG_TYPE_LITE_T2H)) {
			qdf_print("Unable to process lite_t2h");
			return;
		}
		break;
	}
	case WDI_EVENT_LITE_RX:
	{
		if (process_pktlog_lite_wifi3(context, log_data,
					      PKTLOG_TYPE_LITE_RX)) {
			qdf_print("Unable to process lite_rx");
			return;
		}
		break;
	}
	default:
		break;
	}
}

#ifdef PKTLOG_LEGACY
A_STATUS
wdi_pktlog_unsubscribe(uint8_t pdev_id, uint32_t log_state)
{
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	/* TODO: WIN implementation to get soc */

	if (log_state & ATH_PKTLOG_TX) {
		if (cdp_wdi_event_unsub(soc, pdev_id,
					&PKTLOG_TX_SUBSCRIBER,
					WDI_EVENT_TX_STATUS)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RX) {
		if (cdp_wdi_event_unsub(soc, pdev_id,
					&PKTLOG_RX_SUBSCRIBER,
					WDI_EVENT_RX_DESC)) {
			return A_ERROR;
		}
		if (cdp_wdi_event_unsub(soc, pdev_id,
					&PKTLOG_RX_REMOTE_SUBSCRIBER,
					WDI_EVENT_RX_DESC_REMOTE)) {
			return A_ERROR;
		}
	}

	if (log_state & ATH_PKTLOG_RCFIND) {
		if (cdp_wdi_event_unsub(soc, pdev_id,
					&PKTLOG_RCFIND_SUBSCRIBER,
					WDI_EVENT_RATE_FIND)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RCUPDATE) {
		if (cdp_wdi_event_unsub(soc, pdev_id,
					&PKTLOG_RCUPDATE_SUBSCRIBER,
					WDI_EVENT_RATE_UPDATE)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RCUPDATE) {
		if (cdp_wdi_event_unsub(soc, pdev_id,
					&PKTLOG_SW_EVENT_SUBSCRIBER,
					WDI_EVENT_SW_EVENT)) {
			return A_ERROR;
		}
	}

	return A_OK;
}
#else
A_STATUS
wdi_pktlog_unsubscribe(uint8_t pdev_id, uint32_t log_state)
{
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	if ((log_state & ATH_PKTLOG_TX) ||
	    (log_state  & ATH_PKTLOG_RCFIND) ||
	    (log_state & ATH_PKTLOG_RCUPDATE) ||
	    (log_state & ATH_PKTLOG_SW_EVENT)) {
		if (cdp_wdi_event_unsub(soc,
					pdev_id,
					&PKTLOG_OFFLOAD_SUBSCRIBER,
					WDI_EVENT_OFFLOAD_ALL)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_RX) {
		if (cdp_wdi_event_unsub(soc, pdev_id,
					&PKTLOG_RX_SUBSCRIBER,
					WDI_EVENT_RX_DESC)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_LITE_T2H) {
		if (cdp_wdi_event_unsub(soc, pdev_id,
					&PKTLOG_LITE_T2H_SUBSCRIBER,
					WDI_EVENT_LITE_T2H)) {
			return A_ERROR;
		}
	}
	if (log_state & ATH_PKTLOG_LITE_RX) {
		if (cdp_wdi_event_unsub(soc, pdev_id,
					&PKTLOG_LITE_RX_SUBSCRIBER,
					WDI_EVENT_LITE_RX)) {
			return A_ERROR;
		}
	}

	return A_OK;
}
#endif

int pktlog_disable(struct hif_opaque_softc *scn)
{
	struct pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	uint8_t save_pktlog_state;
	uint8_t pdev_id = WMI_PDEV_ID_SOC;

	pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		qdf_print("Invalid pl_dev");
		return -EINVAL;
	}

	pl_info = pl_dev->pl_info;

	if (!pl_dev->pl_info) {
		qdf_print("Invalid pl_info");
		return -EINVAL;
	}

	if (pdev_id < 0) {
		qdf_print("Invalid pdev");
		return -EINVAL;
	}

	if (pl_info->curr_pkt_state == PKTLOG_OPR_IN_PROGRESS ||
	    pl_info->curr_pkt_state ==
			PKTLOG_OPR_IN_PROGRESS_READ_START_PKTLOG_DISABLED ||
	    pl_info->curr_pkt_state == PKTLOG_OPR_IN_PROGRESS_READ_COMPLETE ||
	    pl_info->curr_pkt_state ==
			PKTLOG_OPR_IN_PROGRESS_CLEARBUFF_COMPLETE)
		return -EBUSY;

	save_pktlog_state = pl_info->curr_pkt_state;
	pl_info->curr_pkt_state = PKTLOG_OPR_IN_PROGRESS;

	if (pktlog_wma_post_msg(0, WMI_PDEV_PKTLOG_DISABLE_CMDID, 0, 0)) {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("Failed to disable pktlog in target");
		return -EINVAL;
	}

	if (pl_dev->is_pktlog_cb_subscribed &&
		wdi_pktlog_unsubscribe(pdev_id, pl_info->log_state)) {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("Cannot unsubscribe pktlog from the WDI");
		return -EINVAL;
	}
	pl_dev->is_pktlog_cb_subscribed = false;
	if (save_pktlog_state == PKTLOG_OPR_IN_PROGRESS_READ_START)
		pl_info->curr_pkt_state =
			PKTLOG_OPR_IN_PROGRESS_READ_START_PKTLOG_DISABLED;
	else
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
	return 0;
}

#ifdef PKTLOG_LEGACY
/**
 * pktlog_callback_registration() - Register pktlog handlers based on
 *                                  on callback type
 * @callback_type: pktlog full or lite registration
 *
 * Return: None
 */
static void pktlog_callback_registration(uint8_t callback_type)
{
	if (callback_type == PKTLOG_DEFAULT_CALLBACK_REGISTRATION) {
		PKTLOG_TX_SUBSCRIBER.callback = pktlog_callback;
		PKTLOG_RX_SUBSCRIBER.callback = pktlog_callback;
		PKTLOG_RX_REMOTE_SUBSCRIBER.callback = pktlog_callback;
		PKTLOG_RCFIND_SUBSCRIBER.callback = pktlog_callback;
		PKTLOG_RCUPDATE_SUBSCRIBER.callback = pktlog_callback;
		PKTLOG_SW_EVENT_SUBSCRIBER.callback = pktlog_callback;
	}
}
#else
static void pktlog_callback_registration(uint8_t callback_type)
{
	if (callback_type == PKTLOG_DEFAULT_CALLBACK_REGISTRATION) {
		PKTLOG_RX_SUBSCRIBER.callback = lit_pktlog_callback;
		PKTLOG_LITE_T2H_SUBSCRIBER.callback = lit_pktlog_callback;
		PKTLOG_OFFLOAD_SUBSCRIBER.callback = pktlog_callback;
	} else if (callback_type == PKTLOG_LITE_CALLBACK_REGISTRATION) {
		PKTLOG_LITE_T2H_SUBSCRIBER.callback = lit_pktlog_callback;
		PKTLOG_LITE_RX_SUBSCRIBER.callback = lit_pktlog_callback;
	}
}
#endif

#define ONE_MEGABYTE (1024 * 1024)

void pktlog_init(struct hif_opaque_softc *scn)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();
	struct ath_pktlog_info *pl_info;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint32_t buff_size;

	if (!pl_dev || !pl_dev->pl_info) {
		qdf_print("pl_dev or pl_info is invalid");
		return;
	}

	pl_info = pl_dev->pl_info;

	OS_MEMZERO(pl_info, sizeof(*pl_info));
	PKTLOG_LOCK_INIT(pl_info);
	mutex_init(&pl_info->pktlog_mutex);

	buff_size = cdp_cfg_get(soc, cfg_dp_pktlog_buffer_size) * ONE_MEGABYTE;

	pl_info->buf_size = (buff_size ? buff_size : ONE_MEGABYTE);
	pl_info->buf = NULL;
	pl_info->log_state = 0;
	pl_info->init_saved_state = 0;
	pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
	pl_info->sack_thr = PKTLOG_DEFAULT_SACK_THR;
	pl_info->tail_length = PKTLOG_DEFAULT_TAIL_LENGTH;
	pl_info->thruput_thresh = PKTLOG_DEFAULT_THRUPUT_THRESH;
	pl_info->per_thresh = PKTLOG_DEFAULT_PER_THRESH;
	pl_info->phyerr_thresh = PKTLOG_DEFAULT_PHYERR_THRESH;
	pl_info->trigger_interval = PKTLOG_DEFAULT_TRIGGER_INTERVAL;
	pl_info->pktlen = 0;
	pl_info->start_time_thruput = 0;
	pl_info->start_time_per = 0;
	pl_dev->vendor_cmd_send = false;

	pktlog_callback_registration(pl_dev->callback_type);
}

int __pktlog_enable(struct hif_opaque_softc *scn, int32_t log_state,
		    bool ini_triggered, uint8_t user_triggered,
		    uint32_t is_iwpriv_command)
{
	struct pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	uint8_t pdev_id;
	int error;

	if (!scn) {
		qdf_print("Invalid scn context");
		ASSERT(0);
		return -EINVAL;
	}

	pl_dev = get_pktlog_handle();
	if (!pl_dev) {
		qdf_print("Invalid pktlog context");
		ASSERT(0);
		return -EINVAL;
	}

	pdev_id = WMI_PDEV_ID_SOC;
	if (pdev_id < 0) {
		qdf_print("Invalid txrx context");
		ASSERT(0);
		return -EINVAL;
	}

	pl_info = pl_dev->pl_info;
	if (!pl_info) {
		qdf_print("Invalid pl_info context");
		ASSERT(0);
		return -EINVAL;
	}

	if (pl_info->curr_pkt_state < PKTLOG_OPR_IN_PROGRESS_CLEARBUFF_COMPLETE)
		return -EBUSY;

	pl_info->curr_pkt_state = PKTLOG_OPR_IN_PROGRESS;
	/* is_iwpriv_command : 0 indicates its a vendor command
	 * log_state: 0 indicates pktlog disable command
	 * vendor_cmd_send flag; false means no vendor pktlog enable
	 * command was sent previously
	 */
	if (is_iwpriv_command == 0 && log_state == 0 &&
	    pl_dev->vendor_cmd_send == false) {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("pktlog operation not in progress");
		return 0;
	}

	if (!pl_dev->tgt_pktlog_alloced) {
		if (!pl_info->buf) {
			error = pktlog_alloc_buf(scn);

			if (error != 0) {
				pl_info->curr_pkt_state =
					PKTLOG_OPR_NOT_IN_PROGRESS;
				qdf_print("pktlog buff alloc failed");
				return -ENOMEM;
			}

			if (!pl_info->buf) {
				pl_info->curr_pkt_state =
					PKTLOG_OPR_NOT_IN_PROGRESS;
				qdf_print("pktlog buf alloc failed");
				ASSERT(0);
				return -ENOMEM;
			}

		}

		qdf_spin_lock_bh(&pl_info->log_lock);
		pl_info->buf->bufhdr.version = CUR_PKTLOG_VER;
		pl_info->buf->bufhdr.magic_num = PKTLOG_MAGIC_NUM;
		pl_info->buf->wr_offset = 0;
		pl_info->buf->rd_offset = -1;
		/* These below variables are used by per packet stats*/
		pl_info->buf->bytes_written = 0;
		pl_info->buf->msg_index = 1;
		pl_info->buf->offset = PKTLOG_READ_OFFSET;
		qdf_spin_unlock_bh(&pl_info->log_lock);

		pl_info->start_time_thruput = os_get_timestamp();
		pl_info->start_time_per = pl_info->start_time_thruput;

		pl_dev->tgt_pktlog_alloced = true;
	}
	if (log_state != 0) {
		/* WDI subscribe */
		if (!pl_dev->is_pktlog_cb_subscribed) {
			error = wdi_pktlog_subscribe(pdev_id, log_state);
			if (error) {
				pl_info->curr_pkt_state =
						PKTLOG_OPR_NOT_IN_PROGRESS;
				qdf_print("Unable to subscribe to the WDI");
				return -EINVAL;
			}
		} else {
			pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
			qdf_print("Unable to subscribe %d to the WDI",
				  log_state);
			return -EINVAL;
		}
		/* WMI command to enable pktlog on the firmware */
		if (pktlog_enable_tgt(scn, log_state, ini_triggered,
				user_triggered)) {
			pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
			qdf_print("Device cannot be enabled");
			return -EINVAL;
		}
		pl_dev->is_pktlog_cb_subscribed = true;

		if (is_iwpriv_command == 0)
			pl_dev->vendor_cmd_send = true;
	} else {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		pl_dev->pl_funcs->pktlog_disable(scn);
		if (is_iwpriv_command == 0)
			pl_dev->vendor_cmd_send = false;
	}

	pl_info->log_state = log_state;
	pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
	return 0;
}

int pktlog_enable(struct hif_opaque_softc *scn, int32_t log_state,
		 bool ini_triggered, uint8_t user_triggered,
		 uint32_t is_iwpriv_command)
{
	struct pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	int err;

	pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		qdf_print("Invalid pl_dev handle");
		return -EINVAL;
	}

	pl_info = pl_dev->pl_info;

	if (!pl_info) {
		qdf_print("Invalid pl_info handle");
		return -EINVAL;
	}

	mutex_lock(&pl_info->pktlog_mutex);
	err = __pktlog_enable(scn, log_state, ini_triggered,
				user_triggered, is_iwpriv_command);
	mutex_unlock(&pl_info->pktlog_mutex);
	return err;
}

static int __pktlog_setsize(struct hif_opaque_softc *scn, int32_t size)
{
	struct pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	uint8_t pdev_id = WMI_PDEV_ID_SOC;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	uint32_t buff_size;
	uint32_t max_allowed_buff_size;

	pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		qdf_print("Invalid pl_dev handle");
		return -EINVAL;
	}

	pl_info = pl_dev->pl_info;

	if (!pl_info) {
		qdf_print("Invalid pl_dev handle");
		return -EINVAL;
	}

	if (pdev_id < 0) {
		qdf_print("Invalid pdev");
		return -EINVAL;
	}

	if (pl_info->curr_pkt_state < PKTLOG_OPR_NOT_IN_PROGRESS) {
		qdf_print("pktlog is not configured");
		return -EBUSY;
	}

	pl_info->curr_pkt_state = PKTLOG_OPR_IN_PROGRESS;

	buff_size = cdp_cfg_get(soc, cfg_dp_pktlog_buffer_size) * ONE_MEGABYTE;
	max_allowed_buff_size = (buff_size ? buff_size : ONE_MEGABYTE);

	if (size < ONE_MEGABYTE || size > max_allowed_buff_size) {
		qdf_print("Cannot Set Pktlog Buffer size of %d bytes.Min required is %d MB and Max allowed is %d MB",
			  size, (ONE_MEGABYTE / ONE_MEGABYTE),
			  (max_allowed_buff_size / ONE_MEGABYTE));
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("Invalid requested buff size");
		return -EINVAL;
	}

	if (size == pl_info->buf_size) {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("Pktlog Buff Size is already of same size");
		return 0;
	}

	if (pl_info->log_state) {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("Logging should be disabled before changing buffer size");
		return -EINVAL;
	}

	qdf_spin_lock_bh(&pl_info->log_lock);
	if (pl_info->buf) {
		if (pl_dev->is_pktlog_cb_subscribed &&
			wdi_pktlog_unsubscribe(pdev_id, pl_info->log_state)) {
			pl_info->curr_pkt_state =
				PKTLOG_OPR_NOT_IN_PROGRESS;
			qdf_spin_unlock_bh(&pl_info->log_lock);
			qdf_print("Cannot unsubscribe pktlog from the WDI");
			return -EFAULT;
		}
		pktlog_release_buf(scn);
		pl_dev->is_pktlog_cb_subscribed = false;
		pl_dev->tgt_pktlog_alloced = false;
	}

	if (size != 0) {
		qdf_print("New Pktlog Buff Size is %d", size);
		pl_info->buf_size = size;
	}
	pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
	qdf_spin_unlock_bh(&pl_info->log_lock);
	return 0;
}

int pktlog_setsize(struct hif_opaque_softc *scn, int32_t size)
{
	struct pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	int status;

	pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		qdf_print("Invalid pl_dev handle");
		return -EINVAL;
	}

	pl_info = pl_dev->pl_info;

	if (!pl_info) {
		qdf_print("Invalid pl_dev handle");
		return -EINVAL;
	}

	mutex_lock(&pl_info->pktlog_mutex);
	status = __pktlog_setsize(scn, size);
	mutex_unlock(&pl_info->pktlog_mutex);

	return status;
}

int pktlog_clearbuff(struct hif_opaque_softc *scn, bool clear_buff)
{
	struct pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	uint8_t save_pktlog_state;

	pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		qdf_print("Invalid pl_dev handle");
		return -EINVAL;
	}

	pl_info = pl_dev->pl_info;

	if (!pl_info) {
		qdf_print("Invalid pl_dev handle");
		return -EINVAL;
	}

	if (!clear_buff)
		return -EINVAL;

	if (pl_info->curr_pkt_state < PKTLOG_OPR_IN_PROGRESS_READ_COMPLETE ||
	    pl_info->curr_pkt_state ==
				PKTLOG_OPR_IN_PROGRESS_CLEARBUFF_COMPLETE)
		return -EBUSY;

	save_pktlog_state = pl_info->curr_pkt_state;
	pl_info->curr_pkt_state = PKTLOG_OPR_IN_PROGRESS;

	if (pl_info->log_state) {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("Logging should be disabled before clearing pktlog buffer");
		return -EINVAL;
	}

	if (pl_info->buf) {
		if (pl_info->buf_size > 0) {
			qdf_debug("pktlog buffer is cleared");
			memset(pl_info->buf, 0, pl_info->buf_size);
			pl_dev->is_pktlog_cb_subscribed = false;
			pl_dev->tgt_pktlog_alloced = false;
			pl_info->buf->rd_offset = -1;
		} else {
			pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
			qdf_print("pktlog buffer size is not proper. "
				  "Existing Buf size %d",
				  pl_info->buf_size);
			return -EFAULT;
		}
	} else {
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;
		qdf_print("pktlog buff is NULL");
		return -EFAULT;
	}

	if (save_pktlog_state == PKTLOG_OPR_IN_PROGRESS_READ_COMPLETE)
		pl_info->curr_pkt_state =
			PKTLOG_OPR_IN_PROGRESS_CLEARBUFF_COMPLETE;
	else
		pl_info->curr_pkt_state = PKTLOG_OPR_NOT_IN_PROGRESS;

	return 0;
}

void pktlog_process_fw_msg(uint8_t pdev_id, uint32_t *buff, uint32_t len)
{
	uint32_t *pl_hdr;
	uint32_t log_type;
	struct ol_fw_data pl_fw_data;

	if (pdev_id == OL_TXRX_INVALID_PDEV_ID) {
		qdf_print("txrx pdev_id is invalid");
		return;
	}
	pl_hdr = buff;
	pl_fw_data.data = pl_hdr;
	pl_fw_data.len = len;

	log_type =
		(*(pl_hdr + 1) & ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
		ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;

	if ((log_type == PKTLOG_TYPE_TX_CTRL)
		|| (log_type == PKTLOG_TYPE_TX_STAT)
		|| (log_type == PKTLOG_TYPE_TX_MSDU_ID)
		|| (log_type == PKTLOG_TYPE_TX_FRM_HDR)
		|| (log_type == PKTLOG_TYPE_TX_VIRT_ADDR))
		wdi_event_handler(WDI_EVENT_TX_STATUS,
				  pdev_id, &pl_fw_data);
	else if (log_type == PKTLOG_TYPE_RC_FIND)
		wdi_event_handler(WDI_EVENT_RATE_FIND,
				  pdev_id, &pl_fw_data);
	else if (log_type == PKTLOG_TYPE_RC_UPDATE)
		wdi_event_handler(WDI_EVENT_RATE_UPDATE,
				  pdev_id, &pl_fw_data);
	else if (log_type == PKTLOG_TYPE_RX_STAT)
		wdi_event_handler(WDI_EVENT_RX_DESC,
				  pdev_id, &pl_fw_data);
	else if (log_type == PKTLOG_TYPE_SW_EVENT)
		wdi_event_handler(WDI_EVENT_SW_EVENT,
				  pdev_id, &pl_fw_data);
}

#if defined(QCA_WIFI_3_0_ADRASTEA)
static inline int pktlog_nbuf_check_sanity(qdf_nbuf_t nbuf)
{
	int rc = 0; /* sane */

	if ((!nbuf) ||
	    (nbuf->data < nbuf->head) ||
	    ((nbuf->data + skb_headlen(nbuf)) > skb_end_pointer(nbuf)))
		rc = -EINVAL;

	return rc;
}
/**
 * pktlog_t2h_msg_handler() - Target to host message handler
 * @context: pdev context
 * @pkt: HTC packet
 *
 * Return: None
 */
static void pktlog_t2h_msg_handler(void *context, HTC_PACKET *pkt)
{
	struct pktlog_dev_t *pdev = (struct pktlog_dev_t *)context;
	qdf_nbuf_t pktlog_t2h_msg = (qdf_nbuf_t) pkt->pPktContext;
	uint32_t *msg_word;
	uint32_t msg_len;

	/* check for sanity of the packet, have seen corrupted pkts */
	if (pktlog_nbuf_check_sanity(pktlog_t2h_msg)) {
		qdf_print("packet 0x%pK corrupted? Leaking...",
			  pktlog_t2h_msg);
		/* do not free; may crash! */
		QDF_ASSERT(0);
		return;
	}

	/* check for successful message reception */
	if (pkt->Status != QDF_STATUS_SUCCESS) {
		if (pkt->Status != QDF_STATUS_E_CANCELED)
			pdev->htc_err_cnt++;
		qdf_nbuf_free(pktlog_t2h_msg);
		return;
	}

	/* confirm alignment */
	qdf_assert((((unsigned long)qdf_nbuf_data(pktlog_t2h_msg)) & 0x3) == 0);

	msg_word = (uint32_t *) qdf_nbuf_data(pktlog_t2h_msg);
	msg_len = qdf_nbuf_len(pktlog_t2h_msg);
	pktlog_process_fw_msg(pdev->pdev_id, msg_word, msg_len);

	qdf_nbuf_free(pktlog_t2h_msg);
}

/**
 * pktlog_tx_resume_handler() - resume callback
 * @context: pdev context
 *
 * Return: None
 */
static void pktlog_tx_resume_handler(void *context)
{
	qdf_print("Not expected");
	qdf_assert(0);
}

/**
 * pktlog_h2t_send_complete() - send complete indication
 * @context: pdev context
 * @htc_pkt: HTC packet
 *
 * Return: None
 */
static void pktlog_h2t_send_complete(void *context, HTC_PACKET *htc_pkt)
{
	qdf_print("Not expected");
	qdf_assert(0);
}

/**
 * pktlog_h2t_full() - queue full indication
 * @context: pdev context
 * @pkt: HTC packet
 *
 * Return: HTC action
 */
static enum htc_send_full_action pktlog_h2t_full(void *context, HTC_PACKET *pkt)
{
	return HTC_SEND_FULL_KEEP;
}

/**
 * pktlog_htc_connect_service() - create new endpoint for packetlog
 * @pdev - pktlog pdev
 *
 * Return: 0 for success/failure
 */
static int pktlog_htc_connect_service(struct pktlog_dev_t *pdev)
{
	struct htc_service_connect_req connect;
	struct htc_service_connect_resp response;
	QDF_STATUS status;

	qdf_mem_zero(&connect, sizeof(connect));
	qdf_mem_zero(&response, sizeof(response));

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

	if (status != QDF_STATUS_SUCCESS) {
		pdev->mt_pktlog_enabled = false;
		return -EIO;       /* failure */
	}

	pdev->htc_endpoint = response.Endpoint;
	pdev->mt_pktlog_enabled = true;

	return 0;               /* success */
}

/**
 * pktlog_htc_attach() - attach pktlog HTC service
 *
 * Return: 0 for success/failure
 */
int pktlog_htc_attach(void)
{
	struct pktlog_dev_t *pl_pdev = get_pktlog_handle();
	void *htc_pdev = cds_get_context(QDF_MODULE_ID_HTC);

	if ((!pl_pdev) || (!htc_pdev)) {
		qdf_print("Invalid pl_dev or htc_pdev handle");
		return -EINVAL;
	}

	pl_pdev->htc_pdev = htc_pdev;
	return pktlog_htc_connect_service(pl_pdev);
}
#else
int pktlog_htc_attach(void)
{
	struct pktlog_dev_t *pl_dev = get_pktlog_handle();

	if (!pl_dev) {
		qdf_print("Invalid pl_dev handle");
		return -EINVAL;
	}

	pl_dev->mt_pktlog_enabled = false;
	return 0;
}
#endif
#endif /* REMOVE_PKT_LOG */
