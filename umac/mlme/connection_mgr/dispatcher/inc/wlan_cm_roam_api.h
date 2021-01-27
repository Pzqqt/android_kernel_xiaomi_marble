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
 * DOC: wlan_cm_roam_api.h
 *
 * Implementation for the Common Roaming interfaces.
 */

#ifndef WLAN_CM_ROAM_API_H__
#define WLAN_CM_ROAM_API_H__

#include "wlan_mlme_dbg.h"
#include "../../core/src/wlan_cm_roam_offload.h"
#include "wlan_mlme_main.h"
#include "wlan_mlme_api.h"

/* Default value of reason code */
#define DISABLE_VENDOR_BTM_CONFIG 2

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * wlan_cm_enable_roaming_on_connected_sta() - Enable roaming on other connected
 * sta vdev
 * @pdev: pointer to pdev object
 * @vdev_id: vdev id on which roaming should not be enabled
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_enable_roaming_on_connected_sta(struct wlan_objmgr_pdev *pdev,
					uint8_t vdev_id);

/**
 * wlan_cm_roam_send_rso_cmd() - send rso command
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
QDF_STATUS wlan_cm_roam_send_rso_cmd(struct wlan_objmgr_psoc *psoc,
				     uint8_t vdev_id, uint8_t rso_command,
				     uint8_t reason);

/**
 * wlan_cm_roam_state_change() - Post roam state change to roam state machine
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @requested_state: roam state to be set
 * @reason: reason for changing roam state for the requested vdev id
 *
 * This function posts roam state change to roam state machine handling
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_roam_state_change(struct wlan_objmgr_pdev *pdev,
				     uint8_t vdev_id,
				     enum roam_offload_state requested_state,
				     uint8_t reason);

/**
 * csr_roam_update_cfg() - Process RSO update cfg request
 * @psoc: psoc context
 * @vdev_id: vdev id
 * @reason: reason for requesting RSO update cfg
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_roam_update_cfg(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		     uint8_t reason);

#ifndef FEATURE_CM_ENABLE
/**
 * wlan_cm_roam_cmd_allowed() - check roam cmd is allowed or not
 * @psoc: pointer to psoc object
 * @vdev_id: vdev id
 * @rso_command: roam scan offload command
 * @reason: reason to roam
 *
 * This function gets called to check roam cmd is allowed or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_roam_cmd_allowed(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			 uint8_t rso_command, uint8_t reason);
#endif

/**
 * wlan_cm_roam_scan_offload_rsp() - send roam scan offload response message
 * @vdev_id: vdev id
 * @reason: reason to roam
 *
 * This function gets called to send roam scan offload response message
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_roam_scan_offload_rsp(uint8_t vdev_id, uint8_t reason);

/**
 * wlan_cm_send_beacon_miss() - initiate beacon miss
 * @vdev_id: vdev id
 * @rssi: AP rssi
 *
 * Return: void
 */
void wlan_cm_send_beacon_miss(uint8_t vdev_id, int32_t rssi);

/**
 * wlan_cm_roam_neighbor_proceed_with_handoff_req() - invoke host handover to
 * new AP
 * @vdev_id: vdev id
 *
 * This function gets called to invoke host handover to new AP
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_roam_neighbor_proceed_with_handoff_req(uint8_t vdev_id);

/**
 * wlan_cm_is_sta_connected() - check if STA is connected
 * @vdev_id: vdev id
 *
 * Return: bool
 */
bool wlan_cm_is_sta_connected(uint8_t vdev_id);

#else
static inline
QDF_STATUS wlan_cm_roam_send_rso_cmd(struct wlan_objmgr_psoc *psoc,
				     uint8_t vdev_id, uint8_t rso_command,
				     uint8_t reason)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
wlan_roam_update_cfg(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
		     uint8_t reason)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
QDF_STATUS wlan_cm_roam_state_change(struct wlan_objmgr_pdev *pdev,
				     uint8_t vdev_id,
				     enum roam_offload_state requested_state,
				     uint8_t reason)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
