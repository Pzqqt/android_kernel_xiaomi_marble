/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 * Copyright (c) 2022-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * DOC: wlan_cm_roam_logging.c
 *
 * Implementation for the connectivity and roam logging api.
 */
#include "wlan_connectivity_logging.h"
#include "wlan_cm_api.h"
#include "wlan_mlme_main.h"

static struct wlan_connectivity_log_buf_data global_cl;

static void
wlan_connectivity_logging_register_callbacks(
				struct wlan_cl_osif_cbks *osif_cbks,
				void *osif_cb_context)
{
	global_cl.osif_cbks.wlan_connectivity_log_send_to_usr =
			osif_cbks->wlan_connectivity_log_send_to_usr;
	global_cl.osif_cb_context = osif_cb_context;
}

void wlan_connectivity_logging_start(struct wlan_objmgr_psoc *psoc,
				     struct wlan_cl_osif_cbks *osif_cbks,
				     void *osif_cb_context)
{
	global_cl.head = qdf_mem_valloc(sizeof(*global_cl.head) *
					WLAN_MAX_LOG_RECORDS);
	if (!global_cl.head) {
		QDF_BUG(0);
		return;
	}

	global_cl.psoc = psoc;
	global_cl.write_idx = 0;
	global_cl.read_idx = 0;

	qdf_atomic_init(&global_cl.dropped_msgs);
	qdf_spinlock_create(&global_cl.write_ptr_lock);

	global_cl.read_ptr = global_cl.head;
	global_cl.write_ptr = global_cl.head;
	global_cl.max_records = WLAN_MAX_LOG_RECORDS;

	wlan_connectivity_logging_register_callbacks(osif_cbks,
						     osif_cb_context);
	qdf_atomic_set(&global_cl.is_active, 1);
}

void wlan_connectivity_logging_stop(void)
{
	if (!qdf_atomic_read(&global_cl.is_active))
		return;

	qdf_spin_lock_bh(&global_cl.write_ptr_lock);

	global_cl.psoc = NULL;
	global_cl.osif_cb_context = NULL;
	global_cl.osif_cbks.wlan_connectivity_log_send_to_usr = NULL;

	qdf_atomic_set(&global_cl.is_active, 0);
	global_cl.read_ptr = NULL;
	global_cl.write_ptr = NULL;
	global_cl.read_idx = 0;
	global_cl.write_idx = 0;

	qdf_mem_vfree(global_cl.head);
	global_cl.head = NULL;
	qdf_spin_unlock_bh(&global_cl.write_ptr_lock);
	qdf_spinlock_destroy(&global_cl.write_ptr_lock);
}

#if defined(WLAN_FEATURE_ROAM_OFFLOAD) && \
	defined(WLAN_FEATURE_CONNECTIVITY_LOGGING)
QDF_STATUS wlan_print_cached_sae_auth_logs(struct qdf_mac_addr *bssid,
					   uint8_t vdev_id)
{
	uint8_t i, j;
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(global_cl.psoc, vdev_id,
						    WLAN_MLME_OBJMGR_ID);
	if (!vdev) {
		logging_err_rl("Invalid vdev:%d", vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		logging_err_rl("vdev legacy private object is NULL");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);
		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * Get the index of matching bssid and queue all the records for
	 * that bssid
	 */
	for (i = 0; i < MAX_ROAM_CANDIDATE_AP; i++) {
		if (!mlme_priv->auth_log[i][0].ktime_us)
			continue;

		if (qdf_is_macaddr_equal(bssid,
					 &mlme_priv->auth_log[i][0].bssid))
			break;
	}

	/*
	 * No matching bssid found in cached log records.
	 * So return from here.
	 */
	if (i >= MAX_ROAM_CANDIDATE_AP) {
		logging_debug("No cached SAE auth logs");
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);
		return QDF_STATUS_E_FAILURE;
	}

	for (j = 0; j < WLAN_ROAM_MAX_CACHED_AUTH_FRAMES; j++) {
		if (!mlme_priv->auth_log[i][j].ktime_us)
			continue;

		wlan_connectivity_log_enqueue(&mlme_priv->auth_log[i][j]);
		qdf_mem_zero(&mlme_priv->auth_log[i][j],
			     sizeof(mlme_priv->auth_log[i][j]));
	}

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);

	return QDF_STATUS_SUCCESS;
}

