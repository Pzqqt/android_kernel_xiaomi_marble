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
 * DOC: wlan_serialization_legacy_api.c
 * This file provides prototypes of the routines needed for the
 * legacy mcl serialization to utilize the services provided by the
 * serialization component.
 */

#include "wlan_serialization_legacy_api.h"
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utils_i.h"
#include "wlan_objmgr_vdev_obj.h"

extern struct serialization_legacy_callback ser_legacy_cb;

static struct wlan_objmgr_pdev *wlan_serialization_get_first_pdev(
			struct wlan_objmgr_psoc *psoc)
{
	struct wlan_objmgr_pdev *pdev;
	uint8_t i = 0;

	if (!psoc) {
		serialization_err("invalid psoc");
		return NULL;
	}
	for (i = 0; i < WLAN_UMAC_MAX_PDEVS; i++) {
		pdev = wlan_objmgr_get_pdev_by_id(psoc, i,
					WLAN_SERIALIZATION_ID);
		if (pdev != NULL)
			break;
	}

	return pdev;
}

static struct wlan_serialization_pdev_priv_obj *
wlan_serialization_get_pdev_priv_obj_using_psoc(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_objmgr_pdev *pdev = NULL;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;

	if (!psoc) {
		serialization_err("invalid psoc");
		return NULL;
	}

	pdev = wlan_serialization_get_first_pdev(psoc);
	if (!pdev) {
		serialization_err("invalid pdev");
		return NULL;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj(pdev);
	wlan_objmgr_pdev_release_ref(pdev, WLAN_SERIALIZATION_ID);
	if (!ser_pdev_obj) {
		serialization_err("invalid ser_pdev_obj");
		return NULL;
	}

	return ser_pdev_obj;
}

uint32_t wlan_serialization_get_active_list_count(
			struct wlan_objmgr_psoc *psoc,
			uint8_t is_cmd_from_active_scan_queue)
{
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	qdf_list_t *queue;

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj_using_psoc(psoc);
	if (!ser_pdev_obj) {
		serialization_err("invalid ser_pdev_obj");
		return 0;
	}

	if (is_cmd_from_active_scan_queue)
		queue = &ser_pdev_obj->active_scan_list;
	else
		queue = &ser_pdev_obj->active_list;

	return qdf_list_size(queue);
}

uint32_t wlan_serialization_get_pending_list_count(
				struct wlan_objmgr_psoc *psoc,
				uint8_t is_cmd_from_pending_scan_queue)
{
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	qdf_list_t *queue;

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj_using_psoc(psoc);
	if (!ser_pdev_obj) {
		serialization_err("invalid ser_pdev_obj");
		return 0;
	}

	if (is_cmd_from_pending_scan_queue)
		queue = &ser_pdev_obj->pending_scan_list;
	else
		queue = &ser_pdev_obj->pending_list;

	return qdf_list_size(queue);
}

struct wlan_serialization_command*
wlan_serialization_peek_head_active_cmd_using_psoc(
			struct wlan_objmgr_psoc *psoc,
			uint8_t is_cmd_from_active_scan_queue)
{
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	struct wlan_serialization_command_list *cmd_list = NULL;
	struct wlan_serialization_command *cmd = NULL;
	qdf_list_node_t *nnode = NULL;
	qdf_list_t *queue;

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj_using_psoc(psoc);
	if (!ser_pdev_obj) {
		serialization_err("invalid ser_pdev_obj");
		return 0;
	}

	if (is_cmd_from_active_scan_queue)
		queue = &ser_pdev_obj->active_scan_list;
	else
		queue = &ser_pdev_obj->active_list;
	if (!qdf_list_size(queue)) {
		serialization_err("Empty Queue");
		return NULL;
	}

	if (QDF_STATUS_SUCCESS != wlan_serialization_get_cmd_from_queue(queue,
						&nnode)) {
		serialization_err("Can't get command from queue");
		return NULL;
	}

	cmd_list = qdf_container_of(nnode,
			struct wlan_serialization_command_list, node);
	serialization_debug("cmd_type[%d]", cmd_list->cmd.cmd_type);
	cmd = &cmd_list->cmd;

	return cmd;
}

struct wlan_serialization_command*
wlan_serialization_peek_head_pending_cmd_using_psoc(
			struct wlan_objmgr_psoc *psoc,
			uint8_t is_cmd_from_pending_scan_queue)
{
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	struct wlan_serialization_command_list *cmd_list = NULL;
	struct wlan_serialization_command *cmd = NULL;
	qdf_list_node_t *nnode = NULL;
	qdf_list_t *queue;

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj_using_psoc(psoc);
	if (!ser_pdev_obj) {
		serialization_err("invalid ser_pdev_obj");
		return 0;
	}

	if (is_cmd_from_pending_scan_queue)
		queue = &ser_pdev_obj->pending_scan_list;
	else
		queue = &ser_pdev_obj->pending_list;
	if (!qdf_list_size(queue)) {
		serialization_err("Empty Queue");
		return NULL;
	}

	if (QDF_STATUS_SUCCESS != wlan_serialization_get_cmd_from_queue(queue,
							&nnode)) {
		serialization_err("Can't get command from queue");
		return NULL;
	}
	cmd_list = qdf_container_of(nnode,
			struct wlan_serialization_command_list, node);
	serialization_debug("cmd_type[%d] matched", cmd_list->cmd.cmd_type);
	cmd = &cmd_list->cmd;

	return cmd;
}

static struct wlan_serialization_command*
wlan_serialization_get_list_next_node(qdf_list_t *queue,
				struct wlan_serialization_command *cmd)
{
	struct wlan_serialization_command_list *cmd_list = NULL;
	qdf_list_node_t *pnode = NULL, *nnode = NULL;
	bool found = false;
	uint32_t i = 0;
	QDF_STATUS status;

	if (!qdf_list_empty(queue)) {
		i = qdf_list_size(queue);
		while (i--) {
			if (!cmd_list)
				status = qdf_list_peek_front(queue, &nnode);
			else
				status = qdf_list_peek_next(queue, pnode,
							&nnode);

			if ((status != QDF_STATUS_SUCCESS) || found)
				break;

			pnode = nnode;
			cmd_list = qdf_container_of(nnode,
					struct wlan_serialization_command_list,
					node);
			if ((cmd_list->cmd.cmd_id == cmd->cmd_id) &&
				(cmd_list->cmd.cmd_type == cmd->cmd_type) &&
					(cmd_list->cmd.vdev == cmd->vdev)) {
				found = true;
			}
			nnode = NULL;
		}
	}
	if (nnode && found) {
		cmd_list = qdf_container_of(nnode,
				struct wlan_serialization_command_list, node);
		return &cmd_list->cmd;
	} else if (!found) {
		serialization_err("Can't locate next command");
		return NULL;
	} else {
		serialization_debug("next node is empty, so fine");
		return NULL;
	}
}

struct wlan_serialization_command*
wlan_serialization_get_active_list_next_node_using_psoc(
			struct wlan_objmgr_psoc *psoc,
			struct wlan_serialization_command *prev_cmd,
			uint8_t is_cmd_for_active_scan_queue)
{
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	qdf_list_t *queue;

	if (!prev_cmd) {
		serialization_err("invalid prev_cmd");
		return NULL;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj_using_psoc(psoc);
	if (!ser_pdev_obj) {
		serialization_err("invalid ser_pdev_obj");
		return 0;
	}

	if (is_cmd_for_active_scan_queue)
		queue = &ser_pdev_obj->active_scan_list;
	else
		queue = &ser_pdev_obj->active_list;
	if (!qdf_list_size(queue)) {
		serialization_err("Empty Queue");
		return NULL;
	}

	return wlan_serialization_get_list_next_node(queue, prev_cmd);
}

struct wlan_serialization_command*
wlan_serialization_get_pending_list_next_node_using_psoc(
			struct wlan_objmgr_psoc *psoc,
			struct wlan_serialization_command *prev_cmd,
			uint8_t is_cmd_for_pending_scan_queue)
{
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	qdf_list_t *queue;

	if (!prev_cmd) {
		serialization_err("invalid prev_cmd");
		return NULL;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj_using_psoc(psoc);
	if (!ser_pdev_obj) {
		serialization_err("invalid ser_pdev_obj");
		return 0;
	}

	if (is_cmd_for_pending_scan_queue)
		queue = &ser_pdev_obj->pending_scan_list;
	else
		queue = &ser_pdev_obj->pending_list;
	if (!qdf_list_size(queue)) {
		serialization_err("Empty Queue");
		return NULL;
	}

	return wlan_serialization_get_list_next_node(queue, prev_cmd);
}

struct wlan_serialization_command*
wlan_serialization_get_scan_cmd_using_scan_id(
		struct wlan_objmgr_psoc *psoc,
		uint8_t vdev_id, uint16_t scan_id,
		uint8_t is_scan_cmd_from_active_queue)
{
	uint32_t qlen;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	struct wlan_serialization_command_list *cmd_list = NULL;
	struct wlan_serialization_command *cmd = NULL;
	qdf_list_node_t *nnode = NULL;
	qdf_list_t *queue;

	if (!psoc) {
		serialization_err("invalid psoc");
		return cmd;
	}
	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
						WLAN_SERIALIZATION_ID);
	if (!vdev) {
		serialization_err("invalid vdev");
		return cmd;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		serialization_err("invalid pdev");
		goto release_vdev_ref;
	}

	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj(pdev);
	if (!ser_pdev_obj) {
		serialization_err("invalid ser_pdev_obj");
		goto release_vdev_ref;
	}
	if (is_scan_cmd_from_active_queue)
		queue = &ser_pdev_obj->active_scan_list;
	else
		queue = &ser_pdev_obj->pending_scan_list;
	qlen = qdf_list_size(queue);
	if (!qlen) {
		serialization_err("Empty Queue");
		goto release_vdev_ref;
	}
	while (qlen--) {
		if (QDF_STATUS_SUCCESS != wlan_serialization_get_cmd_from_queue(
							queue, &nnode)) {
			serialization_err("unsuccessful attempt");
			break;
		}
		cmd_list = qdf_container_of(nnode,
				struct wlan_serialization_command_list, node);
		if ((cmd_list->cmd.cmd_id == scan_id) &&
				(cmd_list->cmd.vdev == vdev)) {
			serialization_debug("cmd_id[%d] matched", scan_id);
			cmd = &cmd_list->cmd;
			break;
		}
	}
release_vdev_ref:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SERIALIZATION_ID);
	return cmd;
}

void wlan_serialization_legacy_init_callback(void)
{
	ser_legacy_cb.serialization_purge_cmd_list =
			wlan_serialization_purge_cmd_list;
}

void wlan_serialization_purge_cmd_list_by_vdev_id(struct wlan_objmgr_psoc *psoc,
			uint8_t vdev_id, bool purge_scan_active_queue,
			bool purge_scan_pending_queue,
			bool purge_nonscan_active_queue,
			bool purge_nonscan_pending_queue,
			bool purge_all_queues)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, vdev_id,
			WLAN_SERIALIZATION_ID);
	if (!vdev) {
		serialization_err("Invalid vdev");
		return;
	}
	wlan_serialization_purge_cmd_list(psoc, vdev, purge_scan_active_queue,
				purge_scan_pending_queue,
				purge_nonscan_active_queue,
				purge_nonscan_pending_queue,
				purge_all_queues);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_SERIALIZATION_ID);
}

