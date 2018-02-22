/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

#if !defined(__SME_API_H)
#define __SME_API_H

/**
 * file  smeApi.h
 *
 * brief prototype for SME APIs
 */

/*--------------------------------------------------------------------------
  Include Files
  ------------------------------------------------------------------------*/
#include "csr_api.h"
#include "qdf_lock.h"
#include "qdf_types.h"
#include "sir_api.h"
#include "cds_regdomain.h"
#include "sme_internal.h"
#include "wma_tgt_cfg.h"
#include "wma_fips_public_structs.h"
#include "wma_sar_public_structs.h"

#include "sme_rrm_internal.h"
#include "sir_types.h"
#include "scheduler_api.h"
#include "wlan_serialization_legacy_api.h"

/*--------------------------------------------------------------------------
  Preprocessor definitions and constants
  ------------------------------------------------------------------------*/

#define SME_SUMMARY_STATS         (1 << eCsrSummaryStats)
#define SME_GLOBAL_CLASSA_STATS   (1 << eCsrGlobalClassAStats)
#define SME_GLOBAL_CLASSD_STATS   (1 << eCsrGlobalClassDStats)
#define SME_PER_CHAIN_RSSI_STATS  (1 << csr_per_chain_rssi_stats)

#define sme_log(level, args...) QDF_TRACE(QDF_MODULE_ID_SME, level, ## args)
#define sme_logfl(level, format, args...) sme_log(level, FL(format), ## args)

#define sme_alert(format, args...) \
		sme_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define sme_err(format, args...) \
		sme_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define sme_warn(format, args...) \
		sme_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define sme_info(format, args...) \
		sme_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define sme_debug(format, args...) \
		sme_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

#define SME_ENTER() sme_logfl(QDF_TRACE_LEVEL_DEBUG, "enter")
#define SME_EXIT() sme_logfl(QDF_TRACE_LEVEL_DEBUG, "exit")

#define SME_SESSION_ID_ANY        50
#define SME_SESSION_ID_BROADCAST  0xFF

#define SME_INVALID_COUNTRY_CODE "XX"
#define INVALID_ROAM_ID 0

#define SME_SET_CHANNEL_REG_POWER(reg_info_1, val) do {	\
	reg_info_1 &= 0xff00ffff;	      \
	reg_info_1 |= ((val & 0xff) << 16);   \
} while (0)

#define SME_SET_CHANNEL_MAX_TX_POWER(reg_info_2, val) do { \
	reg_info_2 &= 0xffff00ff;	      \
	reg_info_2 |= ((val & 0xff) << 8);   \
} while (0)

#define SME_CONFIG_TO_ROAM_CONFIG 1
#define ROAM_CONFIG_TO_SME_CONFIG 2

#define NUM_OF_BANDS 2

#define SME_ACTIVE_LIST_CMD_TIMEOUT_VALUE (30*1000)
#define SME_CMD_TIMEOUT_VALUE (SME_ACTIVE_LIST_CMD_TIMEOUT_VALUE + 1000)
/*--------------------------------------------------------------------------
  Type declarations
  ------------------------------------------------------------------------*/
typedef void (*hdd_ftm_msg_processor)(void *);
typedef struct _smeConfigParams {
	tCsrConfigParam csrConfig;
	struct rrm_config_param rrmConfig;
	bool snr_monitor_enabled;
} tSmeConfigParams, *tpSmeConfigParams;

#ifdef FEATURE_WLAN_TDLS
#define SME_TDLS_MAX_SUPP_CHANNELS       128
#define SME_TDLS_MAX_SUPP_OPER_CLASSES   32

typedef struct _smeTdlsPeerCapParams {
	uint8_t isPeerResponder;
	uint8_t peerUapsdQueue;
	uint8_t peerMaxSp;
	uint8_t peerBuffStaSupport;
	uint8_t peerOffChanSupport;
	uint8_t peerCurrOperClass;
	uint8_t selfCurrOperClass;
	uint8_t peerChanLen;
	uint8_t peerChan[SME_TDLS_MAX_SUPP_CHANNELS];
	uint8_t peerOperClassLen;
	uint8_t peerOperClass[SME_TDLS_MAX_SUPP_OPER_CLASSES];
	uint8_t prefOffChanNum;
	uint8_t prefOffChanBandwidth;
	uint8_t opClassForPrefOffChan;
} tSmeTdlsPeerCapParams;

/**
 * eSmeTdlsPeerState - tdls peer state
 * @eSME_TDLS_PEER_STATE_PEERING: tdls connection in progress
 * @eSME_TDLS_PEER_STATE_CONNECTED: tdls peer is connected
 * @eSME_TDLS_PEER_STATE_TEARDOWN: tdls peer is tear down
 * @eSME_TDLS_PEER_ADD_MAC_ADDR: add peer mac into connection table
 * @eSME_TDLS_PEER_REMOVE_MAC_ADDR: remove peer mac from connection table
 */
typedef enum {
	eSME_TDLS_PEER_STATE_PEERING,
	eSME_TDLS_PEER_STATE_CONNECTED,
	eSME_TDLS_PEER_STATE_TEARDOWN,
	eSME_TDLS_PEER_ADD_MAC_ADDR,
	eSME_TDLS_PEER_REMOVE_MAC_ADDR,
} eSmeTdlsPeerState;

typedef struct _smeTdlsPeerStateParams {
	uint32_t vdevId;
	tSirMacAddr peerMacAddr;
	uint32_t peerState;
	tSmeTdlsPeerCapParams peerCap;
} tSmeTdlsPeerStateParams;

#define ENABLE_CHANSWITCH  1
#define DISABLE_CHANSWITCH 2
#define BW_20_OFFSET_BIT   0
#define BW_40_OFFSET_BIT   1
#define BW_80_OFFSET_BIT   2
#define BW_160_OFFSET_BIT  3
typedef struct sme_tdls_chan_switch_param_struct {
	uint32_t vdev_id;
	tSirMacAddr peer_mac_addr;
	uint16_t tdls_off_ch_bw_offset;/* Target Off Channel Bandwidth offset */
	uint8_t tdls_off_channel;      /* Target Off Channel */
	uint8_t tdls_off_ch_mode;      /* TDLS Off Channel Mode */
	uint8_t is_responder;          /* is peer responder or initiator */
	uint8_t opclass;           /* tdls operating class */
} sme_tdls_chan_switch_params;
#endif /* FEATURE_WLAN_TDLS */

/* Thermal Mitigation*/
typedef struct {
	uint16_t smeMinTempThreshold;
	uint16_t smeMaxTempThreshold;
} tSmeThermalLevelInfo;

#define SME_MAX_THERMAL_LEVELS (4)
#define SME_MAX_THROTTLE_LEVELS (4)

typedef struct {
	/* Array of thermal levels */
	tSmeThermalLevelInfo smeThermalLevels[SME_MAX_THERMAL_LEVELS];
	uint8_t smeThermalMgmtEnabled;
	uint32_t smeThrottlePeriod;
	uint8_t sme_throttle_duty_cycle_tbl[SME_MAX_THROTTLE_LEVELS];
} tSmeThermalParams;

typedef enum {
	SME_AC_BK = 0,
	SME_AC_BE = 1,
	SME_AC_VI = 2,
	SME_AC_VO = 3
} sme_ac_enum_type;

/*
 * Enumeration of the various TSPEC directions
 * From 802.11e/WMM specifications
 */
enum sme_qos_wmm_dir_type {
	SME_QOS_WMM_TS_DIR_UPLINK = 0,
	SME_QOS_WMM_TS_DIR_DOWNLINK = 1,
	SME_QOS_WMM_TS_DIR_RESV = 2,    /* Reserved */
	SME_QOS_WMM_TS_DIR_BOTH = 3,
};

/**
 * struct sme_oem_capability - OEM capability to be exchanged between host
 *                             and userspace
 * @ftm_rr: FTM range report capability bit
 * @lci_capability: LCI capability bit
 * @reserved1: reserved
 * @reserved2: reserved
 */
struct sme_oem_capability {
	uint32_t ftm_rr:1;
	uint32_t lci_capability:1;
	uint32_t reserved1:30;
	uint32_t reserved2;
};

/**
 * struct sme_5g_pref_params : 5G preference params to be read from ini
 * @rssi_boost_threshold_5g: RSSI threshold above which 5 GHz is favored
 * @rssi_boost_factor_5g: Factor by which 5GHz RSSI is boosted
 * @max_rssi_boost_5g: Maximum boost that can be applied to 5GHz RSSI
 * @rssi_penalize_threshold_5g: RSSI threshold below which 5G is not favored
 * @rssi_penalize_factor_5g: Factor by which 5GHz RSSI is penalized
 * @max_rssi_penalize_5g: Maximum penalty that can be applied to 5G RSSI
 */
struct sme_5g_band_pref_params {
	int8_t      rssi_boost_threshold_5g;
	uint8_t     rssi_boost_factor_5g;
	uint8_t     max_rssi_boost_5g;
	int8_t      rssi_penalize_threshold_5g;
	uint8_t     rssi_penalize_factor_5g;
	uint8_t     max_rssi_penalize_5g;
};

/**
 * struct sme_session_params: Session creation params passed by HDD layer
 * @session_open_cb: callback to be registered with SME for opening the session
 * @session_close_cb: callback to be registered with SME for closing the session
 * @self_mac_addr: Self mac address
 * @sme_session_id: SME session id
 * @type_of_persona: person type
 * @subtype_of_persona: sub type of persona
 */
struct sme_session_params {
	csr_session_open_cb  session_open_cb;
	csr_session_close_cb session_close_cb;
	csr_roam_completeCallback callback;
	void *callback_ctx;
	uint8_t *self_mac_addr;
	uint8_t sme_session_id;
	uint32_t type_of_persona;
	uint32_t subtype_of_persona;
};

#define MAX_CANDIDATE_INFO 10

/**
 * struct bss_candidate_info - Candidate bss information
 *
 * @bssid : BSSID of candidate bss
 * @status : status code for candidate bss
 */
struct bss_candidate_info {
	struct qdf_mac_addr bssid;
	uint32_t status;
};

/*
 * MBO transition reason codes
 */
enum {
	MBO_TRANSITION_REASON_UNSPECIFIED,
	MBO_TRANSITION_REASON_EXCESSIVE_FRAME_LOSS_RATE,
	MBO_TRANSITION_REASON_EXCESSIVE_DELAY_FOR_CURRENT_TRAFFIC,
	MBO_TRANSITION_REASON_INSUFFICIENT_BANDWIDTH_FOR_CURRENT_TRAFFIC,
	MBO_TRANSITION_REASON_LOAD_BALANCING,
	MBO_TRANSITION_REASON_LOW_RSSI,
	MBO_TRANSITION_REASON_RECEIVED_EXCESSIVE_RETRANSMISSIONS,
	MBO_TRANSITION_REASON_HIGH_INTERFERENCE,
	MBO_TRANSITION_REASON_GRAY_ZONE,
	MBO_TRANSITION_REASON_TRANSITIONING_TO_PREMIUM_AP,
};

/*-------------------------------------------------------------------------
  Function declarations and documenation
  ------------------------------------------------------------------------*/
QDF_STATUS sme_open(tHalHandle hHal);
QDF_STATUS sme_init_chan_list(tHalHandle hal, uint8_t *alpha2,
		enum country_src cc_src);
