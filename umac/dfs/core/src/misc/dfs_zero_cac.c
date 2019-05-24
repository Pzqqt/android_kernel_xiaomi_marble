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
 * Pre-CAC-required list.  Pre-CAC period is same as the CAC period. After
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

#include "dfs_zero_cac.h"
#include "wlan_dfs_lmac_api.h"
#include "wlan_dfs_mlme_api.h"
#include "wlan_dfs_utils_api.h"
#include "dfs_internal.h"
#include "dfs_etsi_precac.h"
#include "target_if.h"
#include "wlan_dfs_init_deinit_api.h"

void dfs_zero_cac_reset(struct wlan_dfs *dfs)
{
	struct dfs_precac_entry *tmp_precac_entry, *precac_entry;

	dfs_get_override_precac_timeout(dfs,
			&(dfs->dfs_precac_timeout_override));
	dfs->dfs_precac_primary_freq = 0;
	dfs->dfs_precac_secondary_freq = 0;

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_nol_list))
		TAILQ_FOREACH_SAFE(precac_entry,
				&dfs->dfs_precac_nol_list,
				pe_list,
				tmp_precac_entry) {
			qdf_timer_free(&precac_entry->precac_nol_timer);
			TAILQ_REMOVE(&dfs->dfs_precac_required_list,
				     precac_entry, pe_list);
			qdf_mem_free(precac_entry);
		}
	PRECAC_LIST_UNLOCK(dfs);
}

void dfs_zero_cac_timer_detach(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	qdf_timer_free(&dfs_soc_obj->dfs_precac_timer);
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

bool dfs_is_ht20_40_80_chan_in_precac_done_list(struct wlan_dfs *dfs,
						struct dfs_channel *chan)
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
			if (IS_WITHIN_RANGE(chan->dfs_ch_ieee,
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

bool dfs_is_ht8080_ht160_chan_in_precac_done_list(struct wlan_dfs *dfs,
						  struct dfs_channel *chan)
{
	struct dfs_precac_entry *precac_entry;
	bool ret_val = 0;
	u_int8_t sec_freq = 0;

	if (WLAN_IS_CHAN_MODE_160(chan)) {
		if (chan->dfs_ch_ieee < chan->dfs_ch_vhtop_ch_freq_seg2)
			sec_freq = chan->dfs_ch_vhtop_ch_freq_seg1 +
				   VHT160_IEEE_FREQ_DIFF;
		else
			sec_freq = chan->dfs_ch_vhtop_ch_freq_seg1 -
				   VHT160_IEEE_FREQ_DIFF;
	} else
		sec_freq = chan->dfs_ch_vhtop_ch_freq_seg2;

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_done_list)) {
		bool primary_found = 0;
		/* Check if primary is DFS then search */
		if (WLAN_IS_CHAN_DFS(chan)) {
			TAILQ_FOREACH(precac_entry,
					&dfs->dfs_precac_done_list,
					pe_list) {
				if (chan->dfs_ch_vhtop_ch_freq_seg1
						== precac_entry->vht80_freq) {
					primary_found = 1;
					break;
				}
			}
		} else {
			primary_found = 1;
		}

		/* Check if secondary DFS then search */
		if (WLAN_IS_CHAN_DFS_CFREQ2(chan) &&
		    primary_found) {
			TAILQ_FOREACH(precac_entry,
					&dfs->dfs_precac_done_list,
					pe_list) {
				if (sec_freq == precac_entry->vht80_freq) {
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
		"freq_seg1 = %u freq_seq2 = %u sec_freq = %u ret_val = %d",
		chan->dfs_ch_vhtop_ch_freq_seg1,
		chan->dfs_ch_vhtop_ch_freq_seg2,
		sec_freq,
		ret_val);

	return ret_val;
}

bool dfs_is_precac_done(struct wlan_dfs *dfs, struct dfs_channel *chan)
{
	bool ret_val = 0;

	if (WLAN_IS_CHAN_MODE_20(chan) ||
	    WLAN_IS_CHAN_MODE_40(chan) ||
	    WLAN_IS_CHAN_MODE_80(chan)) {
		ret_val = dfs_is_ht20_40_80_chan_in_precac_done_list(dfs, chan);
	} else if (WLAN_IS_CHAN_MODE_80_80(chan) ||
		   WLAN_IS_CHAN_MODE_160(chan)) {
		ret_val = dfs_is_ht8080_ht160_chan_in_precac_done_list(dfs, chan);
	}

	dfs_debug(dfs, WLAN_DEBUG_DFS, "ret_val = %d", ret_val);

	return ret_val;
}

#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_find_pdev_for_agile_precac(struct wlan_objmgr_pdev *pdev,
				    uint8_t *cur_precac_dfs_index)
{
	struct wlan_dfs *dfs;
	struct dfs_soc_priv_obj *dfs_soc_obj;
	struct wlan_objmgr_psoc *psoc;

	dfs = wlan_pdev_get_dfs_obj(pdev);
	psoc = wlan_pdev_get_psoc(pdev);

	dfs_soc_obj = dfs->dfs_soc_obj;

	*cur_precac_dfs_index =
	   (dfs_soc_obj->cur_precac_dfs_index + 1) % dfs_soc_obj->num_dfs_privs;
}

void dfs_prepare_agile_precac_chan(struct wlan_dfs *dfs)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_dfs *temp_dfs;
	struct dfs_soc_priv_obj *dfs_soc_obj;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	uint8_t ch_freq = 0;
	uint8_t cur_dfs_idx = 0;
	int i;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	dfs_soc_obj = dfs->dfs_soc_obj;

	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);

	pdev = dfs->dfs_pdev_obj;

	for (i = 0; i < dfs_soc_obj->num_dfs_privs; i++) {
		dfs_find_pdev_for_agile_precac(pdev, &cur_dfs_idx);
		dfs_soc_obj->cur_precac_dfs_index = cur_dfs_idx;
		temp_dfs = dfs_soc_obj->dfs_priv[cur_dfs_idx].dfs;
		pdev = temp_dfs->dfs_pdev_obj;
		if (!dfs_soc_obj->dfs_priv[cur_dfs_idx].agile_precac_active)
			continue;

		dfs_find_vht80_chan_for_agile_precac(temp_dfs,
						     &ch_freq,
			temp_dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg1,
			temp_dfs->dfs_curchan->dfs_ch_vhtop_ch_freq_seg2);

		if (!ch_freq) {
			qdf_info(" %s : %d No preCAC required channels left in current pdev: %pK",
				 __func__, __LINE__, pdev);
			continue;
		} else {
			break;
		}
	}

	if (ch_freq) {
		qdf_info("%s : %d ADFS channel set request sent for pdev: %pK ch_freq: %d",
			 __func__, __LINE__, pdev, ch_freq);
		if (dfs_tx_ops && dfs_tx_ops->dfs_agile_ch_cfg_cmd)
			dfs_tx_ops->dfs_agile_ch_cfg_cmd(pdev, &ch_freq);
		else
			dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
				"dfs_tx_ops=%pK", dfs_tx_ops);
	} else {
		qdf_info("No channels in preCAC required list");
	}
}
#endif

