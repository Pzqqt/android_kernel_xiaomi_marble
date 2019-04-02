/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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
#include "wlan_mlme_public_struct.h"
#include "sme_rrm_internal.h"
#include "sir_types.h"
#include "scheduler_api.h"
#include "wlan_serialization_legacy_api.h"
#include <qca_vendor.h>
#include "wmi_unified.h"

/*--------------------------------------------------------------------------
  Preprocessor definitions and constants
  ------------------------------------------------------------------------*/

#define SME_SUMMARY_STATS         (1 << eCsrSummaryStats)
#define SME_GLOBAL_CLASSA_STATS   (1 << eCsrGlobalClassAStats)
#define SME_GLOBAL_CLASSD_STATS   (1 << eCsrGlobalClassDStats)
#define SME_PER_CHAIN_RSSI_STATS  (1 << csr_per_chain_rssi_stats)

#define sme_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_SME, params)
#define sme_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_SME, params)
#define sme_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_SME, params)
#define sme_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_SME, params)
#define sme_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_SME, params)

#define sme_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_SME, params)
#define sme_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_SME, params)
#define sme_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_SME, params)
#define sme_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_SME, params)
#define sme_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_SME, params)

#define sme_alert_rl(params...) QDF_TRACE_FATAL_RL(QDF_MODULE_ID_SME, params)
#define sme_err_rl(params...) QDF_TRACE_ERROR_RL(QDF_MODULE_ID_SME, params)
#define sme_warn_rl(params...) QDF_TRACE_WARN_RL(QDF_MODULE_ID_SME, params)
#define sme_info_rl(params...) QDF_TRACE_INFO_RL(QDF_MODULE_ID_SME, params)
#define sme_debug_rl(params...) QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_SME, params)

#define SME_ENTER() QDF_TRACE_ENTER(QDF_MODULE_ID_SME, "enter")
#define SME_EXIT() QDF_TRACE_EXIT(QDF_MODULE_ID_SME, "exit")

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

#define SUPPORTED_CRYPTO_CAPS 0x3FFFF

#define SME_ACTIVE_LIST_CMD_TIMEOUT_VALUE (30*1000)
#define SME_CMD_TIMEOUT_VALUE (SME_ACTIVE_LIST_CMD_TIMEOUT_VALUE + 1000)

#define SME_CMD_VDEV_DISCONNECT_TIMEOUT (SIR_VDEV_STOP_REQUEST_TIMEOUT + 4000)
#define SME_DISCONNECT_TIMEOUT (SME_CMD_VDEV_DISCONNECT_TIMEOUT + 1000)

#define SME_CMD_VDEV_START_BSS_TIMEOUT (SIR_VDEV_START_REQUEST_TIMEOUT + 4000)
#define SME_CMD_START_BSS_TIMEOUT (SME_CMD_VDEV_START_BSS_TIMEOUT + 1000)

/* SME timeout for Start/Stop BSS commands is set to 6 secs */
#define SME_START_STOP_BSS_CMD_TIMEOUT (SIR_VDEV_STOP_REQUEST_TIMEOUT + 4000)
#define SME_CMD_STOP_BSS_TIMEOUT (SME_START_STOP_BSS_CMD_TIMEOUT + 4000)

#define SME_CMD_PEER_DISCONNECT_TIMEOUT (SIR_DELETE_STA_TIMEOUT + 2000)
#define SME_PEER_DISCONNECT_TIMEOUT (SME_START_STOP_BSS_CMD_TIMEOUT + 1000)

#define SME_CMD_ROAM_CMD_TIMEOUT (SIR_VDEV_START_REQUEST_TIMEOUT + 4000)
#define SME_CMD_ADD_DEL_TS_TIMEOUT (4 * 1000)

#define SME_CMD_POLICY_MGR_CMD_TIMEOUT (SIR_VDEV_PLCY_MGR_TIMEOUT + 2000)
#define SME_POLICY_MGR_CMD_TIMEOUT (SME_CMD_POLICY_MGR_CMD_TIMEOUT + 1000)

#define SME_VDEV_DELETE_CMD_TIMEOUT (6 * 1000)
#define SME_CMD_VDEV_CREATE_DELETE_TIMEOUT (SME_VDEV_DELETE_CMD_TIMEOUT + 4000)

/*--------------------------------------------------------------------------
  Type declarations
  ------------------------------------------------------------------------*/
struct sme_config_params {
	struct csr_config_params csr_config;
};

#ifdef FEATURE_WLAN_TDLS
#define BW_20_OFFSET_BIT   0
#define BW_40_OFFSET_BIT   1
#define BW_80_OFFSET_BIT   2
#define BW_160_OFFSET_BIT  3
#endif /* FEATURE_WLAN_TDLS */

/* Thermal Mitigation*/
typedef struct {
	uint16_t smeMinTempThreshold;
	uint16_t smeMaxTempThreshold;
} tSmeThermalLevelInfo;

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
 * @callback: callback to be invoked for roaming events
 * @callback_ctx: user-supplied context to be passed back on roaming events
 * @self_mac_addr: Self mac address
 * @sme_session_id: SME session id
 * @type_of_persona: person type
 * @subtype_of_persona: sub type of persona
 */
