/*
 * Copyright (c) 2011-2017 The Linux Foundation. All rights reserved.
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
 * DOC: csr_api_scan.c
 *
 * Implementation for the Common Scan interfaces.
 */

#include "ani_global.h"

#include "csr_inside_api.h"
#include "sme_inside.h"

#include "csr_support.h"

#include "host_diag_core_log.h"
#include "host_diag_core_event.h"

#include "cds_reg_service.h"
#include "wma_types.h"
#include "cds_utils.h"
#include "cfg_api.h"
#include "wma.h"

#include "wlan_policy_mgr_api.h"
#include "wlan_hdd_main.h"
#include "pld_common.h"
#include "csr_internal.h"
#ifdef NAPIER_SCAN
#include <wlan_scan_ucfg_api.h>
#include <wlan_scan_tgt_api.h>
#include <wlan_scan_utils_api.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_utility.h>
#endif
#include "wlan_reg_services_api.h"

#define MIN_CHN_TIME_TO_FIND_GO 100
#define MAX_CHN_TIME_TO_FIND_GO 100
#define DIRECT_SSID_LEN 7

/* Purpose of HIDDEN_TIMER
 * When we remove hidden ssid from the profile i.e., forget the SSID via GUI
 * that SSID shouldn't see in the profile For above requirement we used timer
 * limit, logic is explained below Timer value is initialsed to current time
 * when it receives corresponding probe response of hidden SSID (The probe
 * request is received regularly till SSID in the profile. Once it is removed
 * from profile probe request is not sent.) when we receive probe response for
 * broadcast probe request, during update SSID with saved SSID we will diff
 * current time with saved SSID time if it is greater than 1 min then we are
 * not updating with old one
 */

#define HIDDEN_TIMER (1*60*1000)
/* must be less than 100, represent the persentage of new RSSI */
#define CSR_SCAN_RESULT_RSSI_WEIGHT     80
#define MAX_ACTIVE_SCAN_FOR_ONE_CHANNEL 140
#define MIN_ACTIVE_SCAN_FOR_ONE_CHANNEL 120

#define MAX_ACTIVE_SCAN_FOR_ONE_CHANNEL_FASTREASSOC 30
#define MIN_ACTIVE_SCAN_FOR_ONE_CHANNEL_FASTREASSOC 20

#define PCL_ADVANTAGE 30
#define PCL_RSSI_THRESHOLD -75

#define CSR_SCAN_IS_OVER_BSS_LIMIT(pMac)  \
	((pMac)->scan.nBssLimit <= (csr_ll_count(&(pMac)->scan.scanResultList)))

static void csr_set_default_scan_timing(tpAniSirGlobal pMac,
					tSirScanType scanType,
					tCsrScanRequest *pScanRequest);
static QDF_STATUS csr_scan_channels(tpAniSirGlobal pMac, tSmeCmd *pCommand);
static void csr_set_cfg_valid_channel_list(tpAniSirGlobal pMac, uint8_t
					*pChannelList, uint8_t NumChannels);
static void csr_save_tx_power_to_cfg(tpAniSirGlobal pMac, tDblLinkList *pList,
			      uint32_t cfgId);
static void csr_set_cfg_country_code(tpAniSirGlobal pMac, uint8_t *countryCode);
static void csr_purge_channel_power(tpAniSirGlobal pMac, tDblLinkList
							*pChannelList);
static bool csr_roam_is_valid_channel(tpAniSirGlobal pMac, uint8_t channel);

#define CSR_IS_SOCIAL_CHANNEL(channel) \
	(((channel) == 1) || ((channel) == 6) || ((channel) == 11))

static void csr_release_scan_cmd_pending_list(tpAniSirGlobal pMac)
{
	tListElem *pEntry;
	tSmeCmd *pCommand;

	while ((pEntry =
			csr_ll_remove_head(&pMac->scan.scanCmdPendingList,
					   LL_ACCESS_LOCK)) != NULL) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		if (eSmeCsrCommandMask & pCommand->command)
			csr_release_command(pMac, pCommand);
		else
			sme_err("Received command: %d", pCommand->command);
	}
}

/* pResult is invalid calling this function. */
void csr_free_scan_result_entry(tpAniSirGlobal pMac, tCsrScanResult *pResult)
{
	if (NULL != pResult->Result.pvIes)
		qdf_mem_free(pResult->Result.pvIes);

	qdf_mem_free(pResult);
}

static QDF_STATUS csr_ll_scan_purge_result(tpAniSirGlobal pMac,
					   tDblLinkList *pList)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tListElem *pEntry;
	tCsrScanResult *pBssDesc;

	csr_ll_lock(pList);

	while ((pEntry = csr_ll_remove_head(pList, LL_ACCESS_NOLOCK)) != NULL) {
		pBssDesc = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
		csr_free_scan_result_entry(pMac, pBssDesc);
	}

	csr_ll_unlock(pList);

	return status;
}

QDF_STATUS csr_scan_open(tpAniSirGlobal mac_ctx)
{
	csr_ll_open(mac_ctx->hHdd, &mac_ctx->scan.channelPowerInfoList24);
	csr_ll_open(mac_ctx->hHdd, &mac_ctx->scan.channelPowerInfoList5G);
#ifdef WLAN_AP_STA_CONCURRENCY
	csr_ll_open(mac_ctx->hHdd, &mac_ctx->scan.scanCmdPendingList);
#endif
	mac_ctx->scan.fFullScanIssued = false;
	mac_ctx->scan.nBssLimit = CSR_MAX_BSS_SUPPORT;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_scan_close(tpAniSirGlobal pMac)
{
#ifdef WLAN_AP_STA_CONCURRENCY
	csr_release_scan_cmd_pending_list(pMac);
#endif
#ifdef WLAN_AP_STA_CONCURRENCY
	csr_ll_close(&pMac->scan.scanCmdPendingList);
#endif
	csr_purge_channel_power(pMac, &pMac->scan.channelPowerInfoList24);
	csr_purge_channel_power(pMac, &pMac->scan.channelPowerInfoList5G);
	csr_ll_close(&pMac->scan.channelPowerInfoList24);
	csr_ll_close(&pMac->scan.channelPowerInfoList5G);
	csr_scan_disable(pMac);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_scan_enable(tpAniSirGlobal pMac)
{

	pMac->scan.fScanEnable = true;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_scan_disable(tpAniSirGlobal pMac)
{
	pMac->scan.fScanEnable = false;

	return QDF_STATUS_SUCCESS;
}

/* Set scan timing parameters according to state of other driver sessions */
/* No validation of the parameters is performed. */
static void csr_set_default_scan_timing(tpAniSirGlobal pMac,
					tSirScanType scanType,
					tCsrScanRequest *pScanRequest)
{
#ifdef WLAN_AP_STA_CONCURRENCY
	if (csr_is_any_session_connected(pMac)) {
		/* Reset passive scan time as per ini parameter. */
		cfg_set_int(pMac, WNI_CFG_PASSIVE_MAXIMUM_CHANNEL_TIME,
			    pMac->roam.configParam.nPassiveMaxChnTimeConc);
		/* If multi-session, use the appropriate default scan times */
		if (scanType == eSIR_ACTIVE_SCAN) {
			pScanRequest->maxChnTime =
				pMac->roam.configParam.nActiveMaxChnTimeConc;
			pScanRequest->minChnTime =
				pMac->roam.configParam.nActiveMinChnTimeConc;
		} else {
			pScanRequest->maxChnTime =
				pMac->roam.configParam.nPassiveMaxChnTimeConc;
			pScanRequest->minChnTime =
				pMac->roam.configParam.nPassiveMinChnTimeConc;
		}
		pScanRequest->restTime = pMac->roam.configParam.nRestTimeConc;

		pScanRequest->min_rest_time =
			pMac->roam.configParam.min_rest_time_conc;
		pScanRequest->idle_time =
			pMac->roam.configParam.idle_time_conc;

		/* Return so that fields set above will not be overwritten. */
		return;
	}
#endif

	/* This portion of the code executed if multi-session not supported */
	/* (WLAN_AP_STA_CONCURRENCY not defined) or no multi-session. */
	/* Use the "regular" (non-concurrency) default scan timing. */
	cfg_set_int(pMac, WNI_CFG_PASSIVE_MAXIMUM_CHANNEL_TIME,
		    pMac->roam.configParam.nPassiveMaxChnTime);
	if (pScanRequest->scanType == eSIR_ACTIVE_SCAN) {
		pScanRequest->maxChnTime =
			pMac->roam.configParam.nActiveMaxChnTime;
		pScanRequest->minChnTime =
			pMac->roam.configParam.nActiveMinChnTime;
	} else {
		pScanRequest->maxChnTime =
			pMac->roam.configParam.nPassiveMaxChnTime;
		pScanRequest->minChnTime =
			pMac->roam.configParam.nPassiveMinChnTime;
	}
#ifdef WLAN_AP_STA_CONCURRENCY

	/* No rest time/Idle time if no sessions are connected. */
	pScanRequest->restTime = 0;
	pScanRequest->min_rest_time = 0;
	pScanRequest->idle_time = 0;

#endif
}

/**
 * csr_scan_2g_only_request() - This function will update the scan request with
 * only 2.4GHz valid channel list.
 * @mac_ctx:      Pointer to Global MAC structure
 * @scan_cmd      scan cmd
 * @scan_req      scan req
 *
 * This function will update the scan request with  only 2.4GHz valid channel
 * list.
 *
 * @Return: status of operation
 */
static QDF_STATUS
csr_scan_2g_only_request(tpAniSirGlobal mac_ctx,
			 tSmeCmd *scan_cmd,
			 tCsrScanRequest *scan_req)
{
	uint8_t idx, lst_sz = 0;

	QDF_ASSERT(scan_cmd && scan_req);
	/* To silence the KW tool null check is added */
	if ((scan_cmd == NULL) || (scan_req == NULL)) {
		sme_err("Scan Cmd or Scan Request is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (eCSR_SCAN_REQUEST_FULL_SCAN != scan_req->requestType)
		return QDF_STATUS_SUCCESS;

	sme_debug("Scanning only 2G Channels during first scan");

	/* Contsruct valid Supported 2.4 GHz Channel List */
	if (NULL == scan_req->ChannelInfo.ChannelList) {
		scan_req->ChannelInfo.ChannelList =
			qdf_mem_malloc(NUM_24GHZ_CHANNELS);
		if (NULL == scan_req->ChannelInfo.ChannelList) {
			sme_err("Memory allocation failed");
			return QDF_STATUS_E_NOMEM;
		}
		for (idx = 1; idx <= NUM_24GHZ_CHANNELS; idx++) {
			if (csr_is_supported_channel(mac_ctx, idx)) {
				scan_req->ChannelInfo.ChannelList[lst_sz] = idx;
				lst_sz++;
			}
		}
	} else {
		for (idx = 0;
		     idx < scan_req->ChannelInfo.numOfChannels;
		     idx++) {
			if (scan_req->ChannelInfo.ChannelList[idx] <=
				CDS_24_GHZ_CHANNEL_14
			    && csr_is_supported_channel(mac_ctx,
				scan_req->ChannelInfo.ChannelList[idx])) {
				scan_req->ChannelInfo.ChannelList[lst_sz] =
					scan_req->ChannelInfo.ChannelList[idx];
				lst_sz++;
			}
		}
	}
	scan_req->ChannelInfo.numOfChannels = lst_sz;
	return QDF_STATUS_SUCCESS;
}

static void
csr_set_scan_reason(tSmeCmd *scan_cmd, eCsrRequestType req_type)
{
	switch (req_type) {
	case eCSR_SCAN_REQUEST_11D_SCAN:
		scan_cmd->u.scanCmd.reason = eCsrScan11d1;
		break;
#ifdef SOFTAP_CHANNEL_RANGE
	case eCSR_SCAN_SOFTAP_CHANNEL_RANGE:
#endif
	case eCSR_SCAN_REQUEST_FULL_SCAN:
	case eCSR_SCAN_P2P_DISCOVERY:
		scan_cmd->u.scanCmd.reason = eCsrScanUserRequest;
		break;
	case eCSR_SCAN_HO_PROBE_SCAN:
		scan_cmd->u.scanCmd.reason = eCsrScanProbeBss;
		break;
	case eCSR_SCAN_P2P_FIND_PEER:
		scan_cmd->u.scanCmd.reason = eCsrScanP2PFindPeer;
		break;
	default:
		break;
	}
}

static QDF_STATUS
csr_issue_11d_scan(tpAniSirGlobal mac_ctx, tSmeCmd *scan_cmd,
		   tCsrScanRequest *scan_req, uint16_t session_id)
{
	QDF_STATUS status;
	tSmeCmd *scan_11d_cmd = NULL;
	tCsrScanRequest tmp_rq;
	tCsrChannelInfo *chn_info = &tmp_rq.ChannelInfo;
	uint32_t num_chn = mac_ctx->scan.base_channels.numChannels;
	tCsrRoamSession *csr_session = CSR_GET_SESSION(mac_ctx, session_id);

	if (csr_session == NULL) {
		sme_err("session %d not found", session_id);
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	if (num_chn > WNI_CFG_VALID_CHANNEL_LIST_LEN) {
		sme_err("invalid number of channels: %d", num_chn);
		return QDF_STATUS_E_FAILURE;
	}

	if (!(((false == mac_ctx->first_scan_done)
	     && (eCSR_SCAN_REQUEST_11D_SCAN != scan_req->requestType))
#ifdef SOFTAP_CHANNEL_RANGE
	    && (eCSR_SCAN_SOFTAP_CHANNEL_RANGE != scan_req->requestType)
#endif
	    && (false == mac_ctx->scan.fEnableBypass11d)))
		return QDF_STATUS_SUCCESS;

	qdf_mem_set(&tmp_rq, sizeof(tCsrScanRequest), 0);
	scan_11d_cmd = csr_get_command_buffer(mac_ctx);
	if (!scan_11d_cmd) {
		sme_err("scan_11d_cmd failed");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_set(&scan_11d_cmd->u.scanCmd, sizeof(tScanCmd), 0);
	chn_info->ChannelList = qdf_mem_malloc(num_chn);
	if (NULL == chn_info->ChannelList) {
		sme_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}
	qdf_mem_copy(chn_info->ChannelList,
		     mac_ctx->scan.base_channels.channelList, num_chn);

	chn_info->numOfChannels = (uint8_t) num_chn;
	scan_11d_cmd->command = eSmeCommandScan;
	scan_11d_cmd->u.scanCmd.callback = mac_ctx->scan.callback11dScanDone;
	scan_11d_cmd->u.scanCmd.pContext = NULL;
	wma_get_scan_id(&scan_11d_cmd->u.scanCmd.scanID);
	tmp_rq.BSSType = eCSR_BSS_TYPE_ANY;
	tmp_rq.scan_id = scan_11d_cmd->u.scanCmd.scanID;

	status = qdf_mc_timer_init(&scan_cmd->u.scanCmd.csr_scan_timer,
			QDF_TIMER_TYPE_SW,
			csr_scan_active_list_timeout_handle, scan_11d_cmd);

	if (csr_is11d_supported(mac_ctx)) {
		tmp_rq.bcnRptReqScan = scan_req->bcnRptReqScan;
		if (scan_req->bcnRptReqScan)
			tmp_rq.scanType = scan_req->scanType ?
				eSIR_PASSIVE_SCAN : scan_req->scanType;
		else
			tmp_rq.scanType = eSIR_PASSIVE_SCAN;
		tmp_rq.requestType = eCSR_SCAN_REQUEST_11D_SCAN;
		scan_11d_cmd->u.scanCmd.reason = eCsrScan11d1;
		tmp_rq.maxChnTime =
			mac_ctx->roam.configParam.nPassiveMaxChnTime;
		tmp_rq.minChnTime =
			mac_ctx->roam.configParam.nPassiveMinChnTime;
	} else {
		tmp_rq.bcnRptReqScan = scan_req->bcnRptReqScan;
		if (scan_req->bcnRptReqScan)
			tmp_rq.scanType = scan_req->scanType;
		else
			tmp_rq.scanType = eSIR_ACTIVE_SCAN;
		tmp_rq.requestType = scan_req->requestType;
		scan_11d_cmd->u.scanCmd.reason = scan_cmd->u.scanCmd.reason;
		tmp_rq.maxChnTime = mac_ctx->roam.configParam.nActiveMaxChnTime;
		tmp_rq.minChnTime = mac_ctx->roam.configParam.nActiveMinChnTime;
	}
	if (mac_ctx->roam.configParam.nInitialDwellTime) {
		tmp_rq.maxChnTime = mac_ctx->roam.configParam.nInitialDwellTime;
		sme_debug("11d scan, updating dwell time for first scan %u",
			tmp_rq.maxChnTime);
	}

	status = csr_scan_copy_request(mac_ctx,
			&scan_11d_cmd->u.scanCmd.u.scanRequest, &tmp_rq);
	/* Free the channel list */
	qdf_mem_free(chn_info->ChannelList);
	chn_info->ChannelList = NULL;
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("csr_scan_copy_request failed");
		return QDF_STATUS_E_FAILURE;
	}

	mac_ctx->scan.scanProfile.numOfChannels =
		scan_11d_cmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels;


	status = csr_queue_sme_command(mac_ctx, scan_11d_cmd, false);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Failed to send message status = %d",
			status);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_scan_request(tpAniSirGlobal pMac, uint16_t sessionId,
			    tCsrScanRequest *scan_req,
			    csr_scan_completeCallback callback, void *pContext)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tSmeCmd *scan_cmd = NULL;
	tCsrScanRequest *pTempScanReq = NULL;
	tCsrConfig *cfg_prm = &pMac->roam.configParam;

	if (scan_req == NULL) {
		sme_err("scan_req is NULL");
		QDF_ASSERT(0);
		return status;
	}

	/*
	 * During group formation, the P2P client scans for GO with the specific
	 * SSID. There will be chances of GO switching to other channels because
	 * of scan or to STA channel in case of STA+GO MCC scenario. So to
	 * increase the possibility of client to find the GO, the dwell time of
	 * scan is increased to 100ms.
	 * If the scan request is for specific SSId the length of SSID will be
	 * greater than 7 as SSID for p2p search contains "DIRECT-")
	 */
	if (scan_req->p2pSearch
	    && scan_req->SSIDs.numOfSSIDs
	    && (NULL != scan_req->SSIDs.SSIDList)
	    && (scan_req->SSIDs.SSIDList->SSID.length > DIRECT_SSID_LEN)) {
		sme_debug("P2P: Increasing the min and max Dwell time to %d for specific SSID scan %.*s",
			MAX_CHN_TIME_TO_FIND_GO,
			scan_req->SSIDs.SSIDList->SSID.length,
			scan_req->SSIDs.SSIDList->SSID.ssId);
		scan_req->maxChnTime = MAX_CHN_TIME_TO_FIND_GO;
		scan_req->minChnTime = MIN_CHN_TIME_TO_FIND_GO;
	}

	if (!pMac->scan.fScanEnable) {
		sme_err("SId: %d Scanning not enabled Scan type=%u, numOfSSIDs=%d P2P search=%d",
			sessionId, scan_req->requestType,
			scan_req->SSIDs.numOfSSIDs,
			scan_req->p2pSearch);
		goto release_cmd;
	}

	scan_cmd = csr_get_command_buffer(pMac);
	if (!scan_cmd) {
		sme_err("scan_cmd is NULL");
		goto release_cmd;
	}

	qdf_mem_set(&scan_cmd->u.scanCmd, sizeof(tScanCmd), 0);
	scan_cmd->command = eSmeCommandScan;
	scan_cmd->sessionId = sessionId;
	if (scan_cmd->sessionId >= CSR_ROAM_SESSION_MAX)
		sme_err("Invalid Sme SessionID: %d", sessionId);
	scan_cmd->u.scanCmd.callback = callback;
	scan_cmd->u.scanCmd.pContext = pContext;
	csr_set_scan_reason(scan_cmd, scan_req->requestType);
	if (scan_req->minChnTime == 0 && scan_req->maxChnTime == 0) {
		/* The caller doesn't set the time correctly. Set it here */
		csr_set_default_scan_timing(pMac, scan_req->scanType, scan_req);
		sme_debug("Setting default min %d and max %d ChnTime",
			scan_req->minChnTime, scan_req->maxChnTime);
	}
#ifdef WLAN_AP_STA_CONCURRENCY
	/*
	 * Need to set restTime/min_Ret_time/idle_time
	 * only if at least one session is connected
	 */
	if (scan_req->restTime == 0 && csr_is_any_session_connected(pMac)) {
		scan_req->restTime = cfg_prm->nRestTimeConc;
		scan_req->min_rest_time = cfg_prm->min_rest_time_conc;
		scan_req->idle_time = cfg_prm->idle_time_conc;
		if (scan_req->scanType == eSIR_ACTIVE_SCAN) {
			scan_req->maxChnTime = cfg_prm->nActiveMaxChnTimeConc;
			scan_req->minChnTime = cfg_prm->nActiveMinChnTimeConc;
		} else {
			scan_req->maxChnTime = cfg_prm->nPassiveMaxChnTimeConc;
			scan_req->minChnTime = cfg_prm->nPassiveMinChnTimeConc;
		}
	}
#endif
	/* Increase dwell time in case P2P Search and Miracast is not present */
	if (scan_req->p2pSearch && scan_req->ChannelInfo.numOfChannels
	    == P2P_SOCIAL_CHANNELS && (!(pMac->sme.miracast_value))) {
		scan_req->maxChnTime += P2P_SEARCH_DWELL_TIME_INCREASE;
	}
	scan_cmd->u.scanCmd.scanID = scan_req->scan_id;
	/*
	 * If it is the first scan request from HDD, CSR checks if it is for 11d
	 * If it is not, CSR will save the scan request in the pending cmd queue
	 * & issue an 11d scan request to PE.
	 */
	status = csr_issue_11d_scan(pMac, scan_cmd, scan_req, sessionId);
	if (status != QDF_STATUS_SUCCESS)
		goto release_cmd;

	/*
	 * Scan only 2G Channels if set in ini file. This is mainly to reduce
	 * the First Scan duration once we turn on Wifi
	 */
	if (pMac->scan.fFirstScanOnly2GChnl
	    && false == pMac->first_scan_done) {
		csr_scan_2g_only_request(pMac, scan_cmd, scan_req);
		pMac->first_scan_done = true;
	}


	if (cfg_prm->nInitialDwellTime) {
		scan_req->maxChnTime = cfg_prm->nInitialDwellTime;
		cfg_prm->nInitialDwellTime = 0;
		sme_debug("updating dwell time for first scan %u",
			scan_req->maxChnTime);
	}
	scan_req->scan_adaptive_dwell_mode = cfg_prm->scan_adaptive_dwell_mode;

	status = csr_scan_copy_request(pMac, &scan_cmd->u.scanCmd.u.scanRequest,
				       scan_req);
	/*
	 * Reset the variable after the first scan is  queued after loading the
	 * driver. The purpose of this parameter is that DFS channels are
	 * skipped during the first scan after loading the driver. The above API
	 * builds the target scan request in which this variable is used.
	 */
	cfg_prm->initial_scan_no_dfs_chnl = 0;
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("fail to copy request status: %d", status);
		goto release_cmd;
	}

	pTempScanReq = &scan_cmd->u.scanCmd.u.scanRequest;
	pMac->scan.scanProfile.numOfChannels =
		pTempScanReq->ChannelInfo.numOfChannels;
	status = qdf_mc_timer_init(&scan_cmd->u.scanCmd.csr_scan_timer,
				QDF_TIMER_TYPE_SW,
				csr_scan_active_list_timeout_handle, scan_cmd);
	sme_debug(
		"SId=%d scanId=%d Scan reason=%u numSSIDs=%d numChan=%d P2P search=%d minCT=%d maxCT=%d uIEFieldLen=%d BSSID: " MAC_ADDRESS_STR,
		sessionId, scan_cmd->u.scanCmd.scanID,
		scan_cmd->u.scanCmd.reason, pTempScanReq->SSIDs.numOfSSIDs,
		pTempScanReq->ChannelInfo.numOfChannels,
		pTempScanReq->p2pSearch, pTempScanReq->minChnTime,
		pTempScanReq->maxChnTime, pTempScanReq->uIEFieldLen,
		MAC_ADDR_ARRAY(scan_cmd->u.scanCmd.u.scanRequest.bssid.bytes));

	status = csr_queue_sme_command(pMac, scan_cmd, false);
	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("fail to send message status: %d", status);

release_cmd:
	if (!QDF_IS_STATUS_SUCCESS(status) && scan_cmd) {
		sme_err(" SId: %d Failed with status=%d Scan reason=%u numOfSSIDs=%d P2P search=%d scanId=%d",
			sessionId, status, scan_cmd->u.scanCmd.reason,
			scan_req->SSIDs.numOfSSIDs, scan_req->p2pSearch,
			scan_cmd->u.scanCmd.scanID);
		csr_release_command(pMac, scan_cmd);
	}

	return status;
}
#ifndef NAPIER_SCAN
static QDF_STATUS csr_issue_roam_after_lostlink_scan(tpAniSirGlobal pMac,
						     uint32_t sessionId,
						     eCsrRoamReason reason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tScanResultHandle hBSSList = NULL;
	tCsrScanResultFilter *pScanFilter = NULL;
	uint32_t roamId = 0;
	tCsrRoamProfile *pProfile = NULL;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sme_err("session %d not found", sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	if (pSession->fCancelRoaming) {
		sme_debug("lost link roaming canceled");
		status = QDF_STATUS_SUCCESS;
		goto free_filter;
	}
	/* Here is the profile we need to connect to */
	pScanFilter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == pScanFilter) {
		status = QDF_STATUS_E_NOMEM;
		goto free_filter;
	}
	if (NULL == pSession->pCurRoamProfile) {
		pScanFilter->EncryptionType.numEntries = 1;
		pScanFilter->EncryptionType.encryptionType[0] =
			eCSR_ENCRYPT_TYPE_NONE;
	} else {
		/*
		 * We have to make a copy of pCurRoamProfile because it will
		 * be free inside csr_roam_issue_connect
		 */
		pProfile = qdf_mem_malloc(sizeof(tCsrRoamProfile));
		if (NULL == pProfile) {
			status = QDF_STATUS_E_NOMEM;
			goto free_filter;
		}
		status = csr_roam_copy_profile(pMac, pProfile,
					       pSession->pCurRoamProfile);
		if (!QDF_IS_STATUS_SUCCESS(status))
			goto free_filter;
		status = csr_roam_prepare_filter_from_profile(pMac, pProfile,
							      pScanFilter);
	} /* We have a profile */
	roamId = GET_NEXT_ROAM_ID(&pMac->roam);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto free_filter;

	status = csr_scan_get_result(pMac, pScanFilter, &hBSSList);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto free_filter;

	if (eCsrLostLink1 == reason) {
		/* if possible put the last connected BSS in beginning */
		csr_move_bss_to_head_from_bssid(pMac,
				&pSession->connectedProfile.bssid, hBSSList);
	}
	status = csr_roam_issue_connect(pMac, sessionId, pProfile, hBSSList,
					reason, roamId, true, true);
	if (!QDF_IS_STATUS_SUCCESS(status))
		csr_scan_result_purge(pMac, hBSSList);

free_filter:
	if (pScanFilter) {
		/* we need to free memory for filter if profile exists */
		csr_free_scan_filter(pMac, pScanFilter);
		qdf_mem_free(pScanFilter);
	}
	if (NULL != pProfile) {
		csr_release_profile(pMac, pProfile);
		qdf_mem_free(pProfile);
	}
	return status;
}

QDF_STATUS csr_scan_handle_failed_lostlink1(tpAniSirGlobal pMac,
					    uint32_t sessionId)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sme_err("session %d not found", sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	sme_debug("Lost link scan 1 failed");
	if (pSession->fCancelRoaming)
		return QDF_STATUS_E_FAILURE;
	if (!pSession->pCurRoamProfile)
		return csr_scan_request_lost_link3(pMac, sessionId);
	/*
	 * We fail lostlink1 but there may be other BSS in the cached result
	 * fit the profile. Give it a try first
	 */
	if (pSession->pCurRoamProfile->SSIDs.numOfSSIDs == 0 ||
	    pSession->pCurRoamProfile->SSIDs.numOfSSIDs > 1)
		/* try lostlink scan2 */
		return csr_scan_request_lost_link2(pMac, sessionId);
	if (!pSession->pCurRoamProfile->ChannelInfo.ChannelList
	    || pSession->pCurRoamProfile->ChannelInfo.ChannelList[0] == 0)
		/* go straight to lostlink scan3 */
		return csr_scan_request_lost_link3(pMac, sessionId);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_scan_handle_failed_lostlink2(tpAniSirGlobal pMac,
					    uint32_t sessionId)
{
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sme_err("session %d not found", sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	sme_debug("Lost link scan 2 failed");
	if (pSession->fCancelRoaming)
		return QDF_STATUS_E_FAILURE;

	if (!pSession->pCurRoamProfile
	    || !pSession->pCurRoamProfile->ChannelInfo.ChannelList
	    || pSession->pCurRoamProfile->ChannelInfo.ChannelList[0] == 0)
		/* try lostlink scan3 */
		return csr_scan_request_lost_link3(pMac, sessionId);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS csr_scan_handle_failed_lostlink3(tpAniSirGlobal pMac,
					    uint32_t sessionId)
{
	sme_debug("Lost link scan 3 failed");
	return QDF_STATUS_SUCCESS;
}
static QDF_STATUS
csr_update_lost_link1_cmd(tpAniSirGlobal mac_ctx, tSmeCmd *cmd,
			  tCsrRoamSession *pSession, uint32_t session_id)
{
	uint8_t i, num_ch = 0;
	tScanResultHandle bss_lst = NULL;
	tCsrScanResultInfo *scan_result = NULL;
	tCsrScanResultFilter *scan_filter = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrSSIDs *ssid_list = &cmd->u.scanCmd.u.scanRequest.SSIDs;
	tCsrChannelInfo *ch_info = &cmd->u.scanCmd.u.scanRequest.ChannelInfo;

	cmd->command = eSmeCommandScan;
	cmd->sessionId = (uint8_t) session_id;
	cmd->u.scanCmd.reason = eCsrScanLostLink1;
	cmd->u.scanCmd.callback = NULL;
	cmd->u.scanCmd.pContext = NULL;
	cmd->u.scanCmd.u.scanRequest.maxChnTime =
		mac_ctx->roam.configParam.nActiveMaxChnTime;
	cmd->u.scanCmd.u.scanRequest.minChnTime =
		mac_ctx->roam.configParam.nActiveMinChnTime;
	cmd->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
	wma_get_scan_id(&cmd->u.scanCmd.scanID);
	status = qdf_mc_timer_init(&cmd->u.scanCmd.csr_scan_timer,
			QDF_TIMER_TYPE_SW,
			csr_scan_active_list_timeout_handle, cmd);
	cmd->u.scanCmd.u.scanRequest.scan_id =
		cmd->u.scanCmd.scanID;

	if (pSession->connectedProfile.SSID.length) {
		/*
		 * on error: following memory will be released by call to
		 * csr_release_command_scan in the end
		 */
		ssid_list->SSIDList = qdf_mem_malloc(sizeof(tCsrSSIDInfo));
		if (NULL == ssid_list->SSIDList)
			return QDF_STATUS_E_NOMEM;
		ssid_list->numOfSSIDs = 1;
		qdf_mem_copy(&ssid_list->SSIDList[0].SSID,
			     &pSession->connectedProfile.SSID,
			     sizeof(tSirMacSSid));
	} else
		ssid_list->numOfSSIDs = 0;

	if (!pSession->pCurRoamProfile)
		return QDF_STATUS_SUCCESS;

	scan_filter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == scan_filter)
		return QDF_STATUS_E_NOMEM;

	status = csr_roam_prepare_filter_from_profile(mac_ctx,
			pSession->pCurRoamProfile, scan_filter);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto free_lost_link1_local_mem;

	if (!(QDF_IS_STATUS_SUCCESS(csr_scan_get_result(mac_ctx, scan_filter,
		&bss_lst)) && bss_lst)) {
		if (csr_roam_is_channel_valid(mac_ctx,
			pSession->connectedProfile.operationChannel)) {
			ch_info->ChannelList = qdf_mem_malloc(1);
			if (NULL == ch_info->ChannelList) {
				status = QDF_STATUS_E_NOMEM;
				goto free_lost_link1_local_mem;
			}
			ch_info->ChannelList[0] =
				pSession->connectedProfile.operationChannel;
			ch_info->numOfChannels = 1;
		}
		csr_free_scan_filter(mac_ctx, scan_filter);
		qdf_mem_free(scan_filter);
		return status;
	}

	/* on error: this mem will be released by csr_release_command_scan */
	ch_info->ChannelList = qdf_mem_malloc(WNI_CFG_VALID_CHANNEL_LIST_LEN);
	if (NULL == ch_info->ChannelList) {
		status = QDF_STATUS_E_NOMEM;
		goto free_lost_link1_local_mem;
	}

	scan_result = csr_scan_result_get_next(mac_ctx, bss_lst);
	while (scan_result != NULL && num_ch < WNI_CFG_VALID_CHANNEL_LIST_LEN) {
		for (i = 0; i < num_ch; i++) {
			if (ch_info->ChannelList[i] ==
				scan_result->BssDescriptor.channelId)
				break;
		}
		if (i == num_ch)
			ch_info->ChannelList[num_ch++] =
				scan_result->BssDescriptor.channelId;
		scan_result = csr_scan_result_get_next(mac_ctx, bss_lst);
	}
	/* Include the last connected BSS' channel */
	if (csr_roam_is_channel_valid(mac_ctx,
		pSession->connectedProfile.operationChannel)) {
		for (i = 0; i < num_ch; i++) {
			if (ch_info->ChannelList[i] ==
				pSession->connectedProfile.operationChannel)
				break;
		}
		if (i == num_ch)
			ch_info->ChannelList[num_ch++] =
				pSession->connectedProfile.operationChannel;
	}
	ch_info->numOfChannels = num_ch;
free_lost_link1_local_mem:
	if (scan_filter) {
		csr_free_scan_filter(mac_ctx, scan_filter);
		qdf_mem_free(scan_filter);
	}
	if (bss_lst)
		csr_scan_result_purge(mac_ctx, bss_lst);
	return status;
}
/**
 * csr_scan_request_lost_link1() - start scan on link lost 1
 * @mac_ctx:       mac global context
 * @session_id:    session id
 *
 * Lostlink1 scan is to actively scan the last connected profile's SSID on all
 * matched BSS channels. If no roam profile (it should not), it is like
 * lostlinkscan3
 *
 * Return: status of operation
 */
QDF_STATUS
csr_scan_request_lost_link1(tpAniSirGlobal mac_ctx, uint32_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *cmd = NULL;
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("session %d not found", session_id);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = csr_get_command_buffer(mac_ctx);
	if (!cmd) {
		status = QDF_STATUS_E_RESOURCES;
		goto release_lost_link1_cmd;
	}
	qdf_mem_set(&cmd->u.scanCmd, sizeof(tScanCmd), 0);
	status = csr_update_lost_link1_cmd(mac_ctx, cmd, session, session_id);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto release_lost_link1_cmd;

	qdf_mem_set(&cmd->u.scanCmd.u.scanRequest.bssid,
		    sizeof(struct qdf_mac_addr), 0xFF);
	status = csr_queue_sme_command(mac_ctx, cmd, false);
	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("fail to send message status: %d", status);

release_lost_link1_cmd:
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_warn("failed with status %d", status);
		if (cmd)
			csr_release_command(mac_ctx, cmd);
		status = csr_scan_handle_failed_lostlink1(mac_ctx, session_id);
	}
	return status;
}

static QDF_STATUS
csr_update_lost_link2_cmd(tpAniSirGlobal mac_ctx, tSmeCmd *cmd,
			  uint32_t session_id, tCsrRoamSession *session)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t i, num_ch = 0;
	tScanResultHandle bss_lst = NULL;
	tCsrScanResultInfo *scan_result = NULL;
	tCsrScanResultFilter *scan_fltr = NULL;
	tCsrChannelInfo *ch_info = &cmd->u.scanCmd.u.scanRequest.ChannelInfo;

	cmd->command = eSmeCommandScan;
	cmd->sessionId = (uint8_t) session_id;
	cmd->u.scanCmd.reason = eCsrScanLostLink2;
	cmd->u.scanCmd.callback = NULL;
	cmd->u.scanCmd.pContext = NULL;
	cmd->u.scanCmd.u.scanRequest.maxChnTime =
		mac_ctx->roam.configParam.nActiveMaxChnTime;
	cmd->u.scanCmd.u.scanRequest.minChnTime =
		mac_ctx->roam.configParam.nActiveMinChnTime;
	cmd->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
	wma_get_scan_id(&cmd->u.scanCmd.scanID);
	cmd->u.scanCmd.u.scanRequest.scan_id =
		cmd->u.scanCmd.scanID;
	if (!session->pCurRoamProfile)
		return QDF_STATUS_SUCCESS;
	status = qdf_mc_timer_init(&cmd->u.scanCmd.csr_scan_timer,
			QDF_TIMER_TYPE_SW,
			csr_scan_active_list_timeout_handle, cmd);
	scan_fltr = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == scan_fltr)
		return QDF_STATUS_E_NOMEM;

	qdf_mem_set(scan_fltr, sizeof(tCsrScanResultFilter), 0);
	status = csr_roam_prepare_filter_from_profile(mac_ctx,
				session->pCurRoamProfile, scan_fltr);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto free_lost_link2_local_mem;

	status = csr_scan_get_result(mac_ctx, scan_fltr, &bss_lst);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto free_lost_link2_local_mem;

	if (!bss_lst)
		goto free_lost_link2_local_mem;

	ch_info->ChannelList = qdf_mem_malloc(WNI_CFG_VALID_CHANNEL_LIST_LEN);
	if (NULL == ch_info->ChannelList) {
		status = QDF_STATUS_E_NOMEM;
		goto free_lost_link2_local_mem;
	}
	scan_result = csr_scan_result_get_next(mac_ctx, bss_lst);
	while (scan_result != NULL && num_ch < WNI_CFG_VALID_CHANNEL_LIST_LEN) {
		for (i = 0; i < num_ch; i++) {
			if (ch_info->ChannelList[i] ==
				scan_result->BssDescriptor.channelId)
				break;
		}
		if (i == num_ch)
			ch_info->ChannelList[num_ch++] =
				scan_result->BssDescriptor.channelId;
		scan_result = csr_scan_result_get_next(mac_ctx, bss_lst);
	}
	ch_info->numOfChannels = num_ch;

free_lost_link2_local_mem:
	if (scan_fltr) {
		csr_free_scan_filter(mac_ctx, scan_fltr);
		qdf_mem_free(scan_fltr);
	}
	if (bss_lst)
		csr_scan_result_purge(mac_ctx, bss_lst);
	return status;
}

/**
 * csr_scan_request_lost_link2() - start scan on link lost 2
 * @mac_ctx:       mac global context
 * @session_id:    session id
 *
 * Lostlink2 scan is to actively scan the all SSIDs of the last roaming
 * profile's on all matched BSS channels. Since MAC doesn't support multiple
 * SSID, we scan all SSIDs and filter them afterwards
 *
 * Return: status of operation
 */
QDF_STATUS
csr_scan_request_lost_link2(tpAniSirGlobal mac_ctx, uint32_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *cmd = NULL;
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!session) {
		sme_err("session %d not found", session_id);
		return QDF_STATUS_E_FAILURE;
	}

	cmd = csr_get_command_buffer(mac_ctx);
	if (!cmd) {
		status = QDF_STATUS_E_RESOURCES;
		goto release_lost_link2_cmd;
	}
	qdf_mem_set(&cmd->u.scanCmd, sizeof(tScanCmd), 0);
	status = csr_update_lost_link2_cmd(mac_ctx, cmd, session_id, session);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto release_lost_link2_cmd;

	qdf_mem_set(&cmd->u.scanCmd.u.scanRequest.bssid,
		    sizeof(struct qdf_mac_addr), 0xFF);
	/* Put to the head in pending queue */
	status = csr_queue_sme_command(mac_ctx, cmd, true);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("fail to send message status: %d", status);
		goto release_lost_link2_cmd;
	}

release_lost_link2_cmd:
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_warn("failed with status %d", status);
		if (cmd)
			csr_release_command(mac_ctx, cmd);
		status = csr_scan_handle_failed_lostlink2(mac_ctx, session_id);
	}
	return status;
}

