/*
 * Copyright (c) 2017, 2020 The Linux Foundation. All rights reserved.
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
 * DOC: wifi_pos_ucfg_i.h
 * This file prototyps the important functions pertinent to wifi positioning
 * component.
 */

#ifndef _WIFI_POS_UCFG_H_
#define _WIFI_POS_UCFG_H_

#include "qdf_types.h"
#include "qdf_status.h"

struct wlan_objmgr_psoc;
struct wifi_pos_req_msg;

/**
 * ucfg_wifi_pos_process_req: ucfg API to be called from HDD/OS_IF to process a
 * wifi_pos request from userspace
 * @psoc: pointer to psoc object
 * @req: wifi_pos request msg
 * @send_rsp_cb: callback pointer required to send msg to userspace
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_wifi_pos_process_req(struct wlan_objmgr_psoc *psoc,
				     struct wifi_pos_req_msg *req,
				     wifi_pos_send_rsp_handler send_rsp_cb);

#endif /* _WIFI_POS_UCFG_H_ */
