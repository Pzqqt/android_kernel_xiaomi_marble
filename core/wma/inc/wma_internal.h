/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

#ifndef WMA_INTERNAL_H
#define WMA_INTERNAL_H

#if !defined(REMOVE_PKT_LOG)
#include "pktlog_ac.h"
#endif

/* ################### defines ################### */
/*
 * TODO: Following constant should be shared by firwmare in
 * wmi_unified.h. This will be done once wmi_unified.h is updated.
 */
#define WMI_PEER_STATE_AUTHORIZED 0x2

#define WMA_2_4_GHZ_MAX_FREQ  3000
#define WOW_CSA_EVENT_OFFSET 12

#define WMA_DEFAULT_SCAN_REQUESTER_ID        1
#define WMI_SCAN_FINISH_EVENTS (WMI_SCAN_EVENT_START_FAILED | \
				WMI_SCAN_EVENT_COMPLETED | \
				WMI_SCAN_EVENT_DEQUEUED)
/* default value */
#define DEFAULT_INFRA_STA_KEEP_ALIVE_PERIOD  20
#define DEFAULT_STA_SA_QUERY_MAX_RETRIES_COUNT       (5)
#define DEFAULT_STA_SA_QUERY_RETRY_INTERVAL    (200)

/* pdev vdev and peer stats*/
#define FW_PDEV_STATS_SET 0x1
#define FW_VDEV_STATS_SET 0x2
#define FW_PEER_STATS_SET 0x4
#define FW_STATS_SET 0x7

/*AR9888/AR6320  noise floor approx value
 * similar to the mentioned the WMA
 */
#define WMA_TGT_NOISE_FLOOR_DBM (-96)

/*
 * Make sure that link monitor and keep alive
 * default values should be in sync with CFG.
 */
#define WMA_LINK_MONITOR_DEFAULT_TIME_SECS 10
#define WMA_KEEP_ALIVE_DEFAULT_TIME_SECS   5

#define AGC_DUMP  1
#define CHAN_DUMP 2
#define WD_DUMP   3
#ifdef CONFIG_ATH_PCIE_ACCESS_DEBUG
#define PCIE_DUMP 4
#endif

/* conformance test limits */
#define FCC       0x10
#define MKK       0x40
#define ETSI      0x30

/* Maximum Buffer length allowed for DFS-2 phyerrors */
#define DFS_MAX_BUF_LENGTH 4096

/*
 * Maximum Buffer length allowed for DFS-3 phyerrors
 * When 160MHz is supported the Max length of phyerrors
 * is larger than the legacy phyerrors.
 */
#define DFS3_MAX_BUF_LENGTH 4436

#define WMI_DEFAULT_NOISE_FLOOR_DBM (-96)

#define WMI_MCC_MIN_CHANNEL_QUOTA             20
#define WMI_MCC_MAX_CHANNEL_QUOTA             80
#define WMI_MCC_MIN_NON_ZERO_CHANNEL_LATENCY  30

/* The maximum number of patterns that can be transmitted by the firmware
 *  and maximum patterns size.
 */
#define WMA_MAXNUM_PERIODIC_TX_PTRNS 6

#define WMI_MAX_HOST_CREDITS 2
#define WMI_WOW_REQUIRED_CREDITS 1

#define WMI_MAX_MHF_ENTRIES 32


#define MAX_HT_MCS_IDX 8
#define MAX_VHT_MCS_IDX 10
#define INVALID_MCS_IDX 255

#define LINK_STATUS_LEGACY      0
#define LINK_STATUS_VHT         0x1
#define LINK_STATUS_MIMO        0x2
#define LINK_SUPPORT_VHT	0x4
#define LINK_SUPPORT_MIMO	0x8

#define LINK_RATE_VHT           0x3

#define MAX_ENTRY_HOLD_REQ_QUEUE 2
#define MAX_ENTRY_VDEV_RESP_QUEUE 10

/* Time(in ms) to detect DOS attack */
#define WMA_MGMT_FRAME_DETECT_DOS_TIMER 1000

/**
 * struct index_data_rate_type - non vht data rate type
 * @mcs_index: mcs rate index
 * @ht20_rate: HT20 supported rate table
 * @ht40_rate: HT40 supported rate table
 */
struct index_data_rate_type {
	uint8_t  mcs_index;
	uint16_t ht20_rate[2];
	uint16_t ht40_rate[2];
};

/**
 * struct index_vht_data_rate_type - vht data rate type
 * @mcs_index: mcs rate index
 * @ht20_rate: VHT20 supported rate table
 * @ht40_rate: VHT40 supported rate table
 * @ht80_rate: VHT80 supported rate table
 */
struct index_vht_data_rate_type {
	uint8_t mcs_index;
	uint16_t ht20_rate[2];
	uint16_t ht40_rate[2];
	uint16_t ht80_rate[2];
};

/*
 * wma_main.c functions declarations
 */

int
wmi_unified_pdev_set_param(wmi_unified_t wmi_handle, WMI_PDEV_PARAM param_id,
			   uint32_t param_value);

void wma_send_msg(tp_wma_handle wma_handle, uint16_t msg_type,
			 void *body_ptr, uint32_t body_val);

void wma_data_tx_ack_comp_hdlr(void *wma_context,
				      qdf_nbuf_t netbuf, int32_t status);

QDF_STATUS wma_set_ppsconfig(uint8_t vdev_id, uint16_t pps_param,
				    int value);

/*
 * wma_scan_roam.c functions declarations
 */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
void wma_process_roam_invoke(WMA_HANDLE handle,
				struct wma_roam_invoke_cmd *roaminvoke);

void wma_process_roam_synch_fail(WMA_HANDLE handle,
				 struct roam_offload_synch_fail *synch_fail);

int wma_roam_synch_event_handler(void *handle, uint8_t *event,
					uint32_t len);
#endif

