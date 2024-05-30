/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_serialization_legacy_api.c
 * This file provides prototypes of the routines needed for the
 * legacy mcl serialization to utilize the services provided by the
 * serialization component.
 */

#include "wlan_serialization_legacy_api.h"
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utils_i.h"
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_serialization_internal_i.h"
#include "wlan_serialization_scan_i.h"
#include "wlan_serialization_non_scan_i.h"

static struct wlan_objmgr_pdev *wlan_serialization_get_first_pdev(
			struct wlan_objmgr_psoc *psoc)
{
	struct wlan_objmgr_pdev *pdev;
	uint8_t i = 0;

	if (!psoc) {
		ser_err("invalid psoc");
		return NULL;
	}
	for (i = 0; i < WLAN_UMAC_MAX_PDEVS; i++) {
		pdev = wlan_objmgr_get_pdev_by_id(psoc, i,
					WLAN_SERIALIZATION_ID);
		if (pdev)
			break;
	}

	return pdev;
}

static struct wlan_ser_pdev_obj *
wlan_serialization_get_pdev_priv_obj_using_psoc(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_ser_pdev_obj *ser_pdev_obj;

	if (!psoc) {
		ser_err("invalid psoc");
		return NULL;
	}

	pdev = wlan_serialization_get_first_pdev(psoc);
	if (!pdev) {
		ser_err("invalid pdev");
		return NULL;
	}

	ser_pdev_obj =  wlan_serialization_get_pdev_obj(pdev);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_SERIALIZATION_ID);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return NULL;
	}

	return ser_pdev_obj;
}

uint32_t wlan_serialization_get_pending_list_count(
				struct wlan_objmgr_psoc *psoc,
				uint8_t is_cmd_from_pending_scan_queue)
{
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	qdf_list_t *queue;
	uint32_t count = 0;
	struct wlan_serialization_pdev_queue  *pdev_queue;

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj_using_psoc(psoc);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return 0;
	}

	if (is_cmd_from_pending_scan_queue)
		pdev_queue = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_SCAN];
	else
		pdev_queue =
		&ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_NON_SCAN];
	queue = &pdev_queue->pending_list;
	wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);
	count = qdf_list_size(queue);
	wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);

	return count;
}

struct wlan_serialization_command*
wlan_serialization_peek_head_active_cmd_using_psoc(
			struct wlan_objmgr_psoc *psoc,
			uint8_t is_cmd_from_active_scan_queue)
{
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_command_list *cmd_list = NULL;
	struct wlan_serialization_command *cmd = NULL;
	qdf_list_node_t *nnode = NULL;
	qdf_list_t *queue;
	struct wlan_serialization_pdev_queue  *pdev_queue;

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj_using_psoc(psoc);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return NULL;
	}

	if (is_cmd_from_active_scan_queue)
		pdev_queue = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_SCAN];
	else
		pdev_queue =
		&ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_NON_SCAN];
	queue = &pdev_queue->active_list;
	if (wlan_serialization_list_empty(queue)) {
		ser_debug_rl("Empty Queue");
		goto end;
	}

	if (QDF_STATUS_SUCCESS != wlan_serialization_get_cmd_from_queue(queue,
						&nnode)) {
		ser_err("Can't get command from queue");
		goto end;
	}

	cmd_list = qdf_container_of(nnode,
			struct wlan_serialization_command_list, pdev_node);
	cmd = &cmd_list->cmd;

end:
	return cmd;
}

