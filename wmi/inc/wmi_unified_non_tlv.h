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

#include <osdep.h>
#include "a_types.h"
#include "wmi_unified_param.h"
#include "legacy/wmi.h"
#include "legacy/wmi_unified.h"
#include "ol_defines.h" /* Fix Me: wmi_unified_t structure definition */

QDF_STATUS send_vdev_create_cmd_non_tlv(wmi_unified_t wmi_handle,
				 uint8_t macaddr[IEEE80211_ADDR_LEN],
				 struct vdev_create_params *param);

QDF_STATUS send_vdev_delete_cmd_non_tlv(wmi_unified_t wmi_handle,
					  uint8_t if_id);

QDF_STATUS send_vdev_stop_cmd_non_tlv(wmi_unified_t wmi,
					uint8_t vdev_id);

QDF_STATUS send_vdev_down_cmd_non_tlv(wmi_unified_t wmi,
			uint8_t vdev_id);

QDF_STATUS send_peer_flush_tids_cmd_non_tlv(wmi_unified_t wmi,
					 uint8_t peer_addr[IEEE80211_ADDR_LEN],
					 struct peer_flush_params *param);

QDF_STATUS send_peer_delete_cmd_non_tlv(wmi_unified_t wmi,
				    uint8_t peer_addr[IEEE80211_ADDR_LEN],
				    uint8_t vdev_id);

QDF_STATUS send_peer_param_cmd_non_tlv(wmi_unified_t wmi,
				uint8_t peer_addr[IEEE80211_ADDR_LEN],
				struct peer_set_params *param);

QDF_STATUS send_vdev_up_cmd_non_tlv(wmi_unified_t wmi,
			     uint8_t bssid[IEEE80211_ADDR_LEN],
				 struct vdev_up_params *params);

QDF_STATUS send_peer_create_cmd_non_tlv(wmi_unified_t wmi,
					struct peer_create_params *param);

QDF_STATUS send_green_ap_ps_cmd_non_tlv(wmi_unified_t wmi_handle,
						uint32_t value, uint8_t mac_id);

QDF_STATUS
send_pdev_utf_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct pdev_utf_params *param,
				uint8_t mac_id);

QDF_STATUS
send_pdev_param_cmd_non_tlv(wmi_unified_t wmi_handle,
			   struct pdev_params *param,
				uint8_t mac_id);

QDF_STATUS send_suspend_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct suspend_params *param,
				uint8_t mac_id);

QDF_STATUS send_resume_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t mac_id);

QDF_STATUS send_wow_enable_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct wow_cmd_params *param,
				uint8_t mac_id);

QDF_STATUS send_set_ap_ps_param_cmd_non_tlv(wmi_unified_t wmi_handle,
					   uint8_t *peer_addr,
					   struct ap_ps_params *param);

QDF_STATUS send_set_sta_ps_param_cmd_non_tlv(wmi_unified_t wmi_handle,
					   struct sta_ps_params *param);

QDF_STATUS send_crash_inject_cmd_non_tlv(wmi_unified_t wmi_handle,
			 struct crash_inject *param);

QDF_STATUS
send_dbglog_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct dbglog_params *dbglog_param);

QDF_STATUS send_vdev_set_param_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct vdev_set_params *param);

QDF_STATUS send_stats_request_cmd_non_tlv(wmi_unified_t wmi_handle,
				uint8_t macaddr[IEEE80211_ADDR_LEN],
				struct stats_request_params *param);

QDF_STATUS send_beacon_send_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct beacon_params *param);

QDF_STATUS send_peer_assoc_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct peer_assoc_params *param);

QDF_STATUS send_scan_start_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct scan_start_params *param);

QDF_STATUS send_scan_stop_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct scan_stop_params *param);

QDF_STATUS send_scan_chan_list_cmd_non_tlv(wmi_unified_t wmi_handle,
				struct scan_chan_list_params *param);

