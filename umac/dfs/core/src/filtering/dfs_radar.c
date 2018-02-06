/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
 * Copyright (c) 2011, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: This file has radar table and initialization function for Beeliner
 * family of chipsets.
 */

#include "../dfs.h"
#include "wlan_dfs_mlme_api.h"
#include "wlan_dfs_utils_api.h"
#include "wlan_dfs_lmac_api.h"
#include "../dfs_internal.h"

/**
 * struct dfs_pulse dfs_fcc_radars - FCC radar table for Offload chipsets.
 */
struct dfs_pulse dfs_fcc_radars[] = {
	/* FCC TYPE 1 */
	{18,  1,  700, 700, 0,  4,  5,  0,  1, 18,  0, 3,  1, 5, 0, 0},
	{18,  1,  350, 350, 0,  4,  5,  0,  1, 18,  0, 3,  0, 5, 0, 0},

	/* FCC TYPE 6 */
	{9,   1, 3003, 3003, 1,  7,  5,  0,  1, 18,  0, 0,  1, 1000, 0, 1},

	/* FCC TYPE 2 */
	{23, 5, 4347, 6666, 0,  4, 11,  0,  7, 22,  0, 3,  0, 5, 0, 2},

	/* FCC TYPE 3 */
	{18, 10, 2000, 5000, 0,  4,  8,  6, 13, 22,  0, 3, 0, 5, 0, 5},

	/* FCC TYPE 4 */
	{16, 15, 2000, 5000, 0,  4,  7, 11, 23, 22,  0, 3, 0, 5, 0, 11},

	/* FCC NEW TYPE 1 */
	/* 518us to 938us pulses (min 56 pulses) */
	{57, 1, 1066, 1930, 0, 4,  20,  0,  1, 22,  0, 3,  0, 5, 0, 21},

	/* 938us to 2000 pulses (min 26 pulses) */
	{27, 1,  500, 1066, 0, 4,  13,  0,  1, 22,  0, 3,  0, 5, 0, 22},

	/* 2000 to 3067us pulses (min 17 pulses) */
	{18, 1,  325,  500, 0, 4,  9,   0,  1, 22,  0, 3,  0, 5, 0, 23},
};

/**
 * struct dfs_pulse dfs_mkk4_radars - MKK4 radar table for Offload chipsets.
 */
struct dfs_pulse dfs_mkk4_radars[] = {

	/* following two filters are specific to Japan/MKK4 */
	/* 1389 +/- 6 us */
	{18,  1,  720,  720, 0,  4,  6,  0,  1, 18,  0, 3, 0, 5, 0, 17},

	/* 4000 +/- 6 us */
	{18,  4,  250,  250, 0,  4,  5,  1,  6, 18,  0, 3, 0, 5, 0, 18},

	/* 3846 +/- 7 us */
	{18,  5,  260,  260, 0,  4,  6,  1,  6, 18,  0, 3, 1, 5, 0, 19},

	/* following filters are common to both FCC and JAPAN */

	/* FCC TYPE 1 */
	{18,  1,  700, 700, 0,  4,  5,  0,  1, 18,  0, 3,  1, 5, 0, 0},
	{18,  1,  350, 350, 0,  4,  5,  0,  1, 18,  0, 3,  0, 5, 0, 0},

	/* FCC TYPE 6 */
	{9,   1, 3003, 3003, 1,  7,  5,  0,  1, 18,  0, 0, 1,  1000, 0, 1},

	/* FCC TYPE 2 */
	{23, 5, 4347, 6666, 0,  4, 11,  0,  7, 22,  0, 3,  0, 5, 0, 2},

	/* FCC TYPE 3 */
	{18, 10, 2000, 5000, 0,  4,  8,  6, 13, 22,  0, 3, 0, 5, 0, 5},

	/* FCC TYPE 4 */
	{16, 15, 2000, 5000, 0,  4,  7, 11, 23, 22,  0, 3, 0, 5, 0, 11},
};

/**
 * struct dfs_bin5pulse dfs_fcc_bin5pulses - FCC BIN5 pulses for Offload
 *                                           chipsets.
 */
struct dfs_bin5pulse dfs_fcc_bin5pulses[] = {
	{6, 28, 105, 12, 18, 5},
};

