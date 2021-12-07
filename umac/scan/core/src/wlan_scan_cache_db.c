/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
 *   is initialized and incremented. Also the cookie will be set to valid value.
 * - The ref count incremented during adding new node should be decremented only
 *   by a delete operation on the node. But there can be multiple concurrent
 *   delete operations on a node from different threads which may lead to ref
 *   count being decremented multiple time and freeing the node even if node
 *   is in use. So to maintain atomicity between multiple delete operations
 *   on a same node from different threads, a cookie is used to check if node is
 *   logically deleted or not. A delete operation will set the cookie to 0
 *   making it invalid. So if the 2nd thread find the cookie as invalid it will
 *   not try to delete and decrement the ref count of the node again.
 * - This Cookie is also used to check if node is valid while iterating through
 *   the scan cache to avoid duplicate entries.
 * - Once ref_cnt become 0, i.e. it is logically deleted and no thread is using
 *   it the node is physically deleted from the scan cache.
 * - While reading the node the ref_cnt should be incremented. Once reading
 *   operation is done ref_cnt is decremented.
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
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_dfs_utils_api.h>
#include "wlan_crypto_global_def.h"
#include "wlan_crypto_global_api.h"
#include "wlan_cm_bss_score_param.h"

#ifdef FEATURE_6G_SCAN_CHAN_SORT_ALGO

struct channel_list_db *scm_get_rnr_channel_db(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_scan_obj *scan_obj = NULL;

	scan_obj = wlan_psoc_get_scan_obj(psoc);

	if (!scan_obj)
		return NULL;

	return &scan_obj->rnr_channel_db;
}

struct meta_rnr_channel *scm_get_chan_meta(struct wlan_objmgr_psoc *psoc,
					   uint32_t chan_freq)
{
	int i;
	struct channel_list_db *rnr_channel_db;

	if (!psoc || !chan_freq || !wlan_reg_is_6ghz_chan_freq(chan_freq))
		return NULL;

	rnr_channel_db = scm_get_rnr_channel_db(psoc);
	if (!rnr_channel_db)
		return NULL;

	for (i = 0; i < QDF_ARRAY_SIZE(rnr_channel_db->channel); i++)
		if (rnr_channel_db->channel[i].chan_freq == chan_freq)
			return &rnr_channel_db->channel[i];

	return NULL;
}

static void scm_add_rnr_channel_db(struct wlan_objmgr_psoc *psoc,
				   struct scan_cache_entry *entry)
{
	uint32_t chan_freq;
	uint8_t is_6g_bss, i;
	struct meta_rnr_channel *channel;
	struct rnr_bss_info *rnr_bss;
	struct scan_rnr_node *rnr_node;

	chan_freq = entry->channel.chan_freq;
	is_6g_bss = wlan_reg_is_6ghz_chan_freq(chan_freq);

	/* Return if the BSS is not 6G and RNR IE is not present */
	if (!(is_6g_bss || entry->ie_list.rnrie))
		return;

	scm_debug("BSS freq %d BSSID: "QDF_MAC_ADDR_FMT, chan_freq,
		  QDF_MAC_ADDR_REF(entry->bssid.bytes));
	if (is_6g_bss) {
		channel = scm_get_chan_meta(psoc, chan_freq);
		if (!channel) {
			scm_debug("Failed to get chan Meta freq %d", chan_freq);
			return;
		}
		channel->bss_beacon_probe_count++;
		channel->beacon_probe_last_time_found = entry->scan_entry_time;
	}

	/*
	 * If scan entry got RNR IE then loop through all
	 * entries and increase the BSS count in respective channels
	 */
	if (!entry->ie_list.rnrie)
		return;

