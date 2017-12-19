/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
#define REG_MAX_CHAN_CHANGE_CBKS 30
#define MAX_STA_VDEV_CNT 4
#define INVALID_VDEV_ID 0xFF
#define INVALID_CHANNEL_NUM 0xBAD
#define CH_AVOID_MAX_RANGE   4

#ifdef CONFIG_LEGACY_CHAN_ENUM

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
 * @CHAN_ENUM_169:  channel number 169
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
	CHAN_ENUM_169,

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

	MIN_49GHZ_CHANNEL = INVALID_CHANNEL_NUM,
	MAX_49GHZ_CHANNEL = INVALID_CHANNEL_NUM - 1,
	NUM_49GHZ_CHANNELS = MAX_49GHZ_CHANNEL - MIN_49GHZ_CHANNEL + 1,

	MIN_5GHZ_CHANNEL = CHAN_ENUM_36,
	MAX_5GHZ_CHANNEL = CHAN_ENUM_184,
	NUM_5GHZ_CHANNELS = (MAX_5GHZ_CHANNEL - MIN_5GHZ_CHANNEL + 1),

	MIN_11P_CHANNEL = CHAN_ENUM_170,
	MAX_11P_CHANNEL = CHAN_ENUM_184,
	NUM_11P_CHANNELS = (MAX_11P_CHANNEL - MIN_11P_CHANNEL + 1),

	INVALID_CHANNEL = 0xBAD,
};

