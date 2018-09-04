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
 * DOC: declare internal API related to the mlme component
 */

#ifndef _WLAN_MLME_MAIN_H_
#define _WLAN_MLME_MAIN_H_

#include <wlan_mlme_public_struct.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_cmn.h>

#define mlme_fatal(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_MLME, params)
#define mlme_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_MLME, params)
#define mlme_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_MLME, params)
#define mlme_info(params...) QDF_TRACE_INFO(QDF_MODULE_ID_MLME, params)
#define mlme_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_MLME, params)

/**
 * struct wlan_mlme_psoc_obj -MLME psoc priv object
 * @cfg:     cfg items
 */
struct wlan_mlme_psoc_obj {
	struct wlan_mlme_cfg cfg;
};

/**
 * mlme_psoc_object_created_notification(): mlme psoc create handler
 * @psoc: psoc which is going to created by objmgr
 * @arg: argument for vdev create handler
 *
 * Register this api with objmgr to detect psoc is created
 *
 * Return: QDF_STATUS status in case of success else return error
 */
QDF_STATUS mlme_psoc_object_created_notification(
		struct wlan_objmgr_psoc *psoc, void *arg);

/**
 * mlme_psoc_object_destroyed_notification(): mlme psoc delete handler
 * @psoc: psoc which is going to delete by objmgr
 * @arg: argument for vdev delete handler
 *
 * Register this api with objmgr to detect psoc is deleted
 *
 * Return: QDF_STATUS status in case of success else return error
 */
QDF_STATUS mlme_psoc_object_destroyed_notification(
		struct wlan_objmgr_psoc *psoc, void *arg);

/**
 * mlme_cfg_on_psoc_enable() - Populate MLME structure from CFG and INI
 * @psoc: pointer to the psoc object
 *
 * Populate the MLME CFG structure from CFG and INI values using CFG APIs
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlme_cfg_on_psoc_enable(struct wlan_objmgr_psoc *psoc);

/**
 * mlme_get_psoc_obj() - Get MLME object from psoc
 * @psoc: pointer to the psoc object
 *
 * Get the MLME object pointer from the psoc
 *
 * Return: pointer to MLME object
 */
struct wlan_mlme_psoc_obj *mlme_get_psoc_obj(struct wlan_objmgr_psoc *psoc);

#endif
