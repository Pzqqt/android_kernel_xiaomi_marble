/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
 * DOC: For each radar pulse that the HW detects, a single radar PHY error is
 * reported to the driver. This PHY error contains information like the RSSI,
 * the pulse duration, the pulse location (primary/extension/DC) and possibly
 * FFT data.
 */

#include "../dfs.h"
#include "../dfs_zero_cac.h"
#include "../dfs_channel.h"
#include "wlan_dfs_mlme_api.h"
#include "../dfs_internal.h"

/**
 * dfs_get_event_freqwidth() - Get frequency width.
 * @dfs: Pointer to wlan_dfs structure.
 *
 * Return: Return the frequency width for the current operating channel.
 * This isn't the channel width - it's how wide the reported event may be.
 * For HT20 this is 20MHz. For HT40 on Howl and later it'll still be 20MHz
 * - the hardware returns either pri or ext channel.
 */
static inline int dfs_get_event_freqwidth(struct wlan_dfs *dfs)
{
	/* Handle edge cases during startup/transition, shouldn't happen! */
	if (!dfs)
		return 0;

	if (!dfs->dfs_curchan)
		return 0;

	/*
	 * For now, assume 20MHz wide - but this is incorrect when operating in
	 * half/quarter mode!
	 */
	return 20;
}

/**
 * dfs_get_event_freqcentre() - Get event frequency centre.
 * @dfs: Pointer to wlan_dfs structure.
 * @is_pri: detected on primary channel.
 * @is_ext: detected on extension channel.
 * @is_dc: detected at DC.
 *
 * Return the centre frequency for the current operating channel and event.
 * This is for post-Owl 11n chips which report pri/extension channel events.
 */
static inline uint16_t dfs_get_event_freqcentre(struct wlan_dfs *dfs,
		int is_pri,
		int is_ext,
		int is_dc)
{
	int chan_offset = 0, chan_width;

	/* Handle edge cases during startup/transition, shouldn't happen! */
	if (!dfs)
		return 0;
	if (!dfs->dfs_curchan)
		return 0;

	/*
	 * For wide channels, DC and ext frequencies need a bit of hand-holding
	 * based on whether it's an upper or lower channel.
	 */
	chan_width = dfs_get_event_freqwidth(dfs);

	if (WLAN_IS_CHAN_11N_HT40PLUS(dfs->dfs_curchan))
		chan_offset = chan_width;
	else if (WLAN_IS_CHAN_11N_HT40MINUS(dfs->dfs_curchan))
		chan_offset = -chan_width;
	else
		chan_offset = 0;

	/*
	 * Check for DC events first - the sowl code may just set all the bits
	 * together.
	 */
	if (is_dc) {
		/* XXX TODO: Should DC events be considered 40MHz wide here? */
		return dfs_chan2freq(
				dfs->dfs_curchan) + (chan_offset / 2);
	}

	/*
	 * For non-wide channels, the centre frequency is just dfs_ch_freq.
	 * The centre frequency for pri events is still dfs_ch_freq.
	 */
	if (is_pri)
		return dfs_chan2freq(dfs->dfs_curchan);

	if (is_ext)
		return dfs_chan2freq(dfs->dfs_curchan) + chan_width;

	return dfs_chan2freq(dfs->dfs_curchan);
}

int dfs_process_phyerr_owl(struct wlan_dfs *dfs,
		void *buf,
		uint16_t datalen,
		uint8_t rssi,
		uint8_t ext_rssi,
		uint32_t rs_tstamp,
		uint64_t fulltsf,
		struct dfs_phy_err *e)
{
	const char *cbuf = (const char *) buf;
	uint8_t dur;
	int event_width;

	dfs->wlan_dfs_stats.owl_phy_errors++;

	/*
	 * HW cannot detect extension channel radar so it only passes us primary
	 * channel radar data.
	 */
	if (datalen == 0)
		dur = 0;
	else
		dur = ((uint8_t *) cbuf)[0];

	/* This is a spurious event; toss. */
	if (rssi == 0 && dur == 0) {
		dfs->wlan_dfs_stats.datalen_discards++;
		return 0;
	}

	/* Fill out dfs_phy_err with the information we have at hand. */
	qdf_mem_set(e, sizeof(*e), 0);
	e->rssi = rssi;
	e->dur = dur;
	e->is_pri = 1;
	e->is_ext = 0;
	e->is_dc = 0;
	e->is_early = 1;
	e->fulltsf = fulltsf;
	e->rs_tstamp = rs_tstamp;

