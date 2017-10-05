/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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
#include "wlan_policy_mgr_api.h"
#include "cds_utils.h"

#ifdef CONVERGED_P2P_ENABLE
#include "wlan_p2p_public_struct.h"
#include "wlan_p2p_ucfg_api.h"
#include "wlan_cfg80211_p2p.h"
#else
#include "wma_api.h"
#endif

/* Ms to Time Unit Micro Sec */
#define MS_TO_TU_MUS(x)   ((x) * 1024)
#define MAX_MUS_VAL       (INT_MAX / 1024)

#ifndef CONVERGED_P2P_ENABLE
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
#endif

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
enum p2p_connection_status global_p2p_connection_status = P2P_NOT_ACTIVE;

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

#ifndef CONVERGED_P2P_ENABLE
static bool wlan_hdd_is_type_p2p_action(const u8 *buf)
{
	const u8 *ouiPtr;

	if (buf[WLAN_HDD_PUBLIC_ACTION_FRAME_CATEGORY_OFFSET] !=
	    WLAN_HDD_PUBLIC_ACTION_FRAME)
		return false;

	if (buf[WLAN_HDD_PUBLIC_ACTION_FRAME_ACTION_OFFSET] !=
	    WLAN_HDD_VENDOR_SPECIFIC_ACTION)
		return false;

	ouiPtr = &buf[WLAN_HDD_PUBLIC_ACTION_FRAME_OUI_OFFSET];

	if (WPA_GET_BE24(ouiPtr) != WLAN_HDD_WFA_OUI)
		return false;

	if (buf[WLAN_HDD_PUBLIC_ACTION_FRAME_OUI_TYPE_OFFSET] !=
	    WLAN_HDD_WFA_P2P_OUI_TYPE)
		return false;

	return true;
}

static bool hdd_p2p_is_action_type_rsp(const u8 *buf)
{
	enum action_frm_type actionFrmType;

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
#endif

static
QDF_STATUS wlan_hdd_remain_on_channel_callback(tHalHandle hHal, void *pCtx,
			QDF_STATUS status, uint32_t scan_id)
{
	struct hdd_adapter *adapter = (struct hdd_adapter *) pCtx;
	struct hdd_cfg80211_state *cfgState =
		WLAN_HDD_GET_CFG_STATE_PTR(adapter);
	struct hdd_remain_on_chan_ctx *pRemainChanCtx;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	enum rem_on_channel_request_type req_type;

	if (!hdd_ctx) {
		hdd_err("Invalid HDD context");
		return QDF_STATUS_E_FAILURE;
	}

	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	pRemainChanCtx = cfgState->remain_on_chan_ctx;

	if (pRemainChanCtx == NULL) {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		hdd_warn("No Rem on channel pending for which Rsp is received");
		return QDF_STATUS_SUCCESS;
	}

	hdd_debug("Received remain on channel rsp");
	if (qdf_mc_timer_stop(&pRemainChanCtx->hdd_remain_on_chan_timer)
			!= QDF_STATUS_SUCCESS)
		hdd_err("Failed to stop hdd_remain_on_chan_timer");
	if (qdf_mc_timer_destroy(&pRemainChanCtx->hdd_remain_on_chan_timer)
			!= QDF_STATUS_SUCCESS)
		hdd_err("Failed to destroy hdd_remain_on_chan_timer");
	cfgState->remain_on_chan_ctx = NULL;
	/*
	 * Resetting the roc in progress early ensures that the subsequent
	 * roc requests are immediately processed without being queued
	 */
	adapter->is_roc_inprogress = false;
	qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.roc);
	/*
	 * If the allow suspend is done later, the scheduled roc wil prevent
	 * the system from going into suspend and immediately this logic
	 * will allow the system to go to suspend breaking the exising logic.
	 * Basically, the system must not go into suspend while roc is in
	 * progress.
	 */
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);

	if (REMAIN_ON_CHANNEL_REQUEST == pRemainChanCtx->rem_on_chan_request) {
		if (cfgState->buf)
			hdd_debug("Yet to rcv an ack for one of the tx pkt");

		cfg80211_remain_on_channel_expired(
			pRemainChanCtx->dev->
			ieee80211_ptr,
			pRemainChanCtx->
			cookie,
			&pRemainChanCtx->chan,
			GFP_KERNEL);
		adapter->last_roc_ts =
			(uint64_t)qdf_mc_timer_get_system_time();
	}
	req_type = pRemainChanCtx->rem_on_chan_request;
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);

	if ((QDF_STA_MODE == adapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == adapter->device_mode) ||
	    (QDF_P2P_DEVICE_MODE == adapter->device_mode)
	    ) {
		uint8_t sessionId = adapter->sessionId;

		if (REMAIN_ON_CHANNEL_REQUEST == req_type) {
			sme_deregister_mgmt_frame(hHal, sessionId,
						  (SIR_MAC_MGMT_FRAME << 2) |
						  (SIR_MAC_MGMT_PROBE_REQ << 4),
						  NULL, 0);
		}
	} else if ((QDF_SAP_MODE == adapter->device_mode) ||
		   (QDF_P2P_GO_MODE == adapter->device_mode)
		   ) {
		wlansap_de_register_mgmt_frame(
			WLAN_HDD_GET_SAP_CTX_PTR(adapter),
			(SIR_MAC_MGMT_FRAME << 2) |
			(SIR_MAC_MGMT_PROBE_REQ << 4),
			 NULL, 0);

	}

	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	if (pRemainChanCtx) {
		if (pRemainChanCtx->action_pkt_buff.frame_ptr != NULL
		    && pRemainChanCtx->action_pkt_buff.frame_length != 0) {
			qdf_mem_free(pRemainChanCtx->action_pkt_buff.frame_ptr);
			pRemainChanCtx->action_pkt_buff.frame_ptr = NULL;
			pRemainChanCtx->action_pkt_buff.frame_length = 0;
		}
	}
	qdf_mem_free(pRemainChanCtx);
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
	complete(&adapter->cancel_rem_on_chan_var);
	if (QDF_STATUS_SUCCESS != status)
		complete(&adapter->rem_on_chan_ready_event);

	/* If we schedule work queue to start new RoC before completing
	 * cancel_rem_on_chan_var then the work queue may immediately get
	 * scheduled and update cfgState->remain_on_chan_ctx which is referred
	 * in mgmt_tx. Due to this update the the mgmt_tx may extend the roc
	 * which was already completed. This will lead to mgmt tx failure.
	 * Always schedule below work queue only after completing the
	 * cancel_rem_on_chan_var event.
	 */
	schedule_delayed_work(&hdd_ctx->roc_req_work, 0);

	return QDF_STATUS_SUCCESS;
}

#ifdef CONVERGED_P2P_ENABLE
void wlan_hdd_cancel_existing_remain_on_channel(struct hdd_adapter *adapter)
{
	QDF_STATUS status;

	if (!adapter) {
		hdd_err("null adapter");
		return;
	}

	status = ucfg_p2p_cleanup_roc(adapter->hdd_vdev);
	hdd_debug("status:%d", status);
}
#else
void wlan_hdd_cancel_existing_remain_on_channel(struct hdd_adapter *adapter)
{
	struct hdd_cfg80211_state *cfgState =
		WLAN_HDD_GET_CFG_STATE_PTR(adapter);
	struct hdd_remain_on_chan_ctx *pRemainChanCtx;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	unsigned long rc;
	uint32_t roc_scan_id;

	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	if (cfgState->remain_on_chan_ctx != NULL) {
		hdd_debug("Cancel Existing Remain on Channel");

		if (QDF_TIMER_STATE_RUNNING == qdf_mc_timer_get_current_state(
		    &cfgState->remain_on_chan_ctx->hdd_remain_on_chan_timer)) {
			if (qdf_mc_timer_stop(&cfgState->remain_on_chan_ctx->
				hdd_remain_on_chan_timer) != QDF_STATUS_SUCCESS)
				hdd_err("Failed to stop hdd_remain_on_chan_timer");
		}

		pRemainChanCtx = cfgState->remain_on_chan_ctx;
		if (pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress ==
			true) {
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
			hdd_err("ROC timer cancellation in progress wait for completion");
			rc = wait_for_completion_timeout(&adapter->
							 cancel_rem_on_chan_var,
							 msecs_to_jiffies
								 (WAIT_CANCEL_REM_CHAN));
			if (!rc)
				hdd_err("wait on cancel_rem_on_chan_var timed out");

			return;
		}
		pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress = true;
		roc_scan_id = pRemainChanCtx->scan_id;
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		/* Wait till remain on channel ready indication before
		 * issuing cancel remain on channel request, otherwise
		 * if remain on channel not received and if the driver issues
		 * cancel remain on channel then lim will be in unknown state.
		 */
		rc = wait_for_completion_timeout(&adapter->
						 rem_on_chan_ready_event,
						 msecs_to_jiffies
							 (WAIT_REM_CHAN_READY));
		if (!rc) {
			hdd_err("timeout waiting for remain on channel ready indication");
			cds_flush_logs(WLAN_LOG_TYPE_FATAL,
				WLAN_LOG_INDICATOR_HOST_DRIVER,
				WLAN_LOG_REASON_HDD_TIME_OUT,
				true, false);
		}

		INIT_COMPLETION(adapter->cancel_rem_on_chan_var);

		/* Issue abort remain on chan request to sme.
		 * The remain on channel callback will make sure the
		 * remain_on_chan expired event is sent.
		 */
		if ((QDF_STA_MODE == adapter->device_mode) ||
		    (QDF_P2P_CLIENT_MODE == adapter->device_mode) ||
		    (QDF_P2P_DEVICE_MODE == adapter->device_mode)
		    ) {
			sme_cancel_remain_on_channel(WLAN_HDD_GET_HAL_CTX
							     (adapter),
				adapter->sessionId, roc_scan_id);
		} else if ((QDF_SAP_MODE == adapter->device_mode)
			   || (QDF_P2P_GO_MODE == adapter->device_mode)
			   ) {
			wlansap_cancel_remain_on_channel(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter), roc_scan_id);
		}

		rc = wait_for_completion_timeout(&adapter->
						 cancel_rem_on_chan_var,
						 msecs_to_jiffies
							 (WAIT_CANCEL_REM_CHAN));

		if (!rc)
			hdd_err("timeout waiting for cancel remain on channel ready indication");

		qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.roc);
		hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
	} else
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
}
#endif

