/*
 * Copyright (c) 2013, 2016-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2002-2010, Atheros Communications Inc.
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: It contains useful print functions that can be used for debug.
 * Add all debug related functionality into this file.
 */
#include "../dfs.h"
#include "wlan_dfs_lmac_api.h"

void dfs_print_delayline(struct wlan_dfs *dfs, struct dfs_delayline *dl)
{
	int i = 0, index;
	struct dfs_delayelem *de;

	index = dl->dl_firstelem;
	for (i = 0; i < dl->dl_numelems; i++) {
		de = &dl->dl_elems[index];
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"Elem %u: ts=%llu diff_ts=%u (0x%x) dur=%u, seg_id=%d sidx=%d delta_peak=%d psidx_diff=%d seq_num=%d",
				i, de->de_ts, de->de_time, de->de_time,
				de->de_dur, de->de_seg_id, de->de_sidx,
				de->de_delta_peak, de->de_psidx_diff,
				de->de_seq_num);

		index = (index + 1) & DFS_MAX_DL_MASK;
	}
}

void dfs_print_filter(struct wlan_dfs *dfs, struct dfs_filter *rf)
{
	dfs_debug(dfs, WLAN_DEBUG_DFS1,
		"filterID[%d] rf_numpulses=%u; rf->rf_minpri=%u; rf->rf_maxpri=%u; rf->rf_threshold=%u; rf->rf_filterlen=%u; rf->rf_mindur=%u; rf->rf_maxdur=%u",
		rf->rf_pulseid, rf->rf_numpulses, rf->rf_minpri, rf->rf_maxpri,
		rf->rf_threshold, rf->rf_filterlen, rf->rf_mindur,
		rf->rf_maxdur);
}

/**
 * dfs_print_filtertype() - Print the filtertype
 * @dfs: Pointer to wlan_dfs structure.
 * @ft:  Pointer to dfs_filtertype structure.
 */
static void dfs_print_filtertype(
		struct wlan_dfs *dfs,
		struct dfs_filtertype *ft)
{
	uint32_t j;
	struct dfs_filter *rf;

	for (j = 0; j < ft->ft_numfilters; j++) {
		rf = ft->ft_filters[j];
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"filter[%d] filterID = %d rf_numpulses=%u; rf->rf_minpri=%u; rf->rf_maxpri=%u; rf->rf_threshold=%u; rf->rf_filterlen=%u; rf->rf_mindur=%u; rf->rf_maxdur=%u",
				j, rf->rf_pulseid, rf->rf_numpulses,
				rf->rf_minpri, rf->rf_maxpri,
				rf->rf_threshold, rf->rf_filterlen,
				rf->rf_mindur, rf->rf_maxdur);
	}
}

void dfs_print_filters(struct wlan_dfs *dfs)
{
	struct dfs_filtertype *ft = NULL;
	uint8_t i;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	for (i = 0; i < DFS_MAX_RADAR_TYPES; i++) {
		if (dfs->dfs_radarf[i]) {
			ft = dfs->dfs_radarf[i];
			if ((ft->ft_numfilters > DFS_MAX_NUM_RADAR_FILTERS) ||
					(!ft->ft_numfilters)) {
				continue;
			}
			dfs_debug(dfs, WLAN_DEBUG_DFS2,
					"===========ft->ft_numfilters = %u===========",
					ft->ft_numfilters);
			dfs_print_filtertype(dfs, ft);
		}
	}
}
