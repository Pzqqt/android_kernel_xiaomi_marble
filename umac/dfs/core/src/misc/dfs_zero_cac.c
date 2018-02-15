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
 * DOC: This file has ZERO CAC DFS functions.
 * Abstract:- Operation in a DFS channel requires CAC that adds additional
 * delay as well as loss of connection even when CSA is used. ETSI allows
 * pre-CAC, i.e. performing CAC at a convenient time and using that channel
 * later. Once Pre-CAC is done in a channel, it is no longer required to
 * perform a CAC in the channel before TX/RX as long as radar is not found in
 * it or we reset or restart the device.
 *
 * Design:-
 * The pre-CAC is done in a RADIO that has  VHT80_80 capable radio where the
 * primary and secondary HT80s can be programmed independently with two
 * different HT80 channels. Three new lists are introduced to handle pre-CAC.
 * The lists are:
 * 1)Pre-CAC-required list
 * 2)Pre-CAC-done list
 * 3)Pre-CAC-NOL list
 * At the beginning the Pre-CAC-required list is populated with the unique
 * secondary HT80 frequencies of HT80_80 channels.  Whenever a HT80 channel
 * change happens we convert the HT80 channel change to a HT80_80 channel and
 * the secondary HT80 is set to the first element(HT80 frequency) from the
 * Pre-CAC-required list.  Pre-CAC period is same same as the CAC period. After
 * the pre-CAC period is over the pre-CAC timer expires and the HT80 frequency
 * (the pre-CAC-required list element) is removed from the Pre-CAC-required
 * list and inserted into the Pre-CAC-done list. While Pre-CAC timer is running
 * if there is any RADAR detect then the current HT80 frequency is removed from
 * the Pre-CAC-required list and inserted into the Pre-CAC-NOL list. Each
 * element of pre-CAC-NOL times out individually after 30 minutes of its
 * insertion. Pre-CAC-NOL timeout is just like the regular NOL timeout. Upon
 * Pre-CAC-NOL expiry of an element (HT80 frequency), the element is removed
 * from the Pre-CAC-NOL list and inserted into the Pre-CAC-required list.
 * At any point of time if there is a channel change and the new channel is
 * DFS, the Pre-CAC-done list is consulted to check if pre-CAC has been
 * completed for the entire bandwidth of the new channel. If Pre-CAC has
 * already been done  for the entire bandwidth of the channel then regular CAC
 * can be skipped(this is what is known as Zero wait DFS) if we are in ETSI
 * domain.
 *
 * New RadarTool commands:-
 * 1)radartool -i wifi[X] secondSegmentBangradar
 * It simulates RADAR from the secondary HT80 when the
 * secondary HT80 is doing pre-CAC. If secondary is not
 * doing any pre-CAC then this command has no effect.
 * 2)radartool -i wifi[X] showPreCACLists
 * It shows all 3 pre-CAC Lists' contents.
 *
 * New iwpriv commands:-
 * 1)iwpriv wifi[X] preCACEn 0/1
 * This command enables/disables the zero-cac-DFS.
 * 2)iwpriv wifi[X] pCACTimeout <timeout>
 * Override the pCACTimeout.
 *
 * FAQ(Frequently Asked Questions):-
 * 1)
 * Question)We already have NOL list. Why do we need separate pre-CAC-NOL
 * list?
 * Answer) pre-CAC is done on an HT80 channel and  the same HT80 channel is
 * inserted into pre-CAC-NOL list after pre-CAC radar detection. NOL list
 * contains HT20 channels. Since after pre-CAC-NOL expiry we need
 * to move the HT80 channel from pre-CAC-NOL list  to pre-CAC-required list
 * it is very easy to remove the HT80 channel and insert it. Having
 * a separate pre-CAC-NOL also provides some separation from the existing
 * code and helps modularize.
 */

#include "../dfs_zero_cac.h"
#include "wlan_dfs_lmac_api.h"
#include "wlan_dfs_mlme_api.h"
#include "wlan_dfs_utils_api.h"
#include "../dfs_internal.h"

