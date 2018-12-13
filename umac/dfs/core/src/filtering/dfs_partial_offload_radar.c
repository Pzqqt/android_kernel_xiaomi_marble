/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_dfs_utils_api.h"
#include "wlan_dfs_lmac_api.h"
#include "../dfs_internal.h"
#include "../dfs_partial_offload_radar.h"
#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
#include "../dfs_process_radar_found_ind.h"
#endif

/**
 * struct dfs_pulse dfs_fcc_radars - FCC radar table for Offload chipsets.
 */
static struct dfs_pulse dfs_fcc_radars[] = {
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
static struct dfs_pulse dfs_mkk4_radars[] = {

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
static struct dfs_bin5pulse dfs_fcc_bin5pulses[] = {
	{6, 28, 105, 12, 18, 5},
};

/**
 * struct dfs_bin5pulse dfs_jpn_bin5pulses - JAPAN BIN5 pulses for Offload
 *                                           chipsets.
 */
static struct dfs_bin5pulse dfs_jpn_bin5pulses[] = {
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

static struct dfs_bin5pulse dfs_fcc_bin5pulses_ar900b[] = {
	{5, 28, 105, 12, 20, 5},
};

/**
 * dfs_bin5pulse dfs_jpn_bin5pulses_ar900b - JAPAN BIN5 pulses for AR9300
 *                                           chipsets.
 */
static struct dfs_bin5pulse dfs_jpn_bin5pulses_ar900b[] = {
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
static struct dfs_bin5pulse dfs_fcc_bin5pulses_qca9984[] = {
	{5, 20, 105, 12, 20, 0},
};

/**
 * dfs_bin5pulse dfs_jpn_bin5pulses_qca9984 - JAPAN BIN5 pulses for QCA9984
 *                                            chipsets.
 */
static struct dfs_bin5pulse dfs_jpn_bin5pulses_qca9984[] = {
	{5, 20, 105, 12, 20, 0},
};

/**
 * dfs_pulse dfs_etsi_radars - ETSI radar table.
 */
static struct dfs_pulse dfs_etsi_radars[] = {

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
static struct dfs_pulse dfs_china_radars[] = {

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
static struct dfs_pulse dfs_korea_radars[] = {
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

void dfs_get_po_radars(struct wlan_dfs *dfs)
{
	struct wlan_dfs_radar_tab_info rinfo;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_target_tx_ops *tx_ops;
	int i;
	uint32_t target_type;
	int dfsdomain = DFS_FCC_DOMAIN;

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
		dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS, "FCC domain");
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

		if (dfs_is_en302_502_applicable(dfs)) {
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

	WLAN_DFS_DATA_STRUCT_LOCK(dfs);
	dfs_init_radar_filters(dfs, &rinfo);
	WLAN_DFS_DATA_STRUCT_UNLOCK(dfs);
}

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(HOST_DFS_SPOOF_TEST)
void dfs_send_avg_params_to_fw(struct wlan_dfs *dfs,
			       struct dfs_radar_found_params *params)
{
	tgt_dfs_send_avg_params_to_fw(dfs->dfs_pdev_obj, params);
}

/**
 * dfs_no_res_from_fw_task() - The timer function that is called if there is no
 * response from fw after sending the average radar pulse parameters.
 */
static os_timer_func(dfs_no_res_from_fw_task)
{
	struct wlan_dfs *dfs = NULL;

	OS_GET_TIMER_ARG(dfs, struct wlan_dfs *);

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "Host wait timer expired");

	dfs->dfs_is_host_wait_running = 0;
	dfs->dfs_no_res_from_fw = 1;
	dfs_radarfound_action_generic(dfs, dfs->dfs_seg_id);
	dfs->dfs_seg_id = 0;
}

void dfs_host_wait_timer_init(struct wlan_dfs *dfs)
{
	qdf_timer_init(NULL,
		       &(dfs->dfs_host_wait_timer),
			dfs_no_res_from_fw_task,
			(void *)(dfs),
			QDF_TIMER_TYPE_WAKE_APPS);
	dfs->dfs_status_timeout_override = -1;
}

QDF_STATUS dfs_set_override_status_timeout(struct wlan_dfs *dfs,
				       int status_timeout)
{
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	dfs->dfs_status_timeout_override = status_timeout;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "Host wait status timeout is now %s : %d",
		(status_timeout == -1) ? "default" : "overridden",
		status_timeout);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dfs_get_override_status_timeout(struct wlan_dfs *dfs,
					   int *status_timeout)
{
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	*status_timeout = dfs->dfs_status_timeout_override;

	return QDF_STATUS_SUCCESS;
}

/**
 * dfs_extract_radar_found_params() - Copy the contents of average radar
 * parameters to dfs_radar_found_params parameter structure.
 *
 * @dfs: Pointer to wlan_dfs structure which contains the average radar
 * parameters.
 * @params: Pointer to dfs_radar_found_params structure.
 */
static
void dfs_extract_radar_found_params(struct wlan_dfs *dfs,
				    struct dfs_radar_found_params *params)
{
	qdf_mem_zero(params, sizeof(*params));
	params->pri_min = dfs->dfs_average_pri;
	params->pri_max = dfs->dfs_average_pri;
	params->duration_min = dfs->dfs_average_duration;
	params->duration_max = dfs->dfs_average_duration;
	params->sidx_min = dfs->dfs_average_sidx;
	params->sidx_max = dfs->dfs_average_sidx;