#define VHT80_IEEE_FREQ_OFFSET 6

void dfs_mark_precac_dfs(struct wlan_dfs *dfs,
		uint8_t is_radar_found_on_secondary_seg, uint8_t detector_id)
{
	struct dfs_precac_entry *precac_entry = NULL, *tmp_precac_entry = NULL;
	uint8_t found = 0;
	struct wlan_objmgr_psoc *psoc;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	dfs_soc_obj = dfs->dfs_soc_obj;

	if (is_radar_found_on_secondary_seg) {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "is_radar_found_on_secondary_seg = %u secondary_freq = %u primary_freq = %u",
			  is_radar_found_on_secondary_seg,
			  dfs->dfs_precac_secondary_freq,
			  dfs->dfs_precac_primary_freq);
	} else {
		dfs->dfs_precac_secondary_freq = dfs->dfs_agile_precac_freq;
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "agile_precac_freq = %u ",
			  dfs->dfs_agile_precac_freq);
	}

	if (detector_id != AGILE_DETECTOR_ID) {
		/*
		 * Even if radar found on primary, we need to move
		 * the channel from precac-required-list and precac-done-list
		 * to precac-nol-list.
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
					      dfs_get_nol_timeout(dfs) * 1000);
				found = 1;
				break;
				}
			}
		}

		/*
		 * If not found in precac-required-list
		 * remove from precac-done-list
		 */
		if (!found && !TAILQ_EMPTY(&dfs->dfs_precac_done_list)) {
			TAILQ_FOREACH_SAFE(precac_entry,
					   &dfs->dfs_precac_done_list,
					   pe_list,
					   tmp_precac_entry) {
				/*
				 * If on primary then use IS_WITHIN_RANGE
				 * else use equality directly.
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
						  "removing the freq = %u from done list and adding to NOL list",
						  precac_entry->vht80_freq);
					TAILQ_INSERT_TAIL(
						&dfs->dfs_precac_nol_list,
						precac_entry, pe_list);
					qdf_timer_mod(
					&precac_entry->precac_nol_timer,
					dfs_get_nol_timeout(dfs) * 1000);
					break;
				}
			}
		}
		PRECAC_LIST_UNLOCK(dfs);
	} else {
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
					  "removing the freq = %u from required list and adding to NOL list",
					  precac_entry->vht80_freq);
				TAILQ_INSERT_TAIL(&dfs->dfs_precac_nol_list,
						  precac_entry, pe_list);
				qdf_timer_mod(&precac_entry->precac_nol_timer,
					      dfs_get_nol_timeout(dfs) * 1000);
				found = 1;
				break;
				}
			}
		}

		/* If not found in precac-required-list
		 * remove from precac-done-list
		 */
		if (!found && !TAILQ_EMPTY(&dfs->dfs_precac_done_list)) {
			TAILQ_FOREACH_SAFE(precac_entry,
					   &dfs->dfs_precac_done_list,
					   pe_list,
					   tmp_precac_entry) {
				if (dfs->dfs_precac_secondary_freq ==
					 precac_entry->vht80_freq) {
					TAILQ_REMOVE(&dfs->dfs_precac_done_list,
						     precac_entry, pe_list);

					dfs_debug(dfs, WLAN_DEBUG_DFS,
						  "removing the the freq = %u from done list and adding to NOL list",
						  precac_entry->vht80_freq);
					TAILQ_INSERT_TAIL(
						&dfs->dfs_precac_nol_list,
						precac_entry, pe_list);
					qdf_timer_mod(
					&precac_entry->precac_nol_timer,
					dfs_get_nol_timeout(dfs) * 1000);
					break;
				}
			}
		}
		PRECAC_LIST_UNLOCK(dfs);
	}

	/* TO BE DONE  xxx:- Need to lock the channel change */
	/*
	 * If radar Found on Primary no need to do restart VAP's channels since
	 * channel change will happen after RANDOM channel selection anyway.
	 */

	if (dfs_soc_obj->dfs_precac_timer_running) {
		/* Cancel the PreCAC timer */
		qdf_timer_stop(&dfs_soc_obj->dfs_precac_timer);
		dfs_soc_obj->dfs_precac_timer_running = 0;

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
		} else {
			dfs_debug(dfs, WLAN_DEBUG_DFS,
				  "PreCAC timer interrupted due to RADAR, Sending Agile channel set command"
				  );
			dfs_prepare_agile_precac_chan(dfs);
		}
	}
}