QDF_STATUS sme_close(tHalHandle hHal);
QDF_STATUS sme_start(tHalHandle hHal);
QDF_STATUS sme_stop(tHalHandle hHal, tHalStopType stopType);
/*
 * sme_open_session() - Open a session for given persona
 *
 * This is a synchronous API. For any protocol stack related activity
 * requires session to be opened. This API needs to be called to open
 * the session in SME module.
 *
 * hal: The handle returned by mac_open.
 * params: to initialize the session open params
 *
 * Return:
 * QDF_STATUS_SUCCESS - session is opened.
 * Other status means SME is failed to open the session.
 */
QDF_STATUS sme_open_session(tHalHandle hal, struct sme_session_params *params);

/*
 * sme_close_session() - Close a session for given persona
 *
 * This is a synchronous API. This API needs to be called to close the session
 * in SME module before terminating the session completely.
 *
 * hal: The handle returned by mac_open.
 * session_id: A previous opened session's ID.
 *
 * Return:
 * QDF_STATUS_SUCCESS - session is closed.
 * Other status means SME is failed to open the session.
 */
QDF_STATUS sme_close_session(tHalHandle hal, uint8_t sessionId);
void sme_set_curr_device_mode(tHalHandle hHal,
		enum QDF_OPMODE currDeviceMode);
QDF_STATUS sme_update_roam_params(tHalHandle hHal, uint8_t session_id,
		struct roam_ext_params *roam_params_src, int update_param);
QDF_STATUS sme_update_config(tHalHandle hHal,
		tpSmeConfigParams pSmeConfigParams);

QDF_STATUS sme_set11dinfo(tHalHandle hHal, tpSmeConfigParams pSmeConfigParams);
QDF_STATUS sme_get_soft_ap_domain(tHalHandle hHal,
		v_REGDOMAIN_t *domainIdSoftAp);
QDF_STATUS sme_hdd_ready_ind(tHalHandle hHal);
/**
 * sme_ser_cmd_callback() - callback from serialization module
 * @buf: serialization command buffer
 * @reason: reason why serialization module has given this callback
 *
 * Serialization module will give callback to SME for why it triggered
 * the callback
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS sme_ser_cmd_callback(void *buf,
				enum wlan_serialization_cb_reason reason);
QDF_STATUS sme_process_msg(tHalHandle hHal, struct scheduler_msg *pMsg);
QDF_STATUS sme_mc_process_handler(struct scheduler_msg *msg);
void sme_free_msg(tHalHandle hHal, struct scheduler_msg *pMsg);
QDF_STATUS sme_scan_get_result(tHalHandle hHal, uint8_t sessionId,
		tCsrScanResultFilter *pFilter,
		tScanResultHandle *phResult);
QDF_STATUS sme_get_ap_channel_from_scan_cache(
		tCsrRoamProfile *profile,
		tScanResultHandle *scan_cache,
		uint8_t *ap_chnl_id);
QDF_STATUS sme_get_ap_channel_from_scan(void *profile,
		tScanResultHandle *scan_cache,
		uint8_t *ap_chnl_id);
bool sme_store_joinreq_param(tHalHandle hal_handle,
		tCsrRoamProfile *profile,
		tScanResultHandle scan_cache,
		uint32_t *roam_id,
		uint32_t session_id);
bool sme_clear_joinreq_param(tHalHandle hal_handle,
		uint32_t session_id);
QDF_STATUS sme_issue_stored_joinreq(tHalHandle hal_handle,
		uint32_t *roam_id,
		uint32_t session_id);
QDF_STATUS sme_scan_flush_result(tHalHandle hHal);
QDF_STATUS sme_filter_scan_results(tHalHandle hHal, uint8_t sessionId);
QDF_STATUS sme_scan_flush_p2p_result(tHalHandle hHal, uint8_t sessionId);
tCsrScanResultInfo *sme_scan_result_get_first(tHalHandle,
		tScanResultHandle hScanResult);
tCsrScanResultInfo *sme_scan_result_get_next(tHalHandle,
		tScanResultHandle hScanResult);
QDF_STATUS sme_scan_result_purge(tScanResultHandle hScanResult);
QDF_STATUS sme_scan_get_pmkid_candidate_list(tHalHandle hHal, uint8_t sessionId,
		tPmkidCandidateInfo *pPmkidList,
		uint32_t *pNumItems);
QDF_STATUS sme_roam_connect(tHalHandle hHal, uint8_t sessionId,
		tCsrRoamProfile *pProfile, uint32_t *pRoamId);
QDF_STATUS sme_roam_reassoc(tHalHandle hHal, uint8_t sessionId,
		tCsrRoamProfile *pProfile,
		tCsrRoamModifyProfileFields modProfileFields,
		uint32_t *pRoamId, bool fForce);
QDF_STATUS sme_roam_connect_to_last_profile(tHalHandle hHal, uint8_t sessionId);
QDF_STATUS sme_roam_disconnect(tHalHandle hHal, uint8_t sessionId,
		eCsrRoamDisconnectReason reason);
void sme_dhcp_done_ind(tHalHandle hal, uint8_t session_id);
QDF_STATUS sme_roam_stop_bss(tHalHandle hHal, uint8_t sessionId);
QDF_STATUS sme_roam_get_associated_stas(tHalHandle hHal, uint8_t sessionId,
		QDF_MODULE_ID modId, void *pUsrContext,
		void *pfnSapEventCallback,
		uint8_t *pAssocStasBuf);
QDF_STATUS sme_roam_disconnect_sta(tHalHandle hHal, uint8_t sessionId,
		struct csr_del_sta_params *p_del_sta_params);
QDF_STATUS sme_roam_deauth_sta(tHalHandle hHal, uint8_t sessionId,
		struct csr_del_sta_params *pDelStaParams);
QDF_STATUS sme_roam_get_connect_state(tHalHandle hHal, uint8_t sessionId,
		eCsrConnectState *pState);
QDF_STATUS sme_roam_get_connect_profile(tHalHandle hHal, uint8_t sessionId,
		tCsrRoamConnectedProfile *pProfile);
void sme_roam_free_connect_profile(tCsrRoamConnectedProfile *profile);
QDF_STATUS sme_roam_set_pmkid_cache(tHalHandle hHal, uint8_t sessionId,
		tPmkidCacheInfo *pPMKIDCache,
		uint32_t numItems,
		bool update_entire_cache);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS sme_roam_set_psk_pmk(tHalHandle hHal, uint8_t sessionId,
		uint8_t *pPSK_PMK, size_t pmk_len);
#endif
QDF_STATUS sme_roam_get_security_req_ie(tHalHandle hHal, uint8_t sessionId,
		uint32_t *pLen, uint8_t *pBuf,
		eCsrSecurityType secType);
QDF_STATUS sme_roam_get_security_rsp_ie(tHalHandle hHal, uint8_t sessionId,
		uint32_t *pLen, uint8_t *pBuf,
		eCsrSecurityType secType);
uint32_t sme_roam_get_num_pmkid_cache(tHalHandle hHal, uint8_t sessionId);
QDF_STATUS sme_roam_get_pmkid_cache(tHalHandle hHal, uint8_t sessionId,
		uint32_t *pNum,
		tPmkidCacheInfo *pPmkidCache);
QDF_STATUS sme_get_config_param(tHalHandle hHal, tSmeConfigParams *pParam);
QDF_STATUS sme_get_statistics(tHalHandle hHal,
		eCsrStatsRequesterType requesterId,
		uint32_t statsMask, tCsrStatsCallback callback,
		uint8_t staId, void *pContext, uint8_t sessionId);
QDF_STATUS sme_get_rssi(tHalHandle hHal,
		tCsrRssiCallback callback,
		uint8_t staId, struct qdf_mac_addr bssId, int8_t lastRSSI,
		void *pContext);
QDF_STATUS sme_get_snr(tHalHandle hHal,
		tCsrSnrCallback callback,
		uint8_t staId, struct qdf_mac_addr bssId, void *pContext);
#ifdef FEATURE_WLAN_ESE
QDF_STATUS sme_get_tsm_stats(tHalHandle hHal,
		tCsrTsmStatsCallback callback,
		uint8_t staId, struct qdf_mac_addr bssId,
		void *pContext, uint8_t tid);
QDF_STATUS sme_set_cckm_ie(tHalHandle hHal,
		uint8_t sessionId,
		uint8_t *pCckmIe, uint8_t cckmIeLen);
QDF_STATUS sme_set_ese_beacon_request(tHalHandle hHal, const uint8_t sessionId,
		const tCsrEseBeaconReq *pEseBcnReq);
QDF_STATUS sme_set_plm_request(tHalHandle hHal, tpSirPlmReq pPlm);
#endif /*FEATURE_WLAN_ESE */
QDF_STATUS sme_cfg_set_int(tHalHandle hal, uint16_t cfg_id, uint32_t value);
QDF_STATUS sme_cfg_set_str(tHalHandle hal, uint16_t cfg_id, uint8_t *str,
		uint32_t length);
QDF_STATUS sme_cfg_get_int(tHalHandle hal, uint16_t cfg_id,
		uint32_t *cfg_value);
QDF_STATUS sme_cfg_get_str(tHalHandle hal, uint16_t cfg_id, uint8_t *str,
		uint32_t *length);
QDF_STATUS sme_get_modify_profile_fields(tHalHandle hHal, uint8_t sessionId,
					 tCsrRoamModifyProfileFields *
					 pModifyProfileFields);

extern QDF_STATUS sme_set_host_power_save(tHalHandle hHal, bool psMode);

void sme_set_dhcp_till_power_active_flag(tHalHandle hHal, uint8_t flag);
extern QDF_STATUS sme_register11d_scan_done_callback(tHalHandle hHal,
		csr_scan_completeCallback);
void sme_deregister11d_scan_done_callback(tHalHandle hHal);

#ifdef FEATURE_OEM_DATA_SUPPORT
extern QDF_STATUS sme_register_oem_data_rsp_callback(tHalHandle h_hal,
		sme_send_oem_data_rsp_msg callback);
void sme_deregister_oem_data_rsp_callback(tHalHandle h_hal);

#else
static inline QDF_STATUS sme_register_oem_data_rsp_callback(tHalHandle hal,
		void *callback)
{
	return QDF_STATUS_SUCCESS;
}
static inline void sme_deregister_oem_data_rsp_callback(tHalHandle h_hal)
{
}

#endif

extern QDF_STATUS sme_enter_wowl(tHalHandle hHal,
			 void (*enter_wowl_callback_routine)(void
						  *callbackContext,
						  QDF_STATUS  status),
			 void *enter_wowl_callback_context,
#ifdef WLAN_WAKEUP_EVENTS
			 void (*wake_reason_ind_cb)(void *callbackContext,
						 tpSirWakeReasonInd
						 wake_reason_ind),
			 void *wake_reason_ind_cb_ctx,
#endif /* WLAN_WAKEUP_EVENTS */
			 tpSirSmeWowlEnterParams wowl_enter_params,
			 uint8_t sessionId);

extern QDF_STATUS sme_exit_wowl(tHalHandle hHal,
		tpSirSmeWowlExitParams wowl_exit_params);
QDF_STATUS sme_roam_set_key(tHalHandle, uint8_t sessionId,
		tCsrRoamSetKey *pSetKey, uint32_t *pRoamId);
QDF_STATUS sme_get_country_code(tHalHandle hHal, uint8_t *pBuf, uint8_t *pbLen);


