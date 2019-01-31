/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#ifndef _CFR_DEFS_I_H_
#define _CFR_DEFS_I_H_

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include <qdf_list.h>
#include <qdf_timer.h>
#include <qdf_util.h>
#include <qdf_types.h>
#include <wlan_cfr_utils_api.h>

#define cfr_log(level, args...) \
QDF_TRACE(QDF_MODULE_ID_CFR, level, ##args)

#define cfr_log_fl(level, format, args...) cfr_log(level, FL(format), ## args)

#define cfr_err(format, args...) \
	cfr_log_fl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define cfr_info(format, args...) \
	cfr_log_fl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define cfr_debug(format, args...) \
	cfr_log_fl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

/**
 * wlan_cfr_psoc_obj_create_handler() - psoc object create handler for cfr
 * @psoc - pointer to psoc object
 * @args - void pointer in case it needs arguments
 *
 * Return: status of object creation
 */
QDF_STATUS
wlan_cfr_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc, void *arg);

/**
 * wlan_cfr_psoc_obj_destroy_handler() - psoc object destroy handler for cfr
 * @psoc - pointer to psoc object
 * @args - void pointer in case it needs arguments
 *
 * Return: status of destroy object
 */
QDF_STATUS
wlan_cfr_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc, void *arg);

/**
 * wlan_cfr_pdev_obj_create_handler() - pdev object create handler for cfr
 * @pdev - pointer to pdev object
 * @args - void pointer in case it needs arguments
 *
 * Return: status of object creation
 */
QDF_STATUS
wlan_cfr_pdev_obj_create_handler(struct wlan_objmgr_pdev *pdev, void *arg);

/**
 * wlan_cfr_pdev_obj_destroy_handler() - pdev object destroy handler for cfr
 * @pdev - pointer to pdev object
 * @args - void pointer in case it needs arguments
 *
 * Return: status of destroy object
 */
QDF_STATUS
wlan_cfr_pdev_obj_destroy_handler(struct wlan_objmgr_pdev *pdev, void *arg);

/**
 * wlan_cfr_peer_obj_create_handler() - peer object create handler for cfr
 * @peer - pointer to peer object
 * @args - void pointer in case it needs arguments
 *
 * Return: status of object creation
 */
QDF_STATUS
wlan_cfr_peer_obj_create_handler(struct wlan_objmgr_peer *peer, void *arg);

/**
 * wlan_cfr_peer_obj_destroy_handler() - peer object destroy handler for cfr
 * @peer - pointer to peer object
 * @args - void pointer in case it needs arguments
 *
 * Return: status ofi destry object
 */
QDF_STATUS
wlan_cfr_peer_obj_destroy_handler(struct wlan_objmgr_peer *peer, void *arg);
#endif
