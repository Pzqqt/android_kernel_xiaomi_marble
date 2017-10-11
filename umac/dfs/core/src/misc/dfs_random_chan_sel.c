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

#include "../dfs.h"
#include "../dfs_random_chan_sel.h"
#include <qdf_mc_timer.h>

/**
 * dfs_populate_80mhz_available_channels()- Populate channels for 80MHz uing bitmap
 * @bitmap: bitmap
 * @avail_chnl: prepared channel list
 *
 * Prepare 80MHz channels from the bitmap.
 *
 * Return: channel count
 */
static uint8_t dfs_populate_80mhz_available_channels(
	struct chan_bonding_bitmap *bitmap,
	uint8_t *avail_chnl)
{
	uint8_t i = 0;
	uint8_t chnl_count = 0;
	uint8_t start_chan = 0;

	for (i = 0; i < DFS_MAX_80MHZ_BANDS; i++) {
		start_chan = bitmap->chan_bonding_set[i].start_chan;
		if (bitmap->chan_bonding_set[i].chan_map ==
			DFS_80MHZ_MASK) {
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 0);
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 1);
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 2);
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 3);
		}
	}
	dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "channel count %d", chnl_count);

	return chnl_count;
}

/**
 * dfs_populate_40mhz_available_channels()- Populate channels for 40MHz uing bitmap
 * @bitmap: bitmap
 * @avail_chnl: prepared channel list
 *
 * Prepare 40MHz channels from the bitmap.
 *
 * Return: channel count
 */
static uint8_t dfs_populate_40mhz_available_channels(
	struct chan_bonding_bitmap *bitmap,
	uint8_t *avail_chnl)
{
	uint8_t i = 0;
	uint8_t chnl_count = 0;
	uint8_t start_chan = 0;

	for (i = 0; i < DFS_MAX_80MHZ_BANDS; i++) {
		start_chan = bitmap->chan_bonding_set[i].start_chan;
		if ((bitmap->chan_bonding_set[i].chan_map &
			DFS_40MHZ_MASK_L) == DFS_40MHZ_MASK_L) {
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 0);
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 1);
		}
		if ((bitmap->chan_bonding_set[i].chan_map &
			DFS_40MHZ_MASK_H) == DFS_40MHZ_MASK_H) {
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 2);
			avail_chnl[chnl_count++] = start_chan +
				(DFS_NEXT_5GHZ_CHANNEL * 3);
		}
	}
	dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "channel count %d", chnl_count);

	return chnl_count;
}

/**
 * dfs_populate_available_channels()- Populate channels based on width and bitmap
 * @bitmap: bitmap
 * @ch_width: channel width
 * @avail_chnl: prepared channel list
 *
 * Prepare channel list based on width and channel bitmap.
 *
 * Return: channel count
 */
static uint8_t dfs_populate_available_channels(
	struct chan_bonding_bitmap *bitmap,
	uint8_t ch_width,
	uint8_t *avail_chnl)
{
	switch (ch_width) {
	case DFS_CH_WIDTH_160MHZ:
	case DFS_CH_WIDTH_80P80MHZ:
	case DFS_CH_WIDTH_80MHZ:
		return dfs_populate_80mhz_available_channels(
			bitmap, avail_chnl);
	case DFS_CH_WIDTH_40MHZ:
		return dfs_populate_40mhz_available_channels(
			bitmap, avail_chnl);
	default:
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
				"Invalid ch_width %d", ch_width);
		break;
	}

	return 0;
}

/**
 * dfs_get_rand_from_lst()- Get random channel from a given channel list
 * @ch_lst: channel list
 * @num_ch: number of channels
 *
 * Get random channel from given channel list.
 *
 * Return: channel number
 */
