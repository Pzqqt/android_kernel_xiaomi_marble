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

/**
 *
 * @file  wlan_hdd_p2p.c
 *
 * @brief WLAN Host Device Driver implementation for P2P commands interface
 *
 */

#include <wlan_hdd_includes.h>
#include <wlan_hdd_hostapd.h>
#include <net/cfg80211.h>
#include "sme_api.h"
#include "sme_qos_api.h"
#include "wlan_hdd_p2p.h"
#include "sap_api.h"
#include "wlan_hdd_main.h"
#include "qdf_trace.h"
#include <linux/netdevice.h>
#include <linux/skbuff.h>
#include <linux/etherdevice.h>
#include <net/ieee80211_radiotap.h>
#include "wlan_hdd_tdls.h"
#include "wlan_hdd_trace.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "cds_sched.h"
#include "cds_concurrency.h"

/* Ms to Time Unit Micro Sec */
#define MS_TO_TU_MUS(x)   ((x) * 1024)

static uint8_t *hdd_get_action_string(uint16_t MsgType)
{
	switch (MsgType) {
		CASE_RETURN_STRING(SIR_MAC_ACTION_SPECTRUM_MGMT);
		CASE_RETURN_STRING(SIR_MAC_ACTION_QOS_MGMT);
		CASE_RETURN_STRING(SIR_MAC_ACTION_DLP);
		CASE_RETURN_STRING(SIR_MAC_ACTION_PUBLIC_USAGE);
		CASE_RETURN_STRING(SIR_MAC_ACTION_RRM);
		CASE_RETURN_STRING(SIR_MAC_ACTION_FAST_BSS_TRNST);
		CASE_RETURN_STRING(SIR_MAC_ACTION_HT);
		CASE_RETURN_STRING(SIR_MAC_ACTION_SA_QUERY);
		CASE_RETURN_STRING(SIR_MAC_ACTION_PROT_DUAL_PUB);
		CASE_RETURN_STRING(SIR_MAC_ACTION_WNM);
		CASE_RETURN_STRING(SIR_MAC_ACTION_UNPROT_WNM);
		CASE_RETURN_STRING(SIR_MAC_ACTION_TDLS);
		CASE_RETURN_STRING(SIR_MAC_ACITON_MESH);
		CASE_RETURN_STRING(SIR_MAC_ACTION_MHF);
		CASE_RETURN_STRING(SIR_MAC_SELF_PROTECTED);
		CASE_RETURN_STRING(SIR_MAC_ACTION_WME);
		CASE_RETURN_STRING(SIR_MAC_ACTION_VHT);
	default:
		return "UNKNOWN";
	}
}

#ifdef WLAN_FEATURE_P2P_DEBUG
#define MAX_P2P_ACTION_FRAME_TYPE 9
const char *p2p_action_frame_type[] = { "GO Negotiation Request",
					"GO Negotiation Response",
					"GO Negotiation Confirmation",
					"P2P Invitation Request",
					"P2P Invitation Response",
					"Device Discoverability Request",
					"Device Discoverability Response",
					"Provision Discovery Request",
					"Provision Discovery Response"};

/* We no need to protect this variable since
 * there is no chance of race to condition
 * and also not make any complicating the code
 * just for debugging log
 */
tP2PConnectionStatus global_p2p_connection_status = P2P_NOT_ACTIVE;

#endif
#define MAX_TDLS_ACTION_FRAME_TYPE 11
const char *tdls_action_frame_type[] = { "TDLS Setup Request",
					 "TDLS Setup Response",
					 "TDLS Setup Confirm",
					 "TDLS Teardown",
					 "TDLS Peer Traffic Indication",
					 "TDLS Channel Switch Request",
					 "TDLS Channel Switch Response",
					 "TDLS Peer PSM Request",
					 "TDLS Peer PSM Response",
					 "TDLS Peer Traffic Response",
					 "TDLS Discovery Request"};

static bool wlan_hdd_is_type_p2p_action(const u8 *buf)
{
	const u8 *ouiPtr;

	if (buf[WLAN_HDD_PUBLIC_ACTION_FRAME_CATEGORY_OFFSET] !=
	    WLAN_HDD_PUBLIC_ACTION_FRAME) {
		return false;
	}

	if (buf[WLAN_HDD_PUBLIC_ACTION_FRAME_ACTION_OFFSET] !=
	    WLAN_HDD_VENDOR_SPECIFIC_ACTION) {
		return false;
	}

	ouiPtr = &buf[WLAN_HDD_PUBLIC_ACTION_FRAME_OUI_OFFSET];

	if (WPA_GET_BE24(ouiPtr) != WLAN_HDD_WFA_OUI) {
		return false;
	}

	if (buf[WLAN_HDD_PUBLIC_ACTION_FRAME_OUI_TYPE_OFFSET] !=
	    WLAN_HDD_WFA_P2P_OUI_TYPE) {
		return false;
	}

	return true;
}

static bool hdd_p2p_is_action_type_rsp(const u8 *buf)
{
	tActionFrmType actionFrmType;

	if (wlan_hdd_is_type_p2p_action(buf)) {
		actionFrmType =
			buf[WLAN_HDD_PUBLIC_ACTION_FRAME_SUB_TYPE_OFFSET];
		if (actionFrmType != WLAN_HDD_INVITATION_REQ
		    && actionFrmType != WLAN_HDD_GO_NEG_REQ
		    && actionFrmType != WLAN_HDD_DEV_DIS_REQ
		    && actionFrmType != WLAN_HDD_PROV_DIS_REQ)
			return true;
	}

	return false;
}

static
QDF_STATUS wlan_hdd_remain_on_channel_callback(tHalHandle hHal, void *pCtx,
			QDF_STATUS status, uint32_t scan_id)
{
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) pCtx;
	hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR(pAdapter);
	hdd_remain_on_chan_ctx_t *pRemainChanCtx;
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);

	if (!hdd_ctx) {
		hdd_err("Invalid HDD context");
		return QDF_STATUS_E_FAILURE;
	}

	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	pRemainChanCtx = cfgState->remain_on_chan_ctx;

	if (pRemainChanCtx == NULL) {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		hddLog(LOGW,
		       "%s: No Rem on channel pending for which Rsp is received",
		       __func__);
		return QDF_STATUS_SUCCESS;
	}

	hddLog(LOG1, "Received remain on channel rsp");
	qdf_mc_timer_stop(&pRemainChanCtx->hdd_remain_on_chan_timer);
	qdf_mc_timer_destroy(&pRemainChanCtx->hdd_remain_on_chan_timer);

	cfgState->remain_on_chan_ctx = NULL;
	/*
	 * Resetting the roc in progress early ensures that the subsequent
	 * roc requests are immediately processed without being queued
	 */
	pAdapter->is_roc_inprogress = false;
	/*
	 * If the allow suspend is done later, the scheduled roc wil prevent
	 * the system from going into suspend and immediately this logic
	 * will allow the system to go to suspend breaking the exising logic.
	 * Basically, the system must not go into suspend while roc is in
	 * progress.
	 */
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);

	if (REMAIN_ON_CHANNEL_REQUEST == pRemainChanCtx->rem_on_chan_request) {
		if (cfgState->buf) {
			hdd_info("We need to receive yet an ack from one of tx packet");
		}
		cfg80211_remain_on_channel_expired(
			pRemainChanCtx->dev->
			ieee80211_ptr,
			pRemainChanCtx->
			cookie,
			&pRemainChanCtx->chan,
			GFP_KERNEL);
		pAdapter->last_roc_ts = qdf_mc_timer_get_system_time();
	}

	/* Schedule any pending RoC: Any new roc request during this time
	 * would have got queued in 'wlan_hdd_request_remain_on_channel'
	 * since the queue is not empty. So, the roc at the head of the
	 * queue will only get the priority. Scheduling the work queue
	 * after sending any cancel remain on channel event will also
	 * ensure that the cancel roc is sent without any delays.
	 */
	schedule_delayed_work(&hdd_ctx->roc_req_work, 0);

	if ((QDF_STA_MODE == pAdapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == pAdapter->device_mode) ||
	    (QDF_P2P_DEVICE_MODE == pAdapter->device_mode)
	    ) {
		uint8_t sessionId = pAdapter->sessionId;
		if (REMAIN_ON_CHANNEL_REQUEST ==
		    pRemainChanCtx->rem_on_chan_request) {
			sme_deregister_mgmt_frame(hHal, sessionId,
						  (SIR_MAC_MGMT_FRAME << 2) |
						  (SIR_MAC_MGMT_PROBE_REQ << 4),
						  NULL, 0);
		}
	} else if ((QDF_SAP_MODE == pAdapter->device_mode) ||
		   (QDF_P2P_GO_MODE == pAdapter->device_mode)
		   ) {
		wlansap_de_register_mgmt_frame(
#ifdef WLAN_FEATURE_MBSSID
			WLAN_HDD_GET_SAP_CTX_PTR
				(pAdapter),
#else
			(WLAN_HDD_GET_CTX
				 (pAdapter))->pcds_context,
#endif
			(SIR_MAC_MGMT_FRAME << 2) |
			(SIR_MAC_MGMT_PROBE_REQ <<
			 4), NULL, 0);

	}

	if (pRemainChanCtx->action_pkt_buff.frame_ptr != NULL
	    && pRemainChanCtx->action_pkt_buff.frame_length != 0) {
		qdf_mem_free(pRemainChanCtx->action_pkt_buff.frame_ptr);
		pRemainChanCtx->action_pkt_buff.frame_ptr = NULL;
		pRemainChanCtx->action_pkt_buff.frame_length = 0;
	}
	qdf_mem_free(pRemainChanCtx);
	complete(&pAdapter->cancel_rem_on_chan_var);
	if (QDF_STATUS_SUCCESS != status)
		complete(&pAdapter->rem_on_chan_ready_event);
	return QDF_STATUS_SUCCESS;
}