/**
 * struct dfs_bin5pulse dfs_jpn_bin5pulses - JAPAN BIN5 pulses for Offload
 *                                           chipsets.
 */
struct dfs_bin5pulse dfs_jpn_bin5pulses[] = {
	{5, 28, 105, 12, 22, 5},
};

/**
 * dfs_bin5pulse dfs_fcc_bin5pulses_ar900b - FCC BIN5 pulses for AR9300
 *                                           chipsets.
 *
 * WAR : IR 42631
 * Beeliner 2 is tested at -65dbm as opposed to -62 dbm.
 * For FCC/JPN chirping pulses, HW reports RSSI value that is lower by 2dbm
 * when we enable noise floor claibration. This is specially true for
 * frequencies that are greater than center frequency and in VHT80 mode.
 */

struct dfs_bin5pulse dfs_fcc_bin5pulses_ar900b[] = {
	{5, 28, 105, 12, 20, 5},
};

/**
 * dfs_bin5pulse dfs_jpn_bin5pulses_ar900b - JAPAN BIN5 pulses for AR9300
 *                                           chipsets.
 */
struct dfs_bin5pulse dfs_jpn_bin5pulses_ar900b[] = {
	{5, 28, 105, 12, 20, 5},
};

/**
 * dfs_bin5pulse dfs_fcc_bin5pulses_qca9984 - FCC BIN5 pulses for QCA9984
 *                                            chipsets.
 * WAR : IR-83400
 * Cascade is tested at -65dbm as opposed to -62 dbm.
 * For FCC/JPN chirping pulses, HW reports RSSI value that is significantly
 * lower at left edge especially in HT80_80 mode. Also, duration may be
 * significantly low. This can result in false detection and we may have to
 * raise the threshold.
 */
struct dfs_bin5pulse dfs_fcc_bin5pulses_qca9984[] = {
	{5, 20, 105, 12, 20, 0},
};

/**
 * dfs_bin5pulse dfs_jpn_bin5pulses_qca9984 - JAPAN BIN5 pulses for QCA9984
 *                                            chipsets.
 */
struct dfs_bin5pulse dfs_jpn_bin5pulses_qca9984[] = {
	{5, 20, 105, 12, 20, 0},
};

/**
 * dfs_pulse dfs_etsi_radars - ETSI radar table.
 */
struct dfs_pulse dfs_etsi_radars[] = {

	/* EN 302 502 frequency hopping pulse */
	/* PRF 3000, 1us duration, 9 pulses per burst */
	{9,   1, 3000, 3000, 1,  4,  5,  0,  1, 18,  0, 0, 1,  1000, 0, 40},
	/* PRF 4500, 20us duration, 9 pulses per burst */
	{9,  20, 4500, 4500, 1,  4,  5, 19, 21, 18,  0, 0, 1,  1000, 0, 41},

	/* TYPE staggered pulse */
	/* Type 5*/
	/* 0.8-2us, 2-3 bursts,300-400 PRF, 10 pulses each */
	{30,  2,  300,  400, 2, 30,  3,  0,  5, 15, 0,   0, 1, 5, 0, 31},
	/* Type 6 */
	/* 0.8-2us, 2-3 bursts, 400-1200 PRF, 15 pulses each */
	{30,  2,  400, 1200, 2, 30,  7,  0,  5, 15, 0,   0, 0, 5, 0, 32},

	/* constant PRF based */
	/* Type 1 */
	/* 0.8-5us, 200  300 PRF, 10 pulses */
	{10, 5,   200,  400, 0,  4,  5,  0,  8, 15, 0,   0, 2, 5, 0, 33},
	{10, 5,   400,  600, 0,  4,  5,  0,  8, 15, 0,   0, 2, 5, 0, 37},
	{10, 5,   600,  800, 0,  4,  5,  0,  8, 15, 0,   0, 2, 5, 0, 38},
	{10, 5,   800, 1000, 0,  4,  5,  0,  8, 15, 0,   0, 2, 5, 0, 39},
	/* {10, 5,   200, 1000, 0,  6,  5,  0,  8, 15, 0,   0, 2, 5, 33}, */

