/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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

/** ------------------------------------------------------------------------- *
    ------------------------------------------------------------------------- *

    \file csr_neighbor_roam.c

	Implementation for the simple roaming algorithm for 802.11r Fast
	transitions and Legacy roaming for Android platform.
   ========================================================================== */

#include "wma_types.h"
#include "cds_mq.h"
#include "csr_inside_api.h"
#include "sms_debug.h"
#include "sme_qos_internal.h"
#include "sme_inside.h"
#include "host_diag_core_event.h"
#include "host_diag_core_log.h"
#include "csr_api.h"
#include "sme_api.h"
#include "csr_neighbor_roam.h"
#include "mac_trace.h"
#include "cds_concurrency.h"

#define NEIGHBOR_ROAM_DEBUG sms_log

static void csr_neighbor_roam_reset_channel_info(tpCsrNeighborRoamChannelInfo
						 rChInfo);
static void csr_neighbor_roam_reset_preauth_control_info(tpAniSirGlobal pMac,
							 uint8_t sessionId);

QDF_STATUS csr_roam_copy_connected_profile(tpAniSirGlobal pMac, uint32_t sessionId,
					   tCsrRoamProfile *pDstProfile);

static QDF_STATUS csr_neighbor_roam_issue_preauth_req(tpAniSirGlobal pMac,
						      uint8_t sessionId);

#define CSR_NEIGHBOR_ROAM_STATE_TRANSITION(mac_ctx, newstate, session) \
{ \
	mac_ctx->roam.neighborRoamInfo[session].prevNeighborRoamState = \
		mac_ctx->roam.neighborRoamInfo[session].neighborRoamState; \
	mac_ctx->roam.neighborRoamInfo[session].neighborRoamState = newstate; \
	CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_DEBUG, \
		FL("Sessionid (%d) NeighborRoam transition from %s to %s"), \
		session, csr_neighbor_roam_state_to_string( \
		mac_ctx->roam.neighborRoamInfo[session].prevNeighborRoamState),\
		csr_neighbor_roam_state_to_string(newstate)); \
}

uint8_t *csr_neighbor_roam_state_to_string(uint8_t state)
{
	switch (state) {
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_CLOSED);
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_INIT);
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_CONNECTED);
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING);
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING);
		CASE_RETURN_STRING(eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE);
	default:
		return "eCSR_NEIGHBOR_ROAM_STATE_UNKNOWN";
	}

}

#ifdef FEATURE_WLAN_LFR_METRICS
/**
 * csr_neighbor_roam_send_lfr_metric_event() - Send event of LFR metric
 * @mac_ctx: Handle returned by mac_open.
 * @session_id: Session information
 * @bssid: BSSID of attempted AP
 * @status: Result of LFR operation
 *
 * LFR metrics - pre-auth completion metric
 * Send the event to supplicant indicating pre-auth result
 *
 * Return: void
 */

static void csr_neighbor_roam_send_lfr_metric_event(
				tpAniSirGlobal mac_ctx,
				uint8_t session_id,
				tSirMacAddr bssid,
				eRoamCmdStatus status)
{
	tCsrRoamInfo *roam_info;

	roam_info = cdf_mem_malloc(sizeof(tCsrRoamInfo));
	if (NULL == roam_info) {
		sms_log(mac_ctx, LOG1, FL("Memory allocation failed!"));
	} else {
		cdf_mem_copy((void *)roam_info->bssid,
			     (void *)bssid, sizeof(*roam_info));
		csr_roam_call_callback(mac_ctx, session_id, roam_info, 0,
			status, 0);
		cdf_mem_free(roam_info);
	}
}
#else
/* Empty inline function will be a no-op */
static inline void csr_neighbor_roam_send_lfr_metric_event(
				tpAniSirGlobal mac_ctx,
				uint8_t session_id,
				tSirMacAddr bssid,
				eRoamCmdStatus status)
{
	;
}
#endif

/**
 * csr_neighbor_roam_free_neighbor_roam_bss_node()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @neighborRoamBSSNode: Neighbor Roam BSS Node to be freed
 *
 * This function frees all the internal pointers CSR NeighborRoam BSS Info
 * and also frees the node itself
 *
 * Return: None
 */
void csr_neighbor_roam_free_neighbor_roam_bss_node(tpAniSirGlobal pMac,
						   tpCsrNeighborRoamBSSInfo
						   neighborRoamBSSNode)
{
	if (neighborRoamBSSNode) {
		if (neighborRoamBSSNode->pBssDescription) {
			cdf_mem_free(neighborRoamBSSNode->pBssDescription);
			neighborRoamBSSNode->pBssDescription = NULL;
		}
		cdf_mem_free(neighborRoamBSSNode);
		neighborRoamBSSNode = NULL;
	}

	return;
}

/**
 * csr_neighbor_roam_remove_roamable_ap_list_entry()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @pList: The list from which the entry should be removed
 * @pNeighborEntry: Neighbor Roam BSS Node to be removed
 *
 * This function removes a given entry from the given list
 *
 * Return: true if successfully removed, else false
 */
bool csr_neighbor_roam_remove_roamable_ap_list_entry(tpAniSirGlobal pMac,
						     tDblLinkList *pList,
						     tpCsrNeighborRoamBSSInfo
						     pNeighborEntry)
{
	if (pList) {
		return csr_ll_remove_entry(pList, &pNeighborEntry->List,
					   LL_ACCESS_LOCK);
	}

	sms_log(pMac, LOGE,
		FL
			("Removing neighbor BSS node from list failed. Current count = %d"),
		csr_ll_count(pList));

	return false;
}

/**
 * csr_neighbor_roam_next_roamable_ap() - Get next AP from roamable AP list
 * @mac_ctx - The handle returned by mac_open.
 * @plist - The list from which the entry should be returned
 * @neighbor_entry - Neighbor Roam BSS Node whose next entry should be returned
 *
 * Gets the entry next to passed entry. If NULL is passed, return the entry
 * in the head of the list
 *
 * Return: Neighbor Roam BSS Node to be returned
 */
tpCsrNeighborRoamBSSInfo csr_neighbor_roam_next_roamable_ap(
				tpAniSirGlobal mac_ctx, tDblLinkList *llist,
				tpCsrNeighborRoamBSSInfo neighbor_entry)
{
	tListElem *entry = NULL;
	tpCsrNeighborRoamBSSInfo result = NULL;

	if (llist) {
		if (NULL == neighbor_entry)
			entry = csr_ll_peek_head(llist, LL_ACCESS_LOCK);
		else
			entry = csr_ll_next(llist, &neighbor_entry->List,
					LL_ACCESS_LOCK);
		if (entry)
			result = GET_BASE_ADDR(entry, tCsrNeighborRoamBSSInfo,
					List);
	}

	return result;
}

/**
 * csr_neighbor_roam_free_roamable_bss_list() - Frees roamable APs list
 * @mac_ctx: The handle returned by mac_open.
 * @llist: Neighbor Roam BSS List to be emptied
 *
 * Empties and frees all the nodes in the roamable AP list
 *
 * Return: none
 */
void csr_neighbor_roam_free_roamable_bss_list(tpAniSirGlobal mac_ctx,
					      tDblLinkList *llist)
{
	tpCsrNeighborRoamBSSInfo result = NULL;

	NEIGHBOR_ROAM_DEBUG(mac_ctx, LOG2,
			    FL("Emptying the BSS list. Current count = %d"),
			    csr_ll_count(llist));

	/*
	 * Pick up the head, remove and free the node till
	 * the list becomes empty
	 */
	while ((result = csr_neighbor_roam_next_roamable_ap(mac_ctx, llist,
							NULL)) != NULL) {
		csr_neighbor_roam_remove_roamable_ap_list_entry(mac_ctx,
			llist, result);
		csr_neighbor_roam_free_neighbor_roam_bss_node(mac_ctx, result);
	}
	return;
}

static void csr_neighbor_roam_trigger_handoff(tpAniSirGlobal pMac,
					      uint8_t sessionId)
{
	if (csr_roam_is_fast_roam_enabled(pMac, sessionId)) {
		csr_neighbor_roam_issue_preauth_req(pMac, sessionId);
	} else 	{
		sms_log(pMac, LOGE, FL("Roaming is diisabled"));
	}
}

QDF_STATUS
csr_neighbor_roam_update_fast_roaming_enabled(tpAniSirGlobal pMac,
					      uint8_t sessionId,
					      const bool fastRoamEnabled)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	/* set fast roam enable/disable flag */
	pMac->roam.configParam.isFastRoamIniFeatureEnabled = fastRoamEnabled;

	switch (pNeighborRoamInfo->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
		if (true == fastRoamEnabled) {
			csr_roam_offload_scan(pMac, sessionId,
					      ROAM_SCAN_OFFLOAD_START,
					      REASON_CONNECT);
		} else {
			csr_roam_offload_scan(pMac, sessionId,
					      ROAM_SCAN_OFFLOAD_STOP,
					      REASON_DISCONNECTED);
		}
	break;
	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				FL
				("Currently in INIT state, Nothing to do"));
		break;
	default:
		NEIGHBOR_ROAM_DEBUG(pMac, LOGE,
				FL
				("Unexpected state %s, returning failure"),
				mac_trace_get_neighbour_roam_state
				(pNeighborRoamInfo->neighborRoamState));
		qdf_status = QDF_STATUS_E_FAILURE;
		break;
	}
	return qdf_status;
}

#ifdef FEATURE_WLAN_ESE
QDF_STATUS csr_neighbor_roam_update_ese_mode_enabled(tpAniSirGlobal pMac,
						     uint8_t sessionId,
						     const bool eseMode)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
	    &pMac->roam.neighborRoamInfo[sessionId];

	switch (pNeighborRoamInfo->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
		if (true == eseMode) {
			csr_roam_offload_scan(pMac, sessionId,
					   ROAM_SCAN_OFFLOAD_START,
					   REASON_CONNECT);
		} else if (false == eseMode) {
			csr_roam_offload_scan(pMac, sessionId,
					   ROAM_SCAN_OFFLOAD_STOP,
					   REASON_DISCONNECTED);
		}
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		NEIGHBOR_ROAM_DEBUG(pMac,
				    LOG2,
				    FL
				    ("Currently in INIT state, Nothing to do"));
		break;

	default:
		NEIGHBOR_ROAM_DEBUG(pMac, LOGE,
				    FL
				    ("Unexpected state %d, returning failure"),
				    pNeighborRoamInfo->neighborRoamState);
		break;
	}
	return qdf_status;
}
#endif

QDF_STATUS csr_neighbor_roam_set_lookup_rssi_threshold(tpAniSirGlobal pMac,
						uint8_t sessionId,
						uint8_t
						  neighborLookupRssiThreshold)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
	    &pMac->roam.neighborRoamInfo[sessionId];
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	switch (pNeighborRoamInfo->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				FL("Currently in CONNECTED state, "
				"sending ROAM_SCAN_OFFLOAD_UPDATE_CFG."));
		pNeighborRoamInfo->cfgParams.neighborLookupThreshold =
		    neighborLookupRssiThreshold;
		pNeighborRoamInfo->currentNeighborLookupThreshold =
		    pNeighborRoamInfo->cfgParams.neighborLookupThreshold;
		csr_roam_offload_scan(pMac, sessionId,
				   ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				   REASON_LOOKUP_THRESH_CHANGED);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				    FL("Currently in INIT state, "
				       "just set lookupRssi threshold."));
		pNeighborRoamInfo->cfgParams.neighborLookupThreshold =
		    neighborLookupRssiThreshold;
		pNeighborRoamInfo->currentNeighborLookupThreshold =
		    pNeighborRoamInfo->cfgParams.neighborLookupThreshold;
		break;

	default:
		NEIGHBOR_ROAM_DEBUG(pMac, LOGE,
				    FL
				    ("Unexpected state %s, returning failure"),
				    mac_trace_get_neighbour_roam_state
				    (pNeighborRoamInfo->neighborRoamState));
		qdf_status = QDF_STATUS_E_FAILURE;
		break;
	}
	return qdf_status;
}

QDF_STATUS
csr_neighbor_roam_set_opportunistic_scan_threshold_diff(tpAniSirGlobal pMac,
						uint8_t sessionId,
						uint8_t
						    nOpportunisticThresholdDiff)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
	    &pMac->roam.neighborRoamInfo[sessionId];

	switch (pNeighborRoamInfo->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				    FL("Currently in CONNECTED state, "
				       "sending ROAM_SCAN_OFFLOAD_UPDATE_CFG"));
		pNeighborRoamInfo->cfgParams.nOpportunisticThresholdDiff =
		    nOpportunisticThresholdDiff;
		pNeighborRoamInfo->currentOpportunisticThresholdDiff =
		    nOpportunisticThresholdDiff;

		csr_roam_offload_scan(pMac,
				   sessionId,
				   ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				   REASON_OPPORTUNISTIC_THRESH_DIFF_CHANGED);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				FL("Currently in INIT state, "
				   "just set opportunistic threshold diff"));
		pNeighborRoamInfo->cfgParams.nOpportunisticThresholdDiff =
		    nOpportunisticThresholdDiff;
		pNeighborRoamInfo->currentOpportunisticThresholdDiff =
		    nOpportunisticThresholdDiff;
		break;

	default:
		NEIGHBOR_ROAM_DEBUG(pMac, LOGE,
				    FL("Unexpected state %d returning failure"),
				    pNeighborRoamInfo->neighborRoamState);
		qdf_status = QDF_STATUS_E_FAILURE;
		break;
	}
	return qdf_status;
}

QDF_STATUS
csr_neighbor_roam_set_roam_rescan_rssi_diff(tpAniSirGlobal pMac,
					    uint8_t sessionId,
					    uint8_t nRoamRescanRssiDiff)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
	    &pMac->roam.neighborRoamInfo[sessionId];

	switch (pNeighborRoamInfo->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				FL("Currently in CONNECTED state, "
				   "sending ROAM_SCAN_OFFLOAD_UPDATE_CFG."));
		pNeighborRoamInfo->cfgParams.nRoamRescanRssiDiff =
		    nRoamRescanRssiDiff;
		pNeighborRoamInfo->currentRoamRescanRssiDiff =
		    nRoamRescanRssiDiff;
		csr_roam_offload_scan(pMac, sessionId,
				   ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				   REASON_ROAM_RESCAN_RSSI_DIFF_CHANGED);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				    FL("Currently in INIT state, "
				       "just set rescan rssi diff"));
		pNeighborRoamInfo->cfgParams.nRoamRescanRssiDiff =
		    nRoamRescanRssiDiff;
		pNeighborRoamInfo->currentRoamRescanRssiDiff =
		    nRoamRescanRssiDiff;
		break;

	default:
		NEIGHBOR_ROAM_DEBUG(pMac, LOGE,
				    FL("Unexpected state %d returning failure"),
				    pNeighborRoamInfo->neighborRoamState);
		qdf_status = QDF_STATUS_E_FAILURE;
		break;
	}
	return qdf_status;
}

QDF_STATUS
csr_neighbor_roam_set_roam_bmiss_first_bcnt(tpAniSirGlobal pMac,
					    uint8_t sessionId,
					    uint8_t nRoamBmissFirstBcnt)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
	    &pMac->roam.neighborRoamInfo[sessionId];

	switch (pNeighborRoamInfo->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				FL("Currently in CONNECTED state, "
				   "sending ROAM_SCAN_OFFLOAD_UPDATE_CFG."));
		pNeighborRoamInfo->cfgParams.nRoamBmissFirstBcnt =
		    nRoamBmissFirstBcnt;
		pNeighborRoamInfo->currentRoamBmissFirstBcnt =
		    nRoamBmissFirstBcnt;

		csr_roam_offload_scan(pMac,
				   sessionId,
				   ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				   REASON_ROAM_BMISS_FIRST_BCNT_CHANGED);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		NEIGHBOR_ROAM_DEBUG(pMac,
		  LOG2, FL
		  ("Currently in INIT state, safe to set roam rescan rssi diff"));
		pNeighborRoamInfo->cfgParams.nRoamBmissFirstBcnt =
		    nRoamBmissFirstBcnt;
		pNeighborRoamInfo->currentRoamBmissFirstBcnt =
		    nRoamBmissFirstBcnt;
		break;

	default:
		NEIGHBOR_ROAM_DEBUG(pMac,
				    LOGE,
				    FL("Unexpected state %d returning failure"),
				    pNeighborRoamInfo->neighborRoamState);
		qdf_status = QDF_STATUS_E_FAILURE;
		break;
	}
	return qdf_status;
}

