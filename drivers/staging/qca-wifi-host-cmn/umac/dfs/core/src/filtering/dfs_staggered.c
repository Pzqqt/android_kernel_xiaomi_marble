/*
 * Copyright (c) 2013, 2016-2018 The Linux Foundation. All rights reserved.
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
 * DOC: ETSI 1.5.1 introduced new waveforms which use staggered PRIs within
 * the same waveform. This file contains the detection implementation for
 * these specific types of radars. This logic is different from the other
 * detection because it must detect waveforms that may have 2 or more
 * different PRIs (pulse repetition intervals).
 */

#include "../dfs.h"
#include "../dfs_process_radar_found_ind.h"

/**
 * dfs_is_pri_multiple() - Is PRI is multiple.
 * @sample_pri: Sample PRI.
 * @refpri: Reference PRI.
 */
static int dfs_is_pri_multiple(uint32_t sample_pri, uint32_t refpri)
{
#define MAX_ALLOWED_MISSED 3
	int i;

	if (sample_pri < refpri || (!refpri))
		return 0;

	for (i = 1; i <= MAX_ALLOWED_MISSED; i++) {
		if ((sample_pri%(i*refpri) <= 5))
			return 1;
	}

	return 0;
#undef MAX_ALLOWED_MISSED
}

/**
 * dfs_is_unique_pri() - Check for the unique PRI.
 * @highestpri: Highest PRI.
 * @midpri: MID PRI.
 * @lowestpri: Lowest PRI.
 * @refpri: Reference PRI.
 */
static int dfs_is_unique_pri(uint32_t highestpri, uint32_t midpri,
		uint32_t lowestpri, uint32_t refpri)
{
#define DFS_STAGGERED_PRI_MARGIN_MIN  20
#define DFS_STAGGERED_PRI_MARGIN_MAX  400
	if ((DFS_DIFF(lowestpri, refpri) >= DFS_STAGGERED_PRI_MARGIN_MIN) &&
		(DFS_DIFF(midpri, refpri) >= DFS_STAGGERED_PRI_MARGIN_MIN) &&
		(DFS_DIFF(highestpri, refpri) >= DFS_STAGGERED_PRI_MARGIN_MIN)
	   )
		return 1;

	if ((dfs_is_pri_multiple(refpri, highestpri)) ||
			(dfs_is_pri_multiple(refpri, lowestpri)) ||
			(dfs_is_pri_multiple(refpri, midpri)))
		return 0;
#undef DFS_STAGGERED_PRI_MARGIN_MIN
#undef DFS_STAGGERED_PRI_MARGIN_MAX

	return 0;
}

