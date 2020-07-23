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

/**
 * DOC: wlan_dcs.c
 *
 * This file provide definitions for following:
 * - (de)register to WMI events for psoc enable
 * - send dcs wmi command
 * - dcs algorithm handling
 */

#include <target_if_dcs.h>
#include "wlan_dcs.h"

struct dcs_pdev_priv_obj *
wlan_dcs_get_pdev_private_obj(struct wlan_objmgr_psoc *psoc, uint32_t pdev_id)
{
	struct dcs_psoc_priv_obj *dcs_psoc_obj;
	struct dcs_pdev_priv_obj *dcs_pdev_priv = NULL;

	if (!psoc) {
		dcs_err("psoc is null");
		goto end;
	}

	dcs_psoc_obj = wlan_objmgr_psoc_get_comp_private_obj(
							psoc,
							WLAN_UMAC_COMP_DCS);
	if (!dcs_psoc_obj) {
		dcs_err("dcs psoc object is null");
		goto end;
	}

	if (pdev_id >= WLAN_DCS_MAX_PDEVS) {
		dcs_err("invalid pdev_id: %u", pdev_id);
		goto end;
	}

	dcs_pdev_priv = &dcs_psoc_obj->dcs_pdev_priv[pdev_id];
end:

	return dcs_pdev_priv;
}

