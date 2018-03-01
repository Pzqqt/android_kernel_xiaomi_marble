/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
 * DOC: contains scan cache api and functionality
 * The Scan entries are protected by scan_db_lock. Holding the lock
 * for whole scan operation during get/flush scan results may take
 * more than 5 ms and thus ref count is used along with scan_db_lock.
 * Below are the operation on scan cache entry:
 * - While adding new node to the entry scan_db_lock is taken and ref_cnt
 *   is initialized and incremented.
 * - While reading the entry ref_cnt is incremented while holding the lock.
 * - Once reading operation is done ref_cnt is decremented while holding
 *   the lock.
 * - Once ref_cnt become 0 the node is deleted from the scan cache.
 */
#include <qdf_status.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_scan_public_structs.h>
#include <wlan_scan_utils_api.h>
#include "wlan_scan_main.h"
#include "wlan_scan_cache_db_i.h"
#include "wlan_reg_services_api.h"
#include "wlan_reg_ucfg_api.h"

/**
 * scm_del_scan_node() - API to remove scan node from the list
 * @list: hash list
 * @scan_node: node to be removed
 *
 * This should be called while holding scan_db_lock.
 *
 * Return: void
 */
static void scm_del_scan_node(qdf_list_t *list,
	struct scan_cache_node *scan_node)
{
	QDF_STATUS status;

	status = qdf_list_remove_node(list, &scan_node->node);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		util_scan_free_cache_entry(scan_node->entry);
		qdf_mem_free(scan_node);
	}
}

/**
 * scm_del_scan_node_from_db() - API to del the scan entry
 * @scan_db: scan database
 * @scan_entry:entry scan_node
 *
 * API to flush the scan entry. This should be called while
 * holding scan_db_lock.
 *
 * Return: QDF status.
 */
static QDF_STATUS scm_del_scan_node_from_db(struct scan_dbs *scan_db,
	struct scan_cache_node *scan_node)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t hash_idx;

	if (!scan_node)
		return QDF_STATUS_E_INVAL;

	hash_idx = SCAN_GET_HASH(scan_node->entry->bssid.bytes);
	scm_del_scan_node(&scan_db->scan_hash_tbl[hash_idx], scan_node);
	scan_db->num_entries--;

	return status;
}

/**
 * scm_scan_entry_get_ref() - api to increase ref count of scan entry
 * @scan_node: scan node
 *
 * Return: void
 */
static void scm_scan_entry_get_ref(struct scan_cache_node *scan_node)
{
	if (scan_node == NULL) {
		scm_err("scan_node is NULL");
		QDF_ASSERT(0);
		return;
	}
	qdf_atomic_inc(&scan_node->ref_cnt);
}

/**
 * scm_scan_entry_put_ref() - Api to decrease ref count of scan entry
 * and free if it become 0
 * @scan_db: scan database
 * @scan_node: scan node
 * @lock_needed: if scan_db_lock is needed
 * @delete: logically delete the entry
 *
 * Return: void
 */
static void scm_scan_entry_put_ref(struct scan_dbs *scan_db,
	struct scan_cache_node *scan_node, bool lock_needed, bool delete)
{

	if (!scan_node) {
		scm_err("scan_node is NULL");
		QDF_ASSERT(0);
		return;
	}

	if (lock_needed)
		qdf_spin_lock_bh(&scan_db->scan_db_lock);

	if (delete && !scan_node->active) {
		if (lock_needed)
			qdf_spin_unlock_bh(&scan_db->scan_db_lock);
		scm_warn("node is already deleted");
		return;
	}

	if (!qdf_atomic_read(&scan_node->ref_cnt)) {
		if (lock_needed)
			qdf_spin_unlock_bh(&scan_db->scan_db_lock);
		scm_err("scan_node ref cnt is 0");
		QDF_ASSERT(0);
		return;
	}

	if (delete)
		scan_node->active = false;

	/* Decrement ref count, free scan_node, if ref count == 0 */
	if (qdf_atomic_dec_and_test(&scan_node->ref_cnt))
		scm_del_scan_node_from_db(scan_db, scan_node);

	if (lock_needed)
		qdf_spin_unlock_bh(&scan_db->scan_db_lock);
}

/**
 * scm_add_scan_node() - API to add scan node
 * @scan_db: data base
 * @scan_node: node to be removed
 *
 * Return: void
 */
static void scm_add_scan_node(struct scan_dbs *scan_db,
	struct scan_cache_node *scan_node)
{
	uint8_t hash_idx;

	hash_idx =
		SCAN_GET_HASH(scan_node->entry->bssid.bytes);

	qdf_spin_lock_bh(&scan_db->scan_db_lock);
	qdf_atomic_init(&scan_node->ref_cnt);
	scan_node->active = true;
	scm_scan_entry_get_ref(scan_node);
	qdf_list_insert_back(&scan_db->scan_hash_tbl[hash_idx],
			&scan_node->node);
	scan_db->num_entries++;
	qdf_spin_unlock_bh(&scan_db->scan_db_lock);
}


/**
 * scm_get_next_valid_node() - API get the next valid scan node from
 * the list
 * @list: hash list
 * @cur_node: current node pointer
 *
 * API to get next active node from the list. If cur_node is NULL
 * it will return first node of the list.
 * Call must be protected by scan_db->scan_db_lock
 *
 * Return: next scan node
 */