	/* Type 2 */
	/* 0.8-15us, 200-1600 PRF, 15 pulses */
	{15, 15,  200, 1600, 0,  4, 8,  0, 18, 24, 0,   0, 0, 5, 0, 34},

	/* Type 3 */
	/* 0.8-15us, 2300-4000 PRF, 25 pulses*/
	{25, 15, 2300, 4000, 0,  4, 10, 0, 18, 24, 0,   0, 0, 5, 0, 35},

	/* Type 4 */
	/* 20-30us, 2000-4000 PRF, 20 pulses*/
	{20, 30, 2000, 4000, 0,  4, 6, 19, 33, 24, 0,   0, 0, 24,  1, 36},
};

/**
 * dfs_pulse dfs_china_radars - CHINA radar table.
 */
struct dfs_pulse dfs_china_radars[] = {

	/* TYPE staggered pulse */
	/* Type 5*/
	/* 0.8-2us, 2-3 bursts,300-400 PRF, 12 pulses each */
	{36,  2,  300,  400, 2, 30,  3,  0,  5, 15, 0,   0, 1, 51},
	/* Type 6 */
	/* 0.8-2us, 2-3 bursts, 400-1200 PRF, 16 pulses each */
	{48,  2,  400, 1200, 2, 30,  7,  0,  5, 15, 0,   0, 0, 52},

	/* constant PRF based */
	/* Type 1 */
	/* 0.5-5us, 200  1000 PRF, 12 pulses */
	{12, 5,   200,  400, 0, 24,  5,  0,  8, 15, 0,   0, 2, 53},
	{12, 5,   400,  600, 0, 24,  5,  0,  8, 15, 0,   0, 2, 57},
	{12, 5,   600,  800, 0, 24,  5,  0,  8, 15, 0,   0, 2, 58},
	{12, 5,   800, 1000, 0, 24,  5,  0,  8, 15, 0,   0, 2, 59},

	/* Type 2 */
	/* 0.5-15us, 200-1600 PRF, 16 pulses */
	{16, 15,  200, 1600, 0, 24, 8,  0, 18, 24, 0,   0, 0, 54},

	/* Type 3 */
	/* 0.5-30us, 2300-4000 PRF, 24 pulses*/
	{24, 15, 2300, 4000,  0, 24, 10, 0, 33, 24, 0,   0, 0, 55},

	/* Type 4 */
	/* 20-30us, 2000-4000 PRF, 20 pulses*/
	{20, 30, 2000, 4000, 0, 24, 6, 19, 33, 24, 0,   0, 0, 56},

	/* 1us, 1000 PRF, 20 pulses */
	/* 1000 us PRI */
	{20,  1, 1000, 1000, 0,  6,  6,  0,  1, 18,  0, 3, 0, 50},
};

/**
 * dfs_pulse dfs_korea_radars - KOREA radar table.
 */
struct dfs_pulse dfs_korea_radars[] = {
	/* Korea Type 1 */
	{18,  1,  700, 700,  0, 4,  5,  0,  1, 18,  0, 3,  1, 5, 0, 40},

	/* Korea Type 2 */
	{10,  1, 1800, 1800, 0, 4,  4,  0,  1, 18,  0, 3,  1, 5, 0, 41},

	/* Korea Type 3 */
	{70,  1,  330, 330,  0, 4, 20,  0,  2, 18,  0, 3,  1, 5, 0, 42},

	/* Korea Type 4 */
	{3,   1, 3003, 3003, 1, 7,  2,  0,  1, 18,  0, 0, 1,  1000, 0, 43},
};

#define RSSI_THERSH_AR900B    15

/**
 * dfs_assign_fcc_pulse_table() - Assign FCC pulse table
 * @rinfo: Pointer to wlan_dfs_radar_tab_info structure.
 * @target_type: Target type.
 * @tx_ops: target tx ops.
 */
static inline void dfs_assign_fcc_pulse_table(
		struct wlan_dfs_radar_tab_info *rinfo,
		uint32_t target_type,
		struct wlan_lmac_if_target_tx_ops *tx_ops)
{
	rinfo->dfs_radars = dfs_fcc_radars;
	rinfo->numradars = QDF_ARRAY_SIZE(dfs_fcc_radars);

