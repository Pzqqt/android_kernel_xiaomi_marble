/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
#include "../dfs_etsi_precac.h"
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
		utils_dfs_deliver_event(dfs->dfs_pdev_obj,
					(uint16_t)
					utils_dfs_chan_to_freq(channels[i]),
					WLAN_EV_NOL_STARTED);
		dfs_info(dfs, WLAN_DEBUG_DFS_NOL, "ch=%d Added to NOL",
			 last_chan);
	}

	if (!num_ch) {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"dfs channels not found in channel list");
		return QDF_STATUS_E_FAILURE;
	}

	utils_dfs_reg_update_nol_ch(dfs->dfs_pdev_obj,
				    nollist, num_ch, DFS_NOL_SET);

	if (dfs->dfs_is_stadfs_enabled)
		if (dfs_mlme_is_opmode_sta(dfs->dfs_pdev_obj))
			utils_dfs_reg_update_nol_history_ch(
					dfs->dfs_pdev_obj, nollist, num_ch,
					DFS_NOL_HISTORY_SET);

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

/* dfs_compute_radar_found_cfreq(): Computes the centre frequency of the
 * radar hit channel.
 * @dfs: Pointer to wlan_dfs structure.
 * @radar_found: Pointer to radar_found_info.
 * @freq_center: Pointer to retrieve the value of radar found cfreq.
 */
static void
dfs_compute_radar_found_cfreq(struct wlan_dfs *dfs,
			      struct radar_found_info
			      *radar_found,
			      uint32_t *freq_center)
{
	struct dfs_channel *curchan = dfs->dfs_curchan;
	uint64_t flag;

	flag = curchan->dfs_ch_flags;
	if (!radar_found->segment_id) {
		*freq_center = utils_dfs_chan_to_freq(
				curchan->dfs_ch_vhtop_ch_freq_seg1);
	} else {
		if (dfs_is_precac_timer_running(dfs)) {
			*freq_center = utils_dfs_chan_to_freq(
					dfs->dfs_precac_secondary_freq);
		} else {
			*freq_center = utils_dfs_chan_to_freq(
					curchan->dfs_ch_vhtop_ch_freq_seg2);
			if ((flag & WLAN_CHAN_VHT160) ||
			    (flag & WLAN_CHAN_HE160))
				*freq_center += DFS_160MHZ_SECOND_SEG_OFFSET;
		}
	}
}

/**
 * dfs_find_radar_affected_subchans() - Finds radar affected sub channels.
 * @dfs: Pointer to wlan_dfs structure.
 * @radar_found: Pointer to radar_found structure.
 * @channels: Pointer to save radar affected channels.
 * @freq_center: Freq_center of the radar affected chan.
 *
 * Return: Number of channels.
 */
static uint8_t dfs_find_radar_affected_subchans(struct wlan_dfs *dfs,
						struct radar_found_info
						*radar_found,
						uint8_t *channels,
						uint32_t freq_center)
{
	int i, j;
	uint8_t num_radar_subchans;
	uint32_t flag;
	int32_t sidx;
	uint8_t candidate_subchan;
	uint8_t cur_subchans[NUM_CHANNELS_160MHZ];
	uint8_t n_cur_subchans;
	struct dfs_channel *curchan = dfs->dfs_curchan;
	struct freqs_offsets freq_offset;

	qdf_mem_zero(&freq_offset, sizeof(freq_offset));
	flag = curchan->dfs_ch_flags;

	for (i = 0; i < DFS_NUM_FREQ_OFFSET; i++)
		freq_offset.offset[i] = radar_found->freq_offset;

	sidx = DFS_FREQ_OFFSET_TO_SIDX(radar_found->freq_offset);

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

	n_cur_subchans = dfs_get_bonding_channels(dfs, curchan,
						  radar_found->segment_id,
						  cur_subchans);

	for (i = 0, num_radar_subchans = 0; i < DFS_NUM_FREQ_OFFSET; i++) {
		candidate_subchan = utils_dfs_freq_to_chan(freq_offset.freq[i]);
		for (j = 0; j < n_cur_subchans; j++) {
			if (cur_subchans[j] == candidate_subchan) {
				channels[num_radar_subchans++] =
						candidate_subchan;
				dfs_info(dfs, WLAN_DEBUG_DFS,
					 "offset=%d, channel=%d",
					 num_radar_subchans,
					 channels[num_radar_subchans - 1]);
				break;
			}
		}
	}
	return num_radar_subchans;
}

