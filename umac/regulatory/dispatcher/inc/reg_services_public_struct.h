/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
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

#ifdef CONFIG_AFC_SUPPORT
#include <wlan_reg_afc.h>
#endif

#define REG_SBS_SEPARATION_THRESHOLD 100

#ifdef CONFIG_BAND_6GHZ
#define REG_MAX_CHANNELS_PER_OPERATING_CLASS        70
/*
 * These tx-power macros are present till the 6G regdomains are defined to
 * support tx-power values for various client types.
 */
#define REG_PSD_MAX_TXPOWER_FOR_DEFAULT_CLIENT      (-1) /* dBm */
#define REG_PSD_MAX_TXPOWER_FOR_SUBORDINATE_CLIENT  5    /* dBm */
#define REG_EIRP_MAX_TXPOWER_FOR_SUBORDINATE_CLIENT 24   /* dBm */
#else
#define REG_MAX_CHANNELS_PER_OPERATING_CLASS        28
#endif

#define REG_MAX_SUPP_OPER_CLASSES 32
#define REG_MAX_CHAN_CHANGE_CBKS 30
#define REG_INVALID_TXPOWER 255
#define MAX_STA_VDEV_CNT 4
#define INVALID_VDEV_ID 0xFF
#define INVALID_CHANNEL_NUM 0x0
#define CH_AVOID_MAX_RANGE   4
#define REG_ALPHA2_LEN 2
#define MAX_REG_RULES 10
#define MAX_6G_REG_RULES 5

#define REGULATORY_CHAN_DISABLED     BIT(0)
#define REGULATORY_CHAN_NO_IR        BIT(1)
#define REGULATORY_CHAN_RADAR        BIT(3)
#define REGULATORY_CHAN_NO_OFDM      BIT(6)
#define REGULATORY_CHAN_INDOOR_ONLY  BIT(9)
#define REGULATORY_CHAN_AFC          BIT(13)

#define REGULATORY_CHAN_NO_HT40      BIT(4)
#define REGULATORY_CHAN_NO_80MHZ     BIT(7)
#define REGULATORY_CHAN_NO_160MHZ    BIT(8)
#define REGULATORY_CHAN_NO_20MHZ     BIT(11)
#define REGULATORY_CHAN_NO_10MHZ     BIT(12)
#define REGULATORY_CHAN_INVALID      BIT(14)

#define REGULATORY_PHYMODE_NO11A     BIT(0)
#define REGULATORY_PHYMODE_NO11B     BIT(1)
#define REGULATORY_PHYMODE_NO11G     BIT(2)
#define REGULATORY_CHAN_NO11N        BIT(3)
#define REGULATORY_PHYMODE_NO11AC    BIT(4)
#define REGULATORY_PHYMODE_NO11AX    BIT(5)
#ifdef WLAN_FEATURE_11BE
#define REGULATORY_PHYMODE_NO11BE    BIT(6)
#endif

#define BW_5_MHZ      5
#define BW_10_MHZ     10
#define BW_20_MHZ     20
#define BW_25_MHZ     25
#define BW_40_MHZ     40
#define BW_80_MHZ     80
#define BW_160_MHZ    160
#ifdef WLAN_FEATURE_11BE
#define BW_320_MHZ    320
#endif
#define BW_40_MHZ     40

#define MAX_NUM_PWR_LEVEL 16

/**
 * enum dfs_reg - DFS region
 * @DFS_UNINIT_REGION: un-initialized region
 * @DFS_FCC_REGION: FCC region
 * @DFS_ETSI_REGION: ETSI region
 * @DFS_MKK_REGION: MKK region
 * @DFS_CN_REGION: China region
 * @DFS_KR_REGION: Korea region
 * @DFS_MKK_REGION: MKKN region
 * that supports updated W53 RADAR pattern
 * detection.
 * @DFS_UNDEF_REGION: Undefined region
 */

enum dfs_reg {
	DFS_UNINIT_REGION = 0,
	DFS_FCC_REGION = 1,
	DFS_ETSI_REGION = 2,
	DFS_MKK_REGION = 3,
	DFS_CN_REGION = 4,
	DFS_KR_REGION = 5,
	DFS_MKKN_REGION = 6,
	DFS_UNDEF_REGION = 0xFFFF,
};

/** enum op_class_table_num
 * OP_CLASS_US- Class corresponds to US
 * OP_CLASS_EU- Class corresponds to EU
 * OP_CLASS_JAPAN- Class corresponds to JAPAN
 * OP_CLASS_GLOBAL- Class corresponds to GLOBAL
 * OP_CLASS_CHINA- Class corresponds to CHINA
 */
enum op_class_table_num {
	OP_CLASS_US = 1,
	OP_CLASS_EU,
	OP_CLASS_JAPAN,
	OP_CLASS_GLOBAL,
	OP_CLASS_CHINA
};

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
 * @CHAN_ENUM_5935: channel with freq 5935
 * @CHAN_ENUM_5955: channel with freq 5955
 * @CHAN_ENUM_5975: channel with freq 5975
 * @CHAN_ENUM_5995: channel with freq 5995
 * @CHAN_ENUM_6015: channel with freq 6015
 * @CHAN_ENUM_6035: channel with freq 6035
 * @CHAN_ENUM_6055: channel with freq 6055
 * @CHAN_ENUM_6075: channel with freq 6075
 * @CHAN_ENUM_6095: channel with freq 6095
 * @CHAN_ENUM_6115: channel with freq 6115
 * @CHAN_ENUM_6135: channel with freq 6135
 * @CHAN_ENUM_6155: channel with freq 6155
 * @CHAN_ENUM_6175: channel with freq 6175
 * @CHAN_ENUM_6195: channel with freq 6195
 * @CHAN_ENUM_6215: channel with freq 6215
 * @CHAN_ENUM_6235: channel with freq 6235
 * @CHAN_ENUM_6255: channel with freq 6255
 * @CHAN_ENUM_6275: channel with freq 6275
 * @CHAN_ENUM_6295: channel with freq 6295
 * @CHAN_ENUM_6315: channel with freq 6315
 * @CHAN_ENUM_6335: channel with freq 6335
 * @CHAN_ENUM_6355: channel with freq 6355
 * @CHAN_ENUM_6375: channel with freq 6375
 * @CHAN_ENUM_6395: channel with freq 6395
 * @CHAN_ENUM_6415: channel with freq 6415
 * @CHAN_ENUM_6435: channel with freq 6435
 * @CHAN_ENUM_6455: channel with freq 6455
 * @CHAN_ENUM_6475: channel with freq 6475
 * @CHAN_ENUM_6495: channel with freq 6495
 * @CHAN_ENUM_6515: channel with freq 6515
 * @CHAN_ENUM_6535: channel with freq 6535
 * @CHAN_ENUM_6555: channel with freq 6555
 * @CHAN_ENUM_6575: channel with freq 6575
 * @CHAN_ENUM_6595: channel with freq 6595
 * @CHAN_ENUM_6615: channel with freq 6615
 * @CHAN_ENUM_6635: channel with freq 6635
 * @CHAN_ENUM_6655: channel with freq 6655
 * @CHAN_ENUM_6675: channel with freq 6675
 * @CHAN_ENUM_6695: channel with freq 6695
 * @CHAN_ENUM_6715: channel with freq 6715
 * @CHAN_ENUM_6735: channel with freq 6735
 * @CHAN_ENUM_6755: channel with freq 6755
 * @CHAN_ENUM_6775: channel with freq 6775
 * @CHAN_ENUM_6795: channel with freq 6795
 * @CHAN_ENUM_6815: channel with freq 6815
 * @CHAN_ENUM_6835: channel with freq 6835
 * @CHAN_ENUM_6855: channel with freq 6855
 * @CHAN_ENUM_6875: channel with freq 6875
 * @CHAN_ENUM_6895: channel with freq 6895
 * @CHAN_ENUM_6915: channel with freq 6915
 * @CHAN_ENUM_6935: channel with freq 6935
 * @CHAN_ENUM_6955: channel with freq 6955
 * @CHAN_ENUM_6975: channel with freq 6975
 * @CHAN_ENUM_6995: channel with freq 6995
 * @CHAN_ENUM_7015: channel with freq 7015
 * @CHAN_ENUM_7035: channel with freq 7035
 * @CHAN_ENUM_7055: channel with freq 7055
 * @CHAN_ENUM_7075: channel with freq 7075
 * @CHAN_ENUM_7095: channel with freq 7095
 * @CHAN_ENUM_7115: channel with freq 7115
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
#ifdef CONFIG_49GHZ_CHAN
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
#endif /* CONFIG_49GHZ_CHAN */
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
#ifdef WLAN_FEATURE_DSRC
	CHAN_ENUM_5850,
	CHAN_ENUM_5855,
	CHAN_ENUM_5860,
#endif
	CHAN_ENUM_5865,
#ifdef WLAN_FEATURE_DSRC
	CHAN_ENUM_5870,
	CHAN_ENUM_5875,
	CHAN_ENUM_5880,
#endif
	CHAN_ENUM_5885,
#ifdef WLAN_FEATURE_DSRC
	CHAN_ENUM_5890,
	CHAN_ENUM_5895,
	CHAN_ENUM_5900,
	CHAN_ENUM_5905,
	CHAN_ENUM_5910,
	CHAN_ENUM_5915,
	CHAN_ENUM_5920,