QDF_STATUS wma_get_buf_start_scan_cmd(tp_wma_handle wma_handle,
				      tSirScanOffloadReq *scan_req,
				      struct scan_start_params *cmd);

QDF_STATUS wma_get_buf_stop_scan_cmd(tp_wma_handle wma_handle,
				     wmi_buf_t *buf,
				     int *buf_len,
				     tAbortScanParams *abort_scan_req);

QDF_STATUS wma_start_scan(tp_wma_handle wma_handle,
			  tSirScanOffloadReq *scan_req, uint16_t msg_type);

QDF_STATUS wma_stop_scan(tp_wma_handle wma_handle,
			 tAbortScanParams *abort_scan_req);

QDF_STATUS wma_update_channel_list(WMA_HANDLE handle,
				   tSirUpdateChanList *chan_list);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS wma_roam_scan_fill_self_caps(tp_wma_handle wma_handle,
					roam_offload_param *
					roam_offload_params,
					tSirRoamOffloadScanReq *roam_req);
#endif

QDF_STATUS wma_roam_scan_offload_mode(tp_wma_handle wma_handle,
				      wmi_start_scan_cmd_fixed_param *
				      scan_cmd_fp,
				      tSirRoamOffloadScanReq *roam_req,
				      uint32_t mode, uint32_t vdev_id);

QDF_STATUS wma_roam_scan_offload_rssi_thresh(tp_wma_handle wma_handle,
					     tSirRoamOffloadScanReq *roam_req);

QDF_STATUS wma_roam_scan_offload_scan_period(tp_wma_handle wma_handle,
					     uint32_t scan_period,
					     uint32_t scan_age,
					     uint32_t vdev_id);

QDF_STATUS wma_roam_scan_offload_rssi_change(tp_wma_handle wma_handle,
					     uint32_t vdev_id,
					     int32_t rssi_change_thresh,
					     uint32_t bcn_rssi_weight,
					     uint32_t hirssi_delay_btw_scans);

QDF_STATUS wma_roam_scan_offload_chan_list(tp_wma_handle wma_handle,
					   uint8_t chan_count,
					   uint8_t *chan_list,
					   uint8_t list_type, uint32_t vdev_id);

A_UINT32 e_csr_auth_type_to_rsn_authmode(eCsrAuthType authtype,
					 eCsrEncryptionType encr);

A_UINT32 e_csr_encryption_type_to_rsn_cipherset(eCsrEncryptionType encr);

void wma_roam_scan_fill_ap_profile(tp_wma_handle wma_handle,
				   tpAniSirGlobal pMac,
				   tSirRoamOffloadScanReq *roam_req,
				   wmi_ap_profile *ap_profile_p);

void wma_roam_scan_fill_scan_params(tp_wma_handle wma_handle,
				    tpAniSirGlobal pMac,
				    tSirRoamOffloadScanReq *roam_req,
				    wmi_start_scan_cmd_fixed_param *
				    scan_params);

QDF_STATUS wma_roam_scan_offload_ap_profile(tp_wma_handle wma_handle,
					    wmi_ap_profile *ap_profile_p,
					    uint32_t vdev_id);

QDF_STATUS wma_roam_scan_bmiss_cnt(tp_wma_handle wma_handle,
				   A_INT32 first_bcnt,
				   A_UINT32 final_bcnt, uint32_t vdev_id);

QDF_STATUS wma_roam_scan_offload_command(tp_wma_handle wma_handle,
					 uint32_t command, uint32_t vdev_id);

QDF_STATUS wma_process_roaming_config(tp_wma_handle wma_handle,
				     tSirRoamOffloadScanReq *roam_req);

QDF_STATUS wma_roam_preauth_chan_set(tp_wma_handle wma_handle,
				     tpSwitchChannelParams params,
				     uint8_t vdev_id);

QDF_STATUS wma_roam_preauth_chan_cancel(tp_wma_handle wma_handle,
					tpSwitchChannelParams params,
					uint8_t vdev_id);

void wma_roam_preauth_scan_event_handler(tp_wma_handle wma_handle,
						uint8_t vdev_id,
						wmi_scan_event_fixed_param *
						wmi_event);

void wma_set_channel(tp_wma_handle wma, tpSwitchChannelParams params);

#ifdef FEATURE_WLAN_SCAN_PNO
QDF_STATUS wma_pno_start(tp_wma_handle wma, tpSirPNOScanReq pno);

QDF_STATUS wma_pno_stop(tp_wma_handle wma, uint8_t vdev_id);

void wma_config_pno(tp_wma_handle wma, tpSirPNOScanReq pno);
void wma_set_pno_channel_prediction(uint8_t *buf_ptr,
		tpSirPNOScanReq pno);
void wma_scan_cache_updated_ind(tp_wma_handle wma, uint8_t sessionId);
#else
static inline void wma_set_pno_channel_prediction(uint8_t *buf_ptr,
		void *pno)
{
	WMA_LOGD("PNO Channel Prediction feature not supported");
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_FIXED_STRUC, 0);
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
void wma_set_ric_req(tp_wma_handle wma, void *msg, uint8_t is_add_ts);
#endif

#ifdef FEATURE_WLAN_EXTSCAN

int wma_extscan_start_stop_event_handler(void *handle,
					 uint8_t *cmd_param_info,
					 uint32_t len);

int wma_extscan_operations_event_handler(void *handle,
					 uint8_t *cmd_param_info,
					 uint32_t len);

int wma_extscan_table_usage_event_handler(void *handle,
					  uint8_t *cmd_param_info,
					  uint32_t len);

int wma_extscan_capabilities_event_handler(void *handle,
					   uint8_t *cmd_param_info,
					   uint32_t len);

int wma_extscan_hotlist_match_event_handler(void *handle,
					    uint8_t *cmd_param_info,
					    uint32_t len);

int wma_extscan_cached_results_event_handler(void *handle,
					     uint8_t *cmd_param_info,
					     uint32_t len);

