/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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
 * DOC: reg_services_common.c
 * This file defines regulatory component service functions
 */

#include <wlan_cmn.h>
#include <reg_services_public_struct.h>
#include <wlan_objmgr_psoc_obj.h>
#include <qdf_lock.h>
#include "reg_priv_objs.h"
#include "reg_utils.h"
#include "reg_callbacks.h"
#include "reg_services_common.h"
#include <wlan_objmgr_psoc_obj.h>
#include "reg_db.h"
#include "reg_db_parser.h"
#include "reg_build_chan_list.h"
#include <wlan_objmgr_pdev_obj.h>

const struct chan_map *channel_map;

static const struct bonded_channel bonded_chan_40mhz_list[] = {
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
	{157, 161},
	{165, 169}
};

static const struct bonded_channel bonded_chan_80mhz_list[] = {
	{36, 48},
	{52, 64},
	{100, 112},
	{116, 128},
	{132, 144},
	{149, 161}
};

static const struct bonded_channel bonded_chan_160mhz_list[] = {
	{36, 64},
	{100, 128}
};

static const enum phy_ch_width get_next_lower_bw[] = {
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
#ifndef WLAN_FEATURE_DSRC
	[CHAN_ENUM_169] = {5845, 169, 2, 40},
	[CHAN_ENUM_173] = {5865, 173, 2, 20},
#else
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
#endif
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

#ifdef CONFIG_LEGACY_CHAN_ENUM
void reg_init_channel_map(enum dfs_reg dfs_region)
{
	channel_map = channel_map_old;
}
#else
void reg_init_channel_map(enum dfs_reg dfs_region)
{
	switch (dfs_region) {
	case DFS_UNINIT_REGION:
	case DFS_UNDEF_REGION:
		channel_map = channel_map_global;
		break;
	case DFS_FCC_REGION:
		channel_map = channel_map_us;
		break;
	case DFS_ETSI_REGION:
		channel_map = channel_map_eu;
		break;
	case DFS_MKK_REGION:
		channel_map = channel_map_jp;
		break;
	case DFS_CN_REGION:
		channel_map = channel_map_china;
		break;
	case DFS_KR_REGION:
		channel_map = channel_map_eu;
		break;
	}
}
#endif

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

struct wlan_lmac_if_reg_tx_ops *reg_get_psoc_tx_ops(
		struct wlan_objmgr_psoc *psoc)
{
	return &((psoc->soc_cb.tx_ops.reg_ops));
}

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
		    reg_channels[i].chan_flags != REGULATORY_CHAN_DISABLED) {
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

enum channel_state reg_get_channel_state(struct wlan_objmgr_pdev *pdev,
					 uint32_t ch)
{
	enum channel_enum ch_idx;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	ch_idx = reg_get_chan_enum(ch);

	if (ch_idx == INVALID_CHANNEL)
		return CHANNEL_STATE_INVALID;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return CHANNEL_STATE_INVALID;
	}

	return pdev_priv_obj->cur_chan_list[ch_idx].state;
}

