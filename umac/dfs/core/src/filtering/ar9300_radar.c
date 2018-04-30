/*
 * Copyright (c) 2011, 2016-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file has the radar table for AR9300 chipset and function to
 * initialize the radar table.
 */

#include "../dfs.h"
#include "../dfs_internal.h"
#include "wlan_dfs_utils_api.h"
#include "wlan_dfs_lmac_api.h"

/*
 * Default 5413/9300 radar phy parameters
 * Values adjusted to fix EV76432/EV76320
 */
#define AR9300_DFS_FIRPWR    -28
#define AR9300_DFS_RRSSI     0
#define AR9300_DFS_HEIGHT    10
#define AR9300_DFS_PRSSI     6
#define AR9300_DFS_INBAND    8
#define AR9300_DFS_RELPWR    8
#define AR9300_DFS_RELSTEP   12
#define AR9300_DFS_MAXLEN    255
#define AR9300_DFS_PRSSI_CAC 10

/*
 * Make sure that value matches value in ar9300_osprey_2p2_mac_core[][2] for
 * register 0x1040 to 0x104c.
 */
#define AR9300_FCC_RADARS_FCC_OFFSET 4

/**
 * struct dfs_pulse ar9300_etsi_radars - ETSI radar pulse table for
 *                                       AR9300 chipset.
 *
 * For short pulses, RSSI threshold should be smaller than Kquick-drop.
 * The chip has only one chance to drop the gain which will be reported
 * as the estimated RSSI.
 */
struct dfs_pulse ar9300_etsi_radars[] = {

	/* EN 302 502 frequency hopping pulse */
	/* PRF 3000, 1us duration, 9 pulses per burst */
	{9,   1, 3000, 3000, 1,  4,  5,  0,  1, 18,  0, 0, 1,  1000, 0, 40},
	/* PRF 4500, 20us duration, 9 pulses per burst */
	{9,  20, 4500, 4500, 1,  4,  5, 19, 21, 18,  0, 0, 1,  1000, 0, 41},

	/* TYPE staggered pulse */
	/* Type 5*/
	/* 0.8-2us, 2-3 bursts,300-400 PRF, 10 pulses each */
	{30,  2,  300,  400, 2, 30,  3,  0,  5, 15, 0,   0, 1, 0, 0, 31},
	/* Type 6 */
	/* 0.8-2us, 2-3 bursts, 400-1200 PRF, 15 pulses each */
	{30,  2,  400, 1200, 2, 30,  7,  0,  5, 15, 0,   0, 0, 0, 0, 32},

	/* constant PRF based */
	/* Type 1 */
	/* 0.8-5us, 200  300 PRF, 10 pulses */
	{10, 5,   200,  400, 0, 24,  5,  0,  8, 15, 0,   0, 2, 0, 0, 33},
	{10, 5,   400,  600, 0, 24,  5,  0,  8, 15, 0,   0, 2, 0, 0, 37},
	{10, 5,   600,  800, 0, 24,  5,  0,  8, 15, 0,   0, 2, 0, 0, 38},
	{10, 5,   800, 1000, 0, 24,  5,  0,  8, 15, 0,   0, 2, 0, 0, 39},

	/* Type 2 */
	/* 0.8-15us, 200-1600 PRF, 15 pulses */
	{15, 15,  200, 1600, 0, 24, 8,  0, 18, 24, 0,   0, 0, 0, 0, 34},

	/* Type 3 */
	/* 0.8-15us, 2300-4000 PRF, 25 pulses*/
	{25, 15, 2300, 4000,  0, 24, 10, 0, 18, 24, 0,   0, 0, 0, 0, 35},

	/* Type 4 */
	/* 20-30us, 2000-4000 PRF, 20 pulses*/
	{20, 30, 2000, 4000, 0, 24, 8, 19, 33, 24, 0,   0, 0, 0, 0, 36},
};

/**
 * struct dfs_pulse ar9300_fcc_radars - FCC radar pulse table for
 *                                       AR9300 chipset.
 */
struct dfs_pulse ar9300_fcc_radars[] = {
	/*
	 * Format is as following:
	 * Numpulses pulsedur pulsefreq max_pulsefreq patterntype pulsevar
	 * threshold mindur maxdur rssithresh meanoffset rssimargin pulseid.
	 */

