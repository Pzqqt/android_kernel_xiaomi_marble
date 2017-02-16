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
 * DOC: wlan_policy_mgr_action.c
 *
 * WLAN Concurrenct Connection Management APIs
 *
 */

/* Include files */

#include "wlan_policy_mgr_api.h"
#include "wlan_policy_mgr_tables_no_dbs_i.h"
#include "wlan_policy_mgr_i.h"
#include "qdf_types.h"
#include "qdf_trace.h"
#include "wlan_objmgr_global_obj.h"

enum policy_mgr_conc_next_action (*policy_mgr_get_current_pref_hw_mode_ptr)
	(struct wlan_objmgr_psoc *psoc);

void policy_mgr_hw_mode_transition_cb(uint32_t old_hw_mode_index,
			uint32_t new_hw_mode_index,
			uint32_t num_vdev_mac_entries,
			struct policy_mgr_vdev_mac_map *vdev_mac_map,
			struct wlan_objmgr_psoc *context)
{
}

QDF_STATUS policy_mgr_pdev_set_hw_mode(struct wlan_objmgr_psoc *psoc,
		uint32_t session_id,
		enum hw_mode_ss_config mac0_ss,
		enum hw_mode_bandwidth mac0_bw,
		enum hw_mode_ss_config mac1_ss,
		enum hw_mode_bandwidth mac1_bw,
		enum hw_mode_dbs_capab dbs,
		enum hw_mode_agile_dfs_capab dfs,
		enum hw_mode_sbs_capab sbs,
		enum policy_mgr_conn_update_reason reason)
{
	return QDF_STATUS_SUCCESS;
}

enum policy_mgr_conc_next_action policy_mgr_need_opportunistic_upgrade(
		struct wlan_objmgr_psoc *psoc)
{
	return PM_NOP;
}

QDF_STATUS policy_mgr_update_connection_info(struct wlan_objmgr_psoc *psoc,
					uint32_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_update_and_wait_for_connection_update(
		struct wlan_objmgr_psoc *psoc,
		uint8_t session_id,
		uint8_t channel,
		enum policy_mgr_conn_update_reason reason)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_current_connections_update(struct wlan_objmgr_psoc *psoc,
		uint32_t session_id,
		uint8_t channel,
		enum policy_mgr_conn_update_reason reason)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_next_actions(struct wlan_objmgr_psoc *psoc,
		uint32_t session_id,
		enum policy_mgr_conc_next_action action,
		enum policy_mgr_conn_update_reason reason)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_handle_conc_multiport(struct wlan_objmgr_psoc *psoc,
		uint8_t session_id, uint8_t channel)
{
	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
void policy_mgr_check_sta_ap_concurrent_ch_intf(void *data)
{
	return;
}

void policy_mgr_check_concurrent_intf_and_restart_sap(
		struct wlan_objmgr_psoc *psoc)
{
	return;
}
#endif /* FEATURE_WLAN_MCC_TO_SCC_SWITCH */

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
void policy_mgr_change_sap_channel_with_csa(struct wlan_objmgr_psoc *psoc)
{
	return;
}
#endif

QDF_STATUS policy_mgr_wait_for_connection_update(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_reset_connection_update(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_set_connection_update(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_restart_opportunistic_timer(
		struct wlan_objmgr_psoc *psoc, bool check_state)
{
	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
QDF_STATUS policy_mgr_register_sap_restart_channel_switch_cb(
		struct wlan_objmgr_psoc *psoc,
		void (*sap_restart_chan_switch_cb)(void *, uint32_t, uint32_t))
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_deregister_sap_restart_channel_switch_cb(
		struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

#endif

QDF_STATUS policy_mgr_set_hw_mode_on_channel_switch(
			struct wlan_objmgr_psoc *psoc, uint8_t session_id)
{
	return QDF_STATUS_SUCCESS;
}

void policy_mgr_checkn_update_hw_mode_single_mac_mode(
		struct wlan_objmgr_psoc *psoc, uint8_t channel)
{
}

#ifdef MPC_UT_FRAMEWORK
QDF_STATUS policy_mgr_update_connection_info_utfw(
		struct wlan_objmgr_psoc *psoc,
		uint32_t vdev_id, uint32_t tx_streams, uint32_t rx_streams,
		uint32_t chain_mask, uint32_t type, uint32_t sub_type,
		uint32_t channelid, uint32_t mac_id)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_incr_connection_count_utfw(struct wlan_objmgr_psoc *psoc,
		uint32_t vdev_id, uint32_t tx_streams, uint32_t rx_streams,
		uint32_t chain_mask, uint32_t type, uint32_t sub_type,
		uint32_t channelid, uint32_t mac_id)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS policy_mgr_decr_connection_count_utfw(struct wlan_objmgr_psoc *psoc,
		uint32_t del_all, uint32_t vdev_id)
{
	return QDF_STATUS_SUCCESS;
}

enum policy_mgr_pcl_type policy_mgr_get_pcl_from_first_conn_table(
		enum policy_mgr_con_mode type,
		enum policy_mgr_conc_priority_mode sys_pref)
{
	return PM_NONE;
}

enum policy_mgr_pcl_type policy_mgr_get_pcl_from_second_conn_table(
	enum policy_mgr_one_connection_mode idx, enum policy_mgr_con_mode type,
	enum policy_mgr_conc_priority_mode sys_pref, uint8_t dbs_capable)
{
	return PM_NONE;
}

enum policy_mgr_pcl_type policy_mgr_get_pcl_from_third_conn_table(
	enum policy_mgr_two_connection_mode idx, enum policy_mgr_con_mode type,
	enum policy_mgr_conc_priority_mode sys_pref, uint8_t dbs_capable)
{
	return PM_NONE;
}
#endif
