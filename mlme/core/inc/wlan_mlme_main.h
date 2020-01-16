/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
/**
 * DOC: declare internal API related to the mlme component
 */

#ifndef _WLAN_MLME_MAIN_H_
#define _WLAN_MLME_MAIN_H_

#include <wlan_mlme_public_struct.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_cmn.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>

#define mlme_legacy_fatal(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_MLME, params)
#define mlme_legacy_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_MLME, params)
#define mlme_legacy_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_MLME, params)
#define mlme_legacy_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_MLME, params)
#define mlme_legacy_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_MLME, params)

/**
 * struct wlan_mlme_psoc_ext_obj -MLME ext psoc priv object
 * @cfg:     cfg items
 */
struct wlan_mlme_psoc_ext_obj {
	struct wlan_mlme_cfg cfg;
};

/**
 * struct wlan_ies - Generic WLAN Information Element(s) format
 * @len: Total length of the IEs
 * @data: IE data
 */
struct wlan_ies {
	uint16_t len;
	uint8_t *data;
};

/**
 * struct peer_mlme_priv_obj - peer MLME component object
 * @is_pmf_enabled: True if PMF is enabled
 * @last_assoc_received_time: last assoc received time
 * @last_disassoc_deauth_received_time: last disassoc/deauth received time
 */
struct peer_mlme_priv_obj {
	bool is_pmf_enabled;
	qdf_time_t last_assoc_received_time;
	qdf_time_t last_disassoc_deauth_received_time;
};

/**
 * enum vdev_assoc_type - VDEV associate/reassociate type
 * @VDEV_ASSOC: associate
 * @VDEV_REASSOC: reassociate
 * @VDEV_FT_REASSOC: fast reassociate
 */
enum vdev_assoc_type {
	VDEV_ASSOC,
	VDEV_REASSOC,
	VDEV_FT_REASSOC
};

/**
 * wlan_mlme_roam_state_info - Structure containing roaming
 * state related details
 * @state: Roaming module state.
 * @mlme_operations_bitmap: Bitmap containing what mlme operations are in
 *  progress where roaming should not be allowed.
 */
struct wlan_mlme_roam_state_info {
	enum roam_offload_state state;
	uint8_t mlme_operations_bitmap;
};

/**
 * struct wlan_mlme_roaming_config - Roaming configurations structure
 * @roam_trigger_bitmap: Master bitmap of roaming triggers. If the bitmap is
 *  zero, roaming module will be deinitialized at firmware for this vdev.
 * @supplicant_disabled_roaming: Enable/disable roam scan in firmware; will be
 *  used by supplicant to do roam invoke after disabling roam scan in firmware
 */
struct wlan_mlme_roaming_config {
	uint32_t roam_trigger_bitmap;
	bool supplicant_disabled_roaming;
};

/**
 * struct wlan_mlme_roam - Roam structure containing roam state and
 *  roam config info
 * @roam_sm: Structure containing roaming state related details
 * @roam_config: Roaming configurations structure
 */
struct wlan_mlme_roam {
	struct wlan_mlme_roam_state_info roam_sm;
	struct wlan_mlme_roaming_config roam_cfg;
};

/**
 * struct mlme_legacy_priv - VDEV MLME legacy priv object
 * @chan_switch_in_progress: flag to indicate that channel switch is in progress
 * @hidden_ssid_restart_in_progress: flag to indicate hidden ssid restart is
 *                                   in progress
 * @vdev_start_failed: flag to indicate that vdev start failed.
 * @connection_fail: flag to indicate connection failed
 * @cac_required_for_new_channel: if CAC is required for new channel
 * @follow_ap_edca: if true, it is forced to follow the AP's edca.
 * @assoc_type: vdev associate/reassociate type
 * @dynamic_cfg: current configuration of nss, chains for vdev.
 * @ini_cfg: Max configuration of nss, chains supported for vdev.
 * @sta_dynamic_oce_value: Dyanmic oce flags value for sta
 * @roam_invoke_params: Roam invoke params
 * @self_disconnect_ies: Disconnect IEs to be sent in deauth/disassoc frames
 *			 originated from driver
 * @peer_disconnect_ies: Disconnect IEs received in deauth/disassoc frames
 *			 from peer
 * @vdev_stop_type: vdev stop type request
 * @roam_off_state: Roam offload state
 */