	/*
	 * Owl only ever reports events on the primary channel. It doesn't
	 * even see events on the secondary channel.
	 */
	event_width = dfs_get_event_freqwidth(dfs);
	e->freq = dfs_get_event_freqcentre(dfs, 1, 0, 0) * 1000;
	e->freq_lo = e->freq - (event_width / 2) * 1000;
	e->freq_hi = e->freq + (event_width / 2) * 1000;

	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR_SUM,
		"rssi=%u dur=%u, freq=%d MHz, freq_lo=%d MHz, freq_hi=%d MHz",
		 rssi, dur, e->freq/1000, e->freq_lo/1000,
		e->freq_hi / 1000);

	return 1;
}

int dfs_process_phyerr_sowl(struct wlan_dfs *dfs,
		void *buf,
		uint16_t datalen,
		uint8_t rssi,
		uint8_t ext_rssi,
		uint32_t rs_tstamp,
		uint64_t fulltsf,
		struct dfs_phy_err *e)
{
#define EXT_CH_RADAR_FOUND 0x02
#define PRI_CH_RADAR_FOUND 0x01
#define EXT_CH_RADAR_EARLY_FOUND 0x04
	const char *cbuf = (const char *)buf;
	uint8_t dur = 0;
	uint8_t pulse_bw_info, pulse_length_ext, pulse_length_pri;
	int pri_found = 0, ext_found = 0;
	int early_ext = 0;
	int event_width;

	/*
	 * If radar can be detected on the extension channel, datalen zero
	 * pulses are bogus, discard them.
	 */
	if (!datalen) {
		dfs->wlan_dfs_stats.datalen_discards++;
		return 0;
	}

	/* Ensure that we have at least three bytes of payload. */
	if (datalen < 3) {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			"short error frame (%d bytes)", datalen);
		dfs->wlan_dfs_stats.datalen_discards++;
		return 0;
	}

	/*
	 * Fetch the payload directly - the compiler will happily generate
	 * byte-read instructions with a const char * cbuf pointer.
	 */
	pulse_length_pri = cbuf[datalen - 3];
	pulse_length_ext = cbuf[datalen - 2];
	pulse_bw_info = cbuf[datalen - 1];

	/*
	 * Only the last 3 bits of the BW info are relevant, they indicate
	 * which channel the radar was detected in.
	 */
	pulse_bw_info &= 0x07;

	/* If pulse on DC, both primary and extension flags will be set */
	if (((pulse_bw_info & EXT_CH_RADAR_FOUND) &&
				(pulse_bw_info & PRI_CH_RADAR_FOUND))) {
		/*
		 * Conducted testing, when pulse is on DC, both pri and ext
		 * durations are reported to be same. Radiated testing, when
		 * pulse is on DC, differentpri and ext durations are reported,
		 * so take the larger of the two.
		 */
		if (pulse_length_ext >= pulse_length_pri) {
			dur = pulse_length_ext;
			ext_found = 1;
		} else {
			dur = pulse_length_pri;
			pri_found = 1;
		}
		dfs->wlan_dfs_stats.dc_phy_errors++;
	} else {
		if (pulse_bw_info & EXT_CH_RADAR_FOUND) {
			dur = pulse_length_ext;
			pri_found = 0;
			ext_found = 1;
			dfs->wlan_dfs_stats.ext_phy_errors++;
		}
		if (pulse_bw_info & PRI_CH_RADAR_FOUND) {
			dur = pulse_length_pri;
			pri_found = 1;
			ext_found = 0;
			dfs->wlan_dfs_stats.pri_phy_errors++;
		}
		if (pulse_bw_info & EXT_CH_RADAR_EARLY_FOUND) {
			dur = pulse_length_ext;
			pri_found = 0;
			ext_found = 1;
			early_ext = 1;
			dfs->wlan_dfs_stats.early_ext_phy_errors++;
			dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
				"EARLY ext channel dur=%u rssi=%u datalen=%d",
				dur, rssi, datalen);
		}
		if (!pulse_bw_info) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
				"ERROR channel dur=%u rssi=%u pulse_bw_info=0x%x datalen MOD 4 = %d",
				dur, rssi, pulse_bw_info, (datalen & 0x3));
			/*
			 * Bogus bandwidth info received in descriptor, so
			 * ignore this PHY error.
			 */
			dfs->wlan_dfs_stats.bwinfo_errors++;
			return 0;
		}
	}

	/*
	 * Always use combined RSSI reported, unless RSSI reported on
	 * extension is stronger.
	 */
	if ((ext_rssi > rssi) && (ext_rssi < 128))
		rssi = ext_rssi;

	/* Fill out the rssi/duration fields from above. */
	qdf_mem_set(e, sizeof(*e), 0);
	e->rssi = rssi;
	e->dur = dur;
	e->is_pri = pri_found;
	e->is_ext = ext_found;
	e->is_dc = !!(((pulse_bw_info & EXT_CH_RADAR_FOUND) &&
				(pulse_bw_info & PRI_CH_RADAR_FOUND)));
	e->is_early = early_ext;
	e->fulltsf = fulltsf;
	e->rs_tstamp = rs_tstamp;

	/* Sowl and later can report pri/ext events. */
	event_width = dfs_get_event_freqwidth(dfs);
	e->freq = dfs_get_event_freqcentre(dfs, e->is_pri, e->is_ext,
			e->is_dc) * 1000;
	e->freq_lo = e->freq - (event_width / 2) * 1000;
	e->freq_hi = e->freq + (event_width / 2) * 1000;

	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR_SUM,
		"pulse_bw_info=0x%x pulse_length_ext=%u pulse_length_pri=%u rssi=%u ext_rssi=%u, freq=%d MHz, freq_lo=%d MHz, freq_hi=%d MHz",
		 pulse_bw_info, pulse_length_ext, pulse_length_pri,
		rssi, ext_rssi, e->freq/1000, e->freq_lo/1000, e->freq_hi/1000);
