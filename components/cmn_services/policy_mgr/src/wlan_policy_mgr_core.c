/*
 * Copyright (c) 2012-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_policy_mgr_core.c
 *
 * WLAN Concurrenct Connection Management functions
 *
 */

/* Include files */

#include "wlan_policy_mgr_i.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "wlan_objmgr_global_obj.h"
#include "wlan_mlme_api.h"
#include "wlan_cm_roam_api.h"
#include "wlan_mlme_ucfg_api.h"
#include "wlan_cm_api.h"
#include "wlan_reg_ucfg_api.h"
#ifdef WLAN_FEATURE_11BE_MLO
#include "wlan_mlo_mgr_cmn.h"
#include "wlan_mlo_mgr_public_structs.h"
#endif
#include "wlan_cm_ucfg_api.h"

#define POLICY_MGR_MAX_CON_STRING_LEN   100
#define LOWER_END_FREQ_5GHZ 4900

static const uint16_t sap_mand_5g_freq_list[] = {5745, 5765, 5785, 5805};

struct policy_mgr_conc_connection_info
	pm_conc_connection_list[MAX_NUMBER_OF_CONC_CONNECTIONS];

struct policy_mgr_psoc_priv_obj *policy_mgr_get_context(
		struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	pm_ctx = (struct policy_mgr_psoc_priv_obj *)
			wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_POLICY_MGR);
	return pm_ctx;
}

/**
 * policy_mgr_get_updated_scan_config() - Get the updated scan configuration
 * @scan_config: Pointer containing the updated scan config
 * @dbs_scan: 0 or 1 indicating if DBS scan needs to be enabled/disabled
 * @dbs_plus_agile_scan: 0 or 1 indicating if DBS plus agile scan needs to be
 * enabled/disabled
 * @single_mac_scan_with_dfs: 0 or 1 indicating if single MAC scan with DFS
 * needs to be enabled/disabled
 *
 * Takes the current scan configuration and set the necessary scan config
 * bits to either 0/1 and provides the updated value to the caller who
 * can use this to pass it on to the FW
 *
 * Return: 0 on success
 */
QDF_STATUS policy_mgr_get_updated_scan_config(
		struct wlan_objmgr_psoc *psoc,
		uint32_t *scan_config,
		bool dbs_scan,
		bool dbs_plus_agile_scan,
		bool single_mac_scan_with_dfs)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}
	*scan_config = pm_ctx->dual_mac_cfg.cur_scan_config;

	WMI_DBS_CONC_SCAN_CFG_DBS_SCAN_SET(*scan_config, dbs_scan);
	WMI_DBS_CONC_SCAN_CFG_AGILE_SCAN_SET(*scan_config,
			dbs_plus_agile_scan);
	WMI_DBS_CONC_SCAN_CFG_AGILE_DFS_SCAN_SET(*scan_config,
			single_mac_scan_with_dfs);

	policy_mgr_debug("scan_config:%x ", *scan_config);
	return QDF_STATUS_SUCCESS;
}

/**
 * policy_mgr_get_updated_fw_mode_config() - Get the updated fw
 * mode configuration
 * @fw_mode_config: Pointer containing the updated fw mode config
 * @dbs: 0 or 1 indicating if DBS needs to be enabled/disabled
 * @agile_dfs: 0 or 1 indicating if agile DFS needs to be enabled/disabled
 *
 * Takes the current fw mode configuration and set the necessary fw mode config
 * bits to either 0/1 and provides the updated value to the caller who
 * can use this to pass it on to the FW
 *
 * Return: 0 on success
 */
QDF_STATUS policy_mgr_get_updated_fw_mode_config(
		struct wlan_objmgr_psoc *psoc,
		uint32_t *fw_mode_config,
		bool dbs,
		bool agile_dfs)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}
	*fw_mode_config = pm_ctx->dual_mac_cfg.cur_fw_mode_config;

	WMI_DBS_FW_MODE_CFG_DBS_SET(*fw_mode_config, dbs);
	WMI_DBS_FW_MODE_CFG_AGILE_DFS_SET(*fw_mode_config, agile_dfs);

	policy_mgr_debug("fw_mode_config:%x ", *fw_mode_config);
	return QDF_STATUS_SUCCESS;
}

/**
 * policy_mgr_is_dual_mac_disabled_in_ini() - Check if dual mac
 * is disabled in INI
 *
 * Checks if the dual mac feature is disabled in INI
 *
 * Return: true if the dual mac connection is disabled from INI
 */
bool policy_mgr_is_dual_mac_disabled_in_ini(
		struct wlan_objmgr_psoc *psoc)
{
	bool is_disabled = false;
	uint8_t dbs_type = DISABLE_DBS_CXN_AND_SCAN;

	policy_mgr_get_dual_mac_feature(psoc, &dbs_type);
	/*
	 * If DBS support for connection is disabled through INI then assume
	 * that DBS is not supported, so that policy manager takes
	 * the decision considering non-dbs cases only.
	 *
	 * For DBS scan check the INI value explicitly
	 */
	switch (dbs_type) {
	case DISABLE_DBS_CXN_AND_SCAN:
	case DISABLE_DBS_CXN_AND_ENABLE_DBS_SCAN:
	case DISABLE_DBS_CXN_AND_ENABLE_DBS_SCAN_WITH_ASYNC_SCAN_OFF:
		is_disabled = true;
		break;
	default:
		break;
	}

	return is_disabled;
}

uint32_t policy_mgr_get_mcc_to_scc_switch_mode(
	struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return 0;
	}

	return pm_ctx->cfg.mcc_to_scc_switch;
}

/**
 * policy_mgr_get_dbs_config() - Get DBS bit
 *
 * Gets the DBS bit of fw_mode_config_bits
 *
 * Return: 0 or 1 to indicate the DBS bit
 */
bool policy_mgr_get_dbs_config(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t fw_mode_config;

	if (policy_mgr_is_dual_mac_disabled_in_ini(psoc))
		return false;


	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		/* We take that it is disabled and proceed */
		return false;
	}
	fw_mode_config = pm_ctx->dual_mac_cfg.cur_fw_mode_config;

	return WMI_DBS_FW_MODE_CFG_DBS_GET(fw_mode_config);
}

/**
 * policy_mgr_get_agile_dfs_config() - Get Agile DFS bit
 *
 * Gets the Agile DFS bit of fw_mode_config_bits
 *
 * Return: 0 or 1 to indicate the Agile DFS bit
 */
bool policy_mgr_get_agile_dfs_config(struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t fw_mode_config;

	if (policy_mgr_is_dual_mac_disabled_in_ini(psoc))
		return false;


	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		/* We take that it is disabled and proceed */
		return false;
	}
	fw_mode_config = pm_ctx->dual_mac_cfg.cur_fw_mode_config;

	return WMI_DBS_FW_MODE_CFG_AGILE_DFS_GET(fw_mode_config);
}

/**
 * policy_mgr_get_dbs_scan_config() - Get DBS scan bit
 *
 * Gets the DBS scan bit of concurrent_scan_config_bits
 *
 * Return: 0 or 1 to indicate the DBS scan bit
 */
bool policy_mgr_get_dbs_scan_config(struct wlan_objmgr_psoc *psoc)
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

	return WMI_DBS_CONC_SCAN_CFG_DBS_SCAN_GET(scan_config);
}

/**
 * policy_mgr_get_tx_rx_ss_from_config() - Get Tx/Rx spatial
 * stream from HW mode config
 * @mac_ss: Config which indicates the HW mode as per 'hw_mode_ss_config'
 * @tx_ss: Contains the Tx spatial stream
 * @rx_ss: Contains the Rx spatial stream
 *
 * Returns the number of spatial streams of Tx and Rx
 *
 * Return: None
 */
void policy_mgr_get_tx_rx_ss_from_config(enum hw_mode_ss_config mac_ss,
		uint32_t *tx_ss, uint32_t *rx_ss)
{
	switch (mac_ss) {
	case HW_MODE_SS_0x0:
		*tx_ss = 0;
		*rx_ss = 0;
		break;
	case HW_MODE_SS_1x1:
		*tx_ss = 1;
		*rx_ss = 1;
		break;
	case HW_MODE_SS_2x2:
		*tx_ss = 2;
		*rx_ss = 2;
		break;
	case HW_MODE_SS_3x3:
		*tx_ss = 3;
		*rx_ss = 3;
		break;
	case HW_MODE_SS_4x4:
		*tx_ss = 4;
		*rx_ss = 4;
		break;
	default:
		*tx_ss = 0;
		*rx_ss = 0;
	}
}

/**
 * policy_mgr_get_matching_hw_mode_index() - Get matching HW mode index
 * @psoc: psoc handle
 * @mac0_tx_ss: Number of tx spatial streams of MAC0
 * @mac0_rx_ss: Number of rx spatial streams of MAC0
 * @mac0_bw: Bandwidth of MAC0 of type 'hw_mode_bandwidth'
 * @mac1_tx_ss: Number of tx spatial streams of MAC1
 * @mac1_rx_ss: Number of rx spatial streams of MAC1
 * @mac1_bw: Bandwidth of MAC1 of type 'hw_mode_bandwidth'
 * @mac0_band_cap: mac0 band capability requirement
 *     (0: Don't care, 1: 2.4G, 2: 5G)
 * @dbs: DBS capability of type 'hw_mode_dbs_capab'
 * @dfs: Agile DFS capability of type 'hw_mode_agile_dfs_capab'
 * @sbs: SBS capability of type 'hw_mode_sbs_capab'
 *
 * Fetches the HW mode index corresponding to the HW mode provided.
 * In Genoa two DBS HW modes (2x2 5G + 1x1 2G, 2x2 2G + 1x1 5G),
 * the "ss" number and "bw" value are not enough to specify the expected
 * HW mode. But in both HW mode, the mac0 can support either 5G or 2G.
 * So, the Parameter "mac0_band_cap" will specify the expected band support
 * requirement on mac 0 to find the expected HW mode.
 *
 * Return: Positive hw mode index in case a match is found or a negative
 * value, otherwise
 */
int8_t policy_mgr_get_matching_hw_mode_index(
		struct wlan_objmgr_psoc *psoc,
		uint32_t mac0_tx_ss, uint32_t mac0_rx_ss,
		enum hw_mode_bandwidth mac0_bw,
		uint32_t mac1_tx_ss, uint32_t mac1_rx_ss,
		enum hw_mode_bandwidth mac1_bw,
		enum hw_mode_mac_band_cap mac0_band_cap,
		enum hw_mode_dbs_capab dbs,
		enum hw_mode_agile_dfs_capab dfs,
		enum hw_mode_sbs_capab sbs)
{
	uint32_t i;
	uint32_t t_mac0_tx_ss, t_mac0_rx_ss, t_mac0_bw;
	uint32_t t_mac1_tx_ss, t_mac1_rx_ss, t_mac1_bw;
	uint32_t dbs_mode, agile_dfs_mode, sbs_mode;
	uint32_t t_mac0_band_cap;
	int8_t found = -EINVAL;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return found;
	}

	for (i = 0; i < pm_ctx->num_dbs_hw_modes; i++) {
		t_mac0_tx_ss = POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		if (t_mac0_tx_ss < mac0_tx_ss)
			continue;

		t_mac0_rx_ss = POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		if (t_mac0_rx_ss < mac0_rx_ss)
			continue;

		t_mac0_bw = POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		/*
		 * Firmware advertises max bw capability as CBW 80+80
		 * for single MAC. Thus CBW 20/40/80 should also be
		 * supported, if CBW 80+80 is supported.
		 */
		if (t_mac0_bw < mac0_bw)
			continue;

		t_mac1_tx_ss = POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		if (t_mac1_tx_ss < mac1_tx_ss)
			continue;

		t_mac1_rx_ss = POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		if (t_mac1_rx_ss < mac1_rx_ss)
			continue;

		t_mac1_bw = POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		if (t_mac1_bw < mac1_bw)
			continue;

		dbs_mode = POLICY_MGR_HW_MODE_DBS_MODE_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		if (dbs_mode != dbs)
			continue;

		agile_dfs_mode = POLICY_MGR_HW_MODE_AGILE_DFS_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		if (agile_dfs_mode != dfs)
			continue;

		sbs_mode = POLICY_MGR_HW_MODE_SBS_MODE_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		if (sbs_mode != sbs)
			continue;

		t_mac0_band_cap = POLICY_MGR_HW_MODE_MAC0_BAND_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);
		if (mac0_band_cap && t_mac0_band_cap != mac0_band_cap)
			continue;

		found = POLICY_MGR_HW_MODE_ID_GET(
				pm_ctx->hw_mode.hw_mode_list[i]);

		policy_mgr_debug("hw_mode id %d found at %d", found, i);

		break;
	}
	return found;
}

/**
 * policy_mgr_get_hw_mode_from_dbs_hw_list() - Get hw_mode index
 * @mac0_ss: MAC0 spatial stream configuration
 * @mac0_bw: MAC0 bandwidth configuration
 * @mac1_ss: MAC1 spatial stream configuration
 * @mac1_bw: MAC1 bandwidth configuration
 * @mac0_band_cap: mac0 band capability requirement
 *     (0: Don't care, 1: 2.4G, 2: 5G)
 * @dbs: HW DBS capability
 * @dfs: HW Agile DFS capability
 * @sbs: HW SBS capability
 *
 * Get the HW mode index corresponding to the HW modes spatial stream,
 * bandwidth, DBS, Agile DFS and SBS capability
 *
 * In Genoa two DBS HW modes (2x2 5G + 1x1 2G, 2x2 2G + 1x1 5G),
 * the "ss" number and "bw" value are not enough to specify the expected
 * HW mode. But in both HW mode, the mac0 can support either 5G or 2G.
 * So, the Parameter "mac0_band_cap" will specify the expected band support
 * requirement on mac 0 to find the expected HW mode.
 *
 * Return: Index number if a match is found or -negative value if not found
 */
int8_t policy_mgr_get_hw_mode_idx_from_dbs_hw_list(
		struct wlan_objmgr_psoc *psoc,
		enum hw_mode_ss_config mac0_ss,
		enum hw_mode_bandwidth mac0_bw,
		enum hw_mode_ss_config mac1_ss,
		enum hw_mode_bandwidth mac1_bw,
		enum hw_mode_mac_band_cap mac0_band_cap,
		enum hw_mode_dbs_capab dbs,
		enum hw_mode_agile_dfs_capab dfs,
		enum hw_mode_sbs_capab sbs)
{
	uint32_t mac0_tx_ss, mac0_rx_ss;
	uint32_t mac1_tx_ss, mac1_rx_ss;

	policy_mgr_get_tx_rx_ss_from_config(mac0_ss, &mac0_tx_ss, &mac0_rx_ss);
	policy_mgr_get_tx_rx_ss_from_config(mac1_ss, &mac1_tx_ss, &mac1_rx_ss);

	policy_mgr_debug("MAC0: TxSS=%d, RxSS=%d, BW=%d band=%d",
			 mac0_tx_ss, mac0_rx_ss, mac0_bw, mac0_band_cap);
	policy_mgr_debug("MAC1: TxSS=%d, RxSS=%d, BW=%d",
			 mac1_tx_ss, mac1_rx_ss, mac1_bw);
	policy_mgr_debug("DBS=%d, Agile DFS=%d, SBS=%d",
			 dbs, dfs, sbs);

	return policy_mgr_get_matching_hw_mode_index(psoc, mac0_tx_ss,
						mac0_rx_ss,
						mac0_bw,
						mac1_tx_ss, mac1_rx_ss,
						mac1_bw,
						mac0_band_cap,
						dbs, dfs, sbs);
}

/**
 * policy_mgr_get_hw_mode_from_idx() - Get HW mode based on index
 * @psoc: psoc object
 * @idx: HW mode id
 * @hw_mode: HW mode params
 *
 * Fetches the HW mode parameters
 *
 * Return: Success if hw mode is obtained and the hw mode params
 */
QDF_STATUS policy_mgr_get_hw_mode_from_idx(
		struct wlan_objmgr_psoc *psoc,
		uint32_t idx,
		struct policy_mgr_hw_mode_params *hw_mode)
{
	uint32_t param;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint8_t mac0_min_ss;
	uint8_t mac1_min_ss;
	uint32_t i, hw_mode_id;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}
	if (!pm_ctx->num_dbs_hw_modes) {
		policy_mgr_err("No dbs hw modes available");
		return QDF_STATUS_E_FAILURE;
	}
	for (i = 0; i < pm_ctx->num_dbs_hw_modes; i++) {
		param = pm_ctx->hw_mode.hw_mode_list[i];
		hw_mode_id = POLICY_MGR_HW_MODE_ID_GET(param);
		if (hw_mode_id == idx)
			break;
	}
	if (i >= pm_ctx->num_dbs_hw_modes) {
		policy_mgr_err("hw mode id %d not found", idx);
		return QDF_STATUS_E_FAILURE;
	}

	param = pm_ctx->hw_mode.hw_mode_list[i];

	hw_mode->mac0_tx_ss = POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_GET(param);
	hw_mode->mac0_rx_ss = POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_GET(param);
	hw_mode->mac0_bw = POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_GET(param);
	hw_mode->mac1_tx_ss = POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_GET(param);
	hw_mode->mac1_rx_ss = POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_GET(param);
	hw_mode->mac1_bw = POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_GET(param);
	hw_mode->mac0_band_cap = POLICY_MGR_HW_MODE_MAC0_BAND_GET(param);
	hw_mode->dbs_cap = POLICY_MGR_HW_MODE_DBS_MODE_GET(param);
	hw_mode->agile_dfs_cap = POLICY_MGR_HW_MODE_AGILE_DFS_GET(param);
	hw_mode->sbs_cap = POLICY_MGR_HW_MODE_SBS_MODE_GET(param);
	if (hw_mode->dbs_cap) {
		mac0_min_ss = QDF_MIN(hw_mode->mac0_tx_ss, hw_mode->mac0_rx_ss);
		mac1_min_ss = QDF_MIN(hw_mode->mac1_tx_ss, hw_mode->mac1_rx_ss);
		if (hw_mode->mac0_band_cap == WLAN_5G_CAPABILITY &&
		    mac0_min_ss && mac1_min_ss &&
		    mac0_min_ss > mac1_min_ss)
			hw_mode->action_type = PM_DBS1;
		else if (hw_mode->mac0_band_cap == WLAN_2G_CAPABILITY &&
			 mac0_min_ss && mac1_min_ss &&
			 mac0_min_ss > mac1_min_ss)
			hw_mode->action_type = PM_DBS2;
		else
			hw_mode->action_type = PM_DBS;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * policy_mgr_get_old_and_new_hw_index() - Get the old and new HW index
 * @old_hw_mode_index: Value at this pointer contains the old HW mode index
 * Default value when not configured is POLICY_MGR_DEFAULT_HW_MODE_INDEX
 * @new_hw_mode_index: Value at this pointer contains the new HW mode index
 * Default value when not configured is POLICY_MGR_DEFAULT_HW_MODE_INDEX
 *
 * Get the old and new HW index configured in the driver
 *
 * Return: Failure in case the HW mode indices cannot be fetched and Success
 * otherwise. When no HW mode transition has happened the values of
 * old_hw_mode_index and new_hw_mode_index will be the same.
 */
QDF_STATUS policy_mgr_get_old_and_new_hw_index(
		struct wlan_objmgr_psoc *psoc,
		uint32_t *old_hw_mode_index,
		uint32_t *new_hw_mode_index)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_INVAL;
	}

	*old_hw_mode_index = pm_ctx->old_hw_mode_index;
	*new_hw_mode_index = pm_ctx->new_hw_mode_index;

	return QDF_STATUS_SUCCESS;
}

