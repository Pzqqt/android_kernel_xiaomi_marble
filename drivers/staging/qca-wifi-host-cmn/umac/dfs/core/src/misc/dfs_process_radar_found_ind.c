/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include "../dfs_zero_cac.h"
#include "../dfs_process_radar_found_ind.h"
#include <wlan_reg_services_api.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_dfs_utils_api.h>
#include <wlan_dfs_tgt_api.h>
#include "wlan_dfs_mlme_api.h"
#include "../dfs_internal.h"
/**
 * TODO: The code is not according to the following description needs
 * modification and correction. Code always adds left and right channels to
 * NOL even if it is not a chirp radar.
 *
 * A) If chirp radar starts at boundary and ends at boundary then three channels
 *    will be affected.
 *    freq_offset.freq[0] = fn   (Center frequency)
 *    freq_offset.freq[1] = fn-1 (Left of center)
 *    freq_offset.freq[2] = fn+1 (Right of center)
 *
 *    Three channels, ch(n-1), ch(n)and ch(n+1) will be added to NOL.
 *
 *                     Chirp start freq         Chirp end freq
 *                             |                       |
 *                             |                       |
 *                             V                       V
 *      _______________________________________________________________________
 *     |       center freq     |       center freq     |       center freq     |
 *     |          ch(n-1)      |          ch(n)        |          ch(n+1)      |
 *     |           |           |           |           |           |           |
 *     |           |           |           |           |           |           |
 *     |           |           |           |           |           |           |
 *                fn-1                    fn         boundary     fn+1
 *     <-------- 20 Mhz ------>
 *
 * B) If chirp radar starts at one channel and continues up to another channel
 *    then two channels will be affected.
 *    freq_offset.freq[0] = fn
 *    freq_offset.freq[1] = 0
 *    freq_offset.freq[2] = fn+1
 *
 *    Three channels, ch(n-1), ch(n)and ch(n+1) will be added to NOL.
 *
 *                                   Chirp start freq         Chirp end freq
 *                                           |                       |
 *                                           |                       |
 *                                           V                       V
 *      _______________________________________________________________________
 *     |       center freq     |       center freq     |       center freq     |
 *     |          ch(n-1)      |          ch(n)        |          ch(n+1)      |
 *     |           |           |           |           |           |           |
 *     |           |           |           |           |           |           |
 *     |           |           |           |           |           |           |
 *                fn-1                    fn         boundary     fn+1
 *     <-------- 20 Mhz ------>
 *
 * C) Radar found at boundary, two channels will be affected.
 *    freq_offset.freq[0] = fn
 *    freq_offset.freq[1] = 0
 *    freq_offset.freq[2] = fn+1
 *
 *    Two channels, ch(n) and ch(n+1) will be added to NOL.
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
 * D) Else only one channel will be affected.
 *    freq_offset.freq[0] = fn
 *    freq_offset.freq[1] = 0
 *    freq_offset.freq[2] = 0
 *
 *   One channel ch(n) will be added to NOL.
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

int dfs_set_nol_subchannel_marking(struct wlan_dfs *dfs,
				   bool nol_subchannel_marking)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!dfs)
		return -EIO;

	dfs->dfs_use_nol_subchannel_marking = nol_subchannel_marking;
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "NOL subchannel marking is %s ",
		 (nol_subchannel_marking) ? "set" : "disabled");
	if (dfs->dfs_is_offload_enabled)
		status = tgt_dfs_send_subchan_marking(dfs->dfs_pdev_obj,
						      nol_subchannel_marking);

	return qdf_status_to_os_return(status);
}

int dfs_get_nol_subchannel_marking(struct wlan_dfs *dfs,
				   bool *nol_subchannel_marking)
{
	if (!dfs)
		return -EIO;

	(*nol_subchannel_marking) = dfs->dfs_use_nol_subchannel_marking;

	return 0;
}

#ifdef CONFIG_CHAN_FREQ_API
QDF_STATUS
dfs_radar_add_channel_list_to_nol_for_freq(struct wlan_dfs *dfs,
					   uint16_t *freq_list,
					   uint16_t *nol_freq_list,
					   uint8_t *num_channels)
{
	int i;
	uint16_t last_chan_freq = 0;
	uint8_t num_ch = 0;

	if (*num_channels > MAX_20MHZ_SUBCHANS) {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"Invalid num channels: %d", *num_channels);
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < *num_channels; i++) {
		if (freq_list[i] == 0 ||
		    freq_list[i] == last_chan_freq)
			continue;
		if (!utils_is_dfs_chan_for_freq(dfs->dfs_pdev_obj,
						freq_list[i])) {
			dfs_info(dfs, WLAN_DEBUG_DFS, "ch=%d is not dfs, skip",
				 freq_list[i]);
			continue;
		}

		last_chan_freq = freq_list[i];
		DFS_NOL_ADD_CHAN_LOCKED(dfs,
					freq_list[i],
					dfs->wlan_dfs_nol_timeout);
		nol_freq_list[num_ch++] = last_chan_freq;
		utils_dfs_deliver_event(dfs->dfs_pdev_obj,
					freq_list[i],
					WLAN_EV_NOL_STARTED);
		dfs_info(dfs, WLAN_DEBUG_DFS_NOL, "ch=%d Added to NOL",
			 last_chan_freq);
	}

	if (!num_ch) {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"dfs channels not found in channel list");
		return QDF_STATUS_E_FAILURE;
	}
	*num_channels = num_ch;

	if (!dfs_get_disable_radar_marking(dfs)) {
		utils_dfs_reg_update_nol_chan_for_freq(dfs->dfs_pdev_obj,
						       nol_freq_list, num_ch,
						       DFS_NOL_SET);

		if (dfs->dfs_is_stadfs_enabled)
			if (dfs_mlme_is_opmode_sta(dfs->dfs_pdev_obj))
				utils_dfs_reg_update_nol_history_chan_for_freq(
					dfs->dfs_pdev_obj, nol_freq_list,
					num_ch, DFS_NOL_HISTORY_SET);
	}

	dfs_nol_update(dfs);
	utils_dfs_save_nol(dfs->dfs_pdev_obj);

	return QDF_STATUS_SUCCESS;
}
#endif
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
		if (freq_offset->offset[i] < DFS_OFFSET_SECOND_LOWER)
			freq_offset->freq[i] =
				DFS_THIRD_LOWER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > DFS_OFFSET_SECOND_LOWER) &&
			 (freq_offset->offset[i] < DFS_OFFSET_FIRST_LOWER))
			freq_offset->freq[i] =
				DFS_SECOND_LOWER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > DFS_OFFSET_FIRST_LOWER) &&
			 (freq_offset->offset[i] < 0))
			freq_offset->freq[i] =
				DFS_FIRST_LOWER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > 0) &&
			  (freq_offset->offset[i] < DFS_OFFSET_FIRST_UPPER))
			freq_offset->freq[i] =
				DFS_FIRST_UPPER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > DFS_OFFSET_FIRST_UPPER) &&
			 (freq_offset->offset[i] < DFS_OFFSET_SECOND_UPPER))
			freq_offset->freq[i] =
				DFS_SECOND_UPPER_CHANNEL(center_freq);
		else if (freq_offset->offset[i] > DFS_OFFSET_SECOND_UPPER)
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
		if (freq_offset->offset[i] < DFS_OFFSET_FIRST_LOWER)
			freq_offset->freq[i] =
				DFS_SECOND_LOWER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > DFS_OFFSET_FIRST_LOWER) &&
			 (freq_offset->offset[i] < 0))
			freq_offset->freq[i] =
				DFS_FIRST_LOWER_CHANNEL(center_freq);
		else if ((freq_offset->offset[i] > 0) &&
			 (freq_offset->offset[i] < DFS_OFFSET_FIRST_UPPER))
			freq_offset->freq[i] =
				DFS_FIRST_UPPER_CHANNEL(center_freq);
		else if (freq_offset->offset[i] > DFS_OFFSET_FIRST_UPPER)
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

#ifdef CONFIG_CHAN_FREQ_API
void
dfs_compute_radar_found_cfreq(struct wlan_dfs *dfs,
			      struct radar_found_info *radar_found,
			      uint32_t *freq_center)
{
	struct dfs_channel *curchan = dfs->dfs_curchan;

	/* In case of 11BE Chipsets, radar found center frequency is
	 * directly obtained from WMI.
	 */
	if (dfs->dfs_is_radar_found_chan_freq_eq_center_freq) {
		*freq_center = radar_found->chan_freq;
		return;
	}

	/* Radar found on agile detector ID.
	 * Applicable to chips that have a separate agile radar detector
	 * engine.
	 */
	if (radar_found->detector_id == dfs_get_agile_detector_id(dfs)) {
		*freq_center = dfs->dfs_agile_precac_freq_mhz;
		if (dfs->dfs_precac_chwidth == CH_WIDTH_160MHZ) {
			if (radar_found->segment_id == PRIMARY_SEG)
				*freq_center -= DFS_160MHZ_SECOND_SEG_OFFSET;
			else
				*freq_center += DFS_160MHZ_SECOND_SEG_OFFSET;
		} else if (dfs->dfs_precac_chwidth == CH_WIDTH_80P80MHZ &&
			   dfs->dfs_agile_precac_freq_mhz ==
				RESTRICTED_80P80_CHAN_CENTER_FREQ) {
			/*
			 * The reason why left and right offsets
			 * are different.
			 * Center of 165 is 5730MHz.
			 * Center of left 80 is 5690MHz.
			 * Center of right 80 is 5775MHz.
			 */
			if (radar_found->segment_id == PRIMARY_SEG)
				*freq_center -=
				    DFS_165MHZ_SECOND_SEG_OFFSET_LEFT;
			else
				*freq_center +=
				    DFS_165MHZ_SECOND_SEG_OFFSET_RIGHT;
		}
	} else if (!radar_found->segment_id) {
		*freq_center = curchan->dfs_ch_mhz_freq_seg1;
	} else {
	    /* Radar found on secondary segment by the HW when
	     * preCAC was running. It (dfs_precac_enable) is specific to
	     * legacy chips.
	     */
		if (dfs_is_precac_timer_running(dfs) &&
			dfs_is_legacy_precac_enabled(dfs)) {
			*freq_center = dfs->dfs_precac_secondary_freq_mhz;
		} else {
		    /* Radar found on secondary segment by the HW, when preCAC
		     * was not running in legacy chips or preCAC was running
		     * in Lithium chips.
		     */
			*freq_center = curchan->dfs_ch_mhz_freq_seg2;
			if (WLAN_IS_CHAN_MODE_160(curchan)) {
				/* If center frequency of entire 160 band
				 * is less than center frequency of primary
				 * segment, then the center frequency of
				 * secondary segment is -40 of center
				 * frequency of entire 160 segment.
				 */
				if (curchan->dfs_ch_mhz_freq_seg2 <
				    curchan->dfs_ch_mhz_freq_seg1)
					*freq_center -=
						DFS_160MHZ_SECOND_SEG_OFFSET;
				else
					*freq_center +=
						DFS_160MHZ_SECOND_SEG_OFFSET;
			}
		}
	}
}
#endif