	/* Bangradar will not populate any of these average
	 * parameters as pulse is not received. If these variables
	 * are not resetted here, these go as radar_found params
	 * for bangradar if bangradar is issued after real radar.
	 */
	dfs->dfs_average_sidx = 0;
	dfs->dfs_average_duration = 0;
	dfs->dfs_average_pri = 0;
}

void dfs_radarfound_action_fcc(struct wlan_dfs *dfs, uint8_t seg_id)
{
	struct dfs_radar_found_params params;

	qdf_mem_copy(&dfs->dfs_radar_found_chan, dfs->dfs_curchan,
		     sizeof(dfs->dfs_radar_found_chan));
	dfs_extract_radar_found_params(dfs, &params);
	dfs_send_avg_params_to_fw(dfs, &params);
	dfs->dfs_is_host_wait_running = 1;
	dfs->dfs_seg_id = seg_id;
	qdf_timer_mod(&dfs->dfs_host_wait_timer,
		      (dfs->dfs_status_timeout_override ==
		       -1) ? HOST_DFS_STATUS_WAIT_TIMER_MS :
		      dfs->dfs_status_timeout_override);
}

void dfs_host_wait_timer_reset(struct wlan_dfs *dfs)
{
	dfs->dfs_is_host_wait_running = 0;
	qdf_timer_sync_cancel(&dfs->dfs_host_wait_timer);
}

/**
 * dfs_action_on_spoof_success() - DFS action on spoof test pass
 * @dfs: Pointer to DFS object
 */
static void dfs_action_on_spoof_success(struct wlan_dfs *dfs)
{
	dfs->dfs_spoof_test_done = 1;
	if (dfs->dfs_radar_found_chan.dfs_ch_freq ==
			dfs->dfs_curchan->dfs_ch_freq) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "cac timer started for channel %d",
			 dfs->dfs_curchan->dfs_ch_ieee);
		dfs_start_cac_timer(dfs);
	} else{
		dfs_remove_spoof_channel_from_nol(dfs);
	}
}

void dfs_action_on_fw_radar_status_check(struct wlan_dfs *dfs,
					 uint32_t *status)
{
	struct wlan_objmgr_pdev *dfs_pdev;
	int no_chans_avail = 0;
	int error_flag = 0;

	dfs_host_wait_timer_reset(dfs);
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "Host DFS status = %d",
		 *status);

	dfs_pdev = dfs->dfs_pdev_obj;
	if (!dfs_pdev) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs_pdev_obj is NULL");
		return;
	}

	switch (*status) {
	case HOST_DFS_STATUS_CHECK_PASSED:
		if (dfs->dfs_average_params_sent)
			dfs_action_on_spoof_success(dfs);
		else
			error_flag = 1;
		break;
	case HOST_DFS_STATUS_CHECK_FAILED:
		dfs->dfs_spoof_check_failed = 1;
		no_chans_avail =
		    dfs_mlme_rebuild_chan_list_with_non_dfs_channels(dfs_pdev);
		dfs_mlme_restart_vaps_with_non_dfs_chan(dfs_pdev,
							no_chans_avail);
		break;
	case HOST_DFS_STATUS_CHECK_HW_RADAR:
		if (dfs->dfs_average_params_sent) {
			if (dfs->dfs_radar_found_chan.dfs_ch_freq ==
			    dfs->dfs_curchan->dfs_ch_freq) {
				dfs_radarfound_action_generic(
						dfs,
						dfs->dfs_seg_id);
			} else {
				/* Else of this case, no action is needed as
				 * dfs_action would have been done at timer
				 * expiry itself.
				 */
				dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
					 "DFS Action already taken");
			}
		} else {
			error_flag = 1;
		}
		break;
	default:
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Status event mismatch:%d, Ignoring it",
			 *status);
	}

	dfs->dfs_average_params_sent = 0;
	qdf_mem_zero(&dfs->dfs_radar_found_chan, sizeof(struct dfs_channel));

	if (error_flag == 1) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Received imroper response %d. Discarding it",
			 *status);
	}
}

void dfs_reset_spoof_test(struct wlan_dfs *dfs)
{
	dfs->dfs_spoof_test_done = 0;
	dfs->dfs_spoof_check_failed = 0;
}
#endif
