/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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

#include "wlan_policy_mgr_api.h"
#include "wlan_policy_mgr_i.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "wlan_objmgr_global_obj.h"

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

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	if (!policy_mgr_is_dbs_enable(psoc)) {
		policy_mgr_notice("DBS is disabled");
		return false;
	}

	policy_mgr_notice("DBS service bit map: %d",
		pm_ctx->wma_cbacks.wma_is_service_enabled(
		WMI_SERVICE_DUAL_BAND_SIMULTANEOUS_SUPPORT));

	/* The agreement with FW is that: To know if the target is DBS
	 * capable, DBS needs to be supported both in the HW mode list
	 * and in the service ready event
	 */
	if (!(pm_ctx->wma_cbacks.wma_is_service_enabled(
			WMI_SERVICE_DUAL_BAND_SIMULTANEOUS_SUPPORT)))
		return false;

	for (i = 0; i < pm_ctx->num_dbs_hw_modes; i++) {
		param = pm_ctx->hw_mode.hw_mode_list[i];
		policy_mgr_notice("HW param: %x", param);
		if (POLICY_MGR_HW_MODE_DBS_MODE_GET(param)) {
			policy_mgr_notice("HW (%d) is DBS capable", i);
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

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	policy_mgr_notice("SBS service bit map: %d",
		pm_ctx->wma_cbacks.wma_is_service_enabled(
		WMI_SERVICE_DUAL_BAND_SIMULTANEOUS_SUPPORT));

	/* The agreement with FW is that: To know if the target is SBS
	 * capable, SBS needs to be supported both in the HW mode list
	 * and DBS needs to be supported in the service ready event
	 */
	if (!(pm_ctx->wma_cbacks.wma_is_service_enabled(
			WMI_SERVICE_DUAL_BAND_SIMULTANEOUS_SUPPORT)))
		return false;

	for (i = 0; i < pm_ctx->num_dbs_hw_modes; i++) {
		param = pm_ctx->hw_mode.hw_mode_list[i];
		policy_mgr_notice("HW param: %x", param);
		if (POLICY_MGR_HW_MODE_SBS_MODE_GET(param)) {
			policy_mgr_notice("HW (%d) is SBS capable", i);
			found = 1;
			break;
		}
	}

	if (found)
		return true;

	return true;
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
			policy_mgr_notice("1x1 hw_mode index %d found", i);
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
			policy_mgr_notice("2x2 hw_mode index %d found", i);
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

	policy_mgr_notice("Get the current hw mode");

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

	if (policy_mgr_is_dual_mac_disabled_in_ini(psoc))
		return false;

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
	int i, param;
	uint32_t dbs, tx_chain0, rx_chain0, tx_chain1, rx_chain1;
	uint32_t final_min_rf_chains = 2;
	uint32_t min_rf_chains, min_mac0_rf_chains, min_mac1_rf_chains;
	bool is_dbs_found = false;
	struct policy_mgr_psoc_priv_obj *pm_ctx;

	pm_ctx = policy_mgr_get_context(psoc);
	if (!pm_ctx) {
		policy_mgr_err("Invalid Context");
		return false;
	}

	for (i = 0; i < pm_ctx->num_dbs_hw_modes; i++) {
		param = pm_ctx->hw_mode.hw_mode_list[i];
		tx_chain0 = POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_GET(param);
		rx_chain0 = POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_GET(param);
		dbs = POLICY_MGR_HW_MODE_DBS_MODE_GET(param);
		tx_chain1 = POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_GET(param);
		rx_chain1 = POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_GET(param);

		if (dbs) {
			min_mac0_rf_chains = QDF_MIN(tx_chain0, rx_chain0);
			min_mac1_rf_chains = QDF_MIN(tx_chain1, rx_chain1);
			min_rf_chains = QDF_MIN(min_mac0_rf_chains,
						min_mac1_rf_chains);
			is_dbs_found = true;
		} else {
			continue;
		}
		final_min_rf_chains = QDF_MIN(final_min_rf_chains,
					min_rf_chains);
	}
	if (false == is_dbs_found)
		final_min_rf_chains = 0;

	return (final_min_rf_chains == 2) ? true : false;
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

void policy_mgr_soc_set_dual_mac_cfg_cb(enum set_hw_mode_status status,
		uint32_t scan_config,
		uint32_t fw_mode_config)
{
	policy_mgr_notice("Status:%d for scan_config:%x fw_mode_config:%x",
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

	policy_mgr_notice("scan_config:%x fw_mode_config:%x",
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

	policy_mgr_notice("scan_config:%x fw_mode_config:%x",
			cfg.scan_config, cfg.fw_mode_config);

	status = pm_ctx->sme_cbacks.sme_soc_set_dual_mac_config(cfg);
	if (status != QDF_STATUS_SUCCESS)
		policy_mgr_err("sme_soc_set_dual_mac_config failed %d", status);
}

bool policy_mgr_current_concurrency_is_mcc(struct wlan_objmgr_psoc *psoc)
{
	return true;
}

void policy_mgr_set_concurrency_mode(enum tQDF_ADAPTER_MODE mode)
{
	return;
}

void policy_mgr_clear_concurrency_mode(enum tQDF_ADAPTER_MODE mode)
{
	return;
}

void policy_mgr_incr_active_session(struct wlan_objmgr_psoc *psoc,
				enum tQDF_ADAPTER_MODE mode,
				uint8_t session_id)
{
}

void policy_mgr_decr_active_session(struct wlan_objmgr_psoc *psoc,
				enum tQDF_ADAPTER_MODE mode,
				uint8_t session_id)
{
}

QDF_STATUS policy_mgr_incr_connection_count(
		struct wlan_objmgr_psoc *psoc, uint32_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_decr_connection_count(struct wlan_objmgr_psoc *psoc,
					uint32_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

bool policy_mgr_map_concurrency_mode(enum tQDF_ADAPTER_MODE *old_mode,
		enum policy_mgr_con_mode *new_mode)
{
	return true;
}

bool policy_mgr_is_ibss_conn_exist(struct wlan_objmgr_psoc *psoc,
				uint8_t *ibss_channel)
{
	return QDF_STATUS_SUCCESS;
}

bool policy_mgr_max_concurrent_connections_reached(
		struct wlan_objmgr_psoc *psoc)
{
	return false;
}

bool policy_mgr_allow_concurrency(struct wlan_objmgr_psoc *psoc,
				enum policy_mgr_con_mode mode,
				uint8_t channel, enum hw_mode_bandwidth bw)
{
	return true;
}

uint32_t policy_mgr_get_concurrency_mode(void)
{
	return 0;
}

QDF_STATUS policy_mgr_get_channel_from_scan_result(
		struct wlan_objmgr_psoc *psoc,
		void *roam_profile, uint8_t *channel)
{
	return QDF_STATUS_SUCCESS;
}

uint8_t policy_mgr_search_and_check_for_session_conc(
		struct wlan_objmgr_psoc *psoc,
		uint8_t session_id,
		void *roam_profile)
{
	return 0;
}

bool policy_mgr_check_for_session_conc(struct wlan_objmgr_psoc *psoc,
				uint8_t session_id, uint8_t channel)
{
	return true;
}

uint8_t policy_mgr_is_mcc_in_24G(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

int32_t policy_mgr_set_mas(struct wlan_objmgr_psoc *psoc, uint8_t mas_value)
{
	return 0;
}

int32_t policy_mgr_set_mcc_p2p_quota(struct wlan_objmgr_psoc *psoc,
		uint32_t set_value)
{
	return 0;
}

QDF_STATUS policy_mgr_change_mcc_go_beacon_interval(
		struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

int32_t policy_mgr_go_set_mcc_p2p_quota(struct wlan_objmgr_psoc *psoc,
		uint32_t set_value)
{
	return 0;
}

void policy_mgr_set_mcc_latency(struct wlan_objmgr_psoc *psoc, int set_value)
{
	return;
}

struct policy_mgr_conc_connection_info *policy_mgr_get_conn_info(uint32_t *len)
{
	return NULL;
}

enum policy_mgr_con_mode policy_mgr_convert_device_mode_to_qdf_type(
			enum tQDF_ADAPTER_MODE device_mode)
{
	return PM_MAX_NUM_OF_MODE;
}

bool policy_mgr_concurrent_open_sessions_running(void)
{
	return true;
}

bool policy_mgr_concurrent_beaconing_sessions_running(void)
{
	return true;
}


void policy_mgr_clear_concurrent_session_count(struct wlan_objmgr_psoc *psoc)
{
}

bool policy_mgr_is_multiple_active_sta_sessions(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

bool policy_mgr_is_sta_active_connection_exists(void)
{
	return true;
}

bool policy_mgr_is_any_nondfs_chnl_present(struct wlan_objmgr_psoc *psoc,
				uint8_t *channel)
{
	return true;
}

bool policy_mgr_is_any_dfs_beaconing_session_present(
		struct wlan_objmgr_psoc *psoc, uint8_t *channel)
{
	return true;
}

QDF_STATUS policy_mgr_get_nss_for_vdev(struct wlan_objmgr_psoc *psoc,
				enum policy_mgr_con_mode mode,
				uint8_t *nss_2g, uint8_t *nss_5g)
{
	return QDF_STATUS_SUCCESS;
}

void policy_mgr_dump_connection_status_info(struct wlan_objmgr_psoc *psoc)
{
}

bool policy_mgr_is_any_mode_active_on_band_along_with_session(
						struct wlan_objmgr_psoc *psoc,
						uint8_t session_id,
						enum policy_mgr_band band)
{
	return true;
}

QDF_STATUS policy_mgr_get_mac_id_by_session_id(struct wlan_objmgr_psoc *psoc,
					uint8_t session_id, uint8_t *mac_id)
{
	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS policy_mgr_get_mcc_session_id_on_mac(struct wlan_objmgr_psoc *psoc,
					uint8_t mac_id, uint8_t session_id,
					uint8_t *mcc_session_id)
{
	return QDF_STATUS_E_FAILURE;
}

uint8_t policy_mgr_get_mcc_operating_channel(struct wlan_objmgr_psoc *psoc,
					uint8_t session_id)
{
	return 0;
}

void policy_mgr_set_do_hw_mode_change_flag(struct wlan_objmgr_psoc *psoc,
		bool flag)
{
}

bool policy_mgr_is_hw_mode_change_after_vdev_up(struct wlan_objmgr_psoc *psoc)
{
	return false;
}
