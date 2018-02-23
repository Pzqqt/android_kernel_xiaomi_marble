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

#define POLICY_MGR_MAX_CON_STRING_LEN   100

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
	enum dbs_support dbs_type = wlan_objmgr_psoc_get_dual_mac_disable(psoc);

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

uint32_t policy_mgr_mcc_to_scc_switch_mode_in_user_cfg(
	struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return 0;
	}

	return pm_ctx->user_cfg.mcc_to_scc_switch_mode;
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
 * @dbs: DBS capability of type 'hw_mode_dbs_capab'
 * @dfs: Agile DFS capability of type 'hw_mode_agile_dfs_capab'
 * @sbs: SBS capability of type 'hw_mode_sbs_capab'
 *
 * Fetches the HW mode index corresponding to the HW mode provided
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
		enum hw_mode_dbs_capab dbs,
		enum hw_mode_agile_dfs_capab dfs,
		enum hw_mode_sbs_capab sbs)
{
	uint32_t i;
	uint32_t t_mac0_tx_ss, t_mac0_rx_ss, t_mac0_bw;
	uint32_t t_mac1_tx_ss, t_mac1_rx_ss, t_mac1_bw;
	uint32_t dbs_mode, agile_dfs_mode, sbs_mode;
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

		found = i;
		policy_mgr_debug("hw_mode index %d found", i);
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
 * @dbs: HW DBS capability
 * @dfs: HW Agile DFS capability
 * @sbs: HW SBS capability
 *
 * Get the HW mode index corresponding to the HW modes spatial stream,
 * bandwidth, DBS, Agile DFS and SBS capability
 *
 * Return: Index number if a match is found or -negative value if not found
 */
int8_t policy_mgr_get_hw_mode_idx_from_dbs_hw_list(
		struct wlan_objmgr_psoc *psoc,
		enum hw_mode_ss_config mac0_ss,
		enum hw_mode_bandwidth mac0_bw,
		enum hw_mode_ss_config mac1_ss,
		enum hw_mode_bandwidth mac1_bw,
		enum hw_mode_dbs_capab dbs,
		enum hw_mode_agile_dfs_capab dfs,
		enum hw_mode_sbs_capab sbs)
{
	uint32_t mac0_tx_ss, mac0_rx_ss;
	uint32_t mac1_tx_ss, mac1_rx_ss;

	policy_mgr_get_tx_rx_ss_from_config(mac0_ss, &mac0_tx_ss, &mac0_rx_ss);
	policy_mgr_get_tx_rx_ss_from_config(mac1_ss, &mac1_tx_ss, &mac1_rx_ss);

	policy_mgr_debug("MAC0: TxSS=%d, RxSS=%d, BW=%d",
		mac0_tx_ss, mac0_rx_ss, mac0_bw);
	policy_mgr_debug("MAC1: TxSS=%d, RxSS=%d, BW=%d",
		mac1_tx_ss, mac1_rx_ss, mac1_bw);
	policy_mgr_debug("DBS=%d, Agile DFS=%d, SBS=%d",
		dbs, dfs, sbs);

	return policy_mgr_get_matching_hw_mode_index(psoc, mac0_tx_ss,
						mac0_rx_ss,
						mac0_bw,
						mac1_tx_ss, mac1_rx_ss,
						mac1_bw,
						dbs, dfs, sbs);
}

/**
 * policy_mgr_get_hw_mode_from_idx() - Get HW mode based on index
 * @idx: HW mode index
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

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return QDF_STATUS_E_FAILURE;
	}

	if (idx > pm_ctx->num_dbs_hw_modes) {
		policy_mgr_err("Invalid index");
		return QDF_STATUS_E_FAILURE;
	}

	if (!pm_ctx->num_dbs_hw_modes) {
		policy_mgr_err("No dbs hw modes available");
		return QDF_STATUS_E_FAILURE;
	}

	param = pm_ctx->hw_mode.hw_mode_list[idx];

	hw_mode->mac0_tx_ss = POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_GET(param);
	hw_mode->mac0_rx_ss = POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_GET(param);
	hw_mode->mac0_bw = POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_GET(param);
	hw_mode->mac1_tx_ss = POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_GET(param);
	hw_mode->mac1_rx_ss = POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_GET(param);
	hw_mode->mac1_bw = POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_GET(param);
	hw_mode->dbs_cap = POLICY_MGR_HW_MODE_DBS_MODE_GET(param);
	hw_mode->agile_dfs_cap = POLICY_MGR_HW_MODE_AGILE_DFS_GET(param);
	hw_mode->sbs_cap = POLICY_MGR_HW_MODE_SBS_MODE_GET(param);

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

/**
 * policy_mgr_update_conc_list() - Update the concurrent connection list
 * @conn_index: Connection index
 * @mode: Mode
 * @chan: Channel
 * @bw: Bandwidth
 * @mac: Mac id
 * @chain_mask: Chain mask
 * @vdev_id: vdev id
 * @in_use: Flag to indicate if the index is in use or not
 *
 * Updates the index value of the concurrent connection list
 *
 * Return: None
 */
void policy_mgr_update_conc_list(struct wlan_objmgr_psoc *psoc,
		uint32_t conn_index,
		enum policy_mgr_con_mode mode,
		uint8_t chan,
		enum hw_mode_bandwidth bw,
		uint8_t mac,
		enum policy_mgr_chain_mode chain_mask,
		uint32_t original_nss,
		uint32_t vdev_id,
		bool in_use)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

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
	pm_conc_connection_list[conn_index].mode = mode;
	pm_conc_connection_list[conn_index].chan = chan;
	pm_conc_connection_list[conn_index].bw = bw;
	pm_conc_connection_list[conn_index].mac = mac;
	pm_conc_connection_list[conn_index].chain_mask = chain_mask;
	pm_conc_connection_list[conn_index].original_nss = original_nss;
	pm_conc_connection_list[conn_index].vdev_id = vdev_id;
	pm_conc_connection_list[conn_index].in_use = in_use;

	if (pm_ctx->mode_change_cb)
		pm_ctx->mode_change_cb();

	policy_mgr_dump_connection_status_info(psoc);
	if (pm_ctx->cdp_cbacks.cdp_update_mac_id)
		pm_ctx->cdp_cbacks.cdp_update_mac_id(psoc, vdev_id, mac);

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
			policy_mgr_notice("Stored %d (%d), deleted STA entry with vdev id %d, index %d",
				info[found_index].vdev_id,
				info[found_index].mode,
				info[found_index].vdev_id, conn_index);
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
		policy_mgr_err("Mode:%d not available in the conn info", mode);
	} else {
		*num_cxn_del = found_index;
		policy_mgr_err("Mode:%d number of conn %d temp del",
				mode, *num_cxn_del);
	}

	/*
	 * Caller should set the PCL and restore the connection entry
	 * in conn info.
	 */
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

	if (MAX_NUMBER_OF_CONC_CONNECTIONS <= num_cxn_del || 0 == num_cxn_del) {
		policy_mgr_err("Failed to restore %d/%d deleted information",
				num_cxn_del, MAX_NUMBER_OF_CONC_CONNECTIONS);
		return;
	}
	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	conn_index = policy_mgr_get_connection_count(psoc);
	if (MAX_NUMBER_OF_CONC_CONNECTIONS <= conn_index) {
		policy_mgr_err("Failed to restore the deleted information %d/%d",
			conn_index, MAX_NUMBER_OF_CONC_CONNECTIONS);
		return;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	qdf_mem_copy(&pm_conc_connection_list[conn_index], info,
			num_cxn_del * sizeof(*info));
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	policy_mgr_debug("Restored the deleleted conn info, vdev:%d, index:%d",
		info->vdev_id, conn_index);
}

