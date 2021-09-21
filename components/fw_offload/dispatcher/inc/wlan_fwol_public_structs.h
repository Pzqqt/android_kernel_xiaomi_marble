/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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
 * DOC: contains fw offload structure definations
 */

#ifndef _WLAN_FWOL_PUBLIC_STRUCTS_H_
#define _WLAN_FWOL_PUBLIC_STRUCTS_H_

#include "wlan_objmgr_psoc_obj.h"
#include "wlan_thermal_public_struct.h"
#include "wmi_unified.h"

#ifdef WLAN_FEATURE_ELNA
/**
 * struct set_elna_bypass_request - set eLNA bypass request
 * @vdev_id: vdev id
 * @en_dis: 0 - disable eLNA bypass
 *          1 - enable eLNA bypass
 */
struct set_elna_bypass_request {
	uint8_t vdev_id;
	uint8_t en_dis;
};

/**
 * struct get_elna_bypass_request - get eLNA bypass request
 * @vdev_id: vdev id
 */
struct get_elna_bypass_request {
	uint8_t vdev_id;
};

/**
 * struct get_elna_bypass_response - get eLNA bypass response
 * @vdev_id: vdev id
 * @en_dis: 0 - disable eLNA bypass
 *          1 - enable eLNA bypass
 */
struct get_elna_bypass_response {
	uint8_t vdev_id;
	uint8_t en_dis;
};
#endif

/**
 * struct thermal_throttle_info - thermal throttle info from Target
 * @temperature: current temperature in c Degree
 * @level: target thermal level info
 * @pdev_id: pdev id
 * @therm_throt_levels: Number of thermal throttle levels
 * @level_info: Thermal Stats for each level
 */
struct thermal_throttle_info {
	uint32_t temperature;
	enum thermal_throttle_level level;
	uint32_t pdev_id;
	uint32_t therm_throt_levels;
	struct thermal_throt_level_stats level_info[WMI_THERMAL_STATS_TEMP_THRESH_LEVEL_MAX];
};

/**
 * struct wlan_fwol_callbacks - fw offload callbacks
 * @get_elna_bypass_callback: callback for get eLNA bypass
 * @get_elna_bypass_context: context for get eLNA bypass
 * @get_thermal_stats_callback: callback for get thermal stats
 * @get_thermal_stats_context: context for get thermal stats
 */
struct wlan_fwol_callbacks {
#ifdef WLAN_FEATURE_ELNA
	void (*get_elna_bypass_callback)(void *context,
				     struct get_elna_bypass_response *response);
	void *get_elna_bypass_context;
#endif
#ifdef THERMAL_STATS_SUPPORT
	void (*get_thermal_stats_callback)(void *context,
				     struct thermal_throttle_info *response);
	void *get_thermal_stats_context;
#endif
};

#ifdef WLAN_FEATURE_MDNS_OFFLOAD

#define MDNS_FQDN_TYPE_GENERAL	(0)
/* Maximum length of FQDN string including the NULL byte */
#define MAX_FQDN_LEN		(64)
/* This length depends on the WMI Message and TLV Header size.
 * ((WMI_SVC_MSG_MAX_SIZE - WMI_TLV_HDR_SIZE)
 */
#define MAX_MDNS_RESP_LEN	(512)

/**
 * struct mdns_config_info - Multicast DNS configuration information
 * @vdev_id: vdev id
 * @enable: false - disable mdns
 *          true - enable mdns
 * @fqdn_type: FQDN type
 * @fqdn_data: Fully Qualified Domain Name of the local network
 * @fqdn_len: FQDN length
 * @resource_record_count: Number Resource Records present in the answer payload
 * @answer_payload_len: Length of the answer payload sent by mdnsResponder in userspace
 * @answer_payload_data: Binary blob used to frame mdns response for mdns queries
 */
struct mdns_config_info {
	uint32_t vdev_id;
	bool enable;
	uint32_t fqdn_type;
	uint32_t fqdn_len;
	uint8_t fqdn_data[MAX_FQDN_LEN];
	uint32_t resource_record_count;
	uint32_t answer_payload_len;
	uint8_t answer_payload_data[MAX_MDNS_RESP_LEN];
};
#endif

/**
 * struct wlan_fwol_tx_ops - structure of tx func pointers
 * @set_elna_bypass: set eLNA bypass
 * @get_elna_bypass: get eLNA bypass
 * @reg_evt_handler: register event handler
 * @unreg_evt_handler: unregister event handler
 * @send_dscp_up_map_to_fw: send dscp-to-up map values to FW
 * @set_mdns_config: set mdns config info
 * @get_thermal_stats: send get_thermal_stats cmd to FW
 */
struct wlan_fwol_tx_ops {
#ifdef WLAN_FEATURE_ELNA
	QDF_STATUS (*set_elna_bypass)(struct wlan_objmgr_psoc *psoc,
				      struct set_elna_bypass_request *req);
	QDF_STATUS (*get_elna_bypass)(struct wlan_objmgr_psoc *psoc,
				      struct get_elna_bypass_request *req);
#endif
	QDF_STATUS (*reg_evt_handler)(struct wlan_objmgr_psoc *psoc,
				      void *arg);
	QDF_STATUS (*unreg_evt_handler)(struct wlan_objmgr_psoc *psoc,
					void *arg);
#ifdef WLAN_SEND_DSCP_UP_MAP_TO_FW
	QDF_STATUS (*send_dscp_up_map_to_fw)(
			struct wlan_objmgr_psoc *psoc,
			uint32_t *dscp_to_up_map);
#endif
#ifdef WLAN_FEATURE_MDNS_OFFLOAD
	QDF_STATUS (*set_mdns_config)(struct wlan_objmgr_psoc *psoc,
				      struct mdns_config_info *mdns_info);
#endif
#ifdef THERMAL_STATS_SUPPORT
	QDF_STATUS (*get_thermal_stats)(struct wlan_objmgr_psoc *psoc,
				      enum thermal_stats_request_type req_type,
				      uint8_t therm_stats_offset);
#endif
};

/**
 * struct wlan_fwol_rx_ops - structure of rx func pointers
 * @get_elna_bypass_resp: get eLNA bypass response
 * @notify_thermal_throttle_handler: thermal stats indication callback to fwol
 *  core from target if layer
 * @get_thermal_stats_resp: thermal stats cmd response callback to fwol
 */
struct wlan_fwol_rx_ops {
#ifdef WLAN_FEATURE_ELNA
	QDF_STATUS (*get_elna_bypass_resp)(struct wlan_objmgr_psoc *psoc,
					 struct get_elna_bypass_response *resp);
#endif
#ifdef FW_THERMAL_THROTTLE_SUPPORT
	QDF_STATUS (*notify_thermal_throttle_handler)(
				struct wlan_objmgr_psoc *psoc,
				struct thermal_throttle_info *info);
#endif
#ifdef THERMAL_STATS_SUPPORT
	QDF_STATUS (*get_thermal_stats_resp)(struct wlan_objmgr_psoc *psoc,
					    struct thermal_throttle_info *resp);
#endif
};

/**
 * struct fwol_thermal_callbacks - structure of rx callback to hdd layer
 * @notify_thermal_throttle_handler: thermal throttle event callback
 */
struct fwol_thermal_callbacks {
	QDF_STATUS (*notify_thermal_throttle_handler)(
					struct wlan_objmgr_psoc *psoc,
					struct thermal_throttle_info *info);
};
#endif /* _WLAN_FWOL_PUBLIC_STRUCTS_H_ */

