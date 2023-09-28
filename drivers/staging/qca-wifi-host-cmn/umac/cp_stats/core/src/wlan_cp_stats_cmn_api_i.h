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
 * DOC: wlan_cp_stats_cmn_api_i.h
 *
 * This header filed declare APIs which have separate definition for both mc
 * and ic
 */
#ifndef __WLAN_CP_STATS_CMN_API_I_H__
#define __WLAN_CP_STATS_CMN_API_I_H__
#ifdef QCA_SUPPORT_CP_STATS
#include "wlan_cp_stats_defs.h"

/**
 * wlan_cp_stats_psoc_cs_init() - common psoc obj initialization
 * @psoc: pointer to psoc object
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_psoc_cs_init(struct psoc_cp_stats *psoc_cs);

/**
 * wlan_cp_stats_psoc_cs_deinit() - common psoc obj deinitialization
 * @psoc: pointer to psoc object
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_psoc_cs_deinit(struct psoc_cp_stats *psoc_cs);

/**
 * wlan_cp_stats_pdev_cs_init() - common pdev obj initialization
 * @pdev: pointer to pdev object
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_pdev_cs_init(struct pdev_cp_stats *pdev_cs);

/**
 * wlan_cp_stats_pdev_cs_deinit() - common pdev obj deinitialization
 * @pdev: pointer to pdev object
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_pdev_cs_deinit(struct pdev_cp_stats *pdev_cs);

/**
 * wlan_cp_stats_vdev_cs_init() - common vdev obj initialization
 * @vdev: pointer to vdev object
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_vdev_cs_init(struct vdev_cp_stats *vdev_cs);

/**
 * wlan_cp_stats_vdev_cs_deinit() - common vdev obj deinitialization
 * @vdev: pointer to vdev object
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_vdev_cs_deinit(struct vdev_cp_stats *vdev_cs);

/**
 * wlan_cp_stats_peer_cs_init() - common peer obj initialization
 * @peer: pointer to peer object
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_peer_cs_init(struct peer_cp_stats *peer_cs);

/**
 * wlan_cp_stats_peer_cs_deinit() - common peer obj deinitialization
 * @peer: pointer to peer object
 *
 * Return: QDF_STATUS - Success or Failure
 */
QDF_STATUS wlan_cp_stats_peer_cs_deinit(struct peer_cp_stats *peer_cs);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_CMN_API_I_H__ */