QDF_STATUS
csr_neighbor_roam_set_roam_bmiss_final_bcnt(tpAniSirGlobal pMac,
					    uint8_t sessionId,
					    uint8_t nRoamBmissFinalBcnt)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
	    &pMac->roam.neighborRoamInfo[sessionId];

	switch (pNeighborRoamInfo->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				    FL("Currently in CONNECTED state, "
				       "sending ROAM_SCAN_OFFLOAD_UPDATE_CFG."));
		pNeighborRoamInfo->cfgParams.nRoamBmissFinalBcnt =
		    nRoamBmissFinalBcnt;
		pNeighborRoamInfo->currentRoamBmissFinalBcnt =
		    nRoamBmissFinalBcnt;

		csr_roam_offload_scan(pMac, sessionId,
				   ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				   REASON_ROAM_BMISS_FINAL_BCNT_CHANGED);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				    FL("Currently in INIT state, "
				       "just set roam fianl bmiss count"));
		pNeighborRoamInfo->cfgParams.nRoamBmissFinalBcnt =
		    nRoamBmissFinalBcnt;
		pNeighborRoamInfo->currentRoamBmissFinalBcnt =
		    nRoamBmissFinalBcnt;
		break;

	default:
		NEIGHBOR_ROAM_DEBUG(pMac,
				    LOGE,
				    FL("Unexpected state %d returning failure"),
				    pNeighborRoamInfo->neighborRoamState);
		qdf_status = QDF_STATUS_E_FAILURE;
		break;
	}
	return qdf_status;
}

QDF_STATUS
csr_neighbor_roam_set_roam_beacon_rssi_weight(tpAniSirGlobal pMac,
					      uint8_t sessionId,
					      uint8_t nRoamBeaconRssiWeight)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
	    &pMac->roam.neighborRoamInfo[sessionId];

	switch (pNeighborRoamInfo->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				FL("Currently in CONNECTED state, "
				   "sending ROAM_SCAN_OFFLOAD_UPDATE_CFG."));
		pNeighborRoamInfo->cfgParams.nRoamBeaconRssiWeight =
		    nRoamBeaconRssiWeight;
		pNeighborRoamInfo->currentRoamBeaconRssiWeight =
		    nRoamBeaconRssiWeight;

		csr_roam_offload_scan(pMac, sessionId,
				   ROAM_SCAN_OFFLOAD_UPDATE_CFG,
				   REASON_ROAM_BEACON_RSSI_WEIGHT_CHANGED);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
				    FL("Currently in INIT state, "
				       "just set roam beacon rssi weight"));
		pNeighborRoamInfo->cfgParams.nRoamBeaconRssiWeight =
		    nRoamBeaconRssiWeight;
		pNeighborRoamInfo->currentRoamBeaconRssiWeight =
		    nRoamBeaconRssiWeight;
		break;

	default:
		NEIGHBOR_ROAM_DEBUG(pMac,
				    LOGE,
				    FL("Unexpected state %d returning failure"),
				    pNeighborRoamInfo->neighborRoamState);
		qdf_status = QDF_STATUS_E_FAILURE;
		break;
	}
	return qdf_status;
}

/*CleanUP Routines*/
static void csr_neighbor_roam_reset_channel_info(tpCsrNeighborRoamChannelInfo
						 rChInfo)
{
	if ((rChInfo->IAPPNeighborListReceived == false) &&
	    (rChInfo->currentChannelListInfo.numOfChannels)) {
		rChInfo->currentChanIndex =
			CSR_NEIGHBOR_ROAM_INVALID_CHANNEL_INDEX;
		rChInfo->currentChannelListInfo.numOfChannels = 0;

		if (rChInfo->currentChannelListInfo.ChannelList)
			cdf_mem_free(rChInfo->currentChannelListInfo.
				     ChannelList);

		rChInfo->currentChannelListInfo.ChannelList = NULL;
	} else {
		rChInfo->currentChanIndex = 0;
	}
}

static void csr_neighbor_roam_reset_preauth_control_info(tpAniSirGlobal pMac,
							 uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];

	pNeighborRoamInfo->is11rAssoc = false;
	/* Purge pre-auth fail list */
	csr_neighbor_roam_purge_preauth_failed_list(pMac);

	pNeighborRoamInfo->FTRoamInfo.preauthRspPending = false;
	pNeighborRoamInfo->FTRoamInfo.numPreAuthRetries = 0;
	/* Do not free up the preauth done list here */
	pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum = 0;
	pNeighborRoamInfo->FTRoamInfo.neighborRptPending = false;
	pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport = 0;
	cdf_mem_zero(pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo,
		     sizeof(tCsrNeighborReportBssInfo) *
		     MAX_BSS_IN_NEIGHBOR_RPT);
	pNeighborRoamInfo->uOsRequestedHandoff = 0;
	cdf_mem_zero(&pNeighborRoamInfo->handoffReqInfo,
		     sizeof(tCsrHandoffRequest));
}

/**
 * csr_neighbor_roam_reset_connected_state_control_info()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @sessionId : session id
 *
 * This function will reset the neighbor roam control info data structures.
 * This function should be invoked whenever we move to CONNECTED state from
 * any state other than INIT state
 *
 * Return: None
 */
void csr_neighbor_roam_reset_connected_state_control_info(tpAniSirGlobal pMac,
							  uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];

	csr_neighbor_roam_reset_channel_info(&pNeighborRoamInfo->roamChannelInfo);
	csr_neighbor_roam_free_roamable_bss_list(pMac,
						 &pNeighborRoamInfo->roamableAPList);

	/* Do not free up the preauth done list here */
	pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum = 0;
	pNeighborRoamInfo->FTRoamInfo.neighborRptPending = false;
	pNeighborRoamInfo->FTRoamInfo.numPreAuthRetries = 0;
	pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport = 0;
	pNeighborRoamInfo->FTRoamInfo.preauthRspPending = 0;
	cdf_mem_zero(pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo,
		     sizeof(tCsrNeighborReportBssInfo) *
		     MAX_BSS_IN_NEIGHBOR_RPT);
	pNeighborRoamInfo->uOsRequestedHandoff = 0;
	cdf_mem_zero(&pNeighborRoamInfo->handoffReqInfo,
		     sizeof(tCsrHandoffRequest));
}

void csr_neighbor_roam_reset_report_scan_state_control_info(tpAniSirGlobal pMac,
							    uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];

	cdf_zero_macaddr(&pNeighborRoamInfo->currAPbssid);
#ifdef FEATURE_WLAN_ESE
	pNeighborRoamInfo->isESEAssoc = false;
	pNeighborRoamInfo->isVOAdmitted = false;
	pNeighborRoamInfo->MinQBssLoadRequired = 0;
#endif

	/* Purge roamable AP list */
	csr_neighbor_roam_free_roamable_bss_list(pMac,
						 &pNeighborRoamInfo->roamableAPList);
	return;
}

/**
 * csr_neighbor_roam_reset_init_state_control_info()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @sessionId : session id
 *
 * This function will reset the neighbor roam control info data structures.
 * This function should be invoked whenever we move to CONNECTED state from
 * INIT state
 *
 * Return: None
 */
void csr_neighbor_roam_reset_init_state_control_info(tpAniSirGlobal pMac,
						     uint8_t sessionId)
{
	csr_neighbor_roam_reset_connected_state_control_info(pMac, sessionId);

	/* In addition to the above resets,
	   we should clear off the curAPBssId/Session ID in the timers */
	csr_neighbor_roam_reset_report_scan_state_control_info(pMac, sessionId);
}

/* ---------------------------------------------------------------------------

    \fn csr_neighbor_roam_purge_preauth_fail_list

    \brief  This function empties the preauth fail list

    \param  pMac - The handle returned by mac_open.

    \return VOID

   ---------------------------------------------------------------------------*/
void csr_neighbor_roam_purge_preauth_fail_list(tpAniSirGlobal pMac)
{
	uint8_t i;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	NEIGHBOR_ROAM_DEBUG(pMac, LOGE, FL("Purging the preauth fail list"));
	for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
		pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[i];
		while (pNeighborRoamInfo->FTRoamInfo.preAuthFailList.
		       numMACAddress) {
			cdf_mem_zero(pNeighborRoamInfo->FTRoamInfo.
				     preAuthFailList.
				     macAddress[pNeighborRoamInfo->FTRoamInfo.
						preAuthFailList.numMACAddress -
						1], sizeof(tSirMacAddr));
			pNeighborRoamInfo->FTRoamInfo.preAuthFailList.
			numMACAddress--;
		}
	}
	return;
}

/**
 * csr_neighbor_roam_add_preauth_fail() - add bssid to preauth failed list
 * @mac_ctx: The handle returned by mac_open.
 * @bssid: BSSID to be added to the preauth fail list
 *
 * This function adds the given BSSID to the Preauth fail list
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE otherwise
 */
QDF_STATUS csr_neighbor_roam_add_preauth_fail(tpAniSirGlobal mac_ctx,
					uint8_t session_id,
					tSirMacAddr bssid)
{
	uint8_t i = 0;
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	uint8_t num_mac_addr = neighbor_roam_info->FTRoamInfo.preAuthFailList.
				numMACAddress;

	NEIGHBOR_ROAM_DEBUG(mac_ctx, LOGE,
		FL(" Added BSSID " MAC_ADDRESS_STR " to Preauth failed list"),
		MAC_ADDR_ARRAY(bssid));

	for (i = 0;
	     i < neighbor_roam_info->FTRoamInfo.preAuthFailList.numMACAddress;
	     i++) {
		if (true == cdf_mem_compare(
		   neighbor_roam_info->FTRoamInfo.preAuthFailList.macAddress[i],
		   bssid, sizeof(tSirMacAddr))) {
			sms_log(mac_ctx, LOGW, FL("BSSID "MAC_ADDRESS_STR" already present in preauth fail list"),
			MAC_ADDR_ARRAY(bssid));
			return QDF_STATUS_SUCCESS;
		}
	}

	if ((num_mac_addr + 1) > MAX_NUM_PREAUTH_FAIL_LIST_ADDRESS) {
		sms_log(mac_ctx, LOGE,
			FL("Cannot add, preauth fail list is full."));
		return QDF_STATUS_E_FAILURE;
	}
	cdf_mem_copy(neighbor_roam_info->FTRoamInfo.preAuthFailList.
		     macAddress[num_mac_addr], bssid, sizeof(tSirMacAddr));
	neighbor_roam_info->FTRoamInfo.preAuthFailList.numMACAddress++;

	return QDF_STATUS_SUCCESS;
}

/**
 * csr_neighbor_roam_is_preauth_candidate()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @bssId : BSSID of the candidate
 *
 * This function checks whether the given MAC address is already present
 * in the preauth fail list and returns true/false accordingly
 *
 * Return: true if preauth candidate, false otherwise
 */
bool csr_neighbor_roam_is_preauth_candidate(tpAniSirGlobal pMac,
					    uint8_t sessionId, tSirMacAddr bssId)
{
	uint8_t i = 0;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];

	if (csr_roam_is_roam_offload_scan_enabled(pMac)) {
		return true;
	}
	if (0 == pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress)
		return true;

	for (i = 0;
	     i < pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress;
	     i++) {
		if (true ==
		    cdf_mem_compare(pNeighborRoamInfo->FTRoamInfo.
				    preAuthFailList.macAddress[i], bssId,
				    sizeof(tSirMacAddr))) {
			NEIGHBOR_ROAM_DEBUG(pMac, LOGE,
					    FL("BSSID " MAC_ADDRESS_STR
					       " already present in preauth fail list"),
					    MAC_ADDR_ARRAY(bssId));
			return false;
		}
	}

	return true;
}

/**
 * csr_neighbor_roam_issue_preauth_req() - Send preauth request to first AP
 * @mac_ctx: The handle returned by mac_open.
 * @session_id: Session information
 *
 * This function issues preauth request to PE with the 1st AP entry in the
 * roamable AP list
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE otherwise
 */
