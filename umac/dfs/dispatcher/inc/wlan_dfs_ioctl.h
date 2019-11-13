/*
 * Copyright (c) 2011, 2016-2019 The Linux Foundation. All rights reserved.
 * Copyright (c) 2010, Atheros Communications Inc.
 * All Rights Reserved.
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

/**
 * DOC: This file has dfs IOCTL Defines.
 */

#ifndef _DFS_IOCTL_H_
#define _DFS_IOCTL_H_

#define DFS_MUTE_TIME            1
#define DFS_SET_THRESH           2
#define DFS_GET_THRESH           3
#define DFS_GET_USENOL           4
#define DFS_SET_USENOL           5
#define DFS_RADARDETECTS         6
#define DFS_BANGRADAR            7
#define DFS_SHOW_NOL             8
#define DFS_DISABLE_DETECT       9
#define DFS_ENABLE_DETECT        10
#define DFS_DISABLE_FFT          11
#define DFS_ENABLE_FFT           12
#define DFS_SET_DEBUG_LEVEL      13
#define DFS_GET_NOL              14
#define DFS_SET_NOL              15

#define DFS_SET_FALSE_RSSI_THRES 16
#define DFS_SET_PEAK_MAG         17
#define DFS_IGNORE_CAC           18
#define DFS_SET_NOL_TIMEOUT      19
#define DFS_GET_CAC_VALID_TIME   20
#define DFS_SET_CAC_VALID_TIME   21
#define DFS_SHOW_NOLHISTORY      22
#define DFS_SHOW_PRECAC_LISTS    23
#define DFS_RESET_PRECAC_LISTS   24
#define DFS_SET_DISABLE_RADAR_MARKING 25
#define DFS_GET_DISABLE_RADAR_MARKING 26

#define DFS_INJECT_SEQUENCE 27
#define DFS_ALLOW_HW_PULSES 28
#define DFS_SET_PRI_MULTIPILER   29

#define RESTRICTED_80P80_START_CHAN 132
#define RESTRICTED_80P80_END_CHAN 161

/* Check if the given channels are within restricted 80P80 start chan(132) and
 * end chan (161).
 */
#define CHAN_WITHIN_RESTRICTED_80P80(chan, cfreq_seg2) \
	((((chan) >= RESTRICTED_80P80_START_CHAN) && \
	  ((chan) <= RESTRICTED_80P80_END_CHAN) && \
	  ((cfreq_seg2) >= RESTRICTED_80P80_START_CHAN) && \
	  ((cfreq_seg2) <= RESTRICTED_80P80_END_CHAN)) ? true : false)

/*
 * Spectral IOCTLs use DFS_LAST_IOCTL as the base.
 * This must always be the last IOCTL in DFS and have
 * the highest value.
 */
#define DFS_LAST_IOCTL 29

#ifndef DFS_CHAN_MAX
#define DFS_CHAN_MAX 1023
#endif

/**
 * struct dfsreq_nolelem - NOL elements.
 * @nol_freq:          NOL channel frequency.
 * @nol_chwidth:       NOL channel width.
 * @nol_start_ticks:   OS ticks when the NOL timer started.
 * @nol_timeout_ms:    Nol timeout value in msec.
 */

struct dfsreq_nolelem {
	uint16_t        nol_freq;
	uint16_t        nol_chwidth;
	unsigned long   nol_start_ticks;
	uint32_t        nol_timeout_ms;
};

struct dfsreq_nolinfo {
	uint32_t  dfs_ch_nchans;
	struct dfsreq_nolelem dfs_nol[DFS_CHAN_MAX];
};

/*
 * IOCTL parameter types
 */

#define DFS_PARAM_FIRPWR  1
#define DFS_PARAM_RRSSI   2
#define DFS_PARAM_HEIGHT  3
#define DFS_PARAM_PRSSI   4
#define DFS_PARAM_INBAND  5
/* 5413 specific parameters */
#define DFS_PARAM_RELPWR  7
#define DFS_PARAM_RELSTEP 8
#define DFS_PARAM_MAXLEN  9

/**
 * struct dfs_ioctl_params - DFS ioctl params.
 * @dfs_firpwr:     FIR pwr out threshold.
 * @dfs_rrssi:      Radar rssi thresh.
 * @dfs_height:     Pulse height thresh.
 * @dfs_prssi:      Pulse rssi thresh.
 * @dfs_inband:     Inband thresh.
 * @dfs_relpwr:     Pulse relative pwr thresh.
 * @dfs_relstep:    Pulse relative step thresh.
 * @dfs_maxlen:     Pulse max duration.
 */
struct dfs_ioctl_params {
	int32_t dfs_firpwr;
	int32_t dfs_rrssi;
	int32_t dfs_height;
	int32_t dfs_prssi;
	int32_t dfs_inband;
	int32_t dfs_relpwr;
	int32_t dfs_relstep;
	int32_t dfs_maxlen;
};