#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_process_ocac_complete(struct wlan_objmgr_pdev *pdev,
			       uint32_t ocac_status,
			       uint32_t center_freq)
{
	struct wlan_dfs *dfs = NULL;

	dfs = wlan_pdev_get_dfs_obj(pdev);

	/* STOP TIMER irrespective of status */
	utils_dfs_cancel_precac_timer(pdev);
	if (ocac_status == OCAC_RESET) {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
			  "PreCAC timer reset, Sending Agile chan set command");
		dfs_prepare_agile_precac_chan(dfs);
	} else if (ocac_status == OCAC_CANCEL) {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
			  "PreCAC timer abort, agile precac stopped");
	} else if (ocac_status == OCAC_SUCCESS) {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS,
			  "PreCAC timer Completed for agile freq: %d",
			  center_freq);
		/*
		 * TRIGGER agile precac timer with 0sec timeout
		 * with ocac_status 0 for old pdev
		 */
		dfs_start_agile_precac_timer(dfs, center_freq, ocac_status);
	} else {
		dfs_debug(NULL, WLAN_DEBUG_DFS_ALWAYS, "Error Unknown");
	}
}
#endif

bool dfs_is_precac_timer_running(struct wlan_dfs *dfs)
{
	return dfs->dfs_soc_obj->dfs_precac_timer_running ? true : false;
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

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
void dfs_precac_csa(struct wlan_dfs *dfs)
{
	/*
	 * Save current chan into intermediate chan, so that
	 * next time a DFS channel needs preCAC, there is no channel switch
	 * until preCAC finishes.
	 */
	dfs->dfs_precac_inter_chan = dfs->dfs_autoswitch_des_chan;
	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "Use %d as intermediate channel for further channel changes",
		  dfs->dfs_precac_inter_chan);

	if (NULL != global_dfs_to_mlme.mlme_precac_chan_change_csa)
		global_dfs_to_mlme.mlme_precac_chan_change_csa(dfs->dfs_pdev_obj,
							       dfs->dfs_autoswitch_des_chan,
							       dfs->dfs_autoswitch_des_mode);
	dfs->dfs_autoswitch_des_chan = 0;
}
#endif

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
/**
 * dfs_precac_check_home_chan_change() - Change the home channel
 *                                       after precac is done.
 *
 * @dfs: Pointer to dfs handler.
 *
 * If precac is done on the home channel, then return true, else false.
 *
 * Return: true if precac done on home channel, else false.
 */
