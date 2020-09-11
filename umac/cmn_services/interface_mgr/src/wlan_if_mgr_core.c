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

/*
 * DOC: contains interface manager public api
 */
#include "wlan_if_mgr_public_struct.h"
#include "wlan_if_mgr_api.h"
#include "wlan_if_mgr_sta.h"
#include "wlan_if_mgr_ap.h"
#include "wlan_if_mgr_main.h"
#include "wlan_if_mgr_roam.h"

QDF_STATUS if_mgr_deliver_event(struct wlan_objmgr_vdev *vdev,
				enum wlan_if_mgr_evt event,
				struct if_mgr_event_data *event_data)
{
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	ifmgr_debug("IF MGR event received: %d", event);

	switch (event) {
	case WLAN_IF_MGR_EV_CONNECT_START:
		status = if_mgr_connect_start(vdev, event_data);
		break;
	case WLAN_IF_MGR_EV_CONNECT_COMPLETE:
		status = if_mgr_connect_complete(vdev, event_data);
		break;
	case WLAN_IF_MGR_EV_AP_START_BSS:
		status = if_mgr_ap_start_bss(vdev, event_data);
		break;
	case WLAN_IF_MGR_EV_AP_START_BSS_COMPLETE:
		status = if_mgr_ap_start_bss_complete(vdev, event_data);
		break;
	case WLAN_IF_MGR_EV_AP_STOP_BSS:
		status = if_mgr_ap_stop_bss(vdev, event_data);
		break;
	case WLAN_IF_MGR_EV_AP_STOP_BSS_COMPLETE:
		status = if_mgr_ap_stop_bss_complete(vdev, event_data);
		break;
	case WLAN_IF_MGR_EV_DISCONNECT_START:
		status = if_mgr_disconnect_start(vdev, event_data);
		break;
	case WLAN_IF_MGR_EV_DISCONNECT_COMPLETE:
		status = if_mgr_disconnect_complete(vdev, event_data);
		break;
	case WLAN_IF_MGR_EV_VALIDATE_CANDIDATE:
		status = if_mgr_validate_candidate(vdev, event_data);
		break;
	default:
		status = QDF_STATUS_E_INVAL;
		ifmgr_err("Invalid event");
		break;
	}

	return status;
}
