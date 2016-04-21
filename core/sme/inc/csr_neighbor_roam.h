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

/**
 * \file csr_neighbor_roam.h
 *
 * Exports and types for the neighbor roaming algorithm which is sepcifically
 * designed for Android.
 */

#ifndef CSR_NEIGHBOR_ROAM_H
#define CSR_NEIGHBOR_ROAM_H

#include "sme_api.h"

#define ROAM_AP_AGE_LIMIT_MS                     10000

/* Enumeration of various states in neighbor roam algorithm */
typedef enum {
	eCSR_NEIGHBOR_ROAM_STATE_CLOSED,
	eCSR_NEIGHBOR_ROAM_STATE_INIT,
	eCSR_NEIGHBOR_ROAM_STATE_CONNECTED,
	eCSR_NEIGHBOR_ROAM_STATE_REASSOCIATING,
	eCSR_NEIGHBOR_ROAM_STATE_PREAUTHENTICATING,
	eCSR_NEIGHBOR_ROAM_STATE_PREAUTH_DONE,
	eNEIGHBOR_STATE_MAX
} eCsrNeighborRoamState;

/* Parameters that are obtained from CFG */
typedef struct sCsrNeighborRoamCfgParams {
	uint8_t maxNeighborRetries;
	uint32_t neighborScanPeriod;
	tCsrChannelInfo channelInfo;
	uint8_t neighborLookupThreshold;
	uint8_t neighborReassocThreshold;
	uint32_t minChannelScanTime;
	uint32_t maxChannelScanTime;
	uint16_t neighborResultsRefreshPeriod;
	uint16_t emptyScanRefreshPeriod;
	uint8_t nOpportunisticThresholdDiff;
	uint8_t nRoamRescanRssiDiff;
	uint8_t nRoamBmissFirstBcnt;
	uint8_t nRoamBmissFinalBcnt;
	uint8_t nRoamBeaconRssiWeight;
	uint8_t delay_before_vdev_stop;
	uint32_t hi_rssi_scan_max_count;
	uint32_t hi_rssi_scan_rssi_delta;
	uint32_t hi_rssi_scan_delay;
	int32_t hi_rssi_scan_rssi_ub;
} tCsrNeighborRoamCfgParams, *tpCsrNeighborRoamCfgParams;

#define CSR_NEIGHBOR_ROAM_INVALID_CHANNEL_INDEX    255
typedef struct sCsrNeighborRoamChannelInfo {
	/* Flag to mark reception of IAPP Neighbor list */
	bool IAPPNeighborListReceived;
	/* Current channel index that is being scanned */
	uint8_t currentChanIndex;
	/* Max number of channels in channel list and the list of channels */
	tCsrChannelInfo currentChannelListInfo;
} tCsrNeighborRoamChannelInfo, *tpCsrNeighborRoamChannelInfo;

typedef struct sCsrNeighborRoamBSSInfo {
	tListElem List;
	uint8_t apPreferenceVal;
	tpSirBssDescription pBssDescription;
} tCsrNeighborRoamBSSInfo, *tpCsrNeighborRoamBSSInfo;

#define CSR_NEIGHBOR_ROAM_REPORT_QUERY_TIMEOUT  1000       /* in milliseconds */
#define CSR_NEIGHBOR_ROAM_PREAUTH_RSP_WAIT_MULTIPLIER   10 /* in milliseconds */
/* Max number of MAC addresses with which the pre-auth was failed */
#define MAX_NUM_PREAUTH_FAIL_LIST_ADDRESS          10
#define MAX_BSS_IN_NEIGHBOR_RPT                    15
#define CSR_NEIGHBOR_ROAM_MAX_NUM_PREAUTH_RETRIES  3

/* Black listed APs. List of MAC Addresses with which the Preauth was failed */
typedef struct sCsrPreauthFailListInfo {
	uint8_t numMACAddress;
	tSirMacAddr macAddress[MAX_NUM_PREAUTH_FAIL_LIST_ADDRESS];
} tCsrPreauthFailListInfo, *tpCsrPreauthFailListInfo;

typedef struct sCsrNeighborReportBssInfo {
	uint8_t channelNum;
	uint8_t neighborScore;
	tSirMacAddr neighborBssId;
} tCsrNeighborReportBssInfo, *tpCsrNeighborReportBssInfo;

