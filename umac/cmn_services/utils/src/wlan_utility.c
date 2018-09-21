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
#include <qdf_module.h>
#include <wlan_vdev_mlme_api.h>

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
qdf_export_symbol(wlan_get_pdev_id_from_vdev_id);

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
qdf_export_symbol(wlan_util_vdev_get_if_name);

#ifdef CMN_VDEV_MLME_SM_ENABLE
static void wlan_vdev_active(struct wlan_objmgr_pdev *pdev, void *object,
			     void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	uint8_t *flag = (uint8_t *)arg;

	wlan_vdev_obj_lock(vdev);
	if (wlan_vdev_mlme_is_active(vdev) == QDF_STATUS_SUCCESS)
		*flag = 1;

	wlan_vdev_obj_unlock(vdev);
}

bool wlan_vdev_is_up(struct wlan_objmgr_vdev *vdev)
{
	bool ret_val = false;

	if (wlan_vdev_allow_connect_n_tx(vdev) == QDF_STATUS_SUCCESS)
		ret_val = true;

	return ret_val;
}

qdf_export_symbol(wlan_vdev_is_up);
#else
static void wlan_vdev_active(struct wlan_objmgr_pdev *pdev, void *object,
			     void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	uint8_t *flag = (uint8_t *)arg;

	wlan_vdev_obj_lock(vdev);
	if ((wlan_vdev_mlme_get_state(vdev) == WLAN_VDEV_S_RUN) ||
		(wlan_vdev_mlme_get_state(vdev) == WLAN_VDEV_S_DFS_WAIT))
		*flag = 1;

	wlan_vdev_obj_unlock(vdev);
}

bool wlan_vdev_is_up(struct wlan_objmgr_vdev *vdev)
{
	bool ret_val = false;

	wlan_vdev_obj_lock(vdev);
	if (wlan_vdev_mlme_get_state(vdev) == WLAN_VDEV_S_RUN)
		ret_val = true;

	wlan_vdev_obj_unlock(vdev);

	return ret_val;
}

qdf_export_symbol(wlan_vdev_is_up);
#endif

QDF_STATUS wlan_util_is_vdev_active(struct wlan_objmgr_pdev *pdev,
				    wlan_objmgr_ref_dbgid dbg_id)
{
	uint8_t flag = 0;

	if (!pdev)
		return QDF_STATUS_E_INVAL;

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP, wlan_vdev_active,
					  &flag, 0, dbg_id);

	if (flag == 1)
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_INVAL;
}

#ifdef CMN_VDEV_MLME_SM_ENABLE
void wlan_util_change_map_index(uint32_t *map, uint8_t id, uint8_t set)
{
	uint8_t map_index = 0;
	uint8_t map_entry_size = 32;
	uint8_t adjust_index = 0;

	/*
	 * Derive map_index and adjust_index to find actual DWORD
	 * the id map is present
	 */
	while ((id - adjust_index) >= map_entry_size) {
		map_index++;
		adjust_index = map_index * map_entry_size;
	}
	if (set)
		map[map_index] |= (1 << (id - adjust_index));
	else
		map[map_index] &= ~(1 << (id - adjust_index));
}

bool wlan_util_map_index_is_set(uint32_t *map, uint8_t id)
{
	uint8_t map_index = 0;
	uint8_t map_entry_size = 32;
	uint8_t adjust_index = 0;

	/*
	 * Derive map_index and adjust_index to find actual DWORD
	 * the id map is present
	 */
	while ((id - adjust_index) >= map_entry_size) {
		map_index++;
		adjust_index = map_index * map_entry_size;
	}
	if (map[map_index] & (1 << (id - adjust_index)))
		return true;

	return false;
}

static void wlan_vdev_chan_change_pending(struct wlan_objmgr_pdev *pdev,
					  void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	uint32_t *vdev_id_map = (uint32_t *)arg;
	uint8_t id = 0;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return;

	wlan_vdev_obj_lock(vdev);
	if (wlan_vdev_chan_config_valid(vdev) == QDF_STATUS_SUCCESS) {
		id = wlan_vdev_get_id(vdev);
		/* Invalid vdev id */
		if (id >= wlan_psoc_get_max_vdev_count(psoc))
			return;

		wlan_util_change_map_index(vdev_id_map, id, 1);
	}

	wlan_vdev_obj_unlock(vdev);
}

QDF_STATUS wlan_pdev_chan_change_pending_vdevs(struct wlan_objmgr_pdev *pdev,
					       uint32_t *vdev_id_map,
					       wlan_objmgr_ref_dbgid dbg_id)
{
	if (!pdev)
		return QDF_STATUS_E_INVAL;

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_vdev_chan_change_pending,
					  vdev_id_map, 0, dbg_id);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_chan_eq(struct wlan_channel *chan1, struct wlan_channel *chan2)
{
	if (!qdf_mem_cmp(chan1, chan2, sizeof(struct wlan_channel)))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

static void wlan_pdev_chan_match(struct wlan_objmgr_pdev *pdev,	void *object,
				 void *arg)
{
	struct wlan_objmgr_vdev *comp_vdev = (struct wlan_objmgr_vdev *)object;
	struct wlan_vdev_ch_check_filter *ch_filter = arg;

	if (ch_filter->flag)
		return;

	wlan_vdev_obj_lock(comp_vdev);
	wlan_vdev_obj_lock(ch_filter->vdev);

	if (wlan_vdev_chan_config_valid(ch_filter->vdev) == QDF_STATUS_SUCCESS)
		if (wlan_chan_eq(wlan_vdev_mlme_get_des_chan(comp_vdev),
				 wlan_vdev_mlme_get_des_chan(ch_filter->vdev))
				 != QDF_STATUS_SUCCESS)
			ch_filter->flag = 1;

	wlan_vdev_obj_unlock(ch_filter->vdev);
	wlan_vdev_obj_unlock(comp_vdev);
}

QDF_STATUS wlan_util_pdev_vdevs_deschan_match(struct wlan_objmgr_pdev *pdev,
					      struct wlan_objmgr_vdev *vdev,
					      wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_vdev_ch_check_filter ch_filter;

	if (!pdev)
		return QDF_STATUS_E_INVAL;

	ch_filter.flag = 0;
	ch_filter.vdev = vdev;

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_pdev_chan_match, &ch_filter, 0,
					  dbg_id);

	if (ch_filter.flag == 0)
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}
#endif

void
wlan_util_stats_get_rssi(bool db2dbm_enabled, int32_t bcn_snr, int32_t dat_snr,
			 int8_t *rssi)
{
	uint32_t snr;

	if (db2dbm_enabled) {
		if (TGT_IS_VALID_RSSI(bcn_snr))
			*rssi = bcn_snr;
		else if (TGT_IS_VALID_RSSI(dat_snr))
			*rssi = dat_snr;
		else
			*rssi = TGT_NOISE_FLOOR_DBM;
	} else {
		if (TGT_IS_VALID_SNR(bcn_snr))
			snr = bcn_snr;
		else if (TGT_IS_VALID_SNR(dat_snr))
			snr = dat_snr;
		else
			snr = TGT_INVALID_SNR;

		/* Get the absolute rssi value from the current rssi value */
		*rssi = snr + TGT_NOISE_FLOOR_DBM;
	}
}
