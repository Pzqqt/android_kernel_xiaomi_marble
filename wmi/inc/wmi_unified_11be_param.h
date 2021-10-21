
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _WMI_UNIFIED_11BE_PARAM_H_
#define _WMI_UNIFIED_11BE_PARAM_H_

#ifdef WLAN_FEATURE_11BE_MLO

#define MAX_LINK_IN_MLO 6
/** struct wmi_mlo_setup_params - MLO setup command params
 * @mld_grp_id: Unique ID to FW for MLD group
 * @pdev_id: pdev id of radio on which this command is sent
 * @num_valid_hw_links: Num of valid links in partner_links array
 * @partner_links[MAX_LINK_IN_MLO]: Partner link IDs
 */
struct wmi_mlo_setup_params {
	uint32_t mld_grp_id;
	uint32_t pdev_id;
	uint8_t num_valid_hw_links;
	uint32_t partner_links[MAX_LINK_IN_MLO];
};

/** struct wmi_mlo_ready_params - MLO ready command params
 * @pdev_id: pdev id of radio on which this command is sent
 */
struct wmi_mlo_ready_params {
	uint32_t pdev_id;
};

/** enum wmi_mlo_teardown_reason - Reason code in WMI MLO teardown command
 * @WMI_MLO_TEARDOWN_REASON_DOWN: Wifi down
 * @WMI_MLO_TEARDOWN_REASON_SSR: Wifi Recovery
 */
enum wmi_mlo_teardown_reason {
	WMI_MLO_TEARDOWN_REASON_DOWN,
	WMI_MLO_TEARDOWN_REASON_SSR,
};

/** struct wmi_mlo_teardown_params - MLO teardown command params
 * @pdev_id: pdev id of radio on which this command is sent
 * @reason: reason code from enum wmi_mlo_teardown_reason
 */
struct wmi_mlo_teardown_params {
	uint32_t pdev_id;
	enum wmi_mlo_teardown_reason reason;
};

/** enum wmi_mlo_setup_status - Status code in WMI MLO setup completion event
 * @WMI_MLO_SETUP_STATUS_SUCCESS: Success
 * @WMI_MLO_SETUP_STATUS_FAILURE: Failure
 */
enum wmi_mlo_setup_status {
	WMI_MLO_SETUP_STATUS_SUCCESS,
	WMI_MLO_SETUP_STATUS_FAILURE,
};

/** struct wmi_mlo_setup_complete_params - MLO setup complete event params
 * @pdev_id: pdev id of radio on which this event is received
 * @status: status code
 */
struct wmi_mlo_setup_complete_params {
	uint32_t pdev_id;
	enum wmi_mlo_setup_status status;
};

/** enum wmi_mlo_teardown_status - Status code in WMI MLO teardown completion
 *                                 event
 * @WMI_MLO_TEARDOWN_STATUS_SUCCESS: Success
 * @WMI_MLO_TEARDOWN_STATUS_FAILURE: Failure
 */
enum wmi_mlo_teardown_status {
	WMI_MLO_TEARDOWN_STATUS_SUCCESS,
	WMI_MLO_TEARDOWN_STATUS_FAILURE,
};

/** struct wmi_mlo_teardown_cmpl_params - MLO setup teardown event params
 * @pdev_id: pdev id of radio on which this event is received
 * @status: Teardown status from enum wmi_mlo_teardown_status
 */
struct wmi_mlo_teardown_cmpl_params {
	uint32_t pdev_id;
	enum wmi_mlo_teardown_status status;
};

/* maximum size of vdev bitmap array for MLO link set active command */
#define WMI_MLO_VDEV_BITMAP_SZ 2

/* maximum size of link number param array for MLO link set active command */
#define WMI_MLO_LINK_NUM_SZ 2