static uint8_t dfs_get_rand_from_lst(uint8_t *ch_lst, uint8_t num_ch)
{
	uint8_t i;
	uint32_t rand_byte = 0;

	if (!num_ch || !ch_lst) {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
				"invalid param ch_lst %p, num_ch = %d",
				ch_lst, num_ch);
		return 0;
	}

	get_random_bytes((uint8_t *)&rand_byte, 1);
	i = (rand_byte + qdf_mc_timer_get_system_ticks()) % num_ch;

	dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "random channel %d", ch_lst[i]);
	return ch_lst[i];
}

/**
 * dfs_random_channel_sel_set_bitmap()- Set channel bit in bitmap based
 * on given channel number
 * @bitmap: bitmap
 * @channel: channel number
 *
 * Set channel bit in bitmap based on given channel number.
 *
 * Return: None
 */
static void dfs_random_channel_sel_set_bitmap(
	struct chan_bonding_bitmap *bitmap,
	uint8_t channel)
{
	int i = 0;
	int start_chan = 0;

	for (i = 0; i < DFS_MAX_80MHZ_BANDS; i++) {
		start_chan = bitmap->chan_bonding_set[i].start_chan;
		if (channel >= start_chan && channel <= start_chan + 12) {
			bitmap->chan_bonding_set[i].chan_map |=
			(1 << ((channel - start_chan) /
			DFS_80_NUM_SUB_CHANNNEL));
			return;
		}
	}

	dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"Channel=%d is not in the bitmap", channel);
}

/**
 * dfs_find_ch_with_fallback()- find random channel
 * @ch_wd: channel width
 * @center_freq_seg1: center frequency of secondary segment.
 * @ch_lst: list of available channels.
 * @num_ch: number of channels in the list.
 *
 * Find random channel based on given channel width and channel list,
 * fallback to lower width if requested channel width not available.
 *
 * Return: channel number
 */