struct sme_session_params {
	csr_session_open_cb  session_open_cb;
	csr_session_close_cb session_close_cb;
	csr_roam_complete_cb callback;
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
QDF_STATUS sme_open(mac_handle_t mac_handle);
QDF_STATUS sme_init_chan_list(mac_handle_t mac_handle, uint8_t *alpha2,
		enum country_src cc_src);
QDF_STATUS sme_close(mac_handle_t mac_handle);
QDF_STATUS sme_start(mac_handle_t mac_handle);

/**
 * sme_stop() - Stop all SME modules and put them at idle state
 * @mac_handle: Opaque handle to the MAC context
 *
 * The function stops each module in SME. Upon return, all modules are
 * at idle state ready to start.
 *
 * This is a synchronous call
 *
 * Return: QDF_STATUS_SUCCESS if SME is stopped.  Other status means
 *         SME failed to stop one or more modules but caller should
 *         still consider SME is stopped.
 */
QDF_STATUS sme_stop(mac_handle_t mac_handle);

/**
 * sme_populate_nss_chain_params() - fill vdev nss chain params from ini
 * @mac_handle: The handle returned by mac_open.
 * @vdev_ini_cfg: pointer to the structure to be filled
 * @device_mode: device mode (eg STA, SAP etc.)
 * @rf_chains_supported: number of chains supported by fw(updated during
 *                       service ready event)
 *
 * This API will fill the nss chain params for the particular vdev from ini
 * configuration for the respective vdev.
 *
 * Return: none
 */
void sme_populate_nss_chain_params(mac_handle_t mac_handle,
			     struct wlan_mlme_nss_chains *vdev_ini_cfg,
			     enum QDF_OPMODE device_mode,
			     uint8_t rf_chains_supported);

/**
 * sme_store_nss_chains_cfg_in_vdev() - fill vdev nss chain params from ini
 * @vdev: Pointer to vdev obj
 * @vdev_ini_cfg: pointer to the structure the values are to be filled from
 *
 * This API will copy the nss chain params for the particular vdev from ini
 * configuration to the respective vdev's dynamic, and ini config.
 *
 * Return: none
 */
void
sme_store_nss_chains_cfg_in_vdev(struct wlan_objmgr_vdev *vdev,
				 struct wlan_mlme_nss_chains *vdev_ini_cfg);

/**
 * sme_modify_nss_chains_tgt_cfg() - Change the nss in ini for
 * particular opmode, and band, according to the chain config supported by FW.
 * @mac_handle: The handle returned by mac_open.
 * @vdev_op_mode: vdev operation mode.
 * @band:- band for which user wants to change nss.
 *
 * This API will change the nss in ini (for eg. rx_nss_2g) in the mlme cfg i.e
 * the global config structure kept in mac context, according to the max
 * supported chains per band which is got as part of ext service ready event.
 *
 * Return: none
 */
void
sme_modify_nss_chains_tgt_cfg(mac_handle_t mac_handle,
			      enum QDF_OPMODE vdev_op_mode,
			      enum nss_chains_band_info band);

/**
 * sme_update_nss_in_mlme_cfg() - Change the nss in ini(rx_nss_(band)) for
 * particular opmode, and band.
 * @mac_handle: The handle returned by mac_open.
 * @rx_nss: new value of rx nss that user wants to change.
 * @tx_nss: new value of tx nss that user wants to change.
 * @vdev_op_mode: vdev operation mode.
 * @band:- band for which user wants to change nss.
 *
 * This API will change the nss in ini (for eg. rx_nss_2g) in the mlme cfg i.e
 * the global config structure kept in mac context.
 *
 * Return: none
 */
void
sme_update_nss_in_mlme_cfg(mac_handle_t mac_handle,
			   uint8_t rx_nss, uint8_t tx_nss,
			   enum QDF_OPMODE vdev_op_mode,
			   enum nss_chains_band_info band);

/**
 * sme_nss_chains_update() - validate and send the user params to fw
 * @mac_handle: The handle returned by mac_open.
 * @user_cfg: pointer to the structure to be validated and sent to fw
 * @vdev_id: vdev id
 *
 *
 * This API will validate the config, and if found correct will update the
 * config in dynamic config, and send to the fw.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_nss_chains_update(mac_handle_t mac_handle,
		      struct wlan_mlme_nss_chains *user_cfg,
		      uint8_t vdev_id);

/**
 * sme_open_session() - Open a session for given persona
 *
 * This is a synchronous API. For any protocol stack related activity
 * requires session to be opened. This API needs to be called to open
 * the session in SME module.
 *
 * mac_handle: The handle returned by mac_open.
 * params: to initialize the session open params
 *
 * Return:
 * QDF_STATUS_SUCCESS - session is opened.
 * Other status means SME is failed to open the session.
 */
QDF_STATUS sme_open_session(mac_handle_t mac_handle,
			    struct sme_session_params *params);

/**
 * sme_close_session() - Close a session for given persona
 *
 * This is a synchronous API. This API needs to be called to close the session
 * in SME module before terminating the session completely.
 *
 * mac_handle: The handle returned by mac_open.
 * session_id: A previous opened session's ID.
 *
 * Return:
 * QDF_STATUS_SUCCESS - session is closed.
 * Other status means SME is failed to open the session.
 */
QDF_STATUS sme_close_session(mac_handle_t mac_handle, uint8_t sessionId);

/**
 * sme_set_curr_device_mode() - Sets the current operating device mode.
 * @mac_handle: The handle returned by mac_open.
 * @curr_device_mode: Current operating device mode.
 */
void sme_set_curr_device_mode(mac_handle_t mac_handle,
			      enum QDF_OPMODE curr_device_mode);

QDF_STATUS sme_update_roam_params(mac_handle_t mac_handle,
				  uint8_t session_id,
				  struct roam_ext_params *roam_params_src,
				  int update_param);
QDF_STATUS sme_update_config(mac_handle_t mac_handle,
			     struct sme_config_params *pSmeConfigParams);

QDF_STATUS sme_set11dinfo(mac_handle_t mac_handle,
			  struct sme_config_params *pSmeConfigParams);
QDF_STATUS sme_hdd_ready_ind(mac_handle_t mac_handle);
/**
 * sme_ser_cmd_callback() - callback from serialization module
 * @cmd: serialization command
 * @reason: reason why serialization module has given this callback
 *
 * Serialization module will give callback to SME for why it triggered
 * the callback
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS sme_ser_cmd_callback(struct wlan_serialization_command *cmd,
				enum wlan_serialization_cb_reason reason);

/**
 * sme_purge_pdev_all_ser_cmd_list() - purge all scan and non-scan
 * active and pending cmds for pdev
 * @mac_handle: pointer to global MAC context
 *
 * Return : none
 */
void sme_purge_pdev_all_ser_cmd_list(mac_handle_t mac_handle);

/*
 * sme_process_msg() - The main message processor for SME.
 * @mac: The global mac context
 * @msg: The message to be processed.
 *
 * This function is called by a message dispatcher when to process a message
 * targeted for SME.
 * This is a synchronous call
 *
 * Return: QDF_STATUS_SUCCESS - SME successfully processed the message.
 * Other status means SME failed to process the message to HAL.
 */
QDF_STATUS sme_process_msg(struct mac_context *mac, struct scheduler_msg *pMsg);

QDF_STATUS sme_mc_process_handler(struct scheduler_msg *msg);
QDF_STATUS sme_scan_get_result(mac_handle_t mac_handle, uint8_t sessionId,
		tCsrScanResultFilter *pFilter,
		tScanResultHandle *phResult);
QDF_STATUS sme_get_ap_channel_from_scan_cache(
		struct csr_roam_profile *profile,
		tScanResultHandle *scan_cache,
		uint8_t *ap_chnl_id);
QDF_STATUS sme_get_ap_channel_from_scan(void *profile,
		tScanResultHandle *scan_cache,
		uint8_t *ap_chnl_id);
QDF_STATUS sme_scan_flush_result(mac_handle_t mac_handle);
tCsrScanResultInfo *sme_scan_result_get_first(mac_handle_t,
		tScanResultHandle hScanResult);
tCsrScanResultInfo *sme_scan_result_get_next(mac_handle_t,
		tScanResultHandle hScanResult);
QDF_STATUS sme_scan_result_purge(tScanResultHandle hScanResult);
QDF_STATUS sme_roam_connect(mac_handle_t mac_handle, uint8_t sessionId,
		struct csr_roam_profile *pProfile, uint32_t *pRoamId);
QDF_STATUS sme_roam_reassoc(mac_handle_t mac_handle, uint8_t sessionId,
		struct csr_roam_profile *pProfile,
		tCsrRoamModifyProfileFields modProfileFields,
		uint32_t *pRoamId, bool fForce);

/**
 * sme_roam_disconnect() - API to request CSR to disconnect
 * @mac_handle: Opaque handle to the global MAC context
 * @session: SME session identifier
 * @reason: Reason to disconnect
 *
 * Return: QDF Status success or failure
 */
QDF_STATUS sme_roam_disconnect(mac_handle_t mac_handle, uint8_t session,
			       eCsrRoamDisconnectReason reason);

void sme_dhcp_done_ind(mac_handle_t mac_handle, uint8_t session_id);
QDF_STATUS sme_roam_stop_bss(mac_handle_t mac_handle, uint8_t sessionId);
QDF_STATUS sme_roam_disconnect_sta(mac_handle_t mac_handle, uint8_t sessionId,
		struct csr_del_sta_params *p_del_sta_params);
QDF_STATUS sme_roam_deauth_sta(mac_handle_t mac_handle, uint8_t sessionId,
		struct csr_del_sta_params *pDelStaParams);
QDF_STATUS sme_roam_get_connect_profile(mac_handle_t mac_handle,
					uint8_t sessionId,
					tCsrRoamConnectedProfile *pProfile);
void sme_roam_free_connect_profile(tCsrRoamConnectedProfile *profile);
QDF_STATUS sme_roam_set_pmkid_cache(mac_handle_t mac_handle, uint8_t sessionId,
		tPmkidCacheInfo *pPMKIDCache,
		uint32_t numItems,
		bool update_entire_cache);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * sme_get_pmk_info(): A wrapper function to request CSR to save PMK
 * @mac_handle: Global structure
 * @session_id: SME session_id
 * @pmk_cache: pointer to a structure of pmk
 *
 * Return: none
 */
void sme_get_pmk_info(mac_handle_t mac_handle, uint8_t session_id,
		      tPmkidCacheInfo *pmk_cache);

QDF_STATUS sme_roam_set_psk_pmk(mac_handle_t mac_handle, uint8_t sessionId,
		uint8_t *pPSK_PMK, size_t pmk_len);
#else
static inline
void sme_get_pmk_info(mac_handle_t mac_handle, uint8_t session_id,
		      tPmkidCacheInfo *pmk_cache)
{}
#endif

/**
 * sme_roam_get_wpa_rsn_req_ie() - Retrieve WPA/RSN Request IE
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: ID of the specific session
 * @len: Caller allocated memory that has the length of @buf as input.
 *	Upon returned, @len has the length of the IE store in @buf
 * @buf: Caller allocated memory that contain the IE field, if any,
 *	upon return
 *
 * A wrapper function to request CSR to return the WPA or RSN IE CSR
 * passes to PE to JOIN request or START_BSS request
 * This is a synchronous call.
 *
 * Return: QDF_STATUS - when fail, it usually means the buffer allocated is not
 *			 big enough
 */
QDF_STATUS sme_roam_get_wpa_rsn_req_ie(mac_handle_t mac_handle,
				       uint8_t session_id,
				       uint32_t *len, uint8_t *buf);

/**
 * sme_roam_get_wpa_rsn_rsp_ie() - Retrieve WPA/RSN Response IE
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: ID of the specific session
 * @len: Caller allocated memory that has the length of @buf as input.
 *	Upon returned, @len has the length of the IE store in @buf
 * @buf: Caller allocated memory that contain the IE field, if any,
 *	upon return
 *
 * A wrapper function to request CSR to return the WPA or RSN IE CSR
 * passes to PE to JOIN request or START_BSS request
 * This is a synchronous call.
 *
 * Return: QDF_STATUS - when fail, it usually means the buffer allocated is not
 *			 big enough
 */
QDF_STATUS sme_roam_get_wpa_rsn_rsp_ie(mac_handle_t mac_handle,
				       uint8_t session_id,
				       uint32_t *len, uint8_t *buf);

QDF_STATUS sme_get_config_param(mac_handle_t mac_handle,
				struct sme_config_params *pParam);
#ifndef QCA_SUPPORT_CP_STATS
QDF_STATUS sme_get_statistics(mac_handle_t mac_handle,
		eCsrStatsRequesterType requesterId,
		uint32_t statsMask, tCsrStatsCallback callback,
		uint8_t staId, void *pContext, uint8_t sessionId);
#endif
QDF_STATUS sme_get_rssi(mac_handle_t mac_handle,
		tCsrRssiCallback callback,
		uint8_t staId, struct qdf_mac_addr bssId, int8_t lastRSSI,
		void *pContext);
QDF_STATUS sme_get_snr(mac_handle_t mac_handle,
		tCsrSnrCallback callback,
		uint8_t staId, struct qdf_mac_addr bssId, void *pContext);
#ifdef FEATURE_WLAN_ESE
QDF_STATUS sme_get_tsm_stats(mac_handle_t mac_handle,
		tCsrTsmStatsCallback callback,
		uint8_t staId, struct qdf_mac_addr bssId,
		void *pContext, uint8_t tid);
QDF_STATUS sme_set_cckm_ie(mac_handle_t mac_handle,
		uint8_t sessionId,
		uint8_t *pCckmIe, uint8_t cckmIeLen);
QDF_STATUS sme_set_ese_beacon_request(mac_handle_t mac_handle,
				      const uint8_t sessionId,
				      const tCsrEseBeaconReq *in_req);

/**
 * sme_set_plm_request() - set plm request
 * @mac_handle: Opaque handle to the global MAC context
 * @req: Pointer to input plm request
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_plm_request(mac_handle_t mac_handle,
			       struct plm_req_params *req);
#endif /*FEATURE_WLAN_ESE */

QDF_STATUS sme_get_modify_profile_fields(mac_handle_t mac_handle,
					 uint8_t sessionId,
					 tCsrRoamModifyProfileFields *
					 pModifyProfileFields);

#ifdef FEATURE_OEM_DATA_SUPPORT
QDF_STATUS sme_register_oem_data_rsp_callback(mac_handle_t mac_handle,
		sme_send_oem_data_rsp_msg callback);
void sme_deregister_oem_data_rsp_callback(mac_handle_t mac_handle);

#else
static inline
QDF_STATUS sme_register_oem_data_rsp_callback(mac_handle_t mac_handle,
					      void *callback)
{
	return QDF_STATUS_SUCCESS;
}

static inline
void sme_deregister_oem_data_rsp_callback(mac_handle_t mac_handle)
{
}

#endif

QDF_STATUS sme_roam_set_key(mac_handle_t mac_handle, uint8_t sessionId,
			    tCsrRoamSetKey *pSetKey, uint32_t *pRoamId);
QDF_STATUS sme_get_country_code(mac_handle_t mac_handle, uint8_t *pBuf,
				uint8_t *pbLen);

QDF_STATUS sme_generic_change_country_code(mac_handle_t mac_handle,
					   uint8_t *pCountry);


/**
 * sme_update_channel_list() - Update configured channel list to fwr
 * This is a synchronous API.
 * @mac_handle: Opaque handle to the global MAC context.
 *
 * Return: QDF_STATUS  SUCCESS.
 * FAILURE or RESOURCES  The API finished and failed.
 */
QDF_STATUS sme_update_channel_list(mac_handle_t mac_handle);

QDF_STATUS sme_tx_fail_monitor_start_stop_ind(mac_handle_t mac_handle,
		uint8_t tx_fail_count,
		void *txFailIndCallback);
QDF_STATUS sme_dhcp_start_ind(mac_handle_t mac_handle,
		uint8_t device_mode,
		uint8_t *macAddr, uint8_t sessionId);
QDF_STATUS sme_dhcp_stop_ind(mac_handle_t mac_handle,
		uint8_t device_mode,
		uint8_t *macAddr, uint8_t sessionId);
QDF_STATUS sme_neighbor_report_request(mac_handle_t mac_handle,
		 uint8_t sessionId,
		tpRrmNeighborReq pRrmNeighborReq,
		tpRrmNeighborRspCallbackInfo callbackInfo);
#ifdef FEATURE_OEM_DATA_SUPPORT
QDF_STATUS sme_oem_data_req(mac_handle_t mac_handle,
			    struct oem_data_req *hdd_oem_req);
QDF_STATUS sme_oem_update_capability(mac_handle_t mac_handle,
				     struct sme_oem_capability *cap);
QDF_STATUS sme_oem_get_capability(mac_handle_t mac_handle,
				  struct sme_oem_capability *cap);
#endif /*FEATURE_OEM_DATA_SUPPORT */
QDF_STATUS sme_change_mcc_beacon_interval(uint8_t sessionId);
QDF_STATUS sme_set_host_offload(mac_handle_t mac_handle, uint8_t sessionId,
		struct sir_host_offload_req *pRequest);
QDF_STATUS sme_set_keep_alive(mac_handle_t mac_handle, uint8_t sessionId,
		struct keep_alive_req *pRequest);
QDF_STATUS sme_get_operation_channel(mac_handle_t mac_handle,
				     uint32_t *pChannel,
				     uint8_t sessionId);
QDF_STATUS sme_register_mgmt_frame(mac_handle_t mac_handle, uint8_t sessionId,
		uint16_t frameType, uint8_t *matchData,
		uint16_t matchLen);
QDF_STATUS sme_deregister_mgmt_frame(mac_handle_t mac_handle,
				     uint8_t sessionId,
				     uint16_t frameType, uint8_t *matchData,
				     uint16_t matchLen);
#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
QDF_STATUS sme_configure_ext_wow(mac_handle_t mac_handle,
		tpSirExtWoWParams wlanExtParams,
		csr_readyToSuspendCallback callback,
		void *callbackContext);
QDF_STATUS sme_configure_app_type1_params(mac_handle_t mac_handle,
		tpSirAppType1Params wlanAppType1Params);
QDF_STATUS sme_configure_app_type2_params(mac_handle_t mac_handle,
		tpSirAppType2Params wlanAppType2Params);
#endif
/**
 * sme_get_beaconing_concurrent_operation_channel() - To get concurrent
 * operating channel of beaconing interface
 * @mac_handle: Pointer to mac context
 * @vdev_id_to_skip: channel of which vdev id to skip
 *
 * This routine will return operating channel of active AP/GO channel
 * and will skip the channel of vdev_id_to_skip.
 * If other no reqested mode is active it will return 0
 *
 * Return: uint8_t
 */
uint8_t sme_get_beaconing_concurrent_operation_channel(mac_handle_t mac_handle,
						       uint8_t vdev_id_to_skip);
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
uint16_t sme_check_concurrent_channel_overlap(mac_handle_t mac_handle,
					      uint16_t sap_ch,
					      eCsrPhyMode sapPhyMode,
					      uint8_t cc_switch_mode);
#endif
QDF_STATUS sme_get_cfg_valid_channels(uint8_t *aValidChannels,
		uint32_t *len);
#ifdef WLAN_FEATURE_PACKET_FILTERING
QDF_STATUS sme_8023_multicast_list(mac_handle_t mac_handle, uint8_t sessionId,
		tpSirRcvFltMcAddrList pMulticastAddrs);
#endif /* WLAN_FEATURE_PACKET_FILTERING */
bool sme_is_channel_valid(mac_handle_t mac_handle, uint8_t channel);
uint16_t sme_chn_to_freq(uint8_t chanNum);
bool sme_is_channel_valid(mac_handle_t mac_handle, uint8_t channel);
QDF_STATUS sme_set_max_tx_power(mac_handle_t mac_handle,
				struct qdf_mac_addr pBssid,
				struct qdf_mac_addr pSelfMacAddress, int8_t dB);
QDF_STATUS sme_set_max_tx_power_per_band(enum band_info band, int8_t db);
QDF_STATUS sme_set_tx_power(mac_handle_t mac_handle, uint8_t sessionId,
		struct qdf_mac_addr bssid,
		enum QDF_OPMODE dev_mode, int power);
QDF_STATUS sme_set_custom_mac_addr(tSirMacAddr customMacAddr);
QDF_STATUS sme_hide_ssid(mac_handle_t mac_handle, uint8_t sessionId,
		uint8_t ssidHidden);

QDF_STATUS sme_update_roam_scan_n_probes(mac_handle_t mac_handle,
					 uint8_t sessionId,
					 const uint8_t nProbes);
QDF_STATUS sme_update_roam_scan_home_away_time(mac_handle_t mac_handle,
		uint8_t sessionId,
		const uint16_t nRoamScanHomeAwayTime,
		const bool bSendOffloadCmd);

bool sme_get_roam_intra_band(mac_handle_t mac_handle);
uint8_t sme_get_roam_scan_n_probes(mac_handle_t mac_handle);
uint16_t sme_get_roam_scan_home_away_time(mac_handle_t mac_handle);
QDF_STATUS sme_update_roam_rssi_diff(mac_handle_t mac_handle, uint8_t sessionId,
		uint8_t RoamRssiDiff);
QDF_STATUS sme_update_wes_mode(mac_handle_t mac_handle, bool isWESModeEnabled,
		uint8_t sessionId);
QDF_STATUS sme_set_roam_scan_control(mac_handle_t mac_handle, uint8_t sessionId,
		bool roamScanControl);

QDF_STATUS sme_update_is_fast_roam_ini_feature_enabled(mac_handle_t mac_handle,
		uint8_t sessionId,
		const bool
		isFastRoamIniFeatureEnabled);

QDF_STATUS sme_config_fast_roaming(mac_handle_t mac_handle, uint8_t session_id,
				   const bool is_fast_roam_enabled);

QDF_STATUS sme_stop_roaming(mac_handle_t mac_handle, uint8_t sessionId,
			    uint8_t reason);

QDF_STATUS sme_start_roaming(mac_handle_t mac_handle, uint8_t sessionId,
		uint8_t reason);
#ifdef FEATURE_WLAN_ESE
QDF_STATUS sme_update_is_ese_feature_enabled(mac_handle_t mac_handle,
					     uint8_t sessionId,
					     const bool isEseIniFeatureEnabled);
#endif /* FEATURE_WLAN_ESE */
QDF_STATUS sme_set_roam_rescan_rssi_diff(mac_handle_t mac_handle,
		uint8_t sessionId,
		const uint8_t nRoamRescanRssiDiff);
uint8_t sme_get_roam_rescan_rssi_diff(mac_handle_t mac_handle);

QDF_STATUS sme_set_roam_opportunistic_scan_threshold_diff(
		mac_handle_t mac_handle,
		uint8_t sessionId,
		const uint8_t nOpportunisticThresholdDiff);
uint8_t sme_get_roam_opportunistic_scan_threshold_diff(mac_handle_t mac_handle);
QDF_STATUS sme_set_neighbor_lookup_rssi_threshold(mac_handle_t mac_handle,
		uint8_t sessionId, uint8_t neighborLookupRssiThreshold);
uint8_t sme_get_neighbor_lookup_rssi_threshold(mac_handle_t mac_handle);
QDF_STATUS sme_set_neighbor_scan_refresh_period(mac_handle_t mac_handle,
		uint8_t sessionId, uint16_t neighborScanResultsRefreshPeriod);
uint16_t sme_get_neighbor_scan_refresh_period(mac_handle_t mac_handle);
uint16_t sme_get_empty_scan_refresh_period(mac_handle_t mac_handle);
QDF_STATUS sme_update_empty_scan_refresh_period(mac_handle_t mac_handle,
		uint8_t sessionId, uint16_t empty_scan_refresh_period);
QDF_STATUS sme_set_neighbor_scan_min_chan_time(mac_handle_t mac_handle,
		const uint16_t nNeighborScanMinChanTime,
		uint8_t sessionId);
QDF_STATUS sme_set_neighbor_scan_max_chan_time(mac_handle_t mac_handle,
				uint8_t sessionId,
				const uint16_t nNeighborScanMaxChanTime);
uint16_t sme_get_neighbor_scan_min_chan_time(mac_handle_t mac_handle,
					     uint8_t sessionId);
uint32_t sme_get_neighbor_roam_state(mac_handle_t mac_handle,
				     uint8_t sessionId);
uint32_t sme_get_current_roam_state(mac_handle_t mac_handle, uint8_t sessionId);
uint32_t sme_get_current_roam_sub_state(mac_handle_t mac_handle,
					uint8_t sessionId);
uint32_t sme_get_lim_sme_state(mac_handle_t mac_handle);
uint32_t sme_get_lim_mlm_state(mac_handle_t mac_handle);
bool sme_is_lim_session_valid(mac_handle_t mac_handle, uint8_t sessionId);
uint32_t sme_get_lim_sme_session_state(mac_handle_t mac_handle,
				       uint8_t sessionId);
uint32_t sme_get_lim_mlm_session_state(mac_handle_t mac_handle,
				       uint8_t sessionId);
uint16_t sme_get_neighbor_scan_max_chan_time(mac_handle_t mac_handle,
					     uint8_t sessionId);
QDF_STATUS sme_set_neighbor_scan_period(mac_handle_t mac_handle,
		uint8_t sessionId,
		const uint16_t nNeighborScanPeriod);
uint16_t sme_get_neighbor_scan_period(mac_handle_t mac_handle,
				      uint8_t sessionId);
QDF_STATUS sme_set_roam_bmiss_first_bcnt(mac_handle_t mac_handle,
		uint8_t sessionId, const uint8_t nRoamBmissFirstBcnt);
QDF_STATUS sme_set_roam_bmiss_final_bcnt(mac_handle_t mac_handle,
					 uint8_t sessionId,
					 const uint8_t nRoamBmissFinalBcnt);
uint8_t sme_get_roam_rssi_diff(mac_handle_t mac_handle);
QDF_STATUS sme_change_roam_scan_channel_list(mac_handle_t mac_handle,
					     uint8_t sessionId,
					     uint8_t *pChannelList,
					     uint8_t numChannels);
QDF_STATUS sme_set_ese_roam_scan_channel_list(mac_handle_t mac_handle,
					      uint8_t sessionId,
					      uint8_t *pChannelList,
					      uint8_t numChannels);
QDF_STATUS sme_get_roam_scan_channel_list(mac_handle_t mac_handle,
					  uint8_t *pChannelList,
					  uint8_t *pNumChannels,
					  uint8_t sessionId);
bool sme_get_is_ese_feature_enabled(mac_handle_t mac_handle);
bool sme_get_wes_mode(mac_handle_t mac_handle);
bool sme_get_roam_scan_control(mac_handle_t mac_handle);
bool sme_get_is_lfr_feature_enabled(mac_handle_t mac_handle);
bool sme_get_is_ft_feature_enabled(mac_handle_t mac_handle);
bool sme_is_feature_supported_by_fw(enum cap_bitmap feature);

QDF_STATUS sme_set_phy_mode(mac_handle_t mac_handle, eCsrPhyMode phyMode);
eCsrPhyMode sme_get_phy_mode(mac_handle_t mac_handle);
QDF_STATUS sme_handoff_request(mac_handle_t mac_handle, uint8_t sessionId,
			       tCsrHandoffRequest *pHandoffInfo);

QDF_STATUS sme_add_periodic_tx_ptrn(mac_handle_t mac_handle,
		tSirAddPeriodicTxPtrn *addPeriodicTxPtrnParams);
QDF_STATUS sme_del_periodic_tx_ptrn(mac_handle_t mac_handle,
		tSirDelPeriodicTxPtrn *delPeriodicTxPtrnParams);
QDF_STATUS sme_send_rate_update_ind(mac_handle_t mac_handle,
		tSirRateUpdateInd *rateUpdateParams);
QDF_STATUS sme_roam_del_pmkid_from_cache(mac_handle_t mac_handle,
					 uint8_t sessionId,
					 tPmkidCacheInfo *pmksa,
					 bool flush_cache);
void sme_get_command_q_status(mac_handle_t mac_handle);

#ifdef FEATURE_WLAN_RMC
QDF_STATUS sme_enable_rmc(mac_handle_t mac_handle, uint32_t sessionId);
QDF_STATUS sme_disable_rmc(mac_handle_t mac_handle, uint32_t sessionId);
QDF_STATUS sme_send_rmc_action_period(mac_handle_t mac_handle,
				      uint32_t sessionId);
#endif
QDF_STATUS sme_request_ibss_peer_info(mac_handle_t mac_handle,
				      void *cb_context,
				      ibss_peer_info_cb peer_info_cb,
				      bool allPeerInfoReqd,
				      uint8_t staIdx);
QDF_STATUS sme_send_cesium_enable_ind(mac_handle_t mac_handle,
				      uint32_t sessionId);

/**
 * sme_set_wlm_latency_level_ind() - Used to set the latency level to fw
 * @mac_handle
 * @session_id
 * @latency_level
 *
 * Return QDF_STATUS
 */
QDF_STATUS sme_set_wlm_latency_level(mac_handle_t mac_handle,
				     uint16_t session_id,
				     uint16_t latency_level);
/*
 * SME API to enable/disable idle mode powersave
 * This should be called only if powersave offload
 * is enabled
 */
QDF_STATUS sme_set_idle_powersave_config(bool value);
QDF_STATUS sme_notify_modem_power_state(mac_handle_t mac_handle,
					uint32_t value);

/*SME API to convert convert the ini value to the ENUM used in csr and MAC*/
ePhyChanBondState sme_get_cb_phy_state_from_cb_ini_value(uint32_t cb_ini_value);
int sme_update_ht_config(mac_handle_t mac_handle, uint8_t sessionId,
			 uint16_t htCapab,
			 int value);
int16_t sme_get_ht_config(mac_handle_t mac_handle, uint8_t session_id,
			  uint16_t ht_capab);
#ifdef QCA_HT_2040_COEX
QDF_STATUS sme_notify_ht2040_mode(mac_handle_t mac_handle, uint16_t staId,
				  struct qdf_mac_addr macAddrSTA,
				  uint8_t sessionId,
				  uint8_t channel_type);
QDF_STATUS sme_set_ht2040_mode(mac_handle_t mac_handle, uint8_t sessionId,
			       uint8_t channel_type, bool obssEnabled);
#endif
QDF_STATUS sme_get_reg_info(mac_handle_t mac_handle, uint8_t chanId,
			    uint32_t *regInfo1, uint32_t *regInfo2);

#ifdef FEATURE_WLAN_CH_AVOID
QDF_STATUS sme_ch_avoid_update_req(mac_handle_t mac_handle);
#else
static inline
QDF_STATUS sme_ch_avoid_update_req(mac_handle_t mac_handle)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
/**
 * sme_set_auto_shutdown_cb() - Register auto shutdown evt handler
 * @mac_handle: Handle to the global MAC context
 * @callback_fn: callback function to be invoked when an auto shutdown
 *               event is received
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_auto_shutdown_cb(mac_handle_t mac_handle,
				    void (*callback_fn)(void));

QDF_STATUS sme_set_auto_shutdown_timer(mac_handle_t mac_handle,
				       uint32_t timer_value);
#endif
QDF_STATUS sme_roam_channel_change_req(mac_handle_t mac_handle,
				       struct qdf_mac_addr bssid,
				       struct ch_params *ch_params,
				       struct csr_roam_profile *profile);

QDF_STATUS sme_roam_start_beacon_req(mac_handle_t mac_handle,
				     struct qdf_mac_addr bssid,
				     uint8_t dfsCacWaitStatus);

#ifdef CONFIG_VDEV_SM
/**
 * sme_csa_restart() - request CSA IE transmission from PE
 * @mac_ctx: mac context
 * @session_id: SAP session id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_csa_restart(struct mac_context *mac_ctx, uint8_t session_id);
#endif

QDF_STATUS sme_roam_csa_ie_request(mac_handle_t mac_handle,
				   struct qdf_mac_addr bssid,
				   uint8_t targetChannel, uint8_t csaIeReqd,
				   struct ch_params *ch_params);

/**
 * sme_set_addba_accept() - Allow/Reject the ADDBA req session
 * @mac_handle: handle returned by mac_open
 * @session_id: sme session id
 * @value: Allow/Reject AddBA session
 *
 * Allows/Rejects the ADDBA req session
 *
 * Return: 0 on success else errno
 */
int sme_set_addba_accept(mac_handle_t mac_handle, uint8_t session_id,
			 int value);

QDF_STATUS sme_init_thermal_info(mac_handle_t mac_handle);

QDF_STATUS sme_set_thermal_level(mac_handle_t mac_handle, uint8_t level);
QDF_STATUS sme_txpower_limit(mac_handle_t mac_handle,
			     struct tx_power_limit *psmetx);

/**
 * sme_get_link_speed() - Retrieve current link speed
 * @mac_handle: Global MAC handle
 * @req: Link speed request structure
 * @context: User context to be passed back when invoking @cb
 * @cb: Callback function to be invoked with link speed results
 *
 * Return: QDF_STATUS_SUCCESS if the request was accepted, otherwise
 * an appropriate error status.
 */
QDF_STATUS sme_get_link_speed(mac_handle_t mac_handle,
			      struct link_speed_info *req,
			      void *context,
			      sme_link_speed_cb cb);

QDF_STATUS sme_modify_add_ie(mac_handle_t mac_handle,
		tSirModifyIE *pModifyIE, eUpdateIEsType updateType);
QDF_STATUS sme_update_add_ie(mac_handle_t mac_handle,
		tSirUpdateIE *pUpdateIE, eUpdateIEsType updateType);
QDF_STATUS sme_update_connect_debug(mac_handle_t mac_handle,
				    uint32_t set_value);
const char *sme_bss_type_to_string(const uint8_t bss_type);
QDF_STATUS sme_ap_disable_intra_bss_fwd(mac_handle_t mac_handle,
					uint8_t sessionId,
					bool disablefwd);

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

typedef struct sStatsExtRequestReq {
	uint32_t request_data_len;
	uint8_t *request_data;
} tStatsExtRequestReq, *tpStatsExtRequestReq;

#ifdef WLAN_FEATURE_STATS_EXT
/**
 * sme_stats_ext_register_callback() - Register stats ext callback
 * @mac_handle: Opaque handle to the MAC context
 * @callback: Function to be invoked for stats ext events
 *
 * This function is called to register the callback that send vendor
 * event for stats ext
 */
void sme_stats_ext_register_callback(mac_handle_t mac_handle,
				     stats_ext_cb callback);

/**
 * sme_stats_ext_deregister_callback() - Deregister stats ext callback
 * @mac_handle: Opaque handle to the MAC context
 *
 * This function is called to deregister the callback that send vendor
 * event for stats ext
 */
void sme_stats_ext_deregister_callback(mac_handle_t mac_handle);

/**
 * sme_stats_ext2_register_callback() - Register stats ext2 callback
 * @mac_handle: Opaque handle to the MAC context
 * @callback: Function to be invoked for stats ext2 events
 *
 * This function will register a callback for frame aggregation failure
 * indications processing.
 *
 * Return: void
 */
void sme_stats_ext2_register_callback(mac_handle_t mac_handle,
				      stats_ext2_cb callback);

QDF_STATUS sme_stats_ext_request(uint8_t session_id,
				 tpStatsExtRequestReq input);
#else
static inline void
sme_stats_ext_register_callback(mac_handle_t mac_handle,
				stats_ext_cb callback)
{
}

static inline void
sme_stats_ext2_register_callback(mac_handle_t mac_handle,
				 stats_ext2_cb callback)
{
}
#endif /* WLAN_FEATURE_STATS_EXT */
QDF_STATUS sme_update_dfs_scan_mode(mac_handle_t mac_handle,
		uint8_t sessionId,
		uint8_t allowDFSChannelRoam);
uint8_t sme_get_dfs_scan_mode(mac_handle_t mac_handle);

#ifdef FEATURE_WLAN_EXTSCAN
QDF_STATUS sme_get_valid_channels_by_band(mac_handle_t mac_handle,
					  uint8_t wifiBand,
					  uint32_t *aValidChannels,
					  uint8_t *pNumChannels);

/**
 * sme_ext_scan_get_capabilities() - SME API to fetch extscan capabilities
 * @mac_handle: Opaque handle to the MAC context
 * @params: extscan capabilities request structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_ext_scan_get_capabilities(mac_handle_t mac_handle,
			      struct extscan_capabilities_params *params);

/**
 * sme_ext_scan_start() - SME API to issue extscan start
 * @mac_handle: Opaque handle to the MAC context
 * @params: extscan start structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_ext_scan_start(mac_handle_t mac_handle,
		   struct wifi_scan_cmd_req_params *params);

/**
 * sme_ext_scan_stop() - SME API to issue extscan stop
 * @mac_handle: Opaque handle to the MAC context
 * @params: extscan stop structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_ext_scan_stop(mac_handle_t mac_handle,
			     struct extscan_stop_req_params *params);

/**
 * sme_set_bss_hotlist() - SME API to set BSSID hotlist
 * @mac_handle: Opaque handle to the MAC context
 * @params: extscan set hotlist structure
 *
 * Handles the request to set the BSSID hotlist in firmware.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_set_bss_hotlist(mac_handle_t mac_handle,
		    struct extscan_bssid_hotlist_set_params *params);

/**
 * sme_reset_bss_hotlist() - SME API to reset BSSID hotlist
 * @mac_handle: Opaque handle to the MAC context
 * @params: extscan reset hotlist structure
 *
 * Handles the request to reset the BSSID hotlist in firmware.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_reset_bss_hotlist(mac_handle_t mac_handle,
		      struct extscan_bssid_hotlist_reset_params *params);

/**
 * sme_set_significant_change() - SME API to set significant change
 * @mac_handle: Opaque handle to the MAC context
 * @params: extscan set significant change structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_set_significant_change(mac_handle_t mac_handle,
			   struct extscan_set_sig_changereq_params *params);

/**
 * sme_reset_significant_change() -  SME API to reset significant change
 * @mac_handle: Opaque handle to the MAC context
 * @params: extscan reset significant change structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_reset_significant_change(mac_handle_t mac_handle,
			     struct extscan_capabilities_reset_params *params);

/**
 * sme_get_cached_results() - SME API to get cached results
 * @mac_handle: Opaque handle to the MAC context
 * @params: extscan get cached results structure
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_get_cached_results(mac_handle_t mac_handle,
		       struct extscan_cached_result_params *params);

/**
 * sme_set_epno_list() - set epno network list
 * @mac_handle: Opaque handle to the MAC context
 * @params: request message
 *
 * This function sends an Enhanced PNO configuration to firmware.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_epno_list(mac_handle_t mac_handle,
			     struct wifi_enhanced_pno_params *params);

/**
 * sme_set_passpoint_list() - set passpoint network list
 * @mac_handle: Opaque handle to the MAC context
 * @params: set passpoint list request parameters
 *
 * This function constructs the cds message and fill in message type,
 * bodyptr with @params and posts it to WDA queue.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_set_passpoint_list(mac_handle_t mac_handle,
				  struct wifi_passpoint_req_param *params);

/**
 * sme_reset_passpoint_list() - reset passpoint network list
 * @mac_handle: Opaque handle to the MAC context
 * @params: reset passpoint list request parameters
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_reset_passpoint_list(mac_handle_t mac_handle,
				    struct wifi_passpoint_req_param *params);

QDF_STATUS sme_ext_scan_register_callback(mac_handle_t mac_handle,
					  ext_scan_ind_cb ext_scan_ind_cb);
#else
static inline
QDF_STATUS sme_ext_scan_register_callback(mac_handle_t mac_handle,
					  ext_scan_ind_cb ext_scan_ind_cb)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* FEATURE_WLAN_EXTSCAN */
QDF_STATUS sme_abort_roam_scan(mac_handle_t mac_handle, uint8_t sessionId);
#ifdef WLAN_FEATURE_LINK_LAYER_STATS
QDF_STATUS sme_ll_stats_clear_req(mac_handle_t mac_handle,
		tSirLLStatsClearReq * pclearStatsReq);
QDF_STATUS sme_ll_stats_set_req(mac_handle_t mac_handle,
		tSirLLStatsSetReq *psetStatsReq);

/**
 * sme_ll_stats_get_req() - SME API to get the Link Layer Statistics
 * @mac_handle: Global MAC handle
 * @get_stats_req: Link Layer get stats request params structure
 * @context: Callback context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_ll_stats_get_req(mac_handle_t mac_handle,
				tSirLLStatsGetReq *get_stats_req,
				void *context);

/**
 * sme_set_link_layer_stats_ind_cb() -
 * SME API to trigger the stats are available after get request
 * @mac_handle: MAC handle
 * @callback: HDD callback which needs to be invoked after
 *    getting status notification from FW
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_link_layer_stats_ind_cb(mac_handle_t mac_handle,
					   link_layer_stats_cb callback);

QDF_STATUS sme_set_link_layer_ext_cb(mac_handle_t mac_handle,
		     void (*ll_stats_ext_cb)(hdd_handle_t callback_ctx,
					     tSirLLStatsResults * rsp));
QDF_STATUS sme_reset_link_layer_stats_ind_cb(mac_handle_t mac_handle);
QDF_STATUS sme_ll_stats_set_thresh(mac_handle_t mac_handle,
				struct sir_ll_ext_stats_threshold *threshold);
#else /* WLAN_FEATURE_LINK_LAYER_STATS */
static inline QDF_STATUS
sme_set_link_layer_ext_cb(mac_handle_t mac_handle, void (*ll_stats_ext_cb)
			  (hdd_handle_t callback_ctx, tSirLLStatsResults
			  *rsp))
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
sme_set_link_layer_stats_ind_cb(mac_handle_t mac_handle,
				link_layer_stats_cb callback)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
sme_reset_link_layer_stats_ind_cb(mac_handle_t mac_handle)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

QDF_STATUS sme_set_wisa_params(mac_handle_t mac_handle,
			       struct sir_wisa_params *wisa_params);
#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS sme_update_roam_key_mgmt_offload_enabled(mac_handle_t mac_handle,
		uint8_t session_id,
		bool key_mgmt_offload_enabled,
		struct pmkid_mode_bits *pmkid_modes);
