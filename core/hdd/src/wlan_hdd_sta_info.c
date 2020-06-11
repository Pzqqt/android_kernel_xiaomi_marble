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

#include <wlan_hdd_includes.h>
#include "wlan_hdd_sta_info.h"

#define HDD_MAX_PEERS 32

QDF_STATUS hdd_sta_info_init(struct hdd_sta_info_obj *sta_info_container)
{
	if (!sta_info_container) {
		hdd_err("Parameter null");
		return QDF_STATUS_E_INVAL;
	}

	qdf_spinlock_create(&sta_info_container->sta_obj_lock);
	qdf_list_create(&sta_info_container->sta_obj, HDD_MAX_PEERS);

	return QDF_STATUS_SUCCESS;
}

void hdd_sta_info_deinit(struct hdd_sta_info_obj *sta_info_container)
{
	if (!sta_info_container) {
		hdd_err("Parameter null");
		return;
	}

	qdf_list_destroy(&sta_info_container->sta_obj);
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
	qdf_list_insert_front(&sta_info_container->sta_obj,
			      &sta_info->sta_node);

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

	hdd_put_sta_info_ref(sta_info_container, sta_info, false);

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

	qdf_list_for_each(&sta_info_container->sta_obj, sta_info, sta_node) {
		if (qdf_is_macaddr_equal(&sta_info->sta_mac,
					 (struct qdf_mac_addr *)mac_addr)) {
			hdd_take_sta_info_ref(sta_info_container,
					      sta_info, false);
			qdf_spin_unlock_bh(&sta_info_container->sta_obj_lock);
			return sta_info;
		}
	}

	qdf_spin_unlock_bh(&sta_info_container->sta_obj_lock);

	return NULL;
}

void hdd_take_sta_info_ref(struct hdd_sta_info_obj *sta_info_container,
			   struct hdd_station_info *sta_info,
			   bool lock_required)
{
	if (!sta_info_container || !sta_info) {
		hdd_err("Parameter(s) null");
		return;
	}

	if (lock_required)
		qdf_spin_lock_bh(&sta_info_container->sta_obj_lock);

	qdf_atomic_inc(&sta_info->ref_cnt);

	if (lock_required)
		qdf_spin_unlock_bh(&sta_info_container->sta_obj_lock);
}

void
hdd_put_sta_info_ref(struct hdd_sta_info_obj *sta_info_container,
		     struct hdd_station_info **sta_info, bool lock_required)
{
	struct hdd_station_info *info;
	struct qdf_mac_addr addr;

	if (!sta_info_container || !sta_info) {
		hdd_err("Parameter(s) null");
		return;
	}

	info = *sta_info;

	if (!info) {
		hdd_err("station info NULL");
		return;
	}

	if (lock_required)
		qdf_spin_lock_bh(&sta_info_container->sta_obj_lock);

	/*
	 * In case the put_ref is called more than twice for a single take_ref,
	 * this will result in either a BUG or page fault. In both the cases,
	 * the root cause would be known and the buggy put_ref can be taken
	 * care of.
	 */
	if (!qdf_atomic_read(&info->ref_cnt)) {
		hdd_alert("sta_info ref count is already 0");
		QDF_BUG(0);
	}

	qdf_atomic_dec(&info->ref_cnt);

	if (qdf_atomic_read(&info->ref_cnt)) {
		if (lock_required)
			qdf_spin_unlock_bh(&sta_info_container->sta_obj_lock);
		return;
	}

	qdf_copy_macaddr(&addr, &info->sta_mac);
	if (info->assoc_req_ies.len) {
		qdf_mem_free(info->assoc_req_ies.data);
		info->assoc_req_ies.data = NULL;
		info->assoc_req_ies.len = 0;
	}

	qdf_list_remove_node(&sta_info_container->sta_obj, &info->sta_node);
	qdf_mem_free(info);
	*sta_info = NULL;

	if (lock_required)
		qdf_spin_unlock_bh(&sta_info_container->sta_obj_lock);

	hdd_nofl_debug("STA_INFO: " QDF_MAC_ADDR_STR " freed",
		       QDF_MAC_ADDR_ARRAY(addr.bytes));
}

void hdd_clear_cached_sta_info(struct hdd_adapter *adapter)
{
	struct hdd_station_info *sta_info = NULL, *tmp = NULL;

	if (!adapter) {
		hdd_err("Parameter null");
		return;
	}

	hdd_for_each_sta_ref_safe(adapter->cache_sta_info_list, sta_info, tmp) {
		hdd_sta_info_detach(&adapter->cache_sta_info_list, &sta_info);
		hdd_put_sta_info_ref(&adapter->cache_sta_info_list, &sta_info,
				     true);
	}
}

QDF_STATUS
hdd_get_front_sta_info_no_lock(struct hdd_sta_info_obj *sta_info_container,
			       struct hdd_station_info **out_sta_info)
{
	QDF_STATUS status;
	qdf_list_node_t *node;

	*out_sta_info = NULL;

	status = qdf_list_peek_front(&sta_info_container->sta_obj, &node);

	if (QDF_IS_STATUS_ERROR(status))
		return status;

	*out_sta_info =
		qdf_container_of(node, struct hdd_station_info, sta_node);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
hdd_get_next_sta_info_no_lock(struct hdd_sta_info_obj *sta_info_container,
			      struct hdd_station_info *current_sta_info,
			      struct hdd_station_info **out_sta_info)
{
	QDF_STATUS status;
	qdf_list_node_t *node;

	if (!current_sta_info)
		return QDF_STATUS_E_INVAL;

	*out_sta_info = NULL;

	status = qdf_list_peek_next(&sta_info_container->sta_obj,
				    &current_sta_info->sta_node,
				    &node);

	if (QDF_IS_STATUS_ERROR(status))
		return status;

	*out_sta_info =
		qdf_container_of(node, struct hdd_station_info, sta_node);

	return QDF_STATUS_SUCCESS;
}

