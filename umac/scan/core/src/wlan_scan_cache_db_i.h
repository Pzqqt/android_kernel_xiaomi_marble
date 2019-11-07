/*
 * Copyright (c) 2017, 2019 The Linux Foundation. All rights reserved.
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
 * DOC: contains scan internal api
 */

#ifndef _WLAN_SCAN_CACHE_DB_I_H_
#define _WLAN_SCAN_CACHE_DB_I_H_

/**
 * scm_filter_match() - private API to check if entry is match to filter
 * psoc: psoc ptr;
 * @db_entry: db entry
 * @filter: filter
 * @security: negotiated security if match is found
 *
 * Return: true if entry match filter
 */
bool scm_filter_match(struct wlan_objmgr_psoc *psoc,
	struct scan_cache_entry *db_entry,
	struct scan_filter *filter,
	struct security_info *security);

/**
 * scm_is_better_bss() - Is bss1 better than bss2
 * @params: scan params
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
bool scm_is_better_bss(struct scan_default_params *params,
	struct scan_cache_entry *bss1,
	struct scan_cache_entry *bss2);

/**
 * scm_calculate_bss_score() - calculate BSS score used to get
 * the preference
 * @psoc: psoc ptr;
 * @params: scan params
 * @entry: scan entry for which score needs to be calculated
 * @pcl_chan_weight: weight for pcl channel
 *
 * Return: scan db for the pdev id
 */
int scm_calculate_bss_score(
		struct wlan_objmgr_psoc *psoc,
		struct scan_default_params *params,
		struct scan_cache_entry *entry,
		int pcl_chan_weight);

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
	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	return wlan_pdevid_get_scan_db(psoc, pdev_id);
}

/**
 * scm_get_pcl_weight_of_channel() - Get PCL weight if channel is present in pcl
 * @chan_freq: channel frequency of bss, unit: MHz
 * @filter: filter
 * @pcl_chan_weight: Get PCL weight for corresponding channel
 * @weight_list: Weight list for all the pcl channels.
 *
 * Get pcl_chan_weight if provided channel is present in pcl list
 *
 * Return: true or false
 */
bool scm_get_pcl_weight_of_channel(uint32_t chan_freq,
				   struct scan_filter *filter,
				   int *pcl_chan_weight,
				   uint8_t *weight_list);
#endif
