/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_cm_roam_offload.h
 *
 * Implementation for the common roaming offload api interfaces.
 */

#ifndef _WLAN_CM_ROAM_OFFLOAD_H_
#define _WLAN_CM_ROAM_OFFLOAD_H_

#include "qdf_str.h"
#include "wlan_cm_roam_public_struct.h"

#if defined(WLAN_FEATURE_CONNECTIVITY_LOGGING) && \
    defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * cm_roam_scan_info_event() - send scan info to userspace
 * @psoc: psoc common object
 * @scan: roam scan data
 * @vdev_id: vdev id
 *
 * Return: None
 */
void cm_roam_scan_info_event(struct wlan_objmgr_psoc *psoc,
			     struct wmi_roam_scan_data *scan, uint8_t vdev_id);

/**
 * cm_roam_trigger_info_event() - send trigger info to userspace
 * @data: roam trigger data
 * @scan_data: Roam scan data
 * @vdev_id: vdev id
 * @is_full_scan: is full scan or partial scan
 *
 * Return: None
 */
void cm_roam_trigger_info_event(struct wmi_roam_trigger_info *data,
				struct wmi_roam_scan_data *scan_data,
				uint8_t vdev_id, bool is_full_scan);

/**
 * cm_roam_candidate_info_event() - send trigger info to userspace
 * @ap: roam candidate info
 * @cand_ap_idx: Candidate AP index
 *
 * Return: void
 */
void cm_roam_candidate_info_event(struct wmi_roam_candidate_info *ap,
				  uint8_t cand_ap_idx);

/**
 * cm_roam_result_info_event() - send scan results info to userspace
 * @psoc: Pointer to PSOC object
 * @trigger: Roam trigger data
 * @res: roam result data
 * @scan_data: Roam scan info
 * @vdev_id: vdev id
 *
 * Return: void
 */
void cm_roam_result_info_event(struct wlan_objmgr_psoc *psoc,
			       struct wmi_roam_trigger_info *trigger,
			       struct wmi_roam_result *res,
			       struct wmi_roam_scan_data *scan_data,
			       uint8_t vdev_id);
#else
static inline void
cm_roam_scan_info_event(struct wlan_objmgr_psoc *psoc,
			struct wmi_roam_scan_data *scan, uint8_t vdev_id)
{
}

static inline void
cm_roam_trigger_info_event(struct wmi_roam_trigger_info *data,
			   struct wmi_roam_scan_data *scan_data,
			   uint8_t vdev_id, bool is_full_scan)
{
}

static inline void
cm_roam_candidate_info_event(struct wmi_roam_candidate_info *ap,
			     uint8_t cand_idx)
{
}

static inline
void cm_roam_result_info_event(struct wlan_objmgr_psoc *psoc,
			       struct wmi_roam_trigger_info *trigger,
			       struct wmi_roam_result *res,
			       struct wmi_roam_scan_data *scan_data,
			       uint8_t vdev_id)
{
}
#endif /* WLAN_FEATURE_CONNECTIVITY_LOGGING */

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)

/**
 * cm_roam_state_change() - Post roam state change to roam state machine
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @requested_state: roam state to be set
 * @reason: reason for changing roam state for the requested vdev id
 * @send_resp: send rso stop response
 * @start_timer: start timer for rso stop
 *
 * This function posts roam state change to roam state machine handling
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_state_change(struct wlan_objmgr_pdev *pdev,
		     uint8_t vdev_id,
		     enum roam_offload_state requested_state,
		     uint8_t reason, bool *send_resp, bool start_timer);

/**
 * cm_handle_sta_sta_roaming_enablement() - To handle roaming in case
 * of STA + STA
 * @psoc: psoc common object
 * @curr_vdev_id: Vdev id
 *
 * This function is to process STA + STA concurrency scenarios after roaming
 * and take care of following:
 * 1. Set PCL to vdev/pdev as per DBS, SCC or MCC
 * 2. Enable/disable roaming based on the concurrency (DBS vs SCC/MCC) after
 * roaming
 *
 * Return: none
 */
void cm_handle_sta_sta_roaming_enablement(struct wlan_objmgr_psoc *psoc,
					  uint8_t curr_vdev_id);