static uint8_t dfs_find_ch_with_fallback(uint8_t *ch_wd,
	uint8_t *center_freq_seg1,
	uint8_t *ch_lst,
	uint32_t num_ch)
{
	bool flag = false;
	uint32_t rand_byte = 0;
	struct  chan_bonding_bitmap ch_map = { { {0} } };
	uint8_t count = 0, i, index = 0, final_cnt = 0, target_channel = 0;
	uint8_t primary_seg_start_ch = 0, sec_seg_ch = 0, new_160_start_ch = 0;
	uint8_t final_lst[DFS_MAX_NUM_CHAN] = {0};

	/* initialize ch_map for all 80 MHz bands: we have 6 80MHz bands */
	ch_map.chan_bonding_set[0].start_chan = 36;
	ch_map.chan_bonding_set[1].start_chan = 52;
	ch_map.chan_bonding_set[2].start_chan = 100;
	ch_map.chan_bonding_set[3].start_chan = 116;
	ch_map.chan_bonding_set[4].start_chan = 132;
	ch_map.chan_bonding_set[5].start_chan = 149;

	for (i = 0; i < num_ch; i++) {
		dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS,
				"channel = %d added to bitmap", ch_lst[i]);
		dfs_random_channel_sel_set_bitmap(&ch_map, ch_lst[i]);
	}

	/* populate available channel list from bitmap */
	final_cnt = dfs_populate_available_channels(&ch_map, *ch_wd, final_lst);

	/* If no valid ch bonding found, fallback */
	if (final_cnt == 0) {
		if ((*ch_wd == DFS_CH_WIDTH_160MHZ) ||
		    (*ch_wd == DFS_CH_WIDTH_80P80MHZ) ||
		    (*ch_wd == DFS_CH_WIDTH_80MHZ)) {
			dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS,
					"from [%d] to 40Mhz", *ch_wd);
			*ch_wd = DFS_CH_WIDTH_40MHZ;
		} else if (*ch_wd == DFS_CH_WIDTH_40MHZ) {
			dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS, "from 40Mhz to 20MHz");
			*ch_wd = DFS_CH_WIDTH_20MHZ;
		}
		return 0;
	}

	/* ch count should be > 8 to switch new channel in 160Mhz band */
	if (((*ch_wd == DFS_CH_WIDTH_160MHZ) ||
	     (*ch_wd == DFS_CH_WIDTH_80P80MHZ)) &&
	     (final_cnt < DFS_MAX_20M_SUB_CH)) {
		dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS,
				"from [%d] to 80Mhz", *ch_wd);
		*ch_wd = DFS_CH_WIDTH_80MHZ;
		return 0;
	}

	if (*ch_wd == DFS_CH_WIDTH_160MHZ) {
		/*
		 * Only 2 blocks for 160Mhz bandwidth i.e 36-64 & 100-128
		 * and all the channels in these blocks are continuous
		 * and seperated by 4Mhz.
		 */
		for (i = 1; ((i < final_cnt)); i++) {
			if ((final_lst[i] - final_lst[i-1]) ==
			     DFS_NEXT_5GHZ_CHANNEL)
				count++;
			else
				count = 0;
			if (count == DFS_MAX_20M_SUB_CH - 1) {
				flag = true;
				new_160_start_ch = final_lst[i - count];
				break;
			}
		}
	} else if (*ch_wd == DFS_CH_WIDTH_80P80MHZ) {
		flag = true;
	}

	if ((flag == false) && (*ch_wd > DFS_CH_WIDTH_80MHZ)) {
		dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS,
				"from [%d] to 80Mhz", *ch_wd);
		*ch_wd = DFS_CH_WIDTH_80MHZ;
		return 0;
	}

	if (*ch_wd == DFS_CH_WIDTH_160MHZ) {
		get_random_bytes((uint8_t *)&rand_byte, 1);
		rand_byte = (rand_byte + qdf_mc_timer_get_system_ticks())
			% DFS_MAX_20M_SUB_CH;
		target_channel = new_160_start_ch + (rand_byte *
				DFS_80_NUM_SUB_CHANNNEL);
	} else if (*ch_wd == DFS_CH_WIDTH_80P80MHZ) {
		get_random_bytes((uint8_t *)&rand_byte, 1);
		index = (rand_byte + qdf_mc_timer_get_system_ticks()) %
			final_cnt;
		target_channel = final_lst[index];
		index -= (index % DFS_80_NUM_SUB_CHANNNEL);
		primary_seg_start_ch = final_lst[index];

		/* reset channels associate with primary 80Mhz */
		for (i = 0; i < DFS_80_NUM_SUB_CHANNNEL; i++)
			final_lst[i + index] = 0;
		/* select and calculate center freq for secondary segement */
		for (i = 0; i < final_cnt / DFS_80_NUM_SUB_CHANNNEL; i++) {
			if (final_lst[i * DFS_80_NUM_SUB_CHANNNEL] &&
			    (abs(primary_seg_start_ch -
			     final_lst[i * DFS_80_NUM_SUB_CHANNNEL]) >
			     (DFS_MAX_20M_SUB_CH * 2))) {
				sec_seg_ch =
					final_lst[i * DFS_80_NUM_SUB_CHANNNEL] +
					DFS_80MHZ_START_CENTER_CH_DIFF;
				break;
			}
		}

		if (!sec_seg_ch && (final_cnt == DFS_MAX_20M_SUB_CH))
			*ch_wd = DFS_CH_WIDTH_160MHZ;
		else if (!sec_seg_ch)
			*ch_wd = DFS_CH_WIDTH_80MHZ;

		*center_freq_seg1 = sec_seg_ch;
		dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS,
				"Center frequency seg1 = %d", sec_seg_ch);
	} else {
		target_channel = dfs_get_rand_from_lst(final_lst, final_cnt);
	}
	dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"target channel = %d", target_channel);

	return target_channel;
}

/**
 * dfs_remove_cur_ch_from_list()- remove current operating channels
 * @ch_list: list of avilable channel list
 * @ch_cnt: number of channels.
 * @ch_wd: channel width.
 * @cur_chan: current channel.
 *
 * Remove current channels from list of available channels.
 *
 * Return: channel number
 */
