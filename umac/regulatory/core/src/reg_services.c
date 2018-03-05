/*
 * Copyright (c) 2014-2018 The Linux Foundation. All rights reserved.
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

/**
 * DOC: reg_services.c
 * This file defines regulatory component service functions
 */

#include "reg_services.h"
#include "reg_priv.h"
#include "reg_db_parser.h"
#include <scheduler_api.h>

#define CHAN_12_CENT_FREQ 2467
#define MAX_PWR_FCC_CHAN_12 8
#define CHAN_13_CENT_FREQ 2472
#define MAX_PWR_FCC_CHAN_13 2
#define CHAN_144_CENT_FREQ 5720
#define DEFAULT_WORLD_REGDMN 0x60

#define IS_VALID_PSOC_REG_OBJ(psoc_priv_obj) (NULL != psoc_priv_obj)
#define IS_VALID_PDEV_REG_OBJ(pdev_priv_obj) (NULL != pdev_priv_obj)

const struct chan_map *channel_map;

const struct bonded_channel bonded_chan_40mhz_list[] = {
	{36, 40},
	{44, 48},
	{52, 56},
	{60, 64},
	{100, 104},
	{108, 112},
	{116, 120},
	{124, 128},
	{132, 136},
	{140, 144},
	{149, 153},
	{157, 161}
};

const struct bonded_channel bonded_chan_80mhz_list[] = {
	{36, 48},
	{52, 64},
	{100, 112},
	{116, 128},
	{132, 144},
	{149, 161}
};

const struct bonded_channel bonded_chan_160mhz_list[] = {
	{36, 64},
	{100, 128}
};

const enum phy_ch_width get_next_lower_bw[] = {
	[CH_WIDTH_80P80MHZ] = CH_WIDTH_160MHZ,
	[CH_WIDTH_160MHZ] = CH_WIDTH_80MHZ,
	[CH_WIDTH_80MHZ] = CH_WIDTH_40MHZ,
	[CH_WIDTH_40MHZ] = CH_WIDTH_20MHZ,
	[CH_WIDTH_20MHZ] = CH_WIDTH_10MHZ,
	[CH_WIDTH_10MHZ] = CH_WIDTH_5MHZ,
	[CH_WIDTH_5MHZ] = CH_WIDTH_INVALID
};

#ifdef CONFIG_LEGACY_CHAN_ENUM
static const struct chan_map channel_map_old[NUM_CHANNELS] = {
	[CHAN_ENUM_1] = {2412, 1, 2, 40},
	[CHAN_ENUM_2] = {2417, 2, 2, 40},
	[CHAN_ENUM_3] = {2422, 3, 2, 40},
	[CHAN_ENUM_4] = {2427, 4, 2, 40},
	[CHAN_ENUM_5] = {2432, 5, 2, 40},
	[CHAN_ENUM_6] = {2437, 6, 2, 40},
	[CHAN_ENUM_7] = {2442, 7, 2, 40},
	[CHAN_ENUM_8] = {2447, 8, 2, 40},
	[CHAN_ENUM_9] = {2452, 9, 2, 40},
	[CHAN_ENUM_10] = {2457, 10, 2, 40},
	[CHAN_ENUM_11] = {2462, 11, 2, 40},
	[CHAN_ENUM_12] = {2467, 12, 2, 40},
	[CHAN_ENUM_13] = {2472, 13, 2, 40},
	[CHAN_ENUM_14] = {2484, 14, 2, 40},

	[CHAN_ENUM_36] = {5180, 36, 2, 160},
	[CHAN_ENUM_40] = {5200, 40, 2, 160},
	[CHAN_ENUM_44] = {5220, 44, 2, 160},
	[CHAN_ENUM_48] = {5240, 48, 2, 160},
	[CHAN_ENUM_52] = {5260, 52, 2, 160},
	[CHAN_ENUM_56] = {5280, 56, 2, 160},
	[CHAN_ENUM_60] = {5300, 60, 2, 160},
	[CHAN_ENUM_64] = {5320, 64, 2, 160},

	[CHAN_ENUM_100] = {5500, 100, 2, 160},
	[CHAN_ENUM_104] = {5520, 104, 2, 160},
	[CHAN_ENUM_108] = {5540, 108, 2, 160},
	[CHAN_ENUM_112] = {5560, 112, 2, 160},
	[CHAN_ENUM_116] = {5580, 116, 2, 160},
	[CHAN_ENUM_120] = {5600, 120, 2, 160},
	[CHAN_ENUM_124] = {5620, 124, 2, 160},
	[CHAN_ENUM_128] = {5640, 128, 2, 160},
	[CHAN_ENUM_132] = {5660, 132, 2, 160},
	[CHAN_ENUM_136] = {5680, 136, 2, 160},
	[CHAN_ENUM_140] = {5700, 140, 2, 160},
	[CHAN_ENUM_144] = {5720, 144, 2, 160},

	[CHAN_ENUM_149] = {5745, 149, 2, 160},
	[CHAN_ENUM_153] = {5765, 153, 2, 160},
	[CHAN_ENUM_157] = {5785, 157, 2, 160},
	[CHAN_ENUM_161] = {5805, 161, 2, 160},
	[CHAN_ENUM_165] = {5825, 165, 2, 160},
	[CHAN_ENUM_169] = {5845, 169, 2, 20},

	[CHAN_ENUM_170] = {5852, 170, 2, 20},
	[CHAN_ENUM_171] = {5855, 171, 2, 20},
	[CHAN_ENUM_172] = {5860, 172, 2, 20},
	[CHAN_ENUM_173] = {5865, 173, 2, 20},
	[CHAN_ENUM_174] = {5870, 174, 2, 20},
	[CHAN_ENUM_175] = {5875, 175, 2, 20},
	[CHAN_ENUM_176] = {5880, 176, 2, 20},
	[CHAN_ENUM_177] = {5885, 177, 2, 20},
	[CHAN_ENUM_178] = {5890, 178, 2, 20},
	[CHAN_ENUM_179] = {5895, 179, 2, 20},
	[CHAN_ENUM_180] = {5900, 180, 2, 20},
	[CHAN_ENUM_181] = {5905, 181, 2, 20},
	[CHAN_ENUM_182] = {5910, 182, 2, 20},
	[CHAN_ENUM_183] = {5915, 183, 2, 20},
	[CHAN_ENUM_184] = {5920, 184, 2, 20},
};

#else
static const struct chan_map channel_map_us[NUM_CHANNELS] = {
	[CHAN_ENUM_2412] = {2412, 1, 20, 40},
	[CHAN_ENUM_2417] = {2417, 2, 20, 40},
	[CHAN_ENUM_2422] = {2422, 3, 20, 40},
	[CHAN_ENUM_2427] = {2427, 4, 20, 40},
	[CHAN_ENUM_2432] = {2432, 5, 20, 40},
	[CHAN_ENUM_2437] = {2437, 6, 20, 40},
	[CHAN_ENUM_2442] = {2442, 7, 20, 40},
	[CHAN_ENUM_2447] = {2447, 8, 20, 40},
	[CHAN_ENUM_2452] = {2452, 9, 20, 40},
	[CHAN_ENUM_2457] = {2457, 10, 20, 40},
	[CHAN_ENUM_2462] = {2462, 11, 20, 40},
	[CHAN_ENUM_2467] = {2467, 12, 20, 40},
	[CHAN_ENUM_2472] = {2472, 13, 20, 40},
	[CHAN_ENUM_2484] = {2484, 14, 20, 20},

	[CHAN_ENUM_4912] = {4912, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4915] = {4915, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4917] = {4917, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4920] = {4920, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4922] = {4922, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4925] = {4925, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4927] = {4927, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4932] = {4932, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4935] = {4935, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4937] = {4937, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4940] = {4940, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4942] = {4942, 1, 5, 5},
	[CHAN_ENUM_4945] = {4945, 11, 10, 10},
	[CHAN_ENUM_4947] = {4947, 2, 5, 5},
	[CHAN_ENUM_4950] = {4950, 20, 10, 20},
	[CHAN_ENUM_4952] = {4952, 3, 5, 5},
	[CHAN_ENUM_4955] = {4955, 21, 10, 20},
	[CHAN_ENUM_4957] = {4957, 4, 5, 5},
	[CHAN_ENUM_4960] = {4960, 22, 10, 20},
	[CHAN_ENUM_4962] = {4962, 5, 5, 5},
	[CHAN_ENUM_4965] = {4965, 23, 10, 20},
	[CHAN_ENUM_4967] = {4967, 6, 5, 5},
	[CHAN_ENUM_4970] = {4970, 24, 10, 20},
	[CHAN_ENUM_4972] = {4972, 7, 5, 5},
	[CHAN_ENUM_4975] = {4975, 25, 10, 20},
	[CHAN_ENUM_4977] = {4977, 8, 5, 5},
	[CHAN_ENUM_4980] = {4980, 26, 10, 20},
	[CHAN_ENUM_4982] = {4982, 9, 5, 5},
	[CHAN_ENUM_4985] = {4985, 19, 10, 10},
	[CHAN_ENUM_4987] = {4987, 10, 5, 5},
	[CHAN_ENUM_5032] = {5032, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5035] = {5035, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5037] = {5037, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5040] = {5040, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5042] = {5042, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5045] = {5045, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5047] = {5047, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5052] = {5052, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5055] = {5055, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5057] = {5057, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5060] = {5060, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5080] = {5080, INVALID_CHANNEL_NUM, 2, 20},

	[CHAN_ENUM_5180] = {5180, 36, 2, 160},
	[CHAN_ENUM_5200] = {5200, 40, 2, 160},
	[CHAN_ENUM_5220] = {5220, 44, 2, 160},
	[CHAN_ENUM_5240] = {5240, 48, 2, 160},
	[CHAN_ENUM_5260] = {5260, 52, 2, 160},
	[CHAN_ENUM_5280] = {5280, 56, 2, 160},
	[CHAN_ENUM_5300] = {5300, 60, 2, 160},
	[CHAN_ENUM_5320] = {5320, 64, 2, 160},
	[CHAN_ENUM_5500] = {5500, 100, 2, 160},
	[CHAN_ENUM_5520] = {5520, 104, 2, 160},
	[CHAN_ENUM_5540] = {5540, 108, 2, 160},
	[CHAN_ENUM_5560] = {5560, 112, 2, 160},
	[CHAN_ENUM_5580] = {5580, 116, 2, 160},
	[CHAN_ENUM_5600] = {5600, 120, 2, 160},
	[CHAN_ENUM_5620] = {5620, 124, 2, 160},
	[CHAN_ENUM_5640] = {5640, 128, 2, 160},
	[CHAN_ENUM_5660] = {5660, 132, 2, 160},
	[CHAN_ENUM_5680] = {5680, 136, 2, 160},
	[CHAN_ENUM_5700] = {5700, 140, 2, 160},
	[CHAN_ENUM_5720] = {5720, 144, 2, 160},
	[CHAN_ENUM_5745] = {5745, 149, 2, 160},
	[CHAN_ENUM_5765] = {5765, 153, 2, 160},
	[CHAN_ENUM_5785] = {5785, 157, 2, 160},
	[CHAN_ENUM_5805] = {5805, 161, 2, 160},
	[CHAN_ENUM_5825] = {5825, 165, 2, 160},
	[CHAN_ENUM_5845] = {5845, 169, 2, 160},
	[CHAN_ENUM_5850] = {5850, 170, 2, 160},
	[CHAN_ENUM_5855] = {5855, 171, 2, 160},
	[CHAN_ENUM_5860] = {5860, 172, 2, 160},
	[CHAN_ENUM_5865] = {5865, 173, 2, 160},
	[CHAN_ENUM_5870] = {5870, 174, 2, 160},
	[CHAN_ENUM_5875] = {5875, 175, 2, 160},
	[CHAN_ENUM_5880] = {5880, 176, 2, 160},
	[CHAN_ENUM_5885] = {5885, 177, 2, 160},
	[CHAN_ENUM_5890] = {5890, 178, 2, 160},
	[CHAN_ENUM_5895] = {5895, 179, 2, 160},
	[CHAN_ENUM_5900] = {5900, 180, 2, 160},
	[CHAN_ENUM_5905] = {5905, 181, 2, 160},
	[CHAN_ENUM_5910] = {5910, 182, 2, 160},
	[CHAN_ENUM_5915] = {5915, 183, 2, 160},
	[CHAN_ENUM_5920] = {5920, 184, 2, 160},
};

static const struct chan_map channel_map_eu[NUM_CHANNELS] = {
	[CHAN_ENUM_2412] = {2412, 1, 20, 40},
	[CHAN_ENUM_2417] = {2417, 2, 20, 40},
	[CHAN_ENUM_2422] = {2422, 3, 20, 40},
	[CHAN_ENUM_2427] = {2427, 4, 20, 40},
	[CHAN_ENUM_2432] = {2432, 5, 20, 40},
	[CHAN_ENUM_2437] = {2437, 6, 20, 40},
	[CHAN_ENUM_2442] = {2442, 7, 20, 40},
	[CHAN_ENUM_2447] = {2447, 8, 20, 40},
	[CHAN_ENUM_2452] = {2452, 9, 20, 40},
	[CHAN_ENUM_2457] = {2457, 10, 20, 40},
	[CHAN_ENUM_2462] = {2462, 11, 20, 40},
	[CHAN_ENUM_2467] = {2467, 12, 20, 40},
	[CHAN_ENUM_2472] = {2472, 13, 20, 40},
	[CHAN_ENUM_2484] = {2484, 14, 20, 20},

	[CHAN_ENUM_4912] = {4912, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4915] = {4915, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4917] = {4917, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4920] = {4920, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4922] = {4922, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4925] = {4925, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4927] = {4927, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4932] = {4932, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4935] = {4935, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4937] = {4937, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4940] = {4940, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4942] = {4942, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4945] = {4945, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4947] = {4947, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4950] = {4950, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4952] = {4952, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4955] = {4955, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4957] = {4957, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4960] = {4960, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4962] = {4962, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4965] = {4965, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4967] = {4967, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4970] = {4970, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4972] = {4972, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4975] = {4975, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4977] = {4977, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4980] = {4980, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4982] = {4982, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4985] = {4985, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4987] = {4987, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5032] = {5032, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5035] = {5035, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5037] = {5037, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5040] = {5040, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5042] = {5042, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5045] = {5045, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5047] = {5047, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5052] = {5052, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5055] = {5055, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5057] = {5057, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5060] = {5060, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5080] = {5080, INVALID_CHANNEL_NUM, 2, 20},

	[CHAN_ENUM_5180] = {5180, 36, 2, 160},
	[CHAN_ENUM_5200] = {5200, 40, 2, 160},
	[CHAN_ENUM_5220] = {5220, 44, 2, 160},
	[CHAN_ENUM_5240] = {5240, 48, 2, 160},
	[CHAN_ENUM_5260] = {5260, 52, 2, 160},
	[CHAN_ENUM_5280] = {5280, 56, 2, 160},
	[CHAN_ENUM_5300] = {5300, 60, 2, 160},
	[CHAN_ENUM_5320] = {5320, 64, 2, 160},
	[CHAN_ENUM_5500] = {5500, 100, 2, 160},
	[CHAN_ENUM_5520] = {5520, 104, 2, 160},
	[CHAN_ENUM_5540] = {5540, 108, 2, 160},
	[CHAN_ENUM_5560] = {5560, 112, 2, 160},
	[CHAN_ENUM_5580] = {5580, 116, 2, 160},
	[CHAN_ENUM_5600] = {5600, 120, 2, 160},
	[CHAN_ENUM_5620] = {5620, 124, 2, 160},
	[CHAN_ENUM_5640] = {5640, 128, 2, 160},
	[CHAN_ENUM_5660] = {5660, 132, 2, 160},
	[CHAN_ENUM_5680] = {5680, 136, 2, 160},
	[CHAN_ENUM_5700] = {5700, 140, 2, 160},
	[CHAN_ENUM_5720] = {5720, 144, 2, 160},
	[CHAN_ENUM_5745] = {5745, 149, 2, 160},
	[CHAN_ENUM_5765] = {5765, 153, 2, 160},
	[CHAN_ENUM_5785] = {5785, 157, 2, 160},
	[CHAN_ENUM_5805] = {5805, 161, 2, 160},
	[CHAN_ENUM_5825] = {5825, 165, 2, 160},
	[CHAN_ENUM_5845] = {5845, 169, 2, 160},
	[CHAN_ENUM_5850] = {5850, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5855] = {5855, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5860] = {5860, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5865] = {5865, 173, 2, 160},
	[CHAN_ENUM_5870] = {5870, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5875] = {5875, 175, 2, 160},
	[CHAN_ENUM_5880] = {5880, 176, 2, 160},
	[CHAN_ENUM_5885] = {5885, 177, 2, 160},
	[CHAN_ENUM_5890] = {5890, 178, 2, 160},
	[CHAN_ENUM_5895] = {5895, 179, 2, 160},
	[CHAN_ENUM_5900] = {5900, 180, 2, 160},
	[CHAN_ENUM_5905] = {5905, 181, 2, 160},
	[CHAN_ENUM_5910] = {5910, 182, 2, 160},
	[CHAN_ENUM_5915] = {5915, 183, 2, 160},
	[CHAN_ENUM_5920] = {5920, 184, 2, 160},
};

