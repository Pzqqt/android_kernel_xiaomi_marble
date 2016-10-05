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
static const char *lfr_get_config_item_string(uint8_t reason)
{
	switch (reason) {
	CASE_RETURN_STRING(REASON_LOOKUP_THRESH_CHANGED);
	CASE_RETURN_STRING(REASON_OPPORTUNISTIC_THRESH_DIFF_CHANGED);
	CASE_RETURN_STRING(REASON_ROAM_RESCAN_RSSI_DIFF_CHANGED);
	CASE_RETURN_STRING(REASON_ROAM_BMISS_FIRST_BCNT_CHANGED);
	CASE_RETURN_STRING(REASON_ROAM_BMISS_FINAL_BCNT_CHANGED);
	CASE_RETURN_STRING(REASON_ROAM_BEACON_RSSI_WEIGHT_CHANGED);
	default:
		return "unknown";
	}
}

static void csr_neighbor_roam_reset_channel_info(tpCsrNeighborRoamChannelInfo
						 rChInfo);

QDF_STATUS csr_roam_copy_connected_profile(tpAniSirGlobal pMac, uint32_t sessionId,
					   tCsrRoamProfile *pDstProfile);

void csr_neighbor_roam_state_transition(tpAniSirGlobal mac_ctx,
		uint8_t newstate, uint8_t session)
{
	mac_ctx->roam.neighborRoamInfo[session].prevNeighborRoamState =
		mac_ctx->roam.neighborRoamInfo[session].neighborRoamState;
	mac_ctx->roam.neighborRoamInfo[session].neighborRoamState = newstate;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		FL("Sessionid (%d) NeighborRoam transition from %s to %s"),
		session, csr_neighbor_roam_state_to_string(
		mac_ctx->roam.neighborRoamInfo[session].prevNeighborRoamState),
		csr_neighbor_roam_state_to_string(newstate));
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

void csr_neighbor_roam_send_lfr_metric_event(
				tpAniSirGlobal mac_ctx,
				uint8_t session_id,
				tSirMacAddr bssid,
				eRoamCmdStatus status)
{
	tCsrRoamInfo *roam_info;

	roam_info = qdf_mem_malloc(sizeof(tCsrRoamInfo));
	if (NULL == roam_info) {
		sms_log(mac_ctx, LOG1, FL("Memory allocation failed!"));
	} else {
		qdf_mem_copy((void *)roam_info->bssid,
			     (void *)bssid, sizeof(*roam_info));
		csr_roam_call_callback(mac_ctx, session_id, roam_info, 0,
			status, 0);
		qdf_mem_free(roam_info);
	}
}
#endif

