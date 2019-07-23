/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * Return: status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_write_wow_stats(
				struct wlan_objmgr_psoc *psoc,
				char *buffer, uint16_t max_len, int *ret);

/**
 * ucfg_mc_cp_stats_send_tx_power_request() - API to send tx_power request to
 * lmac
 * @vdev: pointer to vdev object
 * @type: request type
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_send_stats_request(struct wlan_objmgr_vdev *vdev,
					       enum stats_req_type type,
					       struct request_info *info);

/**
 * ucfg_mc_cp_stats_get_tx_power() - API to fetch tx_power
 * @vdev: pointer to vdev object
 * @dbm: pointer to tx power in dbm
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_get_tx_power(struct wlan_objmgr_vdev *vdev,
					 int *dbm);

/**
 * ucfg_mc_cp_stats_is_req_pending() - API to tell if given request is pending
 * @psoc: pointer to psoc object
 * @type: request type to check
 *
 * Return: true of request is pending, false otherwise
 */
bool ucfg_mc_cp_stats_is_req_pending(struct wlan_objmgr_psoc *psoc,
				     enum stats_req_type type);

/**
 * ucfg_mc_cp_stats_set_pending_req() - API to set pending request
 * @psoc: pointer to psoc object
 * @type: request to update
 * @req: value to update
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_set_pending_req(struct wlan_objmgr_psoc *psoc,
					    enum stats_req_type type,
					    struct request_info *req);

/**
 * ucfg_mc_cp_stats_reset_pending_req() - API to reset pending request
 * @psoc: pointer to psoc object
 * @type: request to update
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_reset_pending_req(struct wlan_objmgr_psoc *psoc,
					      enum stats_req_type type);

/**
 * ucfg_mc_cp_stats_get_pending_req() - API to get pending request
 * @psoc: pointer to psoc object
 * @type: request to update
 * @info: buffer to populate
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_get_pending_req(struct wlan_objmgr_psoc *psoc,
					    enum stats_req_type type,
					    struct request_info *info);

/**
 * ucfg_mc_cp_stats_free_stats_resources() - API to free buffers within stats_event
 * structure
 * @ev: strcture whose buffer are to freed
 *
 * Return: none
 */
void ucfg_mc_cp_stats_free_stats_resources(struct stats_event *ev);

/**
 * ucfg_mc_cp_stats_cca_stats_get() - API to fetch cca stats
 * @vdev: pointer to vdev object
 * @cca_stats: pointer to cca info
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_cca_stats_get(struct wlan_objmgr_vdev *vdev,
					  struct cca_stats *cca_stats);

/**
 * ucfg_mc_cp_stats_set_rate_flags() - API to set rate flags
 * @vdev: pointer to vdev object
 * @flags: value to set (enum tx_rate_info)
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_mc_cp_stats_set_rate_flags(struct wlan_objmgr_vdev *vdev,
					   enum tx_rate_info flags);

/**
 * ucfg_mc_cp_stats_register_lost_link_info_cb() - API to register lost link
 * info callback
 * @psoc: pointer to psoc object
 * @lost_link_cp_stats_info_cb: Lost link info callback to be registered
 *
 */
void ucfg_mc_cp_stats_register_lost_link_info_cb(
		struct wlan_objmgr_psoc *psoc,
		void (*lost_link_cp_stats_info_cb)(void *stats_ev));

#ifdef WLAN_POWER_MANAGEMENT_OFFLOAD
/**
 * ucfg_mc_cp_stats_register_pmo_handler() - API to register pmo handler
 *
 * Return: none
 */
void ucfg_mc_cp_stats_register_pmo_handler(void);
#else
void static inline ucfg_mc_cp_stats_register_pmo_handler(void) { };
#endif /* WLAN_POWER_MANAGEMENT_OFFLOAD */
#else
void static inline ucfg_mc_cp_stats_register_pmo_handler(void) { };
#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_MC_UCFG_API_H__ */
