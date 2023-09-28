/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: Implements the unit test framework for serialization module
 */

#include <qdf_status.h>
#include <qdf_timer.h>
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_serialization_api.h>
#include "wlan_serialization_main_i.h"
#include "wlan_serialization_utf_i.h"

struct wlan_ser_utf_vdev_info ser_utf_vdev[WLAN_SER_UTF_MAX_VDEVS];

struct wlan_ser_utf_data *
wlan_ser_utf_data_alloc(struct wlan_ser_utf_data **ser_data,
			struct wlan_objmgr_vdev *vdev,
			uint8_t cmd_id)
{
	struct wlan_ser_utf_data *data;

	data = qdf_mem_malloc(sizeof(*data));

	if (!data) {
		QDF_ASSERT(0);
		return data;
	}

	data->id = cmd_id;
	WLAN_SER_DATA_STR(data->str, wlan_vdev_get_id(vdev), cmd_id);

	*ser_data = data;
	return data;
}

enum wlan_serialization_status
wlan_ser_utf_add_cmd(struct wlan_serialization_command *cmd)
{
	enum wlan_serialization_status status;
	struct wlan_ser_utf_data *data = cmd->umac_cmd;

	cmd->cmd_timeout_duration = WLAN_SER_UTF_TEST_CMD_TIMEOUT_MS;
	cmd->source = WLAN_UMAC_COMP_SERIALIZATION;
	cmd->cmd_cb = wlan_ser_utf_cb;

	status = wlan_serialization_request(cmd);
	ser_debug("ADD : cmd_type:%d %9s %s status: %s",
		  cmd->cmd_type, SER_UTF_BLOCK_STR(cmd->is_blocking), data->str,
		  wlan_serialization_status_strings[status]);

	return status;
}

enum wlan_serialization_status
wlan_ser_utf_add_scan_cmd(struct wlan_objmgr_vdev *vdev,
			  uint32_t cmd_id, void *umac_cmd,
			  bool is_high_priority)
{
	struct wlan_serialization_command cmd;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.umac_cmd = umac_cmd;
	cmd.is_high_priority = is_high_priority;
	cmd.is_blocking = false;
	cmd.cmd_type = WLAN_SER_CMD_SCAN;

	return wlan_ser_utf_add_cmd(&cmd);
}

enum wlan_serialization_status
wlan_ser_utf_add_nonscan_cmd(struct wlan_objmgr_vdev *vdev,
			     uint32_t cmd_id, void *umac_cmd,
			     bool is_high_priority, bool is_blocking)
{
	struct wlan_serialization_command cmd;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.umac_cmd = umac_cmd;
	cmd.is_blocking = is_blocking;
	cmd.is_high_priority = is_high_priority;
	cmd.cmd_type = WLAN_SER_CMD_NONSCAN;

	return wlan_ser_utf_add_cmd(&cmd);
}

void wlan_ser_utf_remove_scan_cmd(struct wlan_objmgr_vdev *vdev,
				  uint32_t cmd_id)
{
	struct wlan_serialization_queued_cmd_info cmd;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.cmd_type = WLAN_SER_CMD_SCAN;
	cmd.requestor = WLAN_UMAC_COMP_SERIALIZATION;

	wlan_serialization_remove_cmd(&cmd);
}

void wlan_ser_utf_remove_nonscan_cmd(struct wlan_objmgr_vdev *vdev,
				     uint32_t cmd_id)
{
	struct wlan_serialization_queued_cmd_info cmd;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.cmd_type = WLAN_SER_CMD_NONSCAN;
	cmd.requestor = WLAN_UMAC_COMP_SERIALIZATION;

	wlan_serialization_remove_cmd(&cmd);
}

enum wlan_serialization_cmd_status
wlan_ser_utf_cancel_scan_cmd(struct wlan_objmgr_vdev *vdev,
			     uint32_t cmd_id, uint8_t queue_type,
			     enum wlan_serialization_cancel_type req_type)
{
	struct wlan_serialization_queued_cmd_info cmd;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.queue_type = queue_type;
	cmd.req_type = req_type;
	cmd.cmd_type = WLAN_SER_CMD_SCAN;
	cmd.requestor = WLAN_UMAC_COMP_SERIALIZATION;

	return wlan_serialization_cancel_request(&cmd);
}

