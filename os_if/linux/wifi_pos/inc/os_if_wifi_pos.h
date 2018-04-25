/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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
 * DOC: os_if_wifi_pos.h
 * This file provide declaration of wifi_pos's os_if APIs
 */
#ifndef _OS_IF_WIFI_POS_H_
#define _OS_IF_WIFI_POS_H_

#include "qdf_types.h"
#include "qdf_status.h"


/* forward declaration */
struct wifi_pos_ch_info;
struct wlan_objmgr_psoc;
struct wifi_pos_driver_caps;

#ifdef WIFI_POS_CONVERGED
/**
 * os_if_wifi_pos_register_nl() - abstration API to register callback with GENL
 * socket.
 *
 * Return: status of operation
 */
int os_if_wifi_pos_register_nl(void);

/**
 * os_if_wifi_pos_deregister_nl() - abstration API to deregister callback with
 * GENL socket.
 *
 * Return: status of operation
 */
int os_if_wifi_pos_deregister_nl(void);

/**
 * os_if_wifi_pos_send_peer_status() - Function to send peer status to a
 * registered application
 * @peer_mac: MAC address of peer
 * @peer_status: ePeerConnected or ePeerDisconnected
 * @peer_timing_meas_cap: 0: RTT/RTT2, 1: RTT3. Default is 0
 * @session_id: SME session id, i.e. vdev_id
 * @chan_info: operating channel information
 * @dev_mode: dev mode for which indication is sent
 *
 * Return: none
 */
void os_if_wifi_pos_send_peer_status(struct qdf_mac_addr *peer_mac,
				uint8_t peer_status,
				uint8_t peer_timing_meas_cap,
				uint8_t session_id,
				struct wifi_pos_ch_info *chan_info,
				enum QDF_OPMODE dev_mode);

/**
 * os_if_wifi_pos_populate_caps() - populate oem capabilities
 * @psoc: psoc object
 * @caps: pointer to populate the capabilities
 *
 * Return: error code
 */
int os_if_wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
				struct wifi_pos_driver_caps *caps);
#else
static inline int os_if_wifi_pos_register_nl(void)
{
	return 0;
}

static inline int os_if_wifi_pos_deregister_nl(void)
{
	return 0;
}

static inline void os_if_wifi_pos_send_peer_status(
		struct qdf_mac_addr *peer_mac,
		uint8_t peer_status,
		uint8_t peer_timing_meas_cap,
		uint8_t session_id,
		struct wifi_pos_ch_info *chan_info,
		enum QDF_OPMODE dev_mode)
{
}

static inline int os_if_wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
					struct wifi_pos_driver_caps *caps)
{
	return 0;
}
#endif

#endif /* _OS_IF_WIFI_POS_H_ */
