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
	return QDF_STATUS_SUCCESS;
}

/**
 * policy_mgr_get_updated_fw_mode_config() - Get the updated fw mode configuration
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
	return QDF_STATUS_SUCCESS;
}

/**
 * policy_mgr_is_dual_mac_disabled_in_ini() - Check if dual mac is disabled in INI
 *
 * Checks if the dual mac feature is disabled in INI
 *
 * Return: true if the dual mac feature is disabled from INI
 */
bool policy_mgr_is_dual_mac_disabled_in_ini(
		struct wlan_objmgr_psoc *psoc)
{
	return false;
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
	return false;
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
	return false;
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
	return false;
}

/**
 * policy_mgr_get_tx_rx_ss_from_config() - Get Tx/Rx spatial stream from HW mode config
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
	return 0;
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
	return 0;
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
}

/**
 * policy_mgr_store_and_del_conn_info() - Store and del a connection info
 * @mode: Mode whose entry has to be deleted
 * @info: Struture pointer where the connection info will be saved
 *
 * Saves the connection info corresponding to the provided mode
 * and deleted that corresponding entry based on vdev from the
 * connection info structure
 *
 * Return: None
 */
void policy_mgr_store_and_del_conn_info(struct wlan_objmgr_psoc *psoc,
				enum policy_mgr_con_mode mode,
				struct policy_mgr_conc_connection_info *info)
{
}

/**
 * policy_mgr_restore_deleted_conn_info() - Restore connection info
 * @info: Saved connection info that is to be restored
 *
 * Restores the connection info of STA that was saved before
 * updating the PCL to the FW
 *
 * Return: None
 */
void policy_mgr_restore_deleted_conn_info(struct wlan_objmgr_psoc *psoc,
		struct policy_mgr_conc_connection_info *info)
{
}

/**
 * policy_mgr_update_hw_mode_conn_info() - Update connection info based on HW mode
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
}

/**
 * policy_mgr_pdev_set_hw_mode_cb() - Callback for set hw mode
 * @status: Status
 * @cfgd_hw_mode_index: Configured HW mode index
 * @num_vdev_mac_entries: Number of vdev-mac id mapping that follows
 * @vdev_mac_map: vdev-mac id map. This memory will be freed by the caller.
 * So, make local copy if needed.
 *
 * Provides the status and configured hw mode index set
 * by the FW
 *
 * Return: None
 */
void policy_mgr_pdev_set_hw_mode_cb(uint32_t status,
				uint32_t cfgd_hw_mode_index,
				uint32_t num_vdev_mac_entries,
				struct policy_mgr_vdev_mac_map *vdev_mac_map,
				void *context)
{
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
				enum tQDF_ADAPTER_MODE mode)
{
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
	return PM_MAX_NUM_OF_MODE;
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
	return QDF_STATUS_SUCCESS;
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
	return false;
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
	return true;
}

/**
 * policy_mgr_vht160_conn_exist() - to check if we have a connection
 * already using vht160 or vht80+80
 *
 * This routine will check if vht160 connection already exist or
 * no. If it exist then this routine will return true.
 *
 * Return: true if vht160 connection exist else false
 */
bool policy_mgr_vht160_conn_exist(struct wlan_objmgr_psoc *psoc)
{
	return false;
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
	return true;
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
	return QDF_STATUS_SUCCESS;
}

enum policy_mgr_con_mode policy_mgr_get_mode_by_vdev_id(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id)
{
	return PM_MAX_NUM_OF_MODE;
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
	return QDF_STATUS_SUCCESS;
}

/**
 * policy_mgr_get_current_pref_hw_mode_dbs_2x2() - Get the current preferred hw mode
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
	return PM_NOP;
}

/**
 * policy_mgr_get_current_pref_hw_mode_dbs_1x1() - Get the current preferred hw mode
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
	return PM_NOP;
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
	return QDF_STATUS_SUCCESS;
}
