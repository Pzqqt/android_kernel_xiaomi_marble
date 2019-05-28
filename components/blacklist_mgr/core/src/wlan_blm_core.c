/*
 * Copyright (c) 2011-2019 The Linux Foundation. All rights reserved.
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
 * DOC: declare internal APIs related to the blacklist component
 */

#include <wlan_objmgr_pdev_obj.h>
#include <wlan_blm_core.h>
#include <qdf_mc_timer.h>
#include <wlan_scan_public_structs.h>
#include <wlan_scan_utils_api.h>
#include "wlan_blm_tgt_api.h"

#define SECONDS_TO_MS(params)       (params * 60)
#define MINUTES_TO_MS(params)       (SECONDS_TO_MS(params) * 1000)

static void
blm_update_ap_info(struct blm_reject_ap *blm_entry, struct blm_config *cfg,
		   struct scan_cache_entry *scan_entry)
{
	qdf_time_t cur_timestamp = qdf_mc_timer_get_system_time();
	qdf_time_t entry_add_time = 0;

	blm_debug("%pM reject ap type = %x", blm_entry->bssid.bytes,
		  blm_entry->reject_ap_type);

	if (BLM_IS_AP_AVOIDED_BY_USERSPACE(blm_entry)) {
		entry_add_time =
			blm_entry->ap_timestamp.userspace_avoid_timestamp;

		if ((cur_timestamp - entry_add_time) >
		     MINUTES_TO_MS(cfg->avoid_list_exipry_time)) {
			/* Move AP to monitor list as avoid list time is over */
			blm_entry->userspace_avoidlist = false;
			blm_entry->driver_monitorlist = true;

			blm_entry->ap_timestamp.driver_monitor_timestamp =
								cur_timestamp;
			blm_debug("Userspace avoid list timer expired, moved to monitor list");
		}
	}

	if (BLM_IS_AP_AVOIDED_BY_DRIVER(blm_entry)) {
		entry_add_time = blm_entry->ap_timestamp.driver_avoid_timestamp;

		if ((cur_timestamp - entry_add_time) >
		     MINUTES_TO_MS(cfg->avoid_list_exipry_time)) {
			/* Move AP to monitor list as avoid list time is over */
			blm_entry->driver_avoidlist = false;
			blm_entry->driver_monitorlist = true;

			blm_entry->ap_timestamp.driver_monitor_timestamp =
								cur_timestamp;
			blm_debug("Driver avoid list timer expired, moved to monitor list");
		}
	}

	if (BLM_IS_AP_BLACKLISTED_BY_DRIVER(blm_entry)) {
		entry_add_time =
			blm_entry->ap_timestamp.driver_blacklist_timestamp;

		if ((cur_timestamp - entry_add_time) >
		     MINUTES_TO_MS(cfg->black_list_exipry_time)) {
			/* Move AP to monitor list as black list time is over */
			blm_entry->driver_blacklist = false;
			blm_entry->driver_monitorlist = true;

			blm_entry->ap_timestamp.driver_monitor_timestamp =
								cur_timestamp;
			blm_debug("Driver blacklist timer expired, moved to monitor list");
		}
	}

	if (BLM_IS_AP_IN_RSSI_REJECT_LIST(blm_entry)) {
		qdf_time_t entry_age = cur_timestamp -
			    blm_entry->ap_timestamp.rssi_reject_timestamp;

		if ((entry_age > blm_entry->rssi_reject_params.retry_delay) ||
		    (scan_entry && (scan_entry->rssi_raw > blm_entry->
					   rssi_reject_params.expected_rssi))) {
			/*
			 * Remove from the rssi reject list as:-
			 * 1. In case of OCE reject, both the time, and RSSI
			 *    param are present, and one of them have improved
			 *    now, so the STA can now connect to the AP.
			 *
			 * 2. In case of BTM message received from the FW,
			 *    the STA just needs to wait for a certain time,
			 *    hence RSSI is not a restriction (MIN RSSI needed
			 *    in that case is filled as 0).
			 *    Hence the above check will still pass, if BTM
			 *    delay is over, and will fail is not. RSSI check
			 *    for BTM message will fail (expected), as BTM does
			 *    not care about the same.
			 */
			blm_entry->rssi_reject_list = false;
			blm_debug("Remove from rssi reject expected RSSI = %d, current RSSI = %d, retry delay required = %d ms, delay = %lu ms",
				  blm_entry->rssi_reject_params.expected_rssi,
				  scan_entry->rssi_raw,
				  blm_entry->rssi_reject_params.retry_delay,
				  entry_age);
		}
	}
	blm_debug("%pM Updated reject ap type = %x", blm_entry->bssid.bytes,
		  blm_entry->reject_ap_type);
}

