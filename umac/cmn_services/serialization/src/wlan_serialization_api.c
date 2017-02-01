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
 * DOC: wlan_serialization_api.c
 * This file provides an interface for the external components
 * to utilize the services provided by the serialization
 * component.
 */

/* Include files */
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_serialization_main_i.h"

QDF_STATUS
wlan_serialization_register_comp_info_cb(struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id comp_id,
		enum wlan_serialization_cmd_type cmd_type,
		wlan_serialization_comp_info_cb cb)
{
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj;
	QDF_STATUS status;

	status = wlan_serialization_validate_cmd(comp_id, cmd_type);
	if (status != QDF_STATUS_SUCCESS)
		return status;
	ser_soc_obj = wlan_serialization_get_psoc_priv_obj(psoc);
	ser_soc_obj->comp_info_cb[cmd_type][comp_id] = cb;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_serialization_deregister_comp_info_cb(struct wlan_objmgr_psoc *psoc,
		enum wlan_umac_comp_id comp_id,
		enum wlan_serialization_cmd_type cmd_type)
{
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj;
	QDF_STATUS status;

	status = wlan_serialization_validate_cmd(comp_id, cmd_type);
	if (status != QDF_STATUS_SUCCESS)
		return status;
	ser_soc_obj = wlan_serialization_get_psoc_priv_obj(psoc);
	ser_soc_obj->comp_info_cb[cmd_type][comp_id] = NULL;

	return QDF_STATUS_SUCCESS;
}

enum wlan_serialization_cmd_status
wlan_serialization_vdev_scan_status(struct wlan_objmgr_vdev *vdev)
{
	serialization_info("vdev scan status entry");

	return WLAN_SER_CMD_NOT_FOUND;
}

enum wlan_serialization_cmd_status
wlan_serialization_pdev_scan_status(struct wlan_objmgr_pdev *pdev)
{
	serialization_info("pdev scan status entry");

	return WLAN_SER_CMD_NOT_FOUND;
}

enum wlan_serialization_cmd_status
wlan_serialization_non_scan_cmd_status(struct wlan_objmgr_pdev *pdev,
		enum wlan_serialization_cmd_type cmd_id)
{
	serialization_info("pdev non cmd status entry");

	return WLAN_SER_CMD_NOT_FOUND;
}

enum wlan_serialization_cmd_status
wlan_serialization_cancel_request(
		struct wlan_serialization_queued_cmd_info *req)
{
	serialization_info("serialization cancel request entry");

	return WLAN_SER_CMD_NOT_FOUND;
}

void wlan_serialization_remove_cmd(
		struct wlan_serialization_queued_cmd_info *cmd)
{
	serialization_info("serialization remove request entry");

	return;
}

enum wlan_serialization_status
wlan_serialization_request(struct wlan_serialization_command *cmd)
{
	serialization_info("serialization queue cmd entry");

	return WLAN_SER_CMD_NOT_FOUND;
}

void wlan_serialization_flush_cmd(
		struct wlan_serialization_queued_cmd_info *cmd)
{
	serialization_info("serialization cmd flushed");

	return;
}

