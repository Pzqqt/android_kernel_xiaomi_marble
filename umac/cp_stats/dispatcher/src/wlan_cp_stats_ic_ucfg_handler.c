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
 * DOC:wlan_cp_stats_ic_ucfg_handler.c
 *
 * This file provide APIs definition for registering cp stats cfg80211 command
 * handlers
 */
#include "wlan_cp_stats_ic_ucfg_handler.h"
#include <wlan_cfg80211_ic_cp_stats.h>
#include <wlan_cp_stats_ic_ucfg_defs.h>
#include "../../core/src/wlan_cp_stats_cmn_api_i.h"

QDF_STATUS wlan_cp_stats_psoc_cs_init(struct psoc_cp_stats *psoc_cs)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_psoc_cs_deinit(struct psoc_cp_stats *psoc_cs)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_pdev_cs_init(struct pdev_cp_stats *pdev_cs)
{
	wlan_cp_stats_register_pdev_ucfg_handlers(pdev_cs->pdev_obj);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_pdev_cs_deinit(struct pdev_cp_stats *pdev_cs)
{
	wlan_cp_stats_unregister_pdev_ucfg_handlers(pdev_cs->pdev_obj);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_vdev_cs_init(struct vdev_cp_stats *vdev_cs)
{
	wlan_cp_stats_register_vdev_ucfg_handlers(vdev_cs->vdev_obj);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_vdev_cs_deinit(struct vdev_cp_stats *vdev_cs)
{
	wlan_cp_stats_unregister_vdev_ucfg_handlers(vdev_cs->vdev_obj);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_peer_cs_init(struct peer_cp_stats *peer_cs)
{
	wlan_cp_stats_register_peer_ucfg_handlers(peer_cs->peer_obj);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_cp_stats_peer_cs_deinit(struct peer_cp_stats *peer_cs)
{
	wlan_cp_stats_unregister_peer_ucfg_handlers(peer_cs->peer_obj);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_register_pdev_ucfg_handlers(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_unregister_pdev_ucfg_handlers(struct wlan_objmgr_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_register_vdev_ucfg_handlers(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_unregister_vdev_ucfg_handlers(struct wlan_objmgr_vdev *vdev)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_register_peer_ucfg_handlers(struct wlan_objmgr_peer *peer)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_cp_stats_unregister_peer_ucfg_handlers(struct wlan_objmgr_peer *peer)
{
	return QDF_STATUS_SUCCESS;
}
