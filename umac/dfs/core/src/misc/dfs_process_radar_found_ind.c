/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
#include <wlan_dfs_utils_api.h>
#include "wlan_dfs_mlme_api.h"

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

/**
 * dfs_radar_add_channel_list_to_nol()- Add given channels to nol
 * @dfs: Pointer to wlan_dfs structure.
 * @channels: Pointer to the channel list.
 * @num_channels: Number of channels in the list.
 *
 * Add list of channels to nol, only if the channel is dfs.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS dfs_radar_add_channel_list_to_nol(struct wlan_dfs *dfs,
						    uint8_t *channels,
						    uint8_t num_channels)
{
	int i;
	uint8_t last_chan = 0;
	uint8_t nollist[NUM_CHANNELS_160MHZ];
	uint8_t num_ch = 0;

	if (num_channels > NUM_CHANNELS_160MHZ) {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"Invalid num channels: %d", num_channels);
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < num_channels; i++) {
		if (channels[i] == 0 ||
		    channels[i] == last_chan)
			continue;
		if (!utils_is_dfs_ch(dfs->dfs_pdev_obj, channels[i])) {
			dfs_info(dfs, WLAN_DEBUG_DFS, "ch=%d is not dfs, skip",
				 channels[i]);
			continue;
		}
		last_chan = channels[i];
		DFS_NOL_ADD_CHAN_LOCKED(dfs,
				(uint16_t)utils_dfs_chan_to_freq(channels[i]),
				dfs->wlan_dfs_nol_timeout);
		nollist[num_ch++] = last_chan;
		dfs_info(dfs, WLAN_DEBUG_DFS, "ch=%d Added to NOL", last_chan);
	}

	if (!num_ch) {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"dfs channels not found in channel list");
		return QDF_STATUS_E_FAILURE;
	}

	utils_dfs_reg_update_nol_ch(dfs->dfs_pdev_obj,
				    nollist, num_ch, DFS_NOL_SET);
	dfs_nol_update(dfs);
	utils_dfs_save_nol(dfs->dfs_pdev_obj);

	return QDF_STATUS_SUCCESS;
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

/**
 * dfs_find_radar_affected_subchans() - Finds radar affected sub channels.
 * @dfs: Pointer to wlan_dfs structure.
 * @radar_found: Pointer to radar_found structure.
 * @channels: Pointer to save radar affected channels.
 *
 * Return: Number of channels.
 */
