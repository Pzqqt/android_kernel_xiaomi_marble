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
 * DOC: define internal APIs related to the mlme component
 */
#include "wlan_mlme_main.h"
#include "wlan_mlme_vdev_mgr_interface.h"
#include "lim_utils.h"
#include "wma_api.h"
#include "lim_types.h"
#include <include/wlan_mlme_cmn.h>

static struct vdev_mlme_ops sta_mlme_ops;
static struct vdev_mlme_ops ap_mlme_ops;
static struct vdev_mlme_ops mon_mlme_ops;
static struct mlme_ext_ops ext_ops;

bool mlme_is_vdev_in_beaconning_mode(enum QDF_OPMODE vdev_opmode)
{
	switch (vdev_opmode) {
	case QDF_SAP_MODE:
	case QDF_P2P_GO_MODE:
	case QDF_IBSS_MODE:
	case QDF_NDI_MODE:
		return true;
	default:
		return false;
	}
}

/**
 * mlme_get_global_ops() - Register ext global ops
 *
 * Return: ext_ops global ops
 */
static struct mlme_ext_ops *mlme_get_global_ops(void)
{
	return &ext_ops;
}

QDF_STATUS mlme_register_mlme_ext_ops(void)
{
	mlme_set_ops_register_cb(mlme_get_global_ops);
	return QDF_STATUS_SUCCESS;
}

/**
 * mlme_register_vdev_mgr_ops() - Register vdev mgr ops
 * @vdev_mlme: vdev mlme object
 *
 * This function is called to register vdev manager operations
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_register_vdev_mgr_ops(struct vdev_mlme_obj *vdev_mlme)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;

	if (mlme_is_vdev_in_beaconning_mode(vdev->vdev_mlme.vdev_opmode))
		vdev_mlme->ops = &ap_mlme_ops;
	else if (vdev->vdev_mlme.vdev_opmode == QDF_MONITOR_MODE)
		vdev_mlme->ops = &mon_mlme_ops;
	else
		vdev_mlme->ops = &sta_mlme_ops;

	return QDF_STATUS_SUCCESS;
}

/**
 * mlme_unregister_vdev_mgr_ops() - Unregister vdev mgr ops
 * @vdev_mlme: vdev mlme object
 *
 * This function is called to unregister vdev manager operations
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_unregister_vdev_mgr_ops(struct vdev_mlme_obj *vdev_mlme)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * sta_mlme_vdev_start_send() - MLME vdev start callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to initiate actions of VDEV.start
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_mlme_vdev_start_send(struct vdev_mlme_obj *vdev_mlme,
					   uint16_t event_data_len,
					   void *event_data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_sta_mlme_vdev_start_send(vdev_mlme, event_data_len,
					    event_data);
}

/**
 * sta_mlme_start_continue() - vdev start rsp calback
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to handle the VDEV START/RESTART calback
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_mlme_start_continue(struct vdev_mlme_obj *vdev_mlme,
					  uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_sta_mlme_vdev_start_continue(vdev_mlme, data_len, data);
}

/**
 * sta_mlme_vdev_restart_send() - MLME vdev restart send
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to initiate actions of VDEV.start
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_mlme_vdev_restart_send(struct vdev_mlme_obj *vdev_mlme,
					     uint16_t event_data_len,
					     void *event_data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_sta_mlme_vdev_restart_send(vdev_mlme, event_data_len,
					    event_data);
}

/**
 * sta_mlme_vdev_start_connection() - MLME vdev start callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to initiate actions of STA connection
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_mlme_vdev_start_connection(struct vdev_mlme_obj *vdev_mlme,
						 uint16_t event_data_len,
						 void *event_data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return QDF_STATUS_SUCCESS;
}

/**
 * sta_mlme_vdev_up_send() - MLME vdev UP callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to send the vdev up command
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_mlme_vdev_up_send(struct vdev_mlme_obj *vdev_mlme,
					uint16_t event_data_len,
					void *event_data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_sta_vdev_up_send(vdev_mlme, event_data_len, event_data);
}

/**
 * sta_mlme_vdev_notify_up_complete() - MLME vdev UP complete callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to VDEV MLME on moving
 *  to UP state
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_mlme_vdev_notify_up_complete(struct vdev_mlme_obj *vdev_mlme,
						   uint16_t event_data_len,
						   void *event_data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return QDF_STATUS_SUCCESS;
}

/**
 * sta_mlme_vdev_notify_roam_start() - MLME vdev Roam start callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to VDEV MLME on roaming
 *  to UP state
 *
 * Return: QDF_STATUS
 */
