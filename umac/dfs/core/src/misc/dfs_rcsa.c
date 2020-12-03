/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: This file has DFS_RCSA Functions.
 *
 */

#include <dfs.h>
#include <dfs_process_radar_found_ind.h>
#include <wlan_dfs_mlme_api.h>

#if defined(QCA_DFS_RCSA_SUPPORT)
/* dfs_prepare_nol_ie_bitmap: Create a Bitmap from the radar found subchannels
 * to be sent along with RCSA.
 * @dfs: Pointer to wlan_dfs.
 * @radar_found: Pointer to radar_found_info.
 * @in_sub_channels: Pointer to Sub-channels.
 * @n_in_sub_channels: Number of sub-channels.
 */
#ifdef CONFIG_CHAN_FREQ_API
static void
dfs_prepare_nol_ie_bitmap_for_freq(struct wlan_dfs *dfs,
				   struct radar_found_info *radar_found,
				   uint16_t *in_sub_channels,
				   uint8_t n_in_sub_channels)
{
	uint16_t cur_subchans[NUM_CHANNELS_160MHZ];
	uint8_t n_cur_subchans;
	uint8_t i;
	uint8_t j;
	uint8_t bits = 0x01;

	n_cur_subchans =
	    dfs_get_bonding_channels_for_freq(dfs, dfs->dfs_curchan,
					      radar_found->segment_id,
					      radar_found->detector_id,
					      cur_subchans);
	dfs->dfs_nol_ie_bandwidth = MIN_DFS_SUBCHAN_BW;
	dfs->dfs_nol_ie_startfreq = cur_subchans[0];

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
#endif

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

#ifdef CONFIG_CHAN_FREQ_API
bool dfs_process_nol_ie_bitmap(struct wlan_dfs *dfs, uint8_t nol_ie_bandwidth,
			       uint16_t nol_ie_startfreq, uint8_t nol_ie_bitmap)
{
	uint8_t num_subchans;
	uint8_t bits = 0x01;
	uint16_t radar_subchans[NUM_CHANNELS_160MHZ];
	uint16_t nol_freq_list[NUM_CHANNELS_160MHZ];
	bool should_nol_ie_be_sent = true;

	qdf_mem_zero(radar_subchans, sizeof(radar_subchans));
	if (!dfs->dfs_use_nol_subchannel_marking) {
		/* Since subchannel marking is disabled, disregard
		 * NOL IE and set NOL IE flag as false, so it
		 * can't be sent to uplink.
		 */
		num_subchans =
		    dfs_get_bonding_channels_for_freq(dfs,
						      dfs->dfs_curchan,
						      SEG_ID_PRIMARY,
						      DETECTOR_ID_0,
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
				radar_subchans[num_subchans] = frequency;
			}
			bits <<= 1;
			frequency += nol_ie_bandwidth;
		}
	}

	dfs_radar_add_channel_list_to_nol_for_freq(dfs, radar_subchans,
						   nol_freq_list,
						   &num_subchans);
	return should_nol_ie_be_sent;
}
#endif
#endif /* QCA_DFS_RCSA_SUPPORT */

#if defined(QCA_DFS_RCSA_SUPPORT)
/**
 * dfs_send_nol_ie_and_rcsa()- Send NOL IE and RCSA action frames.
 * @dfs: Pointer to wlan_dfs structure.
 * @radar_found: Pointer to radar found structure.
 * @nol_freq_list: List of 20MHz frequencies on which radar has been detected.
 * @num_channels: number of radar affected channels.
 * @wait_for_csa: indicates if the repeater AP should take DFS action or wait
 * for CSA
 *
 * Return: void.
 */
void dfs_send_nol_ie_and_rcsa(struct wlan_dfs *dfs,
			      struct radar_found_info *radar_found,
			      uint16_t *nol_freq_list,
			      uint8_t num_channels,
			      bool *wait_for_csa)
{
	dfs->dfs_is_nol_ie_sent = false;
	(dfs->is_radar_during_precac ||
	 radar_found->detector_id == dfs_get_agile_detector_id(dfs)) ?
		(dfs->dfs_is_rcsa_ie_sent = false) :
		(dfs->dfs_is_rcsa_ie_sent = true);
	if (dfs->dfs_use_nol_subchannel_marking) {
		dfs_reset_nol_ie_bitmap(dfs);
		dfs_prepare_nol_ie_bitmap_for_freq(dfs, radar_found,
						   nol_freq_list,
						   num_channels);
		dfs->dfs_is_nol_ie_sent = true;
	}

	/*
	 * This calls into the umac DFS code, which sets the umac
	 * related radar flags and begins the channel change
	 * machinery.

	 * Even during precac, this API is called, but with a flag
	 * saying not to send RCSA, but only the radar affected subchannel
	 * information.
	 */
	dfs_mlme_start_rcsa(dfs->dfs_pdev_obj, wait_for_csa);
}
#endif /* QCA_DFS_RCSA_SUPPORT */

