/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains definition for mandatory legacy API
 */

#include "wlan_utility.h"
#include <wlan_cmn.h>

uint32_t wlan_chan_to_freq(uint8_t chan)
{
	/* ch 0 - ch 13 */
	if (chan < WLAN_24_GHZ_CHANNEL_14)
		return WLAN_24_GHZ_BASE_FREQ + chan * WLAN_CHAN_SPACING_5MHZ;
	else if (chan == WLAN_24_GHZ_CHANNEL_14)
		return WLAN_CHAN_14_FREQ;
	else if (chan < WLAN_24_GHZ_CHANNEL_27)
		/* ch 15 - ch 26 */
		return WLAN_CHAN_15_FREQ +
		  (chan - WLAN_24_GHZ_CHANNEL_15) * WLAN_CHAN_SPACING_20MHZ;
	else if (chan == WLAN_5_GHZ_CHANNEL_170)
		return WLAN_CHAN_170_FREQ;
	else
		return WLAN_5_GHZ_BASE_FREQ + chan * WLAN_CHAN_SPACING_5MHZ;
}

bool wlan_is_dsrc_channel(uint16_t center_freq)
{
	if (center_freq >= 5852 &&
	    center_freq <= 5920)
		return true;

	return false;
}

uint8_t wlan_freq_to_chan(uint32_t freq)
{
	uint8_t chan;

	if (freq > WLAN_24_GHZ_BASE_FREQ && freq < WLAN_CHAN_14_FREQ)
		chan = ((freq - WLAN_24_GHZ_BASE_FREQ) /
			WLAN_CHAN_SPACING_5MHZ);
	else if (freq == WLAN_CHAN_14_FREQ)
		chan = WLAN_24_GHZ_CHANNEL_14;
	else if ((freq > WLAN_24_GHZ_BASE_FREQ) &&
		(freq < WLAN_5_GHZ_BASE_FREQ))
		chan = (((freq - WLAN_CHAN_15_FREQ) /
			WLAN_CHAN_SPACING_20MHZ) +
			WLAN_24_GHZ_CHANNEL_15);
	else
		chan = (freq - WLAN_5_GHZ_BASE_FREQ) /
			WLAN_CHAN_SPACING_5MHZ;

	return chan;
}

uint8_t *wlan_get_vendor_ie_ptr_from_oui(uint8_t *oui,
	uint8_t oui_size, uint8_t *ie, uint16_t ie_len)
{
	int32_t left = ie_len;
	uint8_t *ptr = ie;
	uint8_t elem_id, elem_len;

	while (left >= 2) {
		elem_id  = ptr[0];
		elem_len = ptr[1];
		left -= 2;
		if (elem_len > left)
			return NULL;
		if (WLAN_MAC_EID_VENDOR == elem_id) {
			if (memcmp(&ptr[2], oui, oui_size) == 0)
				return ptr;
		}

		left -= elem_len;
		ptr += (elem_len + 2);
	}

	return NULL;
}

bool wlan_is_emulation_platform(uint32_t phy_version)
{
	if ((phy_version == 0xABC0) || (phy_version == 0xABC1) ||
		(phy_version == 0xABC2) || (phy_version == 0xABC3) ||
		(phy_version == 0xFFFF) || (phy_version == 0xABCD))
		return true;

	return false;
}

uint32_t wlan_get_pdev_id_from_vdev_id(struct wlan_objmgr_psoc *psoc,
				      uint8_t vdev_id,
				      wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev = NULL;
	uint32_t pdev_id = WLAN_INVALID_PDEV_ID;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
						    vdev_id, dbg_id);

	if (vdev) {
		pdev = wlan_vdev_get_pdev(vdev);
		if (pdev)
			pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
		wlan_objmgr_vdev_release_ref(vdev, dbg_id);
	}

	return pdev_id;
}
EXPORT_SYMBOL(wlan_get_pdev_id_from_vdev_id);
