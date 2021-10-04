/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#ifndef _DP_MON_1_0_H_
#define _DP_MON_1_0_H_

#ifdef WLAN_RX_PKT_CAPTURE_ENH
#include <dp_rx_mon_feature.h>
#endif

#include <dp_rx_mon.h>

void dp_flush_monitor_rings(struct dp_soc *soc);

/* MCL specific functions */
#if defined(DP_CON_MON)

/*
 * dp_service_mon_rings()- service monitor rings
 * @soc: soc dp handle
 * @quota: number of ring entry that can be serviced
 *
 * Return: None
 *
 */
void dp_service_mon_rings(struct  dp_soc *soc, uint32_t quota);
#endif

/**
 * dp_mon_drop_packets_for_mac() - Drop the mon status ring and
 *  dest ring packets for a given mac. Packets in status ring and
 *  dest ring are dropped independently.
 * @pdev: DP pdev
 * @mac_id: mac id
 * @quota: max number of status ring entries that can be processed
 *
 * Return: work done
 */
uint32_t dp_mon_drop_packets_for_mac(struct dp_pdev *pdev, uint32_t mac_id,
				     uint32_t quota);

#endif /* _DP_MON_1_0_H_ */
