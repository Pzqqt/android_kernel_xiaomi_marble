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

/*
 * DOC: contains scan cache internal api
 */

#ifndef _WLAN_SCAN_CACHE_DB_I_H_
#define _WLAN_SCAN_CACHE_DB_I_H_

/**
 * scm_filter_match() - private API to check if entry is match to filter
 * @db_entry: db entry
 * @filter: filter
 * @security: negotiated security if match is found
 *
 * Return: true if entry match filter
 */
bool scm_filter_match(struct scan_cache_entry *db_entry,
	struct scan_filter *filter,
	struct security_info *security);

/**
 * scm_get_bss_prefer_value() - Get the preference value for BSS
 * @filter: scan filter
 * @entry: entry
 *
 * Each BSS should be assigned a preference value ranging from
 * 14-0, which will be used as an RSSI bucket score while sorting the
 * scan results.
 *
 * Return: Preference value for the BSSID
 */
uint32_t scm_get_bss_prefer_value(struct scan_filter *filter,
	struct scan_cache_entry *entry);

/**
 * scm_is_better_bss() - Is bss1 better than bss2
 * @filter: scan filter
 * @bss1: Pointer to the first BSS.
 * @bss2: Pointer to the second BSS.
 *
 * This routine helps in determining the preference value
 * of a particular BSS in the scan result which is further
 * used in the sorting logic of the final candidate AP's.
 *
 * Return: true, if bss1 is better than bss2
 *         false, if bss2 is better than bss1.
 */
bool scm_is_better_bss(struct scan_filter *filter,
	struct scan_cache_entry *bss1,
	struct scan_cache_entry *bss2);

/**
 * scm_get_bss_cap_value() - get bss capability value
 * @filter: filter
 * @entry: scan entry entry
 *
 * Return: CapValue base on the capabilities of a BSS
 */
uint32_t scm_get_bss_cap_value(struct scan_filter *filter,
	struct scan_cache_entry *entry);

/**
 * is_channel_found_in_pcl() - to check if channel is present in pcl
 * @channel_id: channel of bss
 * @filter: pointer to filter created through profile
 *
 * to check if provided channel is present in pcl
 *
 * Return: true or false
 */
static inline bool is_channel_found_in_pcl(int channel_id,
		struct scan_filter *filter)
{
	int i;
	bool status = false;

	if (!filter)
		return status;

	for (i = 0; i < filter->num_of_pcl_channels; i++) {
		if (filter->pcl_channel_list[i] == channel_id) {
			status = true;
			break;
		}
	}

	return status;
}

/**
 * scm_derive_prefer_value_from_rssi() - to derive prefer value
 * @filter: filter
 * @rssi: RSSI of the BSS
 *
 * This routine will derive preferred value from given rssi
 *
 * Return: value between 0 to 14
 */
static inline int scm_derive_prefer_value_from_rssi(struct scan_filter *filter,
	int rssi)
{
	int i = SCM_NUM_RSSI_CAT - 1, pref_val = 0;

	while (i >= 0) {
		if (rssi >= filter->rssi_cat[i]) {
			pref_val = filter->bss_prefer_val[i];
			break;
		}
		i--;
	};

	return pref_val;
}

/**
 * scm_calc_pref_val_by_pcl() - to calculate preferred value
 * @filter: filter to find match from scan result
 * @bss_descr: pointer to bss descriptor
 *
 * this routine calculates the new preferred value to be given to
 * provided bss if its channel falls under preferred channel list.
 * Thump rule is higer the RSSI better the boost.
 *
 * Return: success or failure
 */
QDF_STATUS scm_calc_pref_val_by_pcl(struct scan_filter *filter,
	struct scan_cache_entry *entry);

/**
 * wlan_pdevid_get_scan_db() - private API to get scan db from pdev id
 * @psoc: psoc object
 * @pdev_id: Pdev_id
 * Return: scan db for the pdev id
 */
static inline struct scan_dbs *
wlan_pdevid_get_scan_db(struct wlan_objmgr_psoc *psoc, uint8_t pdev_id)
{
	struct wlan_scan_obj *scan_obj = NULL;

	if (pdev_id > WLAN_UMAC_MAX_PDEVS) {
		scm_err("invalid pdev_id %d", pdev_id);
		return NULL;
	}
	scan_obj = wlan_psoc_get_scan_obj(psoc);

	if (!scan_obj)
		return NULL;

	return &(scan_obj->scan_db[pdev_id]);
}

/**
 * wlan_pdev_get_scan_db() - private API to get scan db from pdev
 * @psoc: psoc object
 * @pdev: Pdev
 *
 * Return: scan db for the pdev
 */
static inline struct scan_dbs *
wlan_pdev_get_scan_db(struct wlan_objmgr_psoc *psoc,
	struct wlan_objmgr_pdev *pdev)
{
	uint8_t pdev_id;

	if (!pdev) {
		scm_err("pdev is NULL");
		return NULL;
	}
	wlan_pdev_obj_lock(pdev);
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	wlan_pdev_obj_unlock(pdev);

	return wlan_pdevid_get_scan_db(psoc, pdev_id);
}
#endif