static QDF_STATUS csr_neighbor_roam_issue_preauth_req(tpAniSirGlobal mac_ctx,
						      uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamBSSInfo neighbor_bss_node;


	if (false != neighbor_roam_info->FTRoamInfo.preauthRspPending) {
		/* This must not be true here */
		CDF_ASSERT(neighbor_roam_info->FTRoamInfo.preauthRspPending ==
			   false);
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * Issue Preauth request to PE here.
	 * Need to issue the preauth request with the BSSID that is in the
	 * head of the roamable AP list. Parameters that should be passed are
	 * BSSID, Channel number and the neighborScanPeriod(probably). If
	 * roamableAPList gets empty, should transition to REPORT_SCAN state
	 */
	neighbor_bss_node = csr_neighbor_roam_next_roamable_ap(mac_ctx,
				&neighbor_roam_info->roamableAPList, NULL);

	if (NULL == neighbor_bss_node) {
		sms_log(mac_ctx, LOGW, FL("Roamable AP list is empty.. "));
		return QDF_STATUS_E_FAILURE;
	} else {
		csr_neighbor_roam_send_lfr_metric_event(mac_ctx, session_id,
			neighbor_bss_node->pBssDescription->bssId,
			eCSR_ROAM_PREAUTH_INIT_NOTIFY);
		status = csr_roam_enqueue_preauth(mac_ctx, session_id,
				neighbor_bss_node->pBssDescription,
				eCsrPerformPreauth, true);

		sms_log(mac_ctx, LOG1,
			FL("Before Pre-Auth: BSSID " MAC_ADDRESS_STR ", Ch:%d"),
			MAC_ADDR_ARRAY(
				neighbor_bss_node->pBssDescription->bssId),
			(int)neighbor_bss_node->pBssDescription->channelId);

		if (QDF_STATUS_SUCCESS != status) {
			sms_log(mac_ctx, LOGE,
				FL("Return failed preauth request status %d"),
				status);
			return status;
		}
	}

	neighbor_roam_info->FTRoamInfo.preauthRspPending = true;

	/* Increment the preauth retry count */
	neighbor_roam_info->FTRoamInfo.numPreAuthRetries++;

	/* Transition the state to preauthenticating */
	CSR_NEIGHBOR_ROAM_STATE_TRANSITION
		(mac_ctx, eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING,
		session_id)

	return status;
}

/**
 * csr_neighbor_roam_preauth_rsp_handler() - handle preauth response
 * @mac_ctx: The handle returned by mac_open.
 * @session_id: SME session
 * @lim_status: eSIR_SUCCESS/eSIR_FAILURE/eSIR_LIM_MAX_STA_REACHED_ERROR/
 *              eSIT_LIM_AUTH_RSP_TIMEOUT status from PE
 *
 * This function handle the Preauth response from PE
 * Every preauth is allowed max 3 tries if it fails. If a bssid failed
 * for more than MAX_TRIES, we will remove it from the list and try
 * with the next node in the roamable AP list and add the BSSID to
 * pre-auth failed list. If no more entries present in roamable AP list,
 * transition to REPORT_SCAN state.
 *
 * Return: QDF_STATUS_SUCCESS on success (i.e. pre-auth processed),
 *         QDF_STATUS_E_FAILURE otherwise
 */
QDF_STATUS csr_neighbor_roam_preauth_rsp_handler(tpAniSirGlobal mac_ctx,
						 uint8_t session_id,
						 tSirRetStatus lim_status)
{
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS preauth_processed = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamBSSInfo preauth_rsp_node = NULL;

	if (false == neighbor_roam_info->FTRoamInfo.preauthRspPending) {
		/*
		 * This can happen when we disconnect immediately
		 * after sending a pre-auth request. During processing
		 * of the disconnect command, we would have reset
		 * preauthRspPending and transitioned to INIT state.
		 */
		NEIGHBOR_ROAM_DEBUG(mac_ctx, LOGW,
			FL("Unexpected pre-auth response in state %d"),
			neighbor_roam_info->neighborRoamState);
		preauth_processed = QDF_STATUS_E_FAILURE;
		goto DEQ_PREAUTH;
	}
	/* We can receive it in these 2 states. */
	if ((neighbor_roam_info->neighborRoamState !=
	     eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING)) {
		NEIGHBOR_ROAM_DEBUG(mac_ctx, LOGW,
			FL("Preauth response received in state %s"),
			mac_trace_get_neighbour_roam_state
				(neighbor_roam_info->neighborRoamState));
		preauth_processed = QDF_STATUS_E_FAILURE;
		goto DEQ_PREAUTH;
	}

	neighbor_roam_info->FTRoamInfo.preauthRspPending = false;

	if (eSIR_SUCCESS == lim_status)
		preauth_rsp_node =
			csr_neighbor_roam_next_roamable_ap(
				mac_ctx,
				&neighbor_roam_info->roamableAPList,
				NULL);
	if ((eSIR_SUCCESS == lim_status) && (NULL != preauth_rsp_node)) {
		NEIGHBOR_ROAM_DEBUG(mac_ctx, LOG1,
			FL("Preauth completed successfully after %d tries"),
			neighbor_roam_info->FTRoamInfo.numPreAuthRetries);
		sms_log(mac_ctx, LOG1,
			FL("After Pre-Auth: BSSID " MAC_ADDRESS_STR ", Ch:%d"),
			MAC_ADDR_ARRAY(
				preauth_rsp_node->pBssDescription->bssId),
			(int)preauth_rsp_node->pBssDescription->channelId);

		csr_neighbor_roam_send_lfr_metric_event(mac_ctx, session_id,
			preauth_rsp_node->pBssDescription->bssId,
			eCSR_ROAM_PREAUTH_STATUS_SUCCESS);
		/*
		 * Preauth completed successfully. Insert the preauthenticated
		 * node to tail of preAuthDoneList.
		 */
		csr_neighbor_roam_remove_roamable_ap_list_entry(mac_ctx,
			&neighbor_roam_info->roamableAPList,
			preauth_rsp_node);
		csr_ll_insert_tail(
			&neighbor_roam_info->FTRoamInfo.preAuthDoneList,
			&preauth_rsp_node->List, LL_ACCESS_LOCK);

		/* Pre-auth successful. Transition to PREAUTH Done state */
		CSR_NEIGHBOR_ROAM_STATE_TRANSITION
			(mac_ctx, eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE,
			session_id)
		neighbor_roam_info->FTRoamInfo.numPreAuthRetries = 0;

		/*
		 * The caller of this function would start a timer and by
		 * the time it expires, supplicant should have provided
		 * the updated FTIEs to SME. So, when it expires, handoff
		 * will be triggered then.
		 */
	} else {
		tpCsrNeighborRoamBSSInfo neighbor_bss_node = NULL;
		tListElem *entry;
		bool is_dis_pending = false;

		sms_log(mac_ctx, LOGE,
			FL("Preauth failed retry number %d, status = 0x%x"),
			neighbor_roam_info->FTRoamInfo.numPreAuthRetries,
			lim_status);

		/*
		 * Preauth failed. Add the bssId to the preAuth failed list
		 * of MAC Address. Also remove the AP from roamable AP list.
		 */
		if ((neighbor_roam_info->FTRoamInfo.numPreAuthRetries >=
		     CSR_NEIGHBOR_ROAM_MAX_NUM_PREAUTH_RETRIES) ||
		    (eSIR_LIM_MAX_STA_REACHED_ERROR == lim_status)) {
			/*
			 * We are going to remove the node as it fails for
			 * more than MAX tries. Reset this count to 0
			 */
			neighbor_roam_info->FTRoamInfo.numPreAuthRetries = 0;

			/*
			 * The one in the head of the list should be one with
			 * which we issued pre-auth and failed
			 */
			entry = csr_ll_remove_head(
					&neighbor_roam_info->roamableAPList,
					LL_ACCESS_LOCK);
			if (!entry) {
				sms_log(mac_ctx, LOGE,
					FL("Preauth list is empty"));
				goto NEXT_PREAUTH;
			}
			neighbor_bss_node = GET_BASE_ADDR(entry,
					tCsrNeighborRoamBSSInfo,
					List);
			/*
			 * Add the BSSID to pre-auth fail list if
			 * it is not requested by HDD
			 */
			if (!neighbor_roam_info->uOsRequestedHandoff)
				status =
					csr_neighbor_roam_add_preauth_fail(
						mac_ctx, session_id,
						neighbor_bss_node->
							pBssDescription->bssId);
			csr_neighbor_roam_send_lfr_metric_event(mac_ctx,
				session_id,
				neighbor_bss_node->pBssDescription->bssId,
				eCSR_ROAM_PREAUTH_STATUS_FAILURE);
			/* Now we can free this node */
			csr_neighbor_roam_free_neighbor_roam_bss_node(
				mac_ctx, neighbor_bss_node);
		}
NEXT_PREAUTH:
		is_dis_pending = is_disconnect_pending(mac_ctx, session_id);
		if (is_dis_pending) {
			sms_log(mac_ctx, LOGE,
				FL("Disconnect in progress, Abort preauth"));
			goto ABORT_PREAUTH;
		}
		/* Issue preauth request for the same/next entry */
		if (QDF_STATUS_SUCCESS == csr_neighbor_roam_issue_preauth_req(
						mac_ctx, session_id))
			goto DEQ_PREAUTH;
ABORT_PREAUTH:
		if (csr_roam_is_roam_offload_scan_enabled(mac_ctx)) {
			if (neighbor_roam_info->uOsRequestedHandoff) {
				neighbor_roam_info->uOsRequestedHandoff = 0;
				csr_roam_offload_scan(mac_ctx, 0,
						      ROAM_SCAN_OFFLOAD_START,
						      REASON_PREAUTH_FAILED_FOR_ALL);
			} else {
				csr_roam_offload_scan(mac_ctx, 0,
						      ROAM_SCAN_OFFLOAD_RESTART,
						      REASON_PREAUTH_FAILED_FOR_ALL);
			}
			CSR_NEIGHBOR_ROAM_STATE_TRANSITION
				(mac_ctx, eCSR_NEIGHBOR_ROAM_STATE_CONNECTED,
				session_id);
		}
	}

DEQ_PREAUTH:
	csr_dequeue_roam_command(mac_ctx, eCsrPerformPreauth);
	return preauth_processed;
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * csr_neighbor_roam_offload_update_preauth_list()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: Session ID
 * @roam_sync_ind_ptr: Roam offload sync Ind Info
 *
 * This function handles the RoamOffloadSynch and adds the
 * roamed AP to the preauth done list
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE otherwise
 */
QDF_STATUS
csr_neighbor_roam_offload_update_preauth_list(tpAniSirGlobal pMac,
	roam_offload_synch_ind *roam_sync_ind_ptr, uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo neighbor_roam_info_ptr =
		&pMac->roam.neighborRoamInfo[session_id];
	tpCsrNeighborRoamBSSInfo bss_info_ptr;
	uint16_t bss_desc_len;

	if (neighbor_roam_info_ptr->neighborRoamState !=
	    eCSR_NEIGHBOR_ROAM_STATE_CONNECTED) {
		NEIGHBOR_ROAM_DEBUG(pMac, LOGW,
			FL("LFR3:Roam Offload Synch Ind received in state %d"),
			neighbor_roam_info_ptr->neighborRoamState);
		return QDF_STATUS_E_FAILURE;
	}

	bss_info_ptr = cdf_mem_malloc(sizeof(tCsrNeighborRoamBSSInfo));
	if (NULL == bss_info_ptr) {
		sms_log(pMac, LOGE,
		FL("LFR3:Memory allocation for Neighbor Roam BSS Info failed"));
		return QDF_STATUS_E_NOMEM;
	}
	bss_desc_len = roam_sync_ind_ptr->bss_desc_ptr->length +
		     sizeof(roam_sync_ind_ptr->bss_desc_ptr->length);
	bss_info_ptr->pBssDescription = cdf_mem_malloc(bss_desc_len);
	if (bss_info_ptr->pBssDescription != NULL) {
		cdf_mem_copy(bss_info_ptr->pBssDescription,
			     roam_sync_ind_ptr->bss_desc_ptr,
			     bss_desc_len);
	} else {
		sms_log(pMac, LOGE,
		FL("LFR3:Mem alloc for Neighbor Roam BSS Descriptor failed"));
		cdf_mem_free(bss_info_ptr);
		return QDF_STATUS_E_NOMEM;

	}
	csr_ll_insert_tail(&neighbor_roam_info_ptr->FTRoamInfo.preAuthDoneList,
			   &bss_info_ptr->List, LL_ACCESS_LOCK);

	CSR_NEIGHBOR_ROAM_STATE_TRANSITION
		(pMac, eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE, session_id)
	neighbor_roam_info_ptr->FTRoamInfo.numPreAuthRetries = 0;
	CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_DEBUG,
		  "LFR3:Entry added to Auth Done List");

	return QDF_STATUS_SUCCESS;
}
#endif
/**
 * csr_neighbor_roam_prepare_scan_profile_filter()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: Session ID
 * @scan_filter: Populated scan filter based on the connected profile
 *
 * This function creates a scan filter based on the currently
 * connected profile. Based on this filter, scan results are obtained
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE otherwise
 */
QDF_STATUS
csr_neighbor_roam_prepare_scan_profile_filter(tpAniSirGlobal pMac,
					      tCsrScanResultFilter *pScanFilter,
					      uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo nbr_roam_info =
		&pMac->roam.neighborRoamInfo[sessionId];
	tCsrRoamConnectedProfile *pCurProfile =
		&pMac->roam.roamSession[sessionId].connectedProfile;
	uint8_t i = 0;
	struct roam_ext_params *roam_params;
	uint8_t num_ch = 0;

	CDF_ASSERT(pScanFilter != NULL);
	if (pScanFilter == NULL)
		return QDF_STATUS_E_FAILURE;

	cdf_mem_zero(pScanFilter, sizeof(tCsrScanResultFilter));
	roam_params = &pMac->roam.configParam.roam_params;
	/* We dont want to set BSSID based Filter */
	pScanFilter->BSSIDs.numOfBSSIDs = 0;
	pScanFilter->scan_filter_for_roam = 1;
	/* only for HDD requested handoff fill in the BSSID in the filter */
	if (nbr_roam_info->uOsRequestedHandoff) {
		pScanFilter->BSSIDs.numOfBSSIDs = 1;
		pScanFilter->BSSIDs.bssid =
			cdf_mem_malloc(sizeof(tSirMacAddr) *
				       pScanFilter->BSSIDs.numOfBSSIDs);
		if (NULL == pScanFilter->BSSIDs.bssid) {
			sms_log(pMac, LOGE,
				FL("Scan Filter BSSID mem alloc failed"));
			return QDF_STATUS_E_NOMEM;
		}

		cdf_mem_zero(pScanFilter->BSSIDs.bssid,
			     sizeof(tSirMacAddr) *
			     pScanFilter->BSSIDs.numOfBSSIDs);

		/* Populate the BSSID from handoff info received from HDD */
		for (i = 0; i < pScanFilter->BSSIDs.numOfBSSIDs; i++) {
			cdf_copy_macaddr(&pScanFilter->BSSIDs.bssid[i],
				 &nbr_roam_info->handoffReqInfo.bssid);
		}
	}
	CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_DEBUG,
		FL("No of Allowed SSID List:%d"),
		roam_params->num_ssid_allowed_list);
	if (roam_params->num_ssid_allowed_list) {
		pScanFilter->SSIDs.numOfSSIDs =
			roam_params->num_ssid_allowed_list;
		pScanFilter->SSIDs.SSIDList =
			cdf_mem_malloc(sizeof(tCsrSSIDInfo) *
				pScanFilter->SSIDs.numOfSSIDs);
		if (NULL == pScanFilter->SSIDs.SSIDList) {
			sms_log(pMac, LOGE,
				FL("Scan Filter SSID mem alloc failed"));
			return QDF_STATUS_E_NOMEM;
		}
		for (i = 0; i < roam_params->num_ssid_allowed_list; i++) {
			pScanFilter->SSIDs.SSIDList[i].handoffPermitted = 1;
			pScanFilter->SSIDs.SSIDList[i].ssidHidden = 0;
			cdf_mem_copy((void *)
				pScanFilter->SSIDs.SSIDList[i].SSID.ssId,
				roam_params->ssid_allowed_list[i].ssId,
				roam_params->ssid_allowed_list[i].length);
			pScanFilter->SSIDs.SSIDList[i].SSID.length =
				roam_params->ssid_allowed_list[i].length;
		}
	} else {
		/* Populate all the information from the connected profile */
		pScanFilter->SSIDs.numOfSSIDs = 1;
		pScanFilter->SSIDs.SSIDList =
			cdf_mem_malloc(sizeof(tCsrSSIDInfo));
		if (NULL == pScanFilter->SSIDs.SSIDList) {
			sms_log(pMac, LOGE,
				FL("Scan Filter SSID mem alloc failed"));
			return QDF_STATUS_E_NOMEM;
		}
		pScanFilter->SSIDs.SSIDList->handoffPermitted = 1;
		pScanFilter->SSIDs.SSIDList->ssidHidden = 0;
		pScanFilter->SSIDs.SSIDList->SSID.length =
			pCurProfile->SSID.length;
		cdf_mem_copy((void *)pScanFilter->SSIDs.SSIDList->SSID.ssId,
			(void *)pCurProfile->SSID.ssId,
			pCurProfile->SSID.length);

		NEIGHBOR_ROAM_DEBUG(pMac, LOG1,
			FL("Filtering for SSID %.*s,length of SSID = %u"),
			pScanFilter->SSIDs.SSIDList->SSID.length,
			pScanFilter->SSIDs.SSIDList->SSID.ssId,
			pScanFilter->SSIDs.SSIDList->SSID.length);
	}
	pScanFilter->authType.numEntries = 1;
	pScanFilter->authType.authType[0] = pCurProfile->AuthType;

	pScanFilter->EncryptionType.numEntries = 1;     /* This must be 1 */
	pScanFilter->EncryptionType.encryptionType[0] =
		pCurProfile->EncryptionType;

	pScanFilter->mcEncryptionType.numEntries = 1;
	pScanFilter->mcEncryptionType.encryptionType[0] =
		pCurProfile->mcEncryptionType;

	pScanFilter->BSSType = pCurProfile->BSSType;

	num_ch =
	    nbr_roam_info->roamChannelInfo.currentChannelListInfo.numOfChannels;
	if (num_ch) {
		/*
		 * We are intrested only in the scan results on channels we
		 * scanned
		 */
		pScanFilter->ChannelInfo.numOfChannels = num_ch;
		pScanFilter->ChannelInfo.ChannelList =
			cdf_mem_malloc(num_ch * sizeof(uint8_t));
		if (NULL == pScanFilter->ChannelInfo.ChannelList) {
			sms_log(pMac, LOGE,
				FL("Scan Filter Ch list mem alloc failed"));
			cdf_mem_free(pScanFilter->SSIDs.SSIDList);
			pScanFilter->SSIDs.SSIDList = NULL;
			return QDF_STATUS_E_NOMEM;
		}
		for (i = 0; i < pScanFilter->ChannelInfo.numOfChannels; i++) {
			pScanFilter->ChannelInfo.ChannelList[i] =
			  nbr_roam_info->roamChannelInfo.currentChannelListInfo.
			  ChannelList[i];
		}
	} else {
		pScanFilter->ChannelInfo.numOfChannels = 0;
		pScanFilter->ChannelInfo.ChannelList = NULL;
	}

	if (nbr_roam_info->is11rAssoc) {
		/*
		 * MDIE should be added as a part of profile. This should be
		 * added as a part of filter as well
		 */
		pScanFilter->MDID.mdiePresent = pCurProfile->MDID.mdiePresent;
		pScanFilter->MDID.mobilityDomain =
			pCurProfile->MDID.mobilityDomain;
	}

#ifdef WLAN_FEATURE_11W
	pScanFilter->MFPEnabled = pCurProfile->MFPEnabled;
	pScanFilter->MFPRequired = pCurProfile->MFPRequired;
	pScanFilter->MFPCapable = pCurProfile->MFPCapable;
#endif
	return QDF_STATUS_SUCCESS;
}