/**
 * dfs_find_radar_affected_subchans_for_freq() - Find radar affected sub chans.
 * @dfs: Pointer to wlan_dfs structure.
 * @radar_found: Pointer to radar_found structure.
 * @freq_list: Pointer to save radar affected channels.
 * @freq_center: Freq_center of the radar affected chan.
 *
 * Return: Number of channels.
 */
#ifdef CONFIG_CHAN_FREQ_API
static uint8_t
dfs_find_radar_affected_subchans_for_freq(struct wlan_dfs *dfs,
					  struct radar_found_info *radar_found,
					  uint16_t *freq_list,
					  uint32_t freq_center)
{
	int i, j;
	uint8_t num_radar_subchans;
	uint32_t flag;
	int32_t sidx;
	uint16_t candidate_subchan_freq;
	uint16_t cur_subchans[MAX_20MHZ_SUBCHANS];
	uint8_t n_cur_subchans;
	struct dfs_channel *curchan = dfs->dfs_curchan;
	struct freqs_offsets freq_offset;

	qdf_mem_zero(&freq_offset, sizeof(freq_offset));
	flag = curchan->dfs_ch_flags;

	for (i = 0; i < DFS_NUM_FREQ_OFFSET; i++)
		freq_offset.offset[i] = radar_found->freq_offset;

	sidx = DFS_FREQ_OFFSET_TO_SIDX(radar_found->freq_offset);

	dfs_info(dfs, WLAN_DEBUG_DFS,
		 "seg=%d, det=%d, sidx=%d, offset=%d, chirp=%d, flag=%d, f=%d",
		 radar_found->segment_id, radar_found->detector_id, sidx,
		 radar_found->freq_offset, radar_found->is_chirp,
		 flag, freq_center);

	if ((WLAN_IS_CHAN_A(curchan)) ||
	    WLAN_IS_CHAN_MODE_20(curchan)) {
		if (radar_found->is_chirp ||
		    (sidx && !(abs(sidx) % DFS_BOUNDARY_SIDX))) {
			freq_offset.offset[LEFT_CH] -= DFS_CHIRP_OFFSET;
			freq_offset.offset[RIGHT_CH] += DFS_CHIRP_OFFSET;
		}
		dfs_radar_chan_for_20(&freq_offset, freq_center);
	} else if (WLAN_IS_CHAN_MODE_40(curchan)) {
		if (radar_found->is_chirp || !(abs(sidx) % DFS_BOUNDARY_SIDX)) {
			freq_offset.offset[LEFT_CH] -= DFS_CHIRP_OFFSET;
			freq_offset.offset[RIGHT_CH] += DFS_CHIRP_OFFSET;
		}
		dfs_radar_chan_for_40(&freq_offset, freq_center);
	} else if (WLAN_IS_CHAN_MODE_80(curchan) ||
			WLAN_IS_CHAN_MODE_160(curchan) ||
			WLAN_IS_CHAN_MODE_80_80(curchan) ||
			WLAN_IS_CHAN_MODE_320(curchan)) {
		if (radar_found->is_chirp || !(abs(sidx) % DFS_BOUNDARY_SIDX)) {
			freq_offset.offset[LEFT_CH] -= DFS_CHIRP_OFFSET;
			freq_offset.offset[RIGHT_CH] += DFS_CHIRP_OFFSET;
		}
		dfs_radar_chan_for_80(&freq_offset, freq_center);
	} else {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"channel flag=%d is invalid", flag);
		return 0;
	}

	n_cur_subchans =
	    dfs_get_bonding_channels_for_freq(dfs, curchan,
					      radar_found->segment_id,
					      radar_found->detector_id,
					      cur_subchans);

	for (i = 0, num_radar_subchans = 0; i < DFS_NUM_FREQ_OFFSET; i++) {
		candidate_subchan_freq = freq_offset.freq[i];
		for (j = 0; j < n_cur_subchans; j++) {
			if (cur_subchans[j] == candidate_subchan_freq) {
				freq_list[num_radar_subchans++] =
						candidate_subchan_freq;
				dfs_info(dfs, WLAN_DEBUG_DFS,
					 "offset=%d, channel=%d",
					 num_radar_subchans,
					 freq_list[num_radar_subchans - 1]);
				break;
			}
		}
	}
	return num_radar_subchans;
}
#endif