static
QDF_STATUS sta_mlme_vdev_notify_roam_start(struct vdev_mlme_obj *vdev_mlme,
					   uint16_t event_data_len,
					   void *event_data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_sta_mlme_vdev_roam_notify(vdev_mlme, event_data_len,
					     event_data);
}

/**
 * sta_mlme_vdev_disconnect_bss() - MLME vdev disconnect bss callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to disconnect BSS/send deauth to AP
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_mlme_vdev_disconnect_bss(struct vdev_mlme_obj *vdev_mlme,
					       uint16_t event_data_len,
					       void *event_data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_sta_mlme_vdev_disconnect_bss(vdev_mlme, event_data_len,
						event_data);
}

/**
 * sta_mlme_vdev_stop_send() - MLME vdev stop send callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to send the vdev stop to firmware
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_mlme_vdev_stop_send(struct vdev_mlme_obj *vdev_mlme,
					  uint16_t data_len,
					  void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_sta_mlme_vdev_stop_send(vdev_mlme, data_len, data);
}

/**
 * vdevmgr_mlme_stop_continue() - MLME vdev stop send callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to initiate operations on
 * LMAC/FW stop response such as remove peer.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS vdevmgr_mlme_stop_continue(struct vdev_mlme_obj *vdev_mlme,
					     uint16_t data_len,
					     void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_mlme_vdev_stop_continue(vdev_mlme, data_len, data);
}

/**
 * ap_mlme_vdev_start_send () - send vdev start req
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to initiate actions of VDEV start ie start bss
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_start_send(struct vdev_mlme_obj *vdev_mlme,
					  uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_ap_mlme_vdev_start_send(vdev_mlme, data_len, data);
}

/**
 * ap_start_continue () - vdev start rsp calback
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to handle the VDEV START/RESTART calback
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_start_continue(struct vdev_mlme_obj *vdev_mlme,
					 uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_ap_mlme_vdev_start_continue(vdev_mlme, data_len, data);
}

/**
 * ap_mlme_vdev_update_beacon() - callback to initiate beacon update
 * @vdev_mlme: vdev mlme object
 * @op: beacon operation
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to update beacon
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_update_beacon(struct vdev_mlme_obj *vdev_mlme,
					     enum beacon_update_op op,
					     uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_ap_mlme_vdev_update_beacon(vdev_mlme, op, data_len, data);
}

/**
 * ap_mlme_vdev_up_send() - callback to send vdev up
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to send vdev up req
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_up_send(struct vdev_mlme_obj *vdev_mlme,
				       uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_ap_mlme_vdev_up_send(vdev_mlme, data_len, data);
}

/**
 * ap_mlme_vdev_notify_up_complete() - callback to notify up completion
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to indicate up is completed
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
ap_mlme_vdev_notify_up_complete(struct vdev_mlme_obj *vdev_mlme,
				uint16_t data_len, void *data)
{
	if (!vdev_mlme) {
		mlme_legacy_err("data is NULL");
		return QDF_STATUS_E_INVAL;
	}

	pe_debug("Vdev %d is up", wlan_vdev_get_id(vdev_mlme->vdev));

	return QDF_STATUS_SUCCESS;
}

/**
 * ap_mlme_vdev_disconnect_peers() - callback to disconnect all connected peers
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to disconnect all connected peers
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_disconnect_peers(struct vdev_mlme_obj *vdev_mlme,
						uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_ap_mlme_vdev_disconnect_peers(vdev_mlme, data_len, data);
}

/**
 * ap_mlme_vdev_stop_send() - callback to send stop vdev request
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to send stop vdev request
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_stop_send(struct vdev_mlme_obj *vdev_mlme,
					 uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_ap_mlme_vdev_stop_send(vdev_mlme, data_len, data);
}

/**
 * ap_mlme_vdev_is_newchan_no_cac - VDEV SM CSA complete notification
 * @vdev_mlme:  VDEV MLME comp object
 *
 * On CSA complete, checks whether Channel does not needs CAC period, if
 * it doesn't need cac return SUCCESS else FAILURE
 *
 * Return: SUCCESS if new channel doesn't need cac
 *         else FAILURE
 */