	if (tx_ops->tgt_is_tgt_type_ar900b(target_type) ||
			tx_ops->tgt_is_tgt_type_ipq4019(target_type)) {
		rinfo->b5pulses = dfs_fcc_bin5pulses_ar900b;
		rinfo->numb5radars = QDF_ARRAY_SIZE(dfs_fcc_bin5pulses_ar900b);
	} else if (tx_ops->tgt_is_tgt_type_qca9984(target_type) ||
			tx_ops->tgt_is_tgt_type_qca9888(target_type)) {
		rinfo->b5pulses = dfs_fcc_bin5pulses_qca9984;
		rinfo->numb5radars =
			QDF_ARRAY_SIZE(dfs_fcc_bin5pulses_qca9984);
	} else {
		rinfo->b5pulses = dfs_fcc_bin5pulses;
		rinfo->numb5radars = QDF_ARRAY_SIZE(dfs_fcc_bin5pulses);
	}
}
void ol_if_dfs_configure(struct wlan_dfs *dfs)
{
	struct wlan_dfs_radar_tab_info rinfo;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_target_tx_ops *tx_ops;
	int i;
	uint32_t target_type;
	int dfsdomain = DFS_FCC_DOMAIN;
	uint16_t ch_freq;
	uint16_t regdmn;

	/* Fetch current radar patterns from the lmac */
	qdf_mem_zero(&rinfo, sizeof(rinfo));

	/*
	 * Look up the current DFS regulatory domain and decide
	 * which radar pulses to use.
	 */
	dfsdomain = utils_get_dfsdomain(dfs->dfs_pdev_obj);
	target_type = lmac_get_target_type(dfs->dfs_pdev_obj);

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	if (!psoc) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "psoc is NULL");
		return;
	}

	tx_ops = &(psoc->soc_cb.tx_ops.target_tx_ops);
	switch (dfsdomain) {
	case DFS_FCC_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "FCC domain");
		rinfo.dfsdomain = DFS_FCC_DOMAIN;
		dfs_assign_fcc_pulse_table(&rinfo, target_type, tx_ops);
		break;
	case DFS_CN_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"FCC domain -- Country China(156) override FCC radar pattern"
				);
		rinfo.dfsdomain = DFS_FCC_DOMAIN;
		/*
		 * China uses a radar pattern that is similar to ETSI but it
		 * follows FCC in all other respect like transmit power, CCA
		 * threshold etc.
		 */
		rinfo.dfs_radars = dfs_china_radars;
		rinfo.numradars = QDF_ARRAY_SIZE(dfs_china_radars);
		rinfo.b5pulses = NULL;
		rinfo.numb5radars = 0;
		break;
	case DFS_ETSI_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "ETSI domain");
		rinfo.dfsdomain = DFS_ETSI_DOMAIN;

		ch_freq = dfs->dfs_curchan->dfs_ch_freq;
		regdmn = utils_dfs_get_cur_rd(dfs->dfs_pdev_obj);

		if ((regdmn == ETSI11_WORLD_REGDMN_PAIR_ID) &&
				DFS_CURCHAN_IS_58GHz(ch_freq)) {
			rinfo.dfs_radars = dfs_etsi_radars;
			rinfo.numradars = QDF_ARRAY_SIZE(dfs_etsi_radars);
		} else {
			uint8_t offset = ETSI_LEGACY_PULSE_ARR_OFFSET;

			rinfo.dfs_radars = &dfs_etsi_radars[offset];
			rinfo.numradars =
				QDF_ARRAY_SIZE(dfs_etsi_radars) - offset;
		}
		rinfo.b5pulses = NULL;
		rinfo.numb5radars = 0;
		break;
	case DFS_KR_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"ETSI domain -- Korea(412)");
		rinfo.dfsdomain = DFS_ETSI_DOMAIN;

		/*
		 * So far we have treated Korea as part of ETSI and did not
		 * support any radar patters specific to Korea other than
		 * standard ETSI radar patterns. Ideally we would want to
		 * treat Korea as a different domain. This is something that
		 * we will address in the future. However, for now override
		 * ETSI tables for Korea.
		 */
		rinfo.dfs_radars = dfs_korea_radars;
		rinfo.numradars = QDF_ARRAY_SIZE(dfs_korea_radars);
		rinfo.b5pulses = NULL;
		rinfo.numb5radars = 0;
		break;
	case DFS_MKK4_DOMAIN:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "MKK4 domain");
		rinfo.dfsdomain = DFS_MKK4_DOMAIN;
		rinfo.dfs_radars = dfs_mkk4_radars;
		rinfo.numradars = QDF_ARRAY_SIZE(dfs_mkk4_radars);

		if (tx_ops->tgt_is_tgt_type_ar900b(target_type) ||
				tx_ops->tgt_is_tgt_type_ipq4019(target_type)) {
			rinfo.b5pulses = dfs_jpn_bin5pulses_ar900b;
			rinfo.numb5radars = QDF_ARRAY_SIZE(
					dfs_jpn_bin5pulses_ar900b);
		} else if (tx_ops->tgt_is_tgt_type_qca9984(target_type) ||
				tx_ops->tgt_is_tgt_type_qca9888(target_type)) {
			rinfo.b5pulses = dfs_jpn_bin5pulses_qca9984;
			rinfo.numb5radars = QDF_ARRAY_SIZE
				(dfs_jpn_bin5pulses_qca9984);
		} else {
			rinfo.b5pulses = dfs_jpn_bin5pulses;
			rinfo.numb5radars = QDF_ARRAY_SIZE(
					dfs_jpn_bin5pulses);
		}
		break;
	default:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "UNINIT domain");
		rinfo.dfsdomain = DFS_UNINIT_DOMAIN;
		rinfo.dfs_radars = NULL;
		rinfo.numradars = 0;
		rinfo.b5pulses = NULL;
		rinfo.numb5radars = 0;
		break;
	}

	if (tx_ops->tgt_is_tgt_type_ar900b(target_type) ||
			tx_ops->tgt_is_tgt_type_ipq4019(target_type) ||
			tx_ops->tgt_is_tgt_type_qca9984(target_type) ||
			tx_ops->tgt_is_tgt_type_qca9888(target_type)) {
		/* Beeliner WAR: lower RSSI threshold to improve detection of
		 * certian radar types
		 */
		/* Cascade WAR:
		 * Cascade can report lower RSSI near the channel boundary then
		 * expected. It can also report significantly low RSSI at center
		 * (as low as 16) at center. So we are lowering threshold for
		 * all types of radar for * Cascade.
		 * This may increase the possibility of false radar detection.
		 * IR -- 083703, 083398, 083387
		 */

		for (i = 0; i < rinfo.numradars; i++)
			rinfo.dfs_radars[i].rp_rssithresh = RSSI_THERSH_AR900B;
	}

	dfs_init_radar_filters(dfs, &rinfo);
}

