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
	return false;
}

bool policy_mgr_get_single_mac_scan_with_dfs_config(
		struct wlan_objmgr_psoc *psoc)
{
	return false;
}

int8_t policy_mgr_get_num_dbs_hw_modes(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

bool policy_mgr_is_hw_dbs_capable(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

bool policy_mgr_is_hw_sbs_capable(struct wlan_objmgr_psoc *psoc)
{
	return true;
}

QDF_STATUS policy_mgr_get_dbs_hw_modes(struct wlan_objmgr_psoc *psoc,
		bool *one_by_one_dbs, bool *two_by_two_dbs)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_get_current_hw_mode(struct wlan_objmgr_psoc *psoc,
		struct policy_mgr_hw_mode_params *hw_mode)
{
	return QDF_STATUS_SUCCESS;
}

bool policy_mgr_is_current_hwmode_dbs(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

bool policy_mgr_is_dbs_enable(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

bool policy_mgr_is_hw_dbs_2x2_capable(struct wlan_objmgr_psoc *psoc)
{
	return false;
}

uint32_t policy_mgr_get_connection_count(struct wlan_objmgr_psoc *psoc)
{
	return 0;
}

uint32_t policy_mgr_mode_specific_connection_count(
		struct wlan_objmgr_psoc *psoc,
		enum policy_mgr_con_mode mode,
		uint32_t *list)
{
	return 0;
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
}

void policy_mgr_set_dual_mac_fw_mode_config(struct wlan_objmgr_psoc *psoc,
			uint8_t dbs, uint8_t dfs)
{
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
