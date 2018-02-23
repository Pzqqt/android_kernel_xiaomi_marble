/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: wlan_policy_mgr_get_set_utils.c
 *
 * WLAN Concurrenct Connection Management APIs
 *
 */

/* Include files */
#include "target_if.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_policy_mgr_i.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "wlan_objmgr_vdev_obj.h"

/* invalid channel id. */
#define INVALID_CHANNEL_ID 0

void policy_mgr_update_new_hw_mode_index(struct wlan_objmgr_psoc *psoc,
		uint32_t new_hw_mode_index)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}
	pm_ctx->new_hw_mode_index = new_hw_mode_index;
}

void policy_mgr_update_old_hw_mode_index(struct wlan_objmgr_psoc *psoc,
		uint32_t old_hw_mode_index)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}
	pm_ctx->old_hw_mode_index = old_hw_mode_index;
}

void policy_mgr_update_hw_mode_index(struct wlan_objmgr_psoc *psoc,
		uint32_t new_hw_mode_index)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}
	if (POLICY_MGR_DEFAULT_HW_MODE_INDEX == pm_ctx->new_hw_mode_index) {
		pm_ctx->new_hw_mode_index = new_hw_mode_index;
	} else {
		pm_ctx->old_hw_mode_index = pm_ctx->new_hw_mode_index;
		pm_ctx->new_hw_mode_index = new_hw_mode_index;
	}
	policy_mgr_debug("Updated: old_hw_mode_index:%d new_hw_mode_index:%d",
		pm_ctx->old_hw_mode_index, pm_ctx->new_hw_mode_index);
}

/**
 * policy_mgr_get_num_of_setbits_from_bitmask() - to get num of
 * setbits from bitmask
 * @mask: given bitmask
 *
 * This helper function should return number of setbits from bitmask
 *
 * Return: number of setbits from bitmask
 */
static uint32_t policy_mgr_get_num_of_setbits_from_bitmask(uint32_t mask)
{
	uint32_t num_of_setbits = 0;

	while (mask) {
		mask &= (mask - 1);
		num_of_setbits++;
	}
	return num_of_setbits;
}

/**
 * policy_mgr_map_wmi_channel_width_to_hw_mode_bw() - returns
 * bandwidth in terms of hw_mode_bandwidth
 * @width: bandwidth in terms of wmi_channel_width
 *
 * This function returns the bandwidth in terms of hw_mode_bandwidth.
 *
 * Return: BW in terms of hw_mode_bandwidth.
 */
static enum hw_mode_bandwidth policy_mgr_map_wmi_channel_width_to_hw_mode_bw(
		wmi_channel_width width)
{
	switch (width) {
	case WMI_CHAN_WIDTH_20:
		return HW_MODE_20_MHZ;
	case WMI_CHAN_WIDTH_40:
		return HW_MODE_40_MHZ;
	case WMI_CHAN_WIDTH_80:
		return HW_MODE_80_MHZ;
	case WMI_CHAN_WIDTH_160:
		return HW_MODE_160_MHZ;
	case WMI_CHAN_WIDTH_80P80:
		return HW_MODE_80_PLUS_80_MHZ;
	case WMI_CHAN_WIDTH_5:
		return HW_MODE_5_MHZ;
	case WMI_CHAN_WIDTH_10:
		return HW_MODE_10_MHZ;
	default:
		return HW_MODE_BW_NONE;
	}

	return HW_MODE_BW_NONE;
}

static void policy_mgr_get_hw_mode_params(
		struct wlan_psoc_host_mac_phy_caps *caps,
		struct policy_mgr_mac_ss_bw_info *info)
{
	if (!caps) {
		policy_mgr_err("Invalid capabilities");
		return;
	}

	info->mac_tx_stream = policy_mgr_get_num_of_setbits_from_bitmask(
		QDF_MAX(caps->tx_chain_mask_2G,
		caps->tx_chain_mask_5G));
	info->mac_rx_stream = policy_mgr_get_num_of_setbits_from_bitmask(
		QDF_MAX(caps->rx_chain_mask_2G,
		caps->rx_chain_mask_5G));
	info->mac_bw = policy_mgr_map_wmi_channel_width_to_hw_mode_bw(
		QDF_MAX(caps->max_bw_supported_2G,
		caps->max_bw_supported_5G));
}

/**
 * policy_mgr_set_hw_mode_params() - sets TX-RX stream,
 * bandwidth and DBS in hw_mode_list
 * @wma_handle: pointer to wma global structure
 * @mac0_ss_bw_info: TX-RX streams, BW for MAC0
 * @mac1_ss_bw_info: TX-RX streams, BW for MAC1
 * @pos: refers to hw_mode_index
 * @dbs_mode: dbs_mode for the dbs_hw_mode
 * @sbs_mode: sbs_mode for the sbs_hw_mode
 *
 * This function sets TX-RX stream, bandwidth and DBS mode in
 * hw_mode_list.
 *
 * Return: none
 */
static void policy_mgr_set_hw_mode_params(struct wlan_objmgr_psoc *psoc,
			struct policy_mgr_mac_ss_bw_info mac0_ss_bw_info,
			struct policy_mgr_mac_ss_bw_info mac1_ss_bw_info,
			uint32_t pos, uint32_t dbs_mode,
			uint32_t sbs_mode)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_SET(
		pm_ctx->hw_mode.hw_mode_list[pos],
		mac0_ss_bw_info.mac_tx_stream);
	POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_SET(
		pm_ctx->hw_mode.hw_mode_list[pos],
		mac0_ss_bw_info.mac_rx_stream);
	POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_SET(
		pm_ctx->hw_mode.hw_mode_list[pos],
		mac0_ss_bw_info.mac_bw);
	POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_SET(
		pm_ctx->hw_mode.hw_mode_list[pos],
		mac1_ss_bw_info.mac_tx_stream);
	POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_SET(
		pm_ctx->hw_mode.hw_mode_list[pos],
		mac1_ss_bw_info.mac_rx_stream);
	POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_SET(
		pm_ctx->hw_mode.hw_mode_list[pos],
		mac1_ss_bw_info.mac_bw);
	POLICY_MGR_HW_MODE_DBS_MODE_SET(
		pm_ctx->hw_mode.hw_mode_list[pos],
		dbs_mode);
	POLICY_MGR_HW_MODE_AGILE_DFS_SET(
		pm_ctx->hw_mode.hw_mode_list[pos],
		HW_MODE_AGILE_DFS_NONE);
	POLICY_MGR_HW_MODE_SBS_MODE_SET(
		pm_ctx->hw_mode.hw_mode_list[pos],
		sbs_mode);
}

QDF_STATUS policy_mgr_update_hw_mode_list(struct wlan_objmgr_psoc *psoc,
					  struct target_psoc_info *tgt_hdl)
{
	struct wlan_psoc_host_mac_phy_caps *tmp;
	uint32_t i, hw_config_type, j = 0;
	uint32_t dbs_mode, sbs_mode;
	struct policy_mgr_mac_ss_bw_info mac0_ss_bw_info = {0};
	struct policy_mgr_mac_ss_bw_info mac1_ss_bw_info = {0};
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	struct tgt_info *info;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	info = &tgt_hdl->info;
	if (!info->service_ext_param.num_hw_modes) {
		policy_mgr_err("Number of HW modes: %d",
			       info->service_ext_param.num_hw_modes);
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * This list was updated as part of service ready event. Re-populate
	 * HW mode list from the device capabilities.
	 */
	if (pm_ctx->hw_mode.hw_mode_list) {
		qdf_mem_free(pm_ctx->hw_mode.hw_mode_list);
		pm_ctx->hw_mode.hw_mode_list = NULL;
		policy_mgr_debug("DBS list is freed");
	}

	pm_ctx->num_dbs_hw_modes = info->service_ext_param.num_hw_modes;
	pm_ctx->hw_mode.hw_mode_list =
		qdf_mem_malloc(sizeof(*pm_ctx->hw_mode.hw_mode_list) *
		pm_ctx->num_dbs_hw_modes);
	if (!pm_ctx->hw_mode.hw_mode_list) {
		policy_mgr_err("Memory allocation failed for DBS");
		return QDF_STATUS_E_FAILURE;
	}

	policy_mgr_debug("Updated HW mode list: Num modes:%d",
		pm_ctx->num_dbs_hw_modes);

	for (i = 0; i < pm_ctx->num_dbs_hw_modes; i++) {
		/* Update for MAC0 */
		tmp = &info->mac_phy_cap[j++];
		policy_mgr_get_hw_mode_params(tmp, &mac0_ss_bw_info);
		hw_config_type = tmp->hw_mode_config_type;
		dbs_mode = HW_MODE_DBS_NONE;
		sbs_mode = HW_MODE_SBS_NONE;
		mac1_ss_bw_info.mac_tx_stream = 0;
		mac1_ss_bw_info.mac_rx_stream = 0;
		mac1_ss_bw_info.mac_bw = 0;

		/* SBS and DBS have dual MAC. Upto 2 MACs are considered. */
		if ((hw_config_type == WMI_HW_MODE_DBS) ||
			(hw_config_type == WMI_HW_MODE_SBS_PASSIVE) ||
			(hw_config_type == WMI_HW_MODE_SBS)) {
			/* Update for MAC1 */
			tmp = &info->mac_phy_cap[j++];
			policy_mgr_get_hw_mode_params(tmp, &mac1_ss_bw_info);
			if (hw_config_type == WMI_HW_MODE_DBS)
				dbs_mode = HW_MODE_DBS;
			if ((hw_config_type == WMI_HW_MODE_SBS_PASSIVE) ||
				(hw_config_type == WMI_HW_MODE_SBS))
				sbs_mode = HW_MODE_SBS;
		}

		/* Updating HW mode list */
		policy_mgr_set_hw_mode_params(psoc, mac0_ss_bw_info,
			mac1_ss_bw_info, i, dbs_mode, sbs_mode);
	}
	return QDF_STATUS_SUCCESS;
}

void policy_mgr_init_dbs_hw_mode(struct wlan_objmgr_psoc *psoc,
		uint32_t num_dbs_hw_modes,
		uint32_t *ev_wlan_dbs_hw_mode_list)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	pm_ctx->num_dbs_hw_modes = num_dbs_hw_modes;
	pm_ctx->hw_mode.hw_mode_list =
		qdf_mem_malloc(sizeof(*pm_ctx->hw_mode.hw_mode_list) *
		pm_ctx->num_dbs_hw_modes);
	if (!pm_ctx->hw_mode.hw_mode_list) {
		policy_mgr_err("Memory allocation failed for DBS");
		return;
	}
	qdf_mem_copy(pm_ctx->hw_mode.hw_mode_list,
		ev_wlan_dbs_hw_mode_list,
		(sizeof(*pm_ctx->hw_mode.hw_mode_list) *
		pm_ctx->num_dbs_hw_modes));
}

