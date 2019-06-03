/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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
 * DOC: declare internal APIs related to the blacklist component
 */

#ifndef _WLAN_BLM_CORE_H_
#define _WLAN_BLM_CORE_H_

#include <wlan_blm_main.h>

#define BLM_IS_AP_AVOIDED_BY_USERSPACE(cur_node) \
			cur_node->userspace_avoidlist

#define BLM_IS_AP_AVOIDED_BY_DRIVER(cur_node) \
		cur_node->driver_avoidlist

#define BLM_IS_AP_BLACKLISTED_BY_USERSPACE(cur_node) \
		cur_node->userspace_blacklist

#define BLM_IS_AP_BLACKLISTED_BY_DRIVER(cur_node) \
		cur_node->driver_blacklist

#define BLM_IS_AP_IN_MONITOR_LIST(cur_node) \
		cur_node->driver_monitorlist

#define BLM_IS_AP_IN_RSSI_REJECT_LIST(cur_node) \
		cur_node->rssi_reject_list

#define BLM_IS_AP_IN_BLACKLIST(cur_node) \
		(BLM_IS_AP_BLACKLISTED_BY_USERSPACE(cur_node) | \
		 BLM_IS_AP_BLACKLISTED_BY_DRIVER(cur_node) | \
		 BLM_IS_AP_IN_RSSI_REJECT_LIST(cur_node))

#define BLM_IS_AP_IN_AVOIDLIST(cur_node) \
		(BLM_IS_AP_AVOIDED_BY_USERSPACE(cur_node) | \
		 BLM_IS_AP_AVOIDED_BY_DRIVER(cur_node))

#define IS_AP_IN_USERSPACE_BLACKLIST_ONLY(cur_node) \
		(BLM_IS_AP_BLACKLISTED_BY_USERSPACE(cur_node) & \
		!(BLM_IS_AP_IN_AVOIDLIST(cur_node) | \
		 BLM_IS_AP_IN_MONITOR_LIST(cur_node) | \
		 BLM_IS_AP_IN_RSSI_REJECT_LIST(cur_node) | \
		 BLM_IS_AP_BLACKLISTED_BY_DRIVER(cur_node)))

#define IS_AP_IN_MONITOR_LIST_ONLY(cur_node) \
		(BLM_IS_AP_IN_MONITOR_LIST(cur_node) & \
		!(BLM_IS_AP_IN_AVOIDLIST(cur_node) | \
		 BLM_IS_AP_IN_BLACKLIST(cur_node)))

#define IS_AP_IN_AVOID_LIST_ONLY(cur_node) \
		(BLM_IS_AP_IN_AVOIDLIST(cur_node) & \
		!(BLM_IS_AP_IN_MONITOR_LIST(cur_node) | \
		 BLM_IS_AP_IN_BLACKLIST(cur_node)))

#define IS_AP_IN_DRIVER_BLACKLIST_ONLY(cur_node) \
		(BLM_IS_AP_BLACKLISTED_BY_DRIVER(cur_node) & \
		!(BLM_IS_AP_IN_AVOIDLIST(cur_node) | \
		 BLM_IS_AP_IN_MONITOR_LIST(cur_node) | \
		 BLM_IS_AP_IN_RSSI_REJECT_LIST(cur_node) | \
		 BLM_IS_AP_BLACKLISTED_BY_USERSPACE(cur_node)))

#define IS_AP_IN_RSSI_REJECT_LIST_ONLY(cur_node) \
		(BLM_IS_AP_IN_RSSI_REJECT_LIST(cur_node) & \
		!(BLM_IS_AP_IN_AVOIDLIST(cur_node) | \
		 BLM_IS_AP_IN_MONITOR_LIST(cur_node) | \
		 BLM_IS_AP_BLACKLISTED_BY_DRIVER(cur_node) | \
		 BLM_IS_AP_BLACKLISTED_BY_USERSPACE(cur_node)))

#define IS_AP_IN_USERSPACE_AVOID_LIST_ONLY(cur_node) \
		(BLM_IS_AP_AVOIDED_BY_USERSPACE(cur_node) & \
		!(BLM_IS_AP_AVOIDED_BY_DRIVER(cur_node) | \
		 BLM_IS_AP_IN_MONITOR_LIST(cur_node) | \
		 BLM_IS_AP_IN_BLACKLIST(cur_node)))

#define IS_AP_IN_DRIVER_AVOID_LIST_ONLY(cur_node) \
		(BLM_IS_AP_AVOIDED_BY_DRIVER(cur_node) & \
		!(BLM_IS_AP_AVOIDED_BY_USERSPACE(cur_node) | \
		 BLM_IS_AP_IN_MONITOR_LIST(cur_node) | \
		 BLM_IS_AP_IN_BLACKLIST(cur_node)))

/**
 * struct blm_reject_ap_timestamp - Structure to store the reject list BSSIDs
 * entry time stamp.
 * @userspace_avoid_timestamp: Time when userspace adds BSSID to avoid list.
 * @driver_avoid_timestamp: Time when driver adds BSSID to avoid list.
 * @userspace_blacklist_timestamp: Time when userspace adds BSSID to black list.
 * @driver_blacklist_timestamp: Time when driver adds BSSID to black list.
 * @rssi_reject_timestamp: Time when driver adds BSSID to rssi reject list.
 * @driver_monitor_timestamp: Time when driver adds BSSID to monitor list.
 */