static qdf_list_node_t *
scm_get_next_valid_node(qdf_list_t *list,
	qdf_list_node_t *cur_node)
{
	qdf_list_node_t *next_node = NULL;
	qdf_list_node_t *temp_node = NULL;
	struct scan_cache_node *scan_node;

	if (cur_node)
		qdf_list_peek_next(list, cur_node, &next_node);
	else
		qdf_list_peek_front(list, &next_node);

	while (next_node) {
		scan_node = qdf_container_of(next_node,
			struct scan_cache_node, node);
		if (scan_node->active)
			return next_node;
		/*
		 * If node is not valid check for next entry
		 * to get next valid node.
		 */
		qdf_list_peek_next(list, next_node, &temp_node);
		next_node = temp_node;
		temp_node = NULL;
	}

	return next_node;
}

/**
 * scm_get_next_node() - API get the next scan node from
 * the list
 * @scan_db: scan data base
 * @list: hash list
 * @cur_node: current node pointer
 *
 * API get the next node from the list. If cur_node is NULL
 * it will return first node of the list
 *
 * Return: next scan cache node
 */
static struct scan_cache_node *
scm_get_next_node(struct scan_dbs *scan_db,
	qdf_list_t *list, struct scan_cache_node *cur_node)
{
	struct scan_cache_node *next_node = NULL;
	qdf_list_node_t *next_list = NULL;

	qdf_spin_lock_bh(&scan_db->scan_db_lock);
	if (cur_node) {
		next_list = scm_get_next_valid_node(list, &cur_node->node);
		/* Decrement the ref count of the previous node */
		scm_scan_entry_put_ref(scan_db,
			cur_node, false, false);
	} else {
		next_list = scm_get_next_valid_node(list, NULL);
	}
	/* Increase the ref count of the obtained node */
	if (next_list) {
		next_node = qdf_container_of(next_list,
			struct scan_cache_node, node);
		scm_scan_entry_get_ref(next_node);
	}
	qdf_spin_unlock_bh(&scan_db->scan_db_lock);

	return next_node;
}

/**
 * scm_check_and_age_out() - check and age out the old entries
 * @scan_db: scan db
 * @scan_node: node to check for age out
 * @scan_aging_time: scan cache aging time
 *
 * Return: void
 */
static void scm_check_and_age_out(struct scan_dbs *scan_db,
	struct scan_cache_node *node,
	uint32_t scan_aging_time)
{
	if (util_scan_entry_age(node->entry) >=
	   scan_aging_time) {
		scm_info("Aging out BSSID: %pM with age %d ms",
			node->entry->bssid.bytes,
			util_scan_entry_age(node->entry));
		scm_scan_entry_put_ref(scan_db, node, true, true);
	}
}

void scm_age_out_entries(struct wlan_objmgr_psoc *psoc,
	struct scan_dbs *scan_db)
{
	int i;
	struct scan_cache_node *cur_node = NULL;
	struct scan_cache_node *next_node = NULL;
	struct scan_default_params *def_param;

	def_param = wlan_scan_psoc_get_def_params(psoc);
	if (!def_param) {
		scm_err("wlan_scan_psoc_get_def_params failed");
		return;
	}

	for (i = 0 ; i < SCAN_HASH_SIZE; i++) {
		cur_node = scm_get_next_node(scan_db,
			&scan_db->scan_hash_tbl[i], NULL);
		while (cur_node) {
			scm_check_and_age_out(scan_db, cur_node,
					def_param->scan_cache_aging_time);
			next_node = scm_get_next_node(scan_db,
				&scan_db->scan_hash_tbl[i], cur_node);
			cur_node = next_node;
			next_node = NULL;
		}
	}
}

/**
 * scm_flush_oldest_entry() - flust out the oldest entry
 * @scan_db: scan db from which oldest etry needs to be flushed
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS scm_flush_oldest_entry(struct scan_dbs *scan_db)
{
	int i;
	struct scan_cache_node *oldest_node = NULL;
	struct scan_cache_node *cur_node;
	qdf_list_node_t *cur_list;

	qdf_spin_lock_bh(&scan_db->scan_db_lock);
	for (i = 0 ; i < SCAN_HASH_SIZE; i++) {
		/* Get the first valid node for the hash */
		cur_list = scm_get_next_valid_node(&scan_db->scan_hash_tbl[i],
						   NULL);
		/*
		 * Check only the first valid node if present as new
		 * entry are added to tail and thus first valid
		 * node is the oldest
		 */
		if (cur_list) {
			cur_node = qdf_container_of(cur_list,
				struct scan_cache_node, node);
			if (!oldest_node ||
			   (util_scan_entry_age(oldest_node->entry) <
			   util_scan_entry_age(cur_node->entry)))
				oldest_node = cur_node;
		}
	}

	if (oldest_node) {
		scm_debug("Flush oldest BSSID: %pM with age %d ms",
				oldest_node->entry->bssid.bytes,
				util_scan_entry_age(oldest_node->entry));
		scm_scan_entry_put_ref(scan_db, oldest_node, false, true);
	}
	qdf_spin_unlock_bh(&scan_db->scan_db_lock);

	return QDF_STATUS_SUCCESS;
}