/**
 * reg_get_5g_bonded_chan_array() - get ptr to bonded channel
 * @pdev: Pointer to pdev structure
 * @oper_ch: operating channel number
 * @bonded_chan_ar: bonded channel array
 * @array_size; Array size
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
			bonded_chan_ptr = &bonded_chan_ar[i];
			break;
		}
	}

	if (!bonded_chan_ptr)
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
 * @pdev: Pointer to pdev structure
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
	if (ch_width == CH_WIDTH_80P80MHZ)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_80mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_80mhz_list),
				bonded_chan_ptr_ptr);
	else if (ch_width == CH_WIDTH_160MHZ)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_160mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_160mhz_list),
				bonded_chan_ptr_ptr);
	else if (ch_width == CH_WIDTH_80MHZ)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_80mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_80mhz_list),
				bonded_chan_ptr_ptr);
	else if (ch_width == CH_WIDTH_40MHZ)
		return reg_get_5g_bonded_chan_array(pdev, chan_num,
				bonded_chan_40mhz_list,
				QDF_ARRAY_SIZE(bonded_chan_40mhz_list),
				bonded_chan_ptr_ptr);
	else
		return reg_get_channel_state(pdev, chan_num);
}

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

	if (bw > CH_WIDTH_80P80MHZ) {
		reg_err("bw passed is not good");
		return CHANNEL_STATE_INVALID;
	}

	chan_state = reg_get_5g_bonded_channel(pdev, ch, bw, &bonded_chan_ptr);

	if ((chan_state == CHANNEL_STATE_INVALID) ||
	    (chan_state == CHANNEL_STATE_DISABLE))
		return chan_state;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg obj is NULL");
		return CHANNEL_STATE_INVALID;
	}
	reg_channels = pdev_priv_obj->cur_chan_list;

	ch_indx = reg_get_chan_enum(ch);
	if (ch_indx == INVALID_CHANNEL)
		return CHANNEL_STATE_INVALID;
	if (bw == CH_WIDTH_5MHZ)
		bw_enabled = true;
	else if (bw == CH_WIDTH_10MHZ)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 10) &&
			(reg_channels[ch_indx].max_bw >= 10);
	else if (bw == CH_WIDTH_20MHZ)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 20) &&
			(reg_channels[ch_indx].max_bw >= 20);
	else if (bw == CH_WIDTH_40MHZ)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 40) &&
			(reg_channels[ch_indx].max_bw >= 40);
	else if (bw == CH_WIDTH_80MHZ)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 80) &&
			(reg_channels[ch_indx].max_bw >= 80);
	else if (bw == CH_WIDTH_160MHZ)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 160) &&
			(reg_channels[ch_indx].max_bw >= 160);
	else if (bw == CH_WIDTH_80P80MHZ)
		bw_enabled = (reg_channels[ch_indx].min_bw <= 80) &&
			(reg_channels[ch_indx].max_bw >= 80);

	if (bw_enabled)
		return chan_state;
	else
		return CHANNEL_STATE_DISABLE;
}

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

	if (bw > CH_WIDTH_40MHZ)
		return CHANNEL_STATE_INVALID;

	if (bw == CH_WIDTH_40MHZ) {
		if ((sec_ch + 4 != oper_ch) &&
		    (oper_ch + 4 != sec_ch))
			return CHANNEL_STATE_INVALID;
		chan_state2 = reg_get_channel_state(pdev, sec_ch);
		if (chan_state2 == CHANNEL_STATE_INVALID)
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

	if ((chan_state == CHANNEL_STATE_INVALID) ||
	    (chan_state == CHANNEL_STATE_DISABLE))
		return chan_state;

	chan_idx = reg_get_chan_enum(oper_ch);
	if (chan_idx == INVALID_CHANNEL)
		return CHANNEL_STATE_INVALID;
	if (bw == CH_WIDTH_5MHZ)
		bw_enabled = true;
	else if (bw == CH_WIDTH_10MHZ)
		bw_enabled = (reg_channels[chan_idx].min_bw <= 10) &&
			(reg_channels[chan_idx].max_bw >= 10);
	else if (bw == CH_WIDTH_20MHZ)
		bw_enabled = (reg_channels[chan_idx].min_bw <= 20) &&
			(reg_channels[chan_idx].max_bw >= 20);
	else if (bw == CH_WIDTH_40MHZ)
		bw_enabled = (reg_channels[chan_idx].min_bw <= 40) &&
			(reg_channels[chan_idx].max_bw >= 40);

	if (bw_enabled)
		return chan_state;
	else
		return CHANNEL_STATE_DISABLE;

	return CHANNEL_STATE_ENABLE;
}

/**
 * reg_combine_channel_states() - Get minimum of channel state1 and state2
 * @chan_state1: Channel state1
 * @chan_state2: Channel state2
 *
 * Return: Channel state
 */
