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

void
dp_rx_mon_buf_desc_pool_deinit(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	dp_mon_desc_pool_deinit(&mon_soc->rx_desc_mon);
}

QDF_STATUS
dp_rx_mon_buf_desc_pool_init(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;
	QDF_STATUS status;

	status = dp_mon_desc_pool_init(&mon_soc->rx_desc_mon);
	if (status != QDF_STATUS_SUCCESS) {
		dp_mon_err("Failed to init rx monior descriptor pool");
		mon_soc->rx_mon_ring_fill_level = 0;
	} else {
		mon_soc->rx_mon_ring_fill_level =
					DP_MON_RING_FILL_LEVEL_DEFAULT;
	}

	return status;
}

void dp_rx_mon_buf_desc_pool_free(struct dp_soc *soc)
{
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	if (mon_soc)
		dp_mon_desc_pool_free(&mon_soc->rx_desc_mon);
}

QDF_STATUS
dp_rx_mon_buf_desc_pool_alloc(struct dp_soc *soc)
{
	struct dp_srng *mon_buf_ring;
	struct dp_mon_desc_pool *rx_mon_desc_pool;
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	mon_buf_ring = &soc->rxdma_mon_buf_ring[0];

	rx_mon_desc_pool = &mon_soc->rx_desc_mon;

	return dp_mon_desc_pool_alloc(mon_soc->rx_mon_ring_fill_level,
				      rx_mon_desc_pool);
}

void
dp_rx_mon_buffers_free(struct dp_soc *soc)
{
	struct dp_mon_desc_pool *rx_mon_desc_pool;
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	rx_mon_desc_pool = &mon_soc->rx_desc_mon;

	dp_mon_pool_frag_unmap_and_free(soc, rx_mon_desc_pool);
}

QDF_STATUS
dp_rx_mon_buffers_alloc(struct dp_soc *soc)
{
	struct dp_srng *mon_buf_ring;
	struct dp_mon_desc_pool *rx_mon_desc_pool;
	union dp_mon_desc_list_elem_t *desc_list = NULL;
	union dp_mon_desc_list_elem_t *tail = NULL;
	struct dp_soc_be *be_soc = dp_get_be_soc_from_dp_soc(soc);
	struct dp_mon_soc_be *mon_soc = be_soc->monitor_soc_be;

	mon_buf_ring = &soc->rxdma_mon_buf_ring[0];

	rx_mon_desc_pool = &mon_soc->rx_desc_mon;

	return dp_mon_buffers_replenish(soc, mon_buf_ring,
					rx_mon_desc_pool,
					mon_soc->rx_mon_ring_fill_level,
					&desc_list, &tail);
}