static uint8_t dfs_find_radar_affected_subchans(struct wlan_dfs *dfs,
						struct radar_found_info
						*radar_found,
						uint8_t *channels)
{
	int i;
	uint32_t freq_center, flag;
	int32_t sidx;
	struct dfs_channel *curchan = dfs->dfs_curchan;
	struct freqs_offsets freq_offset;

	qdf_mem_set(&freq_offset, sizeof(freq_offset), 0);
	flag = curchan->dfs_ch_flags;

	for (i = 0; i < DFS_NUM_FREQ_OFFSET; i++)
		freq_offset.offset[i] = radar_found->freq_offset;

	sidx = DFS_FREQ_OFFSET_TO_SIDX(radar_found->freq_offset);

	if (!radar_found->segment_id)
		freq_center = utils_dfs_chan_to_freq(
				curchan->dfs_ch_vhtop_ch_freq_seg1);
	else {
		if (dfs_is_precac_timer_running(dfs)) {
			freq_center = utils_dfs_chan_to_freq(
					dfs->dfs_precac_secondary_freq);
		} else {
			freq_center = utils_dfs_chan_to_freq(
					curchan->dfs_ch_vhtop_ch_freq_seg2);
			if (flag & WLAN_CHAN_VHT160)
				freq_center += DFS_160MHZ_SECOND_SEG_OFFSET;
		}
	}

	dfs_info(dfs, WLAN_DEBUG_DFS,
		 "seg=%d, sidx=%d, offset=%d, chirp=%d, flag=%d, f=%d",
		 radar_found->segment_id, sidx,
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
			WLAN_IS_CHAN_MODE_80_80(curchan)) {
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

	for (i = 0; i < DFS_NUM_FREQ_OFFSET; i++) {
		channels[i] = utils_dfs_freq_to_chan(freq_offset.freq[i]);
		dfs_info(dfs, WLAN_DEBUG_DFS, "offset=%d, channel=%d",
			 i, channels[i]);
	}

	return i;
}

/**
 * dfs_get_bonding_channels() - Get bonding channels.
 * @curchan: Pointer to dfs_channels to know width and primary channel.
 * @segment_id: Segment id, useful for 80+80/160 MHz operating band.
 * @channels: Pointer to save radar affected channels.
 *
 * Return: Number of channels.
 */
uint8_t dfs_get_bonding_channels(struct dfs_channel *curchan,
				 uint32_t segment_id,
				 uint8_t *channels)
{
	uint8_t center_chan;
	uint8_t nchannels = 0;

	if (!segment_id)
		center_chan = curchan->dfs_ch_vhtop_ch_freq_seg1;
	else
		center_chan = curchan->dfs_ch_vhtop_ch_freq_seg2;

	if (WLAN_IS_CHAN_MODE_20(curchan)) {
		nchannels = 1;
		channels[0] = center_chan;
	} else if (WLAN_IS_CHAN_MODE_40(curchan)) {
		nchannels = 2;
		channels[0] = center_chan - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[1] = center_chan + DFS_5GHZ_NEXT_CHAN_OFFSET;
	} else if (WLAN_IS_CHAN_MODE_80(curchan) ||
		   WLAN_IS_CHAN_MODE_80_80(curchan)) {
		nchannels = 4;
		channels[0] = center_chan - DFS_5GHZ_2ND_CHAN_OFFSET;
		channels[1] = center_chan - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[2] = center_chan + DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[3] = center_chan + DFS_5GHZ_2ND_CHAN_OFFSET;
	} else if (WLAN_IS_CHAN_MODE_160(curchan)) {
		nchannels = 8;
		center_chan = curchan->dfs_ch_vhtop_ch_freq_seg2;
		channels[0] = center_chan - DFS_5GHZ_4TH_CHAN_OFFSET;
		channels[1] = center_chan - DFS_5GHZ_3RD_CHAN_OFFSET;
		channels[2] = center_chan - DFS_5GHZ_2ND_CHAN_OFFSET;
		channels[3] = center_chan - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[4] = center_chan + DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[5] = center_chan + DFS_5GHZ_2ND_CHAN_OFFSET;
		channels[6] = center_chan + DFS_5GHZ_3RD_CHAN_OFFSET;
		channels[7] = center_chan + DFS_5GHZ_4TH_CHAN_OFFSET;
	}

	return nchannels;
}

int dfs_radarevent_basic_sanity(struct wlan_dfs *dfs,
		struct dfs_channel *chan)
{
	if (!(dfs->dfs_second_segment_bangradar ||
				dfs_is_precac_timer_running(dfs)))
		if (!(WLAN_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(chan))) {
			dfs_debug(dfs, WLAN_DEBUG_DFS2,
					"radar event on non-DFS chan");
			if (!(dfs->dfs_is_offload_enabled)) {
				dfs_reset_radarq(dfs);
				dfs_reset_alldelaylines(dfs);
				dfs->dfs_bangradar = 0;
			}
			return 0;
		}

	return 1;
}

/**
 * dfs_send_csa_to_current_chan() - Send CSA to current channel
 * @dfs: Pointer to wlan_dfs structure.
 *
 * For the test mode(usenol = 0), don't do a CSA; but setup the test timer so
 * we get a CSA _back_ to the current operating channel.
 */
static inline void dfs_send_csa_to_current_chan(struct wlan_dfs *dfs)
{
	qdf_timer_stop(&dfs->wlan_dfstesttimer);
	dfs->wlan_dfstest = 1;
	dfs->wlan_dfstest_ieeechan = dfs->dfs_curchan->dfs_ch_ieee;
	dfs->wlan_dfstesttime = 1;   /* 1ms */
	qdf_timer_mod(&dfs->wlan_dfstesttimer, dfs->wlan_dfstesttime);
}

int dfs_second_segment_radar_disable(struct wlan_dfs *dfs)
{
	dfs->dfs_proc_phyerr &= ~DFS_SECOND_SEGMENT_RADAR_EN;

	return 0;
}

QDF_STATUS dfs_process_radar_ind(struct wlan_dfs *dfs,
				 struct radar_found_info *radar_found)
{
	bool wait_for_csa = false;
	uint8_t channels[NUM_CHANNELS_160MHZ];
	uint8_t num_channels;
	QDF_STATUS status;

	if (!dfs->dfs_curchan) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs->dfs_curchan is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* Check if the current channel is a non DFS channel */
	if (!dfs_radarevent_basic_sanity(dfs, dfs->dfs_curchan)) {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"radar event on a non-DFS channel");
		return QDF_STATUS_E_FAILURE;
	}

	if (radar_found->segment_id == SEG_ID_SECONDARY)
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Radar found on second segment VHT80 freq=%d MHz",
			 dfs->dfs_precac_secondary_freq);
	else
		dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS,
			 "Radar found on channel=%d, freq=%d MHz",
			 dfs->dfs_curchan->dfs_ch_ieee,
			 dfs->dfs_curchan->dfs_ch_freq);

	if (!dfs->dfs_use_nol) {
		dfs_send_csa_to_current_chan(dfs);
		return QDF_STATUS_SUCCESS;
	}

	if (dfs->dfs_use_nol_subchannel_marking)
		num_channels = dfs_find_radar_affected_subchans(dfs,
								radar_found,
								channels);
	else
		num_channels = dfs_get_bonding_channels(dfs->dfs_curchan,
							radar_found->segment_id,
							channels);

	status = dfs_radar_add_channel_list_to_nol(dfs, channels, num_channels);
	if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"radar event received on invalid channel");
		return status;
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
	 * Even if radar found on primary, we need to move the channel
	 * from precac-required-list and precac-done-list to
	 * precac-nol-list.
	 */
	if (dfs->dfs_precac_enable)
		dfs_mark_precac_dfs(dfs, dfs->is_radar_found_on_secondary_seg);

	if (!dfs->dfs_is_offload_enabled &&
	    dfs->is_radar_found_on_secondary_seg) {
		dfs_second_segment_radar_disable(dfs);
		dfs->is_radar_found_on_secondary_seg = 0;

		if (dfs->is_radar_during_precac) {
			dfs->is_radar_during_precac = 0;
			return QDF_STATUS_SUCCESS;
		}
	}

	/*
	 * This calls into the umac DFS code, which sets the umac
	 * related radar flags and begins the channel change
	 * machinery.
	 * XXX TODO: the umac NOL code isn't used, but
	 * WLAN_CHAN_DFS_RADAR still gets set. Since the umac
	 * NOL code isn't used, that flag is never cleared. This
	 * needs to be fixed. See EV 105776.
	 */
	dfs_mlme_start_rcsa(dfs->dfs_pdev_obj, &wait_for_csa);
	if (wait_for_csa)
		return QDF_STATUS_SUCCESS;

	/*
	 * EV 129487 : We have detected radar in the channel,
	 * stop processing PHY error data as this can cause
	 * false detect in the new channel while channel
	 * change is in progress.
	 */

	if (!dfs->dfs_is_offload_enabled) {
		dfs_radar_disable(dfs);
		dfs_second_segment_radar_disable(dfs);
	}

	dfs_mlme_mark_dfs(dfs->dfs_pdev_obj,
			dfs->dfs_curchan->dfs_ch_ieee,
			dfs->dfs_curchan->dfs_ch_freq,
			dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg2,
			dfs->dfs_curchan->dfs_ch_flags);

	return QDF_STATUS_SUCCESS;
}
