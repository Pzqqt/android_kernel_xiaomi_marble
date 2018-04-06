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
 * DOC: wlan_cp_stats_mc_ucfg_api.h
 *
 * This header file maintain API declaration required for northbound interaction
 */

#ifndef __WLAN_CP_STATS_MC_UCFG_API_H__
#define __WLAN_CP_STATS_MC_UCFG_API_H__

#ifdef QCA_SUPPORT_CP_STATS

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_cp_stats_mc_defs.h>

struct psoc_cp_stats;
struct vdev_cp_stats;

/**
 * ucfg_mc_cp_stats_get_psoc_wake_lock_stats() : API to get wake lock stats from
 * psoc
 * @psoc: pointer to psoc object
 * @stats: stats object to populate
 *
 * Return : status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_get_psoc_wake_lock_stats(
						struct wlan_objmgr_psoc *psoc,
						struct wake_lock_stats *stats);

/**
 * ucfg_mc_cp_stats_get_vdev_wake_lock_stats() : API to get wake lock stats from
 * vdev
 * @vdev: pointer to vdev object
 * @stats: stats object to populate
 *
 * Return : status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_get_vdev_wake_lock_stats(
						struct wlan_objmgr_vdev *vdev,
						struct wake_lock_stats *stats);

/**
 * ucfg_mc_cp_stats_inc_wake_lock_stats_by_protocol() : API to increment wake
 * lock stats given the protocol of the packet that was received.
 * @psoc: pointer to psoc object
 * @vdev_id: vdev_id for which the packet was received
 * @protocol: protocol of the packet that was received
 *
 * Return : status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_inc_wake_lock_stats_by_protocol(
					struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id,
					enum qdf_proto_subtype protocol);

/**
 * ucfg_mc_cp_stats_inc_wake_lock_stats_by_protocol() : API to increment wake
 * lock stats given destnation of packet that was received.
 * @psoc: pointer to psoc object
 * @dest_mac: destinamtion mac address of packet that was received
 *
 * Return : status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_inc_wake_lock_stats_by_dst_addr(
					struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id, uint8_t *dest_mac);

/**
 * ucfg_mc_cp_stats_inc_wake_lock_stats() : API to increment wake lock stats
 * given wake reason.
 * @psoc: pointer to psoc object
 * @vdev_id: vdev_id on with WOW was received
 * @reason: reason of WOW
 *
 * Return : status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_inc_wake_lock_stats(struct wlan_objmgr_psoc *psoc,
						uint8_t vdev_id,
						uint32_t reason);

/**
 * ucfg_mc_cp_stats_write_wow_stats() - Writes WOW stats to buffer
 * @psoc: pointer to psoc object
 * @buffer: The char buffer to write to
 * @max_len: The maximum number of chars to write
 * @ret: number of bytes written
 *
 * Return - status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_write_wow_stats(
				struct wlan_objmgr_psoc *psoc,
				char *buffer, uint16_t max_len, int *ret);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_UCFG_API_H__ */