/**
 * policy_mgr_update_hw_mode_conn_info() - Update connection
 * info based on HW mode
 * @num_vdev_mac_entries: Number of vdev-mac id entries that follow
 * @vdev_mac_map: Mapping of vdev-mac id
 * @hw_mode: HW mode
 *
 * Updates the connection info parameters based on the new HW mode
 *
 * Return: None
 */
void policy_mgr_update_hw_mode_conn_info(struct wlan_objmgr_psoc *psoc,
				uint32_t num_vdev_mac_entries,
				struct policy_mgr_vdev_mac_map *vdev_mac_map,
				struct policy_mgr_hw_mode_params hw_mode)
{
	uint32_t i, conn_index, found;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

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
				uint32_t session_id, void *context)
{
	QDF_STATUS ret;
	struct policy_mgr_hw_mode_params hw_mode;
	uint32_t i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(context);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	policy_mgr_set_hw_mode_change_in_progress(context,
		POLICY_MGR_HW_MODE_NOT_IN_PROGRESS);

	if (status != SET_HW_MODE_STATUS_OK) {
		policy_mgr_err("Set HW mode failed with status %d", status);
		return;
	}

	if (!vdev_mac_map) {
		policy_mgr_err("vdev_mac_map is NULL");
		return;
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
		return;
	}

	policy_mgr_debug("MAC0: TxSS:%d, RxSS:%d, Bw:%d",
		hw_mode.mac0_tx_ss, hw_mode.mac0_rx_ss, hw_mode.mac0_bw);
	policy_mgr_debug("MAC1: TxSS:%d, RxSS:%d, Bw:%d",
		hw_mode.mac1_tx_ss, hw_mode.mac1_rx_ss, hw_mode.mac1_bw);
	policy_mgr_debug("DBS:%d, Agile DFS:%d, SBS:%d",
		hw_mode.dbs_cap, hw_mode.agile_dfs_cap, hw_mode.sbs_cap);

	/* update pm_conc_connection_list */
	policy_mgr_update_hw_mode_conn_info(context, num_vdev_mac_entries,
			vdev_mac_map,
			hw_mode);
	if (pm_ctx->mode_change_cb)
		pm_ctx->mode_change_cb();

	ret = policy_mgr_set_connection_update(context);
	if (!QDF_IS_STATUS_SUCCESS(ret))
		policy_mgr_err("ERROR: set connection_update_done event failed");

	if (PM_NOP != next_action)
		policy_mgr_next_actions(context, session_id,
			next_action, reason);
	else {
		policy_mgr_debug("No action needed right now");
		ret = policy_mgr_set_opportunistic_update(context);
		if (!QDF_IS_STATUS_SUCCESS(ret))
			policy_mgr_err("ERROR: set opportunistic_update event failed");
	}

	return;
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
	case PM_IBSS_MODE:
		count = strlcat(cc_mode, "IBSS",
					length);
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
	case PM_IBSS_MODE:
		count = policy_mgr_dump_current_concurrency_one_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+IBSS",
					length);
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
	case PM_IBSS_MODE:
		count = policy_mgr_dump_current_concurrency_two_connection(
				cc_mode, length);
		count += strlcat(cc_mode, "+IBSS",
					length);
		break;
	default:
		policy_mgr_err("unexpected mode %d", mode);
		break;
	}

	return count;
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
	char buf[4] = {0};
	uint8_t mac = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	strlcat(cc_mode, " DBS", length);
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	if (pm_conc_connection_list[0].mac ==
		pm_conc_connection_list[1].mac) {
		if (pm_conc_connection_list[0].chan ==
			pm_conc_connection_list[1].chan)
			strlcat(cc_mode,
				" with SCC for 1st two connections on mac ",
				length);
		else
			strlcat(cc_mode,
				" with MCC for 1st two connections on mac ",
				length);
		mac = pm_conc_connection_list[0].mac;
	}
	if (pm_conc_connection_list[0].mac == pm_conc_connection_list[2].mac) {
		if (pm_conc_connection_list[0].chan ==
			pm_conc_connection_list[2].chan)
			strlcat(cc_mode,
				" with SCC for 1st & 3rd connections on mac ",
				length);
		else
			strlcat(cc_mode,
				" with MCC for 1st & 3rd connections on mac ",
				length);
		mac = pm_conc_connection_list[0].mac;
	}
	if (pm_conc_connection_list[1].mac == pm_conc_connection_list[2].mac) {
		if (pm_conc_connection_list[1].chan ==
			pm_conc_connection_list[2].chan)
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
		policy_mgr_err("%s Standalone", cc_mode);
		break;
	case 2:
		count = policy_mgr_dump_current_concurrency_two_connection(
			cc_mode, sizeof(cc_mode));
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
		if (pm_conc_connection_list[0].chan ==
			pm_conc_connection_list[1].chan) {
			strlcat(cc_mode, " SCC", sizeof(cc_mode));
		} else if (pm_conc_connection_list[0].mac ==
					pm_conc_connection_list[1].mac) {
			strlcat(cc_mode, " MCC", sizeof(cc_mode));
		} else
			strlcat(cc_mode, " DBS", sizeof(cc_mode));
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		policy_mgr_err("%s", cc_mode);
		break;
	case 3:
		count = policy_mgr_dump_current_concurrency_three_connection(
			cc_mode, sizeof(cc_mode));
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
		if ((pm_conc_connection_list[0].chan ==
			pm_conc_connection_list[1].chan) &&
			(pm_conc_connection_list[0].chan ==
				pm_conc_connection_list[2].chan)){
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
				strlcat(cc_mode, " SCC",
						sizeof(cc_mode));
		} else if ((pm_conc_connection_list[0].mac ==
				pm_conc_connection_list[1].mac)
				&& (pm_conc_connection_list[0].mac ==
					pm_conc_connection_list[2].mac)) {
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
					strlcat(cc_mode, " MCC on single MAC",
						sizeof(cc_mode));
		} else {
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
			policy_mgr_dump_dbs_concurrency(psoc, cc_mode,
					sizeof(cc_mode));
		}
		policy_mgr_err("%s", cc_mode);
		break;
	default:
		policy_mgr_err("unexpected num_connections value %d",
			num_connections);
		break;
	}

	return;
}

/**
 * policy_mgr_pdev_set_pcl() - Sets PCL to FW
 * @mode: adapter mode
 *
 * Fetches the PCL and sends the PCL to SME
 * module which in turn will send the WMI
 * command WMI_PDEV_SET_PCL_CMDID to the fw
 *
 * Return: None
 */
