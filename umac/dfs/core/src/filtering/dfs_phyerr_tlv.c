/*
 * Copyright (c) 2012, 2016-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains TLV frame processing functions.
 */

#include "../dfs.h"
#include "../dfs_channel.h"
#include "../dfs_phyerr_tlv.h"
#include "wlan_dfs_mlme_api.h"
#include "../dfs_internal.h"

#define AGC_MB_GAIN_THRESH1    68
#define AGC_OTHER_GAIN_THRESH1 40
#define AGC_MB_GAIN_THRESH2    80
#define AGC_OTHER_GAIN_THRESH2 60
#define AGC_GAIN_RSSI_THRESH   25

/*
 * Until "fastclk" is stored in the DFS configuration.
 */
#define PERE_IS_OVERSAMPLING(_dfs) \
	(_dfs->dfs_caps.wlan_chip_is_over_sampled ? 1 : 0)

/**
 * dfs_sign_extend_32() - Calculates extended 32bit value.
 * @v: Value.
 * @nb: Offset.
 *
 * Return: Returns Extend vale.
 */
static int32_t dfs_sign_extend_32(uint32_t v, int nb)
{
	uint32_t m = 1U << (nb - 1);

	/* Chop off high bits, just in case. */
	v &= v & ((1U << nb) - 1);

	/* Extend */
	return (v ^ m) - m;
}

/**
 * dfs_calc_freq_offset() - Calculate the frequency offset.
 * @sindex: signed bin index.
 * @is_oversampling: oversampling mode
 *
 * Calculate the frequency offset from the given signed bin index from the
 * radar summary report. This takes the oversampling mode into account.
 * For oversampling, each bin has resolution 44MHz/128. For non-oversampling,
 * each bin has resolution 40MHz/128. It returns kHz - ie, 1000th's of MHz.
 */
static int dfs_calc_freq_offset(int sindex, int is_oversampling)
{
	if (is_oversampling)
		return sindex * (44000 / 128);
	else
		return sindex * (40000 / 128);
}

/**
 * dfs_radar_summary_print() - Prints the Radar summary.
 * @dfs: Pointer to wlan_dfs structure.
 * @rsu: Pointer rx_radar_status structure.
 */
static void dfs_radar_summary_print(struct wlan_dfs *dfs,
		struct rx_radar_status *rsu)
{

	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"    pulsedur=%d", rsu->pulse_duration);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"    rssi=%d", rsu->rssi);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"    ischirp=%d", rsu->is_chirp);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"    sidx=%d", rsu->sidx);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"    raw tsf=%d", rsu->raw_tsf);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"    tsf_offset=%d", rsu->tsf_offset);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"    cooked tsf=%d", rsu->raw_tsf - rsu->tsf_offset);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"    frequency offset=%d.%d MHz (oversampling=%d)",
		(int)(rsu->freq_offset / 1000),
		(int)abs(rsu->freq_offset % 1000),
		PERE_IS_OVERSAMPLING(dfs));
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"    agc_total_gain=%d", rsu->agc_total_gain);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"    agc_mb_gain=%d", rsu->agc_mb_gain);
}

/**
 * dfs_radar_summary_parse() - Parse the radar summary frame.
 * @dfs: pointer to wlan_dfs structure.
 * @buf: Phyerr buffer.
 * @len: Phyerr buflen.
 * @rsu: Pointer to rx_radar_status structure.
 *
 * The frame contents _minus_ the TLV are passed in.
 */
static void dfs_radar_summary_parse(struct wlan_dfs *dfs,
		const char *buf,
		size_t len,
		struct rx_radar_status *rsu)
{
	uint32_t rs[3];

