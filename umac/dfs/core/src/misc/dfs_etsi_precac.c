/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
 * Copyright (c) 2007-2008 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * DOC: This file has ETSI CAC DFS functions.
 * Summary: In ETSI domain, after CAC, channel is added to ETSI CAC cleared
 * list, so that next time it can skip CAC if channel already present in this
 * list. If radar found, remove channel from this list indicating to redo
 * CAC,when channel is selected in future.
 */

#include "dfs_etsi_precac.h"
#include "wlan_dfs_lmac_api.h"
#include "wlan_dfs_mlme_api.h"
#include "wlan_dfs_utils_api.h"
#include "dfs_internal.h"
#include "dfs_process_radar_found_ind.h"

#define ETSI_CAC_TIME_OUT_MS 86400000
int dfs_is_subchan_in_etsi_precac_done_list(struct wlan_dfs *dfs,
					    uint8_t channel)
{
	struct dfs_etsi_precac_entry *precac_entry = NULL;
	struct dfs_etsi_precac_entry *tmp_precac_entry = NULL;
	uint8_t found = 0;
	uint32_t diff_ms;

	if (utils_is_dfs_ch(dfs->dfs_pdev_obj, channel)) {
		/*For each channel in the list here do the below operation*/
		PRECAC_LIST_LOCK(dfs);
		if (TAILQ_EMPTY(&dfs->dfs_etsiprecac_done_list)) {
			PRECAC_LIST_UNLOCK(dfs);
			return 0;
		}

		TAILQ_FOREACH_SAFE(precac_entry, &dfs->dfs_etsiprecac_done_list,
				   pe_list, tmp_precac_entry) {
			diff_ms = qdf_system_ticks_to_msecs(qdf_system_ticks() -
					precac_entry->etsi_caclst_ticks);
			if (channel == precac_entry->ieee &&
			    diff_ms < ETSI_CAC_TIME_OUT_MS) {
				found = 1;
				break;
			}
		}
		PRECAC_LIST_UNLOCK(dfs);
		if (found)
			return 1;
	} else {
		return 1;
	}

	return 0;
}

bool dfs_is_etsi_precac_done(struct wlan_dfs *dfs)
{
	bool ret_val = 1;
	uint8_t channels[NUM_CHANNELS_160MHZ];
	uint8_t nchannels = 0;
	int i;

	nchannels = dfs_get_bonding_channels_without_seg_info(dfs->dfs_curchan,
							      channels);

	for (i = 0; i < nchannels; i++) {
		if (dfs_is_subchan_in_etsi_precac_done_list(dfs, channels[i])) {
			continue;
		} else {
			ret_val = 0;
			break;
		}
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS, "ret_val = %d", ret_val);

	return ret_val;
}

void dfs_mark_etsi_precac_dfs(struct wlan_dfs *dfs, uint8_t *channels,
			      uint8_t num_channels)
{
	struct dfs_etsi_precac_entry *precac_entry = NULL;
	struct dfs_etsi_precac_entry *tmp_precac_entry = NULL;
	int i = 0;

	if (num_channels > NUM_CHANNELS_160MHZ) {
		dfs_err(dfs, WLAN_DEBUG_DFS,
			"Invalid num channels: %d", num_channels);
		return;
	}

	/*
	 * Here, the radar affected channels are only removed from etsi-precac
	 * done list and etsi precac required list, but not added to NOL.
	 * They are added to NOL by the dfs_radar_add_channel_list_to_nol()
	 * function.
	 */
	PRECAC_LIST_LOCK(dfs);
	for (i = 0; i < num_channels; i++) {
		TAILQ_FOREACH_SAFE(precac_entry, &dfs->dfs_etsiprecac_done_list,
				   pe_list, tmp_precac_entry) {
			if (channels[i] != precac_entry->ieee)
				continue;

		TAILQ_REMOVE(&dfs->dfs_etsiprecac_done_list,
			     precac_entry, pe_list);
		qdf_mem_free(precac_entry);
		}
		TAILQ_FOREACH_SAFE(precac_entry,
				   &dfs->dfs_etsiprecac_required_list,
				   pe_list, tmp_precac_entry) {
					if (channels[i] != precac_entry->ieee)
						continue;
		TAILQ_REMOVE(&dfs->dfs_etsiprecac_required_list,
			     precac_entry, pe_list);
		qdf_mem_free(precac_entry);
		}
	}
	PRECAC_LIST_UNLOCK(dfs);
}

