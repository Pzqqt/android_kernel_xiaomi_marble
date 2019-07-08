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
 * DOC: This file contains the dfs_fill_emulate_bang_radar_test()
 * which is used to send command to firmware to emulate RADAR found event.
 */

#include "../dfs_zero_cac.h"
#include "wlan_dfs_lmac_api.h"
#include "wlan_dfs_mlme_api.h"
#include "wlan_dfs_tgt_api.h"
#include "../dfs_internal.h"
#include "../dfs_full_offload.h"

#if defined(WLAN_DFS_FULL_OFFLOAD)
#define ADD_TO_32BYTE(_arg, _shift, _mask) (((_arg) & (_mask)) << (_shift))
int
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
				&dfs_unit_test) == QDF_STATUS_E_FAILURE) {
		return -EINVAL;
	}

	return 0;
}
#endif
