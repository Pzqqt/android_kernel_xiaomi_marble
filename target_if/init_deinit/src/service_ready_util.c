/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: service_ready_util.c
 *
 * Public APIs implementation source file for accessing (ext)service ready
 * data from psoc object
 */
#include "service_ready_util.h"
#include <wlan_reg_ucfg_api.h>
#include <target_type.h>
#include <qdf_module.h>

QDF_STATUS init_deinit_chainmask_table_alloc(
		struct wlan_psoc_host_service_ext_param *ser_ext_par)
{
	int i;
	uint32_t alloc_size;
	QDF_STATUS status;

	if (ser_ext_par->num_chainmask_tables > 0) {
		status = QDF_STATUS_SUCCESS;
		for (i = 0; i < ser_ext_par->num_chainmask_tables; i++) {
			alloc_size =
			(sizeof(struct wlan_psoc_host_chainmask_capabilities) *
			ser_ext_par->chainmask_table[i].num_valid_chainmasks);

			ser_ext_par->chainmask_table[i].cap_list =
				qdf_mem_alloc_outline(NULL, alloc_size);
			if (!ser_ext_par->chainmask_table[i].cap_list) {
				init_deinit_chainmask_table_free(ser_ext_par);
				status = QDF_STATUS_E_NOMEM;
				break;
			}
		}
	} else {
		status = QDF_STATUS_E_NOSUPPORT;
	}

	return status;
}

qdf_export_symbol(init_deinit_chainmask_table_alloc);

QDF_STATUS init_deinit_chainmask_table_free(
		struct wlan_psoc_host_service_ext_param *ser_ext_par)
{
	struct wlan_psoc_host_chainmask_table *table;
	int i;

