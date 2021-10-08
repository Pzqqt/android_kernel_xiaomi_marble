/*
 * Copyright (c) 2011, 2017-2021 The Linux Foundation. All rights reserved.
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

#ifndef _SPECTRAL_IOCTL_H_
#define _SPECTRAL_IOCTL_H_
#include <wlan_dfs_ioctl.h>

#ifndef AH_MAX_CHAINS
#define AH_MAX_CHAINS 3
#endif

/* Compile time Assert */
#define SPECTRAL_COMPILE_TIME_ASSERT(assertion_name, predicate) \
	typedef char assertion_name[(predicate) ? 1 : -1]

/*
 * ioctl defines
 */

#define SPECTRAL_SET_CONFIG              (DFS_LAST_IOCTL + 1)
#define SPECTRAL_GET_CONFIG              (DFS_LAST_IOCTL + 2)
#define SPECTRAL_SHOW_INTERFERENCE       (DFS_LAST_IOCTL + 3)
#define SPECTRAL_ENABLE_SCAN             (DFS_LAST_IOCTL + 4)
#define SPECTRAL_DISABLE_SCAN            (DFS_LAST_IOCTL + 5)
#define SPECTRAL_ACTIVATE_SCAN           (DFS_LAST_IOCTL + 6)
#define SPECTRAL_STOP_SCAN               (DFS_LAST_IOCTL + 7)
#define SPECTRAL_SET_DEBUG_LEVEL         (DFS_LAST_IOCTL + 8)
#define SPECTRAL_IS_ACTIVE               (DFS_LAST_IOCTL + 9)
#define SPECTRAL_IS_ENABLED              (DFS_LAST_IOCTL + 10)
#define SPECTRAL_CLASSIFY_SCAN           (DFS_LAST_IOCTL + 11)
#define SPECTRAL_GET_CLASSIFIER_CONFIG   (DFS_LAST_IOCTL + 12)
#define SPECTRAL_EACS                    (DFS_LAST_IOCTL + 13)
#define SPECTRAL_ACTIVATE_FULL_SCAN      (DFS_LAST_IOCTL + 14)
#define SPECTRAL_STOP_FULL_SCAN          (DFS_LAST_IOCTL + 15)
#define SPECTRAL_GET_CAPABILITY_INFO     (DFS_LAST_IOCTL + 16)
#define SPECTRAL_GET_DIAG_STATS          (DFS_LAST_IOCTL + 17)
#define SPECTRAL_GET_CHAN_WIDTH          (DFS_LAST_IOCTL + 18)
#define SPECTRAL_GET_CHANINFO            (DFS_LAST_IOCTL + 19)
#define SPECTRAL_CLEAR_CHANINFO          (DFS_LAST_IOCTL + 20)
#define SPECTRAL_SET_ICM_ACTIVE          (DFS_LAST_IOCTL + 21)
#define SPECTRAL_GET_NOMINAL_NOISEFLOOR  (DFS_LAST_IOCTL + 22)
#define SPECTRAL_GET_DEBUG_LEVEL         (DFS_LAST_IOCTL + 23)
#define SPECTRAL_SET_DMA_DEBUG           (DFS_LAST_IOCTL + 24)

/*
 * Increase spectral sub version if struct spectral_samp_msg updated.
 */
#define SPECTRAL_VERSION     (3)
#define SPECTRAL_SUB_VERSION (1)

/*
 * ioctl parameter types
 */
enum spectral_params {
	SPECTRAL_PARAM_FFT_PERIOD,
	SPECTRAL_PARAM_SCAN_PERIOD,
	SPECTRAL_PARAM_SCAN_COUNT,
	SPECTRAL_PARAM_SHORT_REPORT,
	SPECTRAL_PARAM_SPECT_PRI,
	SPECTRAL_PARAM_FFT_SIZE,
	SPECTRAL_PARAM_GC_ENA,
	SPECTRAL_PARAM_RESTART_ENA,
	SPECTRAL_PARAM_NOISE_FLOOR_REF,
	SPECTRAL_PARAM_INIT_DELAY,
	SPECTRAL_PARAM_NB_TONE_THR,
	SPECTRAL_PARAM_STR_BIN_THR,
	SPECTRAL_PARAM_WB_RPT_MODE,
	SPECTRAL_PARAM_RSSI_RPT_MODE,
	SPECTRAL_PARAM_RSSI_THR,
	SPECTRAL_PARAM_PWR_FORMAT,
	SPECTRAL_PARAM_RPT_MODE,
	SPECTRAL_PARAM_BIN_SCALE,
	SPECTRAL_PARAM_DBM_ADJ,
	SPECTRAL_PARAM_CHN_MASK,
	SPECTRAL_PARAM_ACTIVE,
	SPECTRAL_PARAM_STOP,
	SPECTRAL_PARAM_ENABLE,
	SPECTRAL_PARAM_FREQUENCY,
	SPECTRAL_PARAM_CHAN_FREQUENCY,
	SPECTRAL_PARAM_CHAN_WIDTH,
	SPECTRAL_PARAM_MAX,
};