void policy_mgr_pdev_set_pcl(struct wlan_objmgr_psoc *psoc,
				enum QDF_OPMODE mode)
{
	QDF_STATUS status;
	enum policy_mgr_con_mode con_mode;
	struct policy_mgr_pcl_list pcl;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	pcl.pcl_len = 0;

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
	case QDF_IBSS_MODE:
		con_mode = PM_IBSS_MODE;
		break;
	default:
		policy_mgr_err("Unable to set PCL to FW: %d", mode);
		return;
	}

	policy_mgr_debug("get pcl to set it to the FW");

	status = policy_mgr_get_pcl(psoc, con_mode,
			pcl.pcl_list, &pcl.pcl_len,
			pcl.weight_list, QDF_ARRAY_SIZE(pcl.weight_list));
	if (status != QDF_STATUS_SUCCESS) {
		policy_mgr_err("Unable to set PCL to FW, Get PCL failed");
		return;
	}

	status = pm_ctx->sme_cbacks.sme_pdev_set_pcl(pcl);
	if (status != QDF_STATUS_SUCCESS)
		policy_mgr_err("Send soc set PCL to SME failed");
	else
		policy_mgr_debug("Set PCL to FW for mode:%d", mode);
}


/**
 * policy_mgr_set_pcl_for_existing_combo() - Set PCL for existing connection
 * @mode: Connection mode of type 'policy_mgr_con_mode'
 *
 * Set the PCL for an existing connection
 *
 * Return: None
 */
void policy_mgr_set_pcl_for_existing_combo(
		struct wlan_objmgr_psoc *psoc, enum policy_mgr_con_mode mode)
{
	struct policy_mgr_conc_connection_info
			info[MAX_NUMBER_OF_CONC_CONNECTIONS] = { {0} };
	enum QDF_OPMODE pcl_mode;
	uint8_t num_cxn_del = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	switch (mode) {
	case PM_STA_MODE:
		pcl_mode = QDF_STA_MODE;
		break;
	case PM_SAP_MODE:
		pcl_mode = QDF_SAP_MODE;
		break;
	case PM_P2P_CLIENT_MODE:
		pcl_mode = QDF_P2P_CLIENT_MODE;
		break;
	case PM_P2P_GO_MODE:
		pcl_mode = QDF_P2P_GO_MODE;
		break;
	case PM_IBSS_MODE:
		pcl_mode = QDF_IBSS_MODE;
		break;
	default:
		policy_mgr_err("Invalid mode to set PCL");
		return;
	};
	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	if (policy_mgr_mode_specific_connection_count(psoc, mode, NULL) > 0) {
		/* Check, store and temp delete the mode's parameter */
		policy_mgr_store_and_del_conn_info(psoc, mode, false,
						info, &num_cxn_del);
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		/* Set the PCL to the FW since connection got updated */
		policy_mgr_pdev_set_pcl(psoc, pcl_mode);
		policy_mgr_debug("Set PCL to FW for mode:%d", mode);
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
		/* Restore the connection info */
		policy_mgr_restore_deleted_conn_info(psoc, info, num_cxn_del);
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
}

static uint32_t pm_get_vdev_id_of_first_conn_idx(struct wlan_objmgr_psoc *psoc)
{
	uint32_t conn_index = 0, vdev_id = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

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
			break;
		}
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
	if (conn_index == MAX_NUMBER_OF_CONC_CONNECTIONS)
		policy_mgr_debug("Use default vdev_id:%d for opportunistic upgrade",
				 vdev_id);
	else
		policy_mgr_debug("Use vdev_id:%d for opportunistic upgrade",
				 vdev_id);

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

	if (!psoc) {
		policy_mgr_err("Invalid Context");
		return;
	}

	/* if we still need it */
	action = policy_mgr_need_opportunistic_upgrade(psoc);
	policy_mgr_debug("action:%d", action);
	if (!action)
		return;
	session_id = pm_get_vdev_id_of_first_conn_idx(psoc);
	policy_mgr_next_actions(psoc, session_id, action,
				POLICY_MGR_UPDATE_REASON_OPPORTUNISTIC);
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
	} else if (type == WMI_VDEV_TYPE_IBSS) {
		mode = PM_IBSS_MODE;
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
	default:
		policy_mgr_err("Unknown channel BW type %d", chan_width);
		break;
	}

	return bw;
}

/**
 * policy_mgr_get_sbs_channels() - provides the sbs channel(s)
 * with respect to current connection(s)
 * @channels:	the channel(s) on which current connection(s) is
 * @len:	Number of channels
 * @pcl_weight: Pointer to the weights of PCL
 * @weight_len: Max length of the weight list
 * @index: Index from which the weight list needs to be populated
 * @group_id: Next available groups for weight assignment
 * @available_5g_channels: List of available 5g channels
 * @available_5g_channels_len: Length of the 5g channels list
 * @add_5g_channels: If this flag is true append 5G channel list as well
 *
 * This function provides the channel(s) on which current
 * connection(s) is/are
 *
 * Return: QDF_STATUS
 */

static QDF_STATUS policy_mgr_get_sbs_channels(uint8_t *channels,
		uint32_t *len, uint8_t *pcl_weight, uint32_t weight_len,
		uint32_t *index, enum policy_mgr_pcl_group_id group_id,
		uint8_t *available_5g_channels,
		uint32_t available_5g_channels_len,
		bool add_5g_channels)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t conn_index = 0, num_channels = 0;
	uint32_t num_5g_channels = 0, cur_5g_channel = 0;
	uint8_t remaining_5g_Channels[QDF_MAX_NUM_CHAN] = {};
	uint32_t remaining_channel_index = 0;
	uint32_t j = 0, i = 0, weight1, weight2;

	if ((NULL == channels) || (NULL == len)) {
		policy_mgr_err("channels or len is NULL");
		status = QDF_STATUS_E_FAILURE;
		return status;
	}

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

	policy_mgr_debug("weight1=%d weight2=%d index=%d ",
		weight1, weight2, *index);

	while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
		if ((WLAN_REG_IS_5GHZ_CH(
			pm_conc_connection_list[conn_index].chan))
			&& (pm_conc_connection_list[conn_index].in_use)) {
			num_5g_channels++;
			cur_5g_channel =
				pm_conc_connection_list[conn_index].chan;
		}
		conn_index++;
	}

	conn_index = 0;
	if (num_5g_channels > 1) {
		/* This case we are already in SBS so return the channels */
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			channels[num_channels++] =
				pm_conc_connection_list[conn_index++].chan;
			if (*index < weight_len)
				pcl_weight[(*index)++] = weight1;
		}
		*len = num_channels;
		/* fix duplicate issue later */
		if (add_5g_channels)
			for (j = 0; j < available_5g_channels_len; j++)
				remaining_5g_Channels[
				remaining_channel_index++] =
				available_5g_channels[j];
	} else {
		/* Get list of valid sbs channels for the current
		 * connected channel
		 */
		for (j = 0; j < available_5g_channels_len; j++) {
			if (WLAN_REG_IS_CHANNEL_VALID_5G_SBS(
			cur_5g_channel, available_5g_channels[j])) {
				channels[num_channels++] =
					available_5g_channels[j];
			} else {
				remaining_5g_Channels[
				remaining_channel_index++] =
				available_5g_channels[j];
				continue;
			}
			if (*index < weight_len)
				pcl_weight[(*index)++] = weight1;
		}
		*len = num_channels;
	}

	if (add_5g_channels) {
		qdf_mem_copy(channels+num_channels, remaining_5g_Channels,
			remaining_channel_index);
		*len += remaining_channel_index;
		for (i = 0; ((i < remaining_channel_index)
					&& (i < weight_len)); i++)
			pcl_weight[i] = weight2;
	}

	return status;
}


