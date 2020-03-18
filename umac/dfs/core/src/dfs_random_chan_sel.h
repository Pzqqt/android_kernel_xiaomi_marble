/*
 * Copyright (c) 2012-2020 The Linux Foundation. All rights reserved.
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

/* Next 5GHz channel number */
#define DFS_80_NUM_SUB_CHANNEL                 4

/* Next 5GHz channel freq offset */
#define DFS_80_NUM_SUB_CHANNEL_FREQ            20

/* Next 5GHz channel number */
#define DFS_NEXT_5GHZ_CHANNEL                   4

/* Next 5GHz channel number */
#define DFS_NEXT_5GHZ_CHANNEL_FREQ_OFFSET       20

/* Number of 20MHz channels in bitmap */
#define DFS_MAX_20M_SUB_CH                      8

/* Frequency difference between 80+80 MHz */
#define DFS_80P80M_FREQ_DIFF                    40

/* Number of 80MHz channels in 5GHz band */
#define DFS_MAX_80MHZ_BANDS                     6

/* Start channel and center channel diff in 80Mhz */
#define DFS_80MHZ_START_CENTER_CH_DIFF          6

/* Start channel and center channel freq diff in 80Mhz */
#define DFS_80MHZ_START_CENTER_CH_FREQ_DIFF     30

/* Bitmap mask for 80MHz */
#define DFS_80MHZ_MASK                          0x0F

/* Bitmap mask for 40MHz lower */
#define DFS_40MHZ_MASK_L                        0x03

/* Bitmap mask for 40MHz higher */
#define DFS_40MHZ_MASK_H                        0x0C

/* Adjacent weather radar channel frequency */
#define DFS_ADJACENT_WEATHER_RADAR_CHANNEL      5580

/* Adjacent weather radar channel number */
#define DFS_ADJACENT_WEATHER_RADAR_CHANNEL_NUM  116

/* Max 2.4 GHz channel number */
#define DFS_MAX_24GHZ_CHANNEL                   14

/* Max 2.4 GHz channel frequency */
#define DFS_MAX_24GHZ_CHANNEL_FREQ              2484

/* Adjacent weather radar channel frequency */
#define DFS_ADJACENT_WEATHER_RADAR_CHANNEL_FREQ  5580
/* Max valid channel number */
#define MAX_CHANNEL_NUM                         184

#ifdef WLAN_ENABLE_CHNL_MATRIX_RESTRICTION
#define DFS_TX_LEAKAGE_THRES 310
#define DFS_TX_LEAKAGE_MAX  1000
#define DFS_TX_LEAKAGE_MIN  200

/*
 * This define is used to block additional channels
 * based on the new data gathered on auto platforms
 * and to differentiate the leakage data among different
 * platforms.
 */

#define DFS_TX_LEAKAGE_AUTO_MIN  210
#endif

#define DFS_IS_CHANNEL_WEATHER_RADAR(_f) (((_f) >= 5600) && ((_f) <= 5650))
#ifdef CONFIG_CHAN_NUM_API
#define DFS_IS_CHAN_JAPAN_INDOOR(_ch)    (((_ch) >= 36)  && ((_ch) <= 64))
#define DFS_IS_CHAN_JAPAN_W53(_ch)       (((_ch) >= 52)  && ((_ch) <= 64))
#define DFS_IS_CHAN_JAPAN_OUTDOOR(_ch)   (((_ch) >= 100) && ((_ch) <= 140))
#endif

#ifdef CONFIG_CHAN_FREQ_API
#define DFS_IS_CHAN_JAPAN_INDOOR_FREQ(_ch)(((_ch) >= 5180)  && ((_ch) <= 5320))
#define DFS_IS_CHAN_JAPAN_OUTDOOR_FREQ(_ch)(((_ch) >= 5500) && ((_ch) <= 5700))
#define DFS_IS_CHAN_JAPAN_W53_FREQ(_ch)    (((_ch) >= 5260)  && ((_ch) <= 5320))
#endif

/**
 * struct chan_bonding_info - for holding channel bonding bitmap
 * @chan_map: channel map
 * @rsvd: reserved
 * @start_chan: start channel
 * @start_chan_freq: start channel frequency in MHZ.
 */
