/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: vdev_mgr_iface_api.h
 *
 * API declarations required for following
 * - get/set common mlme data structure fields
 * - send WMI command using Tx Ops
 */

#ifndef __WLAN_VDEV_MGR_TX_OPS_API_H__
#define __WLAN_VDEV_MGR_TX_OPS_API_H__

#ifdef CMN_VDEV_MGR_TGT_IF_ENABLE
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_vdev_mgr_tgt_if_tx_defs.h>
#include <include/wlan_vdev_mlme.h>

/**
 * tgt_vdev_mgr_create_send() – API to send create command
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to vdev_create_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_create_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_create_params *param);

/**
 * tgt_vdev_mgr_create_complete() – API to send wmi cfg corresponding
 * to create command
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_create_complete(struct vdev_mlme_obj *mlme_obj);

/**
 * tgt_vdev_mgr_start_send() – API to send start command
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to vdev_start_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_start_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_start_params *param);

/**
 * tgt_vdev_mgr_delete_send() – API to send delete
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to vdev_delete_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_delete_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_delete_params *param);

/**
 * tgt_vdev_mgr_peer_flush_tids_send() – API to send peer flush tids in FW
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to peer_flush_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_peer_flush_tids_send(
				struct vdev_mlme_obj *mlme_obj,
				struct peer_flush_params *param);

/**
 * tgt_vdev_mgr_stop_send() – API to send stop command
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to vdev_stop_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_stop_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_stop_params *param);

/**
 * tgt_vdev_mgr_beacon_stop() – API to handle beacon buffer
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_beacon_stop(struct vdev_mlme_obj *mlme_obj);

/**
 * tgt_vdev_mgr_beacon_free() – API to free beacon buffer
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_beacon_free(struct vdev_mlme_obj *mlme_obj);

/**
 * tgt_vdev_mgr_up_send() – API to send up
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to vdev_up_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_up_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_up_params *param);

/**
 * tgt_vdev_mgr_down_send() – API to send down
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to vdev_down_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_down_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_down_params *param);

/**
 * tgt_vdev_mgr_set_neighbour_rx_cmd_send() – API to send neighbour rx
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to set_neighbour_rx_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_set_neighbour_rx_cmd_send(
				struct vdev_mlme_obj *mlme_obj,
				struct set_neighbour_rx_params *param);

/**
 * tgt_vdev_mgr_nac_rssi_send() – API to send NAC RSSI
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to vdev_scan_nac_rssi_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_nac_rssi_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_scan_nac_rssi_params *param);

/**
 * tgt_vdev_mgr_sifs_trigger_send() – API to send SIFS trigger
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to sifs_trigger_param
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_sifs_trigger_send(
				struct vdev_mlme_obj *mlme_obj,
				struct sifs_trigger_param *param);

/**
 * tgt_vdev_mgr_set_custom_aggr_size_send() – API to send custom aggr size
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to set_custom_aggr_size_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_set_custom_aggr_size_send(
				struct vdev_mlme_obj *mlme_obj,
				struct set_custom_aggr_size_params *param);

/**
 * tgt_vdev_mgr_config_ratemask_cmd_send() – API to configure ratemask
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to config_ratemask_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_config_ratemask_cmd_send(
				struct vdev_mlme_obj *mlme_obj,
				struct config_ratemask_params *param);

/**
 * tgt_vdev_mgr_sta_ps_param_send() – API to send sta power save configuration
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to sta_ps_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_sta_ps_param_send(
				struct vdev_mlme_obj *mlme_obj,
				struct sta_ps_params *param);

/**
 * tgt_vdev_mgr_beacon_cmd_send() – API to send beacon
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to beacon_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_beacon_cmd_send(
				struct vdev_mlme_obj *mlme_obj,
				struct beacon_params *param);

/**
 * tgt_vdev_mgr_beacon_tmpl_send() – API to send beacon template
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to beacon_tmpl_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_beacon_tmpl_send(
				struct vdev_mlme_obj *mlme_obj,
				struct beacon_tmpl_params *param);

/**
 * tgt_vdev_mgr_multiple_vdev_restart_send() – API to send multiple vdev
 * restart
 * @pdev: pointer to pdev
 * @param: pointer to multiple_vdev_restart_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_multiple_vdev_restart_send(
				struct wlan_objmgr_pdev *pdev,
				struct multiple_vdev_restart_params *param);

/**
 * tgt_vdev_mgr_set_param_send() – API to send parameter cfg
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to vdev_set_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_set_param_send(
				struct vdev_mlme_obj *mlme_obj,
				struct vdev_set_params *param);

/**
 * tgt_vdev_mgr_bcn_miss_offload_send() – API to send beacon miss offload
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS tgt_vdev_mgr_bcn_miss_offload_send(struct vdev_mlme_obj *mlme_obj);

#endif /* CMN_VDEV_MGR_TGT_IF_ENABLE */
#endif /* __WLAN_VDEV_MGR_TX_OPS_API_H__ */
