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

/*
 * DOC: contains Interface MGR init/deinit public api
 */

#ifndef _WLAN_IF_MGR_MAIN_API_H_
#define _WLAN_IF_MGR_MAIN_API_H_

#include <qdf_atomic.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_if_mgr_public_struct.h>

#define ifmgr_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_IFMGR, params)

/* Rate Limited Logs */
#define ifmgr_alert_rl(params...) \
	QDF_TRACE_FATAL_RL(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_err_rl(params...) \
	QDF_TRACE_ERROR_RL(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_warn_rl(params...) \
	QDF_TRACE_WARN_RL(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_info_rl(params...) \
	QDF_TRACE_INFO_RL(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_debug_rl(params...) \
	QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_IFMGR, params)

#define ifmgr_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_IFMGR, params)
#define ifmgr_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_IFMGR, params)

#define ifmgr_hex_dump(level, data, buf_len) \
		qdf_trace_hex_dump(QDF_MODULE_ID_IFMGR, level, data, buf_len)

/*
 * wlan_if_mgr_init() - Interface manager module initialization API
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_if_mgr_init(void);

/**
 * wlan_if_mgr_deinit() - interface manager module deinitialization API
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_if_mgr_deinit(void);

/**
 * wlan_if_mgr_psoc_created_notification() - interface mgr psoc create handler
 * @psoc: psoc object
 * @arg_list: Argument list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_if_mgr_psoc_created_notification(struct wlan_objmgr_psoc *psoc,
						 void *arg_list);

/**
 * wlan_scan_psoc_deleted_notification() - scan psoc delete handler
 * @psoc: psoc object
 * @arg_list: Argument list
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_if_mgr_psoc_destroyed_notification(struct wlan_objmgr_psoc *psoc,
						   void *arg_list);

#endif