/* some support functions */
bool sme_is11d_supported(tHalHandle hHal);
bool sme_is11h_supported(tHalHandle hHal);
bool sme_is_wmm_supported(tHalHandle hHal);

typedef void (*tSmeChangeCountryCallback)(void *pContext);
QDF_STATUS sme_change_country_code(tHalHandle hHal,
		tSmeChangeCountryCallback callback,
		uint8_t *pCountry,
		void *pContext,
		void *p_cds_context,
		bool countryFromUserSpace,
		bool sendRegHint);
QDF_STATUS sme_generic_change_country_code(tHalHandle hHal,
					   uint8_t *pCountry);
QDF_STATUS sme_tx_fail_monitor_start_stop_ind(tHalHandle hHal,
		uint8_t tx_fail_count,
		void *txFailIndCallback);
QDF_STATUS sme_dhcp_start_ind(tHalHandle hHal,
		uint8_t device_mode,
		uint8_t *macAddr, uint8_t sessionId);
QDF_STATUS sme_dhcp_stop_ind(tHalHandle hHal,
		uint8_t device_mode,
		uint8_t *macAddr, uint8_t sessionId);
void sme_set_cfg_privacy(tHalHandle hHal, tCsrRoamProfile *pProfile,
		bool fPrivacy);
void sme_get_recovery_stats(tHalHandle hHal);
QDF_STATUS sme_neighbor_report_request(tHalHandle hHal, uint8_t sessionId,
		tpRrmNeighborReq pRrmNeighborReq,
		tpRrmNeighborRspCallbackInfo callbackInfo);
QDF_STATUS sme_get_wcnss_wlan_compiled_version(tHalHandle hHal,
		tSirVersionType * pVersion);
QDF_STATUS sme_get_wcnss_wlan_reported_version(tHalHandle hHal,
		tSirVersionType *pVersion);
QDF_STATUS sme_get_wcnss_software_version(tHalHandle hHal,
		uint8_t *pVersion, uint32_t versionBufferSize);
QDF_STATUS sme_get_wcnss_hardware_version(tHalHandle hHal,
		uint8_t *pVersion, uint32_t versionBufferSize);
#ifdef FEATURE_WLAN_WAPI
QDF_STATUS sme_scan_get_bkid_candidate_list(tHalHandle hHal, uint32_t sessionId,
		tBkidCandidateInfo * pBkidList,
		uint32_t *pNumItems);
#endif /* FEATURE_WLAN_WAPI */
#ifdef FEATURE_OEM_DATA_SUPPORT
QDF_STATUS sme_oem_data_req(tHalHandle hHal, struct oem_data_req *);
QDF_STATUS sme_oem_update_capability(tHalHandle hHal,
				     struct sme_oem_capability *cap);
QDF_STATUS sme_oem_get_capability(tHalHandle hHal,
				  struct sme_oem_capability *cap);
#endif /*FEATURE_OEM_DATA_SUPPORT */
QDF_STATUS sme_change_mcc_beacon_interval(uint8_t sessionId);
QDF_STATUS sme_set_host_offload(tHalHandle hHal, uint8_t sessionId,
		tpSirHostOffloadReq pRequest);
QDF_STATUS sme_set_keep_alive(tHalHandle hHal, uint8_t sessionId,
		tpSirKeepAliveReq pRequest);
QDF_STATUS sme_get_operation_channel(tHalHandle hHal, uint32_t *pChannel,
		uint8_t sessionId);
QDF_STATUS sme_register_mgmt_frame(tHalHandle hHal, uint8_t sessionId,
		uint16_t frameType, uint8_t *matchData,
		uint16_t matchLen);
QDF_STATUS sme_deregister_mgmt_frame(tHalHandle hHal, uint8_t sessionId,
		uint16_t frameType, uint8_t *matchData,
		uint16_t matchLen);
QDF_STATUS sme_ConfigureAppsCpuWakeupState(tHalHandle hHal, bool isAppsAwake);
#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
QDF_STATUS sme_configure_ext_wow(tHalHandle hHal,
		tpSirExtWoWParams wlanExtParams,
		csr_readyToSuspendCallback callback,
		void *callbackContext);
QDF_STATUS sme_configure_app_type1_params(tHalHandle hHal,
		tpSirAppType1Params wlanAppType1Params);
QDF_STATUS sme_configure_app_type2_params(tHalHandle hHal,
		tpSirAppType2Params wlanAppType2Params);
#endif
int8_t sme_get_infra_session_id(tHalHandle hHal);
uint8_t sme_get_infra_operation_channel(tHalHandle hHal, uint8_t sessionId);
uint8_t sme_get_concurrent_operation_channel(tHalHandle hHal);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
uint16_t sme_check_concurrent_channel_overlap(tHalHandle hHal, uint16_t sap_ch,
		eCsrPhyMode sapPhyMode,
		uint8_t cc_switch_mode);
#endif
QDF_STATUS sme_get_cfg_valid_channels(uint8_t *aValidChannels,
		uint32_t *len);
#ifdef WLAN_FEATURE_PACKET_FILTERING
QDF_STATUS sme_8023_multicast_list(tHalHandle hHal, uint8_t sessionId,
		tpSirRcvFltMcAddrList pMulticastAddrs);
#endif /* WLAN_FEATURE_PACKET_FILTERING */
bool sme_is_channel_valid(tHalHandle hHal, uint8_t channel);
QDF_STATUS sme_set_freq_band(tHalHandle hHal, uint8_t sessionId,
		enum band_info eBand);
QDF_STATUS sme_get_freq_band(tHalHandle hHal, enum band_info *pBand);
uint16_t sme_chn_to_freq(uint8_t chanNum);
bool sme_is_channel_valid(tHalHandle hHal, uint8_t channel);
QDF_STATUS sme_set_max_tx_power(tHalHandle hHal, struct qdf_mac_addr pBssid,
		struct qdf_mac_addr pSelfMacAddress, int8_t dB);
QDF_STATUS sme_set_max_tx_power_per_band(enum band_info band, int8_t db);
QDF_STATUS sme_set_tx_power(tHalHandle hHal, uint8_t sessionId,
		struct qdf_mac_addr bssid,
		enum QDF_OPMODE dev_mode, int power);
QDF_STATUS sme_set_custom_mac_addr(tSirMacAddr customMacAddr);
QDF_STATUS sme_hide_ssid(tHalHandle hHal, uint8_t sessionId,
		uint8_t ssidHidden);
QDF_STATUS sme_set_tm_level(tHalHandle hHal, uint16_t newTMLevel,
		uint16_t tmMode);
void sme_feature_caps_exchange(tHalHandle hHal);
void sme_disable_feature_capablity(uint8_t feature_index);
void sme_reset_power_values_for5_g(tHalHandle hHal);
QDF_STATUS sme_update_roam_prefer5_g_hz(tHalHandle hHal, bool nRoamPrefer5GHz);
QDF_STATUS sme_set_roam_intra_band(tHalHandle hHal, const bool nRoamIntraBand);
QDF_STATUS sme_update_roam_scan_n_probes(tHalHandle hHal, uint8_t sessionId,
		const uint8_t nProbes);
QDF_STATUS sme_update_roam_scan_home_away_time(tHalHandle hHal,
		uint8_t sessionId,
		const uint16_t nRoamScanHomeAwayTime,
		const bool bSendOffloadCmd);

bool sme_get_roam_intra_band(tHalHandle hHal);
uint8_t sme_get_roam_scan_n_probes(tHalHandle hHal);
uint16_t sme_get_roam_scan_home_away_time(tHalHandle hHal);
QDF_STATUS sme_update_roam_rssi_diff(tHalHandle hHal, uint8_t sessionId,
		uint8_t RoamRssiDiff);
QDF_STATUS sme_update_fast_transition_enabled(tHalHandle hHal,
		bool isFastTransitionEnabled);
QDF_STATUS sme_update_wes_mode(tHalHandle hHal, bool isWESModeEnabled,
		uint8_t sessionId);
QDF_STATUS sme_set_roam_scan_control(tHalHandle hHal, uint8_t sessionId,
		bool roamScanControl);

QDF_STATUS sme_update_is_fast_roam_ini_feature_enabled(tHalHandle hHal,
		uint8_t sessionId,
		const bool
		isFastRoamIniFeatureEnabled);

QDF_STATUS sme_config_fast_roaming(tHalHandle hal, uint8_t session_id,
		const bool is_fast_roam_enabled);

QDF_STATUS sme_update_is_mawc_ini_feature_enabled(tHalHandle hHal,
		const bool MAWCEnabled);
QDF_STATUS sme_stop_roaming(tHalHandle hHal, uint8_t sessionId, uint8_t reason);

/**
 * sme_indicate_disconnect_inprogress() - Indicate to csr that disconnect is in
 * progress
 * @hal: The handle returned by mac_open
 * @session_id: sessionId on which disconenct has started
 *
 * Return: void
 */
void sme_indicate_disconnect_inprogress(tHalHandle hal, uint8_t session_id);
QDF_STATUS sme_start_roaming(tHalHandle hHal, uint8_t sessionId,
		uint8_t reason);
QDF_STATUS sme_update_enable_fast_roam_in_concurrency(tHalHandle hHal,
		bool bFastRoamInConIniFeatureEnabled);
#ifdef FEATURE_WLAN_ESE
QDF_STATUS sme_update_is_ese_feature_enabled(tHalHandle hHal, uint8_t sessionId,
		const bool isEseIniFeatureEnabled);
#endif /* FEATURE_WLAN_ESE */
QDF_STATUS sme_update_config_fw_rssi_monitoring(tHalHandle hHal,
		bool fEnableFwRssiMonitoring);
QDF_STATUS sme_set_roam_rescan_rssi_diff(tHalHandle hHal,
		uint8_t sessionId,
		const uint8_t nRoamRescanRssiDiff);
uint8_t sme_get_roam_rescan_rssi_diff(tHalHandle hHal);

QDF_STATUS sme_set_roam_opportunistic_scan_threshold_diff(tHalHandle hHal,
		uint8_t sessionId,
		const uint8_t nOpportunisticThresholdDiff);
uint8_t sme_get_roam_opportunistic_scan_threshold_diff(tHalHandle hHal);
QDF_STATUS sme_set_neighbor_lookup_rssi_threshold(tHalHandle hHal,
		uint8_t sessionId, uint8_t neighborLookupRssiThreshold);
QDF_STATUS sme_set_delay_before_vdev_stop(tHalHandle hHal,
		uint8_t sessionId, uint8_t delay_before_vdev_stop);
uint8_t sme_get_neighbor_lookup_rssi_threshold(tHalHandle hHal);
QDF_STATUS sme_set_neighbor_scan_refresh_period(tHalHandle hHal,
		uint8_t sessionId, uint16_t neighborScanResultsRefreshPeriod);
uint16_t sme_get_neighbor_scan_refresh_period(tHalHandle hHal);
uint16_t sme_get_empty_scan_refresh_period(tHalHandle hHal);
QDF_STATUS sme_update_empty_scan_refresh_period(tHalHandle hHal,
		uint8_t sessionId, uint16_t nEmptyScanRefreshPeriod);
QDF_STATUS sme_set_neighbor_scan_min_chan_time(tHalHandle hHal,
		const uint16_t nNeighborScanMinChanTime,
		uint8_t sessionId);
QDF_STATUS sme_set_neighbor_scan_max_chan_time(tHalHandle hHal,
		uint8_t sessionId, const uint16_t nNeighborScanMaxChanTime);
