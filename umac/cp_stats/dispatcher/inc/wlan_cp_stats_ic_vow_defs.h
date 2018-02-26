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
 * DOC: wlan_cp_stats_ic_vow_defs.h
 *
 * This header file provide structure definitions vow ext stats
 */

#ifndef __WLAN_CP_STATS_IC_VOW_DEFS_H__
#define __WLAN_CP_STATS_IC_VOW_DEFS_H__

#ifdef QCA_SUPPORT_CP_STATS

/**
 * struct vow_extstats - vow extension structure
 * @rx_rssi_ctl0: control channel chain0 rssi
 * @rx_rssi_ctl1: control channel chain1 rssi
 * @rx_rssi_ctl2: control channel chain2 rssi
 * @rx_rssi_ext0: extension channel chain0 rssi
 * @rx_rssi_ext1: extension channel chain1  rssi
 * @rx_rssi_ext2: extension channel chain2 rssi
 * @rx_rssi_comb: combined RSSI value
 * @rx_bw: Band width 0-20, 1-40, 2-80
 * @rx_sgi: Guard interval, 0-Long GI, 1-Short GI
 * @rx_nss: Number of spatial streams
 * @rx_mcs: Rate MCS value
 * @rx_ratecode: Hardware rate code
 * @rx_rs_flags: Receive misc flags
 * @rx_moreaggr: 0 - non aggr frame
 * @rx_mac_ts: Time stamp
 * @rx_seqno: rx sequence number
 */
struct vow_extstats {
	uint8_t		rx_rssi_ctl0;
	uint8_t		rx_rssi_ctl1;
	uint8_t		rx_rssi_ctl2;
	uint8_t		rx_rssi_ext0;
	uint8_t		rx_rssi_ext1;
	uint8_t		rx_rssi_ext2;
	uint8_t		rx_rssi_comb;
	uint8_t		rx_bw;
	uint8_t		rx_sgi;
	uint8_t		rx_nss;
	uint8_t		rx_mcs;
	uint8_t		rx_ratecode;
	uint8_t		rx_rs_flags;
	uint8_t		rx_moreaggr;
	uint32_t	rx_mac_ts;
	uint16_t	rx_seqno;
};

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_VOW_DEFS_H__ */
