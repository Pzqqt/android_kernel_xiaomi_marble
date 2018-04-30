/*
 * Copyright (c) 2011, 2016-2018 The Linux Foundation. All rights reserved.
 * Copyright (c) 2002-2005 Atheros Communications, Inc.
 * Copyright (c) 2008-2010, Atheros Communications Inc.
 * All Rights Reserved.
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
 * DOC: This file has the radar table for AR5212 chipset and function to
 * initialize the radar table.
 */
#include "../dfs.h"
#include "../dfs_internal.h"
#include "wlan_dfs_utils_api.h"

/* Default 5212/5312 radar phy parameters. */
#define AR5212_DFS_FIRPWR -41
#define AR5212_DFS_RRSSI  12
#define AR5212_DFS_HEIGHT 20
#define AR5212_DFS_PRSSI  22
#define AR5212_DFS_INBAND 6

/**
 * struct dfs_pulse ar5212_etsi_radars - ETSI radar pulse table for
 *                                       AR5212 chipset.
 */
struct dfs_pulse ar5212_etsi_radars[] = {
	/* EN 302 502 frequency hopping pulse */
	/* PRF 3000, 1us duration, 9 pulses per burst */
	{9,   1, 3000, 3000, 1,  4,  5,  0,  1, 18,  0, 0, 1,  1000, 0, 40},
	/* PRF 4500, 20us duration, 9 pulses per burst */
	{9,  20, 4500, 4500, 1,  4,  5, 19, 21, 18,  0, 0, 1,  1000, 0, 41},

	/* TYPE 1 */
	{10, 2,   750,  0, 24, 50,  0,  2, 22,  0, 3, 0, 0},

	/* TYPE 2 */
	{7,  2,   200,  0, 24, 50,  0,  2, 22,  0, 3, 0, 1},
	{7,  2,   300,  0, 24, 50,  0,  2, 22,  0, 3, 0, 2},
	{7,  2,   500,  0, 24, 50,  0,  2, 22,  1, 3, 0, 3},
	{7,  2,   800,  0, 24, 50,  0,  2, 22,  1, 3, 0, 4},
	{7,  2,  1001,  0, 24, 50,  0,  2, 22,  0, 3, 0, 5},
	{7,  8,   200,  0, 24, 50,  6,  9, 22,  8, 3, 0, 6},
	{7,  8,   300,  0, 24, 50,  6,  9, 22,  8, 3, 0, 7},
	{7,  8,   502,  0, 24, 50,  6,  9, 22,  0, 3, 0, 8},
	{7,  8,   805,  0, 24, 50,  6,  9, 22,  0, 3, 0, 9},
	{7,  8,  1008,  0, 24, 50,  6,  9, 22,  0, 3, 0, 10},

	/* TYPE 3 */
	{10, 14,  200,  0, 24, 50, 12, 15, 22, 14, 3, 0, 11},
	{10, 14,  300,  0, 24, 50, 12, 15, 22, 14, 3, 0, 12},
	{10, 14,  503,  0, 24, 50, 12, 15, 22,  2, 3, 0, 13},
	{10, 14,  809,  0, 24, 50, 12, 15, 22,  0, 3, 0, 14},
	{10, 14, 1014,  0, 24, 50, 12, 15, 22,  0, 3, 0, 15},
	{10, 18,  200,  0, 24, 50, 15, 19, 22, 18, 3, 0, 16},
	{10, 18,  301,  0, 24, 50, 15, 19, 22,  7, 3, 0, 17},
	{10, 18,  504,  0, 24, 50, 15, 19, 22,  2, 3, 0, 18},
	{10, 18,  811,  0, 24, 50, 15, 19, 22,  0, 3, 0, 19},
	{10, 18, 1018,  0, 24, 50, 15, 19, 22,  0, 3, 0, 20},

	/* TYPE 4 */
	{10, 2,  1200,  0, 24, 50,  0,  2, 22,  0, 3, 0, 21},
	{10, 2,  1500,  0, 24, 50,  0,  2, 22,  0, 3, 0, 22},
	{10, 2,  1600,  0, 24, 50,  0,  2, 22,  0, 3, 0, 23},
	{10, 8,  1212,  0, 24, 50,  6,  9, 22,  0, 3, 0, 24},
	{10, 8,  1517,  0, 24, 50,  6,  9, 22,  0, 3, 0, 25},
	{10, 8,  1620,  0, 24, 50,  6,  9, 22,  0, 3, 0, 26},
	{10, 14, 1221,  0, 24, 50, 12, 15, 22,  0, 3, 0, 27},
	{10, 14, 1531,  0, 24, 50, 12, 15, 22,  0, 3, 0, 28},
	{10, 14, 1636,  0, 24, 50, 12, 15, 22,  0, 3, 0, 29},
	{10, 18, 1226,  0, 24, 50, 15, 19, 22,  0, 3, 0, 30},
	{10, 18, 1540,  0, 24, 50, 15, 19, 22,  0, 3, 0, 31},
	{10, 18, 1647,  0, 24, 50, 15, 19, 22,  0, 3, 0, 32},

