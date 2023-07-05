/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
 * DOC: DFS specs specify various types of radars to be detected.
 * Each separate type is called a Bin and has different characteristics.
 * This file contains the functionality to look at a group of pulses and
 * to detect whether we have detected a valid radar waveform. To do that,
 * it must match the group against each different Bin's characteristics.
 */

#include "../dfs.h"
#include "../dfs_process_radar_found_ind.h"

/**
 * dfs_find_first_index_within_window() - Find first index within window
 * @pl: Pointer to dfs_pulseline structure.
 * @index: Index to dfs pulse elements.
 * @start_ts: Start timestamp.
 *
 * Return: Returns index.
 */
static inline uint32_t dfs_find_first_index_within_window(
		struct dfs_pulseline *pl,
		uint32_t index,
		uint64_t start_ts)
{
	uint16_t i;

	/* Find the index of first element in our window of interest. */
	for (i = 0; i < pl->pl_numelems; i++) {
		index = (index - 1) & DFS_MAX_PULSE_BUFFER_MASK;
		if (pl->pl_elems[index].p_time >= start_ts) {
			continue;
		} else {
			index = (index) & DFS_MAX_PULSE_BUFFER_MASK;
			break;
		}
	}

	return index;
}

/**
 * dfs_ts_within_window() - Calculate pulses for timestamp within window
 * @dfs: Pointer to wlan_dfs structure.
 * @pl: Pointer to dfs_pulseline structure.
 * @index: Index to dfs pulse elements.
 * @dur: Pulse duration/width
 * @numpulses: Number of pulses
 *
 * Return: Returns 1 if pulse count is incremented else returns 0.
 */
static inline bool dfs_ts_within_window(
		struct wlan_dfs *dfs,
		struct dfs_pulseline *pl,
		uint32_t *index,
		uint32_t dur,
		int *numpulses)
{
	uint32_t deltadur;

	deltadur = DFS_DIFF(pl->pl_elems[*index].p_dur, dur);
	if ((pl->pl_elems[*index].p_dur == 1) ||
			((dur != 1) && (deltadur <= 2))) {
		(*numpulses)++;
		dfs_debug(dfs, WLAN_DEBUG_DFS2, "numpulses %u", *numpulses);
		return 1;
	}

	return 0;
}

/**
 * dfs_ts_eq_prevts() - Calculate pulses for timestamp equals to prev event
 * @dfs: Pointer to wlan_dfs structure.
 * @pl: Pointer to dfs_pulseline structure.
 * @index: Index to dfs pulse elements.
 * @dur: Pulse duration/width
 * @numpulses: Number of pulses
 *
 * Return: Returns 1 if pulse count is incremented else returns 0.
 */
static inline bool dfs_ts_eq_prevts(
		struct wlan_dfs *dfs,
		struct dfs_pulseline *pl,
		uint64_t next_event_ts,
		uint64_t event_ts,
		uint32_t refpri,
		uint32_t *index,
		uint32_t dur,
		int *numpulses)

{
	uint32_t deltadur;

	if (((next_event_ts - event_ts) > refpri) ||
			((next_event_ts - event_ts) == 0)) {
		deltadur = DFS_DIFF(pl->pl_elems[*index].p_dur, dur);
		if ((pl->pl_elems[*index].p_dur == 1) ||
				((pl->pl_elems[*index].p_dur != 1) &&
				 (deltadur <= 2))) {
			(*numpulses)++;
			dfs_debug(dfs, WLAN_DEBUG_DFS2,
					"zero PRI: numpulses %u", *numpulses);
			return 1;
		}
	}

	return 0;
}

/**
 * dfs_pulses_within_window() - Calculate pulses within window
 * @dfs: Pointer to wlan_dfs structure.
 * @window_start: Start of the window.
 * @window_end: End of the window.
 * @index: Index to dfs pulse elements.
 * @dur: Pulse duration/width.
 * @refpri: reference PRI.
 *
 * Return: Returns 1 if pulse count is incremented else returns 0.
 */
