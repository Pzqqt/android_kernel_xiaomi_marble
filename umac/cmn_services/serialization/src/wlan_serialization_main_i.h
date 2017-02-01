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
#include "wlan_serialization_api.h"
#include "wlan_serialization_rules_i.h"
#include "wlan_serialization_utils_i.h"
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
#define serialization_notice(format, args...) \
	serialization_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define serialization_info(format, args...) \
	serialization_logfl(QDF_TRACE_LEVEL_INFO_HIGH, format, ## args)
#define serialization_debug(format, args...) \
	serialization_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)


/**
 * struct wlan_serialization_timer - Timer used for serialization
 * @cmd:      Cmd to which the timer is linked
 * @timer:    Timer associated with the command
 *
 * Timers are allocated statically during init, one each for the
 * maximum active commands permitted in the system. Once a cmd is
 * moved from pending list to active list, the timer is activated
 * and once the cmd is completed, the timer is cancelled. Timer is
 * also cancelled if the command is aborted
 *
 * The timers are maintained per psoc. A timer is associated to
 * unique combination of pdev, cmd_type and cmd_id.
 */
struct wlan_serialization_timer {
	struct wlan_serialization_command *cmd;
	qdf_mc_timer_t timer;
};

/**
 * struct wlan_serialization_command_list - List of commands to be serialized
 * @node: Node identifier in the list
 * @cmd: Command to be serialized
 */
struct wlan_serialization_command_list {
	qdf_list_node_t node;
	struct wlan_serialization_command cmd;
};

/**
 * wlan_serialization_psoc_obj_create_notification() - PSOC obj create callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization and
 * when obj manager gets its turn to create the object, it would notify each
 * component with the corresponding callback registered to inform the
 * completion of the creation of the respective object.
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_serialization_psoc_obj_create_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list);

/**
 * wlan_serialization_psoc_obj_destroy_notification() - PSOC obj delete callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization and
 * when obj manager gets its turn to delete the object, it would notify each
 * component with the corresponding callback registered to inform the
 * completion of the deletion of the respective object.
 *
 * Return: QDF Status
 */
QDF_STATUS  wlan_serialization_psoc_obj_destroy_notification(
		struct wlan_objmgr_psoc *psoc, void *arg_list);

/**
 * wlan_serialization_pdev_obj_create_notification() - PDEV obj create callback
 * @psoc: PDEV object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization and
 * when obj manager gets its turn to create the object, it would notify each
 * component with the corresponding callback registered to inform the
 * completion of the creation of the respective object.
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_serialization_pdev_obj_create_notification(
		struct wlan_objmgr_pdev *pdev, void *arg_list);

/**
 * wlan_serialization_pdev_obj_destroy_notification() - PSOC obj delete callback
 * @pdev: PDEV object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization and
 * when obj manager gets its turn to delete the object, it would notify each
 * component with the corresponding callback registered to inform the
 * completion of the deletion of the respective object.
 *
 * Return: QDF Status
 */
QDF_STATUS wlan_serialization_pdev_obj_destroy_notification(
		struct wlan_objmgr_pdev *pdev, void *arg_list);

#endif