uint16_t sme_get_neighbor_scan_min_chan_time(tHalHandle hHal,
		uint8_t sessionId);
uint32_t sme_get_neighbor_roam_state(tHalHandle hHal, uint8_t sessionId);
uint32_t sme_get_current_roam_state(tHalHandle hHal, uint8_t sessionId);
uint32_t sme_get_current_roam_sub_state(tHalHandle hHal, uint8_t sessionId);
uint32_t sme_get_lim_sme_state(tHalHandle hHal);
uint32_t sme_get_lim_mlm_state(tHalHandle hHal);
bool sme_is_lim_session_valid(tHalHandle hHal, uint8_t sessionId);
uint32_t sme_get_lim_sme_session_state(tHalHandle hHal, uint8_t sessionId);
uint32_t sme_get_lim_mlm_session_state(tHalHandle hHal, uint8_t sessionId);
uint16_t sme_get_neighbor_scan_max_chan_time(tHalHandle hHal,
		uint8_t sessionId);
QDF_STATUS sme_set_neighbor_scan_period(tHalHandle hHal, uint8_t sessionId,
		const uint16_t nNeighborScanPeriod);
uint16_t sme_get_neighbor_scan_period(tHalHandle hHal, uint8_t sessionId);
QDF_STATUS sme_set_neighbor_scan_min_period(tHalHandle h_hal,
		uint8_t session_id, const uint16_t neighbor_scan_min_period);
QDF_STATUS sme_set_roam_bmiss_first_bcnt(tHalHandle hHal,
		uint8_t sessionId, const uint8_t nRoamBmissFirstBcnt);
uint8_t sme_get_roam_bmiss_first_bcnt(tHalHandle hHal);
QDF_STATUS sme_set_roam_bmiss_final_bcnt(tHalHandle hHal, uint8_t sessionId,
		const uint8_t nRoamBmissFinalBcnt);
uint8_t sme_get_roam_bmiss_final_bcnt(tHalHandle hHal);
QDF_STATUS sme_set_roam_beacon_rssi_weight(tHalHandle hHal, uint8_t sessionId,
		const uint8_t nRoamBeaconRssiWeight);
uint8_t sme_get_roam_beacon_rssi_weight(tHalHandle hHal);
uint8_t sme_get_roam_rssi_diff(tHalHandle hHal);
QDF_STATUS sme_change_roam_scan_channel_list(tHalHandle hHal, uint8_t sessionId,
		uint8_t *pChannelList,
		uint8_t numChannels);
QDF_STATUS sme_set_ese_roam_scan_channel_list(tHalHandle hHal,
		uint8_t sessionId, uint8_t *pChannelList,
		uint8_t numChannels);
QDF_STATUS sme_get_roam_scan_channel_list(tHalHandle hHal,
		uint8_t *pChannelList, uint8_t *pNumChannels,
		uint8_t sessionId);
bool sme_get_is_ese_feature_enabled(tHalHandle hHal);
bool sme_get_wes_mode(tHalHandle hHal);
bool sme_get_roam_scan_control(tHalHandle hHal);
bool sme_get_is_lfr_feature_enabled(tHalHandle hHal);
bool sme_get_is_ft_feature_enabled(tHalHandle hHal);
QDF_STATUS sme_update_roam_scan_offload_enabled(tHalHandle hHal,
		bool nRoamScanOffloadEnabled);
bool sme_is_feature_supported_by_fw(enum cap_bitmap feature);

/*
 * SME API to enable/disable WLAN driver initiated SSR
 */
void sme_update_enable_ssr(tHalHandle hHal, bool enableSSR);
QDF_STATUS sme_set_phy_mode(tHalHandle hHal, eCsrPhyMode phyMode);
eCsrPhyMode sme_get_phy_mode(tHalHandle hHal);
QDF_STATUS sme_handoff_request(tHalHandle hHal, uint8_t sessionId,
			       tCsrHandoffRequest *pHandoffInfo);
QDF_STATUS sme_is_sta_p2p_client_connected(tHalHandle hHal);
QDF_STATUS sme_add_periodic_tx_ptrn(tHalHandle hHal, tSirAddPeriodicTxPtrn
		*addPeriodicTxPtrnParams);
QDF_STATUS sme_del_periodic_tx_ptrn(tHalHandle hHal, tSirDelPeriodicTxPtrn
		*delPeriodicTxPtrnParams);
QDF_STATUS sme_send_rate_update_ind(tHalHandle hHal,
		tSirRateUpdateInd *rateUpdateParams);
QDF_STATUS sme_roam_del_pmkid_from_cache(tHalHandle hHal, uint8_t sessionId,
		tPmkidCacheInfo *pmksa, bool flush_cache);
void sme_get_command_q_status(tHalHandle hHal);

QDF_STATUS sme_enable_rmc(tHalHandle hHal, uint32_t sessionId);
QDF_STATUS sme_disable_rmc(tHalHandle hHal, uint32_t sessionId);
QDF_STATUS sme_send_rmc_action_period(tHalHandle hHal, uint32_t sessionId);
QDF_STATUS sme_request_ibss_peer_info(tHalHandle hHal, void *pUserData,
	pIbssPeerInfoCb peerInfoCbk, bool allPeerInfoReqd, uint8_t staIdx);
QDF_STATUS sme_send_cesium_enable_ind(tHalHandle hHal, uint32_t sessionId);

/**
 * sme_set_wlm_latency_level_ind() - Used to set the latency level to fw
 * @hal
 * @session_id
 * @latency_level
 *
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_wlm_latency_level(tHalHandle hal,
				     uint16_t session_id,
				     uint16_t latency_level);
/*
 * SME API to enable/disable idle mode powersave
 * This should be called only if powersave offload
 * is enabled
 */
QDF_STATUS sme_set_idle_powersave_config(bool value);
QDF_STATUS sme_notify_modem_power_state(tHalHandle hHal, uint32_t value);

/*SME API to convert convert the ini value to the ENUM used in csr and MAC*/
ePhyChanBondState sme_get_cb_phy_state_from_cb_ini_value(uint32_t cb_ini_value);
int sme_update_ht_config(tHalHandle hHal, uint8_t sessionId, uint16_t htCapab,
		int value);
int16_t sme_get_ht_config(tHalHandle hHal, uint8_t session_id,
		uint16_t ht_capab);
#ifdef QCA_HT_2040_COEX
QDF_STATUS sme_notify_ht2040_mode(tHalHandle hHal, uint16_t staId,
		struct qdf_mac_addr macAddrSTA,
		uint8_t sessionId,
		uint8_t channel_type);
QDF_STATUS sme_set_ht2040_mode(tHalHandle hHal, uint8_t sessionId,
		uint8_t channel_type, bool obssEnabled);
#endif
QDF_STATUS sme_get_reg_info(tHalHandle hHal, uint8_t chanId,
		uint32_t *regInfo1, uint32_t *regInfo2);
#ifdef FEATURE_WLAN_TDLS
QDF_STATUS sme_update_fw_tdls_state(tHalHandle hHal, void *psmeTdlsParams,
		bool useSmeLock);
#endif /* FEATURE_WLAN_TDLS */
QDF_STATUS sme_ch_avoid_update_req(tHalHandle hHal);
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
QDF_STATUS sme_set_auto_shutdown_cb(tHalHandle hHal, void (*pCallbackfn)(void));
QDF_STATUS sme_set_auto_shutdown_timer(tHalHandle hHal, uint32_t timer_value);
#endif
QDF_STATUS sme_roam_channel_change_req(tHalHandle hHal,
				       struct qdf_mac_addr bssid,
				       struct ch_params *ch_params,
				       tCsrRoamProfile *profile);

QDF_STATUS sme_roam_start_beacon_req(tHalHandle hHal,
		struct qdf_mac_addr bssid, uint8_t dfsCacWaitStatus);
QDF_STATUS sme_roam_csa_ie_request(tHalHandle hHal, struct qdf_mac_addr bssid,
				   uint8_t targetChannel, uint8_t csaIeReqd,
				   struct ch_params *ch_params);

/**
 * sme_set_addba_accept() - Allow/Reject the ADDBA req session
 * @hal: handle returned by mac_open
 * @session_id: sme session id
 * @value: Allow/Reject AddBA session
 *
 * Allows/Rejects the ADDBA req session
 *
 * Return: 0 on success else errno
 */
int sme_set_addba_accept(tHalHandle hal, uint8_t session_id, int value);

QDF_STATUS sme_init_thermal_info(tHalHandle hHal,
				 tSmeThermalParams thermalParam);

QDF_STATUS sme_set_thermal_level(tHalHandle hHal, uint8_t level);
QDF_STATUS sme_txpower_limit(tHalHandle hHal, tSirTxPowerLimit *psmetx);
QDF_STATUS sme_get_link_speed(tHalHandle hHal, tSirLinkSpeedInfo *lsReq,
		void *plsContext,
		void (*pCallbackfn)(tSirLinkSpeedInfo *indParam,
		void *pContext));
QDF_STATUS sme_modify_add_ie(tHalHandle hHal,
		tSirModifyIE *pModifyIE, eUpdateIEsType updateType);
QDF_STATUS sme_update_add_ie(tHalHandle hHal,
		tSirUpdateIE *pUpdateIE, eUpdateIEsType updateType);
QDF_STATUS sme_update_connect_debug(tHalHandle hHal, uint32_t set_value);
const char *sme_bss_type_to_string(const uint8_t bss_type);
QDF_STATUS sme_ap_disable_intra_bss_fwd(tHalHandle hHal, uint8_t sessionId,
		bool disablefwd);
QDF_STATUS sme_get_channel_bonding_mode5_g(tHalHandle hHal, uint32_t *mode);
QDF_STATUS sme_get_channel_bonding_mode24_g(tHalHandle hHal, uint32_t *mode);
#ifdef WLAN_FEATURE_STATS_EXT
typedef struct sStatsExtRequestReq {
	uint32_t request_data_len;
	uint8_t *request_data;
} tStatsExtRequestReq, *tpStatsExtRequestReq;
typedef void (*StatsExtCallback)(void *, tStatsExtEvent *);
void sme_stats_ext_register_callback(tHalHandle hHal,
		StatsExtCallback callback);
/**
 * sme_register_stats_ext2_callback() - Register stats ext2 register
 * @hal_handle: hal handle for getting global mac struct
 * @stats_ext2_cb: callback to be registered
 *
 * This function will register a callback for frame aggregation failure
 * indications processing.
 *
 * Return: void
 */
void sme_stats_ext2_register_callback(tHalHandle hal_handle,
	void (*stats_ext2_cb)(void *, struct sir_sme_rx_aggr_hole_ind *));
/**
 * sme_send_unit_test_cmd() - send unit test command to lower layer
 * @session_id: sme session id to be filled while forming the command
 * @module_id: module id given by user to be filled in the command
 * @arg_count: number of argument count
 * @arg: pointer to argument list
 *
 * This API exposed to HDD layer which takes the argument from user and sends
 * down to lower layer for further processing
 *
 * Return: QDF_STATUS based on overall success
 */
QDF_STATUS sme_send_unit_test_cmd(uint32_t vdev_id, uint32_t module_id,
				  uint32_t arg_count, uint32_t *arg);
void sme_stats_ext_deregister_callback(tHalHandle hhal);
QDF_STATUS sme_stats_ext_request(uint8_t session_id,
		tpStatsExtRequestReq input);