#endif /* WLAN_FEATURE_DSRC */
#ifdef CONFIG_BAND_6GHZ
	CHAN_ENUM_5935,
	CHAN_ENUM_5955,
	CHAN_ENUM_5975,
	CHAN_ENUM_5995,
	CHAN_ENUM_6015,
	CHAN_ENUM_6035,
	CHAN_ENUM_6055,
	CHAN_ENUM_6075,
	CHAN_ENUM_6095,
	CHAN_ENUM_6115,
	CHAN_ENUM_6135,
	CHAN_ENUM_6155,
	CHAN_ENUM_6175,
	CHAN_ENUM_6195,
	CHAN_ENUM_6215,
	CHAN_ENUM_6235,
	CHAN_ENUM_6255,
	CHAN_ENUM_6275,
	CHAN_ENUM_6295,
	CHAN_ENUM_6315,
	CHAN_ENUM_6335,
	CHAN_ENUM_6355,
	CHAN_ENUM_6375,
	CHAN_ENUM_6395,
	CHAN_ENUM_6415,
	CHAN_ENUM_6435,
	CHAN_ENUM_6455,
	CHAN_ENUM_6475,
	CHAN_ENUM_6495,
	CHAN_ENUM_6515,
	CHAN_ENUM_6535,
	CHAN_ENUM_6555,
	CHAN_ENUM_6575,
	CHAN_ENUM_6595,
	CHAN_ENUM_6615,
	CHAN_ENUM_6635,
	CHAN_ENUM_6655,
	CHAN_ENUM_6675,
	CHAN_ENUM_6695,
	CHAN_ENUM_6715,
	CHAN_ENUM_6735,
	CHAN_ENUM_6755,
	CHAN_ENUM_6775,
	CHAN_ENUM_6795,
	CHAN_ENUM_6815,
	CHAN_ENUM_6835,
	CHAN_ENUM_6855,
	CHAN_ENUM_6875,
	CHAN_ENUM_6895,
	CHAN_ENUM_6915,
	CHAN_ENUM_6935,
	CHAN_ENUM_6955,
	CHAN_ENUM_6975,
	CHAN_ENUM_6995,
	CHAN_ENUM_7015,
	CHAN_ENUM_7035,
	CHAN_ENUM_7055,
	CHAN_ENUM_7075,
	CHAN_ENUM_7095,
	CHAN_ENUM_7115,
#endif /* CONFIG_BAND_6GHZ */

	NUM_CHANNELS,

	MIN_CHANNEL = CHAN_ENUM_2412,
	MAX_CHANNEL = (NUM_CHANNELS - 1),

	MIN_24GHZ_CHANNEL = CHAN_ENUM_2412,
	MAX_24GHZ_CHANNEL = CHAN_ENUM_2484,
	NUM_24GHZ_CHANNELS = (MAX_24GHZ_CHANNEL - MIN_24GHZ_CHANNEL + 1),

	INVALID_CHANNEL = 0xBAD,

#ifdef CONFIG_49GHZ_CHAN
	MIN_49GHZ_CHANNEL = CHAN_ENUM_4912,
	MAX_49GHZ_CHANNEL = CHAN_ENUM_5080,
	NUM_49GHZ_CHANNELS = (MAX_49GHZ_CHANNEL - MIN_49GHZ_CHANNEL + 1),
#else
	MIN_49GHZ_CHANNEL = INVALID_CHANNEL,
	MAX_49GHZ_CHANNEL = INVALID_CHANNEL,
	NUM_49GHZ_CHANNELS = 0,
#endif /* CONFIG_49GHZ_CHAN */

	MIN_5GHZ_CHANNEL = CHAN_ENUM_5180,
#ifdef WLAN_FEATURE_DSRC
	MAX_5GHZ_CHANNEL = CHAN_ENUM_5920,
#else
	MAX_5GHZ_CHANNEL = CHAN_ENUM_5885,
#endif
	NUM_5GHZ_CHANNELS = (MAX_5GHZ_CHANNEL - MIN_5GHZ_CHANNEL + 1),

#ifdef WLAN_FEATURE_DSRC
	MIN_DSRC_CHANNEL = CHAN_ENUM_5850,
	MAX_DSRC_CHANNEL = CHAN_ENUM_5920,
	NUM_DSRC_CHANNELS = (MAX_DSRC_CHANNEL - MIN_DSRC_CHANNEL + 1),
#endif

	MIN_5DOT9_CHANNEL = CHAN_ENUM_5845,
	MAX_5DOT9_CHANNEL = CHAN_ENUM_5885,
	NUM_5DOT9_CHANNELS = (MAX_5DOT9_CHANNEL - MIN_5DOT9_CHANNEL + 1),

#ifdef CONFIG_49GHZ_CHAN
#define BAND_5GHZ_START_CHANNEL MIN_49GHZ_CHANNEL
#else
#define BAND_5GHZ_START_CHANNEL MIN_5GHZ_CHANNEL
#endif /* CONFIG_49GHZ_CHAN */

#ifdef DISABLE_UNII_SHARED_BANDS
	MIN_UNII_1_BAND_CHANNEL = CHAN_ENUM_5180,
	MAX_UNII_1_BAND_CHANNEL = CHAN_ENUM_5240,
	NUM_UNII_1_BAND_CHANNELS = (MAX_UNII_1_BAND_CHANNEL -
				    MIN_UNII_1_BAND_CHANNEL + 1),

	MIN_UNII_2A_BAND_CHANNEL = CHAN_ENUM_5260,
	MAX_UNII_2A_BAND_CHANNEL = CHAN_ENUM_5320,
	NUM_UNII_2A_BAND_CHANNELS = (MAX_UNII_2A_BAND_CHANNEL -
				     MIN_UNII_2A_BAND_CHANNEL + 1),
#endif

#ifdef CONFIG_BAND_6GHZ
	MIN_6GHZ_CHANNEL = CHAN_ENUM_5935,
	MAX_6GHZ_CHANNEL = CHAN_ENUM_7115,
	NUM_6GHZ_CHANNELS = (MAX_6GHZ_CHANNEL - MIN_6GHZ_CHANNEL + 1),
	MIN_6GHZ_NON_ORPHAN_CHANNEL = CHAN_ENUM_5955,
#else
	MIN_6GHZ_CHANNEL = INVALID_CHANNEL,
	MAX_6GHZ_CHANNEL = INVALID_CHANNEL,
	NUM_6GHZ_CHANNELS = 0,
	MIN_6GHZ_NON_ORPHAN_CHANNEL = INVALID_CHANNEL,
#endif /* CONFIG_BAND_6GHZ */
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
 * enum reg_6g_ap_type - Regulatory AP type for regulatory info subfield.
 * @REG_INDOOR_AP: Indoor AP
 * @REG_STANDARD_POWER_AP: Standard Power AP
 * @REG_VERY_LOW_POWER_AP: Very low power AP
 * @REG_CURRENT_MAX_AP_TYPE: current maximum, used to determine array size
 * @REG_MAX_SUPP_AP_TYPE: Current maximum AP power typer supported in the IEEE
 * standard.
 * @REG_MAX_AP_TYPE: Maximum value possible for (3 bits) regulatory info
 * sub-field in the 6G HE Operation IE
 */
enum reg_6g_ap_type {
	REG_INDOOR_AP = 0,
	REG_STANDARD_POWER_AP = 1,
	REG_VERY_LOW_POWER_AP = 2,
	REG_CURRENT_MAX_AP_TYPE,
	REG_MAX_SUPP_AP_TYPE = REG_VERY_LOW_POWER_AP,
	REG_MAX_AP_TYPE = 7,
};

/**
 * enum reg_6g_client_type - Regulatory client type for max tx-power category
 * @REG_DEFAULT_CLIENT: Default client
 * @REG_SUBORDINATE_CLIENT: Subordinate client
 * @REG_MAX_CLIENT_TYPE: Maximum value possible for max tx-power category
 * (2 bits) sub-field in the TPE (Transmit Power Envelope) IE
 */
