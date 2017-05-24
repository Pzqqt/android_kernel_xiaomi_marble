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

#ifndef WLAN_POLICY_MGR_I_H
#define WLAN_POLICY_MGR_I_H

#include "wlan_policy_mgr_api.h"
#include "qdf_event.h"
#include "qdf_mc_timer.h"
#include "qdf_lock.h"
#include "qdf_defer.h"
#include "wlan_reg_services_api.h"

#define MAX_NUMBER_OF_CONC_CONNECTIONS 3
#define DBS_OPPORTUNISTIC_TIME    10
#ifdef QCA_WIFI_3_0_EMU
#define CONNECTION_UPDATE_TIMEOUT 3000
#else
#define CONNECTION_UPDATE_TIMEOUT 1000
#endif

/**
 * Policy Mgr hardware mode list bit-mask definitions.
 * Bits 4:0, 31:29 are unused.
 *
 * The below definitions are added corresponding to WMI DBS HW mode
 * list to make it independent of firmware changes for WMI definitions.
 * Currently these definitions have dependency with BIT positions of
 * the existing WMI macros. Thus, if the BIT positions are changed for
 * WMI macros, then these macros' BIT definitions are also need to be
 * changed.
 */
#define POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_BITPOS  (28)
#define POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_BITPOS  (24)
#define POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_BITPOS  (20)
#define POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_BITPOS  (16)
#define POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_BITPOS   (12)
#define POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_BITPOS   (8)
#define POLICY_MGR_HW_MODE_DBS_MODE_BITPOS         (7)
#define POLICY_MGR_HW_MODE_AGILE_DFS_MODE_BITPOS   (6)
#define POLICY_MGR_HW_MODE_SBS_MODE_BITPOS         (5)

#define POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_MASK    \
	(0xf << POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_BITPOS)
#define POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_MASK    \
	(0xf << POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_BITPOS)
#define POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_MASK    \
	(0xf << POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_BITPOS)
#define POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_MASK    \
	(0xf << POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_BITPOS)
#define POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_MASK     \
	(0xf << POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_BITPOS)
#define POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_MASK     \
	(0xf << POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_BITPOS)
#define POLICY_MGR_HW_MODE_DBS_MODE_MASK           \
	(0x1 << POLICY_MGR_HW_MODE_DBS_MODE_BITPOS)
#define POLICY_MGR_HW_MODE_AGILE_DFS_MODE_MASK     \
	(0x1 << POLICY_MGR_HW_MODE_AGILE_DFS_MODE_BITPOS)
#define POLICY_MGR_HW_MODE_SBS_MODE_MASK           \
	(0x1 << POLICY_MGR_HW_MODE_SBS_MODE_BITPOS)

#define POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_SET(hw_mode, value) \
	WMI_SET_BITS(hw_mode, POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_BITPOS,\
	4, value)
#define POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_SET(hw_mode, value) \
	WMI_SET_BITS(hw_mode, POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_BITPOS,\
	4, value)
#define POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_SET(hw_mode, value) \
	WMI_SET_BITS(hw_mode, POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_BITPOS,\
	4, value)
#define POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_SET(hw_mode, value) \
	WMI_SET_BITS(hw_mode, POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_BITPOS,\
	4, value)
#define POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_SET(hw_mode, value)  \
	WMI_SET_BITS(hw_mode, POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_BITPOS,\
	4, value)
#define POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_SET(hw_mode, value)  \
	WMI_SET_BITS(hw_mode, POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_BITPOS,\
	4, value)
#define POLICY_MGR_HW_MODE_DBS_MODE_SET(hw_mode, value)        \
	WMI_SET_BITS(hw_mode, POLICY_MGR_HW_MODE_DBS_MODE_BITPOS,\
	1, value)
#define POLICY_MGR_HW_MODE_AGILE_DFS_SET(hw_mode, value)       \
	WMI_SET_BITS(hw_mode, POLICY_MGR_HW_MODE_AGILE_DFS_MODE_BITPOS,\
	1, value)