void wlan_serialization_purge_cmd_list(struct wlan_objmgr_psoc *psoc,
		struct wlan_objmgr_vdev *vdev,
		bool purge_scan_active_queue,
		bool purge_scan_pending_queue,
		bool purge_nonscan_active_queue,
		bool purge_nonscan_pending_queue,
		bool purge_all_queues)
{
	struct wlan_serialization_timer *ser_timer;
	struct wlan_serialization_psoc_priv_obj *psoc_ser_obj;
	struct wlan_serialization_pdev_priv_obj *ser_pdev_obj;
	struct wlan_objmgr_pdev *pdev = NULL;
	uint32_t i = 0;

	if (!psoc) {
		serialization_err("Invalid psoc");
		return;
	}
	ser_pdev_obj = wlan_serialization_get_pdev_priv_obj_using_psoc(psoc);
	if (!ser_pdev_obj) {
		serialization_err("Invalid ser_pdev_obj");
		return;
	}
	psoc_ser_obj = wlan_serialization_get_psoc_priv_obj(psoc);
	if (!psoc_ser_obj) {
		serialization_err("Invalid psoc_ser_obj");
		return;
	}
	pdev = wlan_serialization_get_first_pdev(psoc);
	if (!pdev) {
		serialization_err("Invalid pdev");
		return;
	}
	for (i = 0; psoc_ser_obj->max_active_cmds > i; i++) {
		ser_timer = &psoc_ser_obj->timers[i];
		if (!ser_timer->cmd)
			continue;
		if (vdev && (vdev != ser_timer->cmd->vdev))
			continue;
		/*
		 * if request is to purge active scan then don't de-activate
		 * non-scan cmds. If request is to purge all queues then
		 * de-activate all the timers.
		 */
		if (!purge_all_queues && purge_scan_active_queue &&
			     (ser_timer->cmd->cmd_type >= WLAN_SER_CMD_NONSCAN))
			continue;
		/*
		 * if request is to purge active nonscan then don't de-activate
		 * scan cmds. If request is to purge all queues then
		 * de-activate all the timers.
		 */
		if (!purge_all_queues && purge_nonscan_active_queue &&
			     (ser_timer->cmd->cmd_type < WLAN_SER_CMD_NONSCAN))
			continue;

		if (QDF_STATUS_SUCCESS !=
				wlan_serialization_find_and_stop_timer(psoc,
							ser_timer->cmd))
			serialization_err("some error in stopping timer");
	}
	if (purge_all_queues || purge_scan_active_queue) {
		/* sending active queue as pending queue to leverage the API */
		wlan_serialization_remove_all_cmd_from_queue(
			&ser_pdev_obj->active_scan_list, ser_pdev_obj,
			pdev, vdev, NULL, false);
	}
	if (purge_all_queues || purge_scan_pending_queue) {
		wlan_serialization_remove_all_cmd_from_queue(
			&ser_pdev_obj->pending_scan_list, ser_pdev_obj,
			pdev, vdev, NULL, false);
	}
	if (purge_all_queues || purge_nonscan_active_queue) {
		/* sending active queue as pending queue to leverage the API */
		wlan_serialization_remove_all_cmd_from_queue(
			&ser_pdev_obj->active_list, ser_pdev_obj,
			pdev, vdev, NULL, false);
	}
	if (purge_all_queues || purge_nonscan_pending_queue) {
		wlan_serialization_remove_all_cmd_from_queue(
			&ser_pdev_obj->pending_list, ser_pdev_obj,
			pdev, vdev, NULL, false);
	}
	wlan_objmgr_pdev_release_ref(pdev, WLAN_SERIALIZATION_ID);

	return;
}