/**
 * scm_update_alt_wcn_ie() - update the alternate WCN IE
 * @from: copy from
 * @dst: copy to
 *
 * Return: void
 */
static void scm_update_alt_wcn_ie(struct scan_cache_entry *from,
	struct scan_cache_entry *dst)
{
	uint32_t alt_wcn_ie_len;

	if (from->frm_subtype == dst->frm_subtype)
		return;

	if (!from->ie_list.wcn && !dst->ie_list.wcn)
		return;

	/* Existing WCN IE is empty. */
	if (!from->ie_list.wcn)
		return;

	alt_wcn_ie_len = 2 + from->ie_list.wcn[1];
	if (alt_wcn_ie_len > WLAN_MAX_IE_LEN + 2) {
		scm_err("invalid IE len");
		return;
	}

	if (!dst->alt_wcn_ie.ptr) {
		/* allocate this additional buffer for alternate WCN IE */
		dst->alt_wcn_ie.ptr = qdf_mem_malloc(WLAN_MAX_IE_LEN + 2);
		if (!dst->alt_wcn_ie.ptr) {
			scm_err("failed to allocate memory");
			return;
		}
	}
	qdf_mem_copy(dst->alt_wcn_ie.ptr,
		from->ie_list.wcn, alt_wcn_ie_len);
	dst->alt_wcn_ie.len = alt_wcn_ie_len;
}

/**
 * scm_add_scan_entry() - add new scan entry to the database
 * @scan_db: scan database
 * @scan_params: new entry to be added
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS scm_add_scan_entry(struct scan_dbs *scan_db,
	struct scan_cache_entry *scan_params)
{
	struct scan_cache_node *scan_node;
	QDF_STATUS status;

	if (scan_db->num_entries >= MAX_SCAN_CACHE_SIZE) {
		status = scm_flush_oldest_entry(scan_db);
		if (QDF_IS_STATUS_ERROR(status))
			return status;
	}

	scan_node = qdf_mem_malloc(sizeof(*scan_node));
	if (!scan_node)
		return QDF_STATUS_E_NOMEM;

	scan_node->entry = scan_params;
	scm_add_scan_node(scan_db, scan_node);

	return QDF_STATUS_SUCCESS;
}

/**
 * scm_update_mlme_info() - update mlme info
 * @src: source scan entry
 * @dest: destination scan entry
 *
 * Return: void
 */
static inline void
scm_update_mlme_info(struct scan_cache_entry *src,
	struct scan_cache_entry *dest)
{
	qdf_mem_copy(&dest->mlme_info, &src->mlme_info,
		sizeof(struct mlme_info));
}

/**
 * scm_delete_duplicate_entry() - remove duplicate node entry
 * @scan_db: scan database
 * @scan_params: new entry to be added
 * @scan_node: old entry to removed
 *
 * Remove duplicate node after copying required
 * info into new entry
 *
 * Return: void
 */
static void scm_delete_duplicate_entry(struct scan_dbs *scan_db,
	struct scan_cache_entry *scan_params,
	struct scan_cache_node *scan_node)
{
	struct scan_cache_entry *scan_entry;
	uint64_t time_gap;

	scan_entry = scan_node->entry;
	/* If old entry have the ssid but new entry does not */
	if (!scan_params->ssid.length && scan_entry->ssid.length) {
		/*
		 * New entry has a hidden SSID and old one has the SSID.
		 * Add the entry by using the ssid of the old entry
		 * only if diff of saved SSID time and current time is
		 * less than HIDDEN_SSID_TIME time.
		 * This will avoid issues in case AP changes its SSID
		 * while remain hidden.
		 */
		time_gap =
			qdf_mc_timer_get_system_time() -
			scan_entry->hidden_ssid_timestamp;
		if (time_gap <= HIDDEN_SSID_TIME) {
			scan_params->hidden_ssid_timestamp =
				scan_entry->hidden_ssid_timestamp;
			scan_params->ssid.length =
				scan_entry->ssid.length;
			qdf_mem_copy(scan_params->ssid.ssid,
				scan_entry->ssid.ssid,
				scan_params->ssid.length);
		}
	}

	/*
	 * Due to Rx sensitivity issue, sometime beacons are seen on adjacent
	 * channel so workaround in software is needed. If DS params or HT info
	 * are present driver can get proper channel info from these IEs and set
	 * channel_mismatch so that the older RSSI values are used in new entry.
	 *
	 * For the cases where DS params and HT info is not present, driver
	 * needs to check below conditions to get proper channel and set
	 * channel_mismatch so that the older RSSI values are used in new entry:
	 *   -- The old entry channel and new entry channel are not same
	 *   -- RSSI is less than -80, this indicate that the signal has leaked
	 *       in adjacent channel.
	 */
	if ((scan_params->frm_subtype == MGMT_SUBTYPE_BEACON) &&
	    !util_scan_entry_htinfo(scan_params) &&
	    !util_scan_entry_ds_param(scan_params) &&
	    (scan_params->channel.chan_idx != scan_entry->channel.chan_idx) &&
	    (scan_params->rssi_raw  < ADJACENT_CHANNEL_RSSI_THRESHOLD)) {
		scan_params->channel.chan_idx = scan_entry->channel.chan_idx;
		scan_params->channel_mismatch = true;
	}

