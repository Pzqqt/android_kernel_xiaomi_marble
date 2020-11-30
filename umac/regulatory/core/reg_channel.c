/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
 *
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
 * DOC: reg_channel.c
 * This file defines the API to access/update/modify regulatory current channel
 * list by WIN host umac components.
 */

#include <wlan_cmn.h>
#include <reg_services_public_struct.h>
#include <wlan_objmgr_psoc_obj.h>
#include <reg_priv_objs.h>
#include <reg_services_common.h>
#include <reg_channel.h>

#ifdef CONFIG_HOST_FIND_CHAN

static inline int is_11ax_supported(uint32_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11AX_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_PHYMODE_NO11AX);
}

static inline int is_11ac_supported(uint32_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11AC_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_PHYMODE_NO11AC);
}

static inline int is_11n_supported(uint32_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11N_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_CHAN_NO11N);
}

static inline int is_11g_supported(uint32_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11G_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_PHYMODE_NO11G);
}

static inline int is_11b_supported(uint32_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11B_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_PHYMODE_NO11B);
}

static inline int is_11a_supported(uint32_t wireless_modes, uint32_t phybitmap)
{
	return (WIRELESS_11A_MODES & wireless_modes) &&
		!(phybitmap & REGULATORY_PHYMODE_NO11A);
}

void reg_update_max_phymode_chwidth_for_pdev(
		struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj)
{
	uint32_t wireless_modes = pdev_priv_obj->wireless_modes;
	uint32_t phybitmap = pdev_priv_obj->phybitmap;
	enum phy_ch_width max_chwidth = CH_WIDTH_20MHZ;
	enum reg_phymode max_phymode = REG_PHYMODE_MAX;

	if (is_11ax_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11AX;
		if (wireless_modes & WIRELESS_160_MODES)
			max_chwidth = CH_WIDTH_160MHZ;
		else if (wireless_modes & WIRELESS_80_MODES)
			max_chwidth = CH_WIDTH_80MHZ;
		else if (wireless_modes & WIRELESS_40_MODES)
			max_chwidth = CH_WIDTH_40MHZ;
	} else if (is_11ac_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11AC;
		if (wireless_modes & WIRELESS_160_MODES)
			max_chwidth = CH_WIDTH_160MHZ;
		else if (wireless_modes & WIRELESS_80_MODES)
			max_chwidth = CH_WIDTH_80MHZ;
		else if (wireless_modes & WIRELESS_40_MODES)
			max_chwidth = CH_WIDTH_40MHZ;
	} else if (is_11n_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11N;
		if (wireless_modes & WIRELESS_40_MODES)
			max_chwidth = CH_WIDTH_40MHZ;
	} else if (is_11g_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11G;
	} else if (is_11b_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11B;
	} else if (is_11a_supported(wireless_modes, phybitmap)) {
		max_phymode = REG_PHYMODE_11A;
	} else {
		reg_err("Device does not support any wireless_mode! %0x",
			wireless_modes);
	}

	pdev_priv_obj->max_phymode = max_phymode;
	pdev_priv_obj->max_chwidth = max_chwidth;
}

void reg_modify_chan_list_for_max_chwidth(
		struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj,
		struct regulatory_channel *cur_chan_list)
{
	int i;

	for (i = 0; i < NUM_CHANNELS; i++) {
		struct ch_params chan_params;

		if (cur_chan_list[i].chan_flags & REGULATORY_CHAN_DISABLED)
			continue;

		chan_params.ch_width = pdev_priv_obj->max_chwidth;

		/*
		 * Correct the max bandwidths if they were not taken care of
		 * while parsing the reg rules.
		 */
		reg_set_channel_params_for_freq(pdev_priv_obj->pdev_ptr,
						cur_chan_list[i].center_freq,
						0,
						&chan_params);

		if (chan_params.ch_width != CH_WIDTH_INVALID)
			cur_chan_list[i].max_bw =
				qdf_min(cur_chan_list[i].max_bw,
					reg_get_bw_value(chan_params.ch_width));
	}
}

static uint32_t convregphymode2wirelessmodes[REG_PHYMODE_MAX] = {
	0xFFFFFFFF,                  /* REG_PHYMODE_INVALID */
	WIRELESS_11B_MODES,          /* REG_PHYMODE_11B     */
	WIRELESS_11G_MODES,          /* REG_PHYMODE_11G     */
	WIRELESS_11A_MODES,          /* REG_PHYMODE_11A     */
	WIRELESS_11N_MODES,          /* REG_PHYMODE_11N     */
	WIRELESS_11AC_MODES,         /* REG_PHYMODE_11AC    */
	WIRELESS_11AX_MODES,         /* REG_PHYMODE_11AX    */
};

static int reg_is_phymode_in_wireless_modes(enum reg_phymode phy_in,
					    uint32_t wireless_modes)
{
	uint32_t sup_wireless_modes = convregphymode2wirelessmodes[phy_in];

	return sup_wireless_modes & wireless_modes;
}

bool reg_is_phymode_chwidth_allowed(
		struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj,
		enum reg_phymode phy_in,
		enum phy_ch_width ch_width,
		qdf_freq_t freq)
{
	uint32_t phymode_bitmap, wireless_modes;
	uint16_t i, bw = reg_get_bw_value(ch_width);

	if (ch_width == CH_WIDTH_INVALID)
		return false;

	phymode_bitmap = pdev_priv_obj->phybitmap;
	wireless_modes = pdev_priv_obj->wireless_modes;

	if (reg_is_phymode_unallowed(phy_in, phymode_bitmap) ||
	    !reg_is_phymode_in_wireless_modes(phy_in, wireless_modes))
		return false;

	for (i = 0; i < NUM_CHANNELS; i++) {
		if (pdev_priv_obj->cur_chan_list[i].center_freq != freq)
			continue;

		if (pdev_priv_obj->cur_chan_list[i].state ==
		    CHANNEL_STATE_DISABLE)
			return false;

		if (bw < pdev_priv_obj->cur_chan_list[i].min_bw ||
		    bw > pdev_priv_obj->cur_chan_list[i].max_bw)
			return false;

		return true;
	}

	return false;
}

#endif