enum wlan_serialization_cmd_status
wlan_ser_utf_cancel_nonscan_cmd(struct wlan_objmgr_vdev *vdev,
				uint32_t cmd_id, uint8_t queue_type,
				enum wlan_serialization_cancel_type req_type)
{
	struct wlan_serialization_queued_cmd_info cmd;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.queue_type = queue_type;
	cmd.req_type = req_type;
	cmd.cmd_type = WLAN_SER_CMD_NONSCAN;
	cmd.requestor = WLAN_UMAC_COMP_SERIALIZATION;

	return wlan_serialization_cancel_request(&cmd);
}

void wlan_ser_utf_remove_start_bss_cmd(struct wlan_objmgr_vdev *vdev,
				       uint32_t cmd_id)
{
	struct wlan_serialization_queued_cmd_info cmd;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.cmd_type = WLAN_SER_CMD_VDEV_START_BSS;
	cmd.requestor = WLAN_UMAC_COMP_SERIALIZATION;

	wlan_serialization_remove_cmd(&cmd);
}

void wlan_ser_utf_remove_stop_bss_cmd(struct wlan_objmgr_vdev *vdev,
				      uint32_t cmd_id)
{
	struct wlan_serialization_queued_cmd_info cmd;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.cmd_type = WLAN_SER_CMD_VDEV_STOP_BSS;
	cmd.requestor = WLAN_UMAC_COMP_SERIALIZATION;

	wlan_serialization_remove_cmd(&cmd);
}

enum wlan_serialization_cmd_status
wlan_ser_utf_cancel_start_bss_cmd(struct wlan_objmgr_vdev *vdev,
				  uint32_t cmd_id, uint8_t queue_type,
				  enum wlan_serialization_cancel_type req_type)
{
	struct wlan_serialization_queued_cmd_info cmd;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.queue_type = queue_type;
	cmd.req_type = req_type;
	cmd.cmd_type = WLAN_SER_CMD_VDEV_START_BSS;
	cmd.requestor = WLAN_UMAC_COMP_SERIALIZATION;

	return wlan_serialization_cancel_request(&cmd);
}

enum wlan_serialization_cmd_status
wlan_ser_utf_cancel_stop_bss_cmd(struct wlan_objmgr_vdev *vdev,
				 uint32_t cmd_id, uint8_t queue_type,
				 enum wlan_serialization_cancel_type req_type)
{
	struct wlan_serialization_queued_cmd_info cmd;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.queue_type = queue_type;
	cmd.req_type = req_type;
	cmd.cmd_type = WLAN_SER_CMD_VDEV_STOP_BSS;
	cmd.requestor = WLAN_UMAC_COMP_SERIALIZATION;

	return wlan_serialization_cancel_request(&cmd);
}

enum wlan_serialization_status
wlan_ser_utf_add_vdev_stop_bss_cmd(struct wlan_objmgr_vdev *vdev,
				   uint32_t cmd_id, void *umac_cmd,
				   bool is_high_priority, bool is_blocking)
{
	struct wlan_serialization_command cmd;
	uint8_t queue_type;
	enum wlan_serialization_cancel_type req_type;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.umac_cmd = umac_cmd;
	cmd.is_blocking = is_blocking;
	cmd.is_high_priority = is_high_priority;
	cmd.cmd_type = WLAN_SER_CMD_VDEV_STOP_BSS;

	/* Command filtering logic */
	req_type = WLAN_SER_CANCEL_NON_SCAN_CMD;
	queue_type = WLAN_SERIALIZATION_PENDING_QUEUE;
	wlan_ser_utf_cancel_start_bss_cmd(vdev, cmd_id,
					  queue_type, req_type);

	wlan_ser_utf_cancel_stop_bss_cmd(vdev, cmd_id,
					 queue_type, req_type);

	if (wlan_serialization_is_cmd_present_in_active_queue(NULL, &cmd))
		return WLAN_SER_CMD_ACTIVE;

	return wlan_ser_utf_add_cmd(&cmd);
}