void dfs_zero_cac_reset(struct wlan_dfs *dfs)
{
	struct dfs_precac_entry *tmp_precac_entry, *precac_entry;

	dfs_get_override_precac_timeout(dfs,
			&(dfs->dfs_precac_timeout_override));
	qdf_timer_stop(&dfs->dfs_precac_timer);
	dfs->dfs_precac_primary_freq = 0;
	dfs->dfs_precac_secondary_freq = 0;

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_nol_list))
		TAILQ_FOREACH_SAFE(precac_entry,
				&dfs->dfs_precac_nol_list,
				pe_list,
				tmp_precac_entry) {
			qdf_timer_stop(&precac_entry->precac_nol_timer);
			TAILQ_REMOVE(&dfs->dfs_precac_required_list,
					precac_entry, pe_list);
			qdf_mem_free(precac_entry);
		}
	PRECAC_LIST_UNLOCK(dfs);
}

int dfs_override_precac_timeout(struct wlan_dfs *dfs, int precac_timeout)
{
	if (!dfs)
		return -EIO;

	dfs->dfs_precac_timeout_override = precac_timeout;
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "PreCAC timeout is now %s (%d)",
		(precac_timeout == -1) ? "default" : "overridden",
		precac_timeout);

	return 0;
}

int dfs_get_override_precac_timeout(struct wlan_dfs *dfs, int *precac_timeout)
{
	if (!dfs)
		return -EIO;

	(*precac_timeout) = dfs->dfs_precac_timeout_override;

	return 0;
}

#define VHT80_OFFSET 6
#define IS_WITHIN_RANGE(_A, _B, _C)  \
	(((_A) >= ((_B)-(_C))) && ((_A) <= ((_B)+(_C))))

bool dfs_is_ht20_40_80_chan_in_precac_done_list(struct wlan_dfs *dfs)
{
	struct dfs_precac_entry *precac_entry;
	bool ret_val = 0;

	/*
	 * A is within B-C and B+C
	 * (B-C) <= A <= (B+C)
	 */
	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_done_list))
		TAILQ_FOREACH(precac_entry,
				&dfs->dfs_precac_done_list,
				pe_list) {
			/* Find if the VHT80 freq1 is in Pre-CAC done list */
			if (IS_WITHIN_RANGE(dfs->dfs_curchan->dfs_ch_ieee,
						precac_entry->vht80_freq,
						VHT80_OFFSET)) {
				ret_val = 1;
				break;
			}
		}
	PRECAC_LIST_UNLOCK(dfs);

	dfs_debug(dfs, WLAN_DEBUG_DFS, "vht80_freq = %u ret_val = %d",
		 dfs->dfs_curchan->dfs_ch_ieee, ret_val);

	return ret_val;
}

bool dfs_is_ht80_80_chan_in_precac_done_list(struct wlan_dfs *dfs)
{
	struct dfs_precac_entry *precac_entry;
	bool ret_val = 0;

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_done_list)) {
		bool primary_found = 0;
		/* Check if primary is DFS then search */
		if (WLAN_IS_CHAN_DFS(dfs->dfs_curchan)) {
			TAILQ_FOREACH(precac_entry,
					&dfs->dfs_precac_done_list,
					pe_list) {
				if (dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg1
						== precac_entry->vht80_freq) {
					primary_found = 1;
					break;
				}
			}
		} else {
			primary_found = 1;
		}

		/* Check if secondary DFS then search */
		if (WLAN_IS_CHAN_DFS_CFREQ2(dfs->dfs_curchan) &&
				primary_found) {
			TAILQ_FOREACH(precac_entry,
					&dfs->dfs_precac_done_list,
					pe_list) {
				if (dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg2
						== precac_entry->vht80_freq) {
					/* Now secondary also found */
					ret_val = 1;
					break;
				}
			}
		} else {
			if (primary_found)
				ret_val = 1;
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"freq_seg1 = %u freq_seq2 = %u ret_val = %d",
		dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg1,
		dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg2,
		ret_val);

	return ret_val;
}