void policy_mgr_dump_dbs_hw_mode(struct wlan_objmgr_psoc *psoc)
{
	uint32_t i, param;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	for (i = 0; i < pm_ctx->num_dbs_hw_modes; i++) {
		param = pm_ctx->hw_mode.hw_mode_list[i];
		policy_mgr_debug("[%d]-MAC0: tx_ss:%d rx_ss:%d bw_idx:%d",
			i,
			POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_GET(param),
			POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_GET(param),
			POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_GET(param));
		policy_mgr_debug("[%d]-MAC1: tx_ss:%d rx_ss:%d bw_idx:%d",
			i,
			POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_GET(param),
			POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_GET(param),
			POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_GET(param));
		policy_mgr_debug("[%d] DBS:%d SBS:%d", i,
			POLICY_MGR_HW_MODE_DBS_MODE_GET(param),
			POLICY_MGR_HW_MODE_SBS_MODE_GET(param));
	}
}

void policy_mgr_init_dbs_config(struct wlan_objmgr_psoc *psoc,
		uint32_t scan_config, uint32_t fw_config)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}
	pm_ctx->dual_mac_cfg.cur_scan_config = 0;
	pm_ctx->dual_mac_cfg.cur_fw_mode_config = 0;

	/* If dual mac features are disabled in the INI, we
	 * need not proceed further
	 */
	if (DISABLE_DBS_CXN_AND_SCAN ==
			wlan_objmgr_psoc_get_dual_mac_disable(psoc)) {
		policy_mgr_err("Disabling dual mac capabilities");
		/* All capabilites are initialized to 0. We can return */
		goto done;
	}

	/* Initialize concurrent_scan_config_bits with default FW value */
	WMI_DBS_CONC_SCAN_CFG_ASYNC_DBS_SCAN_SET(
		pm_ctx->dual_mac_cfg.cur_scan_config,
		WMI_DBS_CONC_SCAN_CFG_ASYNC_DBS_SCAN_GET(scan_config));
	WMI_DBS_CONC_SCAN_CFG_SYNC_DBS_SCAN_SET(
		pm_ctx->dual_mac_cfg.cur_scan_config,
		WMI_DBS_CONC_SCAN_CFG_SYNC_DBS_SCAN_GET(scan_config));
	WMI_DBS_CONC_SCAN_CFG_DBS_SCAN_SET(
		pm_ctx->dual_mac_cfg.cur_scan_config,
		WMI_DBS_CONC_SCAN_CFG_DBS_SCAN_GET(scan_config));
	WMI_DBS_CONC_SCAN_CFG_AGILE_SCAN_SET(
		pm_ctx->dual_mac_cfg.cur_scan_config,
		WMI_DBS_CONC_SCAN_CFG_AGILE_SCAN_GET(scan_config));
	WMI_DBS_CONC_SCAN_CFG_AGILE_DFS_SCAN_SET(
		pm_ctx->dual_mac_cfg.cur_scan_config,
		WMI_DBS_CONC_SCAN_CFG_AGILE_DFS_SCAN_GET(scan_config));

	/* Initialize fw_mode_config_bits with default FW value */
	WMI_DBS_FW_MODE_CFG_DBS_SET(
		pm_ctx->dual_mac_cfg.cur_fw_mode_config,
		WMI_DBS_FW_MODE_CFG_DBS_GET(fw_config));
	WMI_DBS_FW_MODE_CFG_AGILE_DFS_SET(
		pm_ctx->dual_mac_cfg.cur_fw_mode_config,
		WMI_DBS_FW_MODE_CFG_AGILE_DFS_GET(fw_config));
	WMI_DBS_FW_MODE_CFG_DBS_FOR_CXN_SET(
		pm_ctx->dual_mac_cfg.cur_fw_mode_config,
		WMI_DBS_FW_MODE_CFG_DBS_FOR_CXN_GET(fw_config));
done:
	/* Initialize the previous scan/fw mode config */
	pm_ctx->dual_mac_cfg.prev_scan_config =
		pm_ctx->dual_mac_cfg.cur_scan_config;
	pm_ctx->dual_mac_cfg.prev_fw_mode_config =
		pm_ctx->dual_mac_cfg.cur_fw_mode_config;

	policy_mgr_debug("cur_scan_config:%x cur_fw_mode_config:%x",
		pm_ctx->dual_mac_cfg.cur_scan_config,
		pm_ctx->dual_mac_cfg.cur_fw_mode_config);
}

void policy_mgr_update_dbs_scan_config(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	pm_ctx->dual_mac_cfg.prev_scan_config =
		pm_ctx->dual_mac_cfg.cur_scan_config;
	pm_ctx->dual_mac_cfg.cur_scan_config =
		pm_ctx->dual_mac_cfg.req_scan_config;
}

void policy_mgr_update_dbs_fw_config(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	pm_ctx->dual_mac_cfg.prev_fw_mode_config =
		pm_ctx->dual_mac_cfg.cur_fw_mode_config;
	pm_ctx->dual_mac_cfg.cur_fw_mode_config =
		pm_ctx->dual_mac_cfg.req_fw_mode_config;
}

void policy_mgr_update_dbs_req_config(struct wlan_objmgr_psoc *psoc,
		uint32_t scan_config, uint32_t fw_mode_config)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}
	pm_ctx->dual_mac_cfg.req_scan_config = scan_config;
	pm_ctx->dual_mac_cfg.req_fw_mode_config = fw_mode_config;
}

bool policy_mgr_get_dbs_plus_agile_scan_config(struct wlan_objmgr_psoc *psoc)
{
	uint32_t scan_config;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	if (policy_mgr_is_dual_mac_disabled_in_ini(psoc))
		return false;


	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		/* We take that it is disabled and proceed */
		return false;
	}
	scan_config = pm_ctx->dual_mac_cfg.cur_scan_config;

	return WMI_DBS_CONC_SCAN_CFG_AGILE_SCAN_GET(scan_config);
}

bool policy_mgr_get_single_mac_scan_with_dfs_config(
		struct wlan_objmgr_psoc *psoc)
{
	uint32_t scan_config;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	if (policy_mgr_is_dual_mac_disabled_in_ini(psoc))
		return false;


	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		/* We take that it is disabled and proceed */
		return false;
	}
	scan_config = pm_ctx->dual_mac_cfg.cur_scan_config;

	return WMI_DBS_CONC_SCAN_CFG_AGILE_DFS_SCAN_GET(scan_config);
}

int8_t policy_mgr_get_num_dbs_hw_modes(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return -EINVAL;
	}
	return pm_ctx->num_dbs_hw_modes;
}

bool policy_mgr_is_hw_dbs_capable(struct wlan_objmgr_psoc *psoc)
{
	uint32_t param, i, found = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	void *wmi_handle;

	pm_ctx = policy_mgr_get_context(psoc);

	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	if (!policy_mgr_is_dbs_enable(psoc)) {
		policy_mgr_debug("DBS is disabled");
		return false;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);
	if (!wmi_handle) {
		policy_mgr_debug("Invalid WMA context");
		return false;
	}

	policy_mgr_debug("DBS service bit map: %d",
		wmi_service_enabled(wmi_handle,
		wmi_service_dual_band_simultaneous_support));

	/* The agreement with FW is that: To know if the target is DBS
	 * capable, DBS needs to be supported both in the HW mode list
	 * and in the service ready event
	 */
	if (!(wmi_service_enabled(wmi_handle,
		wmi_service_dual_band_simultaneous_support)))
		return false;

	for (i = 0; i < pm_ctx->num_dbs_hw_modes; i++) {
		param = pm_ctx->hw_mode.hw_mode_list[i];
		policy_mgr_debug("HW param: %x", param);
		if (POLICY_MGR_HW_MODE_DBS_MODE_GET(param)) {
			policy_mgr_debug("HW (%d) is DBS capable", i);
			found = 1;
			break;
		}
	}

	if (found)
		return true;

	return false;
}

bool policy_mgr_is_hw_sbs_capable(struct wlan_objmgr_psoc *psoc)
{
	uint32_t param, i, found = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	void *wmi_handle;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	wmi_handle = GET_WMI_HDL_FROM_PSOC(psoc);

	policy_mgr_debug("DBS service bit map: %d",
		wmi_service_enabled(wmi_handle,
		wmi_service_dual_band_simultaneous_support));

	/* The agreement with FW is that: To know if the target is SBS
	 * capable, SBS needs to be supported both in the HW mode list
	 * and DBS needs to be supported in the service ready event
	 */
	if (!(wmi_service_enabled(wmi_handle,
		wmi_service_dual_band_simultaneous_support)))
		return false;

	for (i = 0; i < pm_ctx->num_dbs_hw_modes; i++) {
		param = pm_ctx->hw_mode.hw_mode_list[i];
		policy_mgr_debug("HW param: %x", param);
		if (POLICY_MGR_HW_MODE_SBS_MODE_GET(param)) {
			policy_mgr_debug("HW (%d) is SBS capable", i);
			found = 1;
			break;
		}
	}

	if (found)
		return true;

	return false;
}

QDF_STATUS policy_mgr_get_dbs_hw_modes(struct wlan_objmgr_psoc *psoc,
		bool *one_by_one_dbs, bool *two_by_two_dbs)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t i;
	int8_t found_one_by_one = -EINVAL, found_two_by_two = -EINVAL;
	uint32_t conf1_tx_ss, conf1_rx_ss;
	uint32_t conf2_tx_ss, conf2_rx_ss;

	*one_by_one_dbs = false;
	*two_by_two_dbs = false;

	if (policy_mgr_is_hw_dbs_capable(psoc) == false) {
		policy_mgr_err("HW is not DBS capable");
		/* Caller will understand that DBS is disabled */
		return QDF_STATUS_SUCCESS;

	}

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	/* To check 1x1 capability */
	policy_mgr_get_tx_rx_ss_from_config(HW_MODE_SS_1x1,
			&conf1_tx_ss, &conf1_rx_ss);
	/* To check 2x2 capability */
	policy_mgr_get_tx_rx_ss_from_config(HW_MODE_SS_2x2,
			&conf2_tx_ss, &conf2_rx_ss);

	for (i = 0; i < pm_ctx->num_dbs_hw_modes; i++) {
		uint32_t t_conf0_tx_ss, t_conf0_rx_ss;
		uint32_t t_conf1_tx_ss, t_conf1_rx_ss;
		uint32_t dbs_mode;

		t_conf0_tx_ss = POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		t_conf0_rx_ss = POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		t_conf1_tx_ss = POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		t_conf1_rx_ss = POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		dbs_mode = POLICY_MGR_HW_MODE_DBS_MODE_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);

		if (((((t_conf0_tx_ss == conf1_tx_ss) &&
		    (t_conf0_rx_ss == conf1_rx_ss)) ||
		    ((t_conf1_tx_ss == conf1_tx_ss) &&
		    (t_conf1_rx_ss == conf1_rx_ss))) &&
		    (dbs_mode == HW_MODE_DBS)) &&
		    (found_one_by_one < 0)) {
			found_one_by_one = i;
			policy_mgr_debug("1x1 hw_mode index %d found", i);
			/* Once an entry is found, need not check for 1x1
			 * again
			 */
			continue;
		}

		if (((((t_conf0_tx_ss == conf2_tx_ss) &&
		    (t_conf0_rx_ss == conf2_rx_ss)) ||
		    ((t_conf1_tx_ss == conf2_tx_ss) &&
		    (t_conf1_rx_ss == conf2_rx_ss))) &&
		    (dbs_mode == HW_MODE_DBS)) &&
		    (found_two_by_two < 0)) {
			found_two_by_two = i;
			policy_mgr_debug("2x2 hw_mode index %d found", i);
			/* Once an entry is found, need not check for 2x2
			 * again
			 */
			continue;
		}
	}

	if (found_one_by_one >= 0)
		*one_by_one_dbs = true;
	if (found_two_by_two >= 0)
		*two_by_two_dbs = true;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_get_current_hw_mode(struct wlan_objmgr_psoc *psoc,
		struct policy_mgr_hw_mode_params *hw_mode)
{
	QDF_STATUS status;
	uint32_t old_hw_index = 0, new_hw_index = 0;

