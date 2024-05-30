/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
 *
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
 * DOC: declare the ftm service data structure and apis
 */
#ifndef _WLAN_FTM_SVC_H_
#define _WLAN_FTM_SVC_H_

#include <qdf_types.h>
#include <qdf_status.h>
#include <wlan_objmgr_pdev_obj.h>

/**
 * struct ftm_seg_hdr_info - the segment header for the event from FW
 * @len: length of the segment header
 * @msgref: message reference
 * @segment_info: segment information
 * @pad: padding
 *
 */
struct ftm_seg_hdr_info {
	uint32_t len;
	uint32_t msgref;
	uint32_t segment_info;
	uint32_t pad;
};

/**
 * wlan_ftm_pdev_obj_create_notification() - ftm pdev create handler
 * @pdev: pdev pointer
 * @arg_list: argument list
 *
 * return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wlan_ftm_pdev_obj_create_notification(struct wlan_objmgr_pdev *pdev,
							void *arg_list);

/**
 * wlan_ftm_pdev_obj_destroy_notification() - ftm pdev destroy handler
 * @pdev: pdev pointer
 * @arg_list: argument list
 *
 * return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wlan_ftm_pdev_obj_destroy_notification(struct wlan_objmgr_pdev *pdev,
							void *arg_list);

/**
 * wlan_ftm_cmd_send() - send ftm command to target_if layer
 * @pdev: pdev pointer
 * @buf: data buffer
 * @len: event length
 *
 * return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wlan_ftm_cmd_send(struct wlan_objmgr_pdev *pdev, uint8_t *buf,
				uint32_t len, uint8_t pdev_id);

/**
 * wlan_ftm_testmode_attach() - Attach FTM UTF handle
 * @psoc: psoc pointer
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_ftm_testmode_attach(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_ftm_testmode_detach() - Attach FTM UTF handle
 * @psoc: psoc pointer
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_ftm_testmode_detach(struct wlan_objmgr_psoc *psoc);
#endif /* _WLAN_FTM_SVC_H_ */