	/* following two filters are specific to Japan/MKK4 */
	/* 1389 +/- 6 us */
	{18,  1,  720,  720, 0,  6,  6,  0,  1, 18,  0, 3, 0, 0, 0, 17},
	/* 4000 +/- 6 us */
	{18,  4,  250,  250, 0, 10,  5,  1,  6, 18,  0, 3, 0, 0, 0, 18},
	/* 3846 +/- 7 us */
	{18,  5,  260,  260, 0, 10,  6,  1,  6, 18,  0, 3, 1, 0, 0, 19},
	/* 3846 +/- 7 us */
	{18, 5, 260, 260, 1, 10, 6, 1, 6, 18, 0, 3, 1, 0, 0, 20},

	/* following filters are common to both FCC and JAPAN */

	/* FCC TYPE 1 */
	{18,  1,  700, 700, 0,  6,  5,  0,  1, 18,  0, 3, 1, 0, 0, 8},
	{18,  1,  350, 350, 0,  6,  5,  0,  1, 18,  0, 3, 0, 0, 0, 0},

	/* FCC TYPE 6 */
	{9,   1, 3003, 3003, 0,  7,  5,  0,  1, 18,  0, 0, 1, 0, 0, 1},

	/* FCC TYPE 2 */
	{23, 5, 4347, 6666, 0, 18, 11,  0,  7, 22,  0, 3, 0, 0, 0, 2},

	/* FCC TYPE 3 */
	{18, 10, 2000, 5000, 0, 23,  8,  6, 13, 22,  0, 3, 0, 0, 0, 5},

	/* FCC TYPE 4 */
	{16, 15, 2000, 5000, 0, 25,  7, 11, 23, 22,  0, 3, 0, 0, 0, 11},

	/* FCC NEW TYPE 1 */
	/* Search duration is numpulses*maxpri.
	 * The last theshold can be increased if false detects happen
	 */
	/* 518us to 938us pulses (min 56 pulses) */
	{57, 1, 1066, 1930, 0, 6, 20,  0,  1, 22,  0, 3, 0, 0, 0, 21},
	/* 938us to 2000 pulses (min 26 pulses) */
	{27, 1,  500, 1066, 0, 6, 13,  0,  1, 22,  0, 3, 0, 0, 0, 22},
	/* 2000 to 3067us pulses (min 17 pulses)*/
	{18, 1,  325,  500, 0, 6,  9,  0,  1, 22,  0, 3, 0, 0, 0, 23},

};

/**
 * struct dfs_bin5pulse ar9300_bin5pulses - BIN5 pulse for AR9300 chipset.
 */
struct dfs_bin5pulse ar9300_bin5pulses[] = {
	{2, 28, 105, 12, 22, 5},
};

/**
 * struct dfs_pulse ar9300_korea_radars - DFS pulses for KOREA domain.
 */
struct dfs_pulse ar9300_korea_radars[] = {
	/* Korea Type 1 */
	{18,  1,  700, 700,  0, 6,  5,  0,  1, 18,  0, 3,  1, 0, 0, 40},
	/* Korea Type 2 */
	{10,  1, 1800, 1800, 0, 6,  4,  0,  1, 18,  0, 3,  1, 0, 0, 41},
	/* Korea Type 3 */
	{70,  1,  330, 330,  0, 6, 20,  0,  2, 18,  0, 3,  1, 0, 0, 42},
	/* Korea Type 4 */
	{3,   1, 3003, 3003, 1, 7,  2,  0,  1, 18,  0, 0, 1,  0, 0, 43},
};

