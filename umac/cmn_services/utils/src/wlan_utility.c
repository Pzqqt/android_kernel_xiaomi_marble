/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains definition for mandatory legacy API
 */

#include "qdf_str.h"
#include "wlan_utility.h"
#include <wlan_cmn.h>
#include "wlan_osif_priv.h"
#include <net/cfg80211.h>
#include <qdf_module.h>
#include <wlan_vdev_mlme_api.h>
#include "cfg_ucfg_api.h"

uint32_t wlan_chan_to_freq(uint8_t chan)
{
	if (chan == 0 )
		return 0;

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

	if (freq == 0)
		return 0;

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

QDF_STATUS wlan_vdev_is_up(struct wlan_objmgr_vdev *vdev)
{
	return wlan_vdev_allow_connect_n_tx(vdev);
}
qdf_export_symbol(wlan_vdev_is_up);

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

qdf_export_symbol(wlan_util_is_vdev_active);

void wlan_util_change_map_index(unsigned long *map, uint8_t id, uint8_t set)
{
	if (set)
		qdf_set_bit(id, map);
	else
		qdf_clear_bit(id, map);
}

bool wlan_util_map_index_is_set(unsigned long *map, uint8_t id)
{
	return qdf_test_bit(id, map);
}

bool wlan_util_map_is_any_index_set(unsigned long *map, unsigned long nbytes)
{
	return !qdf_bitmap_empty(map, QDF_CHAR_BIT * nbytes);
}

static void wlan_vdev_chan_change_pending(struct wlan_objmgr_pdev *pdev,
					  void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	unsigned long *vdev_id_map = (unsigned long *)arg;
	uint8_t id = 0;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return;

	wlan_vdev_obj_lock(vdev);
	if (wlan_vdev_chan_config_valid(vdev) == QDF_STATUS_SUCCESS) {
		id = wlan_vdev_get_id(vdev);
		/* Invalid vdev id */
		if (id >= wlan_psoc_get_max_vdev_count(psoc)) {
			wlan_vdev_obj_unlock(vdev);
			return;
		}

		wlan_util_change_map_index(vdev_id_map, id, 1);
	}

	wlan_vdev_obj_unlock(vdev);
}

QDF_STATUS wlan_pdev_chan_change_pending_vdevs(struct wlan_objmgr_pdev *pdev,
					       unsigned long *vdev_id_map,
					       wlan_objmgr_ref_dbgid dbg_id)
{
	if (!pdev)
		return QDF_STATUS_E_INVAL;

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_vdev_chan_change_pending,
					  vdev_id_map, 0, dbg_id);

	return QDF_STATUS_SUCCESS;
}

static void wlan_vdev_down_pending(struct wlan_objmgr_pdev *pdev,
				   void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	unsigned long *vdev_id_map = (unsigned long *)arg;
	uint8_t id = 0;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return;

	wlan_vdev_obj_lock(vdev);
	if (wlan_vdev_mlme_is_init_state(vdev) != QDF_STATUS_SUCCESS) {
		id = wlan_vdev_get_id(vdev);
		/* Invalid vdev id */
		if (id >= wlan_psoc_get_max_vdev_count(psoc)) {
			wlan_vdev_obj_unlock(vdev);
			return;
		}
		wlan_util_change_map_index(vdev_id_map, id, 1);
	}

	wlan_vdev_obj_unlock(vdev);
}

static void wlan_vdev_ap_down_pending(struct wlan_objmgr_pdev *pdev,
				      void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	unsigned long *vdev_id_map = (unsigned long *)arg;
	uint8_t id = 0;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return;

	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_SAP_MODE)
		return;

	wlan_vdev_obj_lock(vdev);
	if (wlan_vdev_mlme_is_init_state(vdev) != QDF_STATUS_SUCCESS) {
		id = wlan_vdev_get_id(vdev);
		/* Invalid vdev id */
		if (id >= wlan_psoc_get_max_vdev_count(psoc)) {
			wlan_vdev_obj_unlock(vdev);
			return;
		}
		wlan_util_change_map_index(vdev_id_map, id, 1);
	}

	wlan_vdev_obj_unlock(vdev);
}