int wma_extscan_change_results_event_handler(void *handle,
					     uint8_t *cmd_param_info,
					     uint32_t len);

int wma_passpoint_match_event_handler(void *handle,
				     uint8_t  *cmd_param_info,
				     uint32_t len);

int
wma_extscan_hotlist_ssid_match_event_handler(void *handle,
					     uint8_t *cmd_param_info,
					     uint32_t len);
#endif

void wma_register_extscan_event_handler(tp_wma_handle wma_handle);

#ifdef FEATURE_WLAN_EXTSCAN
QDF_STATUS wma_get_buf_extscan_start_cmd(tp_wma_handle wma_handle,
					 tSirWifiScanCmdReqParams *pstart,
					 wmi_buf_t *buf, int *buf_len);

QDF_STATUS wma_start_extscan(tp_wma_handle wma,
			     tSirWifiScanCmdReqParams *pstart);

QDF_STATUS wma_stop_extscan(tp_wma_handle wma,
			    tSirExtScanStopReqParams *pstopcmd);

QDF_STATUS wma_get_buf_extscan_hotlist_cmd(tp_wma_handle wma_handle,
					   tSirExtScanSetBssidHotListReqParams *
					   photlist, int *buf_len);

QDF_STATUS wma_extscan_start_hotlist_monitor(tp_wma_handle wma,
					     tSirExtScanSetBssidHotListReqParams
					     *photlist);

QDF_STATUS wma_extscan_stop_hotlist_monitor(tp_wma_handle wma,
					    tSirExtScanResetBssidHotlistReqParams
					    *photlist_reset);

QDF_STATUS wma_get_buf_extscan_change_monitor_cmd(tp_wma_handle wma_handle,
						  tSirExtScanSetSigChangeReqParams
						  *psigchange, wmi_buf_t *buf,
						  int *buf_len);

QDF_STATUS wma_extscan_start_change_monitor(tp_wma_handle wma,
					    tSirExtScanSetSigChangeReqParams *
					    psigchange);

QDF_STATUS wma_extscan_stop_change_monitor(tp_wma_handle wma,
					   tSirExtScanResetSignificantChangeReqParams
					   *pResetReq);

QDF_STATUS wma_extscan_get_cached_results(tp_wma_handle wma,
					  tSirExtScanGetCachedResultsReqParams *
					  pcached_results);

QDF_STATUS wma_extscan_get_capabilities(tp_wma_handle wma,
					tSirGetExtScanCapabilitiesReqParams *
					pgetcapab);
QDF_STATUS wma_set_epno_network_list(tp_wma_handle wma,
				struct wifi_epno_params *req);

QDF_STATUS wma_set_passpoint_network_list(tp_wma_handle wma,
					struct wifi_passpoint_req *req);

QDF_STATUS wma_reset_passpoint_network_list(tp_wma_handle wma,
					struct wifi_passpoint_req *req);
QDF_STATUS
wma_set_ssid_hotlist(tp_wma_handle wma,
		     struct sir_set_ssid_hotlist_request *request);
#endif

QDF_STATUS  wma_ipa_offload_enable_disable(tp_wma_handle wma,
			struct sir_ipa_offload_enable_disable *ipa_offload);

void wma_process_unit_test_cmd(WMA_HANDLE handle,
				      t_wma_unit_test_cmd * wma_utest);

QDF_STATUS wma_scan_probe_setoui(tp_wma_handle wma, tSirScanMacOui *psetoui);

int wma_scan_event_callback(WMA_HANDLE handle, uint8_t *data, uint32_t len);

void wma_roam_better_ap_handler(tp_wma_handle wma, uint32_t vdev_id);

int wma_roam_event_callback(WMA_HANDLE handle, uint8_t *event_buf,
			    uint32_t len);

#ifdef FEATURE_WLAN_SCAN_PNO
int wma_nlo_match_evt_handler(void *handle, uint8_t *event, uint32_t len);

int wma_nlo_scan_cmp_evt_handler(void *handle, uint8_t *event, uint32_t len);
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
void wma_process_roam_synch_complete(WMA_HANDLE handle, uint8_t vdev_id);
static inline bool wma_is_roam_synch_in_progress(tp_wma_handle wma,
		uint8_t vdev_id)
{
	return wma->interfaces[vdev_id].roam_synch_in_progress;
}
#else
static inline bool wma_is_roam_synch_in_progress(tp_wma_handle wma,
		uint8_t vdev_id)
{
	return false;
}
static inline uint32_t wma_roam_scan_get_cckm_mode(
		struct sSirRoamOffloadScanReq *roam_req, uint32_t auth_mode)
{
	return WMI_AUTH_CCKM;
}
#endif

/*
 * wma_dev_if.c functions declarations
 */

void *wma_find_vdev_by_addr(tp_wma_handle wma, uint8_t *addr,
				   uint8_t *vdev_id);

/**
 * wma_find_vdev_by_id() - Returns vdev handle for given vdev id.
 * @wma - wma handle
 * @vdev_id - vdev ID
 *
 * Return: Returns vdev handle if given vdev id is valid.
 *         Otherwise returns NULL.
 */
static inline void *wma_find_vdev_by_id(tp_wma_handle wma, uint8_t vdev_id)
{
	if (vdev_id > wma->max_bssid)
		return NULL;

	return wma->interfaces[vdev_id].handle;
}

/**
 * wma_get_vdev_count() - Returns number of active vdev.
 * @wma - wma handle
 *
 * Return: Returns valid vdev count.
 */
static inline uint8_t wma_get_vdev_count(tp_wma_handle wma)
{
	uint8_t vdev_count = 0, i;

	for (i = 0; i < wma->max_bssid; i++) {
		if (wma->interfaces[i].handle)
			vdev_count++;
	}
	return vdev_count;
}

bool wma_is_vdev_in_ap_mode(tp_wma_handle wma, uint8_t vdev_id);