	/* Use old value for rssi if beacon was heard on adjacent channel. */
	if (scan_params->channel_mismatch) {
		scan_params->rssi_raw = scan_entry->rssi_raw;
		scan_params->avg_rssi = scan_entry->avg_rssi;
		scan_params->rssi_timestamp =
			scan_entry->rssi_timestamp;
	} else {
		/* If elapsed time since last rssi update for this
		 * entry is smaller than a thresold, calculate a
		 * running average of the RSSI values.
		 * Otherwise new frames RSSI is more representive
		 * of the signal strength.
		 */
		time_gap =
			scan_params->scan_entry_time -
			scan_entry->rssi_timestamp;
		if (time_gap > WLAN_RSSI_AVERAGING_TIME)
			scan_params->avg_rssi =
				WLAN_RSSI_IN(scan_params->rssi_raw);
		else {
			/* Copy previous average rssi to new entry */
			scan_params->avg_rssi = scan_entry->avg_rssi;
			/* Average with previous samples */
			WLAN_RSSI_LPF(scan_params->avg_rssi,
					scan_params->rssi_raw);
		}

		scan_params->rssi_timestamp = scan_params->scan_entry_time;
	}

	/* copy wsn ie from scan_entry to scan_params*/
	scm_update_alt_wcn_ie(scan_entry, scan_params);

	/* copy mlme info from scan_entry to scan_params*/
	scm_update_mlme_info(scan_entry, scan_params);

	/* Mark delete the duplicate node */
	scm_scan_entry_put_ref(scan_db, scan_node, true, true);
}

/**
 * scm_find_duplicate_and_del() - find duplicate entry if present
 * and update it
 * @scan_db: scan db
 * @entry: input scan cache entry
 *
 * Return: true if entry is found and updated else false
 */
static bool
scm_find_duplicate_and_del(struct scan_dbs *scan_db,
	struct scan_cache_entry *entry)
{
	uint8_t hash_idx;
	struct scan_cache_node *cur_node;
	struct scan_cache_node *next_node = NULL;

	hash_idx = SCAN_GET_HASH(entry->bssid.bytes);

	cur_node = scm_get_next_node(scan_db,
		   &scan_db->scan_hash_tbl[hash_idx], NULL);

	while (cur_node) {
		if (util_is_scan_entry_match(entry,
		   cur_node->entry)) {
			scm_delete_duplicate_entry(scan_db,
				entry, cur_node);
			scm_scan_entry_put_ref(scan_db,
				cur_node, true, false);
			return true;
		}
		next_node = scm_get_next_node(scan_db,
			 &scan_db->scan_hash_tbl[hash_idx], cur_node);
		cur_node = next_node;
		next_node = NULL;
	}

	return false;
}