typedef struct sCsr11rAssocNeighborInfo {
	bool preauthRspPending;
	bool neighborRptPending;
	uint8_t currentNeighborRptRetryNum;
	tCsrPreauthFailListInfo preAuthFailList;
	uint32_t neighborReportTimeout;
	uint32_t PEPreauthRespTimeout;
	uint8_t numPreAuthRetries;
	tDblLinkList preAuthDoneList;   /* llist which consists/preauth nodes */
	uint8_t numBssFromNeighborReport;
	/* Contains info needed during REPORT_SCAN State */
	tCsrNeighborReportBssInfo neighboReportBssInfo[MAX_BSS_IN_NEIGHBOR_RPT];
} tCsr11rAssocNeighborInfo, *tpCsr11rAssocNeighborInfo;

typedef enum {
	eFirstEmptyScan = 1,
	eSecondEmptyScan,
	eThirdEmptyScan,
	eFourthEmptyScan,
	eFifthEmptyScan,
	eMaxEmptyScan = eFifthEmptyScan,
} eNeighborRoamEmptyScanCount;

typedef enum {
	DEFAULT_SCAN = 0,
	SPLIT_SCAN_OCCUPIED_LIST = 1,
} eNeighborRoamScanMode;

/* Complete control information for neighbor roam algorithm */
typedef struct sCsrNeighborRoamControlInfo {
	eCsrNeighborRoamState neighborRoamState;
	eCsrNeighborRoamState prevNeighborRoamState;
	tCsrNeighborRoamCfgParams cfgParams;
	struct qdf_mac_addr currAPbssid;  /* current assoc AP */
	uint8_t currAPoperationChannel; /* current assoc AP */
	tCsrNeighborRoamChannelInfo roamChannelInfo;
	uint8_t currentNeighborLookupThreshold;
	uint8_t currentOpportunisticThresholdDiff;
	uint8_t currentRoamRescanRssiDiff;
	tDblLinkList roamableAPList;    /* List of current FT candidates */
	tCsrRoamProfile csrNeighborRoamProfile;
	bool is11rAssoc;
	tCsr11rAssocNeighborInfo FTRoamInfo;
#ifdef FEATURE_WLAN_ESE
	bool isESEAssoc;
	bool isVOAdmitted;
	uint16_t MinQBssLoadRequired;
#endif
	/*
	 * Previous connected profile.
	 * If the new profile does not match previous we re-initialize
	 * occupied channel list
	 */
	tCsrRoamConnectedProfile prevConnProfile;
	/* upper layer requested a reassoc */
	uint8_t uOsRequestedHandoff;
	/* handoff related info came with upper layer's req for reassoc */
	tCsrHandoffRequest handoffReqInfo;
	uint8_t currentRoamBmissFirstBcnt;
	uint8_t currentRoamBmissFinalBcnt;
	uint8_t currentRoamBeaconRssiWeight;
	uint8_t last_sent_cmd;
} tCsrNeighborRoamControlInfo, *tpCsrNeighborRoamControlInfo;

/* All the necessary Function declarations are here */
QDF_STATUS csr_neighbor_roam_indicate_connect(tpAniSirGlobal pMac,
		uint8_t sessionId, QDF_STATUS status);
QDF_STATUS csr_neighbor_roam_indicate_disconnect(tpAniSirGlobal pMac,
		uint8_t sessionId);
QDF_STATUS csr_neighbor_roam_init(tpAniSirGlobal pMac, uint8_t sessionId);
void csr_neighbor_roam_close(tpAniSirGlobal pMac, uint8_t sessionId);
QDF_STATUS csr_neighbor_roam_transit_to_cfg_chan_scan(tpAniSirGlobal pMac,
		uint8_t sessionId);
QDF_STATUS csrNeighborRoamTransitionToPreauthDone(tpAniSirGlobal pMac);
QDF_STATUS csr_neighbor_roam_prepare_scan_profile_filter(tpAniSirGlobal pMac,
		tCsrScanResultFilter *pScanFilter, uint8_t sessionId);
QDF_STATUS csr_neighbor_roam_preauth_rsp_handler(tpAniSirGlobal pMac,
		uint8_t sessionId, tSirRetStatus limStatus);
bool csr_neighbor_roam_is11r_assoc(tpAniSirGlobal pMac, uint8_t sessionId);
#ifdef WLAN_FEATURE_HOST_ROAM
void csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
		tpAniSirGlobal pMac, uint8_t sessionId);
