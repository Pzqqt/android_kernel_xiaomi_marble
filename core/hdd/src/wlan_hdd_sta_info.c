/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_sta_info.c
 *
 * Store and manage station info structure.
 *
 */

#include "wlan_hdd_sta_info.h"
#include <wlan_hdd_includes.h>

/* Generate a numeric constant to form the key to be provided for hashing */
#define WLAN_HDD_STA_INFO_HASH(addr) \
	(((const uint8_t *)addr)[QDF_MAC_ADDR_SIZE - 1])

QDF_STATUS hdd_sta_info_init(struct hdd_sta_info_obj *sta_info_container)
{
	if (!sta_info_container) {
		hdd_err("Parameter null");
		return QDF_STATUS_E_INVAL;
	}

	qdf_spinlock_create(&sta_info_container->sta_obj_lock);
	qdf_ht_init(sta_info_container->sta_obj);

	return QDF_STATUS_SUCCESS;
}

void hdd_sta_info_deinit(struct hdd_sta_info_obj *sta_info_container)
{
	if (!sta_info_container) {
		hdd_err("Parameter null");
		return;
	}

	qdf_ht_deinit(sta_info_container->sta_obj);
	qdf_spinlock_destroy(&sta_info_container->sta_obj_lock);
}

QDF_STATUS hdd_sta_info_attach(struct hdd_sta_info_obj *sta_info_container,
			       struct hdd_station_info *sta_info)
{
	if (!sta_info_container || !sta_info) {
		hdd_err("Parameter(s) null");
		return QDF_STATUS_E_INVAL;
	}

	qdf_spin_lock_bh(&sta_info_container->sta_obj_lock);

	qdf_atomic_set(&sta_info->ref_cnt, 1);
	qdf_ht_add(sta_info_container->sta_obj, &sta_info->sta_node,
		   WLAN_HDD_STA_INFO_HASH(sta_info->sta_mac.bytes));

	qdf_spin_unlock_bh(&sta_info_container->sta_obj_lock);

	return QDF_STATUS_SUCCESS;
}

void hdd_sta_info_detach(struct hdd_sta_info_obj *sta_info_container,
			 struct hdd_station_info **sta_info)
{
	struct hdd_station_info *info;

	if (!sta_info_container || !sta_info) {
		hdd_err("Parameter(s) null");
		return;
	}

	info = *sta_info;

	if (!info)
		return;

	qdf_spin_lock_bh(&sta_info_container->sta_obj_lock);

	qdf_ht_remove(&(info->sta_node));
	hdd_put_sta_info(sta_info_container, sta_info, false);

	qdf_spin_unlock_bh(&sta_info_container->sta_obj_lock);
}

struct hdd_station_info *hdd_get_sta_info_by_mac(
				struct hdd_sta_info_obj *sta_info_container,
				const uint8_t *mac_addr)
{
	struct hdd_station_info *sta_info = NULL;

	if (!mac_addr || !sta_info_container) {
		hdd_err("Parameter(s) null");
		return NULL;
	}

	qdf_spin_lock_bh(&sta_info_container->sta_obj_lock);

	qdf_ht_for_each_in_bucket(sta_info_container->sta_obj, sta_info,
				  sta_node, WLAN_HDD_STA_INFO_HASH(mac_addr)) {
		if (qdf_is_macaddr_equal(&sta_info->sta_mac,
					 (struct qdf_mac_addr *)mac_addr)) {
			qdf_atomic_inc(&sta_info->ref_cnt);
			qdf_spin_unlock_bh(&sta_info_container->sta_obj_lock);
			return sta_info;
		}
	}

	qdf_spin_unlock_bh(&sta_info_container->sta_obj_lock);

	return NULL;
}

void hdd_put_sta_info(struct hdd_sta_info_obj *sta_info_container,
		      struct hdd_station_info **sta_info, bool lock_required)
{
	struct hdd_station_info *info;

	if (!sta_info_container || !sta_info) {
		hdd_err("Parameter(s) null");
		return;
	}

	info = *sta_info;

	if (!info)
		return;

	if (lock_required)
		qdf_spin_lock_bh(&sta_info_container->sta_obj_lock);

	qdf_atomic_dec(&info->ref_cnt);

	if (qdf_atomic_read(&info->ref_cnt)) {
		if (lock_required)
			qdf_spin_unlock_bh(&sta_info_container->sta_obj_lock);
		return;
	}

	if (info->assoc_req_ies.len) {
		qdf_mem_free(info->assoc_req_ies.data);
		info->assoc_req_ies.data = NULL;
		info->assoc_req_ies.len = 0;
	}

	qdf_mem_free(info);
	*sta_info = NULL;

	if (lock_required)
		qdf_spin_unlock_bh(&sta_info_container->sta_obj_lock);
}

void hdd_clear_cached_sta_info(struct hdd_sta_info_obj *sta_info_container)
{
	struct hdd_station_info *sta_info = NULL;
	uint8_t index = 0;
	struct qdf_ht_entry *tmp;

	if (!sta_info_container) {
		hdd_err("Parameter null");
		return;
	}

	qdf_ht_for_each_safe(sta_info_container->sta_obj, index, tmp, sta_info,
			     sta_node) {
		if (sta_info) {
			hdd_sta_info_detach(sta_info_container, &sta_info);
		}
	}
}