	for (i = 0; i < MAX_RNR_BSS; i++) {
		rnr_bss = &entry->rnr.bss_info[i];
		/* Skip if entry is not valid */
		if (!rnr_bss->channel_number)
			continue;
		chan_freq = wlan_reg_chan_opclass_to_freq(rnr_bss->channel_number,
							  rnr_bss->operating_class,
							  false);
		channel = scm_get_chan_meta(psoc, chan_freq);
		if (!channel) {
			scm_debug("Failed to get chan Meta freq %d", chan_freq);
			continue;
		}
		channel->bss_beacon_probe_count++;
		/* Don't add RNR entry if list is full */
		if (qdf_list_size(&channel->rnr_list) >= WLAN_MAX_RNR_COUNT) {
			scm_debug("List is full");
			return;
		}

		rnr_node = qdf_mem_malloc(sizeof(struct scan_rnr_node));
		if (!rnr_node)
			return;
		rnr_node->entry.timestamp = entry->scan_entry_time;
		if (!qdf_is_macaddr_zero(&rnr_bss->bssid))
			qdf_mem_copy(&rnr_node->entry.bssid,
				     &rnr_bss->bssid,
				     QDF_MAC_ADDR_SIZE);
		if (rnr_bss->short_ssid)
			rnr_node->entry.short_ssid = rnr_bss->short_ssid;
		scm_debug("Add freq %d: "QDF_MAC_ADDR_FMT" short ssid %x", chan_freq,
			  QDF_MAC_ADDR_REF(rnr_bss->bssid.bytes),
			  rnr_bss->short_ssid);
		qdf_list_insert_back(&channel->rnr_list,
				     &rnr_node->node);
	}
}
#endif

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
	if (!scan_node) {
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
 *
 * Return: void
 */
static void scm_scan_entry_put_ref(struct scan_dbs *scan_db,
	struct scan_cache_node *scan_node, bool lock_needed)
{

	if (!scan_node) {
		scm_err("scan_node is NULL");
		QDF_ASSERT(0);
		return;
	}

	if (lock_needed)
		qdf_spin_lock_bh(&scan_db->scan_db_lock);

	if (!qdf_atomic_read(&scan_node->ref_cnt)) {
		if (lock_needed)
			qdf_spin_unlock_bh(&scan_db->scan_db_lock);
		scm_err("scan_node ref cnt is 0");
		QDF_ASSERT(0);
		return;
	}

	/* Decrement ref count, free scan_node, if ref count == 0 */
	if (qdf_atomic_dec_and_test(&scan_node->ref_cnt))
		scm_del_scan_node_from_db(scan_db, scan_node);

	if (lock_needed)
		qdf_spin_unlock_bh(&scan_db->scan_db_lock);
}

/**
 * scm_scan_entry_del() - API to delete scan node
 * @scan_db: data base
 * @scan_node: node to be deleted
 *
 * Call must be protected by scan_db->scan_db_lock
 *
 * Return: void
 */

static void scm_scan_entry_del(struct scan_dbs *scan_db,
			       struct scan_cache_node *scan_node)
{
	if (!scan_node) {
		scm_err("scan node is NULL");
		QDF_ASSERT(0);
		return;
	}

	if (scan_node->cookie != SCAN_NODE_ACTIVE_COOKIE) {
		scm_debug("node is already deleted");
		return;
	}
	/* Seems node is already deleted */
	if (!qdf_atomic_read(&scan_node->ref_cnt)) {
		scm_debug("node is already deleted ref 0");
		return;
	}
	scan_node->cookie = 0;
	scm_scan_entry_put_ref(scan_db, scan_node, false);
}

/**
 * scm_add_scan_node() - API to add scan node
 * @scan_db: data base
 * @scan_node: node to be added
 * @dup_node: node before which new node to be added
 * if it's not NULL, otherwise add node to tail
 *
 * Call must be protected by scan_db->scan_db_lock
 *
 * Return: void
 */
static void scm_add_scan_node(struct scan_dbs *scan_db,
	struct scan_cache_node *scan_node,
	struct scan_cache_node *dup_node)
{
	uint8_t hash_idx;

	hash_idx =
		SCAN_GET_HASH(scan_node->entry->bssid.bytes);

	qdf_atomic_init(&scan_node->ref_cnt);
	scan_node->cookie = SCAN_NODE_ACTIVE_COOKIE;
	scm_scan_entry_get_ref(scan_node);
	if (!dup_node)
		qdf_list_insert_back(&scan_db->scan_hash_tbl[hash_idx],
				     &scan_node->node);
	else
		qdf_list_insert_before(&scan_db->scan_hash_tbl[hash_idx],
				       &scan_node->node, &dup_node->node);

	scan_db->num_entries++;
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
		if (scan_node->cookie == SCAN_NODE_ACTIVE_COOKIE)
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
			cur_node, false);
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
	qdf_time_t scan_aging_time)
{
	if (util_scan_entry_age(node->entry) >=
	   scan_aging_time) {
		scm_debug("Aging out BSSID: "QDF_MAC_ADDR_FMT" with age %lu ms",
			  QDF_MAC_ADDR_REF(node->entry->bssid.bytes),
			  util_scan_entry_age(node->entry));
		qdf_spin_lock_bh(&scan_db->scan_db_lock);
		scm_scan_entry_del(scan_db, node);
		qdf_spin_unlock_bh(&scan_db->scan_db_lock);
	}
}

static bool scm_bss_is_connected(struct scan_cache_entry *entry)
{
	if (entry->mlme_info.assoc_state == SCAN_ENTRY_CON_STATE_ASSOC)
		return true;
	return false;
}

/**
 * scm_get_conn_node() - Get the scan cache entry node of the connected BSS
 * @scan_db: scan DB pointer
 *
 * Return: scan cache entry node of connected BSS if exists, NULL otherwise
 */
static
struct scan_cache_node *scm_get_conn_node(struct scan_dbs *scan_db)
{
	int i;
	struct scan_cache_node *cur_node = NULL;
	struct scan_cache_node *next_node = NULL;

	for (i = 0 ; i < SCAN_HASH_SIZE; i++) {
		cur_node = scm_get_next_node(scan_db,
			&scan_db->scan_hash_tbl[i], NULL);
		while (cur_node) {
			if (scm_bss_is_connected(cur_node->entry))
				return cur_node;
			next_node = scm_get_next_node(scan_db,
				&scan_db->scan_hash_tbl[i], cur_node);
			cur_node = next_node;
			next_node = NULL;
		}
	}