/**
 * dfs_calc_bonding_freqs: Calculate bonding channel frequencies from the
 * channel width's center frequency and channel width.
 * It is assumed that the caller has allocated sufficient memory for 'freq_list'
 * so that it can hold all the output subchannels.
 *
 * center_freq: Center frequency of the channel width.
 * ch_width: Channel width.
 * freq_list: output array of sub-channel frequencies.
 *
 * Return: void
 */
static void
dfs_calc_bonding_freqs(qdf_freq_t center_freq,
		       uint16_t ch_width,
		       uint16_t *freq_list)
{
#define CHAN_SPACING_MHZ_5G 20
#define SUB20CHAN_BW_MHZ_5G 20
	uint8_t nchans = ch_width / CHAN_SPACING_MHZ_5G;
	qdf_freq_t first_subchan_cfreq = center_freq - (ch_width / 2) +
					 (SUB20CHAN_BW_MHZ_5G / 2);
	uint8_t i;

	for (i = 0; i < nchans; ++i)
		freq_list[i] = first_subchan_cfreq + (i * CHAN_SPACING_MHZ_5G);
}

/**
 * dfs_get_20mhz_bonding_channels() - Get bonding frequency list of 20MHz
 * channel.
 * @center_freq: Center frequency of the 20MHz channel.
 * @freq_list: Pointer to frequency list.
 *
 * Return: void
 */
