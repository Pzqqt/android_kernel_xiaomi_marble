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

#define REG_SBS_SEPARATION_THRESHOLD 100
#define REG_MAX_CHANNELS_PER_OPERATING_CLASS  25
#define REG_MAX_SUPP_OPER_CLASSES 32

/**
 * enum channel_enum - channel enumeration
 * @CHAN_ENUM_1:  channel number 1
 * @CHAN_ENUM_2:  channel number 2
 * @CHAN_ENUM_3:  channel number 3
 * @CHAN_ENUM_4:  channel number 4
 * @CHAN_ENUM_5:  channel number 5
 * @CHAN_ENUM_6:  channel number 6
 * @CHAN_ENUM_7:  channel number 7
 * @CHAN_ENUM_8:  channel number 8
 * @CHAN_ENUM_9:  channel number 9
 * @CHAN_ENUM_10:  channel number 10
 * @CHAN_ENUM_11:  channel number 11
 * @CHAN_ENUM_12:  channel number 12
 * @CHAN_ENUM_13:  channel number 13
 * @CHAN_ENUM_14:  channel number 14
 * @CHAN_ENUM_183:  channel number 183
 * @CHAN_ENUM_184:  channel number 184
 * @CHAN_ENUM_185:  channel number 185
 * @CHAN_ENUM_187:  channel number 187
 * @CHAN_ENUM_188:  channel number 188
 * @CHAN_ENUM_189:  channel number 189
 * @CHAN_ENUM_192:  channel number 192
 * @CHAN_ENUM_196:  channel number 196
 * @CHAN_ENUM_36:  channel number 36
 * @CHAN_ENUM_40:  channel number 40
 * @CHAN_ENUM_44:  channel number 44
 * @CHAN_ENUM_48:  channel number 48
 * @CHAN_ENUM_52:  channel number 52
 * @CHAN_ENUM_56:  channel number 56
 * @CHAN_ENUM_60:  channel number 60
 * @CHAN_ENUM_64:  channel number 64
 * @CHAN_ENUM_100:  channel number 100
 * @CHAN_ENUM_104:  channel number 104
 * @CHAN_ENUM_108:  channel number 108
 * @CHAN_ENUM_112:  channel number 112
 * @CHAN_ENUM_116:  channel number 116
 * @CHAN_ENUM_120:  channel number 120
 * @CHAN_ENUM_124:  channel number 124
 * @CHAN_ENUM_128:  channel number 128
 * @CHAN_ENUM_132:  channel number 132
 * @CHAN_ENUM_136:  channel number 136
 * @CHAN_ENUM_140:  channel number 140
 * @CHAN_ENUM_144:  channel number 144
 * @CHAN_ENUM_149:  channel number 149
 * @CHAN_ENUM_153:  channel number 153
 * @CHAN_ENUM_157:  channel number 157
 * @CHAN_ENUM_161:  channel number 161
 * @CHAN_ENUM_165:  channel number 165
 * @CHAN_ENUM_170:  channel number 170
 * @CHAN_ENUM_171:  channel number 171
 * @CHAN_ENUM_172:  channel number 172
 * @CHAN_ENUM_173:  channel number 173
 * @CHAN_ENUM_174:  channel number 174
 * @CHAN_ENUM_175:  channel number 175
 * @CHAN_ENUM_176:  channel number 176
 * @CHAN_ENUM_177:  channel number 177
 * @CHAN_ENUM_178:  channel number 178
 * @CHAN_ENUM_179:  channel number 179
 * @CHAN_ENUM_180:  channel number 180
 * @CHAN_ENUM_181:  channel number 181
 * @CHAN_ENUM_182:  channel number 182
 * @CHAN_ENUM_183:  channel number 183
 * @CHAN_ENUM_184:  channel number 184
 */
enum channel_enum {
	CHAN_ENUM_1,
	CHAN_ENUM_2,
	CHAN_ENUM_3,
	CHAN_ENUM_4,
	CHAN_ENUM_5,
	CHAN_ENUM_6,
	CHAN_ENUM_7,
	CHAN_ENUM_8,
	CHAN_ENUM_9,
	CHAN_ENUM_10,
	CHAN_ENUM_11,
	CHAN_ENUM_12,
	CHAN_ENUM_13,
	CHAN_ENUM_14,

	CHAN_ENUM_36,
	CHAN_ENUM_40,
	CHAN_ENUM_44,
	CHAN_ENUM_48,
	CHAN_ENUM_52,
	CHAN_ENUM_56,
	CHAN_ENUM_60,
	CHAN_ENUM_64,

