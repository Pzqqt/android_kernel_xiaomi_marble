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
 * DOC: csr_inside_api.h
 *
 * Define interface only used by CSR.
 */
#ifndef CSR_INSIDE_API_H__
#define CSR_INSIDE_API_H__

#include "csr_support.h"
#include "sme_inside.h"
#include "cds_reg_service.h"
#include "wlan_objmgr_vdev_obj.h"

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

#endif

#define CSR_MAX_NUM_SUPPORTED_CHANNELS 55

#define CSR_MAX_2_4_GHZ_SUPPORTED_CHANNELS 14

#define CSR_MAX_BSS_SUPPORT            512

/* This number minus 1 means the number of times a channel is scanned before
 * a BSS is remvoed from
 */
/* cache scan result */
#define CSR_AGING_COUNT     3
/* 5 seconds */
#define CSR_SCAN_GET_RESULT_INTERVAL    (5 * QDF_MC_TIMER_TO_SEC_UNIT)
/* 60 seconds */
#define CSR_MIC_ERROR_TIMEOUT  (60 * QDF_MC_TIMER_TO_SEC_UNIT)
/* 60 seconds */
#define CSR_TKIP_COUNTER_MEASURE_TIMEOUT  (60 * QDF_MC_TIMER_TO_SEC_UNIT)

/* the following defines are NOT used by palTimer */
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

#ifdef QCA_WIFI_3_0_EMU
#define CSR_ACTIVE_SCAN_LIST_CMD_TIMEOUT (1000*30*20)
#else
#define CSR_ACTIVE_SCAN_LIST_CMD_TIMEOUT (1000*30)
#endif
/* ***************************************************************************
 * The MAX BSSID Count should be lower than the command timeout value and it
 * can be of a fraction of 1/3 to 1/2 of the total command timeout value.
 * ***************************************************************************/
#define CSR_MAX_BSSID_COUNT     (SME_ACTIVE_LIST_CMD_TIMEOUT_VALUE/3000) - 2
#define CSR_CUSTOM_CONC_GO_BI    100
extern uint8_t csr_wpa_oui[][CSR_WPA_OUI_SIZE];
bool csr_is_supported_channel(tpAniSirGlobal pMac, uint8_t channelId);

enum csr_scancomplete_nextcommand {
	eCsrNextScanNothing,
	eCsrNexteScanForSsidSuccess,
	eCsrNexteScanForSsidFailure,
	eCsrNextCheckAllowConc,
};

enum csr_roamcomplete_result {
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
};

struct tag_scanreq_param {
	uint8_t bReturnAfter1stMatch;
	uint8_t fUniqueResult;
	uint8_t freshScan;
	uint8_t hiddenSsid;
	uint8_t reserved;
};

struct tag_csrscan_result {
	tListElem Link;
	/* This BSS is removed when it reaches 0 or less */
	int32_t AgingCount;
	/* The bigger the number, the better the BSS.
	 * This value override capValue
	 */
	uint32_t preferValue;
	/* The biggger the better. This value is in use only if we have
	 * equal preferValue
	 */
	uint32_t capValue;
	/* This member must be the last in the structure because the end of
	 * tSirBssDescription (inside) is an
	 * array with nonknown size at this time
	 */
	/* Preferred Encryption type that matched with profile. */
	eCsrEncryptionType ucEncryptionType;
	eCsrEncryptionType mcEncryptionType;
	/* Preferred auth type that matched with the profile. */
	eCsrAuthType authType;
	int  bss_score;

	tCsrScanResultInfo Result;
	/*
	 * WARNING - Do not add any element here
	 * This member Result must be the last in the structure because the end
	 * of tSirBssDescription (inside) is an array with nonknown size at
	 * this time.
	 */
};

struct scan_result_list {
	tDblLinkList List;
	tListElem *pCurEntry;
};

#define CSR_IS_ROAM_REASON(pCmd, reason) \
					((reason) == (pCmd)->roamCmd.roamReason)
#define CSR_IS_BETTER_PREFER_VALUE(v1, v2)   ((v1) > (v2))
#define CSR_IS_EQUAL_PREFER_VALUE(v1, v2)   ((v1) == (v2))
#define CSR_IS_BETTER_CAP_VALUE(v1, v2)     ((v1) > (v2))
#define CSR_IS_EQUAL_CAP_VALUE(v1, v2)  ((v1) == (v2))
#define CSR_IS_BETTER_RSSI(v1, v2)   ((v1) > (v2))
#define CSR_IS_ENC_TYPE_STATIC(encType) ((eCSR_ENCRYPT_TYPE_NONE == (encType)) \
			|| (eCSR_ENCRYPT_TYPE_WEP40_STATICKEY == (encType)) || \
			(eCSR_ENCRYPT_TYPE_WEP104_STATICKEY == (encType)))