static inline int dfs_pulses_within_window(
		struct wlan_dfs *dfs,
		uint64_t window_start,
		uint64_t window_end,
		uint32_t *index,
		uint32_t dur,
		uint32_t refpri)
{
	int numpulses = 0;
	uint32_t i;
	struct dfs_pulseline *pl = dfs->pulses;
	uint64_t event_ts, prev_event_ts, next_event_ts;
	uint32_t next_index;

	for (i = 0; i < pl->pl_numelems; i++) {
		prev_event_ts = pl->pl_elems[*index].p_time;
		*index = (*index+1) & DFS_MAX_PULSE_BUFFER_MASK;
		event_ts = pl->pl_elems[*index].p_time;
		next_index = (*index+1) & DFS_MAX_PULSE_BUFFER_MASK;
		next_event_ts = pl->pl_elems[next_index].p_time;
		dfs_debug(dfs, WLAN_DEBUG_DFS2, "ts %u",
				(uint32_t)event_ts);

		if ((event_ts <= window_end) && (event_ts >= window_start)) {
			if (dfs_ts_within_window(dfs, pl, index, dur,
					&numpulses))
				break;
		} else if (event_ts > window_end) {
			*index = (*index-1) & DFS_MAX_PULSE_BUFFER_MASK;
			break;
		} else if (event_ts == prev_event_ts) {
			if (dfs_ts_eq_prevts(dfs, pl, next_event_ts, event_ts,
					refpri, index, dur, &numpulses))
				break;
		}
		if (dfs->dfs_min_sidx > pl->pl_elems[*index].p_sidx)
			dfs->dfs_min_sidx = pl->pl_elems[*index].p_sidx;

		if (dfs->dfs_max_sidx < pl->pl_elems[*index].p_sidx)
			dfs->dfs_max_sidx = pl->pl_elems[*index].p_sidx;
	}

	dfs->dfs_freq_offset =
		DFS_SIDX_TO_FREQ_OFFSET((dfs->dfs_min_sidx +
					 dfs->dfs_min_sidx) / 2);
	return numpulses;
}

/**
 * dfs_count_pulses() - Count pulses
 * @dfs: Pointer to wlan_dfs structure.
 * @rf:  Pointer to dfs_filter structure.
 * @dur: Pulse duration/width.
 * @ext_chan_flag : Ext channel flag.
 * @primargin: Primary margin.
 * @index: Index to dfs pulse elements.
 * @refpri: reference PRI.
 * @start_ts: Start timestamp.
 *
 * Return: Returns number of pulses within window.
 */
static inline int dfs_count_pulses(
		struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		uint32_t dur,
		int ext_chan_flag,
		int primargin,
		uint32_t index,
		uint32_t refpri,
		uint64_t start_ts)
{
	uint32_t n;
	int numpulses = 0;
	uint64_t window_start, window_end;

	for (n = 0; n <= rf->rf_numpulses; n++) {
		window_start = (start_ts + (refpri*n))-(primargin+n);
		window_end = window_start + 2*(primargin+n);
		dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"window_start %u window_end %u",
				(uint32_t)window_start, (uint32_t)window_end);
		numpulses += dfs_pulses_within_window(dfs, window_start,
				window_end, &index, dur, refpri);
	}

	return numpulses;
}

/**
 * dfs_bin_fixedpattern_check() - Fixed pattern check
 * @dfs: Pointer to wlan_dfs structure.
 * @rf:  Pointer to dfs_filter structure.
 * @dur: Pulse duration/width.
 * @ext_chan_flag : Ext channel flag.
 */
static int  dfs_bin_fixedpattern_check(
		struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		uint32_t dur,
		int ext_chan_flag)
{
	struct dfs_pulseline *pl = dfs->pulses;
	int primargin, numpulses, fil_thresh;
	uint64_t start_ts, end_ts;
	uint32_t last_index, first_index;
	uint32_t refpri;

	refpri = (rf->rf_minpri + rf->rf_maxpri)/2;
	last_index = pl->pl_lastelem;
	end_ts = pl->pl_elems[last_index].p_time;
	start_ts = end_ts - (refpri*rf->rf_numpulses);

	dfs_debug(dfs, WLAN_DEBUG_DFS3,
		"lastelem ts=%llu start_ts=%llu, end_ts=%llu",
		(unsigned long long)pl->pl_elems[last_index].p_time,
		(unsigned long long)start_ts,
		(unsigned long long) end_ts);

	first_index = dfs_find_first_index_within_window(pl, last_index,
			start_ts);

	/* For fixed pattern types, rf->rf_patterntype=1. */
	primargin = dfs_get_pri_margin(dfs, ext_chan_flag,
			(rf->rf_patterntype == 1));

	numpulses = dfs_count_pulses(dfs, rf, dur, ext_chan_flag, primargin,
			first_index, refpri, start_ts);

	fil_thresh = dfs_get_filter_threshold(dfs, rf, ext_chan_flag);

	if (numpulses >= fil_thresh) {
		dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"FOUND filterID=%u numpulses=%d unadj thresh=%d",
			 rf->rf_pulseid, numpulses, rf->rf_threshold);
		return 1;
	} else {
		return 0;
	}
}

