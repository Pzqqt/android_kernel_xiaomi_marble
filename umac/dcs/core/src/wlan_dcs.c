/*
 * Copyright (c) 2020-2021, The Linux Foundation. All rights reserved.
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
#include <wlan_objmgr_psoc_obj_i.h>
#include "wlan_utility.h"
#ifdef WLAN_POLICY_MGR_ENABLE
#include "wlan_policy_mgr_api.h"
#endif

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

	if (dcs_tx_ops && dcs_tx_ops->dcs_cmd_send) {
		dcs_info("dcs_enable: %u, pdev_id: %u", dcs_enable, pdev_id);
		return dcs_tx_ops->dcs_cmd_send(psoc,
						pdev_id,
						is_host_pdev_id,
						dcs_enable);
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
 * wlan_dcs_update_chan_util() - update chan utilization of dcs stats
 * @p_dcs_im_stats: pointer to pdev_dcs_im_stats
 * @rx_cu: rx channel utilization
 * @tx_cu: tx channel utilization
 * @obss_rx_cu: obss rx channel utilization
 * @total_cu: total channel utilization
 * @chan_nf: Channel noise floor (units are in dBm)
 *
 * Return: Void
 */
static void wlan_dcs_update_chan_util(struct pdev_dcs_im_stats *p_dcs_im_stats,
				      uint32_t rx_cu, uint32_t tx_cu,
				      uint32_t obss_rx_cu,
				      uint32_t total_cu, uint32_t chan_nf)
{
	if (p_dcs_im_stats) {
		p_dcs_im_stats->dcs_ch_util_im_stats.rx_cu = rx_cu;
		p_dcs_im_stats->dcs_ch_util_im_stats.tx_cu = tx_cu;
		p_dcs_im_stats->dcs_ch_util_im_stats.obss_rx_cu = obss_rx_cu;
		p_dcs_im_stats->dcs_ch_util_im_stats.total_cu = total_cu;
		p_dcs_im_stats->dcs_ch_util_im_stats.chan_nf = chan_nf;
	}
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
	uint32_t my_bss_rx_delta = 0;
	uint32_t reg_total_cu = 0;
	uint32_t reg_tx_cu = 0;
	uint32_t reg_rx_cu = 0;
	uint32_t obss_rx_cu = 0;
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
		goto copy_stats;
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
		goto copy_stats;
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

	my_bss_rx_delta = curr_stats->my_bss_rx_cycle_count -
				prev_stats->my_bss_rx_cycle_count;

	if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
		dcs_debug("rxclr_delta: %u, rxclr_ext_delta: %u, tx_frame_delta: %u, rx_frame_delta: %u, cycle_count_delta: %u, my_bss_rx_delta: %u",
			  rxclr_delta, rxclr_ext_delta, tx_frame_delta,
			  rx_frame_delta, cycle_count_delta, my_bss_rx_delta);

	if (0 == (cycle_count_delta >> 8)) {
		if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
			dcs_debug("cycle count NULL --Investigate--");
		goto copy_stats;
	}

	/* Update user stats */
	wlan_dcs_pdev_obj_lock(dcs_pdev_priv);
	if (dcs_pdev_priv->dcs_host_params.user_request_count) {
		struct wlan_host_dcs_im_user_stats *p_user_stats =
					     &p_dcs_im_stats->user_dcs_im_stats;

		p_user_stats->cycle_count += cycle_count_delta;
		p_user_stats->rxclr_count += rxclr_delta;
		p_user_stats->rx_frame_count += rx_frame_delta;
		p_user_stats->my_bss_rx_cycle_count += my_bss_rx_delta;
		if (0 == p_user_stats->max_rssi &&
		    0 == p_user_stats->min_rssi) {
			p_user_stats->max_rssi = curr_stats->last_ack_rssi;
			p_user_stats->min_rssi = curr_stats->last_ack_rssi;
		} else {
			if (curr_stats->last_ack_rssi > p_user_stats->max_rssi)
				p_user_stats->max_rssi =
						      curr_stats->last_ack_rssi;
			if (curr_stats->last_ack_rssi < p_user_stats->min_rssi)
				p_user_stats->min_rssi =
						      curr_stats->last_ack_rssi;
		}
		dcs_pdev_priv->dcs_host_params.user_request_count--;
		if (0 == dcs_pdev_priv->dcs_host_params.user_request_count)
			dcs_pdev_priv->dcs_host_params.notify_user = 1;
	}
	wlan_dcs_pdev_obj_unlock(dcs_pdev_priv);

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
	obss_rx_cu = (((rx_frame_delta - my_bss_rx_delta) >> 8) * 100) /
		     (cycle_count_delta >> 8);
	wlan_dcs_update_chan_util(p_dcs_im_stats, reg_rx_cu, reg_tx_cu,
				  obss_rx_cu, reg_total_cu,
				  curr_stats->chan_nf);

	/*
	 * Amount of the time AP received cannot go higher than the receive
	 * cycle count delta. If at all it is, there should have been a
	 * computation error, ceil it to receive_cycle_count_diff
	 */
	if (rx_time_cu > reg_rx_cu)
		rx_time_cu = reg_rx_cu;

	if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
		dcs_debug("reg_total_cu: %u, reg_tx_cu: %u, reg_rx_cu: %u, rx_time_cu: %u, obss_rx_cu: %u dcs_algorithm: %d",
			  reg_total_cu, reg_tx_cu, reg_rx_cu,
			  rx_time_cu, obss_rx_cu,
			  dcs_host_params.dcs_algorithm_process);

	/*
	 * For below scenario, will ignore dcs event data and won't do
	 * interference detection algorithm calculation:
	 * 1: Current SAP channel isn't on 5G band
	 * 2: In the process of ACS
	 * 3: In the process of dcs disabling dcs_restart_delay time duration
	 */
	if (!dcs_host_params.dcs_algorithm_process)
		goto copy_stats;

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

	if (unlikely(dcs_host_params.dcs_debug >= DCS_DEBUG_VERBOSE))
		dcs_debug("intfr_count: %u, sample_count: %u",
			  p_dcs_im_stats->im_intfr_cnt,
			  p_dcs_im_stats->im_samp_cnt);
copy_stats:
	 /* Copy the stats for next cycle */
	wlan_dcs_im_copy_stats(prev_stats, curr_stats);
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
	dcs_pdev_priv->dcs_freq_ctrl_params.disable_delay_process = false;

	dcs_info("dcs disable timeout, enable dcs detection again");
	wlan_dcs_set_algorithm_process(psoc, pdev_id, true);
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
				       struct wlan_host_dcs_event *event)
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
	 * need to ignore dcs event data and won't do interference detection
	 * algorithm calculation for disabling dcs detection firstly.
	 */
	wlan_dcs_set_algorithm_process(psoc, event->dcs_param.pdev_id, false);

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

