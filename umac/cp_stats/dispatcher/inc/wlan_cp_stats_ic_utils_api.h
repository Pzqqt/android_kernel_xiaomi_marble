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
 * DOC: wlan_cp_stats_ic_utils_api.h
 *
 * This header file provide definitions and declarations required for northbound
 * specific to WIN
 */

#ifndef __WLAN_CP_STATS_IC_UTILS_API_H__
#define __WLAN_CP_STATS_IC_UTILS_API_H__

#ifdef QCA_SUPPORT_CP_STATS
#include "wlan_cp_stats_ic_ucfg_api.h"

#define PEER_CP_STATS_SET_FUNCS(field) \
	static inline void \
	peer_cp_stats_##field##_inc(struct wlan_objmgr_peer *_peer, \
				    uint64_t _val) \
	{ \
		ucfg_peer_cp_stats_##field##_inc(_peer, _val); \
	} \
	static inline void \
	peer_cp_stats_##field##_dec(struct wlan_objmgr_peer *_peer, \
				    uint64_t _val) \
	{ \
		ucfg_peer_cp_stats_##field##_inc(_peer, _val); \
	} \
	static inline void \
	peer_cp_stats_##field##_update(struct wlan_objmgr_peer *_peer, \
				       uint64_t _val) \
	{ \
		ucfg_peer_cp_stats_##field##_update(_peer, _val); \
	}

PEER_CP_STATS_SET_FUNCS(rx_mgmt);
PEER_CP_STATS_SET_FUNCS(tx_mgmt);
PEER_CP_STATS_SET_FUNCS(rx_mgmt_rate);
PEER_CP_STATS_SET_FUNCS(is_tx_not_ok);
PEER_CP_STATS_SET_FUNCS(rx_noprivacy);
PEER_CP_STATS_SET_FUNCS(rx_wepfail);
PEER_CP_STATS_SET_FUNCS(rx_tkipicv);
PEER_CP_STATS_SET_FUNCS(rx_wpimic);
PEER_CP_STATS_SET_FUNCS(rx_ccmpmic);
PEER_CP_STATS_SET_FUNCS(ps_discard);
PEER_CP_STATS_SET_FUNCS(psq_drops);
PEER_CP_STATS_SET_FUNCS(tx_assoc);
PEER_CP_STATS_SET_FUNCS(tx_assoc_fail);
#ifdef ATH_SUPPORT_IQUE
PEER_CP_STATS_SET_FUNCS(tx_dropblock);
#endif
#ifdef WLAN_ATH_SUPPORT_EXT_STAT
PEER_CP_STATS_SET_FUNCS(tx_bytes_rate);
PEER_CP_STATS_SET_FUNCS(rx_bytes_rate);
PEER_CP_STATS_SET_FUNCS(tx_data_rate);
PEER_CP_STATS_SET_FUNCS(rx_data_rate);
PEER_CP_STATS_SET_FUNCS(rx_bytes_last);
PEER_CP_STATS_SET_FUNCS(rx_data_last);
PEER_CP_STATS_SET_FUNCS(tx_bytes_success_last);
PEER_CP_STATS_SET_FUNCS(tx_data_success_last);
#endif

#define PEER_CP_STATS_GET_FUNCS(field) \
	static inline uint64_t \
	peer_cp_stats_##field##_get(struct wlan_objmgr_peer *_peer) \
	{ \
		return ucfg_peer_cp_stats_##field##_get(_peer); \
	}

PEER_CP_STATS_GET_FUNCS(rx_mgmt_rate);
#ifdef ATH_SUPPORT_IQUE
PEER_CP_STATS_GET_FUNCS(tx_dropblock);
#endif

static inline void
peer_cp_stats_rx_mgmt_rssi_update(struct wlan_objmgr_peer *peer,
				  int8_t rssi)
{
	ucfg_peer_cp_stats_rx_mgmt_rssi_update(peer, rssi);
}

static inline int8_t
peer_cp_stats_rx_mgmt_rssi_get(struct wlan_objmgr_peer *peer)
{
	return ucfg_peer_cp_stats_rx_mgmt_rssi_get(peer);
}

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_UTILS_API_H__ */