enum reg_6g_client_type {
	REG_DEFAULT_CLIENT = 0,
	REG_SUBORDINATE_CLIENT = 1,
	REG_MAX_CLIENT_TYPE = 2,
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
 * enum ctl_value - CTL value
 * @CTL_FCC: CTL FCC
 * @CTL_MKK: CTL MKK
 * @CTL_ETSI: CTL ETSI
 * @CTL_KOR: CTL KOR
 * @CTL_CHN: CTL CHINA
 * @CTL_USER_DEF: CTL USER_DEF
 * @CTL_NONE: CTL NONE
 */
enum ctl_value {
	CTL_FCC = 0x10,
	CTL_ETSI = 0x30,
	CTL_MKK = 0x40,
	CTL_KOR = 0x50,
	CTL_CHN = 0x60,
	CTL_USER_DEF = 0x70,
	CTL_NONE = 0xff
};

/**
 * struct freq_range: The range/band of frequencies, indicated by left and right
 * edge frequencies.
 * @left: Left edge freqency(inclusive)
 * @right: Right edge freqency(inclusive)
 */
struct freq_range {
	qdf_freq_t left;
	qdf_freq_t right;
};

/**
 * struct ch_params
 * @ch_width: channel width
 * @sec_ch_offset: secondary channel offset
 * @center_freq_seg0: channel number for segment 0
 * @center_freq_seg1: channel number segment 1
 * @mhz_freq_seg0: Center frequency for segment 0
 * @mhz_freq_seg1: Center frequency for segment 1
 * @reg_punc_pattern: Output puncturing pattern
 */
struct ch_params {
	enum phy_ch_width ch_width;
	uint8_t sec_ch_offset;
	uint8_t center_freq_seg0;
	uint8_t center_freq_seg1;
	qdf_freq_t mhz_freq_seg0;
	qdf_freq_t mhz_freq_seg1;
#ifdef WLAN_FEATURE_11BE
	uint16_t reg_punc_pattern;
#endif
};

/**
 * struct reg_channel_list
 * @num_ch_params: Number of chan_param elements
 * @chan_param: Object of type struct ch_params to hold channel params
 * Currently chan_param is an array of 2 because maximum possible 320
 * channels for a given primary channel is 2. This may be dynamically
 * allocated in the future by the caller if num_ch_params is greater than 2.
 *
 */
#ifdef WLAN_FEATURE_11BE
#define MAX_NUM_CHAN_PARAM 2
struct reg_channel_list {
	uint8_t num_ch_params;
	struct ch_params chan_param[MAX_NUM_CHAN_PARAM];
};
#endif

/**
 * struct channel_power
 * @center_freq: Channel Center Frequency
 * @chan_num: channel number
 * @tx_power: TX power
 */
struct channel_power {
	qdf_freq_t center_freq;
	uint8_t chan_num;
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
	BWALL,
	BW_INVALID = 0xFF
};

/**
 * enum behav_limit - behavior limit
 * @BEHAV_NONE: none
 * @BEHAV_BW40_LOW_PRIMARY: BW40 low primary
 * @BEHAV_BW40_HIGH_PRIMARY: BW40 high primary
 * @BEHAV_BW80_PLUS: BW 80 plus
 * @BEHAV_INVALID: invalid behavior
 */
enum behav_limit {
	BEHAV_NONE,
	BEHAV_BW40_LOW_PRIMARY,
	BEHAV_BW40_HIGH_PRIMARY,
	BEHAV_BW80_PLUS,
	BEHAV_INVALID = 0xFF
};

/**
 * struct c_freq_lst: The list data strucuture for the center frequencies
 * @num_cfis: Number of center frequencies
 * @p_cfis_arr: Start address of the array of center frequency indices. Center
 *              for 40/80/160/320MHz band channel opclasses. For 20MHz the list
 *              is empty as it is already available in @channels variable.
 */
struct c_freq_lst {
	uint8_t num_cfis;
	const uint8_t *p_cfis_arr;
};

/**
 * struct reg_dmn_op_class_map_t: operating class
 * @op_class: operating class number
 * @chan_spacing: channel spacing
 * @offset: offset
 * @behav_limit: OR of bitmaps of enum behav_limit
 * @start_freq: starting frequency
 * @channels: channel set
 * @p_cfi_lst_obj: Pointer to center frequency indices list
 */
struct reg_dmn_op_class_map_t {
	uint8_t op_class;
	uint16_t chan_spacing;
	enum offset_t offset;
	uint16_t behav_limit;
	qdf_freq_t start_freq;
	uint8_t channels[REG_MAX_CHANNELS_PER_OPERATING_CLASS];
	const struct c_freq_lst *p_cfi_lst_obj;
};

/**
 * struct regdmn_ap_cap_opclass_t: AP Cap operation class table
 * @op_class: operating class number
 * @ch_width: channel width in MHz
 * @start_freq: Starting Frequency in MHz
 * @behav_limit: OR of bitmaps of enum behav_limit
 * @max_tx_pwr_dbm: Maximum tx power in dbm
 * @num_supported_chan: Number of supported channels
 * @num_non_supported_chan: Number of non-supported channels
 * @sup_chan_list: Array of supported channel numbers
 * @non_sup_chan_list: Array of non supported channel numbers
 */
struct regdmn_ap_cap_opclass_t {
	uint8_t op_class;
	uint8_t ch_width;
	qdf_freq_t start_freq;
	uint16_t behav_limit;
	uint8_t max_tx_pwr_dbm;
	uint8_t num_supported_chan;
	uint8_t num_non_supported_chan;
	uint8_t sup_chan_list[REG_MAX_CHANNELS_PER_OPERATING_CLASS];
	uint8_t non_sup_chan_list[REG_MAX_CHANNELS_PER_OPERATING_CLASS];
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
 * struct reg_11d_scan_msg: 11d scan message structure
 * @psoc: pointer to psoc object
 * @enable_11d_supp: enable 11d scan or disable 11d scan
 */
struct reg_11d_scan_msg {
	struct wlan_objmgr_psoc *psoc;
	bool enable_11d_supp;
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

#ifdef WLAN_FEATURE_GET_USABLE_CHAN_LIST
/**
 * enum iftype - (virtual) interface types
 *
 * @IFTYPE_UNSPECIFIED: unspecified type, driver decides
 * @IFTYPE_ADHOC: independent BSS member
 * @IFTYPE_STATION: managed BSS member
 * @IFTYPE_AP: access point
 * @IFTYPE_AP_VLAN: VLAN interface for access points; VLAN interfaces
 *      are a bit special in that they must always be tied to a pre-existing
 *      AP type interface.
 * @IFTYPE_WDS: wireless distribution interface
 * @IFTYPE_MONITOR: monitor interface receiving all frames
 * @IFTYPE_MESH_POINT: mesh point
 * @IFTYPE_P2P_CLIENT: P2P client
 * @IFTYPE_P2P_GO: P2P group owner
 * @IFTYPE_P2P_DEVICE: P2P device interface type, this is not a netdev
 *      and therefore can't be created in the normal ways, use the
 *      %NL80211_CMD_START_P2P_DEVICE and %NL80211_CMD_STOP_P2P_DEVICE
 *      commands to create and destroy one
 * @IF_TYPE_OCB: Outside Context of a BSS
 *      This mode corresponds to the MIB variable dot11OCBActivated=true
 * @IF_TYPE_NAN: NAN mode
 * @IFTYPE_MAX: highest interface type number currently defined
 * @NUM_IFTYPES: number of defined interface types
 *
 * These values are used with the %NL80211_ATTR_IFTYPE
 * to set the type of an interface.
 *
 */
enum iftype {
	IFTYPE_UNSPECIFIED,
	IFTYPE_ADHOC,
	IFTYPE_STATION,
	IFTYPE_AP,
	IFTYPE_AP_VLAN,
	IFTYPE_WDS,
	IFTYPE_MONITOR,
	IFTYPE_MESH_POINT,
	IFTYPE_P2P_CLIENT,
	IFTYPE_P2P_GO,
	IFTYPE_P2P_DEVICE,
	IFTYPE_OCB,
	IFTYPE_NAN,

