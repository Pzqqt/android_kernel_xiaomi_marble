/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

/*
 * This file contains the API definitions for the Unified Wireless
 * Module Interface (WMI).
 */
#ifndef _WMI_UNIFIED_PRIV_H_
#define _WMI_UNIFIED_PRIV_H_
#include <osdep.h>
#include "a_types.h"
#include "wmi.h"
#include "wmi_unified.h"
#include "cdf_atomic.h"

#define WMI_UNIFIED_MAX_EVENT 0x100
#define WMI_MAX_CMDS  1024

typedef cdf_nbuf_t wmi_buf_t;

#ifdef WMI_INTERFACE_EVENT_LOGGING

#define WMI_EVENT_DEBUG_MAX_ENTRY (1024)

struct wmi_command_debug {
	uint32_t command;
	uint32_t data[4]; /*16 bytes of WMI cmd excluding TLV and WMI headers */
	uint64_t time;
};

struct wmi_event_debug {
	uint32_t event;
	uint32_t data[4]; /*16 bytes of WMI event data excluding TLV header */
	uint64_t time;
};

#endif /*WMI_INTERFACE_EVENT_LOGGING */

#ifdef WLAN_OPEN_SOURCE
struct fwdebug {
	struct sk_buff_head fwlog_queue;
	struct completion fwlog_completion;
	A_BOOL fwlog_open;
};
#endif /* WLAN_OPEN_SOURCE */

struct wmi_ops {
int32_t (*send_vdev_create_cmd)(wmi_unified_t wmi_handle,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param);

int32_t (*send_vdev_delete_cmd)(wmi_unified_t wmi_handle,
					  uint8_t if_id);

int32_t (*send_vdev_stop_cmd)(wmi_unified_t wmi,
					uint8_t vdev_id);

int32_t (*send_vdev_down_cmd)(wmi_unified_t wmi,
			uint8_t vdev_id);

int32_t (*send_peer_flush_tids_cmd)(wmi_unified_t wmi,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param);

int32_t (*send_peer_delete_cmd)(wmi_unified_t wmi,
				    uint8_t peer_addr[IEEE80211_ADDR_LEN],
				    uint8_t vdev_id);

int32_t (*send_peer_param_cmd)(wmi_unified_t wmi,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param);

int32_t (*send_vdev_up_cmd)(wmi_unified_t wmi,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params);

int32_t (*send_peer_create_cmd)(wmi_unified_t wmi,
					struct peer_create_params *param);

int32_t (*send_green_ap_ps_cmd)(wmi_unified_t wmi_handle,
						uint32_t value, uint8_t mac_id);

int32_t
(*send_pdev_utf_cmd)(wmi_unified_t wmi_handle,
				struct pdev_utf_params *param,
				uint8_t mac_id);

int32_t
(*send_pdev_param_cmd)(wmi_unified_t wmi_handle,
			   struct pdev_params *param,
				uint8_t mac_id);

int32_t (*send_suspend_cmd)(wmi_unified_t wmi_handle,
				struct suspend_params *param,
				uint8_t mac_id);

int32_t (*send_resume_cmd)(wmi_unified_t wmi_handle,
				uint8_t mac_id);

int32_t (*send_wow_enable_cmd)(wmi_unified_t wmi_handle,
				struct wow_cmd_params *param,
				uint8_t mac_id);

int32_t (*send_set_ap_ps_param_cmd)(wmi_unified_t wmi_handle,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param);

int32_t (*send_set_sta_ps_param_cmd)(wmi_unified_t wmi_handle,
					   struct sta_ps_params *param);

int32_t (*send_crash_inject_cmd)(wmi_unified_t wmi_handle,
			 struct crash_inject *param);

int32_t
(*send_dbglog_cmd)(wmi_unified_t wmi_handle,
				struct dbglog_params *dbglog_param);

int32_t (*send_vdev_set_param_cmd)(wmi_unified_t wmi_handle,
				struct vdev_set_params *param);

int32_t (*send_stats_request_cmd)(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param);

int32_t (*send_packet_log_enable_cmd)(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param);

int32_t (*send_beacon_send_cmd)(wmi_unified_t wmi_handle,
				struct beacon_params *param);

int32_t (*send_peer_assoc_cmd)(wmi_unified_t wmi_handle,
				struct peer_assoc_params *param);

int32_t (*send_scan_start_cmd)(wmi_unified_t wmi_handle,
				struct scan_start_params *param);

int32_t (*send_scan_stop_cmd)(wmi_unified_t wmi_handle,
				struct scan_stop_params *param);

int32_t (*send_scan_chan_list_cmd)(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *param);

int32_t (*send_mgmt_cmd)(wmi_unified_t wmi_handle,
				struct wmi_mgmt_params *param);

int32_t (*send_modem_power_state_cmd)(wmi_unified_t wmi_handle,
		uint32_t param_value);

int32_t (*send_set_sta_ps_mode_cmd)(wmi_unified_t wmi_handle,
			       uint32_t vdev_id, uint8_t val);

int32_t (*send_get_temperature_cmd)(wmi_unified_t wmi_handle);

int32_t (*send_set_p2pgo_oppps_req_cmd)(wmi_unified_t wmi_handle,
		struct p2p_ps_params *oppps);

int32_t (*send_set_p2pgo_noa_req_cmd)(wmi_unified_t wmi_handle,
			struct p2p_ps_params *noa);

int32_t (*send_set_smps_params_cmd)(wmi_unified_t wmi_handle, uint8_t vdev_id,
			       int value);

int32_t (*send_set_mimops_cmd)(wmi_unified_t wmi_handle,
			uint8_t vdev_id, int value);

int32_t (*send_set_sta_uapsd_auto_trig_cmd)(wmi_unified_t wmi_handle,
				struct sta_uapsd_trig_params *param);

int32_t (*send_ocb_set_utc_time_cmd)(wmi_unified_t wmi_handle,
				struct ocb_utc_param *utc);

int32_t (*send_ocb_get_tsf_timer_cmd)(wmi_unified_t wmi_handle,
			  uint8_t vdev_id);

int32_t (*send_ocb_start_timing_advert_cmd)(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert);

int32_t (*send_ocb_stop_timing_advert_cmd)(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert);

int32_t (*send_dcc_get_stats_cmd)(wmi_unified_t wmi_handle,
		     struct dcc_get_stats_param *get_stats_param);

int32_t (*send_dcc_clear_stats_cmd)(wmi_unified_t wmi_handle,
				uint32_t vdev_id, uint32_t dcc_stats_bitmap);

int32_t (*send_dcc_update_ndl_cmd)(wmi_unified_t wmi_handle,
		       struct dcc_update_ndl_param *update_ndl_param);

int32_t (*send_ocb_set_config_cmd)(wmi_unified_t wmi_handle,
		  struct ocb_config_param *config, uint32_t *ch_mhz);
};