void policy_mgr_update_conc_list(struct wlan_objmgr_psoc *psoc,
		uint32_t conn_index,
		enum policy_mgr_con_mode mode,
		uint32_t ch_freq,
		enum hw_mode_bandwidth bw,
		uint8_t mac,
		enum policy_mgr_chain_mode chain_mask,
		uint32_t original_nss,
		uint32_t vdev_id,
		bool in_use,
		bool update_conn,
		uint16_t ch_flagext)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	bool mcc_mode;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	if (conn_index >= MAX_NUMBER_OF_CONC_CONNECTIONS) {
		policy_mgr_err("Number of connections exceeded conn_index: %d",
			conn_index);
		return;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	pm_conc_connection_list[conn_index].mode = mode;
	pm_conc_connection_list[conn_index].freq = ch_freq;
	pm_conc_connection_list[conn_index].bw = bw;
	pm_conc_connection_list[conn_index].mac = mac;
	pm_conc_connection_list[conn_index].chain_mask = chain_mask;
	pm_conc_connection_list[conn_index].original_nss = original_nss;
	pm_conc_connection_list[conn_index].vdev_id = vdev_id;
	pm_conc_connection_list[conn_index].in_use = in_use;
	pm_conc_connection_list[conn_index].ch_flagext = ch_flagext;
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	/*
	 * For STA and P2P client mode, the mode change event sent as part
	 * of the callback causes delay in processing M1 frame at supplicant
	 * resulting in cert test case failure. The mode change event is sent
	 * as part of add key for STA and P2P client mode.
	 */
	if (pm_ctx->mode_change_cb && update_conn)
		pm_ctx->mode_change_cb();

	policy_mgr_dump_connection_status_info(psoc);
	if (pm_ctx->cdp_cbacks.cdp_update_mac_id)
		pm_ctx->cdp_cbacks.cdp_update_mac_id(psoc, vdev_id, mac);

	/* IPA only cares about STA or SAP mode */
	if (mode == PM_STA_MODE || mode == PM_SAP_MODE) {
		mcc_mode = policy_mgr_current_concurrency_is_mcc(psoc);

		if (pm_ctx->dp_cbacks.hdd_ipa_set_mcc_mode_cb)
			pm_ctx->dp_cbacks.hdd_ipa_set_mcc_mode_cb(mcc_mode);
	}

	if (pm_ctx->conc_cbacks.connection_info_update)
		pm_ctx->conc_cbacks.connection_info_update();
}

/**
 * policy_mgr_store_and_del_conn_info() - Store and del a connection info
 * @mode: Mode whose entry has to be deleted
 * @all_matching_cxn_to_del: All the specified mode entries should be deleted
 * @info: Struture array pointer where the connection info will be saved
 * @num_cxn_del: Number of connection which are going to be deleted
 *
 * Saves the connection info corresponding to the provided mode
 * and deleted that corresponding entry based on vdev from the
 * connection info structure
 *
 * Return: None
 */
void policy_mgr_store_and_del_conn_info(struct wlan_objmgr_psoc *psoc,
	enum policy_mgr_con_mode mode, bool all_matching_cxn_to_del,
	struct policy_mgr_conc_connection_info *info, uint8_t *num_cxn_del)
{
	int32_t conn_index = 0;
	uint32_t found_index = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	if (!num_cxn_del) {
		policy_mgr_err("num_cxn_del is NULL");
		return;
	}
	*num_cxn_del = 0;
	if (!info) {
		policy_mgr_err("Invalid connection info");
		return;
	}
	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
		if (mode == pm_conc_connection_list[conn_index].mode) {
			/*
			 * Storing the connection entry which will be
			 * temporarily deleted.
			 */
			info[found_index] = pm_conc_connection_list[conn_index];
			/* Deleting the connection entry */
			policy_mgr_decr_connection_count(psoc,
					info[found_index].vdev_id);
			policy_mgr_debug("Stored %d (%d), deleted STA entry with vdev id %d, index %d",
					 info[found_index].vdev_id,
					 info[found_index].mode,
					 info[found_index].vdev_id, conn_index);
			pm_ctx->no_of_active_sessions[info->mode]--;
			found_index++;
			if (all_matching_cxn_to_del)
				continue;
			else
				break;
		}
		conn_index++;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	if (!found_index) {
		*num_cxn_del = 0;
		policy_mgr_debug("Mode:%d not available in the conn info",
				 mode);
	} else {
		*num_cxn_del = found_index;
		policy_mgr_debug("Mode:%d number of conn %d temp del",
				 mode, *num_cxn_del);
	}

	/*
	 * Caller should set the PCL and restore the connection entry
	 * in conn info.
	 */
}

void policy_mgr_store_and_del_conn_info_by_vdev_id(
			struct wlan_objmgr_psoc *psoc,
			uint32_t vdev_id,
			struct policy_mgr_conc_connection_info *info,
			uint8_t *num_cxn_del)
{
	uint32_t conn_index = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	if (!info || !num_cxn_del) {
		policy_mgr_err("Invalid parameters");
		return;
	}

	*num_cxn_del = 0;
	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
	     conn_index++) {
		if ((pm_conc_connection_list[conn_index].vdev_id == vdev_id) &&
		    pm_conc_connection_list[conn_index].in_use) {
			*num_cxn_del = 1;
			break;
		}
	}
	/*
	 * Storing the connection entry which will be
	 * temporarily deleted.
	 */
	if (*num_cxn_del == 1) {
		*info = pm_conc_connection_list[conn_index];
		pm_ctx->no_of_active_sessions[info->mode]--;
		/* Deleting the connection entry */
		policy_mgr_decr_connection_count(
			psoc,
			pm_conc_connection_list[conn_index].vdev_id);
	}

	policy_mgr_debug("vdev id %d, num_cxn_del %d", vdev_id, *num_cxn_del);
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
}

void policy_mgr_store_and_del_conn_info_by_chan_and_mode(
			struct wlan_objmgr_psoc *psoc,
			uint32_t ch_freq,
			enum policy_mgr_con_mode mode,
			struct policy_mgr_conc_connection_info *info,
			uint8_t *num_cxn_del)
{
	uint32_t conn_index = 0;
	uint8_t found_index = 0;

	struct policy_mgr_psoc_priv_obj *pm_ctx;

	if (!info || !num_cxn_del) {
		policy_mgr_err("Invalid parameters");
		return;
	}
	*num_cxn_del = 0;
	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
		if (ch_freq != pm_conc_connection_list[conn_index].freq ||
		    mode != pm_conc_connection_list[conn_index].mode) {
			conn_index++;
			continue;
		}
		info[found_index] = pm_conc_connection_list[conn_index];
		policy_mgr_debug("Stored %d (%d), deleted STA entry with vdev id %d, index %d ch %d",
				 info[found_index].vdev_id,
				 info[found_index].mode,
				 info[found_index].vdev_id, conn_index,
				 ch_freq);
		found_index++;
		conn_index++;
	}
	conn_index = 0;
	while (conn_index < found_index) {
		policy_mgr_decr_connection_count(
			psoc, info[conn_index].vdev_id);

		pm_ctx->no_of_active_sessions[info[conn_index].mode]--;
		conn_index++;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
	*num_cxn_del = found_index;
}

/**
 * policy_mgr_restore_deleted_conn_info() - Restore connection info
 * @info: An array saving connection info that is to be restored
 * @num_cxn_del: Number of connection temporary deleted
 *
 * Restores the connection info of STA that was saved before
 * updating the PCL to the FW
 *
 * Return: None
 */
void policy_mgr_restore_deleted_conn_info(struct wlan_objmgr_psoc *psoc,
		struct policy_mgr_conc_connection_info *info,
		uint8_t num_cxn_del)
{
	uint32_t conn_index;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	int i;

	if (MAX_NUMBER_OF_CONC_CONNECTIONS < num_cxn_del || 0 == num_cxn_del) {
		policy_mgr_err("Failed to restore %d/%d deleted information",
				num_cxn_del, MAX_NUMBER_OF_CONC_CONNECTIONS);
		return;
	}
	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	conn_index = policy_mgr_get_connection_count(psoc);
	if (MAX_NUMBER_OF_CONC_CONNECTIONS - num_cxn_del <= conn_index) {
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		policy_mgr_err("Failed to restore the deleted information %d/%d/%d",
			       conn_index, num_cxn_del,
			       MAX_NUMBER_OF_CONC_CONNECTIONS);
		return;
	}

	qdf_mem_copy(&pm_conc_connection_list[conn_index], info,
		     num_cxn_del * sizeof(*info));
	pm_ctx->no_of_active_sessions[info->mode] += num_cxn_del;
	for (i = 0; i < num_cxn_del; i++)
		policy_mgr_debug("Restored the deleleted conn info, vdev:%d, index:%d",
				 info[i].vdev_id, conn_index++);
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
}

static bool
policy_mgr_is_freq_range_5_6ghz(qdf_freq_t start_freq, qdf_freq_t end_freq)
{
	/*
	 * As Fw is sending the whole hardware range which include 4.9Ghz as
	 * well. Use LOWER_END_FREQ_5GHZ to differentiate 2.4Ghz and 5Ghz
	 */
	if (start_freq >= LOWER_END_FREQ_5GHZ &&
	    end_freq >= LOWER_END_FREQ_5GHZ)
		return true;

	return false;
}

static bool
policy_mgr_is_freq_range_2ghz(qdf_freq_t start_freq, qdf_freq_t end_freq)
{
	/*
	 * As Fw is sending the whole hardware range which include 4.9Ghz as
	 * well. Use LOWER_END_FREQ_5GHZ to differentiate 2.4Ghz and 5Ghz
	 */
	if (start_freq < LOWER_END_FREQ_5GHZ && end_freq < LOWER_END_FREQ_5GHZ)
		return true;

	return false;
}

static void
policy_mgr_fill_curr_mac_2ghz_freq(uint32_t mac_id,
				   struct policy_mgr_pdev_mac_freq_map *freq,
				   struct policy_mgr_psoc_priv_obj *pm_ctx)
{
	pm_ctx->hw_mode.cur_mac_freq_range[mac_id].low_2ghz_freq =
					QDF_MAX(freq->start_freq,
						wlan_reg_min_24ghz_chan_freq());
	pm_ctx->hw_mode.cur_mac_freq_range[mac_id].high_2ghz_freq =
					QDF_MIN(freq->end_freq,
						wlan_reg_max_24ghz_chan_freq());
}

static void
policy_mgr_fill_curr_mac_5ghz_freq(uint32_t mac_id,
				   struct policy_mgr_pdev_mac_freq_map *freq,
				   struct policy_mgr_psoc_priv_obj *pm_ctx)
{
	qdf_freq_t max_5g_freq;

	max_5g_freq = wlan_reg_max_6ghz_chan_freq() ?
			wlan_reg_max_6ghz_chan_freq() :
			wlan_reg_max_5ghz_chan_freq();

	pm_ctx->hw_mode.cur_mac_freq_range[mac_id].low_5ghz_freq =
					QDF_MAX(freq->start_freq,
						wlan_reg_min_5ghz_chan_freq());
	pm_ctx->hw_mode.cur_mac_freq_range[mac_id].high_5ghz_freq =
					QDF_MIN(freq->end_freq, max_5g_freq);
}

void
policy_mgr_fill_curr_mac_freq_by_hwmode(struct policy_mgr_psoc_priv_obj *pm_ctx,
					enum policy_mgr_mode mode_hw)
{
	uint8_t i;
	struct policy_mgr_freq_range *cur_mac_freq, *hwmode_freq;

	cur_mac_freq = pm_ctx->hw_mode.cur_mac_freq_range;
	hwmode_freq = pm_ctx->hw_mode.freq_range_caps[mode_hw];

	for (i = 0; i < MAX_MAC; i++) {
		cur_mac_freq[i].low_2ghz_freq = hwmode_freq[i].low_2ghz_freq;
		cur_mac_freq[i].high_2ghz_freq = hwmode_freq[i].high_2ghz_freq;
		cur_mac_freq[i].low_5ghz_freq = hwmode_freq[i].low_5ghz_freq;
		cur_mac_freq[i].high_5ghz_freq = hwmode_freq[i].high_5ghz_freq;
	}
}

static void
policy_mgr_fill_legacy_freq_range(struct policy_mgr_psoc_priv_obj *pm_ctx,
				  struct policy_mgr_hw_mode_params hw_mode)
{
	enum policy_mgr_mode mode;

	mode = hw_mode.dbs_cap ? MODE_DBS : MODE_SMM;
	policy_mgr_fill_curr_mac_freq_by_hwmode(pm_ctx, mode);
}

static QDF_STATUS
policy_mgr_fill_curr_freq_by_pdev_freq(int32_t num_mac_freq,
				struct policy_mgr_pdev_mac_freq_map *freq,
				struct policy_mgr_psoc_priv_obj *pm_ctx,
				struct policy_mgr_hw_mode_params hw_mode)
{
	uint32_t mac_id, i;

	/* memzero before filling it */
	qdf_mem_zero(pm_ctx->hw_mode.cur_mac_freq_range,
		     sizeof(pm_ctx->hw_mode.cur_mac_freq_range));
	for (i = 0; i < num_mac_freq; i++) {
		mac_id = freq[i].mac_id;

		if (mac_id >= MAX_MAC) {
			policy_mgr_debug("Invalid pdev id %d", mac_id);
			return QDF_STATUS_E_INVAL;
		}

		policy_mgr_debug("mac_id %d start freq %d end_freq %d",
				 mac_id, freq[i].start_freq,
				 freq[i].end_freq);

		if (policy_mgr_is_freq_range_2ghz(freq[i].start_freq,
						  freq[i].end_freq)) {
			policy_mgr_fill_curr_mac_2ghz_freq(mac_id,
							   &freq[i],
							   pm_ctx);
		} else if (policy_mgr_is_freq_range_5_6ghz(freq[i].start_freq,
							 freq[i].end_freq)) {
			policy_mgr_fill_curr_mac_5ghz_freq(mac_id, &freq[i],
							   pm_ctx);
		} else  {
			policy_mgr_err("Invalid different band freq range: mac_id %d start freq %d end_freq %d",
				       mac_id, freq[i].start_freq,
				       freq[i].end_freq);
			return QDF_STATUS_E_INVAL;
		}
	}

	return QDF_STATUS_SUCCESS;
}

static void
policy_mgr_update_curr_mac_freq(uint32_t num_mac_freq,
				struct policy_mgr_pdev_mac_freq_map *freq,
				struct policy_mgr_psoc_priv_obj *pm_ctx,
				struct policy_mgr_hw_mode_params hw_mode)
{
	QDF_STATUS status;

	if (num_mac_freq && freq) {
		status = policy_mgr_fill_curr_freq_by_pdev_freq(num_mac_freq,
								freq, pm_ctx,
								hw_mode);
		if (QDF_IS_STATUS_SUCCESS(status))
			return;
	}

	policy_mgr_fill_legacy_freq_range(pm_ctx, hw_mode);
}

/**
 * policy_mgr_update_hw_mode_conn_info() - Update connection
 * info based on HW mode
 * @num_vdev_mac_entries: Number of vdev-mac id entries that follow
 * @vdev_mac_map: Mapping of vdev-mac id
 * @hw_mode: HW mode
 * @num_mac_freq: number of Frequency Range
 * @freq_info: Pointer to Frequency Range
 *
 * Updates the connection info parameters based on the new HW mode
 *
 * Return: None
 */
void policy_mgr_update_hw_mode_conn_info(struct wlan_objmgr_psoc *psoc,
				uint32_t num_vdev_mac_entries,
				struct policy_mgr_vdev_mac_map *vdev_mac_map,
				struct policy_mgr_hw_mode_params hw_mode,
				uint32_t num_mac_freq,
				struct policy_mgr_pdev_mac_freq_map *freq_info)
{
	uint32_t i, conn_index, found;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	policy_mgr_update_curr_mac_freq(num_mac_freq, freq_info, pm_ctx,
					hw_mode);

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (i = 0; i < num_vdev_mac_entries; i++) {
		conn_index = 0;
		found = 0;
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			if (vdev_mac_map[i].vdev_id ==
				pm_conc_connection_list[conn_index].vdev_id) {
				found = 1;
				break;
			}
			conn_index++;
		}
		if (found) {
			pm_conc_connection_list[conn_index].mac =
				vdev_mac_map[i].mac_id;
			policy_mgr_debug("vdev:%d, mac:%d",
			  pm_conc_connection_list[conn_index].vdev_id,
			  pm_conc_connection_list[conn_index].mac);
			if (pm_ctx->cdp_cbacks.cdp_update_mac_id)
				pm_ctx->cdp_cbacks.cdp_update_mac_id(
					psoc,
					vdev_mac_map[i].vdev_id,
					vdev_mac_map[i].mac_id);
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	policy_mgr_dump_connection_status_info(psoc);
}

void policy_mgr_pdev_set_hw_mode_cb(uint32_t status,
				uint32_t cfgd_hw_mode_index,
				uint32_t num_vdev_mac_entries,
				struct policy_mgr_vdev_mac_map *vdev_mac_map,
				uint8_t next_action,
				enum policy_mgr_conn_update_reason reason,
				uint32_t session_id, void *context,
				uint32_t request_id)
{
	QDF_STATUS ret;
	struct policy_mgr_hw_mode_params hw_mode;
	uint32_t i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(context);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		goto set_done_event;
	}

	policy_mgr_set_hw_mode_change_in_progress(context,
		POLICY_MGR_HW_MODE_NOT_IN_PROGRESS);

	if (status == SET_HW_MODE_STATUS_OK ||
	    status == SET_HW_MODE_STATUS_ALREADY) {
		policy_mgr_set_connection_update(context);
	}

	if (status != SET_HW_MODE_STATUS_OK) {
		policy_mgr_debug("Set HW mode failed with status %d", status);
		goto next_action;
	}

	/* vdev mac map for NAN Discovery is expected in NAN Enable resp */
	if (reason != POLICY_MGR_UPDATE_REASON_NAN_DISCOVERY &&
	    !vdev_mac_map) {
		policy_mgr_err("vdev_mac_map is NULL");
		goto set_done_event;
	}

	policy_mgr_debug("cfgd_hw_mode_index=%d", cfgd_hw_mode_index);

	for (i = 0; i < num_vdev_mac_entries; i++)
		policy_mgr_debug("vdev_id:%d mac_id:%d",
				vdev_mac_map[i].vdev_id,
				vdev_mac_map[i].mac_id);

	ret = policy_mgr_get_hw_mode_from_idx(context, cfgd_hw_mode_index,
			&hw_mode);
	if (ret != QDF_STATUS_SUCCESS) {
		policy_mgr_err("Get HW mode failed: %d", ret);
		goto set_done_event;
	}

	policy_mgr_debug("MAC0: TxSS:%d, RxSS:%d, Bw:%d, band_cap %d",
			 hw_mode.mac0_tx_ss, hw_mode.mac0_rx_ss,
			 hw_mode.mac0_bw, hw_mode.mac0_band_cap);
	policy_mgr_debug("MAC1: TxSS:%d, RxSS:%d, Bw:%d",
			 hw_mode.mac1_tx_ss, hw_mode.mac1_rx_ss,
			 hw_mode.mac1_bw);
	policy_mgr_debug("DBS:%d, Agile DFS:%d, SBS:%d",
			 hw_mode.dbs_cap, hw_mode.agile_dfs_cap,
			 hw_mode.sbs_cap);

