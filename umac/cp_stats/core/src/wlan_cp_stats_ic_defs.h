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
 * DOC:	wlan_cp_stats_ic_defs.h
 *
 * This header file maintain structure definitions for cp stats specific to ic
 */

#ifndef __WLAN_CP_STATS_IC_DEFS_H__
#define __WLAN_CP_STATS_IC_DEFS_H__

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_cp_stats_ic_atf_defs.h>
#include <wlan_cp_stats_ic_arpdbg_defs.h>
#include <wlan_cp_stats_ic_vow_defs.h>
#include <wlan_cp_stats_ic_rrm_bcn_defs.h>
#include <ieee80211_rrm.h>

/**
 * struct pdev_ic_cp_stats - control plane stats specific to WIN at pdev
 * @rx_mgmt:		rx management frames
 * @tx_mgmt:		tx management frames
 * @tx_bcn:		tx beacon frames
 * @rx_phyofdmerr:	rx PHY error
 * @rx_phycckerr:	rx PHY error
 * @tx_fcs_err:		tx fcs error
 * @vow_stats:		vow ext stats
 * @arp_dbg_cs:		arp dbg stats
 */
struct pdev_ic_cp_stats {
	uint64_t	rx_mgmt;
	uint64_t	tx_mgmt;
	uint64_t	tx_bcn;
	uint64_t	rx_phyofdmerr;
	uint64_t	rx_phycckerr;
	uint64_t	tx_fcs_err;
	struct vow_extstats vow_stats;
	struct arp_dbg_stats arp_dbg_cs;
};

/**
 * struct vdev_ic_cp_stats - control plane stats specific to WIN at vdev
 * @tx_success:		total tx success
 * @tx_failure:		total tx failures
 * @rx_mgmt:		rx mgmt frames
 * @tx_mgmt:		tx mgmt frames
 * @rx_ctrl:		rx control frames
 * @tx_ctrl:		tx control frames
 * @tx_bcn_swba:	beacon intr SWBA counter
 * @tx_offchan_mgmt:	tx offchan mgmt success
 * @tx_offchan_date:	tx offchan data success
 * @tx_offchan_fail:	tx offchan total failures
 * @atf_cs:		atf stats
 * @rrm_bcnreq_info:	beacon report
 */
struct vdev_ic_cp_stats {
	uint64_t	tx_success;
	uint64_t	tx_failure;
	uint64_t	rx_mgmt;
	uint64_t	tx_mgmt;
	uint64_t	rx_ctrl;
	uint64_t	tx_ctrl;
	uint64_t	tx_bcn_swba;
	uint64_t	tx_offchan_mgmt;
	uint64_t	tx_offchan_data;
	uint64_t	tx_offchan_fail;
	struct atf_cp_stats atf_cs;
	ieee80211_rrm_beaconreq_info_t rrm_bcnreq_info;
};

/**
 * struct peer_ic_cp_stats - control plane stats specific to WIN at peer
 * @rx_mgmt_rate:	rx mgmt rate
 * @rx_mgmt_rssi:	rx mgmt rssi
 */
struct peer_ic_cp_stats {
	uint32_t	rx_mgmt_rate;
	int8_t		rx_mgmt_rssi;
};

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_DEFS_H__ */
