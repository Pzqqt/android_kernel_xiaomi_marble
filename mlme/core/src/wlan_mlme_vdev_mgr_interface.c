/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_mlme_main.h"
#include "wlan_mlme_vdev_mgr_interface.h"

static struct vdev_mlme_ops sta_mlme_ops;
static struct vdev_mlme_ops ap_mlme_ops;

/**
 * mlme_is_vdev_in_beaconning_mode() - check if vdev is beaconing mode
 * @vdev_opmode: vdev opmode
 *
 * This function is called to register vdev manager operations
 *
 * Return: QDF_STATUS
 */
static inline bool mlme_is_vdev_in_beaconning_mode(enum QDF_OPMODE vdev_opmode)
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
 * mlme_register_vdev_mgr_ops() - Register vdev mgr ops
 * @vdev_mlme: vdev mlme object
 *
 * This function is called to register vdev manager operations
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_register_vdev_mgr_ops(void *mlme)
{
	struct wlan_objmgr_vdev *vdev;
	struct vdev_mlme_obj *vdev_mlme = (struct vdev_mlme_obj *)mlme;

	vdev = vdev_mlme->vdev;

	if (mlme_is_vdev_in_beaconning_mode(vdev->vdev_mlme.vdev_opmode))
		vdev_mlme->ops = &ap_mlme_ops;
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
	return QDF_STATUS_SUCCESS;
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
	return QDF_STATUS_SUCCESS;
}

/**
 * sta_mlme_vdev_start_continue() - MLME vdev restart send
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to initiate actions of VDEV.start
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_mlme_vdev_start_continue(struct vdev_mlme_obj *vdev_mlme,
					       uint16_t event_data_len,
					       void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * sta_mlme_vdev_start_continue() - MLME vdev start callback
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
	return QDF_STATUS_SUCCESS;
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
	return QDF_STATUS_SUCCESS;
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
	return QDF_STATUS_SUCCESS;
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
					  uint16_t event_data_len,
					  void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * sta_mlme_vdev_stop_continue() - MLME vdev stop send callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to initiate operations on
 * LMAC/FW stop response such as remove peer.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_mlme_vdev_stop_continue(struct vdev_mlme_obj *vdev_mlme,
					      uint16_t event_data_len,
					      void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * sta_mlme_vdev_down_send() - MLME vdev down send callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is to send the vdev down to firmware
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_mlme_vdev_down_send(struct vdev_mlme_obj *vdev_mlme,
					  uint16_t event_data_len,
					  void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * sta_vdev_notify_down_complete() - MLME vdev down complete callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called on moving vdev state to down.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS sta_vdev_notify_down_complete(struct vdev_mlme_obj *vdev_mlme,
						 uint16_t event_data_len,
						 void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ap_mlme_vdev_start_send () - send vdev start
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to initiate actions of VDEV start ie start bss
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_start_send(struct vdev_mlme_obj *vdev_mlme,
					  uint16_t event_data_len,
					  void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ap_mlme_vdev_start_continue () - vdev start rsp calback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to handle the VDEV START/RESTART calback
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_start_continue(struct vdev_mlme_obj *vdev_mlme,
					      uint16_t event_data_len,
					      void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ap_mlme_vdev_start_req_failed () - vdev start req fail callback
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to handle vdev start req/rsp failure
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_start_req_failed(struct vdev_mlme_obj *vdev_mlme,
						uint16_t event_data_len,
						void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ap_mlme_vdev_update_beacon() - callback to initiate beacon update
 * @vdev_mlme: vdev mlme object
 * @op: beacon operation
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to update beacon
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_update_beacon(struct vdev_mlme_obj *vdev_mlme,
					     enum beacon_update_op op,
					     uint16_t event_data_len,
					     void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ap_mlme_vdev_up_send() – callback to send vdev up
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to send vdev up req
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_up_send(struct vdev_mlme_obj *vdev_mlme,
				       uint16_t event_data_len,
				       void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ap_mlme_vdev_notify_up_complete() – callback to notify up completion
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to indicate up is completed
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_notify_up_complete(struct vdev_mlme_obj *vdev_mlme,
						  uint16_t event_data_len,
						  void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * sap_mlme_vdev_restart_send() a callback to send vdev restart
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to initiate and send vdev restart req
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_restart_send(struct vdev_mlme_obj *vdev_mlme,
					    uint16_t event_data_len,
					    void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS ap_mlme_vdev_stop_start_send(struct vdev_mlme_obj *vdev_mlme,
					       uint8_t restart,
					       uint16_t event_data_len,
					       void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ap_mlme_vdev_disconnect_peers() – callback to disconnect all
 *                                   connected peers
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to disconnect all connected peers
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_disconnect_peers(struct vdev_mlme_obj *vdev_mlme,
						uint16_t event_data_len,
						void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ap_mlme_vdev_stop_send() – callback to send stop vdev request
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to send stop vdev request
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_stop_send(struct vdev_mlme_obj *vdev_mlme,
					 uint16_t event_data_len,
					 void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ap_mlme_vdev_stop_continue() – callback to handle stop vdev resp
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to handle stop vdev resp
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_stop_continue(struct vdev_mlme_obj *vdev_mlme,
					     uint16_t event_data_len,
					     void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ap_mlme_vdev_down_send() – callback to send vdev down req
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to send vdev down req
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_mlme_vdev_down_send(struct vdev_mlme_obj *vdev_mlme,
					 uint16_t event_data_len,
					 void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * ap_vdev_notify_down_complete() – callback to indicate vdev
 *                                  down is completed
 * @vdev_mlme: vdev mlme object
 * @event_data_len: event data length
 * @event_data: event data
 *
 * This function is called to indicate vdev down is completed
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ap_vdev_notify_down_complete(struct vdev_mlme_obj *vdev_mlme,
					       uint16_t event_data_len,
					       void *event_data)
{
	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS ap_mlme_vdev_legacy_hdl_create(struct vdev_mlme_obj *vdev_mlme)
{
	return QDF_STATUS_SUCCESS;
}

static
QDF_STATUS ap_mlme_vdev_legacy_hdl_destroy(struct vdev_mlme_obj *vdev_mlme)
{
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
	return QDF_STATUS_SUCCESS;
}

/**
 * struct sta_mlme_ops - VDEV MLME operation callbacks strucutre for sta
 * @mlme_vdev_validate_basic_params:    callback to validate VDEV basic params
 * @mlme_vdev_reset_proto_params:       callback to Reset protocol params
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
 * @mlme_vdev_bss_peer_delete_continue: callback to initiate operations on BSS
 *                                      peer delete completion
 * @mlme_vdev_down_send:                callback to initiate actions of VDEV
 *                                      MLME down operation
 * @mlme_vdev_legacy_hdl_create:        callback to invoke creation of legacy
 *                                      vdev object
 * @mlme_vdev_legacy_hdl_post_create:   callback to invoke post creation actions
 *                                      of legacy vdev object
 * @mlme_vdev_legacy_hdl_destroy:       callback to invoke destroy of legacy
 *                                      vdev object
 */
