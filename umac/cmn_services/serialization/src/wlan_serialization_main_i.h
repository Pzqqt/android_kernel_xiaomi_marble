/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_serialization_main.h
 * This file contains all the prototype definitions necessary for the
 * serialization component's internal functions
 */
#ifndef __WLAN_SERIALIZATION_MAIN_I_H
#define __WLAN_SERIALIZATION_MAIN_I_H

#ifdef CONFIG_SERIALIZATION_V1
/* Include files */
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "qdf_mc_timer.h"

#define WLAN_SERIALIZATION_MAX_GLOBAL_POOL_CMDS 24
#define WLAN_SERIALIZATION_MAX_ACTIVE_CMDS 1
#define WLAN_SERIALIZATION_MAX_ACTIVE_SCAN_CMDS 8

#define serialization_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_SERIALIZATION, params)
#define serialization_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_SERIALIZATION, params)
#define serialization_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_SERIALIZATION, params)
#define serialization_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_SERIALIZATION, params)
#define serialization_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_SERIALIZATION, params)

#define serialization_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_SERIALIZATION, params)
#define serialization_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_SERIALIZATION, params)
#define serialization_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_SERIALIZATION, params)
#define serialization_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_SERIALIZATION, params)
#define serialization_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_SERIALIZATION, params)

#define serialization_enter() \
	QDF_TRACE_ENTER(QDF_MODULE_ID_SERIALIZATION, "enter")
#define serialization_exit() \
	QDF_TRACE_EXIT(QDF_MODULE_ID_SERIALIZATION, "exit")

/**
 * struct serialization_legacy_callback - to handle legacy serialization cb
 *
 * @serialization_purge_cmd_list: function ptr to be filled by serialization
 *				  module
 *
 * Some of the legacy modules wants to call API to purge the commands in
 * order to handle backward compatibility.
 */
struct serialization_legacy_callback {
	void (*serialization_purge_cmd_list) (struct wlan_objmgr_psoc *,
		struct wlan_objmgr_vdev *, bool, bool, bool, bool, bool);
};
#else /*New serialization code*/
/* Include files */
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <qdf_mc_timer.h>

#define WLAN_SER_MAX_VDEVS 17

#define WLAN_SER_MAX_ACTIVE_CMDS WLAN_SER_MAX_VDEVS
#define WLAN_SER_MAX_PENDING_CMDS (WLAN_SER_MAX_VDEVS * 4)

#define WLAN_SER_MAX_ACTIVE_SCAN_CMDS 8
#define WLAN_SER_MAX_PENDING_SCAN_CMDS 24

#define WLAN_SERIALIZATION_MAX_GLOBAL_POOL_CMDS \
	(WLAN_SER_MAX_ACTIVE_CMDS + \
	 WLAN_SER_MAX_PENDING_CMDS + \
	 WLAN_SER_MAX_ACTIVE_SCAN_CMDS + \
	 WLAN_SER_MAX_PENDING_SCAN_CMDS)

#define ser_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_SERIALIZATION, params)
#define ser_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_SERIALIZATION, params)
#define ser_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_SERIALIZATION, params)
#define ser_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_SERIALIZATION, params)
#define ser_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_SERIALIZATION, params)
#define ser_enter() \
	QDF_TRACE_ENTER(QDF_MODULE_ID_SERIALIZATION, "enter")
#define ser_exit() \
	QDF_TRACE_EXIT(QDF_MODULE_ID_SERIALIZATION, "exit")

#define ser_err_no_fl(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_SERIALIZATION, params)
/**
 * struct serialization_legacy_callback - to handle legacy serialization cb
 * @serialization_purge_cmd_list: function ptr to be filled by serialization
 *				  module
 *
 * Some of the legacy modules wants to call API to purge the commands in
 * order to handle backward compatibility.
 */
struct serialization_legacy_callback {
	void (*serialization_purge_cmd_list)(struct wlan_objmgr_psoc *,
					     struct wlan_objmgr_vdev *,
					     bool, bool, bool, bool, bool);
};
#endif
#endif