#ifdef QCA_IBSS_SUPPORT
bool wma_is_vdev_in_ibss_mode(tp_wma_handle wma, uint8_t vdev_id);
#else
/**
 * wma_is_vdev_in_ibss_mode(): dummy function
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Return false since no vdev can be in ibss mode without ibss support
 */
static inline
bool wma_is_vdev_in_ibss_mode(tp_wma_handle wma, uint8_t vdev_id);
{
	return false;
}
#endif

/**
 * wma_is_vdev_in_beaconning_mode() - check if vdev is in a beaconning mode
 * @wma: wma handle
 * @vdev_id: vdev id
 *
 * Helper function to know whether given vdev id
 * is in a beaconning mode or not.
 *
 * Return: True if vdev needs to beacon.
 */
static inline
bool wma_is_vdev_in_beaconning_mode(tp_wma_handle wma, uint8_t vdev_id)
{
	return wma_is_vdev_in_ap_mode(wma, vdev_id) ||
		wma_is_vdev_in_ibss_mode(wma, vdev_id);
}

/**
 * wma_find_bssid_by_vdev_id() - Get the BSS ID corresponding to the vdev ID
 * @wma - wma handle
 * @vdev_id - vdev ID
 *
 * Return: Returns pointer to bssid on success,
 *         otherwise returns NULL.
 */
static inline uint8_t *wma_find_bssid_by_vdev_id(tp_wma_handle wma,
						 uint8_t vdev_id)
{
	if (vdev_id >= wma->max_bssid)
		return NULL;

	return wma->interfaces[vdev_id].bssid;
}

void *wma_find_vdev_by_bssid(tp_wma_handle wma, uint8_t *bssid,
				    uint8_t *vdev_id);

QDF_STATUS wma_vdev_detach(tp_wma_handle wma_handle,
			struct del_sta_self_params *pdel_sta_self_req_param,
			uint8_t generateRsp);

int wma_vdev_start_resp_handler(void *handle, uint8_t *cmd_param_info,
				       uint32_t len);

QDF_STATUS wma_vdev_set_param(wmi_unified_t wmi_handle, uint32_t if_id,
				uint32_t param_id, uint32_t param_value);

void wma_remove_peer(tp_wma_handle wma, uint8_t *bssid,
			    uint8_t vdev_id, ol_txrx_peer_handle peer,
			    bool roam_synch_in_progress);

QDF_STATUS wma_create_peer(tp_wma_handle wma, ol_txrx_pdev_handle pdev,
			  ol_txrx_vdev_handle vdev,
			  u8 peer_addr[IEEE80211_ADDR_LEN],
			  uint32_t peer_type, uint8_t vdev_id,
			  bool roam_synch_in_progress);

int wma_vdev_stop_resp_handler(void *handle, uint8_t *cmd_param_info,
				      u32 len);

ol_txrx_vdev_handle wma_vdev_attach(tp_wma_handle wma_handle,
				struct add_sta_self_params *self_sta_req,
				uint8_t generateRsp);

QDF_STATUS wma_vdev_start(tp_wma_handle wma,
				 struct wma_vdev_start_req *req, bool isRestart);

void wma_vdev_resp_timer(void *data);

struct wma_target_req *wma_fill_vdev_req(tp_wma_handle wma,
						uint8_t vdev_id,
						uint32_t msg_type, uint8_t type,
						void *params, uint32_t timeout);

void wma_hold_req_timer(void *data);
struct wma_target_req *wma_fill_hold_req(tp_wma_handle wma,
				    uint8_t vdev_id, uint32_t msg_type,
				    uint8_t type, void *params,
				    uint32_t timeout);

void wma_remove_vdev_req(tp_wma_handle wma, uint8_t vdev_id,
				uint8_t type);

void wma_add_bss(tp_wma_handle wma, tpAddBssParams params);

void wma_add_sta(tp_wma_handle wma, tpAddStaParams add_sta);

void wma_delete_sta(tp_wma_handle wma, tpDeleteStaParams del_sta);

void wma_delete_bss(tp_wma_handle wma, tpDeleteBssParams params);

int32_t wma_find_vdev_by_type(tp_wma_handle wma, int32_t type);

void wma_set_vdev_intrabss_fwd(tp_wma_handle wma_handle,
				      tpDisableIntraBssFwd pdis_intra_fwd);

/*
 * wma_mgmt.c functions declarations
 */

int wma_beacon_swba_handler(void *handle, uint8_t *event, uint32_t len);

int wma_peer_sta_kickout_event_handler(void *handle, u8 *event, u32 len);

void wma_extscan_wow_event_callback(void *handle, void *event, uint32_t len);

int wma_unified_bcntx_status_event_handler(void *handle,
					   uint8_t *cmd_param_info,
					   uint32_t len);

void wma_set_sta_sa_query_param(tp_wma_handle wma,
				  uint8_t vdev_id);

void wma_set_sta_keep_alive(tp_wma_handle wma, uint8_t vdev_id,
				   uint32_t method, uint32_t timeperiod,
				   uint8_t *hostv4addr, uint8_t *destv4addr,
				   uint8_t *destmac);

int wma_vdev_install_key_complete_event_handler(void *handle,
						uint8_t *event,
						uint32_t len);

QDF_STATUS wma_send_peer_assoc(tp_wma_handle wma,
					   tSirNwType nw_type,
					   tpAddStaParams params);

QDF_STATUS wmi_unified_vdev_set_gtx_cfg_send(wmi_unified_t wmi_handle,
				  uint32_t if_id,
				  gtx_config_t *gtx_info);

void wma_update_protection_mode(tp_wma_handle wma, uint8_t vdev_id,
			   uint8_t llbcoexist);

void wma_process_update_beacon_params(tp_wma_handle wma,
				 tUpdateBeaconParams *bcn_params);

void wma_update_cfg_params(tp_wma_handle wma, tSirMsgQ *cfgParam);

