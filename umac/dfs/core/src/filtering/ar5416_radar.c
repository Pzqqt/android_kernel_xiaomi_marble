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
 * DOC: This file has the radar table for AR5416 chipset and function to
 * initialize the radar table.
 */

#include "../dfs.h"
#include "../dfs_internal.h"
#include "wlan_dfs_utils_api.h"

/* Default 5413/5416 radar phy parameters. */
#define AR5416_DFS_FIRPWR  -33
#define AR5416_DFS_RRSSI   20
#define AR5416_DFS_HEIGHT  10
#define AR5416_DFS_PRSSI   15
#define AR5416_DFS_INBAND  15
#define AR5416_DFS_RELPWR  8
#define AR5416_DFS_RELSTEP 12
#define AR5416_DFS_MAXLEN  255

/**
 * struct dfs_pulse ar5416_etsi_radars - ETSI radar pulse table for
 *                                       AR5416 chipset.
 */
struct dfs_pulse ar5416_etsi_radars[] = {

	/* EN 302 502 frequency hopping pulse */
	/* PRF 3000, 1us duration, 9 pulses per burst */
	{9,   1, 3000, 3000, 1,  4,  5,  0,  1, 18,  0, 0, 1,  1000, 0, 40},
	/* PRF 4500, 20us duration, 9 pulses per burst */
	{9,  20, 4500, 4500, 1,  4,  5, 19, 21, 18,  0, 0, 1,  1000, 0, 41},

	/* TYPE staggered pulse */
	/* 0.8-2us, 2-3 bursts,300-400 PRF, 10 pulses each */
	{20,  2,  300,  400, 2, 30,  4,  0,  2, 15, 0,   0, 0, 0, 0, 31},
	/* 0.8-2us, 2-3 bursts, 400-1200 PRF, 15 pulses each */
	{30,  2,  400, 1200, 2, 30,  7,  0,  2, 15, 0,   0, 0, 0, 0, 32},

	/* constant PRF based */
	/* 0.8-5us, 200  300 PRF, 10 pulses */
	{10, 5,   200,  400, 0, 24,  5,  0,  8, 18, 0,   0, 0, 0, 0, 33},
	{10, 5,   400,  600, 0, 24,  5,  0,  8, 18, 0,   0, 0, 0, 0, 37},
	{10, 5,   600,  800, 0, 24,  5,  0,  8, 18, 0,   0, 0, 0, 0, 38},
	{10, 5,   800, 1000, 0, 24,  5,  0,  8, 18, 0,   0, 0, 0, 0, 39},

	/* 0.8-15us, 200-1600 PRF, 15 pulses */
	{15, 15,  200, 1600, 0, 24, 6,  0, 18, 15, 0,   0, 0, 0, 0, 34},

	/* 0.8-15us, 2300-4000 PRF, 25 pulses*/
	{25, 15, 2300, 4000,  0, 24, 8, 0, 18, 15, 0,   0, 0, 0, 0, 35},

	/* 20-30us, 2000-4000 PRF, 20 pulses*/
	{20, 30, 2000, 4000, 0, 24, 8, 19, 33, 15, 0,   0, 0, 0, 0, 36},
};

/**
 * struct dfs_pulse ar5416_fcc_radars - FCC radar pulse table for
 *                                       AR5416 chipset.
 */
struct dfs_pulse ar5416_fcc_radars[] = {
	/* following two filters are specific to Japan/MKK4 */
	/* 1389 +/- 6 us */
	{18,  1,  720,  720, 0,  6,  6,  0,  1, 18,  0, 3, 0, 0, 0, 17},
	/* 4000 +/- 6 us */
	{18,  4,  250,  250, 0, 10,  5,  1,  6, 18,  0, 3, 0, 0, 0, 18},
	/* 3846 +/- 7 us */
	{18,  5,  260,  260, 0, 10,  6,  1,  6, 18,  0, 3, 0, 0, 0, 19},

	/* following filters are common to both FCC and JAPAN */
	/* FCC TYPE 1 */
	{18,  1,  700, 700, 0,  6,  5,  0,  1, 18,  0, 3,  0, 0, 0, 0},
	{18,  1,  350, 350, 0,  6,  5,  0,  1, 18,  0, 3,  0, 0, 0, 0},

	/* FCC TYPE 6 */
	{9,   1, 3003, 3003, 1,  7,  5,  0,  1, 18,  0, 0,  0, 0, 0, 1},