bool dfs_is_precac_done(struct wlan_dfs *dfs)
{
	bool ret_val = 0;

	if (WLAN_IS_CHAN_11AC_VHT20(dfs->dfs_curchan) ||
			WLAN_IS_CHAN_11AC_VHT40(dfs->dfs_curchan) ||
			WLAN_IS_CHAN_11AC_VHT80(dfs->dfs_curchan)) {
		ret_val = dfs_is_ht20_40_80_chan_in_precac_done_list(dfs);
	} else if (WLAN_IS_CHAN_11AC_VHT80_80(dfs->dfs_curchan) ||
			WLAN_IS_CHAN_11AC_VHT160(dfs->dfs_curchan)) {
		ret_val = dfs_is_ht80_80_chan_in_precac_done_list(dfs);
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS, "ret_val = %d", ret_val);

	return ret_val;
}

#define VHT80_IEEE_FREQ_OFFSET 6

void dfs_mark_precac_dfs(struct wlan_dfs *dfs,
		uint8_t is_radar_found_on_secondary_seg)
{
	struct dfs_precac_entry *precac_entry = NULL, *tmp_precac_entry = NULL;
	uint8_t found = 0;

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"is_radar_found_on_secondary_seg = %u secondary_freq = %u primary_freq = %u",
		is_radar_found_on_secondary_seg,
		dfs->dfs_precac_secondary_freq,
		dfs->dfs_precac_primary_freq);

	/*
	 * Even if radar found on primary, we need to move the channel from
	 * precac-required-list and precac-done-list to precac-nol-list.
	 */
	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_required_list)) {
		TAILQ_FOREACH_SAFE(precac_entry,
				&dfs->dfs_precac_required_list,
				pe_list,
				tmp_precac_entry) {
			/*
			 * If on primary then use IS_WITHIN_RANGE else use
			 * equality directly.
			 */
			if (is_radar_found_on_secondary_seg ?
				(dfs->dfs_precac_secondary_freq ==
				 precac_entry->vht80_freq) : IS_WITHIN_RANGE(
				     dfs->dfs_curchan->dfs_ch_ieee,
				     precac_entry->vht80_freq,
				     VHT80_IEEE_FREQ_OFFSET)) {
				TAILQ_REMOVE(&dfs->dfs_precac_required_list,
						precac_entry, pe_list);

				dfs_debug(dfs, WLAN_DEBUG_DFS,
					"removing the freq = %u from required list and adding to NOL list",
					precac_entry->vht80_freq);
				TAILQ_INSERT_TAIL(&dfs->dfs_precac_nol_list,
						precac_entry, pe_list);
				qdf_timer_mod(&precac_entry->precac_nol_timer,
						dfs_get_nol_timeout(dfs)*1000);
				found = 1;
				break;
			}
		}
	}

	/* If not found in precac-required-list remove from precac-done-list */
	if (!found && !TAILQ_EMPTY(&dfs->dfs_precac_done_list)) {
		TAILQ_FOREACH_SAFE(precac_entry,
				&dfs->dfs_precac_done_list,
				pe_list,
				tmp_precac_entry) {
			/*
			 * If on primary then use IS_WITHIN_RANGE else use
			 * equality directly.
			 */
			if (is_radar_found_on_secondary_seg ?
					(dfs->dfs_precac_secondary_freq ==
					 precac_entry->vht80_freq) :
					IS_WITHIN_RANGE(
						dfs->dfs_curchan->dfs_ch_ieee,
						precac_entry->vht80_freq, 6)) {
				TAILQ_REMOVE(&dfs->dfs_precac_done_list,
					precac_entry, pe_list);

				dfs_debug(dfs, WLAN_DEBUG_DFS,
					"removing the the freq = %u from done list and adding to NOL list",
					precac_entry->vht80_freq);
				TAILQ_INSERT_TAIL(&dfs->dfs_precac_nol_list,
						precac_entry, pe_list);
				qdf_timer_mod(&precac_entry->precac_nol_timer,
						dfs_get_nol_timeout(dfs)*1000);
				break;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	/* TODO xxx:- Need to lock the channel change */
	/*
	 * If radar Found on Primary no need to do restart VAP's channels since
	 * channel change will happen after RANDOM channel selection anyway.
	 */

	if (dfs->dfs_precac_timer_running) {
		/* Cancel the PreCAC timer */
		qdf_timer_stop(&dfs->dfs_precac_timer);
		dfs->dfs_precac_timer_running = 0;

		/*
		 * Change the channel
		 * case 1:-  No  VHT80 channel for precac is available so bring
		 * it back to VHT80
		 * case 2:-  pick a new VHT80 channel for precac
		 */
		if (is_radar_found_on_secondary_seg) {
			if (dfs_is_ap_cac_timer_running(dfs)) {
				dfs->dfs_defer_precac_channel_change = 1;
				dfs_debug(dfs, WLAN_DEBUG_DFS,
					"Primary CAC is running, defer the channel change"
					);
			} else {
				dfs_mlme_channel_change_by_precac(
						dfs->dfs_pdev_obj);
			}
		}
	}
}

bool dfs_is_precac_timer_running(struct wlan_dfs *dfs)
{
	return dfs->dfs_precac_timer_running ? true : false;
}

#define VHT80_IEEE_FREQ_OFFSET 6
void dfs_find_precac_secondary_vht80_chan(struct wlan_dfs *dfs,
		struct dfs_channel *chan)
{
	uint8_t first_primary_dfs_ch_ieee;

	first_primary_dfs_ch_ieee =
		dfs->dfs_precac_secondary_freq - VHT80_IEEE_FREQ_OFFSET;

	dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
			first_primary_dfs_ch_ieee, 0,
			WLAN_PHYMODE_11AC_VHT80,
			&(chan->dfs_ch_freq),
			&(chan->dfs_ch_flags),
			&(chan->dfs_ch_flagext),
			&(chan->dfs_ch_ieee),
			&(chan->dfs_ch_vhtop_ch_freq_seg1),
			&(chan->dfs_ch_vhtop_ch_freq_seg2));
}

/**
 * dfs_precac_timeout() - Precac timeout.
 *
 * Removes the channel from precac_required list and adds it to the
 * precac_done_list. Triggers a precac channel change.
 */
static os_timer_func(dfs_precac_timeout)
{
	struct dfs_precac_entry *precac_entry, *tmp_precac_entry;
	struct wlan_dfs *dfs = NULL;

	OS_GET_TIMER_ARG(dfs, struct wlan_dfs *);
	dfs->dfs_precac_timer_running = 0;

	/*
	 * Remove the HVT80 freq from the precac-required-list and add it to the
	 * precac-done-list
	 */

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_required_list)) {
		TAILQ_FOREACH_SAFE(precac_entry,
				&dfs->dfs_precac_required_list,
				pe_list,
				tmp_precac_entry) {
			if (dfs->dfs_precac_secondary_freq ==
					precac_entry->vht80_freq) {
				TAILQ_REMOVE(&dfs->dfs_precac_required_list,
						precac_entry, pe_list);
				dfs_debug(dfs, WLAN_DEBUG_DFS,
					"removing the the freq = %u from required list and adding to done list",
					precac_entry->vht80_freq);
				TAILQ_INSERT_TAIL(&dfs->dfs_precac_done_list,
						precac_entry, pe_list);
				break;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"Pre-cac expired, Precac Secondary chan %u curr time %d",
		dfs->dfs_precac_secondary_freq,
		(qdf_system_ticks_to_msecs(qdf_system_ticks()) / 1000));
	/* Do vdev restart so that we can change the secondary VHT80 channel. */

	/* TODO xxx : Need to lock the channel change */
	dfs_mlme_channel_change_by_precac(dfs->dfs_pdev_obj);
}

void dfs_zero_cac_timer_init(struct wlan_dfs *dfs)
{
	qdf_timer_init(NULL,
			&(dfs->dfs_precac_timer),
			dfs_precac_timeout,
			(void *) dfs,
			QDF_TIMER_TYPE_WAKE_APPS);
}

void dfs_zero_cac_attach(struct wlan_dfs *dfs)
{
	dfs->dfs_precac_timeout_override = -1;
	dfs_zero_cac_timer_init(dfs);
	PRECAC_LIST_LOCK_CREATE(dfs);
}

/**
 * dfs_precac_nol_timeout() - NOL timeout for precac channel.
 *
 * Removes the VHT80 channel from precac nol list and adds it to precac required
 * list.
 */
static os_timer_func(dfs_precac_nol_timeout)
{
	struct dfs_precac_entry *precac_entry;
	struct wlan_dfs *dfs = NULL;

	OS_GET_TIMER_ARG(precac_entry, struct dfs_precac_entry *);
	dfs = (struct wlan_dfs *)precac_entry->dfs;

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_nol_list)) {
		/* Move the channel from precac-NOL to precac-required-list */
		TAILQ_REMOVE(&dfs->dfs_precac_nol_list, precac_entry, pe_list);
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			"removing the the freq = %u from PreCAC NOL-list and adding Precac-required list",
			 precac_entry->vht80_freq);
		TAILQ_INSERT_TAIL(&dfs->dfs_precac_required_list, precac_entry,
				pe_list);
	}
	PRECAC_LIST_UNLOCK(dfs);

	/* TODO xxx : Need to lock the channel change */
	/* Do a channel change */
	dfs_mlme_channel_change_by_precac(dfs->dfs_pdev_obj);
}

