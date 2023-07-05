/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (ser_ext_par->num_chainmask_tables == 0)
		return QDF_STATUS_E_NOSUPPORT;

	for (i = 0; i < ser_ext_par->num_chainmask_tables; i++) {
		if (ser_ext_par->chainmask_table[i].num_valid_chainmasks >
		    (UINT_MAX / sizeof(
		     struct wlan_psoc_host_chainmask_capabilities))) {
			target_if_err("invalid valid chanmask num %d",
				      ser_ext_par->chainmask_table[i].
				      num_valid_chainmasks);
			status = QDF_STATUS_E_FAILURE;
			break;
		}
		alloc_size =
			(sizeof(struct wlan_psoc_host_chainmask_capabilities) *
			 ser_ext_par->chainmask_table[i].num_valid_chainmasks);

		ser_ext_par->chainmask_table[i].cap_list =
			qdf_mem_malloc(alloc_size);
		if (!ser_ext_par->chainmask_table[i].cap_list) {
			init_deinit_chainmask_table_free(ser_ext_par);
			status = QDF_STATUS_E_NOMEM;
			break;
		}
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

int init_deinit_populate_service_bitmap(
		wmi_unified_t wmi_handle, uint8_t *event,
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

int init_deinit_populate_fw_version_cmd(wmi_unified_t wmi_handle,
					uint8_t *event)
{
	QDF_STATUS status;

	status = wmi_unified_save_fw_version_cmd(wmi_handle, event);
	if (QDF_IS_STATUS_ERROR(status))
		target_if_err("failed to save fw version");

	return 0;
}

int init_deinit_populate_target_cap(
		wmi_unified_t wmi_handle, uint8_t *event,
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

int init_deinit_populate_service_ready_ext_param(
		wmi_unified_t handle, uint8_t *evt,
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

int init_deinit_populate_service_ready_ext2_param(
		wmi_unified_t handle, uint8_t *evt,
		struct tgt_info *info)
{
	QDF_STATUS status;

	status = wmi_extract_service_ready_ext2(handle, evt,
						&info->service_ext2_param);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to parse wmi service ready ext param");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

int init_deinit_populate_chainmask_tables(
		wmi_unified_t handle, uint8_t *evt,
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

int init_deinit_populate_mac_phy_capability(
	wmi_unified_t handle, uint8_t *evt,
	struct wlan_psoc_host_hw_mode_caps *hw_cap, struct tgt_info *info)
{
	QDF_STATUS status;
	uint32_t hw_mode_id;
	uint32_t phy_bit_map;
	uint8_t mac_phy_id;

	hw_mode_id = hw_cap->hw_mode_id;
	phy_bit_map = hw_cap->phy_id_map;
	target_if_debug("hw_mode_id %d phy_bit_map 0x%x",
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
	target_if_debug("total_mac_phy_cnt %d", info->total_mac_phy_cnt);

	return 0;
}

static int get_hw_mode(wmi_unified_t handle, uint8_t *evt, uint8_t hw_idx,
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

static int get_sar_version(wmi_unified_t handle, uint8_t *evt,
			   struct wlan_psoc_host_service_ext_param *ext_param)
{
	QDF_STATUS status;

	status = wmi_extract_sar_cap_service_ready_ext(handle, evt, ext_param);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("failed to parse sar capability");
		return qdf_status_to_os_return(status);
	}

	return 0;
}

static bool new_hw_mode_preferred(uint32_t current_hw_mode,
				  uint32_t new_hw_mode)
{
	uint8_t hw_mode_id_precedence[WMI_HOST_HW_MODE_MAX + 1] = { 6, 2, 5,
								    4, 1, 3,
								    7, 0, 8};

	if (current_hw_mode > WMI_HOST_HW_MODE_MAX ||
	    new_hw_mode > WMI_HOST_HW_MODE_MAX)
		return false;

	/* Above precedence is defined by low to high, lower the value
	 * higher the precedence
	 */
	if (hw_mode_id_precedence[current_hw_mode] >
	    hw_mode_id_precedence[new_hw_mode])
		return true;

	return false;
}

/**
 * select_preferred_mode() - Select preferred hw mode based on current mode.
 * @tgt_hdl: target_psoc_info object
 * @hw_mode_caps: HW mode caps of new mode id that needs to checked for
 *                selection.
 * @current_mode: Current mode.
 *
 * API to select preferred hw mode based on the current config.
 * Based on host config for preferred mode, final mode selected as follows-
 * 1) If preferred_mode == WMI_HOST_HW_MODE_DETECT, Then select mode from FW
 *    supported modes such that it is a super set of all modes FW advertises.
 *    For e.g., If FW supports DBS(2 radio) and DBS_SBS(3 radio)- Choose DBS_SBS
 * 2) If preferred_mode == WMI_HOST_HW_MODE_MAX, Then do not select any mode
 *    from FW advertised modes. Host needs to maintain all modes supported in FW
 *    and can switch dynamically.
 * 3) Else, A valid preferred_mode is set, Hence check if this is part of FW
 *    supported modes. If it is found, then use it to bring up the device.
 *
 * Return: selected_mode based on the above criteria.
 */
static uint32_t
select_preferred_hw_mode(struct target_psoc_info *tgt_hdl,
			 struct wlan_psoc_host_hw_mode_caps *hw_mode_caps,
			 uint32_t current_mode)
{
	uint32_t preferred_mode, selected_mode = current_mode;
	struct tgt_info *info;

	info = &tgt_hdl->info;
	preferred_mode = target_psoc_get_preferred_hw_mode(tgt_hdl);
	if (preferred_mode == WMI_HOST_HW_MODE_DETECT) {
		uint32_t new_mode = hw_mode_caps->hw_mode_id;

		/* Choose hw_mode_id based on precedence */
		if (new_hw_mode_preferred(selected_mode, new_mode)) {
			selected_mode = new_mode;
			qdf_mem_copy(&info->hw_mode_cap, hw_mode_caps,
				     sizeof(info->hw_mode_cap));
		}
	} else if ((preferred_mode != WMI_HOST_HW_MODE_MAX) &&
		   (preferred_mode == hw_mode_caps->hw_mode_id)) {
		selected_mode = preferred_mode;
		qdf_mem_copy(&info->hw_mode_cap, hw_mode_caps,
			     sizeof(info->hw_mode_cap));
	}

	return selected_mode;
}

#ifdef FEATURE_NO_DBS_INTRABAND_MCC_SUPPORT
static void init_deinit_change_def_hw_mode(struct target_psoc_info *tgt_hdl,
					   struct wmi_unified *wmi_handle)
{
	struct tgt_info *info = &tgt_hdl->info;

	if ((info->hw_modes.num_modes == 1) &&
	    (info->hw_modes.hw_mode_ids[0] == WMI_HOST_HW_MODE_DBS) &&
	    !wmi_service_enabled(wmi_handle,
				 wmi_service_dual_band_simultaneous_support))
		target_psoc_set_preferred_hw_mode(tgt_hdl,
						  WMI_HOST_HW_MODE_DETECT);
}
#else
static void init_deinit_change_def_hw_mode(struct target_psoc_info *tgt_hdl,
					   struct wmi_unified *wmi_handle)
{
}
#endif

int init_deinit_populate_hw_mode_capability(
		wmi_unified_t wmi_handle, uint8_t *event,
		struct target_psoc_info *tgt_hdl)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t hw_idx;
	uint32_t num_hw_modes;
	struct wlan_psoc_host_hw_mode_caps hw_mode_caps[PSOC_MAX_HW_MODE];
	uint32_t preferred_mode, selected_mode = WMI_HOST_HW_MODE_MAX;
	struct tgt_info *info;

	info = &tgt_hdl->info;
	num_hw_modes = info->service_ext_param.num_hw_modes;
	if (num_hw_modes > PSOC_MAX_HW_MODE) {
		target_if_err("invalid num_hw_modes %d", num_hw_modes);
		return -EINVAL;
	}
	target_if_debug("num_hw_modes %d", num_hw_modes);

	qdf_mem_zero(&hw_mode_caps, sizeof(hw_mode_caps));
	info->hw_modes.num_modes = 0;
	info->hw_mode_cap.hw_mode_id = WMI_HOST_HW_MODE_MAX;

	for (hw_idx = 0; hw_idx < num_hw_modes; hw_idx++) {
		status = get_hw_mode(wmi_handle, event, hw_idx,
						&hw_mode_caps[hw_idx]);
		if (status)
			goto return_exit;

		if (hw_idx < WMI_HOST_HW_MODE_MAX) {
			info->hw_modes.hw_mode_ids[hw_idx] =
				hw_mode_caps[hw_idx].hw_mode_id;
			info->hw_modes.phy_bit_map[hw_idx] =
				hw_mode_caps[hw_idx].phy_id_map;
			info->hw_modes.num_modes++;
		}

		status = init_deinit_populate_mac_phy_capability(wmi_handle,
				event, &hw_mode_caps[hw_idx], info);
		if (status)
			goto return_exit;

		if (num_hw_modes == 1)
			init_deinit_change_def_hw_mode(tgt_hdl, wmi_handle);

		selected_mode = select_preferred_hw_mode(tgt_hdl,
							 &hw_mode_caps[hw_idx],
							 selected_mode);
	}

	preferred_mode = target_psoc_get_preferred_hw_mode(tgt_hdl);
	if (preferred_mode == WMI_HOST_HW_MODE_DETECT) {
		target_if_info("Preferred mode is not set, use mode id %d\n",
			       selected_mode);
		target_psoc_set_preferred_hw_mode(tgt_hdl, selected_mode);

		/* Change default DBS hw mode as per selected one */
		info->target_caps.default_dbs_hw_mode_index = selected_mode;
	}

	status = get_sar_version(wmi_handle, event, &info->service_ext_param);
	target_if_debug("sar version %d", info->service_ext_param.sar_version);

return_exit:
	return qdf_status_to_os_return(status);
}

int init_deinit_populate_dbr_ring_cap(struct wlan_objmgr_psoc *psoc,
				      wmi_unified_t handle, uint8_t *event,
				      struct tgt_info *info)

{
	uint8_t cap_idx;
	uint32_t num_dbr_ring_caps;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	num_dbr_ring_caps = info->service_ext_param.num_dbr_ring_caps;
	target_if_debug("Num DMA Capabilities = %d", num_dbr_ring_caps);

	if (!num_dbr_ring_caps)
		return 0;

	info->dbr_ring_cap = qdf_mem_malloc(
				sizeof(struct wlan_psoc_host_dbr_ring_caps) *
				num_dbr_ring_caps);

	if (!info->dbr_ring_cap)
		return -EINVAL;

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

int init_deinit_populate_dbr_ring_cap_ext2(struct wlan_objmgr_psoc *psoc,
					   wmi_unified_t handle, uint8_t *event,
					   struct tgt_info *info)

{
	uint8_t cap_idx;
	uint32_t num_dbr_ring_caps;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_psoc_host_dbr_ring_caps *param;

	/*
	 * If FW had already sent this info as part of EXT event,
	 * we need to discard the same and use the info from EXT2.
	 */
	if (info->service_ext_param.num_dbr_ring_caps) {
		target_if_debug("dbr_ring_caps already populated");
		info->service_ext_param.num_dbr_ring_caps = 0;
		qdf_mem_free(info->dbr_ring_cap);
		info->dbr_ring_cap = NULL;
	}

	num_dbr_ring_caps = info->service_ext2_param.num_dbr_ring_caps;
	target_if_debug("Num DMA Capabilities = %d", num_dbr_ring_caps);

	if (!num_dbr_ring_caps)
		return 0;

	info->dbr_ring_cap = qdf_mem_malloc(
				sizeof(struct wlan_psoc_host_dbr_ring_caps) *
				num_dbr_ring_caps);

	if (!info->dbr_ring_cap)
		return -EINVAL;

	for (cap_idx = 0; cap_idx < num_dbr_ring_caps; cap_idx++) {
		param = &info->dbr_ring_cap[cap_idx];
		status = wmi_extract_dbr_ring_cap_service_ready_ext2(handle,
								     event,
								     cap_idx,
								     param);
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
int init_deinit_populate_spectral_bin_scale_params(
			struct wlan_objmgr_psoc *psoc, wmi_unified_t handle,
			uint8_t *event, struct tgt_info *info)

{
	uint8_t param_idx;
	uint32_t num_bin_scaling_params;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	num_bin_scaling_params = info->service_ext_param.num_bin_scaling_params;

	if (!num_bin_scaling_params)
		return 0;

	info->scaling_params = qdf_mem_malloc(
		sizeof(struct wlan_psoc_host_spectral_scaling_params) *
		num_bin_scaling_params);

	if (!info->scaling_params) {
		target_if_err("Mem alloc for bin scaling params failed");
		return -EINVAL;
	}

	for (param_idx = 0; param_idx < num_bin_scaling_params; param_idx++) {
		status = wmi_extract_spectral_scaling_params_service_ready_ext(
				handle,
				event, param_idx,
				&info->scaling_params[param_idx]);
		if (QDF_IS_STATUS_ERROR(status)) {
			target_if_err("Extraction of scaling params failed");
			goto free_and_return;
		}
	}

	return 0;

free_and_return:
	qdf_mem_free(info->scaling_params);
	info->scaling_params = NULL;

	return qdf_status_to_os_return(status);
}

#ifdef WLAN_SUPPORT_TWT
int init_deinit_populate_twt_cap_ext2(struct wlan_objmgr_psoc *psoc,
				      wmi_unified_t handle, uint8_t *event,
				      struct tgt_info *info)
{
	struct wmi_twt_cap_bitmap_params param;
	struct target_psoc_info *psoc_info;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wmi_extract_twt_cap_service_ready_ext2(handle, event,
							&param);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Extraction of twt capability failed");
		goto exit;
	}

	psoc_info = wlan_psoc_get_tgt_if_handle(psoc);

	target_psoc_set_twt_ack_cap(psoc_info, param.twt_ack_support_cap);

exit:
	return qdf_status_to_os_return(status);
}
#endif

int init_deinit_populate_dbs_or_sbs_cap_ext2(struct wlan_objmgr_psoc *psoc,
					     wmi_unified_t handle,
					     uint8_t *event,
					     struct tgt_info *info)
{
	uint32_t sbs_lower_band_end_freq;
	struct target_psoc_info *psoc_info;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wmi_extract_dbs_or_sbs_cap_service_ready_ext2(handle, event,
						&sbs_lower_band_end_freq);
	if (QDF_IS_STATUS_ERROR(status)) {
		target_if_err("Extraction of twt capability failed");
		goto exit;
	}
	psoc_info = wlan_psoc_get_tgt_if_handle(psoc);
	target_psoc_set_sbs_lower_band_end(psoc_info, sbs_lower_band_end_freq);

exit:
	return qdf_status_to_os_return(status);
}


QDF_STATUS init_deinit_dbr_ring_cap_free(
		struct target_psoc_info *tgt_psoc_info)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (tgt_psoc_info->info.dbr_ring_cap) {
		qdf_mem_free(tgt_psoc_info->info.dbr_ring_cap);
		tgt_psoc_info->info.dbr_ring_cap = NULL;
	}

	return status;
}
qdf_export_symbol(init_deinit_dbr_ring_cap_free);

QDF_STATUS init_deinit_spectral_scaling_params_free(
		struct target_psoc_info *tgt_psoc_info)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (tgt_psoc_info->info.scaling_params) {
		qdf_mem_free(tgt_psoc_info->info.scaling_params);
		tgt_psoc_info->info.scaling_params = NULL;
		status = QDF_STATUS_SUCCESS;
	}

	return status;
}

qdf_export_symbol(init_deinit_spectral_scaling_params_free);

#ifdef DBS_SBS_BAND_LIMITATION_WAR
#define phy0               0
#define phy2               2
#define NUM_RF_MODES       2 /* (DBS + DBS_SBS) */
/**
 * init_deinit_update_phy_reg_cap() - Update the low/high frequency for phy0.
 * @psoc: PSOC common object
 * @info: FW or lower layer related info
 * @wlan_psoc_host_hal_reg_capabilities_ext: Reg caps per PHY
 *
 * For the DBS_SBS capable board, update the low or high frequency
 * for phy0 by leveraging the frequency populated for phy2
 * depending on whether it is mapped to upper or lower 5G band by
 * FW/HAL-PHY.
 */
static void init_deinit_update_phy_reg_cap(struct wlan_objmgr_psoc *psoc,
					struct tgt_info *info,
					struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap)
{
	struct target_psoc_info *tgt_hdl;
	enum wmi_host_hw_mode_config_type mode;
	uint32_t num_hw_modes;
	uint8_t idx;

	tgt_hdl = (struct target_psoc_info *)wlan_psoc_get_tgt_if_handle(
						psoc);
	if (!tgt_hdl) {
		target_if_err("target_psoc_info is null in service ready ev");
		return;
	}

	mode = target_psoc_get_preferred_hw_mode(tgt_hdl);

	num_hw_modes = info->hw_modes.num_modes;

	if ((mode != WMI_HOST_HW_MODE_DBS) || (num_hw_modes < NUM_RF_MODES))
		return;

	for (idx = 0; idx < num_hw_modes; idx++)
		if (info->hw_modes.hw_mode_ids[idx] ==
			WMI_HOST_HW_MODE_DBS_SBS) {
			if (reg_cap[phy0].low_5ghz_chan >
					reg_cap[phy2].low_5ghz_chan)
				reg_cap[phy0].low_5ghz_chan =
				    reg_cap[phy2].low_5ghz_chan;
			else if (reg_cap[phy0].high_5ghz_chan <
					reg_cap[phy2].high_5ghz_chan)
				reg_cap[phy0].high_5ghz_chan =
				    reg_cap[phy2].high_5ghz_chan;
			break;
		}
}
#else
static void init_deinit_update_phy_reg_cap(struct wlan_objmgr_psoc *psoc,
					struct tgt_info *info,
					struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap)
{
}
#endif

/**
 * init_deinit_fill_host_reg_cap() - Fill the host regulatory cap
 * with target hal reg capabilities.
 * @cap: Pointer to wlan_psoc_hal_reg_capability where FW capabilities
 * are extracted.
 * @reg_cap: Pointer to wlan_psoc_host_hal_reg_capabilities_ext, host reg
 * capabilities to be filled.
 *
 * Return - None
 */
static void
init_deinit_fill_host_reg_cap(struct wlan_psoc_hal_reg_capability *cap,
			      struct wlan_psoc_host_hal_reg_capabilities_ext
			      *reg_cap)
{
	reg_cap->phy_id = 0;
	reg_cap->eeprom_reg_domain = cap->eeprom_rd;
	reg_cap->eeprom_reg_domain_ext = cap->eeprom_rd_ext;
	reg_cap->regcap1 = cap->regcap1;
	reg_cap->regcap2 = cap->regcap2;
	reg_cap->wireless_modes = (uint64_t)cap->wireless_modes;
	reg_cap->low_2ghz_chan = cap->low_2ghz_chan;
	reg_cap->high_2ghz_chan = cap->high_2ghz_chan;
	reg_cap->low_5ghz_chan = cap->low_5ghz_chan;
	reg_cap->high_5ghz_chan = cap->high_5ghz_chan;
}

int init_deinit_populate_phy_reg_cap(struct wlan_objmgr_psoc *psoc,
				     wmi_unified_t handle, uint8_t *event,
				     struct tgt_info *info,
				     bool service_ready)
{
	uint8_t reg_idx;
	uint32_t num_phy_reg_cap;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_psoc_hal_reg_capability cap;
	struct wlan_psoc_host_hal_reg_capabilities_ext
				reg_cap[PSOC_MAX_PHY_REG_CAP] = {{0} };

	if (service_ready) {
		status = wmi_extract_hal_reg_cap(handle, event, &cap);
		if (QDF_IS_STATUS_ERROR(status)) {
			target_if_err("failed to parse hal reg cap");
			return qdf_status_to_os_return(status);
		}
		info->service_ext_param.num_phy = 1;
		num_phy_reg_cap = 1;
		init_deinit_fill_host_reg_cap(&cap, &reg_cap[0]);
		target_if_debug("FW wireless modes 0x%llx",
				reg_cap[0].wireless_modes);
	} else {
		num_phy_reg_cap = info->service_ext_param.num_phy;
		if (num_phy_reg_cap > PSOC_MAX_PHY_REG_CAP) {
			target_if_err("Invalid num_phy_reg_cap %d",
				      num_phy_reg_cap);
			return -EINVAL;
		}
		target_if_debug("num_phy_reg_cap %d", num_phy_reg_cap);

		for (reg_idx = 0; reg_idx < num_phy_reg_cap; reg_idx++) {
			status = wmi_extract_reg_cap_service_ready_ext(handle,
					event, reg_idx, &(reg_cap[reg_idx]));
			if (QDF_IS_STATUS_ERROR(status)) {
				target_if_err("failed to parse reg cap");
				return qdf_status_to_os_return(status);
			}
		}
	}

	init_deinit_update_phy_reg_cap(psoc, info, reg_cap);
	status = ucfg_reg_set_hal_reg_cap(psoc, reg_cap, num_phy_reg_cap);

	return qdf_status_to_os_return(status);
}

int init_deinit_populate_mac_phy_cap_ext2(wmi_unified_t wmi_handle,
					  uint8_t *event,
					  struct tgt_info *info)
{
	uint32_t num_hw_modes;
	uint8_t hw_idx;
	uint32_t hw_mode_id;
	uint32_t phy_bit_map;
	uint8_t phy_id;
	uint8_t mac_phy_count = 0;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct wlan_psoc_host_mac_phy_caps *mac_phy_cap;
	struct wlan_psoc_host_mac_phy_caps_ext2 *mac_phy_caps_ext2;

	if (!event)
		return -EINVAL;

	num_hw_modes = info->hw_modes.num_modes;

	for (hw_idx = 0; hw_idx < num_hw_modes; hw_idx++) {
		hw_mode_id = info->hw_modes.hw_mode_ids[hw_idx];
		phy_bit_map = info->hw_modes.phy_bit_map[hw_idx];

		phy_id = info->mac_phy_cap[mac_phy_count].phy_id;
		while (phy_bit_map) {
			if (mac_phy_count >= info->total_mac_phy_cnt) {
				target_if_err("total MAC PHY count exceeds max limit %d, mac_phy_count = %d",
					      info->total_mac_phy_cnt,
					      mac_phy_count);
				return -EINVAL;
			}

			mac_phy_cap = &info->mac_phy_cap[mac_phy_count];
			mac_phy_caps_ext2 =
				&info->mac_phy_caps_ext2[mac_phy_count];
			status = wmi_extract_mac_phy_cap_service_ready_ext2(
					wmi_handle, event, hw_mode_id, phy_id,
					mac_phy_cap->phy_idx,
					mac_phy_caps_ext2);

			if (QDF_IS_STATUS_ERROR(status)) {
				target_if_err("failed to parse mac phy capability ext2");
				return qdf_status_to_os_return(status);
			}

			mac_phy_cap->reg_cap_ext.wireless_modes |=
				mac_phy_caps_ext2[phy_id].wireless_modes_ext;

			mac_phy_count++;
			phy_bit_map &= (phy_bit_map - 1);
			phy_id++;
		}
	}

	return 0;
}

int init_deinit_populate_hal_reg_cap_ext2(wmi_unified_t wmi_handle,
					  uint8_t *event,
					  struct tgt_info *info)
{
	struct wlan_psoc_host_hal_reg_capabilities_ext2
		reg_cap[PSOC_MAX_PHY_REG_CAP] = {{0} };
	struct wlan_objmgr_psoc *psoc;
	uint32_t num_phy_reg_cap;
	uint8_t reg_idx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!event) {
		target_if_err("event buffer is null");
		return -EINVAL;
	}

	psoc = target_if_get_psoc_from_scn_hdl(wmi_handle->scn_handle);
	if (!psoc) {
		target_if_err("psoc is null");
		return -EINVAL;
	}

	num_phy_reg_cap = info->service_ext_param.num_phy;
	if (num_phy_reg_cap > PSOC_MAX_PHY_REG_CAP) {
		target_if_err("Invalid num_phy_reg_cap %d", num_phy_reg_cap);
		return -EINVAL;
	}

	for (reg_idx = 0; reg_idx < num_phy_reg_cap; reg_idx++) {
		status = wmi_extract_hal_reg_cap_ext2(
				wmi_handle, event, reg_idx, &reg_cap[reg_idx]);
		if (QDF_IS_STATUS_ERROR(status)) {
			target_if_err("failed to parse hal reg cap ext2");
			return qdf_status_to_os_return(status);
		}

		status = ucfg_reg_update_hal_reg_cap(
				psoc, reg_cap[reg_idx].wireless_modes_ext,
				reg_idx);
		if (QDF_IS_STATUS_ERROR(status)) {
			target_if_err("Failed to update hal reg cap");
			return qdf_status_to_os_return(status);
		}
	}

	return 0;
}

int init_deinit_populate_scan_radio_cap_ext2(wmi_unified_t wmi_handle,
					     uint8_t *event,
					     struct tgt_info *info)
{
	struct wlan_psoc_host_scan_radio_caps *param;
	uint32_t num_scan_radio_caps;
	uint8_t cap_idx;
	QDF_STATUS status;

	if (!event) {
		target_if_err("Invalid event buffer");
		return -EINVAL;
	}

	num_scan_radio_caps = info->service_ext2_param.num_scan_radio_caps;
	target_if_debug("num scan radio capabilities = %d",
			num_scan_radio_caps);

	if (!num_scan_radio_caps)
		return 0;

	info->scan_radio_caps = qdf_mem_malloc(
				sizeof(struct wlan_psoc_host_scan_radio_caps) *
				num_scan_radio_caps);

	if (!info->scan_radio_caps) {
		target_if_err("Failed to allocate memory for scan radio caps");
		return -EINVAL;
	}

	for (cap_idx = 0; cap_idx < num_scan_radio_caps; cap_idx++) {
		param = &info->scan_radio_caps[cap_idx];
		status = wmi_extract_scan_radio_cap_service_ready_ext2(
				wmi_handle, event, cap_idx, param);
		if (QDF_IS_STATUS_ERROR(status)) {
			target_if_err("Extraction of scan radio cap failed");
			goto free_and_return;
		}
	}

	return 0;

free_and_return:
	qdf_mem_free(info->scan_radio_caps);
	info->scan_radio_caps = NULL;

	return qdf_status_to_os_return(status);
}

QDF_STATUS init_deinit_scan_radio_cap_free(
		struct target_psoc_info *tgt_psoc_info)
{
	qdf_mem_free(tgt_psoc_info->info.scan_radio_caps);
	tgt_psoc_info->info.scan_radio_caps = NULL;

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(init_deinit_scan_radio_cap_free);

static bool init_deinit_regdmn_160mhz_support(
		struct wlan_psoc_host_hal_reg_capabilities_ext *hal_cap)
{
	return ((hal_cap->wireless_modes &
		HOST_REGDMN_MODE_11AC_VHT160) != 0);
}

static bool init_deinit_regdmn_80p80mhz_support(
		struct wlan_psoc_host_hal_reg_capabilities_ext *hal_cap)
{
	return ((hal_cap->wireless_modes &
			HOST_REGDMN_MODE_11AC_VHT80_80) != 0);
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
	struct wmi_unified *wmi_handle;

	if (!tgt_hdl) {
		target_if_err(
		"target_psoc_info is null in validate 160n80p80 cap check");
		return QDF_STATUS_E_INVAL;
	}

	wmi_handle = target_psoc_get_wmi_hdl(tgt_hdl);

	if ((tgt_hdl->info.target_type == TARGET_TYPE_QCA8074) ||
	    (tgt_hdl->info.target_type == TARGET_TYPE_QCA8074V2) ||
	    (tgt_hdl->info.target_type == TARGET_TYPE_QCN6122) ||
	    (tgt_hdl->info.target_type == TARGET_TYPE_QCA6290)) {
		/**
		 * Return true for now. This is not available in
		 * qca8074 fw yet
		 */
		return QDF_STATUS_SUCCESS;
	}

	reg_cap = ucfg_reg_get_hal_reg_cap(psoc);
	if (!reg_cap) {
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
	struct wmi_unified *wmi_handle;

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

	if (wlan_psoc_nif_feat_cap_get(psoc, WLAN_SOC_F_DYNAMIC_HW_MODE)) {
		if (!wlan_psoc_nif_fw_ext_cap_get(psoc,
					WLAN_SOC_CEXT_DYNAMIC_HW_MODE)) {
			target_if_err(
			"WMI service bit for DYNAMIC HW mode is not set!");
			return FALSE;
		}
	}

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
	qdf_event_set(&tgt_hdl->info.event);
}