static
void dfs_get_20mhz_bonding_channels(uint16_t center_freq, uint16_t *freq_list)
{
	uint16_t chwidth = BW_20;

	dfs_calc_bonding_freqs(center_freq, chwidth, freq_list);
}

/**
 * dfs_get_40mhz_bonding_channels() - Get bonding frequency list of 40MHz
 * channel.
 * @center_freq: Center frequency of the 40MHz channel.
 * @freq_list: Pointer to frequency list.
 *
 * Return: void
 */
static
void dfs_get_40mhz_bonding_channels(uint16_t center_freq, uint16_t *freq_list)
{
	uint16_t chwidth = BW_40;

	dfs_calc_bonding_freqs(center_freq, chwidth, freq_list);
}

/**
 * dfs_get_80mhz_bonding_channels() - Get bonding frequency list of 80MHz
 * channel.
 * @center_freq: Center frequency of the 80MHz channel.
 * @freq_list: Pointer to frequency list.
 *
 * Return: void
 */
static
void dfs_get_80mhz_bonding_channels(uint16_t center_freq, uint16_t *freq_list)
{
	uint16_t chwidth = BW_80;

	dfs_calc_bonding_freqs(center_freq, chwidth, freq_list);
}

/**
 * dfs_get_160mhz_bonding_channels() - Get bonding frequency list of 160MHz
 * channel.
 * @center_freq: Center frequency of the 160MHz channel.
 * @freq_list: Pointer to frequency list.
 *
 * Return: void
 */
static
void dfs_get_160mhz_bonding_channels(uint16_t center_freq, uint16_t *freq_list)
{
	uint16_t chwidth = BW_160;

	dfs_calc_bonding_freqs(center_freq, chwidth, freq_list);
}

/**
 * dfs_get_320mhz_bonding_channels() - Get bonding frequency list of 320MHz
 * channel.
 * @center_freq: Center frequency of the 320MHz channel.
 * @freq_list: Pointer to frequency list.
 *
 * Return: void
 */
#ifdef WLAN_FEATURE_11BE
static
void dfs_get_320mhz_bonding_channels(uint16_t center_freq, uint16_t *freq_list,
				     uint8_t *nchannels)
{
	uint16_t chwidth = 320;

	*nchannels = 16;
	dfs_calc_bonding_freqs(center_freq, chwidth, freq_list);
}
#else
static
void dfs_get_320mhz_bonding_channels(uint16_t center_freq, uint16_t *freq_list,
				     uint8_t *nchannels)
{
	*nchannels = 0;
	dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
		  "320MHz chan width for non 11be");
}
#endif

/*
 * dfs_get_bonding_channel_without_seg_info_for_freq() - Get bonding frequency
 * list.
 * @chan: Pointer to dfs_channel.
 * @freq_list: Pointer to frequency list.
 */
#ifdef CONFIG_CHAN_FREQ_API
uint8_t
dfs_get_bonding_channel_without_seg_info_for_freq(struct dfs_channel *chan,
						  uint16_t *freq_list)
{
	uint16_t center_freq;
	uint8_t nchannels = 0;

	center_freq = chan->dfs_ch_mhz_freq_seg1;

	if (WLAN_IS_CHAN_MODE_20(chan)) {
		nchannels = 1;
		dfs_get_20mhz_bonding_channels(center_freq,
					       freq_list);
	} else if (WLAN_IS_CHAN_MODE_40(chan)) {
		nchannels = 2;
		dfs_get_40mhz_bonding_channels(center_freq,
					       freq_list);
	} else if (WLAN_IS_CHAN_MODE_80(chan)) {
		nchannels = 4;
		dfs_get_80mhz_bonding_channels(center_freq,
					       freq_list);
	} else if (WLAN_IS_CHAN_MODE_80_80(chan)) {
		nchannels = 8;
		dfs_get_80mhz_bonding_channels(center_freq,
					       freq_list);
		center_freq = chan->dfs_ch_mhz_freq_seg2;
		dfs_get_80mhz_bonding_channels(center_freq,
					       freq_list + 4);
	} else if (WLAN_IS_CHAN_MODE_160(chan)) {
		nchannels = 8;
		center_freq = chan->dfs_ch_mhz_freq_seg2;
		dfs_get_160mhz_bonding_channels(center_freq, freq_list);
	} else if (WLAN_IS_CHAN_MODE_320(chan)) {
		center_freq = chan->dfs_ch_mhz_freq_seg2;
		dfs_get_320mhz_bonding_channels(center_freq, freq_list,
						&nchannels);
	}

	return nchannels;
}
#endif