static const struct chan_map channel_map_jp[NUM_CHANNELS] = {
	[CHAN_ENUM_2412] = {2412, 1, 20, 40},
	[CHAN_ENUM_2417] = {2417, 2, 20, 40},
	[CHAN_ENUM_2422] = {2422, 3, 20, 40},
	[CHAN_ENUM_2427] = {2427, 4, 20, 40},
	[CHAN_ENUM_2432] = {2432, 5, 20, 40},
	[CHAN_ENUM_2437] = {2437, 6, 20, 40},
	[CHAN_ENUM_2442] = {2442, 7, 20, 40},
	[CHAN_ENUM_2447] = {2447, 8, 20, 40},
	[CHAN_ENUM_2452] = {2452, 9, 20, 40},
	[CHAN_ENUM_2457] = {2457, 10, 20, 40},
	[CHAN_ENUM_2462] = {2462, 11, 20, 40},
	[CHAN_ENUM_2467] = {2467, 12, 20, 40},
	[CHAN_ENUM_2472] = {2472, 13, 20, 40},
	[CHAN_ENUM_2484] = {2484, 14, 20, 20},

	[CHAN_ENUM_4912] = {4912, 182, 5, 5},
	[CHAN_ENUM_4915] = {4915, 183, 10, 10},
	[CHAN_ENUM_4917] = {4917, 183, 5, 5},
	[CHAN_ENUM_4920] = {4920, 184, 10, 20},
	[CHAN_ENUM_4922] = {4922, 184, 5, 5},
	[CHAN_ENUM_4925] = {4925, 185, 10, 10},
	[CHAN_ENUM_4927] = {4927, 185, 5, 5},
	[CHAN_ENUM_4932] = {4932, 186, 5, 5},
	[CHAN_ENUM_4935] = {4935, 187, 10, 10},
	[CHAN_ENUM_4937] = {4937, 187, 5, 5},
	[CHAN_ENUM_4940] = {4940, 188, 10, 20},
	[CHAN_ENUM_4942] = {4942, 188, 5, 5},
	[CHAN_ENUM_4945] = {4945, 189, 10, 10},
	[CHAN_ENUM_4947] = {4947, 189, 5, 5},
	[CHAN_ENUM_4950] = {4950, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4952] = {4952, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4955] = {4955, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4957] = {4957, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4960] = {4960, 192, 20, 20},
	[CHAN_ENUM_4962] = {4962, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4965] = {4965, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4967] = {4967, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4970] = {4970, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4972] = {4972, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4975] = {4975, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4977] = {4977, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4980] = {4980, 196, 20, 20},
	[CHAN_ENUM_4982] = {4982, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4985] = {4985, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4987] = {4987, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5032] = {5032, 6, 5, 5},
	[CHAN_ENUM_5035] = {5035, 7, 10, 10},
	[CHAN_ENUM_5037] = {5037, 7, 5, 5},
	[CHAN_ENUM_5040] = {5040, 8, 10, 20},
	[CHAN_ENUM_5042] = {5042, 8, 5, 5},
	[CHAN_ENUM_5045] = {5045, 9, 10, 10},
	[CHAN_ENUM_5047] = {5047, 9, 5, 5},
	[CHAN_ENUM_5052] = {5052, 10, 5, 5},
	[CHAN_ENUM_5055] = {5055, 11, 10, 10},
	[CHAN_ENUM_5057] = {5057, 11, 5, 5},
	[CHAN_ENUM_5060] = {5060, 12, 20, 20},
	[CHAN_ENUM_5080] = {5080, 16, 20, 20},

	[CHAN_ENUM_5180] = {5180, 36, 2, 160},
	[CHAN_ENUM_5200] = {5200, 40, 2, 160},
	[CHAN_ENUM_5220] = {5220, 44, 2, 160},
	[CHAN_ENUM_5240] = {5240, 48, 2, 160},
	[CHAN_ENUM_5260] = {5260, 52, 2, 160},
	[CHAN_ENUM_5280] = {5280, 56, 2, 160},
	[CHAN_ENUM_5300] = {5300, 60, 2, 160},
	[CHAN_ENUM_5320] = {5320, 64, 2, 160},
	[CHAN_ENUM_5500] = {5500, 100, 2, 160},
	[CHAN_ENUM_5520] = {5520, 104, 2, 160},
	[CHAN_ENUM_5540] = {5540, 108, 2, 160},
	[CHAN_ENUM_5560] = {5560, 112, 2, 160},
	[CHAN_ENUM_5580] = {5580, 116, 2, 160},
	[CHAN_ENUM_5600] = {5600, 120, 2, 160},
	[CHAN_ENUM_5620] = {5620, 124, 2, 160},
	[CHAN_ENUM_5640] = {5640, 128, 2, 160},
	[CHAN_ENUM_5660] = {5660, 132, 2, 160},
	[CHAN_ENUM_5680] = {5680, 136, 2, 160},
	[CHAN_ENUM_5700] = {5700, 140, 2, 160},
	[CHAN_ENUM_5720] = {5720, 144, 2, 160},
	[CHAN_ENUM_5745] = {5745, 149, 2, 160},
	[CHAN_ENUM_5765] = {5765, 153, 2, 160},
	[CHAN_ENUM_5785] = {5785, 157, 2, 160},
	[CHAN_ENUM_5805] = {5805, 161, 2, 160},
	[CHAN_ENUM_5825] = {5825, 165, 2, 160},
	[CHAN_ENUM_5845] = {5845, 169, 2, 160},
	[CHAN_ENUM_5850] = {5850, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5855] = {5855, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5860] = {5860, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5865] = {5865, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5870] = {5870, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5875] = {5875, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5880] = {5880, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5885] = {5885, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5890] = {5890, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5895] = {5895, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5900] = {5900, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5905] = {5905, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5910] = {5910, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5915] = {5915, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5920] = {5920, INVALID_CHANNEL_NUM, 2, 160},
};

static const struct chan_map channel_map_global[NUM_CHANNELS] = {
	[CHAN_ENUM_2412] = {2412, 1, 20, 40},
	[CHAN_ENUM_2417] = {2417, 2, 20, 40},
	[CHAN_ENUM_2422] = {2422, 3, 20, 40},
	[CHAN_ENUM_2427] = {2427, 4, 20, 40},
	[CHAN_ENUM_2432] = {2432, 5, 20, 40},
	[CHAN_ENUM_2437] = {2437, 6, 20, 40},
	[CHAN_ENUM_2442] = {2442, 7, 20, 40},
	[CHAN_ENUM_2447] = {2447, 8, 20, 40},
	[CHAN_ENUM_2452] = {2452, 9, 20, 40},
	[CHAN_ENUM_2457] = {2457, 10, 20, 40},
	[CHAN_ENUM_2462] = {2462, 11, 20, 40},
	[CHAN_ENUM_2467] = {2467, 12, 20, 40},
	[CHAN_ENUM_2472] = {2472, 13, 20, 40},
	[CHAN_ENUM_2484] = {2484, 14, 20, 20},

	[CHAN_ENUM_4912] = {4912, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4915] = {4915, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4917] = {4917, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4920] = {4920, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4922] = {4922, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4925] = {4925, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4927] = {4927, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4932] = {4932, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4935] = {4935, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4937] = {4937, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4940] = {4940, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4942] = {4942, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4945] = {4945, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4947] = {4947, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4950] = {4950, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4952] = {4952, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4955] = {4955, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4957] = {4957, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4960] = {4960, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4962] = {4962, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4965] = {4965, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4967] = {4967, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4970] = {4970, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4972] = {4972, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4975] = {4975, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4977] = {4977, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4980] = {4980, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4982] = {4982, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4985] = {4985, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4987] = {4987, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5032] = {5032, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5035] = {5035, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5037] = {5037, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5040] = {5040, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5042] = {5042, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5045] = {5045, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5047] = {5047, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5052] = {5052, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5055] = {5055, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5057] = {5057, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5060] = {5060, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5080] = {5080, INVALID_CHANNEL_NUM, 2, 20},

	[CHAN_ENUM_5180] = {5180, 36, 2, 160},
	[CHAN_ENUM_5200] = {5200, 40, 2, 160},
	[CHAN_ENUM_5220] = {5220, 44, 2, 160},
	[CHAN_ENUM_5240] = {5240, 48, 2, 160},
	[CHAN_ENUM_5260] = {5260, 52, 2, 160},
	[CHAN_ENUM_5280] = {5280, 56, 2, 160},
	[CHAN_ENUM_5300] = {5300, 60, 2, 160},
	[CHAN_ENUM_5320] = {5320, 64, 2, 160},
	[CHAN_ENUM_5500] = {5500, 100, 2, 160},
	[CHAN_ENUM_5520] = {5520, 104, 2, 160},
	[CHAN_ENUM_5540] = {5540, 108, 2, 160},
	[CHAN_ENUM_5560] = {5560, 112, 2, 160},
	[CHAN_ENUM_5580] = {5580, 116, 2, 160},
	[CHAN_ENUM_5600] = {5600, 120, 2, 160},
	[CHAN_ENUM_5620] = {5620, 124, 2, 160},
	[CHAN_ENUM_5640] = {5640, 128, 2, 160},
	[CHAN_ENUM_5660] = {5660, 132, 2, 160},
	[CHAN_ENUM_5680] = {5680, 136, 2, 160},
	[CHAN_ENUM_5700] = {5700, 140, 2, 160},
	[CHAN_ENUM_5720] = {5720, 144, 2, 160},
	[CHAN_ENUM_5745] = {5745, 149, 2, 160},
	[CHAN_ENUM_5765] = {5765, 153, 2, 160},
	[CHAN_ENUM_5785] = {5785, 157, 2, 160},
	[CHAN_ENUM_5805] = {5805, 161, 2, 160},
	[CHAN_ENUM_5825] = {5825, 165, 2, 160},
	[CHAN_ENUM_5845] = {5845, 169, 2, 160},
	[CHAN_ENUM_5850] = {5850, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5855] = {5855, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5860] = {5860, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5865] = {5865, 173, 2, 160},
	[CHAN_ENUM_5870] = {5870, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5875] = {5875, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5880] = {5880, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5885] = {5885, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5890] = {5890, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5895] = {5895, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5900] = {5900, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5905] = {5905, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5910] = {5910, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5915] = {5915, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5920] = {5920, INVALID_CHANNEL_NUM, 2, 160},
};

static const struct chan_map channel_map_china[NUM_CHANNELS] = {
	[CHAN_ENUM_2412] = {2412, 1, 20, 40},
	[CHAN_ENUM_2417] = {2417, 2, 20, 40},
	[CHAN_ENUM_2422] = {2422, 3, 20, 40},
	[CHAN_ENUM_2427] = {2427, 4, 20, 40},
	[CHAN_ENUM_2432] = {2432, 5, 20, 40},
	[CHAN_ENUM_2437] = {2437, 6, 20, 40},
	[CHAN_ENUM_2442] = {2442, 7, 20, 40},
	[CHAN_ENUM_2447] = {2447, 8, 20, 40},
	[CHAN_ENUM_2452] = {2452, 9, 20, 40},
	[CHAN_ENUM_2457] = {2457, 10, 20, 40},
	[CHAN_ENUM_2462] = {2462, 11, 20, 40},
	[CHAN_ENUM_2467] = {2467, 12, 20, 40},
	[CHAN_ENUM_2472] = {2472, 13, 20, 40},
	[CHAN_ENUM_2484] = {2484, 14, 20, 20},

	[CHAN_ENUM_4912] = {4912, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4915] = {4915, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4917] = {4917, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4920] = {4920, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4922] = {4922, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4925] = {4925, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4927] = {4927, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4932] = {4932, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4935] = {4935, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4937] = {4937, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4940] = {4940, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4942] = {4942, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4945] = {4945, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4947] = {4947, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4950] = {4950, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4952] = {4952, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4955] = {4955, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4957] = {4957, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4960] = {4960, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4962] = {4962, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4965] = {4965, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4967] = {4967, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4970] = {4970, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4972] = {4972, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4975] = {4975, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4977] = {4977, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4980] = {4980, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4982] = {4982, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4985] = {4985, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_4987] = {4987, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5032] = {5032, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5035] = {5035, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5037] = {5037, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5040] = {5040, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5042] = {5042, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5045] = {5045, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5047] = {5047, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5052] = {5052, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5055] = {5055, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5057] = {5057, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5060] = {5060, INVALID_CHANNEL_NUM, 2, 20},
	[CHAN_ENUM_5080] = {5080, INVALID_CHANNEL_NUM, 2, 20},

	[CHAN_ENUM_5180] = {5180, 36, 2, 160},
	[CHAN_ENUM_5200] = {5200, 40, 2, 160},
	[CHAN_ENUM_5220] = {5220, 44, 2, 160},
	[CHAN_ENUM_5240] = {5240, 48, 2, 160},
	[CHAN_ENUM_5260] = {5260, 52, 2, 160},
	[CHAN_ENUM_5280] = {5280, 56, 2, 160},
	[CHAN_ENUM_5300] = {5300, 60, 2, 160},
	[CHAN_ENUM_5320] = {5320, 64, 2, 160},
	[CHAN_ENUM_5500] = {5500, 100, 2, 160},
	[CHAN_ENUM_5520] = {5520, 104, 2, 160},
	[CHAN_ENUM_5540] = {5540, 108, 2, 160},
	[CHAN_ENUM_5560] = {5560, 112, 2, 160},
	[CHAN_ENUM_5580] = {5580, 116, 2, 160},
	[CHAN_ENUM_5600] = {5600, 120, 2, 160},
	[CHAN_ENUM_5620] = {5620, 124, 2, 160},
	[CHAN_ENUM_5640] = {5640, 128, 2, 160},
	[CHAN_ENUM_5660] = {5660, 132, 2, 160},
	[CHAN_ENUM_5680] = {5680, 136, 2, 160},
	[CHAN_ENUM_5700] = {5700, 140, 2, 160},
	[CHAN_ENUM_5720] = {5720, 144, 2, 160},
	[CHAN_ENUM_5745] = {5745, 149, 2, 160},
	[CHAN_ENUM_5765] = {5765, 153, 2, 160},
	[CHAN_ENUM_5785] = {5785, 157, 2, 160},
	[CHAN_ENUM_5805] = {5805, 161, 2, 160},
	[CHAN_ENUM_5825] = {5825, 165, 2, 160},
	[CHAN_ENUM_5845] = {5845, 169, 2, 160},
	[CHAN_ENUM_5850] = {5850, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5855] = {5855, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5860] = {5860, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5865] = {5865, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5870] = {5870, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5875] = {5875, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5880] = {5880, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5885] = {5885, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5890] = {5890, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5895] = {5895, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5900] = {5900, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5905] = {5905, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5910] = {5910, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5915] = {5915, INVALID_CHANNEL_NUM, 2, 160},
	[CHAN_ENUM_5920] = {5920, INVALID_CHANNEL_NUM, 2, 160},
};
#endif

static struct reg_dmn_supp_op_classes reg_dmn_curr_supp_opp_classes = { 0 };