QDF_STATUS
csr_neighbor_roam_update_fast_roaming_enabled(tpAniSirGlobal pMac,
					      uint8_t sessionId,
					      const bool fastRoamEnabled)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];

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
QDF_STATUS csr_neighbor_roam_update_config(tpAniSirGlobal mac_ctx,
		uint8_t session_id, uint8_t value, uint8_t reason)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
	    &mac_ctx->roam.neighborRoamInfo[session_id];
	tpCsrNeighborRoamCfgParams cfg_params;
	eCsrNeighborRoamState state;
	uint8_t old_value;

	if (NULL == pNeighborRoamInfo) {
		sms_log(mac_ctx, LOG1, FL("Invalid Session ID %d"), session_id);
		return QDF_STATUS_E_FAILURE;
	}
	cfg_params = &pNeighborRoamInfo->cfgParams;
	state = pNeighborRoamInfo->neighborRoamState;
	if ((state == eCSR_NEIGHBOR_ROAM_STATE_CONNECTED) ||
			state == eCSR_NEIGHBOR_ROAM_STATE_INIT) {
		switch (reason) {
		case REASON_LOOKUP_THRESH_CHANGED:
			old_value = cfg_params->neighborLookupThreshold;
			cfg_params->neighborLookupThreshold = value;
			pNeighborRoamInfo->currentNeighborLookupThreshold =
				value;
			break;
		case REASON_OPPORTUNISTIC_THRESH_DIFF_CHANGED:
			old_value = cfg_params->nOpportunisticThresholdDiff;
			cfg_params->nOpportunisticThresholdDiff = value;
			pNeighborRoamInfo->currentOpportunisticThresholdDiff =
				value;
			break;
		case REASON_ROAM_RESCAN_RSSI_DIFF_CHANGED:
			old_value = cfg_params->nRoamRescanRssiDiff;
			cfg_params->nRoamRescanRssiDiff = value;
			pNeighborRoamInfo->currentRoamRescanRssiDiff = value;
			break;
		case REASON_ROAM_BMISS_FIRST_BCNT_CHANGED:
			old_value = cfg_params->nRoamBmissFirstBcnt;
			cfg_params->nRoamBmissFirstBcnt = value;
			pNeighborRoamInfo->currentRoamBmissFirstBcnt = value;
			break;
		case REASON_ROAM_BMISS_FINAL_BCNT_CHANGED:
			old_value = cfg_params->nRoamBmissFinalBcnt;
			cfg_params->nRoamBmissFinalBcnt = value;
			pNeighborRoamInfo->currentRoamBmissFinalBcnt = value;
			break;
		case REASON_ROAM_BEACON_RSSI_WEIGHT_CHANGED:
			old_value = cfg_params->nRoamBeaconRssiWeight;
			cfg_params->nRoamBeaconRssiWeight = value;
			pNeighborRoamInfo->currentRoamBeaconRssiWeight = value;
			break;
		default:
			sms_log(mac_ctx, LOG2, FL("Unknown update cfg reason"));
			return QDF_STATUS_E_FAILURE;
		}
	} else {
		sms_log(mac_ctx, LOGE, FL("Unexpected state %s, return fail"),
			mac_trace_get_neighbour_roam_state(state));
		return QDF_STATUS_E_FAILURE;
	}
	if (state == eCSR_NEIGHBOR_ROAM_STATE_CONNECTED) {
		sms_log(mac_ctx, LOG2, FL("CONNECTED, send update cfg cmd"));
		csr_roam_offload_scan(mac_ctx, session_id,
			ROAM_SCAN_OFFLOAD_UPDATE_CFG, reason);
	}
	sms_log(mac_ctx, LOG2, FL("LFR config for %s changed from %d to %d"),
			lfr_get_config_item_string(reason), old_value, value);
	return QDF_STATUS_SUCCESS;
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
			qdf_mem_free(rChInfo->currentChannelListInfo.
				     ChannelList);

		rChInfo->currentChannelListInfo.ChannelList = NULL;
	} else {
		rChInfo->currentChanIndex = 0;
	}
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
	qdf_mem_zero(pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo,
		     sizeof(tCsrNeighborReportBssInfo) *
		     MAX_BSS_IN_NEIGHBOR_RPT);
	pNeighborRoamInfo->uOsRequestedHandoff = 0;
	qdf_mem_zero(&pNeighborRoamInfo->handoffReqInfo,
		     sizeof(tCsrHandoffRequest));
}