/**
 * policy_mgr_get_connection_channels() - provides the channel(s)
 * on which current connection(s) is
 * @channels:	the channel(s) on which current connection(s) is
 * @len:	Number of channels
 * @order:	no order OR 2.4 Ghz channel followed by 5 Ghz
 *	channel OR 5 Ghz channel followed by 2.4 Ghz channel
 * @skip_dfs_channel: if this flag is true then skip the dfs channel
 * @pcl_weight: Pointer to the weights of PCL
 * @weight_len: Max length of the weight list
 * @index: Index from which the weight list needs to be populated
 * @group_id: Next available groups for weight assignment
 *
 *
 * This function provides the channel(s) on which current
 * connection(s) is/are
 *
 * Return: QDF_STATUS
 */
static
QDF_STATUS policy_mgr_get_connection_channels(struct wlan_objmgr_psoc *psoc,
			uint8_t *channels,
			uint32_t *len, enum policy_mgr_pcl_channel_order order,
			bool skip_dfs_channel,
			uint8_t *pcl_weight, uint32_t weight_len,
			uint32_t *index, enum policy_mgr_pcl_group_id group_id)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t conn_index = 0, num_channels = 0;
	uint32_t weight1, weight2;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}

	if ((NULL == channels) || (NULL == len)) {
		policy_mgr_err("channels or len is NULL");
		status = QDF_STATUS_E_FAILURE;
		return status;
	}

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
	} else {
		weight1 = WEIGHT_OF_GROUP2_PCL_CHANNELS;
		weight2 = WEIGHT_OF_GROUP3_PCL_CHANNELS;
	}

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	if (POLICY_MGR_PCL_ORDER_NONE == order) {
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			if (skip_dfs_channel && wlan_reg_is_dfs_ch(pm_ctx->pdev,
				    pm_conc_connection_list[conn_index].chan)) {
				conn_index++;
			} else if (*index < weight_len) {
				channels[num_channels++] =
				pm_conc_connection_list[conn_index++].chan;
				pcl_weight[(*index)++] = weight1;
			} else {
				conn_index++;
			}
		}
		*len = num_channels;
	} else if (POLICY_MGR_PCL_ORDER_24G_THEN_5G == order) {
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			if (WLAN_REG_IS_24GHZ_CH(
				    pm_conc_connection_list[conn_index].chan)
				&& (*index < weight_len)) {
				channels[num_channels++] =
				pm_conc_connection_list[conn_index++].chan;
				pcl_weight[(*index)++] = weight1;
			} else {
				conn_index++;
			}
		}
		conn_index = 0;
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			if (skip_dfs_channel && wlan_reg_is_dfs_ch(pm_ctx->pdev,
				    pm_conc_connection_list[conn_index].chan)) {
				conn_index++;
			} else if (WLAN_REG_IS_5GHZ_CH(
				    pm_conc_connection_list[conn_index].chan)
				&& (*index < weight_len)) {
				channels[num_channels++] =
				pm_conc_connection_list[conn_index++].chan;
				pcl_weight[(*index)++] = weight2;
			} else {
				conn_index++;
			}
		}
		*len = num_channels;
	} else if (POLICY_MGR_PCL_ORDER_5G_THEN_2G == order) {
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			if (skip_dfs_channel && wlan_reg_is_dfs_ch(pm_ctx->pdev,
				pm_conc_connection_list[conn_index].chan)) {
				conn_index++;
			} else if (WLAN_REG_IS_5GHZ_CH(
				    pm_conc_connection_list[conn_index].chan)
				&& (*index < weight_len)) {
				channels[num_channels++] =
				pm_conc_connection_list[conn_index++].chan;
				pcl_weight[(*index)++] = weight1;
			} else {
				conn_index++;
			}
		}
		conn_index = 0;
		while (PM_CONC_CONNECTION_LIST_VALID_INDEX(conn_index)) {
			if (WLAN_REG_IS_24GHZ_CH(
				    pm_conc_connection_list[conn_index].chan)
				&& (*index < weight_len)) {
				channels[num_channels++] =
				pm_conc_connection_list[conn_index++].chan;
				pcl_weight[(*index)++] = weight2;

			} else {
				conn_index++;
			}
		}
		*len = num_channels;
	} else {
		policy_mgr_err("unknown order %d", order);
		status = QDF_STATUS_E_FAILURE;
	}
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
		struct wlan_objmgr_psoc *psoc, uint8_t *pcl_channels,
		uint32_t *len, uint8_t *weight_list, uint32_t weight_len)
{
	uint8_t i;
	uint32_t orig_channel_count = 0;
	bool mcc_to_scc_mode;
	uint32_t sap_count;
	enum channel_state channel_state;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	mcc_to_scc_mode = policy_mgr_is_force_scc(psoc);
	sap_count = policy_mgr_mode_specific_connection_count(psoc,
			PM_SAP_MODE, NULL);
	policy_mgr_debug("mcc_to_scc_mode %u, sap_count %u", mcc_to_scc_mode,
			sap_count);

	if (!mcc_to_scc_mode || !sap_count)
		return;

	if (len)
		orig_channel_count = QDF_MIN(*len, QDF_MAX_NUM_CHAN);
	else {
		policy_mgr_err("invalid number of channel length");
		return;
	}

	policy_mgr_debug("Set weight of DFS/passive channels to 0");

	for (i = 0; i < orig_channel_count; i++) {
		channel_state = reg_get_channel_state(pm_ctx->pdev,
				pcl_channels[i]);
		if ((channel_state == CHANNEL_STATE_DISABLE) ||
				(channel_state == CHANNEL_STATE_INVALID))
			/* Set weight of inactive channels to 0 */
			weight_list[i] = 0;

		policy_mgr_debug("chan[%d] - %d, weight[%d] - %d",
				i, pcl_channels[i], i, weight_list[i]);
	}

	return;
}

/**
 * policy_mgr_get_channel_list() - provides the channel list
 * suggestion for new connection
 * @pcl:	The preferred channel list enum
 * @pcl_channels: PCL channels
 * @len: length of the PCL
 * @mode: concurrency mode for which channel list is requested
 * @pcl_weights: Weights of the PCL
 * @weight_len: Max length of the weight list
 *
 * This function provides the actual channel list based on the
 * current regulatory domain derived using preferred channel
 * list enum obtained from one of the pcl_table
 *
 * Return: Channel List
 */