void dfs_init_etsi_precac_list(struct wlan_dfs *dfs)
{
	u_int i;
	uint8_t found;
	struct dfs_etsi_precac_entry *tmp_precac_entry;
	int nchans = 0;
	struct dfs_channel *chan_list = NULL;

	/*
	 * We need to prepare list of unique VHT20 center frequencies.
	 * But at the beginning we do not know how many unique frequencies
	 * are present. Therefore, we calculate the MAX size and allocate
	 * a temporary list/array. However we fill the temporary array with
	 * unique frequencies and copy the unique list of frequencies to
	 * the final list with exact size.
	 */
	TAILQ_INIT(&dfs->dfs_etsiprecac_done_list);
	TAILQ_INIT(&dfs->dfs_etsiprecac_required_list);

	nchans = dfs_get_num_chans();

	chan_list = qdf_mem_malloc(nchans * sizeof(*chan_list));
	if (!chan_list)
		return;

	utils_dfs_get_chan_list(dfs->dfs_pdev_obj, (void *)chan_list, &nchans);
	if (!nchans) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "zero channels");
		qdf_mem_free(chan_list);
		return;
	}

	PRECAC_LIST_LOCK(dfs);
	for (i = 0; i < nchans; i++) {
		if (!WLAN_IS_CHAN_DFS(&chan_list[i]))
			continue;

		found = 0;
		TAILQ_FOREACH(tmp_precac_entry,
			      &dfs->dfs_etsiprecac_required_list,
			      pe_list) {
			if (tmp_precac_entry->ieee ==
					chan_list[i].dfs_ch_ieee) {
				found = 1;
				break;
			}
		}
		if (!found) {
			struct dfs_etsi_precac_entry *etsi_precac_entry;

			etsi_precac_entry = qdf_mem_malloc(
					sizeof(*etsi_precac_entry));
			if (!etsi_precac_entry) {
				dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
					"etsi precac entry alloc fail");
				continue;
			}
			etsi_precac_entry->ieee = chan_list[i].dfs_ch_ieee;
			etsi_precac_entry->dfs = dfs;
			TAILQ_INSERT_TAIL(&dfs->dfs_etsiprecac_required_list,
					  etsi_precac_entry, pe_list);
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "Print the list of VHT20 frequencies from linked list");
	TAILQ_FOREACH(tmp_precac_entry,
		      &dfs->dfs_etsiprecac_required_list,
			pe_list)
	dfs_info(dfs, WLAN_DEBUG_DFS, "ieee=%u", tmp_precac_entry->ieee);

	qdf_mem_free(chan_list);
}

void dfs_deinit_etsi_precac_list(struct wlan_dfs *dfs)
{
	struct dfs_etsi_precac_entry *tmp_precac_entry, *precac_entry;

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "Free list of HT20/VHT20 freq from etsiprecac_required list");
	PRECAC_LIST_LOCK(dfs);

	if (!TAILQ_EMPTY(&dfs->dfs_etsiprecac_required_list))
		TAILQ_FOREACH_SAFE(precac_entry,
				   &dfs->dfs_etsiprecac_required_list,
				pe_list,
				tmp_precac_entry) {
			TAILQ_REMOVE(&dfs->dfs_etsiprecac_required_list,
				     precac_entry, pe_list);
			qdf_mem_free(precac_entry);
		}

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "Free list of HT20/VHT20 freq from etsipcac_done list");

	if (!TAILQ_EMPTY(&dfs->dfs_etsiprecac_done_list))
		TAILQ_FOREACH_SAFE(precac_entry,
				   &dfs->dfs_etsiprecac_done_list,
				pe_list,
				tmp_precac_entry) {
			TAILQ_REMOVE(&dfs->dfs_etsiprecac_done_list,
				     precac_entry, pe_list);
			qdf_mem_free(precac_entry);
		}

	PRECAC_LIST_UNLOCK(dfs);
}

void dfs_etsi_precac_attach(struct wlan_dfs *dfs)
{
}

