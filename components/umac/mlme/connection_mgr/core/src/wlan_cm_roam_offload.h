/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
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

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)

/**
 * cm_roam_state_change() - Post roam state change to roam state machine
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @requested_state: roam state to be set
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function posts roam state change to roam state machine handling
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_state_change(struct wlan_objmgr_pdev *pdev,
		     uint8_t vdev_id,
		     enum roam_offload_state requested_state,
		     uint8_t reason);

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
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_stop_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		 uint8_t reason);

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
 * cm_dump_freq_list() - dump chan list
 * @chan_info: chan info to dump
 *
 * Return: void
 */
void cm_dump_freq_list(struct rso_chan_info *chan_info);

#if defined(WLAN_FEATURE_ROAM_OFFLOAD) && defined(FEATURE_CM_ENABLE)
/**
 * cm_start_roam_invoke() - Validate and send Roam invoke req to CM
 * @pdev: Pdev pointer
 * @vdev: vdev
 * @bssid: Target bssid
 * @chan_freq: channel frequency on which reassoc should be send
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_start_roam_invoke(struct wlan_objmgr_psoc *psoc,
		     struct wlan_objmgr_vdev *vdev,
		     struct qdf_mac_addr *bssid,
		     uint32_t chan_freq);
#else
static inline QDF_STATUS
cm_start_roam_invoke(struct wlan_objmgr_psoc *psoc,
		     struct wlan_objmgr_vdev *vdev,
		     struct qdf_mac_addr *bssid,
		     uint32_t chan_freq)
{
	return QDF_STATUS_SUCCESS;
}
#endif
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
 *
 * Return: None
 */
void cm_update_pmk_cache_ft(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id);

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

/**
 * cm_roam_start_init_on_connect() - init roaming
 * @pdev: pdev pointer
 * @vdev_id: vdev_id
 *
 * Return: void
 */
void cm_roam_start_init_on_connect(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id);

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
 *
 * Return: void
 */
void cm_check_and_set_sae_single_pmk_cap(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id);
#else
static inline void
cm_store_sae_single_pmk_to_global_cache(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_vdev *vdev)
{}
static inline void
cm_check_and_set_sae_single_pmk_cap(struct wlan_objmgr_psoc *psoc,
				    uint8_t vdev_id)
{}
#endif

bool cm_is_auth_type_11r(struct wlan_mlme_psoc_ext_obj *mlme_obj,
			 struct wlan_objmgr_vdev *vdev,
			 bool mdie_present);
#endif /* _WLAN_CM_ROAM_OFFLOAD_H_ */