/**
 * csr_scan_request_lost_link3() - To actively scan all valid channels
 * @mac_ctx:       mac global context
 * @session_id:    session id
 *
 * Return: status of operation
 */
QDF_STATUS
csr_scan_request_lost_link3(tpAniSirGlobal mac_ctx, uint32_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *cmd;

	do {
		cmd = csr_get_command_buffer(mac_ctx);
		if (!cmd) {
			status = QDF_STATUS_E_RESOURCES;
			break;
		}
		qdf_mem_set(&cmd->u.scanCmd, sizeof(tScanCmd), 0);
		cmd->command = eSmeCommandScan;
		cmd->sessionId = (uint8_t) session_id;
		cmd->u.scanCmd.reason = eCsrScanLostLink3;
		cmd->u.scanCmd.callback = NULL;
		cmd->u.scanCmd.pContext = NULL;
		cmd->u.scanCmd.u.scanRequest.maxChnTime =
			mac_ctx->roam.configParam.nActiveMaxChnTime;
		cmd->u.scanCmd.u.scanRequest.minChnTime =
			mac_ctx->roam.configParam.nActiveMinChnTime;
		cmd->u.scanCmd.u.scanRequest.scanType = eSIR_ACTIVE_SCAN;
		wma_get_scan_id(&cmd->u.scanCmd.scanID);
		status = qdf_mc_timer_init(&cmd->u.scanCmd.csr_scan_timer,
			QDF_TIMER_TYPE_SW,
			csr_scan_active_list_timeout_handle, cmd);
		cmd->u.scanCmd.u.scanRequest.scan_id =
			cmd->u.scanCmd.scanID;
		qdf_set_macaddr_broadcast(&cmd->u.scanCmd.u.scanRequest.bssid);
		/* Put to the head of pending queue */
		status = csr_queue_sme_command(mac_ctx, cmd, true);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("fail to send message status: %d", status);
			break;
		}
	} while (0);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_warn("failed with status %d", status);
		if (cmd)
			csr_release_command(mac_ctx, cmd);
	}

	return status;
}
#else
QDF_STATUS
csr_scan_request_lost_link1(tpAniSirGlobal mac_ctx, uint32_t session_id)
{
	return QDF_STATUS_E_FAILURE;
}
QDF_STATUS
csr_scan_request_lost_link3(tpAniSirGlobal mac_ctx, uint32_t session_id)
{
	return QDF_STATUS_E_FAILURE;
}
QDF_STATUS
csr_scan_request_lost_link2(tpAniSirGlobal mac_ctx, uint32_t session_id)
{
	return QDF_STATUS_E_FAILURE;
}
QDF_STATUS csr_scan_handle_failed_lostlink3(tpAniSirGlobal pMac,
					    uint32_t sessionId)
{
	sme_debug("Lost link scan 3 failed");
	return QDF_STATUS_SUCCESS;
}
#endif
#ifdef NAPIER_SCAN
QDF_STATUS csr_scan_handle_search_for_ssid(tpAniSirGlobal mac_ctx,
					   uint32_t session_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tScanResultHandle hBSSList = CSR_INVALID_SCANRESULT_HANDLE;
	tCsrScanResultFilter *pScanFilter = NULL;
	tCsrRoamProfile *profile;
	tCsrRoamSession *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!session) {
		sme_err("session %d not found", session_id);
		return QDF_STATUS_E_FAILURE;
	}
	profile = session->scan_info.profile;
	sme_debug("session %d", session_id);
	do {
		/*
		 * If there is roam command waiting, ignore this roam because
		 * the newer roam command is the one to execute
		 */
		if (csr_is_roam_command_waiting_for_session(mac_ctx, session_id)) {
			sme_warn("aborts because roam command waiting");
			break;
		}
		if (profile == NULL)
			break;
		pScanFilter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
		if (NULL == pScanFilter) {
			status = QDF_STATUS_E_NOMEM;
			break;
		}
		status = csr_roam_prepare_filter_from_profile(mac_ctx, profile,
							      pScanFilter);
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		status = csr_scan_get_result(mac_ctx, pScanFilter, &hBSSList);
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		if (mac_ctx->roam.roamSession[session_id].connectState ==
				eCSR_ASSOC_STATE_TYPE_INFRA_DISCONNECTING) {
			sme_err("upper layer issued disconnetion");
			status = QDF_STATUS_E_FAILURE;
			break;
		}
		status = csr_roam_issue_connect(mac_ctx, session_id, profile,
						hBSSList, eCsrHddIssued,
						session->scan_info.roam_id,
						true, true);
	} while (0);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_INVALID_SCANRESULT_HANDLE != hBSSList) {
			csr_scan_result_purge(mac_ctx, hBSSList);
		}
		/* We haven't done anything to this profile */
		csr_roam_call_callback(mac_ctx, session_id, NULL,
				       session->scan_info.roam_id,
				       eCSR_ROAM_ASSOCIATION_FAILURE,
				       eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE);
	}
	if (pScanFilter) {
		csr_free_scan_filter(mac_ctx, pScanFilter);
		qdf_mem_free(pScanFilter);
	}
	return status;
}

QDF_STATUS csr_scan_handle_search_for_ssid_failure(tpAniSirGlobal mac_ctx,
						  uint32_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamProfile *profile;
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);
	eCsrRoamResult roam_result;

	if (!session) {
		sme_err("session %d not found", session_id);
		return QDF_STATUS_E_FAILURE;
	}
	profile = session->scan_info.profile;

	/*
	 * Check whether it is for start ibss. No need to do anything if it
	 * is a JOIN request
	 */
	if (profile && CSR_IS_START_IBSS(profile)) {
		status = csr_roam_issue_connect(mac_ctx, session_id, profile, NULL,
				eCsrHddIssued, session->scan_info.roam_id,
				true, true);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("failed to issue startIBSS, session_id %d status: 0x%08X roam id %d",
				session_id, status, session->scan_info.roam_id);
			csr_roam_call_callback(mac_ctx, session_id, NULL,
				session->scan_info.roam_id, eCSR_ROAM_FAILED,
				eCSR_ROAM_RESULT_FAILURE);
		}
		return status;
	}
	roam_result = eCSR_ROAM_RESULT_FAILURE;
	if (NULL != profile && csr_is_bss_type_ibss(profile->BSSType)) {
		roam_result = eCSR_ROAM_RESULT_IBSS_START_FAILED;
		goto roam_completion;
	}
	/* Only indicate assoc_completion if we indicate assoc_start. */
	if (session->bRefAssocStartCnt > 0) {
		tCsrRoamInfo *pRoamInfo = NULL, roamInfo;

		qdf_mem_set(&roamInfo, sizeof(tCsrRoamInfo), 0);
		pRoamInfo = &roamInfo;
		if (session->scan_info.roambssentry) {
			tCsrScanResult *pScanResult = GET_BASE_ADDR(
				session->scan_info.roambssentry,
				tCsrScanResult, Link);
			roamInfo.pBssDesc = &pScanResult->Result.BssDescriptor;
		}

		roamInfo.statusCode = session->joinFailStatusCode.statusCode;
		roamInfo.reasonCode = session->joinFailStatusCode.reasonCode;
		session->bRefAssocStartCnt--;
		csr_roam_call_callback(mac_ctx, session_id, pRoamInfo,
				       session->scan_info.roam_id,
				       eCSR_ROAM_ASSOCIATION_COMPLETION,
				       eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE);
	} else {
		csr_roam_call_callback(mac_ctx, session_id, NULL,
				       session->scan_info.roam_id,
				       eCSR_ROAM_ASSOCIATION_FAILURE,
				       eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE);
	}
roam_completion:
	csr_roam_completion(mac_ctx, session_id, NULL, NULL, roam_result,
			    false);
	return status;
}
#else
QDF_STATUS csr_scan_handle_search_for_ssid(tpAniSirGlobal pMac,
					   tSmeCmd *pCommand)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tScanResultHandle hBSSList = CSR_INVALID_SCANRESULT_HANDLE;
	tCsrScanResultFilter *pScanFilter = NULL;
	tCsrRoamProfile *pProfile = pCommand->u.scanCmd.pToRoamProfile;
	uint32_t sessionId = pCommand->sessionId;

	do {
		/* If this scan is for LFR */
		if (pMac->roam.neighborRoamInfo[sessionId].
					uOsRequestedHandoff) {
			/* notify LFR state m/c */
			status = csr_neighbor_roam_sssid_scan_done(pMac,
						sessionId, QDF_STATUS_SUCCESS);
			if (QDF_STATUS_SUCCESS != status)
				csr_neighbor_roam_start_lfr_scan(pMac,
								 sessionId);
			status = QDF_STATUS_SUCCESS;
			break;
		}
		/*
		 * If there is roam command waiting, ignore this roam because
		 * the newer roam command is the one to execute
		 */
		if (csr_is_roam_command_waiting_for_session(pMac, sessionId)) {
			sme_warn("aborts because roam command waiting");
			break;
		}
		if (pProfile == NULL)
			break;
		pScanFilter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
		if (NULL == pScanFilter) {
			status = QDF_STATUS_E_NOMEM;
			break;
		}
		status = csr_roam_prepare_filter_from_profile(pMac, pProfile,
							      pScanFilter);
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		status = csr_scan_get_result(pMac, pScanFilter, &hBSSList);
		if (!QDF_IS_STATUS_SUCCESS(status))
			break;
		if (pMac->roam.roamSession[sessionId].connectState ==
				eCSR_ASSOC_STATE_TYPE_INFRA_DISCONNECTING) {
			sme_err("upper layer issued disconnetion");
			status = QDF_STATUS_E_FAILURE;
			break;
		}
		status = csr_roam_issue_connect(pMac, sessionId, pProfile,
						hBSSList, eCsrHddIssued,
						pCommand->u.scanCmd.roamId,
						true, true);
	} while (0);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		if (CSR_INVALID_SCANRESULT_HANDLE != hBSSList)
			csr_scan_result_purge(pMac, hBSSList);
		/* We haven't done anything to this profile */
		csr_roam_call_callback(pMac, sessionId, NULL,
				       pCommand->u.scanCmd.roamId,
				       eCSR_ROAM_ASSOCIATION_FAILURE,
				       eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE);
	}
	if (pScanFilter) {
		csr_free_scan_filter(pMac, pScanFilter);
		qdf_mem_free(pScanFilter);
	}
	return status;
}

QDF_STATUS csr_scan_handle_search_for_ssid_failure(tpAniSirGlobal pMac,
						   tSmeCmd *pCommand)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t sessionId = pCommand->sessionId;
	tCsrRoamProfile *pProfile = pCommand->u.scanCmd.pToRoamProfile;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	eCsrRoamResult roam_result;

	if (!pSession) {
		sme_err("session %d not found", sessionId);
		return QDF_STATUS_E_FAILURE;
	}
	/* If this scan is for LFR */
	if (pMac->roam.neighborRoamInfo[sessionId].uOsRequestedHandoff) {
		/* notify LFR state m/c */
		status = csr_neighbor_roam_sssid_scan_done(pMac, sessionId,
							QDF_STATUS_E_FAILURE);
		if (QDF_STATUS_SUCCESS != status)
			csr_neighbor_roam_start_lfr_scan(pMac, sessionId);
		return QDF_STATUS_SUCCESS;
	}
#ifdef WLAN_DEBUG
	if (pCommand->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs == 1) {
		char str[36];
		tSirMacSSid *ptr_ssid =
		&pCommand->u.scanCmd.u.scanRequest.SSIDs.SSIDList[0].SSID;
		qdf_mem_copy(str, ptr_ssid->ssId, ptr_ssid->length);
		str[ptr_ssid->length] = 0;
		sme_debug("SSID: %s", str);
	}
#endif
	/*
	 * Check whether it is for start ibss. No need to do anything if it
	 * is a JOIN request
	 */
	if (pProfile && CSR_IS_START_IBSS(pProfile)) {
		status = csr_roam_issue_connect(pMac, sessionId, pProfile, NULL,
				eCsrHddIssued, pCommand->u.scanCmd.roamId,
				true, true);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err(
				"failed to issue startIBSS, status: 0x%08X",
				status);
			csr_roam_call_callback(pMac, sessionId, NULL,
				pCommand->u.scanCmd.roamId, eCSR_ROAM_FAILED,
				eCSR_ROAM_RESULT_FAILURE);
		}
		return status;
	}
	roam_result = eCSR_ROAM_RESULT_FAILURE;
	if (NULL != pProfile && csr_is_bss_type_ibss(pProfile->BSSType)) {
		roam_result = eCSR_ROAM_RESULT_IBSS_START_FAILED;
		goto roam_completion;
	}
	/* Only indicate assoc_completion if we indicate assoc_start. */
	if (pSession->bRefAssocStartCnt > 0) {
		tCsrRoamInfo *pRoamInfo = NULL, roamInfo;

		qdf_mem_set(&roamInfo, sizeof(tCsrRoamInfo), 0);
		pRoamInfo = &roamInfo;
		if (pCommand->u.roamCmd.pRoamBssEntry) {
			tCsrScanResult *pScanResult = GET_BASE_ADDR(
				pCommand->u.roamCmd.pRoamBssEntry,
				tCsrScanResult, Link);
			roamInfo.pBssDesc = &pScanResult->Result.BssDescriptor;
		}
		roamInfo.statusCode = pSession->joinFailStatusCode.statusCode;
		roamInfo.reasonCode = pSession->joinFailStatusCode.reasonCode;
		pSession->bRefAssocStartCnt--;
		csr_roam_call_callback(pMac, sessionId, pRoamInfo,
				       pCommand->u.scanCmd.roamId,
				       eCSR_ROAM_ASSOCIATION_COMPLETION,
				       eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE);
	} else {
		csr_roam_call_callback(pMac, sessionId, NULL,
				       pCommand->u.scanCmd.roamId,
				       eCSR_ROAM_ASSOCIATION_FAILURE,
				       eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE);
	}
roam_completion:
	csr_roam_completion(pMac, sessionId, NULL, pCommand, roam_result,
			    false);
	return status;
}
#endif
QDF_STATUS csr_scan_result_purge(tpAniSirGlobal pMac,
				 tScanResultHandle hScanList)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	tScanResultList *pScanList = (tScanResultList *) hScanList;

	if (pScanList) {
		status = csr_ll_scan_purge_result(pMac, &pScanList->List);
		csr_ll_close(&pScanList->List);
		qdf_mem_free(pScanList);
	}
	return status;
}

/* Add the channel to the occupiedChannels array */
static void csr_scan_add_to_occupied_channels(tpAniSirGlobal pMac,
					tCsrScanResult *pResult,
					uint8_t sessionId,
					tCsrChannel *occupied_ch,
					tDot11fBeaconIEs *pIes,
					bool is_init_list)
{
	QDF_STATUS status;
	uint8_t ch;
	uint8_t num_occupied_ch = occupied_ch->numChannels;
	uint8_t *occupied_ch_lst = occupied_ch->channelList;

	ch = pResult->Result.BssDescriptor.channelId;
	if (!csr_neighbor_roam_connected_profile_match(pMac,
						sessionId, pResult, pIes))
		return;

	if (is_init_list)
		pMac->scan.roam_candidate_count[sessionId]++;

	if (csr_is_channel_present_in_list(occupied_ch_lst,
					   num_occupied_ch, ch))
		return;

	status = csr_add_to_channel_list_front(occupied_ch_lst,
					       num_occupied_ch, ch);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		occupied_ch->numChannels++;
		sme_debug("Added channel %d to the list (count=%d)",
			ch, occupied_ch->numChannels);
		if (occupied_ch->numChannels >
		    CSR_BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN)
			occupied_ch->numChannels =
				CSR_BG_SCAN_OCCUPIED_CHANNEL_LIST_LEN;
	}
}