#endif
QDF_STATUS sme_get_link_status(mac_handle_t mac_handle,
			       csr_link_status_callback callback,
			       void *context, uint8_t session_id);
QDF_STATUS sme_get_temperature(mac_handle_t mac_handle,
		void *tempContext,
		void (*pCallbackfn)(int temperature,
			void *pContext));

/**
 * sme_set_scanning_mac_oui() - SME API to set scanning mac oui
 * @mac_handle: MAC Handle
 * @scan_mac_oui: Scanning Mac Oui
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_scanning_mac_oui(mac_handle_t mac_handle,
				    struct scan_mac_oui *scan_mac_oui);

#ifdef DHCP_SERVER_OFFLOAD
/**
 * sme_set_dhcp_srv_offload() - Set DHCP server offload
 * @mac_handle: Handle to the global MAC context
 * @dhcp_srv_info : DHCP server offload info struct
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_set_dhcp_srv_offload(mac_handle_t mac_handle,
			 struct dhcp_offload_info_params *dhcp_srv_info);
#endif /* DHCP_SERVER_OFFLOAD */
#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
QDF_STATUS sme_set_led_flashing(mac_handle_t mac_handle, uint8_t type,
		uint32_t x0, uint32_t x1);
#endif
QDF_STATUS sme_enable_dfs_chan_scan(mac_handle_t mac_handle, uint8_t dfs_flag);
QDF_STATUS sme_set_mas(uint32_t val);
QDF_STATUS sme_set_miracast(mac_handle_t mac_handle, uint8_t filter_type);
QDF_STATUS sme_ext_change_channel(mac_handle_t mac_handle, uint32_t channel,
				  uint8_t session_id);

