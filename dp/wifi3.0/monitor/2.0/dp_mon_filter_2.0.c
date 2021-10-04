/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <hal_api.h>
#include <wlan_cfg.h>
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_htt.h"
#include "dp_mon.h"
#include "dp_mon_filter.h"
#include <dp_mon_2.0.h>
#include <dp_rx_mon_2.0.h>
#include <dp_mon_filter_2.0.h>

#ifdef QCA_ENHANCED_STATS_SUPPORT
void dp_mon_filter_setup_enhanced_stats_2_0(struct dp_pdev *pdev)
{
}

void dp_mon_filter_reset_enhanced_stats_2_0(struct dp_pdev *pdev)
{
}
#endif /* QCA_ENHANCED_STATS_SUPPORT */

#ifdef QCA_MCOPY_SUPPORT
void dp_mon_filter_setup_mcopy_mode_2_0(struct dp_pdev *pdev)
{
}

void dp_mon_filter_reset_mcopy_mode_2_0(struct dp_pdev *pdev)
{
}
#endif

#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
void dp_mon_filter_setup_smart_monitor_2_0(struct dp_pdev *pdev)
{
}

void dp_mon_filter_reset_smart_monitor_2_0(struct dp_pdev *pdev)
{
}
#endif /* ATH_SUPPORT_NAC_RSSI || ATH_SUPPORT_NAC */

#ifdef WLAN_RX_PKT_CAPTURE_ENH
void dp_mon_filter_setup_rx_enh_capture_2_0(struct dp_pdev *pdev)
{
}

void dp_mon_filter_reset_rx_enh_capture_2_0(struct dp_pdev *pdev)
{
}
#endif /* WLAN_RX_PKT_CAPTURE_ENH */

void dp_mon_filter_setup_mon_mode_2_0(struct dp_pdev *pdev)
{
}

void dp_mon_filter_reset_mon_mode_2_0(struct dp_pdev *pdev)
{
}

#ifdef WDI_EVENT_ENABLE
void dp_mon_filter_setup_rx_pkt_log_full_2_0(struct dp_pdev *pdev)
{
}

void dp_mon_filter_reset_rx_pkt_log_full_2_0(struct dp_pdev *pdev)
{
}

void dp_mon_filter_setup_rx_pkt_log_lite_2_0(struct dp_pdev *pdev)
{
}

void dp_mon_filter_reset_rx_pkt_log_lite_2_0(struct dp_pdev *pdev)
{
}

void dp_mon_filter_setup_rx_pkt_log_cbf_2_0(struct dp_pdev *pdev)
{
}

void dp_mon_filter_reset_rx_pktlog_cbf_2_0(struct dp_pdev *pdev)
{
}
#endif /* WDI_EVENT_ENABLE */

QDF_STATUS dp_mon_filter_update_2_0(struct dp_pdev *pdev)
{
	return status;
}