/**
 * enum spectral_report_mode: Spectral report mode
 * @SPECTRAL_REPORT_MODE_0: No FFT report (only spectral scan summary report)
 * @SPECTRAL_REPORT_MODE_1: FFT report header + spectral scan summary report
 * @SPECTRAL_REPORT_MODE_2: FFt report header + in-band bins per
 *                          FFT (half of the number of FFT bins), where the
 *                          FFT input is sampled at two times the channel
 *                          bandwidth + spectral scan summary report
 * @SPECTRAL_REPORT_MODE_3: FFT report header + all bins per FFT, where the FFT
 *                          input is sampled at two times the channel bandwidth
 *                          + spectral scan summary report
 * @SPECTRAL_REPORT_MODE_MAX: Max number of report modes
 */
enum spectral_report_mode {
	SPECTRAL_REPORT_MODE_0,
	SPECTRAL_REPORT_MODE_1,
	SPECTRAL_REPORT_MODE_2,
	SPECTRAL_REPORT_MODE_3,
	SPECTRAL_REPORT_MODE_MAX,
};

/**
 * enum spectral_scan_priority: Spectral scan priority
 * @SPECTRAL_SCAN_PRIORITY_LOW: Low priority Spectral scan
 * @SPECTRAL_SCAN_PRIORITY_HIGH: High priority Spectral scan
 */
enum spectral_scan_priority {
	SPECTRAL_SCAN_PRIORITY_LOW = 0,
	SPECTRAL_SCAN_PRIORITY_HIGH = 1,
};

/**
 * enum spectral_fft_size : FFT size values
 * @SPECTRAL_FFT_SIZE_INVALID: Invalid FFT size
 * @SPECTRAL_FFT_SIZE_1: FFT size 1
 * @SPECTRAL_FFT_SIZE_2: FFT size 2
 * @SPECTRAL_FFT_SIZE_3: FFT size 3
 * @SPECTRAL_FFT_SIZE_4: FFT size 4
 * @SPECTRAL_FFT_SIZE_5: FFT size 5
 * @SPECTRAL_FFT_SIZE_6: FFT size 6
 * @SPECTRAL_FFT_SIZE_7: FFT size 7
 * @SPECTRAL_FFT_SIZE_8: FFT size 8
 * @SPECTRAL_FFT_SIZE_9: FFT size 9
 * @SPECTRAL_FFT_SIZE_10: FFT size 10
 * @SPECTRAL_FFT_SIZE_MAX: Max number of FFT size
 */
enum spectral_fft_size {
	SPECTRAL_FFT_SIZE_INVALID,
	SPECTRAL_FFT_SIZE_1,
	SPECTRAL_FFT_SIZE_2,
	SPECTRAL_FFT_SIZE_3,
	SPECTRAL_FFT_SIZE_4,
	SPECTRAL_FFT_SIZE_5,
	SPECTRAL_FFT_SIZE_6,
	SPECTRAL_FFT_SIZE_7,
	SPECTRAL_FFT_SIZE_8,
	SPECTRAL_FFT_SIZE_9,
	SPECTRAL_FFT_SIZE_10,
	SPECTRAL_FFT_SIZE_MAX,
};

/**
 * enum spectral_scan_mode - Spectral scan mode
 * @SPECTRAL_SCAN_MODE_NORMAL: Normal mode
 * @SPECTRAL_SCAN_MODE_AGILE: Agile mode
 * @SPECTRAL_SCAN_MODE_MAX: Max number of Spectral modes
 * @SPECTRAL_SCAN_MODE_INVALID: Invalid Spectral mode
 */
enum spectral_scan_mode {
	SPECTRAL_SCAN_MODE_NORMAL,
	SPECTRAL_SCAN_MODE_AGILE,
	SPECTRAL_SCAN_MODE_MAX,
	SPECTRAL_SCAN_MODE_INVALID = 0xff,
};

