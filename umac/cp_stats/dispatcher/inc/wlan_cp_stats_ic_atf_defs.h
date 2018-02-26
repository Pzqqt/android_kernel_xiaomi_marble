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
 * DOC: wlan_cp_stats_atf_defs.h
 *
 * This header file provides structure definitions to ATF control plane stats
 */

#ifndef __WLAN_CP_STATS_ATF_DEFS_H__
#define __WLAN_CP_STATS_ATF_DEFS_H__

#ifdef QCA_SUPPORT_CP_STATS
#include <qdf_status.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_atf_utils_defs.h>

/**
 * struct atf_cp_stats - ATF statistics
 * @tokens: tokens distributed by strictq/fairq
 * @act_tokens: tokens available, after adjustemnt of
 *			excess consumed in prev cycle
 * @total: total tokens distributed by strictq/fairq
 * @contribution: tokens contributed by this node
 * @tot_contribution: tokens contributed by all nodes
 * @borrow: tokens borrowed by this node
 * @unused: tokens not used
 * @pkt_drop_nobuf: packets dropped as node is already holding
 *			it's share of tx buffers
 * @allowed_bufs: max tx buffers that this node can hold
 * @max_num_buf_held: max tx buffers held by this node
 * @min_num_buf_held: min tx buffers held by this node
 * @num_tx_bufs: packets sent for this node
 * @num_tx_bytes: bytes sent for this node
 * @tokens_common: tokens distributed by strictq/fairq
 *			(for non-atf nodes)
 * @act_tokens_common: tokens available, after adjustemnt of
 *			excess consumed in prev cycle
 *			(for non-atf nodes)
 * @timestamp: time when stats are updated
 * @weighted_unusedtokens_percent: weighted unused tokens percent
 * @raw_tx_tokens: raw tokens
 * @throughput: attainable throughput assuming 100% airtime
 * @total_used_tokens: total of used tokens
 */
struct atf_cp_stats {
	uint8_t		vdev_id;
	uint32_t	tokens;
	uint32_t	act_tokens;
	uint32_t	total;
	uint32_t	contribution;
	uint32_t	tot_contribution;
	uint32_t	borrow;
	uint32_t	unused;
	uint32_t	pkt_drop_nobuf;
	uint16_t	allowed_bufs;
	uint16_t	max_num_buf_held;
	uint16_t	min_num_buf_held;
	uint16_t	num_tx_bufs;
	uint32_t	num_tx_bytes;
	uint32_t	tokens_common;
	uint32_t	act_tokens_common;
	uint32_t	timestamp;
	uint32_t	weighted_unusedtokens_percent;
	uint32_t	raw_tx_tokens;
	uint32_t	throughput;
	uint64_t	total_used_tokens;
};

/**
 * struct atfcntbl - Structure for ATF config table element
 * @ssid: SSID
 * @sta_mac: STA MAC address
 * @value: value to configure
 * @info_mark: 1 -- STA, 0 -- VDEV
 * @assoc_status: 1 -- Yes, 0 -- No
 * @all_tokens_used: 1 -- Yes, 0 -- No
 * @cfg_value: Config value
 * @grpname: Group name
 */
struct atfcntbl_cs {
	uint8_t		ssid[SSID_MAX_LEN + 1];
	uint8_t		sta_mac[MAC_ADDR_SIZE];
	uint32_t	value;
	uint8_t		info_mark;
	uint8_t		assoc_status;
	uint8_t		all_tokens_used;
	uint32_t	cfg_value;
	uint8_t		grpname[SSID_MAX_LEN + 1];
};

/**
 * struct atftable - ATF table
 * @id_type: Sub command
 * @atf_info: Array of atfcntbl
 * @info_cnt: Count of info entry
 * @busy: Busy state flag
 * @atf_group: Group ID
 * @show_per_peer_table: Flag to show per peer table
 */
struct atftable_cs {
	uint16_t	id_type;
	struct atfcntbl	atf_info[ATF_ACTIVED_MAX_CLIENTS + ATF_CFG_NUM_VDEV];
	uint16_t	info_cnt;
	uint8_t		atf_status;
	uint32_t	busy;
	uint32_t	atf_group;
};

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_ATF_DEFS_H__ */
