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
 * DOC: This contains the functionality to process the radar event generated
 * for a pulse. This will group together pulses and call various detection
 * functions to figure out whether a valid radar has been detected.
 */

#include "../dfs.h"
#include "../dfs_channel.h"
#include "../dfs_internal.h"

#define FREQ_5500_MHZ  5500
#define FREQ_5500_MHZ       5500

#define DFS_MAX_FREQ_SPREAD            (1375 * 1)
#define DFS_LARGE_PRI_MULTIPLIER       4
#define DFS_W53_DEFAULT_PRI_MULTIPLIER 2

static char debug_dup[33];
static int debug_dup_cnt;

/**
 * dfs_process_pulse_dur() - Process pulse duration.
 * @dfs: Pointer to wlan_dfs structure.
 * @re_dur: Duration.
 *
 * Convert the hardware provided duration to TSF ticks (usecs) taking the clock
 * (fast or normal) into account. Legacy (pre-11n, Owl, Sowl, Howl) operate
 * 5GHz using a 40MHz clock.  Later 11n chips (Merlin, Osprey, etc) operate
 * 5GHz using a 44MHz clock, so the reported pulse durations are different.
 * Peregrine reports the pulse duration in microseconds regardless of the
 * operating mode. (XXX TODO: verify this, obviously.)
 *
 * Return: Returns the duration.
 */
static inline uint8_t dfs_process_pulse_dur(struct wlan_dfs *dfs,
		uint8_t re_dur)
{
	/*
	 * Short pulses are sometimes returned as having a duration of 0,
	 * so round those up to 1.
	 * XXX This holds true for BB TLV chips too, right?
	 */
	if (re_dur == 0)
		return 1;

	/*
	 * For BB TLV chips, the hardware always returns microsecond pulse
	 * durations.
	 */
	if (dfs->dfs_caps.wlan_chip_is_bb_tlv)
		return re_dur;

	/*
	 * This is for 11n and legacy chips, which may or may not use the 5GHz
	 * fast clock mode.
	 */
	/* Convert 0.8us durations to TSF ticks (usecs) */
	return (uint8_t)dfs_round((int32_t)((dfs->dur_multiplier)*re_dur));
}

/*
 * dfs_print_radar_events() - Prints the Radar events.
 * @dfs: Pointer to wlan_dfs structure.
 */
static void dfs_print_radar_events(struct wlan_dfs *dfs)
{
	int i;

	DFS_PRINTK("%s:#Phyerr=%d, #false detect=%d, #queued=%d\n",
		__func__,
		dfs->dfs_phyerr_count,
		dfs->dfs_phyerr_reject_count,
		dfs->dfs_phyerr_queued_count);

	DFS_PRINTK("%s:dfs_phyerr_freq_min=%d, dfs_phyerr_freq_max=%d\n",
		__func__,
		dfs->dfs_phyerr_freq_min,
		dfs->dfs_phyerr_freq_max);

	DFS_PRINTK(
		"%s:Total radar events detected=%d, entries in the radar queue follows:\n",
		__func__,
		dfs->dfs_event_log_count);

	for (i = 0; (i < DFS_EVENT_LOG_SIZE) && (i < dfs->dfs_event_log_count);
			i++) {
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
			"ts=%llu diff_ts=%u rssi=%u dur=%u, is_chirp=%d, seg_id=%d, sidx=%d, freq_offset=%d.%dMHz, peak_mag=%d, total_gain=%d, mb_gain=%d, relpwr_db=%d\n",
			dfs->radar_log[i].ts,
			dfs->radar_log[i].diff_ts,
			dfs->radar_log[i].rssi,
			dfs->radar_log[i].dur,
			dfs->radar_log[i].is_chirp,
			dfs->radar_log[i].seg_id,
			dfs->radar_log[i].sidx,
			(int)dfs->radar_log[i].freq_offset_khz/1000,
			(int)abs(dfs->radar_log[i].freq_offset_khz)%1000,
			dfs->radar_log[i].peak_mag,
			dfs->radar_log[i].total_gain,
			dfs->radar_log[i].mb_gain,
			dfs->radar_log[i].relpwr_db);
	}
	dfs->dfs_event_log_count = 0;
	dfs->dfs_phyerr_count = 0;
	dfs->dfs_phyerr_reject_count = 0;
	dfs->dfs_phyerr_queued_count = 0;
	dfs->dfs_phyerr_freq_min = 0x7fffffff;
	dfs->dfs_phyerr_freq_max = 0;
}

