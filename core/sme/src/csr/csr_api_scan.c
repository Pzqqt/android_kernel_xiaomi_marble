/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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
#include <wlan_scan_ucfg_api.h>
#include <wlan_scan_tgt_api.h>
#include <wlan_scan_utils_api.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_utility.h>
#include "wlan_reg_services_api.h"

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

#ifndef NAPIER_SCAN

#define MIN_CHN_TIME_TO_FIND_GO 100
#define MAX_CHN_TIME_TO_FIND_GO 100
#define DIRECT_SSID_LEN 7

/* Increase dwell time for P2P search in ms */
#define P2P_SEARCH_DWELL_TIME_INCREASE   20
#define P2P_SOCIAL_CHANNELS              3

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
	if (dst_req->ChannelInfo.numOfChannels == 0) {
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
					"reqType= %d, numOfChannels=%d, ignoring DFS channel %d",
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
						BAND_2G) ||
						(WLAN_REG_IS_5GHZ_CH(
							src_req->ChannelInfo.
							ChannelList[index]) &&
					mac_ctx->roam.configParam.
					sta_roam_policy.sap_operating_band ==
						BAND_5G))) {
					QDF_TRACE(QDF_MODULE_ID_SME,
						QDF_TRACE_LEVEL_DEBUG,
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

static QDF_STATUS
csr_issue_11d_scan(tpAniSirGlobal mac_ctx, tSmeCmd *scan_cmd,
		   tCsrScanRequest *scan_req, uint16_t session_id)
{
	QDF_STATUS status;
	tSmeCmd *scan_11d_cmd = NULL;
	tCsrScanRequest tmp_rq;
	tCsrChannelInfo *chn_info = &tmp_rq.ChannelInfo;
	uint32_t num_chn = mac_ctx->scan.base_channels.numChannels;
	struct csr_roam_session *csr_session = CSR_GET_SESSION(mac_ctx,
							       session_id);

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

	qdf_mem_set(&scan_11d_cmd->u.scanCmd, sizeof(struct scan_cmd), 0);
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
	tmp_rq.BSSType = eCSR_BSS_TYPE_ANY;
	tmp_rq.scan_id = scan_11d_cmd->u.scanCmd.scanID;

	status = qdf_mc_timer_init(&scan_11d_cmd->u.scanCmd.csr_scan_timer,
			QDF_TIMER_TYPE_SW,
			csr_scan_active_list_timeout_handle, scan_11d_cmd);

	if (wlan_reg_11d_enabled_on_host(mac_ctx->psoc)) {
		tmp_rq.bcnRptReqScan = scan_req->bcnRptReqScan;
		if (scan_req->bcnRptReqScan)
			tmp_rq.scanType = scan_req->scanType ?
				eSIR_PASSIVE_SCAN : scan_req->scanType;
		else
			tmp_rq.scanType = eSIR_PASSIVE_SCAN;
		tmp_rq.requestType = eCSR_SCAN_REQUEST_11D_SCAN;
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
	struct csr_config *cfg_prm = &pMac->roam.configParam;

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

	scan_cmd = csr_get_command_buffer(pMac);
	if (!scan_cmd) {
		sme_err("scan_cmd is NULL");
		goto release_cmd;
	}

	qdf_mem_set(&scan_cmd->u.scanCmd, sizeof(struct scan_cmd), 0);
	scan_cmd->command = eSmeCommandScan;
	scan_cmd->sessionId = sessionId;
	if (scan_cmd->sessionId >= CSR_ROAM_SESSION_MAX)
		sme_err("Invalid Sme SessionID: %d", sessionId);
	scan_cmd->u.scanCmd.callback = callback;
	scan_cmd->u.scanCmd.pContext = pContext;
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

	if (csr_is_conn_state_disconnected(pMac, sessionId))
		scan_req->scan_adaptive_dwell_mode =
					cfg_prm->scan_adaptive_dwell_mode_nc;
	else
		scan_req->scan_adaptive_dwell_mode =
					cfg_prm->scan_adaptive_dwell_mode;

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
		"SId=%d scanId=%d numSSIDs=%d numChan=%d P2P search=%d minCT=%d maxCT=%d uIEFieldLen=%d BSSID: " MAC_ADDRESS_STR,
		sessionId, scan_cmd->u.scanCmd.scanID,
		pTempScanReq->SSIDs.numOfSSIDs,
		pTempScanReq->ChannelInfo.numOfChannels,
		pTempScanReq->p2pSearch, pTempScanReq->minChnTime,
		pTempScanReq->maxChnTime, pTempScanReq->uIEFieldLen,
		MAC_ADDR_ARRAY(scan_cmd->u.scanCmd.u.scanRequest.bssid.bytes));

	status = csr_queue_sme_command(pMac, scan_cmd, false);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("fail to send message status: %d", status);
		return status;
	}

release_cmd:
	if (!QDF_IS_STATUS_SUCCESS(status) && scan_cmd) {
		sme_err(" SId: %d Failed with status=%d numOfSSIDs=%d P2P search=%d scanId=%d",
			sessionId, status,
			scan_req->SSIDs.numOfSSIDs, scan_req->p2pSearch,
			scan_cmd->u.scanCmd.scanID);
		csr_release_command_buffer(pMac, scan_cmd);
	}

	return status;
}
#endif

/* pResult is invalid calling this function. */
void csr_free_scan_result_entry(tpAniSirGlobal pMac, struct tag_csrscan_result
				*pResult)
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
	struct tag_csrscan_result *pBssDesc;

	csr_ll_lock(pList);

	while ((pEntry = csr_ll_remove_head(pList, LL_ACCESS_NOLOCK)) != NULL) {
		pBssDesc = GET_BASE_ADDR(pEntry, struct tag_csrscan_result,
					Link);
		csr_free_scan_result_entry(pMac, pBssDesc);
	}

	csr_ll_unlock(pList);

	return status;
}

