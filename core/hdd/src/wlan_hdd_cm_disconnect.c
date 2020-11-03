/*
 * Copyright (c) 2012-2020, The Linux Foundation. All rights reserved.
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
 * DOC: hdd_cm_disconnect.c
 *
 * WLAN Host Device Driver disconnect APIs implementation
 *
 */

#include "wlan_hdd_main.h"
#include "wlan_hdd_object_manager.h"
#include "wlan_hdd_trace.h"
#include <osif_cm_req.h>
#include "wlan_hdd_cm_api.h"

int wlan_hdd_cm_disconnect(struct wiphy *wiphy,
			   struct net_device *dev, u16 reason)
{
	struct hdd_adapter *adapter = WLAN_HDD_GET_PRIV_PTR(dev);
	int status;
	struct wlan_objmgr_vdev *vdev;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam()) {
		hdd_err("Command not allowed in FTM mode");
		return -EINVAL;
	}

	if (wlan_hdd_validate_vdev_id(adapter->vdev_id))
		return -EINVAL;

	vdev = hdd_objmgr_get_vdev(adapter);
	if (!vdev)
		return -EINVAL;

	qdf_mtrace(QDF_MODULE_ID_HDD, QDF_MODULE_ID_HDD,
		   TRACE_CODE_HDD_CFG80211_DISCONNECT,
		   adapter->vdev_id, reason);

	hdd_print_netdev_txq_status(dev);

	if (reason == WLAN_REASON_DEAUTH_LEAVING)
		qdf_dp_trace_dump_all(
				WLAN_DEAUTH_DPTRACE_DUMP_COUNT,
				QDF_TRACE_DEFAULT_PDEV_ID);

	/* To-Do: This is static api move this api to this file
	 * once this is removed from wlan_hdd_cfg80211.c file
	 * reset_mscs_params(adapter);
	 */
	wlan_hdd_netif_queue_control(adapter,
				     WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
				     WLAN_CONTROL_PATH);
	status = osif_cm_disconnect(dev, vdev, reason);
	hdd_objmgr_put_vdev(vdev);

	return status;
}