#define CSR_IS_AUTH_TYPE_FILS(auth_type) \
		((eCSR_AUTH_TYPE_FILS_SHA256 == auth_type) || \
		(eCSR_AUTH_TYPE_FILS_SHA384 == auth_type) || \
		(eCSR_AUTH_TYPE_FT_FILS_SHA256 == auth_type) || \
		(eCSR_AUTH_TYPE_FT_FILS_SHA384 == auth_type))
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

#define CSR_IS_DISCONNECT_COMMAND(pCommand) ((eSmeCommandRoam == \
		(pCommand)->command) && \
		((eCsrForcedDisassoc == (pCommand)->u.roamCmd.roamReason) || \
		(eCsrForcedDeauth == (pCommand)->u.roamCmd.roamReason) || \
					(eCsrSmeIssuedDisassocForHandoff == \
					(pCommand)->u.roamCmd.roamReason) || \
					(eCsrForcedDisassocMICFailure == \
					(pCommand)->u.roamCmd.roamReason)))

enum csr_roam_state csr_roam_state_change(tpAniSirGlobal pMac,
					  enum csr_roam_state NewRoamState,
					  uint8_t sessionId);
void csr_roaming_state_msg_processor(tpAniSirGlobal pMac, void *pMsgBuf);
void csr_roam_joined_state_msg_processor(tpAniSirGlobal pMac, void *pMsgBuf);
void csr_scan_callback(struct wlan_objmgr_vdev *vdev,
				struct scan_event *event, void *arg);
