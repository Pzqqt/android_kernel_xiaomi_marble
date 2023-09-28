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
 * DOC: init_deinit_lmac.c
 *
 * APIs to get/set target_if params
 */
#include <qdf_status.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <target_if.h>
#include <target_type.h>
#include <init_deinit_lmac.h>
#include <qdf_module.h>

struct wlan_psoc_target_capability_info *lmac_get_target_cap(
				struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return NULL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return NULL;
	}

	return target_psoc_get_target_caps(tgt_hdl);
}

target_resource_config *lmac_get_tgt_res_cfg(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return NULL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return NULL;
	}

	return target_psoc_get_wlan_res_cfg(tgt_hdl);
}

int32_t lmac_get_pdev_idx(struct wlan_objmgr_pdev *pdev)
{
	if (!pdev) {
		target_if_err("pdev is null");
		return 0xffffffff;
	}

	return wlan_objmgr_pdev_get_pdev_id(pdev);
}

uint32_t lmac_get_tgt_type(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return 0;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return 0;
	}

	return target_psoc_get_target_type(tgt_hdl);
}
qdf_export_symbol(lmac_get_tgt_type);

QDF_STATUS lmac_get_pdev_target_type(struct wlan_objmgr_pdev *pdev,
					uint32_t *target_type)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		target_if_err("psoc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	*target_type = lmac_get_tgt_type(psoc);
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(lmac_get_pdev_target_type);

uint32_t lmac_get_tgt_version(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return -EINVAL;
	}

	return target_psoc_get_target_ver(tgt_hdl);
}
qdf_export_symbol(lmac_get_tgt_version);

uint32_t lmac_get_tgt_revision(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return -EINVAL;
	}

	return target_psoc_get_target_rev(tgt_hdl);
}
qdf_export_symbol(lmac_get_tgt_revision);

bool lmac_is_target_ar900b(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;
	uint32_t target_type;

	if (!psoc) {
		target_if_err("psoc is null\n");
		return false;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return false;
	}
	target_type = tgt_hdl->info.target_type;

	switch (target_type) {
	case TARGET_TYPE_AR900B:
	case TARGET_TYPE_QCA9984:
	case TARGET_TYPE_IPQ4019:
	case TARGET_TYPE_QCA9888:
		return true;
	default:
		return false;
	}
	return false;
}
qdf_export_symbol(lmac_is_target_ar900b);

struct wmi_unified *lmac_get_wmi_hdl(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return NULL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return NULL;
	}

	return target_psoc_get_wmi_hdl(tgt_hdl);
}
qdf_export_symbol(lmac_get_wmi_hdl);

wmi_unified_t lmac_get_wmi_unified_hdl(struct wlan_objmgr_psoc *psoc)
{
	return (wmi_unified_t)lmac_get_wmi_hdl(psoc);
}
qdf_export_symbol(lmac_get_wmi_unified_hdl);

HTC_HANDLE lmac_get_htc_hdl(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return NULL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return NULL;
	}

	return target_psoc_get_htc_hdl(tgt_hdl);
}
qdf_export_symbol(lmac_get_htc_hdl);

void lmac_set_htc_hdl(struct wlan_objmgr_psoc *psoc,
		      HTC_HANDLE htc_hdl)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return;
	}
	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return;
	}

	target_psoc_set_htc_hdl(tgt_hdl, htc_hdl);
}

struct hif_opaque_softc *lmac_get_hif_hdl(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return NULL;
	}

	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return NULL;
	}

	return target_psoc_get_hif_hdl(tgt_hdl);
}
qdf_export_symbol(lmac_get_hif_hdl);

struct hif_opaque_softc *lmac_get_ol_hif_hdl(struct wlan_objmgr_psoc *psoc)
{
	return (struct hif_opaque_softc *)lmac_get_hif_hdl(psoc);
}
qdf_export_symbol(lmac_get_ol_hif_hdl);

struct wmi_unified *lmac_get_pdev_wmi_handle(
		struct wlan_objmgr_pdev *pdev)
{
	struct target_pdev_info *tgt_hdl;

	if (!pdev) {
		target_if_err("pdev is null");
		return NULL;
	}

	tgt_hdl = wlan_pdev_get_tgt_if_handle(pdev);
	if (!tgt_hdl) {
		target_if_err("target_pdev_info is null");
		return NULL;
	}

	return target_pdev_get_wmi_handle(tgt_hdl);
}
qdf_export_symbol(lmac_get_pdev_wmi_handle);

wmi_unified_t
lmac_get_pdev_wmi_unified_handle(struct wlan_objmgr_pdev *pdev)
{
	return (wmi_unified_t)lmac_get_pdev_wmi_handle(pdev);
}

uint32_t lmac_get_num_radios(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return 0;
	}
	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return 0;
	}

	return target_psoc_get_num_radios(tgt_hdl);
}
qdf_export_symbol(lmac_get_num_radios);

void *lmac_get_psoc_feature_ptr(struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return NULL;
	}
	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return NULL;
	}

	return target_psoc_get_feature_ptr(tgt_hdl);
}
qdf_export_symbol(lmac_get_psoc_feature_ptr);

void *lmac_get_pdev_feature_ptr(struct wlan_objmgr_pdev *pdev)
{
	struct target_pdev_info *tgt_hdl;

	if (!pdev) {
		target_if_err("pdev is null");
		return NULL;
	}
	tgt_hdl = wlan_pdev_get_tgt_if_handle(pdev);
	if (!tgt_hdl) {
		target_if_err("target_pdev_info is null");
		return NULL;
	}

	return target_pdev_get_feature_ptr(tgt_hdl);
}
qdf_export_symbol(lmac_get_pdev_feature_ptr);

enum wmi_host_hw_mode_config_type lmac_get_preferred_hw_mode(
				struct wlan_objmgr_psoc *psoc)
{
	struct target_psoc_info *tgt_hdl;

	if (!psoc) {
		target_if_err("psoc is null");
		return WMI_HOST_HW_MODE_MAX;
	}
	tgt_hdl = wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null");
		return WMI_HOST_HW_MODE_MAX;
	}

	return target_psoc_get_preferred_hw_mode(tgt_hdl);
}

qdf_export_symbol(lmac_get_preferred_hw_mode);