void csr_neighbor_roam_reset_report_scan_state_control_info(tpAniSirGlobal pMac,
							    uint8_t sessionId)
{
	tpCsrNeighborRoamControlInfo pNeighborRoamInfo =
		&pMac->roam.neighborRoamInfo[sessionId];

	qdf_zero_macaddr(&pNeighborRoamInfo->currAPbssid);
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

	QDF_ASSERT(pScanFilter != NULL);
	if (pScanFilter == NULL)
		return QDF_STATUS_E_FAILURE;

	qdf_mem_zero(pScanFilter, sizeof(tCsrScanResultFilter));
	roam_params = &pMac->roam.configParam.roam_params;
	/* We dont want to set BSSID based Filter */
	pScanFilter->BSSIDs.numOfBSSIDs = 0;
	pScanFilter->scan_filter_for_roam = 1;
	/* only for HDD requested handoff fill in the BSSID in the filter */
	if (nbr_roam_info->uOsRequestedHandoff) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			FL("OS Requested Handoff"));
		pScanFilter->BSSIDs.numOfBSSIDs = 1;
		pScanFilter->BSSIDs.bssid =
			qdf_mem_malloc(sizeof(tSirMacAddr) *
				       pScanFilter->BSSIDs.numOfBSSIDs);
		if (NULL == pScanFilter->BSSIDs.bssid) {
			sms_log(pMac, LOGE,
				FL("Scan Filter BSSID mem alloc failed"));
			return QDF_STATUS_E_NOMEM;
		}

		qdf_mem_zero(pScanFilter->BSSIDs.bssid,
			     sizeof(tSirMacAddr) *
			     pScanFilter->BSSIDs.numOfBSSIDs);

		/* Populate the BSSID from handoff info received from HDD */
		for (i = 0; i < pScanFilter->BSSIDs.numOfBSSIDs; i++) {
			qdf_copy_macaddr(&pScanFilter->BSSIDs.bssid[i],
				 &nbr_roam_info->handoffReqInfo.bssid);
		}
	}
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		FL("No of Allowed SSID List:%d"),
		roam_params->num_ssid_allowed_list);
	if (roam_params->num_ssid_allowed_list) {
		pScanFilter->SSIDs.numOfSSIDs =
			roam_params->num_ssid_allowed_list;
		pScanFilter->SSIDs.SSIDList =
			qdf_mem_malloc(sizeof(tCsrSSIDInfo) *
				pScanFilter->SSIDs.numOfSSIDs);
		if (NULL == pScanFilter->SSIDs.SSIDList) {
			sms_log(pMac, LOGE,
				FL("Scan Filter SSID mem alloc failed"));
			return QDF_STATUS_E_NOMEM;
		}
		for (i = 0; i < roam_params->num_ssid_allowed_list; i++) {
			pScanFilter->SSIDs.SSIDList[i].handoffPermitted = 1;
			pScanFilter->SSIDs.SSIDList[i].ssidHidden = 0;
			qdf_mem_copy((void *)
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
			qdf_mem_malloc(sizeof(tCsrSSIDInfo));
		if (NULL == pScanFilter->SSIDs.SSIDList) {
			sms_log(pMac, LOGE,
				FL("Scan Filter SSID mem alloc failed"));
			return QDF_STATUS_E_NOMEM;
		}
		pScanFilter->SSIDs.SSIDList->handoffPermitted = 1;
		pScanFilter->SSIDs.SSIDList->ssidHidden = 0;
		pScanFilter->SSIDs.SSIDList->SSID.length =
			pCurProfile->SSID.length;
		qdf_mem_copy((void *)pScanFilter->SSIDs.SSIDList->SSID.ssId,
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
			qdf_mem_malloc(num_ch * sizeof(uint8_t));
		if (NULL == pScanFilter->ChannelInfo.ChannelList) {
			sms_log(pMac, LOGE,
				FL("Scan Filter Ch list mem alloc failed"));
			qdf_mem_free(pScanFilter->SSIDs.SSIDList);
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
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
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
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "%s: Wrong Number of Input Channels %d",
			  __func__, inputNumOfChannels);
		return QDF_STATUS_E_INVAL;
	}
	if (outputNumOfChannels >= WNI_CFG_VALID_CHANNEL_LIST_LEN) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
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
				QDF_TRACE(QDF_MODULE_ID_SME,
					  QDF_TRACE_LEVEL_INFO,
					  "%s: [INFOLOG] Adding extra %d to Neighbor channel list",
					  __func__, pInputChannelList[i]);
				pOutputChannelList[numChannels] =
					pInputChannelList[i];
				numChannels++;
			}
		}
		if (numChannels >= WNI_CFG_VALID_CHANNEL_LIST_LEN) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
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
			qdf_mem_free(pIes);
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
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		FL("Roaming parameters are reset"));
	roam_params = &mac_ctx->roam.configParam.roam_params;
	roam_params->num_ssid_allowed_list = 0;
	qdf_mem_set(&roam_params->ssid_allowed_list, 0,
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
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
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
			qdf_mem_copy(&pSession->prevApSSID,
				&pSession->connectedProfile.SSID,
				sizeof(tSirMacSSid));
			qdf_copy_macaddr(&pSession->prevApBssid,
					&pSession->connectedProfile.bssid);
			pSession->prevOpChannel =
				pSession->connectedProfile.operationChannel;
			pSession->isPrevApInfoValid = true;
			pSession->roamTS1 = qdf_mc_timer_get_system_time();
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
		csr_neighbor_roam_state_transition(pMac,
				eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId);
			pNeighborRoamInfo->roamChannelInfo.
				IAPPNeighborListReceived = false;
		}
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_INIT:
		csr_neighbor_roam_reset_init_state_control_info(pMac,
				sessionId);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_CONNECTED:
		csr_neighbor_roam_state_transition(pMac,
				eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId);
		pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived =
				false;
		csr_neighbor_roam_reset_connected_state_control_info(pMac,
				sessionId);
		break;

	case eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE:
		/* Stop pre-auth to reassoc interval timer */
		qdf_mc_timer_stop(&pSession->ftSmeContext.
				preAuthReassocIntvlTimer);
	case eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING:
		csr_neighbor_roam_state_transition(pMac,
				eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId);
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
		csr_neighbor_roam_state_transition(pMac,
				eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId);
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
 * Return: QDF status
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
	csr_neighbor_roam_state_transition(pMac,
			eCSR_NEIGHBOR_ROAM_STATE_CONNECTED, session_id);

	qdf_copy_macaddr(&ngbr_roam_info->currAPbssid,
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
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_HIGH,
			FL("11rAssoc is = %d"), ngbr_roam_info->is11rAssoc);