#else
/**
 * enum channel_enum - channel enumeration
 * @CHAN_ENUM_2412: channel with freq 2412
 * @CHAN_ENUM_2417: channel with freq 2417
 * @CHAN_ENUM_2422: channel with freq 2422
 * @CHAN_ENUM_2427: channel with freq 2427
 * @CHAN_ENUM_2432: channel with freq 2432
 * @CHAN_ENUM_2437: channel with freq 2437
 * @CHAN_ENUM_2442: channel with freq 2442
 * @CHAN_ENUM_2447: channel with freq 2447
 * @CHAN_ENUM_2452: channel with freq 2452
 * @CHAN_ENUM_2457: channel with freq 2457
 * @CHAN_ENUM_2462: channel with freq 2462
 * @CHAN_ENUM_2467: channel with freq 2467
 * @CHAN_ENUM_2472: channel with freq 2472
 * @CHAN_ENUM_2484: channel with freq 2484
 * @CHAN_ENUM_4912: channel with freq 4912
 * @CHAN_ENUM_4915: channel with freq 4915
 * @CHAN_ENUM_4917: channel with freq 4917
 * @CHAN_ENUM_4920: channel with freq 4920
 * @CHAN_ENUM_4922: channel with freq 4922
 * @CHAN_ENUM_4925: channel with freq 4925
 * @CHAN_ENUM_4927: channel with freq 4927
 * @CHAN_ENUM_4932: channel with freq 4932
 * @CHAN_ENUM_4935: channel with freq 4935
 * @CHAN_ENUM_4937: channel with freq 4937
 * @CHAN_ENUM_4940: channel with freq 4940
 * @CHAN_ENUM_4942: channel with freq 4942
 * @CHAN_ENUM_4945: channel with freq 4945
 * @CHAN_ENUM_4947: channel with freq 4947
 * @CHAN_ENUM_4950: channel with freq 4950
 * @CHAN_ENUM_4952: channel with freq 4952
 * @CHAN_ENUM_4955: channel with freq 4955
 * @CHAN_ENUM_4957: channel with freq 4957
 * @CHAN_ENUM_4960: channel with freq 4960
 * @CHAN_ENUM_4962: channel with freq 4962
 * @CHAN_ENUM_4965: channel with freq 4965
 * @CHAN_ENUM_4967: channel with freq 4967
 * @CHAN_ENUM_4970: channel with freq 4970
 * @CHAN_ENUM_4972: channel with freq 4972
 * @CHAN_ENUM_4975: channel with freq 4975
 * @CHAN_ENUM_4977: channel with freq 4977
 * @CHAN_ENUM_4980: channel with freq 4980
 * @CHAN_ENUM_4982: channel with freq 4982
 * @CHAN_ENUM_4985: channel with freq 4985
 * @CHAN_ENUM_4987: channel with freq 4987
 * @CHAN_ENUM_5032: channel with freq 5032
 * @CHAN_ENUM_5035: channel with freq 5035
 * @CHAN_ENUM_5037: channel with freq 5037
 * @CHAN_ENUM_5040: channel with freq 5040
 * @CHAN_ENUM_5042: channel with freq 5042
 * @CHAN_ENUM_5045: channel with freq 5045
 * @CHAN_ENUM_5047: channel with freq 5047
 * @CHAN_ENUM_5052: channel with freq 5052
 * @CHAN_ENUM_5055: channel with freq 5055
 * @CHAN_ENUM_5057: channel with freq 5057
 * @CHAN_ENUM_5060: channel with freq 5060
 * @CHAN_ENUM_5080: channel with freq 5080
 * @CHAN_ENUM_5180: channel with freq 5180
 * @CHAN_ENUM_5200: channel with freq 5200
 * @CHAN_ENUM_5220: channel with freq 5220
 * @CHAN_ENUM_5240: channel with freq 5240
 * @CHAN_ENUM_5260: channel with freq 5260
 * @CHAN_ENUM_5280: channel with freq 5280
 * @CHAN_ENUM_5300: channel with freq 5300
 * @CHAN_ENUM_5320: channel with freq 5320
 * @CHAN_ENUM_5500: channel with freq 5500
 * @CHAN_ENUM_5520: channel with freq 5520
 * @CHAN_ENUM_5540: channel with freq 5540
 * @CHAN_ENUM_5560: channel with freq 5560
 * @CHAN_ENUM_5580: channel with freq 5580
 * @CHAN_ENUM_5600: channel with freq 5600
 * @CHAN_ENUM_5620: channel with freq 5620
 * @CHAN_ENUM_5640: channel with freq 5640
 * @CHAN_ENUM_5660: channel with freq 5660
 * @CHAN_ENUM_5680: channel with freq 5680
 * @CHAN_ENUM_5700: channel with freq 5700
 * @CHAN_ENUM_5720: channel with freq 5720
 * @CHAN_ENUM_5745: channel with freq 5745
 * @CHAN_ENUM_5765: channel with freq 5765
 * @CHAN_ENUM_5785: channel with freq 5785
 * @CHAN_ENUM_5805: channel with freq 5805
 * @CHAN_ENUM_5825: channel with freq 5825
 * @CHAN_ENUM_5845: channel with freq 5845
 * @CHAN_ENUM_5850: channel with freq 5850
 * @CHAN_ENUM_5855: channel with freq 5855
 * @CHAN_ENUM_5860: channel with freq 5860
 * @CHAN_ENUM_5865: channel with freq 5865
 * @CHAN_ENUM_5870: channel with freq 5870
 * @CHAN_ENUM_5875: channel with freq 5875
 * @CHAN_ENUM_5880: channel with freq 5880
 * @CHAN_ENUM_5885: channel with freq 5885
 * @CHAN_ENUM_5890: channel with freq 5890
 * @CHAN_ENUM_5895: channel with freq 5895
 * @CHAN_ENUM_5900: channel with freq 5900
 * @CHAN_ENUM_5905: channel with freq 5905
 * @CHAN_ENUM_5910: channel with freq 5910
 * @CHAN_ENUM_5915: channel with freq 5915
 * @CHAN_ENUM_5920: channel with freq 5920
 */
enum channel_enum {
	CHAN_ENUM_2412,
	CHAN_ENUM_2417,
	CHAN_ENUM_2422,
	CHAN_ENUM_2427,
	CHAN_ENUM_2432,
	CHAN_ENUM_2437,
	CHAN_ENUM_2442,
	CHAN_ENUM_2447,
	CHAN_ENUM_2452,
	CHAN_ENUM_2457,
	CHAN_ENUM_2462,
	CHAN_ENUM_2467,
	CHAN_ENUM_2472,
	CHAN_ENUM_2484,