uint32_t csr_get_current_ap_rssi(tpAniSirGlobal pMac,
				 tScanResultHandle *pScanResultList,
				 uint8_t sessionId)
{
	tCsrScanResultInfo *pScanResult;
	tpCsrNeighborRoamControlInfo nbr_roam_info =
		&pMac->roam.neighborRoamInfo[sessionId];
	/* We are setting this as default value to make sure we return this value,
	   when we do not see this AP in the scan result for some reason.However,it is
	   less likely that we are associated to an AP and do not see it in the scan list */
	uint32_t CurrAPRssi = -125;

	while (NULL !=
	       (pScanResult = csr_scan_result_get_next(pMac, *pScanResultList))) {
		if (true ==
		    cdf_mem_compare(pScanResult->BssDescriptor.bssId,
				    nbr_roam_info->currAPbssid.bytes,
				    sizeof(tSirMacAddr))) {
			/* We got a match with the currently associated AP.
			 * Capture the RSSI value and complete the while loop.
			 * The while loop is completed in order to make the current entry go back to NULL,
			 * and in the next while loop, it properly starts searching from the head of the list.
			 * TODO: Can also try setting the current entry directly to NULL as soon as we find the new AP*/

			CurrAPRssi =
				(int)pScanResult->BssDescriptor.rssi * (-1);

		} else {
			continue;
		}
	}

	return CurrAPRssi;

}

/**
 * csr_neighbor_roam_process_scan_results() - build roaming candidate list
 *
 * @mac_ctx: The handle returned by mac_open.
 * @sessionid: Session information
 * @scan_results_list: List obtained from csr_scan_get_result()
 *
 * This function applies various candidate checks like LFR, 11r, preauth, ESE
 * and builds a roamable AP list. It applies age limit only if no suitable
 * recent candidates are found.
 *
 * Output list is built in mac_ctx->roam.neighborRoamInfo[sessionid].
 *
 * Return: void
 */

static void
csr_neighbor_roam_process_scan_results(tpAniSirGlobal mac_ctx,
		uint8_t sessionid,
		tScanResultHandle *scan_results_list)
{
	tCsrScanResultInfo *scan_result;
	tpCsrNeighborRoamControlInfo n_roam_info =
		&mac_ctx->roam.neighborRoamInfo[sessionid];
	tpCsrNeighborRoamBSSInfo bss_info;
	uint32_t cur_ap_rssi;
	uint32_t age_ticks = 0;
	uint32_t limit_ticks =
		qdf_system_msecs_to_ticks(ROAM_AP_AGE_LIMIT_MS);
	uint8_t num_candidates = 0;
	uint8_t num_dropped = 0;
	/*
	 * first iteration of scan list should consider
	 * age constraint for candidates
	 */
	bool age_constraint = true;
#ifdef FEATURE_WLAN_ESE
	uint16_t qpresent;
	uint16_t qavail;
	bool voadmitted;
#endif
	/*
	 * Find out the Current AP RSSI and keep it handy to check if
	 * it is better than the RSSI of the AP which we are
	 * going to roam.If so, we are going to continue with the
	 * current AP.
	 */
	cur_ap_rssi = csr_get_current_ap_rssi(mac_ctx, scan_results_list,
				sessionid);

	/*
	 * Expecting the scan result already to be in the sorted order based on
	 * RSSI. Based on the previous state we need to check whether the list
	 * should be sorted again taking neighbor score into consideration. If
	 * previous state is CFG_CHAN_LIST_SCAN, there should not be a neighbor
	 * score associated with any of the BSS. If the previous state is
	 * REPORT_QUERY, then there will be neighbor score for each of the APs.
	 * For now, let us take top of the list provided as it is by CSR Scan
	 * result API. Hence it is assumed that neighbor score and rssi score
	 * are in the same order. This will be taken care later.
	 */

	do {
		while (true) {
			tSirBssDescription *descr;

			scan_result = csr_scan_result_get_next(
						mac_ctx, *scan_results_list);
			if (NULL == scan_result)
				break;
			descr = &scan_result->BssDescriptor;
			CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_DEBUG,
				  FL("Scan result: BSSID " MAC_ADDRESS_STR
				     " (Rssi %ld, Ch:%d)"),
				  MAC_ADDR_ARRAY(descr->bssId),
				  abs(descr->rssi), descr->channelId);

			if (true == cdf_mem_compare(descr->bssId,
					n_roam_info->currAPbssid.bytes,
					sizeof(tSirMacAddr))) {
				/*
				 * currently associated AP. Do not have this
				 * in the roamable AP list
				 */
				CDF_TRACE(QDF_MODULE_ID_SME,
					  CDF_TRACE_LEVEL_INFO,
					  "SKIP-currently associated AP");
				continue;
			}
			/*
			 * In case of reassoc requested by upper layer, look
			 * for exact match of bssid & channel. csr cache might
			 * have duplicates
			 */
			if ((n_roam_info->uOsRequestedHandoff) &&
			    ((false == cdf_mem_compare(descr->bssId,
					n_roam_info->handoffReqInfo.bssid.bytes,
					sizeof(tSirMacAddr)))
			     || (descr->channelId !=
				 n_roam_info->handoffReqInfo.channel))) {
				CDF_TRACE(QDF_MODULE_ID_SME,
					  CDF_TRACE_LEVEL_INFO,
					  "SKIP-not a candidate AP for OS requested roam");
				continue;
			}

			if ((n_roam_info->is11rAssoc) &&
			    (!csr_neighbor_roam_is_preauth_candidate(mac_ctx,
					sessionid, descr->bssId))) {
				sms_log(mac_ctx, LOGE,
					FL("BSSID present in pre-auth fail list.. Ignoring"));
				continue;
			}

#ifdef FEATURE_WLAN_ESE
			if (!csr_roam_is_roam_offload_scan_enabled(mac_ctx) &&
			    (n_roam_info->isESEAssoc) &&
			    !csr_neighbor_roam_is_preauth_candidate(mac_ctx,
				sessionid, descr->bssId)) {
				sms_log(mac_ctx, LOGE,
					FL("BSSID present in pre-auth fail list.. Ignoring"));
				continue;
			}

			qpresent = descr->QBSSLoad_present;
			qavail = descr->QBSSLoad_avail;
			voadmitted = n_roam_info->isVOAdmitted;
			if (voadmitted)
				sms_log(mac_ctx, LOG1,
					FL("New AP QBSS present = %s, BW available = 0x%x, required = 0x%x"),
					((qpresent) ? "yes" : "no"), qavail,
					n_roam_info->MinQBssLoadRequired);
			if (voadmitted && qpresent &&
			    (qavail < n_roam_info->MinQBssLoadRequired)) {
				CDF_TRACE(QDF_MODULE_ID_SME,
					CDF_TRACE_LEVEL_INFO,
					"BSSID : " MAC_ADDRESS_STR " has no bandwidth, ignoring",
					MAC_ADDR_ARRAY(descr->bssId));
				continue;
			}
			if (voadmitted && !qpresent) {
				CDF_TRACE(QDF_MODULE_ID_SME,
					CDF_TRACE_LEVEL_INFO,
					"BSSID : " MAC_ADDRESS_STR " has no QBSS LOAD IE, ignoring",
					MAC_ADDR_ARRAY(descr->bssId));
				continue;
			}
#endif /* FEATURE_WLAN_ESE */

			/*
			 * If we are supporting legacy roaming, and
			 * if the candidate is on the "pre-auth failed" list,
			 * ignore it.
			 */
			if (csr_roam_is_fast_roam_enabled(mac_ctx, sessionid) &&
			    !csr_neighbor_roam_is_preauth_candidate(mac_ctx,
				sessionid, descr->bssId)) {
				sms_log(mac_ctx, LOGE,
					FL("BSSID present in pre-auth fail list.. Ignoring"));
				continue;
			}

			/* check the age of the AP */
			age_ticks = (uint32_t) cdf_mc_timer_get_system_ticks() -
					descr->nReceivedTime;
			if (age_constraint == true && age_ticks > limit_ticks) {
				num_dropped++;
				CDF_TRACE(QDF_MODULE_ID_SME,
					CDF_TRACE_LEVEL_WARN,
					FL("Old AP (probe rsp/beacon) skipped.")
					);
				continue;
			}

			/* Finished all checks, now add it to candidate list */
			bss_info =
				cdf_mem_malloc(sizeof(tCsrNeighborRoamBSSInfo));
			if (NULL == bss_info) {
				sms_log(mac_ctx, LOGE,
					FL("Memory allocation failed. Ignored candidate."));
				continue;
			}
			bss_info->pBssDescription =
				cdf_mem_malloc(descr->length +
					sizeof(descr->length));
			if (bss_info->pBssDescription != NULL) {
				cdf_mem_copy(bss_info->pBssDescription, descr,
					descr->length + sizeof(descr->length));
			} else {
				sms_log(mac_ctx, LOGE,
					FL("Memory allocation failed. Ignored candidate."));
				cdf_mem_free(bss_info);
				continue;
			}
			/*
			 * Assign some preference value for now. Need to
			 * calculate theactual score based on RSSI and neighbor
			 * AP score
			 */
			bss_info->apPreferenceVal = 10;
			num_candidates++;
			csr_ll_insert_tail(&n_roam_info->roamableAPList,
				&bss_info->List, LL_ACCESS_LOCK);
		} /* end of while (csr_scan_result_get_next) */

		/* if some candidates were found, then no need to repeat */
		if (num_candidates)
			break;
		/*
		 * if age_constraint is already false, we have done two
		 * iterations and no candidate were found
		 */
		if (age_constraint == false) {
			CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_ERROR,
				  "%s: No roam able candidates found",
				  __func__);
			break;
		}
		/*
		 * if all candidates were dropped rescan the scan
		 * list but this time without age constraint.
		 */
		age_constraint = false;
		/* if no candidates were dropped no need to repeat */
	} while (num_dropped);

	/*
	 * Now we have all the scan results in our local list. Good time to free
	 * up the the list we got as a part of csrGetScanResult
	 */
	csr_scan_result_purge(mac_ctx, *scan_results_list);
}

static QDF_STATUS csr_neighbor_roam_process_scan_complete(tpAniSirGlobal pMac,
							  uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	tCsrScanResultFilter scanFilter;
	tScanResultHandle scanResult;
	uint32_t tempVal = 0;
	QDF_STATUS hstatus;

	hstatus = csr_neighbor_roam_prepare_scan_profile_filter(pMac,
								&scanFilter,
								sessionId);
	NEIGHBOR_ROAM_DEBUG(pMac, LOGW,
			    FL
			    ("11R/ESE/Other Association: Prepare scan to find neighbor AP filter status  = %d"),
			    hstatus);
	if (QDF_STATUS_SUCCESS != hstatus) {
		sms_log(pMac, LOGE,
			FL
			("Scan Filter preparation failed for Assoc type %d.. Bailing out.."),
			tempVal);
		return QDF_STATUS_E_FAILURE;
	}
	hstatus = csr_scan_get_result(pMac, &scanFilter, &scanResult);
	if (hstatus != QDF_STATUS_SUCCESS) {
		NEIGHBOR_ROAM_DEBUG(pMac, LOGE,
				    FL("Get Scan Result status code %d"),
				    hstatus);
	}
	/* Process the scan results and update roamable AP list */
	csr_neighbor_roam_process_scan_results(pMac, sessionId, &scanResult);

	/* Free the scan filter */
	csr_free_scan_filter(pMac, &scanFilter);

	tempVal = csr_ll_count(&pNeighborRoamInfo->roamableAPList);

	if (tempVal) {
		csr_neighbor_roam_trigger_handoff(pMac, sessionId);
		return QDF_STATUS_SUCCESS;
	}

	if (csr_roam_is_roam_offload_scan_enabled(pMac)) {
		if (pNeighborRoamInfo->uOsRequestedHandoff) {
			csr_roam_offload_scan(pMac, sessionId,
					      ROAM_SCAN_OFFLOAD_START,
					      REASON_NO_CAND_FOUND_OR_NOT_ROAMING_NOW);
			pNeighborRoamInfo->uOsRequestedHandoff = 0;
		} else {
			/* There is no candidate or We are not roaming Now.
			 * Inform the FW to restart Roam Offload Scan  */
			csr_roam_offload_scan(pMac, sessionId,
					      ROAM_SCAN_OFFLOAD_RESTART,
					      REASON_NO_CAND_FOUND_OR_NOT_ROAMING_NOW);
		}
		CSR_NEIGHBOR_ROAM_STATE_TRANSITION
			(pMac, eCSR_NEIGHBOR_ROAM_STATE_CONNECTED, sessionId);
	}
	return QDF_STATUS_SUCCESS;

}


#if defined WLAN_FEATURE_VOWIFI
/**
 * csr_neighbor_roam_channels_filter_by_current_band()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: Session ID
 * @input_ch_list: The input channel list
 * @input_num_of_ch: The number of channels in input channel list
 * @output_ch_list: The output channel list
 * @output_num_of_ch: The number of channels in output channel list
 * @merged_output_num_of_ch: The final number of channels in the
 * 				output channel list.
 *
 * This function is used to filter out the channels based on the
 * currently associated AP channel
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE otherwise
 */
QDF_STATUS csr_neighbor_roam_channels_filter_by_current_band(tpAniSirGlobal pMac,
							     uint8_t sessionId,
							     uint8_t *
							     pInputChannelList,
							     uint8_t
							     inputNumOfChannels,
							     uint8_t *
							     pOutputChannelList,
							     uint8_t *
							     pMergedOutputNumOfChannels)
{
	uint8_t i = 0;
	uint8_t numChannels = 0;
	uint8_t currAPoperationChannel =
		pMac->roam.neighborRoamInfo[sessionId].currAPoperationChannel;
	/* Check for NULL pointer */
	if (!pInputChannelList)
		return QDF_STATUS_E_INVAL;

	/* Check for NULL pointer */
	if (!pOutputChannelList)
		return QDF_STATUS_E_INVAL;

	if (inputNumOfChannels > WNI_CFG_VALID_CHANNEL_LIST_LEN) {
		CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_ERROR,
			  "%s: Wrong Number of Input Channels %d",
			  __func__, inputNumOfChannels);
		return QDF_STATUS_E_INVAL;
	}
	for (i = 0; i < inputNumOfChannels; i++) {
		if (get_rf_band(currAPoperationChannel) ==
		    get_rf_band(pInputChannelList[i])) {
			pOutputChannelList[numChannels] = pInputChannelList[i];
			numChannels++;
		}
	}

	/* Return final number of channels */
	*pMergedOutputNumOfChannels = numChannels;

	return QDF_STATUS_SUCCESS;
}

/**
 * csr_neighbor_roam_channels_filter_by_current_band()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: Session ID
 * @input_ch_list: The additional channels to merge in to the
 * 			"merged" channels list.
 * @input_num_of_ch: The number of additional channels.
 * @output_ch_list: The place to put the "merged" channel list.
 * @output_num_of_ch: The original number of channels in the
 * 			"merged" channels list.
 * @merged_output_num_of_ch: The final number of channels in the
 * 				"merged" channel list.
 *
 * This function is used to merge two channel list.
 * NB: If called with outputNumOfChannels == 0, this routines simply
 * copies the input channel list to the output channel list. if number
 * of merged channels are more than 100, num of channels set to 100
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE otherwise
 */
QDF_STATUS csr_neighbor_roam_merge_channel_lists(tpAniSirGlobal pMac,
						 uint8_t *pInputChannelList,
						 uint8_t inputNumOfChannels,
						 uint8_t *pOutputChannelList,
						 uint8_t outputNumOfChannels,
						 uint8_t *
						 pMergedOutputNumOfChannels)
{
	uint8_t i = 0;
	uint8_t j = 0;
	uint8_t numChannels = outputNumOfChannels;

	/* Check for NULL pointer */
	if (!pInputChannelList)
		return QDF_STATUS_E_INVAL;

	/* Check for NULL pointer */
	if (!pOutputChannelList)
		return QDF_STATUS_E_INVAL;

	if (inputNumOfChannels > WNI_CFG_VALID_CHANNEL_LIST_LEN) {
		CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_ERROR,
			  "%s: Wrong Number of Input Channels %d",
			  __func__, inputNumOfChannels);
		return QDF_STATUS_E_INVAL;
	}
	if (outputNumOfChannels >= WNI_CFG_VALID_CHANNEL_LIST_LEN) {
		CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_ERROR,
			  "%s: Wrong Number of Output Channels %d",
			  __func__, outputNumOfChannels);
		return QDF_STATUS_E_INVAL;
	}
	/* Add the "new" channels in the input list to the end of the output list. */
	for (i = 0; i < inputNumOfChannels; i++) {
		for (j = 0; j < outputNumOfChannels; j++) {
			if (pInputChannelList[i] == pOutputChannelList[j])
				break;
		}
		if (j == outputNumOfChannels) {
			if (pInputChannelList[i]) {
				CDF_TRACE(QDF_MODULE_ID_SME,
					  CDF_TRACE_LEVEL_INFO,
					  "%s: [INFOLOG] Adding extra %d to Neighbor channel list",
					  __func__, pInputChannelList[i]);
				pOutputChannelList[numChannels] =
					pInputChannelList[i];
				numChannels++;
			}
		}
		if (numChannels >= WNI_CFG_VALID_CHANNEL_LIST_LEN) {
			CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_INFO,
				  "%s: Merge Neighbor channel list reached Max "
				  "limit %d", __func__, numChannels);
			break;
		}
	}

	/* Return final number of channels */
	*pMergedOutputNumOfChannels = numChannels;

	return QDF_STATUS_SUCCESS;
}