void __dfs_process_radarevent(struct wlan_dfs *dfs,
		struct dfs_filtertype *ft,
		struct dfs_filter *rf,
		struct dfs_event *re,
		uint64_t this_ts,
		int *found)
{
	int p;
	uint64_t deltaT = 0;
	int ext_chan_event_flag = 0;

	for (p = 0, *found = 0; (p < ft->ft_numfilters) &&
			(!(*found)); p++) {
		rf = &(ft->ft_filters[p]);
		if ((re->re_dur >= rf->rf_mindur) &&
				(re->re_dur <= rf->rf_maxdur)) {
			/* The above check is probably not necessary. */
			deltaT = (this_ts < rf->rf_dl.dl_last_ts) ?
			    (int64_t)((DFS_TSF_WRAP - rf->rf_dl.dl_last_ts) +
				    this_ts + 1) :
			    this_ts - rf->rf_dl.dl_last_ts;

			if ((deltaT < rf->rf_minpri) && (deltaT != 0)) {
				/* Second line of PRI filtering. */
				DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
					"filterID %d : Rejecting on individual filter min PRI deltaT=%lld rf->rf_minpri=%u\n",
					rf->rf_pulseid,
					(uint64_t)deltaT,
					rf->rf_minpri);
				continue;
			}

			if (rf->rf_ignore_pri_window > 0) {
				if (deltaT < rf->rf_minpri) {
					DFS_DPRINTK(dfs,
						WLAN_DEBUG_DFS2,
						"filterID %d : Rejecting on individual filter max PRI deltaT=%lld rf->rf_minpri=%u\n",
						rf->rf_pulseid,
						(uint64_t)
						deltaT,
						rf->rf_minpri);
					/* But update the last time stamp. */
					rf->rf_dl.dl_last_ts = this_ts;
					continue;
				}
			} else {
				/*
				 * The HW may miss some pulses especially with
				 * high channel loading. This is true for Japan
				 * W53 where channel loaoding is 50%. Also for
				 * ETSI where channel loading is 30% this can
				 * be an issue too. To take care of missing
				 * pulses, we introduce pri_margin multiplie.
				 * This is normally 2 but can be higher for W53.
				 */

				if ((deltaT > (dfs->dfs_pri_multiplier *
						rf->rf_maxpri)) ||
					(deltaT < rf->rf_minpri)
				   ) {
					DFS_DPRINTK(dfs,
						WLAN_DEBUG_DFS2,
						"filterID %d : Rejecting on individual filter max PRI deltaT=%lld rf->rf_minpri=%u\n",
						rf->rf_pulseid,
						(uint64_t)
						deltaT,
						rf->rf_minpri);
					/* But update the last time stamp. */
					rf->rf_dl.dl_last_ts = this_ts;
					continue;
				}
			}
			dfs_add_pulse(dfs, rf, re, deltaT, this_ts);

			/*
			 * If this is an extension channel event, flag it for
			 * false alarm reduction.
			 */
			if (re->re_chanindex == dfs->dfs_extchan_radindex)
				ext_chan_event_flag = 1;

			if (rf->rf_patterntype == 2) {
				*found = dfs_staggered_check(dfs, rf,
					(uint32_t) deltaT, re->re_dur);
			} else {
				*found = dfs_bin_check(dfs, rf,
					(uint32_t) deltaT, re->re_dur,
					ext_chan_event_flag);
			}
			if (dfs->dfs_debug_mask & WLAN_DEBUG_DFS2)
				dfs_print_delayline(dfs, &rf->rf_dl);

			rf->rf_dl.dl_last_ts = this_ts;
		}
	}
}