int dfs_staggered_check(struct wlan_dfs *dfs, struct dfs_filter *rf,
		uint32_t deltaT, uint32_t width)
{
	uint32_t refpri, refdur, searchpri = 0, deltapri;
	uint32_t n, i, primargin, durmargin;
	int score[DFS_MAX_DL_SIZE], delayindex, dindex, found = 0;
	struct dfs_delayline *dl;
	uint32_t scoreindex, lowpriindex = 0, lowpri = 0xffff;
	int  higherthan, lowerthan, numscores;
	int numpulseshigh = 0, numpulsesmid = 0, numpulsestemp = 0;
	uint32_t lowestscore = 0, lowestscoreindex = 0, lowestpri = 0;
	uint32_t midscore = 0, midscoreindex = 0, midpri = 0;
	uint32_t highestscore = 0, highestscoreindex = 0, highestpri = 0;

	dl = &rf->rf_dl;
	if (dl->dl_numelems < (rf->rf_threshold-1)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"numelems %d < threshold for filter %d",
				dl->dl_numelems,
				rf->rf_pulseid);
		return 0;
	}
	if (deltaT > rf->rf_filterlen) {
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"numelems %d < threshold for filter %d",
				dl->dl_numelems,
				rf->rf_pulseid);
		return 0;
	}
	primargin = 6;
	if (rf->rf_maxdur < 10)
		durmargin = 4;
	else
		durmargin = 6;

	qdf_mem_zero(score, sizeof(int)*DFS_MAX_DL_SIZE);
	/* Find out the lowest pri */
	for (n = 0; n < dl->dl_numelems; n++) {
		delayindex = (dl->dl_firstelem + n) & DFS_MAX_DL_MASK;
		refpri = dl->dl_elems[delayindex].de_time;
		if (refpri == 0) {
			continue;
		} else if (refpri < lowpri) {
			lowpri = dl->dl_elems[delayindex].de_time;
			lowpriindex = n;
		}
	}

	/* Find out the each delay element's pri score */
	for (n = 0; n < dl->dl_numelems; n++) {
		delayindex = (dl->dl_firstelem + n) & DFS_MAX_DL_MASK;
		refpri = dl->dl_elems[delayindex].de_time;
		if (refpri == 0)
			continue;

		if ((refpri > rf->rf_maxpri) || (refpri < rf->rf_minpri)) {
			score[n] = 0;
			continue;
		}

		for (i = 0; i < dl->dl_numelems; i++) {
			dindex = (dl->dl_firstelem + i) & DFS_MAX_DL_MASK;
			searchpri = dl->dl_elems[dindex].de_time;
			deltapri = DFS_DIFF(searchpri, refpri);
			if (deltapri < primargin)
				score[n]++;
		}
	}

	dfs->dfs_freq_offset = DFS_SIDX_TO_FREQ_OFFSET(
				       (dl->dl_min_sidx + dl->dl_max_sidx) / 2);

	for (n = 0; n < dl->dl_numelems; n++) {
		delayindex = (dl->dl_firstelem + n) & DFS_MAX_DL_MASK;
		refdur = dl->dl_elems[delayindex].de_time;
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"score[%d]=%d pri=%d",
				n, score[n], refdur);
	}

	/* Find out the 2 or 3 highest scorers */
	scoreindex = 0;
	highestscore = 0;
	highestscoreindex = 0;
	highestpri = 0; numscores = 0; lowestscore = 0;

	for (n = 0; n < dl->dl_numelems; n++) {
		higherthan = 0;
		lowerthan = 0;
		delayindex = (dl->dl_firstelem + n) & DFS_MAX_DL_MASK;
		refpri = dl->dl_elems[delayindex].de_time;

		if (!dfs_is_unique_pri(highestpri,
					midpri,
					lowestpri,
					refpri))
			continue;

		if (score[n] >= highestscore) {
			lowestscore = midscore;
			lowestpri = midpri;
			lowestscoreindex = midscoreindex;
			midscore = highestscore;
			midpri = highestpri;
			midscoreindex = highestscoreindex;
			highestscore = score[n];
			highestpri = refpri;
			highestscoreindex = n;
		} else if (score[n] >= midscore) {
			lowestscore = midscore;
			lowestpri = midpri;
			lowestscoreindex = midscoreindex;
			midscore = score[n];
			midpri = refpri;
			midscoreindex = n;
		} else if (score[n] >= lowestscore) {
			lowestscore = score[n];
			lowestpri = refpri;
			lowestscoreindex = n;
		}
	}

	if (midscore == 0)
		return 0;

	dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"FINAL highestscore=%d highestscoreindex = %d highestpri = %d",
			highestscore, highestscoreindex, highestpri);

	dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"FINAL lowestscore=%d lowestscoreindex=%d lowpri=%d",
			lowestscore, lowestscoreindex, lowestpri);

	dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"FINAL midscore=%d midscoreindex=%d midpri=%d",
			midscore, midscoreindex, midpri);

	delayindex = (dl->dl_firstelem + highestscoreindex) & DFS_MAX_DL_MASK;
	refdur = dl->dl_elems[delayindex].de_dur;
	refpri = dl->dl_elems[delayindex].de_time;

	dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"highscoreindex=%d refdur=%d refpri=%d",
			highestscoreindex, refdur, refpri);

	numpulsestemp = dfs_bin_pri_check(dfs, rf, dl, highestscore, refpri,
			refdur, 0, highestpri);
	numpulseshigh = numpulsestemp;
	numpulsestemp = dfs_bin_pri_check(dfs, rf, dl, highestscore, refpri,
			refdur, 0, highestpri + midpri);
	if (numpulsestemp > numpulseshigh)
		numpulseshigh = numpulsestemp;

	numpulsestemp = dfs_bin_pri_check(dfs, rf, dl, highestscore, refpri,
			refdur, 0, highestpri + midpri + lowestpri);
	if (numpulsestemp > numpulseshigh)
		numpulseshigh = numpulsestemp;

	delayindex = (dl->dl_firstelem + midscoreindex) & DFS_MAX_DL_MASK;
	refdur = dl->dl_elems[delayindex].de_dur;
	refpri = dl->dl_elems[delayindex].de_time;
	dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"midscoreindex=%d refdur=%d refpri=%d",
			midscoreindex, refdur, refpri);

	numpulsestemp = dfs_bin_pri_check(dfs, rf, dl, midscore, refpri, refdur,
			0, midpri);
	numpulsesmid = numpulsestemp;
	numpulsestemp = dfs_bin_pri_check(dfs, rf, dl, midscore, refpri, refdur,
			0, highestpri + midpri);
	if (numpulsestemp > numpulsesmid)
		numpulsesmid = numpulsestemp;
	numpulsestemp = dfs_bin_pri_check(dfs, rf, dl, midscore, refpri, refdur,
			0, highestpri + midpri + lowestpri);
	if (numpulsestemp > numpulsesmid)
		numpulsesmid = numpulsestemp;

	dfs_debug(dfs, WLAN_DEBUG_DFS2,
			"numpulseshigh=%d, numpulsesmid=%d",
			numpulseshigh, numpulsesmid);

	if ((numpulseshigh >= rf->rf_threshold) &&
			(numpulsesmid >= rf->rf_threshold)) {
		found = 1;
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"MATCH filter=%u numpulseshigh=%u numpulsesmid= %u thresh=%u",
				rf->rf_pulseid, numpulseshigh,
				numpulsesmid, rf->rf_threshold);
	}

	return found;
}