struct mlme_legacy_priv {
	bool chan_switch_in_progress;
	bool hidden_ssid_restart_in_progress;
	bool vdev_start_failed;
	bool connection_fail;
	bool cac_required_for_new_channel;
	bool follow_ap_edca;
	enum vdev_assoc_type assoc_type;
	struct wlan_mlme_nss_chains dynamic_cfg;
	struct wlan_mlme_nss_chains ini_cfg;
	uint8_t sta_dynamic_oce_value;
	struct mlme_roam_after_data_stall roam_invoke_params;
	struct wlan_ies self_disconnect_ies;
	struct wlan_ies peer_disconnect_ies;
	uint32_t vdev_stop_type;
	struct wlan_mlme_roam mlme_roam;
};

/**
 * wma_get_peer_mic_len() - get mic hdr len and mic length for peer
 * @psoc: psoc
 * @pdev_id: pdev id for the peer
 * @peer_mac: peer mac
 * @mic_len: mic length for peer
 * @mic_hdr_len: mic header length for peer
 *
 * Return: Success or Failure status
 */
QDF_STATUS mlme_get_peer_mic_len(struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
				 uint8_t *peer_mac, uint8_t *mic_len,
				 uint8_t *mic_hdr_len);

/**
 * mlme_peer_object_created_notification(): mlme peer create handler
 * @peer: peer which is going to created by objmgr
 * @arg: argument for vdev create handler
 *
 * Register this api with objmgr to detect peer is created
 *
 * Return: QDF_STATUS status in case of success else return error
 */

QDF_STATUS
mlme_peer_object_created_notification(struct wlan_objmgr_peer *peer,
				      void *arg);

/**
 * mlme_peer_object_destroyed_notification(): mlme peer delete handler
 * @peer: peer which is going to delete by objmgr
 * @arg: argument for vdev delete handler
 *
 * Register this api with objmgr to detect peer is deleted
 *
 * Return: QDF_STATUS status in case of success else return error
 */
QDF_STATUS
mlme_peer_object_destroyed_notification(struct wlan_objmgr_peer *peer,
					void *arg);

/**
 * mlme_get_dynamic_oce_flags(): mlme get dynamic oce flags
 * @vdev: pointer to vdev object
 *
 * This api is used to get the dynamic oce flags pointer
 *
 * Return: QDF_STATUS status in case of success else return error
 */