	/* Drop out if we have < 2 DWORDs available. */
	if (len < sizeof(rs)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR |
			WLAN_DEBUG_DFS_PHYERR_SUM,
			"len (%zu) < expected (%zu)!", len, sizeof(rs));
	}

	/*
	 * Since the TLVs may be unaligned for some reason
	 * we take a private copy into aligned memory.
	 * This enables us to use the HAL-like accessor macros
	 * into the DWORDs to access sub-DWORD fields.
	 */
	qdf_mem_copy(rs, buf, sizeof(rs));

	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"two 32 bit values are: %08x %08x", rs[0], rs[1]);

	/* Populate the fields from the summary report. */
	rsu->tsf_offset =
		MS(rs[RADAR_REPORT_PULSE_REG_2], RADAR_REPORT_PULSE_TSF_OFFSET);
	rsu->pulse_duration =
		MS(rs[RADAR_REPORT_PULSE_REG_2], RADAR_REPORT_PULSE_DUR);
	rsu->is_chirp =
		MS(rs[RADAR_REPORT_PULSE_REG_1], RADAR_REPORT_PULSE_IS_CHIRP);
	rsu->sidx = dfs_sign_extend_32(MS(rs[RADAR_REPORT_PULSE_REG_1],
				RADAR_REPORT_PULSE_SIDX),
			10);
	rsu->freq_offset =
		dfs_calc_freq_offset(rsu->sidx, PERE_IS_OVERSAMPLING(dfs));

	/* These are only relevant if the pulse is a chirp. */
	rsu->delta_peak = dfs_sign_extend_32(MS(rs[RADAR_REPORT_PULSE_REG_1],
		    RADAR_REPORT_PULSE_DELTA_PEAK), 6);
	rsu->delta_diff =
		MS(rs[RADAR_REPORT_PULSE_REG_1], RADAR_REPORT_PULSE_DELTA_DIFF);
	rsu->agc_total_gain =
		MS(rs[RADAR_REPORT_PULSE_REG_1], RADAR_REPORT_AGC_TOTAL_GAIN);
	rsu->agc_mb_gain = MS(rs[RADAR_REPORT_PULSE_REG_2],
		RADAR_REPORT_PULSE_AGC_MB_GAIN);
}

/**
 * dfs_radar_fft_search_report_parse () - Parse FFT report.
 * @dfs: pointer to wlan_dfs structure.
 * @buf: Phyerr buffer.
 * @len: Phyerr buflen.
 * @rsu: Pointer to rx_radar_status structure.
 */
static void dfs_radar_fft_search_report_parse(struct wlan_dfs *dfs,
		const char *buf,
		size_t len,
		struct rx_search_fft_report *rsfr)
{
	uint32_t rs[3];

	/* Drop out if we have < 2 DWORDs available. */
	if (len < sizeof(rs)) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR |
			WLAN_DEBUG_DFS_PHYERR_SUM,
			"len (%zu) < expected (%zu)!", len, sizeof(rs));
	}

	/*
	 * Since the TLVs may be unaligned for some reason we take a private
	 * copy into aligned memory. This enables us to use the HAL-like
	 * accessor macros into the DWORDs to access sub-DWORD fields.
	 */
	qdf_mem_copy(rs, buf, sizeof(rs));

	rsfr->total_gain_db =
	    MS(rs[SEARCH_FFT_REPORT_REG_1], SEARCH_FFT_REPORT_TOTAL_GAIN_DB);

	rsfr->base_pwr_db =
	    MS(rs[SEARCH_FFT_REPORT_REG_1], SEARCH_FFT_REPORT_BASE_PWR_DB);

	rsfr->fft_chn_idx =
	    MS(rs[SEARCH_FFT_REPORT_REG_1], SEARCH_FFT_REPORT_FFT_CHN_IDX);

	rsfr->peak_sidx = dfs_sign_extend_32(MS(rs[SEARCH_FFT_REPORT_REG_1],
				SEARCH_FFT_REPORT_PEAK_SIDX), 12);

	rsfr->relpwr_db =
	    MS(rs[SEARCH_FFT_REPORT_REG_2], SEARCH_FFT_REPORT_RELPWR_DB);

	rsfr->avgpwr_db =
	    MS(rs[SEARCH_FFT_REPORT_REG_2], SEARCH_FFT_REPORT_AVGPWR_DB);

	rsfr->peak_mag =
	    MS(rs[SEARCH_FFT_REPORT_REG_2], SEARCH_FFT_REPORT_PEAK_MAG);

	rsfr->num_str_bins_ib =
	    MS(rs[SEARCH_FFT_REPORT_REG_2], SEARCH_FFT_REPORT_NUM_STR_BINS_IB);

	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"two 32 bit values are: %08x %08x", rs[0], rs[1]);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"rsfr->total_gain_db = %d", rsfr->total_gain_db);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"rsfr->base_pwr_db = %d", rsfr->base_pwr_db);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"rsfr->fft_chn_idx = %d", rsfr->fft_chn_idx);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"rsfr->peak_sidx = %d", rsfr->peak_sidx);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"rsfr->relpwr_db = %d", rsfr->relpwr_db);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"rsfr->avgpwr_db = %d", rsfr->avgpwr_db);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"rsfr->peak_mag = %d", rsfr->peak_mag);
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
		"rsfr->num_str_bins_ib = %d", rsfr->num_str_bins_ib);

	if (dfs->dfs_caps.wlan_chip_is_ht160) {
		rsfr->seg_id =
		    MS(rs[SEARCH_FFT_REPORT_REG_3], SEARCH_FFT_REPORT_SEG_ID);
		dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
			"rsfr->seg_id = %d", rsfr->seg_id);
	}
}