void dfs_add_pulse(
		struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		struct dfs_event *re,
		uint32_t deltaT,
		uint64_t this_ts)
{
	uint32_t index, n, window;
	struct dfs_delayline *dl;

	dl = &rf->rf_dl;
	/* Circular buffer of size 2^n */
	index = (dl->dl_lastelem + 1) & DFS_MAX_DL_MASK;
	if ((dl->dl_numelems) == DFS_MAX_DL_SIZE)
		dl->dl_firstelem = (dl->dl_firstelem + 1) & DFS_MAX_DL_MASK;
	else
		dl->dl_numelems++;
	dl->dl_lastelem = index;
	dl->dl_elems[index].de_time = deltaT;
	dl->dl_elems[index].de_ts = this_ts;
	window = deltaT;
	dl->dl_elems[index].de_dur = re->re_dur;
	dl->dl_elems[index].de_rssi = re->re_rssi;
	dl->dl_elems[index].de_seg_id = re->re_seg_id;
	dl->dl_elems[index].de_sidx = re->re_sidx;
	dl->dl_elems[index].de_delta_peak = re->re_delta_peak;
	dl->dl_elems[index].de_psidx_diff = re->re_psidx_diff;
	dl->dl_elems[index].de_seq_num = dfs->dfs_seq_num;

	dfs_debug(dfs, WLAN_DEBUG_DFS2,
		"adding: filter id %d, dur=%d, rssi=%d, ts=%llu",
		 rf->rf_pulseid, re->re_dur,
		re->re_rssi, (unsigned long long int)this_ts);

	for (n = 0; n < dl->dl_numelems-1; n++) {
		index = (index-1) & DFS_MAX_DL_MASK;
		/*
		 * Calculate window based on full time stamp instead of deltaT
		 * deltaT (de_time) may result in incorrect window value
		 */
		window = (uint32_t) (this_ts - dl->dl_elems[index].de_ts);

		if (window > rf->rf_filterlen) {
			dl->dl_firstelem = (index+1) & DFS_MAX_DL_MASK;
			dl->dl_numelems = n+1;
		}
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS2, "dl firstElem = %d  lastElem = %d",
			dl->dl_firstelem, dl->dl_lastelem);
}

/**
 * dfs_find_lowestpri() - Find lowest PRI
 * @dl: Pointer to dfs delayline.
 * @lowpriindex: Low PRI index.
 * @lowpri: Low PRI
 */
static inline void dfs_find_lowestpri(
	struct dfs_delayline *dl,
	uint32_t *lowpriindex,
	uint32_t *lowpri)
{
	int delayindex;
	uint32_t refpri;
	uint32_t n;

	/* Find out the lowest pri. */
	for (n = 0; n < dl->dl_numelems; n++) {
		delayindex = (dl->dl_firstelem + n) & DFS_MAX_DL_MASK;
		refpri = dl->dl_elems[delayindex].de_time;
		if (refpri == 0) {
			continue;
		} else if (refpri < *lowpri) {
			*lowpri = dl->dl_elems[delayindex].de_time;
			*lowpriindex = n;
		}
	}
}

/**
 * dfs_calculate_score() - Calculate score for the score index
 * if PRI match is found
 * @dl: Pointer to dfs delayline.
 * @rf: Pointer to dfs_filter structure.
 * @score: score array.
 * @refpri: reference PRI.
 * @primargin: PRI margin.
 * @score_index: Score index.
 */
static inline void dfs_calculate_score(
	struct dfs_delayline *dl,
	struct dfs_filter *rf,
	int *score,
	uint32_t refpri,
	uint32_t primargin,
	uint32_t score_index)
{
	int pri_match = 0;
	int dindex;
	uint32_t searchpri, deltapri, deltapri_2, deltapri_3;
	uint32_t i;

	for (i = 0; i < dl->dl_numelems; i++) {
		dindex = (dl->dl_firstelem + i) & DFS_MAX_DL_MASK;
		searchpri = dl->dl_elems[dindex].de_time;
		deltapri = DFS_DIFF(searchpri, refpri);
		deltapri_2 = DFS_DIFF(searchpri, 2*refpri);
		deltapri_3 = DFS_DIFF(searchpri, 3*refpri);
		if (rf->rf_ignore_pri_window == 2)
			pri_match = ((deltapri < primargin) ||
					(deltapri_2 < primargin) ||
					(deltapri_3 < primargin));
		else
			pri_match = (deltapri < primargin);

		if (pri_match)
			score[score_index]++;
	}
}