	/* update pm_conc_connection_list */
	policy_mgr_update_hw_mode_conn_info(context,
					    num_vdev_mac_entries,
					    vdev_mac_map,
					    hw_mode, 0, NULL);
	if (pm_ctx->mode_change_cb)
		pm_ctx->mode_change_cb();

	/* Notify tdls */
	if (pm_ctx->tdls_cbacks.tdls_notify_decrement_session)
		pm_ctx->tdls_cbacks.tdls_notify_decrement_session(pm_ctx->psoc);

next_action:
	if (PM_NOP != next_action && (status == SET_HW_MODE_STATUS_ALREADY ||
	    status == SET_HW_MODE_STATUS_OK))
		policy_mgr_next_actions(context, session_id,
			next_action, reason, request_id);
	else
		policy_mgr_debug("No action needed right now");

set_done_event:
	ret = policy_mgr_set_opportunistic_update(context);
	if (!QDF_IS_STATUS_SUCCESS(ret))
		policy_mgr_err("ERROR: set opportunistic_update event failed");
}

/**
 * policy_mgr_dump_current_concurrency_one_connection() - To dump the
 * current concurrency info with one connection
 * @cc_mode: connection string
 * @length: Maximum size of the string
 *
 * This routine is called to dump the concurrency info
 *
 * Return: length of the string
 */
static uint32_t policy_mgr_dump_current_concurrency_one_connection(
		char *cc_mode, uint32_t length)
{
	uint32_t count = 0;
	enum policy_mgr_con_mode mode;

	mode = pm_conc_connection_list[0].mode;

	switch (mode) {
	case PM_STA_MODE:
		count = strlcat(cc_mode, "STA",
					length);
		break;
	case PM_SAP_MODE:
		count = strlcat(cc_mode, "SAP",
					length);
		break;
	case PM_P2P_CLIENT_MODE:
		count = strlcat(cc_mode, "P2P CLI",
					length);
		break;
	case PM_P2P_GO_MODE:
		count = strlcat(cc_mode, "P2P GO",
					length);
		break;
	case PM_NAN_DISC_MODE:
		count = strlcat(cc_mode, "NAN DISC", length);
		break;
	case PM_NDI_MODE:
		count = strlcat(cc_mode, "NDI", length);
		break;
	default:
		policy_mgr_err("unexpected mode %d", mode);
		break;
	}

	return count;
}

/**
 * policy_mgr_dump_current_concurrency_two_connection() - To dump the
 * current concurrency info with two connections
 * @cc_mode: connection string
 * @length: Maximum size of the string
 *
 * This routine is called to dump the concurrency info
 *
 * Return: length of the string
 */
static uint32_t policy_mgr_dump_current_concurrency_two_connection(
		char *cc_mode, uint32_t length)
{
	uint32_t count = 0;
	enum policy_mgr_con_mode mode;

	mode = pm_conc_connection_list[1].mode;

	switch (mode) {
	case PM_STA_MODE:
		count = policy_mgr_dump_current_concurrency_one_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+STA",
					length);
		break;
	case PM_SAP_MODE:
		count = policy_mgr_dump_current_concurrency_one_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+SAP",
					length);
		break;
	case PM_P2P_CLIENT_MODE:
		count = policy_mgr_dump_current_concurrency_one_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+P2P CLI",
					length);
		break;
	case PM_P2P_GO_MODE:
		count = policy_mgr_dump_current_concurrency_one_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+P2P GO",
					length);
		break;
	case PM_NDI_MODE:
		count = policy_mgr_dump_current_concurrency_one_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+NDI",
					length);
		break;
	case PM_NAN_DISC_MODE:
		count = policy_mgr_dump_current_concurrency_one_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+NAN Disc", length);
		break;
	default:
		policy_mgr_err("unexpected mode %d", mode);
		break;
	}

	return count;
}

/**
 * policy_mgr_dump_current_concurrency_three_connection() - To dump the
 * current concurrency info with three connections
 * @cc_mode: connection string
 * @length: Maximum size of the string
 *
 * This routine is called to dump the concurrency info
 *
 * Return: length of the string
 */
static uint32_t policy_mgr_dump_current_concurrency_three_connection(
		char *cc_mode, uint32_t length)
{
	uint32_t count = 0;
	enum policy_mgr_con_mode mode;

	mode = pm_conc_connection_list[2].mode;

	switch (mode) {
	case PM_STA_MODE:
		count = policy_mgr_dump_current_concurrency_two_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+STA",
					length);
		break;
	case PM_SAP_MODE:
		count = policy_mgr_dump_current_concurrency_two_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+SAP",
					length);
		break;
	case PM_P2P_CLIENT_MODE:
		count = policy_mgr_dump_current_concurrency_two_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+P2P CLI",
					length);
		break;
	case PM_P2P_GO_MODE:
		count = policy_mgr_dump_current_concurrency_two_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+P2P GO",
					length);
		break;
	case PM_NAN_DISC_MODE:
		count = policy_mgr_dump_current_concurrency_two_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+NAN Disc",
					length);
		break;
	case PM_NDI_MODE:
		count = policy_mgr_dump_current_concurrency_two_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+NDI",
					length);
		break;
	default:
		policy_mgr_err("unexpected mode %d", mode);
		break;
	}

	return count;
}

static void
policy_mgr_dump_dual_mac_concurrency(struct policy_mgr_psoc_priv_obj *pm_ctx,
				     char *cc_mode, uint32_t length)
{
	uint8_t mac = 0;
	char buf[4] = {0};

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	if (policy_mgr_are_2_freq_on_same_mac(pm_ctx->psoc,
					      pm_conc_connection_list[0].freq,
					      pm_conc_connection_list[1].freq)
					     ) {
		if (pm_conc_connection_list[0].freq ==
			pm_conc_connection_list[1].freq)
			strlcat(cc_mode,
				" with SCC for 1st two connections on mac ",
				length);
		else
			strlcat(cc_mode,
				" with MCC for 1st two connections on mac ",
				length);
		mac = pm_conc_connection_list[0].mac;
	}

	if (policy_mgr_are_2_freq_on_same_mac(pm_ctx->psoc,
					      pm_conc_connection_list[0].freq,
					      pm_conc_connection_list[2].freq)
					     ) {
		if (pm_conc_connection_list[0].freq ==
			pm_conc_connection_list[2].freq)
			strlcat(cc_mode,
				" with SCC for 1st & 3rd connections on mac ",
				length);
		else
			strlcat(cc_mode,
				" with MCC for 1st & 3rd connections on mac ",
				length);
		mac = pm_conc_connection_list[0].mac;
	}

	if (policy_mgr_are_2_freq_on_same_mac(pm_ctx->psoc,
					      pm_conc_connection_list[1].freq,
					      pm_conc_connection_list[2].freq)
					     ) {
		if (pm_conc_connection_list[1].freq ==
			pm_conc_connection_list[2].freq)
			strlcat(cc_mode,
				" with SCC for 2nd & 3rd connections on mac ",
				length);
		else
			strlcat(cc_mode,
				" with MCC for 2nd & 3rd connections on mac ",
				length);
		mac = pm_conc_connection_list[1].mac;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
	snprintf(buf, sizeof(buf), "%d ", mac);
	strlcat(cc_mode, buf, length);
}

/**
 * policy_mgr_dump_dbs_concurrency() - To dump the dbs concurrency
 * combination
 * @cc_mode: connection string
 *
 * This routine is called to dump the concurrency info
 *
 * Return: None
 */
static void policy_mgr_dump_dbs_concurrency(struct wlan_objmgr_psoc *psoc,
					char *cc_mode, uint32_t length)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	strlcat(cc_mode, " DBS", length);
	policy_mgr_dump_dual_mac_concurrency(pm_ctx, cc_mode, length);
}

/**
 * policy_mgr_dump_sbs_concurrency() - To dump the sbs concurrency
 * combination
 * @cc_mode: connection string
 *
 * This routine is called to dump the concurrency info
 *
 * Return: None
 */
static void policy_mgr_dump_sbs_concurrency(struct wlan_objmgr_psoc *psoc,
					    char *cc_mode, uint32_t length)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	strlcat(cc_mode, " SBS", length);
	policy_mgr_dump_dual_mac_concurrency(pm_ctx, cc_mode, length);
}

/**
 * policy_mgr_dump_current_concurrency() - To dump the current
 * concurrency combination
 *
 * This routine is called to dump the concurrency info
 *
 * Return: None
 */
void policy_mgr_dump_current_concurrency(struct wlan_objmgr_psoc *psoc)
{
	uint32_t num_connections = 0;
	char cc_mode[POLICY_MGR_MAX_CON_STRING_LEN] = {0};
	uint32_t count = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	num_connections = policy_mgr_get_connection_count(psoc);

	switch (num_connections) {
	case 1:
		policy_mgr_dump_current_concurrency_one_connection(cc_mode,
					sizeof(cc_mode));
		policy_mgr_debug("%s Standalone", cc_mode);
		break;
	case 2:
		count = policy_mgr_dump_current_concurrency_two_connection(
			cc_mode, sizeof(cc_mode));
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
		if (pm_conc_connection_list[0].freq ==
			pm_conc_connection_list[1].freq) {
			strlcat(cc_mode, " SCC", sizeof(cc_mode));
		} else if (policy_mgr_is_current_hwmode_dbs(psoc)) {
			strlcat(cc_mode, " DBS", sizeof(cc_mode));
		} else if (policy_mgr_is_current_hwmode_sbs(psoc)) {
			strlcat(cc_mode, " SBS", sizeof(cc_mode));
		} else {
			strlcat(cc_mode, " MCC", sizeof(cc_mode));
		}
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		policy_mgr_debug("%s", cc_mode);
		break;
	case 3:
		count = policy_mgr_dump_current_concurrency_three_connection(
			cc_mode, sizeof(cc_mode));
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
		if (pm_conc_connection_list[0].freq ==
		    pm_conc_connection_list[1].freq &&
		    pm_conc_connection_list[0].freq ==
		    pm_conc_connection_list[2].freq){
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
			strlcat(cc_mode, " SCC", sizeof(cc_mode));
		} else if (policy_mgr_are_3_freq_on_same_mac(psoc,
				pm_conc_connection_list[0].freq,
				pm_conc_connection_list[1].freq,
				pm_conc_connection_list[2].freq)) {
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
			strlcat(cc_mode, " MCC on single MAC", sizeof(cc_mode));
		} else {
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
			if (policy_mgr_is_current_hwmode_dbs(psoc))
				policy_mgr_dump_dbs_concurrency(psoc, cc_mode,
							sizeof(cc_mode));
			else if (policy_mgr_is_current_hwmode_sbs(psoc))
				policy_mgr_dump_sbs_concurrency(psoc, cc_mode,
							sizeof(cc_mode));
			else
				strlcat(cc_mode, " MCC", sizeof(cc_mode));
		}
		policy_mgr_debug("%s", cc_mode);
		break;
	default:
		policy_mgr_debug("unexpected num_connections value %d",
				 num_connections);
		break;
	}

	return;
}

QDF_STATUS policy_mgr_pdev_get_pcl(struct wlan_objmgr_psoc *psoc,
				   enum QDF_OPMODE mode,
				   struct policy_mgr_pcl_list *pcl)
{
	QDF_STATUS status;
	enum policy_mgr_con_mode con_mode;

	pcl->pcl_len = 0;

	switch (mode) {
	case QDF_STA_MODE:
		con_mode = PM_STA_MODE;
		break;
	case QDF_P2P_CLIENT_MODE:
		con_mode = PM_P2P_CLIENT_MODE;
		break;
	case QDF_P2P_GO_MODE:
		con_mode = PM_P2P_GO_MODE;
		break;
	case QDF_SAP_MODE:
		con_mode = PM_SAP_MODE;
		break;
	default:
		policy_mgr_err("Unable to set PCL to FW: %d", mode);
		return QDF_STATUS_E_FAILURE;
	}

	policy_mgr_debug("get pcl to set it to the FW");

	status = policy_mgr_get_pcl(psoc, con_mode,
				    pcl->pcl_list, &pcl->pcl_len,
				    pcl->weight_list,
				    QDF_ARRAY_SIZE(pcl->weight_list));
	if (status != QDF_STATUS_SUCCESS)
		policy_mgr_err("Unable to set PCL to FW, Get PCL failed");

	return status;
}

/**
 * policy_mgr_set_pcl_for_existing_combo() - Set PCL for existing connection
 * @mode: Connection mode of type 'policy_mgr_con_mode'
 * @vdev_id: Vdev Id
 *
 * Set the PCL for an existing connection
 *
 * Return: None
 */
void policy_mgr_set_pcl_for_existing_combo(struct wlan_objmgr_psoc *psoc,
					   enum policy_mgr_con_mode mode,
					   uint8_t vdev_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct policy_mgr_pcl_list pcl;

	status = policy_mgr_get_pcl_for_vdev_id(psoc, mode, pcl.pcl_list,
						&pcl.pcl_len,
						pcl.weight_list,
						QDF_ARRAY_SIZE(pcl.weight_list),
						vdev_id);
	/* Send PCL only if policy_mgr_pdev_get_pcl returned success */
	if (QDF_IS_STATUS_SUCCESS(status)) {
		status = policy_mgr_set_pcl(psoc, &pcl, vdev_id, false);
		if (QDF_IS_STATUS_ERROR(status))
			policy_mgr_err("Send set PCL to policy mgr failed");
	}
}

void policy_mgr_set_pcl_for_connected_vdev(struct wlan_objmgr_psoc *psoc,
					   uint8_t vdev_id, bool clear_pcl)
{
	struct policy_mgr_pcl_list msg = { {0} };
	struct wlan_objmgr_vdev *vdev;
	uint8_t roam_enabled_vdev_id;
	bool sta_concurrency_is_dbs, dual_sta_roam_enabled;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_POLICY_MGR_ID);
	if (!vdev) {
		policy_mgr_err("vdev is NULL");
		return;
	}

	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_STA_MODE) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_POLICY_MGR_ID);
		return;
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_POLICY_MGR_ID);

	/*
	 * Get the vdev id of the STA on which roaming is already
	 * initialized and set the vdev PCL for that STA vdev if dual
	 * STA roaming feature is enabled and concurrency is STA + STA.
	 */
	roam_enabled_vdev_id = policy_mgr_get_roam_enabled_sta_session_id(psoc,
								       vdev_id);
	if (roam_enabled_vdev_id == WLAN_UMAC_VDEV_ID_MAX)
		return;

	sta_concurrency_is_dbs = policy_mgr_concurrent_sta_doing_dbs(psoc);
	dual_sta_roam_enabled = wlan_mlme_get_dual_sta_roaming_enabled(psoc);
	policy_mgr_debug("dual_sta_roam:%d, is_dbs:%d, clear_pcl:%d",
			 dual_sta_roam_enabled, sta_concurrency_is_dbs,
			 clear_pcl);

	if (dual_sta_roam_enabled) {
		if (clear_pcl) {
			/*
			 * Here the PCL level should be at vdev level already
			 * as this is invoked from disconnect handler. Clear the
			 * vdev pcl for the existing connected STA vdev and this
			 * is followed by set PDEV pcl.
			 */
			policy_mgr_set_pcl(psoc, &msg,
					   roam_enabled_vdev_id, true);
			wlan_cm_roam_activate_pcl_per_vdev(psoc,
							   roam_enabled_vdev_id,
							   false);
		} else if (sta_concurrency_is_dbs) {
			wlan_cm_roam_activate_pcl_per_vdev(psoc,
							   roam_enabled_vdev_id,
							   true);
		}
		policy_mgr_set_pcl_for_existing_combo(psoc, PM_STA_MODE,
						      roam_enabled_vdev_id);
	}
}

#ifdef WLAN_FEATURE_11BE_MLO
uint32_t
policy_mgr_get_connected_vdev_band_mask(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_channel *chan;
	uint32_t band_mask = 0;
	struct wlan_objmgr_vdev *ml_vdev_list[WLAN_UMAC_MLO_MAX_VDEVS] = {0};
	uint16_t ml_vdev_cnt = 0;
	struct wlan_objmgr_vdev *t_vdev;
	int i;

	if (!vdev) {
		policy_mgr_err("vdev is NULL");
		return band_mask;
	}

	if (wlan_vdev_mlme_is_link_sta_vdev(vdev)) {
		policy_mgr_debug("skip mlo link sta");
		return band_mask;
	}

	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_STA_MODE ||
	    !wlan_vdev_mlme_is_mlo_vdev(vdev)) {
		chan = wlan_vdev_get_active_channel(vdev);
		if (!chan) {
			policy_mgr_err("no active channel");
			return band_mask;
		}

		band_mask |= BIT(wlan_reg_freq_to_band(chan->ch_freq));
		return band_mask;
	}

	mlo_get_ml_vdev_list(vdev, &ml_vdev_cnt, ml_vdev_list);
	for (i = 0; i < ml_vdev_cnt; i++) {
		t_vdev = ml_vdev_list[i];
		if (!ucfg_cm_is_vdev_connected(t_vdev))
			goto next;

		chan = wlan_vdev_get_active_channel(t_vdev);
		if (!chan)
			goto next;

		band_mask |= BIT(wlan_reg_freq_to_band(chan->ch_freq));
next:
		mlo_release_vdev_ref(t_vdev);
	}

	return band_mask;
}
#else
uint32_t
policy_mgr_get_connected_vdev_band_mask(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_channel *chan;
	uint32_t band_mask = 0;

	if (!vdev) {
		policy_mgr_err("vdev is NULL");
		return band_mask;
	}

	chan = wlan_vdev_get_active_channel(vdev);
	if (!chan) {
		policy_mgr_err("no active channel");
		return band_mask;
	}

	band_mask |= BIT(wlan_reg_freq_to_band(chan->ch_freq));
	return band_mask;
}
#endif

/**
 * policy_mgr_get_connected_roaming_vdev_band_mask() - get connected vdev
 * band mask
 * @psoc: PSOC object
 * @vdev_id: Vdev id
 *
 * Return: reg wifi band mask
 */
static uint32_t
policy_mgr_get_connected_roaming_vdev_band_mask(struct wlan_objmgr_psoc *psoc,
						uint8_t vdev_id)
{
	uint32_t band_mask = 0, roam_band_mask, band_mask_for_vdev;
	struct wlan_objmgr_vdev *vdev;
	bool dual_sta_roam_active, is_pcl_per_vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_POLICY_MGR_ID);
	if (!vdev) {
		policy_mgr_err("vdev is NULL");
		return 0;
	}

	band_mask_for_vdev = policy_mgr_get_connected_vdev_band_mask(vdev);

	is_pcl_per_vdev = wlan_cm_roam_is_pcl_per_vdev_active(psoc, vdev_id);
	dual_sta_roam_active = wlan_mlme_get_dual_sta_roaming_enabled(psoc);

	policy_mgr_debug("connected STA vdev_id:%d, pcl_per_vdev:%d, dual_sta_roam_active:%d",
			 vdev_id, is_pcl_per_vdev,
			 dual_sta_roam_active);

	if (dual_sta_roam_active && is_pcl_per_vdev) {
		policy_mgr_debug("connected vdev band mask:%d",
				 band_mask_for_vdev);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_POLICY_MGR_ID);
		return band_mask_for_vdev;
	}

	/*
	 * if vendor command to configure roam band is set , we will
	 * take this as priority instead of drv cmd "SETROAMINTRABAND" or
	 * active connection band.
	 */
	ucfg_reg_get_band(wlan_vdev_get_pdev(vdev), &band_mask);
	roam_band_mask = wlan_cm_get_roam_band_value(psoc, vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_POLICY_MGR_ID);

	if (roam_band_mask != band_mask) {
		policy_mgr_debug("roam_band_mask:%d", roam_band_mask);
		return roam_band_mask;
	}

	/*
	 * If PCL command is PDEV level, only one sta is active.
	 * So fill the band mask if intra band roaming is enabled
	 */
	if ((!is_pcl_per_vdev) && ucfg_mlme_is_roam_intra_band(psoc)) {
		policy_mgr_debug("connected STA band mask:%d",
				 band_mask_for_vdev);
		return band_mask_for_vdev;
	}

	policy_mgr_debug("band_mask:%d", band_mask);
	return band_mask;
}

