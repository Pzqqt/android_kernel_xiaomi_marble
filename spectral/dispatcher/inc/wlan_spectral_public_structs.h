/*
 * Copyright (c) 2011,2017 The Linux Foundation. All rights reserved.
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

#ifdef ATH_SPECTRAL_USE_EMU_DEFAULTS
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
/* Static default values for spectral state and configuration.
 * These definitions should be treated as temporary. Ideally,
 * we should get the defaults from firmware - this will be discussed.
 *
 * Use defaults from Spectral Hardware Micro-Architecture
 * document (v1.0)
 */
#define SPECTRAL_SCAN_ACTIVE_DEFAULT           (0)
#define SPECTRAL_SCAN_ENABLE_DEFAULT           (0)
#define SPECTRAL_SCAN_COUNT_DEFAULT            (0)
#define SPECTRAL_SCAN_PERIOD_DEFAULT           (35)
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
#endif /* ATH_SPECTRAL_USE_EMU_DEFAULTS */

/* The below two definitions apply only to pre-11ac chipsets */
#define SPECTRAL_SCAN_SHORT_REPORT_DEFAULT     (1)
#define SPECTRAL_SCAN_FFT_PERIOD_DEFAULT       (1)

/**
 * enum spectral_debug - Spectral debug level
 * @ATH_DEBUG_SPECTRAL:  Minimal SPECTRAL debug
 * @ATH_DEBUG_SPECTRAL1: Normal SPECTRAL debug
 * @ATH_DEBUG_SPECTRAL2: Maximal SPECTRAL debug
 * @ATH_DEBUG_SPECTRAL3: Matched filterID display
 * @ATH_DEBUG_SPECTRAL4: One time dump of FFT report
 */
enum spectral_debug {
	ATH_DEBUG_SPECTRAL       = 0x00000100,
	ATH_DEBUG_SPECTRAL1      = 0x00000200,
	ATH_DEBUG_SPECTRAL2      = 0x00000400,
	ATH_DEBUG_SPECTRAL3      = 0x00000800,
	ATH_DEBUG_SPECTRAL4      = 0x00001000,
};

/**
 * enum SPECTRAL_CAPABILITY_TYPE - Spectral capability type
 * @SPECTRAL_CAP_PHYDIAG:              Phydiag capability
 * @SPECTRAL_CAP_RADAR:                Radar detection capability
 * @SPECTRAL_CAP_SPECTRAL_SCAN:        Spectral capability
 * @SPECTRAL_CAP_ADVNCD_SPECTRAL_SCAN: Advanced spectral capability
 */
typedef enum {
	SPECTRAL_CAP_PHYDIAG,
	SPECTRAL_CAP_RADAR,
	SPECTRAL_CAP_SPECTRAL_SCAN,
	SPECTRAL_CAP_ADVNCD_SPECTRAL_SCAN,
} SPECTRAL_CAPABILITY_TYPE;

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
	int          cycle_count;
	int          channel_load;
	int          per;
	int          noisefloor;
	u_int16_t    comp_usablity;
	int8_t       maxregpower;
	u_int16_t    comp_usablity_sec80;
	int8_t       maxregpower_sec80;
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
	u_int64_t spectral_mismatch;
	u_int64_t spectral_sec80_sfft_insufflen;
	u_int64_t spectral_no_sec80_sfft;
	u_int64_t spectral_vhtseg1id_mismatch;
	u_int64_t spectral_vhtseg2id_mismatch;
};

/**
 * struct spectral_caps - Spectral capabilities structure
 * @phydiag_cap:         Phydiag capability
 * @radar_cap:           Radar detection capability
 * @spectral_cap:        Spectral capability
 * @advncd_spectral_cap: Advanced spectral capability
 */
struct spectral_caps {
	u_int8_t phydiag_cap;
	u_int8_t radar_cap;
	u_int8_t spectral_cap;
	u_int8_t advncd_spectral_cap;
};

/**
 * struct spectral_config
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
	u_int16_t   ss_fft_period;
	u_int16_t   ss_period;
	u_int16_t   ss_count;
	u_int16_t   ss_short_report;
	u_int8_t    radar_bin_thresh_sel;
	u_int16_t   ss_spectral_pri;
	u_int16_t   ss_fft_size;
	u_int16_t   ss_gc_ena;
	u_int16_t   ss_restart_ena;
	u_int16_t   ss_noise_floor_ref;
	u_int16_t   ss_init_delay;
	u_int16_t   ss_nb_tone_thr;
	u_int16_t   ss_str_bin_thr;
	u_int16_t   ss_wb_rpt_mode;
	u_int16_t   ss_rssi_rpt_mode;
	u_int16_t   ss_rssi_thr;
	u_int16_t   ss_pwr_format;
	u_int16_t   ss_rpt_mode;
	u_int16_t   ss_bin_scale;
	u_int16_t   ss_dbm_adj;
	u_int16_t   ss_chn_mask;
	int8_t      ss_nf_cal[AH_MAX_CHAINS * 2];
	int8_t      ss_nf_pwr[AH_MAX_CHAINS * 2];
	int32_t     ss_nf_temp_data;
};

/**
 * struct spectral_caps - Spectral capabilities structure
 * @phydiag_cap:         Phydiag capability
 * @radar_cap:           Radar detection capability
 * @spectral_cap:        Spectral capability
 * @advncd_spectral_cap: Advanced spectral capability
 */