/**
 * dfs_find_priscores() - Find PRI score
 * @dl: Pointer to dfs delayline.
 * @rf: Pointer to dfs_filter structure.
 * @score: score array.
 * @primargin: PRI margin.
 */
static void dfs_find_priscores(
	struct dfs_delayline *dl,
	struct dfs_filter *rf,
	int *score,
	uint32_t primargin)
{
	int delayindex;
	uint32_t refpri;
	uint32_t n;

	qdf_mem_zero(score, sizeof(int)*DFS_MAX_DL_SIZE);

	for (n = 0; n < dl->dl_numelems; n++) {
		delayindex = (dl->dl_firstelem + n) & DFS_MAX_DL_MASK;
		refpri = dl->dl_elems[delayindex].de_time;
		if (refpri == 0)
			continue;
		if (refpri < rf->rf_maxpri) {
			/* Use only valid PRI range for high score. */
			dfs_calculate_score(dl, rf, score, refpri, primargin,
				n);
		} else {
			score[n] = 0;
		}

		if (score[n] > rf->rf_threshold) {
			/*
			 * We got the most possible candidate,
			 * no need to continue further.
			 */
			break;
		}
	}
}

/**
 * dfs_find_highscore() - Find PRI high score
 * @dl: Pointer to dfs delayline.
 * @score: score array.
 * @highscore: High score.
 * @highscoreindex: High score index.
 */
static inline void dfs_find_highscore(
		struct dfs_delayline *dl,
		int *score,
		uint32_t *highscore,
		uint32_t *highscoreindex)
{
	int delayindex, dindex;
	uint32_t n;

	*highscore = 0;
	*highscoreindex = 0;

	for (n = 0; n < dl->dl_numelems; n++) {
		if (score[n] > *highscore) {
			*highscore = score[n];
			*highscoreindex = n;
		} else if (score[n] == *highscore) {
			/*
			 * More than one pri has highscore take the least pri.
			 */
			delayindex = (dl->dl_firstelem + *highscoreindex) &
				DFS_MAX_DL_MASK;
			dindex = (dl->dl_firstelem + n) & DFS_MAX_DL_MASK;
			if (dl->dl_elems[dindex].de_time <=
					dl->dl_elems[delayindex].de_time) {
				*highscoreindex = n;
			}
		}
	}

	return;
}

/**
 * dfs_get_durmargin() - Find duration margin
 * @rf: Pointer to dfs_filter structure.
 * @durmargin: Duration margin
 */
static inline void dfs_get_durmargin(
		struct dfs_filter *rf,
		uint32_t *durmargin)
{
#define DUR_THRESH 10
#define LOW_MARGIN 4
#define HIGH_MARGIN 6

	if (rf->rf_maxdur < DUR_THRESH)
		*durmargin = LOW_MARGIN;
	else
		*durmargin = HIGH_MARGIN;

#undef DUR_THRESH
#undef LOW_MARGIN
#undef HIGH_MARGIN
}

/**
 * dfs_handle_fixedpattern() - Handle Fixed pattern radar
 * @dfs: Pointer to wlan_dfs structure.
 * @dl: Pointer to dfs delayline.
 * @rf: Pointer to dfs_filter structure.
 * @dur: Pulse duration/width
 * @ext_chan_flag : Ext channel flag.
 */
static inline int dfs_handle_fixedpattern(
		struct wlan_dfs *dfs,
		struct dfs_delayline *dl,
		struct dfs_filter *rf,
		uint32_t dur,
		int ext_chan_flag)
{
	int found = 0;

	found = dfs_bin_fixedpattern_check(dfs, rf, dur, ext_chan_flag);
	if (found)
		dl->dl_numelems = 0;

	return found;
}

/**
 * dfs_bin_basic_sanity() - Sanity check
 * @dl: Pointer to dfs delayline.
 * @rf: Pointer to dfs_filter structure.
 * @deltaT: Delta time.
 */
static inline int dfs_bin_basic_sanity(
		struct dfs_delayline *dl,
		struct dfs_filter *rf,
		uint32_t *deltaT)
{
	if (dl->dl_numelems < (rf->rf_threshold-1))
		return 0;

	if (*deltaT > rf->rf_filterlen)
		return 0;

	return 1;
}

/**
 * dfs_pick_lowpri() - Pick lowpri as refpri
 * @dfs: Pointer to wlan_dfs structure.
 * @dl: Pointer to dfs delayline.
 * @rf: Pointer to dfs_filter structure.
 * @lowpriindex: Low PRI index.
 * @scoreindex: score index.
 * @primargin: PRI margin.
 */
