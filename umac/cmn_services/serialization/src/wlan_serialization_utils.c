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
 * DOC: wlan_serialization_utils.c
 * This file defines the utility helper functions for serialization component.
 */

#include "wlan_serialization_utils_i.h"

QDF_STATUS wlan_serialization_validate_cmd(
		 enum wlan_umac_comp_id comp_id,
		 enum wlan_serialization_cmd_type cmd_type)
{
	serialization_info("validate cmd_type:%d, comp_id:%d",
			cmd_type, comp_id);
	if (cmd_type < 0 || comp_id < 0 ||
			cmd_type >= WLAN_SER_CMD_MAX ||
			comp_id >= WLAN_UMAC_COMP_ID_MAX) {
		serialization_err("Invalid cmd or comp passed");
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

void wlan_serialization_release_list_cmds(
		struct wlan_serialization_pdev_priv_obj *ser_pdev_obj,
		qdf_list_t *list)
{
	qdf_list_node_t *node = NULL;

	while (!qdf_list_empty(list)) {
		qdf_list_remove_front(list, &node);
		qdf_list_insert_back(&ser_pdev_obj->global_cmd_pool_list, node);
	}

	return;
}

void wlan_serialization_destroy_list(
		struct wlan_serialization_pdev_priv_obj *ser_pdev_obj,
		qdf_list_t *list)
{
	wlan_serialization_release_list_cmds(ser_pdev_obj, list);
	qdf_list_destroy(list);
}

struct wlan_serialization_psoc_priv_obj *wlan_serialization_get_psoc_priv_obj(
		struct wlan_objmgr_psoc *psoc)
{
	struct wlan_serialization_psoc_priv_obj *ser_soc_obj;
	wlan_psoc_obj_lock(psoc);
	ser_soc_obj = (struct wlan_serialization_psoc_priv_obj *)
			wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_SERIALIZATION);
	wlan_psoc_obj_unlock(psoc);

	return ser_soc_obj;
}

struct wlan_serialization_pdev_priv_obj *wlan_serialization_get_pdev_priv_obj(
		struct wlan_objmgr_pdev *pdev)
{
	struct wlan_serialization_pdev_priv_obj *obj;
	wlan_pdev_obj_lock(pdev);
	obj = (struct wlan_serialization_pdev_priv_obj *)
			wlan_objmgr_pdev_get_comp_private_obj(pdev,
					WLAN_UMAC_COMP_SERIALIZATION);
	wlan_pdev_obj_unlock(pdev);

	return obj;
}