static bool dfs_precac_check_home_chan_change(struct wlan_dfs *dfs)
{
	struct dfs_channel chan;

	qdf_mem_zero(&chan, sizeof(struct dfs_channel));
	if (QDF_STATUS_SUCCESS !=
		dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
					    dfs->dfs_autoswitch_des_chan, 0,
					    dfs->dfs_autoswitch_des_mode,
					    &chan.dfs_ch_freq,
					    &chan.dfs_ch_flags,
					    &chan.dfs_ch_flagext,
					    &chan.dfs_ch_ieee,
					    &chan.dfs_ch_vhtop_ch_freq_seg1,
					    &chan.dfs_ch_vhtop_ch_freq_seg2)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Channel %d not found for mode %d",
			dfs->dfs_autoswitch_des_chan,
			dfs->dfs_autoswitch_des_mode);
		return false;
	}
	/*
	 * If desired channel is in precac done list,
	 * Change channel to desired channel using CSA.
	 */
	if (dfs->dfs_autoswitch_des_chan && dfs_is_precac_done(dfs, &chan)) {
		dfs_precac_csa(dfs);
		dfs->dfs_soc_obj->precac_state_started = false;
		return true;
	}
	return false;
}
#else
static inline bool dfs_precac_check_home_chan_change(struct wlan_dfs *dfs)
{
	return false;
}
#endif

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
	struct dfs_soc_priv_obj *dfs_soc_obj = NULL;
	uint32_t current_time;
	bool is_cac_done_on_des_chan = false;

	OS_GET_TIMER_ARG(dfs_soc_obj, struct dfs_soc_priv_obj *);

	dfs = dfs_soc_obj->dfs_priv[dfs_soc_obj->cur_precac_dfs_index].dfs;
	dfs_soc_obj->dfs_precac_timer_running = 0;

	if (!dfs->dfs_agile_precac_enable) {
		/*
		 * Remove the HT80 freq from the precac-required-list
		 * and add it to the precac-done-list
		 */

		PRECAC_LIST_LOCK(dfs);
		if (!TAILQ_EMPTY(&dfs->dfs_precac_required_list)) {
			TAILQ_FOREACH_SAFE(precac_entry,
					   &dfs->dfs_precac_required_list,
					   pe_list,
					   tmp_precac_entry) {
				if (dfs->dfs_precac_secondary_freq !=
						precac_entry->vht80_freq)
					continue;

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
		PRECAC_LIST_UNLOCK(dfs);

		current_time = qdf_system_ticks_to_msecs(qdf_system_ticks());
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "Pre-cac expired, Precac Secondary chan %u curr time %d",
			  dfs->dfs_precac_secondary_freq,
			  (current_time) / 1000);
		/*
		 * Do vdev restart so that we can change
		 * the secondary VHT80 channel.
		 */

		/* check if CAC done on home channel */
		is_cac_done_on_des_chan = dfs_precac_check_home_chan_change(dfs);
		if (!is_cac_done_on_des_chan) {
			/*
			 * Use same home channel, only change preCAC channel.
			 */

			/*
			 * TO BE DONE xxx : Need to lock the channel change.
			 */
			dfs_mlme_channel_change_by_precac(dfs->dfs_pdev_obj);
		}
	} else {
		current_time = qdf_system_ticks_to_msecs(qdf_system_ticks());
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Pre-cac expired, Agile Precac chan %u curr time %d",
			 dfs->dfs_agile_precac_freq,
			 current_time / 1000);
		if (dfs_soc_obj->ocac_status == OCAC_SUCCESS) {
			dfs_soc_obj->ocac_status = OCAC_RESET;
			PRECAC_LIST_LOCK(dfs);
				if (!TAILQ_EMPTY(
					&dfs->dfs_precac_required_list)) {
					TAILQ_FOREACH_SAFE(
						precac_entry,
						&dfs->dfs_precac_required_list,
						pe_list,
						tmp_precac_entry) {
					if (dfs->dfs_agile_precac_freq !=
						precac_entry->vht80_freq)
						continue;

					TAILQ_REMOVE(
						&dfs->dfs_precac_required_list,
						precac_entry, pe_list);
					dfs_info(dfs, WLAN_DEBUG_DFS,
						 "removing the freq = %u from required list and adding to done list",
						 precac_entry->vht80_freq);
					TAILQ_INSERT_TAIL(
						&dfs->dfs_precac_done_list,
						precac_entry, pe_list);
					break;
				}
			}
			PRECAC_LIST_UNLOCK(dfs);
		}
		/* check if CAC done on home channel */
		is_cac_done_on_des_chan = dfs_precac_check_home_chan_change(dfs);
		if (!is_cac_done_on_des_chan) {
			/*
			 * Use same home channel, only change preCAC channel.
			 */

			/*
			 * TO BE DONE xxx : Need to lock the channel change.
			 */
			dfs_prepare_agile_precac_chan(dfs);
		}
	}
}

void dfs_zero_cac_timer_init(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	dfs_soc_obj->precac_state_started = false;
	qdf_timer_init(NULL, &dfs_soc_obj->dfs_precac_timer,
		       dfs_precac_timeout,
		       (void *)dfs_soc_obj,
		       QDF_TIMER_TYPE_WAKE_APPS);
}

void dfs_zero_cac_attach(struct wlan_dfs *dfs)
{
	dfs->dfs_precac_timeout_override = -1;
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
		TAILQ_INSERT_TAIL(&dfs->dfs_precac_required_list,
				  precac_entry,
				  pe_list);
	}
	PRECAC_LIST_UNLOCK(dfs);

	if (!dfs->dfs_agile_precac_enable) {
		/* TO BE DONE xxx : Need to lock the channel change */
		/* Do a channel change */
		dfs_mlme_channel_change_by_precac(dfs->dfs_pdev_obj);
	} else {
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "Precac NOL timeout, sending agile channel set command"
			  );
		dfs_prepare_agile_precac_chan(dfs);
	}
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

#if defined(QCA_SUPPORT_AGILE_DFS) || defined(ATH_SUPPORT_ZERO_CAC_DFS)
void dfs_agile_soc_obj_init(struct wlan_dfs *dfs,
			    struct wlan_objmgr_psoc *psoc)
{
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							    WLAN_UMAC_COMP_DFS);
	dfs->dfs_psoc_idx = dfs_soc_obj->num_dfs_privs;
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "dfs->dfs_psoc_idx: %d ", dfs->dfs_psoc_idx);
	dfs_soc_obj->dfs_priv[dfs_soc_obj->num_dfs_privs].dfs = dfs;
	dfs_soc_obj->num_dfs_privs++;
	dfs->dfs_soc_obj = dfs_soc_obj;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "dfs_soc_obj->num_dfs_privs: %d ",
		 dfs_soc_obj->num_dfs_privs);
}
#endif

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

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "exclude_ieee_freq = %u",
		 exclude_ieee_freq);

	PRECAC_LIST_LOCK(dfs);
	if (!TAILQ_EMPTY(&dfs->dfs_precac_required_list)) {
		TAILQ_FOREACH(precac_entry, &dfs->dfs_precac_required_list,
				pe_list) {
			dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
				 "freq: %d ", precac_entry->vht80_freq);
			if (precac_entry->vht80_freq != exclude_ieee_freq) {
				ieee_freq = precac_entry->vht80_freq;
				break;
			}
		}
	}
	PRECAC_LIST_UNLOCK(dfs);
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "ieee_freq = %u", ieee_freq);

	return ieee_freq;
}