/**
 * dfs_check_for_false_detection() -  Check for possible false detection on
 * beeliner this may also work for Cascade but parameters
 * (e.g. AGC_MB_GAIN_THRESH1) may be different for Cascade.
 * @dfs: pointer to wlan_dfs structure.
 * @rs: pointer to rx_radar_status structure.
 * @false_detect: Pointer to save false detect value.
 * @rssi: RSSI.
 */
static inline void dfs_check_for_false_detection(
		struct wlan_dfs *dfs,
		struct rx_radar_status *rs,
		bool *false_detect,
		uint8_t rssi)
{
	bool is_ht160 = false;
	bool is_false_detect = false;

	is_ht160 = dfs->dfs_caps.wlan_chip_is_ht160;
	is_false_detect = dfs->dfs_caps.wlan_chip_is_false_detect;

	if ((dfs->dfs_caps.wlan_chip_is_over_sampled == 0) &&
			(is_ht160 == 0 && is_false_detect)) {
		if ((rs->agc_mb_gain > AGC_MB_GAIN_THRESH1) &&
				((rs->agc_total_gain - rs->agc_mb_gain) <
				 AGC_OTHER_GAIN_THRESH1)) {
			*false_detect = true;
		}

		if ((rs->agc_mb_gain > AGC_MB_GAIN_THRESH2) &&
				((rs->agc_total_gain - rs->agc_mb_gain) >
				 AGC_OTHER_GAIN_THRESH2) &&
				(rssi > AGC_GAIN_RSSI_THRESH)) {
			*false_detect = true;
		}
	}

	if (*false_detect)
		dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
				"setting false_detect to TRUE because of mb/total_gain/rssi, agc_mb_gain=%d, agc_total_gain=%d, rssi=%d",
				rs->agc_mb_gain, rs->agc_total_gain, rssi);
}

/**
 * dfs_tlv_parse_frame () - Parse a Peregrine BB TLV frame.
 * @dfs: pointer to wlan_dfs structure.
 * @rs: pointer to rx_radar_status structure.
 * @rsfr: Pointer to rx_search_fft_report structure.
 * @buf: Phyerr buffer.
 * @len: Phyerr buflen.
 * @rssi: RSSI.
 * @first_short_fft_peak_mag: first short FFT peak_mag.
 * @psidx_diff: Pointer to psidx diff.
 *
 * This routine parses each TLV, prints out what's going on and calls an
 * appropriate sub-function. Since the TLV format doesn't _specify_ all TLV
 * components are DWORD aligned, we must treat them as not and access the
 * fields appropriately.
 */
