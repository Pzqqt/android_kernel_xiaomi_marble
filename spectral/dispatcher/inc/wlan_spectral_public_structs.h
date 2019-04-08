/*
 * Copyright (c) 2011,2017-2019 The Linux Foundation. All rights reserved.
 *
 *
 * Permission to use, copy, modify, and/or distribute this software for
 * any purpose with or without fee is hereby granted, provided that the
 * above copyright notice and this permission notice appear in all
 * copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 * WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include <qdf_types.h>
#include "wlan_dfs_ioctl.h"
#include <spectral_ioctl.h>

#ifndef __KERNEL__
#include <math.h>
#endif /*  __KERNEL__  */

#ifndef _WLAN_SPECTRAL_PUBLIC_STRUCTS_H_
#define _WLAN_SPECTRAL_PUBLIC_STRUCTS_H_

#ifndef AH_MAX_CHAINS
#define AH_MAX_CHAINS 3
#endif

#define MAX_NUM_CHANNELS             255
#define SPECTRAL_PHYERR_PARAM_NOVAL  65535

#ifdef SPECTRAL_USE_EMU_DEFAULTS
/* Use defaults from emulation */
#define SPECTRAL_SCAN_ACTIVE_DEFAULT           (0x0)
#define SPECTRAL_SCAN_ENABLE_DEFAULT           (0x0)
#define SPECTRAL_SCAN_COUNT_DEFAULT            (0x0)
#define SPECTRAL_SCAN_PERIOD_DEFAULT           (250)
#define SPECTRAL_SCAN_PRIORITY_DEFAULT         (0x1)
#define SPECTRAL_SCAN_FFT_SIZE_DEFAULT         (0x7)
#define SPECTRAL_SCAN_GC_ENA_DEFAULT           (0x1)
#define SPECTRAL_SCAN_RESTART_ENA_DEFAULT      (0x0)
#define SPECTRAL_SCAN_NOISE_FLOOR_REF_DEFAULT  (0xa0)
#define SPECTRAL_SCAN_INIT_DELAY_DEFAULT       (0x50)
#define SPECTRAL_SCAN_NB_TONE_THR_DEFAULT      (0xc)
#define SPECTRAL_SCAN_STR_BIN_THR_DEFAULT      (0x7)
#define SPECTRAL_SCAN_WB_RPT_MODE_DEFAULT      (0x0)
#define SPECTRAL_SCAN_RSSI_RPT_MODE_DEFAULT    (0x1)
#define SPECTRAL_SCAN_RSSI_THR_DEFAULT         (0xf)
#define SPECTRAL_SCAN_PWR_FORMAT_DEFAULT       (0x1)
#define SPECTRAL_SCAN_RPT_MODE_DEFAULT         (0x2)
#define SPECTRAL_SCAN_BIN_SCALE_DEFAULT        (0x1)
#define SPECTRAL_SCAN_DBM_ADJ_DEFAULT          (0x0)
#define SPECTRAL_SCAN_CHN_MASK_DEFAULT         (0x1)
#else
/*
 * Static default values for spectral state and configuration.
 * These definitions should be treated as temporary. Ideally,
 * we should get the defaults from firmware - this will be discussed.
 *
 * Use defaults from Spectral Hardware Micro-Architecture
 * document (v1.0)
 */