/**
 * wlan_dcs_switch_chan() - switch channel for vdev
 * @vdev: vdev ptr
 * @tgt_freq: target frequency
 * @tgt_width: target channel width
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_dcs_switch_chan(struct wlan_objmgr_vdev *vdev, qdf_freq_t tgt_freq,
		     enum phy_ch_width tgt_width)
{
	struct wlan_objmgr_psoc *psoc;
	struct dcs_psoc_priv_obj *dcs_psoc_priv;
	dcs_switch_chan_cb switch_chan_cb;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return QDF_STATUS_E_INVAL;

	dcs_psoc_priv = wlan_objmgr_psoc_get_comp_private_obj(psoc,
					WLAN_UMAC_COMP_DCS);
	if (!dcs_psoc_priv)
		return QDF_STATUS_E_INVAL;

	switch_chan_cb = dcs_psoc_priv->switch_chan_cb;
	if (!switch_chan_cb)
		return QDF_STATUS_E_NOSUPPORT;

	return switch_chan_cb(vdev, tgt_freq, tgt_width);
}

#ifdef WLAN_POLICY_MGR_ENABLE
/**
 * wlan_dcs_get_pcl_for_sap() - get preferred channel list for SAP
 * @vdev: vdev ptr
 * @freq_list: Pointer to PCL
 * @freq_list_sz: Max size of PCL
 *
 * Return: number of channels in PCL
 */
