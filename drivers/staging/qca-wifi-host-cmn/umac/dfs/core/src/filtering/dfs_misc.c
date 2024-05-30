/*
 * Copyright (c) 2016-2017,2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2002-2010, Atheros Communications Inc.
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
 * DOC: This file really does contain miscellaneous functions that didn't fit
 * in anywhere else.
 */

#include "../dfs.h"
#include "wlan_dfs_lmac_api.h"
#include "wlan_dfs_mlme_api.h"
#include "../dfs_internal.h"

/**
 * dfs_adjust_pri_per_chan_busy() - Calculates adjust_pri.
 * @ext_chan_busy: Extension channel PRI.
 * @pri_margin: Primary margin.
 *
 * Calculates the adjust_pri using ext_chan_busy, DFS_EXT_CHAN_LOADING_THRESH
 * and pri_margin.
 *
 * Return: adjust_pri.
 */
static int dfs_adjust_pri_per_chan_busy(int ext_chan_busy, int pri_margin)
{
	int adjust_pri = 0;

	if (ext_chan_busy > DFS_EXT_CHAN_LOADING_THRESH) {
		adjust_pri = ((ext_chan_busy - DFS_EXT_CHAN_LOADING_THRESH) *
				(pri_margin));
		adjust_pri /= 100;
	}

	return adjust_pri;
}

/**
 * dfs_adjust_thresh_per_chan_busy() - Calculates adjust_thresh.
 * @ext_chan_busy: Extension channel PRI.
 * @thresh: Threshold value.
 *
 * Calculates the adjust_thresh using ext_chan_busy, DFS_EXT_CHAN_LOADING_THRESH
 * and thresh.
 *
 * Return: adjust_thresh.
 */
static int dfs_adjust_thresh_per_chan_busy(int ext_chan_busy, int thresh)
{
	int adjust_thresh = 0;

	if (ext_chan_busy > DFS_EXT_CHAN_LOADING_THRESH) {
		adjust_thresh = ((ext_chan_busy - DFS_EXT_CHAN_LOADING_THRESH) *
				thresh);
		adjust_thresh /= 100;
	}

	return adjust_thresh;
}

/**
 * dfs_get_cached_ext_chan_busy() - Get cached ext chan busy.
 * @dfs: Pointer to wlan_dfs structure.
 * @ext_chan_busy: Extension channel PRI.
 */
static inline void dfs_get_cached_ext_chan_busy(
		struct wlan_dfs *dfs,
		int *ext_chan_busy)
{
	*ext_chan_busy = 0;
	/* Check to see if the cached value of ext_chan_busy can be used. */

	if (dfs->dfs_rinfo.dfs_ext_chan_busy &&
			(dfs->dfs_rinfo.rn_lastfull_ts <
			 dfs->dfs_rinfo.ext_chan_busy_ts)) {
		*ext_chan_busy = dfs->dfs_rinfo.dfs_ext_chan_busy;
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"Use cached copy of ext_chan_busy extchanbusy=%d rn_lastfull_ts=%llu ext_chan_busy_ts=%llu",
				*ext_chan_busy,
				(uint64_t)dfs->dfs_rinfo.rn_lastfull_ts,
				(uint64_t)dfs->dfs_rinfo.ext_chan_busy_ts);
	}
}

#if defined(WLAN_DFS_PARTIAL_OFFLOAD)
int dfs_get_pri_margin(struct wlan_dfs *dfs,
		int is_extchan_detect,
		int is_fixed_pattern)
{
	int adjust_pri = 0, ext_chan_busy = 0;
	int pri_margin;

	if (is_fixed_pattern)
		pri_margin = DFS_DEFAULT_FIXEDPATTERN_PRI_MARGIN;
	else
		pri_margin = DFS_DEFAULT_PRI_MARGIN;

	if (WLAN_IS_CHAN_11N_HT40(dfs->dfs_curchan)) {
		ext_chan_busy = lmac_get_ext_busy(dfs->dfs_pdev_obj);
		if (ext_chan_busy >= 0) {
			dfs->dfs_rinfo.ext_chan_busy_ts =
				lmac_get_tsf64(dfs->dfs_pdev_obj);
			dfs->dfs_rinfo.dfs_ext_chan_busy = ext_chan_busy;
		} else {
			dfs_get_cached_ext_chan_busy(dfs, &ext_chan_busy);
		}
		adjust_pri = dfs_adjust_pri_per_chan_busy(ext_chan_busy,
			pri_margin);
		pri_margin -= adjust_pri;
	}

	return pri_margin;
}
#endif

int dfs_get_filter_threshold(struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		int is_extchan_detect)
{
	int ext_chan_busy = 0;
	int thresh, adjust_thresh = 0;

	thresh = rf->rf_threshold;

	if (WLAN_IS_CHAN_11N_HT40(dfs->dfs_curchan)) {
		ext_chan_busy = lmac_get_ext_busy(dfs->dfs_pdev_obj);
		if (ext_chan_busy >= 0) {
			dfs->dfs_rinfo.ext_chan_busy_ts =
				lmac_get_tsf64(dfs->dfs_pdev_obj);
			dfs->dfs_rinfo.dfs_ext_chan_busy = ext_chan_busy;
		} else {
			dfs_get_cached_ext_chan_busy(dfs, &ext_chan_busy);
		}

		adjust_thresh =
			dfs_adjust_thresh_per_chan_busy(ext_chan_busy, thresh);
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
			" filterID=%d extchanbusy=%d adjust_thresh=%d",
			rf->rf_pulseid, ext_chan_busy, adjust_thresh);

		thresh += adjust_thresh;
	}

	return thresh;
}

uint32_t dfs_round(int32_t val)
{
	uint32_t ival, rem;

	if (val < 0)
		return 0;
	ival = val/100;
	rem = val - (ival * 100);
	if (rem < 50)
		return ival;
	else
		return ival + 1;
}
