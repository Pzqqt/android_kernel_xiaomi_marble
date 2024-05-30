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
 * DOC: wlan_cp_stats_ol_api.h
 *
 * This header file provide API declarations required for cp stats global
 * context specific to offload
 */

#ifndef __WLAN_CP_STATS_OL_API_H__
#define __WLAN_CP_STATS_OL_API_H__

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_objmgr_cmn.h>
#include "wlan_cp_stats_defs.h"
#include "wlan_cp_stats_ol_api.h"

/**
 * wlan_cp_stats_psoc_obj_init_ol() - private API to init psoc cp stats obj
 * @psoc_cs: pointer to psoc cp stat object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_psoc_obj_init_ol(struct psoc_cp_stats *psoc_cs);

/**
 * wlan_cp_stats_psoc_obj_deinit_ol() - private API to deinit psoc cp stats obj
 * @psoc_cs: pointer to psoc cp stat object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_psoc_obj_deinit_ol(struct psoc_cp_stats *psoc_cs);

/**
 * wlan_cp_stats_pdev_obj_init_ol() - private API to init pdev cp stats obj
 * @pdev_cs: pointer to pdev cp stat object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_pdev_obj_init_ol(struct pdev_cp_stats *pdev_cs);

/**
 * wlan_cp_stats_pdev_obj_deinit_ol() - private API to deinit pdev cp stats obj
 * @pdev_cs: pointer to pdev cp stat object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_pdev_obj_deinit_ol(struct pdev_cp_stats *pdev_cs);

/**
 * wlan_cp_stats_vdev_obj_init_ol() - private API to init vdev cp stats obj
 * @vdev_cs: pointer to vdev cp stat object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_vdev_obj_init_ol(struct vdev_cp_stats *vdev_cs);

/**
 * wlan_cp_stats_vdev_obj_deinit_ol() - private API to deinit vdev cp stats obj
 * @vdev_cs: pointer to vdev cp stat object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_vdev_obj_deinit_ol(struct vdev_cp_stats *vdev_cs);

/**
 * wlan_cp_stats_peer_obj_init_ol() - private API to init peer cp stats obj
 * @peer_cs: pointer to peer cp stat object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_peer_obj_init_ol(struct peer_cp_stats *peer_cs);

/**
 * wlan_cp_stats_peer_obj_deinit_ol() - private API to deinit peer cp stats obj
 * @peer_cs: pointer to peer cp stat object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_peer_obj_deinit_ol(struct peer_cp_stats *peer_cs);

/**
 * wlan_cp_stats_open_ol() - private API for psoc open
 * @psoc: pointer to psoc object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_open_ol(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cp_stats_close_ol() - private API for psoc close
 * @psoc: pointer to psoc object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_close_ol(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cp_stats_enable_ol() - private API for psoc enable
 * @psoc: pointer to psoc enable
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_enable_ol(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cp_stats_disable_ol() - private API for psoc disable
 * @psoc: pointer to psoc enable
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_disable_ol(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_cp_stats_ctx_init_ol() - private API to initialize cp stat global ctx
 * @csc: pointer to cp stats global context object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_ctx_init_ol(struct cp_stats_context *csc);

/**
 * wlan_cp_stats_ctx_deinit_ol() - private API to deinit cp stat global ctx
 * @csc: pointer to cp stats global context object
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_cp_stats_ctx_deinit_ol(struct cp_stats_context *csc);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_OL_API_H__ */