	CHAN_ENUM_4912,
	CHAN_ENUM_4915,
	CHAN_ENUM_4917,
	CHAN_ENUM_4920,
	CHAN_ENUM_4922,
	CHAN_ENUM_4925,
	CHAN_ENUM_4927,
	CHAN_ENUM_4932,
	CHAN_ENUM_4935,
	CHAN_ENUM_4937,
	CHAN_ENUM_4940,
	CHAN_ENUM_4942,
	CHAN_ENUM_4945,
	CHAN_ENUM_4947,
	CHAN_ENUM_4950,
	CHAN_ENUM_4952,
	CHAN_ENUM_4955,
	CHAN_ENUM_4957,
	CHAN_ENUM_4960,
	CHAN_ENUM_4962,
	CHAN_ENUM_4965,
	CHAN_ENUM_4967,
	CHAN_ENUM_4970,
	CHAN_ENUM_4972,
	CHAN_ENUM_4975,
	CHAN_ENUM_4977,
	CHAN_ENUM_4980,
	CHAN_ENUM_4982,
	CHAN_ENUM_4985,
	CHAN_ENUM_4987,
	CHAN_ENUM_5032,
	CHAN_ENUM_5035,
	CHAN_ENUM_5037,
	CHAN_ENUM_5040,
	CHAN_ENUM_5042,
	CHAN_ENUM_5045,
	CHAN_ENUM_5047,
	CHAN_ENUM_5052,
	CHAN_ENUM_5055,
	CHAN_ENUM_5057,
	CHAN_ENUM_5060,
	CHAN_ENUM_5080,

	CHAN_ENUM_5180,
	CHAN_ENUM_5200,
	CHAN_ENUM_5220,
	CHAN_ENUM_5240,
	CHAN_ENUM_5260,
	CHAN_ENUM_5280,
	CHAN_ENUM_5300,
	CHAN_ENUM_5320,
	CHAN_ENUM_5500,
	CHAN_ENUM_5520,
	CHAN_ENUM_5540,
	CHAN_ENUM_5560,
	CHAN_ENUM_5580,
	CHAN_ENUM_5600,
	CHAN_ENUM_5620,
	CHAN_ENUM_5640,
	CHAN_ENUM_5660,
	CHAN_ENUM_5680,
	CHAN_ENUM_5700,
	CHAN_ENUM_5720,
	CHAN_ENUM_5745,
	CHAN_ENUM_5765,
	CHAN_ENUM_5785,
	CHAN_ENUM_5805,
	CHAN_ENUM_5825,
	CHAN_ENUM_5845,

	CHAN_ENUM_5850,
	CHAN_ENUM_5855,
	CHAN_ENUM_5860,
	CHAN_ENUM_5865,
	CHAN_ENUM_5870,
	CHAN_ENUM_5875,
	CHAN_ENUM_5880,
	CHAN_ENUM_5885,
	CHAN_ENUM_5890,
	CHAN_ENUM_5895,
	CHAN_ENUM_5900,
	CHAN_ENUM_5905,
	CHAN_ENUM_5910,
	CHAN_ENUM_5915,
	CHAN_ENUM_5920,

	NUM_CHANNELS,

	MIN_24GHZ_CHANNEL = CHAN_ENUM_2412,
	MAX_24GHZ_CHANNEL = CHAN_ENUM_2484,
	NUM_24GHZ_CHANNELS = (MAX_24GHZ_CHANNEL - MIN_24GHZ_CHANNEL + 1),

	MIN_49GHZ_CHANNEL = CHAN_ENUM_4912,
	MAX_49GHZ_CHANNEL = CHAN_ENUM_5080,
	NUM_49GHZ_CHANNELS = (MAX_49GHZ_CHANNEL - MIN_49GHZ_CHANNEL + 1),

	MIN_5GHZ_CHANNEL = CHAN_ENUM_5180,
	MAX_5GHZ_CHANNEL = CHAN_ENUM_5920,
	NUM_5GHZ_CHANNELS = (MAX_5GHZ_CHANNEL - MIN_5GHZ_CHANNEL + 1),