QDF_STATUS policy_mgr_set_pcl(struct wlan_objmgr_psoc *psoc,
			      struct policy_mgr_pcl_list *msg,
			      uint8_t vdev_id,
			      bool clear_vdev_pcl)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct scheduler_msg message = {0};
	struct set_pcl_req *req_msg;
	uint32_t i;

	if (!msg) {
		policy_mgr_err("msg is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!MLME_IS_ROAM_INITIALIZED(psoc, vdev_id)) {
		policy_mgr_debug("Roam is not initialized on vdev:%d", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	req_msg = qdf_mem_malloc(sizeof(*req_msg));
	if (!req_msg)
		return QDF_STATUS_E_NOMEM;

	req_msg->band_mask =
		policy_mgr_get_connected_roaming_vdev_band_mask(psoc, vdev_id);
	for (i = 0; i < msg->pcl_len; i++) {
		req_msg->chan_weights.pcl_list[i] =  msg->pcl_list[i];
		req_msg->chan_weights.weight_list[i] =  msg->weight_list[i];
	}

	req_msg->chan_weights.pcl_len = msg->pcl_len;
	req_msg->clear_vdev_pcl = clear_vdev_pcl;

	/*
	 * Set vdev value as WLAN_UMAC_VDEV_ID_MAX, if PDEV level
	 * PCL command needs to be sent.
	 */
	if (!wlan_cm_roam_is_pcl_per_vdev_active(psoc, vdev_id))
		vdev_id = WLAN_UMAC_VDEV_ID_MAX;

	req_msg->vdev_id = vdev_id;

	/* Serialize the req through MC thread */
	message.bodyptr = req_msg;
	message.type    = SIR_HAL_SET_PCL_TO_FW;
	status = scheduler_post_message(QDF_MODULE_ID_POLICY_MGR,
					QDF_MODULE_ID_WMA,
					QDF_MODULE_ID_WMA, &message);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("scheduler_post_msg failed!(err=%d)", status);
		qdf_mem_free(req_msg);
		status = QDF_STATUS_E_FAILURE;
	}

	return status;
}

static uint32_t pm_get_vdev_id_of_first_conn_idx(struct wlan_objmgr_psoc *psoc)
{
	uint32_t conn_index = 0, vdev_id = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	struct wlan_objmgr_vdev *vdev;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return conn_index;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
	     conn_index++)  {
		if (pm_conc_connection_list[conn_index].in_use) {
			vdev_id = pm_conc_connection_list[conn_index].vdev_id;
			policy_mgr_debug("Use vdev_id:%d for opportunistic upgrade",
					 vdev_id);
			break;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
	if (conn_index == MAX_NUMBER_OF_CONC_CONNECTIONS) {
		vdev = wlan_objmgr_pdev_get_first_vdev(pm_ctx->pdev,
						       WLAN_POLICY_MGR_ID);
		if (vdev) {
			vdev_id = wlan_vdev_get_id(vdev);
			wlan_objmgr_vdev_release_ref(vdev, WLAN_POLICY_MGR_ID);
		}
		policy_mgr_debug("Use default vdev_id:%d for opportunistic upgrade",
				 vdev_id);
	}

	return vdev_id;
}

/**
 * pm_dbs_opportunistic_timer_handler() - handler of
 * dbs_opportunistic_timer
 * @data:  context
 *
 * handler for dbs_opportunistic_timer
 *
 * Return: None
 */
void pm_dbs_opportunistic_timer_handler(void *data)
{
	enum policy_mgr_conc_next_action action = PM_NOP;
	uint32_t session_id;
	struct wlan_objmgr_psoc *psoc = (struct wlan_objmgr_psoc *)data;
	enum policy_mgr_conn_update_reason reason;
	struct policy_mgr_psoc_priv_obj *pm_ctx = policy_mgr_get_context(psoc);

	if (!psoc) {
		policy_mgr_err("Invalid Context");
		return;
	}

	/* if we still need it */
	action = policy_mgr_need_opportunistic_upgrade(psoc, &reason);
	policy_mgr_debug("action:%d", action);
	if (!action) {
		return;
	} else if (pm_ctx->hdd_cbacks.hdd_is_cac_in_progress &&
		   pm_ctx->hdd_cbacks.hdd_is_cac_in_progress()) {
		policy_mgr_debug("SAP is in CAC_IN_PROGRESS state, restarting");
		policy_mgr_restart_opportunistic_timer(psoc, false);
		return;
	}
	session_id = pm_get_vdev_id_of_first_conn_idx(psoc);
	policy_mgr_next_actions(psoc, session_id, action,
				reason, POLICY_MGR_DEF_REQ_ID);
}

/**
 * policy_mgr_get_connection_for_vdev_id() - provides the
 * perticular connection with the requested vdev id
 * @vdev_id: vdev id of the connection
 *
 * This function provides the specific connection with the
 * requested vdev id
 *
 * Return: index in the connection table
 */
static uint32_t policy_mgr_get_connection_for_vdev_id(
		struct wlan_objmgr_psoc *psoc, uint32_t vdev_id)
{
	uint32_t conn_index = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return conn_index;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
		 conn_index++) {
		if ((pm_conc_connection_list[conn_index].vdev_id == vdev_id) &&
			pm_conc_connection_list[conn_index].in_use) {
			break;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return conn_index;
}

/**
 * policy_mgr_get_mode() - Get mode from type and subtype
 * @type: type
 * @subtype: subtype
 *
 * Get the concurrency mode from the type and subtype
 * of the interface
 *
 * Return: policy_mgr_con_mode
 */
enum policy_mgr_con_mode policy_mgr_get_mode(uint8_t type,
		uint8_t subtype)
{
	enum policy_mgr_con_mode mode = PM_MAX_NUM_OF_MODE;

	if (type == WMI_VDEV_TYPE_AP) {
		switch (subtype) {
		case 0:
			mode = PM_SAP_MODE;
			break;
		case WMI_UNIFIED_VDEV_SUBTYPE_P2P_GO:
			mode = PM_P2P_GO_MODE;
			break;
		default:
			policy_mgr_err("Unknown subtype %d for type %d",
				subtype, type);
			break;
		}
	} else if (type == WMI_VDEV_TYPE_STA) {
		switch (subtype) {
		case 0:
			mode = PM_STA_MODE;
			break;
		case WMI_UNIFIED_VDEV_SUBTYPE_P2P_CLIENT:
			mode = PM_P2P_CLIENT_MODE;
			break;
		default:
			policy_mgr_err("Unknown subtype %d for type %d",
				subtype, type);
			break;
		}
	} else if (type == WMI_VDEV_TYPE_NAN) {
		mode = PM_NAN_DISC_MODE;
	} else if (type == WMI_VDEV_TYPE_NDI) {
		mode = PM_NDI_MODE;
	} else {
		policy_mgr_err("Unknown type %d", type);
	}

	return mode;
}

/**
 * policy_mgr_get_bw() - Get channel bandwidth type used by WMI
 * @chan_width: channel bandwidth type defined by host
 *
 * Get the channel bandwidth type used by WMI
 *
 * Return: hw_mode_bandwidth
 */
enum hw_mode_bandwidth policy_mgr_get_bw(enum phy_ch_width chan_width)
{
	enum hw_mode_bandwidth bw = HW_MODE_BW_NONE;

	switch (chan_width) {
	case CH_WIDTH_20MHZ:
		bw = HW_MODE_20_MHZ;
		break;
	case CH_WIDTH_40MHZ:
		bw = HW_MODE_40_MHZ;
		break;
	case CH_WIDTH_80MHZ:
		bw = HW_MODE_80_MHZ;
		break;
	case CH_WIDTH_160MHZ:
		bw = HW_MODE_160_MHZ;
		break;
	case CH_WIDTH_80P80MHZ:
		bw = HW_MODE_80_PLUS_80_MHZ;
		break;
	case CH_WIDTH_5MHZ:
		bw = HW_MODE_5_MHZ;
		break;
	case CH_WIDTH_10MHZ:
		bw = HW_MODE_10_MHZ;
		break;
#if defined(WLAN_FEATURE_11BE)
	case CH_WIDTH_320MHZ:
		bw = HW_MODE_320_MHZ;
		break;
#endif
	default:
		policy_mgr_err("Unknown channel BW type %d", chan_width);
		break;
	}

	return bw;
}

enum phy_ch_width policy_mgr_get_ch_width(enum hw_mode_bandwidth bw)
{
	enum phy_ch_width ch_width = CH_WIDTH_INVALID;

	switch (bw) {
	case HW_MODE_20_MHZ:
		ch_width = CH_WIDTH_20MHZ;
		break;
	case HW_MODE_40_MHZ:
		ch_width = CH_WIDTH_40MHZ;
		break;
	case HW_MODE_80_MHZ:
		ch_width = CH_WIDTH_80MHZ;
		break;
	case HW_MODE_160_MHZ:
		ch_width = CH_WIDTH_160MHZ;
		break;
	case HW_MODE_80_PLUS_80_MHZ:
		ch_width = CH_WIDTH_80P80MHZ;
		break;
	case HW_MODE_5_MHZ:
		ch_width = CH_WIDTH_5MHZ;
		break;
	case HW_MODE_10_MHZ:
		ch_width = CH_WIDTH_10MHZ;
		break;
#if defined(WLAN_FEATURE_11BE)
	case HW_MODE_320_MHZ:
		ch_width = CH_WIDTH_320MHZ;
		break;
#endif
	default:
		policy_mgr_err("Invalid phy_ch_width type %d", ch_width);
		break;
	}

	return ch_width;
}

static bool
is_preset_in_chlist(uint32_t chan_freq, const uint32_t *chlist,
		    uint32_t chlist_len)
{
	uint32_t i;

	for (i = 0; i < chlist_len; i++)
		if (chlist[i] == chan_freq)
			return true;

	return false;
}

static void
get_sbs_chlist(struct wlan_objmgr_psoc *psoc,
	       uint32_t *sbs_freqs, uint32_t *sbs_num, uint32_t chan_freq,
	       const uint32_t *chlist1, uint32_t chlist1_len,
	       const uint32_t *chlist2, uint32_t chlist2_len)
{
	uint32_t size_of_sbs = *sbs_num;
	uint32_t i;

	*sbs_num = 0;
	for (i = 0; i < chlist1_len; i++) {
		if (*sbs_num >= size_of_sbs)
			return;
		if (policy_mgr_are_sbs_chan(psoc, chan_freq, chlist1[i]))
			sbs_freqs[(*sbs_num)++] = chlist1[i];
	}
	for (i = 0; i < chlist2_len; i++) {
		if (*sbs_num >= size_of_sbs)
			return;
		if (policy_mgr_are_sbs_chan(psoc, chan_freq, chlist2[i]))
			sbs_freqs[(*sbs_num)++] = chlist2[i];
	}
}

static void
get_rest_chlist(uint32_t *rest_freqs, uint32_t *rest_num,
		uint32_t *scc_freqs, uint32_t scc_num,
		uint32_t *sbs_freqs, uint32_t sbs_num,
		const uint32_t *chlist1, uint32_t chlist1_len,
		const uint32_t *chlist2, uint32_t chlist2_len)
{
	uint32_t size_of_rest = *rest_num;
	uint32_t i;

	*rest_num = 0;
	for (i = 0; i < chlist1_len; i++) {
		if (*rest_num >= size_of_rest)
			return;
		if (is_preset_in_chlist(chlist1[i], scc_freqs, scc_num) ||
		    is_preset_in_chlist(chlist1[i], sbs_freqs, sbs_num))
			continue;
		rest_freqs[(*rest_num)++] = chlist1[i];
	}
	for (i = 0; i < chlist2_len; i++) {
		if (*rest_num >= size_of_rest)
			return;
		if (is_preset_in_chlist(chlist2[i], scc_freqs, scc_num) ||
		    is_preset_in_chlist(chlist2[i], sbs_freqs, sbs_num))
			continue;
		rest_freqs[(*rest_num)++] = chlist2[i];
	}
}

static void
get_sub_channels(struct wlan_objmgr_psoc *psoc,
		 uint32_t *sbs_freqs, uint32_t *sbs_num,
		 uint32_t *scc_freqs, uint32_t *scc_num,
		 uint32_t *rest_freqs, uint32_t *rest_num,
		 const uint32_t *chlist_5g, uint32_t chlist_5g_len,
		 const uint32_t *chlist_6g, uint32_t chlist_6g_len)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t i = 0;
	const uint32_t *chlist1;
	uint8_t chlist1_len;
	const uint32_t *chlist2;
	uint8_t chlist2_len;
	uint32_t size_of_scc = *scc_num;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		*scc_num = 0;
		*sbs_num = 0;
		*rest_num = 0;
		return;
	}

	if (pm_ctx->cfg.pcl_band_priority == POLICY_MGR_PCL_BAND_6G_THEN_5G) {
		chlist1 = chlist_6g;
		chlist1_len = chlist_6g_len;
		chlist2 = chlist_5g;
		chlist2_len = chlist_5g_len;
	} else {
		chlist1 = chlist_5g;
		chlist1_len = chlist_5g_len;
		chlist2 = chlist_6g;
		chlist2_len = chlist_6g_len;
	}
	*scc_num = 0;
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	/* For SCC channels, 6G first then 5G */
	i = 0;
	while (PM_CONC_CONNECTION_LIST_VALID_INDEX(i)) {
		if (WLAN_REG_IS_6GHZ_CHAN_FREQ(
		    pm_conc_connection_list[i].freq)) {
			if (*scc_num < size_of_scc &&
			    !is_preset_in_chlist(
					pm_conc_connection_list[i].freq,
					scc_freqs, *scc_num))
				scc_freqs[(*scc_num)++] =
					pm_conc_connection_list[i].freq;
		}
		i++;
	}
	i = 0;
	while (PM_CONC_CONNECTION_LIST_VALID_INDEX(i)) {
		if (WLAN_REG_IS_5GHZ_CH_FREQ(
		    pm_conc_connection_list[i].freq)) {
			if (*scc_num < size_of_scc &&
			    !is_preset_in_chlist(
					pm_conc_connection_list[i].freq,
					scc_freqs, *scc_num))
				scc_freqs[(*scc_num)++] =
					pm_conc_connection_list[i].freq;
		}
		i++;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	if (policy_mgr_is_hw_sbs_capable(psoc)) {
		if (*scc_num > 1) {
			if (policy_mgr_are_sbs_chan(psoc, scc_freqs[0],
						    scc_freqs[1])) {
				/* 2/3 home channels with SBS separation */
				*sbs_num = 0;
			} else {
				/* SCC or MCC (not SBS separation) */
				get_sbs_chlist(psoc, sbs_freqs, sbs_num,
					       scc_freqs[0],
					       chlist1, chlist1_len,
					       chlist2, chlist2_len);
			}
		} else if (*scc_num > 0) {
			get_sbs_chlist(psoc, sbs_freqs, sbs_num, scc_freqs[0],
				       chlist1, chlist1_len,
				       chlist2, chlist2_len);
		}
	} else {
		*sbs_num = 0;
	}

	get_rest_chlist(rest_freqs, rest_num, scc_freqs, *scc_num,
			sbs_freqs, *sbs_num, chlist1, chlist1_len,
			chlist2, chlist2_len);
}

static void
add_chlist_to_pcl(struct wlan_objmgr_pdev *pdev,
		  uint32_t *pcl_freqs, uint8_t *pcl_weights,
		  uint32_t pcl_sz, uint32_t *index, uint32_t weight,
		  const uint32_t *chlist, uint8_t chlist_len,
		  bool skip_dfs_channel, bool skip_6gh_channel)
{
	uint32_t i;

	for (i = 0; i < chlist_len && *index < pcl_sz; i++) {
		if (skip_dfs_channel &&
		    wlan_reg_is_dfs_for_freq(pdev, chlist[i]))
			continue;
		if (skip_6gh_channel &&
		    WLAN_REG_IS_6GHZ_CHAN_FREQ(chlist[i]))
			continue;
		pcl_freqs[*index] = chlist[i];
		pcl_weights[*index] = weight;
		(*index)++;
	}
	policy_mgr_debug("Add chlist len %d index %d",
			 chlist_len, *index);
}

/**
 * policy_mgr_get_connection_channels() - provides the channel(s)
 * on which current connection(s) is
 * @psoc: psoc object
 * @mode: conn mode
 * @order: no order OR 2.4 Ghz channel followed by 5 Ghz channel OR
 *  5 Ghz channel followed by 2.4 Ghz channel
 * @skip_dfs_channel: if this flag is true then skip the dfs channel
 * @group_id: Next available groups for weight assignment
 * @pcl_freqs: Pointer to the frequencies of PCL
 * @pcl_weights: Pointer to the weights of PCL
 * @pcl_sz: Max length of the PCL list
 * @index: Index from which the PCL list needs to be populated,
 *  will increase accordingly if any channel is obtained
 *
 * This function provides the channel(s) on which current
 * connection(s) is/are
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
policy_mgr_get_connection_channels(struct wlan_objmgr_psoc *psoc,
				   enum policy_mgr_con_mode mode,
				   enum policy_mgr_pcl_channel_order order,
				   bool skip_dfs_channel,
				   enum policy_mgr_pcl_group_id group_id,
				   uint32_t *pcl_freqs, uint8_t *pcl_weights,
				   uint32_t pcl_sz, uint32_t *index)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t conn_index = 0;
	uint32_t weight1, weight2;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	struct policy_mgr_conc_connection_info *cl;
	bool add_6ghz = true;
	uint32_t idx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}

	if (!pcl_freqs || !pcl_weights || !index || !pcl_sz) {
		policy_mgr_err("list or index is NULL");
		status = QDF_STATUS_E_INVAL;
		return status;
	}

	idx = *index;

	/* POLICY_MGR_PCL_GROUP_ID1_ID2 indicates that all three weights are
	 * available for assignment. i.e., WEIGHT_OF_GROUP1_PCL_CHANNELS,
	 * WEIGHT_OF_GROUP2_PCL_CHANNELS and WEIGHT_OF_GROUP3_PCL_CHANNELS
	 * are all available. Since in this function only two weights are
	 * assigned at max, only group1 and group2 weights are considered.
	 *
	 * The other possible group id POLICY_MGR_PCL_GROUP_ID2_ID3 indicates
	 * group1 was assigned the weight WEIGHT_OF_GROUP1_PCL_CHANNELS and
	 * only weights WEIGHT_OF_GROUP2_PCL_CHANNELS and
	 * WEIGHT_OF_GROUP3_PCL_CHANNELS are available for further weight
	 * assignments.
	 *
	 * e.g., when order is POLICY_MGR_PCL_ORDER_24G_THEN_5G and group id is
	 * POLICY_MGR_PCL_GROUP_ID2_ID3, WEIGHT_OF_GROUP2_PCL_CHANNELS is
	 * assigned to 2.4GHz channels and the weight
	 * WEIGHT_OF_GROUP3_PCL_CHANNELS is assigned to the 5GHz channels.
	 */
	if (group_id == POLICY_MGR_PCL_GROUP_ID1_ID2) {
		weight1 = WEIGHT_OF_GROUP1_PCL_CHANNELS;
		weight2 = WEIGHT_OF_GROUP2_PCL_CHANNELS;
	} else if (group_id == POLICY_MGR_PCL_GROUP_ID2_ID3) {
		weight1 = WEIGHT_OF_GROUP2_PCL_CHANNELS;
		weight2 = WEIGHT_OF_GROUP3_PCL_CHANNELS;
	} else {
		weight1 = WEIGHT_OF_GROUP3_PCL_CHANNELS;
		weight2 = WEIGHT_OF_GROUP4_PCL_CHANNELS;
	}
	if (!policy_mgr_is_6ghz_conc_mode_supported(psoc, mode))
		add_6ghz = false;

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	cl = pm_conc_connection_list;
	if (POLICY_MGR_PCL_ORDER_NONE == order) {
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			bool is_6ghz_ch = WLAN_REG_IS_6GHZ_CHAN_FREQ(
				cl[conn_index].freq);
			if (skip_dfs_channel && wlan_reg_is_dfs_for_freq(
			    pm_ctx->pdev, cl[conn_index].freq)) {
				conn_index++;
			} else if ((idx < pcl_sz) &&
				   (!is_6ghz_ch || add_6ghz)) {
				pcl_freqs[idx] = cl[conn_index++].freq;
				pcl_weights[idx] = weight1;
				idx++;
			} else {
				conn_index++;
			}
		}
	} else if (POLICY_MGR_PCL_ORDER_24G_THEN_5G == order) {
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			if (WLAN_REG_IS_24GHZ_CH_FREQ(cl[conn_index].freq) &&
			    idx < pcl_sz) {
				pcl_freqs[idx] = cl[conn_index++].freq;
				pcl_weights[idx] = weight1;
				idx++;
			} else {
				conn_index++;
			}
		}

		conn_index = 0;
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			if (skip_dfs_channel &&
			    wlan_reg_is_dfs_for_freq(pm_ctx->pdev,
						     cl[conn_index].freq)) {
				conn_index++;
			} else if (WLAN_REG_IS_5GHZ_CH_FREQ(
					cl[conn_index].freq) &&
				   (idx < pcl_sz)) {
				pcl_freqs[idx] = cl[conn_index++].freq;
				pcl_weights[idx] = weight2;
				idx++;
			} else {
				conn_index++;
			}
		}
		conn_index = 0;
		while (add_6ghz &&
		       PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			bool is_6ghz_ch = WLAN_REG_IS_6GHZ_CHAN_FREQ(
				cl[conn_index].freq);
			if (is_6ghz_ch && idx < pcl_sz) {
				pcl_freqs[idx] = cl[conn_index++].freq;
				pcl_weights[idx] = weight2;
				idx++;
			} else {
				conn_index++;
			}
		}
	} else if (POLICY_MGR_PCL_ORDER_5G_THEN_2G == order) {
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			if (skip_dfs_channel &&
			    wlan_reg_is_dfs_for_freq(
			    pm_ctx->pdev, cl[conn_index].freq)) {
				conn_index++;
			} else if (WLAN_REG_IS_5GHZ_CH_FREQ(
					cl[conn_index].freq) &&
				   (idx < pcl_sz)) {
				pcl_freqs[idx] = cl[conn_index++].freq;
				pcl_weights[idx] = weight1;
				idx++;
			} else {
				conn_index++;
			}
		}
		conn_index = 0;
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			if (WLAN_REG_IS_24GHZ_CH_FREQ(cl[conn_index].freq) &&
			    idx < pcl_sz) {
				pcl_freqs[idx] = cl[conn_index++].freq;
				pcl_weights[idx] = weight2;
				idx++;

			} else {
				conn_index++;
			}
		}
		conn_index = 0;
		while (add_6ghz &&
		       PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			bool is_6ghz_ch = WLAN_REG_IS_6GHZ_CHAN_FREQ(
				cl[conn_index].freq);
			if (is_6ghz_ch && idx < pcl_sz) {
				pcl_freqs[idx] = cl[conn_index++].freq;
				pcl_weights[idx] = weight2;
				idx++;
			} else {
				conn_index++;
			}
		}
	} else {
		policy_mgr_err("unknown order %d", order);
		status = QDF_STATUS_E_FAILURE;
	}

	*index = idx;
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return status;
}