void wlan_hdd_cancel_existing_remain_on_channel(hdd_adapter_t *pAdapter)
{
	hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR(pAdapter);
	hdd_remain_on_chan_ctx_t *pRemainChanCtx;
	unsigned long rc;

	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	if (cfgState->remain_on_chan_ctx != NULL) {
		hddLog(LOGE, "Cancel Existing Remain on Channel");

		if (QDF_TIMER_STATE_RUNNING == qdf_mc_timer_get_current_state(
		    &cfgState->remain_on_chan_ctx->hdd_remain_on_chan_timer))
			qdf_mc_timer_stop(&cfgState->remain_on_chan_ctx->
					  hdd_remain_on_chan_timer);

		pRemainChanCtx = cfgState->remain_on_chan_ctx;
		if (pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress ==
		    true) {
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
			hddLog(LOGE,
			       "ROC timer cancellation in progress,"
			       " wait for completion");
			rc = wait_for_completion_timeout(&pAdapter->
							 cancel_rem_on_chan_var,
							 msecs_to_jiffies
								 (WAIT_CANCEL_REM_CHAN));
			if (!rc) {
				hddLog(LOGE,
				       "%s:wait on cancel_rem_on_chan_var timed out",
				       __func__);
			}
			return;
		}
		pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress = true;
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		/* Wait till remain on channel ready indication before issuing cancel
		 * remain on channel request, otherwise if remain on channel not
		 * received and if the driver issues cancel remain on channel then lim
		 * will be in unknown state.
		 */
		rc = wait_for_completion_timeout(&pAdapter->
						 rem_on_chan_ready_event,
						 msecs_to_jiffies
							 (WAIT_REM_CHAN_READY));
		if (!rc) {
			hddLog(LOGE,
			       "%s: timeout waiting for remain on channel ready indication",
			       __func__);
			cds_flush_logs(WLAN_LOG_TYPE_FATAL,
				WLAN_LOG_INDICATOR_HOST_DRIVER,
				WLAN_LOG_REASON_HDD_TIME_OUT,
				true, false);
		}

		INIT_COMPLETION(pAdapter->cancel_rem_on_chan_var);

		/* Issue abort remain on chan request to sme.
		 * The remain on channel callback will make sure the remain_on_chan
		 * expired event is sent.
		 */
		if ((QDF_STA_MODE == pAdapter->device_mode) ||
		    (QDF_P2P_CLIENT_MODE == pAdapter->device_mode) ||
		    (QDF_P2P_DEVICE_MODE == pAdapter->device_mode)
		    ) {
			sme_cancel_remain_on_channel(WLAN_HDD_GET_HAL_CTX
							     (pAdapter),
				pAdapter->sessionId,
				pRemainChanCtx->scan_id);
		} else if ((QDF_SAP_MODE == pAdapter->device_mode)
			   || (QDF_P2P_GO_MODE == pAdapter->device_mode)
			   ) {
			wlansap_cancel_remain_on_channel(
#ifdef WLAN_FEATURE_MBSSID
				WLAN_HDD_GET_SAP_CTX_PTR
					(pAdapter),
#else
			(WLAN_HDD_GET_CTX(pAdapter))->pcds_context,
#endif
			pRemainChanCtx->scan_id);
		}

		rc = wait_for_completion_timeout(&pAdapter->
						 cancel_rem_on_chan_var,
						 msecs_to_jiffies
							 (WAIT_CANCEL_REM_CHAN));

		if (!rc) {
			hddLog(LOGE,
			       "%s: timeout waiting for cancel remain on channel ready"
			       " indication", __func__);
		}
		hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
	} else
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
}

int wlan_hdd_check_remain_on_channel(hdd_adapter_t *pAdapter)
{
	int status = 0;
	hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR(pAdapter);

	if (QDF_P2P_GO_MODE != pAdapter->device_mode) {
		/* Cancel Existing Remain On Channel */
		/* If no action frame is pending */
		if (cfgState->remain_on_chan_ctx != NULL) {
			/* Check whether Action Frame is pending or not */
			if (cfgState->buf == NULL) {
				wlan_hdd_cancel_existing_remain_on_channel
					(pAdapter);
			} else {
				hddLog(LOG1,
				       "Cannot Cancel Existing Remain on Channel");
				status = -EBUSY;
			}
		}
	}
	return status;
}

/**
 * wlan_hdd_cancel_pending_roc() - Cancel pending roc
 * @adapter: HDD adapter
 *
 * Cancels any pending remain on channel request
 *
 * Return: None
 */
void wlan_hdd_cancel_pending_roc(hdd_adapter_t *adapter)
{
	hdd_remain_on_chan_ctx_t *roc_ctx;
	unsigned long rc;
	hdd_cfg80211_state_t *cfg_state = WLAN_HDD_GET_CFG_STATE_PTR(adapter);

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			"%s: ROC completion is not received.!!!",
			__func__);

	mutex_lock(&cfg_state->remain_on_chan_ctx_lock);
	roc_ctx = cfg_state->remain_on_chan_ctx;

	if (roc_ctx->hdd_remain_on_chan_cancel_in_progress) {
		mutex_unlock(&cfg_state->remain_on_chan_ctx_lock);
		hdd_debug("roc cancel already in progress");
		/*
		 * Since a cancel roc is already issued and is
		 * in progress, we need not send another
		 * cancel roc again. Instead we can just wait
		 * for cancel roc completion
		 */
		goto wait;
	}
	mutex_unlock(&cfg_state->remain_on_chan_ctx_lock);

	if (adapter->device_mode == QDF_P2P_GO_MODE) {
		wlansap_cancel_remain_on_channel((WLAN_HDD_GET_CTX
					(adapter))->pcds_context,
					cfg_state->remain_on_chan_ctx->scan_id);
	} else if (adapter->device_mode == QDF_P2P_CLIENT_MODE
			|| adapter->device_mode ==
			QDF_P2P_DEVICE_MODE) {
		sme_cancel_remain_on_channel(WLAN_HDD_GET_HAL_CTX
				(adapter),
				adapter->sessionId,
				cfg_state->remain_on_chan_ctx->scan_id);
	}

wait:
	rc = wait_for_completion_timeout(&adapter->cancel_rem_on_chan_var,
			msecs_to_jiffies
			(WAIT_CANCEL_REM_CHAN));
	if (!rc) {
		QDF_TRACE(QDF_MODULE_ID_HDD,
		    QDF_TRACE_LEVEL_ERROR,
		    "%s: Timeout occurred while waiting for RoC Cancellation",
		    __func__);
		mutex_lock(&cfg_state->remain_on_chan_ctx_lock);
		roc_ctx = cfg_state->remain_on_chan_ctx;
		if (roc_ctx != NULL) {
			cfg_state->remain_on_chan_ctx = NULL;
			qdf_mc_timer_stop(&roc_ctx->hdd_remain_on_chan_timer);
			qdf_mc_timer_destroy(
					&roc_ctx->hdd_remain_on_chan_timer);
			if (roc_ctx->action_pkt_buff.frame_ptr != NULL
				&& roc_ctx->action_pkt_buff.frame_length != 0) {
				qdf_mem_free(
					roc_ctx->action_pkt_buff.frame_ptr);
				roc_ctx->action_pkt_buff.frame_ptr = NULL;
				roc_ctx->action_pkt_buff.frame_length = 0;
			}
			qdf_mem_free(roc_ctx);
			adapter->is_roc_inprogress = false;
		}
		mutex_unlock(&cfg_state->remain_on_chan_ctx_lock);
	}
}

/* Clean up RoC context at hdd_stop_adapter*/
void wlan_hdd_cleanup_remain_on_channel_ctx(hdd_adapter_t *pAdapter)
{
	uint8_t retry = 0;
	hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR(pAdapter);

	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	while (pAdapter->is_roc_inprogress) {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: ROC in progress for session %d!!!",
			  __func__, pAdapter->sessionId);
		msleep(500);
		if (retry++ > 3) {
			wlan_hdd_cancel_pending_roc(pAdapter);
			/* hold the lock before break from the loop */
			mutex_lock(&cfgState->remain_on_chan_ctx_lock);
			break;
		}
		mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	} /* end of while */
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);

}

void wlan_hdd_remain_on_chan_timeout(void *data)
{
	hdd_adapter_t *pAdapter = (hdd_adapter_t *) data;
	hdd_remain_on_chan_ctx_t *pRemainChanCtx;
	hdd_cfg80211_state_t *cfgState;

	if (NULL == pAdapter) {
		hddLog(LOGE, "%s: pAdapter is NULL !!!", __func__);
		return;
	}

	cfgState = WLAN_HDD_GET_CFG_STATE_PTR(pAdapter);
	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	pRemainChanCtx = cfgState->remain_on_chan_ctx;

	if (NULL == pRemainChanCtx) {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		hddLog(LOGE, "%s: No Remain on channel is pending", __func__);
		return;
	}

	if (true == pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress) {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		hddLog(LOGE, FL("Cancellation already in progress"));
		return;
	}

	pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress = true;
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
	hddLog(LOG1, "%s: Cancel Remain on Channel on timeout", __func__);

	if ((QDF_STA_MODE == pAdapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == pAdapter->device_mode) ||
	    (QDF_P2P_DEVICE_MODE == pAdapter->device_mode)
	    ) {
		sme_cancel_remain_on_channel(WLAN_HDD_GET_HAL_CTX(pAdapter),
			pAdapter->sessionId,
			pRemainChanCtx->scan_id);
	} else if ((QDF_SAP_MODE == pAdapter->device_mode) ||
		   (QDF_P2P_GO_MODE == pAdapter->device_mode)
		   ) {
		wlansap_cancel_remain_on_channel(
			(WLAN_HDD_GET_CTX(pAdapter))->pcds_context,
			pRemainChanCtx->scan_id);
	}

	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);

}

static int wlan_hdd_execute_remain_on_channel(hdd_adapter_t *pAdapter,
		      hdd_remain_on_chan_ctx_t *pRemainChanCtx)
{
	hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR(pAdapter);
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	hdd_adapter_t *pAdapter_temp;
	QDF_STATUS status;
	bool isGoPresent = false;
	unsigned int duration;


	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	if (pAdapter->is_roc_inprogress == true) {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		hddLog(QDF_TRACE_LEVEL_ERROR,
			FL("remain on channel request is in execution"));
		return -EBUSY;
	}

	cfgState->remain_on_chan_ctx = pRemainChanCtx;
	cfgState->current_freq = pRemainChanCtx->chan.center_freq;
	pAdapter->is_roc_inprogress = true;
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);

	/* Initialize Remain on chan timer */
	qdf_status =
		qdf_mc_timer_init(&pRemainChanCtx->hdd_remain_on_chan_timer,
				  QDF_TIMER_TYPE_SW,
				  wlan_hdd_remain_on_chan_timeout, pAdapter);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
		       FL("Not able to initialize remain_on_chan timer"));
		mutex_lock(&cfgState->remain_on_chan_ctx_lock);
		cfgState->remain_on_chan_ctx = NULL;
		pAdapter->is_roc_inprogress = false;
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		qdf_mem_free(pRemainChanCtx);
		return -EINVAL;
	}

	status = hdd_get_front_adapter(pHddCtx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		pAdapter_temp = pAdapterNode->pAdapter;
		if (pAdapter_temp->device_mode == QDF_P2P_GO_MODE)
			isGoPresent = true;
		status = hdd_get_next_adapter(pHddCtx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}

	/* Extending duration for proactive extension logic for RoC */
	duration = pRemainChanCtx->duration;
	if (isGoPresent == true)
		duration = P2P_ROC_DURATION_MULTIPLIER_GO_PRESENT * duration;
	else
		duration = P2P_ROC_DURATION_MULTIPLIER_GO_ABSENT * duration;

	hdd_prevent_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
	INIT_COMPLETION(pAdapter->rem_on_chan_ready_event);

	/* call sme API to start remain on channel. */
	if ((QDF_STA_MODE == pAdapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == pAdapter->device_mode) ||
	    (QDF_P2P_DEVICE_MODE == pAdapter->device_mode)
	    ) {
		uint8_t sessionId = pAdapter->sessionId;
		/* call sme API to start remain on channel. */

		if (QDF_STATUS_SUCCESS != sme_remain_on_channel(
				WLAN_HDD_GET_HAL_CTX(pAdapter),
				sessionId,
				pRemainChanCtx->chan.hw_value, duration,
				wlan_hdd_remain_on_channel_callback,
				pAdapter,
				(pRemainChanCtx->rem_on_chan_request ==
				 REMAIN_ON_CHANNEL_REQUEST) ? true : false,
				 &pRemainChanCtx->scan_id)) {
			hddLog(LOGE, FL("sme_remain_on_channel failed"));
			mutex_lock(&cfgState->remain_on_chan_ctx_lock);
			cfgState->remain_on_chan_ctx = NULL;
			pAdapter->is_roc_inprogress = false;
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
			qdf_mc_timer_destroy(
				&pRemainChanCtx->hdd_remain_on_chan_timer);
			qdf_mem_free(pRemainChanCtx);
			hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
			return -EINVAL;
		}

		if (REMAIN_ON_CHANNEL_REQUEST ==
		    pRemainChanCtx->rem_on_chan_request) {
			if (QDF_STATUS_SUCCESS != sme_register_mgmt_frame(
						WLAN_HDD_GET_HAL_CTX(pAdapter),
						sessionId,
						(SIR_MAC_MGMT_FRAME << 2) |
						(SIR_MAC_MGMT_PROBE_REQ << 4),
						NULL, 0))
				hddLog(LOGE,
					FL("sme_register_mgmt_frame failed"));
		}

	} else if ((QDF_SAP_MODE == pAdapter->device_mode) ||
		   (QDF_P2P_GO_MODE == pAdapter->device_mode)) {
		/* call sme API to start remain on channel. */
		if (QDF_STATUS_SUCCESS != wlansap_remain_on_channel(
#ifdef WLAN_FEATURE_MBSSID
			    WLAN_HDD_GET_SAP_CTX_PTR(pAdapter),
#else
			    (WLAN_HDD_GET_CTX(pAdapter))->pcds_context,
#endif
			    pRemainChanCtx->chan.hw_value,
			    duration, wlan_hdd_remain_on_channel_callback,
			    pAdapter, &pRemainChanCtx->scan_id)) {
			hddLog(LOGE, FL("wlansap_remain_on_channel failed"));
			mutex_lock(&cfgState->remain_on_chan_ctx_lock);
			cfgState->remain_on_chan_ctx = NULL;
			pAdapter->is_roc_inprogress = false;
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
			qdf_mc_timer_destroy(
				&pRemainChanCtx->hdd_remain_on_chan_timer);
			qdf_mem_free(pRemainChanCtx);
			hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
			return -EINVAL;
		}

		if (QDF_STATUS_SUCCESS != wlansap_register_mgmt_frame(
#ifdef WLAN_FEATURE_MBSSID
			    WLAN_HDD_GET_SAP_CTX_PTR(pAdapter),
#else
			    (WLAN_HDD_GET_CTX(pAdapter))->pcds_context,
#endif
			(SIR_MAC_MGMT_FRAME << 2) |
			(SIR_MAC_MGMT_PROBE_REQ << 4), NULL, 0)) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				FL("wlansap_register_mgmt_frame return fail"));
			wlansap_cancel_remain_on_channel(
#ifdef WLAN_FEATURE_MBSSID
				WLAN_HDD_GET_SAP_CTX_PTR(pAdapter),
#else
				(WLAN_HDD_GET_CTX(pAdapter))->pcds_context,
#endif
			pRemainChanCtx->scan_id);
			hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
			return -EINVAL;
		}

	}
	return 0;
}