	/* FCC TYPE 2 */
	{23, 5, 4347, 6666, 0, 18, 11,  0,  7, 20,  0, 3,  0, 0, 0, 2},

	/* FCC TYPE 3 */
	{18, 10, 2000, 5000, 0, 23,  8,  6, 13, 20,  0, 3, 0, 0, 0, 5},

	/* FCC TYPE 4 */
	{16, 15, 2000, 5000, 0, 25,  7, 11, 23, 20,  0, 3, 0, 0, 0, 11},
};

/**
 * struct dfs_bin5pulse ar5416_bin5pulses - BIN5 pulse for AR5416 chipset.
 */
struct dfs_bin5pulse ar5416_bin5pulses[] = {
	{2, 28, 105, 12, 22, 5},
};

void dfs_get_radars_for_ar5416(struct wlan_dfs *dfs)
{
	struct wlan_dfs_radar_tab_info rinfo;
	int dfsdomain = DFS_FCC_DOMAIN;
	uint16_t ch_freq;
	uint16_t regdmn;

	qdf_mem_zero(&rinfo, sizeof(rinfo));
	dfsdomain = utils_get_dfsdomain(dfs->dfs_pdev_obj);

	switch (dfsdomain) {
	case DFS_FCC_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "DFS_FCC_DOMAIN_5416");
		rinfo.dfsdomain = DFS_FCC_DOMAIN;
		rinfo.dfs_radars = &ar5416_fcc_radars[3];
		rinfo.numradars = QDF_ARRAY_SIZE(ar5416_fcc_radars)-3;
		rinfo.b5pulses = &ar5416_bin5pulses[0];
		rinfo.numb5radars = QDF_ARRAY_SIZE(ar5416_bin5pulses);
		break;
	case DFS_ETSI_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "DFS_ETSI_DOMAIN_5416");
		rinfo.dfsdomain = DFS_ETSI_DOMAIN;

		ch_freq = dfs->dfs_curchan->dfs_ch_freq;
		regdmn = utils_dfs_get_cur_rd(dfs->dfs_pdev_obj);

		if (((regdmn == ETSI11_WORLD_REGDMN_PAIR_ID) ||
		    (regdmn == ETSI12_WORLD_REGDMN_PAIR_ID) ||
		    (regdmn == ETSI13_WORLD_REGDMN_PAIR_ID) ||
		    (regdmn == ETSI14_WORLD_REGDMN_PAIR_ID)) &&
		    DFS_CURCHAN_IS_58GHz(ch_freq)) {
			rinfo.dfs_radars = ar5416_etsi_radars;
			rinfo.numradars = QDF_ARRAY_SIZE(ar5416_etsi_radars);
		} else {
			uint8_t offset = ETSI_LEGACY_PULSE_ARR_OFFSET;

			rinfo.dfs_radars = &ar5416_etsi_radars[offset];
			rinfo.numradars =
				QDF_ARRAY_SIZE(ar5416_etsi_radars) - offset;
		}

		rinfo.b5pulses = &ar5416_bin5pulses[0];
		rinfo.numb5radars = QDF_ARRAY_SIZE(ar5416_bin5pulses);
		break;
	case DFS_MKK4_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "DFS_MKK4_DOMAIN_5416");
		rinfo.dfsdomain = DFS_MKK4_DOMAIN;
		rinfo.dfs_radars = &ar5416_fcc_radars[0];
		rinfo.numradars = QDF_ARRAY_SIZE(ar5416_fcc_radars);
		rinfo.b5pulses = &ar5416_bin5pulses[0];
		rinfo.numb5radars = QDF_ARRAY_SIZE(ar5416_bin5pulses);
		break;
	default:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "no domain");
		return;
	}

	rinfo.dfs_defaultparams.pe_firpwr = AR5416_DFS_FIRPWR;
	rinfo.dfs_defaultparams.pe_rrssi = AR5416_DFS_RRSSI;
	rinfo.dfs_defaultparams.pe_height = AR5416_DFS_HEIGHT;
	rinfo.dfs_defaultparams.pe_prssi = AR5416_DFS_PRSSI;
	rinfo.dfs_defaultparams.pe_inband = AR5416_DFS_INBAND;
	rinfo.dfs_defaultparams.pe_relpwr = AR5416_DFS_RELPWR;
	rinfo.dfs_defaultparams.pe_relstep = AR5416_DFS_RELSTEP;
	rinfo.dfs_defaultparams.pe_maxlen = AR5416_DFS_MAXLEN;

	dfs_init_radar_filters(dfs, &rinfo);
}
