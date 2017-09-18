/*
 * Copyright (c) 2013, 2016-2017 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains initialization functions and functions that reset
 * internal data structures.
 */

#include "../dfs.h"
#include "wlan_dfs_lmac_api.h"

/**
 * dfs_reset_filtertype() - Reset filtertype.
 * @ft: Pointer to dfs_filtertype structure.
 */
static inline void dfs_reset_filtertype(
		struct dfs_filtertype *ft)
{
	int j;
	struct dfs_filter *rf;
	struct dfs_delayline *dl;

	for (j = 0; j < ft->ft_numfilters; j++) {
		rf = &(ft->ft_filters[j]);
		dl = &(rf->rf_dl);
		if (dl != NULL) {
			qdf_mem_zero(dl, sizeof(*dl));
			dl->dl_lastelem = (0xFFFFFFFF) & DFS_MAX_DL_MASK;
		}
	}
}

void dfs_reset_alldelaylines(struct wlan_dfs *dfs)
{
	struct dfs_filtertype *ft = NULL;
	struct dfs_pulseline *pl;
	int i;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
				"%s: sc_dfs is NULL\n", __func__);
		return;
	}
	pl = dfs->pulses;

	if (pl == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS, "%s: pl==NULL, dfs=%pK\n",
				__func__, dfs);
		return;
	}

	if (dfs->dfs_b5radars == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS, "%s: pl==NULL, b5radars=%pK\n",
				__func__, dfs->dfs_b5radars);
		return;
	}

	/* Reset the pulse log. */
	pl->pl_firstelem = pl->pl_numelems = 0;
	pl->pl_lastelem = DFS_MAX_PULSE_BUFFER_MASK;

	for (i = 0; i < DFS_MAX_RADAR_TYPES; i++) {
		if (dfs->dfs_radarf[i] != NULL) {
			ft = dfs->dfs_radarf[i];
			dfs_reset_filtertype(ft);
		}
	}

	for (i = 0; i < dfs->dfs_rinfo.rn_numbin5radars; i++) {
		qdf_mem_zero(&(dfs->dfs_b5radars[i].br_elems[0]),
				sizeof(struct dfs_bin5elem) * DFS_MAX_B5_SIZE);
		dfs->dfs_b5radars[i].br_firstelem = 0;
		dfs->dfs_b5radars[i].br_numelems = 0;
		dfs->dfs_b5radars[i].br_lastelem =
			(0xFFFFFFFF) & DFS_MAX_B5_MASK;
	}
}

void dfs_reset_delayline(struct dfs_delayline *dl)
{
	qdf_mem_zero(&(dl->dl_elems[0]), sizeof(dl->dl_elems));
	dl->dl_lastelem = (0xFFFFFFFF) & DFS_MAX_DL_MASK;
}

void dfs_reset_filter_delaylines(struct dfs_filtertype *dft)
{
	struct dfs_filter *df;
	int i;

	for (i = 0; i < DFS_MAX_NUM_RADAR_FILTERS; i++) {
		df = &dft->ft_filters[i];
		dfs_reset_delayline(&(df->rf_dl));
	}
}

void dfs_reset_radarq(struct wlan_dfs *dfs)
{
	struct dfs_event *event;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS, "%s: sc_dfs is NULL\n",
				__func__);
		return;
	}

	WLAN_DFSQ_LOCK(dfs);
	WLAN_DFSEVENTQ_LOCK(dfs);
	while (!STAILQ_EMPTY(&(dfs->dfs_radarq))) {
		event = STAILQ_FIRST(&(dfs->dfs_radarq));
		STAILQ_REMOVE_HEAD(&(dfs->dfs_radarq), re_list);
		qdf_mem_zero(event, sizeof(struct dfs_event));
		STAILQ_INSERT_TAIL(&(dfs->dfs_eventq), event, re_list);
	}
	WLAN_DFSEVENTQ_UNLOCK(dfs);
	WLAN_DFSQ_UNLOCK(dfs);
}

/**
 * dfs_fill_ft_index_table() - DFS fill ft index table.
 * @dfs: Pointer to wlan_dfs structure.
 * @i: Duration used as an index.
 *
 * Return: 1 if too many overlapping radar filters else 0.
 */
static inline bool dfs_fill_ft_index_table(
		struct wlan_dfs *dfs,
		int i)
{
	uint32_t stop = 0, tableindex = 0;

	while ((tableindex < DFS_MAX_RADAR_OVERLAP) && (!stop)) {
		if ((dfs->dfs_ftindextable[i])[tableindex] == -1)
			stop = 1;
		else
			tableindex++;
	}

	if (stop) {
		(dfs->dfs_ftindextable[i])[tableindex] =
			(int8_t)(dfs->dfs_rinfo.rn_ftindex);
	} else {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
				"%s: Too many overlapping radar filters\n",
				__func__);
		return 1;
	}

	return 0;
}