static const struct reg_dmn_op_class_map_t global_op_class[] = {
	{81, 25, BW20, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{82, 25, BW20, {14} },
	{83, 40, BW40_LOW_PRIMARY, {1, 2, 3, 4, 5, 6, 7, 8, 9} },
	{84, 40, BW40_HIGH_PRIMARY, {5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{115, 20, BW20, {36, 40, 44, 48} },
	{116, 40, BW40_LOW_PRIMARY, {36, 44} },
	{117, 40, BW40_HIGH_PRIMARY, {40, 48} },
	{118, 20, BW20, {52, 56, 60, 64} },
	{119, 40, BW40_LOW_PRIMARY, {52, 60} },
	{120, 40, BW40_HIGH_PRIMARY, {56, 64} },
	{121, 20, BW20,
	 {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140} },
	{122, 40, BW40_LOW_PRIMARY, {100, 108, 116, 124, 132} },
	{123, 40, BW40_HIGH_PRIMARY, {104, 112, 120, 128, 136} },
	{125, 20, BW20, {149, 153, 157, 161, 165, 169} },
	{126, 40, BW40_LOW_PRIMARY, {149, 157} },
	{127, 40, BW40_HIGH_PRIMARY, {153, 161} },
	{128, 80, BW80, {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108,
			   112, 116, 120, 124, 128, 132, 136, 140, 144,
			   149, 153, 157, 161} },
	{0, 0, 0, {0} },
};

static const struct reg_dmn_op_class_map_t us_op_class[] = {
	{1, 20, BW20, {36, 40, 44, 48} },
	{2, 20, BW20, {52, 56, 60, 64} },
	{4, 20, BW20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140,
		       144} },
	{5, 20, BW20, {149, 153, 157, 161, 165} },
	{12, 25, BW20, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11} },
	{22, 40, BW40_LOW_PRIMARY, {36, 44} },
	{23, 40, BW40_LOW_PRIMARY, {52, 60} },
	{24, 40, BW40_LOW_PRIMARY, {100, 108, 116, 124, 132} },
	{26, 40, BW40_LOW_PRIMARY, {149, 157} },
	{27, 40, BW40_HIGH_PRIMARY, {40, 48} },
	{28, 40, BW40_HIGH_PRIMARY, {56, 64} },
	{29, 40, BW40_HIGH_PRIMARY, {104, 112, 120, 128, 136} },
	{31, 40, BW40_HIGH_PRIMARY, {153, 161} },
	{32, 40, BW40_LOW_PRIMARY, {1, 2, 3, 4, 5, 6, 7} },
	{33, 40, BW40_HIGH_PRIMARY, {5, 6, 7, 8, 9, 10, 11} },
	{128, 80, BW80, {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108,
			 112, 116, 120, 124, 128, 132, 136, 140, 144,
			 149, 153, 157, 161} },
	{0, 0, 0, {0} },
};

static const struct reg_dmn_op_class_map_t euro_op_class[] = {
	{1, 20, BW20, {36, 40, 44, 48} },
	{2, 20, BW20, {52, 56, 60, 64} },
	{3, 20, BW20, {100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140} },
	{4, 25, BW20, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{5, 40, BW40_LOW_PRIMARY, {36, 44} },
	{6, 40, BW40_LOW_PRIMARY, {52, 60} },
	{7, 40, BW40_LOW_PRIMARY, {100, 108, 116, 124, 132} },
	{8, 40, BW40_HIGH_PRIMARY, {40, 48} },
	{9, 40, BW40_HIGH_PRIMARY, {56, 64} },
	{10, 40, BW40_HIGH_PRIMARY, {104, 112, 120, 128, 136} },
	{11, 40, BW40_LOW_PRIMARY, {1, 2, 3, 4, 5, 6, 7, 8, 9} },
	{12, 40, BW40_HIGH_PRIMARY, {5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{17, 20, BW20, {149, 153, 157, 161, 165, 169} },
	{128, 80, BW80, {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112,
			 116, 120, 124, 128} },
	{0, 0, 0, {0} },
};

static const struct reg_dmn_op_class_map_t japan_op_class[] = {
	{1, 20, BW20, {36, 40, 44, 48} },
	{30, 25, BW20, {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13} },
	{31, 25, BW20, {14} },
	{32, 20, BW20, {52, 56, 60, 64} },
	{34, 20, BW20,
		{100, 104, 108, 112, 116, 120, 124, 128, 132, 136, 140} },
	{36, 40, BW40_LOW_PRIMARY, {36, 44} },
	{37, 40, BW40_LOW_PRIMARY, {52, 60} },
	{39, 40, BW40_LOW_PRIMARY, {100, 108, 116, 124, 132} },
	{41, 40, BW40_HIGH_PRIMARY, {40, 48} },
	{42, 40, BW40_HIGH_PRIMARY, {56, 64} },
	{44, 40, BW40_HIGH_PRIMARY, {104, 112, 120, 128, 136} },
	{128, 80, BW80, {36, 40, 44, 48, 52, 56, 60, 64, 100, 104, 108, 112,
			 116, 120, 124, 128} },
	{0, 0, 0, {0} },
};

struct wlan_regulatory_psoc_priv_obj *reg_get_psoc_obj(
		struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	if (!psoc) {
		reg_alert("psoc is NULL");
		return NULL;
	}
	soc_reg = wlan_objmgr_psoc_get_comp_private_obj(psoc,
				WLAN_UMAC_COMP_REGULATORY);

	return soc_reg;
}

/**
 * reg_get_pdev_obj() - Provides the reg component object pointer
 * @psoc: pointer to psoc object.
 *
 * Return: reg component object pointer
 */
static struct wlan_regulatory_pdev_priv_obj *reg_get_pdev_obj(
		struct wlan_objmgr_pdev *pdev)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_reg;

	if (!pdev) {
		reg_alert("pdev is NULL");
		return NULL;
	}
	pdev_reg = wlan_objmgr_pdev_get_comp_private_obj(pdev,
				WLAN_UMAC_COMP_REGULATORY);

	return pdev_reg;
}
/**
 * reg_get_bw_value() - give bandwidth value
 * bw: bandwidth enum
 *
 * Return: uint16_t
 */
uint16_t reg_get_bw_value(enum phy_ch_width bw)
{
	switch (bw) {
	case CH_WIDTH_20MHZ:
		return 20;
	case CH_WIDTH_40MHZ:
		return 40;
	case CH_WIDTH_80MHZ:
		return 80;
	case CH_WIDTH_160MHZ:
		return 160;
	case CH_WIDTH_80P80MHZ:
		return 160;
	case CH_WIDTH_INVALID:
		return 0;
	case CH_WIDTH_5MHZ:
		return 5;
	case CH_WIDTH_10MHZ:
		return 10;
	case CH_WIDTH_MAX:
		return 160;
	default:
		return 0;
	}
}

/**
 * reg_get_channel_list_with_power() - Provides the channel list with power
 * @ch_list: pointer to the channel list.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_get_channel_list_with_power(struct wlan_objmgr_pdev *pdev,
					   struct channel_power *ch_list,
					   uint8_t *num_chan)
{
	int i, count;
	struct regulatory_channel *reg_channels;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	if (!num_chan || !ch_list) {
		reg_err("chan_list or num_ch is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* set the current channel list */
	reg_channels = pdev_priv_obj->cur_chan_list;

	for (i = 0, count = 0; i < NUM_CHANNELS; i++) {
		if (reg_channels[i].state &&
		    reg_channels[i].state != REGULATORY_CHAN_DISABLED) {
			ch_list[count].chan_num =
				reg_channels[i].chan_num;
			ch_list[count++].tx_power =
				reg_channels[i].tx_power;
		}
	}

	*num_chan = count;

	return QDF_STATUS_SUCCESS;
}

enum channel_enum reg_get_chan_enum(uint32_t chan_num)
{
	uint32_t count;

	for (count = 0; count < NUM_CHANNELS; count++)
		if (channel_map[count].chan_num == chan_num)
			return count;

	reg_err("invalid channel number %d", chan_num);

	return INVALID_CHANNEL;
}
/**
 * reg_get_channel_state() - Get channel state from regulatory
 * @ch: channel number.
 *
 * Return: channel state
 */
enum channel_state reg_get_channel_state(struct wlan_objmgr_pdev *pdev,
		uint32_t ch)
{
	enum channel_enum ch_idx;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	ch_idx = reg_get_chan_enum(ch);

	if (INVALID_CHANNEL == ch_idx)
		return CHANNEL_STATE_INVALID;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return pdev_priv_obj->cur_chan_list[ch_idx].state;
}

/**
 * reg_get_5g_bonded_chan_array() - get ptr to bonded channel
 * @oper_ch: operating channel number
 * @bonded_chan_ar: bonded channel array
 * @bonded_chan_ptr_ptr: bonded channel ptr ptr
 *
 * Return: bonded channel state
 */
static enum channel_state reg_get_5g_bonded_chan_array(
		struct wlan_objmgr_pdev *pdev,
		uint32_t oper_chan,
		const struct bonded_channel bonded_chan_ar[],
		uint16_t array_size,
		const struct bonded_channel **bonded_chan_ptr_ptr)
{
	int i;
	uint8_t chan_num;
	const struct bonded_channel *bonded_chan_ptr = NULL;
	enum channel_state chan_state = CHANNEL_STATE_INVALID;
	enum channel_state temp_chan_state;

	for (i = 0; i < array_size; i++) {
		if ((oper_chan >= bonded_chan_ar[i].start_ch) &&
		    (oper_chan <= bonded_chan_ar[i].end_ch)) {
			bonded_chan_ptr =  &(bonded_chan_ar[i]);
			break;
		}
	}

	if (NULL == bonded_chan_ptr)
		return chan_state;

	*bonded_chan_ptr_ptr = bonded_chan_ptr;
	chan_num =  bonded_chan_ptr->start_ch;
	while (chan_num <= bonded_chan_ptr->end_ch) {
		temp_chan_state = reg_get_channel_state(pdev, chan_num);
		if (temp_chan_state < chan_state)
			chan_state = temp_chan_state;
		chan_num = chan_num + 4;
	}

	return chan_state;
}

/**
 * reg_get_5g_bonded_channel() - get the 5G bonded channel state
 * @chan_num: channel number
 * @ch_width: channel width
 * @bonded_chan_ptr_ptr: bonded channel ptr ptr
 *
 * Return: channel state
 */
static enum channel_state reg_get_5g_bonded_channel(
		struct wlan_objmgr_pdev *pdev, uint32_t chan_num,
		enum phy_ch_width ch_width,
		const struct bonded_channel **bonded_chan_ptr_ptr)
{
	if (CH_WIDTH_80P80MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_80mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_80mhz_list),
				bonded_chan_ptr_ptr);
	else if (CH_WIDTH_160MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_160mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_160mhz_list),
				bonded_chan_ptr_ptr);
	else if (CH_WIDTH_80MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_80mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_80mhz_list),
				bonded_chan_ptr_ptr);
	else if (CH_WIDTH_40MHZ == ch_width)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_40mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_40mhz_list),
				bonded_chan_ptr_ptr);
	else
		return reg_get_channel_state(pdev, chan_num);
}
/**
 * reg_get_5g_bonded_channel_state() - Get channel state for 5G bonded channel
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state reg_get_5g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev,
		uint8_t ch, enum phy_ch_width bw)
{
	enum channel_enum ch_indx;
	enum channel_state chan_state;
	struct regulatory_channel *reg_channels;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	bool bw_enabled = false;
	const struct bonded_channel *bonded_chan_ptr = NULL;

	if (CH_WIDTH_80P80MHZ < bw) {
		reg_err("bw passed is not good");
		return CHANNEL_STATE_INVALID;
	}

	chan_state = reg_get_5g_bonded_channel(pdev, ch, bw, &bonded_chan_ptr);

	if ((CHANNEL_STATE_INVALID == chan_state) ||
			(CHANNEL_STATE_DISABLE == chan_state))
		return chan_state;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return CHANNEL_STATE_INVALID;
	}
	reg_channels = pdev_priv_obj->cur_chan_list;

	ch_indx = reg_get_chan_enum(ch);
	if (INVALID_CHANNEL == ch_indx)
		return CHANNEL_STATE_INVALID;
	if (CH_WIDTH_5MHZ == bw)
		bw_enabled = true;
	else if (CH_WIDTH_10MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 10) &&
			(reg_channels[ch_indx].max_bw >= 10);
	else if (CH_WIDTH_20MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 20) &&
			(reg_channels[ch_indx].max_bw >= 20);
	else if (CH_WIDTH_40MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 40) &&
			(reg_channels[ch_indx].max_bw >= 40);
	else if (CH_WIDTH_80MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 80) &&
			(reg_channels[ch_indx].max_bw >= 80);
	else if (CH_WIDTH_160MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 160) &&
			(reg_channels[ch_indx].max_bw >= 160);
	else if (CH_WIDTH_80P80MHZ == bw)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 80) &&
			(reg_channels[ch_indx].max_bw >= 80);

	if (bw_enabled)
		return chan_state;
	else
		return CHANNEL_STATE_DISABLE;
}

/**
 * reg_get_2g_bonded_channel_state() - Get channel state for 2G bonded channel
 * @ch: channel number.
 * @bw: channel band width
 *
 * Return: channel state
 */
enum channel_state reg_get_2g_bonded_channel_state(
		struct wlan_objmgr_pdev *pdev,
		uint8_t oper_ch, uint8_t sec_ch,
		enum phy_ch_width bw)
{
	enum channel_enum chan_idx;
	enum channel_state chan_state;
	struct regulatory_channel *reg_channels;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	bool bw_enabled = false;
	enum channel_state chan_state2 = CHANNEL_STATE_INVALID;

	if (CH_WIDTH_40MHZ < bw)
		return CHANNEL_STATE_INVALID;

	if (CH_WIDTH_40MHZ == bw) {
		if ((sec_ch + 4 != oper_ch) &&
		    (oper_ch + 4 != sec_ch))
			return CHANNEL_STATE_INVALID;
		chan_state2 = reg_get_channel_state(pdev, sec_ch);
		if (CHANNEL_STATE_INVALID == chan_state2)
			return chan_state2;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return CHANNEL_STATE_INVALID;
	}

	reg_channels = pdev_priv_obj->cur_chan_list;

	chan_state = reg_get_channel_state(pdev, oper_ch);
	if (chan_state2 < chan_state)
		chan_state = chan_state2;

	if ((CHANNEL_STATE_INVALID == chan_state) ||
	    (CHANNEL_STATE_DISABLE == chan_state))
		return chan_state;

	chan_idx = reg_get_chan_enum(oper_ch);
	if (INVALID_CHANNEL == chan_idx)
		return CHANNEL_STATE_INVALID;
	if (CH_WIDTH_5MHZ == bw)
		bw_enabled = true;
	else if (CH_WIDTH_10MHZ == bw)
		bw_enabled = (reg_channels[chan_idx].min_bw <= 10) &&
			(reg_channels[chan_idx].max_bw >= 10);
	else if (CH_WIDTH_20MHZ == bw)
		bw_enabled = (reg_channels[chan_idx].min_bw <= 20) &&
			(reg_channels[chan_idx].max_bw >= 20);
	else if (CH_WIDTH_40MHZ == bw)
		bw_enabled = (reg_channels[chan_idx].min_bw <= 40) &&
			(reg_channels[chan_idx].max_bw >= 40);

	if (bw_enabled)
		return chan_state;
	else
		return CHANNEL_STATE_DISABLE;

	return CHANNEL_STATE_ENABLE;
}

static enum channel_state reg_combine_channel_states(
	enum channel_state chan_state1,
	enum channel_state chan_state2)
{
	if ((CHANNEL_STATE_INVALID == chan_state1) ||
	    (CHANNEL_STATE_INVALID == chan_state2))
		return CHANNEL_STATE_INVALID;
	else
		return min(chan_state1, chan_state2);
}

/**
 * reg_set_5g_channel_params () - Sets channel parameteres for given bandwidth
 * @ch: channel number.
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
static void reg_set_5g_channel_params(struct wlan_objmgr_pdev *pdev,
			       uint8_t ch,
			       struct ch_params *ch_params)
{
	/*
	 * Set channel parameters like center frequency for a bonded channel
	 * state. Also return the maximum bandwidth supported by the channel.
	 */

	enum phy_ch_width next_lower_bw;
	enum channel_state chan_state = CHANNEL_STATE_ENABLE;
	enum channel_state chan_state2 = CHANNEL_STATE_ENABLE;
	const struct bonded_channel *bonded_chan_ptr = NULL;
	const struct bonded_channel *bonded_chan_ptr2 = NULL;

	if (NULL == ch_params) {
		reg_err("ch_params is NULL");
		return;
	}

	if (CH_WIDTH_MAX <= ch_params->ch_width) {
		if (0 != ch_params->center_freq_seg1)
			ch_params->ch_width = CH_WIDTH_80P80MHZ;
		else
			ch_params->ch_width = CH_WIDTH_160MHZ;
	}
	next_lower_bw = ch_params->ch_width;

	while (ch_params->ch_width != CH_WIDTH_INVALID) {
		ch_params->ch_width = next_lower_bw;
		next_lower_bw = get_next_lower_bw[ch_params->ch_width];
		bonded_chan_ptr = NULL;
		bonded_chan_ptr2 = NULL;
		chan_state = reg_get_5g_bonded_channel(pdev, ch,
				ch_params->ch_width, &bonded_chan_ptr);

		chan_state = reg_get_5g_bonded_channel_state(pdev, ch,
				ch_params->ch_width);

		if (CH_WIDTH_80P80MHZ == ch_params->ch_width) {
			chan_state2 = reg_get_5g_bonded_channel_state(pdev,
					   ch_params->center_freq_seg1 - 2,
					   CH_WIDTH_80MHZ);

			chan_state = reg_combine_channel_states(chan_state,
					chan_state2);
		}

		if ((CHANNEL_STATE_ENABLE != chan_state) &&
		    (CHANNEL_STATE_DFS != chan_state))
			continue;
		if (CH_WIDTH_20MHZ >= ch_params->ch_width) {
			ch_params->sec_ch_offset = NO_SEC_CH;
			ch_params->center_freq_seg0 = ch;
			break;
		} else if (CH_WIDTH_40MHZ <= ch_params->ch_width) {
			reg_get_5g_bonded_chan_array(pdev, ch,
					bonded_chan_40mhz_list,
					QDF_ARRAY_SIZE(bonded_chan_40mhz_list),
					&bonded_chan_ptr2);
			if (!bonded_chan_ptr || !bonded_chan_ptr2)
				continue;
			if (ch == bonded_chan_ptr2->start_ch)
				ch_params->sec_ch_offset = LOW_PRIMARY_CH;
			else
				ch_params->sec_ch_offset = HIGH_PRIMARY_CH;

			ch_params->center_freq_seg0 =
				(bonded_chan_ptr->start_ch +
				 bonded_chan_ptr->end_ch)/2;
			break;
		}
	}

	if (CH_WIDTH_160MHZ == ch_params->ch_width) {
		ch_params->center_freq_seg1 = ch_params->center_freq_seg0;
		chan_state = reg_get_5g_bonded_channel(pdev, ch,
				    CH_WIDTH_80MHZ, &bonded_chan_ptr);
		if (bonded_chan_ptr)
			ch_params->center_freq_seg0 =
				(bonded_chan_ptr->start_ch +
				 bonded_chan_ptr->end_ch)/2;
	}

	reg_debug("ch %d ch_wd %d freq0 %d freq1 %d", ch,
		  ch_params->ch_width, ch_params->center_freq_seg0,
		  ch_params->center_freq_seg1);
}

