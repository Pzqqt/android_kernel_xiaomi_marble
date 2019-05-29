/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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
 * DOC: define public structures of blacklist mgr.
 */

#ifndef _WLAN_BLM_PUBLIC_STRUCT_H
#define _WLAN_BLM_PUBLIC_STRUCT_H

#include <qdf_types.h>
#include "wlan_objmgr_pdev_obj.h"

#define MAX_BAD_AP_LIST_SIZE               28
#define MAX_RSSI_AVOID_BSSID_LIST    10
#define PDEV_MAX_NUM_BSSID_DISALLOW_LIST  28
/**
 * struct blm_rssi_disallow_params - structure to specify params for RSSI reject
 * @retry_delay: Time before which the AP doesn't expect a connection.
 * @expected_rssi: RSSI less than which only the STA should try association.
 */
struct blm_rssi_disallow_params {
	uint32_t retry_delay;
	int8_t expected_rssi;
};

/**
 * enum blm_reject_ap_type - Rejection type of the AP
 * @USERSPACE_AVOID_TYPE: userspace wants the AP to be avoided.
 * @USERSPACE_BLACKLIST_TYPE: userspace wants the AP to be blacklisted.
 * @DRIVER_AVOID_TYPE: driver wants the AP to be avoided.
 * @DRIVER_BLACKLIST_TYPE: driver wants the AP to be blacklisted.
 * @DRIVER_RSSI_REJECT_TYPE: driver wants the AP to be in driver rssi reject.
 * @DRIVER_MONITOR_TYPE: driver wants the AP to be in monitor list.
 */
enum blm_reject_ap_type {
	USERSPACE_AVOID_TYPE =     0,
	USERSPACE_BLACKLIST_TYPE = 1,
	DRIVER_AVOID_TYPE    =     2,
	DRIVER_BLACKLIST_TYPE    = 3,
	DRIVER_RSSI_REJECT_TYPE =  4,
	DRIVER_MONITOR_TYPE =      5
};

/**
 * enum blm_connection_state - State with AP (Connected, Disconnected)
 * @BLM_AP_CONNECTED: Connected with the AP
 * @BLM_AP_DISCONNECTED: Disconnected with the AP
 */
enum blm_connection_state {
	BLM_AP_CONNECTED,
	BLM_AP_DISCONNECTED,
};

/**
 * struct reject_ap_config_params - Structure to send reject ap list to FW
 * @bssid: BSSID of the AP
 * @reject_ap_type: Type of the rejection done with the BSSID
 * @reject_duration: time left till the AP is in the reject list.
 * @expected_rssi: expected RSSI when the AP expects the connection to be made.
 */
struct reject_ap_config_params {
	struct qdf_mac_addr bssid;
	enum blm_reject_ap_type reject_ap_type;
	uint32_t reject_duration;
	int32_t expected_rssi;
};

/**
 * struct reject_ap_params - Struct to send bssid list and there num to FW
 * @num_of_reject_bssid: num of bssid params there in bssid config.
 * @bssid_list: Pointer to the bad bssid list
 */
struct reject_ap_params {
	uint8_t num_of_reject_bssid;
	struct reject_ap_config_params *bssid_list;
};

/**
 * struct wlan_blm_tx_ops - structure of tx operation function
 * pointers for blacklist manager component
 * @blm_send_reject_ap_list: send reject ap list to fw
 */
struct wlan_blm_tx_ops {
	QDF_STATUS (*blm_send_reject_ap_list)(struct wlan_objmgr_pdev *pdev,
					struct reject_ap_params *reject_params);
};

/**
 * struct reject_ap_info - structure to specify the reject ap info.
 * @bssid: BSSID of the AP.
 * @rssi_reject_params: RSSI reject params of the AP is of type RSSI reject
 * @reject_ap_type: Reject type of AP (eg. avoid, blacklist, rssi reject etc.)
 */
struct reject_ap_info {
	struct qdf_mac_addr bssid;
	struct blm_rssi_disallow_params rssi_reject_params;
	enum blm_reject_ap_type reject_ap_type;
};

#endif
