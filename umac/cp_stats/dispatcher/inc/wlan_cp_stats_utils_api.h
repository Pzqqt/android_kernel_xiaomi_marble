/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_cp_stats_utils_api.h
 *
 * This header file provide declaration to public APIs exposed for other UMAC
 * components to init/deinit, (de)register to required WMI events on
 * soc enable/disable
 */

#ifndef __WLAN_CP_STATS_UTILS_API_H__
#define __WLAN_CP_STATS_UTILS_API_H__

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_objmgr_cmn.h>

#define cp_stats_debug(args ...) \
		QDF_TRACE_DEBUG(QDF_MODULE_ID_CP_STATS, ## args)
#define cp_stats_err(args ...) \
		QDF_TRACE_ERROR(QDF_MODULE_ID_CP_STATS, ## args)

/**
 * enum wlan_cp_stats_cfg_state - State of Object configuration to
 * indicate whether object has to be attached/detached in cp stats
 * @WLAN_CP_STATS_OBJ_DETACH: Object has to be detached
 * @WLAN_CP_STATS_OBJ_ATTACH: Object has to be attached
 * @WLAN_CP_STATS_OBJ_INVALID: Object is invalid
 */
enum wlan_cp_stats_cfg_state {
	WLAN_CP_STATS_OBJ_DETACH = 0,
	WLAN_CP_STATS_OBJ_ATTACH = 1,
	WLAN_CP_STATS_OBJ_INVALID
};

/**
 * enum wlan_cp_stats_comp_id - component id for other umac components
 * @WLAN_CP_STATS_ATF: ATF component specific id
 * @WLAN_CP_STATS_MAX_COMPONENTS : Max id of cp stats components
 */
enum wlan_cp_stats_comp_id {
	WLAN_CP_STATS_ATF = 0,
	WLAN_CP_STATS_MAX_COMPONENTS,
};

/**
 * wlan_cp_stats_init(): API to init stats component
 *
 * This API is invoked from dispatcher init during all component init.
 * This API will register all required handlers for psoc, pdev,vdev
 * and peer object create/delete notification.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_init(void);

/**
 * wlan_cp_stats_init(): API to deinit stats component
 *
 * This API is invoked from dispatcher deinit during all component deinit.
 * This API will unregister all required handlers for psoc, pdev,vdev
 * and peer object create/delete notification.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_deinit(void);

/**
 * wlan_cp_stats_open(): API to open cp stats component
 * @psoc: pointer to psoc
 *
 * This API is invoked from dispatcher psoc open.
 * This API will initialize psoc level cp stats object.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_open(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cp_stats_close(): API to close cp stats component
 * @psoc: pointer to psoc
 *
 * This API is invoked from dispatcher psoc close.
 * This API will de-initialize psoc level cp stats object.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_close(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cp_stats_enable(): API to enable cp stats component
 * @psoc: pointer to psoc
 *
 * This API is invoked from dispatcher psoc enable.
 * This API will register cp_stats WMI event handlers.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_enable(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cp_stats_disable(): API to disable cp stats component
 * @psoc: pointer to psoc
 *
 * This API is invoked from dispatcher psoc disable.
 * This API will unregister cp_stats WMI event handlers.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_disable(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cp_stats_comp_obj_cfg() - public API to umac for
 * attach/detach
 * component specific stat obj to cp stats obj
 * @obj_type: common object type
 * @cfg_state: config state either to attach of detach
 * @comp_id: umac component id
 * @cmn_obj: pointer to common object
 * @comp_priv_obj: pointer to component specific cp stats object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_comp_obj_cfg(
		enum wlan_objmgr_obj_type obj_type,
		enum wlan_cp_stats_cfg_state cfg_state,
		enum wlan_umac_comp_id comp_id,
		void *cmn_obj,
		void *data);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_UTILS_API_H__ */