	policy_mgr_debug("Get the current hw mode");

	status = policy_mgr_get_old_and_new_hw_index(psoc, &old_hw_index,
			&new_hw_index);
	if (QDF_STATUS_SUCCESS != status) {
		policy_mgr_err("Failed to get HW mode index");
		return QDF_STATUS_E_FAILURE;
	}

	if (new_hw_index == POLICY_MGR_DEFAULT_HW_MODE_INDEX) {
		policy_mgr_err("HW mode is not yet initialized");
		return QDF_STATUS_E_FAILURE;
	}

	status = policy_mgr_get_hw_mode_from_idx(psoc, new_hw_index, hw_mode);
	if (QDF_STATUS_SUCCESS != status) {
		policy_mgr_err("Failed to get HW mode index");
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

bool policy_mgr_is_current_hwmode_dbs(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_hw_mode_params hw_mode;

	if (!policy_mgr_is_hw_dbs_capable(psoc))
		return false;
	if (QDF_STATUS_SUCCESS !=
		policy_mgr_get_current_hw_mode(psoc, &hw_mode))
		return false;
	if (hw_mode.dbs_cap)
		return true;
	return false;
}

bool policy_mgr_is_dbs_enable(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	if (policy_mgr_is_dual_mac_disabled_in_ini(psoc)) {
		policy_mgr_err("DBS is disabled from ini");
		return false;
	}

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	policy_mgr_debug("DBS=%d",
		WMI_DBS_FW_MODE_CFG_DBS_GET(
			pm_ctx->dual_mac_cfg.cur_fw_mode_config));

	if (WMI_DBS_FW_MODE_CFG_DBS_GET(
			pm_ctx->dual_mac_cfg.cur_fw_mode_config))
		return true;

	return false;
}

bool policy_mgr_is_hw_dbs_2x2_capable(struct wlan_objmgr_psoc *psoc)
{
	struct dbs_nss nss_dbs;

	return ((policy_mgr_get_hw_dbs_nss(psoc, &nss_dbs)) >= HW_MODE_SS_2x2)
		? true : false;
}

uint32_t policy_mgr_get_connection_count(struct wlan_objmgr_psoc *psoc)
{
	uint32_t conn_index, count = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return count;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
		conn_index++) {
		if (pm_conc_connection_list[conn_index].in_use)
			count++;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return count;
}

uint32_t policy_mgr_mode_specific_vdev_id(struct wlan_objmgr_psoc *psoc,
					  enum policy_mgr_con_mode mode)
{
	uint32_t conn_index = 0;
	uint32_t vdev_id = WLAN_INVALID_VDEV_ID;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return vdev_id;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	/*
	 * Note: This gives you the first vdev id of the mode type in a
	 * sta+sta or sap+sap or p2p + p2p case
	 */
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
		conn_index++) {
		if ((pm_conc_connection_list[conn_index].mode == mode) &&
			pm_conc_connection_list[conn_index].in_use) {
			vdev_id = pm_conc_connection_list[conn_index].vdev_id;
			break;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return vdev_id;
}

uint32_t policy_mgr_mode_specific_connection_count(
		struct wlan_objmgr_psoc *psoc,
		enum policy_mgr_con_mode mode,
		uint32_t *list)
{
	uint32_t conn_index = 0, count = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return count;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
		conn_index++) {
		if ((pm_conc_connection_list[conn_index].mode == mode) &&
			pm_conc_connection_list[conn_index].in_use) {
			if (list != NULL)
				list[count] = conn_index;
			 count++;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return count;
}

QDF_STATUS policy_mgr_check_conn_with_mode_and_vdev_id(
		struct wlan_objmgr_psoc *psoc, enum policy_mgr_con_mode mode,
		uint32_t vdev_id)
{
	QDF_STATUS qdf_status = QDF_STATUS_E_FAILURE;
	uint32_t conn_index = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return qdf_status;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
		if ((pm_conc_connection_list[conn_index].mode == mode) &&
		    (pm_conc_connection_list[conn_index].vdev_id == vdev_id)) {
			qdf_status = QDF_STATUS_SUCCESS;
			break;
		}
		conn_index++;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
	return qdf_status;
}

void policy_mgr_soc_set_dual_mac_cfg_cb(enum set_hw_mode_status status,
		uint32_t scan_config,
		uint32_t fw_mode_config)
{
	policy_mgr_debug("Status:%d for scan_config:%x fw_mode_config:%x",
			status, scan_config, fw_mode_config);
}

void policy_mgr_set_dual_mac_scan_config(struct wlan_objmgr_psoc *psoc,
		uint8_t dbs_val,
		uint8_t dbs_plus_agile_scan_val,
		uint8_t single_mac_scan_with_dbs_val)
{
	struct policy_mgr_dual_mac_config cfg;
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	/* Any non-zero positive value is treated as 1 */
	if (dbs_val != 0)
		dbs_val = 1;
	if (dbs_plus_agile_scan_val != 0)
		dbs_plus_agile_scan_val = 1;
	if (single_mac_scan_with_dbs_val != 0)
		single_mac_scan_with_dbs_val = 1;

	status = policy_mgr_get_updated_scan_config(psoc, &cfg.scan_config,
			dbs_val,
			dbs_plus_agile_scan_val,
			single_mac_scan_with_dbs_val);
	if (status != QDF_STATUS_SUCCESS) {
		policy_mgr_err("policy_mgr_get_updated_scan_config failed %d",
			status);
		return;
	}

	status = policy_mgr_get_updated_fw_mode_config(psoc,
			&cfg.fw_mode_config,
			policy_mgr_get_dbs_config(psoc),
			policy_mgr_get_agile_dfs_config(psoc));
	if (status != QDF_STATUS_SUCCESS) {
		policy_mgr_err("policy_mgr_get_updated_fw_mode_config failed %d",
			status);
		return;
	}

	cfg.set_dual_mac_cb = (void *)policy_mgr_soc_set_dual_mac_cfg_cb;

	policy_mgr_debug("scan_config:%x fw_mode_config:%x",
			cfg.scan_config, cfg.fw_mode_config);

	status = pm_ctx->sme_cbacks.sme_soc_set_dual_mac_config(cfg);
	if (status != QDF_STATUS_SUCCESS)
		policy_mgr_err("sme_soc_set_dual_mac_config failed %d", status);
}

void policy_mgr_set_dual_mac_fw_mode_config(struct wlan_objmgr_psoc *psoc,
			uint8_t dbs, uint8_t dfs)
{
	struct policy_mgr_dual_mac_config cfg;
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	/* Any non-zero positive value is treated as 1 */
	if (dbs != 0)
		dbs = 1;
	if (dfs != 0)
		dfs = 1;

	status = policy_mgr_get_updated_scan_config(psoc, &cfg.scan_config,
			policy_mgr_get_dbs_scan_config(psoc),
			policy_mgr_get_dbs_plus_agile_scan_config(psoc),
			policy_mgr_get_single_mac_scan_with_dfs_config(psoc));
	if (status != QDF_STATUS_SUCCESS) {
		policy_mgr_err("policy_mgr_get_updated_scan_config failed %d",
			status);
		return;
	}

	status = policy_mgr_get_updated_fw_mode_config(psoc,
				&cfg.fw_mode_config, dbs, dfs);
	if (status != QDF_STATUS_SUCCESS) {
		policy_mgr_err("policy_mgr_get_updated_fw_mode_config failed %d",
			status);
		return;
	}

	cfg.set_dual_mac_cb = (void *)policy_mgr_soc_set_dual_mac_cfg_cb;

	policy_mgr_debug("scan_config:%x fw_mode_config:%x",
			cfg.scan_config, cfg.fw_mode_config);

	status = pm_ctx->sme_cbacks.sme_soc_set_dual_mac_config(cfg);
	if (status != QDF_STATUS_SUCCESS)
		policy_mgr_err("sme_soc_set_dual_mac_config failed %d", status);
}

bool policy_mgr_current_concurrency_is_mcc(struct wlan_objmgr_psoc *psoc)
{
	uint32_t num_connections = 0;
	bool is_mcc = false;

	num_connections = policy_mgr_get_connection_count(psoc);

	switch (num_connections) {
	case 1:
		break;
	case 2:
		if ((pm_conc_connection_list[0].chan !=
			pm_conc_connection_list[1].chan) &&
		    (pm_conc_connection_list[0].mac ==
			pm_conc_connection_list[1].mac)) {
			is_mcc = true;
		}
		break;
	case 3:
		if ((pm_conc_connection_list[0].chan !=
			pm_conc_connection_list[1].chan) ||
		    (pm_conc_connection_list[0].chan !=
			pm_conc_connection_list[2].chan) ||
		    (pm_conc_connection_list[1].chan !=
			pm_conc_connection_list[2].chan)){
				is_mcc = true;
		}
		break;
	default:
		policy_mgr_err("unexpected num_connections value %d",
			num_connections);
		break;
	}

	return is_mcc;
}

/**
 * policy_mgr_set_concurrency_mode() - To set concurrency mode
 * @psoc: PSOC object data
 * @mode: device mode
 *
 * This routine is called to set the concurrency mode
 *
 * Return: NONE
 */
void policy_mgr_set_concurrency_mode(struct wlan_objmgr_psoc *psoc,
				     enum QDF_OPMODE mode)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return;
	}

	switch (mode) {
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
	case QDF_P2P_GO_MODE:
	case QDF_SAP_MODE:
	case QDF_IBSS_MODE:
	case QDF_MONITOR_MODE:
		pm_ctx->concurrency_mode |= (1 << mode);
		pm_ctx->no_of_open_sessions[mode]++;
		break;
	default:
		break;
	}

	policy_mgr_info("concurrency_mode = 0x%x Number of open sessions for mode %d = %d",
		pm_ctx->concurrency_mode, mode,
		pm_ctx->no_of_open_sessions[mode]);
}

/**
 * policy_mgr_clear_concurrency_mode() - To clear concurrency mode
 * @psoc: PSOC object data
 * @mode: device mode
 *
 * This routine is called to clear the concurrency mode
 *
 * Return: NONE
 */
void policy_mgr_clear_concurrency_mode(struct wlan_objmgr_psoc *psoc,
				       enum QDF_OPMODE mode)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return;
	}