/**
 * enum spectral_chan_width - Spectral-specific channel width enum
 * @SPECTRAL_CH_WIDTH_20MHZ: 20 mhz width
 * @SPECTRAL_CH_WIDTH_40MHZ: 40 mhz width
 * @SPECTRAL_CH_WIDTH_80MHZ: 80 mhz width
 * @SPECTRAL_CH_WIDTH_160MHZ: 160 mhz width
 * @SPECTRAL_CH_WIDTH_80P80MHZ: 80+80 mhz width
 * @SPECTRAL_CH_WIDTH_5MHZ: 5 mhz width
 * @SPECTRAL_CH_WIDTH_10MHZ: 10 mhz width
 * @SPECTRAL_CH_WIDTH_320MHZ: 320 mhz width
 * @SPECTRAL_CH_WIDTH_MAX: Max possible width
 * @SPECTRAL_CH_WIDTH_INVALID: invalid width
 */
enum spectral_chan_width {
	SPECTRAL_CH_WIDTH_20MHZ,
	SPECTRAL_CH_WIDTH_40MHZ,
	SPECTRAL_CH_WIDTH_80MHZ,
	SPECTRAL_CH_WIDTH_160MHZ,
	SPECTRAL_CH_WIDTH_80P80MHZ,
	SPECTRAL_CH_WIDTH_320MHZ,
	SPECTRAL_CH_WIDTH_5MHZ,
	SPECTRAL_CH_WIDTH_10MHZ,
	SPECTRAL_CH_WIDTH_MAX,
	SPECTRAL_CH_WIDTH_INVALID,
};

struct spectral_ioctl_params {
	int16_t   spectral_fft_period;
	int16_t   pectral_period;
	int16_t   spectral_count;
	uint16_t spectral_short_report;
	uint16_t spectral_pri;
};

/**
 * spectral_cap_hw_gen: Definitions for the Spectral hardware generation.
 * This corresponds to definitions in qca_wlan_vendor_spectral_scan_cap_hw_gen.
 * @SPECTRAL_CAP_HW_GEN_1: Generation 1
 * @SPECTRAL_CAP_HW_GEN_2: Generation 2
 * @SPECTRAL_CAP_HW_GEN_3: Generation 3
 */
enum spectral_cap_hw_gen {
	SPECTRAL_CAP_HW_GEN_1 = 0,
	SPECTRAL_CAP_HW_GEN_2 = 1,
	SPECTRAL_CAP_HW_GEN_3 = 2,
};

/**
 * struct spectral_config_frequency - Spectral scan frequency
 * @cfreq1: Center frequency (in MHz) of the span of interest(primary 80 MHz
 *          span for 80 + 80 agile scan request) or center frequency (in MHz)
 *          of any WLAN channel in the span of interest.
 * @cfreq2: Applicable only for Agile Spectral scan request in 80+80 MHz mode.
 *          For 80+80 mode it represents  the center frequency (in MHz) of the
 *          secondary 80 MHz span of interest or center frequency (in MHz) of
 *          any WLAN channel in the secondary 80 MHz span of interest.
 */
struct spectral_config_frequency {
	uint32_t cfreq1;
	uint32_t cfreq2;
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
 * @ss_frequency:         This specifies the frequency span over which Spectral
 *                        scan would be carried out. Its value depends on the
 *                        Spectral scan mode.
 *                        Normal mode:-
 *                          Not applicable. Spectral scan would happen in the
 *                          operating span.
 *                        Agile mode:-
 *                          cfreq1 represents the center frequency (in MHz) of
 *                          the span of interest(primary 80 MHz span for 80 + 80
 *                          agile scan request) or center frequency (in MHz) of
 *                          any WLAN channel in the span of interest. cfreq2 is
 *                          applicable only for Agile Spectral scan request in
 *                          80+80 MHz mode. For 80+80 mode it represents  the
 *                          center frequency (in MHz) of the secondary 80 MHz
*                           span of interest or center frequency (in MHz) of
 *                          any WLAN channel in the secondary 80 MHz span of
 *                          interest.
 * @ss_bandwidth: Spectral scan bandwidth
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
	struct spectral_config_frequency ss_frequency;
	uint16_t ss_bandwidth;
};