bool csr_neighbor_roam_state_preauth_done(tpAniSirGlobal pMac,
		uint8_t sessionId);
QDF_STATUS csr_roam_issue_reassociate_cmd(tpAniSirGlobal pMac,
		uint32_t sessionId);
void csr_neighbor_roam_free_roamable_bss_list(tpAniSirGlobal mac_ctx,
		tDblLinkList *llist);
bool csr_neighbor_roam_get_handoff_ap_info(tpAniSirGlobal pMac,
		tpCsrNeighborRoamBSSInfo pHandoffNode, uint8_t sessionId);
QDF_STATUS csr_roam_issue_reassociate(tpAniSirGlobal pMac,
		uint32_t sessionId, tSirBssDescription *pSirBssDesc,
		tDot11fBeaconIEs *pIes, tCsrRoamProfile *pProfile);
void csr_neighbor_roam_request_handoff(tpAniSirGlobal pMac, uint8_t sessionId);
QDF_STATUS csr_neighbor_roam_candidate_found_ind_hdlr(tpAniSirGlobal pMac,
		void *pMsg);
QDF_STATUS csr_neighbor_roam_process_scan_complete(tpAniSirGlobal pMac,
		uint8_t sessionId);
bool csr_neighbor_roam_is_handoff_in_progress(tpAniSirGlobal pMac,
		uint8_t sessionId);
void csr_neighbor_roam_reset_preauth_control_info(
		tpAniSirGlobal mac_ctx, uint8_t session_id);
void csr_neighbor_roam_purge_preauth_failed_list(tpAniSirGlobal pMac);
#else
static inline bool csr_neighbor_roam_state_preauth_done(tpAniSirGlobal pMac,
		uint8_t sessionId)
{
	return false;
}
static inline QDF_STATUS csr_roam_issue_reassociate_cmd(tpAniSirGlobal pMac,
		uint32_t sessionId)
{
	return QDF_STATUS_E_NOSUPPORT;
}
static inline QDF_STATUS csr_roam_issue_reassociate(tpAniSirGlobal pMac,
		uint32_t sessionId, tSirBssDescription *pSirBssDesc,
		tDot11fBeaconIEs *pIes, tCsrRoamProfile *pProfile)
{
	return QDF_STATUS_E_NOSUPPORT;
}
static inline QDF_STATUS csr_neighbor_roam_candidate_found_ind_hdlr(
		tpAniSirGlobal pMac, void *pMsg)
{
	return QDF_STATUS_E_NOSUPPORT;
}
static inline QDF_STATUS csr_neighbor_roam_process_scan_complete(
		tpAniSirGlobal pMac, uint8_t sessionId)
{
	return QDF_STATUS_E_NOSUPPORT;
}
static inline void csr_neighbor_roam_tranistion_preauth_done_to_disconnected(
		tpAniSirGlobal pMac, uint8_t sessionId)
{}
static inline void csr_neighbor_roam_free_roamable_bss_list(
		tpAniSirGlobal mac_ctx, tDblLinkList *llist)
{}
static inline void csr_neighbor_roam_request_handoff(tpAniSirGlobal pMac,
		uint8_t sessionId)
{}
static inline void csr_neighbor_roam_reset_preauth_control_info(
		tpAniSirGlobal mac_ctx, uint8_t session_id)
{}
static inline void csr_neighbor_roam_purge_preauth_failed_list(
		tpAniSirGlobal pMac)
{}
static inline bool csr_neighbor_roam_get_handoff_ap_info(tpAniSirGlobal pMac,
		tpCsrNeighborRoamBSSInfo pHandoffNode, uint8_t sessionId)
{
	return false;
}
static inline bool csr_neighbor_roam_is_handoff_in_progress(tpAniSirGlobal pMac,
		uint8_t sessionId)
{
	return false;
}
#endif
bool csr_neighbor_middle_of_roaming(tpAniSirGlobal pMac, uint8_t sessionId);
QDF_STATUS csr_neighbor_roam_set_lookup_rssi_threshold(tpAniSirGlobal pMac,
		uint8_t sessionId, uint8_t neighborLookupRssiThreshold);
QDF_STATUS csr_neighbor_roam_set_opportunistic_scan_threshold_diff(
		tpAniSirGlobal pMac, uint8_t sessionId,
		uint8_t nOpportunisticThresholdDiff);