QDF_STATUS policy_mgr_get_channel_list(struct wlan_objmgr_psoc *psoc,
			enum policy_mgr_pcl_type pcl,
			uint8_t *pcl_channels, uint32_t *len,
			enum policy_mgr_con_mode mode,
			uint8_t *pcl_weights, uint32_t weight_len)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t num_channels = 0;
	uint32_t sbs_num_channels = 0;
	uint32_t chan_index = 0, chan_index_24 = 0, chan_index_5 = 0;
	uint8_t channel_list[QDF_MAX_NUM_CHAN] = {0};
	uint8_t channel_list_24[QDF_MAX_NUM_CHAN] = {0};
	uint8_t channel_list_5[QDF_MAX_NUM_CHAN] = {0};
	uint8_t sbs_channel_list[QDF_MAX_NUM_CHAN] = {0};
	bool skip_dfs_channel = false;
	uint32_t i = 0, j = 0;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}

	if ((NULL == pcl_channels) || (NULL == len)) {
		policy_mgr_err("pcl_channels or len is NULL");
		return status;
	}

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
	/* get the channel list for current domain */
	status = policy_mgr_get_valid_chans(psoc, channel_list, &num_channels);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("Error in getting valid channels");
		return status;
	}

	/*
	 * if you have atleast one STA connection then don't fill DFS channels
	 * in the preferred channel list
	 */
	if (((mode == PM_SAP_MODE) || (mode == PM_P2P_GO_MODE)) &&
	    (policy_mgr_mode_specific_connection_count(
		psoc, PM_STA_MODE, NULL) > 0)) {
		policy_mgr_debug("STA present, skip DFS channels from pcl for SAP/Go");
		skip_dfs_channel = true;
	}

	/* Let's divide the list in 2.4 & 5 Ghz lists */
	while ((chan_index < QDF_MAX_NUM_CHAN) &&
		(channel_list[chan_index] <= 11) &&
		(chan_index_24 < QDF_MAX_NUM_CHAN))
		channel_list_24[chan_index_24++] = channel_list[chan_index++];
	if ((chan_index < QDF_MAX_NUM_CHAN) &&
		(channel_list[chan_index] == 12) &&
		(chan_index_24 < QDF_MAX_NUM_CHAN)) {
		channel_list_24[chan_index_24++] = channel_list[chan_index++];
		if ((chan_index < QDF_MAX_NUM_CHAN) &&
			(channel_list[chan_index] == 13) &&
			(chan_index_24 < QDF_MAX_NUM_CHAN)) {
			channel_list_24[chan_index_24++] =
				channel_list[chan_index++];
			if ((chan_index < QDF_MAX_NUM_CHAN) &&
				(channel_list[chan_index] == 14) &&
				(chan_index_24 < QDF_MAX_NUM_CHAN))
				channel_list_24[chan_index_24++] =
					channel_list[chan_index++];
		}
	}

	while ((chan_index < num_channels) &&
		(chan_index_5 < QDF_MAX_NUM_CHAN)) {
		if ((true == skip_dfs_channel) &&
		    wlan_reg_is_dfs_ch(pm_ctx->pdev,
				       channel_list[chan_index])) {
			chan_index++;
			continue;
		}
		channel_list_5[chan_index_5++] = channel_list[chan_index++];
	}

	num_channels = 0;
	sbs_num_channels = 0;
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
		chan_index_24 = QDF_MIN(chan_index_24, weight_len);
		qdf_mem_copy(pcl_channels, channel_list_24,
			chan_index_24);
		*len = chan_index_24;
		for (i = 0; i < *len; i++)
			pcl_weights[i] = WEIGHT_OF_GROUP1_PCL_CHANNELS;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_5G:
		chan_index_5 = QDF_MIN(chan_index_5, weight_len);
		qdf_mem_copy(pcl_channels, channel_list_5,
			chan_index_5);
		*len = chan_index_5;
		for (i = 0; i < *len; i++)
			pcl_weights[i] = WEIGHT_OF_GROUP1_PCL_CHANNELS;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_CH:
	case PM_MCC_CH:
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels, POLICY_MGR_PCL_ORDER_NONE,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID1_ID2);
		qdf_mem_copy(pcl_channels, channel_list, num_channels);
		*len = num_channels;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_CH_24G:
	case PM_MCC_CH_24G:
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels, POLICY_MGR_PCL_ORDER_NONE,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID1_ID2);
		qdf_mem_copy(pcl_channels, channel_list, num_channels);
		*len = num_channels;
		chan_index_24 = QDF_MIN((num_channels + chan_index_24),
					weight_len) - num_channels;
		qdf_mem_copy(&pcl_channels[num_channels],
			channel_list_24, chan_index_24);
		*len += chan_index_24;
		for (j = 0; j < chan_index_24; i++, j++)
			pcl_weights[i] = WEIGHT_OF_GROUP2_PCL_CHANNELS;

		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_CH_5G:
	case PM_MCC_CH_5G:
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels, POLICY_MGR_PCL_ORDER_NONE,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID1_ID2);
		qdf_mem_copy(pcl_channels, channel_list,
			num_channels);
		*len = num_channels;
		chan_index_5 = QDF_MIN((num_channels + chan_index_5),
					weight_len) - num_channels;
		qdf_mem_copy(&pcl_channels[num_channels],
			channel_list_5, chan_index_5);
		*len += chan_index_5;
		for (j = 0; j < chan_index_5; i++, j++)
			pcl_weights[i] = WEIGHT_OF_GROUP2_PCL_CHANNELS;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_24G_SCC_CH:
	case PM_24G_MCC_CH:
		chan_index_24 = QDF_MIN(chan_index_24, weight_len);
		qdf_mem_copy(pcl_channels, channel_list_24,
			chan_index_24);
		*len = chan_index_24;
		for (i = 0; i < chan_index_24; i++)
			pcl_weights[i] = WEIGHT_OF_GROUP1_PCL_CHANNELS;
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels, POLICY_MGR_PCL_ORDER_NONE,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID2_ID3);
		qdf_mem_copy(&pcl_channels[chan_index_24],
			channel_list, num_channels);
		*len += num_channels;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_5G_SCC_CH:
	case PM_5G_MCC_CH:
		chan_index_5 = QDF_MIN(chan_index_5, weight_len);
		qdf_mem_copy(pcl_channels, channel_list_5,
			chan_index_5);
		*len = chan_index_5;
		for (i = 0; i < chan_index_5; i++)
			pcl_weights[i] = WEIGHT_OF_GROUP1_PCL_CHANNELS;
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels, POLICY_MGR_PCL_ORDER_NONE,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID2_ID3);
		qdf_mem_copy(&pcl_channels[chan_index_5],
			channel_list, num_channels);
		*len += num_channels;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_24_SCC_ON_5:
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels,
			POLICY_MGR_PCL_ORDER_24G_THEN_5G,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID1_ID2);
		qdf_mem_copy(pcl_channels, channel_list,
			num_channels);
		*len = num_channels;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_5_SCC_ON_24:
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels,
			POLICY_MGR_PCL_ORDER_5G_THEN_2G,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID1_ID2);
		qdf_mem_copy(pcl_channels, channel_list, num_channels);
		*len = num_channels;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_24_SCC_ON_5_24G:
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels,
			POLICY_MGR_PCL_ORDER_24G_THEN_5G,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID1_ID2);
		qdf_mem_copy(pcl_channels, channel_list, num_channels);
		*len = num_channels;
		chan_index_24 = QDF_MIN((num_channels + chan_index_24),
					weight_len) - num_channels;
		qdf_mem_copy(&pcl_channels[num_channels],
			channel_list_24, chan_index_24);
		*len += chan_index_24;
		for (j = 0; j < chan_index_24; i++, j++)
			pcl_weights[i] = WEIGHT_OF_GROUP3_PCL_CHANNELS;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_24_SCC_ON_5_5G:
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels,
			POLICY_MGR_PCL_ORDER_24G_THEN_5G,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID1_ID2);
		qdf_mem_copy(pcl_channels, channel_list, num_channels);
		*len = num_channels;
		chan_index_5 = QDF_MIN((num_channels + chan_index_5),
					weight_len) - num_channels;
		qdf_mem_copy(&pcl_channels[num_channels],
			channel_list_5, chan_index_5);
		*len += chan_index_5;
		for (j = 0; j < chan_index_5; i++, j++)
			pcl_weights[i] = WEIGHT_OF_GROUP3_PCL_CHANNELS;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_5_SCC_ON_24_24G:
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels,
			POLICY_MGR_PCL_ORDER_5G_THEN_2G,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID1_ID2);
		qdf_mem_copy(pcl_channels, channel_list, num_channels);
		*len = num_channels;
		chan_index_24 = QDF_MIN((num_channels + chan_index_24),
					weight_len) - num_channels;
		qdf_mem_copy(&pcl_channels[num_channels],
			channel_list_24, chan_index_24);
		*len += chan_index_24;
		for (j = 0; j < chan_index_24; i++, j++)
			pcl_weights[i] = WEIGHT_OF_GROUP3_PCL_CHANNELS;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SCC_ON_5_SCC_ON_24_5G:
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels,
			POLICY_MGR_PCL_ORDER_5G_THEN_2G,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID1_ID2);
		qdf_mem_copy(pcl_channels, channel_list, num_channels);
		*len = num_channels;
		chan_index_5 = QDF_MIN((num_channels + chan_index_5),
					weight_len) - num_channels;
		qdf_mem_copy(&pcl_channels[num_channels],
			channel_list_5, chan_index_5);
		*len += chan_index_5;
		for (j = 0; j < chan_index_5; i++, j++)
			pcl_weights[i] = WEIGHT_OF_GROUP3_PCL_CHANNELS;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_24G_SCC_CH_SBS_CH:
		qdf_mem_copy(pcl_channels, channel_list_24,
			chan_index_24);
		*len = chan_index_24;
		for (i = 0; ((i < chan_index_24) && (i < weight_len)); i++)
			pcl_weights[i] = WEIGHT_OF_GROUP1_PCL_CHANNELS;
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels, POLICY_MGR_PCL_ORDER_NONE,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID2_ID3);
		qdf_mem_copy(&pcl_channels[chan_index_24],
			channel_list, num_channels);
		*len += num_channels;
		if (policy_mgr_is_hw_sbs_capable(psoc)) {
			policy_mgr_get_sbs_channels(
			sbs_channel_list, &sbs_num_channels, pcl_weights,
			weight_len, &i, POLICY_MGR_PCL_GROUP_ID3_ID4,
			channel_list_5, chan_index_5, false);
			qdf_mem_copy(
				&pcl_channels[chan_index_24 + num_channels],
				sbs_channel_list, sbs_num_channels);
			*len += sbs_num_channels;
		}
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_24G_SCC_CH_SBS_CH_5G:
		qdf_mem_copy(pcl_channels, channel_list_24,
			chan_index_24);
		*len = chan_index_24;
		for (i = 0; ((i < chan_index_24) && (i < weight_len)); i++)
			pcl_weights[i] = WEIGHT_OF_GROUP1_PCL_CHANNELS;
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels, POLICY_MGR_PCL_ORDER_NONE,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID2_ID3);
		qdf_mem_copy(&pcl_channels[chan_index_24],
			channel_list, num_channels);
		*len += num_channels;
		if (policy_mgr_is_hw_sbs_capable(psoc)) {
			policy_mgr_get_sbs_channels(
			sbs_channel_list, &sbs_num_channels, pcl_weights,
			weight_len, &i, POLICY_MGR_PCL_GROUP_ID3_ID4,
			channel_list_5, chan_index_5, true);
			qdf_mem_copy(
				&pcl_channels[chan_index_24 + num_channels],
				sbs_channel_list, sbs_num_channels);
			*len += sbs_num_channels;
		} else {
			qdf_mem_copy(
				&pcl_channels[chan_index_24 + num_channels],
				channel_list_5, chan_index_5);
			*len += chan_index_5;
			for (i = chan_index_24 + num_channels;
				((i < *len) && (i < weight_len)); i++)
				pcl_weights[i] = WEIGHT_OF_GROUP3_PCL_CHANNELS;
		}
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_24G_SBS_CH_MCC_CH:
		qdf_mem_copy(pcl_channels, channel_list_24,
			chan_index_24);
		*len = chan_index_24;
		for (i = 0; ((i < chan_index_24) && (i < weight_len)); i++)
			pcl_weights[i] = WEIGHT_OF_GROUP1_PCL_CHANNELS;
		if (policy_mgr_is_hw_sbs_capable(psoc)) {
			policy_mgr_get_sbs_channels(
			sbs_channel_list, &sbs_num_channels, pcl_weights,
			weight_len, &i, POLICY_MGR_PCL_GROUP_ID2_ID3,
			channel_list_5, chan_index_5, false);
			qdf_mem_copy(&pcl_channels[num_channels],
			sbs_channel_list, sbs_num_channels);
			*len += sbs_num_channels;
		}
		policy_mgr_get_connection_channels(psoc,
			channel_list, &num_channels, POLICY_MGR_PCL_ORDER_NONE,
			skip_dfs_channel, pcl_weights, weight_len, &i,
			POLICY_MGR_PCL_GROUP_ID2_ID3);
		qdf_mem_copy(&pcl_channels[chan_index_24],
			channel_list, num_channels);
		*len += num_channels;
		status = QDF_STATUS_SUCCESS;
		break;
	case PM_SBS_CH_5G:
		if (policy_mgr_is_hw_sbs_capable(psoc)) {
			policy_mgr_get_sbs_channels(
			sbs_channel_list, &sbs_num_channels, pcl_weights,
			weight_len, &i, POLICY_MGR_PCL_GROUP_ID1_ID2,
			channel_list_5, chan_index_5, true);
			qdf_mem_copy(&pcl_channels[num_channels],
			sbs_channel_list, sbs_num_channels);
			*len += sbs_num_channels;
		} else {
			qdf_mem_copy(pcl_channels, channel_list_5,
			chan_index_5);
			*len = chan_index_5;
			for (i = 0; ((i < *len) && (i < weight_len)); i++)
				pcl_weights[i] = WEIGHT_OF_GROUP1_PCL_CHANNELS;
		}
		status = QDF_STATUS_SUCCESS;
		break;
	default:
		policy_mgr_err("unknown pcl value %d", pcl);
		break;
	}

	if ((*len != 0) && (*len != i))
		policy_mgr_debug("pcl len (%d) and weight list len mismatch (%d)",
			*len, i);

	/* check the channel avoidance list */
	policy_mgr_update_with_safe_channel_list(psoc, pcl_channels, len,
				pcl_weights, weight_len);

	policy_mgr_set_weight_of_dfs_passive_channels_to_zero(psoc,
			pcl_channels, len, pcl_weights, weight_len);
	return status;
}

