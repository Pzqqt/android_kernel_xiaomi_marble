
/*
 * Copyright (c) 2021 The Linux Foundation. All rights reserved.
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

#ifndef _DP_MESH_LATENCY_H_
#define _DP_MESH_LATENCY_H_

#if WLAN_SUPPORT_MESH_LATENCY

/**
 * dp_mesh_latency_service_interval - service interval values
 * @DP_MESH_LATENCY_SERVICE_INTERVAL_10MS: 10ms service interval
 * @DP_MESH_LATENCY_SERVICE_INTERVAL_20MS: 20ms service interval
 * @DP_MESH_LATENCY_SERVICE_INTERVAL_40MS: 40ms service interval
 * @DP_MESH_LATENCY_SERVICE_INTERVAL_80MS: 80ms service interval
 */
enum dp_mesh_latency_service_interval {
	DP_MESH_LATENCY_SERVICE_INTERVAL_10MS = 10,
	DP_MESH_LATENCY_SERVICE_INTERVAL_20MS = 20,
	DP_MESH_LATENCY_SERVICE_INTERVAL_40MS = 40,
	DP_MESH_LATENCY_SERVICE_INTERVAL_80MS = 80,
};

QDF_STATUS dp_mesh_latency_update_peer_parameter(struct cdp_soc_t *soc_hdl,
				    uint8_t *dest_mac, uint32_t service_interval,
					uint32_t burst_size, uint16_t priority,
					uint8_t add_or_sub);
#endif

#endif /* DP_MESH_LATENCY_H*/
