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
 * DOC: wlan_cp_stats_ic_ucfg_handler.h
 *
 * This header file provide APIs declaration required for (de)register ucfg
 * handlers, these APIs are invoked from architecture specific (de)init
 * APIs when corresponding common object specific obj mgt event notification is
 * triggered
 */

#ifndef __WLAN_CP_STATS_IC_UCFG_HANDLER_H__
#define __WLAN_CP_STATS_IC_UCFG_HANDLER_H__

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_objmgr_cmn.h>
#include <wlan_cp_stats_ucfg_api.h>

/**
 * wlan_cp_stats_register_pdev_ucfg_handlers() : API to register callbacks
 * for commands at pdev object
 * @pdev: reference to pdev global object
 *
 * This API is invoked from pdev create handler and used to register callbacks
 * required by ucfg layer
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
wlan_cp_stats_register_pdev_ucfg_handlers(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_cp_stats_register_vdev_ucfg_handlers() : API to register callbacks
 * for commands at vdev object
 * @vdev: reference to vdev global object
 *
 * This API is invoked from vdev create handler and used to register callbacks
 * required by ucfg layer
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
wlan_cp_stats_register_vdev_ucfg_handlers(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cp_stats_register_peer_ucfg_handlers() : API to register callbacks
 * for commands at peer object
 * @peer: reference to peer global object
 *
 * This API is invoked from peer create handler and used to register callbacks
 * required by ucfg layer
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
wlan_cp_stats_register_peer_ucfg_handlers(struct wlan_objmgr_peer *peer);

/**
 * wlan_cp_stats_unregister_pdev_ucfg_handlers() : API to unregister callbacks
 * for commands at pdev object
 * @pdev: reference to pdev global object
 *
 * This API is invoked from pdev create handler and used to unregister callbacks
 * required by ucfg layer
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
wlan_cp_stats_unregister_pdev_ucfg_handlers(struct wlan_objmgr_pdev *pdev);

/**
 * wlan_cp_stats_unregister_vdev_ucfg_handlers() : API to unregister callbacks
 * for commands at vdev object
 * @vdev: reference to vdev global object
 *
 * This API is invoked from vdev create handler and used to unregister callbacks
 * required by ucfg layer
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
wlan_cp_stats_unregister_vdev_ucfg_handlers(struct wlan_objmgr_vdev *vdev);

/**
 * wlan_cp_stats_unregister_peer_ucfg_handlers() : API to unregister callbacks
 * for commands at peer object
 * @peer: reference to peer global object
 *
 * This API is invoked from peer create handler and used to unregister callbacks
 * required by ucfg layer
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS
wlan_cp_stats_unregister_peer_ucfg_handlers(struct wlan_objmgr_peer *peer);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_IC_UCFG_HANDLER_H__ */