static int dfs_tlv_parse_frame(struct wlan_dfs *dfs,
		struct rx_radar_status *rs,
		struct rx_search_fft_report *rsfr,
		const char *buf,
		size_t len,
		uint8_t rssi,
		int *first_short_fft_peak_mag,
		int16_t *psidx_diff)
{
	int i = 0;
	uint32_t tlv_hdr[1];
	bool false_detect = false;
	/* total search FFT reports including short and long */
	int8_t sfr_count = 0;
	int16_t first_short_fft_psidx = 0;

	*psidx_diff = 0;
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
			"total length = %zu bytes", len);
	while ((i < len) && (false_detect == false)) {
		/* Ensure we at least have four bytes. */
		if ((len - i) < sizeof(tlv_hdr)) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR |
				WLAN_DEBUG_DFS_PHYERR_SUM,
				"ran out of bytes, len=%zu, i=%d", len, i);
			return 0;
		}

		/*
		 * Copy the offset into the header, so the DWORD style access
		 * macros can be used.
		 */
		qdf_mem_copy(&tlv_hdr, buf + i, sizeof(tlv_hdr));

		dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
			"HDR: TLV SIG=0x%x, TAG=0x%x, LEN=%d bytes",
			MS(tlv_hdr[TLV_REG], TLV_SIG),
			MS(tlv_hdr[TLV_REG], TLV_TAG),
			MS(tlv_hdr[TLV_REG], TLV_LEN));

		/*
		 * Sanity check the length field is available in the remaining
		 * frame. Drop out if this isn't the case - we can't trust the
		 * rest of the TLV entries.
		 */
		if (MS(tlv_hdr[TLV_REG], TLV_LEN) + i >= len) {
			dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
				"TLV oversize: TLV LEN=%d, available=%zu, i=%d",
				 MS(tlv_hdr[TLV_REG], TLV_LEN),
				len, i);
			break;
		}

		/* Skip the TLV header - one DWORD. */
		i += sizeof(tlv_hdr);

		/* Handle the payload. */
		switch (MS(tlv_hdr[TLV_REG], TLV_SIG)) {
		case TAG_ID_RADAR_PULSE_SUMMARY: /* Radar pulse summary */
			dfs_radar_summary_parse(dfs, buf + i,
					MS(tlv_hdr[TLV_REG], TLV_LEN), rs);

			dfs_check_for_false_detection(dfs, rs, &false_detect,
					rssi);
			break;
		case TAG_ID_SEARCH_FFT_REPORT:
			sfr_count++;
			dfs_radar_fft_search_report_parse(dfs, buf + i,
					MS(tlv_hdr[TLV_REG], TLV_LEN), rsfr);

			/* we are interested in the first short FFT report's
			 * peak_mag for this value to be reliable, we must
			 * ensure that
			 * BB_srch_fft_ctrl_4.radar_fft_short_rpt_scl is set to
			 * 0.
			 */
			if (sfr_count == 1) {
				*first_short_fft_peak_mag = rsfr->peak_mag;
				first_short_fft_psidx = rsfr->peak_sidx;
			}

			/*
			 * Check for possible false detection on Peregrine.
			 * we examine search FFT report and make the following
			 * assumption as per algorithms group's input:
			 * (1) There may be multiple TLV
			 * (2) We make false detection decison solely based on
			 * the first TLV
			 * (3) If the first TLV is a serch FFT report then we
			 * check the peak_mag value.
			 * When RSSI is equal to dfs->wlan_dfs_false_rssI_thres
			 * (default 50) and peak_mag is less than
			 * 2 * dfs->wlan_dfs_peak_mag (default 40) we treat it
			 * as false detect. Please note that 50 is not a true
			 * RSSI estimate, but value indicated by HW for RF
			 * saturation event.
			 */
			if (PERE_IS_OVERSAMPLING(dfs) &&
				(sfr_count == 1) &&
				(rssi == dfs->wlan_dfs_false_rssi_thres) &&
				(rsfr->peak_mag < (2 * dfs->wlan_dfs_peak_mag))
				) {
				false_detect = true;
				dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
					"setting false_detect to TRUE because of false_rssi_thres");
			}

			/*
			 * The first FFT report indicated by (sfr_count == 1)
			 * should correspond to the first short FFT report from
			 * HW and the second FFT report indicated by
			 * (sfr_count == 2) should correspond to the first long
			 * FFT report from HW for the same pulse. The short and
			 * log FFT reports have a factor of 4 difference in
			 * resolution; hence the need to multiply by 4 when
			 * computing the psidx_diff.
			 */
			if (sfr_count == 2)
				*psidx_diff = rsfr->peak_sidx -
					      4 * first_short_fft_psidx;

			break;
		default:
			dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR,
				"unknown entry, SIG=0x%02x",
				 MS(tlv_hdr[TLV_REG], TLV_SIG));
		}

		/* Skip the payload. */
		i += MS(tlv_hdr[TLV_REG], TLV_LEN);
	}
	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR, "done");

	return false_detect ? 0 : 1;
}

/**
 * dfs_tlv_calc_freq_info() - Calculate the channel centre in MHz.
 * @dfs: pointer to wlan_dfs structure.
 * @rs: pointer to rx_radar_status structure.
 *
 * Return: Returns the channel center.
 */
