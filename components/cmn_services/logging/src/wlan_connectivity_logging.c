/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#include "wlan_mlme_api.h"
#include "wlan_mlme_main.h"
#include "wlan_connectivity_logging.h"

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

void wlan_connectivity_logging_start(struct wlan_cl_osif_cbks *osif_cbks,
				     void *osif_cb_context)
{
	global_cl.head = qdf_mem_valloc(sizeof(*global_cl.head) *
					WLAN_MAX_LOG_RECORDS);
	if (!global_cl.head) {
		QDF_BUG(0);
		return;
	}

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

void
wlan_connectivity_mgmt_event(struct wlan_frame_hdr *mac_hdr,
			     uint8_t vdev_id, uint16_t status_code,
			     enum qdf_dp_tx_rx_status tx_status,
			     int8_t peer_rssi,
			     uint8_t auth_algo, uint8_t auth_type,
			     uint8_t auth_seq, enum wlan_main_tag tag)
{
	struct wlan_log_record *new_rec;

	new_rec = qdf_mem_malloc(sizeof(*new_rec));
	if (!new_rec)
		return;

	new_rec->timestamp_us = qdf_get_time_of_the_day_us();
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

	wlan_connectivity_log_enqueue(new_rec);

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
	struct wlan_log_record *write_block;

	if (!new_record) {
		logging_debug("NULL entry");
		return QDF_STATUS_E_FAILURE;
	}

	if (new_record->log_subtype >= WLAN_TAG_MAX) {
		logging_debug("Enqueue failed subtype:%d",
			      new_record->log_subtype);
		return QDF_STATUS_E_FAILURE;
	}

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

		global_cl.sent_msgs_count %= WLAN_RECORDS_PER_SEC;
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