	switch (mode) {
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
	case QDF_P2P_GO_MODE:
	case QDF_SAP_MODE:
	case QDF_MONITOR_MODE:
		pm_ctx->no_of_open_sessions[mode]--;
		if (!(pm_ctx->no_of_open_sessions[mode]))
			pm_ctx->concurrency_mode &= (~(1 << mode));
		break;
	default:
		break;
	}

	policy_mgr_info("concurrency_mode = 0x%x Number of open sessions for mode %d = %d",
		pm_ctx->concurrency_mode, mode,
		pm_ctx->no_of_open_sessions[mode]);
}

void policy_mgr_incr_active_session(struct wlan_objmgr_psoc *psoc,
				enum QDF_OPMODE mode,
				uint8_t session_id)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	/*
	 * Need to aquire mutex as entire functionality in this function
	 * is in critical section
	 */
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	switch (mode) {
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
	case QDF_P2P_GO_MODE:
	case QDF_SAP_MODE:
	case QDF_IBSS_MODE:
		pm_ctx->no_of_active_sessions[mode]++;
		break;
	default:
		break;
	}


	policy_mgr_debug("No.# of active sessions for mode %d = %d",
		mode, pm_ctx->no_of_active_sessions[mode]);
	/*
	 * Get PCL logic makes use of the connection info structure.
	 * Let us set the PCL to the FW before updating the connection
	 * info structure about the new connection.
	 */
	if (mode == QDF_STA_MODE) {
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		/* Set PCL of STA to the FW */
		policy_mgr_pdev_set_pcl(psoc, mode);
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
		policy_mgr_debug("Set PCL of STA to FW");
	}
	policy_mgr_incr_connection_count(psoc, session_id);
	if ((policy_mgr_mode_specific_connection_count(
		psoc, PM_STA_MODE, NULL) > 0) && (mode != QDF_STA_MODE)) {
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		policy_mgr_set_pcl_for_existing_combo(psoc, PM_STA_MODE);
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	}

	/* Notify tdls */
	if (pm_ctx->tdls_cbacks.tdls_notify_increment_session)
		pm_ctx->tdls_cbacks.tdls_notify_increment_session(psoc);

	/*
	 * Disable LRO if P2P or IBSS or SAP connection has come up or
	 * there are more than one STA connections
	 */
	if ((policy_mgr_mode_specific_connection_count(psoc, PM_STA_MODE, NULL) > 1) ||
	    (policy_mgr_mode_specific_connection_count(psoc, PM_SAP_MODE, NULL) > 0) ||
	    (policy_mgr_mode_specific_connection_count(psoc, PM_P2P_CLIENT_MODE, NULL) >
									0) ||
	    (policy_mgr_mode_specific_connection_count(psoc, PM_P2P_GO_MODE, NULL) > 0) ||
	    (policy_mgr_mode_specific_connection_count(psoc, PM_IBSS_MODE, NULL) > 0)) {
		if (pm_ctx->dp_cbacks.hdd_disable_lro_in_concurrency != NULL)
			pm_ctx->dp_cbacks.hdd_disable_lro_in_concurrency(true);
	};

	/* Enable RPS if SAP interface has come up */
	if (policy_mgr_mode_specific_connection_count(psoc, PM_SAP_MODE, NULL)
		== 1) {
		if (pm_ctx->dp_cbacks.hdd_set_rx_mode_rps_cb != NULL)
			pm_ctx->dp_cbacks.hdd_set_rx_mode_rps_cb(true);
	}

	policy_mgr_dump_current_concurrency(psoc);

	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
}

QDF_STATUS policy_mgr_decr_active_session(struct wlan_objmgr_psoc *psoc,
				enum QDF_OPMODE mode,
				uint8_t session_id)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	QDF_STATUS qdf_status;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("context is NULL");
		return QDF_STATUS_E_EMPTY;
	}

	qdf_status = policy_mgr_check_conn_with_mode_and_vdev_id(psoc,
			policy_mgr_convert_device_mode_to_qdf_type(mode),
			session_id);
	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		policy_mgr_err("No connection with mode:%d vdev_id:%d",
			policy_mgr_convert_device_mode_to_qdf_type(mode),
			session_id);
		return qdf_status;
	}

	switch (mode) {
	case QDF_STA_MODE:
	case QDF_P2P_CLIENT_MODE:
	case QDF_P2P_GO_MODE:
	case QDF_SAP_MODE:
	case QDF_IBSS_MODE:
		if (pm_ctx->no_of_active_sessions[mode])
			pm_ctx->no_of_active_sessions[mode]--;
		break;
	default:
		break;
	}

	policy_mgr_debug("No.# of active sessions for mode %d = %d",
		mode, pm_ctx->no_of_active_sessions[mode]);

	policy_mgr_decr_connection_count(psoc, session_id);

	/* Notify tdls */
	if (pm_ctx->tdls_cbacks.tdls_notify_decrement_session)
		pm_ctx->tdls_cbacks.tdls_notify_decrement_session(psoc);
	/* Enable LRO if there no concurrency */
	if ((policy_mgr_mode_specific_connection_count(psoc, PM_STA_MODE, NULL) == 1) &&
	    (policy_mgr_mode_specific_connection_count(psoc, PM_SAP_MODE, NULL) == 0) &&
	    (policy_mgr_mode_specific_connection_count(psoc, PM_P2P_CLIENT_MODE, NULL) ==
									0) &&
	    (policy_mgr_mode_specific_connection_count(psoc, PM_P2P_GO_MODE, NULL) == 0) &&
	    (policy_mgr_mode_specific_connection_count(psoc, PM_IBSS_MODE, NULL) == 0)) {
		if (pm_ctx->dp_cbacks.hdd_disable_lro_in_concurrency != NULL)
			pm_ctx->dp_cbacks.hdd_disable_lro_in_concurrency(false);
	};

	/* Disable RPS if SAP interface has come up */
	if (policy_mgr_mode_specific_connection_count(psoc, PM_SAP_MODE, NULL)
		== 0) {
		if (pm_ctx->dp_cbacks.hdd_set_rx_mode_rps_cb != NULL)
			pm_ctx->dp_cbacks.hdd_set_rx_mode_rps_cb(false);
	}

	policy_mgr_dump_current_concurrency(psoc);

	return qdf_status;
}

QDF_STATUS policy_mgr_incr_connection_count(
		struct wlan_objmgr_psoc *psoc, uint32_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t conn_index;
	struct policy_mgr_vdev_entry_info conn_table_entry;
	enum policy_mgr_chain_mode chain_mask = POLICY_MGR_ONE_ONE;
	uint8_t nss_2g = 0, nss_5g = 0;
	enum policy_mgr_con_mode mode;
	uint8_t chan;
	uint32_t nss = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("context is NULL");
		return status;
	}

	conn_index = policy_mgr_get_connection_count(psoc);
	if (pm_ctx->user_cfg.max_concurrent_active_sessions < conn_index) {
		policy_mgr_err("exceeded max connection limit %d",
			pm_ctx->user_cfg.max_concurrent_active_sessions);
		return status;
	}
	if (pm_ctx->wma_cbacks.wma_get_connection_info) {
		status = pm_ctx->wma_cbacks.wma_get_connection_info(
				vdev_id, &conn_table_entry);
		if (QDF_STATUS_SUCCESS != status) {
			policy_mgr_err("can't find vdev_id %d in connection table",
			vdev_id);
			return status;
		}
	} else {
		policy_mgr_err("wma_get_connection_info is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mode = policy_mgr_get_mode(conn_table_entry.type,
					conn_table_entry.sub_type);
	chan = wlan_reg_freq_to_chan(pm_ctx->pdev, conn_table_entry.mhz);
	status = policy_mgr_get_nss_for_vdev(psoc, mode, &nss_2g, &nss_5g);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		if ((WLAN_REG_IS_24GHZ_CH(chan) && (nss_2g > 1)) ||
			(WLAN_REG_IS_5GHZ_CH(chan) && (nss_5g > 1)))
			chain_mask = POLICY_MGR_TWO_TWO;
		else
			chain_mask = POLICY_MGR_ONE_ONE;
		nss = (WLAN_REG_IS_24GHZ_CH(chan)) ? nss_2g : nss_5g;
	} else {
		policy_mgr_err("Error in getting nss");
	}


	/* add the entry */
	policy_mgr_update_conc_list(psoc, conn_index,
			mode,
			chan,
			policy_mgr_get_bw(conn_table_entry.chan_width),
			conn_table_entry.mac_id,
			chain_mask,
			nss, vdev_id, true);
	policy_mgr_debug("Add at idx:%d vdev %d mac=%d",
		conn_index, vdev_id,
		conn_table_entry.mac_id);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_decr_connection_count(struct wlan_objmgr_psoc *psoc,
					uint32_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t conn_index = 0, next_conn_index = 0;
	bool found = false;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
		if (vdev_id == pm_conc_connection_list[conn_index].vdev_id) {
			/* debug msg */
			found = true;
			break;
		}
		conn_index++;
	}
	if (!found) {
		policy_mgr_err("can't find vdev_id %d in pm_conc_connection_list",
			vdev_id);
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		return status;
	}
	next_conn_index = conn_index + 1;
	while (PM_CONC_CONNECTION_LIST_VALID_INDEX(next_conn_index)) {
		pm_conc_connection_list[conn_index].vdev_id =
			pm_conc_connection_list[next_conn_index].vdev_id;
		pm_conc_connection_list[conn_index].mode =
			pm_conc_connection_list[next_conn_index].mode;
		pm_conc_connection_list[conn_index].mac =
			pm_conc_connection_list[next_conn_index].mac;
		pm_conc_connection_list[conn_index].chan =
			pm_conc_connection_list[next_conn_index].chan;
		pm_conc_connection_list[conn_index].bw =
			pm_conc_connection_list[next_conn_index].bw;
		pm_conc_connection_list[conn_index].chain_mask =
			pm_conc_connection_list[next_conn_index].chain_mask;
		pm_conc_connection_list[conn_index].original_nss =
			pm_conc_connection_list[next_conn_index].original_nss;
		pm_conc_connection_list[conn_index].in_use =
			pm_conc_connection_list[next_conn_index].in_use;
		conn_index++;
		next_conn_index++;
	}

	/* clean up the entry */
	qdf_mem_zero(&pm_conc_connection_list[next_conn_index - 1],
		sizeof(*pm_conc_connection_list));
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return QDF_STATUS_SUCCESS;
}