int wlan_hdd_check_remain_on_channel(struct hdd_adapter *adapter)
{
	int status = 0;
	struct hdd_cfg80211_state *cfgState =
		WLAN_HDD_GET_CFG_STATE_PTR(adapter);

	if (QDF_P2P_GO_MODE != adapter->device_mode) {
		/* Cancel Existing Remain On Channel */
		/* If no action frame is pending */
		if (cfgState->remain_on_chan_ctx != NULL) {
			/* Check whether Action Frame is pending or not */
			if (cfgState->buf == NULL) {
				wlan_hdd_cancel_existing_remain_on_channel
					(adapter);
			} else {
				hdd_debug("Cannot Cancel Existing Remain on Channel");
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
#ifndef CONVERGED_P2P_ENABLE
static void wlan_hdd_cancel_pending_roc(struct hdd_adapter *adapter)
{
	struct hdd_remain_on_chan_ctx *roc_ctx;
	unsigned long rc;
	struct hdd_cfg80211_state *cfg_state =
		WLAN_HDD_GET_CFG_STATE_PTR(adapter);
	uint32_t roc_scan_id;

	hdd_debug("ROC completion is not received !!!");

	mutex_lock(&cfg_state->remain_on_chan_ctx_lock);
	roc_ctx = cfg_state->remain_on_chan_ctx;

	if (!roc_ctx) {
		mutex_unlock(&cfg_state->remain_on_chan_ctx_lock);
		hdd_debug("roc_ctx is NULL, No pending RoC");
		return;
	}

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
	roc_scan_id = roc_ctx->scan_id;
	mutex_unlock(&cfg_state->remain_on_chan_ctx_lock);

	if (adapter->device_mode == QDF_P2P_GO_MODE) {
		void *sap_ctx = WLAN_HDD_GET_SAP_CTX_PTR(adapter);

		wlansap_cancel_remain_on_channel(sap_ctx, roc_scan_id);
	} else if (adapter->device_mode == QDF_P2P_CLIENT_MODE ||
		   adapter->device_mode == QDF_P2P_DEVICE_MODE) {
		sme_cancel_remain_on_channel(WLAN_HDD_GET_HAL_CTX
				(adapter),
				adapter->sessionId, roc_scan_id);
	}

wait:
	rc = wait_for_completion_timeout(&adapter->cancel_rem_on_chan_var,
			msecs_to_jiffies
			(WAIT_CANCEL_REM_CHAN));
	if (!rc) {
		hdd_err("Timeout occurred while waiting for RoC Cancellation");
		mutex_lock(&cfg_state->remain_on_chan_ctx_lock);
		roc_ctx = cfg_state->remain_on_chan_ctx;
		if (roc_ctx != NULL) {
			cfg_state->remain_on_chan_ctx = NULL;
			if (qdf_mc_timer_stop(&roc_ctx->
				hdd_remain_on_chan_timer)
				!= QDF_STATUS_SUCCESS)
				hdd_err("Failed to stop hdd_remain_on_chan_timer");
			if (qdf_mc_timer_destroy(
				&roc_ctx->hdd_remain_on_chan_timer)
				!= QDF_STATUS_SUCCESS)
				hdd_err("Failed to destroy hdd_remain_on_chan_timer");
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
#endif

/* Clean up RoC context at hdd_stop_adapter*/
#ifdef CONVERGED_P2P_ENABLE
void wlan_hdd_cleanup_remain_on_channel_ctx(struct hdd_adapter *adapter)
{
	QDF_STATUS status;

	if (!adapter) {
		hdd_err("null adapter");
		return;
	}

	status = ucfg_p2p_cleanup_roc(adapter->hdd_vdev);
	hdd_debug("status:%d", status);
}
#else
void wlan_hdd_cleanup_remain_on_channel_ctx(struct hdd_adapter *adapter)
{
	uint8_t retry = 0;
	struct hdd_cfg80211_state *cfgState =
		WLAN_HDD_GET_CFG_STATE_PTR(adapter);

	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	while (adapter->is_roc_inprogress) {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		hdd_debug("ROC in progress for session %d!!!",
			adapter->sessionId);
		msleep(500);
		if (retry++ > 3) {
			wlan_hdd_cancel_pending_roc(adapter);
			/* hold the lock before break from the loop */
			mutex_lock(&cfgState->remain_on_chan_ctx_lock);
			break;
		}
		mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	} /* end of while */
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);

}
#endif

static void wlan_hdd_remain_on_chan_timeout(void *data)
{
	struct hdd_adapter *adapter = (struct hdd_adapter *) data;
	struct hdd_remain_on_chan_ctx *pRemainChanCtx;
	struct hdd_cfg80211_state *cfgState;
	struct hdd_context *hdd_ctx;
	uint32_t roc_scan_id;

	if ((NULL == adapter) ||
	    (WLAN_HDD_ADAPTER_MAGIC != adapter->magic)) {
		hdd_err("adapter is invalid %pK !!!", adapter);
		return;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	cfgState = WLAN_HDD_GET_CFG_STATE_PTR(adapter);
	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	pRemainChanCtx = cfgState->remain_on_chan_ctx;

	if (NULL == pRemainChanCtx) {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		hdd_err("No Remain on channel is pending");
		return;
	}

	if (true == pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress) {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		hdd_err("Cancellation already in progress");
		return;
	}

	pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress = true;
	roc_scan_id = pRemainChanCtx->scan_id;
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
	hdd_debug("Cancel Remain on Channel on timeout");

	if ((QDF_STA_MODE == adapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == adapter->device_mode) ||
	    (QDF_P2P_DEVICE_MODE == adapter->device_mode)) {
		sme_cancel_remain_on_channel(WLAN_HDD_GET_HAL_CTX(adapter),
			adapter->sessionId, roc_scan_id);
	} else if ((QDF_SAP_MODE == adapter->device_mode) ||
		   (QDF_P2P_GO_MODE == adapter->device_mode)) {
		void *sap_ctx = WLAN_HDD_GET_SAP_CTX_PTR(adapter);

		wlansap_cancel_remain_on_channel(sap_ctx, roc_scan_id);
	}

	qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.roc);
	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
}

static int wlan_hdd_execute_remain_on_channel(struct hdd_adapter *adapter,
		      struct hdd_remain_on_chan_ctx *pRemainChanCtx)
{
	struct hdd_cfg80211_state *cfgState =
		WLAN_HDD_GET_CFG_STATE_PTR(adapter);
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	hdd_adapter_list_node_t *pAdapterNode = NULL, *pNext = NULL;
	struct hdd_adapter *adapter_temp;
	QDF_STATUS status;
	bool isGoPresent = false;
	unsigned int duration;


	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	if (adapter->is_roc_inprogress == true) {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		hdd_err("remain on channel request is in execution");
		return -EBUSY;
	}

	cfgState->remain_on_chan_ctx = pRemainChanCtx;
	cfgState->current_freq = pRemainChanCtx->chan.center_freq;
	adapter->is_roc_inprogress = true;
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);

	/* Initialize Remain on chan timer */
	qdf_status =
		qdf_mc_timer_init(&pRemainChanCtx->hdd_remain_on_chan_timer,
				  QDF_TIMER_TYPE_SW,
				  wlan_hdd_remain_on_chan_timeout, adapter);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		hdd_err("Not able to initialize remain_on_chan timer");
		mutex_lock(&cfgState->remain_on_chan_ctx_lock);
		cfgState->remain_on_chan_ctx = NULL;
		adapter->is_roc_inprogress = false;
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		qdf_mem_free(pRemainChanCtx);
		return -EINVAL;
	}

	status = hdd_get_front_adapter(hdd_ctx, &pAdapterNode);
	while (NULL != pAdapterNode && QDF_STATUS_SUCCESS == status) {
		adapter_temp = pAdapterNode->adapter;
		if (adapter_temp->device_mode == QDF_P2P_GO_MODE)
			isGoPresent = true;
		status = hdd_get_next_adapter(hdd_ctx, pAdapterNode, &pNext);
		pAdapterNode = pNext;
	}

	/* Extending duration for proactive extension logic for RoC */
	duration = pRemainChanCtx->duration;
	if (duration < HDD_P2P_MAX_ROC_DURATION) {
		if (isGoPresent == true)
			duration *= P2P_ROC_DURATION_MULTIPLIER_GO_PRESENT;
		else
			duration *= P2P_ROC_DURATION_MULTIPLIER_GO_ABSENT;
	}

	hdd_prevent_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
	qdf_runtime_pm_prevent_suspend(&hdd_ctx->runtime_context.roc);
	INIT_COMPLETION(adapter->rem_on_chan_ready_event);

	/* call sme API to start remain on channel. */
	if ((QDF_STA_MODE == adapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == adapter->device_mode) ||
	    (QDF_P2P_DEVICE_MODE == adapter->device_mode)
	    ) {
		uint8_t sessionId = adapter->sessionId;
		/* call sme API to start remain on channel. */

		if (QDF_STATUS_SUCCESS != sme_remain_on_channel(
				WLAN_HDD_GET_HAL_CTX(adapter),
				sessionId,
				pRemainChanCtx->chan.hw_value, duration,
				wlan_hdd_remain_on_channel_callback,
				adapter,
				(pRemainChanCtx->rem_on_chan_request ==
				 REMAIN_ON_CHANNEL_REQUEST) ? true : false,
				 &pRemainChanCtx->scan_id)) {
			hdd_err("sme_remain_on_channel failed");
			mutex_lock(&cfgState->remain_on_chan_ctx_lock);
			adapter->is_roc_inprogress = false;
			pRemainChanCtx = cfgState->remain_on_chan_ctx;
			hdd_debug("Freeing ROC ctx cfgState->remain_on_chan_ctx=%pK",
				 cfgState->remain_on_chan_ctx);
			if (pRemainChanCtx) {
				if (qdf_mc_timer_destroy(
					&pRemainChanCtx->
						hdd_remain_on_chan_timer)
						!= QDF_STATUS_SUCCESS)
					hdd_err("Failed to destroy hdd_remain_on_chan_timer");
				qdf_mem_free(pRemainChanCtx);
				cfgState->remain_on_chan_ctx = NULL;
			}
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
			qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.
						     roc);
			hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
			return -EINVAL;
		}

		if (REMAIN_ON_CHANNEL_REQUEST ==
		    pRemainChanCtx->rem_on_chan_request) {
			if (QDF_STATUS_SUCCESS != sme_register_mgmt_frame(
						WLAN_HDD_GET_HAL_CTX(adapter),
						sessionId,
						(SIR_MAC_MGMT_FRAME << 2) |
						(SIR_MAC_MGMT_PROBE_REQ << 4),
						NULL, 0))
				hdd_err("sme_register_mgmt_frame failed");
		}

	} else if ((QDF_SAP_MODE == adapter->device_mode) ||
		   (QDF_P2P_GO_MODE == adapter->device_mode)) {
		/* call sme API to start remain on channel. */
		if (QDF_STATUS_SUCCESS != wlansap_remain_on_channel(
			    WLAN_HDD_GET_SAP_CTX_PTR(adapter),
			    pRemainChanCtx->chan.hw_value,
			    duration, wlan_hdd_remain_on_channel_callback,
			    adapter, &pRemainChanCtx->scan_id)) {
			hdd_err("wlansap_remain_on_channel failed");
			mutex_lock(&cfgState->remain_on_chan_ctx_lock);
			adapter->is_roc_inprogress = false;
			pRemainChanCtx = cfgState->remain_on_chan_ctx;
			hdd_debug("Freeing ROC ctx cfgState->remain_on_chan_ctx=%pK",
				 cfgState->remain_on_chan_ctx);
			if (pRemainChanCtx) {
				if (qdf_mc_timer_destroy(
					&pRemainChanCtx->
					hdd_remain_on_chan_timer)
					!= QDF_STATUS_SUCCESS)
					hdd_err("Failed to destroy hdd_remain_on_chan_timer");
				qdf_mem_free(pRemainChanCtx);
				cfgState->remain_on_chan_ctx = NULL;
			}
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
			qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.
						     roc);
			hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);
			return -EINVAL;
		}

		if (QDF_STATUS_SUCCESS != wlansap_register_mgmt_frame(
			WLAN_HDD_GET_SAP_CTX_PTR(adapter),
			(SIR_MAC_MGMT_FRAME << 2) |
			(SIR_MAC_MGMT_PROBE_REQ << 4), NULL, 0)) {
			hdd_err("wlansap_register_mgmt_frame return fail");
			wlansap_cancel_remain_on_channel(
				WLAN_HDD_GET_SAP_CTX_PTR(adapter),
				pRemainChanCtx->scan_id);
			qdf_runtime_pm_allow_suspend(&hdd_ctx->runtime_context.
						     roc);
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
#ifndef CONVERGED_P2P_ENABLE
static int wlan_hdd_roc_request_enqueue(struct hdd_adapter *adapter,
			struct hdd_remain_on_chan_ctx *remain_chan_ctx)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	struct hdd_roc_req *hdd_roc_req;
	QDF_STATUS status;

	/*
	 * "Driver is busy" OR "there is already RoC request inside the queue"
	 * so enqueue this RoC Request and execute sequentially later.
	 */

	hdd_roc_req = qdf_mem_malloc(sizeof(*hdd_roc_req));

	if (NULL == hdd_roc_req) {
		hdd_err("malloc failed for roc req context");
		return -ENOMEM;
	}

	hdd_roc_req->adapter = adapter;
	hdd_roc_req->remain_chan_ctx = remain_chan_ctx;

	/* Enqueue this RoC request */
	qdf_spin_lock(&hdd_ctx->hdd_roc_req_q_lock);
	status = qdf_list_insert_back(&hdd_ctx->hdd_roc_req_q,
					&hdd_roc_req->node);
	qdf_spin_unlock(&hdd_ctx->hdd_roc_req_q_lock);

	if (QDF_STATUS_SUCCESS != status) {
		hdd_err("Not able to enqueue RoC Req context");
		qdf_mem_free(hdd_roc_req);
		return -EINVAL;
	}

	return 0;
}
#endif

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
static void wlan_hdd_indicate_roc_drop(struct hdd_adapter *adapter,
				       struct hdd_remain_on_chan_ctx *ctx)
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
	struct hdd_roc_req *hdd_roc_req;
	struct hdd_context *hdd_ctx =
			container_of(work, struct hdd_context, roc_req_work.work);

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
			hdd_roc_req->adapter,
			hdd_roc_req->remain_chan_ctx);
	if (ret == -EBUSY) {
		hdd_err("dropping RoC request");
		wlan_hdd_indicate_roc_drop(hdd_roc_req->adapter,
					   hdd_roc_req->remain_chan_ctx);
		qdf_mem_free(hdd_roc_req->remain_chan_ctx);
	}
	qdf_mem_free(hdd_roc_req);
}

#ifndef CONVERGED_P2P_ENABLE
static int wlan_hdd_request_remain_on_channel(struct wiphy *wiphy,
					      struct net_device *dev,
					      struct ieee80211_channel *chan,
					      unsigned int duration,
					      u64 *cookie,
					      enum rem_on_channel_request_type
					      request_type)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	struct hdd_remain_on_chan_ctx *pRemainChanCtx;
	bool isBusy = false;
	uint32_t size = 0;
	struct hdd_adapter *sta_adapter;
	int ret;
	int status = 0;

	hdd_debug("Device_mode %s(%d)",
		   hdd_device_mode_to_string(adapter->device_mode),
		   adapter->device_mode);
	hdd_debug("chan(hw_val)0x%x chan(centerfreq) %d, duration %d",
		 chan->hw_value, chan->center_freq, duration);

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;
	if (hdd_is_connection_in_progress(NULL, NULL)) {
		hdd_debug("Connection is in progress");
		isBusy = true;
	}
	pRemainChanCtx = qdf_mem_malloc(sizeof(struct hdd_remain_on_chan_ctx));
	if (NULL == pRemainChanCtx) {
		hdd_err("Not able to allocate memory for Channel context");
		return -ENOMEM;
	}

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
		sta_adapter = hdd_get_adapter(hdd_ctx, QDF_STA_MODE);
		if ((NULL != sta_adapter) &&
			hdd_conn_is_connected(
				WLAN_HDD_GET_STATION_CTX_PTR(sta_adapter))) {
			if (adapter->last_roc_ts != 0 &&
				(((uint64_t)qdf_mc_timer_get_system_time() -
					 adapter->last_roc_ts) <
				hdd_ctx->config->p2p_listen_defer_interval)) {
			if (pRemainChanCtx->duration > HDD_P2P_MAX_ROC_DURATION)
				pRemainChanCtx->duration =
						HDD_P2P_MAX_ROC_DURATION;

			wlan_hdd_roc_request_enqueue(adapter, pRemainChanCtx);
			schedule_delayed_work(&hdd_ctx->roc_req_work,
			msecs_to_jiffies(
				hdd_ctx->config->p2p_listen_defer_interval));
			hdd_debug("Defer interval is %hu, adapter %pK",
				hdd_ctx->config->p2p_listen_defer_interval,
				adapter);
			return 0;
			}
		}
	}

	qdf_spin_lock(&hdd_ctx->hdd_roc_req_q_lock);
	size = qdf_list_size(&(hdd_ctx->hdd_roc_req_q));
	qdf_spin_unlock(&hdd_ctx->hdd_roc_req_q_lock);
	if ((isBusy == false) && (!size)) {
		status = wlan_hdd_execute_remain_on_channel(adapter,
							    pRemainChanCtx);
		if (status == -EBUSY) {
			if (wlan_hdd_roc_request_enqueue(adapter,
							 pRemainChanCtx)) {
				qdf_mem_free(pRemainChanCtx);
				return -EAGAIN;
			}
		}
		return 0;
	}

	if (wlan_hdd_roc_request_enqueue(adapter, pRemainChanCtx)) {
		qdf_mem_free(pRemainChanCtx);
		return -EAGAIN;
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
	if (isBusy == false && adapter->is_roc_inprogress == false) {
		hdd_debug("scheduling delayed work: no connection/roc active");
		schedule_delayed_work(&hdd_ctx->roc_req_work, 0);
	}
	return 0;
}
#endif