typedef enum _dcs_int_type {
	SPECTRAL_DCS_INT_NONE,
	SPECTRAL_DCS_INT_CW,
	SPECTRAL_DCS_INT_WIFI
} DCS_INT_TYPE;

/**
 * struct INTERF_RSP - Interference record
 * @interf_type:         eINTERF_TYPE giving type of interference
 * @interf_min_freq:     Minimum frequency in MHz at which interference has been
 * found
 * @interf_max_freq:     Maximum frequency in MHz at which interference has been
 * found
 * @advncd_spectral_cap: Advanced spectral capability
 */
struct INTERF_RSP {
	u_int8_t  interf_type;
	u_int16_t interf_min_freq;
	u_int16_t interf_max_freq;
} __ATTRIB_PACKED;

/**
 * struct INTERF_SRC_RSP - List of interference sources
 * @count: Number of interference records
 * @interf: Array of interference records
 */
struct INTERF_SRC_RSP {
	u_int16_t count;
	struct INTERF_RSP interf[MAX_INTERF];
} __ATTRIB_PACKED;

/**
 * struct spectral_classifier_params -
 * @spectral_20_40_mode:  Is AP in 20/40 mode?
 * @spectral_dc_index:    DC index
 * @spectral_dc_in_mhz:   DC in MHz
 * @upper_chan_in_mhz:    Upper channel in MHz
 * @lower_chan_in_mhz:    Lower channel in MHz
 */
typedef struct spectral_classifier_params {
	int spectral_20_40_mode;
	int spectral_dc_index;
	int spectral_dc_in_mhz;
	int upper_chan_in_mhz;
	int lower_chan_in_mhz;
} __ATTRIB_PACKED SPECTRAL_CLASSIFIER_PARAMS;

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
typedef struct spectral_samp_data {
	int16_t                              spectral_data_len;
	int16_t                              spectral_data_len_sec80;
	int16_t                              spectral_rssi;
	int16_t                              spectral_rssi_sec80;
	int8_t                               spectral_combined_rssi;
	int8_t                               spectral_upper_rssi;
	int8_t                               spectral_lower_rssi;
	int8_t  spectral_chain_ctl_rssi[MAX_SPECTRAL_CHAINS];
	int8_t  spectral_chain_ext_rssi[MAX_SPECTRAL_CHAINS];
	u_int8_t                             spectral_max_scale;
	int16_t                              spectral_bwinfo;
	int32_t                              spectral_tstamp;
	int16_t                              spectral_max_index;
	int16_t                              spectral_max_index_sec80;
	int16_t                              spectral_max_mag;
	int16_t                              spectral_max_mag_sec80;
	u_int8_t                             spectral_max_exp;
	int32_t                              spectral_last_tstamp;
	int16_t                              spectral_upper_max_index;
	int16_t                              spectral_lower_max_index;
	u_int8_t                             spectral_nb_upper;
	u_int8_t                             spectral_nb_lower;
	struct spectral_classifier_params    classifier_params;
	u_int16_t                            bin_pwr_count;
	/* For 11ac chipsets prior to AR900B version 2.0, a max of 512 bins are
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
	u_int8_t                             lb_edge_extrabins;
	u_int8_t                             rb_edge_extrabins;
	u_int16_t                            bin_pwr_count_sec80;
	u_int8_t                             bin_pwr[MAX_NUM_BINS];
	u_int8_t                             bin_pwr_sec80[MAX_NUM_BINS];
	struct INTERF_SRC_RSP                interf_list;
	int16_t                              noise_floor;
	int16_t                              noise_floor_sec80;
	u_int32_t                            ch_width;
} __ATTRIB_PACKED SPECTRAL_SAMP_DATA;

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
typedef struct spectral_samp_msg {
	u_int32_t             signature;
	u_int16_t             freq;
	u_int16_t             vhtop_ch_freq_seg1;
	u_int16_t             vhtop_ch_freq_seg2;
	u_int16_t             freq_loading;
	u_int16_t             dcs_enabled;
	DCS_INT_TYPE          int_type;
	u_int8_t              macaddr[6];
	SPECTRAL_SAMP_DATA    samp_data;
} __ATTRIB_PACKED SPECTRAL_SAMP_MSG;

#ifdef WIN32
#pragma pack(pop, spectral)
#endif
#ifdef __ATTRIB_PACKED
#undef __ATTRIB_PACKED
#endif

#endif /* _WLAN_SPECTRAL_PUBLIC_STRUCTS_H_ */