/**
 * wlan_hdd_roc_request_enqueue() - enqueue remain on channel request
 * @adapter: Pointer to the adapter
 * @remain_chan_ctx: Pointer to the remain on channel context
 *
 * Return: 0 on success, error number otherwise
 */
static int wlan_hdd_roc_request_enqueue(hdd_adapter_t *adapter,
			hdd_remain_on_chan_ctx_t *remain_chan_ctx)
{
	hdd_context_t *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	hdd_roc_req_t *hdd_roc_req;
	QDF_STATUS status;

	/*
	 * "Driver is busy" OR "there is already RoC request inside the queue"
	 * so enqueue this RoC Request and execute sequentially later.
	 */

	hdd_roc_req = qdf_mem_malloc(sizeof(*hdd_roc_req));

	if (NULL == hdd_roc_req) {
		hddLog(LOGP, FL("malloc failed for roc req context"));
		return -ENOMEM;
	}

	hdd_roc_req->pAdapter = adapter;
	hdd_roc_req->pRemainChanCtx = remain_chan_ctx;

	/* Enqueue this RoC request */
	qdf_spin_lock(&hdd_ctx->hdd_roc_req_q_lock);
	status = qdf_list_insert_back(&hdd_ctx->hdd_roc_req_q,
					&hdd_roc_req->node);
	qdf_spin_unlock(&hdd_ctx->hdd_roc_req_q_lock);

	if (QDF_STATUS_SUCCESS != status) {
		hddLog(LOGP, FL("Not able to enqueue RoC Req context"));
		qdf_mem_free(hdd_roc_req);
		return -EINVAL;
	}

	return 0;
}

/**
 * wlan_hdd_indicate_roc_drop() - Indicate roc drop to userspace
 * @adapter: HDD adapter
 * @ctx: Remain on channel context
 *
 * Send remain on channel ready and cancel event for the queued
 * roc that is being dropped. This will ensure that the userspace
 * will send more roc requests. If this drop is not indicated to
 * userspace, subsequent roc will not be sent to the driver since
 * the userspace times out waiting for the remain on channel ready
 * event.
 *
 * Return: None
 */
void wlan_hdd_indicate_roc_drop(hdd_adapter_t *adapter,
				hdd_remain_on_chan_ctx_t *ctx)
{
	hdd_debug("indicate roc drop to userspace");
	cfg80211_ready_on_channel(
			adapter->dev->ieee80211_ptr,
			(uintptr_t)ctx,
			&ctx->chan,
			ctx->duration, GFP_KERNEL);

	cfg80211_remain_on_channel_expired(
			ctx->dev->ieee80211_ptr,
			ctx->cookie,
			&ctx->chan,
			GFP_KERNEL);
}

/**
 * wlan_hdd_roc_request_dequeue() - dequeue remain on channel request
 * @work: Pointer to work queue struct
 *
 * Return: none
 */
void wlan_hdd_roc_request_dequeue(struct work_struct *work)
{
	QDF_STATUS status;
	int ret = 0;
	hdd_roc_req_t *hdd_roc_req;
	hdd_context_t *hdd_ctx =
			container_of(work, hdd_context_t, roc_req_work.work);

	hdd_debug("going to dequeue roc");

	if (0 != (wlan_hdd_validate_context(hdd_ctx)))
		return;

	/*
	 * The queued roc requests is dequeued and processed one at a time.
	 * Callback 'wlan_hdd_remain_on_channel_callback' ensures
	 * that any pending roc in the queue will be scheduled
	 * on the current roc completion by scheduling the work queue.
	 */
	qdf_spin_lock(&hdd_ctx->hdd_roc_req_q_lock);
	if (list_empty(&hdd_ctx->hdd_roc_req_q.anchor)) {
		qdf_spin_unlock(&hdd_ctx->hdd_roc_req_q_lock);
		return;
	}
	status = qdf_list_remove_front(&hdd_ctx->hdd_roc_req_q,
			(qdf_list_node_t **) &hdd_roc_req);
	qdf_spin_unlock(&hdd_ctx->hdd_roc_req_q_lock);
	if (QDF_STATUS_SUCCESS != status) {
		hdd_debug("unable to remove roc element from list");
		return;
	}
	ret = wlan_hdd_execute_remain_on_channel(
			hdd_roc_req->pAdapter,
			hdd_roc_req->pRemainChanCtx);
	if (ret == -EBUSY) {
		hddLog(QDF_TRACE_LEVEL_ERROR,
				FL("dropping RoC request"));
		wlan_hdd_indicate_roc_drop(hdd_roc_req->pAdapter,
					   hdd_roc_req->pRemainChanCtx);
		qdf_mem_free(hdd_roc_req->pRemainChanCtx);
	}
	qdf_mem_free(hdd_roc_req);
}

static int wlan_hdd_request_remain_on_channel(struct wiphy *wiphy,
					      struct net_device *dev,
					      struct ieee80211_channel *chan,
					      unsigned int duration,
					      u64 *cookie,
					      rem_on_channel_request_type_t
					      request_type)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *pHddCtx;
	hdd_remain_on_chan_ctx_t *pRemainChanCtx;
	bool isBusy = false;
	uint32_t size = 0;
	hdd_adapter_t *sta_adapter;
	int ret;
	int status = 0;

	ENTER();

	hddLog(LOG1, FL("Device_mode %s(%d)"),
	       hdd_device_mode_to_string(pAdapter->device_mode),
	       pAdapter->device_mode);
	hddLog(LOG1,
	       "chan(hw_val)0x%x chan(centerfreq) %d, duration %d",
	       chan->hw_value, chan->center_freq, duration);
	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(pHddCtx);
	if (0 != ret)
		return ret;
	if (cds_is_connection_in_progress()) {
		hddLog(LOGE, FL("Connection is in progress"));
		isBusy = true;
	}
	pRemainChanCtx = qdf_mem_malloc(sizeof(hdd_remain_on_chan_ctx_t));
	if (NULL == pRemainChanCtx) {
		hddLog(QDF_TRACE_LEVEL_FATAL,
		       "%s: Not able to allocate memory for Channel context",
		       __func__);
		return -ENOMEM;
	}

	qdf_mem_zero(pRemainChanCtx, sizeof(*pRemainChanCtx));
	qdf_mem_copy(&pRemainChanCtx->chan, chan,
		     sizeof(struct ieee80211_channel));
	pRemainChanCtx->duration = duration;
	pRemainChanCtx->dev = dev;
	*cookie = (uintptr_t) pRemainChanCtx;
	pRemainChanCtx->cookie = *cookie;
	pRemainChanCtx->rem_on_chan_request = request_type;
	pRemainChanCtx->action_pkt_buff.freq = 0;
	pRemainChanCtx->action_pkt_buff.frame_ptr = NULL;
	pRemainChanCtx->action_pkt_buff.frame_length = 0;
	pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress = false;
	if (REMAIN_ON_CHANNEL_REQUEST == request_type) {
		sta_adapter = hdd_get_adapter(pHddCtx, QDF_STA_MODE);
		if ((NULL != sta_adapter) &&
			hdd_conn_is_connected(
				WLAN_HDD_GET_STATION_CTX_PTR(sta_adapter))) {
			if (pAdapter->last_roc_ts != 0 &&
				((qdf_mc_timer_get_system_time() -
					 pAdapter->last_roc_ts) <
				pHddCtx->config->p2p_listen_defer_interval)) {
			if (pRemainChanCtx->duration > HDD_P2P_MAX_ROC_DURATION)
				pRemainChanCtx->duration =
						HDD_P2P_MAX_ROC_DURATION;

			wlan_hdd_roc_request_enqueue(pAdapter, pRemainChanCtx);
			schedule_delayed_work(&pHddCtx->roc_req_work,
			msecs_to_jiffies(
				pHddCtx->config->p2p_listen_defer_interval));
			hddLog(LOG1, "Defer interval is %hu, pAdapter %p",
				pHddCtx->config->p2p_listen_defer_interval,
				pAdapter);
			return 0;
			}
		}
	}

	qdf_spin_lock(&pHddCtx->hdd_roc_req_q_lock);
	size = qdf_list_size(&(pHddCtx->hdd_roc_req_q));
	qdf_spin_unlock(&pHddCtx->hdd_roc_req_q_lock);
	if ((isBusy == false) && (!size)) {
		status = wlan_hdd_execute_remain_on_channel(pAdapter,
							    pRemainChanCtx);
		if (status == -EBUSY) {
			if (wlan_hdd_roc_request_enqueue(pAdapter,
							 pRemainChanCtx)) {
				qdf_mem_free(pRemainChanCtx);
				return -EAGAIN;
			}
		}
		return 0;
	} else {
		if (wlan_hdd_roc_request_enqueue(pAdapter, pRemainChanCtx)) {
			qdf_mem_free(pRemainChanCtx);
			return -EAGAIN;
		}
	}

	/*
	 * If a connection is not in progress (isBusy), before scheduling
	 * the work queue it is necessary to check if a roc in in progress
	 * or not because: if an roc is in progress, the dequeued roc
	 * that will be processed will be dropped. To ensure that this new
	 * roc request is not dropped, it is suggested to check if an roc
	 * is in progress or not. The existing roc completion will provide
	 * the trigger to dequeue the next roc request.
	 */
	if (isBusy == false && pAdapter->is_roc_inprogress == false) {
		hdd_debug("scheduling delayed work: no connection/roc active");
		schedule_delayed_work(&pHddCtx->roc_req_work, 0);
	}
	EXIT();
	return 0;
}