	return NULL;
}

void scm_age_out_entries(struct wlan_objmgr_psoc *psoc,
	struct scan_dbs *scan_db)
{
	int i;
	struct scan_cache_node *cur_node = NULL;
	struct scan_cache_node *next_node = NULL;
	struct scan_cache_node *conn_node = NULL;
	struct scan_default_params *def_param;

	def_param = wlan_scan_psoc_get_def_params(psoc);
	if (!def_param) {
		scm_err("wlan_scan_psoc_get_def_params failed");
		return;
	}

	conn_node = scm_get_conn_node(scan_db);
	for (i = 0 ; i < SCAN_HASH_SIZE; i++) {
		cur_node = scm_get_next_node(scan_db,
			&scan_db->scan_hash_tbl[i], NULL);
		while (cur_node) {
			if (!conn_node /* if there is no connected node */ ||
			    /* OR cur_node is not part of the MBSSID of the
			     * connected node
			     */
			    memcmp(conn_node->entry->mbssid_info.trans_bssid,
				   cur_node->entry->mbssid_info.trans_bssid,
				   QDF_MAC_ADDR_SIZE)
			   ) {
				scm_check_and_age_out(scan_db, cur_node,
					def_param->scan_cache_aging_time);
			}
			next_node = scm_get_next_node(scan_db,
				&scan_db->scan_hash_tbl[i], cur_node);
			cur_node = next_node;
			next_node = NULL;
		}
	}

	if (conn_node)
		scm_scan_entry_put_ref(scan_db, conn_node, true);
}

/**
 * scm_flush_oldest_entry() - Iterate over scan db and flust out the
 *  oldest entry
 * @scan_db: scan db from which oldest entry needs to be flushed
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS scm_flush_oldest_entry(struct scan_dbs *scan_db)
{
	int i;
	struct scan_cache_node *oldest_node = NULL;
	struct scan_cache_node *cur_node;

	for (i = 0 ; i < SCAN_HASH_SIZE; i++) {
		/* Get the first valid node for the hash */
		cur_node = scm_get_next_node(scan_db,
					     &scan_db->scan_hash_tbl[i],
					     NULL);
		 /* Iterate scan db and flush out oldest node
		  * take ref_cnt for oldest_node
		  */

		while (cur_node) {
			if (!oldest_node ||
			   (util_scan_entry_age(oldest_node->entry) <
			    util_scan_entry_age(cur_node->entry))) {
				if (oldest_node)
					scm_scan_entry_put_ref(scan_db,
							       oldest_node,
							       true);
				qdf_spin_lock_bh(&scan_db->scan_db_lock);
				oldest_node = cur_node;
				scm_scan_entry_get_ref(oldest_node);
				qdf_spin_unlock_bh(&scan_db->scan_db_lock);
			}

			cur_node = scm_get_next_node(scan_db,
					&scan_db->scan_hash_tbl[i],
					cur_node);
		};
	}

	if (oldest_node) {
		scm_debug("Flush oldest BSSID: "QDF_MAC_ADDR_FMT" with age %lu ms",
			  QDF_MAC_ADDR_REF(oldest_node->entry->bssid.bytes),
			  util_scan_entry_age(oldest_node->entry));
		/* Release ref_cnt taken for oldest_node and delete it */
		qdf_spin_lock_bh(&scan_db->scan_db_lock);
		scm_scan_entry_del(scan_db, oldest_node);
		scm_scan_entry_put_ref(scan_db, oldest_node, false);
		qdf_spin_unlock_bh(&scan_db->scan_db_lock);
	}

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
		dst->alt_wcn_ie.ptr =
			qdf_mem_malloc_atomic(WLAN_MAX_IE_LEN + 2);
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
 * scm_copy_info_from_dup_entry() - copy duplicate node info
 * to new scan entry
 * @pdev: pdev ptr
 * @scan_obj: scan obj ptr
 * @scan_db: scan database
 * @scan_params: new entry to be added
 * @scan_node: duplicate entry
 *
 * Copy duplicate node info to new entry.
 *
 * Return: void
 */
static void
scm_copy_info_from_dup_entry(struct wlan_objmgr_pdev *pdev,
			     struct wlan_scan_obj *scan_obj,
			     struct scan_dbs *scan_db,
			     struct scan_cache_entry *scan_params,
			     struct scan_cache_node *scan_node)
{
	struct scan_cache_entry *scan_entry;
	uint64_t time_gap;

	scan_entry = scan_node->entry;

	/* Update probe resp entry as well if AP is in hidden mode */
	if (scan_params->frm_subtype == MGMT_SUBTYPE_PROBE_RESP &&
	    scan_entry->is_hidden_ssid)
		scan_params->is_hidden_ssid = true;