#define POLICY_MGR_HW_MODE_SBS_MODE_SET(hw_mode, value)        \
	WMI_SET_BITS(hw_mode, POLICY_MGR_HW_MODE_SBS_MODE_BITPOS,\
	1, value)

#define POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_GET(hw_mode)                \
		((hw_mode & POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_MASK) >>        \
		POLICY_MGR_HW_MODE_MAC0_TX_STREAMS_BITPOS)
#define POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_GET(hw_mode)                \
		((hw_mode & POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_MASK) >>        \
		POLICY_MGR_HW_MODE_MAC0_RX_STREAMS_BITPOS)
#define POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_GET(hw_mode)                \
		((hw_mode & POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_MASK) >>        \
		POLICY_MGR_HW_MODE_MAC1_TX_STREAMS_BITPOS)
#define POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_GET(hw_mode)                \
		((hw_mode & POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_MASK) >>        \
		POLICY_MGR_HW_MODE_MAC1_RX_STREAMS_BITPOS)
#define POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_GET(hw_mode)                 \
		((hw_mode & POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_MASK) >>         \
		POLICY_MGR_HW_MODE_MAC0_BANDWIDTH_BITPOS)
#define POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_GET(hw_mode)                 \
		((hw_mode & POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_MASK) >>         \
		POLICY_MGR_HW_MODE_MAC1_BANDWIDTH_BITPOS)
#define POLICY_MGR_HW_MODE_DBS_MODE_GET(hw_mode)                       \
		((hw_mode & POLICY_MGR_HW_MODE_DBS_MODE_MASK) >>               \
		POLICY_MGR_HW_MODE_DBS_MODE_BITPOS)
#define POLICY_MGR_HW_MODE_AGILE_DFS_GET(hw_mode)                      \
		((hw_mode & POLICY_MGR_HW_MODE_AGILE_DFS_MODE_MASK) >>         \
		POLICY_MGR_HW_MODE_AGILE_DFS_MODE_BITPOS)
#define POLICY_MGR_HW_MODE_SBS_MODE_GET(hw_mode)                       \
		((hw_mode & POLICY_MGR_HW_MODE_SBS_MODE_MASK) >>               \
		POLICY_MGR_HW_MODE_SBS_MODE_BITPOS)

#define POLICY_MGR_DEFAULT_HW_MODE_INDEX 0xFFFF

