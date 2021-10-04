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

#include <dp_types.h>
#include "dp_rx.h"
#include "dp_peer.h"
#include <dp_htt.h>
#include <dp_mon_filter.h>
#include <dp_mon.h>
#include <dp_rx_mon.h>
#include <dp_rx_mon_2.0.h>
#include <dp_mon_2.0.h>
#include <dp_mon_filter_2.0.h>

#include "htt_ppdu_stats.h"
#include "dp_cal_client_api.h"
#if defined(DP_CON_MON)
#ifndef REMOVE_PKT_LOG
#include <pktlog_ac_api.h>
#include <pktlog_ac.h>
#endif
#endif
#ifdef FEATURE_PERPKT_INFO
#include "dp_ratetable.h"
#endif

void
dp_rx_mon_buf_desc_pool_deinit(struct dp_soc *soc)
{
}

void
dp_rx_mon_buf_desc_pool_init(struct dp_soc *soc)
{
}

void dp_rx_mon_buf_desc_pool_free(struct dp_soc *soc)
{
}

QDF_STATUS
dp_rx_mon_buf_desc_pool_alloc(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

void
dp_rx_mon_buffers_free(struct dp_soc *soc)
{
}

QDF_STATUS
dp_rx_mon_buffers_alloc(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}
