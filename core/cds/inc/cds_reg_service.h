/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#if !defined __CDS_REG_SERVICE_H
#define __CDS_REG_SERVICE_H

/**=========================================================================

   \file  cds_reg_service.h

   \brief Connectivity driver services (CDS): Non-Volatile storage API

   ========================================================================*/

#include "qdf_status.h"

#define CDS_COUNTRY_CODE_LEN  2
#define CDS_MAC_ADDRESS_LEN 6

#define CDS_CHANNEL_STATE(chan_enum) reg_channels[chan_enum].state
#define CDS_CHANNEL_NUM(chan_enum) chan_mapping[chan_enum].chan_num
#define CDS_CHANNEL_FREQ(chan_enum) chan_mapping[chan_enum].center_freq
#define CDS_IS_DFS_CH(chan_num) (cds_get_channel_state((chan_num)) == \
				CHANNEL_STATE_DFS)

#define CDS_IS_PASSIVE_OR_DISABLE_CH(chan_num) \
	(cds_get_channel_state(chan_num) != CHANNEL_STATE_ENABLE)

#define CDS_MIN_24GHZ_CHANNEL_NUMBER chan_mapping[MIN_24GHZ_CHANNEL].chan_num
#define CDS_MAX_24GHZ_CHANNEL_NUMBER chan_mapping[MAX_24GHZ_CHANNEL].chan_num
#define CDS_MIN_59GHZ_CHANNEL_NUMBER chan_mapping[MIN_59GHZ_CHANNEL].chan_num
#define CDS_MAX_59GHZ_CHANNEL_NUMBER chan_mapping[MAX_59GHZ_CHANNEL].chan_num
#define CDS_MIN_5GHZ_CHANNEL_NUMBER chan_mapping[MIN_5GHZ_CHANNEL].chan_num
#define CDS_MAX_5GHZ_CHANNEL_NUMBER chan_mapping[MAX_5GHZ_CHANNEL].chan_num

#define CDS_IS_CHANNEL_DSRC(chan_num) \
	((chan_num >= CDS_MIN_59GHZ_CHANNEL_NUMBER) && \
	 (chan_num <= CDS_MAX_59GHZ_CHANNEL_NUMBER))

#define CDS_IS_CHANNEL_5GHZ(chan_num) \
	((chan_num >= CDS_MIN_5GHZ_CHANNEL_NUMBER) && \
	 (chan_num <= CDS_MAX_5GHZ_CHANNEL_NUMBER))

#define CDS_IS_CHANNEL_24GHZ(chan_num) \
	((chan_num >= CDS_MIN_24GHZ_CHANNEL_NUMBER) && \
	 (chan_num <= CDS_MAX_24GHZ_CHANNEL_NUMBER))

#define CDS_IS_SAME_BAND_CHANNELS(chan_num1, chan_num2) \
	(chan_num1 && chan_num2 && \
	(CDS_IS_CHANNEL_5GHZ(chan_num1) == CDS_IS_CHANNEL_5GHZ(chan_num2)))

#define CDS_MIN_11P_CHANNEL chan_mapping[MIN_59GHZ_CHANNEL].chan_num

typedef enum {
	REGDOMAIN_FCC,
	REGDOMAIN_ETSI,
	REGDOMAIN_JAPAN,
	REGDOMAIN_WORLD,
	REGDOMAIN_COUNT
} v_REGDOMAIN_t;