/**
 * csr_neighbor_roam_create_chan_list_from_neighbor_report()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: Session ID
 *
 * This function is invoked when neighbor report is received for the
 * neighbor request. Based on the channels present in the neighbor report,
 * it generates channel list which will be used in REPORT_SCAN state to
 * scan for these neighbor APs
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_FAILURE otherwise
 */
QDF_STATUS csr_neighbor_roam_create_chan_list_from_neighbor_report(tpAniSirGlobal pMac,
								   uint8_t sessionId)
{
	tpRrmNeighborReportDesc pNeighborBssDesc;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	uint8_t numChannels = 0;
	uint8_t i = 0;
	uint8_t channelList[MAX_BSS_IN_NEIGHBOR_RPT];
	uint8_t mergedOutputNumOfChannels = 0;

	/* This should always start from 0 whenever we create a channel list out of neighbor AP list */
	pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport = 0;

	pNeighborBssDesc = sme_rrm_get_first_bss_entry_from_neighbor_cache(pMac);

	while (pNeighborBssDesc) {
		if (pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport >=
		    MAX_BSS_IN_NEIGHBOR_RPT)
			break;

		/* Update the neighbor BSS Info in the 11r FT Roam Info */
		pNeighborRoamInfo->FTRoamInfo.
		neighboReportBssInfo[pNeighborRoamInfo->FTRoamInfo.
				     numBssFromNeighborReport].channelNum =
			pNeighborBssDesc->pNeighborBssDescription->channel;
		pNeighborRoamInfo->FTRoamInfo.
		neighboReportBssInfo[pNeighborRoamInfo->FTRoamInfo.
				     numBssFromNeighborReport].
		neighborScore = (uint8_t) pNeighborBssDesc->roamScore;
		cdf_mem_copy(pNeighborRoamInfo->FTRoamInfo.
			     neighboReportBssInfo[pNeighborRoamInfo->FTRoamInfo.
						  numBssFromNeighborReport].
			     neighborBssId,
			     pNeighborBssDesc->pNeighborBssDescription->bssId,
			     sizeof(tSirMacAddr));
		pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport++;

		/* Saving the channel list non-redundantly */
		for (i = 0; (i < numChannels && i < MAX_BSS_IN_NEIGHBOR_RPT);
		     i++) {
			if (pNeighborBssDesc->pNeighborBssDescription->
			    channel == channelList[i])
				break;
		}

		if (i == numChannels) {
			if (pNeighborBssDesc->pNeighborBssDescription->channel) {
				if (CSR_IS_ROAM_INTRA_BAND_ENABLED(pMac)) {
					/* Make sure to add only if its the same band */
					if (get_rf_band
						    (pNeighborRoamInfo->
						    currAPoperationChannel) ==
					    get_rf_band(pNeighborBssDesc->
							pNeighborBssDescription->
							channel)) {
						CDF_TRACE(QDF_MODULE_ID_SME,
							  CDF_TRACE_LEVEL_INFO,
							  "%s: [INFOLOG] Adding %d to Neighbor channel list (Same band)\n",
							  __func__,
							  pNeighborBssDesc->
							  pNeighborBssDescription->
							  channel);
						channelList[numChannels] =
							pNeighborBssDesc->
							pNeighborBssDescription->
							channel;
						numChannels++;
					}
				} else {
					CDF_TRACE(QDF_MODULE_ID_SME,
						  CDF_TRACE_LEVEL_INFO,
						  "%s: [INFOLOG] Adding %d to Neighbor channel list\n",
						  __func__,
						  pNeighborBssDesc->
						  pNeighborBssDescription->
						  channel);
					channelList[numChannels] =
						pNeighborBssDesc->
						pNeighborBssDescription->channel;
					numChannels++;
				}
			}
		}

		pNeighborBssDesc =
			sme_rrm_get_next_bss_entry_from_neighbor_cache(pMac,
								       pNeighborBssDesc);
	}

	if (pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
	    ChannelList) {
		cdf_mem_free(pNeighborRoamInfo->roamChannelInfo.
			     currentChannelListInfo.ChannelList);
	}

	pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList =
		NULL;
	pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
	numOfChannels = 0;
	/* Store the obtained channel list to the Neighbor Control data structure */
	if (numChannels)
		pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
		ChannelList =
			cdf_mem_malloc((numChannels) * sizeof(uint8_t));
	if (NULL ==
	    pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
	    ChannelList) {
		sms_log(pMac, LOGE,
			FL
				("Memory allocation for Channel list failed.. TL event ignored"));
		return QDF_STATUS_E_NOMEM;
	}

	cdf_mem_copy(pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
		     ChannelList, channelList, (numChannels) * sizeof(uint8_t));
	pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
	numOfChannels = numChannels;
	/*
	 * Create a Union of occupied channel list learnt by the DUT along with the Neighbor
	 * report Channels. This increases the chances of the DUT to get a candidate AP while
	 * roaming even if the Neighbor Report is not able to provide sufficient information.
	 * */
	if (pMac->scan.occupiedChannels[sessionId].numChannels) {
		csr_neighbor_roam_merge_channel_lists(pMac,
						      &pMac->scan.
						      occupiedChannels[sessionId].
						      channelList[0],
						      pMac->scan.
						      occupiedChannels[sessionId].
						      numChannels,
						      &pNeighborRoamInfo->
						      roamChannelInfo.
						      currentChannelListInfo.
						      ChannelList[0],
						      pNeighborRoamInfo->
						      roamChannelInfo.
						      currentChannelListInfo.
						      numOfChannels,
						      &mergedOutputNumOfChannels);
		pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
		numOfChannels = mergedOutputNumOfChannels;

	}
	/*Indicate the firmware about the update only if any new channels are added.
	 * Otherwise, the firmware would already be knowing the non-IAPPneighborlist
	 * channels. There is no need to update.*/
	if (numChannels) {
		sms_log(pMac, LOG1,
			FL("IAPP Neighbor list callback received as expected"
			   "in state %s."),
			mac_trace_get_neighbour_roam_state(pNeighborRoamInfo->
							   neighborRoamState));
		pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived =
			true;
		if (csr_roam_is_roam_offload_scan_enabled(pMac)) {
			csr_roam_offload_scan(pMac, sessionId,
					      ROAM_SCAN_OFFLOAD_UPDATE_CFG,
					      REASON_CHANNEL_LIST_CHANGED);
		}
	}
	pNeighborRoamInfo->roamChannelInfo.currentChanIndex = 0;

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_VOWIFI */

/**
 * csr_neighbor_roam_is_ssid_and_security_match() - to match ssid/security
 * @pMac: Pointer to mac context
 * @pCurProfile: pointer to current roam profile
 * @pBssDesc: pointer to bss description
 * @pIes: pointer to local ies
 *
 * This routine will be called to see if SSID and security parameters
 * are matching.
 *
 * Return: bool
 */
bool csr_neighbor_roam_is_ssid_and_security_match(tpAniSirGlobal pMac,
		tCsrRoamConnectedProfile *pCurProfile,
		tSirBssDescription *pBssDesc, tDot11fBeaconIEs *pIes)
{
	tCsrAuthList authType;
	tCsrEncryptionList uCEncryptionType;
	tCsrEncryptionList mCEncryptionType;
	bool fMatch = false;

	authType.numEntries = 1;
	authType.authType[0] = pCurProfile->AuthType;
	uCEncryptionType.numEntries = 1;
	uCEncryptionType.encryptionType[0] = pCurProfile->EncryptionType;
	mCEncryptionType.numEntries = 1;
	mCEncryptionType.encryptionType[0] = pCurProfile->mcEncryptionType;

	/* Again, treat missing pIes as a non-match. */
	if (!pIes)
		return false;

	/* Treat a missing SSID as a non-match. */
	if (!pIes->SSID.present)
		return false;

	fMatch = csr_is_ssid_match(pMac,
			(void *)pCurProfile->SSID.ssId,
			pCurProfile->SSID.length,
			pIes->SSID.ssid,
			pIes->SSID.num_ssid, true);
	if (true == fMatch) {
#ifdef WLAN_FEATURE_11W
		/*
		 * We are sending current connected APs profile setting
		 * if other AP doesn't have the same PMF setting as currently
		 * connected AP then we will have some issues in roaming.
		 *
		 * Make sure all the APs have same PMF settings to avoid
		 * any corner cases.
		 */
		fMatch = csr_is_security_match(pMac, &authType,
				&uCEncryptionType, &mCEncryptionType,
				&pCurProfile->MFPEnabled,
				&pCurProfile->MFPRequired,
				&pCurProfile->MFPCapable,
				pBssDesc, pIes, NULL, NULL, NULL);
#else
		fMatch = csr_is_security_match(pMac, &authType,
				&uCEncryptionType,
				&mCEncryptionType, NULL,
				NULL, NULL, pBssDesc,
				pIes, NULL, NULL, NULL);
#endif
		return fMatch;
	} else {
		return fMatch;
	}

}

bool csr_neighbor_roam_is_new_connected_profile(tpAniSirGlobal pMac,
						uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	tCsrRoamConnectedProfile *pCurrProfile = NULL;
	tCsrRoamConnectedProfile *pPrevProfile = NULL;
	tDot11fBeaconIEs *pIes = NULL;
	tSirBssDescription *pBssDesc = NULL;
	bool fNew = true;

	if (!(pMac->roam.roamSession && CSR_IS_SESSION_VALID(pMac, sessionId))) {
		return fNew;
	}

	pCurrProfile = &pMac->roam.roamSession[sessionId].connectedProfile;
	if (!pCurrProfile) {
		return fNew;
	}

	pPrevProfile = &pNeighborRoamInfo->prevConnProfile;
	if (!pPrevProfile) {
		return fNew;
	}

	pBssDesc = pPrevProfile->pBssDesc;
	if (pBssDesc) {
		if (QDF_IS_STATUS_SUCCESS(
		    csr_get_parsed_bss_description_ies(pMac, pBssDesc, &pIes))
		    && csr_neighbor_roam_is_ssid_and_security_match(pMac,
					pCurrProfile, pBssDesc, pIes)) {
			fNew = false;
		}
		if (pIes) {
			cdf_mem_free(pIes);
		}
	}

	if (fNew) {
		sms_log(pMac, LOG1,
			FL("Prev roam profile did not match current"));
	} else {
		sms_log(pMac, LOG1, FL("Prev roam profile matches current"));
	}

	return fNew;
}

bool csr_neighbor_roam_connected_profile_match(tpAniSirGlobal pMac,
					       uint8_t sessionId,
					       tCsrScanResult *pResult,
					       tDot11fBeaconIEs *pIes)
{
	tCsrRoamConnectedProfile *pCurProfile = NULL;
	tSirBssDescription *pBssDesc = &pResult->Result.BssDescriptor;

	if (!(pMac->roam.roamSession && CSR_IS_SESSION_VALID(pMac, sessionId))) {
		return false;
	}

	pCurProfile = &pMac->roam.roamSession[sessionId].connectedProfile;

	if (!pCurProfile) {
		return false;
	}

	return csr_neighbor_roam_is_ssid_and_security_match(pMac, pCurProfile,
							    pBssDesc, pIes);
}

/**
 * csr_neighbor_roam_prepare_non_occupied_channel_list() - prepare non-occup CL
 * @pMac: The handle returned by mac_open
 * @pInputChannelList: The default channels list
 * @numOfChannels: The number of channels in the default channels list
 * @pOutputChannelList: The place to put the non-occupied channel list
 * @pOutputNumOfChannels: Number of channels in the non-occupied channel list
 *
 * This function is used to prepare a channel list that is derived from
 * the list of valid channels and does not include those in the occupied list
 *
 * Return QDF_STATUS
 */
QDF_STATUS
csr_neighbor_roam_prepare_non_occupied_channel_list(tpAniSirGlobal pMac,
		uint8_t sessionId, uint8_t *pInputChannelList,
		uint8_t numOfChannels, uint8_t *pOutputChannelList,
		uint8_t *pOutputNumOfChannels)
{
	uint8_t i = 0;
	uint8_t outputNumOfChannels = 0;
	uint8_t numOccupiedChannels =
			pMac->scan.occupiedChannels[sessionId].numChannels;
	uint8_t *pOccupiedChannelList =
			pMac->scan.occupiedChannels[sessionId].channelList;

	for (i = 0; i < numOfChannels; i++) {
		if (csr_is_channel_present_in_list
				(pOccupiedChannelList, numOccupiedChannels,
				 pInputChannelList[i]))
			continue;
		/*
		 * DFS channel will be added in the list only when the
		 * DFS Roaming scan flag is enabled
		 */
		if (CDS_IS_DFS_CH(pInputChannelList[i])) {
			if (CSR_ROAMING_DFS_CHANNEL_DISABLED !=
				pMac->roam.configParam.allowDFSChannelRoam) {
				pOutputChannelList[outputNumOfChannels++] =
						pInputChannelList[i];
			}
		} else {
			pOutputChannelList[outputNumOfChannels++] =
						pInputChannelList[i];
		}
	}

	sms_log(pMac, LOG2,
		FL("Number of channels in the valid channel list=%d; "
		   "Number of channels in the non-occupied list list=%d"),
		numOfChannels, outputNumOfChannels);

	/* Return the number of channels */
	*pOutputNumOfChannels = outputNumOfChannels;
	return QDF_STATUS_SUCCESS;
}

/**
 * csr_roam_reset_roam_params - API to reset the roaming parameters
 * @mac_ctx:          Pointer to the global MAC structure
 *
 * The BSSID blacklist should not be cleared since it has to
 * be used across connections. These parameters will be cleared
 * and sent to firmware with with the roaming STOP command.
 *
 * Return: VOID
 */
void csr_roam_reset_roam_params(tpAniSirGlobal mac_ctx)
{
	struct roam_ext_params *roam_params = NULL;

	/*
	 * clear all the whitelist parameters and remaining
	 * needs to be retained across connections.
	 */
	CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_DEBUG,
		FL("Roaming parameters are reset"));
	roam_params = &mac_ctx->roam.configParam.roam_params;
	roam_params->num_ssid_allowed_list = 0;
	cdf_mem_set(&roam_params->ssid_allowed_list, 0,
			sizeof(tSirMacSSid) * MAX_SSID_ALLOWED_LIST);
}