QDF_STATUS sme_stats_ext_event(tHalHandle hHal, void *pMsg);
#endif
QDF_STATUS sme_update_dfs_scan_mode(tHalHandle hHal,
		uint8_t sessionId,
		uint8_t allowDFSChannelRoam);
uint8_t sme_get_dfs_scan_mode(tHalHandle hHal);

#ifdef FEATURE_WLAN_EXTSCAN
QDF_STATUS sme_get_valid_channels_by_band(tHalHandle hHal, uint8_t wifiBand,
		uint32_t *aValidChannels,
		uint8_t *pNumChannels);
QDF_STATUS sme_ext_scan_get_capabilities(tHalHandle hHal,
		tSirGetExtScanCapabilitiesReqParams *pReq);
QDF_STATUS sme_ext_scan_start(tHalHandle hHal,
		tSirWifiScanCmdReqParams *pStartCmd);
QDF_STATUS sme_ext_scan_stop(tHalHandle hHal,
		tSirExtScanStopReqParams *pStopReq);
QDF_STATUS sme_set_bss_hotlist(tHalHandle hHal,
		tSirExtScanSetBssidHotListReqParams *
		pSetHotListReq);
QDF_STATUS sme_reset_bss_hotlist(tHalHandle hHal,
		tSirExtScanResetBssidHotlistReqParams *
		pResetReq);
QDF_STATUS sme_set_significant_change(tHalHandle hHal,
		tSirExtScanSetSigChangeReqParams *
		pSetSignificantChangeReq);
QDF_STATUS sme_reset_significant_change(tHalHandle hHal,
		tSirExtScanResetSignificantChangeReqParams
		*pResetReq);
QDF_STATUS sme_get_cached_results(tHalHandle hHal,
		tSirExtScanGetCachedResultsReqParams *
		pCachedResultsReq);

QDF_STATUS sme_set_epno_list(tHalHandle hal,
			     struct wifi_epno_params *req_msg);
QDF_STATUS sme_set_passpoint_list(tHalHandle hal,
					struct wifi_passpoint_req *req_msg);
QDF_STATUS sme_reset_passpoint_list(tHalHandle hal,
					struct wifi_passpoint_req *req_msg);

QDF_STATUS sme_ext_scan_register_callback(tHalHandle hHal,
		void (*pExtScanIndCb)(void *, const uint16_t, void *));
