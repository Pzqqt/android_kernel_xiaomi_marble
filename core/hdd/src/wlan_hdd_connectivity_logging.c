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