/* Put the BSS into the scan result list */
/* pIes can not be NULL */
static void csr_scan_add_result(tpAniSirGlobal mac_ctx, tCsrScanResult *pResult,
				tDot11fBeaconIEs *pIes, uint32_t sessionId)
{
	qdf_nbuf_t buf;
	uint8_t *data;
	struct mgmt_rx_event_params rx_param = {0};
	struct wlan_frame_hdr *hdr;
	struct wlan_bcn_frame *fixed_frame;
	uint32_t buf_len;
	tSirBssDescription *bss_desc;
	enum mgmt_frame_type frm_type = MGMT_BEACON;

	bss_desc = &pResult->Result.BssDescriptor;
	if (bss_desc->fProbeRsp)
		frm_type = MGMT_PROBE_RESP;

	rx_param.pdev_id = 0;
	rx_param.channel = bss_desc->channelId;
	rx_param.rssi = bss_desc->rssi;
	rx_param.tsf_delta = bss_desc->tsf_delta;
	buf_len = GET_IE_LEN_IN_BSS(bss_desc->length) +
		+ offsetof(struct wlan_bcn_frame, ie) + sizeof(*hdr);

	buf = qdf_nbuf_alloc(NULL, qdf_roundup(buf_len, 4),
				0, 4, false);
	if (!buf) {
		sme_err("Failed to allocate wbuf for mgmt rx len (%u)",
			buf_len);
		return;
	}
	qdf_nbuf_put_tail(buf, buf_len);
	qdf_nbuf_set_protocol(buf, ETH_P_CONTROL);

	data = qdf_nbuf_data(buf);
	hdr = (struct wlan_frame_hdr *) data;
	qdf_mem_copy(hdr->i_addr3, bss_desc->bssId, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(hdr->i_addr2, bss_desc->bssId, QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(hdr->i_seq,
		&bss_desc->seq_ctrl, sizeof(uint16_t));

	data += sizeof(*hdr);
	fixed_frame = (struct wlan_bcn_frame *)data;
	qdf_mem_copy(fixed_frame->timestamp,
		bss_desc->timeStamp, 8);
	fixed_frame->beacon_interval = bss_desc->beaconInterval;
	fixed_frame->capability.value = bss_desc->capabilityInfo;
	data += offsetof(struct wlan_bcn_frame, ie);

	qdf_mem_copy(data, bss_desc->ieFields,
		GET_IE_LEN_IN_BSS(bss_desc->length));
	tgt_scan_bcn_probe_rx_callback(mac_ctx->psoc, NULL, buf, &rx_param,
		frm_type);
}

/*
 * NOTE: This routine is being added to make
 * sure that scan results are not being flushed
 * while roaming. If the scan results are flushed,
 * we are unable to recover from
 * csr_roam_roaming_state_disassoc_rsp_processor.
 * If it is needed to remove this routine,
 * first ensure that we recover gracefully from
 * csr_roam_roaming_state_disassoc_rsp_processor if
 * csr_scan_get_result returns with a failure because
 * of not being able to find the roaming BSS.
 */
static bool csr_scan_flush_denied(tpAniSirGlobal pMac)
{
	uint8_t sessionId;

	for (sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++) {
		if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
			if (csr_neighbor_middle_of_roaming(pMac, sessionId))
				return 1;
		}
	}
	return 0;
}

QDF_STATUS csr_scanning_state_msg_processor(tpAniSirGlobal pMac,
	void *pMsgBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirMbMsg *pMsg = (tSirMbMsg *) pMsgBuf;
	tCsrRoamSession *pSession;
	tSirSmeAssocIndToUpperLayerCnf *pUpperLayerAssocCnf;
	tCsrRoamInfo roamInfo;
	tCsrRoamInfo *pRoamInfo = NULL;
	uint32_t sessionId;

	if (eWNI_SME_SCAN_RSP == pMsg->type)
		return csr_scan_sme_scan_response(pMac, pMsgBuf);

	if (pMsg->type != eWNI_SME_UPPER_LAYER_ASSOC_CNF) {
		if (csr_is_any_session_in_connect_state(pMac)) {
			/*
			 * In case of we are connected, we need to check whether
			 * connect status changes because scan may also run
			 * while connected.
			 */
			csr_roam_check_for_link_status_change(pMac,
						(tSirSmeRsp *) pMsgBuf);
		} else {
			sme_warn("Message [0x%04x] received in wrong state",
				pMsg->type);
		}
		return status;
	}

	sme_debug("Scanning: ASSOC cnf can be given to upper layer");
	qdf_mem_set(&roamInfo, sizeof(tCsrRoamInfo), 0);
	pRoamInfo = &roamInfo;
	pUpperLayerAssocCnf = (tSirSmeAssocIndToUpperLayerCnf *) pMsgBuf;
	status = csr_roam_get_session_id_from_bssid(pMac,
			(struct qdf_mac_addr *)pUpperLayerAssocCnf->bssId,
			&sessionId);
	pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sme_err("session %d not found", sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	/* send the status code as Success */
	pRoamInfo->statusCode = eSIR_SME_SUCCESS;
	pRoamInfo->u.pConnectedProfile = &pSession->connectedProfile;
	pRoamInfo->staId = (uint8_t) pUpperLayerAssocCnf->aid;
	pRoamInfo->rsnIELen = (uint8_t) pUpperLayerAssocCnf->rsnIE.length;
	pRoamInfo->prsnIE = pUpperLayerAssocCnf->rsnIE.rsnIEdata;
	pRoamInfo->addIELen = (uint8_t) pUpperLayerAssocCnf->addIE.length;
	pRoamInfo->paddIE = pUpperLayerAssocCnf->addIE.addIEdata;
	qdf_mem_copy(pRoamInfo->peerMac.bytes,
			pUpperLayerAssocCnf->peerMacAddr,
			QDF_MAC_ADDR_SIZE);
	qdf_mem_copy(&pRoamInfo->bssid.bytes, pUpperLayerAssocCnf->bssId,
		     QDF_MAC_ADDR_SIZE);
	pRoamInfo->wmmEnabledSta = pUpperLayerAssocCnf->wmmEnabledSta;
	if (CSR_IS_INFRA_AP(pRoamInfo->u.pConnectedProfile)) {
		pMac->roam.roamSession[sessionId].connectState =
			eCSR_ASSOC_STATE_TYPE_INFRA_CONNECTED;
		pRoamInfo->fReassocReq = pUpperLayerAssocCnf->reassocReq;
		status = csr_roam_call_callback(pMac, sessionId,
					pRoamInfo, 0,
					eCSR_ROAM_INFRA_IND,
					eCSR_ROAM_RESULT_INFRA_ASSOCIATION_CNF);
	}
	return status;
}

static QDF_STATUS csr_add_pmkid_candidate_list(tpAniSirGlobal pMac,
					       uint32_t sessionId,
					       tSirBssDescription *pBssDesc,
					       tDot11fBeaconIEs *pIes)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	tPmkidCandidateInfo *pmkid_info = NULL;
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	WLAN_HOST_DIAG_EVENT_DEF(secEvent,
				 host_event_wlan_security_payload_type);
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
	if (!pSession) {
		sme_err("session %d not found", sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	sme_debug("NumPmkidCandidate: %d", pSession->NumPmkidCandidate);
	if (!pIes)
		return status;
		/* check if this is a RSN BSS */
	if (!pIes->RSN.present)
		return status;

	if (pSession->NumPmkidCandidate >= CSR_MAX_PMKID_ALLOWED)
		return QDF_STATUS_E_FAILURE;

	/* BSS is capable of doing pre-authentication */
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	qdf_mem_set(&secEvent, sizeof(host_event_wlan_security_payload_type),
		    0);
	secEvent.eventId = WLAN_SECURITY_EVENT_PMKID_CANDIDATE_FOUND;
	secEvent.encryptionModeMulticast = (uint8_t)diag_enc_type_from_csr_type(
				pSession->connectedProfile.mcEncryptionType);
	secEvent.encryptionModeUnicast = (uint8_t)diag_enc_type_from_csr_type(
				pSession->connectedProfile.EncryptionType);
	qdf_mem_copy(secEvent.bssid, pSession->connectedProfile.bssid.bytes,
			QDF_MAC_ADDR_SIZE);
	secEvent.authMode = (uint8_t)diag_auth_type_from_csr_type(
				pSession->connectedProfile.AuthType);
	WLAN_HOST_DIAG_EVENT_REPORT(&secEvent, EVENT_WLAN_SECURITY);
#endif /* #ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR */

	pmkid_info = &pSession->PmkidCandidateInfo[pSession->NumPmkidCandidate];
	/* if yes, then add to PMKIDCandidateList */
	qdf_mem_copy(pmkid_info->BSSID.bytes, pBssDesc->bssId,
			QDF_MAC_ADDR_SIZE);
	/* Bit 0 offirst byte - PreAuthentication Capability */
	if ((pIes->RSN.RSN_Cap[0] >> 0) & 0x1)
		pmkid_info->preAuthSupported = true;
	else
		pmkid_info->preAuthSupported = false;
	pSession->NumPmkidCandidate++;
	return status;
}
/*
 * This function checks whether new AP is found for the current connected
 * profile. If it is found, it return the sessionId, else it return invalid
 * sessionID
 */
static QDF_STATUS csr_process_bss_desc_for_pmkid_list(tpAniSirGlobal pMac,
						tSirBssDescription *pBssDesc,
						tDot11fBeaconIEs *pIes,
						uint8_t sessionId)
{
	tCsrRoamSession *pSession;
	tDot11fBeaconIEs *pIesLocal = pIes;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!(pIesLocal ||
	    QDF_IS_STATUS_SUCCESS(
		csr_get_parsed_bss_description_ies(pMac, pBssDesc,
						   &pIesLocal))))
		return status;

	if (!CSR_IS_SESSION_VALID(pMac, sessionId)) {
		if (!pIes)
			qdf_mem_free(pIesLocal);
		return status;
	}

	pSession = CSR_GET_SESSION(pMac, sessionId);
	if (csr_is_conn_state_connected_infra(pMac, sessionId)
	    && (eCSR_AUTH_TYPE_RSN == pSession->connectedProfile.AuthType)
	    && csr_match_bss_to_connect_profile(pMac,
				&pSession->connectedProfile,
				pBssDesc, pIesLocal)) {
		/* This new BSS fits the current profile connected */
		status = csr_add_pmkid_candidate_list(pMac, sessionId,
						      pBssDesc, pIesLocal);
		if (!QDF_IS_STATUS_SUCCESS(status))
			sme_err("csr_add_pmkid_candidate_list failed");
		else
			status = QDF_STATUS_SUCCESS;
	}

	if (!pIes)
		qdf_mem_free(pIesLocal);

	return status;
}

#ifdef FEATURE_WLAN_WAPI
static QDF_STATUS csr_add_bkid_candidate_list(tpAniSirGlobal pMac,
					      uint32_t sessionId,
					      tSirBssDescription *pBssDesc,
					      tDot11fBeaconIEs *pIes)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);

	if (!pSession) {
		sme_err("session %d not found", sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	sme_debug("csr_add_bkid_candidate_list called pMac->scan.NumBkidCandidate: %d",
		pSession->NumBkidCandidate);
	if (pIes) {
		/* check if this is a WAPI BSS */
		if (pIes->WAPI.present) {
			/* Check if the BSS is capable of doing
			 * pre-authentication
			 */
			if (pSession->NumBkidCandidate < CSR_MAX_BKID_ALLOWED) {

				/* if yes, then add to BKIDCandidateList */
				qdf_mem_copy(pSession->
					     BkidCandidateInfo[pSession->
							      NumBkidCandidate].
					     BSSID.bytes, pBssDesc->bssId,
					     QDF_MAC_ADDR_SIZE);
				if (pIes->WAPI.preauth) {
					pSession->BkidCandidateInfo[pSession->
							NumBkidCandidate].
					preAuthSupported = true;
				} else {
					pSession->BkidCandidateInfo[pSession->
							NumBkidCandidate].
					preAuthSupported = false;
				}
				pSession->NumBkidCandidate++;
			} else
				status = QDF_STATUS_E_FAILURE;
		}
	}

	return status;
}

/*
 * This function checks whether new AP is found for the current connected
 * profile, if so add to BKIDCandidateList
 */
static bool csr_process_bss_desc_for_bkid_list(tpAniSirGlobal pMac,
					       tSirBssDescription *pBssDesc,
					       tDot11fBeaconIEs *pIes)
{
	bool fRC = false;
	tDot11fBeaconIEs *pIesLocal = pIes;
	uint32_t sessionId;
	tCsrRoamSession *pSession;
	QDF_STATUS status;

	if (!(pIesLocal ||
	    QDF_IS_STATUS_SUCCESS(
		csr_get_parsed_bss_description_ies(pMac, pBssDesc,
						   &pIesLocal))))
		return fRC;

	for (sessionId = 0; sessionId < CSR_ROAM_SESSION_MAX; sessionId++) {
		if (!CSR_IS_SESSION_VALID(pMac, sessionId))
			continue;
		pSession = CSR_GET_SESSION(pMac, sessionId);
		if (csr_is_conn_state_connected_infra(pMac, sessionId)
		    && (eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE ==
			pSession->connectedProfile.AuthType)
		    && csr_match_bss_to_connect_profile(pMac,
				&pSession->connectedProfile,
				pBssDesc, pIesLocal)) {
			/* this new BSS fits the current profile connected */
			status = csr_add_bkid_candidate_list(pMac, sessionId,
							pBssDesc, pIesLocal);
			if (QDF_IS_STATUS_SUCCESS(status))
				fRC = true;
		}
	}
	if (!pIes)
		qdf_mem_free(pIesLocal);
	return fRC;
}

#endif

static tCsrScanResult *csr_scan_save_bss_description(tpAniSirGlobal pMac,
						     tSirBssDescription *
						     pBSSDescription,
						     tDot11fBeaconIEs *pIes,
						     uint8_t sessionId)
{
	tCsrScanResult *pCsrBssDescription = NULL;
	uint32_t cbBSSDesc;
	uint32_t cbAllocated;

	/* figure out how big the BSS description is (the BSSDesc->length does
	 * NOT include the size of the length field itself).
	 */
	cbBSSDesc = pBSSDescription->length + sizeof(pBSSDescription->length);

	cbAllocated = sizeof(tCsrScanResult) + cbBSSDesc;

	pCsrBssDescription = qdf_mem_malloc(cbAllocated);
	if (NULL != pCsrBssDescription) {
		pCsrBssDescription->AgingCount =
			(int32_t) pMac->roam.configParam.agingCount;
		sme_debug(
			"Set Aging Count = %d for BSS " MAC_ADDRESS_STR " ",
			pCsrBssDescription->AgingCount,
			MAC_ADDR_ARRAY(pCsrBssDescription->Result.BssDescriptor.
				       bssId));
		qdf_mem_copy(&pCsrBssDescription->Result.BssDescriptor,
			     pBSSDescription, cbBSSDesc);
#if defined(QDF_ENSBALED)
		if (NULL != pCsrBssDescription->Result.pvIes) {
			QDF_ASSERT(pCsrBssDescription->Result.pvIes == NULL);
			return NULL;
		}
#endif
		csr_scan_add_result(pMac, pCsrBssDescription, pIes, sessionId);
	}

	return pCsrBssDescription;
}

/* Append a Bss Description... */
tCsrScanResult *csr_scan_append_bss_description(tpAniSirGlobal pMac,
						tSirBssDescription *
						pSirBssDescription,
						tDot11fBeaconIEs *pIes,
						bool fForced, uint8_t sessionId)
{
	tCsrScanResult *pCsrBssDescription = NULL;
	pCsrBssDescription = csr_scan_save_bss_description(pMac,
					pSirBssDescription, pIes, sessionId);

	return pCsrBssDescription;
}

static void csr_purge_channel_power(tpAniSirGlobal pMac, tDblLinkList
					*pChannelList)
{
	tCsrChannelPowerInfo *pChannelSet;
	tListElem *pEntry;

	csr_ll_lock(pChannelList);
	/*
	 * Remove the channel sets from the learned list and put them
	 * in the free list
	 */
	while ((pEntry = csr_ll_remove_head(pChannelList,
					    LL_ACCESS_NOLOCK)) != NULL) {
		pChannelSet = GET_BASE_ADDR(pEntry, tCsrChannelPowerInfo, link);
		if (pChannelSet)
			qdf_mem_free(pChannelSet);
	}
	csr_ll_unlock(pChannelList);
}

/*
 * Save the channelList into the ultimate storage as the final stage of channel
 * Input: pCountryInfo -- the country code (e.g. "USI"), channel list, and power
 * limit are all stored inside this data structure
 */
QDF_STATUS csr_save_to_channel_power2_g_5_g(tpAniSirGlobal pMac,
					    uint32_t tableSize,
					    tSirMacChanInfo *channelTable)
{
	uint32_t i = tableSize / sizeof(tSirMacChanInfo);
	tSirMacChanInfo *pChannelInfo;
	tCsrChannelPowerInfo *pChannelSet;
	bool f2GHzInfoFound = false;
	bool f2GListPurged = false, f5GListPurged = false;

	pChannelInfo = channelTable;
	/* atleast 3 bytes have to be remaining  -- from "countryString" */
	while (i--) {
		pChannelSet = qdf_mem_malloc(sizeof(tCsrChannelPowerInfo));
		if (NULL == pChannelSet) {
			pChannelInfo++;
			continue;
		}
		pChannelSet->firstChannel = pChannelInfo->firstChanNum;
		pChannelSet->numChannels = pChannelInfo->numChannels;
		/*
		 * Now set the inter-channel offset based on the frequency band
		 * the channel set lies in
		 */
		if ((WLAN_REG_IS_24GHZ_CH(pChannelSet->firstChannel)) &&
		    ((pChannelSet->firstChannel +
		      (pChannelSet->numChannels - 1)) <=
		     WLAN_REG_MAX_24GHZ_CH_NUM)) {
			pChannelSet->interChannelOffset = 1;
			f2GHzInfoFound = true;
		} else if ((WLAN_REG_IS_5GHZ_CH(pChannelSet->firstChannel))
		    && ((pChannelSet->firstChannel +
		      ((pChannelSet->numChannels - 1) * 4)) <=
		     WLAN_REG_MAX_5GHZ_CH_NUM)) {
			pChannelSet->interChannelOffset = 4;
			f2GHzInfoFound = false;
		} else {
			sme_warn("Invalid Channel %d Present in Country IE",
				pChannelSet->firstChannel);
			qdf_mem_free(pChannelSet);
			return QDF_STATUS_E_FAILURE;
		}
		pChannelSet->txPower = QDF_MIN(pChannelInfo->maxTxPower,
					pMac->roam.configParam.nTxPowerCap);
		if (f2GHzInfoFound) {
			if (!f2GListPurged) {
				/* purge previous results if found new */
				csr_purge_channel_power(pMac,
							&pMac->scan.
							channelPowerInfoList24);
				f2GListPurged = true;
			}
			if (CSR_IS_OPERATING_BG_BAND(pMac)) {
				/* add to the list of 2.4 GHz channel sets */
				csr_ll_insert_tail(&pMac->scan.
						   channelPowerInfoList24,
						   &pChannelSet->link,
						   LL_ACCESS_LOCK);
			} else {
				sme_debug(
					"Adding 11B/G ch in 11A. 1st ch %d",
					pChannelSet->firstChannel);
				qdf_mem_free(pChannelSet);
			}
		} else {
			/* 5GHz info found */
			if (!f5GListPurged) {
				/* purge previous results if found new */
				csr_purge_channel_power(pMac,
							&pMac->scan.
							channelPowerInfoList5G);
				f5GListPurged = true;
			}
			if (CSR_IS_OPERATING_A_BAND(pMac)) {
				/* add to the list of 5GHz channel sets */
				csr_ll_insert_tail(&pMac->scan.
						   channelPowerInfoList5G,
						   &pChannelSet->link,
						   LL_ACCESS_LOCK);
			} else {
				sme_debug(
					"Adding 11A ch in B/G. 1st ch %d",
					pChannelSet->firstChannel);
				qdf_mem_free(pChannelSet);
			}
		}
		pChannelInfo++; /* move to next entry */
	}
	return QDF_STATUS_SUCCESS;
}

static void csr_clear_dfs_channel_list(tpAniSirGlobal pMac)
{
	tSirMbMsg *pMsg;
	uint16_t msgLen;

	msgLen = (uint16_t) (sizeof(tSirMbMsg));
	pMsg = qdf_mem_malloc(msgLen);
	if (NULL != pMsg) {
		pMsg->type = eWNI_SME_CLEAR_DFS_CHANNEL_LIST;
		pMsg->msgLen = msgLen;
		umac_send_mb_message_to_mac(pMsg);
	}
}

void csr_apply_power2_current(tpAniSirGlobal pMac)
{
	sme_debug("Updating Cfg with power settings");
	csr_save_tx_power_to_cfg(pMac, &pMac->scan.channelPowerInfoList24,
				 WNI_CFG_MAX_TX_POWER_2_4);
	csr_save_tx_power_to_cfg(pMac, &pMac->scan.channelPowerInfoList5G,
				 WNI_CFG_MAX_TX_POWER_5);
}

void csr_apply_channel_power_info_to_fw(tpAniSirGlobal mac_ctx,
					  tCsrChannel *ch_lst,
					  uint8_t *countryCode)
{
	int i;
	uint8_t num_ch = 0;
	uint8_t tempNumChannels = 0;
	tCsrChannel tmp_ch_lst;

	if (ch_lst->numChannels) {
		tempNumChannels = CSR_MIN(ch_lst->numChannels,
					  WNI_CFG_VALID_CHANNEL_LIST_LEN);
		for (i = 0; i < tempNumChannels; i++) {
			tmp_ch_lst.channelList[num_ch] = ch_lst->channelList[i];
			num_ch++;
		}
		tmp_ch_lst.numChannels = num_ch;
		/* Store the channel+power info in the global place: Cfg */
		csr_apply_power2_current(mac_ctx);
		csr_set_cfg_valid_channel_list(mac_ctx, tmp_ch_lst.channelList,
					       tmp_ch_lst.numChannels);
		/*
		 * extend scan capability, build a scan list based on the
		 * channel list : channel# + active/passive scan
		 */
		csr_set_cfg_scan_control_list(mac_ctx, countryCode,
					      &tmp_ch_lst);
		/* Send msg to Lim to clear DFS channel list */
		csr_clear_dfs_channel_list(mac_ctx);
	} else {
		sme_err("11D channel list is empty");
	}
	csr_set_cfg_country_code(mac_ctx, countryCode);
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
static void csr_diag_reset_country_information(tpAniSirGlobal pMac)
{

	host_log_802_11d_pkt_type *p11dLog;
	int Index;

	WLAN_HOST_DIAG_LOG_ALLOC(p11dLog, host_log_802_11d_pkt_type,
				 LOG_WLAN_80211D_C);
	if (!p11dLog)
		return;

	p11dLog->eventId = WLAN_80211D_EVENT_RESET;
	qdf_mem_copy(p11dLog->countryCode, pMac->scan.countryCodeCurrent, 3);
	p11dLog->numChannel = pMac->scan.base_channels.numChannels;
	if (p11dLog->numChannel <= HOST_LOG_MAX_NUM_CHANNEL) {
		qdf_mem_copy(p11dLog->Channels,
			     pMac->scan.base_channels.channelList,
			     p11dLog->numChannel);
		for (Index = 0;
		     Index < pMac->scan.base_channels.numChannels;
		     Index++) {
			p11dLog->TxPwr[Index] = QDF_MIN(
				pMac->scan.defaultPowerTable[Index].tx_power,
				pMac->roam.configParam.nTxPowerCap);
		}
	}
	if (!pMac->roam.configParam.Is11dSupportEnabled)
		p11dLog->supportMultipleDomain = WLAN_80211D_DISABLED;
	else
		p11dLog->supportMultipleDomain =
			WLAN_80211D_SUPPORT_MULTI_DOMAIN;
	WLAN_HOST_DIAG_LOG_REPORT(p11dLog);
}
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */

/**
 * csr_apply_channel_power_info_wrapper() - sends channel info to fw
 * @pMac: main MAC data structure
 *
 * This function sends the channel power info to firmware
 *
 * Return: none
 */
void csr_apply_channel_power_info_wrapper(tpAniSirGlobal pMac)
{

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	csr_diag_reset_country_information(pMac);
#endif /* FEATURE_WLAN_DIAG_SUPPORT_CSR */
	csr_prune_channel_list_for_mode(pMac, &pMac->scan.base_channels);
	csr_save_channel_power_for_band(pMac, false);
	csr_save_channel_power_for_band(pMac, true);
	/* apply the channel list, power settings, and the country code. */
	csr_apply_channel_power_info_to_fw(pMac,
		&pMac->scan.base_channels, pMac->scan.countryCodeCurrent);
	/* clear the 11d channel list */
	qdf_mem_set(&pMac->scan.channels11d, sizeof(pMac->scan.channels11d), 0);
}

void csr_clear_votes_for_country_info(tpAniSirGlobal pMac)
{
	pMac->scan.countryCodeCount = 0;
	qdf_mem_set(pMac->scan.votes11d,
		    sizeof(tCsrVotes11d) * CSR_MAX_NUM_COUNTRY_CODE, 0);
}

/**
 * csr_set_country_code() - Set country code
 * @pMac: main MAC data structure
 * @pCountry: ptr to Country Code
 *
 * This function sends the channel power info to firmware
 *
 * Return: none
 */
QDF_STATUS csr_set_country_code(tpAniSirGlobal pMac, uint8_t *pCountry)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	v_REGDOMAIN_t domainId;

	if (pCountry) {

		status = csr_get_regulatory_domain_for_country(pMac, pCountry,
							     &domainId,
							     SOURCE_USERSPACE);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			qdf_mem_copy(pMac->scan.countryCodeCurrent,
				     pCountry,
				     WNI_CFG_COUNTRY_CODE_LEN);
			csr_set_cfg_country_code(pMac, pCountry);
		}
	}
	return status;
}

/* caller allocated memory for pNumChn and pChnPowerInfo */
/* As input, *pNumChn has the size of the array of pChnPowerInfo */
/* Upon return, *pNumChn has the number of channels assigned. */
static void csr_get_channel_power_info(tpAniSirGlobal pMac, tDblLinkList *list,
				       uint32_t *num_ch,
				       struct channel_power *chn_pwr_info)
{
	tListElem *entry;
	uint32_t chn_idx = 0, idx;
	tCsrChannelPowerInfo *ch_set;

	/* Get 2.4Ghz first */
	entry = csr_ll_peek_head(list, LL_ACCESS_LOCK);
	while (entry && (chn_idx < *num_ch)) {
		ch_set = GET_BASE_ADDR(entry, tCsrChannelPowerInfo, link);
		for (idx = 0; (idx < ch_set->numChannels)
				&& (chn_idx < *num_ch); idx++) {
			chn_pwr_info[chn_idx].chan_num =
				(uint8_t) (ch_set->firstChannel
				 + (idx * ch_set->interChannelOffset));
			chn_pwr_info[chn_idx++].tx_power = ch_set->txPower;
		}
		entry = csr_ll_next(list, entry, LL_ACCESS_LOCK);
	}
	*num_ch = chn_idx;

}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
static void csr_diag_apply_country_info(tpAniSirGlobal mac_ctx)
{
	host_log_802_11d_pkt_type *p11dLog;
	struct channel_power chnPwrInfo[WNI_CFG_VALID_CHANNEL_LIST_LEN];
	uint32_t nChnInfo = WNI_CFG_VALID_CHANNEL_LIST_LEN, nTmp;

	WLAN_HOST_DIAG_LOG_ALLOC(p11dLog, host_log_802_11d_pkt_type,
				 LOG_WLAN_80211D_C);
	if (!p11dLog)
		return;

	p11dLog->eventId = WLAN_80211D_EVENT_COUNTRY_SET;
	qdf_mem_copy(p11dLog->countryCode, mac_ctx->scan.countryCode11d, 3);
	p11dLog->numChannel = mac_ctx->scan.channels11d.numChannels;
	if (p11dLog->numChannel > HOST_LOG_MAX_NUM_CHANNEL)
		goto diag_end;

	qdf_mem_copy(p11dLog->Channels,
		     mac_ctx->scan.channels11d.channelList,
		     p11dLog->numChannel);
	csr_get_channel_power_info(mac_ctx,
				&mac_ctx->scan.channelPowerInfoList24,
				&nChnInfo, chnPwrInfo);
	nTmp = nChnInfo;
	nChnInfo = WNI_CFG_VALID_CHANNEL_LIST_LEN - nTmp;
	csr_get_channel_power_info(mac_ctx,
				&mac_ctx->scan.channelPowerInfoList5G,
				&nChnInfo, &chnPwrInfo[nTmp]);
	for (nTmp = 0; nTmp < p11dLog->numChannel; nTmp++) {
		for (nChnInfo = 0;
		     nChnInfo < WNI_CFG_VALID_CHANNEL_LIST_LEN;
		     nChnInfo++) {
			if (p11dLog->Channels[nTmp] ==
			    chnPwrInfo[nChnInfo].chan_num) {
				p11dLog->TxPwr[nTmp] =
					chnPwrInfo[nChnInfo].tx_power;
				break;
			}
		}
	}
diag_end:
	if (!mac_ctx->roam.configParam.Is11dSupportEnabled)
		p11dLog->supportMultipleDomain = WLAN_80211D_DISABLED;
	else
		p11dLog->supportMultipleDomain =
				WLAN_80211D_SUPPORT_MULTI_DOMAIN;
	WLAN_HOST_DIAG_LOG_REPORT(p11dLog);
}
#endif /* #ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR */

/**
 * csr_apply_country_information() - apply country code information
 * @pMac: core MAC data structure
 *
 * This function programs the new country code
 *
 * Return: none
 */
void csr_apply_country_information(tpAniSirGlobal pMac)
{
	v_REGDOMAIN_t domainId;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!csr_is11d_supported(pMac)
	    || 0 == pMac->scan.channelOf11dInfo)
		return;
	status = csr_get_regulatory_domain_for_country(pMac,
			pMac->scan.countryCode11d, &domainId, SOURCE_QUERY);
	if (!QDF_IS_STATUS_SUCCESS(status))
		return;
	/* Check whether we need to enforce default domain */
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	csr_diag_apply_country_info(pMac);
#endif /* #ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR */

	if (pMac->scan.domainIdCurrent != domainId)
		return;
	if (pMac->scan.domainIdCurrent != domainId) {
		sme_debug("Domain Changed Old %d, new %d",
			pMac->scan.domainIdCurrent, domainId);
		if (domainId >= REGDOMAIN_COUNT)
			sme_err("fail to set regId %d", domainId);
	}
	pMac->scan.domainIdCurrent = domainId;
	/* switch to active scans using this new channel list */
	pMac->scan.curScanType = eSIR_ACTIVE_SCAN;
}