int dfs_process_radarevent(struct wlan_dfs *dfs,
	struct dfs_ieee80211_channel *chan)
{
	struct dfs_event re, *event;
	struct dfs_state *rs = NULL;
	struct dfs_filtertype *ft;
	struct dfs_filter *rf;
	int found, retval = 0, p, empty;
	int events_processed = 0;
	uint32_t tabledepth, index;
	uint64_t deltafull_ts = 0, this_ts, deltaT;
	struct dfs_ieee80211_channel *thischan;
	struct dfs_pulseline *pl;
	static uint32_t  test_ts;
	static uint32_t  diff_ts;
	int i;
	uint8_t   seg_id = 0;

	pl = dfs->pulses;

	if (!(dfs->dfs_second_segment_bangradar ||
				dfs_is_precac_timer_running(dfs)))
		if (!(IEEE80211_IS_CHAN_DFS(chan) ||
			    ((IEEE80211_IS_CHAN_11AC_VHT160(chan) ||
			      IEEE80211_IS_CHAN_11AC_VHT80_80(chan)) &&
			     IEEE80211_IS_CHAN_DFS_CFREQ2(chan)))) {
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
				"%s: radar event on non-DFS chan\n",
				__func__);
			dfs_reset_radarq(dfs);
			dfs_reset_alldelaylines(dfs);
			dfs->dfs_bangradar = 0;
			return 0;
		}

	/*
	 * TEST : Simulate radar bang, make sure we add the channel to NOL
	 * (bug 29968)
	 */
	if (dfs->dfs_bangradar) {
		/*
		 * Bangradar will always simulate radar found on the primary
		 * channel.
		 */
		rs = &dfs->dfs_radar[dfs->dfs_curchan_radindex];
		dfs->dfs_bangradar = 0; /* Reset */
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS, "%s: bangradar\n", __func__);
		retval = 1;
		goto dfsfound;
	}

	if (dfs->dfs_second_segment_bangradar) {
		if (dfs_is_precac_timer_running(dfs) ||
				IEEE80211_IS_CHAN_11AC_VHT160(chan) ||
				IEEE80211_IS_CHAN_11AC_VHT80_80(chan)) {
			dfs->is_radar_found_on_secondary_seg = 1;
			rs = &dfs->dfs_radar[dfs->dfs_curchan_radindex];
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
					"%s: second segment bangradar on cfreq = %u\n",
					__func__,
					dfs->dfs_precac_secondary_freq);
			retval = 1;
			seg_id = SEG_ID_SECONDARY;
		} else {
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
					"%s: Do not process the second segment bangradar\n",
					__func__);
		}
		dfs->dfs_second_segment_bangradar = 0; /* Reset */
		goto dfsfound;
	}

	/*
	 * The HW may miss some pulses especially with high channel loading.
	 * This is true for Japan W53 where channel loaoding is 50%. Also
	 * for ETSI where channel loading is 30% this can be an issue too.
	 * To take care of missing pulses, we introduce pri_margin multiplie.
	 * This is normally 2 but can be higher for W53.
	 */

	if ((dfs->dfsdomain  == DFS_MKK4_DOMAIN) &&
		(dfs->dfs_caps.wlan_chip_is_bb_tlv) &&
		(chan->ic_freq < FREQ_5500_MHZ)) {

		dfs->dfs_pri_multiplier = DFS_W53_DEFAULT_PRI_MULTIPLIER;
		/*
		 * Do not process W53 pulses unless we have a minimum number
		 * of them.
		 */
		if (dfs->dfs_phyerr_w53_counter >= 5) {
			/*
			 * For chips that support frequency information, we
			 * can relax PRI restriction if the frequency spread
			 * is narrow.
			 */
			if ((dfs->dfs_phyerr_freq_max -
				    dfs->dfs_phyerr_freq_min) <
				DFS_MAX_FREQ_SPREAD) {
				dfs->dfs_pri_multiplier =
				    DFS_LARGE_PRI_MULTIPLIER;
			}

			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
				"%s: w53_counter=%d, freq_max=%d, freq_min=%d, pri_multiplier=%d\n",
				__func__,
				dfs->dfs_phyerr_w53_counter,
				dfs->dfs_phyerr_freq_max,
				dfs->dfs_phyerr_freq_min,
				dfs->dfs_pri_multiplier);
			dfs->dfs_phyerr_freq_min = 0x7fffffff;
			dfs->dfs_phyerr_freq_max = 0;
		} else {
			return 0;
		}
	}
	DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
			"%s: pri_multiplier=%d\n",
			__func__, dfs->dfs_pri_multiplier);

	WLAN_DFSQ_LOCK(dfs);
	empty = STAILQ_EMPTY(&(dfs->dfs_radarq));
	WLAN_DFSQ_UNLOCK(dfs);

	while ((!empty) && (!retval) && (events_processed < MAX_EVENTS)) {
		WLAN_DFSQ_LOCK(dfs);
		event = STAILQ_FIRST(&(dfs->dfs_radarq));
		if (event != NULL)
			STAILQ_REMOVE_HEAD(&(dfs->dfs_radarq), re_list);
		WLAN_DFSQ_UNLOCK(dfs);

		if (event == NULL) {
			empty = 1;
			break;
		}
		events_processed++;
		re = *event;

		qdf_mem_zero(event, sizeof(struct dfs_event));
		WLAN_DFSEVENTQ_LOCK(dfs);
		STAILQ_INSERT_TAIL(&(dfs->dfs_eventq), event, re_list);
		WLAN_DFSEVENTQ_UNLOCK(dfs);

		seg_id = re.re_seg_id;
		found = 0;
		if (re.re_chanindex < DFS_NUM_RADAR_STATES)
			rs = &dfs->dfs_radar[re.re_chanindex];
		else {
			WLAN_DFSQ_LOCK(dfs);
			empty = STAILQ_EMPTY(&(dfs->dfs_radarq));
			WLAN_DFSQ_UNLOCK(dfs);
			continue;
		}
		if (rs->rs_chan.ic_flagext & CHANNEL_INTERFERENCE) {
			WLAN_DFSQ_LOCK(dfs);
			empty = STAILQ_EMPTY(&(dfs->dfs_radarq));
			WLAN_DFSQ_UNLOCK(dfs);
			continue;
		}

		if (dfs->dfs_rinfo.rn_lastfull_ts == 0) {
			/*
			 * Either not started, or 64-bit rollover exactly to
			 * zero Just prepend zeros to the 15-bit ts.
			 */
			dfs->dfs_rinfo.rn_ts_prefix = 0;
		} else {
			/* WAR 23031- patch duplicate ts on very short pulses.
			 * This pacth has two problems in linux environment.
			 * 1)The time stamp created and hence PRI depends
			 * entirely on the latency. If the latency is high, it
			 * possibly can split two consecutive pulses in the
			 * same burst so far away (the same amount of latency)
			 * that make them look like they are from differenct
			 * bursts. It is observed to happen too often. It sure
			 * makes the detection fail.
			 * 2)Even if the latency is not that bad, it simply
			 * shifts the duplicate timestamps to a new duplicate
			 * timestamp based on how they are processed.
			 * This is not worse but not good either.
			 * Take this pulse as a good one and create a probable
			 * PRI later.
			 */
			if (re.re_dur == 0 && re.re_ts ==
					dfs->dfs_rinfo.rn_last_unique_ts) {
				debug_dup[debug_dup_cnt++] = '1';
				DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
					"\n %s deltaT is 0\n", __func__);
			} else {
				dfs->dfs_rinfo.rn_last_unique_ts = re.re_ts;
				debug_dup[debug_dup_cnt++] = '0';
			}

			if (debug_dup_cnt >= 32)
				debug_dup_cnt = 0;

			if (re.re_ts <= dfs->dfs_rinfo.rn_last_ts) {
				dfs->dfs_rinfo.rn_ts_prefix +=
					(((uint64_t) 1) << DFS_TSSHIFT);
				/* Now, see if it's been more than 1 wrap */
				deltafull_ts = re.re_full_ts -
					dfs->dfs_rinfo.rn_lastfull_ts;
				if (deltafull_ts >
					((uint64_t)((DFS_TSMASK -
						dfs->dfs_rinfo.rn_last_ts)
					    + 1 + re.re_ts)))
					deltafull_ts -= (DFS_TSMASK -
						dfs->dfs_rinfo.rn_last_ts) + 1 +
					    re.re_ts;
				deltafull_ts = deltafull_ts >> DFS_TSSHIFT;

				if (deltafull_ts > 1) {
					dfs->dfs_rinfo.rn_ts_prefix +=
						((deltafull_ts - 1) <<
						 DFS_TSSHIFT);
				}
			} else {
				deltafull_ts = re.re_full_ts -
					dfs->dfs_rinfo.rn_lastfull_ts;
				if (deltafull_ts > (uint64_t) DFS_TSMASK) {
					deltafull_ts =
					    deltafull_ts >> DFS_TSSHIFT;
					dfs->dfs_rinfo.rn_ts_prefix +=
					    ((deltafull_ts - 1) << DFS_TSSHIFT);
				}
			}
		}

		/*
		 * At this stage rn_ts_prefix has either been blanked or
		 * calculated, so it's safe to use.
		 */
		this_ts = dfs->dfs_rinfo.rn_ts_prefix | ((uint64_t) re.re_ts);
		dfs->dfs_rinfo.rn_lastfull_ts = re.re_full_ts;
		dfs->dfs_rinfo.rn_last_ts = re.re_ts;

		/*
		 * The hardware returns the duration in a variety of formats,
		 * so it's converted from the hardware format to TSF (usec)
		 * values here.
		 * XXX TODO: this should really be done when the PHY error
		 * is processed, rather than way out here..
		 */
		re.re_dur = dfs_process_pulse_dur(dfs, re.re_dur);

		/*
		 * Calculate the start of the radar pulse.
		 *
		 * The TSF is stamped by the MAC upon reception of the event,
		 * which is (typically?) at the end of the event. But the
		 * pattern matching code expects the event timestamps to be at
		 * the start of the event. So to fake it, we subtract the pulse
		 * duration from the given TSF. This is done after the 64-bit
		 * timestamp has been calculated so long pulses correctly
		 * under-wrap the counter.  Ie, if this was done on the 32
		 * (or 15!) bit TSF when the TSF value is closed to 0, it will
		 * underflow to 0xfffffXX, which would mess up the logical "OR"
		 * operation done above.
		 * This isn't valid for Peregrine as the hardware gives us the
		 * actual TSF offset of the radar event, not just the MAC TSF
		 * of the completed receive.
		 *
		 * XXX TODO: ensure that the TLV PHY error processing code will
		 * correctly calculate the TSF to be the start of the radar
		 * pulse.
		 *
		 * XXX TODO TODO: modify the TLV parsing code to subtract the
		 * duration from the TSF, based on the current fast clock value.
		 */
		if ((!dfs->dfs_caps.wlan_chip_is_bb_tlv) && re.re_dur != 1)
			this_ts -= re.re_dur;

		/* Save the pulse parameters in the pulse buffer(pulse line). */
		index = (pl->pl_lastelem + 1) & DFS_MAX_PULSE_BUFFER_MASK;

		if (pl->pl_numelems == DFS_MAX_PULSE_BUFFER_SIZE)
			pl->pl_firstelem = (pl->pl_firstelem+1) &
				DFS_MAX_PULSE_BUFFER_MASK;
		else
			pl->pl_numelems++;

		pl->pl_lastelem = index;
		pl->pl_elems[index].p_time = this_ts;
		pl->pl_elems[index].p_dur = re.re_dur;
		pl->pl_elems[index].p_rssi = re.re_rssi;
		diff_ts = (uint32_t)this_ts - test_ts;
		test_ts = (uint32_t)this_ts;

		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
			"ts%u %u %u diff %u pl->pl_lastelem.p_time=%llu\n",
			(uint32_t)this_ts, re.re_dur,
			re.re_rssi, diff_ts,
			(uint64_t)pl->pl_elems[index].p_time);

		if (dfs->dfs_event_log_on) {
			i = dfs->dfs_event_log_count % DFS_EVENT_LOG_SIZE;
			dfs->radar_log[i].ts = this_ts;
			dfs->radar_log[i].diff_ts = diff_ts;
			dfs->radar_log[i].rssi = re.re_rssi;
			dfs->radar_log[i].dur = re.re_dur;
			dfs->radar_log[i].seg_id = re.re_seg_id;
			dfs->radar_log[i].sidx = re.re_sidx;
			dfs->radar_log[i].freq_offset_khz =
				re.re_freq_offset_khz;
			dfs->radar_log[i].peak_mag = re.re_peak_mag;
			dfs->radar_log[i].total_gain = re.re_total_gain;
			dfs->radar_log[i].mb_gain = re.re_mb_gain;
			dfs->radar_log[i].relpwr_db = re.re_relpwr_db;
			dfs->radar_log[i].is_chirp = DFS_EVENT_NOTCHIRP(&re) ?
				0 : 1;
			dfs->dfs_event_log_count++;
		}

		/* If diff_ts is very small, we might be getting false pulse
		 * detects due to heavy interference. We might be getting
		 * spectral splatter from adjacent channel. In order to prevent
		 * false alarms we clear the delay-lines. This might impact
		 * positive detections under harsh environments, but helps with
		 * false detects.
		 */

		if (diff_ts < 100) {
			dfs_reset_alldelaylines(dfs);
			dfs_reset_radarq(dfs);
		}
		found = 0;

		/* BIN5 pulses are FCC and Japan specific. */
		if ((dfs->dfsdomain == DFS_FCC_DOMAIN) ||
				(dfs->dfsdomain == DFS_MKK4_DOMAIN)) {
			for (p = 0; (p < dfs->dfs_rinfo.rn_numbin5radars) &&
					(!found); p++) {
				struct dfs_bin5radars *br;

				br = &(dfs->dfs_b5radars[p]);
				if (dfs_bin5_check_pulse(dfs, &re, br)) {
					/*
					 * This is a valid Bin5 pulse, check if
					 * it belongs to a burst.
					 */
					re.re_dur =
					    dfs_retain_bin5_burst_pattern(dfs,
						    diff_ts, re.re_dur);
					/*
					 * Remember our computed duration for
					 * the next pulse in the burst
					 * (if needed).
					 */
					dfs->dfs_rinfo.dfs_bin5_chirp_ts =
						this_ts;
					dfs->dfs_rinfo.dfs_last_bin5_dur =
						re.re_dur;

					if (dfs_bin5_addpulse(dfs, br, &re,
								this_ts)) {
						found |= dfs_bin5_check(dfs);
					}
				} else
					DFS_DPRINTK(dfs,
						WLAN_DEBUG_DFS_BIN5_PULSE,
						"%s not a BIN5 pulse (dur=%d)\n",
						__func__, re.re_dur);
			}
		}

		if (found) {
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS,
				"%s: Found bin5 radar\n", __func__);
			retval |= found;
			goto dfsfound;
		}

		tabledepth = 0;
		rf = NULL;
		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
			"  *** chan freq (%d): ts %llu dur %u rssi %u\n",
			rs->rs_chan.ic_freq,
			(uint64_t)this_ts,
			re.re_dur,
			re.re_rssi);

		while ((tabledepth < DFS_MAX_RADAR_OVERLAP) &&
				((dfs->dfs_radartable[re.re_dur])[tabledepth] !=
				 -1) && (!retval)) {
			ft = dfs->dfs_radarf[((dfs->dfs_radartable[re.re_dur])
					[tabledepth])];
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
				"  ** RD (%d): ts %x dur %u rssi %u\n",
				rs->rs_chan.ic_freq,
				re.re_ts,
				re.re_dur,
				re.re_rssi);

			if (re.re_rssi < ft->ft_rssithresh && re.re_dur > 4) {
				DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
					"%s : Rejecting on rssi rssi=%u thresh=%u\n",
					__func__,
					re.re_rssi,
					ft->ft_rssithresh);
				tabledepth++;
				WLAN_DFSQ_LOCK(dfs);
				empty = STAILQ_EMPTY(&(dfs->dfs_radarq));
				WLAN_DFSQ_UNLOCK(dfs);
				continue;
			}
			deltaT = this_ts - ft->ft_last_ts;
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
				"deltaT = %lld (ts: 0x%llx) (last ts: 0x%llx)\n",
				(uint64_t)deltaT,
				(uint64_t)this_ts,
				(uint64_t)ft->ft_last_ts);

			if ((deltaT < ft->ft_minpri) && (deltaT != 0)) {
				/*
				 * This check is for the whole filter type.
				 * Individual filters will check this again.
				 * This is first line of filtering.
				 */
				DFS_DPRINTK(dfs, WLAN_DEBUG_DFS2,
					"%s: Rejecting on pri pri=%lld minpri=%u\n",
					__func__,
					(uint64_t)deltaT,
					ft->ft_minpri);
				tabledepth++;
				continue;
			}

			__dfs_process_radarevent(dfs, ft, rf, &re, this_ts,
					&found);
			ft->ft_last_ts = this_ts;
			retval |= found;
			if (found) {
				DFS_PRINTK(
					"Found on channel minDur = %d, filterId = %d\n",
					ft->ft_mindur,
					rf != NULL ?  rf->rf_pulseid : -1);
			}
			tabledepth++;
		}
		WLAN_DFSQ_LOCK(dfs);
		empty = STAILQ_EMPTY(&(dfs->dfs_radarq));
		WLAN_DFSQ_UNLOCK(dfs);
	}
