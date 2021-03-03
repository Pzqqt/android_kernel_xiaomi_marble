/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2002-2006, Atheros Communications Inc.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: This file contains the miscellanous files
 *
 */

#include "dfs.h"
#include "../dfs_misc.h"
#include "dfs_zero_cac.h"
#include "wlan_dfs_mlme_api.h"
#include "ieee80211_mlme_dfs_interface.h"
#include "wlan_dfs_init_deinit_api.h"
#include "_ieee80211.h"
#include "ieee80211_var.h"
#include "ieee80211_mlme_dfs_dispatcher.h"
#include "ieee80211_channel.h"
#include "wlan_mlme_if.h"
#include "dfs_postnol_ucfg.h"
#include "wlan_dfs_lmac_api.h"
#include "dfs_process_radar_found_ind.h"
#include "wlan_dfs_utils_api.h"

#if defined(QCA_SUPPORT_DFS_CHAN_POSTNOL)
void dfs_postnol_attach(struct wlan_dfs *dfs)
{
	dfs->dfs_chan_postnol_mode = CH_WIDTH_INVALID;
}
#endif

#ifdef QCA_SUPPORT_DFS_CHAN_POSTNOL
void dfs_set_postnol_freq(struct wlan_dfs *dfs, qdf_freq_t postnol_freq)
{
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "dfs_chan_postnol_freq configured as %d", postnol_freq);

	dfs->dfs_chan_postnol_freq = postnol_freq;
}

void dfs_set_postnol_mode(struct wlan_dfs *dfs, uint8_t postnol_mode)
{
	if (dfs->dfs_chan_postnol_cfreq2) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "postNOL cfreq2 has been set,reset it to change mode");
		return;
	}

	switch (postnol_mode) {
	case DFS_CHWIDTH_20_VAL:
		dfs->dfs_chan_postnol_mode = CH_WIDTH_20MHZ;
		break;
	case DFS_CHWIDTH_40_VAL:
		dfs->dfs_chan_postnol_mode = CH_WIDTH_40MHZ;
		break;
	case DFS_CHWIDTH_80_VAL:
		dfs->dfs_chan_postnol_mode = CH_WIDTH_80MHZ;
		break;
	case DFS_CHWIDTH_160_VAL:
		dfs->dfs_chan_postnol_mode = CH_WIDTH_160MHZ;
		break;
	default:
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Invalid postNOL mode configured");
		return;
	}

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "DFS postnol mode configured as %d",
		 dfs->dfs_chan_postnol_mode);
}

void dfs_set_postnol_cfreq2(struct wlan_dfs *dfs, qdf_freq_t postnol_cfreq2)
{
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "dfs_chan_postnol_cfreq2 configured as %d", postnol_cfreq2);

	dfs->dfs_chan_postnol_cfreq2 = postnol_cfreq2;

	if (postnol_cfreq2) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "postNOL cfreq2 is set, changing mode to 80P80");
		dfs->dfs_chan_postnol_mode = CH_WIDTH_80P80MHZ;
	}
}

void dfs_get_postnol_freq(struct wlan_dfs *dfs, qdf_freq_t *postnol_freq)
{
	*postnol_freq = dfs->dfs_chan_postnol_freq;
}

void dfs_get_postnol_mode(struct wlan_dfs *dfs, uint8_t *postnol_mode)
{
	*postnol_mode = dfs->dfs_chan_postnol_mode;
}

void dfs_get_postnol_cfreq2(struct wlan_dfs *dfs, qdf_freq_t *postnol_cfreq2)
{
	*postnol_cfreq2 = dfs->dfs_chan_postnol_cfreq2;
}
#endif

