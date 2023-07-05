/*
 * Copyright (c) 2020-2021 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_mesh_latency.h
 * @brief Define the host data path MESH latency API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_MESH_LATENCY_H_
#define _CDP_TXRX_MESH_LATENCY_H_
#include "cdp_txrx_handle.h"
#ifdef WLAN_SUPPORT_MESH_LATENCY
/**
 * @brief find MSCS enabled peer for this mac address and validate priority
 * @details
 *  This function updates per peer per TID mesh latency related parameters.
 *
 * @param dest_mac - destination mac address
 * @param service_interval_dl - Service Interval per tid on DL
 * @param burst_size_dl - Burst size per tid on DL
 * @param service_interval_ul - Service Interval per tid on UL
 * @param burst_size_ul - Burst size per tid on UL
 * @param priority - user priority combination of tid and msdu queue
 * @add_or_sub - indicates to add or subtract latency parameter
 * @return - 0 for non error case, -1 for failure
 */
static inline QDF_STATUS
cdp_mesh_latency_update_peer_parameter(ol_txrx_soc_handle soc,
	uint8_t *dest_mac, uint32_t service_interval_dl,
	uint32_t burst_size_dl, uint32_t service_interval_ul,
	uint32_t burst_size_ul,	uint16_t priority,
	uint8_t add_or_sub)
{
	if (!soc || !soc->ops || !soc->ops->mesh_latency_ops) {
		return 1;
	}

	if (soc->ops->mesh_latency_ops->mesh_latency_update_peer_parameter)
		return soc->ops->mesh_latency_ops->
				mesh_latency_update_peer_parameter(soc,
						dest_mac, service_interval_dl,
						burst_size_dl, service_interval_ul,
						burst_size_ul, priority, add_or_sub);
	return 0;
}
#endif
#endif