	/*
	 * If AP changed its beacon from not having an SSID to showing it the
	 * kernel will drop the entry asumming that something is wrong with AP.
	 * This can result in connection failure while updating the bss during
	 * connection. So flush the hidden entry from kernel before indicating
	 * the new entry.
	 */
	if (scan_entry->is_hidden_ssid &&
	    scan_params->frm_subtype == MGMT_SUBTYPE_BEACON &&
	    !util_scan_is_null_ssid(&scan_params->ssid)) {
		if (scan_obj->cb.unlink_bss) {
			scm_debug("Hidden AP "QDF_MAC_ADDR_FMT" switch to non-hidden SSID, So unlink the entry",
				  QDF_MAC_ADDR_REF(scan_entry->bssid.bytes));
			scan_obj->cb.unlink_bss(pdev, scan_entry);
		}
	}

	/* If old entry have the ssid but new entry does not */
	if (util_scan_is_null_ssid(&scan_params->ssid) &&
	    scan_entry->ssid.length) {
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
				scan_entry->ssid.length);
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
	    (scan_params->channel.chan_freq != scan_entry->channel.chan_freq) &&
	    (scan_params->rssi_raw  < ADJACENT_CHANNEL_RSSI_THRESHOLD)) {
		scan_params->channel.chan_freq = scan_entry->channel.chan_freq;
		scan_params->channel_mismatch = true;
	}

	/* Use old value for rssi if beacon was heard on adjacent channel. */
	if (scan_params->channel_mismatch) {
		scan_params->snr = scan_entry->snr;
		scan_params->avg_snr = scan_entry->avg_snr;
		scan_params->rssi_raw = scan_entry->rssi_raw;
		scan_params->avg_rssi = scan_entry->avg_rssi;
		scan_params->rssi_timestamp =
			scan_entry->rssi_timestamp;
	} else {
		/* If elapsed time since last rssi and snr update for this
		 * entry is smaller than a thresold, calculate a
		 * running average of the RSSI and SNR values.
		 * Otherwise new frames RSSI and SNR are more representive
		 * of the signal strength.
		 */
		time_gap =
			scan_params->scan_entry_time -
			scan_entry->rssi_timestamp;
		if (time_gap > WLAN_RSSI_AVERAGING_TIME) {
			scan_params->avg_rssi =
				WLAN_RSSI_IN(scan_params->rssi_raw);
			scan_params->avg_snr =
				WLAN_SNR_IN(scan_params->snr);
		}
		else {
			/* Copy previous average rssi and snr to new entry */
			scan_params->avg_snr = scan_entry->avg_snr;
			scan_params->avg_rssi = scan_entry->avg_rssi;
			/* Average with previous samples */
			WLAN_RSSI_LPF(scan_params->avg_rssi,
				      scan_params->rssi_raw);
			WLAN_SNR_LPF(scan_params->avg_snr,
				     scan_params->snr);
		}

		scan_params->rssi_timestamp = scan_params->scan_entry_time;
	}

	/* copy wsn ie from scan_entry to scan_params*/
	scm_update_alt_wcn_ie(scan_entry, scan_params);

	/* copy mlme info from scan_entry to scan_params*/
	scm_update_mlme_info(scan_entry, scan_params);
}

/**
 * scm_find_duplicate() - find duplicate entry,
 * if present, add input scan entry before it and delete
 * duplicate entry. otherwise add entry to tail
 * @pdev: pdev ptr
 * @scan_obj: scan obj ptr
 * @scan_db: scan db
 * @entry: input scan cache entry
 * @dup_node: node before which new entry to be added
 *
 * ref_cnt is taken for dup_node, caller should release ref taken
 * if returns true.
 *
 * Return: bool
 */
static bool
scm_find_duplicate(struct wlan_objmgr_pdev *pdev,
		   struct wlan_scan_obj *scan_obj,
		   struct scan_dbs *scan_db,
		   struct scan_cache_entry *entry,
		   struct scan_cache_node **dup_node)
{
	uint8_t hash_idx;
	struct scan_cache_node *cur_node;
	struct scan_cache_node *next_node = NULL;

	hash_idx = SCAN_GET_HASH(entry->bssid.bytes);

	cur_node = scm_get_next_node(scan_db,
				     &scan_db->scan_hash_tbl[hash_idx],
				     NULL);

