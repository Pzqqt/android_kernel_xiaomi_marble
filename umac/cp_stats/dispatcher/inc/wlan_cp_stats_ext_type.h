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

/**
 * DOC: wlan_cp_stats_ext_types.h
 *
 * This header file maintain structure definitions for cp stats specific to ic
 */

#ifndef __WLAN_CP_STATS_EXT_TYPE_H__
#define __WLAN_CP_STATS_EXT_TYPE_H__

/**
 * typedef psoc_ext_cp_stats_t - Opaque datatype
 */
typedef struct psoc_ext_cp_stats psoc_ext_cp_stats_t;

/**
 * typedef pdev_ext_cp_stats_t - Opaque definition of pdev ic cp stats
 */
struct pdev_ic_cp_stats;
typedef struct pdev_ic_cp_stats pdev_ext_cp_stats_t;

/**
 * typedef vdev_ext_cp_stats_t - Opaque definition of vdev ic cp stats
 */
struct vdev_ic_cp_stats;
typedef struct vdev_ic_cp_stats vdev_ext_cp_stats_t;

/**
 * typedef peer_ext_cp_stats_t - Opaque definition of peer ic cp stats
 */
struct peer_ic_cp_stats;
typedef struct peer_ic_cp_stats peer_ext_cp_stats_t;

#endif /* __WLAN_CP_STATS_EXT_TYPE_H__ */