/**
 * struct spectral_caps - Spectral capabilities structure
 * @phydiag_cap:         Phydiag capability
 * @radar_cap:           Radar detection capability
 * @spectral_cap:        Spectral capability
 * @advncd_spectral_cap: Advanced spectral capability
 * @hw_gen: Spectral hw generation as defined in spectral_cap_hw_gen
 * @is_scaling_params_populated: indicates whether scaling params is populated
 * @formula_id: formula_id
 * @low_level_offset: low_level_offset
 * @high_level_offset: high_level_offset
 * @rssi_thr: rssi_thr
 * @default_agc_max_gain: default_agc_max_gain
 * @agile_spectral_cap: agile Spectral capability for 20/40/80
 * @agile_spectral_cap_160: agile Spectral capability for 160 MHz
 * @agile_spectral_cap_80p80: agile Spectral capability for 80p80
 * @agile_spectral_cap_320: agile Spectral capability for 320 MHz
 * @num_detectors_20mhz: number of Spectral detectors in 20 MHz
 * @num_detectors_40mhz: number of Spectral detectors in 40 MHz
 * @num_detectors_80mhz: number of Spectral detectors in 80 MHz
 * @num_detectors_160mhz: number of Spectral detectors in 160 MHz
 * @num_detectors_80p80mhz: number of Spectral detectors in 80p80 MHz
 * @num_detectors_320mhz: number of Spectral detectors in 320 MHz
 */
struct spectral_caps {
	uint8_t phydiag_cap;
	uint8_t radar_cap;
	uint8_t spectral_cap;
	uint8_t advncd_spectral_cap;
	uint32_t hw_gen;
	bool is_scaling_params_populated;
	uint16_t formula_id;
	int16_t low_level_offset;
	int16_t high_level_offset;
	int16_t rssi_thr;
	uint8_t default_agc_max_gain;
	bool agile_spectral_cap;
	bool agile_spectral_cap_160;
	bool agile_spectral_cap_80p80;
	bool agile_spectral_cap_320;
	uint32_t num_detectors_20mhz;
	uint32_t num_detectors_40mhz;
	uint32_t num_detectors_80mhz;
	uint32_t num_detectors_160mhz;
	uint32_t num_detectors_80p80mhz;
	uint32_t num_detectors_320mhz;
};

#define SPECTRAL_IOCTL_PARAM_NOVAL (65535)

#define MAX_SPECTRAL_CHAINS           (3)
#define MAX_NUM_BINS                  (2048)
#define MAX_NUM_BINS_PRI80            (1024)
#define MAX_NUM_BINS_SEC80            (520)
#define MAX_NUM_BINS_5MHZ             (32)
/* 5 categories x (lower + upper) bands */
#define MAX_INTERF                   10
#define SPECTRAL_MAC_ADDR_SIZE        (6)
#define MAX_NUM_FREQ_SPANS            (3)
#define MAX_NUM_DETECTORS             (2)
#define MAX_SPECTRAL_PAYLOAD          (2004)

/**
 * enum dcs_int_type - Interference type indicated by DCS
 * @SPECTRAL_DCS_INT_NONE:  No interference
 * @SPECTRAL_DCS_INT_CW:  CW interference
 * @SPECTRAL_DCS_INT_WIFI:  WLAN interference
 */
enum dcs_int_type {
	SPECTRAL_DCS_INT_NONE,
	SPECTRAL_DCS_INT_CW,
	SPECTRAL_DCS_INT_WIFI
};

/**
 * struct interf_rsp - Interference record
 * @interf_type:         eINTERF_TYPE giving type of interference
 * @interf_min_freq:     Minimum frequency in MHz at which interference has been
 * found
 * @interf_max_freq:     Maximum frequency in MHz at which interference has been
 * found
 * @advncd_spectral_cap: Advanced spectral capability
 */
struct interf_rsp {
	uint8_t interf_type;
	uint16_t interf_min_freq;
	uint16_t interf_max_freq;
} __packed;

/**
 * struct interf_src_rsp - List of interference sources
 * @count: Number of interference records
 * @interf: Array of interference records
 */
struct interf_src_rsp {
	uint16_t count;
	struct interf_rsp interf[MAX_INTERF];
} __packed;

/**
 * struct spectral_classifier_params - spectral classifier parameters
 * @spectral_20_40_mode:  Is AP in 20/40 mode?
 * @spectral_dc_index:    DC index
 * @spectral_dc_in_mhz:   DC in MHz
 * @upper_chan_in_mhz:    Upper channel in MHz
 * @lower_chan_in_mhz:    Lower channel in MHz
 */
