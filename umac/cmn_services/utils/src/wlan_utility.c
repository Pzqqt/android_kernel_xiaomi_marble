/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <wlan_serialization_api.h>

/* CRC polynomial 0xedb88320 */
static unsigned long const wlan_shortssid_table[] = {
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

uint32_t wlan_construct_shortssid(uint8_t *ssid, uint8_t ssid_len)
{
	uint32_t shortssid = 0xffffffff;
	uint8_t i;

	if (!ssid || ssid_len > WLAN_SSID_MAX_LEN)
		return shortssid;

	for (i = 0; i < ssid_len; i++)
		shortssid = wlan_shortssid_table[(shortssid ^ ssid[i]) & 0xff] ^
						(shortssid >> 8);
	return (shortssid ^ 0xffffffff);
}

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

void
wlan_get_320_center_freq(qdf_freq_t freq,
			 qdf_freq_t *center_freq1,
			 qdf_freq_t *center_freq2)
{
	*center_freq1 = 0;
	*center_freq2 = 0;

	if ((freq >= 5500) && (freq <= 5800)) {
		*center_freq1 = 5650;
	} else if ((freq >= 5955) && (freq <= 6095)) {
		*center_freq1 = 6105;
	} else if ((freq >= 6115) && (freq <= 6255)) {
		*center_freq1 = 6105;
		*center_freq2 = 6265;
	} else if ((freq >= 6275) && (freq <= 6415)) {
		*center_freq1 = 6265;
		*center_freq2 = 6425;
	} else if ((freq >= 6435) && (freq <= 6575)) {
		*center_freq1 = 6425;
		*center_freq2 = 6585;
	} else if ((freq >= 6595) && (freq <= 6735)) {
		*center_freq1 = 6585;
		*center_freq2 = 6745;
	} else if ((freq >= 6755) && (freq <= 6895)) {
		*center_freq1 = 6745;
		*center_freq2 = 6905;
	} else if ((freq >= 6915) && (freq <= 7055)) {
		*center_freq1 = 6905;
	}
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
	enum wlan_serialization_cmd_type cmd_type;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return;

	cmd_type = wlan_serialization_get_vdev_active_cmd_type(vdev);
	wlan_vdev_obj_lock(vdev);
	if ((wlan_vdev_mlme_is_init_state(vdev) != QDF_STATUS_SUCCESS) ||
	    (cmd_type == WLAN_SER_CMD_VDEV_START_BSS)) {
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
	enum wlan_serialization_cmd_type cmd_type;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return;

	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_SAP_MODE)
		return;

	cmd_type = wlan_serialization_get_vdev_active_cmd_type(vdev);
	wlan_vdev_obj_lock(vdev);
	if ((wlan_vdev_mlme_is_init_state(vdev) != QDF_STATUS_SUCCESS) ||
	    (cmd_type == WLAN_SER_CMD_VDEV_START_BSS)) {
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

#ifdef WLAN_FEATURE_11BE
static inline bool
wlan_chan_puncture_eq(struct wlan_channel *chan1, struct wlan_channel *chan2)
{
	if (chan1->puncture_bitmap == chan2->puncture_bitmap)
		return true;

	return false;
}
#else
static inline bool
wlan_chan_puncture_eq(struct wlan_channel *chan1, struct wlan_channel *chan2)
{
	return true;
}
#endif /* WLAN_FEATURE_11BE */

QDF_STATUS wlan_chan_eq(struct wlan_channel *chan1, struct wlan_channel *chan2)
{
	if ((chan1->ch_ieee == chan2->ch_ieee) &&
	    (chan1->ch_freq_seg2 == chan2->ch_freq_seg2) &&
	    wlan_chan_puncture_eq(chan1, chan2))
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

/**
 * struct wlan_check_bssid_context - bssid check context
 * @bssid: bssid to be checked
 * @connected: connected by vdev or not
 * @vdev_id: vdev id of connected vdev
 */
struct wlan_check_bssid_context {
	struct qdf_mac_addr bssid;
	bool connected;
	uint8_t vdev_id;
};

/**
 * wlan_get_connected_vdev_handler() - check vdev connected on bssid
 * @psoc: psoc object
 * @obj: vdev object
 * @args: handler context
 *
 * This function will check whether vdev is connected on bssid or not and
 * update the result to handler context accordingly.
 *
 * Return: void
 */
static void wlan_get_connected_vdev_handler(struct wlan_objmgr_psoc *psoc,
					    void *obj, void *args)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)obj;
	struct wlan_check_bssid_context *context =
				(struct wlan_check_bssid_context *)args;
	struct qdf_mac_addr bss_peer_mac;
	enum QDF_OPMODE op_mode;

	if (context->connected)
		return;
	op_mode = wlan_vdev_mlme_get_opmode(vdev);
	if (op_mode != QDF_STA_MODE && op_mode != QDF_P2P_CLIENT_MODE)
		return;
	if (wlan_vdev_is_up(vdev) != QDF_STATUS_SUCCESS)
		return;
	if (wlan_vdev_get_bss_peer_mac(vdev, &bss_peer_mac) !=
	    QDF_STATUS_SUCCESS)
		return;
	if (qdf_is_macaddr_equal(&bss_peer_mac, &context->bssid)) {
		context->connected = true;
		context->vdev_id = wlan_vdev_get_id(vdev);
	}
}

bool wlan_get_connected_vdev_from_psoc_by_bssid(struct wlan_objmgr_psoc *psoc,
						uint8_t *bssid,
						uint8_t *vdev_id)
{
	struct wlan_check_bssid_context context;

	qdf_mem_zero(&context, sizeof(struct wlan_check_bssid_context));
	qdf_mem_copy(context.bssid.bytes, bssid, QDF_MAC_ADDR_SIZE);
	wlan_objmgr_iterate_obj_list(psoc, WLAN_VDEV_OP,
				     wlan_get_connected_vdev_handler,
				     &context, true, WLAN_OSIF_SCAN_ID);
	if (context.connected)
		*vdev_id = context.vdev_id;

	return context.connected;
}

bool wlan_get_connected_vdev_by_bssid(struct wlan_objmgr_pdev *pdev,
				      uint8_t *bssid, uint8_t *vdev_id)
{
	return wlan_get_connected_vdev_from_psoc_by_bssid(
			wlan_pdev_get_psoc(pdev), bssid, vdev_id);
}

qdf_export_symbol(wlan_get_connected_vdev_by_bssid);

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * struct wlan_check_mld_addr_context - mld mac addr check context
 * @mld_addr: mld_addrto be checked
 * @connected: connected by vdev or not
 * @vdev_id: vdev id of connected vdev
 */
struct wlan_check_mld_addr_context {
	struct qdf_mac_addr mld_addr;
	bool connected;
	uint8_t vdev_id;
};

/**
 * wlan_get_connected_mlo_dev_ctx_handler() - check vdev connected on mld mac
 * @psoc: psoc object
 * @obj: vdev object
 * @args: handler context
 *
 * This function will check whether vdev is connected on mld mac or not and
 * update the result to handler context accordingly.
 *
 * Return: void
 */
static void wlan_get_connected_mlo_dev_ctx_handler(
			struct wlan_objmgr_psoc *psoc,
			void *obj, void *args)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)obj;
	struct wlan_check_mld_addr_context *context =
				(struct wlan_check_mld_addr_context *)args;
	struct qdf_mac_addr bss_peer_mld_mac;
	enum QDF_OPMODE op_mode;

	if (context->connected)
		return;
	op_mode = wlan_vdev_mlme_get_opmode(vdev);
	if (op_mode != QDF_STA_MODE && op_mode != QDF_P2P_CLIENT_MODE)
		return;
	if (wlan_vdev_is_up(vdev) != QDF_STATUS_SUCCESS)
		return;
	if (QDF_IS_STATUS_ERROR(wlan_vdev_get_bss_peer_mld_mac(
					vdev, &bss_peer_mld_mac)))
		return;
	if (qdf_is_macaddr_equal(&bss_peer_mld_mac, &context->mld_addr)) {
		context->connected = true;
		context->vdev_id = wlan_vdev_get_id(vdev);
	}
}