/**
 * enum channel_enum: channel enumeration
 * @CHAN_ENUM_1: channel number 1
 * @CHAN_ENUM_2: channel number 2
 * @CHAN_ENUM_3: channel number 3
 * @CHAN_ENUM_4: channel number 4
 * @CHAN_ENUM_5: channel number 5
 * @CHAN_ENUM_6: channel number 6
 * @CHAN_ENUM_7: channel number 7
 * @CHAN_ENUM_8: channel number 8
 * @CHAN_ENUM_9: channel number 9
 * @CHAN_ENUM_10: channel number 10
 * @CHAN_ENUM_11: channel number 11
 * @CHAN_ENUM_12: channel number 12
 * @CHAN_ENUM_13: channel number 13
 * @CHAN_ENUM_14: channel number 14
 * @CHAN_ENUM_36: channel number 36
 * @CHAN_ENUM_40: channel number 40
 * @CHAN_ENUM_44: channel number 44
 * @CHAN_ENUM_48: channel number 48
 * @CHAN_ENUM_52: channel number 52
 * @CHAN_ENUM_56: channel number 56
 * @CHAN_ENUM_60: channel number 60
 * @CHAN_ENUM_64: channel number 64
 * @CHAN_ENUM_100: channel number 100
 * @CHAN_ENUM_104: channel number 104
 * @CHAN_ENUM_108: channel number 108
 * @CHAN_ENUM_112: channel number 112
 * @CHAN_ENUM_116: channel number 116
 * @CHAN_ENUM_120: channel number 120
 * @CHAN_ENUM_124: channel number 124
 * @CHAN_ENUM_128: channel number 128
 * @CHAN_ENUM_132: channel number 132
 * @CHAN_ENUM_136: channel number 136
 * @CHAN_ENUM_140: channel number 140
 * @CHAN_ENUM_144: channel number 144
 * @CHAN_ENUM_149: channel number 149
 * @CHAN_ENUM_153: channel number 153
 * @CHAN_ENUM_157: channel number 157
 * @CHAN_ENUM_161: channel number 161
 * @CHAN_ENUM_165: channel number 165
 * @CHAN_ENUM_170: channel number 108
 * @CHAN_ENUM_171: channel number 112
 * @CHAN_ENUM_172: channel number 116
 * @CHAN_ENUM_173: channel number 120
 * @CHAN_ENUM_174: channel number 124
 * @CHAN_ENUM_175: channel number 128
 * @CHAN_ENUM_176: channel number 132
 * @CHAN_ENUM_177: channel number 136
 * @CHAN_ENUM_178: channel number 140
 * @CHAN_ENUM_179: channel number 144
 * @CHAN_ENUM_180: channel number 149
 * @CHAN_ENUM_181: channel number 153
 * @CHAN_ENUM_182: channel number 157
 * @CHAN_ENUM_183: channel number 161
 * @CHAN_ENUM_184: channel number 165
 */
enum channel_enum {
	CHAN_ENUM_1 = 0,
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

	MIN_59GHZ_CHANNEL = CHAN_ENUM_170,
	MAX_59GHZ_CHANNEL = CHAN_ENUM_184,

	INVALID_CHANNEL = 0xBAD,
};

/**
 * enum channel_state: channel state
 * @CHANNEL_STATE_DISABLE: channel disabled
 * @CHANNEL_STATE_DFS: rx enabled, tx DFS
 * @CHANNEL_STATE_ENABLE: tx/rx enabled
 * @CHANNEL_STATE_INVALID: not a valid channel
 */
enum channel_state {
	CHANNEL_STATE_DISABLE,
	CHANNEL_STATE_DFS,
	CHANNEL_STATE_ENABLE,
	CHANNEL_STATE_INVALID,
};

/**
 * struct regulatory_channel: regulatory channel
 * @state: channel state
 * @flags: channel flags
 * @pwr_limit: channel tx power limit
 */
struct regulatory_channel {
	uint32_t state:4;
	uint32_t flags:28;
	int8_t pwr_limit;
};

/**
 * struct chan_map: channel mapping
 * @center_freq: channel center freq
 * @chan_num: channel number
 */
struct chan_map {
	uint16_t center_freq;
	uint16_t chan_num;
};

/**
 * struct channel_power: channel power
 * @chan_num: channel number
 * @power: tx power
 */
