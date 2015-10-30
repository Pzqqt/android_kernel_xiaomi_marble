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
#define CDS_MIN_5GHZ_CHANNEL_NUMBER chan_mapping[MIN_5GHZ_CHANNEL].chan_num
#define CDS_MAX_5GHZ_CHANNEL_NUMBER chan_mapping[MAX_5GHZ_CHANNEL].chan_num

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

enum channel_enum {
	RF_CHAN_1 = 0,
	RF_CHAN_2,
	RF_CHAN_3,
	RF_CHAN_4,
	RF_CHAN_5,
	RF_CHAN_6,
	RF_CHAN_7,
	RF_CHAN_8,
	RF_CHAN_9,
	RF_CHAN_10,
	RF_CHAN_11,
	RF_CHAN_12,
	RF_CHAN_13,
	RF_CHAN_14,

	RF_CHAN_36,
	RF_CHAN_40,
	RF_CHAN_44,
	RF_CHAN_48,
	RF_CHAN_52,
	RF_CHAN_56,
	RF_CHAN_60,
	RF_CHAN_64,

	RF_CHAN_100,
	RF_CHAN_104,
	RF_CHAN_108,
	RF_CHAN_112,
	RF_CHAN_116,
	RF_CHAN_120,
	RF_CHAN_124,
	RF_CHAN_128,
	RF_CHAN_132,
	RF_CHAN_136,
	RF_CHAN_140,
	RF_CHAN_144,

	RF_CHAN_149,
	RF_CHAN_153,
	RF_CHAN_157,
	RF_CHAN_161,
	RF_CHAN_165,

	RF_CHAN_170,
	RF_CHAN_171,
	RF_CHAN_172,
	RF_CHAN_173,
	RF_CHAN_174,
	RF_CHAN_175,
	RF_CHAN_176,
	RF_CHAN_177,
	RF_CHAN_178,
	RF_CHAN_179,
	RF_CHAN_180,
	RF_CHAN_181,
	RF_CHAN_182,
	RF_CHAN_183,
	RF_CHAN_184,

	NUM_RF_CHANNELS,

	MIN_24GHZ_CHANNEL = RF_CHAN_1,
	MAX_24GHZ_CHANNEL = RF_CHAN_14,
	NUM_24GHZ_CHANNELS = (MAX_24GHZ_CHANNEL - MIN_24GHZ_CHANNEL + 1),

	MIN_5GHZ_CHANNEL = RF_CHAN_36,
	MAX_5GHZ_CHANNEL = RF_CHAN_184,
	NUM_5GHZ_CHANNELS = (MAX_5GHZ_CHANNEL - MIN_5GHZ_CHANNEL + 1),

	MIN_20MHZ_RF_CHANNEL = RF_CHAN_1,
	MAX_20MHZ_RF_CHANNEL = RF_CHAN_184,
	NUM_20MHZ_RF_CHANNELS =
		(MAX_20MHZ_RF_CHANNEL - MIN_20MHZ_RF_CHANNEL + 1),

	MIN_59GHZ_CHANNEL = RF_CHAN_170,
	MAX_59GHZ_CHANNEL = RF_CHAN_184,

	INVALID_RF_CHANNEL = 0xBAD,
	RF_CHANNEL_INVALID_MAX_FIELD = 0x7FFFFFFF
};

/**
 * enum channel_state: channel state
 *
 * @CHANNEL_STATE_DISABLE: channel disabled
 * @CHANNEL_STATE_ENABLE: tx/rx enabled
 * @CHANNEL_STATE_DFS: rx enabled, tx DFS
 * @CHANNEL_STATE_INVALID: not a valid channel
 */
enum channel_state {
	CHANNEL_STATE_DISABLE,
	CHANNEL_STATE_ENABLE,
	CHANNEL_STATE_DFS,
	CHANNEL_STATE_INVALID
};

/**
 * struct regulatory_channel: regulatory channel
 *
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
 *
 * @center_freq: channel center freq
 * @chan_num: channel number
 */
struct chan_map {
	uint16_t center_freq;
	uint16_t chan_num;
};

/**
 * struct channel_power: channel power
 *
 * @chan_num: channel number
 * @power: tx power
 */
struct channel_power {
	uint8_t chan_num;
	int8_t power;
};

/**
 * enum country_src: country source
 *
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
 *
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
 * enum chan_width: channel width
 *
 * @CHAN_WIDTH_0MHZ: channel disabled or invalid
 * @CHAN_WIDTH_5MHZ: channel width 5 MHZ
 * @CHAN_WIDTH_10MHZ: channel width 10 MHZ
 * @CHAN_WIDTH_20MHZ: channel width 20 MHZ
 * @CHAN_WIDTH_40MHZ: channel width 40 MHZ
 * @CHAN_WIDTH_80MHZ: channel width 80MHZ
 * @CHAN_WIDTH_160MHZ: channel width 160 MHZ
 */
enum channel_width {
	CHAN_WIDTH_0MHZ,
	CHAN_WIDTH_5MHZ,
	CHAN_WIDTH_10MHZ,
	CHAN_WIDTH_20MHZ,
	CHAN_WIDTH_40MHZ,
	CHAN_WIDTH_80MHZ,
	CHAN_WIDTH_160MHZ
};

extern const struct chan_map chan_mapping[NUM_RF_CHANNELS];
extern struct regulatory_channel reg_channels[NUM_RF_CHANNELS];

QDF_STATUS cds_get_reg_domain_from_country_code(v_REGDOMAIN_t *pRegDomain,
						const uint8_t *country_alpha2,
						enum country_src source);

QDF_STATUS cds_read_default_country(uint8_t *default_country);

QDF_STATUS cds_get_channel_list_with_power(struct channel_power
					   *base_channels,
					   uint8_t *num_base_channels);

enum channel_state cds_get_channel_state(uint32_t chan_num);
QDF_STATUS cds_get_dfs_region(uint8_t *dfs_region);
QDF_STATUS cds_put_dfs_region(uint8_t dfs_region);

bool cds_is_dsrc_channel(uint16_t);
enum channel_state cds_get_bonded_channel_state(uint32_t chan_num,
					   enum channel_width chan_width);
enum channel_width cds_get_max_channel_bw(uint32_t chan_num);

QDF_STATUS cds_set_reg_domain(void *client_ctxt, v_REGDOMAIN_t reg_domain);

QDF_STATUS cds_put_default_country(uint8_t *def_country);
#endif /* __CDS_REG_SERVICE_H */