static uint32_t wlan_dcs_get_pcl_for_sap(struct wlan_objmgr_vdev *vdev,
					 qdf_freq_t *freq_list,
					 uint32_t freq_list_sz)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev;
	struct policy_mgr_pcl_list *pcl;
	qdf_freq_t freq;
	enum channel_state state;
	QDF_STATUS status;
	int i, j;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return 0;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return 0;

	pcl = qdf_mem_malloc(sizeof(*pcl));
	if (!pcl)
		return 0;

	status = policy_mgr_get_pcl_for_vdev_id(psoc,
						PM_SAP_MODE,
						pcl->pcl_list, &pcl->pcl_len,
						pcl->weight_list,
						QDF_ARRAY_SIZE(pcl->weight_list),
						wlan_vdev_get_id(vdev));
	if (QDF_IS_STATUS_ERROR(status) || !pcl->pcl_len) {
		qdf_mem_free(pcl);
		return 0;
	}

	for (i = 0, j = 0; i < pcl->pcl_len && i < freq_list_sz; i++) {
		freq = (qdf_freq_t)pcl->pcl_list[i];
		state = wlan_reg_get_channel_state_for_freq(pdev, freq);
		if (state != CHANNEL_STATE_ENABLE)
			continue;

		freq_list[j++] = freq;
	}

	qdf_mem_free(pcl);
	return j;
}
#else
static uint32_t wlan_dcs_get_pcl_for_sap(struct wlan_objmgr_vdev *vdev,
					 qdf_freq_t *freq_list,
					 uint32_t freq_list_sz)
{
	struct wlan_objmgr_pdev *pdev;
	struct regulatory_channel *cur_chan_list;
	qdf_freq_t freq;
	enum channel_state state;
	int i, j;

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev)
		return 0;

	cur_chan_list = qdf_mem_malloc(NUM_CHANNELS *
			sizeof(struct regulatory_channel));
	if (!cur_chan_list)
		return 0;

	if (wlan_reg_get_current_chan_list(pdev, cur_chan_list) !=
					   QDF_STATUS_SUCCESS) {
		qdf_mem_free(cur_chan_list);
		return 0;
	}

	for (i = 0, j = 0; i < NUM_CHANNELS && i < freq_list_sz; i++) {
		freq = cur_chan_list[i].center_freq;
		state = wlan_reg_get_channel_state_for_freq(pdev, freq);
		if (state != CHANNEL_STATE_ENABLE)
			continue;

		freq_list[j++] = freq;
	}

	qdf_mem_free(cur_chan_list);
	return j;
}
#endif

/**
 * wlan_dcs_awgn_get_intf_for_seg() - get interference for specified segment
 * @awgn_info: awgn info pointer
 * @segment: segment index in channel band
 *
 * This function extracts the information from awgn event and check interference
 * within the specified segment.
 *
 * Return: true if interference is found within the segment, false otherwise.
 */
static bool
wlan_dcs_awgn_get_intf_for_seg(struct wlan_host_dcs_awgn_info *awgn_info,
			       uint32_t segment)
{
	uint32_t seg_mask;

	switch (segment) {
	case WLAN_DCS_SEG_PRI20:
		seg_mask = WLAN_DCS_SEG_PRI20_MASK;
		break;
	case WLAN_DCS_SEG_SEC20:
		seg_mask = WLAN_DCS_SEG_SEC20_MASK;
		break;
	case WLAN_DCS_SEG_SEC40:
		seg_mask = WLAN_DCS_SEG_SEC40_MASK;
		break;
	case WLAN_DCS_SEG_SEC80:
		seg_mask = WLAN_DCS_SEG_SEC80_MASK;
		break;
	case WLAN_DCS_SEG_SEC160:
		seg_mask = WLAN_DCS_SEG_SEC160_MASK;
		break;
	default:
		seg_mask = 0xFFFFFFFF;
		break;
	}

	return (awgn_info->chan_bw_intf_bitmap & seg_mask);
}