enum wlan_serialization_status
wlan_ser_utf_add_vdev_start_bss_cmd(struct wlan_objmgr_vdev *vdev,
				    uint32_t cmd_id, void *umac_cmd,
				    bool is_high_priority, bool is_blocking)
{
	struct wlan_serialization_command cmd;
	uint8_t queue_type;
	enum wlan_serialization_cancel_type req_type;
	struct wlan_ser_utf_data *data;

	cmd.vdev = vdev;
	cmd.cmd_id = cmd_id;
	cmd.umac_cmd = umac_cmd;
	cmd.is_blocking = is_blocking;
	cmd.is_high_priority = is_high_priority;
	cmd.cmd_type = WLAN_SER_CMD_VDEV_START_BSS;

	/* Command filtering logic */
	req_type = WLAN_SER_CANCEL_NON_SCAN_CMD;
	queue_type = WLAN_SERIALIZATION_PENDING_QUEUE;
	wlan_ser_utf_cancel_start_bss_cmd(vdev, cmd_id,
					  queue_type, req_type);

	if (wlan_serialization_is_cmd_present_in_active_queue(NULL, &cmd)) {
		cmd.cmd_type = WLAN_SER_CMD_VDEV_STOP_BSS;
		if (!wlan_serialization_is_cmd_present_in_pending_queue(
					NULL, &cmd)) {
		if (!wlan_ser_utf_data_alloc(&data, vdev, cmd_id))
			return WLAN_SER_CMD_DENIED_UNSPECIFIED;
			wlan_ser_utf_add_vdev_stop_bss_cmd(
					vdev, cmd_id, (void *)data,
					is_high_priority, is_blocking);
		}
	}

	cmd.cmd_type = WLAN_SER_CMD_VDEV_START_BSS;
	return wlan_ser_utf_add_cmd(&cmd);
}

QDF_STATUS wlan_ser_utf_cb(struct wlan_serialization_command *cmd,
			   enum wlan_serialization_cb_reason reason)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	uint8_t vdev_id;
	struct wlan_ser_utf_data *data;

	if (!cmd) {
		ser_err("Error: reason:%d", reason);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	vdev_id = wlan_vdev_get_id(cmd->vdev);
	data = cmd->umac_cmd;
	if (!data) {
		ser_err("Error: reason:%d", reason);
		QDF_ASSERT(0);
		return QDF_STATUS_E_NULL_VALUE;
	}

	switch (reason) {
	case WLAN_SER_CB_ACTIVATE_CMD:
		ser_debug("ACTIVATE: cmd_type:%d %9s %s\n", cmd->cmd_type,
			  SER_UTF_BLOCK_STR(cmd->is_blocking), data->str);
		break;

	case WLAN_SER_CB_CANCEL_CMD:
		ser_debug("CANCEL  : cmd_type:%d %9s %s", cmd->cmd_type,
			  SER_UTF_BLOCK_STR(cmd->is_blocking), data->str);
		break;

	case WLAN_SER_CB_ACTIVE_CMD_TIMEOUT:
		ser_debug("TIMEOUT : cmd_type:%d %9s %s", cmd->cmd_type,
			  SER_UTF_BLOCK_STR(cmd->is_blocking), data->str);
		qdf_mem_free(data);
		QDF_ASSERT(0);
		status = QDF_STATUS_E_INVAL;
		break;

	case WLAN_SER_CB_RELEASE_MEM_CMD:
		ser_debug("RELEASE : cmd_type:%d %9s %s", cmd->cmd_type,
			  SER_UTF_BLOCK_STR(cmd->is_blocking), data->str);
		qdf_mem_free(data);
		break;

	default:
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

static void wlan_ser_utf_scan_timer_cb(void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)arg;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);

	wlan_ser_utf_remove_scan_cmd(vdev, ser_utf_vdev[vdev_id].ser_count++);
}

static void wlan_ser_utf_nonscan_timer_cb(void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)arg;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);

	wlan_ser_utf_remove_nonscan_cmd(vdev,
					ser_utf_vdev[vdev_id].ser_count++);
}

void wlan_ser_utf_run(struct wlan_objmgr_vdev *vdev, uint8_t scan_cmd,
		      uint8_t max_cmds,
		      bool is_high_priority, bool is_blocking)
{
	struct wlan_ser_utf_data *data;
	uint8_t id;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	enum wlan_serialization_status ret;

	if (!max_cmds)
		max_cmds = WLAN_SER_UTF_SCAN_CMD_TESTS;

	ser_utf_vdev[vdev_id].ser_count = 0;
	for (id = 0; id < max_cmds; id++) {
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			return;

		if (scan_cmd)
			ret = wlan_ser_utf_add_scan_cmd(vdev, id, data,
							is_high_priority);
		else
			ret = wlan_ser_utf_add_nonscan_cmd(vdev, id,
							   data,
							   is_high_priority,
							   is_blocking);
	}

	for (id = 0; id < max_cmds; id++) {
		if (scan_cmd)
			qdf_timer_mod(
			&ser_utf_vdev[vdev_id].utf_scan_timer[id],
			WLAN_SER_UTF_TIMER_TIMEOUT_MS);
		else
			qdf_timer_mod(
			&ser_utf_vdev[vdev_id].utf_nonscan_timer[id],
			WLAN_SER_UTF_TIMER_TIMEOUT_MS);
	}
}