#undef EXT_CH_RADAR_FOUND
#undef PRI_CH_RADAR_FOUND
#undef EXT_CH_RADAR_EARLY_FOUND

	return 1;
}

int dfs_process_phyerr_merlin(struct wlan_dfs *dfs,
		void *buf,
		uint16_t datalen,
		uint8_t rssi,
		uint8_t ext_rssi,
		uint32_t rs_tstamp,
		uint64_t fulltsf,
		struct dfs_phy_err *e)
{
	const char *cbuf = (const char *) buf;
	uint8_t pulse_bw_info = 0;

	/* Process using the sowl code. */
	if (!dfs_process_phyerr_sowl(dfs, buf, datalen, rssi, ext_rssi,
				rs_tstamp, fulltsf, e)) {
		return 0;
	}

	/*
	 * For osprey (and Merlin) bw_info has implication for selecting RSSI
	 * value. So re-fetch the bw_info field so the RSSI values can be
	 * appropriately overridden.
	 */
	pulse_bw_info = cbuf[datalen - 1];

	switch (pulse_bw_info & 0x03) {
	case 0x00:
		/* No radar in ctrl or ext channel */
		rssi = 0;
		break;
	case 0x01:
		/* Radar in ctrl channel */
		dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
			"RAW RSSI: rssi=%u ext_rssi=%u", rssi, ext_rssi);
		if (ext_rssi >= (rssi + 3)) {
			/*
			 * Cannot use ctrl channel RSSI if extension channel is
			 * stronger.
			 */
			rssi = 0;
		}
		break;
	case 0x02:
		/* Radar in extension channel */
		dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
			"RAW RSSI: rssi=%u ext_rssi=%u", rssi, ext_rssi);
		if (rssi >= (ext_rssi + 12)) {
			/*
			 * Cannot use extension channel RSSI if control channel
			 * is stronger
			 */
			rssi = 0;
		} else {
			rssi = ext_rssi;
		}
		break;
	case 0x03:
		/* When both are present use stronger one */
		if (rssi < ext_rssi)
			rssi = ext_rssi;
		break;
	}

	/*
	 * Override the rssi decision made by the sowl code. The rest of the
	 * fields (duration, timestamp, etc) are left untouched.
	 */
	e->rssi = rssi;

	return 1;
}

/**
 * dfs_dump_phyerr_contents() - Dump the phyerr contents.
 * @d: Phyerr buffer.
 * @len: Phyerr buf length.
 */

static void dfs_dump_phyerr_contents(const char *d, int len)
{
	int i, n, bufsize = 64;

	/*
	 * This is statically sized for a 4-digit address + 16 * 2 digit data
	 * string. It's done so the printk() passed to the kernel is an entire
	 * line, so the kernel logging code will atomically print it. Otherwise
	 * we'll end up with interleaved lines with output from other kernel
	 * threads.
	 */
	char buf[64];

	/* Initial conditions */
	buf[0] = '\n';
	n = 0;

	for (i = 0; i < len; i++) {
		if (i % 16 == 0)
			n += snprintf(buf + n, bufsize - n, "%04x: ", i);

		n += snprintf(buf + n, bufsize - n, "%02x ", d[i] & 0xff);
		if (i % 16 == 15) {
			dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "%s", buf);
			n = 0;
			buf[0] = '\0';
		}
	}

	/* Print the final line if we didn't print it above. */
	if (n != 0)
		dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "%s", buf);
}

