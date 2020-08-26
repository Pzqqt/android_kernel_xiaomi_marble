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
 * DOC: Define APIs related to the Blacklist manager
 */

#ifndef _WLAN_CM_BLM_H_
#define _WLAN_CM_BLM_H_
#include <wlan_scan_utils_api.h>

/*
 * Flags used to set field APState
 */
#define AP_STATE_GOOD    0x00
#define AP_STATE_BAD     0x01
#define AP_STATE_RETRY   0x10


/**
 * enum cm_blm_exc_mac_mode - Exclude mac list mode
 * @CM_BLM_EXC_MAC_NONE - No entries in exclude mac list
 * @CM_BLM_EXC_MAC_FEW - Entries available in exclude mac list
 * @CM_BLM_EXC_MAC_ALL - Ignore all the mac
 */
enum cm_blm_exc_mac_mode {
	CM_BLM_EXC_MAC_NONE,
	CM_BLM_EXC_MAC_FEW,
	CM_BLM_EXC_MAC_ALL,
};

static inline uint32_t
wlan_cm_blm_scan_mlme_get_status(struct scan_cache_entry *scan_entry)
{
    return util_scan_entry_mlme_info(scan_entry)->status;
}

static inline void
wlan_cm_blm_scan_mlme_set_status(struct scan_cache_entry *scan_entry, uint32_t val)
{
    util_scan_entry_mlme_info(scan_entry)->status = val;
}

static inline qdf_time_t
wlan_cm_blm_scan_mlme_get_bad_ap_time(struct scan_cache_entry *scan_entry)
{
	return util_scan_entry_mlme_info(scan_entry)->bad_ap_time;
}

static inline void
wlan_cm_blm_scan_mlme_set_bad_ap_time(struct scan_cache_entry *scan_entry, qdf_time_t val)
{
	util_scan_entry_mlme_info(scan_entry)->bad_ap_time = val;
}

/*
 * wlan_cm_get_exc_mac_addr_list: Get excluded mac address list
 * @vdev: vdev object
 * @exc_mac_list: Pointer to the excluded mac address list
 * @exc_mac_count: Number of entries in the excluded list
 *
 * Return: Status of the excluded mac addresses
 */
enum cm_blm_exc_mac_mode wlan_cm_get_exc_mac_addr_list(
		struct wlan_objmgr_vdev *vdev,
		uint8_t (**exc_mac_list)[QDF_MAC_ADDR_SIZE],
		uint8_t *exc_mac_count);

/*
 * wlan_cm_get_bad_ap_timeout: Get bad ap timeout duration
 * @vdev: vdev object
 *
 * Return: Congfigured bad ap timeout value
 */
qdf_time_t wlan_cm_get_bad_ap_timeout(struct wlan_objmgr_vdev *vdev);
#endif