/**
 * policy_mgr_set_weight_of_dfs_passive_channels_to_zero() - set weight of dfs
 * and passive channels to 0
 * @psoc: pointer to soc
 * @pcl_channels: preferred channel list
 * @len: length of preferred channel list
 * @weight_list: preferred channel weight list
 * @weight_len: length of weight list
 * This function set the weight of dfs and passive channels to 0
 *
 * Return: None
 */
void policy_mgr_set_weight_of_dfs_passive_channels_to_zero(
		struct wlan_objmgr_psoc *psoc, uint32_t *pcl_channels,
		uint32_t *len, uint8_t *weight_list, uint32_t weight_len)
{
	uint8_t i;
	uint32_t orig_channel_count = 0;
	bool sta_sap_scc_on_dfs_chan;
	uint32_t sap_count;
	enum channel_state channel_state;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	sta_sap_scc_on_dfs_chan =
		policy_mgr_is_sta_sap_scc_allowed_on_dfs_chan(psoc);
	sap_count = policy_mgr_mode_specific_connection_count(psoc,
			PM_SAP_MODE, NULL);
	policy_mgr_debug("sta_sap_scc_on_dfs_chan %u, sap_count %u",
			 sta_sap_scc_on_dfs_chan, sap_count);

	if (!sta_sap_scc_on_dfs_chan || !sap_count)
		return;

	if (len)
		orig_channel_count = QDF_MIN(*len, NUM_CHANNELS);
	else {
		policy_mgr_err("invalid number of channel length");
		return;
	}

	policy_mgr_debug("Set weight of DFS/passive channels to 0");

	for (i = 0; i < orig_channel_count; i++) {
		channel_state = wlan_reg_get_channel_state_for_freq(
						pm_ctx->pdev, pcl_channels[i]);
		if ((channel_state == CHANNEL_STATE_DISABLE) ||
				(channel_state == CHANNEL_STATE_INVALID))
			/* Set weight of inactive channels to 0 */
			weight_list[i] = 0;
	}

	return;
}

/**
 * policy_mgr_add_5g_to_pcl() - add the 5G/6G channels into PCL
 * @psoc: psoc object
 * @pcl_freqs: Pointer to the frequencies of PCL
 * @pcl_weights: Pointer to the weights of PCL
 * @pcl_sz: Max length of the PCL list
 * @index: Index from which the PCL list needs to be populated,
 *  will increase accordingly if any channel is obtained
 * @group_id: Next available groups for weight assignment
 * @chlist_5g: Pointer to the 5G channel list
 * @chlist_5g_len: Length of the 5G channel list
 * @chlist_6g: Pointer to the 6G channel list
 * @chlist_6g_len: Length of the 6G channel list
 *
 * Return: None
 */
static void
policy_mgr_add_5g_to_pcl(struct wlan_objmgr_psoc *psoc, uint32_t *pcl_freqs,
			 uint8_t *pcl_weights, uint32_t pcl_sz, uint32_t *index,
			 enum policy_mgr_pcl_group_id group_id,
			 const uint32_t *chlist_5g, uint8_t chlist_5g_len,
			 const uint32_t *chlist_6g, uint8_t chlist_6g_len)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t weight1, weight2;
	const uint32_t *chlist1;
	uint8_t chlist1_len;
	const uint32_t *chlist2;
	uint8_t chlist2_len;
	uint32_t i;
	uint32_t len = 0, idx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	idx = *index;

	if (group_id == POLICY_MGR_PCL_GROUP_ID1_ID2) {
		weight1 = WEIGHT_OF_GROUP1_PCL_CHANNELS;
		weight2 = WEIGHT_OF_GROUP2_PCL_CHANNELS;
	} else if (group_id == POLICY_MGR_PCL_GROUP_ID2_ID3) {
		weight1 = WEIGHT_OF_GROUP2_PCL_CHANNELS;
		weight2 = WEIGHT_OF_GROUP3_PCL_CHANNELS;
	} else {
		weight1 = WEIGHT_OF_GROUP3_PCL_CHANNELS;
		weight2 = WEIGHT_OF_GROUP4_PCL_CHANNELS;
	}
	if (pm_ctx->cfg.pcl_band_priority == POLICY_MGR_PCL_BAND_6G_THEN_5G) {
		chlist1 = chlist_6g;
		chlist1_len = chlist_6g_len;
		chlist2 = chlist_5g;
		chlist2_len = chlist_5g_len;
	} else {
		chlist1 = chlist_5g;
		chlist1_len = chlist_5g_len;
		chlist2 = chlist_6g;
		chlist2_len = chlist_6g_len;
	}
	if ((chlist1_len + idx) > pcl_sz) {
		policy_mgr_err("no enough weight len %d chlist1_len %d %d",
			       pcl_sz, chlist1_len, idx);
		return;
	}
	qdf_mem_copy(&pcl_freqs[idx], chlist1, chlist1_len * sizeof(*chlist1));
	for (i = 0; i < chlist1_len; i++)
		pcl_weights[idx++] = weight1;

	len += chlist1_len;

	if ((chlist2_len + idx) > pcl_sz) {
		policy_mgr_err("no enough weight len chlist2_len %d %d %d",
			       pcl_sz, chlist2_len, idx);
		return;
	}
	qdf_mem_copy(&pcl_freqs[idx], chlist2,
		     chlist2_len * sizeof(*chlist2));
	for (i = 0; i < chlist2_len; i++)
		pcl_weights[idx++] = weight2;
	len += chlist2_len;

	*index = idx;
	policy_mgr_debug("Add 5g chlist len %d 6g chlist len %d len %d index %d order %d",
			 chlist_5g_len, chlist_6g_len, len, idx,
			 pm_ctx->cfg.pcl_band_priority);
}

/**
 * policy_mgr_add_24g_to_pcl() - add the 2.4G channels into PCL
 * @pcl_freqs: Pointer to the frequencies of PCL
 * @pcl_weights: Pointer to the weights of PCL
 * @pcl_sz: Max length of the PCL list
 * @index: Index from which the PCL list needs to be populated,
 *  will increase accordingly if any channel is obtained
 * @weight: group for weight assignment
 * @chlist_24g: Pointer to the 2.4G channel list
 * @chlist_24g_len: Length of the 2.4G channel list
 *
 * Return: None
 */
static void
policy_mgr_add_24g_to_pcl(uint32_t *pcl_freqs, uint8_t *pcl_weights,
			  uint32_t pcl_sz, uint32_t *index, uint32_t weight,
			  const uint32_t *chlist_24g, uint8_t chlist_24g_len)
{
	uint32_t num_to_add, i;

	if (*index >= NUM_CHANNELS || *index >= pcl_sz)
		return;
	num_to_add = QDF_MIN((*index + chlist_24g_len), pcl_sz) - *index;
	for (i = 0; i < num_to_add; i++) {
		if ((i + *index) >= NUM_CHANNELS || (i + *index) >= pcl_sz)
			break;
		pcl_weights[i + *index] = weight;
		pcl_freqs[i + *index] = chlist_24g[i];
	}

	*index = i;
	policy_mgr_debug("Add 24g chlist len %d len %d index %d",
			 chlist_24g_len, num_to_add, *index);
}

/**
 * policy_mgr_get_channel_list() - provides the channel list
 * suggestion for new connection
 * @pcl: The preferred channel list enum
 * @mode: concurrency mode for which channel list is requested
 * @pcl_channels: PCL channels
 * @pcl_weights: Weights of the PCL
 * @pcl_sz: Max length of the PCL list
 * @len: length of the PCL obtained
 *
 * This function provides the actual channel list based on the
 * current regulatory domain derived using preferred channel
 * list enum obtained from one of the pcl_table
 *
 * Return: Channel List
 */
