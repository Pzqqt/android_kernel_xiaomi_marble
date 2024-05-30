/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "wlan_reg_ucfg_api.h"

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

/*
 * wlan_cm_handle_sta_sta_roaming_enablement() - Handle roaming in case
 * of STA + STA
 * @psoc: psoc common object
 * @vdev_id: Vdev id
 *
 * Wrapper function to cm_handle_sta_sta_roaming_enablement
 *
 * Return: none
 */
void wlan_cm_handle_sta_sta_roaming_enablement(struct wlan_objmgr_psoc *psoc,
					       uint8_t vdev_id);

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

/**
 * wlan_cm_send_beacon_miss() - initiate beacon miss
 * @vdev_id: vdev id
 * @rssi: AP rssi
 *
 * Return: void
 */
void wlan_cm_send_beacon_miss(uint8_t vdev_id, int32_t rssi);

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

static inline void
wlan_cm_handle_sta_sta_roaming_enablement(struct wlan_objmgr_psoc *psoc,
					  uint8_t vdev_id)
{
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
 * cm_roam_acquire_lock() - Wrapper for rso lock.
 * @vdev: Pointer to vdev
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_roam_acquire_lock(struct wlan_objmgr_vdev *vdev);

/**
 * cm_roam_release_lock() - Wrapper for rso lock
 * @vdev: Pointer to vdev
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_roam_release_lock(struct wlan_objmgr_vdev *vdev);

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

static inline void
wlan_cm_flush_roam_channel_list(struct rso_chan_info *channel_info)
{
	cm_flush_roam_channel_list(channel_info);
}

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

/**
 * wlan_cm_set_country_code - set country code to vdev rso config
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @cc: country code
 *
 * Return: void
 */
void wlan_cm_set_country_code(struct wlan_objmgr_pdev *pdev,
			      uint8_t vdev_id, uint8_t  *cc);

/**
 * wlan_cm_get_country_code - get country code from vdev rso config
 * @pdev: pdev pointer
 * @vdev_id: vdev id
 * @cc: country code
 *
 * Return: qdf status
 */
QDF_STATUS wlan_cm_get_country_code(struct wlan_objmgr_pdev *pdev,
				    uint8_t vdev_id, uint8_t *cc);

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

uint32_t cm_crypto_authmode_to_wmi_authmode(int32_t authmodeset,
					    int32_t akm,
					    int32_t ucastcipherset);
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

static inline
bool wlan_cm_is_auth_type_11r(struct wlan_mlme_psoc_ext_obj *mlme_obj,
			      struct wlan_objmgr_vdev *vdev,
			      bool mdie_present)
{
	return cm_is_auth_type_11r(mlme_obj, vdev, mdie_present);
}

static inline bool cm_is_open_mode(struct wlan_objmgr_vdev *vdev)
{
	return wlan_vdev_is_open_mode(vdev);
}

#ifdef FEATURE_WLAN_ESE
bool
cm_ese_open_present(struct wlan_objmgr_vdev *vdev,
		    struct wlan_mlme_psoc_ext_obj *mlme_obj,
		    bool ese_version_present);
bool
cm_is_ese_connection(struct wlan_objmgr_vdev *vdev, bool ese_version_present);
#else
static inline bool
cm_ese_open_present(struct wlan_objmgr_vdev *vdev,
		    struct wlan_mlme_psoc_ext_obj *mlme_obj,
		    bool ese_version_present)
{
	return false;
}
static inline bool
cm_is_ese_connection(struct wlan_objmgr_vdev *vdev, bool ese_version_present)
{
	return false;
}
#endif

/**
 * cm_roam_start_init_on_connect() - init roaming
 * @pdev: pdev pointer
 * @vdev_id: vdev_id
 *
 * Return: void
 */
void cm_roam_start_init_on_connect(struct wlan_objmgr_pdev *pdev,
				   uint8_t vdev_id);

void cm_update_session_assoc_ie(struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id,
				struct element_info *assoc_ie);

/**
 * wlan_cm_roam_invoke() - Validate and send Roam invoke req to CM
 * @pdev: Pdev pointer
 * @vdev_id: vdev_id
 * @bssid: Target bssid
 * @chan_freq: channel frequency on which reassoc should be send
 * @source: source of roam
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_roam_invoke(struct wlan_objmgr_pdev *pdev, uint8_t vdev_id,
		    struct qdf_mac_addr *bssid, qdf_freq_t chan_freq,
		    enum wlan_cm_source source);

/**
 * cm_is_fast_roam_enabled() - check fast roam enabled or not
 * @psoc: psoc pointer
 *
 * Return: true or false
 */
bool cm_is_fast_roam_enabled(struct wlan_objmgr_psoc *psoc);

/**
 * cm_is_rsn_or_8021x_sha256_auth_type() - check whether auth type is rsn
 * or 8021x_sha256 or not
 * @vdev: vdev object pointer
 *
 * Return: true, if auth type is rsn/8021x_sha256, false otherwise
 */
bool cm_is_rsn_or_8021x_sha256_auth_type(struct wlan_objmgr_vdev *vdev);

#ifdef WLAN_FEATURE_HOST_ROAM
/**
 * wlan_cm_host_roam_start() - fw host roam start handler
 * @msg: msg pointer
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_host_roam_start(struct scheduler_msg *msg);

/**
 * cm_mlme_roam_preauth_fail() - roam preauth fail
 * @vdev: VDEV object
 * @req: Connection manager roam request
 * @reason: connection manager connect fail reason
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_mlme_roam_preauth_fail(struct wlan_objmgr_vdev *vdev,
			  struct wlan_cm_roam_req *req,
			  enum wlan_cm_connect_fail_reason reason);

/**
 * cm_free_preauth_req() - free preauth request related memory
 * @preauth_req: preauth request
 *
 * Return: void
 */
void cm_free_preauth_req(struct wlan_preauth_req *preauth_req);

/**
 * cm_handle_preauth_rsp() - Process vdev preauth rsp and send to CM
 * @msg: scheduler message
 *
 * Process preauth rsp and send it to CM SM.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_handle_preauth_rsp(struct scheduler_msg *msg);

/**
 * cm_reassoc_timer_callback() - reassoc timer callback, gets called at time out
 * @context: context
 *
 * Timer callback for the timer that is started between the preauth completion
 * and reassoc request. In this interval, it is expected that the
 * pre-auth response and RIC IEs are passed up to the WPA supplicant and
 * received back the necessary FTIEs required to be sent in the reassoc request
 *
 * Return: None
 */
void cm_reassoc_timer_callback(void *context);
#else
static inline QDF_STATUS wlan_cm_host_roam_start(struct scheduler_msg *msg)
{
	if (msg && msg->bodyptr)
		qdf_mem_free(msg->bodyptr);

	return QDF_STATUS_SUCCESS;
}
#endif

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * wlan_cm_fw_roam_abort_req() - roam abort request handling
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_fw_roam_abort_req(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id);

/**
 * wlan_cm_get_roam_band_value  - Get roam band value from RSO config
 * @psoc: psoc pointer
 * @vdev: Pointer to vdev
 *
 * Return: Roam Band
 */
uint32_t wlan_cm_get_roam_band_value(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_vdev *vdev);

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
 * @param: vendor configured roam trigger param
 *
 * Return: none
 */
void
wlan_cm_roam_set_vendor_btm_params(struct wlan_objmgr_psoc *psoc,
				   struct wlan_cm_roam_vendor_btm_params
								*param);
/**
 * wlan_cm_roam_get_vendor_btm_params() - API to get vendor btm param
 * @psoc: PSOC pointer
 * @param: vendor configured roam trigger param
 *
 * Return: none
 */
void wlan_cm_roam_get_vendor_btm_params(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_cm_roam_vendor_btm_params *param);

/**
 * wlan_cm_roam_get_score_delta_params() - API to get roam score delta param
 * @psoc: PSOC pointer
 * @params: roam trigger param
 *
 * Return: none
 */
void
wlan_cm_roam_get_score_delta_params(struct wlan_objmgr_psoc *psoc,
				    struct wlan_roam_triggers *params);

/**
 * wlan_cm_roam_get_min_rssi_params() - API to get roam trigger min rssi param
 * @psoc: PSOC pointer
 * @params: roam trigger param
 *
 * Return: none
 */
void
wlan_cm_roam_get_min_rssi_params(struct wlan_objmgr_psoc *psoc,
				 struct wlan_roam_triggers *params);

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
 * wlan_cm_set_roam_band_bitmask() - Set roam band bitmask for vdev
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @roam_band_bitmask: bitmask of roam band for which roam scan needs to be
 * enabled in fw
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_set_roam_band_bitmask(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id,
					 uint32_t roam_band_bitmask);

/**
 * wlan_cm_set_roam_band_update() - send rso update on set band
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_set_roam_band_update(struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id);

/**
 * wlan_cm_get_roam_scan_scheme_bitmap() - Get roam scan scheme bitmap value
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 *
 * Return: Roam scan scheme bitmap value
 */
uint32_t wlan_cm_get_roam_scan_scheme_bitmap(struct wlan_objmgr_psoc *psoc,
					     uint8_t vdev_id);

/**
 * wlan_cm_update_roam_states() - Set roam states for the vdev
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 * @value: Value to update
 * @states: type of value to update
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_update_roam_states(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			   uint32_t value, enum roam_fail_params states);

/**
 * wlan_cm_get_roam_states() - Get roam states value
 * @psoc: PSOC pointer
 * @vdev_id: VDEV id
 * @states: For which action get roam states
 *
 * Return: Roam fail reason value
 */
uint32_t
wlan_cm_get_roam_states(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			enum roam_fail_params states);

void wlan_cm_set_psk_pmk(struct wlan_objmgr_pdev *pdev,
			 uint8_t vdev_id, uint8_t *psk_pmk,
			 uint8_t pmk_len);

void wlan_cm_get_psk_pmk(struct wlan_objmgr_pdev *pdev,
			 uint8_t vdev_id, uint8_t *psk_pmk,
			 uint8_t *pmk_len);

/**
 * cm_akm_roam_allowed() - check if  roam allowed for some akm type
 * roaming using single pmk
 * with same pmk or not
 * @psoc: psoc
 * @vdev: vdev pointer
 *
 * Return: QDF_STATUS: QDF_STATUS_SUCCESS is allowed
 */
QDF_STATUS
cm_akm_roam_allowed(struct wlan_objmgr_psoc *psoc,
		    struct wlan_objmgr_vdev *vdev);

/**
 * cm_invalid_roam_reason_handler() - Handler for invalid roam reason
 * @vdev_id: vdev id
 * @notif: roam notification of type enum cm_roam_notif
 * @reason: Notif param value from the roam event that carries trigger reason
 *
 * Return: QDF_STATUS
 */
void cm_invalid_roam_reason_handler(uint32_t vdev_id, enum cm_roam_notif notif,
				    uint32_t reason);

/**
 * cm_handle_roam_reason_ho_failed() - Handler for roam due to ho failure
 * @vdev_id: vdev id
 * @bssid: carries the BSSID mac address
 * @hw_mode_trans_ind: hw_mode transition indication
 *
 * Return: None
 */
void
cm_handle_roam_reason_ho_failed(uint8_t vdev_id, struct qdf_mac_addr bssid,
				struct cm_hw_mode_trans_ind *hw_mode_trans_ind);

/**
 * cm_handle_scan_ch_list_data() - Roam scan ch evt wrapper for wma
 * @data: roam scan channel event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_handle_scan_ch_list_data(struct cm_roam_scan_ch_resp *data);

/**
 * wlan_cm_free_roam_synch_frame_ind() - Free the bcn_probe_rsp, reassoc_req,
 * reassoc_rsp received as part of the ROAM_SYNC_FRAME event
 *
 * @vdev - vdev obj mgr ptr
 *
 * This API is used to free the buffer allocated during the ROAM_SYNC_FRAME
 * event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_cm_free_roam_synch_frame_ind(struct rso_config *rso_cfg);

/**
 * cm_roam_sync_event_handler() - CM handler for roam sync event
 *
 * @psoc - psoc objmgr ptr
 * @event - event ptr
 * @len - event buff length
 * @vdev_id - vdev id
 *
 * This API is used to handle the buffer allocated during the ROAM_SYNC_EVENT
 * event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_roam_sync_event_handler(struct wlan_objmgr_psoc *psoc,
				      uint8_t *event,
				      uint32_t len,
				      struct roam_offload_synch_ind *sync_ind);

/**
 * cm_roam_sync_frame_event_handler() - CM handler for roam sync frame event
 *
 * @psoc - psoc objmgr ptr
 * @frame_ind - ptr to roam sync frame struct
 *
 * This API is used to handle the buffer allocated during the ROAM_SYNC_FRAME
 * event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_sync_frame_event_handler(struct wlan_objmgr_psoc *psoc,
				 struct roam_synch_frame_ind *frame_ind);

/**
 * cm_roam_sync_event_handler_cb() - CM callback handler for roam
 * sync event
 *
 * @vdev - vdev objmgr ptr
 * @event - event ptr
 * @len - event data len
 *
 * This API is used to handle the buffer allocated during the ROAM_SYNC
 * event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS cm_roam_sync_event_handler_cb(struct wlan_objmgr_vdev *vdev,
					 uint8_t *event,
					 uint32_t len);

/**
 * wlan_cm_update_roam_rt_stats() - Store roam event stats command params
 * @psoc: PSOC pointer
 * @value: Value to update
 * @stats: type of value to update
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
wlan_cm_update_roam_rt_stats(struct wlan_objmgr_psoc *psoc,
			     uint8_t value, enum roam_rt_stats_params stats);

/**
 * wlan_cm_get_roam_rt_stats() - Get roam event stats value
 * @psoc: PSOC pointer
 * @stats: Get roam event command param for specific attribute
 *
 * Return: Roam events stats param value
 */
uint8_t
wlan_cm_get_roam_rt_stats(struct wlan_objmgr_psoc *psoc,
			  enum roam_rt_stats_params stats);

/**
 * cm_report_roam_rt_stats - Gathers/Sends the roam events stats
 * @psoc:      Pointer to psoc structure
 * @vdev_id:   Vdev ID
 * @events:    Event/Notif type from roam event/roam stats event
 * @roam_info: Roam stats from the roam stats event
 * @value:     Notif param value from the roam event
 * @idx:       TLV index in roam stats event
 * @reason:    Notif param value from the roam event that carries trigger reason
 *
 * Gathers the roam stats from the roam event and the roam stats event and
 * sends them to hdd for filling the vendor attributes.
 *
 * Return: none
 */
void cm_report_roam_rt_stats(struct wlan_objmgr_psoc *psoc,
			     uint8_t vdev_id,
			     enum roam_rt_stats_type events,
			     struct roam_stats_event *roam_info,
			     uint32_t value, uint8_t idx, uint32_t reason);
/**
 * cm_roam_candidate_event_handler() - CM callback to save roam
 * candidate entry in scan db
 *
 * @psoc - psoc objmgr ptr
 * @frame - roam scan candidate info
 */
QDF_STATUS
cm_roam_candidate_event_handler(struct wlan_objmgr_psoc *psoc,
				struct roam_scan_candidate_frame *candidate);

/** wlan_cm_is_roam_sync_in_progress() - Check if the vdev is in roam sync
 * substate
 *
 * @psoc: psoc pointer
 * @vdev_id: vdev_id
 *
 * Return: bool
 */
bool wlan_cm_is_roam_sync_in_progress(struct wlan_objmgr_psoc *psoc,
				      uint8_t vdev_id);

/**
 * wlan_cm_roam_set_ho_delay_config() - Set roam hand-off delay
 * @psoc: PSOC pointer
 * @roam_ho_delay: vendor configured roam HO delay value
 *
 * Return: none
 */
void
wlan_cm_roam_set_ho_delay_config(struct wlan_objmgr_psoc *psoc,
				 uint16_t roam_ho_delay);

/**
 * wlan_cm_roam_get_ho_delay_config() - Get roam hand-off delay
 * @psoc: PSOC pointer
 *
 * Return: Roam HO delay value
 */
uint16_t
wlan_cm_roam_get_ho_delay_config(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cm_set_exclude_rm_partial_scan_freq() - set value to include/exclude
 * the partial scan channels in roam full scan.
 * @psoc: PSOC pointer
 * @exclude_rm_partial_scan_freq: Include/exclude the channels in roam full scan
 * that are already scanned as part of partial scan.
 *
 * Return: none
 */
void
wlan_cm_set_exclude_rm_partial_scan_freq(struct wlan_objmgr_psoc *psoc,
					 uint8_t exclude_rm_partial_scan_freq);

/**
 * wlan_cm_get_exclude_rm_partial_scan_freq() - Get value to include/exclude
 * the partial scan channels in roam full scan.
 * @psoc: PSOC pointer
 *
 * Return: value to include/exclude the partial scan channels in roam full scan
 */
uint8_t
wlan_cm_get_exclude_rm_partial_scan_freq(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cm_roam_set_full_scan_6ghz_on_disc() - set value to include the 6 GHz
 * channels in roam full scan only on prior discovery of any 6 GHz support in
 * the environment.
 * @psoc: PSOC pointer
 * @roam_full_scan_6ghz_on_disc: Include the 6 GHz channels in roam full scan:
 * 1 - Include only on prior discovery of any 6 GHz support in the environment
 * 0 - Include all the supported 6 GHz channels by default
 *
 * Return: none
 */
void
wlan_cm_roam_set_full_scan_6ghz_on_disc(struct wlan_objmgr_psoc *psoc,
					uint8_t roam_full_scan_6ghz_on_disc);

/**
 * wlan_cm_roam_get_full_scan_6ghz_on_disc() - Get value to include the 6 GHz
 * channels in roam full scan only on prior discovery of any 6 GHz support in
 * the environment.
 * @psoc: PSOC pointer
 *
 * Return:
 * 1 - Include only on prior discovery of any 6 GHz support in the environment
 * 0 - Include all the supported 6 GHz channels by default
 */
uint8_t wlan_cm_roam_get_full_scan_6ghz_on_disc(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cm_set_roam_scan_high_rssi_offset() - Set the delta change in high RSSI
 * at which roam scan is triggered in 2.4/5 GHz.
 * @psoc: PSOC pointer
 * @roam_high_rssi_delta: Set the High RSSI delta for roam scan trigger
 * * 1-16 - Set an offset value in this range
 * * 0    - Disable
 *
 * Return: none
 */
void
wlan_cm_set_roam_scan_high_rssi_offset(struct wlan_objmgr_psoc *psoc,
				       uint8_t roam_high_rssi_delta);

/**
 * wlan_cm_get_roam_scan_high_rssi_offset() - Get the delta change in high RSSI
 * at which roam scan is triggered in 2.4/5 GHz.
 * @psoc: PSOC pointer
 *
 * Return: High RSSI delta for roam scan trigger
 */
uint8_t wlan_cm_get_roam_scan_high_rssi_offset(struct wlan_objmgr_psoc *psoc);
#else
static inline
void wlan_cm_roam_activate_pcl_per_vdev(struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id,
					bool pcl_per_vdev)
{}

static inline
uint32_t wlan_cm_get_roam_band_value(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_vdev *vdev)
{
	uint32_t current_band;

	ucfg_reg_get_band(wlan_vdev_get_pdev(vdev), &current_band);

	return current_band;
}

static inline
bool wlan_cm_roam_is_pcl_per_vdev_active(struct wlan_objmgr_psoc *psoc,
					 uint8_t vdev_id)
{
	return false;
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
wlan_cm_roam_extract_frame_info(wmi_unified_t wmi, void *evt_buf,
				struct roam_frame_info *dst, uint8_t idx)
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
wlan_cm_roam_disable_vendor_btm(struct wlan_objmgr_psoc *psoc)
{}

static inline void
wlan_cm_roam_set_vendor_btm_params(struct wlan_objmgr_psoc *psoc,
				   struct wlan_cm_roam_vendor_btm_params *param)
{}

static inline void
wlan_cm_roam_get_score_delta_params(struct wlan_objmgr_psoc *psoc,
				    uint8_t vdev_id,
				    struct roam_trigger_score_delta *param)
{}

static inline void
wlan_cm_roam_get_min_rssi_params(struct wlan_objmgr_psoc *psoc,
				 struct wlan_roam_triggers *params)
{}

static inline QDF_STATUS
wlan_cm_update_roam_scan_scheme_bitmap(struct wlan_objmgr_psoc *psoc,
				       uint8_t vdev_id,
				       uint32_t roam_scan_scheme_bitmap)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
wlan_cm_set_roam_band_bitmask(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			      uint32_t roam_band_bitmask)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline
uint32_t wlan_cm_get_roam_scan_scheme_bitmap(struct wlan_objmgr_psoc *psoc,
					     uint8_t vdev_id)
{
	return 0;
}

static inline QDF_STATUS
wlan_cm_update_roam_states(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			   uint32_t value, enum roam_fail_params states)
{
	return QDF_STATUS_SUCCESS;
}

static inline uint32_t
wlan_cm_get_roam_states(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			enum roam_fail_params states)
{
	return 0;
}

static inline void wlan_cm_set_psk_pmk(struct wlan_objmgr_pdev *pdev,
				       uint8_t vdev_id, uint8_t *psk_pmk,
				       uint8_t pmk_len)
{}

static inline QDF_STATUS
cm_akm_roam_allowed(struct wlan_objmgr_psoc *psoc,
		    struct wlan_objmgr_vdev *vdev)
{
	return false;
}

static inline void
cm_handle_roam_reason_ho_failed(uint8_t vdev_id, struct qdf_mac_addr bssid,
				struct cm_hw_mode_trans_ind *hw_mode_trans_ind)
{}

static inline QDF_STATUS
cm_handle_scan_ch_list_data(struct cm_roam_scan_ch_resp *data)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline QDF_STATUS
wlan_cm_update_roam_rt_stats(struct wlan_objmgr_psoc *psoc,
			     uint8_t value, enum roam_rt_stats_params stats)
{
	return QDF_STATUS_SUCCESS;
}

static inline uint8_t
wlan_cm_get_roam_rt_stats(struct wlan_objmgr_psoc *psoc,
			  enum roam_rt_stats_params stats)
{
	return 0;
}

static inline void
cm_report_roam_rt_stats(struct wlan_objmgr_psoc *psoc,
			uint8_t vdev_id, enum roam_rt_stats_type events,
			struct roam_stats_event *roam_info,
			uint32_t value, uint8_t idx, uint32_t reason)
{}

static inline QDF_STATUS
cm_roam_candidate_event_handler(struct wlan_objmgr_psoc *psoc,
				struct roam_scan_candidate_frame *candidate)
{
	return QDF_STATUS_SUCCESS;
}

static inline bool
wlan_cm_is_roam_sync_in_progress(struct wlan_objmgr_psoc *psoc,
				 uint8_t vdev_id)
{
	return false;
}

static inline uint16_t
wlan_cm_roam_get_ho_delay_config(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

static inline uint8_t
wlan_cm_get_exclude_rm_partial_scan_freq(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

static inline uint8_t
wlan_cm_roam_get_full_scan_6ghz_on_disc(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

static inline uint8_t
wlan_cm_get_roam_scan_high_rssi_offset(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}
#endif /* WLAN_FEATURE_ROAM_OFFLOAD */

#ifdef WLAN_FEATURE_FIPS
/**
 * cm_roam_pmkid_req_ind() - Function to handle
 * roam event from firmware for pmkid generation.
 * @psoc: psoc pointer
 * @vdev_id: Vdev id
 * @bss_list: candidate AP bssid list
 */
QDF_STATUS
cm_roam_pmkid_req_ind(struct wlan_objmgr_psoc *psoc,
		      uint8_t vdev_id, struct roam_pmkid_req_event *bss_list);
#else /* WLAN_FEATURE_FIPS */
static inline QDF_STATUS
cm_roam_pmkid_req_ind(struct wlan_objmgr_psoc *psoc,
		      uint8_t vdev_id, struct roam_pmkid_req_event *bss_list)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_FEATURE_FIPS */

/**
 * wlan_get_chan_by_bssid_from_rnr: get chan from rnr through bssid
 * @vdev: vdev
 * @cm_id: connect manager id
 * @link_addr: bssid of given link
 * @chan: chan to get
 * @op_class: operation class
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_get_chan_by_bssid_from_rnr(struct wlan_objmgr_vdev *vdev,
					   wlan_cm_id cm_id,
					   struct qdf_mac_addr *link_addr,
					   uint8_t *chan, uint8_t *op_class);

/**
 * wlan_get_chan_by_link_id_from_rnr: get chan from rnr through link id
 * @vdev: vdev
 * @cm_id: connect manager id
 * @link_id: link id of given link
 * @chan: chan to get
 * @op_class: operation class
 *
 * Return: QDF_STATUS
 */
#ifdef WLAN_FEATURE_11BE_MLO
QDF_STATUS wlan_get_chan_by_link_id_from_rnr(struct wlan_objmgr_vdev *vdev,
					     wlan_cm_id cm_id,
					     uint8_t link_id,
					     uint8_t *chan, uint8_t *op_class);
#endif

/**
 * cm_rso_cmd_status_event_handler() - Handler for rso cmd status
 * @vdev_id: vdev id
 * @notif: roam notification of type enum cm_roam_notif
 *
 * Return: QDF_STATUS
 */
int cm_rso_cmd_status_event_handler(uint8_t vdev_id, enum cm_roam_notif notif);

/**
 * cm_handle_roam_reason_invoke_roam_fail() - Handler for roam invoke fail event
 * @vdev_id: vdev id
 * @notif_params: contains roam invoke fail reason from wmi_roam_invoke_error_t
 * @trans_ind: hw_mode transition indication
 *
 * Return: None
 */
void
cm_handle_roam_reason_invoke_roam_fail(uint8_t vdev_id,	uint32_t notif_params,
				       struct cm_hw_mode_trans_ind *trans_ind);

/**
 * cm_handle_roam_sync_update_hw_mode() - Handler for roam sync hw mode update
 * @trans_ind: hw_mode transition indication
 *
 * Return: None
 */
void
cm_handle_roam_sync_update_hw_mode(struct cm_hw_mode_trans_ind *trans_ind);

/**
 * cm_handle_roam_reason_deauth() - Handler for roam due to deauth from AP
 * @vdev_id: vdev id
 * @notif_params: contains roam invoke fail reason from wmi_roam_invoke_error_t
 * @deauth_disassoc_frame: Disassoc or deauth frame
 * @frame_len: Contains the length of @deauth_disassoc_frame
 *
 * Return: None
 */
void cm_handle_roam_reason_deauth(uint8_t vdev_id, uint32_t notif_params,
				  uint8_t *deauth_disassoc_frame,
				  uint32_t frame_len);

/**
 * cm_handle_roam_reason_btm() - Handler for roam due to btm from AP
 * @vdev_id: vdev id
 *
 * Return: None
 */
void cm_handle_roam_reason_btm(uint8_t vdev_id);

/**
 * cm_handle_roam_reason_bmiss() - Handler for roam due to bmiss
 * @vdev_id: vdev id
 * @rssi: RSSI value
 *
 * Return: None
 */
void cm_handle_roam_reason_bmiss(uint8_t vdev_id, uint32_t rssi);

/**
 * cm_handle_roam_reason_better_ap() - Handler for roam due to better AP
 * @vdev_id: vdev id
 * @rssi: RSSI value
 *
 * Return: None
 */
void cm_handle_roam_reason_better_ap(uint8_t vdev_id, uint32_t rssi);

/**
 * cm_handle_roam_reason_suitable_ap() - Handler for roam due to suitable AP
 * @vdev_id: vdev id
 * @rssi: RSSI value
 *
 * Return: None
 */
void cm_handle_roam_reason_suitable_ap(uint8_t vdev_id, uint32_t rssi);

/**
 * cm_roam_event_handler() - Carries extracted roam info
 * @roam_event: data carried by roam event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_event_handler(struct roam_offload_roam_event *roam_event);

/**
 * cm_btm_blacklist_event_handler() - Black list the given BSSID due to btm
 * @psoc: PSOC pointer
 * @list: Roam blacklist info
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_btm_blacklist_event_handler(struct wlan_objmgr_psoc *psoc,
			       struct roam_blacklist_event *list);

/**
 * cm_vdev_disconnect_event_handler() - disconnect evt handler for target_if
 * @data: disconnect event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_vdev_disconnect_event_handler(struct vdev_disconnect_event_data *data);

/**
 * cm_handle_disconnect_reason() - disconnect reason evt wrapper for wma
 * @data: disconnect event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_handle_disconnect_reason(struct vdev_disconnect_event_data *data);

/**
 * cm_roam_scan_ch_list_event_handler() - Roam scan ch evt handler for target_if
 * @data: roam scan channel event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_scan_ch_list_event_handler(struct cm_roam_scan_ch_resp *data);

/**
 * cm_roam_stats_event_handler() - Carries extracted roam stats info
 * @psoc: PSOC pointer
 * @stats_info: stats data carried by roam_stats_event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_stats_event_handler(struct wlan_objmgr_psoc *psoc,
			    struct roam_stats_event *stats_info);

/**
 * cm_handle_auth_offload() - auth offload evt wrapper for wma
 * @auth_event: auth offload event data
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_handle_auth_offload(struct auth_offload_event *auth_event);

/**
 * cm_roam_auth_offload_event_handler() - Handler for auth offload event
 * @auth_event: Authentication event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_auth_offload_event_handler(struct auth_offload_event *auth_event);

/*
 * cm_roam_pmkid_request_handler() - Carries extracted pmkid list info
 * @data: Pmkid event with entries
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_pmkid_request_handler(struct roam_pmkid_req_event *data);

/**
 * cm_roam_update_vdev() - Update the STA and BSS
 * @sync_ind: Information needed for roam sync propagation
 * @vdev_id: vdev id
 *
 * This function will perform all the vdev related operations with
 * respect to the self sta and the peer after roaming and completes
 * the roam synch propagation with respect to WMA layer.
 *
 * Return: None
 */
void cm_roam_update_vdev(struct roam_offload_synch_ind *sync_ind,
			 uint8_t vdev_id);

/**
 * cm_roam_pe_sync_callback() - Callback registered at pe, gets invoked when
 * ROAM SYNCH event is received from firmware
 * @sync_ind: Structure with roam synch parameters
 * @vdev_id: vdev id
 * @len: length for bss_description
 *
 * This is a PE level callback called from CM to complete the roam synch
 * propagation at PE level and also fill the BSS descriptor which will be
 * helpful further to complete the roam synch propagation.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
cm_roam_pe_sync_callback(struct roam_offload_synch_ind *sync_ind,
			 uint8_t vdev_id, uint16_t len);

/**
 * cm_update_phymode_on_roam() - Update new phymode after
 * ROAM SYNCH event is received from firmware
 * @vdev_id: roamed vdev id
 * @bssid: bssid
 * @chan: wmi channel
 *
 * This api will update the phy mode after roam sync is received.
 *
 * Return: none
 */
void cm_update_phymode_on_roam(uint8_t vdev_id, uint8_t *bssid,
			       wmi_channel *chan);

/**
 * wlan_cm_fw_to_host_phymode() - Convert fw phymode to host
 * @phymode: wmi phymode
 *
 * This api will convert the phy mode from fw to host type.
 *
 * Return: wlan phymode
 */
enum wlan_phymode
wlan_cm_fw_to_host_phymode(WMI_HOST_WLAN_PHY_MODE phymode);

/**
 * wlan_cm_sta_mlme_vdev_roam_notify() - STA mlme vdev roam nottify
 *
 * @sync_ind: Information needed for roam sync propagation
 *
 * This function will invokes CM roam callback api to continue
 * the roam synch propagation.
 *
 * Return: None
 */
QDF_STATUS
wlan_cm_sta_mlme_vdev_roam_notify(struct vdev_mlme_obj *vdev_mlme,
				  uint16_t data_len, void *data);

/**
 * wlan_cm_same_band_sta_allowed() - check if same band STA +STA is allowed
 *
 * @psoc: psoc ptr
 *
 * Return: true if same band STA+STA is allowed
 */
bool wlan_cm_same_band_sta_allowed(struct wlan_objmgr_psoc *psoc);

/**
 * cm_cleanup_mlo_link() - Cleanup the MLO link
 *
 * @vdev: MLO link vdev
 *
 * This posts the event WLAN_CM_SM_EV_ROAM_LINK_DOWN to CM to cleanup the
 * resources allocated for MLO link e.g. vdev, pe_session, etc..
 * This gets called when MLO to non-MLO roaming happens
 *
 * Return: qdf_status
 */
QDF_STATUS cm_cleanup_mlo_link(struct wlan_objmgr_vdev *vdev);
#endif  /* WLAN_CM_ROAM_API_H__ */