/**
 * policy_mgr_disallow_mcc() - Check for mcc
 *
 * @channel: channel on which new connection is coming up
 *
 * When a new connection is about to come up check if current
 * concurrency combination including the new connection is
 * causing MCC
 *
 * Return: True/False
 */
bool policy_mgr_disallow_mcc(struct wlan_objmgr_psoc *psoc,
		uint8_t channel)
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
			if (pm_conc_connection_list[index].chan !=
				channel) {
				match = true;
				break;
			}
		} else if (WLAN_REG_IS_5GHZ_CH
			(pm_conc_connection_list[index].chan)) {
			if (pm_conc_connection_list[index].chan != channel) {
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
 * policy_mgr_allow_new_home_channel() - Check for allowed number of
 * home channels
 * @channel: channel on which new connection is coming up
 * @num_connections: number of current connections
 *
 * When a new connection is about to come up check if current
 * concurrency combination including the new connection is
 * allowed or not based on the HW capability
 *
 * Return: True/False
 */
bool policy_mgr_allow_new_home_channel(struct wlan_objmgr_psoc *psoc,
			uint8_t channel, uint32_t num_connections)
{
	bool status = true;
	struct policy_mgr_psoc_priv_obj *pm_ctx;
	uint32_t mcc_to_scc_switch;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}
	mcc_to_scc_switch =
		policy_mgr_mcc_to_scc_switch_mode_in_user_cfg(psoc);

	qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
	if (num_connections == 2) {
	/* No SCC or MCC combination is allowed with / on DFS channel */
		if ((mcc_to_scc_switch ==
		QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION)
		&& wlan_reg_is_dfs_ch(pm_ctx->pdev, channel) &&
		(wlan_reg_is_dfs_ch(pm_ctx->pdev,
			pm_conc_connection_list[0].chan) ||
		wlan_reg_is_dfs_ch(pm_ctx->pdev,
			pm_conc_connection_list[1].chan))) {

			policy_mgr_err("Existing DFS connection, new 3-port DFS connection is not allowed");
			status = false;

		} else if (((pm_conc_connection_list[0].chan !=
				pm_conc_connection_list[1].chan)
		|| (mcc_to_scc_switch ==
		QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION)
		) && (pm_conc_connection_list[0].mac ==
			pm_conc_connection_list[1].mac)) {
			if (policy_mgr_is_hw_dbs_capable(psoc) == false) {
				if ((channel !=
				     pm_conc_connection_list[0].chan) &&
				    (channel !=
				     pm_conc_connection_list[1].chan)) {
					policy_mgr_err("don't allow 3rd home channel on same MAC");
					status = false;
				}
			} else if (((WLAN_REG_IS_24GHZ_CH(channel)) &&
				(WLAN_REG_IS_24GHZ_CH
				(pm_conc_connection_list[0].chan)) &&
				(WLAN_REG_IS_24GHZ_CH
				(pm_conc_connection_list[1].chan))) ||
				   ((WLAN_REG_IS_5GHZ_CH(channel)) &&
				(WLAN_REG_IS_5GHZ_CH
				(pm_conc_connection_list[0].chan)) &&
				(WLAN_REG_IS_5GHZ_CH
				(pm_conc_connection_list[1].chan)))) {
					policy_mgr_err("don't allow 3rd home channel on same MAC");
					status = false;
			}
		}
	} else if ((num_connections == 1)
		&& (mcc_to_scc_switch ==
		QDF_MCC_TO_SCC_SWITCH_FORCE_PREFERRED_WITHOUT_DISCONNECTION)
		&& wlan_reg_is_dfs_ch(pm_ctx->pdev, channel)
		&& wlan_reg_is_dfs_ch(pm_ctx->pdev,
			pm_conc_connection_list[0].chan)) {

		policy_mgr_err("Existing DFS connection, new 2-port DFS connection is not allowed");
		status = false;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return status;
}

/**
 * policy_mgr_is_5g_channel_allowed() - check if 5g channel is allowed
 * @channel: channel number which needs to be validated
 * @list: list of existing connections.
 * @mode: mode against which channel needs to be validated
 *
 * This API takes the channel as input and compares with existing
 * connection channels. If existing connection's channel is DFS channel
 * and provided channel is 5G channel then don't allow concurrency to
 * happen as MCC with DFS channel is not yet supported
 *
 * Return: true if 5G channel is allowed, false if not allowed
 *
 */
bool policy_mgr_is_5g_channel_allowed(struct wlan_objmgr_psoc *psoc,
				uint8_t channel, uint32_t *list,
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
		if (wlan_reg_is_dfs_ch(
			pm_ctx->pdev,
			pm_conc_connection_list[list[index]].chan) &&
		    WLAN_REG_IS_5GHZ_CH(channel) &&
		    (channel != pm_conc_connection_list[list[index]].chan)) {
			qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
			policy_mgr_err("don't allow MCC if SAP/GO on DFS channel");
			return false;
		}
		index++;
	}
	qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);

	return true;
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
		enum policy_mgr_conn_update_reason reason)
{
	uint32_t conn_index = 0;
	QDF_STATUS ret;

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