QDF_STATUS wlan_pdev_chan_change_pending_vdevs_down(
					struct wlan_objmgr_pdev *pdev,
					unsigned long *vdev_id_map,
					wlan_objmgr_ref_dbgid dbg_id)
{
	if (!pdev)
		return QDF_STATUS_E_INVAL;

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_vdev_down_pending,
					  vdev_id_map, 0, dbg_id);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_pdev_chan_change_pending_ap_vdevs_down(
						struct wlan_objmgr_pdev *pdev,
						unsigned long *vdev_id_map,
						wlan_objmgr_ref_dbgid dbg_id)
{
	if (!pdev)
		return QDF_STATUS_E_INVAL;

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_vdev_ap_down_pending,
					  vdev_id_map, 0, dbg_id);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_chan_eq(struct wlan_channel *chan1, struct wlan_channel *chan2)
{
	if ((chan1->ch_ieee == chan2->ch_ieee) &&
	    (chan1->ch_freq_seg2 == chan2->ch_freq_seg2))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

void wlan_chan_copy(struct wlan_channel *tgt, struct wlan_channel *src)
{
	qdf_mem_copy(tgt, src, sizeof(struct wlan_channel));
}

struct wlan_channel *wlan_vdev_get_active_channel(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_channel *comp_vdev_chan = NULL;

	if (wlan_vdev_chan_config_valid(vdev) == QDF_STATUS_SUCCESS) {
		/* compare with BSS channel, when vdev is active, since desired
		 * channel gets update, if channel is triggered in another path
		 */
		if (wlan_vdev_mlme_is_active(vdev) == QDF_STATUS_SUCCESS)
			comp_vdev_chan = wlan_vdev_mlme_get_bss_chan(vdev);
		else
			comp_vdev_chan = wlan_vdev_mlme_get_des_chan(vdev);
	}

	return comp_vdev_chan;
}

static void wlan_pdev_chan_match(struct wlan_objmgr_pdev *pdev, void *object,
				 void *arg)
{
	struct wlan_objmgr_vdev *comp_vdev = (struct wlan_objmgr_vdev *)object;
	struct wlan_vdev_ch_check_filter *ch_filter = arg;
	struct wlan_channel vdev_chan, *chan;
	struct wlan_channel *iter_vdev_chan;

	if (ch_filter->flag)
		return;

	if (comp_vdev == ch_filter->vdev)
		return;

	wlan_vdev_obj_lock(comp_vdev);
	chan = wlan_vdev_get_active_channel(comp_vdev);
	if (!chan) {
		wlan_vdev_obj_unlock(comp_vdev);
		return;
	}
	wlan_chan_copy(&vdev_chan, chan);
	wlan_vdev_obj_unlock(comp_vdev);

	wlan_vdev_obj_lock(ch_filter->vdev);
	iter_vdev_chan = wlan_vdev_mlme_get_des_chan(ch_filter->vdev);
	if (wlan_chan_eq(&vdev_chan, iter_vdev_chan)
		!= QDF_STATUS_SUCCESS) {
		ch_filter->flag = 1;
		qdf_nofl_err("==> iter vdev id: %d: ieee %d, mode %d",
			     wlan_vdev_get_id(comp_vdev),
			     vdev_chan.ch_ieee,
			     vdev_chan.ch_phymode);
		qdf_nofl_err("fl %016llx, fl-ext %08x, s1 %d, s2 %d ",
			     vdev_chan.ch_flags, vdev_chan.ch_flagext,
			     vdev_chan.ch_freq_seg1,
			     vdev_chan.ch_freq_seg2);
		qdf_nofl_err("==> base vdev id: %d: ieee %d mode %d",
			     wlan_vdev_get_id(ch_filter->vdev),
			     iter_vdev_chan->ch_ieee,
			     iter_vdev_chan->ch_phymode);
		qdf_nofl_err("fl %016llx, fl-ext %08x s1 %d, s2 %d",
			     iter_vdev_chan->ch_flags,
			     iter_vdev_chan->ch_flagext,
			     iter_vdev_chan->ch_freq_seg1,
			     iter_vdev_chan->ch_freq_seg2);
	}
	wlan_vdev_obj_unlock(ch_filter->vdev);
}

QDF_STATUS wlan_util_pdev_vdevs_deschan_match(struct wlan_objmgr_pdev *pdev,
					      struct wlan_objmgr_vdev *vdev,
					      wlan_objmgr_ref_dbgid dbg_id)
{
	struct wlan_vdev_ch_check_filter ch_filter;

	if (!pdev)
		return QDF_STATUS_E_INVAL;

	if (wlan_pdev_nif_feat_cap_get(pdev, WLAN_PDEV_F_CHAN_CONCURRENCY))
		return QDF_STATUS_SUCCESS;

	if (wlan_objmgr_vdev_try_get_ref(vdev, dbg_id) == QDF_STATUS_SUCCESS) {
		ch_filter.flag = 0;
		ch_filter.vdev = vdev;

		wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
						  wlan_pdev_chan_match,
						  &ch_filter, 0, dbg_id);

		wlan_objmgr_vdev_release_ref(vdev, dbg_id);

		if (ch_filter.flag == 0)
			return QDF_STATUS_SUCCESS;
	}

	return QDF_STATUS_E_FAILURE;
}