/**
 * cm_roam_send_rso_cmd() - send rso command
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @rso_command: roam command to send
 * @reason: reason for changing roam state for the requested vdev id
 *
 * similar to csr_roam_offload_scan, will be used from many legacy
 * process directly, generate a new function wlan_cm_roam_send_rso_cmd
 * for external usage.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_roam_send_rso_cmd(struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id, uint8_t rso_command,
				uint8_t reason);

/**
 * cm_rso_set_roam_trigger() - Send roam trigger bitmap firmware
 * @pdev: Pointer to pdev
 * @vdev_id: vdev id
 * @triggers: Carries pointer of the object containing vdev id and
 *  roam_trigger_bitmap.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_rso_set_roam_trigger(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id,
				   struct wlan_roam_triggers *trigger);

/**
 * cm_roam_stop_req() - roam stop request handling
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 * @send_resp: send rso stop response
 * @start_timer: start timer for rso stop
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_stop_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		 uint8_t reason, bool *send_resp, bool start_timer);

/**
 * cm_roam_fill_rssi_change_params() - Fill roam scan rssi change parameters
 * @psoc: PSOC pointer
 * @vdev_id: vdev_id
 * @params: RSSI change parameters
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_fill_rssi_change_params(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
				struct wlan_roam_rssi_change_params *params);

/**
 * cm_roam_is_change_in_band_allowed  - Check whether change in roam band is
 * allowed in FW or not
 * @psoc: psoc pointer
 * @vdev_id: VDEV id
 * @roam_band_mask: band mask to check
 *
 * Return: return true if change in band allowed
 */
bool
cm_roam_is_change_in_band_allowed(struct wlan_objmgr_psoc *psoc,
				  uint8_t vdev_id, uint32_t roam_band_mask);

/**
 * cm_dump_freq_list() - dump chan list
 * @chan_info: chan info to dump
 *
 * Return: void
 */
void cm_dump_freq_list(struct rso_chan_info *chan_info);

/**
 * cm_start_roam_invoke() - Validate and send Roam invoke req to CM
 * @pdev: Pdev pointer
 * @vdev: vdev
 * @bssid: Target bssid
 * @chan_freq: channel frequency on which reassoc should be send
 * @source: source of the roam invoke
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_start_roam_invoke(struct wlan_objmgr_psoc *psoc,
		     struct wlan_objmgr_vdev *vdev,
		     struct qdf_mac_addr *bssid,
		     uint32_t chan_freq,
		     enum wlan_cm_source source);
#endif

#ifdef FEATURE_WLAN_ESE
/**
 * cm_create_roam_scan_channel_list() - create roam scan channel list
 * @pdev: pdev
 * @rso_cfg: roam config
 * @vdev_id: vdev id
 * @chan_freq_list: pointer to channel list
 * @num_chan: number of channels
 * @band: band enumeration
 *
 * This function modifies the roam scan channel list as per AP neighbor
 * report; AP neighbor report may be empty or may include only other AP
 * channels; in any case, we merge the channel list with the learned occupied
 * channels list.
 * if the band is 2.4G, then make sure channel list contains only 2.4G
 * valid channels if the band is 5G, then make sure channel list contains
 * only 5G valid channels
 *
 * Return: QDF_STATUS enumeration
 */
QDF_STATUS cm_create_roam_scan_channel_list(struct wlan_objmgr_pdev *pdev,
					    struct rso_config *rso_cfg,
					    uint8_t vdev_id,
					    qdf_freq_t *chan_freq_list,
					    uint8_t num_chan,
					    const enum band_info band);
#endif

QDF_STATUS cm_neighbor_roam_update_config(struct wlan_objmgr_pdev *pdev,
					  uint8_t vdev_id, uint8_t value,
					  uint8_t reason);
void cm_flush_roam_channel_list(struct rso_chan_info *channel_info);

QDF_STATUS cm_roam_control_restore_default_config(struct wlan_objmgr_pdev *pdev,
						  uint8_t vdev_id);

/**
 * cm_update_pmk_cache_ft - API to update MDID in PMKSA cache entry
 * @psoc: psoc pointer
 * @vdev_id: dvev ID
 * @pmk_cache: pmksa from the userspace
 *
 * Return: None
 */
void cm_update_pmk_cache_ft(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			    struct wlan_crypto_pmksa *pmk_cache);

/**
 * cm_lookup_pmkid_using_bssid() - lookup pmkid using bssid
 * @psoc: pointer to psoc
 * @vdev_id: vdev_id
 * @pmk_cache: pointer to pmk cache
 *
 * Return: true if pmkid is found else false
 */
bool cm_lookup_pmkid_using_bssid(struct wlan_objmgr_psoc *psoc,
				 uint8_t vdev_id,
				 struct wlan_crypto_pmksa *pmk_cache);

void cm_roam_restore_default_config(struct wlan_objmgr_pdev *pdev,
				    uint8_t vdev_id);

/**
 * cm_roam_send_disable_config() - Send roam module enable/disable cfg to fw
 * @psoc: PSOC pointer
 * @vdev_id: vdev id
 * @cfg: roaming enable/disable cfg
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_send_disable_config(struct wlan_objmgr_psoc *psoc,
			    uint8_t vdev_id, uint8_t cfg);

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * cm_roam_send_rt_stats_config() - Send roam event stats cfg value to FW
 * @psoc: PSOC pointer
 * @vdev_id: vdev id
 * @param_value: roam stats enable/disable cfg
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_send_rt_stats_config(struct wlan_objmgr_psoc *psoc,
			     uint8_t vdev_id, uint8_t param_value);
#else
static inline QDF_STATUS
cm_roam_send_rt_stats_config(struct wlan_objmgr_psoc *psoc,
			     uint8_t vdev_id, uint8_t param_value)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

#if defined(WLAN_SAE_SINGLE_PMK) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
void
cm_store_sae_single_pmk_to_global_cache(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_vdev *vdev);
/**
 * cm_check_and_set_sae_single_pmk_cap() - check if the Roamed AP support
 * roaming using single pmk
 * with same pmk or not
 * @psoc: psoc
 * @vdev_id: vdev id
 * @psk_pmk: pmk of roamed AP
 * @pmk_len: pml length
 *
 * Return: void
 */