	MIN_11P_CHANNEL = CHAN_ENUM_5850,
	MAX_11P_CHANNEL = CHAN_ENUM_5920,
	NUM_11P_CHANNELS = (MAX_11P_CHANNEL - MIN_11P_CHANNEL + 1),

	INVALID_CHANNEL = 0xBAD,
};
#endif

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
 * struct reg_start_11d_scan_req: start 11d scan request
 * @vdev_id: vdev id
 * @scan_period_msec: scan duration in milli-seconds
 * @start_interval_msec: offset duration to start the scan in milli-seconds
 */
struct reg_start_11d_scan_req {
	uint8_t vdev_id;
	uint32_t scan_period_msec;
	uint32_t start_interval_msec;
};

/**
 * struct reg_stop_11d_scan_req: stop 11d scan request
 * @vdev_id: vdev id
 */
struct reg_stop_11d_scan_req {
	uint8_t vdev_id;
};

/**
 * struct reg_11d_new_country: regulatory 11d new coutry code
 * @alpha2: new 11d alpha2
 */
struct reg_11d_new_country {
	uint8_t alpha2[REG_ALPHA2_LEN + 1];
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
 * @center_freq: center freq in mhz
 * @chan_num: channel number
 * @min_bw: min bw
 * @max_bw: max bw
 */
struct chan_map {
	uint32_t center_freq;
	uint32_t chan_num;
	uint16_t min_bw;
	uint16_t max_bw;
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

struct set_country {
	uint8_t country[REG_ALPHA2_LEN + 1];
	uint8_t pdev_id;
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

enum cc_setting_code {
	REG_SET_CC_STATUS_PASS = 0,
	REG_CURRENT_ALPHA2_NOT_FOUND = 1,
	REG_INIT_ALPHA2_NOT_FOUND = 2,
	REG_SET_CC_CHANGE_NOT_ALLOWED = 3,
	REG_SET_CC_STATUS_NO_MEMORY = 4,
	REG_SET_CC_STATUS_FAIL = 5,
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
 * @status_code: status value
 * @num_phy: number of phy
 * @phy_id: phy id
 * @reg_dmn_pair: reg domain pair
 * @ctry_code: country code
 * @alpha2: country alpha2
 * @offload_enabled: offload enabled
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
	enum cc_setting_code status_code;
	uint8_t num_phy;
	uint8_t phy_id;
	uint16_t reg_dmn_pair;
	uint16_t ctry_code;
	uint8_t alpha2[REG_ALPHA2_LEN + 1];
	bool offload_enabled;
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
 * @BAND_UNKNOWN: Unsupported band
 */
enum band_info {
	BAND_ALL,
	BAND_2G,
	BAND_5G,
	BAND_UNKNOWN
};

/**
 * enum restart_beaconing_on_ch_avoid_rule: control the beaconing entity to
 * move away from active LTE channels
 * @CH_AVOID_RULE_DO_NOT_RESTART: Do not move from active LTE
 *                              channels
 * @CH_AVOID_RULE_RESTART: Move from active LTE channels
 * @CH_AVOID_RULE_RESTART_24G_ONLY: move from 2.4G active LTE
 *                                channels only
 */
enum restart_beaconing_on_ch_avoid_rule {
	CH_AVOID_RULE_DO_NOT_RESTART,
	CH_AVOID_RULE_RESTART,
	CH_AVOID_RULE_RESTART_24G_ONLY,
};

/**
 * struct reg_config_vars
 * @enable_11d_support: enable 11d support
 * @scan_11d_interval: 11d scan interval in ms
 * @userspace_ctry_priority: user priority
 * @band_capability: band capability
 * @dfs_disable: dfs disabled
 * @indoor_channel_support: indoor channel support
 * @force_ssc_disable_indoor_channel: Disable indoor channel on sap start
 * @restart_beaconing: control the beaconing entity to move
 * away from active LTE channels
 */
struct reg_config_vars {
	uint32_t enable_11d_support;
	uint32_t scan_11d_interval;
	uint32_t userspace_ctry_priority;
	enum band_info band_capability;
	uint32_t dfs_enabled;
	uint32_t indoor_chan_enabled;
	uint32_t force_ssc_disable_indoor_channel;
	enum restart_beaconing_on_ch_avoid_rule restart_beaconing;
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

/**
 * struct reg_sched_payload
 * @psoc: psoc ptr
 * @pdev: pdev ptr
 */
struct reg_sched_payload {
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
};

/**
 * enum direction
 * @NORTHBOUND: northbound
 * @SOUTHBOUND: southbound
 */
enum direction {
	NORTHBOUND,
	SOUTHBOUND,
};

/**
 * struct mas_chan_params
 * @dfs_region: dfs region
 * @phybitmap: phybitmap
 * @mas_chan_list: master chan list
 * @default_country: default country
 * @current_country: current country
 * @def_region_domain: default reg domain
 * @def_country_code: default country code
 * @reg_dmn_pair: reg domain pair
 * @ctry_code: country code
 */
struct mas_chan_params {
	enum dfs_reg dfs_region;
	uint32_t phybitmap;
	struct regulatory_channel mas_chan_list[NUM_CHANNELS];
	char default_country[REG_ALPHA2_LEN + 1];
	char current_country[REG_ALPHA2_LEN + 1];
	uint16_t def_region_domain;
	uint16_t def_country_code;
	uint16_t reg_dmn_pair;
	uint16_t ctry_code;
};

/**
 * enum cc_regdmn_flag: Regdomain flags
 * @INVALID:       Invalid flag
 * @CC_IS_SET:     Country code is set
 * @REGDMN_IS_SET: Regdomain ID is set
 * @ALPHA_IS_SET:  Country ISO is set
 */
enum cc_regdmn_flag {
	INVALID_CC,
	CC_IS_SET,
	REGDMN_IS_SET,
	ALPHA_IS_SET,
};

/**
 * struct cc_regdmn_s: User country code or regdomain
 * @country_code: Country code
 * @regdmn_id:    Regdomain pair ID
 * @alpha:        Country ISO
 * @flags:        Regdomain flags
 */
struct cc_regdmn_s {
	union {
		uint16_t country_code;
		uint16_t regdmn_id;
		uint8_t alpha[REG_ALPHA2_LEN + 1];
	} cc;
	uint8_t flags;
};

/**
 * struct cur_regdmn_info: Current regulatory info
 * @regdmn_pair_id: Current regdomain pair ID
 * @dmn_id_2g: 2GHz regdomain ID
 * @dmn_id_5g: 5GHz regdomain ID
 * @ctl_2g: 2GHz CTL value
 * @ctl_5g: 5GHzCTL value
 * @dfs_region: dfs region
 */
struct cur_regdmn_info {
	uint16_t regdmn_pair_id;
	uint16_t dmn_id_2g;
	uint16_t dmn_id_5g;
	uint8_t ctl_2g;
	uint8_t ctl_5g;
	uint8_t dfs_region;
};

/**
 * struct ch_avoid_freq_type
 * @start_freq: start freq
 * @end_freq: end freq
 */
struct ch_avoid_freq_type {
	uint32_t start_freq;
	uint32_t end_freq;
};

/**
 * struct ch_avoid_ind_type
 * @ch_avoid_range_cnt: count
 * @avoid_freq_range: avoid freq range array
 */
struct ch_avoid_ind_type {
	uint32_t ch_avoid_range_cnt;
	struct ch_avoid_freq_type avoid_freq_range[CH_AVOID_MAX_RANGE];
};

/**
 * struct unsafe_ch_list
 * @ch_cnt: no.of channels
 * @ch_list: channel list
 */
struct unsafe_ch_list {
	uint16_t ch_cnt;
	uint16_t ch_list[NUM_CHANNELS];
};

/**
 * struct avoid_freq_ind_data
 * @freq_list: frequency list
 * @chan_list: channel list
 */
struct avoid_freq_ind_data {
	struct ch_avoid_ind_type freq_list;
	struct unsafe_ch_list chan_list;
};

#endif