void csr_save_channel_power_for_band(tpAniSirGlobal pMac, bool fill_5f)
{
	uint32_t idx, count = 0;
	tSirMacChanInfo *chan_info;
	tSirMacChanInfo *ch_info_start;
	int32_t max_ch_idx;
	bool tmp_bool;
	uint8_t ch = 0;

	max_ch_idx =
		(pMac->scan.base_channels.numChannels <
		WNI_CFG_VALID_CHANNEL_LIST_LEN) ?
		pMac->scan.base_channels.numChannels :
		WNI_CFG_VALID_CHANNEL_LIST_LEN;

	chan_info = qdf_mem_malloc(sizeof(tSirMacChanInfo) *
				   WNI_CFG_VALID_CHANNEL_LIST_LEN);
	if (NULL == chan_info)
		return;

	ch_info_start = chan_info;
	for (idx = 0; idx < max_ch_idx; idx++) {
		ch = pMac->scan.defaultPowerTable[idx].chan_num;
		tmp_bool = (fill_5f && WLAN_REG_IS_5GHZ_CH(ch)) ||
			(!fill_5f && WLAN_REG_IS_24GHZ_CH(ch));
		if (!tmp_bool)
			continue;

		if (count >= WNI_CFG_VALID_CHANNEL_LIST_LEN) {
			sme_warn("count: %d exceeded", count);
			break;
		}

		chan_info->firstChanNum =
			pMac->scan.defaultPowerTable[idx].chan_num;
		chan_info->numChannels = 1;
		chan_info->maxTxPower =
			QDF_MIN(pMac->scan.defaultPowerTable[idx].tx_power,
				pMac->roam.configParam.nTxPowerCap);
		chan_info++;
		count++;
	}
	if (count) {
		csr_save_to_channel_power2_g_5_g(pMac,
				count * sizeof(tSirMacChanInfo), ch_info_start);
	}
	qdf_mem_free(ch_info_start);
}

bool csr_is_supported_channel(tpAniSirGlobal pMac, uint8_t channelId)
{
	bool fRet = false;
	uint32_t i;

	for (i = 0; i < pMac->scan.base_channels.numChannels; i++) {
		if (channelId ==
		    pMac->scan.base_channels.channelList[i]) {
			fRet = true;
			break;
		}
	}

	return fRet;
}

/*
 * 802.11D only: Gather 11d IE via beacon or Probe response and store them in
 * pAdapter->channels11d
 */
bool csr_learn_11dcountry_information(tpAniSirGlobal pMac,
				   tSirBssDescription *pSirBssDesc,
				   tDot11fBeaconIEs *pIes, bool fForce)
{
	QDF_STATUS status;
	uint8_t *pCountryCodeSelected;
	bool fRet = false;
	v_REGDOMAIN_t domainId;
	tDot11fBeaconIEs *pIesLocal = pIes;
	bool useVoting = false;

	if ((NULL == pSirBssDesc) && (NULL == pIes))
		useVoting = true;

	/* check if .11d support is enabled */
	if (!csr_is11d_supported(pMac))
		goto free_ie;

	if (false == useVoting) {
		if (!pIesLocal &&
		   (!QDF_IS_STATUS_SUCCESS(
			csr_get_parsed_bss_description_ies(
				pMac, pSirBssDesc, &pIesLocal))))
			goto free_ie;
		/* check if country information element is present */
		if (!pIesLocal->Country.present)
			/* No country info */
			goto free_ie;
		status = csr_get_regulatory_domain_for_country(pMac,
				pIesLocal->Country.country, &domainId,
				SOURCE_QUERY);
		if (QDF_IS_STATUS_SUCCESS(status)
		    && (domainId == REGDOMAIN_WORLD))
			goto free_ie;
	} /* useVoting == false */

	if (false == useVoting)
		pCountryCodeSelected = pIesLocal->Country.country;
	else
		pCountryCodeSelected = pMac->scan.countryCodeElected;

	if (qdf_mem_cmp(pCountryCodeSelected, pMac->scan.countryCodeCurrent,
			CDS_COUNTRY_CODE_LEN) == 0) {
		qdf_mem_copy(pMac->scan.countryCode11d,
			     pMac->scan.countryCodeCurrent,
			     CDS_COUNTRY_CODE_LEN);
		goto free_ie;
	}

	pMac->reg_hint_src = SOURCE_11D;
	status = csr_get_regulatory_domain_for_country(pMac,
				pCountryCodeSelected, &domainId, SOURCE_11D);
	if (status != QDF_STATUS_SUCCESS) {
		sme_err("fail to get regId %d", domainId);
		fRet = false;
		goto free_ie;
	}

	fRet = true;
free_ie:
	if (!pIes && pIesLocal) {
		/* locally allocated */
		qdf_mem_free(pIesLocal);
	}
	return fRet;
}

void csr_reinit_scan_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	switch (pCommand->u.scanCmd.reason) {
	case eCsrScanAbortNormalScan:
	default:
		csr_scan_free_request(pMac, &pCommand->u.scanCmd.u.scanRequest);
		break;
	}
	if (pCommand->u.scanCmd.pToRoamProfile) {
		csr_release_profile(pMac, pCommand->u.scanCmd.pToRoamProfile);
		qdf_mem_free(pCommand->u.scanCmd.pToRoamProfile);
	}
	qdf_mem_set(&pCommand->u.scanCmd, sizeof(tScanCmd), 0);
}

#ifdef NAPIER_SCAN
static eCsrScanCompleteNextCommand csr_scan_get_next_command_state(
						tpAniSirGlobal mac_ctx,
						uint32_t session_id,
						eCsrScanStatus scan_status,
						uint8_t *chan)
{
	eCsrScanCompleteNextCommand NextCommand = eCsrNextScanNothing;
	int8_t channel;
	tCsrRoamSession *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);
	switch (session->scan_info.scan_reason) {
	case eCsrScanForSsid:
		sme_debug("Resp for Scan For Ssid");
		channel = policy_mgr_search_and_check_for_session_conc(
				mac_ctx->psoc,
				session_id,
				session->scan_info.profile);
		if ((!channel) || scan_status) {
			NextCommand = eCsrNexteScanForSsidFailure;
			sme_err("next Scan For Ssid Failure %d %d",
				channel, scan_status);
		} else {
			NextCommand = eCsrNextCheckAllowConc;
			*chan = channel;
			sme_debug("next CheckAllowConc");
		}
		break;
	default:
		NextCommand = eCsrNextScanNothing;
		break;
	}
	sme_debug("Next Command %d", NextCommand);
	return NextCommand;
}
#else
static eCsrScanCompleteNextCommand csr_scan_get_next_command_state(
							tpAniSirGlobal pMac,
							tSmeCmd *pCommand,
							bool fSuccess,
							uint8_t *chan)
{
	eCsrScanCompleteNextCommand NextCommand = eCsrNextScanNothing;
	int8_t channel;

	switch (pCommand->u.scanCmd.reason) {
	case eCsrScan11d1:
		NextCommand =
			(fSuccess) ? eCsrNext11dScan1Success :
			eCsrNext11dScan1Failure;
		break;
	case eCsrScan11d2:
		NextCommand =
			(fSuccess) ? eCsrNext11dScan2Success :
			eCsrNext11dScan2Failure;
		break;
	case eCsrScan11dDone:
		NextCommand = eCsrNext11dScanComplete;
		break;
	case eCsrScanLostLink1:
		NextCommand =
			(fSuccess) ? eCsrNextLostLinkScan1Success :
			eCsrNextLostLinkScan1Failed;
		break;
	case eCsrScanLostLink2:
		NextCommand =
			(fSuccess) ? eCsrNextLostLinkScan2Success :
			eCsrNextLostLinkScan2Failed;
		break;
	case eCsrScanLostLink3:
		NextCommand =
			(fSuccess) ? eCsrNextLostLinkScan3Success :
			eCsrNextLostLinkScan3Failed;
		break;
	case eCsrScanForSsid:
		/* success:
		 *   set hw_mode success -> csr_scan_handle_search_for_ssid
		 *   set hw_mode fail -> csr_scan_handle_search_for_ssid_failure
		 * failure: csr_scan_handle_search_for_ssid_failure
		 */
		sme_debug("Resp for eCsrScanForSsid");
		channel = policy_mgr_search_and_check_for_session_conc(
				pMac->psoc,
				pCommand->sessionId,
				pCommand->u.scanCmd.pToRoamProfile);
		if ((!channel) || !fSuccess) {
			NextCommand = eCsrNexteScanForSsidFailure;
			sme_debug("next ScanForSsidFailure %d %d",
				channel, fSuccess);
		} else {
			NextCommand = eCsrNextCheckAllowConc;
			*chan = channel;
			sme_debug("next CheckAllowConc");
		}
		break;
	default:
		NextCommand = eCsrNextScanNothing;
		break;
	}
	return NextCommand;
}
#endif

#ifndef NAPIER_SCAN
/* Return whether the pCommand is finished. */
static bool csr_handle_scan11d1_failure(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	bool fRet = true;

	/* Apply back the default setting and passively scan one more time. */
	csr_apply_channel_power_info_wrapper(pMac);
	pCommand->u.scanCmd.reason = eCsrScan11d2;
	if (QDF_IS_STATUS_SUCCESS(csr_scan_channels(pMac, pCommand)))
		fRet = false;

	return fRet;
}
#endif
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
static void
csr_diag_scan_complete(tpAniSirGlobal mac_ctx,
		       eCsrScanStatus scan_status)
{
	host_log_scan_pkt_type *pScanLog = NULL;
	qdf_list_t *list = NULL;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct scan_cache_node *cur_node = NULL;
	struct scan_cache_node *next_node = NULL;
	int n = 0, c = 0;

	WLAN_HOST_DIAG_LOG_ALLOC(pScanLog,
				 host_log_scan_pkt_type,
				 LOG_WLAN_SCAN_C);
	if (!pScanLog)
		return;

	pScanLog->eventId = WLAN_SCAN_EVENT_ACTIVE_SCAN_RSP;

	if (eCSR_SCAN_SUCCESS != scan_status) {
		pScanLog->status = WLAN_SCAN_STATUS_FAILURE;
		WLAN_HOST_DIAG_LOG_REPORT(pScanLog);
		return;
	}
	pdev = wlan_objmgr_get_pdev_by_id(mac_ctx->psoc,
		0, WLAN_LEGACY_MAC_ID);

	if (!pdev) {
		sme_err("pdev is NULL");
		return;
	}

	list = ucfg_scan_get_result(pdev, NULL);
	sme_debug("num_entries %d",
					qdf_list_size(list));
	if (!list || !qdf_list_size(list)) {
		sme_err("get scan result failed");
		WLAN_HOST_DIAG_LOG_REPORT(pScanLog);
		wlan_objmgr_pdev_release_ref(pdev, WLAN_LEGACY_MAC_ID);
		return;
	}

	qdf_list_peek_front(list,
		   (qdf_list_node_t **) &cur_node);
	while (cur_node) {
		if (n < HOST_LOG_MAX_NUM_BSSID) {
			qdf_mem_copy(pScanLog->bssid[n],
				cur_node->entry->bssid.bytes,
				QDF_MAC_ADDR_SIZE);
			if (cur_node->entry->ssid.length >
			   WLAN_SSID_MAX_LEN)
				cur_node->entry->ssid.length =
				  WLAN_SSID_MAX_LEN;
			qdf_mem_copy(pScanLog->ssid[n],
				cur_node->entry->ssid.ssid,
				cur_node->entry->ssid.length);
			n++;
		}
		c++;
		qdf_list_peek_next(
		  list,
		  (qdf_list_node_t *) cur_node,
		  (qdf_list_node_t **) &next_node);
		cur_node = next_node;
		next_node = NULL;
	}
	pScanLog->numSsid = (uint8_t) n;
	pScanLog->totalSsid = (uint8_t) c;
	ucfg_scan_purge_results(list);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_LEGACY_MAC_ID);
	WLAN_HOST_DIAG_LOG_REPORT(pScanLog);
}
#endif /* #ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR */

#ifdef NAPIER_SCAN
/**
 * csr_save_profile() - Save the profile info from sme command
 * @mac_ctx: Global MAC context
 * @save_cmd: Pointer where the command will be saved
 * @command: Command from which the profile will be saved
 *
 * Saves the profile information from the SME's scan command
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS csr_save_profile(tpAniSirGlobal mac_ctx,
				   uint32_t session_id)
{
	tCsrScanResult *scan_result;
	tCsrScanResult *temp;
	uint32_t bss_len;
	tCsrRoamSession *session;

	session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!session->scan_info.roambssentry)
		return QDF_STATUS_SUCCESS;

	scan_result = GET_BASE_ADDR(session->scan_info.roambssentry,
			tCsrScanResult, Link);

	bss_len = scan_result->Result.BssDescriptor.length +
		sizeof(scan_result->Result.BssDescriptor.length);

	temp = qdf_mem_malloc(sizeof(tCsrScanResult) + bss_len);
	if (!temp) {
		sme_err("bss mem fail");
		goto error;
	}

	temp->AgingCount = scan_result->AgingCount;
	temp->preferValue = scan_result->preferValue;
	temp->capValue = scan_result->capValue;
	temp->ucEncryptionType = scan_result->ucEncryptionType;
	temp->mcEncryptionType = scan_result->mcEncryptionType;
	temp->authType = scan_result->authType;
	/* pvIes is unsued in success/failure */
	temp->Result.pvIes = NULL;

	qdf_mem_copy(temp->Result.pvIes,
			scan_result->Result.pvIes,
			sizeof(*scan_result->Result.pvIes));
	temp->Result.ssId.length = scan_result->Result.ssId.length;
	qdf_mem_copy(temp->Result.ssId.ssId,
			scan_result->Result.ssId.ssId,
			sizeof(scan_result->Result.ssId.ssId));
	temp->Result.timer = scan_result->Result.timer;
	qdf_mem_copy(&temp->Result.BssDescriptor,
			&scan_result->Result.BssDescriptor,
			sizeof(temp->Result.BssDescriptor));
	temp->Link.last = temp->Link.next = NULL;
	session->scan_info.roambssentry = (tListElem *)temp;

	return QDF_STATUS_SUCCESS;
error:
	csr_scan_handle_search_for_ssid_failure(mac_ctx,
			session_id);
	if (session->scan_info.roambssentry) {
		qdf_mem_free(session->scan_info.roambssentry);
		session->scan_info.roambssentry = NULL;
	}
	if (session->scan_info.profile) {
		csr_release_profile(mac_ctx,
				    session->scan_info.profile);
		qdf_mem_free(session->scan_info.profile);
		session->scan_info.profile = NULL;
	}

	return QDF_STATUS_E_FAILURE;
}

static void csr_handle_nxt_cmd(tpAniSirGlobal mac_ctx,
		   eCsrScanCompleteNextCommand nxt_cmd,
		   uint32_t session_id,
		   uint8_t chan)
{
	QDF_STATUS status, ret;

	switch (nxt_cmd) {

	case eCsrNexteScanForSsidSuccess:
		csr_scan_handle_search_for_ssid(mac_ctx, session_id);
		break;
	case eCsrNexteScanForSsidFailure:
		csr_scan_handle_search_for_ssid_failure(mac_ctx, session_id);
		break;
	case eCsrNextCheckAllowConc:
		ret = policy_mgr_current_connections_update(mac_ctx->psoc,
					session_id, chan,
					SIR_UPDATE_REASON_HIDDEN_STA);
		sme_debug("chan: %d session: %d status: %d",
					chan, session_id, ret);

		status = csr_save_profile(mac_ctx, session_id);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			/* csr_save_profile should report error */
			sme_err("profile save failed %d", status);
			return;
		}

		if (QDF_STATUS_E_FAILURE == ret) {
			sme_err("conn update fail %d", chan);
			csr_scan_handle_search_for_ssid_failure(mac_ctx,
								session_id);
		} else if ((QDF_STATUS_E_NOSUPPORT == ret) ||
			(QDF_STATUS_E_ALREADY == ret)) {
			sme_err("conn update ret %d", ret);
			csr_scan_handle_search_for_ssid(mac_ctx, session_id);
		}
		/* Else: Set hw mode was issued and the saved connect would
		 * be issued after set hw mode response
		 */
		break;
	default:
		break;
	}
}
#else
/**
 * csr_save_profile() - Save the profile info from sme command
 * @mac_ctx: Global MAC context
 * @save_cmd: Pointer where the command will be saved
 * @command: Command from which the profile will be saved
 *
 * Saves the profile information from the SME's scan command
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS csr_save_profile(tpAniSirGlobal mac_ctx,
				   tSmeCmd *save_cmd, tSmeCmd *command)
{
	tCsrScanResult *scan_result;
	tCsrScanResult *temp;
	uint32_t bss_len;
	QDF_STATUS status;

	save_cmd->u.scanCmd.pToRoamProfile =
		qdf_mem_malloc(sizeof(tCsrRoamProfile));
	if (!save_cmd->u.scanCmd.pToRoamProfile) {
		sme_err("pToRoamProfile mem fail");
		goto error;
	}

	status = csr_roam_copy_profile(mac_ctx,
			save_cmd->u.scanCmd.pToRoamProfile,
			command->u.scanCmd.pToRoamProfile);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("csr_roam_copy_profile fail");
		goto error;
	}

	save_cmd->sessionId = command->sessionId;
	save_cmd->u.scanCmd.roamId = command->u.scanCmd.roamId;
	save_cmd->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs =
		command->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs;
	save_cmd->u.scanCmd.u.scanRequest.SSIDs.SSIDList =
		qdf_mem_malloc(
			save_cmd->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs *
			sizeof(tCsrSSIDInfo));
	if (!save_cmd->u.scanCmd.u.scanRequest.SSIDs.SSIDList) {
		sme_err("SSIDList mem fail");
		goto error;
	}

	qdf_mem_copy(save_cmd->u.scanCmd.u.scanRequest.SSIDs.SSIDList,
			command->u.scanCmd.u.scanRequest.SSIDs.SSIDList,
			save_cmd->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs *
			sizeof(tCsrSSIDInfo));

	if (!command->u.roamCmd.pRoamBssEntry)
		return QDF_STATUS_SUCCESS;

	scan_result = GET_BASE_ADDR(command->u.roamCmd.pRoamBssEntry,
			tCsrScanResult, Link);

	bss_len = scan_result->Result.BssDescriptor.length +
		sizeof(scan_result->Result.BssDescriptor.length);

	temp = qdf_mem_malloc(sizeof(tCsrScanResult) + bss_len);
	if (!temp) {
		sme_err("bss mem fail");
		goto error;
	}

	temp->AgingCount = scan_result->AgingCount;
	temp->preferValue = scan_result->preferValue;
	temp->capValue = scan_result->capValue;
	temp->ucEncryptionType = scan_result->ucEncryptionType;
	temp->mcEncryptionType = scan_result->mcEncryptionType;
	temp->authType = scan_result->authType;
	/* pvIes is unsued in success/failure */
	temp->Result.pvIes = NULL;

	qdf_mem_copy(temp->Result.pvIes,
			scan_result->Result.pvIes,
			sizeof(*scan_result->Result.pvIes));
	temp->Result.ssId.length = scan_result->Result.ssId.length;
	qdf_mem_copy(temp->Result.ssId.ssId,
			scan_result->Result.ssId.ssId,
			sizeof(scan_result->Result.ssId.ssId));
	temp->Result.timer = scan_result->Result.timer;
	qdf_mem_copy(&temp->Result.BssDescriptor,
			&scan_result->Result.BssDescriptor,
			sizeof(temp->Result.BssDescriptor));
	temp->Link.last = temp->Link.next = NULL;
	save_cmd->u.roamCmd.pRoamBssEntry = (tListElem *)temp;

	return QDF_STATUS_SUCCESS;
error:
	csr_scan_handle_search_for_ssid_failure(mac_ctx,
			command);
	if (save_cmd->u.roamCmd.pRoamBssEntry)
		qdf_mem_free(save_cmd->u.roamCmd.pRoamBssEntry);
	if (save_cmd->u.scanCmd.u.scanRequest.SSIDs.SSIDList)
		qdf_mem_free(save_cmd->u.scanCmd.u.scanRequest.SSIDs.SSIDList);
	if (save_cmd->u.scanCmd.pToRoamProfile) {
		csr_release_profile(mac_ctx,
				    save_cmd->u.scanCmd.pToRoamProfile);
		qdf_mem_free(save_cmd->u.scanCmd.pToRoamProfile);
	}

	return QDF_STATUS_E_FAILURE;
}
static void
csr_handle_nxt_cmd(tpAniSirGlobal mac_ctx, tSmeCmd *pCommand,
		   eCsrScanCompleteNextCommand *nxt_cmd,
		   bool *remove_cmd, uint32_t session_id,
		   uint8_t chan)
{
	QDF_STATUS status, ret;
	tSmeCmd *save_cmd = NULL;
	tSmeCmd *saved_scan_cmd;

	switch (*nxt_cmd) {
	case eCsrNext11dScan1Success:
	case eCsrNext11dScan2Success:
		sme_debug(
			"11dScan1/3 produced results. Reissue Active scan");
		/*
		 * if we found country information, no need to continue scanning
		 * further, bail out
		 */
		*remove_cmd = true;
		*nxt_cmd = eCsrNext11dScanComplete;
		break;
	case eCsrNext11dScan1Failure:
		/*
		 * We are not done yet. 11d scan fail once. We will try to reset
		 * anything and do it over again. The only meaningful thing for
		 * this retry is that we cannot find 11d information after a
		 * reset so we clear the "old" 11d info and give it once more
		 * chance
		 */
		*remove_cmd = csr_handle_scan11d1_failure(mac_ctx, pCommand);
		if (*remove_cmd)
			*nxt_cmd = eCsrNext11dScanComplete;
		break;
	case eCsrNextLostLinkScan1Success:
		status = csr_issue_roam_after_lostlink_scan(mac_ctx, session_id,
							    eCsrLostLink1);
		if (!QDF_IS_STATUS_SUCCESS(status))
			csr_scan_handle_failed_lostlink1(mac_ctx, session_id);
		break;
	case eCsrNextLostLinkScan2Success:
		status = csr_issue_roam_after_lostlink_scan(mac_ctx, session_id,
							    eCsrLostLink2);
		if (!QDF_IS_STATUS_SUCCESS(status))
			csr_scan_handle_failed_lostlink2(mac_ctx, session_id);
		break;
	case eCsrNextLostLinkScan3Success:
		status = csr_issue_roam_after_lostlink_scan(mac_ctx, session_id,
							    eCsrLostLink3);
		if (!QDF_IS_STATUS_SUCCESS(status))
			csr_scan_handle_failed_lostlink3(mac_ctx, session_id);
		break;
	case eCsrNextLostLinkScan1Failed:
		csr_scan_handle_failed_lostlink1(mac_ctx, session_id);
		break;
	case eCsrNextLostLinkScan2Failed:
		csr_scan_handle_failed_lostlink2(mac_ctx, session_id);
		break;
	case eCsrNextLostLinkScan3Failed:
		csr_scan_handle_failed_lostlink3(mac_ctx, session_id);
		break;
	case eCsrNexteScanForSsidSuccess:
		csr_scan_handle_search_for_ssid(mac_ctx, pCommand);
		break;
	case eCsrNexteScanForSsidFailure:
		csr_scan_handle_search_for_ssid_failure(mac_ctx, pCommand);
		break;
	case eCsrNextCheckAllowConc:
		ret = policy_mgr_current_connections_update(mac_ctx->psoc,
					pCommand->sessionId, chan,
					SIR_UPDATE_REASON_HIDDEN_STA);
		sme_debug("chan: %d session: %d status: %d",
					chan, pCommand->sessionId, ret);
		saved_scan_cmd = (tSmeCmd *)mac_ctx->sme.saved_scan_cmd;
		if (saved_scan_cmd) {
			csr_release_profile(mac_ctx, saved_scan_cmd->u.scanCmd.
					    pToRoamProfile);
			if (saved_scan_cmd->u.scanCmd.pToRoamProfile) {
				qdf_mem_free(saved_scan_cmd->u.scanCmd.
					     pToRoamProfile);
				saved_scan_cmd->u.scanCmd.
					pToRoamProfile = NULL;
			}
			if (saved_scan_cmd->u.scanCmd.u.scanRequest.SSIDs.
			    SSIDList) {
				qdf_mem_free(saved_scan_cmd->u.scanCmd.u.
					     scanRequest.SSIDs.SSIDList);
				saved_scan_cmd->u.scanCmd.u.scanRequest.SSIDs.
					SSIDList = NULL;
			}
			if (saved_scan_cmd->u.roamCmd.pRoamBssEntry) {
				qdf_mem_free(saved_scan_cmd->u.roamCmd.
					     pRoamBssEntry);
				saved_scan_cmd->u.roamCmd.pRoamBssEntry = NULL;
			}
			qdf_mem_free(saved_scan_cmd);
			saved_scan_cmd = NULL;
			sme_err(FL("memory should have been free. Check!"));
		}

		save_cmd = (tSmeCmd *) qdf_mem_malloc(sizeof(*pCommand));
		if (!save_cmd) {
			sme_err("save_cmd mem fail");
			goto error;
		}

		status = csr_save_profile(mac_ctx, save_cmd, pCommand);
		if (!QDF_IS_STATUS_SUCCESS(status)) {
			sme_err("profile save failed %d", status);
			qdf_mem_free(save_cmd);
			return;
		}

		mac_ctx->sme.saved_scan_cmd = (void *)save_cmd;

		if (QDF_STATUS_E_FAILURE == ret) {
error:
			sme_err("conn update fail %d", chan);
			csr_scan_handle_search_for_ssid_failure(mac_ctx,
								pCommand);
			if (mac_ctx->sme.saved_scan_cmd) {
				qdf_mem_free(mac_ctx->sme.saved_scan_cmd);
				mac_ctx->sme.saved_scan_cmd = NULL;
			}
		} else if ((QDF_STATUS_E_NOSUPPORT == ret) ||
			(QDF_STATUS_E_ALREADY == ret)) {
			sme_err("conn update ret %d", ret);
			csr_scan_handle_search_for_ssid(mac_ctx, pCommand);
			if (mac_ctx->sme.saved_scan_cmd) {
				qdf_mem_free(mac_ctx->sme.saved_scan_cmd);
				mac_ctx->sme.saved_scan_cmd = NULL;
			}
		}
		/* Else: Set hw mode was issued and the saved connect would
		 * be issued after set hw mode response
		 */
		break;
	default:
		break;
	}
}