static enum channel_state reg_combine_channel_states(
	enum channel_state chan_state1,
	enum channel_state chan_state2)
{
	if ((chan_state1 == CHANNEL_STATE_INVALID) ||
	    (chan_state2 == CHANNEL_STATE_INVALID))
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

	if (!ch_params) {
		reg_err("ch_params is NULL");
		return;
	}

	if (ch_params->ch_width >= CH_WIDTH_MAX) {
		if (ch_params->center_freq_seg1 != 0)
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
		chan_state = reg_get_5g_bonded_channel(
				pdev, ch, ch_params->ch_width,
				&bonded_chan_ptr);

		chan_state = reg_get_5g_bonded_channel_state(
				pdev, ch, ch_params->ch_width);

		if (ch_params->ch_width == CH_WIDTH_80P80MHZ) {
			chan_state2 = reg_get_5g_bonded_channel_state(
					pdev, ch_params->center_freq_seg1 - 2,
					CH_WIDTH_80MHZ);

			chan_state = reg_combine_channel_states(
					chan_state, chan_state2);
		}

		if ((chan_state != CHANNEL_STATE_ENABLE) &&
		    (chan_state != CHANNEL_STATE_DFS))
			continue;
		if (ch_params->ch_width <= CH_WIDTH_20MHZ) {
			ch_params->sec_ch_offset = NO_SEC_CH;
			ch_params->center_freq_seg0 = ch;
			break;
		} else if (ch_params->ch_width >= CH_WIDTH_40MHZ) {
			reg_get_5g_bonded_chan_array(
					pdev, ch, bonded_chan_40mhz_list,
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
				 bonded_chan_ptr->end_ch) / 2;
			break;
		}
	}

	if (ch_params->ch_width == CH_WIDTH_160MHZ) {
		ch_params->center_freq_seg1 = ch_params->center_freq_seg0;
		chan_state = reg_get_5g_bonded_channel(
				pdev, ch, CH_WIDTH_80MHZ, &bonded_chan_ptr);
		if (bonded_chan_ptr)
			ch_params->center_freq_seg0 =
				(bonded_chan_ptr->start_ch +
				 bonded_chan_ptr->end_ch) / 2;
	}

	/* Overwrite center_freq_seg1 to 0 for non 160 and 80+80 width */
	if (!(ch_params->ch_width == CH_WIDTH_160MHZ ||
	      ch_params->ch_width == CH_WIDTH_80P80MHZ))
		ch_params->center_freq_seg1 = 0;

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
				      uint16_t oper_ch,
				      struct ch_params *ch_params,
				      uint16_t sec_ch_2g)
{
	enum channel_state chan_state = CHANNEL_STATE_ENABLE;

	if (ch_params->ch_width >= CH_WIDTH_MAX)
		ch_params->ch_width = CH_WIDTH_40MHZ;
	if ((reg_get_bw_value(ch_params->ch_width) > 20) && !sec_ch_2g) {
		if (oper_ch >= 1 && oper_ch <= 5)
			sec_ch_2g = oper_ch + 4;
		else if (oper_ch >= 6 && oper_ch <= 13)
			sec_ch_2g = oper_ch - 4;
	}

	while (ch_params->ch_width != CH_WIDTH_INVALID) {
		chan_state = reg_get_2g_bonded_channel_state(
				pdev, oper_ch, sec_ch_2g, ch_params->ch_width);
		if (chan_state == CHANNEL_STATE_ENABLE) {
			if (ch_params->ch_width == CH_WIDTH_40MHZ) {
				if (oper_ch < sec_ch_2g)
					ch_params->sec_ch_offset =
						LOW_PRIMARY_CH;
				else
					ch_params->sec_ch_offset =
						HIGH_PRIMARY_CH;
				ch_params->center_freq_seg0 =
					(oper_ch + sec_ch_2g) / 2;
			} else {
				ch_params->sec_ch_offset = NO_SEC_CH;
				ch_params->center_freq_seg0 = oper_ch;
			}
			break;
		}

		ch_params->ch_width = get_next_lower_bw[ch_params->ch_width];
	}
	/* Overwrite center_freq_seg1 to 0 for 2.4 Ghz */
	ch_params->center_freq_seg1 = 0;
}