/**
 * reg_set_2g_channel_params() - set the 2.4G bonded channel parameters
 * @oper_ch: operating channel
 * @ch_params: channel parameters
 * @sec_ch_2g: 2.4G secondary channel
 *
 * Return: void
 */
static void reg_set_2g_channel_params(struct wlan_objmgr_pdev *pdev,
		uint16_t oper_ch, struct ch_params *ch_params,
		uint16_t sec_ch_2g)
{
	enum channel_state chan_state = CHANNEL_STATE_ENABLE;

	if (CH_WIDTH_MAX <= ch_params->ch_width)
		ch_params->ch_width = CH_WIDTH_40MHZ;
	if ((reg_get_bw_value(ch_params->ch_width) > 20) && !sec_ch_2g) {
		if (oper_ch >= 1 && oper_ch <= 5)
			sec_ch_2g = oper_ch + 4;
		else if (oper_ch >= 6 && oper_ch <= 13)
			sec_ch_2g = oper_ch - 4;
	}

	while (ch_params->ch_width != CH_WIDTH_INVALID) {
		chan_state = reg_get_2g_bonded_channel_state(pdev, oper_ch,
							  sec_ch_2g,
							  ch_params->ch_width);
		if (CHANNEL_STATE_ENABLE == chan_state) {
			if (CH_WIDTH_40MHZ == ch_params->ch_width) {
				if (oper_ch < sec_ch_2g)
					ch_params->sec_ch_offset =
						LOW_PRIMARY_CH;
				else
					ch_params->sec_ch_offset =
						HIGH_PRIMARY_CH;
				ch_params->center_freq_seg0 =
					(oper_ch + sec_ch_2g)/2;
			} else {
				ch_params->sec_ch_offset = NO_SEC_CH;
				ch_params->center_freq_seg0 = oper_ch;
			}
			break;
		}

		ch_params->ch_width = get_next_lower_bw[ch_params->ch_width];
	}
}

/**
 * reg_set_channel_params () - Sets channel parameteres for given bandwidth
 * @ch: channel number.
 * @ch_params: pointer to the channel parameters.
 *
 * Return: None
 */
void reg_set_channel_params(struct wlan_objmgr_pdev *pdev,
			    uint8_t ch, uint8_t sec_ch_2g,
			    struct ch_params *ch_params)
{
	if (REG_IS_5GHZ_CH(ch))
		reg_set_5g_channel_params(pdev, ch, ch_params);
	else if  (REG_IS_24GHZ_CH(ch))
		reg_set_2g_channel_params(pdev, ch, ch_params,
					  sec_ch_2g);
}

QDF_STATUS reg_get_curr_band(struct wlan_objmgr_pdev *pdev,
			     enum band_info *band)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_reg;

	pdev_reg = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_reg)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	*band = pdev_reg->band_capability;

	return QDF_STATUS_SUCCESS;
}

static bool
reg_is_world_ctry_code(uint16_t ctry_code)
{
	if ((ctry_code & 0xFFF0) == DEFAULT_WORLD_REGDMN)
		return true;

	return false;
}

QDF_STATUS reg_read_default_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country_code)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;

	if (!country_code) {
		reg_err("country_code is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_copy(country_code,
		     psoc_reg->def_country,
		     REG_ALPHA2_LEN + 1);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_read_current_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country_code)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;

	if (!country_code) {
		reg_err("country_code is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_copy(country_code,
		     psoc_reg->cur_country,
		     REG_ALPHA2_LEN + 1);

	return QDF_STATUS_SUCCESS;
}
/**
 * reg_set_default_country() - Read the default country for the regdomain
 * @country: country code.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS reg_set_default_country(struct wlan_objmgr_psoc *psoc,
				   uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;

	if (!country) {
		reg_err("country is NULL");
		return QDF_STATUS_E_INVAL;
	}
	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("setting default_country: %s", country);

	qdf_mem_copy(psoc_reg->def_country,
		     country, REG_ALPHA2_LEN + 1);

	return QDF_STATUS_SUCCESS;
}

bool reg_is_world_alpha2(uint8_t *alpha2)
{
	if ((alpha2[0] == '0') && (alpha2[1] == '0'))
		return true;

	return false;
}

bool reg_is_us_alpha2(uint8_t *alpha2)
{
	if ((alpha2[0] == 'U') && (alpha2[1] == 'S'))
		return true;

	return false;
}

QDF_STATUS reg_set_country(struct wlan_objmgr_pdev *pdev,
			   uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;
	struct set_country country_code;
	struct wlan_objmgr_psoc *psoc;
	struct cc_regdmn_s rd;

	if (!country) {
		reg_err("country code is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!qdf_mem_cmp(psoc_reg->cur_country,
			country, REG_ALPHA2_LEN)) {
		reg_err("country is not different");
		return QDF_STATUS_SUCCESS;
	}

	reg_debug("programming new country:%s to firmware", country);

	qdf_mem_copy(country_code.country,
		     country, REG_ALPHA2_LEN + 1);
	country_code.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	if (reg_is_world_alpha2(country))
		psoc_reg->world_country_pending = true;
	else
		psoc_reg->new_user_ctry_pending = true;

	if (psoc_reg->offload_enabled) {
		tx_ops = reg_get_psoc_tx_ops(psoc);
		if (tx_ops->set_country_code) {
			tx_ops->set_country_code(psoc, &country_code);
		} else {
			reg_err("country set fw handler not present");
			psoc_reg->new_user_ctry_pending = false;
			return QDF_STATUS_E_FAULT;
		}
	} else {
		if (reg_is_world_alpha2(country)) {
			pdev_priv_obj = reg_get_pdev_obj(pdev);
			if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
				reg_err("reg component pdev priv is NULL");
				return QDF_STATUS_E_INVAL;
			}
			if (reg_is_world_ctry_code(
				    pdev_priv_obj->def_region_domain))
				rd.cc.regdmn_id =
					pdev_priv_obj->def_region_domain;
			else
				rd.cc.regdmn_id = DEFAULT_WORLD_REGDMN;
			rd.flags = REGDMN_IS_SET;
		} else {
			qdf_mem_copy(rd.cc.alpha, country,
				     REG_ALPHA2_LEN + 1);
			rd.flags = ALPHA_IS_SET;
		}

		reg_program_chan_list(pdev, &rd);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_set_11d_country(struct wlan_objmgr_pdev *pdev,
			       uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;
	struct set_country country_code;
	struct wlan_objmgr_psoc *psoc;
	struct cc_regdmn_s rd;
	QDF_STATUS status;

	if (!country) {
		reg_err("country code is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!qdf_mem_cmp(psoc_reg->cur_country,
			 country, REG_ALPHA2_LEN)) {
		reg_debug("country is not different");
		return QDF_STATUS_SUCCESS;
	}

	reg_info("programming new 11d country:%c%c to firmware",
		 country[0], country[1]);

	qdf_mem_copy(country_code.country,
		     country, REG_ALPHA2_LEN + 1);
	country_code.pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	psoc_reg->new_11d_ctry_pending = true;

	if (psoc_reg->offload_enabled) {
		reg_err("reg offload, 11d offload too!");
		status = QDF_STATUS_E_FAULT;
	} else {
		qdf_mem_copy(rd.cc.alpha, country, REG_ALPHA2_LEN + 1);
		rd.flags = ALPHA_IS_SET;
		reg_program_chan_list(pdev, &rd);
		status = QDF_STATUS_SUCCESS;
	}

	return status;
}

QDF_STATUS reg_reset_country(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_reg;

	psoc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("re-setting user country to default");
	qdf_mem_copy(psoc_reg->cur_country,
		     psoc_reg->def_country,
		     REG_ALPHA2_LEN + 1);

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_get_current_dfs_region () - Get the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void reg_get_current_dfs_region(struct wlan_objmgr_pdev *pdev,
			enum dfs_reg *dfs_reg)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg component pdev priv is NULL");
		return;
	}

	*dfs_reg = pdev_priv_obj->dfs_region;
}

#ifdef CONFIG_LEGACY_CHAN_ENUM
static void reg_init_channel_map(enum dfs_reg dfs_region)
{
	channel_map = channel_map_old;
}
#else
static void reg_init_channel_map(enum dfs_reg dfs_region)
{
	switch (dfs_region) {
	case DFS_UNINIT_REG:
	case DFS_UNDEF_REG:
		channel_map = channel_map_global;
		break;
	case DFS_FCC_REG:
		channel_map = channel_map_us;
		break;
	case DFS_ETSI_REG:
		channel_map = channel_map_eu;
		break;
	case DFS_MKK_REG:
		channel_map = channel_map_jp;
		break;
	case DFS_CN_REG:
		channel_map = channel_map_china;
		break;
	case DFS_KR_REG:
		channel_map = channel_map_eu;
		break;
	}
}
#endif


/**
 * reg_set_dfs_region () - Set the current dfs region
 * @dfs_reg: pointer to dfs region
 *
 * Return: None
 */
void reg_set_dfs_region(struct wlan_objmgr_pdev *pdev,
			enum dfs_reg dfs_reg)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return;
	}

	pdev_priv_obj->dfs_region = dfs_reg;

	reg_init_channel_map(dfs_reg);
}

QDF_STATUS reg_get_domain_from_country_code(v_REGDOMAIN_t *reg_domain_ptr,
					    const uint8_t *country_alpha2,
					    enum country_src source)
{
	if (NULL == reg_domain_ptr) {
		reg_err("Invalid reg domain pointer");
		return QDF_STATUS_E_FAULT;
	}

	*reg_domain_ptr = 0;

	if (NULL == country_alpha2) {
		reg_err("Country code array is NULL");
		return QDF_STATUS_E_FAULT;
	}

	return QDF_STATUS_SUCCESS;
}

uint32_t reg_get_channel_reg_power(struct wlan_objmgr_pdev *pdev,
				   uint32_t chan_num)
{
	enum channel_enum chan_enum;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *reg_channels;

	chan_enum = reg_get_chan_enum(chan_num);

	if (chan_enum == INVALID_CHANNEL) {
		reg_err("channel is invalid");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	reg_channels = pdev_priv_obj->cur_chan_list;

	return reg_channels[chan_enum].tx_power;
}

uint32_t reg_get_channel_freq(struct wlan_objmgr_pdev *pdev,
			      uint32_t chan_num)
{
	enum channel_enum chan_enum;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *reg_channels;

	chan_enum = reg_get_chan_enum(chan_num);

	if (chan_enum == INVALID_CHANNEL)
		return CHANNEL_STATE_INVALID;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	reg_channels = pdev_priv_obj->cur_chan_list;

	return reg_channels[chan_enum].center_freq;
}


bool reg_is_dfs_ch(struct wlan_objmgr_pdev *pdev,
		   uint32_t chan)
{
	enum channel_state ch_state;

	ch_state = reg_get_channel_state(pdev, chan);

	return ch_state == CHANNEL_STATE_DFS;
}

bool reg_is_passive_or_disable_ch(struct wlan_objmgr_pdev *pdev,
				  uint32_t chan)
{
	enum channel_state ch_state;

	ch_state = reg_get_channel_state(pdev, chan);

	return (ch_state == CHANNEL_STATE_DFS) ||
		(ch_state == CHANNEL_STATE_DISABLE);
}


uint32_t reg_freq_to_chan(struct wlan_objmgr_pdev *pdev,
			  uint32_t freq)
{
	uint32_t count;
	struct regulatory_channel *chan_list;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	chan_list = pdev_priv_obj->cur_chan_list;

	for (count = 0; count < NUM_CHANNELS; count++)
		if (chan_list[count].center_freq == freq)
			return chan_list[count].chan_num;

	reg_err("invalid frequency %d", freq);

	return 0;
}

uint32_t reg_chan_to_freq(struct wlan_objmgr_pdev *pdev,
			  uint32_t chan_num)
{
	uint32_t count;
	struct regulatory_channel *chan_list;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	chan_list = pdev_priv_obj->cur_chan_list;

	for (count = 0; count < NUM_CHANNELS; count++)
		if (chan_list[count].chan_num == chan_num)
			return chan_list[count].center_freq;

	reg_err("invalid channel %d", chan_num);

	return 0;
}

enum band_info reg_chan_to_band(uint32_t chan_num)
{
	if (chan_num <= 14)
		return BAND_2G;

	return BAND_5G;
}

uint16_t reg_dmn_get_chanwidth_from_opclass(uint8_t *country,
					    uint8_t channel,
					    uint8_t opclass)
{
	const struct reg_dmn_op_class_map_t *class;
	uint16_t i;

	if (!qdf_mem_cmp(country, "US", 2))
		class = us_op_class;
	else if (!qdf_mem_cmp(country, "EU", 2))
		class = euro_op_class;
	else if (!qdf_mem_cmp(country, "JP", 2))
		class = japan_op_class;
	else
		class = global_op_class;

	while (class->op_class) {
		if (opclass == class->op_class) {
			for (i = 0;
			     (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
			      class->channels[i]);
			     i++) {
				if (channel == class->channels[i])
					return class->ch_spacing;
			}
		}
		class++;
	}

	return 0;
}


uint16_t reg_dmn_get_opclass_from_channel(uint8_t *country,
					  uint8_t channel,
					  uint8_t offset)
{
	const struct reg_dmn_op_class_map_t *class = NULL;
	uint16_t i = 0;

	if (!qdf_mem_cmp(country, "US", 2))
		class = us_op_class;
	else if (!qdf_mem_cmp(country, "EU", 2))
		class = euro_op_class;
	else if (!qdf_mem_cmp(country, "JP", 2))
		class = japan_op_class;
	else
		class = global_op_class;

	while (class->op_class) {
		if ((offset == class->offset) || (offset == BWALL)) {
			for (i = 0;
			     (i < REG_MAX_CHANNELS_PER_OPERATING_CLASS &&
			      class->channels[i]); i++) {
				if (channel == class->channels[i])
					return class->op_class;
			}
		}
		class++;
	}

	return 0;
}

uint16_t reg_dmn_set_curr_opclasses(uint8_t num_classes,
				    uint8_t *class)
{
	uint8_t i;

	if (REG_MAX_SUPP_OPER_CLASSES < num_classes) {
		reg_err("invalid num classes %d", num_classes);
		return 0;
	}

	for (i = 0; i < num_classes; i++)
		reg_dmn_curr_supp_opp_classes.classes[i] = class[i];

	reg_dmn_curr_supp_opp_classes.num_classes = num_classes;

	return 0;
}

uint16_t reg_dmn_get_curr_opclasses(uint8_t *num_classes,
				    uint8_t *class)
{
	uint8_t i;

	if (!num_classes || !class) {
		reg_err("either num_classes or class is null");
		return 0;
	}

	for (i = 0; i < reg_dmn_curr_supp_opp_classes.num_classes; i++)
		class[i] = reg_dmn_curr_supp_opp_classes.classes[i];

	*num_classes = reg_dmn_curr_supp_opp_classes.num_classes;

	return 0;
}


static void reg_fill_channel_info(enum channel_enum chan_enum,
				  struct cur_reg_rule *reg_rule,
				  struct regulatory_channel *master_list,
				  uint16_t min_bw)
{

	master_list[chan_enum].chan_flags &=
		~REGULATORY_CHAN_DISABLED;

	master_list[chan_enum].tx_power = reg_rule->reg_power;
	master_list[chan_enum].ant_gain = reg_rule->ant_gain;
	master_list[chan_enum].state = CHANNEL_STATE_ENABLE;

	if (reg_rule->flags & REGULATORY_CHAN_NO_IR) {

		master_list[chan_enum].chan_flags |=
			REGULATORY_CHAN_NO_IR;

		master_list[chan_enum].state =
			CHANNEL_STATE_DFS;
	}

	if (reg_rule->flags & REGULATORY_CHAN_RADAR) {
		master_list[chan_enum].chan_flags |=
			REGULATORY_CHAN_RADAR;

		master_list[chan_enum].state =
			CHANNEL_STATE_DFS;
	}

	if (reg_rule->flags & REGULATORY_CHAN_INDOOR_ONLY)
		master_list[chan_enum].chan_flags |=
			REGULATORY_CHAN_INDOOR_ONLY;

	if (reg_rule->flags & REGULATORY_CHAN_NO_OFDM)
		master_list[chan_enum].chan_flags |=
			REGULATORY_CHAN_NO_OFDM;

	master_list[chan_enum].min_bw = min_bw;
	if (20 == master_list[chan_enum].max_bw)
		master_list[chan_enum].max_bw = reg_rule->max_bw;
}


static void reg_populate_band_channels(enum channel_enum start_chan,
				       enum channel_enum end_chan,
				       struct cur_reg_rule *rule_start_ptr,
				       uint32_t num_reg_rules,
				       uint16_t min_reg_bw,
				       struct regulatory_channel *mas_chan_list)
{
	struct cur_reg_rule *found_rule_ptr;
	struct cur_reg_rule *cur_rule_ptr;
	struct regulatory_channel;
	enum channel_enum chan_enum;
	uint32_t rule_num, bw;
	uint16_t max_bw;
	uint16_t min_bw;

	for (chan_enum = start_chan; chan_enum <= end_chan; chan_enum++) {
		found_rule_ptr = NULL;

		max_bw = QDF_MIN((uint16_t)20, channel_map[chan_enum].max_bw);
		min_bw = QDF_MAX(min_reg_bw, channel_map[chan_enum].min_bw);

		if (channel_map[chan_enum].chan_num == INVALID_CHANNEL_NUM)
			continue;

		for (bw = max_bw; bw >= min_bw; bw = bw/2) {
			for (rule_num = 0, cur_rule_ptr =
				     rule_start_ptr;
			     rule_num < num_reg_rules;
			     cur_rule_ptr++, rule_num++) {

				if ((cur_rule_ptr->start_freq <=
				     mas_chan_list[chan_enum].center_freq -
				     bw/2) &&
				    (cur_rule_ptr->end_freq >=
				     mas_chan_list[chan_enum].center_freq +
				     bw/2) && (min_bw <= bw)) {
					found_rule_ptr = cur_rule_ptr;
					break;
				}
			}
			if (found_rule_ptr)
				break;
		}

		if (found_rule_ptr) {
			mas_chan_list[chan_enum].max_bw = bw;
			reg_fill_channel_info(chan_enum, found_rule_ptr,
					      mas_chan_list, min_bw);
			/* Disable 2.4 Ghz channels that dont have 20 mhz bw */
			if (start_chan == MIN_24GHZ_CHANNEL &&
			    20 > mas_chan_list[chan_enum].max_bw) {
				mas_chan_list[chan_enum].chan_flags |=
						REGULATORY_CHAN_DISABLED;
				mas_chan_list[chan_enum].state =
						REGULATORY_CHAN_DISABLED;
			}
		}
	}
}

static void reg_update_max_bw_per_rule(uint32_t num_reg_rules,
				       struct cur_reg_rule *reg_rule_start,
				       uint16_t max_bw)
{
	uint32_t count;

	for (count = 0; count < num_reg_rules; count++)
		reg_rule_start[count].max_bw =
			min(reg_rule_start[count].max_bw, max_bw);
}

static void reg_do_auto_bw_correction(uint32_t num_reg_rules,
				  struct cur_reg_rule *reg_rule_ptr,
				  uint16_t max_bw)
{
	uint32_t count;
	uint16_t new_bw;

	for (count = 0; count < num_reg_rules - 1; count++) {
		if ((reg_rule_ptr[count].end_freq ==
		     reg_rule_ptr[count+1].start_freq) &&
		    ((reg_rule_ptr[count].max_bw + reg_rule_ptr[count+1].max_bw)
		     <= max_bw)) {
			new_bw = reg_rule_ptr[count].max_bw +
				reg_rule_ptr[count+1].max_bw;
			reg_rule_ptr[count].max_bw = new_bw;
			reg_rule_ptr[count+1].max_bw = new_bw;
		}
	}
}

static void reg_modify_chan_list_for_dfs_channels(struct regulatory_channel
					      *chan_list,
					      bool dfs_enabled)
{
	enum channel_enum chan_enum;

	if (dfs_enabled)
		return;

	for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
		if (CHANNEL_STATE_DFS == chan_list[chan_enum].state) {
			chan_list[chan_enum].state =
				CHANNEL_STATE_DISABLE;
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
		}
	}
}
static void reg_modify_chan_list_for_indoor_channels(
		struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj)
{
	enum channel_enum chan_enum;
	struct regulatory_channel *chan_list = pdev_priv_obj->cur_chan_list;

	if (pdev_priv_obj->indoor_chan_enabled)
		return;

	if (!pdev_priv_obj->force_ssc_disable_indoor_channel) {
		for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
			if (REGULATORY_CHAN_INDOOR_ONLY &
			    chan_list[chan_enum].chan_flags) {
				chan_list[chan_enum].state =
					CHANNEL_STATE_DISABLE;
				chan_list[chan_enum].chan_flags |=
					REGULATORY_CHAN_DISABLED;
			}
		}
	} else {

		for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {

			if (pdev_priv_obj->sap_state) {

				if (REGULATORY_CHAN_INDOOR_ONLY &
					chan_list[chan_enum].chan_flags) {
					chan_list[chan_enum].state =
						CHANNEL_STATE_DISABLE;
					chan_list[chan_enum].chan_flags |=
						REGULATORY_CHAN_DISABLED;
				}
			} else {

				if (REGULATORY_CHAN_INDOOR_ONLY &
					chan_list[chan_enum].chan_flags) {
					chan_list[chan_enum].state =
						CHANNEL_STATE_DFS;
					chan_list[chan_enum].chan_flags |=
						REGULATORY_CHAN_NO_IR;
				}
			}
		}
	}
}

