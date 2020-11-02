/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#if !defined(WLAN_HDD_BUS_BANDWIDTH_H)
#define WLAN_HDD_BUS_BANDWIDTH_H
/**
 * DOC: wlan_hdd_bus_bandwidth.h
 *
 * Bus Bandwidth Manager implementation
 */

#include "qca_vendor.h"

/**
 * enum bus_bw_level - bus bandwidth vote levels
 *
 * @BUS_BW_LEVEL_NONE: No vote for bus bandwidth
 * @BUS_BW_LEVEL_1: vote for level-1 bus bandwidth
 * @BUS_BW_LEVEL_2: vote for level-2 bus bandwidth
 * @BUS_BW_LEVEL_3: vote for level-3 bus bandwidth
 * @BUS_BW_LEVEL_4: vote for level-4 bus bandwidth
 * @BUS_BW_LEVEL_5: vote for level-5 bus bandwidth
 * @BUS_BW_LEVEL_6: vote for level-6 bus bandwidth
 */
enum bus_bw_level {
	BUS_BW_LEVEL_NONE,
	BUS_BW_LEVEL_1,
	BUS_BW_LEVEL_2,
	BUS_BW_LEVEL_3,
	BUS_BW_LEVEL_4,
	BUS_BW_LEVEL_5,
	BUS_BW_LEVEL_6,
	BUS_BW_LEVEL_MAX,
};

/**
 * enum tput_level - throughput levels
 *
 * @TPUT_LEVEL_NONE: No throughput
 * @TPUT_LEVEL_IDLE: idle throughtput level
 * @TPUT_LEVEL_LOW: low throughput level
 * @TPUT_LEVEL_MEDIUM: medium throughtput level
 * @TPUT_LEVEL_HIGH: high throughput level
 * @TPUT_LEVEL_VERY_HIGH: very high throughput level
 */
enum tput_level {
	TPUT_LEVEL_NONE,
	TPUT_LEVEL_IDLE,
	TPUT_LEVEL_LOW,
	TPUT_LEVEL_MEDIUM,
	TPUT_LEVEL_HIGH,
	TPUT_LEVEL_VERY_HIGH,
	TPUT_LEVEL_MAX,
};

/**
 * enum bbm_non_per_flag - Non persistent policy related flag
 *
 * @BBM_APPS_RESUME: system resume flag
 * @BBM_APPS_SUSPEND: system suspend flag
 */
enum bbm_non_per_flag {
	BBM_APPS_RESUME,
	BBM_APPS_SUSPEND,
	BBM_FLAG_MAX,
};

/**
 * enum bbm_policy - BBM policy
 *
 * @BBM_DRIVER_MODE_POLICY: driver mode policy
 * @BBM_TPUT_POLICY: throughput policy
 * @BBM_USER_POLICY: user policy
 * @BBM_NON_PERSISTENT_POLICY: non persistent policy. For example, bus resume
 *  sets the bus bw level to LEVEL_3 if any adapter is connected but
 *  this is only a one time setting and is not persistent. This bus bw level
 *  is set without taking other policy vote levels into consideration.
 * @BBM_SELECT_TABLE_POLICY: policy where bus bw table is selected based on
 *  the latency level.
 */
enum bbm_policy {
	BBM_DRIVER_MODE_POLICY,
	BBM_TPUT_POLICY,
	BBM_USER_POLICY,
	BBM_NON_PERSISTENT_POLICY,
	BBM_SELECT_TABLE_POLICY,
	BBM_MAX_POLICY,
};

/**
 * enum wlm_ll_level - WLM latency levels
 *
 * @WLM_LL_NORMAL: normal latency level
 * @WLM_LL_LOW: low latency level
 */
enum wlm_ll_level {
	WLM_LL_NORMAL,
	WLM_LL_LOW,
	WLM_LL_MAX,
};

/**
 * union bbm_policy_info - BBM policy specific info. Only one of the value
 *  would be valid based on the BBM policy.
 *
 * @driver_mode: global driver mode. valid for BBM_DRIVER_MODE_POLICY.
 * @flag: BBM non persistent flag. valid for BBM_NON_PERSISTENT_POLICY.
 * @tput_level: throughput level. valid for BBM_TPUT_POLICY.
 * @wlm_level: latency level. valid for BBM_WLM_POLICY.
 * @user_level: user bus bandwidth vote. valid for BBM_USER_POLICY.
 * @set: set or reset user level. valid for BBM_USER_POLICY.
 */
union bbm_policy_info {
	enum QDF_GLOBAL_MODE driver_mode;
	enum bbm_non_per_flag flag;
	enum tput_level tput_level;
	enum wlm_ll_level wlm_level;
	struct {
		enum bus_bw_level user_level;
		bool set;
	} usr;
};

/**
 * struct bbm_params - BBM params
 *
 * @policy: BBM policy
 * @policy_info: policy related info
 */
struct bbm_params {
	enum bbm_policy policy;
	union bbm_policy_info policy_info;
};

typedef const enum bus_bw_level
	bus_bw_table_type[QCA_WLAN_802_11_MODE_INVALID][TPUT_LEVEL_MAX];

/**
 * struct bbm_context: Bus Bandwidth Manager context
 *
 * @curr_bus_bw_lookup_table: current bus bw lookup table
 * @curr_vote_level: current vote level
 * @per_policy_vote: per BBM policy related vote
 * @bbm_lock: BBM API lock
 */
struct bbm_context {
	bus_bw_table_type *curr_bus_bw_lookup_table;
	enum bus_bw_level curr_vote_level;
	enum bus_bw_level per_policy_vote[BBM_MAX_POLICY];
	qdf_mutex_t bbm_lock;
};

#ifdef FEATURE_BUS_BANDWIDTH_MGR
/**
 * hdd_bbm_context_init() - Initialize BBM context
 * @hdd_ctx: HDD context
 *
 * Returns: error code
 */
int hdd_bbm_context_init(struct hdd_context *hdd_ctx);

/**
 * hdd_bbm_context_deinit() - De-initialize BBM context
 * @hdd_ctx: HDD context
 *
 * Returns: None
 */
void hdd_bbm_context_deinit(struct hdd_context *hdd_ctx);
#else
static inline int hdd_bbm_context_init(struct hdd_context *hdd_ctx)
{
	return 0;
}

static inline void hdd_bbm_context_deinit(struct hdd_context *hdd_ctx)
{
}
#endif
#endif
