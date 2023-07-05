/*
 * Copyright (c) 2011-2012, 2016-2018 The Linux Foundation. All rights reserved.
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
 * DOC: This file has dfs capability, dfs pulse structures.
 */

#ifndef _DFS_STRUCTS_H_
#define _DFS_STRUCTS_H_

/**
 * This represents the general case of the radar PHY configuration,
 * across all chips.
 *
 * It's then up to each chip layer to translate to/from this
 * (eg to HAL_PHYERR_PARAM for the HAL case.)
 */

#define WLAN_DFS_PHYERR_PARAM_NOVAL   0xFFFF
#define WLAN_DFS_PHYERR_PARAM_ENABLE  0x8000

/**
 * For the dfs_nol_clist_update() method - this is the
 * update command.
 */
enum {
	DFS_NOL_CLIST_CMD_NONE      = 0x0,
	DFS_NOL_CLIST_CMD_UPDATE    = 0x1,
};

/**
 * struct dfs_pulse - DFS pulses.
 * @rp_numpulses:         Num of pulses in radar burst.
 * @rp_pulsedur:          Duration of each pulse in usecs.
 * @rp_pulsefreq:         Frequency of pulses in burst.
 * @rp_max_pulsefreq:     Frequency of pulses in burst.
 * @rp_patterntype:       fixed or variable pattern type.
 * @rp_pulsevar:          Time variation of pulse duration for matched
 *                        filter (single-sided) in usecs.
 * @rp_threshold:         Threshold for MF output to indicate radar match.
 * @rp_mindur:            Min pulse duration to be considered for this pulse
 *                        type.
 * @rp_maxdur:            Min pulse duration to be considered for this pulse
 *                        type.
 * @rp_rssithresh:        Minimum rssi to be considered a radar pulse.
 * @rp_meanoffset:        Offset for timing adjustment.
 * @rp_rssimargin:        rssi threshold margin. In Turbo Mode HW reports
 *                        rssi 3dBm. lower than in non TURBO mode. This
 *                        will be used to offset that diff.
 * @rp_ignore_pri_window: Ignore PRI window.
 * @rp_sidx_spread:       To reduce false detection use sidx spread. For HT160,
 *                        for consistency, push all pulses at center of the
 *                        channel to 80MHz ext when both segments are DFS.
 *                        Maximum SIDX value spread in a matched sequence
 *                        excluding FCC Bin 5.
 * @rp_check_delta_peak:  This is mainly used for ETSI Type 4 5MHz chirp pulses
 *                        which HW cnanot identify.
 *                        Reliably as chirping but can correctly characterize
 *                        these with delta_peak non-zero.
 *                        Is delta_peak check required for this filter.
 * @rp_pulseid:           Unique ID for identifying filter.
 */
struct dfs_pulse {
	uint32_t  rp_numpulses;
	uint32_t  rp_pulsedur;
	uint32_t  rp_pulsefreq;
	uint32_t  rp_max_pulsefreq;
	uint32_t  rp_patterntype;
	uint32_t  rp_pulsevar;
	uint32_t  rp_threshold;
	uint32_t  rp_mindur;
	uint32_t  rp_maxdur;
	uint32_t  rp_rssithresh;
	uint32_t  rp_meanoffset;
	int32_t   rp_rssimargin;
	uint32_t  rp_ignore_pri_window;
	uint16_t  rp_sidx_spread;
	int8_t    rp_check_delta_peak;
	uint16_t  rp_pulseid;
};

/**
 * struct dfs_bin5pulse - DFS bin5 pulse.
 * @b5_threshold:    Number of bin5 pulses to indicate detection.
 * @b5_mindur:       Min duration for a bin5 pulse.
 * @b5_maxdur:       Max duration for a bin5 pulse.
 * @b5_timewindow:   Window over which to count bin5 pulses.
 * @b5_rssithresh:   Min rssi to be considered a pulse.
 * @b5_rssimargin:   rssi threshold margin. In Turbo Mode HW reports rssi 3dB
 */
struct dfs_bin5pulse {
	uint32_t  b5_threshold;
	uint32_t  b5_mindur;
	uint32_t  b5_maxdur;
	uint32_t  b5_timewindow;
	uint32_t  b5_rssithresh;
	uint32_t  b5_rssimargin;
};

/**
 * wlan_dfs_phyerr_init_noval() - Fill wlan_dfs_phyerr_param with 0xFF.
 * @pe: Pointer to wlan_dfs_phyerr_param structure.
 */
static inline void wlan_dfs_phyerr_init_noval(struct wlan_dfs_phyerr_param *pe)
{
	pe->pe_firpwr = WLAN_DFS_PHYERR_PARAM_NOVAL;
	pe->pe_rrssi = WLAN_DFS_PHYERR_PARAM_NOVAL;
	pe->pe_height = WLAN_DFS_PHYERR_PARAM_NOVAL;
	pe->pe_prssi = WLAN_DFS_PHYERR_PARAM_NOVAL;
	pe->pe_inband = WLAN_DFS_PHYERR_PARAM_NOVAL;
	pe->pe_relpwr = WLAN_DFS_PHYERR_PARAM_NOVAL;
	pe->pe_relstep = WLAN_DFS_PHYERR_PARAM_NOVAL;
	pe->pe_maxlen = WLAN_DFS_PHYERR_PARAM_NOVAL;
}

/**
 * struct wlan_dfs_radar_tab_info - Radar table information.
 * @dfsdomain:         DFS domain.
 * @numradars:         Number of radars.
 * @dfs_radars:        Pointer to dfs_pulse structure.
 * @numb5radars:       NUM5 radars.
 * @b5pulses:          BIN5 radars.
 * @dfs_defaultparams: phyerr params.
 */
struct wlan_dfs_radar_tab_info {
	uint32_t          dfsdomain;
	int               numradars;
	struct dfs_pulse *dfs_radars;
	int               numb5radars;
	struct dfs_bin5pulse *b5pulses;
	struct wlan_dfs_phyerr_param dfs_defaultparams;
};

#endif  /* _DFS_STRUCTS_H_ */