static QDF_STATUS
ap_mlme_vdev_is_newchan_no_cac(struct vdev_mlme_obj *vdev_mlme)
{
	bool cac_required;

	cac_required = mlme_get_cac_required(vdev_mlme->vdev);
	mlme_legacy_debug("vdev id = %d cac_required %d",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id, cac_required);

	if (!cac_required)
		return QDF_STATUS_SUCCESS;

	mlme_set_cac_required(vdev_mlme->vdev, false);

	return QDF_STATUS_E_FAILURE;
}

/**
 * ap_mlme_vdev_down_send() - callback to send vdev down req
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to send vdev down req
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS vdevmgr_mlme_vdev_down_send(struct vdev_mlme_obj *vdev_mlme,
					      uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_ap_mlme_vdev_down_send(vdev_mlme, data_len, data);
}

/**
 * vdevmgr_notify_down_complete() - callback to indicate vdev down is completed
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to indicate vdev down is completed
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS vdevmgr_notify_down_complete(struct vdev_mlme_obj *vdev_mlme,
					       uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_mlme_vdev_notify_down_complete(vdev_mlme, data_len, data);
}

/**
 * ap_mlme_vdev_start_req_failed () - vdev start req fail callback
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to handle vdev start req/rsp failure
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_start_req_failed(struct vdev_mlme_obj *vdev_mlme,
						uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_ap_mlme_vdev_start_req_failed(vdev_mlme, data_len, data);
}

/**
 * ap_mlme_vdev_restart_send() a callback to send vdev restart
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to initiate and send vdev restart req
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_restart_send(struct vdev_mlme_obj *vdev_mlme,
					    uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_ap_mlme_vdev_restart_send(vdev_mlme, data_len, data);
}

/**
 * ap_mlme_vdev_stop_start_send() - handle vdev stop during start req
 * @vdev_mlme: vdev mlme object
 * @type: restart req or start req
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to handle vdev stop during start req
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_stop_start_send(struct vdev_mlme_obj *vdev_mlme,
					       enum vdev_cmd_type type,
					       uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_ap_mlme_vdev_stop_start_send(vdev_mlme, type,
						data_len, data);
}

QDF_STATUS mlme_set_chan_switch_in_progress(struct wlan_objmgr_vdev *vdev,
					       bool val)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	mlme_priv->chan_switch_in_progress = val;

	return QDF_STATUS_SUCCESS;
}

bool mlme_is_chan_switch_in_progress(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return false;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	return mlme_priv->chan_switch_in_progress;
}

QDF_STATUS
ap_mlme_set_hidden_ssid_restart_in_progress(struct wlan_objmgr_vdev *vdev,
					    bool val)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	mlme_priv->hidden_ssid_restart_in_progress = val;

	return QDF_STATUS_SUCCESS;
}

bool ap_mlme_is_hidden_ssid_restart_in_progress(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return false;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	return mlme_priv->hidden_ssid_restart_in_progress;
}

QDF_STATUS mlme_set_connection_fail(struct wlan_objmgr_vdev *vdev, bool val)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	mlme_priv->connection_fail = val;

	return QDF_STATUS_SUCCESS;
}

bool mlme_is_connection_fail(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return false;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	return mlme_priv->connection_fail;
}

QDF_STATUS mlme_set_assoc_type(struct wlan_objmgr_vdev *vdev,
			       enum vdev_assoc_type assoc_type)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	mlme_priv->assoc_type = assoc_type;

	return QDF_STATUS_SUCCESS;
}

enum vdev_assoc_type  mlme_get_assoc_type(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return false;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	return mlme_priv->assoc_type;
}

QDF_STATUS
mlme_set_vdev_start_failed(struct wlan_objmgr_vdev *vdev, bool val)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	mlme_priv->vdev_start_failed = val;

	return QDF_STATUS_SUCCESS;
}

bool mlme_get_vdev_start_failed(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return false;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	return mlme_priv->vdev_start_failed;
}

QDF_STATUS mlme_set_cac_required(struct wlan_objmgr_vdev *vdev, bool val)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	mlme_priv->cac_required_for_new_channel = val;

	return QDF_STATUS_SUCCESS;
}

bool mlme_get_cac_required(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct mlme_legacy_priv *mlme_priv;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return false;
	}

	mlme_priv = (struct mlme_legacy_priv *)vdev_mlme->ext_vdev_ptr;

	return mlme_priv->cac_required_for_new_channel;
}

QDF_STATUS mlme_set_mbssid_info(struct wlan_objmgr_vdev *vdev,
				struct scan_mbssid_info *mbssid_info)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct vdev_mlme_mbss_11ax *mbss_11ax;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mbss_11ax = &vdev_mlme->mgmt.mbss_11ax;
	mbss_11ax->profile_idx = mbssid_info->profile_num;
	mbss_11ax->profile_num = mbssid_info->profile_count;
	qdf_mem_copy(mbss_11ax->trans_bssid,
		     mbssid_info->trans_bssid, QDF_MAC_ADDR_SIZE);

	return QDF_STATUS_SUCCESS;
}

void mlme_get_mbssid_info(struct wlan_objmgr_vdev *vdev,
			  struct vdev_mlme_mbss_11ax *mbss_11ax)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_legacy_err("vdev component object is NULL");
		return;
	}

	mbss_11ax = &vdev_mlme->mgmt.mbss_11ax;
}

/**
 * vdevmgr_mlme_ext_hdl_create () - Create mlme legacy priv object
 * @vdev_mlme: vdev mlme object
 *
 * Return: QDF_STATUS
 */