void wma_set_bsskey(tp_wma_handle wma_handle, tpSetBssKeyParams key_info);

void wma_adjust_ibss_heart_beat_timer(tp_wma_handle wma,
				      uint8_t vdev_id,
				      int8_t peer_num_delta);

void wma_set_stakey(tp_wma_handle wma_handle, tpSetStaKeyParams key_info);

QDF_STATUS wma_process_update_edca_param_req(WMA_HANDLE handle,
						    tEdcaParams *edca_params);

int wma_tbttoffset_update_event_handler(void *handle, uint8_t *event,
					       uint32_t len);

void wma_send_probe_rsp_tmpl(tp_wma_handle wma,
				    tpSendProbeRespParams probe_rsp_info);

void wma_send_beacon(tp_wma_handle wma, tpSendbeaconParams bcn_info);

void wma_set_keepalive_req(tp_wma_handle wma,
				  tSirKeepAliveReq *keepalive);

void wma_beacon_miss_handler(tp_wma_handle wma, uint32_t vdev_id);

void wma_process_update_opmode(tp_wma_handle wma_handle,
				      tUpdateVHTOpMode *update_vht_opmode);

void wma_process_update_rx_nss(tp_wma_handle wma_handle,
				      tUpdateRxNss *update_rx_nss);

void wma_process_update_membership(tp_wma_handle wma_handle,
					  tUpdateMembership *membership);

void wma_process_update_userpos(tp_wma_handle wma_handle,
				       tUpdateUserPos *userpos);

void wma_hidden_ssid_vdev_restart(tp_wma_handle wma_handle,
				  tHalHiddenSsidVdevRestart *pReq);

/*
 * wma_power.c functions declarations
 */

void wma_enable_sta_ps_mode(tp_wma_handle wma, tpEnablePsParams ps_req);

QDF_STATUS wma_unified_set_sta_ps_param(wmi_unified_t wmi_handle,
					    uint32_t vdev_id, uint32_t param,
					    uint32_t value);

QDF_STATUS
wma_set_ibss_pwrsave_params(tp_wma_handle wma, uint8_t vdev_id);

QDF_STATUS wma_set_ap_peer_uapsd(tp_wma_handle wma, uint32_t vdev_id,
				     uint8_t *peer_addr, uint8_t uapsd_value,
				     uint8_t max_sp);

void wma_update_edca_params_for_ac(tSirMacEdcaParamRecord *edca_param,
					  wmi_wmm_vparams *wmm_param, int ac);

void wma_set_tx_power(WMA_HANDLE handle,
			     tMaxTxPowerParams *tx_pwr_params);

void wma_set_max_tx_power(WMA_HANDLE handle,
				 tMaxTxPowerParams *tx_pwr_params);

void wma_disable_sta_ps_mode(tp_wma_handle wma, tpDisablePsParams ps_req);

void wma_enable_uapsd_mode(tp_wma_handle wma, tpEnableUapsdParams ps_req);

void wma_disable_uapsd_mode(tp_wma_handle wma,
				   tpDisableUapsdParams ps_req);

QDF_STATUS wma_get_temperature(tp_wma_handle wma_handle);

int wma_pdev_temperature_evt_handler(void *handle, uint8_t *event,
					    uint32_t len);

QDF_STATUS wma_process_tx_power_limits(WMA_HANDLE handle,
				       tSirTxPowerLimit *ptxlim);

void wma_update_noa(struct beacon_info *beacon,
			   struct p2p_sub_element_noa *noa_ie);

void wma_update_probe_resp_noa(tp_wma_handle wma_handle,
				      struct p2p_sub_element_noa *noa_ie);

int wma_p2p_noa_event_handler(void *handle, uint8_t *event,
				     uint32_t len);

void wma_process_set_p2pgo_noa_req(tp_wma_handle wma,
					  tP2pPsParams *ps_params);

void wma_process_set_mimops_req(tp_wma_handle wma_handle,
				       tSetMIMOPS *mimops);

QDF_STATUS wma_set_mimops(tp_wma_handle wma, uint8_t vdev_id, int value);

QDF_STATUS wma_notify_modem_power_state(void *wma_ptr,
					tSirModemPowerStateInd *pReq);

QDF_STATUS wma_set_smps_params(tp_wma_handle wma, uint8_t vdev_id,
				      int value);

void wma_set_suspend_dtim(tp_wma_handle wma);

void wma_set_resume_dtim(tp_wma_handle wma);

/*
 * wma_data.c functions declarations
 */


void wma_set_bss_rate_flags(struct wma_txrx_node *iface,
				   tpAddBssParams add_bss);

int32_t wmi_unified_send_txbf(tp_wma_handle wma, tpAddStaParams params);

void wma_update_txrx_chainmask(int num_rf_chains, int *cmd_value);

int wma_peer_state_change_event_handler(void *handle,
					       uint8_t *event_buff,
					       uint32_t len);

QDF_STATUS wma_set_enable_disable_mcc_adaptive_scheduler(uint32_t
						mcc_adaptive_scheduler);

QDF_STATUS wma_set_mcc_channel_time_latency
	(tp_wma_handle wma,
	uint32_t mcc_channel, uint32_t mcc_channel_time_latency);

QDF_STATUS wma_set_mcc_channel_time_quota
	(tp_wma_handle wma,
	uint32_t adapter_1_chan_number,
	uint32_t adapter_1_quota, uint32_t adapter_2_chan_number);

void wma_set_linkstate(tp_wma_handle wma, tpLinkStateParams params);

void wma_unpause_vdev(tp_wma_handle wma);

QDF_STATUS wma_process_rate_update_indicate(tp_wma_handle wma,
					    tSirRateUpdateInd *
					    pRateUpdateParams);

QDF_STATUS wma_tx_attach(tp_wma_handle wma_handle);

QDF_STATUS wma_tx_detach(tp_wma_handle wma_handle);