struct spectral_classifier_params {
	int spectral_20_40_mode;
	int spectral_dc_index;
	int spectral_dc_in_mhz;
	int upper_chan_in_mhz;
	int lower_chan_in_mhz;
} __packed;

#ifdef OPTIMIZED_SAMP_MESSAGE
/**
 * struct samp_edge_extra_bin_info - Spectral edge extra bins Information
 * For 11ac chipsets prior to AR900B version 2.0, a max of 512 bins are
 * delivered.  However, there can be additional bins reported for
 * AR900B version 2.0 and QCA9984 as described next:
 * AR900B version 2.0: An additional tone is processed on the right
 * hand side in order to facilitate detection of radar pulses out to
 * the extreme band-edge of the channel frequency.
 * Since the HW design processes four tones at a time,
 * this requires one additional Dword to be added to the
 * search FFT report.
 * QCA9984: When spectral_scan_rpt_mode=2, i.e 2-dword summary +
 * 1x-oversampled bins (in-band) per FFT,
 * then 8 more bins (4 more on left side and 4 more on right side)
 * are added.
 *
 * @num_bins: Number of edge extra bins
 * @start_bin_idx: Indicates the start index of extra bins
 */
struct samp_edge_extra_bin_info {
	uint16_t num_bins;
	uint16_t start_bin_idx;
} __packed;

/* Compile time assert to check struct size is divisible by 4 Bytes */
SPECTRAL_COMPILE_TIME_ASSERT(struct_size_4byte_assertion,
			     (sizeof(struct samp_edge_extra_bin_info) % 4)
			     == 0);

/**
 * struct samp_detector_info - SAMP per-detector information
 * A detector here refers to the HW carrying out the Spectral scan, to
 * detect the presence of interferences.
 * @start_frequency: Indicates start frequency per-detector (in MHz)
 * @end_frequency: Indicates last frequency per-detector (in MHz)
 * @timestamp: Indicates Spectral HW timestamp (usec)
 * @last_tstamp: Indicates the last time stamp
 * @last_raw_timestamp: Previous FFT report's raw timestamp. In case of
 * 160Mhz it will be primary 80 segment's timestamp as both primary & secondary
 * segment's timestamp are expected to be almost equal.
 * @timestamp_war_offset: Offset calculated based on reset_delay and
 * last_raw_timestamp. It will be added to raw_timestamp to get timestamp.
 * @raw_timestamp: Actual FFT timestamp reported by HW
 * @reset_delay: Time gap between the last spectral report before reset and the
 * end of reset. It is provided by FW via direct DMA framework.
 * @left_edge_bins: Number of extra bins on left band edge
 * @right_edge_bins: Number of extra bins on right band edge
 * @start_bin_idx: Indicates the first bin index per-detector
 * @end_bin_idx: Indicates the last bin index per-detector
 * @max_index: Indicates the index of max magnitude
 * @max_magnitude: Indicates the maximum magnitude
 * @noise_floor: Indicates the current noise floor
 * @rssi: Indicates RSSI
 * @pri80ind: Indication from hardware that the sample was received on the
 * primary 80 MHz segment. If this is set for smode = SPECTRAL_SCAN_MODE_AGILE,
 * it indicates that Spectral scan was carried out on pri80 instead of the
 * Agile frequency due to a channel switch - Software may choose to ignore
 * the sample in this case.
 * @is_sec80: Indicates whether the frequency span corresponds to pri80 or
 * sec80 (only applicable for 160/80p80 operating_bw for
 * smode SPECTRAL_SCAN_MODE_NORMAL)
 */
struct samp_detector_info {
	uint32_t start_frequency;
	uint32_t end_frequency;
	uint32_t timestamp;
	uint32_t last_tstamp;
	uint32_t last_raw_timestamp;
	uint32_t timestamp_war_offset;
	uint32_t raw_timestamp;
	uint32_t reset_delay;
	struct samp_edge_extra_bin_info left_edge_bins;
	struct samp_edge_extra_bin_info right_edge_bins;
	uint16_t start_bin_idx;
	uint16_t end_bin_idx;
	uint16_t max_index;
	uint16_t max_magnitude;
	int16_t noise_floor;
	int8_t rssi;
	uint8_t agc_total_gain;
	uint8_t gainchange;
	uint8_t pri80ind;
	uint8_t is_sec80;
	/* Padding bits to make struct size multiple of 4 bytes */
	uint8_t padding_detector_info[1];
} __packed;

