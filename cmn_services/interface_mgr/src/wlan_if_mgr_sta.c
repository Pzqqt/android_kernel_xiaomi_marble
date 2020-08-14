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
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_if_mgr_public_struct.h"
#include "wlan_if_mgr_sta.h"
#include "nan_ucfg_api.h"
#include "wlan_policy_mgr_api.h"

QDF_STATUS wlan_process_connect_start(struct wlan_objmgr_vdev *vdev,
				      struct if_mgr_event_data *event_data)
{
	uint8_t sta_cnt, sap_cnt;
	struct wlan_objmgr_psoc *psoc;
	enum QDF_OPMODE op_mode;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	/*
	 * Disable NAN Discovery if incoming connection is P2P or if a STA
	 * connection already exists and if this is a case of STA+STA
	 * or SAP+STA concurrency
	 */
	sta_cnt = policy_mgr_mode_specific_connection_count(psoc,
							    PM_STA_MODE,
							    NULL);
	sap_cnt = policy_mgr_mode_specific_connection_count(psoc,
							    PM_SAP_MODE,
							    NULL);
	op_mode = wlan_vdev_mlme_get_opmode(vdev);
	if (op_mode == QDF_P2P_CLIENT_MODE || sap_cnt || sta_cnt)
		ucfg_nan_disable_concurrency(psoc);

	/* STA+NDI concurrency gets preference over NDI+NDI. Disable
	 * first NDI in case an NDI+NDI concurrency exists.
	 */
	ucfg_nan_check_and_disable_unsupported_ndi(psoc, false);

	/*
	 * STA+NDI concurrency gets preference over NDI+NDI. Disable
	 * first NDI in case an NDI+NDI concurrency exists if FW does
	 * not support 4 port concurrency of two NDI + NAN with STA.
	 */
	if (!ucfg_nan_is_sta_nan_ndi_4_port_allowed(psoc))
		ucfg_nan_check_and_disable_unsupported_ndi(psoc,
							   false);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_process_connect_complete(struct wlan_objmgr_vdev *vdev,
					 struct if_mgr_event_data *event_data)
{
	return QDF_STATUS_SUCCESS;
}
