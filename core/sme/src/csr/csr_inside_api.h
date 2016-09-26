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

    \file csr_inside_api.h

    Define interface only used by CSR.
   ========================================================================== */
#ifndef CSR_INSIDE_API_H__
#define CSR_INSIDE_API_H__

#include "csr_support.h"
#include "sme_inside.h"
#include "cds_reg_service.h"

#define CSR_PASSIVE_MAX_CHANNEL_TIME   110
#define CSR_PASSIVE_MIN_CHANNEL_TIME   60

#define CSR_ACTIVE_MAX_CHANNEL_TIME    40
#define CSR_ACTIVE_MIN_CHANNEL_TIME    20

#ifdef WLAN_AP_STA_CONCURRENCY
#define CSR_PASSIVE_MAX_CHANNEL_TIME_CONC   110
#define CSR_PASSIVE_MIN_CHANNEL_TIME_CONC   60

#define CSR_ACTIVE_MAX_CHANNEL_TIME_CONC    27
#define CSR_ACTIVE_MIN_CHANNEL_TIME_CONC    20

#define CSR_REST_TIME_CONC                  100
#define CSR_MIN_REST_TIME_CONC              50
#define CSR_IDLE_TIME_CONC                  25

#define CSR_NUM_STA_CHAN_COMBINED_CONC      3
#define CSR_NUM_P2P_CHAN_COMBINED_CONC      1
#endif

#define CSR_MAX_NUM_SUPPORTED_CHANNELS 55

#define CSR_MAX_2_4_GHZ_SUPPORTED_CHANNELS 14

#define CSR_MAX_BSS_SUPPORT            512
#define SYSTEM_TIME_MSEC_TO_USEC      1000

/* This number minus 1 means the number of times a channel is scanned before a BSS is remvoed from */
/* cache scan result */
#define CSR_AGING_COUNT     3
#define CSR_SCAN_GET_RESULT_INTERVAL    (5 * QDF_MC_TIMER_TO_SEC_UNIT)  /* 5 seconds */
#define CSR_MIC_ERROR_TIMEOUT  (60 * QDF_MC_TIMER_TO_SEC_UNIT)  /* 60 seconds */
#define CSR_TKIP_COUNTER_MEASURE_TIMEOUT  (60 * QDF_MC_TIMER_TO_SEC_UNIT)       /* 60 seconds */

/* the following defines are NOT used by palTimer */
#define CSR_SCAN_AGING_TIME_NOT_CONNECT_NO_PS 50        /* 50 seconds */
#define CSR_SCAN_AGING_TIME_NOT_CONNECT_W_PS 300        /* 300 seconds */
#define CSR_SCAN_AGING_TIME_CONNECT_NO_PS 150   /* 150 seconds */
#define CSR_SCAN_AGING_TIME_CONNECT_W_PS 600    /* 600 seconds */
#define CSR_JOIN_FAILURE_TIMEOUT_DEFAULT (3000)
#define CSR_JOIN_FAILURE_TIMEOUT_MIN   (1000)   /* minimal value */
/* These are going against the signed RSSI (int8_t) so it is between -+127 */
#define CSR_BEST_RSSI_VALUE         (-30)       /* RSSI >= this is in CAT4 */
#define CSR_DEFAULT_RSSI_DB_GAP     30  /* every 30 dbm for one category */
#define CSR_BSS_CAP_VALUE_NONE  0       /* not much value */
#define CSR_BSS_CAP_VALUE_HT    1
#define CSR_BSS_CAP_VALUE_VHT    2
#define CSR_BSS_CAP_VALUE_WMM   1
#define CSR_BSS_CAP_VALUE_UAPSD 1
#define CSR_BSS_CAP_VALUE_5GHZ  2

#define CSR_ROAMING_DFS_CHANNEL_DISABLED           (0)
#define CSR_ROAMING_DFS_CHANNEL_ENABLED_NORMAL     (1)
#define CSR_ROAMING_DFS_CHANNEL_ENABLED_ACTIVE     (2)

/* ***************************************************************************
 * The MAX BSSID Count should be lower than the command timeout value and it
 * can be of a fraction of 3/4 of the total command timeout value.
 * ***************************************************************************/
#define CSR_ACTIVE_LIST_CMD_TIMEOUT_VALUE (1000*30*4)
#ifdef QCA_WIFI_3_0_EMU
#define CSR_ACTIVE_SCAN_LIST_CMD_TIMEOUT (1000*30*20)
#else
#define CSR_ACTIVE_SCAN_LIST_CMD_TIMEOUT (1000*30)
#endif
#define CSR_MAX_BSSID_COUNT     ((CSR_ACTIVE_LIST_CMD_TIMEOUT_VALUE/4000) * 3)
#define CSR_CUSTOM_CONC_GO_BI    100

typedef enum {
	eCsrNextScanNothing,
	eCsrNextLostLinkScan1Success,
	eCsrNextLostLinkScan1Failed,
	eCsrNextLostLinkScan2Success,
	eCsrNextLostLinkScan2Failed,
	eCsrNextLostLinkScan3Success,
	eCsrNexteScanForSsidSuccess,
	eCsrNextLostLinkScan3Failed,
	eCsrNext11dScan1Failure,
	eCsrNext11dScan1Success,
	eCsrNext11dScan2Failure,
	eCsrNext11dScan2Success,
	eCsrNext11dScanComplete,
	eCsrNexteScanForSsidFailure,
	eCsrNextCheckAllowConc,

} eCsrScanCompleteNextCommand;

typedef enum {
	eCsrJoinSuccess,
	eCsrJoinFailure,
	eCsrReassocSuccess,
	eCsrReassocFailure,
	eCsrNothingToJoin,
	eCsrStartBssSuccess,
	eCsrStartBssFailure,
	eCsrSilentlyStopRoaming,
	eCsrSilentlyStopRoamingSaveState,
	eCsrJoinFailureDueToConcurrency,
	eCsrStopBssSuccess,
	eCsrStopBssFailure,
} eCsrRoamCompleteResult;

typedef struct tagScanReqParam {
	uint8_t bReturnAfter1stMatch;
	uint8_t fUniqueResult;
	uint8_t freshScan;
	uint8_t hiddenSsid;
	uint8_t reserved;
} tScanReqParam;

typedef struct tagCsrScanResult {
	tListElem Link;
	int32_t AgingCount;     /* This BSS is removed when it reaches 0 or less */
	uint32_t preferValue;   /* The bigger the number, the better the BSS. This value override capValue */
	uint32_t capValue;      /* The biggger the better. This value is in use only if we have equal preferValue */
	/* This member must be the last in the structure because the end of tSirBssDescription (inside) is an */
	/*    array with nonknown size at this time */

	eCsrEncryptionType ucEncryptionType;    /* Preferred Encryption type that matched with profile. */
	eCsrEncryptionType mcEncryptionType;
	eCsrAuthType authType;  /* Preferred auth type that matched with the profile. */

	tCsrScanResultInfo Result;
} tCsrScanResult;

typedef struct {
	tDblLinkList List;
	tListElem *pCurEntry;
} tScanResultList;