	policy_mgr_debug("nss update successful for vdev:%d", vdev_id);
	if (PM_NOP != next_action)
		policy_mgr_next_actions(psoc, vdev_id, next_action, reason);
	else {
		policy_mgr_debug("No action needed right now");
		ret = policy_mgr_set_opportunistic_update(psoc);
		if (!QDF_IS_STATUS_SUCCESS(ret))
			policy_mgr_err("ERROR: set opportunistic_update event failed");
	}

	return;
}

QDF_STATUS policy_mgr_nss_update(struct wlan_objmgr_psoc *psoc,
		uint8_t  new_nss, uint8_t next_action,
		enum policy_mgr_conn_update_reason reason)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t index, count;
	uint32_t list[MAX_NUMBER_OF_CONC_CONNECTIONS];
	uint32_t conn_index = 0;
	uint32_t vdev_id;
	uint32_t original_nss;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return status;
	}

	count = policy_mgr_mode_specific_connection_count(psoc,
			PM_P2P_GO_MODE, list);
	for (index = 0; index < count; index++) {
		qdf_mutex_acquire(&pm_ctx->qdf_conc_list_lock);
		vdev_id = pm_conc_connection_list[list[index]].vdev_id;
		original_nss =
		pm_conc_connection_list[list[index]].original_nss;
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		conn_index = policy_mgr_get_connection_for_vdev_id(
			psoc, vdev_id);
		if (MAX_NUMBER_OF_CONC_CONNECTIONS == conn_index) {
			policy_mgr_err("connection not found for vdev %d",
				vdev_id);
			continue;
		}

		if (2 == original_nss) {
			status = pm_ctx->sme_cbacks.sme_nss_update_request(
					vdev_id, new_nss,
					policy_mgr_nss_update_cb,
					next_action, psoc, reason);
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
		qdf_mutex_release(&pm_ctx->qdf_conc_list_lock);
		conn_index = policy_mgr_get_connection_for_vdev_id(
			psoc, vdev_id);
		if (MAX_NUMBER_OF_CONC_CONNECTIONS == conn_index) {
			policy_mgr_err("connection not found for vdev %d",
				vdev_id);
			continue;
		}
		if (2 == original_nss) {
			status = pm_ctx->sme_cbacks.sme_nss_update_request(
					vdev_id, new_nss,
					policy_mgr_nss_update_cb,
					next_action, psoc, reason);
			if (!QDF_IS_STATUS_SUCCESS(status)) {
				policy_mgr_err("sme_nss_update_request() failed for vdev %d",
				vdev_id);
			}
		}
	}

	return status;
}