struct channel_power {
	uint8_t chan_num;
	int8_t power;
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
	SOURCE_QUERY,
	SOURCE_CORE,
	SOURCE_DRIVER,
	SOURCE_USERSPACE,
	SOURCE_11D
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
	uint8_t alpha2[CDS_COUNTRY_CODE_LEN + 1];
	uint8_t ctl_2g;
	uint8_t ctl_5g;
	const void *regpair;
	enum country_src cc_src;
	uint32_t reg_flags;
};

/**
 * enum phy_ch_width - physical layer channel width
 * @CH_WIDTH_20MHZ: channel width 20 mhz
 * @CH_WIDTH_40MHZ: channel width 40 mhz
 * @CH_WIDTH_80MHZ: channel width 80 mhz
 * @CH_WIDTH_160MHZ: channel width 160 mhz
 * @CH_WIDTH_80P80MHZ: channel width 80+80 mhz
 * @CH_WIDTH_5MHZ: channel width 5 mhz
 * @CH_WIDTH_10MHZ: channel width 10 mhz
 * @CH_WIDTH_INVALID: invalid channel width
 * @CH_WIDTH_MAX: maximum channel width
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
 * struct ch_params_s
 * @ch_width: channel width
 * @sec_ch_offset: secondary channel offset
 * @center_freq_seg0: center freq for segment 0
 * @center_freq_seg1: center freq for segment 1
 */
struct ch_params_s {
	enum phy_ch_width ch_width;
	uint8_t sec_ch_offset;
	uint8_t center_freq_seg0;
	uint8_t center_freq_seg1;
};

/**
 * enum dfs_region - DFS region
 * @DFS_UNINIT_REGION: un-initialized region
 * @DFS_FCC_REGION: FCC region
 * @DFS_ETSI_REGION: ETSI region
 * @DFS_MKK_REGION: MKK region
 * @DFS_CN_REGION: China region
 * @DFS_KR_REGION: Korea region
 * @DFS_UNDEF_REGION: Undefined region
 */
enum dfs_region {
	DFS_UNINIT_REGION = 0,
	DFS_FCC_REGION = 1,
	DFS_ETSI_REGION = 2,
	DFS_MKK_REGION = 3,
	DFS_CN_REGION = 4,
	DFS_KR_REGION = 5,
	DFS_UNDEF_REGION
};

extern const struct chan_map chan_mapping[NUM_CHANNELS];
extern struct regulatory_channel reg_channels[NUM_CHANNELS];

QDF_STATUS cds_get_reg_domain_from_country_code(v_REGDOMAIN_t *pRegDomain,
						const uint8_t *country_alpha2,
						enum country_src source);

QDF_STATUS cds_read_default_country(uint8_t *default_country);

QDF_STATUS cds_get_channel_list_with_power(struct channel_power
					   *base_channels,
					   uint8_t *num_base_channels);

enum channel_state cds_get_channel_state(uint32_t chan_num);
QDF_STATUS cds_get_dfs_region(enum dfs_region *dfs_reg);
QDF_STATUS cds_put_dfs_region(enum dfs_region dfs_reg);

bool cds_is_dsrc_channel(uint16_t center_freq);
enum channel_state cds_get_5g_bonded_channel_state(uint16_t chan_num,
						 enum phy_ch_width chan_width);

enum channel_state cds_get_2g_bonded_channel_state(uint16_t chan_num,
						   enum phy_ch_width chan_width,
						   uint16_t sec_ch);

void cds_set_channel_params(uint16_t oper_ch, uint16_t ht_offset_2g,
			    struct ch_params_s *ch_params);

QDF_STATUS cds_set_reg_domain(void *client_ctxt, v_REGDOMAIN_t reg_domain);
QDF_STATUS cds_put_default_country(uint8_t *def_country);
uint16_t cds_bw_value(enum phy_ch_width bw);
uint8_t cds_skip_dfs_and_2g(uint32_t rf_channel);

#endif /* __CDS_REG_SERVICE_H */
