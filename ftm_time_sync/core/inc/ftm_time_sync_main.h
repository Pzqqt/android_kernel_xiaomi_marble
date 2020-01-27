/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: Declare private API which shall be used internally only
 * in ftm_time_sync component. This file shall include prototypes of
 * various notification handlers and logging functions.
 *
 * Note: This API should be never accessed out of ftm_time_sync component.
 */

#ifndef _FTM_TIME_SYNC_MAIN_H_
#define _FTM_TIME_SYNC_MAIN_H_

#include <qdf_types.h>
#include "ftm_time_sync_priv.h"
#include "ftm_time_sync_objmgr.h"

#define ftm_time_sync_log(level, args...) \
	QDF_TRACE(QDF_MODULE_ID_FTM_TIME_SYNC, level, ## args)

#define ftm_time_sync_logfl(level, format, args...) \
	ftm_time_sync_log(level, FL(format), ## args)

#define ftm_time_sync_fatal(format, args...) \
		ftm_time_sync_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define ftm_time_sync_err(format, args...) \
		ftm_time_sync_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define ftm_time_sync_warn(format, args...) \
		ftm_time_sync_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define ftm_time_sync_info(format, args...) \
		ftm_time_sync_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define ftm_time_sync_debug(format, args...) \
		ftm_time_sync_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

#define FTM_TIME_SYNC_ENTER() ftm_time_sync_debug("enter")
#define FTM_TIME_SYNC_EXIT() ftm_time_sync_debug("exit")

/**
 * ftm_time_sync_vdev_create_notification(): Handler for vdev create notify.
 * @vdev: vdev which is going to be created by objmgr
 * @arg: argument for notification handler.
 *
 * Allocate and attach vdev private object.
 *
 * Return: QDF_STATUS status in case of success else return error.
 */
QDF_STATUS ftm_timesync_vdev_create_notification(struct wlan_objmgr_vdev *vdev,
						 void *arg);

/**
 * ftm_time_sync_vdev_destroy_notification(): Handler for vdev destroy notify.
 * @vdev: vdev which is going to be destroyed by objmgr
 * @arg: argument for notification handler.
 *
 * Deallocate and detach vdev private object.
 *
 * Return QDF_STATUS status in case of success else return error
 */
QDF_STATUS
ftm_timesync_vdev_destroy_notification(struct wlan_objmgr_vdev *vdev,
				       void *arg);

#endif /* end of _FTM_TIME_SYNC_MAIN_H_ */
