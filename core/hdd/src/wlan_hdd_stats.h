/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
 * enum - data_rate_11ac_max_mcs
 * @DATA_RATE_11AC_MAX_MCS_7: MCS7 rate
 * @DATA_RATE_11AC_MAX_MCS_8: MCS8 rate
 * @DATA_RATE_11AC_MAX_MCS_9: MCS9 rate
 * @DATA_RATE_11AC_MAX_MCS_NA:i Not applicable
 */
enum data_rate_11ac_max_mcs {
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

/**
 * struct hdd_ll_stats_context - hdd link layer stats context
 *
 * @request_id: userspace-assigned link layer stats request id
 * @request_bitmap: userspace-assigned link layer stats request bitmap
 * @response_event: LL stats request wait event
 */
struct hdd_ll_stats_context {
	uint32_t request_id;
	uint32_t request_bitmap;
	struct completion response_event;
	spinlock_t context_lock;
};

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

/**
 * __wlan_hdd_cfg80211_ll_stats_ext_set_param - config monitor parameters
 * @wiphy: wiphy handle
 * @wdev: wdev handle
 * @data: user layer input
 * @data_len: length of user layer input
 *
 * return: 0 success, einval failure
 */
int wlan_hdd_cfg80211_ll_stats_ext_set_param(struct wiphy *wiphy,
					     struct wireless_dev *wdev,
					     const void *data,
					     int data_len);
/**
 * hdd_get_interface_info() - get interface info
 * @adapter: Pointer to device adapter
 * @info: Pointer to interface info
 *
 * Return: bool
 */
bool hdd_get_interface_info(struct hdd_adapter *adapter,
			    tpSirWifiInterfaceInfo info);

/**
 * wlan_hdd_ll_stats_get() - Get Link Layer statistics from FW
 * @adapter: Pointer to device adapter
 * @req_id: request id
 * @req_mask: bitmask used by FW for the request
 *
 * Return: 0 on success and error code otherwise
 */
int wlan_hdd_ll_stats_get(struct hdd_adapter *adapter, uint32_t req_id,
			  uint32_t req_mask);

#else

static inline void hdd_init_ll_stats_ctx(void)
{
}

static inline bool hdd_link_layer_stats_supported(void)
{
	return false;
}

static inline int
wlan_hdd_cfg80211_ll_stats_ext_set_param(struct wiphy *wiphy,
					 struct wireless_dev *wdev,
					 const void *data,
					 int data_len)
{
	return -EINVAL;
}

static inline
int wlan_hdd_ll_stats_get(hdd_adapter_t *adapter, uint32_t req_id,
			  uint32_t req_mask)
{
	return -EINVAL;
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

/**
 * wlan_hdd_cfg80211_stats_ext2_callback - stats_ext2_callback
 * @ctx: hdd context
 * @pmsg: sir_sme_rx_aggr_hole_ind
 *
 * Return: void
 */
void wlan_hdd_cfg80211_stats_ext2_callback(void *ctx,
				struct sir_sme_rx_aggr_hole_ind *pmsg);

void wlan_hdd_cfg80211_link_layer_stats_callback(void *ctx,
						 int indType, void *pRsp);
/**
 * wlan_hdd_cfg80211_link_layer_stats_ext_callback() - Callback for LL ext
 * @ctx: HDD context
 * @rsp: msg from FW
 *
 * This function is an extension of
 * wlan_hdd_cfg80211_link_layer_stats_callback. It converts
 * monitoring parameters offloaded to NL data and send the same to the
 * kernel/upper layers.
 *
 * Return: None.
 */
void wlan_hdd_cfg80211_link_layer_stats_ext_callback(tHddHandle ctx,
						     tSirLLStatsResults *rsp);

/**
 * wlan_hdd_get_rcpi() - Wrapper to get current RCPI
 * @adapter: adapter upon which the measurement is requested
 * @mac: peer addr for which measurement is requested
 * @rcpi_value: pointer to where the RCPI should be returned
 * @measurement_type: type of rcpi measurement
 *
 * This is a wrapper function for getting RCPI, invoke this function only
 * when rcpi support is enabled in firmware
 *
 * Return: 0 for success, non-zero for failure
 */
int wlan_hdd_get_rcpi(struct hdd_adapter *adapter, uint8_t *mac,
		      int32_t *rcpi_value,
		      enum rcpi_measurement_type measurement_type);

#endif /* end #if !defined(WLAN_HDD_STATS_H) */

