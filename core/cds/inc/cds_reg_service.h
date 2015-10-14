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

#include "cdf_status.h"

#define CDS_COUNTRY_CODE_LEN  2
#define CDS_MAC_ADDRESS_LEN   6

typedef enum {
	REGDOMAIN_FCC,
	REGDOMAIN_ETSI,
	REGDOMAIN_JAPAN,
	REGDOMAIN_WORLD,
	REGDOMAIN_COUNT
} v_REGDOMAIN_t;

typedef enum {
	/* 2.4GHz Band */
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

	/* 4.9GHz Band */
	RF_CHAN_240,
	RF_CHAN_244,
	RF_CHAN_248,
	RF_CHAN_252,
	RF_CHAN_208,
	RF_CHAN_212,
	RF_CHAN_216,

	/* 5GHz Low & Mid U-NII Band */
	RF_CHAN_36,
	RF_CHAN_40,
	RF_CHAN_44,
	RF_CHAN_48,
	RF_CHAN_52,
	RF_CHAN_56,
	RF_CHAN_60,
	RF_CHAN_64,

	/* 5GHz Mid Band - ETSI & FCC */
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

	/* 5GHz High U-NII Band */
	RF_CHAN_149,
	RF_CHAN_153,
	RF_CHAN_157,
	RF_CHAN_161,
	RF_CHAN_165,

	/* 802.11p */
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

	/* CHANNEL BONDED CHANNELS */
	RF_CHAN_BOND_3,
	RF_CHAN_BOND_4,
	RF_CHAN_BOND_5,
	RF_CHAN_BOND_6,
	RF_CHAN_BOND_7,
	RF_CHAN_BOND_8,
	RF_CHAN_BOND_9,
	RF_CHAN_BOND_10,
	RF_CHAN_BOND_11,
	RF_CHAN_BOND_242,       /* 4.9GHz Band */
	RF_CHAN_BOND_246,
	RF_CHAN_BOND_250,
	RF_CHAN_BOND_210,
	RF_CHAN_BOND_214,
	RF_CHAN_BOND_38,        /* 5GHz Low & Mid U-NII Band */
	RF_CHAN_BOND_42,
	RF_CHAN_BOND_46,
	RF_CHAN_BOND_50,
	RF_CHAN_BOND_54,
	RF_CHAN_BOND_58,
	RF_CHAN_BOND_62,
	RF_CHAN_BOND_102,       /* 5GHz Mid Band - ETSI & FCC */
	RF_CHAN_BOND_106,
	RF_CHAN_BOND_110,
	RF_CHAN_BOND_114,
	RF_CHAN_BOND_118,
	RF_CHAN_BOND_122,
	RF_CHAN_BOND_126,
	RF_CHAN_BOND_130,
	RF_CHAN_BOND_134,
	RF_CHAN_BOND_138,

	RF_CHAN_BOND_142,

	RF_CHAN_BOND_151,       /* 5GHz High U-NII Band */
	RF_CHAN_BOND_155,
	RF_CHAN_BOND_159,
	RF_CHAN_BOND_163,

	NUM_RF_CHANNELS,

	MIN_2_4GHZ_CHANNEL = RF_CHAN_1,
	MAX_2_4GHZ_CHANNEL = RF_CHAN_14,
	NUM_24GHZ_CHANNELS = (MAX_2_4GHZ_CHANNEL - MIN_2_4GHZ_CHANNEL + 1),

	MIN_5GHZ_CHANNEL = RF_CHAN_240,
	MAX_5GHZ_CHANNEL = RF_CHAN_184,
	NUM_5GHZ_CHANNELS = (MAX_5GHZ_CHANNEL - MIN_5GHZ_CHANNEL + 1),

	MIN_20MHZ_RF_CHANNEL = RF_CHAN_1,
	MAX_20MHZ_RF_CHANNEL = RF_CHAN_184,
	NUM_20MHZ_RF_CHANNELS =
		(MAX_20MHZ_RF_CHANNEL - MIN_20MHZ_RF_CHANNEL + 1),

	MIN_40MHZ_RF_CHANNEL = RF_CHAN_BOND_3,
	MAX_40MHZ_RF_CHANNEL = RF_CHAN_BOND_163,
	NUM_40MHZ_RF_CHANNELS =
		(MAX_40MHZ_RF_CHANNEL - MIN_40MHZ_RF_CHANNEL + 1),

	MIN_5_9GHZ_CHANNEL = RF_CHAN_170,
	MAX_5_9GHZ_CHANNEL = RF_CHAN_184,

	INVALID_RF_CHANNEL = 0xBAD,
	RF_CHANNEL_INVALID_MAX_FIELD = 0x7FFFFFFF
} eRfChannels;