void reg_set_channel_params(struct wlan_objmgr_pdev *pdev,
			    uint8_t ch, uint8_t sec_ch_2g,
			    struct ch_params *ch_params)
{
	if (REG_IS_5GHZ_CH(ch))
		reg_set_5g_channel_params(pdev, ch, ch_params);
	else if  (REG_IS_24GHZ_CH(ch))
		reg_set_2g_channel_params(pdev, ch, ch_params, sec_ch_2g);
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

QDF_STATUS reg_read_default_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country_code)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	if (!country_code) {
		reg_err("country_code is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	qdf_mem_copy(country_code, psoc_priv_obj->def_country,
		     REG_ALPHA2_LEN + 1);

	return QDF_STATUS_SUCCESS;
}

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
		if (chan_list[count].chan_num == chan_num) {
			if (reg_chan_in_range(chan_list,
					      pdev_priv_obj->range_2g_low,
					      pdev_priv_obj->range_2g_high,
					      pdev_priv_obj->range_5g_low,
					      pdev_priv_obj->range_5g_high,
					      count)) {
				return chan_list[count].center_freq;
			}
		}

	reg_debug_rl("invalid channel %d", chan_num);

	return 0;
}

#ifndef CONFIG_LEGACY_CHAN_ENUM
bool reg_chan_is_49ghz(struct wlan_objmgr_pdev *pdev, uint8_t chan_num)
{
	uint32_t freq = 0;

	freq = reg_chan_to_freq(pdev, chan_num);

	return REG_IS_49GHZ_FREQ(freq) ? true : false;
}
#else
bool reg_chan_is_49ghz(struct wlan_objmgr_pdev *pdev, uint8_t chan_num)
{
	return false;
}
#endif

enum band_info reg_chan_to_band(uint32_t chan_num)
{
	if (chan_num <= 14)
		return BAND_2G;