	while (cur_node) {
		if (util_is_scan_entry_match(entry,
		   cur_node->entry)) {
			scm_copy_info_from_dup_entry(pdev, scan_obj, scan_db,
						     entry, cur_node);
			*dup_node = cur_node;
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
	struct scan_cache_node *dup_node = NULL;
	struct scan_cache_node *scan_node = NULL;
	bool is_dup_found = false;
	QDF_STATUS status;
	struct scan_dbs *scan_db;
	struct wlan_scan_obj *scan_obj;
	uint8_t security_type;

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
		scm_debug("Probe resp doesn't contain SSID");


	if (scan_params->ie_list.csa ||
	   scan_params->ie_list.xcsa ||
	   scan_params->ie_list.cswrp)
		scm_debug("CSA IE present for BSSID: "QDF_MAC_ADDR_FMT,
			  QDF_MAC_ADDR_REF(scan_params->bssid.bytes));

	is_dup_found = scm_find_duplicate(pdev, scan_obj, scan_db, scan_params,
					  &dup_node);

	security_type = scan_params->security_type;
	scm_nofl_debug("Received %s: "QDF_MAC_ADDR_FMT" \"%.*s\" freq %d rssi %d tsf_delta %u seq %d snr %d phy %d hidden %d mismatch %d %s%s%s%s pdev %d boot_time %llu ns",
		       (scan_params->frm_subtype == MGMT_SUBTYPE_PROBE_RESP) ?
		       "prb rsp" : "bcn",
		       QDF_MAC_ADDR_REF(scan_params->bssid.bytes),
		       scan_params->ssid.length, scan_params->ssid.ssid,
		       scan_params->channel.chan_freq, scan_params->rssi_raw,
		       scan_params->tsf_delta, scan_params->seq_num,
		       scan_params->snr, scan_params->phy_mode,
		       scan_params->is_hidden_ssid,
		       scan_params->channel_mismatch,
		       security_type & SCAN_SECURITY_TYPE_WPA ? "[WPA]" : "",
		       security_type & SCAN_SECURITY_TYPE_RSN ? "[RSN]" : "",
		       security_type & SCAN_SECURITY_TYPE_WAPI ? "[WAPI]" : "",
		       security_type & SCAN_SECURITY_TYPE_WEP ? "[WEP]" : "",
		       wlan_objmgr_pdev_get_pdev_id(pdev),
		       scan_params->boottime_ns);

	if (scan_obj->cb.inform_beacon)
		scan_obj->cb.inform_beacon(pdev, scan_params);

	if (scan_db->num_entries >= MAX_SCAN_CACHE_SIZE) {
		status = scm_flush_oldest_entry(scan_db);
		if (QDF_IS_STATUS_ERROR(status)) {
			/* release ref taken for dup node */
			if (is_dup_found)
				scm_scan_entry_put_ref(scan_db, dup_node, true);
			return status;
		}
	}

	scan_node = qdf_mem_malloc(sizeof(*scan_node));
	if (!scan_node) {
		/* release ref taken for dup node */
		if (is_dup_found)
			scm_scan_entry_put_ref(scan_db, dup_node, true);
		return QDF_STATUS_E_NOMEM;
	}

	scan_node->entry = scan_params;
	qdf_spin_lock_bh(&scan_db->scan_db_lock);
	scm_add_scan_node(scan_db, scan_node, dup_node);

	if (is_dup_found) {
		/* release ref taken for dup node and delete it */
		scm_scan_entry_del(scan_db, dup_node);
		scm_scan_entry_put_ref(scan_db, dup_node, false);
	}
	qdf_spin_unlock_bh(&scan_db->scan_db_lock);

	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_REG_CLIENT
/**
 * scm_is_bss_allowed_for_country() - Check if bss is allowed to start for a
 * specific country and power mode (VLP?LPI/SP) for 6GHz.
 * @psoc: psoc ptr
 * @scan_entry: ptr to scan entry
 *
 * Return: True if allowed, False if not.
 */
static bool scm_is_bss_allowed_for_country(struct wlan_objmgr_psoc *psoc,
					   struct scan_cache_entry *scan_entry)
{
	struct wlan_country_ie *cc_ie;
	uint8_t programmed_country[REG_ALPHA2_LEN + 1];

	if (wlan_reg_is_6ghz_chan_freq(scan_entry->channel.chan_freq)) {
		cc_ie = util_scan_entry_country(scan_entry);
		wlan_reg_read_current_country(psoc, programmed_country);
		if (cc_ie && qdf_mem_cmp(cc_ie->cc, programmed_country,
					 REG_ALPHA2_LEN)) {
			if (wlan_reg_is_us(programmed_country))
				return false;
		}
	}
	return true;
}
#else
static bool scm_is_bss_allowed_for_country(struct wlan_objmgr_psoc *psoc,
					   struct scan_cache_entry *scan_entry)
{
	return true;
}
#endif

QDF_STATUS __scm_handle_bcn_probe(struct scan_bcn_probe_event *bcn)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct scan_cache_entry *scan_entry;
	struct wlan_scan_obj *scan_obj;
	qdf_list_t *scan_list = NULL;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint32_t list_count, i;
	qdf_list_node_t *next_node = NULL;
	struct scan_cache_node *scan_node;
	struct wlan_frame_hdr *hdr = NULL;
	struct wlan_crypto_params sec_params;

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

	hdr = (struct wlan_frame_hdr *)qdf_nbuf_data(bcn->buf);
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
		scm_debug("invalid beacon/probe length");
		status = QDF_STATUS_E_INVAL;
		goto free_nbuf;
	}

	if (bcn->frm_type == MGMT_SUBTYPE_BEACON &&
	    wlan_reg_is_dfs_for_freq(pdev, bcn->rx_data->chan_freq)) {
		util_scan_add_hidden_ssid(pdev, bcn->buf);
	}

	scan_list =
		 util_scan_unpack_beacon_frame(pdev, qdf_nbuf_data(bcn->buf),
			qdf_nbuf_len(bcn->buf), bcn->frm_type,
			bcn->rx_data);
	if (!scan_list || qdf_list_empty(scan_list)) {
		scm_debug("failed to unpack %d frame BSSID: "QDF_MAC_ADDR_FMT,
			  bcn->frm_type, QDF_MAC_ADDR_REF(hdr->i_addr3));
		status = QDF_STATUS_E_INVAL;
		goto free_nbuf;
	}

	list_count = qdf_list_size(scan_list);
	for (i = 0; i < list_count; i++) {
		status = qdf_list_remove_front(scan_list, &next_node);
		if (QDF_IS_STATUS_ERROR(status) || !next_node) {
			scm_debug("list remove failure i:%d, lsize:%d, BSSID: "QDF_MAC_ADDR_FMT,
				  i, list_count, QDF_MAC_ADDR_REF(hdr->i_addr3));
			status = QDF_STATUS_E_INVAL;
			goto free_nbuf;
		}

		scan_node = qdf_container_of(next_node,
			struct scan_cache_node, node);

		scan_entry = scan_node->entry;

		if (scan_obj->drop_bcn_on_chan_mismatch &&
		    scan_entry->channel_mismatch) {
			scm_nofl_debug("Drop frame for chan mismatch "QDF_MAC_ADDR_FMT" Seq Num: %d freq %d RSSI %d",
				       QDF_MAC_ADDR_REF(scan_entry->bssid.bytes),
				       scan_entry->seq_num,
				       scan_entry->channel.chan_freq,
				       scan_entry->rssi_raw);
			util_scan_free_cache_entry(scan_entry);
			qdf_mem_free(scan_node);
			continue;
		}
		/* Do not add invalid channel entry as kernel will reject it */
		if (scan_obj->drop_bcn_on_invalid_freq &&
		    wlan_reg_is_disable_for_freq(pdev,
					scan_entry->channel.chan_freq)) {
			scm_nofl_debug("Drop frame for invalid freq %d: "QDF_MAC_ADDR_FMT" Seq Num: %d RSSI %d",
				       scan_entry->channel.chan_freq,
				       QDF_MAC_ADDR_REF(scan_entry->bssid.bytes),
				       scan_entry->seq_num,
				       scan_entry->rssi_raw);
			util_scan_free_cache_entry(scan_entry);
			qdf_mem_free(scan_node);
			continue;
		}
		if (wlan_cm_get_check_6ghz_security(psoc) &&
		    wlan_reg_is_6ghz_chan_freq(scan_entry->channel.chan_freq)) {
			if (!util_scan_entry_rsn(scan_entry)) {
				scm_info_rl(
					"Drop frame from "QDF_MAC_ADDR_FMT
					": No RSN IE for 6GHz AP",
					QDF_MAC_ADDR_REF(
						scan_entry->bssid.bytes));
				util_scan_free_cache_entry(scan_entry);
				qdf_mem_free(scan_node);
				continue;
			}
			status = wlan_crypto_rsnie_check(&sec_params,
					util_scan_entry_rsn(scan_entry));
			if (QDF_IS_STATUS_ERROR(status)) {
				scm_info_rl(
					"Drop frame from 6GHz AP "
					QDF_MAC_ADDR_FMT
					": RSN IE parse failed, status %d",
					QDF_MAC_ADDR_REF(
						scan_entry->bssid.bytes),
					status);
				util_scan_free_cache_entry(scan_entry);
				qdf_mem_free(scan_node);
				continue;
			}
			if ((QDF_HAS_PARAM(sec_params.ucastcipherset,
					   WLAN_CRYPTO_CIPHER_NONE)) ||
			    (QDF_HAS_PARAM(sec_params.ucastcipherset,
					   WLAN_CRYPTO_CIPHER_TKIP)) ||
			    (QDF_HAS_PARAM(sec_params.ucastcipherset,
					   WLAN_CRYPTO_CIPHER_WEP_40)) ||
			    (QDF_HAS_PARAM(sec_params.ucastcipherset,
					   WLAN_CRYPTO_CIPHER_WEP_104))) {
				scm_info_rl(
					"Drop frame from "QDF_MAC_ADDR_FMT
					": Invalid sec type %0X for 6GHz AP",
					QDF_MAC_ADDR_REF(
						scan_entry->bssid.bytes),
					sec_params.ucastcipherset);
				util_scan_free_cache_entry(scan_entry);
				qdf_mem_free(scan_node);
				continue;
			}
			if (!wlan_cm_6ghz_allowed_for_akm(psoc,
					sec_params.key_mgmt,
					sec_params.rsn_caps,
					util_scan_entry_rsnxe(scan_entry),
					0, false)) {
				scm_info_rl(
					"Drop frame from "QDF_MAC_ADDR_FMT
					": Invalid AKM suite %0X for 6GHz AP",
					QDF_MAC_ADDR_REF(
						scan_entry->bssid.bytes),
					sec_params.key_mgmt);
				util_scan_free_cache_entry(scan_entry);
				qdf_mem_free(scan_node);
				continue;
			}
		}
		if (scan_obj->cb.update_beacon)
			scan_obj->cb.update_beacon(pdev, scan_entry);

		/**
		 * Do not drop the frame if Wi-Fi safe mode or RF test mode is
		 * enabled. wlan_cm_get_check_6ghz_security API returns true if
		 * neither Safe mode nor RF test mode are enabled.
		 */
		if (!scm_is_bss_allowed_for_country(psoc, scan_entry) &&
		    wlan_cm_get_check_6ghz_security(psoc)) {
			scm_info_rl(
				"Drop frame from "QDF_MAC_ADDR_FMT
				": AP in VLP mode not supported for US",
				QDF_MAC_ADDR_REF(scan_entry->bssid.bytes));
			util_scan_free_cache_entry(scan_entry);
			qdf_mem_free(scan_node);
			continue;
		}

		status = scm_add_update_entry(psoc, pdev, scan_entry);
		if (QDF_IS_STATUS_ERROR(status)) {
			scm_debug("failed to add entry for BSSID: "QDF_MAC_ADDR_FMT" Seq Num: %d",
				  QDF_MAC_ADDR_REF(scan_entry->bssid.bytes),
				  scan_entry->seq_num);
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

QDF_STATUS scm_handle_bcn_probe(struct scheduler_msg *msg)
{
	if (!msg) {
		scm_err("msg is NULL");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return __scm_handle_bcn_probe(msg->bodyptr);
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

	scan_node = qdf_mem_malloc_atomic(sizeof(*scan_node));
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

	qdf_list_insert_front(scan_list, &scan_node->node);

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

	tmp_list = qdf_mem_malloc_atomic(sizeof(*tmp_list));
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
					cur_node, true);
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

	qdf_spin_lock_bh(&scan_db->scan_db_lock);
	scm_scan_entry_del(scan_db, db_node);
	qdf_spin_unlock_bh(&scan_db->scan_db_lock);

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
 * @chan_freq_list: valid channel frequency (in MHz) list
 * @num_chan: number of channels
 *
 * Return: QDF_STATUS
 */
static void scm_filter_channels(struct wlan_objmgr_pdev *pdev,
				struct scan_dbs *scan_db,
				struct scan_cache_node *db_node,
				uint32_t *chan_freq_list, uint32_t num_chan)
{
	int i;
	bool match = false;

	for (i = 0; i < num_chan; i++) {
		if (chan_freq_list[i] == util_scan_entry_channel_frequency(
							db_node->entry)) {
			match = true;
			break;
		}
	}

	if (!match) {
		qdf_spin_lock_bh(&scan_db->scan_db_lock);
		scm_scan_entry_del(scan_db, db_node);
		qdf_spin_unlock_bh(&scan_db->scan_db_lock);
	}
}

void scm_filter_valid_channel(struct wlan_objmgr_pdev *pdev,
	uint32_t *chan_freq_list, uint32_t num_chan)
{
	int i;
	struct wlan_objmgr_psoc *psoc;
	struct scan_dbs *scan_db;
	struct scan_cache_node *cur_node;
	struct scan_cache_node *next_node = NULL;

	scm_debug("num_chan = %d", num_chan);

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
			scm_filter_channels(pdev, scan_db,
					    cur_node, chan_freq_list, num_chan);
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
	case SCAN_CB_TYPE_UNLINK_BSS:
		scan_obj->cb.unlink_bss = cb;
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

#ifdef FEATURE_6G_SCAN_CHAN_SORT_ALGO
QDF_STATUS scm_channel_list_db_init(struct wlan_objmgr_psoc *psoc)
{
	uint32_t i, j;
	uint32_t min_freq, max_freq;
	struct channel_list_db *rnr_channel_db;

	min_freq = wlan_reg_min_6ghz_chan_freq();
	max_freq = wlan_reg_max_6ghz_chan_freq();

	scm_info("min_freq %d max_freq %d", min_freq, max_freq);
	i = min_freq;
	rnr_channel_db = scm_get_rnr_channel_db(psoc);
	if (!rnr_channel_db)
		return QDF_STATUS_E_INVAL;

	for (j = 0; j < QDF_ARRAY_SIZE(rnr_channel_db->channel); j++) {
		if (i >= min_freq && i <= max_freq)
			rnr_channel_db->channel[j].chan_freq = i;
		i += 20;
		/* init list for all to avoid uninitialized list */
		qdf_list_create(&rnr_channel_db->channel[j].rnr_list,
				WLAN_MAX_RNR_COUNT);
	}
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scm_channel_list_db_deinit(struct wlan_objmgr_psoc *psoc)
{
	int i;
	qdf_list_node_t *cur_node, *next_node;
	struct meta_rnr_channel *channel;
	struct scan_rnr_node *rnr_node;
	struct channel_list_db *rnr_channel_db;

	rnr_channel_db = scm_get_rnr_channel_db(psoc);
	if (!rnr_channel_db)
		return QDF_STATUS_E_INVAL;

	for (i = 0; i < QDF_ARRAY_SIZE(rnr_channel_db->channel); i++) {
		channel = &rnr_channel_db->channel[i];
		channel->chan_freq = 0;
		channel->beacon_probe_last_time_found = 0;
		channel->bss_beacon_probe_count = 0;
		channel->saved_profile_count = 0;
		cur_node = NULL;
		qdf_list_peek_front(&channel->rnr_list, &cur_node);
		while (cur_node) {
			next_node = NULL;
			qdf_list_peek_next(&channel->rnr_list, cur_node,
					   &next_node);
			rnr_node = qdf_container_of(cur_node,
						    struct scan_rnr_node,
						    node);
			qdf_list_remove_node(&channel->rnr_list,
					     &rnr_node->node);
			qdf_mem_free(rnr_node);
			cur_node = next_node;
			next_node = NULL;
		}
		qdf_list_destroy(&channel->rnr_list);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS scm_rnr_db_flush(struct wlan_objmgr_psoc *psoc)
{
	int i;
	qdf_list_node_t *cur_node, *next_node;
	struct meta_rnr_channel *channel;
	struct scan_rnr_node *rnr_node;
	struct channel_list_db *rnr_channel_db;

	rnr_channel_db = scm_get_rnr_channel_db(psoc);
	if (!rnr_channel_db)
		return QDF_STATUS_E_INVAL;

	for (i = 0; i < QDF_ARRAY_SIZE(rnr_channel_db->channel); i++) {
		channel = &rnr_channel_db->channel[i];
		cur_node = NULL;
		qdf_list_peek_front(&channel->rnr_list, &cur_node);
		while (cur_node) {
			next_node = NULL;
			qdf_list_peek_next(&channel->rnr_list, cur_node,
					   &next_node);
			rnr_node = qdf_container_of(cur_node,
						    struct scan_rnr_node,
						    node);
			qdf_list_remove_node(&channel->rnr_list,
					     &rnr_node->node);
			qdf_mem_free(rnr_node);
			cur_node = next_node;
			next_node = NULL;
		}
		/* Reset beacon info */
		channel->beacon_probe_last_time_found = 0;
		channel->bss_beacon_probe_count = 0;
	}

	return QDF_STATUS_SUCCESS;
}

void scm_update_rnr_from_scan_cache(struct wlan_objmgr_pdev *pdev)
{
	uint8_t i;
	struct scan_dbs *scan_db;
	struct scan_cache_node *cur_node;
	struct scan_cache_node *next_node = NULL;
	struct wlan_objmgr_psoc *psoc;
	struct scan_cache_entry *entry;

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
			entry = cur_node->entry;
			scm_add_rnr_channel_db(psoc, entry);
			next_node =
				scm_get_next_node(scan_db,
						  &scan_db->scan_hash_tbl[i],
						  cur_node);
			cur_node = next_node;
			next_node = NULL;
		}
	}
}
#endif

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
					cur_node, true);
			return QDF_STATUS_SUCCESS;
		}
		next_node = scm_get_next_node(scan_db,
				&scan_db->scan_hash_tbl[hash_idx], cur_node);
		cur_node = next_node;
	}

	return QDF_STATUS_E_INVAL;
}

QDF_STATUS scm_scan_update_mlme_by_bssinfo(struct wlan_objmgr_pdev *pdev,
		struct bss_info *bss_info, struct mlme_info *mlme)
{
	uint8_t hash_idx;
	struct scan_dbs *scan_db;
	struct scan_cache_node *cur_node;
	struct scan_cache_node *next_node = NULL;
	struct wlan_objmgr_psoc *psoc;
	struct scan_cache_entry *entry;

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

	hash_idx = SCAN_GET_HASH(bss_info->bssid.bytes);
	cur_node = scm_get_next_node(scan_db,
			&scan_db->scan_hash_tbl[hash_idx], NULL);
	while (cur_node) {
		entry = cur_node->entry;
		if (qdf_is_macaddr_equal(&bss_info->bssid, &entry->bssid) &&
			(util_is_ssid_match(&bss_info->ssid, &entry->ssid)) &&
			(bss_info->freq == entry->channel.chan_freq)) {
			/* Acquire db lock to prevent simultaneous update */
			qdf_spin_lock_bh(&scan_db->scan_db_lock);
			qdf_mem_copy(&entry->mlme_info, mlme,
					sizeof(struct mlme_info));
			scm_scan_entry_put_ref(scan_db,
					cur_node, false);
			qdf_spin_unlock_bh(&scan_db->scan_db_lock);
			return QDF_STATUS_SUCCESS;
		}
		next_node = scm_get_next_node(scan_db,
				&scan_db->scan_hash_tbl[hash_idx], cur_node);
		cur_node = next_node;
	}

	return QDF_STATUS_E_INVAL;
}
