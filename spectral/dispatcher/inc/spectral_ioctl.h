/*
 * Copyright (c) 2011, 2017-2019 The Linux Foundation. All rights reserved.
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

/*
 * ioctl parameter types
 */

#define SPECTRAL_PARAM_FFT_PERIOD        (1)
#define SPECTRAL_PARAM_SCAN_PERIOD       (2)
#define SPECTRAL_PARAM_SCAN_COUNT        (3)
#define SPECTRAL_PARAM_SHORT_REPORT      (4)
#define SPECTRAL_PARAM_SPECT_PRI         (5)
#define SPECTRAL_PARAM_FFT_SIZE          (6)
#define SPECTRAL_PARAM_GC_ENA            (7)
#define SPECTRAL_PARAM_RESTART_ENA       (8)
#define SPECTRAL_PARAM_NOISE_FLOOR_REF   (9)
#define SPECTRAL_PARAM_INIT_DELAY        (10)
#define SPECTRAL_PARAM_NB_TONE_THR       (11)
#define SPECTRAL_PARAM_STR_BIN_THR       (12)
#define SPECTRAL_PARAM_WB_RPT_MODE       (13)
#define SPECTRAL_PARAM_RSSI_RPT_MODE     (14)
#define SPECTRAL_PARAM_RSSI_THR          (15)
#define SPECTRAL_PARAM_PWR_FORMAT        (16)
#define SPECTRAL_PARAM_RPT_MODE          (17)
#define SPECTRAL_PARAM_BIN_SCALE         (18)
#define SPECTRAL_PARAM_DBM_ADJ           (19)
#define SPECTRAL_PARAM_CHN_MASK          (20)
#define SPECTRAL_PARAM_ACTIVE            (21)
#define SPECTRAL_PARAM_STOP              (22)
#define SPECTRAL_PARAM_ENABLE            (23)

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
};

#define SPECTRAL_IOCTL_PARAM_NOVAL (65535)

#define MAX_SPECTRAL_CHAINS          3
#define MAX_NUM_BINS                 520
/* 5 categories x (lower + upper) bands */
#define MAX_INTERF                   10

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
	uint8_t bin_pwr[MAX_NUM_BINS];
	uint8_t bin_pwr_sec80[MAX_NUM_BINS];
	struct interf_src_rsp interf_list;
	int16_t noise_floor;
	int16_t noise_floor_sec80;
	uint32_t ch_width;
	uint8_t spectral_agc_total_gain;
	uint8_t spectral_agc_total_gain_sec80;
	uint8_t spectral_gainchange;
	uint8_t spectral_gainchange_sec80;
} __packed;

/**
 * struct spectral_samp_msg - Spectral SAMP message
 * @signature:          Validates the SAMP message
 * @freq:               Operating frequency in MHz
 * @vhtop_ch_freq_seg1: VHT Segment 1 centre frequency in MHz
 * @vhtop_ch_freq_seg2: VHT Segment 2 centre frequency in MHz
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
	uint16_t freq_loading;
	uint16_t dcs_enabled;
	enum dcs_int_type int_type;
	uint8_t macaddr[6];
	struct spectral_samp_data samp_data;
} __packed;

#endif