void dfs_get_radars(struct wlan_dfs *dfs)
{
#define AR5212_DEVID_IBM            0x1014 /* IBM minipci ID */
#define AR5212_AR2413               0x001a /* AR2413 aka Griffin-lite */
#define AR5212_AR2413               0x001a /* AR2413 aka Griffin-lite */
#define AR5212_AR5413               0x001b /* Eagle */
#define AR5212_AR5424               0x001c /* Condor (PCI express) */
#define AR5212_DEVID_FF19           0xff19 /* PCI express */
#define AR5212_AR2417               0x001d /* Nala, PCI */
#define AR5212_DEVID                0x0013 /* Final ar5212 devid */
#define AR5212_FPGA                 0xf013 /* Emulation board */
#define AR5212_DEFAULT              0x1113 /* No eeprom HW default */

#define AR5416_DEVID_PCI            0x0023 /* AR5416 PCI (CB/MB) (Owl)*/
#define AR5416_DEVID_PCIE           0x0024 /* AR5416 PCI-E (XB) (Owl) */
#define AR5416_DEVID_AR9160_PCI     0x0027 /* AR9160 PCI (Sowl) */
#define AR5416_AR9100_DEVID         0x000b /* AR9100 (Howl)    */
#define AR5416_DEVID_AR9280_PCI     0x0029 /* PCI (Merlin) */
#define AR5416_DEVID_AR9280_PCIE    0x002a /* PCIE (Merlin) */
#define AR5416_DEVID_AR9285_PCIE    0x002b /* PCIE (Kite) */
#define AR5416_DEVID_AR9285G_PCIE   0x002c /* PCIE (Kite G only) */
#define AR5416_DEVID_AR9287_PCI     0x002d /* PCI (Kiwi) */
#define AR5416_DEVID_AR9287_PCIE    0x002e /* PCIE (Kiwi) */

#define AR9300_DEVID_AR9380_PCIE    0x0030 /* PCIE (Osprey) */
#define AR9300_DEVID_AR9340         0x0031 /* Wasp */
#define AR9300_DEVID_AR9485_PCIE    0x0032 /* Poseidon */
#define AR9300_DEVID_AR9580_PCIE    0x0033 /* Peacock */
#define AR9300_DEVID_AR1111_PCIE    0x0037 /* AR1111 */
#define AR9300_DEVID_AR946X_PCIE    0x0034 /* Jupiter: 2x2 DB + BT - AR9462 */
#define AR9300_DEVID_AR955X         0x0039 /* Scorpion */
#define AR9300_DEVID_AR953X         0x003d /* Honey Bee */
#define AR9300_DEVID_AR956X         0x003f /* Dragonfly */
#define AR9300_DEVID_AR956X_PCIE    0x0036 /* Aphrodite: 1x1 DB + BT - AR9564 */
#define AR9300_DEVID_EMU_PCIE       0xabcd

	struct wlan_objmgr_psoc *psoc;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	if (!psoc) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "psoc is NULL");
		return;
	}

	if (wlan_objmgr_psoc_get_dev_type(psoc) == WLAN_DEV_OL) {
		/* For offload chip */
		ol_if_dfs_configure(dfs);
	} else {
		uint16_t devid = lmac_get_ah_devid(dfs->dfs_pdev_obj);
		/* For DA chip*/

		switch (devid) {
		case AR5212_DEVID_IBM:
		case AR5212_AR2413:
		case AR5212_AR5413:
		case AR5212_AR5424:
		case AR5212_DEVID_FF19:
			devid = AR5212_DEVID;
		case AR5212_AR2417:
		case AR5212_DEVID:
		case AR5212_FPGA:
		case AR5212_DEFAULT:
			dfs_get_radars_for_ar5212(dfs);
			break;
		case AR5416_DEVID_PCI:
		case AR5416_DEVID_PCIE:
		case AR5416_DEVID_AR9160_PCI:
		case AR5416_AR9100_DEVID:
		case AR5416_DEVID_AR9280_PCI:
		case AR5416_DEVID_AR9280_PCIE:
		case AR5416_DEVID_AR9285_PCIE:
		case AR5416_DEVID_AR9285G_PCIE:
		case AR5416_DEVID_AR9287_PCI:
		case AR5416_DEVID_AR9287_PCIE:
			dfs_get_radars_for_ar5416(dfs);
			break;
		case AR9300_DEVID_AR9380_PCIE:
		case AR9300_DEVID_AR9340:
		case AR9300_DEVID_AR9485_PCIE:
		case AR9300_DEVID_AR9580_PCIE:
		case AR9300_DEVID_AR1111_PCIE:
		case AR9300_DEVID_AR946X_PCIE:
		case AR9300_DEVID_AR955X:
		case AR9300_DEVID_AR953X:
		case AR9300_DEVID_AR956X:
		case AR9300_DEVID_AR956X_PCIE:
		case AR9300_DEVID_EMU_PCIE:
			dfs_get_radars_for_ar9300(dfs);
			break;
		}
	}
}

void dfs_send_csa_to_current_chan(struct wlan_dfs *dfs)
{
	qdf_timer_stop(&dfs->wlan_dfstesttimer);
	dfs->wlan_dfstest = 1;
	dfs->wlan_dfstest_ieeechan = dfs->dfs_curchan->dfs_ch_ieee;
	dfs->wlan_dfstesttime = 1;   /* 1ms */
	qdf_timer_mod(&dfs->wlan_dfstesttimer, dfs->wlan_dfstesttime);
}