#ifdef QCA_SUPPORT_DFS_CHAN_POSTNOL
bool dfs_switch_to_postnol_chan_if_nol_expired(struct wlan_dfs *dfs)
{
	struct dfs_channel chan;
	struct dfs_channel *curchan = dfs->dfs_curchan;
	bool is_curchan_11ac = false, is_curchan_11axa = false;
	enum wlan_phymode postnol_phymode;

	if (!dfs->dfs_chan_postnol_freq)
		return false;

	if (WLAN_IS_CHAN_11AC_VHT20(curchan) ||
	    WLAN_IS_CHAN_11AC_VHT40(curchan) ||
	    WLAN_IS_CHAN_11AC_VHT80(curchan) ||
	    WLAN_IS_CHAN_11AC_VHT160(curchan) ||
	    WLAN_IS_CHAN_11AC_VHT80_80(curchan))
		is_curchan_11ac = true;
	else if (WLAN_IS_CHAN_11AXA_HE20(curchan) ||
		 WLAN_IS_CHAN_11AXA_HE40PLUS(curchan) ||
		 WLAN_IS_CHAN_11AXA_HE40MINUS(curchan) ||
		 WLAN_IS_CHAN_11AXA_HE80(curchan) ||
		 WLAN_IS_CHAN_11AXA_HE160(curchan) ||
		 WLAN_IS_CHAN_11AXA_HE80_80(curchan))
		is_curchan_11axa = true;

	switch (dfs->dfs_chan_postnol_mode) {
	case CH_WIDTH_20MHZ:
		if (is_curchan_11ac)
			postnol_phymode = WLAN_PHYMODE_11AC_VHT20;
		else if (is_curchan_11axa)
			postnol_phymode = WLAN_PHYMODE_11AXA_HE20;
		else
			return false;
		break;
	case CH_WIDTH_40MHZ:
		if (is_curchan_11ac)
			postnol_phymode = WLAN_PHYMODE_11AC_VHT40;
		else if (is_curchan_11axa)
			postnol_phymode = WLAN_PHYMODE_11AXA_HE40;
		else
			return false;
		break;
	case CH_WIDTH_80MHZ:
		if (is_curchan_11ac)
			postnol_phymode = WLAN_PHYMODE_11AC_VHT80;
		else if (is_curchan_11axa)
			postnol_phymode = WLAN_PHYMODE_11AXA_HE80;
		else
			return false;
		break;
	case CH_WIDTH_160MHZ:
		if (is_curchan_11ac)
			postnol_phymode = WLAN_PHYMODE_11AC_VHT160;
		else if (is_curchan_11axa)
			postnol_phymode = WLAN_PHYMODE_11AXA_HE160;
		else
			return false;
		break;
	case CH_WIDTH_80P80MHZ:
		if (is_curchan_11ac)
			postnol_phymode = WLAN_PHYMODE_11AC_VHT80_80;
		else if (is_curchan_11axa)
			postnol_phymode = WLAN_PHYMODE_11AXA_HE80_80;
		else
			return false;
		break;
	default:
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Invalid postNOL mode set. Cannot switch to the chan");
		return false;
	}

	qdf_mem_zero(&chan, sizeof(struct dfs_channel));
	if (QDF_STATUS_SUCCESS !=
		dfs_mlme_find_dot11_chan_for_freq(
			dfs->dfs_pdev_obj,
			dfs->dfs_chan_postnol_freq,
			dfs->dfs_chan_postnol_cfreq2,
			postnol_phymode,
			&chan.dfs_ch_freq,
			&chan.dfs_ch_flags,
			&chan.dfs_ch_flagext,
			&chan.dfs_ch_ieee,
			&chan.dfs_ch_vhtop_ch_freq_seg1,
			&chan.dfs_ch_vhtop_ch_freq_seg2,
			&chan.dfs_ch_mhz_freq_seg1,
			&chan.dfs_ch_mhz_freq_seg2)) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"Channel %d not found for mode %d and cfreq2 %d",
			dfs->dfs_chan_postnol_freq,
			postnol_phymode,
			dfs->dfs_chan_postnol_cfreq2);
		return false;
	}
	if (WLAN_IS_CHAN_RADAR(dfs, &chan))
		return false;

	if (global_dfs_to_mlme.mlme_postnol_chan_switch)
		global_dfs_to_mlme.mlme_postnol_chan_switch(
				dfs->dfs_pdev_obj,
				dfs->dfs_chan_postnol_freq,
				dfs->dfs_chan_postnol_cfreq2,
				postnol_phymode);
	return true;
}
#endif

#ifdef QCA_HW_MODE_SWITCH
bool dfs_is_hw_mode_switch_in_progress(struct wlan_dfs *dfs)
{
	return lmac_dfs_is_hw_mode_switch_in_progress(dfs->dfs_pdev_obj);
}


void dfs_complete_deferred_tasks(struct wlan_dfs *dfs)
{
	if (dfs->dfs_defer_params.is_radar_detected) {
		/* Handle radar event that was deferred and free the temporary
		 * storage of the radar event parameters.
		 */
		dfs_process_radar_ind(dfs, dfs->dfs_defer_params.radar_params);
		qdf_mem_free(dfs->dfs_defer_params.radar_params);
		dfs->dfs_defer_params.is_radar_detected = false;
	} else if (dfs->dfs_defer_params.is_cac_completed) {
		/* Handle CAC completion event that was deferred for HW mode
		 * switch.
		 */
		dfs_process_cac_completion(dfs);
		dfs->dfs_defer_params.is_cac_completed = false;
	}
}

void dfs_init_tmp_psoc_nol(struct wlan_dfs *dfs, uint8_t num_radios)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;

	if (WLAN_UMAC_MAX_PDEVS < num_radios) {
		dfs_err(dfs, WLAN_DEBUG_DFS_ALWAYS,
			"num_radios (%u) exceeds limit", num_radios);
		return;
	}

	/* Allocate the temporary psoc NOL copy structure for the number
	 * of radios provided.
	 */
	dfs_soc_obj->dfs_psoc_nolinfo =
		qdf_mem_malloc(sizeof(struct dfsreq_nolinfo) * num_radios);
}