bool wlan_is_log_record_present_for_bssid(struct qdf_mac_addr *bssid,
					  uint8_t vdev_id)
{
	struct wlan_log_record *record;
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;
	int i;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(global_cl.psoc, vdev_id,
						    WLAN_MLME_OBJMGR_ID);
	if (!vdev) {
		logging_err_rl("Invalid vdev:%d", vdev_id);
		return false;
	}

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);
		logging_err_rl("vdev legacy private object is NULL");
		return false;
	}

	for (i = 0; i < MAX_ROAM_CANDIDATE_AP; i++) {
		record = &mlme_priv->auth_log[i][0];
		if (!record->ktime_us)
			continue;

		if (qdf_is_macaddr_equal(bssid, &record->bssid)) {
			wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);
			return true;
		}
	}
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);

	return false;
}

/**
 * wlan_add_sae_log_record_to_available_slot() - Add a new log record into the
 * cache for the queue.
 * @mlme_priv: Mlme private object
 * @rec: Log record pointer
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_add_sae_log_record_to_available_slot(struct mlme_legacy_priv *mlme_priv,
					  struct wlan_log_record *rec)
{
	uint8_t i, j;
	bool is_entry_exist =
		wlan_is_log_record_present_for_bssid(&rec->bssid, rec->vdev_id);

	for (i = 0; i < MAX_ROAM_CANDIDATE_AP; i++) {
		if (is_entry_exist && mlme_priv->auth_log[i][0].ktime_us &&
		    qdf_is_macaddr_equal(&rec->bssid,
					 &mlme_priv->auth_log[i][0].bssid)) {
			/*
			 * Frames for given bssid already exists
			 * store the new frame in corresponding array
			 * in empty slot
			 */
			for (j = 0; j < WLAN_ROAM_MAX_CACHED_AUTH_FRAMES; j++) {
				if (mlme_priv->auth_log[i][j].ktime_us)
					continue;

				mlme_priv->auth_log[i][j] = *rec;
				break;
			}

		} else if (!is_entry_exist &&
			   !mlme_priv->auth_log[i][0].ktime_us) {
			/*
			 * For given record, there is no existing bssid
			 * so add the entry at first available slot
			 */
			mlme_priv->auth_log[i][0] = *rec;
			break;
		}
	}

	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
wlan_add_sae_log_record_to_available_slot(struct mlme_legacy_priv *mlme_priv,
					  struct wlan_log_record *rec)
{
	return QDF_STATUS_SUCCESS;
}
#endif

static QDF_STATUS
wlan_add_sae_auth_log_record(struct wlan_objmgr_vdev *vdev,
			     struct wlan_log_record *rec)
{
	struct mlme_legacy_priv *mlme_priv;

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		logging_err_rl("vdev legacy private object is NULL");
		return QDF_STATUS_E_INVAL;
	}

	return wlan_add_sae_log_record_to_available_slot(mlme_priv, rec);
}

void wlan_clear_sae_auth_logs_cache(uint8_t vdev_id)
{
	struct wlan_objmgr_vdev *vdev;
	struct mlme_legacy_priv *mlme_priv;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(global_cl.psoc, vdev_id,
						    WLAN_MLME_OBJMGR_ID);
	if (!vdev) {
		logging_err_rl("Invalid vdev:%d", vdev_id);
		return;
	}

	mlme_priv = wlan_vdev_mlme_get_ext_hdl(vdev);
	if (!mlme_priv) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);
		logging_err_rl("vdev legacy private object is NULL");
		return;
	}

	qdf_mem_zero(mlme_priv->auth_log, sizeof(mlme_priv->auth_log));
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);
}

static void
wlan_cache_connectivity_log(struct wlan_objmgr_psoc *psoc, uint8_t vdev_id,
			    struct wlan_log_record *rec)
{
	struct wlan_objmgr_vdev *vdev;

	if (!psoc) {
		logging_err_rl("PSOC is NULL");
		return;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						    WLAN_MLME_OBJMGR_ID);
	if (!vdev) {
		logging_err_rl("Invalid vdev:%d", vdev_id);
		return;
	}

	wlan_add_sae_auth_log_record(vdev, rec);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);
}

