/*
 * Copyright (c) 2011,2017-2018 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_SPECTRAL_PUBLIC_STRUCTS_H_
#define _WLAN_SPECTRAL_PUBLIC_STRUCTS_H_

#ifdef WIN32
#pragma pack(push, spectral, 1)
#define __ATTRIB_PACKED
#else
#ifndef __ATTRIB_PACKED
#define __ATTRIB_PACKED __attribute__ ((packed))
#endif
#endif

#ifndef AH_MAX_CHAINS
#define AH_MAX_CHAINS 3
#endif

#define MAX_NUM_CHANNELS             255
#define MAX_SPECTRAL_CHAINS          3
#define MAX_NUM_BINS                 520
#define SPECTRAL_PHYERR_PARAM_NOVAL  65535
/* 5 categories x (lower + upper) bands */
#define MAX_INTERF                   10

/* ioctl parameter types */
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
 */
struct spectral_diag_stats {
	uint64_t spectral_mismatch;
	uint64_t spectral_sec80_sfft_insufflen;
	uint64_t spectral_no_sec80_sfft;
	uint64_t spectral_vhtseg1id_mismatch;
	uint64_t spectral_vhtseg2id_mismatch;
};

/**
 * struct spectral_caps - Spectral capabilities structure
 * @phydiag_cap:         Phydiag capability
 * @radar_cap:           Radar detection capability
 * @spectral_cap:        Spectral capability
 * @advncd_spectral_cap: Advanced spectral capability
 * @hw_gen: Spectral hw generation
 */
struct spectral_caps {
	uint8_t phydiag_cap;
	uint8_t radar_cap;
	uint8_t spectral_cap;
	uint8_t advncd_spectral_cap;
	uint32_t hw_gen;
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
} __ATTRIB_PACKED;

/**
 * struct interf_src_rsp - List of interference sources
 * @count: Number of interference records
 * @interf: Array of interference records
 */
struct interf_src_rsp {
	uint16_t count;
	struct interf_rsp interf[MAX_INTERF];
} __ATTRIB_PACKED;

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
} __ATTRIB_PACKED;

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
} __ATTRIB_PACKED;

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
} __ATTRIB_PACKED;

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
};
#ifdef WIN32
#pragma pack(pop, spectral)
#endif
#ifdef __ATTRIB_PACKED
#undef __ATTRIB_PACKED
#endif

#endif				/* _WLAN_SPECTRAL_PUBLIC_STRUCTS_H_ */