#endif
#ifndef NAPIER_SCAN
/**
 * csr_get_active_scan_entry() - To get scan entry from active command list
 *
 * @mac_ctx - MAC context
 * @scan_id - Scan identifier of the scan request
 * @entry - scan entry returned.
 *
 * Scan entry in the active scan list mapping to the sent scan id
 * is returned to the caller.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS csr_get_active_scan_entry(tpAniSirGlobal mac_ctx,
	uint32_t scan_id, tListElem **entry)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tListElem *localentry;
	tSmeCmd *cmd;
	uint32_t cmd_scan_id = 0;

	csr_scan_active_ll_lock(mac_ctx);

	if (csr_scan_active_ll_is_list_empty(mac_ctx, LL_ACCESS_NOLOCK)) {
		sme_err("Active list Empty scanId: %d", scan_id);
		csr_scan_active_ll_unlock(mac_ctx);
		return QDF_STATUS_SUCCESS;
	}

	localentry = csr_scan_active_ll_peek_head(mac_ctx,
			LL_ACCESS_NOLOCK);
	while (localentry) {
		cmd = GET_BASE_ADDR(localentry, tSmeCmd, Link);
		if (cmd->command == eSmeCommandScan)
			cmd_scan_id = cmd->u.scanCmd.u.scanRequest.scan_id;
		else if (cmd->command == eSmeCommandRemainOnChannel)
			cmd_scan_id = cmd->u.remainChlCmd.scan_id;
		if (cmd_scan_id == scan_id) {
			sme_debug("scanId Matched %d", scan_id);
			*entry = localentry;
			csr_scan_active_ll_unlock(mac_ctx);
			return QDF_STATUS_SUCCESS;
		}
		localentry = csr_scan_active_ll_next(mac_ctx,
				localentry, LL_ACCESS_NOLOCK);
	}
	csr_scan_active_ll_unlock(mac_ctx);
	return status;
}

#else

/* API will be removed after p2p component L0 */
QDF_STATUS csr_get_active_scan_entry(tpAniSirGlobal mac_ctx,
	uint32_t scan_id, tListElem **entry)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct wlan_serialization_command *cmn_cmd;
	tSmeCmd *sme_cmd;
	uint32_t cmd_scan_id = 0;

	csr_scan_active_ll_lock(mac_ctx);

	if (csr_scan_active_ll_is_list_empty(mac_ctx, LL_ACCESS_NOLOCK)) {
		sme_err("Active list Empty scanId: %d", scan_id);
		csr_scan_active_ll_unlock(mac_ctx);
		return QDF_STATUS_SUCCESS;
	}
	cmn_cmd = wlan_serialization_peek_head_active_cmd_using_psoc(
			mac_ctx->psoc, true);
	while (cmn_cmd) {
		sme_cmd = cmn_cmd->umac_cmd;
		if (cmn_cmd->cmd_type == WLAN_SER_CMD_SCAN)
			cmd_scan_id = cmn_cmd->cmd_id;
		else if (cmn_cmd->cmd_type == WLAN_SER_CMD_REMAIN_ON_CHANNEL)
			cmd_scan_id = sme_cmd->u.remainChlCmd.scan_id;
		sme_debug("cmd_scan_id %d", cmd_scan_id);
		if ((cmn_cmd->cmd_type == WLAN_SER_CMD_REMAIN_ON_CHANNEL) &&
			(cmd_scan_id == scan_id)) {
			sme_debug("scanId Matched %d", scan_id);
			*entry = &sme_cmd->Link;
			csr_scan_active_ll_unlock(mac_ctx);
			return QDF_STATUS_SUCCESS;
		}
		cmn_cmd =
			wlan_serialization_get_active_list_next_node_using_psoc(
				mac_ctx->psoc, cmn_cmd, true);
	}
	csr_scan_active_ll_unlock(mac_ctx);

	return status;
}
#endif

void csr_scan_callback(struct wlan_objmgr_vdev *vdev,
				struct scan_event *event, void *arg)
{
	eCsrScanStatus scan_status = eCSR_SCAN_FAILURE;
	eCsrScanCompleteNextCommand NextCommand = eCsrNextScanNothing;
	tpAniSirGlobal mac_ctx;
	tCsrRoamSession *session;
	uint32_t session_id = 0;
	uint8_t chan = 0;

	mac_ctx = (tpAniSirGlobal)arg;
	if ((event->type == SCAN_EVENT_TYPE_COMPLETED) &&
			((event->reason == SCAN_REASON_CANCELLED) ||
			(event->reason == SCAN_REASON_TIMEDOUT) ||
			(event->reason == SCAN_REASON_INTERNAL_FAILURE)))
		scan_status = eCSR_SCAN_FAILURE;
	else if ((event->type == SCAN_EVENT_TYPE_COMPLETED) &&
			(event->reason == SCAN_REASON_COMPLETED))
		scan_status = eCSR_SCAN_SUCCESS;
	else
		return;

	session_id = wlan_vdev_get_id(vdev);
	session = CSR_GET_SESSION(mac_ctx, session_id);

	sme_debug("Scan Completion: status %d session %d scan_id %d",
			scan_status, session_id, event->scan_id);

	/* verify whether scan event is related to scan interested by CSR */
	if (session->scan_info.scan_id != event->scan_id) {
		sme_debug("Scan Completion on wrong scan_id %d, expected %d",
			session->scan_info.scan_id, event->scan_id);
		return;
	}
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
	csr_diag_scan_complete(mac_ctx, scan_status);
#endif
	NextCommand = csr_scan_get_next_command_state(mac_ctx,
					session_id, scan_status,
					&chan);
	/* We reuse the command here instead reissue a new command */
	csr_handle_nxt_cmd(mac_ctx, NextCommand,
			   session_id, chan);
}
bool csr_scan_complete(tpAniSirGlobal pMac, tSirSmeScanRsp *pScanRsp)
{
	return true;
}

/* Return whether last scan result is received */
static bool csr_scan_process_scan_results(tpAniSirGlobal pMac, tSmeCmd
					*pCommand, tSirSmeScanRsp *pScanRsp,
					  bool *pfRemoveCommand)
{
	bool fRet = false, fRemoveCommand = false;
	QDF_STATUS status;

	sme_debug("scan reason = %d, response status code = %d",
		pCommand->u.scanCmd.reason, pScanRsp->statusCode);
	fRemoveCommand = csr_scan_complete(pMac, pScanRsp);
	fRet = true;
	if (pfRemoveCommand)
		*pfRemoveCommand = fRemoveCommand;

	/*
	 * Currently SET_FCC_CHANNEL issues updated channel list to fw.
	 * At the time of driver load, if scan is issued followed with
	 * SET_FCC_CHANNEL, driver will send update channel list to fw.
	 * Fw will stop ongoing scan because of that GUI will have very less
	 * scan list.
	 * Update channel list should be sent to fw once scan is done
	 */
	if (pMac->scan.defer_update_channel_list) {
		status = csr_update_channel_list(pMac);
		if (!QDF_IS_STATUS_SUCCESS(status))
			sme_err("failed to update the supported channel list");
		pMac->scan.defer_update_channel_list = false;
	}

	return fRet;
}

static bool csr_scan_is_wild_card_scan(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	uint8_t bssid[QDF_MAC_ADDR_SIZE] = {0};
	/*
	 * It is not a wild card scan if the bssid is not broadcast and
	 * the number of SSID is 1.
	 */
	return ((!qdf_mem_cmp(pCommand->u.scanCmd.u.scanRequest.bssid.bytes,
					bssid, sizeof(struct qdf_mac_addr)))
		|| (0xff == pCommand->u.scanCmd.u.scanRequest.bssid.bytes[0]))
		&& (pCommand->u.scanCmd.u.scanRequest.SSIDs.numOfSSIDs != 1);
}

QDF_STATUS csr_scan_sme_scan_response(tpAniSirGlobal pMac,
		void *pMsgBuf)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tListElem *pEntry = NULL;
	tSmeCmd *pCommand;
	eCsrScanStatus scanStatus;
	tSirSmeScanRsp *pScanRsp = (tSirSmeScanRsp *)pMsgBuf;
	bool fRemoveCommand = true;
	eCsrScanReason reason = eCsrScanOther;

	csr_get_active_scan_entry(pMac, pScanRsp->scan_id, &pEntry);
	if (!pEntry)
		goto error_handling;

	sme_debug("Scan completion called:scan_id %d, entry = %p",
		pScanRsp->scan_id, pEntry);

	pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
	if (eSmeCommandScan != pCommand->command)
		goto error_handling;

	scanStatus = (eSIR_SME_SUCCESS == pScanRsp->statusCode) ?
			eCSR_SCAN_SUCCESS : eCSR_SCAN_FAILURE;
	reason = pCommand->u.scanCmd.reason;
	switch (pCommand->u.scanCmd.reason) {
	case eCsrScanAbortNormalScan:
		break;
	case eCsrScanP2PFindPeer:
		scanStatus = (eSIR_SME_SUCCESS == pScanRsp->statusCode) ?
				eCSR_SCAN_FOUND_PEER : eCSR_SCAN_FAILURE;
		csr_scan_process_scan_results(pMac, pCommand, pScanRsp, NULL);
		break;
	default:
		if (csr_scan_process_scan_results(pMac, pCommand, pScanRsp,
						  &fRemoveCommand)
		    && csr_scan_is_wild_card_scan(pMac, pCommand)
		    && !pCommand->u.scanCmd.u.scanRequest.p2pSearch) {

		/* Age out logic will be taken care by the age out timer */
		}
		break;
	}
	if (fRemoveCommand) {
		pCommand->u.scanCmd.status = scanStatus;
		csr_release_command(pMac, pCommand);
	}
	return status;

error_handling:
	sme_err("Scan completion called, but no active SCAN command");
	return QDF_STATUS_E_FAILURE;
}

tCsrScanResultInfo *csr_scan_result_get_first(tpAniSirGlobal pMac,
					      tScanResultHandle hScanResult)
{
	tListElem *pEntry;
	tCsrScanResult *pResult;
	tCsrScanResultInfo *pRet = NULL;
	tScanResultList *pResultList = (tScanResultList *) hScanResult;

	if (pResultList) {
		csr_ll_lock(&pResultList->List);
		pEntry = csr_ll_peek_head(&pResultList->List, LL_ACCESS_NOLOCK);
		if (pEntry) {
			pResult = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
			pRet = &pResult->Result;
		}
		pResultList->pCurEntry = pEntry;
		csr_ll_unlock(&pResultList->List);
	}

	return pRet;
}

tCsrScanResultInfo *csr_scan_result_get_next(tpAniSirGlobal pMac,
					     tScanResultHandle hScanResult)
{
	tListElem *pEntry = NULL;
	tCsrScanResult *pResult = NULL;
	tCsrScanResultInfo *pRet = NULL;
	tScanResultList *pResultList = (tScanResultList *) hScanResult;

	if (!pResultList)
		return NULL;

	csr_ll_lock(&pResultList->List);
	if (NULL == pResultList->pCurEntry)
		pEntry = csr_ll_peek_head(&pResultList->List, LL_ACCESS_NOLOCK);
	else
		pEntry = csr_ll_next(&pResultList->List, pResultList->pCurEntry,
				     LL_ACCESS_NOLOCK);

	if (pEntry) {
		pResult = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
		pRet = &pResult->Result;
	}
	pResultList->pCurEntry = pEntry;
	csr_ll_unlock(&pResultList->List);
	return pRet;
}

/*
 * This function moves the first BSS that matches the bssid to the
 * head of the result
 */
QDF_STATUS csr_move_bss_to_head_from_bssid(tpAniSirGlobal pMac,
					   struct qdf_mac_addr *bssid,
					   tScanResultHandle hScanResult)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tScanResultList *pResultList = (tScanResultList *) hScanResult;
	tCsrScanResult *pResult = NULL;
	tListElem *pEntry = NULL;

	if (!(pResultList && bssid))
		return status;

	csr_ll_lock(&pResultList->List);
	pEntry = csr_ll_peek_head(&pResultList->List, LL_ACCESS_NOLOCK);
	while (pEntry) {
		pResult = GET_BASE_ADDR(pEntry, tCsrScanResult, Link);
		if (!qdf_mem_cmp(bssid, pResult->Result.BssDescriptor.bssId,
				    sizeof(struct qdf_mac_addr))) {
			status = QDF_STATUS_SUCCESS;
			csr_ll_remove_entry(&pResultList->List, pEntry,
					    LL_ACCESS_NOLOCK);
			csr_ll_insert_head(&pResultList->List, pEntry,
					   LL_ACCESS_NOLOCK);
			break;
		}
		pEntry = csr_ll_next(&pResultList->List, pResultList->pCurEntry,
				     LL_ACCESS_NOLOCK);
	}
	csr_ll_unlock(&pResultList->List);

	return status;
}

static QDF_STATUS csr_send_mb_scan_req(tpAniSirGlobal pMac, uint16_t sessionId,
				       tCsrScanRequest *pScanReq,
				       tScanReqParam *pScanReqParam)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSirSmeScanReq *pMsg;
	uint16_t msgLen;
	tSirScanType scanType = pScanReq->scanType;
	uint32_t minChnTime;    /* in units of milliseconds */
	uint32_t maxChnTime;    /* in units of milliseconds */
	uint32_t i;
	struct qdf_mac_addr selfmac;

	msgLen = (uint16_t) (sizeof(tSirSmeScanReq) -
		 sizeof(pMsg->channelList.channelNumber) +
		 (sizeof(pMsg->channelList.channelNumber) *
		 pScanReq->ChannelInfo.numOfChannels)) +
		 (pScanReq->uIEFieldLen);

	pMsg = qdf_mem_malloc(msgLen);
	if (NULL == pMsg) {
		sme_err("memory allocation failed");
		sme_debug("Failed: SId: %d FirstMatch: %d UniqueResult: %d freshScan: %d hiddenSsid: %d",
			sessionId, pScanReqParam->bReturnAfter1stMatch,
			pScanReqParam->fUniqueResult, pScanReqParam->freshScan,
			pScanReqParam->hiddenSsid);
		sme_debug("scanType: %s (%u) BSSType: %s (%u) numOfSSIDs: %d numOfChannels: %d requestType: %s (%d) p2pSearch: %d",
			sme_scan_type_to_string(pScanReq->scanType),
			pScanReq->scanType,
			sme_bss_type_to_string(pScanReq->BSSType),
			pScanReq->BSSType,
			pScanReq->SSIDs.numOfSSIDs,
			pScanReq->ChannelInfo.numOfChannels,
			sme_request_type_to_string(pScanReq->requestType),
			pScanReq->requestType, pScanReq->p2pSearch);
		return QDF_STATUS_E_NOMEM;
	}

	pMsg->messageType = eWNI_SME_SCAN_REQ;
	pMsg->length = msgLen;
	/* ToDO: Fill in session info when we need to do scan base on session */
	if (sessionId != CSR_SESSION_ID_INVALID)
		pMsg->sessionId = sessionId;
	else
		/* if sessionId == CSR_SESSION_ID_INVALID, then send the scan
		 * request on first available session
		 */
		pMsg->sessionId = 0;

	if (pMsg->sessionId >= CSR_ROAM_SESSION_MAX)
		sme_err("Invalid Sme Session ID: %d",
			pMsg->sessionId);
	pMsg->transactionId = 0;
	pMsg->dot11mode = (uint8_t) csr_translate_to_wni_cfg_dot11_mode(pMac,
				csr_find_best_phy_mode(pMac,
					pMac->roam.configParam.phyMode));
	pMsg->bssType = csr_translate_bsstype_to_mac_type(pScanReq->BSSType);

	if (CSR_IS_SESSION_VALID(pMac, sessionId)) {
		qdf_copy_macaddr(&selfmac,
			&pMac->roam.roamSession[sessionId].selfMacAddr);
	} else {
		/*
		 * Since we don't have session for the scanning, we find a valid
		 * session. In case we fail to do so, get the WNI_CFG_STA_ID
		 */
		for (i = 0; i < CSR_ROAM_SESSION_MAX; i++) {
			if (CSR_IS_SESSION_VALID(pMac, i)) {
				qdf_copy_macaddr(&selfmac,
					&pMac->roam.roamSession[i].selfMacAddr);
				break;
			}
		}
		if (CSR_ROAM_SESSION_MAX == i) {
			uint32_t len = QDF_MAC_ADDR_SIZE;

			status = wlan_cfg_get_str(pMac, WNI_CFG_STA_ID,
						  selfmac.bytes, &len);
			if (!QDF_IS_STATUS_SUCCESS(status)
			    || (len < QDF_MAC_ADDR_SIZE)) {
				sme_err("Can't get self MAC address: %d",
					status);
				status = QDF_STATUS_E_FAILURE;
				goto send_scan_req;
			}
		}
	}
	qdf_copy_macaddr(&pMsg->selfMacAddr, &selfmac);

	qdf_copy_macaddr(&pMsg->bssId, &pScanReq->bssid);
	if (qdf_is_macaddr_zero(&pScanReq->bssid))
		qdf_set_macaddr_broadcast(&pMsg->bssId);
	else
		qdf_copy_macaddr(&pMsg->bssId, &pScanReq->bssid);
	minChnTime = pScanReq->minChnTime;
	maxChnTime = pScanReq->maxChnTime;

	/*
	 * Verify the scan type first, if the scan is active scan, we need to
	 * make sure we are allowed to do so. if 11d is enabled & we don't see
	 * any beacon around, scan type falls back to passive. But in BT AMP STA
	 * mode we need to send out a directed probe
	 */
	if ((eSIR_PASSIVE_SCAN != scanType)
	    && (eCSR_SCAN_P2P_DISCOVERY != pScanReq->requestType)
	    && (false == pMac->scan.fEnableBypass11d)) {
		scanType = pMac->scan.curScanType;
		if (eSIR_PASSIVE_SCAN == pMac->scan.curScanType) {
			if (minChnTime <
			    pMac->roam.configParam.nPassiveMinChnTime) {
				minChnTime =
				    pMac->roam.configParam.nPassiveMinChnTime;
			}
			if (maxChnTime <
			    pMac->roam.configParam.nPassiveMaxChnTime) {
				maxChnTime =
				    pMac->roam.configParam.nPassiveMaxChnTime;
			}
		}
	}
	pMsg->scanType = scanType;
	pMsg->scan_adaptive_dwell_mode = pScanReq->scan_adaptive_dwell_mode;

	pMsg->numSsid = (pScanReq->SSIDs.numOfSSIDs < SIR_SCAN_MAX_NUM_SSID) ?
			 pScanReq->SSIDs.numOfSSIDs : SIR_SCAN_MAX_NUM_SSID;
	if ((pScanReq->SSIDs.numOfSSIDs != 0)
	    && (eSIR_PASSIVE_SCAN != scanType)) {
		for (i = 0; i < pMsg->numSsid; i++) {
			qdf_mem_copy(&pMsg->ssId[i],
				     &pScanReq->SSIDs.SSIDList[i].SSID,
				     sizeof(tSirMacSSid));
		}
	} else {
		/* Otherwise we scan all SSID and let the result filter later */
		for (i = 0; i < SIR_SCAN_MAX_NUM_SSID; i++)
			pMsg->ssId[i].length = 0;
	}

	pMsg->minChannelTime = minChnTime;
	pMsg->maxChannelTime = maxChnTime;
	/* hidden SSID option */
	pMsg->hiddenSsid = pScanReqParam->hiddenSsid;
	/* maximum rest time */
	pMsg->restTime = pScanReq->restTime;
	/* Minimum rest time */
	pMsg->min_rest_time = pScanReq->min_rest_time;
	/* Idle time */
	pMsg->idle_time = pScanReq->idle_time;
	pMsg->returnAfterFirstMatch = pScanReqParam->bReturnAfter1stMatch;
	/* All the scan results caching will be done by Roaming */
	/* We do not want LIM to do any caching of scan results, */
	/* so delete the LIM cache on all scan requests */
	pMsg->returnFreshResults = pScanReqParam->freshScan;
	/* Always ask for unique result */
	pMsg->returnUniqueResults = pScanReqParam->fUniqueResult;
	pMsg->channelList.numChannels =
		(uint8_t) pScanReq->ChannelInfo.numOfChannels;
	if (pScanReq->ChannelInfo.numOfChannels) {
		/* Assuming the channelNumber is uint8_t (1 byte) */
		qdf_mem_copy(pMsg->channelList.channelNumber,
			     pScanReq->ChannelInfo.ChannelList,
			     pScanReq->ChannelInfo.numOfChannels);
	}

	pMsg->uIEFieldLen = (uint16_t) pScanReq->uIEFieldLen;
	pMsg->uIEFieldOffset = (uint16_t) (sizeof(tSirSmeScanReq) -
			sizeof(pMsg->channelList.channelNumber) +
			(sizeof(pMsg->channelList.channelNumber) *
			 pScanReq->ChannelInfo.numOfChannels));
	if (pScanReq->uIEFieldLen != 0) {
		qdf_mem_copy((uint8_t *) pMsg + pMsg->uIEFieldOffset,
			     pScanReq->pIEField, pScanReq->uIEFieldLen);
	}
	pMsg->p2pSearch = pScanReq->p2pSearch;
	pMsg->scan_id = pScanReq->scan_id;

send_scan_req:
	sme_debug(
		"scanId %d domainIdCurrent %d scanType %s (%d) bssType %s (%d) requestType %s (%d) numChannels %d",
		pMsg->scan_id, pMac->scan.domainIdCurrent,
		sme_scan_type_to_string(pMsg->scanType), pMsg->scanType,
		sme_bss_type_to_string(pMsg->bssType), pMsg->bssType,
		sme_request_type_to_string(pScanReq->requestType),
		pScanReq->requestType, pMsg->channelList.numChannels);

	for (i = 0; i < pMsg->channelList.numChannels; i++) {
		sme_debug("channelNumber[%d]= %d", i,
			pMsg->channelList.channelNumber[i]);
	}

	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = umac_send_mb_message_to_mac(pMsg);
	} else {
		sme_err(
			"failed to send down scan req with status: %d",
			status);
		qdf_mem_free(pMsg);
	}
	return status;
}

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
static void csr_diag_scan_channels(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	host_log_scan_pkt_type *pScanLog = NULL;

	WLAN_HOST_DIAG_LOG_ALLOC(pScanLog,
				 host_log_scan_pkt_type,
				 LOG_WLAN_SCAN_C);
	if (!pScanLog)
		return;

	if (eCsrScanProbeBss == pCommand->u.scanCmd.reason) {
		pScanLog->eventId = WLAN_SCAN_EVENT_HO_SCAN_REQ;
	} else {
		if ((eSIR_PASSIVE_SCAN !=
			pCommand->u.scanCmd.u.scanRequest.scanType)
		    && (eSIR_PASSIVE_SCAN != pMac->scan.curScanType)) {
			pScanLog->eventId = WLAN_SCAN_EVENT_ACTIVE_SCAN_REQ;
		} else {
			pScanLog->eventId = WLAN_SCAN_EVENT_PASSIVE_SCAN_REQ;
		}
	}
	pScanLog->minChnTime =
		(uint8_t) pCommand->u.scanCmd.u.scanRequest.minChnTime;
	pScanLog->maxChnTime =
		(uint8_t) pCommand->u.scanCmd.u.scanRequest.maxChnTime;
	pScanLog->numChannel =
	(uint8_t) pCommand->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels;
	if (pScanLog->numChannel &&
	    (pScanLog->numChannel < HOST_LOG_MAX_NUM_CHANNEL)) {
		qdf_mem_copy(pScanLog->channels,
		      pCommand->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList,
		      pScanLog->numChannel);
	}
	WLAN_HOST_DIAG_LOG_REPORT(pScanLog);
}
#else
#define csr_diag_scan_channels(tpAniSirGlobal pMac, tSmeCmd *pCommand) (void)0;
#endif /* #ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR */

static QDF_STATUS csr_scan_channels(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tScanReqParam scanReq;

	/*
	 * Don't delete cached results. Rome rssi based scan candidates may land
	 * up in scan cache instead of LFR cache. They will be deleted upon
	 * query
	 */
	scanReq.freshScan = SIR_BG_SCAN_RETURN_FRESH_RESULTS;
	scanReq.fUniqueResult = true;
	scanReq.hiddenSsid = SIR_SCAN_NO_HIDDEN_SSID;
	if (eCsrScanForSsid == pCommand->u.scanCmd.reason) {
		scanReq.bReturnAfter1stMatch =
			CSR_SCAN_RETURN_AFTER_FIRST_MATCH;
	} else {
		/*
		 * Basically do scan on all channels even for 11D 1st scan case
		 */
		scanReq.bReturnAfter1stMatch =
			CSR_SCAN_RETURN_AFTER_ALL_CHANNELS;
	}
	if (eCsrScanProbeBss == pCommand->u.scanCmd.reason)
		scanReq.hiddenSsid = SIR_SCAN_HIDDEN_SSID_PE_DECISION;
	csr_diag_scan_channels(pMac, pCommand);
	csr_clear_votes_for_country_info(pMac);
	status = csr_send_mb_scan_req(pMac, pCommand->sessionId,
				      &pCommand->u.scanCmd.u.scanRequest,
				      &scanReq);
	return status;
}

static QDF_STATUS
csr_issue_user_scan(tpAniSirGlobal mac_ctx, tSmeCmd *cmd)
{
	int i, j;
	QDF_STATUS status;
	uint32_t len = 0;
	uint8_t *ch_lst = NULL;
	tCsrChannelInfo new_ch_info = { 0, NULL };

	if (!mac_ctx->roam.configParam.fScanTwice)
		return csr_scan_channels(mac_ctx, cmd);

	/* We scan 2.4 channel twice */
	if (cmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels
	    && (NULL != cmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList)) {
		len = cmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels;
		/* allocate twice the channel */
		new_ch_info.ChannelList = (uint8_t *) qdf_mem_malloc(len * 2);
		ch_lst = cmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList;
	} else {
		/* get the valid channel list to scan all. */
		len = sizeof(mac_ctx->roam.validChannelList);
		status = csr_get_cfg_valid_channels(mac_ctx,
			    (uint8_t *) mac_ctx->roam.validChannelList, &len);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			/* allocate twice the channel */
			new_ch_info.ChannelList =
				(uint8_t *) qdf_mem_malloc(len * 2);
			ch_lst = mac_ctx->roam.validChannelList;
		}
	}
	if (NULL == new_ch_info.ChannelList) {
		new_ch_info.numOfChannels = 0;
	} else {
		j = 0;
		for (i = 0; i < len; i++) {
			new_ch_info.ChannelList[j++] = ch_lst[i];
			if (WLAN_REG_MAX_24GHZ_CH_NUM >= ch_lst[i])
				new_ch_info.ChannelList[j++] = ch_lst[i];
		}
		if (NULL !=
		    cmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList) {
			/*
			 * ch_lst points to the channellist from the command,
			 * free it.
			 */
			qdf_mem_free(
			  cmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList);
			cmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList =
			  NULL;
		}
		cmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels = j;
		cmd->u.scanCmd.u.scanRequest.ChannelInfo.ChannelList =
							new_ch_info.ChannelList;
	}

	return csr_scan_channels(mac_ctx, cmd);
}

QDF_STATUS csr_process_scan_command(tpAniSirGlobal pMac, tSmeCmd *pCommand)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	sme_debug("starting SCAN cmd in %d state. reason %d",
		pCommand->u.scanCmd.lastRoamState[pCommand->sessionId],
		pCommand->u.scanCmd.reason);

	switch (pCommand->u.scanCmd.reason) {
	case eCsrScanUserRequest:
		status = csr_issue_user_scan(pMac, pCommand);
		break;
	default:
		status = csr_scan_channels(pMac, pCommand);
		break;
	}

	if (!QDF_IS_STATUS_SUCCESS(status))
		pCommand->u.scanCmd.status = eCSR_SCAN_FAILURE;

	return status;
}

/**
 * csr_scan_copy_request_valid_channels_only() - scan request of valid channels
 * @mac_ctx : pointer to Global Mac Structure
 * @dst_req: pointer to tCsrScanRequest
 * @skip_dfs_chnl: 1 - skip dfs channel, 0 - don't skip dfs channel
 * @src_req: pointer to tCsrScanRequest
 *
 * This function makes a copy of scan request with valid channels
 *
 * Return: none
 */