static void wlan_vdev_restart_progress(struct wlan_objmgr_pdev *pdev,
				       void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	uint8_t *flag = (uint8_t *)arg;

	wlan_vdev_obj_lock(vdev);
	if (wlan_vdev_is_restart_progress(vdev) == QDF_STATUS_SUCCESS)
		*flag = 1;

	wlan_vdev_obj_unlock(vdev);
}

QDF_STATUS wlan_util_is_pdev_restart_progress(struct wlan_objmgr_pdev *pdev,
					      wlan_objmgr_ref_dbgid dbg_id)
{
	uint8_t flag = 0;

	if (!pdev)
		return QDF_STATUS_E_INVAL;

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_vdev_restart_progress,
					  &flag, 0, dbg_id);

	if (flag == 1)
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_INVAL;
}

static void wlan_vdev_scan_allowed(struct wlan_objmgr_pdev *pdev, void *object,
				   void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	uint8_t *flag = (uint8_t *)arg;

	wlan_vdev_obj_lock(vdev);
	if (wlan_vdev_mlme_is_scan_allowed(vdev) != QDF_STATUS_SUCCESS)
		*flag = 1;

	wlan_vdev_obj_unlock(vdev);
}

QDF_STATUS wlan_util_is_pdev_scan_allowed(struct wlan_objmgr_pdev *pdev,
					  wlan_objmgr_ref_dbgid dbg_id)
{
	uint8_t flag = 0;

	if (!pdev)
		return QDF_STATUS_E_INVAL;

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_vdev_scan_allowed,
					  &flag, 0, dbg_id);

	if (flag == 1)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

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

/**
 * wlan_util_get_mode_specific_peer_count - This api gives vdev mode specific
 * peer count`
 * @pdev: PDEV object
 * @object: vdev object
 * @arg: argument passed by caller
 *
 * Return: void
 */
static void
wlan_util_get_mode_specific_peer_count(struct wlan_objmgr_pdev *pdev,
				       void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = object;
	uint16_t temp_count = 0;
	struct wlan_op_mode_peer_count *count = arg;

	wlan_vdev_obj_lock(vdev);
	if (wlan_vdev_mlme_get_opmode(vdev) == count->opmode) {
		temp_count = wlan_vdev_get_peer_count(vdev);
		/* Decrement the self peer count */
		if (temp_count > 1)
			count->peer_count += (temp_count - 1);
	}
	wlan_vdev_obj_unlock(vdev);
}

uint16_t wlan_util_get_peer_count_for_mode(struct wlan_objmgr_pdev *pdev,
					   enum QDF_OPMODE mode)
{
	struct wlan_op_mode_peer_count count;

	count.opmode = mode;
	count.peer_count = 0;
	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
				wlan_util_get_mode_specific_peer_count, &count,
				0, WLAN_OBJMGR_ID);

	return count.peer_count;
}