/**
 * scm_add_update_entry() - add or update scan entry
 * @psoc: psoc ptr
 * @pdev: pdev pointer
 * @scan_params: new received entry
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS scm_add_update_entry(struct wlan_objmgr_psoc *psoc,
	struct wlan_objmgr_pdev *pdev, struct scan_cache_entry *scan_params)
{
	QDF_STATUS status;
	struct scan_dbs *scan_db;
	struct wlan_scan_obj *scan_obj;

	scan_db = wlan_pdev_get_scan_db(psoc, pdev);
	if (!scan_db) {
		scm_err("scan_db is NULL");
		return QDF_STATUS_E_INVAL;
	}

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("scan_obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (scan_params->frm_subtype ==
	   MGMT_SUBTYPE_PROBE_RESP &&
	   !scan_params->ie_list.ssid)
		scm_info("Probe resp doesnt contain SSID");


	if (scan_params->ie_list.csa ||
	   scan_params->ie_list.xcsa ||
	   scan_params->ie_list.cswrp)
		scm_info("CSA IE present for BSSID: %pM",
			scan_params->bssid.bytes);

	scm_find_duplicate_and_del(scan_db, scan_params);

	if (scan_obj->cb.inform_beacon)
		scan_obj->cb.inform_beacon(pdev, scan_params);

	status = scm_add_scan_entry(scan_db, scan_params);

	return status;
}

QDF_STATUS scm_handle_bcn_probe(struct scheduler_msg *msg)
{
	struct scan_bcn_probe_event *bcn;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct scan_cache_entry *scan_entry;
	struct wlan_scan_obj *scan_obj;
	qdf_list_t *scan_list = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t list_count, i;
	qdf_list_node_t *next_node = NULL;
	struct scan_cache_node *scan_node;

	bcn = msg->bodyptr;
	if (!bcn) {
		scm_err("bcn is NULL");
		return QDF_STATUS_E_INVAL;
	}
	if (!bcn->rx_data) {
		scm_err("rx_data iS NULL");
		status = QDF_STATUS_E_INVAL;
		goto free_nbuf;
	}
	if (!bcn->buf) {
		scm_err("buf is NULL");
		status = QDF_STATUS_E_INVAL;
		goto free_nbuf;
	}

	psoc = bcn->psoc;
	pdev = wlan_objmgr_get_pdev_by_id(psoc,
			   bcn->rx_data->pdev_id, WLAN_SCAN_ID);
	if (!pdev) {
		scm_err("pdev is NULL");
		status = QDF_STATUS_E_INVAL;
		goto free_nbuf;
	}
	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("scan_obj is NULL");
		status = QDF_STATUS_E_INVAL;
		goto free_nbuf;
	}

	if (qdf_nbuf_len(bcn->buf) <=
	   (sizeof(struct wlan_frame_hdr) +
	   offsetof(struct wlan_bcn_frame, ie))) {
		scm_err("invalid beacon/probe length");
		status = QDF_STATUS_E_INVAL;
		goto free_nbuf;
	}

	scan_list =
		 util_scan_unpack_beacon_frame(qdf_nbuf_data(bcn->buf),
			qdf_nbuf_len(bcn->buf), bcn->frm_type,
			bcn->rx_data);
	if (!scan_list || qdf_list_empty(scan_list)) {
		scm_err("failed to unpack frame");
		status = QDF_STATUS_E_INVAL;
		goto free_nbuf;
	}

	list_count = qdf_list_size(scan_list);
	for (i = 0; i < list_count; i++) {
		status = qdf_list_remove_front(scan_list, &next_node);
		if (QDF_IS_STATUS_ERROR(status) || next_node == NULL) {
			scm_err("failed to unpack frame");
			status = QDF_STATUS_E_INVAL;
			goto free_nbuf;
		}

		scan_node = qdf_container_of(next_node,
			struct scan_cache_node, node);

		scan_entry = scan_node->entry;

		scm_debug("Received %s from BSSID: %pM tsf_delta = %u Seq Num: %x  ssid:%.*s, rssi: %d",
			  (bcn->frm_type == MGMT_SUBTYPE_PROBE_RESP) ?
			  "Probe Rsp" : "Beacon", scan_entry->bssid.bytes,
			  scan_entry->tsf_delta, scan_entry->seq_num,
			  scan_entry->ssid.length, scan_entry->ssid.ssid,
			  scan_entry->rssi_raw);

		if (scan_obj->drop_bcn_on_chan_mismatch &&
			scan_entry->channel_mismatch) {
			scm_debug("Drop frame, as channel mismatch Received for from BSSID: %pM ",
				   scan_entry->bssid.bytes);
			util_scan_free_cache_entry(scan_entry);
			qdf_mem_free(scan_node);
			continue;
		}

		if (scan_obj->cb.update_beacon)
			scan_obj->cb.update_beacon(pdev, scan_entry);

		if (wlan_reg_11d_enabled_on_host(psoc))
			scm_11d_handle_country_info(psoc, pdev, scan_entry);

		status = scm_add_update_entry(psoc, pdev, scan_entry);
		if (QDF_IS_STATUS_ERROR(status)) {
			scm_debug("failed to add entry for BSSID: %pM",
				  scan_entry->bssid.bytes);
			util_scan_free_cache_entry(scan_entry);
			qdf_mem_free(scan_node);
			continue;
		}

		qdf_mem_free(scan_node);
	}

free_nbuf:
	if (scan_list)
		qdf_mem_free(scan_list);
	if (bcn->psoc)
		wlan_objmgr_psoc_release_ref(bcn->psoc, WLAN_SCAN_ID);
	if (pdev)
		wlan_objmgr_pdev_release_ref(pdev, WLAN_SCAN_ID);
	if (bcn->rx_data)
		qdf_mem_free(bcn->rx_data);
	if (bcn->buf)
		qdf_nbuf_free(bcn->buf);
	qdf_mem_free(bcn);

	return status;
}

/**
 * scm_list_insert_sorted() - add the entries in scan_list in sorted way
 * @psoc: psoc ptr
 * @filter: scan filter
 * @scan_node: node entry to be inserted
 * @scan_list: Temp scan list
 *
 * Add the entries in scan_list in sorted way considering
 * cap_val and prefer val. The node is copy of original scan entry and
 * thus no lock is required.
 *
 * Return: void
 */
