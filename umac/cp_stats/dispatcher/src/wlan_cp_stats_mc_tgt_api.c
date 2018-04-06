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
 * DOC:wlan_cp_stats_mc_tgt_api.c
 *
 * This file provide API definitions to update control plane statistics received
 * from southbound interface
 */

#include "target_if_cp_stats.h"
#include "wlan_cp_stats_mc_defs.h"
#include "wlan_cp_stats_tgt_api.h"
#include "wlan_cp_stats_mc_tgt_api.h"
#include "../../core/src/wlan_cp_stats_defs.h"
#include <wlan_cp_stats_utils_api.h>


QDF_STATUS tgt_mc_cp_stats_inc_wake_lock_stats(struct wlan_objmgr_psoc *psoc,
					       uint32_t reason,
					       struct wake_lock_stats *stats,
					       uint32_t *unspecified_wake_count)
{
	struct wlan_lmac_if_cp_stats_tx_ops *tx_ops;

	tx_ops = target_if_cp_stats_get_tx_ops(psoc);
	if (!tx_ops)
		return QDF_STATUS_E_NULL_VALUE;

	tx_ops->inc_wake_lock_stats(reason, stats, unspecified_wake_count);

	return QDF_STATUS_SUCCESS;
}