#define policy_mgr_log(level, args...) \
		QDF_TRACE(QDF_MODULE_ID_POLICY_MGR, level, ## args)
#define policy_mgr_logfl(level, format, args...) \
		policy_mgr_log(level, FL(format), ## args)

#define policy_mgr_alert(format, args...) \
		policy_mgr_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define policy_mgr_err(format, args...) \
		policy_mgr_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define policy_mgr_warn(format, args...) \
		policy_mgr_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define policy_mgr_notice(format, args...) \
		policy_mgr_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define policy_mgr_info(format, args...) \
		policy_mgr_logfl(QDF_TRACE_LEVEL_INFO_HIGH, format, ## args)
#define policy_mgr_debug(format, args...) \
		policy_mgr_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

#define PM_CONC_CONNECTION_LIST_VALID_INDEX(index) \
		((MAX_NUMBER_OF_CONC_CONNECTIONS > index) && \
			(pm_conc_connection_list[index].in_use))

extern struct policy_mgr_conc_connection_info
	pm_conc_connection_list[MAX_NUMBER_OF_CONC_CONNECTIONS];

extern const enum policy_mgr_pcl_type
	first_connection_pcl_table[PM_MAX_NUM_OF_MODE]
			[PM_MAX_CONC_PRIORITY_MODE];
extern pm_dbs_pcl_second_connection_table_type
		*second_connection_pcl_dbs_table;
extern pm_dbs_pcl_third_connection_table_type
		*third_connection_pcl_dbs_table;
extern policy_mgr_next_action_two_connection_table_type
		*next_action_two_connection_table;
extern policy_mgr_next_action_three_connection_table_type
		*next_action_three_connection_table;
extern enum policy_mgr_conc_next_action
	(*policy_mgr_get_current_pref_hw_mode_ptr)
	(struct wlan_objmgr_psoc *psoc);

/**
 * struct policy_mgr_psoc_priv_obj - Policy manager private data
 * @psoc: pointer to PSOC object information
 * @pdev: pointer to PDEV object information
 * @connection_update_done_evt: qdf event to synchronize
 *                            connection activities
 * @qdf_conc_list_lock: To protect connection table
 * @dbs_opportunistic_timer: Timer to drop down to Single Mac
 *                         Mode opportunistically
 * @sap_restart_chan_switch_cb: Callback for channel switch
 *                            notification for SAP
 * @sme_cbacks: callbacks to be registered by SME for
 *            interaction with Policy Manager
 * @wma_cbacks: callbacks to be registered by SME for
 * interaction with Policy Manager
 * @tdls_cbacks: callbacks to be registered by SME for
 * interaction with Policy Manager
 * @cdp_cbacks: callbacks to be registered by SME for
 * interaction with Policy Manager
 * @sap_mandatory_channels: The user preferred master list on
 *                        which SAP can be brought up. This
 *                        mandatory channel list would be as per
 *                        OEMs preference & conforming to the
 *                        regulatory/other considerations
 * @sap_mandatory_channels_len: Length of the SAP mandatory
 *                            channel list
 * @do_hw_mode_change: Flag to check if HW mode change is needed
 *                   after vdev is up. Especially used after
 *                   channel switch related vdev restart
 * @concurrency_mode: active concurrency combination
 * @no_of_open_sessions: Number of active vdevs
 * @no_of_active_sessions: Number of active connections
 * @sta_ap_intf_check_work: delayed sap restart work
 * @num_dbs_hw_modes: Number of different HW modes supported
 * @hw_mode: List of HW modes supported
 * @old_hw_mode_index: Old HW mode from hw_mode table
 * @new_hw_mode_index: New HW mode from hw_mode table
 * @dual_mac_cfg: DBS configuration currenctly used by FW for
 *              scan & connections
 * @hw_mode_change_in_progress: This is to track if HW mode
 *                            change is in progress
 * @enable_mcc_adaptive_scheduler: Enable MCC adaptive scheduler
 *      value from INI
 */
struct policy_mgr_psoc_priv_obj {
		struct wlan_objmgr_psoc *psoc;
		struct wlan_objmgr_pdev *pdev;
		qdf_event_t connection_update_done_evt;
		qdf_mutex_t qdf_conc_list_lock;
		qdf_mc_timer_t dbs_opportunistic_timer;
		struct policy_mgr_hdd_cbacks hdd_cbacks;
		struct policy_mgr_sme_cbacks sme_cbacks;
		struct policy_mgr_wma_cbacks wma_cbacks;
		struct policy_mgr_tdls_cbacks tdls_cbacks;
		struct policy_mgr_cdp_cbacks cdp_cbacks;
		uint8_t sap_mandatory_channels[QDF_MAX_NUM_CHAN];
		uint32_t sap_mandatory_channels_len;
		bool do_hw_mode_change;
		uint32_t concurrency_mode;
		uint8_t no_of_open_sessions[QDF_MAX_NO_OF_MODE];
		uint8_t no_of_active_sessions[QDF_MAX_NO_OF_MODE];
		qdf_work_t sta_ap_intf_check_work;
		uint32_t num_dbs_hw_modes;
		struct dbs_hw_mode_info hw_mode;
		uint32_t old_hw_mode_index;
		uint32_t new_hw_mode_index;
		struct dual_mac_config dual_mac_cfg;
		uint32_t hw_mode_change_in_progress;
		struct policy_mgr_user_cfg user_cfg;
};

/**
 * struct policy_mgr_mac_ss_bw_info - hw_mode_list PHY/MAC params for each MAC
 * @mac_tx_stream: Max TX stream
 * @mac_rx_stream: Max RX stream
 * @mac_bw: Max bandwidth
 */
struct policy_mgr_mac_ss_bw_info {
	uint32_t mac_tx_stream;
	uint32_t mac_rx_stream;
	uint32_t mac_bw;
};

struct policy_mgr_psoc_priv_obj *policy_mgr_get_context(
		struct wlan_objmgr_psoc *psoc);
QDF_STATUS policy_mgr_get_updated_scan_config(
		struct wlan_objmgr_psoc *psoc,
		uint32_t *scan_config,
		bool dbs_scan,
		bool dbs_plus_agile_scan,
		bool single_mac_scan_with_dfs);
QDF_STATUS policy_mgr_get_updated_fw_mode_config(
		struct wlan_objmgr_psoc *psoc,
		uint32_t *fw_mode_config,
		bool dbs,
		bool agile_dfs);
bool policy_mgr_is_dual_mac_disabled_in_ini(
		struct wlan_objmgr_psoc *psoc);

/**
 * policy_mgr_mcc_to_scc_switch_mode_in_user_cfg() - MCC to SCC
 * switch mode value in the user config
 * @psoc: PSOC object information
 *
 * MCC to SCC switch mode value in user config
 *
 * Return: MCC to SCC switch mode value
 */
uint32_t policy_mgr_mcc_to_scc_switch_mode_in_user_cfg(
	struct wlan_objmgr_psoc *psoc);
bool policy_mgr_get_dbs_config(struct wlan_objmgr_psoc *psoc);
bool policy_mgr_get_agile_dfs_config(struct wlan_objmgr_psoc *psoc);
bool policy_mgr_get_dbs_scan_config(struct wlan_objmgr_psoc *psoc);
void policy_mgr_get_tx_rx_ss_from_config(enum hw_mode_ss_config mac_ss,
		uint32_t *tx_ss, uint32_t *rx_ss);
int8_t policy_mgr_get_matching_hw_mode_index(
		struct wlan_objmgr_psoc *psoc,
		uint32_t mac0_tx_ss, uint32_t mac0_rx_ss,
		enum hw_mode_bandwidth mac0_bw,
		uint32_t mac1_tx_ss, uint32_t mac1_rx_ss,
		enum hw_mode_bandwidth mac1_bw,
		enum hw_mode_dbs_capab dbs,
		enum hw_mode_agile_dfs_capab dfs,
		enum hw_mode_sbs_capab sbs);
int8_t policy_mgr_get_hw_mode_idx_from_dbs_hw_list(
		struct wlan_objmgr_psoc *psoc,
		enum hw_mode_ss_config mac0_ss,
		enum hw_mode_bandwidth mac0_bw,
		enum hw_mode_ss_config mac1_ss,
		enum hw_mode_bandwidth mac1_bw,
		enum hw_mode_dbs_capab dbs,
		enum hw_mode_agile_dfs_capab dfs,
		enum hw_mode_sbs_capab sbs);
QDF_STATUS policy_mgr_get_hw_mode_from_idx(
		struct wlan_objmgr_psoc *psoc,
		uint32_t idx,
		struct policy_mgr_hw_mode_params *hw_mode);
QDF_STATUS policy_mgr_get_old_and_new_hw_index(
		struct wlan_objmgr_psoc *psoc,
		uint32_t *old_hw_mode_index,
		uint32_t *new_hw_mode_index);
void policy_mgr_update_conc_list(struct wlan_objmgr_psoc *psoc,
		uint32_t conn_index,
		enum policy_mgr_con_mode mode,
		uint8_t chan,
		enum hw_mode_bandwidth bw,
		uint8_t mac,
		enum policy_mgr_chain_mode chain_mask,
		uint32_t original_nss,
		uint32_t vdev_id,
		bool in_use);
void policy_mgr_store_and_del_conn_info(struct wlan_objmgr_psoc *psoc,
				enum policy_mgr_con_mode mode,
				struct policy_mgr_conc_connection_info *info);
void policy_mgr_restore_deleted_conn_info(struct wlan_objmgr_psoc *psoc,
		struct policy_mgr_conc_connection_info *info);
void policy_mgr_update_hw_mode_conn_info(struct wlan_objmgr_psoc *psoc,
				uint32_t num_vdev_mac_entries,
				struct policy_mgr_vdev_mac_map *vdev_mac_map,
				struct policy_mgr_hw_mode_params hw_mode);
void policy_mgr_pdev_set_hw_mode_cb(uint32_t status,
				uint32_t cfgd_hw_mode_index,
				uint32_t num_vdev_mac_entries,
				struct policy_mgr_vdev_mac_map *vdev_mac_map,
				void *context);
void policy_mgr_dump_current_concurrency(struct wlan_objmgr_psoc *psoc);
void policy_mgr_pdev_set_pcl(struct wlan_objmgr_psoc *psoc,
				enum tQDF_ADAPTER_MODE mode);
void policy_mgr_set_pcl_for_existing_combo(
		struct wlan_objmgr_psoc *psoc, enum policy_mgr_con_mode mode);
void pm_dbs_opportunistic_timer_handler(void *data);
enum policy_mgr_con_mode policy_mgr_get_mode(uint8_t type,
		uint8_t subtype);
enum hw_mode_bandwidth policy_mgr_get_bw(enum phy_ch_width chan_width);
QDF_STATUS policy_mgr_get_channel_list(struct wlan_objmgr_psoc *psoc,
			enum policy_mgr_pcl_type pcl,
			uint8_t *pcl_channels, uint32_t *len,
			enum policy_mgr_con_mode mode,
			uint8_t *pcl_weights, uint32_t weight_len);
bool policy_mgr_disallow_mcc(struct wlan_objmgr_psoc *psoc,
		uint8_t channel);
bool policy_mgr_allow_new_home_channel(struct wlan_objmgr_psoc *psoc,
			uint8_t channel, uint32_t num_connections);
bool policy_mgr_is_5g_channel_allowed(struct wlan_objmgr_psoc *psoc,
				uint8_t channel, uint32_t *list,
				enum policy_mgr_con_mode mode);
QDF_STATUS policy_mgr_complete_action(struct wlan_objmgr_psoc *psoc,
				uint8_t  new_nss, uint8_t next_action,
				enum policy_mgr_conn_update_reason reason,
				uint32_t session_id);
enum policy_mgr_con_mode policy_mgr_get_mode_by_vdev_id(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id);
QDF_STATUS policy_mgr_init_connection_update(
		struct policy_mgr_psoc_priv_obj *pm_ctx);
enum policy_mgr_conc_next_action
		policy_mgr_get_current_pref_hw_mode_dbs_2x2(
		struct wlan_objmgr_psoc *psoc);
enum policy_mgr_conc_next_action
		policy_mgr_get_current_pref_hw_mode_dbs_1x1(
		struct wlan_objmgr_psoc *psoc);
QDF_STATUS policy_mgr_reset_sap_mandatory_channels(
		struct policy_mgr_psoc_priv_obj *pm_ctx);

/**
 * policy_mgr_get_sap_conn_info() - Get active SAP channel and
 * vdev id
 * @psoc: PSOC object information
 * @channel: SAP channel
 * @vdev_id: SAP vdev id
 *
 * Get active SAP channel and vdev id
 *
 * Return: true for success, else false
 */
bool policy_mgr_get_sap_conn_info(struct wlan_objmgr_psoc *psoc,
				uint8_t *channel, uint8_t *vdev_id);

/**
 * policy_mgr_get_mode_specific_conn_info() - Get active mode specific
 * channel and vdev id
 * @psoc: PSOC object information
 * @channel: Mode specific channel
 * @vdev_id: Mode specific vdev id
 * @mode: Connection Mode
 *
 * Get active mode specific channel and vdev id
 *
 * Return: true for success, else false
 */
bool policy_mgr_get_mode_specific_conn_info(struct wlan_objmgr_psoc *psoc,
				  uint8_t *channel, uint8_t *vdev_id,
				  enum policy_mgr_con_mode mode);
#endif
