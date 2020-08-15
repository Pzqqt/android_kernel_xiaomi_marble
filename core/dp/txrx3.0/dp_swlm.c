/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#ifdef WLAN_DP_FEATURE_SW_LATENCY_MGR

#include "dp_swlm.h"

/**
 * dp_tcl_should_coalesc() - To know if the current TCL reg write is to be
 *			     processed or coalesced.
 * @soc: Datapath global soc handle
 * @tx_desc: descriptor for the current packet to be transmitted
 *
 * This function takes into account the current tx and rx throughput and
 * decides whether the TCL register write corresponding to the current packet,
 * to be transmitted, is to be processed or coalesced.
 * It maintains a session for which the TCL register writes are coalesced and
 * then flushed if a certain time/bytes threshold is reached.
 *
 * Returns: 1 if the current TCL write is to be coalesced
 *	    0, if the current TCL write is to be processed.
 */
static int dp_tcl_should_coalesc(struct dp_soc *soc,
				 struct dp_tx_desc_s *tx_desc)
{
	return 0;
}

/**
 * dp_swlm_query_policy() - apply software latency policy based on ring type.
 * @soc: Datapath global soc handle
 * @ring_type: SRNG type
 * @query_data: private data for the query corresponding to the ring type
 *
 * Returns: 0 always
 */
int dp_swlm_query_policy(struct dp_soc *soc, int ring_type,
			 union swlm_data query_data)
{
	if (qdf_unlikely(!soc->swlm.is_enabled))
		return 0;

	switch (ring_type) {
	case TCL_DATA:
		return soc->swlm.ops->tcl_should_coalesc(soc,
							 query_data.tx_desc);
	default:
		break
	}

	return 0;
}

struct dp_swlm_ops dp_latency_mgr_ops = {
	.tcl_should_coalesc = dp_tcl_should_coalesc,
};

/**
 * dp_soc_swlm_attach() - attach the software latency manager resources
 * @soc: Datapath global soc handle
 *
 * Returns: QDF_STATUS
 */
QDF_STATUS dp_soc_swlm_attach(struct dp_soc *soc)
{
	soc->swlm.ops = &dp_latency_mgr_ops;
	soc->swlm.is_enabled = true;

	return QDF_STATUS_SUCCESS;
}

#endif /* WLAN_DP_FEATURE_SW_LATENCY_MGR */