void dfs_init_precac_list(struct wlan_dfs *dfs)
{
	u_int i;
	uint8_t found;
	struct dfs_precac_entry *tmp_precac_entry;
	int nchans = 0;

	/*
	 * We need to prepare list of uniq VHT80 center frequencies. But at the
	 * beginning we do not know how many uniq frequencies are present.
	 * Therefore, we calculate the MAX size and allocate a temporary
	 * list/array. However we fill the temporary array with uniq frequencies
	 * and copy the uniq list of frequencies to the final list with exact
	 * size.
	 */
	TAILQ_INIT(&dfs->dfs_precac_required_list);
	TAILQ_INIT(&dfs->dfs_precac_done_list);
	TAILQ_INIT(&dfs->dfs_precac_nol_list);
	dfs_mlme_get_dfs_ch_nchans(dfs->dfs_pdev_obj, &nchans);

	PRECAC_LIST_LOCK(dfs);
	/* Fill the  precac-required-list with unique elements */
	for (i = 0; i < nchans; i++) {
		struct dfs_channel *ichan = NULL, lc;

		ichan = &lc;
		dfs_mlme_get_dfs_ch_channels(dfs->dfs_pdev_obj,
				&(ichan->dfs_ch_freq),
				&(ichan->dfs_ch_flags),
				&(ichan->dfs_ch_flagext),
				&(ichan->dfs_ch_ieee),
				&(ichan->dfs_ch_vhtop_ch_freq_seg1),
				&(ichan->dfs_ch_vhtop_ch_freq_seg2),
				i);

		if (WLAN_IS_CHAN_11AC_VHT80(ichan) &&
				WLAN_IS_CHAN_DFS(ichan)) {
			found = 0;
			TAILQ_FOREACH(tmp_precac_entry,
					&dfs->dfs_precac_required_list,
					pe_list) {
				if (tmp_precac_entry->vht80_freq ==
						ichan->
						dfs_ch_vhtop_ch_freq_seg1) {
					found = 1;
					break;
				}
			}
			if (!found) {
				struct dfs_precac_entry *precac_entry;

				precac_entry = qdf_mem_malloc(
					sizeof(*precac_entry));
				if (!precac_entry) {
					dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
						"entry alloc fail for : %d", i);
					continue;
				}
				precac_entry->vht80_freq =
					ichan->dfs_ch_vhtop_ch_freq_seg1;
				precac_entry->dfs = dfs;

				/*
				 * Initialize per entry timer. Shall be used
				 * when the entry moves to precac_nol_list.
				 */
				qdf_timer_init(NULL,
					&(precac_entry->precac_nol_timer),
					dfs_precac_nol_timeout,
					(void *) (precac_entry),
					QDF_TIMER_TYPE_WAKE_APPS);
				TAILQ_INSERT_TAIL(
						&dfs->dfs_precac_required_list,
						precac_entry, pe_list);
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"Print the list of VHT80 frequencies from linked list");
	TAILQ_FOREACH(tmp_precac_entry,
			&dfs->dfs_precac_required_list,
			pe_list)
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "freq=%u",
				tmp_precac_entry->vht80_freq);
}

