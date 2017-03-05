/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: API for processing radar found indication.
 *
 */

#include "../dfs.h"
#include "../dfs_process_radar_found_ind.h"
#include <wlan_reg_services_api.h>
#include <wlan_dfs_utils_api.h>

/**
 * A) If is chirp or radar found at boundary, two channels will effected.
 *    freq_offset.freq[0] = fn+1
 *    freq_offset.freq[1] = fn+1
 *    freq_offset.freq[2] = fn
 *
 *   Two channels, ch(n) and ch(n+1) will be added to nol.
 *
 *
 *                                            dfs_freq_offset (radar found freq)
 *                                                     |
 *                                                     |
 *                                                     V
 *      _______________________________________________________________________
 *     |       center freq     |       center freq     |       center freq     |
 *     |          ch(n-1)      |          ch(n)        |          ch(n+1)      |
 *     |           |           |           |           |           |           |
 *     |           |           |           |           |           |           |
 *     |           |           |           |           |           |           |
 *                fn-1                    fn         boundary     fn+1
 *     <-------- 20 Mhz ------>
 *
 *
 * B) Else only one channel will be effected
 *    freq_offset.freq[0] = fn
 *    freq_offset.freq[1] = fn
 *    freq_offset.freq[2] = fn
 *
 *   One channel ch(n) will be added to nol.
 *
 *
 *                                            dfs_freq_offset (radar found freq)
 *                                                |
 *                                                |
 *                                                V
 *      _______________________________________________________________________
 *     |       center freq     |       center freq     |       center freq     |
 *     |          ch(n-1)      |          ch(n)        |          ch(n+1)      |
 *     |           |           |           |           |           |           |
 *     |           |           |           |           |           |           |
 *     |           |           |           |           |           |           |
 *                fn-1                    fn         boundary     fn+1
 *     <-------- 20 Mhz ------>
 */

/**
 * dfs_radar_add_to_nol()- add channel to nol list
 * @dfs: dfs handler
 * @freq_offset: freq offset
 *
 * add channel to nol list.
 *
 * Return: None
 */
static void dfs_radar_add_to_nol(struct wlan_dfs *dfs,
		struct freqs_offsets *freq_offset)
{
	int i;
	uint8_t last_chan = 0;
	uint8_t nollist[DFS_NUM_FREQ_OFFSET];
	uint8_t num_ch = 0;

	for (i = 0; i < DFS_NUM_FREQ_OFFSET; i++) {
		if (freq_offset->chan_num[i] == 0 ||
		    freq_offset->chan_num[i] == last_chan)
			continue;
		if (!utils_is_dfs_ch(dfs->dfs_pdev_obj,
		     freq_offset->chan_num[i])) {
			DFS_PRINTK("%s: ch=%d is not dfs skip\n",
				   __func__, freq_offset->chan_num[i]);
			continue;
		}
		last_chan = freq_offset->chan_num[i];
		dfs_nol_addchan(dfs, (uint16_t)freq_offset->freq[i],
				DFS_NOL_TIMEOUT_S);
		nollist[num_ch++] = last_chan;
		DFS_PRINTK("%s: ch=%d Added to NOL\n", __func__, last_chan);
	}
	utils_dfs_reg_update_nol_ch(dfs->dfs_pdev_obj,
			nollist, num_ch, DFS_NOL_SET);
	dfs_save_nol(dfs->dfs_pdev_obj);
}

/**
 * dfs_radar_chan_for_80()- Find frequency offsets for 80MHz
 * @freq_offset: freq offset
 * @center_freq: center frequency
 *
 * Find frequency offsets for 80MHz
 *
 * Return: None
 */
static void dfs_radar_chan_for_80(struct freqs_offsets *freq_offset,
		uint32_t center_freq)
{
	int i;

	for (i = 0; i < DFS_NUM_FREQ_OFFSET; i++) {
		if (freq_offset->offset[i] < DFS_OFFET_SECOND_LOWER)
			freq_offset->freq[i] =
				DFS_THIRD_LOWER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > DFS_OFFET_SECOND_LOWER) &&
			 (freq_offset->offset[i] < DFS_OFFET_FIRST_LOWER))
			freq_offset->freq[i] =
				DFS_SECOND_LOWER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > DFS_OFFET_FIRST_LOWER) &&
			 (freq_offset->offset[i] < 0))
			freq_offset->freq[i] =
				DFS_FIRST_LOWER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > 0) &&
			  (freq_offset->offset[i] < DFS_OFFET_FIRST_UPPER))
			freq_offset->freq[i] =
				DFS_FIRST_UPPER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > DFS_OFFET_FIRST_UPPER) &&
			 (freq_offset->offset[i] < DFS_OFFET_SECOND_UPPER))
			freq_offset->freq[i] =
				DFS_SECOND_UPPER_CHANNEL(center_freq);
		else if (freq_offset->offset[i] > DFS_OFFET_SECOND_UPPER)
			freq_offset->freq[i] =
				DFS_THIRD_UPPER_CHANNEL(center_freq);
	}
}

/**
 * dfs_radar_chan_for_40()- Find frequency offsets for 40MHz
 * @freq_offset: freq offset
 * @center_freq: center frequency
 *
 * Find frequency offsets for 40MHz
 *
 * Return: None
 */