typedef enum {
	CHANNEL_STATE_DISABLE,
	CHANNEL_STATE_ENABLE,
	CHANNEL_STATE_DFS,
	CHANNEL_STATE_INVALID
} CHANNEL_STATE;

typedef int8_t tPowerdBm;

typedef struct {
	uint32_t enabled:4;
	uint32_t flags:28;
	tPowerdBm pwrLimit;
} sRegulatoryChannel;

typedef struct {
	uint16_t targetFreq;
	uint16_t channelNum;
} tRfChannelProps;

typedef struct {
	uint8_t chanId;
	tPowerdBm pwr;
} tChannelListWithPower;

typedef enum {
	COUNTRY_CODE_SET_BY_CORE,
	COUNTRY_CODE_SET_BY_DRIVER,
	COUNTRY_CODE_SET_BY_USER
} COUNTRY_CODE_SOURCE;


struct regulatory {
	uint32_t reg_domain;
	uint32_t eeprom_rd_ext;
	uint16_t country_code;
	uint8_t alpha2[3];
	uint8_t def_country[3];
	uint8_t dfs_region;
	uint8_t ctl_2g;
	uint8_t ctl_5g;
	const void *regpair;
	COUNTRY_CODE_SOURCE cc_src;
	uint32_t reg_flags;
};

typedef enum {
	COUNTRY_INIT,
	COUNTRY_IE,
	COUNTRY_USER,
	COUNTRY_QUERY,
	COUNTRY_MAX = COUNTRY_QUERY
} v_CountryInfoSource_t;

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

/**
 * @country_code_t : typedef for country code. One extra
 * char for holding null character
 */
typedef uint8_t country_code_t[CDS_COUNTRY_CODE_LEN + 1];


CDF_STATUS cds_get_reg_domain_from_country_code(v_REGDOMAIN_t *pRegDomain,
						const country_code_t countryCode,
						v_CountryInfoSource_t source);

CDF_STATUS cds_read_default_country(country_code_t default_country);

CDF_STATUS cds_get_channel_list_with_power(tChannelListWithPower
					   *pChannels20MHz,
					   uint8_t *pNum20MHzChannelsFound,
					   tChannelListWithPower
					   *pChannels40MHz,
					   uint8_t *pNum40MHzChannelsFound);

CDF_STATUS cds_set_reg_domain(void *clientCtxt, v_REGDOMAIN_t regId);

CHANNEL_STATE cds_get_channel_state(uint32_t rfChannel);

CDF_STATUS cds_regulatory_init(void);
CDF_STATUS cds_get_dfs_region(uint8_t *dfs_region);
CDF_STATUS cds_set_dfs_region(uint8_t dfs_region);
bool cds_is_dsrc_channel(uint16_t);
CHANNEL_STATE cds_get_bonded_channel_state(uint32_t chan_num,
					   enum channel_width ch_width);
enum channel_width cds_get_max_channel_bw(uint32_t chan_num);


#define CDS_IS_DFS_CH(channel) (cds_get_channel_state((channel)) == \
				CHANNEL_STATE_DFS)

#define CDS_IS_PASSIVE_OR_DISABLE_CH(channel) \
    (cds_get_channel_state((channel)) != CHANNEL_STATE_ENABLE)

#define CDS_MAX_24GHz_CHANNEL_NUMBER \
    (rf_channels[MAX_2_4GHZ_CHANNEL].channelNum)
#define CDS_MIN_5GHz_CHANNEL_NUMBER  (rf_channels[RF_CHAN_36].channelNum)
#define CDS_MAX_5GHz_CHANNEL_NUMBER  (rf_channels[MAX_5GHZ_CHANNEL].channelNum)

#define CDS_IS_CHANNEL_5GHZ(chnNum) \
	(((chnNum) >= CDS_MIN_5GHz_CHANNEL_NUMBER) && ((chnNum) <= CDS_MAX_5GHz_CHANNEL_NUMBER))

#define CDS_IS_CHANNEL_24GHZ(chnNum) \
	(((chnNum) > 0) && ((chnNum) <= CDS_MAX_24GHz_CHANNEL_NUMBER))

#define CDS_IS_SAME_BAND_CHANNELS(ch1, ch2) \
	(ch1 && ch2 && \
	(CDS_IS_CHANNEL_5GHZ(ch1) == CDS_IS_CHANNEL_5GHZ(ch2)))

#endif /* __CDS_REG_SERVICE_H */