void dfs_deinit_precac_list(struct wlan_dfs *dfs)
{
	struct dfs_precac_entry *tmp_precac_entry, *precac_entry;

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"Free the list of VHT80 frequencies from linked list(precac_required)"
		);
	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_required_list))
		TAILQ_FOREACH_SAFE(precac_entry,
				&dfs->dfs_precac_required_list,
				pe_list, tmp_precac_entry) {
			TAILQ_REMOVE(&dfs->dfs_precac_required_list,
					precac_entry, pe_list);
			qdf_mem_free(precac_entry);
		}

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"Free the list of VHT80 frequencies from linked list(precac_done)"
		);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_done_list))
		TAILQ_FOREACH_SAFE(precac_entry,
				&dfs->dfs_precac_done_list,
				pe_list, tmp_precac_entry) {
			TAILQ_REMOVE(&dfs->dfs_precac_done_list,
					precac_entry, pe_list);
			qdf_mem_free(precac_entry);
		}

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"Free the list of VHT80 frequencies from linked list(precac_nol)"
		);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_nol_list))
		TAILQ_FOREACH_SAFE(precac_entry,
				&dfs->dfs_precac_nol_list,
				pe_list,
				tmp_precac_entry) {
			qdf_timer_stop(&precac_entry->precac_nol_timer);
			TAILQ_REMOVE(&dfs->dfs_precac_nol_list,
					precac_entry, pe_list);
			qdf_mem_free(precac_entry);
		}
	PRECAC_LIST_UNLOCK(dfs);

}

