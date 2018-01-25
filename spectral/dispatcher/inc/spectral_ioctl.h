/*
 * Copyright (c) 2011, 2017-2018 The Linux Foundation. All rights reserved.
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

struct ath_spectral_caps {
	uint8_t phydiag_cap;
	uint8_t radar_cap;
	uint8_t spectral_cap;
	uint8_t advncd_spectral_cap;
};

#define SPECTRAL_IOCTL_PARAM_NOVAL (65535)

#endif