wlan_cm_enable_roaming_on_connected_sta(struct wlan_objmgr_pdev *pdev,
					uint8_t vdev_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
#define wlan_is_roam_offload_enabled(lfr) \
	(lfr.lfr3_roaming_offload)
#else
#define wlan_is_roam_offload_enabled(lfr)  false
#endif

/**
 * wlan_cm_neighbor_roam_in_progress() -Check if STA is in the middle of
 * roaming states
 * @psoc: psoc
 * @vdev_id: vdev id
 *
 * Return: True or False
 */
bool wlan_cm_host_roam_in_progress(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id);

/**
 * cm_roam_acquire_lock() - Wrapper for sme_acquire_global_lock.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_roam_acquire_lock(void);

/**
 * cm_roam_release_lock() - Wrapper for sme_release_global_lock()
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_roam_release_lock(void);

/**
 * cm_roam_get_requestor_string() - RSO control requestor to string api
 * @requestor: Requestor of type enum wlan_cm_rso_control_requestor
 *
 * Return: Pointer to converted string
 */
char
*cm_roam_get_requestor_string(enum wlan_cm_rso_control_requestor requestor);

/**
 * wlan_cm_rso_set_roam_trigger() - Send roam trigger bitmap firmware
 * @pdev: Pointer to pdev
 * @vdev_id: vdev id
 * @triggers: Carries pointer of the object containing vdev id and
 *  roam_trigger_bitmap.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_rso_set_roam_trigger(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			     struct wlan_roam_triggers *trigger_data);

/**
 * wlan_cm_disable_rso() - Disable roam scan offload to firmware
 * @pdev: Pointer to pdev
 * @vdev_id: vdev id
 * @requestor: RSO disable requestor
 * @reason: Reason for RSO disable
 *
 * Return:  QDF_STATUS
 */
QDF_STATUS wlan_cm_disable_rso(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			       enum wlan_cm_rso_control_requestor requestor,
			       uint8_t reason);

/**
 * ucfg_cm_enable_rso() - Enable roam scan offload to firmware
 * @pdev: Pointer to pdev
 * @vdev_id: vdev id
 * @requestor: RSO disable requestor
 * @reason: Reason for RSO disable
 *
 * Return:  QDF_STATUS
 */
QDF_STATUS wlan_cm_enable_rso(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
			      enum wlan_cm_rso_control_requestor requestor,
			      uint8_t reason);

/**
 * wlan_cm_abort_rso() - Enable roam scan offload to firmware
 * @pdev: Pointer to pdev
 * @vdev_id: vdev id
 *
 * Returns:
 * QDF_STATUS_E_BUSY if roam_synch is in progress and upper layer has to wait
 *                   before RSO stop cmd can be issued;
 * QDF_STATUS_SUCCESS if roam_synch is not outstanding. RSO stop cmd will be
 *                    issued with the global SME lock held in this case, and
 *                    uppler layer doesn't have to do any wait.
 */
QDF_STATUS wlan_cm_abort_rso(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id);

/**
 * wlan_cm_roaming_in_progress() - check if roaming is in progress
 * @pdev: Pointer to pdev
 * @vdev_id: vdev id
 *
 * Return: true or false
 */
bool
wlan_cm_roaming_in_progress(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id);

/**
 * wlan_cm_roam_stop_req() - roam stop request handling
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @reason: reason for changing roam state for the requested vdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_roam_stop_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
				 uint8_t reason);

/**
 * wlan_cm_roam_cfg_get_value  - Get RSO config value from mlme vdev private
 * object
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @roam_cfg_type: Value needed
 * @dst_config: Destination config
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_roam_cfg_get_value(struct wlan_objmgr_psoc *psoc,
				      uint8_t vdev_id,
				      enum roam_cfg_param roam_cfg_type,
				      struct cm_roam_values_copy *dst_config);

/**
 * wlan_cm_roam_cfg_set_value  - Set RSO config value
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @roam_cfg_type: Roam configuration type to set
 * @src_config: Source config
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_roam_cfg_set_value(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			   enum roam_cfg_param roam_cfg_type,
			   struct cm_roam_values_copy *src_config);


struct rso_config *wlan_cm_get_rso_config_fl(struct wlan_objmgr_vdev *vdev,
					     const char *func, uint32_t line);

/**
 * wlan_cm_get_rso_config  - get per vdev RSO config
 * @vdev: vdev pointer
 *
 * Return: rso config pointer
 */
#define wlan_cm_get_rso_config(vdev) \
	wlan_cm_get_rso_config_fl(vdev, __func__, __LINE__)

/**
 * wlan_cm_set_disable_hi_rssi  - set disable hi rssi config
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @value: value to set
 *
 * Return: void
 */
void wlan_cm_set_disable_hi_rssi(struct wlan_objmgr_pdev *pdev,
				 uint8_t vdev_id, bool value);

#ifdef FEATURE_WLAN_ESE
/**
 * wlan_cm_set_ese_assoc  - set ese assoc
 * @pdev: pdev pointer
 * @vdev_id: vdev
 * @value: value to set
 *
 * Return: void
 */
void wlan_cm_set_ese_assoc(struct wlan_objmgr_pdev *pdev,
			   uint8_t vdev_id, bool value);

/**
 * wlan_cm_get_ese_assoc  - get ese assoc
 * @pdev: pdev pointer
 * @vdev_id: vdev
 *
 * Return: value
 */
bool wlan_cm_get_ese_assoc(struct wlan_objmgr_pdev *pdev,
			   uint8_t vdev_id);
void wlan_cm_ese_populate_addtional_ies(struct wlan_objmgr_pdev *pdev,
			struct wlan_mlme_psoc_ext_obj *mlme_obj,
			uint8_t vdev_id,
			struct wlan_roam_scan_offload_params *rso_mode_cfg);
#else
static inline void wlan_cm_set_ese_assoc(struct wlan_objmgr_pdev *pdev,
					 uint8_t vdev_id, bool value) {}
static inline
bool wlan_cm_get_ese_assoc(struct wlan_objmgr_pdev *pdev,
			   uint8_t vdev_id)
{
	return false;
}
static inline void wlan_cm_ese_populate_addtional_ies(
		struct wlan_objmgr_pdev *pdev,
		struct wlan_mlme_psoc_ext_obj *mlme_obj,
		uint8_t vdev_id,
		struct wlan_roam_scan_offload_params *rso_mode_cfg)
{}
#endif

/**
 * wlan_roam_reset_roam_params  - reset_roam params
 * @psoc: vdev pointer
 *
 * Return: QDF_STATUS
 */
void wlan_roam_reset_roam_params(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cm_rso_config_init  - initialize RSO config
 * @vdev: vdev pointer
 * @rso_cfg: rso config to init
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_rso_config_init(struct wlan_objmgr_vdev *vdev,
				   struct rso_config *rso_cfg);

/**
 * wlan_cm_rso_config_deinit  - deinit RSO config
 * @vdev: vdev pointer
 * @rso_cfg: rso config to deinit
 *
 * Return: void
 */
void wlan_cm_rso_config_deinit(struct wlan_objmgr_vdev *vdev,
			       struct rso_config *rso_cfg);

/**
 * wlan_cm_fill_crypto_filter_from_vdev() - fill scan filter crypto from vdev
 * given vdev id
 * @vdev: vdev
 * @filter: filetr to fill
 *
 * Return: void
 */
void wlan_cm_fill_crypto_filter_from_vdev(struct wlan_objmgr_vdev *vdev,
					  struct scan_filter *filter);

/**
 * wlan_cm_init_occupied_ch_freq_list  - init occupied chan freq list
 * @pdev: pdev pointer
 * @psoc: psoc
 * @vdev_id: vdev_id of vdev for which init is required
 *
 * Return: void
 */
void wlan_cm_init_occupied_ch_freq_list(struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id);

uint32_t cm_crpto_authmode_to_wmi_authmode(int32_t authmodeset,
					   int32_t akm, int32_t ucastcipherset);
uint8_t *wlan_cm_get_rrm_cap_ie_data(void);

/**
 * wlan_cm_append_assoc_ies() - Append specific IE to assoc IE's buffer
 * @req_buf: Pointer to Roam offload scan request
 * @ie_id: IE ID to be appended
 * @ie_len: IE length to be appended
 * @ie_data: IE data to be appended
 *
 * Return: None
 */
void wlan_cm_append_assoc_ies(struct wlan_roam_scan_offload_params *rso_mode_cfg,
			      uint8_t ie_id, uint8_t ie_len,
			      const uint8_t *ie_data);
/**
 * wlan_add_supported_5Ghz_channels()- Add valid 5Ghz channels
 * in Join req.
 * @psoc: psoc ptr
 * @pdev: pdev
 * @chan_list: Pointer to channel list buffer to populate
 * @num_chan: Pointer to number of channels value to update
 * @supp_chan_ie: Boolean to check if we need to populate as IE
 *
 * This function is called to update valid 5Ghz channels
 * in Join req. If @supp_chan_ie is true, supported channels IE
 * format[chan num 1, num of channels 1, chan num 2, num of
 * channels 2, ..] is populated. Else, @chan_list would be a list
 * of supported channels[chan num 1, chan num 2..]
 *
 * Return: void
 */
void wlan_add_supported_5Ghz_channels(struct wlan_objmgr_psoc *psoc,
				      struct wlan_objmgr_pdev *pdev,
				      uint8_t *chan_list,
				      uint8_t *num_chnl,
				      bool supp_chan_ie);
#ifdef WLAN_ADAPTIVE_11R
/**
 * wlan_get_adaptive_11r_enabled() - Function to check if adaptive 11r
 * ini is enabled or disabled
 * @mac: pointer to mac context
 *
 * Return: true if adaptive 11r is enabled
 */
static inline bool
wlan_get_adaptive_11r_enabled(struct wlan_mlme_lfr_cfg *lfr_cfg)
{
	return lfr_cfg->enable_adaptive_11r;
}
#else
static inline bool
wlan_get_adaptive_11r_enabled(struct wlan_mlme_lfr_cfg *lfr_cfg)
{
	return false;
}
#endif

#ifdef WLAN_FEATURE_FILS_SK
/**
 * wlan_cm_get_fils_connection_info  - Copy fils connection information from
 * mlme vdev private object
 * @psoc: Pointer to psoc object
 * @dst_fils_info: Destination buffer to copy the fils info
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS
 */
struct wlan_fils_connection_info *wlan_cm_get_fils_connection_info(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id);

#ifndef FEATURE_CM_ENABLE
/**
 * wlan_cm_update_mlme_fils_connection_info  - Update FILS connection info
 * to mlme vdev private object
 * @psoc: Pointer to psoc object
 * @src_fils_info: Current profile FILS connection information
 * @vdev_id: vdev_id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_update_mlme_fils_connection_info(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_fils_connection_info *src_fils_info,
		uint8_t vdev_id);
#else
/**
 * wlan_cm_update_mlme_fils_info  - Update FILS connection info
 * to mlme vdev private object
 * @vdev: Pointer to pdev object
 * @src_fils_info: Current profile FILS connection information
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_update_mlme_fils_info(struct wlan_objmgr_vdev *vdev,
			      struct wlan_fils_con_info *src_fils_info);
#endif
/**
 * wlan_cm_update_fils_ft - Update the FILS FT derived to mlme
 * @psoc: Psoc pointer
 * @vdev_id: vdev id
 * @fils_ft: Pointer to FILS FT
 * @fils_ft_len: FILS FT length
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_update_fils_ft(struct wlan_objmgr_psoc *psoc,
				  uint8_t vdev_id, uint8_t *fils_ft,
				  uint8_t fils_ft_len);

/**
 * wlan_cm_update_hlp_info - API to save HLP IE
 * @psoc: Pointer to psoc
 * @gen_ie: IE buffer to store
 * @len: length of the IE buffer @gen_ie
 * @vdev_id: vdev id
 * @flush: Flush the older saved HLP if any
 *
 * Return: None
 */
void wlan_cm_update_hlp_info(struct wlan_objmgr_psoc *psoc,
			     const uint8_t *gen_ie, uint16_t len,
			     uint8_t vdev_id, bool flush);
#else
static inline
struct wlan_fils_connection_info *wlan_cm_get_fils_connection_info(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id)
{
	return NULL;
}

static inline void wlan_cm_update_hlp_info(struct wlan_objmgr_psoc *psoc,
					   const uint8_t *gen_ie, uint16_t len,
					   uint8_t vdev_id, bool flush)
{}
#endif

#if defined(WLAN_SAE_SINGLE_PMK) && defined(WLAN_FEATURE_ROAM_OFFLOAD)
void
cm_store_sae_single_pmk_to_global_cache(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_vdev *vdev);
#else
static inline void
cm_store_sae_single_pmk_to_global_cache(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_vdev *vdev)
{}
#endif

#ifdef FEATURE_CM_ENABLE
/**
 * cm_roam_start_init_on_connect() - init roaming
 * @pdev: pdev pointer
 * @vdev_id: vdev_id
 *
 * Return: void
 */
void cm_roam_start_init_on_connect(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id);
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * wlan_cm_roam_extract_btm_response() - Extract BTM rsp stats
 * @wmi:       wmi handle
 * @evt_buf:   Pointer to the event buffer
 * @dst:       Pointer to destination structure to fill data
 * @idx:       TLV id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_roam_extract_btm_response(wmi_unified_t wmi, void *evt_buf,
				  struct roam_btm_response_data *dst,
				  uint8_t idx);

/**
 * wlan_cm_roam_extract_roam_initial_info() - Extract Roam Initial stats
 * @wmi:       wmi handle
 * @evt_buf:   Pointer to the event buffer
 * @dst:       Pointer to destination structure to fill data
 * @idx:       TLV id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_roam_extract_roam_initial_info(wmi_unified_t wmi, void *evt_buf,
				       struct roam_initial_data *dst,
				       uint8_t idx);

/**
 * wlan_cm_roam_extract_roam_msg_info() - Extract Roam msg stats
 * @wmi:       wmi handle
 * @evt_buf:   Pointer to the event buffer
 * @dst:       Pointer to destination structure to fill data
 * @idx:       TLV id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_roam_extract_roam_msg_info(wmi_unified_t wmi, void *evt_buf,
				   struct roam_msg_info *dst, uint8_t idx);

/**
 * wlan_cm_roam_activate_pcl_per_vdev() - Set the PCL command to be sent per
 * vdev instead of pdev.
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 * @pcl_per_vdev: Activate vdev PCL type. 1- VDEV PCL, 0- PDEV PCL
 *
 * pcl_per_vdev will be set when:
 *  STA + STA is connected in DBS mode and roaming init is done on
 *  the 2nd STA.
 *
 * pcl_per_vdev will be false when only 1 sta connection exists or
 * when 2nd sta gets disconnected
 *
 * Return: None
 */
void wlan_cm_roam_activate_pcl_per_vdev(struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id,
					bool pcl_per_vdev);

/**
 * wlan_cm_roam_is_pcl_per_vdev_active() - API to know if the pcl command needs
 * to be sent per vdev or not
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 *
 * Return: PCL level
 */
bool wlan_cm_roam_is_pcl_per_vdev_active(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id);

/**
 * wlan_cm_dual_sta_is_freq_allowed() - This API is used to check if the
 * provided frequency is allowed for the 2nd STA vdev for connection.
 * @psoc:   Pointer to PSOC object
 * @freq:   Frequency in the given frequency list for the STA that is about to
 * connect
 * @opmode: Operational mode
 *
 * This API will be called while filling scan filter channels during connection.
 *
 * Return: True if this channel is allowed for connection when dual sta roaming
 * is enabled
 */
bool
wlan_cm_dual_sta_is_freq_allowed(struct wlan_objmgr_psoc *psoc, uint32_t freq,
				 enum QDF_OPMODE opmode);

/**
 * wlan_cm_dual_sta_roam_update_connect_channels() - Fill the allowed channels
 * for connection of the 2nd STA based on the 1st STA connected band if dual
 * sta roaming is enabled.
 * @psoc:   Pointer to PSOC object
 * @filter: Pointer to scan filter
 *
 * Return: None
 */
void
wlan_cm_dual_sta_roam_update_connect_channels(struct wlan_objmgr_psoc *psoc,
					      struct scan_filter *filter);
/**
 * wlan_cm_roam_set_vendor_btm_params() - API to set vendor btm params
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 * @param: vendor configured roam trigger param
 *
 * Return: none
 */
void
wlan_cm_roam_set_vendor_btm_params(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id,
				   struct wlan_cm_roam_vendor_btm_params
								*param);
/**
 * wlan_cm_roam_disable_vendor_btm() - API to disable vendor btm by default
 * reason
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 *
 * Return: none
 */
void
wlan_cm_roam_disable_vendor_btm(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id);

/**
 * wlan_cm_roam_get_vendor_btm_params() - API to get vendor btm param
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 * @param: vendor configured roam trigger param
 *
 * Return: none
 */
void
wlan_cm_roam_get_vendor_btm_params(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id,
				   struct wlan_cm_roam_vendor_btm_params
								*param);

/**
 * wlan_cm_update_roam_scan_scheme_bitmap() - Set roam scan scheme bitmap for
 * each vdev
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 * @roam_scan_scheme_bitmap: bitmap of roam triggers for which partial roam
 * scan needs to be enabled
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_update_roam_scan_scheme_bitmap(struct wlan_objmgr_psoc *psoc,
				       uint8_t vdev_id,
				       uint32_t roam_scan_scheme_bitmap);

/**
 * wlan_cm_get_roam_scan_scheme_bitmap() - Get roam scan scheme bitmap value
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 *
 * Return: Roam scan scheme bitmap value
 */
uint32_t wlan_cm_get_roam_scan_scheme_bitmap(struct wlan_objmgr_psoc *psoc,
					     uint8_t vdev_id);
void wlan_cm_set_psk_pmk(struct wlan_objmgr_pdev *pdev,
			 uint8_t vdev_id, uint8_t *psk_pmk,
			 uint8_t pmk_len);

void wlan_cm_get_psk_pmk(struct wlan_objmgr_pdev *pdev,
			 uint8_t vdev_id, uint8_t *psk_pmk,
			 uint8_t *pmk_len);
#else
static inline
void wlan_cm_roam_activate_pcl_per_vdev(struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id,
					bool pcl_per_vdev)
{}

static inline
bool wlan_cm_roam_is_pcl_per_vdev_active(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id)
{
	return false;
}

static inline bool
wlan_cm_dual_sta_is_freq_allowed(struct wlan_objmgr_psoc *psoc, uint32_t freq,
				 enum QDF_OPMODE opmode)
{
	return true;
}

static inline void
wlan_cm_dual_sta_roam_update_connect_channels(struct wlan_objmgr_psoc *psoc,
					      struct scan_filter *filter)
{}

static inline QDF_STATUS
wlan_cm_roam_extract_btm_response(wmi_unified_t wmi, void *evt_buf,
				  struct roam_btm_response_data *dst,
				  uint8_t idx)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
wlan_cm_roam_extract_roam_initial_info(wmi_unified_t wmi, void *evt_buf,
				       struct roam_initial_data *dst,
				       uint8_t idx)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
wlan_cm_roam_extract_roam_msg_info(wmi_unified_t wmi, void *evt_buf,
				   struct roam_msg_info *dst, uint8_t idx)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline void
wlan_cm_roam_disable_vendor_btm(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id)
{}

static inline void
wlan_cm_roam_set_vendor_btm_params(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id,
				   struct wlan_cm_roam_vendor_btm_params *param)
{}

static inline void
wlan_cm_roam_get_vendor_btm_params(struct wlan_objmgr_psoc *psoc,
				   uint8_t vdev_id,
				   struct wlan_cm_roam_vendor_btm_params *param)
{}

static inline QDF_STATUS
wlan_cm_update_roam_scan_scheme_bitmap(struct wlan_objmgr_psoc *psoc,
				       uint8_t vdev_id,
				       uint32_t roam_scan_scheme_bitmap)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
uint32_t wlan_cm_get_roam_scan_scheme_bitmap(struct wlan_objmgr_psoc *psoc,
					     uint8_t vdev_id)
{
	return 0;
}
static inline void wlan_cm_set_psk_pmk(struct wlan_objmgr_pdev *pdev,
				       uint8_t vdev_id, uint8_t *psk_pmk,
				       uint8_t pmk_len)
{}
#endif  /* FEATURE_ROAM_OFFLOAD */
#endif  /* WLAN_CM_ROAM_API_H__ */
