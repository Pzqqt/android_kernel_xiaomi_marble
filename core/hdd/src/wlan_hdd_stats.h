/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/**
 * DOC : wlan_hdd_stats.h
 *
 * WLAN Host Device Driver statistics related implementation
 *
 */

#if !defined(WLAN_HDD_STATS_H)
#define WLAN_HDD_STATS_H

#include "wlan_hdd_main.h"

#define INVALID_MCS_IDX 255
#define MAX_HT_MCS_IDX 8
#define MAX_VHT_MCS_IDX 10

#define DATA_RATE_11AC_MCS_MASK    0x03

/* LL stats get request time out value */
#define WLAN_WAIT_TIME_LL_STATS 800

#define WLAN_HDD_TGT_NOISE_FLOOR_DBM     (-96)

/**
 * struct index_vht_data_rate_type - vht data rate type
 * @beacon_rate_index: Beacon rate index
 * @supported_VHT80_rate: VHT80 rate
 * @supported_VHT40_rate: VHT40 rate
 * @supported_VHT20_rate: VHT20 rate
 */
struct index_vht_data_rate_type {
	uint8_t beacon_rate_index;
	uint16_t supported_VHT80_rate[2];
	uint16_t supported_VHT40_rate[2];
	uint16_t supported_VHT20_rate[2];
};

/**
 * enum - eDataRate11ACMaxMcs
 * @DATA_RATE_11AC_MAX_MCS_7: MCS7 rate
 * @DATA_RATE_11AC_MAX_MCS_8: MCS8 rate
 * @DATA_RATE_11AC_MAX_MCS_9: MCS9 rate
 * @DATA_RATE_11AC_MAX_MCS_NA:i Not applicable
 */
enum eDataRate11ACMaxMcs{
	DATA_RATE_11AC_MAX_MCS_7,
	DATA_RATE_11AC_MAX_MCS_8,
	DATA_RATE_11AC_MAX_MCS_9,
	DATA_RATE_11AC_MAX_MCS_NA
};

/**
 * struct index_data_rate_type - non vht data rate type
 * @beacon_rate_index: Beacon rate index
 * @supported_rate: Supported rate table
 */
struct index_data_rate_type {
	uint8_t beacon_rate_index;
	uint16_t supported_rate[4];
};

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
/*
 * Used to allocate the size of 4096 for the link layer stats.
 * The size of 4096 is considered assuming that all data per
 * respective event fit with in the limit.Please take a call
 * on the limit based on the data requirements on link layer
 * statistics.
 */
#define LL_STATS_EVENT_BUF_SIZE 4096

/**
 * wlan_hdd_cfg80211_ll_stats_set() - set link layer stats
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: int
 */
int wlan_hdd_cfg80211_ll_stats_set(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data,
				   int data_len);

/**
 * wlan_hdd_cfg80211_ll_stats_get() - get link layer stats
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: int
 */
int wlan_hdd_cfg80211_ll_stats_get(struct wiphy *wiphy,
				   struct wireless_dev *wdev,
				   const void *data,
				   int data_len);


/**
 * wlan_hdd_cfg80211_ll_stats_clear() - clear link layer stats
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: int
 */
int wlan_hdd_cfg80211_ll_stats_clear(struct wiphy *wiphy,
				     struct wireless_dev *wdev,
				     const void *data,
				     int data_len);

void hdd_init_ll_stats_ctx(void);

static inline bool hdd_link_layer_stats_supported(void)
{
	return true;
}

#else

static inline void hdd_init_ll_stats_ctx(void)
{
	return;
}

static inline bool hdd_link_layer_stats_supported(void)
{
	return false;
}

#endif /* End of WLAN_FEATURE_LINK_LAYER_STATS */

#ifdef WLAN_FEATURE_STATS_EXT
/**
 * wlan_hdd_cfg80211_stats_ext_request() - ext stats request
 * @wiphy: Pointer to wiphy
 * @wdev: Pointer to wdev
 * @data: Pointer to data
 * @data_len: Data length
 *
 * Return: int
 */
int wlan_hdd_cfg80211_stats_ext_request(struct wiphy *wiphy,
					struct wireless_dev *wdev,
					const void *data,
					int data_len);

#else
static inline void wlan_hdd_cfg80211_stats_ext_init(hdd_context_t *pHddCtx) {}
#endif /* End of WLAN_FEATURE_STATS_EXT */

/**
 * wlan_hdd_cfg80211_get_station() - get station statistics
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @mac: Pointer to mac
 * @sinfo: Pointer to station info
 *
 * Return: 0 for success, non-zero for failure
 */
#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 16, 0))
int wlan_hdd_cfg80211_get_station(struct wiphy *wiphy,
				  struct net_device *dev, const uint8_t *mac,
				  struct station_info *sinfo);
#else
int wlan_hdd_cfg80211_get_station(struct wiphy *wiphy,
				  struct net_device *dev, uint8_t *mac,
				  struct station_info *sinfo);
#endif

/**
 * wlan_hdd_cfg80211_dump_station() - dump station statistics
 * @wiphy: Pointer to wiphy
 * @dev: Pointer to network device
 * @idx: variable to determine whether to get stats or not
 * @mac: Pointer to mac
 * @sinfo: Pointer to station info
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_cfg80211_dump_station(struct wiphy *wiphy,
				struct net_device *dev,
				int idx, u8 *mac,
				struct station_info *sinfo);

struct net_device_stats *hdd_get_stats(struct net_device *dev);

int wlan_hdd_cfg80211_dump_survey(struct wiphy *wiphy,
				  struct net_device *dev,
				  int idx, struct survey_info *survey);

void hdd_display_hif_stats(void);
void hdd_clear_hif_stats(void);

void wlan_hdd_cfg80211_stats_ext_callback(void *ctx,
					  tStatsExtEvent *msg);

void wlan_hdd_cfg80211_link_layer_stats_callback(void *ctx,
						 int indType, void *pRsp);
#endif /* end #if !defined(WLAN_HDD_STATS_H) */

