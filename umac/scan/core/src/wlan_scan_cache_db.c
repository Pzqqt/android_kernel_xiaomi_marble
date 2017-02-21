/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 */
#include <qdf_status.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_scan_public_structs.h>
#include <wlan_scan_utils_api.h>
#include "wlan_scan_main.h"
#include "wlan_scan_cache_db_i.h"

QDF_STATUS scm_handle_bcn_probe(struct scheduler_msg *msg)
{
	struct scan_bcn_probe_event *bcn;
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev = NULL;
	struct scan_cache_entry *scan_entry;
	struct wlan_scan_obj *scan_obj;
	struct scan_dbs *scan_db;
	QDF_STATUS status;

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
	scan_db = wlan_pdev_get_scan_db(psoc, pdev);
	if (!scan_db) {
		scm_err("scan_db is NULL");
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

	scan_entry =
		 util_scan_unpack_beacon_frame(qdf_nbuf_data(bcn->buf),
			qdf_nbuf_len(bcn->buf), bcn->frm_type,
			bcn->rx_data);
	if (!scan_entry) {
		scm_err("failed to unpack frame");
		status = QDF_STATUS_E_INVAL;
		goto free_nbuf;
	}

	scm_info("Received %s from BSSID: %pM tsf_delta = %u Seq Num: %x ssid:%.*s",
		(bcn->frm_type == MGMT_SUBTYPE_PROBE_RESP) ?
		"Probe Rsp" : "Beacon", scan_entry->bssid.bytes,
		scan_entry->tsf_delta, scan_entry->seq_num,
		scan_entry->ssid.length, scan_entry->ssid.ssid);

	if (scan_obj->cb.update_beacon)
		scan_obj->cb.update_beacon(pdev, scan_entry);

	if (scan_obj->cb.inform_beacon)
		scan_obj->cb.inform_beacon(pdev, scan_entry);

free_nbuf:
	if (pdev)
		wlan_objmgr_pdev_release_ref(pdev, WLAN_SCAN_ID);
	if (bcn->rx_data)
		qdf_mem_free(bcn->rx_data);
	if (bcn->buf)
		qdf_nbuf_free(bcn->buf);
	qdf_mem_free(bcn);

	return status;
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

		for (j = 0; j < SCAN_HASH_SIZE; j++)
			qdf_list_destroy(&scan_db->scan_hash_tbl[j]);
		qdf_spinlock_destroy(&scan_db->scan_db_lock);
	}

	return QDF_STATUS_SUCCESS;
}