struct wmi_unified {
	ol_scn_t scn_handle;    /* handle to device */
	osdev_t  osdev; /* handle to use OS-independent services */
	cdf_atomic_t pending_cmds;
	HTC_ENDPOINT_ID wmi_endpoint_id;
	uint16_t max_msg_len;
	WMI_EVT_ID event_id[WMI_UNIFIED_MAX_EVENT];
	wmi_unified_event_handler event_handler[WMI_UNIFIED_MAX_EVENT];
	enum wmi_rx_exec_ctx ctx[WMI_UNIFIED_MAX_EVENT];
	uint32_t max_event_idx;
	void *htc_handle;
	cdf_spinlock_t eventq_lock;
	cdf_nbuf_queue_t event_queue;
	struct work_struct rx_event_work;
	int wmi_stop_in_progress;
#ifdef WLAN_OPEN_SOURCE
	struct fwdebug dbglog;
	struct dentry *debugfs_phy;
#endif /* WLAN_OPEN_SOURCE */

#ifdef WMI_INTERFACE_EVENT_LOGGING
	cdf_spinlock_t wmi_record_lock;
#endif /*WMI_INTERFACE_EVENT_LOGGING */

	cdf_atomic_t is_target_suspended;

#ifdef FEATURE_RUNTIME_PM
	cdf_atomic_t runtime_pm_inprogress;
#endif

	struct wmi_rx_ops rx_ops;
	struct wmi_ops *ops;
	void *event_handler_cookie[WMI_UNIFIED_MAX_EVENT];
	bool use_cookie;
};
struct wmi_ops *wmi_get_tlv_ops(void);
struct wmi_ops *wmi_get_non_tlv_ops(void);
#endif