bool wlan_get_connected_vdev_by_mld_addr(struct wlan_objmgr_psoc *psoc,
					 uint8_t *mld_mac, uint8_t *vdev_id)
{
	struct wlan_check_mld_addr_context context;

	qdf_mem_zero(&context, sizeof(struct wlan_check_mld_addr_context));
	qdf_copy_macaddr(&context.mld_addr, (struct qdf_mac_addr *)mld_mac);
	wlan_objmgr_iterate_obj_list(psoc, WLAN_VDEV_OP,
				     wlan_get_connected_mlo_dev_ctx_handler,
				     &context, true, WLAN_MLME_OBJMGR_ID);

	if (context.connected)
		*vdev_id = context.vdev_id;

	return context.connected;
}
#endif

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
static bool wlan_minidump_log_enabled(struct wlan_objmgr_psoc *psoc,
				      enum wlan_minidump_host_data type)
{
	bool setval = false;

	switch (type) {
	case WLAN_MD_CP_EXT_PDEV:
		if (cfg_get(psoc, CFG_OL_MD_CP_EXT_PDEV))
			setval = true;
		break;
	case WLAN_MD_CP_EXT_PSOC:
		if (cfg_get(psoc, CFG_OL_MD_CP_EXT_PSOC))
			setval = true;
		break;
	case WLAN_MD_CP_EXT_VDEV:
		if (cfg_get(psoc, CFG_OL_MD_CP_EXT_VDEV))
			setval = true;
		break;
	case WLAN_MD_CP_EXT_PEER:
		if (cfg_get(psoc, CFG_OL_MD_CP_EXT_PEER))
			setval = true;
		break;
	case WLAN_MD_DP_SOC:
		if (cfg_get(psoc, CFG_OL_MD_DP_SOC))
			setval = true;
		break;
	case WLAN_MD_DP_PDEV:
		if (cfg_get(psoc, CFG_OL_MD_DP_PDEV))
			setval = true;
		break;
	case WLAN_MD_DP_PEER:
		if (cfg_get(psoc, CFG_OL_MD_DP_PEER))
			setval = true;
		break;
	case WLAN_MD_DP_SRNG_REO_DEST:
	case WLAN_MD_DP_SRNG_REO_EXCEPTION:
	case WLAN_MD_DP_SRNG_RX_REL:
	case WLAN_MD_DP_SRNG_REO_REINJECT:
	case WLAN_MD_DP_SRNG_REO_CMD:
	case WLAN_MD_DP_SRNG_REO_STATUS:
		if (cfg_get(psoc, CFG_OL_MD_DP_SRNG_REO))
			setval = true;
		break;
	case WLAN_MD_DP_SRNG_TCL_DATA:
	case WLAN_MD_DP_SRNG_TCL_CMD:
	case WLAN_MD_DP_SRNG_TCL_STATUS:
	case WLAN_MD_DP_SRNG_TX_COMP:
		if (cfg_get(psoc, CFG_OL_MD_DP_SRNG_TCL))
			setval = true;
		break;
	case WLAN_MD_DP_SRNG_WBM_DESC_REL:
	case WLAN_MD_DP_SRNG_WBM_IDLE_LINK:
		if (cfg_get(psoc, CFG_OL_MD_DP_SRNG_WBM))
			setval = true;
		break;
	case WLAN_MD_DP_LINK_DESC_BANK:
		if (cfg_get(psoc, CFG_OL_MD_DP_LINK_DESC_BANK))
			setval = true;
		break;
	case WLAN_MD_DP_SRNG_RXDMA_MON_BUF:
	case WLAN_MD_DP_SRNG_RXDMA_MON_DST:
	case WLAN_MD_DP_SRNG_RXDMA_MON_DESC:
	case WLAN_MD_DP_SRNG_RXDMA_ERR_DST:
	case WLAN_MD_DP_SRNG_RXDMA_MON_STATUS:
		if (cfg_get(psoc, CFG_OL_MD_DP_SRNG_RXDMA))
			setval = true;
		break;
	case WLAN_MD_DP_HAL_SOC:
		if (cfg_get(psoc, CFG_OL_MD_DP_HAL_SOC))
			setval = true;
		break;
	case WLAN_MD_OBJMGR_PSOC:
	case WLAN_MD_OBJMGR_PSOC_TGT_INFO:
		if (cfg_get(psoc, CFG_OL_MD_OBJMGR_PSOC))
			setval = true;
		break;
	case WLAN_MD_OBJMGR_PDEV:
	case WLAN_MD_OBJMGR_PDEV_MLME:
		if (cfg_get(psoc, CFG_OL_MD_OBJMGR_PDEV))
			setval = true;
		break;
	case WLAN_MD_OBJMGR_VDEV_MLME:
	case WLAN_MD_OBJMGR_VDEV_SM:
	case WLAN_MD_OBJMGR_VDEV:
		if (cfg_get(psoc, CFG_OL_MD_OBJMGR_VDEV))
			setval = true;
		break;
	default:
		qdf_debug("Minidump: Type not implemented");
	}