QDF_STATUS sme_configure_stats_avg_factor(mac_handle_t mac_handle,
					  uint8_t session_id,
					  uint16_t stats_avg_factor);

QDF_STATUS sme_configure_guard_time(mac_handle_t mac_handle, uint8_t session_id,
				    uint32_t guard_time);

QDF_STATUS sme_wifi_start_logger(mac_handle_t mac_handle,
				 struct sir_wifi_start_log start_log);

bool sme_neighbor_middle_of_roaming(mac_handle_t mac_handle,
				    uint8_t sessionId);

/**
 * sme_is_any_session_in_middle_of_roaming() - check if roaming is in progress
 * @mac_handle: MAC Handle
 *
 * Checks if any SME session is in middle of roaming
 *
 * Return: true if roaming is in progress else false
 */
bool sme_is_any_session_in_middle_of_roaming(mac_handle_t mac_handle);

/**
 * sme_send_flush_logs_cmd_to_fw() - Initiate command to FW to flush logs
 *
 * This function will initiate a command to firmware to flush their logs.
 * This should normally be done in response to an anomaly detected by the
 * host.
 *
 * Return: QDF_STATUS_SUCCESS if the command was sent, otherwise an
 *         appropriate QDF_STATUS error
 */
QDF_STATUS sme_send_flush_logs_cmd_to_fw(void);

/**
 * sme_enable_uapsd_for_ac() - enable uapsd for access category request to WMA
 * @sta_id: station id
 * @ac: access category
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
 * sme_disable_uapsd_for_ac() - disable uapsd access category request to WMA
 * @sta_id: station id
 * @ac: access category
 * @sessionId: session id
 *
 * Return: QDF status
 */
QDF_STATUS sme_disable_uapsd_for_ac(uint8_t sta_id,
				    sme_ac_enum_type ac,
				    uint32_t sessionId);

#ifdef FEATURE_RSSI_MONITOR
QDF_STATUS sme_set_rssi_monitoring(mac_handle_t mac_handle,
				   struct rssi_monitor_param *input);

/**
 * sme_set_rssi_threshold_breached_cb() - Set RSSI threshold breached callback
 * @mac_handle: global MAC handle
 * @cb: callback function pointer
 *
 * This function registers the RSSI threshold breached callback function.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_set_rssi_threshold_breached_cb(mac_handle_t mac_handle,
					      rssi_threshold_breached_cb cb);
#else /* FEATURE_RSSI_MONITOR */
static inline
QDF_STATUS sme_set_rssi_threshold_breached_cb(mac_handle_t mac_handle,
					      rssi_threshold_breached_cb cb)
{
	return QDF_STATUS_SUCCESS;
}
#endif
/**
 * sme_reset_rssi_threshold_breached_cb() - Reset RSSI threshold breached
 *                                          callback
 * @mac_handle: global MAC handle
 *
 * This function de-registers the RSSI threshold breached callback function.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_reset_rssi_threshold_breached_cb(mac_handle_t mac_handle);

QDF_STATUS sme_register_mgmt_frame_ind_callback(mac_handle_t mac_handle,
			sir_mgmt_frame_ind_callback callback);

QDF_STATUS sme_update_nss(mac_handle_t mac_handle, uint8_t nss);
void sme_update_user_configured_nss(mac_handle_t mac_handle, uint8_t nss);

bool sme_is_any_session_in_connected_state(mac_handle_t mac_handle);

QDF_STATUS sme_pdev_set_pcl(struct policy_mgr_pcl_list *msg);
QDF_STATUS sme_pdev_set_hw_mode(struct policy_mgr_hw_mode msg);
QDF_STATUS sme_nss_update_request(uint32_t vdev_id,
				  uint8_t  new_nss,
				  policy_mgr_nss_update_cback cback,
				  uint8_t next_action,
				  struct wlan_objmgr_psoc *psoc,
				  enum policy_mgr_conn_update_reason reason,
				  uint32_t original_vdev_id);

typedef void (*sme_peer_authorized_fp) (uint32_t vdev_id);
QDF_STATUS sme_set_peer_authorized(uint8_t *peer_addr,
				   sme_peer_authorized_fp auth_fp,
				   uint32_t vdev_id);
QDF_STATUS sme_soc_set_dual_mac_config(struct policy_mgr_dual_mac_config msg);
QDF_STATUS sme_soc_set_antenna_mode(mac_handle_t mac_handle,
				    struct sir_antenna_mode_param *msg);

void sme_setdef_dot11mode(mac_handle_t mac_handle);

/**
 * sme_update_tx_bfee_supp() - sets the Tx Bfee support
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @cfg_val: Tx Bfee config value
 *
 * Return: 0 on success else err code
 */
int sme_update_tx_bfee_supp(mac_handle_t mac_handle, uint8_t session_id,
			    uint8_t cfg_val);

/**
 * sme_update_tx_bfee_nsts() - sets the Tx Bfee nsts
 * @mac_handle: MAC handle
 * @session_id: SME session id
 * @usr_cfg_val: user config value
 * @nsts_val: Tx Bfee nsts config value
 *
 * Return: 0 on success else err code
 */
int sme_update_tx_bfee_nsts(mac_handle_t mac_handle, uint8_t session_id,
			    uint8_t usr_cfg_val, uint8_t nsts_val);

void wlan_sap_enable_phy_error_logs(mac_handle_t mac_handle,
				    uint32_t enable_log);
#ifdef WLAN_FEATURE_DSRC
int sme_ocb_gen_timing_advert_frame(mac_handle_t mac_handle,
				    tSirMacAddr self_addr,
				    uint8_t **buf, uint32_t *timestamp_offset,
				    uint32_t *time_value_offset);

#else
static inline
int sme_ocb_gen_timing_advert_frame(mac_handle_t mac_handle,
				    tSirMacAddr self_addr, uint8_t **buf,
				    uint32_t *timestamp_offset,
				    uint32_t *time_value_offset)
{
	return 0;
}

#endif

void sme_add_set_thermal_level_callback(mac_handle_t mac_handle,
		sme_set_thermal_level_callback callback);

void sme_update_tgt_services(mac_handle_t mac_handle,
			     struct wma_tgt_services *cfg);

