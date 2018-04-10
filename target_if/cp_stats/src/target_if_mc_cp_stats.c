/*
 * Copyright (c) 2013-2018 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: target_if_cp_stats.c
 *
 * This file provide definition for APIs registered through lmac Tx Ops
 */

#include <qdf_mem.h>
#include <qdf_status.h>
#include <target_if_cp_stats.h>
#include <wmi_unified_priv.h>
#include <wmi_unified_param.h>
#include <target_if.h>
#include <wlan_tgt_def_config.h>
#include <wmi_unified_api.h>
#include <wlan_osif_priv.h>
#include "wlan_cp_stats_utils_api.h"

static void target_if_cp_stats_inc_wake_lock_stats(uint32_t reason,
					struct wake_lock_stats *stats,
					uint32_t *unspecified_wake_count)
{
	switch (reason) {
	case WOW_REASON_UNSPECIFIED:
		(*unspecified_wake_count)++;
		break;
	case WOW_REASON_RA_MATCH:
		stats->ipv6_mcast_wake_up_count++;
		stats->ipv6_mcast_ra_stats++;
		stats->icmpv6_count++;
		break;
	case WOW_REASON_NLOD:
		stats->pno_match_wake_up_count++;
		break;
	case WOW_REASON_NLO_SCAN_COMPLETE:
		stats->pno_complete_wake_up_count++;
		break;
	case WOW_REASON_LOW_RSSI:
		stats->low_rssi_wake_up_count++;
		break;
	case WOW_REASON_EXTSCAN:
		stats->gscan_wake_up_count++;
		break;
	case WOW_REASON_RSSI_BREACH_EVENT:
		stats->rssi_breach_wake_up_count++;
		break;
	case WOW_REASON_OEM_RESPONSE_EVENT:
		stats->oem_response_wake_up_count++;
	case WOW_REASON_11D_SCAN:
		stats->scan_11d++;
		break;
	case WOW_REASON_CHIP_POWER_FAILURE_DETECT:
		stats->pwr_save_fail_detected++;
		break;
	default:
		break;
	}
}

static QDF_STATUS
target_if_cp_stats_register_event_handler(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		cp_stats_err("PSOC is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS
target_if_cp_stats_unregister_event_handler(struct wlan_objmgr_psoc *psoc)
{
	if (!psoc) {
		cp_stats_err("PSOC is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
target_if_cp_stats_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	struct wlan_lmac_if_cp_stats_tx_ops *cp_stats_tx_ops;

	if (!tx_ops) {
		cp_stats_err("lmac tx ops is NULL!");
		return QDF_STATUS_E_INVAL;
	}

	cp_stats_tx_ops = &tx_ops->cp_stats_tx_ops;
	if (!cp_stats_tx_ops) {
		cp_stats_err("lmac tx ops is NULL!");
		return QDF_STATUS_E_FAILURE;
	}

	cp_stats_tx_ops->cp_stats_attach =
		target_if_cp_stats_register_event_handler;
	cp_stats_tx_ops->cp_stats_detach =
		target_if_cp_stats_unregister_event_handler;
	cp_stats_tx_ops->inc_wake_lock_stats =
		target_if_cp_stats_inc_wake_lock_stats;

	return QDF_STATUS_SUCCESS;
}