int __wlan_hdd_cfg80211_remain_on_channel(struct wiphy *wiphy,
					  struct wireless_dev *wdev,
					  struct ieee80211_channel *chan,
					  unsigned int duration, u64 *cookie)
{
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_context_t *hdd_ctx;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(pAdapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_REMAIN_ON_CHANNEL,
			 pAdapter->sessionId, REMAIN_ON_CHANNEL_REQUEST));

	ret = wlan_hdd_request_remain_on_channel(wiphy, dev, chan,
						  duration, cookie,
						  REMAIN_ON_CHANNEL_REQUEST);
	EXIT();
	return ret;
}

int wlan_hdd_cfg80211_remain_on_channel(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					struct ieee80211_channel *chan,
					unsigned int duration, u64 *cookie)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_remain_on_channel(wiphy,
						    wdev,
						    chan,
						    duration, cookie);
	cds_ssr_unprotect(__func__);

	return ret;
}

void hdd_remain_chan_ready_handler(hdd_adapter_t *pAdapter,
	uint32_t scan_id)
{
	hdd_cfg80211_state_t *cfgState = NULL;
	hdd_remain_on_chan_ctx_t *pRemainChanCtx = NULL;
	QDF_STATUS status;

	if (NULL == pAdapter) {
		hddLog(LOGE, FL("pAdapter is NULL"));
		return;
	}
	cfgState = WLAN_HDD_GET_CFG_STATE_PTR(pAdapter);
	hddLog(LOG1, "Ready on chan ind %d", scan_id);

	pAdapter->start_roc_ts = qdf_mc_timer_get_system_time();
	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	pRemainChanCtx = cfgState->remain_on_chan_ctx;
	if (pRemainChanCtx != NULL) {
		MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
				 TRACE_CODE_HDD_REMAINCHANREADYHANDLER,
				 pAdapter->sessionId,
				 pRemainChanCtx->duration));
		/* start timer for actual duration */
		if (QDF_TIMER_STATE_RUNNING ==
			qdf_mc_timer_get_current_state(
				&pRemainChanCtx->hdd_remain_on_chan_timer)) {
			hddLog(LOGE, "Timer Started before ready event!!!");
			qdf_mc_timer_stop(&pRemainChanCtx->
					  hdd_remain_on_chan_timer);
		}
		status =
			qdf_mc_timer_start(&pRemainChanCtx->
					   hdd_remain_on_chan_timer,
					   (pRemainChanCtx->duration +
					    COMPLETE_EVENT_PROPOGATE_TIME));
		if (status != QDF_STATUS_SUCCESS) {
			hddLog(LOGE, "%s: Remain on Channel timer start failed",
			       __func__);
		}

		if (REMAIN_ON_CHANNEL_REQUEST ==
		    pRemainChanCtx->rem_on_chan_request) {
			cfg80211_ready_on_channel(
				pAdapter->dev->
				ieee80211_ptr,
				(uintptr_t)
				pRemainChanCtx,
				&pRemainChanCtx->chan,
				pRemainChanCtx->
				duration, GFP_KERNEL);
		} else if (OFF_CHANNEL_ACTION_TX ==
			   pRemainChanCtx->rem_on_chan_request) {
			complete(&pAdapter->offchannel_tx_event);
		}
		/* Check for cached action frame */
		if (pRemainChanCtx->action_pkt_buff.frame_length != 0) {
			hddLog(LOGE,
			       "%s: Sent cached action frame to supplicant",
			       __func__);
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
			cfg80211_rx_mgmt(pAdapter->dev->ieee80211_ptr,
				pRemainChanCtx->action_pkt_buff.freq, 0,
				pRemainChanCtx->action_pkt_buff.frame_ptr,
				pRemainChanCtx->action_pkt_buff.frame_length,
				NL80211_RXMGMT_FLAG_ANSWERED);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
			cfg80211_rx_mgmt(pAdapter->dev->ieee80211_ptr,
				pRemainChanCtx->action_pkt_buff.freq, 0,
				pRemainChanCtx->action_pkt_buff.frame_ptr,
				pRemainChanCtx->action_pkt_buff.frame_length,
				NL80211_RXMGMT_FLAG_ANSWERED, GFP_ATOMIC);
#else
			cfg80211_rx_mgmt(pAdapter->dev->ieee80211_ptr,
					 pRemainChanCtx->action_pkt_buff.freq,
					 0,
					 pRemainChanCtx->action_pkt_buff.
					 frame_ptr,
					 pRemainChanCtx->action_pkt_buff.
					 frame_length, GFP_ATOMIC);
#endif /* LINUX_VERSION_CODE */

			qdf_mem_free(pRemainChanCtx->action_pkt_buff.frame_ptr);
			pRemainChanCtx->action_pkt_buff.frame_length = 0;
			pRemainChanCtx->action_pkt_buff.freq = 0;
			pRemainChanCtx->action_pkt_buff.frame_ptr = NULL;
		}
		complete(&pAdapter->rem_on_chan_ready_event);
	} else {
		hddLog(LOGW, "%s: No Pending Remain on channel Request",
		       __func__);
	}
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
	return;
}

int __wlan_hdd_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 u64 cookie)
{
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR(pAdapter);
	hdd_remain_on_chan_ctx_t *pRemainChanCtx;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	int status;
	int qdf_status;
	unsigned long rc;
	qdf_list_node_t *tmp, *q;
	hdd_roc_req_t *curr_roc_req;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;
	qdf_spin_lock(&pHddCtx->hdd_roc_req_q_lock);
	list_for_each_safe(tmp, q, &pHddCtx->hdd_roc_req_q.anchor) {
		curr_roc_req = list_entry(tmp, hdd_roc_req_t, node);
		if ((uintptr_t) curr_roc_req->pRemainChanCtx == cookie) {
			qdf_status = qdf_list_remove_node(&pHddCtx->hdd_roc_req_q,
						      (qdf_list_node_t *)
						      curr_roc_req);
			qdf_spin_unlock(&pHddCtx->hdd_roc_req_q_lock);
			if (qdf_status == QDF_STATUS_SUCCESS) {
				qdf_mem_free(curr_roc_req->pRemainChanCtx);
				qdf_mem_free(curr_roc_req);
			}
			return 0;
		}
	}
	qdf_spin_unlock(&pHddCtx->hdd_roc_req_q_lock);
	/* FIXME cancel currently running remain on chan.
	 * Need to check cookie and cancel accordingly
	 */
	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	pRemainChanCtx = cfgState->remain_on_chan_ctx;
	if ((cfgState->remain_on_chan_ctx == NULL) ||
	    (cfgState->remain_on_chan_ctx->cookie != cookie)) {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		hddLog(LOGE,
		       "%s: No Remain on channel pending with specified cookie value",
		       __func__);
		return -EINVAL;
	}

	if (NULL != cfgState->remain_on_chan_ctx) {
		qdf_mc_timer_stop(&cfgState->remain_on_chan_ctx->
				  hdd_remain_on_chan_timer);
		if (true ==
		    pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress) {
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
			hddLog(LOG1,
			       FL("ROC timer cancellation in progress,"
				  " wait for completion"));
			rc = wait_for_completion_timeout(&pAdapter->
							 cancel_rem_on_chan_var,
							 msecs_to_jiffies
								 (WAIT_CANCEL_REM_CHAN));
			if (!rc) {
				hddLog(LOGE,
				       "%s:wait on cancel_rem_on_chan_var timed out",
				       __func__);
			}
			return 0;
		} else
			pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress =
				true;
	}
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);

	/* wait until remain on channel ready event received
	 * for already issued remain on channel request */
	rc = wait_for_completion_timeout(&pAdapter->rem_on_chan_ready_event,
					 msecs_to_jiffies(WAIT_REM_CHAN_READY));
	if (!rc) {
		hddLog(LOGE,
		       "%s: timeout waiting for remain on channel ready indication",
		       __func__);

		if (cds_is_driver_recovering()) {
			hdd_err("Recovery in Progress. State: 0x%x Ignore!!!",
				 cds_get_driver_state());
			return -EAGAIN;
		}
		cds_flush_logs(WLAN_LOG_TYPE_FATAL,
			WLAN_LOG_INDICATOR_HOST_DRIVER,
			WLAN_LOG_REASON_HDD_TIME_OUT,
			true, false);
	}
	INIT_COMPLETION(pAdapter->cancel_rem_on_chan_var);
	/* Issue abort remain on chan request to sme.
	 * The remain on channel callback will make sure the remain_on_chan
	 * expired event is sent.
	 */
	if ((QDF_STA_MODE == pAdapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == pAdapter->device_mode) ||
	    (QDF_P2P_DEVICE_MODE == pAdapter->device_mode)
	    ) {

		uint8_t sessionId = pAdapter->sessionId;
		sme_cancel_remain_on_channel(WLAN_HDD_GET_HAL_CTX(pAdapter),
			sessionId,
			pRemainChanCtx->scan_id);
	} else if ((QDF_SAP_MODE == pAdapter->device_mode) ||
		   (QDF_P2P_GO_MODE == pAdapter->device_mode)
		   ) {
		wlansap_cancel_remain_on_channel(
#ifdef WLAN_FEATURE_MBSSID
			WLAN_HDD_GET_SAP_CTX_PTR(pAdapter),
#else
			(WLAN_HDD_GET_CTX(pAdapter))->pcds_context,
#endif
			pRemainChanCtx->scan_id);

	} else {
		hddLog(LOGE, FL("Invalid device_mode %s(%d)"),
		       hdd_device_mode_to_string(pAdapter->device_mode),
		       pAdapter->device_mode);
		return -EIO;
	}
	rc = wait_for_completion_timeout(&pAdapter->cancel_rem_on_chan_var,
					 msecs_to_jiffies
						 (WAIT_CANCEL_REM_CHAN));
	if (!rc) {
		hddLog(LOGE,
		       "%s:wait on cancel_rem_on_chan_var timed out ",
		       __func__);
	}
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
	EXIT();
	return 0;
}

int wlan_hdd_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
					       struct wireless_dev *wdev,
					       u64 cookie)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_cancel_remain_on_channel(wiphy,
							   wdev,
							   cookie);
	cds_ssr_unprotect(__func__);

	return ret;
}