/**
 * dfs_fill_filter_type() - DFS fill filter type.
 * @dfs: Pointer to wlan_dfs structure.
 * @ft: Double pointer to dfs_filtertype structure.
 * @dfs_radars: Pointer to dfs_pulse structure.
 * @min_rssithresh: Minimum RSSI threshold.
 * @max_pulsedur: Maximum RSSI threshold.
 * @p: Index to dfs_pulse structure.
 *
 * Return: 1 if too many overlapping radar filters else 0.
 */
static inline bool dfs_fill_filter_type(
		struct wlan_dfs *dfs,
		struct dfs_filtertype **ft,
		struct dfs_pulse *dfs_radars,
		int32_t *min_rssithresh,
		uint32_t *max_pulsedur,
		int p)
{
	int i;

	/* No filter of the appropriate dur was found. */
	if ((dfs->dfs_rinfo.rn_ftindex + 1) > DFS_MAX_RADAR_TYPES) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
				"%s: Too many filter types\n", __func__);
		return 1;
	}
	(*ft) = dfs->dfs_radarf[dfs->dfs_rinfo.rn_ftindex];
	(*ft)->ft_numfilters = 0;
	(*ft)->ft_numpulses = dfs_radars[p].rp_numpulses;
	(*ft)->ft_patterntype = dfs_radars[p].rp_patterntype;
	(*ft)->ft_mindur = dfs_radars[p].rp_mindur;
	(*ft)->ft_maxdur = dfs_radars[p].rp_maxdur;
	(*ft)->ft_filterdur = dfs_radars[p].rp_pulsedur;
	(*ft)->ft_rssithresh = dfs_radars[p].rp_rssithresh;
	(*ft)->ft_rssimargin = dfs_radars[p].rp_rssimargin;
	(*ft)->ft_minpri = 1000000;

	if ((*ft)->ft_rssithresh < *min_rssithresh)
		*min_rssithresh = (*ft)->ft_rssithresh;

	if ((*ft)->ft_maxdur > *max_pulsedur)
		*max_pulsedur = (*ft)->ft_maxdur;

	for (i = (*ft)->ft_mindur; i <= (*ft)->ft_maxdur; i++) {
		if (dfs_fill_ft_index_table(dfs, i))
			return 1;
	}

	dfs->dfs_rinfo.rn_ftindex++;

	return 0;
}

int dfs_init_radar_filters(struct wlan_dfs *dfs,
		struct wlan_dfs_radar_tab_info *radar_info)
{
	struct dfs_filtertype *ft = NULL;
	struct dfs_filter *rf = NULL;
	struct dfs_pulse *dfs_radars;
	struct dfs_bin5pulse *b5pulses = NULL;
	uint32_t T, Tmax;
	int32_t min_rssithresh = DFS_MAX_RSSI_VALUE;
	uint32_t max_pulsedur = 0;
	int numpulses, p, n, i;
	int numradars = 0, numb5radars = 0;
	int retval;