QDF_STATUS csr_scan_open(tpAniSirGlobal mac_ctx)
{
	csr_ll_open(mac_ctx->hHdd, &mac_ctx->scan.channelPowerInfoList24);
	csr_ll_open(mac_ctx->hHdd, &mac_ctx->scan.channelPowerInfoList5G);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS csr_scan_close(tpAniSirGlobal pMac)
{
	csr_purge_channel_power(pMac, &pMac->scan.channelPowerInfoList24);
	csr_purge_channel_power(pMac, &pMac->scan.channelPowerInfoList5G);
	csr_ll_close(&pMac->scan.channelPowerInfoList24);
	csr_ll_close(&pMac->scan.channelPowerInfoList5G);
	ucfg_scan_set_enable(pMac->psoc, false);
	return QDF_STATUS_SUCCESS;
}
QDF_STATUS csr_scan_handle_search_for_ssid(tpAniSirGlobal mac_ctx,
					   uint32_t session_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tScanResultHandle hBSSList = CSR_INVALID_SCANRESULT_HANDLE;
	tCsrScanResultFilter *pScanFilter = NULL;
	tCsrRoamProfile *profile;
	struct csr_roam_session *session;

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

/**
 * csr_handle_fils_scan_for_ssid_failure() - Checks and fills FILS seq number
 * in roam_info structure to send to hdd
 *
 * @roam_profile: Pointer to current roam_profile structure
 * @roam_info: Pointer to roam_info strucure to be filled
 *
 * Return: true for FILS connection else false
 */
#ifdef WLAN_FEATURE_FILS_SK
static
bool csr_handle_fils_scan_for_ssid_failure(tCsrRoamProfile *roam_profile,
					   struct csr_roam_info *roam_info)
{
	if (roam_profile && roam_profile->fils_con_info &&
	    roam_profile->fils_con_info->is_fils_connection) {
		sme_debug("send roam_info for FILS connection failure, seq %d",
			  roam_profile->fils_con_info->sequence_number);
		roam_info->is_fils_connection = true;
		roam_info->fils_seq_num =
				roam_profile->fils_con_info->sequence_number;
		return true;
	}

	return false;
}
#else
static
bool csr_handle_fils_scan_for_ssid_failure(tCsrRoamProfile *roam_profile,
					   struct csr_roam_info *roam_info)
{
	return false;
}
#endif

QDF_STATUS csr_scan_handle_search_for_ssid_failure(tpAniSirGlobal mac_ctx,
						  uint32_t session_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tCsrRoamProfile *profile;
	struct csr_roam_session *session = CSR_GET_SESSION(mac_ctx, session_id);
	eCsrRoamResult roam_result;
	struct csr_roam_info *roam_info = NULL;
	struct tag_csrscan_result *scan_result;

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

	roam_info = qdf_mem_malloc(sizeof(struct csr_roam_info));
	if (!roam_info) {
		sme_err("Failed to allocate memory for roam_info");
		goto roam_completion;
	}

	if (session->scan_info.roambssentry) {
		scan_result = GET_BASE_ADDR(session->scan_info.roambssentry,
				struct tag_csrscan_result, Link);
		roam_info->pBssDesc = &scan_result->Result.BssDescriptor;
	}
	roam_info->statusCode = session->joinFailStatusCode.statusCode;
	roam_info->reasonCode = session->joinFailStatusCode.reasonCode;

	/* Only indicate assoc_completion if we indicate assoc_start. */
	if (session->bRefAssocStartCnt > 0) {
		session->bRefAssocStartCnt--;
		csr_roam_call_callback(mac_ctx, session_id, roam_info,
				       session->scan_info.roam_id,
				       eCSR_ROAM_ASSOCIATION_COMPLETION,
				       eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE);
	} else {
		if (!csr_handle_fils_scan_for_ssid_failure(
		    profile, roam_info)) {
			qdf_mem_free(roam_info);
			roam_info = NULL;
		}
		csr_roam_call_callback(mac_ctx, session_id, roam_info,
				       session->scan_info.roam_id,
				       eCSR_ROAM_ASSOCIATION_FAILURE,
				       eCSR_ROAM_RESULT_SCAN_FOR_SSID_FAILURE);
	}

	if (roam_info)
		qdf_mem_free(roam_info);
roam_completion:
	csr_roam_completion(mac_ctx, session_id, NULL, NULL, roam_result,
			    false);
	return status;
}

QDF_STATUS csr_scan_result_purge(tpAniSirGlobal pMac,
				 tScanResultHandle hScanList)
{
	QDF_STATUS status = QDF_STATUS_E_INVAL;
	struct scan_result_list *pScanList =
				(struct scan_result_list *) hScanList;

	if (pScanList) {
		status = csr_ll_scan_purge_result(pMac, &pScanList->List);
		csr_ll_close(&pScanList->List);
		qdf_mem_free(pScanList);
	}
	return status;
}

/* Add the channel to the occupiedChannels array */
static void csr_scan_add_to_occupied_channels(tpAniSirGlobal pMac,
					struct tag_csrscan_result *pResult,
					uint8_t sessionId,
					struct csr_channel *occupied_ch,
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
static void csr_scan_add_result(tpAniSirGlobal mac_ctx,
				struct tag_csrscan_result *pResult)
{
	qdf_nbuf_t buf;
	uint8_t *data;
	struct mgmt_rx_event_params rx_param = {0};
	struct wlan_frame_hdr *hdr;
	struct wlan_bcn_frame *fixed_frame;
	uint32_t buf_len;
	tSirBssDescription *bss_desc;
	enum mgmt_frame_type frm_type = MGMT_BEACON;

	if (!pResult) {
		sme_err("pResult is null");
		return;
	}

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

static QDF_STATUS csr_add_pmkid_candidate_list(tpAniSirGlobal pMac,
					       uint32_t sessionId,
					       tSirBssDescription *pBssDesc,
					       tDot11fBeaconIEs *pIes)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct csr_roam_session *pSession = CSR_GET_SESSION(pMac, sessionId);
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
	struct csr_roam_session *pSession;
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
	struct csr_roam_session *pSession = CSR_GET_SESSION(pMac, sessionId);

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
	struct csr_roam_session *pSession;
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

static bool csr_scan_save_bss_description(tpAniSirGlobal pMac,
						     tSirBssDescription *
						     pBSSDescription)
{
	struct tag_csrscan_result *pCsrBssDescription = NULL;
	uint32_t cbBSSDesc;
	uint32_t cbAllocated;

	/* figure out how big the BSS description is (the BSSDesc->length does
	 * NOT include the size of the length field itself).
	 */
	cbBSSDesc = pBSSDescription->length + sizeof(pBSSDescription->length);

	cbAllocated = sizeof(struct tag_csrscan_result) + cbBSSDesc;

	pCsrBssDescription = qdf_mem_malloc(cbAllocated);
	if (!pCsrBssDescription) {
		sme_err(" Failed to allocate memory for pCsrBssDescription");
		return false;
	}

	pCsrBssDescription->AgingCount =
		(int32_t) pMac->roam.configParam.agingCount;
	sme_debug(
		"Set Aging Count = %d for BSS " MAC_ADDRESS_STR " ",
		pCsrBssDescription->AgingCount,
		MAC_ADDR_ARRAY(pCsrBssDescription->Result.BssDescriptor.
			       bssId));
	qdf_mem_copy(&pCsrBssDescription->Result.BssDescriptor,
		     pBSSDescription, cbBSSDesc);
	csr_scan_add_result(pMac, pCsrBssDescription);
	csr_free_scan_result_entry(pMac, pCsrBssDescription);

	return true;
}

/* Append a Bss Description... */
bool csr_scan_append_bss_description(tpAniSirGlobal pMac,
				     tSirBssDescription *pSirBssDescription)
{
	return csr_scan_save_bss_description(pMac,
					pSirBssDescription);
}

static void csr_purge_channel_power(tpAniSirGlobal pMac, tDblLinkList
					*pChannelList)
{
	struct csr_channel_powerinfo *pChannelSet;
	tListElem *pEntry;

	csr_ll_lock(pChannelList);
	/*
	 * Remove the channel sets from the learned list and put them
	 * in the free list
	 */
	while ((pEntry = csr_ll_remove_head(pChannelList,
					    LL_ACCESS_NOLOCK)) != NULL) {
		pChannelSet = GET_BASE_ADDR(pEntry,
					struct csr_channel_powerinfo, link);
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
	struct csr_channel_powerinfo *pChannelSet;
	bool f2GHzInfoFound = false;
	bool f2GListPurged = false, f5GListPurged = false;

	pChannelInfo = channelTable;
	/* atleast 3 bytes have to be remaining  -- from "countryString" */
	while (i--) {
	pChannelSet = qdf_mem_malloc(sizeof(struct csr_channel_powerinfo));
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
					  struct csr_channel *ch_lst,
					  uint8_t *countryCode)
{
	int i;
	uint8_t num_ch = 0;
	uint8_t tempNumChannels = 0;
	struct csr_channel tmp_ch_lst;

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
		    sizeof(struct csr_votes11d) * CSR_MAX_NUM_COUNTRY_CODE, 0);
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
	struct csr_channel_powerinfo *ch_set;

	/* Get 2.4Ghz first */
	entry = csr_ll_peek_head(list, LL_ACCESS_LOCK);
	while (entry && (chn_idx < *num_ch)) {
		ch_set = GET_BASE_ADDR(entry,
				struct csr_channel_powerinfo, link);
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

	if (!wlan_reg_11d_enabled_on_host(pMac->psoc))
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
	if (!wlan_reg_11d_enabled_on_host(pMac->psoc))
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

static enum csr_scancomplete_nextcommand csr_scan_get_next_command_state(
						tpAniSirGlobal mac_ctx,
						uint32_t session_id,
						eCsrScanStatus scan_status,
						uint8_t *chan)
{
	enum csr_scancomplete_nextcommand NextCommand = eCsrNextScanNothing;
	int8_t channel;
	struct csr_roam_session *session;

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("session %d is invalid", session_id);
		return NextCommand;
	}
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
	if (list)
		sme_debug("num_entries %d", qdf_list_size(list));
	if (!list || (list && !qdf_list_size(list))) {
		sme_err("get scan result failed");
		WLAN_HOST_DIAG_LOG_REPORT(pScanLog);
		wlan_objmgr_pdev_release_ref(pdev, WLAN_LEGACY_MAC_ID);
		if (list)
			ucfg_scan_purge_results(list);
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

/**
 * csr_saved_scan_cmd_free_fields() - Free internal fields of scan command
 *
 * @mac_ctx: Global MAC context
 * @session: sme session
 *
 * Frees data structures allocated inside saved_scan_cmd and releases
 * the profile.
 * Return: None
 */

void csr_saved_scan_cmd_free_fields(tpAniSirGlobal mac_ctx,
				    struct csr_roam_session *session)
{
	if (session->scan_info.profile) {
		csr_release_profile(mac_ctx,
				    session->scan_info.profile);
		qdf_mem_free(session->scan_info.profile);
		session->scan_info.profile = NULL;
	}

	if (session->scan_info.roambssentry) {
		qdf_mem_free(session->scan_info.roambssentry);
		session->scan_info.roambssentry = NULL;
	}
}
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
	struct tag_csrscan_result *scan_result;
	struct tag_csrscan_result *temp;
	uint32_t bss_len;
	struct csr_roam_session *session;

	/*
	 * check the session's validity first, if session itself
	 * is not valid then there is no point of releasing the memory
	 * for invalid session (i.e. "goto error" case)
	 */
	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("session %d is invalid", session_id);
		return QDF_STATUS_E_FAILURE;
	}
	session = CSR_GET_SESSION(mac_ctx, session_id);
	if (!session->scan_info.roambssentry)
		return QDF_STATUS_SUCCESS;

	scan_result = GET_BASE_ADDR(session->scan_info.roambssentry,
			struct tag_csrscan_result, Link);

	bss_len = scan_result->Result.BssDescriptor.length +
		sizeof(scan_result->Result.BssDescriptor.length);

	temp = qdf_mem_malloc(sizeof(struct tag_csrscan_result) + bss_len);
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
	csr_saved_scan_cmd_free_fields(mac_ctx, session);

	return QDF_STATUS_E_FAILURE;
}

static void csr_handle_nxt_cmd(tpAniSirGlobal mac_ctx,
		   enum csr_scancomplete_nextcommand nxt_cmd,
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

void csr_scan_callback(struct wlan_objmgr_vdev *vdev,
				struct scan_event *event, void *arg)
{
	eCsrScanStatus scan_status = eCSR_SCAN_FAILURE;
	enum csr_scancomplete_nextcommand NextCommand = eCsrNextScanNothing;
	tpAniSirGlobal mac_ctx;
	struct csr_roam_session *session;
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
	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("session %d is invalid", session_id);
		return;
	}
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

	if (session->scan_info.profile != NULL) {
		sme_debug("Free the profile scan_id %d", event->scan_id);
		csr_release_profile(mac_ctx, session->scan_info.profile);
		qdf_mem_free(session->scan_info.profile);
		session->scan_info.profile = NULL;
	}

}

tCsrScanResultInfo *csr_scan_result_get_first(tpAniSirGlobal pMac,
					      tScanResultHandle hScanResult)
{
	tListElem *pEntry;
	struct tag_csrscan_result *pResult;
	tCsrScanResultInfo *pRet = NULL;
	struct scan_result_list *pResultList =
				(struct scan_result_list *) hScanResult;

	if (pResultList) {
		csr_ll_lock(&pResultList->List);
		pEntry = csr_ll_peek_head(&pResultList->List, LL_ACCESS_NOLOCK);
		if (pEntry) {
			pResult = GET_BASE_ADDR(pEntry, struct
						tag_csrscan_result, Link);
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
	struct tag_csrscan_result *pResult = NULL;
	tCsrScanResultInfo *pRet = NULL;
	struct scan_result_list *pResultList =
				(struct scan_result_list *) hScanResult;

	if (!pResultList)
		return NULL;

	csr_ll_lock(&pResultList->List);
	if (NULL == pResultList->pCurEntry)
		pEntry = csr_ll_peek_head(&pResultList->List, LL_ACCESS_NOLOCK);
	else
		pEntry = csr_ll_next(&pResultList->List, pResultList->pCurEntry,
				     LL_ACCESS_NOLOCK);

	if (pEntry) {
		pResult = GET_BASE_ADDR(pEntry, struct tag_csrscan_result,
					Link);
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
	struct scan_result_list *pResultList =
				(struct scan_result_list *) hScanResult;
	struct tag_csrscan_result *pResult = NULL;
	tListElem *pEntry = NULL;

	if (!(pResultList && bssid))
		return status;

	csr_ll_lock(&pResultList->List);
	pEntry = csr_ll_peek_head(&pResultList->List, LL_ACCESS_NOLOCK);
	while (pEntry) {
		pResult = GET_BASE_ADDR(pEntry, struct tag_csrscan_result,
					Link);
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

QDF_STATUS csr_scan_get_pmkid_candidate_list(tpAniSirGlobal pMac,
					     uint32_t sessionId,
					     tPmkidCandidateInfo *pPmkidList,
					     uint32_t *pNumItems)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct csr_roam_session *pSession = CSR_GET_SESSION(pMac, sessionId);
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
	struct csr_roam_session *pSession = CSR_GET_SESSION(pMac, sessionId);
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
	uint8_t i, chan, num_chan = 0;
	wlan_scan_id scan_id;
	struct csr_roam_session *session = CSR_GET_SESSION(mac_ctx, session_id);

	if (!CSR_IS_SESSION_VALID(mac_ctx, session_id)) {
		sme_err("session %d is invalid", session_id);
		return status;
	}

	if (!ucfg_scan_get_enable(mac_ctx->psoc) && (num_ssid != 1)) {
		sme_err(
			"cannot scan because scanEnable (%d) or numSSID (%d) is invalid",
			ucfg_scan_get_enable(mac_ctx->psoc), profile->SSIDs.numOfSSIDs);
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
	req->scan_req.scan_f_passive = false;
	req->scan_req.scan_f_bcast_probe = false;


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
				req->scan_req.chan_list.chan[num_chan].freq =
						wlan_chan_to_freq(chan);
				num_chan++;
			}
		}
		req->scan_req.chan_list.num_chan = num_chan;
	}

	/* Extend it for multiple SSID */
	if (profile->SSIDs.numOfSSIDs) {
		if (profile->SSIDs.SSIDList[0].SSID.length > MAX_SSID_LEN) {
			sme_debug("wrong ssid length = %d",
					profile->SSIDs.SSIDList[0].SSID.length);
			status = QDF_STATUS_E_INVAL;
			goto error;
		}
		req->scan_req.num_ssids = 1;
		qdf_mem_copy(&req->scan_req.ssid[0].ssid,
				&profile->SSIDs.SSIDList[0].SSID.ssId,
				profile->SSIDs.SSIDList[0].SSID.length);
		req->scan_req.ssid[0].length =
				profile->SSIDs.SSIDList[0].SSID.length;
		sme_debug("scan for SSID = %.*s", req->scan_req.ssid[0].length,
			  req->scan_req.ssid[0].ssid);
	}
	status = ucfg_scan_start(req);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);
error:
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("failed to initiate scan with status: %d", status);
		csr_release_profile(mac_ctx, session->scan_info.profile);
		qdf_mem_free(session->scan_info.profile);
		session->scan_info.profile = NULL;
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

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
		  "%s: dump valid channel list(NumChannels(%d))",
		  __func__, NumChannels);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
			   pChannelList, NumChannels);
	cfg_set_str(pMac, WNI_CFG_VALID_CHANNEL_LIST, pChannelList,
			dataLen);

	QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
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
	struct csr_channel_powerinfo *ch_set;
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
		ch_set = GET_BASE_ADDR(pEntry,
				struct csr_channel_powerinfo, link);
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
	tSirRetStatus status;
	uint32_t len;

	if (pBuf && pbLen && (*pbLen >= WNI_CFG_COUNTRY_CODE_LEN)) {
		len = *pbLen;
		status = wlan_cfg_get_str(pMac, WNI_CFG_COUNTRY_CODE, pBuf,
					&len);
		if (status == eSIR_SUCCESS) {
			*pbLen = (uint8_t) len;
			return QDF_STATUS_SUCCESS;
		}
	}

	return QDF_STATUS_E_INVAL;
}

void csr_set_cfg_scan_control_list(tpAniSirGlobal pMac, uint8_t *countryCode,
				   struct csr_channel *pChannelList)
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
			QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_DEBUG,
				  "%s: dump scan control list", __func__);
			QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_SME,
					   QDF_TRACE_LEVEL_DEBUG, pControlList,
					   len);

			cfg_set_str(pMac, WNI_CFG_SCAN_CONTROL_LIST,
					pControlList, len);
		} /* Successfully getting scan control list */
		qdf_mem_free(pControlList);
	} /* AllocateMemory */
}

