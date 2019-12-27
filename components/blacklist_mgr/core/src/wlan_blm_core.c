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

#define SECONDS_TO_MS(params)       (params * 1000)
#define MINUTES_TO_MS(params)       (SECONDS_TO_MS(params) * 60)

static void
blm_update_ap_info(struct blm_reject_ap *blm_entry, struct blm_config *cfg,
		   struct scan_cache_entry *scan_entry)
{
	qdf_time_t cur_timestamp = qdf_mc_timer_get_system_time();
	qdf_time_t entry_add_time = 0;
	bool update_done = false;
	uint8_t old_reject_ap_type;

	old_reject_ap_type = blm_entry->reject_ap_type;

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
			update_done = true;
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
			update_done = true;
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
			update_done = true;
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
				  scan_entry ? scan_entry->rssi_raw : 0,
				  blm_entry->rssi_reject_params.retry_delay,
				  entry_age);
			update_done = true;
		}
	}

	if (!update_done)
		return;

	blm_debug("%pM Old %d Updated reject ap type = %x",
		  blm_entry->bssid.bytes, old_reject_ap_type,
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

static void
blm_handle_avoid_list(struct blm_reject_ap *entry,
		      struct blm_config *cfg,
		      struct reject_ap_info *ap_info)
{
	qdf_time_t cur_timestamp = qdf_mc_timer_get_system_time();

	if (ap_info->reject_ap_type == USERSPACE_AVOID_TYPE) {
		entry->userspace_avoidlist = true;
		entry->ap_timestamp.userspace_avoid_timestamp = cur_timestamp;
	} else if (ap_info->reject_ap_type == DRIVER_AVOID_TYPE) {
		entry->driver_avoidlist = true;
		entry->ap_timestamp.driver_avoid_timestamp = cur_timestamp;
	} else {
		return;
	}
	/* Update bssid info for new entry */
	entry->bssid = ap_info->bssid;

	/* Clear the monitor list bit if the AP was present in monitor list */
	entry->driver_monitorlist = false;

	/* Increment bad bssid counter as NUD failure happenend with this ap */
	entry->bad_bssid_counter++;

	/* If bad bssid counter has reached threshold, move it to blacklist */
	if (entry->bad_bssid_counter >= cfg->bad_bssid_counter_thresh) {
		if (ap_info->reject_ap_type == USERSPACE_AVOID_TYPE)
			entry->userspace_avoidlist = false;
		else if (ap_info->reject_ap_type == DRIVER_AVOID_TYPE)
			entry->driver_avoidlist = false;

		/* Move AP to blacklist list */
		entry->driver_blacklist = true;
		entry->ap_timestamp.driver_blacklist_timestamp = cur_timestamp;

		blm_debug("%pM moved to black list with counter %d",
			  entry->bssid.bytes, entry->bad_bssid_counter);
		return;
	}
	blm_debug("Added %pM to avoid list type %d, counter %d",
		  entry->bssid.bytes, ap_info->reject_ap_type,
		  entry->bad_bssid_counter);

	entry->connect_timestamp = qdf_mc_timer_get_system_time();
}

static void
blm_handle_blacklist(struct blm_reject_ap *entry,
		     struct reject_ap_info *ap_info)
{
	/*
	 * No entity will blacklist an AP internal to driver, so only
	 * userspace blacklist is the case to be taken care. Driver blacklist
	 * will only happen when the bad bssid counter has reached the max
	 * threshold.
	 */
	entry->bssid = ap_info->bssid;
	entry->userspace_blacklist = true;
	entry->ap_timestamp.userspace_blacklist_timestamp =
						qdf_mc_timer_get_system_time();

	blm_debug("%pM added to userspace blacklist", entry->bssid.bytes);
}

static void
blm_handle_rssi_reject_list(struct blm_reject_ap *entry,
			    struct reject_ap_info *ap_info)
{
	entry->bssid = ap_info->bssid;
	entry->rssi_reject_list = true;
	entry->ap_timestamp.rssi_reject_timestamp =
					qdf_mc_timer_get_system_time();
	entry->rssi_reject_params = ap_info->rssi_reject_params;

	blm_debug("%pM Added to rssi reject list, expected RSSI %d retry delay %d",
		  entry->bssid.bytes, entry->rssi_reject_params.expected_rssi,
		  entry->rssi_reject_params.retry_delay);
}

static void
blm_modify_entry(struct blm_reject_ap *entry, struct blm_config *cfg,
		 struct reject_ap_info *ap_info)
{
	/* Modify the entry according to the ap_info */
	switch (ap_info->reject_ap_type) {
	case USERSPACE_AVOID_TYPE:
	case DRIVER_AVOID_TYPE:
		blm_handle_avoid_list(entry, cfg, ap_info);
		break;
	case USERSPACE_BLACKLIST_TYPE:
		blm_handle_blacklist(entry, ap_info);
		break;
	case DRIVER_RSSI_REJECT_TYPE:
		blm_handle_rssi_reject_list(entry, ap_info);
		break;
	default:
		blm_debug("Invalid input of ap type %d",
			  ap_info->reject_ap_type);
	}
}

static bool
blm_is_bssid_present_only_in_list_type(enum blm_reject_ap_type list_type,
				       struct blm_reject_ap *blm_entry)
{
	switch (list_type) {
	case USERSPACE_AVOID_TYPE:
		return IS_AP_IN_USERSPACE_AVOID_LIST_ONLY(blm_entry);
	case USERSPACE_BLACKLIST_TYPE:
		return IS_AP_IN_USERSPACE_BLACKLIST_ONLY(blm_entry);
	case DRIVER_AVOID_TYPE:
		return IS_AP_IN_DRIVER_AVOID_LIST_ONLY(blm_entry);
	case DRIVER_BLACKLIST_TYPE:
		return IS_AP_IN_DRIVER_BLACKLIST_ONLY(blm_entry);
	case DRIVER_RSSI_REJECT_TYPE:
		return IS_AP_IN_RSSI_REJECT_LIST_ONLY(blm_entry);
	case DRIVER_MONITOR_TYPE:
		return IS_AP_IN_MONITOR_LIST_ONLY(blm_entry);
	default:
		blm_debug("Wrong list type %d passed", list_type);
		return false;
	}
}

static bool
blm_is_bssid_of_type(enum blm_reject_ap_type reject_ap_type,
		     struct blm_reject_ap *blm_entry)
{
	switch (reject_ap_type) {
	case USERSPACE_AVOID_TYPE:
		return BLM_IS_AP_AVOIDED_BY_USERSPACE(blm_entry);
	case USERSPACE_BLACKLIST_TYPE:
		return BLM_IS_AP_BLACKLISTED_BY_USERSPACE(blm_entry);
	case DRIVER_AVOID_TYPE:
		return BLM_IS_AP_AVOIDED_BY_DRIVER(blm_entry);
	case DRIVER_BLACKLIST_TYPE:
		return BLM_IS_AP_BLACKLISTED_BY_DRIVER(blm_entry);
	case DRIVER_RSSI_REJECT_TYPE:
		return BLM_IS_AP_IN_RSSI_REJECT_LIST(blm_entry);
	case DRIVER_MONITOR_TYPE:
		return BLM_IS_AP_IN_MONITOR_LIST(blm_entry);
	default:
		blm_err("Wrong list type %d passed", reject_ap_type);
		return false;
	}
}

static qdf_time_t
blm_get_delta_of_bssid(enum blm_reject_ap_type list_type,
		       struct blm_reject_ap *blm_entry,
		       struct blm_config *cfg)
{
	qdf_time_t cur_timestamp = qdf_mc_timer_get_system_time();

	/*
	 * For all the list types, delta would be the entry age only. Hence the
	 * oldest entry would be removed first in case of list is full, and the
	 * driver needs to make space for newer entries.
	 */

	switch (list_type) {
	case USERSPACE_AVOID_TYPE:
		return MINUTES_TO_MS(cfg->avoid_list_exipry_time) -
			(cur_timestamp -
			 blm_entry->ap_timestamp.userspace_avoid_timestamp);
	case USERSPACE_BLACKLIST_TYPE:
		return cur_timestamp -
			  blm_entry->ap_timestamp.userspace_blacklist_timestamp;
	case DRIVER_AVOID_TYPE:
		return MINUTES_TO_MS(cfg->avoid_list_exipry_time) -
			(cur_timestamp -
			 blm_entry->ap_timestamp.driver_avoid_timestamp);
	case DRIVER_BLACKLIST_TYPE:
		return MINUTES_TO_MS(cfg->black_list_exipry_time) -
			(cur_timestamp -
			 blm_entry->ap_timestamp.driver_blacklist_timestamp);

	/*
	 * For RSSI reject lowest delta would be the BSSID whose retry delay
	 * is about to expire, hence the delta would be remaining duration for
	 * de-blacklisting the AP from rssi reject list.
	 */
	case DRIVER_RSSI_REJECT_TYPE:
		return blm_entry->rssi_reject_params.retry_delay -
			(cur_timestamp -
				blm_entry->ap_timestamp.rssi_reject_timestamp);
	case DRIVER_MONITOR_TYPE:
		return cur_timestamp -
			       blm_entry->ap_timestamp.driver_monitor_timestamp;
	default:
		blm_debug("Wrong list type %d passed", list_type);
		return 0;
	}
}

static bool
blm_is_oldest_entry(enum blm_reject_ap_type list_type,
		    qdf_time_t cur_node_delta,
		    qdf_time_t oldest_node_delta)
{
	switch (list_type) {
	/*
	 * For RSSI reject, userspace avoid, driver avoid/blacklist type the
	 * lowest retry delay has to be found out hence if oldest_node_delta is
	 * 0, mean this is the first entry and thus return true, If
	 * oldest_node_delta is non zero, compare the delta and return true if
	 * the cur entry has lower retry delta.
	 */
	case DRIVER_RSSI_REJECT_TYPE:
	case USERSPACE_AVOID_TYPE:
	case DRIVER_AVOID_TYPE:
	case DRIVER_BLACKLIST_TYPE:
		if (!oldest_node_delta || (cur_node_delta < oldest_node_delta))
			return true;
		break;
	case USERSPACE_BLACKLIST_TYPE:
	case DRIVER_MONITOR_TYPE:
		if (cur_node_delta > oldest_node_delta)
			return true;
		break;
	default:
		blm_debug("Wrong list type passed %d", list_type);
		return false;
	}

	return false;
}

static QDF_STATUS
blm_try_delete_bssid_in_list(qdf_list_t *reject_ap_list,
			     enum blm_reject_ap_type list_type,
			     struct blm_config *cfg)
{
	struct blm_reject_ap *blm_entry = NULL;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	struct blm_reject_ap *oldest_blm_entry = NULL;
	qdf_time_t oldest_node_delta = 0;
	qdf_time_t cur_node_delta = 0;

	qdf_list_peek_front(reject_ap_list, &cur_node);

	while (cur_node) {
		qdf_list_peek_next(reject_ap_list, cur_node, &next_node);

		blm_entry = qdf_container_of(cur_node, struct blm_reject_ap,
					    node);

		if (blm_is_bssid_present_only_in_list_type(list_type,
							   blm_entry)) {
			cur_node_delta = blm_get_delta_of_bssid(list_type,
								blm_entry, cfg);

			if (blm_is_oldest_entry(list_type, cur_node_delta,
						oldest_node_delta)) {
				/* now this is the oldest entry*/
				oldest_blm_entry = blm_entry;
				oldest_node_delta = cur_node_delta;
			}
		}
		cur_node = next_node;
		next_node = NULL;
	}

	if (oldest_blm_entry) {
		/* Remove this entry to make space for the next entry */
		blm_debug("Removed %pM, type = %d",
			  oldest_blm_entry->bssid.bytes, list_type);
		qdf_list_remove_node(reject_ap_list, &oldest_blm_entry->node);
		qdf_mem_free(oldest_blm_entry);
		return QDF_STATUS_SUCCESS;
	}
	/* If the flow has reached here, that means no entry could be removed */

	return QDF_STATUS_E_FAILURE;
}

static QDF_STATUS
blm_remove_lowest_delta_entry(qdf_list_t *reject_ap_list,
			      struct blm_config *cfg)
{
	QDF_STATUS status;

	/*
	 * According to the Priority, the driver will try to remove the entries,
	 * as the least priority list, that is monitor list would not penalize
	 * the BSSIDs for connection. The priority order for the removal is:-
	 * 1. Monitor list
	 * 2. Driver avoid list
	 * 3. Userspace avoid list.
	 * 4. RSSI reject list.
	 * 5. Driver Blacklist.
	 * 6. Userspace Blacklist.
	 */

	status = blm_try_delete_bssid_in_list(reject_ap_list,
					      DRIVER_MONITOR_TYPE, cfg);
	if (QDF_IS_STATUS_SUCCESS(status))
		return QDF_STATUS_SUCCESS;

	status = blm_try_delete_bssid_in_list(reject_ap_list,
					      DRIVER_AVOID_TYPE, cfg);
	if (QDF_IS_STATUS_SUCCESS(status))
		return QDF_STATUS_SUCCESS;

	status = blm_try_delete_bssid_in_list(reject_ap_list,
					      USERSPACE_AVOID_TYPE, cfg);
	if (QDF_IS_STATUS_SUCCESS(status))
		return QDF_STATUS_SUCCESS;

	status = blm_try_delete_bssid_in_list(reject_ap_list,
					      DRIVER_RSSI_REJECT_TYPE, cfg);
	if (QDF_IS_STATUS_SUCCESS(status))
		return QDF_STATUS_SUCCESS;

	status = blm_try_delete_bssid_in_list(reject_ap_list,
					      DRIVER_BLACKLIST_TYPE, cfg);
	if (QDF_IS_STATUS_SUCCESS(status))
		return QDF_STATUS_SUCCESS;

	status = blm_try_delete_bssid_in_list(reject_ap_list,
					      USERSPACE_BLACKLIST_TYPE, cfg);
	if (QDF_IS_STATUS_SUCCESS(status))
		return QDF_STATUS_SUCCESS;

	blm_debug("Failed to remove AP from blacklist manager");

	return QDF_STATUS_E_FAILURE;
}

static void blm_fill_reject_list(qdf_list_t *reject_db_list,
				 struct reject_ap_config_params *reject_list,
				 uint8_t *num_of_reject_bssid,
				 enum blm_reject_ap_type reject_ap_type,
				 uint8_t max_bssid_to_be_filled,
				 struct blm_config *cfg)
{
	struct blm_reject_ap *blm_entry = NULL;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;

	qdf_list_peek_front(reject_db_list, &cur_node);
	while (cur_node) {
		if (*num_of_reject_bssid == max_bssid_to_be_filled) {
			blm_debug("Max size reached in list, reject_ap_type %d",
				  reject_ap_type);
			return;
		}
		qdf_list_peek_next(reject_db_list, cur_node, &next_node);

		blm_entry = qdf_container_of(cur_node, struct blm_reject_ap,
					    node);

		blm_update_ap_info(blm_entry, cfg, NULL);
		if (!blm_entry->reject_ap_type) {
			blm_debug("%pM cleared from list",
				  blm_entry->bssid.bytes);
			qdf_list_remove_node(reject_db_list, &blm_entry->node);
			qdf_mem_free(blm_entry);
			cur_node = next_node;
			next_node = NULL;
			continue;
		}

		if (blm_is_bssid_of_type(reject_ap_type, blm_entry)) {
			reject_list[*num_of_reject_bssid].expected_rssi =
				    blm_entry->rssi_reject_params.expected_rssi;
			reject_list[*num_of_reject_bssid].reject_duration =
			       blm_get_delta_of_bssid(reject_ap_type, blm_entry,
						      cfg);
			reject_list[*num_of_reject_bssid].reject_ap_type =
								reject_ap_type;
			reject_list[*num_of_reject_bssid].bssid =
							blm_entry->bssid;
			(*num_of_reject_bssid)++;
			blm_debug("Adding BSSID %pM of type %d to reject ap list, total entries added yet = %d",
				  blm_entry->bssid.bytes, reject_ap_type,
				  *num_of_reject_bssid);
		}
		cur_node = next_node;
		next_node = NULL;
	}
}

static void
blm_send_reject_ap_list_to_fw(struct wlan_objmgr_pdev *pdev,
			      qdf_list_t *reject_db_list,
			      struct blm_config *cfg)
{
	QDF_STATUS status;
	struct reject_ap_params reject_params = {0};

	reject_params.bssid_list =
			qdf_mem_malloc(sizeof(*reject_params.bssid_list) *
				       PDEV_MAX_NUM_BSSID_DISALLOW_LIST);
	if (!reject_params.bssid_list)
		return;

	/* The priority for filling is as below */
	blm_fill_reject_list(reject_db_list, reject_params.bssid_list,
			     &reject_params.num_of_reject_bssid,
			     USERSPACE_BLACKLIST_TYPE,
			     PDEV_MAX_NUM_BSSID_DISALLOW_LIST, cfg);
	blm_fill_reject_list(reject_db_list, reject_params.bssid_list,
			     &reject_params.num_of_reject_bssid,
			     DRIVER_BLACKLIST_TYPE,
			     PDEV_MAX_NUM_BSSID_DISALLOW_LIST, cfg);
	blm_fill_reject_list(reject_db_list, reject_params.bssid_list,
			     &reject_params.num_of_reject_bssid,
			     DRIVER_RSSI_REJECT_TYPE,
			     PDEV_MAX_NUM_BSSID_DISALLOW_LIST, cfg);
	blm_fill_reject_list(reject_db_list, reject_params.bssid_list,
			     &reject_params.num_of_reject_bssid,
			     USERSPACE_AVOID_TYPE,
			     PDEV_MAX_NUM_BSSID_DISALLOW_LIST, cfg);
	blm_fill_reject_list(reject_db_list, reject_params.bssid_list,
			     &reject_params.num_of_reject_bssid,
			     DRIVER_AVOID_TYPE,
			     PDEV_MAX_NUM_BSSID_DISALLOW_LIST, cfg);

	if (!reject_params.num_of_reject_bssid) {
		blm_debug("no candidate present in reject ap list.");
		qdf_mem_free(reject_params.bssid_list);
		return;
	}

	status = tgt_blm_send_reject_list_to_fw(pdev, &reject_params);

	if (QDF_IS_STATUS_ERROR(status))
		blm_err("failed to send the reject Ap list to FW");

	qdf_mem_free(reject_params.bssid_list);
}

QDF_STATUS
blm_add_bssid_to_reject_list(struct wlan_objmgr_pdev *pdev,
			     struct reject_ap_info *ap_info)
{
	struct blm_pdev_priv_obj *blm_ctx;
	struct blm_psoc_priv_obj *blm_psoc_obj;
	struct blm_config *cfg;
	struct blm_reject_ap *blm_entry;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	QDF_STATUS status;

	blm_ctx = blm_get_pdev_obj(pdev);
	blm_psoc_obj = blm_get_psoc_obj(wlan_pdev_get_psoc(pdev));

	if (!blm_ctx || !blm_psoc_obj) {
		blm_err("blm_ctx or blm_psoc_obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (qdf_is_macaddr_zero(&ap_info->bssid) ||
	    qdf_is_macaddr_group(&ap_info->bssid)) {
		blm_err("Zero/Broadcast BSSID received, entry not added");
		return QDF_STATUS_E_INVAL;
	}

	status = qdf_mutex_acquire(&blm_ctx->reject_ap_list_lock);
	if (QDF_IS_STATUS_ERROR(status)) {
		blm_err("failed to acquire reject_ap_list_lock");
		return status;
	}

	cfg = &blm_psoc_obj->blm_cfg;

	qdf_list_peek_front(&blm_ctx->reject_ap_list, &cur_node);

	while (cur_node) {
		qdf_list_peek_next(&blm_ctx->reject_ap_list,
				   cur_node, &next_node);

		blm_entry = qdf_container_of(cur_node, struct blm_reject_ap,
					    node);

		/* Update the AP info to the latest list first */
		blm_update_ap_info(blm_entry, cfg, NULL);
		if (!blm_entry->reject_ap_type) {
			blm_debug("%pM cleared from list",
				  blm_entry->bssid.bytes);
			qdf_list_remove_node(&blm_ctx->reject_ap_list,
					     &blm_entry->node);
			qdf_mem_free(blm_entry);
			cur_node = next_node;
			next_node = NULL;
			continue;
		}

		if (qdf_is_macaddr_equal(&blm_entry->bssid, &ap_info->bssid)) {
			blm_modify_entry(blm_entry, cfg, ap_info);
			goto end;
		}

		cur_node = next_node;
		next_node = NULL;
	}

	if (qdf_list_size(&blm_ctx->reject_ap_list) == MAX_BAD_AP_LIST_SIZE) {
		/* List is FULL, need to delete entries */
		status =
			blm_remove_lowest_delta_entry(&blm_ctx->reject_ap_list,
						      cfg);

		if (QDF_IS_STATUS_ERROR(status)) {
			qdf_mutex_release(&blm_ctx->reject_ap_list_lock);
			return status;
		}
	}

	blm_entry = qdf_mem_malloc(sizeof(*blm_entry));
	if (!blm_entry) {
		blm_err("Memory allocation of node failed");
		qdf_mutex_release(&blm_ctx->reject_ap_list_lock);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_list_insert_back(&blm_ctx->reject_ap_list, &blm_entry->node);
	blm_modify_entry(blm_entry, cfg, ap_info);

end:
	blm_send_reject_ap_list_to_fw(pdev, &blm_ctx->reject_ap_list, cfg);
	qdf_mutex_release(&blm_ctx->reject_ap_list_lock);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
blm_clear_userspace_blacklist_info(struct wlan_objmgr_pdev *pdev)
{
	struct blm_pdev_priv_obj *blm_ctx;
	struct blm_reject_ap *blm_entry;
	QDF_STATUS status;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;

	blm_ctx = blm_get_pdev_obj(pdev);
	if (!blm_ctx) {
		blm_err("blm_ctx is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = qdf_mutex_acquire(&blm_ctx->reject_ap_list_lock);
	if (QDF_IS_STATUS_ERROR(status)) {
		blm_err("failed to acquire reject_ap_list_lock");
		return QDF_STATUS_E_RESOURCES;
	}

	qdf_list_peek_front(&blm_ctx->reject_ap_list, &cur_node);

	while (cur_node) {
		qdf_list_peek_next(&blm_ctx->reject_ap_list, cur_node,
				   &next_node);
		blm_entry = qdf_container_of(cur_node, struct blm_reject_ap,
					    node);

		if (IS_AP_IN_USERSPACE_BLACKLIST_ONLY(blm_entry)) {
			blm_debug("removing bssid: %pM", blm_entry->bssid.bytes);
			qdf_list_remove_node(&blm_ctx->reject_ap_list,
					     &blm_entry->node);
			qdf_mem_free(blm_entry);
		} else if (BLM_IS_AP_BLACKLISTED_BY_USERSPACE(blm_entry)) {
			blm_debug("Clearing userspace blacklist bit for %pM",
				  blm_entry->bssid.bytes);
			blm_entry->userspace_blacklist = false;
		}
		cur_node = next_node;
		next_node = NULL;
	}
	qdf_mutex_release(&blm_ctx->reject_ap_list_lock);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
blm_add_userspace_black_list(struct wlan_objmgr_pdev *pdev,
			     struct qdf_mac_addr *bssid_black_list,
			     uint8_t num_of_bssid)
{
	uint8_t i = 0;
	struct reject_ap_info ap_info;
	QDF_STATUS status;
	struct blm_pdev_priv_obj *blm_ctx;
	struct blm_psoc_priv_obj *blm_psoc_obj;
	struct blm_config *cfg;

	blm_ctx = blm_get_pdev_obj(pdev);
	blm_psoc_obj = blm_get_psoc_obj(wlan_pdev_get_psoc(pdev));

	if (!blm_ctx || !blm_psoc_obj) {
		blm_err("blm_ctx or blm_psoc_obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

	/* Clear all the info of APs already existing in BLM first */
	blm_clear_userspace_blacklist_info(pdev);
	cfg = &blm_psoc_obj->blm_cfg;

	status = qdf_mutex_acquire(&blm_ctx->reject_ap_list_lock);
	if (QDF_IS_STATUS_ERROR(status)) {
		blm_err("failed to acquire reject_ap_list_lock");
		return status;
	}

	blm_send_reject_ap_list_to_fw(pdev, &blm_ctx->reject_ap_list, cfg);
	qdf_mutex_release(&blm_ctx->reject_ap_list_lock);

	if (!bssid_black_list || !num_of_bssid) {
		blm_debug("Userspace blacklist/num of blacklist NULL");
		return QDF_STATUS_SUCCESS;
	}

	for (i = 0; i < num_of_bssid; i++) {
		ap_info.bssid = bssid_black_list[i];
		ap_info.reject_ap_type = USERSPACE_BLACKLIST_TYPE;

		status = blm_add_bssid_to_reject_list(pdev, &ap_info);
		if (QDF_IS_STATUS_ERROR(status)) {
			blm_err("Failed to add bssid to userspace blacklist");
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}

void
blm_flush_reject_ap_list(struct blm_pdev_priv_obj *blm_ctx)
{
	struct blm_reject_ap *blm_entry = NULL;
	QDF_STATUS status;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;

	status = qdf_mutex_acquire(&blm_ctx->reject_ap_list_lock);
	if (QDF_IS_STATUS_ERROR(status)) {
		blm_err("failed to acquire reject_ap_list_lock");
		return;
	}

	qdf_list_peek_front(&blm_ctx->reject_ap_list, &cur_node);

	while (cur_node) {
		qdf_list_peek_next(&blm_ctx->reject_ap_list, cur_node,
				   &next_node);
		blm_entry = qdf_container_of(cur_node, struct blm_reject_ap,
					    node);
		qdf_list_remove_node(&blm_ctx->reject_ap_list,
				     &blm_entry->node);
		qdf_mem_free(blm_entry);
		cur_node = next_node;
		next_node = NULL;
	}

	blm_debug("BLM reject ap list flushed");
	qdf_mutex_release(&blm_ctx->reject_ap_list_lock);
}

uint8_t
blm_get_bssid_reject_list(struct wlan_objmgr_pdev *pdev,
			  struct reject_ap_config_params *reject_list,
			  uint8_t max_bssid_to_be_filled,
			  enum blm_reject_ap_type reject_ap_type)
{
	struct blm_pdev_priv_obj *blm_ctx;
	struct blm_psoc_priv_obj *blm_psoc_obj;
	uint8_t num_of_reject_bssid = 0;
	QDF_STATUS status;

	blm_ctx = blm_get_pdev_obj(pdev);
	blm_psoc_obj = blm_get_psoc_obj(wlan_pdev_get_psoc(pdev));

	if (!blm_ctx || !blm_psoc_obj) {
		blm_err("blm_ctx or blm_psoc_obj is NULL");
		return 0;
	}

	status = qdf_mutex_acquire(&blm_ctx->reject_ap_list_lock);
	if (QDF_IS_STATUS_ERROR(status)) {
		blm_err("failed to acquire reject_ap_list_lock");
		return 0;
	}

	blm_fill_reject_list(&blm_ctx->reject_ap_list, reject_list,
			     &num_of_reject_bssid, reject_ap_type,
			     max_bssid_to_be_filled, &blm_psoc_obj->blm_cfg);

	qdf_mutex_release(&blm_ctx->reject_ap_list_lock);

	return num_of_reject_bssid;
}

void
blm_update_bssid_connect_params(struct wlan_objmgr_pdev *pdev,
				struct qdf_mac_addr bssid,
				enum blm_connection_state con_state)
{
	struct blm_pdev_priv_obj *blm_ctx;
	struct blm_psoc_priv_obj *blm_psoc_obj;
	qdf_list_node_t *cur_node = NULL, *next_node = NULL;
	QDF_STATUS status;
	struct blm_reject_ap *blm_entry = NULL;
	qdf_time_t connection_age = 0;
	bool entry_found = false;
	qdf_time_t max_entry_time;

	blm_ctx = blm_get_pdev_obj(pdev);
	blm_psoc_obj = blm_get_psoc_obj(wlan_pdev_get_psoc(pdev));

	if (!blm_ctx || !blm_psoc_obj) {
		blm_err("blm_ctx or blm_psoc_obj is NULL");
		return;
	}

	status = qdf_mutex_acquire(&blm_ctx->reject_ap_list_lock);
	if (QDF_IS_STATUS_ERROR(status)) {
		blm_err("failed to acquire reject_ap_list_lock");
		return;
	}

	qdf_list_peek_front(&blm_ctx->reject_ap_list, &cur_node);

	while (cur_node) {
		qdf_list_peek_next(&blm_ctx->reject_ap_list, cur_node,
				   &next_node);
		blm_entry = qdf_container_of(cur_node, struct blm_reject_ap,
					     node);

		if (!qdf_mem_cmp(blm_entry->bssid.bytes, bssid.bytes,
				 QDF_MAC_ADDR_SIZE)) {
			blm_debug("%pM present in BLM reject list, updating connect info con_state = %d",
				  blm_entry->bssid.bytes, con_state);
			entry_found = true;
			break;
		}
		cur_node = next_node;
		next_node = NULL;
	}

	/* This means that the BSSID was not added in the reject list of BLM */
	if (!entry_found) {
		qdf_mutex_release(&blm_ctx->reject_ap_list_lock);
		return;
	}
	switch (con_state) {
	case BLM_AP_CONNECTED:
		blm_entry->connect_timestamp = qdf_mc_timer_get_system_time();
		break;
	case BLM_AP_DISCONNECTED:
		/* Update the blm info first */
		blm_update_ap_info(blm_entry, &blm_psoc_obj->blm_cfg, NULL);

		max_entry_time = blm_entry->connect_timestamp;
		if (blm_entry->driver_blacklist) {
			max_entry_time =
			   blm_entry->ap_timestamp.driver_blacklist_timestamp;
		} else if (blm_entry->driver_avoidlist) {
			max_entry_time =
			 QDF_MAX(blm_entry->ap_timestamp.driver_avoid_timestamp,
				 blm_entry->connect_timestamp);
		}
		connection_age = qdf_mc_timer_get_system_time() -
							max_entry_time;
		if ((connection_age >
		     SECONDS_TO_MS(blm_psoc_obj->blm_cfg.
				   bad_bssid_counter_reset_time))) {
			blm_entry->driver_avoidlist = false;
			blm_entry->driver_blacklist = false;
			blm_entry->driver_monitorlist = false;
			blm_entry->userspace_avoidlist = false;
			blm_debug("updated reject ap type %d ",
				  blm_entry->reject_ap_type);
			if (!blm_entry->reject_ap_type) {
				blm_debug("Bad Bssid timer expired/AP cleared from all blacklisting, removed %pM from list",
					  blm_entry->bssid.bytes);
				qdf_list_remove_node(&blm_ctx->reject_ap_list,
						     &blm_entry->node);
				qdf_mem_free(blm_entry);
				blm_send_reject_ap_list_to_fw(pdev,
					&blm_ctx->reject_ap_list,
					&blm_psoc_obj->blm_cfg);
			}
		}
		break;
	default:
		blm_debug("Invalid AP connection state recevied %d", con_state);
	};

	qdf_mutex_release(&blm_ctx->reject_ap_list_lock);
}
