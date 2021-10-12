/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: wlan_cm_roam_ucfg_api.h
 *
 * Implementation for roaming public ucfg API interfaces.
 */

#ifndef _WLAN_CM_ROAM_UCFG_API_H_
#define _WLAN_CM_ROAM_UCFG_API_H_

#include "wlan_cm_roam_api.h"

/**
 * ucfg_user_space_enable_disable_rso() - Enable/Disable Roam Scan offload
 * to firmware.
 * @pdev: Pointer to pdev
 * @vdev_id: vdev id
 * @is_fast_roam_enabled: Value provided by userspace.
 * is_fast_roam_enabled - true: enable RSO if FastRoamEnabled ini is enabled
 *                        false: disable RSO
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
ucfg_user_space_enable_disable_rso(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id,
				   const bool is_fast_roam_enabled);

/**
 * ucfg_is_roaming_enabled() - Check if roaming enabled
 * to firmware.
 * @psoc: psoc context
 * @vdev_id: vdev id
 *
 * Return: True if Roam state machine is in
 *	   WLAN_ROAM_RSO_ENABLED/WLAN_ROAMING_IN_PROG/WLAN_ROAM_SYNCH_IN_PROG
 */
bool ucfg_is_roaming_enabled(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id);

/*
 * ucfg_cm_abort_roam_scan() -abort current roam scan cycle by roam scan
 * offload module.
 * @pdev: Pointer to pdev
 * vdev_id - vdev Identifier
 *
 * Return QDF_STATUS
 */
QDF_STATUS ucfg_cm_abort_roam_scan(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id);

#ifdef FEATURE_WLAN_ESE
/**
 * ucfg_cm_set_ese_roam_scan_channel_list() - To set ese roam scan channel list
 * @pdev: pdev pointer
 * @vdev_id: vdev_id id
 * @chan_freq_list: Output channel list
 * @num_chan: Output number of channels
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_cm_set_ese_roam_scan_channel_list(struct wlan_objmgr_pdev *pdev,
						  uint8_t vdev_id,
						  qdf_freq_t *chan_freq_list,
						  uint8_t num_chan);
QDF_STATUS ucfg_cm_set_cckm_ie(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			       const uint8_t *cck_ie, const uint8_t cck_ie_len);
#endif

/**
 * ucfg_cm_get_roam_band() - Get roam band from rso config
 * @psoc: Pointer to psoc
 * @vdev_id: vdev id
 * @roam_band: Pointer of a buffer to fill the roam band
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_cm_get_roam_band(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
				 uint32_t *roam_band);

/**
 * ucfg_cm_rso_set_roam_trigger() - Send roam trigger bitmap firmware
 * @pdev: Pointer to pdev
 * @vdev_id: vdev id
 * @trigger: Carries pointer of the object containing vdev id and
 *  roam_trigger_bitmap.
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
ucfg_cm_rso_set_roam_trigger(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			     struct wlan_roam_triggers *trigger)
{
	return wlan_cm_rso_set_roam_trigger(pdev, vdev_id, trigger);
}

/**
 * ucfg_cm_update_session_assoc_ie() - Send assoc ie
 * @psoc: Pointer to psoc
 * @vdev_id: vdev id
 * @assoc_ie: assoc ir to update.
 *
 * Return: QDF_STATUS
 */
static inline void
ucfg_cm_update_session_assoc_ie(struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id,
				struct element_info *assoc_ie)
{
	cm_update_session_assoc_ie(psoc, vdev_id, assoc_ie);
}

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
static inline QDF_STATUS
ucfg_cm_update_roam_scan_scheme_bitmap(struct wlan_objmgr_psoc *psoc,
				       uint8_t vdev_id,
				       uint32_t roam_scan_scheme_bitmap)
{
	return wlan_cm_update_roam_scan_scheme_bitmap(psoc, vdev_id,
						      roam_scan_scheme_bitmap);
}

static inline QDF_STATUS
ucfg_cm_set_roam_band_mask(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			   uint32_t roam_band_mask)
{
	return wlan_cm_set_roam_band_bitmask(psoc, vdev_id, roam_band_mask);
}