#define CSR_IS_ROAM_REASON(pCmd, reason) ((reason) == (pCmd)->roamCmd.roamReason)
#define CSR_IS_BETTER_PREFER_VALUE(v1, v2)   ((v1) > (v2))
#define CSR_IS_EQUAL_PREFER_VALUE(v1, v2)   ((v1) == (v2))
#define CSR_IS_BETTER_CAP_VALUE(v1, v2)     ((v1) > (v2))
#define CSR_IS_EQUAL_CAP_VALUE(v1, v2)  ((v1) == (v2))
#define CSR_IS_BETTER_RSSI(v1, v2)   ((v1) > (v2))
#define CSR_IS_ENC_TYPE_STATIC(encType) ((eCSR_ENCRYPT_TYPE_NONE == (encType)) || \
					    (eCSR_ENCRYPT_TYPE_WEP40_STATICKEY == (encType)) || \
					    (eCSR_ENCRYPT_TYPE_WEP104_STATICKEY == (encType)))
#define CSR_IS_WAIT_FOR_KEY(pMac, sessionId) \
		 (CSR_IS_ROAM_JOINED(pMac, sessionId) && \
		  CSR_IS_ROAM_SUBSTATE_WAITFORKEY(pMac, sessionId))
/* WIFI has a test case for not using HT rates with TKIP as encryption */
/* We may need to add WEP but for now, TKIP only. */

#define CSR_IS_11n_ALLOWED(encType) ((eCSR_ENCRYPT_TYPE_TKIP != (encType)) && \
				       (eCSR_ENCRYPT_TYPE_WEP40_STATICKEY != (encType)) && \
				       (eCSR_ENCRYPT_TYPE_WEP104_STATICKEY != (encType)) && \
				       (eCSR_ENCRYPT_TYPE_WEP40 != (encType)) && \
				       (eCSR_ENCRYPT_TYPE_WEP104 != (encType)))

#define CSR_IS_DISCONNECT_COMMAND(pCommand) ((eSmeCommandRoam == (pCommand)->command) && \
					      ((eCsrForcedDisassoc == (pCommand)->u.roamCmd.roamReason) || \
						(eCsrForcedDeauth == (pCommand)->u.roamCmd.roamReason) || \
						(eCsrSmeIssuedDisassocForHandoff == \
						  (pCommand)->u.roamCmd.roamReason) ||	\
						(eCsrForcedDisassocMICFailure == \
						  (pCommand)->u.roamCmd.roamReason)))

eCsrRoamState csr_roam_state_change(tpAniSirGlobal pMac,
				    eCsrRoamState NewRoamState, uint8_t sessionId);