QDF_STATUS csr_scan_abort_mac_scan(tpAniSirGlobal mac_ctx, uint8_t vdev_id,
				   uint32_t scan_id)
{
	struct scan_cancel_request *req;
	QDF_STATUS status;
	struct wlan_objmgr_vdev *vdev;

	req = qdf_mem_malloc(sizeof(*req));
	if (!req) {
		sme_err("Failed to allocate memory");
		return QDF_STATUS_E_NOMEM;
	}

	/* Get NL global context from objmgr*/
	if (vdev_id == INVAL_VDEV_ID)
		vdev = wlan_objmgr_get_vdev_by_id_from_pdev(mac_ctx->pdev,
				0, WLAN_LEGACY_SME_ID);
	else
		vdev = wlan_objmgr_get_vdev_by_id_from_pdev(mac_ctx->pdev,
				vdev_id, WLAN_LEGACY_SME_ID);

	if (!vdev) {
		sme_err("Failed get vdev");
		qdf_mem_free(req);
		return QDF_STATUS_E_INVAL;
	}

	req->vdev = vdev;
	req->cancel_req.scan_id = scan_id;
	req->cancel_req.pdev_id = wlan_objmgr_pdev_get_pdev_id(mac_ctx->pdev);
	req->cancel_req.vdev_id = vdev_id;
	if (scan_id != INVAL_SCAN_ID)
		req->cancel_req.req_type = WLAN_SCAN_CANCEL_SINGLE;
	if (vdev_id == INVAL_VDEV_ID)
		req->cancel_req.req_type = WLAN_SCAN_CANCEL_PDEV_ALL;
	else
		req->cancel_req.req_type = WLAN_SCAN_CANCEL_VDEV_ALL;

	status = ucfg_scan_cancel(req);
	if (QDF_IS_STATUS_ERROR(status))
		sme_err("Cancel scan request failed");

	wlan_objmgr_vdev_release_ref(vdev, WLAN_LEGACY_SME_ID);

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
			sessionId);
		csr_release_command(pMac, pCommand);
	}

	csr_ll_close(&localList);
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
	struct csr_roam_session *pSession = CSR_GET_SESSION(pMac, sessionId);
	tSirBssDescription *pNewBssDescriptor = NULL;
	uint32_t size = 0;

	if (NULL == pSession) {
		return QDF_STATUS_E_FAILURE;
	}
	sme_debug("Current bssid::"MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(pSession->pConnectBssDesc->bssId));
	sme_debug("My bssid::"MAC_ADDRESS_STR" channel %d",
		MAC_ADDR_ARRAY(bssid.bytes), channel);

	size = pSession->pConnectBssDesc->length +
		sizeof(pSession->pConnectBssDesc->length);
	if (!size) {
		sme_err("length of bss descriptor is 0");
		return QDF_STATUS_E_FAILURE;
	}
	pNewBssDescriptor = qdf_mem_malloc(size);
	if (NULL == pNewBssDescriptor) {
		sme_err("memory allocation failed");
		return QDF_STATUS_E_FAILURE;
	}
	qdf_mem_copy(pNewBssDescriptor, pSession->pConnectBssDesc, size);
	/* change the BSSID & channel as passed */
	qdf_mem_copy(pNewBssDescriptor->bssId, bssid.bytes,
			sizeof(tSirMacAddr));
	pNewBssDescriptor->channelId = channel;
	if (!csr_scan_append_bss_description(pMac, pNewBssDescriptor)) {
		sme_err("csr_scan_append_bss_description failed");
		status = QDF_STATUS_E_FAILURE;
		goto free_mem;
	}
	sme_err("entry successfully added in scan cache");