static int dfs_tlv_calc_freq_info(struct wlan_dfs *dfs,
		struct rx_radar_status *rs)
{
	uint32_t chan_centre;
	uint32_t chan_width;
	int chan_offset;

	/* For now, just handle up to VHT80 correctly. */
	if (!dfs->dfs_curchan) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs_curchan is null");
		return 0;
		/*
		 * For now, the only 11ac channel with freq1/freq2 setup is
		 * VHT80. Should have a flag macro to check this!
		 */
	} else if (WLAN_IS_CHAN_11AC_VHT80(dfs->dfs_curchan)) {
		/*
		 * 11AC, so cfreq1/cfreq2 are setup.
		 * If it's 80+80 this won't work - need to use seg
		 * appropriately!
		 */
		chan_centre = dfs_mlme_ieee2mhz(dfs->dfs_pdev_obj,
				dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg1,
				dfs->dfs_curchan->dfs_ch_flags);
	} else {
		/*
		 * HT20/HT40.
		 * This is hard-coded - it should be 5 or 10 for half/quarter
		 * appropriately.
		 */
		chan_width = 20;

		/* Grab default channel centre. */
		chan_centre = dfs_chan2freq(dfs->dfs_curchan);

		/* Calculate offset based on HT40U/HT40D and VHT40U/VHT40D. */
		if (WLAN_IS_CHAN_11N_HT40PLUS(dfs->dfs_curchan) ||
			dfs->dfs_curchan->dfs_ch_flags &
			WLAN_CHAN_VHT40PLUS)
			chan_offset = chan_width;
		else if (WLAN_IS_CHAN_11N_HT40MINUS(dfs->dfs_curchan) ||
			dfs->dfs_curchan->dfs_ch_flags &
			WLAN_CHAN_VHT40MINUS)
			chan_offset = -chan_width;
		else
			chan_offset = 0;

		/* Calculate new _real_ channel centre. */
		chan_centre += (chan_offset / 2);
	}

	/* Return ev_chan_centre in MHz. */
	return chan_centre;
}

/**
 * dfs_tlv_calc_event_freq_pulse() - Calculate the centre frequency and
 *                                   low/high range for a radar pulse event.
 * @dfs: pointer to wlan_dfs structure.
 * @rs: pointer to rx_radar_status structure.
 * @freq_centre: center frequency
 * @freq_lo: lower bounds of frequency.
 * @freq_hi: upper bounds of frequency.
 *
 * XXX TODO: Handle half/quarter rates correctly!
 * XXX TODO: handle VHT160 correctly!
 * XXX TODO: handle VHT80+80 correctly!
 *
 * Return: Returns 1.
 */
static int dfs_tlv_calc_event_freq_pulse(struct wlan_dfs *dfs,
		struct rx_radar_status *rs,
		uint32_t *freq_centre,
		uint32_t *freq_lo,
		uint32_t *freq_hi)
{
	int chan_width;
	int chan_centre;

	/* Fetch the channel centre frequency in MHz. */
	chan_centre = dfs_tlv_calc_freq_info(dfs, rs);

	/* Convert to KHz. */
	chan_centre *= 1000;

	/*
	 * XXX hard-code event width to be 2 * bin size for now;
	 * XXX this needs to take into account the core clock speed
	 * XXX for half/quarter rate mode.
	 */
	if (PERE_IS_OVERSAMPLING(dfs))
		chan_width = (44000 * 2 / 128);
	else
		chan_width = (40000 * 2 / 128);

	/* XXX adjust chan_width for half/quarter rate! */

	/* Now we can do the math to figure out the correct channel range. */
	(*freq_centre) = (uint32_t) (chan_centre + rs->freq_offset);
	(*freq_lo) = (uint32_t) ((chan_centre + rs->freq_offset) - chan_width);
	(*freq_hi) = (uint32_t) ((chan_centre + rs->freq_offset) + chan_width);

	return 1;
}

/**
 * dfs_tlv_calc_event_freq_chirp() - Calculate the event freq.
 * @dfs: pointer to wlan_dfs structure.
 * @rs: pointer to rx_radar_status structure.
 * @freq_centre: center frequency
 * @freq_lo: lower bounds of frequency.
 * @freq_hi: upper bounds of frequency.
 *
 * The chirp bandwidth in KHz is defined as:
 * totalBW(KHz) = delta_peak(mean)
 *    * [ (bin resolution in KHz) / (radar_fft_long_period in uS) ]
 *    * pulse_duration (us)
 * The bin resolution depends upon oversampling.
 * For now, we treat the radar_fft_long_period as a hard-coded 8uS.
 *
 * Return: Returns 1
 */
