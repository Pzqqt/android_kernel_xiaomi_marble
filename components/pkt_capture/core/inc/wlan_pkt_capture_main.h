/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: Declare private API which shall be used internally only
 * in pkt_capture component. This file shall include prototypes of
 * various notification handlers and logging functions.
 *
 * Note: This API should be never accessed out of pkt_capture component.
 */

#ifndef _WLAN_PKT_CAPTURE_MAIN_H_
#define _WLAN_PKT_CAPTURE_MAIN_H_

#include <qdf_types.h>
#include "wlan_pkt_capture_priv.h"
#include "wlan_pkt_capture_objmgr.h"

#define pkt_capture_log(level, args...) \
	QDF_TRACE(QDF_MODULE_ID_PKT_CAPTURE, level, ## args)

#define pkt_capture_logfl(level, format, args...) \
	pkt_capture_log(level, FL(format), ## args)

#define pkt_capture_fatal(format, args...) \
		pkt_capture_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define pkt_capture_err(format, args...) \
		pkt_capture_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define pkt_capture_warn(format, args...) \
		pkt_capture_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define pkt_capture_info(format, args...) \
		pkt_capture_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define pkt_capture_debug(format, args...) \
		pkt_capture_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

#define PKT_CAPTURE_ENTER() pkt_capture_debug("enter")
#define PKT_CAPTURE_EXIT() pkt_capture_debug("exit")

/**
 * pkt_capture_vdev_create_notification() - Handler for vdev create notify.
 * @vdev: vdev which is going to be created by objmgr
 * @arg: argument for notification handler.
 *
 * Allocate and attach vdev private object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
pkt_capture_vdev_create_notification(struct wlan_objmgr_vdev *vdev, void *arg);

/**
 * pkt_capture_vdev_destroy_notification() - Handler for vdev destroy notify.
 * @vdev: vdev which is going to be destroyed by objmgr
 * @arg: argument for notification handler.
 *
 * Deallocate and detach vdev private object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
pkt_capture_vdev_destroy_notification(struct wlan_objmgr_vdev *vdev, void *arg);

/**
 * pkt_capture_get_mode() - get packet capture mode
 * @psoc: pointer to psoc object
 *
 * Return: enum pkt_capture_mode
 */
enum pkt_capture_mode pkt_capture_get_mode(struct wlan_objmgr_psoc *psoc);

/**
 * pkt_capture_psoc_create_notification() - Handler for psoc create notify.
 * @psoc: psoc which is going to be created by objmgr
 * @arg: argument for notification handler.
 *
 * Allocate and attach psoc private object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
pkt_capture_psoc_create_notification(struct wlan_objmgr_psoc *psoc, void *arg);

/**
 * pkt_capture_psoc_destroy_notification() - Handler for psoc destroy notify.
 * @psoc: psoc which is going to be destroyed by objmgr
 * @arg: argument for notification handler.
 *
 * Deallocate and detach psoc private object.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
pkt_capture_psoc_destroy_notification(struct wlan_objmgr_psoc *psoc, void *arg);
#endif /* end of _WLAN_PKT_CAPTURE_MAIN_H_ */