#define SPECTRAL_SCAN_ACTIVE_DEFAULT           (0)
#define SPECTRAL_SCAN_ENABLE_DEFAULT           (0)
#define SPECTRAL_SCAN_COUNT_DEFAULT            (0)
#define SPECTRAL_SCAN_PERIOD_GEN_I_DEFAULT     (35)
#define SPECTRAL_SCAN_PERIOD_GEN_II_DEFAULT    (35)
#define SPECTRAL_SCAN_PERIOD_GEN_III_DEFAULT   (224)
#define SPECTRAL_SCAN_PRIORITY_DEFAULT         (1)
#define SPECTRAL_SCAN_FFT_SIZE_DEFAULT         (7)
#define SPECTRAL_SCAN_GC_ENA_DEFAULT           (1)
#define SPECTRAL_SCAN_RESTART_ENA_DEFAULT      (0)
#define SPECTRAL_SCAN_NOISE_FLOOR_REF_DEFAULT  (-96)
#define SPECTRAL_SCAN_INIT_DELAY_DEFAULT       (80)
#define SPECTRAL_SCAN_NB_TONE_THR_DEFAULT      (12)
#define SPECTRAL_SCAN_STR_BIN_THR_DEFAULT      (8)
#define SPECTRAL_SCAN_WB_RPT_MODE_DEFAULT      (0)
#define SPECTRAL_SCAN_RSSI_RPT_MODE_DEFAULT    (0)
#define SPECTRAL_SCAN_RSSI_THR_DEFAULT         (0xf0)
#define SPECTRAL_SCAN_PWR_FORMAT_DEFAULT       (0)
#define SPECTRAL_SCAN_RPT_MODE_DEFAULT         (2)
#define SPECTRAL_SCAN_BIN_SCALE_DEFAULT        (1)
#define SPECTRAL_SCAN_DBM_ADJ_DEFAULT          (1)
#define SPECTRAL_SCAN_CHN_MASK_DEFAULT         (1)
#endif				/* SPECTRAL_USE_EMU_DEFAULTS */

/* The below two definitions apply only to pre-11ac chipsets */
#define SPECTRAL_SCAN_SHORT_REPORT_DEFAULT     (1)
#define SPECTRAL_SCAN_FFT_PERIOD_DEFAULT       (1)

/*
 * Definitions to help in scaling of gen3 linear format Spectral bins to values
 * similar to those from gen2 chipsets.
 */

/*
 * Max gain for QCA9984. Since this chipset is a prime representative of gen2
 * chipsets, it is chosen for this value.
 */
#define SPECTRAL_QCA9984_MAX_GAIN                               (78)

/* Temporary section for hard-coded values. These need to come from FW. */

/* Max gain for IPQ8074 */
#define SPECTRAL_IPQ8074_DEFAULT_MAX_GAIN_HARDCODE              (62)

/*
 * Section for values needing tuning per customer platform. These too may need
 * to come from FW. To be considered as hard-coded for now.
 */

/*
 * If customers have a different gain line up than QCA reference designs for
 * IPQ8074 and/or QCA9984, they may have to tune the low level threshold and
 * the RSSI threshold.
 */
#define SPECTRAL_SCALING_LOW_LEVEL_OFFSET                       (7)
#define SPECTRAL_SCALING_RSSI_THRESH                            (5)

/*
 * If customers set the AGC backoff differently, they may have to tune the high
 * level threshold.
 */
#define SPECTRAL_SCALING_HIGH_LEVEL_OFFSET                      (5)

/* End of section for values needing fine tuning. */
/* End of temporary section for hard-coded values */

/**
 * enum wlan_cfg80211_spectral_vendorcmd_handler_idx - Indices to cfg80211
 * spectral vendor command handlers
 * @SPECTRAL_SCAN_START_HANDLER_IDX:  Index to SPECTRAL_SCAN_START handler
 * @SPECTRAL_SCAN_STOP_HANDLER_IDX:  Index to SPECTRAL_SCAN_STOP handler
 * @SPECTRAL_SCAN_GET_CONFIG_HANDLER_IDX: Index to SPECTRAL_SCAN_GET_CONFIG
 * handler
 * @SPECTRAL_SCAN_GET_DIAG_STATS_HANDLER_IDX: Index to
 * SPECTRAL_SCAN_GET_DIAG_STATS handler
 * @SPECTRAL_SCAN_GET_CAP_HANDLER_IDX: Index to SPECTRAL_SCAN_GET_CAP handler
 * @SPECTRAL_SCAN_GET_STATUS_HANDLER_IDX: Index to SPECTRAL_SCAN_GET_STATUS
 * handler
 * @SPECTRAL_SCAN_VENDOR_CMD_HANDLER_MAX: Number of cfg80211 spectral
 * vendor command handlers supported
 */
enum wlan_cfg80211_spectral_vendorcmd_handler_idx {
	SPECTRAL_SCAN_START_HANDLER_IDX,
	SPECTRAL_SCAN_STOP_HANDLER_IDX,
	SPECTRAL_SCAN_GET_CONFIG_HANDLER_IDX,
	SPECTRAL_SCAN_GET_DIAG_STATS_HANDLER_IDX,
	SPECTRAL_SCAN_GET_CAP_HANDLER_IDX,
	SPECTRAL_SCAN_GET_STATUS_HANDLER_IDX,
	SPECTRAL_SCAN_VENDOR_CMD_HANDLER_MAX,
};