bool sme_validate_sap_channel_switch(mac_handle_t mac_handle,
				     uint16_t sap_ch, eCsrPhyMode sap_phy_mode,
				     uint8_t cc_switch_mode,
				     uint8_t session_id);

bool sme_is_session_id_valid(mac_handle_t mac_handle, uint32_t session_id);

#ifdef FEATURE_WLAN_TDLS
void sme_get_opclass(mac_handle_t mac_handle, uint8_t channel,
		     uint8_t bw_offset, uint8_t *opclass);
#else
static inline void
sme_get_opclass(mac_handle_t mac_handle, uint8_t channel, uint8_t bw_offset,
		uint8_t *opclass)
{
}
#endif

#ifdef FEATURE_LFR_SUBNET_DETECTION
QDF_STATUS sme_gateway_param_update(mac_handle_t mac_handle,
				struct gateway_update_req_param *request);
#endif

void sme_update_fine_time_measurement_capab(mac_handle_t mac_handle,
					    uint8_t session_id,
					    uint32_t val);
QDF_STATUS sme_ht40_stop_obss_scan(mac_handle_t mac_handle, uint32_t vdev_id);
QDF_STATUS sme_set_fw_test(struct set_fwtest_params *fw_test);
QDF_STATUS sme_set_tsfcb(mac_handle_t mac_handle,
	int (*cb_fn)(void *cb_ctx, struct stsf *ptsf), void *cb_ctx);

QDF_STATUS sme_reset_tsfcb(mac_handle_t mac_handle);

#if defined(WLAN_FEATURE_TSF) && !defined(WLAN_FEATURE_TSF_PLUS_NOIRQ)
QDF_STATUS sme_set_tsf_gpio(mac_handle_t mac_handle, uint32_t pinvalue);
#endif

QDF_STATUS sme_update_mimo_power_save(mac_handle_t mac_handle,
				      uint8_t is_ht_smps_enabled,
				      uint8_t ht_smps_mode,
				      bool send_smps_action);

QDF_STATUS sme_add_beacon_filter(mac_handle_t mac_handle,
				 uint32_t session_id, uint32_t *ie_map);
QDF_STATUS sme_remove_beacon_filter(mac_handle_t mac_handle,
				    uint32_t session_id);

#ifdef FEATURE_WLAN_APF
/**
 * sme_get_apf_capabilities() - Get APF capabilities
 * @mac_handle: Opaque handle to the global MAC context
 * @callback: Callback function to be called with the result
 * @context: Opaque context to be used by the caller to associate the
 *   request with the response
 *
 * This function constructs the cds message and fill in message type,
 * post the same to WDA.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_get_apf_capabilities(mac_handle_t mac_handle,
				    apf_get_offload_cb callback,
				    void *context);

/**
 * sme_set_apf_instructions() - Set APF apf filter instructions.
 * @mac_handle: Opaque handle to the global MAC context
 * @apf_set_offload: struct to set apf filter instructions.
 *
 * APFv2 (Legacy APF) API to set the APF packet filter.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_set_apf_instructions(mac_handle_t mac_handle,
				    struct sir_apf_set_offload
							*apf_set_offload);

/**
 * sme_set_apf_enable_disable - Send apf enable/disable cmd
 * @mac_handle: Opaque handle to the global MAC context
 * @vdev_id: vdev id
 * @apf_enable: true: Enable APF Int., false: Disable APF Int.
 *
 * API to either enable or disable the APF interpreter.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_set_apf_enable_disable(mac_handle_t mac_handle, uint8_t vdev_id,
				      bool apf_enable);

/**
 * sme_apf_write_work_memory - Write into the apf work memory
 * @mac_handle: Opaque handle to the global MAC context
 * @write_params: APF parameters for the write operation
 *
 * API for writing into the APF work memory.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_apf_write_work_memory(mac_handle_t mac_handle,
				    struct wmi_apf_write_memory_params
								*write_params);

/**
 * sme_apf_read_work_memory - Read part of apf work memory
 * @mac_handle: Opaque handle to the global MAC context
 * @read_params: APF parameters for the get operation
 * @callback: callback to handle the the read response
 *
 * API for issuing a APF read memory request.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS
sme_apf_read_work_memory(mac_handle_t mac_handle,
			 struct wmi_apf_read_memory_params *read_params,
			 apf_read_mem_cb callback);

#endif /* FEATURE_WLAN_APF */

uint32_t sme_get_wni_dot11_mode(mac_handle_t mac_handle);
QDF_STATUS sme_create_mon_session(mac_handle_t mac_handle, uint8_t *bssid,
				  uint8_t vdev_id);

void sme_set_vdev_ies_per_band(mac_handle_t mac_handle, uint8_t vdev_id);
void sme_set_pdev_ht_vht_ies(mac_handle_t mac_handle, bool enable2x2);

/**
 * sme_update_vdev_type_nss() - sets the nss per vdev type
 * @mac_handle: Opaque handle to the global MAC context
 * @max_supp_nss: max_supported Nss
 * @band: 5G or 2.4G band
 *
 * Sets the per band Nss for each vdev type based on INI and configured
 * chain mask value.
 *
 * Return: None
 */
void sme_update_vdev_type_nss(mac_handle_t mac_handle, uint8_t max_supp_nss,
			      enum nss_chains_band_info band);

#ifdef FEATURE_P2P_LISTEN_OFFLOAD
void sme_register_p2p_lo_event(mac_handle_t mac_handle, void *context,
					p2p_lo_callback callback);
#else
static inline void sme_register_p2p_lo_event(mac_handle_t mac_handle,
					     void *context,
					     p2p_lo_callback callback)
{
}
#endif

QDF_STATUS sme_remove_bssid_from_scan_list(mac_handle_t mac_handle,
	tSirMacAddr bssid);

QDF_STATUS sme_process_mac_pwr_dbg_cmd(mac_handle_t mac_handle,
				       uint32_t session_id,
				       struct sir_mac_pwr_dbg_cmd*
				       dbg_args);

void sme_get_vdev_type_nss(enum QDF_OPMODE dev_mode,
			   uint8_t *nss_2g, uint8_t *nss_5g);
QDF_STATUS sme_roam_set_default_key_index(mac_handle_t mac_handle,
					  uint8_t session_id,
					  uint8_t default_idx);
void sme_send_disassoc_req_frame(mac_handle_t mac_handle,
				 uint8_t session_id, uint8_t *peer_mac,
				 uint16_t reason, uint8_t wait_for_ack);