void cm_check_and_set_sae_single_pmk_cap(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id, uint8_t *psk_pmk,
					 uint8_t pmk_len);
#else
static inline void
cm_store_sae_single_pmk_to_global_cache(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_vdev *vdev)
{}
static inline void
cm_check_and_set_sae_single_pmk_cap(struct wlan_objmgr_psoc *psoc,
				    uint8_t vdev_id, uint8_t *psk_pmk,
				    uint8_t pmk_len)
{}
#endif

bool cm_is_auth_type_11r(struct wlan_mlme_psoc_ext_obj *mlme_obj,
			 struct wlan_objmgr_vdev *vdev,
			 bool mdie_present);

/**
 * cm_update_owe_info() - Update owe transition mode element info
 * @vdev: Object manager VDEV
 * @rsp: connect resp from VDEV mgr
 * @vdev_id: vdev id
 *
 * Return: none
 */
void cm_update_owe_info(struct wlan_objmgr_vdev *vdev,
			struct wlan_cm_connect_resp *rsp, uint8_t vdev_id);

#ifdef WLAN_FEATURE_11BE_MLO
QDF_STATUS
cm_handle_mlo_rso_state_change(struct wlan_objmgr_pdev *pdev,
			       uint8_t *vdev_id,
			       uint8_t reason,
			       bool *is_rso_skip);
#else
static inline QDF_STATUS
cm_handle_mlo_rso_state_change(struct wlan_objmgr_pdev *pdev,
			       uint8_t *vdev_id,
			       uint8_t reason,
			       bool *is_rso_skip)
{
	return QDF_STATUS_E_NOSUPPORT;
}

#endif

#if defined(WLAN_FEATURE_CONNECTIVITY_LOGGING) && \
	defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * cm_roam_mgmt_frame_event() - Roam management frame event
 * @frame_data: frame_data
 * @scan_data: Roam scan data
 * @vdev_id: vdev_id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_mgmt_frame_event(struct roam_frame_info *frame_data,
			 struct wmi_roam_scan_data *scan_data, uint8_t vdev_id);

/**
 * cm_roam_btm_req_event  - Send BTM request related logging event
 * @btm_data: BTM trigger related data
 * @vdev_id: Vdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_btm_req_event(struct wmi_roam_btm_trigger_data *btm_data,
		      uint8_t vdev_id);

/**
 * cm_roam_btm_resp_event() - Send BTM response logging event
 * @trigger_info: Roam trigger related data
 * @btm_data: BTM response data
 * @vdev_id: Vdev id
 * @is_wtc: Is WTC or BTM response
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_btm_resp_event(struct wmi_roam_trigger_info *trigger_info,
		       struct roam_btm_response_data *btm_data,
		       uint8_t vdev_id, bool is_wtc);

/**
 * cm_roam_btm_query_event()  - Send BTM query logging event
 * @btm_data: BTM data
 * @vdev_id: Vdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_btm_query_event(struct wmi_neighbor_report_data *btm_data,
			uint8_t vdev_id);

/**
 * cm_roam_beacon_loss_disconnect_event() - Send BMISS disconnection logging
 * event
 * @psoc: Pointer to PSOC object
 * @bssid: BSSID
 * @vdev_id: Vdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_beacon_loss_disconnect_event(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr bssid,
				     uint8_t vdev_id);
#else
static inline QDF_STATUS
cm_roam_mgmt_frame_event(struct roam_frame_info *frame_data,
			 struct wmi_roam_scan_data *scan_data, uint8_t vdev_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
cm_roam_btm_req_event(struct wmi_roam_btm_trigger_data *btm_data,
		      uint8_t vdev_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
cm_roam_btm_resp_event(struct wmi_roam_trigger_info *trigger_info,
		       struct roam_btm_response_data *btm_data,
		       uint8_t vdev_id, bool is_wtc)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
cm_roam_btm_query_event(struct wmi_neighbor_report_data *btm_data,
			uint8_t vdev_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
cm_roam_beacon_loss_disconnect_event(struct wlan_objmgr_psoc *psoc,
				     struct qdf_mac_addr bssid,
				     uint8_t vdev_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif /* FEATURE_CONNECTIVITY_LOGGING */
#endif /* _WLAN_CM_ROAM_OFFLOAD_H_ */
