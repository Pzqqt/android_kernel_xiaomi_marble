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
 * dp_swlm_is_tput_thresh_reached() - Calculate the current tx and rx TPUT
 *				      and check if it passes the pre-set
 *				      threshold.
 * @soc: Datapath global soc handle
 *
 * This function calculates the current TX and RX throughput and checks
 * if it is above the pre-set thresholds by SWLM.
 *
 * Returns: true, if the TX/RX throughput is passing the threshold
 *	    false, otherwise
 */
static bool dp_swlm_is_tput_thresh_reached(struct dp_soc *soc)
{
	struct dp_swlm *swlm = &soc->swlm;
	static int prev_rx_bytes, prev_tx_bytes;
	int rx_delta, tx_delta;

	tx_delta = soc->stats.tx.egress.bytes - prev_tx_bytes;
	prev_tx_bytes = soc->stats.tx.egress.bytes;
	if (tx_delta > swlm->params.tcl.tx_traffic_thresh) {
		swlm->params.tcl.sampling_session_tx_bytes = tx_delta;
		return true;
	}

	rx_delta = soc->stats.rx.ingress.bytes - prev_rx_bytes;
	prev_rx_bytes = soc->stats.rx.ingress.bytes;
	if (rx_delta > swlm->params.tcl.rx_traffic_thresh) {
		swlm->params.tcl.sampling_session_tx_bytes = tx_delta;
		return true;
	}

	return false;
}

/**
 * dp_swlm_can_tcl_wr_coalesce() - To check if current TCL reg write can be
 *				   coalesced or not.
 * @soc: Datapath global soc handle
 * @tcl_data: priv data for tcl coalescing
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
static int
dp_swlm_can_tcl_wr_coalesce(struct dp_soc *soc,
			    struct dp_swlm_tcl_data *tcl_data)
{
	u64 curr_time = qdf_get_log_timestamp_usecs();
	int tput_level_pass, coalesce = 0;
	struct dp_swlm *swlm = &soc->swlm;
	static int tput_pass_cnt;
	static u64 expire_time;

	if (curr_time >= expire_time) {
		expire_time = qdf_get_log_timestamp_usecs() +
			      swlm->params.tcl.sampling_time;
		tput_level_pass = dp_swlm_is_tput_thresh_reached(soc);
		if (tput_level_pass) {
			tput_pass_cnt++;
		} else {
			tput_pass_cnt = 0;
			DP_STATS_INC(swlm, tcl.tput_criteria_fail, 1);
			goto coalescing_fail;
		}
	}

	swlm->params.tcl.bytes_coalesced += qdf_nbuf_len(tcl_data->nbuf);

	if (tput_pass_cnt > DP_SWLM_TCL_TPUT_PASS_THRESH) {
		coalesce = 1;
		if (swlm->params.tcl.bytes_coalesced >
		    swlm->params.tcl.bytes_flush_thresh) {
			coalesce = 0;
			DP_STATS_INC(swlm, tcl.bytes_thresh_reached, 1);
		} else if (curr_time > swlm->params.tcl.coalesce_end_time) {
			coalesce = 0;
			DP_STATS_INC(swlm, tcl.time_thresh_reached, 1);
		}
	}

coalescing_fail:
	if (!coalesce) {
		dp_swlm_tcl_reset_session_data(soc);
		return 0;
	}

	qdf_timer_mod(&swlm->params.tcl.flush_timer, 1);

	return 1;
}

static struct dp_swlm_ops dp_latency_mgr_ops = {
	.tcl_wr_coalesce_check = dp_swlm_can_tcl_wr_coalesce,
};

/**
 * dp_swlm_tcl_flush_timer() - Timer handler for tcl register write coalescing
 * @arg: private data of the timer
 *
 * Returns: none
 */
static void dp_swlm_tcl_flush_timer(void *arg)
{
	struct dp_soc *soc = arg;
	struct dp_swlm *swlm = &soc->swlm;
	hal_ring_handle_t hal_ring_hdl =
				soc->tcl_data_ring[0].hal_srng;

	if (hal_srng_try_access_start(soc->hal_soc, hal_ring_hdl) < 0) {
		DP_STATS_INC(swlm, tcl.timer_flush_fail, 1);
		return;
	}

	DP_STATS_INC(swlm, tcl.timer_flush_success, 1);
	hal_srng_access_end(soc->hal_soc, hal_ring_hdl);
}

/**
 * dp_soc_swlm_tcl_attach() - attach the TCL resources for the software
 *			      latency manager.
 * @soc: Datapath global soc handle
 *
 * Returns: QDF_STATUS
 */
static inline QDF_STATUS dp_soc_swlm_tcl_attach(struct dp_soc *soc)
{
	struct dp_swlm *swlm = &soc->swlm;

	swlm->params.tcl.rx_traffic_thresh = DP_SWLM_TCL_RX_TRAFFIC_THRESH;
	swlm->params.tcl.tx_traffic_thresh = DP_SWLM_TCL_TX_TRAFFIC_THRESH;
	swlm->params.tcl.sampling_time = DP_SWLM_TCL_TRAFFIC_SAMPLING_TIME;
	swlm->params.tcl.bytes_flush_thresh = 0;
	swlm->params.tcl.time_flush_thresh = DP_SWLM_TCL_TIME_FLUSH_THRESH;
	swlm->params.tcl.tx_thresh_multiplier =
					DP_SWLM_TCL_TX_THRESH_MULTIPLIER;

	qdf_timer_init(soc->osdev, &swlm->params.tcl.flush_timer,
		       dp_swlm_tcl_flush_timer, (void *)soc,
		       QDF_TIMER_TYPE_WAKE_APPS);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_soc_swlm_tcl_detach() - detach the TCL resources for the software
 *			      latency manager.
 * @soc: Datapath global soc handle
 *
 * Returns: QDF_STATUS
 */
static inline QDF_STATUS dp_soc_swlm_tcl_detach(struct dp_soc *soc)
{
	struct dp_swlm *swlm = &soc->swlm;

	qdf_timer_stop(&swlm->params.tcl.flush_timer);
	qdf_timer_free(&swlm->params.tcl.flush_timer);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_soc_swlm_attach(struct dp_soc *soc)
{
	struct dp_swlm *swlm = &soc->swlm;
	QDF_STATUS ret;

	swlm->ops = &dp_latency_mgr_ops;

	ret = dp_soc_swlm_tcl_attach(soc);
	if (QDF_IS_STATUS_ERROR(ret))
		goto swlm_tcl_setup_fail;

	swlm->is_enabled = true;

	return QDF_STATUS_SUCCESS;

swlm_tcl_setup_fail:
	swlm->is_enabled = false;
	return ret;
}

QDF_STATUS dp_soc_swlm_detach(struct dp_soc *soc)
{
	struct dp_swlm *swlm = &soc->swlm;
	QDF_STATUS ret;

	swlm->is_enabled = false;

	ret = dp_soc_swlm_tcl_detach(soc);
	if (QDF_IS_STATUS_ERROR(ret))
		return ret;

	swlm->ops = NULL;

	return QDF_STATUS_SUCCESS;
}
#endif /* WLAN_DP_FEATURE_SW_LATENCY_MGR */