/**
 * wlan_dcs_get_max_seg_idx() - get max segment index for channel width
 * @width: channel width
 *
 * Return: max segment index(enum wlan_dcs_chan_seg) for the channel width.
 */
static enum wlan_dcs_chan_seg wlan_dcs_get_max_seg_idx(enum phy_ch_width width)
{
	switch (width) {
	case CH_WIDTH_160MHZ: /* fallthrough */
	case CH_WIDTH_80P80MHZ:
		return WLAN_DCS_SEG_SEC80;
	case CH_WIDTH_80MHZ:
		return WLAN_DCS_SEG_SEC40;
	case CH_WIDTH_40MHZ:
		return WLAN_DCS_SEG_SEC20;
	case CH_WIDTH_20MHZ:
		return WLAN_DCS_SEG_PRI20;
	default:
		dcs_err("Invalid ch width %d", width);
		return WLAN_DCS_SEG_INVALID;
	}
}

/**
 * wlan_dcs_get_chan_width_for_seg() - get channel width for specified segment
 * @seg_idx: segment index
 *
 * Return: channel width for segment index
 */
static enum phy_ch_width
wlan_dcs_get_chan_width_for_seg(enum wlan_dcs_chan_seg seg_idx)
{
	switch (seg_idx) {
	case WLAN_DCS_SEG_SEC80:
		return CH_WIDTH_160MHZ;
	case WLAN_DCS_SEG_SEC40:
		return CH_WIDTH_80MHZ;
	case WLAN_DCS_SEG_SEC20:
		return CH_WIDTH_40MHZ;
	case WLAN_DCS_SEG_PRI20:
		return CH_WIDTH_20MHZ;
	default:
		dcs_err("Invalid seg idx %d", seg_idx);
		return CH_WIDTH_INVALID;
	}
}

/**
 * wlan_dcs_get_max_no_intf_bw() - get max no interference band width
 * @awgn_info: pointer to awgn info
 * @width: pointer to channel width
 *
 * This function trys to get max no interference band width according to
 * awgn event.
 *
 * Return: true if valid no interference band width is found, false otherwise.
 */
static bool
wlan_dcs_get_max_no_intf_bw(struct wlan_host_dcs_awgn_info *awgn_info,
			    enum phy_ch_width *width)
{
	enum wlan_dcs_chan_seg seg_idx, max_seg_idx;

	max_seg_idx = wlan_dcs_get_max_seg_idx(awgn_info->channel_width);
	if (max_seg_idx == WLAN_DCS_SEG_INVALID)
		return false;

	seg_idx = WLAN_DCS_SEG_PRI20;
	while (seg_idx <= max_seg_idx) {
		if (wlan_dcs_awgn_get_intf_for_seg(awgn_info, seg_idx)) {
			dcs_debug("Intf found for seg idx %d", seg_idx);
			break;
		}
		seg_idx++;
	}

	/* scroll back to the last no-intf idx */
	seg_idx--;

	if (seg_idx == WLAN_DCS_SEG_INVALID) {
		/* If pri20 contains interference, do full channel change */
		dcs_debug("Primary 20MHz Channel interference detected");
		return false;
	}

	*width = wlan_dcs_get_chan_width_for_seg(seg_idx);
	if (*width == CH_WIDTH_160MHZ &&
	    awgn_info->channel_width == CH_WIDTH_80P80MHZ)
		*width = CH_WIDTH_80P80MHZ;

	dcs_debug("Found the max no intf width %d", *width);
	return (*width != CH_WIDTH_INVALID);
}