QDF_STATUS policy_mgr_get_channel_list(struct wlan_objmgr_psoc *psoc,
				       enum policy_mgr_pcl_type pcl,
				       enum policy_mgr_con_mode mode,
				       uint32_t *pcl_channels,
				       uint8_t *pcl_weights,
				       uint32_t pcl_sz, uint32_t *len)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t num_channels = 0;
	uint32_t chan_index_24 = 0, chan_index_5 = 0, chan_index_6 = 0;
	bool skip_dfs_channel = false;
	uint32_t i = 0;
	bool skip_6ghz_channel = false;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t *channel_list, *channel_list_24, *channel_list_5,
		 *sbs_freqs, *channel_list_6, *scc_freqs, *rest_freqs;
	uint32_t sbs_num, scc_num, rest_num;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}

	if ((!pcl_channels) || (!len)) {
		policy_mgr_err("pcl_channels or len is NULL");
		return status;
	}

	*len = 0;
	if (PM_MAX_PCL_TYPE == pcl) {
		/* msg */
		policy_mgr_err("pcl is invalid");
		return status;
	}

	if (PM_NONE == pcl) {
		/* msg */
		policy_mgr_debug("pcl is 0");
		return QDF_STATUS_SUCCESS;
	}

	channel_list = qdf_mem_malloc(NUM_CHANNELS * sizeof(uint32_t));
	channel_list_24 = qdf_mem_malloc(NUM_CHANNELS * sizeof(uint32_t));
	channel_list_5 = qdf_mem_malloc(NUM_CHANNELS * sizeof(uint32_t));
	sbs_freqs = qdf_mem_malloc(NUM_CHANNELS * sizeof(uint32_t));
	channel_list_6 = qdf_mem_malloc(NUM_CHANNELS * sizeof(uint32_t));
	rest_freqs = qdf_mem_malloc(NUM_CHANNELS * sizeof(uint32_t));
	scc_freqs = qdf_mem_malloc(
			MAX_NUMBER_OF_CONC_CONNECTIONS * sizeof(uint32_t));
	if (!channel_list || !channel_list_24 || !channel_list_5 ||
	    !sbs_freqs || !channel_list_6 || !rest_freqs || !scc_freqs) {
		status = QDF_STATUS_E_NOMEM;
		goto end;
	}
	/* get the channel list for current domain */
	status = policy_mgr_get_valid_chans(psoc, channel_list,
					    &num_channels);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("Error in getting valid channels");
		goto end;
	}

	if ((mode == PM_SAP_MODE) || (mode == PM_P2P_GO_MODE))
		policy_mgr_skip_dfs_ch(psoc,
				       &skip_dfs_channel);

	/* Let's divide the list in 2.4 & 5 Ghz lists */
	for (i = 0; i < num_channels; i++) {
		if (wlan_reg_is_24ghz_ch_freq(channel_list[i])) {
			channel_list_24[chan_index_24++] = channel_list[i];
		} else if (wlan_reg_is_5ghz_ch_freq(channel_list[i])) {
			if ((true == skip_dfs_channel) &&
			    wlan_reg_is_dfs_for_freq(pm_ctx->pdev,
						     channel_list[i]))
				continue;

			channel_list_5[chan_index_5++] = channel_list[i];
		} else if (wlan_reg_is_6ghz_chan_freq(channel_list[i])) {
			/* Add to 5G list untill 6G conc support is enabled */
			channel_list_6[chan_index_6++] = channel_list[i];
		}
	}
	if (!policy_mgr_is_6ghz_conc_mode_supported(psoc, mode)) {
		chan_index_6 = 0;
		skip_6ghz_channel = true;
	}
	sbs_num = NUM_CHANNELS;
	scc_num = MAX_NUMBER_OF_CONC_CONNECTIONS;
	rest_num = NUM_CHANNELS;

	/* In the below switch case, the channel list is populated based on the
	 * pcl. e.g., if the pcl is PM_SCC_CH_24G, the SCC channel group is
	 * populated first followed by the 2.4GHz channel group. Along with
	 * this, the weights are also populated in the same order for each of
	 * these groups. There are three weight groups:
	 * WEIGHT_OF_GROUP1_PCL_CHANNELS, WEIGHT_OF_GROUP2_PCL_CHANNELS and
	 * WEIGHT_OF_GROUP3_PCL_CHANNELS.
	 *
	 * e.g., if pcl is PM_SCC_ON_5_SCC_ON_24_24G: scc on 5GHz (group1)
	 * channels take the weight WEIGHT_OF_GROUP1_PCL_CHANNELS, scc on 2.4GHz
	 * (group2) channels take the weight WEIGHT_OF_GROUP2_PCL_CHANNELS and
	 * 2.4GHz (group3) channels take the weight
	 * WEIGHT_OF_GROUP3_PCL_CHANNELS.
	 *
	 * When the weight to be assigned to the group is known along with the
	 * number of channels, the weights are directly assigned to the
	 * pcl_weights list. But, the channel list is populated using
	 * policy_mgr_get_connection_channels(), the order of weights to be used
	 * is passed as an argument to the function
	 * policy_mgr_get_connection_channels() using
	 * 'enum policy_mgr_pcl_group_id' which indicates the next available
	 * weights to be used and policy_mgr_get_connection_channels() will take
	 * care of the weight assignments.
	 *
	 * e.g., 'enum policy_mgr_pcl_group_id' value of
	 * POLICY_MGR_PCL_GROUP_ID2_ID3 indicates that the next available groups
	 * for weight assignment are WEIGHT_OF_GROUP2_PCL_CHANNELS and
	 * WEIGHT_OF_GROUP3_PCL_CHANNELS and that the
	 * weight WEIGHT_OF_GROUP1_PCL_CHANNELS was already allocated.
	 * So, in the same example, when order is
	 * POLICY_MGR_PCL_ORDER_24G_THEN_5G,
	 * policy_mgr_get_connection_channels() will assign the weight
	 * WEIGHT_OF_GROUP2_PCL_CHANNELS to 2.4GHz channels and assign the
	 * weight WEIGHT_OF_GROUP3_PCL_CHANNELS to 5GHz channels.
	 */
	switch (pcl) {
	case PM_24G:
		policy_mgr_add_24g_to_pcl(pcl_channels, pcl_weights, pcl_sz,
					  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
					  channel_list_24, chan_index_24);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_5G:
		policy_mgr_add_5g_to_pcl(psoc, pcl_channels, pcl_weights,
					 pcl_sz, len,
					 POLICY_MGR_PCL_GROUP_ID1_ID2,
					 channel_list_5, chan_index_5,
					 channel_list_6, chan_index_6);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_CH:
	case PM_MCC_CH:
		policy_mgr_get_connection_channels(psoc, mode,
						   POLICY_MGR_PCL_ORDER_NONE,
						   skip_dfs_channel,
						   POLICY_MGR_PCL_GROUP_ID1_ID2,
						   pcl_channels, pcl_weights,
						   pcl_sz, len);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_CH_24G:
	case PM_MCC_CH_24G:
		policy_mgr_get_connection_channels(psoc, mode,
						   POLICY_MGR_PCL_ORDER_NONE,
						   skip_dfs_channel,
						   POLICY_MGR_PCL_GROUP_ID1_ID2,
						   pcl_channels, pcl_weights,
						   pcl_sz, len);
		policy_mgr_add_24g_to_pcl(pcl_channels, pcl_weights, pcl_sz,
					  len, WEIGHT_OF_GROUP2_PCL_CHANNELS,
					  channel_list_24, chan_index_24);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_CH_5G:
	case PM_MCC_CH_5G:
		policy_mgr_get_connection_channels(psoc, mode,
						   POLICY_MGR_PCL_ORDER_NONE,
						   skip_dfs_channel,
						   POLICY_MGR_PCL_GROUP_ID1_ID2,
						   pcl_channels, pcl_weights,
						   pcl_sz, len);
		policy_mgr_add_5g_to_pcl(psoc, pcl_channels, pcl_weights,
					 pcl_sz, len,
					 POLICY_MGR_PCL_GROUP_ID2_ID3,
					 channel_list_5, chan_index_5,
					 channel_list_6, chan_index_6);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_24G_SCC_CH:
	case PM_24G_MCC_CH:
		policy_mgr_add_24g_to_pcl(pcl_channels, pcl_weights, pcl_sz,
					  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
					  channel_list_24, chan_index_24);
		policy_mgr_get_connection_channels(psoc, mode,
						   POLICY_MGR_PCL_ORDER_NONE,
						   skip_dfs_channel,
						   POLICY_MGR_PCL_GROUP_ID2_ID3,
						   pcl_channels, pcl_weights,
						   pcl_sz, len);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_5G_SCC_CH:
	case PM_5G_MCC_CH:
		policy_mgr_add_5g_to_pcl(psoc, pcl_channels, pcl_weights,
					 pcl_sz, len,
					 POLICY_MGR_PCL_GROUP_ID1_ID2,
					 channel_list_5, chan_index_5,
					 channel_list_6, chan_index_6);
		policy_mgr_get_connection_channels(psoc, mode,
						   POLICY_MGR_PCL_ORDER_NONE,
						   skip_dfs_channel,
						   POLICY_MGR_PCL_GROUP_ID3_ID4,
						   pcl_channels, pcl_weights,
						   pcl_sz, len);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_24_SCC_ON_5:
		policy_mgr_get_connection_channels(
					psoc, mode,
					POLICY_MGR_PCL_ORDER_24G_THEN_5G,
					skip_dfs_channel,
					POLICY_MGR_PCL_GROUP_ID1_ID2,
					pcl_channels, pcl_weights, pcl_sz, len);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_5_SCC_ON_24:
		policy_mgr_get_connection_channels(
					psoc, mode,
					POLICY_MGR_PCL_ORDER_5G_THEN_2G,
					skip_dfs_channel,
					POLICY_MGR_PCL_GROUP_ID1_ID2,
					pcl_channels, pcl_weights, pcl_sz, len);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_24_SCC_ON_5_24G:
		policy_mgr_get_connection_channels(
					psoc, mode,
					POLICY_MGR_PCL_ORDER_24G_THEN_5G,
					skip_dfs_channel,
					POLICY_MGR_PCL_GROUP_ID1_ID2,
					pcl_channels, pcl_weights, pcl_sz, len);
		policy_mgr_add_24g_to_pcl(pcl_channels, pcl_weights, pcl_sz,
					  len, WEIGHT_OF_GROUP3_PCL_CHANNELS,
					  channel_list_24, chan_index_24);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_24_SCC_ON_5_5G:
		policy_mgr_get_connection_channels(
					psoc, mode,
					POLICY_MGR_PCL_ORDER_24G_THEN_5G,
					skip_dfs_channel,
					POLICY_MGR_PCL_GROUP_ID1_ID2,
					pcl_channels, pcl_weights, pcl_sz, len);
		policy_mgr_add_5g_to_pcl(psoc, pcl_channels, pcl_weights,
					 pcl_sz, len,
					 POLICY_MGR_PCL_GROUP_ID3_ID4,
					 channel_list_5, chan_index_5,
					 channel_list_6, chan_index_6);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_5_SCC_ON_24_24G:
		policy_mgr_get_connection_channels(
					psoc, mode,
					POLICY_MGR_PCL_ORDER_5G_THEN_2G,
					skip_dfs_channel,
					POLICY_MGR_PCL_GROUP_ID1_ID2,
					pcl_channels, pcl_weights, pcl_sz, len);
		policy_mgr_add_24g_to_pcl(pcl_channels, pcl_weights, pcl_sz,
					  len, WEIGHT_OF_GROUP3_PCL_CHANNELS,
					  channel_list_24, chan_index_24);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_5_SCC_ON_24_5G:
		policy_mgr_get_connection_channels(
					psoc, mode,
					POLICY_MGR_PCL_ORDER_5G_THEN_2G,
					skip_dfs_channel,
					POLICY_MGR_PCL_GROUP_ID1_ID2,
					pcl_channels, pcl_weights, pcl_sz, len);
		policy_mgr_add_5g_to_pcl(psoc, pcl_channels, pcl_weights,
					 pcl_sz, len,
					 POLICY_MGR_PCL_GROUP_ID3_ID4,
					 channel_list_5, chan_index_5,
					 channel_list_6, chan_index_6);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_24G_SCC_CH_SBS_CH:
		get_sub_channels(psoc,
				 sbs_freqs, &sbs_num,
				 scc_freqs, &scc_num,
				 rest_freqs, &rest_num,
				 channel_list_5, chan_index_5,
				 channel_list_6, chan_index_6);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
				  channel_list_24, chan_index_24,
				  false, false);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP2_PCL_CHANNELS,
				  scc_freqs, scc_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP3_PCL_CHANNELS,
				  sbs_freqs, sbs_num,
				  skip_dfs_channel, skip_6ghz_channel);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_24G_SCC_CH_SBS_CH_5G:
		get_sub_channels(psoc,
				 sbs_freqs, &sbs_num,
				 scc_freqs, &scc_num,
				 rest_freqs, &rest_num,
				 channel_list_5, chan_index_5,
				 channel_list_6, chan_index_6);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
				  channel_list_24, chan_index_24,
				  false, false);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP2_PCL_CHANNELS,
				  scc_freqs, scc_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP3_PCL_CHANNELS,
				  sbs_freqs, sbs_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP4_PCL_CHANNELS,
				  rest_freqs, rest_num,
				  skip_dfs_channel, skip_6ghz_channel);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_24G_SBS_CH_MCC_CH:
		get_sub_channels(psoc,
				 sbs_freqs, &sbs_num,
				 scc_freqs, &scc_num,
				 rest_freqs, &rest_num,
				 channel_list_5, chan_index_5,
				 channel_list_6, chan_index_6);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
				  channel_list_24, chan_index_24,
				  false, false);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP2_PCL_CHANNELS,
				  sbs_freqs, sbs_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP3_PCL_CHANNELS,
				  scc_freqs, scc_num,
				  skip_dfs_channel, skip_6ghz_channel);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SBS_CH:
		get_sub_channels(psoc,
				 sbs_freqs, &sbs_num,
				 scc_freqs, &scc_num,
				 rest_freqs, &rest_num,
				 channel_list_5, chan_index_5,
				 channel_list_6, chan_index_6);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
				  sbs_freqs, sbs_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP2_PCL_CHANNELS,
				  scc_freqs, scc_num,
				  skip_dfs_channel, skip_6ghz_channel);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SBS_CH_5G:
		get_sub_channels(psoc,
				 sbs_freqs, &sbs_num,
				 scc_freqs, &scc_num,
				 rest_freqs, &rest_num,
				 channel_list_5, chan_index_5,
				 channel_list_6, chan_index_6);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
				  sbs_freqs, sbs_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP2_PCL_CHANNELS,
				  scc_freqs, scc_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP3_PCL_CHANNELS,
				  rest_freqs, rest_num,
				  skip_dfs_channel, skip_6ghz_channel);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SBS_CH_24G_SCC_CH:
		get_sub_channels(psoc,
				 sbs_freqs, &sbs_num,
				 scc_freqs, &scc_num,
				 rest_freqs, &rest_num,
				 channel_list_5, chan_index_5,
				 channel_list_6, chan_index_6);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
				  sbs_freqs, sbs_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP2_PCL_CHANNELS,
				  channel_list_24, chan_index_24,
				  false, false);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP3_PCL_CHANNELS,
				  scc_freqs, scc_num,
				  skip_dfs_channel, skip_6ghz_channel);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SBS_CH_SCC_CH_24G:
		get_sub_channels(psoc,
				 sbs_freqs, &sbs_num,
				 scc_freqs, &scc_num,
				 rest_freqs, &rest_num,
				 channel_list_5, chan_index_5,
				 channel_list_6, chan_index_6);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
				  sbs_freqs, sbs_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP2_PCL_CHANNELS,
				  scc_freqs, scc_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP3_PCL_CHANNELS,
				  channel_list_24, chan_index_24,
				  false, false);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_CH_SBS_CH_24G:
		get_sub_channels(psoc,
				 sbs_freqs, &sbs_num,
				 scc_freqs, &scc_num,
				 rest_freqs, &rest_num,
				 channel_list_5, chan_index_5,
				 channel_list_6, chan_index_6);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
				  scc_freqs, scc_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP2_PCL_CHANNELS,
				  sbs_freqs, sbs_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP3_PCL_CHANNELS,
				  channel_list_24, chan_index_24,
				  false, false);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SBS_CH_SCC_CH_5G_24G:
		get_sub_channels(psoc,
				 sbs_freqs, &sbs_num,
				 scc_freqs, &scc_num,
				 rest_freqs, &rest_num,
				 channel_list_5, chan_index_5,
				 channel_list_6, chan_index_6);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
				  sbs_freqs, sbs_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP2_PCL_CHANNELS,
				  scc_freqs, scc_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP3_PCL_CHANNELS,
				  rest_freqs, rest_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP4_PCL_CHANNELS,
				  channel_list_24, chan_index_24,
				  false, false);
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_CH_MCC_CH_SBS_CH_24G:
		get_sub_channels(psoc,
				 sbs_freqs, &sbs_num,
				 scc_freqs, &scc_num,
				 rest_freqs, &rest_num,
				 channel_list_5, chan_index_5,
				 channel_list_6, chan_index_6);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP1_PCL_CHANNELS,
				  scc_freqs, scc_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP2_PCL_CHANNELS,
				  rest_freqs, rest_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP3_PCL_CHANNELS,
				  sbs_freqs, sbs_num,
				  skip_dfs_channel, skip_6ghz_channel);
		add_chlist_to_pcl(pm_ctx->pdev,
				  pcl_channels, pcl_weights, pcl_sz,
				  len, WEIGHT_OF_GROUP4_PCL_CHANNELS,
				  channel_list_24, chan_index_24,
				  false, false);
		status = QDF_STATUS_SUCCESS;
		break;
	default:
		policy_mgr_err("unknown pcl value %d", pcl);
		break;
	}

	policy_mgr_debug("pcl %s: mode %s", pcl_type_to_string(pcl),
			 device_mode_to_string(mode));
	policy_mgr_debug("pcl len %d and weight list sz %d",
			 *len, pcl_sz);

	/* check the channel avoidance list for beaconing entities */
	if ((mode == PM_SAP_MODE) || (mode == PM_P2P_GO_MODE))
		policy_mgr_update_with_safe_channel_list(psoc, pcl_channels,
							 len, pcl_weights,
							 pcl_sz);

	policy_mgr_set_weight_of_dfs_passive_channels_to_zero(psoc,
			pcl_channels, len, pcl_weights, pcl_sz);
end:
	qdf_mem_free(channel_list);
	qdf_mem_free(channel_list_24);
	qdf_mem_free(channel_list_5);
	qdf_mem_free(sbs_freqs);
	qdf_mem_free(channel_list_6);
	qdf_mem_free(scc_freqs);
	qdf_mem_free(rest_freqs);

	return status;
}

