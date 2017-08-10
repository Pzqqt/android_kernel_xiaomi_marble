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
 * DOC: wlan_serialization_main.h
 * This file contains all the prototype definitions necessary for the
 * serialization component's internal functions
 */
#ifndef __WLAN_SERIALIZATION_MAIN_I_H
#define __WLAN_SERIALIZATION_MAIN_I_H
/* Include files */
#include "wlan_objmgr_cmn.h"
#include "wlan_objmgr_psoc_obj.h"
#include "wlan_objmgr_pdev_obj.h"
#include "qdf_mc_timer.h"

#define WLAN_SERIALIZATION_MAX_GLOBAL_POOL_CMDS 24
#define WLAN_SERIALIZATION_MAX_ACTIVE_CMDS 1
#define WLAN_SERIALIZATION_MAX_ACTIVE_SCAN_CMDS 8

#define serialization_log(level, args...) \
	QDF_TRACE(QDF_MODULE_ID_SERIALIZATION, level, ## args)
#define serialization_logfl(level, format, args...) \
	serialization_log(level, FL(format), ## args)

#define serialization_alert(format, args...) \
	serialization_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define serialization_err(format, args...) \
	serialization_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define serialization_warn(format, args...) \
	serialization_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define serialization_info(format, args...) \
	serialization_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define serialization_debug(format, args...) \
	serialization_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)
#define serialization_enter() \
	serialization_logfl(QDF_TRACE_LEVEL_DEBUG, "enter")
#define serialization_exit() serialization_logfl(QDF_TRACE_LEVEL_DEBUG, "exit")

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

#endif
