/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

/*
 * This file contains the API definitions for the Unified Wireless Module
 * Interface (WMI).
 */

#ifndef _WMI_UNIFIED_ATF_PARAM_H_
#define _WMI_UNIFIED_ATF_PARAM_H_

#define ATF_ACTIVED_MAX_CLIENTS   50
#define ATF_ACTIVED_MAX_ATFGROUPS 16

/**
 * struct atf_peer_info - ATF peer info params
 * @peer_macaddr: peer mac addr
 * @percentage_peer: percentage of air time for this peer
 * @vdev_id: Associated vdev id
 * @pdev_id: Associated pdev id
 */
typedef struct {
	struct wmi_macaddr_t peer_macaddr;
	uint32_t percentage_peer;
	uint32_t vdev_id;
	uint32_t pdev_id;
} atf_peer_info;

/**
 * struct bwf_peer_info_t - BWF peer info params
 * @peer_macaddr: peer mac addr
 * @throughput: Throughput
 * @max_airtime: Max airtime
 * @priority: Priority level
 * @reserved: Reserved array
 * @vdev_id: Associated vdev id
 * @pdev_id: Associated pdev id
 */
typedef struct {
	struct wmi_macaddr_t peer_macaddr;
	uint32_t     throughput;
	uint32_t     max_airtime;
	uint32_t     priority;
	uint32_t     reserved[4];
	uint32_t     vdev_id;
	uint32_t     pdev_id;
} bwf_peer_info;

/**
 * struct set_bwf_params - BWF params
 * @num_peers: number of peers
 * @atf_peer_info: BWF peer info
 */
struct set_bwf_params {
	uint32_t num_peers;
	bwf_peer_info peer_info[1];
};

/**
 * struct atf_peer_ext_info - ATF peer ext info params
 * @peer_macaddr: peer mac address
 * @group_index: group index
 * @atf_index_reserved: ATF index rsvd
 * @vdev_id: Associated vdev id
 * @pdev_id: Associated pdev id
 */
typedef struct {
	struct wmi_macaddr_t peer_macaddr;
	uint32_t group_index;
	uint32_t atf_index_reserved;
	uint16_t vdev_id;
	uint16_t pdev_id;
} atf_peer_ext_info;

/**
 * struct set_atf_params - ATF params
 * @num_peers: number of peers
 * @atf_peer_info: ATF peer info
 */
struct set_atf_params {
	uint32_t num_peers;
	atf_peer_info peer_info[ATF_ACTIVED_MAX_CLIENTS];
};

/**
 * struct atf_peer_request_params - ATF peer req params
 * @num_peers: number of peers
 * @pdev_id: Associated pdev id
 * @atf_peer_ext_info: ATF peer ext info
 */
struct atf_peer_request_params {
	uint32_t num_peers;
	uint32_t pdev_id;
	atf_peer_ext_info peer_ext_info[ATF_ACTIVED_MAX_CLIENTS];
};

/**
 * struct atf_group_info - ATF group info params
 * @percentage_group: Percentage AT for group
 * @atf_group_units_reserved: ATF group information
 * @pdev_id: Associated pdev id
 */
typedef struct {
	uint32_t percentage_group;
	uint32_t atf_group_units_reserved;
	uint32_t pdev_id;
} atf_group_info;

/**
 * struct atf_grouping_params - ATF grouping params
 * @num_groups: number of groups
 * @pdev_id: Associated pdev id
 * @group_inf: Group informaition
 */
struct atf_grouping_params {
	uint32_t num_groups;
	uint32_t pdev_id;
	atf_group_info group_info[ATF_ACTIVED_MAX_ATFGROUPS];
};

/**
 * struct atf_group_wmm_ac_info - ATF group AC info params
 * @atf_config_ac_be: Relative ATF% for BE traffic
 * @atf_config_ac_bk: Relative ATF% for BK traffic
 * @atf_config_ac_vi: Relative ATF% for VI traffic
 * @atf_config_ac_vo: Relative ATF% for VO traffic
 * @reserved: Reserved for future use
 */
struct atf_group_wmm_ac_info {
	uint32_t  atf_config_ac_be;
	uint32_t  atf_config_ac_bk;
	uint32_t  atf_config_ac_vi;
	uint32_t  atf_config_ac_vo;
	uint32_t reserved[2];
};

/**
 * struct atf_grp_ac_params - ATF group AC config params
 * @num_groups: number of groups
 * @pdev_id: Associated pdev id
 * @group_inf: Group informaition
 */
struct atf_group_ac_params {
	uint32_t num_groups;
	uint32_t pdev_id;
	struct atf_group_wmm_ac_info group_info[ATF_ACTIVED_MAX_ATFGROUPS];
};

enum {
	WMI_HOST_ATF_PEER_STATS_DISABLED = 0,
	WMI_HOST_ATF_PEER_STATS_ENABLED  = 1,
};

#define WMI_HOST_ATF_PEER_STATS_GET_PEER_AST_IDX(token_info) \
	(token_info.field1 & 0xffff)

#define WMI_HOST_ATF_PEER_STATS_GET_USED_TOKENS(token_info) \
	((token_info.field2 & 0xffff0000) >> 16)

#define WMI_HOST_ATF_PEER_STATS_GET_UNUSED_TOKENS(token_info) \
	(token_info.field2 & 0xffff)

#define WMI_HOST_ATF_PEER_STATS_SET_PEER_AST_IDX(token_info, peer_ast_idx) \
	do { \
		token_info.field1 &= 0xffff0000; \
		token_info.field1 |= ((peer_ast_idx) & 0xffff); \
	} while (0)

#define WMI_HOST_ATF_PEER_STATS_SET_USED_TOKENS(token_info, used_token) \
	do { \
		token_info.field2 &= 0x0000ffff; \
		token_info.field2 |= (((used_token) & 0xffff) << 16); \
	} while (0)

#define WMI_HOST_ATF_PEER_STATS_SET_UNUSED_TOKENS(token_info, unused_token) \
	do { \
		token_info.field2 &= 0xffff0000; \
		token_info.field2 |= ((unused_token) & 0xffff); \
	} while (0)

/**
 * struct wmi_host_atf_peer_stats_info
 * @field1: bits 15:0   peer_ast_index  WMI_ATF_PEER_STATS_GET_PEER_AST_IDX
 *          bits 31:16  reserved
 * @field2: bits 15:0   used tokens     WMI_ATF_PEER_STATS_GET_USED_TOKENS
 *          bits 31:16  unused tokens   WMI_ATF_PEER_STATS_GET_UNUSED_TOKENS
 * @field3: for future use
 */
typedef struct {
	uint32_t    field1;
	uint32_t    field2;
	uint32_t    field3;
} wmi_host_atf_peer_stats_info;

/**
 * struct wmi_host_atf_peer_stats_event
 * @pdev_id: pdev_id
 * @num_atf_peers: number of peers in token_info_list
 * @comp_usable_airtime: computed usable airtime in tokens
 * @reserved[4]: reserved for future use
 * @wmi_host_atf_peer_stats_info token_info_list: list of num_atf_peers
 */
typedef struct {
	uint32_t pdev_id;
	uint32_t num_atf_peers;
	uint32_t comp_usable_airtime;
	uint32_t reserved[4];
	wmi_host_atf_peer_stats_info token_info_list[1];
} wmi_host_atf_peer_stats_event;

#endif /* _WMI_UNIFIED_ATF_PARAM_H_ */