static
QDF_STATUS vdevmgr_mlme_ext_hdl_create(struct vdev_mlme_obj *vdev_mlme)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	vdev_mlme->ext_vdev_ptr =
		qdf_mem_malloc(sizeof(struct mlme_legacy_priv));
	if (!vdev_mlme->ext_vdev_ptr) {
		mlme_legacy_err("failed to allocate meory for ext_vdev_ptr");
		return QDF_STATUS_E_NOMEM;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * vdevmgr_mlme_ext_hdl_destroy () - Destroy mlme legacy priv object
 * @vdev_mlme: vdev mlme object
 *
 * Return: QDF_STATUS
 */
static
QDF_STATUS vdevmgr_mlme_ext_hdl_destroy(struct vdev_mlme_obj *vdev_mlme)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	qdf_mem_free(vdev_mlme->ext_vdev_ptr);
	vdev_mlme->ext_vdev_ptr = NULL;

	return QDF_STATUS_SUCCESS;
}

/**
 * ap_vdev_dfs_cac_timer_stop() – callback to stop cac timer
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to stop cac timer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_vdev_dfs_cac_timer_stop(struct vdev_mlme_obj *vdev_mlme,
					     uint16_t event_data_len,
					     void *event_data)
{
	mlme_legacy_debug("vdev id = %d ",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return QDF_STATUS_SUCCESS;
}

/**
 * mon_mlme_vdev_start_restart_send () - send vdev start/restart req
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to initiate actions of VDEV start/restart
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS mon_mlme_vdev_start_restart_send(
	struct vdev_mlme_obj *vdev_mlme,
	uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return lim_mon_mlme_vdev_start_send(vdev_mlme, data_len, data);
}

/**
 * mon_start_continue () - vdev start rsp calback
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to handle the VDEV START/RESTART calback
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS mon_mlme_start_continue(struct vdev_mlme_obj *vdev_mlme,
					  uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_mon_mlme_vdev_start_continue(vdev_mlme, data_len, data);
}

/**
 * mon_mlme_vdev_up_send() - callback to send vdev up
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to send vdev up req
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS mon_mlme_vdev_up_send(struct vdev_mlme_obj *vdev_mlme,
					uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_mon_mlme_vdev_up_send(vdev_mlme, data_len, data);
}

/**
 * mon_mlme_vdev_disconnect_peers() - callback to disconnect all connected peers
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * montior mode no connected peers, only do VDEV state transition.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS mon_mlme_vdev_disconnect_peers(
		struct vdev_mlme_obj *vdev_mlme,
		uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wlan_vdev_mlme_sm_deliver_evt(
				vdev_mlme->vdev,
				WLAN_VDEV_SM_EV_DISCONNECT_COMPLETE,
				0, NULL);
}

/**
 * mon_mlme_vdev_stop_send() - callback to send stop vdev request
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to send stop vdev request
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS mon_mlme_vdev_stop_send(struct vdev_mlme_obj *vdev_mlme,
					  uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_mon_mlme_vdev_stop_send(vdev_mlme, data_len, data);
}

/**
 * mon_mlme_vdev_down_send() - callback to send vdev down req
 * @vdev_mlme: vdev mlme object
 * @data_len: event data length
 * @data: event data
 *
 * This function is called to send vdev down req
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS mon_mlme_vdev_down_send(struct vdev_mlme_obj *vdev_mlme,
					  uint16_t data_len, void *data)
{
	mlme_legacy_debug("vdev id = %d",
			  vdev_mlme->vdev->vdev_objmgr.vdev_id);
	return wma_mon_mlme_vdev_down_send(vdev_mlme, data_len, data);
}

/**
 * struct sta_mlme_ops - VDEV MLME operation callbacks strucutre for sta
 * @mlme_vdev_start_send:               callback to initiate actions of VDEV
 *                                      MLME start operation
 * @mlme_vdev_restart_send:             callback to initiate actions of VDEV
 *                                      MLME restart operation
 * @mlme_vdev_stop_start_send:          callback to block start/restart VDEV
 *                                      request command
 * @mlme_vdev_sta_conn_start:           callback to initiate connection
 * @mlme_vdev_start_continue:           callback to initiate operations on
 *                                      LMAC/FW start response
 * @mlme_vdev_up_send:                  callback to initiate actions of VDEV
 *                                      MLME up operation
 * @mlme_vdev_notify_up_complete:       callback to notify VDEV MLME on moving
 *                                      to UP state
 * @mlme_vdev_update_beacon:            callback to initiate beacon update
 * @mlme_vdev_disconnect_peers:         callback to initiate disconnection of
 *                                      peers
 * @mlme_vdev_stop_send:                callback to initiate actions of VDEV
 *                                      MLME stop operation
 * @mlme_vdev_stop_continue:            callback to initiate operations on
 *                                      LMAC/FW stop response
 * @mlme_vdev_down_send:                callback to initiate actions of VDEV
 *                                      MLME down operation
 * @mlme_vdev_notify_down_complete:     callback to notify VDEV MLME on moving
 *                                      to INIT state
 */