#ifdef CONFIG_QCA_MINIDUMP
void wlan_minidump_log(void *start_addr, size_t size,
		       void *psoc_obj,
		       enum wlan_minidump_host_data type,
		       const char *name)
{
	int setval = 0;

	struct wlan_objmgr_psoc *psoc;

	if (!psoc_obj) {
		qdf_debug("Minidump: Psoc is NULL");
		return;
	}

	psoc = (struct wlan_objmgr_psoc *)psoc_obj;

	switch (type) {
	case WLAN_MD_CP_EXT_PDEV:
		if (cfg_get(psoc, CFG_OL_MD_CP_EXT_PDEV))
			setval = 1;
		break;
	case WLAN_MD_CP_EXT_PSOC:
		if (cfg_get(psoc, CFG_OL_MD_CP_EXT_PSOC))
			setval = 1;
		break;
	case WLAN_MD_CP_EXT_VDEV:
		if (cfg_get(psoc, CFG_OL_MD_CP_EXT_VDEV))
			setval = 1;
		break;
	case WLAN_MD_CP_EXT_PEER:
		if (cfg_get(psoc, CFG_OL_MD_CP_EXT_PEER))
			setval = 1;
		break;
	case WLAN_MD_DP_SOC:
		if (cfg_get(psoc, CFG_OL_MD_DP_SOC))
			setval = 1;
		break;
	case WLAN_MD_DP_PDEV:
		if (cfg_get(psoc, CFG_OL_MD_DP_PDEV))
			setval = 1;
		break;
	case WLAN_MD_DP_PEER:
		if (cfg_get(psoc, CFG_OL_MD_DP_PEER))
			setval = 1;
		break;
	case WLAN_MD_DP_SRNG_REO_DEST:
	case WLAN_MD_DP_SRNG_REO_EXCEPTION:
	case WLAN_MD_DP_SRNG_RX_REL:
	case WLAN_MD_DP_SRNG_REO_REINJECT:
	case WLAN_MD_DP_SRNG_REO_CMD:
	case WLAN_MD_DP_SRNG_REO_STATUS:
		if (cfg_get(psoc, CFG_OL_MD_DP_SRNG_REO))
			setval = 1;
		break;
	case WLAN_MD_DP_SRNG_TCL_DATA:
	case WLAN_MD_DP_SRNG_TCL_CMD:
	case WLAN_MD_DP_SRNG_TCL_STATUS:
	case WLAN_MD_DP_SRNG_TX_COMP:
		if (cfg_get(psoc, CFG_OL_MD_DP_SRNG_TCL))
			setval = 1;
		break;
	case WLAN_MD_DP_SRNG_WBM_DESC_REL:
	case WLAN_MD_DP_SRNG_WBM_IDLE_LINK:
		if (cfg_get(psoc, CFG_OL_MD_DP_SRNG_WBM))
			setval = 1;
		break;
	case WLAN_MD_DP_LINK_DESC_BANK:
		if (cfg_get(psoc, CFG_OL_MD_DP_LINK_DESC_BANK))
			setval = 1;
		break;
	case WLAN_MD_DP_SRNG_RXDMA_MON_BUF:
	case WLAN_MD_DP_SRNG_RXDMA_MON_DST:
	case WLAN_MD_DP_SRNG_RXDMA_MON_DESC:
	case WLAN_MD_DP_SRNG_RXDMA_ERR_DST:
	case WLAN_MD_DP_SRNG_RXDMA_MON_STATUS:
		if (cfg_get(psoc, CFG_OL_MD_DP_SRNG_RXDMA))
			setval = 1;
		break;
	case WLAN_MD_DP_HAL_SOC:
		if (cfg_get(psoc, CFG_OL_MD_DP_HAL_SOC))
			setval = 1;
		break;
	case WLAN_MD_OBJMGR_PSOC:
	case WLAN_MD_OBJMGR_PSOC_TGT_INFO:
		if (cfg_get(psoc, CFG_OL_MD_OBJMGR_PSOC))
			setval = 1;
		break;
	case WLAN_MD_OBJMGR_PDEV:
	case WLAN_MD_OBJMGR_PDEV_MLME:
		if (cfg_get(psoc, CFG_OL_MD_OBJMGR_PDEV))
			setval = 1;
		break;
	case WLAN_MD_OBJMGR_VDEV_MLME:
	case WLAN_MD_OBJMGR_VDEV_SM:
	case WLAN_MD_OBJMGR_VDEV:
		if (cfg_get(psoc, CFG_OL_MD_OBJMGR_VDEV))
			setval = 1;
		break;
	default:
		qdf_debug("Minidump: Type not implemented");
	}
	if (setval)
		qdf_minidump_log(start_addr, size, name);
}
qdf_export_symbol(wlan_minidump_log);

void wlan_minidump_remove(void *addr)
{
	qdf_minidump_remove(addr);
}
qdf_export_symbol(wlan_minidump_remove);
#else
void wlan_minidump_log(void *start_addr, size_t size,
		       void *psoc_obj,
		       enum wlan_minidump_host_data type,
		       const char *name) {}
qdf_export_symbol(wlan_minidump_log);

void wlan_minidump_remove(void *addr) {}
qdf_export_symbol(wlan_minidump_remove);
#endif /* CONFIG_QCA_MINIDUMP */