bool policy_mgr_disallow_mcc(struct wlan_objmgr_psoc *psoc,
			     uint32_t ch_freq)
{
	uint32_t index = 0;
	bool match = false;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return match;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	while (PM_CONC_CONNECTION_LIST_VALID_INDEX(index)) {
		if (policy_mgr_is_hw_dbs_capable(psoc) == false) {
			if (pm_conc_connection_list[index].freq != ch_freq) {
				match = true;
				break;
			}
		} else if (WLAN_REG_IS_5GHZ_CH_FREQ
			(pm_conc_connection_list[index].freq)) {
			if (pm_conc_connection_list[index].freq != ch_freq &&
			    !policy_mgr_are_sbs_chan(psoc, ch_freq,
					pm_conc_connection_list[index].freq)) {
				match = true;
				break;
			}
		}
		index++;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return match;
}

/**
 * policy_mgr_allow_same_mac_diff_freq() - Check whether diff freq are allowed
 * on same mac
 *
 * @psoc: Pointer to Psoc
 * @ch_freq: channel frequency
 *
 * Check whether diff freq are allowed on same mac
 *
 * Return: True/False
 */
static
bool policy_mgr_allow_same_mac_diff_freq(struct wlan_objmgr_psoc *psoc,
					 qdf_freq_t ch_freq)
{
	bool allow = true;

	if ((pm_conc_connection_list[0].mode == PM_NAN_DISC_MODE &&
	     pm_conc_connection_list[1].mode == PM_NDI_MODE) ||
	    (pm_conc_connection_list[0].mode == PM_NDI_MODE &&
	     pm_conc_connection_list[1].mode == PM_NAN_DISC_MODE)) {
		/*
		 * NAN + NDI are managed in Firmware by dividing
		 * up slots. Connection on NDI is re-negotiable
		 * and therefore a 3rd connection with the
		 * same MAC is possible.
		 */
	} else if (!policy_mgr_is_hw_dbs_capable(psoc) &&
		    policy_mgr_is_interband_mcc_supported(psoc)) {
		if (ch_freq !=  pm_conc_connection_list[0].freq &&
		    ch_freq !=  pm_conc_connection_list[1].freq) {
			policy_mgr_rl_debug("don't allow 3rd home channel on same MAC");
			allow = false;
		}
	} else if (policy_mgr_are_3_freq_on_same_mac(psoc, ch_freq,
					pm_conc_connection_list[0].freq,
					pm_conc_connection_list[1].freq)) {
			policy_mgr_rl_debug("don't allow 3rd home channel on same MAC");
			allow = false;
	}

	return allow;
}

/**
 * policy_mgr_allow_same_mac_same_freq() - check whether given frequency is
 * allowed for same mac
 *
 * @psoc: Pointer to Psoc
 * @ch_freq: channel frequency
 * @mode: Concurrency Mode
 *
 * check whether given frequency is allowed for same mac
 *
 * Return: True/False
 */
static
bool policy_mgr_allow_same_mac_same_freq(struct wlan_objmgr_psoc *psoc,
					 qdf_freq_t ch_freq,
					 enum policy_mgr_con_mode mode)
{
	bool allow = true;

	if (!policy_mgr_is_hw_dbs_capable(psoc) &&
	    policy_mgr_is_interband_mcc_supported(psoc)) {
		policy_mgr_rl_debug("allow 2 intf SCC + new intf ch %d for legacy hw",
				    ch_freq);
	} else if ((pm_conc_connection_list[0].mode == PM_NAN_DISC_MODE &&
		    pm_conc_connection_list[1].mode == PM_NDI_MODE) ||
		    (pm_conc_connection_list[0].mode == PM_NDI_MODE &&
		    pm_conc_connection_list[1].mode == PM_NAN_DISC_MODE)) {
		/*
		 * NAN + NDI are managed in Firmware by dividing
		 * up slots. Connection on NDI is re-negotiable
		 * and therefore a 3rd connection with the
		 * same MAC is possible.
		 */
	} else if (policy_mgr_are_2_freq_on_same_mac(psoc, ch_freq,
					pm_conc_connection_list[0].freq) &&
		   !policy_mgr_is_3rd_conn_on_same_band_allowed(psoc, mode)) {
			policy_mgr_rl_debug("don't allow 3rd home channel on same MAC – for sta+multi-AP");
			allow = false;
	}

	return allow;
}

/**
 * policy_mgr_allow_new_home_channel() - Check for allowed number of
 * home channels
 * @mode: policy_mgr_con_mode of new connection,
 * @channel: channel on which new connection is coming up
 * @num_connections: number of current connections
 * @is_dfs_ch: DFS channel or not
 *
 * When a new connection is about to come up check if current
 * concurrency combination including the new connection is
 * allowed or not based on the HW capability
 *
 * Return: True/False
 */
bool policy_mgr_allow_new_home_channel(
	struct wlan_objmgr_psoc *psoc, enum policy_mgr_con_mode mode,
	uint32_t ch_freq, uint32_t num_connections, bool is_dfs_ch)
{
	bool status = true;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	bool on_same_mac = false, force_switch_without_dis = false;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	force_switch_without_dis =
		policy_mgr_get_mcc_to_scc_switch_mode(psoc) ==
		QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION;

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	if (num_connections == 2) {
	/* No SCC or MCC combination is allowed with / on DFS channel */
		on_same_mac = policy_mgr_are_2_freq_on_same_mac(psoc,
				pm_conc_connection_list[0].freq,
				pm_conc_connection_list[1].freq);
		if (force_switch_without_dis && is_dfs_ch &&
		    ((pm_conc_connection_list[0].ch_flagext &
		      (IEEE80211_CHAN_DFS | IEEE80211_CHAN_DFS_CFREQ2)) ||
		     (pm_conc_connection_list[1].ch_flagext &
		      (IEEE80211_CHAN_DFS | IEEE80211_CHAN_DFS_CFREQ2)))) {
			policy_mgr_rl_debug("Existing DFS connection, new 3-port DFS connection is not allowed");
			status = false;
		} else if (((pm_conc_connection_list[0].freq !=
			     pm_conc_connection_list[1].freq) ||
			    force_switch_without_dis) && on_same_mac) {
			status = policy_mgr_allow_same_mac_diff_freq(psoc,
								     ch_freq);
		} else if (on_same_mac) {
			status = policy_mgr_allow_same_mac_same_freq(psoc,
								     ch_freq,
								     mode);
		}
	} else if (num_connections == 1 && force_switch_without_dis &&
		   is_dfs_ch &&
		   (pm_conc_connection_list[0].ch_flagext &
		    (IEEE80211_CHAN_DFS | IEEE80211_CHAN_DFS_CFREQ2))) {
		policy_mgr_rl_debug("Existing DFS connection, new 2-port DFS connection is not allowed");
		status = false;
	} else if ((num_connections == 1) &&
		   !policy_mgr_is_hw_dbs_capable(psoc) &&
		   !policy_mgr_is_interband_mcc_supported(psoc)) {
		/* For target which is single mac and doesn't support
		 * interband MCC
		 */
		if ((pm_conc_connection_list[0].mode != PM_NAN_DISC_MODE) &&
		    (mode != PM_NAN_DISC_MODE))
			status = policy_mgr_are_2_freq_on_same_mac(psoc,
								   ch_freq,
					pm_conc_connection_list[0].freq);
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return status;
}

bool policy_mgr_is_5g_channel_allowed(struct wlan_objmgr_psoc *psoc,
				uint32_t ch_freq, uint32_t *list,
				enum policy_mgr_con_mode mode)
{
	uint32_t index = 0, count = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	count = policy_mgr_mode_specific_connection_count(psoc, mode, list);
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	while (index < count) {
		if ((pm_conc_connection_list[list[index]].ch_flagext &
		     (IEEE80211_CHAN_DFS | IEEE80211_CHAN_DFS_CFREQ2)) &&
		    WLAN_REG_IS_5GHZ_CH_FREQ(ch_freq) &&
		    (ch_freq != pm_conc_connection_list[list[index]].freq &&
		     !policy_mgr_are_sbs_chan(psoc, ch_freq,
				pm_conc_connection_list[list[index]].freq))) {
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
			policy_mgr_rl_debug("don't allow MCC if SAP/GO on DFS channel");
			return false;
		}
		index++;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return true;
}

static qdf_freq_t
policy_mgr_get_iface_5g_freq(struct wlan_objmgr_psoc *psoc,
			     bool allow_6ghz)
{
	qdf_freq_t if_freq = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t conn_index;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return 0;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
		conn_index++) {
		if (pm_conc_connection_list[conn_index].in_use &&
		    (WLAN_REG_IS_5GHZ_CH_FREQ(
				pm_conc_connection_list[conn_index].freq) ||
		     (allow_6ghz && WLAN_REG_IS_6GHZ_CHAN_FREQ(
				pm_conc_connection_list[conn_index].freq)))) {
			if_freq = pm_conc_connection_list[conn_index].freq;
			break;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return if_freq;
}

static qdf_freq_t
policy_mgr_get_iface_2g_freq(struct wlan_objmgr_psoc *psoc)
{
	qdf_freq_t if_freq = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t conn_index;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return 0;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
		conn_index++) {
		if (pm_conc_connection_list[conn_index].in_use &&
		    (WLAN_REG_IS_24GHZ_CH_FREQ(
				pm_conc_connection_list[conn_index].freq))) {
			if_freq = pm_conc_connection_list[conn_index].freq;
			break;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return if_freq;
}

static qdf_freq_t
policy_mgr_get_same_band_iface_frq(struct wlan_objmgr_psoc *psoc,
				   qdf_freq_t ch_freq, bool allow_6ghz)
{
	return (WLAN_REG_IS_24GHZ_CH_FREQ(ch_freq) ?
		policy_mgr_get_iface_2g_freq(psoc) :
		policy_mgr_get_iface_5g_freq(psoc, allow_6ghz));
}

static void
policy_mgr_check_force_scc_two_connection(struct wlan_objmgr_psoc *psoc,
					  qdf_freq_t *intf_ch_freq,
					  qdf_freq_t sap_ch_freq,
					  uint8_t cc_mode,
					  bool same_band_present,
					  uint32_t acs_band,
					  bool allow_6ghz)
{
	bool sbs_mlo_present = false;

	/*
	 * 1st:    2nd:           3rd(SAP):      Action:
	 * -------------------------------------------------
	 * 2Ghz    2Ghz(SMM/DBS)  5Ghz           Start on 5Ghz
	 * 2Ghz    5Ghz(DBS)      5Ghz           Force SCC to 5Ghz
	 * 2Ghz    5Ghz(DBS)      2Ghz           Force SCC to 5Ghz
	 * 5Ghz    5Ghz(SBS)      2Ghz           If acs is ALL band force SCC
	 *                                       else start on 2.4Ghz
	 * 5Ghz    5Ghz(SBS)      5Ghz           Force SCC to one of 5Ghz
	 * 5Ghz    5Ghz(SMM)      2Ghz           Start on 2.4Ghz
	 * 5Ghz    5Ghz(SMM)      5Ghz           Allow SAP on sap_ch_freq if all
	 *                                       3, 5Ghz freq does't end up
	 *                                       on same mac, ie 2 of them lead
	 *                                       to SBS. Else force SCC on
	 *                                       one of the freq (3 home channel
	 *                                       will not be allowed)
	 * 2Ghz    2Ghz(SMM/DBS)  2Ghz           force SCC on one of the freq
	 *                                       (3 home channel
	 *                                       will not be allowed)
	 */

	/* Check if STA or SAP SBS MLO is present */
	if (policy_mgr_is_hw_sbs_capable(psoc) &&
	    (policy_mgr_is_mlo_in_mode_sbs(psoc, PM_STA_MODE,
					   NULL, NULL) ||
	     policy_mgr_is_mlo_in_mode_sbs(psoc, PM_SAP_MODE,
					   NULL, NULL)))
		sbs_mlo_present = true;

	/*
	 * Check for SBS mlo present as if 1 link is inactive the
	 * HW mode will be SMM and not SBS.
	 */
	if (policy_mgr_is_current_hwmode_sbs(psoc) || sbs_mlo_present) {
		/*
		 * 1st:    2nd:        3rd(SAP):   Action:
		 * -------------------------------------------------
		 * 5Ghz    5Ghz(SBS)   2Ghz        If acs is ALL band force SCC
		 *                                 else start on 2.4Ghz
		 * 5Ghz    5Ghz(SBS)   5Ghz        Force SCC to one of 5Ghz
		 */
		if (acs_band == QCA_ACS_MODE_IEEE80211ANY ||
		    !WLAN_REG_IS_24GHZ_CH_FREQ(sap_ch_freq))
			*intf_ch_freq =
				policy_mgr_get_iface_5g_freq(psoc, allow_6ghz);
		else
			*intf_ch_freq =
				policy_mgr_get_same_band_iface_frq(psoc,
								   sap_ch_freq,
								   allow_6ghz);
		return;
	}
	if (policy_mgr_is_current_hwmode_dbs(psoc)) {
		/*
		 * 1st:    2nd:           3rd(SAP):      Action:
		 * -------------------------------------------------
		 * 2Ghz    2Ghz(DBS)      5Ghz           Start on 5Ghz
		 * 2Ghz    5Ghz(DBS)      5Ghz           Force SCC to 5Ghz
		 * 2Ghz    5Ghz(DBS)      2Ghz           Force SCC to 5Ghz
		 * 2Ghz    2Ghz(DBS)      2Ghz           force SCC on one of
		 *                                       the freq (3 home
		 *                                       channel will not be
		 *                                       allowed)
		 */
		/*
		 * For DBS allow the 6Ghz as we may only have 1 5Ghz freq.
		 * policy_mgr_valid_sap_conc_channel_check->
		 * policy_mgr_check_6ghz_sap_conc will take care of switching to
		 * other channel if 6Ghz is not allowed
		 */
		*intf_ch_freq =
			policy_mgr_get_same_band_iface_frq(psoc, sap_ch_freq,
							   true);
		return;
	}

	if (!same_band_present) {
		/*
		 * 1st:    2nd:           3rd(SAP):      Action:
		 * -------------------------------------------------
		 * 2Ghz    2Ghz(SMM)      5Ghz           Start on 5Ghz(DBS)
		 * 5Ghz    5Ghz(SMM)      2Ghz           Start on 2.4Ghz(DBS)
		 */
		if (policy_mgr_is_hw_dbs_capable(psoc))
			*intf_ch_freq = 0;
		return;
	}

	if (!policy_mgr_are_3_freq_on_same_mac(psoc, sap_ch_freq,
					pm_conc_connection_list[0].freq,
					pm_conc_connection_list[1].freq)) {
		/*
		 * 1st:    2nd:           3rd(SAP):     Action:
		 * -------------------------------------------------
		 * 5Ghz    5Ghz(SMM)      5Ghz          Allow SAP on sap_ch_freq
		 *                                      if all 3, 5Ghz freq
		 *                                      does't end up, on same
		 *                                      mac, ie 2 of them lead
		 *                                      to SBS, ie at least one
		 *                                      of them is high 5Ghz and
		 *                                      one low 5Ghz.
		 */
		*intf_ch_freq = 0;
		return;
	}

	/*
	 * 1st:    2nd:           3rd(SAP):     Action:
	 * -------------------------------------------------
	 * 5Ghz    5Ghz(SMM)      5Ghz          force SCC on one of the freq
	 *                                      (3 home channel will not be
	 *                                      allowed)
	 * 2Ghz    2Ghz(SMM)      2Ghz          force SCC on one of the freq
	 *                                      (3 home channel will not be
	 *                                      allowed)
	 */
	policy_mgr_debug("%d Can lead to 3 home channel on same MAC",
			 sap_ch_freq);
}

static void
policy_mgr_check_force_scc_one_connection(struct wlan_objmgr_psoc *psoc,
					  qdf_freq_t *intf_ch_freq,
					  qdf_freq_t sap_ch_freq,
					  bool same_band_present,
					  uint8_t cc_mode)
{
	/*
	 * 1st:    2nd(SAP):      Action:
	 * ------------------------------------
	 * 2Ghz     2Ghz          Force SCC on 2Ghz
	 * 5Ghz     5Ghz          Force SCC on 5Ghz for non SBS, for SBS freq
	 *                        allow sap freq
	 * 2Ghz     5Ghz          Start on 5Ghz (DBS)
	 * 5Ghz     2Ghz          Start on 2.4Ghz(DBS)
	 */

	if (same_band_present) {
		/*
		 * 1st:    2nd(SAP):      Action:
		 * ------------------------------------
		 * 2Ghz     2Ghz          Force SCC on 2Ghz
		 * 5Ghz     5Ghz          Force SCC on 5Ghz for non SBS,
		 *                        for SBS freq allow sap freq
		 */
		if (policy_mgr_are_sbs_chan(psoc, sap_ch_freq, *intf_ch_freq)) {
			policy_mgr_debug("Do not overwrite as sap_ch_freq %d intf_ch_freq %d are SBS freq",
					 sap_ch_freq, *intf_ch_freq);
			*intf_ch_freq = 0;
		}
		return;
	}
	if (policy_mgr_is_hw_dbs_capable(psoc) ||
	    cc_mode ==  QDF_MCC_TO_SCC_WITH_PREFERRED_BAND) {
		/*
		 * 1st:    2nd(SAP):      Action:
		 * ------------------------------------
		 * 2Ghz     5Ghz          Start on 5Ghz (DBS)
		 * 5Ghz     2Ghz          Start on 2.4Ghz(DBS)
		 */
		/* Different bands can do DBS so dont overwrite */
		*intf_ch_freq = 0;
	}
}

void policy_mgr_check_scc_sbs_channel(struct wlan_objmgr_psoc *psoc,
				      qdf_freq_t *intf_ch_freq,
				      qdf_freq_t sap_ch_freq,
				      uint8_t vdev_id, uint8_t cc_mode)
{
	uint32_t num_connections, acs_band = QCA_ACS_MODE_IEEE80211ANY;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	QDF_STATUS status;
	struct policy_mgr_conc_connection_info
			info[MAX_NUMBER_OF_CONC_CONNECTIONS] = { {0} };
	uint8_t num_cxn_del = 0;
	bool same_band_present = false;
	bool sbs_mlo_present = false;
	bool allow_6ghz = true;
	uint8_t sta_count;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	/* Always do force SCC on non-DBS platforms */
	if (!policy_mgr_is_hw_dbs_capable(psoc) &&
	    cc_mode !=  QDF_MCC_TO_SCC_WITH_PREFERRED_BAND)
		return;

	sta_count = policy_mgr_mode_specific_connection_count(psoc, PM_STA_MODE,
							      NULL);
	/*
	 * If same band interface is present, as
	 * csr_check_concurrent_channel_overlap try to find same band vdev
	 * if available
	 */
	if ((WLAN_REG_IS_24GHZ_CH_FREQ(sap_ch_freq) &&
	     WLAN_REG_IS_24GHZ_CH_FREQ(*intf_ch_freq)) ||
	    (!WLAN_REG_IS_24GHZ_CH_FREQ(*intf_ch_freq) &&
	     !WLAN_REG_IS_24GHZ_CH_FREQ(sap_ch_freq)))
		same_band_present = true;

	/* Check if STA or SAP SBS MLO is present */
	if (policy_mgr_is_hw_sbs_capable(psoc) &&
	    (policy_mgr_is_mlo_in_mode_sbs(psoc, PM_STA_MODE,
					   NULL, NULL) ||
	     policy_mgr_is_mlo_in_mode_sbs(psoc, PM_SAP_MODE,
					   NULL, NULL)))
		sbs_mlo_present = true;

	/*
	 * Different band, this also mean that there is no other interface on
	 * on same band as csr_check_concurrent_channel_overlap
	 * try to find same band vdev if available.
	 * this mean for DBS HW we can use the other available band and thus
	 * set *intf_ch_freq = 0, to bring sap on sap_ch_freq.
	 */
	if (!same_band_present) {
		if (policy_mgr_is_current_hwmode_sbs(psoc) || sbs_mlo_present)
			goto sbs_check;
		/*
		 * #1 port:
		 * 1st:    2nd(SAP):      Action:
		 * ------------------------------------
		 * 2Ghz     5Ghz          Start on 5Ghz(DBS)
		 * 5Ghz     2Ghz          Start on 2.4Ghz(DBS)
		 *
		 * #2 port:
		 * 1st:    2nd:           3rd(SAP):      Action:
		 * -------------------------------------------------
		 * 2Ghz    2Ghz(SMM/DBS)  5Ghz           Start on 5Ghz(DBS)
		 * 5Ghz    5Ghz(SMM)      2Ghz           Start on 2.4Ghz(DBS)
		 *
		 * #3 port:
		 * 1st:    2nd:    3rd:    4th(SAP)      Action:
		 * -------------------------------------------------
		 * 2Ghz    2Ghz    2Ghz   5Ghz           Start on 5Ghz(DBS)
		 * 5Ghz    5Ghz    5Ghz   2Ghz           Start on 2.4Ghz(DBS)
		 */
		if (policy_mgr_is_hw_dbs_capable(psoc) ||
		    cc_mode ==  QDF_MCC_TO_SCC_WITH_PREFERRED_BAND) {
			*intf_ch_freq = 0;
			return;
		}
	} else if (sta_count &&
		   policy_mgr_is_hw_dbs_capable(psoc) &&
		   cc_mode == QDF_MCC_TO_SCC_SWITCH_WITH_FAVORITE_CHANNEL) {
		/* Same band with Fav channel if STA is present */
		status = policy_mgr_get_sap_mandatory_channel(psoc,
							      sap_ch_freq,
							      intf_ch_freq);

		if (QDF_IS_STATUS_SUCCESS(status))
			return;

		policy_mgr_debug("no mandatory channels (%d, %d)", sap_ch_freq,
				 *intf_ch_freq);
	}

sbs_check:

	/* Get allow 6Gz before interface entry is temporary deleted */
	if (sap_ch_freq && !WLAN_REG_IS_6GHZ_CHAN_FREQ(sap_ch_freq) &&
	    !policy_mgr_get_ap_6ghz_capable(psoc, vdev_id, NULL))
		allow_6ghz = false;

	if (pm_ctx->hdd_cbacks.wlan_get_sap_acs_band) {
		status = pm_ctx->hdd_cbacks.wlan_get_sap_acs_band(psoc,
								  vdev_id,
								  &acs_band);
		if (QDF_IS_STATUS_SUCCESS(status))
			policy_mgr_debug("acs_band: %d", acs_band);
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	/*
	 * For SAP restart case SAP entry might be present in table,
	 * so delete it temporary
	 */
	policy_mgr_store_and_del_conn_info_by_vdev_id(psoc, vdev_id, info,
						      &num_cxn_del);

	/*
	 * If at least one interface is in same band OR HW mode is SBS OR
	 * SBS MLO is present, try set SBS/DBS/SCC.
	 */
	num_connections = policy_mgr_get_connection_count(psoc);
	policy_mgr_dump_sbs_freq_range(pm_ctx);
	switch (num_connections) {
	case 0:
		/* use sap channel */
		*intf_ch_freq = 0;
		break;
	case 1:
		policy_mgr_check_force_scc_one_connection(psoc, intf_ch_freq,
							  sap_ch_freq,
							  same_band_present,
							  cc_mode);
		break;
	case 2:
		policy_mgr_check_force_scc_two_connection(psoc, intf_ch_freq,
							  sap_ch_freq,
							  cc_mode,
							  same_band_present,
							  acs_band,
							  allow_6ghz);
		break;
	default:
		policy_mgr_debug("invalid num_connections: %d",
				 num_connections);
		break;
	}
	/* Restore the connection entry */
	if (num_cxn_del > 0)
		policy_mgr_restore_deleted_conn_info(psoc, info, num_cxn_del);

	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
}

/**
 * policy_mgr_nss_update_cb() - callback from SME confirming nss
 * update
 * @hdd_ctx:	HDD Context
 * @tx_status: tx completion status for updated beacon with new
 *		nss value
 * @vdev_id: vdev id for the specific connection
 * @next_action: next action to happen at policy mgr after
 *		beacon update
 * @reason: Reason for nss update
 * @original_vdev_id: original request hwmode change vdev id
 *
 * This function is the callback registered with SME at nss
 * update request time
 *
 * Return: None
 */
static void policy_mgr_nss_update_cb(struct wlan_objmgr_psoc *psoc,
		uint8_t tx_status,
		uint8_t vdev_id,
		uint8_t next_action,
		enum policy_mgr_conn_update_reason reason,
		uint32_t original_vdev_id, uint32_t request_id)
{
	uint32_t conn_index = 0;
	QDF_STATUS ret;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	if (QDF_STATUS_SUCCESS != tx_status)
		policy_mgr_err("nss update failed(%d) for vdev %d",
			tx_status, vdev_id);

	/*
	 * Check if we are ok to request for HW mode change now
	 */
	conn_index = policy_mgr_get_connection_for_vdev_id(psoc, vdev_id);
	if (MAX_NUMBER_OF_CONC_CONNECTIONS == conn_index) {
		policy_mgr_err("connection not found for vdev %d", vdev_id);
		return;
	}

	policy_mgr_debug("nss update successful for vdev:%d ori %d reason %d",
			 vdev_id, original_vdev_id, reason);
	if (PM_NOP != next_action) {
		if (reason == POLICY_MGR_UPDATE_REASON_AFTER_CHANNEL_SWITCH)
			policy_mgr_next_actions(psoc, vdev_id, next_action,
						reason, request_id);
		else
			policy_mgr_next_actions(psoc, original_vdev_id,
						next_action, reason,
						request_id);
	} else {
		if (reason == POLICY_MGR_UPDATE_REASON_STA_CONNECT ||
		    reason == POLICY_MGR_UPDATE_REASON_LFR2_ROAM) {
			sme_debug("Continue connect/reassoc on vdev %d request_id %x reason %d",
				  vdev_id, request_id, reason);
			wlan_cm_hw_mode_change_resp(pm_ctx->pdev, vdev_id,
						    request_id,
						    QDF_STATUS_SUCCESS);
		}
		policy_mgr_debug("No action needed right now");
		ret = policy_mgr_set_opportunistic_update(psoc);
		if (!QDF_IS_STATUS_SUCCESS(ret))
			policy_mgr_err("ERROR: set opportunistic_update event failed");
	}

	return;
}

QDF_STATUS policy_mgr_nss_update(struct wlan_objmgr_psoc *psoc,
		uint8_t  new_nss, uint8_t next_action,
		enum policy_mgr_band band,
		enum policy_mgr_conn_update_reason reason,
		uint32_t original_vdev_id, uint32_t request_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t index, count;
	uint32_t list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	uint32_t conn_index = 0;
	uint32_t vdev_id;
	uint32_t original_nss, ch_freq;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	enum phy_ch_width ch_width = CH_WIDTH_MAX;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}
	if (next_action == PM_DBS2 && band == POLICY_MGR_BAND_5)
		ch_width = CH_WIDTH_40MHZ;

	count = policy_mgr_mode_specific_connection_count(psoc,
			PM_P2P_GO_MODE, list);
	for (index = 0; index < count; index++) {
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
		vdev_id = pm_conc_connection_list[list[index]].vdev_id;
		original_nss =
		pm_conc_connection_list[list[index]].original_nss;
		ch_freq = pm_conc_connection_list[list[index]].freq;
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		conn_index = policy_mgr_get_connection_for_vdev_id(
			psoc, vdev_id);
		if (MAX_NUMBER_OF_CONC_CONNECTIONS == conn_index) {
			policy_mgr_err("connection not found for vdev %d",
				vdev_id);
			continue;
		}

		if (original_nss == 2 &&
		    (band == POLICY_MGR_ANY ||
		    (band == POLICY_MGR_BAND_24 &&
		    WLAN_REG_IS_24GHZ_CH_FREQ(ch_freq)) ||
		    (band == POLICY_MGR_BAND_5 &&
		    WLAN_REG_IS_5GHZ_CH_FREQ(ch_freq)))) {
			status = pm_ctx->sme_cbacks.sme_nss_update_request(
					vdev_id, new_nss, ch_width,
					policy_mgr_nss_update_cb,
					next_action, psoc, reason,
					original_vdev_id, request_id);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				policy_mgr_err("sme_nss_update_request() failed for vdev %d",
				vdev_id);
			}
		}
	}

	count = policy_mgr_mode_specific_connection_count(psoc,
			PM_SAP_MODE, list);
	for (index = 0; index < count; index++) {
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
		vdev_id = pm_conc_connection_list[list[index]].vdev_id;
		original_nss =
		pm_conc_connection_list[list[index]].original_nss;
		ch_freq = pm_conc_connection_list[list[index]].freq;
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		conn_index = policy_mgr_get_connection_for_vdev_id(
			psoc, vdev_id);
		if (MAX_NUMBER_OF_CONC_CONNECTIONS == conn_index) {
			policy_mgr_err("connection not found for vdev %d",
				vdev_id);
			continue;
		}
		if (original_nss == 2 &&
		    (band == POLICY_MGR_ANY ||
		    (band == POLICY_MGR_BAND_24 &&
		    WLAN_REG_IS_24GHZ_CH_FREQ(ch_freq)) ||
		    (band == POLICY_MGR_BAND_5 &&
		    WLAN_REG_IS_5GHZ_CH_FREQ(ch_freq)))) {
			status = pm_ctx->sme_cbacks.sme_nss_update_request(
					vdev_id, new_nss, ch_width,
					policy_mgr_nss_update_cb,
					next_action, psoc, reason,
					original_vdev_id, request_id);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				policy_mgr_err("sme_nss_update_request() failed for vdev %d",
				vdev_id);
			}
		}
	}

	return status;
}

QDF_STATUS policy_mgr_complete_action(struct wlan_objmgr_psoc *psoc,
				uint8_t  new_nss, uint8_t next_action,
				enum policy_mgr_conn_update_reason reason,
				uint32_t session_id, uint32_t request_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	enum policy_mgr_band downgrade_band;

	if (policy_mgr_is_hw_dbs_capable(psoc) == false) {
		policy_mgr_rl_debug("driver isn't dbs capable, no further action needed");
		return QDF_STATUS_E_NOSUPPORT;
	}

	/* policy_mgr_complete_action() is called by policy_mgr_next_actions().
	 * All other callers of policy_mgr_next_actions() have taken mutex
	 * protection. So, not taking any lock inside
	 * policy_mgr_complete_action() during pm_conc_connection_list access.
	 */
	if (next_action == PM_DBS1)
		downgrade_band = POLICY_MGR_BAND_24;
	else if (next_action == PM_DBS2)
		downgrade_band = POLICY_MGR_BAND_5;
	else
		downgrade_band = POLICY_MGR_ANY;

	status = policy_mgr_nss_update(psoc, new_nss, next_action,
				       downgrade_band, reason,
				       session_id, request_id);
	if (!QDF_IS_STATUS_SUCCESS(status))
		status = policy_mgr_next_actions(psoc, session_id,
						 next_action, reason,
						 request_id);

	return status;
}

enum policy_mgr_con_mode policy_mgr_get_mode_by_vdev_id(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id)
{
	enum policy_mgr_con_mode mode = PM_MAX_NUM_OF_MODE;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t conn_index;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return mode;
	}
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	for (conn_index = 0; conn_index < MAX_NUMBER_OF_CONC_CONNECTIONS;
		conn_index++)
		if ((pm_conc_connection_list[conn_index].vdev_id == vdev_id) &&
			pm_conc_connection_list[conn_index].in_use){
				mode = pm_conc_connection_list[conn_index].mode;
				break;
		}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return mode;
}

/**
 * policy_mgr_init_connection_update() - Initialize connection
 * update event
 * @pm_ctx: policy mgr context
 *
 * Initializes the concurrent connection update event
 *
 * Return: QDF_STATUS
 */
QDF_STATUS policy_mgr_init_connection_update(
		struct policy_mgr_psoc_priv_obj *pm_ctx)
{
	QDF_STATUS qdf_status;

	qdf_status = qdf_event_create(&pm_ctx->connection_update_done_evt);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		policy_mgr_err("init event failed");
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * policy_mgr_get_current_pref_hw_mode_dbs_2x2() - Get the
 * current preferred hw mode
 *
 * Get the preferred hw mode based on the current connection combinations
 *
 * Return: No change (PM_NOP), MCC (PM_SINGLE_MAC),
 *         DBS (PM_DBS), SBS (PM_SBS)
 */
enum policy_mgr_conc_next_action
		policy_mgr_get_current_pref_hw_mode_dbs_2x2(
		struct wlan_objmgr_psoc *psoc)
{
	uint32_t num_connections;
	uint8_t band1, band2, band3;
	struct policy_mgr_hw_mode_params hw_mode;
	QDF_STATUS status;

	status = policy_mgr_get_current_hw_mode(psoc, &hw_mode);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		policy_mgr_err("policy_mgr_get_current_hw_mode failed");
		return PM_NOP;
	}

	num_connections = policy_mgr_get_connection_count(psoc);

	policy_mgr_debug("chan[0]:%d chan[1]:%d chan[2]:%d num_connections:%d dbs:%d",
		pm_conc_connection_list[0].freq,
		pm_conc_connection_list[1].freq,
		pm_conc_connection_list[2].freq, num_connections,
		hw_mode.dbs_cap);

	/* If the band of operation of both the MACs is the same,
	 * single MAC is preferred, otherwise DBS is preferred.
	 */
	switch (num_connections) {
	case 1:
		band1 = wlan_reg_freq_to_band(pm_conc_connection_list[0].freq);
		if (band1 == REG_BAND_2G)
			return PM_DBS;
		else
			return PM_NOP;
	case 2:
		band1 = wlan_reg_freq_to_band(pm_conc_connection_list[0].freq);
		band2 = wlan_reg_freq_to_band(pm_conc_connection_list[1].freq);
		if (band1 == REG_BAND_2G || band2 == REG_BAND_2G) {
			if (!hw_mode.dbs_cap)
				return PM_DBS;
			else
				return PM_NOP;
		} else if (band1 == REG_BAND_5G && band2 == REG_BAND_5G) {
			if (policy_mgr_are_sbs_chan(psoc,
					pm_conc_connection_list[0].freq,
					pm_conc_connection_list[1].freq)) {
				if (!hw_mode.sbs_cap)
					return PM_SBS;
				else
					return PM_NOP;
			} else {
				if (hw_mode.sbs_cap || hw_mode.dbs_cap)
					return PM_SINGLE_MAC;
				else
					return PM_NOP;
			}
		} else
			return PM_NOP;
	case 3:
		band1 = wlan_reg_freq_to_band(pm_conc_connection_list[0].freq);
		band2 = wlan_reg_freq_to_band(pm_conc_connection_list[1].freq);
		band3 = wlan_reg_freq_to_band(pm_conc_connection_list[2].freq);
		if (band1 == REG_BAND_2G || band2 == REG_BAND_2G ||
		    band3 == REG_BAND_2G) {
			if (!hw_mode.dbs_cap)
				return PM_DBS;
			else
				return PM_NOP;
		} else if (band1 == REG_BAND_5G && band2 == REG_BAND_5G &&
			   band3 == REG_BAND_5G) {
			if (policy_mgr_are_sbs_chan(psoc,
					pm_conc_connection_list[0].freq,
					pm_conc_connection_list[2].freq) &&
			    policy_mgr_are_sbs_chan(psoc,
					pm_conc_connection_list[1].freq,
					pm_conc_connection_list[2].freq) &&
			    policy_mgr_are_sbs_chan(psoc,
					pm_conc_connection_list[0].freq,
					pm_conc_connection_list[1].freq)) {
				if (!hw_mode.sbs_cap)
					return PM_SBS;
				else
					return PM_NOP;
			} else {
				if (hw_mode.sbs_cap || hw_mode.dbs_cap)
					return PM_SINGLE_MAC;
				else
					return PM_NOP;
			}
		} else
			return PM_NOP;
	default:
		policy_mgr_err("unexpected num_connections value %d",
				num_connections);
		return PM_NOP;
	}
}

/**
 * policy_mgr_get_current_pref_hw_mode_dbs_1x1() - Get the
 * current preferred hw mode
 *
 * Get the preferred hw mode based on the current connection combinations
 *
 * Return: No change (PM_NOP), MCC (PM_SINGLE_MAC_UPGRADE),
 *         DBS (PM_DBS_DOWNGRADE)
 */
enum policy_mgr_conc_next_action
		policy_mgr_get_current_pref_hw_mode_dbs_1x1(
		struct wlan_objmgr_psoc *psoc)
{
	uint32_t num_connections;
	uint8_t band1, band2, band3;
	struct policy_mgr_hw_mode_params hw_mode;
	QDF_STATUS status;
	enum policy_mgr_conc_next_action next_action;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return PM_NOP;
	}

	status = policy_mgr_get_current_hw_mode(psoc, &hw_mode);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		policy_mgr_err("policy_mgr_get_current_hw_mode failed");
		return PM_NOP;
	}

	num_connections = policy_mgr_get_connection_count(psoc);

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	policy_mgr_debug("chan[0]:%d chan[1]:%d chan[2]:%d num_connections:%d dbs:%d",
		pm_conc_connection_list[0].freq,
		pm_conc_connection_list[1].freq,
		pm_conc_connection_list[2].freq, num_connections,
		hw_mode.dbs_cap);

	/* If the band of operation of both the MACs is the same,
	 * single MAC is preferred, otherwise DBS is preferred.
	 */
	switch (num_connections) {
	case 1:
		/* The driver would already be in the required hw mode */
		next_action = PM_NOP;
		break;
	case 2:
		band1 = wlan_reg_freq_to_band(pm_conc_connection_list[0].freq);
		band2 = wlan_reg_freq_to_band(pm_conc_connection_list[1].freq);
		if ((band1 == band2) && (hw_mode.dbs_cap))
			next_action = PM_SINGLE_MAC_UPGRADE;
		else if ((band1 != band2) && (!hw_mode.dbs_cap))
			next_action = PM_DBS_DOWNGRADE;
		else
			next_action = PM_NOP;

		break;

	case 3:
		band1 = wlan_reg_freq_to_band(pm_conc_connection_list[0].freq);
		band2 = wlan_reg_freq_to_band(pm_conc_connection_list[1].freq);
		band3 = wlan_reg_freq_to_band(pm_conc_connection_list[2].freq);
		if (((band1 == band2) && (band2 == band3)) &&
				(hw_mode.dbs_cap)) {
			next_action = PM_SINGLE_MAC_UPGRADE;
		} else if (((band1 != band2) || (band2 != band3) ||
					(band1 != band3)) &&
					(!hw_mode.dbs_cap)) {
			next_action = PM_DBS_DOWNGRADE;
		} else {
			next_action = PM_NOP;
		}
		break;
	default:
		policy_mgr_err("unexpected num_connections value %d",
				num_connections);
		next_action = PM_NOP;
		break;
	}

	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return next_action;
}