/**
 * csr_neighbor_roam_indicate_disconnect() - To indicate disconnect
 * @pMac: The handle returned by mac_open
 * @sessionId: CSR session id that got disconnected
 *
 * This function is called by CSR as soon as the station disconnects
 * from the AP. This function does the necessary cleanup of neighbor roam
 * data structures. Neighbor roam state transitions to INIT state whenever
 * this function is called except if the current state is REASSOCIATING
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_neighbor_roam_indicate_disconnect(tpAniSirGlobal pMac,
						 uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
			&pMac->roam.neighborRoamInfo[sessionId];
	tCsrRoamConnectedProfile *pPrevProfile =
			&pNeighborRoamInfo->prevConnProfile;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	tCsrRoamSession *roam_session = NULL;

	if (NULL == pSession) {
		sms_log(pMac, LOGE, FL("pSession is NULL"));
		return QDF_STATUS_E_FAILURE;
	}
	CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_INFO,
			FL("Disconn ind on session %d in state %d from bss :"
			MAC_ADDRESS_STR), sessionId,
			pNeighborRoamInfo->neighborRoamState,
			MAC_ADDR_ARRAY(pSession->connectedProfile.bssid.bytes));
	/*
	 * Free the current previous profile and move
	 * the current profile to prev profile.
	 */
	csr_roam_free_connect_profile(pPrevProfile);
	csr_roam_copy_connect_profile(pMac, sessionId, pPrevProfile);
	/*
	 * clear the roaming parameters that are per connection.
	 * For a new connection, they have to be programmed again.
	 */
	if (!csr_neighbor_middle_of_roaming((tHalHandle)pMac, sessionId))
		csr_roam_reset_roam_params(pMac);
	if (NULL != pSession) {
		roam_session = &pMac->roam.roamSession[sessionId];
		if (NULL != pSession->pCurRoamProfile && (QDF_STA_MODE !=
			roam_session->pCurRoamProfile->csrPersona)) {
			sms_log(pMac, LOGE,
				FL("Ignore disconn ind rcvd from nonSTA persona"
					"sessionId: %d, csrPersonna %d"),
				sessionId,
				(int)roam_session->pCurRoamProfile->csrPersona);
			return QDF_STATUS_SUCCESS;
		}
#ifdef FEATURE_WLAN_ESE
		if (pSession->connectedProfile.isESEAssoc) {
			cdf_mem_copy(&pSession->prevApSSID,
				&pSession->connectedProfile.SSID,
				sizeof(tSirMacSSid));
			cdf_copy_macaddr(&pSession->prevApBssid,
					&pSession->connectedProfile.bssid);
			pSession->prevOpChannel =
				pSession->connectedProfile.operationChannel;
			pSession->isPrevApInfoValid = true;
			pSession->roamTS1 = cdf_mc_timer_get_system_time();
		}
#endif
	}

	switch (pNeighborRoamInfo->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING:
		/*
		 * Stop scan and neighbor refresh timers.
		 * These are indeed not required when we'r in reassoc state.
		 */
		if (!CSR_IS_ROAM_SUBSTATE_DISASSOC_HO(pMac, sessionId)) {
			/*
			 * Disconnect indication during Disassoc Handoff
			 * sub-state is received when we are trying to
			 * disconnect with the old AP during roam.
			 * BUT, if receive a disconnect indication outside of
			 * Disassoc Handoff sub-state, then it means that
			 * this is a genuine disconnect and we need to clean up.
			 * Otherwise, we will be stuck in reassoc state which'll
			 * in-turn block scans.
			 */
			CSR_NEIGHBOR_ROAM_STATE_TRANSITION
				(pMac, eCSR_NEIGHBOR_ROAM_STATE_INIT,
				sessionId);
			pNeighborRoamInfo->roamChannelInfo.
				IAPPNeighborListReceived = false;
		}
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		csr_neighbor_roam_reset_init_state_control_info(pMac,
				sessionId);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
		CSR_NEIGHBOR_ROAM_STATE_TRANSITION(
				pMac, eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId)
		pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived =
				false;
		csr_neighbor_roam_reset_connected_state_control_info(pMac,
				sessionId);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE:
		/* Stop pre-auth to reassoc interval timer */
		cdf_mc_timer_stop(&pSession->ftSmeContext.
				preAuthReassocIntvlTimer);
	case eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING:
		CSR_NEIGHBOR_ROAM_STATE_TRANSITION(
				pMac, eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId)
		pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived =
				false;
		csr_neighbor_roam_reset_preauth_control_info(pMac, sessionId);
		csr_neighbor_roam_reset_report_scan_state_control_info(pMac,
				sessionId);
		break;
	default:
		NEIGHBOR_ROAM_DEBUG(pMac, LOGW, FL("Received disconnect event"
				"in state %s "),
				mac_trace_get_neighbour_roam_state(
					pNeighborRoamInfo->neighborRoamState));
		NEIGHBOR_ROAM_DEBUG(pMac, LOGW, FL("Transit to INIT state"));
		CSR_NEIGHBOR_ROAM_STATE_TRANSITION
			(pMac, eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId)
			pNeighborRoamInfo->roamChannelInfo.
			IAPPNeighborListReceived = false;
		break;
	}
	/*Inform the Firmware to STOP Scanning as the host has a disconnect. */
	if (csr_roam_is_sta_mode(pMac, sessionId)) {
		csr_roam_offload_scan(pMac, sessionId, ROAM_SCAN_OFFLOAD_STOP,
				REASON_DISCONNECTED);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * csr_neighbor_roam_info_ctx_init() - Initialize neighbor roam struct
 * @pMac: mac context
 * @session_id: Session Id
 *
 * This initializes all the necessary data structures related to the
 * associated AP.
 *
 * Return: CDF status
 */
static void csr_neighbor_roam_info_ctx_init(
		tpAniSirGlobal pMac,
		uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo ngbr_roam_info =
		&pMac->roam.neighborRoamInfo[session_id];
	tCsrRoamSession *session = &pMac->roam.roamSession[session_id];

	int init_ft_flag = false;

	/*
	 * Initialize the occupied list ONLY if we are
	 * transitioning from INIT state to CONNECTED state.
	 */
	if (eCSR_NEIGHBOR_ROAM_STATE_INIT ==
		ngbr_roam_info->neighborRoamState)
		csr_init_occupied_channels_list(pMac, session_id);
	CSR_NEIGHBOR_ROAM_STATE_TRANSITION
		(pMac, eCSR_NEIGHBOR_ROAM_STATE_CONNECTED, session_id);

	cdf_copy_macaddr(&ngbr_roam_info->currAPbssid,
			&session->connectedProfile.bssid);
	ngbr_roam_info->currAPoperationChannel =
		session->connectedProfile.operationChannel;
	ngbr_roam_info->currentNeighborLookupThreshold =
		ngbr_roam_info->cfgParams.neighborLookupThreshold;
	ngbr_roam_info->currentOpportunisticThresholdDiff =
		ngbr_roam_info->cfgParams.nOpportunisticThresholdDiff;
	ngbr_roam_info->currentRoamRescanRssiDiff =
		ngbr_roam_info->cfgParams.nRoamRescanRssiDiff;
	ngbr_roam_info->currentRoamBmissFirstBcnt =
		ngbr_roam_info->cfgParams.nRoamBmissFirstBcnt;
	ngbr_roam_info->currentRoamBmissFinalBcnt =
		ngbr_roam_info->cfgParams.nRoamBmissFinalBcnt;
	ngbr_roam_info->currentRoamBeaconRssiWeight =
		ngbr_roam_info->cfgParams.nRoamBeaconRssiWeight;

	/**
	 * Now we can clear the preauthDone that
	 * was saved as we are connected afresh */
	csr_neighbor_roam_free_roamable_bss_list(pMac,
		&ngbr_roam_info->FTRoamInfo.preAuthDoneList);

	/* Based on the auth scheme tell if we are 11r */
	if (csr_is_auth_type11r
		(session->connectedProfile.AuthType,
		session->connectedProfile.MDID.mdiePresent)) {
		if (pMac->roam.configParam.isFastTransitionEnabled)
			init_ft_flag = true;
		ngbr_roam_info->is11rAssoc = true;
	} else
		ngbr_roam_info->is11rAssoc = false;
	NEIGHBOR_ROAM_DEBUG(pMac, LOG2, FL("11rAssoc is = %d"),
		ngbr_roam_info->is11rAssoc);

#ifdef FEATURE_WLAN_ESE
	/* Based on the auth scheme tell if we are 11r */
	if (session->connectedProfile.isESEAssoc) {
		if (pMac->roam.configParam.isFastTransitionEnabled)
			init_ft_flag = true;
		ngbr_roam_info->isESEAssoc = true;
	} else
		ngbr_roam_info->isESEAssoc = false;
	NEIGHBOR_ROAM_DEBUG(pMac, LOG2,
		FL("isESEAssoc is = %d ft = %d"),
		ngbr_roam_info->isESEAssoc, init_ft_flag);
#endif
	/* If "Legacy Fast Roaming" is enabled */
	if (csr_roam_is_fast_roam_enabled(pMac, session_id))
		init_ft_flag = true;
	if (init_ft_flag == false)
		return;
	/* Initialize all the data structures needed for the 11r FT Preauth */
	ngbr_roam_info->FTRoamInfo.currentNeighborRptRetryNum = 0;
	csr_neighbor_roam_purge_preauth_failed_list(pMac);
	if (!cds_is_multiple_active_sta_sessions() &&
		csr_roam_is_roam_offload_scan_enabled(pMac)) {
		/*
		 * If this is not a INFRA type BSS, then do not send the command
		 * down to firmware.Do not send the START command for
		 * other session connections.*/
		if (!csr_roam_is_sta_mode(pMac, session_id)) {
			CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_DEBUG,
				"Wrong Mode %d",
				session->connectedProfile.BSSType);
			return;
		}
		ngbr_roam_info->uOsRequestedHandoff = 0;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (session->roam_synch_in_progress) {
			if (pMac->roam.pReassocResp != NULL) {
				CDF_TRACE(QDF_MODULE_ID_SME,
					CDF_TRACE_LEVEL_DEBUG,
					"Free Reassoc Rsp");
				cdf_mem_free(pMac->roam.pReassocResp);
				pMac->roam.pReassocResp = NULL;
			}
		} else
#endif
			csr_roam_offload_scan(pMac, session_id,
				ROAM_SCAN_OFFLOAD_START,
				REASON_CONNECT);
	}
}

/**
 * csr_neighbor_roam_indicate_connect()
 * @pMac: mac context
 * @session_id: Session Id
 * @qdf_status: CDF status
 *
 * This function is called by CSR as soon as the station connects to an AP.
 * This initializes all the necessary data structures related to the
 * associated AP and transitions the state to CONNECTED state
 *
 * Return: CDF status
 */
QDF_STATUS csr_neighbor_roam_indicate_connect(
		tpAniSirGlobal pMac, uint8_t session_id,
		QDF_STATUS qdf_status)
{
	tpCsrNeighborRoamControlInfo ngbr_roam_info =
		&pMac->roam.neighborRoamInfo[session_id];
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	tCsrRoamInfo roamInfo;
	tCsrRoamSession *session = &pMac->roam.roamSession[session_id];
	tpSirSetActiveModeSetBncFilterReq msg;
#endif
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* if session id invalid then we need return failure */
	if (NULL == ngbr_roam_info || !CSR_IS_SESSION_VALID(pMac, session_id)
	|| (NULL == pMac->roam.roamSession[session_id].pCurRoamProfile)) {
		return QDF_STATUS_E_FAILURE;
	}

	sms_log(pMac, LOG2,
		FL("Connect ind. received with session id %d in state %s"),
		session_id, mac_trace_get_neighbour_roam_state(
			ngbr_roam_info->neighborRoamState));

	/* Bail out if this is NOT a STA persona */
	if (pMac->roam.roamSession[session_id].pCurRoamProfile->csrPersona !=
	QDF_STA_MODE) {
		sms_log(pMac, LOGE,
			FL("Ignoring Connect ind received from a non STA."
			"session_id: %d, csrPersonna %d"), session_id,
			(int)session->pCurRoamProfile->csrPersona);
		return QDF_STATUS_SUCCESS;
	}
	/* if a concurrent session is running */
	if ((false ==
		CSR_IS_FASTROAM_IN_CONCURRENCY_INI_FEATURE_ENABLED(pMac)) &&
		(csr_is_concurrent_session_running(pMac))) {
		sms_log(pMac, LOGE,
			FL("Ignoring Connect ind. received in multisession %d"),
			csr_is_concurrent_session_running(pMac));
		return QDF_STATUS_SUCCESS;
	}
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	if (session->roam_synch_in_progress &&
		(eSIR_ROAM_AUTH_STATUS_AUTHENTICATED ==
		session->roam_synch_data->authStatus)) {
		CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_DEBUG,
			"LFR3:csr_neighbor_roam_indicate_connect");
		msg = cdf_mem_malloc(sizeof(tSirSetActiveModeSetBncFilterReq));
		if (msg == NULL) {
			CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_ERROR,
				"LFR3:Mem Alloc failed for beacon Filter Req");
			return QDF_STATUS_E_NOMEM;
		}
		msg->messageType = eWNI_SME_SET_BCN_FILTER_REQ;
		msg->length = sizeof(uint8_t);
		msg->seesionId = session_id;
		status = cds_send_mb_message_to_mac(msg);
		cdf_copy_macaddr(&roamInfo.peerMac,
			&session->connectedProfile.bssid);
		roamInfo.roamSynchInProgress =
			session->roam_synch_in_progress;
		csr_roam_call_callback(pMac, session_id, &roamInfo, 0,
			eCSR_ROAM_SET_KEY_COMPLETE,
			eCSR_ROAM_RESULT_AUTHENTICATED);
		csr_neighbor_roam_reset_init_state_control_info(pMac,
			session_id);
		csr_neighbor_roam_info_ctx_init(pMac, session_id);

		return status;
	}
#endif

	switch (ngbr_roam_info->neighborRoamState) {
	case eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING:
		if (QDF_STATUS_SUCCESS != qdf_status) {
			/**
			 * Just transition the state to INIT state.Rest of the
			 * clean up happens when we get next connect indication
			 */
			CSR_NEIGHBOR_ROAM_STATE_TRANSITION(
				pMac, eCSR_NEIGHBOR_ROAM_STATE_INIT,
				session_id)
			ngbr_roam_info->roamChannelInfo.IAPPNeighborListReceived =
				false;
			break;
		}
	/* Fall through if the status is SUCCESS */
	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		/* Reset all the data structures here */
		csr_neighbor_roam_reset_init_state_control_info(pMac,
			session_id);
		csr_neighbor_roam_info_ctx_init(pMac, session_id);
		break;
	default:
		sms_log(pMac, LOGE,
			FL("Connect evt received in invalid state %s Ignoring"),
			mac_trace_get_neighbour_roam_state(
			ngbr_roam_info->neighborRoamState));
		break;
	}
	return status;
}

/* ---------------------------------------------------------------------------

    \fn csr_neighbor_roam_purge_preauth_failed_list

    \brief  This function purges all the MAC addresses in the pre-auth fail list

    \param  pMac - The handle returned by mac_open.

    \return VOID

   ---------------------------------------------------------------------------*/
void csr_neighbor_roam_purge_preauth_failed_list(tpAniSirGlobal pMac)
{
	uint8_t i, j;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;

	for (j = 0; j < CSR_ROAM_SESSION_MAX; j++) {
		pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[j];
		for (i = 0;
		     i <
		     pNeighborRoamInfo->FTRoamInfo.preAuthFailList.
		     numMACAddress; i++) {
			cdf_mem_zero(pNeighborRoamInfo->FTRoamInfo.
				     preAuthFailList.macAddress[i],
				     sizeof(tSirMacAddr));
		}
		pNeighborRoamInfo->FTRoamInfo.preAuthFailList.numMACAddress = 0;
	}
}

/* ---------------------------------------------------------------------------

    \fn csr_neighbor_roam_init11r_assoc_info

    \brief  This function initializes 11r related neighbor roam data structures

    \param  pMac - The handle returned by mac_open.

    \return QDF_STATUS_SUCCESS on success, corresponding error code otherwise

   ---------------------------------------------------------------------------*/
QDF_STATUS csr_neighbor_roam_init11r_assoc_info(tpAniSirGlobal pMac)
{
	QDF_STATUS status;
	uint8_t i;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo = NULL;
	tpCsr11rAssocNeighborInfo pFTRoamInfo = NULL;

	for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
		pNeighborRoamInfo = &pMac->roam.neighborRoamInfo[i];
		pFTRoamInfo = &pNeighborRoamInfo->FTRoamInfo;

		pNeighborRoamInfo->is11rAssoc = false;
		pNeighborRoamInfo->cfgParams.maxNeighborRetries =
			pMac->roam.configParam.neighborRoamConfig.
			nMaxNeighborRetries;

		pFTRoamInfo->neighborReportTimeout =
			CSR_NEIGHBOR_ROAM_REPORT_QUERY_TIMEOUT;
		pFTRoamInfo->PEPreauthRespTimeout =
			CSR_NEIGHBOR_ROAM_PREAUTH_RSP_WAIT_MULTIPLIER *
			pNeighborRoamInfo->cfgParams.neighborScanPeriod;
		pFTRoamInfo->neighborRptPending = false;
		pFTRoamInfo->preauthRspPending = false;

		pFTRoamInfo->currentNeighborRptRetryNum = 0;
		pFTRoamInfo->numBssFromNeighborReport = 0;

		cdf_mem_zero(pFTRoamInfo->neighboReportBssInfo,
			     sizeof(tCsrNeighborReportBssInfo) *
			     MAX_BSS_IN_NEIGHBOR_RPT);

		status = csr_ll_open(pMac->hHdd, &pFTRoamInfo->preAuthDoneList);
		if (QDF_STATUS_SUCCESS != status) {
			sms_log(pMac, LOGE,
				FL("LL Open of preauth done AP List failed"));
			return QDF_STATUS_E_RESOURCES;
		}
	}
	return status;
}