/**
 * dfs_bump_up_bin5_pulse_dur() - Bump up to a random BIN 5 pulse duration.
 * @dfs: Pointer to wlan_dfs structure.
 * @e: Pointer to dfs_phy_err structure.
 * @slope: Slope value.
 */
static inline void dfs_bump_up_bin5_pulse_dur(
		struct wlan_dfs *dfs,
		struct dfs_phy_err *e,
		int slope)
{
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR, "old dur %d slope =%d",
			e->dur, slope);

	e->is_sw_chirp = 1;
	/* bump up to a random bin5 pulse duration */
	if (e->dur < MIN_BIN5_DUR)
		e->dur = dfs_get_random_bin5_dur(dfs, e->fulltsf);

	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR, "new dur %d", e->dur);
}

/**
 * dfs_filter_short_pulses() - Filter short pulses.
 * @dfs: Pointer to wlan_dfs structure.
 * @e: Pointer to dfs_phy_err structure.
 * @retval: Return value
 *
 * Rssi is not accurate for short pulses, so donot filter based on that for
 * short duration pulses.
 */
static inline void dfs_filter_short_pulses(
		struct wlan_dfs *dfs,
		struct dfs_phy_err *e,
		int *retval)
{
	if (dfs->dfs_caps.wlan_dfs_ext_chan_ok) {
		if ((e->rssi < dfs->dfs_rinfo.rn_minrssithresh &&
					(e->dur > MAX_DUR_FOR_LOW_RSSI)) ||
				e->dur > (dfs->dfs_rinfo.rn_maxpulsedur)) {
			dfs->wlan_dfs_stats.rssi_discards++;
			*retval = 1;
		}
	} else if (e->rssi < dfs->dfs_rinfo.rn_minrssithresh ||
			e->dur > dfs->dfs_rinfo.rn_maxpulsedur) {
		dfs->wlan_dfs_stats.rssi_discards++;
		*retval = 1;
	}

	if (*retval) {
		dfs_debug(dfs, WLAN_DEBUG_DFS1,
				"%s pulse is discarded: dur=%d, maxpulsedur=%d, rssi=%d, minrssi=%d",
				(dfs->dfs_caps.wlan_dfs_ext_chan_ok) ?
				"Extension channel" : "",
				e->dur, dfs->dfs_rinfo.rn_maxpulsedur,
				e->rssi, dfs->dfs_rinfo.rn_minrssithresh);
	}
}

/**
 * dfs_set_chan_index() - Set channel index.
 * @dfs: Pointer to wlan_dfs structure.
 * @e: Pointer to dfs_phy_err structure.
 * @event: Pointer to dfs_event structure.
 */
static inline void dfs_set_chan_index(
		struct wlan_dfs *dfs,
		struct dfs_phy_err *e,
		struct dfs_event *event)
{
	if (e->is_pri) {
		event->re_chanindex = dfs->dfs_curchan_radindex;
	} else {
		event->re_chanindex = dfs->dfs_extchan_radindex;
		dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
				"%s New extension channel event is added to queue",
				(event->re_chanindex == -1) ?
				"- phyerr on ext channel" : "");
	}
}

/**
 * dfs_is_second_seg_radar_disabled() - Check for second segment radar disabled.
 * @dfs: Pointer to wlan_dfs structure.
 * @seg_id: Segment id.
 *
 * Return: true if the second segment RADAR is enabled else false.
 */
static bool dfs_is_second_seg_radar_disabled(
		struct wlan_dfs *dfs, int seg_id)
{
	if ((seg_id == SEG_ID_SECONDARY) &&
			!(dfs->dfs_proc_phyerr & DFS_SECOND_SEGMENT_RADAR_EN)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS3,
				"Second segment radar detection is disabled");
		return true;
	}

	return false;
}