enum policy_mgr_conc_next_action
policy_mgr_get_current_pref_hw_mode_dual_dbs(
	struct wlan_objmgr_psoc *psoc)
{
	enum policy_mgr_conc_next_action next_action;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	enum policy_mgr_conc_next_action preferred_dbs;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return PM_NOP;
	}

	next_action = policy_mgr_get_current_pref_hw_mode_dbs_1x1(psoc);
	policy_mgr_info("next_action %d", next_action);
	if (next_action != PM_DBS_DOWNGRADE)
		return next_action;

	preferred_dbs = policy_mgr_get_preferred_dbs_action_table(
				psoc, INVALID_VDEV_ID, 0, 0);
	if (preferred_dbs == PM_DBS1) {
		next_action = PM_DBS1_DOWNGRADE;
	} else if (preferred_dbs == PM_DBS2) {
		next_action = PM_DBS2_DOWNGRADE;
	} else {
		policy_mgr_err("DBS1 and DBS2 hw mode not supported");
		return PM_NOP;
	}
	policy_mgr_info("preferred_dbs %d", next_action);
	return next_action;
}

/**
 * policy_mgr_reset_sap_mandatory_channels() - Reset the SAP mandatory channels
 *
 * Resets the SAP mandatory channel list and the length of the list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS policy_mgr_reset_sap_mandatory_channels(
		struct policy_mgr_psoc_priv_obj *pm_ctx)
{
	pm_ctx->sap_mandatory_channels_len = 0;
	qdf_mem_zero(pm_ctx->sap_mandatory_channels,
		     QDF_ARRAY_SIZE(pm_ctx->sap_mandatory_channels) *
		     sizeof(*pm_ctx->sap_mandatory_channels));

	return QDF_STATUS_SUCCESS;
}

void policy_mgr_add_sap_mandatory_chan(struct wlan_objmgr_psoc *psoc,
				       uint32_t ch_freq)
{
	int i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	for (i = 0; i < pm_ctx->sap_mandatory_channels_len; i++) {
		if (ch_freq == pm_ctx->sap_mandatory_channels[i])
			return;
	}
	if (pm_ctx->sap_mandatory_channels_len >= NUM_CHANNELS) {
		policy_mgr_err("mand list overflow (%u)", ch_freq);
		return;
	}
	policy_mgr_debug("Ch freq: %u", ch_freq);
	pm_ctx->sap_mandatory_channels[pm_ctx->sap_mandatory_channels_len++]
		= ch_freq;
}

uint32_t policy_mgr_get_sap_mandatory_chan_list_len(
		struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return 0;
	}

	return pm_ctx->sap_mandatory_channels_len;
}

#if defined(CONFIG_BAND_6GHZ)
/**
 * policy_mgr_add_sap_mandatory_6ghz_chan() - Add 6GHz SAP mandatory channel
 * list
 * @psoc: Pointer to soc
 *
 * Add the 6GHz PSC VLP channel to SAP mandatory channel list.
 *
 * Return: None
 */
static
void  policy_mgr_add_sap_mandatory_6ghz_chan(struct wlan_objmgr_psoc *psoc)
{
	uint32_t ch_freq_list[NUM_CHANNELS] = {0};
	uint32_t len = 0;
	int i;
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	bool is_psd;
	uint16_t tx_power;
	uint16_t eirp_psd_power;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	status = policy_mgr_get_valid_chans(psoc, ch_freq_list, &len);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("Error in getting valid channels");
		return;
	}

	for (i = 0; (i < len) && (i < NUM_CHANNELS) &&
		    (pm_ctx->sap_mandatory_channels_len < NUM_CHANNELS); i++) {
		if (!WLAN_REG_IS_6GHZ_CHAN_FREQ(ch_freq_list[i]))
			continue;
		if (WLAN_REG_IS_6GHZ_PSC_CHAN_FREQ(ch_freq_list[i])) {
			status = wlan_reg_get_6g_chan_ap_power(
				pm_ctx->pdev, ch_freq_list[i], &is_psd,
				&tx_power, &eirp_psd_power);
			if (status != QDF_STATUS_SUCCESS || !tx_power)
				continue;

			policy_mgr_debug("Add chan %u to mandatory list",
					 ch_freq_list[i]);
			pm_ctx->sap_mandatory_channels[
				pm_ctx->sap_mandatory_channels_len++] =
				ch_freq_list[i];
		}
	}
}
#else
static inline
void  policy_mgr_add_sap_mandatory_6ghz_chan(struct wlan_objmgr_psoc *psoc)
{
}
#endif

/**
 * policy_mgr_init_sap_mandatory_chan_by_band() - Init SAP mandatory channel
 * list based on band
 * @psoc: Pointer to soc
 * @band_bitmap: band bitmap of type reg_wifi_band
 *
 * Initialize the 2.4G 5G 6G SAP mandatory channels based on band
 *
 * Return: None
 */
static void
policy_mgr_init_sap_mandatory_chan_by_band(struct wlan_objmgr_psoc *psoc,
					   uint32_t band_bitmap)
{
	uint32_t ch_freq_list[NUM_CHANNELS] = {0};
	uint32_t len = 0;
	int i;
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	status = policy_mgr_get_valid_chans(psoc, ch_freq_list, &len);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("Error in getting valid channels");
		return;
	}
	pm_ctx->sap_mandatory_channels_len = 0;
	for (i = 0; (i < len) && (i < NUM_CHANNELS); i++) {
		if (WLAN_REG_IS_24GHZ_CH_FREQ(ch_freq_list[i])) {
			policy_mgr_debug("Add chan %u to mandatory list",
					ch_freq_list[i]);
			pm_ctx->sap_mandatory_channels[
				pm_ctx->sap_mandatory_channels_len++] =
				ch_freq_list[i];
		}
	}
	if (band_bitmap & BIT(REG_BAND_5G))
		for (i = 0; i < ARRAY_SIZE(sap_mand_5g_freq_list); i++)
			policy_mgr_add_sap_mandatory_chan(
				psoc, sap_mand_5g_freq_list[i]);
	if (band_bitmap & BIT(REG_BAND_6G))
		policy_mgr_add_sap_mandatory_6ghz_chan(psoc);
}

void  policy_mgr_init_sap_mandatory_chan(struct wlan_objmgr_psoc *psoc,
					 uint32_t org_ch_freq)
{
	if (WLAN_REG_IS_5GHZ_CH_FREQ(org_ch_freq)) {
		policy_mgr_debug("channel %u, sap mandatory chan list enabled",
				 org_ch_freq);
		policy_mgr_init_sap_mandatory_chan_by_band(
			psoc, BIT(REG_BAND_2G) | BIT(REG_BAND_5G));
		policy_mgr_add_sap_mandatory_chan(
			psoc, org_ch_freq);
	} else if (WLAN_REG_IS_6GHZ_CHAN_FREQ(org_ch_freq)) {
		policy_mgr_init_sap_mandatory_chan_by_band(
				psoc,
				BIT(REG_BAND_2G) | BIT(REG_BAND_5G) |
				BIT(REG_BAND_6G));
	} else {
		policy_mgr_init_sap_mandatory_chan_by_band(
				psoc, BIT(REG_BAND_2G));
	}
}

void policy_mgr_remove_sap_mandatory_chan(struct wlan_objmgr_psoc *psoc,
					  uint32_t ch_freq)
{
	uint32_t ch_freq_list[NUM_CHANNELS] = {0};
	uint32_t num_chan = 0;
	int i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	if (pm_ctx->sap_mandatory_channels_len >= NUM_CHANNELS) {
		policy_mgr_err("Invalid channel len %d ",
				pm_ctx->sap_mandatory_channels_len);
		return;
	}

	for (i = 0; i < pm_ctx->sap_mandatory_channels_len; i++) {
		if (ch_freq == pm_ctx->sap_mandatory_channels[i])
			continue;
		ch_freq_list[num_chan++] = pm_ctx->sap_mandatory_channels[i];
	}

	qdf_mem_zero(pm_ctx->sap_mandatory_channels,
		     pm_ctx->sap_mandatory_channels_len *
		     sizeof(*pm_ctx->sap_mandatory_channels));
	qdf_mem_copy(pm_ctx->sap_mandatory_channels, ch_freq_list,
		     num_chan * sizeof(*pm_ctx->sap_mandatory_channels));
	pm_ctx->sap_mandatory_channels_len = num_chan;
}