	/* keep last */
	NUM_IFTYPES,
	IFTYPE_MAX = NUM_IFTYPES - 1
};

/**
 * usable_channels_filter - Filters to get usable channels
 * FILTER_CELLULAR_COEX: Avoid lte coex channels
 * FILTER_WLAN_CONCURRENCY: Avoid con channels
 **/
enum usable_channels_filter {
	FILTER_CELLULAR_COEX = 0,
	FILTER_WLAN_CONCURRENCY = 1,
};

/**
 * get_usable_chan_res_params - Usable channels resp params
 * freq : center freq
 * seg0_freq : seg0 freq
 * seg1_freq: seg1 freq
 * bw : bandwidth
 * state: channel state
 * iface_mode_mask: interface mode mask
 **/
struct get_usable_chan_res_params {
	qdf_freq_t freq;
	uint32_t seg0_freq;
	uint32_t seg1_freq;
	enum phy_ch_width bw;
	uint32_t iface_mode_mask;
	enum channel_state state;
};

/**
 * get_usable_chan_req_params - Usable channels req params
 * band_mask : band mask
 * iface_mode_mask: interface mode mask
 * filter_mask: filter mask
 **/
struct get_usable_chan_req_params {
	uint32_t band_mask;
	uint32_t iface_mode_mask;
	uint32_t filter_mask;
};
#endif

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
 * @nol_history: Set NOL-History when STA vap detects RADAR.
 * @is_chan_hop_blocked: Whether channel is blocked for ACS hopping.
 * @ht40intol_flags: Contains Flags to indicate whether the 40PLUS/40MINUS
 *                   version of the channel is blocked by ACS due to
 *                   intolerance.
 * @psd_flag: is PSD channel or not
 * @psd_eirp: PSD power level
 */
struct regulatory_channel {
	qdf_freq_t center_freq;
	uint8_t chan_num;
	enum channel_state state;
	uint32_t chan_flags;
	uint32_t tx_power;
	uint16_t min_bw;
	uint16_t max_bw;
	uint8_t ant_gain;
	bool nol_chan;
	bool nol_history;
#ifdef CONFIG_HOST_FIND_CHAN
	bool is_chan_hop_blocked;
	uint8_t ht40intol_flags;
#endif
#ifdef CONFIG_BAND_6GHZ
	bool psd_flag;
	uint16_t psd_eirp;
#endif
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
	qdf_freq_t center_freq;
	uint8_t chan_num;
	uint16_t min_bw;
	uint16_t max_bw;
};

/**
 * struct bonded_channel
 * @start_ch: start channel
 * @end_ch: end channel
 */
struct bonded_channel {
	uint8_t start_ch;
	uint8_t end_ch;
};

/**
 * struct bonded_channel_freq
 * @start_freq: start channel frequency
 * @end_freq: end channel frequency
 */
struct bonded_channel_freq {
	uint16_t start_freq;
	uint16_t end_freq;
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
 * @psd_flag: is PSD power used
 * @psd_eirp: maximum PSD EIRP value
 */
struct cur_reg_rule {
	uint16_t start_freq;
	uint16_t end_freq;
	uint16_t max_bw;
	uint8_t reg_power;
	uint8_t ant_gain;
	uint16_t flags;
	bool psd_flag;
	uint16_t psd_eirp;
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
 * @num_5g_reg_rules: number 5G  and 6G reg rules
 * @reg_rules_2g_ptr: ptr to 2G reg rules
 * @reg_rules_5g_ptr: ptr to 5G reg rules
 * @client_type: type of client
 * @rnr_tpe_usable: if RNR TPE octet is usable for country
 * @unspecified_ap_usable: if not set, AP usable for country
 * @domain_code_6g_ap: domain code for 6G AP
 * @domain_code_6g_client: domain code for 6G client in SP mode
 * @domain_code_6g_super_id: 6G super domain ID
 * @min_bw_6g_ap: minimum 6G bw for AP
 * @max_bw_6g_ap: maximum 6G bw for AP
 * @min_bw_6g_client: list of minimum 6G bw for clients
 * @max_bw_6g_client: list of maximum 6G bw for clients
 * @num_6g_reg_rules_ap: number of 6G reg rules for AP
 * @num_6g_reg_rules_client: list of number of 6G reg rules for client
 * @reg_rules_6g_ap_ptr: ptr to 6G AP reg rules
 * @reg_rules_6g_client_ptr: list of ptr to 6G client reg rules
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
	enum reg_6g_client_type client_type;
	bool rnr_tpe_usable;
	bool unspecified_ap_usable;
	uint8_t domain_code_6g_ap[REG_CURRENT_MAX_AP_TYPE];
	uint8_t domain_code_6g_client[REG_CURRENT_MAX_AP_TYPE][REG_MAX_CLIENT_TYPE];
	uint32_t domain_code_6g_super_id;
	uint32_t min_bw_6g_ap[REG_CURRENT_MAX_AP_TYPE];
	uint32_t max_bw_6g_ap[REG_CURRENT_MAX_AP_TYPE];
	uint32_t min_bw_6g_client[REG_CURRENT_MAX_AP_TYPE][REG_MAX_CLIENT_TYPE];
	uint32_t max_bw_6g_client[REG_CURRENT_MAX_AP_TYPE][REG_MAX_CLIENT_TYPE];
	uint32_t num_6g_reg_rules_ap[REG_CURRENT_MAX_AP_TYPE];
	uint32_t num_6g_reg_rules_client[REG_CURRENT_MAX_AP_TYPE][REG_MAX_CLIENT_TYPE];
	struct cur_reg_rule *reg_rules_6g_ap_ptr[REG_CURRENT_MAX_AP_TYPE];
	struct cur_reg_rule *reg_rules_6g_client_ptr[REG_CURRENT_MAX_AP_TYPE][REG_MAX_CLIENT_TYPE];
};

#if defined(CONFIG_AFC_SUPPORT) && defined(CONFIG_BAND_6GHZ)

/**
 * reg_afc_event_type indicates the type of AFC event sent from FW to host.
 * 1. For sending Power Info REG_AFC_EVENT_POWER_INFO event is used.
 * 2. For sending AFC expiry use REG_AFC_EVENT_TIMER_EXPIRY
 * This type can be expanded in future as per requirements.
 */
enum reg_afc_event_type {
	REG_AFC_EVENT_POWER_INFO = 1,
	REG_AFC_EVENT_TIMER_EXPIRY = 2,
};

/**
 * reg_afc_expiry_event_subtype indicates the subtype.
 * 1. At boot up AFC expiry will contain AFC start.
 * 2. If AFC timer expires AFC_RENEW status code will be sent to host
 *    with expiry event.
 * 3. If AFC server is not responding to FW in specified time, FW will
 *    indicate host to switch to LPI.
 */
enum reg_afc_expiry_event_subtype {
	REG_AFC_EXPIRY_EVENT_START = 1,
	REG_AFC_EXPIRY_EVENT_RENEW = 2,
	REG_AFC_EXPIRY_EVENT_SWITCH_TO_LPI = 3,
};

/**
 * The following fw_status_code is mutually exclusive
 * and is used in power event.
 * 0. AFC power event is success.
 * 1. If Host does not indicate AFC indication cmd within certain time
 *    of AFC expiry, REG_FW_AFC_POWER_EVENT_RESP_NOT_RECEIVED will be used.
 * 2. If FW is not able to parse afc_info, parsing_failure will be
 *    indicated using REG_FW_AFC_POWER_EVENT_RESP_NOT_RECEIVED.
 * 3. If due to some local reason AFC event is failed, AFC event failure
 *    is indicated using REG_FW_AFC_POWER_EVENT_FAILURE.
 */
enum reg_fw_afc_power_event_status_code {
	REG_FW_AFC_POWER_EVENT_SUCCESS = 0,
	REG_FW_AFC_POWER_EVENT_RESP_NOT_RECEIVED = 1,
	REG_FW_AFC_POWER_EVENT_RESP_PARSING_FAILURE = 2,
	REG_FW_AFC_POWER_EVENT_FAILURE = 3,
};

/**
 * The following reg_afc_server_resp_code is mutually exclusive.
 * This response code will be indicated to AFC server.
 * These codes are defined in WIFI spec doc for AFC as follows
 * -1: General Failure
 * 0:  Success
 * 100 - 199: General errors related to the protocol
 * 300 - 399: Error events specific to message exchange for the
 *            Available Spectrum Inquiry
 */
enum reg_afc_serv_resp_code {
	REG_AFC_SERV_RESP_GENERAL_FAILURE = -1,
	REG_AFC_SERV_RESP_SUCCESS = 0,
	REG_AFC_SERV_RESP_VERSION_NOT_SUPPORTED = 100,
	REG_AFC_SERV_RESP_DEVICE_UNALLOWED = 101,
	REG_AFC_SERV_RESP_MISSING_PARAM = 102,
	REG_AFC_SERV_RESP_INVALID_VALUE = 103,
	REG_AFC_SERV_RESP_UNEXPECTED_PARAM = 106,
	REG_AFC_SERV_RESP_UNSUPPORTED_SPECTRUM = 300,
};

/**
 * struct afc_freq_obj
 * @low_freq: Lower edge frequency
 * @high_freq: Upper edge frequency
 * @max_psd: Max PSD in 0.01 dBm/MHz units
 */
struct afc_freq_obj {
	qdf_freq_t low_freq;
	qdf_freq_t high_freq;
	int16_t max_psd;
};

/**
 * struct chan_eirp_obj
 * @cfi: Channel frequency index
 * @eirp_power: Max EIRP power in 0.01 dBm units
 */
struct chan_eirp_obj {
	uint8_t cfi;
	uint16_t eirp_power;
};

/**
 * struct afc_chan_obj
 * @global_opclass: Global Operating class
 * @num_chans: Number of channels
 * @chan_eirp_info: Pointer to afc channel EIRP object
 */
struct afc_chan_obj {
	uint8_t global_opclass;
	uint8_t num_chans;
	struct chan_eirp_obj *chan_eirp_info;
};

/**
 * struct reg_afc_expiry_event
 * @request_id: AFC request id generated by the firmware
 * @event_subtype: AFC expiry event subtype
 */
struct reg_afc_expiry_event {
	uint32_t request_id;
	enum reg_afc_expiry_event_subtype event_subtype;
};

/**
 * struct reg_fw_afc_power_event
 * @resp_id: AFC server response id
 * @fw_status_code: firmware status code
 * @serv_resp_code: AFC server response code
 * @afc_wfa_version: AFC version
 * @avail_exp_time_d: Expiry date of the AFC power info
 * @avail_exp_time_t: Time left for expiry of the AFC power info
 * @num_freq_objs: Number of freq objects
 * @num_chan_objs: Number of channel objects
 * @afc_freq_info: Pointer to AFC freq object
 * @afc_chan_info: Pointer to AFC channel object
 */
struct reg_fw_afc_power_event {
	uint8_t resp_id;
	enum reg_fw_afc_power_event_status_code fw_status_code;
	enum reg_afc_serv_resp_code serv_resp_code;
	uint32_t afc_wfa_version;
	uint32_t avail_exp_time_d;
	uint32_t avail_exp_time_t;
	uint8_t num_freq_objs;
	uint8_t num_chan_objs;
	struct afc_freq_obj *afc_freq_info;
	struct afc_chan_obj *afc_chan_info;
};

/**
 * struct afc_regulatory_info
 * @psoc: psoc ptr
 * @phy_id: phy id
 * @event_type: AFC event type
 * @expiry_info: pointer to information present in the AFC expiry event
 * @power_info: pointer to information present in the AFC power event
 */
struct afc_regulatory_info {
	struct wlan_objmgr_psoc *psoc;
	uint8_t phy_id;
	enum reg_afc_event_type event_type;
	union {
		struct reg_afc_expiry_event *expiry_info;
		struct reg_fw_afc_power_event *power_info;
	};
};
#endif

/**
 * struct reg_rule_info
 * @alpha2: alpha2 of reg rules
 * @dfs_region: dfs region
 * @num_of_reg_rules: number of reg rules
 * @reg_rules: regulatory rules array
 * @num_of_6g_client_reg_rules: number of 6g reg rules
 * @reg_rules_6g_client: reg rules for all 6g clients
 */
struct reg_rule_info {
	uint8_t alpha2[REG_ALPHA2_LEN + 1];
	enum dfs_reg dfs_region;
	uint8_t num_of_reg_rules;
	struct cur_reg_rule reg_rules[MAX_REG_RULES];
#ifdef CONFIG_BAND_6GHZ
	uint8_t num_of_6g_ap_reg_rules[REG_CURRENT_MAX_AP_TYPE];
	struct cur_reg_rule reg_rules_6g_ap[REG_CURRENT_MAX_AP_TYPE][MAX_6G_REG_RULES];
	uint8_t num_of_6g_client_reg_rules[REG_CURRENT_MAX_AP_TYPE];
	struct cur_reg_rule reg_rules_6g_client[REG_CURRENT_MAX_AP_TYPE][MAX_6G_REG_RULES];
#endif
};

/**
 * enum reg_reg_wifi_band
 * @REG_BAND_2G: 2G band
 * @REG_BAND_5G: 5G band
 * @REG_BAND_6G: 6G band
 * @REG_BAND_UNKNOWN: Unsupported band
 */
enum reg_wifi_band {
	REG_BAND_2G,
	REG_BAND_5G,
	REG_BAND_6G,
	REG_BAND_UNKNOWN
};

#ifdef DISABLE_UNII_SHARED_BANDS
/**
 * enum reg_unii_band
 * @REG_UNII_BAND_1: Disable UNII-1 band channels
 * @REG_UNII_BAND_2A: Disable UNII-2A band channels
 */
enum reg_unii_band {
	REG_UNII_BAND_1 = 0x0,
	REG_UNII_BAND_2A = 0x1,
};
#endif

#define REG_BAND_MASK_ALL (BIT(REG_BAND_2G) | BIT(REG_BAND_5G) \
			  | BIT(REG_BAND_6G))

/* Avoid the use of band_info as it does not support 6GHz band. Use
 * reg_wifi_band, as it supports the 6GHz band
 */
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
 * @enable_srd_chan_in_master_mode: SRD channel support in master mode
 * @enable_11d_in_world_mode: enable 11d in world mode
 * @enable_5dot9_ghz_chan_in_master_mode: 5.9 GHz channel support in
 * master mode
 * @retain_nol_across_regdmn_update: Retain the NOL list across the regdomain.
 */
struct reg_config_vars {
	uint32_t enable_11d_support;
	uint32_t scan_11d_interval;
	uint32_t userspace_ctry_priority;
	uint32_t band_capability;
	uint32_t dfs_enabled;
	uint32_t indoor_chan_enabled;
	uint32_t force_ssc_disable_indoor_channel;
	enum restart_beaconing_on_ch_avoid_rule restart_beaconing;
	uint8_t enable_srd_chan_in_master_mode;
	bool enable_11d_in_world_mode;
	bool enable_5dot9_ghz_chan_in_master_mode;
	bool retain_nol_across_regdmn_update;
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
 * @mas_chan_list: master chan list for 2GHz and 5GHz channels
 * @is_6g_channel_list_populated: indicates the channel lists are populated
 * @mas_chan_list_6g_ap: master chan list for 6GHz AP channels
 * @mas_chan_list_6g_client: master chan list for 6GHz client
 * @is_6g_afc_power_event_received: indicates if the AFC event is received.
 * @mas_chan_list_6g_afc: master chan list for 6GHz AFC
 * @default_country: default country
 * @current_country: current country
 * @def_region_domain: default reg domain
 * @def_country_code: default country code
 * @reg_dmn_pair: reg domain pair
 * @reg_6g_superid: 6G super domain ID
 * @ctry_code: country code
 * @reg_rules: regulatory rules
 * @client_type: type of client
 * @rnr_tpe_usable: if RNR TPE octet is usable for country
 * @unspecified_ap_usable: if not set, AP usable for country
 */
struct mas_chan_params {
	enum dfs_reg dfs_region;
	uint32_t phybitmap;
	struct regulatory_channel mas_chan_list[NUM_CHANNELS];
#ifdef CONFIG_BAND_6GHZ
	bool is_6g_channel_list_populated;
	struct regulatory_channel mas_chan_list_6g_ap[REG_CURRENT_MAX_AP_TYPE][NUM_6GHZ_CHANNELS];
	struct regulatory_channel mas_chan_list_6g_client[REG_CURRENT_MAX_AP_TYPE][REG_MAX_CLIENT_TYPE][NUM_6GHZ_CHANNELS];
#ifdef CONFIG_AFC_SUPPORT
	bool is_6g_afc_power_event_received;
	struct regulatory_channel mas_chan_list_6g_afc[NUM_6GHZ_CHANNELS];
#endif
#endif
	char default_country[REG_ALPHA2_LEN + 1];
	char current_country[REG_ALPHA2_LEN + 1];
	uint16_t def_region_domain;
	uint16_t def_country_code;
	uint32_t reg_dmn_pair;
	uint16_t reg_6g_superid;
	uint16_t ctry_code;
	struct reg_rule_info reg_rules;
#ifdef CONFIG_BAND_6GHZ
	enum reg_6g_ap_type ap_pwr_type;
	enum reg_6g_client_type client_type;
	bool rnr_tpe_usable;
	bool unspecified_ap_usable;
#endif
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
 * @country_code:     Country code
 * @reg_2g_5g_pair_id:  Regdomain pair ID (2Ghz + 5Ghz domain pair)
 * @sixg_superdmn_id: 6Ghz super domain id
 * @alpha:            Country ISO
 * @flags:            Regdomain flags
 */
struct cc_regdmn_s {
	union {
		uint16_t country_code;
		struct {
			uint16_t reg_2g_5g_pair_id;
			uint16_t sixg_superdmn_id;
		} regdmn;
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
	qdf_freq_t start_freq;
	qdf_freq_t end_freq;
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
 * @chan_cnt: no.of channels
 * @chan_freq_list: channel frequency list
 */
struct unsafe_ch_list {
	uint16_t chan_cnt;
	uint16_t chan_freq_list[NUM_CHANNELS];
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

/**
 * struct reg_sched_payload
 * @psoc: psoc ptr
 * @pdev: pdev ptr
 * @ch_avoid_ind: if avoidance event indicated
 * @avoid_info: chan avoid info if @ch_avoid_ind is true
 */
struct reg_sched_payload {
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	bool ch_avoid_ind;
	struct avoid_freq_ind_data avoid_info;
};

#define FIVEG_STARTING_FREQ        5000
#define TWOG_STARTING_FREQ         2407
#define TWOG_CHAN_14_IN_MHZ        2484
#define TWOG_CHAN_1_IN_MHZ         2412
#define TWOG_CHAN_5_IN_MHZ         2432
#define TWOG_CHAN_6_IN_MHZ         2437
#define TWOG_CHAN_9_IN_MHZ         2452
#define TWOG_CHAN_13_IN_MHZ        2472

#define HT40_SEC_OFFSET            20

/**
 * struct reg_ctl_params - reg ctl and regd info
 * @regd: regdomain pair
 * @regd_2g: 2g sub domain code
 * @regd_5g: 5g sub domain code
 * @ctl_2g: 2g ctl info
 * @ctl_5g: 5g ctl info
 */
struct reg_ctl_params {
	uint32_t regd;
	uint16_t regd_2g;
	uint16_t regd_5g;
	uint8_t ctl_2g;
	uint8_t ctl_5g;
};

/**
 * enum reg_phymode - Regulatory phymode
 * @REG_PHYMODE_INVALID: Invalid phymode
 * @REG_PHYMODE_11B: 802.11b phymode
 * @REG_PHYMODE_11G: 802.11g phymode
 * @REG_PHYMODE_11A: 802.11a phymode
 * @REG_PHYMODE_11N: 802.11n phymode
 * @REG_PHYMODE_11AC: 802.11ac phymode
 * @REG_PHYMODE_11AX: 802.11ax phymode
 * @REG_PHYMODE_11BE: 802.11be phymode
 * @REG_PHYMODE_MAX: placeholder for future phymodes
 */
enum reg_phymode {
	REG_PHYMODE_INVALID,
	REG_PHYMODE_11B,
	REG_PHYMODE_11G,
	REG_PHYMODE_11A,
	REG_PHYMODE_11N,
	REG_PHYMODE_11AC,
	REG_PHYMODE_11AX,
#ifdef WLAN_FEATURE_11BE
	REG_PHYMODE_11BE,
#endif
	REG_PHYMODE_MAX,
};

/**
 * struct chan_power_info - TPE containing power info per channel chunk
 * @chan_cfreq: channel center freq (MHz)
 * @tx_power: transmit power (dBm)
 */
struct chan_power_info {
	qdf_freq_t chan_cfreq;
	uint8_t tx_power;
};

/**
 * struct reg_tpc_power_info - regulatory TPC power info
 * @is_psd_power: is PSD power or not
 * @eirp_power: Maximum EIRP power (dBm), valid only if power is PSD
 * @power_type_6g: type of power (SP/LPI/VLP)
 * @num_pwr_levels: number of power levels
 * @reg_max: Array of maximum TX power (dBm) per PSD value
 * @ap_constraint_power: AP constraint power (dBm)
 * @frequency: Array of operating frequency
 * @tpe: TPE values processed from TPE IE
 * @chan_power_info: power info to send to FW
 */
struct reg_tpc_power_info {
	bool is_psd_power;
	uint8_t eirp_power;
	uint8_t power_type_6g;
	uint8_t num_pwr_levels;
	uint8_t reg_max[MAX_NUM_PWR_LEVEL];
	uint8_t ap_constraint_power;
	qdf_freq_t frequency[MAX_NUM_PWR_LEVEL];
	uint8_t tpe[MAX_NUM_PWR_LEVEL];
	struct chan_power_info chan_power_info[MAX_NUM_PWR_LEVEL];
};

#ifdef FEATURE_WLAN_CH_AVOID_EXT
typedef struct unsafe_ch_list avoid_ch_ext_list;
/**
 * struct chan_5g_center_freq
 * @center_freq_20: center frequency of max 200Mhz
 * @center_freq_40: center frequency of max 40Mhz
 * @center_freq_80: center frequency of max 80Mhz
 * @center_freq_160: center frequency of max 160Mhz
 */
struct chan_5g_center_freq {
	qdf_freq_t center_freq_20;
	qdf_freq_t center_freq_40;
	qdf_freq_t center_freq_80;
	qdf_freq_t center_freq_160;
};

#define INVALID_CENTER_FREQ 0
/*MAX 5g channel numbers, not include dsrc*/
#define MAX_5G_CHAN_NUM 28

#endif

/**
 * enum HOST_REGDMN_MODE:
 * @HOST_REGDMN_MODE_11A: 11a channels
 * @HOST_REGDMN_MODE_TURBO: 11a turbo-only channels
 * @HOST_REGDMN_MODE_11B: 11b channels
 * @HOST_REGDMN_MODE_PUREG: 11g channels (OFDM only)
 * @HOST_REGDMN_MODE_11G: historical
 * @HOST_REGDMN_MODE_108G: 11g+Turbo channels
 * @HOST_REGDMN_MODE_108A: 11a+Turbo channels
 * @HOST_REGDMN_MODE_11AC_VHT20_2G: 2GHz, VHT20
 * @HOST_REGDMN_MODE_XR: XR channels
 * @HOST_REGDMN_MODE_11A_HALF_RATE: 11a half rate channels
 * @HOST_REGDMN_MODE_11A_QUARTER_RATE: 11a quarter rate channels
 * @HOST_REGDMN_MODE_11NG_HT20: 11ng HT20 channels
 * @HOST_REGDMN_MODE_11NA_HT20: 11na HT20 channels
 * @HOST_REGDMN_MODE_11NG_HT40PLUS: 11ng HT40+ channels
 * @HOST_REGDMN_MODE_11NG_HT40MINUS: 11ng HT40- channels
 * @HOST_REGDMN_MODE_11NA_HT40PLUS: 11na HT40+ channels
 * @HOST_REGDMN_MODE_11NA_HT40MINUS: 11na HT40- channels
 * @HOST_REGDMN_MODE_11AC_VHT20: 5GHz, VHT20
 * @HOST_REGDMN_MODE_11AC_VHT40PLUS: 5GHz, VHT40+ channels
 * @HOST_REGDMN_MODE_11AC_VHT40MINUS: 5GHz, VHT40- channels
 * @HOST_REGDMN_MODE_11AC_VHT80: 5GHz, VHT80 channels
 * @HOST_REGDMN_MODE_11AC_VHT160: 5GHz, VHT160 channels
 * @HOST_REGDMN_MODE_11AC_VHT80_80: 5GHz, VHT80+80 channels
 * @HOST_REGDMN_MODE_11AXG_HE20: 11ax 2.4GHz, HE20 channels
 * @HOST_REGDMN_MODE_11AXA_HE20: 11ax 5GHz, HE20 channels
 * @HOST_REGDMN_MODE_11AXG_HE40PLUS: 11ax 2.4GHz, HE40+ channels
 * @HOST_REGDMN_MODE_11AXG_HE40MINUS: 11ax 2.4GHz, HE40- channels
 * @HOST_REGDMN_MODE_11AXA_HE40PLUS: 11ax 5GHz, HE40+ channels
 * @HOST_REGDMN_MODE_11AXA_HE40MINUS: 11ax 5GHz, HE40- channels
 * @HOST_REGDMN_MODE_11AXA_HE80: 11ax 5GHz, HE80 channels
 * @HOST_REGDMN_MODE_11AXA_HE160: 11ax 5GHz, HE160 channels
 * @HOST_REGDMN_MODE_11AXA_HE80_80: 11ax 5GHz, HE80+80 channels
 * @HOST_REGDMN_MODE_11BEG_EHT20: 11be 2.4GHz, EHT20 channels
 * @HOST_REGDMN_MODE_11BEA_EHT20: 11be 5GHz, EHT20 channels
 * @HOST_REGDMN_MODE_11BEG_EHT40PLUS: 11be 2.4GHz, EHT40+ channels
 * @HOST_REGDMN_MODE_11BEG_EHT40MINUS: 11be 2.4GHz, EHT40- channels
 * @HOST_REGDMN_MODE_11BEA_EHT40PLUS: 11be 5GHz, EHT40+ channels
 * @HOST_REGDMN_MODE_11BEA_EHT40MINUS: 11be 5GHz, EHT40- channels
 * @HOST_REGDMN_MODE_11BEA_EHT80: 11be 5GHz, EHT80 channels
 * @HOST_REGDMN_MODE_11BEA_EHT160: 11be 5GHz, EHT160 channels
 * @HOST_REGDMN_MODE_11BEA_EHT320: 11be 5GHz, EHT320 channels
 */
enum {
	HOST_REGDMN_MODE_11A = 0x00000001,
	HOST_REGDMN_MODE_TURBO = 0x00000002,
	HOST_REGDMN_MODE_11B = 0x00000004,
	HOST_REGDMN_MODE_PUREG = 0x00000008,
	HOST_REGDMN_MODE_11G = 0x00000008,
	HOST_REGDMN_MODE_108G = 0x00000020,
	HOST_REGDMN_MODE_108A = 0x00000040,
	HOST_REGDMN_MODE_11AC_VHT20_2G = 0x00000080,
	HOST_REGDMN_MODE_XR = 0x00000100,
	HOST_REGDMN_MODE_11A_HALF_RATE = 0x00000200,
	HOST_REGDMN_MODE_11A_QUARTER_RATE = 0x00000400,
	HOST_REGDMN_MODE_11NG_HT20 = 0x00000800,
	HOST_REGDMN_MODE_11NA_HT20 = 0x00001000,
	HOST_REGDMN_MODE_11NG_HT40PLUS = 0x00002000,
	HOST_REGDMN_MODE_11NG_HT40MINUS = 0x00004000,
	HOST_REGDMN_MODE_11NA_HT40PLUS = 0x00008000,
	HOST_REGDMN_MODE_11NA_HT40MINUS = 0x00010000,
	HOST_REGDMN_MODE_11AC_VHT20 = 0x00020000,
	HOST_REGDMN_MODE_11AC_VHT40PLUS = 0x00040000,
	HOST_REGDMN_MODE_11AC_VHT40MINUS = 0x00080000,
	HOST_REGDMN_MODE_11AC_VHT80 = 0x00100000,
	HOST_REGDMN_MODE_11AC_VHT160 = 0x00200000,
	HOST_REGDMN_MODE_11AC_VHT80_80 = 0x00400000,
	HOST_REGDMN_MODE_11AXG_HE20 = 0x00800000,
	HOST_REGDMN_MODE_11AXA_HE20 = 0x01000000,
	HOST_REGDMN_MODE_11AXG_HE40PLUS = 0x02000000,
	HOST_REGDMN_MODE_11AXG_HE40MINUS = 0x04000000,
	HOST_REGDMN_MODE_11AXA_HE40PLUS = 0x08000000,
	HOST_REGDMN_MODE_11AXA_HE40MINUS = 0x10000000,
	HOST_REGDMN_MODE_11AXA_HE80 = 0x20000000,
	HOST_REGDMN_MODE_11AXA_HE160 = 0x40000000,
	HOST_REGDMN_MODE_11AXA_HE80_80 = 0x80000000,
#ifdef WLAN_FEATURE_11BE
	HOST_REGDMN_MODE_11BEG_EHT20 = 0x0000000100000000,
	HOST_REGDMN_MODE_11BEA_EHT20 = 0x0000000200000000,
	HOST_REGDMN_MODE_11BEG_EHT40PLUS = 0x0000000400000000,
	HOST_REGDMN_MODE_11BEG_EHT40MINUS = 0x0000000800000000,
	HOST_REGDMN_MODE_11BEA_EHT40PLUS = 0x0000001000000000,
	HOST_REGDMN_MODE_11BEA_EHT40MINUS = 0x0000002000000000,
	HOST_REGDMN_MODE_11BEA_EHT80 = 0x0000004000000000,
	HOST_REGDMN_MODE_11BEA_EHT160 = 0x0000008000000000,
	HOST_REGDMN_MODE_11BEA_EHT320 = 0x0000010000000000,
#endif
	HOST_REGDMN_MODE_ALL = 0xffffffffffffffff
};

#define WIRELESS_11AX_MODES  (HOST_REGDMN_MODE_11AXG_HE20 \
			      | HOST_REGDMN_MODE_11AXG_HE40PLUS \
			      | HOST_REGDMN_MODE_11AXG_HE40MINUS \
			      | HOST_REGDMN_MODE_11AXA_HE20 \
			      | HOST_REGDMN_MODE_11AXA_HE40PLUS \
			      | HOST_REGDMN_MODE_11AXA_HE40MINUS \
			      | HOST_REGDMN_MODE_11AXA_HE80 \
			      | HOST_REGDMN_MODE_11AXA_HE160 \
			      | HOST_REGDMN_MODE_11AXA_HE80_80)

#ifdef WLAN_FEATURE_11BE
#define WIRELESS_11BE_MODES  (HOST_REGDMN_MODE_11BEG_EHT20 \
			      | HOST_REGDMN_MODE_11BEA_EHT20 \
			      | HOST_REGDMN_MODE_11BEG_EHT40PLUS \
			      | HOST_REGDMN_MODE_11BEG_EHT40MINUS \
			      | HOST_REGDMN_MODE_11BEA_EHT40PLUS \
			      | HOST_REGDMN_MODE_11BEA_EHT40MINUS \
			      | HOST_REGDMN_MODE_11BEA_EHT80 \
			      | HOST_REGDMN_MODE_11BEA_EHT160 \
			      | HOST_REGDMN_MODE_11BEA_EHT320)
#endif

#define WIRELESS_11AC_MODES  (HOST_REGDMN_MODE_11AC_VHT20 \
			      | HOST_REGDMN_MODE_11AC_VHT40PLUS \
			      | HOST_REGDMN_MODE_11AC_VHT40MINUS \
			      | HOST_REGDMN_MODE_11AC_VHT80 \
			      | HOST_REGDMN_MODE_11AC_VHT160 \
			      | HOST_REGDMN_MODE_11AC_VHT80_80)

#define WIRELESS_11N_MODES   (HOST_REGDMN_MODE_11NG_HT20 \
			      | HOST_REGDMN_MODE_11NA_HT20 \
			      | HOST_REGDMN_MODE_11NG_HT40PLUS \
			      | HOST_REGDMN_MODE_11NG_HT40MINUS \
			      | HOST_REGDMN_MODE_11NA_HT40PLUS \
			      | HOST_REGDMN_MODE_11NA_HT40MINUS)

#define WIRELESS_11G_MODES   (HOST_REGDMN_MODE_PUREG \
			      | HOST_REGDMN_MODE_11G \
			      | HOST_REGDMN_MODE_108G)

#define WIRELESS_11B_MODES   (HOST_REGDMN_MODE_11B)

#define WIRELESS_11A_MODES   (HOST_REGDMN_MODE_11A \
			      | HOST_REGDMN_MODE_TURBO \
			      | HOST_REGDMN_MODE_108A \
			      | HOST_REGDMN_MODE_11A_HALF_RATE \
			      | HOST_REGDMN_MODE_11A_QUARTER_RATE)

#ifdef WLAN_FEATURE_11BE
#define WIRELESS_20_MODES    (HOST_REGDMN_MODE_11A \
			      | HOST_REGDMN_MODE_TURBO \
			      | HOST_REGDMN_MODE_11B \
			      | HOST_REGDMN_MODE_PUREG \
			      | HOST_REGDMN_MODE_11G \
			      | HOST_REGDMN_MODE_11AC_VHT20_2G \
			      | HOST_REGDMN_MODE_11NG_HT20 \
			      | HOST_REGDMN_MODE_11NA_HT20 \
			      | HOST_REGDMN_MODE_11AC_VHT20 \
			      | HOST_REGDMN_MODE_11AXG_HE20 \
			      | HOST_REGDMN_MODE_11AXA_HE20 \
			      | HOST_REGDMN_MODE_11BEA_EHT20 \
			      | HOST_REGDMN_MODE_11BEG_EHT20)
#else
#define WIRELESS_20_MODES    (HOST_REGDMN_MODE_11A \
			      | HOST_REGDMN_MODE_TURBO \
			      | HOST_REGDMN_MODE_11B \
			      | HOST_REGDMN_MODE_PUREG \
			      | HOST_REGDMN_MODE_11G \
			      | HOST_REGDMN_MODE_11AC_VHT20_2G \
			      | HOST_REGDMN_MODE_11NG_HT20 \
			      | HOST_REGDMN_MODE_11NA_HT20 \
			      | HOST_REGDMN_MODE_11AC_VHT20 \
			      | HOST_REGDMN_MODE_11AXG_HE20 \
			      | HOST_REGDMN_MODE_11AXA_HE20)