struct chan_bonding_info {
	uint8_t chan_map:4;
	uint8_t rsvd:4;
	uint8_t start_chan;
	uint16_t start_chan_freq;
};

/**
 * struct chan_bonding_bitmap - bitmap structure which  represent
 * all 5GHZ channels.
 * @chan_bonding_set: channel bonding bitmap
 */
struct chan_bonding_bitmap {
	struct chan_bonding_info chan_bonding_set[DFS_MAX_80MHZ_BANDS];
};

#ifdef WLAN_ENABLE_CHNL_MATRIX_RESTRICTION
/**
 * struct dfs_tx_leak_info - DFS leakage info
 * @leak_chan: leak channel.
 * @leak_lvl: tx leakage lvl.
 */
struct dfs_tx_leak_info {
	uint8_t leak_chan;
	uint16_t leak_chan_freq;
	uint32_t leak_lvl;
};

/**
 * struct dfs_matrix_tx_leak_info - DFS leakage matrix info for dfs channel.
 * @channel: channel to switch from
 * @chan_matrix DFS leakage matrix info for given dfs channel.
 */
struct dfs_matrix_tx_leak_info {
	uint8_t channel;
	uint16_t channel_freq;
	struct dfs_tx_leak_info chan_matrix[CHAN_ENUM_5720 -
					    CHAN_ENUM_5180 + 1];
};
#endif

/**
 * dfs_mark_leaking_ch() - to mark channel leaking in to nol
 * @dfs: dfs handler.
 * @ch_width: channel width
 * @temp_ch_lst_sz: the target channel list
 * @temp_ch_lst: the target channel list
 *
 * This function removes the channels from temp channel list that
 * (if selected as target channel) will cause leakage in one of
 * the NOL channels
 *
 * Return: QDF_STATUS
 */
#ifdef CONFIG_CHAN_NUM_API
QDF_STATUS dfs_mark_leaking_ch(struct wlan_dfs *dfs,
		enum phy_ch_width ch_width,
		uint8_t temp_ch_lst_sz,
		uint8_t *temp_ch_lst);
#endif

/**
 * dfs_mark_leaking_chan_for_freq() - to mark channel leaking in to nol
 * @dfs: dfs handler.
 * @ch_width: channel width
 * @temp_chan_lst_sz: the target channel list size.
 * @temp_freq_lst: the target frequency channel list
 *
 * This function removes the channels from temp channel list that
 * (if selected as target channel) will cause leakage in one of
 * the NOL channels
 *
 * Return: QDF_STATUS
 */
#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS dfs_mark_leaking_chan_for_freq(struct wlan_dfs *dfs,
					enum phy_ch_width ch_width,
					uint8_t temp_chan_lst_sz,
					uint16_t *temp_freq_lst);
#endif

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
#ifdef CONFIG_CHAN_NUM_API
uint8_t dfs_prepare_random_channel(struct wlan_dfs *dfs,
	struct dfs_channel *ch_list,
	uint32_t ch_count,
	uint32_t flags,
	uint8_t *ch_wd,
	struct dfs_channel *cur_chan,
	uint8_t dfs_region,
	struct dfs_acs_info *acs_info);
#endif

/**
 * dfs_prepare_random_channel() - This function picks a random channel from
 * the list of available channels.
 * @dfs: dfs handler.
 * @chan_list: channel list.
 * @ch_count: Number of channels in given list.
 * @flags: DFS_RANDOM_CH_FLAG_*
 * @chan_wd: input channel width, used same variable to return new ch width.
 * @cur_chan: current channel.
 * @dfs_region: DFS region.
 * @acs_info: acs channel range information.
 *
 * Function used to find random channel selection from a given list.
 * First this function removes channels  based on flags and then uses final
 * list to find channel based on requested bandwidth, if requested bandwidth
 * not available, it chooses next lower bandwidth and try.
 *
 * Return: channel frequency, else zero.
 */
#ifdef CONFIG_CHAN_FREQ_API
uint16_t dfs_prepare_random_channel_for_freq(struct wlan_dfs *dfs,
					     struct dfs_channel *ch_list,
					     uint32_t chan_count,
					     uint32_t flags,
					     uint8_t *chan_wd,
					     struct dfs_channel *cur_chan,
					     uint8_t dfs_region,
					     struct dfs_acs_info *acs_info);
#endif