	return BAND_5G;
}

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

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!pdev_priv_obj) {
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

QDF_STATUS reg_program_default_cc(struct wlan_objmgr_pdev *pdev,
				  uint16_t regdmn)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct cur_regulatory_info *reg_info;
	uint16_t cc = -1;
	uint16_t country_index = -1, regdmn_pair = -1;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS err;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!pdev_priv_obj) {
		reg_err("reg soc is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	reg_info = (struct cur_regulatory_info *)qdf_mem_malloc
		(sizeof(struct cur_regulatory_info));
	if (!reg_info) {
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
		reg_get_rdpair_from_regdmn_id(regdmn, &regdmn_pair);

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
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	uint8_t pdev_id;
	QDF_STATUS err;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!pdev_priv_obj) {
		reg_err(" pdev priv obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (psoc_priv_obj->offload_enabled) {
		if ((rd->flags == ALPHA_IS_SET) && (rd->cc.alpha[2] == 'O'))
			pdev_priv_obj->indoor_chan_enabled = false;
		else
			pdev_priv_obj->indoor_chan_enabled = true;

		pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
		tx_ops = reg_get_psoc_tx_ops(psoc);
		if (tx_ops->set_user_country_code) {
			psoc_priv_obj->new_init_ctry_pending[pdev_id] = true;
			return tx_ops->set_user_country_code(psoc, pdev_id, rd);
		}

		return QDF_STATUS_E_FAILURE;
	}

	reg_info = (struct cur_regulatory_info *)qdf_mem_malloc
		(sizeof(struct cur_regulatory_info));
	if (!reg_info) {
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

QDF_STATUS reg_get_current_cc(struct wlan_objmgr_pdev *pdev,
			      struct cc_regdmn_s *rd)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!pdev_priv_obj) {
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

QDF_STATUS reg_set_regdb_offloaded(struct wlan_objmgr_psoc *psoc, bool val)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	psoc_priv_obj->offload_enabled = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_get_curr_regdomain(struct wlan_objmgr_pdev *pdev,
				  struct cur_regdmn_info *cur_regdmn)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	uint16_t index;
	int num_reg_dmn;
	uint8_t phy_id;

	psoc = wlan_pdev_get_psoc(pdev);
	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("soc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	phy_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	cur_regdmn->regdmn_pair_id =
		psoc_priv_obj->mas_chan_params[phy_id].reg_dmn_pair;

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
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return NULL;
	}

	return psoc_priv_obj->reg_cap;
}

QDF_STATUS reg_set_hal_reg_cap(
		struct wlan_objmgr_psoc *psoc,
		struct wlan_psoc_host_hal_reg_capabilities_ext *reg_cap,
		uint16_t phy_cnt)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = reg_get_psoc_obj(psoc);

	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (phy_cnt > PSOC_MAX_PHY_REG_CAP) {
		reg_err("phy cnt:%d is more than %d", phy_cnt,
			PSOC_MAX_PHY_REG_CAP);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_copy(psoc_priv_obj->reg_cap, reg_cap,
		     phy_cnt *
		     sizeof(struct wlan_psoc_host_hal_reg_capabilities_ext));

	return QDF_STATUS_SUCCESS;
}

bool reg_chan_in_range(struct regulatory_channel *chan_list,
		       uint32_t low_freq_2g, uint32_t high_freq_2g,
		       uint32_t low_freq_5g, uint32_t high_freq_5g,
		       enum channel_enum ch_enum)
{
	uint32_t low_limit_2g = NUM_CHANNELS;
	uint32_t high_limit_2g = NUM_CHANNELS;
	uint32_t low_limit_5g = NUM_CHANNELS;
	uint32_t high_limit_5g = NUM_CHANNELS;
	bool chan_in_range;
	enum channel_enum chan_enum;
	uint16_t min_bw;
	uint32_t center_freq;

	for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
		min_bw = chan_list[chan_enum].min_bw;
		center_freq = chan_list[chan_enum].center_freq;

		if ((center_freq - min_bw / 2) >= low_freq_2g) {
			low_limit_2g = chan_enum;
			break;
		}
	}

	for (chan_enum = 0; chan_enum < NUM_CHANNELS; chan_enum++) {
		min_bw = chan_list[chan_enum].min_bw;
		center_freq = chan_list[chan_enum].center_freq;

		if ((center_freq - min_bw / 2) >= low_freq_5g) {
			low_limit_5g = chan_enum;
			break;
		}
	}

	for (chan_enum = NUM_CHANNELS - 1; chan_enum >= 0; chan_enum--) {
		min_bw = chan_list[chan_enum].min_bw;
		center_freq = chan_list[chan_enum].center_freq;

		if (center_freq + min_bw / 2 <= high_freq_2g) {
			high_limit_2g = chan_enum;
			break;
		}
		if (chan_enum == 0)
			break;
	}

	for (chan_enum = NUM_CHANNELS - 1; chan_enum >= 0; chan_enum--) {
		min_bw = chan_list[chan_enum].min_bw;
		center_freq = chan_list[chan_enum].center_freq;

		if (center_freq + min_bw / 2 <= high_freq_5g) {
			high_limit_5g = chan_enum;
			break;
		}
		if (chan_enum == 0)
			break;
	}

	chan_in_range = false;
	if  ((low_limit_2g <= ch_enum) &&
	     (high_limit_2g >= ch_enum) &&
	     (low_limit_2g != NUM_CHANNELS) &&
	     (high_limit_2g != NUM_CHANNELS))
		chan_in_range = true;
	if  ((low_limit_5g <= ch_enum) &&
	     (high_limit_5g >= ch_enum) &&
	     (low_limit_5g != NUM_CHANNELS) &&
	     (high_limit_5g != NUM_CHANNELS))
		chan_in_range = true;

	if (chan_in_range)
		return true;
	else
		return false;
}

void reg_update_nol_history_ch(struct wlan_objmgr_pdev *pdev,
			       uint8_t *chan_list, uint8_t num_chan,
			       bool nol_history_chan)
{
	enum channel_enum chan_enum;
	struct regulatory_channel *mas_chan_list;
	struct regulatory_channel *cur_chan_list;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	uint16_t i;

	if (!num_chan || !chan_list) {
		reg_err("chan_list or num_ch is NULL");
		return;
	}

	pdev_priv_obj = wlan_objmgr_pdev_get_comp_private_obj(
			pdev, WLAN_UMAC_COMP_REGULATORY);

	if (!pdev_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	mas_chan_list = pdev_priv_obj->mas_chan_list;
	cur_chan_list = pdev_priv_obj->cur_chan_list;

	for (i = 0; i < num_chan; i++) {
		chan_enum = reg_get_chan_enum(chan_list[i]);
		if (chan_enum == INVALID_CHANNEL) {
			reg_err("Invalid ch in nol list, chan %d",
				chan_list[i]);
			continue;
		}
		mas_chan_list[chan_enum].nol_history = nol_history_chan;
		cur_chan_list[chan_enum].nol_history = nol_history_chan;
	}
}

bool reg_is_24ghz_ch(uint32_t chan)
{
	return REG_IS_24GHZ_CH(chan);
}

bool reg_is_5ghz_ch(uint32_t chan)
{
	return REG_IS_5GHZ_CH(chan);
}

bool reg_is_24ghz_ch_freq(uint32_t freq)
{
	return REG_IS_24GHZ_CH_FREQ(freq);
}

bool reg_is_5ghz_ch_freq(uint32_t freq)
{
	return REG_IS_5GHZ_FREQ(freq);
}

#ifndef CONFIG_LEGACY_CHAN_ENUM
bool reg_is_49ghz_freq(uint32_t freq)
{
	return REG_IS_49GHZ_FREQ(freq);
}
#endif

uint32_t reg_ch_num(uint32_t ch_enum)
{
	return REG_CH_NUM(ch_enum);
}

uint32_t reg_ch_to_freq(uint32_t ch_enum)
{
	return REG_CH_TO_FREQ(ch_enum);
}

bool reg_is_same_band_channels(uint32_t chan_num1, uint32_t chan_num2)
{
	return (chan_num1 && chan_num2 &&
		(REG_IS_5GHZ_CH(chan_num1) == REG_IS_5GHZ_CH(chan_num2)));
}

bool reg_is_channel_valid_5g_sbs(uint32_t curchan, uint32_t newchan)
{
	return REG_IS_CHANNEL_VALID_5G_SBS(curchan, newchan);
}

uint32_t reg_min_24ghz_ch_num(void)
{
	return REG_MIN_24GHZ_CH_NUM;
}

uint32_t reg_max_24ghz_ch_num(void)
{
	return REG_MAX_24GHZ_CH_NUM;
}

uint32_t reg_min_5ghz_ch_num(void)
{
	return REG_MIN_5GHZ_CH_NUM;
}

uint32_t reg_max_5ghz_ch_num(void)
{
	return REG_MAX_5GHZ_CH_NUM;
}

QDF_STATUS reg_enable_dfs_channels(struct wlan_objmgr_pdev *pdev,
				   bool enable)
{
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;
	struct wlan_lmac_if_reg_tx_ops *reg_tx_ops;

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

	reg_tx_ops = reg_get_psoc_tx_ops(psoc);

	/* Fill the ic channel list with the updated current channel
	 * chan list.
	 */
	if (reg_tx_ops->fill_umac_legacy_chanlist)
		reg_tx_ops->fill_umac_legacy_chanlist(pdev,
				pdev_priv_obj->cur_chan_list);

	status = reg_send_scheduler_msg_sb(psoc, pdev);

	return status;
}