static int dfs_tlv_calc_event_freq_chirp(struct wlan_dfs *dfs,
		struct rx_radar_status *rs,
		uint32_t *freq_centre,
		uint32_t *freq_lo,
		uint32_t *freq_hi)
{
	int32_t bin_resolution; /* KHz * 100 */
	int32_t radar_fft_long_period = 8; /* microseconds */
	int32_t delta_peak;
	int32_t pulse_duration;
	int32_t total_bw;
	int32_t chan_centre;
	int32_t freq_1, freq_2;

	/*
	 * KHz isn't enough resolution here!
	 * So treat it as deci-hertz (10Hz) and convert back to KHz later.
	 */

	if (PERE_IS_OVERSAMPLING(dfs))
		bin_resolution = (OVER_SAMPLING_FREQ * HUNDRED) / NUM_BINS;
	else
		bin_resolution = (SAMPLING_FREQ * HUNDRED) / NUM_BINS;

	delta_peak = rs->delta_peak;
	pulse_duration = rs->pulse_duration;

	total_bw = delta_peak * (bin_resolution / radar_fft_long_period) *
		pulse_duration;

	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR | WLAN_DEBUG_DFS_PHYERR_SUM,
		"delta_peak=%d, pulse_duration=%d, bin_resolution=%d.%dKHz, radar_fft_long_period=%d, total_bw=%d.%ldKHz",
		delta_peak, pulse_duration, bin_resolution / THOUSAND,
		bin_resolution % THOUSAND, radar_fft_long_period,
		total_bw / HUNDRED,
		(long)abs(total_bw % HUNDRED));

	total_bw /= HUNDRED; /* back to KHz */
	/* Grab the channel centre frequency in MHz. */
	chan_centre = dfs_tlv_calc_freq_info(dfs, rs);

	/* Early abort! */
	if (chan_centre == 0) {
		(*freq_centre) = 0;
		return 0;
	}

	/* Convert to KHz. */
	chan_centre *= THOUSAND;

	/*
	 * Sidx is the starting frequency; total_bw is a signed value and for
	 * negative chirps (ie, moving down in frequency rather than up) the end
	 * frequency may be less than the start frequency.
	 */
	if (total_bw > 0) {
		freq_1 = chan_centre + rs->freq_offset;
		freq_2 = chan_centre + rs->freq_offset + total_bw;
	} else {
		freq_1 = chan_centre + rs->freq_offset + total_bw;
		freq_2 = chan_centre + rs->freq_offset;
	}

	(*freq_lo) = (uint32_t)(freq_1);
	(*freq_hi) = (uint32_t)(freq_2);
	(*freq_centre) = (uint32_t) (freq_1 + (abs(total_bw) / 2));

	return 1;
}

/**
 * dfs_tlv_calc_event_freq() - Calculate the centre and band edge frequencies
 *                             of the given radar event.
 * @dfs: Pointer to wlan_dfs structure.
 * @rs: Pointer to rx_radar_status structure.
 * @freq_centre: Center frequency
 * @freq_lo: Lower bounds of frequency.
 * @freq_hi: Upper bounds of frequency.
 */
static int dfs_tlv_calc_event_freq(struct wlan_dfs *dfs,
		struct rx_radar_status *rs,
		uint32_t *freq_centre,
		uint32_t *freq_lo,
		uint32_t *freq_hi)
{
	if (rs->is_chirp)
		return dfs_tlv_calc_event_freq_chirp(dfs, rs, freq_centre,
				freq_lo, freq_hi);
	else
		return dfs_tlv_calc_event_freq_pulse(dfs, rs, freq_centre,
				freq_lo, freq_hi);
}

int dfs_process_phyerr_bb_tlv(struct wlan_dfs *dfs,
		void *buf,
		uint16_t datalen,
		uint8_t rssi,
		uint8_t ext_rssi,
		uint32_t rs_tstamp,
		uint64_t fulltsf,
		struct dfs_phy_err *e)
{
	struct rx_radar_status rs;
	struct rx_search_fft_report rsfr;
	int first_short_fft_peak_mag = 0;
	int16_t psidx_diff;