static void dfs_radar_chan_for_40(struct freqs_offsets *freq_offset,
		uint32_t center_freq)
{
	int i;

	for (i = 0; i < DFS_NUM_FREQ_OFFSET; i++) {
		if (freq_offset->offset[i] < DFS_OFFET_FIRST_LOWER)
			freq_offset->freq[i] =
				DFS_SECOND_LOWER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > DFS_OFFET_FIRST_LOWER) &&
			 (freq_offset->offset[i] < 0))
			freq_offset->freq[i] =
				DFS_FIRST_LOWER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > 0) &&
			 (freq_offset->offset[i] < DFS_OFFET_FIRST_UPPER))
			freq_offset->freq[i] =
				DFS_FIRST_UPPER_CHANNEL(center_freq);
		else if (freq_offset->offset[i] > DFS_OFFET_FIRST_UPPER)
			freq_offset->freq[i] =
				DFS_SECOND_UPPER_CHANNEL(center_freq);
	}
}

/**
 * dfs_radar_chan_for_20()- Find frequency offsets for 20MHz
 * @freq_offset: freq offset
 * @center_freq: center frequency
 *
 * Find frequency offsets for 20MHz
 *
 * Return: None
 */
static void dfs_radar_chan_for_20(struct freqs_offsets *freq_offset,
		uint32_t center_freq)
{
	int i;

	for (i = 0; i < DFS_NUM_FREQ_OFFSET; i++) {
		if (freq_offset->offset[i] <= DFS_20MZ_OFFSET_LOWER)
			freq_offset->freq[i] =
				DFS_20MHZ_LOWER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > DFS_20MZ_OFFSET_LOWER) &&
			  (freq_offset->offset[i] < DFS_20MZ_OFFSET_UPPER))
			freq_offset->freq[i] = center_freq;
		else if (freq_offset->offset[i] >= DFS_20MZ_OFFSET_UPPER)
			freq_offset->freq[i] =
				DFS_20MHZ_UPPER_CHANNEL(center_freq);
	}
}

void dfs_process_radar_found_indication(struct wlan_dfs *dfs,
		struct radar_found_info *radar_found)
{
	int i;
	struct freqs_offsets freq_offset;
	uint32_t freq_center, flag;
	int32_t sidx;

	if (!dfs || !dfs->dfs_curchan) {
		DFS_PRINTK("%s: dfs is null\n", __func__);
		return;
	}

	qdf_mem_set(&freq_offset, sizeof(freq_offset), 0);
	flag = dfs->dfs_curchan->ic_flags;

	for (i = 0; i < DFS_NUM_FREQ_OFFSET; i++)
		freq_offset.offset[i] = radar_found->freq_offset;

	sidx = DFS_FREQ_OFFSET_TO_SIDX(radar_found->freq_offset);

	if (!radar_found->segment_id)
		freq_center = utils_dfs_chan_to_freq(dfs->dfs_pdev_obj,
				dfs->dfs_curchan->ic_vhtop_ch_freq_seg1);
	else {
		freq_center = utils_dfs_chan_to_freq(dfs->dfs_pdev_obj,
				dfs->dfs_curchan->ic_vhtop_ch_freq_seg2);
		if (flag & IEEE80211_CHAN_VHT160)
			freq_center += DFS_160MHZ_SECOND_SEG_OFFSET;
	}

	DFS_PRINTK("%s: seg=%d, sidx=%d, offset=%d, chirp=%d, flag=%d, f=%d\n",
		   __func__, radar_found->segment_id, sidx,
		   radar_found->freq_offset, radar_found->is_chirp,
		   flag, freq_center);

	if ((flag & IEEE80211_CHAN_HT20) ||
	    (flag & IEEE80211_CHAN_VHT20)) {
		if (radar_found->is_chirp ||
		   (sidx && !(abs(sidx) % DFS_BOUNDRY_SIDX))) {
			freq_offset.offset[1] -= DFS_CHIRP_OFFSET;
			freq_offset.offset[2] += DFS_CHIRP_OFFSET;
		}
		dfs_radar_chan_for_20(&freq_offset, freq_center);
	} else if ((flag & IEEE80211_CHAN_VHT40PLUS)  ||
		   (flag & IEEE80211_CHAN_HT40PLUS)   ||
		   (flag & IEEE80211_CHAN_VHT40MINUS) ||
		   (flag & IEEE80211_CHAN_HT40MINUS)) {
		if (radar_found->is_chirp || !(abs(sidx) % DFS_BOUNDRY_SIDX)) {
			freq_offset.offset[1] -= DFS_CHIRP_OFFSET;
			freq_offset.offset[2] += DFS_CHIRP_OFFSET;
		}
		dfs_radar_chan_for_40(&freq_offset, freq_center);
	} else if ((flag & IEEE80211_CHAN_VHT80)    ||
		   (flag & IEEE80211_CHAN_VHT80_80) ||
		   (flag & IEEE80211_CHAN_VHT160)) {
		if (radar_found->is_chirp || !(abs(sidx) % DFS_BOUNDRY_SIDX)) {
			freq_offset.offset[1] -= DFS_CHIRP_OFFSET;
			freq_offset.offset[2] += DFS_CHIRP_OFFSET;
		}
		dfs_radar_chan_for_80(&freq_offset, freq_center);
	}

	for (i = 0; i < DFS_NUM_FREQ_OFFSET; i++) {
		freq_offset.chan_num[i] = utils_dfs_freq_to_chan(
				dfs->dfs_pdev_obj, freq_offset.freq[i]);
		DFS_PRINTK("%s: offset=%d, channel%d\n",
			   __func__, i, freq_offset.chan_num[i]);
	}

	dfs_radar_add_to_nol(dfs, &freq_offset);
}
