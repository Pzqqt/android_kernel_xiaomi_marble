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
#include "wlan_p2p_tgt_api.h"
#include "../../core/inc/wlan_p2p_main.h"

QDF_STATUS tgt_p2p_scan_event_cb(uint8_t vdev,
	struct scan_event *event, void *arg)
{
	/* call p2p_process_scan_event directly*/
	/* since this is from target thread */
	return QDF_STATUS_SUCCESS;
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

QDF_STATUS tgt_p2p_mgmt_frame_rx_cb(
	struct wlan_objmgr_psoc *psoc,
	struct wlan_objmgr_peer *peer,
	qdf_nbuf_t buf, void *params,
	enum mgmt_frame_type frm_type)
{
	return QDF_STATUS_SUCCESS;
}

int tgt_p2p_noa_event_cb(void *data, uint8_t *event_buf,
	uint32_t len)
{
	return 0;
}

int tgt_p2p_lo_event_cb(void *data, uint8_t *event_buf, uint32_t len)
{
	return 0;
}