bool policy_mgr_map_concurrency_mode(enum QDF_OPMODE *old_mode,
				     enum policy_mgr_con_mode *new_mode)
{
	bool status = true;

	switch (*old_mode) {

	case QDF_STA_MODE:
		*new_mode = PM_STA_MODE;
		break;
	case QDF_SAP_MODE:
		*new_mode = PM_SAP_MODE;
		break;
	case QDF_P2P_CLIENT_MODE:
		*new_mode = PM_P2P_CLIENT_MODE;
		break;
	case QDF_P2P_GO_MODE:
		*new_mode = PM_P2P_GO_MODE;
		break;
	case QDF_IBSS_MODE:
		*new_mode = PM_IBSS_MODE;
		break;
	default:
		*new_mode = PM_MAX_NUM_OF_MODE;
		status = false;
		break;
	}

	return status;
}

bool policy_mgr_is_ibss_conn_exist(struct wlan_objmgr_psoc *psoc,
				uint8_t *ibss_channel)
{
	uint32_t count = 0, index = 0;
	uint32_t list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	bool status = false;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}
	if (NULL == ibss_channel) {
		policy_mgr_err("Null pointer error");
		return false;
	}
	count = policy_mgr_mode_specific_connection_count(
				psoc, PM_IBSS_MODE, list);
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	if (count == 0) {
		/* No IBSS connection */
		status = false;
	} else if (count == 1) {
		*ibss_channel = pm_conc_connection_list[list[index]].chan;
		status = true;
	} else {
		*ibss_channel = pm_conc_connection_list[list[index]].chan;
		policy_mgr_debug("Multiple IBSS connections, picking first one");
		status = true;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return status;
}

uint32_t policy_mgr_get_mode_specific_conn_info(struct wlan_objmgr_psoc *psoc,
				  uint8_t *channel, uint8_t *vdev_id,
				  enum policy_mgr_con_mode mode)
{

	uint32_t count = 0, index = 0;
	uint32_t list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return count;
	}
	if (NULL == channel || NULL == vdev_id) {
		policy_mgr_err("Null pointer error");
		return count;
	}

	count = policy_mgr_mode_specific_connection_count(
				psoc, mode, list);
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	if (count == 0) {
		policy_mgr_debug("No mode:[%d] connection", mode);
	} else if (count == 1) {
		*channel = pm_conc_connection_list[list[index]].chan;
		*vdev_id =
			pm_conc_connection_list[list[index]].vdev_id;
	} else {
		for (index = 0; index < count; index++) {
			channel[index] =
			pm_conc_connection_list[list[index]].chan;

			vdev_id[index] =
			pm_conc_connection_list[list[index]].vdev_id;
		}
		policy_mgr_debug("Multiple mode:[%d] connections", mode);
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return count;
}

bool policy_mgr_max_concurrent_connections_reached(
		struct wlan_objmgr_psoc *psoc)
{
	uint8_t i = 0, j = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (NULL != pm_ctx) {
		for (i = 0; i < QDF_MAX_NO_OF_MODE; i++)
			j += pm_ctx->no_of_active_sessions[i];
		return j >
			(pm_ctx->user_cfg.
			 max_concurrent_active_sessions - 1);
	}

	return false;
}

static bool policy_mgr_is_sub_20_mhz_enabled(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	return pm_ctx->user_cfg.sub_20_mhz_enabled;
}

bool policy_mgr_allow_concurrency(struct wlan_objmgr_psoc *psoc,
				enum policy_mgr_con_mode mode,
				uint8_t channel, enum hw_mode_bandwidth bw)
{
	uint32_t num_connections = 0, count = 0, index = 0;
	bool status = false, match = false;
	uint32_t list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	QDF_STATUS ret;
	struct policy_mgr_pcl_list pcl;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}


	qdf_mem_zero(&pcl, sizeof(pcl));
	ret = policy_mgr_get_pcl(psoc, mode, pcl.pcl_list, &pcl.pcl_len,
			pcl.weight_list, QDF_ARRAY_SIZE(pcl.weight_list));
	if (QDF_IS_STATUS_ERROR(ret)) {
		policy_mgr_err("disallow connection:%d", ret);
		goto done;
	}

	/* find the current connection state from pm_conc_connection_list*/
	num_connections = policy_mgr_get_connection_count(psoc);

	if (num_connections && policy_mgr_is_sub_20_mhz_enabled(psoc)) {
		policy_mgr_err("dont allow concurrency if Sub 20 MHz is enabled");
		status = false;
		goto done;
	}

	if (policy_mgr_max_concurrent_connections_reached(psoc)) {
		policy_mgr_err("Reached max concurrent connections: %d",
			pm_ctx->user_cfg.max_concurrent_active_sessions);
		goto done;
	}

	if (channel) {
		/* don't allow 3rd home channel on same MAC */
		if (!policy_mgr_allow_new_home_channel(psoc,
			channel, num_connections))
			goto done;

		/*
		 * 1) DFS MCC is not yet supported
		 * 2) If you already have STA connection on 5G channel then
		 *    don't allow any other persona to make connection on DFS
		 *    channel because STA 5G + DFS MCC is not allowed.
		 * 3) If STA is on 2G channel and SAP is coming up on
		 *    DFS channel then allow concurrency but make sure it is
		 *    going to DBS and send PCL to firmware indicating that
		 *    don't allow STA to roam to 5G channels.
		 */
		if (!policy_mgr_is_5g_channel_allowed(psoc,
			channel, list, PM_P2P_GO_MODE))
			goto done;
		if (!policy_mgr_is_5g_channel_allowed(psoc,
			channel, list, PM_SAP_MODE))
			goto done;

		if ((PM_P2P_GO_MODE == mode) || (PM_SAP_MODE == mode)) {
			if (wlan_reg_is_dfs_ch(pm_ctx->pdev, channel))
				match = policy_mgr_disallow_mcc(psoc, channel);
		}
		if (true == match) {
			policy_mgr_err("No MCC, SAP/GO about to come up on DFS channel");
			goto done;
		}
	}

	/*
	 * Check all IBSS+STA concurrencies
	 *
	 * don't allow IBSS + STA MCC
	 * don't allow IBSS + STA SCC if IBSS is on DFS channel
	 */
	count = policy_mgr_mode_specific_connection_count(psoc,
				PM_STA_MODE, list);
	if ((PM_IBSS_MODE == mode) &&
		(policy_mgr_mode_specific_connection_count(psoc,
		PM_IBSS_MODE, list)) && count) {
		policy_mgr_err("No 2nd IBSS, we already have STA + IBSS");
		goto done;
	}
	if ((PM_IBSS_MODE == mode) &&
		(wlan_reg_is_dfs_ch(pm_ctx->pdev, channel)) && count) {
		policy_mgr_err("No IBSS + STA SCC/MCC, IBSS is on DFS channel");
		goto done;
	}
	if (PM_IBSS_MODE == mode) {
		if (policy_mgr_is_hw_dbs_capable(psoc) == true) {
			if (num_connections > 1) {
				policy_mgr_err("No IBSS, we have concurrent connections already");
				goto done;
			}
			qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
			if (PM_STA_MODE != pm_conc_connection_list[0].mode) {
				policy_mgr_err("No IBSS, we've a non-STA connection");
				qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
				goto done;
			}
			/*
			 * This logic protects STA and IBSS to come up on same
			 * band. If requirement changes then this condition
			 * needs to be removed
			 */
			if (channel &&
				(pm_conc_connection_list[0].chan != channel) &&
				WLAN_REG_IS_SAME_BAND_CHANNELS(
				pm_conc_connection_list[0].chan, channel)) {
				qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
				policy_mgr_err("No IBSS + STA MCC");
				goto done;
			}
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		} else if (num_connections) {
			policy_mgr_err("No IBSS, we have one connection already");
			goto done;
		}
	}

	if ((PM_STA_MODE == mode) &&
		(policy_mgr_mode_specific_connection_count(psoc,
		PM_IBSS_MODE, list)) && count) {
		policy_mgr_err("No 2nd STA, we already have STA + IBSS");
		goto done;
	}

	if ((PM_STA_MODE == mode) &&
		(policy_mgr_mode_specific_connection_count(psoc,
		PM_IBSS_MODE, list))) {
		if (policy_mgr_is_hw_dbs_capable(psoc) == true) {
			if (num_connections > 1) {
				policy_mgr_err("No 2nd STA, we already have IBSS concurrency");
				goto done;
			}
			qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
			if (channel &&
				(wlan_reg_is_dfs_ch(pm_ctx->pdev,
				pm_conc_connection_list[0].chan))
				&& (WLAN_REG_IS_5GHZ_CH(channel))) {
				qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
				policy_mgr_err("No IBSS + STA SCC/MCC, IBSS is on DFS channel");
				goto done;
			}
			/*
			 * This logic protects STA and IBSS to come up on same
			 * band. If requirement changes then this condition
			 * needs to be removed
			 */
			if ((pm_conc_connection_list[0].chan != channel) &&
				WLAN_REG_IS_SAME_BAND_CHANNELS(
				pm_conc_connection_list[0].chan, channel)) {
				policy_mgr_err("No IBSS + STA MCC");
				qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
				goto done;
			}
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		} else {
			policy_mgr_err("No STA, we have IBSS connection already");
			goto done;
		}
	}

	/* don't allow two P2P GO on same band */
	if (channel && (mode == PM_P2P_GO_MODE) && num_connections) {
		index = 0;
		count = policy_mgr_mode_specific_connection_count(psoc,
						PM_P2P_GO_MODE, list);
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
		while (index < count) {
			if (WLAN_REG_IS_SAME_BAND_CHANNELS(channel,
				pm_conc_connection_list[list[index]].chan)) {
				policy_mgr_err("Don't allow P2P GO on same band");
				qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
				goto done;
			}
			index++;
		}
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
	}

	status = true;

done:
	return status;
}

/**
 * policy_mgr_get_concurrency_mode() - return concurrency mode
 * @psoc: PSOC object information
 *
 * This routine is used to retrieve concurrency mode
 *
 * Return: uint32_t value of concurrency mask
 */
uint32_t policy_mgr_get_concurrency_mode(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return QDF_STA_MASK;
	}

	policy_mgr_info("concurrency_mode: 0x%x",
			pm_ctx->concurrency_mode);

	return pm_ctx->concurrency_mode;
}

