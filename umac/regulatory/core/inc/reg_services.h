/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
 *
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
 * DOC: reg_services.h
 * This file provides prototypes of the regulatory component
 * service functions
 */

enum channel_state {
	CHANNEL_STATE_DISABLE,
	CHANNEL_STATE_PASSIVE,
	CHANNEL_STATE_DFS,
	CHANNEL_STATE_ENABLE,
	CHANNEL_STATE_INVALID,
};

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

struct regulatory_channel {
	enum channel_state state;
	uint32_t chan_flags;
	uint32_t tx_power;
};

struct channel_power {
	uint32_t chan_num;
	uint32_t tx_power;
};

struct chan_map {
	uint32_t center_freq;
	uint32_t chan_num;
};


enum channel_enum {
	CHAN_ENUM_1 = 1,
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

	CHAN_ENUM_183,
	CHAN_ENUM_184,
	CHAN_ENUM_185,
	CHAN_ENUM_186,
	CHAN_ENUM_187,
	CHAN_ENUM_188,
	CHAN_ENUM_189,
	CHAN_ENUM_190,
	CHAN_ENUM_191,
	CHAN_ENUM_192,
	CHAN_ENUM_193,
	CHAN_ENUM_194,
	CHAN_ENUM_195,
	CHAN_ENUM_196,

	NUM_CHANNELS,

	MIN_24GHZ_CHANNEL = CHAN_ENUM_1,
	MAX_24GHZ_CHANNEL = CHAN_ENUM_14,
	NUM_24GHZ_CHANNELS = (MAX_24GHZ_CHANNEL - MIN_24GHZ_CHANNEL + 1),

	MIN_5GHZ_CHANNEL = CHAN_ENUM_36,
	MAX_5GHZ_CHANNEL = CHAN_ENUM_184,
	NUM_5GHZ_CHANNELS = (MAX_5GHZ_CHANNEL - MIN_5GHZ_CHANNEL + 1),

	MIN_49GHZ_CHANNEL = CHAN_ENUM_183,
	MAX_49GHZ_CHANNEL = CHAN_ENUM_196,

	INVALID_CHANNEL = 0xBAD,
};

enum band_info {
	band_2g = 0x1,
	band_49g = 0x2,
	band_5g_36_48 = 0x4,
	band_5g_52_64 = 0x8,
	band_5g_100_144 = 0x10,
	band_5g_149_165 = 0x20
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

enum ht_sec_ch_offset {
	NO_SEC_CH = 0,
	LOW_PRIMARY_CH = 1,
	HIGH_PRIMARY_CH = 3,
};

struct reg_ini_vars {
	uint32_t enable_11d_support;
	uint32_t userspace_ctry_priority;
	enum band_info band_capability;
	uint32_t dfs_enable;
	uint32_t indoor_channel_support;
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


QDF_STATUS reg_get_channel_list_with_power(struct wlan_objmgr_psoc *psoc,
					   struct channel_power *ch_list,
					   uint8_t *num_chan);

void reg_read_default_country(struct wlan_objmgr_psoc *psoc,
		uint8_t *country);
enum channel_state reg_get_channel_state(struct wlan_objmgr_psoc *psoc,
		uint32_t ch);
enum channel_state reg_get_5g_bonded_channel_state(
		struct wlan_objmgr_psoc *psoc,
		uint8_t ch, enum phy_ch_width bw);
enum channel_state reg_get_2g_bonded_channel_state(
		struct wlan_objmgr_psoc *psoc,
		uint8_t oper_ch, uint8_t sec_ch,
		enum phy_ch_width bw);
void reg_set_channel_params(struct wlan_objmgr_psoc *psoc,
		uint8_t ch, struct ch_params *ch_params);
void reg_get_dfs_region(struct wlan_objmgr_psoc *psoc,
		enum dfs_region *dfs_reg);
bool reg_is_dfs_ch(struct wlan_objmgr_psoc *psoc, uint8_t ch);