free_mem:
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
	struct tag_csrscan_result *scan_res_ptr = NULL;

	length = roam_sync_ind_ptr->beaconProbeRespLength -
		(SIR_MAC_HDR_LEN_3A + SIR_MAC_B_PR_SSID_OFFSET);
	scan_res_ptr = qdf_mem_malloc(sizeof(struct tag_csrscan_result) +
				length);
	if (scan_res_ptr == NULL) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				" fail to allocate memory for frame");
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(&scan_res_ptr->Result.BssDescriptor,
			bss_desc_ptr,
			(sizeof(tSirBssDescription) + length));

	sme_debug("LFR3:Add BSSID to scan cache" MAC_ADDRESS_STR,
		MAC_ADDR_ARRAY(scan_res_ptr->Result.BssDescriptor.bssId));
	csr_scan_add_result(pMac, scan_res_ptr);
	csr_free_scan_result_entry(pMac, scan_res_ptr);
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
	struct tag_csrscan_result *scan_result = NULL;
	struct scan_result_list *bss_list =
				(struct scan_result_list *)result_handle;

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

	scan_result = GET_BASE_ADDR(first_element, struct tag_csrscan_result,
					Link);

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
	struct tag_csrscan_result *scan_result = NULL;
	struct scan_result_list *bss_list =
				(struct scan_result_list *)result_handle;

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
				struct tag_csrscan_result,
				Link);
		qdf_mem_copy(bss_descr,
				&scan_result->Result.BssDescriptor,
				sizeof(tSirBssDescription));
	}
	return bss_descr;
}

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
	case eCSR_AUTH_TYPE_FILS_SHA256:
		return WLAN_AUTH_TYPE_FILS_SHA256;
	case eCSR_AUTH_TYPE_FILS_SHA384:
		return WLAN_AUTH_TYPE_FILS_SHA384;
	case eCSR_AUTH_TYPE_FT_FILS_SHA256:
		return WLAN_AUTH_TYPE_FT_FILS_SHA256;
	case eCSR_AUTH_TYPE_FT_FILS_SHA384:
		return WLAN_AUTH_TYPE_FT_FILS_SHA384;
	case eCSR_AUTH_TYPE_DPP_RSN:
		return WLAN_AUTH_TYPE_DPP_RSN;
	case eCSR_AUTH_TYPE_OWE:
		return WLAN_AUTH_TYPE_OWE;
	case eCSR_AUTH_TYPE_SUITEB_EAP_SHA256:
		return WLAN_AUTH_TYPE_SUITEB_EAP_SHA256;
	case eCSR_AUTH_TYPE_SUITEB_EAP_SHA384:
		return WLAN_AUTH_TYPE_SUITEB_EAP_SHA384;
	case eCSR_AUTH_TYPE_SAE:
		return WLAN_AUTH_TYPE_SAE;
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
	case WLAN_AUTH_TYPE_FILS_SHA256:
		return eCSR_AUTH_TYPE_FILS_SHA256;
	case WLAN_AUTH_TYPE_FILS_SHA384:
		return eCSR_AUTH_TYPE_FILS_SHA384;
	case WLAN_AUTH_TYPE_FT_FILS_SHA256:
		return eCSR_AUTH_TYPE_FT_FILS_SHA256;
	case WLAN_AUTH_TYPE_FT_FILS_SHA384:
		return eCSR_AUTH_TYPE_FT_FILS_SHA384;
	case WLAN_AUTH_TYPE_DPP_RSN:
		return eCSR_AUTH_TYPE_DPP_RSN;
	case WLAN_AUTH_TYPE_OWE:
		return eCSR_AUTH_TYPE_OWE;
	case WLAN_AUTH_TYPE_SUITEB_EAP_SHA256:
		return eCSR_AUTH_TYPE_SUITEB_EAP_SHA256;
	case WLAN_AUTH_TYPE_SUITEB_EAP_SHA384:
		return eCSR_AUTH_TYPE_SUITEB_EAP_SHA384;
	case WLAN_AUTH_TYPE_SAE:
		return eCSR_AUTH_TYPE_SAE;
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
	case eCSR_ENCRYPT_TYPE_AES_GCMP:
		return WLAN_ENCRYPT_TYPE_AES_GCMP;
	case eCSR_ENCRYPT_TYPE_AES_GCMP_256:
		return WLAN_ENCRYPT_TYPE_AES_GCMP_256;
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
	case WLAN_ENCRYPT_TYPE_AES_GCMP:
		return eCSR_ENCRYPT_TYPE_AES_GCMP;
	case WLAN_ENCRYPT_TYPE_AES_GCMP_256:
		return eCSR_ENCRYPT_TYPE_AES_GCMP_256;
	case WLAN_ENCRYPT_TYPE_ANY:
	default:
		return eCSR_ENCRYPT_TYPE_NONE;
	}
}