#define WLAN_SAE_AUTH_ALGO_NUMBER 3
void
wlan_connectivity_mgmt_event(struct wlan_frame_hdr *mac_hdr,
			     uint8_t vdev_id, uint16_t status_code,
			     enum qdf_dp_tx_rx_status tx_status,
			     int8_t peer_rssi,
			     uint8_t auth_algo, uint8_t auth_type,
			     uint8_t auth_seq, enum wlan_main_tag tag)
{
	struct wlan_log_record *new_rec;
	struct wlan_objmgr_vdev *vdev;
	bool is_initial_connection = false;
	bool is_auth_frame_caching_required = false;
	enum QDF_OPMODE opmode;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(global_cl.psoc, vdev_id,
						    WLAN_MLME_OBJMGR_ID);
	if (!vdev) {
		logging_debug("Unable to find vdev:%d", vdev_id);
		return;
	}

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	if (opmode != QDF_STA_MODE && opmode != QDF_P2P_CLIENT_MODE) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);
		return;
	}

	is_initial_connection = wlan_cm_is_vdev_connecting(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);

	new_rec = qdf_mem_malloc(sizeof(*new_rec));
	if (!new_rec)
		return;

	new_rec->timestamp_us = qdf_get_time_of_the_day_us();
	new_rec->ktime_us = qdf_ktime_to_us(qdf_ktime_get());
	new_rec->vdev_id = vdev_id;
	new_rec->log_subtype = tag;
	qdf_copy_macaddr(&new_rec->bssid,
			 (struct qdf_mac_addr *)&mac_hdr->i_addr3[0]);

	new_rec->pkt_info.tx_status = tx_status;
	new_rec->pkt_info.rssi = peer_rssi;
	new_rec->pkt_info.seq_num =
		(le16toh(*(uint16_t *)mac_hdr->i_seq) >> WLAN_SEQ_SEQ_SHIFT);
	new_rec->pkt_info.frame_status_code = status_code;
	new_rec->pkt_info.auth_algo = auth_algo;
	new_rec->pkt_info.auth_type = auth_type;
	new_rec->pkt_info.auth_seq_num = auth_seq;
	new_rec->pkt_info.is_retry_frame =
		(mac_hdr->i_fc[1] & IEEE80211_FC1_RETRY);

	if (global_cl.psoc)
		is_auth_frame_caching_required =
			wlan_psoc_nif_fw_ext2_cap_get(
				global_cl.psoc,
				WLAN_ROAM_STATS_FRAME_INFO_PER_CANDIDATE);

	if (global_cl.psoc && !is_initial_connection &&
	    (new_rec->log_subtype == WLAN_AUTH_REQ ||
	     new_rec->log_subtype == WLAN_AUTH_RESP) &&
	    auth_algo == WLAN_SAE_AUTH_ALGO_NUMBER &&
	    is_auth_frame_caching_required) {
		wlan_cache_connectivity_log(global_cl.psoc, vdev_id, new_rec);
	} else {
		wlan_connectivity_log_enqueue(new_rec);
	}

	qdf_mem_free(new_rec);
}

static bool wlan_logging_is_queue_empty(void)
{
	if (!qdf_atomic_read(&global_cl.is_active))
		return true;

	qdf_spin_lock_bh(&global_cl.write_ptr_lock);

	if (!global_cl.write_ptr) {
		qdf_spin_unlock_bh(&global_cl.write_ptr_lock);
		return true;
	}

	if (global_cl.read_ptr == global_cl.write_ptr &&
	    !global_cl.write_ptr->is_record_filled) {
		qdf_spin_unlock_bh(&global_cl.write_ptr_lock);
		return true;
	}

	qdf_spin_unlock_bh(&global_cl.write_ptr_lock);

	return false;
}