#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || \
	defined(QCA_LL_TX_FLOW_CONTROL_V2) || defined(CONFIG_HL_SUPPORT)

int wma_mcc_vdev_tx_pause_evt_handler(void *handle, uint8_t *event,
					     uint32_t len);
#endif

#if defined(CONFIG_HL_SUPPORT) && defined(QCA_BAD_PEER_TX_FLOW_CL)
QDF_STATUS wma_process_init_bad_peer_tx_ctl_info(tp_wma_handle wma,
					struct t_bad_peer_txtcl_config *config);
#else
static inline QDF_STATUS
wma_process_init_bad_peer_tx_ctl_info(tp_wma_handle wma,
			struct t_bad_peer_txtcl_config *config)
{
	return QDF_STATUS_E_FAILURE;
}
#endif

QDF_STATUS wma_process_init_thermal_info(tp_wma_handle wma,
					 t_thermal_mgmt *pThermalParams);

QDF_STATUS wma_process_set_thermal_level(tp_wma_handle wma,
					 uint8_t thermal_level);

QDF_STATUS wma_set_thermal_mgmt(tp_wma_handle wma_handle,
				       t_thermal_cmd_params thermal_info);

int wma_thermal_mgmt_evt_handler(void *handle, uint8_t *event,
					uint32_t len);

int wma_ibss_peer_info_event_handler(void *handle, uint8_t *data,
					    uint32_t len);

int wma_fast_tx_fail_event_handler(void *handle, uint8_t *data,
					  uint32_t len);

/*
 * wma_utils.c functions declarations
 */

#ifdef WLAN_FEATURE_STATS_EXT
int wma_stats_ext_event_handler(void *handle, uint8_t *event_buf,
				       uint32_t len);
#endif

tSmpsModeValue host_map_smps_mode(A_UINT32 fw_smps_mode);
int wma_smps_mode_to_force_mode_param(uint8_t smps_mode);

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
void wma_register_ll_stats_event_handler(tp_wma_handle wma_handle);

QDF_STATUS wma_process_ll_stats_clear_req
	(tp_wma_handle wma, const tpSirLLStatsClearReq clearReq);

QDF_STATUS wma_process_ll_stats_set_req
	(tp_wma_handle wma, const tpSirLLStatsSetReq setReq);

QDF_STATUS wma_process_ll_stats_get_req
	(tp_wma_handle wma, const tpSirLLStatsGetReq getReq) ;

int wma_unified_link_iface_stats_event_handler(void *handle,
					       uint8_t *cmd_param_info,
					       uint32_t len);
#endif

void wma_post_link_status(tAniGetLinkStatus *pGetLinkStatus,
			  uint8_t link_status);

int wma_link_status_event_handler(void *handle, uint8_t *cmd_param_info,
				  uint32_t len);

int wma_stats_event_handler(void *handle, uint8_t *cmd_param_info,
			    uint32_t len);

QDF_STATUS wma_send_link_speed(uint32_t link_speed);

int wma_link_speed_event_handler(void *handle, uint8_t *cmd_param_info,
				 uint32_t len);

QDF_STATUS wma_wni_cfg_dnld(tp_wma_handle wma_handle);

int wma_unified_debug_print_event_handler(void *handle, uint8_t *datap,
					  uint32_t len);

bool wma_is_sap_active(tp_wma_handle wma_handle);

bool wma_is_p2p_go_active(tp_wma_handle wma_handle);

bool wma_is_p2p_cli_active(tp_wma_handle wma_handle);

bool wma_is_sta_active(tp_wma_handle wma_handle);

WLAN_PHY_MODE wma_peer_phymode(tSirNwType nw_type, uint8_t sta_type,
			       uint8_t is_ht, uint8_t ch_width,
			       uint8_t is_vht);

int32_t wma_txrx_fw_stats_reset(tp_wma_handle wma_handle,
				uint8_t vdev_id, uint32_t value);

int32_t wma_set_txrx_fw_stats_level(tp_wma_handle wma_handle,
				    uint8_t vdev_id, uint32_t value);

void wma_get_stats_req(WMA_HANDLE handle,
		       tAniGetPEStatsReq *get_stats_param);

#if defined(QCA_WIFI_FTM)
void wma_utf_detach(tp_wma_handle wma_handle);

void wma_utf_attach(tp_wma_handle wma_handle);

QDF_STATUS
wma_process_ftm_command(tp_wma_handle wma_handle,
			struct ar6k_testmode_cmd_data *msg_buffer);
#endif

/*
 * wma_features.c functions declarations
 */

void wma_process_link_status_req(tp_wma_handle wma,
				 tAniGetLinkStatus *pGetLinkStatus);

#ifdef FEATURE_WLAN_LPHB
QDF_STATUS wma_process_lphb_conf_req(tp_wma_handle wma_handle,
				     tSirLPHBReq *lphb_conf_req);

#endif

QDF_STATUS wma_process_dhcp_ind(tp_wma_handle wma_handle,
				tAniDHCPInd *ta_dhcp_ind);

QDF_STATUS wma_get_link_speed(WMA_HANDLE handle, tSirLinkSpeedInfo *pLinkSpeed);

int wma_profile_data_report_event_handler(void *handle, uint8_t *event_buf,
				       uint32_t len);

QDF_STATUS wma_unified_fw_profiling_cmd(wmi_unified_t wmi_handle,
				uint32_t cmd, uint32_t value1, uint32_t value2);

void wma_wow_tx_complete(void *wma);

int wmi_unified_nat_keepalive_enable(tp_wma_handle wma, uint8_t vdev_id);

int wma_unified_csa_offload_enable(tp_wma_handle wma, uint8_t vdev_id);

#ifdef WLAN_FEATURE_NAN
int wma_nan_rsp_event_handler(void *handle, uint8_t *event_buf, uint32_t len);
#endif

