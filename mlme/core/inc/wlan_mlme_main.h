/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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

#define mlme_fatal(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_MLME, params)
#define mlme_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_MLME, params)
#define mlme_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_MLME, params)
#define mlme_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_MLME, params)
#define mlme_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_MLME, params)

/**
 * struct wlan_mlme_psoc_obj -MLME psoc priv object
 * @cfg:     cfg items
 */
struct wlan_mlme_psoc_obj {
	struct wlan_mlme_cfg cfg;
};

/**
 * struct peer_mlme_priv_obj - peer MLME component object
 * @ucast_key_cipher: unicast crypto type.
 */
struct peer_mlme_priv_obj {
	uint32_t ucast_key_cipher;
};

#ifdef CONFIG_VDEV_SM

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
 * struct mlme_legacy_priv - VDEV MLME legacy priv object
 * @chan_switch_in_progress: flag to indicate that channel switch is in progress
 * @hidden_ssid_restart_in_progress: flag to indicate hidden ssid restart is
 *                                   in progress
 * @vdev_start_failed: flag to indicate that vdev start failed.
 * @connection_fail: flag to indicate connection failed
 * @assoc_type: vdev associate/reassociate type
 * @dynamic_cfg: current configuration of nss, chains for vdev.
 * @ini_cfg: Max configuration of nss, chains supported for vdev.
 */
struct mlme_legacy_priv {
	bool chan_switch_in_progress;
	bool hidden_ssid_restart_in_progress;
	bool vdev_start_failed;
	bool connection_fail;
	enum vdev_assoc_type assoc_type;
	struct wlan_mlme_nss_chains dynamic_cfg;
	struct wlan_mlme_nss_chains ini_cfg;
};

#else

/**
 * struct vdev_mlme_obj - VDEV MLME component object
 * @dynamic_cfg: current configuration of nss, chains for vdev.
 * @ini_cfg: Max configuration of nss, chains supported for vdev.
 */
struct vdev_mlme_priv_obj {
	struct wlan_mlme_nss_chains dynamic_cfg;
	struct wlan_mlme_nss_chains ini_cfg;
};

/**
 * mlme_vdev_object_created_notification(): mlme vdev create handler
 * @vdev: vdev which is going to created by objmgr
 * @arg: argument for vdev create handler
 *
 * Register this api with objmgr to detect vdev is created
 *
 * Return: QDF_STATUS status in case of success else return error
 */

QDF_STATUS
mlme_vdev_object_created_notification(struct wlan_objmgr_vdev *vdev,
				      void *arg);

/**
 * mlme_vdev_object_destroyed_notification(): mlme vdev delete handler
 * @psoc: vdev which is going to delete by objmgr
 * @arg: argument for vdev delete handler
 *
 * Register this api with objmgr to detect vdev is deleted
 *
 * Return: QDF_STATUS status in case of success else return error
 */
QDF_STATUS
mlme_vdev_object_destroyed_notification(struct wlan_objmgr_vdev *vdev,
					void *arg);

#endif

/**
 * wlan_peer_set_unicast_cipher() - set unicast cipher
 * @peer: PEER object
 * @value: value to be set
 *
 * Return: void
 */
static inline
void wlan_peer_set_unicast_cipher(struct wlan_objmgr_peer *peer, uint32_t value)
{
	struct peer_mlme_priv_obj *peer_priv;

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_MLME);
	if (!peer_priv) {
		mlme_err(" peer mlme component object is NULL");
		return;
	}
	peer_priv->ucast_key_cipher  = value;
}

/**
 * wlan_peer_get_unicast_cipher() - get unicast cipher
 * @peer: PEER object
 *
 * Return: ucast_key_cipher value
 */
static inline
uint32_t wlan_peer_get_unicast_cipher(struct wlan_objmgr_peer *peer)
{
	struct peer_mlme_priv_obj *peer_priv;

	peer_priv = wlan_objmgr_peer_get_comp_private_obj(peer,
							  WLAN_UMAC_COMP_MLME);
	if (!peer_priv) {
		mlme_err("peer mlme component object is NULL");
		return 0;
	}

	return peer_priv->ucast_key_cipher;
}

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
 * mlme_psoc_object_created_notification(): mlme psoc create handler
 * @psoc: psoc which is going to created by objmgr
 * @arg: argument for vdev create handler
 *
 * Register this api with objmgr to detect psoc is created
 *
 * Return: QDF_STATUS status in case of success else return error
 */
QDF_STATUS
mlme_psoc_object_created_notification(struct wlan_objmgr_psoc *psoc,
				      void *arg);

/**
 * mlme_psoc_object_destroyed_notification(): mlme psoc delete handler
 * @psoc: psoc which is going to delete by objmgr
 * @arg: argument for vdev delete handler
 *
 * Register this api with objmgr to detect psoc is deleted
 *
 * Return: QDF_STATUS status in case of success else return error
 */
QDF_STATUS
mlme_psoc_object_destroyed_notification(struct wlan_objmgr_psoc *psoc,
					void *arg);

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
 * mlme_get_psoc_obj() - Get MLME object from psoc
 * @psoc: pointer to the psoc object
 *
 * Get the MLME object pointer from the psoc
 *
 * Return: pointer to MLME object
 */
#define mlme_get_psoc_obj(psoc) mlme_get_psoc_obj_fl(psoc, __func__, __LINE__)
struct wlan_mlme_psoc_obj *mlme_get_psoc_obj_fl(struct wlan_objmgr_psoc *psoc,
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

#endif