static struct vdev_mlme_ops sta_mlme_ops = {
	.mlme_vdev_start_send = sta_mlme_vdev_start_send,
	.mlme_vdev_restart_send = sta_mlme_vdev_restart_send,
	.mlme_vdev_start_continue = sta_mlme_start_continue,
	.mlme_vdev_sta_conn_start = sta_mlme_vdev_start_connection,
	.mlme_vdev_up_send = sta_mlme_vdev_up_send,
	.mlme_vdev_notify_up_complete = sta_mlme_vdev_notify_up_complete,
	.mlme_vdev_notify_roam_start = sta_mlme_vdev_notify_roam_start,
	.mlme_vdev_disconnect_peers = sta_mlme_vdev_disconnect_bss,
	.mlme_vdev_stop_send = sta_mlme_vdev_stop_send,
	.mlme_vdev_stop_continue = vdevmgr_mlme_stop_continue,
	.mlme_vdev_down_send = vdevmgr_mlme_vdev_down_send,
	.mlme_vdev_notify_down_complete = vdevmgr_notify_down_complete,
};

/**
 * struct ap_mlme_ops - VDEV MLME operation callbacks strucutre for beaconing
 *                      interface
 * @mlme_vdev_start_send:               callback to initiate actions of VDEV
 *                                      MLME start operation
 * @mlme_vdev_restart_send:             callback to initiate actions of VDEV
 *                                      MLME restart operation
 * @mlme_vdev_stop_start_send:          callback to block start/restart VDEV
 *                                      request command
 * @mlme_vdev_start_continue:           callback to initiate operations on
 *                                      LMAC/FW start response
 * @mlme_vdev_up_send:                  callback to initiate actions of VDEV
 *                                      MLME up operation
 * @mlme_vdev_notify_up_complete:       callback to notify VDEV MLME on moving
 *                                      to UP state
 * @mlme_vdev_update_beacon:            callback to initiate beacon update
 * @mlme_vdev_disconnect_peers:         callback to initiate disconnection of
 *                                      peers
 * @mlme_vdev_dfs_cac_timer_stop:       callback to stop the DFS CAC timer
 * @mlme_vdev_stop_send:                callback to initiate actions of VDEV
 *                                      MLME stop operation
 * @mlme_vdev_stop_continue:            callback to initiate operations on
 *                                      LMAC/FW stop response
 * @mlme_vdev_down_send:                callback to initiate actions of VDEV
 *                                      MLME down operation
 * @mlme_vdev_notify_down_complete:     callback to notify VDEV MLME on moving
 *                                      to INIT state
 * @mlme_vdev_is_newchan_no_cac:        callback to check if new channel is DFS
 *                                      and cac is not required
 */