QDF_STATUS csr_neighbor_roam_set_roam_rescan_rssi_diff(tpAniSirGlobal pMac,
		uint8_t sessionId, uint8_t nRoamRescanRssiDiff);
QDF_STATUS csr_neighbor_roam_set_roam_bmiss_first_bcnt(tpAniSirGlobal pMac,
		uint8_t sessionId, uint8_t nRoamBmissFirstBcnt);
QDF_STATUS csr_neighbor_roam_set_roam_bmiss_final_bcnt(tpAniSirGlobal pMac,
		uint8_t sessionId, uint8_t nRoamBmissFinalBcnt);
QDF_STATUS csr_neighbor_roam_set_roam_beacon_rssi_weight(tpAniSirGlobal pMac,
		uint8_t sessionId, uint8_t nRoamBeaconRssiWeight);
QDF_STATUS csr_neighbor_roam_update_fast_roaming_enabled(tpAniSirGlobal pMac,
		uint8_t sessionId, const bool fastRoamEnabled);
QDF_STATUS csr_neighbor_roam_update_ese_mode_enabled(tpAniSirGlobal pMac,
		uint8_t sessionId, const bool eseMode);
QDF_STATUS csr_neighbor_roam_channels_filter_by_current_band(
		tpAniSirGlobal pMac, uint8_t sessionId,
		uint8_t *pInputChannelList,
		uint8_t inputNumOfChannels,
		uint8_t *pOutputChannelList,
		uint8_t *pMergedOutputNumOfChannels);
QDF_STATUS csr_neighbor_roam_merge_channel_lists(tpAniSirGlobal pMac,
		uint8_t *pInputChannelList,
		uint8_t inputNumOfChannels,
		uint8_t *pOutputChannelList,
		uint8_t outputNumOfChannels,
		uint8_t *pMergedOutputNumOfChannels);
void csr_roam_reset_roam_params(tpAniSirGlobal mac_ptr);
#define ROAM_SCAN_OFFLOAD_START                     1
#define ROAM_SCAN_OFFLOAD_STOP                      2
#define ROAM_SCAN_OFFLOAD_RESTART                   3
#define ROAM_SCAN_OFFLOAD_UPDATE_CFG                4
#define ROAM_SCAN_OFFLOAD_ABORT_SCAN                5

#define REASON_CONNECT                              1
#define REASON_CHANNEL_LIST_CHANGED                 2
#define REASON_LOOKUP_THRESH_CHANGED                3
#define REASON_DISCONNECTED                         4
#define REASON_RSSI_DIFF_CHANGED                    5
#define REASON_ESE_INI_CFG_CHANGED                  6
#define REASON_NEIGHBOR_SCAN_REFRESH_PERIOD_CHANGED 7
#define REASON_VALID_CHANNEL_LIST_CHANGED           8
#define REASON_FLUSH_CHANNEL_LIST                   9
#define REASON_EMPTY_SCAN_REF_PERIOD_CHANGED        10
#define REASON_PREAUTH_FAILED_FOR_ALL               11
#define REASON_NO_CAND_FOUND_OR_NOT_ROAMING_NOW     12
#define REASON_NPROBES_CHANGED                      13
#define REASON_HOME_AWAY_TIME_CHANGED               14
#define REASON_OS_REQUESTED_ROAMING_NOW             15
#define REASON_SCAN_CH_TIME_CHANGED                 16
#define REASON_SCAN_HOME_TIME_CHANGED               17
#define REASON_OPPORTUNISTIC_THRESH_DIFF_CHANGED    18
#define REASON_ROAM_RESCAN_RSSI_DIFF_CHANGED        19
#define REASON_ROAM_BMISS_FIRST_BCNT_CHANGED        20
#define REASON_ROAM_BMISS_FINAL_BCNT_CHANGED        21
#define REASON_ROAM_BEACON_RSSI_WEIGHT_CHANGED      22
#define REASON_ROAM_DFS_SCAN_MODE_CHANGED           23
#define REASON_ROAM_ABORT_ROAM_SCAN                 24
#define REASON_ROAM_EXT_SCAN_PARAMS_CHANGED         25
#define REASON_ROAM_SET_SSID_ALLOWED                26
#define REASON_ROAM_SET_FAVORED_BSSID               27
#define REASON_ROAM_GOOD_RSSI_CHANGED               28
#define REASON_ROAM_SET_BLACKLIST_BSSID             29
#define REASON_ROAM_SCAN_HI_RSSI_MAXCOUNT_CHANGED   30
#define REASON_ROAM_SCAN_HI_RSSI_DELTA_CHANGED      31
#define REASON_ROAM_SCAN_HI_RSSI_DELAY_CHANGED      32
#define REASON_ROAM_SCAN_HI_RSSI_UB_CHANGED         33

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
QDF_STATUS csr_roam_offload_scan(tpAniSirGlobal pMac, uint8_t sessionId,
		uint8_t command, uint8_t reason);