	CHAN_ENUM_100,
	CHAN_ENUM_104,
	CHAN_ENUM_108,
	CHAN_ENUM_112,
	CHAN_ENUM_116,
	CHAN_ENUM_120,
	CHAN_ENUM_124,
	CHAN_ENUM_128,
	CHAN_ENUM_132,
	CHAN_ENUM_136,
	CHAN_ENUM_140,
	CHAN_ENUM_144,

	CHAN_ENUM_149,
	CHAN_ENUM_153,
	CHAN_ENUM_157,
	CHAN_ENUM_161,
	CHAN_ENUM_165,

	CHAN_ENUM_170,
	CHAN_ENUM_171,
	CHAN_ENUM_172,
	CHAN_ENUM_173,
	CHAN_ENUM_174,
	CHAN_ENUM_175,
	CHAN_ENUM_176,
	CHAN_ENUM_177,
	CHAN_ENUM_178,
	CHAN_ENUM_179,
	CHAN_ENUM_180,
	CHAN_ENUM_181,
	CHAN_ENUM_182,
	CHAN_ENUM_183,
	CHAN_ENUM_184,

	NUM_CHANNELS,

	MIN_24GHZ_CHANNEL = CHAN_ENUM_1,
	MAX_24GHZ_CHANNEL = CHAN_ENUM_14,
	NUM_24GHZ_CHANNELS = (MAX_24GHZ_CHANNEL - MIN_24GHZ_CHANNEL + 1),

	MIN_5GHZ_CHANNEL = CHAN_ENUM_36,
	MAX_5GHZ_CHANNEL = CHAN_ENUM_184,
	NUM_5GHZ_CHANNELS = (MAX_5GHZ_CHANNEL - MIN_5GHZ_CHANNEL + 1),

	MIN_11P_CHANNEL = CHAN_ENUM_170,
	MAX_11P_CHANNEL = CHAN_ENUM_184,
	NUM_11P_CHANNELS = (MAX_11P_CHANNEL - MIN_11P_CHANNEL + 1),

	INVALID_CHANNEL = 0xBAD,
};


/**
 * enum channel_state - channel state
 * @CHANNEL_STATE_DISABLE: disabled state
 * @CHANNEL_STATE_PASSIVE: passive state
 * @CHANNEL_STATE_DFS: dfs state
 * @CHANNEL_STATE_ENABLE: enabled state
 * @CHANNEL_STATE_INVALID: invalid state
 */
enum channel_state {
	CHANNEL_STATE_DISABLE,
	CHANNEL_STATE_PASSIVE,
	CHANNEL_STATE_DFS,
	CHANNEL_STATE_ENABLE,
	CHANNEL_STATE_INVALID,
};

/**
 * enum reg_domain: reg domain
 * @REGDOMAIN_FCC: FCC domain
 * @REGDOMAIN_ETSI: ETSI domain
 * @REGDOMAIN_JAPAN: JAPAN domain
 * @REGDOMAIN_WORLD: WORLD domain
 * @REGDOMAIN_COUNT: Max domain
 */
typedef enum {
	REGDOMAIN_FCC,
	REGDOMAIN_ETSI,
	REGDOMAIN_JAPAN,
	REGDOMAIN_WORLD,
	REGDOMAIN_COUNT
} v_REGDOMAIN_t;


/**
 * enum phy_ch_width - channel width
 * @CH_WIDTH_20MHZ: 20 mhz width
 * @CH_WIDTH_40MHZ: 40 mhz width
 * @CH_WIDTH_80MHZ: 80 mhz width
 * @CH_WIDTH_160MHZ: 160 mhz width
 * @CH_WIDTH_80P80HZ: 80+80 mhz width
 * @CH_WIDTH_5MHZ: 5 mhz width
 * @CH_WIDTH_10MHZ: 10 mhz width
 * @CH_WIDTH_INVALID: invalid width
 * @CH_WIDTH_MAX: max possible width
 */
enum phy_ch_width {
	CH_WIDTH_20MHZ = 0,
	CH_WIDTH_40MHZ,
	CH_WIDTH_80MHZ,
	CH_WIDTH_160MHZ,
	CH_WIDTH_80P80MHZ,
	CH_WIDTH_5MHZ,
	CH_WIDTH_10MHZ,
	CH_WIDTH_INVALID,
	CH_WIDTH_MAX
};