void dfs_process_phyerr(struct wlan_dfs *dfs, void *buf, uint16_t datalen,
		uint8_t r_rssi, uint8_t r_ext_rssi, uint32_t r_rs_tstamp,
		uint64_t r_fulltsf)
{
	struct dfs_event *event;
	struct dfs_phy_err e;
	int empty;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	if (dfs->dfs_ignore_dfs) {
		dfs_debug(dfs, WLAN_DEBUG_DFS1, "ignoring dfs");
		return;
	}

	/*
	 * EV 129487: If radar detection is disabled, do not process PHY error
	 * data.
	 */

	if (!(dfs->dfs_proc_phyerr & DFS_RADAR_EN)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"DFS_RADAR_EN not set in dfs->dfs_proc_phyerr");
		return;
	}

	/*
	 * The combined_rssi_ok support has been removed. This was only clear
	 * for Owl.
	 * XXX TODO: re-add this; it requires passing in the ctl/ext
	 * RSSI set from the RX status descriptor.
	 * XXX TODO : this may be done for us from the legacy phy error path in
	 * wlan_dev; please review that code.
	 */

	/*
	 * At this time we have a radar pulse that we need to examine and
	 * queue. But if dfs_process_radarevent already detected radar and set
	 * CHANNEL_INTERFERENCE flag then do not queue any more radar data.
	 * When we are in a new channel this flag will be clear and we will
	 * start queueing data for new channel. (EV74162)
	 */
	if (dfs->dfs_debug_mask & WLAN_DEBUG_DFS_PHYERR_PKT)
		dfs_dump_phyerr_contents(buf, datalen);

	if (WLAN_IS_CHAN_RADAR(dfs->dfs_curchan)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"Radar already found in the channel, do not queue radar data");
		return;
	}

	dfs->wlan_dfs_stats.total_phy_errors++;
	dfs_debug(dfs, WLAN_DEBUG_DFS2, "phyerr %d len %d",
		dfs->wlan_dfs_stats.total_phy_errors, datalen);

	/*
	 * Hardware stores this as 8 bit signed value. we will cap it at 0 if it
	 * is a negative number.
	 */
	if (r_rssi & 0x80)
		r_rssi = 0;

	if (r_ext_rssi & 0x80)
		r_ext_rssi = 0;

	qdf_mem_set(&e, sizeof(e), 0);

	/*
	 * This is a bit evil - instead of just passing in the chip version, the
	 * existing code uses a set of HAL capability bits to determine what is
	 * possible.
	 * The way I'm decoding it is thus:
	 * + DFS enhancement? Merlin or later
	 * + DFS extension channel? Sowl or later. (Howl?)
	 * + otherwise, Owl (and legacy.)
	 */
	if (dfs->dfs_caps.wlan_chip_is_bb_tlv) {
		if (dfs_process_phyerr_bb_tlv(dfs, buf, datalen, r_rssi,
			    r_ext_rssi, r_rs_tstamp, r_fulltsf, &e) == 0) {
			dfs->dfs_phyerr_reject_count++;
			return;
		}

		if (dfs->dfs_phyerr_freq_min > e.freq)
			dfs->dfs_phyerr_freq_min = e. freq;

		if (dfs->dfs_phyerr_freq_max < e.freq)
			dfs->dfs_phyerr_freq_max = e. freq;
	} else if (dfs->dfs_caps.wlan_dfs_use_enhancement) {
		if (dfs_process_phyerr_merlin(dfs, buf, datalen, r_rssi,
			    r_ext_rssi, r_rs_tstamp, r_fulltsf, &e) == 0)
			return;
	} else if (dfs->dfs_caps.wlan_dfs_ext_chan_ok) {
		if (dfs_process_phyerr_sowl(dfs, buf, datalen, r_rssi,
			    r_ext_rssi, r_rs_tstamp, r_fulltsf, &e) == 0)
			return;
	} else {
		if (dfs_process_phyerr_owl(dfs, buf, datalen, r_rssi,
			    r_ext_rssi, r_rs_tstamp, r_fulltsf, &e) == 0)
			return;
	}

	/*
	 * If the hardware supports radar reporting on the extension channel
	 * it will supply FFT data for longer radar pulses.
	 * TLV chips don't go through this software check - the hardware
	 * check should be enough.  If we want to do software checking
	 * later on then someone will have to craft an FFT parser
	 * suitable for the TLV FFT data format.
	 */
	if ((!dfs->dfs_caps.wlan_chip_is_bb_tlv) &&
			dfs->dfs_caps.wlan_dfs_ext_chan_ok) {
		/*
		 * HW has a known issue with chirping pulses injected at or
		 * around DC in 40MHz mode. Such pulses are reported with much
		 * lower durations and SW then discards them because they do
		 * not fit the minimum bin5 pulse duration. To work around this
		 * issue, if a pulse is within a 10us range of the bin5 min
		 * duration, check if the pulse is chirping. If the pulse is
		 * chirping, bump up the duration to the minimum bin5 duration.
		 * This makes sure that a valid chirping pulse will not be
		 * discarded because of incorrect low duration. TBD - Is it
		 * possible to calculate the 'real' duration of the pulse using
		 * the slope of the FFT data? TBD - Use FFT data to
		 * differentiate between radar pulses and false PHY errors.
		 * This will let us reduce the number of false alarms seen.
		 * BIN 5 chirping pulses are only for FCC or Japan MMK4 domain
		 */
		if (((dfs->dfsdomain == DFS_FCC_DOMAIN) ||
			    (dfs->dfsdomain == DFS_MKK4_DOMAIN)) &&
			(e.dur >= MAYBE_BIN5_DUR) && (e.dur < MAX_BIN5_DUR)) {
			int add_dur;
			int slope = 0, dc_found = 0;

			/*
			 * Set the event chirping flags; as we're doing an
			 * actual chirp check.
			 */
			e.do_check_chirp = 1;
			e.is_hw_chirp = 0;
			e.is_sw_chirp = 0;

			/*
			 * dfs_check_chirping() expects is_pri and is_ext to
			 * be '1' for true and '0' for false for now, as the
			 * function itself uses these values in constructing
			 * things rather than testing them
			 */
			add_dur = dfs_check_chirping(dfs, buf, datalen,
					(e.is_pri ? 1 : 0),
					(e.is_ext ? 1 : 0), &slope, &dc_found);
			if (add_dur) {
				dfs_bump_up_bin5_pulse_dur(dfs, &e, slope);
			} else {
				/* Set the duration so that it is rejected. */
				e.is_sw_chirp = 0;
				e.dur = MAX_BIN5_DUR + 100;
				dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
					"is_chirping = %d dur=%d",
					add_dur, e.dur);
			}
		} else {
			/*
			 * We have a pulse that is either bigger than
			 * MAX_BIN5_DUR or less than MAYBE_BIN5_DUR
			 */
			if ((dfs->dfsdomain == DFS_FCC_DOMAIN) ||
					(dfs->dfsdomain == DFS_MKK4_DOMAIN)) {
				/*
				 * Would this result in very large pulses
				 * wrapping around to become short pulses?
				 */
				if (e.dur >= MAX_BIN5_DUR) {
					/*
					 * Set the duration so that it is
					 * rejected.
					 */
					e.dur = MAX_BIN5_DUR + 50;
				}
			}
		}
	}

	/*
	 * Add the parsed, checked and filtered entry to the radar pulse
	 * event list.  This is then checked by dfs_radar_processevent().
	 *
	 * XXX TODO: some filtering is still done below this point - fix this!
	 */
	WLAN_DFSEVENTQ_LOCK(dfs);
	empty = STAILQ_EMPTY(&(dfs->dfs_eventq));
	WLAN_DFSEVENTQ_UNLOCK(dfs);
	if (empty)
		return;

	/*
	 * If the channel is a turbo G channel, then the event is for the
	 * adaptive radio (AR) pattern matching rather than radar detection.
	 */
	if ((dfs->dfs_curchan->dfs_ch_flags & CHANNEL_108G) == CHANNEL_108G) {
		if (!(dfs->dfs_proc_phyerr & DFS_AR_EN)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS2,
				"DFS_AR_EN not enabled");
			return;
		}
		WLAN_DFSEVENTQ_LOCK(dfs);
		event = STAILQ_FIRST(&(dfs->dfs_eventq));
		if (!event) {
			WLAN_DFSEVENTQ_UNLOCK(dfs);
			dfs_debug(dfs, WLAN_DEBUG_DFS,
				"no more events space left");
			return;
		}
		STAILQ_REMOVE_HEAD(&(dfs->dfs_eventq), re_list);
		WLAN_DFSEVENTQ_UNLOCK(dfs);
		event->re_rssi = e.rssi;
		event->re_dur = e.dur;
		event->re_full_ts = e.fulltsf;
		event->re_ts = (e.rs_tstamp) & DFS_TSMASK;
		event->re_chanindex = dfs->dfs_curchan_radindex;
		event->re_flags = 0;

		/* Handle chirp flags. */
		if (e.do_check_chirp) {
			event->re_flags |= DFS_EVENT_CHECKCHIRP;
			if (e.is_hw_chirp)
				event->re_flags |= DFS_EVENT_HW_CHIRP;
			if (e.is_sw_chirp)
				event->re_flags |= DFS_EVENT_SW_CHIRP;
		}

		WLAN_ARQ_LOCK(dfs);
		STAILQ_INSERT_TAIL(&(dfs->dfs_arq), event, re_list);
		WLAN_ARQ_UNLOCK(dfs);
	} else {
		if ((WLAN_IS_CHAN_DFS(dfs->dfs_curchan) ||
		    ((WLAN_IS_CHAN_11AC_VHT160(dfs->dfs_curchan) ||
		      WLAN_IS_CHAN_11AC_VHT80_80(dfs->dfs_curchan)) &&
		     WLAN_IS_CHAN_DFS_CFREQ2(dfs->dfs_curchan))) ||
			(dfs_is_precac_timer_running(dfs))) {

			int retval = 0;

			if (!(dfs->dfs_proc_phyerr & DFS_RADAR_EN)) {
				dfs_debug(dfs, WLAN_DEBUG_DFS3,
					"DFS_RADAR_EN not enabled");
				return;
			}

			dfs_filter_short_pulses(dfs, &e, &retval);
			if (retval)
				return;

			if (dfs_is_second_seg_radar_disabled(dfs, e.seg_id))
				return;

			/* Add the event to the list, if there's space. */
			WLAN_DFSEVENTQ_LOCK(dfs);
			event = STAILQ_FIRST(&(dfs->dfs_eventq));
			if (!event) {
				WLAN_DFSEVENTQ_UNLOCK(dfs);
				dfs_debug(dfs, WLAN_DEBUG_DFS,
					"no more events space left");
				return;
			}
			STAILQ_REMOVE_HEAD(&(dfs->dfs_eventq), re_list);
			WLAN_DFSEVENTQ_UNLOCK(dfs);

			dfs->dfs_phyerr_queued_count++;
			dfs->dfs_phyerr_w53_counter++;

			event->re_dur = e.dur;
			event->re_full_ts = e.fulltsf;
			event->re_ts = (e.rs_tstamp) & DFS_TSMASK;
			event->re_rssi = e.rssi;

			event->re_seg_id = e.seg_id;
			event->re_sidx = e.sidx;
			event->re_freq_offset_khz = e.freq_offset_khz;
			event->re_peak_mag = e.peak_mag;
			event->re_total_gain = e.total_gain;
			event->re_mb_gain = e.mb_gain;
			event->re_relpwr_db = e.relpwr_db;
			event->re_delta_diff = e.pulse_delta_diff;
			event->re_delta_peak = e.pulse_delta_peak;
			event->re_psidx_diff = e.pulse_psidx_diff;
			event->re_flags = 0;
			event->re_flags |= DFS_EVENT_VALID_PSIDX_DIFF;
			/* Handle chirp flags. */
			if (e.do_check_chirp) {
				event->re_flags |= DFS_EVENT_CHECKCHIRP;
				if (e.is_hw_chirp)
					event->re_flags |= DFS_EVENT_HW_CHIRP;
				if (e.is_sw_chirp)
					event->re_flags |= DFS_EVENT_SW_CHIRP;
			}

			/* Correctly set which channel is being reported on */
			dfs_set_chan_index(dfs, &e, event);

			WLAN_DFSQ_LOCK(dfs);
			STAILQ_INSERT_TAIL(&(dfs->dfs_radarq), event, re_list);
			WLAN_DFSQ_UNLOCK(dfs);
		}
	}

	/*
	 * Schedule the radar/AR task as appropriate.
	 * XXX isn't a lock needed for wlan_radar_tasksched?
	 */
	if (!STAILQ_EMPTY(&dfs->dfs_arq)) {
		/* XXX shouldn't this be a task/timer too? */
		dfs_process_ar_event(dfs, dfs->dfs_curchan);
	}
	if (!STAILQ_EMPTY(&dfs->dfs_radarq) && !dfs->wlan_radar_tasksched) {
		dfs->wlan_radar_tasksched = 1;
		qdf_timer_mod(&dfs->wlan_dfs_task_timer, 0);
	}