#ifdef CONFIG_CHAN_FREQ_API
/*
 * dfs_get_agile_subchans_for_curchan_160() - Get bonding frequency list of
 * agile channels when current operating channel is 160MHz.
 *
 * @dfs: Pointer to DFS structure.
 * @center_freq: Center frequency of the channel.
 * @segment_id: Segment ID of interest. 0 for primary segment and 1 for
 * secondary segment.
 * @freq_list: Pointer to frequency list.
 * @nchannels: Number of subchannel.
 */
static void
dfs_get_agile_subchans_for_curchan_160(struct wlan_dfs *dfs,
				       uint16_t center_freq,
				       uint32_t segment_id,
				       uint16_t *freq_list,
				       uint8_t *nchannels)
{
	if (dfs->dfs_precac_chwidth == CH_WIDTH_80MHZ) {
		/*
		 * The current operating channel is 160MHz and
		 * the agile channel is 80MHz. This can happen
		 * in HK only.
		 */
		*nchannels = 4;
		dfs_get_80mhz_bonding_channels(center_freq,
					       freq_list);
	} else if (dfs->dfs_precac_chwidth == CH_WIDTH_160MHZ)
		/*
		 * The current operating channel is 160MHz and
		 * the agile channel is 160MHz.
		 * Pine ADFS specific.
		 */
		dfs_get_160mhz_bonding_channels(center_freq,
						freq_list);
	else if (dfs->dfs_precac_chwidth == CH_WIDTH_80P80MHZ) {
		/*
		 * The current operating channel is 160MHz and the agile channel
		 * is 165MHz(restricted 80P80MHZ). Pine ADFS specific.
		 * If the segment id is primary segment 0, shift the center
		 * frequency 5730MHz to the center of left 80MHz segment 5690MHz
		 * and add the subchannels of the left 80MHz segment.
		 * If the segment id is secondary segment 1, shift the center
		 * frequency 5730MHz to the center of right 80MHz segment
		 * 5775MHz and add the subchannels of the right 80MHz segment.
		 */
		*nchannels = 4;
		center_freq = (segment_id) ?
			(center_freq + DFS_165MHZ_SECOND_SEG_OFFSET_RIGHT) :
			(center_freq - DFS_165MHZ_SECOND_SEG_OFFSET_LEFT);
		dfs_get_80mhz_bonding_channels(center_freq,
					       freq_list);
	}
}

/*
 * dfs_get_bonding_channels_for_freq() - Get bonding channel frequency.
 * @dfs: Pointer to wlan_dfs.
 * @curchan: Pointer to dfs_channel.
 * @segment_id: Segment ID.
 * @detector_id: Detector ID.
 * @freq_list: Pointer to frequency list.
 */
uint8_t dfs_get_bonding_channels_for_freq(struct wlan_dfs *dfs,
					  struct dfs_channel *curchan,
					  uint32_t segment_id,
					  uint8_t detector_id,
					  uint16_t *freq_list)
{
	uint16_t center_freq;
	uint8_t nchannels = 0;

	/*
	 * For radar in agile detector, use the center of the channel
	 * configured to the agile detector.
	 * For radar on a 160MHz home channel, use the center of 160MHz.
	 * For radar on all other bandwidths, use the center of the segment
	 * affected.
	 */
	if (detector_id == dfs_get_agile_detector_id(dfs))
		center_freq = dfs->dfs_agile_precac_freq_mhz;
	else if (WLAN_IS_CHAN_MODE_160(curchan) ||
		 WLAN_IS_CHAN_MODE_320(curchan))
		center_freq = curchan->dfs_ch_mhz_freq_seg2;
	else if (!segment_id)
		center_freq = curchan->dfs_ch_mhz_freq_seg1;
	else {
		/* When precac is running "dfs_ch_vhtop_ch_freq_seg2" is
		 * zero and "dfs_precac_secondary_freq" holds the secondary
		 * frequency.
		 */
		if (dfs_is_legacy_precac_enabled(dfs) &&
		    dfs_is_precac_timer_running(dfs))
			center_freq = dfs->dfs_precac_secondary_freq_mhz;
		else
			center_freq = curchan->dfs_ch_mhz_freq_seg2;
	}

	if (WLAN_IS_CHAN_MODE_20(curchan)) {
		nchannels = 1;
		dfs_get_20mhz_bonding_channels(center_freq, freq_list);
	} else if (WLAN_IS_CHAN_MODE_40(curchan)) {
		nchannels = 2;
		dfs_get_40mhz_bonding_channels(center_freq, freq_list);
	} else if (WLAN_IS_CHAN_MODE_80(curchan)) {
		nchannels = 4;
		dfs_get_80mhz_bonding_channels(center_freq, freq_list);
	} else if (WLAN_IS_CHAN_MODE_160(curchan)) {
		nchannels = 8;
		if (detector_id == dfs_get_agile_detector_id(dfs))
			dfs_get_agile_subchans_for_curchan_160(dfs,
							       center_freq,
							       segment_id,
							       freq_list,
							       &nchannels);
		else
			dfs_get_160mhz_bonding_channels(center_freq, freq_list);
	} else if (WLAN_IS_CHAN_MODE_320(curchan)) {
		dfs_get_320mhz_bonding_channels(center_freq, freq_list,
						&nchannels);
	}  else if (WLAN_IS_CHAN_MODE_80_80(curchan)) {
		/*
		 * If the current channel's bandwidth is 80P80MHz,
		 * the corresponding agile Detector's bandwidth will be 160MHz
		 * in case of Pine ADFS.
		 */
		if (detector_id == dfs_get_agile_detector_id(dfs)) {
			if (dfs->dfs_precac_chwidth == CH_WIDTH_160MHZ) {
				nchannels = 8;
				dfs_get_160mhz_bonding_channels(center_freq,
								freq_list);
			} else if (dfs->dfs_precac_chwidth == CH_WIDTH_80MHZ) {
				nchannels = 4;
				dfs_get_80mhz_bonding_channels(center_freq,
							       freq_list);
			} else {
				dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
					"Incorrect precac width %u",
					dfs->dfs_precac_chwidth);
			}
		} else {
			/*
			 * If the radar is getting detected in 80P80MHz home
			 * channel, only the 80MHz segment that is infected with
			 * radar is of interest. The other 80MHz segment is
			 * ignored. The center frequency of the radar infected
			 * segment is dfs_ch_mhz_freq_seg1 if primary and
			 * dfs_ch_mhz_freq_seg2 in case of secondary.
			 */
			nchannels = 4;
			dfs_get_80mhz_bonding_channels(center_freq, freq_list);
		}
	}

	return nchannels;
}
#endif

