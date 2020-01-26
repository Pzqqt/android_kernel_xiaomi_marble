/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: Public API implementation of pkt_capture called by north bound HDD/OSIF.
 */

#include "wlan_pkt_capture_objmgr.h"
#include "wlan_pkt_capture_main.h"
#include "wlan_pkt_capture_ucfg_api.h"

enum pkt_capture_mode ucfg_pkt_capture_get_mode(struct wlan_objmgr_psoc *psoc)
{
	return pkt_capture_get_mode(psoc);
}

QDF_STATUS ucfg_pkt_capture_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_psoc_create_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		pkt_capture_err("Failed to register psoc create handler");
		return status;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_psoc_destroy_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		pkt_capture_err("Failed to register psoc delete handler");
		goto fail_destroy_psoc;
	}

	status = wlan_objmgr_register_vdev_create_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_vdev_create_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		pkt_capture_err("Failed to register vdev create handler");
		goto fail_create_vdev;
	}

	status = wlan_objmgr_register_vdev_destroy_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_vdev_destroy_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		pkt_capture_err("Failed to register vdev destroy handler");
		goto fail_destroy_vdev;
	}
	return status;

fail_destroy_vdev:
	wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_PKT_CAPTURE,
		pkt_capture_vdev_create_notification, NULL);

fail_create_vdev:
	wlan_objmgr_unregister_psoc_destroy_handler(WLAN_UMAC_COMP_PKT_CAPTURE,
		pkt_capture_psoc_destroy_notification, NULL);

fail_destroy_psoc:
	wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_PKT_CAPTURE,
		pkt_capture_psoc_create_notification, NULL);

	return status;
}

void ucfg_pkt_capture_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_vdev_destroy_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_vdev_destroy_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Failed to unregister vdev destroy handler");

	status = wlan_objmgr_unregister_vdev_create_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_vdev_create_notification, NULL);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Failed to unregister vdev create handler");

	status = wlan_objmgr_unregister_psoc_destroy_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_psoc_destroy_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Failed to unregister psoc destroy handler");

	status = wlan_objmgr_unregister_psoc_create_handler(
				WLAN_UMAC_COMP_PKT_CAPTURE,
				pkt_capture_psoc_create_notification,
				NULL);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Failed to unregister psoc create handler");
}
