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
 * DOC: This file contains definition for mandatory legacy API
 */

#include "qdf_str.h"
#include "wlan_utility.h"
#include <wlan_cmn.h>
#include "wlan_osif_priv.h"
#include <net/cfg80211.h>

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

bool wlan_is_ie_valid(const uint8_t *ie, size_t ie_len)
{
	uint8_t elen;

	while (ie_len) {
		if (ie_len < 2)
			return false;

		elen = ie[1];
		ie_len -= 2;
		ie += 2;
		if (elen > ie_len)
			return false;

		ie_len -= elen;
		ie += elen;
	}

	return true;
}

static const uint8_t *wlan_get_ie_ptr_from_eid_n_oui(uint8_t eid,
						     const uint8_t *oui,
						     uint8_t oui_size,
						     const uint8_t *ie,
						     uint16_t ie_len)
{
	int32_t left = ie_len;
	const uint8_t *ptr = ie;
	uint8_t elem_id, elem_len;

	while (left >= 2) {
		elem_id  = ptr[0];
		elem_len = ptr[1];
		left -= 2;

		if (elem_len > left)
			return NULL;

		if (eid == elem_id) {
			/* if oui is not provide eid match is enough */
			if (!oui)
				return ptr;

			/*
			 * if oui is provided and oui_size is more than left
			 * bytes, then we cannot have match
			 */
			if (oui_size > left)
				return NULL;

			if (qdf_mem_cmp(&ptr[2], oui, oui_size) == 0)
				return ptr;
		}

		left -= elem_len;
		ptr += (elem_len + 2);
	}

	return NULL;
}

const uint8_t *wlan_get_ie_ptr_from_eid(uint8_t eid,
					const uint8_t *ie,
					int ie_len)
{
	return wlan_get_ie_ptr_from_eid_n_oui(eid, NULL, 0, ie, ie_len);
}

const uint8_t *wlan_get_vendor_ie_ptr_from_oui(const uint8_t *oui,
					       uint8_t oui_size,
					       const uint8_t *ie,
					       uint16_t ie_len)
{
	return wlan_get_ie_ptr_from_eid_n_oui(WLAN_MAC_EID_VENDOR,
					      oui, oui_size, ie, ie_len);
}

const uint8_t *wlan_get_ext_ie_ptr_from_ext_id(const uint8_t *oui,
					       uint8_t oui_size,
					       const uint8_t *ie,
					       uint16_t ie_len)
{
	return wlan_get_ie_ptr_from_eid_n_oui(WLAN_MAC_EID_EXT,
					      oui, oui_size, ie, ie_len);
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

static void wlan_util_get_vdev_by_ifname_cb(struct wlan_objmgr_psoc *psoc,
					    void *obj, void *arg)
{
	struct wlan_objmgr_vdev *vdev = obj;
	struct wlan_find_vdev_filter *filter = arg;

	if (filter->found_vdev)
		return;

	wlan_vdev_obj_lock(vdev);
	if (!qdf_str_cmp(vdev->vdev_nif.osdev->wdev->netdev->name,
			 filter->ifname)) {
		filter->found_vdev = vdev;
	}
	wlan_vdev_obj_unlock(vdev);
}

struct wlan_objmgr_vdev *wlan_util_get_vdev_by_ifname(
				struct wlan_objmgr_psoc *psoc, char *ifname,
				wlan_objmgr_ref_dbgid ref_id)
{
	QDF_STATUS status;
	struct wlan_find_vdev_filter filter = {0};

	filter.ifname = ifname;
	wlan_objmgr_iterate_obj_list(psoc, WLAN_VDEV_OP,
				     wlan_util_get_vdev_by_ifname_cb,
				     &filter, 0, ref_id);

	if (!filter.found_vdev)
		return NULL;

	status = wlan_objmgr_vdev_try_get_ref(filter.found_vdev, ref_id);
	if (QDF_IS_STATUS_ERROR(status))
		return NULL;

	return filter.found_vdev;
}

/**
 * wlan_util_vdev_get_if_name() - get vdev's interface name
 * @vdev: VDEV object
 *
 * API to get vdev's interface name
 *
 * Return:
 * @id: vdev's interface name
 */
uint8_t *wlan_util_vdev_get_if_name(struct wlan_objmgr_vdev *vdev)
{
	uint8_t *name;
	struct vdev_osif_priv *osif_priv;

	wlan_vdev_obj_lock(vdev);

	osif_priv = wlan_vdev_get_ospriv(vdev);
	if (!osif_priv) {
		wlan_vdev_obj_unlock(vdev);
		return NULL;
	}

	if (!osif_priv->wdev) {
		wlan_vdev_obj_unlock(vdev);
		return NULL;
	}

	name = osif_priv->wdev->netdev->name;
	wlan_vdev_obj_unlock(vdev);

	return name;
}
EXPORT_SYMBOL(wlan_util_vdev_get_if_name);

static void wlan_vap_active(struct wlan_objmgr_pdev *pdev,
			void *object,
			void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	uint8_t *flag = (uint8_t *)arg;

	wlan_vdev_obj_lock(vdev);
	if ((wlan_vdev_mlme_get_state(vdev) == WLAN_VDEV_S_RUN) ||
		(wlan_vdev_mlme_get_state(vdev) == WLAN_VDEV_S_DFS_WAIT)) {
		*flag = 1;
	}
	wlan_vdev_obj_unlock(vdev);
}

QDF_STATUS wlan_util_is_vap_active(struct wlan_objmgr_pdev *pdev)
{
	uint8_t flag = 0;

	if (!pdev)
		return QDF_STATUS_E_INVAL;

	wlan_objmgr_pdev_iterate_obj_list(pdev,
				WLAN_VDEV_OP,
				wlan_vap_active,
				&flag, 0, WLAN_OBJMGR_ID);

	if (flag == 1)
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_INVAL;
}