int __wlan_hdd_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
		       struct ieee80211_channel *chan, bool offchan,
		       unsigned int wait,
		       const u8 *buf, size_t len, bool no_cck,
		       bool dont_wait_for_ack, u64 *cookie)
{
	struct net_device *dev = wdev->netdev;
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR(pAdapter);
	hdd_remain_on_chan_ctx_t *pRemainChanCtx;
	hdd_context_t *pHddCtx = WLAN_HDD_GET_CTX(pAdapter);
	uint16_t extendedWait = 0;
	uint8_t type = WLAN_HDD_GET_TYPE_FRM_FC(buf[0]);
	uint8_t subType = WLAN_HDD_GET_SUBTYPE_FRM_FC(buf[0]);
	tActionFrmType actionFrmType;
	bool noack = 0;
	int status;
	unsigned long rc;
	hdd_adapter_t *goAdapter;
	uint16_t current_freq;
	uint8_t home_ch = 0;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_ACTION, pAdapter->sessionId,
			 pAdapter->device_mode));
	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;

	hddLog(LOG1, FL("Device_mode %s(%d) type: %d, wait: %d, offchan: %d, category: %d, actionId: %d"),
	       hdd_device_mode_to_string(pAdapter->device_mode),
	       pAdapter->device_mode, type, wait, offchan,
	       buf[WLAN_HDD_PUBLIC_ACTION_FRAME_BODY_OFFSET +
			WLAN_HDD_PUBLIC_ACTION_FRAME_CATEGORY_OFFSET],
	       buf[WLAN_HDD_PUBLIC_ACTION_FRAME_BODY_OFFSET +
			WLAN_HDD_PUBLIC_ACTION_FRAME_ACTION_OFFSET]);

#ifdef WLAN_FEATURE_P2P_DEBUG
	if ((type == SIR_MAC_MGMT_FRAME) &&
	    (subType == SIR_MAC_MGMT_ACTION) &&
	    wlan_hdd_is_type_p2p_action(&buf
				[WLAN_HDD_PUBLIC_ACTION_FRAME_BODY_OFFSET])) {
		actionFrmType = buf[WLAN_HDD_PUBLIC_ACTION_FRAME_TYPE_OFFSET];
		if (actionFrmType >= MAX_P2P_ACTION_FRAME_TYPE) {
			hddLog(QDF_TRACE_LEVEL_ERROR,
			       "[P2P] unknown[%d] ---> OTA to " MAC_ADDRESS_STR,
			       actionFrmType,
			       MAC_ADDR_ARRAY(&buf
					      [WLAN_HDD_80211_FRM_DA_OFFSET]));
		} else {
			hddLog(QDF_TRACE_LEVEL_ERROR, "[P2P] %s ---> OTA to "
			       MAC_ADDRESS_STR,
			       p2p_action_frame_type[actionFrmType],
			       MAC_ADDR_ARRAY(&buf
					      [WLAN_HDD_80211_FRM_DA_OFFSET]));
			if ((actionFrmType == WLAN_HDD_PROV_DIS_REQ)
			    && (global_p2p_connection_status == P2P_NOT_ACTIVE)) {
				global_p2p_connection_status = P2P_GO_NEG_PROCESS;
				hddLog(LOGE, "[P2P State]Inactive state to "
				       "GO negotiation progress state");
			} else if ((actionFrmType == WLAN_HDD_GO_NEG_CNF) &&
				   (global_p2p_connection_status ==
				    P2P_GO_NEG_PROCESS)) {
				global_p2p_connection_status =
					P2P_GO_NEG_COMPLETED;
				hddLog(LOGE,
				       "[P2P State]GO nego progress to GO nego"
				       " completed state");
			}
		}
	}
#endif

	noack = dont_wait_for_ack;

	/* If the wait is coming as 0 with off channel set */
	/* then set the wait to 200 ms */
	if (offchan && !wait) {
		wait = ACTION_FRAME_DEFAULT_WAIT;
		mutex_lock(&cfgState->remain_on_chan_ctx_lock);
		if (cfgState->remain_on_chan_ctx) {

			uint32_t current_time = qdf_mc_timer_get_system_time();
			int remaining_roc_time =
				((int) cfgState->remain_on_chan_ctx->duration -
				(current_time - pAdapter->start_roc_ts));

			if (remaining_roc_time > ACTION_FRAME_DEFAULT_WAIT)
				wait = remaining_roc_time;
		}
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
	}

	if ((QDF_STA_MODE == pAdapter->device_mode) &&
		(type == SIR_MAC_MGMT_FRAME &&
		subType == SIR_MAC_MGMT_PROBE_RSP)) {
			/* Drop Probe response received
			 * from supplicant in sta mode
			 */
			goto err_rem_channel;
	}

	/* Call sme API to send out a action frame. */
	/* OR can we send it directly through data path?? */
	/* After tx completion send tx status back. */
	if ((QDF_SAP_MODE == pAdapter->device_mode) ||
	    (QDF_P2P_GO_MODE == pAdapter->device_mode)
	    ) {
		if (type == SIR_MAC_MGMT_FRAME) {
			if (subType == SIR_MAC_MGMT_PROBE_RSP) {
				/* Drop Probe response recieved from supplicant, as for GO and
				   SAP PE itself sends probe response
				 */
				goto err_rem_channel;
			} else if ((subType == SIR_MAC_MGMT_DISASSOC) ||
				   (subType == SIR_MAC_MGMT_DEAUTH)) {
				/* During EAP failure or P2P Group Remove supplicant
				 * is sending del_station command to driver. From
				 * del_station function, Driver will send deauth frame to
				 * p2p client. No need to send disassoc frame from here.
				 * so Drop the frame here and send tx indication back to
				 * supplicant.
				 */
				uint8_t dstMac[ETH_ALEN] = { 0 };
				memcpy(&dstMac,
				       &buf[WLAN_HDD_80211_FRM_DA_OFFSET],
				       ETH_ALEN);
				hddLog(QDF_TRACE_LEVEL_INFO,
				       "%s: Deauth/Disassoc received for STA:"
				       MAC_ADDRESS_STR, __func__,
				       MAC_ADDR_ARRAY(dstMac));
				goto err_rem_channel;
			}
		}
	}

	if (NULL != cfgState->buf) {
		if (!noack) {
			hddLog(LOGE,
			       "(%s):Previous P2P Action frame packet pending",
			       __func__);
			hdd_cleanup_actionframe(pAdapter->pHddCtx, pAdapter);
		} else {
			hddLog(LOGE,
			       "(%s):Pending Action frame packet return EBUSY",
			       __func__);
			return -EBUSY;
		}
	}

	if (subType == SIR_MAC_MGMT_ACTION) {
		hddLog(LOG1, "Action frame tx request : %s",
		       hdd_get_action_string(buf
					     [WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET]));
	}

	if (pAdapter->device_mode == QDF_SAP_MODE) {
		home_ch = pAdapter->sessionCtx.ap.operatingChannel;
	} else if (pAdapter->device_mode == QDF_STA_MODE) {
		home_ch =
			pAdapter->sessionCtx.station.conn_info.operationChannel;
	} else {
		goAdapter = hdd_get_adapter(pAdapter->pHddCtx, QDF_P2P_GO_MODE);
		if (goAdapter)
			home_ch = goAdapter->sessionCtx.ap.operatingChannel;
	}

	if (chan &&
	    (ieee80211_frequency_to_channel(chan->center_freq) ==
	     home_ch)) {
		/* if adapter is already on requested ch, no need for ROC */
		wait = 0;
		hddLog(LOG1,
			FL("Adapter already on requested ch. No ROC needed"));
		goto send_frame;
	}

	if (offchan && wait && chan) {
		int status;
		rem_on_channel_request_type_t req_type = OFF_CHANNEL_ACTION_TX;
		/* In case of P2P Client mode if we are already */
		/* on the same channel then send the frame directly */

		mutex_lock(&cfgState->remain_on_chan_ctx_lock);
		pRemainChanCtx = cfgState->remain_on_chan_ctx;
		if ((type == SIR_MAC_MGMT_FRAME) &&
		    (subType == SIR_MAC_MGMT_ACTION) &&
		    hdd_p2p_is_action_type_rsp(&buf
					       [WLAN_HDD_PUBLIC_ACTION_FRAME_BODY_OFFSET])
		    && cfgState->remain_on_chan_ctx
		    && cfgState->current_freq == chan->center_freq) {
			if (QDF_TIMER_STATE_RUNNING ==
			    qdf_mc_timer_get_current_state(&cfgState->
						   remain_on_chan_ctx->
						   hdd_remain_on_chan_timer)) {
				qdf_mc_timer_stop(&cfgState->
						  remain_on_chan_ctx->
						  hdd_remain_on_chan_timer);
				status =
					qdf_mc_timer_start(&cfgState->
							   remain_on_chan_ctx->
							   hdd_remain_on_chan_timer,
							   wait);
				if (status != QDF_STATUS_SUCCESS) {
					hddLog(LOGE,
					       "%s: Remain on Channel timer start failed",
					       __func__);
				}
				mutex_unlock(&cfgState->
					     remain_on_chan_ctx_lock);
				goto send_frame;
			} else {
				if (pRemainChanCtx->
				    hdd_remain_on_chan_cancel_in_progress ==
				    true) {
					mutex_unlock(&cfgState->
						     remain_on_chan_ctx_lock);
					hddLog(QDF_TRACE_LEVEL_INFO,
					       "action frame tx: waiting for completion of ROC ");

					rc = wait_for_completion_timeout
						     (&pAdapter->cancel_rem_on_chan_var,
						     msecs_to_jiffies
							     (WAIT_CANCEL_REM_CHAN));
					if (!rc) {
						hddLog(LOGE,
						       "%s:wait on cancel_rem_on_chan_var timed out",
						       __func__);
					}

				} else
					mutex_unlock(&cfgState->
						     remain_on_chan_ctx_lock);
			}
		} else
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);

		if ((cfgState->remain_on_chan_ctx != NULL) &&
		    (cfgState->current_freq == chan->center_freq)
		    ) {
			hddLog(LOG1, "action frame: extending the wait time");
			extendedWait = (uint16_t) wait;
			goto send_frame;
		}

		INIT_COMPLETION(pAdapter->offchannel_tx_event);

		status = wlan_hdd_request_remain_on_channel(wiphy, dev, chan,
							    wait, cookie,
							    req_type);
		if (0 != status) {
			if ((-EBUSY == status) &&
			    (cfgState->current_freq == chan->center_freq)) {
				goto send_frame;
			}
			goto err_rem_channel;
		}
		/* This will extend timer in LIM when sending Any action frame
		 * It will cover remain on channel timer till next action frame
		 * in rx direction.
		 */
		extendedWait = (uint16_t) wait;
		/* Wait for driver to be ready on the requested channel */
		rc = wait_for_completion_timeout(&pAdapter->offchannel_tx_event,
						 msecs_to_jiffies
							 (WAIT_CHANGE_CHANNEL_FOR_OFFCHANNEL_TX));
		if (!rc) {
			hddLog(LOGE, "wait on offchannel_tx_event timed out");
			goto err_rem_channel;
		}
	} else if (offchan) {
		/* Check before sending action frame
		   whether we already remain on channel */
		if (NULL == cfgState->remain_on_chan_ctx) {
			goto err_rem_channel;
		}
	}