uint8_t dfs_get_bonding_channels_without_seg_info(struct dfs_channel *chan,
						  uint8_t *channels)
{
	uint8_t center_chan;
	uint8_t nchannels = 0;

	center_chan = chan->dfs_ch_vhtop_ch_freq_seg1;

	if (WLAN_IS_CHAN_MODE_20(chan)) {
		nchannels = 1;
		channels[0] = center_chan;
	} else if (WLAN_IS_CHAN_MODE_40(chan)) {
		nchannels = 2;
		channels[0] = center_chan - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[1] = center_chan + DFS_5GHZ_NEXT_CHAN_OFFSET;
	} else if (WLAN_IS_CHAN_MODE_80(chan)) {
		nchannels = 4;
		channels[0] = center_chan - DFS_5GHZ_2ND_CHAN_OFFSET;
		channels[1] = center_chan - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[2] = center_chan + DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[3] = center_chan + DFS_5GHZ_2ND_CHAN_OFFSET;
	} else if (WLAN_IS_CHAN_MODE_80_80(chan)) {
		nchannels = 8;
		channels[0] = center_chan - DFS_5GHZ_2ND_CHAN_OFFSET;
		channels[1] = center_chan - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[2] = center_chan + DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[3] = center_chan + DFS_5GHZ_2ND_CHAN_OFFSET;
		center_chan = chan->dfs_ch_vhtop_ch_freq_seg2;
		channels[4] = center_chan - DFS_5GHZ_2ND_CHAN_OFFSET;
		channels[5] = center_chan - DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[6] = center_chan + DFS_5GHZ_NEXT_CHAN_OFFSET;
		channels[7] = center_chan + DFS_5GHZ_2ND_CHAN_OFFSET;
	} else if (WLAN_IS_CHAN_MODE_160(chan)) {
		nchannels = 8;
		center_chan = chan->dfs_ch_vhtop_ch_freq_seg2;
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

uint8_t dfs_get_bonding_channels(struct wlan_dfs *dfs,
				 struct dfs_channel *curchan,
				 uint32_t segment_id,
				 uint8_t *channels)
{
	uint8_t center_chan;
	uint8_t nchannels = 0;

	if (!segment_id)
		center_chan = curchan->dfs_ch_vhtop_ch_freq_seg1;
	else {
		/* When precac is running "dfs_ch_vhtop_ch_freq_seg2" is
		 * zero and "dfs_precac_secondary_freq" holds the secondary
		 * frequency.
		 */
		if (dfs_is_precac_timer_running(dfs))
			center_chan = dfs->dfs_precac_secondary_freq;
		else
			center_chan = curchan->dfs_ch_vhtop_ch_freq_seg2;
	}

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

static inline void dfs_reset_bangradar(struct wlan_dfs *dfs)
{
	dfs->dfs_bangradar_type = DFS_NO_BANGRADAR;
}

int dfs_radarevent_basic_sanity(struct wlan_dfs *dfs,
		struct dfs_channel *chan)
{
	if (!(dfs->dfs_seg_id == SEG_ID_SECONDARY &&
	      dfs_is_precac_timer_running(dfs)))
		if (!(WLAN_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(chan))) {
			dfs_debug(dfs, WLAN_DEBUG_DFS2,
					"radar event on non-DFS chan");
			if (!(dfs->dfs_is_offload_enabled)) {
				dfs_reset_radarq(dfs);
				dfs_reset_alldelaylines(dfs);
				dfs_reset_bangradar(dfs);
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

/* dfs_prepare_nol_ie_bitmap: Create a Bitmap from the radar found subchannels
 * to be sent along with RCSA.
 *
 * Get the subchannels affected by radar and all the channels in current
 * channel.
 * start from the first bit pointing to first subchannel in the current
 * channel, set as 1 if radar affected, 0 if unaffected.
 * If the number of subchannels increases (future cases), the bitmap should
 * be an array of required size.
 *
 * Please change macro "MIN_DFS_SUBCHAN_BW" when NOL logic changes.
 */
static void dfs_prepare_nol_ie_bitmap(struct wlan_dfs *dfs,
				      struct radar_found_info *radar_found,
				      uint8_t *in_sub_channels,
				      uint8_t n_in_sub_channels)
{
	uint8_t cur_subchans[NUM_CHANNELS_160MHZ];
	uint8_t n_cur_subchans;
	uint8_t i;
	uint8_t j;
	uint8_t bits = 0x01;

	n_cur_subchans = dfs_get_bonding_channels(dfs, dfs->dfs_curchan,
						  radar_found->segment_id,
						  cur_subchans);
	dfs->dfs_nol_ie_bandwidth = MIN_DFS_SUBCHAN_BW;
	dfs->dfs_nol_ie_startfreq =
		(uint16_t)utils_dfs_chan_to_freq(cur_subchans[0]);

	/* Search through the array list of radar affected subchannels
	 * to find if the subchannel in our current channel has radar hit.
	 * Break if found to reduce loop count.
	 */
	for (i = 0; i < n_cur_subchans; i++) {
		for (j = 0; j < n_in_sub_channels; j++) {
			if (cur_subchans[i] == in_sub_channels[j]) {
				dfs->dfs_nol_ie_bitmap |= bits;
				break;
			}
		}
		bits <<= 1;
	}
}

void dfs_fetch_nol_ie_info(struct wlan_dfs *dfs,
			   uint8_t *nol_ie_bandwidth,
			   uint16_t *nol_ie_startfreq,
			   uint8_t *nol_ie_bitmap)
{
	if (nol_ie_bandwidth)
		*nol_ie_bandwidth = dfs->dfs_nol_ie_bandwidth;
	if (nol_ie_startfreq)
		*nol_ie_startfreq = dfs->dfs_nol_ie_startfreq;
	if (nol_ie_bitmap)
		*nol_ie_bitmap = dfs->dfs_nol_ie_bitmap;
}

void dfs_get_rcsa_flags(struct wlan_dfs *dfs, bool *is_rcsa_ie_sent,
			bool *is_nol_ie_sent)
{
	if (is_rcsa_ie_sent)
		*is_rcsa_ie_sent = dfs->dfs_is_rcsa_ie_sent;
	if (is_nol_ie_sent)
		*is_nol_ie_sent = dfs->dfs_is_nol_ie_sent;
}

void dfs_set_rcsa_flags(struct wlan_dfs *dfs, bool is_rcsa_ie_sent,
			bool is_nol_ie_sent)
{
	dfs->dfs_is_rcsa_ie_sent = is_rcsa_ie_sent;
	dfs->dfs_is_nol_ie_sent = is_nol_ie_sent;
}

static void dfs_reset_nol_ie_bitmap(struct wlan_dfs *dfs)
{
	dfs->dfs_nol_ie_bitmap = 0;
}

bool dfs_process_nol_ie_bitmap(struct wlan_dfs *dfs, uint8_t nol_ie_bandwidth,
			       uint16_t nol_ie_startfreq, uint8_t nol_ie_bitmap)
{
	uint8_t num_subchans;
	uint8_t bits = 0x01;
	uint8_t radar_subchans[NUM_CHANNELS_160MHZ];
	bool should_nol_ie_be_sent = true;

	qdf_mem_zero(radar_subchans, sizeof(radar_subchans));
	if (!dfs->dfs_use_nol_subchannel_marking) {
		/* Since subchannel marking is disabled, disregard
		 * NOL IE and set NOL IE flag as false, so it
		 * can't be sent to uplink.
		 */
		num_subchans =
			dfs_get_bonding_channels(dfs,
						 dfs->dfs_curchan,
						 dfs->dfs_curchan->dfs_ch_freq,
						 radar_subchans);
		should_nol_ie_be_sent = false;
	} else {
		/* Add the NOL IE information in DFS structure so that RCSA
		 * and NOL IE can be sent to uplink if uplink exists.
		 */
		uint32_t frequency = (uint32_t)nol_ie_startfreq;

		dfs->dfs_nol_ie_bandwidth = nol_ie_bandwidth;
		dfs->dfs_nol_ie_startfreq = nol_ie_startfreq;
		dfs->dfs_nol_ie_bitmap = nol_ie_bitmap;
		for (num_subchans = 0; num_subchans < NUM_CHANNELS_160MHZ;
			num_subchans++) {
			if (nol_ie_bitmap & bits) {
				radar_subchans[num_subchans] =
					utils_dfs_freq_to_chan(frequency);
			}
			bits <<= 1;
			frequency += nol_ie_bandwidth;
		}
	}

	dfs_radar_add_channel_list_to_nol(dfs, radar_subchans, num_subchans);
	return should_nol_ie_be_sent;
}

QDF_STATUS dfs_process_radar_ind(struct wlan_dfs *dfs,
				 struct radar_found_info *radar_found)
{
	bool wait_for_csa = false;
	uint8_t channels[NUM_CHANNELS_160MHZ];
	uint8_t num_channels;
	QDF_STATUS status;
	uint32_t freq_center;
	uint32_t radarfound_freq;

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

	/* For Full Offload, FW sends segment id,freq_offset and chirp
	 * information and gets assigned when there is radar detect. In
	 * case of radartool bangradar enhanced command and real radar
	 * for DA and PO, we assign these information here.
	 */
	if (!(dfs->dfs_is_offload_enabled && dfs->dfs_radar_found_for_fo)) {
		radar_found->segment_id = dfs->dfs_seg_id;
		radar_found->freq_offset = dfs->dfs_freq_offset;
		radar_found->is_chirp = dfs->dfs_is_chirp;
	}

	dfs_compute_radar_found_cfreq(dfs, radar_found, &freq_center);
	radarfound_freq = freq_center + radar_found->freq_offset;

	if (radar_found->segment_id == SEG_ID_SECONDARY)
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Radar found on second segment.Radarfound Freq=%d MHz.Secondary Chan cfreq=%d MHz.",
			 radarfound_freq, freq_center);
	else
		dfs_info(NULL, WLAN_DEBUG_DFS_ALWAYS,
			 "Radar found on channel=%d, freq=%d MHz. Primary beaconning chan:%d, freq=%d MHz.",
			 utils_dfs_freq_to_chan(radarfound_freq),
			 radarfound_freq, dfs->dfs_curchan->dfs_ch_ieee,
			 dfs->dfs_curchan->dfs_ch_freq);

	utils_dfs_deliver_event(dfs->dfs_pdev_obj,
				radarfound_freq,
				WLAN_EV_RADAR_DETECTED);

	if (!dfs->dfs_use_nol) {
		dfs_reset_bangradar(dfs);
		dfs_send_csa_to_current_chan(dfs);
		return QDF_STATUS_SUCCESS;
	}

	if (dfs->dfs_bangradar_type == DFS_BANGRADAR_FOR_ALL_SUBCHANS)
		num_channels = dfs_get_bonding_channels_without_seg_info(
				 dfs->dfs_curchan, channels);
	/* BW reduction is dependent on subchannel marking */
	else if ((dfs->dfs_use_nol_subchannel_marking) &&
		 (!(dfs->dfs_bangradar_type) ||
		 (dfs->dfs_bangradar_type ==
		 DFS_BANGRADAR_FOR_SPECIFIC_SUBCHANS)))
		num_channels = dfs_find_radar_affected_subchans(dfs,
								radar_found,
								channels,
								freq_center);
	else
		num_channels = dfs_get_bonding_channels(dfs,
							dfs->dfs_curchan,
							radar_found->segment_id,
							channels);

	dfs_reset_bangradar(dfs);

	if (dfs->dfs_agile_precac_enable && radar_found->detector_id ==
			AGILE_DETECTOR_ID) {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "%s: %d Radar found on agile detector:%d , STAY in Same operating Channel",
			  __func__, __LINE__, radar_found->detector_id);
		dfs_mark_precac_dfs(dfs, dfs->is_radar_found_on_secondary_seg,
				    radar_found->detector_id);
		return QDF_STATUS_SUCCESS;
	}

	status = dfs_radar_add_channel_list_to_nol(dfs, channels, num_channels);
	if (QDF_IS_STATUS_ERROR(status)) {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"radar event received on invalid channel");
		return status;
	}
	dfs->dfs_is_nol_ie_sent = false;
	(dfs->is_radar_during_precac) ?
		(dfs->dfs_is_rcsa_ie_sent = false) :
		(dfs->dfs_is_rcsa_ie_sent = true);
	if (dfs->dfs_use_nol_subchannel_marking) {
		dfs_reset_nol_ie_bitmap(dfs);
		dfs_prepare_nol_ie_bitmap(dfs, radar_found, channels,
					  num_channels);
		dfs->dfs_is_nol_ie_sent = true;
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

	if (dfs->dfs_precac_enable || dfs->dfs_agile_precac_enable) {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "%s: %d Radar found on dfs detector:%d",
			  __func__, __LINE__, radar_found->detector_id);
		dfs_mark_precac_dfs(dfs,
				    dfs->is_radar_found_on_secondary_seg,
				    radar_found->detector_id);
	}

	if (utils_get_dfsdomain(dfs->dfs_pdev_obj) == DFS_ETSI_DOMAIN) {
		/* Remove chan from ETSI Pre-CAC Cleared List*/
		dfs_info(dfs, WLAN_DEBUG_DFS_NOL,
			 "%s : %d remove channel from ETSI PreCAC List\n",
			 __func__, __LINE__);
		dfs_mark_etsi_precac_dfs(dfs, channels, num_channels);
	}
	/*
	 * This calls into the umac DFS code, which sets the umac
	 * related radar flags and begins the channel change
	 * machinery.

	 * Even during precac, this API is called, but with a flag
	 * saying not to send RCSA, but only the radar affected subchannel
	 * information.
	 */

	dfs_mlme_start_rcsa(dfs->dfs_pdev_obj, &wait_for_csa);

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
	 * XXX TODO: the umac NOL code isn't used, but
	 * WLAN_CHAN_DFS_RADAR still gets set. Since the umac
	 * NOL code isn't used, that flag is never cleared. This
	 * needs to be fixed. See EV 105776.
	 */
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