/**
 * policy_mgr_get_channel_from_scan_result() - to get channel from scan result
 * @psoc: PSOC object information
 * @roam_profile: pointer to roam profile
 * @channel: channel to be filled
 *
 * This routine gets channel which most likely a candidate to which STA
 * will make connection.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS policy_mgr_get_channel_from_scan_result(
		struct wlan_objmgr_psoc *psoc,
		void *roam_profile, uint8_t *channel)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	void *scan_cache = NULL;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return QDF_STATUS_E_INVAL;
	}

	if (!roam_profile || !channel) {
		policy_mgr_err("Invalid input parameters");
		return QDF_STATUS_E_INVAL;
	}

	if (pm_ctx->sme_cbacks.sme_get_ap_channel_from_scan) {
		status = pm_ctx->sme_cbacks.sme_get_ap_channel_from_scan
			(roam_profile, &scan_cache, channel);
		if (status != QDF_STATUS_SUCCESS) {
			policy_mgr_err("Get AP channel failed");
			return status;
		}
	} else {
		policy_mgr_err("sme_get_ap_channel_from_scan_cache NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (pm_ctx->sme_cbacks.sme_scan_result_purge)
		status = pm_ctx->sme_cbacks.sme_scan_result_purge(scan_cache);
	else
		policy_mgr_err("sme_scan_result_purge NULL");

	return status;
}

QDF_STATUS policy_mgr_set_user_cfg(struct wlan_objmgr_psoc *psoc,
				struct policy_mgr_user_cfg *user_cfg)
{

	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return QDF_STATUS_E_FAILURE;
	}
	if (NULL == user_cfg) {
		policy_mgr_err("Invalid User Config");
		return QDF_STATUS_E_FAILURE;
	}

	pm_ctx->user_cfg = *user_cfg;

	pm_ctx->cur_conc_system_pref = pm_ctx->user_cfg.conc_system_pref;

	return QDF_STATUS_SUCCESS;
}

uint8_t policy_mgr_search_and_check_for_session_conc(
		struct wlan_objmgr_psoc *psoc,
		uint8_t session_id,
		void *roam_profile)
{
	uint8_t channel = 0;
	QDF_STATUS status;
	enum policy_mgr_con_mode mode;
	bool ret;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return channel;
	}

	if (pm_ctx->hdd_cbacks.get_mode_for_non_connected_vdev) {
		mode = pm_ctx->hdd_cbacks.get_mode_for_non_connected_vdev(
			psoc, session_id);
		if (PM_MAX_NUM_OF_MODE == mode) {
			policy_mgr_err("Invalid mode");
			return channel;
		}
	} else
		return channel;

	status = policy_mgr_get_channel_from_scan_result(psoc,
			roam_profile, &channel);
	if ((QDF_STATUS_SUCCESS != status) || (channel == 0)) {
		policy_mgr_err("%s error %d %d",
			__func__, status, channel);
		return 0;
	}

	/* Take care of 160MHz and 80+80Mhz later */
	ret = policy_mgr_allow_concurrency(psoc, mode, channel, HW_MODE_20_MHZ);
	if (false == ret) {
		policy_mgr_err("Connection failed due to conc check fail");
		return 0;
	}

	return channel;
}

/**
 * policy_mgr_is_two_connection_mcc() - Check if MCC scenario
 * when there are two connections
 *
 * If if MCC scenario when there are two connections
 *
 * Return: true or false
 */
static bool policy_mgr_is_two_connection_mcc(void)
{
	return ((pm_conc_connection_list[0].chan !=
		 pm_conc_connection_list[1].chan) &&
		(pm_conc_connection_list[0].mac ==
		 pm_conc_connection_list[1].mac) &&
		(pm_conc_connection_list[0].chan <=
		 WLAN_REG_MAX_24GHZ_CH_NUM) &&
		(pm_conc_connection_list[1].chan <=
		 WLAN_REG_MAX_24GHZ_CH_NUM)) ? true : false;
}

/**
 * policy_mgr_is_three_connection_mcc() - Check if MCC scenario
 * when there are three connections
 *
 * If if MCC scenario when there are three connections
 *
 * Return: true or false
 */
static bool policy_mgr_is_three_connection_mcc(void)
{
	return (((pm_conc_connection_list[0].chan !=
		  pm_conc_connection_list[1].chan) ||
		 (pm_conc_connection_list[0].chan !=
		  pm_conc_connection_list[2].chan) ||
		 (pm_conc_connection_list[1].chan !=
		  pm_conc_connection_list[2].chan)) &&
		(pm_conc_connection_list[0].chan <=
		 WLAN_REG_MAX_24GHZ_CH_NUM) &&
		(pm_conc_connection_list[1].chan <=
		 WLAN_REG_MAX_24GHZ_CH_NUM) &&
		(pm_conc_connection_list[2].chan <=
		 WLAN_REG_MAX_24GHZ_CH_NUM)) ? true : false;
}

bool policy_mgr_is_mcc_in_24G(struct wlan_objmgr_psoc *psoc)
{
	uint32_t num_connections = 0;
	bool is_24G_mcc = false;

	num_connections = policy_mgr_get_connection_count(psoc);

	switch (num_connections) {
	case 1:
		break;
	case 2:
		if (policy_mgr_is_two_connection_mcc())
			is_24G_mcc = true;
		break;
	case 3:
		if (policy_mgr_is_three_connection_mcc())
			is_24G_mcc = true;
		break;
	default:
		policy_mgr_err("unexpected num_connections value %d",
			num_connections);
		break;
	}

	return is_24G_mcc;
}

bool policy_mgr_check_for_session_conc(struct wlan_objmgr_psoc *psoc,
				uint8_t session_id, uint8_t channel)
{
	enum policy_mgr_con_mode mode;
	bool ret;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	if (pm_ctx->hdd_cbacks.get_mode_for_non_connected_vdev) {
		mode = pm_ctx->hdd_cbacks.get_mode_for_non_connected_vdev(
			psoc, session_id);
		if (PM_MAX_NUM_OF_MODE == mode) {
			policy_mgr_err("Invalid mode");
			return false;
		}
	} else
		return false;

	if (channel == 0) {
		policy_mgr_err("Invalid channel number 0");
		return false;
	}

	/* Take care of 160MHz and 80+80Mhz later */
	ret = policy_mgr_allow_concurrency(psoc, mode, channel, HW_MODE_20_MHZ);
	if (false == ret) {
		policy_mgr_err("Connection failed due to conc check fail");
		return 0;
	}

	return true;
}

bool policy_mgr_is_mcc_adaptive_scheduler_enabled(
	struct wlan_objmgr_psoc *psoc) {
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return false;
	}

	return pm_ctx->user_cfg.enable_mcc_adaptive_scheduler ?
		true : false;
}

/**
 * policy_mgr_change_mcc_go_beacon_interval() - Change MCC beacon interval
 * @psoc: PSOC object information
 * @vdev_id: vdev id
 * @dev_mode: device mode
 *
 * Updates the beacon parameters of the GO in MCC scenario
 *
 * Return: Success or Failure depending on the overall function behavior
 */
QDF_STATUS policy_mgr_change_mcc_go_beacon_interval(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id, enum QDF_OPMODE dev_mode)
{
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return QDF_STATUS_E_FAILURE;
	}

	policy_mgr_info("UPDATE Beacon Params");

	if (QDF_SAP_MODE == dev_mode) {
		if (pm_ctx->sme_cbacks.sme_change_mcc_beacon_interval
		    ) {
			status = pm_ctx->sme_cbacks.
				sme_change_mcc_beacon_interval(vdev_id);
			if (status == QDF_STATUS_E_FAILURE) {
				policy_mgr_err("Failed to update Beacon Params");
				return QDF_STATUS_E_FAILURE;
			}
		} else {
			policy_mgr_err("sme_change_mcc_beacon_interval callback is NULL");
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}

struct policy_mgr_conc_connection_info *policy_mgr_get_conn_info(uint32_t *len)
{
	struct policy_mgr_conc_connection_info *conn_ptr =
		&pm_conc_connection_list[0];
	*len = MAX_NUMBER_OF_CONC_CONNECTIONS;

	return conn_ptr;
}

enum policy_mgr_con_mode policy_mgr_convert_device_mode_to_qdf_type(
			enum QDF_OPMODE device_mode)
{
	enum policy_mgr_con_mode mode = PM_MAX_NUM_OF_MODE;
	switch (device_mode) {
	case QDF_STA_MODE:
		mode = PM_STA_MODE;
		break;
	case QDF_P2P_CLIENT_MODE:
		mode = PM_P2P_CLIENT_MODE;
		break;
	case QDF_P2P_GO_MODE:
		mode = PM_P2P_GO_MODE;
		break;
	case QDF_SAP_MODE:
		mode = PM_SAP_MODE;
		break;
	case QDF_IBSS_MODE:
		mode = PM_IBSS_MODE;
		break;
	default:
		policy_mgr_err("Unsupported mode (%d)",
			device_mode);
	}

	return mode;
}

QDF_STATUS policy_mgr_mode_specific_num_open_sessions(
		struct wlan_objmgr_psoc *psoc, enum QDF_OPMODE mode,
		uint8_t *num_sessions)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return QDF_STATUS_E_FAILURE;
	}

	*num_sessions = pm_ctx->no_of_open_sessions[mode];
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_mode_specific_num_active_sessions(
		struct wlan_objmgr_psoc *psoc, enum QDF_OPMODE mode,
		uint8_t *num_sessions)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return QDF_STATUS_E_FAILURE;
	}

	*num_sessions = pm_ctx->no_of_active_sessions[mode];
	return QDF_STATUS_SUCCESS;
}

/**
 * policy_mgr_concurrent_open_sessions_running() - Checks for
 * concurrent open session
 * @psoc: PSOC object information
 *
 * Checks if more than one open session is running for all the allowed modes
 * in the driver
 *
 * Return: True if more than one open session exists, False otherwise
 */
bool policy_mgr_concurrent_open_sessions_running(
	struct wlan_objmgr_psoc *psoc)
{
	uint8_t i = 0;
	uint8_t j = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return false;
	}

	for (i = 0; i < QDF_MAX_NO_OF_MODE; i++)
		j += pm_ctx->no_of_open_sessions[i];

	return j > 1;
}

/**
 * policy_mgr_concurrent_beaconing_sessions_running() - Checks
 * for concurrent beaconing entities
 * @psoc: PSOC object information
 *
 * Checks if multiple beaconing sessions are running i.e., if SAP or GO or IBSS
 * are beaconing together
 *
 * Return: True if multiple entities are beaconing together, False otherwise
 */
bool policy_mgr_concurrent_beaconing_sessions_running(
	struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid context");
		return false;
	}

	return (pm_ctx->no_of_open_sessions[QDF_SAP_MODE] +
		pm_ctx->no_of_open_sessions[QDF_P2P_GO_MODE] +
		pm_ctx->no_of_open_sessions[QDF_IBSS_MODE] > 1) ? true : false;
}


void policy_mgr_clear_concurrent_session_count(struct wlan_objmgr_psoc *psoc)
{
	uint8_t i = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (NULL != pm_ctx) {
		for (i = 0; i < QDF_MAX_NO_OF_MODE; i++)
			pm_ctx->no_of_active_sessions[i] = 0;
	}
}