/* ---------------------------------------------------------------------------

    \fn csr_neighbor_roam_init

    \brief  This function initializes neighbor roam data structures

    \param  pMac - The handle returned by mac_open.

    \return QDF_STATUS_SUCCESS on success, corresponding error code otherwise

   ---------------------------------------------------------------------------*/
QDF_STATUS csr_neighbor_roam_init(tpAniSirGlobal pMac, uint8_t sessionId)
{
	QDF_STATUS status;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];

	pNeighborRoamInfo->neighborRoamState = eCSR_NEIGHBOR_ROAM_STATE_CLOSED;
	pNeighborRoamInfo->prevNeighborRoamState =
		eCSR_NEIGHBOR_ROAM_STATE_CLOSED;
	pNeighborRoamInfo->cfgParams.maxChannelScanTime =
		pMac->roam.configParam.neighborRoamConfig.nNeighborScanMaxChanTime;
	pNeighborRoamInfo->cfgParams.minChannelScanTime =
		pMac->roam.configParam.neighborRoamConfig.nNeighborScanMinChanTime;
	pNeighborRoamInfo->cfgParams.maxNeighborRetries = 0;
	pNeighborRoamInfo->cfgParams.neighborLookupThreshold =
		pMac->roam.configParam.neighborRoamConfig.
		nNeighborLookupRssiThreshold;
	pNeighborRoamInfo->cfgParams.delay_before_vdev_stop =
		pMac->roam.configParam.neighborRoamConfig.
		delay_before_vdev_stop;
	pNeighborRoamInfo->cfgParams.nOpportunisticThresholdDiff =
		pMac->roam.configParam.neighborRoamConfig.
		nOpportunisticThresholdDiff;
	pNeighborRoamInfo->cfgParams.nRoamRescanRssiDiff =
		pMac->roam.configParam.neighborRoamConfig.nRoamRescanRssiDiff;
	pNeighborRoamInfo->cfgParams.nRoamBmissFirstBcnt =
		pMac->roam.configParam.neighborRoamConfig.nRoamBmissFirstBcnt;
	pNeighborRoamInfo->cfgParams.nRoamBmissFinalBcnt =
		pMac->roam.configParam.neighborRoamConfig.nRoamBmissFinalBcnt;
	pNeighborRoamInfo->cfgParams.nRoamBeaconRssiWeight =
		pMac->roam.configParam.neighborRoamConfig.nRoamBeaconRssiWeight;
	pNeighborRoamInfo->cfgParams.neighborScanPeriod =
		pMac->roam.configParam.neighborRoamConfig.nNeighborScanTimerPeriod;
	pNeighborRoamInfo->cfgParams.neighborResultsRefreshPeriod =
		pMac->roam.configParam.neighborRoamConfig.
		nNeighborResultsRefreshPeriod;
	pNeighborRoamInfo->cfgParams.emptyScanRefreshPeriod =
		pMac->roam.configParam.neighborRoamConfig.nEmptyScanRefreshPeriod;

	pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels =
		pMac->roam.configParam.neighborRoamConfig.neighborScanChanList.
		numChannels;
	if (pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels != 0) {
		pNeighborRoamInfo->cfgParams.channelInfo.ChannelList =
		cdf_mem_malloc(pMac->roam.configParam.neighborRoamConfig.
				neighborScanChanList.numChannels);
		if (NULL ==
			pNeighborRoamInfo->cfgParams.channelInfo.ChannelList) {
			sms_log(pMac, LOGE,
			FL("Memory Allocation for CFG Channel List failed"));
			return QDF_STATUS_E_NOMEM;
		}
	} else {
		pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;
		sms_log(pMac, LOGE,
			FL("invalid neighbor roam channel list: %u"),
			pNeighborRoamInfo->cfgParams.channelInfo.numOfChannels);
	}

	/* Update the roam global structure from CFG */
	cdf_mem_copy(pNeighborRoamInfo->cfgParams.channelInfo.ChannelList,
		     pMac->roam.configParam.neighborRoamConfig.
		     neighborScanChanList.channelList,
		     pMac->roam.configParam.neighborRoamConfig.
		     neighborScanChanList.numChannels);
	pNeighborRoamInfo->cfgParams.hi_rssi_scan_max_count =
		pMac->roam.configParam.neighborRoamConfig.
			nhi_rssi_scan_max_count;
	pNeighborRoamInfo->cfgParams.hi_rssi_scan_rssi_delta =
		pMac->roam.configParam.neighborRoamConfig.
			nhi_rssi_scan_rssi_delta;
	pNeighborRoamInfo->cfgParams.hi_rssi_scan_delay =
		pMac->roam.configParam.neighborRoamConfig.
			nhi_rssi_scan_delay;
	pNeighborRoamInfo->cfgParams.hi_rssi_scan_rssi_ub =
		pMac->roam.configParam.neighborRoamConfig.
			nhi_rssi_scan_rssi_ub;

	cdf_zero_macaddr(&pNeighborRoamInfo->currAPbssid);
	pNeighborRoamInfo->currentNeighborLookupThreshold =
		pNeighborRoamInfo->cfgParams.neighborLookupThreshold;
	pNeighborRoamInfo->currentOpportunisticThresholdDiff =
		pNeighborRoamInfo->cfgParams.nOpportunisticThresholdDiff;
	pNeighborRoamInfo->currentRoamRescanRssiDiff =
		pNeighborRoamInfo->cfgParams.nRoamRescanRssiDiff;
	pNeighborRoamInfo->currentRoamBmissFirstBcnt =
		pNeighborRoamInfo->cfgParams.nRoamBmissFirstBcnt;
	pNeighborRoamInfo->currentRoamBmissFinalBcnt =
		pNeighborRoamInfo->cfgParams.nRoamBmissFinalBcnt;
	pNeighborRoamInfo->currentRoamBeaconRssiWeight =
		pNeighborRoamInfo->cfgParams.nRoamBeaconRssiWeight;
	cdf_mem_set(&pNeighborRoamInfo->prevConnProfile,
		    sizeof(tCsrRoamConnectedProfile), 0);

	status = csr_ll_open(pMac->hHdd, &pNeighborRoamInfo->roamableAPList);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(pMac, LOGE, FL("LL Open of roam able AP List failed"));
		cdf_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.
			     ChannelList);
		pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;
		return QDF_STATUS_E_RESOURCES;
	}

	pNeighborRoamInfo->roamChannelInfo.currentChanIndex =
		CSR_NEIGHBOR_ROAM_INVALID_CHANNEL_INDEX;
	pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
	numOfChannels = 0;
	pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList =
		NULL;
	pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived = false;

	status = csr_neighbor_roam_init11r_assoc_info(pMac);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(pMac, LOGE, FL("LL Open of roam able AP List failed"));
		cdf_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.
			     ChannelList);
		pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;
		csr_ll_close(&pNeighborRoamInfo->roamableAPList);
		return QDF_STATUS_E_RESOURCES;
	}

	CSR_NEIGHBOR_ROAM_STATE_TRANSITION(pMac, eCSR_NEIGHBOR_ROAM_STATE_INIT,
					   sessionId)
	pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived = false;
	/* Set the Last Sent Cmd as RSO_STOP */
	pNeighborRoamInfo->last_sent_cmd = ROAM_SCAN_OFFLOAD_STOP;
	return QDF_STATUS_SUCCESS;
}

/* ---------------------------------------------------------------------------

    \fn csr_neighbor_roam_close

    \brief  This function closes/frees all the neighbor roam data structures

    \param  pMac - The handle returned by mac_open.
    \param  sessionId - Session identifier
    \return VOID

   ---------------------------------------------------------------------------*/
void csr_neighbor_roam_close(tpAniSirGlobal pMac, uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];

	if (eCSR_NEIGHBOR_ROAM_STATE_CLOSED ==
	    pNeighborRoamInfo->neighborRoamState) {
		sms_log(pMac, LOGW,
			FL("Neighbor Roam Algorithm Already Closed"));
		return;
	}

	if (pNeighborRoamInfo->cfgParams.channelInfo.ChannelList)
		cdf_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.
			     ChannelList);

	pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;

	/* Should free up the nodes in the list before closing the double Linked list */
	csr_neighbor_roam_free_roamable_bss_list(pMac,
						 &pNeighborRoamInfo->roamableAPList);
	csr_ll_close(&pNeighborRoamInfo->roamableAPList);

	if (pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
	    ChannelList) {
		cdf_mem_free(pNeighborRoamInfo->roamChannelInfo.
			     currentChannelListInfo.ChannelList);
	}

	pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList =
		NULL;
	pNeighborRoamInfo->roamChannelInfo.currentChanIndex =
		CSR_NEIGHBOR_ROAM_INVALID_CHANNEL_INDEX;
	pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
	numOfChannels = 0;
	pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList =
		NULL;
	pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived = false;

	/* Free the profile.. */
	csr_release_profile(pMac, &pNeighborRoamInfo->csrNeighborRoamProfile);
	csr_roam_free_connect_profile(&pNeighborRoamInfo->prevConnProfile);
	pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum = 0;
	pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport = 0;
	cdf_mem_zero(pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo,
		     sizeof(tCsrNeighborReportBssInfo) *
		     MAX_BSS_IN_NEIGHBOR_RPT);
	csr_neighbor_roam_free_roamable_bss_list(pMac,
						 &pNeighborRoamInfo->FTRoamInfo.
						 preAuthDoneList);
	csr_ll_close(&pNeighborRoamInfo->FTRoamInfo.preAuthDoneList);

	CSR_NEIGHBOR_ROAM_STATE_TRANSITION(pMac,
		eCSR_NEIGHBOR_ROAM_STATE_CLOSED, sessionId)

	return;
}

/**
 * csr_neighbor_roam_request_handoff() - Handoff to a different AP
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: Session ID
 *
 * This function triggers actual switching from one AP to the new AP.
 * It issues disassociate with reason code as Handoff and CSR as a part of
 * handling disassoc rsp, issues reassociate to the new AP
 *
 * Return: none
 */
void csr_neighbor_roam_request_handoff(tpAniSirGlobal mac_ctx,
		uint8_t session_id)
{
	tCsrRoamInfo roam_info;
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	tCsrNeighborRoamBSSInfo handoff_node;
	uint32_t roamid = 0;
	QDF_STATUS status;

	CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_DEBUG, "%s session_id=%d",
		  __func__, session_id);

	if (neighbor_roam_info->neighborRoamState !=
		eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE) {
		sms_log(mac_ctx, LOGE,
			FL("Roam requested when Neighbor roam is in %s state"),
			mac_trace_get_neighbour_roam_state(
			neighbor_roam_info->neighborRoamState));
		return;
	}

	if (false == csr_neighbor_roam_get_handoff_ap_info(mac_ctx,
			&handoff_node, session_id)) {
		CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_ERROR,
		FL("failed to obtain handoff AP"));
		return;
	}
	CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_DEBUG,
		  FL("HANDOFF CANDIDATE BSSID "MAC_ADDRESS_STR),
		  MAC_ADDR_ARRAY(handoff_node.pBssDescription->bssId));

	cdf_mem_zero(&roam_info, sizeof(tCsrRoamInfo));
	csr_roam_call_callback(mac_ctx, session_id, &roam_info, roamid,
			       eCSR_ROAM_FT_START, eSIR_SME_SUCCESS);

	cdf_mem_zero(&roam_info, sizeof(tCsrRoamInfo));
	CSR_NEIGHBOR_ROAM_STATE_TRANSITION
		(mac_ctx, eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING, session_id)

	csr_neighbor_roam_send_lfr_metric_event(mac_ctx, session_id,
		handoff_node.pBssDescription->bssId,
		eCSR_ROAM_HANDOVER_SUCCESS);
	/* Free the profile.. Just to make sure we dont leak memory here */
	csr_release_profile(mac_ctx,
		&neighbor_roam_info->csrNeighborRoamProfile);
	/*
	 * Create the Handoff AP profile. Copy the currently connected profile
	 * and update only the BSSID and channel number. This should happen
	 * before issuing disconnect
	 */
	status = csr_roam_copy_connected_profile(mac_ctx, session_id,
			&neighbor_roam_info->csrNeighborRoamProfile);
	if (QDF_STATUS_SUCCESS != status) {
		CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_ERROR,
			FL("csr_roam_copy_connected_profile failed %d"),
			status);
		return;
	}
	cdf_mem_copy(neighbor_roam_info->csrNeighborRoamProfile.BSSIDs.bssid,
		     handoff_node.pBssDescription->bssId, sizeof(tSirMacAddr));
	neighbor_roam_info->csrNeighborRoamProfile.ChannelInfo.ChannelList[0] =
		handoff_node.pBssDescription->channelId;

	NEIGHBOR_ROAM_DEBUG(mac_ctx, LOGW,
		" csr_roamHandoffRequested: disassociating with current AP");

	if (!QDF_IS_STATUS_SUCCESS
		    (csr_roam_issue_disassociate_cmd
			    (mac_ctx, session_id,
			    eCSR_DISCONNECT_REASON_HANDOFF))) {
		sms_log(mac_ctx, LOGW,
			"csr_roamHandoffRequested:  fail to issue disassociate");
		return;
	}
	/* notify HDD for handoff, providing the BSSID too */
	roam_info.reasonCode = eCsrRoamReasonBetterAP;

	cdf_mem_copy(roam_info.bssid.bytes,
		     handoff_node.pBssDescription->bssId,
		     sizeof(struct qdf_mac_addr));

	csr_roam_call_callback(mac_ctx, session_id, &roam_info, 0,
			       eCSR_ROAM_ROAMING_START, eCSR_ROAM_RESULT_NONE);

	return;
}

/**
 * csr_neighbor_roam_is_handoff_in_progress()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: Session ID
 *
 * This function returns whether handoff is in progress or not based on
 * the current neighbor roam state
 *
 * Return: true if reassoc in progress, false otherwise
 */
bool csr_neighbor_roam_is_handoff_in_progress(tpAniSirGlobal pMac, uint8_t sessionId)
{
	if (eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING ==
	    pMac->roam.neighborRoamInfo[sessionId].neighborRoamState)
		return true;

	return false;
}
/**
 * csr_neighbor_roam_is11r_assoc() - Check if association type is 11R
 * @mac_ctx: MAC Global context
 * @session_id: Session ID
 *
 * Return: true if 11r Association, false otherwise.
 */
bool csr_neighbor_roam_is11r_assoc(tpAniSirGlobal mac_ctx, uint8_t session_id)
{
	return mac_ctx->roam.neighborRoamInfo[session_id].is11rAssoc;
}

/**
 * csr_neighbor_roam_get_handoff_ap_info - Identifies the best AP for roaming
 *
 * @pMac:        mac context
 * @session_id:     Session Id
 * @hand_off_node:    AP node that is the handoff candidate returned
 *
 * This function returns the best possible AP for handoff. For 11R case, it
 * returns the 1st entry from pre-auth done list. For non-11r case, it returns
 * the 1st entry from roamable AP list
 *
 * Return: true if able find handoff AP, false otherwise
 */