static struct vdev_mlme_ops sta_mlme_ops = {
	.mlme_vdev_start_send = sta_mlme_vdev_start_send,
	.mlme_vdev_restart_send = sta_mlme_vdev_restart_send,
	.mlme_vdev_start_continue = sta_mlme_vdev_start_continue,
	.mlme_vdev_sta_conn_start = sta_mlme_vdev_start_connection,
	.mlme_vdev_up_send = sta_mlme_vdev_up_send,
	.mlme_vdev_notify_up_complete = sta_mlme_vdev_notify_up_complete,
	.mlme_vdev_disconnect_peers = sta_mlme_vdev_disconnect_bss,
	.mlme_vdev_stop_send = sta_mlme_vdev_stop_send,
	.mlme_vdev_stop_continue = sta_mlme_vdev_stop_continue,
	.mlme_vdev_down_send = sta_mlme_vdev_down_send,
	.mlme_vdev_notify_down_complete = sta_vdev_notify_down_complete,
};

/**
 * struct ap_mlme_ops - VDEV MLME operation callbacks strucutre for beaconing
 *                      interface
 * @mlme_vdev_validate_basic_params:    callback to validate VDEV basic params
 * @mlme_vdev_reset_proto_params:       callback to Reset protocol params
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
 * @mlme_vdev_bss_peer_delete_continue: callback to initiate operations on BSS
 *                                      peer delete completion
 * @mlme_vdev_down_send:                callback to initiate actions of VDEV
 *                                      MLME down operation
 * @mlme_vdev_legacy_hdl_create:        callback to invoke creation of legacy
 *                                      vdev object
 * @mlme_vdev_legacy_hdl_post_create:   callback to invoke post creation actions
 *                                      of legacy vdev object
 * @mlme_vdev_legacy_hdl_destroy:       callback to invoke destroy of legacy
 *                                      vdev object
 */
static struct vdev_mlme_ops ap_mlme_ops = {
	.mlme_vdev_start_send = ap_mlme_vdev_start_send,
	.mlme_vdev_restart_send = ap_mlme_vdev_restart_send,
	.mlme_vdev_stop_start_send = ap_mlme_vdev_stop_start_send,
	.mlme_vdev_start_continue = ap_mlme_vdev_start_continue,
	.mlme_vdev_start_req_failed = ap_mlme_vdev_start_req_failed,
	.mlme_vdev_up_send = ap_mlme_vdev_up_send,
	.mlme_vdev_notify_up_complete = ap_mlme_vdev_notify_up_complete,
	.mlme_vdev_update_beacon = ap_mlme_vdev_update_beacon,
	.mlme_vdev_disconnect_peers = ap_mlme_vdev_disconnect_peers,
	.mlme_vdev_disconnect_peers = ap_vdev_dfs_cac_timer_stop,
	.mlme_vdev_stop_send = ap_mlme_vdev_stop_send,
	.mlme_vdev_stop_continue = ap_mlme_vdev_stop_continue,
	.mlme_vdev_down_send = ap_mlme_vdev_down_send,
	.mlme_vdev_notify_down_complete = ap_vdev_notify_down_complete,
	.mlme_vdev_legacy_hdl_create = ap_mlme_vdev_legacy_hdl_create,
	.mlme_vdev_legacy_hdl_destroy = ap_mlme_vdev_legacy_hdl_destroy,
};