#ifdef CONVERGED_P2P_ENABLE
static int __wlan_hdd_cfg80211_remain_on_channel(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 struct ieee80211_channel *chan,
						 unsigned int duration,
						 u64 *cookie)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	QDF_STATUS status;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->sessionId)) {
		hdd_err("invalid session id: %d", adapter->sessionId);
		return -EINVAL;
	}

	status = wlan_cfg80211_roc(adapter->hdd_vdev, chan,
				duration, cookie);
	hdd_info("remain on channel request, status:%d", status);

	return qdf_status_to_os_return(status);
}
#else
static int __wlan_hdd_cfg80211_remain_on_channel(struct wiphy *wiphy,
						 struct wireless_dev *wdev,
						 struct ieee80211_channel *chan,
						 unsigned int duration,
						 u64 *cookie)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_context *hdd_ctx;
	int ret;

	ENTER();

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	ret = wlan_hdd_validate_context(hdd_ctx);
	if (0 != ret)
		return ret;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->sessionId)) {
		hdd_err("invalid session id: %d", adapter->sessionId);
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_REMAIN_ON_CHANNEL,
			 adapter->sessionId, REMAIN_ON_CHANNEL_REQUEST));

	ret = wlan_hdd_request_remain_on_channel(wiphy, dev, chan,
						  duration, cookie,
						  REMAIN_ON_CHANNEL_REQUEST);
	EXIT();
	return ret;
}
#endif

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

void hdd_remain_chan_ready_handler(struct hdd_adapter *adapter,
	uint32_t scan_id)
{
	struct hdd_cfg80211_state *cfgState = NULL;
	struct hdd_remain_on_chan_ctx *pRemainChanCtx = NULL;
	QDF_STATUS status;

	if (NULL == adapter) {
		hdd_err("adapter is NULL");
		return;
	}
	cfgState = WLAN_HDD_GET_CFG_STATE_PTR(adapter);
	hdd_debug("Ready on chan ind %d", scan_id);

	adapter->start_roc_ts = (uint64_t)qdf_mc_timer_get_system_time();
	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	pRemainChanCtx = cfgState->remain_on_chan_ctx;
	if (pRemainChanCtx != NULL) {
		MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
				 TRACE_CODE_HDD_REMAINCHANREADYHANDLER,
				 adapter->sessionId,
				 pRemainChanCtx->duration));
		/* start timer for actual duration */
		if (QDF_TIMER_STATE_RUNNING ==
			qdf_mc_timer_get_current_state(
				&pRemainChanCtx->hdd_remain_on_chan_timer)) {
			hdd_err("Timer Started before ready event!!!");
			if (qdf_mc_timer_stop(&pRemainChanCtx->
						hdd_remain_on_chan_timer)
					!= QDF_STATUS_SUCCESS)
				hdd_err("Failed to stop hdd_remain_on_chan_timer");
		}
		status =
			qdf_mc_timer_start(&pRemainChanCtx->
					   hdd_remain_on_chan_timer,
					   (pRemainChanCtx->duration +
					    COMPLETE_EVENT_PROPOGATE_TIME));
		if (status != QDF_STATUS_SUCCESS)
			hdd_err("Remain on Channel timer start failed");

		if (REMAIN_ON_CHANNEL_REQUEST ==
		    pRemainChanCtx->rem_on_chan_request) {
			cfg80211_ready_on_channel(
				adapter->dev->
				ieee80211_ptr,
				(uintptr_t)
				pRemainChanCtx,
				&pRemainChanCtx->chan,
				pRemainChanCtx->
				duration, GFP_KERNEL);
		} else if (OFF_CHANNEL_ACTION_TX ==
			   pRemainChanCtx->rem_on_chan_request) {
			complete(&adapter->offchannel_tx_event);
		}
		/* Check for cached action frame */
		if (pRemainChanCtx->action_pkt_buff.frame_length != 0) {
			hdd_debug("Sent cached action frame to supplicant");
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
			cfg80211_rx_mgmt(adapter->dev->ieee80211_ptr,
				pRemainChanCtx->action_pkt_buff.freq, 0,
				pRemainChanCtx->action_pkt_buff.frame_ptr,
				pRemainChanCtx->action_pkt_buff.frame_length,
				NL80211_RXMGMT_FLAG_ANSWERED);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
			cfg80211_rx_mgmt(adapter->dev->ieee80211_ptr,
				pRemainChanCtx->action_pkt_buff.freq, 0,
				pRemainChanCtx->action_pkt_buff.frame_ptr,
				pRemainChanCtx->action_pkt_buff.frame_length,
				NL80211_RXMGMT_FLAG_ANSWERED, GFP_ATOMIC);
#else
			cfg80211_rx_mgmt(adapter->dev->ieee80211_ptr,
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
		complete(&adapter->rem_on_chan_ready_event);
	} else {
		hdd_debug("No Pending Remain on channel Request");
	}
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
}