/**
 * enum wmi_mlo_link_force_mode: MLO link force modes
 * @WMI_MLO_LINK_FORCE_MODE_ACTIVE:
 *  Force specific links active
 * @WMI_MLO_LINK_FORCE_MODE_INACTIVE:
 *  Force specific links inactive
 * @WMI_MLO_LINK_FORCE_MODE_ACTIVE_NUM:
 *  Force active a number of links, firmware to decide which links to inactive
 * @WMI_MLO_LINK_FORCE_MODE_INACTIVE_NUM:
 *  Force inactive a number of links, firmware to decide which links to inactive
 * @WMI_MLO_LINK_FORCE_MODE_NO_FORCE:
 *  Cancel the force operation of specific links, allow firmware to decide
 */
enum wmi_mlo_link_force_mode {
	WMI_MLO_LINK_FORCE_MODE_ACTIVE       = 1,
	WMI_MLO_LINK_FORCE_MODE_INACTIVE     = 2,
	WMI_MLO_LINK_FORCE_MODE_ACTIVE_NUM   = 3,
	WMI_MLO_LINK_FORCE_MODE_INACTIVE_NUM = 4,
	WMI_MLO_LINK_FORCE_MODE_NO_FORCE     = 5,
};

/**
 * enum wmi_mlo_link_force_reason: MLO link force reasons
 * @WMI_MLO_LINK_FORCE_REASON_CONNECT:
 *  Set force specific links because of new connection
 * @WMI_MLO_LINK_FORCE_REASON_DISCONNECT:
 *  Set force specific links because of new dis-connection
 */
enum wmi_mlo_link_force_reason {
	WMI_MLO_LINK_FORCE_REASON_CONNECT    = 1,
	WMI_MLO_LINK_FORCE_REASON_DISCONNECT = 2,
};

/**
 * struct wmi_mlo_link_set_active_resp: MLO link set active response structure
 * @status: Return status, 0 for success, non-zero otherwise
 * @active_sz: size of current active vdev bitmap array
 * @active: current active vdev bitmap array
 * @inactive_sz: size of current inactive vdev bitmap array
 * @inactive: current inactive vdev bitmap array
 */
struct wmi_mlo_link_set_active_resp {
	uint32_t status;
	uint32_t active_sz;
	uint32_t active[WMI_MLO_VDEV_BITMAP_SZ];
	uint32_t inactive_sz;
	uint32_t inactive[WMI_MLO_VDEV_BITMAP_SZ];
};

/**
 * struct wmi_mlo_link_num_param: MLO link set active number params
 * @num_of_link: number of links to active/inactive
 * @vdev_type: type of vdev
 * @vdev_subtype: subtype of vdev
 * @home_freq: home frequency of the link
 */
struct wmi_mlo_link_num_param {
	uint32_t num_of_link;
	uint32_t vdev_type;
	uint32_t vdev_subtype;
	uint32_t home_freq;
};

/**
 * struct wmi_mlo_link_set_active_param: MLO link set active params
 * @force_mode: operation to take (enum wmi_mlo_link_force_mode)
 * @reason: reason for the operation (enum wmi_mlo_link_force_reason)
 * @entry_num: number of the valid entries for link_num/vdev_bitmap
 * @link_num: link number param array
 *  It's present only when force_mode is WMI_MLO_LINK_FORCE_MODE_ACTIVE_NUM or
 *  WMI_MLO_LINK_FORCE_MODE_INACTIVE_NUM
 * @vdev_bitmap: active/inactive vdev bitmap array
 *  It's present only when force_mode is WMI_MLO_LINK_FORCE_MODE_ACTIVE,
 *  WMI_MLO_LINK_FORCE_MODE_INACTIVE or WMI_MLO_LINK_FORCE_MODE_NO_FORCE.
 */
struct wmi_mlo_link_set_active_param {
	uint32_t force_mode;
	uint32_t reason;
	uint32_t entry_num;
	union {
		struct wmi_mlo_link_num_param link_num[WMI_MLO_LINK_NUM_SZ];
		uint32_t vdev_bitmap[WMI_MLO_VDEV_BITMAP_SZ];
	};
};
#endif
#endif