#ifdef FEATURE_WLAN_TDLS
int wma_tdls_event_handler(void *handle, uint8_t *event, uint32_t len);
#endif

int wma_csa_offload_handler(void *handle, uint8_t *event, uint32_t len);

#ifdef WLAN_FEATURE_GTK_OFFLOAD
int wma_gtk_offload_status_event(void *handle, uint8_t *event, uint32_t len);
#endif

#ifdef FEATURE_OEM_DATA_SUPPORT
int wma_oem_data_response_handler(void *handle, uint8_t *datap,
				  uint32_t len);
#endif

void wma_register_dfs_event_handler(tp_wma_handle wma_handle);

int
wma_unified_dfs_phyerr_filter_offload_enable(tp_wma_handle wma_handle);

#if !defined(REMOVE_PKT_LOG)
QDF_STATUS wma_pktlog_wmi_send_cmd(WMA_HANDLE handle,
				   struct ath_pktlog_wmi_params *params);
#endif

int wma_wow_wakeup_host_event(void *handle, uint8_t *event,
				     uint32_t len);
int wma_pdev_resume_event_handler(void *handle, uint8_t *event, uint32_t len);

/**
 * wma_get_wow_bus_suspend() - check is wow bus suspended or not
 * @wma: wma handle
 *
 * Return: true/false
 */
static inline int wma_get_wow_bus_suspend(tp_wma_handle wma)
{

	return qdf_atomic_read(&wma->is_wow_bus_suspended);
}

QDF_STATUS wma_resume_req(tp_wma_handle wma, enum qdf_suspend_type type);

QDF_STATUS wma_wow_add_pattern(tp_wma_handle wma,
			struct wow_add_pattern *ptrn);

QDF_STATUS wma_wow_delete_user_pattern(tp_wma_handle wma,
			struct wow_delete_pattern *pattern);

QDF_STATUS wma_wow_enter(tp_wma_handle wma, tpSirHalWowlEnterParams info);

QDF_STATUS wma_wow_exit(tp_wma_handle wma, tpSirHalWowlExitParams info);

QDF_STATUS wma_suspend_req(tp_wma_handle wma, enum qdf_suspend_type type);
void wma_calculate_and_update_conn_state(tp_wma_handle wma);
void wma_update_conn_state(tp_wma_handle wma, uint32_t conn_mask);
void wma_update_conn_state(tp_wma_handle wma, uint32_t conn_mask);

void wma_del_ts_req(tp_wma_handle wma, tDelTsParams *msg);

void wma_aggr_qos_req(tp_wma_handle wma,
			     tAggrAddTsParams *pAggrQosRspMsg);

void wma_add_ts_req(tp_wma_handle wma, tAddTsParams *msg);

int wma_process_receive_filter_set_filter_req(tp_wma_handle wma_handle,
						     tSirRcvPktFilterCfgType *
						     rcv_filter_param);

int wma_process_receive_filter_clear_filter_req(tp_wma_handle wma_handle,
						       tSirRcvFltPktClearParam *
						       rcv_clear_param);

#ifdef FEATURE_WLAN_ESE
QDF_STATUS wma_process_tsm_stats_req(tp_wma_handle wma_handler,
				     void *pTsmStatsMsg);
QDF_STATUS wma_plm_start(tp_wma_handle wma, const tpSirPlmReq plm);
QDF_STATUS wma_plm_stop(tp_wma_handle wma, const tpSirPlmReq plm);
void wma_config_plm(tp_wma_handle wma, tpSirPlmReq plm);
#endif

QDF_STATUS wma_process_mcbc_set_filter_req(tp_wma_handle wma_handle,
					   tSirRcvFltMcAddrList * mcbc_param);
#ifdef WLAN_FEATURE_GTK_OFFLOAD
QDF_STATUS wma_process_gtk_offload_req(tp_wma_handle wma,
					      tpSirGtkOffloadParams params);

QDF_STATUS wma_process_gtk_offload_getinfo_req(tp_wma_handle wma,
					       tpSirGtkOffloadGetInfoRspParams
					       params);
#endif

QDF_STATUS wma_enable_arp_ns_offload(tp_wma_handle wma,
				     tpSirHostOffloadReq pHostOffloadParams,
				     bool bArpOnly);

QDF_STATUS wma_process_cesium_enable_ind(tp_wma_handle wma);

QDF_STATUS wma_process_get_peer_info_req
	(tp_wma_handle wma, tSirIbssGetPeerInfoReqParams *pReq);

QDF_STATUS wma_process_tx_fail_monitor_ind
	(tp_wma_handle wma, tAniTXFailMonitorInd *pReq);

QDF_STATUS wma_process_rmc_enable_ind(tp_wma_handle wma);

QDF_STATUS wma_process_rmc_disable_ind(tp_wma_handle wma);

QDF_STATUS wma_process_rmc_action_period_ind(tp_wma_handle wma);

QDF_STATUS wma_process_add_periodic_tx_ptrn_ind(WMA_HANDLE handle,
						tSirAddPeriodicTxPtrn *
						pAddPeriodicTxPtrnParams);

QDF_STATUS wma_process_del_periodic_tx_ptrn_ind(WMA_HANDLE handle,
						tSirDelPeriodicTxPtrn *
						pDelPeriodicTxPtrnParams);

#ifdef WLAN_FEATURE_STATS_EXT
QDF_STATUS wma_stats_ext_req(void *wma_ptr, tpStatsExtRequest preq);
#endif

QDF_STATUS wma_process_ibss_route_table_update_ind(void *wma_handle,
						   tAniIbssRouteTable * pData);

#ifdef WLAN_FEATURE_EXTWOW_SUPPORT
QDF_STATUS wma_enable_ext_wow(tp_wma_handle wma, tpSirExtWoWParams params);

int wma_set_app_type1_params_in_fw(tp_wma_handle wma,
				   tpSirAppType1Params appType1Params);

