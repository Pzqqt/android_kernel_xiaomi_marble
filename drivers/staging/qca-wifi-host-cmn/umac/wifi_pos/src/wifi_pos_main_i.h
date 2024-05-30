/*
 * Copyright (c) 2017, 2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: wifi_pos_main_i.h
 * This file prototyps the important functions pertinent to wifi positioning
 * component.
 */

#ifndef _WIFI_POS_MAIN_H_
#define _WIFI_POS_MAIN_H_

#ifdef CNSS_GENL
#define ENHNC_FLAGS_LEN 4
#define NL_ENABLE_OEM_REQ_RSP 0x00000001
#endif

/* forward reference */
struct wlan_objmgr_psoc;

/**
 * wifi_pos_psoc_obj_created_notification: callback registered to be called when
 * psoc object is created.
 * @psoc: pointer to psoc object just created
 * @arg_list: argument list
 *
 * This function will:
 *         create WIFI POS psoc object and attach to psoc
 *         register TLV vs nonTLV callbacks
 * Return: status of operation
 */
QDF_STATUS wifi_pos_psoc_obj_created_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list);

/**
 * wifi_pos_psoc_obj_destroyed_notification: callback registered to be called
 * when psoc object is destroyed.
 * @psoc: pointer to psoc object just about to be destroyed
 * @arg_list: argument list
 *
 * This function will:
 *         detach WIFI POS from psoc object and free
 * Return: status of operation
 */
QDF_STATUS  wifi_pos_psoc_obj_destroyed_notification(
				struct wlan_objmgr_psoc *psoc, void *arg_list);

/**
 * wifi_pos_oem_rsp_handler: lmac rx ops registered
 * @psoc: pointer to psoc object
 * @oem_rsp: response from firmware
 *
 * Return: status of operation
 */
int wifi_pos_oem_rsp_handler(struct wlan_objmgr_psoc *psoc,
			     struct oem_data_rsp *oem_rsp);

/**
 * wifi_pos_get_tx_ops: api to get tx ops
 * @psoc: pointer to psoc object
 *
 * Return: tx ops
 */
struct wlan_lmac_if_wifi_pos_tx_ops *
	wifi_pos_get_tx_ops(struct wlan_objmgr_psoc *psoc);
#endif