#ifdef WLAN_FEATURE_11W
/**
 * csr_update_pmf_cap: Updates PMF cap
 * @src_filter: Source filter
 * @dst_filter: Destination filter
 *
 * Return: None
 */
static void csr_update_pmf_cap(tCsrScanResultFilter *src_filter,
		struct scan_filter *dst_filter) {

	if (src_filter->MFPCapable || src_filter->MFPEnabled)
		dst_filter->pmf_cap = WLAN_PMF_CAPABLE;
	if (src_filter->MFPRequired)
		dst_filter->pmf_cap = WLAN_PMF_REQUIRED;
}
#else
static inline void csr_update_pmf_cap(tCsrScanResultFilter *src_filter,
		struct scan_filter *dst_filter)
{}
#endif

static QDF_STATUS csr_prepare_scan_filter(tpAniSirGlobal mac_ctx,
	tCsrScanResultFilter *pFilter, struct scan_filter *filter)
{
	int i;
	uint32_t len = 0;
	QDF_STATUS status;
	enum policy_mgr_con_mode new_mode;

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

	if (pFilter->realm_check) {
		filter->fils_scan_filter.realm_check = true;
		qdf_mem_copy(filter->fils_scan_filter.fils_realm,
			pFilter->fils_realm, REAM_HASH_LEN);
	}

	if (pFilter->force_rsne_override) {
		int idx;

		sme_debug("force_rsne_override enabled fill all auth type and enctype");
		filter->num_of_auth = WLAN_NUM_OF_SUPPORT_AUTH_TYPE;
		for (i = 0; i < filter->num_of_auth; i++)
			filter->auth_type[i] = i;

		idx = 0;
		for (i = 0; i < WLAN_NUM_OF_ENCRYPT_TYPE; i++) {
			if (i == WLAN_ENCRYPT_TYPE_TKIP ||
			    i == WLAN_ENCRYPT_TYPE_AES ||
			    i == WLAN_ENCRYPT_TYPE_AES_GCMP ||
			    i == WLAN_ENCRYPT_TYPE_AES_GCMP_256) {
				filter->enc_type[idx] = i;
				filter->mc_enc_type[idx] = i;
				idx++;
			}
		}
		filter->num_of_enc_type = idx;
		filter->num_of_mc_enc_type = idx;
		filter->ignore_pmf_cap = true;
	} else {
		filter->num_of_auth =
			pFilter->authType.numEntries;
		if (filter->num_of_auth > WLAN_NUM_OF_SUPPORT_AUTH_TYPE)
			filter->num_of_auth = WLAN_NUM_OF_SUPPORT_AUTH_TYPE;
		for (i = 0; i < filter->num_of_auth; i++)
			filter->auth_type[i] =
			  csr_covert_auth_type_new(
			  pFilter->authType.authType[i]);
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
	}
	qdf_mem_copy(filter->country,
		pFilter->countryCode, WNI_CFG_COUNTRY_CODE_LEN);

	if (pFilter->bWPSAssociation || pFilter->bOSENAssociation)
		filter->ignore_auth_enc_type = true;

	filter->rrm_measurement_filter = pFilter->fMeasurement;

	filter->mobility_domain = pFilter->MDID.mobilityDomain;

	filter->p2p_results = pFilter->p2pResult;

	csr_update_pmf_cap(pFilter, filter);

	if (pFilter->BSSType == eCSR_BSS_TYPE_INFRASTRUCTURE)
		filter->bss_type = WLAN_TYPE_BSS;
	else if (pFilter->BSSType == eCSR_BSS_TYPE_IBSS ||
		pFilter->BSSType == eCSR_BSS_TYPE_START_IBSS)
		filter->bss_type = WLAN_TYPE_IBSS;
	else
		filter->bss_type = WLAN_TYPE_ANY;

	filter->dot11_mode = pFilter->phyMode;

	// enable bss scoring for only STA mode
	if (pFilter->csrPersona == QDF_STA_MODE)
		filter->bss_scoring_required = true;
	else
		filter->bss_scoring_required = false;
	if (!pFilter->BSSIDs.numOfBSSIDs) {
		if (policy_mgr_map_concurrency_mode(
		   &pFilter->csrPersona, &new_mode)) {
			status = policy_mgr_get_pcl(mac_ctx->psoc, new_mode,
				filter->pcl_channel_list, &len,
				filter->pcl_weight_list, QDF_MAX_NUM_CHAN);
			filter->num_of_pcl_channels = (uint8_t)len;
		}
	}
	qdf_mem_copy(filter->bssid_hint.bytes,
			pFilter->bssid_hint.bytes,
			QDF_MAC_ADDR_SIZE);

	return QDF_STATUS_SUCCESS;
}