static void wlan_ser_utf_init_iter_op(struct wlan_objmgr_pdev *pdev,
				      void *obj, void *args)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)obj;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	uint8_t id;

	ser_utf_vdev[vdev_id].vdev = vdev;
	for (id = 0; id < WLAN_SER_UTF_SCAN_CMD_TESTS; id++) {
		qdf_timer_init(NULL,
			       &ser_utf_vdev[vdev_id].utf_scan_timer[id],
			       wlan_ser_utf_scan_timer_cb,
			       (void *)vdev, QDF_TIMER_TYPE_WAKE_APPS);
		qdf_timer_init(NULL,
			       &ser_utf_vdev[vdev_id].utf_nonscan_timer[id],
			       wlan_ser_utf_nonscan_timer_cb,
			       (void *)vdev, QDF_TIMER_TYPE_WAKE_APPS);
	}
}

static void wlan_ser_utf_deinit_iter_op(struct wlan_objmgr_pdev *pdev,
					void *obj, void *args)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)obj;
	uint8_t vdev_id = wlan_vdev_get_id(vdev);
	uint8_t id;

	for (id = 0; id < WLAN_SER_UTF_SCAN_CMD_TESTS; id++) {
		qdf_timer_free(
			&ser_utf_vdev[vdev_id].utf_nonscan_timer[id]);
		qdf_timer_free(
			&ser_utf_vdev[vdev_id].utf_scan_timer[id]);
	}
}

static void wlan_ser_utf_vdev_iter_op(struct wlan_objmgr_pdev *pdev,
				      void *obj, void *args)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)obj;
	uint8_t is_blocking = *(uint8_t *)args;

	wlan_ser_utf_run(vdev, false, 2, false, is_blocking);
}

/*
 * List of available APIs
 * 1. wlan_serialization_request(
 *	struct wlan_serialization_command *cmd)
 * 2. wlan_serialization_remove_cmd(
 *	struct wlan_serialization_queued_cmd_info *cmd_info)
 * 3. wlan_serialization_cancel_request(
 *	struct wlan_serialization_queued_cmd_info *cmd_info)
 *	sub_val:
 *		1st byte : cmd_id
 *		2nd byte : scan_cmd
 *		3rd byte : queue_type
 *		4th byte : req_type
 */