/**
 * enum spectral_debug - Spectral debug level
 * @DEBUG_SPECTRAL:  Minimal SPECTRAL debug
 * @DEBUG_SPECTRAL1: Normal SPECTRAL debug
 * @DEBUG_SPECTRAL2: Maximal SPECTRAL debug
 * @DEBUG_SPECTRAL3: Matched filterID display
 * @DEBUG_SPECTRAL4: One time dump of FFT report
 */
enum spectral_debug {
	DEBUG_SPECTRAL = 0x00000100,
	DEBUG_SPECTRAL1 = 0x00000200,
	DEBUG_SPECTRAL2 = 0x00000400,
	DEBUG_SPECTRAL3 = 0x00000800,
	DEBUG_SPECTRAL4 = 0x00001000,
};

/**
 * enum spectral_capability_type - Spectral capability type
 * @SPECTRAL_CAP_PHYDIAG:              Phydiag capability
 * @SPECTRAL_CAP_RADAR:                Radar detection capability
 * @SPECTRAL_CAP_SPECTRAL_SCAN:        Spectral capability
 * @SPECTRAL_CAP_ADVNCD_SPECTRAL_SCAN: Advanced spectral capability
 */
enum spectral_capability_type {
	SPECTRAL_CAP_PHYDIAG,
	SPECTRAL_CAP_RADAR,
	SPECTRAL_CAP_SPECTRAL_SCAN,
	SPECTRAL_CAP_ADVNCD_SPECTRAL_SCAN,
};

/**
 * struct spectral_chan_stats - channel status info
 * @cycle_count:         Cycle count
 * @channel_load:        Channel load
 * @per:                 Period
 * @noisefloor:          Noise floor
 * @comp_usablity:       Computed usability
 * @maxregpower:         Maximum allowed regulatory power
 * @comp_usablity_sec80: Computed usability of secondary 80 Mhz
 * @maxregpower_sec80:   Max regulatory power of secondary 80 Mhz
 */
struct spectral_chan_stats {
	int cycle_count;
	int channel_load;
	int per;
	int noisefloor;
	uint16_t comp_usablity;
	int8_t maxregpower;
	uint16_t comp_usablity_sec80;
	int8_t maxregpower_sec80;
};

/**
 * struct spectral_diag_stats - spectral diag stats
 * @spectral_mismatch:             Spectral TLV signature mismatches
 * @spectral_sec80_sfft_insufflen: Insufficient length when parsing for
 *                                 Secondary 80 Search FFT report
 * @spectral_no_sec80_sfft:        Secondary 80 Search FFT report
 *                                 TLV not found
 * @spectral_vhtseg1id_mismatch:   VHT Operation Segment 1 ID
 *                                 mismatches in Search FFT report
 * @spectral_vhtseg2id_mismatch:   VHT Operation Segment 2 ID
 *                                 mismatches in Search FFT report
 * @spectral_invalid_detector_id:  Invalid detector id
 */
struct spectral_diag_stats {
	uint64_t spectral_mismatch;
	uint64_t spectral_sec80_sfft_insufflen;
	uint64_t spectral_no_sec80_sfft;
	uint64_t spectral_vhtseg1id_mismatch;
	uint64_t spectral_vhtseg2id_mismatch;
	uint64_t spectral_invalid_detector_id;
};