void dfs_cancel_precac_timer(struct wlan_dfs *dfs)
{
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	qdf_timer_stop(&dfs_soc_obj->dfs_precac_timer);
	dfs_soc_obj->dfs_precac_timer_running = 0;
}

#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_start_agile_precac_timer(struct wlan_dfs *dfs, uint8_t precac_chan,
				  uint8_t ocac_status)
{
	struct dfs_channel *ichan, lc;
	uint8_t first_primary_dfs_ch_ieee;
	int agile_cac_timeout;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	dfs_soc_obj->dfs_precac_timer_running = 1;

	first_primary_dfs_ch_ieee = precac_chan - VHT80_IEEE_FREQ_OFFSET;
	ichan = &lc;
	dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
				    first_primary_dfs_ch_ieee, 0,
				    WLAN_PHYMODE_11AC_VHT80,
				    &ichan->dfs_ch_freq,
				    &ichan->dfs_ch_flags,
				    &ichan->dfs_ch_flagext,
				    &ichan->dfs_ch_ieee,
				    &ichan->dfs_ch_vhtop_ch_freq_seg1,
				    &ichan->dfs_ch_vhtop_ch_freq_seg2);
	agile_cac_timeout = (dfs->dfs_precac_timeout_override != -1) ?
				dfs->dfs_precac_timeout_override :
	dfs_mlme_get_cac_timeout(dfs->dfs_pdev_obj,
				 ichan->dfs_ch_freq,
				 ichan->dfs_ch_vhtop_ch_freq_seg2,
				 ichan->dfs_ch_flags);
	if (ocac_status == OCAC_SUCCESS) {
		dfs_soc_obj->ocac_status = OCAC_SUCCESS;
		agile_cac_timeout = 0;
	}

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "precactimeout = %d", (agile_cac_timeout) * 1000);
	qdf_timer_mod(&dfs_soc_obj->dfs_precac_timer,
		      (agile_cac_timeout) * 1000);
}
#endif

void dfs_start_precac_timer(struct wlan_dfs *dfs,
			    uint8_t precac_chan)
{
	struct dfs_channel *ichan, lc;
	uint8_t first_primary_dfs_ch_ieee;
	int primary_cac_timeout;
	int secondary_cac_timeout;
	int precac_timeout;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	dfs = dfs_soc_obj->dfs_priv[dfs_soc_obj->cur_precac_dfs_index].dfs;
#define EXTRA_TIME_IN_SEC 5
	dfs_soc_obj->dfs_precac_timer_running = 1;

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
	 *
	 * If CAC/preCAC was already completed on primary, then we do not need
	 * to calculate which CAC timeout is maximum.
	 * For example: If primary's CAC is 600 seconds and secondary's CAC
	 * is 60 seconds then maximum gives 600 seconds which is not needed
	 * if CAC/preCAC was already completed on primary. It is to be noted
	 * that etsi_precac/cac is done on primary segment.
	 */
	if (WLAN_IS_CHAN_DFS(dfs->dfs_curchan) &&
	    !dfs_is_etsi_precac_done(dfs) &&
	    !dfs_is_precac_done(dfs, dfs->dfs_curchan))
		precac_timeout = QDF_MAX(primary_cac_timeout,
					 secondary_cac_timeout) +
				 EXTRA_TIME_IN_SEC;
	else
		precac_timeout = secondary_cac_timeout + EXTRA_TIME_IN_SEC;

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		"precactimeout = %d", (precac_timeout)*1000);
	qdf_timer_mod(&dfs_soc_obj->dfs_precac_timer, (precac_timeout) * 1000);
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

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
void dfs_set_precac_preferred_channel(struct wlan_dfs *dfs,
				      struct dfs_channel *chan, uint8_t mode)
{
	bool found = false;
	uint8_t freq_160_sec = 0;
	struct dfs_precac_entry *precac_entry;

	if (dfs_is_precac_timer_running(dfs) &&
	    WLAN_IS_CHAN_MODE_80(chan) &&
	    (dfs->dfs_precac_secondary_freq == chan->dfs_ch_freq)) {
		return;
	}

	/* Remove and insert into head, so that the user configured channel
	 * is picked first for preCAC.
	 */
	PRECAC_LIST_LOCK(dfs);
	if (WLAN_IS_CHAN_DFS(chan) &&
	    !TAILQ_EMPTY(&dfs->dfs_precac_required_list)) {
		TAILQ_FOREACH(precac_entry,
			      &dfs->dfs_precac_required_list, pe_list) {
			if (precac_entry->vht80_freq ==
			    chan->dfs_ch_vhtop_ch_freq_seg1) {
				found = true;
				TAILQ_REMOVE(&dfs->dfs_precac_required_list,
					     precac_entry, pe_list);
				TAILQ_INSERT_HEAD(&dfs->dfs_precac_required_list,
						  precac_entry, pe_list);
				break;
			}
		}
	}

