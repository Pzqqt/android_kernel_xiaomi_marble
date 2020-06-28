/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: This file contains definitions for MLME roaming offload.
 */

#ifndef CM_ROAM_PUBLIC_STRUCT_H__
#define CM_ROAM_PUBLIC_STRUCT_H__

#include "wlan_objmgr_cmn.h"
#include "reg_services_public_struct.h"
#include "wmi_unified_param.h"
#include "wmi_unified_sta_param.h"

#if defined(WLAN_FEATURE_HOST_ROAM) || defined(WLAN_FEATURE_ROAM_OFFLOAD)
/**
 * enum roam_offload_state - Roaming module state for each STA vdev.
 * @WLAN_ROAM_DEINIT: Roaming module is not initialized at the
 *  firmware.
 * @WLAN_ROAM_INIT: Roaming module initialized at the firmware.
 * @WLAN_ROAM_RSO_ENABLED: RSO enabled, firmware can roam to different AP.
 * @WLAN_ROAM_RSO_STOPPED: RSO stopped - roaming module is initialized at
 * firmware, but firmware cannot do roaming due to supplicant disabled
 * roaming/driver disabled roaming.
 * @WLAN_ROAMING_IN_PROG: Roaming started at firmware. This state is
 * transitioned after candidate selection is done at fw and preauth to
 * the AP is started.
 * @WLAN_ROAM_SYNCH_IN_PROG: Roaming handoff complete
 */
enum roam_offload_state {
	WLAN_ROAM_DEINIT,
	WLAN_ROAM_INIT,
	WLAN_ROAM_RSO_ENABLED,
	WLAN_ROAM_RSO_STOPPED,
	WLAN_ROAMING_IN_PROG,
	WLAN_ROAM_SYNCH_IN_PROG,
};

/**
 *  struct roam_btm_response_data - BTM response related data
 *  @present:       Flag to check if the roam btm_rsp tlv is present
 *  @btm_status:    Btm request status
 *  @target_bssid:  AP MAC address
 *  @vsie_reason:   Vsie_reason value
 */
struct roam_btm_response_data {
	bool present;
	uint32_t btm_status;
	struct qdf_mac_addr target_bssid;
	uint32_t vsie_reason;
};

/**
 *  struct roam_initial_data - Roam initial related data
 *  @present:                Flag to check if the roam btm_rsp tlv is present
 *  @roam_full_scan_count:   Roam full scan count
 *  @rssi_th:                RSSI threhold
 *  @cu_th:                  Channel utilization threhold
 *  @fw_cancel_timer_bitmap: FW timers, which are getting cancelled
 */
struct roam_initial_data {
	bool present;
	uint32_t roam_full_scan_count;
	uint32_t rssi_th;
	uint32_t cu_th;
	uint32_t fw_cancel_timer_bitmap;
};

/**
 * enum wlan_cm_rso_control_requestor - Driver disabled roaming requestor that
 * will request the roam module to disable roaming based on the mlme operation
 * @RSO_INVALID_REQUESTOR: invalid requestor
 * @RSO_START_BSS: disable roaming temporarily due to start bss
 * @RSO_CHANNEL_SWITCH: disable roaming due to STA channel switch
 * @RSO_CONNECT_START: disable roaming temporarily due to connect
 * @RSO_SAP_CHANNEL_CHANGE: disable roaming due to SAP channel change
 * @RSO_NDP_CON_ON_NDI: disable roaming due to NDP connection on NDI
 * @RSO_SET_PCL: Disable roaming to set pcl to firmware
 */
enum wlan_cm_rso_control_requestor {
	RSO_INVALID_REQUESTOR,
	RSO_START_BSS          = BIT(0),
	RSO_CHANNEL_SWITCH     = BIT(1),
	RSO_CONNECT_START      = BIT(2),
	RSO_SAP_CHANNEL_CHANGE = BIT(3),
	RSO_NDP_CON_ON_NDI     = BIT(4),
	RSO_SET_PCL            = BIT(5),
};
#endif

/**
 * struct set_pcl_req - Request message to set the PCL
 * @vdev_id:   Vdev id
 * @band_mask: Supported band mask
 * @clear_vdev_pcl: Clear the configured vdev pcl channels
 * @chan_weights: PCL channel weights
 */
struct set_pcl_req {
	uint8_t vdev_id;
	uint32_t band_mask;
	bool clear_vdev_pcl;
	struct wmi_pcl_chan_weights chan_weights;
};

/**
 * wlan_cm_roam_tx_ops  - structure of tx function pointers for
 * roaming related commands
 * @send_vdev_set_pcl_cmd: TX ops function pointer to send set vdev PCL
 * command
 */
struct wlan_cm_roam_tx_ops {
	QDF_STATUS (*send_vdev_set_pcl_cmd) (struct wlan_objmgr_vdev *vdev,
					     struct set_pcl_req *req);
};

/**
 * struct wlan_cm_roam  - Connection manager roam configs, state and roam
 * data related structure
 * @tx_ops: Roam Tx ops to send roam offload commands to firmware
 * @pcl_vdev_cmd_active:  Flag to check if vdev level pcl command needs to be
 * sent or PDEV level PCL command needs to be sent
 */
struct wlan_cm_roam {
	struct wlan_cm_roam_tx_ops tx_ops;
	bool pcl_vdev_cmd_active;
};
#endif