void dfs_get_radars_for_ar9300(struct wlan_dfs *dfs)
{
	struct wlan_dfs_radar_tab_info rinfo;
	int dfsdomain = DFS_FCC_DOMAIN;
	uint16_t ch_freq;
	uint16_t regdmn;

	qdf_mem_zero(&rinfo, sizeof(rinfo));
	dfsdomain = utils_get_dfsdomain(dfs->dfs_pdev_obj);

	switch (dfsdomain) {
	case DFS_FCC_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "DFS_FCC_DOMAIN_9300");
		rinfo.dfsdomain = DFS_FCC_DOMAIN;
		rinfo.dfs_radars =
			&ar9300_fcc_radars[AR9300_FCC_RADARS_FCC_OFFSET];
		rinfo.numradars =
			(QDF_ARRAY_SIZE(ar9300_fcc_radars) -
			 AR9300_FCC_RADARS_FCC_OFFSET);
		rinfo.b5pulses = &ar9300_bin5pulses[0];
		rinfo.numb5radars = QDF_ARRAY_SIZE(ar9300_bin5pulses);
		break;
	case DFS_ETSI_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "DFS_ETSI_DOMAIN_9300");
		rinfo.dfsdomain = DFS_ETSI_DOMAIN;

		ch_freq = dfs->dfs_curchan->dfs_ch_freq;
		regdmn = utils_dfs_get_cur_rd(dfs->dfs_pdev_obj);

		if (((regdmn == ETSI11_WORLD_REGDMN_PAIR_ID) ||
		    (regdmn == ETSI12_WORLD_REGDMN_PAIR_ID) ||
		    (regdmn == ETSI13_WORLD_REGDMN_PAIR_ID) ||
		    (regdmn == ETSI14_WORLD_REGDMN_PAIR_ID)) &&
		    DFS_CURCHAN_IS_58GHz(ch_freq)) {
			rinfo.dfs_radars = ar9300_etsi_radars;
			rinfo.numradars = QDF_ARRAY_SIZE(ar9300_etsi_radars);
		} else {
			uint8_t offset = ETSI_LEGACY_PULSE_ARR_OFFSET;

			rinfo.dfs_radars = &ar9300_etsi_radars[offset];
			rinfo.numradars =
				QDF_ARRAY_SIZE(ar9300_etsi_radars) - offset;
		}

		rinfo.b5pulses = &ar9300_bin5pulses[0];
		rinfo.numb5radars = QDF_ARRAY_SIZE(ar9300_bin5pulses);
		break;
	case DFS_KR_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"DFS_ETSI_DOMAIN_9300_Country_Korea");
		rinfo.dfsdomain = DFS_ETSI_DOMAIN;
		rinfo.dfs_radars = &ar9300_korea_radars[0];
		rinfo.numradars = QDF_ARRAY_SIZE(ar9300_korea_radars);
		rinfo.b5pulses = &ar9300_bin5pulses[0];
		rinfo.numb5radars = QDF_ARRAY_SIZE(ar9300_bin5pulses);
		break;
	case DFS_MKK4_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "DFS_MKK4_DOMAIN_9300");
		rinfo.dfsdomain = DFS_MKK4_DOMAIN;
		rinfo.dfs_radars = &ar9300_fcc_radars[0];
		rinfo.numradars = QDF_ARRAY_SIZE(ar9300_fcc_radars);
		rinfo.b5pulses = &ar9300_bin5pulses[0];
		rinfo.numb5radars = QDF_ARRAY_SIZE(ar9300_bin5pulses);
		break;
	default:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "no domain");
		return;
	}

	lmac_set_use_cac_prssi(dfs->dfs_pdev_obj);

	rinfo.dfs_defaultparams.pe_firpwr = AR9300_DFS_FIRPWR;
	rinfo.dfs_defaultparams.pe_rrssi = AR9300_DFS_RRSSI;
	rinfo.dfs_defaultparams.pe_height = AR9300_DFS_HEIGHT;
	rinfo.dfs_defaultparams.pe_prssi = AR9300_DFS_PRSSI;

	/*
	 * We have an issue with PRSSI.
	 * For normal operation we use AR9300_DFS_PRSSI, which is set to 6.
	 * Please refer to EV91563, 94164.
	 * However, this causes problem during CAC as no radar is detected
	 * during that period with PRSSI=6. Only PRSSI= 10 seems to fix this.
	 * We use this flag to keep track of change in PRSSI.
	 */
	rinfo.dfs_defaultparams.pe_inband = AR9300_DFS_INBAND;
	rinfo.dfs_defaultparams.pe_relpwr = AR9300_DFS_RELPWR;
	rinfo.dfs_defaultparams.pe_relstep = AR9300_DFS_RELSTEP;
	rinfo.dfs_defaultparams.pe_maxlen = AR9300_DFS_MAXLEN;

	dfs_init_radar_filters(dfs, &rinfo);
}
