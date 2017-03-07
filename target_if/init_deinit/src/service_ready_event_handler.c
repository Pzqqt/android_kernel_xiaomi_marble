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
 * DOC: wmi_unified_event_handler.c
 *
 * WMI common event handler implementation source file
 */
#include "service_ready_event_handler.h"
#include "wlan_objmgr_psoc_service_ready_api.h"

static int populate_service_bitmap(void *wmi_handle, uint8_t *event,
				      uint32_t *service_bitmap)
{
	QDF_STATUS status;

	status = wmi_save_service_bitmap(wmi_handle, event, service_bitmap);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("failed to parse service bitmap");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

static int populate_target_cap(void *wmi_handle, uint8_t *event,
			       struct wlan_psoc_target_capability_info *cap)
{
	QDF_STATUS status;

	status = wmi_get_target_cap_from_service_ready(wmi_handle, event, cap);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("failed to parse target cap");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

static int populate_hal_reg_cap(void *wmi_handle, uint8_t *event,
				struct wlan_psoc_hal_reg_capability *cap)
{
	QDF_STATUS status;

	status = wmi_extract_hal_reg_cap(wmi_handle, event, cap);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("failed to parse hal reg cap");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

int init_deinit_service_ready_event_handler(ol_scn_t scn_handle,
					    uint8_t *event,
					    uint32_t data_len)
{
	int err_code;
	struct wlan_objmgr_psoc_service_ready_param *service_param;
	struct wlan_objmgr_psoc *psoc;
	wmi_legacy_service_ready_callback legacy_callback;
	void *wmi_handle;

	if (!scn_handle) {
		WMI_LOGE("scn handle NULL in service ready handler");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn_handle);
	if (!psoc) {
		WMI_LOGE("psoc is null in service ready handler");
		return -EINVAL;
	}

	wmi_handle = psoc->tgt_if_handle;

	service_param = qdf_mem_malloc(sizeof(*service_param));
	if (!service_param) {
		WMI_LOGE("memory alloc failed for psoc service ready");
		return -ENOMEM;
	}

	err_code = populate_service_bitmap(wmi_handle, event,
			service_param->service_bitmap);
	if (err_code)
		goto free_param_and_exit;

	err_code = populate_target_cap(wmi_handle, event,
				   &(service_param->target_caps));
	if (err_code)
		goto free_param_and_exit;

	err_code = populate_hal_reg_cap(wmi_handle, event,
				    &(service_param->hal_reg_cap));
	if (err_code)
		goto free_param_and_exit;

	legacy_callback = target_if_get_psoc_legacy_service_ready_cb();
	err_code = legacy_callback(wmi_service_ready_event_id,
				  scn_handle, event, data_len);

	wlan_objmgr_populate_service_ready_data(psoc, service_param);

free_param_and_exit:
	qdf_mem_free(service_param);

	return err_code;
}

static int populate_service_ready_ext_param(void *handle, uint8_t *evt,
			struct wlan_psoc_host_service_ext_param *param)
{
	QDF_STATUS status;

	status = wmi_extract_service_ready_ext(handle, evt, param);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("failed to parse wmi service ready ext param");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

static int populate_chainmaks_tables(void *handle, uint8_t *evt,
		struct wlan_psoc_host_chainmask_table *param)
{
	QDF_STATUS status;

	status = wmi_extract_chainmask_tables(handle, evt, param);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("failed to parse wmi service ready ext param");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

static int populate_mac_phy_capability(void *handle, uint8_t *evt,
	struct wlan_psoc_host_hw_mode_caps *hw_cap, uint8_t *total_mac_phy,
	struct wlan_objmgr_psoc_ext_service_ready_param *service_param)
{
	QDF_STATUS status;
	uint32_t hw_mode_id;
	uint32_t phy_bit_map;
	uint8_t mac_phy_id;

	hw_mode_id = hw_cap->hw_mode_id;
	phy_bit_map = hw_cap->phy_id_map;
	WMI_LOGE("hw_mode_id %d phy_bit_map 0x%x", hw_mode_id, phy_bit_map);

	mac_phy_id = 0;
	while (phy_bit_map) {
		status = wmi_extract_mac_phy_cap_service_ready_ext(handle,
				evt, hw_mode_id, mac_phy_id,
				&(service_param->mac_phy_cap[*total_mac_phy]));
		if (QDF_IS_STATUS_ERROR(status)) {
			WMI_LOGE("failed to parse mac phy capability");
			return qdf_status_to_os_return(status);
		}
		(*total_mac_phy)++;
		if (*total_mac_phy > PSOC_MAX_MAC_PHY_CAP) {
			WMI_LOGE("total mac phy exceeds max limit %d",
				*total_mac_phy);
			return -EINVAL;
		}
		phy_bit_map &= (phy_bit_map - 1);
		mac_phy_id++;
	}
	WMI_LOGE("total_mac_phy %d", *total_mac_phy);

	return 0;
}

static int get_hw_mode(void *handle, uint8_t *evt, uint8_t i,
			struct wlan_psoc_host_hw_mode_caps *cap)
{
	QDF_STATUS status;

	status = wmi_extract_hw_mode_cap_service_ready_ext(handle, evt, i, cap);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMI_LOGE("failed to parse hw mode capability");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

static int populate_hw_mode_capability(void *wmi_handle,
		uint8_t *event, uint8_t *total_mac_phy,
		struct wlan_objmgr_psoc_ext_service_ready_param *service_param)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t hw_idx;
	uint32_t num_hw_modes;

	num_hw_modes = service_param->service_ext_param.num_hw_modes;
	if (num_hw_modes > PSOC_MAX_HW_MODE) {
		WMI_LOGE("invalid num_hw_modes %d", num_hw_modes);
		return -EINVAL;
	}
	WMI_LOGE("num_hw_modes %d", num_hw_modes);

	for (hw_idx = 0; hw_idx < num_hw_modes; hw_idx++) {
		status = get_hw_mode(wmi_handle, event, hw_idx,
				      &service_param->hw_mode_caps[hw_idx]);
		if (status)
			goto return_exit;

		status = populate_mac_phy_capability(wmi_handle, event,
					&service_param->hw_mode_caps[hw_idx],
					total_mac_phy, service_param);
		if (status)
			goto return_exit;
	}

return_exit:
	return qdf_status_to_os_return(status);
}

static int populate_phy_reg_capability(void *handle, uint8_t *event,
		struct wlan_objmgr_psoc_ext_service_ready_param *service_param)
{
	uint8_t reg_idx;
	uint32_t num_phy_reg_cap;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	num_phy_reg_cap = service_param->service_ext_param.num_phy;
	if (num_phy_reg_cap > PSOC_MAX_PHY_REG_CAP) {
		WMI_LOGE("Invalid num_phy_reg_cap %d", num_phy_reg_cap);
		return -EINVAL;
	}
	WMI_LOGE("num_phy_reg_cap %d", num_phy_reg_cap);

	for (reg_idx = 0; reg_idx < num_phy_reg_cap; reg_idx++) {
		status = wmi_extract_reg_cap_service_ready_ext(handle, event,
				reg_idx, &(service_param->reg_cap[reg_idx]));
		if (QDF_IS_STATUS_ERROR(status)) {
			WMI_LOGE("failed to parse reg cap");
			return qdf_status_to_os_return(status);
		}
	}

	return qdf_status_to_os_return(status);
}

int init_deinit_service_ext_ready_event_handler(ol_scn_t scn_handle,
						uint8_t *event,
						uint32_t data_len)
{
	int err_code;
	struct wlan_objmgr_psoc_ext_service_ready_param *service_param;
	struct wlan_objmgr_psoc *psoc;
	wmi_legacy_service_ready_callback legacy_callback;
	void *wmi_handle;

	if (!scn_handle) {
		WMI_LOGE("scn handle NULL in service ready handler");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(scn_handle);
	if (!psoc) {
		WMI_LOGE("psoc is null in service ready handler");
		return -EINVAL;
	}

	wmi_handle = psoc->tgt_if_handle;

	service_param =
		qdf_mem_malloc(sizeof(*service_param));
	if (!service_param) {
		WMI_LOGE("ext_service_ready_param alloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	err_code = populate_service_ready_ext_param(wmi_handle,
				event, &(service_param->service_ext_param));
	if (err_code)
		goto free_param_and_exit;

	err_code =  populate_hw_mode_capability(wmi_handle,
					    event,
					    &psoc->total_mac_phy,
					    service_param);
	if (err_code)
		goto free_param_and_exit;

	err_code = populate_phy_reg_capability(wmi_handle,
					   event, service_param);
	if (err_code)
		goto free_param_and_exit;

	if (wlan_objmgr_ext_service_ready_chainmask_table_caplist_alloc(
				&(service_param->service_ext_param)) == QDF_STATUS_SUCCESS) {
		err_code = populate_chainmaks_tables(wmi_handle, event,
				&(service_param->service_ext_param.chainmask_table[0]));
		if (err_code)
			goto free_param_and_exit;
	}

	legacy_callback = target_if_get_psoc_legacy_service_ready_cb();
	err_code = legacy_callback(wmi_service_ready_ext_event_id,
				  scn_handle, event, data_len);

	wlan_objmgr_populate_ext_service_ready_data(psoc, service_param);

free_param_and_exit:
	qdf_mem_free(service_param);

	return err_code;
}