#endif

#define WIRELESS_10_MODES   (HOST_REGDMN_MODE_11A_HALF_RATE)
#define WIRELESS_5_MODES    (HOST_REGDMN_MODE_11A_QUARTER_RATE)

#ifdef WLAN_FEATURE_11BE
#define WIRELESS_40_MODES    (HOST_REGDMN_MODE_11NG_HT40PLUS \
			      | HOST_REGDMN_MODE_11NG_HT40MINUS \
			      | HOST_REGDMN_MODE_11NA_HT40PLUS \
			      | HOST_REGDMN_MODE_11NA_HT40MINUS \
			      | HOST_REGDMN_MODE_11AC_VHT40PLUS \
			      | HOST_REGDMN_MODE_11AC_VHT40MINUS \
			      | HOST_REGDMN_MODE_11AXG_HE40PLUS \
			      | HOST_REGDMN_MODE_11AXG_HE40MINUS \
			      | HOST_REGDMN_MODE_11AXA_HE40PLUS \
			      | HOST_REGDMN_MODE_11AXA_HE40MINUS \
			      | HOST_REGDMN_MODE_11BEA_EHT40PLUS \
			      | HOST_REGDMN_MODE_11BEA_EHT40MINUS \
			      | HOST_REGDMN_MODE_11BEG_EHT40PLUS \
			      | HOST_REGDMN_MODE_11BEG_EHT40MINUS)