dfsfound:
	if (retval) {

		/*
		 * TODO: Instead of discarding the radar, create a workqueue
		 * if the channel change is happenning through userspace and
		 * process the radar event once the channel change is completed.
		 */

		/* Collect stats */
		dfs->wlan_dfs_stats.num_radar_detects++;
		thischan = &rs->rs_chan;
		if ((seg_id == SEG_ID_SECONDARY) &&
				(dfs_is_precac_timer_running(dfs))) {
			dfs->is_radar_during_precac = 1;
			DFS_PRINTK(
				"Radar found on second segment VHT80 freq=%d MHz\n",
				dfs->dfs_precac_secondary_freq);
		} else {
			DFS_PRINTK(
				"Radar found on channel %d (%d MHz)\n",
				thischan->ic_ieee, thischan->ic_freq);
		}

		/*
		 * If event log is on then dump the radar event queue on
		 * filter match. This can be used to collect information
		 * on false radar detection.
		 */
		if (dfs->dfs_event_log_on)
			dfs_print_radar_events(dfs);

		dfs_reset_radarq(dfs);
		dfs_reset_alldelaylines(dfs);

		DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
			"Primary channel freq = %u flags=0x%x\n",
			chan->ic_freq, chan->ic_flagext);

		if (chan->ic_freq != thischan->ic_freq)
			DFS_DPRINTK(dfs, WLAN_DEBUG_DFS1,
				"Ext channel freq = %u flags=0x%x\n",
				thischan->ic_freq,
				thischan->ic_flagext);

		dfs->dfs_phyerr_freq_min = 0x7fffffff;
		dfs->dfs_phyerr_freq_max = 0;
		dfs->dfs_phyerr_w53_counter = 0;
		if (seg_id == SEG_ID_SECONDARY) {
			dfs->wlan_dfs_stats.num_seg_two_radar_detects++;
			dfs->is_radar_found_on_secondary_seg = 1;
		}
	}

	return retval;
}
