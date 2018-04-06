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
 * DOC: wlan_cp_stats_mc_tgt_api.h
 *
 * This header file provide with API declarations to interface with Southbound
 */
#ifndef __WLAN_CP_STATS_MC_TGT_API_H__
#define __WLAN_CP_STATS_MC_TGT_API_H__

#ifdef QCA_SUPPORT_CP_STATS
#include "wlan_cp_stats_mc_defs.h"

/**
 * tgt_mc_cp_stats_process_stats_event(): API to process stats event
 * @psoc: pointer to psoc object
 * @event: event parameters
 *
 * Return: status of operation
 */
QDF_STATUS tgt_mc_cp_stats_process_stats_event(struct wlan_objmgr_psoc *psoc,
					       struct stats_event *event);

/**
 * tgt_send_mc_cp_stats_req(): API to send stats request to lmac
 * @psoc: pointer to psoc object
 *
 * Return: status of operation
 */
QDF_STATUS tgt_send_mc_cp_stats_req(struct wlan_objmgr_psoc *psoc,
				    enum stats_req_type type,
				    struct request_info *req);

/**
 * tgt_mc_cp_stats_inc_wake_lock_stats() : API to increment wake lock stats
 * given the wake reason code
 * @psoc: pointer to psoc object
 * @reason: wake reason
 * @stats: vdev wow stats to update
 * @unspecified_wake_count: unspecified wake count to update
 *
 * Return : status of operation
 */
QDF_STATUS tgt_mc_cp_stats_inc_wake_lock_stats(struct wlan_objmgr_psoc *psoc,
				uint32_t reason, struct wake_lock_stats *stats,
				uint32_t *unspecified_wake_count);

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_MC_TGT_API_H__ */