/* Compile time assert to check struct size is divisible by 4 Bytes */
SPECTRAL_COMPILE_TIME_ASSERT(struct_size_4byte_assertion,
			     (sizeof(struct samp_detector_info) % 4) == 0);

/**
 * struct samp_freq_span_info - SAMP per-frequency span information
 * A frequency span here refers to a contiguous span of frequencies in which
 * Spectral scan and interference detection is carried out.
 * @detector_info: Per-detector Spectral information
 * @num_detectors: Number of detectors per span
 */
struct samp_freq_span_info {
	struct samp_detector_info detector_info[MAX_NUM_DETECTORS];
	uint8_t num_detectors;
	/* Padding bits to make struct size multiple of 4 bytes */
	uint8_t padding_span_info[3];
} __packed;

/* Compile time assert to check struct size is divisible by 4 Bytes */
SPECTRAL_COMPILE_TIME_ASSERT(struct_size_4byte_assertion,
			     (sizeof(struct samp_freq_span_info) % 4) == 0);

/**
 * struct spectral_samp_msg - Spectral SAMP message
 * @signature: Validates the SAMP message
 * @target_reset_count: Indicates the number of times target went through
 * reset routine after spectral was enabled.
 * @pri20_freq: Primary 20MHz operating frequency in MHz
 * @cfreq1: Segment 1 centre frequency in MHz
 * @cfreq2: For 80p80, indicates segment 2 centre frequency in MHz. For 160MHz,
 * indicates the center frequency of 160MHz span.
 * @sscan_cfreq1: Normal/Agile scan Center frequency for Segment 1
 * based on Spectral Scan mode.
 * @sscan_cfreq2: Normal/Agile scan Center frequency for Segment 2 in case of
 * 80p80, and for 160MHz center frequency of the 160MHz span based on Spectral
 * Scan mode.
 * @bin_pwr_count: Indicates the number of FFT bins
 * @freq_span_info: Spectral per-contiguous frequency span information
 * @spectral_upper_rssi: Indicates RSSI of upper band
 * @spectral_lower_rssi: Indicates RSSI of lower band
 * @spectral_chain_ctl_rssi: RSSI for control channel, for all antennas
 * @spectral_chain_ext_rssi: RSSI for extension channel, for all antennas
 * @macaddr: Indicates the device interface
 * @spectral_mode: Spectral scan mode
 * @operating_bw: Device's operating bandwidth. Values = enum phy_ch_width
 * @sscan_bw: Normal/Agile Scan BW based on Spectral scan mode.
 * Values = enum phy_ch_width
 * @fft_width: Indicates the number of bits representing an FFT bin
 * @dcs_enabled: Whether DCS is enabled
 * @int_type: Interference type indicated by DCS. Values = enum dcs_int_type
 * @num_freq_spans: Number of contiguous frequency spans in operating bandwidth
 * @bin_pwr: Contains FFT magnitudes
 */
struct spectral_samp_msg {
	uint32_t signature;
	uint32_t target_reset_count;
	uint32_t pri20_freq;
	uint32_t cfreq1;
	uint32_t cfreq2;
	uint32_t sscan_cfreq1;
	uint32_t sscan_cfreq2;
	uint32_t bin_pwr_count;
	struct samp_freq_span_info freq_span_info[MAX_NUM_FREQ_SPANS];
	int8_t spectral_lower_rssi;
	int8_t spectral_upper_rssi;
	int8_t spectral_chain_ctl_rssi[MAX_SPECTRAL_CHAINS];
	int8_t spectral_chain_ext_rssi[MAX_SPECTRAL_CHAINS];
	uint8_t macaddr[SPECTRAL_MAC_ADDR_SIZE];
	uint8_t spectral_mode;
	uint8_t operating_bw;
	uint8_t sscan_bw;
	uint8_t fft_width;
	uint8_t dcs_enabled;
	uint8_t int_type;
	uint8_t num_freq_spans;
	uint8_t bin_pwr[0];  /*This should be the last item in the structure*/
} __packed;

