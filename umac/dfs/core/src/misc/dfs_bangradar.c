/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2002-2006, Atheros Communications Inc.
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
 * DOC: This file contains the DFS Bangradar functionality.
 *
 */

#include "dfs.h"
#include "dfs_zero_cac.h"
#include "wlan_dfs_lmac_api.h"
#include "wlan_dfs_mlme_api.h"
#include "wlan_dfs_tgt_api.h"
#include "dfs_internal.h"

/**
 * dfs_check_bangradar_sanity() - Check the sanity of bangradar
 * @dfs: Pointer to wlan_dfs structure.
 * @bangradar_params: Parameters of the radar to be simulated.
 */
static QDF_STATUS
dfs_check_bangradar_sanity(struct wlan_dfs *dfs,
			   struct dfs_bangradar_params *bangradar_params)
{
	if (!bangradar_params) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS,
			  "bangradar params is NULL");
		return -EINVAL;
	}
	if (dfs_is_true_160mhz_supported(dfs)) {
		if (abs(bangradar_params->freq_offset) >
		    FREQ_OFFSET_BOUNDARY_FOR_160MHZ) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS,
				  "Frequency Offset out of bound");
			return -EINVAL;
		}
	} else if (abs(bangradar_params->freq_offset) >
		   FREQ_OFFSET_BOUNDARY_FOR_80MHZ) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS,
			  "Frequency Offset out of bound");
		return -EINVAL;
	}
	if (bangradar_params->seg_id > SEG_ID_SECONDARY) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS, "Invalid segment ID");
		return -EINVAL;
	}
	if ((bangradar_params->detector_id > dfs_get_agile_detector_id(dfs)) ||
	    ((bangradar_params->detector_id ==
	      dfs_get_agile_detector_id(dfs)) &&
	      !dfs->dfs_is_offload_enabled)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS, "Invalid detector ID");
		return -EINVAL;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * dfs_start_host_based_bangradar() - Mark as bangradar and start
 * wlan_dfs_task_timer.
 * @dfs: Pointer to wlan_dfs structure.
 */
#if defined(WLAN_DFS_PARTIAL_OFFLOAD)
static int dfs_start_host_based_bangradar(struct wlan_dfs *dfs)
{
	dfs->wlan_radar_tasksched = 1;
	qdf_timer_mod(&dfs->wlan_dfs_task_timer, 0);

	return 0;
}
#else
static inline int dfs_start_host_based_bangradar(struct wlan_dfs *dfs)
{
	return 0;
}
#endif

/**
 * dfs_fill_emulate_bang_radar_test() - Update dfs unit test arguments and
 * send bangradar command to firmware.
 * @dfs: Pointer to wlan_dfs structure.
 * @bangradar_params: Parameters of the radar to be simulated.
 *
 * Return: If the event is received return 0.
 */
#if defined(WLAN_DFS_FULL_OFFLOAD)
#define ADD_TO_32BYTE(_arg, _shift, _mask) (((_arg) & (_mask)) << (_shift))
static int
dfs_fill_emulate_bang_radar_test(struct wlan_dfs *dfs,
				 struct dfs_bangradar_params *bangradar_params)
{
	struct dfs_emulate_bang_radar_test_cmd dfs_unit_test;
	uint32_t packed_args = 0;

	if (!(WLAN_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(dfs->dfs_curchan))) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Ignore bangradar on a NON-DFS channel");
		return -EINVAL;
	}

	packed_args =
		ADD_TO_32BYTE(bangradar_params->seg_id,
			      SEG_ID_SHIFT,
			      SEG_ID_MASK) |
		ADD_TO_32BYTE(bangradar_params->is_chirp,
			      IS_CHIRP_SHIFT,
			      IS_CHIRP_MASK) |
		ADD_TO_32BYTE(bangradar_params->freq_offset,
			      FREQ_OFF_SHIFT,
			      FREQ_OFFSET_MASK) |
		ADD_TO_32BYTE(bangradar_params->detector_id,
			      DET_ID_SHIFT,
			      DET_ID_MASK);

	qdf_mem_zero(&dfs_unit_test, sizeof(dfs_unit_test));
	dfs_unit_test.num_args = DFS_UNIT_TEST_NUM_ARGS;
	dfs_unit_test.args[IDX_CMD_ID] =
			DFS_PHYERR_OFFLOAD_TEST_SET_RADAR;
	dfs_unit_test.args[IDX_PDEV_ID] =
			wlan_objmgr_pdev_get_pdev_id(dfs->dfs_pdev_obj);
	dfs_unit_test.args[IDX_RADAR_PARAM1_ID] = packed_args;

	if (tgt_dfs_process_emulate_bang_radar_cmd(dfs->dfs_pdev_obj,
						   &dfs_unit_test) ==
			QDF_STATUS_E_FAILURE) {
		return -EINVAL;
	}

	return 0;
}
#else
static inline int
dfs_fill_emulate_bang_radar_test(struct wlan_dfs *dfs,
				 struct dfs_bangradar_params *bangradar_params)
{
	return 0;
}
#endif

/*
 * Handle all types of Bangradar here.
 * Bangradar arguments:
 * seg_id      : Segment ID where radar should be injected.
 * is_chirp    : Is chirp radar or non chirp radar.
 * freq_offset : Frequency offset from center frequency.
 *
 * Type 1 (DFS_BANGRADAR_FOR_ALL_SUBCHANS): To add all subchans.
 * Type 2 (DFS_BANGRADAR_FOR_ALL_SUBCHANS_OF_SEGID): To add all
 *               subchans of given segment_id.
 * Type 3 (DFS_BANGRADAR_FOR_SPECIFIC_SUBCHANS): To add specific
 *               subchans based on the arguments.
 *
 * The arguments will already be filled in the indata structure
 * based on the type.
 * If an argument is not specified by user, it will be set to
 * default (0) in the indata already and correspondingly,
 * the type will change.
 */

int dfs_bang_radar(struct wlan_dfs *dfs, void *indata, uint32_t insize)
{
	struct dfs_bangradar_params *bangradar_params;
	int error = -EINVAL;

	if (!dfs) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return error;
	}

	if (insize < sizeof(struct dfs_bangradar_params) || !indata) {
		dfs_debug(dfs, WLAN_DEBUG_DFS1,
			  "insize = %d, expected = %zu bytes, indata = %pK",
			  insize,
			  sizeof(struct dfs_bangradar_params),
			  indata);
		return error;
	}
	bangradar_params = (struct dfs_bangradar_params *)indata;
	error = dfs_check_bangradar_sanity(dfs, bangradar_params);
	if (error != QDF_STATUS_SUCCESS)
		return error;
	dfs->dfs_bangradar_type = bangradar_params->bangradar_type;
	dfs->dfs_seg_id = bangradar_params->seg_id;
	dfs->dfs_is_chirp = bangradar_params->is_chirp;
	dfs->dfs_freq_offset = bangradar_params->freq_offset;

	if (dfs->dfs_is_offload_enabled) {
		error = dfs_fill_emulate_bang_radar_test(
				dfs,
				bangradar_params);
	} else {
		error = dfs_start_host_based_bangradar(dfs);
	}
	return error;
}
