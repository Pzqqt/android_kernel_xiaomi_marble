/*
 * Copyright (c) 2017, 2019-2020 The Linux Foundation. All rights reserved.
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

#endif