static void scm_list_insert_sorted(struct wlan_objmgr_psoc *psoc,
	struct scan_filter *filter,
	struct scan_cache_node *scan_node,
	qdf_list_t *scan_list)
{
	struct scan_cache_node *cur_node;
	qdf_list_node_t *cur_lst = NULL, *next_lst = NULL;
	struct scan_default_params *params;
	int pcl_chan_weight = 0;

	params = wlan_scan_psoc_get_def_params(psoc);
	if (!params) {
		scm_err("wlan_scan_psoc_get_def_params failed");
		return;
	}

	if (filter->num_of_pcl_channels > 0 &&
			(scan_node->entry->rssi_raw > SCM_PCL_RSSI_THRESHOLD)) {
		if (scm_get_pcl_weight_of_channel(
					scan_node->entry->channel.chan_idx,
					filter, &pcl_chan_weight,
					filter->pcl_weight_list)) {
			scm_debug("pcl channel %d pcl_chan_weight %d",
					scan_node->entry->channel.chan_idx,
					pcl_chan_weight);
		}
	}
	if (params->is_bssid_hint_priority &&
	    !qdf_mem_cmp(filter->bssid_hint.bytes,
			 scan_node->entry->bssid.bytes,
			 QDF_MAC_ADDR_SIZE))
		scan_node->entry->bss_score = BEST_CANDIDATE_MAX_BSS_SCORE;
	else
		scm_calculate_bss_score(psoc, params,
					scan_node->entry, pcl_chan_weight);

	if (qdf_list_empty(scan_list)) {
		qdf_list_insert_front(scan_list, &scan_node->node);
		return;
	}

	qdf_list_peek_front(scan_list, &cur_lst);

	while (cur_lst) {
		cur_node = qdf_container_of(cur_lst,
				struct scan_cache_node, node);
		if (scm_is_better_bss(params,
		   scan_node->entry, cur_node->entry)) {
			qdf_list_insert_before(scan_list,
				&scan_node->node,
				&cur_node->node);
			break;
		}
		qdf_list_peek_next(scan_list,
			cur_lst, &next_lst);
		cur_lst = next_lst;
		next_lst = NULL;
	}

	if (!cur_lst)
		qdf_list_insert_back(scan_list,
			&scan_node->node);

}

/**
 * scm_scan_apply_filter_get_entry() - apply filter and get the
 * scan entry
 * @psoc: psoc pointer
 * @db_entry: scan entry
 * @filter: filter to be applied
 * @scan_list: scan list to which entry is added
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
scm_scan_apply_filter_get_entry(struct wlan_objmgr_psoc *psoc,
	struct scan_cache_entry *db_entry,
	struct scan_filter *filter,
	qdf_list_t *scan_list)
{
	struct scan_cache_node *scan_node = NULL;
	struct security_info security = {0};
	bool match;

	if (!filter)
		match = true;
	else
		match = scm_filter_match(psoc, db_entry,
					filter, &security);

	if (!match)
		return QDF_STATUS_SUCCESS;

	scan_node = qdf_mem_malloc(sizeof(*scan_node));
	if (!scan_node)
		return QDF_STATUS_E_NOMEM;

	scan_node->entry =
		util_scan_copy_cache_entry(db_entry);

	if (!scan_node->entry) {
		qdf_mem_free(scan_node);
		return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(&scan_node->entry->neg_sec_info,
		&security, sizeof(scan_node->entry->neg_sec_info));

	if (!filter || !filter->bss_scoring_required)
		qdf_list_insert_front(scan_list,
			&scan_node->node);
	else
		scm_list_insert_sorted(psoc, filter, scan_node, scan_list);

	return QDF_STATUS_SUCCESS;
}

/**
 * scm_get_results() - Iterate and get scan results
 * @psoc: psoc ptr
 * @scan_db: scan db
 * @filter: filter to be applied
 * @scan_list: scan list to which entry is added
 *
 * Return: void
 */
static void scm_get_results(struct wlan_objmgr_psoc *psoc,
	struct scan_dbs *scan_db, struct scan_filter *filter,
	qdf_list_t *scan_list)
{
	int i, count;
	struct scan_cache_node *cur_node;
	struct scan_cache_node *next_node = NULL;

	for (i = 0 ; i < SCAN_HASH_SIZE; i++) {
		cur_node = scm_get_next_node(scan_db,
			   &scan_db->scan_hash_tbl[i], NULL);
		count = qdf_list_size(&scan_db->scan_hash_tbl[i]);
		if (!count)
			continue;
		while (cur_node) {
			scm_scan_apply_filter_get_entry(psoc,
				cur_node->entry, filter, scan_list);
			next_node = scm_get_next_node(scan_db,
				&scan_db->scan_hash_tbl[i], cur_node);
			cur_node = next_node;
		}
	}
}

QDF_STATUS scm_purge_scan_results(qdf_list_t *scan_list)
{
	QDF_STATUS status;
	struct scan_cache_node *cur_node;
	qdf_list_node_t *cur_lst = NULL, *next_lst = NULL;

	if (!scan_list) {
		scm_err("scan_result is NULL");
		return QDF_STATUS_E_INVAL;
	}

	status = qdf_list_peek_front(scan_list, &cur_lst);

	while (cur_lst) {
		qdf_list_peek_next(
			scan_list, cur_lst, &next_lst);
		cur_node = qdf_container_of(cur_lst,
			struct scan_cache_node, node);
		status = qdf_list_remove_node(scan_list,
					cur_lst);
		if (QDF_IS_STATUS_SUCCESS(status)) {
			util_scan_free_cache_entry(cur_node->entry);
			qdf_mem_free(cur_node);
		}
		cur_lst = next_lst;
		next_lst = NULL;
	}

	qdf_list_destroy(scan_list);
	qdf_mem_free(scan_list);

	return status;
}

qdf_list_t *scm_get_scan_result(struct wlan_objmgr_pdev *pdev,
	struct scan_filter *filter)
{
	struct wlan_objmgr_psoc *psoc;
	struct scan_dbs *scan_db;
	qdf_list_t *tmp_list;

	if (!pdev) {
		scm_err("pdev is NULL");
		return NULL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		scm_err("psoc is NULL");
		return NULL;
	}