#ifdef WLAN_FEATURE_FILS_SK
/**
 * csr_update_bss_with_fils_data: Fill FILS params in bss desc from scan entry
 * @mac_ctx: mac context
 * @scan_entry: scan entry
 * @bss_descr: bss description
 */
static void csr_update_bss_with_fils_data(tpAniSirGlobal mac_ctx,
					  struct scan_cache_entry *scan_entry,
					  tSirBssDescription *bss_descr)
{
	tDot11fIEfils_indication fils_indication = {0};
	struct sir_fils_indication fils_ind;

	if (!scan_entry->ie_list.fils_indication)
		return;

	dot11f_unpack_ie_fils_indication(mac_ctx,
				scan_entry->ie_list.fils_indication +
				SIR_FILS_IND_ELEM_OFFSET,
				*(scan_entry->ie_list.fils_indication + 1),
				&fils_indication, false);

	update_fils_data(&fils_ind, &fils_indication);
	if (fils_ind.realm_identifier.realm_cnt > SIR_MAX_REALM_COUNT)
		fils_ind.realm_identifier.realm_cnt = SIR_MAX_REALM_COUNT;

	bss_descr->fils_info_element.realm_cnt =
		fils_ind.realm_identifier.realm_cnt;
	qdf_mem_copy(bss_descr->fils_info_element.realm,
			fils_ind.realm_identifier.realm,
			bss_descr->fils_info_element.realm_cnt * SIR_REALM_LEN);
	if (fils_ind.cache_identifier.is_present) {
		bss_descr->fils_info_element.is_cache_id_present = true;
		qdf_mem_copy(bss_descr->fils_info_element.cache_id,
			fils_ind.cache_identifier.identifier, CACHE_ID_LEN);
	}
	if (fils_ind.is_fils_sk_auth_supported)
		bss_descr->fils_info_element.is_fils_sk_supported = true;
}
#else
static void csr_update_bss_with_fils_data(tpAniSirGlobal mac_ctx,
					  struct scan_cache_entry *scan_entry,
					  tSirBssDescription *bss_descr)
{ }
#endif

