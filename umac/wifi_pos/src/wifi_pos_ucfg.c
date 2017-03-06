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
 * This file defines the important dispatcher APIs pertinent to
 * wifi positioning.
 */
#include "wifi_pos_utils_i.h"
#include "wifi_pos_api.h"
#include "wifi_pos_ucfg_i.h"

QDF_STATUS ucfg_wifi_pos_process_req(struct wlan_objmgr_psoc *psoc,
		struct wifi_pos_req_msg *req,
		void (*send_rsp_cb)(uint32_t *,
			uint32_t, uint32_t, uint8_t *))
{
	struct wifi_pos_psoc_priv_obj *wifi_pos_psoc_obj =
					wifi_pos_get_psoc_priv_obj(psoc);

	if (!wifi_pos_psoc_obj) {
		wifi_pos_err("wifi_pos_psoc_obj is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	/* assign callback pointer to be called for rsp or error */
	wifi_pos_psoc_obj->wifi_pos_send_rsp = send_rsp_cb;

	return wifi_pos_psoc_obj->wifi_pos_req_handler(psoc, req);
}