/* Types of Bangradar commands:
 * @DFS_BANGRADAR_FOR_ALL_SUBCHANS          : Bangradar with no arguments.
 *                                            All the subchannels in the current
 *                                            channel shall be added.
 * @DFS_BANGRADAR_FOR_ALL_SUBCHANS_OF_SEGID : Bangradar with 1 (seg_id) argument
 *                                            All subchannels of the specific
 *                                            seg_id shall be added.
 * @DFS_BANGRADAR_FOR_SPECIFIC_SUBCHANS     : Bangradar with all (segment ID,
 *                                            is_chirp and frequency offset)
 *                                            arguments.
 *                                            Only radar infected subchannels
 *                                            of the specific seg_id shall be
 *                                            added.
 *
 * (Unless all arguments are given, we cannot determine which specific
 * subchannels to simulate the radar on, hence simulate in all subchans).
 */
enum dfs_bangradar_types {
	DFS_NO_BANGRADAR = 0,
	DFS_BANGRADAR_FOR_ALL_SUBCHANS,
	DFS_BANGRADAR_FOR_ALL_SUBCHANS_OF_SEGID,
	DFS_BANGRADAR_FOR_SPECIFIC_SUBCHANS,
	DFS_INVALID_BANGRADAR_TYPE
};

/**
 * struct dfs_bangradar_params - DFS bangradar params.
 * @bangradar_type: Type of Bangradar.
 * @seg_id:         Segment ID information.
 * @is_chirp:       Chirp radar or not.
 * @freq_offset:    Frequency offset at which radar was found.
 */
struct dfs_bangradar_params {
	enum dfs_bangradar_types bangradar_type;
	uint8_t seg_id;
	uint8_t is_chirp;
	int32_t freq_offset;
};
#define DFS_IOCTL_PARAM_NOVAL  65535
#define DFS_IOCTL_PARAM_ENABLE 0x8000

/* Random channel flags */
/* Flag to exclude current operating channels */
#define DFS_RANDOM_CH_FLAG_NO_CURR_OPE_CH       0x0001 /* 0000 0000 0000 0001 */

/* Flag to exclude weather channels */
#define DFS_RANDOM_CH_FLAG_NO_WEATHER_CH        0x0002 /* 0000 0000 0000 0010 */

/* Flag to exclude indoor channels */
#define DFS_RANDOM_CH_FLAG_NO_LOWER_5G_CH       0x0004 /* 0000 0000 0000 0100 */

/* Flag to exclude outdoor channels */
#define DFS_RANDOM_CH_FLAG_NO_UPEER_5G_CH       0x0008 /* 0000 0000 0000 1000 */

/* Flag to exclude dfs channels */
#define DFS_RANDOM_CH_FLAG_NO_DFS_CH            0x0010 /* 0000 0000 0001 0000 */

/* Flag to exclude all 5GHz channels */
#define DFS_RANDOM_CH_FLAG_NO_5GHZ_CH           0x0020 /* 0000 0000 0010 0000 */

/* Flag to exclude all 2.4GHz channels */
#define DFS_RANDOM_CH_FLAG_NO_2GHZ_CH           0x0040 /* 0000 0000 0100 0000 */

/* Flag to enable Reduced BW Agile DFS */
#define DFS_RANDOM_CH_FLAG_ENABLE_REDUCED_BW    0x0080 /* 0000 0000 1000 0000 */

/* Flag to exclude Japan W53 channnels */
#define DFS_RANDOM_CH_FLAG_NO_JAPAN_W53_CH      0x0100 /* 0000 0001 0000 0000 */

/* Restricted 80P80 MHz is enabled */
#define DFS_RANDOM_CH_FLAG_RESTRICTED_80P80_ENABLED 0x0200
						       /* 0000 0010 0000 0000 */

/**
 * struct wlan_dfs_caps - DFS capability structure.
 * @wlan_dfs_ext_chan_ok:         Can radar be detected on the extension chan?
 * @wlan_dfs_combined_rssi_ok:    Can use combined radar RSSI?
 * @wlan_dfs_use_enhancement:     This flag is used to indicate if radar
 *                                detection scheme should use enhanced chirping
 *                                detection algorithm. This flag also determines
 *                                if certain radar data should be discarded to
 *                                minimize false detection of radar.
 * @wlan_strong_signal_diversiry: Strong Signal fast diversity count.
 * @wlan_chip_is_bb_tlv:          Chip is BB TLV?
 * @wlan_chip_is_over_sampled:    Is Over sampled.
 * @wlan_chip_is_ht160:           IS VHT160?
 * @wlan_chip_is_false_detect:    Is False detected?
 * @wlan_fastdiv_val:             Goes with wlan_strong_signal_diversiry: If we
 *                                have fast diversity capability, read off
 *                                Strong Signal fast diversity count set in the
 *                                ini file, and store so we can restore the
 *                                value when radar is disabled.
 */
struct wlan_dfs_caps {
	uint32_t wlan_dfs_ext_chan_ok:1,
			 wlan_dfs_combined_rssi_ok:1,
			 wlan_dfs_use_enhancement:1,
			 wlan_strong_signal_diversiry:1,
			 wlan_chip_is_bb_tlv:1,
			 wlan_chip_is_over_sampled:1,
			 wlan_chip_is_ht160:1,
			 wlan_chip_is_false_detect:1;
	uint32_t wlan_fastdiv_val;
};

