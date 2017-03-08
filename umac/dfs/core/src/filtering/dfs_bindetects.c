/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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

int dfs_bin_fixedpattern_check(struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		uint32_t dur,
		int ext_chan_flag)
{
	struct dfs_pulseline *pl = dfs->pulses;
	int i, n, refpri, primargin, numpulses = 0;
	uint64_t start_ts, end_ts, event_ts, prev_event_ts;
	uint64_t next_event_ts, window_start, window_end;
	uint32_t index, next_index, deltadur;

	/* For fixed pattern types, rf->rf_patterntype=1. */
	primargin = dfs_get_pri_margin(dfs, ext_chan_flag,
		(rf->rf_patterntype == 1));

	refpri = (rf->rf_minpri + rf->rf_maxpri)/2;
	index = pl->pl_lastelem;
	end_ts = pl->pl_elems[index].p_time;
	start_ts = end_ts - (refpri*rf->rf_numpulses);

	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS3,
		"lastelem ts=%llu start_ts=%llu, end_ts=%llu\n",
		(unsigned long long)pl->pl_elems[index].p_time,
		(unsigned long long)start_ts,
		(unsigned long long) end_ts);

	/* Find the index of first element in our window of interest. */
	for (i = 0; i < pl->pl_numelems; i++) {
		index = (index - 1) & DFS_MAX_PULSE_BUFFER_MASK;
		if (pl->pl_elems[index].p_time >= start_ts)
			continue;
		else {
			index = (index) & DFS_MAX_PULSE_BUFFER_MASK;
			break;
		}
	}
	for (n = 0; n <= rf->rf_numpulses; n++) {
		window_start = (start_ts + (refpri*n))-(primargin+n);
		window_end = window_start + 2*(primargin+n);
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
			"window_start %u window_end %u\n",
			(uint32_t)window_start, (uint32_t)window_end);

		for (i = 0; i < pl->pl_numelems; i++) {
			prev_event_ts = pl->pl_elems[index].p_time;
			index = (index+1) & DFS_MAX_PULSE_BUFFER_MASK;
			event_ts = pl->pl_elems[index].p_time;
			next_index = (index+1) & DFS_MAX_PULSE_BUFFER_MASK;
			next_event_ts = pl->pl_elems[next_index].p_time;
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2, "ts %u\n",
				(uint32_t)event_ts);

			if ((event_ts <= window_end) &&
				(event_ts >= window_start)) {
				deltadur = DFS_DIFF(pl->pl_elems[index].p_dur,
					dur);
				if ((pl->pl_elems[index].p_dur == 1) ||
					((dur != 1) && (deltadur <= 2))) {
					numpulses++;
					DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
						"numpulses %u\n", numpulses);
					break;
				}
			} else if (event_ts > window_end) {
				index = (index-1) & DFS_MAX_PULSE_BUFFER_MASK;
				break;
			} else if (event_ts == prev_event_ts) {
				if (((next_event_ts - event_ts) > refpri) ||
					((next_event_ts - event_ts) == 0)) {
					deltadur =
					    DFS_DIFF(pl->pl_elems[index].p_dur,
						    dur);
					if ((pl->pl_elems[index].p_dur == 1) ||
						((pl->pl_elems[index].p_dur !=
						  1) && (deltadur <= 2))) {
						numpulses++;
						DFS_DPRINTK(dfs,
							WLAN_DEBUG_DFS2,
							"zero PRI: numpulses %u\n",
							numpulses);
						break;
					}
				}
			}
		}
	}
	if (numpulses >= dfs_get_filter_threshold(dfs, rf, ext_chan_flag)) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
			"%s FOUND filterID=%u numpulses=%d unadj thresh=%d\n",
			__func__, rf->rf_pulseid, numpulses, rf->rf_threshold);
		return 1;
	} else
		return 0;
}