QDF_STATUS wma_set_app_type2_params_in_fw(tp_wma_handle wma,
				   tpSirAppType2Params appType2Params);
#endif

#ifdef FEATURE_WLAN_AUTO_SHUTDOWN
int wma_auto_shutdown_event_handler(void *handle, uint8_t *event,
				    uint32_t len);

QDF_STATUS wma_set_auto_shutdown_timer_req(tp_wma_handle wma_handle,
					   tSirAutoShutdownCmdParams *
					   auto_sh_cmd);
#endif

#ifdef WLAN_FEATURE_TSF
int wma_vdev_tsf_handler(void *handle, uint8_t *data, uint32_t data_len);
QDF_STATUS wma_capture_tsf(tp_wma_handle wma_handle, uint32_t vdev_id);
QDF_STATUS wma_reset_tsf_gpio(tp_wma_handle wma_handle, uint32_t vdev_id);
QDF_STATUS wma_set_tsf_gpio_pin(WMA_HANDLE handle, uint32_t pin);
#else
static inline QDF_STATUS wma_capture_tsf(tp_wma_handle wma_handle,
					uint32_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wma_reset_tsf_gpio(tp_wma_handle wma_handle,
					 uint32_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline int wma_vdev_tsf_handler(void *handle, uint8_t *data,
					uint32_t data_len)
{
	return 0;
}

static inline QDF_STATUS wma_set_tsf_gpio_pin(WMA_HANDLE handle, uint32_t pin)
{
	return QDF_STATUS_E_INVAL;
}
#endif
QDF_STATUS wma_set_wisa_params(tp_wma_handle wma, struct sir_wisa_params *wisa);

#ifdef WLAN_FEATURE_NAN
QDF_STATUS wma_nan_req(void *wma_ptr, tpNanRequest nan_req);
#endif

#ifdef DHCP_SERVER_OFFLOAD
int wma_process_dhcpserver_offload(tp_wma_handle wma_handle,
				   tSirDhcpSrvOffloadInfo *
				   pDhcpSrvOffloadInfo);
#endif

#ifdef WLAN_FEATURE_GPIO_LED_FLASHING
QDF_STATUS wma_set_led_flashing(tp_wma_handle wma_handle,
				tSirLedFlashingReq *flashing);
#endif

#ifdef FEATURE_WLAN_CH_AVOID
int wma_channel_avoid_evt_handler(void *handle, uint8_t *event,
					 uint32_t len);

QDF_STATUS wma_process_ch_avoid_update_req(tp_wma_handle wma_handle,
					   tSirChAvoidUpdateReq *
					   ch_avoid_update_req);
#endif

QDF_STATUS wma_suspend_target(WMA_HANDLE handle, int disable_target_intr);

#ifdef FEATURE_WLAN_TDLS

QDF_STATUS wma_update_fw_tdls_state(WMA_HANDLE handle, void *pwmaTdlsparams);
int wma_update_tdls_peer_state(WMA_HANDLE handle,
			       tTdlsPeerStateParams *peerStateParams);
/**
 * wma_set_tdls_offchan_mode() - set tdls off channel mode
 * @handle: wma handle
 * @chan_switch_params: Pointer to tdls channel switch parameter structure
 *
 * This function sets tdls off channel mode
 *
 * Return: 0 on success; negative errno otherwise
 */
QDF_STATUS wma_set_tdls_offchan_mode(WMA_HANDLE wma_handle,
			      tdls_chan_switch_params *chan_switch_params);
#endif

struct ieee80211com *wma_dfs_attach(struct ieee80211com *dfs_ic);

void wma_dfs_detach(struct ieee80211com *dfs_ic);

void wma_dfs_configure(struct ieee80211com *ic);

struct dfs_ieee80211_channel *wma_dfs_configure_channel(
						struct ieee80211com *dfs_ic,
						uint32_t band_center_freq1,
						uint32_t band_center_freq2,
						struct wma_vdev_start_req
						*req);
void wma_set_sap_keepalive(tp_wma_handle wma, uint8_t vdev_id);

#ifdef REMOVE_PKT_LOG
static inline void wma_set_wifi_start_packet_stats(void *wma_handle,
		struct sir_wifi_start_log *start_log)
{
	return;
}
#endif
int wma_rssi_breached_event_handler(void *handle,
				u_int8_t  *cmd_param_info, u_int32_t len);
#ifdef WLAN_FEATURE_MEMDUMP
int wma_fw_mem_dump_event_handler(void *handle, u_int8_t *cmd_param_info,
				  u_int32_t len);
QDF_STATUS wma_process_fw_mem_dump_req(tp_wma_handle wma,
					struct fw_dump_req *mem_dump_req);
#else
static inline int wma_fw_mem_dump_event_handler(void *handle,
			u_int8_t *cmd_param_info, u_int32_t len)
{
	return 0;
}

static inline QDF_STATUS wma_process_fw_mem_dump_req(tp_wma_handle wma,
						     void *mem_dump_req)
{
	return QDF_STATUS_SUCCESS;
}
#endif
QDF_STATUS wma_process_set_ie_info(tp_wma_handle wma,
				   struct vdev_ie_info *ie_info);
int wma_peer_assoc_conf_handler(void *handle, uint8_t *cmd_param_info,
				uint32_t len);
int wma_vdev_delete_handler(void *handle, uint8_t *cmd_param_info,
				uint32_t len);

int wma_peer_delete_handler(void *handle, uint8_t *cmd_param_info,
				uint32_t len);
void wma_remove_req(tp_wma_handle wma, uint8_t vdev_id,
			    uint8_t type);
int wma_p2p_lo_event_handler(void *handle, uint8_t *event_buf,
				uint32_t len);

QDF_STATUS wma_process_hal_pwr_dbg_cmd(WMA_HANDLE handle,
				       struct sir_mac_pwr_dbg_cmd *
				       sir_pwr_dbg_params);

#endif