	/* TYPE 5 */
	{17, 2,  2305,  0, 24, 50,  0,  2, 22,  0, 3, 0, 33},
	{17, 2,  3009,  0, 24, 50,  0,  2, 22,  0, 3, 0, 34},
	{17, 2,  3512,  0, 24, 50,  0,  2, 22,  0, 3, 0, 35},
	{17, 2,  4016,  0, 24, 50,  0,  2, 22,  0, 3, 0, 36},
	{17, 8,  2343,  0, 24, 50,  6,  9, 22,  0, 3, 0, 37},
	{17, 8,  3073,  0, 24, 50,  6,  9, 22,  0, 3, 0, 38},
	{17, 8,  3601,  0, 24, 50,  6,  9, 22,  0, 3, 0, 39},
	{17, 8,  4132,  0, 24, 50,  6,  9, 22,  0, 3, 0, 40},
	{17, 14, 2376,  0, 24, 50, 12, 15, 22,  0, 3, 0, 41},
	{17, 14, 3131,  0, 24, 50, 12, 15, 22,  0, 3, 0, 42},
	{17, 14, 3680,  0, 24, 50, 12, 15, 22,  0, 3, 0, 43},
	{17, 14, 4237,  0, 24, 50, 12, 15, 22,  0, 3, 0, 44},
	{17, 18, 2399,  0, 24, 50, 15, 19, 22,  0, 3, 0, 45},
	{17, 18, 3171,  0, 24, 50, 15, 19, 22,  0, 3, 0, 46},
	{17, 18, 3735,  0, 24, 50, 15, 19, 22,  0, 3, 0, 47},
	{17, 18, 4310,  0, 24, 50, 15, 19, 22,  0, 3, 0, 48},

	/* TYPE 6 */
	{14, 22, 2096,  0, 24, 50, 21, 24, 22,  0, 3, 0, 49},
	{14, 22, 3222,  0, 24, 50, 21, 24, 22,  0, 3, 0, 50},
	{14, 22, 4405,  0, 24, 50, 21, 24, 22,  0, 3, 0, 51},
	{14, 32, 2146,  0, 24, 50, 30, 35, 22,  0, 3, 0, 52},
	{14, 32, 3340,  0, 24, 50, 30, 35, 22,  0, 3, 0, 53},
	{14, 32, 4629,  0, 24, 50, 30, 35, 22,  0, 3, 0, 54},
};

/**
 * struct dfs_pulse ar5212_fcc_radars - FCC radar pulse table for
 *                                       AR5212 chipset.
 */
struct dfs_pulse ar5212_fcc_radars[] = {
	/* following two filters are specific to Japan/MKK4 */
	{16,   2,  720,  6, 40,  0,  2, 18,  0, 3, 0, 30},
	{16,   3,  260,  6, 40,  0,  5, 18,  0, 3, 0, 31},

	/* following filters are common to both FCC and JAPAN */
	{9,   2, 3003,   6, 50,  0,  2, 18,  0, 0, 0, 29},
	{16,  2,  700,   6, 35,  0,  2, 18,  0, 3, 0, 28},

	{10,  3, 6666,  10, 90,  2,  3, 22,  0, 3,  0, 0},
	{10,  3, 5900,  10, 90,  2,  3, 22,  0, 3,  0, 1},
	{10,  3, 5200,  10, 90,  2,  3, 22,  0, 3,  0, 2},
	{10,  3, 4800,  10, 90,  2,  3, 22,  0, 3,  0, 3},
	{10,  3, 4400,  10, 90,  2,  3, 22,  0, 3,  0, 4},
	{10,  5, 6666,  50, 30,  3, 10, 22,  0, 3,  0, 5},
	{10,  5, 5900,  70, 30,  3, 10, 22,  0, 3,  0, 6},
	{10,  5, 5200,  70, 30,  3, 10, 22,  0, 3,  0, 7},
	{10,  5, 4800,  70, 30,  3, 10, 22,  0, 3,  0, 8},
	{10,  5, 4400,  50, 30,  3,  9, 22,  0, 3,  0, 9},

	{8,  10, 5000, 100, 40,  7, 17, 22,  0, 3, 0, 10},
	{8,  10, 3000, 100, 40,  7, 17, 22,  0, 3, 0, 11},
	{8,  10, 2000,  40, 40,  9, 17, 22,  0, 3, 0, 12},
	{8,  14, 5000, 100, 40, 13, 16, 22,  0, 3, 0, 13},
	{8,  14, 3000, 100, 40, 13, 16, 22,  0, 3, 0, 14},
	{8,  14, 2000,  40, 40, 13, 16, 22,  0, 3, 0, 15},