void dfs_add_pulse(struct wlan_dfs *dfs,
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

	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
		"%s: adding: filter id %d, dur=%d, rssi=%d, ts=%llu\n",
		__func__, rf->rf_pulseid, re->re_dur,
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
	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2, "dl firstElem = %d  lastElem = %d\n",
			dl->dl_firstelem, dl->dl_lastelem);
}

int dfs_bin_check(struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		uint32_t deltaT,
		uint32_t width,
		int ext_chan_flag)
{
	struct dfs_delayline *dl;
	uint32_t refpri, refdur, searchpri, deltapri, deltapri_2, deltapri_3;
	uint32_t averagerefpri, n, i, primargin, durmargin, highscore;
	uint32_t highscoreindex;
	int score[DFS_MAX_DL_SIZE], delayindex, dindex, found = 0;
	uint32_t scoreindex, lowpriindex = 0, lowpri = 0xffff;
	int numpulses = 0;
	int lowprichk = 3, pri_match = 0;

	dl = &rf->rf_dl;
	if (dl->dl_numelems < (rf->rf_threshold-1))
		return 0;

	if (deltaT > rf->rf_filterlen)
		return 0;

	primargin = dfs_get_pri_margin(dfs, ext_chan_flag,
			(rf->rf_patterntype == 1));

	if (rf->rf_maxdur < 10)
		durmargin = 4;
	else
		durmargin = 6;

	if (rf->rf_patterntype == 1) {
		found = dfs_bin_fixedpattern_check(dfs, rf, width,
				ext_chan_flag);
		if (found)
			dl->dl_numelems = 0;
		return found;
	}

	qdf_mem_zero(score, sizeof(int)*DFS_MAX_DL_SIZE);
	/* Find out the lowest pri. */
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
	/* Find out the each delay element's pri score. */
	for (n = 0; n < dl->dl_numelems; n++) {
		delayindex = (dl->dl_firstelem + n) &
			DFS_MAX_DL_MASK;
		refpri = dl->dl_elems[delayindex].de_time;
		if (refpri == 0)
			continue;
		if (refpri < rf->rf_maxpri) {
			/* Use only valid PRI range for high score. */
			for (i = 0; i < dl->dl_numelems; i++) {
				dindex = (dl->dl_firstelem + i) &
				    DFS_MAX_DL_MASK;
				searchpri = dl->dl_elems[dindex].de_time;
				deltapri = DFS_DIFF(searchpri, refpri);
				deltapri_2 = DFS_DIFF(searchpri, 2*refpri);
				deltapri_3 = DFS_DIFF(searchpri, 3*refpri);
				if (rf->rf_ignore_pri_window == 2) {
					pri_match = ((deltapri < primargin) ||
						(deltapri_2 < primargin) ||
						(deltapri_3 < primargin));
				} else {
					pri_match = (deltapri < primargin);
				}
				if (pri_match)
					score[n]++;
			}
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

	/* Find out the high scorer. */
	highscore = 0;
	highscoreindex = 0;
	for (n = 0; n < dl->dl_numelems; n++) {
		if (score[n] > highscore) {
			highscore = score[n];
			highscoreindex = n;
		} else if (score[n] == highscore) {
			/*
			 * More than one pri has highscore take the least pri.
			 */
			delayindex = (dl->dl_firstelem + highscoreindex) &
				DFS_MAX_DL_MASK;
			dindex = (dl->dl_firstelem + n) & DFS_MAX_DL_MASK;
			if (dl->dl_elems[dindex].de_time <=
					dl->dl_elems[delayindex].de_time) {
				highscoreindex = n;
			}
		}
	}

	/*
	 * Find the average pri of pulses around the pri of highscore
	 * or the pulses around the lowest pri.
	 */
	if (rf->rf_ignore_pri_window > 0)
		lowprichk = (rf->rf_threshold >> 1)+1;
	else
		lowprichk = 3;

	if (highscore < lowprichk)
		scoreindex = lowpriindex;
	else
		scoreindex = highscoreindex;

	/* We got the possible pri, save its parameters as reference. */
	delayindex = (dl->dl_firstelem + scoreindex) & DFS_MAX_DL_MASK;
	refdur = dl->dl_elems[delayindex].de_dur;
	refpri = dl->dl_elems[delayindex].de_time;
	averagerefpri = 0;

	if (rf->rf_fixed_pri_radar_pulse)
		refpri = (rf->rf_minpri + rf->rf_maxpri)/2;

	numpulses = dfs_bin_pri_check(dfs, rf, dl, score[scoreindex], refpri,
			refdur, ext_chan_flag, refpri);
	if (numpulses >= dfs_get_filter_threshold(dfs, rf, ext_chan_flag)) {
		found = 1;
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
			"ext_flag=%d MATCH filter=%u numpulses=%u thresh=%u refdur=%d refpri=%d primargin=%d\n",
			ext_chan_flag, rf->rf_pulseid, numpulses,
			rf->rf_threshold, refdur, refpri, primargin);
		dfs_print_delayline(dfs, &rf->rf_dl);
		dfs_print_filter(dfs, rf);
	}

	return found;
}

void count_the_other_delay_elements(struct wlan_dfs *dfs,
		struct dfs_filter *rf,
		struct dfs_delayline *dl,
		uint32_t i,
		uint32_t refpri,
		uint32_t refdur,
		uint32_t primargin,
		uint32_t durmargin,
		int *numpulses,
		uint32_t *prev_good_timestamp,
		int fundamentalpri
		)
{
	int delayindex;
	uint32_t searchpri, searchdur, deltadur, deltapri1, deltapri2;
	uint32_t j = 0, delta_time_stamps, delta_ts_variance, deltapri;
	int dindex, primatch, numpulsetochk = 2;

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
	deltapri1 = DFS_DIFF(searchpri, refpri);
	deltapri2 = DFS_DIFF(searchpri, 2 * refpri);
	primatch = 0;

	if ((rf->rf_ignore_pri_window > 0) &&
			(rf->rf_patterntype != 2)) {
		for (j = 0; j < rf->rf_numpulses; j++) {
			deltapri1 = DFS_DIFF(searchpri,
					(j + 1) * refpri);
			if (deltapri1 < (2 * primargin)) {
				primatch = 1;
				break;
			}
		}
	} else {
		if ((deltapri1 < primargin) ||
				(deltapri2 < primargin)) {
			primatch = 1;
		}
	}

	if (primatch && (deltadur < durmargin)) {
		if ((*numpulses == 1)) {
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
			for (j = 0; j < numpulsetochk; j++) {
				delta_ts_variance = DFS_DIFF(delta_time_stamps,
					((j + 1) * fundamentalpri));
				if (delta_ts_variance <
					(2 * (j + 1) * primargin)) {
					(*numpulses)++;
					if (rf->rf_ignore_pri_window > 0)
						break;
				}
			}
		}
		*prev_good_timestamp = dl->dl_elems[delayindex].de_ts;

		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
			"rf->minpri=%d rf->maxpri=%d searchpri = %d index = %d numpulses = %d deltapri=%d j=%d\n",
			rf->rf_minpri, rf->rf_maxpri, searchpri,
			i, *numpulses, deltapri, j);
	}
}

int dfs_bin_pri_check(struct wlan_dfs *dfs,
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

	if (rf->rf_maxdur < 10)
		durmargin = 4;
	else
		durmargin = 6;

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
	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
		"refpri = %d high score = %d index = %d numpulses = %d\n",
		refpri, highscore, highscoreindex, numpulses);
	/*
	 * Count the other delay elements that have pri and dur with
	 * in the acceptable range from the reference one.
	 */
	for (i = 0; i < dl->dl_numelems; i++)
		count_the_other_delay_elements(dfs, rf, dl, i, refpri, refdur,
			primargin, durmargin, &numpulses, &prev_good_timestamp,
			fundamentalpri);

	return numpulses;
}