	if (WLAN_IS_CHAN_MODE_160(chan) && WLAN_IS_CHAN_DFS(chan) &&
	    !TAILQ_EMPTY(&dfs->dfs_precac_required_list)) {
		if (chan->dfs_ch_ieee < chan->dfs_ch_vhtop_ch_freq_seg2)
			freq_160_sec = chan->dfs_ch_vhtop_ch_freq_seg1 +
				       VHT160_IEEE_FREQ_DIFF;
		else
			freq_160_sec = chan->dfs_ch_vhtop_ch_freq_seg1 -
				       VHT160_IEEE_FREQ_DIFF;

		found = false;
		TAILQ_FOREACH(precac_entry,
			      &dfs->dfs_precac_required_list, pe_list) {
			if (precac_entry->vht80_freq ==
			    freq_160_sec) {
				found = true;
				TAILQ_REMOVE(&dfs->dfs_precac_required_list,
					     precac_entry, pe_list);
				TAILQ_INSERT_HEAD(&dfs->dfs_precac_required_list,
						  precac_entry, pe_list);
				break;
			}
		}
	}

	PRECAC_LIST_UNLOCK(dfs);

	if (!found) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"frequency not found in precac list");
		return;
	}
}

bool
dfs_decide_precac_preferred_chan(struct wlan_dfs *dfs,
				 uint8_t *pref_chan,
				 enum wlan_phymode mode)
{
	struct dfs_channel chan;

	qdf_mem_zero(&chan, sizeof(struct dfs_channel));
	if (QDF_STATUS_SUCCESS !=
	    dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
					*pref_chan, 0,
					mode,
					&chan.dfs_ch_freq,
					&chan.dfs_ch_flags,
					&chan.dfs_ch_flagext,
					&chan.dfs_ch_ieee,
					&chan.dfs_ch_vhtop_ch_freq_seg1,
					&chan.dfs_ch_vhtop_ch_freq_seg2))
		return false;
	if (!dfs->dfs_precac_inter_chan)
		return false;

	/*
	 * If precac is done on this channel use it, else use a intermediate
	 * non-DFS channel and trigger a precac on this channel.
	 */
	if ((WLAN_IS_CHAN_DFS(&chan) ||
	    (WLAN_IS_CHAN_MODE_160(&chan) &&
	     WLAN_IS_CHAN_DFS_CFREQ2(&chan))) &&
	    !dfs_is_precac_done(dfs, &chan)) {
		dfs_set_precac_preferred_channel(dfs, &chan, mode);
		dfs->dfs_autoswitch_des_chan = *pref_chan;
		dfs->dfs_autoswitch_des_mode = mode;
		*pref_chan = dfs->dfs_precac_inter_chan;
		dfs_debug(dfs, WLAN_DEBUG_DFS,
			  "des_chan=%d, des_mode=%d. Current operating channel=%d",
			  dfs->dfs_autoswitch_des_chan,
			  dfs->dfs_autoswitch_des_mode,
			  *pref_chan);
		return true;
	}

	dfs->dfs_precac_inter_chan = chan.dfs_ch_ieee;
	return false;
}

enum precac_chan_state
dfs_get_precac_chan_state(struct wlan_dfs *dfs, uint8_t precac_chan)
{
	struct dfs_channel chan;
	struct dfs_precac_entry *tmp_precac_entry;
	enum precac_chan_state ret = PRECAC_ERR;

	qdf_mem_zero(&chan, sizeof(struct dfs_channel));
	if (QDF_STATUS_SUCCESS !=
	    dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
					precac_chan, 0,
					WLAN_PHYMODE_11AC_VHT80,
					&chan.dfs_ch_freq,
					&chan.dfs_ch_flags,
					&chan.dfs_ch_flagext,
					&chan.dfs_ch_ieee,
					&chan.dfs_ch_vhtop_ch_freq_seg1,
					&chan.dfs_ch_vhtop_ch_freq_seg2)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Intermediate channel not found");
		return PRECAC_ERR;
	}

	if (!WLAN_IS_CHAN_DFS(&chan)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"[%d] Not a DFS channel", precac_chan);
		return PRECAC_ERR;
	}

	PRECAC_LIST_LOCK(dfs);
	if (dfs_is_precac_timer_running(dfs)) {
		tmp_precac_entry = TAILQ_FIRST(&dfs->dfs_precac_required_list);
		if (tmp_precac_entry && (tmp_precac_entry->vht80_freq ==
			chan.dfs_ch_vhtop_ch_freq_seg1)) {
			ret = PRECAC_NOW;
			goto end;
		}
	}

	TAILQ_FOREACH(tmp_precac_entry,
		      &dfs->dfs_precac_required_list, pe_list) {
		if (tmp_precac_entry->vht80_freq ==
		    chan.dfs_ch_vhtop_ch_freq_seg1) {
			ret = PRECAC_REQUIRED;
			goto end;
		}
	}

	TAILQ_FOREACH(tmp_precac_entry,
		      &dfs->dfs_precac_done_list, pe_list) {
		if (tmp_precac_entry->vht80_freq ==
		    chan.dfs_ch_vhtop_ch_freq_seg1) {
			ret = PRECAC_DONE;
			goto end;
		}
	}

	TAILQ_FOREACH(tmp_precac_entry,
		      &dfs->dfs_precac_nol_list, pe_list) {
		if (tmp_precac_entry->vht80_freq ==
		    chan.dfs_ch_vhtop_ch_freq_seg1) {
			ret = PRECAC_NOL;
			goto end;
		}
	}

