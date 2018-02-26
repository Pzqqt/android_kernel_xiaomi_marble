/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cp_stats_ic_arpdbg_defs.h
 *
 * This header file provide structure definitions for arp dbg stats
 */

#ifndef __WLAN_CP_STATS_IC_ARPDBG_DEFS_H__
#define __WLAN_CP_STATS_IC_ARPDBG_DEFS_H__
#ifdef QCA_SUPPORT_CP_STATS

/**
 * struct arp_dbg_stats - defines arp debug stats
 * @tx_arp_req_count: tx arp req count
 * @tx_arp_resp_count: tx arp response count
 * @rx_arp_req_count: rx arp req count
 * @rx_arp_resp_count: rx arp response count
 */
struct arp_dbg_stats {
	uint32_t	tx_arp_req_count; /* tx arp request counters */
	uint32_t	rx_arp_req_count; /* rx arp request counters  */
	uint32_t	tx_arp_resp_count; /* tx arp response counters  */
	uint32_t	rx_arp_resp_count; /* rx arp response counters  */
};

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_ARPDBG_DEFS_H__ */
