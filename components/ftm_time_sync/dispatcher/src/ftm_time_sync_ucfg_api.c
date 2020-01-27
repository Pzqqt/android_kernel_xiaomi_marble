/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

/**
 * DOC: Public API implementation of ftm timesync called by north bound iface.
 */

#include "ftm_time_sync_ucfg_api.h"
#include "ftm_time_sync_main.h"
#include <qdf_str.h>

QDF_STATUS ucfg_ftm_timesync_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_vdev_create_handler(
				WLAN_UMAC_COMP_FTM_TIME_SYNC,
				ftm_timesync_vdev_create_notification, NULL);
	if (!QDF_IS_STATUS_SUCCESS(status)) {
		ftm_time_sync_err("Failed to register vdev create handler");
		goto exit;
	}

	status = wlan_objmgr_register_vdev_destroy_handler(
				WLAN_UMAC_COMP_FTM_TIME_SYNC,
				ftm_timesync_vdev_destroy_notification, NULL);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		ftm_time_sync_debug("vdev create/delete notif registered");
		goto exit;
	}

	ftm_time_sync_err("Failed to register vdev delete handler");
	wlan_objmgr_unregister_vdev_create_handler(
			WLAN_UMAC_COMP_FTM_TIME_SYNC,
			ftm_timesync_vdev_create_notification, NULL);

exit:
	return status;
}

void ucfg_ftm_timesync_deinit(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_unregister_vdev_create_handler(
				WLAN_UMAC_COMP_FTM_TIME_SYNC,
				ftm_timesync_vdev_create_notification, NULL);
	if (!QDF_IS_STATUS_SUCCESS(status))
		ftm_time_sync_err("Failed to unregister vdev create handler");

	status = wlan_objmgr_unregister_vdev_destroy_handler(
				WLAN_UMAC_COMP_FTM_TIME_SYNC,
				ftm_timesync_vdev_destroy_notification,
				NULL);
	if (!QDF_IS_STATUS_SUCCESS(status))
		ftm_time_sync_err("Failed to unregister vdev delete handler");
}