static void dfs_remove_cur_ch_from_list(
	struct dfs_ieee80211_channel *ch_list,
	uint32_t *ch_cnt,
	uint8_t *ch_wd,
	struct dfs_ieee80211_channel *cur_chan)
{
	/* TODO */
	return;
}

/**
 * dfs_freq_is_in_nol()- check if given channel in nol list
 * @dfs: dfs handler
 * @freq: channel frequency.
 *
 * check if given channel in nol list.
 *
 * Return: true if channel in nol, false else
 */
static bool dfs_freq_is_in_nol(struct wlan_dfs *dfs, uint32_t freq)
{
	struct dfs_nolelem *nol;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "null dfs");
		return false;
	}

	nol = dfs->dfs_nol;
	while (nol) {
		if (freq == nol->nol_freq) {
			dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
					"%d is in nol", freq);
			return true;
		}
		nol = nol->nol_next;
	}

	return false;
}

/**
 * dfs_apply_rules()- prepare channel list based on flags
 * @dfs: dfs handler
 * @flags: channel flags
 * @random_chan_list: ouput channel list
 * @random_chan_cnt: output channel count
 * @ch_list: input channel list
 * @ch_cnt: input channel count
 * @dfs_region: dfs region
 * @acs_info: acs channel range information
 *
 * prepare channel list based on flags
 *
 * Return: None
 */
static void dfs_apply_rules(struct wlan_dfs *dfs,
	uint32_t flags,
	uint8_t *random_chan_list,
	uint32_t *random_chan_cnt,
	struct dfs_ieee80211_channel *ch_list,
	uint32_t ch_cnt,
	uint8_t dfs_region,
	struct dfs_acs_info *acs_info)
{
	struct dfs_ieee80211_channel *chan;
	uint16_t flag_no_wheather = 0;
	uint16_t flag_no_lower_5g = 0;
	uint16_t flag_no_upper_5g = 0;
	uint16_t flag_no_dfs_chan = 0;
	uint16_t flag_no_2g_chan  = 0;
	uint16_t flag_no_5g_chan  = 0;
	int i;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "flags %d", flags);
	flag_no_wheather = (dfs_region == DFS_ETSI_REGION_VAL) ?
		flags & DFS_RANDOM_CH_FLAG_NO_WEATHER_CH : 0;

	flag_no_lower_5g = (dfs_region == DFS_MKK_REGION_VAL) ?
		flags & DFS_RANDOM_CH_FLAG_NO_LOWER_5G_CH : 0;

	flag_no_upper_5g = (dfs_region == DFS_MKK_REGION_VAL) ?
		flags & DFS_RANDOM_CH_FLAG_NO_UPEER_5G_CH : 0;

	flag_no_dfs_chan = flags & DFS_RANDOM_CH_FLAG_NO_DFS_CH;
	flag_no_2g_chan  = flags & DFS_RANDOM_CH_FLAG_NO_2GHZ_CH;
	flag_no_5g_chan  = flags & DFS_RANDOM_CH_FLAG_NO_5GHZ_CH;

	for (i = 0; i < ch_cnt; i++) {
		chan = &ch_list[i];

		if ((chan->dfs_ch_ieee == 0) ||
				(chan->dfs_ch_ieee > MAX_CHANNEL_NUM)) {
			dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "invalid channel %d",
					chan->dfs_ch_ieee);
			continue;
		}

		if (acs_info && (acs_info->acs_mode == 1) &&
		    ((chan->dfs_ch_ieee < acs_info->start_ch) ||
		    (chan->dfs_ch_ieee > acs_info->end_ch))) {
			dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
					"skip ch %d not in acs range (%d-%d)",
				    chan->dfs_ch_ieee, acs_info->start_ch,
				   acs_info->end_ch);
			continue;

		}

		if (flag_no_2g_chan &&
				chan->dfs_ch_ieee <= DFS_MAX_24GHZ_CHANNEL) {
			dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
					"skip 2.4 GHz channel=%d",
				    chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_5g_chan &&
				chan->dfs_ch_ieee > DFS_MAX_24GHZ_CHANNEL) {
			dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
					"skip 5 GHz channel=%d",
				    chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_wheather) {
			/*
			 * We should also avoid this channel in HT40 mode as
			 * extension channel will be on 5600.
			 */
			/* TODO check if reg updating chan->dfs_ch_flags for
			 * IEEE80211_CHAN_11NA_HT40PLUS
			 */
			if (DFS_IS_CHANNEL_WEATHER_RADAR(chan->dfs_ch_freq)) {
				dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
						"skip weather channel=%d",
						chan->dfs_ch_ieee);
				continue;
			} else if (DFS_ADJACENT_WEATHER_RADAR_CHANNEL ==
				   chan->dfs_ch_freq && (chan->dfs_ch_flags &
				   IEEE80211_CHAN_11NA_HT40PLUS)) {
				dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
						"skip weather adjacent ch=%d",
					    chan->dfs_ch_ieee);
				continue;
			}
		}

		if (flag_no_lower_5g &&
		    DFS_IS_CHAN_JAPAN_INDOOR(chan->dfs_ch_freq)) {
			dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
					"skip indoor channel=%d",
					chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_upper_5g &&
		    DFS_IS_CHAN_JAPAN_OUTDOOR(chan->dfs_ch_freq)) {
			dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "skip outdoor channel=%d",
				    chan->dfs_ch_ieee);
			continue;
		}

		if (flag_no_dfs_chan &&
		    (chan->dfs_ch_flagext & IEEE80211_CHAN_DFS)) {
			dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "skip dfs channel=%d",
				    chan->dfs_ch_ieee);
			continue;
		}

		if (dfs_freq_is_in_nol(dfs, chan->dfs_ch_freq)) {
			dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "skip nol channel=%d",
				    chan->dfs_ch_ieee);
			continue;
		}

		random_chan_list[*random_chan_cnt] = chan->dfs_ch_ieee;
		*random_chan_cnt += 1;
	}
}