#else
#define WIRELESS_40_MODES    (HOST_REGDMN_MODE_11NG_HT40PLUS \
			      | HOST_REGDMN_MODE_11NG_HT40MINUS \
			      | HOST_REGDMN_MODE_11NA_HT40PLUS \
			      | HOST_REGDMN_MODE_11NA_HT40MINUS \
			      | HOST_REGDMN_MODE_11AC_VHT40PLUS \
			      | HOST_REGDMN_MODE_11AC_VHT40MINUS \
			      | HOST_REGDMN_MODE_11AXG_HE40PLUS \
			      | HOST_REGDMN_MODE_11AXG_HE40MINUS \
			      | HOST_REGDMN_MODE_11AXA_HE40PLUS \
			      | HOST_REGDMN_MODE_11AXA_HE40MINUS)
#endif

#ifdef WLAN_FEATURE_11BE
#define WIRELESS_80_MODES    (HOST_REGDMN_MODE_11AC_VHT80 \
			      | HOST_REGDMN_MODE_11AXA_HE80 \
			      | HOST_REGDMN_MODE_11BEA_EHT80)
#else
#define WIRELESS_80_MODES    (HOST_REGDMN_MODE_11AC_VHT80 \
			      | HOST_REGDMN_MODE_11AXA_HE80)