struct wlan_serialization_command*
wlan_serialization_peek_head_pending_cmd_using_psoc(
			struct wlan_objmgr_psoc *psoc,
			uint8_t is_cmd_from_pending_scan_queue)
{
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	struct wlan_serialization_command_list *cmd_list = NULL;
	struct wlan_serialization_command *cmd = NULL;
	qdf_list_node_t *nnode = NULL;
	qdf_list_t *queue;
	struct wlan_serialization_pdev_queue  *pdev_queue;

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj_using_psoc(psoc);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return NULL;
	}
	if (is_cmd_from_pending_scan_queue)
		pdev_queue = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_SCAN];
	else
		pdev_queue =
		&ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_NON_SCAN];
	queue = &pdev_queue->pending_list;
	if (wlan_serialization_list_empty(queue))
		goto end;

	wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);
	if (QDF_STATUS_SUCCESS != wlan_serialization_get_cmd_from_queue(
							queue,
							&nnode)) {
		wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);
		ser_err("Can't get command from queue");
		goto end;
	}
	wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);
	cmd_list = qdf_container_of(nnode,
			struct wlan_serialization_command_list, pdev_node);
	cmd = &cmd_list->cmd;
	ser_debug("cmd_type[%d] cmd_id[%d]matched",
		  cmd_list->cmd.cmd_type, cmd_list->cmd.cmd_id);

end:
	return cmd;
}

static struct wlan_serialization_command*
wlan_serialization_get_list_next_node(qdf_list_t *queue,
			struct wlan_serialization_command *cmd,
			struct wlan_ser_pdev_obj *ser_pdev_obj)
{
	struct wlan_serialization_command_list *cmd_list = NULL;
	qdf_list_node_t *pnode = NULL, *nnode = NULL;
	bool found = false;
	uint32_t i = 0;
	QDF_STATUS status;
	struct wlan_serialization_command *ret_cmd = NULL;

	i = wlan_serialization_list_size(queue);
	if (i == 0) {
		ser_err("Empty Queue");
		return NULL;
	}
	while (i--) {
		if (!cmd_list)
			status = wlan_serialization_peek_front(queue, &nnode);
		else
			status = wlan_serialization_peek_next(queue, pnode,
							      &nnode);

		if ((status != QDF_STATUS_SUCCESS) || found)
			break;

		pnode = nnode;
		cmd_list = qdf_container_of(
					nnode,
					struct wlan_serialization_command_list,
					pdev_node);
		if (wlan_serialization_match_cmd_id_type(
				nnode, cmd, WLAN_SER_PDEV_NODE) &&
		    wlan_serialization_match_cmd_vdev(nnode,
						      cmd->vdev,
						      WLAN_SER_PDEV_NODE)) {
			found = true;
		}
		nnode = NULL;
	}
	if (nnode && found) {
		cmd_list = qdf_container_of(
				nnode,
				struct wlan_serialization_command_list,
				pdev_node);
		ret_cmd = &cmd_list->cmd;
	}
	if (!found) {
		ser_err("Can't locate next command");
		return NULL;
	}
	if (!nnode) {
		ser_debug("next node is empty, so fine");
		return NULL;
	}

	return ret_cmd;
}

struct wlan_serialization_command*
wlan_serialization_get_pending_list_next_node_using_psoc(
			struct wlan_objmgr_psoc *psoc,
			struct wlan_serialization_command *prev_cmd,
			uint8_t is_cmd_for_pending_scan_queue)
{
	struct wlan_ser_pdev_obj *ser_pdev_obj;
	qdf_list_t *queue;
	struct wlan_serialization_pdev_queue  *pdev_queue;
	struct wlan_serialization_command *cmd;

	if (!prev_cmd) {
		ser_err("invalid prev_cmd");
		return NULL;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj_using_psoc(psoc);
	if (!ser_pdev_obj) {
		ser_err("invalid ser_pdev_obj");
		return NULL;
	}
	if (is_cmd_for_pending_scan_queue)
		pdev_queue = &ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_SCAN];
	else
		pdev_queue =
		&ser_pdev_obj->pdev_q[SER_PDEV_QUEUE_COMP_NON_SCAN];
	queue = &pdev_queue->pending_list;

	wlan_serialization_acquire_lock(&pdev_queue->pdev_queue_lock);
	cmd = wlan_serialization_get_list_next_node(queue, prev_cmd,
						    ser_pdev_obj);
	wlan_serialization_release_lock(&pdev_queue->pdev_queue_lock);

	return cmd;
}