QDF_STATUS wlan_dcs_attach(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_target_if_dcs_tx_ops *dcs_tx_ops;

	if (!psoc) {
		dcs_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	dcs_tx_ops = target_if_dcs_get_tx_ops(psoc);
	if (!dcs_tx_ops) {
		dcs_err("tx_ops is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!dcs_tx_ops->dcs_attach) {
		dcs_err("dcs_attach function is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return dcs_tx_ops->dcs_attach(psoc);
}

QDF_STATUS wlan_dcs_detach(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_target_if_dcs_tx_ops *dcs_tx_ops;

	if (!psoc) {
		dcs_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	dcs_tx_ops = target_if_dcs_get_tx_ops(psoc);
	if (!dcs_tx_ops) {
		dcs_err("tx_ops is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	if (!dcs_tx_ops->dcs_detach) {
		dcs_err("dcs_detach function is null!");
		return QDF_STATUS_E_NULL_VALUE;
	}

	return dcs_tx_ops->dcs_detach(psoc);
}

QDF_STATUS wlan_dcs_cmd_send(struct wlan_objmgr_psoc *psoc,
			     uint32_t pdev_id,
			     bool is_host_pdev_id)
{
	struct wlan_target_if_dcs_tx_ops *dcs_tx_ops;
	struct dcs_pdev_priv_obj *dcs_pdev_priv;
	uint32_t dcs_enable;

	if (!psoc) {
		dcs_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	dcs_enable = dcs_pdev_priv->dcs_host_params.dcs_enable &
			dcs_pdev_priv->dcs_host_params.dcs_enable_cfg;
	dcs_tx_ops = target_if_dcs_get_tx_ops(psoc);

	if (dcs_tx_ops->dcs_cmd_send) {
		dcs_info("dcs_enable: %u, pdev_id: %u", dcs_enable, pdev_id);
		return dcs_tx_ops->dcs_cmd_send(psoc,
						pdev_id,
						is_host_pdev_id,
						dcs_enable);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_dcs_disable_cmd_send() - send dcs disable command to target_if layer
 * @psoc: psoc pointer
 * @pdev_id: pdev_id
 * @is_host_pdev_id: pdev_id is host id or not
 *
 * The function gets called to send dcs disable command to FW
 *
 * return: QDF_STATUS_SUCCESS for success or error code
 */
static QDF_STATUS wlan_dcs_disable_cmd_send(struct wlan_objmgr_psoc *psoc,
					    uint32_t pdev_id,
					    bool is_host_pdev_id)
{
	struct wlan_target_if_dcs_tx_ops *dcs_tx_ops;
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	if (!psoc) {
		dcs_err("psoc is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return QDF_STATUS_E_NULL_VALUE;
	}

	dcs_tx_ops = target_if_dcs_get_tx_ops(psoc);
	if (dcs_tx_ops->dcs_cmd_send) {
		dcs_info("dcs_enable: %u, pdev_id: %u", 0, pdev_id);
		return dcs_tx_ops->dcs_cmd_send(psoc,
						pdev_id,
						is_host_pdev_id,
						0);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_dcs_im_copy_stats() - dcs target interference mitigation statistics copy
 * @prev_stats: previous statistics pointer
 * @curr_stats: current statistics pointer
 *
 * Return: None
 */
static inline void
wlan_dcs_im_copy_stats(struct wlan_host_dcs_im_tgt_stats *prev_stats,
		       struct wlan_host_dcs_im_tgt_stats *curr_stats)
{
	if (!prev_stats || !curr_stats) {
		dcs_err("previous or current stats is null");
		return;
	}

	/*
	 * Right now no other actions are required beyond memcopy,
	 * if required the rest of the code would follow.
	 */
	qdf_mem_copy(prev_stats, curr_stats,
		     sizeof(struct wlan_host_dcs_im_tgt_stats));
}

/**
 * wlan_dcs_im_print_stats() - print current/previous dcs target im statistics
 * @prev_stats: previous statistics pointer
 * @curr_stats: current statistics pointer
 *
 * Return: None
 */
static void
wlan_dcs_im_print_stats(struct wlan_host_dcs_im_tgt_stats *prev_stats,
			struct wlan_host_dcs_im_tgt_stats *curr_stats)
{
	if (!prev_stats || !curr_stats) {
		dcs_err("previous or current stats is null");
		return;
	}

	/* Debug, dump all received stats first */
	dcs_debug("tgt_curr/tsf: %u", curr_stats->reg_tsf32);
	dcs_debug("tgt_curr/last_ack_rssi: %u", curr_stats->last_ack_rssi);
	dcs_debug("tgt_curr/tx_waste_time: %u", curr_stats->tx_waste_time);
	dcs_debug("tgt_curr/dcs_rx_time: %u", curr_stats->rx_time);
	dcs_debug("tgt_curr/listen_time: %u",
		  curr_stats->mib_stats.listen_time);
	dcs_debug("tgt_curr/tx_frame_cnt: %u",
		  curr_stats->mib_stats.reg_tx_frame_cnt);
	dcs_debug("tgt_curr/rx_frame_cnt: %u",
		  curr_stats->mib_stats.reg_rx_frame_cnt);
	dcs_debug("tgt_curr/rxclr_cnt: %u",
		  curr_stats->mib_stats.reg_rxclr_cnt);
	dcs_debug("tgt_curr/reg_cycle_cnt: %u",
		  curr_stats->mib_stats.reg_cycle_cnt);
	dcs_debug("tgt_curr/rxclr_ext_cnt: %u",
		  curr_stats->mib_stats.reg_rxclr_ext_cnt);
	dcs_debug("tgt_curr/ofdm_phyerr_cnt: %u",
		  curr_stats->mib_stats.reg_ofdm_phyerr_cnt);
	dcs_debug("tgt_curr/cck_phyerr_cnt: %u",
		  curr_stats->mib_stats.reg_cck_phyerr_cnt);

	dcs_debug("tgt_prev/tsf: %u", prev_stats->reg_tsf32);
	dcs_debug("tgt_prev/last_ack_rssi: %u", prev_stats->last_ack_rssi);
	dcs_debug("tgt_prev/tx_waste_time: %u", prev_stats->tx_waste_time);
	dcs_debug("tgt_prev/rx_time: %u", prev_stats->rx_time);
	dcs_debug("tgt_prev/listen_time: %u",
		  prev_stats->mib_stats.listen_time);
	dcs_debug("tgt_prev/tx_frame_cnt: %u",
		  prev_stats->mib_stats.reg_tx_frame_cnt);
	dcs_debug("tgt_prev/rx_frame_cnt: %u",
		  prev_stats->mib_stats.reg_rx_frame_cnt);
	dcs_debug("tgt_prev/rxclr_cnt: %u",
		  prev_stats->mib_stats.reg_rxclr_cnt);
	dcs_debug("tgt_prev/reg_cycle_cnt: %u",
		  prev_stats->mib_stats.reg_cycle_cnt);
	dcs_debug("tgt_prev/rxclr_ext_cnt: %u",
		  prev_stats->mib_stats.reg_rxclr_ext_cnt);
	dcs_debug("tgt_prev/ofdm_phyerr_cnt: %u",
		  prev_stats->mib_stats.reg_ofdm_phyerr_cnt);
	dcs_debug("tgt_prev/cck_phyerr_cnt: %u",
		  prev_stats->mib_stats.reg_cck_phyerr_cnt);
}

/**
 * wlan_dcs_wlan_interference_process() - dcs detection algorithm handling
 * @curr_stats: current target im stats pointer
 * @dcs_pdev_priv: dcs pdev priv pointer
 *
 * Return: true or false means start dcs callback handler or not
 */
static bool
wlan_dcs_wlan_interference_process(
				struct wlan_host_dcs_im_tgt_stats *curr_stats,
				struct dcs_pdev_priv_obj *dcs_pdev_priv)
{
	struct wlan_host_dcs_im_tgt_stats *prev_stats;
	struct pdev_dcs_params dcs_host_params;
	struct pdev_dcs_im_stats *p_dcs_im_stats;
	bool start_dcs_cbk_handler = false;

	uint32_t reg_tsf_delta = 0;
	uint32_t rxclr_delta = 0;
	uint32_t rxclr_ext_delta = 0;
	uint32_t cycle_count_delta = 0;
	uint32_t tx_frame_delta = 0;
	uint32_t rx_frame_delta = 0;
	uint32_t reg_total_cu = 0;
	uint32_t reg_tx_cu = 0;
	uint32_t reg_rx_cu = 0;
	uint32_t reg_unused_cu = 0;
	uint32_t rx_time_cu = 0;
	uint32_t reg_ofdm_phyerr_delta = 0;
	uint32_t reg_cck_phyerr_delta = 0;
	uint32_t reg_ofdm_phyerr_cu = 0;
	uint32_t ofdm_phy_err_rate = 0;
	uint32_t cck_phy_err_rate = 0;
	uint32_t max_phy_err_rate = 0;
	uint32_t max_phy_err_count = 0;
	uint32_t total_wasted_cu = 0;
	uint32_t wasted_tx_cu = 0;
	uint32_t tx_err = 0;
	uint32_t too_many_phy_errors = 0;

	if (!curr_stats) {
		dcs_err("curr_stats is NULL");
		goto end;
	}

	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is NULL");
		goto end;
	}

	dcs_host_params = dcs_pdev_priv->dcs_host_params;
	p_dcs_im_stats = &dcs_pdev_priv->dcs_im_stats;
	prev_stats =  &dcs_pdev_priv->dcs_im_stats.prev_dcs_im_stats;

	if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
		wlan_dcs_im_print_stats(prev_stats, curr_stats);

	/*
	 * Counters would have wrapped. Ideally we should be able to figure this
	 * out, but we never know how many times counters wrapped, just ignore.
	 */
	if ((curr_stats->mib_stats.listen_time <= 0) ||
	    (curr_stats->reg_tsf32 <= prev_stats->reg_tsf32)) {
		if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
			dcs_debug("ignoring due to negative TSF value");

		/* Copy the current stats to previous stats for next run */
		wlan_dcs_im_copy_stats(prev_stats, curr_stats);

		goto end;
	}

	reg_tsf_delta = curr_stats->reg_tsf32 - prev_stats->reg_tsf32;

	/*
	 * Do nothing if current stats are not seeming good, probably
	 * a reset happened on chip, force cleared
	 */
	if (prev_stats->mib_stats.reg_rxclr_cnt >
		curr_stats->mib_stats.reg_rxclr_cnt) {
		if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
			dcs_debug("ignoring due to negative rxclr count");

		/* Copy the current stats to previous stats for next run */
		wlan_dcs_im_copy_stats(prev_stats, curr_stats);

		goto end;
	}

	rxclr_delta = curr_stats->mib_stats.reg_rxclr_cnt -
			prev_stats->mib_stats.reg_rxclr_cnt;
	rxclr_ext_delta = curr_stats->mib_stats.reg_rxclr_ext_cnt -
				prev_stats->mib_stats.reg_rxclr_ext_cnt;
	tx_frame_delta = curr_stats->mib_stats.reg_tx_frame_cnt -
				prev_stats->mib_stats.reg_tx_frame_cnt;

	rx_frame_delta = curr_stats->mib_stats.reg_rx_frame_cnt -
				prev_stats->mib_stats.reg_rx_frame_cnt;

	cycle_count_delta = curr_stats->mib_stats.reg_cycle_cnt -
				prev_stats->mib_stats.reg_cycle_cnt;

	if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
		dcs_debug("rxclr_delta: %u, rxclr_ext_delta: %u, tx_frame_delta: %u, rx_frame_delta: %u, cycle_count_delta: %u",
			  rxclr_delta, rxclr_ext_delta,
			  tx_frame_delta, rx_frame_delta, cycle_count_delta);

	if (0 == (cycle_count_delta >> 8)) {
		wlan_dcs_im_copy_stats(prev_stats, curr_stats);
		if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
			dcs_debug("cycle count NULL --Investigate--");

		goto end;
	}

	/*
	 * Total channel utiliztaion is the amount of time RXCLR is
	 * counted. RXCLR is counted, when 'RX is NOT clear', please
	 * refer to mac documentation. It means either TX or RX is ON
	 *
	 * Why shift by 8 ? after multiplication it could overflow. At one
	 * second rate, neither cycle_count_celta nor the tsf_delta would be
	 * zero after shift by 8 bits
	 */
	reg_total_cu = ((rxclr_delta >> 8) * 100) / (cycle_count_delta >> 8);
	reg_tx_cu = ((tx_frame_delta >> 8) * 100) / (cycle_count_delta >> 8);
	reg_rx_cu = ((rx_frame_delta >> 8) * 100) / (cycle_count_delta >> 8);
	rx_time_cu = ((curr_stats->rx_time >> 8) * 100) / (reg_tsf_delta >> 8);

	/*
	 * Amount of the time AP received cannot go higher than the receive
	 * cycle count delta. If at all it is, there should have been a
	 * computation error, ceil it to receive_cycle_count_diff
	 */
	if (rx_time_cu > reg_rx_cu)
		rx_time_cu = reg_rx_cu;

	if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
		dcs_debug("reg_total_cu: %u, reg_tx_cu: %u, reg_rx_cu: %u, rx_time_cu: %u",
			  reg_total_cu, reg_tx_cu, reg_rx_cu, rx_time_cu);

	/*
	 * Unusable channel utilization is amount of time that we
	 * spent in backoff or waiting for other transmit/receive to
	 * complete. If there is interference it is more likely that
	 * we overshoot the limit. In case of multiple stations, we
	 * still see increased channel utilization.  This assumption may
	 * not be true for the VOW scenario where either multicast or
	 * unicast-UDP is used ( mixed traffic would still cause high
	 * channel utilization).
	 */
	wasted_tx_cu = ((curr_stats->tx_waste_time >> 8) * 100) /
							(reg_tsf_delta >> 8);

	/*
	 * Transmit channel utilization cannot go higher than the amount of time
	 * wasted, if so cap the wastage to transmit channel utillzation. This
	 * could happen to compution error.
	 */
	if (reg_tx_cu < wasted_tx_cu)
		wasted_tx_cu = reg_tx_cu;

	tx_err = (reg_tx_cu && wasted_tx_cu) ?
			(wasted_tx_cu * 100) / reg_tx_cu : 0;

	/*
	 * The below actually gives amount of time we are not using, or the
	 * interferer is active.
	 * rx_time_cu is what computed receive time *NOT* rx_cycle_count
	 * rx_cycle_count is our receive+interferer's transmit
	 * un-used is really total_cycle_counts -
	 *      (our_rx_time(rx_time_cu) + our_receive_time)
	 */
	reg_unused_cu = (reg_total_cu >= (reg_tx_cu + rx_time_cu)) ?
				(reg_total_cu - (reg_tx_cu + rx_time_cu)) : 0;

	/* If any retransmissions are there, count them as wastage */
	total_wasted_cu = reg_unused_cu + wasted_tx_cu;

	/* Check ofdm and cck errors */
	if (unlikely(curr_stats->mib_stats.reg_ofdm_phyerr_cnt <
			prev_stats->mib_stats.reg_ofdm_phyerr_cnt))
		reg_ofdm_phyerr_delta =
			curr_stats->mib_stats.reg_ofdm_phyerr_cnt;
	else
		reg_ofdm_phyerr_delta =
			curr_stats->mib_stats.reg_ofdm_phyerr_cnt -
				prev_stats->mib_stats.reg_ofdm_phyerr_cnt;

	if (unlikely(curr_stats->mib_stats.reg_cck_phyerr_cnt <
			prev_stats->mib_stats.reg_cck_phyerr_cnt))
		reg_cck_phyerr_delta = curr_stats->mib_stats.reg_cck_phyerr_cnt;
	else
		reg_cck_phyerr_delta =
			curr_stats->mib_stats.reg_cck_phyerr_cnt -
				prev_stats->mib_stats.reg_cck_phyerr_cnt;

	/*
	 * Add the influence of ofdm phy errors to the wasted channel
	 * utillization, this computed through time wasted in errors
	 */
	reg_ofdm_phyerr_cu = reg_ofdm_phyerr_delta *
				dcs_host_params.phy_err_penalty;
	total_wasted_cu +=
		(reg_ofdm_phyerr_cu > 0) ?
		(((reg_ofdm_phyerr_cu >> 8) * 100) / (reg_tsf_delta >> 8)) : 0;

	ofdm_phy_err_rate = (curr_stats->mib_stats.reg_ofdm_phyerr_cnt * 1000) /
				curr_stats->mib_stats.listen_time;
	cck_phy_err_rate = (curr_stats->mib_stats.reg_cck_phyerr_cnt * 1000) /
				curr_stats->mib_stats.listen_time;

	if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE)) {
		dcs_debug("reg_unused_cu: %u, reg_ofdm_phyerr_delta: %u, reg_cck_phyerr_delta: %u, reg_ofdm_phyerr_cu: %u",
			  reg_unused_cu, reg_ofdm_phyerr_delta,
			  reg_cck_phyerr_delta, reg_ofdm_phyerr_cu);
		dcs_debug("total_wasted_cu: %u, ofdm_phy_err_rate: %u, cck_phy_err_rate: %u",
			  total_wasted_cu, ofdm_phy_err_rate, cck_phy_err_rate);
		dcs_debug("new_unused_cu: %u, reg_ofdm_phy_error_cu: %u",
			  reg_unused_cu,
			 (curr_stats->mib_stats.reg_ofdm_phyerr_cnt * 100) /
					curr_stats->mib_stats.listen_time);
	}

	/* Check if the error rates are higher than the thresholds */
	max_phy_err_rate = QDF_MAX(ofdm_phy_err_rate, cck_phy_err_rate);

	max_phy_err_count = QDF_MAX(curr_stats->mib_stats.reg_ofdm_phyerr_cnt,
				    curr_stats->mib_stats.reg_cck_phyerr_cnt);

	if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
		dcs_debug("max_phy_err_rate: %u, max_phy_err_count: %u",
			  max_phy_err_rate, max_phy_err_count);

	if (((max_phy_err_rate >= dcs_host_params.phy_err_threshold) &&
	     (max_phy_err_count > dcs_host_params.phy_err_threshold)) ||
		(curr_stats->phyerr_cnt > dcs_host_params.radar_err_threshold))
		too_many_phy_errors = 1;

	if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_CRITICAL)) {
		dcs_debug("total_cu: %u, tx_cu: %u, rx_cu: %u, rx_time_cu: %u, unused cu: %u",
			  reg_total_cu, reg_tx_cu,
			  reg_rx_cu, rx_time_cu, reg_unused_cu);
		dcs_debug("phyerr: %u, total_wasted_cu: %u, phyerror_cu: %u, wasted_cu: %u, reg_tx_cu: %u, reg_rx_cu: %u",
			  too_many_phy_errors, total_wasted_cu,
			  reg_ofdm_phyerr_cu, wasted_tx_cu,
			  reg_tx_cu, reg_rx_cu);
		dcs_debug("tx_err: %u", tx_err);
	}

	if (reg_unused_cu >= dcs_host_params.coch_intfr_threshold)
		/* Quickly reach to decision */
		p_dcs_im_stats->im_intfr_cnt += 2;
	else if (too_many_phy_errors &&
		 (((total_wasted_cu >
			(dcs_host_params.coch_intfr_threshold + 10)) &&
		((reg_tx_cu + reg_rx_cu) > dcs_host_params.user_max_cu)) ||
		((reg_tx_cu > DCS_TX_MAX_CU) &&
			(tx_err >= dcs_host_params.tx_err_threshold))))
		p_dcs_im_stats->im_intfr_cnt++;

	if (p_dcs_im_stats->im_intfr_cnt >=
		dcs_host_params.intfr_detection_threshold) {
		if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_CRITICAL)) {
			dcs_debug("interference threshold exceeded");
			dcs_debug("unused_cu: %u, too_any_phy_errors: %u, total_wasted_cu: %u, reg_tx_cu: %u, reg_rx_cu: %u",
				  reg_unused_cu, too_many_phy_errors,
				  total_wasted_cu, reg_tx_cu, reg_rx_cu);
		}

		p_dcs_im_stats->im_intfr_cnt = 0;
		p_dcs_im_stats->im_samp_cnt = 0;
		/*
		 * Once the interference is detected, change the channel, as on
		 * today this is common routine for wirelesslan and
		 * non-wirelesslan interference. Name as such kept the same
		 * because of the DA code, which is using the same function.
		 */
		start_dcs_cbk_handler = true;
	} else if (0 == p_dcs_im_stats->im_intfr_cnt ||
			p_dcs_im_stats->im_samp_cnt >=
				dcs_host_params.intfr_detection_window) {
		p_dcs_im_stats->im_intfr_cnt = 0;
		p_dcs_im_stats->im_samp_cnt = 0;
	}

	/* Count the current run too */
	p_dcs_im_stats->im_samp_cnt++;

	/* Copy the stats for next cycle */
	wlan_dcs_im_copy_stats(prev_stats, curr_stats);

	if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
		dcs_debug("intfr_count: %u, sample_count: %u",
			  p_dcs_im_stats->im_intfr_cnt,
			  p_dcs_im_stats->im_samp_cnt);

end:
	return start_dcs_cbk_handler;
}

void wlan_dcs_disable_timer_fn(void *dcs_timer_args)
{
	struct pdev_dcs_timer_args *dcs_timer_args_ctx;
	struct wlan_objmgr_psoc *psoc;
	uint32_t pdev_id;
	struct dcs_psoc_priv_obj *dcs_psoc_priv;
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	if (!dcs_timer_args) {
		dcs_err("dcs timer args is null");
		return;
	}

	dcs_timer_args_ctx = (struct pdev_dcs_timer_args *)dcs_timer_args;
	psoc = dcs_timer_args_ctx->psoc;
	pdev_id = dcs_timer_args_ctx->pdev_id;

	dcs_psoc_priv =
		wlan_objmgr_psoc_get_comp_private_obj(psoc, WLAN_UMAC_COMP_DCS);
	if (!dcs_psoc_priv) {
		dcs_err("dcs psoc private object is null");
		return;
	}

	dcs_pdev_priv = &dcs_psoc_priv->dcs_pdev_priv[pdev_id];

	qdf_mem_set(&dcs_pdev_priv->dcs_im_stats,
		    sizeof(dcs_pdev_priv->dcs_im_stats), 0);
	dcs_pdev_priv->dcs_freq_ctrl_params.disable_delay_process = false;

	dcs_info("dcs disable timeout, enable dcs detection again");
	wlan_dcs_cmd_send(psoc, pdev_id, true);
}

/**
 * wlan_dcs_frequency_control() - dcs frequency control handling
 * @psoc: psoc pointer
 * @dcs_pdev_priv: dcs pdev priv pointer
 * @event: dcs stats event pointer
 *
 * Return: none
 */
static void wlan_dcs_frequency_control(struct wlan_objmgr_psoc *psoc,
				       struct dcs_pdev_priv_obj *dcs_pdev_priv,
				       struct dcs_stats_event *event)
{
	struct dcs_psoc_priv_obj *dcs_psoc_priv;
	struct pdev_dcs_freq_ctrl_params *dcs_freq_ctrl_params;
	uint8_t timestamp_pos;
	unsigned long current_time;
	uint8_t delta_pos;
	unsigned long delta_time;
	bool disable_dcs_sometime = false;

	if (!psoc || !dcs_pdev_priv || !event) {
		dcs_err("psoc or dcs_pdev_priv or event is null");
		return;
	}

	dcs_freq_ctrl_params = &dcs_pdev_priv->dcs_freq_ctrl_params;
	if (dcs_freq_ctrl_params->disable_delay_process) {
		dcs_err("In the process of dcs disable, shouldn't go to here");
		return;
	}

	current_time = qdf_get_system_timestamp();
	if (dcs_freq_ctrl_params->dcs_happened_count >=
		dcs_freq_ctrl_params->disable_threshold_per_5mins) {
		delta_pos =
			dcs_freq_ctrl_params->dcs_happened_count -
			dcs_freq_ctrl_params->disable_threshold_per_5mins;
		delta_pos = delta_pos % MAX_DCS_TIME_RECORD;

		delta_time = current_time -
				dcs_freq_ctrl_params->timestamp[delta_pos];
		if (delta_time < DCS_FREQ_CONTROL_TIME)
			disable_dcs_sometime = true;
	}

	if (!disable_dcs_sometime) {
		timestamp_pos = dcs_freq_ctrl_params->dcs_happened_count %
							MAX_DCS_TIME_RECORD;
		dcs_freq_ctrl_params->timestamp[timestamp_pos] = current_time;
		dcs_freq_ctrl_params->dcs_happened_count++;
	}

	/*
	 * Before start dcs callback handler or disable dcs for some time,
	 * need to send dcs command to disable dcs detection firstly.
	 */
	wlan_dcs_disable_cmd_send(psoc, event->dcs_param.pdev_id, true);

	if (disable_dcs_sometime) {
		dcs_freq_ctrl_params->disable_delay_process = true;
		dcs_pdev_priv->dcs_timer_args.psoc = psoc;
		dcs_pdev_priv->dcs_timer_args.pdev_id =
						event->dcs_param.pdev_id;
		qdf_timer_start(&dcs_pdev_priv->dcs_disable_timer,
				dcs_pdev_priv->dcs_freq_ctrl_params.
				restart_delay * 60 * 1000);
		dcs_info("start dcs disable timer");
	} else {
		dcs_psoc_priv = wlan_objmgr_psoc_get_comp_private_obj(
							psoc,
							WLAN_UMAC_COMP_DCS);
		if (!dcs_psoc_priv) {
			dcs_err("dcs private psoc object is null");
			return;
		}

		dcs_info("start dcs callback handler");
		dcs_psoc_priv->dcs_cbk.cbk(psoc, event->dcs_param.pdev_id,
					   event->dcs_param.interference_type,
					   dcs_psoc_priv->dcs_cbk.arg);
	}
}

QDF_STATUS
wlan_dcs_process(struct wlan_objmgr_psoc *psoc, struct dcs_stats_event *event)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;
	bool start_dcs_cbk_handler = false;

	if (!psoc || !event) {
		dcs_err("psoc or event is NULL");
		return QDF_STATUS_E_INVAL;
	}

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc,
						      event->dcs_param.pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return QDF_STATUS_E_INVAL;
	}

	if (unlikely(dcs_pdev_priv->dcs_host_params.dcs_debug
			>= DCS_DEBUG_VERBOSE))
		dcs_debug("dcs_enable: %u, interference_type: %u",
			  dcs_pdev_priv->dcs_host_params.dcs_enable,
			  event->dcs_param.interference_type);

	if (!dcs_pdev_priv->dcs_host_params.dcs_enable)
		return QDF_STATUS_SUCCESS;

	switch (event->dcs_param.interference_type) {
	case CAP_DCS_CWIM:
		break;
	case CAP_DCS_WLANIM:
		if (dcs_pdev_priv->dcs_host_params.dcs_enable & CAP_DCS_WLANIM)
			start_dcs_cbk_handler =
				wlan_dcs_wlan_interference_process(
							&event->wlan_stat,
							dcs_pdev_priv);
		if (start_dcs_cbk_handler)
			wlan_dcs_frequency_control(psoc,
						   dcs_pdev_priv,
						   event);
		break;
	default:
		dcs_err("unidentified interference type reported");
		break;
	}

	return QDF_STATUS_SUCCESS;
}

void wlan_dcs_clear(struct wlan_objmgr_psoc *psoc, uint32_t pdev_id)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	if (!psoc) {
		dcs_err("psoc is null");
		return;
	}

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return;
	}

	qdf_timer_stop(&dcs_pdev_priv->dcs_disable_timer);
	qdf_mem_set(&dcs_pdev_priv->dcs_im_stats,
		    sizeof(dcs_pdev_priv->dcs_im_stats), 0);
	qdf_mem_set(dcs_pdev_priv->dcs_freq_ctrl_params.timestamp,
		    MAX_DCS_TIME_RECORD * sizeof(unsigned long), 0);
	dcs_pdev_priv->dcs_freq_ctrl_params.dcs_happened_count = 0;
	dcs_pdev_priv->dcs_freq_ctrl_params.disable_delay_process = false;
}
