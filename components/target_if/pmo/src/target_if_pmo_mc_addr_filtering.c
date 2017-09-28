/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: target_if_pmo_mc_addr_filtering.c
 *
 * Target interface file for pmo component to
 * send mc address filtering offload related cmd and process event.
 */


#include "target_if.h"
#include "target_if_pmo.h"
#include "wmi_unified_api.h"

QDF_STATUS target_if_pmo_set_mc_filter_req(
		struct wlan_objmgr_vdev *vdev,
		struct qdf_mac_addr multicast_addr)
{
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	if (!vdev) {
		target_if_err("vdev ptr passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	vdev_id = wlan_vdev_get_id(vdev);
	if (!psoc) {
		target_if_err("psoc handle is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_add_clear_mcbc_filter_cmd(
			GET_WMI_HDL_FROM_PSOC(psoc),
			vdev_id,
			multicast_addr, false);
	if (status)
		target_if_err("Failed to send add/clear mcbc filter cmd");

	return status;
}

QDF_STATUS target_if_pmo_clear_mc_filter_req(
		struct wlan_objmgr_vdev *vdev,
		struct qdf_mac_addr multicast_addr)
{
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	if (!vdev) {
		target_if_err("vdev ptr passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	vdev_id = wlan_vdev_get_id(vdev);
	if (!psoc) {
		target_if_err("psoc handle is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = wmi_unified_add_clear_mcbc_filter_cmd(
			GET_WMI_HDL_FROM_PSOC(psoc),
			vdev_id,
			multicast_addr, true);
	if (status)
		target_if_err("Failed to send add/clear mcbc filter cmd");

	return status;

}

bool target_if_pmo_get_multiple_mc_filter_support(
		struct wlan_objmgr_psoc *psoc)
{
	return WMI_SERVICE_IS_ENABLED(psoc->service_param.service_bitmap,
				      WMI_SERVICE_MULTIPLE_MCAST_FILTER_SET);
}

QDF_STATUS target_if_pmo_set_multiple_mc_filter_req(
		struct wlan_objmgr_vdev *vdev,
		struct pmo_mc_addr_list *mc_list)
{
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	struct pmo_mcast_filter_params filter_params;
	QDF_STATUS status;

	if (!vdev) {
		target_if_err("vdev ptr passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	vdev_id = wlan_vdev_get_id(vdev);
	if (!psoc) {
		target_if_err("psoc handle is NULL");
		return QDF_STATUS_E_INVAL;
	}

	filter_params.multicast_addr_cnt = mc_list->mc_cnt;
	qdf_mem_copy(filter_params.multicast_addr,
		     mc_list->mc_addr,
		     mc_list->mc_cnt * ATH_MAC_LEN);
	/* add one/multiple mc list */
	filter_params.action = 1;

	status = wmi_unified_multiple_add_clear_mcbc_filter_cmd(
			GET_WMI_HDL_FROM_PSOC(psoc),
			vdev_id,
			&filter_params);
	if (status)
		target_if_err("Failed to send add/clear mcbc filter cmd");

	return status;
}

QDF_STATUS target_if_pmo_clear_multiple_mc_filter_req(
		struct wlan_objmgr_vdev *vdev,
		struct pmo_mc_addr_list *mc_list)
{
	uint8_t vdev_id;
	struct wlan_objmgr_psoc *psoc;
	struct pmo_mcast_filter_params filter_params;
	QDF_STATUS status;

	if (!vdev) {
		target_if_err("vdev ptr passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	vdev_id = wlan_vdev_get_id(vdev);
	if (!psoc) {
		target_if_err("psoc handle is NULL");
		return QDF_STATUS_E_INVAL;
	}

	filter_params.multicast_addr_cnt = mc_list->mc_cnt;
	qdf_mem_copy(filter_params.multicast_addr,
		     mc_list->mc_addr,
		     mc_list->mc_cnt * ATH_MAC_LEN);
	/* delete one/multiple mc list */
	filter_params.action = 0;

	status = wmi_unified_multiple_add_clear_mcbc_filter_cmd(
			GET_WMI_HDL_FROM_PSOC(psoc),
			vdev_id,
			&filter_params);
	if (status)
		target_if_err("Failed to send add/clear mcbc filter cmd");

	return status;
}


