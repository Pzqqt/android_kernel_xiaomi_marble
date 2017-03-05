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

/**
 * DOC: This file contains p2p south bound interface definitions
 */

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_mgmt_txrx_utils_api.h>
#include <scheduler_api.h>
#include "wlan_p2p_tgt_api.h"
#include "wlan_p2p_public_struct.h"
#include "../../core/src/wlan_p2p_main.h"

void tgt_p2p_scan_event_cb(struct wlan_objmgr_vdev *vdev,
	struct scan_event *event, void *arg)
{
}

QDF_STATUS tgt_p2p_mgmt_download_comp_cb(void *context,
	qdf_nbuf_t buf, bool free)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_p2p_mgmt_ota_comp_cb(void *context, qdf_nbuf_t buf,
	uint32_t status, void *tx_compl_params)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_p2p_mgmt_frame_rx_cb(struct wlan_objmgr_psoc *psoc,
	struct wlan_objmgr_peer *peer, qdf_nbuf_t buf,
	struct mgmt_rx_event_params *mgmt_rx_params,
	enum mgmt_frame_type frm_type)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS  tgt_p2p_noa_event_cb(struct wlan_objmgr_psoc *psoc,
		struct p2p_noa_info *event_info)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS tgt_p2p_lo_event_cb(struct wlan_objmgr_psoc *psoc,
		struct p2p_lo_event *event_info)
{
	return QDF_STATUS_SUCCESS;
}