bool csr_neighbor_roam_get_handoff_ap_info(tpAniSirGlobal pMac,
			tpCsrNeighborRoamBSSInfo hand_off_node,
			uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo ngbr_roam_info =
		&pMac->roam.neighborRoamInfo[session_id];
	tpCsrNeighborRoamBSSInfo bss_node = NULL;

	if (NULL == hand_off_node) {
		CDF_ASSERT(NULL != hand_off_node);
		return false;
	}
	if (ngbr_roam_info->is11rAssoc) {
		/* Always the BSS info in the head is the handoff candidate */
		bss_node = csr_neighbor_roam_next_roamable_ap(
			pMac,
			&ngbr_roam_info->FTRoamInfo.preAuthDoneList,
			NULL);
		NEIGHBOR_ROAM_DEBUG(pMac, LOG1,
			FL("Number of Handoff candidates = %d"),
			csr_ll_count(&
				ngbr_roam_info->FTRoamInfo.preAuthDoneList));
	} else
#ifdef FEATURE_WLAN_ESE
	if (ngbr_roam_info->isESEAssoc) {
		/* Always the BSS info in the head is the handoff candidate */
		bss_node =
			csr_neighbor_roam_next_roamable_ap(pMac,
				&ngbr_roam_info->FTRoamInfo.preAuthDoneList,
				NULL);
		NEIGHBOR_ROAM_DEBUG(pMac, LOG1,
			FL("Number of Handoff candidates = %d"),
			csr_ll_count(&ngbr_roam_info->FTRoamInfo.
			preAuthDoneList));
	} else
#endif
	if (csr_roam_is_fast_roam_enabled(pMac, session_id)) {
		/* Always the BSS info in the head is the handoff candidate */
		bss_node =
			csr_neighbor_roam_next_roamable_ap(pMac,
			&ngbr_roam_info->FTRoamInfo.preAuthDoneList,
			NULL);
		NEIGHBOR_ROAM_DEBUG(pMac, LOG1,
			FL("Number of Handoff candidates = %d"),
			csr_ll_count(
				&ngbr_roam_info->FTRoamInfo.preAuthDoneList));
	} else {
		bss_node =
			csr_neighbor_roam_next_roamable_ap(pMac,
				&ngbr_roam_info->roamableAPList,
				NULL);
		NEIGHBOR_ROAM_DEBUG(pMac, LOG1,
			FL("Number of Handoff candidates = %d"),
			csr_ll_count(&ngbr_roam_info->roamableAPList));
	}
	if (NULL == bss_node)
		return false;
	cdf_mem_copy(hand_off_node, bss_node, sizeof(tCsrNeighborRoamBSSInfo));
	return true;
}

/* ---------------------------------------------------------------------------
    \brief  This function returns true if preauth is completed

    \param  pMac - The handle returned by mac_open.

    \return bool

   ---------------------------------------------------------------------------*/
bool csr_neighbor_roam_state_preauth_done(tpAniSirGlobal pMac, uint8_t sessionId)
{
	return pMac->roam.neighborRoamInfo[sessionId].neighborRoamState ==
		eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE;
}

/* ---------------------------------------------------------------------------
    \brief  In the event that we are associated with AP1 and we have
    completed pre auth with AP2. Then we receive a deauth/disassoc from
    AP1.
    At this point neighbor roam is in pre auth done state, pre auth timer
    is running. We now handle this case by stopping timer and clearing
    the pre-auth state. We basically clear up and just go to disconnected
    state.

    \param  pMac - The handle returned by mac_open.

    \return bool
   ---------------------------------------------------------------------------*/
void csr_neighbor_roam_tranistion_preauth_done_to_disconnected(tpAniSirGlobal pMac,
							       uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	tCsrRoamSession *session = CSR_GET_SESSION(pMac, sessionId);

	if (!session) {
		CDF_TRACE(QDF_MODULE_ID_SME, CDF_TRACE_LEVEL_DEBUG,
			  FL("session is NULL"));
		return;
	}

	if (pNeighborRoamInfo->neighborRoamState !=
	    eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE)
		return;

	/* Stop timer */
	cdf_mc_timer_stop(&session->ftSmeContext.preAuthReassocIntvlTimer);

	/* Transition to init state */
	CSR_NEIGHBOR_ROAM_STATE_TRANSITION(pMac, eCSR_NEIGHBOR_ROAM_STATE_INIT,
					   sessionId)
	pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived = false;
}

/* ---------------------------------------------------------------------------
    \brief  This function returns true if STA is in the middle of roaming states

    \param  halHandle - The handle from HDD context.
    \param  sessionId - Session identifier

    \return bool

   ---------------------------------------------------------------------------*/
bool csr_neighbor_middle_of_roaming(tpAniSirGlobal pMac, uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	bool val = (eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING ==
		    pNeighborRoamInfo->neighborRoamState) ||
		   (eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING ==
		    pNeighborRoamInfo->neighborRoamState) ||
		   (eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE ==
		    pNeighborRoamInfo->neighborRoamState);
	return val;
}

/**
 * csr_neighbor_roam_candidate_found_ind_hdlr()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @msg_buf: pointer to msg buff
 *
 * This function is called by CSR as soon as TL posts the candidate
 * found indication to SME via MC thread
 *
 * Return: QDF_STATUS_SUCCESS on success, corresponding error code otherwise
 */
QDF_STATUS csr_neighbor_roam_candidate_found_ind_hdlr(tpAniSirGlobal pMac, void *pMsg)
{
	tSirSmeCandidateFoundInd *pSirSmeCandidateFoundInd =
		(tSirSmeCandidateFoundInd *) pMsg;
	uint32_t sessionId = pSirSmeCandidateFoundInd->sessionId;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	sms_log(pMac, LOG1, FL("Received indication from firmware"));

	/* we must be in connected state, if not ignore it */
	if ((eCSR_NEIGHBOR_ROAM_STATE_CONNECTED !=
	     pNeighborRoamInfo->neighborRoamState)
	    || (pNeighborRoamInfo->uOsRequestedHandoff)) {
		sms_log(pMac, LOGE,
			FL
				("Received in not CONNECTED state OR uOsRequestedHandoff is set. Ignore it"));
		status = QDF_STATUS_E_FAILURE;
	} else {
		/* Firmware indicated that roaming candidate is found. Beacons
		 * are already in the SME scan results table.
		 * Process the results for choosing best roaming candidate.
		 */
		csr_save_scan_results(pMac, eCsrScanCandidateFound,
				      sessionId);
		/* Future enhancements:
		 * If firmware tags candidate beacons, give them preference
		 * for roaming.
		 * Age out older entries so that new candidate beacons
		 * will get preference.
		 */
		status = csr_neighbor_roam_process_scan_complete(pMac,
								 sessionId);
		if (QDF_STATUS_SUCCESS != status) {
			sms_log(pMac, LOGE,
				FL("Neighbor scan process complete failed with status %d"),
				status);
			return QDF_STATUS_E_FAILURE;
		}
	}

	return status;
}

/**
 * csr_neighbor_roam_process_handoff_req - Processes handoff request
 *
 * @mac_ctx  Pointer to mac context
 * @session_id  SME session id
 *
 * This function is called start with the handoff process. First do a
 * SSID scan for the BSSID provided.
 *
 * Return: status
 */
QDF_STATUS csr_neighbor_roam_process_handoff_req(
			tpAniSirGlobal mac_ctx,
			uint8_t session_id)
{
	tpCsrNeighborRoamControlInfo roam_ctrl_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t roam_id;
	tCsrRoamProfile *profile = NULL;
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);
	uint8_t i = 0;

	if (NULL == session) {
		sms_log(mac_ctx, LOGE, FL("session is NULL "));
		return QDF_STATUS_E_FAILURE;
	}

	roam_id = GET_NEXT_ROAM_ID(&mac_ctx->roam);
	profile = cdf_mem_malloc(sizeof(tCsrRoamProfile));
	if (NULL == profile) {
		sms_log(mac_ctx, LOGE, FL("Memory alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}
	cdf_mem_set(profile, sizeof(tCsrRoamProfile), 0);
	status =
		csr_roam_copy_profile(mac_ctx, profile,
				      session->pCurRoamProfile);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOGE, FL("Profile copy failed"));
		goto end;
	}
	/* Add the BSSID & Channel */
	profile->BSSIDs.numOfBSSIDs = 1;
	if (NULL == profile->BSSIDs.bssid) {
		profile->BSSIDs.bssid =
			cdf_mem_malloc(sizeof(tSirMacAddr) *
				profile->BSSIDs.numOfBSSIDs);
		if (NULL == profile->BSSIDs.bssid) {
			sms_log(mac_ctx, LOGE, FL("mem alloc failed for BSSID"));
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
	}

	cdf_mem_zero(profile->BSSIDs.bssid,
		sizeof(tSirMacAddr) *
		profile->BSSIDs.numOfBSSIDs);

	/* Populate the BSSID from handoff info received from HDD */
	for (i = 0; i < profile->BSSIDs.numOfBSSIDs; i++) {
		cdf_copy_macaddr(&profile->BSSIDs.bssid[i],
				&roam_ctrl_info->handoffReqInfo.bssid);
	}

	profile->ChannelInfo.numOfChannels = 1;
	if (NULL == profile->ChannelInfo.ChannelList) {
		profile->ChannelInfo.ChannelList =
			cdf_mem_malloc(sizeof(*profile->
				ChannelInfo.ChannelList) *
				profile->ChannelInfo.numOfChannels);
		if (NULL == profile->ChannelInfo.ChannelList) {
			sms_log(mac_ctx, LOGE,
				FL("mem alloc failed for ChannelList"));
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
	}
	profile->ChannelInfo.ChannelList[0] =
		roam_ctrl_info->handoffReqInfo.channel;

	/* do a SSID scan */
	status =
		csr_scan_for_ssid(mac_ctx, session_id, profile, roam_id, false);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(mac_ctx, LOGE, FL("SSID scan failed"));
	}

end:
	if (NULL != profile) {
		csr_release_profile(mac_ctx, profile);
		cdf_mem_free(profile);
	}

	return status;
}

/* ---------------------------------------------------------------------------

    \fn csr_neighbor_roam_sssid_scan_done

    \brief  This function is called once SSID scan is done. If SSID scan failed
    to find our candidate add an entry to csr scan cache ourself before starting
    the handoff process

    \param  pMac - The handle returned by mac_open.

    \return QDF_STATUS_SUCCESS on success, corresponding error code otherwise

   ---------------------------------------------------------------------------*/
QDF_STATUS csr_neighbor_roam_sssid_scan_done(tpAniSirGlobal pMac,
					     uint8_t sessionId, QDF_STATUS status)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	QDF_STATUS hstatus;

	sms_log(pMac, LOGE, FL("called "));

	/* we must be in connected state, if not ignore it */
	if (eCSR_NEIGHBOR_ROAM_STATE_CONNECTED !=
	    pNeighborRoamInfo->neighborRoamState) {
		sms_log(pMac, LOGE,
			FL("Received in not CONNECTED state. Ignore it"));
		return QDF_STATUS_E_FAILURE;
	}
	/* if SSID scan failed to find our candidate add an entry to csr scan cache ourself */
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sms_log(pMac, LOGE, FL("Add an entry to csr scan cache"));
		hstatus = csr_scan_create_entry_in_scan_cache(pMac, sessionId,
							      pNeighborRoamInfo->
							      handoffReqInfo.bssid,
							      pNeighborRoamInfo->
							      handoffReqInfo.channel);
		if (QDF_STATUS_SUCCESS != hstatus) {
			sms_log(pMac, LOGE,
				FL
					("csr_scan_create_entry_in_scan_cache failed with status %d"),
				hstatus);
			return QDF_STATUS_E_FAILURE;
		}
	}

	/* Now we have completed scanning for the candidate provided by HDD. Let move on to HO */
	hstatus = csr_neighbor_roam_process_scan_complete(pMac, sessionId);

	if (QDF_STATUS_SUCCESS != hstatus) {
		sms_log(pMac, LOGE,
			FL
				("Neighbor scan process complete failed with status %d"),
			hstatus);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}


/**
 * csr_neighbor_roam_handoff_req_hdlr - Processes handoff request
 * @mac_ctx  Pointer to mac context
 * @msg  message sent to HDD
 *
 * This function is called by CSR as soon as it gets a handoff request
 * to SME via MC thread
 *
 * Return: status
 */
QDF_STATUS csr_neighbor_roam_handoff_req_hdlr(
			tpAniSirGlobal mac_ctx, void *msg)
{
	tAniHandoffReq *handoff_req = (tAniHandoffReq *) msg;
	uint32_t session_id = handoff_req->sessionId;
	tpCsrNeighborRoamControlInfo roam_ctrl_info =
		&mac_ctx->roam.neighborRoamInfo[session_id];
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	/* we must be in connected state, if not ignore it */
	if (eCSR_NEIGHBOR_ROAM_STATE_CONNECTED !=
		roam_ctrl_info->neighborRoamState) {
		sms_log(mac_ctx, LOGE,
			FL("Received in not CONNECTED state. Ignore it"));
		return QDF_STATUS_E_FAILURE;
	}

	/* save the handoff info came from HDD as part of the reassoc req */
	handoff_req = (tAniHandoffReq *) msg;
	if (NULL == handoff_req) {
		sms_log(mac_ctx, LOGE, FL("Received msg is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	/* sanity check */
	if (true == cdf_mem_compare(handoff_req->bssid,
		roam_ctrl_info->currAPbssid.bytes,
		sizeof(tSirMacAddr))) {
		sms_log(mac_ctx, LOGE,
			FL
			("Received req has same BSSID as current AP!!"));
		return QDF_STATUS_E_FAILURE;
	}
	roam_ctrl_info->handoffReqInfo.channel =
		handoff_req->channel;
	roam_ctrl_info->handoffReqInfo.src =
		handoff_req->handoff_src;
	cdf_mem_copy(&roam_ctrl_info->handoffReqInfo.bssid.bytes,
			&handoff_req->bssid, QDF_MAC_ADDR_SIZE);
	roam_ctrl_info->uOsRequestedHandoff = 1;
	status = csr_roam_offload_scan(mac_ctx, session_id,
		ROAM_SCAN_OFFLOAD_STOP,
		REASON_OS_REQUESTED_ROAMING_NOW);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac_ctx, LOGE,
			FL("csr_roam_offload_scan failed"));
		roam_ctrl_info->uOsRequestedHandoff = 0;
	}
	return status;
}

/**
 * csr_neighbor_roam_proceed_with_handoff_req()
 *
 * @mac_ctx: Pointer to Global MAC structure
 * @session_id: Session ID
 *
 * This function is called by CSR as soon as it gets rsp back for
 * ROAM_SCAN_OFFLOAD_STOP with reason REASON_OS_REQUESTED_ROAMING_NOW
 *
 * Return: QDF_STATUS_SUCCESS on success, corresponding error code otherwise
 */
QDF_STATUS csr_neighbor_roam_proceed_with_handoff_req(tpAniSirGlobal pMac,
						      uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	/* we must be in connected state, if not ignore it */
	if ((eCSR_NEIGHBOR_ROAM_STATE_CONNECTED !=
	     pNeighborRoamInfo->neighborRoamState)
	    || (!pNeighborRoamInfo->uOsRequestedHandoff)) {
		sms_log(pMac, LOGE,
			FL
				("Received in not CONNECTED state or uOsRequestedHandoff is not set. Ignore it"));
		status = QDF_STATUS_E_FAILURE;
	} else {
		/* Let's go ahead with handoff */
		status = csr_neighbor_roam_process_handoff_req(pMac, sessionId);
	}
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		pNeighborRoamInfo->uOsRequestedHandoff = 0;
	}
	return status;
}

/* ---------------------------------------------------------------------------

    \fn csr_neighbor_roam_start_lfr_scan

    \brief  This function is called if HDD requested handoff failed for some
    reason. start the LFR logic at that point.By the time, this function is
    called, a STOP command has already been issued.

    \param  pMac - The handle returned by mac_open.

    \return QDF_STATUS_SUCCESS on success, corresponding error code otherwise

   ---------------------------------------------------------------------------*/
QDF_STATUS csr_neighbor_roam_start_lfr_scan(tpAniSirGlobal pMac, uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];
	pNeighborRoamInfo->uOsRequestedHandoff = 0;
	/* There is no candidate or We are not roaming Now.
	 * Inform the FW to restart Roam Offload Scan  */
	csr_roam_offload_scan(pMac, sessionId, ROAM_SCAN_OFFLOAD_START,
			      REASON_NO_CAND_FOUND_OR_NOT_ROAMING_NOW);

	return QDF_STATUS_SUCCESS;
}
