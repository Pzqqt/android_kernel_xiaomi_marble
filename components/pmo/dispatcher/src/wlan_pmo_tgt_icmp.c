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

/**
 * DOC: Implements public API for pmo to interact with target/WMI
 */

#include "wlan_pmo_tgt_api.h"
#include "wlan_pmo_obj_mgmt_public_struct.h"
#include "wlan_pmo_main.h"

QDF_STATUS
pmo_tgt_config_icmp_offload_req(struct wlan_objmgr_psoc *psoc,
				struct pmo_icmp_offload *pmo_icmp_req)
{
	QDF_STATUS status;
	struct wlan_pmo_tx_ops pmo_tx_ops;

	pmo_debug("vdev_id: %d: ICMP offload %d", pmo_icmp_req->vdev_id,
		  pmo_icmp_req->enable);

	pmo_tx_ops = GET_PMO_TX_OPS_FROM_PSOC(psoc);
	if (!pmo_tx_ops.send_icmp_offload_req) {
		pmo_err("send_icmp_offload_req is null");
		status = QDF_STATUS_E_NULL_VALUE;
		return status;
	}

	status = pmo_tx_ops.send_icmp_offload_req(psoc, pmo_icmp_req);

	return status;
}