#ifdef DFS_PRI_MULTIPLIER
static inline void dfs_pick_lowpri(struct wlan_dfs *dfs,
				   struct dfs_delayline *dl,
				   struct dfs_filter *rf,
				   uint32_t lowpriindex,
				   uint32_t *scoreindex,
				   uint32_t primargin)
{
	uint32_t candidate_refpri, deltapri, lowpri;
	uint32_t dindex_candidate, dindex_lowpri;
	uint32_t i;

	dindex_candidate = (dl->dl_firstelem + *scoreindex) & DFS_MAX_DL_MASK;
	dindex_lowpri = (dl->dl_firstelem + lowpriindex) & DFS_MAX_DL_MASK;

	candidate_refpri = dl->dl_elems[dindex_candidate].de_time;
	lowpri = dl->dl_elems[dindex_lowpri].de_time;

	if (rf->rf_ignore_pri_window == 0 &&
	    candidate_refpri != lowpri) {
		for (i = 1; i <= dfs->dfs_pri_multiplier; i++) {
			deltapri = DFS_DIFF(candidate_refpri, i * lowpri);
			if (deltapri < primargin) {
				*scoreindex = lowpriindex;
				break;
			}
		}
	}
}
#else
static inline void dfs_pick_lowpri(struct wlan_dfs *dfs,
				   struct dfs_delayline *dl,
				   struct dfs_filter *rf,
				   uint32_t lowpriindex,
				   uint32_t *scoreindex,
				   uint32_t primargin)
{
}
#endif

/**
 * dfs_find_scoreindex() - Find score index
 * @rf: Pointer to dfs_filter structure.
 * @highscore: High score.
 * @lowpriindex: Low PRI index.
 * @highscoreindex: High score index.
 * @scoreindex: score index.
 */
static inline void dfs_find_scoreindex(
		struct dfs_filter *rf,
		uint32_t highscore,
		uint32_t lowpriindex,
		uint32_t highscoreindex,
		uint32_t *scoreindex)
{
	int lowprichk = 3;

	if (rf->rf_ignore_pri_window > 0)
		lowprichk = (rf->rf_threshold >> 1)+1;
	else
		lowprichk = 3;

	if (highscore < lowprichk)
		*scoreindex = lowpriindex;
	else
		*scoreindex = highscoreindex;
}

/**
 * dfs_find_refs() - Find reference values.
 * @dl: Pointer to dfs delayline.
 * @rf: Pointer to dfs_filter structure.
 * @scoreindex: score index.
 * @refdur: Duration value.
 * @refpri: Current "filter" time for start of pulse in usecs.
 */
static inline void dfs_find_refs(
		struct dfs_delayline *dl,
		struct dfs_filter *rf,
		uint32_t scoreindex,
		uint32_t *refdur,
		uint32_t *refpri)
{
	int delayindex;

	delayindex = (dl->dl_firstelem + scoreindex) & DFS_MAX_DL_MASK;
	*refdur = dl->dl_elems[delayindex].de_dur;
	*refpri = dl->dl_elems[delayindex].de_time;

	if (rf->rf_fixed_pri_radar_pulse)
		*refpri = (rf->rf_minpri + rf->rf_maxpri)/2;
}

/**
 * dfs_bin_success_print() - Debug print
 * @dfs: Pointer to wlan_dfs structure.
 * @rf: Pointer to dfs_filter structure.
 * @ext_chan_flag: Extension channel flag.
 * @numpulses: Number of pulses.
 * @refpri: Current "filter" time for start of pulse in usecs.
 * @refdur: Duration value.
 * @primargin: PRI margin.
 */
static inline void dfs_bin_success_print(
		struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		int ext_chan_flag,
		int numpulses,
		uint32_t refpri,
		uint32_t refdur,
		uint32_t primargin)
{
	dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"ext_flag=%d MATCH filter=%u numpulses=%u thresh=%u refdur=%d refpri=%d primargin=%d",
			ext_chan_flag, rf->rf_pulseid, numpulses,
			rf->rf_threshold, refdur, refpri, primargin);
	dfs_print_delayline(dfs, &rf->rf_dl);
	dfs_print_filter(dfs, rf);
}

