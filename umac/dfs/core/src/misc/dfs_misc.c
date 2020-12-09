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
	if (WLAN_IS_CHAN_RADAR(&chan))
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
