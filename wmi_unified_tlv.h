/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
#ifndef _WMI_UNIFIED_TLV_H_
#define _WMI_UNIFIED_TLV_H_
#include <osdep.h>
#include "a_types.h"
#include "wmi_unified_param.h"
#include "ol_defines.h" /* Fix Me: wmi_unified_t structure definition */

int32_t send_vdev_create_cmd_tlv(wmi_unified_t wmi_handle,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param);

int32_t send_vdev_delete_cmd_tlv(wmi_unified_t wmi_handle,
					  uint8_t if_id);

int32_t send_vdev_stop_cmd_tlv(wmi_unified_t wmi,
					uint8_t vdev_id);

int32_t send_vdev_down_cmd_tlv(wmi_unified_t wmi,
			uint8_t vdev_id);

int32_t send_peer_flush_tids_cmd_tlv(wmi_unified_t wmi,
			 uint8_t peer_addr[IEEE80211_ADDR_LEN],
			 struct peer_flush_params *param);

int32_t send_peer_delete_cmd_tlv(wmi_unified_t wmi,
			    uint8_t peer_addr[IEEE80211_ADDR_LEN],
			    uint8_t vdev_id);

int32_t send_peer_param_cmd_tlv(wmi_unified_t wmi,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param);

int32_t send_vdev_up_cmd_tlv(wmi_unified_t wmi,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params);

int32_t send_peer_create_cmd_tlv(wmi_unified_t wmi,
					struct peer_create_params *param);

int32_t send_green_ap_ps_cmd_tlv(wmi_unified_t wmi_handle,
						uint32_t value, uint8_t mac_id);

int32_t
send_pdev_utf_cmd_tlv(wmi_unified_t wmi_handle,
				struct pdev_utf_params *param,
				uint8_t mac_id);

int32_t
send_pdev_param_cmd_tlv(wmi_unified_t wmi_handle,
			   struct pdev_params *param,
				uint8_t mac_id);

int32_t send_suspend_cmd_tlv(wmi_unified_t wmi_handle,
				struct suspend_params *param,
				uint8_t mac_id);

int32_t send_resume_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t mac_id);

int32_t send_wow_enable_cmd_tlv(wmi_unified_t wmi_handle,
				struct wow_cmd_params *param,
				uint8_t mac_id);

int32_t send_set_ap_ps_param_cmd_tlv(wmi_unified_t wmi_handle,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param);

int32_t send_set_sta_ps_param_cmd_tlv(wmi_unified_t wmi_handle,
					   struct sta_ps_params *param);

int32_t send_crash_inject_cmd_tlv(wmi_unified_t wmi_handle,
			 struct crash_inject *param);

int32_t
send_dbglog_cmd_tlv(wmi_unified_t wmi_handle,
				struct dbglog_params *dbglog_param);


int32_t send_vdev_set_param_cmd_tlv(wmi_unified_t wmi_handle,
				struct vdev_set_params *param);

int32_t send_stats_request_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param);

int32_t send_packet_log_enable_cmd_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct packet_enable_params *param);

int32_t send_beacon_send_cmd_tlv(wmi_unified_t wmi_handle,
				struct beacon_params *param);

int32_t send_peer_assoc_cmd_tlv(wmi_unified_t wmi_handle,
				struct peer_assoc_params *param);

int32_t send_scan_start_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_start_params *param);

int32_t send_scan_stop_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_stop_params *param);

int32_t send_scan_chan_list_cmd_tlv(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *param);

int32_t send_mgmt_cmd_tlv(wmi_unified_t wmi_handle,
				struct wmi_mgmt_params *param);

int32_t send_modem_power_state_cmd_tlv(wmi_unified_t wmi_handle,
		uint32_t param_value);

int32_t send_set_sta_ps_mode_cmd_tlv(wmi_unified_t wmi_handle,
			       uint32_t vdev_id, uint8_t val);

int32_t send_set_sta_uapsd_auto_trig_cmd_tlv(wmi_unified_t wmi_handle,
				struct sta_uapsd_trig_params *param);

int32_t send_get_temperature_cmd_tlv(wmi_unified_t wmi_handle);

int32_t send_set_p2pgo_oppps_req_cmd_tlv(wmi_unified_t wmi_handle,
		struct p2p_ps_params *oppps);

int32_t send_set_p2pgo_noa_req_cmd_tlv(wmi_unified_t wmi_handle,
			struct p2p_ps_params *noa);

int32_t send_set_smps_params_cmd_tlv(wmi_unified_t wmi_handle, uint8_t vdev_id,
			       int value);

int32_t send_set_mimops_cmd_tlv(wmi_unified_t wmi_handle,
			uint8_t vdev_id, int value);

int32_t send_ocb_set_utc_time_cmd_tlv(wmi_unified_t wmi_handle,
				struct ocb_utc_param *utc);

int send_ocb_start_timing_advert_cmd_tlv(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert);

int send_ocb_stop_timing_advert_cmd_tlv(wmi_unified_t wmi_handle,
	struct ocb_timing_advert_param *timing_advert);

int send_ocb_get_tsf_timer_cmd_tlv(wmi_unified_t wmi_handle,
			  uint8_t vdev_id);

int32_t send_dcc_get_stats_cmd_tlv(wmi_unified_t wmi_handle,
		     struct dcc_get_stats_param *get_stats_param);

int32_t send_dcc_clear_stats_cmd_tlv(wmi_unified_t wmi_handle,
				uint32_t vdev_id, uint32_t dcc_stats_bitmap);

int send_dcc_update_ndl_cmd_tlv(wmi_unified_t wmi_handle,
		       struct dcc_update_ndl_param *update_ndl_param);

int32_t send_ocb_set_config_cmd_tlv(wmi_unified_t wmi_handle,
		struct ocb_config_param *config, uint32_t *ch_mhz);