int dfs_bin_check(
		struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		uint32_t deltaT,
		uint32_t width,
		int ext_chan_flag)
{
	struct dfs_delayline *dl;
	uint32_t refpri, refdur;
	uint32_t highscoreindex;
	uint32_t primargin, highscore;
	int score[DFS_MAX_DL_SIZE], found = 0;
	uint32_t scoreindex, lowpriindex = 0, lowpri = 0xffff;
	int numpulses = 0;
	int fil_thresh;

	dl = &rf->rf_dl;
	if (!dfs_bin_basic_sanity(dl, rf, &deltaT))
		return 0;

	primargin = dfs_get_pri_margin(dfs, ext_chan_flag,
			(rf->rf_patterntype == 1));


	if (rf->rf_patterntype == 1)
		return dfs_handle_fixedpattern(dfs, dl, rf, width,
				ext_chan_flag);

	dfs_find_lowestpri(dl, &lowpriindex, &lowpri);

	/* Find out the each delay element's pri score. */
	dfs_find_priscores(dl, rf, score, primargin);

	/* Find out the high scorer. */
	dfs_find_highscore(dl, score, &highscore, &highscoreindex);

	/*
	 * Find the average pri of pulses around the pri of highscore
	 * or the pulses around the lowest pri.
	 */
	dfs_find_scoreindex(rf, highscore, lowpriindex, highscoreindex,
			&scoreindex);

	/*
	 * Observed ETSI type2 while channel loading 31% with pulse pri:
	 * 1489, 2978, 2978, 2978, 1489, 2978, 1489 us. With above logic,
	 * the highscore will be 4 (2978), scoreindex is 5. In this case,
	 * index 0, 4, 6 pulses will be not matched later in
	 * dfs_count_the_other_delay_elements(), which leads to the radar was
	 * not detected. The fix is: compare the highscore pri with lowpri,
	 * if they have relationship, within primargin of
	 * [1, dfs_pri_multiplier] times of lowpri, choose lowpri as refpri.
	 */
	dfs_pick_lowpri(dfs, dl, rf, lowpriindex, &scoreindex, primargin);

	/* We got the possible pri, save its parameters as reference. */
	dfs_find_refs(dl, rf, scoreindex, &refdur, &refpri);

	numpulses = dfs_bin_pri_check(dfs, rf, dl, score[scoreindex], refpri,
			refdur, ext_chan_flag, refpri);

	fil_thresh = dfs_get_filter_threshold(dfs, rf, ext_chan_flag);

	if (numpulses >= fil_thresh) {
		found = 1;
		dfs_bin_success_print(dfs, rf, ext_chan_flag, numpulses,
				refpri, refdur, primargin);
	}

	return found;
}

/**
 * dfs_update_min_and_max_sidx() - Calculate min and max sidx.
 * @dl: Pointer to dfs_delayline structure.
 * @delayindex: Delay index.
 * @sidx_min: Sidx min.
 * @sidx_max: Sidx max.
 * @delta_peak_match_count: Delta peak match count.
 * @psidx_diff_match_count: Psidx diff match count.
 * @rf: Pointer to dfs_filter structure.
 */
static inline void dfs_update_min_and_max_sidx(
		struct dfs_delayline *dl,
		int delayindex,
		int32_t *sidx_min,
		int32_t *sidx_max,
		uint8_t *delta_peak_match_count,
		uint8_t *psidx_diff_match_count,
		struct dfs_filter *rf)
{
	/* update sidx min/max for false detection check later */
	if (*sidx_min > dl->dl_elems[delayindex].de_sidx)
		*sidx_min = dl->dl_elems[delayindex].de_sidx;

	if (*sidx_max < dl->dl_elems[delayindex].de_sidx)
		*sidx_max = dl->dl_elems[delayindex].de_sidx;

	if (rf->rf_check_delta_peak) {
		if (dl->dl_elems[delayindex].de_delta_peak != 0)
			(*delta_peak_match_count)++;
		else if ((dl->dl_elems[delayindex].de_psidx_diff >=
				DFS_MIN_PSIDX_DIFF) &&
			(dl->dl_elems[delayindex].de_psidx_diff <=
				DFS_MAX_PSIDX_DIFF))
			(*psidx_diff_match_count)++;
	}
}

/**
 * dfs_check_pulses_for_delta_variance() - Check pulses for delta variance.
 * @rf: Pointer to dfs_filter structure.
 * @numpulsetochk: Number of pulses to check.
 * @delta_time_stamps: Delta time stamp.
 * @fundamentalpri: Highest PRI.
 * @primargin: Primary margin.
 * @numpulses: Number of pulses.
 * @delayindex: Delay index.
 * @sidx_min: Sidx min.
 * @sidx_max: Sidx max.
 * @delta_peak_match_count: Delta peak match count.
 * @psidx_diff_match_count: Psidx diff match count.
 * @dl: Pointer to dfs_delayline structure.
 */
