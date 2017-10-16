/*
 * Copyright (c) 2011, 2016-2017 The Linux Foundation. All rights reserved.
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
#define DFS_SECOND_SEGMENT_BANGRADAR  23
#define DFS_SHOW_PRECAC_LISTS    24
#define DFS_RESET_PRECAC_LISTS   25

/*
 * Spectral IOCTLs use DFS_LAST_IOCTL as the base.
 * This must always be the last IOCTL in DFS and have
 * the highest value.
 */
#define DFS_LAST_IOCTL 26

#ifndef IEEE80211_CHAN_MAX
#define IEEE80211_CHAN_MAX 1023
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
	struct dfsreq_nolelem dfs_nol[IEEE80211_CHAN_MAX];
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

#endif  /* _DFS_IOCTL_H_ */