static void reg_modify_chan_list_for_band(struct regulatory_channel *chan_list,
					  enum band_info band_val)
{
	enum channel_enum chan_enum;

	if (BAND_2G == band_val) {
		for (chan_enum = MIN_5GHZ_CHANNEL;
		     chan_enum <= MAX_5GHZ_CHANNEL;
		     chan_enum++) {
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
			chan_list[chan_enum].state = CHANNEL_STATE_DISABLE;
		}
	}

	if (BAND_5G == band_val) {
		for (chan_enum = MIN_24GHZ_CHANNEL;
		     chan_enum <= MAX_24GHZ_CHANNEL;
		     chan_enum++) {
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
			chan_list[chan_enum].state = CHANNEL_STATE_DISABLE;
		}
	}
}

static void reg_modify_chan_list_for_fcc_channel(struct regulatory_channel
						 *chan_list,
						 bool set_fcc_channel)
{
	enum channel_enum chan_enum;

	if (set_fcc_channel) {
		for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
			if (chan_list[chan_enum].center_freq ==
			    CHAN_12_CENT_FREQ)
				chan_list[chan_enum].tx_power =
					MAX_PWR_FCC_CHAN_12;
			if (chan_list[chan_enum].center_freq ==
			    CHAN_13_CENT_FREQ)
				chan_list[chan_enum].tx_power =
					MAX_PWR_FCC_CHAN_13;
		}
	}
}

static void reg_modify_chan_list_for_chan_144(struct regulatory_channel
					      *chan_list,
					      bool en_chan_144)
{
	enum channel_enum chan_enum;

	if (en_chan_144)
		return;

	for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
		if (chan_list[chan_enum].center_freq == CHAN_144_CENT_FREQ) {
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
			chan_list[chan_enum].state =
				CHANNEL_STATE_DISABLE;
		}
	}
}

static void
reg_modify_chan_list_for_nol_list(struct regulatory_channel *chan_list)
{
	enum channel_enum chan_enum;

	for (chan_enum = 0; chan_enum < NUM_CHANNELS;
	     chan_enum++) {
		if (chan_list[chan_enum].nol_chan) {
			chan_list[chan_enum].state =
				CHANNEL_STATE_DISABLE;
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
		}
	}
}

/**
 * reg_find_low_limit_chan_enum() - Find low limit 2G and 5G channel enums.
 * @chan_list: Pointer to regulatory channel list.
 * @low_freq: low limit frequency.
 * @low_limit: pointer to output low limit enum.
 *
 * Return: None
 */
static void reg_find_low_limit_chan_enum(struct regulatory_channel *chan_list,
		uint32_t low_freq,
		uint32_t *low_limit)
{
	enum channel_enum chan_enum;
	uint16_t min_bw;
	uint16_t max_bw;
	uint32_t center_freq;

	for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
		min_bw = chan_list[chan_enum].min_bw;
		max_bw = chan_list[chan_enum].max_bw;
		center_freq = chan_list[chan_enum].center_freq;

		if ((center_freq - min_bw/2) >= low_freq) {
			if ((center_freq - max_bw/2) < low_freq) {
				if (max_bw <= 20)
					max_bw = ((center_freq -
						   low_freq) * 2);
				if (max_bw < min_bw)
					max_bw = min_bw;
				chan_list[chan_enum].max_bw = max_bw;
			}
			*low_limit = chan_enum;
			break;
		}
	}
}

/**
 * reg_find_high_limit_chan_enum() - Find high limit 2G and 5G channel enums.
 * @chan_list: Pointer to regulatory channel list.
 * @high_freq: high limit frequency.
 * @high_limit: pointer to output high limit enum.
 *
 * Return: None
 */
static void reg_find_high_limit_chan_enum(struct regulatory_channel *chan_list,
		uint32_t high_freq,
		uint32_t *high_limit)
{
	enum channel_enum chan_enum;
	uint16_t min_bw;
	uint16_t max_bw;
	uint32_t center_freq;

	for (chan_enum = NUM_CHANNELS - 1; chan_enum >= 0; chan_enum--) {
		min_bw = chan_list[chan_enum].min_bw;
		max_bw = chan_list[chan_enum].max_bw;
		center_freq = chan_list[chan_enum].center_freq;

		if (center_freq + min_bw/2 <= high_freq) {
			if ((center_freq + max_bw/2) > high_freq) {
				if (max_bw <= 20)
					max_bw = ((high_freq -
						   center_freq) * 2);
				if (max_bw < min_bw)
					max_bw = min_bw;
				chan_list[chan_enum].max_bw = max_bw;
			}
			*high_limit = chan_enum;
			break;
		}
		if (chan_enum == 0)
			break;
	}
}

/**
 * reg_modify_chan_list_for_freq_range() - Modify channel list for the given low
 * and high frequency range.
 * @chan_list: Pointer to regulatory channel list.
 * @low_freq_2g: Low frequency 2G.
 * @high_freq_2g: High frequency 2G.
 * @low_freq_5g: Low frequency 5G.
 * @high_freq_5g: High frequency 5G.
 *
 * Return: None
 */
static void
reg_modify_chan_list_for_freq_range(struct regulatory_channel *chan_list,
				uint32_t low_freq_2g,
				uint32_t high_freq_2g,
				uint32_t low_freq_5g,
				uint32_t high_freq_5g)
{
	uint32_t low_limit_2g = NUM_CHANNELS;
	uint32_t high_limit_2g = NUM_CHANNELS;
	uint32_t low_limit_5g = NUM_CHANNELS;
	uint32_t high_limit_5g = NUM_CHANNELS;
	enum channel_enum chan_enum;
	bool chan_in_range;

	reg_find_low_limit_chan_enum(chan_list, low_freq_2g, &low_limit_2g);
	reg_find_low_limit_chan_enum(chan_list, low_freq_5g, &low_limit_5g);
	reg_find_high_limit_chan_enum(chan_list, high_freq_2g, &high_limit_2g);
	reg_find_high_limit_chan_enum(chan_list, high_freq_5g, &high_limit_5g);

	for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
		chan_in_range = false;
		if  ((low_limit_2g <= chan_enum) &&
		     (high_limit_2g >= chan_enum) &&
		     (low_limit_2g != NUM_CHANNELS) &&
		     (high_limit_2g != NUM_CHANNELS))
			chan_in_range = true;
		if  ((low_limit_5g <= chan_enum) &&
		     (high_limit_5g >= chan_enum) &&
		     (low_limit_5g != NUM_CHANNELS) &&
		     (high_limit_5g != NUM_CHANNELS))
			chan_in_range = true;
		if (!chan_in_range) {
			chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
			chan_list[chan_enum].state =
				CHANNEL_STATE_DISABLE;
		}
	}
}

static void reg_init_pdev_mas_chan_list(struct wlan_regulatory_pdev_priv_obj
				    *pdev_priv_obj,
				    struct mas_chan_params
				    *mas_chan_params)
{
	qdf_mem_copy(pdev_priv_obj->mas_chan_list,
		     mas_chan_params->mas_chan_list,
		     NUM_CHANNELS * sizeof(struct regulatory_channel));

	pdev_priv_obj->dfs_region = mas_chan_params->dfs_region;

	pdev_priv_obj->phybitmap = mas_chan_params->phybitmap;

	pdev_priv_obj->reg_dmn_pair = mas_chan_params->reg_dmn_pair;
	pdev_priv_obj->ctry_code =  mas_chan_params->ctry_code;

	pdev_priv_obj->def_region_domain = mas_chan_params->reg_dmn_pair;
	pdev_priv_obj->def_country_code =  mas_chan_params->ctry_code;

	qdf_mem_copy(pdev_priv_obj->default_country,
		     mas_chan_params->default_country, REG_ALPHA2_LEN + 1);

	qdf_mem_copy(pdev_priv_obj->current_country,
		     mas_chan_params->current_country, REG_ALPHA2_LEN + 1);
}


static void reg_compute_pdev_current_chan_list(
			struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj)
{
	qdf_mem_copy(pdev_priv_obj->cur_chan_list,
		     pdev_priv_obj->mas_chan_list,
		     NUM_CHANNELS * sizeof(struct regulatory_channel));

	reg_modify_chan_list_for_freq_range(pdev_priv_obj->cur_chan_list,
					    pdev_priv_obj->range_2g_low,
					    pdev_priv_obj->range_2g_high,
					    pdev_priv_obj->range_5g_low,
					    pdev_priv_obj->range_5g_high);

	reg_modify_chan_list_for_band(pdev_priv_obj->cur_chan_list,
				      pdev_priv_obj->band_capability);

	reg_modify_chan_list_for_dfs_channels(pdev_priv_obj->cur_chan_list,
					      pdev_priv_obj->dfs_enabled);

	reg_modify_chan_list_for_nol_list(pdev_priv_obj->cur_chan_list);

	reg_modify_chan_list_for_indoor_channels(pdev_priv_obj);

	reg_modify_chan_list_for_fcc_channel(pdev_priv_obj->cur_chan_list,
					     pdev_priv_obj->set_fcc_channel);

	reg_modify_chan_list_for_chan_144(pdev_priv_obj->cur_chan_list,
					  pdev_priv_obj->en_chan_144);
}

static void reg_call_chan_change_cbks(struct wlan_objmgr_psoc *psoc,
				  struct wlan_objmgr_pdev *pdev)
{
	struct chan_change_cbk_entry *cbk_list;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	uint32_t ctr;
	struct avoid_freq_ind_data *avoid_freq_ind = NULL;
	reg_chan_change_callback callback;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_alert("psoc reg component is NULL");
		return;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_alert("pdev reg component is NULL");
		return;
	}

	cur_chan_list = qdf_mem_malloc(NUM_CHANNELS * sizeof(*cur_chan_list));
	if (NULL == cur_chan_list) {
		reg_alert("Mem alloc failed for current channel list");
		return;
	}

	qdf_mem_copy(cur_chan_list,
		     pdev_priv_obj->cur_chan_list,
		     NUM_CHANNELS *
		     sizeof(struct regulatory_channel));

	if (psoc_priv_obj->ch_avoid_ind) {
		avoid_freq_ind = qdf_mem_malloc(sizeof(*avoid_freq_ind));
		if (!avoid_freq_ind) {
			reg_alert("Mem alloc failed for avoid freq ind");
			goto skip_ch_avoid_ind;
		}
		qdf_mem_copy(&avoid_freq_ind->freq_list,
				&psoc_priv_obj->avoid_freq_list,
				sizeof(struct ch_avoid_ind_type));
		qdf_mem_copy(&avoid_freq_ind->chan_list,
				&psoc_priv_obj->unsafe_chan_list,
				sizeof(struct unsafe_ch_list));
		psoc_priv_obj->ch_avoid_ind = false;
	}

skip_ch_avoid_ind:
	cbk_list = psoc_priv_obj->cbk_list;

	for (ctr = 0; ctr < REG_MAX_CHAN_CHANGE_CBKS; ctr++) {
		callback  = NULL;
		qdf_spin_lock_bh(&psoc_priv_obj->cbk_list_lock);
		if (cbk_list[ctr].cbk != NULL)
			callback = cbk_list[ctr].cbk;
		qdf_spin_unlock_bh(&psoc_priv_obj->cbk_list_lock);
		if (callback != NULL)
			callback(psoc, pdev, cur_chan_list, avoid_freq_ind,
					cbk_list[ctr].arg);
	}
	qdf_mem_free(cur_chan_list);
	if (avoid_freq_ind)
		qdf_mem_free(avoid_freq_ind);
}