void dfs_zero_cac_detach(struct wlan_dfs *dfs)
{
	dfs_deinit_precac_list(dfs);
	PRECAC_LIST_LOCK_DESTROY(dfs);
}

uint8_t dfs_get_freq_from_precac_required_list(struct wlan_dfs *dfs,
		uint8_t exclude_ieee_freq)
{
	struct dfs_precac_entry *precac_entry;
	uint8_t ieee_freq = 0;

	dfs_debug(dfs, WLAN_DEBUG_DFS, "exclude_ieee_freq = %u",
		 exclude_ieee_freq);

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_required_list)) {
		TAILQ_FOREACH(precac_entry, &dfs->dfs_precac_required_list,
				pe_list) {
			if (precac_entry->vht80_freq != exclude_ieee_freq) {
				ieee_freq = precac_entry->vht80_freq;
				break;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);
	dfs_debug(dfs, WLAN_DEBUG_DFS, "ieee_freq = %u", ieee_freq);

	return ieee_freq;
}

void dfs_cancel_precac_timer(struct wlan_dfs *dfs)
{
	qdf_timer_stop(&dfs->dfs_precac_timer);
	dfs->dfs_precac_timer_running = 0;
}

void dfs_start_precac_timer(struct wlan_dfs *dfs, uint8_t precac_chan)
{
	struct dfs_channel *ichan, lc;
	uint8_t first_primary_dfs_ch_ieee;
	int primary_cac_timeout;
	int secondary_cac_timeout;
	int precac_timeout;

#define EXTRA_TIME_IN_SEC 5
	dfs->dfs_precac_timer_running = 1;

	/*
	 * Get the first primary ieee chan in the HT80 band and find the channel
	 * pointer.
	 */
	first_primary_dfs_ch_ieee = precac_chan - VHT80_IEEE_FREQ_OFFSET;

	primary_cac_timeout = dfs_mlme_get_cac_timeout(dfs->dfs_pdev_obj,
			dfs->dfs_curchan->dfs_ch_freq,
			dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg2,
			dfs->dfs_curchan->dfs_ch_flags);

	ichan = &lc;
	dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
			first_primary_dfs_ch_ieee, 0,
			WLAN_PHYMODE_11AC_VHT80,
			&(ichan->dfs_ch_freq),
			&(ichan->dfs_ch_flags),
			&(ichan->dfs_ch_flagext),
			&(ichan->dfs_ch_ieee),
			&(ichan->dfs_ch_vhtop_ch_freq_seg1),
			&(ichan->dfs_ch_vhtop_ch_freq_seg2));

	secondary_cac_timeout = (dfs->dfs_precac_timeout_override != -1) ?
		dfs->dfs_precac_timeout_override :
		dfs_mlme_get_cac_timeout(dfs->dfs_pdev_obj,
				ichan->dfs_ch_freq,
				ichan->dfs_ch_vhtop_ch_freq_seg2,
				ichan->dfs_ch_flags);

	/*
	 * EXTRA time is needed so that if CAC and PreCAC is running
	 * simultaneously, PreCAC expiry function may be called before CAC
	 * expiry and PreCAC expiry does a channel change (vdev_restart) the
	 * restart response calls CAC_start function(ieee80211_dfs_cac_start)
	 * which cancels any previous CAC timer and starts a new CAC again.
	 * So CAC expiry does not happen and moreover a new CAC is started.
	 * Therefore do not disturb the CAC by channel restart (vdev_restart).
	 */
	precac_timeout = QDF_MAX(primary_cac_timeout, secondary_cac_timeout) +
		EXTRA_TIME_IN_SEC;
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"precactimeout = %d", (precac_timeout)*1000);
	qdf_timer_mod(&dfs->dfs_precac_timer, (precac_timeout) * 1000);
}