void csr_release_command_roam(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_release_command_wm_status_change(tpAniSirGlobal pMac,
					  tSmeCmd *pCommand);
void csr_release_roc_req_cmd(tpAniSirGlobal mac_ctx,
			     tSmeCmd *pCommand);

bool csr_is_duplicate_bss_description(tpAniSirGlobal pMac,
				      tSirBssDescription *pSirBssDesc1,
				      tSirBssDescription *pSirBssDesc2,
				      tDot11fBeaconIEs *pIes2, bool fForced);
QDF_STATUS csr_roam_save_connected_bss_desc(tpAniSirGlobal pMac,
					    uint32_t sessionId,
					    tSirBssDescription *pBssDesc);
bool csr_is_network_type_equal(tSirBssDescription *pSirBssDesc1,
			       tSirBssDescription *pSirBssDesc2);
/*
 * Prepare a filter base on a profile for parsing the scan results.
 * Upon successful return, caller MUST call csr_free_scan_filter on
 * pScanFilter when it is done with the filter.
 */
QDF_STATUS csr_roam_prepare_filter_from_profile(tpAniSirGlobal pMac,
						tCsrRoamProfile *pProfile,
						tCsrScanResultFilter
						*pScanFilter);
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
bool csr_scan_append_bss_description(tpAniSirGlobal pMac,
				     tSirBssDescription *pSirBssDescription);
QDF_STATUS csr_scan_for_ssid(tpAniSirGlobal pMac, uint32_t sessionId,
			     tCsrRoamProfile *pProfile, uint32_t roamId,
			     bool notify);
/**
 * csr_scan_abort_mac_scan() - Generic API to abort scan request
 * @pMac: pointer to pmac
 * @vdev_id: pdev id
 * @scan_id: scan id
 *
 * Generic API to abort scans
 *
 * Return: 0 for success, non zero for failure
 */
QDF_STATUS csr_scan_abort_mac_scan(tpAniSirGlobal pMac, uint8_t vdev_id,
				   uint32_t scan_id);
QDF_STATUS csr_remove_nonscan_cmd_from_pending_list(tpAniSirGlobal pMac,
			uint8_t sessionId, eSmeCommandType commandType);

/* If fForce is true we will save the new String that is learn't. */
/* Typically it will be true in case of Join or user initiated ioctl */
bool csr_learn_11dcountry_information(tpAniSirGlobal pMac,
				   tSirBssDescription *pSirBssDesc,
				   tDot11fBeaconIEs *pIes, bool fForce);
void csr_apply_country_information(tpAniSirGlobal pMac);
void csr_set_cfg_scan_control_list(tpAniSirGlobal pMac, uint8_t *countryCode,
				   struct csr_channel *pChannelList);
void csr_free_scan_result_entry(tpAniSirGlobal pMac, struct tag_csrscan_result
				*pResult);

QDF_STATUS csr_roam_call_callback(tpAniSirGlobal pMac, uint32_t sessionId,
				  struct csr_roam_info *roam_info,
				  uint32_t roamId,
				  eRoamCmdStatus u1, eCsrRoamResult u2);
QDF_STATUS csr_roam_issue_connect(tpAniSirGlobal pMac, uint32_t sessionId,
				  tCsrRoamProfile *pProfile,
				  tScanResultHandle hBSSList,
				  enum csr_roam_reason reason, uint32_t roamId,
				  bool fImediate, bool fClearScan);
QDF_STATUS csr_roam_issue_reassoc(tpAniSirGlobal pMac, uint32_t sessionId,
				  tCsrRoamProfile *pProfile,
				 tCsrRoamModifyProfileFields *pModProfileFields,
				  enum csr_roam_reason reason, uint32_t roamId,
				  bool fImediate);
void csr_roam_complete(tpAniSirGlobal pMac, enum csr_roamcomplete_result Result,
		       void *Context, uint8_t session_id);
QDF_STATUS csr_roam_issue_set_context_req(tpAniSirGlobal pMac,
					uint32_t sessionId,
					  eCsrEncryptionType EncryptType,
					  tSirBssDescription *pBssDescription,
					  tSirMacAddr *bssId, bool addKey,
					  bool fUnicast,
					  tAniKeyDirection aniKeyDirection,
					  uint8_t keyId, uint16_t keyLength,
					  uint8_t *pKey, uint8_t paeRole);
QDF_STATUS csr_roam_process_disassoc_deauth(tpAniSirGlobal pMac,
						tSmeCmd *pCommand,
					    bool fDisassoc, bool fMICFailure);
QDF_STATUS csr_roam_save_connected_infomation(tpAniSirGlobal pMac,
					      uint32_t sessionId,
					      tCsrRoamProfile *pProfile,
					      tSirBssDescription *pSirBssDesc,
					      tDot11fBeaconIEs *pIes);
void csr_roam_check_for_link_status_change(tpAniSirGlobal pMac,
					tSirSmeRsp *pSirMsg);
void csr_roam_stats_rsp_processor(tpAniSirGlobal pMac, tSirSmeRsp *pSirMsg);
QDF_STATUS csr_roam_issue_start_bss(tpAniSirGlobal pMac, uint32_t sessionId,
				    struct csr_roamstart_bssparams *pParam,
				    tCsrRoamProfile *pProfile,
				    tSirBssDescription *pBssDesc,
					uint32_t roamId);
QDF_STATUS csr_roam_issue_stop_bss(tpAniSirGlobal pMac, uint32_t sessionId,
				   enum csr_roam_substate NewSubstate);
bool csr_is_same_profile(tpAniSirGlobal pMac, tCsrRoamConnectedProfile
			*pProfile1, tCsrRoamProfile *pProfile2);
bool csr_is_roam_command_waiting(tpAniSirGlobal pMac);
bool csr_is_roam_command_waiting_for_session(tpAniSirGlobal pMac,
					uint32_t sessionId);
eRoamCmdStatus csr_get_roam_complete_status(tpAniSirGlobal pMac,
					    uint32_t sessionId);
/* pBand can be NULL if caller doesn't need to get it */
QDF_STATUS csr_roam_issue_disassociate_cmd(tpAniSirGlobal pMac,
					uint32_t sessionId,
					   eCsrRoamDisconnectReason reason);
QDF_STATUS csr_roam_disconnect_internal(tpAniSirGlobal pMac, uint32_t sessionId,
					eCsrRoamDisconnectReason reason);
/* pCommand may be NULL */
void csr_roam_remove_duplicate_command(tpAniSirGlobal pMac, uint32_t sessionId,
				       tSmeCmd *pCommand,
				       enum csr_roam_reason eRoamReason);

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
QDF_STATUS csr_send_assoc_cnf_msg(tpAniSirGlobal pMac, tpSirSmeAssocInd
				pAssocInd,
				  QDF_STATUS status);
QDF_STATUS csr_send_assoc_ind_to_upper_layer_cnf_msg(tpAniSirGlobal pMac,
						     tpSirSmeAssocInd pAssocInd,
						     QDF_STATUS Halstatus,
						     uint8_t sessionId);
QDF_STATUS csr_send_mb_start_bss_req_msg(tpAniSirGlobal pMac,
					uint32_t sessionId,
					 eCsrRoamBssType bssType,
					 struct csr_roamstart_bssparams *pParam,
					 tSirBssDescription *pBssDesc);
QDF_STATUS csr_send_mb_stop_bss_req_msg(tpAniSirGlobal pMac,
					uint32_t sessionId);

/* Caller should put the BSS' ssid to fiedl bssSsid when
 * comparing SSID for a BSS.
 */
bool csr_is_ssid_match(tpAniSirGlobal pMac, uint8_t *ssid1, uint8_t ssid1Len,
		       uint8_t *bssSsid, uint8_t bssSsidLen,
			bool fSsidRequired);
bool csr_is_phy_mode_match(tpAniSirGlobal pMac, uint32_t phyMode,
			   tSirBssDescription *pSirBssDesc,
			   tCsrRoamProfile *pProfile,
			   enum csr_cfgdot11mode *pReturnCfgDot11Mode,
			   tDot11fBeaconIEs *pIes);
bool csr_roam_is_channel_valid(tpAniSirGlobal pMac, uint8_t channel);

/* pNumChan is a caller allocated space with the sizeof pChannels */
QDF_STATUS csr_get_cfg_valid_channels(tpAniSirGlobal pMac, uint8_t *pChannels,
				      uint32_t *pNumChan);
void csr_roam_ccm_cfg_set_callback(tpAniSirGlobal pMac, int32_t result,
					uint8_t session_id);

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
void csr_free_scan_filter(tpAniSirGlobal pMac, tCsrScanResultFilter
			*pScanFilter);
enum csr_cfgdot11mode csr_get_cfg_dot11_mode_from_csr_phy_mode(tCsrRoamProfile
							    *pProfile,
							    eCsrPhyMode phyMode,
							    bool fProprietary);
uint32_t csr_translate_to_wni_cfg_dot11_mode(tpAniSirGlobal pMac,
				    enum csr_cfgdot11mode csrDot11Mode);
void csr_save_channel_power_for_band(tpAniSirGlobal pMac, bool fPopulate5GBand);
void csr_apply_channel_power_info_to_fw(tpAniSirGlobal pMac,
					struct csr_channel *pChannelList,
					uint8_t *countryCode);
void csr_apply_power2_current(tpAniSirGlobal pMac);
void csr_assign_rssi_for_category(tpAniSirGlobal pMac, int8_t bestApRssi,
				  uint8_t catOffset);
QDF_STATUS csr_roam_start_roaming(tpAniSirGlobal pMac, uint32_t sessionId,
				  enum csr_roaming_reason roamingReason);
/* return a bool to indicate whether roaming completed or continue. */
bool csr_roam_complete_roaming(tpAniSirGlobal pMac, uint32_t sessionId,
			       bool fForce, eCsrRoamResult roamResult);
void csr_roam_completion(tpAniSirGlobal pMac, uint32_t sessionId,
			 struct csr_roam_info *roam_info, tSmeCmd *pCommand,
			 eCsrRoamResult roamResult, bool fSuccess);
void csr_roam_cancel_roaming(tpAniSirGlobal pMac, uint32_t sessionId);
void csr_apply_channel_power_info_wrapper(tpAniSirGlobal pMac);
void csr_reset_pmkid_candidate_list(tpAniSirGlobal pMac, uint32_t sessionId);
#ifdef FEATURE_WLAN_WAPI
void csr_reset_bkid_candidate_list(tpAniSirGlobal pMac, uint32_t sessionId);
#endif /* FEATURE_WLAN_WAPI */
QDF_STATUS csr_save_to_channel_power2_g_5_g(tpAniSirGlobal pMac,
					uint32_t tableSize, tSirMacChanInfo
					*channelTable);
QDF_STATUS csr_roam_set_key(tpAniSirGlobal pMac, uint32_t sessionId,
			    tCsrRoamSetKey *pSetKey, uint32_t roamId);
QDF_STATUS csr_roam_open_session(tpAniSirGlobal pMac,
				 struct sme_session_params *session_param);
QDF_STATUS csr_roam_close_session(tpAniSirGlobal mac_ctx,
				  uint32_t session_id, bool sync);
void csr_cleanup_session(tpAniSirGlobal pMac, uint32_t sessionId);
QDF_STATUS csr_roam_get_session_id_from_bssid(tpAniSirGlobal pMac,
						struct qdf_mac_addr *bssid,
					      uint32_t *pSessionId);
enum csr_cfgdot11mode csr_find_best_phy_mode(tpAniSirGlobal pMac,
							uint32_t phyMode);

/*
 * csr_scan_get_result() -
 * Return scan results.
 *
 * pFilter - If pFilter is NULL, all cached results are returned
 * phResult - an object for the result.
 * Return QDF_STATUS
 */
QDF_STATUS csr_scan_get_result(tpAniSirGlobal pMac, tCsrScanResultFilter
				*pFilter, tScanResultHandle *phResult);

/**
 * csr_scan_get_result_for_bssid - gets the scan result from scan cache for the
 *      bssid specified
 * @mac_ctx: mac context
 * @bssid: bssid to get the scan result for
 * @res: pointer to tCsrScanResultInfo
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_scan_get_result_for_bssid(tpAniSirGlobal mac_ctx,
					 struct qdf_mac_addr *bssid,
					 tCsrScanResultInfo *res);

/*
 * csr_scan_flush_result() -
 * Clear scan results.
 *
 * pMac - pMac global pointer
 * sessionId - Session Identifier
 * Return QDF_STATUS
 */
QDF_STATUS csr_scan_flush_result(tpAniSirGlobal mac_ctx);
/*
 * csr_scan_filter_results() -
 *  Filter scan results based on valid channel list.
 *
 * pMac - Pointer to Global MAC structure
 * Return QDF_STATUS
 */
QDF_STATUS csr_scan_filter_results(tpAniSirGlobal pMac);

QDF_STATUS csr_scan_flush_selective_result(tpAniSirGlobal pMac, bool flushP2P);

/*
 * csr_scan_result_get_first
 * Returns the first element of scan result.
 *
 * hScanResult - returned from csr_scan_get_result
 * tCsrScanResultInfo * - NULL if no result
 */
tCsrScanResultInfo *csr_scan_result_get_first(tpAniSirGlobal pMac,
					      tScanResultHandle hScanResult);
/*
 * csr_scan_result_get_next
 * Returns the next element of scan result. It can be called without calling
 * csr_scan_result_get_first first
 *
 * hScanResult - returned from csr_scan_get_result
 * Return Null if no result or reach the end
 */
tCsrScanResultInfo *csr_scan_result_get_next(tpAniSirGlobal pMac,
					     tScanResultHandle hScanResult);

/*
 * csr_get_country_code() -
 * This function is to get the country code current being used
 * pBuf - Caller allocated buffer with at least 3 bytes, upon success return,
 * this has the country code
 * pbLen - Caller allocated, as input, it indicates the length of pBuf. Upon
 * success return, this contains the length of the data in pBuf
 * Return QDF_STATUS
 */
QDF_STATUS csr_get_country_code(tpAniSirGlobal pMac, uint8_t *pBuf,
				uint8_t *pbLen);

/*
 * csr_set_country_code() -
 * This function is to set the country code so channel/power setting matches
 * the countrycode and the domain it belongs to.
 *
 * pCountry - Caller allocated buffer with at least 3 bytes specifying the
 * country code
 * Return QDF_STATUS
 */
QDF_STATUS csr_set_country_code(tpAniSirGlobal pMac, uint8_t *pCountry);

/*
 * csr_get_regulatory_domain_for_country() -
 * This function is to get the regulatory domain for a country.
 * This function must be called after CFG is downloaded and all the band/mode
 * setting already passed into CSR.
 *
 * pCountry - Caller allocated buffer with at least 3 bytes specifying the
 * country code
 * pDomainId - Caller allocated buffer to get the return domain ID upon
 * success return. Can be NULL.
 * source - the source of country information.
 * Return QDF_STATUS
 */
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
void csr_release_command_buffer(tpAniSirGlobal pMac, tSmeCmd *pCommand);
void csr_scan_flush_bss_entry(tpAniSirGlobal pMac,
			      tpSmeCsaOffloadInd pCsaOffloadInd);

#ifdef FEATURE_WLAN_WAPI
bool csr_is_profile_wapi(tCsrRoamProfile *pProfile);
#endif /* FEATURE_WLAN_WAPI */

void csr_get_vdev_type_nss(tpAniSirGlobal mac_ctx,
		enum QDF_OPMODE dev_mode,
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
#define ENC_MODE_AES_GCMP    5
#define ENC_MODE_AES_GCMP_256    6
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
/*
 * csr_scan_result_purge() -
 * Remove all items(tCsrScanResult) in the list and free memory for each item
 * hScanResult - returned from csr_scan_get_result. hScanResult is considered
 * gone by calling this function and even before this function reutrns.
 * Return QDF_STATUS
 */
QDF_STATUS csr_scan_result_purge(tpAniSirGlobal pMac,
				 tScanResultHandle hScanResult);

/* /////////////////////////////////////////Common Scan ends */

/*
 * csr_roam_connect() -
 * To inititiate an association
 * pProfile - can be NULL to join to any open ones
 * pRoamId - to get back the request ID
 * Return QDF_STATUS
 */
QDF_STATUS csr_roam_connect(tpAniSirGlobal pMac, uint32_t sessionId,
			    tCsrRoamProfile *pProfile,
			    uint32_t *pRoamId);

/*
 * csr_roam_reassoc() -
 * To inititiate a re-association
 * pProfile - can be NULL to join the currently connected AP. In that
 * case modProfileFields should carry the modified field(s) which could trigger
 * reassoc
 * modProfileFields - fields which are part of tCsrRoamConnectedProfile
 * that might need modification dynamically once STA is up & running and this
 * could trigger a reassoc
 * pRoamId - to get back the request ID
 * Return QDF_STATUS
 */
QDF_STATUS csr_roam_reassoc(tpAniSirGlobal pMac, uint32_t sessionId,
			    tCsrRoamProfile *pProfile,
			    tCsrRoamModifyProfileFields modProfileFields,
			    uint32_t *pRoamId);

/*
 * csr_roam_reconnect() -
 * To disconnect and reconnect with the same profile
 *
 * Return QDF_STATUS. It returns fail if currently not connected
 */
QDF_STATUS csr_roam_reconnect(tpAniSirGlobal pMac, uint32_t sessionId);

/*
 * csr_roam_set_pmkid_cache() -
 * return the PMKID candidate list
 *
 * pPMKIDCache - caller allocated buffer point to an array of tPmkidCacheInfo
 * numItems - a variable that has the number of tPmkidCacheInfo allocated
 * when retruning, this is either the number needed or number of items put
 * into pPMKIDCache
 * Return QDF_STATUS - when fail, it usually means the buffer allocated is not
 * big enough and pNumItems has the number of tPmkidCacheInfo.
 * \Note: pNumItems is a number of tPmkidCacheInfo, not
 * sizeof(tPmkidCacheInfo) * something
 */
QDF_STATUS csr_roam_set_pmkid_cache(tpAniSirGlobal pMac, uint32_t sessionId,
				    tPmkidCacheInfo *pPMKIDCache,
				   uint32_t numItems, bool update_entire_cache);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/*
 * csr_roam_set_psk_pmk() -
 * store PSK/PMK
 * pMac  - pointer to global structure for MAC
 * sessionId - Sme session id
 * pPSK_PMK - pointer to an array of Psk/Pmk
 * Return QDF_STATUS - usually it succeed unless sessionId is not found
 * Note:
 */
QDF_STATUS csr_roam_set_psk_pmk(tpAniSirGlobal pMac, uint32_t sessionId,
				uint8_t *pPSK_PMK, size_t pmk_len);

QDF_STATUS csr_roam_set_key_mgmt_offload(tpAniSirGlobal mac_ctx,
					 uint32_t session_id,
					 bool roam_key_mgmt_offload_enabled,
					 struct pmkid_mode_bits *pmkid_modes);
#endif
/*
 * csr_roam_get_wpa_rsn_req_ie() -
 * Return the WPA or RSN IE CSR passes to PE to JOIN request or START_BSS
 * request
 * pLen - caller allocated memory that has the length of pBuf as input.
 * Upon returned, *pLen has the needed or IE length in pBuf.
 * pBuf - Caller allocated memory that contain the IE field, if any, upon return
 * Return QDF_STATUS - when fail, it usually means the buffer allocated is not
 * big enough
 */
QDF_STATUS csr_roam_get_wpa_rsn_req_ie(tpAniSirGlobal pMac, uint32_t sessionId,
				       uint32_t *pLen, uint8_t *pBuf);

/*
 * csr_roam_get_wpa_rsn_rsp_ie() -
 * Return the WPA or RSN IE from the beacon or probe rsp if connected
 *
 * pLen - caller allocated memory that has the length of pBuf as input.
 * Upon returned, *pLen has the needed or IE length in pBuf.
 * pBuf - Caller allocated memory that contain the IE field, if any, upon return
 * Return QDF_STATUS - when fail, it usually means the buffer allocated is not
 * big enough
 */
QDF_STATUS csr_roam_get_wpa_rsn_rsp_ie(tpAniSirGlobal pMac, uint32_t sessionId,
				       uint32_t *pLen, uint8_t *pBuf);

/*
 * csr_roam_get_num_pmkid_cache() -
 * Return number of PMKID cache entries
 *
 * Return uint32_t - the number of PMKID cache entries
 */
uint32_t csr_roam_get_num_pmkid_cache(tpAniSirGlobal pMac, uint32_t sessionId);

/*
 * csr_roam_get_pmkid_cache() -
 * Return PMKID cache from CSR
 *
 * pNum - caller allocated memory that has the space of the number of pBuf
 * tPmkidCacheInfo as input. Upon returned, *pNum has the needed or actually
 * number in tPmkidCacheInfo.
 * pPmkidCache - Caller allocated memory that contains PMKID cache, if any,
 * upon return
 * Return QDF_STATUS - when fail, it usually means the buffer allocated is
 * not big enough
 */
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

/*
 * csr_roam_get_connect_state()
 *  To return the current connect state of Roaming
 *
 * Return QDF_STATUS
 */
QDF_STATUS csr_roam_get_connect_state(tpAniSirGlobal pMac, uint32_t sessionId,
				      eCsrConnectState *pState);

void csr_roam_free_connect_profile(tCsrRoamConnectedProfile *profile);

/*
 * csr_apply_channel_and_power_list() -
 *  HDD calls this function to set the WNI_CFG_VALID_CHANNEL_LIST base on the
 * band/mode settings. This function must be called after CFG is downloaded
 * and all the band/mode setting already passed into CSR.

 * Return QDF_STATUS
 */
QDF_STATUS csr_apply_channel_and_power_list(tpAniSirGlobal pMac);

/*
 * csr_roam_connect_to_last_profile() -
 * To disconnect and reconnect with the same profile
 *
 * Return QDF_STATUS. It returns fail if currently connected
 */
QDF_STATUS csr_roam_connect_to_last_profile(tpAniSirGlobal pMac,
					uint32_t sessionId);

/*
 * csr_roam_disconnect() -
 *  To disconnect from a network
 *
 * Reason -- To indicate the reason for disconnecting. Currently, only
 * eCSR_DISCONNECT_REASON_MIC_ERROR is meanful.
 * Return QDF_STATUS
 */
QDF_STATUS csr_roam_disconnect(tpAniSirGlobal pMac, uint32_t sessionId,
			       eCsrRoamDisconnectReason reason);

/*
 * csr_scan_get_pmkid_candidate_list() -
 *  Return the PMKID candidate list
 *
 * pPmkidList - caller allocated buffer point to an array of tPmkidCandidateInfo
 * pNumItems - pointer to a variable that has the number of tPmkidCandidateInfo
 * allocated when retruning, this is either the number needed or number of
 * itemsput into pPmkidList
 * Return QDF_STATUS - when fail, it usually means the buffer allocated is not
 * big enough and pNumItems has the number of tPmkidCandidateInfo.
 * Note: pNumItems is a number of tPmkidCandidateInfo, not
 * sizeof(tPmkidCandidateInfo) * something
 */
QDF_STATUS csr_scan_get_pmkid_candidate_list(tpAniSirGlobal pMac,
						uint32_t sessionId,
					     tPmkidCandidateInfo *pPmkidList,
					     uint32_t *pNumItems);

/* This function is used to stop a BSS. It is similar of csr_roamIssueDisconnect
 * but this function doesn't have any logic other than blindly trying to stop
 * BSS
 */
QDF_STATUS csr_roam_issue_stop_bss_cmd(tpAniSirGlobal pMac, uint32_t sessionId,
				       bool fHighPriority);

void csr_call_roaming_completion_callback(tpAniSirGlobal pMac,
					  struct csr_roam_session *pSession,
					  struct csr_roam_info *roam_info,
					  uint32_t roamId,
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
					       struct csr_del_sta_params
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
		struct csr_del_sta_params *pDelStaParams);

/*
 * csr_roam_get_associated_stas
 *  csr function that HDD calls to get list of associated stations based on
 * module ID
 * sessionId - session Id for Soft AP
 * modId - module ID - PE/HAL/TL
 * pUsrContext - Opaque HDD context
 * pfnSapEventCallback - Sap event callback in HDD
 * pAssocStasBuf - Caller allocated memory to be filled with associatd
 * stations info
 * Return QDF_STATUS
 */
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

/*
 * csr_send_chng_mcc_beacon_interval() -
 *   csr function that HDD calls to send Update beacon interval
 *
 * sessionId - session Id for Soft AP
 * Return QDF_STATUS
 */
QDF_STATUS
csr_send_chng_mcc_beacon_interval(tpAniSirGlobal pMac, uint32_t sessionId);

#ifdef WLAN_FEATURE_HOST_ROAM
void csr_roam_ft_pre_auth_rsp_processor(tHalHandle hHal,
		tpSirFTPreAuthRsp pFTPreAuthRsp);
#else
static inline void csr_roam_ft_pre_auth_rsp_processor(tHalHandle hHal,
		tpSirFTPreAuthRsp pFTPreAuthRsp)
{}
#endif

#if defined(FEATURE_WLAN_ESE)
void update_cckmtsf(uint32_t *timeStamp0, uint32_t *timeStamp1,
		    uint64_t *incr);
#endif

QDF_STATUS csr_roam_enqueue_preauth(tpAniSirGlobal pMac, uint32_t sessionId,
			    tpSirBssDescription pBssDescription,
			    enum csr_roam_reason reason, bool fImmediate);
QDF_STATUS csr_dequeue_roam_command(tpAniSirGlobal pMac,
				enum csr_roam_reason reason,
				uint8_t session_id);
void csr_init_occupied_channels_list(tpAniSirGlobal pMac, uint8_t sessionId);
bool csr_neighbor_roam_is_new_connected_profile(tpAniSirGlobal pMac,
						uint8_t sessionId);
bool csr_neighbor_roam_connected_profile_match(tpAniSirGlobal pMac,
					       uint8_t sessionId,
					       struct tag_csrscan_result
						*pResult,
					       tDot11fBeaconIEs *pIes);

QDF_STATUS csr_scan_create_entry_in_scan_cache(tpAniSirGlobal pMac,
						uint32_t sessionId,
						struct qdf_mac_addr bssid,
						uint8_t channel);

QDF_STATUS csr_update_channel_list(tpAniSirGlobal pMac);
QDF_STATUS csr_roam_del_pmkid_from_cache(tpAniSirGlobal pMac,
					 uint32_t sessionId,
					 tPmkidCacheInfo *pmksa,
					 bool flush_cache);

bool csr_elected_country_info(tpAniSirGlobal pMac);
void csr_add_vote_for_country_info(tpAniSirGlobal pMac, uint8_t *pCountryCode);
void csr_clear_votes_for_country_info(tpAniSirGlobal pMac);

QDF_STATUS csr_send_ext_change_channel(tpAniSirGlobal mac_ctx,
				uint32_t channel, uint8_t session_id);

#ifdef QCA_HT_2040_COEX
QDF_STATUS csr_set_ht2040_mode(tpAniSirGlobal pMac, uint32_t sessionId,
			       ePhyChanBondState cbMode, bool obssEnabled);
#endif
QDF_STATUS csr_scan_handle_search_for_ssid(tpAniSirGlobal mac_ctx,
					   uint32_t session_id);
QDF_STATUS csr_scan_handle_search_for_ssid_failure(tpAniSirGlobal mac,
		uint32_t session_id);
void csr_saved_scan_cmd_free_fields(tpAniSirGlobal mac_ctx,
				    struct csr_roam_session *session);
tpSirBssDescription csr_get_fst_bssdescr_ptr(tScanResultHandle result_handle);

tSirBssDescription*
csr_get_bssdescr_from_scan_handle(tScanResultHandle result_handle,
				  tSirBssDescription *bss_descr);
bool is_disconnect_pending(tpAniSirGlobal mac_ctx,
				   uint8_t sessionid);
void csr_scan_active_list_timeout_handle(void *userData);
QDF_STATUS csr_prepare_disconnect_command(tpAniSirGlobal mac,
			uint32_t session_id, tSmeCmd **sme_cmd);
QDF_STATUS csr_roam_prepare_bss_config_from_profile(tpAniSirGlobal mac_ctx,
		tCsrRoamProfile *profile, struct bss_config_param *bss_cfg,
		tSirBssDescription *bss_desc);
void csr_roam_prepare_bss_params(tpAniSirGlobal mac_ctx, uint32_t session_id,
		tCsrRoamProfile *profile, tSirBssDescription *bss_desc,
		struct bss_config_param *bss_cfg, tDot11fBeaconIEs *ies);

/**
 * csr_remove_bssid_from_scan_list() - remove the bssid from
 * scan list
 * @mac_tx: mac context.
 * @bssid: bssid to be removed
 *
 * This function remove the given bssid from scan list.
 *
 * Return: void.
 */
void csr_remove_bssid_from_scan_list(tpAniSirGlobal mac_ctx,
	tSirMacAddr bssid);

QDF_STATUS csr_roam_set_bss_config_cfg(tpAniSirGlobal mac_ctx,
		uint32_t session_id,
		tCsrRoamProfile *profile, tSirBssDescription *bss_desc,
		struct bss_config_param *bss_cfg, tDot11fBeaconIEs *ies,
		bool reset_country);
void csr_prune_channel_list_for_mode(tpAniSirGlobal pMac,
				     struct csr_channel *pChannelList);

#ifdef WLAN_FEATURE_11W
bool csr_is_mfpc_capable(struct sDot11fIERSN *rsn);
#else
static inline bool csr_is_mfpc_capable(struct sDot11fIERSN *rsn)
{
	return false;
}
#endif

/**
 * csr_get_rf_band()
 *
 * @channel: channel number
 *
 * This function is used to translate channel number to band
 *
 * Return: BAND_2G -  if 2.4GHZ channel
 *         BAND_5G -  if 5GHZ channel
 */
enum band_info csr_get_rf_band(uint8_t channel);

/**
 * csr_lookup_pmkid_using_bssid() - lookup pmkid using bssid
 * @mac: pointer to mac
 * @session: sme session pointer
 * @pmk_cache: pointer to pmk cache
 * @index: index value needs to be seached
 *
 * Return: true if pmkid is found else false
 */
bool csr_lookup_pmkid_using_bssid(tpAniSirGlobal mac,
					struct csr_roam_session *session,
					tPmkidCacheInfo *pmk_cache,
					uint32_t *index);
#ifdef WLAN_FEATURE_11AX
void csr_update_session_he_cap(tpAniSirGlobal mac_ctx,
			struct csr_roam_session *session);
#else
static inline void csr_update_session_he_cap(tpAniSirGlobal mac_ctx,
			struct csr_roam_session *session)
{
}
#endif

#endif /* CSR_INSIDE_API_H__ */