QDF_STATUS csr_scanning_state_msg_processor(tpAniSirGlobal pMac, void *pMsgBuf);
void csr_roaming_state_msg_processor(tpAniSirGlobal pMac, void *pMsgBuf);
void csr_roam_joined_state_msg_processor(tpAniSirGlobal pMac, void *pMsgBuf);
bool csr_scan_complete(tpAniSirGlobal pMac, tSirSmeScanRsp *pScanRsp);
void csr_release_command_roam(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_release_command_scan(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_release_command_wm_status_change(tpAniSirGlobal pMac, tSmeCmd *pCommand);
extern void csr_release_roc_req_cmd(tpAniSirGlobal mac_ctx);

bool csr_is_duplicate_bss_description(tpAniSirGlobal pMac,
				      tSirBssDescription *pSirBssDesc1,
				      tSirBssDescription *pSirBssDesc2,
				      tDot11fBeaconIEs *pIes2, bool fForced);
QDF_STATUS csr_roam_save_connected_bss_desc(tpAniSirGlobal pMac, uint32_t sessionId,
					    tSirBssDescription *pBssDesc);
bool csr_is_network_type_equal(tSirBssDescription *pSirBssDesc1,
			       tSirBssDescription *pSirBssDesc2);
QDF_STATUS csr_scan_sme_scan_response(tpAniSirGlobal pMac, void *pMsgBuf);
/*
   Prepare a filter base on a profile for parsing the scan results.
   Upon successful return, caller MUST call csr_free_scan_filter on
   pScanFilter when it is done with the filter.
 */
QDF_STATUS csr_roam_prepare_filter_from_profile(tpAniSirGlobal pMac,
						tCsrRoamProfile *pProfile,
						tCsrScanResultFilter *pScanFilter);
QDF_STATUS csr_roam_copy_profile(tpAniSirGlobal pMac,
				 tCsrRoamProfile *pDstProfile,
				 tCsrRoamProfile *pSrcProfile);
QDF_STATUS csr_roam_start(tpAniSirGlobal pMac);
void csr_roam_stop(tpAniSirGlobal pMac, uint32_t sessionId);
void csr_roam_startMICFailureTimer(tpAniSirGlobal pMac);
void csr_roam_stopMICFailureTimer(tpAniSirGlobal pMac);
void csr_roam_startTKIPCounterMeasureTimer(tpAniSirGlobal pMac);
void csr_roam_stopTKIPCounterMeasureTimer(tpAniSirGlobal pMac);

QDF_STATUS csr_scan_open(tpAniSirGlobal pMac);
QDF_STATUS csr_scan_close(tpAniSirGlobal pMac);
QDF_STATUS csr_scan_request_lost_link1(tpAniSirGlobal pMac, uint32_t sessionId);
QDF_STATUS csr_scan_request_lost_link2(tpAniSirGlobal pMac, uint32_t sessionId);
QDF_STATUS csr_scan_request_lost_link3(tpAniSirGlobal pMac, uint32_t sessionId);
QDF_STATUS csr_scan_handle_failed_lostlink1(tpAniSirGlobal pMac,
					    uint32_t sessionId);
QDF_STATUS csr_scan_handle_failed_lostlink2(tpAniSirGlobal pMac,
					    uint32_t sessionId);
QDF_STATUS csr_scan_handle_failed_lostlink3(tpAniSirGlobal pMac,
					    uint32_t sessionId);
tCsrScanResult *csr_scan_append_bss_description(tpAniSirGlobal pMac,
						tSirBssDescription *
						pSirBssDescription,
						tDot11fBeaconIEs *pIes,
						bool fForced, uint8_t sessionId);
void csr_scan_call_callback(tpAniSirGlobal pMac, tSmeCmd *pCommand,
			    eCsrScanStatus scanStatus);
QDF_STATUS csr_scan_copy_request(tpAniSirGlobal pMac, tCsrScanRequest *pDstReq,
				 tCsrScanRequest *pSrcReq);
QDF_STATUS csr_scan_free_request(tpAniSirGlobal pMac, tCsrScanRequest *pReq);
QDF_STATUS csr_scan_copy_result_list(tpAniSirGlobal pMac, tScanResultHandle hIn,
				     tScanResultHandle *phResult);
QDF_STATUS csr_scan_for_ssid(tpAniSirGlobal pMac, uint32_t sessionId,
			     tCsrRoamProfile *pProfile, uint32_t roamId,
			     bool notify);
/* To remove fresh scan commands from the pending queue */
bool csr_scan_remove_fresh_scan_command(tpAniSirGlobal pMac, uint8_t sessionId);
QDF_STATUS csr_scan_abort_mac_scan(tpAniSirGlobal pMac, uint8_t sessionId,
				   eCsrAbortReason reason);
void csr_remove_cmd_from_pending_list(tpAniSirGlobal pMac, tDblLinkList *pList,
				      eSmeCommandType commandType);
void csr_remove_cmd_with_session_id_from_pending_list(tpAniSirGlobal pMac,
						      uint8_t sessionId,
						      tDblLinkList *pList,
						      eSmeCommandType commandType);
QDF_STATUS csr_scan_abort_mac_scan_not_for_connect(tpAniSirGlobal pMac,
						   uint8_t sessionId);
QDF_STATUS csr_scan_abort_scan_for_ssid(tpAniSirGlobal pMac, uint32_t sessionId);
void csr_remove_scan_for_ssid_from_pending_list(tpAniSirGlobal pMac,
						tDblLinkList *pList,
						uint32_t sessionId);

QDF_STATUS csr_abort_scan_from_active_list(tpAniSirGlobal pMac,
		tDblLinkList *pList, uint32_t sessionId,
		eSmeCommandType scan_cmd_type, eCsrAbortReason abort_reason);

/* To age out scan results base. tSmeGetScanChnRsp is a pointer returned by LIM that */
/* has the information regarding scanned channels. */
/* The logic is that whenever CSR add a BSS to scan result, it set the age count to */
/* a value. This function deduct the age count if channelId matches the BSS' channelId */
/* The BSS is remove if the count reaches 0. */
QDF_STATUS csr_scan_age_results(tpAniSirGlobal pMac,
				tSmeGetScanChnRsp *pScanChnInfo);

/* If fForce is true we will save the new String that is learn't. */
/* Typically it will be true in case of Join or user initiated ioctl */
bool csr_learn_11dcountry_information(tpAniSirGlobal pMac,
				   tSirBssDescription *pSirBssDesc,
				   tDot11fBeaconIEs *pIes, bool fForce);
void csr_apply_country_information(tpAniSirGlobal pMac);
void csr_set_cfg_scan_control_list(tpAniSirGlobal pMac, uint8_t *countryCode,
				   tCsrChannel *pChannelList);
void csr_reinit_scan_cmd(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_free_scan_result_entry(tpAniSirGlobal pMac, tCsrScanResult *pResult);

QDF_STATUS csr_roam_call_callback(tpAniSirGlobal pMac, uint32_t sessionId,
				  tCsrRoamInfo *pRoamInfo, uint32_t roamId,
				  eRoamCmdStatus u1, eCsrRoamResult u2);
QDF_STATUS csr_roam_issue_connect(tpAniSirGlobal pMac, uint32_t sessionId,
				  tCsrRoamProfile *pProfile,
				  tScanResultHandle hBSSList,
				  eCsrRoamReason reason, uint32_t roamId,
				  bool fImediate, bool fClearScan);
QDF_STATUS csr_roam_issue_reassoc(tpAniSirGlobal pMac, uint32_t sessionId,
				  tCsrRoamProfile *pProfile,
				  tCsrRoamModifyProfileFields *pModProfileFields,
				  eCsrRoamReason reason, uint32_t roamId,
				  bool fImediate);
void csr_roam_complete(tpAniSirGlobal pMac, eCsrRoamCompleteResult Result,
		       void *Context);
QDF_STATUS csr_roam_issue_set_context_req(tpAniSirGlobal pMac, uint32_t sessionId,
					  eCsrEncryptionType EncryptType,
					  tSirBssDescription *pBssDescription,
					  tSirMacAddr *bssId, bool addKey,
					  bool fUnicast,
					  tAniKeyDirection aniKeyDirection,
					  uint8_t keyId, uint16_t keyLength,
					  uint8_t *pKey, uint8_t paeRole);
QDF_STATUS csr_roam_process_disassoc_deauth(tpAniSirGlobal pMac, tSmeCmd *pCommand,
					    bool fDisassoc, bool fMICFailure);
QDF_STATUS csr_roam_save_connected_infomation(tpAniSirGlobal pMac,
					      uint32_t sessionId,
					      tCsrRoamProfile *pProfile,
					      tSirBssDescription *pSirBssDesc,
					      tDot11fBeaconIEs *pIes);
void csr_roam_check_for_link_status_change(tpAniSirGlobal pMac, tSirSmeRsp *pSirMsg);
void csr_roam_stats_rsp_processor(tpAniSirGlobal pMac, tSirSmeRsp *pSirMsg);
QDF_STATUS csr_roam_issue_start_bss(tpAniSirGlobal pMac, uint32_t sessionId,
				    tCsrRoamStartBssParams *pParam,
				    tCsrRoamProfile *pProfile,
				    tSirBssDescription *pBssDesc, uint32_t roamId);
QDF_STATUS csr_roam_issue_stop_bss(tpAniSirGlobal pMac, uint32_t sessionId,
				   eCsrRoamSubState NewSubstate);
bool csr_is_same_profile(tpAniSirGlobal pMac, tCsrRoamConnectedProfile *pProfile1,
			 tCsrRoamProfile *pProfile2);
bool csr_is_roam_command_waiting(tpAniSirGlobal pMac);
bool csr_is_roam_command_waiting_for_session(tpAniSirGlobal pMac, uint32_t sessionId);
bool csr_is_scan_for_roam_command_active(tpAniSirGlobal pMac);
eRoamCmdStatus csr_get_roam_complete_status(tpAniSirGlobal pMac,
					    uint32_t sessionId);
/* pBand can be NULL if caller doesn't need to get it */
QDF_STATUS csr_roam_issue_disassociate_cmd(tpAniSirGlobal pMac, uint32_t sessionId,
					   eCsrRoamDisconnectReason reason);
QDF_STATUS csr_roam_disconnect_internal(tpAniSirGlobal pMac, uint32_t sessionId,
					eCsrRoamDisconnectReason reason);
/* pCommand may be NULL */
void csr_roam_remove_duplicate_command(tpAniSirGlobal pMac, uint32_t sessionId,
				       tSmeCmd *pCommand,
				       eCsrRoamReason eRoamReason);

QDF_STATUS csr_send_join_req_msg(tpAniSirGlobal pMac, uint32_t sessionId,
				 tSirBssDescription *pBssDescription,
				 tCsrRoamProfile *pProfile,
				 tDot11fBeaconIEs *pIes, uint16_t messageType);
QDF_STATUS csr_send_mb_disassoc_req_msg(tpAniSirGlobal pMac, uint32_t sessionId,
					tSirMacAddr bssId, uint16_t reasonCode);
QDF_STATUS csr_send_mb_deauth_req_msg(tpAniSirGlobal pMac, uint32_t sessionId,
				      tSirMacAddr bssId, uint16_t reasonCode);
QDF_STATUS csr_send_mb_disassoc_cnf_msg(tpAniSirGlobal pMac,
					tpSirSmeDisassocInd pDisassocInd);
QDF_STATUS csr_send_mb_deauth_cnf_msg(tpAniSirGlobal pMac,
				      tpSirSmeDeauthInd pDeauthInd);
QDF_STATUS csr_send_assoc_cnf_msg(tpAniSirGlobal pMac, tpSirSmeAssocInd pAssocInd,
				  QDF_STATUS status);
QDF_STATUS csr_send_assoc_ind_to_upper_layer_cnf_msg(tpAniSirGlobal pMac,
						     tpSirSmeAssocInd pAssocInd,
						     QDF_STATUS Halstatus,
						     uint8_t sessionId);
QDF_STATUS csr_send_mb_start_bss_req_msg(tpAniSirGlobal pMac, uint32_t sessionId,
					 eCsrRoamBssType bssType,
					 tCsrRoamStartBssParams *pParam,
					 tSirBssDescription *pBssDesc);
QDF_STATUS csr_send_mb_stop_bss_req_msg(tpAniSirGlobal pMac, uint32_t sessionId);

/* Caller should put the BSS' ssid to fiedl bssSsid when comparing SSID for a BSS. */
bool csr_is_ssid_match(tpAniSirGlobal pMac, uint8_t *ssid1, uint8_t ssid1Len,
		       uint8_t *bssSsid, uint8_t bssSsidLen, bool fSsidRequired);
bool csr_is_phy_mode_match(tpAniSirGlobal pMac, uint32_t phyMode,
			   tSirBssDescription *pSirBssDesc,
			   tCsrRoamProfile *pProfile,
			   eCsrCfgDot11Mode *pReturnCfgDot11Mode,
			   tDot11fBeaconIEs *pIes);
bool csr_roam_is_channel_valid(tpAniSirGlobal pMac, uint8_t channel);

/* pNumChan is a caller allocated space with the sizeof pChannels */
QDF_STATUS csr_get_cfg_valid_channels(tpAniSirGlobal pMac, uint8_t *pChannels,
				      uint32_t *pNumChan);
void csr_roam_ccm_cfg_set_callback(tpAniSirGlobal pMac, int32_t result);

int8_t csr_get_cfg_max_tx_power(tpAniSirGlobal pMac, uint8_t channel);

/* To free the last roaming profile */
void csr_free_roam_profile(tpAniSirGlobal pMac, uint32_t sessionId);
void csr_free_connect_bss_desc(tpAniSirGlobal pMac, uint32_t sessionId);
QDF_STATUS csr_move_bss_to_head_from_bssid(tpAniSirGlobal pMac,
					   struct qdf_mac_addr *bssid,
					   tScanResultHandle hScanResult);
bool csr_check_ps_ready(void *pv);
bool csr_check_ps_offload_ready(void *pv, uint32_t sessionId);

/* to free memory allocated inside the profile structure */
void csr_release_profile(tpAniSirGlobal pMac, tCsrRoamProfile *pProfile);
/* To free memory allocated inside scanFilter */
void csr_free_scan_filter(tpAniSirGlobal pMac, tCsrScanResultFilter *pScanFilter);
eCsrCfgDot11Mode csr_get_cfg_dot11_mode_from_csr_phy_mode(tCsrRoamProfile *pProfile,
							  eCsrPhyMode phyMode,
							  bool fProprietary);
uint32_t csr_translate_to_wni_cfg_dot11_mode(tpAniSirGlobal pMac,
					     eCsrCfgDot11Mode csrDot11Mode);
void csr_save_channel_power_for_band(tpAniSirGlobal pMac, bool fPopulate5GBand);
void csr_apply_channel_power_info_to_fw(tpAniSirGlobal pMac,
					tCsrChannel *pChannelList,
					uint8_t *countryCode);
void csr_apply_power2_current(tpAniSirGlobal pMac);
void csr_assign_rssi_for_category(tpAniSirGlobal pMac, int8_t bestApRssi,
				  uint8_t catOffset);
QDF_STATUS csr_roam_remove_connected_bss_from_scan_cache(tpAniSirGlobal pMac,
							 tCsrRoamConnectedProfile *
							 pConnProfile);
QDF_STATUS csr_roam_start_roaming(tpAniSirGlobal pMac, uint32_t sessionId,
				  eCsrRoamingReason roamingReason);
/* return a bool to indicate whether roaming completed or continue. */
bool csr_roam_complete_roaming(tpAniSirGlobal pMac, uint32_t sessionId,
			       bool fForce, eCsrRoamResult roamResult);
void csr_roam_completion(tpAniSirGlobal pMac, uint32_t sessionId,
			 tCsrRoamInfo *pRoamInfo, tSmeCmd *pCommand,
			 eCsrRoamResult roamResult, bool fSuccess);
void csr_roam_cancel_roaming(tpAniSirGlobal pMac, uint32_t sessionId);
void csr_apply_channel_power_info_wrapper(tpAniSirGlobal pMac);
void csr_reset_pmkid_candidate_list(tpAniSirGlobal pMac, uint32_t sessionId);
#ifdef FEATURE_WLAN_WAPI
void csr_reset_bkid_candidate_list(tpAniSirGlobal pMac, uint32_t sessionId);
#endif /* FEATURE_WLAN_WAPI */
QDF_STATUS csr_save_to_channel_power2_g_5_g(tpAniSirGlobal pMac, uint32_t tableSize,
					    tSirMacChanInfo *channelTable);

/* To check whether a country code matches the one in the IE */
/* Only check the first two characters, ignoring in/outdoor */
/* pCountry -- caller allocated buffer contain the country code that is checking against */
/* the one in pIes. It can be NULL. */
/* caller must provide pIes, it cannot be NULL */
/* This function always return true if 11d support is not turned on. */
/* pIes cannot be NULL */
bool csr_match_country_code(tpAniSirGlobal pMac, uint8_t *pCountry,
			    tDot11fBeaconIEs *pIes);
QDF_STATUS csr_roam_set_key(tpAniSirGlobal pMac, uint32_t sessionId,
			    tCsrRoamSetKey *pSetKey, uint32_t roamId);
QDF_STATUS csr_roam_open_session(tpAniSirGlobal pMac,
				 csr_roam_completeCallback callback, void *pContext,
				 uint8_t *pSelfMacAddr, uint8_t *pbSessionId,
				 uint32_t type, uint32_t subType);
/* fSync: true means cleanupneeds to handle synchronously. */
QDF_STATUS csr_roam_close_session(tpAniSirGlobal pMac, uint32_t sessionId,
				  bool fSync,
				  csr_roamSessionCloseCallback callback,
				  void *pContext);
void csr_cleanup_session(tpAniSirGlobal pMac, uint32_t sessionId);
QDF_STATUS csr_roam_get_session_id_from_bssid(tpAniSirGlobal pMac, struct qdf_mac_addr *bssid,
					      uint32_t *pSessionId);
eCsrCfgDot11Mode csr_find_best_phy_mode(tpAniSirGlobal pMac, uint32_t phyMode);

/* ---------------------------------------------------------------------------
    \fn csr_scan_enable
    \brief Enable the scanning feature of CSR. It must be called before any scan request can be performed.
    \param tHalHandle - HAL context handle
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_scan_enable(tpAniSirGlobal);

/* ---------------------------------------------------------------------------
    \fn csr_scan_disable
    \brief Disableing the scanning feature of CSR. After this function return success, no scan is performed until
   a successfull to csr_scan_enable
    \param tHalHandle - HAL context handle
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_scan_disable(tpAniSirGlobal);
/* ---------------------------------------------------------------------------
    \fn csr_scan_request
    \brief Request a 11d or full scan.
    \param callback - a callback function that scan calls upon finish, will not be called if csr_scan_request returns error
    \param pContext - a pointer passed in for the callback
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_scan_request(tpAniSirGlobal, uint16_t, tCsrScanRequest *,
			csr_scan_completeCallback callback, void *pContext);

/* ---------------------------------------------------------------------------
    \fn csrScanAbort
    \brief If a scan request is abort, the scan complete callback will be called first before csrScanAbort returns.
    \param pScanRequestID - The request ID returned from csr_scan_request
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csrScanAbort(tpAniSirGlobal, uint32_t scanRequestID);

/* ---------------------------------------------------------------------------
    \fn csr_scan_get_result
    \brief Return scan results.
    \param pFilter - If pFilter is NULL, all cached results are returned
    \param phResult - an object for the result.
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_scan_get_result(tpAniSirGlobal, tCsrScanResultFilter *pFilter,
			       tScanResultHandle *phResult);

/* ---------------------------------------------------------------------------
    \fn csr_scan_flush_result
    \brief Clear scan results.
    \param pMac - pMac global pointer
    \param sessionId - Session Identifier
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_scan_flush_result(tpAniSirGlobal);
/* ---------------------------------------------------------------------------
 *  \fn csr_scan_filter_results
 *  \brief Filter scan results based on valid channel list.
 *  \param  pMac - Pointer to Global MAC structure
 *  \return QDF_STATUS
 ***-------------------------------------------------------------------------------
 */
QDF_STATUS csr_scan_filter_results(tpAniSirGlobal pMac);

void csr_save_scan_results(tpAniSirGlobal pMac, uint8_t reason,
				  uint8_t sessionId);

QDF_STATUS csr_scan_flush_selective_result(tpAniSirGlobal, bool flushP2P);

/* ---------------------------------------------------------------------------
    \fn csr_scan_result_get_first
    \brief Returns the first element of scan result.
    \param hScanResult - returned from csr_scan_get_result
    \return tCsrScanResultInfo * - NULL if no result
   -------------------------------------------------------------------------------*/
tCsrScanResultInfo *csr_scan_result_get_first(tpAniSirGlobal,
					      tScanResultHandle hScanResult);
/* ---------------------------------------------------------------------------
    \fn csr_scan_result_get_next
    \brief Returns the next element of scan result. It can be called without calling csr_scan_result_get_first first
    \param hScanResult - returned from csr_scan_get_result
    \return Null if no result or reach the end
   -------------------------------------------------------------------------------*/
tCsrScanResultInfo *csr_scan_result_get_next(tpAniSirGlobal,
					     tScanResultHandle hScanResult);

/* ---------------------------------------------------------------------------
    \fn csr_get_country_code
    \brief this function is to get the country code current being used
    \param pBuf - Caller allocated buffer with at least 3 bytes, upon success return, this has the country code
    \param pbLen - Caller allocated, as input, it indicates the length of pBuf. Upon success return,
    this contains the length of the data in pBuf
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_get_country_code(tpAniSirGlobal pMac, uint8_t *pBuf,
				uint8_t *pbLen);

/* ---------------------------------------------------------------------------
    \fn csr_set_country_code
    \brief this function is to set the country code so channel/power setting matches the countrycode and
    the domain it belongs to.
    \param pCountry - Caller allocated buffer with at least 3 bytes specifying the country code
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_set_country_code(tpAniSirGlobal pMac, uint8_t *pCountry);

/* ---------------------------------------------------------------------------
    \fn csr_get_regulatory_domain_for_country
    \brief this function is to get the regulatory domain for a country.
    This function must be called after CFG is downloaded and all the band/mode setting already passed into
    CSR.
    \param pCountry - Caller allocated buffer with at least 3 bytes specifying the country code
    \param pDomainId - Caller allocated buffer to get the return domain ID upon success return. Can be NULL.
    \param source - the source of country information.
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_get_regulatory_domain_for_country(tpAniSirGlobal pMac,
						 uint8_t *pCountry,
						 v_REGDOMAIN_t *pDomainId,
						 enum country_src source);

/* some support functions */
bool csr_is11d_supported(tpAniSirGlobal pMac);
bool csr_is11h_supported(tpAniSirGlobal pMac);
bool csr_is11e_supported(tpAniSirGlobal pMac);
bool csr_is_wmm_supported(tpAniSirGlobal pMac);
bool csr_is_mcc_supported(tpAniSirGlobal pMac);

/* Return SUCCESS is the command is queued, failed */
QDF_STATUS csr_queue_sme_command(tpAniSirGlobal pMac, tSmeCmd *pCommand,
				 bool fHighPriority);
tSmeCmd *csr_get_command_buffer(tpAniSirGlobal pMac);
void csr_release_command(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_scan_flush_bss_entry(tpAniSirGlobal pMac,
			      tpSmeCsaOffloadInd pCsaOffloadInd);
QDF_STATUS csr_get_active_scan_entry(tpAniSirGlobal mac, uint32_t scan_id,
	tListElem **entry);

#ifdef FEATURE_WLAN_WAPI
bool csr_is_profile_wapi(tCsrRoamProfile *pProfile);
#endif /* FEATURE_WLAN_WAPI */

void csr_get_vdev_type_nss(tpAniSirGlobal mac_ctx,
		enum tQDF_ADAPTER_MODE dev_mode,
		uint8_t *nss_2g, uint8_t *nss_5g);

#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR

/* Security */
#define WLAN_SECURITY_EVENT_REMOVE_KEY_REQ  5
#define WLAN_SECURITY_EVENT_REMOVE_KEY_RSP  6
#define WLAN_SECURITY_EVENT_PMKID_CANDIDATE_FOUND  7
#define WLAN_SECURITY_EVENT_PMKID_UPDATE    8
#define WLAN_SECURITY_EVENT_MIC_ERROR       9
#define WLAN_SECURITY_EVENT_SET_UNICAST_REQ  10
#define WLAN_SECURITY_EVENT_SET_UNICAST_RSP  11
#define WLAN_SECURITY_EVENT_SET_BCAST_REQ    12
#define WLAN_SECURITY_EVENT_SET_BCAST_RSP    13


#define AUTH_OPEN       0
#define AUTH_SHARED     1
#define AUTH_WPA_EAP    2
#define AUTH_WPA_PSK    3
#define AUTH_WPA2_EAP   4
#define AUTH_WPA2_PSK   5
#ifdef FEATURE_WLAN_WAPI
#define AUTH_WAPI_CERT  6
#define AUTH_WAPI_PSK   7
#endif /* FEATURE_WLAN_WAPI */

#define ENC_MODE_OPEN   0
#define ENC_MODE_WEP40  1
#define ENC_MODE_WEP104 2
#define ENC_MODE_TKIP   3
#define ENC_MODE_AES    4
#ifdef FEATURE_WLAN_WAPI
#define ENC_MODE_SMS4   5       /* WAPI */
#endif /* FEATURE_WLAN_WAPI */

#define NO_MATCH    0
#define MATCH       1

#define WLAN_SECURITY_STATUS_SUCCESS        0
#define WLAN_SECURITY_STATUS_FAILURE        1

/* Scan */
#define WLAN_SCAN_EVENT_ACTIVE_SCAN_REQ     1
#define WLAN_SCAN_EVENT_ACTIVE_SCAN_RSP     2
#define WLAN_SCAN_EVENT_PASSIVE_SCAN_REQ    3
#define WLAN_SCAN_EVENT_PASSIVE_SCAN_RSP    4
#define WLAN_SCAN_EVENT_HO_SCAN_REQ         5
#define WLAN_SCAN_EVENT_HO_SCAN_RSP         6

#define WLAN_SCAN_STATUS_SUCCESS        0
#define WLAN_SCAN_STATUS_FAILURE        1
#define WLAN_SCAN_STATUS_ABORT          2

/* Ibss */
#define WLAN_IBSS_EVENT_START_IBSS_REQ      0
#define WLAN_IBSS_EVENT_START_IBSS_RSP      1
#define WLAN_IBSS_EVENT_JOIN_IBSS_REQ       2
#define WLAN_IBSS_EVENT_JOIN_IBSS_RSP       3
#define WLAN_IBSS_EVENT_COALESCING          4
#define WLAN_IBSS_EVENT_PEER_JOIN           5
#define WLAN_IBSS_EVENT_PEER_LEAVE          6
#define WLAN_IBSS_EVENT_STOP_REQ            7
#define WLAN_IBSS_EVENT_STOP_RSP            8

#define AUTO_PICK       0
#define SPECIFIED       1

#define WLAN_IBSS_STATUS_SUCCESS        0
#define WLAN_IBSS_STATUS_FAILURE        1

/* 11d */
#define WLAN_80211D_EVENT_COUNTRY_SET   0
#define WLAN_80211D_EVENT_RESET         1

#define WLAN_80211D_DISABLED         0
#define WLAN_80211D_SUPPORT_MULTI_DOMAIN     1
#define WLAN_80211D_NOT_SUPPORT_MULTI_DOMAIN     2

int diag_auth_type_from_csr_type(eCsrAuthType authType);
int diag_enc_type_from_csr_type(eCsrEncryptionType encType);
#endif /* #ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR */
/* ---------------------------------------------------------------------------
    \fn csr_scan_result_purge
    \brief remove all items(tCsrScanResult) in the list and free memory for each item
    \param hScanResult - returned from csr_scan_get_result. hScanResult is considered gone by
    calling this function and even before this function reutrns.
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_scan_result_purge(tpAniSirGlobal pMac,
				 tScanResultHandle hScanResult);

/* /////////////////////////////////////////Common Scan ends */

/* ---------------------------------------------------------------------------
    \fn csr_roam_connect
    \brief To inititiate an association
    \param pProfile - can be NULL to join to any open ones
    \param pRoamId - to get back the request ID
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_roam_connect(tpAniSirGlobal pMac, uint32_t sessionId,
			    tCsrRoamProfile *pProfile,
			    uint32_t *pRoamId);

/* ---------------------------------------------------------------------------
    \fn csr_roam_reassoc
    \brief To inititiate a re-association
    \param pProfile - can be NULL to join the currently connected AP. In that
    case modProfileFields should carry the modified field(s) which could trigger
    reassoc
    \param modProfileFields - fields which are part of tCsrRoamConnectedProfile
    that might need modification dynamically once STA is up & running and this
    could trigger a reassoc
    \param pRoamId - to get back the request ID
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_roam_reassoc(tpAniSirGlobal pMac, uint32_t sessionId,
			    tCsrRoamProfile *pProfile,
			    tCsrRoamModifyProfileFields modProfileFields,
			    uint32_t *pRoamId);

/* ---------------------------------------------------------------------------
    \fn csr_roam_reconnect
    \brief To disconnect and reconnect with the same profile
    \return QDF_STATUS. It returns fail if currently not connected
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_roam_reconnect(tpAniSirGlobal pMac, uint32_t sessionId);

/* ---------------------------------------------------------------------------
    \fn csr_roam_set_pmkid_cache
    \brief return the PMKID candidate list
    \param pPMKIDCache - caller allocated buffer point to an array of tPmkidCacheInfo
    \param numItems - a variable that has the number of tPmkidCacheInfo allocated
    when retruning, this is either the number needed or number of items put into pPMKIDCache
    \return QDF_STATUS - when fail, it usually means the buffer allocated is not big enough and pNumItems
    has the number of tPmkidCacheInfo.
    \Note: pNumItems is a number of tPmkidCacheInfo, not sizeof(tPmkidCacheInfo) * something
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_roam_set_pmkid_cache(tpAniSirGlobal pMac, uint32_t sessionId,
				    tPmkidCacheInfo *pPMKIDCache,
				    uint32_t numItems, bool update_entire_cache);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/* ---------------------------------------------------------------------------
   *\fn csr_roam_set_psk_pmk
   *\brief store PSK/PMK
   *\param pMac  - pointer to global structure for MAC
   *\param sessionId - Sme session id
   *\param pPSK_PMK - pointer to an array of Psk/Pmk
   *\return QDF_STATUS - usually it succeed unless sessionId is not found
   *\Note:
 *-------------------------------------------------------------------------------*/
QDF_STATUS csr_roam_set_psk_pmk(tpAniSirGlobal pMac, uint32_t sessionId,
				uint8_t *pPSK_PMK, size_t pmk_len);

QDF_STATUS csr_roam_set_key_mgmt_offload(tpAniSirGlobal mac_ctx,
					 uint32_t session_id,
					 bool roam_key_mgmt_offload_enabled,
					 bool okc_enabled);
#endif
/* ---------------------------------------------------------------------------
    \fn csr_roam_get_wpa_rsn_req_ie
    \brief return the WPA or RSN IE CSR passes to PE to JOIN request or START_BSS request
    \param pLen - caller allocated memory that has the length of pBuf as input. Upon returned, *pLen has the
    needed or IE length in pBuf.
    \param pBuf - Caller allocated memory that contain the IE field, if any, upon return
    \return QDF_STATUS - when fail, it usually means the buffer allocated is not big enough
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_roam_get_wpa_rsn_req_ie(tpAniSirGlobal pMac, uint32_t sessionId,
				       uint32_t *pLen, uint8_t *pBuf);

/* ---------------------------------------------------------------------------
    \fn csr_roam_get_wpa_rsn_rsp_ie
    \brief return the WPA or RSN IE from the beacon or probe rsp if connected
    \param pLen - caller allocated memory that has the length of pBuf as input. Upon returned, *pLen has the
    needed or IE length in pBuf.
    \param pBuf - Caller allocated memory that contain the IE field, if any, upon return
    \return QDF_STATUS - when fail, it usually means the buffer allocated is not big enough
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_roam_get_wpa_rsn_rsp_ie(tpAniSirGlobal pMac, uint32_t sessionId,
				       uint32_t *pLen, uint8_t *pBuf);

/* ---------------------------------------------------------------------------
    \fn csr_roam_get_num_pmkid_cache
    \brief return number of PMKID cache entries
    \return uint32_t - the number of PMKID cache entries
   -------------------------------------------------------------------------------*/
uint32_t csr_roam_get_num_pmkid_cache(tpAniSirGlobal pMac, uint32_t sessionId);

/* ---------------------------------------------------------------------------
    \fn csr_roam_get_pmkid_cache
    \brief return PMKID cache from CSR
    \param pNum - caller allocated memory that has the space of the number of pBuf tPmkidCacheInfo as input. Upon returned, *pNum has the
    needed or actually number in tPmkidCacheInfo.
    \param pPmkidCache - Caller allocated memory that contains PMKID cache, if any, upon return
    \return QDF_STATUS - when fail, it usually means the buffer allocated is not big enough
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_roam_get_pmkid_cache(tpAniSirGlobal pMac, uint32_t sessionId,
				    uint32_t *pNum, tPmkidCacheInfo *pPmkidCache);

/**
 * csr_roam_get_connect_profile() - To return the current connect profile,
 * caller must call csr_roam_free_connect_profile after it is done and before
 * reuse for another csr_roam_get_connect_profile call.
 *
 * @pMac:          pointer to global adapter context
 * @sessionId:     session ID
 * @pProfile:      pointer to a caller allocated structure
 *                 tCsrRoamConnectedProfile
 *
 * Return: QDF_STATUS. Failure if not connected, success otherwise
 */
QDF_STATUS csr_roam_get_connect_profile(tpAniSirGlobal pMac, uint32_t sessionId,
					tCsrRoamConnectedProfile *pProfile);

/* ---------------------------------------------------------------------------
    \fn csr_roam_get_connect_state
    \brief To return the current connect state of Roaming
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_roam_get_connect_state(tpAniSirGlobal pMac, uint32_t sessionId,
				      eCsrConnectState *pState);

void csr_roam_free_connect_profile(tCsrRoamConnectedProfile *profile);

/* ---------------------------------------------------------------------------
    \fn csr_apply_channel_and_power_list
    \brief HDD calls this function to set the WNI_CFG_VALID_CHANNEL_LIST base on the band/mode settings.
    This function must be called after CFG is downloaded and all the band/mode setting already passed into
    CSR.
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_apply_channel_and_power_list(tpAniSirGlobal pMac);

/**
 * csr_change_config_params() - The CSR API exposed for HDD to provide config
 * params to CSR during SMEs stop -> start sequence.
 *
 * @pMac:                 pointer to global adapter context
 * @pUpdateConfigParam:   a pointer to a structure (tCsrUpdateConfigParam) that
 * currently provides 11d related information like country code, Regulatory
 * domain, valid channel list, Tx power per channel, a list with active/passive
 * scan allowed per valid channel.
 *
 * If HDD changed the domain that will cause a reset. This function will
 * provide the new set of 11d information for the new domain. Currrently this
 * API provides info regarding 11d only at reset but we can extend this for
 * other params (PMC, QoS) which needs to be initialized again at reset.
 *
 * Return: QDF_STATUS. status of operation
 */
QDF_STATUS csr_change_config_params(tpAniSirGlobal pMac,
				    tCsrUpdateConfigParam *pUpdateConfigParam);

/* ---------------------------------------------------------------------------
    \fn csr_roam_connect_to_last_profile
    \brief To disconnect and reconnect with the same profile
    \return QDF_STATUS. It returns fail if currently connected
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_roam_connect_to_last_profile(tpAniSirGlobal pMac, uint32_t sessionId);

/* ---------------------------------------------------------------------------
    \fn csr_roam_disconnect
    \brief To disconnect from a network
    \param reason -- To indicate the reason for disconnecting. Currently, only eCSR_DISCONNECT_REASON_MIC_ERROR is meanful.
    \return QDF_STATUS
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_roam_disconnect(tpAniSirGlobal pMac, uint32_t sessionId,
			       eCsrRoamDisconnectReason reason);

/* ---------------------------------------------------------------------------
    \fn csr_scan_get_pmkid_candidate_list
    \brief return the PMKID candidate list
    \param pPmkidList - caller allocated buffer point to an array of tPmkidCandidateInfo
    \param pNumItems - pointer to a variable that has the number of tPmkidCandidateInfo allocated
    when retruning, this is either the number needed or number of items put into pPmkidList
    \return QDF_STATUS - when fail, it usually means the buffer allocated is not big enough and pNumItems
    has the number of tPmkidCandidateInfo.
    \Note: pNumItems is a number of tPmkidCandidateInfo, not sizeof(tPmkidCandidateInfo) * something
   -------------------------------------------------------------------------------*/
QDF_STATUS csr_scan_get_pmkid_candidate_list(tpAniSirGlobal pMac, uint32_t sessionId,
					     tPmkidCandidateInfo *pPmkidList,
					     uint32_t *pNumItems);

/* This function is used to stop a BSS. It is similar of csr_roamIssueDisconnect but this function */
/* doesn't have any logic other than blindly trying to stop BSS */
QDF_STATUS csr_roam_issue_stop_bss_cmd(tpAniSirGlobal pMac, uint32_t sessionId,
				       bool fHighPriority);

void csr_call_roaming_completion_callback(tpAniSirGlobal pMac,
					  tCsrRoamSession *pSession,
					  tCsrRoamInfo *pRoamInfo, uint32_t roamId,
					  eCsrRoamResult roamResult);
/**
 * csr_roam_issue_disassociate_sta_cmd() - disassociate a associated station
 * @pMac:          Pointer to global structure for MAC
 * @sessionId:     Session Id for Soft AP
 * @p_del_sta_params: Pointer to parameters of the station to disassoc
 *
 * CSR function that HDD calls to issue a deauthenticate station command
 *
 * Return: QDF_STATUS_SUCCESS on success or another QDF_STATUS_* on error
 */
QDF_STATUS csr_roam_issue_disassociate_sta_cmd(tpAniSirGlobal pMac,
					       uint32_t sessionId,
					       struct tagCsrDelStaParams
					       *p_del_sta_params);
/**
 * csr_roam_issue_deauth_sta_cmd() - issue deauthenticate station command
 * @pMac:          Pointer to global structure for MAC
 * @sessionId:     Session Id for Soft AP
 * @pDelStaParams: Pointer to parameters of the station to deauthenticate
 *
 * CSR function that HDD calls to issue a deauthenticate station command
 *
 * Return: QDF_STATUS_SUCCESS on success or another QDF_STATUS_** on error
 */
QDF_STATUS csr_roam_issue_deauth_sta_cmd(tpAniSirGlobal pMac,
		uint32_t sessionId,
		struct tagCsrDelStaParams *pDelStaParams);

/* ---------------------------------------------------------------------------
    \fn csr_roam_issue_tkip_counter_measures
    \brief csr function that HDD calls to start and stop tkip countermeasures
    \param sessionId - session Id for Soft AP
    \param bEnable   - Flag to start/stop countermeasures
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS csr_roam_issue_tkip_counter_measures(tpAniSirGlobal pMac,
						uint32_t sessionId, bool bEnable);

/* ---------------------------------------------------------------------------
    \fn csr_roam_get_associated_stas
    \brief csr function that HDD calls to get list of associated stations based on module ID
    \param sessionId - session Id for Soft AP
    \param modId - module ID - PE/HAL/TL
    \param pUsrContext - Opaque HDD context
    \param pfnSapEventCallback - Sap event callback in HDD
    \param pAssocStasBuf - Caller allocated memory to be filled with associatd stations info
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS csr_roam_get_associated_stas(tpAniSirGlobal pMac, uint32_t sessionId,
					QDF_MODULE_ID modId, void *pUsrContext,
					void *pfnSapEventCallback,
					uint8_t *pAssocStasBuf);

QDF_STATUS csr_send_mb_get_associated_stas_req_msg(tpAniSirGlobal pMac,
						   uint32_t sessionId,
						   QDF_MODULE_ID modId,
						   struct qdf_mac_addr bssId,
						   void *pUsrContext,
						   void *pfnSapEventCallback,
						   uint8_t *pAssocStasBuf);

/* ---------------------------------------------------------------------------
    \fn csr_roam_get_wps_session_overlap
    \brief csr function that HDD calls to get WPS PBC session overlap information
    \param sessionId - session Id for Soft AP
    \param pUsrContext - Opaque HDD context
    \param pfnSapEventCallback - Sap event callback in HDD
    \param pRemoveMac - pointer to MAC address of session to be removed
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS csr_roam_get_wps_session_overlap(tpAniSirGlobal pMac, uint32_t sessionId,
					    void *pUsrContext,
					    void *pfnSapEventCallback,
					    struct qdf_mac_addr pRemoveMac);

QDF_STATUS csr_send_mb_get_wpspbc_sessions(tpAniSirGlobal pMac,
					   uint32_t sessionId,
					   struct qdf_mac_addr bssId,
					   void *pUsrContext,
					   void *pfnSapEventCallback,
					   struct qdf_mac_addr pRemoveMac);

/* ---------------------------------------------------------------------------
    \fn csr_send_chng_mcc_beacon_interval
    \brief csr function that HDD calls to send Update beacon interval
    \param sessionId - session Id for Soft AP
    \return QDF_STATUS
   ---------------------------------------------------------------------------*/
QDF_STATUS
csr_send_chng_mcc_beacon_interval(tpAniSirGlobal pMac, uint32_t sessionId);

#ifdef WLAN_FEATURE_HOST_ROAM
void csr_roam_ft_pre_auth_rsp_processor(tHalHandle hHal,
		tpSirFTPreAuthRsp pFTPreAuthRsp);
void csr_release_command_preauth(tpAniSirGlobal mac_ctx, tSmeCmd *command);
#else
static inline void csr_roam_ft_pre_auth_rsp_processor(tHalHandle hHal,
		tpSirFTPreAuthRsp pFTPreAuthRsp)
{}
static inline void csr_release_command_preauth(tpAniSirGlobal mac_ctx,
		tSmeCmd *command)
{}
#endif

#if defined(FEATURE_WLAN_ESE)
void update_cckmtsf(uint32_t *timeStamp0, uint32_t *timeStamp1,
		    uint32_t *incr);
#endif

QDF_STATUS csr_roam_enqueue_preauth(tpAniSirGlobal pMac, uint32_t sessionId,
				    tpSirBssDescription pBssDescription,
				    eCsrRoamReason reason, bool fImmediate);
QDF_STATUS csr_dequeue_roam_command(tpAniSirGlobal pMac, eCsrRoamReason reason);
void csr_init_occupied_channels_list(tpAniSirGlobal pMac, uint8_t sessionId);
bool csr_neighbor_roam_is_new_connected_profile(tpAniSirGlobal pMac,
						uint8_t sessionId);
bool csr_neighbor_roam_connected_profile_match(tpAniSirGlobal pMac,
					       uint8_t sessionId,
					       tCsrScanResult *pResult,
					       tDot11fBeaconIEs *pIes);

QDF_STATUS csr_scan_create_entry_in_scan_cache(tpAniSirGlobal pMac,
						uint32_t sessionId,
						struct qdf_mac_addr bssid,
						uint8_t channel);

QDF_STATUS csr_update_channel_list(tpAniSirGlobal pMac);
QDF_STATUS csr_roam_del_pmkid_from_cache(tpAniSirGlobal pMac,
					 uint32_t sessionId,
					 const uint8_t *pBSSId,
					 bool flush_cache);

bool csr_elected_country_info(tpAniSirGlobal pMac);
void csr_add_vote_for_country_info(tpAniSirGlobal pMac, uint8_t *pCountryCode);
void csr_clear_votes_for_country_info(tpAniSirGlobal pMac);

#endif
QDF_STATUS csr_send_ext_change_channel(tpAniSirGlobal mac_ctx,
				uint32_t channel, uint8_t session_id);

#ifdef QCA_HT_2040_COEX
QDF_STATUS csr_set_ht2040_mode(tpAniSirGlobal pMac, uint32_t sessionId,
			       ePhyChanBondState cbMode, bool obssEnabled);
#endif
QDF_STATUS csr_scan_handle_search_for_ssid(tpAniSirGlobal mac,
		tSmeCmd *command);
QDF_STATUS csr_scan_handle_search_for_ssid_failure(tpAniSirGlobal mac,
		tSmeCmd *command);
tSirBssDescription*
csr_get_bssdescr_from_scan_handle(tScanResultHandle result_handle,
				  tSirBssDescription *bss_descr);
void csr_release_scan_command(tpAniSirGlobal pMac, tSmeCmd *pCommand,
			      eCsrScanStatus scanStatus);
bool is_disconnect_pending(tpAniSirGlobal mac_ctx,
				   uint8_t sessionid);
void csr_scan_active_list_timeout_handle(void *userData);
QDF_STATUS csr_prepare_disconnect_command(tpAniSirGlobal mac,
			uint32_t session_id, tSmeCmd **sme_cmd);
QDF_STATUS csr_roam_prepare_bss_config_from_profile(tpAniSirGlobal mac_ctx,
		tCsrRoamProfile *profile, tBssConfigParam *bss_cfg,
		tSirBssDescription *bss_desc);
void csr_roam_prepare_bss_params(tpAniSirGlobal mac_ctx, uint32_t session_id,
		tCsrRoamProfile *profile, tSirBssDescription *bss_desc,
		tBssConfigParam *bss_cfg, tDot11fBeaconIEs *ies);

void csr_remove_bssid_from_scan_list(tpAniSirGlobal mac_ctx,
	tSirMacAddr bssid);

QDF_STATUS csr_roam_set_bss_config_cfg(tpAniSirGlobal mac_ctx,
		uint32_t session_id,
		tCsrRoamProfile *profile, tSirBssDescription *bss_desc,
		tBssConfigParam *bss_cfg, tDot11fBeaconIEs *ies,
		bool reset_country);