static QDF_STATUS csr_fill_bss_from_scan_entry(tpAniSirGlobal mac_ctx,
	struct scan_cache_entry *scan_entry,
	struct tag_csrscan_result **p_result)
{
	tDot11fBeaconIEs *bcn_ies;
	tSirBssDescription *bss_desc;
	tCsrScanResultInfo *result_info;
	tpSirMacMgmtHdr hdr;
	uint8_t *ie_ptr;
	struct tag_csrscan_result *bss;
	uint32_t bss_len, alloc_len, ie_len;
	QDF_STATUS status;

	ie_len = util_scan_entry_ie_len(scan_entry);
	ie_ptr = util_scan_entry_ie_data(scan_entry);

	hdr = (tpSirMacMgmtHdr)scan_entry->raw_frame.ptr;

	bss_len = (uint16_t)(offsetof(tSirBssDescription,
			   ieFields[0]) + ie_len);
	alloc_len = sizeof(struct tag_csrscan_result) + bss_len;
	bss = qdf_mem_malloc(alloc_len);

	if (!bss) {
		sme_err("could not allocate bss");
		return QDF_STATUS_E_NOMEM;
	}

	bss->AgingCount =
		(int32_t) mac_ctx->roam.configParam.agingCount;
	bss->ucEncryptionType =
		csr_covert_enc_type_old(scan_entry->neg_sec_info.uc_enc);
	bss->mcEncryptionType =
		csr_covert_enc_type_old(scan_entry->neg_sec_info.mc_enc);
	bss->authType =
		csr_covert_auth_type_old(scan_entry->neg_sec_info.auth_type);
	bss->bss_score = scan_entry->bss_score;

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
	csr_update_bss_with_fils_data(mac_ctx, scan_entry, bss_desc);
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
	struct scan_result_list *ret_list,
	qdf_list_t *scan_list)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct tag_csrscan_result *pResult = NULL;
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

/**
 * csr_remove_ap_due_to_rssi() - check if bss is present in
 * list of BSSID which rejected Assoc due to RSSI
 * @list: rssi based rejected BSS list
 * @bss_descr: pointer to bss description
 *
 * Check if the time interval indicated in last Assoc reject
 * has expired OR rssi has improved by margin indicated
 * in last Assoc reject. If any of the condition match remove
 * the AP from the avoid list, else do not try to conenct
 * to the AP
 *
 * Return: true if connection cannot be tried with AP else false
 */
static bool csr_remove_ap_due_to_rssi(qdf_list_t *list,
	tSirBssDescription *bss_descr)
{
	QDF_STATUS status;
	struct sir_rssi_disallow_lst *cur_node = NULL;
	qdf_list_node_t *cur_lst = NULL, *next_lst = NULL;
	qdf_time_t cur_time;
	uint32_t time_diff;

	if (!qdf_list_size(list))
		return false;

	cur_time = qdf_do_div(qdf_get_monotonic_boottime(),
		QDF_MC_TIMER_TO_MS_UNIT);

	qdf_list_peek_front(list, &cur_lst);
	while (cur_lst) {
		cur_node = qdf_container_of(cur_lst,
				struct sir_rssi_disallow_lst, node);

		qdf_list_peek_next(list, cur_lst, &next_lst);

		time_diff = cur_time - cur_node->time_during_rejection;
		if ((time_diff > cur_node->retry_delay)) {
			sme_debug("Remove %pM as time diff %d is greater retry delay %d",
				cur_node->bssid.bytes, time_diff,
				cur_node->retry_delay);
			status = qdf_list_remove_node(list, cur_lst);
			if (QDF_IS_STATUS_SUCCESS(status))
				qdf_mem_free(cur_node);
			cur_lst = next_lst;
			next_lst = NULL;
			cur_node = NULL;
			continue;
		}

		if (!qdf_mem_cmp(cur_node->bssid.bytes,
		    bss_descr->bssId, QDF_MAC_ADDR_SIZE))
			break;
		cur_lst = next_lst;
		next_lst = NULL;
		cur_node = NULL;
	}

	if (cur_node) {
		time_diff = cur_time - cur_node->time_during_rejection;
		if (!(time_diff > cur_node->retry_delay ||
		   bss_descr->rssi_raw >= cur_node->expected_rssi)) {
			sme_err("Don't Attempt to connect %pM (time diff %d retry delay %d rssi %d expected rssi %d)",
				cur_node->bssid.bytes, time_diff,
				cur_node->retry_delay, bss_descr->rssi_raw,
				cur_node->expected_rssi);
			return true;
		}
		sme_debug("Remove %pM as time diff %d is greater retry delay %d or RSSI %d is greater than expected %d",
				cur_node->bssid.bytes, time_diff,
				cur_node->retry_delay,
				bss_descr->rssi_raw,
				cur_node->expected_rssi);
		status = qdf_list_remove_node(list, cur_lst);
		if (QDF_IS_STATUS_SUCCESS(status))
			qdf_mem_free(cur_node);
	}

	return false;
}