	scan_db = wlan_pdev_get_scan_db(psoc, pdev);
	if (!scan_db) {
		scm_err("scan_db is NULL");
		return NULL;
	}

	tmp_list = qdf_mem_malloc(sizeof(*tmp_list));
	if (!tmp_list) {
		scm_err("failed tp allocate scan_result");
		return NULL;
	}
	qdf_list_create(tmp_list,
			MAX_SCAN_CACHE_SIZE);
	scm_age_out_entries(psoc, scan_db);
	scm_get_results(psoc, scan_db, filter, tmp_list);

	return tmp_list;
}

/**
 * scm_iterate_db_and_call_func() - iterate and call the func
 * @scan_db: scan db
 * @func: func to be called
 * @arg: func arg
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
scm_iterate_db_and_call_func(struct scan_dbs *scan_db,
	scan_iterator_func func, void *arg)
{
	int i;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct scan_cache_node *cur_node;
	struct scan_cache_node *next_node = NULL;

	if (!func)
		return QDF_STATUS_E_INVAL;

	for (i = 0 ; i < SCAN_HASH_SIZE; i++) {
		cur_node = scm_get_next_node(scan_db,
			&scan_db->scan_hash_tbl[i], NULL);
		while (cur_node) {
			status = func(arg, cur_node->entry);
			if (QDF_IS_STATUS_ERROR(status)) {
				scm_scan_entry_put_ref(scan_db,
					cur_node, true, false);
				return status;
			}
			next_node = scm_get_next_node(scan_db,
				&scan_db->scan_hash_tbl[i], cur_node);
			cur_node = next_node;
		}
	}

	return status;
}

QDF_STATUS
scm_iterate_scan_db(struct wlan_objmgr_pdev *pdev,
	scan_iterator_func func, void *arg)
{
	struct wlan_objmgr_psoc *psoc;
	struct scan_dbs *scan_db;
	QDF_STATUS status;

	if (!func) {
		scm_err("func is NULL");
		return QDF_STATUS_E_INVAL;
	}

	if (!pdev) {
		scm_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		scm_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}
	scan_db = wlan_pdev_get_scan_db(psoc, pdev);
	if (!scan_db) {
		scm_err("scan_db is NULL");
		return QDF_STATUS_E_INVAL;
	}

	scm_age_out_entries(psoc, scan_db);
	status = scm_iterate_db_and_call_func(scan_db, func, arg);

	return status;
}

/**
 * scm_scan_apply_filter_flush_entry() -flush scan entries depending
 * on filter
 * @psoc: psoc ptr
 * @scan_db: scan db
 * @db_node: node on which filters are applied
 * @filter: filter to be applied
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
scm_scan_apply_filter_flush_entry(struct wlan_objmgr_psoc *psoc,
	struct scan_dbs *scan_db,
	struct scan_cache_node *db_node,
	struct scan_filter *filter)
{
	struct security_info security = {0};
	bool match;

	if (!filter)
		match = true;
	else
		match = scm_filter_match(psoc, db_node->entry,
					filter, &security);

	if (!match)
		return QDF_STATUS_SUCCESS;

	scm_scan_entry_put_ref(scan_db, db_node, true, true);

	return QDF_STATUS_SUCCESS;
}

/**
 * scm_flush_scan_entries() - API to flush scan entries depending on filters
 * @psoc: psoc ptr
 * @scan_db: scan db
 * @filter: filter
 *
 * Return: void
 */
static void scm_flush_scan_entries(struct wlan_objmgr_psoc *psoc,
	struct scan_dbs *scan_db,
	struct scan_filter *filter)
{
	int i;
	struct scan_cache_node *cur_node;
	struct scan_cache_node *next_node = NULL;

	for (i = 0 ; i < SCAN_HASH_SIZE; i++) {
		cur_node = scm_get_next_node(scan_db,
			   &scan_db->scan_hash_tbl[i], NULL);
		while (cur_node) {
			scm_scan_apply_filter_flush_entry(psoc, scan_db,
				cur_node, filter);
			next_node = scm_get_next_node(scan_db,
				&scan_db->scan_hash_tbl[i], cur_node);
			cur_node = next_node;
		}
	}
}

QDF_STATUS scm_flush_results(struct wlan_objmgr_pdev *pdev,
	struct scan_filter *filter)
{
	struct wlan_objmgr_psoc *psoc;
	struct scan_dbs *scan_db;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	if (!pdev) {
		scm_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		scm_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	scan_db = wlan_pdev_get_scan_db(psoc, pdev);
	if (!scan_db) {
		scm_err("scan_db is NULL");
		return QDF_STATUS_E_INVAL;
	}

	scm_flush_scan_entries(psoc, scan_db, filter);