#endif

#ifdef WLAN_FEATURE_11BE
#define WIRELESS_160_MODES   (HOST_REGDMN_MODE_11AC_VHT160 \
			      | HOST_REGDMN_MODE_11AXA_HE160 \
			      | HOST_REGDMN_MODE_11BEA_EHT160)
#else
#define WIRELESS_160_MODES   (HOST_REGDMN_MODE_11AC_VHT160 \
			      | HOST_REGDMN_MODE_11AXA_HE160)
#endif

#define WIRELESS_80P80_MODES (HOST_REGDMN_MODE_11AC_VHT80_80 \
			      | HOST_REGDMN_MODE_11AXA_HE80_80)

#ifdef WLAN_FEATURE_11BE
#define WIRELESS_320_MODES (HOST_REGDMN_MODE_11BEA_EHT320)
#endif

#ifdef WLAN_FEATURE_11BE
#define WIRELESS_6G_MODES_11BE (HOST_REGDMN_MODE_11BEA_EHT20 \
				| HOST_REGDMN_MODE_11BEA_EHT40PLUS \
				| HOST_REGDMN_MODE_11BEA_EHT40MINUS \
				| HOST_REGDMN_MODE_11BEA_EHT80 \
				| HOST_REGDMN_MODE_11BEA_EHT160 \
				| HOST_REGDMN_MODE_11BEA_EHT320)
#else
#define WIRELESS_6G_MODES_11BE 0
#endif /* WLAN_FEATURE_11BE*/