/**
 * struct spectral_config - spectral config parameters
 * @ss_fft_period:        Skip interval for FFT reports
 * @ss_period:            Spectral scan period
 * @ss_count:             # of reports to return from ss_active
 * @ss_short_report:      Set to report only 1 set of FFT results
 * @radar_bin_thresh_sel: Select threshold to classify strong bin for FFT
 * @ss_spectral_pri:      Priority, and are we doing a noise power cal ?
 * @ss_fft_size:          Defines the number of FFT data points to compute,
 *                        defined as a log index num_fft_pts =
 *                        2^ss_fft_size
 * @ss_gc_ena:            Set, to enable targeted gain change before
 *                        starting the spectral scan FFT
 * @ss_restart_ena:       Set, to enable abort of receive frames when in high
 *                        priority and a spectral scan is queued
 * @ss_noise_floor_ref:   Noise floor reference number (signed) for the
 *                        calculation of bin power (dBm) Though stored as an
 *                        unsigned this should be treated as a signed 8-bit int.
 * @ss_init_delay:        Disallow spectral scan triggers after tx/rx packets
 *                        by setting this delay value to roughly SIFS time
 *                        period or greater Delay timer count in units of 0.25us
 * @ss_nb_tone_thr:       Number of strong bins (inclusive) per sub-channel,
 *                        below which a signal is declared a narrowband tone
 * @ss_str_bin_thr:       Bin/max_bin ratio threshold over which a bin is
 *                        declared strong (for spectral scan bandwidth analysis)
 * @ss_wb_rpt_mode:       Set this bit to report spectral scans as EXT_BLOCKER
 *                        (phy_error=36), if none of the sub-channels are
 *                        deemed narrowband
 * @ss_rssi_rpt_mode:     Set this bit to report spectral scans as EXT_BLOCKER
 *                        (phy_error=36), if the ADC RSSI is below the
 *                        threshold ss_rssi_thr
 * @ss_rssi_thr:          ADC RSSI must be greater than or equal to this
 *                        threshold (signed Db) to ensure spectral scan
 *                        reporting with normal phy error codes (please see
 *                        ss_rssi_rpt_mode above).Though stored as an unsigned
 *                        value, this should be treated as a signed 8-bit int
 * @ss_pwr_format:        Format of frequency bin magnitude for spectral scan
 *                        triggered FFTs 0: linear magnitude
 *                        1: log magnitude (20*log10(lin_mag), 1/2 dB step size)
 * @ss_rpt_mode:          Format of per-FFT reports to software for spectral
 *                        scan triggered FFTs
 *                        0: No FFT report (only pulse end summary)
 *                        1: 2-dword summary of metrics for each completed FFT
 *                        2: 2-dword summary + 1x-oversampled bins(in-band) per
 *                           FFT
 *                        3: 2-dword summary + 2x-oversampled bins (all) per FFT
 * @ss_bin_scale:         Number of LSBs to shift out to scale the FFT bins
 *                        for spectral scan triggered FFTs
 * @ss_dbm_adj:           Set (with ss_pwr_format=1), to report bin
 *                        magnitudes
 *                        converted to dBm power using the noisefloor
 *                        calibration results
 * @ss_chn_mask:          Per chain enable mask to select input ADC for search
 *                        FFT
 * @ss_nf_cal:            nf calibrated values for ctl+ext
 * @ss_nf_pwr:            nf pwr values for ctl+ext
 * @ss_nf_temp_data:      temperature data taken during nf scan
 */
struct spectral_config {
	uint16_t ss_fft_period;
	uint16_t ss_period;
	uint16_t ss_count;
	uint16_t ss_short_report;
	uint8_t radar_bin_thresh_sel;
	uint16_t ss_spectral_pri;
	uint16_t ss_fft_size;
	uint16_t ss_gc_ena;
	uint16_t ss_restart_ena;
	uint16_t ss_noise_floor_ref;
	uint16_t ss_init_delay;
	uint16_t ss_nb_tone_thr;
	uint16_t ss_str_bin_thr;
	uint16_t ss_wb_rpt_mode;
	uint16_t ss_rssi_rpt_mode;
	uint16_t ss_rssi_thr;
	uint16_t ss_pwr_format;
	uint16_t ss_rpt_mode;
	uint16_t ss_bin_scale;
	uint16_t ss_dbm_adj;
	uint16_t ss_chn_mask;
	int8_t ss_nf_cal[AH_MAX_CHAINS * 2];
	int8_t ss_nf_pwr[AH_MAX_CHAINS * 2];
	int32_t ss_nf_temp_data;
};

/**
 * struct spectral_scan_state - State of spectral scan
 * @is_active:  Is spectral scan active
 * @is_enabled: Is spectral scan enabled
 */
struct spectral_scan_state {
	uint8_t is_active;
	uint8_t is_enabled;
};

/* Forward declarations */
struct wlan_objmgr_pdev;

/**
 * struct spectral_nl_cb - Spectral Netlink callbacks
 * @get_nbuff:      Get the socket buffer to send the data to the application
 * @send_nl_bcast:  Send data to the application using netlink broadcast
 * @send_nl_unicast:  Send data to the application using netlink unicast
 */