send_frame:

	if (!noack) {
		cfgState->buf = qdf_mem_malloc(len);    /* buf; */
		if (cfgState->buf == NULL)
			return -ENOMEM;

		cfgState->len = len;

		qdf_mem_copy(cfgState->buf, buf, len);

		mutex_lock(&cfgState->remain_on_chan_ctx_lock);

		if (cfgState->remain_on_chan_ctx) {
			cfgState->action_cookie =
				cfgState->remain_on_chan_ctx->cookie;
			*cookie = cfgState->action_cookie;
		} else {
			*cookie = (uintptr_t) cfgState->buf;
			cfgState->action_cookie = *cookie;
		}

		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
	}

	/*
	 * Firmware needs channel information for action frames
	 * which are not sent on the current operating channel of VDEV
	 */
	if ((QDF_P2P_DEVICE_MODE == pAdapter->device_mode) ||
		(QDF_P2P_CLIENT_MODE == pAdapter->device_mode) ||
		(QDF_P2P_GO_MODE == pAdapter->device_mode)) {
		if (chan && (chan->center_freq != 0))
			current_freq = chan->center_freq;
		else
			current_freq = cfgState->current_freq;
	} else {
		current_freq = 0;
	}

	INIT_COMPLETION(pAdapter->tx_action_cnf_event);

	if ((QDF_STA_MODE == pAdapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == pAdapter->device_mode) ||
	    (QDF_P2P_DEVICE_MODE == pAdapter->device_mode)
	    ) {
		uint8_t sessionId = pAdapter->sessionId;

		if ((type == SIR_MAC_MGMT_FRAME) &&
		    (subType == SIR_MAC_MGMT_ACTION) &&
		    (buf[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET] ==
		     WLAN_HDD_PUBLIC_ACTION_FRAME)) {
			actionFrmType =
				buf[WLAN_HDD_PUBLIC_ACTION_FRAME_TYPE_OFFSET];
			hddLog(LOG1, "Tx Action Frame %u", actionFrmType);
			if (actionFrmType == WLAN_HDD_PROV_DIS_REQ) {
				cfgState->actionFrmState =
					HDD_PD_REQ_ACK_PENDING;
				hddLog(LOG1, "%s: HDD_PD_REQ_ACK_PENDING",
				       __func__);
			} else if (actionFrmType == WLAN_HDD_GO_NEG_REQ) {
				cfgState->actionFrmState =
					HDD_GO_NEG_REQ_ACK_PENDING;
				hddLog(LOG1, "%s: HDD_GO_NEG_REQ_ACK_PENDING",
				       __func__);
			}
		}

		if (QDF_STATUS_SUCCESS !=
		    sme_send_action(WLAN_HDD_GET_HAL_CTX(pAdapter),
				    sessionId, buf, len, extendedWait, noack,
				    current_freq)) {
			QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
				  "%s: sme_send_action returned fail", __func__);
			goto err;
		}
	} else if (QDF_SAP_MODE == pAdapter->device_mode ||
		   QDF_P2P_GO_MODE == pAdapter->device_mode) {
		if (QDF_STATUS_SUCCESS !=
#ifdef WLAN_FEATURE_MBSSID
		    wlansap_send_action(WLAN_HDD_GET_SAP_CTX_PTR(pAdapter),
#else
		    wlansap_send_action((WLAN_HDD_GET_CTX(pAdapter))->
					pcds_context,
#endif
					buf, len, 0, current_freq)) {
			hddLog(LOGE,
				FL("wlansap_send_action returned fail"));
			goto err;
		}
	}

	return 0;
err:
	if (!noack) {
		hdd_send_action_cnf(pAdapter, false);
	}
	return 0;
err_rem_channel:
	*cookie = (uintptr_t) cfgState;
	cfg80211_mgmt_tx_status(
		pAdapter->dev->ieee80211_ptr,
		*cookie, buf, len, false, GFP_KERNEL);
	EXIT();
	return 0;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
int wlan_hdd_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
		     struct cfg80211_mgmt_tx_params *params, u64 *cookie)
#else
int wlan_hdd_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
		     struct ieee80211_channel *chan, bool offchan,
		     unsigned int wait,
		     const u8 *buf, size_t len, bool no_cck,
		     bool dont_wait_for_ack, u64 *cookie)
#endif /* LINUX_VERSION_CODE */
{
	int ret;

	cds_ssr_protect(__func__);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 14, 0))
	ret = __wlan_hdd_mgmt_tx(wiphy, wdev, params->chan, params->offchan,
				 params->wait, params->buf, params->len,
				 params->no_cck, params->dont_wait_for_ack,
				 cookie);
#else
	ret = __wlan_hdd_mgmt_tx(wiphy, wdev, chan, offchan,
				 wait, buf, len, no_cck,
				 dont_wait_for_ack, cookie);
#endif /* LINUX_VERSION_CODE */
	cds_ssr_unprotect(__func__);

	return ret;
}

int __wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
					    struct wireless_dev *wdev,
					    u64 cookie)
{
	return wlan_hdd_cfg80211_cancel_remain_on_channel(wiphy, wdev, cookie);
}

int wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
					  struct wireless_dev *wdev, u64 cookie)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_mgmt_tx_cancel_wait(wiphy, wdev, cookie);
	cds_ssr_unprotect(__func__);

	return ret;
}

void hdd_send_action_cnf(hdd_adapter_t *pAdapter, bool actionSendSuccess)
{
	hdd_cfg80211_state_t *cfgState = WLAN_HDD_GET_CFG_STATE_PTR(pAdapter);

	cfgState->actionFrmState = HDD_IDLE;

	hddLog(LOG1, "Send Action cnf, actionSendSuccess %d",
	       actionSendSuccess);

	if (NULL == cfgState->buf) {
		return;
	}

	/*
	 * buf is the same pointer it passed us to send. Since we are sending
	 * it through control path, we use different buffers.
	 * In case of mac80211, they just push it to the skb and pass the same
	 * data while sending tx ack status.
	 * */
	cfg80211_mgmt_tx_status(
		pAdapter->dev->ieee80211_ptr,
		cfgState->action_cookie,
		cfgState->buf, cfgState->len,
		actionSendSuccess, GFP_KERNEL);

	qdf_mem_free(cfgState->buf);
	cfgState->buf = NULL;

	complete(&pAdapter->tx_action_cnf_event);
}

/**
 * hdd_set_p2p_noa
 *
 ***FUNCTION:
 * This function is called from hdd_hostapd_ioctl function when Driver
 * get P2P_SET_NOA comand from wpa_supplicant using private ioctl
 *
 ***LOGIC:
 * Fill NoA Struct According to P2P Power save Option and Pass it to SME layer
 *
 ***ASSUMPTIONS:
 *
 *
 ***NOTE:
 *
 * @param dev          Pointer to net device structure
 * @param command      Pointer to command
 *
 * @return Status
 */

int hdd_set_p2p_noa(struct net_device *dev, uint8_t *command)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	tP2pPsConfig NoA;
	int count, duration, interval;
	char *param;
	int ret;

	param = strnchr(command, strlen(command), ' ');
	if (param == NULL) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: strnchr failed to find delimeter", __func__);
		return -EINVAL;
	}
	param++;
	ret = sscanf(param, "%d %d %d", &count, &interval, &duration);
	if (ret != 3) {
		QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_ERROR,
			  "%s: P2P_SET GO NoA: fail to read params, ret=%d",
			  __func__, ret);
		return -EINVAL;
	}
	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: P2P_SET GO NoA: count=%d interval=%d duration=%d",
		  __func__, count, interval, duration);
	duration = MS_TO_TU_MUS(duration);
	/* PS Selection
	 * Periodic NoA (2)
	 * Single NOA   (4)
	 */
	NoA.opp_ps = 0;
	NoA.ctWindow = 0;
	if (count == 1) {
		NoA.duration = 0;
		NoA.single_noa_duration = duration;
		NoA.psSelection = P2P_POWER_SAVE_TYPE_SINGLE_NOA;
	} else {
		NoA.duration = duration;
		NoA.single_noa_duration = 0;
		NoA.psSelection = P2P_POWER_SAVE_TYPE_PERIODIC_NOA;
	}
	NoA.interval = MS_TO_TU_MUS(interval);
	NoA.count = count;
	NoA.sessionid = pAdapter->sessionId;

	QDF_TRACE(QDF_MODULE_ID_HDD, QDF_TRACE_LEVEL_INFO,
		  "%s: P2P_PS_ATTR:oppPS %d ctWindow %d duration %d "
		  "interval %d count %d single noa duration %d "
		  "PsSelection %x", __func__, NoA.opp_ps,
		  NoA.ctWindow, NoA.duration, NoA.interval,
		  NoA.count, NoA.single_noa_duration, NoA.psSelection);

	sme_p2p_set_ps(hHal, &NoA);
	return 0;
}

/**
 * hdd_set_p2p_opps
 *
 ***FUNCTION:
 * This function is called from hdd_hostapd_ioctl function when Driver
 * get P2P_SET_PS comand from wpa_supplicant using private ioctl
 *
 ***LOGIC:
 * Fill NoA Struct According to P2P Power save Option and Pass it to SME layer
 *
 ***ASSUMPTIONS:
 *
 *
 ***NOTE:
 *
 * @param  dev         Pointer to net device structure
 * @param  command     Pointer to command
 *
 * @return Status
 */

int hdd_set_p2p_opps(struct net_device *dev, uint8_t *command)
{
	hdd_adapter_t *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle handle = WLAN_HDD_GET_HAL_CTX(adapter);
	tP2pPsConfig noa;
	char *param;
	int legacy_ps, opp_ps, ctwindow;
	int ret;

	param = strnchr(command, strlen(command), ' ');
	if (param == NULL) {
		hdd_err("strnchr failed to find delimiter");
		return -EINVAL;
	}
	param++;
	ret = sscanf(param, "%d %d %d", &legacy_ps, &opp_ps, &ctwindow);
	if (ret != 3) {
		hdd_err("P2P_SET GO PS: fail to read params, ret=%d", ret);
		return -EINVAL;
	}

	if ((opp_ps != -1) && (opp_ps != 0) && (opp_ps != 1)) {
		hdd_err("Invalid opp_ps value:%d", opp_ps);
		return -EINVAL;
	}

	/* P2P spec: 3.3.2 Power Management and discovery:
	 *     CTWindow should be at least 10 TU.
	 * P2P spec: Table 27 - CTWindow and OppPS Parameters field format:
	 *     CTWindow and OppPS Parameters together is 8 bits.
	 *     CTWindow uses 7 bits (0-6, Bit 7 is for OppPS)
	 * 0 indicates that there shall be no CTWindow
	 */
	if ((ctwindow != -1) && (ctwindow != 0) &&
	    (!((ctwindow >= 10) && (ctwindow <= 127)))) {
		hdd_err("Invalid CT window value:%d", ctwindow);
		return -EINVAL;
	}

	hdd_info("P2P_SET GO PS: legacy_ps=%d opp_ps=%d ctwindow=%d",
		  legacy_ps, opp_ps, ctwindow);

	/* PS Selection
	 * Opportunistic Power Save (1)
	 */

	/* From wpa_cli user need to use separate command to set ctWindow and Opps
	 * When user want to set ctWindow during that time other parameters
	 * values are coming from wpa_supplicant as -1.
	 * Example : User want to set ctWindow with 30 then wpa_cli command :
	 * P2P_SET ctwindow 30
	 * Command Received at hdd_hostapd_ioctl is as below:
	 * P2P_SET_PS -1 -1 30 (legacy_ps = -1, opp_ps = -1, ctwindow = 30)
	 *
	 * e.g., 1: P2P_SET_PS 1 1 30
	 * Driver sets the Opps and CTwindow as 30 and send it to FW.
	 * e.g., 2: P2P_SET_PS 1 -1 15
	 * Driver caches the CTwindow value but not send the command to FW.
	 * e.g., 3: P2P_SET_PS 1 1 -1
	 * Driver sends the command to FW with Opps enabled and CT window as
	 * 15 (last cached CTWindow value).
	 * (or) : P2P_SET_PS 1 1 20
	 * Driver sends the command to FW with opps enabled and CT window
	 * as 20.
	 *
	 * legacy_ps param remains unused until required in the future.
	 */
	if (ctwindow != -1)
		adapter->ctw = ctwindow;

	/* Send command to FW when OppPS is either enabled(1)/disbaled(0) */
	if (opp_ps != -1) {
		adapter->ops = opp_ps;
		noa.opp_ps = adapter->ops;
		noa.ctWindow = adapter->ctw;
		noa.duration = 0;
		noa.single_noa_duration = 0;
		noa.interval = 0;
		noa.count = 0;
		noa.psSelection = P2P_POWER_SAVE_TYPE_OPPORTUNISTIC;
		noa.sessionid = adapter->sessionId;

		hdd_debug("P2P_PS_ATTR: oppPS %d ctWindow %d duration %d interval %d count %d single noa duration %d PsSelection %x",
			noa.opp_ps, noa.ctWindow,
			noa.duration, noa.interval, noa.count,
			noa.single_noa_duration,
			noa.psSelection);

		sme_p2p_set_ps(handle, &noa);
	}

	return 0;
}