static void csr_scan_copy_request_valid_channels_only(tpAniSirGlobal mac_ctx,
				tCsrScanRequest *dst_req, uint8_t skip_dfs_chnl,
				tCsrScanRequest *src_req)
{
	uint32_t index = 0;
	uint32_t new_index = 0;
	uint16_t  unsafe_chan[NUM_CHANNELS];
	uint16_t  unsafe_chan_cnt = 0;
	uint16_t  cnt = 0;
	bool      is_unsafe_chan;
	qdf_device_t qdf_ctx = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!qdf_ctx) {
		cds_err("qdf_ctx is NULL");
		return;
	}
	pld_get_wlan_unsafe_channel(qdf_ctx->dev, unsafe_chan,
			&unsafe_chan_cnt,
			sizeof(unsafe_chan));

	if (mac_ctx->roam.configParam.sta_roam_policy.dfs_mode ==
			CSR_STA_ROAM_POLICY_DFS_DISABLED)
		skip_dfs_chnl = true;

	for (index = 0; index < src_req->ChannelInfo.numOfChannels; index++) {
		/* Allow scan on valid channels only.
		 * If it is p2p scan and valid channel list doesnt contain
		 * social channels, enforce scan on social channels because
		 * that is the only way to find p2p peers.
		 * This can happen only if band is set to 5Ghz mode.
		 */
		if (src_req->ChannelInfo.ChannelList[index] <
		    WLAN_REG_MIN_11P_CH_NUM &&
		    ((csr_roam_is_valid_channel(mac_ctx,
		     src_req->ChannelInfo.ChannelList[index])) ||
		     ((eCSR_SCAN_P2P_DISCOVERY == src_req->requestType) &&
		      CSR_IS_SOCIAL_CHANNEL(
				src_req->ChannelInfo.ChannelList[index])))) {
			if (((src_req->skipDfsChnlInP2pSearch ||
				skip_dfs_chnl) && (CHANNEL_STATE_DFS ==
				wlan_reg_get_channel_state(mac_ctx->pdev,
				src_req->ChannelInfo.ChannelList[index]))) &&
				(src_req->ChannelInfo.numOfChannels > 1)) {
				sme_debug(
					"reqType= %s (%d), numOfChannels=%d, ignoring DFS channel %d",
					sme_request_type_to_string(
						src_req->requestType),
					src_req->requestType,
					src_req->ChannelInfo.numOfChannels,
					src_req->ChannelInfo.ChannelList
						[index]);
				continue;
			}
			if (mac_ctx->roam.configParam.
					sta_roam_policy.skip_unsafe_channels &&
					unsafe_chan_cnt) {
				is_unsafe_chan = false;
				for (cnt = 0; cnt < unsafe_chan_cnt; cnt++) {
					if (unsafe_chan[cnt] ==
						src_req->ChannelInfo.
						ChannelList[index]) {
						is_unsafe_chan = true;
						break;
					}
				}
				if (is_unsafe_chan &&
					((CSR_IS_CHANNEL_24GHZ(
						src_req->ChannelInfo.
						ChannelList[index]) &&
					mac_ctx->roam.configParam.
					sta_roam_policy.sap_operating_band ==
						eCSR_BAND_24) ||
						(WLAN_REG_IS_5GHZ_CH(
							src_req->ChannelInfo.
							ChannelList[index]) &&
					mac_ctx->roam.configParam.
					sta_roam_policy.sap_operating_band ==
						eCSR_BAND_5G))) {
					QDF_TRACE(QDF_MODULE_ID_SME,
						QDF_TRACE_LEVEL_INFO,
					      FL("ignoring unsafe channel %d"),
						src_req->ChannelInfo.
						ChannelList[index]);
					continue;
				}
			}

			dst_req->ChannelInfo.ChannelList[new_index] =
				src_req->ChannelInfo.ChannelList[index];
			new_index++;
		}
	}
	dst_req->ChannelInfo.numOfChannels = new_index;
}

/**
 * csr_scan_filter_given_chnl_band() - filter all channels which matches given
 *                                    channel's band
 * @mac_ctx: pointer to mac context
 * @channel: Given channel
 * @dst_req: destination scan request
 *
 * when ever particular connection already exist, STA should not scan the
 * channels which fall under same band as given channel's band.
 * this routine will filter out those channels
 *
 * Return: true if success otherwise false for any failure
 */
static bool csr_scan_filter_given_chnl_band(tpAniSirGlobal mac_ctx,
			uint8_t channel, tCsrScanRequest *dst_req) {
	uint8_t valid_chnl_list[WNI_CFG_VALID_CHANNEL_LIST_LEN] = {0};
	uint32_t filter_chnl_len = 0, i = 0;
	uint32_t valid_chnl_len = WNI_CFG_VALID_CHANNEL_LIST_LEN;

	if (!channel) {
		sme_debug("Nothing to filter as no IBSS session");
		return true;
	}

	if (!dst_req) {
		sme_err("No valid scan requests");
		return false;
	}
	/*
	 * In case of concurrent IBSS session exist, scan only
	 * those channels which are not in IBSS channel's band.
	 * In case if no-concurrent IBSS session exist then scan
	 * full band
	 */
	if ((dst_req->ChannelInfo.numOfChannels == 0)) {
		csr_get_cfg_valid_channels(mac_ctx, valid_chnl_list,
				&valid_chnl_len);
	} else {
		valid_chnl_len = (WNI_CFG_VALID_CHANNEL_LIST_LEN >
					dst_req->ChannelInfo.numOfChannels) ?
					dst_req->ChannelInfo.numOfChannels :
					WNI_CFG_VALID_CHANNEL_LIST_LEN;
		qdf_mem_copy(valid_chnl_list, dst_req->ChannelInfo.ChannelList,
				valid_chnl_len);
	}
	for (i = 0; i < valid_chnl_len; i++) {
		if (valid_chnl_list[i] >= WLAN_REG_MIN_11P_CH_NUM)
			continue;
		if (WLAN_REG_IS_5GHZ_CH(channel) &&
			WLAN_REG_IS_24GHZ_CH(valid_chnl_list[i])) {
			valid_chnl_list[filter_chnl_len] =
					valid_chnl_list[i];
			filter_chnl_len++;
		} else if (WLAN_REG_IS_24GHZ_CH(channel) &&
			WLAN_REG_IS_5GHZ_CH(valid_chnl_list[i])) {
			valid_chnl_list[filter_chnl_len] =
					valid_chnl_list[i];
			filter_chnl_len++;
		}
	}
	if (filter_chnl_len == 0) {
		sme_err("there no channels to scan due to IBSS session");
		return false;
	}

	if (dst_req->ChannelInfo.ChannelList) {
		qdf_mem_free(dst_req->ChannelInfo.ChannelList);
		dst_req->ChannelInfo.ChannelList = NULL;
		dst_req->ChannelInfo.numOfChannels = 0;
	}

	dst_req->ChannelInfo.ChannelList =
			qdf_mem_malloc(filter_chnl_len *
				sizeof(*dst_req->ChannelInfo.ChannelList));
	dst_req->ChannelInfo.numOfChannels = filter_chnl_len;
	if (NULL == dst_req->ChannelInfo.ChannelList) {
		sme_err("Memory allocation failed");
		return false;
	}
	qdf_mem_copy(dst_req->ChannelInfo.ChannelList, valid_chnl_list,
			filter_chnl_len);
	return true;
}

/**
 * csr_scan_copy_request() - Function to copy scan request
 * @mac_ctx : pointer to Global Mac Structure
 * @dst_req: pointer to tCsrScanRequest
 * @src_req: pointer to tCsrScanRequest
 *
 * This function makes a copy of scan request
 *
 * Return: 0 - Success, Error number - Failure
 */
QDF_STATUS csr_scan_copy_request(tpAniSirGlobal mac_ctx,
				tCsrScanRequest *dst_req,
				tCsrScanRequest *src_req)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t len = sizeof(mac_ctx->roam.validChannelList);
	uint32_t index = 0;
	uint32_t new_index = 0;
	enum channel_state channel_state;
	uint8_t channel = 0;

	bool skip_dfs_chnl =
			mac_ctx->roam.configParam.initial_scan_no_dfs_chnl ||
				!mac_ctx->scan.fEnableDFSChnlScan;

	status = csr_scan_free_request(mac_ctx, dst_req);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto complete;
	qdf_mem_copy(dst_req, src_req, sizeof(tCsrScanRequest));
	/* Re-initialize the pointers to NULL since we did a copy */
	dst_req->pIEField = NULL;
	dst_req->ChannelInfo.ChannelList = NULL;
	dst_req->SSIDs.SSIDList = NULL;

	if (src_req->uIEFieldLen) {
		dst_req->pIEField =
			qdf_mem_malloc(src_req->uIEFieldLen);
		if (NULL == dst_req->pIEField) {
			status = QDF_STATUS_E_NOMEM;
			sme_err("No memory for scanning IE fields");
			goto complete;
		} else {
			status = QDF_STATUS_SUCCESS;
			qdf_mem_copy(dst_req->pIEField, src_req->pIEField,
				src_req->uIEFieldLen);
			dst_req->uIEFieldLen = src_req->uIEFieldLen;
		}
	}

	/* Allocate memory for IE field */
	if (src_req->ChannelInfo.numOfChannels == 0) {
		dst_req->ChannelInfo.ChannelList = NULL;
		dst_req->ChannelInfo.numOfChannels = 0;
	} else {
		dst_req->ChannelInfo.ChannelList =
			qdf_mem_malloc(src_req->ChannelInfo.numOfChannels *
				sizeof(*dst_req->ChannelInfo.ChannelList));
		if (NULL == dst_req->ChannelInfo.ChannelList) {
			status = QDF_STATUS_E_NOMEM;
			dst_req->ChannelInfo.numOfChannels = 0;
			sme_err("No memory for scanning Channel List");
			goto complete;
		}

		if ((src_req->scanType == eSIR_PASSIVE_SCAN) &&
			(src_req->requestType == eCSR_SCAN_REQUEST_11D_SCAN)) {
			for (index = 0; index < src_req->ChannelInfo.
						numOfChannels; index++) {
				channel_state = wlan_reg_get_channel_state(
						mac_ctx->pdev,
						src_req->ChannelInfo.
						ChannelList[index]);
				if (src_req->ChannelInfo.ChannelList[index] <
				    WLAN_REG_MIN_11P_CH_NUM &&
				    ((CHANNEL_STATE_ENABLE ==
				      channel_state) ||
				     ((CHANNEL_STATE_DFS == channel_state) &&
				      !skip_dfs_chnl))) {
					dst_req->ChannelInfo.ChannelList
						[new_index] =
						src_req->
						ChannelInfo.
						ChannelList
						[index];
					new_index++;
				}
			}
			dst_req->ChannelInfo.numOfChannels = new_index;
		} else if (QDF_IS_STATUS_SUCCESS(
				csr_get_cfg_valid_channels(mac_ctx,
						mac_ctx->roam.validChannelList,
						&len))) {
			new_index = 0;
			mac_ctx->roam.numValidChannels = len;
			csr_scan_copy_request_valid_channels_only(mac_ctx,
							dst_req, skip_dfs_chnl,
							src_req);
		} else {
			sme_err(
				"Couldn't get the valid Channel List, keeping requester's list");
			new_index = 0;
			for (index = 0; index < src_req->ChannelInfo.
				     numOfChannels; index++) {
				if (src_req->ChannelInfo.ChannelList[index] <
				    WLAN_REG_MIN_11P_CH_NUM) {
					dst_req->ChannelInfo.
						ChannelList[new_index] =
						src_req->ChannelInfo.
						ChannelList[index];
					new_index++;
				}
			}
			dst_req->ChannelInfo.numOfChannels =
				new_index;
		}
	} /* Allocate memory for Channel List */

	/*
	 * If IBSS concurrent connection exist, and if the scan
	 * request comes from STA adapter then we need to filter
	 * out IBSS channel's band otherwise it will cause issue
	 * in IBSS+STA concurrency
	 *
	 * If DFS SAP/GO concurrent connection exist, and if the scan
	 * request comes from STA adapter then we need to filter
	 * out SAP/GO channel's band otherwise it will cause issue in
	 * SAP+STA concurrency
	 */
	if (policy_mgr_is_ibss_conn_exist(mac_ctx->psoc, &channel)) {
		sme_debug("Conc IBSS exist, channel list will be modified");
	} else if (policy_mgr_is_any_dfs_beaconing_session_present(
			mac_ctx->psoc, &channel)) {
		/*
		 * 1) if agile & DFS scans are supported
		 * 2) if hardware is DBS capable
		 * 3) if current hw mode is non-dbs
		 * if all above 3 conditions are true then don't skip any
		 * channel from scan list
		 */
		if (true != policy_mgr_is_current_hwmode_dbs(mac_ctx->psoc) &&
		    policy_mgr_get_dbs_plus_agile_scan_config(mac_ctx->psoc) &&
		    policy_mgr_get_single_mac_scan_with_dfs_config(
		    mac_ctx->psoc))
			channel = 0;
		else
			sme_debug(
				"Conc DFS SAP/GO exist, channel list will be modified");
	}

	if ((channel > 0) &&
	    (!csr_scan_filter_given_chnl_band(mac_ctx, channel, dst_req))) {
		sme_err("Can't filter channels due to IBSS/SAP DFS");
		goto complete;
	}

	if (src_req->SSIDs.numOfSSIDs == 0) {
		dst_req->SSIDs.numOfSSIDs = 0;
		dst_req->SSIDs.SSIDList = NULL;
	} else {
		dst_req->SSIDs.SSIDList =
			qdf_mem_malloc(src_req->SSIDs.numOfSSIDs *
					sizeof(*dst_req->SSIDs.SSIDList));
		if (NULL == dst_req->SSIDs.SSIDList)
			status = QDF_STATUS_E_NOMEM;
		else
			status = QDF_STATUS_SUCCESS;
		if (QDF_IS_STATUS_SUCCESS(status)) {
			dst_req->SSIDs.numOfSSIDs =
				src_req->SSIDs.numOfSSIDs;
			qdf_mem_copy(dst_req->SSIDs.SSIDList,
				src_req->SSIDs.SSIDList,
				src_req->SSIDs.numOfSSIDs *
				sizeof(*dst_req->SSIDs.SSIDList));
		} else {
			dst_req->SSIDs.numOfSSIDs = 0;
			sme_err("No memory for scanning SSID List");
			goto complete;
		}
	} /* Allocate memory for SSID List */
	qdf_mem_copy(&dst_req->bssid, &src_req->bssid,
		sizeof(struct qdf_mac_addr));
	dst_req->p2pSearch = src_req->p2pSearch;
	dst_req->skipDfsChnlInP2pSearch =
		src_req->skipDfsChnlInP2pSearch;
	dst_req->scan_id = src_req->scan_id;
	dst_req->timestamp = src_req->timestamp;

complete:
	if (!QDF_IS_STATUS_SUCCESS(status))
		csr_scan_free_request(mac_ctx, dst_req);

	return status;
}

QDF_STATUS csr_scan_free_request(tpAniSirGlobal pMac, tCsrScanRequest *pReq)
{

	if (pReq->ChannelInfo.ChannelList) {
		qdf_mem_free(pReq->ChannelInfo.ChannelList);
		pReq->ChannelInfo.ChannelList = NULL;
	}
	pReq->ChannelInfo.numOfChannels = 0;
	if (pReq->pIEField) {
		qdf_mem_free(pReq->pIEField);
		pReq->pIEField = NULL;
	}
	pReq->uIEFieldLen = 0;
	if (pReq->SSIDs.SSIDList) {
		qdf_mem_free(pReq->SSIDs.SSIDList);
		pReq->SSIDs.SSIDList = NULL;
	}
	pReq->SSIDs.numOfSSIDs = 0;

	return QDF_STATUS_SUCCESS;
}

void csr_scan_call_callback(tpAniSirGlobal pMac, tSmeCmd *pCommand,
			    eCsrScanStatus scanStatus)
{
	if (pCommand->u.scanCmd.callback) {
		if (pCommand->u.scanCmd.abort_scan_indication) {
			sme_debug("scanDone due to abort");
			scanStatus = eCSR_SCAN_ABORT;
		}
		pCommand->u.scanCmd.callback(pMac, pCommand->u.scanCmd.pContext,
					     pCommand->sessionId,
					     pCommand->u.scanCmd.scanID,
					     scanStatus);
	} else {
		sme_debug("Callback NULL!!!");
	}
}

bool csr_scan_remove_fresh_scan_command(tpAniSirGlobal pMac, uint8_t sessionId)
{
	bool fRet = false;
	tListElem *pEntry, *pEntryTmp;
	tSmeCmd *pCommand;
	tDblLinkList localList;

	qdf_mem_zero(&localList, sizeof(tDblLinkList));
	if (!QDF_IS_STATUS_SUCCESS(csr_ll_open(pMac->hHdd, &localList))) {
		sme_err("failed to open list");
		return fRet;
	}

	csr_scan_pending_ll_lock(pMac);
	pEntry = csr_scan_pending_ll_peek_head(pMac, LL_ACCESS_NOLOCK);
	while (pEntry) {
		pEntryTmp = csr_scan_pending_ll_next(pMac, pEntry,
						LL_ACCESS_NOLOCK);
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		if (!((eSmeCommandScan == pCommand->command)
		    && (sessionId == pCommand->sessionId))) {
			pEntry = pEntryTmp;
			continue;
		}
		sme_debug("-------- abort scan command reason = %d",
			pCommand->u.scanCmd.reason);
		/* The rest are fresh scan requests */
		if (csr_scan_pending_ll_remove_entry(pMac, pEntry,
					LL_ACCESS_NOLOCK)) {
			csr_ll_insert_tail(&localList, pEntry,
					   LL_ACCESS_NOLOCK);
		}
		fRet = true;
		pEntry = pEntryTmp;
	}

	csr_scan_pending_ll_unlock(pMac);

	while ((pEntry = csr_ll_remove_head(&localList, LL_ACCESS_NOLOCK))) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		if (pCommand->u.scanCmd.callback) {
			/*
			 * User scan request is pending, send response with
			 * status eCSR_SCAN_ABORT
			 */
			pCommand->u.scanCmd.callback(pMac,
				pCommand->u.scanCmd.pContext, sessionId,
				pCommand->u.scanCmd.scanID, eCSR_SCAN_ABORT);
		}
		csr_release_command(pMac, pCommand);
	}
	csr_ll_close(&localList);

	return fRet;
}

QDF_STATUS csr_scan_get_pmkid_candidate_list(tpAniSirGlobal pMac,
					     uint32_t sessionId,
					     tPmkidCandidateInfo *pPmkidList,
					     uint32_t *pNumItems)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	tCsrScanResultFilter *pScanFilter;
	tCsrScanResultInfo *pScanResult;
	tScanResultHandle hBSSList;
	uint32_t nItems = *pNumItems;

	if (!pSession) {
		sme_err("session %d not found", sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	sme_debug("pMac->scan.NumPmkidCandidate = %d",
		pSession->NumPmkidCandidate);
	csr_reset_pmkid_candidate_list(pMac, sessionId);
	if (!(csr_is_conn_state_connected(pMac, sessionId)
	    && pSession->pCurRoamProfile))
		return status;

	*pNumItems = 0;
	pScanFilter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == pScanFilter)
		return QDF_STATUS_E_NOMEM;

	/* Here is the profile we need to connect to */
	status = csr_roam_prepare_filter_from_profile(pMac,
					pSession->pCurRoamProfile, pScanFilter);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		qdf_mem_free(pScanFilter);
		return status;
	}

	status = csr_scan_get_result(pMac, pScanFilter, &hBSSList);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		csr_free_scan_filter(pMac, pScanFilter);
		qdf_mem_free(pScanFilter);
		return status;
	}

	if (pSession->NumPmkidCandidate < nItems) {
		pScanResult = csr_scan_result_get_next(pMac, hBSSList);
		while (pScanResult != NULL) {
			/* NumPmkidCandidate adds up here */
			csr_process_bss_desc_for_pmkid_list(pMac,
				&pScanResult->BssDescriptor,
				(tDot11fBeaconIEs *)(pScanResult->pvIes),
				sessionId);
			pScanResult = csr_scan_result_get_next(pMac, hBSSList);
		}
	}

	if (pSession->NumPmkidCandidate) {
		*pNumItems = pSession->NumPmkidCandidate;
		qdf_mem_copy(pPmkidList, pSession->PmkidCandidateInfo,
			     pSession->NumPmkidCandidate *
			     sizeof(tPmkidCandidateInfo));
	}

	csr_scan_result_purge(pMac, hBSSList);
	csr_free_scan_filter(pMac, pScanFilter);
	qdf_mem_free(pScanFilter);
	return status;
}

#ifdef FEATURE_WLAN_WAPI
QDF_STATUS csr_scan_get_bkid_candidate_list(tpAniSirGlobal pMac,
					    uint32_t sessionId,
					    tBkidCandidateInfo *pBkidList,
					    uint32_t *pNumItems)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	tCsrScanResultFilter *pScanFilter;
	tCsrScanResultInfo *pScanResult;
	tScanResultHandle hBSSList;
	uint32_t nItems = *pNumItems;

	if (!pSession) {
		sme_err("session %d not found", sessionId);
		return QDF_STATUS_E_FAILURE;
	}

	sme_debug("pMac->scan.NumBkidCandidate = %d",
		pSession->NumBkidCandidate);
	csr_reset_bkid_candidate_list(pMac, sessionId);
	if (!(csr_is_conn_state_connected(pMac, sessionId)
	    && pSession->pCurRoamProfile))
		return status;

	*pNumItems = 0;
	pScanFilter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (NULL == pScanFilter)
		return QDF_STATUS_E_NOMEM;

	/* Here is the profile we need to connect to */
	status = csr_roam_prepare_filter_from_profile(pMac,
					pSession->pCurRoamProfile, pScanFilter);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		qdf_mem_free(pScanFilter);
		return status;
	}

	status = csr_scan_get_result(pMac, pScanFilter, &hBSSList);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		csr_free_scan_filter(pMac, pScanFilter);
		qdf_mem_free(pScanFilter);
		return status;
	}

	if (pSession->NumBkidCandidate < nItems) {
		pScanResult = csr_scan_result_get_next(pMac, hBSSList);
		while (pScanResult != NULL) {
			/* pMac->scan.NumBkidCandidate adds up here */
			csr_process_bss_desc_for_bkid_list(pMac,
				&pScanResult->BssDescriptor,
				(tDot11fBeaconIEs *)(pScanResult->pvIes));
			pScanResult = csr_scan_result_get_next(pMac, hBSSList);
		}
	}

	if (pSession->NumBkidCandidate) {
		*pNumItems = pSession->NumBkidCandidate;
		qdf_mem_copy(pBkidList, pSession->BkidCandidateInfo,
			     pSession->NumBkidCandidate *
			     sizeof(tBkidCandidateInfo));
	}

	csr_scan_result_purge(pMac, hBSSList);
	csr_free_scan_filter(pMac, pScanFilter);
	qdf_mem_free(pScanFilter);
	return status;
}
#endif /* FEATURE_WLAN_WAPI */

#ifndef NAPIER_SCAN
/**
 * csr_roam_copy_channellist() - Function to copy channel list
 * @mac_ctx: pointer to Global Mac structure
 * @profile: pointer to tCsrRoamProfile
 * @scan_cmd: pointer to tSmeCmd
 * @index: index for channellist
 *
 * Function copies channel list
 *
 * Return: none
 */
static void csr_roam_copy_channellist(tpAniSirGlobal mac_ctx,
				tCsrRoamProfile *profile,
				tSmeCmd *scan_cmd, uint8_t index)
{
	tCsrChannelInfo *channel_info =
		&scan_cmd->u.scanCmd.u.scanRequest.ChannelInfo;

	for (index = 0; index < profile->ChannelInfo.numOfChannels;
			index++) {
		if (!csr_roam_is_valid_channel(mac_ctx,
			profile->ChannelInfo.ChannelList[index])) {
			sme_warn("process a channel: %d that is invalid",
			profile->ChannelInfo.ChannelList[index]);
			continue;
		}
		channel_info->ChannelList[channel_info->numOfChannels] =
			profile->ChannelInfo.ChannelList[index];
		scan_cmd->u.scanCmd.u.scanRequest.ChannelInfo.numOfChannels++;
	}
}
#endif
/**
 * csr_scan_for_ssid() -  Function usually used for BSSs that suppresses SSID
 * @mac_ctx: Pointer to Global Mac structure
 * @profile: pointer to tCsrRoamProfile
 * @roam_id: variable representing roam id
 * @notify: boolean variable
 *
 * Function is usually used for BSSs that suppresses SSID so the profile
 * shall have one and only one SSID.
 *
 * Return: Success - QDF_STATUS_SUCCESS, Failure - error number
 */
QDF_STATUS csr_scan_for_ssid(tpAniSirGlobal mac_ctx, uint32_t session_id,
			tCsrRoamProfile *profile, uint32_t roam_id,
			bool notify)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	uint32_t num_ssid = profile->SSIDs.numOfSSIDs;
	struct scan_start_request *req;
	struct wlan_objmgr_vdev *vdev;
	uint8_t i, chan, num_chan = 0, str[MAX_SSID_LEN];
	wlan_scan_id scan_id;
	tCsrRoamSession *session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!(mac_ctx->scan.fScanEnable) && (num_ssid != 1)) {
		sme_err(
			"cannot scan because scanEnable (%d) or numSSID (%d) is invalid",
			mac_ctx->scan.fScanEnable, profile->SSIDs.numOfSSIDs);
		return status;
	}

	session->scan_info.profile =
			qdf_mem_malloc(sizeof(tCsrRoamProfile));
	if (!session->scan_info.profile)
		status = QDF_STATUS_E_NOMEM;
	else
		status = csr_roam_copy_profile(mac_ctx,
					session->scan_info.profile,
					profile);
	if (!QDF_IS_STATUS_SUCCESS(status))
		goto error;
	scan_id = ucfg_scan_get_scan_id(mac_ctx->psoc);
	session->scan_info.scan_id = scan_id;
	session->scan_info.scan_reason = eCsrScanForSsid;
	session->scan_info.roam_id = roam_id;
	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  FL("Failed to allocate memory"));
		goto error;
	}
	vdev = wlan_objmgr_get_vdev_by_macaddr_from_psoc(mac_ctx->psoc,
				session->selfMacAddr.bytes,
				WLAN_LEGACY_SME_ID);
	ucfg_scan_init_default_params(vdev, req);
	req->scan_req.dwell_time_active = 0;
	req->scan_req.scan_id = scan_id;
	req->scan_req.vdev_id = session_id;
	req->scan_req.scan_req_id = mac_ctx->scan.requester_id;

	if (QDF_P2P_CLIENT_MODE == profile->csrPersona)
		req->scan_req.scan_priority = SCAN_PRIORITY_HIGH;

	/* Allocate memory for IE field */
	if (profile->pAddIEScan) {
		req->scan_req.extraie.ptr =
			qdf_mem_malloc(profile->nAddIEScanLength);

		if (NULL == req->scan_req.extraie.ptr)
			status = QDF_STATUS_E_NOMEM;
		else
			status = QDF_STATUS_SUCCESS;

		if (QDF_IS_STATUS_SUCCESS(status)) {
			qdf_mem_copy(req->scan_req.extraie.ptr,
					profile->pAddIEScan,
					profile->nAddIEScanLength);
			req->scan_req.extraie.len = profile->nAddIEScanLength;
		} else {
			sme_err("No memory for scanning IE fields");
		}
	}

	req->scan_req.num_bssid = 1;
	if (profile->BSSIDs.numOfBSSIDs == 1)
		qdf_copy_macaddr(&req->scan_req.bssid_list[0],
					profile->BSSIDs.bssid);
	else
		qdf_set_macaddr_broadcast(&req->scan_req.bssid_list[0]);

	if (profile->ChannelInfo.numOfChannels) {
		for (i = 0; i < profile->ChannelInfo.numOfChannels; i++) {
			if (csr_roam_is_valid_channel(mac_ctx,
				profile->ChannelInfo.ChannelList[i])) {
				chan = profile->ChannelInfo.ChannelList[i];
				req->scan_req.chan_list[num_chan] =
						wlan_chan_to_freq(chan);
				num_chan++;
			}
		}
		req->scan_req.num_chan = num_chan;
	}

	/* Extend it for multiple SSID */
	if (profile->SSIDs.numOfSSIDs) {
		req->scan_req.num_ssids = 1;
		qdf_mem_copy(&req->scan_req.ssid[0].ssid,
				&profile->SSIDs.SSIDList[0].SSID.ssId,
				profile->SSIDs.SSIDList[0].SSID.length);
		req->scan_req.ssid[0].length =
				profile->SSIDs.SSIDList[0].SSID.length;
		qdf_mem_copy(str, req->scan_req.ssid[0].ssid,
			req->scan_req.ssid[0].length);
		str[req->scan_req.ssid[0].length] = 0;
		sme_debug("scan for SSID = %s", str);
	}
	status = ucfg_scan_start(req);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
error:
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("failed to initiate scan with status: %d", status);

		if (notify)
			csr_roam_call_callback(mac_ctx, session_id, NULL,
					roam_id, eCSR_ROAM_FAILED,
					eCSR_ROAM_RESULT_FAILURE);
	}
	return status;
}