	return status;
}

/**
 * scm_filter_channels() - Remove entries not belonging to channel list
 * @scan_db: scan db
 * @db_node: node on which filters are applied
 * @chan_list: valid channel list
 * @num_chan: number of channels
 *
 * Return: QDF_STATUS
 */
static void scm_filter_channels(struct scan_dbs *scan_db,
	struct scan_cache_node *db_node,
	uint8_t *chan_list, uint32_t num_chan)
{
	int i;
	bool match = false;

	for (i = 0; i < num_chan; i++) {
		if (chan_list[i] ==
		   util_scan_entry_channel_num(db_node->entry)) {
			match = true;
			break;
		}
	}

	if (!match)
		scm_scan_entry_put_ref(scan_db, db_node, true, true);

}

void scm_filter_valid_channel(struct wlan_objmgr_pdev *pdev,
	uint8_t *chan_list, uint32_t num_chan)
{
	int i;
	struct wlan_objmgr_psoc *psoc;
	struct scan_dbs *scan_db;
	struct scan_cache_node *cur_node;
	struct scan_cache_node *next_node = NULL;

	scm_info("num_chan = %d", num_chan);

	if (!pdev) {
		scm_err("pdev is NULL");
		return;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		scm_err("psoc is NULL");
		return;
	}

	scan_db = wlan_pdev_get_scan_db(psoc, pdev);
	if (!scan_db) {
		scm_err("scan_db is NULL");
		return;
	}

	for (i = 0 ; i < SCAN_HASH_SIZE; i++) {
		cur_node = scm_get_next_node(scan_db,
			   &scan_db->scan_hash_tbl[i], NULL);
		while (cur_node) {
			scm_filter_channels(scan_db,
				cur_node, chan_list, num_chan);
			next_node = scm_get_next_node(scan_db,
				&scan_db->scan_hash_tbl[i], cur_node);
			cur_node = next_node;
		}
	}
}

QDF_STATUS scm_scan_register_bcn_cb(struct wlan_objmgr_psoc *psoc,
	update_beacon_cb cb, enum scan_cb_type type)
{
	struct wlan_scan_obj *scan_obj;

	scan_obj = wlan_psoc_get_scan_obj(psoc);
	if (!scan_obj) {
		scm_err("scan obj is NULL");
		return QDF_STATUS_E_INVAL;
	}
	switch (type) {
	case SCAN_CB_TYPE_INFORM_BCN:
		scan_obj->cb.inform_beacon = cb;
		break;
	case SCAN_CB_TYPE_UPDATE_BCN:
		scan_obj->cb.update_beacon = cb;
		break;
	default:
		scm_err("invalid cb type %d", type);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scm_db_init(struct wlan_objmgr_psoc *psoc)
{
	int i, j;
	struct scan_dbs *scan_db;

	if (!psoc) {
		scm_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	/* Initialize the scan database per pdev */
	for (i = 0; i < WLAN_UMAC_MAX_PDEVS; i++) {
		scan_db = wlan_pdevid_get_scan_db(psoc, i);
		if (!scan_db) {
			scm_err("scan_db is NULL %d", i);
			continue;
		}
		scan_db->num_entries = 0;
		qdf_spinlock_create(&scan_db->scan_db_lock);
		for (j = 0; j < SCAN_HASH_SIZE; j++)
			qdf_list_create(&scan_db->scan_hash_tbl[j],
				MAX_SCAN_CACHE_SIZE);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scm_db_deinit(struct wlan_objmgr_psoc *psoc)
{
	int i, j;
	struct scan_dbs *scan_db;

	if (!psoc) {
		scm_err("scan obj is NULL");
		return QDF_STATUS_E_INVAL;
	}

	/* Initialize the scan database per pdev */
	for (i = 0; i < WLAN_UMAC_MAX_PDEVS; i++) {
		scan_db = wlan_pdevid_get_scan_db(psoc, i);
		if (!scan_db) {
			scm_err("scan_db is NULL %d", i);
			continue;
		}

		scm_flush_scan_entries(psoc, scan_db, NULL);
		for (j = 0; j < SCAN_HASH_SIZE; j++)
			qdf_list_destroy(&scan_db->scan_hash_tbl[j]);
		qdf_spinlock_destroy(&scan_db->scan_db_lock);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scm_update_scan_mlme_info(struct wlan_objmgr_pdev *pdev,
	struct scan_cache_entry *entry)
{
	uint8_t hash_idx;
	struct scan_dbs *scan_db;
	struct scan_cache_node *cur_node;
	struct scan_cache_node *next_node = NULL;
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		scm_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}
	scan_db = wlan_pdev_get_scan_db(psoc, pdev);
	if (!scan_db) {
		scm_err("scan_db is NULL");
		return QDF_STATUS_E_INVAL;
	}

	hash_idx = SCAN_GET_HASH(entry->bssid.bytes);

	cur_node = scm_get_next_node(scan_db,
			&scan_db->scan_hash_tbl[hash_idx], NULL);

	while (cur_node) {
		if (util_is_scan_entry_match(entry,
					cur_node->entry)) {
			/* Acquire db lock to prevent simultaneous update */
			qdf_spin_lock_bh(&scan_db->scan_db_lock);
			scm_update_mlme_info(entry, cur_node->entry);
			qdf_spin_unlock_bh(&scan_db->scan_db_lock);
			scm_scan_entry_put_ref(scan_db,
					cur_node, true, false);
			return QDF_STATUS_SUCCESS;
		}
		next_node = scm_get_next_node(scan_db,
				&scan_db->scan_hash_tbl[hash_idx], cur_node);
		cur_node = next_node;
	}

	return QDF_STATUS_E_INVAL;
}