#undef EXT_CH_RADAR_FOUND
#undef PRI_CH_RADAR_FOUND
#undef EXT_CH_RADAR_EARLY_FOUND
}

#ifdef QCA_MCL_DFS_SUPPORT
void dfs_process_phyerr_filter_offload(struct wlan_dfs *dfs,
	struct radar_event_info *wlan_radar_event)
{
	struct dfs_event *event;
	int empty;
	int do_check_chirp = 0;
	int is_hw_chirp = 0;
	int is_sw_chirp = 0;
	int is_pri = 0;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	if (dfs->dfs_ignore_dfs) {
		dfs_debug(dfs, WLAN_DEBUG_DFS1, "ignoring dfs");
		return;
	}

	if (!(dfs->dfs_proc_phyerr & DFS_RADAR_EN)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"DFS_RADAR_EN not set in dfs->dfs_proc_phyerr");
		return;
	}

	if (WLAN_IS_CHAN_RADAR(dfs->dfs_curchan)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"Radar already found in the channel, do not queue radar data");
		return;
	}

	dfs->wlan_dfs_stats.total_phy_errors++;
	if (dfs->dfs_caps.wlan_chip_is_bb_tlv) {
		do_check_chirp = 1;
		is_pri = 1;
		is_hw_chirp = wlan_radar_event->pulse_is_chirp;

		if ((uint32_t) dfs->dfs_phyerr_freq_min >
		    wlan_radar_event->pulse_center_freq) {
			dfs->dfs_phyerr_freq_min =
				(int)wlan_radar_event->pulse_center_freq;
		}

		if (dfs->dfs_phyerr_freq_max <
		    (int)wlan_radar_event->pulse_center_freq) {
			dfs->dfs_phyerr_freq_max =
				(int)wlan_radar_event->pulse_center_freq;
		}
	}

	/*
	 * Now, add the parsed, checked and filtered
	 * radar phyerror event radar pulse event list.
	 * This event will then be processed by
	 * dfs_radar_processevent() to see if the pattern
	 * of pulses in radar pulse list match any radar
	 * singnature in the current regulatory domain.
	 */

	WLAN_DFSEVENTQ_LOCK(dfs);
	empty = STAILQ_EMPTY(&(dfs->dfs_eventq));
	WLAN_DFSEVENTQ_UNLOCK(dfs);
	if (empty)
		return;
	/*
	 * Add the event to the list, if there's space.
	 */
	WLAN_DFSEVENTQ_LOCK(dfs);
	event = STAILQ_FIRST(&(dfs->dfs_eventq));
	if (!event) {
		WLAN_DFSEVENTQ_UNLOCK(dfs);
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"%s: No more space left for queuing DFS Phyerror events",
			__func__);
		return;
	}
	STAILQ_REMOVE_HEAD(&(dfs->dfs_eventq), re_list);
	WLAN_DFSEVENTQ_UNLOCK(dfs);
	dfs->dfs_phyerr_queued_count++;
	dfs->dfs_phyerr_w53_counter++;
	event->re_dur = (uint8_t) wlan_radar_event->pulse_duration;
	event->re_rssi = wlan_radar_event->rssi;
	event->re_ts = wlan_radar_event->pulse_detect_ts & DFS_TSMASK;
	event->re_full_ts = (((uint64_t) wlan_radar_event->upload_fullts_high)
				<< 32) | wlan_radar_event->upload_fullts_low;

	/*
	 * Index of peak magnitude
	 */
	event->re_sidx = wlan_radar_event->peak_sidx;
	event->re_delta_diff = wlan_radar_event->delta_diff;
	event->re_delta_peak = wlan_radar_event->delta_peak;
	event->re_flags = 0;
	if (wlan_radar_event->is_psidx_diff_valid) {
		event->re_flags |= DFS_EVENT_VALID_PSIDX_DIFF;
		event->re_psidx_diff = wlan_radar_event->psidx_diff;
	}

	/*
	 * Handle chirp flags.
	 */
	if (do_check_chirp) {
		event->re_flags |= DFS_EVENT_CHECKCHIRP;
		if (is_hw_chirp)
			event->re_flags |= DFS_EVENT_HW_CHIRP;
		if (is_sw_chirp)
			event->re_flags |= DFS_EVENT_SW_CHIRP;
	}
	/*
	 * Correctly set which channel is being reported on
	 */
	if (is_pri) {
		event->re_chanindex = (uint8_t) dfs->dfs_curchan_radindex;
	} else {
		if (dfs->dfs_extchan_radindex == -1)
			dfs_debug(dfs, WLAN_DEBUG_DFS1,
				"%s phyerr on ext channel", __func__);
		event->re_chanindex = (uint8_t) dfs->dfs_extchan_radindex;
		dfs_debug(dfs, WLAN_DEBUG_DFS1,
			"%s:New extension channel event is added to queue",
			 __func__);
	}

	WLAN_DFSQ_LOCK(dfs);

	STAILQ_INSERT_TAIL(&(dfs->dfs_radarq), event, re_list);

	empty = STAILQ_EMPTY(&dfs->dfs_radarq);

	WLAN_DFSQ_UNLOCK(dfs);

	if (!empty && !dfs->wlan_radar_tasksched) {
		dfs->wlan_radar_tasksched = 1;
		qdf_timer_mod(&dfs->wlan_dfs_task_timer, 0);
	}
}
#endif

void dfs_is_radar_enabled(struct wlan_dfs *dfs, int *ignore_dfs)
{
	*ignore_dfs = dfs->dfs_ignore_dfs;
}