/**
 * csr_filter_ap_due_to_rssi_reject() - filter the AP who has sent
 * assoc reject due to RSSI if condition has not improved
 * @mac_ctx: mac context
 * @scan_list: candidate list for the connection
 *
 * Return: void
 */
static void csr_filter_ap_due_to_rssi_reject(tpAniSirGlobal mac_ctx,
	struct scan_result_list *scan_list)
{
	tListElem *cur_entry;
	tListElem *next_entry;
	struct tag_csrscan_result *scan_res;
	bool remove;

	if (!scan_list ||
	   !qdf_list_size(&mac_ctx->roam.rssi_disallow_bssid))
		return;

	csr_ll_lock(&scan_list->List);

	cur_entry = csr_ll_peek_head(&scan_list->List, LL_ACCESS_NOLOCK);
	while (cur_entry) {
		scan_res = GET_BASE_ADDR(cur_entry, struct tag_csrscan_result,
					Link);
		next_entry = csr_ll_next(&scan_list->List,
						cur_entry, LL_ACCESS_NOLOCK);
		remove = csr_remove_ap_due_to_rssi(
			&mac_ctx->roam.rssi_disallow_bssid,
			&scan_res->Result.BssDescriptor);
		if (remove) {
			csr_ll_remove_entry(&scan_list->List,
				cur_entry, LL_ACCESS_NOLOCK);
			csr_free_scan_result_entry(mac_ctx, scan_res);
		}
		cur_entry = next_entry;
		next_entry = NULL;
	}
	csr_ll_unlock(&scan_list->List);

}

QDF_STATUS csr_scan_get_result(tpAniSirGlobal mac_ctx,
			       tCsrScanResultFilter *pFilter,
			       tScanResultHandle *results)
{
	QDF_STATUS status;
	struct scan_result_list *ret_list = NULL;
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
	if (list)
		sme_debug("num_entries %d", qdf_list_size(list));

	if (!list || (list && !qdf_list_size(list))) {
		sme_err("get scan result failed");
		status = QDF_STATUS_E_NULL_VALUE;
		goto error;
	}

	ret_list = qdf_mem_malloc(sizeof(struct scan_result_list));
	if (!ret_list) {
		sme_err("pRetList is NULL");
		status = QDF_STATUS_E_NOMEM;
		goto error;
	}

	csr_ll_open(mac_ctx->hHdd, &ret_list->List);
	ret_list->pCurEntry = NULL;
	status = csr_parse_scan_list(mac_ctx,
		ret_list, list);
	sme_debug("status: %d No of BSS: %d",
		  status, csr_ll_count(&ret_list->List));
	if (QDF_IS_STATUS_ERROR(status) || !results)
		/* Fail or No one wants the result. */
		csr_scan_result_purge(mac_ctx, (tScanResultHandle) ret_list);
	else {
		if (pFilter)
			csr_filter_ap_due_to_rssi_reject(mac_ctx, ret_list);
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

QDF_STATUS csr_scan_get_result_for_bssid(tpAniSirGlobal mac_ctx,
					struct qdf_mac_addr *bssid,
					tCsrScanResultInfo *res)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	tCsrScanResultFilter *scan_filter = NULL;
	tScanResultHandle filtered_scan_result = NULL;
	tCsrScanResultInfo *scan_result;

	if (!mac_ctx) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
				FL("mac_ctx is NULL"));
		return QDF_STATUS_E_FAILURE;
	}

	scan_filter = qdf_mem_malloc(sizeof(tCsrScanResultFilter));
	if (!scan_filter) {
		sme_err("Failed to allocated memory for scan_filter");
		return QDF_STATUS_E_NOMEM;
	}

	scan_filter->BSSIDs.bssid = qdf_mem_malloc(sizeof(*bssid));
	if (!scan_filter->BSSIDs.bssid) {
		sme_err("Failed to allocate memory for BSSIDs");
		status = QDF_STATUS_E_FAILURE;
		goto free_filter;
	}

	scan_filter->BSSIDs.numOfBSSIDs = 1;
	qdf_mem_copy(scan_filter->BSSIDs.bssid, bssid, sizeof(*bssid));

	status = csr_scan_get_result(mac_ctx, scan_filter,
				&filtered_scan_result);

	if (!QDF_IS_STATUS_SUCCESS(status)) {
		sme_err("Failed to get scan result");
		goto free_filter;
	}

	scan_result = csr_scan_result_get_first(mac_ctx, filtered_scan_result);

	if (scan_result) {
		res->pvIes = NULL;
		res->ssId.length = scan_result->ssId.length;
		qdf_mem_copy(&res->ssId.ssId, &scan_result->ssId.ssId,
			res->ssId.length);
		res->timer = scan_result->timer;
		qdf_mem_copy(&res->BssDescriptor, &scan_result->BssDescriptor,
			sizeof(tSirBssDescription));
		status = QDF_STATUS_SUCCESS;
	} else {
		status = QDF_STATUS_E_FAILURE;
	}

	csr_scan_result_purge(mac_ctx, filtered_scan_result);

free_filter:
	csr_free_scan_filter(mac_ctx, scan_filter);
	if (scan_filter)
		qdf_mem_free(scan_filter);

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

void csr_init_occupied_channels_list(tpAniSirGlobal mac_ctx,
	uint8_t sessionId)
{
	tScanResultHandle results;
	struct scan_result_list *scan_list = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tListElem *scan_entry = NULL;
	struct tag_csrscan_result *bss_desc = NULL;
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
		bss_desc = GET_BASE_ADDR(scan_entry, struct tag_csrscan_result,
					 Link);
		ie_ptr = (tDot11fBeaconIEs *) (bss_desc->Result.pvIes);
		if (!ie_ptr && !QDF_IS_STATUS_SUCCESS(
			csr_get_parsed_bss_description_ies(mac_ctx,
				&bss_desc->Result.BssDescriptor, &ie_ptr))) {
			/* Pick next bss entry before continuing */
			scan_entry = csr_ll_next(&scan_list->List, scan_entry,
				     LL_ACCESS_NOLOCK);
			continue;
		}
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
