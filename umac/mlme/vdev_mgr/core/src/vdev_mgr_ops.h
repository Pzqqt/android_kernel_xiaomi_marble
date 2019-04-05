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
 * DOC: vdev_mgr_ops.h
 *
 * This header file provides API declarations for filling data structures
 * and sending vdev mgmt commands to target_if/mlme/vdev_mgr
 */

#ifndef __VDEV_MGR_OPS_H__
#define __VDEV_MGR_OPS_H__

#ifdef CMN_VDEV_MGR_TGT_IF_ENABLE
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_vdev_mgr_tgt_if_tx_defs.h>
#include <wlan_mlme_dbg.h>
#include "include/wlan_vdev_mlme.h"

/**
 * vdev_mgr_create_send() – MLME API to create command to
 * target_if
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_create_send(struct vdev_mlme_obj *mlme_obj);

/**
 * vdev_mgr_start_send() – MLME API to send start request to
 * target_if
 * @mlme_obj: pointer to vdev_mlme_obj
 * @restart: flag to indicate type of request START/RESTART
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_start_send(struct vdev_mlme_obj *mlme_obj, bool restart);

/**
 * vdev_mgr_delete_send() – MLME API to send delete request to
 * target_if
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_delete_send(struct vdev_mlme_obj *mlme_obj);

/**
 * vdev_mgr_peer_flush_tids_send () – MLME API to setup peer flush tids
 * @mlme_obj: pointer to vdev_mlme_obj
 * @mac: pointer to peer mac address
 * @peer_tid_bitmap: peer tid bitmap
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_peer_flush_tids_send(struct vdev_mlme_obj *mlme_obj,
					 uint8_t *mac,
					 uint32_t peer_tid_bitmap);
/**
 * vdev_mgr_stop_send () – MLME API to send STOP request
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_stop_send(struct vdev_mlme_obj *mlme_obj);

/**
 * vdev_mgr_up_send () – MLME API to send UP command
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_up_send(struct vdev_mlme_obj *mlme_obj);

/**
 * vdev_mgr_down_send () – MLME API to send down command
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_down_send(struct vdev_mlme_obj *mlme_obj);

/**
 * vdev_mgr_set_param_send() – MLME API to send vdev param
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param_id: parameter id
 * @param_value: value corresponding to parameter id
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_set_param_send(struct vdev_mlme_obj *mlme_obj,
				   uint32_t param_id,
				   uint32_t param_value);

/**
 * vdev_mgr_set_neighbour_rx_cmd_send() – MLME API to send neighbour Rx
 * cmd
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to set neighbour rx params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_set_neighbour_rx_cmd_send(
				struct vdev_mlme_obj *mlme_obj,
				struct set_neighbour_rx_params *param);

/**
 * vdev_mgr_set_nac_rssi_send() – MLME API to send nac rssi
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to vdev_scan_nac_rssi_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_nac_rssi_send(
			struct vdev_mlme_obj *mlme_obj,
			struct vdev_scan_nac_rssi_params *param);

/**
 * vdev_mgr_sifs_trigger_send() – MLME API to send SIFS trigger
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param_value: parameter value
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_sifs_trigger_send(
			struct vdev_mlme_obj *mlme_obj,
			uint32_t param_value);

/**
 * vdev_mgr_set_custom_aggr_size_cmd_send() – MLME API to send custom aggr
 * size
 * @mlme_obj: pointer to vdev_mlme_obj
 * @is_amsdu: boolean to indicate value corresponds to amsdu/ampdu
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_set_custom_aggr_size_send(
			struct vdev_mlme_obj *mlme_obj, bool is_amsdu);

/**
 * vdev_mgr_onfig_ratemask_cmd_send() – MLME API to send ratemask
 * @mlme_obj: pointer to vdev_mlme_obj
 * @type: type of ratemask configuration
 * @lower32: Lower 32 bits in the 1st 64-bit value
 * @higher32: Higher 32 bits in the 1st 64-bit value
 * @lower32_2: Lower 32 bits in the 2nd 64-bit value
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_config_ratemask_cmd_send(
				struct vdev_mlme_obj *mlme_obj,
				uint8_t type,
				uint32_t lower32,
				uint32_t higher32,
				uint32_t lower32_2);

/**
 * vdev_mgr_beacon_stop() – MLME API to stop beacon
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_beacon_stop(struct vdev_mlme_obj *mlme_obj);

/**
 * vdev_mgr_beacon_free() – MLME API to free beacon
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_beacon_free(struct vdev_mlme_obj *mlme_obj);

/**
 * vdev_mgr_beacon_send() – MLME API to send beacon
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to beacon_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_beacon_send(struct vdev_mlme_obj *mlme_obj,
				struct beacon_params *param);

/**
 * vdev_mgr_beacon_tmpl_send() – MLME API to send beacon template
 * @mlme_obj: pointer to vdev_mlme_obj
 * @param: pointer to beacon_tmpl_params
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_beacon_tmpl_send(struct vdev_mlme_obj *mlme_obj,
				     struct beacon_tmpl_params *param);

/**
 * vdev_mgr_bcn_miss_offload_send() – MLME API to send bcn miss offload
 * @mlme_obj: pointer to vdev_mlme_obj
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mgr_bcn_miss_offload_send(struct vdev_mlme_obj *mlme_obj);

/**
 * vdev_mgr_multiple_restart_send() – MLME API to send multiple vdev restart
 * @pdev: pointer to pdev object
 * @chan: pointer to channel param structure
 * @disable_hw_ack: ddisable hw ack value
 * @vdev_ids: pointer to list of vdev ids which require restart
 * @num_vdevs: number of vdevs in list
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS vdev_mlme_multiple_restart_send(struct wlan_objmgr_pdev *pdev,
					   struct mlme_channel_param *chan,
					   uint32_t disable_hw_ack,
					   uint32_t *vdev_ids,
					   uint32_t num_vdevs);

#endif /* CMN_VDEV_MGR_TGT_IF_ENABLE */
#endif /* __VDEV_MGR_OPS_H__ */