uint8_t dfs_prepare_random_channel(struct wlan_dfs *dfs,
	struct dfs_ieee80211_channel *ch_list,
	uint32_t ch_cnt,
	uint32_t flags,
	uint8_t *ch_wd,
	struct dfs_ieee80211_channel *cur_chan,
	uint8_t dfs_region,
	struct dfs_acs_info *acs_info)
{
	uint8_t target_ch = 0;
	uint8_t *random_chan_list = NULL;
	uint32_t random_chan_cnt = 0;

	if (!ch_list || !ch_cnt) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"Invalid params %p, ch_cnt=%d",
				ch_list, ch_cnt);
		return 0;
	}

	if (*ch_wd < DFS_CH_WIDTH_20MHZ || *ch_wd > DFS_CH_WIDTH_80P80MHZ) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"Invalid ch_wd %d", *ch_wd);
		return 0;
	}

	random_chan_list = qdf_mem_malloc(ch_cnt * sizeof(*random_chan_list));
	if (!random_chan_list) {
		dfs_alert(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"Memory allocation failed");
		return 0;
	}

	if (flags & DFS_RANDOM_CH_FLAG_NO_CURR_OPE_CH)
		dfs_remove_cur_ch_from_list(ch_list, &ch_cnt, ch_wd, cur_chan);

	dfs_apply_rules(dfs, flags, random_chan_list, &random_chan_cnt,
		    ch_list, ch_cnt, dfs_region, acs_info);

	do {
		if (*ch_wd == DFS_CH_WIDTH_20MHZ) {
			target_ch = dfs_get_rand_from_lst(
				random_chan_list, random_chan_cnt);
			break;
		}

		target_ch = dfs_find_ch_with_fallback(ch_wd,
				&cur_chan->dfs_ch_vhtop_ch_freq_seg2,
				random_chan_list,
				random_chan_cnt);
		if (target_ch)
			break;
	} while (true);

	qdf_mem_free(random_chan_list);
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "target_ch = %d", target_ch);

	return target_ch;
}
