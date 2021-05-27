/*
 * Copyright (c) 2011-2021 The Linux Foundation. All rights reserved.
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
 * \file csr_neighbor_roam.h
 *
 * Exports and types for the neighbor roaming algorithm which is sepcifically
 * designed for Android.
 */

#ifndef CSR_NEIGHBOR_ROAM_H
#define CSR_NEIGHBOR_ROAM_H

#include "sme_api.h"
#include "wlan_cm_roam_api.h"

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
/**
 * csr_roam_auth_offload_callback() - Registered CSR Callback function to handle
 * WPA3 roam pre-auth event from firmware.
 * @mac_ctx: Global mac context pointer
 * @vdev_id: Vdev id
 * @bssid: candidate AP bssid
 */
QDF_STATUS
csr_roam_auth_offload_callback(struct mac_context *mac_ctx,
			       uint8_t vdev_id,
			       struct qdf_mac_addr bssid);
#else
static inline QDF_STATUS
csr_roam_auth_offload_callback(struct mac_context *mac_ctx,
			       uint8_t vdev_id,
			       struct qdf_mac_addr bssid)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

/**
 * csr_invoke_neighbor_report_request - Send neighbor report invoke command to
 *					WMA
 * @mac_ctx: MAC context
 * @session_id: session id
 *
 * API called from IW to invoke neighbor report request to WMA then to FW
 *
 * Return: QDF_STATUS
 */
QDF_STATUS csr_invoke_neighbor_report_request(uint8_t session_id,
				struct sRrmNeighborReq *neighbor_report_req,
				bool send_resp_to_host);

#endif /* CSR_NEIGHBOR_ROAM_H */