int hdd_set_p2p_ps(struct net_device *dev, void *msgData)
{
	hdd_adapter_t *pAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tHalHandle hHal = WLAN_HDD_GET_HAL_CTX(pAdapter);
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tP2pPsConfig NoA;
	p2p_app_setP2pPs_t *pappNoA = (p2p_app_setP2pPs_t *) msgData;

	NoA.opp_ps = pappNoA->opp_ps;
	NoA.ctWindow = pappNoA->ctWindow;
	NoA.duration = pappNoA->duration;
	NoA.interval = pappNoA->interval;
	NoA.count = pappNoA->count;
	NoA.single_noa_duration = pappNoA->single_noa_duration;
	NoA.psSelection = pappNoA->psSelection;
	NoA.sessionid = pAdapter->sessionId;

	sme_p2p_set_ps(hHal, &NoA);
	return status;
}

static uint8_t wlan_hdd_get_session_type(enum nl80211_iftype type)
{
	uint8_t sessionType;

	switch (type) {
	case NL80211_IFTYPE_AP:
		sessionType = QDF_SAP_MODE;
		break;
	case NL80211_IFTYPE_P2P_GO:
		sessionType = QDF_P2P_GO_MODE;
		break;
	case NL80211_IFTYPE_P2P_CLIENT:
		sessionType = QDF_P2P_CLIENT_MODE;
		break;
	case NL80211_IFTYPE_STATION:
		sessionType = QDF_STA_MODE;
		break;
	default:
		sessionType = QDF_STA_MODE;
		break;
	}

	return sessionType;
}

/**
 * __wlan_hdd_add_virtual_intf() - Add virtual interface
 * @wiphy: wiphy pointer
 * @name: User-visible name of the interface
 * @name_assign_type: the name of assign type of the netdev
 * @nl80211_iftype: (virtual) interface types
 * @flags: moniter configuraiton flags (not used)
 * @vif_params: virtual interface parameters (not used)
 *
 * Return: the pointer of wireless dev, otherwise ERR_PTR.
 */
struct wireless_dev *__wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
						 const char *name,
						 unsigned char name_assign_type,
						 enum nl80211_iftype type,
						 u32 *flags,
						 struct vif_params *params)
{
	hdd_context_t *pHddCtx = (hdd_context_t *) wiphy_priv(wiphy);
	hdd_adapter_t *pAdapter = NULL;
	hdd_scaninfo_t *scan_info = NULL;
	int ret;
	uint8_t session_type;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return ERR_PTR(-EINVAL);
	}

	ret = wlan_hdd_validate_context(pHddCtx);
	if (0 != ret)
		return ERR_PTR(ret);

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_ADD_VIRTUAL_INTF, NO_SESSION, type));
	/*
	 * Allow addition multiple interfaces for QDF_P2P_GO_MODE,
	 * QDF_SAP_MODE, QDF_P2P_CLIENT_MODE and QDF_STA_MODE
	 * session type.
	 */
	session_type = wlan_hdd_get_session_type(type);
	if ((hdd_get_adapter(pHddCtx, session_type) != NULL)
#ifdef WLAN_FEATURE_MBSSID
	    && QDF_SAP_MODE != session_type
	    && QDF_P2P_GO_MODE != session_type
#endif
	    && QDF_P2P_CLIENT_MODE != session_type
	    && QDF_STA_MODE != session_type) {
		hddLog(LOGE,
		       "%s: Interface type %d already exists. "
		       "Two interfaces of same type are not supported currently.",
		       __func__, type);
		return ERR_PTR(-EINVAL);
	}

	wlan_hdd_tdls_disable_offchan_and_teardown_links(pHddCtx);

	pAdapter = hdd_get_adapter(pHddCtx, QDF_STA_MODE);
	if (pAdapter != NULL) {
		scan_info = &pAdapter->scan_info;
		if (scan_info->mScanPending) {
			hdd_abort_mac_scan(pHddCtx, pAdapter->sessionId,
						eCSR_SCAN_ABORT_DEFAULT);
			hddLog(LOG1,
			       FL("Abort Scan while adding virtual interface"));
		}
	}

	pAdapter = NULL;
	if (pHddCtx->config->isP2pDeviceAddrAdministrated &&
	    ((NL80211_IFTYPE_P2P_GO == type) ||
	     (NL80211_IFTYPE_P2P_CLIENT == type))) {
		/*
		 * Generate the P2P Interface Address. this address must be
		 * different from the P2P Device Address.
		 */
		struct qdf_mac_addr p2pDeviceAddress =
						pHddCtx->p2pDeviceAddress;
		p2pDeviceAddress.bytes[4] ^= 0x80;
		pAdapter = hdd_open_adapter(pHddCtx,
					    wlan_hdd_get_session_type(type),
					    name, p2pDeviceAddress.bytes,
					    name_assign_type,
					    true);
	} else {
		pAdapter = hdd_open_adapter(pHddCtx,
					    wlan_hdd_get_session_type(type),
					    name,
					    wlan_hdd_get_intf_addr(pHddCtx),
					    name_assign_type,
					    true);
	}

	if (NULL == pAdapter) {
		hddLog(QDF_TRACE_LEVEL_ERROR, "%s: hdd_open_adapter failed",
		       __func__);
		return ERR_PTR(-ENOSPC);
	}

	/*
	 * Add interface can be requested from the upper layer at any time
	 * check the statemachine for modules state and if they are closed
	 * open the modules.
	 */
	ret = hdd_wlan_start_modules(pHddCtx, pAdapter, false);
	if (ret)
		return ERR_PTR(ret);

	if (NL80211_IFTYPE_AP == type) {
		ret = hdd_start_adapter(pAdapter);
		if (ret) {
			hdd_err("Failed to start %s", name);
			return ERR_PTR(-EINVAL);
		}
	}

	if (pHddCtx->rps)
		hdd_send_rps_ind(pAdapter);

	EXIT();
	return pAdapter->dev->ieee80211_ptr;
}

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) || defined(WITH_BACKPORTS)
/**
 * wlan_hdd_add_virtual_intf() - Add virtual interface wrapper
 * @wiphy: wiphy pointer
 * @name: User-visible name of the interface
 * @name_assign_type: the name of assign type of the netdev
 * @nl80211_iftype: (virtual) interface types
 * @flags: monitor mode configuration flags (not used)
 * @vif_params: virtual interface parameters (not used)
 *
 * Return: the pointer of wireless dev, otherwise ERR_PTR.
 */
struct wireless_dev *wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
					       const char *name,
					       unsigned char name_assign_type,
					       enum nl80211_iftype type,
					       u32 *flags,
					       struct vif_params *params)
{
	struct wireless_dev *wdev;

	cds_ssr_protect(__func__);
	wdev = __wlan_hdd_add_virtual_intf(wiphy, name, name_assign_type,
					   type, flags, params);
	cds_ssr_unprotect(__func__);
	return wdev;

}
#else
/**
 * wlan_hdd_add_virtual_intf() - Add virtual interface wrapper
 * @wiphy: wiphy pointer
 * @name: User-visible name of the interface
 * @nl80211_iftype: (virtual) interface types
 * @flags: monitor mode configuration flags (not used)
 * @vif_params: virtual interface parameters (not used)
 *
 * Return: the pointer of wireless dev, otherwise ERR_PTR.
 */
struct wireless_dev *wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
					       const char *name,
					       enum nl80211_iftype type,
					       u32 *flags,
					       struct vif_params *params)
{
	struct wireless_dev *wdev;
	unsigned char name_assign_type = 0;

	cds_ssr_protect(__func__);
	wdev = __wlan_hdd_add_virtual_intf(wiphy, name, name_assign_type,
					   type, flags, params);
	cds_ssr_unprotect(__func__);
	return wdev;

}
#endif

int __wlan_hdd_del_virtual_intf(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	struct net_device *dev = wdev->netdev;
	hdd_context_t *pHddCtx = (hdd_context_t *) wiphy_priv(wiphy);
	hdd_adapter_t *pVirtAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int status;
	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hddLog(LOGE, FL("Command not allowed in FTM mode"));
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_DEL_VIRTUAL_INTF,
			 pVirtAdapter->sessionId, pVirtAdapter->device_mode));
	hddLog(LOG1, FL("Device_mode %s(%d)"),
	       hdd_device_mode_to_string(pVirtAdapter->device_mode),
	       pVirtAdapter->device_mode);

	status = wlan_hdd_validate_context(pHddCtx);

	if (0 != status)
		return status;

	/*
	 * check state machine state and kickstart modules if they are closed.
	 */
	status = hdd_wlan_start_modules(pHddCtx, pVirtAdapter, false);
	if (status)
		return status;

	wlan_hdd_release_intf_addr(pHddCtx,
				   pVirtAdapter->macAddressCurrent.bytes);

	if ((pVirtAdapter->device_mode == QDF_SAP_MODE) &&
		wlan_sap_is_pre_cac_active(pHddCtx->hHal)) {
		hdd_clean_up_pre_cac_interface(pHddCtx);
	}

	hdd_stop_adapter(pHddCtx, pVirtAdapter, true);
	hdd_close_adapter(pHddCtx, pVirtAdapter, true);
	EXIT();
	return 0;
}

int wlan_hdd_del_virtual_intf(struct wiphy *wiphy, struct wireless_dev *wdev)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_del_virtual_intf(wiphy, wdev);
	cds_ssr_unprotect(__func__);

	return ret;
}

#ifdef WLAN_FEATURE_P2P_DEBUG
/*
 * wlan_hdd_p2p_action_debug() - Log P2P state and update global status
 * @actionFrmType: action frame type
 * @macFrom: peer mac address
 *
 * return: void
 */
static void wlan_hdd_p2p_action_debug(tActionFrmType actionFrmType,
						uint8_t *macFrom)
{
	if (actionFrmType >= MAX_P2P_ACTION_FRAME_TYPE) {
		hdd_err("[P2P] unknown[%d] <--- OTA from " MAC_ADDRESS_STR,
			actionFrmType, MAC_ADDR_ARRAY(macFrom));
	} else {
		hdd_err("[P2P] %s <--- OTA from " MAC_ADDRESS_STR,
			p2p_action_frame_type[actionFrmType],
			MAC_ADDR_ARRAY(macFrom));
		if ((actionFrmType == WLAN_HDD_PROV_DIS_REQ)
		    && (global_p2p_connection_status == P2P_NOT_ACTIVE)) {
			global_p2p_connection_status = P2P_GO_NEG_PROCESS;
			hdd_err("[P2P State]Inactive state to GO negotiation progress state");
		} else
		if ((actionFrmType == WLAN_HDD_GO_NEG_CNF)
		    && (global_p2p_connection_status == P2P_GO_NEG_PROCESS)) {
			global_p2p_connection_status = P2P_GO_NEG_COMPLETED;
			hdd_err("[P2P State]GO negotiation progress to GO negotiation completed state");
		} else
		if ((actionFrmType == WLAN_HDD_INVITATION_REQ)
		    && (global_p2p_connection_status == P2P_NOT_ACTIVE)) {
			global_p2p_connection_status = P2P_GO_NEG_COMPLETED;
			hdd_err("[P2P State]Inactive state to GO negotiation completed state Autonomous GO formation");
		}
	}
}
#else
/*
 * wlan_hdd_p2p_action_debug() - dummy
 * @actionFrmType: action frame type
 * @macFrom: peer mac address
 *
 * return: void
 */