#else
/**
 * struct spectral_samp_data - Spectral Analysis Messaging Protocol Data format
 * @spectral_data_len:        Indicates the bin size
 * @spectral_data_len_sec80:  Indicates the bin size for secondary 80 segment
 * @spectral_rssi:            Indicates RSSI
 * @spectral_rssi_sec80:      Indicates RSSI for secondary 80 segment
 * @spectral_combined_rssi:   Indicates combined RSSI from all antennas
 * @spectral_upper_rssi:      Indicates RSSI of upper band
 * @spectral_lower_rssi:      Indicates RSSI of lower band
 * @spectral_chain_ctl_rssi:  RSSI for control channel, for all antennas
 * @spectral_chain_ext_rssi:  RSSI for extension channel, for all antennas
 * @spectral_max_scale:       Indicates scale factor
 * @spectral_bwinfo:          Indicates bandwidth info
 * @spectral_tstamp:          Indicates timestamp
 * @spectral_max_index:       Indicates the index of max magnitude
 * @spectral_max_index_sec80: Indicates the index of max magnitude for secondary
 *                            80 segment
 * @spectral_max_mag:         Indicates the maximum magnitude
 * @spectral_max_mag_sec80:   Indicates the maximum magnitude for secondary 80
 *                            segment
 * @spectral_max_exp:         Indicates the max exp
 * @spectral_last_tstamp:     Indicates the last time stamp
 * @spectral_upper_max_index: Indicates the index of max mag in upper band
 * @spectral_lower_max_index: Indicates the index of max mag in lower band
 * @spectral_nb_upper:        Not Used
 * @spectral_nb_lower:        Not Used
 * @classifier_params:        Indicates classifier parameters
 * @bin_pwr_count:            Indicates the number of FFT bins
 * @lb_edge_extrabins:        Number of extra bins on left band edge
 * @rb_edge_extrabins:        Number of extra bins on right band edge
 * @bin_pwr_count_sec80:      Indicates the number of FFT bins in secondary 80
 *                            segment
 * @bin_pwr:                  Contains FFT magnitudes
 * @bin_pwr_sec80:            Contains FFT magnitudes for the secondary 80
 *                            segment
 * @interf_list:              List of interfernce sources
 * @noise_floor:              Indicates the current noise floor
 * @noise_floor_sec80:        Indicates the current noise floor for secondary 80
 *                            segment
 * @ch_width:                 Channel width 20/40/80/160 MHz
 * @spectral_mode:            Spectral scan mode
 * @spectral_pri80ind:        Indication from hardware that the sample was
 *                            received on the primary 80 MHz segment. If this
 *                            is set when smode = SPECTRAL_SCAN_MODE_AGILE, it
 *                            indicates that Spectral was carried out on pri80
 *                            instead of the Agile frequency due to a
 *                            channel switch - Software may choose
 *                            to ignore the sample in this case.
 * @spectral_pri80ind_sec80:  Indication from hardware that the sample was
 *                            received on the primary 80 MHz segment instead of
 *                            the secondary 80 MHz segment due to a channel
 *                            switch - Software may choose to ignore the sample
 *                            if this is set. Applicable only if smode =
 *                            SPECTRAL_SCAN_MODE_NORMAL and for 160/80+80 MHz
 *                            Spectral operation.
 * @last_raw_timestamp:       Previous FFT report's raw timestamp. In case of
 *                            160Mhz it will be primary 80 segment's timestamp
 *                            as both primary & secondary segment's timestamp
 *                            are expected to be almost equal.
 * @timestamp_war_offset:     Offset calculated based on reset_delay and
 *                            last_raw_timestamp. It will be added to
 *                            raw_timestamp to get spectral_tstamp.
 * @raw_timestamp:            Actual FFT timestamp reported by HW on primary
 *                            segment.
 * @raw_timestamp_sec80:      Actual FFT timestamp reported by HW on sec80 MHz
 *                            segment.
 * @reset_delay:              Time gap between the last spectral report before
 *                            reset and the end of reset. It is provided by FW
 *                            via direct DMA framework.
 * @target_reset_count:       Indicates the number of times target went through
 *                            reset routine after spectral was enabled.
 * @bin_pwr_count_5mhz:       Indicates the number of FFT bins in the extra
 *                            5 MHz for 165 MHz/ Restricted 80p80 mode
 * @bin_pwr_5mhz:             Contains FFT magnitudes corresponding to the extra
 *                            5 MHz in 165 MHz/ Restricted 80p80 mode
 */