static void csr_set_cfg_valid_channel_list(tpAniSirGlobal pMac,
				uint8_t *pChannelList, uint8_t NumChannels)
{
	uint32_t dataLen = sizeof(uint8_t) * NumChannels;
	QDF_STATUS status;

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "%s: dump valid channel list(NumChannels(%d))",
		  __func__, NumChannels);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
			   pChannelList, NumChannels);
	cfg_set_str(pMac, WNI_CFG_VALID_CHANNEL_LIST, pChannelList,
			dataLen);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
		  "Scan offload is enabled, update default chan list");
	/*
	 * disable fcc constraint since new country code
	 * is being set
	 */
	pMac->scan.fcc_constraint = false;
	status = csr_update_channel_list(pMac);
	if (QDF_STATUS_SUCCESS != status) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			  "failed to update the supported channel list");
	}
}

/*
 * The Tx power limits are saved in the cfg for future usage.
 */
static void csr_save_tx_power_to_cfg(tpAniSirGlobal pMac, tDblLinkList *pList,
			      uint32_t cfgId)
{
	tListElem *pEntry;
	uint32_t cbLen = 0, dataLen, tmp_len;
	tCsrChannelPowerInfo *ch_set;
	uint32_t idx;
	tSirMacChanInfo *ch_pwr_set;
	uint8_t *pBuf = NULL;

	/* allocate maximum space for all channels */
	dataLen = WNI_CFG_VALID_CHANNEL_LIST_LEN * sizeof(tSirMacChanInfo);
	pBuf = qdf_mem_malloc(dataLen);
	if (pBuf == NULL)
		return;

	ch_pwr_set = (tSirMacChanInfo *) (pBuf);
	pEntry = csr_ll_peek_head(pList, LL_ACCESS_LOCK);
	/*
	 * write the tuples (startChan, numChan, txPower) for each channel found
	 * in the channel power list.
	 */
	while (pEntry) {
		ch_set = GET_BASE_ADDR(pEntry, tCsrChannelPowerInfo, link);
		if (1 != ch_set->interChannelOffset) {
			/*
			 * we keep the 5G channel sets internally with an
			 * interchannel offset of 4. Expand these to the right
			 * format. (inter channel offset of 1 is the only option
			 * for the triplets that 11d advertises.
			 */
			tmp_len = cbLen + (ch_set->numChannels *
						sizeof(tSirMacChanInfo));
			if (tmp_len >= dataLen) {
				/*
				 * expanding this entry will overflow our
				 * allocation
				 */
				sme_err(
					"Buffer overflow, start %d, num %d, offset %d",
					ch_set->firstChannel,
					ch_set->numChannels,
					ch_set->interChannelOffset);
				break;
			}

			for (idx = 0; idx < ch_set->numChannels; idx++) {
				ch_pwr_set->firstChanNum = (tSirMacChanNum)
					(ch_set->firstChannel + (idx *
						ch_set->interChannelOffset));
				sme_debug(
					"Setting Channel Number %d",
					ch_pwr_set->firstChanNum);
				ch_pwr_set->numChannels = 1;
				ch_pwr_set->maxTxPower =
					QDF_MIN(ch_set->txPower,
					pMac->roam.configParam.nTxPowerCap);
				sme_debug(
					"Setting Max Transmit Power %d",
					ch_pwr_set->maxTxPower);
				cbLen += sizeof(tSirMacChanInfo);
				ch_pwr_set++;
			}
		} else {
			if (cbLen >= dataLen) {
				/* this entry will overflow our allocation */
				sme_err(
					"Buffer overflow, start %d, num %d, offset %d",
					ch_set->firstChannel,
					ch_set->numChannels,
					ch_set->interChannelOffset);
				break;
			}
			ch_pwr_set->firstChanNum = ch_set->firstChannel;
			sme_debug("Setting Channel Number %d",
				ch_pwr_set->firstChanNum);
			ch_pwr_set->numChannels = ch_set->numChannels;
			ch_pwr_set->maxTxPower = QDF_MIN(ch_set->txPower,
					pMac->roam.configParam.nTxPowerCap);
			sme_debug(
				"Setting Max Tx Power %d, nTxPower %d",
				ch_pwr_set->maxTxPower,
				pMac->roam.configParam.nTxPowerCap);
			cbLen += sizeof(tSirMacChanInfo);
			ch_pwr_set++;
		}
		pEntry = csr_ll_next(pList, pEntry, LL_ACCESS_LOCK);
	}
	if (cbLen)
		cfg_set_str(pMac, cfgId, (uint8_t *) pBuf, cbLen);

	qdf_mem_free(pBuf);
}

static void csr_set_cfg_country_code(tpAniSirGlobal pMac, uint8_t *countryCode)
{
	uint8_t cc[WNI_CFG_COUNTRY_CODE_LEN];
	/* v_REGDOMAIN_t DomainId */

	sme_debug("Setting Country Code in Cfg %s", countryCode);
	qdf_mem_copy(cc, countryCode, WNI_CFG_COUNTRY_CODE_LEN);

	/*
	 * Don't program the bogus country codes that we created for Korea in
	 * the MAC. if we see the bogus country codes, program the MAC with
	 * the right country code.
	 */
	if (('K' == countryCode[0] && '1' == countryCode[1]) ||
	    ('K' == countryCode[0] && '2' == countryCode[1]) ||
	    ('K' == countryCode[0] && '3' == countryCode[1]) ||
	    ('K' == countryCode[0] && '4' == countryCode[1])) {
		/*
		 * replace the alternate Korea country codes, 'K1', 'K2', ..
		 * with 'KR' for Korea
		 */
		cc[1] = 'R';
	}
	cfg_set_str(pMac, WNI_CFG_COUNTRY_CODE, cc, WNI_CFG_COUNTRY_CODE_LEN);

	/*
	 * Need to let HALPHY know about the current domain so it can apply some
	 * domain-specific settings (TX filter...)
	 */
}

QDF_STATUS csr_get_country_code(tpAniSirGlobal pMac, uint8_t *pBuf,
				uint8_t *pbLen)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	uint32_t len;

	if (pBuf && pbLen && (*pbLen >= WNI_CFG_COUNTRY_CODE_LEN)) {
		len = *pbLen;
		status = wlan_cfg_get_str(pMac, WNI_CFG_COUNTRY_CODE, pBuf,
					&len);
		if (QDF_IS_STATUS_SUCCESS(status))
			*pbLen = (uint8_t) len;
	}

	return status;
}

void csr_set_cfg_scan_control_list(tpAniSirGlobal pMac, uint8_t *countryCode,
				   tCsrChannel *pChannelList)
{
	uint8_t i, j;
	bool found = false;
	uint8_t *pControlList = NULL;
	uint32_t len = WNI_CFG_SCAN_CONTROL_LIST_LEN;

	pControlList = qdf_mem_malloc(WNI_CFG_SCAN_CONTROL_LIST_LEN);
	if (pControlList != NULL) {
		if (IS_SIR_STATUS_SUCCESS(wlan_cfg_get_str(pMac,
					WNI_CFG_SCAN_CONTROL_LIST,
					pControlList, &len))) {
			for (i = 0; i < pChannelList->numChannels; i++) {
				for (j = 0; j < len; j += 2) {
					if (pControlList[j] ==
					    pChannelList->channelList[i]) {
						found = true;
						break;
					}
				}

				if (found) {
					/* insert a pair(channel#, flag) */
					pControlList[j + 1] =
						csr_get_scan_type(pMac,
							pControlList[j]);
					found = false;  /* reset the flag */
				}

			}
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_INFO,
				  "%s: dump scan control list", __func__);
			QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_SME,
					   QDF_TRACE_LEVEL_INFO, pControlList,
					   len);

			cfg_set_str(pMac, WNI_CFG_SCAN_CONTROL_LIST,
					pControlList, len);
		} /* Successfully getting scan control list */
		qdf_mem_free(pControlList);
	} /* AllocateMemory */
}


/**
 * csr_scan_abort_all_scans() - Abort scan on all Sessions
 * @mac_ctx: pointer to Global Mac structure
 * @reason: reason for cancelling scan
 *
 * Abort scan on all Sessions
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_scan_abort_all_scans(tpAniSirGlobal mac_ctx,
				   eCsrAbortReason reason)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t session_id;

	mac_ctx->scan.fDropScanCmd = true;
	for (session_id = 0; session_id < CSR_ROAM_SESSION_MAX; session_id++) {
		if (CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
			csr_remove_cmd_from_pending_list(
				mac_ctx,
				session_id, INVALID_SCAN_ID,
				eSmeCommandScan);
			csr_abort_scan_from_active_list(mac_ctx,
				 session_id, INVALID_SCAN_ID, eSmeCommandScan,
				 reason);
		}
	}
	mac_ctx->scan.fDropScanCmd = false;

	return status;
}

QDF_STATUS csr_scan_abort_mac_scan(tpAniSirGlobal pMac, uint8_t sessionId,
				   uint32_t scan_id, eCsrAbortReason reason)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	QDF_STATUS ret;

	pMac->scan.fDropScanCmd = true;
	ret = csr_remove_cmd_from_pending_list(pMac,
			sessionId, scan_id, eSmeCommandScan);
	pMac->scan.fDropScanCmd = false;

	/*
	 * If we are not able to find command for scan id in
	 * pending list, check active list. Also if the session
	 * id is valid then we have to check below active list.
	 */
	if (ret != QDF_STATUS_SUCCESS ||
			sessionId != CSR_SESSION_ID_INVALID) {
		status = csr_abort_scan_from_active_list(pMac,
				sessionId, scan_id,
				eSmeCommandScan, reason);
	}
	return status;
}
QDF_STATUS csr_remove_nonscan_cmd_from_pending_list(tpAniSirGlobal pMac,
						uint8_t sessionId,
						eSmeCommandType commandType)
{
	tDblLinkList localList;
	tListElem *pEntry;
	tSmeCmd *pCommand;
	tListElem *pEntryToRemove;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	qdf_mem_zero(&localList, sizeof(tDblLinkList));
	if (!QDF_IS_STATUS_SUCCESS(csr_ll_open(pMac->hHdd, &localList))) {
		sme_err("failed to open list");
		return status;
	}

	csr_nonscan_pending_ll_lock(pMac);
	pEntry = csr_nonscan_pending_ll_peek_head(pMac, LL_ACCESS_NOLOCK);

	/*
	 * Have to make sure we don't loop back to the head of the list,
	 * which will happen if the entry is NOT on the list
	 */
	while (pEntry) {
		pEntryToRemove = pEntry;
		pEntry = csr_nonscan_pending_ll_next(pMac,
					pEntry, LL_ACCESS_NOLOCK);
		pCommand = GET_BASE_ADDR(pEntryToRemove, tSmeCmd, Link);

		if ((pCommand->command == commandType) &&
		    (pCommand->sessionId == sessionId)) {
			/* Remove that entry only */
			if (csr_nonscan_pending_ll_remove_entry(pMac,
						pEntryToRemove,
			    LL_ACCESS_NOLOCK)) {
				csr_ll_insert_tail(&localList, pEntryToRemove,
						   LL_ACCESS_NOLOCK);
				status = QDF_STATUS_SUCCESS;
			}
		}
	}
	csr_nonscan_pending_ll_unlock(pMac);

	while ((pEntry = csr_ll_remove_head(&localList, LL_ACCESS_NOLOCK))) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		sme_debug("Sending abort for command ID %d",
			(commandType == eSmeCommandScan) ? pCommand->u.
			scanCmd.scanID : sessionId);
		csr_release_command(pMac, pCommand);
	}

	csr_ll_close(&localList);
	return status;
}

QDF_STATUS csr_remove_cmd_from_pending_list(tpAniSirGlobal pMac,
						uint8_t sessionId,
						uint32_t scan_id,
						eSmeCommandType commandType)
{
	tDblLinkList localList;
	tListElem *pEntry;
	tSmeCmd *pCommand;
	tListElem *pEntryToRemove;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	qdf_mem_zero(&localList, sizeof(tDblLinkList));
	if (!QDF_IS_STATUS_SUCCESS(csr_ll_open(pMac->hHdd, &localList))) {
		sme_err("failed to open list");
		return status;
	}

	csr_scan_pending_ll_lock(pMac);
	pEntry = csr_scan_pending_ll_peek_head(pMac, LL_ACCESS_NOLOCK);

	/*
	 * Have to make sure we don't loop back to the head of the list,
	 * which will happen if the entry is NOT on the list
	 */
	while (pEntry) {
		pEntryToRemove = pEntry;
		pEntry = csr_scan_pending_ll_next(pMac, pEntry,
						LL_ACCESS_NOLOCK);
		pCommand = GET_BASE_ADDR(pEntryToRemove, tSmeCmd, Link);

		if ((pCommand->command == commandType) &&
		    (((commandType == eSmeCommandScan) &&
		    (pCommand->u.scanCmd.scanID == scan_id)) ||
		    (pCommand->sessionId == sessionId))) {
			/* Remove that entry only */
			if (csr_scan_pending_ll_remove_entry(pMac,
					pEntryToRemove, LL_ACCESS_NOLOCK)) {
				csr_ll_insert_tail(&localList, pEntryToRemove,
						   LL_ACCESS_NOLOCK);
				status = QDF_STATUS_SUCCESS;
			}
		}
	}
	csr_scan_pending_ll_unlock(pMac);

	while ((pEntry = csr_ll_remove_head(&localList, LL_ACCESS_NOLOCK))) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		sme_debug("Sending abort for command ID %d",
			(commandType == eSmeCommandScan) ? pCommand->u.
			scanCmd.scanID : sessionId);
		csr_release_command(pMac, pCommand);
	}

	csr_ll_close(&localList);
	return status;
}

QDF_STATUS csr_scan_abort_scan_for_ssid(tpAniSirGlobal pMac, uint32_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	pMac->scan.fDropScanCmd = true;
	csr_remove_scan_for_ssid_from_pending_list(pMac, sessionId);
	pMac->scan.fDropScanCmd = false;
	csr_abort_scan_from_active_list(pMac,
			sessionId, INVALID_SCAN_ID, eSmeCommandScan,
			eCSR_SCAN_ABORT_SSID_ONLY);
	return status;
}

void csr_remove_scan_for_ssid_from_pending_list(tpAniSirGlobal pMac,
						uint32_t sessionId)
{
	tDblLinkList localList;
	tListElem *pEntry;
	tSmeCmd *pCommand;
	tListElem *pEntryToRemove;

	qdf_mem_zero(&localList, sizeof(tDblLinkList));
	if (!QDF_IS_STATUS_SUCCESS(csr_ll_open(pMac->hHdd, &localList))) {
		sme_err("failed to open list");
		return;
	}
	csr_scan_pending_ll_lock(pMac);
	if (!csr_scan_pending_ll_is_list_empty(pMac, LL_ACCESS_NOLOCK)) {
		pEntry = csr_scan_pending_ll_peek_head(pMac, LL_ACCESS_NOLOCK);
		/*
		 * Have to make sure we don't loop back to the head of the list,
		 * which will happen if the entry is NOT on the list...
		 */
		while (pEntry) {
			pEntryToRemove = pEntry;
			pEntry = csr_scan_pending_ll_next(pMac, pEntry,
						LL_ACCESS_NOLOCK);
			pCommand = GET_BASE_ADDR(pEntryToRemove, tSmeCmd, Link);

			if (!((eSmeCommandScan == pCommand->command) &&
			    (sessionId == pCommand->sessionId)))
				continue;
			if (eCsrScanForSsid != pCommand->u.scanCmd.reason)
				continue;
			/* Remove that entry only */
			if (csr_scan_pending_ll_remove_entry(pMac,
					pEntryToRemove, LL_ACCESS_NOLOCK)) {
				csr_ll_insert_tail(&localList, pEntryToRemove,
						   LL_ACCESS_NOLOCK);
			}
		}
	}
	csr_scan_pending_ll_unlock(pMac);
	while ((pEntry = csr_ll_remove_head(&localList, LL_ACCESS_NOLOCK))) {
		pCommand = GET_BASE_ADDR(pEntry, tSmeCmd, Link);
		csr_release_command(pMac, pCommand);
	}
	csr_ll_close(&localList);
}


/**
 * csr_send_scan_abort() -  Sends scan abort command to firmware
 * @mac_ctx: Pointer to Global Mac structure
 * @session_id: CSR session identification
 * @scan_id: scan identifier
 *
 * .Sends scan abort command to firmware
 *
 * Return: None
 */
static void csr_send_scan_abort(tpAniSirGlobal mac_ctx,
	uint32_t session_id, uint32_t scan_id)
{
	tSirSmeScanAbortReq *msg;
	uint16_t msg_len;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	msg_len = (uint16_t)(sizeof(tSirSmeScanAbortReq));
	msg = qdf_mem_malloc(msg_len);
	if (NULL == msg) {
		sme_err("Failed to alloc memory for SmeScanAbortReq");
		return;
	}
	msg->type = eWNI_SME_SCAN_ABORT_IND;
	msg->msgLen = msg_len;
	msg->sessionId = session_id;
	msg->scan_id = scan_id;
	sme_debug(
		"Abort scan sent to Firmware scan_id %d session %d",
		scan_id, session_id);
	status = umac_send_mb_message_to_mac(msg);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Failed to send abort scan.scan_id %d session %d",
			scan_id, session_id);
	}
}

/**
 * csr_abort_scan_from_active_list() -  Remove Scan command from active list
 * @mac_ctx: Pointer to Global Mac structure
 * @list: pointer to scan active list
 * @session_id: CSR session identification
 * @scan_id: scan id
 * @scan_cmd_type: scan command type
 * @abort_reason: abort reason
 *
 * Remove Scan command from active scan list by matching either the scan id
 * or session id.
 *
 * Return: Success - QDF_STATUS_SUCCESS, Failure - error number
 */
QDF_STATUS csr_abort_scan_from_active_list(tpAniSirGlobal mac_ctx,
		uint32_t session_id, uint32_t scan_id,
		eSmeCommandType scan_cmd_type, eCsrAbortReason abort_reason)
{
	tListElem *entry;
	tSmeCmd *cmd;
	tListElem *entry_remove;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	csr_scan_active_ll_lock(mac_ctx);
	if (!csr_scan_active_ll_is_list_empty(mac_ctx, LL_ACCESS_NOLOCK)) {
		entry = csr_scan_active_ll_peek_head(mac_ctx, LL_ACCESS_NOLOCK);
		while (entry) {
			entry_remove = entry;
			entry = csr_scan_active_ll_next(mac_ctx, entry,
						LL_ACCESS_NOLOCK);
			cmd = GET_BASE_ADDR(entry_remove, tSmeCmd, Link);

			/*skip if abort reason is for SSID*/
			if ((abort_reason == eCSR_SCAN_ABORT_SSID_ONLY) &&
				(eCsrScanForSsid != cmd->u.scanCmd.reason))
				continue;
			/*
			 * Do not skip if command and either session id
			 * or scan id is matched
			 */
			if ((cmd->command == scan_cmd_type) &&
			    ((cmd->u.scanCmd.scanID == scan_id) ||
			    (cmd->sessionId == session_id))) {
				if (abort_reason ==
				    eCSR_SCAN_ABORT_DUE_TO_BAND_CHANGE)
					cmd->u.scanCmd.abort_scan_indication =
					eCSR_SCAN_ABORT_DUE_TO_BAND_CHANGE;

				csr_send_scan_abort(mac_ctx, cmd->sessionId,
						    cmd->u.scanCmd.scanID);

			}
		}
	}
	csr_scan_active_ll_unlock(mac_ctx);

	return status;
}


QDF_STATUS csr_scan_abort_mac_scan_not_for_connect(tpAniSirGlobal pMac,
						   uint8_t sessionId)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	if (!csr_is_scan_for_roam_command_active(pMac, sessionId)) {
		/*
		 * Only abort the scan if it is not used for other roam/connect
		 * purpose
		 */
		status = csr_scan_abort_mac_scan(pMac, sessionId,
				INVALID_SCAN_ID, eCSR_SCAN_ABORT_DEFAULT);
	}
	return status;
}
bool csr_roam_is_valid_channel(tpAniSirGlobal pMac, uint8_t channel)
{
	bool fValid = false;
	uint32_t idx_valid_ch;
	uint32_t len = pMac->roam.numValidChannels;

	for (idx_valid_ch = 0; (idx_valid_ch < len); idx_valid_ch++) {
		if (channel == pMac->roam.validChannelList[idx_valid_ch]) {
			fValid = true;
			break;
		}
	}
	return fValid;
}

QDF_STATUS csr_scan_create_entry_in_scan_cache(tpAniSirGlobal pMac,
					       uint32_t sessionId,
					       struct qdf_mac_addr bssid,
					       uint8_t channel)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tDot11fBeaconIEs *pNewIes = NULL;
	tCsrRoamSession *pSession = CSR_GET_SESSION(pMac, sessionId);
	tSirBssDescription *pNewBssDescriptor = NULL;
	uint32_t size = 0;

	if (NULL == pSession) {
		status = QDF_STATUS_E_FAILURE;
		return status;
	}
	sme_debug("Current bssid::"MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(pSession->pConnectBssDesc->bssId));
	sme_debug("My bssid::"MAC_ADDRESS_STR" channel %d",
		MAC_ADDR_ARRAY(bssid.bytes), channel);

	if (!QDF_IS_STATUS_SUCCESS(csr_get_parsed_bss_description_ies(
					pMac, pSession->pConnectBssDesc,
					&pNewIes))) {
		sme_err("Failed to parse IEs");
		status = QDF_STATUS_E_FAILURE;
		goto free_mem;
	}
	size = pSession->pConnectBssDesc->length +
		sizeof(pSession->pConnectBssDesc->length);
	if (!size) {
		sme_err("length of bss descriptor is 0");
		status = QDF_STATUS_E_FAILURE;
		goto free_mem;
	}
	pNewBssDescriptor = qdf_mem_malloc(size);
	if (NULL == pNewBssDescriptor) {
		sme_err("memory allocation failed");
		status = QDF_STATUS_E_FAILURE;
		goto free_mem;
	}
	qdf_mem_copy(pNewBssDescriptor, pSession->pConnectBssDesc, size);
	/* change the BSSID & channel as passed */
	qdf_mem_copy(pNewBssDescriptor->bssId, bssid.bytes,
			sizeof(tSirMacAddr));
	pNewBssDescriptor->channelId = channel;
	if (NULL == csr_scan_append_bss_description(pMac, pNewBssDescriptor,
						pNewIes, true, sessionId)) {
		sme_err("csr_scan_append_bss_description failed");
		status = QDF_STATUS_E_FAILURE;
		goto free_mem;
	}
	sme_err("entry successfully added in scan cache");

free_mem:
	if (pNewIes)
		qdf_mem_free(pNewIes);

	if (pNewBssDescriptor)
		qdf_mem_free(pNewBssDescriptor);

	return status;
}

#ifdef FEATURE_WLAN_ESE
/*  Update the TSF with the difference in system time */
void update_cckmtsf(uint32_t *timeStamp0, uint32_t *timeStamp1,
		    uint64_t *incr)
{
	uint64_t timeStamp64 = ((uint64_t) *timeStamp1 << 32) | (*timeStamp0);

	timeStamp64 = (uint64_t)(timeStamp64 + (*incr));
	*timeStamp0 = (uint32_t) (timeStamp64 & 0xffffffff);
	*timeStamp1 = (uint32_t) ((timeStamp64 >> 32) & 0xffffffff);
}
#endif

/**
 * csr_scan_save_roam_offload_ap_to_scan_cache
 * This function parses the received beacon/probe response
 * from the firmware as part of the roam synch indication.
 * The beacon or the probe response is parsed and is also
 * saved into the scan cache
 *
 * @param  pMac Pointer to Global Mac
 * @param  roam_sync_ind_ptr Roam Synch Indication from
 *         firmware which also contains the beacon/probe
 *         response
 * @return Status
 */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS csr_scan_save_roam_offload_ap_to_scan_cache(tpAniSirGlobal pMac,
		roam_offload_synch_ind *roam_sync_ind_ptr,
		tpSirBssDescription  bss_desc_ptr)
{
	uint32_t length = 0;
	tDot11fBeaconIEs *ies_local_ptr = NULL;
	tCsrScanResult *scan_res_ptr = NULL;
	uint8_t session_id = roam_sync_ind_ptr->roamedVdevId;

	length = roam_sync_ind_ptr->beaconProbeRespLength -
		(SIR_MAC_HDR_LEN_3A + SIR_MAC_B_PR_SSID_OFFSET);
	scan_res_ptr = qdf_mem_malloc(sizeof(tCsrScanResult) + length);
	if (scan_res_ptr == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				" fail to allocate memory for frame");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(&scan_res_ptr->Result.BssDescriptor,
			bss_desc_ptr,
			(sizeof(tSirBssDescription) + length));
	ies_local_ptr = (tDot11fBeaconIEs *)(scan_res_ptr->Result.pvIes);
	if (!ies_local_ptr &&
		(!QDF_IS_STATUS_SUCCESS(csr_get_parsed_bss_description_ies(
						pMac, &scan_res_ptr->Result.
						BssDescriptor,
						&ies_local_ptr)))) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				"%s:Cannot Parse IEs", __func__);
		csr_free_scan_result_entry(pMac, scan_res_ptr);
		return QDF_STATUS_E_RESOURCES;
	}

	sme_debug("LFR3:Add BSSID to scan cache" MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(scan_res_ptr->Result.BssDescriptor.bssId));
	csr_scan_add_result(pMac, scan_res_ptr, ies_local_ptr, session_id);
	if ((scan_res_ptr->Result.pvIes == NULL) && ies_local_ptr)
		qdf_mem_free(ies_local_ptr);
	return QDF_STATUS_SUCCESS;
}
#endif
/**
 * csr_get_fst_bssdescr_ptr() - This function returns the pointer to first bss
 * description from scan handle
 * @result_handle: an object for the result.
 *
 * Return: first bss descriptor from the scan handle.
 */
tpSirBssDescription csr_get_fst_bssdescr_ptr(tScanResultHandle result_handle)
{
	tListElem *first_element = NULL;
	tCsrScanResult *scan_result = NULL;
	tScanResultList *bss_list = (tScanResultList *)result_handle;

	if (NULL == bss_list) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Empty bss_list"));
		return NULL;
	}
	if (csr_ll_is_list_empty(&bss_list->List, LL_ACCESS_NOLOCK)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("bss_list->List is empty"));
		return NULL;
	}
	first_element = csr_ll_peek_head(&bss_list->List, LL_ACCESS_NOLOCK);
	if (NULL == first_element) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("peer head return NULL"));
		return NULL;
	}

	scan_result = GET_BASE_ADDR(first_element, tCsrScanResult, Link);

	return &scan_result->Result.BssDescriptor;
}

/**
 * csr_get_bssdescr_from_scan_handle() - This function to extract
 *                                       first bss description from scan handle
 * @result_handle: an object for the result.
 *
 * This function is written to extract first bss from scan handle.
 *
 * Return: first bss descriptor from the scan handle.
 */
tSirBssDescription*
csr_get_bssdescr_from_scan_handle(tScanResultHandle result_handle,
				tSirBssDescription *bss_descr)
{
	tListElem *first_element = NULL;
	tCsrScanResult *scan_result = NULL;
	tScanResultList *bss_list = (tScanResultList *)result_handle;

	if (NULL == bss_list) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("Empty bss_list"));
		return NULL;
	}
	if (csr_ll_is_list_empty(&bss_list->List, LL_ACCESS_NOLOCK)) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("bss_list->List is empty"));
		qdf_mem_free(bss_list);
		return NULL;
	}
	first_element = csr_ll_peek_head(&bss_list->List, LL_ACCESS_NOLOCK);
	if (first_element) {
		scan_result = GET_BASE_ADDR(first_element,
				tCsrScanResult,
				Link);
		qdf_mem_copy(bss_descr,
				&scan_result->Result.BssDescriptor,
				sizeof(tSirBssDescription));
	}
	return bss_descr;
}

/**
 * scan_active_list_cmd_timeout_handle() - To handle scan active command timeout
 * @userData: scan context
 *
 * This routine is to handle scan active command timeout
 *
 * Return: None
 */
void csr_scan_active_list_timeout_handle(void *userData)
{
	tSmeCmd *scan_cmd = (tSmeCmd *) userData;
	tHalHandle *hal_ctx = cds_get_context(QDF_MODULE_ID_PE);
	tpAniSirGlobal mac_ctx;
	uint16_t scan_id;
	tSirSmeScanAbortReq *msg;
	uint16_t msg_len;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (scan_cmd == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("Scan Timeout: Scan command is NULL"));
		return;
	}
	if (hal_ctx == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("HAL Context is NULL"));
		return;
	}
	mac_ctx = PMAC_STRUCT(hal_ctx);
	scan_id = scan_cmd->u.scanCmd.scanID;
	sme_err("Scan Timeout:Sending abort to Firmware ID %d session %d",
		scan_id, scan_cmd->sessionId);
	msg_len = (uint16_t)(sizeof(tSirSmeScanAbortReq));
	msg = qdf_mem_malloc(msg_len);
	if (NULL == msg) {
		sme_err("Failed to alloc memory for SmeScanAbortReq");
		return;
	}
	msg->type = eWNI_SME_SCAN_ABORT_IND;
	msg->msgLen = msg_len;
	msg->sessionId = scan_cmd->sessionId;
	msg->scan_id = scan_id;
	status = umac_send_mb_message_to_mac(msg);
	if (!QDF_IS_STATUS_SUCCESS(status))
		sme_err("Failed to post message to LIM");

	scan_cmd->u.scanCmd.status = eCSR_SCAN_FAILURE;
	csr_release_command(mac_ctx, scan_cmd);
}

