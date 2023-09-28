/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_vdev_mgr_tgt_if_rx_defs.h
 *
 * This header file provides definitions to data structures for
 * corresponding vdev mgmt operation
 */

#ifndef __WLAN_VDEV_MGR_TGT_IF_RX_DEFS_H__
#define __WLAN_VDEV_MGR_TGT_IF_RX_DEFS_H__

#include <qdf_timer.h>
#include <qdf_atomic.h>
#include <qdf_util.h>
#include <wlan_cmn.h>
#ifdef FEATURE_RUNTIME_PM
#include <wlan_pmo_common_public_struct.h>
#endif

/**
 * enum wlan_vdev_mgr_tgt_if_rsp_bit - response status bit
 * START_RESPONSE_BIT: vdev start response bit
 * RESTART_RESPONSE_BIT: vdev restart response bit
 * STOP_RESPONSE_BIT: vdev stop response bit
 * DELETE_RESPONSE_BIT:  vdev delete response bit
 * PEER_DELETE_ALL_RESPONSE_BIT: vdev peer delete all response bit
 * RSO_STOP_RESPONSE_BIT : RSO stop response bit
 */
enum wlan_vdev_mgr_tgt_if_rsp_bit {
	START_RESPONSE_BIT = 0,
	RESTART_RESPONSE_BIT = 1,
	STOP_RESPONSE_BIT = 2,
	DELETE_RESPONSE_BIT = 3,
	PEER_DELETE_ALL_RESPONSE_BIT = 4,
	RSO_STOP_RESPONSE_BIT = 5,
	RESPONSE_BIT_MAX,
};

/**
 * string_from_rsp_bit() - Convert response bit to string
 * @bit - response bit as in wlan_vdev_mgr_tgt_if_rsp_bit
 *
 * Please note to add new string in the array at index equal to
 * its enum value in wlan_vdev_mgr_tgt_if_rsp_bit.
 */
static inline char *string_from_rsp_bit(enum wlan_vdev_mgr_tgt_if_rsp_bit bit)
{
	static const char *strings[] = { "START",
					"RESTART",
					"STOP",
					"DELETE",
					"PEER DELETE ALL",
					"RSO STOP",
					"RESPONE MAX"};
	return (char *)strings[bit];
}

#ifdef FEATURE_RUNTIME_PM
/* Add extra PMO_RESUME_TIMEOUT for runtime PM resume timeout */
#define START_RESPONSE_TIMER           (6000 + PMO_RESUME_TIMEOUT)
#define STOP_RESPONSE_TIMER            (4000 + PMO_RESUME_TIMEOUT)
#define DELETE_RESPONSE_TIMER          (4000 + PMO_RESUME_TIMEOUT)
#define PEER_DELETE_ALL_RESPONSE_TIMER (6000 + PMO_RESUME_TIMEOUT)
#define RSO_STOP_RESPONSE_TIMER        (6000 + PMO_RESUME_TIMEOUT)
#elif defined(QCA_LOWMEM_CONFIG) || defined(QCA_512M_CONFIG) || \
defined(QCA_WIFI_QCA5018)
#define START_RESPONSE_TIMER           15000
#define STOP_RESPONSE_TIMER            15000
#define DELETE_RESPONSE_TIMER          15000
#define PEER_DELETE_ALL_RESPONSE_TIMER 15000
#define RSO_STOP_RESPONSE_TIMER        15000
#else
#define START_RESPONSE_TIMER           8000
#define STOP_RESPONSE_TIMER            6000
#define DELETE_RESPONSE_TIMER          4000
#define PEER_DELETE_ALL_RESPONSE_TIMER 6000
#define RSO_STOP_RESPONSE_TIMER        6000
#endif

#ifdef WLAN_FEATURE_DYNAMIC_MAC_ADDR_UPDATE
#define WLAN_SET_MAC_ADDR_TIMEOUT      START_RESPONSE_TIMER
#endif

/**
 * struct vdev_response_timer - vdev mgmt response ops timer
 * @psoc: Object manager psoc
 * @rsp_timer: VDEV MLME mgmt response timer
 * @rsp_status: variable to check response status
 * @expire_time: time to expire timer
 * @timer_status: status of timer
 * @rsp_timer_inuse: Status bit to inform whether the rsp timer is inuse
 * @vdev_id: vdev object id
 */
struct vdev_response_timer {
	struct wlan_objmgr_psoc *psoc;
	qdf_timer_t rsp_timer;
	unsigned long rsp_status;
	uint32_t expire_time;
	QDF_STATUS timer_status;
	qdf_atomic_t rsp_timer_inuse;
	uint8_t vdev_id;
};

/**
 * struct vdev_start_response - start response structure
 * @vdev_id: vdev id
 * @requestor_id: requester id
 * @status: status of start request
 * @resp_type: response of event type START/RESTART
 * @chain_mask: chain mask
 * @smps_mode: smps mode
 * @mac_id: mac id
 * @cfgd_tx_streams: configured tx streams
 * @cfgd_rx_streams: configured rx streams
 * @max_allowed_tx_power: max tx power allowed
 */
struct vdev_start_response {
	uint8_t vdev_id;
	uint32_t requestor_id;
	uint32_t status;
	uint32_t resp_type;
	uint32_t chain_mask;
	uint32_t smps_mode;
	uint32_t mac_id;
	uint32_t cfgd_tx_streams;
	uint32_t cfgd_rx_streams;
	uint32_t max_allowed_tx_power;
};

/**
 * struct vdev_stop_response - stop response structure
 * @vdev_id: vdev id
 */
struct vdev_stop_response {
	uint8_t vdev_id;
};

/**
 * struct vdev_delete_response - delete response structure
 * @vdev_id: vdev id
 */
struct vdev_delete_response {
	uint8_t vdev_id;
};

/**
 * struct peer_delete_all_response - peer delete all response structure
 * @vdev_id: vdev id
 * @status: FW status for vdev delete all peer request
 */
struct peer_delete_all_response {
	uint8_t vdev_id;
	uint8_t status;
};

/**
 * struct multi_vdev_restart_resp - multi-vdev restart response structure
 * @pdev_id: pdev id
 * @status: FW status for multi vdev restart request
 * @vdev_id_bmap: Bitmap of vdev_ids
 */
struct multi_vdev_restart_resp {
	uint8_t pdev_id;
	uint8_t status;
	qdf_bitmap(vdev_id_bmap, WLAN_UMAC_PSOC_MAX_VDEVS);
};

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * struct vdev_sta_quiet_event - mlo sta quiet offload structure
 * @mld_mac: AP mld mac address
 * @link_mac: AP link mac address
 * @link_id: Link id associated with AP
 * @quiet_status: WMI_QUIET_EVENT_FLAG: quiet start or stop
 */
struct vdev_sta_quiet_event {
	struct qdf_mac_addr mld_mac;
	struct qdf_mac_addr link_mac;
	uint8_t link_id;
	bool quiet_status;
};
#endif
#endif /* __WLAN_VDEV_MGR_TGT_IF_RX_DEFS_H__ */
