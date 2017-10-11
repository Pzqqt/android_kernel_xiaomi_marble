/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/* dfs regions definitions */
/* un-initialized region */
#define DFS_UNINIT_REGION_VAL   0

/* FCC region */
#define DFS_FCC_REGION_VAL      1

/* ETSI region */
#define DFS_ETSI_REGION_VAL     2

/* MKK region */
#define DFS_MKK_REGION_VAL      3

/* China region */
#define DFS_CN_REGION_VAL       4

/* Korea region */
#define DFS_KR_REGION_VAL       5

/* Undefined region */
#define DFS_UNDEF_REGION_VAL    6

/* Channel width definitions */
/* 20MHz channel width */
#define DFS_CH_WIDTH_20MHZ      0

/* 40MHz channel width */
#define DFS_CH_WIDTH_40MHZ      1

/* 80MHz channel width */
#define DFS_CH_WIDTH_80MHZ      2

/* 160MHz channel width */
#define DFS_CH_WIDTH_160MHZ     3

/* 80+80 non-contiguous */
#define DFS_CH_WIDTH_80P80MHZ   4

/* 5MHz channel width */
#define DFS_CH_WIDTH_5MHZ       5

/* 10MHz channel width */
#define DFS_CH_WIDTH_10MHZ      6

/* Invalid channel width */
#define DFS_CH_WIDTH_INVALID    7

/* Max channel width */
#define DFS_CH_WIDTH_MAX        8

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

/* Next 5GHz channel number */
#define DFS_80_NUM_SUB_CHANNNEL                 4

/* Next 5GHz channel number */
#define DFS_NEXT_5GHZ_CHANNEL                   4

/* Number of 20MHz channels in bitmap */
#define DFS_MAX_20M_SUB_CH                      8

/* Number of 80MHz channels in 5GHz band */
#define DFS_MAX_80MHZ_BANDS                     6

/* Start channel and center channel diff in 80Mhz */
#define DFS_80MHZ_START_CENTER_CH_DIFF          6

/* Max number of channels */
#define DFS_MAX_NUM_CHAN                        128

/* Bitmap mask for 80MHz */
#define DFS_80MHZ_MASK                          0x0F

/* Bitmap mask for 40MHz lower */
#define DFS_40MHZ_MASK_L                        0x03

/* Bitmap mask for 40MHz higher */
#define DFS_40MHZ_MASK_H                        0x0C

/* Adjacent weather radar channel */
#define DFS_ADJACENT_WEATHER_RADAR_CHANNEL      5580

/* Max 2.4 GHz channel number */
#define DFS_MAX_24GHZ_CHANNEL                   14

/* Max valid channel number */
#define MAX_CHANNEL_NUM                         184

#define DFS_IS_CHANNEL_WEATHER_RADAR(_f) (((_f) >= 5600) && ((_f) <= 5650))
#define DFS_IS_CHAN_JAPAN_INDOOR(_ch)  (((_ch) >= 36)  && ((_ch) <= 64))
#define DFS_IS_CHAN_JAPAN_OUTDOOR(_ch) (((_ch) >= 100) && ((_ch) <= 140))

/**
 * struct chan_bonding_info - for holding channel bonding bitmap
 * @chan_map: channel map
 * @rsvd: reserved
 * @start_chan: start channel
 */
struct chan_bonding_info {
	uint8_t chan_map:4;
	uint8_t rsvd:4;
	uint8_t start_chan;
};

/**
 * struct chan_bonding_bitmap - bitmap structure which  represent
 * all 5GHZ channels.
 * @chan_bonding_set: channel bonding bitmap
 */
struct chan_bonding_bitmap {
	struct chan_bonding_info chan_bonding_set[DFS_MAX_80MHZ_BANDS];
};

/**
 * dfs_prepare_random_channel() - This function picks a random channel from
 * the list of available channels.
 * @dfs: dfs handler.
 * @ch_list: channel list.
 * @ch_count: Number of channels in given list.
 * @flags: DFS_RANDOM_CH_FLAG_*
 * @ch_wd: input channel width, used same variable to return new ch width.
 * @cur_chan: current channel.
 * @dfs_region: DFS region.
 * @acs_info: acs channel range information.
 *
 * Function used to find random channel selection from a given list.
 * First this function removes channels  based on flags and then uses final
 * list to find channel based on requested bandwidth, if requested bandwidth
 * not available, it chooses next lower bandwidth and try.
 *
 * Return: channel number, else zero.
 */
uint8_t dfs_prepare_random_channel(struct wlan_dfs *dfs,
	struct dfs_ieee80211_channel *ch_list,
	uint32_t ch_count,
	uint32_t flags,
	uint8_t *ch_wd,
	struct dfs_ieee80211_channel *cur_chan,
	uint8_t dfs_region,
	struct dfs_acs_info *acs_info);