/**
 * wlan_dcs_get_available_chan_for_bw() - get available channel for specified
 *  band width
 * @pdev: pdev ptr
 * @awgn_info: pointer to awgn info
 * @bw: channel width
 * @freq_list: List of preferred channels
 * @freq_num: Number of channels in the PCL
 * @random: request for random channel
 *
 * Return: the selected channel frequency, 0 if no available chan is found.
 */
static qdf_freq_t
wlan_dcs_get_available_chan_for_bw(struct wlan_objmgr_pdev *pdev,
				   struct wlan_host_dcs_awgn_info *awgn_info,
				   enum phy_ch_width bw, qdf_freq_t *freq_list,
				   uint32_t freq_num, bool random)
{
	int i, j = 0;
	uint32_t random_chan_idx;
	qdf_freq_t freq, selected_freq = 0;
	const struct bonded_channel_freq *bonded_chan_ptr = NULL;
	enum channel_state state;
	uint16_t chan_cfreq;
	bool is_safe = true;

	if (!freq_list || !freq_num)
		return selected_freq;

	for (i = 0; i < freq_num; i++) {
		if (j && !random) {
			selected_freq = freq_list[0];
			dcs_debug("get the first available freq %u for bw %u",
				  selected_freq, bw);
			break;
		}

		freq = freq_list[i];
		if (!WLAN_REG_IS_SAME_BAND_FREQS(freq, awgn_info->center_freq))
			continue;

		/**
		 * DFS channel may need CAC during restart, which costs time
		 * and may cause failure.
		 */
		if (wlan_reg_is_dfs_for_freq(pdev, freq)) {
			dcs_debug("skip dfs freq %u", freq);
			continue;
		}

		if (bonded_chan_ptr &&
		    freq >= bonded_chan_ptr->start_freq &&
		    freq <= bonded_chan_ptr->end_freq) {
			if (is_safe) {
				dcs_debug("add freq directly [%d] = %u",
					  j, freq);
				freq_list[j++] = freq;
			}
			continue;
		}

		state = wlan_reg_get_5g_bonded_channel_and_state_for_freq(
				pdev, freq, bw, &bonded_chan_ptr);
		if (state != CHANNEL_STATE_ENABLE)
			continue;

		/* no bonding channel for 20MHz */
		if (bw == CH_WIDTH_20MHZ) {
			if (WLAN_DCS_IS_FREQ_IN_WIDTH(awgn_info->center_freq,
						      awgn_info->center_freq0,
						      awgn_info->center_freq1,
						      awgn_info->channel_width,
						      freq))
				continue;

			dcs_debug("add freq[%d] = %u", j, freq);
			freq_list[j++] = freq;
			continue;
		}

		is_safe = true;
		chan_cfreq =  bonded_chan_ptr->start_freq;
		while (chan_cfreq <= bonded_chan_ptr->end_freq) {
			if (WLAN_DCS_IS_FREQ_IN_WIDTH(awgn_info->center_freq,
						      awgn_info->center_freq0,
						      awgn_info->center_freq1,
						      awgn_info->channel_width,
						      chan_cfreq)) {
				is_safe = false;
				break;
			}
			chan_cfreq = chan_cfreq + 20;
		}
		if (is_safe) {
			dcs_debug("add freq[%d] = %u", j, freq);
			freq_list[j++] = freq;
		}
	}

	if (j && random) {
		qdf_get_random_bytes(&random_chan_idx, sizeof(random_chan_idx));
		random_chan_idx = random_chan_idx % j;
		selected_freq = freq_list[random_chan_idx];
		dcs_debug("get freq[%d] = %u for bw %u",
			  random_chan_idx, selected_freq, bw);
	}

	return selected_freq;
}