#ifdef CONVERGED_P2P_ENABLE
static int
__wlan_hdd_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     u64 cookie)
{
	QDF_STATUS status;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->sessionId)) {
		hdd_err("invalid session id: %d", adapter->sessionId);
		return -EINVAL;
	}

	status = wlan_cfg80211_cancel_roc(adapter->hdd_vdev, cookie);
	hdd_info("cancel remain on channel, status:%d", status);

	return 0;
}
#else
static int
__wlan_hdd_cfg80211_cancel_remain_on_channel(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     u64 cookie)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_cfg80211_state *cfgState =
		WLAN_HDD_GET_CFG_STATE_PTR(adapter);
	struct hdd_remain_on_chan_ctx *pRemainChanCtx;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	int status;
	int qdf_status;
	unsigned long rc;
	qdf_list_node_t *tmp, *q;
	struct hdd_roc_req *curr_roc_req;
	uint32_t roc_scan_id;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->sessionId)) {
		hdd_err("invalid session id: %d", adapter->sessionId);
		return -EINVAL;
	}
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;
	qdf_spin_lock(&hdd_ctx->hdd_roc_req_q_lock);
	list_for_each_safe(tmp, q, &hdd_ctx->hdd_roc_req_q.anchor) {
		curr_roc_req = list_entry(tmp, struct hdd_roc_req, node);
		if ((uintptr_t) curr_roc_req->remain_chan_ctx == cookie) {
			qdf_status = qdf_list_remove_node(&hdd_ctx->hdd_roc_req_q,
						      (qdf_list_node_t *)
						      curr_roc_req);
			qdf_spin_unlock(&hdd_ctx->hdd_roc_req_q_lock);
			if (qdf_status == QDF_STATUS_SUCCESS) {
				qdf_mem_free(curr_roc_req->remain_chan_ctx);
				qdf_mem_free(curr_roc_req);
			}
			return 0;
		}
	}
	qdf_spin_unlock(&hdd_ctx->hdd_roc_req_q_lock);
	/* FIXME cancel currently running remain on chan.
	 * Need to check cookie and cancel accordingly
	 */
	mutex_lock(&cfgState->remain_on_chan_ctx_lock);
	pRemainChanCtx = cfgState->remain_on_chan_ctx;

	if (pRemainChanCtx) {
		hdd_debug("action_cookie = %08llx, roc cookie = %08llx, cookie = %08llx",
				cfgState->action_cookie, pRemainChanCtx->cookie,
				cookie);

		if (pRemainChanCtx->cookie == cookie) {
			/* request to cancel on-going roc */
			if (cfgState->buf) {
				/* Tx frame pending */
				if (cfgState->action_cookie != cookie) {
					hdd_debug("Cookie matched with RoC cookie but not with tx cookie, indicate expired event for roc");
					/* RoC was extended to accomodate the tx frame */
					if (REMAIN_ON_CHANNEL_REQUEST ==
							pRemainChanCtx->
							rem_on_chan_request) {
					cfg80211_remain_on_channel_expired(
							pRemainChanCtx->dev->
							ieee80211_ptr,
							pRemainChanCtx->cookie,
							&pRemainChanCtx->chan,
							GFP_KERNEL);
					}
					pRemainChanCtx->rem_on_chan_request =
						OFF_CHANNEL_ACTION_TX;
					pRemainChanCtx->cookie =
						cfgState->action_cookie;
					mutex_unlock(&cfgState->
						remain_on_chan_ctx_lock);
					return 0;
				}
			}
		} else if (cfgState->buf && cfgState->action_cookie ==
				cookie) {
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
			hdd_debug("Cookie not matched with RoC cookie but matched with tx cookie, cleanup action frame");
			/*free the buf and return 0*/
			hdd_cleanup_actionframe(hdd_ctx, adapter);
			return 0;
		} else {
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
			hdd_debug("No matching cookie");
			return -EINVAL;
		}
	} else {
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		hdd_debug("RoC context is NULL, return success");
		return 0;
	}
	if (NULL != cfgState->remain_on_chan_ctx) {
		if (qdf_mc_timer_stop(&cfgState->remain_on_chan_ctx->
					hdd_remain_on_chan_timer)
				!= QDF_STATUS_SUCCESS)
			hdd_err("Failed to stop hdd_remain_on_chan_timer");
		if (pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress) {
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
			hdd_debug("ROC timer cancellation in progress, wait for completion");
			rc = wait_for_completion_timeout(&adapter->
							 cancel_rem_on_chan_var,
							 msecs_to_jiffies
								 (WAIT_CANCEL_REM_CHAN));
			if (!rc)
				hdd_err("wait on cancel_rem_on_chan_var timed out");

			return 0;
		}
		pRemainChanCtx->hdd_remain_on_chan_cancel_in_progress = true;
	}
	roc_scan_id = pRemainChanCtx->scan_id;
	mutex_unlock(&cfgState->remain_on_chan_ctx_lock);

	/* wait until remain on channel ready event received
	 * for already issued remain on channel request
	 */
	rc = wait_for_completion_timeout(&adapter->rem_on_chan_ready_event,
					 msecs_to_jiffies(WAIT_REM_CHAN_READY));
	if (!rc) {
		hdd_err("timeout waiting for remain on channel ready indication");

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
	INIT_COMPLETION(adapter->cancel_rem_on_chan_var);
	/* Issue abort remain on chan request to sme.
	 * The remain on channel callback will make sure the remain_on_chan
	 * expired event is sent.
	 */
	if ((QDF_STA_MODE == adapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == adapter->device_mode) ||
	    (QDF_P2P_DEVICE_MODE == adapter->device_mode)
	    ) {
		uint8_t sessionId = adapter->sessionId;

		sme_cancel_remain_on_channel(WLAN_HDD_GET_HAL_CTX(adapter),
			sessionId, roc_scan_id);
	} else if ((QDF_SAP_MODE == adapter->device_mode) ||
		   (QDF_P2P_GO_MODE == adapter->device_mode)
		   ) {
		wlansap_cancel_remain_on_channel(
			WLAN_HDD_GET_SAP_CTX_PTR(adapter), roc_scan_id);

	} else {
		hdd_err("Invalid device_mode %s(%d)",
			hdd_device_mode_to_string(adapter->device_mode),
			adapter->device_mode);
		return -EIO;
	}
	rc = wait_for_completion_timeout(&adapter->cancel_rem_on_chan_var,
					 msecs_to_jiffies
						 (WAIT_CANCEL_REM_CHAN));
	if (!rc)
		hdd_err("wait on cancel_rem_on_chan_var timed out");

	hdd_allow_suspend(WIFI_POWER_EVENT_WAKELOCK_ROC);

	EXIT();
	return 0;
}
#endif

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