struct blm_reject_ap_timestamp {
	qdf_time_t userspace_avoid_timestamp;
	qdf_time_t driver_avoid_timestamp;
	qdf_time_t userspace_blacklist_timestamp;
	qdf_time_t driver_blacklist_timestamp;
	qdf_time_t rssi_reject_timestamp;
	qdf_time_t driver_monitor_timestamp;
};

/**
 * struct blm_reject_ap - Structure of a node added to blacklist manager
 * @node: Node of the entry
 * @bssid: Bssid of the AP entry.
 * @rssi_reject_params: Rssi reject params of the AP entry.
 * @bad_bssid_counter: It represent how many times data stall happened.
 * @ap_timestamp: Ap timestamp.
 * @reject_ap_type: what is the type of rejection for the AP (avoid, black etc.)
 * @connect_timestamp: Timestamp when the STA got connected with this BSSID
 */
struct blm_reject_ap {
	qdf_list_node_t node;
	struct qdf_mac_addr bssid;
	struct blm_rssi_disallow_params rssi_reject_params;
	uint8_t bad_bssid_counter;
	struct blm_reject_ap_timestamp ap_timestamp;
	union {
		struct {
			uint8_t userspace_blacklist:1,
				driver_blacklist:1,
				userspace_avoidlist:1,
				driver_avoidlist:1,
				rssi_reject_list:1,
				driver_monitorlist:1;
		};
		uint8_t reject_ap_type;
	};
	qdf_time_t connect_timestamp;
};

/**
 * enum blm_bssid_action - action taken by driver for the scan results
 * @BLM_ACTION_NOP: No operation to be taken for the BSSID in the scan list.
 * @BLM_REMOVE_FROM_LIST: Remove the BSSID from the scan list ( Blacklisted APs)
 * @BLM_MOVE_AT_LAST: Attach the Ap at last of the scan list (Avoided Aps)
 */
enum blm_bssid_action {
	BLM_ACTION_NOP,
	BLM_REMOVE_FROM_LIST,
	BLM_MOVE_AT_LAST,
};

/**
 * blm_filter_bssid() - Filter out the bad Aps from the scan list.
 * @pdev: Pdev object
 * @scan_list: Scan list from the caller
 *
 * This API will filter out the bad Aps, or add the bad APs at the last
 * of the linked list if the APs are to be avoided.
 *
 * Return: QDF status
 */
QDF_STATUS
blm_filter_bssid(struct wlan_objmgr_pdev *pdev, qdf_list_t *scan_list);

/**
 * blm_add_bssid_to_reject_list() - Add BSSID to the specific reject list.
 * @pdev: Pdev object
 * @ap_info: Ap info params such as BSSID, and the type of rejection to be done
 *
 * This API will add the BSSID to the reject AP list maintained by the blacklist
 * manager.
 *
 * Return: QDF status
 */
QDF_STATUS
blm_add_bssid_to_reject_list(struct wlan_objmgr_pdev *pdev,
			     struct reject_ap_info *ap_info);

/**
 * blm_add_userspace_black_list() - Clear already existing userspace BSSID, and
 * add the new ones to blacklist manager.
 * @pdev: pdev object
 * @bssid_black_list: BSSIDs to be blacklisted by userspace.
 * @num_of_bssid: num of bssids to be blacklisted.
 *
 * This API will Clear already existing userspace BSSID, and add the new ones
 * to blacklist manager's reject list.
 *
 * Return: QDF status
 */
QDF_STATUS
blm_add_userspace_black_list(struct wlan_objmgr_pdev *pdev,
			     struct qdf_mac_addr *bssid_black_list,
			     uint8_t num_of_bssid);

/**
 * blm_update_bssid_connect_params() - Inform the BLM about connect/disconnect
 * with the current AP.
 * @pdev: pdev object
 * @bssid: BSSID of the AP
 * @con_state: Connection stae (connected/disconnected)
 *
 * This API will inform the BLM about the state with the AP so that if the AP
 * is selected, and the connection went through, and the connection did not
 * face any data stall till the bad bssid reset timer, BLM can remove the
 * AP from the reject ap list maintained by it.
 *
 * Return: None
 */
void
blm_update_bssid_connect_params(struct wlan_objmgr_pdev *pdev,
				struct qdf_mac_addr bssid,
				enum blm_connection_state con_state);

/**
 * blm_flush_reject_ap_list() - Clear away BSSID and destroy the reject ap list
 * @blm_ctx: blacklist manager pdev priv object
 *
 * This API will clear the BSSID info in the reject AP list maintained by the
 * blacklist manager, and will destroy the list as well.
 *
 * Return: None
 */
void
blm_flush_reject_ap_list(struct blm_pdev_priv_obj *blm_ctx);

/**
 * blm_get_bssid_reject_list() - Get the BSSIDs in reject list from BLM
 * @pdev: pdev object
 * @reject_list: reject list to be filled (passed by caller)
 * @max_bssid_to_be_filled: num of bssids filled in reject list by BLM
 * @reject_ap_type: reject ap type of the BSSIDs to be filled.
 *
 * This API will fill the reject ap list requested by caller of type given as
 * argument reject_ap_type, and will return the number of BSSIDs filled.
 *
 * Return: Unsigned integer (number of BSSIDs filled by the blacklist manager)
 */
uint8_t
blm_get_bssid_reject_list(struct wlan_objmgr_pdev *pdev,
			  struct reject_ap_config_params *reject_list,
			  uint8_t max_bssid_to_be_filled,
			  enum blm_reject_ap_type reject_ap_type);
#endif