QDF_STATUS
wlan_connectivity_log_enqueue(struct wlan_log_record *new_record)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_log_record *write_block;
	enum QDF_OPMODE opmode;

	if (!new_record) {
		logging_debug("NULL entry");
		return QDF_STATUS_E_FAILURE;
	}

	if (new_record->log_subtype >= WLAN_TAG_MAX) {
		logging_debug("Enqueue failed subtype:%d",
			      new_record->log_subtype);
		return QDF_STATUS_E_FAILURE;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(global_cl.psoc,
						    new_record->vdev_id,
						    WLAN_MLME_OBJMGR_ID);
	if (!vdev) {
		logging_debug("invalid vdev:%d", new_record->vdev_id);
		return QDF_STATUS_E_FAILURE;
	}

	opmode = wlan_vdev_mlme_get_opmode(vdev);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_OBJMGR_ID);

	if (opmode != QDF_STA_MODE)
		return QDF_STATUS_E_INVAL;

	/*
	 * This API writes to the logging buffer if the buffer is not full.
	 * 1. Acquire the write spinlock.
	 * 2. Copy the record to the write block.
	 * 3. Update the write pointer
	 * 4. Release the spinlock
	 */
	qdf_spin_lock_bh(&global_cl.write_ptr_lock);

	write_block = global_cl.write_ptr;
	/* If the buffer is full, increment the dropped msgs counter and
	 * return
	 */
	if (global_cl.read_ptr == global_cl.write_ptr &&
	    write_block->is_record_filled) {
		qdf_spin_unlock_bh(&global_cl.write_ptr_lock);
		qdf_atomic_inc(&global_cl.dropped_msgs);
		logging_debug("vdev:%d dropping msg sub-type:%d total dropped:%d",
			      new_record->vdev_id, new_record->log_subtype,
			      qdf_atomic_read(&global_cl.dropped_msgs));
		wlan_logging_set_connectivity_log();

		return QDF_STATUS_E_NOMEM;
	}

	*write_block = *new_record;
	write_block->is_record_filled = true;

	global_cl.write_idx++;
	global_cl.write_idx %= global_cl.max_records;

	global_cl.write_ptr =
		&global_cl.head[global_cl.write_idx];

	qdf_spin_unlock_bh(&global_cl.write_ptr_lock);

	wlan_logging_set_connectivity_log();

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_connectivity_log_dequeue(void)
{
	struct wlan_log_record *data;
	struct wlan_cl_osif_cbks *osif_cbk;
	void *osif_cb_context;
	uint8_t idx = 0;
	uint64_t current_timestamp, time_delta;

	if (wlan_logging_is_queue_empty())
		return QDF_STATUS_SUCCESS;

	data = qdf_mem_malloc(MAX_RECORD_IN_SINGLE_EVT * sizeof(*data));
	if (!data)
		return QDF_STATUS_E_NOMEM;

	while (global_cl.read_ptr->is_record_filled) {
		current_timestamp = qdf_get_time_of_the_day_ms();
		time_delta = current_timestamp -
				global_cl.first_record_timestamp_in_last_sec;
		/*
		 * Don't send logs if the time difference between the first
		 * packet queued and current timestamp is less than 1 second and
		 * the sent messages count is 20.
		 * Else if the current record to be dequeued is 1 sec apart from
		 * the previous first packet timestamp, then reset the
		 * sent messages counter and first packet timestamp.
		 */
		if (time_delta < 1000 &&
		    global_cl.sent_msgs_count >= WLAN_RECORDS_PER_SEC) {
			break;
		} else if (time_delta > 1000) {
			global_cl.sent_msgs_count = 0;
			global_cl.first_record_timestamp_in_last_sec =
							current_timestamp;
		}

		global_cl.sent_msgs_count =
				qdf_do_div_rem(global_cl.sent_msgs_count,
					       WLAN_RECORDS_PER_SEC);

		data[idx] = *global_cl.read_ptr;

		/*
		 * Reset the read block after copy. This will set the
		 * is_record_filled to false.
		 */
		qdf_mem_zero(global_cl.read_ptr, sizeof(*global_cl.read_ptr));

		global_cl.read_idx++;
		global_cl.read_idx %= global_cl.max_records;

		global_cl.read_ptr =
			&global_cl.head[global_cl.read_idx];

		global_cl.sent_msgs_count++;
		idx++;

		if (idx >= MAX_RECORD_IN_SINGLE_EVT) {
			wlan_logging_set_connectivity_log();
			break;
		}
	}

	osif_cbk = &global_cl.osif_cbks;
	osif_cb_context = global_cl.osif_cb_context;
	if (osif_cbk->wlan_connectivity_log_send_to_usr)
		osif_cbk->wlan_connectivity_log_send_to_usr(data,
							   osif_cb_context,
							   idx);

	qdf_mem_free(data);

	return QDF_STATUS_SUCCESS;
}

void wlan_connectivity_logging_init(void)
{
	qdf_atomic_set(&global_cl.is_active, 0);
}