static inline void dfs_check_pulses_for_delta_variance(
		struct dfs_filter *rf,
		int numpulsetochk,
		uint32_t delta_time_stamps,
		int fundamentalpri,
		uint32_t primargin,
		int *numpulses,
		int delayindex,
		int32_t *sidx_min,
		int32_t *sidx_max,
		uint8_t *delta_peak_match_count,
		uint8_t *psidx_diff_match_count,
		struct dfs_delayline *dl)
{
	uint32_t delta_ts_variance, j;

	for (j = 0; j < numpulsetochk; j++) {
		delta_ts_variance = DFS_DIFF(delta_time_stamps,
				((j + 1) * fundamentalpri));
		if (delta_ts_variance < (2 * (j + 1) * primargin)) {
			dl->dl_seq_num_stop =
				dl->dl_elems[delayindex].de_seq_num;
			dfs_update_min_and_max_sidx(dl, delayindex,
					sidx_min, sidx_max,
					delta_peak_match_count,
					psidx_diff_match_count,
					rf);
			(*numpulses)++;
			if (rf->rf_ignore_pri_window > 0)
				break;
		}
	}
}

/**
 * dfs_count_the_other_delay_elements() - Counts the ther delay elements.
 * @dfs: Pointer to wlan_dfs structure.
 * @rf: Pointer to dfs_filter structure.
 * @dl: Pointer to dfs_delayline structure.
 * @i: Index value.
 * @refpri: Current "filter" time for start of pulse in usecs.
 * @refdur: Duration value.
 * @primargin: Primary margin.
 * @durmargin: Duration margin.
 * @numpulses: Number of pulses.
 * @delta_peak_match_count: Pointer to delta_peak_match_count.
 * @psidx_diff_match_count: Pointer to psidx_diff_match_count.
 * @prev_good_timestamp: Previous good timestamp.
 * @fundamentalpri: Highest PRI.
 */
static void dfs_count_the_other_delay_elements(
		struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		struct dfs_delayline *dl,
		uint32_t i,
		uint32_t refpri,
		uint32_t refdur,
		uint32_t primargin,
		uint32_t durmargin,
		int *numpulses,
		uint8_t *delta_peak_match_count,
		uint8_t *psidx_diff_match_count,
		uint32_t *prev_good_timestamp,
		int fundamentalpri)
{
	int delayindex;
	uint32_t searchpri, searchdur, deltadur;
	uint32_t j = 0, delta_time_stamps, deltapri, k;
	int dindex, primatch, numpulsetochk = 2;
	int32_t sidx_min = DFS_BIG_SIDX;
	int32_t sidx_max = -DFS_BIG_SIDX;

	delayindex = (dl->dl_firstelem + i) & DFS_MAX_DL_MASK;
	searchpri = dl->dl_elems[delayindex].de_time;
	if (searchpri == 0) {
		/*
		 * This events PRI is zero, take it as a valid pulse
		 * but decrement next event's PRI by refpri.
		 */
		dindex = (delayindex + 1) & DFS_MAX_DL_MASK;
		dl->dl_elems[dindex].de_time -=  refpri;
		searchpri = refpri;
	}

	searchdur = dl->dl_elems[delayindex].de_dur;
	deltadur = DFS_DIFF(searchdur, refdur);
	deltapri = DFS_DIFF(searchpri, refpri);
	primatch = 0;

	if ((rf->rf_ignore_pri_window > 0) && (rf->rf_patterntype != 2)) {
		for (j = 0; j < rf->rf_numpulses; j++) {
			deltapri = DFS_DIFF(searchpri, (j + 1) * refpri);
			if (deltapri < (2 * primargin)) {
				primatch = 1;
				break;
			}
		}
	} else if (rf->rf_patterntype == 2) {
		primatch = 1;
	} else {
		for (k = 1; k <= dfs->dfs_pri_multiplier; k++) {
			deltapri = DFS_DIFF(searchpri, k * refpri);
			if (deltapri < primargin) {
				primatch = 1;
				break;
			}
		}
	}

	if (primatch && (deltadur < durmargin)) {
		if (*numpulses == 1) {
			dl->dl_seq_num_second =
				dl->dl_elems[delayindex].de_seq_num;
			dfs_update_min_and_max_sidx(dl, delayindex,
					&sidx_min, &sidx_max,
					delta_peak_match_count,
					psidx_diff_match_count,
					rf);
			(*numpulses)++;
		} else {
			delta_time_stamps = (dl->dl_elems[delayindex].de_ts -
				*prev_good_timestamp);
			if ((rf->rf_ignore_pri_window > 0)) {
				numpulsetochk = rf->rf_numpulses;
				if ((rf->rf_patterntype == 2) &&
					(fundamentalpri < refpri + 100)) {
					numpulsetochk = 4;
				}
			} else {
				numpulsetochk = 4;
			}

			dfs_check_pulses_for_delta_variance(rf, numpulsetochk,
					delta_time_stamps, fundamentalpri,
					primargin, numpulses, delayindex,
					&sidx_min, &sidx_max,
					delta_peak_match_count,
					psidx_diff_match_count,
					dl);
		}
		*prev_good_timestamp = dl->dl_elems[delayindex].de_ts;
		dl->dl_search_pri = searchpri;
		dl->dl_min_sidx = sidx_min;
		dl->dl_max_sidx = sidx_max;
		dl->dl_delta_peak_match_count = *delta_peak_match_count;
		dl->dl_psidx_diff_match_count = *psidx_diff_match_count;

		dfs_debug(dfs, WLAN_DEBUG_DFS2,
			"rf->minpri=%d rf->maxpri=%d searchpri = %d index = %d numpulses = %d delta peak match count = %d psidx diff match count = %d deltapri=%d j=%d",
			rf->rf_minpri, rf->rf_maxpri, searchpri, i,
			*numpulses, *delta_peak_match_count,
			*psidx_diff_match_count, deltapri, j);
	}
}

