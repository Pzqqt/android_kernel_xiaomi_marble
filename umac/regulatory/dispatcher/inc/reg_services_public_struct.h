/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: reg_services_public_struct.h
 * This file contains regulatory data structures
 */

#ifndef __REG_SERVICES_PUBLIC_STRUCT_H_
#define __REG_SERVICES_PUBLIC_STRUCT_H_

#include "../../core/src/reg_db.h"

/**
 * struct cur_reg_rule
 * @start_freq: start frequency
 * @end_freq: end frequency
 * @max_bw: maximum bandwidth
 * @reg_power: regulatory power
 * @ant_gain: antenna gain
 * @flags: regulatory flags
 */
struct cur_reg_rule {
	uint16_t start_freq;
	uint16_t end_freq;
	uint16_t max_bw;
	uint8_t reg_power;
	uint8_t ant_gain;
	uint16_t flags;
};

/**
 * struct cur_regulatory_info
 * @psoc: psoc ptr
 * @alpha2: country alpha2
 * @dfs_reg: dfs region
 * @phybitmap: phy bit map
 * @min_bw_2g: minimum 2G bw
 * @max_bw_2g: maximum 2G bw
 * @min_bw_5g: minimum 5G bw
 * @max_bw_5g: maximum 5G bw
 * @num_2g_reg_rules: number 2G reg rules
 * @num_5g_reg_rules: number 5G reg rules
 * @reg_rules_2g_ptr: ptr to 2G reg rules
 * @reg_rules_5g_ptr: ptr to 5G reg rules
 */
struct cur_regulatory_info {
	struct wlan_objmgr_psoc *psoc;
	uint8_t alpha2[REG_ALPHA2_LEN + 1];
	enum dfs_reg dfs_region;
	uint32_t phybitmap;
	uint32_t min_bw_2g;
	uint32_t max_bw_2g;
	uint32_t min_bw_5g;
	uint32_t max_bw_5g;
	uint32_t num_2g_reg_rules;
	uint32_t num_5g_reg_rules;
	struct cur_reg_rule *reg_rules_2g_ptr;
	struct cur_reg_rule *reg_rules_5g_ptr;
};

/**
 * enum band_info
 * @BAND_ALL:all bands
 * @BAND_2G: 2G band
 * @BAND_5G: 5G band
 */
enum band_info {
	BAND_ALL,
	BAND_2G,
	BAND_5G,
};

/**
 * struct reg_config_vars
 * @enable_11d_support: enable 11d support
 * @userspace_ctry_priority: user priority
 * @band_capability: band capability
 * @dfs_disable: dfs disabled
 * @indoor_channel_support: indoor channel support
 */
struct reg_config_vars {
	uint32_t enable_11d_support;
	uint32_t userspace_ctry_priority;
	enum band_info band_capability;
	uint32_t dfs_enabled;
	uint32_t indoor_chan_enabled;
};

struct set_band_req {
	enum band_info band;
	uint32_t pdev_id;
};

struct country_info {
	uint8_t country_code[REG_ALPHA2_LEN + 1];
};

struct reg_country_update {
	uint8_t country_code[REG_ALPHA2_LEN + 1];
};

/**
 * struct reg_freq_range
 * @low_freq: low frequency
 * @high_freq: high frequency
 */
struct reg_freq_range {
	uint32_t low_freq;
	uint32_t high_freq;
};

#endif