	{6,  10, 5000,  80, 40, 10, 15, 22,  0, 3, 0, 16},
	{6,  10, 3000,  80, 40, 10, 15, 22,  0, 3, 0, 17},
	{6,  10, 2000,  40, 40, 10, 15, 22,  0, 3, 0, 18},
	{6,  10, 5000,  80, 40, 10, 12, 22,  0, 3, 0, 19},
	{6,  10, 3000,  80, 40, 10, 12, 22,  0, 3, 0, 20},
	{6,  10, 2000,  40, 40, 10, 12, 22,  0, 3, 0, 21},

	{6,  18, 5000,  80, 40, 16, 25, 22,  0, 3, 0, 22},
	{6,  18, 3000,  80, 40, 16, 25, 22,  0, 3, 0, 23},
	{6,  18, 2000,  40, 40, 16, 25, 22,  0, 3, 0, 24},

	{6,  21, 5000,  80, 40, 12, 25, 22,  0, 3, 0, 25},
	{6,  21, 3000,  80, 40, 12, 25, 22,  0, 3, 0, 26},
	{6,  21, 2000,  40, 40, 12, 25, 22,  0, 3, 0, 27},
};

/**
 * struct dfs_bin5pulse ar5212_bin5pulses - BIN5 pulse for AR5212 chipset.
 */
struct dfs_bin5pulse ar5212_bin5pulses[] = {
	{5, 52, 100, 12, 22, 3},
};

void dfs_get_radars_for_ar5212(struct wlan_dfs *dfs)
{
	struct wlan_dfs_radar_tab_info rinfo;
	int dfsdomain = DFS_FCC_DOMAIN;
	uint16_t ch_freq;
	uint16_t regdmn;

	qdf_mem_zero(&rinfo, sizeof(rinfo));
	dfsdomain = utils_get_dfsdomain(dfs->dfs_pdev_obj);

	switch (dfsdomain) {
	case DFS_FCC_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "DFS_FCC_DOMAIN_5212");
		rinfo.dfsdomain = DFS_FCC_DOMAIN;
		rinfo.dfs_radars = &ar5212_fcc_radars[2];
		rinfo.numradars = QDF_ARRAY_SIZE(ar5212_fcc_radars)-2;
		rinfo.b5pulses = &ar5212_bin5pulses[0];
		rinfo.numb5radars = QDF_ARRAY_SIZE(ar5212_bin5pulses);
		break;
	case DFS_ETSI_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "DFS_ETSI_DOMAIN_5412");
		rinfo.dfsdomain = DFS_ETSI_DOMAIN;

		ch_freq = dfs->dfs_curchan->dfs_ch_freq;
		regdmn = utils_dfs_get_cur_rd(dfs->dfs_pdev_obj);

		if (((regdmn == ETSI11_WORLD_REGDMN_PAIR_ID) ||
		    (regdmn == ETSI12_WORLD_REGDMN_PAIR_ID) ||
		    (regdmn == ETSI13_WORLD_REGDMN_PAIR_ID) ||
		    (regdmn == ETSI14_WORLD_REGDMN_PAIR_ID)) &&
		    DFS_CURCHAN_IS_58GHz(ch_freq)) {
			rinfo.dfs_radars = ar5212_etsi_radars;
			rinfo.numradars = QDF_ARRAY_SIZE(ar5212_etsi_radars);
		} else {
			uint8_t offset = ETSI_LEGACY_PULSE_ARR_OFFSET;

			rinfo.dfs_radars = &ar5212_etsi_radars[offset];
			rinfo.numradars =
				QDF_ARRAY_SIZE(ar5212_etsi_radars) - offset;
		}
		rinfo.b5pulses = &ar5212_bin5pulses[0];
		rinfo.numb5radars = QDF_ARRAY_SIZE(ar5212_bin5pulses);
		break;
	case DFS_MKK4_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "DFS_MKK4_DOMAIN_5412");
		rinfo.dfsdomain = DFS_MKK4_DOMAIN;
		rinfo.dfs_radars = &ar5212_fcc_radars[0];
		rinfo.numradars = QDF_ARRAY_SIZE(ar5212_fcc_radars);
		rinfo.b5pulses = &ar5212_bin5pulses[0];
		rinfo.numb5radars = QDF_ARRAY_SIZE(ar5212_bin5pulses);
		break;
	default:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "No domain");
		return;
	}

	rinfo.dfs_defaultparams.pe_firpwr = AR5212_DFS_FIRPWR;
	rinfo.dfs_defaultparams.pe_rrssi = AR5212_DFS_RRSSI;
	rinfo.dfs_defaultparams.pe_height = AR5212_DFS_HEIGHT;
	rinfo.dfs_defaultparams.pe_prssi = AR5212_DFS_PRSSI;
	rinfo.dfs_defaultparams.pe_inband = AR5212_DFS_INBAND;

	dfs_init_radar_filters(dfs, &rinfo);
}