end:
	PRECAC_LIST_UNLOCK(dfs);
	return ret;
}
#endif

#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_find_vht80_chan_for_agile_precac(struct wlan_dfs *dfs,
					  uint8_t *ch_freq,
					  uint8_t ch_freq_seg1,
					  uint8_t ch_freq_seg2)
{
	uint8_t ieee_freq;

	dfs->dfs_soc_obj->ocac_status = OCAC_RESET;
	ieee_freq = dfs_get_freq_from_precac_required_list(
				dfs,
				ch_freq_seg1);
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "ieee_freq: %d  ch_freq_seg1: %d , ch_freq_seg2 : %d ",
		 ieee_freq, ch_freq_seg1, ch_freq_seg2);
	if (ieee_freq == ch_freq_seg2 && ieee_freq != 0)
		ieee_freq = dfs_get_freq_from_precac_required_list(
				dfs,
				ch_freq_seg2);
	if (ieee_freq) {
		dfs->dfs_agile_precac_freq = ieee_freq;
		/* Start the pre_cac_timer */
		dfs_start_agile_precac_timer(dfs,
					     dfs->dfs_agile_precac_freq,
					     dfs->dfs_soc_obj->ocac_status);
	} else {
		dfs->dfs_agile_precac_freq = 0;
	}

	*ch_freq = dfs->dfs_agile_precac_freq;
}
#endif

void dfs_find_vht80_chan_for_precac(struct wlan_dfs *dfs,
				    uint32_t chan_mode,
				    uint8_t ch_freq_seg1,
				    uint32_t *cfreq1,
				    uint32_t *cfreq2,
				    uint32_t *phy_mode,
				    bool *dfs_set_cfreq2,
				    bool *set_agile)
{
	uint8_t ieee_freq;

	if (chan_mode != WLAN_PHYMODE_11AC_VHT80)
		return;

	dfs_debug(dfs, WLAN_DEBUG_DFS,
		  "precac_secondary_freq = %u precac_running = %u",
		  dfs->dfs_precac_secondary_freq,
		  dfs->dfs_soc_obj->dfs_precac_timer_running);

	/*
	 * If Pre-CAC is enabled then find a center frequency for
	 * the secondary VHT80 and Change the mode to
	 * VHT80_80 or VHT160.
	 */
	if (dfs->dfs_precac_enable) {
		/*
		 * If precac timer is running then do not change the
		 * secondary channel use the old secondary VHT80
		 * channel. If precac timer is not running then try to
		 * find a new channel from precac-required-list.
		 */
		if (dfs->dfs_soc_obj->dfs_precac_timer_running) {
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

				ieee_160_cfreq = (ieee_freq + ch_freq_seg1) / 2;
				chan_mode = WLAN_PHYMODE_11AC_VHT160;
				*cfreq1 = dfs_mlme_ieee2mhz(dfs->dfs_pdev_obj,
							    ch_freq_seg1,
							    WLAN_CHAN_5GHZ);
				*cfreq2 = dfs_mlme_ieee2mhz(dfs->dfs_pdev_obj,
							    ieee_160_cfreq,
							    WLAN_CHAN_5GHZ);
			} else {
				/*
				 * Override the HW channel mode to
				 * VHT80_80.
				 */
				chan_mode = WLAN_PHYMODE_11AC_VHT80_80;
				*cfreq2 = dfs_mlme_ieee2mhz(dfs->dfs_pdev_obj,
							    ieee_freq,
							    WLAN_CHAN_5GHZ);
			}
			*phy_mode = lmac_get_phymode_info(dfs->dfs_pdev_obj,
							  chan_mode);
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
			if (dfs->dfs_precac_primary_freq == ch_freq_seg1)
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
		} else {
			dfs->dfs_precac_secondary_freq = 0;
		} /* End of if(ieee_freq) */
	} /* End of if(dfs->dfs_precac_enable) */
}