#else
static inline QDF_STATUS csr_roam_offload_scan(tpAniSirGlobal pMac,
		uint8_t sessionId, uint8_t command, uint8_t reason)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif
QDF_STATUS csr_neighbor_roam_handoff_req_hdlr(tpAniSirGlobal pMac, void *pMsg);
QDF_STATUS csr_neighbor_roam_proceed_with_handoff_req(tpAniSirGlobal pMac,
		uint8_t sessionId);
QDF_STATUS csr_neighbor_roam_sssid_scan_done(tpAniSirGlobal pMac,
		uint8_t sessionId, QDF_STATUS status);
QDF_STATUS csr_neighbor_roam_start_lfr_scan(tpAniSirGlobal pMac,
		uint8_t sessionId);

#ifdef FEATURE_WLAN_ESE
QDF_STATUS csr_set_cckm_ie(tpAniSirGlobal pMac, const uint8_t sessionId,
		const uint8_t *pCckmIe, const uint8_t ccKmIeLen);
QDF_STATUS csr_roam_read_tsf(tpAniSirGlobal pMac, uint8_t *pTimestamp,
		const uint8_t sessionId);
#endif /* FEATURE_WLAN_ESE */
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
void csr_roam_synch_callback(tpAniSirGlobal mac,
	roam_offload_synch_ind *roam_synch_data,
	tpSirBssDescription  bss_desc_ptr, enum sir_roam_op_code reason);
#else
static inline void csr_roam_synch_callback(tpAniSirGlobal mac,
	roam_offload_synch_ind *roam_synch_data,
	tpSirBssDescription  bss_desc_ptr, enum sir_roam_op_code reason)
{}
#endif
void csr_neighbor_roam_state_transition(tpAniSirGlobal mac_ctx,
		uint8_t newstate, uint8_t session);
uint8_t *csr_neighbor_roam_state_to_string(uint8_t state);
tpCsrNeighborRoamBSSInfo csr_neighbor_roam_next_roamable_ap(
		tpAniSirGlobal mac_ctx, tDblLinkList *llist,
		tpCsrNeighborRoamBSSInfo neighbor_entry);
bool csr_neighbor_roam_remove_roamable_ap_list_entry(tpAniSirGlobal pMac,
		tDblLinkList *pList, tpCsrNeighborRoamBSSInfo pNeighborEntry);
void csr_neighbor_roam_free_neighbor_roam_bss_node(tpAniSirGlobal pMac,
		tpCsrNeighborRoamBSSInfo neighborRoamBSSNode);
QDF_STATUS csr_neighbor_roam_issue_preauth_req(tpAniSirGlobal pMac,
		uint8_t sessionId);
bool csr_neighbor_roam_is_preauth_candidate(tpAniSirGlobal pMac,
		    uint8_t sessionId, tSirMacAddr bssId);
#ifdef FEATURE_WLAN_LFR_METRICS
void csr_neighbor_roam_send_lfr_metric_event(tpAniSirGlobal mac_ctx,
		uint8_t session_id, tSirMacAddr bssid, eRoamCmdStatus status);
#else
static inline void csr_neighbor_roam_send_lfr_metric_event(
		tpAniSirGlobal mac_ctx, uint8_t session_id,
		tSirMacAddr bssid, eRoamCmdStatus status)
{}
#endif
QDF_STATUS csr_roam_stop_wait_for_key_timer(tpAniSirGlobal pMac);
uint32_t csr_get_current_ap_rssi(tpAniSirGlobal pMac,
		tScanResultHandle *pScanResultList, uint8_t sessionId);
QDF_STATUS csr_roam_copy_connected_profile(tpAniSirGlobal pMac,
		uint32_t sessionId, tCsrRoamProfile *pDstProfile);

#endif /* CSR_NEIGHBOR_ROAM_H */