#ifdef NAPIER_SCAN
static enum wlan_auth_type csr_covert_auth_type_new(eCsrAuthType auth)
{
	switch (auth) {
	case eCSR_AUTH_TYPE_NONE:
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		return WLAN_AUTH_TYPE_OPEN_SYSTEM;
	case eCSR_AUTH_TYPE_SHARED_KEY:
		return WLAN_AUTH_TYPE_SHARED_KEY;
	case eCSR_AUTH_TYPE_AUTOSWITCH:
		return WLAN_AUTH_TYPE_AUTOSWITCH;
	case eCSR_AUTH_TYPE_WPA:
		return WLAN_AUTH_TYPE_WPA;
	case eCSR_AUTH_TYPE_WPA_PSK:
		return WLAN_AUTH_TYPE_WPA_PSK;
	case eCSR_AUTH_TYPE_WPA_NONE:
		return WLAN_AUTH_TYPE_WPA_NONE;
	case eCSR_AUTH_TYPE_RSN:
		return WLAN_AUTH_TYPE_RSN;
	case eCSR_AUTH_TYPE_RSN_PSK:
		return WLAN_AUTH_TYPE_RSN_PSK;
	case eCSR_AUTH_TYPE_FT_RSN:
		return WLAN_AUTH_TYPE_FT_RSN;
	case eCSR_AUTH_TYPE_FT_RSN_PSK:
		return WLAN_AUTH_TYPE_FT_RSN_PSK;
	case eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE:
		return WLAN_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
	case eCSR_AUTH_TYPE_WAPI_WAI_PSK:
		return WLAN_AUTH_TYPE_WAPI_WAI_PSK;
	case eCSR_AUTH_TYPE_CCKM_WPA:
		return WLAN_AUTH_TYPE_CCKM_WPA;
	case eCSR_AUTH_TYPE_CCKM_RSN:
		return WLAN_AUTH_TYPE_CCKM_RSN;
	case eCSR_AUTH_TYPE_RSN_PSK_SHA256:
		return WLAN_AUTH_TYPE_RSN_PSK_SHA256;
	case eCSR_AUTH_TYPE_RSN_8021X_SHA256:
		return WLAN_AUTH_TYPE_RSN_8021X_SHA256;
	case eCSR_NUM_OF_SUPPORT_AUTH_TYPE:
	default:
		return WLAN_AUTH_TYPE_OPEN_SYSTEM;
	}
}

static eCsrAuthType csr_covert_auth_type_old(enum wlan_auth_type auth)
{
	switch (auth) {
	case WLAN_AUTH_TYPE_OPEN_SYSTEM:
		return eCSR_AUTH_TYPE_OPEN_SYSTEM;
	case WLAN_AUTH_TYPE_SHARED_KEY:
		return eCSR_AUTH_TYPE_SHARED_KEY;
	case WLAN_AUTH_TYPE_AUTOSWITCH:
		return eCSR_AUTH_TYPE_AUTOSWITCH;
	case WLAN_AUTH_TYPE_WPA:
		return eCSR_AUTH_TYPE_WPA;
	case WLAN_AUTH_TYPE_WPA_PSK:
		return eCSR_AUTH_TYPE_WPA_PSK;
	case WLAN_AUTH_TYPE_WPA_NONE:
		return eCSR_AUTH_TYPE_WPA_NONE;
	case WLAN_AUTH_TYPE_RSN:
		return eCSR_AUTH_TYPE_RSN;
	case WLAN_AUTH_TYPE_RSN_PSK:
		return eCSR_AUTH_TYPE_RSN_PSK;
	case WLAN_AUTH_TYPE_FT_RSN:
		return eCSR_AUTH_TYPE_FT_RSN;
	case WLAN_AUTH_TYPE_FT_RSN_PSK:
		return eCSR_AUTH_TYPE_FT_RSN_PSK;
	case WLAN_AUTH_TYPE_WAPI_WAI_CERTIFICATE:
		return eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE;
	case WLAN_AUTH_TYPE_WAPI_WAI_PSK:
		return eCSR_AUTH_TYPE_WAPI_WAI_PSK;
	case WLAN_AUTH_TYPE_CCKM_WPA:
		return eCSR_AUTH_TYPE_CCKM_WPA;
	case WLAN_AUTH_TYPE_CCKM_RSN:
		return eCSR_AUTH_TYPE_CCKM_RSN;
	case WLAN_AUTH_TYPE_RSN_PSK_SHA256:
		return eCSR_AUTH_TYPE_RSN_PSK_SHA256;
	case WLAN_AUTH_TYPE_RSN_8021X_SHA256:
		return eCSR_AUTH_TYPE_RSN_8021X_SHA256;
	case WLAN_NUM_OF_SUPPORT_AUTH_TYPE:
	default:
		return eCSR_AUTH_TYPE_OPEN_SYSTEM;
	}
}

static enum wlan_enc_type csr_covert_enc_type_new(eCsrEncryptionType enc)
{
	switch (enc) {
	case eCSR_ENCRYPT_TYPE_NONE:
		return WLAN_ENCRYPT_TYPE_NONE;
	case eCSR_ENCRYPT_TYPE_WEP40_STATICKEY:
		return WLAN_ENCRYPT_TYPE_WEP40_STATICKEY;
	case eCSR_ENCRYPT_TYPE_WEP104_STATICKEY:
		return WLAN_ENCRYPT_TYPE_WEP104_STATICKEY;
	case eCSR_ENCRYPT_TYPE_WEP40:
		return WLAN_ENCRYPT_TYPE_WEP40;
	case eCSR_ENCRYPT_TYPE_WEP104:
		return WLAN_ENCRYPT_TYPE_WEP104;
	case eCSR_ENCRYPT_TYPE_TKIP:
		return WLAN_ENCRYPT_TYPE_TKIP;
	case eCSR_ENCRYPT_TYPE_AES:
		return WLAN_ENCRYPT_TYPE_AES;
	case eCSR_ENCRYPT_TYPE_WPI:
		return WLAN_ENCRYPT_TYPE_WPI;
	case eCSR_ENCRYPT_TYPE_KRK:
		return WLAN_ENCRYPT_TYPE_KRK;
	case eCSR_ENCRYPT_TYPE_BTK:
		return WLAN_ENCRYPT_TYPE_BTK;
	case eCSR_ENCRYPT_TYPE_AES_CMAC:
		return WLAN_ENCRYPT_TYPE_AES_CMAC;
	case eCSR_ENCRYPT_TYPE_ANY:
	default:
		return WLAN_ENCRYPT_TYPE_NONE;
	}
}

static eCsrEncryptionType csr_covert_enc_type_old(enum wlan_enc_type enc)
{
	switch (enc) {
	case WLAN_ENCRYPT_TYPE_NONE:
		return eCSR_ENCRYPT_TYPE_NONE;
	case WLAN_ENCRYPT_TYPE_WEP40_STATICKEY:
		return eCSR_ENCRYPT_TYPE_WEP40_STATICKEY;
	case WLAN_ENCRYPT_TYPE_WEP104_STATICKEY:
		return eCSR_ENCRYPT_TYPE_WEP104_STATICKEY;
	case WLAN_ENCRYPT_TYPE_WEP40:
		return eCSR_ENCRYPT_TYPE_WEP40;
	case WLAN_ENCRYPT_TYPE_WEP104:
		return eCSR_ENCRYPT_TYPE_WEP104;
	case WLAN_ENCRYPT_TYPE_TKIP:
		return eCSR_ENCRYPT_TYPE_TKIP;
	case WLAN_ENCRYPT_TYPE_AES:
		return eCSR_ENCRYPT_TYPE_AES;
	case WLAN_ENCRYPT_TYPE_WPI:
		return eCSR_ENCRYPT_TYPE_WPI;
	case WLAN_ENCRYPT_TYPE_KRK:
		return eCSR_ENCRYPT_TYPE_KRK;
	case WLAN_ENCRYPT_TYPE_BTK:
		return eCSR_ENCRYPT_TYPE_BTK;
	case WLAN_ENCRYPT_TYPE_AES_CMAC:
		return eCSR_ENCRYPT_TYPE_AES_CMAC;
	case WLAN_ENCRYPT_TYPE_ANY:
	default:
		return eCSR_ENCRYPT_TYPE_NONE;
	}
}


static QDF_STATUS csr_prepare_scan_filter(tpAniSirGlobal mac_ctx,
	tCsrScanResultFilter *pFilter, struct scan_filter *filter)
{
	int i;
	uint32_t len = 0;
	QDF_STATUS status;
	enum policy_mgr_con_mode new_mode;
	uint8_t weight_list[QDF_MAX_NUM_CHAN];

	filter->num_of_bssid = pFilter->BSSIDs.numOfBSSIDs;
	if (filter->num_of_bssid > WLAN_SCAN_FILTER_NUM_BSSID)
		filter->num_of_bssid = WLAN_SCAN_FILTER_NUM_BSSID;
	for (i = 0; i < filter->num_of_bssid; i++)
		qdf_mem_copy(filter->bssid_list[i].bytes,
			pFilter->BSSIDs.bssid[i].bytes,
			QDF_MAC_ADDR_SIZE);

	filter->num_of_ssid = pFilter->SSIDs.numOfSSIDs;
	if (filter->num_of_ssid > WLAN_SCAN_FILTER_NUM_SSID)
		filter->num_of_ssid = WLAN_SCAN_FILTER_NUM_SSID;
	for (i = 0; i < filter->num_of_ssid; i++) {
		filter->ssid_list[i].length =
			pFilter->SSIDs.SSIDList[i].SSID.length;
		qdf_mem_copy(filter->ssid_list[i].ssid,
			pFilter->SSIDs.SSIDList[i].SSID.ssId,
			filter->ssid_list[i].length);
	}

	filter->num_of_channels =
		pFilter->ChannelInfo.numOfChannels;
	if (filter->num_of_channels > QDF_MAX_NUM_CHAN)
		filter->num_of_channels = QDF_MAX_NUM_CHAN;
	qdf_mem_copy(filter->channel_list,
			pFilter->ChannelInfo.ChannelList,
			filter->num_of_channels);

	filter->num_of_auth =
		pFilter->authType.numEntries;
	if (filter->num_of_auth > WLAN_NUM_OF_SUPPORT_AUTH_TYPE)
		filter->num_of_auth = WLAN_NUM_OF_SUPPORT_AUTH_TYPE;
	for (i = 0; i < filter->num_of_auth; i++)
		filter->auth_type[i] =
		  csr_covert_auth_type_new(pFilter->authType.authType[i]);
	filter->num_of_enc_type =
		pFilter->EncryptionType.numEntries;
	if (filter->num_of_enc_type > WLAN_NUM_OF_ENCRYPT_TYPE)
		filter->num_of_enc_type = WLAN_NUM_OF_ENCRYPT_TYPE;
	for (i = 0; i < filter->num_of_enc_type; i++)
		filter->enc_type[i] =
		  csr_covert_enc_type_new(
		  pFilter->EncryptionType.encryptionType[i]);
	filter->num_of_mc_enc_type =
			pFilter->mcEncryptionType.numEntries;
	if (filter->num_of_mc_enc_type > WLAN_NUM_OF_ENCRYPT_TYPE)
		filter->num_of_mc_enc_type = WLAN_NUM_OF_ENCRYPT_TYPE;
	for (i = 0; i < filter->num_of_mc_enc_type; i++)
		filter->mc_enc_type[i] =
		  csr_covert_enc_type_new(
		  pFilter->mcEncryptionType.encryptionType[i]);

	qdf_mem_copy(filter->country,
		pFilter->countryCode, WNI_CFG_COUNTRY_CODE_LEN);

	if (pFilter->bWPSAssociation || pFilter->bWPSAssociation)
		filter->ignore_auth_enc_type = true;

	filter->rrm_measurement_filter = pFilter->fMeasurement;

	filter->mobility_domain = pFilter->MDID.mobilityDomain;

	filter->p2p_results = pFilter->p2pResult;

	if (pFilter->MFPCapable || pFilter->MFPEnabled)
		filter->pmf_cap = WLAN_PMF_CAPABLE;
	if (pFilter->MFPRequired)
		filter->pmf_cap = WLAN_PMF_REQUIRED;

	if (pFilter->BSSType == eCSR_BSS_TYPE_INFRASTRUCTURE)
		filter->bss_type = WLAN_TYPE_BSS;
	else if (pFilter->BSSType == eCSR_BSS_TYPE_IBSS ||
		pFilter->BSSType == eCSR_BSS_TYPE_START_IBSS)
		filter->bss_type = WLAN_TYPE_IBSS;
	else
		filter->bss_type = WLAN_TYPE_ANY;

	filter->dot11_mode = pFilter->phyMode;

	if (!pFilter->BSSIDs.numOfBSSIDs) {
		if (policy_mgr_map_concurrency_mode(
		   &pFilter->csrPersona, &new_mode)) {
			status = policy_mgr_get_pcl(mac_ctx->psoc, new_mode,
				filter->pcl_channel_list, &len,
				weight_list, QDF_ARRAY_SIZE(weight_list));
			filter->num_of_pcl_channels = (uint8_t)len;
		}
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS csr_fill_bss_from_scan_entry(tpAniSirGlobal mac_ctx,
	struct scan_cache_entry *scan_entry,
	tCsrScanResult **p_result)
{
	tDot11fBeaconIEs *bcn_ies;
	tSirBssDescription *bss_desc;
	tCsrScanResultInfo *result_info;
	tpSirMacMgmtHdr hdr;
	uint8_t *ie_ptr;
	tCsrScanResult *bss;
	uint32_t bss_len, alloc_len, ie_len;
	QDF_STATUS status;

	ie_len = util_scan_entry_ie_len(scan_entry);
	ie_ptr = util_scan_entry_ie_data(scan_entry);

	hdr = (tpSirMacMgmtHdr)scan_entry->raw_frame.ptr;

	bss_len = (uint16_t)(offsetof(tSirBssDescription,
			   ieFields[0]) + ie_len);
	alloc_len = sizeof(tCsrScanResult) + bss_len;
	bss = qdf_mem_malloc(alloc_len);

	if (!bss) {
		sme_err("could not allocate bss");
		return QDF_STATUS_E_NOMEM;
	}

	bss->AgingCount =
		(int32_t) mac_ctx->roam.configParam.agingCount;
	bss->preferValue = scan_entry->prefer_value;
	bss->capValue = scan_entry->cap_val;
	bss->ucEncryptionType =
		csr_covert_enc_type_old(scan_entry->neg_sec_info.uc_enc);
	bss->mcEncryptionType =
		csr_covert_enc_type_old(scan_entry->neg_sec_info.mc_enc);
	bss->authType =
		csr_covert_auth_type_old(scan_entry->neg_sec_info.auth_type);

	result_info = &bss->Result;
	result_info->ssId.length = scan_entry->ssid.length;
	qdf_mem_copy(result_info->ssId.ssId,
		scan_entry->ssid.ssid,
		result_info->ssId.length);
	result_info->timer = scan_entry->hidden_ssid_timestamp;

	bss_desc = &result_info->BssDescriptor;

	bss_desc->length = (uint16_t) (offsetof(tSirBssDescription,
			   ieFields[0]) - sizeof(bss_desc->length) + ie_len);

	qdf_mem_copy(bss_desc->bssId,
			scan_entry->bssid.bytes,
			QDF_MAC_ADDR_SIZE);
	bss_desc->scansystimensec = scan_entry->scan_entry_time;
	qdf_mem_copy(bss_desc->timeStamp,
		scan_entry->tsf_info.data, 8);

	bss_desc->beaconInterval = scan_entry->bcn_int;
	bss_desc->capabilityInfo = scan_entry->cap_info.value;

	if (WLAN_REG_IS_5GHZ_CH(scan_entry->channel.chan_idx))
		bss_desc->nwType = eSIR_11A_NW_TYPE;
	else if (scan_entry->phy_mode == WLAN_PHYMODE_11B)
		bss_desc->nwType = eSIR_11B_NW_TYPE;
	else
		bss_desc->nwType = eSIR_11G_NW_TYPE;

	bss_desc->rssi = scan_entry->rssi_raw;
	bss_desc->rssi_raw = scan_entry->rssi_raw;

	/* channelId what peer sent in beacon/probersp. */
	bss_desc->channelId =
		scan_entry->channel.chan_idx;
	/* channelId on which we are parked at. */
	/* used only in scan case. */
	bss_desc->channelIdSelf =
		scan_entry->channel.chan_idx;
	bss_desc->rx_channel = bss_desc->channelIdSelf;
	bss_desc->received_time =
		scan_entry->scan_entry_time;
	bss_desc->startTSF[0] =
		mac_ctx->rrm.rrmPEContext.startTSF[0];
	bss_desc->startTSF[1] =
		mac_ctx->rrm.rrmPEContext.startTSF[1];
	bss_desc->parentTSF =
		scan_entry->rrm_parent_tsf;
	bss_desc->fProbeRsp = (scan_entry->frm_subtype ==
			  IEEE80211_FC0_SUBTYPE_PROBE_RESP);
	bss_desc->seq_ctrl = hdr->seqControl;
	bss_desc->tsf_delta = scan_entry->tsf_delta;

	qdf_mem_copy((uint8_t *) &bss_desc->ieFields,
		ie_ptr, ie_len);

	status = csr_get_parsed_bss_description_ies(mac_ctx,
			  bss_desc, &bcn_ies);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_mem_free(bss);
		return status;
	}
	result_info->pvIes = bcn_ies;

	if (bcn_ies->MobilityDomain.present) {
		bss_desc->mdiePresent = true;
		qdf_mem_copy((uint8_t *)&(bss_desc->mdie[0]),
			     (uint8_t *)&(bcn_ies->MobilityDomain.MDID),
			     sizeof(uint16_t));
		bss_desc->mdie[2] =
			((bcn_ies->MobilityDomain.overDSCap << 0) |
			(bcn_ies->MobilityDomain.resourceReqCap << 1));
	}
#ifdef FEATURE_WLAN_ESE
	if (bcn_ies->QBSSLoad.present) {
		bss_desc->QBSSLoad_present = true;
		bss_desc->QBSSLoad_avail =
			bcn_ies->QBSSLoad.avail;
	}
#endif

	if (scan_entry->alt_wcn_ie.ptr) {
		bss_desc->WscIeLen = scan_entry->alt_wcn_ie.len;
		qdf_mem_copy(bss_desc->WscIeProbeRsp,
			scan_entry->alt_wcn_ie.ptr,
			scan_entry->alt_wcn_ie.len);
	}

	*p_result = bss;
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS csr_parse_scan_list(tpAniSirGlobal mac_ctx,
	tScanResultList *ret_list,
	qdf_list_t *scan_list)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrScanResult *pResult = NULL;
	struct scan_cache_node *cur_node = NULL;
	struct scan_cache_node *next_node = NULL;

	status =
		qdf_list_peek_front(scan_list,
		   (qdf_list_node_t **) &cur_node);

	while (cur_node) {
		qdf_list_peek_next(
		  scan_list,
		  (qdf_list_node_t *) cur_node,
		  (qdf_list_node_t **) &next_node);
		status = csr_fill_bss_from_scan_entry(mac_ctx,
			cur_node->entry, &pResult);
		if (QDF_IS_STATUS_ERROR(status))
			return status;
		if (pResult)
			csr_ll_insert_tail(&ret_list->List, &pResult->Link,
			   LL_ACCESS_NOLOCK);
		cur_node = next_node;
		next_node = NULL;
	}

	return status;
}
QDF_STATUS csr_scan_get_result(tpAniSirGlobal mac_ctx,
			       tCsrScanResultFilter *pFilter,
			       tScanResultHandle *results)
{
	QDF_STATUS status;
	tScanResultList *ret_list = NULL;
	qdf_list_t *list = NULL;
	struct scan_filter filter = {0};
	struct wlan_objmgr_pdev *pdev = NULL;

	if (results)
		*results = CSR_INVALID_SCANRESULT_HANDLE;

	pdev = wlan_objmgr_get_pdev_by_id(mac_ctx->psoc,
		0, WLAN_LEGACY_MAC_ID);

	if (!pdev) {
		sme_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (pFilter) {
		status = csr_prepare_scan_filter(mac_ctx, pFilter, &filter);
		if (QDF_IS_STATUS_ERROR(status)) {
			sme_err("Prepare filter failed");
			goto error;
		}
	}

	list = ucfg_scan_get_result(pdev,
		    pFilter ? &filter : NULL);
	sme_debug("num_entries %d", qdf_list_size(list));

	if (!list || !qdf_list_size(list)) {
		sme_err("get scan result failed");
		status = QDF_STATUS_E_EMPTY;
		goto error;
	}

	ret_list = qdf_mem_malloc(sizeof(tScanResultList));
	if (!ret_list) {
		sme_err("pRetList is NULL");
		status = QDF_STATUS_E_NOMEM;
		goto error;
	}

	csr_ll_open(mac_ctx->hHdd, &ret_list->List);
	ret_list->pCurEntry = NULL;
	status = csr_parse_scan_list(mac_ctx,
		ret_list, list);
	sme_debug("return %d BSS status %d",
			csr_ll_count(&ret_list->List), status);
	if (QDF_IS_STATUS_ERROR(status) || !results)
		/* Fail or No one wants the result. */
		csr_scan_result_purge(mac_ctx, (tScanResultHandle) ret_list);
	else {
		if (!csr_ll_count(&ret_list->List)) {
			/* This mean that there is no match */
			csr_ll_close(&ret_list->List);
			qdf_mem_free(ret_list);
			status = QDF_STATUS_E_NULL_VALUE;
		} else if (results) {
			*results = ret_list;
		}
	}

error:
	if (list)
		ucfg_scan_purge_results(list);
	if (pdev)
		wlan_objmgr_pdev_release_ref(pdev, WLAN_LEGACY_MAC_ID);

	return status;
}

static inline QDF_STATUS
csr_flush_scan_results(tpAniSirGlobal mac_ctx,
	struct scan_filter *filter)
{
	struct wlan_objmgr_pdev *pdev = NULL;
	QDF_STATUS status;

	pdev = wlan_objmgr_get_pdev_by_id(mac_ctx->psoc,
		0, WLAN_LEGACY_MAC_ID);
	if (!pdev) {
		sme_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}
	status = ucfg_scan_flush_results(pdev, filter);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_LEGACY_MAC_ID);
	return status;
}

QDF_STATUS csr_scan_flush_result(tpAniSirGlobal mac_ctx)
{

	if (csr_scan_flush_denied(mac_ctx)) {
		sme_err("scan flush denied in roam state");
		return QDF_STATUS_E_FAILURE;
	}

	return csr_flush_scan_results(mac_ctx, NULL);
}

QDF_STATUS csr_scan_flush_selective_result(tpAniSirGlobal mac_ctx,
	bool flush_p2p)
{
	struct scan_filter filter = {0};

	filter.p2p_results = flush_p2p;
	return csr_flush_scan_results(mac_ctx, &filter);
}

static inline void csr_flush_bssid(tpAniSirGlobal mac_ctx,
	uint8_t *bssid)
{
	struct scan_filter filter = {0};

	filter.num_of_bssid = 1;
	qdf_mem_copy(filter.bssid_list[0].bytes,
		bssid, QDF_MAC_ADDR_SIZE);

	csr_flush_scan_results(mac_ctx, &filter);
	sme_debug("Removed BSS entry:%pM", bssid);
}

void csr_scan_flush_bss_entry(tpAniSirGlobal mac_ctx,
			tpSmeCsaOffloadInd csa_off_ind)
{
	csr_flush_bssid(mac_ctx,
		csa_off_ind->bssid.bytes);
}

void csr_remove_bssid_from_scan_list(tpAniSirGlobal mac_ctx,
			tSirMacAddr bssid)
{
	csr_flush_bssid(mac_ctx, bssid);
}

QDF_STATUS csr_scan_process_single_bssdescr(tpAniSirGlobal mac_ctx,
					tSirBssDescription *bssdescr,
					uint32_t scan_id, uint32_t flags)
{
	/* Not needed now as scan module handle this*/
	return QDF_STATUS_SUCCESS;
}

void csr_init_occupied_channels_list(tpAniSirGlobal mac_ctx,
	uint8_t sessionId)
{
	tScanResultHandle results;
	tScanResultList *scan_list = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tListElem *scan_entry = NULL;
	tCsrScanResult *bss_desc = NULL;
	tDot11fBeaconIEs *ie_ptr = NULL;
	tpCsrNeighborRoamControlInfo neighbor_roam_info =
		&mac_ctx->roam.neighborRoamInfo[sessionId];

	if (neighbor_roam_info->cfgParams.channelInfo.numOfChannels) {
		/*
		 * Ini file contains neighbor scan channel list, hence NO need
		 * to build occupied channel list"
		 */
		sme_debug("Ini contains neighbor scan ch list");
		return;
	}

	if (!csr_neighbor_roam_is_new_connected_profile(mac_ctx, sessionId)) {
		/*
		 * Do not flush occupied list since current roam profile matches
		 * previous
		 */
		sme_debug("Current roam profile matches prev");
		return;
	}

	/* Empty occupied channels here */
	mac_ctx->scan.occupiedChannels[sessionId].numChannels = 0;
	mac_ctx->scan.roam_candidate_count[sessionId] = 0;

	status = csr_scan_get_result(mac_ctx, NULL, &results);
		if (!QDF_IS_STATUS_SUCCESS(status))
			return;
	scan_list = results;
	csr_ll_lock(&scan_list->List);
	scan_entry = csr_ll_peek_head(&scan_list->List, LL_ACCESS_NOLOCK);
	while (scan_entry) {
		bss_desc = GET_BASE_ADDR(scan_entry, tCsrScanResult, Link);
		ie_ptr = (tDot11fBeaconIEs *) (bss_desc->Result.pvIes);
		if (!ie_ptr && !QDF_IS_STATUS_SUCCESS(
			csr_get_parsed_bss_description_ies(mac_ctx,
				&bss_desc->Result.BssDescriptor, &ie_ptr)))
			continue;
		csr_scan_add_to_occupied_channels(mac_ctx, bss_desc, sessionId,
				&mac_ctx->scan.occupiedChannels[sessionId], ie_ptr,
				true);
		/*
		 * Free the memory allocated for pIes in
		 * csr_get_parsed_bss_description_ies
		 */
		if ((bss_desc->Result.pvIes == NULL) && ie_ptr)
			qdf_mem_free(ie_ptr);
		scan_entry = csr_ll_next(&scan_list->List, scan_entry,
				     LL_ACCESS_NOLOCK);
	}
	csr_ll_unlock(&scan_list->List);

	csr_scan_result_purge(mac_ctx, scan_list);
}

/**
 * csr_scan_filter_results: filter scan result based
 * on valid channel list number.
 * @mac_ctx: mac context
 *
 * Get scan result from scan list and Check Scan result channel number
 * with 11d channel list if channel number is found in 11d channel list
 * then do not remove scan result entry from scan list
 *
 * return: QDF Status
 */
QDF_STATUS csr_scan_filter_results(tpAniSirGlobal mac_ctx)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t len = sizeof(mac_ctx->roam.validChannelList);
	struct wlan_objmgr_pdev *pdev = NULL;

	pdev = wlan_objmgr_get_pdev_by_id(mac_ctx->psoc,
		0, WLAN_LEGACY_MAC_ID);
	if (!pdev) {
		sme_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}
	status = csr_get_cfg_valid_channels(mac_ctx,
			  mac_ctx->roam.validChannelList,
			  &len);

	/* Get valid channels list from CFG */
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_pdev_release_ref(pdev, WLAN_LEGACY_MAC_ID);
		sme_err("Failed to get Channel list from CFG");
		return status;
	}
	sme_debug("No of valid channel %d", len);

	ucfg_scan_filter_valid_channel(pdev,
		mac_ctx->roam.validChannelList, len);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_LEGACY_MAC_ID);
	return QDF_STATUS_SUCCESS;
}
#endif