void dfs_reset_bangradar(struct wlan_dfs *dfs)
{
	dfs->dfs_bangradar_type = DFS_NO_BANGRADAR;
}

/**
 * dfs_radar_found_event_basic_sanity() - Check if radar event is received on a
 * DFS channel.
 * @dfs: Pointer to wlan_dfs structure.
 * @chan: Current channel.
 *
 * Return: If a radar event found on NON-DFS channel return false. Otherwise,
 * return true.
 */
static
bool dfs_radar_found_event_basic_sanity(struct wlan_dfs *dfs,
					struct dfs_channel *chan)
{
	if (!chan) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"dfs->dfs_curchan is NULL");
		return false;
	}

	if (!(WLAN_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(chan))) {
		dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS,
			  "radar event on non-DFS chan");
		return false;
	}

	return true;
}

void dfs_send_csa_to_current_chan(struct wlan_dfs *dfs)
{
	dfs->wlan_dfstest = 1;
	dfs->wlan_dfstest_ieeechan = dfs->dfs_curchan->dfs_ch_ieee;
	dfs->wlan_dfstesttime = 1;   /* 1ms */
	qdf_timer_sync_cancel(&dfs->wlan_dfstesttimer);
	qdf_timer_start(&dfs->wlan_dfstesttimer, dfs->wlan_dfstesttime);
}

int dfs_second_segment_radar_disable(struct wlan_dfs *dfs)
{
	dfs->dfs_proc_phyerr &= ~DFS_SECOND_SEGMENT_RADAR_EN;

	return 0;
}

#ifdef WLAN_DFS_FULL_OFFLOAD
void dfs_inc_num_radar(struct wlan_dfs *dfs)
{
	dfs->wlan_dfs_stats.num_radar_detects++;
}
#endif /* WLAN_DFS_FULL_OFFLOAD */

#if defined(WLAN_DFS_TRUE_160MHZ_SUPPORT) && defined(WLAN_DFS_FULL_OFFLOAD)
void dfs_translate_radar_params(struct wlan_dfs *dfs,
				struct radar_found_info *radar_found)
{
	struct dfs_channel *curchan = dfs->dfs_curchan;
	bool is_primary_ch_right_of_center = false;

	if (!dfs_is_true_160mhz_supported(dfs))
		return;

	if (radar_found->detector_id == dfs_get_agile_detector_id(dfs)) {
		dfs_translate_radar_params_for_agile_chan(dfs, radar_found);
		return;
	}

	/* Is the primary channel ( or primary 80 segment) to the right
	 * of the center of 160/165Mhz channel.
	 */
	if (curchan->dfs_ch_freq > curchan->dfs_ch_mhz_freq_seg2)
		is_primary_ch_right_of_center = true;

	if (WLAN_IS_CHAN_MODE_160(curchan)) {
		if (radar_found->freq_offset > 0) {
			/* Offset positive: Equivalent to Upper IEEE
			 * 80Mhz chans Synthesizer.
			 */
			if (!is_primary_ch_right_of_center)
				radar_found->segment_id = SEG_ID_SECONDARY;
			radar_found->freq_offset -=
				DFS_160MHZ_SECOND_SEG_OFFSET;
		} else {
			/* Offset negative: Equivalent to Lower IEEE
			 * 80Mhz chans Synthesizer.
			 */
			if (is_primary_ch_right_of_center)
				radar_found->segment_id = SEG_ID_SECONDARY;
			radar_found->freq_offset +=
				DFS_160MHZ_SECOND_SEG_OFFSET;
		}
	} else if (WLAN_IS_CHAN_MODE_165(dfs, curchan)) {
		/* If offset is greater than 40MHz, radar is found on the
		 * secondary segment.
		 */
		if (abs(radar_found->freq_offset) > 40) {
			radar_found->segment_id = SEG_ID_SECONDARY;
			/* Update the freq. offset with respect to the
			 * secondary segment center freq.
			 */
			if (is_primary_ch_right_of_center)
				radar_found->freq_offset +=
					DFS_80P80MHZ_SECOND_SEG_OFFSET;
			else
				radar_found->freq_offset -=
					DFS_80P80MHZ_SECOND_SEG_OFFSET;
		}
	}
}
#endif /* WLAN_DFS_TRUE_160MHZ_SUPPORT */