int wlan_ser_utf_main(struct wlan_objmgr_vdev *vdev, uint8_t val,
		      uint32_t sub_val)
{
	uint8_t id;
	uint8_t vdev_id;
	static uint8_t wlan_ser_utf_init;
	struct wlan_ser_utf_data *data;
	bool is_blocking;
	struct wlan_objmgr_pdev *pdev = wlan_vdev_get_pdev(vdev);
	uint8_t cmd_id = (uint8_t)sub_val;
	uint8_t scan_cmd = (uint8_t)(sub_val >> 8);
	uint8_t queue_type = (uint8_t)(sub_val >> 16);
	enum wlan_serialization_cancel_type req_type = (uint8_t)(sub_val >> 24);

	if (wlan_ser_utf_init == 0 && val != 1) {
		ser_err("Init UTF before running test cases");
		return 0;
	}

	switch (val) {
	case SER_UTF_TC_DEINIT:
		if (wlan_objmgr_pdev_try_get_ref(pdev, WLAN_SERIALIZATION_ID) ==
				QDF_STATUS_SUCCESS) {
			wlan_objmgr_pdev_iterate_obj_list(
					pdev, WLAN_VDEV_OP,
					wlan_ser_utf_deinit_iter_op,
					NULL, 0, WLAN_SERIALIZATION_ID);
			wlan_objmgr_pdev_release_ref(pdev,
						     WLAN_SERIALIZATION_ID);
			ser_err("Serialization Timer Deinit Done");
		}
		break;
	case SER_UTF_TC_INIT:
		if (wlan_objmgr_pdev_try_get_ref(pdev, WLAN_SERIALIZATION_ID) ==
				QDF_STATUS_SUCCESS) {
			wlan_objmgr_pdev_iterate_obj_list(
					pdev, WLAN_VDEV_OP,
					wlan_ser_utf_init_iter_op,
					NULL, 0, WLAN_SERIALIZATION_ID);
			wlan_objmgr_pdev_release_ref(pdev,
						     WLAN_SERIALIZATION_ID);
			wlan_ser_utf_init = 1;
			ser_err("Serialization Timer Init Done");
		}
		break;
	case SER_UTF_TC_ADD:
		ser_err("Add:%s, id:%d", scan_cmd ? "SCAN" : "NONSCAN", cmd_id);
		if (!wlan_ser_utf_data_alloc(&data, vdev, cmd_id))
			break;

		if (scan_cmd)
			wlan_ser_utf_add_scan_cmd(vdev, cmd_id, data, false);
		else
			wlan_ser_utf_add_nonscan_cmd(vdev, cmd_id, data,
						     false, false);
		break;
	case SER_UTF_TC_REMOVE:
		ser_err("Remove:%s, id:%d", scan_cmd ? "SCAN" : "NONSCAN",
			cmd_id);
		if (scan_cmd)
			wlan_ser_utf_remove_scan_cmd(vdev, cmd_id);
		else
			wlan_ser_utf_remove_nonscan_cmd(vdev, cmd_id);
		break;
	case SER_UTF_TC_CANCEL:
		ser_err("Cancel:%s, id:%d", scan_cmd ? "SCAN" : "NONSCAN",
			cmd_id);
		if (scan_cmd)
			wlan_ser_utf_cancel_scan_cmd(vdev, cmd_id, queue_type,
						     req_type);
		else
			wlan_ser_utf_cancel_nonscan_cmd(vdev, cmd_id,
							queue_type, req_type);
		break;
	case SER_UTF_TC_SINGLE_SCAN:
		id = 1;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_scan_cmd(vdev, id, data, false);
		wlan_ser_utf_remove_scan_cmd(vdev, id);
		break;
	case SER_UTF_TC_MULTI_SCAN:
		wlan_ser_utf_run(vdev, true, 10, false, false);
		break;
	case SER_UTF_TC_MAX_SCAN:
		wlan_ser_utf_run(vdev, true, 0, false, false);
		break;
	case SER_UTF_TC_SINGLE_NONSCAN:
		id = 1;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);
		wlan_ser_utf_remove_nonscan_cmd(vdev, id);
		break;
	case SER_UTF_TC_MULTI_NONSCAN:
		wlan_ser_utf_run(vdev, false, 10, false, false);
		break;
	case SER_UTF_TC_MAX_NONSCAN:
		wlan_ser_utf_run(vdev, false, 0, false, false);
		break;
	case SER_UTF_TC_MULTI_VDEV_NONSCAN:
		is_blocking = false;
		if (wlan_objmgr_pdev_try_get_ref(pdev, WLAN_SERIALIZATION_ID) ==
				QDF_STATUS_SUCCESS) {
			wlan_objmgr_pdev_iterate_obj_list(
					pdev, WLAN_VDEV_OP,
					wlan_ser_utf_vdev_iter_op,
					&is_blocking, 0, WLAN_SERIALIZATION_ID);
			wlan_objmgr_pdev_release_ref(pdev,
						     WLAN_SERIALIZATION_ID);
		}
		break;
	case SER_UTF_TC_CANCEL_SCAN_AC_SINGLE:
		id = 1;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_scan_cmd(vdev, id, data, false);
		req_type = WLAN_SER_CANCEL_SINGLE_SCAN;
		queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;
		wlan_ser_utf_cancel_scan_cmd(vdev, id, queue_type,
					     req_type);
		break;
	case SER_UTF_TC_CANCEL_SCAN_AC_PDEV:
		wlan_ser_utf_run(vdev, true, 15, false, false);
		req_type = WLAN_SER_CANCEL_PDEV_SCANS;
		queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;
		wlan_ser_utf_cancel_scan_cmd(vdev, cmd_id, queue_type,
					     req_type);
		break;
	case SER_UTF_TC_CANCEL_SCAN_AC_VDEV:
		wlan_ser_utf_run(vdev, true, 15, false, false);
		req_type = WLAN_SER_CANCEL_VDEV_SCANS;
		queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;
		wlan_ser_utf_cancel_scan_cmd(vdev, cmd_id, queue_type,
					     req_type);
		break;
	case SER_UTF_TC_CANCEL_SCAN_PD_SINGLE:
		wlan_ser_utf_run(vdev, true, 15, false, false);
		req_type = WLAN_SER_CANCEL_SINGLE_SCAN;
		queue_type = WLAN_SERIALIZATION_PENDING_QUEUE;
		wlan_ser_utf_cancel_scan_cmd(vdev, cmd_id, queue_type,
					     req_type);
		break;
	case SER_UTF_TC_CANCEL_SCAN_PD_PDEV:
		wlan_ser_utf_run(vdev, true, 15, false, false);
		req_type = WLAN_SER_CANCEL_PDEV_SCANS;
		queue_type = WLAN_SERIALIZATION_PENDING_QUEUE;
		wlan_ser_utf_cancel_scan_cmd(vdev, cmd_id, queue_type,
					     req_type);
		break;
	case SER_UTF_TC_CANCEL_SCAN_PD_VDEV:
		wlan_ser_utf_run(vdev, true, 15, false, false);
		req_type = WLAN_SER_CANCEL_VDEV_SCANS;
		queue_type = WLAN_SERIALIZATION_PENDING_QUEUE;
		wlan_ser_utf_cancel_scan_cmd(vdev, cmd_id, queue_type,
					     req_type);
		break;
	case SER_UTF_TC_CANCEL_NONSCAN_AC_SINGLE:
		req_type = WLAN_SER_CANCEL_NON_SCAN_CMD;
		queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;
		wlan_ser_utf_cancel_nonscan_cmd(vdev, cmd_id, queue_type,
						req_type);
		break;
	case SER_UTF_TC_CANCEL_NONSCAN_AC_PDEV:
		req_type = WLAN_SER_CANCEL_PDEV_NON_SCAN_CMD;
		queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;
		wlan_ser_utf_cancel_nonscan_cmd(vdev, cmd_id, queue_type,
						req_type);
		break;
	case SER_UTF_TC_CANCEL_NONSCAN_AC_VDEV:
		req_type = WLAN_SER_CANCEL_VDEV_NON_SCAN_CMD;
		queue_type = WLAN_SERIALIZATION_ACTIVE_QUEUE;
		wlan_ser_utf_cancel_nonscan_cmd(vdev, cmd_id, queue_type,
						req_type);
		break;
	case SER_UTF_TC_CANCEL_NONSCAN_PD_SINGLE:
		req_type = WLAN_SER_CANCEL_NON_SCAN_CMD;
		queue_type = WLAN_SERIALIZATION_PENDING_QUEUE;
		wlan_ser_utf_cancel_nonscan_cmd(vdev, cmd_id, queue_type,
						req_type);
		break;
	case SER_UTF_TC_CANCEL_NONSCAN_PD_PDEV:
		req_type = WLAN_SER_CANCEL_PDEV_NON_SCAN_CMD;
		queue_type = WLAN_SERIALIZATION_PENDING_QUEUE;
		wlan_ser_utf_cancel_nonscan_cmd(vdev, cmd_id, queue_type,
						req_type);
		break;
	case SER_UTF_TC_CANCEL_NONSCAN_PD_VDEV:
		req_type = WLAN_SER_CANCEL_VDEV_NON_SCAN_CMD;
		queue_type = WLAN_SERIALIZATION_PENDING_QUEUE;
		wlan_ser_utf_cancel_nonscan_cmd(vdev, cmd_id, queue_type,
						req_type);
		break;
	case SER_UTF_TC_START_BSS_FILTERING:
		id = 1;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_vdev_start_bss_cmd(vdev, id, data,
						    false, false);

		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_vdev_start_bss_cmd(vdev, id, data,
						    false, false);

		wlan_ser_utf_remove_start_bss_cmd(vdev, id);
		wlan_ser_utf_remove_stop_bss_cmd(vdev, id);
		wlan_ser_utf_remove_start_bss_cmd(vdev, id);
		break;
	case SER_UTF_TC_STOP_BSS_FILTERING:
		id = 1;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_vdev_start_bss_cmd(vdev, id, data,
						    false, false);

		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_vdev_start_bss_cmd(vdev, id, data,
						    false, false);

		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_vdev_stop_bss_cmd(vdev, id, data,
						   false, false);

		wlan_ser_utf_remove_start_bss_cmd(vdev, id);
		wlan_ser_utf_remove_stop_bss_cmd(vdev, id);
		break;
	case SER_UTF_TC_ADD_BLOCKING_NONSCAN_AC_1:
		id = 1;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, true);
		wlan_ser_utf_remove_nonscan_cmd(vdev, id);
		break;
	case SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_1:
		id = 1;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, true);

		id = 2;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, true);

		wlan_ser_utf_remove_nonscan_cmd(vdev, 1);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 2);
		break;
	case SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_2:
		id = 1;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 2;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, true);

		wlan_ser_utf_remove_nonscan_cmd(vdev, 1);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 2);
		break;
	case SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_3:
		id = 1;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 2;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 3;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 4;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, true);

		wlan_ser_utf_remove_nonscan_cmd(vdev, 1);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 2);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 3);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 4);
		break;
	case SER_UTF_TC_ADD_BLOCKING_NONSCAN_PD_4:
		id = 1;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 2;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 3;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 4;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, true);

		id = 5;
		if (!wlan_ser_utf_data_alloc(&data, vdev, id))
			break;
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		wlan_ser_utf_remove_nonscan_cmd(vdev, 1);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 2);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 3);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 4);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 5);
		break;
	case SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_1:
		if (wlan_pdev_get_vdev_count(pdev) < WLAN_SER_UTF_MAX_VDEVS) {
			ser_err("Requires atleast %d vdevs for the given pdev",
				WLAN_SER_UTF_MAX_VDEVS);
			break;
		}
		is_blocking = true;
		if (wlan_objmgr_pdev_try_get_ref(pdev, WLAN_SERIALIZATION_ID) ==
				QDF_STATUS_SUCCESS) {
			wlan_objmgr_pdev_iterate_obj_list(
					pdev, WLAN_VDEV_OP,
					wlan_ser_utf_vdev_iter_op,
					&is_blocking, 0, WLAN_SERIALIZATION_ID);
			wlan_objmgr_pdev_release_ref(pdev,
						     WLAN_SERIALIZATION_ID);
		}
		break;
	case SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_2:
		if (wlan_pdev_get_vdev_count(pdev) < WLAN_SER_UTF_MAX_VDEVS) {
			ser_err("Requires atleast %d vdevs for the given pdev",
				WLAN_SER_UTF_MAX_VDEVS);
			break;
		}
		id = 1;
		wlan_ser_utf_data_alloc(&data, ser_utf_vdev[0].vdev, id);
		wlan_ser_utf_add_nonscan_cmd(ser_utf_vdev[0].vdev, id,
					     data, false, false);

		wlan_ser_utf_data_alloc(&data, ser_utf_vdev[1].vdev, id);
		wlan_ser_utf_add_nonscan_cmd(ser_utf_vdev[1].vdev, id,
					     data, false, false);

		wlan_ser_utf_data_alloc(&data, ser_utf_vdev[2].vdev, id);
		wlan_ser_utf_add_nonscan_cmd(ser_utf_vdev[2].vdev, id,
					     data, false, false);

		wlan_ser_utf_data_alloc(&data, ser_utf_vdev[2].vdev, id);
		wlan_ser_utf_add_nonscan_cmd(ser_utf_vdev[2].vdev, id,
					     data, false, true);

		wlan_ser_utf_remove_nonscan_cmd(ser_utf_vdev[0].vdev, id);
		wlan_ser_utf_remove_nonscan_cmd(ser_utf_vdev[1].vdev, id);
		wlan_ser_utf_remove_nonscan_cmd(ser_utf_vdev[2].vdev, id);
		wlan_ser_utf_remove_nonscan_cmd(ser_utf_vdev[2].vdev, id);
		break;
	case SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_3:
		if (wlan_pdev_get_vdev_count(pdev) < WLAN_SER_UTF_MAX_VDEVS) {
			ser_err("Requires atleast %d vdevs for the given pdev",
				WLAN_SER_UTF_MAX_VDEVS);
			break;
		}
		id = 1;
		wlan_ser_utf_data_alloc(&data, ser_utf_vdev[0].vdev, id);
		wlan_ser_utf_add_nonscan_cmd(ser_utf_vdev[0].vdev, id,
					     data, false, true);

		wlan_ser_utf_data_alloc(&data, ser_utf_vdev[0].vdev, id);
		wlan_ser_utf_add_nonscan_cmd(ser_utf_vdev[0].vdev, id,
					     data, false, false);

		wlan_ser_utf_data_alloc(&data, ser_utf_vdev[1].vdev, id);
		wlan_ser_utf_add_nonscan_cmd(ser_utf_vdev[1].vdev, id,
					     data, false, false);

		wlan_ser_utf_data_alloc(&data, ser_utf_vdev[2].vdev, id);
		wlan_ser_utf_add_nonscan_cmd(ser_utf_vdev[2].vdev, id,
					     data, false, false);

		wlan_ser_utf_remove_nonscan_cmd(ser_utf_vdev[0].vdev, id);
		wlan_ser_utf_remove_nonscan_cmd(ser_utf_vdev[0].vdev, id);
		wlan_ser_utf_remove_nonscan_cmd(ser_utf_vdev[1].vdev, id);
		wlan_ser_utf_remove_nonscan_cmd(ser_utf_vdev[2].vdev, id);
		break;
	case SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_4:
		if (wlan_pdev_get_vdev_count(pdev) < WLAN_SER_UTF_MAX_VDEVS) {
			ser_err("Requires atleast %d vdevs for the given pdev",
				WLAN_SER_UTF_MAX_VDEVS);
			break;
		}
		for (id = 1; id <= 2; id++) {
			for (vdev_id = 0; vdev_id < WLAN_SER_UTF_MAX_VDEVS;
					vdev_id++) {
				wlan_ser_utf_data_alloc(
						&data,
						ser_utf_vdev[vdev_id].vdev, id);
				wlan_ser_utf_add_nonscan_cmd(
						ser_utf_vdev[vdev_id].vdev,
						id, data, false, false);
			}
		}

		id = 3;
		for (vdev_id = 0; vdev_id < WLAN_SER_UTF_MAX_VDEVS; vdev_id++) {
			wlan_ser_utf_data_alloc(
					&data, ser_utf_vdev[vdev_id].vdev, id);
			wlan_ser_utf_add_nonscan_cmd(
					ser_utf_vdev[vdev_id].vdev, id,
					data, false, true);
		}

		for (id = 1; id <= 3; id++) {
			for (vdev_id = 0; vdev_id < WLAN_SER_UTF_MAX_VDEVS;
					vdev_id++)
				wlan_ser_utf_remove_nonscan_cmd(
						ser_utf_vdev[vdev_id].vdev, id);
		}
		break;
	case SER_UTF_TC_MULTI_VDEV_BL_NONSCAN_5:
		if (wlan_pdev_get_vdev_count(pdev) < WLAN_SER_UTF_MAX_VDEVS) {
			ser_err("Requires atleast %d vdevs for the given pdev",
				WLAN_SER_UTF_MAX_VDEVS);
			break;
		}
		id = 1;
		for (vdev_id = 0; vdev_id < WLAN_SER_UTF_MAX_VDEVS;
					vdev_id++) {
			wlan_ser_utf_data_alloc(
					&data,
					ser_utf_vdev[vdev_id].vdev, id);
			wlan_ser_utf_add_nonscan_cmd(
					ser_utf_vdev[vdev_id].vdev,
					id, data, false, false);
		}
		id = 2;
		for (vdev_id = 0; vdev_id < WLAN_SER_UTF_MAX_VDEVS;
					vdev_id++) {
			wlan_ser_utf_data_alloc(
					&data,
					ser_utf_vdev[vdev_id].vdev, id);
			wlan_ser_utf_add_nonscan_cmd(
					ser_utf_vdev[vdev_id].vdev,
					id, data, false, true);
		}
		id = 3;
		for (vdev_id = 0; vdev_id < WLAN_SER_UTF_MAX_VDEVS;
					vdev_id++) {
			wlan_ser_utf_data_alloc(
					&data,
					ser_utf_vdev[vdev_id].vdev, id);
			wlan_ser_utf_add_nonscan_cmd(
					ser_utf_vdev[vdev_id].vdev,
					id, data, false, false);
		}

		for (id = 1; id <= 3; id++) {
			for (vdev_id = 0; vdev_id < WLAN_SER_UTF_MAX_VDEVS;
					vdev_id++)
				wlan_ser_utf_remove_nonscan_cmd(
						ser_utf_vdev[vdev_id].vdev, id);
		}
		break;
	case SER_UTF_TC_HIGH_PRIO_NONSCAN_WO_BL:
		id = 1;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 2;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 3;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 4;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, true, false);

		wlan_ser_utf_remove_nonscan_cmd(vdev, 1);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 4);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 2);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 3);
		break;
	case SER_UTF_TC_HIGH_PRIO_NONSCAN_W_BL:
		id = 1;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 2;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, true);

		id = 3;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, true, false);

		id = 4;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		wlan_ser_utf_remove_nonscan_cmd(vdev, 1);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 3);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 2);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 4);
		break;
	case SER_UTF_TC_HIGH_PRIO_BL_NONSCAN:
		id = 1;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 2;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 3;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, false, false);

		id = 4;
		wlan_ser_utf_data_alloc(&data, vdev, id);
		wlan_ser_utf_add_nonscan_cmd(vdev, id, data, true, true);

		wlan_ser_utf_remove_nonscan_cmd(vdev, 1);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 4);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 2);
		wlan_ser_utf_remove_nonscan_cmd(vdev, 3);
		break;
	default:
		ser_err("Error: Unknown val");
		break;
	}

	return 0;
}