struct spectral_nl_cb {
	void *(*get_nbuff)(struct wlan_objmgr_pdev *pdev);
	int (*send_nl_bcast)(struct wlan_objmgr_pdev *pdev);
	int (*send_nl_unicast)(struct wlan_objmgr_pdev *pdev);
	void (*free_nbuff)(struct wlan_objmgr_pdev *pdev);
};

#ifndef __KERNEL__

static inline int16_t
spectral_pwfactor_max(int16_t pwfactor1,
		      int16_t pwfactor2)
{
	return ((pwfactor1 > pwfactor2) ? pwfactor1 : pwfactor2);
}

/**
 * get_spectral_scale_rssi_corr() - Compute RSSI correction factor for scaling
 * @agc_total_gain_db: AGC total gain in dB steps
 * @gen3_defmaxgain: Default max gain value of the gen III chipset
 * @gen2_maxgain: Max gain value used by the reference gen II chipset
 * @lowlevel_offset: Low level offset for scaling
 * @inband_pwr: In band power in dB steps
 * @rssi_thr: RSSI threshold for scaling
 *
 * Helper function to compute RSSI correction factor for Gen III linear format
 * Spectral scaling. It is the responsibility of the caller to ensure that
 * correct values are passed.
 *
 * Return: RSSI correction factor
 */
static inline int16_t
get_spectral_scale_rssi_corr(u_int8_t agc_total_gain_db,
			     u_int8_t gen3_defmaxgain, u_int8_t gen2_maxgain,
			     int16_t lowlevel_offset, int16_t inband_pwr,
			     int16_t rssi_thr)
{
	return ((agc_total_gain_db < gen3_defmaxgain) ?
		(gen2_maxgain - gen3_defmaxgain + lowlevel_offset) :
		spectral_pwfactor_max((inband_pwr - rssi_thr), 0));
}

/**
 * spectral_scale_linear_to_gen2() - Scale linear bin value to gen II equivalent
 * @gen3_binmag: Captured FFT bin value from the Spectral Search FFT report
 * generated by the Gen III chipset
 * @gen2_maxgain: Max gain value used by the reference gen II chipset
 * @gen3_defmaxgain: Default max gain value of the gen III chipset
 * @lowlevel_offset: Low level offset for scaling
 * @inband_pwr: In band power in dB steps
 * @rssi_thr: RSSI threshold for scaling
 * @agc_total_gain_db: AGC total gain in dB steps
 * @highlevel_offset: High level offset for scaling
 * @gen2_bin_scale: Bin scale value used on reference gen II chipset
 * @gen3_bin_scale: Bin scale value used on gen III chipset
 *
 * Helper function to scale a given gen III linear format bin value into an
 * approximately equivalent gen II value. The scaled value can possibly be
 * higher than 8 bits.  If the caller is incapable of handling values larger
 * than 8 bits, the caller can saturate the value at 255. This function does not
 * carry out this saturation for the sake of flexibility so that callers
 * interested in the larger values can avail of this. Also note it is the
 * responsibility of the caller to ensure that correct values are passed.
 *
 * Return: Scaled bin value
 */
static inline u_int32_t
spectral_scale_linear_to_gen2(u_int8_t gen3_binmag,
			      u_int8_t gen2_maxgain, u_int8_t gen3_defmaxgain,
			      int16_t lowlevel_offset, int16_t inband_pwr,
			      int16_t rssi_thr, u_int8_t agc_total_gain_db,
			      int16_t highlevel_offset, u_int8_t gen2_bin_scale,
			      u_int8_t gen3_bin_scale)
{
	return (gen3_binmag *
		sqrt(pow(10, (((double)spectral_pwfactor_max(gen2_maxgain -
			gen3_defmaxgain + lowlevel_offset -
			get_spectral_scale_rssi_corr(agc_total_gain_db,
						     gen3_defmaxgain,
						     gen2_maxgain,
						     lowlevel_offset,
						     inband_pwr,
						     rssi_thr),
			(agc_total_gain_db < gen3_defmaxgain) *
				highlevel_offset)) / 10))) *
		 pow(2, (gen3_bin_scale - gen2_bin_scale)));
}

#endif /*  __KERNEL__  */

#endif				/* _WLAN_SPECTRAL_PUBLIC_STRUCTS_H_ */