/**
 * struct wlan_dfs_phyerr_param - DFS Phyerr structure.
 * @pe_firpwr:     FIR pwr out threshold.
 * @pe_rrssi:      Radar rssi thresh.
 * @pe_height:     Pulse height thresh.
 * @pe_prssi:      Pulse rssi thresh.
 * @pe_inband:     Inband thresh.
 * @pe_relpwr:     Relative power threshold in 0.5dB steps.
 * @pe_relstep:    Pulse Relative step threshold in 0.5dB steps.
 * @pe_maxlen:     Max length of radar sign in 0.8us units.
 * @pe_usefir128:  Use the average in-band power measured over 128 cycles.
 * @pe_blockradar: Enable to block radar check if pkt detect is done via OFDM
 *                 weak signal detect or pkt is detected immediately after tx
 *                 to rx transition.
 * @pe_enmaxrssi:  Enable to use the max rssi instead of the last rssi during
 *                 fine gain changes for radar detection.
 */
struct wlan_dfs_phyerr_param {
	int32_t    pe_firpwr;
	int32_t    pe_rrssi;
	int32_t    pe_height;
	int32_t    pe_prssi;
	int32_t    pe_inband;
	uint32_t   pe_relpwr;
	uint32_t   pe_relstep;
	uint32_t   pe_maxlen;
	bool       pe_usefir128;
	bool       pe_blockradar;
	bool       pe_enmaxrssi;
};

/**
 * enum WLAN_DFS_EVENTS - DFS Events that will be sent to userspace
 * @WLAN_EV_RADAR_DETECTED: Radar is detected
 * @WLAN_EV_CAC_STARTED:    CAC timer has started
 * @WLAN_EV_CAC_COMPLETED:  CAC timer completed
 * @WLAN_EV_NOL_STARTED:    NOL started
 * @WLAN_EV_NOL_FINISHED:   NOL Completed
 *
 * DFS events such as radar detected, CAC started,
 * CAC completed, NOL started, NOL finished
 */
enum WLAN_DFS_EVENTS {
	WLAN_EV_RADAR_DETECTED,
	WLAN_EV_CAC_STARTED,
	WLAN_EV_CAC_COMPLETED,
	WLAN_EV_NOL_STARTED,
	WLAN_EV_NOL_FINISHED,
};

#if defined(WLAN_DFS_PARTIAL_OFFLOAD) && defined(WLAN_DFS_SYNTHETIC_RADAR)
/**
 * Structure of Pulse to be injected into the DFS Module
 * ******************************************************
 * Header
 * ======
 * ----------|--------------|
 * num_pulses| total_len_seq|
 * ----------|--------------|
 * Buffer Contents per pulse:
 * ==========================
 * ------|----------|-----------|----------|-----------|---------------|--------
 * r_rssi|r_ext_rssi|r_rs_tstamp|r_fulltsf |fft_datalen|total_len_pulse|FFT
 *       |          |           |          |           |               |Buffer..
 * ------|----------|-----------|----------|-----------|---------------|--------
 */

/**
 * struct synthetic_pulse - Radar Pulse Structure to be filled on reading the
 * user file.
 * @r_rssi:          RSSI of the pulse.
 * @r_ext_rssi:      Extension Channel RSSI.
 * @r_rs_tstamp:     Timestamp.
 * @r_fulltsf:       TSF64.
 * @fft_datalen:     Total len of FFT.
 * @total_len_pulse: Total len of the pulse.
 * @fft_buf:         Pointer to fft data.
 */

struct synthetic_pulse {
	uint8_t r_rssi;
	uint8_t r_ext_rssi;
	uint32_t r_rs_tstamp;
	uint64_t r_fulltsf;
	uint16_t fft_datalen;
	uint16_t total_len_pulse;
	unsigned char *fft_buf;
} qdf_packed;

/**
 * struct synthetic_seq - Structure to hold an array of pointers to the
 * pulse structure.
 * @num_pulses:    Total num of pulses in the sequence.
 * @total_len_seq: Total len of the sequence.
 * @pulse:         Array of pointers to synthetic_pulse structure.
 */

struct synthetic_seq {
	uint8_t num_pulses;
	uint32_t total_len_seq;
	struct synthetic_pulse *pulse[0];
};

/**
 * struct seq_store - Structure to hold an array of pointers to the synthetic
 * sequence structure.
 * @num_sequence: Total number of "sequence of pulses" in the file.
 * @seq_arr:      Array of pointers to synthetic_seq structure.
 */

struct seq_store {
	uint8_t num_sequence;
	struct synthetic_seq *seq_arr[0];
};
#endif /* WLAN_DFS_PARTIAL_OFFLOAD && WLAN_DFS_SYNTHETIC_RADAR */
#endif  /* _DFS_IOCTL_H_ */