/**
 * wlan_dcs_sap_get_available_chan() - get available channel for sap
 * @vdev: vdev ptr
 * @awgn_info: pointer to awgn info
 * @tgt_freq: frequency of the selected channel
 * @tgt_width: band width of the selected channel
 * @random: request for random channel
 *
 * This function trys to get no-interference chan with max possible bandwidth
 * from pcl for sap according to awgn info.
 *
 * Return: true if available channel is found, false otherwise.
 */
static bool
wlan_dcs_sap_select_chan(struct wlan_objmgr_vdev *vdev,
			 struct wlan_host_dcs_awgn_info *awgn_info,
			 qdf_freq_t *tgt_freq, enum phy_ch_width *tgt_width,
			 bool random)
{
	int32_t tmp_width;
	qdf_freq_t tmp_freq = 0;
	struct wlan_objmgr_pdev *pdev;
	qdf_freq_t *freq_list;
	uint32_t freq_num;

	freq_list = qdf_mem_malloc(sizeof(*freq_list) * NUM_CHANNELS);
	if (!freq_list)
		return false;

	freq_num = wlan_dcs_get_pcl_for_sap(vdev, freq_list, NUM_CHANNELS);
	if (!freq_num) {
		qdf_mem_free(freq_list);
		return false;
	}

	tmp_width = awgn_info->channel_width;
	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		qdf_mem_free(freq_list);
		return false;
	}

	while (tmp_width >= CH_WIDTH_20MHZ) {
		tmp_freq = wlan_dcs_get_available_chan_for_bw(pdev, awgn_info,
							      tmp_width,
							      freq_list,
							      freq_num,
							      random);
		if (tmp_freq)
			break;
		tmp_width--;
	}

	if (tmp_freq) {
		*tgt_width = tmp_width;
		*tgt_freq = tmp_freq;
		dcs_debug("new_width: %d new_freq %u", tmp_width, tmp_freq);

		qdf_mem_free(freq_list);
		return true;
	}

	qdf_mem_free(freq_list);
	return false;
}

/**
 * wlan_dcs_is_awgnim_valid() - validate awgn info
 * @awgn_info: pointer to awgn info
 *
 * Return: true if valid, false otherwise.
 */
static inline bool
wlan_dcs_is_awgnim_valid(struct wlan_host_dcs_awgn_info *awgn_info)
{
	return (awgn_info &&
		awgn_info->center_freq && awgn_info->chan_bw_intf_bitmap &&
		awgn_info->channel_width != CH_WIDTH_INVALID &&
		WLAN_REG_IS_6GHZ_CHAN_FREQ(awgn_info->center_freq));
}

/**
 * wlan_dcs_vdev_get_op_chan_info() - get operating channel info for vdev
 * @vdev: pointer to vdev object
 * @cfreq: Center frequency of primary channel
 * @cfreq0: Center frequency of segment 1
 * @cfreq1: Center frequency of segment 2
 * @ch_width: Channel width, enum phy_ch_width
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
wlan_dcs_vdev_get_op_chan_info(struct wlan_objmgr_vdev *vdev,
			       qdf_freq_t *cfreq, qdf_freq_t *cfreq0,
			       qdf_freq_t *cfreq1, enum phy_ch_width *ch_width)
{
	struct wlan_channel *chan;

	if (!vdev)
		return QDF_STATUS_E_INVAL;

	*cfreq = 0;
	*cfreq0 = 0;
	*cfreq1 = 0;
	*ch_width = 0;

	if (wlan_vdev_mlme_is_active(vdev) != QDF_STATUS_SUCCESS)
		return QDF_STATUS_E_INVAL;

	chan = wlan_vdev_get_active_channel(vdev);
	if (!chan)
		return QDF_STATUS_E_INVAL;

	*cfreq = chan->ch_freq;
	*cfreq0 = chan->ch_cfreq1;
	*cfreq1 = chan->ch_cfreq2;
	*ch_width = chan->ch_width;

	return QDF_STATUS_SUCCESS;
}

/**
 * wlan_dcs_process_awgn_sta() - process AWGN event for STA
 * @pdev: pointer to pdev object
 * @object: vdev object
 * @arg: Arguments to the handler
 *
 * Return: void
 */
