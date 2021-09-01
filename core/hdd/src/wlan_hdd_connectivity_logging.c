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
 * DOC: wlan_hdd_connectivity_logging.c
 *
 * Implementation for the Common connectivity and roam logging api.
 */

#include <wlan_hdd_connectivity_logging.h>

/**
 * wlan_hdd_send_connectivity_log_to_user  - Send the connectivity log buffer
 * to userspace
 * @rec: Pointer to the log record
 * @num_records: Number of records
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_hdd_send_connectivity_log_to_user(struct wlan_log_record *rec,
				       uint8_t num_records)
{
	struct hdd_context *hdd_ctx;

	hdd_enter();

	hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!hdd_ctx)
		return QDF_STATUS_E_FAILURE;

	hdd_exit();

	return QDF_STATUS_SUCCESS;
}

void wlan_hdd_start_connectivity_logging(void)
{
	struct wlan_cl_hdd_cbks hdd_cb;

	hdd_cb.wlan_connectivity_log_send_to_usr =
			wlan_hdd_send_connectivity_log_to_user;
	wlan_connectivity_logging_start(&hdd_cb);
}

void wlan_hdd_connectivity_event_connecting(struct hdd_context *hdd_ctx,
					    struct cfg80211_connect_params *req,
					    uint8_t vdev_id)
{
	struct wlan_log_record *record;

	record = qdf_mem_malloc(sizeof(*record));
	if (!record)
		return;

	record->timestamp_us = qdf_get_time_of_the_day_us();
	record->vdev_id = vdev_id;

	record->conn_info.ssid_len = req->ssid_len;
	if (req->ssid_len > WLAN_SSID_MAX_LEN)
		record->conn_info.ssid_len = WLAN_SSID_MAX_LEN;
	qdf_mem_copy(record->conn_info.ssid, req->ssid, req->ssid_len);

	if (req->bssid)
		qdf_mem_copy(record->bssid.bytes, req->bssid,
			     QDF_MAC_ADDR_SIZE);
	else if (req->bssid_hint)
		qdf_mem_copy(record->conn_info.bssid_hint.bytes,
			     req->bssid_hint,
			     QDF_MAC_ADDR_SIZE);

	if (req->channel)
		record->conn_info.freq = req->channel->center_freq;

	if (req->channel_hint)
		record->conn_info.freq_hint = req->channel_hint->center_freq;

	record->conn_info.pairwise = req->crypto.ciphers_pairwise[0];
	record->conn_info.group = req->crypto.cipher_group;
	record->conn_info.akm = req->crypto.akm_suites[0];
	record->conn_info.auth_type = req->auth_type;
	if (hdd_ctx->bt_a2dp_active || hdd_ctx->bt_vo_active)
		record->conn_info.is_bt_coex_active = true;

	wlan_connectivity_log_enqueue(record);

	qdf_mem_free(record);
}

void
wlan_hdd_connectivity_fail_event(struct wlan_objmgr_vdev *vdev,
				 struct wlan_cm_connect_resp *rsp)
{
	uint8_t vdev_id;
	struct wlan_log_record *log;
	enum QDF_OPMODE op_mode;

	/* Send the event only for failure reason, else return */
	if (!rsp->reason)
		return;

	vdev_id = wlan_vdev_get_id(vdev);
	op_mode = wlan_vdev_mlme_get_opmode(vdev);
	if (op_mode != QDF_STA_MODE)
		return;

	log = qdf_mem_malloc(sizeof(*log));
	if (!log)
		return;

	log->timestamp_us = qdf_get_time_of_the_day_us();
	log->vdev_id = vdev_id;
	log->bssid = rsp->bssid;
	log->conn_info.freq = rsp->freq;
	log->conn_info.conn_status = rsp->reason;

	wlan_connectivity_log_enqueue(log);
	qdf_mem_free(log);
}
