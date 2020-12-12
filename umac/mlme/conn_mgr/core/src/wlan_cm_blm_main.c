/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: Implement APIs related to the Blacklist manager
 */
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_scan_utils_api.h>
#include <wlan_cm_bss_score_param.h>
#include <wlan_cm_blm.h>
#include "wlan_cm_blm_main.h"

static void blm_filter_vdev_mac_cmp(struct wlan_objmgr_pdev *pdev,
				    void *obj, void *args)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)obj;
	struct blm_entry_iter_obj *blm_obj = (struct blm_entry_iter_obj *)args;
	uint8_t *scan_entry_mac;

	if (wlan_vdev_mlme_get_opmode(vdev) == QDF_STA_MODE)
		blm_obj->sta_vdev = vdev;

	if (blm_obj->match)
		return;

	scan_entry_mac = util_scan_entry_macaddr(blm_obj->db_entry);
	if (qdf_mem_cmp(scan_entry_mac,
			wlan_vdev_mlme_get_macaddr(vdev),
			QDF_MAC_ADDR_SIZE) == 0) {
		blm_obj->match = true;
	}
}

enum cm_blacklist_action
wlan_blacklist_action_on_bssid(struct wlan_objmgr_pdev *pdev,
			       struct scan_cache_entry *entry)
{
	struct blm_entry_iter_obj blm_iter_obj = {0};
	uint8_t (*exc_mac_list)[QDF_MAC_ADDR_SIZE] = NULL;
	uint8_t *scan_entry_mac;
	struct wlan_objmgr_vdev *sta_vdev;
	enum cm_blm_exc_mac_mode exc_mac_status;
	uint8_t num_exc_mac = 0;
	qdf_time_t bad_ap_timeout = 0;
	qdf_time_t time_diff = 0;
	uint8_t idx;

	/*
	 * Avoid scan entry with bssid matching any of the vdev mac
	 */
	blm_iter_obj.db_entry = entry;
	blm_iter_obj.match = false;

	wlan_objmgr_pdev_iterate_obj_list(
			pdev, WLAN_VDEV_OP,
			blm_filter_vdev_mac_cmp,
			&blm_iter_obj, 0, WLAN_SCAN_ID);

	if (blm_iter_obj.match) {
		qdf_info("Ignore entry %pM match vdev mac", entry->bssid.bytes);
		return CM_BLM_FORCE_REMOVE;
	}

	if (!blm_iter_obj.sta_vdev)
		return CM_BLM_NO_ACTION;

	sta_vdev = blm_iter_obj.sta_vdev;

	/*
	 * Skip scan entry that is marked as BAD AP
	 */
	bad_ap_timeout  = wlan_cm_get_bad_ap_timeout(sta_vdev);
	if (wlan_cm_blm_scan_mlme_get_status(entry) & AP_STATE_BAD) {
		time_diff  = qdf_system_ticks() -
			wlan_cm_blm_scan_mlme_get_bad_ap_time(entry);
		if (!bad_ap_timeout ||
		    (qdf_system_ticks_to_msecs(time_diff) > bad_ap_timeout)) {
			wlan_cm_blm_scan_mlme_set_bad_ap_time(entry, 0);
			wlan_cm_blm_scan_mlme_set_status(entry,
							 AP_STATE_GOOD);
		} else {
			qdf_info("Ignore bssid entry %pM", entry->bssid.bytes);
			return CM_BLM_FORCE_REMOVE;
		}
	}

	/*
	 * Skip scan entries in the excluded mac address list
	 */
	exc_mac_status = wlan_cm_get_exc_mac_addr_list(sta_vdev,
						       &exc_mac_list,
						       &num_exc_mac);
	if (exc_mac_status == CM_BLM_EXC_MAC_ALL) {
		qdf_info("Ignore bssid entry %pM", entry->bssid.bytes);
		return CM_BLM_FORCE_REMOVE;
	} else if (exc_mac_status == CM_BLM_EXC_MAC_NONE) {
		return CM_BLM_NO_ACTION;
	}

	scan_entry_mac = util_scan_entry_macaddr(entry);
	for (idx = 0; idx < num_exc_mac; idx++) {
		if (qdf_mem_cmp(scan_entry_mac,
				exc_mac_list[idx],
				QDF_MAC_ADDR_SIZE) == 0) {
			qdf_info("Ignore bssid entry %pM", entry->bssid.bytes);
			return CM_BLM_FORCE_REMOVE;
		}
	}

	return CM_BLM_NO_ACTION;
}