	return setval;
}
#else /* CONFIG_QCA_MINIDUMP */
static bool wlan_minidump_log_enabled(struct wlan_objmgr_psoc *psoc,
				      enum wlan_minidump_host_data type)
{
	return false;
}
#endif
void wlan_minidump_log(void *start_addr, const size_t size,
		       void *psoc_obj,
		       enum wlan_minidump_host_data type,
		       const char *name)
{
	struct wlan_objmgr_psoc *psoc;

	if (!psoc_obj) {
		qdf_debug("Minidump: Psoc is NULL");
		return;
	}

	psoc = (struct wlan_objmgr_psoc *)psoc_obj;

	if (psoc && wlan_minidump_log_enabled(psoc, type))
		qdf_minidump_log(start_addr, size, name);
}
qdf_export_symbol(wlan_minidump_log);

void wlan_minidump_remove(void *start_addr, const size_t size,
			  void *psoc_obj,
			  enum wlan_minidump_host_data type,
			  const char *name)
{
	struct wlan_objmgr_psoc *psoc;

	if (!psoc_obj) {
		qdf_debug("Minidump: Psoc is NULL");
		return;
	}

	psoc = (struct wlan_objmgr_psoc *)psoc_obj;

	if (psoc && wlan_minidump_log_enabled(psoc, type))
		qdf_minidump_remove(start_addr, size, name);
}
qdf_export_symbol(wlan_minidump_remove);