static void wlan_dcs_process_awgn_sta(struct wlan_objmgr_pdev *pdev,
				      void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = object;
	struct wlan_host_dcs_awgn_info *awgn_info = arg;
	enum phy_ch_width ch_width;
	enum phy_ch_width tgt_width = CH_WIDTH_INVALID;
	qdf_freq_t op_freq, cfreq0, cfreq1;
	qdf_freq_t tgt_freq = 0;
	QDF_STATUS status;
	uint8_t vdev_id;
	bool found;

	if (!vdev || !pdev)
		return;

	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_STA_MODE)
		return;

	vdev_id = wlan_vdev_get_id(vdev);
	status = wlan_dcs_vdev_get_op_chan_info(vdev, &op_freq, &cfreq0,
						&cfreq1, &ch_width);
	if (QDF_IS_STATUS_ERROR(status))
		return;

	if (awgn_info->center_freq != op_freq) {
		dcs_debug("STA-%d: freq not match", vdev_id);
		return;
	}

	found = wlan_dcs_get_max_no_intf_bw(awgn_info, &tgt_width);
	if (found) {
		if (ch_width <= tgt_width) {
			dcs_debug("STA-%d: freq and bw are unchanged", vdev_id);
			return;
		}

		tgt_freq = op_freq;
	}

	/* If no width is found, means to disconnect */
	dcs_debug("STA-%d: target freq %u width %u",
		  vdev_id, tgt_freq, tgt_width);
	wlan_dcs_switch_chan(vdev, tgt_freq, tgt_width);
}

/**
 * wlan_dcs_process_awgn_sap() - process AWGN event for SAP
 * @pdev: pointer to pdev object
 * @object: vdev object
 * @arg: Arguments to the handler
 *
 * Return: void
 */
static void wlan_dcs_process_awgn_sap(struct wlan_objmgr_pdev *pdev,
				      void *object, void *arg)
{
	struct wlan_objmgr_vdev *vdev = object;
	struct wlan_host_dcs_awgn_info *awgn_info = arg;
	enum phy_ch_width ch_width;
	enum phy_ch_width tgt_width = CH_WIDTH_INVALID;
	qdf_freq_t op_freq, cfreq0, cfreq1;
	qdf_freq_t tgt_freq = 0;
	QDF_STATUS status;
	uint8_t vdev_id;
	bool found;

	if (!vdev || !pdev)
		return;

	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_SAP_MODE)
		return;

	vdev_id = wlan_vdev_get_id(vdev);
	status = wlan_dcs_vdev_get_op_chan_info(vdev, &op_freq, &cfreq0, &cfreq1, &ch_width);
	if (QDF_IS_STATUS_ERROR(status))
		return;

	if (awgn_info->center_freq != op_freq) {
		dcs_debug("SAP-%d: freq not match rpt:%u - op:%u",
			  vdev_id, awgn_info->center_freq, op_freq);
		return;
	}

	found = wlan_dcs_get_max_no_intf_bw(awgn_info, &tgt_width);
	if (found) {
		if (ch_width <= tgt_width) {
			dcs_debug("SAP-%d: both freq and bw are unchanged",
				  vdev_id);
			return;
		}

		tgt_freq = op_freq;
	} else {
		wlan_dcs_sap_select_chan(vdev, awgn_info, &tgt_freq,
					 &tgt_width, true);
	}

	/* If no chan is selected, means to stop sap */
	dcs_debug("SAP-%d: target freq %u width %u",
		  vdev_id, tgt_freq, tgt_width);
	wlan_dcs_switch_chan(vdev, tgt_freq, tgt_width);
}

/**
 * wlan_dcs_awgnim_process() - process awgn IM
 * @psoc: psoc ptr
 * @pdev_id: pdev id
 * @awgn_info: pointer to awgn info
 *
 * This function triggers channel change for all STAs and SAPs, according
 * to AWGN info.
 *
 * Return: None.
 */