#ifdef WLAN_FEATURE_11BE
#define WIRELESS_6G_MODES (HOST_REGDMN_MODE_11AXA_HE20 \
			   | HOST_REGDMN_MODE_11AXA_HE40PLUS \
			   | HOST_REGDMN_MODE_11AXA_HE40MINUS \
			   | HOST_REGDMN_MODE_11AXA_HE80 \
			   | HOST_REGDMN_MODE_11AXA_HE160 \
			   | HOST_REGDMN_MODE_11AXA_HE80_80 \
			   | HOST_REGDMN_MODE_11BEA_EHT20 \
			   | HOST_REGDMN_MODE_11BEA_EHT40PLUS \
			   | HOST_REGDMN_MODE_11BEA_EHT40MINUS \
			   | HOST_REGDMN_MODE_11BEA_EHT80 \
			   | HOST_REGDMN_MODE_11BEA_EHT160 \
			   | HOST_REGDMN_MODE_11BEA_EHT320)
#else
#define WIRELESS_6G_MODES (HOST_REGDMN_MODE_11AXA_HE20 \
			   | HOST_REGDMN_MODE_11AXA_HE40PLUS \
			   | HOST_REGDMN_MODE_11AXA_HE40MINUS \
			   | HOST_REGDMN_MODE_11AXA_HE80 \
			   | HOST_REGDMN_MODE_11AXA_HE160 \
			   | HOST_REGDMN_MODE_11AXA_HE80_80)
#endif

#ifdef WLAN_FEATURE_11BE
#define WIRELESS_5G_MODES_11BE (HOST_REGDMN_MODE_11BEA_EHT20 \
				| HOST_REGDMN_MODE_11BEA_EHT40PLUS \
				| HOST_REGDMN_MODE_11BEA_EHT40MINUS \
				| HOST_REGDMN_MODE_11BEA_EHT80 \
				| HOST_REGDMN_MODE_11BEA_EHT160 \
				| HOST_REGDMN_MODE_11BEA_EHT320)
#else
#define WIRELESS_5G_MODES_11BE 0
#endif /* WLAN_FEATURE_11BE*/

#ifdef WLAN_FEATURE_11BE
#define WIRELESS_5G_MODES (HOST_REGDMN_MODE_11BEA_EHT20 \
			   | HOST_REGDMN_MODE_11BEA_EHT40PLUS \
			   | HOST_REGDMN_MODE_11BEA_EHT40MINUS \
			   | HOST_REGDMN_MODE_11BEA_EHT80 \
			   | HOST_REGDMN_MODE_11BEA_EHT160 \
			   | HOST_REGDMN_MODE_11BEA_EHT320 \
			   | HOST_REGDMN_MODE_11AXA_HE20 \
			   | HOST_REGDMN_MODE_11AXA_HE40PLUS \
			   | HOST_REGDMN_MODE_11AXA_HE40MINUS \
			   | HOST_REGDMN_MODE_11AXA_HE80 \
			   | HOST_REGDMN_MODE_11AXA_HE160 \
			   | HOST_REGDMN_MODE_11AXA_HE80_80 \
			   | HOST_REGDMN_MODE_11AC_VHT20 \
			   | HOST_REGDMN_MODE_11AC_VHT40PLUS \
			   | HOST_REGDMN_MODE_11AC_VHT40MINUS \
			   | HOST_REGDMN_MODE_11AC_VHT80 \
			   | HOST_REGDMN_MODE_11AC_VHT160 \
			   | HOST_REGDMN_MODE_11AC_VHT80_80 \
			   | HOST_REGDMN_MODE_11NA_HT20 \
			   | HOST_REGDMN_MODE_11NA_HT40PLUS \
			   | HOST_REGDMN_MODE_11NA_HT40MINUS \
			   | HOST_REGDMN_MODE_11A \
			   | HOST_REGDMN_MODE_TURBO \
			   | HOST_REGDMN_MODE_108A \
			   | HOST_REGDMN_MODE_11A_HALF_RATE \
			   | HOST_REGDMN_MODE_11A_QUARTER_RATE)
#else
#define WIRELESS_5G_MODES (HOST_REGDMN_MODE_11AXA_HE20 \
			   | HOST_REGDMN_MODE_11AXA_HE40PLUS \
			   | HOST_REGDMN_MODE_11AXA_HE40MINUS \
			   | HOST_REGDMN_MODE_11AXA_HE80 \
			   | HOST_REGDMN_MODE_11AXA_HE160 \
			   | HOST_REGDMN_MODE_11AXA_HE80_80 \
			   | HOST_REGDMN_MODE_11AC_VHT20 \
			   | HOST_REGDMN_MODE_11AC_VHT40PLUS \
			   | HOST_REGDMN_MODE_11AC_VHT40MINUS \
			   | HOST_REGDMN_MODE_11AC_VHT80 \
			   | HOST_REGDMN_MODE_11AC_VHT160 \
			   | HOST_REGDMN_MODE_11AC_VHT80_80 \
			   | HOST_REGDMN_MODE_11NA_HT20 \
			   | HOST_REGDMN_MODE_11NA_HT40PLUS \
			   | HOST_REGDMN_MODE_11NA_HT40MINUS \
			   | HOST_REGDMN_MODE_11A \
			   | HOST_REGDMN_MODE_TURBO \
			   | HOST_REGDMN_MODE_108A \
			   | HOST_REGDMN_MODE_11A_HALF_RATE \
			   | HOST_REGDMN_MODE_11A_QUARTER_RATE)
#endif

#define WIRELESS_49G_MODES (HOST_REGDMN_MODE_11A \
			    | HOST_REGDMN_MODE_11A_HALF_RATE \
			    | HOST_REGDMN_MODE_11A_QUARTER_RATE)
#ifdef WLAN_FEATURE_11BE
#define WIRELESS_2G_MODES_11BE (HOST_REGDMN_MODE_11BEG_EHT20 \
				| HOST_REGDMN_MODE_11BEG_EHT40PLUS \
				| HOST_REGDMN_MODE_11BEG_EHT40MINUS)
#else
#define WIRELESS_2G_MODES_11BE 0
#endif /* WLAN_FEATURE_11BE*/

#ifdef WLAN_FEATURE_11BE
#define WIRELESS_2G_MODES (HOST_REGDMN_MODE_11BEG_EHT20 \
			   | HOST_REGDMN_MODE_11BEG_EHT40PLUS \
			   | HOST_REGDMN_MODE_11BEG_EHT40MINUS \
			   | HOST_REGDMN_MODE_11AXG_HE20 \
			   | HOST_REGDMN_MODE_11AXG_HE40PLUS \
			   | HOST_REGDMN_MODE_11AXG_HE40MINUS \
			   | HOST_REGDMN_MODE_11NG_HT20 \
			   | HOST_REGDMN_MODE_11NG_HT40PLUS \
			   | HOST_REGDMN_MODE_11NG_HT40MINUS \
			   | HOST_REGDMN_MODE_PUREG \
			   | HOST_REGDMN_MODE_11G \
			   | HOST_REGDMN_MODE_108G \
			   | HOST_REGDMN_MODE_11B)
#else
#define WIRELESS_2G_MODES (HOST_REGDMN_MODE_11AXG_HE20 \
			   | HOST_REGDMN_MODE_11AXG_HE40PLUS \
			   | HOST_REGDMN_MODE_11AXG_HE40MINUS \
			   | HOST_REGDMN_MODE_11NG_HT20 \
			   | HOST_REGDMN_MODE_11NG_HT40PLUS \
			   | HOST_REGDMN_MODE_11NG_HT40MINUS \
			   | HOST_REGDMN_MODE_PUREG \
			   | HOST_REGDMN_MODE_11G \
			   | HOST_REGDMN_MODE_108G \
			   | HOST_REGDMN_MODE_11B)
#endif

#ifdef CONFIG_AFC_SUPPORT
/* enum reg_afc_cmd_type - Type of AFC command sent to FW
 * @REG_AFC_CMD_SERV_RESP_READY : Server response is ready
 */
enum reg_afc_cmd_type {
	REG_AFC_CMD_SERV_RESP_READY = 1,
};

/* enum reg_afc_serv_resp_format - Indicate the format in which afc_serv_format
 * is written in FW memory
 * @REG_AFC_SERV_RESP_FORMAT_JSON - Server response in JSON format
 * @REG_AFC_SERV_RESP_FORMAT_BINARY - Server response in BINARY format
 */
enum reg_afc_serv_resp_format {
	REG_AFC_SERV_RESP_FORMAT_JSON = 0,
	REG_AFC_SERV_RESP_FORMAT_BINARY = 1,
};

/**
 * struct reg_afc_resp_rx_ind_info - regulatory AFC indication info
 * @cmd_type: Type of AFC command send to FW
 * @serv_resp_format: AFC server response format
 */
struct reg_afc_resp_rx_ind_info {
	enum reg_afc_cmd_type cmd_type;
	enum reg_afc_serv_resp_format serv_resp_format;
};

/**
 * afc_req_rx_evt_handler() - Function prototype of AFC request received event
 * handler
 * @pdev: Pointer to pdev
 * @afc_par_req: Pointer to AFC partial request
 * @arg: Pointer to void (opaque) argument object
 *
 * Return: void
 */
typedef void (*afc_req_rx_evt_handler)(struct wlan_objmgr_pdev *pdev,
				       struct wlan_afc_host_partial_request *afc_par_req,
				       void *arg);
#endif
#endif