#else
static inline QDF_STATUS sme_ext_scan_register_callback(tHalHandle hHal,
		void (*pExtScanIndCb)(void *, const uint16_t, void *))
{
	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_WLAN_EXTSCAN */
QDF_STATUS sme_abort_roam_scan(tHalHandle hHal, uint8_t sessionId);
#ifdef WLAN_FEATURE_LINK_LAYER_STATS
QDF_STATUS sme_ll_stats_clear_req(tHalHandle hHal,
		tSirLLStatsClearReq * pclearStatsReq);
QDF_STATUS sme_ll_stats_set_req(tHalHandle hHal,
		tSirLLStatsSetReq *psetStatsReq);
QDF_STATUS sme_ll_stats_get_req(tHalHandle hHal,
		tSirLLStatsGetReq *pgetStatsReq);
QDF_STATUS sme_set_link_layer_stats_ind_cb(tHalHandle hHal,
		void (*callbackRoutine)(void *callbackCtx,
				int indType, void *pRsp));
QDF_STATUS sme_set_link_layer_ext_cb(tHalHandle hal,
		     void (*ll_stats_ext_cb)(tHddHandle callback_ctx,
					     tSirLLStatsResults * rsp));
QDF_STATUS sme_reset_link_layer_stats_ind_cb(tHalHandle hhal);
QDF_STATUS sme_ll_stats_set_thresh(tHalHandle hal,
				struct sir_ll_ext_stats_threshold *threshold);
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

QDF_STATUS sme_set_wisa_params(tHalHandle hal,
				struct sir_wisa_params *wisa_params);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS sme_update_roam_offload_enabled(tHalHandle hHal,
		bool nRoamOffloadEnabled);
QDF_STATUS sme_update_roam_key_mgmt_offload_enabled(tHalHandle hal_ctx,
		uint8_t session_id,
		bool key_mgmt_offload_enabled,
		struct pmkid_mode_bits *pmkid_modes);
#endif
#ifdef WLAN_FEATURE_NAN
QDF_STATUS sme_nan_event(tHalHandle hHal, void *pMsg);
#endif /* WLAN_FEATURE_NAN */
QDF_STATUS sme_get_link_status(tHalHandle hHal,
		tCsrLinkStatusCallback callback,
		void *pContext, uint8_t sessionId);
QDF_STATUS sme_get_temperature(tHalHandle hHal,
		void *tempContext,
		void (*pCallbackfn)(int temperature,
			void *pContext));
QDF_STATUS sme_set_scanning_mac_oui(tHalHandle hHal,
		tSirScanMacOui *pScanMacOui);

#ifdef IPA_OFFLOAD
/* ---------------------------------------------------------------------------
    \fn sme_ipa_offload_enable_disable
    \brief  API to enable/disable IPA offload
    \param  hHal - The handle returned by macOpen.
    \param  sessionId - Session Identifier
    \param  pRequest -  Pointer to the offload request.
    \return QDF_STATUS
  ---------------------------------------------------------------------------*/
QDF_STATUS sme_ipa_offload_enable_disable(tHalHandle hal,
				uint8_t session_id,
				struct sir_ipa_offload_enable_disable *request);
#else
static inline QDF_STATUS sme_ipa_offload_enable_disable(tHalHandle hal,
				uint8_t session_id,
				struct sir_ipa_offload_enable_disable *request)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* IPA_OFFLOAD */

#ifdef DHCP_SERVER_OFFLOAD
QDF_STATUS sme_set_dhcp_srv_offload(tHalHandle hHal,
		tSirDhcpSrvOffloadInfo * pDhcpSrvInfo);
#endif /* DHCP_SERVER_OFFLOAD */
#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
QDF_STATUS sme_set_led_flashing(tHalHandle hHal, uint8_t type,
		uint32_t x0, uint32_t x1);
#endif
QDF_STATUS sme_handle_dfs_chan_scan(tHalHandle hHal, uint8_t dfs_flag);
QDF_STATUS sme_set_mas(uint32_t val);
QDF_STATUS sme_set_miracast(tHalHandle hal, uint8_t filter_type);
QDF_STATUS sme_ext_change_channel(tHalHandle hHal, uint32_t channel,
					  uint8_t session_id);

QDF_STATUS sme_configure_modulated_dtim(tHalHandle hal, uint8_t session_id,
				      uint32_t modulated_dtim);

QDF_STATUS sme_override_listen_interval(tHalHandle h_hal, uint8_t session_id,
		uint32_t override_li);

QDF_STATUS sme_configure_stats_avg_factor(tHalHandle hal, uint8_t session_id,
					  uint16_t stats_avg_factor);

QDF_STATUS sme_configure_guard_time(tHalHandle hal, uint8_t session_id,
				    uint32_t guard_time);

QDF_STATUS sme_wifi_start_logger(tHalHandle hal,
		struct sir_wifi_start_log start_log);

bool sme_neighbor_middle_of_roaming(tHalHandle hHal,
						uint8_t sessionId);

/**
 * sme_enable_uapsd_for_ac() - enable uapsd for access catagory request to WMA
 * @sta_id: station id
 * @ac: access catagory
 * @tid: tid value
 * @pri: user priority
 * @srvc_int: service interval
 * @sus_int: suspend interval
 * @dir: tspec direction
 * @psb: PSB value
 * @sessionId: session id
 * @delay_interval: delay interval
 *
 * Return: QDF status
 */
QDF_STATUS sme_enable_uapsd_for_ac(uint8_t sta_id,
				      sme_ac_enum_type ac, uint8_t tid,
				      uint8_t pri, uint32_t srvc_int,
				      uint32_t sus_int,
				      enum sme_qos_wmm_dir_type dir,
				      uint8_t psb, uint32_t sessionId,
				      uint32_t delay_interval);

/**
 * sme_disable_uapsd_for_ac() - disable uapsd access catagory request to WMA
 * @sta_id: station id
 * @ac: access catagory
 * @sessionId: session id
 *
 * Return: QDF status
 */
QDF_STATUS sme_disable_uapsd_for_ac(uint8_t sta_id,
				       sme_ac_enum_type ac,
				       uint32_t sessionId);

QDF_STATUS sme_set_rssi_monitoring(tHalHandle hal,
					struct rssi_monitor_req *input);
QDF_STATUS sme_set_rssi_threshold_breached_cb(tHalHandle hal,
			void (*cb)(void *, struct rssi_breach_event *));
QDF_STATUS sme_reset_rssi_threshold_breached_cb(tHalHandle hal);

QDF_STATUS sme_register_mgmt_frame_ind_callback(tHalHandle hal,
			sir_mgmt_frame_ind_callback callback);

QDF_STATUS sme_update_nss(tHalHandle h_hal, uint8_t nss);
void sme_update_user_configured_nss(tHalHandle hal, uint8_t nss);

bool sme_is_any_session_in_connected_state(tHalHandle h_hal);

QDF_STATUS sme_pdev_set_pcl(struct policy_mgr_pcl_list msg);
QDF_STATUS sme_pdev_set_hw_mode(struct policy_mgr_hw_mode msg);
void sme_register_hw_mode_trans_cb(tHalHandle hal,
		hw_mode_transition_cb callback);
QDF_STATUS sme_nss_update_request(uint32_t vdev_id,
				uint8_t  new_nss, policy_mgr_nss_update_cback cback,
				uint8_t next_action, struct wlan_objmgr_psoc *psoc,
				enum policy_mgr_conn_update_reason reason);

typedef void (*sme_peer_authorized_fp) (uint32_t vdev_id);
QDF_STATUS sme_set_peer_authorized(uint8_t *peer_addr,
				   sme_peer_authorized_fp auth_fp,
				   uint32_t vdev_id);
QDF_STATUS sme_soc_set_dual_mac_config(struct policy_mgr_dual_mac_config msg);
QDF_STATUS sme_soc_set_antenna_mode(tHalHandle hal,
		struct sir_antenna_mode_param *msg);

void sme_set_scan_disable(tHalHandle h_hal, int value);
void sme_setdef_dot11mode(tHalHandle hal);

QDF_STATUS sme_handle_set_fcc_channel(tHalHandle hHal,
				      bool fcc_constraint,
				      bool scan_pending);

QDF_STATUS sme_update_roam_scan_hi_rssi_scan_params(tHalHandle hal_handle,
	uint8_t session_id,
	uint32_t notify_id,
	int32_t val);

void wlan_sap_enable_phy_error_logs(tHalHandle hal, uint32_t enable_log);
#ifdef WLAN_FEATURE_DSRC
void sme_set_dot11p_config(tHalHandle hal, bool enable_dot11p);

QDF_STATUS sme_ocb_set_config(tHalHandle hHal, void *context,
			      ocb_callback callback,
			      struct sir_ocb_config *config);

QDF_STATUS sme_ocb_set_utc_time(tHalHandle hHal, struct sir_ocb_utc *utc);

QDF_STATUS sme_ocb_start_timing_advert(tHalHandle hHal,
	struct sir_ocb_timing_advert *timing_advert);

QDF_STATUS sme_ocb_stop_timing_advert(tHalHandle hHal,
	struct sir_ocb_timing_advert *timing_advert);

int sme_ocb_gen_timing_advert_frame(tHalHandle hHal, tSirMacAddr self_addr,
				    uint8_t **buf, uint32_t *timestamp_offset,
				    uint32_t *time_value_offset);

QDF_STATUS sme_ocb_get_tsf_timer(tHalHandle hHal, void *context,
				 ocb_callback callback,
				 struct sir_ocb_get_tsf_timer *request);

QDF_STATUS sme_dcc_get_stats(tHalHandle hHal, void *context,
			     ocb_callback callback,
			     struct sir_dcc_get_stats *request);

QDF_STATUS sme_dcc_clear_stats(tHalHandle hHal, uint32_t vdev_id,
			       uint32_t dcc_stats_bitmap);

QDF_STATUS sme_dcc_update_ndl(tHalHandle hHal, void *context,
			      ocb_callback callback,
			      struct sir_dcc_update_ndl *request);

QDF_STATUS sme_register_for_dcc_stats_event(tHalHandle hHal, void *context,
					    ocb_callback callback);
QDF_STATUS sme_deregister_for_dcc_stats_event(tHalHandle hHal);

#else
static inline void sme_set_dot11p_config(tHalHandle hal, bool enable_dot11p)
{
	return;
}

static inline QDF_STATUS sme_ocb_set_config(tHalHandle hHal, void *context,
		ocb_callback callback,
		struct sir_ocb_config *config)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS sme_ocb_set_utc_time(struct sir_ocb_utc *utc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS sme_ocb_start_timing_advert(
		struct sir_ocb_timing_advert *timing_advert)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS sme_ocb_stop_timing_advert(struct sir_ocb_timing_advert
		*timing_advert)
{
	return QDF_STATUS_SUCCESS;
}

static inline int sme_ocb_gen_timing_advert_frame(tHalHandle hHal,
		tSirMacAddr self_addr, uint8_t **buf,
		uint32_t *timestamp_offset,
		uint32_t *time_value_offset)
{
	return 0;
}

static inline QDF_STATUS sme_ocb_get_tsf_timer(tHalHandle hHal, void *context,
		ocb_callback callback,
		struct sir_ocb_get_tsf_timer *request)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS sme_dcc_get_stats(tHalHandle hHal, void *context,
		ocb_callback callback,
		struct sir_dcc_get_stats *request)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS sme_dcc_clear_stats(uint32_t vdev_id,
		uint32_t dcc_stats_bitmap)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS sme_dcc_update_ndl(tHalHandle hHal, void *context,
		ocb_callback callback,
		struct sir_dcc_update_ndl *request)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS sme_register_for_dcc_stats_event(tHalHandle hHal,
		void *context, ocb_callback callback)
{
	return QDF_STATUS_SUCCESS;
}
static inline QDF_STATUS sme_deregister_for_dcc_stats_event(tHalHandle hHal)
{
	return QDF_STATUS_SUCCESS;
}
#endif
void sme_add_set_thermal_level_callback(tHalHandle hal,
		sme_set_thermal_level_callback callback);

void sme_update_tgt_services(tHalHandle hal, struct wma_tgt_services *cfg);
bool sme_validate_sap_channel_switch(tHalHandle hal,
		uint16_t sap_ch, eCsrPhyMode sap_phy_mode,
		uint8_t cc_switch_mode, uint8_t session_id);

bool sme_is_session_id_valid(tHalHandle hal, uint32_t session_id);

#ifdef FEATURE_WLAN_TDLS
void sme_get_opclass(tHalHandle hal, uint8_t channel, uint8_t bw_offset,
		uint8_t *opclass);
#else
static inline void
sme_get_opclass(tHalHandle hal, uint8_t channel, uint8_t bw_offset,
		uint8_t *opclass)
{
}
#endif

#ifdef FEATURE_LFR_SUBNET_DETECTION
QDF_STATUS sme_gateway_param_update(tHalHandle hHal,
				struct gateway_param_update_req *request);
#endif

void sme_update_fine_time_measurement_capab(tHalHandle hal, uint8_t session_id,
								uint32_t val);
QDF_STATUS sme_ht40_stop_obss_scan(tHalHandle hHal, uint32_t vdev_id);
QDF_STATUS sme_set_fw_test(struct set_fwtest_params *fw_test);
QDF_STATUS sme_set_tsfcb(tHalHandle hHal,
	int (*cb_fn)(void *cb_ctx, struct stsf *ptsf), void *cb_ctx);

QDF_STATUS sme_reset_tsfcb(tHalHandle h_hal);

#ifdef WLAN_FEATURE_TSF
QDF_STATUS sme_set_tsf_gpio(tHalHandle h_hal, uint32_t pinvalue);
QDF_STATUS sme_reset_tsf_gpio(tHalHandle h_hal);

#else
static inline QDF_STATUS sme_set_tsf_gpio(tHalHandle h_hal, uint32_t pinvalue)
{
	return QDF_STATUS_E_FAILURE;
}
static inline QDF_STATUS sme_reset_tsf_gpio(tHalHandle h_hal)
{
	return QDF_STATUS_E_FAILURE;
}

#endif

QDF_STATUS sme_update_mimo_power_save(tHalHandle hHal,
				      uint8_t is_ht_smps_enabled,
				      uint8_t ht_smps_mode,
				      bool send_smps_action);

bool sme_is_sta_smps_allowed(tHalHandle hHal, uint8_t session_id);
QDF_STATUS sme_add_beacon_filter(tHalHandle hal,
				uint32_t session_id, uint32_t *ie_map);
QDF_STATUS sme_remove_beacon_filter(tHalHandle hal, uint32_t session_id);

/**
 * sme_get_bpf_offload_capabilities() - Get BPF offload capabilities
 * @hal: Global HAL handle
 * @callback: Callback function to be called with the result
 * @context: Opaque context to be used by the caller to associate the
 *   request with the response
 *
 * This function constructs the cds message and fill in message type,
 * post the same to WDA.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_get_bpf_offload_capabilities(tHalHandle hal,
					    bpf_get_offload_cb callback,
					    void *context);

QDF_STATUS sme_set_bpf_instructions(tHalHandle hal,
				struct sir_bpf_set_offload *);
uint32_t sme_get_wni_dot11_mode(tHalHandle hal);
QDF_STATUS sme_create_mon_session(tHalHandle hal_handle, uint8_t *bssid);
QDF_STATUS sme_set_adaptive_dwelltime_config(tHalHandle hal,
			struct adaptive_dwelltime_params *dwelltime_params);

void sme_set_vdev_ies_per_band(tHalHandle hal, uint8_t vdev_id);
void sme_set_pdev_ht_vht_ies(tHalHandle hHal, bool enable2x2);

void sme_update_vdev_type_nss(tHalHandle hal, uint8_t max_supp_nss,
		uint32_t vdev_type_nss, enum band_info band);
void sme_update_hw_dbs_capable(tHalHandle hal, uint8_t hw_dbs_capable);
void sme_register_p2p_lo_event(tHalHandle hHal, void *context,
					p2p_lo_callback callback);

QDF_STATUS sme_remove_bssid_from_scan_list(tHalHandle hal,
	tSirMacAddr bssid);

QDF_STATUS sme_process_mac_pwr_dbg_cmd(tHalHandle hal, uint32_t session_id,
				       struct sir_mac_pwr_dbg_cmd*
				       dbg_args);

void sme_get_vdev_type_nss(enum QDF_OPMODE dev_mode,
			   uint8_t *nss_2g, uint8_t *nss_5g);
QDF_STATUS sme_roam_set_default_key_index(tHalHandle hal, uint8_t session_id,
					  uint8_t default_idx);
void sme_send_disassoc_req_frame(tHalHandle hal, uint8_t session_id, uint8_t
				*peer_mac, uint16_t reason, uint8_t
				wait_for_ack);
QDF_STATUS sme_update_access_policy_vendor_ie(tHalHandle hal,
					uint8_t session_id, uint8_t *vendor_ie,
					int access_policy);

/**
 * sme_set_peer_param() - set peer param
 * @vdev_id: vdev ID
 * @peer_addr: peer MAC address
 * @param_id: param ID to be updated
 * @param_Value: paraam value
 *
 * This SME API is used to send the peer param to WMA to be sent to FW.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_peer_param(uint8_t *peer_addr, uint32_t param_id,
			      uint32_t param_value, uint32_t vdev_id);

QDF_STATUS sme_update_sta_roam_policy(tHalHandle hal,
		enum sta_roam_policy_dfs_mode dfs_mode,
		bool skip_unsafe_channels,
		uint8_t session_id, uint8_t sap_operating_band);
QDF_STATUS sme_enable_disable_chanavoidind_event(tHalHandle hal,
					uint8_t set_value);
QDF_STATUS sme_set_default_scan_ie(tHalHandle hal, uint16_t session_id,
				uint8_t *ie_data, uint16_t ie_len);
/**
 * sme_update_session_param() - API to update PE session param
 * @hal: HAL handle for device
 * @session_id: Session ID
 * @param_type: Param type to be updated
 * @param_val: Param value to be update
 *
 * Note: this setting will not persist over reboots.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_update_session_param(tHalHandle hal, uint8_t session_id,
		uint32_t param_type, uint32_t param_val);

/**
 * sme_update_fils_setting() - API to update PE FILS setting
 * @hal: HAL handle for device
 * @session_id: Session ID
 * @param_val: Param value to be update
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_update_fils_setting(tHalHandle hal, uint8_t session_id,
				   uint8_t param_val);
#ifdef WLAN_FEATURE_FIPS
/**
 * sme_fips_request() - Perform a FIPS certification operation
 * @hal: Hal handle for the object being certified
 * @param: The FIPS certification parameters
 * @callback: Callback function to invoke with the results
 * @context: Opaque context to pass back to caller in the callback
 *
 * Return: QDF_STATUS_SUCCESS if the request is successfully sent
 * to firmware for processing, otherwise an error status.
 */
QDF_STATUS sme_fips_request(tHalHandle hal, struct fips_params *param,
			    wma_fips_cb callback, void *context);
#else
static inline
QDF_STATUS sme_fips_request(tHalHandle hal, struct fips_params *param,
			    wma_fips_cb callback, void *context)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif /* WLAN_FEATURE_FIPS */

/**
 * sme_set_cts2self_for_p2p_go() - sme function to set ini parms to FW.
 * @hal:                    reference to the HAL
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_cts2self_for_p2p_go(tHalHandle hal);
void sme_set_prefer_80MHz_over_160MHz(tHalHandle hal,
		bool sta_prefer_80MHz_over_160MHz);
QDF_STATUS sme_update_tx_fail_cnt_threshold(tHalHandle hal_handle,
		uint8_t session_id, uint32_t tx_fail_count);
QDF_STATUS sme_update_short_retry_limit_threshold(tHalHandle hal_handle,
		struct sme_short_retry_limit *short_retry_limit_th);
QDF_STATUS sme_update_long_retry_limit_threshold(tHalHandle hal_handle,
		struct sme_long_retry_limit  *long_retry_limit_th);
/**
 * sme_roam_is_ese_assoc() - Check if association type is ESE
 * @roam_info: Pointer to roam info
 *
 * Return: true if ESE Association, false otherwise.
 */
#ifdef FEATURE_WLAN_ESE
bool sme_roam_is_ese_assoc(struct csr_roam_info *roam_info);
#else
static inline bool sme_roam_is_ese_assoc(struct csr_roam_info *roam_info)
{
	return false;
}
#endif
/**
 * sme_neighbor_roam_is11r_assoc() - Check if association type is 11R
 * @hal_ctx: HAL handle
 * @session_id: session id
 *
 * Return: true if 11r Association, false otherwise.
 */
bool sme_neighbor_roam_is11r_assoc(tHalHandle hal_ctx, uint8_t session_id);

/**
 * sme_update_sta_inactivity_timeout(): Update sta_inactivity_timeout to FW
 * @hal: Handle returned by mac_open
 * @sta_inactivity_timer:  struct for sta inactivity timer
 *
 * If a station does not send anything in sta_inactivity_timeout seconds, an
 * empty data frame is sent to it in order to verify whether it is
 * still in range. If this frame is not ACKed, the station will be
 * disassociated and then deauthenticated.
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure.
*/
QDF_STATUS sme_update_sta_inactivity_timeout(tHalHandle hal_handle,
		struct sme_sta_inactivity_timeout  *sta_inactivity_timer);

/**
 * sme_set_lost_link_info_cb() - plug in callback function for receiving
 * @hal: HAL handle
 * @cb: callback function
 *
 * Return: HAL status
 */
QDF_STATUS sme_set_lost_link_info_cb(tHalHandle hal,
		void (*cb)(void *, struct sir_lost_link_info *));

/**
 * sme_update_new_channel_event() - update new channel event for sapFsm
 * @hal: HAL handle
 * @session_id: session id
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure.
 */
QDF_STATUS sme_update_new_channel_event(tHalHandle hal, uint8_t session_id);
#ifdef WLAN_POWER_DEBUGFS
QDF_STATUS sme_power_debug_stats_req(tHalHandle hal, void (*callback_fn)
				(struct  power_stats_response *response,
				void *context), void *power_stats_context);
#endif

/**
 * sme_get_sar_power_limits() - get SAR limits
 * @hal: HAL handle
 * @callback: Callback function to invoke with the results
 * @context: Opaque context to pass back to caller in the callback
 *
 * Return: QDF_STATUS_SUCCESS if the request is successfully sent
 * to firmware for processing, otherwise an error status.
 */
QDF_STATUS sme_get_sar_power_limits(tHalHandle hal,
				    wma_sar_cb callback, void *context);

/**
 * sme_set_sar_power_limits() - set sar limits
 * @hal: HAL handle
 * @sar_limit_cmd: struct to send sar limit cmd.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_set_sar_power_limits(tHalHandle hal,
		struct sar_limit_cmd_params *sar_limit_cmd);

void sme_set_cc_src(tHalHandle hal_handle, enum country_src);


#ifdef WLAN_FEATURE_WOW_PULSE
QDF_STATUS sme_set_wow_pulse(struct wow_pulse_mode *wow_pulse_set_info);
#endif

/* ARP DEBUG STATS */
QDF_STATUS sme_set_nud_debug_stats(tHalHandle hal,
				   struct set_arp_stats_params
				   *set_stats_param);
QDF_STATUS sme_get_nud_debug_stats(tHalHandle hal,
				   struct get_arp_stats_params
				   *get_stats_param);
QDF_STATUS sme_set_nud_debug_stats_cb(tHalHandle hal,
				      void (*cb)(void *, struct rsp_stats *));

/**
 * sme_set_chan_info_callback() - Register chan info callback
 * @hal - MAC global handle
 * @callback_routine - callback routine from HDD
 *
 * This API is invoked by HDD to register its callback to mac
 *
 * Return: QDF_STATUS
 */
void sme_set_chan_info_callback(tHalHandle hal_handle,
			void (*callback)(struct scan_chan_info *chan_info));

/**
 * sme_get_rssi_snr_by_bssid() - gets the rssi and snr by bssid from scan cache
 * @hal: handle returned by mac_open
 * @profile: current connected profile
 * @bssid: bssid to look for in scan cache
 * @rssi: rssi value found
 * @snr: snr value found
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_rssi_snr_by_bssid(tHalHandle hal, tCsrRoamProfile *profile,
				     const uint8_t *bssid, int8_t *rssi,
				     int8_t *snr);

/**
 * sme_get_beacon_frm() - gets the bss descriptor from scan cache and prepares
 * beacon frame
 * @hal: handle returned by mac_open
 * @profile: current connected profile
 * @bssid: bssid to look for in scan cache
 * @frame_buf: frame buffer to populate
 * @frame_len: length of constructed frame
 * @channel: Pointer to channel info to be filled
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_beacon_frm(tHalHandle hal, tCsrRoamProfile *profile,
			    const tSirMacAddr bssid,
			    uint8_t **frame_buf, uint32_t *frame_len,
			    int *channel);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * sme_fast_reassoc() - invokes FAST REASSOC command
 * @hal: handle returned by mac_open
 * @profile: current connected profile
 * @bssid: bssid to look for in scan cache
 * @channel: channel on which reassoc should be send
 * @vdev_id: vdev id
 * @connected_bssid: bssid of currently connected profile
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_fast_reassoc(tHalHandle hal, tCsrRoamProfile *profile,
			    const tSirMacAddr bssid, int channel,
			    uint8_t vdev_id, const tSirMacAddr connected_bssid);
#endif
/**
 * sme_congestion_register_callback(): registers congestion callback
 * @hal: handler for HAL
 * @congestion_cb: congestion callback
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_congestion_register_callback(tHalHandle hal,
	void (*congestion_cb)(void *, uint32_t congestion, uint32_t vdev_id));

/**
 * sme_register_tx_queue_cb(): Register tx queue callback
 * @hal: handler for HAL
 * @tx_queue_cb: Transmit Queues callback
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_register_tx_queue_cb(tHalHandle hal,
				    void (*tx_queue_cb)(void *,
				    uint32_t vdev_id,
				    enum netif_action_type action,
				    enum netif_reason_type reason));

/**
 * sme_deregister_tx_queue_cb() - Deregister the tx queue callback
 * @hal: HAL handle
 *
 * Return: QDF status
 */
QDF_STATUS sme_deregister_tx_queue_cb(tHalHandle hal);

/**
 * sme_rso_cmd_status_cb() - Set RSO cmd status callback
 * @hal: HAL Handle
 * @cb: HDD Callback to rso comman status read
 *
 * This function is used to save HDD RSO Command status callback in MAC
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_rso_cmd_status_cb(tHalHandle hal,
		void (*cb)(void *, struct rso_cmd_status *));

/**
 * sme_register_set_connection_info_cb() - Register connection
 * info callback
 * @hal - MAC global handle
 * @set_connection_info_cb - callback routine from HDD to set
 *                   connection info flag
 * @get_connection_info_cb - callback routine from HDD to get
 *                         connection info
 *
 * This API is invoked by HDD to register its callback to mac
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_register_set_connection_info_cb(tHalHandle hHal,
				bool (*set_connection_info_cb)(bool),
				bool (*get_connection_info_cb)(uint8_t *session_id,
				enum scan_reject_states *reason));

/**
 * sme_set_dbs_scan_selection_config() - Update DBS scan selection
 * configuration
 * @hal: The handle returned by macOpen
 * @params: wmi_dbs_scan_sel_params config
 *
 * Return: QDF_STATUS if DBS scan selection update
 * configuration success else failure status
 */
QDF_STATUS sme_set_dbs_scan_selection_config(tHalHandle hal,
		struct wmi_dbs_scan_sel_params *params);

/**
 * sme_store_pdev() - store pdev
 * @hal - MAC global handle
 * @pdev - pdev ptr
 *
 * Return: QDF_STATUS
 */
void sme_store_pdev(tHalHandle hal, struct wlan_objmgr_pdev *pdev);

/**
 * sme_ipa_uc_stat_request() - set ipa config parameters
 * @vdev_id: virtual device for the command
 * @param_id: parameter id
 * @param_val: parameter value
 * @req_cat: parameter category
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_ipa_uc_stat_request(tHalHandle hal,
			uint32_t vdev_id, uint32_t param_id,
			uint32_t param_val, uint32_t req_cat);

/**
 * sme_set_reorder_timeout() - set reorder timeout value
 * including Voice,Video,Besteffort,Background parameters
 * @hal: hal handle for getting global mac struct
 * @reg: struct sir_set_rx_reorder_timeout_val
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure.
 */
QDF_STATUS sme_set_reorder_timeout(tHalHandle hal,
		struct sir_set_rx_reorder_timeout_val *req);

/**
 * sme_set_rx_set_blocksize() - set blocksize value
 * including mac_addr and win_limit parameters
 * @hal: hal handle for getting global mac struct
 * @reg: struct sir_peer_set_rx_blocksize
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure.
 */

QDF_STATUS sme_set_rx_set_blocksize(tHalHandle hal,
		struct sir_peer_set_rx_blocksize *req);

/**
 * sme_get_rcpi() - gets the rcpi value for peer mac addr
 * @hal: handle returned by mac_open
 * @rcpi: rcpi request containing peer mac addr, callback and related info
 *
 * This function posts the rcpi measurement request message to wma queue
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_rcpi(tHalHandle hal, struct sme_rcpi_req *rcpi);

/*
 * sme_set_chip_pwr_save_fail_cb() - set chip power save failure callback
 * @hal: global hal handle
 * @cb: callback function pointer
 *
 * This function stores the chip power save failure callback function.
 *
 * Return: QDF_STATUS enumeration.
 */

QDF_STATUS sme_set_chip_pwr_save_fail_cb(tHalHandle hal, void (*cb)(void *,
				 struct chip_pwr_save_fail_detected_params *));
/**
 * sme_cli_set_command() - SME wrapper API over WMA "set" command
 * processor cmd
 * @vdev_id: virtual device for the command
 * @param_id: parameter id
 * @sval: parameter value
 * @vpdev: parameter category
 *
 * Command handler for set operations
 *
 * Return: 0 on success, errno on failure
 */
int sme_cli_set_command(int vdev_id, int param_id, int sval, int vpdev);

/**
 * sme_set_bt_activity_info_cb - set the callback handler for bt events
 * @hal: handle returned by mac_open
 * @cb: callback handler
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_bt_activity_info_cb(tHalHandle hal,
				void (*cb)(void *, uint32_t profile_info));

/**
 * sme_set_smps_cfg() - set SMPS config params
 * @vdev_id: virtual device for the command
 * @param_id: parameter id
 * @param_val: parameter value
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */

QDF_STATUS sme_set_smps_cfg(uint32_t vdev_id, uint32_t param_id,
				uint32_t param_val);

/**
 * sme_get_peer_stats() - sme api to post peer info request
 * @mac: mac handle
 * @req: peer info request struct send to wma
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */

QDF_STATUS sme_get_peer_stats(tpAniSirGlobal mac,
			      struct sir_peer_info_req req);

/**
 * sme_get_peer_info() - sme api to get peer info
 * @hal: hal handle for getting global mac struct
 * @req: peer info request struct send to wma
 * @context: context of callback function
 * @callbackfn: hdd callback function when receive response
 *
 * This function will send WMA_GET_PEER_INFO to WMA
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_get_peer_info(tHalHandle hal,
		struct sir_peer_info_req req,
		void *context,
		void (*callbackfn)(struct sir_peer_info_resp *param,
			void *pcontext));

/**
 * sme_get_peer_info_ext() - sme api to get peer ext info
 * @hal: hal handle for getting global mac struct
 * @req: peer ext info request struct send to wma
 * @context: context of callback function
 * @callbackfn: hdd callback function when receive response
 *
 * This function will send WMA_GET_PEER_INFO_EXT to WMA
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_get_peer_info_ext(tHalHandle hal,
		struct sir_peer_info_ext_req *req,
		void *context,
		void (*callbackfn)(struct sir_peer_info_ext_resp *param,
			void *pcontext));

/**
 * sme_set_5g_band_pref() - sme api to set 5Ghz preference
 * @hal: hal handle for getting global mac struct
 * @pref_params: preference info request struct
 *
 * This function will set 5Ghz preference for STA connection
 *
 * Return: None
 */
void sme_set_5g_band_pref(tHalHandle hal_handle,
			  struct sme_5g_band_pref_params *pref_params);
/**
 * sme_get_chain_rssi() - Get chain rssi
 * @hal: Global HAL handle
 * @input: get chain rssi req params
 * @callback: Callback function to be called with the result
 * @context: Opaque context to be used by the caller to associate the
 *   request with the response
 *
 * This function constructs the cds message and fill in message type,
 * post the same to WDA.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_get_chain_rssi(tHalHandle hal,
			      struct get_chain_rssi_req_params *input,
			      get_chain_rssi_callback callback,
			      void *context);

/**
 * sme_get_valid_channels() - sme api to get valid channels for
 * current regulatory domain
 * @chan_list: list of the valid channels
 * @list_len: length of the channel list
 *
 * This function will get valid channels for current regulatory
 * domain
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_get_valid_channels(uint8_t *chan_list, uint32_t *list_len);

/**
 * sme_get_mac_context() - sme api to get the pmac context
 *
 * This function will return the pmac context
 *
 * Return: pointer to pmac context
 */
tpAniSirGlobal sme_get_mac_context(void);

/**
 * sme_display_disconnect_stats() - Display per session Disconnect stats
 * @hal: hal global context
 * session_id: SME session id
 *
 * Return: None
 */
void sme_display_disconnect_stats(tHalHandle hal, uint8_t session_id);

/**
 * sme_process_msg_callback() - process callback message from LIM
 * @hal: global hal handle
 * @msg: scheduler message
 *
 * This function process the callback messages from LIM.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_process_msg_callback(tHalHandle hal,
				struct scheduler_msg *msg);

/**
 * sme_set_bmiss_bcnt() - set bmiss config parameters
 * @vdev_id: virtual device for the command
 * @first_cnt: bmiss first value
 * @final_cnt: bmiss final value
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_set_bmiss_bcnt(uint32_t vdev_id, uint32_t first_cnt,
		uint32_t final_cnt);

/**
 * sme_send_limit_off_channel_params() - send limit off channel parameters
 * @hal: global hal handle
 * @vdev_id: vdev id
 * @is_tos_active: tos active or inactive
 * @max_off_chan_time: max off channel time
 * @rest_time: rest time
 * @skip_dfs_chan: skip dfs channel
 *
 * This function sends command to WMA for setting limit off channel command
 * parameters.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_send_limit_off_channel_params(tHalHandle hal, uint8_t vdev_id,
		bool is_tos_active, uint32_t max_off_chan_time,
		uint32_t rest_time, bool skip_dfs_chan);

/**
 * sme_set_vc_mode_config() - Set voltage corner config to FW.
 * @bitmap:	Bitmap that refers to voltage corner config with
 * different phymode and bw configuration
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_vc_mode_config(uint32_t vc_bitmap);

/**
 * sme_set_del_pmkid_cache() - API to update PMKID cache
 * @hal: HAL handle for device
 * @session_id: Session id
 * @pmk_cache_info: Pointer to PMK cache info
 * @is_add: boolean that implies whether to add or delete PMKID entry
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_del_pmkid_cache(tHalHandle hal, uint8_t session_id,
				   tPmkidCacheInfo *pmk_cache_info,
				   bool is_add);

/**
 * sme_send_hlp_ie_info() - API to send HLP IE info to fw
 * @hal: HAL handle for device
 * @session_id: Session id
 * @profile: CSR Roam profile
 * @if_addr: IP address
 *
 * This API is used to send HLP IE info along with IP address
 * to fw if LFR3 is enabled.
 *
 * Return: QDF_STATUS
 */
void sme_send_hlp_ie_info(tHalHandle hal, uint8_t session_id,
			  tCsrRoamProfile *profile, uint32_t if_addr);

#if defined(WLAN_FEATURE_FILS_SK)
/**
 * sme_update_fils_config - Update FILS config to CSR roam session
 * @hal: HAL handle for device
 * @session_id: session id
 * @src_profile: Source profile having latest FILS config
 *
 * API to update FILS config to roam csr session and update the same
 * to fw if LFR3 is enabled.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_update_fils_config(tHalHandle hal, uint8_t session_id,
				tCsrRoamProfile *src_profile);

/**
 * sme_free_join_rsp_fils_params - free fils params
 * @roam_info: roam info
 *
 * Return: void
 */
void sme_free_join_rsp_fils_params(struct csr_roam_info *roam_info);
#else
static inline QDF_STATUS sme_update_fils_config(tHalHandle hal,
				uint8_t session_id,
				tCsrRoamProfile *src_profile)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void sme_free_join_rsp_fils_params(struct csr_roam_info *roam_info)
{}

#endif

#ifdef WLAN_FEATURE_11AX_BSS_COLOR
/**
 * sme_set_he_bss_color() - Sets the HE BSS color
 *
 * @hal: The handle returned by mac_open
 * @session_id: session_id of the request
 * @bss_color: HE BSS color value to set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_he_bss_color(tHalHandle hal, uint8_t session_id,
		uint8_t bss_color);
#else
static inline QDF_STATUS sme_set_he_bss_color(tHalHandle hal,
		uint8_t session_id, uint8_t bss_color)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * sme_scan_get_result_for_bssid - gets the scan result from scan cache for the
 * bssid specified
 * @hal: handle returned by mac_open
 * @bssid: bssid to get the scan result for
 * @res: pointer to tCsrScanResultInfo allocated from caller
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_scan_get_result_for_bssid(tHalHandle hal_handle,
					 struct qdf_mac_addr *bssid,
					 tCsrScanResultInfo *res);

/**
 * sme_get_bss_transition_status() - get bss transition status all cadidates
 * @hal: handle returned by mac_open
 * @transition_reason : Transition reason
 * @bssid: bssid to get BSS transition status
 * @info : bss candidate information
 * @n_candidates : number of candidates
 * @is_bt_in_progress: bt activity indicator
 *
 * Return : 0 on success otherwise errno
 */
int sme_get_bss_transition_status(tHalHandle hal,
		uint8_t transition_reason,
		struct qdf_mac_addr *bssid,
		struct bss_candidate_info *info,
		uint16_t n_candidates,
		bool is_bt_in_progress);

/**
 * sme_unpack_rsn_ie: wrapper to unpack RSN IE and update def RSN params
 * if optional fields are not present.
 * @hal: handle returned by mac_open
 * @buf: rsn ie buffer pointer
 * @buf_len: rsn ie buffer length
 * @rsn_ie: outframe rsn ie structure
 * @append_ie: flag to indicate if the rsn_ie need to be appended from buf
 *
 * Return: parse status
 */
uint32_t sme_unpack_rsn_ie(tHalHandle hal, uint8_t *buf,
				  uint8_t buf_len, tDot11fIERSN *rsn_ie,
				  bool append_ie);
/**
 * sme_get_oper_chan_freq - gets the operating channel freq
 * @vdev: vdev handle
 *
 * Return: operating channel frequency
 */
int16_t sme_get_oper_chan_freq(struct wlan_objmgr_vdev *vdev);

/**
 * sme_get_oper_ch_width - gets the operating channel width
 * @vdev: vdev handle
 *
 * Return: operating channel width
 */
enum phy_ch_width sme_get_oper_ch_width(struct wlan_objmgr_vdev *vdev);

/**
 * sme_get_oper_ch_width - gets the secondary channel frequency
 * @vdev: vdev handle
 * @sec20chan_freq: secondary channel frequency
 *
 * Return: secondary channel frequency
 */
int sme_get_sec20chan_freq_mhz(struct wlan_objmgr_vdev *vdev,
						uint16_t *sec20chan_freq);

/**
 * sme_enable_roaming_on_connected_sta() - Enable roaming on an connected sta
 * @hal: handle returned by mac_open
 *
 * The function check if any connected STA is present on which roaming is not
 * enabled and if present enabled roaming on that STA.
 *
 * Return: none
 */
void sme_enable_roaming_on_connected_sta(tHalHandle hal);

/**
 * sme_send_mgmt_tx() - Sends mgmt frame from CSR to LIM
 * @hal: The handle returned by mac_open
 * @session_id: session id
 * @buf: pointer to frame
 * @len: frame length
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_send_mgmt_tx(tHalHandle hal, uint8_t session_id,
			   const uint8_t *buf, uint32_t len);

#ifdef WLAN_FEATURE_SAE
/**
 * sme_handle_sae_msg() - Sends SAE message received from supplicant
 * @hal: The handle returned by mac_open
 * @session_id: session id
 * @sae_status: status of SAE authentication
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_handle_sae_msg(tHalHandle hal, uint8_t session_id,
		uint8_t sae_status);
#else
static inline QDF_STATUS sme_handle_sae_msg(tHalHandle hal, uint8_t session_id,
		uint8_t sae_status)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_11AX
/**
 * sme_update_he_cap_nss() - sets the nss based on user request
 * @hal: Pointer to HAL
 * @session_id: SME session id
 * @nss: no.of spatial streams value
 *
 * Return: None
 */
void sme_update_he_cap_nss(tHalHandle hal, uint8_t session_id,
		uint8_t nss);

/**
 * sme_update_he_mcs() - sets the HE MCS based on user request
 * @hal: Pointer to HAL
 * @session_id: SME session id
 * @he_mcs: HE MCS value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_mcs(tHalHandle hal, uint8_t session_id, uint16_t he_mcs);
/**
 * sme_update_he_tx_stbc_cap() - Sets the HE Tx STBC capability
 * @hal: Pointer to HAL
 * @session_id: SME session id
 * @value: set value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_tx_stbc_cap(tHalHandle hal, uint8_t session_id, int value);

/**
 * sme_update_he_rx_stbc_cap() - Sets the HE Rx STBC capability
 * @hal: Pointer to HAL
 * @session_id: SME session id
 * @value: set value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_rx_stbc_cap(tHalHandle hal, uint8_t session_id, int value);
#else
static inline void sme_update_he_cap_nss(tHalHandle hal, uint8_t session_id,
		uint8_t nss)
{}
static inline int sme_update_he_mcs(tHalHandle hal, uint8_t session_id,
				    uint16_t he_mcs)
{
	return 0;
}
static inline int sme_update_he_tx_stbc_cap(tHalHandle hal, uint8_t session_id,
					    int value)
{
	return 0;
}

static inline int sme_update_he_rx_stbc_cap(tHalHandle hal, uint8_t session_id,
					    int value)
{
	return 0;
}
#endif

/**
 * sme_is_sta_key_exchange_in_progress() - checks whether the STA/P2P client
 * session has key exchange in progress
 *
 * @hal: global hal handle
 * @session_id: session id
 *
 * Return: true - if key exchange in progress
 *         false - if not in progress
 */
bool sme_is_sta_key_exchange_in_progress(tHalHandle hal, uint8_t session_id);

#endif /* #if !defined( __SME_API_H ) */