static enum blm_bssid_action
blm_prune_old_entries_and_get_action(struct blm_reject_ap *blm_entry,
				     struct blm_config *cfg,
				     struct scan_cache_entry *entry,
				     qdf_list_t *reject_ap_list)
{
	blm_update_ap_info(blm_entry, cfg, entry);

	/*
	 * If all entities have cleared the bits of reject ap type, then
	 * the AP is not needed in the database,(reject_ap_type should be 0),
	 * then remove the entry from the reject ap list.
	 */
	if (!blm_entry->reject_ap_type) {
		blm_debug("%pM cleared from list", blm_entry->bssid.bytes);
		qdf_list_remove_node(reject_ap_list, &blm_entry->node);
		qdf_mem_free(blm_entry);
		return BLM_ACTION_NOP;
	}

	if (BLM_IS_AP_IN_BLACKLIST(blm_entry))
		return BLM_REMOVE_FROM_LIST;

	if (BLM_IS_AP_IN_AVOIDLIST(blm_entry))
		return BLM_MOVE_AT_LAST;

	return BLM_ACTION_NOP;

}

static enum blm_bssid_action
blm_action_on_bssid(struct wlan_objmgr_pdev *pdev,
		    struct scan_cache_entry *entry)
{
	struct blm_pdev_priv_obj *blm_ctx;
	struct blm_psoc_priv_obj *blm_psoc_obj;
	struct blm_config *cfg;
	struct blm_reject_ap *blm_entry = NULL;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	QDF_STATUS status;
	enum blm_bssid_action action = BLM_ACTION_NOP;

	blm_ctx = blm_get_pdev_obj(pdev);
	blm_psoc_obj = blm_get_psoc_obj(wlan_pdev_get_psoc(pdev));
	if (!blm_ctx || !blm_psoc_obj) {
		blm_err("blm_ctx or blm_psoc_obj is NULL");
		return BLM_ACTION_NOP;
	}

	status = qdf_mutex_acquire(&blm_ctx->reject_ap_list_lock);
	if (QDF_IS_STATUS_ERROR(status)) {
		blm_err("failed to acquire reject_ap_list_lock");
		return BLM_ACTION_NOP;
	}

	cfg = &blm_psoc_obj->blm_cfg;

	qdf_list_peek_front(&blm_ctx->reject_ap_list, &cur_node);

	while (cur_node) {
		qdf_list_peek_next(&blm_ctx->reject_ap_list, cur_node,
				   &next_node);

		blm_entry = qdf_container_of(cur_node, struct blm_reject_ap,
					    node);

		if (qdf_is_macaddr_equal(&blm_entry->bssid, &entry->bssid)) {
			action = blm_prune_old_entries_and_get_action(blm_entry,
					cfg, entry, &blm_ctx->reject_ap_list);
			qdf_mutex_release(&blm_ctx->reject_ap_list_lock);
			return action;
		}
		cur_node = next_node;
		next_node = NULL;
	}

	qdf_mutex_release(&blm_ctx->reject_ap_list_lock);

	return BLM_ACTION_NOP;
}

static void
blm_modify_scan_list(qdf_list_t *scan_list,
		     struct scan_cache_node *scan_node,
		     enum blm_bssid_action action)
{
	blm_debug("%pM Action %d", scan_node->entry->bssid.bytes, action);

	switch (action) {
	case BLM_REMOVE_FROM_LIST:
		qdf_list_remove_node(scan_list, &scan_node->node);
		util_scan_free_cache_entry(scan_node->entry);
		qdf_mem_free(scan_node);
		break;

	case BLM_MOVE_AT_LAST:
		qdf_list_remove_node(scan_list, &scan_node->node);
		qdf_list_insert_back(scan_list, &scan_node->node);
		scan_node->entry->bss_score = 0;
		break;

	default:
		break;
	}
}

QDF_STATUS
blm_filter_bssid(struct wlan_objmgr_pdev *pdev, qdf_list_t *scan_list)
{
	struct scan_cache_node *scan_node = NULL;
	uint32_t scan_list_size;
	enum blm_bssid_action action;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;

	if (!scan_list || !qdf_list_size(scan_list)) {
		blm_err("Scan list is NULL or No BSSIDs present");
		return QDF_STATUS_E_EMPTY;
	}

	scan_list_size = qdf_list_size(scan_list);
	qdf_list_peek_front(scan_list, &cur_node);

	while (cur_node && scan_list_size) {
		qdf_list_peek_next(scan_list, cur_node, &next_node);

		scan_node = qdf_container_of(cur_node, struct scan_cache_node,
					    node);
		action = blm_action_on_bssid(pdev, scan_node->entry);
		if (action != BLM_ACTION_NOP)
			blm_modify_scan_list(scan_list, scan_node, action);
		cur_node = next_node;
		next_node = NULL;
		scan_list_size--;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
blm_add_bssid_to_reject_list(struct wlan_objmgr_pdev *pdev,
			     struct reject_ap_info *ap_info)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
blm_add_userspace_black_list(struct wlan_objmgr_pdev *pdev,
			     struct qdf_mac_addr *bssid_black_list,
			     uint8_t num_of_bssid)
{
	return QDF_STATUS_SUCCESS;
}

uint8_t
blm_get_bssid_reject_list(struct wlan_objmgr_pdev *pdev,
			  struct reject_ap_config_params *reject_list,
			  uint8_t max_bssid_to_be_filled,
			  enum blm_reject_ap_type reject_ap_type)
{
	return 0;
}
