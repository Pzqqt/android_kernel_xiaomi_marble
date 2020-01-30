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
 * DOC: Target interface file for pkt_capture component to
 * Implement api's which shall be used by pkt_capture component
 * in target_if internally.
 */

#include <wlan_pkt_capture_main.h>
#include <wlan_pkt_capture_ucfg_api.h>
#include <target_if_pkt_capture.h>
#include <wmi_unified_api.h>
#include <target_if.h>
#include <init_deinit_lmac.h>

QDF_STATUS target_if_set_packet_capture_mode(struct wlan_objmgr_psoc *psoc,
					     uint8_t vdev_id,
					     enum pkt_capture_mode mode)
{
	wmi_unified_t wmi_handle = lmac_get_wmi_unified_hdl(psoc);
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	if (!wmi_handle) {
		target_if_err("Invalid wmi handle");
		return QDF_STATUS_E_INVAL;
	}

	target_if_debug("psoc:%pK, vdev_id:%d mode:%d",
			psoc, vdev_id, mode);

	if (mode != PACKET_CAPTURE_MODE_DISABLE) {
		struct vdev_set_params param;

		param.vdev_id = vdev_id;
		param.param_id = WMI_VDEV_PARAM_PACKET_CAPTURE_MODE;
		param.param_value = (uint32_t)mode;

		status = wmi_unified_vdev_set_param_send(wmi_handle, &param);
		if (QDF_IS_STATUS_SUCCESS(status))
			ucfg_pkt_capture_set_pktcap_mode(psoc, mode);
		else
			pkt_capture_err("failed to set packet capture mode");
	}
	return status;
}