void dfs_print_precaclists(struct wlan_dfs *dfs)
{
	struct dfs_precac_entry *tmp_precac_entry;

	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}

	PRECAC_LIST_LOCK(dfs);

	/* Print the Pre-CAC required List */
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Pre-cac-required list of VHT80 frequencies");
	TAILQ_FOREACH(tmp_precac_entry,
			&dfs->dfs_precac_required_list,
			pe_list) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"freq=%u", tmp_precac_entry->vht80_freq);
	}

	/* Print the Pre-CAC done List */
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Pre-cac-done list of VHT80 frequencies");
	TAILQ_FOREACH(tmp_precac_entry,
			&dfs->dfs_precac_done_list,
			pe_list) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "freq=%u",
			 tmp_precac_entry->vht80_freq);
	}

	/* Print the Pre-CAC NOL List */
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Pre-cac-NOL list of VHT80 frequencies");
	TAILQ_FOREACH(tmp_precac_entry,
			&dfs->dfs_precac_nol_list,
			pe_list) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				"freq=%u", tmp_precac_entry->vht80_freq);
	}

	PRECAC_LIST_UNLOCK(dfs);
}

void dfs_reset_precaclists(struct wlan_dfs *dfs)
{
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"Reset precaclist of VHT80 frequencies");
	dfs_deinit_precac_list(dfs);
	dfs_init_precac_list(dfs);
}

void dfs_reset_precac_lists(struct wlan_dfs *dfs)
{
	if (!dfs) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "dfs is NULL");
		return;
	}
	dfs_reset_precaclists(dfs);
}