/**
 * struct ch_params
 * @ch_width: channel width
 * @sec_ch_offset: secondary channel offset
 * @center_freq_seg0: center freq for segment 0
 * @center_freq_seg1: center freq for segment 1
 */
struct ch_params {
	enum phy_ch_width ch_width;
	uint8_t sec_ch_offset;
	uint8_t center_freq_seg0;
	uint8_t center_freq_seg1;
};

/**
 * struct channel_power
 * @chan_num: channel number
 * @tx_power: TX power
 */
struct channel_power {
	uint32_t chan_num;
	uint32_t tx_power;
};

/**
 * enum offset_t: channel offset
 * @BW20: 20 mhz channel
 * @BW40_LOW_PRIMARY: lower channel in 40 mhz
 * @BW40_HIGH_PRIMARY: higher channel in 40 mhz
 * @BW80: 80 mhz channel
 * @BWALL: unknown bandwidth
 */
enum offset_t {
	BW20 = 0,
	BW40_LOW_PRIMARY = 1,
	BW40_HIGH_PRIMARY = 3,
	BW80,
	BWALL
};

/**
 * struct reg_dmn_op_class_map_t: operating class
 * @op_class: operating class number
 * @ch_spacing: channel spacing
 * @offset: offset
 * @channels: channel set
 */
struct reg_dmn_op_class_map_t {
	uint8_t op_class;
	uint8_t ch_spacing;
	enum offset_t offset;
	uint8_t channels[REG_MAX_CHANNELS_PER_OPERATING_CLASS];
};

/**
 * struct reg_dmn_supp_op_classes: operating classes
 * @num_classes: number of classes
 * @classes: classes
 */
struct reg_dmn_supp_op_classes {
	uint8_t num_classes;
	uint8_t classes[REG_MAX_SUPP_OPER_CLASSES];
};

/**
 * enum country_src: country source
 * @SOURCE_QUERY: source query
 * @SOURCE_CORE: source regulatory core
 * @SOURCE_DRIVER: source driver
 * @SOURCE_USERSPACE: source userspace
 * @SOURCE_11D: source 11D
 */
enum country_src {
	SOURCE_UNKNOWN,
	SOURCE_QUERY,
	SOURCE_CORE,
	SOURCE_DRIVER,
	SOURCE_USERSPACE,
	SOURCE_11D
};

/**
 * struct regulatory_channel
 * @center_freq: center frequency
 * @chan_num: channel number
 * @state: channel state
 * @chan_flags: channel flags
 * @tx_power: TX powers
 * @min_bw: min bandwidth
 * @max_bw: max bandwidth
 * @nol_chan: whether channel is nol
 */
struct regulatory_channel {
	uint32_t center_freq;
	uint32_t chan_num;
	enum channel_state state;
	uint32_t chan_flags;
	uint32_t tx_power;
	uint16_t min_bw;
	uint16_t max_bw;
	uint8_t ant_gain;
	bool nol_chan;
};


/**
 * struct regulatory: regulatory information
 * @reg_domain: regulatory domain pair
 * @eeprom_rd_ext: eeprom value
 * @country_code: current country in integer
 * @alpha2: current alpha2
 * @def_country: default country alpha2
 * @def_region: DFS region
 * @ctl_2g: 2G CTL value
 * @ctl_5g: 5G CTL value
 * @reg_pair: pointer to regulatory pair
 * @cc_src: country code src
 * @reg_flags: kernel regulatory flags
 */
struct regulatory {
	uint32_t reg_domain;
	uint32_t eeprom_rd_ext;
	uint16_t country_code;
	uint8_t alpha2[REG_ALPHA2_LEN + 1];
	uint8_t ctl_2g;
	uint8_t ctl_5g;
	const void *regpair;
	enum country_src cc_src;
	uint32_t reg_flags;
};

/**
 * struct chan_map
 * @start_ch: center frequency
 * @chan_num: channel number
 */
struct chan_map {
	uint32_t center_freq;
	uint32_t chan_num;
};

/**
 * struct bonded_channel
 * @start_ch: start channel
 * @end_ch: end channel
 */
struct bonded_channel {
	uint16_t start_ch;
	uint16_t end_ch;
};

/**
 * enum ht_sec_ch_offset
 * @NO_SEC_CH: no secondary
 * @LOW_PRIMARY_CH: low primary
 * @HIGH_PRIMARY_CH: high primary
 */
enum ht_sec_ch_offset {
	NO_SEC_CH = 0,
	LOW_PRIMARY_CH = 1,
	HIGH_PRIMARY_CH = 3,
};


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
