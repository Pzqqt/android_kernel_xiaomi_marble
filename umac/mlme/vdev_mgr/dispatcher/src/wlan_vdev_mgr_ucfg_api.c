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
 * DOC: wlan_vdev_mgr_ucfg_api.c
 *
 * This file provides definitions to APIs to get/set mlme fields in
 * vdev mlme core data structures
 */
#include <wlan_vdev_mgr_tgt_if_tx_api.h>
#include "wlan_vdev_mgr_ucfg_api.h"
#include "include/wlan_vdev_mlme.h"
#include <wlan_mlme_dbg.h>
#include <wlan_vdev_mgr_utils_api.h>
#include <wlan_vdev_mlme_api.h>
#include <qdf_module.h>

void ucfg_wlan_vdev_mgr_get_param_bssid(
				struct wlan_objmgr_vdev *vdev,
				uint8_t *bssid)
{
	struct vdev_mlme_mgmt *mlme_mgmt;
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_objmgr_vdev_get_comp_private_obj(
						vdev, WLAN_UMAC_COMP_MLME);

	if (!vdev_mlme) {
		mlme_err("VDEV_MLME is NULL");
		return;
	}

	mlme_mgmt = &vdev_mlme->mgmt;

	qdf_mem_copy(bssid, mlme_mgmt->generic.bssid,
		     QDF_MAC_ADDR_SIZE);
}

qdf_export_symbol(ucfg_wlan_vdev_mgr_get_param_bssid);

void ucfg_wlan_vdev_mgr_get_param_ssid(
				struct wlan_objmgr_vdev *vdev,
				uint8_t *ssid, uint8_t *ssid_len)
{
	struct vdev_mlme_mgmt *mlme_mgmt;
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_objmgr_vdev_get_comp_private_obj(
			vdev, WLAN_UMAC_COMP_MLME);

	if (!vdev_mlme) {
		QDF_ASSERT(0);
		return;
	}

	mlme_mgmt = &vdev_mlme->mgmt;

	*ssid_len = mlme_mgmt->generic.ssid_len;
	qdf_mem_copy(ssid, mlme_mgmt->generic.ssid,
		     mlme_mgmt->generic.ssid_len);
}

qdf_export_symbol(ucfg_wlan_vdev_mgr_get_param_ssid);

void ucfg_wlan_vdev_mgr_get_beacon_buffer(
				struct wlan_objmgr_vdev *vdev,
				qdf_nbuf_t buf)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct vdev_mlme_mgmt *mlme_mgmt;

	vdev_mlme = wlan_objmgr_vdev_get_comp_private_obj(
			vdev, WLAN_UMAC_COMP_MLME);

	if (!vdev_mlme) {
		QDF_ASSERT(0);
		return;
	}

	mlme_mgmt = &vdev_mlme->mgmt;

	buf = mlme_mgmt->beacon_info.beacon_buffer;
}

qdf_export_symbol(ucfg_wlan_vdev_mgr_get_beacon_buffer);

void ucfg_wlan_vdev_mgr_get_trans_bssid(
				struct wlan_objmgr_vdev *vdev,
				uint8_t *addr)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct vdev_mlme_mgmt *mlme_mgmt;

	vdev_mlme = wlan_objmgr_vdev_get_comp_private_obj(
			vdev, WLAN_UMAC_COMP_MLME);

	if (!vdev_mlme) {
		QDF_ASSERT(0);
		return;
	}

	mlme_mgmt = &vdev_mlme->mgmt;

	qdf_mem_copy(addr, mlme_mgmt->mbss_11ax.trans_bssid, QDF_MAC_ADDR_SIZE);
}

qdf_export_symbol(ucfg_wlan_vdev_mgr_get_trans_bssid);

void ucfg_wlan_vdev_mgr_get_tsf_adjust(
				struct wlan_objmgr_vdev *vdev,
				uint64_t *tsf_adjust)
{
	struct vdev_mlme_obj *vdev_mlme;
	struct vdev_mlme_proto *mlme_proto;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);

	if (!vdev_mlme) {
		mlme_err("VDEV MLME is NULL");
		return;
	}

	mlme_proto = &vdev_mlme->proto;

	*tsf_adjust = mlme_proto->generic.tsfadjust;
}

qdf_export_symbol(ucfg_wlan_vdev_mgr_get_tsf_adjust);

QDF_STATUS ucfg_wlan_vdev_mgr_set_param(
				struct wlan_objmgr_vdev *vdev,
				enum wlan_mlme_cfg_id param_id,
				struct wlan_vdev_mgr_cfg mlme_cfg)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_objmgr_vdev_get_comp_private_obj(
							vdev,
							WLAN_UMAC_COMP_MLME);

	if (!vdev_mlme) {
		QDF_ASSERT(0);
		return QDF_STATUS_E_FAILURE;
	}

	return wlan_util_vdev_mlme_set_param(vdev_mlme, param_id, mlme_cfg);
}

qdf_export_symbol(ucfg_wlan_vdev_mgr_set_param);

void ucfg_wlan_vdev_mgr_get_param(
				struct wlan_objmgr_vdev *vdev,
				enum wlan_mlme_cfg_id param_id,
				uint32_t *value)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_objmgr_vdev_get_comp_private_obj(
							vdev,
							WLAN_UMAC_COMP_MLME);

	if (!vdev_mlme) {
		QDF_ASSERT(0);
		return;
	}

	wlan_util_vdev_mlme_get_param(vdev_mlme, param_id, value);
}

qdf_export_symbol(ucfg_wlan_vdev_mgr_get_param);