struct spectral_samp_data {
	int16_t spectral_data_len;
	int16_t spectral_data_len_sec80;
	int16_t spectral_rssi;
	int16_t spectral_rssi_sec80;
	int8_t spectral_combined_rssi;
	int8_t spectral_upper_rssi;
	int8_t spectral_lower_rssi;
	int8_t spectral_chain_ctl_rssi[MAX_SPECTRAL_CHAINS];
	int8_t spectral_chain_ext_rssi[MAX_SPECTRAL_CHAINS];
	uint8_t spectral_max_scale;
	int16_t spectral_bwinfo;
	int32_t spectral_tstamp;
	int16_t spectral_max_index;
	int16_t spectral_max_index_sec80;
	int16_t spectral_max_mag;
	int16_t spectral_max_mag_sec80;
	uint8_t spectral_max_exp;
	int32_t spectral_last_tstamp;
	int16_t spectral_upper_max_index;
	int16_t spectral_lower_max_index;
	uint8_t spectral_nb_upper;
	uint8_t spectral_nb_lower;
	struct spectral_classifier_params classifier_params;
	uint16_t bin_pwr_count;
	/*
	 * For 11ac chipsets prior to AR900B version 2.0, a max of 512 bins are
	 * delivered.  However, there can be additional bins reported for
	 * AR900B version 2.0 and QCA9984 as described next:
	 *
	 * AR900B version 2.0: An additional tone is processed on the right
	 * hand side in order to facilitate detection of radar pulses out to
	 * the extreme band-edge of the channel frequency.
	 * Since the HW design processes four tones at a time,
	 * this requires one additional Dword to be added to the
	 * search FFT report.
	 *
	 * QCA9984: When spectral_scan_rpt_mode=2, i.e 2-dword summary +
	 * 1x-oversampled bins (in-band) per FFT,
	 * then 8 more bins (4 more on left side and 4 more on right side)
	 * are added.
	 */
	uint8_t lb_edge_extrabins;
	uint8_t rb_edge_extrabins;
	uint16_t bin_pwr_count_sec80;
	uint8_t bin_pwr[MAX_NUM_BINS_PRI80];
	uint8_t bin_pwr_sec80[MAX_NUM_BINS_SEC80];
	struct interf_src_rsp interf_list;
	int16_t noise_floor;
	int16_t noise_floor_sec80;
	uint32_t ch_width;
	uint8_t spectral_agc_total_gain;
	uint8_t spectral_agc_total_gain_sec80;
	uint8_t spectral_gainchange;
	uint8_t spectral_gainchange_sec80;
	enum spectral_scan_mode spectral_mode;
	uint8_t spectral_pri80ind;
	uint8_t spectral_pri80ind_sec80;
	uint32_t last_raw_timestamp;
	uint32_t timestamp_war_offset;
	uint32_t raw_timestamp;
	uint32_t raw_timestamp_sec80;
	uint32_t reset_delay;
	uint32_t target_reset_count;
	uint32_t agile_ch_width;
	uint16_t bin_pwr_count_5mhz;
	uint8_t bin_pwr_5mhz[MAX_NUM_BINS_5MHZ];
} __packed;

/**
 * struct spectral_samp_msg - Spectral SAMP message
 * @signature:          Validates the SAMP message
 * @freq:               Operating frequency in MHz
 * @vhtop_ch_freq_seg1: VHT Segment 1 centre frequency in MHz
 * @vhtop_ch_freq_seg2: VHT Segment 2 centre frequency in MHz
 * @agile_freq1:        Center frequency in MHz of the entire span(for 80+80 MHz
 *                      agile Scan it is primary 80 MHz span) across which
 *                      Agile Spectral is carried out. Applicable only for Agile
 *                      Spectral samples.
 * @agile_freq2:        Center frequency in MHz of the secondary 80 MHz span
 *                      across which Agile Spectral is carried out. Applicable
 *                      only for Agile Spectral samples in 80+80 MHz mode.
 * @freq_loading:       How busy was the channel
 * @dcs_enabled:        Whether DCS is enabled
 * @int_type:           Interference type indicated by DCS
 * @macaddr:            Indicates the device interface
 * @samp_data:          SAMP Data
 */
struct spectral_samp_msg {
	uint32_t signature;
	uint16_t freq;
	uint16_t vhtop_ch_freq_seg1;
	uint16_t vhtop_ch_freq_seg2;
	uint16_t agile_freq1;
	uint16_t agile_freq2;
	uint16_t freq_loading;
	uint16_t dcs_enabled;
	enum dcs_int_type int_type;
	uint8_t macaddr[6];
	struct spectral_samp_data samp_data;
} __packed;

#endif /* OPTIMIZED_SAMP_MESSAGE */
#endif