void dfs_deinit_tmp_psoc_nol(struct wlan_dfs *dfs)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;

	if (!dfs_soc_obj->dfs_psoc_nolinfo)
		return;

	qdf_mem_free(dfs_soc_obj->dfs_psoc_nolinfo);
	dfs_soc_obj->dfs_psoc_nolinfo = NULL;
}

void dfs_save_dfs_nol_in_psoc(struct wlan_dfs *dfs,
			      uint8_t pdev_id)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;
	struct dfsreq_nolinfo tmp_nolinfo, *nolinfo;
	uint32_t i, num_chans = 0;

	if (!dfs->dfs_nol_count)
		return;

	if (!dfs_soc_obj->dfs_psoc_nolinfo)
		return;

	nolinfo = &dfs_soc_obj->dfs_psoc_nolinfo[pdev_id];
	/* Fetch the NOL entries for the DFS object. */
	dfs_getnol(dfs, &tmp_nolinfo);

	/* nolinfo might already have some data. Do not overwrite it */
	num_chans = nolinfo->dfs_ch_nchans;
	for (i = 0; i < tmp_nolinfo.dfs_ch_nchans; i++) {
		/* Figure out the completed duration of each NOL. */
		uint32_t nol_completed_ms = qdf_do_div(
			qdf_get_monotonic_boottime() -
			tmp_nolinfo.dfs_nol[i].nol_start_us, 1000);

		nolinfo->dfs_nol[num_chans] = tmp_nolinfo.dfs_nol[i];
		/* Remember the remaining NOL time in the timeout
		 * variable.
		 */
		nolinfo->dfs_nol[num_chans++].nol_timeout_ms -=
			nol_completed_ms;
	}

	nolinfo->dfs_ch_nchans = num_chans;
}

void dfs_reinit_nol_from_psoc_copy(struct wlan_dfs *dfs,
				   uint8_t pdev_id,
				   uint16_t low_5ghz_freq,
				   uint16_t high_5ghz_freq)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;
	struct dfsreq_nolinfo *nol, req_nol;
	uint8_t i, j = 0;

	if (!dfs_soc_obj->dfs_psoc_nolinfo)
		return;

	if (!dfs_soc_obj->dfs_psoc_nolinfo[pdev_id].dfs_ch_nchans)
		return;

	nol = &dfs_soc_obj->dfs_psoc_nolinfo[pdev_id];

	for (i = 0; i < nol->dfs_ch_nchans; i++) {
		uint16_t tmp_freq = nol->dfs_nol[i].nol_freq;

		/* Add to nol only if within the tgt pdev's frequency range. */
		if ((low_5ghz_freq < tmp_freq) && (high_5ghz_freq > tmp_freq)) {
			/* The NOL timeout value in each entry points to the
			 * remaining time of the NOL. This is to indicate that
			 * the NOL entries are paused and are not left to
			 * continue.
			 * While adding these NOL, update the start ticks to
			 * current time to avoid losing entries which might
			 * have timed out during the pause and resume mechanism.
			 */
			nol->dfs_nol[i].nol_start_us =
				qdf_get_monotonic_boottime();
			req_nol.dfs_nol[j++] = nol->dfs_nol[i];
		}
	}
	dfs_set_nol(dfs, req_nol.dfs_nol, j);
}
#endif

#ifdef CONFIG_HOST_FIND_CHAN
bool wlan_is_chan_radar(struct wlan_dfs *dfs, struct dfs_channel *chan)
{
	qdf_freq_t sub_freq_list[NUM_CHANNELS_160MHZ];
	uint8_t n_subchans, i;

	if (!chan || !WLAN_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(chan))
		return false;

	n_subchans = dfs_get_bonding_channel_without_seg_info_for_freq(
				chan,
				sub_freq_list);

	for (i = 0; i < n_subchans; i++) {
		if (wlan_reg_is_nol_for_freq(dfs->dfs_pdev_obj,
					     sub_freq_list[i]))
			return true;
	}

	return false;
}

bool wlan_is_chan_history_radar(struct wlan_dfs *dfs, struct dfs_channel *chan)
{
	qdf_freq_t sub_freq_list[NUM_CHANNELS_160MHZ];
	uint8_t n_subchans, i;

	if (!chan || !WLAN_IS_PRIMARY_OR_SECONDARY_CHAN_DFS(chan))
		return false;

	n_subchans = dfs_get_bonding_channel_without_seg_info_for_freq(
				chan,
				sub_freq_list);

	for (i = 0; i < n_subchans; i++) {
		if (wlan_reg_is_nol_hist_for_freq(dfs->dfs_pdev_obj,
						  sub_freq_list[i]))
			return true;
	}

	return false;
}
#endif /* CONFIG_HOST_FIND_CHAN */