void dfs_set_precac_enable(struct wlan_dfs *dfs, uint32_t value)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_target_tx_ops *tx_ops;
	uint32_t target_type;
	struct target_psoc_info *tgt_hdl;
	struct tgt_info *info;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	if (!psoc) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "psoc is NULL");
		dfs->dfs_precac_enable = 0;
		dfs->dfs_agile_precac_enable = 0;
		return;
	}

	tx_ops = &psoc->soc_cb.tx_ops.target_tx_ops;
	target_type = lmac_get_target_type(dfs->dfs_pdev_obj);

	tgt_hdl = (struct target_psoc_info *)wlan_psoc_get_tgt_if_handle(psoc);
	if (!tgt_hdl) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "target_psoc_info is null");
		return;
	}

	info = (struct tgt_info *)(&tgt_hdl->info);

	if (!info) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "tgt_info is null");
		return;
	}

	/*
	 * If
	 * 1) The chip is CASCADE,
	 * 2) The user has enabled Pre-CAC and
	 * 3) The regdomain the ETSI,
	 * then enable preCAC.
	 *
	 * OR
	 *
	 * If
	 * 1) The chip has agile_capability enabled
	 * 2) The user has enabled Pre-CAC and
	 * 3) The regdomain the ETSI,
	 * then enable Agile preCAC.
	 */

	if ((1 == value) && tx_ops->tgt_is_tgt_type_qca9984(target_type) &&
	    (utils_get_dfsdomain(dfs->dfs_pdev_obj) == DFS_ETSI_DOMAIN)) {
		dfs->dfs_precac_enable = value;
	} else if ((1 == value) && (info->wlan_res_cfg.agile_capability == 1) &&
		(utils_get_dfsdomain(dfs->dfs_pdev_obj) == DFS_ETSI_DOMAIN)) {
		dfs->dfs_agile_precac_enable = value;
	} else {
		dfs->dfs_agile_precac_enable = 0;
		dfs->dfs_precac_enable = 0;
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "preCAC disabled");
	}

	if (dfs_is_precac_timer_running(dfs)) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Precac flag changed. Cancel the precac timer");
		dfs_cancel_precac_timer(dfs);
		dfs->dfs_soc_obj->precac_state_started = 0;
	}
}

#ifdef QCA_SUPPORT_AGILE_DFS
void dfs_agile_precac_start(struct wlan_dfs *dfs)
{
	uint8_t agile_freq = 0;
	uint8_t ocac_status = 0;
	struct dfs_soc_priv_obj *dfs_soc_obj;
	uint8_t cur_dfs_idx;

	dfs_soc_obj = dfs->dfs_soc_obj;
	/*
	 * Initiate first call to start preCAC here, for agile_freq as 0,
	 * and ocac_status as 0
	 */

	qdf_info("%s : %d agile_precac_started: %d",
		 __func__, __LINE__,
		dfs_soc_obj->precac_state_started);

	if (!dfs_soc_obj->precac_state_started)
		dfs_soc_obj->cur_precac_dfs_index = dfs->dfs_psoc_idx;

	cur_dfs_idx = dfs_soc_obj->cur_precac_dfs_index;
	dfs_soc_obj->dfs_priv[cur_dfs_idx].agile_precac_active = true;
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 " setting true to cur_precac_dfs_index = %d, dfs: %pK",
		 dfs_soc_obj->cur_precac_dfs_index,
		 dfs->dfs_soc_obj->dfs_priv[cur_dfs_idx].dfs);

	if (!dfs->dfs_soc_obj->precac_state_started) {
		qdf_info("%s : %d Initiated agile precac",
			 __func__, __LINE__);
		dfs->dfs_soc_obj->precac_state_started = true;
		dfs_start_agile_precac_timer(dfs,
					     agile_freq,
					     ocac_status);
	}
}
#endif

uint32_t dfs_get_precac_enable(struct wlan_dfs *dfs)
{
	struct wlan_objmgr_psoc *psoc;
	struct target_psoc_info *tgt_hdl;
	uint32_t retval = 0;
	struct tgt_info *info;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	if (!psoc) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "psoc is NULL");
		dfs->dfs_agile_precac_enable = 0;
		retval = 0;
	}

	tgt_hdl = (struct target_psoc_info *)wlan_psoc_get_tgt_if_handle(psoc);

	info = (struct tgt_info *)(&tgt_hdl->info);
	if (!tgt_hdl) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS, "target_psoc_info is null");
		dfs->dfs_agile_precac_enable = 0;
		retval = 0;
	}

	if (info->wlan_res_cfg.agile_capability == 0)
		retval = dfs->dfs_precac_enable;
	else
		retval = dfs->dfs_agile_precac_enable;

	return retval;
}

#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
int32_t dfs_set_precac_intermediate_chan(struct wlan_dfs *dfs, uint32_t value)
{
	struct dfs_channel chan;

	qdf_mem_zero(&chan, sizeof(struct dfs_channel));
	if (QDF_STATUS_SUCCESS !=
	    dfs_mlme_find_dot11_channel(dfs->dfs_pdev_obj,
					value, 0,
					WLAN_PHYMODE_11AC_VHT80,
					&chan.dfs_ch_freq,
					&chan.dfs_ch_flags,
					&chan.dfs_ch_flagext,
					&chan.dfs_ch_ieee,
					&chan.dfs_ch_vhtop_ch_freq_seg1,
					&chan.dfs_ch_vhtop_ch_freq_seg2)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Intermediate channel not found");
		return -EINVAL;
	}

	/*Intermediate channel should be non-DFS channel*/
	if (!WLAN_IS_CHAN_DFS(&chan)) {
		dfs->dfs_precac_inter_chan = value;
	} else {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,  "intermediate channel %s",
			(chan.dfs_ch_ieee == value) ?
			"should not be DFS channel" : "is invalid");
		dfs->dfs_precac_inter_chan = 0;
		return -EINVAL;
	}

	return 0;
}

uint32_t dfs_get_precac_intermediate_chan(struct wlan_dfs *dfs)
{
	return dfs->dfs_precac_inter_chan;
}
#endif