static void wlan_hdd_p2p_action_debug(tActionFrmType actionFrmType,
						uint8_t *macFrom)
{

}
#endif

void __hdd_indicate_mgmt_frame(hdd_adapter_t *pAdapter,
			     uint32_t nFrameLength,
			     uint8_t *pbFrames,
			     uint8_t frameType, uint32_t rxChan, int8_t rxRssi)
{
	uint16_t freq;
	uint16_t extend_time;
	uint8_t type = 0;
	uint8_t subType = 0;
	tActionFrmType actionFrmType;
	hdd_cfg80211_state_t *cfgState = NULL;
	QDF_STATUS status;
	hdd_remain_on_chan_ctx_t *pRemainChanCtx = NULL;
	hdd_context_t *pHddCtx;

	hddLog(QDF_TRACE_LEVEL_INFO, "%s: Frame Type = %d Frame Length = %d",
	       __func__, frameType, nFrameLength);

	if (NULL == pAdapter) {
		hddLog(LOGE, FL("pAdapter is NULL"));
		return;
	}
	pHddCtx = WLAN_HDD_GET_CTX(pAdapter);

	if (0 == nFrameLength) {
		hddLog(LOGE, FL("Frame Length is Invalid ZERO"));
		return;
	}

	if (NULL == pbFrames) {
		hddLog(LOGE, FL("pbFrames is NULL"));
		return;
	}

	type = WLAN_HDD_GET_TYPE_FRM_FC(pbFrames[0]);
	subType = WLAN_HDD_GET_SUBTYPE_FRM_FC(pbFrames[0]);

	/* Get pAdapter from Destination mac address of the frame */
	if ((type == SIR_MAC_MGMT_FRAME) && (subType != SIR_MAC_MGMT_PROBE_REQ)) {
		pAdapter =
			hdd_get_adapter_by_macaddr(WLAN_HDD_GET_CTX(pAdapter),
						   &pbFrames
						   [WLAN_HDD_80211_FRM_DA_OFFSET]);
		if (NULL == pAdapter) {
			/*
			 * Under assumtion that we don't receive any action
			 * frame with BCST as destination,
			 * we are dropping action frame
			 */
			hddLog(LOGP,
			       "pAdapter for action frame is NULL Macaddr = "
			       MAC_ADDRESS_STR,
			       MAC_ADDR_ARRAY(&pbFrames
					      [WLAN_HDD_80211_FRM_DA_OFFSET]));
			hddLog(LOGP,
			       FL("Frame Type = %d Frame Length = %d subType = %d"),
				frameType, nFrameLength, subType);
			return;
		}
	}

	if (NULL == pAdapter->dev) {
		hddLog(LOGE, FL("pAdapter->dev is NULL"));
		return;
	}

	if (WLAN_HDD_ADAPTER_MAGIC != pAdapter->magic) {
		hddLog(LOGE, FL("pAdapter has invalid magic"));
		return;
	}

	/* Channel indicated may be wrong. TODO */
	/* Indicate an action frame. */
	if (rxChan <= MAX_NO_OF_2_4_CHANNELS)
		freq = ieee80211_channel_to_frequency(rxChan,
						      IEEE80211_BAND_2GHZ);
	else
		freq = ieee80211_channel_to_frequency(rxChan,
						      IEEE80211_BAND_5GHZ);

	cfgState = WLAN_HDD_GET_CFG_STATE_PTR(pAdapter);

	if ((type == SIR_MAC_MGMT_FRAME) && (subType == SIR_MAC_MGMT_ACTION)) {
		if (pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET] ==
		    WLAN_HDD_PUBLIC_ACTION_FRAME) {
			/* Public action frame */
			if ((pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET + 1]
			     == SIR_MAC_ACTION_VENDOR_SPECIFIC) &&
			    !qdf_mem_cmp(&pbFrames
					    [WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET
					     + 2], SIR_MAC_P2P_OUI,
					    SIR_MAC_P2P_OUI_SIZE)) {
			/* P2P action frames */
				uint8_t *macFrom = &pbFrames
					[WLAN_HDD_80211_PEER_ADDR_OFFSET];
				actionFrmType =
					pbFrames
					[WLAN_HDD_PUBLIC_ACTION_FRAME_TYPE_OFFSET];
				hddLog(LOG1, "Rx Action Frame %u",
				       actionFrmType);

				wlan_hdd_p2p_action_debug(actionFrmType,
								macFrom);

				mutex_lock(&cfgState->remain_on_chan_ctx_lock);
				pRemainChanCtx = cfgState->remain_on_chan_ctx;
				if (pRemainChanCtx != NULL) {
					if (actionFrmType == WLAN_HDD_GO_NEG_REQ
					    || actionFrmType ==
					    WLAN_HDD_GO_NEG_RESP
					    || actionFrmType ==
					    WLAN_HDD_INVITATION_REQ
					    || actionFrmType ==
					    WLAN_HDD_DEV_DIS_REQ
					    || actionFrmType ==
					    WLAN_HDD_PROV_DIS_REQ) {
						hddLog(LOG1,
						       "Extend RoC timer on reception of Action Frame");

						if ((actionFrmType ==
						     WLAN_HDD_GO_NEG_REQ)
						    || (actionFrmType ==
							WLAN_HDD_GO_NEG_RESP))
							extend_time =
								2 *
								ACTION_FRAME_DEFAULT_WAIT;
						else
							extend_time =
								ACTION_FRAME_DEFAULT_WAIT;

						if (completion_done
							    (&pAdapter->
							    rem_on_chan_ready_event)) {
							if (QDF_TIMER_STATE_RUNNING == qdf_mc_timer_get_current_state(&pRemainChanCtx->hdd_remain_on_chan_timer)) {
								qdf_mc_timer_stop
									(&pRemainChanCtx->
									hdd_remain_on_chan_timer);
								status =
									qdf_mc_timer_start
										(&pRemainChanCtx->
										hdd_remain_on_chan_timer,
										extend_time);
								if (status !=
								    QDF_STATUS_SUCCESS) {
									hddLog
										(LOGE,
										"%s: Remain on Channel timer start failed",
										__func__);
								}
							} else {
								hddLog(LOG1,
								       "%s: Rcvd action frame after timer expired",
								       __func__);
							}
						} else {
							/* Buffer Packet */
							if (pRemainChanCtx->
							    action_pkt_buff.
							    frame_length == 0) {
								pRemainChanCtx->
								action_pkt_buff.
								frame_length
									=
										nFrameLength;
								pRemainChanCtx->
								action_pkt_buff.
								freq = freq;
								pRemainChanCtx->
								action_pkt_buff.
								frame_ptr =
									qdf_mem_malloc
										(nFrameLength);
								qdf_mem_copy
									(pRemainChanCtx->
									action_pkt_buff.
									frame_ptr,
									pbFrames,
									nFrameLength);
								hddLog(LOGE,
								       "%s:"
								       "Action Pkt Cached successfully !!!",
								       __func__);
							} else {
								hddLog(LOGE,
								       "%s:"
								       "Frames are pending. dropping frame !!!",
								       __func__);
							}
							mutex_unlock(&cfgState->
								     remain_on_chan_ctx_lock);
							return;
						}
					}
				}
				mutex_unlock(&cfgState->
					     remain_on_chan_ctx_lock);

				if (((actionFrmType == WLAN_HDD_PROV_DIS_RESP)
				     && (cfgState->actionFrmState ==
					 HDD_PD_REQ_ACK_PENDING))
				    || ((actionFrmType == WLAN_HDD_GO_NEG_RESP)
					&& (cfgState->actionFrmState ==
					    HDD_GO_NEG_REQ_ACK_PENDING))) {
					hddLog(LOG1,
					       "%s: ACK_PENDING and But received RESP for Action frame ",
					       __func__);
					hdd_send_action_cnf(pAdapter, true);
				}
			}
#ifdef FEATURE_WLAN_TDLS
			else if (pbFrames
				 [WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET + 1] ==
				 WLAN_HDD_PUBLIC_ACTION_TDLS_DISC_RESP) {
				u8 *mac = &pbFrames
					[WLAN_HDD_80211_PEER_ADDR_OFFSET];

				hddLog(LOG1,
				       "[TDLS] TDLS Discovery Response,"
				       MAC_ADDRESS_STR " RSSI[%d] <--- OTA",
				       MAC_ADDR_ARRAY(mac), rxRssi);

				wlan_hdd_tdls_set_rssi(pAdapter, mac, rxRssi);
				wlan_hdd_tdls_recv_discovery_resp(pAdapter,
								  mac);
				cds_tdls_tx_rx_mgmt_event(SIR_MAC_ACTION_TDLS,
				   SIR_MAC_ACTION_RX, SIR_MAC_MGMT_ACTION,
				   WLAN_HDD_PUBLIC_ACTION_TDLS_DISC_RESP,
				   &pbFrames[WLAN_HDD_80211_PEER_ADDR_OFFSET]);
			}
#endif
		}

		if (pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET] ==
		    WLAN_HDD_TDLS_ACTION_FRAME) {
			actionFrmType =
				pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET + 1];
			if (actionFrmType >= MAX_TDLS_ACTION_FRAME_TYPE) {
				hddLog(LOG1,
				       "[TDLS] unknown[%d] <--- OTA",
				       actionFrmType);
			} else {
				hddLog(LOG1,
				       "[TDLS] %s <--- OTA",
				       tdls_action_frame_type[actionFrmType]);
			}
			cds_tdls_tx_rx_mgmt_event(SIR_MAC_ACTION_TDLS,
				SIR_MAC_ACTION_RX, SIR_MAC_MGMT_ACTION,
				actionFrmType,
				&pbFrames[WLAN_HDD_80211_PEER_ADDR_OFFSET]);
		}

		if ((pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET] ==
		     WLAN_HDD_QOS_ACTION_FRAME)
		    && (pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET + 1] ==
			WLAN_HDD_QOS_MAP_CONFIGURE)) {
			sme_update_dsc_pto_up_mapping(pHddCtx->hHal,
						      pAdapter->hddWmmDscpToUpMap,
						      pAdapter->sessionId);
		}
	}
	/* Indicate Frame Over Normal Interface */
	hddLog(LOG1, FL("Indicate Frame over NL80211 sessionid : %d, idx :%d"),
			pAdapter->sessionId, pAdapter->dev->ifindex);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	cfg80211_rx_mgmt(pAdapter->dev->ieee80211_ptr, freq, 0, pbFrames,
			 nFrameLength, NL80211_RXMGMT_FLAG_ANSWERED);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
	cfg80211_rx_mgmt(pAdapter->dev->ieee80211_ptr, freq, 0, pbFrames,
			 nFrameLength, NL80211_RXMGMT_FLAG_ANSWERED,
			 GFP_ATOMIC);
#else
	cfg80211_rx_mgmt(pAdapter->dev->ieee80211_ptr, freq, 0,
			 pbFrames, nFrameLength, GFP_ATOMIC);
#endif /* LINUX_VERSION_CODE */
}