QDF_STATUS sme_update_access_policy_vendor_ie(mac_handle_t mac_handle,
					      uint8_t session_id,
					      uint8_t *vendor_ie,
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

QDF_STATUS sme_update_sta_roam_policy(mac_handle_t mac_handle,
		enum sta_roam_policy_dfs_mode dfs_mode,
		bool skip_unsafe_channels,
		uint8_t session_id, uint8_t sap_operating_band);
QDF_STATUS sme_enable_disable_chanavoidind_event(mac_handle_t mac_handle,
					uint8_t set_value);
QDF_STATUS sme_set_default_scan_ie(mac_handle_t mac_handle, uint16_t session_id,
				uint8_t *ie_data, uint16_t ie_len);
/**
 * sme_update_session_param() - API to update PE session param
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: Session ID
 * @param_type: Param type to be updated
 * @param_val: Param value to be update
 *
 * Note: this setting will not persist over reboots.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_update_session_param(mac_handle_t mac_handle, uint8_t session_id,
		uint32_t param_type, uint32_t param_val);
#ifdef WLAN_FEATURE_FIPS
/**
 * sme_fips_request() - Perform a FIPS certification operation
 * @mac_handle: Opaque handle to the global MAC context
 * @param: The FIPS certification parameters
 * @callback: Callback function to invoke with the results
 * @context: Opaque context to pass back to caller in the callback
 *
 * Return: QDF_STATUS_SUCCESS if the request is successfully sent
 * to firmware for processing, otherwise an error status.
 */
QDF_STATUS sme_fips_request(mac_handle_t mac_handle, struct fips_params *param,
			    wma_fips_cb callback, void *context);
#else
static inline
QDF_STATUS sme_fips_request(mac_handle_t mac_handle, struct fips_params *param,
			    wma_fips_cb callback, void *context)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif /* WLAN_FEATURE_FIPS */

/**
 * sme_set_cts2self_for_p2p_go() - sme function to set ini parms to FW.
 * @mac_handle: Opaque handle to the global MAC context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_cts2self_for_p2p_go(mac_handle_t mac_handle);

QDF_STATUS sme_update_tx_fail_cnt_threshold(mac_handle_t mac_handle,
		uint8_t session_id, uint32_t tx_fail_count);

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
 * @mac_handle: MAC_HANDLE handle
 * @session_id: session id
 *
 * Return: true if 11r Association, false otherwise.
 */
bool sme_neighbor_roam_is11r_assoc(mac_handle_t mac_handle, uint8_t session_id);

/**
 * sme_update_sta_inactivity_timeout(): Update sta_inactivity_timeout to FW
 * @mac_handle: Handle returned by mac_open
 * @sta_inactivity_timer:  struct for sta inactivity timer
 *
 * If a station does not send anything in sta_inactivity_timeout seconds, an
 * empty data frame is sent to it in order to verify whether it is
 * still in range. If this frame is not ACKed, the station will be
 * disassociated and then deauthenticated.
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure.
*/
QDF_STATUS sme_update_sta_inactivity_timeout(mac_handle_t mac_handle,
		struct sme_sta_inactivity_timeout  *sta_inactivity_timer);

/**
 * sme_set_lost_link_info_cb() - plug in callback function for receiving
 * @mac_handle: Opaque handle to the MAC context
 * @cb: callback function
 *
 * Return: HAL status
 */
QDF_STATUS sme_set_lost_link_info_cb(mac_handle_t mac_handle,
				     lost_link_info_cb cb);

/**
 * sme_update_new_channel_event() - update new channel event for sapFsm
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: session id
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure.
 */
QDF_STATUS sme_update_new_channel_event(mac_handle_t mac_handle,
					uint8_t session_id);
#ifdef WLAN_POWER_DEBUGFS
QDF_STATUS sme_power_debug_stats_req(
		mac_handle_t mac_handle,
		void (*callback_fn)(struct power_stats_response *response,
				    void *context),
		void *power_stats_context);
#endif

#ifdef WLAN_FEATURE_BEACON_RECEPTION_STATS
/**
 * sme_beacon_debug_stats_req() - SME API to collect beacon debug stats
 * @vdev_id: Vdev id on which stats is being requested
 * @callback_fn: Pointer to the callback function for beacon stats event
 * @beacon_stats_context: Pointer to context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_beacon_debug_stats_req(
		mac_handle_t mac_handle, uint32_t vdev_id,
		void (*callback_fn)(struct bcn_reception_stats_rsp
				    *response, void *context),
		void *beacon_stats_context);
#endif

/**
 * sme_get_sar_power_limits() - get SAR limits
 * @mac_handle: Opaque handle to the global MAC context
 * @callback: Callback function to invoke with the results
 * @context: Opaque context to pass back to caller in the callback
 *
 * Return: QDF_STATUS_SUCCESS if the request is successfully sent
 * to firmware for processing, otherwise an error status.
 */
QDF_STATUS sme_get_sar_power_limits(mac_handle_t mac_handle,
				    wma_sar_cb callback, void *context);

/**
 * sme_set_sar_power_limits() - set sar limits
 * @mac_handle: Opaque handle to the global MAC context
 * @sar_limit_cmd: struct to send sar limit cmd.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_set_sar_power_limits(mac_handle_t mac_handle,
		struct sar_limit_cmd_params *sar_limit_cmd);

/**
 * sme_send_coex_config_cmd() - Send COEX config params
 * @coex_cfg_params: struct to coex config params
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_send_coex_config_cmd(struct coex_config_params *coex_cfg_params);

void sme_set_cc_src(mac_handle_t mac_handle, enum country_src);


#ifdef WLAN_FEATURE_WOW_PULSE
QDF_STATUS sme_set_wow_pulse(struct wow_pulse_mode *wow_pulse_set_info);
#endif

/* ARP DEBUG STATS */
QDF_STATUS sme_set_nud_debug_stats(mac_handle_t mac_handle,
				   struct set_arp_stats_params
				   *set_stats_param);
QDF_STATUS sme_get_nud_debug_stats(mac_handle_t mac_handle,
				   struct get_arp_stats_params
				   *get_stats_param);
QDF_STATUS sme_set_nud_debug_stats_cb(mac_handle_t mac_handle,
			void (*cb)(void *, struct rsp_stats *, void *context),
			void *context);

/**
 * sme_set_chan_info_callback() - Register chan info callback
 * @mac_handle - MAC global handle
 * @callback_routine - callback routine from HDD
 *
 * This API is invoked by HDD to register its callback to mac
 *
 * Return: QDF_STATUS
 */
void sme_set_chan_info_callback(mac_handle_t mac_handle,
			void (*callback)(struct scan_chan_info *chan_info));

/**
 * sme_get_rssi_snr_by_bssid() - gets the rssi and snr by bssid from scan cache
 * @mac_handle: handle returned by mac_open
 * @profile: current connected profile
 * @bssid: bssid to look for in scan cache
 * @rssi: rssi value found
 * @snr: snr value found
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_rssi_snr_by_bssid(mac_handle_t mac_handle,
				     struct csr_roam_profile *profile,
				     const uint8_t *bssid, int8_t *rssi,
				     int8_t *snr);

/**
 * sme_get_beacon_frm() - gets the bss descriptor from scan cache and prepares
 * beacon frame
 * @mac_handle: handle returned by mac_open
 * @profile: current connected profile
 * @bssid: bssid to look for in scan cache
 * @frame_buf: frame buffer to populate
 * @frame_len: length of constructed frame
 * @channel: Pointer to channel info to be filled
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_beacon_frm(mac_handle_t mac_handle,
			      struct csr_roam_profile *profile,
			      const tSirMacAddr bssid,
			      uint8_t **frame_buf, uint32_t *frame_len,
			      int *channel);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * sme_fast_reassoc() - invokes FAST REASSOC command
 * @mac_handle: handle returned by mac_open
 * @profile: current connected profile
 * @bssid: bssid to look for in scan cache
 * @channel: channel on which reassoc should be send
 * @vdev_id: vdev id
 * @connected_bssid: bssid of currently connected profile
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_fast_reassoc(mac_handle_t mac_handle,
			    struct csr_roam_profile *profile,
			    const tSirMacAddr bssid, int channel,
			    uint8_t vdev_id, const tSirMacAddr connected_bssid);
#else
static inline
QDF_STATUS sme_fast_reassoc(mac_handle_t mac_handle,
			    struct csr_roam_profile *profile,
			    const tSirMacAddr bssid, int channel,
			    uint8_t vdev_id, const tSirMacAddr connected_bssid)
{
	return QDF_STATUS_SUCCESS;
}
#endif
/**
 * sme_congestion_register_callback() - registers congestion callback
 * @mac_handle: Opaque handle to the global MAC context
 * @congestion_cb: congestion callback
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_congestion_register_callback(mac_handle_t mac_handle,
					    congestion_cb congestion_cb);

/**
 * sme_register_tx_queue_cb(): Register tx queue callback
 * @mac_handle: Opaque handle for MAC context
 * @tx_queue_cb: Transmit Queues callback
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_register_tx_queue_cb(mac_handle_t mac_handle,
				    tx_queue_cb tx_queue_cb);

/**
 * sme_deregister_tx_queue_cb() - Deregister the tx queue callback
 * @mac_handle: Opaque handle for MAC context
 *
 * Return: QDF status
 */
QDF_STATUS sme_deregister_tx_queue_cb(mac_handle_t mac_handle);

/**
 * sme_rso_cmd_status_cb() - Set RSO cmd status callback
 * @mac_handle: Opaque handle for the MAC context
 * @cb: HDD Callback to rso command status read
 *
 * This function is used to save HDD RSO Command status callback in MAC
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_rso_cmd_status_cb(mac_handle_t mac_handle,
				 rso_cmd_status_cb cb);

/**
 * sme_register_set_connection_info_cb() - Register connection
 * info callback
 * @mac_handle - MAC global handle
 * @set_connection_info_cb - callback routine from HDD to set
 *                   connection info flag
 * @get_connection_info_cb - callback routine from HDD to get
 *                         connection info
 *
 * This API is invoked by HDD to register its callback to mac
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_register_set_connection_info_cb(mac_handle_t mac_handle,
				bool (*set_connection_info_cb)(bool),
				bool (*get_connection_info_cb)(uint8_t *session_id,
				enum scan_reject_states *reason));

/**
 * sme_set_dbs_scan_selection_config() - Update DBS scan selection
 * configuration
 * @mac_handle: The handle returned by macOpen
 * @params: wmi_dbs_scan_sel_params config
 *
 * Return: QDF_STATUS if DBS scan selection update
 * configuration success else failure status
 */
QDF_STATUS sme_set_dbs_scan_selection_config(mac_handle_t mac_handle,
		struct wmi_dbs_scan_sel_params *params);

/**
 * sme_store_pdev() - store pdev
 * @mac_handle - MAC global handle
 * @pdev - pdev ptr
 *
 * Return: QDF_STATUS
 */
void sme_store_pdev(mac_handle_t mac_handle, struct wlan_objmgr_pdev *pdev);

/**
 * sme_set_reorder_timeout() - set reorder timeout value
 * including Voice,Video,Besteffort,Background parameters
 * @mac_handle: Opaque handle to the global MAC context
 * @reg: struct sir_set_rx_reorder_timeout_val
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure.
 */
QDF_STATUS sme_set_reorder_timeout(mac_handle_t mac_handle,
		struct sir_set_rx_reorder_timeout_val *req);

/**
 * sme_set_rx_set_blocksize() - set blocksize value
 * including mac_addr and win_limit parameters
 * @mac_handle: Opaque handle to the global MAC context
 * @reg: struct sir_peer_set_rx_blocksize
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure.
 */

QDF_STATUS sme_set_rx_set_blocksize(mac_handle_t mac_handle,
				    struct sir_peer_set_rx_blocksize *req);

/**
 * sme_get_rcpi() - gets the rcpi value for peer mac addr
 * @mac_handle: handle returned by mac_open
 * @rcpi: rcpi request containing peer mac addr, callback and related info
 *
 * This function posts the rcpi measurement request message to wma queue
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_get_rcpi(mac_handle_t mac_handle, struct sme_rcpi_req *rcpi);

/**
 * sme_set_chip_pwr_save_fail_cb() - set chip power save failure callback
 * @mac_handle: opaque handle to the MAC context
 * @cb: callback function pointer
 *
 * This function stores the chip power save failure callback function.
 *
 * Return: QDF_STATUS enumeration.
 */

QDF_STATUS sme_set_chip_pwr_save_fail_cb(mac_handle_t mac_handle,
					 pwr_save_fail_cb cb);
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
 * @mac_handle: handle returned by mac_open
 * @cb: callback handler
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_bt_activity_info_cb(mac_handle_t mac_handle,
				       bt_activity_info_cb cb);

/**
 * sme_set_enable_mem_deep_sleep - set the mem deep sleep config to FW
 * @mac_handle: handle returned by mac_open
 * @vdev_id: vdev id
 *
 * Return: 0 for success else failure code
 */
int sme_set_enable_mem_deep_sleep(mac_handle_t mac_handle, int vdev_id);

/**
 * sme_set_cck_tx_fir_override - set the CCK TX FIR Override to FW
 * @mac_handle: handle returned by mac_open
 * @vdev_id: vdev id
 *
 * Return: 0 for success else failure code
 */
int sme_set_cck_tx_fir_override(mac_handle_t mac_handle, int vdev_id);

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

QDF_STATUS sme_get_peer_stats(struct mac_context *mac,
			      struct sir_peer_info_req req);

/**
 * sme_get_peer_info() - sme api to get peer info
 * @mac_handle: Opaque handle to the global MAC context
 * @req: peer info request struct send to wma
 * @context: context of callback function
 * @callbackfn: hdd callback function when receive response
 *
 * This function will send WMA_GET_PEER_INFO to WMA
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_get_peer_info(mac_handle_t mac_handle,
		struct sir_peer_info_req req,
		void *context,
		void (*callbackfn)(struct sir_peer_info_resp *param,
			void *pcontext));

/**
 * sme_get_peer_info_ext() - sme api to get peer ext info
 * @mac_handle: Opaque handle to the global MAC context
 * @req: peer ext info request struct send to wma
 * @context: context of callback function
 * @callbackfn: hdd callback function when receive response
 *
 * This function will send WMA_GET_PEER_INFO_EXT to WMA
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_get_peer_info_ext(mac_handle_t mac_handle,
		struct sir_peer_info_ext_req *req,
		void *context,
		void (*callbackfn)(struct sir_peer_info_ext_resp *param,
			void *pcontext));

/**
 * sme_get_chain_rssi() - Get chain rssi
 * @mac_handle: Opaque handle to the global MAC context
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
QDF_STATUS sme_get_chain_rssi(mac_handle_t mac_handle,
			      struct get_chain_rssi_req_params *input,
			      get_chain_rssi_callback callback,
			      void *context);

#ifdef FEATURE_FW_STATE
/**
 * sme_get_fw_state() - Get fw state
 * @mac_handle: Opaque handle to the global MAC context
 * @callback: Callback function to be called with the result
 * @context: Opaque context to be used by the caller to associate the
 *   request with the response
 *
 * This function constructs the cds message and fill in message type,
 * post the same to WDA.
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS sme_get_fw_state(mac_handle_t mac_handle,
			    fw_state_callback callback,
			    void *context);
#endif /* FEATURE_FW_STATE */

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
struct mac_context *sme_get_mac_context(void);

/**
 * sme_display_disconnect_stats() - Display per session Disconnect stats
 * @mac_handle: Opaque handle to the global MAC context
 * session_id: SME session id
 *
 * Return: None
 */
void sme_display_disconnect_stats(mac_handle_t mac_handle, uint8_t session_id);

/**
 * sme_process_msg_callback() - process callback message from LIM
 * @mac: global mac context
 * @msg: scheduler message
 *
 * This function process the callback messages from LIM.
 *
 * Return: QDF_STATUS enumeration.
 */
QDF_STATUS sme_process_msg_callback(struct mac_context *mac,
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
 * @mac_handle: Opaque handle to the global MAC context
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
QDF_STATUS sme_send_limit_off_channel_params(mac_handle_t mac_handle,
					     uint8_t vdev_id,
					     bool is_tos_active,
					     uint32_t max_off_chan_time,
					     uint32_t rest_time,
					     bool skip_dfs_chan);

#ifdef FEATURE_WLAN_DYNAMIC_CVM
/**
 * sme_set_vc_mode_config() - Set voltage corner config to FW.
 * @bitmap:	Bitmap that refers to voltage corner config with
 * different phymode and bw configuration
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_vc_mode_config(uint32_t vc_bitmap);
#endif

/**
 * sme_set_del_pmkid_cache() - API to update PMKID cache
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: Session id
 * @pmk_cache_info: Pointer to PMK cache info
 * @is_add: boolean that implies whether to add or delete PMKID entry
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_del_pmkid_cache(mac_handle_t mac_handle, uint8_t session_id,
				   tPmkidCacheInfo *pmk_cache_info,
				   bool is_add);

/**
 * sme_send_hlp_ie_info() - API to send HLP IE info to fw
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: Session id
 * @profile: CSR Roam profile
 * @if_addr: IP address
 *
 * This API is used to send HLP IE info along with IP address
 * to fw if LFR3 is enabled.
 *
 * Return: QDF_STATUS
 */
void sme_send_hlp_ie_info(mac_handle_t mac_handle, uint8_t session_id,
			  struct csr_roam_profile *profile, uint32_t if_addr);

#if defined(WLAN_FEATURE_FILS_SK)
/**
 * sme_update_fils_config - Update FILS config to CSR roam session
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: session id
 * @src_profile: Source profile having latest FILS config
 *
 * API to update FILS config to roam csr session and update the same
 * to fw if LFR3 is enabled.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_update_fils_config(mac_handle_t mac_handle, uint8_t session_id,
				  struct csr_roam_profile *src_profile);

/**
 * sme_free_join_rsp_fils_params - free fils params
 * @roam_info: roam info
 *
 * Return: void
 */
void sme_free_join_rsp_fils_params(struct csr_roam_info *roam_info);
#else
static inline
QDF_STATUS sme_update_fils_config(mac_handle_t mac_handle, uint8_t session_id,
				  struct csr_roam_profile *src_profile)
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
 * @mac_handle: The handle returned by mac_open
 * @session_id: session_id of the request
 * @bss_color: HE BSS color value to set
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_he_bss_color(mac_handle_t mac_handle, uint8_t session_id,
				uint8_t bss_color);
#else
static inline
QDF_STATUS sme_set_he_bss_color(mac_handle_t mac_handle, uint8_t session_id,
				uint8_t bss_color)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * sme_is_conn_state_connected() -- check if SME connection state is connected
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: current Session Id
 *
 * This API checks if the current SME connection state is connected for the
 * given session id.
 *
 * Return: True if connected, false if any other state.
 */
bool sme_is_conn_state_connected(mac_handle_t mac_handle, uint8_t session_id);

/**
 * sme_scan_get_result_for_bssid - gets the scan result from scan cache for the
 * bssid specified
 * @mac_handle: handle returned by mac_open
 * @bssid: bssid to get the scan result for
 * @res: pointer to tCsrScanResultInfo allocated from caller
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_scan_get_result_for_bssid(mac_handle_t mac_handle,
					 struct qdf_mac_addr *bssid,
					 tCsrScanResultInfo *res);

/**
 * sme_get_bss_transition_status() - get bss transition status all cadidates
 * @mac_handle: handle returned by mac_open
 * @transition_reason : Transition reason
 * @bssid: bssid to get BSS transition status
 * @info : bss candidate information
 * @n_candidates : number of candidates
 * @is_bt_in_progress: bt activity indicator
 *
 * Return: QDF_STATUS_SUCCESS on success otherwise a QDF_STATUS error
 */
QDF_STATUS sme_get_bss_transition_status(mac_handle_t mac_handle,
					 uint8_t transition_reason,
					 struct qdf_mac_addr *bssid,
					 struct bss_candidate_info *info,
					 uint16_t n_candidates,
					 bool is_bt_in_progress);

/**
 * sme_unpack_rsn_ie: wrapper to unpack RSN IE and update def RSN params
 * if optional fields are not present.
 * @mac_handle: handle returned by mac_open
 * @buf: rsn ie buffer pointer
 * @buf_len: rsn ie buffer length
 * @rsn_ie: outframe rsn ie structure
 * @append_ie: flag to indicate if the rsn_ie need to be appended from buf
 *
 * Return: parse status
 */
uint32_t sme_unpack_rsn_ie(mac_handle_t mac_handle, uint8_t *buf,
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
 * @mac_handle: handle returned by mac_open
 *
 * The function check if any connected STA is present on which roaming is not
 * enabled and if present enabled roaming on that STA.
 *
 * Return: none
 */
void sme_enable_roaming_on_connected_sta(mac_handle_t mac_handle);

/**
 * sme_send_mgmt_tx() - Sends mgmt frame from CSR to LIM
 * @mac_handle: The handle returned by mac_open
 * @session_id: session id
 * @buf: pointer to frame
 * @len: frame length
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_send_mgmt_tx(mac_handle_t mac_handle, uint8_t session_id,
			    const uint8_t *buf, uint32_t len);

#ifdef WLAN_FEATURE_SAE
/**
 * sme_handle_sae_msg() - Sends SAE message received from supplicant
 * @mac_handle: The handle returned by mac_open
 * @session_id: session id
 * @sae_status: status of SAE authentication
 * @peer_mac_addr: mac address of the peer to be authenticated
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_handle_sae_msg(mac_handle_t mac_handle,
			      uint8_t session_id,
			      uint8_t sae_status,
			      struct qdf_mac_addr peer_mac_addr);
#else
static inline
QDF_STATUS sme_handle_sae_msg(mac_handle_t mac_handle,
			      uint8_t session_id,
			      uint8_t sae_status,
			      struct qdf_mac_addr peer_mac_addr)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * sme_set_ba_buff_size() - sets BA buffer size
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @buff_size: BA buffer size
 *
 * Return: 0 on success else err code
 */
int sme_set_ba_buff_size(mac_handle_t mac_handle, uint8_t session_id,
			 uint16_t buff_size);

/**
 * sme_send_addba_req() - send ADDBA request with user config
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @tid: tid val for BA session
 * @buff_size: BA buffer size
 *
 * Return: 0 on success else err code
 */
int sme_send_addba_req(mac_handle_t mac_handle, uint8_t session_id, uint8_t tid,
		       uint16_t buff_size);

/**
 * sme_set_no_ack_policy() - Sets no ack policy for AC
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @val: no ack policy value
 * @ac: access category
 *
 * Return: 0 on success else err code
 */
int sme_set_no_ack_policy(mac_handle_t mac_handle, uint8_t session_id,
			  uint8_t val, uint8_t ac);

/**
 * sme_set_auto_rate_he_sgi() - Sets SGI for auto rate
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @cfg_val: SGI configuration value
 *
 * Return: 0 on success else err code
 */
int sme_set_auto_rate_he_sgi(mac_handle_t mac_handle, uint8_t session_id,
			     uint8_t cfg_val);

/**
 * sme_set_auto_rate_he_ltf() - Sets HE LTF for auto rate
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @cfg_val: LTF configuration value
 *
 * Return: 0 on success else err code
 */
int sme_set_auto_rate_he_ltf(mac_handle_t mac_handle, uint8_t session_id,
			     uint8_t cfg_val);

#ifdef WLAN_FEATURE_11AX
/**
 * sme_update_tgt_he_cap() - sets the HE caps to pmac
 * @mac_handle: Pointer to MAC handle
 * @cfg: Pointer to WMA target CFG
 * @he_cap_ini: Pointer to HE CAP configured by INI
 *
 * Return: None
 */
void sme_update_tgt_he_cap(mac_handle_t mac_handle,
			   struct wma_tgt_cfg *cfg,
			   tDot11fIEhe_cap *he_cap_ini);

/**
 * sme_update_he_cap_nss() - sets the nss based on user request
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @nss: no.of spatial streams value
 *
 * Return: None
 */
void sme_update_he_cap_nss(mac_handle_t mac_handle, uint8_t session_id,
			   uint8_t nss);

/**
 * sme_update_he_tx_bfee_supp() - sets the HE Tx Bfee support
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @cfg_val: Tx Bfee config value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_tx_bfee_supp(mac_handle_t mac_handle, uint8_t session_id,
			       uint8_t cfg_val);

/**
 * sme_update_he_tx_bfee_nsts() - sets the HE Tx Bfee NSTS
 * @mac_handle: MAC handle
 * @session_id: SME session id
 * @cfg_val: Tx Bfee NSTS value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_tx_bfee_nsts(mac_handle_t mac_handle, uint8_t session_id,
			       uint8_t cfg_val);

/**
 * sme_set_he_tx_bf_cbf_rates() - sets the HE Tx Bfee CBF frame rates to FW
 * @session_id: SME session id
 *
 * Return: None
 */
void sme_set_he_tx_bf_cbf_rates(uint8_t session_id);

/**
 * sme_config_su_ppdu_queue() - Configures SU PPDU queue enable/disable in FW
 * @session_id: SME session id
 * @enable: Enable/Disable config
 *
 * Return: None
 */
void sme_config_su_ppdu_queue(uint8_t session_id, bool enable);

/**
 * sme_update_he_mcs() - sets the HE MCS based on user request
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @he_mcs: HE MCS value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_mcs(mac_handle_t mac_handle, uint8_t session_id,
		      uint16_t he_mcs);

/**
 * sme_update_he_trigger_frm_mac_pad() - sets the HE MAC padding capability
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @cfg_val: HE MAC padding duration value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_trigger_frm_mac_pad(mac_handle_t mac_handle,
				      uint8_t session_id,
				      uint8_t cfg_val);

/**
 * sme_update_he_om_ctrl_supp() - sets the HE OM control capability
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @cfg_val: HE OM control config
 *
 * Return: 0 on success else err code
 */
int sme_update_he_om_ctrl_supp(mac_handle_t mac_handle, uint8_t session_id,
			       uint8_t cfg_val);

#define A_CTRL_ID_OMI 0x1
struct omi_ctrl_tx {
	uint32_t omi_in_vht:1;
	uint32_t omi_in_he:1;
	uint32_t a_ctrl_id:4;
	uint32_t rx_nss:3;
	uint32_t ch_bw:2;
	uint32_t ul_mu_dis:1;
	uint32_t tx_nsts:3;
	uint32_t er_su_dis:1;
	uint32_t dl_mu_mimo_resound:1;
	uint32_t ul_mu_data_dis:1;
	uint32_t reserved:14;
};

int sme_send_he_om_ctrl_bw_update(mac_handle_t mac_handle, uint8_t session_id,
				  uint8_t cfg_val);

int sme_send_he_om_ctrl_nss_update(mac_handle_t mac_handle, uint8_t session_id,
				   uint8_t cfg_val);

void sme_reset_he_om_ctrl(mac_handle_t mac_handle);

/**
 * sme_config_action_tx_in_tb_ppdu() - Sends action frame in TB PPDU cfg to FW
 * @mac_handle: Pointer to MAC handle
 * @session_id: SME session id
 * @cfg_val: configuration setting value
 *
 * Return: 0 on success else err code
 */
int sme_config_action_tx_in_tb_ppdu(mac_handle_t mac_handle, uint8_t session_id,
				    uint8_t cfg_val);

/**
 * sme_send_he_om_ctrl_update() - Send HE OM ctrl Tx cmd to FW
 * @mac_handle: Pointer to mac handle
 * @session_id: SME session id
 *
 * Return: 0 on success else err code
 */
int sme_send_he_om_ctrl_update(mac_handle_t mac_handle, uint8_t session_id);

/**
 * sme_set_he_om_ctrl_param() - Update HE OM control params for OMI Tx
 * @mac_handle: Pointer to mac handle
 * @session_id: SME session id
 * @param: HE om control parameter
 * @cfg_val: HE OM control parameter config value
 *
 * Return: 0 on success else err code
 */
int sme_set_he_om_ctrl_param(mac_handle_t mac_handle, uint8_t session_id,
			     enum qca_wlan_vendor_attr_he_omi_tx param,
			     uint8_t cfg_val);

/**
 * sme_set_usr_cfg_mu_edca() - sets the user cfg MU EDCA params flag
 * @mac_handle: Opaque handle to the global MAC context
 * @val: value to be set
 *
 * Return: none
 */
void sme_set_usr_cfg_mu_edca(mac_handle_t mac_handle, bool val);

/**
 * sme_set_he_mu_edca_def_cfg() - sets the default MU EDCA params values
 * @mac_handle: Opaque handle to the global MAC context
 *
 * Return: none
 */
void sme_set_he_mu_edca_def_cfg(mac_handle_t mac_handle);

/**
 * sme_update_he_htc_he_supp() - Update +HTC-HE support in HE capabilities
 * @mac_handle: Pointer to mac handle
 * @session_id: SME session id
 * @cfg_val: config setting
 *
 * Return: 0 on success else err code
 */
int sme_update_he_htc_he_supp(mac_handle_t mac_handle, uint8_t session_id,
			      bool cfg_val);

/**
 * sme_update_mu_edca_params() - updates MU EDCA params values
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 *
 * Return: 0 on success else err code
 */
int sme_update_mu_edca_params(mac_handle_t mac_handle, uint8_t session_id);

/**
 * sme_update_he_tx_stbc_cap() - Sets the HE Tx STBC capability
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @value: set value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_tx_stbc_cap(mac_handle_t mac_handle, uint8_t session_id,
			      int value);

/**
 * sme_update_he_rx_stbc_cap() - Sets the HE Rx STBC capability
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @value: set value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_rx_stbc_cap(mac_handle_t mac_handle, uint8_t session_id,
			      int value);

/**
 * sme_update_he_frag_supp() - sets the HE fragmentation support
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @he_frag: HE fragmention support value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_frag_supp(mac_handle_t mac_handle, uint8_t session_id,
			    uint16_t he_frag);

/**
 * sme_update_he_ldpc_supp() - sets the HE LDPC support
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @he_ldpc: HE LDPC support value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_ldpc_supp(mac_handle_t mac_handle, uint8_t session_id,
			    uint16_t he_ldpc);

/**
 * sme_update_he_twt_req_support() - Sets twt request capability
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: SME session id
 * @value: set value
 *
 * Return: 0 on success else err code
 */
int sme_update_he_twt_req_support(mac_handle_t mac_handle, uint8_t session_id,
				  uint8_t cfg_val);
#else
static inline void sme_update_tgt_he_cap(mac_handle_t mac_handle,
					 struct wma_tgt_cfg *cfg,
					 tDot11fIEhe_cap *he_cap_ini)
{}
static inline void sme_update_he_cap_nss(mac_handle_t mac_handle,
					 uint8_t session_id,
					 uint8_t nss)
{}
static inline int sme_update_he_mcs(mac_handle_t mac_handle, uint8_t session_id,
				    uint16_t he_mcs)
{
	return 0;
}

static inline void sme_set_he_mu_edca_def_cfg(mac_handle_t mac_handle)
{
}

static inline int sme_update_mu_edca_params(mac_handle_t mac_handle,
					    uint8_t session_id)
{
	return 0;
}

static inline int sme_update_he_trigger_frm_mac_pad(mac_handle_t mac_handle,
						    uint8_t session_id,
						    uint8_t cfg_val)
{
	return 0;
}

static inline int sme_update_he_om_ctrl_supp(mac_handle_t mac_handle,
					     uint8_t session_id,
					     uint8_t cfg_val)
{
	return 0;
}


static inline int
sme_set_he_om_ctrl_param(mac_handle_t mac_handle, uint8_t session_id,
			 enum qca_wlan_vendor_attr_he_omi_tx param,
			 uint8_t cfg_val)
{
	return 0;
}

static inline void sme_reset_he_om_ctrl(mac_handle_t mac_handle)
{
}

static inline int sme_config_action_tx_in_tb_ppdu(mac_handle_t mac_handle,
						  uint8_t session_id,
						  uint8_t cfg_val)
{
	return 0;
}

static inline int sme_update_he_htc_he_supp(mac_handle_t mac_handle,
					    uint8_t session_id,
					    bool cfg_val)
{
	return 0;
}

static inline int
sme_send_he_om_ctrl_update(mac_handle_t mac_handle, uint8_t session_id)
{
	return 0;
}
static inline void sme_set_usr_cfg_mu_edca(mac_handle_t mac_handle, bool val)
{
}

static inline int sme_update_he_tx_stbc_cap(mac_handle_t mac_handle,
					    uint8_t session_id,
					    int value)
{
	return 0;
}

static inline int sme_update_he_rx_stbc_cap(mac_handle_t mac_handle,
					    uint8_t session_id,
					    int value)
{
	return 0;
}

static inline int sme_update_he_frag_supp(mac_handle_t mac_handle,
					  uint8_t session_id,
					  uint16_t he_frag)
{
	return 0;
}

static inline int sme_update_he_ldpc_supp(mac_handle_t mac_handle,
					  uint8_t session_id,
					  uint16_t he_ldpc)
{
	return 0;
}

static inline int sme_update_he_tx_bfee_supp(mac_handle_t mac_handle,
					     uint8_t session_id,
					     uint8_t cfg_val)
{
	return 0;
}
static inline int sme_update_he_tx_bfee_nsts(mac_handle_t mac_handle,
					     uint8_t session_id,
					     uint8_t cfg_val)
{
	return 0;
}

static inline void sme_set_he_tx_bf_cbf_rates(uint8_t session_id)
{
}

static inline void sme_config_su_ppdu_queue(uint8_t session_id, bool enable)
{
}

static inline int sme_update_he_twt_req_support(mac_handle_t mac_handle,
						uint8_t session_id,
						uint8_t cfg_val)
{
	return 0;
}

#endif

/**
 * sme_is_sta_key_exchange_in_progress() - checks whether the STA/P2P client
 * session has key exchange in progress
 *
 * @mac_handle: Opaque handle to the global MAC context
 * @session_id: session id
 *
 * Return: true - if key exchange in progress
 *         false - if not in progress
 */
bool sme_is_sta_key_exchange_in_progress(mac_handle_t mac_handle,
					 uint8_t session_id);

/*
 * sme_validate_channel_list() - Validate the given channel list
 * @mac_handle: Opaque handle to the global MAC context
 * @chan_list: Pointer to the channel list
 * @num_channels: number of channels present in the chan_list
 *
 * Validates the given channel list with base channels in mac context
 *
 * Return: True if all channels in the list are valid, false otherwise
 */
bool sme_validate_channel_list(mac_handle_t mac_handle,
			       uint8_t *chan_list,
			       uint8_t num_channels);
/**
 * sme_set_amsdu() - set amsdu enable/disable based on user cfg
 * @mac_handle: Opaque handle to the global MAC context
 * @enable: enable or disable
 *
 * Return: None
 */
void sme_set_amsdu(mac_handle_t mac_handle, bool enable);

#ifdef WLAN_FEATURE_11AX
void sme_set_he_testbed_def(mac_handle_t mac_handle, uint8_t vdev_id);
void sme_reset_he_caps(mac_handle_t mac_handle, uint8_t vdev_id);
#else
static inline void sme_set_he_testbed_def(mac_handle_t mac_handle,
					  uint8_t vdev_id)
{
}
static inline void sme_reset_he_caps(mac_handle_t mac_handle, uint8_t vdev_id)
{
}
#endif

/**
 * sme_get_mcs_idx() - gets mcs index
 * @max_rate: max rate
 * @rate_flags: rate flags
 * @nss: number of nss
 * @mcs_rate_flags: mcs rate flag
 *
 * Return: return mcs index
 */
uint8_t sme_get_mcs_idx(uint16_t max_rate, uint8_t rate_flags,
			uint8_t *nss, uint8_t *mcs_rate_flags);

#ifdef WLAN_SUPPORT_TWT
/**
 * sme_register_twt_enable_complete_cb() - TWT enable registrar
 * @mac_handle: MAC handle
 * @twt_enable_cb: Function callback to handle enable event
 *
 * Return: QDF Status
 */
QDF_STATUS sme_register_twt_enable_complete_cb(mac_handle_t mac_handle,
					       twt_enable_cb twt_enable_cb);

/**
 * sme_register_twt_disable_complete_cb - TWT disable registrar
 * @mac_handle: MAC handle
 * @twt_disable_cb: Function callback to handle disable event
 *
 * Return: QDF Status
 */
QDF_STATUS sme_register_twt_disable_complete_cb(mac_handle_t mac_handle,
						twt_disable_cb twt_disable_cb);

/**
 * sme_deregister_twt_enable_complete_cb() - TWT enable deregistrar
 * @mac_handle: Opaque handle to the global MAC context
 *
 * Return: QDF Status
 */
QDF_STATUS sme_deregister_twt_enable_complete_cb(mac_handle_t mac_handle);

/**
 * sme_deregister_twt_disable_complete_cb - TWT disable deregistrar
 * @mac_handle: Opaque handle to the global MAC context
 *
 * Return: QDF Status
 */
QDF_STATUS sme_deregister_twt_disable_complete_cb(mac_handle_t mac_handle);

#else
static inline
QDF_STATUS sme_register_twt_enable_complete_cb(mac_handle_t mac_handle,
					       twt_enable_cb twt_enable_cb)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS sme_register_twt_disable_complete_cb(mac_handle_t mac_handle,
						twt_disable_cb twt_disable_cb)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS sme_deregister_twt_enable_complete_cb(mac_handle_t mac_handle)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS sme_deregister_twt_disable_complete_cb(mac_handle_t mac_handle)
{
	return QDF_STATUS_SUCCESS;
}
#endif
/**
 * sme_get_sta_cxn_info() - This function populates all the connection
 *			    information which is formed by DUT-STA to AP
 *			    by calling CSR helper API.
 * @mac_ctx: pointer to mac context
 * @session: pointer to sta session
 * @conn_profile: pointer to connected DUTSTA-REFAP profile
 * @buf: pointer to char buffer to write all the connection information.
 * @buf_size: maximum size of the provided buffer
 *
 * Returns: QDF_STATUS
 */
#ifdef FEATURE_WLAN_DIAG_SUPPORT_CSR
QDF_STATUS sme_get_sta_cxn_info(mac_handle_t mac_handle, uint32_t session_id,
				char *buf, uint32_t buf_sz);
#else
static inline QDF_STATUS
sme_get_sta_cxn_info(mac_handle_t mac_handle, uint32_t session_id,
		     char *buf, uint32_t buf_sz)
{
	qdf_scnprintf(buf, buf_sz,
		      "\nDiag macro disable, ask vendor to enable");
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(FEATURE_WLAN_ESE) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * sme_add_key_btk() - Add BTK key
 * @mac_handle: MAC handle
 * @session_id: SME session identifier
 * @key: key material
 * @key_len: length of the key
 *
 * Return: 0 on success and negative value for failure
 */
int sme_add_key_btk(mac_handle_t mac_handle, uint8_t session_id,
		    const uint8_t *key, const int key_len);

#else
static inline int sme_add_key_btk(mac_handle_t mac_handle, uint8_t session_id,
				  const uint8_t *key, const int key_len)
{
	return 0;
}
#endif

#ifdef FEATURE_WLAN_ESE
/**
 * sme_add_key_krk() - Add KRK key
 * @mac_handle: MAC handle
 * @session_id: SME session identifier
 * @key: key material
 * @key_len: length of the key
 *
 * Return: 0 on success and negative value for failure
 */
int sme_add_key_krk(mac_handle_t mac_handle, uint8_t session_id,
		    const uint8_t *key, const int key_len);

#else

static inline int sme_add_key_krk(mac_handle_t mac_handle, uint8_t session_id,
				  const uint8_t *key, const int key_len)
{
	return 0;
}
#endif

/**
 * sme_find_session_by_bssid() - checks whether has session
 * with given bssid
 * @mac_handle: Opaque handle to the global MAC context
 * @bssid: bssid
 * Return: true - if has the session
 *         false - if not has the session
 */
bool sme_find_session_by_bssid(mac_handle_t mac_handle, uint8_t *bssid);

/**
 * sme_get_roam_scan_stats() - Send roam scan stats cmd to wma
 * @mac_handle: handle returned by mac_open
 * @cb: call-back invoked for roam scan stats response
 * @context: context of callback
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_get_roam_scan_stats(mac_handle_t mac_handle, roam_scan_stats_cb cb,
			void *context, uint32_t vdev_id);

/**
 * sme_update_score_config() - Update the Scoring Config from MLME
 * @mac_handle: Mac Handle
 * @score_config: Pointer to the scoring config structure to be populated
 *
 * Return: None
 */
void sme_update_score_config(mac_handle_t mac_handle,
			     struct scoring_config *score_config);

/**
 * sme_enable_fw_module_log_level() - enable fw module log level
 * @mac_handle: handle returned by mac_open
 * @vdev_id: vdev id
 *
 * Return: None
 */
void sme_enable_fw_module_log_level(mac_handle_t mac_handle, int vdev_id);

#ifdef WLAN_FEATURE_MOTION_DETECTION
/**
 * sme_motion_det_cfg - motion detection configuration
 * @vdev_id: vdev id
 * @time_t1: Time T1 for motion detection in msecs
 * @time_t2: Time T2 for motion detection in msecs
 * @n1: number of packets for coarse detection
 * @n2: number of packets for fine detection
 * @time_t1_gap: gap between packets in coarse detection in msecs
 * @time_t2_gap: gap between packets in fine detection in msecs
 * @coarse_k: number of times fine motion detection has to be performed for
 *	      coarse detection
 * @fine_k: number of times fine motion detection has to be performed for
 *	    fine detection
 * @coarse_q: number of times motion is expected to be detected for success
 *	      case in coarse detection
 * @fine_q: number of times motion is expected to be detected for success
 *	    case in fine detection
 * @md_coarse_thr_high: higher threshold value (in percent) from host to FW,
 *			which will be used in coarse detection phase of motion
 *			detection. This is the threshold for the correlation of
 *			the old RF local-scattering environment with current RF
 *			local-scattering environment. Value of 100(%) indicates
 *			that neither the transceiver nor any nearby objects
 *			have changed position
 * @md_fine_thr_high: higher threshold value (in percent) from host to FW, which
 *		      will be used in fine detection phase of motion detection.
 *		      This is the threshold for correlation between the old and
 *		      current RF environments, as explained above
 * @md_coarse_thr_low: lower threshold value (in percent) for immediate
 *		       detection of motion in coarse detection phase. This is
 *		       the threshold for correlation between the old and current
 *		       RF environments, as explained above
 * @md_fine_thr_low: lower threshold value (in percent) for immediate detection
 *		     of motion in fine detection phase. This is the threshold
 *		     for correlation between the old and current RF
 *		     environments, as explained above
 * @eSME_TDLS_PEER_REMOVE_MAC_ADDR: remove peer mac from connection table
 */

struct sme_motion_det_cfg {
	uint8_t vdev_id;
	uint32_t time_t1;
	uint32_t time_t2;
	uint32_t n1;
	uint32_t n2;
	uint32_t time_t1_gap;
	uint32_t time_t2_gap;
	uint32_t coarse_K;
	uint32_t fine_K;
	uint32_t coarse_Q;
	uint32_t fine_Q;
	uint8_t md_coarse_thr_high;
	uint8_t md_fine_thr_high;
	uint8_t md_coarse_thr_low;
	uint8_t md_fine_thr_low;
};

/**
 * sme_motion_det_base_line_cfg - motion detection base line configuration
 * @vdev_id : vdev id
 * @bl_time_t: time T for baseline (in ms), every bl_time_t, bl_n pkts are sent
 * @bl_packet_gap: gap between packets for baseline  in msecs
 * bl_n: number of packets to be sent during one baseline
 * bl_num_meas: number of times the baseline measurement to be done
 */
struct sme_motion_det_base_line_cfg {
	uint8_t vdev_id;
	uint32_t bl_time_t;
	uint32_t bl_packet_gap;
	uint32_t bl_n;
	uint32_t bl_num_meas;
};

/**
 * sme_motion_det_en - Start/Stop motion detection
 * @vdev_id: vdev_id
 * @enable: start = 1, stop =0
 */
struct sme_motion_det_en {
	uint8_t vdev_id;
	bool enable;
};

/**
 * sme_motion_det_base_line_en - Start/Stop motion detection base line
 * @vdev_id: vdev_id
 * @enable: start = 1, stop =0
 */
struct sme_motion_det_base_line_en {
	uint8_t vdev_id;
	bool enable;
};

/**
 * sme_motion_det_config - Post motion detection configuration msg to scheduler
 * @mac_handle: mac handle
 * @motion_det_cfg: motion detection configuration
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_motion_det_config(mac_handle_t mac_handle,
				 struct sme_motion_det_cfg *motion_det_cfg);

/**
 * sme_motion_det_enable - Post motion detection start/stop msg to scheduler
 * @mac_handle: mac handle
 * @motion_det_en: motion detection start/stop
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_motion_det_enable(mac_handle_t mac_handle,
				 struct sme_motion_det_en *motion_det_en);

/**
 * sme_motion_det_base_line_config - Post md baselining cfg msg to scheduler
 * @mac_handle: mac handle
 * @motion_det_base_line_cfg: motion detection baselining configuration
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_motion_det_base_line_config(
		mac_handle_t mac_handle,
		struct sme_motion_det_base_line_cfg *motion_det_base_line_cfg);

/**
 * sme_motion_det_base_line_enable - Post md baselining enable msg to scheduler
 * @mac_handle: mac handle
 * @motion_det_base_line_en: motion detection baselining start/stop
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */
QDF_STATUS sme_motion_det_base_line_enable(
		mac_handle_t mac_handle,
		struct sme_motion_det_base_line_en *motion_det_base_line_en);

/**
 * sme_set_md_host_evt_cb - Register/set motion detection callback
 * @mac_handle: mac handle
 * @callback_fn: motion detection callback function pointer
 * @hdd_ctx: hdd context
 *
 * Return: QDF_STATUS_SUCCESS or non-zero on failure
 */

QDF_STATUS sme_set_md_host_evt_cb
(
	mac_handle_t mac_handle,
	QDF_STATUS (*callback_fn)(void *ctx, struct sir_md_evt *event),
	void *hdd_ctx
);
#endif /* WLAN_FEATURE_MOTION_DETECTION */
#ifdef FW_THERMAL_THROTTLE_SUPPORT
/**
 * sme_set_thermal_throttle_cfg() - SME API to set the thermal throttle
 * configuration parameters
 * @mac_handle: Opaque handle to the global MAC context
 * @enable: Enable Throttle
 * @dc: duty cycle in msecs
 * @dc_off_percent: duty cycle off percentage
 * @prio: Disables the transmit queues in fw that have lower priority
 * than value defined by prio
 * @target_temp: Target temperature
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_thermal_throttle_cfg(mac_handle_t mac_handle, bool enable,
					uint32_t dc, uint32_t dc_off_percent,
					uint32_t prio, uint32_t target_temp);

/**
 * sme_set_thermal_mgmt() - SME API to set the thermal management params
 * @mac_handle: Opaque handle to the global MAC context
 * @lower_thresh_deg: Lower threshold value of Temperature
 * @higher_thresh_deg: Higher threshold value of Temperature
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_set_thermal_mgmt(mac_handle_t mac_handle,
				uint16_t lower_thresh_deg,
				uint16_t higher_thresh_deg);
#endif /* FW_THERMAL_THROTTLE_SUPPORT */

/**
 * sme_update_hidden_ssid_status_cb() - cb fun to update hidden ssid stats
 * @mac_handle: mac handler
 * @cb: cb of type hidden_ssid_cb
 */
QDF_STATUS sme_update_hidden_ssid_status_cb(mac_handle_t mac_handle,
					    hidden_ssid_cb cb);

/**
 * sme_update_owe_info() - Update OWE info
 * @mac: mac context
 * @assoc_ind: assoc ind
 *
 * Return: QDF_STATUS
 */
QDF_STATUS sme_update_owe_info(struct mac_context *mac,
			       struct assoc_ind *assoc_ind);

#ifdef WLAN_MWS_INFO_DEBUGFS
/**
 * sme_get_mws_coex_info() - SME API to get the coex information
 * @mac_handle: mac handler
 * @vdev_id: Vdev_id
 * @cmd_id: enum mws_coex_cmdid which information is needed.
 * @callback_fn: Callback function
 * @context: callback context
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
sme_get_mws_coex_info(mac_handle_t mac_handle, uint32_t vdev_id,
		      uint32_t cmd_id, void (*callback_fn)(void *coex_info_data,
							   void *context,
							   wmi_mws_coex_cmd_id
							   cmd_id),
		      void *context);
#endif /* WLAN_MWS_INFO_DEBUGFS */

#endif /* #if !defined( __SME_API_H ) */