	for (i = 0; i < ser_ext_par->num_chainmask_tables; i++) {
		table =  &(ser_ext_par->chainmask_table[i]);
		if (table->cap_list) {
			qdf_mem_free(table->cap_list);
			table->cap_list = NULL;
		}
	}

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(init_deinit_chainmask_table_free);

int init_deinit_populate_service_bitmap(void *wmi_handle, uint8_t *event,
				      uint32_t *service_bitmap)
{
	QDF_STATUS status;

	status = wmi_save_service_bitmap(wmi_handle, event, service_bitmap);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to parse service bitmap");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

int init_deinit_populate_fw_version_cmd(void *wmi_handle, uint8_t *event)
{
	QDF_STATUS status;

	status = wmi_unified_save_fw_version_cmd(wmi_handle, event);
	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("failed to save fw version");

	return 0;
}

int init_deinit_populate_target_cap(void *wmi_handle, uint8_t *event,
			       struct wlan_psoc_target_capability_info *cap)
{
	QDF_STATUS status;

	status = wmi_get_target_cap_from_service_ready(wmi_handle, event, cap);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to parse target cap");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

int init_deinit_populate_service_ready_ext_param(void *handle, uint8_t *evt,
			struct wlan_psoc_host_service_ext_param *param)
{
	QDF_STATUS status;

	status = wmi_extract_service_ready_ext(handle, evt, param);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to parse wmi service ready ext param");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

int init_deinit_populate_chainmask_tables(void *handle, uint8_t *evt,
		struct wlan_psoc_host_chainmask_table *param)
{
	QDF_STATUS status;

	status = wmi_extract_chainmask_tables(handle, evt, param);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to parse wmi service ready ext param");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

int init_deinit_populate_mac_phy_capability(void *handle, uint8_t *evt,
	struct wlan_psoc_host_hw_mode_caps *hw_cap, struct tgt_info *info)
{
	QDF_STATUS status;
	uint32_t hw_mode_id;
	uint32_t phy_bit_map;
	uint8_t mac_phy_id;

	hw_mode_id = hw_cap->hw_mode_id;
	phy_bit_map = hw_cap->phy_id_map;
	target_if_info("hw_mode_id %d phy_bit_map 0x%x",
		       hw_mode_id, phy_bit_map);

	mac_phy_id = 0;
	while (phy_bit_map) {
		if (info->total_mac_phy_cnt >= PSOC_MAX_MAC_PHY_CAP) {
			target_if_err("total mac phy exceeds max limit %d",
				      info->total_mac_phy_cnt);
			return -EINVAL;
		}

		status = wmi_extract_mac_phy_cap_service_ready_ext(handle,
				evt, hw_mode_id, mac_phy_id,
				&(info->mac_phy_cap[info->total_mac_phy_cnt]));
		if (QDF_IS_STATUS_ERROR(status)) {
			target_if_err("failed to parse mac phy capability");
			return qdf_status_to_os_return(status);
		}
		info->mac_phy_cap[info->total_mac_phy_cnt].hw_mode_config_type
					= hw_cap->hw_mode_config_type;
		info->total_mac_phy_cnt++;
		phy_bit_map &= (phy_bit_map - 1);
		mac_phy_id++;
	}
	target_if_info("total_mac_phy_cnt %d", info->total_mac_phy_cnt);

	return 0;
}

static int get_hw_mode(void *handle, uint8_t *evt, uint8_t hw_idx,
			struct wlan_psoc_host_hw_mode_caps *cap)
{
	QDF_STATUS status;

	status = wmi_extract_hw_mode_cap_service_ready_ext(handle, evt,
					hw_idx, cap);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to parse hw mode capability");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

int init_deinit_populate_hw_mode_capability(void *wmi_handle,
		uint8_t *event, struct target_psoc_info *tgt_hdl)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t hw_idx;
	uint32_t num_hw_modes;
	struct wlan_psoc_host_hw_mode_caps hw_mode_caps[PSOC_MAX_HW_MODE];
	uint32_t preferred_mode;
	struct tgt_info *info;

	info = &tgt_hdl->info;
	num_hw_modes = info->service_ext_param.num_hw_modes;
	if (num_hw_modes > PSOC_MAX_HW_MODE) {
		target_if_err("invalid num_hw_modes %d", num_hw_modes);
		return -EINVAL;
	}
	target_if_info("num_hw_modes %d", num_hw_modes);

	qdf_mem_zero(&hw_mode_caps, sizeof(hw_mode_caps));

	preferred_mode = target_psoc_get_preferred_hw_mode(tgt_hdl);
	for (hw_idx = 0; hw_idx < num_hw_modes; hw_idx++) {
		status = get_hw_mode(wmi_handle, event, hw_idx,
						&hw_mode_caps[hw_idx]);
		if (status)
			goto return_exit;

		if ((preferred_mode != WMI_HOST_HW_MODE_MAX) &&
		    (hw_mode_caps[hw_idx].hw_mode_id != preferred_mode))
			continue;

		status = init_deinit_populate_mac_phy_capability(wmi_handle,
				event, &hw_mode_caps[hw_idx], info);
		if (status)
			goto return_exit;

		if ((preferred_mode != WMI_HOST_HW_MODE_MAX) &&
		    (hw_mode_caps[hw_idx].hw_mode_id == preferred_mode)) {
			info->num_radios = info->total_mac_phy_cnt;
			target_if_info("num radios is %d\n", info->num_radios);
		}
	}

return_exit:
	return qdf_status_to_os_return(status);
}

int init_deinit_populate_dbr_ring_cap(struct wlan_objmgr_psoc *psoc,
			void *handle, uint8_t *event, struct tgt_info *info)

{
	uint8_t cap_idx;
	uint32_t num_dbr_ring_caps;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	num_dbr_ring_caps = info->service_ext_param.num_dbr_ring_caps;

	target_if_info("Num DMA Capabilities = %d", num_dbr_ring_caps);

	if (!num_dbr_ring_caps)
		return 0;

	info->dbr_ring_cap = qdf_mem_malloc(
				sizeof(struct wlan_psoc_host_dbr_ring_caps) *
				num_dbr_ring_caps);

	if (!info->dbr_ring_cap) {
		target_if_err("Mem alloc for DMA cap failed");
		return -EINVAL;
	}

	for (cap_idx = 0; cap_idx < num_dbr_ring_caps; cap_idx++) {
		status = wmi_extract_dbr_ring_cap_service_ready_ext(handle,
				event, cap_idx,
				&(info->dbr_ring_cap[cap_idx]));
		if (QDF_IS_STATUS_ERROR(status)) {
			target_if_err("Extraction of DMA cap failed");
			goto free_and_return;
		}
	}

	return 0;

free_and_return:
	qdf_mem_free(info->dbr_ring_cap);
	info->dbr_ring_cap = NULL;

	return qdf_status_to_os_return(status);
}

int init_deinit_populate_phy_reg_cap(struct wlan_objmgr_psoc *psoc,
				void *handle, uint8_t *event,
				struct tgt_info *info, bool service_ready)
{
	uint8_t reg_idx;
	uint32_t num_phy_reg_cap;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_psoc_hal_reg_capability cap;
	struct wlan_psoc_host_hal_reg_capabilities_ext
				reg_cap[PSOC_MAX_PHY_REG_CAP];

	if (service_ready) {
		status = wmi_extract_hal_reg_cap(handle, event, &cap);
		if (QDF_IS_STATUS_ERROR(status)) {
			target_if_err("failed to parse hal reg cap");
			return qdf_status_to_os_return(status);
		}
		info->service_ext_param.num_phy = 1;
		num_phy_reg_cap = 1;
		reg_cap[0].phy_id = 0;
		qdf_mem_copy(&(reg_cap[0].eeprom_reg_domain), &cap,
			     sizeof(struct wlan_psoc_hal_reg_capability));
		target_if_info("FW wireless modes 0x%x",
			       reg_cap[0].wireless_modes);
	} else {
		num_phy_reg_cap = info->service_ext_param.num_phy;
		if (num_phy_reg_cap > PSOC_MAX_PHY_REG_CAP) {
			target_if_err("Invalid num_phy_reg_cap %d",
				      num_phy_reg_cap);
			return -EINVAL;
		}
		target_if_info("num_phy_reg_cap %d", num_phy_reg_cap);

		for (reg_idx = 0; reg_idx < num_phy_reg_cap; reg_idx++) {
			status = wmi_extract_reg_cap_service_ready_ext(handle,
					event, reg_idx, &(reg_cap[reg_idx]));
			if (QDF_IS_STATUS_ERROR(status)) {
				target_if_err("failed to parse reg cap");
				return qdf_status_to_os_return(status);
			}
		}
	}

	status = ucfg_reg_set_hal_reg_cap(psoc, reg_cap, num_phy_reg_cap);

	return qdf_status_to_os_return(status);
}

static bool init_deinit_regdmn_160mhz_support(
		struct wlan_psoc_host_hal_reg_capabilities_ext *hal_cap)
{
	return ((hal_cap->wireless_modes &
		WMI_HOST_REGDMN_MODE_11AC_VHT160) != 0);
}

static bool init_deinit_regdmn_80p80mhz_support(
		struct wlan_psoc_host_hal_reg_capabilities_ext *hal_cap)
{
	return ((hal_cap->wireless_modes &
			WMI_HOST_REGDMN_MODE_11AC_VHT80_80) != 0);
}

static bool init_deinit_vht_160mhz_is_supported(uint32_t vhtcap)
{
	return ((vhtcap & WLAN_VHTCAP_SUP_CHAN_WIDTH_160) != 0);
}

static bool init_deinit_vht_80p80mhz_is_supported(uint32_t vhtcap)
{
	return ((vhtcap & WLAN_VHTCAP_SUP_CHAN_WIDTH_80_160) != 0);
}

static bool init_deinit_vht_160mhz_shortgi_is_supported(uint32_t vhtcap)
{
	return ((vhtcap & WLAN_VHTCAP_SHORTGI_160) != 0);
}

QDF_STATUS init_deinit_validate_160_80p80_fw_caps(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_hdl)
{
	bool wireless_mode_160mhz = false;
	bool wireless_mode_80p80mhz = false;
	bool vhtcap_160mhz = false;
	bool vhtcap_80p80_160mhz = false;
	bool vhtcap_160mhz_sgi = false;
	bool valid = false;
	struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap;
	void *wmi_handle;

	if (!tgt_hdl) {
		target_if_err(
		"target_psoc_info is null in validate 160n80p80 cap check");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_psoc_get_wmi_hdl(tgt_hdl);

	if ((tgt_hdl->info.target_type == TARGET_TYPE_QCA8074) ||
	    (tgt_hdl->info.target_type == TARGET_TYPE_QCA6290)) {
		/**
		 * Return true for now. This is not available in
		 * qca8074 fw yet
		 */
		return QDF_STATUS_SUCCESS;
	}

	reg_cap = ucfg_reg_get_hal_reg_cap(psoc);
	if (reg_cap == NULL) {
		target_if_err("reg cap is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* NOTE: Host driver gets vht capability and supported channel
	 * width / channel frequency range from FW/HALPHY and obeys it.
	 * Host driver is unaware of any physical filters or any other
	 * hardware factors that can impact these capabilities.
	 * These need to be correctly determined by firmware.
	 */

	/*This table lists all valid and invalid combinations
	 * WMODE160 WMODE80_80  VHTCAP_160 VHTCAP_80+80_160  IsCombinationvalid?
	 *      0         0           0              0                 YES
	 *      0         0           0              1                 NO
	 *      0         0           1              0                 NO
	 *      0         0           1              1                 NO
	 *      0         1           0              0                 NO
	 *      0         1           0              1                 NO
	 *      0         1           1              0                 NO
	 *      0         1           1              1                 NO
	 *      1         0           0              0                 NO
	 *      1         0           0              1                 NO
	 *      1         0           1              0                 YES
	 *      1         0           1              1                 NO
	 *      1         1           0              0                 NO
	 *      1         1           0              1                 YES
	 *      1         1           1              0                 NO
	 *      1         1           1              1                 NO
	 */

	/* NOTE: Last row in above table is invalid because value corresponding
	 * to both VHTCAP_160 and VHTCAP_80+80_160 being set is reserved as per
	 * 802.11ac. Only one of them can be set at a time.
	 */

	wireless_mode_160mhz = init_deinit_regdmn_160mhz_support(reg_cap);
	wireless_mode_80p80mhz = init_deinit_regdmn_80p80mhz_support(reg_cap);
	vhtcap_160mhz = init_deinit_vht_160mhz_is_supported(
				tgt_hdl->info.target_caps.vht_cap_info);
	vhtcap_80p80_160mhz = init_deinit_vht_80p80mhz_is_supported(
				tgt_hdl->info.target_caps.vht_cap_info);
	vhtcap_160mhz_sgi = init_deinit_vht_160mhz_shortgi_is_supported(
				tgt_hdl->info.target_caps.vht_cap_info);

	if (!(wireless_mode_160mhz || wireless_mode_80p80mhz ||
	      vhtcap_160mhz || vhtcap_80p80_160mhz)) {
		valid = QDF_STATUS_SUCCESS;
	} else if (wireless_mode_160mhz && !wireless_mode_80p80mhz &&
		   vhtcap_160mhz && !vhtcap_80p80_160mhz) {
		valid = QDF_STATUS_SUCCESS;
	} else if (wireless_mode_160mhz && wireless_mode_80p80mhz &&
		   !vhtcap_160mhz && vhtcap_160mhz_sgi) {
		valid = QDF_STATUS_SUCCESS;
	}

	if (valid == QDF_STATUS_SUCCESS) {
		/*
		 * Ensure short GI for 160 MHz is enabled
		 * only if 160/80+80 is supported.
		 */
		if (vhtcap_160mhz_sgi &&
		    !(vhtcap_160mhz || vhtcap_80p80_160mhz)) {
			valid = QDF_STATUS_E_FAILURE;
		}
	}

	/* Invalid config specified by FW */
	if (valid != QDF_STATUS_SUCCESS) {
		target_if_err("Invalid 160/80+80 MHz config specified by FW. Take care of it first");
		target_if_err("wireless_mode_160mhz: %d, wireless_mode_80p80mhz: %d",
			      wireless_mode_160mhz, wireless_mode_80p80mhz);
		target_if_err("vhtcap_160mhz: %d, vhtcap_80p80_160mhz: %d,vhtcap_160mhz_sgi: %d",
			      vhtcap_160mhz, vhtcap_80p80_160mhz,
			      vhtcap_160mhz_sgi);
	}
	return valid;
}

void init_deinit_chainmask_config(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_hdl)
{
	tgt_hdl->info.wlan_res_cfg.tx_chain_mask =
		((1 << tgt_hdl->info.target_caps.num_rf_chains) - 1);
	tgt_hdl->info.wlan_res_cfg.rx_chain_mask =
		((1 << tgt_hdl->info.target_caps.num_rf_chains) - 1);
}

QDF_STATUS init_deinit_is_service_ext_msg(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_hdl)
{
	void *wmi_handle;

	if (!tgt_hdl) {
		target_if_err(
			"psoc target_psoc_info is null in service ext msg");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_psoc_get_wmi_hdl(tgt_hdl);

	if (wmi_service_enabled(wmi_handle, wmi_service_ext_msg))
		return QDF_STATUS_SUCCESS;
	else
		return QDF_STATUS_E_FAILURE;
}

bool init_deinit_is_preferred_hw_mode_supported(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_hdl)
{
	uint16_t i;
	struct tgt_info *info;

	if (!tgt_hdl) {
		target_if_err(
			"psoc target_psoc_info is null in service ext msg");
		return FALSE;
	}

	info = &tgt_hdl->info;

	if (info->preferred_hw_mode == WMI_HOST_HW_MODE_MAX)
		return TRUE;

	for (i = 0; i < target_psoc_get_total_mac_phy_cnt(tgt_hdl); i++) {
		if (info->mac_phy_cap[i].hw_mode_id == info->preferred_hw_mode)
			return TRUE;
	}

	return FALSE;
}

void init_deinit_wakeup_host_wait(
		 struct wlan_objmgr_psoc *psoc,
		 struct target_psoc_info *tgt_hdl)
{
	if (!tgt_hdl) {
		target_if_err("psoc target_psoc_info is null in target ready");
		return;
	}
	qdf_wake_up(&tgt_hdl->info.event_queue);
}