static struct reg_sched_payload
*reg_alloc_and_fill_payload(struct wlan_objmgr_psoc *psoc,
			    struct wlan_objmgr_pdev *pdev)
{
	struct reg_sched_payload *payload;

	payload = qdf_mem_malloc(sizeof(*payload));
	if (payload != NULL) {
		payload->psoc = psoc;
		payload->pdev = pdev;
	}

	return payload;
}

static QDF_STATUS reg_chan_change_flush_cbk_sb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS reg_sched_chan_change_cbks_sb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	reg_call_chan_change_cbks(psoc, pdev);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS reg_chan_change_flush_cbk_nb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS reg_sched_chan_change_cbks_nb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	reg_call_chan_change_cbks(psoc, pdev);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_send_scheduler_msg_sb(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev)
{
	struct scheduler_msg msg = {0};
	struct reg_sched_payload *payload;
	QDF_STATUS status;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}

	status = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
		reg_err("error taking pdev ref cnt");
		return status;
	}

	payload = reg_alloc_and_fill_payload(psoc, pdev);
	if (payload == NULL) {
		reg_err("payload memory alloc failed");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
		return QDF_STATUS_E_NOMEM;
	}

	msg.bodyptr = payload;
	msg.callback = reg_sched_chan_change_cbks_sb;
	msg.flush_callback = reg_chan_change_flush_cbk_sb;

	status = scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
		reg_err("scheduler msg posting failed");
		qdf_mem_free(payload);
	}

	return status;
}

static QDF_STATUS reg_send_scheduler_msg_nb(struct wlan_objmgr_psoc *psoc,
					struct wlan_objmgr_pdev *pdev)
{
	struct scheduler_msg msg = {0};
	struct reg_sched_payload *payload;
	QDF_STATUS status;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}

	status = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_REGULATORY_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
		reg_err("error taking pdev ref cnt");
		return status;
	}

	payload = reg_alloc_and_fill_payload(psoc, pdev);
	if (payload == NULL) {
		reg_err("payload memory alloc failed");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
		return QDF_STATUS_E_NOMEM;
	}
	msg.bodyptr = payload;
	msg.callback = reg_sched_chan_change_cbks_nb;
	msg.flush_callback = reg_chan_change_flush_cbk_nb;

	status = scheduler_post_msg(QDF_MODULE_ID_OS_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
		reg_err("scheduler msg posting failed");
		qdf_mem_free(payload);
	}

	return status;
}

static QDF_STATUS reg_send_11d_flush_cbk(struct scheduler_msg *msg)
{
	struct wlan_objmgr_psoc *psoc = msg->bodyptr;

	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS reg_send_11d_msg_cbk(struct scheduler_msg *msg)
{
	struct wlan_objmgr_psoc *psoc = msg->bodyptr;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;
	struct reg_start_11d_scan_req start_req;
	struct reg_stop_11d_scan_req stop_req;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	tx_ops = reg_get_psoc_tx_ops(psoc);

	psoc_priv_obj = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
					 WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("psoc priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (psoc_priv_obj->enable_11d_supp) {
		start_req.vdev_id = psoc_priv_obj->vdev_id_for_11d_scan;
		start_req.scan_period_msec = psoc_priv_obj->scan_11d_interval;
		start_req.start_interval_msec = 0;
		reg_debug("sending start msg");
		tx_ops->start_11d_scan(psoc, &start_req);
	} else {
		stop_req.vdev_id = psoc_priv_obj->vdev_id_for_11d_scan;
		reg_debug("sending stop msg");
		tx_ops->stop_11d_scan(psoc, &stop_req);
	}

	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS reg_sched_11d_msg(struct wlan_objmgr_psoc *psoc)
{
	struct scheduler_msg msg = {0};
	QDF_STATUS status;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}

	msg.bodyptr = psoc;
	msg.callback = reg_send_11d_msg_cbk;
	msg.flush_callback = reg_send_11d_flush_cbk;

	status = scheduler_post_msg(QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
		reg_err("scheduler msg posting failed");
	}

	return status;
}


static void reg_propagate_mas_chan_list_to_pdev(struct wlan_objmgr_psoc *psoc,
						void *object, void *arg)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)object;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	enum direction *dir = arg;
	uint32_t pdev_id;
	struct wlan_lmac_if_reg_tx_ops *reg_tx_ops;

	psoc_priv_obj = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				       WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("psoc priv obj is NULL");
		return;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	reg_init_pdev_mas_chan_list(pdev_priv_obj,
		      &psoc_priv_obj->mas_chan_params[pdev_id]);

	reg_compute_pdev_current_chan_list(pdev_priv_obj);

	reg_tx_ops = reg_get_psoc_tx_ops(psoc);
	if (reg_tx_ops->fill_umac_legacy_chanlist) {
		reg_tx_ops->fill_umac_legacy_chanlist(pdev,
				pdev_priv_obj->cur_chan_list);
	} else {
		if (*dir == NORTHBOUND)
			reg_send_scheduler_msg_nb(psoc, pdev);
		else
			reg_send_scheduler_msg_sb(psoc, pdev);
	}
}

static void reg_run_11d_state_machine(struct wlan_objmgr_psoc *psoc)
{
	bool temp_11d_support;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						 WLAN_UMAC_COMP_REGULATORY);
	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	temp_11d_support = psoc_priv_obj->enable_11d_supp;
	if (((psoc_priv_obj->user_ctry_set) &&
	     (psoc_priv_obj->user_ctry_priority)) ||
	    (psoc_priv_obj->master_vdev_cnt))
		psoc_priv_obj->enable_11d_supp = false;
	else
		psoc_priv_obj->enable_11d_supp =
			psoc_priv_obj->enable_11d_supp_original;

	reg_debug("inside 11d state machine");
	if ((temp_11d_support != psoc_priv_obj->enable_11d_supp) &&
	    (psoc_priv_obj->is_11d_offloaded)) {
		reg_sched_11d_msg(psoc);
	}
}

QDF_STATUS reg_process_master_chan_list(struct cur_regulatory_info
					*regulat_info)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;
	uint32_t num_2g_reg_rules, num_5g_reg_rules;
	struct cur_reg_rule *reg_rule_2g, *reg_rule_5g;
	uint16_t min_bw_2g, max_bw_2g, min_bw_5g, max_bw_5g;
	struct regulatory_channel *mas_chan_list;
	struct wlan_objmgr_psoc *psoc;
	enum channel_enum chan_enum;
	wlan_objmgr_ref_dbgid dbg_id;
	enum direction dir;
	uint8_t phy_id;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;

	reg_debug("process reg master chan list");

	psoc = regulat_info->psoc;
	soc_reg = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	phy_id = regulat_info->phy_id;

	if (soc_reg->offload_enabled) {
		dbg_id = WLAN_REGULATORY_NB_ID;
		dir = NORTHBOUND;
	} else {
		dbg_id = WLAN_REGULATORY_SB_ID;
		dir = SOUTHBOUND;
	}

	if (regulat_info->status_code != REG_SET_CC_STATUS_PASS) {
		reg_err("Setting country code failed, status code is %d",
				regulat_info->status_code);

		pdev = wlan_objmgr_get_pdev_by_id(psoc, phy_id, dbg_id);
		if (!pdev) {
			reg_err("pdev is NULL");
			return QDF_STATUS_E_FAILURE;
		}
		wlan_objmgr_pdev_release_ref(pdev, dbg_id);

		tx_ops = reg_get_psoc_tx_ops(psoc);
		if (tx_ops->set_country_failed)
			tx_ops->set_country_failed(pdev);

		return QDF_STATUS_E_FAILURE;
	}

	mas_chan_list = soc_reg->mas_chan_params[phy_id].mas_chan_list;

	reg_init_channel_map(regulat_info->dfs_region);

	for (chan_enum = 0; chan_enum < NUM_CHANNELS;
	     chan_enum++) {
		mas_chan_list[chan_enum].chan_num =
			channel_map[chan_enum].chan_num;
		mas_chan_list[chan_enum].center_freq =
			channel_map[chan_enum].center_freq;
		mas_chan_list[chan_enum].chan_flags =
			REGULATORY_CHAN_DISABLED;
		mas_chan_list[chan_enum].state =
			CHANNEL_STATE_DISABLE;
		mas_chan_list[chan_enum].nol_chan = false;
	}

	soc_reg->num_phy = regulat_info->num_phy;
	soc_reg->mas_chan_params[phy_id].phybitmap =
		regulat_info->phybitmap;
	soc_reg->mas_chan_params[phy_id].dfs_region =
		regulat_info->dfs_region;
	soc_reg->mas_chan_params[phy_id].ctry_code =
		regulat_info->ctry_code;
	soc_reg->mas_chan_params[phy_id].reg_dmn_pair =
		regulat_info->reg_dmn_pair;
	qdf_mem_copy(soc_reg->mas_chan_params[phy_id].current_country,
		     regulat_info->alpha2,
		     REG_ALPHA2_LEN + 1);
	qdf_mem_copy(soc_reg->cur_country,
		     regulat_info->alpha2,
		     REG_ALPHA2_LEN + 1);

	min_bw_2g = regulat_info->min_bw_2g;
	max_bw_2g = regulat_info->max_bw_2g;
	reg_rule_2g = regulat_info->reg_rules_2g_ptr;
	num_2g_reg_rules = regulat_info->num_2g_reg_rules;
	reg_update_max_bw_per_rule(num_2g_reg_rules,
			       reg_rule_2g, max_bw_2g);

	min_bw_5g = regulat_info->min_bw_5g;
	max_bw_5g = regulat_info->max_bw_5g;
	reg_rule_5g = regulat_info->reg_rules_5g_ptr;
	num_5g_reg_rules = regulat_info->num_5g_reg_rules;
	reg_update_max_bw_per_rule(num_5g_reg_rules,
			       reg_rule_5g, max_bw_5g);

	if (num_5g_reg_rules != 0)
		reg_do_auto_bw_correction(num_5g_reg_rules,
				      reg_rule_5g, max_bw_5g);

	if (num_2g_reg_rules != 0)
		reg_populate_band_channels(MIN_24GHZ_CHANNEL, MAX_24GHZ_CHANNEL,
					   reg_rule_2g, num_2g_reg_rules,
					   min_bw_2g, mas_chan_list);

	if (num_5g_reg_rules != 0)
		reg_populate_band_channels(MIN_5GHZ_CHANNEL, MAX_5GHZ_CHANNEL,
					   reg_rule_5g, num_5g_reg_rules,
					   min_bw_5g, mas_chan_list);

	if (num_5g_reg_rules != 0)
		reg_populate_band_channels(MIN_49GHZ_CHANNEL,
					MAX_49GHZ_CHANNEL,
					reg_rule_5g, num_5g_reg_rules,
					min_bw_5g, mas_chan_list);

	if (soc_reg->new_user_ctry_pending == true) {
		soc_reg->new_user_ctry_pending = false;
		soc_reg->cc_src = SOURCE_USERSPACE;
		soc_reg->user_ctry_set = true;
		reg_debug("new country code is set");
		reg_run_11d_state_machine(psoc);
	} else if (soc_reg->new_11d_ctry_pending == true) {
		soc_reg->new_11d_ctry_pending = false;
		soc_reg->cc_src = SOURCE_11D;
		soc_reg->user_ctry_set = false;
	} else if (soc_reg->world_country_pending == true) {
		soc_reg->world_country_pending = false;
		soc_reg->cc_src = SOURCE_CORE;
		soc_reg->user_ctry_set = false;
	} else {
		soc_reg->cc_src = SOURCE_DRIVER;

		if (reg_is_world_alpha2(regulat_info->alpha2))
			soc_reg->cc_src = SOURCE_CORE;

		qdf_mem_copy(soc_reg->mas_chan_params[phy_id].default_country,
			     regulat_info->alpha2,
			     REG_ALPHA2_LEN + 1);

		soc_reg->mas_chan_params[phy_id].def_country_code =
			regulat_info->ctry_code;
		soc_reg->mas_chan_params[phy_id].def_region_domain =
			regulat_info->reg_dmn_pair;

		qdf_mem_copy(soc_reg->def_country,
			     regulat_info->alpha2,
			     REG_ALPHA2_LEN + 1);

		soc_reg->def_country_code = regulat_info->ctry_code;
		soc_reg->def_region_domain = regulat_info->reg_dmn_pair;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, phy_id, dbg_id);
	if (pdev != NULL) {
		reg_propagate_mas_chan_list_to_pdev(psoc, pdev, &dir);
		wlan_objmgr_pdev_release_ref(pdev, dbg_id);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_regulatory_psoc_obj_created_notification() - PSOC obj create callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is created.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_psoc_obj_created_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg_obj;
	struct regulatory_channel *mas_chan_list;
	enum channel_enum chan_enum;
	QDF_STATUS status;
	uint8_t i;
	uint8_t pdev_cnt;

	soc_reg_obj = qdf_mem_malloc(sizeof(*soc_reg_obj));
	if (NULL == soc_reg_obj) {
		reg_alert("Mem alloc failed for reg psoc priv obj");
		return QDF_STATUS_E_NOMEM;
	}

	soc_reg_obj->offload_enabled = false;
	soc_reg_obj->psoc_ptr = psoc;
	soc_reg_obj->dfs_enabled = true;
	soc_reg_obj->band_capability = BAND_ALL;
	soc_reg_obj->enable_11d_supp = false;
	soc_reg_obj->indoor_chan_enabled = true;
	soc_reg_obj->force_ssc_disable_indoor_channel = false;
	soc_reg_obj->master_vdev_cnt = 0;
	soc_reg_obj->vdev_cnt_11d = 0;
	soc_reg_obj->restart_beaconing = CH_AVOID_RULE_RESTART;

	for (i = 0; i < MAX_STA_VDEV_CNT; i++)
		soc_reg_obj->vdev_ids_11d[i] = INVALID_VDEV_ID;

	qdf_spinlock_create(&soc_reg_obj->cbk_list_lock);

	for (pdev_cnt = 0; pdev_cnt < PSOC_MAX_PHY_REG_CAP; pdev_cnt++) {
		mas_chan_list =
			soc_reg_obj->mas_chan_params[pdev_cnt].mas_chan_list;

		for (chan_enum = 0; chan_enum < NUM_CHANNELS;
		     chan_enum++) {
			mas_chan_list[chan_enum].chan_flags |=
				REGULATORY_CHAN_DISABLED;
			mas_chan_list[chan_enum].state =
				CHANNEL_STATE_DISABLE;
			mas_chan_list[chan_enum].nol_chan = false;
		}
	}

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
			WLAN_UMAC_COMP_REGULATORY, soc_reg_obj,
			QDF_STATUS_SUCCESS);

	reg_debug("reg psoc obj created with status %d", status);

	return status;
}

/**
 * wlan_regulatory_psoc_obj_destroyed_notification() - PSOC obj delete callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is deleted.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_psoc_obj_destroyed_notification(
	struct wlan_objmgr_psoc *psoc, void *arg_list)
{
	QDF_STATUS status;
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	soc_reg = wlan_objmgr_psoc_get_comp_private_obj(psoc,
				     WLAN_UMAC_COMP_REGULATORY);

	if (NULL == soc_reg) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	soc_reg->psoc_ptr = NULL;
	qdf_spinlock_destroy(&soc_reg->cbk_list_lock);

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
					  WLAN_UMAC_COMP_REGULATORY,
					  soc_reg);

	if (status != QDF_STATUS_SUCCESS)
		reg_err("soc_reg private obj detach failed");

	reg_debug("reg psoc obj detached with status %d", status);

	qdf_mem_free(soc_reg);

	return status;
}

QDF_STATUS reg_set_band(struct wlan_objmgr_pdev *pdev,
			enum band_info band)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (pdev_priv_obj->band_capability == band) {
		reg_info("band is already set to %d", band);
		return QDF_STATUS_SUCCESS;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("setting band_info: %d", band);
	pdev_priv_obj->band_capability = band;

	reg_compute_pdev_current_chan_list(pdev_priv_obj);

	status = reg_send_scheduler_msg_sb(psoc, pdev);

	return status;
}

QDF_STATUS reg_notify_sap_event(struct wlan_objmgr_pdev *pdev,
			bool sap_state)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("sap_state: %d", sap_state);

	if (pdev_priv_obj->sap_state == sap_state)
		return QDF_STATUS_SUCCESS;

	pdev_priv_obj->sap_state = sap_state;

	reg_compute_pdev_current_chan_list(pdev_priv_obj);
	status = reg_send_scheduler_msg_sb(psoc, pdev);

	return status;
}

QDF_STATUS reg_set_fcc_constraint(struct wlan_objmgr_pdev *pdev,
				  bool fcc_constraint)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (pdev_priv_obj->set_fcc_channel == fcc_constraint) {
		reg_info("fcc_constraint is already set to %d", fcc_constraint);
		return QDF_STATUS_SUCCESS;
	}

	reg_info("setting set_fcc_channel: %d", fcc_constraint);
	pdev_priv_obj->set_fcc_channel = fcc_constraint;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_compute_pdev_current_chan_list(pdev_priv_obj);

	status = reg_send_scheduler_msg_sb(psoc, pdev);

	return status;
}

QDF_STATUS reg_enable_dfs_channels(struct wlan_objmgr_pdev *pdev,
				   bool enable)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (pdev_priv_obj->dfs_enabled == enable) {
		reg_info("dfs_enabled is already set to %d", enable);
		return QDF_STATUS_SUCCESS;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("setting dfs_enabled: %d", enable);

	pdev_priv_obj->dfs_enabled = enable;

	reg_compute_pdev_current_chan_list(pdev_priv_obj);

	status = reg_send_scheduler_msg_sb(psoc, pdev);

	return status;
}

/**
 * wlan_regulatory_pdev_obj_created_notification() - PDEV obj create callback
 * @pdev: pdev object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the pdev object is created.
 *
 * Return: Success or Failure
 */