/**
 * dfs_radar_action_for_hw_mode_switch()- Radar cannot be processed when HW
 * switch is in progress. So save the radar found parameters for
 * future processing.
 * @dfs: Pointer to wlan_dfs structure.
 * @radar_found: Pointer to radar found structure.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
dfs_radar_action_for_hw_mode_switch(struct wlan_dfs *dfs,
				    struct radar_found_info *radar_found)
{
	struct radar_found_info *radar_params = NULL;

	radar_params = qdf_mem_malloc(sizeof(*radar_params));
	if (!radar_params)
		return QDF_STATUS_E_NOMEM;

	/* If CAC timer is running, cancel it here rather than
	 * after processing to avoid handling unnecessary CAC timeouts.
	 */
	if (dfs->dfs_cac_timer_running)
		dfs_cac_stop(dfs);

	/* If CAC timer is to be handled after mode switch and then
	 * we receive radar, no point in handling CAC completion.
	 */
	if (dfs->dfs_defer_params.is_cac_completed)
		dfs->dfs_defer_params.is_cac_completed = false;
	qdf_mem_copy(radar_params, radar_found, sizeof(*radar_params));
	dfs->dfs_defer_params.radar_params = radar_params;
	dfs->dfs_defer_params.is_radar_detected = true;

	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_CHAN_FREQ_API
uint8_t
dfs_find_radar_affected_channels(struct wlan_dfs *dfs,
				 struct radar_found_info *radar_found,
				 uint16_t *freq_list,
				 uint32_t freq_center)
{
	uint8_t num_channels;

	if (dfs->dfs_bangradar_type == DFS_BANGRADAR_FOR_ALL_SUBCHANS)
		num_channels =
			dfs_get_bonding_channel_without_seg_info_for_freq
			(dfs->dfs_curchan, freq_list);
	/* BW reduction is dependent on subchannel marking */
	else if ((dfs->dfs_use_nol_subchannel_marking) &&
		 (!(dfs->dfs_bangradar_type) ||
		 (dfs->dfs_bangradar_type ==
		  DFS_BANGRADAR_FOR_SPECIFIC_SUBCHANS)))
		num_channels =
		dfs_find_radar_affected_subchans_for_freq(dfs,
							  radar_found,
							  freq_list,
							  freq_center);
	else
		num_channels = dfs_get_bonding_channels_for_freq
			(dfs,
			 dfs->dfs_curchan,
			 radar_found->segment_id,
			 radar_found->detector_id,
			 freq_list);

	return num_channels;
}

#if defined(QCA_SUPPORT_AGILE_DFS) || defined(ATH_SUPPORT_ZERO_CAC_DFS) || \
	defined(QCA_SUPPORT_ADFS_RCAC)
/**
 * dfs_is_radarsource_agile() - Indicates whether the radar event is received
 * on the agile channel.
 * @dfs: Pointer to wlan_dfs structure.
 * @radar_found: Pointer to radar_found_info structure.
 *
 * Return: QDF_STATUS
 */
static
bool dfs_is_radarsource_agile(struct wlan_dfs *dfs,
			      struct radar_found_info *radar_found)
{
	bool is_radar_from_agile_dfs =
	    ((dfs_is_agile_precac_enabled(dfs) &&
	      dfs_is_precac_timer_running(dfs)) ||
	     dfs_is_agile_rcac_enabled(dfs)) &&
	    (radar_found->detector_id == dfs_get_agile_detector_id(dfs));
	bool is_radar_from_zero_wait_dfs =
	    (dfs_is_legacy_precac_enabled(dfs) &&
	     dfs_is_precac_timer_running(dfs) &&
	     (radar_found->segment_id == SEG_ID_SECONDARY));

	dfs_debug(dfs, WLAN_DEBUG_DFS_AGILE,
		  "radar on PreCAC segment: ADFS:%d Zero Wait DFS:%d",
		  is_radar_from_agile_dfs,
		  is_radar_from_zero_wait_dfs);

	return (is_radar_from_agile_dfs || is_radar_from_zero_wait_dfs);
}
#else
static
bool dfs_is_radarsource_agile(struct wlan_dfs *dfs,
			      struct radar_found_info *radar_found)
{
	return false;
}
#endif

QDF_STATUS
dfs_process_radar_ind(struct wlan_dfs *dfs,
		      struct radar_found_info *radar_found)
{
	QDF_STATUS status;

	/* Acquire a lock to avoid initiating mode switch till radar
	 * processing is completed.
	 */
	DFS_RADAR_MODE_SWITCH_LOCK(dfs);

	if (utils_dfs_can_ignore_radar_event(dfs->dfs_pdev_obj)) {
		DFS_RADAR_MODE_SWITCH_UNLOCK(dfs);
		return QDF_STATUS_SUCCESS;
	}

	/* Before processing radar, check if HW mode switch is in progress.
	 * If in progress, defer the processing of radar event received till
	 * the mode switch is completed.
	 */
	if (dfs_is_hw_mode_switch_in_progress(dfs))
		status = dfs_radar_action_for_hw_mode_switch(dfs, radar_found);
	else if (dfs_is_radarsource_agile(dfs, radar_found))
		status = dfs_process_radar_ind_on_agile_chan(dfs, radar_found);
	else
		status = dfs_process_radar_ind_on_home_chan(dfs, radar_found);

	DFS_RADAR_MODE_SWITCH_UNLOCK(dfs);