	qdf_mem_zero(&rs, sizeof(rs));
	qdf_mem_zero(&rsfr, sizeof(rsfr));

	/*
	 * Add the ppdu_start/ppdu_end fields given to us by the upper layers.
	 * The firmware gives us a summary set of parameters rather than the
	 * whole PPDU_START/PPDU_END descriptor contenst.
	 */
	rs.rssi = rssi;
	rs.raw_tsf = rs_tstamp;

	/* Try parsing the TLV set. */
	if (!dfs_tlv_parse_frame(dfs, &rs, &rsfr, buf, datalen, rssi,
				&first_short_fft_peak_mag, &psidx_diff))
		return 0;

	/* For debugging, print what we have parsed. */
	dfs_radar_summary_print(dfs, &rs);

	/* Populate dfs_phy_err from rs. */
	qdf_mem_set(e, sizeof(*e), 0);
	e->rssi = rs.rssi;
	e->dur = rs.pulse_duration;
	e->is_pri = 1; /* Always PRI for now */
	e->is_ext = 0;
	e->is_dc = 0;
	e->is_early = 0;

	/*
	 * XXX TODO: add a "chirp detection enabled" capability or config bit
	 * somewhere, in case for some reason the hardware chirp detection AND
	 * FFTs are disabled.
	 * For now, assume this hardware always does chirp detection.
	 */
	e->do_check_chirp = 1;
	e->is_hw_chirp = !!(rs.is_chirp);
	e->is_sw_chirp = 0; /* We don't yet do software chirp checking */

	e->fulltsf = fulltsf;
	e->rs_tstamp = rs.raw_tsf - rs.tsf_offset;

	/* XXX error check */
	(void)dfs_tlv_calc_event_freq(dfs, &rs, &e->freq, &e->freq_lo,
			&e->freq_hi);

	e->seg_id = rsfr.seg_id;
	e->sidx = rs.sidx;
	e->freq_offset_khz = rs.freq_offset;
	e->peak_mag = first_short_fft_peak_mag;
	e->total_gain = rs.agc_total_gain;
	e->mb_gain = rs.agc_mb_gain;
	e->relpwr_db = rsfr.relpwr_db;
	e->pulse_delta_peak = rs.delta_peak;
	e->pulse_psidx_diff = psidx_diff;
	e->pulse_delta_diff = rs.delta_diff;

	dfs_debug(dfs, WLAN_DEBUG_DFS_PHYERR_SUM,
		"fbin=%d, freq=%d.%d MHz, raw tsf=%u, offset=%d, cooked tsf=%u, rssi=%d, dur=%d, is_chirp=%d, fulltsf=%llu, freq=%d.%d MHz, freq_lo=%d.%dMHz, freq_hi=%d.%d MHz",
		 rs.sidx, (int) (rs.freq_offset / 1000),
		(int) abs(rs.freq_offset % 1000), rs.raw_tsf, rs.tsf_offset,
		e->rs_tstamp, rs.rssi, rs.pulse_duration, (int)rs.is_chirp,
		(unsigned long long) fulltsf, (int)e->freq / 1000,
		(int) abs(e->freq) % 1000, (int)e->freq_lo / 1000,
		(int) abs(e->freq_lo) % 1000, (int)e->freq_hi / 1000,
		(int) abs(e->freq_hi) % 1000);

	dfs_debug(dfs, WLAN_DEBUG_DFS_FALSE_DET,
		"ts=%u, dur=%d, rssi=%d, freq_offset=%d.%dMHz, is_chirp=%d, seg_id=%d, peak_mag=%d, total_gain=%d, mb_gain=%d, relpwr_db=%d, delta_peak=%d, delta_diff=%d, psidx_diff=%d",
		e->rs_tstamp, rs.pulse_duration, rs.rssi,
		(int)e->freq_offset_khz / 1000,
		(int)abs(e->freq_offset_khz) % 1000, (int)rs.is_chirp,
		rsfr.seg_id, rsfr.peak_mag, rs.agc_total_gain, rs.agc_mb_gain,
		rsfr.relpwr_db,
		rs.delta_peak,
		rs.delta_diff,
		psidx_diff);

	return 1;
}