#ifdef FEATURE_WLAN_ESE
	/* Based on the auth scheme tell if we are 11r */
	if (session->connectedProfile.isESEAssoc) {
		if (pMac->roam.configParam.isFastTransitionEnabled)
			init_ft_flag = true;
		ngbr_roam_info->isESEAssoc = true;
	} else
		ngbr_roam_info->isESEAssoc = false;
	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO_HIGH,
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
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				"Wrong Mode %d",
				session->connectedProfile.BSSType);
			return;
		}
		ngbr_roam_info->uOsRequestedHandoff = 0;
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
		if (session->roam_synch_in_progress) {
			if (pMac->roam.pReassocResp != NULL) {
				QDF_TRACE(QDF_MODULE_ID_SME,
					QDF_TRACE_LEVEL_DEBUG,
					"Free Reassoc Rsp");
				qdf_mem_free(pMac->roam.pReassocResp);
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
 * @qdf_status: QDF status
 *
 * This function is called by CSR as soon as the station connects to an AP.
 * This initializes all the necessary data structures related to the
 * associated AP and transitions the state to CONNECTED state
 *
 * Return: QDF status
 */
QDF_STATUS csr_neighbor_roam_indicate_connect(
		tpAniSirGlobal pMac, uint8_t session_id,
		QDF_STATUS qdf_status)
{
	tpCsrNeighborRoamControlInfo ngbr_roam_info =
		&pMac->roam.neighborRoamInfo[session_id];
	tCsrRoamSession *session = &pMac->roam.roamSession[session_id];
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
	tCsrRoamInfo roamInfo;
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
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			"LFR3:csr_neighbor_roam_indicate_connect");
		msg = qdf_mem_malloc(sizeof(tSirSetActiveModeSetBncFilterReq));
		if (msg == NULL) {
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"LFR3:Mem Alloc failed for beacon Filter Req");
			return QDF_STATUS_E_NOMEM;
		}
		msg->messageType = eWNI_SME_SET_BCN_FILTER_REQ;
		msg->length = sizeof(tSirSetActiveModeSetBncFilterReq);
		msg->seesionId = session_id;
		qdf_copy_macaddr(&msg->bssid,
			&session->connectedProfile.bssid);
		status = cds_send_mb_message_to_mac(msg);
		qdf_copy_macaddr(&roamInfo.peerMac,
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
			csr_neighbor_roam_state_transition(pMac,
				eCSR_NEIGHBOR_ROAM_STATE_INIT, session_id);
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

		qdf_mem_zero(pFTRoamInfo->neighboReportBssInfo,
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
		qdf_mem_malloc(pMac->roam.configParam.neighborRoamConfig.
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
	qdf_mem_copy(pNeighborRoamInfo->cfgParams.channelInfo.ChannelList,
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

	qdf_zero_macaddr(&pNeighborRoamInfo->currAPbssid);
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
	qdf_mem_set(&pNeighborRoamInfo->prevConnProfile,
		    sizeof(tCsrRoamConnectedProfile), 0);

	status = csr_ll_open(pMac->hHdd, &pNeighborRoamInfo->roamableAPList);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(pMac, LOGE, FL("LL Open of roam able AP List failed"));
		qdf_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.
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
		qdf_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.
			     ChannelList);
		pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;
		csr_ll_close(&pNeighborRoamInfo->roamableAPList);
		return QDF_STATUS_E_RESOURCES;
	}

	csr_neighbor_roam_state_transition(pMac,
			eCSR_NEIGHBOR_ROAM_STATE_INIT, sessionId);
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
		qdf_mem_free(pNeighborRoamInfo->cfgParams.channelInfo.
			     ChannelList);

	pNeighborRoamInfo->cfgParams.channelInfo.ChannelList = NULL;

	/* Should free up the nodes in the list before closing the double Linked list */
	csr_neighbor_roam_free_roamable_bss_list(pMac,
						 &pNeighborRoamInfo->roamableAPList);
	csr_ll_close(&pNeighborRoamInfo->roamableAPList);

	if (pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
	    ChannelList) {
		qdf_mem_free(pNeighborRoamInfo->roamChannelInfo.
			     currentChannelListInfo.ChannelList);
	}

	pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.ChannelList =
		NULL;
	pNeighborRoamInfo->roamChannelInfo.currentChanIndex =
		CSR_NEIGHBOR_ROAM_INVALID_CHANNEL_INDEX;
	pNeighborRoamInfo->roamChannelInfo.currentChannelListInfo.
	numOfChannels = 0;
	pNeighborRoamInfo->roamChannelInfo.IAPPNeighborListReceived = false;

	/* Free the profile.. */
	csr_release_profile(pMac, &pNeighborRoamInfo->csrNeighborRoamProfile);
	csr_roam_free_connect_profile(&pNeighborRoamInfo->prevConnProfile);
	pNeighborRoamInfo->FTRoamInfo.currentNeighborRptRetryNum = 0;
	pNeighborRoamInfo->FTRoamInfo.numBssFromNeighborReport = 0;
	qdf_mem_zero(pNeighborRoamInfo->FTRoamInfo.neighboReportBssInfo,
		     sizeof(tCsrNeighborReportBssInfo) *
		     MAX_BSS_IN_NEIGHBOR_RPT);
	csr_neighbor_roam_free_roamable_bss_list(pMac,
						 &pNeighborRoamInfo->FTRoamInfo.
						 preAuthDoneList);
	csr_ll_close(&pNeighborRoamInfo->FTRoamInfo.preAuthDoneList);

	csr_neighbor_roam_state_transition(pMac,
		eCSR_NEIGHBOR_ROAM_STATE_CLOSED, sessionId);

	return;
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
	uint8_t roam_now = 0;
	uint8_t roamable_ap_count = 0;
	tCsrScanResultFilter    scan_filter;
	tScanResultHandle       scan_result;

	if (NULL == session) {
		sms_log(mac_ctx, LOGE, FL("session is NULL "));
		return QDF_STATUS_E_FAILURE;
	}

	roam_id = GET_NEXT_ROAM_ID(&mac_ctx->roam);
	profile = qdf_mem_malloc(sizeof(tCsrRoamProfile));
	if (NULL == profile) {
		sms_log(mac_ctx, LOGE, FL("Memory alloc failed"));
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_set(profile, sizeof(tCsrRoamProfile), 0);
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
			qdf_mem_malloc(sizeof(tSirMacAddr) *
				profile->BSSIDs.numOfBSSIDs);
		if (NULL == profile->BSSIDs.bssid) {
			sms_log(mac_ctx, LOGE, FL("mem alloc failed for BSSID"));
			status = QDF_STATUS_E_NOMEM;
			goto end;
		}
	}

	qdf_mem_zero(profile->BSSIDs.bssid,
		sizeof(tSirMacAddr) *
		profile->BSSIDs.numOfBSSIDs);

	/* Populate the BSSID from handoff info received from HDD */
	for (i = 0; i < profile->BSSIDs.numOfBSSIDs; i++) {
		qdf_copy_macaddr(&profile->BSSIDs.bssid[i],
				&roam_ctrl_info->handoffReqInfo.bssid);
	}

	profile->ChannelInfo.numOfChannels = 1;
	if (NULL == profile->ChannelInfo.ChannelList) {
		profile->ChannelInfo.ChannelList =
			qdf_mem_malloc(sizeof(*profile->
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

	/*
	 * For User space connect requests, the scan has already been done.
	 * So, check if the BSS descriptor exists in the scan cache and
	 * proceed with the handoff instead of a redundant scan again.
	 */
	if (roam_ctrl_info->handoffReqInfo.src == CONNECT_CMD_USERSPACE) {
		sms_log(mac_ctx, LOG1,
			FL("Connect cmd with bssid within same ESS"));
		status = csr_neighbor_roam_prepare_scan_profile_filter(mac_ctx,
								&scan_filter,
								session_id);
		sms_log(mac_ctx, LOG1, FL("Filter creation status = %d"),
			status);
		status = csr_scan_get_result(mac_ctx, &scan_filter,
					     &scan_result);
		csr_neighbor_roam_process_scan_results(mac_ctx, session_id,
							&scan_result);
		roamable_ap_count = csr_ll_count(
					&roam_ctrl_info->roamableAPList);
		csr_free_scan_filter(mac_ctx, &scan_filter);
		sms_log(mac_ctx, LOG1, FL("roam_now=%d, roamable_ap_count=%d"),
			roam_now, roamable_ap_count);
	}
	if (roam_now && roamable_ap_count) {
		csr_neighbor_roam_trigger_handoff(mac_ctx, session_id);
	} else {
		status = csr_scan_for_ssid(mac_ctx, session_id, profile,
					   roam_id, false);
		if (status != QDF_STATUS_SUCCESS)
			sms_log(mac_ctx, LOGE, FL("SSID scan failed"));
	}

end:
	if (NULL != profile) {
		csr_release_profile(mac_ctx, profile);
		qdf_mem_free(profile);
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
	if (!qdf_mem_cmp(handoff_req->bssid,
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
	qdf_mem_copy(&roam_ctrl_info->handoffReqInfo.bssid.bytes,
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
			FL("Received in not CONNECTED state(%d) or uOsRequestedHandoff(%d) is not set. Ignore it"),
			pNeighborRoamInfo->neighborRoamState,
			pNeighborRoamInfo->uOsRequestedHandoff);
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