	if (dfs == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS, "dfs is NULL %s", __func__);
		return 1;
	}

	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
			"%s: dfsdomain=%d, numradars=%d, numb5radars=%d\n",
			__func__, radar_info->dfsdomain,
			radar_info->numradars, radar_info->numb5radars);

	/* Clear up the dfs domain flag first. */
	dfs->wlan_dfs_isdfsregdomain = 0;

	/*
	 * If radar_info is NULL or dfsdomain is NULL, treat the
	 * rest of the radar configuration as suspect.
	 */
	if (radar_info == NULL || radar_info->dfsdomain == 0) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS, "%s: Unknown dfs domain %d\n",
				__func__, dfs->dfsdomain);
		/* Disable radar detection since we don't have a radar domain.*/
		dfs->dfs_proc_phyerr &= ~DFS_RADAR_EN;
		dfs->dfs_proc_phyerr &= ~DFS_SECOND_SEGMENT_RADAR_EN;
		return 0;
	}

	dfs->dfsdomain = radar_info->dfsdomain;
	dfs_radars = radar_info->dfs_radars;
	numradars = radar_info->numradars;
	b5pulses = radar_info->b5pulses;
	numb5radars = radar_info->numb5radars;

	dfs->dfs_defaultparams = radar_info->dfs_defaultparams;

	dfs->wlan_dfs_isdfsregdomain = 1;
	dfs->dfs_rinfo.rn_ftindex = 0;
	/* Clear filter type table. */
	for (n = 0; n < 256; n++) {
		for (i = 0; i < DFS_MAX_RADAR_OVERLAP; i++)
			(dfs->dfs_ftindextable[n])[i] = -1;
	}

	/* Now, initialize the radar filters. */
	for (p = 0; p < numradars; p++) {
		ft = NULL;
		for (n = 0; n < dfs->dfs_rinfo.rn_ftindex; n++) {
			if ((dfs_radars[p].rp_pulsedur ==
				    dfs->dfs_radarf[n]->ft_filterdur) &&
				(dfs_radars[p].rp_numpulses ==
				 dfs->dfs_radarf[n]->ft_numpulses) &&
				(dfs_radars[p].rp_mindur ==
				 dfs->dfs_radarf[n]->ft_mindur) &&
				(dfs_radars[p].rp_maxdur ==
				 dfs->dfs_radarf[n]->ft_maxdur)) {
				ft = dfs->dfs_radarf[n];
				break;
			}
		}

		if (ft == NULL) {
			retval = dfs_fill_filter_type(dfs, &ft, dfs_radars,
					&min_rssithresh, &max_pulsedur, p);
			if (retval == 1)
				goto bad4;
		}

		rf = &(ft->ft_filters[ft->ft_numfilters++]);
		dfs_reset_delayline(&rf->rf_dl);
		numpulses = dfs_radars[p].rp_numpulses;

		rf->rf_numpulses = numpulses;
		rf->rf_patterntype = dfs_radars[p].rp_patterntype;
		rf->rf_pulseid = dfs_radars[p].rp_pulseid;
		rf->rf_mindur = dfs_radars[p].rp_mindur;
		rf->rf_maxdur = dfs_radars[p].rp_maxdur;
		rf->rf_numpulses = dfs_radars[p].rp_numpulses;
		rf->rf_ignore_pri_window = dfs_radars[p].rp_ignore_pri_window;
		T = (100000000 / dfs_radars[p].rp_max_pulsefreq) -
			100 * (dfs_radars[p].rp_meanoffset);
		rf->rf_minpri = dfs_round((int32_t)T -
				(100 * (dfs_radars[p].rp_pulsevar)));
		Tmax = (100000000 / dfs_radars[p].rp_pulsefreq) -
			100 * (dfs_radars[p].rp_meanoffset);
		rf->rf_maxpri = dfs_round((int32_t)Tmax +
				(100 * (dfs_radars[p].rp_pulsevar)));

		if (rf->rf_minpri < ft->ft_minpri)
			ft->ft_minpri = rf->rf_minpri;

		rf->rf_fixed_pri_radar_pulse = (
				dfs_radars[p].rp_max_pulsefreq ==
				dfs_radars[p].rp_pulsefreq) ?  1 : 0;
		rf->rf_threshold = dfs_radars[p].rp_threshold;
		rf->rf_filterlen = rf->rf_maxpri * rf->rf_numpulses;

		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
				"minprf = %d maxprf = %d pulsevar = %d thresh=%d\n",
				dfs_radars[p].rp_pulsefreq,
				dfs_radars[p].rp_max_pulsefreq,
				dfs_radars[p].rp_pulsevar,
				rf->rf_threshold);

		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
				"minpri = %d maxpri = %d filterlen = %d filterID = %d\n",
				rf->rf_minpri, rf->rf_maxpri,
				rf->rf_filterlen, rf->rf_pulseid);
	}

	dfs_print_filters(dfs);

	dfs->dfs_rinfo.rn_numbin5radars  = numb5radars;
	if (dfs->dfs_b5radars != NULL)
		qdf_mem_free(dfs->dfs_b5radars);

	dfs->dfs_b5radars = (struct dfs_bin5radars *)qdf_mem_malloc(
			numb5radars * sizeof(struct dfs_bin5radars));
	if (dfs->dfs_b5radars == NULL) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
				"%s: cannot allocate memory for bin5 radars\n",
				__func__);
		goto bad4;
	}
	for (n = 0; n < numb5radars; n++) {
		dfs->dfs_b5radars[n].br_pulse = b5pulses[n];
		dfs->dfs_b5radars[n].br_pulse.b5_timewindow *= 1000000;
		if (dfs->dfs_b5radars[n].br_pulse.b5_rssithresh <
				min_rssithresh)
			min_rssithresh =
				dfs->dfs_b5radars[n].br_pulse.b5_rssithresh;

		if (dfs->dfs_b5radars[n].br_pulse.b5_maxdur > max_pulsedur)
			max_pulsedur = dfs->dfs_b5radars[n].br_pulse.b5_maxdur;
	}
	dfs_reset_alldelaylines(dfs);
	dfs_reset_radarq(dfs);
	dfs->dfs_curchan_radindex = -1;
	dfs->dfs_extchan_radindex = -1;
	dfs->dfs_rinfo.rn_minrssithresh = min_rssithresh;

	/* Convert durations to TSF ticks. */
	dfs->dfs_rinfo.rn_maxpulsedur =
		dfs_round((int32_t)((max_pulsedur * 100/80) * 100));
	/*
	 * Relax the max pulse duration a little bit due to inaccuracy
	 * caused by chirping.
	 */
	dfs->dfs_rinfo.rn_maxpulsedur = dfs->dfs_rinfo.rn_maxpulsedur + 20;

	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS, "DFS min filter rssiThresh = %d\n",
			min_rssithresh);

	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS, "DFS max pulse dur = %d ticks\n",
			dfs->dfs_rinfo.rn_maxpulsedur);

	return 0;

bad4:
	return 1;
}

void dfs_clear_stats(struct wlan_dfs *dfs)
{
	if (dfs == NULL)
		return;

	qdf_mem_zero(&dfs->wlan_dfs_stats, sizeof(struct dfs_stats));
	dfs->wlan_dfs_stats.last_reset_tstamp =
	    lmac_get_tsf64(dfs->dfs_pdev_obj);
}