static void
wlan_dcs_awgn_process(struct wlan_objmgr_psoc *psoc, uint8_t pdev_id,
		      struct wlan_host_dcs_awgn_info *awgn_info)
{
	struct wlan_objmgr_pdev *pdev;

	if (!wlan_dcs_is_awgnim_valid(awgn_info)) {
		dcs_err("Invalid awgnim event");
		return;
	}

	pdev = wlan_objmgr_get_pdev_by_id(psoc, pdev_id, WLAN_DCS_ID);
	if (!pdev) {
		dcs_err("Invalid pdev id %d", pdev_id);
		return;
	}

	dcs_debug("pdev id %u width %u freq %u freq0 %u fre1 %u bitmap 0x%x",
		  pdev_id, awgn_info->channel_width, awgn_info->center_freq,
		  awgn_info->center_freq0, awgn_info->center_freq1,
		  awgn_info->chan_bw_intf_bitmap);

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_dcs_process_awgn_sta,
					  awgn_info, 0, WLAN_DCS_ID);

	wlan_objmgr_pdev_iterate_obj_list(pdev, WLAN_VDEV_OP,
					  wlan_dcs_process_awgn_sap,
					  awgn_info, 0, WLAN_DCS_ID);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_DCS_ID);
}

QDF_STATUS
wlan_dcs_process(struct wlan_objmgr_psoc *psoc,
		 struct wlan_host_dcs_event *event)
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
		dcs_debug("dcs_enable: %u, interference_type: %u, pdev_id: %u",
			  dcs_pdev_priv->dcs_host_params.dcs_enable,
			  event->dcs_param.interference_type,
			  event->dcs_param.pdev_id);

	switch (event->dcs_param.interference_type) {
	case WLAN_HOST_DCS_CWIM:
		break;
	case WLAN_HOST_DCS_WLANIM:
		if (!dcs_pdev_priv->dcs_host_params.dcs_enable)
			break;

		if (dcs_pdev_priv->dcs_host_params.dcs_enable &
		    WLAN_HOST_DCS_WLANIM)
			start_dcs_cbk_handler =
				wlan_dcs_wlan_interference_process(
							&event->wlan_stat,
							dcs_pdev_priv);
		if (dcs_pdev_priv->user_cb &&
		    dcs_pdev_priv->dcs_host_params.notify_user) {
			dcs_pdev_priv->dcs_host_params.notify_user = 0;
			dcs_pdev_priv->user_cb(dcs_pdev_priv->requestor_vdev_id,
				 &dcs_pdev_priv->dcs_im_stats.user_dcs_im_stats,
				 0);
		}
		if (start_dcs_cbk_handler)
			wlan_dcs_frequency_control(psoc,
						   dcs_pdev_priv,
						   event);
		break;
	case WLAN_HOST_DCS_AWGNIM:
		/* Skip frequency control for AWGNIM */
		wlan_dcs_awgn_process(psoc, event->dcs_param.pdev_id,
				      &event->awgn_info);
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
	wlan_dcs_set_algorithm_process(psoc, pdev_id, false);
}

void wlan_dcs_set_algorithm_process(struct wlan_objmgr_psoc *psoc,
				    uint32_t pdev_id,
				    bool dcs_algorithm_process)
{
	struct dcs_pdev_priv_obj *dcs_pdev_priv;

	dcs_pdev_priv = wlan_dcs_get_pdev_private_obj(psoc, pdev_id);
	if (!dcs_pdev_priv) {
		dcs_err("dcs pdev private object is null");
		return;
	}

	if (dcs_pdev_priv->dcs_host_params.force_disable_algorithm) {
		dcs_debug("dcs alorithm is disabled forcely");
		dcs_pdev_priv->dcs_host_params.dcs_algorithm_process = false;
		return;
	}

	dcs_pdev_priv->dcs_host_params.dcs_algorithm_process =
							dcs_algorithm_process;
}
