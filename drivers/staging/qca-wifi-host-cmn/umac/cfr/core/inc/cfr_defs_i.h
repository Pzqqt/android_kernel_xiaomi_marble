/*
 * Copyright (c) 2019-2021 The Linux Foundation. All rights reserved.
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

#define CFR_STOP_STR           "CFR-CAPTURE-STOPPED"

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

/**
 * cfr_streamfs_init() - stream filesystem init
 * @pdev - pointer to pdev object
 *
 * Return: status of fs init
 */
QDF_STATUS
cfr_streamfs_init(struct wlan_objmgr_pdev *pdev);

/**
 * cfr_streamfs_remove() - stream filesystem remove
 * @pdev - pointer to pdev object
 *
 * Return: status of fs remove
 */
QDF_STATUS
cfr_streamfs_remove(struct wlan_objmgr_pdev *pdev);

/**
 * cfr_streamfs_write() - write to stream filesystem
 * @pa - pointer to pdev_cfr object
 * @write_data - Pointer to data
 * @write_len - data len
 *
 * Return: status of fs write
 */
QDF_STATUS
cfr_streamfs_write(struct pdev_cfr *pa, const void *write_data,
		   size_t write_len);

/**
 * cfr_streamfs_flush() - flush the write to streamfs
 * @pa - pointer to pdev_cfr object
 *
 * Return: status of fs flush
 */
QDF_STATUS
cfr_streamfs_flush(struct pdev_cfr *pa);

/**
 * cfr_stop_indication() - write cfr stop string
 * @vdev - pointer to vdev object
 *
 * Write stop string and indicate to up layer.
 *
 * Return: status of write CFR stop string
 */
QDF_STATUS cfr_stop_indication(struct wlan_objmgr_vdev *vdev);

#ifdef WLAN_CFR_PM
/**
 * cfr_prevent_suspend() - Acquire wake lock and prevent suspend
 * @pcfr - pointer to pdev_cfr object
 *
 * Return: QDF status
 */
QDF_STATUS cfr_prevent_suspend(struct pdev_cfr *pcfr);

/**
 * cfr_allow_suspend() - Release wake lock and allow suspend
 * @pcfr - pointer to pdev_cfr object
 *
 * Return: QDF status
 */
QDF_STATUS cfr_allow_suspend(struct pdev_cfr *pcfr);
#endif
#endif