void dfs_etsi_precac_detach(struct wlan_dfs *dfs)
{
	dfs_deinit_etsi_precac_list(dfs);
}

void dfs_print_etsi_precaclists(struct wlan_dfs *dfs)
{
	struct dfs_etsi_precac_entry *tmp_precac_entry;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	PRECAC_LIST_LOCK(dfs);
	/* Print the ETSI Pre-CAC required List */
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "Pre-cac-required list of VHT20 frequencies");
	TAILQ_FOREACH(tmp_precac_entry,
		      &dfs->dfs_etsiprecac_required_list,
			pe_list) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "ieee=%u ", tmp_precac_entry->ieee);
	}

	/* Print the ETSI Pre-CAC done List */
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "Pre-cac-done list of VHT20 frequencies");
	TAILQ_FOREACH(tmp_precac_entry,
		      &dfs->dfs_etsiprecac_done_list,
			pe_list) {
		unsigned long time_added =
			qdf_system_ticks_to_msecs(
					tmp_precac_entry->etsi_caclst_ticks);

		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "ieee=%u added at (msec): %lu", tmp_precac_entry->ieee,
			 time_added);
	}
	PRECAC_LIST_UNLOCK(dfs);
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "NOL list of VHT20 frequencies");
	DFS_PRINT_NOL_LOCKED(dfs);
}

void dfs_reset_etsiprecaclists(struct wlan_dfs *dfs)
{
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "Reset precaclist of VHT80 frequencies");
	dfs_deinit_etsi_precac_list(dfs);
	dfs_init_etsi_precac_list(dfs);
}

void dfs_reset_etsi_precac_lists(struct wlan_dfs *dfs)
{
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}
	dfs_reset_etsiprecaclists(dfs);
}

void dfs_add_to_etsi_precac_required_list(struct wlan_dfs *dfs, uint8_t *chan)
{
	struct dfs_etsi_precac_entry *etsi_precac_entry;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	if (utils_get_dfsdomain(dfs->dfs_pdev_obj) == DFS_ETSI_REGION &&
	    dfs->dfs_precac_enable) {
		etsi_precac_entry = qdf_mem_malloc(sizeof(*etsi_precac_entry));

		if (!etsi_precac_entry) {
			dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"etsi_precac_entry is NULL");
			return;
		}
		etsi_precac_entry->ieee = *chan;
		etsi_precac_entry->dfs = dfs;
		PRECAC_LIST_LOCK(dfs);
		TAILQ_INSERT_TAIL(&dfs->dfs_etsiprecac_required_list,
				  etsi_precac_entry,
				  pe_list);
		PRECAC_LIST_UNLOCK(dfs);
	}
}

int dfs_add_chan_to_etsi_done_list(struct wlan_dfs *dfs, uint8_t channel)
{
	struct dfs_etsi_precac_entry *precac_entry, *tmp_precac_entry;

	PRECAC_LIST_LOCK(dfs);
	TAILQ_FOREACH_SAFE(precac_entry,
			   &dfs->dfs_etsiprecac_required_list,
			   pe_list, tmp_precac_entry) {
		if (channel == precac_entry->ieee) {
			TAILQ_REMOVE(&dfs->dfs_etsiprecac_required_list,
				     precac_entry, pe_list);
			TAILQ_INSERT_TAIL(&dfs->dfs_etsiprecac_done_list,
					  precac_entry,
					  pe_list);
			precac_entry->etsi_caclst_ticks = qdf_system_ticks();
			PRECAC_LIST_UNLOCK(dfs);
			return 1;
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	return 0;
}

void dfs_add_to_etsi_precac_done_list(struct wlan_dfs *dfs)
{
	uint8_t channels[NUM_CHANNELS_160MHZ];
	uint8_t nchannels = 0;
	int i = 0;

	nchannels = dfs_get_bonding_channels_without_seg_info(dfs->dfs_curchan,
							      channels);
	for (i = 0; i < nchannels; i++) {
		if (!utils_is_dfs_ch(dfs->dfs_pdev_obj, channels[i]))
			continue;

		if (TAILQ_EMPTY(&dfs->dfs_etsiprecac_required_list))
			break;

		dfs_add_chan_to_etsi_done_list(dfs, channels[i]);
	}
}
