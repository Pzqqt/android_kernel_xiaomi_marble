/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC : contains interface prototypes for OS_IF layer
 */
#ifndef _SON_UCFG_API_H_
#define _SON_UCFG_API_H_

#include <qdf_trace.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_mlme_ucfg_api.h>
#include <son_api.h>

/**
 * ucfg_son_get_operation_chan_freq_vdev_id() - get operating chan freq of
 *                                              given vdev id
 * @pdev: Pointer to pdev
 * @vdev_id: vdev id
 *
 * Return: chan freq of given vdev id
 */
qdf_freq_t
ucfg_son_get_operation_chan_freq_vdev_id(struct wlan_objmgr_pdev *pdev,
					 uint8_t vdev_id);

/**
 * ucfg_son_get_min_and_max_power() - get min and max power
 * @psoc: pointer to psoc
 * @max_tx_power: max tx power(dBm units) to get.
 * @min_tx_power: min tx power(dBm units) to get.
 *
 * Return: Void
 */
void ucfg_son_get_min_and_max_power(struct wlan_objmgr_psoc *psoc,
				    int8_t *max_tx_power,
				    int8_t *min_tx_power);

/**
 * ucfg_son_is_cac_in_progress() - whether cac in progress or not
 * @vdev: Pointer to vdev
 *
 * Return: whether vdev in cac or not
 */
bool ucfg_son_is_cac_in_progress(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_son_get_sta_count() - get sta count
 * @vdev: Pointer to vdev
 *
 * Return: sta count
 */
uint32_t ucfg_son_get_sta_count(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_son_get_chan_flag() - get chan flag
 * @pdev: pointer to pdev
 * @freq: qdf_freq_t
 * @flag_160: whether 160 band width is enabled or not
 * @chan_params: chan parameters
 *
 * Return: chan flag
 */
uint32_t ucfg_son_get_chan_flag(struct wlan_objmgr_pdev *pdev,
				qdf_freq_t freq, bool flag_160,
				struct ch_params *chan_params);

/**
 * ucfg_son_get_he_supported() - get he supported
 * @psoc: pointer to psoc
 * @he_supported: he supported or not
 *
 * Return: void
 */
#ifdef WLAN_FEATURE_11AX
void ucfg_son_get_he_supported(struct wlan_objmgr_psoc *psoc,
			       bool *he_supported);
#else
static inline void ucfg_son_get_he_supported(struct wlan_objmgr_psoc *psoc,
					     bool *he_supported)
{
	*he_supported = false;
}
#endif /*WLAN_FEATURE_11AX*/

/**
 * ucfg_son_set_peer_kickout_allow() - set the peer is allowed to kickout
 * @vdev: pointer to vdev
 * @peer: pointer to peer
 * @kickout_allow: kickout_allow to set
 *
 * Return: QDF_STATUS_SUCCESS on Success else failure.
 */
QDF_STATUS ucfg_son_set_peer_kickout_allow(struct wlan_objmgr_vdev *vdev,
					   struct wlan_objmgr_peer *peer,
					   bool kickout_allow);

/**
 * ucfg_son_register_deliver_opmode_cb() - register deliver opmode cb
 * psoc: pointer to psoc
 * cb: deliver opmode callback
 *
 * Return: QDF_STATUS_SUCCESS on Success else failure.
 */
QDF_STATUS ucfg_son_register_deliver_opmode_cb(struct wlan_objmgr_psoc *psoc,
					       mlme_deliver_cb cb);

/**
 * ucfg_son_register_deliver_smps_cb() - register deliver smps cb
 * psoc: pointer to psoc
 * cb: deliver smps callback
 *
 * Return: QDF_STATUS_SUCCESS on Success else failure.
 */

QDF_STATUS ucfg_son_register_deliver_smps_cb(struct wlan_objmgr_psoc *psoc,
					     mlme_deliver_cb cb);
#endif