/**
 * policy_mgr_complete_action() - initiates actions needed on
 * current connections once channel has been decided for the new
 * connection
 * @new_nss: the new nss value
 * @next_action: next action to happen at policy mgr after
 *		beacon update
 * @reason: Reason for connection update
 * @session_id: Session id
 *
 * This function initiates initiates actions
 * needed on current connections once channel has been decided
 * for the new connection. Notifies UMAC & FW as well
 *
 * Return: QDF_STATUS enum
 */
QDF_STATUS policy_mgr_complete_action(struct wlan_objmgr_psoc *psoc,
				uint8_t  new_nss, uint8_t next_action,
				enum policy_mgr_conn_update_reason reason,
				uint32_t session_id)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (policy_mgr_is_hw_dbs_capable(psoc) == false) {
		policy_mgr_err("driver isn't dbs capable, no further action needed");
		return QDF_STATUS_E_NOSUPPORT;
	}

	/* policy_mgr_complete_action() is called by policy_mgr_next_actions().
	 * All other callers of policy_mgr_next_actions() have taken mutex
	 * protection. So, not taking any lock inside
	 * policy_mgr_complete_action() during pm_conc_connection_list access.
	 */
	status = policy_mgr_nss_update(psoc, new_nss, next_action, reason);
	if (!QDF_IS_STATUS_SUCCESS(status))
		status = policy_mgr_next_actions(psoc, session_id,
						next_action, reason);

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
		pm_conc_connection_list[0].chan,
		pm_conc_connection_list[1].chan,
		pm_conc_connection_list[2].chan, num_connections,
		hw_mode.dbs_cap);

	/* If the band of operation of both the MACs is the same,
	 * single MAC is preferred, otherwise DBS is preferred.
	 */
	switch (num_connections) {
	case 1:
		band1 = reg_chan_to_band(pm_conc_connection_list[0].chan);
		if (band1 == BAND_2G)
			return PM_DBS;
		else
			return PM_NOP;
	case 2:
		band1 = reg_chan_to_band(pm_conc_connection_list[0].chan);
		band2 = reg_chan_to_band(pm_conc_connection_list[1].chan);
		if ((band1 == BAND_2G) ||
			(band2 == BAND_2G)) {
			if (!hw_mode.dbs_cap)
				return PM_DBS;
			else
				return PM_NOP;
		} else if ((band1 == BAND_5G) &&
				(band2 == BAND_5G)) {
			if (WLAN_REG_IS_CHANNEL_VALID_5G_SBS(
				pm_conc_connection_list[0].chan,
				pm_conc_connection_list[1].chan)) {
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
		band1 = reg_chan_to_band(pm_conc_connection_list[0].chan);
		band2 = reg_chan_to_band(pm_conc_connection_list[1].chan);
		band3 = reg_chan_to_band(pm_conc_connection_list[2].chan);
		if ((band1 == BAND_2G) ||
			(band2 == BAND_2G) ||
			(band3 == BAND_2G)) {
			if (!hw_mode.dbs_cap)
				return PM_DBS;
			else
				return PM_NOP;
		} else if ((band1 == BAND_5G) &&
				(band2 == BAND_5G) &&
					(band3 == BAND_5G)) {
			if (WLAN_REG_IS_CHANNEL_VALID_5G_SBS(
				pm_conc_connection_list[0].chan,
				pm_conc_connection_list[2].chan) &&
				WLAN_REG_IS_CHANNEL_VALID_5G_SBS(
				pm_conc_connection_list[1].chan,
				pm_conc_connection_list[2].chan) &&
				WLAN_REG_IS_CHANNEL_VALID_5G_SBS(
				pm_conc_connection_list[0].chan,
				pm_conc_connection_list[1].chan)) {
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
		pm_conc_connection_list[0].chan,
		pm_conc_connection_list[1].chan,
		pm_conc_connection_list[2].chan, num_connections,
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
		band1 = reg_chan_to_band(pm_conc_connection_list[0].chan);
		band2 = reg_chan_to_band(pm_conc_connection_list[1].chan);
		if ((band1 == band2) && (hw_mode.dbs_cap))
			next_action = PM_SINGLE_MAC_UPGRADE;
		else if ((band1 != band2) && (!hw_mode.dbs_cap))
			next_action = PM_DBS_DOWNGRADE;
		else
			next_action = PM_NOP;

		break;

	case 3:
		band1 = reg_chan_to_band(pm_conc_connection_list[0].chan);
		band2 = reg_chan_to_band(pm_conc_connection_list[1].chan);
		band3 = reg_chan_to_band(pm_conc_connection_list[2].chan);
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
		QDF_ARRAY_SIZE(pm_ctx->sap_mandatory_channels));

	return QDF_STATUS_SUCCESS;
}

void policy_mgr_enable_disable_sap_mandatory_chan_list(
		struct wlan_objmgr_psoc *psoc, bool val)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	pm_ctx->enable_sap_mandatory_chan_list = val;
}

void policy_mgr_add_sap_mandatory_chan(struct wlan_objmgr_psoc *psoc,
		uint8_t chan)
{
	int i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	for (i = 0; i < pm_ctx->sap_mandatory_channels_len; i++) {
		if (chan == pm_ctx->sap_mandatory_channels[i])
			return;
	}

	policy_mgr_debug("chan %hu", chan);
	pm_ctx->sap_mandatory_channels[pm_ctx->sap_mandatory_channels_len++]
		= chan;
}

bool policy_mgr_is_sap_mandatory_chan_list_enabled(
		struct wlan_objmgr_psoc *psoc)
{
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	return pm_ctx->enable_sap_mandatory_chan_list;
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

void  policy_mgr_init_sap_mandatory_2g_chan(struct wlan_objmgr_psoc *psoc)
{
	uint8_t chan_list[QDF_MAX_NUM_CHAN] = {0};
	uint32_t len = 0;
	int i;
	QDF_STATUS status;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	status = policy_mgr_get_valid_chans(psoc, chan_list, &len);
	if (QDF_IS_STATUS_ERROR(status)) {
		policy_mgr_err("Error in getting valid channels");
		return;
	}
	for (i = 0; i < len; i++) {
		if (WLAN_REG_IS_24GHZ_CH(chan_list[i])) {
			policy_mgr_debug("Add chan %hu to mandatory list",
					chan_list[i]);
			pm_ctx->sap_mandatory_channels[
				pm_ctx->sap_mandatory_channels_len++] =
				chan_list[i];
		}
	}
}

void policy_mgr_remove_sap_mandatory_chan(struct wlan_objmgr_psoc *psoc,
		uint8_t chan)
{
	uint8_t chan_list[QDF_MAX_NUM_CHAN] = {0};
	uint32_t num_chan = 0;
	int i;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return;
	}

	if (pm_ctx->sap_mandatory_channels_len >= QDF_MAX_NUM_CHAN) {
		policy_mgr_err("Invalid channel len %d ",
				pm_ctx->sap_mandatory_channels_len);
		return;
	}

	for (i = 0; i < pm_ctx->sap_mandatory_channels_len; i++) {
		if (chan == pm_ctx->sap_mandatory_channels[i])
			continue;
		chan_list[num_chan++] = pm_ctx->sap_mandatory_channels[i];
	}

	qdf_mem_zero(pm_ctx->sap_mandatory_channels,
			pm_ctx->sap_mandatory_channels_len);
	qdf_mem_copy(pm_ctx->sap_mandatory_channels, chan_list, num_chan);
	pm_ctx->sap_mandatory_channels_len = num_chan;
}