int dfs_bin_pri_check(
		struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		struct dfs_delayline *dl,
		uint32_t score,
		uint32_t refpri,
		uint32_t refdur,
		int ext_chan_flag,
		int fundamentalpri)
{
	uint32_t searchpri, deltapri = 0;
	uint32_t averagerefpri = 0, MatchCount = 0;
	uint32_t prev_good_timestamp = 0;
	int dindex;
	uint32_t i, primargin, durmargin, highscore = score;
	uint32_t highscoreindex = 0;
	/*
	 * First pulse in the burst is most likely being filtered out based on
	 * maxfilterlen.
	 */
	int numpulses = 1;
	uint8_t delta_peak_match_count = 1;
	uint8_t psidx_diff_match_count = 1;
	int priscorechk = 1;

	/* Use the adjusted PRI margin to reduce false alarms
	 * For non fixed pattern types, rf->rf_patterntype=0.
	 */
	primargin = dfs_get_pri_margin(dfs, ext_chan_flag,
			(rf->rf_patterntype == 1));

	if ((refpri > rf->rf_maxpri) || (refpri < rf->rf_minpri)) {
		numpulses = 0;
		return numpulses;
	}

	dfs_get_durmargin(rf, &durmargin);

	if ((!rf->rf_fixed_pri_radar_pulse)) {
		if (rf->rf_ignore_pri_window == 1)
			priscorechk = (rf->rf_threshold >> 1);
		else
			priscorechk = 1;

		MatchCount = 0;
		if (score > priscorechk) {
			for (i = 0; i < dl->dl_numelems; i++) {
				dindex = (dl->dl_firstelem + i) &
					DFS_MAX_DL_MASK;
				searchpri = dl->dl_elems[dindex].de_time;
				deltapri = DFS_DIFF(searchpri, refpri);
				if (deltapri < primargin) {
					averagerefpri += searchpri;
					MatchCount++;
				}
			}
			if (rf->rf_patterntype != 2) {
				if (MatchCount > 0)
					refpri = (averagerefpri / MatchCount);
			} else {
				refpri = (averagerefpri / score);
			}
		}
	}

	/* Note: Following primultiple calculation should be done
	 * once per filter during initialization stage (dfs_attach)
	 * and stored in its array atleast for fixed frequency
	 * types like FCC Bin1 to save some CPU cycles.
	 * multiplication, devide operators in the following code
	 * are left as it is for readability hoping the complier
	 * will use left/right shifts wherever possible.
	 */
	dfs_debug(dfs, WLAN_DEBUG_DFS2,
		"refpri = %d high score = %d index = %d numpulses = %d",
		refpri, highscore, highscoreindex, numpulses);
	/*
	 * Count the other delay elements that have pri and dur with
	 * in the acceptable range from the reference one.
	 */
	for (i = 0; i < dl->dl_numelems; i++)
		dfs_count_the_other_delay_elements(dfs, rf, dl, i, refpri,
				refdur, primargin, durmargin, &numpulses,
				&delta_peak_match_count,
				&psidx_diff_match_count,
				&prev_good_timestamp, fundamentalpri);

	return numpulses;
}