QDF_STATUS wlan_regulatory_pdev_obj_created_notification(
	struct wlan_objmgr_pdev *pdev, void *arg_list)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap_ptr;
	struct wlan_objmgr_psoc *parent_psoc;
	uint32_t pdev_id;
	uint32_t cnt;
	uint32_t range_2g_low, range_2g_high;
	uint32_t range_5g_low, range_5g_high;
	QDF_STATUS status;

	pdev_priv_obj = qdf_mem_malloc(sizeof(*pdev_priv_obj));
	if (NULL == pdev_priv_obj) {
		reg_alert("Mem alloc failed for pdev priv obj");
		return QDF_STATUS_E_NOMEM;
	}

	parent_psoc = wlan_pdev_get_psoc(pdev);
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	psoc_priv_obj = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(parent_psoc,
				     WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		qdf_mem_free(pdev_priv_obj);
		return QDF_STATUS_E_FAULT;
	}

	pdev_priv_obj->pdev_ptr = pdev;
	pdev_priv_obj->dfs_enabled = psoc_priv_obj->dfs_enabled;
	pdev_priv_obj->set_fcc_channel = false;
	pdev_priv_obj->band_capability =  psoc_priv_obj->band_capability;
	pdev_priv_obj->indoor_chan_enabled =
		psoc_priv_obj->indoor_chan_enabled;
	pdev_priv_obj->en_chan_144 = true;

	reg_cap_ptr = psoc_priv_obj->reg_cap;
	pdev_priv_obj->force_ssc_disable_indoor_channel =
		psoc_priv_obj->force_ssc_disable_indoor_channel;

	for (cnt = 0; cnt < PSOC_MAX_PHY_REG_CAP; cnt++) {
		if (reg_cap_ptr == NULL) {
			qdf_mem_free(pdev_priv_obj);
			reg_err(" reg cap ptr is NULL");
			return QDF_STATUS_E_FAULT;
		}

		if (reg_cap_ptr->phy_id == pdev_id)
			break;
		reg_cap_ptr++;
	}

	if (cnt == PSOC_MAX_PHY_REG_CAP) {
		qdf_mem_free(pdev_priv_obj);
		reg_err("extended capabilities not found for pdev");
		return QDF_STATUS_E_FAULT;
	}

	range_2g_low = reg_cap_ptr->low_2ghz_chan;
	range_2g_high = reg_cap_ptr->high_2ghz_chan;
	range_5g_low = reg_cap_ptr->low_5ghz_chan;
	range_5g_high = reg_cap_ptr->high_5ghz_chan;

	pdev_priv_obj->range_2g_low = range_2g_low;
	pdev_priv_obj->range_2g_high = range_2g_high;
	pdev_priv_obj->range_5g_low = range_5g_low;
	pdev_priv_obj->range_5g_high = range_5g_high;
	pdev_priv_obj->wireless_modes = reg_cap_ptr->wireless_modes;

	reg_init_pdev_mas_chan_list(pdev_priv_obj,
				&psoc_priv_obj->mas_chan_params[pdev_id]);

	reg_compute_pdev_current_chan_list(pdev_priv_obj);

	status = wlan_objmgr_pdev_component_obj_attach(pdev,
						     WLAN_UMAC_COMP_REGULATORY,
						     pdev_priv_obj,
						     QDF_STATUS_SUCCESS);

	reg_debug("reg pdev obj created with status %d", status);

	return status;
}

QDF_STATUS wlan_regulatory_pdev_obj_destroyed_notification(
		struct wlan_objmgr_pdev *pdev, void *arg_list)
{
	QDF_STATUS status;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev_priv_obj->pdev_ptr = NULL;

	status = wlan_objmgr_pdev_component_obj_detach(pdev,
			WLAN_UMAC_COMP_REGULATORY,
			pdev_priv_obj);

	if (status != QDF_STATUS_SUCCESS)
		reg_err("reg pdev private obj detach failed");

	reg_debug("reg pdev obj deleted with status %d", status);

	qdf_mem_free(pdev_priv_obj);

	return status;
}

/**
 * reg_11d_vdev_created_update() - vdev obj create callback
 * @vdev: vdev pointer
 *
 * updates 11d state when a vdev is created.
 *
 * Return: Success or Failure
 */
QDF_STATUS reg_11d_vdev_created_update(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_objmgr_pdev *parent_pdev;
	struct wlan_objmgr_psoc *parent_psoc;
	uint32_t vdev_id;
	enum QDF_OPMODE op_mode;
	uint8_t i;

	op_mode = wlan_vdev_mlme_get_opmode(vdev);

	parent_pdev = wlan_vdev_get_pdev(vdev);
	parent_psoc = wlan_pdev_get_psoc(parent_pdev);

	psoc_priv_obj = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(parent_psoc,
					     WLAN_UMAC_COMP_REGULATORY);

	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	if ((op_mode == QDF_STA_MODE) ||
	    (op_mode == QDF_P2P_DEVICE_MODE) ||
	    (op_mode == QDF_P2P_CLIENT_MODE)) {
		vdev_id = wlan_vdev_get_id(vdev);
		if (!psoc_priv_obj->vdev_cnt_11d) {
			psoc_priv_obj->vdev_id_for_11d_scan = vdev_id;
			reg_debug("running 11d state machine, opmode %d",
					op_mode);
			reg_run_11d_state_machine(parent_psoc);
		}

		for (i = 0; i < MAX_STA_VDEV_CNT; i++) {
			if (psoc_priv_obj->vdev_ids_11d[i] ==
					INVALID_VDEV_ID) {
				psoc_priv_obj->vdev_ids_11d[i] = vdev_id;
				break;
			}
		}
		psoc_priv_obj->vdev_cnt_11d++;
	}

	if ((op_mode == QDF_P2P_GO_MODE) ||
	    (op_mode == QDF_SAP_MODE)) {
		reg_debug("running 11d state machine, opmode %d", op_mode);
		psoc_priv_obj->master_vdev_cnt++;
		reg_run_11d_state_machine(parent_psoc);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_11d_vdev_delete_update() - update 11d state upon vdev delete
 * @vdev: vdev pointer
 *
 * Return: Success or Failure
 */
QDF_STATUS reg_11d_vdev_delete_update(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_objmgr_pdev *parent_pdev;
	struct wlan_objmgr_psoc *parent_psoc;
	enum QDF_OPMODE op_mode;
	uint32_t vdev_id;
	uint8_t i;

	if (!vdev) {
		reg_err("vdev is NULL");
		return QDF_STATUS_E_INVAL;
	}
	op_mode = wlan_vdev_mlme_get_opmode(vdev);

	parent_pdev = wlan_vdev_get_pdev(vdev);
	parent_psoc = wlan_pdev_get_psoc(parent_pdev);

	psoc_priv_obj = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(parent_psoc,
					     WLAN_UMAC_COMP_REGULATORY);

	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	if ((op_mode == QDF_P2P_GO_MODE) ||
	    (op_mode == QDF_SAP_MODE)) {
		psoc_priv_obj->master_vdev_cnt--;
		reg_debug("run 11d state machine, deleted opmode %d",
				op_mode);
		reg_run_11d_state_machine(parent_psoc);
		return QDF_STATUS_SUCCESS;
	}

	if ((op_mode == QDF_STA_MODE) ||
	    (op_mode == QDF_P2P_DEVICE_MODE) ||
	    (op_mode == QDF_P2P_CLIENT_MODE)) {
		vdev_id = wlan_vdev_get_id(vdev);
		for (i = 0; i < MAX_STA_VDEV_CNT; i++) {
			if (psoc_priv_obj->vdev_ids_11d[i] == vdev_id) {
				psoc_priv_obj->vdev_ids_11d[i] =
					INVALID_VDEV_ID;
				psoc_priv_obj->vdev_cnt_11d--;
				break;
			}
		}

		if ((psoc_priv_obj->vdev_id_for_11d_scan != vdev_id) ||
				!psoc_priv_obj->vdev_cnt_11d)
			return QDF_STATUS_SUCCESS;

		for (i = 0; i < MAX_STA_VDEV_CNT; i++) {
			if (psoc_priv_obj->vdev_ids_11d[i] ==
			    INVALID_VDEV_ID)
				continue;
			psoc_priv_obj->vdev_id_for_11d_scan =
				psoc_priv_obj->vdev_ids_11d[i];
			psoc_priv_obj->enable_11d_supp = false;
			reg_debug("running 11d state machine, vdev %d",
				  psoc_priv_obj->vdev_id_for_11d_scan);
			reg_run_11d_state_machine(parent_psoc);
			break;
		}
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_get_current_chan_list(struct wlan_objmgr_pdev *pdev,
				     struct regulatory_channel *chan_list)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(chan_list, pdev_priv_obj->cur_chan_list,
		     NUM_CHANNELS * sizeof(struct regulatory_channel));

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_update_nol_ch () - Updates NOL channels in current channel list
 * @pdev: pointer to pdev object
 * @ch_list: pointer to NOL channel list
 * @num_ch: No.of channels in list
 * @update_nol: set/reset the NOL status
 *
 * Return: None
 */
void reg_update_nol_ch(struct wlan_objmgr_pdev *pdev,
		       uint8_t *chan_list,
		       uint8_t num_chan,
		       bool nol_chan)
{
	enum channel_enum chan_enum;
	struct regulatory_channel *mas_chan_list;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	uint16_t i;

	if (!num_chan || !chan_list) {
		reg_err("chan_list or num_ch is NULL");
		return;
	}

	pdev_priv_obj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						 WLAN_UMAC_COMP_REGULATORY);

	if (NULL == pdev_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	mas_chan_list = pdev_priv_obj->mas_chan_list;
	for (i = 0; i < num_chan; i++) {
		chan_enum = reg_get_chan_enum(chan_list[i]);
		if (chan_enum == INVALID_CHANNEL) {
			reg_err("Invalid ch in nol list, chan %d",
				chan_list[i]);
			continue;
		}
		mas_chan_list[chan_enum].nol_chan = nol_chan;
	}

	reg_compute_pdev_current_chan_list(pdev_priv_obj);
}

static void reg_change_pdev_for_config(struct wlan_objmgr_psoc *psoc,
				       void *object, void *arg)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)object;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	psoc_priv_obj = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
					     WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("psoc priv obj is NULL");
		return;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev private obj is NULL");
		return;
	}

	pdev_priv_obj->dfs_enabled =
		psoc_priv_obj->dfs_enabled;
	pdev_priv_obj->indoor_chan_enabled =
		psoc_priv_obj->indoor_chan_enabled;
	pdev_priv_obj->force_ssc_disable_indoor_channel =
		psoc_priv_obj->force_ssc_disable_indoor_channel;
	pdev_priv_obj->band_capability = psoc_priv_obj->band_capability;

	reg_compute_pdev_current_chan_list(pdev_priv_obj);

	reg_send_scheduler_msg_sb(psoc, pdev);
}

QDF_STATUS reg_set_config_vars(struct wlan_objmgr_psoc *psoc,
			    struct reg_config_vars config_vars)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	QDF_STATUS status;

	psoc_priv_obj = (struct wlan_regulatory_psoc_priv_obj *)
		wlan_objmgr_psoc_get_comp_private_obj(psoc,
				       WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("psoc priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	psoc_priv_obj->enable_11d_supp_original =
		config_vars.enable_11d_support;
	psoc_priv_obj->scan_11d_interval =
		config_vars.scan_11d_interval;
	psoc_priv_obj->user_ctry_priority =
		config_vars.userspace_ctry_priority;
	psoc_priv_obj->dfs_enabled =
		config_vars.dfs_enabled;
	psoc_priv_obj->indoor_chan_enabled =
		config_vars.indoor_chan_enabled;
	psoc_priv_obj->force_ssc_disable_indoor_channel =
		config_vars.force_ssc_disable_indoor_channel;
	psoc_priv_obj->band_capability = config_vars.band_capability;
	psoc_priv_obj->restart_beaconing = config_vars.restart_beaconing;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}
	status = wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
					      reg_change_pdev_for_config,
					      NULL, 1, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);

	return status;
}

bool reg_is_disable_ch(struct wlan_objmgr_pdev *pdev, uint32_t chan)
{
	enum channel_state ch_state;

	ch_state = reg_get_channel_state(pdev, chan);

	return ch_state == CHANNEL_STATE_DISABLE;
}

bool reg_is_regdb_offloaded(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return false;
	}

	return psoc_priv_obj->offload_enabled;
}

void reg_program_mas_chan_list(struct wlan_objmgr_psoc *psoc,
			       struct regulatory_channel *reg_channels,
			       uint8_t *alpha2,
			       enum dfs_reg dfs_region)
{

	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	QDF_STATUS status;
	uint32_t count;
	enum direction dir;
	uint32_t pdev_cnt;

	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	qdf_mem_copy(psoc_priv_obj->cur_country, alpha2,
		     REG_ALPHA2_LEN);

	for (count = 0; count < NUM_CHANNELS; count++) {
		reg_channels[count].chan_num =
			channel_map[count].chan_num;
		reg_channels[count].center_freq =
			channel_map[count].center_freq;
		reg_channels[count].nol_chan = false;
	}

	for (pdev_cnt = 0; pdev_cnt < PSOC_MAX_PHY_REG_CAP; pdev_cnt++) {
		qdf_mem_copy(psoc_priv_obj->mas_chan_params[pdev_cnt].
			     mas_chan_list,
			     reg_channels,
			     NUM_CHANNELS * sizeof(struct regulatory_channel));

		psoc_priv_obj->mas_chan_params[pdev_cnt].dfs_region =
			dfs_region;
	}

	dir = SOUTHBOUND;
	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return;
	}
	status = wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
					  reg_propagate_mas_chan_list_to_pdev,
					  &dir, 1, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
}

void reg_register_chan_change_callback(struct wlan_objmgr_psoc *psoc,
				       reg_chan_change_callback cbk,
				       void *arg)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	uint32_t count;


	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	qdf_spin_lock_bh(&psoc_priv_obj->cbk_list_lock);
	for (count = 0; count < REG_MAX_CHAN_CHANGE_CBKS; count++)
		if (psoc_priv_obj->cbk_list[count].cbk == NULL) {
			psoc_priv_obj->cbk_list[count].cbk = cbk;
			psoc_priv_obj->cbk_list[count].arg = arg;
			psoc_priv_obj->num_chan_change_cbks++;
			break;
		}
	qdf_spin_unlock_bh(&psoc_priv_obj->cbk_list_lock);

	if (count == REG_MAX_CHAN_CHANGE_CBKS)
		reg_err("callback list is full, could not add the cbk");
}

void reg_unregister_chan_change_callback(struct wlan_objmgr_psoc *psoc,
				       reg_chan_change_callback cbk)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	uint32_t count;

	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	qdf_spin_lock_bh(&psoc_priv_obj->cbk_list_lock);
	for (count = 0; count < REG_MAX_CHAN_CHANGE_CBKS; count++)
		if (psoc_priv_obj->cbk_list[count].cbk == cbk) {
			psoc_priv_obj->cbk_list[count].cbk = NULL;
			psoc_priv_obj->num_chan_change_cbks--;
			break;
		}
	qdf_spin_unlock_bh(&psoc_priv_obj->cbk_list_lock);

	if (count == REG_MAX_CHAN_CHANGE_CBKS)
		reg_err("callback not found in the list");
}

enum country_src reg_get_cc_and_src(struct wlan_objmgr_psoc *psoc,
				    uint8_t *alpha2)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return SOURCE_UNKNOWN;
	}

	qdf_mem_copy(alpha2, psoc_priv_obj->cur_country,
		     REG_ALPHA2_LEN + 1);

	return psoc_priv_obj->cc_src;
}