uint8_t *mlme_get_dynamic_oce_flags(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_get_dynamic_vdev_config() - get the vdev dynamic config params
 * @vdev: vdev pointer
 *
 * Return: pointer to the dynamic vdev config structure
 */
struct wlan_mlme_nss_chains *mlme_get_dynamic_vdev_config(
					struct wlan_objmgr_vdev *vdev);

/**
 * mlme_get_ini_vdev_config() - get the vdev ini config params
 * @vdev: vdev pointer
 *
 * Return: pointer to the ini vdev config structure
 */
struct wlan_mlme_nss_chains *mlme_get_ini_vdev_config(
					struct wlan_objmgr_vdev *vdev);

/**
 * mlme_get_roam_invoke_params() - get the roam invoke params
 * @vdev: vdev pointer
 *
 * Return: pointer to the vdev roam invoke config structure
 */
struct mlme_roam_after_data_stall *
mlme_get_roam_invoke_params(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_cfg_on_psoc_enable() - Populate MLME structure from CFG and INI
 * @psoc: pointer to the psoc object
 *
 * Populate the MLME CFG structure from CFG and INI values using CFG APIs
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cfg_on_psoc_enable(struct wlan_objmgr_psoc *psoc);

/**
 * mlme_get_psoc_ext_obj() - Get MLME object from psoc
 * @psoc: pointer to the psoc object
 *
 * Get the MLME object pointer from the psoc
 *
 * Return: pointer to MLME object
 */
#define mlme_get_psoc_ext_obj(psoc) \
			mlme_get_psoc_ext_obj_fl(psoc, __func__, __LINE__)
struct wlan_mlme_psoc_ext_obj *mlme_get_psoc_ext_obj_fl(struct wlan_objmgr_psoc
							*psoc,
							const char *func,
							uint32_t line);

/**
 * mlme_init_ibss_cfg() - Init IBSS config data structure with default CFG value
 * @psoc: pointer to the psoc object
 * @ibss_cfg: Pointer to IBSS cfg data structure to return values
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_init_ibss_cfg(struct wlan_objmgr_psoc *psoc,
			      struct wlan_mlme_ibss_cfg *ibss_cfg);

/**
 * mlme_set_self_disconnect_ies() - Set diconnect IEs configured from userspace
 * @vdev: vdev pointer
 * @ie: pointer for disconnect IEs
 *
 * Return: None
 */
void mlme_set_self_disconnect_ies(struct wlan_objmgr_vdev *vdev,
				  struct wlan_ies *ie);

/**
 * mlme_free_self_disconnect_ies() - Free the self diconnect IEs
 * @vdev: vdev pointer
 *
 * Return: None
 */
void mlme_free_self_disconnect_ies(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_get_self_disconnect_ies() - Get diconnect IEs from vdev object
 * @vdev: vdev pointer
 *
 * Return: Returns a pointer to the self disconnect IEs present in vdev object
 */
struct wlan_ies *mlme_get_self_disconnect_ies(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_set_peer_disconnect_ies() - Cache disconnect IEs received from peer
 * @vdev: vdev pointer
 * @ie: pointer for disconnect IEs
 *
 * Return: None
 */
void mlme_set_peer_disconnect_ies(struct wlan_objmgr_vdev *vdev,
				  struct wlan_ies *ie);

/**
 * mlme_free_peer_disconnect_ies() - Free the peer diconnect IEs
 * @vdev: vdev pointer
 *
 * Return: None
 */
void mlme_free_peer_disconnect_ies(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_set_follow_ap_edca_flag() - Set follow ap's edca flag
 * @vdev: vdev pointer
 * @flag: carries if following ap's edca is true or not.
 *
 * Return: None
 */
void mlme_set_follow_ap_edca_flag(struct wlan_objmgr_vdev *vdev, bool flag);

/**
 * mlme_get_follow_ap_edca_flag() - Get follow ap's edca flag
 * @vdev: vdev pointer
 *
 * Return: value of follow_ap_edca
 */
bool mlme_get_follow_ap_edca_flag(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_get_peer_disconnect_ies() - Get diconnect IEs from vdev object
 * @vdev: vdev pointer
 *
 * Return: Returns a pointer to the peer disconnect IEs present in vdev object
 */
struct wlan_ies *mlme_get_peer_disconnect_ies(struct wlan_objmgr_vdev *vdev);

/**
 * mlme_set_peer_pmf_status() - set pmf status of peer
 * @peer: PEER object
 * @is_pmf_enabled: Carries if PMF is enabled or not
 *
 * is_pmf_enabled will be set to true if PMF is enabled by peer
 *
 * Return: void
 */
void mlme_set_peer_pmf_status(struct wlan_objmgr_peer *peer,
			      bool is_pmf_enabled);
/**
 * mlme_get_peer_pmf_status() - get if peer is of pmf capable
 * @peer: PEER object
 *
 * Return: Value of is_pmf_enabled; True if PMF is enabled by peer
 */
bool mlme_get_peer_pmf_status(struct wlan_objmgr_peer *peer);

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * mlme_get_supplicant_disabled_roaming() - Get supplicant disabled roaming
 *  value for a given vdev.
 * @psoc: PSOC pointer
 * @vdev_id: Vdev for which the supplicant disabled roaming value is being
 *  requested
 *
 * Return: True if supplicant disabled roaming else false
 */
bool
mlme_get_supplicant_disabled_roaming(struct wlan_objmgr_psoc *psoc,
				     uint8_t vdev_id);

/**
 * mlme_set_supplicant_disabled_roaming - Set the supplicant disabled
 *  roaming flag.
 * @psoc: PSOC pointer
 * @vdev_id: Vdev for which the supplicant disabled roaming needs to
 *  be set
 * @val: value true is to disable RSO and false to enable RSO
 *
 * Return: None
 */
void mlme_set_supplicant_disabled_roaming(struct wlan_objmgr_psoc *psoc,
					  uint8_t vdev_id, bool val);

/**
 * mlme_get_roam_trigger_bitmap() - Get roaming trigger bitmap value for a given
 *  vdev.
 * @psoc: PSOC pointer
 * @vdev_id: Vdev for which the roam trigger bitmap is being requested
 *
 * Return: roaming trigger bitmap
 */
uint32_t
mlme_get_roam_trigger_bitmap(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id);

/**
 * mlme_set_roam_trigger_bitmap() - Set the roaming trigger bitmap value for
 *  the given vdev. If the bitmap is zero then roaming is completely disabled
 *  on the vdev which means roam structure in firmware is not allocated and no
 *  RSO start/stop commands can be sent
 * @psoc: PSOC pointer
 * @vdev_id: Vdev for which the roam trigger bitmap is to be set
 * @val: bitmap value to set
 *
 * Return: None
 */
void mlme_set_roam_trigger_bitmap(struct wlan_objmgr_psoc *psoc,
				  uint8_t vdev_id, uint32_t val);

/**
 * mlme_get_roam_state() - Get roam state from vdev object
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 *
 * Return: Returns roam offload state
 */
enum roam_offload_state
mlme_get_roam_state(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id);

/**
 * mlme_set_roam_state() - Set roam state in vdev object
 * @psoc: psoc pointer
 * @vdev_id: vdev id
 * @val: roam offload state
 *
 * Return: None
 */
void mlme_set_roam_state(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			 enum roam_offload_state val);

/**
 * mlme_get_operations_bitmap() - Get the mlme operations bitmap which
 *  contains the bitmap of mlme operations which have disabled roaming
 *  temporarily
 * @psoc: PSOC pointer
 * @vdev_id: vdev for which the mlme operation bitmap is requested
 *
 * Return: bitmap value
 */
uint8_t
mlme_get_operations_bitmap(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id);

/**
 * mlme_set_operations_bitmap() - Set the mlme operations bitmap which
 *  indicates what mlme operations are in progress
 * @psoc: PSOC pointer
 * @vdev_id: vdev for which the mlme operation bitmap is requested
 * @reqs: RSO stop requestor
 * @clear: clear bit if true else set bit
 *
 * Return: bitmap value
 */
void
mlme_set_operations_bitmap(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			   enum roam_control_requestor reqs, bool clear);

#define MLME_IS_ROAM_STATE_RSO_STARTED(psoc, vdev_id) \
	(mlme_get_roam_state(psoc, vdev_id) == ROAM_RSO_STARTED)

#define MLME_IS_ROAM_STATE_DEINIT(psoc, vdev_id) \
	(mlme_get_roam_state(psoc, vdev_id) == ROAM_DEINIT)

#define MLME_IS_ROAM_STATE_INIT(psoc, vdev_id) \
	(mlme_get_roam_state(psoc, vdev_id) == ROAM_INIT)

#define MLME_IS_ROAM_STATE_STOPPED(psoc, vdev_id) \
	(mlme_get_roam_state(psoc, vdev_id) == ROAM_RSO_STOPPED)

#define MLME_IS_ROAM_INITIALIZED(psoc, vdev_id) \
	(mlme_get_roam_state(psoc, vdev_id) >= ROAM_INIT)
#endif

/**
 * mlme_reinit_control_config_lfr_params() - Reinitialize roam control config
 * @psoc: PSOC pointer
 * @lfr: Pointer of an lfr_cfg buffer to fill.
 *
 * Reinitialize/restore the param related control roam config lfr params with
 * default values of corresponding ini params.
 *
 * Return: None
 */
void mlme_reinit_control_config_lfr_params(struct wlan_objmgr_psoc *psoc,
					   struct wlan_mlme_lfr_cfg *lfr);
#endif