static struct vdev_mlme_ops ap_mlme_ops = {
	.mlme_vdev_start_send = ap_mlme_vdev_start_send,
	.mlme_vdev_restart_send = ap_mlme_vdev_restart_send,
	.mlme_vdev_stop_start_send = ap_mlme_vdev_stop_start_send,
	.mlme_vdev_start_continue = ap_mlme_start_continue,
	.mlme_vdev_start_req_failed = ap_mlme_vdev_start_req_failed,
	.mlme_vdev_up_send = ap_mlme_vdev_up_send,
	.mlme_vdev_notify_up_complete = ap_mlme_vdev_notify_up_complete,
	.mlme_vdev_update_beacon = ap_mlme_vdev_update_beacon,
	.mlme_vdev_disconnect_peers = ap_mlme_vdev_disconnect_peers,
	.mlme_vdev_dfs_cac_timer_stop = ap_vdev_dfs_cac_timer_stop,
	.mlme_vdev_stop_send = ap_mlme_vdev_stop_send,
	.mlme_vdev_stop_continue = vdevmgr_mlme_stop_continue,
	.mlme_vdev_down_send = vdevmgr_mlme_vdev_down_send,
	.mlme_vdev_notify_down_complete = vdevmgr_notify_down_complete,
	.mlme_vdev_is_newchan_no_cac = ap_mlme_vdev_is_newchan_no_cac,
};

static struct vdev_mlme_ops mon_mlme_ops = {
	.mlme_vdev_start_send = mon_mlme_vdev_start_restart_send,
	.mlme_vdev_restart_send = mon_mlme_vdev_start_restart_send,
	.mlme_vdev_start_continue = mon_mlme_start_continue,
	.mlme_vdev_up_send = mon_mlme_vdev_up_send,
	.mlme_vdev_disconnect_peers = mon_mlme_vdev_disconnect_peers,
	.mlme_vdev_stop_send = mon_mlme_vdev_stop_send,
	.mlme_vdev_down_send = mon_mlme_vdev_down_send,
};

/**
 * struct mlme_ext_ops - VDEV MLME legacy global callbacks structure
 * @mlme_vdev_ext_hdl_create:           callback to invoke creation of legacy
 *                                      vdev object
 * @mlme_vdev_ext_hdl_destroy:          callback to invoke destroy of legacy
 *                                      vdev object
 */
static struct mlme_ext_ops ext_ops = {
	.mlme_vdev_ext_hdl_create = vdevmgr_mlme_ext_hdl_create,
	.mlme_vdev_ext_hdl_destroy = vdevmgr_mlme_ext_hdl_destroy,
};
