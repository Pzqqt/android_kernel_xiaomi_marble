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
 * DOC: wlan_vdev_mgr_tgt_if_rx_defs.h
 *
 * This header file provides definitions to data structures for
 * corresponding vdev mgmt operation
 */

#ifndef __WLAN_VDEV_MGR_TGT_IF_RX_DEFS_H__
#define __WLAN_VDEV_MGR_TGT_IF_RX_DEFS_H__

#include <qdf_timer.h>

#define START_RESPONSE_BIT  0x1
#define RESTART_RESPONSE_BIT  0x2
#define STOP_RESPONSE_BIT   0x3
#define DELETE_RESPONSE_BIT 0x4
#define RESPONSE_BIT_MAX (START_RESPONSE_BIT | RESTART_RESPONSE_BIT |\
			  STOP_RESPONSE_BIT | DELETE_RESPONSE_BIT)

#define START_RESPONSE_TIMER 6000 /* 6 seconds */
#define STOP_RESPONSE_TIMER  3000 /* 3 seconds */
#define DELETE_RESPONSE_TIMER  3000 /* 3 seconds */

/**
 * struct vdev_response_timer - vdev mgmt response ops timer
 * @rsp_timer: VDEV MLME mgmt response timer
 * @rsp_status: variable to check response status
 * @expire_time: time to expire timer
 * @timer_status: status of timer
 */
struct vdev_response_timer {
	qdf_timer_t rsp_timer;
	unsigned long rsp_status;
	uint32_t expire_time;
	QDF_STATUS timer_status;
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

#endif /* __WLAN_VDEV_MGR_TGT_IF_RX_DEFS_H__ */