void dfs_find_vht80_chan_for_precac(struct wlan_dfs *dfs,
		uint32_t chan_mode,
		uint8_t ch_freq_seg1,
		uint32_t *cfreq1,
		uint32_t *cfreq2,
		uint32_t *phy_mode,
		bool *dfs_set_cfreq2,
		bool *set_agile)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_target_tx_ops *tx_ops;
	uint32_t target_type;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	if (!psoc) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "psoc is NULL");
		return;
	}

	tx_ops = &(psoc->soc_cb.tx_ops.target_tx_ops);
	target_type = lmac_get_target_type(dfs->dfs_pdev_obj);

	if (chan_mode == WLAN_PHYMODE_11AC_VHT80) {
		/*
		 * If
		 * 1) The chip is CASCADE
		 * 2) The user phy_mode is VHT80 and
		 * 3) The user has enabled Pre-CAC and
		 * 4) The regdomain the ETSI
		 * then find a center frequency for the secondary VHT80 and
		 * Change the mode to VHT80_80 or VHT160
		 */
		uint8_t ieee_freq;

		dfs_debug(dfs, WLAN_DEBUG_DFS,
			"precac_secondary_freq = %u precac_running = %u",
			dfs->dfs_precac_secondary_freq,
			dfs->dfs_precac_timer_running);

		if (dfs->dfs_precac_enable &&
				tx_ops->tgt_is_tgt_type_qca9984(target_type) &&
				(utils_get_dfsdomain(dfs->dfs_pdev_obj) ==
				 DFS_ETSI_DOMAIN)) {
			/*
			 * If precac timer is running then do not change the
			 * secondary channel use the old secondary VHT80
			 * channel. If precac timer is not running then try to
			 * find a new channel from precac-required-list.
			 */
			if (dfs->dfs_precac_timer_running) {
				/*
				 * Primary and secondary VHT80 cannot be the
				 * same. Therefore exclude the primary
				 * frequency while getting new channel from
				 * precac-required-list.
				 */
				if (ch_freq_seg1 ==
						dfs->dfs_precac_secondary_freq)
					ieee_freq =
					dfs_get_freq_from_precac_required_list(
							dfs,
							ch_freq_seg1);
				else
					ieee_freq =
						dfs->dfs_precac_secondary_freq;
			} else
				ieee_freq =
					dfs_get_freq_from_precac_required_list(
							dfs, ch_freq_seg1);

			if (ieee_freq) {
				if (ieee_freq == (ch_freq_seg1 +
						VHT160_IEEE_FREQ_DIFF)) {
					/*
					 * Override the HW channel mode to
					 * VHT160
					 */
					uint8_t ieee_160_cfreq;

					ieee_160_cfreq =
						(ieee_freq + ch_freq_seg1)/2;
					chan_mode = WLAN_PHYMODE_11AC_VHT160;
					*cfreq1 = dfs_mlme_ieee2mhz(
							dfs->dfs_pdev_obj,
							ch_freq_seg1,
							WLAN_CHAN_5GHZ);
					*cfreq2 = dfs_mlme_ieee2mhz(
							dfs->dfs_pdev_obj,
							ieee_160_cfreq,
							WLAN_CHAN_5GHZ);
				} else {
					/*
					 * Override the HW channel mode to
					 * VHT80_80.
					 */
					chan_mode =
						WLAN_PHYMODE_11AC_VHT80_80;
					*cfreq2 = dfs_mlme_ieee2mhz(
							dfs->dfs_pdev_obj,
							ieee_freq,
							WLAN_CHAN_5GHZ);
				}
				*phy_mode = lmac_get_phymode_info(
						dfs->dfs_pdev_obj, chan_mode);
				*dfs_set_cfreq2 = true;

				/*
				 * Finally set the agile flag.
				 * When we want a full calibration of both
				 * primary VHT80 and secondary VHT80 the agile
				 * flag is set to FALSE else set to TRUE. When
				 * a channel is being set for the first time
				 * this flag must be FALSE because first time
				 * the entire channel must be calibrated. All
				 * subsequent times the flag must be set to TRUE
				 * if we are changing only the secondary VHT80.
				 */
				if (dfs->dfs_precac_primary_freq ==
						ch_freq_seg1)
					*set_agile = true;
				else
					*set_agile = false;

				dfs_debug(dfs, WLAN_DEBUG_DFS,
					"cfreq1 = %u cfreq2 = %u ieee_freq = %u mode = %u set_agile = %d",
					*cfreq1, *cfreq2, ieee_freq,
					chan_mode, *set_agile);

				dfs->dfs_precac_secondary_freq = ieee_freq;
				dfs->dfs_precac_primary_freq = ch_freq_seg1;

				/* Start the pre_cac_timer */
				dfs_start_precac_timer(dfs,
						dfs->dfs_precac_secondary_freq);
			} /* End of if(ieee_freq) */
		} /* End of if(dfs->dfs_precac_enable) */
	}
}

void dfs_set_precac_enable(struct wlan_dfs *dfs, uint32_t value)
{
	dfs->dfs_precac_enable = value;
}

uint32_t dfs_get_precac_enable(struct wlan_dfs *dfs)
{
	return dfs->dfs_precac_enable;
}
