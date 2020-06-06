/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
 *
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

#include <target_if_son.h>
#include <target_if.h>
#include <wlan_lmac_if_def.h>
#include <wmi_unified_api.h>
#include <cdp_txrx_ctrl.h>

#if QCA_SUPPORT_SON

u_int32_t son_ol_get_peer_rate(struct wlan_objmgr_peer *peer, u_int8_t type)
{
	return ol_if_peer_get_rate(peer, type);
}

QDF_STATUS son_ol_send_null(struct wlan_objmgr_pdev *pdev,
			 u_int8_t *macaddr,
			 struct wlan_objmgr_vdev *vdev)
{
	struct stats_request_params param = {0};
	struct wlan_objmgr_psoc *psoc = NULL;
	wmi_unified_t wmi_handle;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	param.vdev_id = wlan_vdev_get_id(vdev);
	param.stats_id = WMI_HOST_REQUEST_INST_STAT;

	wmi_handle = get_wmi_unified_hdl_from_psoc(psoc);
	if (!wmi_handle)
		return QDF_STATUS_E_FAILURE;

	return wmi_unified_stats_request_send(wmi_handle, macaddr, &param);
}

void target_if_son_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	/* wlan son related function handler */
	tx_ops->son_tx_ops.son_send_null = son_ol_send_null;
	tx_ops->son_tx_ops.get_peer_rate = son_ol_get_peer_rate;
	return;
}

#else

u_int32_t son_ol_get_peer_rate(struct wlan_objmgr_peer *peer, u_int8_t type)
{
	return 0;
}

void target_if_son_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return;
}

QDF_STATUS son_ol_send_null(struct wlan_objmgr_pdev *pdev,
			    u_int8_t *macaddr,
			    struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

#endif