static inline QDF_STATUS
ucfg_cm_set_roam_band_update(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	return wlan_cm_set_roam_band_update(psoc, vdev_id);
}

static inline bool
ucfg_cm_is_change_in_band_allowed(struct wlan_objmgr_psoc *psoc,
				  uint8_t vdev_id, uint32_t roam_band_mask)
{
	return cm_roam_is_change_in_band_allowed(psoc, vdev_id, roam_band_mask);
}

#else
static inline QDF_STATUS
ucfg_cm_update_roam_scan_scheme_bitmap(struct wlan_objmgr_psoc *psoc,
				       uint8_t vdev_id,
				       uint32_t roam_scan_scheme_bitmap)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_cm_set_roam_band_mask(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			   uint32_t roam_band_mask)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
ucfg_cm_set_roam_band_update(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline bool
ucfg_cm_is_change_in_band_allowed(struct wlan_objmgr_psoc *psoc,
				  uint8_t vdev_id, uint32_t roam_band_mask)
{
	return true;
}

#endif

/**
 * ucfg_wlan_cm_roam_invoke() - Invokes Roam request
 * @pdev: Pointer to pdev
 * @vdev_id: vdev id
 * @bssid: Pointer to bssid to look for in scan cache
 * @ch_freq: channel on which reassoc should be send
 * @source: source of roam
 *
 * Return: true or false
 */
QDF_STATUS
ucfg_wlan_cm_roam_invoke(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			 struct qdf_mac_addr *bssid, qdf_freq_t ch_freq,
			 enum wlan_cm_source source);

#ifdef WLAN_FEATURE_FILS_SK
QDF_STATUS
ucfg_cm_update_fils_config(struct wlan_objmgr_psoc *psoc,
			   uint8_t vdev_id,
			   struct wlan_fils_con_info *fils_info);
#else
static inline QDF_STATUS
ucfg_cm_update_fils_config(struct wlan_objmgr_psoc *psoc,
			   uint8_t vdev_id,
			   struct wlan_fils_con_info *fils_info)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_HOST_ROAM
void ucfg_cm_ft_reset(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cm_set_ft_ies() - to set FT IEs
 * @pdev: pdev ctx
 * @vdev_id: vdev identifier
 * @ft_ies: pointer to FT IEs
 * @ft_ies_length: length of FT IEs
 *
 * Each time the supplicant sends down the FT IEs to the driver. This function
 * is called in SME. This function packages and sends the FT IEs to PE.
 *
 * Return: none
 */
void ucfg_cm_set_ft_ies(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			const uint8_t *ft_ies, uint16_t ft_ies_length);

/**
 * ucfg_cm_check_ft_status() - Check for key wait status in FT mode
 * @pdev: pdev ctx
 * @vdev_id: vdev identifier
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_cm_check_ft_status(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id);

/**
 * ucfg_cm_ft_key_ready_for_install() - API to check ft key ready for install
 * @vdev: pdev handle
 *
 * It is only applicable for LFR2.0 enabled
 *
 * Return: true when ft key is ready otherwise false
 */
bool ucfg_cm_ft_key_ready_for_install(struct wlan_objmgr_vdev *vdev);
void ucfg_cm_set_ft_pre_auth_state(struct wlan_objmgr_vdev *vdev, bool state);

#else /* WLAN_FEATURE_HOST_ROAM */

static inline void ucfg_cm_ft_reset(struct wlan_objmgr_vdev *vdev) {}
static inline
void ucfg_cm_set_ft_ies(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			const uint8_t *ft_ies, uint16_t ft_ies_length) {}

static inline
QDF_STATUS ucfg_cm_check_ft_status(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id)
{
	return QDF_STATUS_E_FAILURE;
}

static inline void ucfg_cm_set_ft_pre_auth_state(struct wlan_objmgr_vdev *vdev,
						 bool state) {}
#endif /* WLAN_FEATURE_HOST_ROAM */

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * ucfg_cm_reset_key() -Reset key information
 * @pdev: pdev handle
 * @vdev_id: vdev identifier
 *
 * Return: None
 */
void ucfg_cm_reset_key(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id);
#else
static inline void
ucfg_cm_reset_key(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id) {}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

#endif /* _WLAN_CM_ROAM_UCFG_API_H_ */
