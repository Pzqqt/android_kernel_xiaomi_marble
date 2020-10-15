/*
 * Copyright (c) 2012-2015, 2020 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: osif_cm_rsp.h
 *
 * This header file maintains declarations of connect, disconnect, roam
 * response apis.
 */

#ifndef __OSIF_CM_RSP_H
#define __OSIF_CM_RSP_H

#ifdef FEATURE_CM_ENABLE
#include "wlan_objmgr_vdev_obj.h"
#include "wlan_cm_public_struct.h"

/**
 * osif_disconnect_handler() - Indicate disconnnect to userspace
 * @vdev: vdev pointer
 * @rsp: Disconnect response from connection manager
 *
 * This function indicates disconnect to the kernel which thus indicates
 * to the userspace.
 *
 * Context: Any context
 * Return: QDF_STATUS_SUCCESS on successful indication to kernel,
 * else QDF_STATUS with failure reason
 */
QDF_STATUS osif_disconnect_handler(struct wlan_objmgr_vdev *vdev,
				   struct wlan_cm_discon_rsp *rsp);

/**
 * osif_connect_handler() - API to send connect response to kernel
 * @vdev: vdev pointer
 * @rsp: Connection manager connect response
 *
 * The API is used to send connection response to kernel
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
QDF_STATUS osif_connect_handler(struct wlan_objmgr_vdev *vdev,
				struct wlan_cm_connect_rsp *rsp);

/**
 * osif_failed_candidate_handler() - API to indicate individual candidate
 * connect failure resp
 * @vdev: vdev pointer
 * @rsp: Connection manager connect response for the candidate
 *
 * The API is is used indicate individual candidate connect failure resp for
 * for tried all but last tried candidate. The last candidate will be sent in
 * osif_connect_handler.
 *
 * Context: Any context.
 * Return: QDF_STATUS
 */
QDF_STATUS osif_failed_candidate_handler(struct wlan_objmgr_vdev *vdev,
					 struct wlan_cm_connect_rsp *rsp);

#endif /* FEATURE_CM_ENABLE */
#endif /* __OSIF_CM_RSP_H */