bool policy_mgr_is_multiple_active_sta_sessions(struct wlan_objmgr_psoc *psoc)
{
	return policy_mgr_mode_specific_connection_count(
		psoc, PM_STA_MODE, NULL) > 1;
}

/**
 * policy_mgr_is_sta_active_connection_exists() - Check if a STA
 * connection is active
 * @psoc: PSOC object information
 *
 * Checks if there is atleast one active STA connection in the driver
 *
 * Return: True if an active STA session is present, False otherwise
 */
bool policy_mgr_is_sta_active_connection_exists(
	struct wlan_objmgr_psoc *psoc)
{
	return (!policy_mgr_mode_specific_connection_count(
		psoc, QDF_STA_MODE, NULL)) ? false : true;
}

bool policy_mgr_is_any_nondfs_chnl_present(struct wlan_objmgr_psoc *psoc,
				uint8_t *channel)
{
	bool status = false;
	uint32_t conn_index = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
			conn_index++) {
		if (pm_conc_connection_list[conn_index].in_use &&
		    !wlan_reg_is_dfs_ch(pm_ctx->pdev,
			pm_conc_connection_list[conn_index].chan)) {
			*channel = pm_conc_connection_list[conn_index].chan;
			status = true;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return status;
}

bool policy_mgr_is_any_dfs_beaconing_session_present(
		struct wlan_objmgr_psoc *psoc, uint8_t *channel)
{
	struct policy_mgr_conc_connection_info *conn_info;
	bool status = false;
	uint32_t conn_index = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
			conn_index++) {
		conn_info = &pm_conc_connection_list[conn_index];
		if (conn_info->in_use &&
			wlan_reg_is_dfs_ch(pm_ctx->pdev, conn_info->chan) &&
		    (PM_SAP_MODE == conn_info->mode ||
		     PM_P2P_GO_MODE == conn_info->mode)) {
			*channel = pm_conc_connection_list[conn_index].chan;
			status = true;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return status;
}

QDF_STATUS policy_mgr_get_nss_for_vdev(struct wlan_objmgr_psoc *psoc,
				enum policy_mgr_con_mode mode,
				uint8_t *nss_2g, uint8_t *nss_5g)
{
	enum QDF_OPMODE dev_mode;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	switch (mode) {
	case PM_STA_MODE:
		dev_mode = QDF_STA_MODE;
		break;
	case PM_SAP_MODE:
		dev_mode = QDF_SAP_MODE;
		break;
	case PM_P2P_CLIENT_MODE:
		dev_mode = QDF_P2P_CLIENT_MODE;
		break;
	case PM_P2P_GO_MODE:
		dev_mode = QDF_P2P_GO_MODE;
		break;
	case PM_IBSS_MODE:
		dev_mode = QDF_IBSS_MODE;
		break;
	default:
		policy_mgr_err("Invalid mode to get allowed NSS value");
		return QDF_STATUS_E_FAILURE;
	};

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	if (pm_ctx->sme_cbacks.sme_get_nss_for_vdev) {
		pm_ctx->sme_cbacks.sme_get_nss_for_vdev(
			dev_mode, nss_2g, nss_5g);

	} else {
		policy_mgr_err("sme_get_nss_for_vdev callback is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

void policy_mgr_dump_connection_status_info(struct wlan_objmgr_psoc *psoc)
{
	uint32_t i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (i = 0; i < MAX_NUMBER_OF_CONC_CONNECTIONS; i++) {
		policy_mgr_debug("%d: use:%d vdev:%d mode:%d mac:%d chan:%d orig chainmask:%d orig nss:%d bw:%d",
				i, pm_conc_connection_list[i].in_use,
				pm_conc_connection_list[i].vdev_id,
				pm_conc_connection_list[i].mode,
				pm_conc_connection_list[i].mac,
				pm_conc_connection_list[i].chan,
				pm_conc_connection_list[i].chain_mask,
				pm_conc_connection_list[i].original_nss,
				pm_conc_connection_list[i].bw);
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
}

bool policy_mgr_is_any_mode_active_on_band_along_with_session(
						struct wlan_objmgr_psoc *psoc,
						uint8_t session_id,
						enum policy_mgr_band band)
{
	uint32_t i;
	bool status = false;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		status = false;
		goto send_status;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (i = 0; i < MAX_NUMBER_OF_CONC_CONNECTIONS; i++) {
		switch (band) {
		case POLICY_MGR_BAND_24:
			if ((pm_conc_connection_list[i].vdev_id != session_id)
			&& (pm_conc_connection_list[i].in_use) &&
			(WLAN_REG_IS_24GHZ_CH(
			pm_conc_connection_list[i].chan))) {
				status = true;
				goto release_mutex_and_send_status;
			}
			break;
		case POLICY_MGR_BAND_5:
			if ((pm_conc_connection_list[i].vdev_id != session_id)
			&& (pm_conc_connection_list[i].in_use) &&
			(WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[i].chan))) {
				status = true;
				goto release_mutex_and_send_status;
			}
			break;
		default:
			policy_mgr_err("Invalidband option:%d", band);
			status = false;
			goto release_mutex_and_send_status;
		}
	}
release_mutex_and_send_status:
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
send_status:
	return status;
}

QDF_STATUS policy_mgr_get_chan_by_session_id(struct wlan_objmgr_psoc *psoc,
					uint8_t session_id, uint8_t *chan)
{
	uint32_t i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (i = 0; i < MAX_NUMBER_OF_CONC_CONNECTIONS; i++) {
		if ((pm_conc_connection_list[i].vdev_id == session_id) &&
		    (pm_conc_connection_list[i].in_use)) {
			*chan = pm_conc_connection_list[i].chan;
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
			return QDF_STATUS_SUCCESS;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS policy_mgr_get_mac_id_by_session_id(struct wlan_objmgr_psoc *psoc,
					uint8_t session_id, uint8_t *mac_id)
{
	uint32_t i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (i = 0; i < MAX_NUMBER_OF_CONC_CONNECTIONS; i++) {
		if ((pm_conc_connection_list[i].vdev_id == session_id) &&
		    (pm_conc_connection_list[i].in_use)) {
			*mac_id = pm_conc_connection_list[i].mac;
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
			return QDF_STATUS_SUCCESS;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS policy_mgr_get_mcc_session_id_on_mac(struct wlan_objmgr_psoc *psoc,
					uint8_t mac_id, uint8_t session_id,
					uint8_t *mcc_session_id)
{
	uint32_t i;
	QDF_STATUS status;
	uint8_t chan;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	status = policy_mgr_get_chan_by_session_id(psoc, session_id, &chan);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("Failed to get channel for session id:%d",
			       session_id);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (i = 0; i < MAX_NUMBER_OF_CONC_CONNECTIONS; i++) {
		if (pm_conc_connection_list[i].mac != mac_id)
			continue;
		if (pm_conc_connection_list[i].vdev_id == session_id)
			continue;
		/* Inter band or intra band MCC */
		if ((pm_conc_connection_list[i].chan != chan) &&
		    (pm_conc_connection_list[i].in_use)) {
			*mcc_session_id = pm_conc_connection_list[i].vdev_id;
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
			return QDF_STATUS_SUCCESS;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return QDF_STATUS_E_FAILURE;
}

uint8_t policy_mgr_get_mcc_operating_channel(struct wlan_objmgr_psoc *psoc,
					uint8_t session_id)
{
	uint8_t mac_id, mcc_session_id;
	QDF_STATUS status;
	uint8_t chan;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return INVALID_CHANNEL_ID;
	}

	status = policy_mgr_get_mac_id_by_session_id(psoc, session_id, &mac_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("failed to get MAC ID");
		return INVALID_CHANNEL_ID;
	}

	status = policy_mgr_get_mcc_session_id_on_mac(psoc, mac_id, session_id,
			&mcc_session_id);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("failed to get MCC session ID");
		return INVALID_CHANNEL_ID;
	}

	status = policy_mgr_get_chan_by_session_id(psoc, mcc_session_id,
						   &chan);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("Failed to get channel for MCC session ID:%d",
			       mcc_session_id);
		return INVALID_CHANNEL_ID;
	}

	return chan;
}

void policy_mgr_set_do_hw_mode_change_flag(struct wlan_objmgr_psoc *psoc,
		bool flag)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	pm_ctx->do_hw_mode_change = flag;
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	policy_mgr_debug("hw_mode_change_channel:%d", flag);
}

bool policy_mgr_is_hw_mode_change_after_vdev_up(struct wlan_objmgr_psoc *psoc)
{
	bool flag;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return INVALID_CHANNEL_ID;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	flag = pm_ctx->do_hw_mode_change;
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return flag;
}

bool policy_mgr_is_dnsc_set(struct wlan_objmgr_vdev *vdev)
{
	bool roffchan;

	if (!vdev) {
		policy_mgr_err("Invalid parameter");
		return false;
	}

	roffchan = wlan_vdev_mlme_cap_get(vdev, WLAN_VDEV_C_RESTRICT_OFFCHAN);

	policy_mgr_debug("Restrict offchannel:%s",
			 roffchan  ? "set" : "clear");

	return roffchan;
}

QDF_STATUS policy_mgr_is_chan_ok_for_dnbs(struct wlan_objmgr_psoc *psoc,
			uint8_t channel, bool *ok)
{
	uint32_t cc_count = 0, i;
	uint8_t operating_channel[MAX_NUMBER_OF_CONC_CONNECTIONS];
	uint8_t vdev_id[MAX_NUMBER_OF_CONC_CONNECTIONS];
	struct wlan_objmgr_vdev *vdev;

	if (!channel || !ok) {
		policy_mgr_err("Invalid parameter");
		return QDF_STATUS_E_INVAL;
	}

	cc_count = policy_mgr_get_mode_specific_conn_info(psoc,
					&operating_channel[cc_count],
					&vdev_id[cc_count],
					PM_SAP_MODE);
	policy_mgr_debug("Number of SAP modes: %d", cc_count);
	cc_count = cc_count + policy_mgr_get_mode_specific_conn_info(psoc,
					&operating_channel[cc_count],
					&vdev_id[cc_count],
					PM_P2P_GO_MODE);
	policy_mgr_debug("Number of beaconing entities (SAP + GO):%d",
							cc_count);
	if (!cc_count) {
		*ok = true;
		return QDF_STATUS_SUCCESS;
	}

	for (i = 0; i < cc_count; i++) {
		vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
							vdev_id[i],
						WLAN_POLICY_MGR_ID);
		if (!vdev) {
			policy_mgr_err("vdev for vdev_id:%d is NULL",
							vdev_id[i]);
			return QDF_STATUS_E_INVAL;
		}

	/**
	 * If channel passed is same as AP/GO operating channel, then
	 *   return true.
	 * If channel is different from operating channel but in same band.
	 *   return false.
	 * If operating channel in different band (DBS capable).
	 *   return true.
	 * If operating channel in different band (not DBS capable).
	 *   return false.
	 */
	/* TODO: To be enhanced for SBS */
		if (policy_mgr_is_dnsc_set(vdev)) {
			if (operating_channel[i] == channel) {
				*ok = true;
				wlan_objmgr_vdev_release_ref(vdev,
						WLAN_POLICY_MGR_ID);
				break;
			} else if (WLAN_REG_IS_SAME_BAND_CHANNELS(
				operating_channel[i], channel)) {
				*ok = false;
				wlan_objmgr_vdev_release_ref(vdev,
						WLAN_POLICY_MGR_ID);
				break;
			} else if (policy_mgr_is_hw_dbs_capable(psoc)) {
				*ok = true;
				wlan_objmgr_vdev_release_ref(vdev,
						WLAN_POLICY_MGR_ID);
				break;
			} else {
				*ok = false;
				wlan_objmgr_vdev_release_ref(vdev,
						WLAN_POLICY_MGR_ID);
				break;
			}
		} else {
			*ok = true;
		}
		wlan_objmgr_vdev_release_ref(vdev, WLAN_POLICY_MGR_ID);
	}
	policy_mgr_debug("chan: %d ok %d", channel, *ok);

	return QDF_STATUS_SUCCESS;
}

uint32_t policy_mgr_get_hw_dbs_nss(struct wlan_objmgr_psoc *psoc,
				   struct dbs_nss *nss_dbs)
{
	int i, param;
	uint32_t dbs, tx_chain0, rx_chain0, tx_chain1, rx_chain1;
	uint32_t min_mac0_rf_chains, min_mac1_rf_chains;
	uint32_t max_rf_chains, final_max_rf_chains = HW_MODE_SS_0x0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return final_max_rf_chains;
	}

	for (i = 0; i < pm_ctx->num_dbs_hw_modes; i++) {
		param = pm_ctx->hw_mode.hw_mode_list[i];
		dbs = POLICY_MGR_HW_MODE_DBS_MODE_GET(param);

		if (dbs) {
			tx_chain0
				= POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_GET(param);
			rx_chain0
				= POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_GET(param);

			tx_chain1
				= POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_GET(param);
			rx_chain1
				= POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_GET(param);

			min_mac0_rf_chains = QDF_MIN(tx_chain0, rx_chain0);
			min_mac1_rf_chains = QDF_MIN(tx_chain1, rx_chain1);

			max_rf_chains
			= QDF_MAX(min_mac0_rf_chains, min_mac1_rf_chains);

			if (final_max_rf_chains < max_rf_chains) {
				final_max_rf_chains
					= (max_rf_chains == 2)
					? HW_MODE_SS_2x2 : HW_MODE_SS_1x1;

				nss_dbs->mac0_ss
					= (min_mac0_rf_chains == 2)
					? HW_MODE_SS_2x2 : HW_MODE_SS_1x1;

				nss_dbs->mac1_ss
					= (min_mac1_rf_chains == 2)
					? HW_MODE_SS_2x2 : HW_MODE_SS_1x1;
			}
		} else {
			continue;
		}
	}

	return final_max_rf_chains;
}

bool policy_mgr_is_scan_simultaneous_capable(struct wlan_objmgr_psoc *psoc)
{
	if (DISABLE_DBS_CXN_AND_SCAN !=
			wlan_objmgr_psoc_get_dual_mac_disable(psoc))
		return true;

	return false;
}

void policy_mgr_set_cur_conc_system_pref(struct wlan_objmgr_psoc *psoc,
		uint8_t conc_system_pref)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);

	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	policy_mgr_debug("conc_system_pref %hu", conc_system_pref);
	pm_ctx->cur_conc_system_pref = conc_system_pref;
}

uint8_t policy_mgr_get_cur_conc_system_pref(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return PM_THROUGHPUT;
	}

	policy_mgr_debug("conc_system_pref %hu", pm_ctx->cur_conc_system_pref);
	return pm_ctx->cur_conc_system_pref;
}

QDF_STATUS policy_mgr_get_updated_scan_and_fw_mode_config(
		struct wlan_objmgr_psoc *psoc, uint32_t *scan_config,
		uint32_t *fw_mode_config, uint32_t dual_mac_disable_ini,
		uint32_t channel_select_logic_conc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	*scan_config = pm_ctx->dual_mac_cfg.cur_scan_config;
	*fw_mode_config = pm_ctx->dual_mac_cfg.cur_fw_mode_config;
	switch (dual_mac_disable_ini) {
	case DISABLE_DBS_CXN_AND_ENABLE_DBS_SCAN_WITH_ASYNC_SCAN_OFF:
		policy_mgr_debug("dual_mac_disable_ini:%d async/dbs off",
			dual_mac_disable_ini);
		WMI_DBS_CONC_SCAN_CFG_ASYNC_DBS_SCAN_SET(*scan_config, 0);
		WMI_DBS_FW_MODE_CFG_DBS_FOR_CXN_SET(*fw_mode_config, 0);
		break;
	case DISABLE_DBS_CXN_AND_ENABLE_DBS_SCAN:
		policy_mgr_debug("dual_mac_disable_ini:%d dbs_cxn off",
			dual_mac_disable_ini);
		WMI_DBS_FW_MODE_CFG_DBS_FOR_CXN_SET(*fw_mode_config, 0);
		break;
	case ENABLE_DBS_CXN_AND_ENABLE_SCAN_WITH_ASYNC_SCAN_OFF:
		policy_mgr_debug("dual_mac_disable_ini:%d async off",
			dual_mac_disable_ini);
		WMI_DBS_CONC_SCAN_CFG_ASYNC_DBS_SCAN_SET(*scan_config, 0);
		break;
	default:
		break;
	}

	WMI_DBS_FW_MODE_CFG_DBS_FOR_STA_PLUS_STA_SET(*fw_mode_config,
		PM_CHANNEL_SELECT_LOGIC_STA_STA_GET(channel_select_logic_conc));
	WMI_DBS_FW_MODE_CFG_DBS_FOR_STA_PLUS_P2P_SET(*fw_mode_config,
		PM_CHANNEL_SELECT_LOGIC_STA_P2P_GET(channel_select_logic_conc));

	policy_mgr_debug("*scan_config:%x ", *scan_config);
	policy_mgr_debug("*fw_mode_config:%x ", *fw_mode_config);

	return QDF_STATUS_SUCCESS;
}

bool policy_mgr_is_force_scc(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return 0;
	}

	return ((pm_ctx->user_cfg.mcc_to_scc_switch_mode ==
		QDF_MCC_TO_SCC_SWITCH_FORCE_WITHOUT_DISCONNECTION) ||
		(pm_ctx->user_cfg.mcc_to_scc_switch_mode ==
		QDF_MCC_TO_SCC_SWITCH_WITH_FAVORITE_CHANNEL) ||
		(pm_ctx->user_cfg.mcc_to_scc_switch_mode ==
		QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION));
}

bool policy_mgr_is_sta_sap_scc_allowed_on_dfs_chan(
		struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	bool status = false;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}

	if (policy_mgr_is_force_scc(psoc) &&
		pm_ctx->user_cfg.is_sta_sap_scc_allowed_on_dfs_chan)
		status = true;

	return status;
}

bool policy_mgr_is_sta_connected_2g(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t conn_index;
	bool ret = false;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return ret;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
	     conn_index++) {
		if (pm_conc_connection_list[conn_index].mode == PM_STA_MODE &&
		    pm_conc_connection_list[conn_index].chan <= 14 &&
		    pm_conc_connection_list[conn_index].in_use)
			ret = true;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return ret;
}

void policy_mgr_trim_acs_channel_list(struct wlan_objmgr_psoc *psoc,
		uint8_t *org_ch_list, uint8_t *org_ch_list_count)
{
	uint32_t list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	uint32_t index, count, i, ch_list_count;
	uint8_t band_mask = 0, ch_5g = 0, ch_24g = 0;
	uint8_t ch_list[QDF_MAX_NUM_CHAN];
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	if (*org_ch_list_count >= QDF_MAX_NUM_CHAN) {
		policy_mgr_err("org_ch_list_count too big %d",
			*org_ch_list_count);
		return;
	}
	/*
	 * if force SCC is enabled and there is a STA connection, trim the
	 * ACS channel list on the band on which STA connection is present
	 */
	count = policy_mgr_mode_specific_connection_count(
				psoc, PM_STA_MODE, list);
	if (policy_mgr_is_force_scc(psoc) && count) {
		index = 0;
		while (index < count) {
			if (WLAN_REG_IS_24GHZ_CH(
				pm_conc_connection_list[list[index]].chan) &&
				policy_mgr_is_safe_channel(psoc,
				pm_conc_connection_list[list[index]].chan)) {
				band_mask |= 1;
				ch_24g = pm_conc_connection_list[list[index]].chan;
			}
			if (WLAN_REG_IS_5GHZ_CH(
				pm_conc_connection_list[list[index]].chan) &&
				policy_mgr_is_safe_channel(psoc,
				pm_conc_connection_list[list[index]].chan) &&
				!wlan_reg_is_dfs_ch(pm_ctx->pdev,
				pm_conc_connection_list[list[index]].chan) &&
				!wlan_reg_is_passive_or_disable_ch(pm_ctx->pdev,
				pm_conc_connection_list[list[index]].chan)) {
				band_mask |= 2;
				ch_5g = pm_conc_connection_list[list[index]].chan;
			}
			index++;
		}
		ch_list_count = 0;
		if (band_mask == 1) {
			ch_list[ch_list_count++] = ch_24g;
			for (i = 0; i < *org_ch_list_count; i++) {
				if (WLAN_REG_IS_24GHZ_CH(
					org_ch_list[i]))
					continue;
				ch_list[ch_list_count++] =
					org_ch_list[i];
			}
		} else if (band_mask == 2) {
			ch_list[ch_list_count++] = ch_5g;
			for (i = 0; i < *org_ch_list_count; i++) {
				if (WLAN_REG_IS_5GHZ_CH(
					org_ch_list[i]))
					continue;
				ch_list[ch_list_count++] =
					org_ch_list[i];
			}
		} else if (band_mask == 3) {
			ch_list[ch_list_count++] = ch_24g;
			ch_list[ch_list_count++] = ch_5g;
		} else {
			policy_mgr_debug("unexpected band_mask value %d",
				band_mask);
			return;
		}

		*org_ch_list_count = ch_list_count;
		for (i = 0; i < *org_ch_list_count; i++)
			org_ch_list[i] = ch_list[i];
	}
}

uint32_t policy_mgr_get_connection_info(struct wlan_objmgr_psoc *psoc,
					struct connection_info *info)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t conn_index, count = 0;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return count;
	}

	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
	     conn_index++) {
		if (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			info[count].vdev_id =
				pm_conc_connection_list[conn_index].vdev_id;
			info[count].mac_id =
				pm_conc_connection_list[conn_index].mac;
			info[count].channel =
				pm_conc_connection_list[conn_index].chan;
			count++;
		}
	}

	return count;
}
