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

/**
 * struct atf_peer_cp_stats - ATF statistics
 * @vdev_id: vdev object identifier
 * @tokens: tokens distributed by strictq/fairq
 * @act_tokens: tokens available, after adjustemnt of excess
 * consumed in prev cycle
 * @total: total tokens distributed by strictq/fairq
 * @contribution: tokens contributed by this node
 * @tot_contribution: tokens contributed by all nodes
 * @borrow: tokens borrowed by this node
 * @unused: tokens not used
 * @pkt_drop_nobuf: packets dropped as node is already holding
 * it's share of tx buffers
 * @allowed_bufs: max tx buffers that this node can hold
 * @max_num_buf_held: max tx buffers held by this node
 * @min_num_buf_held: min tx buffers held by this node
 * @num_tx_bufs: packets sent for this node
 * @num_tx_bytes: bytes sent for this node
 * @tokens_common: tokens distributed by strictq/fairq
 * (for non-atf nodes)
 * @act_tokens_common: tokens available, after adjustemnt of
 * excess consumed in prev cycle (for non-atf nodes)
 * @timestamp: time when stats are updated
 * @weighted_unusedtokens_percent: weighted unused tokens percent
 * @raw_tx_tokens: raw tokens
 * @throughput: attainable throughput assuming 100% airtime
 * @total_used_tokens: total of used tokens
 */
struct atf_peer_cp_stats {
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

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_ATF_DEFS_H__ */