#ifdef CONVERGED_P2P_ENABLE
static int __wlan_hdd_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
			      struct ieee80211_channel *chan, bool offchan,
			      unsigned int wait,
			      const u8 *buf, size_t len, bool no_cck,
			      bool dont_wait_for_ack, u64 *cookie)
{
	QDF_STATUS status;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->sessionId)) {
		hdd_err("invalid session id: %d", adapter->sessionId);
		return -EINVAL;
	}

	status = wlan_cfg80211_mgmt_tx(adapter->hdd_vdev, chan,
			offchan, wait, buf, len, no_cck,
			dont_wait_for_ack, cookie);
	hdd_info("mgmt tx, status:%d", status);

	return 0;
}
#else
static int __wlan_hdd_mgmt_tx(struct wiphy *wiphy, struct wireless_dev *wdev,
			      struct ieee80211_channel *chan, bool offchan,
			      unsigned int wait,
			      const u8 *buf, size_t len, bool no_cck,
			      bool dont_wait_for_ack, u64 *cookie)
{
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	struct hdd_cfg80211_state *cfgState =
		WLAN_HDD_GET_CFG_STATE_PTR(adapter);
	struct hdd_remain_on_chan_ctx *pRemainChanCtx;
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	uint16_t extendedWait = 0;
	uint8_t type = WLAN_HDD_GET_TYPE_FRM_FC(buf[0]);
	uint8_t subType = WLAN_HDD_GET_SUBTYPE_FRM_FC(buf[0]);
	enum action_frm_type actionFrmType;
	bool noack = 0;
	int status;
	unsigned long rc;
	struct hdd_adapter *goAdapter;
	uint16_t current_freq;
	uint8_t home_ch = 0;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->sessionId)) {
		hdd_err("invalid session id: %d", adapter->sessionId);
		return -EINVAL;
	}
	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_ACTION, adapter->sessionId,
			 adapter->device_mode));
	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	hdd_debug("Device_mode %s(%d) type: %d, wait: %d, offchan: %d, category: %d, actionId: %d",
		   hdd_device_mode_to_string(adapter->device_mode),
		   adapter->device_mode, type, wait, offchan,
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
			hdd_debug("[P2P] unknown[%d] ---> OTA to " MAC_ADDRESS_STR,
				actionFrmType,
				MAC_ADDR_ARRAY(&buf
					       [WLAN_HDD_80211_FRM_DA_OFFSET]));
		} else {
			hdd_debug("[P2P] %s ---> OTA to "
			       MAC_ADDRESS_STR,
			       p2p_action_frame_type[actionFrmType],
			       MAC_ADDR_ARRAY(&buf
					      [WLAN_HDD_80211_FRM_DA_OFFSET]));
			if ((actionFrmType == WLAN_HDD_PROV_DIS_REQ)
			    && (global_p2p_connection_status == P2P_NOT_ACTIVE)) {
				global_p2p_connection_status = P2P_GO_NEG_PROCESS;
				hdd_debug("[P2P State]Inactive state to GO negotiation progress state");
			} else if ((actionFrmType == WLAN_HDD_GO_NEG_CNF) &&
				   (global_p2p_connection_status ==
				    P2P_GO_NEG_PROCESS)) {
				global_p2p_connection_status =
					P2P_GO_NEG_COMPLETED;
				hdd_debug("[P2P State]GO nego progress to GO nego completed state");
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
			uint64_t current_time =
				(uint64_t)qdf_mc_timer_get_system_time();
			int remaining_roc_time =
				((int) cfgState->remain_on_chan_ctx->duration -
				(current_time - adapter->start_roc_ts));

			if (remaining_roc_time > ACTION_FRAME_DEFAULT_WAIT)
				wait = remaining_roc_time;
		}
		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
	}

	if ((QDF_STA_MODE == adapter->device_mode) &&
	    (type == SIR_MAC_MGMT_FRAME &&
	     subType == SIR_MAC_MGMT_PROBE_RSP)) {
		/*
		 * Drop Probe response received
		 * from supplicant in sta mode
		 */
		goto err_rem_channel;
	}

	/*
	 * Call sme API to send out a action frame.
	 * OR can we send it directly through data path??
	 * After tx completion send tx status back.
	 */
	if ((QDF_SAP_MODE == adapter->device_mode) ||
	    (QDF_P2P_GO_MODE == adapter->device_mode)
	    ) {
		if (type == SIR_MAC_MGMT_FRAME) {
			if (subType == SIR_MAC_MGMT_PROBE_RSP) {
				/* Drop Probe response recieved from supplicant,
				 * as for GO and SAP PE itself
				 * sends probe response
				 */
				goto err_rem_channel;
			} else if ((subType == SIR_MAC_MGMT_DISASSOC) ||
				   (subType == SIR_MAC_MGMT_DEAUTH)) {
				/*
				 * During EAP failure or P2P Group
				 * Remove supplicant is sending
				 * del_station command to driver. From
				 * del_station function, Driver will
				 * send deauth frame to p2p client. No
				 * need to send disassoc frame from
				 * here.  so Drop the frame here and
				 * send tx indication back to
				 * supplicant.
				 */
				uint8_t dstMac[ETH_ALEN] = { 0 };

				memcpy(&dstMac,
				       &buf[WLAN_HDD_80211_FRM_DA_OFFSET],
				       ETH_ALEN);
				hdd_debug("Deauth/Disassoc received for STA:"
					 MAC_ADDRESS_STR,
					 MAC_ADDR_ARRAY(dstMac));
				goto err_rem_channel;
			}
		}
	}

	if (NULL != cfgState->buf) {
		if (!noack) {
			hdd_err("Previous P2P Action frame packet pending");
			hdd_cleanup_actionframe(hdd_ctx, adapter);
		} else {
			hdd_err("Pending Action frame packet return EBUSY");
			return -EBUSY;
		}
	}

	if (subType == SIR_MAC_MGMT_ACTION) {
		hdd_debug("Action frame tx request : %s",
			   hdd_get_action_string(buf
						 [WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET]));
	}

	if ((adapter->device_mode == QDF_SAP_MODE) &&
	    (test_bit(SOFTAP_BSS_STARTED, &adapter->event_flags))) {
		home_ch = adapter->sessionCtx.ap.operatingChannel;
	} else if ((adapter->device_mode == QDF_STA_MODE) &&
		   (adapter->sessionCtx.station.conn_info.connState ==
				eConnectionState_Associated)) {
		home_ch =
			adapter->sessionCtx.station.conn_info.operationChannel;
	} else {
		goAdapter = hdd_get_adapter(hdd_ctx, QDF_P2P_GO_MODE);
		if (goAdapter &&
		    (test_bit(SOFTAP_BSS_STARTED, &goAdapter->event_flags)))
			home_ch = goAdapter->sessionCtx.ap.operatingChannel;
	}

	if (chan &&
	    (ieee80211_frequency_to_channel(chan->center_freq) ==
	     home_ch)) {
		/* if adapter is already on requested ch, no need for ROC */
		wait = 0;
		hdd_debug("Adapter already on requested ch. No ROC needed");
		goto send_frame;
	}

	if (offchan && wait && chan) {
		int status;
		enum rem_on_channel_request_type req_type = OFF_CHANNEL_ACTION_TX;
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

				/* In the latest wpa_supplicant, the wait time
				 * for go negotiation response is set to 100ms,
				 * due to which, there could be a possibility
				 * that, if the go negotaition confirmation
				 * frame is not received within 100 msec, ROC
				 * would be timeout and resulting in connection
				 * failures as the device will not be on the
				 * listen channel anymore to receive the conf
				 * frame. Also wpa_supplicant has set the wait
				 * to 50msec for go negotiation confirmation,
				 * invitation response and prov discovery rsp
				 * frames. So increase the wait time for all
				 * these frames.
				 */
				actionFrmType = buf
				[WLAN_HDD_PUBLIC_ACTION_FRAME_TYPE_OFFSET];
				if (actionFrmType == WLAN_HDD_GO_NEG_RESP ||
				    actionFrmType == WLAN_HDD_PROV_DIS_RESP)
					wait = wait + ACTION_FRAME_RSP_WAIT;
				else if (actionFrmType ==
					 WLAN_HDD_GO_NEG_CNF ||
					 actionFrmType ==
					 WLAN_HDD_INVITATION_RESP)
					wait = wait + ACTION_FRAME_ACK_WAIT;

				hdd_debug("Extending the wait time %d for actionFrmType=%d",
					 wait, actionFrmType);

				if (qdf_mc_timer_stop(&cfgState->
						remain_on_chan_ctx->
						hdd_remain_on_chan_timer)
						!= QDF_STATUS_SUCCESS)
					hdd_err("Failed to stop hdd_remain_on_chan_timer");
				status =
					qdf_mc_timer_start(&cfgState->
							   remain_on_chan_ctx->
							   hdd_remain_on_chan_timer,
							   wait);
				if (status != QDF_STATUS_SUCCESS)
					hdd_warn("Remain on Channel timer start failed");

				mutex_unlock(&cfgState->
					     remain_on_chan_ctx_lock);
				goto send_frame;
			} else {
				if (pRemainChanCtx->
				    hdd_remain_on_chan_cancel_in_progress ==
				    true) {
					mutex_unlock(&cfgState->
						     remain_on_chan_ctx_lock);
					hdd_debug("action frame tx: waiting for completion of ROC ");

					rc = wait_for_completion_timeout
						     (&adapter->cancel_rem_on_chan_var,
						     msecs_to_jiffies
							     (WAIT_CANCEL_REM_CHAN));
					if (!rc)
						hdd_warn("wait on cancel_rem_on_chan_var timed out");
				} else
					mutex_unlock(&cfgState->
						     remain_on_chan_ctx_lock);
			}
		} else
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);

		mutex_lock(&cfgState->remain_on_chan_ctx_lock);
		pRemainChanCtx = cfgState->remain_on_chan_ctx;

		/* At this point if remain_on_chan_ctx exists but timer not
		 * running means that roc workqueue requested a new RoC and it
		 * is in progress. So wait for Ready on channel indication
		 */
		if ((pRemainChanCtx) &&
			(QDF_TIMER_STATE_RUNNING !=
			 qdf_mc_timer_get_current_state(
				 &pRemainChanCtx->hdd_remain_on_chan_timer))) {
			hdd_debug("remain_on_chan_ctx exists but RoC timer not running. wait for ready on channel");
			rc = wait_for_completion_timeout(&adapter->
					rem_on_chan_ready_event,
					msecs_to_jiffies
					(WAIT_REM_CHAN_READY));
			if (!rc)
				hdd_err("timeout waiting for remain on channel ready indication");
		}

		if ((pRemainChanCtx != NULL) &&
			(cfgState->current_freq == chan->center_freq)) {
			mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
			hdd_debug("action frame: extending the wait time");
			extendedWait = (uint16_t) wait;
			goto send_frame;
		}

		mutex_unlock(&cfgState->remain_on_chan_ctx_lock);
		INIT_COMPLETION(adapter->offchannel_tx_event);

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
		rc = wait_for_completion_timeout(&adapter->offchannel_tx_event,
						 msecs_to_jiffies
							 (WAIT_CHANGE_CHANNEL_FOR_OFFCHANNEL_TX));
		if (!rc) {
			hdd_err("wait on offchannel_tx_event timed out");
			goto err_rem_channel;
		}
	} else if (offchan) {
		/*
		 * Check before sending action frame
		 * whether we already remain on channel
		 */
		if (NULL == cfgState->remain_on_chan_ctx)
			goto err_rem_channel;
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
	if ((QDF_P2P_DEVICE_MODE == adapter->device_mode) ||
		(QDF_P2P_CLIENT_MODE == adapter->device_mode) ||
		(QDF_P2P_GO_MODE == adapter->device_mode)) {
		if (chan && (chan->center_freq != 0))
			current_freq = chan->center_freq;
		else
			current_freq = cfgState->current_freq;
	} else {
		current_freq = 0;
	}

	INIT_COMPLETION(adapter->tx_action_cnf_event);

	if ((QDF_STA_MODE == adapter->device_mode) ||
	    (QDF_P2P_CLIENT_MODE == adapter->device_mode) ||
	    (QDF_P2P_DEVICE_MODE == adapter->device_mode)) {
		uint8_t sessionId = adapter->sessionId;

		if ((type == SIR_MAC_MGMT_FRAME) &&
		    (subType == SIR_MAC_MGMT_ACTION) &&
		     wlan_hdd_is_type_p2p_action(&buf
				[WLAN_HDD_PUBLIC_ACTION_FRAME_BODY_OFFSET])) {
			actionFrmType =
				buf[WLAN_HDD_PUBLIC_ACTION_FRAME_TYPE_OFFSET];
			hdd_debug("Tx Action Frame %u", actionFrmType);
			if (actionFrmType == WLAN_HDD_PROV_DIS_REQ) {
				cfgState->actionFrmState =
					HDD_PD_REQ_ACK_PENDING;
				hdd_debug("HDD_PD_REQ_ACK_PENDING");
			} else if (actionFrmType == WLAN_HDD_GO_NEG_REQ) {
				cfgState->actionFrmState =
					HDD_GO_NEG_REQ_ACK_PENDING;
				hdd_debug("HDD_GO_NEG_REQ_ACK_PENDING");
			}
		}

		if (QDF_STATUS_SUCCESS !=
		    sme_send_action(WLAN_HDD_GET_HAL_CTX(adapter),
				    sessionId, buf, len, extendedWait, noack,
				    current_freq)) {
			hdd_err("sme_send_action returned fail");
			goto err;
		}
	} else if (QDF_SAP_MODE == adapter->device_mode ||
		   QDF_P2P_GO_MODE == adapter->device_mode) {
		if (QDF_STATUS_SUCCESS !=
		    wlansap_send_action(WLAN_HDD_GET_SAP_CTX_PTR(adapter),
					buf, len, 0, current_freq)) {
			hdd_err("wlansap_send_action returned fail");
			goto err;
		}
	}

	return 0;
err:
	if (!noack) {
		hdd_send_action_cnf(adapter, false);
	}
	return 0;
err_rem_channel:
	*cookie = (uintptr_t) cfgState;
	cfg80211_mgmt_tx_status(
		adapter->dev->ieee80211_ptr,
		*cookie, buf, len, false, GFP_KERNEL);
	EXIT();
	return 0;
}
#endif

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

#ifdef CONVERGED_P2P_ENABLE
static int __wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
						   struct wireless_dev *wdev,
						   u64 cookie)
{
	QDF_STATUS status;
	struct net_device *dev = wdev->netdev;
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_session_id(adapter->sessionId)) {
		hdd_err("invalid session id: %d", adapter->sessionId);
		return -EINVAL;
	}

	status = wlan_cfg80211_mgmt_tx_cancel(adapter->hdd_vdev,
						cookie);
	hdd_info("cancel mgmt tx, status:%d", status);

	return 0;
}
#else
static int __wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
						   struct wireless_dev *wdev,
						   u64 cookie)
{
	return wlan_hdd_cfg80211_cancel_remain_on_channel(wiphy, wdev, cookie);
}
#endif

int wlan_hdd_cfg80211_mgmt_tx_cancel_wait(struct wiphy *wiphy,
					  struct wireless_dev *wdev, u64 cookie)
{
	int ret;

	cds_ssr_protect(__func__);
	ret = __wlan_hdd_cfg80211_mgmt_tx_cancel_wait(wiphy, wdev, cookie);
	cds_ssr_unprotect(__func__);

	return ret;
}

