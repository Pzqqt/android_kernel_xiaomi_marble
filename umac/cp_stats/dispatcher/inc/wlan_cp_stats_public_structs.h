/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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

/**
 * DOC: contains cp_stats structure definitions
 */

#ifndef _WLAN_CP_STATS_PUBLIC_STRUCTS_H_
#define _WLAN_CP_STATS_PUBLIC_STRUCTS_H_

#define CTRL_PATH_STATS_MAX_MAC_ADDR 1
#define CTRL_PATH_STATS_MAX_PDEV_ID 1
#define CTRL_PATH_STATS_MAX_VDEV_ID 1


#define INFRA_CP_STATS_MAX_REQ_TWT_DIALOG_ID 1

/*
 * Maximum of 1 TWT session can be supported per vdev.
 * This can be extended later to support more sessions.
 * if there is a request to retrieve stats for all existing
 * TWT sessions then response path can have multiple session
 * stats.
 */
#define INFRA_CP_STATS_MAX_RESP_TWT_DIALOG_ID 1

#ifdef WLAN_SUPPORT_TWT
/**
 * struct twt_infra_cp_stats_event - TWT statistics event structure
 * @vdev_id: virtual interface id
 * @peer_mac_addr: peer mac address corresponding to a TWT session
 * @dialog_id: Represents dialog_id of the TWT session
 * @num_sp_cycles: Number of TWT service period elapsed so far
 * @avg_sp_dur_us: Average of actual wake duration observed so far
 * @min_sp_dur_us: Minimum value of wake duration observed across
 * @max_sp_dur_us: Maximum value of wake duration observed
 * @tx_mpdu_per_sp: Average number of MPDU's transmitted successfully
 * @rx_mpdu_per_sp: Average number of MPDU's received successfully
 * @tx_bytes_per_sp: Average number of bytes transmitted successfully
 * @rx_bytes_per_sp: Average number of bytes received successfully
 */
struct twt_infra_cp_stats_event {
	uint8_t vdev_id;
	struct qdf_mac_addr peer_macaddr;
	uint32_t dialog_id;
	uint32_t status;
	uint32_t num_sp_cycles;
	uint32_t avg_sp_dur_us;
	uint32_t min_sp_dur_us;
	uint32_t max_sp_dur_us;
	uint32_t tx_mpdu_per_sp;
	uint32_t rx_mpdu_per_sp;
	uint32_t tx_bytes_per_sp;
	uint32_t rx_bytes_per_sp;
};
#endif /* WLAN_SUPPORT_TWT */

/**
 * struct infra_cp_stats_event - Event structure to store stats
 * @action: action for which this response was recevied
 *          (get/reset/start/stop)
 * @request_id: request cookie sent to Firmware in the command
 * @status: status of the infra_cp_stats command processing
 * @num_twt_infra_cp_stats: number of twt_infra_cp_stats buffers
 *                          available
 * @twt_infra_cp_stats: pointer to TWT session statistics structures
 *
 * This structure is used to store the statistics information
 * extracted from firmware event(wmi_pdev_cp_fwstats_eventid)
 */
struct infra_cp_stats_event {
	uint32_t action;
	uint32_t request_id;
	uint32_t status;
#ifdef WLAN_SUPPORT_TWT
	uint32_t num_twt_infra_cp_stats;
	struct twt_infra_cp_stats_event *twt_infra_cp_stats;
#endif
	/* Extend with other required infra_cp_stats structs */
};

enum infra_cp_stats_action {
	ACTION_REQ_CTRL_PATH_STAT_GET = 0,
	ACTION_REQ_CTRL_PATH_STAT_RESET,
	ACTION_REQ_CTRL_PATH_STAT_START,
	ACTION_REQ_CTRL_PATH_STAT_STOP,
};

enum infra_cp_stats_id {
	TYPE_REQ_CTRL_PATH_PDEV_TX_STAT = 0,
	TYPE_REQ_CTRL_PATH_VDEV_EXTD_STAT,
	TYPE_REQ_CTRL_PATH_MEM_STAT,
	TYPE_REQ_CTRL_PATH_TWT_STAT,
};

/**
 * struct infra_cp_stats_cmd_info - details of infra cp stats request
 * @stats_id: ID of the statistics type requested
 * @action: action to be performed (get/reset/start/stop)
 * @request_cookie: osif request cookie
 * @request_id: request id cookie to FW
 * @num_pdev_ids: number of pdev ids in the request
 * @pdev_id: array of pdev_ids
 * @num_vdev_ids: number of vdev ids in the request
 * @vdev_id: array of vdev_ids
 * @num_mac_addr_list: number of mac addresses in the request
 * @peer_mac_addr: array of mac addresses
 * @dialog_id: This is a TWT specific field. only one dialog_id
 *             can be specified for TWT stats. 0 to 254 are
 *             valid dialog_id's representing a single TWT session.
 *             255 represents all twt sessions
 * @infra_cp_stats_resp_cb: callback function to handle the response
 */
struct infra_cp_stats_cmd_info {
	enum infra_cp_stats_id stats_id;
	enum infra_cp_stats_action action;
	void *request_cookie;
	uint32_t request_id;
	uint32_t num_pdev_ids;
	uint32_t pdev_id[CTRL_PATH_STATS_MAX_PDEV_ID];
	uint32_t num_vdev_ids;
	uint32_t vdev_id[CTRL_PATH_STATS_MAX_VDEV_ID];
	uint32_t num_mac_addr_list;
	uint8_t peer_mac_addr[CTRL_PATH_STATS_MAX_MAC_ADDR][QDF_MAC_ADDR_SIZE];
#ifdef WLAN_SUPPORT_TWT
	uint32_t dialog_id;
#endif
	void (*infra_cp_stats_resp_cb)(struct infra_cp_stats_event *ev,
				       void *cookie);
};
#endif