QDF_STATUS reg_program_default_cc(struct wlan_objmgr_pdev *pdev,
				  uint16_t regdmn)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct cur_regulatory_info *reg_info;
	uint16_t cc = -1;
	uint16_t country_index = -1, regdmn_pair = -1;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS err;

	pdev_priv_obj = (struct wlan_regulatory_pdev_priv_obj *)
		wlan_objmgr_pdev_get_comp_private_obj(pdev,
					   WLAN_UMAC_COMP_REGULATORY);

	if (NULL == pdev_priv_obj) {
		reg_err("reg soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	reg_info = (struct cur_regulatory_info *)qdf_mem_malloc
		(sizeof(struct cur_regulatory_info));
	if (reg_info == NULL) {
		reg_err("reg info is NULL");
		return QDF_STATUS_E_NOMEM;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info->psoc = psoc;
	reg_info->phy_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	if (regdmn == 0) {
		reg_get_default_country(&regdmn);
		regdmn |= COUNTRY_ERD_FLAG;
	}

	if (regdmn & COUNTRY_ERD_FLAG) {
		cc = regdmn & ~COUNTRY_ERD_FLAG;

		reg_get_rdpair_from_country_code(cc,
						 &country_index,
						 &regdmn_pair);

		err = reg_get_cur_reginfo(reg_info, country_index, regdmn_pair);
		if (err == QDF_STATUS_E_FAILURE) {
			reg_err("%s : Unable to set country code\n", __func__);
			qdf_mem_free(reg_info->reg_rules_2g_ptr);
			qdf_mem_free(reg_info->reg_rules_5g_ptr);
			qdf_mem_free(reg_info);
			return QDF_STATUS_E_FAILURE;
		}

		pdev_priv_obj->ctry_code = cc;

	} else {
		reg_get_rdpair_from_regdmn_id(regdmn,
				&regdmn_pair);

		err = reg_get_cur_reginfo(reg_info, country_index, regdmn_pair);
		if (err == QDF_STATUS_E_FAILURE) {
			reg_err("%s : Unable to set country code\n", __func__);
			qdf_mem_free(reg_info->reg_rules_2g_ptr);
			qdf_mem_free(reg_info->reg_rules_5g_ptr);
			qdf_mem_free(reg_info);
			return QDF_STATUS_E_FAILURE;
		}

		pdev_priv_obj->reg_dmn_pair = regdmn;
	}

	reg_info->offload_enabled = false;
	reg_process_master_chan_list(reg_info);

	qdf_mem_free(reg_info->reg_rules_2g_ptr);
	qdf_mem_free(reg_info->reg_rules_5g_ptr);
	qdf_mem_free(reg_info);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_program_chan_list(struct wlan_objmgr_pdev *pdev,
				 struct cc_regdmn_s *rd)
{
	struct cur_regulatory_info *reg_info;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	uint16_t country_index = -1, regdmn_pair = -1;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;
	struct wlan_regulatory_psoc_priv_obj *soc_reg;
	uint8_t pdev_id;
	QDF_STATUS err;

	pdev_priv_obj = (struct wlan_regulatory_pdev_priv_obj *)
		wlan_objmgr_pdev_get_comp_private_obj(pdev,
					WLAN_UMAC_COMP_REGULATORY);

	if (NULL == pdev_priv_obj) {
		reg_err(" pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	soc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (soc_reg->offload_enabled) {
		if ((rd->flags == ALPHA_IS_SET) && (rd->cc.alpha[2] == 'O'))
			pdev_priv_obj->indoor_chan_enabled = false;
		else
			pdev_priv_obj->indoor_chan_enabled = true;

		pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
		tx_ops = reg_get_psoc_tx_ops(psoc);
		if (tx_ops->set_user_country_code)
			return tx_ops->set_user_country_code(psoc, pdev_id, rd);

		return QDF_STATUS_E_FAILURE;
	}

	reg_info = (struct cur_regulatory_info *)qdf_mem_malloc
		(sizeof(struct cur_regulatory_info));
	if (reg_info == NULL) {
		reg_err("reg info is NULL");
		return QDF_STATUS_E_NOMEM;
	}

	reg_info->psoc = psoc;
	reg_info->phy_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	if (rd->flags == CC_IS_SET) {
		reg_get_rdpair_from_country_code(rd->cc.country_code,
						 &country_index,
						 &regdmn_pair);
	} else if (rd->flags == ALPHA_IS_SET) {
		reg_get_rdpair_from_country_iso(rd->cc.alpha,
						&country_index,
						&regdmn_pair);
	} else if (rd->flags == REGDMN_IS_SET) {
		reg_get_rdpair_from_regdmn_id(rd->cc.regdmn_id,
					      &regdmn_pair);
	}

	err = reg_get_cur_reginfo(reg_info, country_index, regdmn_pair);
	if (err == QDF_STATUS_E_FAILURE) {
		reg_err("%s : Unable to set country code\n", __func__);
		qdf_mem_free(reg_info->reg_rules_2g_ptr);
		qdf_mem_free(reg_info->reg_rules_5g_ptr);
		qdf_mem_free(reg_info);
		return QDF_STATUS_E_FAILURE;
	}

	reg_info->offload_enabled = false;
	reg_process_master_chan_list(reg_info);

	qdf_mem_free(reg_info->reg_rules_2g_ptr);
	qdf_mem_free(reg_info->reg_rules_5g_ptr);
	qdf_mem_free(reg_info);

	return QDF_STATUS_SUCCESS;
}

bool reg_is_11d_scan_inprogress(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_REGULATORY);

	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return false;
	}

	return psoc_priv_obj->enable_11d_supp;
}

QDF_STATUS reg_get_current_cc(struct wlan_objmgr_pdev *pdev,
			      struct cc_regdmn_s *rd)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = (struct wlan_regulatory_pdev_priv_obj *)
		wlan_objmgr_pdev_get_comp_private_obj(pdev,
				     WLAN_UMAC_COMP_REGULATORY);

	if (NULL == pdev_priv_obj) {
		reg_err("reg pdev priv is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (rd->flags == CC_IS_SET) {
		rd->cc.country_code = pdev_priv_obj->ctry_code;
	} else if (rd->flags == ALPHA_IS_SET) {
		qdf_mem_copy(rd->cc.alpha, pdev_priv_obj->current_country,
			     sizeof(rd->cc.alpha));
	} else if (rd->flags == REGDMN_IS_SET) {
		rd->cc.regdmn_id = pdev_priv_obj->reg_dmn_pair;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS reg_process_ch_avoid_freq(struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_pdev *pdev)
{
	enum channel_enum ch_loop;
	enum channel_enum start_ch_idx;
	enum channel_enum end_ch_idx;
	uint16_t start_channel;
	uint16_t end_channel;
	uint32_t i;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct ch_avoid_freq_type *range;

	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
		WLAN_UMAC_COMP_REGULATORY);

	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < psoc_priv_obj->avoid_freq_list.ch_avoid_range_cnt;
		i++) {
		if (psoc_priv_obj->unsafe_chan_list.ch_cnt >= NUM_CHANNELS) {
			reg_warn("LTE Coex unsafe channel list full");
			break;
		}

		start_ch_idx = INVALID_CHANNEL;
		end_ch_idx = INVALID_CHANNEL;
		range = &psoc_priv_obj->avoid_freq_list.avoid_freq_range[i];

		start_channel = reg_freq_to_chan(pdev, range->start_freq);
		end_channel = reg_freq_to_chan(pdev, range->end_freq);
		reg_debug("start: freq %d, ch %d, end: freq %d, ch %d",
			range->start_freq, start_channel, range->end_freq,
			end_channel);

		/* do not process frequency bands that are not mapped to
		 * predefined channels
		 */
		if (start_channel == 0 || end_channel == 0)
			continue;

		for (ch_loop = 0; ch_loop < NUM_CHANNELS;
			ch_loop++) {
			if (REG_CH_TO_FREQ(ch_loop) >= range->start_freq) {
				start_ch_idx = ch_loop;
				break;
			}
		}
		for (ch_loop = 0; ch_loop < NUM_CHANNELS;
			ch_loop++) {
			if (REG_CH_TO_FREQ(ch_loop) >= range->end_freq) {
				end_ch_idx = ch_loop;
				if (REG_CH_TO_FREQ(ch_loop) > range->end_freq)
					end_ch_idx--;
				break;
			}
		}

		if (start_ch_idx == INVALID_CHANNEL ||
				end_ch_idx == INVALID_CHANNEL)
			continue;

		for (ch_loop = start_ch_idx; ch_loop <=	end_ch_idx;
			ch_loop++) {
			psoc_priv_obj->unsafe_chan_list.ch_list[
				psoc_priv_obj->unsafe_chan_list.ch_cnt++] =
				REG_CH_NUM(ch_loop);
			if (psoc_priv_obj->unsafe_chan_list.ch_cnt >=
				NUM_CHANNELS) {
				reg_warn("LTECoex unsafe ch list full");
				break;
			}
		}
	}

	reg_debug("number of unsafe channels is %d ",
		psoc_priv_obj->unsafe_chan_list.ch_cnt);

	if (!psoc_priv_obj->unsafe_chan_list.ch_cnt) {
		reg_warn("No valid ch are present in avoid freq event");
		psoc_priv_obj->ch_avoid_ind = false;
		return QDF_STATUS_SUCCESS;
	}

	for (ch_loop = 0; ch_loop < psoc_priv_obj->unsafe_chan_list.ch_cnt;
		ch_loop++) {
		if (ch_loop >= NUM_CHANNELS)
			break;
		reg_debug("channel %d is not safe",
			psoc_priv_obj->unsafe_chan_list.
			ch_list[ch_loop]);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_update_unsafe_ch () - Updates unsafe channels in current channel list
 * @pdev: pointer to pdev object
 * @ch_avoid_list: pointer to unsafe channel list
 *
 * Return: None
 */
static void reg_update_unsafe_ch(struct wlan_objmgr_psoc *psoc,
		void *object, void *arg)
{
	struct wlan_objmgr_pdev *pdev = (struct wlan_objmgr_pdev *)object;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	QDF_STATUS status;

	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						  WLAN_UMAC_COMP_REGULATORY);

	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("reg pdev priv obj is NULL");
		return;
	}

	if (psoc_priv_obj->ch_avoid_ind) {
		status = reg_process_ch_avoid_freq(psoc, pdev);
		if (QDF_IS_STATUS_ERROR(status))
			psoc_priv_obj->ch_avoid_ind = false;
	}

	reg_compute_pdev_current_chan_list(pdev_priv_obj);
	status = reg_send_scheduler_msg_nb(psoc, pdev);

	if (QDF_IS_STATUS_ERROR(status))
		reg_err("channel change msg schedule failed");

}

QDF_STATUS reg_process_ch_avoid_event(struct wlan_objmgr_psoc *psoc,
		struct ch_avoid_ind_type *ch_avoid_event)
{
	uint32_t i;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	QDF_STATUS status;

	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
			WLAN_UMAC_COMP_REGULATORY);
	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	/* Make unsafe channel list */
	reg_debug("band count %d", ch_avoid_event->ch_avoid_range_cnt);

	/* generate vendor specific event */
	qdf_mem_zero((void *)&psoc_priv_obj->avoid_freq_list,
			sizeof(struct ch_avoid_ind_type));
	qdf_mem_zero((void *)&psoc_priv_obj->unsafe_chan_list,
			sizeof(struct unsafe_ch_list));

	for (i = 0; i < ch_avoid_event->ch_avoid_range_cnt; i++) {
		if ((CH_AVOID_RULE_RESTART_24G_ONLY ==
				psoc_priv_obj->restart_beaconing) &&
			REG_IS_5GHZ_FREQ(ch_avoid_event->
				avoid_freq_range[i].start_freq)) {
			reg_debug("skipping 5Ghz LTE Coex unsafe channel range");
			continue;
		}
		psoc_priv_obj->avoid_freq_list.avoid_freq_range[i].start_freq =
			ch_avoid_event->avoid_freq_range[i].start_freq;
		psoc_priv_obj->avoid_freq_list.avoid_freq_range[i].end_freq =
			ch_avoid_event->avoid_freq_range[i].end_freq;
	}
	psoc_priv_obj->avoid_freq_list.ch_avoid_range_cnt =
		ch_avoid_event->ch_avoid_range_cnt;

	psoc_priv_obj->ch_avoid_ind = true;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_NB_ID);

	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}

	status = wlan_objmgr_iterate_obj_list(psoc, WLAN_PDEV_OP,
			reg_update_unsafe_ch, NULL, 1,
			WLAN_REGULATORY_NB_ID);

	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);

	return status;
}

QDF_STATUS reg_save_new_11d_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					     WLAN_UMAC_COMP_REGULATORY);

	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");

		return QDF_STATUS_E_FAILURE;
	}

	psoc_priv_obj->new_11d_ctry_pending = true;

	return QDF_STATUS_SUCCESS;
}

bool reg_11d_original_enabled_on_host(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj =
	     wlan_objmgr_psoc_get_comp_private_obj(psoc,
						   WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return (psoc_priv_obj->enable_11d_supp_original &&
		!psoc_priv_obj->is_11d_offloaded);
}

bool reg_11d_enabled_on_host(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_REGULATORY);

	if (NULL == psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	return (psoc_priv_obj->enable_11d_supp &&
		!psoc_priv_obj->is_11d_offloaded);
}

QDF_STATUS reg_set_regdb_offloaded(struct wlan_objmgr_psoc *psoc,
		bool val)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	soc_reg = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	soc_reg->offload_enabled = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_set_11d_offloaded(struct wlan_objmgr_psoc *psoc,
		bool val)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	soc_reg = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	soc_reg->is_11d_offloaded = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_get_curr_regdomain(struct wlan_objmgr_pdev *pdev,
		struct cur_regdmn_info *cur_regdmn)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_regulatory_psoc_priv_obj *soc_reg;
	uint16_t index;
	int num_reg_dmn;
	uint8_t phy_id;

	psoc = wlan_pdev_get_psoc(pdev);
	soc_reg = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("soc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	phy_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	cur_regdmn->regdmn_pair_id =
		soc_reg->mas_chan_params[phy_id].reg_dmn_pair;

	reg_get_num_reg_dmn_pairs(&num_reg_dmn);
	for (index = 0; index < num_reg_dmn; index++) {
		if (g_reg_dmn_pairs[index].reg_dmn_pair_id ==
				cur_regdmn->regdmn_pair_id)
			break;
	}

	if (index == num_reg_dmn) {
		reg_err("invalid regdomain");
		return QDF_STATUS_E_FAILURE;
	}

	cur_regdmn->dmn_id_2g = g_reg_dmn_pairs[index].dmn_id_2g;
	cur_regdmn->dmn_id_5g = g_reg_dmn_pairs[index].dmn_id_5g;
	cur_regdmn->ctl_2g = regdomains_2g[cur_regdmn->dmn_id_2g].ctl_val;
	cur_regdmn->ctl_5g = regdomains_5g[cur_regdmn->dmn_id_5g].ctl_val;
	cur_regdmn->dfs_region =
		regdomains_5g[cur_regdmn->dmn_id_5g].dfs_region;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_modify_chan_144(struct wlan_objmgr_pdev *pdev,
				    bool enable_ch_144)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_reg_tx_ops *reg_tx_ops;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (pdev_priv_obj->en_chan_144 == enable_ch_144) {
		reg_info("chan 144 is already  %d", enable_ch_144);
		return QDF_STATUS_SUCCESS;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_debug("setting chan 144: %d", enable_ch_144);
	pdev_priv_obj->en_chan_144 = enable_ch_144;

	reg_compute_pdev_current_chan_list(pdev_priv_obj);

	reg_tx_ops = reg_get_psoc_tx_ops(psoc);
	if (reg_tx_ops->fill_umac_legacy_chanlist)
		reg_tx_ops->fill_umac_legacy_chanlist(pdev,
				pdev_priv_obj->cur_chan_list);

	status = reg_send_scheduler_msg_sb(psoc, pdev);

	return status;

}

bool reg_get_en_chan_144(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return false;
	}

	return pdev_priv_obj->en_chan_144;
}

struct wlan_psoc_host_hal_reg_capabilities_ext *reg_get_hal_reg_cap(
						struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	soc_reg = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("psoc reg component is NULL");
		return NULL;
	}

	return soc_reg->reg_cap;
}

QDF_STATUS reg_set_hal_reg_cap(struct wlan_objmgr_psoc *psoc,
		struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap,
		uint16_t phy_cnt)
{
	struct wlan_regulatory_psoc_priv_obj *soc_reg;

	soc_reg = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(soc_reg)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (phy_cnt > PSOC_MAX_PHY_REG_CAP) {
		reg_err("phy cnt:%d is more than %d", phy_cnt,
						PSOC_MAX_PHY_REG_CAP);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(soc_reg->reg_cap, reg_cap,
		phy_cnt *
			sizeof(struct wlan_psoc_host_hal_reg_capabilities_ext));

	return QDF_STATUS_SUCCESS;
}