void hdd_send_action_cnf(struct hdd_adapter *adapter, bool actionSendSuccess)
{
	struct hdd_cfg80211_state *cfgState =
		WLAN_HDD_GET_CFG_STATE_PTR(adapter);

	cfgState->actionFrmState = HDD_IDLE;

	if (NULL == cfgState->buf)
		return;

	if (cfgState->is_go_neg_ack_received) {

		cfgState->is_go_neg_ack_received = 0;
		/* Sometimes its possible that host may receive the ack for GO
		 * negotiation req after sending go negotaition confirmation,
		 * in such case drop the ack received for the go negotiation
		 * request, so that supplicant waits for the confirmation ack
		 * from firmware.
		 */
		hdd_debug("Drop the pending ack received in cfgState->actionFrmState %d",
				cfgState->actionFrmState);
		return;
	}

	hdd_debug("Send Action cnf, actionSendSuccess %d",
		actionSendSuccess);
	/*
	 * buf is the same pointer it passed us to send. Since we are sending
	 * it through control path, we use different buffers.
	 * In case of mac80211, they just push it to the skb and pass the same
	 * data while sending tx ack status.
	 */
	cfg80211_mgmt_tx_status(
		adapter->dev->ieee80211_ptr,
		cfgState->action_cookie,
		cfgState->buf, cfgState->len,
		actionSendSuccess, GFP_KERNEL);

	qdf_mem_free(cfgState->buf);
	cfgState->buf = NULL;

	complete(&adapter->tx_action_cnf_event);
}

/**
 * hdd_send_action_cnf_cb - action confirmation callback
 * @session_id: SME session ID
 * @tx_completed: ack status
 *
 * This function invokes hdd_sendActionCnf to update ack status to
 * supplicant.
 */
void hdd_send_action_cnf_cb(uint32_t session_id, bool tx_completed)
{
	struct hdd_context *hdd_ctx;
	struct hdd_adapter *adapter;

	ENTER();

	/* Get the HDD context.*/
	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (0 != wlan_hdd_validate_context(hdd_ctx))
		return;

	adapter = hdd_get_adapter_by_sme_session_id(hdd_ctx, session_id);
	if (NULL == adapter) {
		hdd_err("adapter not found");
		return;
	}

	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hdd_err("adapter has invalid magic");
		return;
	}

	hdd_send_action_cnf(adapter, tx_completed);
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
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tP2pPsConfig NoA;
	int count, duration, interval;
	char *param;
	int ret;

	param = strnchr(command, strlen(command), ' ');
	if (param == NULL) {
		hdd_err("strnchr failed to find delimeter");
		return -EINVAL;
	}
	param++;
	ret = sscanf(param, "%d %d %d", &count, &interval, &duration);
	if (ret != 3) {
		hdd_err("P2P_SET GO NoA: fail to read params, ret=%d",
			ret);
		return -EINVAL;
	}
	if (count < 0 || interval < 0 || duration < 0 ||
	    interval > MAX_MUS_VAL || duration > MAX_MUS_VAL) {
		hdd_err("Invalid NOA parameters");
		return -EINVAL;
	}
	hdd_debug("P2P_SET GO NoA: count=%d interval=%d duration=%d",
		count, interval, duration);
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
	NoA.sessionid = adapter->sessionId;

	hdd_debug("P2P_PS_ATTR:oppPS %d ctWindow %d duration %d "
		  "interval %d count %d single noa duration %d "
		  "PsSelection %x", NoA.opp_ps,
		  NoA.ctWindow, NoA.duration, NoA.interval,
		  NoA.count, NoA.single_noa_duration, NoA.psSelection);

	return wlan_hdd_set_power_save(adapter, &NoA);
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
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
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

	hdd_debug("P2P_SET GO PS: legacy_ps=%d opp_ps=%d ctwindow=%d",
		  legacy_ps, opp_ps, ctwindow);

	/* PS Selection
	 * Opportunistic Power Save (1)
	 */

	/* From wpa_cli user need to use separate command to set ctWindow and
	 * Opps when user want to set ctWindow during that time other parameters
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

		wlan_hdd_set_power_save(adapter, &noa);
	}

	return 0;
}

int hdd_set_p2p_ps(struct net_device *dev, void *msgData)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	tP2pPsConfig NoA;
	struct p2p_app_set_ps *pappNoA = (struct p2p_app_set_ps *) msgData;

	NoA.opp_ps = pappNoA->opp_ps;
	NoA.ctWindow = pappNoA->ctWindow;
	NoA.duration = pappNoA->duration;
	NoA.interval = pappNoA->interval;
	NoA.count = pappNoA->count;
	NoA.single_noa_duration = pappNoA->single_noa_duration;
	NoA.psSelection = pappNoA->psSelection;
	NoA.sessionid = adapter->sessionId;

	return wlan_hdd_set_power_save(adapter, &NoA);
}

static uint8_t wlan_hdd_get_session_type(enum nl80211_iftype type)
{
	switch (type) {
	case NL80211_IFTYPE_AP:
		return QDF_SAP_MODE;
	case NL80211_IFTYPE_P2P_GO:
		return QDF_P2P_GO_MODE;
	case NL80211_IFTYPE_P2P_CLIENT:
		return QDF_P2P_CLIENT_MODE;
	case NL80211_IFTYPE_STATION:
		return QDF_STA_MODE;
	default:
		return QDF_STA_MODE;
	}
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
static
struct wireless_dev *__wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
						 const char *name,
						 unsigned char name_assign_type,
						 enum nl80211_iftype type,
						 u32 *flags,
						 struct vif_params *params)
{
	struct hdd_context *hdd_ctx = (struct hdd_context *) wiphy_priv(wiphy);
	struct hdd_adapter *adapter = NULL;
	struct hdd_scan_info *scan_info = NULL;
	int ret;
	uint8_t session_type;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return ERR_PTR(-EINVAL);
	}

	ret = wlan_hdd_validate_context(hdd_ctx);
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
	if (hdd_get_adapter(hdd_ctx, session_type) != NULL
	    && QDF_SAP_MODE != session_type
	    && QDF_P2P_GO_MODE != session_type
	    && QDF_P2P_CLIENT_MODE != session_type
	    && QDF_STA_MODE != session_type) {
		hdd_err("Interface type %d already exists. Two interfaces of same type are not supported currently.",
			type);
		return ERR_PTR(-EINVAL);
	}

	adapter = hdd_get_adapter(hdd_ctx, QDF_STA_MODE);
	if ((adapter != NULL) &&
		!(wlan_hdd_validate_session_id(adapter->sessionId))) {
		scan_info = &adapter->scan_info;
		if (scan_info->mScanPending) {
			wlan_abort_scan(hdd_ctx->hdd_pdev, INVAL_PDEV_ID,
				adapter->sessionId, INVALID_SCAN_ID, false);
			hdd_debug("Abort Scan while adding virtual interface");
		}
	}

	adapter = NULL;
	if (hdd_ctx->config->isP2pDeviceAddrAdministrated &&
	    ((NL80211_IFTYPE_P2P_GO == type) ||
	     (NL80211_IFTYPE_P2P_CLIENT == type))) {
		/*
		 * Generate the P2P Interface Address. this address must be
		 * different from the P2P Device Address.
		 */
		struct qdf_mac_addr p2pDeviceAddress =
						hdd_ctx->p2pDeviceAddress;
		p2pDeviceAddress.bytes[4] ^= 0x80;
		adapter = hdd_open_adapter(hdd_ctx,
					    session_type,
					    name, p2pDeviceAddress.bytes,
					    name_assign_type,
					    true);
	} else {
		adapter = hdd_open_adapter(hdd_ctx,
					    session_type,
					    name,
					    wlan_hdd_get_intf_addr(hdd_ctx),
					    name_assign_type,
					    true);
	}

	if (NULL == adapter) {
		hdd_err("hdd_open_adapter failed");
		return ERR_PTR(-ENOSPC);
	}

	/*
	 * Add interface can be requested from the upper layer at any time
	 * check the statemachine for modules state and if they are closed
	 * open the modules.
	 */
	ret = hdd_wlan_start_modules(hdd_ctx, adapter, false);
	if (ret) {
		hdd_err("Failed to start the wlan_modules");
		goto close_adapter;
	}

	/*
	 * Once the support for session creation/deletion from
	 * hdd_hostapd_open/hdd_host_stop is in place.
	 * The support for starting adapter from here can be removed.
	 */
	if (NL80211_IFTYPE_AP == type || (NL80211_IFTYPE_P2P_GO == type)) {
		ret = hdd_start_adapter(adapter);
		if (ret) {
			hdd_err("Failed to start %s", name);
			goto stop_modules;
		}
	}

	if (hdd_ctx->rps)
		hdd_send_rps_ind(adapter);

	EXIT();
	return adapter->dev->ieee80211_ptr;

stop_modules:
	/*
	 * Find if any iface is up. If there is not iface which is up
	 * start the timer to close the modules
	 */
	if (hdd_check_for_opened_interfaces(hdd_ctx)) {
		hdd_debug("Closing all modules from the add_virt_iface");
		qdf_mc_timer_start(&hdd_ctx->iface_change_timer,
				   hdd_ctx->config->iface_change_wait_time);
	} else
		hdd_debug("Other interfaces are still up dont close modules!");

close_adapter:
	hdd_close_adapter(hdd_ctx, adapter, true);

	return ERR_PTR(-EINVAL);
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(4, 12, 0)
struct wireless_dev *wlan_hdd_add_virtual_intf(struct wiphy *wiphy,
					       const char *name,
					       unsigned char name_assign_type,
					       enum nl80211_iftype type,
					       struct vif_params *params)
{
	struct wireless_dev *wdev;

	cds_ssr_protect(__func__);
	wdev = __wlan_hdd_add_virtual_intf(wiphy, name, name_assign_type,
					   type, &params->flags, params);
	cds_ssr_unprotect(__func__);