	return status;
}

QDF_STATUS
dfs_process_radar_ind_on_home_chan(struct wlan_dfs *dfs,
				   struct radar_found_info *radar_found)
{
	bool wait_for_csa = false;
	uint16_t freq_list[MAX_20MHZ_SUBCHANS];
	uint16_t nol_freq_list[MAX_20MHZ_SUBCHANS];
	uint8_t num_channels;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint32_t freq_center;
	uint32_t radarfound_freq;
	struct dfs_channel *dfs_curchan;

	dfs_curchan = dfs->dfs_curchan;

	/* Check if the current channel is a non DFS channel
	 * If the current channel is non-DFS and the radar is from Agile
	 * Detector we need to process it since Agile Detector has a
	 * different channel.
	 */
	if (!dfs_radar_found_event_basic_sanity(dfs, dfs_curchan))
		goto exit;

	dfs_compute_radar_found_cfreq(dfs, radar_found, &freq_center);
	radarfound_freq = freq_center + radar_found->freq_offset;

	if (radar_found->segment_id == SEG_ID_SECONDARY)
		dfs_debug(dfs, WLAN_DEBUG_DFS_ALWAYS,
			  "Radar found on second segment.Radarfound Freq=%d MHz.Secondary Chan cfreq=%d MHz.",
			  radarfound_freq, freq_center);
	else
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
			  "Radar found on channel=%d, freq=%d MHz. Primary beaconning chan:%d, freq=%d MHz.",
			  utils_dfs_freq_to_chan(radarfound_freq),
			  radarfound_freq, dfs_curchan->dfs_ch_ieee,
			  dfs_curchan->dfs_ch_freq);

	utils_dfs_deliver_event(dfs->dfs_pdev_obj, radarfound_freq,
				WLAN_EV_RADAR_DETECTED);

	if (!dfs->dfs_use_nol) {
		if (!dfs->dfs_is_offload_enabled) {
			dfs_radar_disable(dfs);
			dfs_second_segment_radar_disable(dfs);
			dfs_flush_additional_pulses(dfs);
		}
		dfs_reset_bangradar(dfs);
		dfs_send_csa_to_current_chan(dfs);
		status = QDF_STATUS_SUCCESS;
		goto exit;
	}
	num_channels = dfs_find_radar_affected_channels(dfs,
							radar_found,
							freq_list,
							freq_center);

	dfs_reset_bangradar(dfs);

	status = dfs_radar_add_channel_list_to_nol_for_freq(dfs,
							    freq_list,
							    nol_freq_list,
							    &num_channels);
	if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"radar event received on invalid channel");
		goto exit;
	}

	/*
	 * If precac is running and the radar found in secondary
	 * VHT80 mark the channel as radar and add to NOL list.
	 * Otherwise random channel selection can choose this
	 * channel.
	 */
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "found_on_second=%d is_pre=%d",
		  dfs->is_radar_found_on_secondary_seg,
		  dfs_is_precac_timer_running(dfs));
	/*
	 * Even if radar found on primary, we need to mark the channel as NOL
	 * in preCAC list. The preCAC list also maintains the current CAC
	 * channels as part of pre-cleared DFS. Hence call the API
	 * to mark channels as NOL irrespective of preCAC being enabled or not.
	 */

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "Radar found on dfs detector: %d", radar_found->detector_id);
	dfs_mark_precac_nol_for_freq(dfs,
				     dfs->is_radar_found_on_secondary_seg,
				     radar_found->detector_id,
				     nol_freq_list,
				     num_channels);

	dfs_send_nol_ie_and_rcsa(dfs,
				 radar_found,
				 nol_freq_list,
				 num_channels,
				 &wait_for_csa);

	if (!dfs->dfs_is_offload_enabled &&
	    dfs->is_radar_found_on_secondary_seg) {
		dfs_second_segment_radar_disable(dfs);
		dfs->is_radar_found_on_secondary_seg = 0;

		if (dfs->is_radar_during_precac) {
			dfs->is_radar_during_precac = 0;
			goto exit;
		}
	}

	/*
	 * XXX TODO: the umac NOL code isn't used, but
	 * WLAN_CHAN_DFS_RADAR still gets set. Since the umac
	 * NOL code isn't used, that flag is never cleared. This
	 * needs to be fixed. See EV 105776.
	 */
	if (wait_for_csa)
		goto exit;

	/*
	 * EV 129487 : We have detected radar in the channel,
	 * stop processing PHY error data as this can cause
	 * false detect in the new channel while channel
	 * change is in progress.
	 */

	if (!dfs->dfs_is_offload_enabled) {
		dfs_radar_disable(dfs);
		dfs_second_segment_radar_disable(dfs);
		/*
		 * The radar queues were reset just after the filter match, but
		 * the phyerror reception was not disabled. This might
		 * cause the unwanted additional/accumulated pulses to be
		 * detected as radar in the new channel. So, clear the radar
		 * queues and the associated variables.
		 */
		dfs_flush_additional_pulses(dfs);
	}

	dfs_mlme_mark_dfs(dfs->dfs_pdev_obj,
			  dfs->dfs_curchan->dfs_ch_ieee,
			  dfs->dfs_curchan->dfs_ch_freq,
			  dfs->dfs_curchan->dfs_ch_mhz_freq_seg2,
			  dfs->dfs_curchan->dfs_ch_flags);

exit:
	return status;
}
#endif