	return wdev;
}
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(4, 1, 0)) || defined(WITH_BACKPORTS)
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
	struct hdd_context *hdd_ctx = (struct hdd_context *) wiphy_priv(wiphy);
	struct hdd_adapter *pVirtAdapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int status;

	ENTER();

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	MTRACE(qdf_trace(QDF_MODULE_ID_HDD,
			 TRACE_CODE_HDD_DEL_VIRTUAL_INTF,
			 pVirtAdapter->sessionId, pVirtAdapter->device_mode));
	hdd_debug("Device_mode %s(%d)",
		   hdd_device_mode_to_string(pVirtAdapter->device_mode),
		   pVirtAdapter->device_mode);

	status = wlan_hdd_validate_context(hdd_ctx);

	if (0 != status)
		return status;

	/* check state machine state and kickstart modules if they are closed */
	status = hdd_wlan_start_modules(hdd_ctx, pVirtAdapter, false);
	if (status)
		return status;

	if (pVirtAdapter->device_mode == QDF_SAP_MODE &&
	    wlan_sap_is_pre_cac_active(hdd_ctx->hHal)) {
		hdd_clean_up_pre_cac_interface(hdd_ctx);
	} else {
		wlan_hdd_release_intf_addr(hdd_ctx,
					 pVirtAdapter->macAddressCurrent.bytes);
		hdd_stop_adapter(hdd_ctx, pVirtAdapter, true);
		hdd_close_adapter(hdd_ctx, pVirtAdapter, true);
	}

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
static void wlan_hdd_p2p_action_debug(enum action_frm_type actionFrmType,
					uint8_t *macFrom)
{
	if (actionFrmType >= MAX_P2P_ACTION_FRAME_TYPE) {
		hdd_debug("[P2P] unknown[%d] <--- OTA from " MAC_ADDRESS_STR,
			actionFrmType, MAC_ADDR_ARRAY(macFrom));
	} else {
		hdd_debug("[P2P] %s <--- OTA from " MAC_ADDRESS_STR,
			p2p_action_frame_type[actionFrmType],
			MAC_ADDR_ARRAY(macFrom));
		if ((actionFrmType == WLAN_HDD_PROV_DIS_REQ)
		    && (global_p2p_connection_status == P2P_NOT_ACTIVE)) {
			global_p2p_connection_status = P2P_GO_NEG_PROCESS;
			hdd_debug("[P2P State]Inactive state to GO negotiation progress state");
		} else
		if ((actionFrmType == WLAN_HDD_GO_NEG_CNF)
		    && (global_p2p_connection_status == P2P_GO_NEG_PROCESS)) {
			global_p2p_connection_status = P2P_GO_NEG_COMPLETED;
			hdd_debug("[P2P State]GO negotiation progress to GO negotiation completed state");
		} else
		if ((actionFrmType == WLAN_HDD_INVITATION_REQ)
		    && (global_p2p_connection_status == P2P_NOT_ACTIVE)) {
			global_p2p_connection_status = P2P_GO_NEG_COMPLETED;
			hdd_debug("[P2P State]Inactive state to GO negotiation completed state Autonomous GO formation");
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
static void wlan_hdd_p2p_action_debug(enum action_frm_type actionFrmType,
					uint8_t *macFrom)
{

}
#endif

void __hdd_indicate_mgmt_frame(struct hdd_adapter *adapter,
			     uint32_t nFrameLength,
			     uint8_t *pbFrames,
			     uint8_t frameType, uint32_t rxChan, int8_t rxRssi)
{
	uint16_t freq;
	uint16_t extend_time;
	uint8_t type = 0;
	uint8_t subType = 0;
	enum action_frm_type actionFrmType;
	struct hdd_cfg80211_state *cfgState = NULL;
	QDF_STATUS status;
	struct hdd_remain_on_chan_ctx *pRemainChanCtx = NULL;
	struct hdd_context *hdd_ctx;
	uint8_t broadcast = 0;

	hdd_debug("Frame Type = %d Frame Length = %d",
		frameType, nFrameLength);

	if (NULL == adapter) {
		hdd_err("adapter is NULL");
		return;
	}
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (0 == nFrameLength) {
		hdd_err("Frame Length is Invalid ZERO");
		return;
	}

	if (NULL == pbFrames) {
		hdd_err("pbFrames is NULL");
		return;
	}

	type = WLAN_HDD_GET_TYPE_FRM_FC(pbFrames[0]);
	subType = WLAN_HDD_GET_SUBTYPE_FRM_FC(pbFrames[0]);

	/* Get adapter from Destination mac address of the frame */
	if ((type == SIR_MAC_MGMT_FRAME) &&
	    (subType != SIR_MAC_MGMT_PROBE_REQ) &&
	    !qdf_is_macaddr_broadcast(
	     (struct qdf_mac_addr *)&pbFrames[WLAN_HDD_80211_FRM_DA_OFFSET])) {
		adapter =
			hdd_get_adapter_by_macaddr(hdd_ctx,
						   &pbFrames
						   [WLAN_HDD_80211_FRM_DA_OFFSET]);
		if (NULL == adapter) {
			/*
			 * Under assumtion that we don't receive any action
			 * frame with BCST as destination,
			 * we are dropping action frame
			 */
			hdd_err("adapter for action frame is NULL Macaddr = "
				  MAC_ADDRESS_STR,
				  MAC_ADDR_ARRAY(&pbFrames
						 [WLAN_HDD_80211_FRM_DA_OFFSET]));
			hdd_debug("Frame Type = %d Frame Length = %d subType = %d",
				frameType, nFrameLength, subType);
			/*
			 * We will receive broadcast management frames
			 * in OCB mode
			 */
			adapter = hdd_get_adapter(hdd_ctx, QDF_OCB_MODE);
			if (NULL == adapter || !qdf_is_macaddr_broadcast(
				(struct qdf_mac_addr *)&pbFrames
				[WLAN_HDD_80211_FRM_DA_OFFSET])) {
				/*
				 * Under assumtion that we don't
				 * receive any action frame with BCST
				 * as destination, we are dropping
				 * action frame
				 */
			return;
			}

		 broadcast = 1;

		}
	}

	if (NULL == adapter->dev) {
		hdd_err("adapter->dev is NULL");
		return;
	}

	if (WLAN_HDD_ADAPTER_MAGIC != adapter->magic) {
		hdd_err("adapter has invalid magic");
		return;
	}

	/* Channel indicated may be wrong. TODO */
	/* Indicate an action frame. */
	if (rxChan <= MAX_NO_OF_2_4_CHANNELS)
		freq = ieee80211_channel_to_frequency(rxChan,
						      NL80211_BAND_2GHZ);
	else
		freq = ieee80211_channel_to_frequency(rxChan,
						      NL80211_BAND_5GHZ);

	cfgState = WLAN_HDD_GET_CFG_STATE_PTR(adapter);

	if ((type == SIR_MAC_MGMT_FRAME) &&
		(subType == SIR_MAC_MGMT_ACTION) && !broadcast) {
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
				hdd_debug("Rx Action Frame %u",
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
						hdd_debug("Extend RoC timer on reception of Action Frame");

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
							    (&adapter->
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
									hdd_err("Remain on Channel timer start failed");
								}
							} else {
								hdd_debug("Rcvd action frame after timer expired");
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
								hdd_debug("Action Pkt Cached successfully !!!");
							} else {
								hdd_err("Frames are pending. dropping frame !!!");
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
					hdd_debug("ACK_PENDING and But received RESP for Action frame ");
					cfgState->is_go_neg_ack_received = 1;
					hdd_send_action_cnf(adapter, true);
				}
			}
		}

		if (pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET] ==
		    WLAN_HDD_TDLS_ACTION_FRAME) {
			actionFrmType =
				pbFrames[WLAN_HDD_PUBLIC_ACTION_FRAME_OFFSET + 1];
			if (actionFrmType >= MAX_TDLS_ACTION_FRAME_TYPE) {
				hdd_debug("[TDLS] unknown[%d] <--- OTA",
					   actionFrmType);
			} else {
				hdd_debug("[TDLS] %s <--- OTA",
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
			sme_update_dsc_pto_up_mapping(hdd_ctx->hHal,
						      adapter->hddWmmDscpToUpMap,
						      adapter->sessionId);
		}
	}
	/* Indicate Frame Over Normal Interface */
	hdd_debug("Indicate Frame over NL80211 sessionid : %d, idx :%d",
		   adapter->sessionId, adapter->dev->ifindex);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	cfg80211_rx_mgmt(adapter->dev->ieee80211_ptr,
		 freq, rxRssi * 100, pbFrames,
			 nFrameLength, NL80211_RXMGMT_FLAG_ANSWERED);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
	cfg80211_rx_mgmt(adapter->dev->ieee80211_ptr,
			freq, rxRssi * 100, pbFrames,
			 nFrameLength, NL80211_RXMGMT_FLAG_ANSWERED,
			 GFP_ATOMIC);
#else
	cfg80211_rx_mgmt(adapter->dev->ieee80211_ptr, freq,
			rxRssi * 100,
			pbFrames, nFrameLength, GFP_ATOMIC);
#endif /* LINUX_VERSION_CODE */
}

#ifdef CONVERGED_P2P_ENABLE
int wlan_hdd_set_power_save(struct hdd_adapter *adapter,
	tpP2pPsConfig pnoa)
{
	struct wlan_objmgr_psoc *psoc;
	struct hdd_context *hdd_ctx;
	struct p2p_ps_config ps_config;
	QDF_STATUS status;

	if (!adapter || !pnoa) {
		hdd_err("null param, adapter:%pK, pnoa:%pK",
			adapter, pnoa);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	psoc = hdd_ctx->hdd_psoc;
	if (!psoc) {
		hdd_err("psoc is null");
		return -EINVAL;
	}

	ps_config.opp_ps = pnoa->opp_ps;
	ps_config.ct_window = pnoa->ctWindow;
	ps_config.duration = pnoa->duration;
	ps_config.interval = pnoa->interval;
	ps_config.count = pnoa->count;
	ps_config.single_noa_duration = pnoa->single_noa_duration;
	ps_config.ps_selection = pnoa->psSelection;
	ps_config.vdev_id = pnoa->sessionid;

	hdd_info("opp ps:%d, ct window:%d, duration:%d, interval:%d, count:%d, single noa duration:%d, ps selection:%d, vdev id:%d",
		ps_config.opp_ps, ps_config.ct_window,
		ps_config.duration, ps_config.interval,
		ps_config.count, ps_config.single_noa_duration,
		ps_config.ps_selection, ps_config.vdev_id);

	status = ucfg_p2p_set_ps(psoc, &ps_config);
	hdd_info("p2p set power save, status:%d", status);

	return qdf_status_to_os_return(status);
}
#else
int wlan_hdd_set_power_save(struct hdd_adapter *adapter,
	tpP2pPsConfig pnoa)
{
	tHalHandle handle;
	QDF_STATUS status;

	if (!adapter || !pnoa) {
		hdd_err("null param, adapter:%pK, pnoa:%pK",
			adapter, pnoa);
		return -EINVAL;
	}

	handle = WLAN_HDD_GET_HAL_CTX(adapter);
	status = sme_p2p_set_ps(handle, pnoa);
	hdd_info("p2p set power save, status:%d", status);

	return qdf_status_to_os_return(status);
}
#endif

#ifdef CONVERGED_P2P_ENABLE
int wlan_hdd_listen_offload_start(struct hdd_adapter *adapter,
	struct sir_p2p_lo_start *params)
{
	struct wlan_objmgr_psoc *psoc;
	struct p2p_lo_start lo_start;
	struct hdd_context *hdd_ctx;
	QDF_STATUS status;

	if (!adapter || !params) {
		hdd_err("null param, adapter:%pK, params:%pK",
			adapter, params);
		return -EINVAL;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	psoc = hdd_ctx->hdd_psoc;
	if (!psoc) {
		hdd_err("psoc is null");
		return -EINVAL;
	}

	lo_start.vdev_id = params->vdev_id;
	lo_start.ctl_flags = params->ctl_flags;
	lo_start.freq = params->freq;
	lo_start.period = params->period;
	lo_start.interval = params->interval;
	lo_start.count = params->count;
	lo_start.device_types = params->device_types;
	lo_start.dev_types_len = params->dev_types_len;
	lo_start.probe_resp_tmplt = params->probe_resp_tmplt;
	lo_start.probe_resp_len = params->probe_resp_len;

	status = ucfg_p2p_lo_start(psoc, &lo_start);
	hdd_info("p2p listen offload start, status:%d", status);

	return qdf_status_to_os_return(status);
}
#else
int wlan_hdd_listen_offload_start(struct hdd_adapter *adapter,
	struct sir_p2p_lo_start *params)
{
	QDF_STATUS status;

	if (!params) {
		hdd_err("params is null, params:%pK", params);
		return -EINVAL;
	}

	status = wma_p2p_lo_start(params);
	hdd_info("p2p listen offload start, status:%d", status);

	return qdf_status_to_os_return(status);
}
#endif

#ifdef CONVERGED_P2P_ENABLE
int wlan_hdd_listen_offload_stop(struct hdd_adapter *adapter)
{
	struct wlan_objmgr_psoc *psoc;
	struct hdd_context *hdd_ctx;
	uint32_t vdev_id;
	QDF_STATUS status;

	if (!adapter) {
		hdd_err("adapter is null, adapter:%pK", adapter);
		return -EINVAL;
	}

	vdev_id = (uint32_t)adapter->sessionId;
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	psoc = hdd_ctx->hdd_psoc;
	if (!psoc) {
		hdd_err("psoc is null");
		return -EINVAL;
	}

	status = ucfg_p2p_lo_stop(psoc, vdev_id);
	hdd_info("p2p listen offload stop, status:%d", status);

	return qdf_status_to_os_return(status);
}
#else
int wlan_hdd_listen_offload_stop(struct hdd_adapter *adapter)
{
	QDF_STATUS status;

	if (!adapter) {
		hdd_err("adapter is null, adapter:%pK", adapter);
		return -EINVAL;
	}

	status = wma_p2p_lo_stop(adapter->sessionId);
	hdd_info("p2p listen offload stop, status:%d", status);

	return qdf_status_to_os_return(status);
}
#endif

/**
 * wlan_hdd_update_mcc_adaptive_scheduler() - Function to update
 * MAS value to FW
 * @adapter:            adapter object data
 * @is_enable:          0-Disable, 1-Enable MAS
 *
 * This function passes down the value of MAS to UMAC
 *
 * Return: 0 for success else non zero
 *
 */
static int32_t wlan_hdd_update_mcc_adaptive_scheduler(
		struct hdd_adapter *adapter, bool is_enable)
{
	struct hdd_context *hdd_ctx = WLAN_HDD_GET_CTX(adapter);

	if (hdd_ctx == NULL) {
		hdd_err("HDD context is null");
		return -EINVAL;
	}

	hdd_info("enable/disable MAS :%d", is_enable);
	if (hdd_ctx->config &&
	    hdd_ctx->config->enableMCCAdaptiveScheduler) {
		/* Todo check where to set the MCC apative SCHED for read */

		if (QDF_STATUS_SUCCESS != sme_set_mas(is_enable)) {
			hdd_err("Failed to enable/disable MAS");
			return -EAGAIN;
		}
	}

	return 0;
}

/**
 * wlan_hdd_update_mcc_p2p_quota() - Function to Update P2P
 * quota to FW
 * @adapter:            Pointer to HDD adapter
 * @is_set:             0-reset, 1-set
 *
 * This function passes down the value of MAS to UMAC
 *
 * Return: none
 *
 */
static void wlan_hdd_update_mcc_p2p_quota(struct hdd_adapter *adapter,
					  bool is_set)
{

	hdd_info("Set/reset P2P quota: %d", is_set);
	if (is_set) {
		if (adapter->device_mode == QDF_STA_MODE)
			wlan_hdd_set_mcc_p2p_quota(adapter,
				100 - HDD_DEFAULT_MCC_P2P_QUOTA
			);
		else if (adapter->device_mode == QDF_P2P_GO_MODE)
			wlan_hdd_go_set_mcc_p2p_quota(adapter,
				HDD_DEFAULT_MCC_P2P_QUOTA);
		else
			wlan_hdd_set_mcc_p2p_quota(adapter,
				HDD_DEFAULT_MCC_P2P_QUOTA);
	} else {
		if (adapter->device_mode == QDF_P2P_GO_MODE)
			wlan_hdd_go_set_mcc_p2p_quota(adapter,
				HDD_RESET_MCC_P2P_QUOTA);
		else
			wlan_hdd_set_mcc_p2p_quota(adapter,
				HDD_RESET_MCC_P2P_QUOTA);
	}
}

int32_t wlan_hdd_set_mas(struct hdd_adapter *adapter, uint8_t mas_value)
{
	int32_t ret = 0;

	if (!adapter) {
		hdd_err("Adapter is NULL");
		return -EINVAL;
	}

	if (mas_value) {
		hdd_info("Miracast is ON. Disable MAS and configure P2P quota");
		ret = wlan_hdd_update_mcc_adaptive_scheduler(
			adapter, false);
		if (0 != ret) {
			hdd_err("Failed to disable MAS");
			goto done;
		}

		/* Config p2p quota */
		wlan_hdd_update_mcc_p2p_quota(adapter, true);
	} else {
		hdd_info("Miracast is OFF. Enable MAS and reset P2P quota");
		wlan_hdd_update_mcc_p2p_quota(adapter, false);

		ret = wlan_hdd_update_mcc_adaptive_scheduler(
			adapter, true);
		if (0 != ret) {
			hdd_err("Failed to enable MAS");
			goto done;
		}
	}

done:
	return ret;
}

/**
 * set_first_connection_operating_channel() - Function to set
 * first connection oerating channel
 * @adapter:   adapter data
 * @set_value: Quota value for the interface
 * @dev_mode:  Device mode
 * This function is used to set the first adapter operating
 * channel
 *
 * Return: operating channel updated in set value
 *
 */
static uint32_t set_first_connection_operating_channel(
		struct hdd_context *hdd_ctx, uint32_t set_value,
		enum tQDF_ADAPTER_MODE dev_mode)
{
	uint8_t operating_channel;

	operating_channel = hdd_get_operating_channel(
					hdd_ctx, dev_mode);
	if (!operating_channel) {
		hdd_err(" First adpter operating channel is invalid");
		return -EINVAL;
	}

	hdd_info("First connection channel No.:%d and quota:%dms",
			operating_channel, set_value);
	/* Move the time quota for first channel to bits 15-8 */
	set_value = set_value << 8;

	/*
	 * Store the channel number of 1st channel at bits 7-0
	 * of the bit vector
	 */
	return set_value | operating_channel;
}

/**
 * set_second_connection_operating_channel() - Function to set
 * second connection oerating channel
 * @adapter:   adapter data
 * @set_value: Quota value for the interface
 * @vdev_id:  vdev id
 *
 * This function is used to set the first adapter operating
 * channel
 *
 * Return: operating channel updated in set value
 *
 */
static uint32_t set_second_connection_operating_channel(
		struct hdd_context *hdd_ctx, uint32_t set_value,
		uint8_t vdev_id)
{
	uint8_t operating_channel;

	operating_channel = policy_mgr_get_mcc_operating_channel(
		hdd_ctx->hdd_psoc, vdev_id);

	if (operating_channel == 0) {
		hdd_err("Second adapter operating channel is invalid");
		return -EINVAL;
	}

	hdd_info("Second connection channel No.:%d and quota:%dms",
			operating_channel, set_value);
	/*
	 * Now move the time quota and channel number of the
	 * 1st adapter to bits 23-16 and bits 15-8 of the bit
	 * vector, respectively.
	 */
	set_value = set_value << 8;

	/*
	 * Set the channel number for 2nd MCC vdev at bits
	 * 7-0 of set_value
	 */
	return set_value | operating_channel;
}

/**
 * wlan_hdd_set_mcc_p2p_quota() - Function to set quota for P2P
 * @psoc: PSOC object information
 * @set_value:          Qouta value for the interface
 * @operating_channel   First adapter operating channel
 * @vdev_id             vdev id
 *
 * This function is used to set the quota for P2P cases
 *
 * Return: Configuration message posting status, SUCCESS or Fail
 *
 */
int wlan_hdd_set_mcc_p2p_quota(struct hdd_adapter *adapter,
			       uint32_t set_value)
{
	int32_t ret = 0;
	uint32_t concurrent_state;
	struct hdd_context *hdd_ctx;

	if (!adapter) {
		hdd_err("Invalid adapter");
		return -EFAULT;
	}
	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (hdd_ctx == NULL) {
		hdd_err("HDD context is null");
		return -EINVAL;
	}

	concurrent_state = policy_mgr_get_concurrency_mode(
		hdd_ctx->hdd_psoc);
	/*
	 * Check if concurrency mode is active.
	 * Need to modify this code to support MCC modes other than STA/P2P
	 */
	if ((concurrent_state ==
	     (QDF_STA_MASK | QDF_P2P_CLIENT_MASK)) ||
	    (concurrent_state == (QDF_STA_MASK | QDF_P2P_GO_MASK))) {
		hdd_info("STA & P2P are both enabled");

		/*
		 * The channel numbers for both adapters and the time
		 * quota for the 1st adapter, i.e., one specified in cmd
		 * are formatted as a bit vector then passed on to WMA
		 * +***********************************************************+
		 * |bit 31-24  | bit 23-16  |   bits 15-8   |   bits 7-0       |
		 * |  Unused   | Quota for  | chan. # for   |   chan. # for    |
		 * |           | 1st chan.  | 1st chan.     |   2nd chan.      |
		 * +***********************************************************+
		 */

		set_value = set_first_connection_operating_channel(
			hdd_ctx, set_value, adapter->device_mode);


		set_value = set_second_connection_operating_channel(
			hdd_ctx, set_value, adapter->sessionId);


		ret = wlan_hdd_send_p2p_quota(adapter, set_value);
	} else {
		hdd_info("MCC is not active. Exit w/o setting latency");
	}

	return ret;
}

int wlan_hdd_go_set_mcc_p2p_quota(struct hdd_adapter *hostapd_adapter,
				  uint32_t set_value)
{
	return wlan_hdd_set_mcc_p2p_quota(hostapd_adapter, set_value);
}

void wlan_hdd_set_mcc_latency(struct hdd_adapter *adapter, int set_value)
{
	uint32_t concurrent_state;
	struct hdd_context *hdd_ctx;

	if (!adapter) {
		hdd_err("Invalid adapter");
		return;
	}

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (hdd_ctx == NULL) {
		hdd_err("HDD context is null");
		return;
	}

	concurrent_state = policy_mgr_get_concurrency_mode(
		hdd_ctx->hdd_psoc);
	/**
	 * Check if concurrency mode is active.
	 * Need to modify this code to support MCC modes other than STA/P2P
	 */
	if ((concurrent_state ==
	     (QDF_STA_MASK | QDF_P2P_CLIENT_MASK)) ||
	    (concurrent_state == (QDF_STA_MASK | QDF_P2P_GO_MASK))) {
		hdd_info("STA & P2P are both enabled");
		/*
		 * The channel number and latency are formatted in
		 * a bit vector then passed on to WMA layer.
		 * +**********************************************+
		 * |bits 31-16 |      bits 15-8    |  bits 7-0    |
		 * |  Unused   | latency - Chan. 1 |  channel no. |
		 * +**********************************************+
		 */
		set_value = set_first_connection_operating_channel(
			hdd_ctx, set_value, adapter->device_mode);

		wlan_hdd_send_mcc_latency(adapter, set_value);
	} else {
		hdd_info("MCC is not active. Exit w/o setting latency");
	}
}
