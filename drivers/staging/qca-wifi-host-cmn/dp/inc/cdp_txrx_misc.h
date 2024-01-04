/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021,2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * @file cdp_txrx_misc.h
 * @brief Define the host data path miscellaneous API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_MISC_H_
#define _CDP_TXRX_MISC_H_

#include "cdp_txrx_handle.h"
#include <cdp_txrx_cmn.h>

/**
 * cdp_tx_non_std() - Allow the control-path SW to send data frames
 * @soc: data path soc handle
 * @vdev_id: id of vdev
 * @tx_spec: what non-standard handling to apply to the tx data frames
 * @msdu_list: NULL-terminated list of tx MSDUs
 *
 * Generally, all tx data frames come from the OS shim into the txrx layer.
 * However, there are rare cases such as TDLS messaging where the UMAC
 * control-path SW creates tx data frames.
 *  This UMAC SW can call this function to provide the tx data frames to
 *  the txrx layer.
 *  The UMAC SW can request a callback for these data frames after their
 *  transmission completes, by using the ol_txrx_data_tx_cb_set function
 *  to register a tx completion callback, and by specifying
 *  ol_tx_spec_no_free as the tx_spec arg when giving the frames to
 *  ol_tx_non_std.
 *  The MSDUs need to have the appropriate L2 header type (802.3 vs. 802.11),
 *  as specified by ol_cfg_frame_type().
 *
 *  Return: null - success, skb - failure
 */
static inline qdf_nbuf_t
cdp_tx_non_std(ol_txrx_soc_handle soc, uint8_t vdev_id,
	       enum ol_tx_spec tx_spec, qdf_nbuf_t msdu_list)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		dp_cdp_debug("invalid instance");
		return NULL;
	}

	if (soc->ops->misc_ops->tx_non_std)
		return soc->ops->misc_ops->tx_non_std(soc, vdev_id, tx_spec,
						      msdu_list);
	return NULL;
}

/**
 * cdp_set_ibss_vdev_heart_beat_timer() - Update ibss vdev heart
 * beat timer
 * @soc: data path soc handle
 * @vdev_id: id of vdev
 * @timer_value_sec: new heart beat timer value
 *
 * Return: Old timer value set in vdev.
 */
static inline uint16_t
cdp_set_ibss_vdev_heart_beat_timer(ol_txrx_soc_handle soc,
				   uint8_t vdev_id, uint16_t timer_value_sec)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->misc_ops->set_ibss_vdev_heart_beat_timer)
		return soc->ops->misc_ops->set_ibss_vdev_heart_beat_timer(
			soc, vdev_id, timer_value_sec);

	return 0;
}

/**
 * cdp_set_wisa_mode() - set wisa mode
 * @soc: data path soc handle
 * @vdev_id: vdev_id
 * @enable: enable or disable
 *
 * Return: QDF_STATUS_SUCCESS mode enable success
 */
static inline QDF_STATUS
cdp_set_wisa_mode(ol_txrx_soc_handle soc, uint8_t vdev_id, bool enable)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->set_wisa_mode)
		return soc->ops->misc_ops->set_wisa_mode(soc, vdev_id, enable);
	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_data_stall_cb_register() - register data stall callback
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 * @cb: callback function
 *
 * Return: QDF_STATUS_SUCCESS register success
 */
static inline QDF_STATUS cdp_data_stall_cb_register(ol_txrx_soc_handle soc,
						    uint8_t pdev_id,
						    data_stall_detect_cb cb)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->txrx_data_stall_cb_register)
		return soc->ops->misc_ops->txrx_data_stall_cb_register(
							soc, pdev_id, cb);
	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_data_stall_cb_deregister() - de-register data stall callback
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 * @cb - callback function
 *
 * Return: QDF_STATUS_SUCCESS de-register success
 */
static inline QDF_STATUS cdp_data_stall_cb_deregister(ol_txrx_soc_handle soc,
						      uint8_t pdev_id,
						      data_stall_detect_cb cb)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->txrx_data_stall_cb_deregister)
		return soc->ops->misc_ops->txrx_data_stall_cb_deregister(
							soc, pdev_id, cb);
	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_post_data_stall_event() - post data stall event
 * @soc: data path soc handle
 * @indicator: Module triggering data stall
 * @data_stall_type: data stall event type
 * @pdev_id: pdev id
 * @vdev_id_bitmap: vdev id bitmap
 * @recovery_type: data stall recovery type
 *
 * Return: None
 */
static inline void
cdp_post_data_stall_event(ol_txrx_soc_handle soc,
			  enum data_stall_log_event_indicator indicator,
			  enum data_stall_log_event_type data_stall_type,
			  uint32_t pdev_id, uint32_t vdev_id_bitmap,
			  enum data_stall_log_recovery_type recovery_type)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->misc_ops ||
	    !soc->ops->misc_ops->txrx_post_data_stall_event)
		return;

	soc->ops->misc_ops->txrx_post_data_stall_event(
				soc, indicator, data_stall_type, pdev_id,
				vdev_id_bitmap, recovery_type);
}

/**
 * cdp_set_wmm_param() - set wmm parameter
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 * @wmm_param: wmm parameter
 *
 * Return: none
 */
static inline void
cdp_set_wmm_param(ol_txrx_soc_handle soc, uint8_t pdev_id,
		  struct ol_tx_wmm_param_t wmm_param)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->set_wmm_param)
		return soc->ops->misc_ops->set_wmm_param(soc, pdev_id,
							 wmm_param);

	return;
}

/**
 * cdp_runtime_suspend() - suspend
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 *
 * Return: QDF_STATUS_SUCCESS suspend success
 */
static inline QDF_STATUS cdp_runtime_suspend(ol_txrx_soc_handle soc,
					     uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->runtime_suspend)
		return soc->ops->misc_ops->runtime_suspend(soc, pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_runtime_resume() - resume
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 *
 * Return: QDF_STATUS_SUCCESS suspend success
 */
static inline QDF_STATUS cdp_runtime_resume(ol_txrx_soc_handle soc,
					    uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->runtime_resume)
		return soc->ops->misc_ops->runtime_resume(soc, pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_hl_tdls_flag_reset() - tdls flag reset
 * @soc: data path soc handle
 * @vdev_id: id of vdev
 * @flag: flag indicating to set/reset tdls
 *
 * Return: none
 */
static inline void
cdp_hl_tdls_flag_reset(ol_txrx_soc_handle soc, uint8_t vdev_id, bool flag)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->hl_tdls_flag_reset)
		return soc->ops->misc_ops->hl_tdls_flag_reset(soc, vdev_id,
							      flag);

	return;
}

/**
 * cdp_get_opmode() - get vdev operation mode
 * @soc: data path soc handle
 * @vdev_id: id of vdev
 *
 * Return virtual device operational mode
 *      op_mode_ap,
 *      op_mode_ibss,
 *      op_mode_sta,
 *      op_mode_monitor,
 *      op_mode_ocb,
 *
 * return interface id
 *        0 unknown interface
 */
static inline int
cdp_get_opmode(ol_txrx_soc_handle soc, uint8_t vdev_id)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->misc_ops->get_opmode)
		return soc->ops->misc_ops->get_opmode(soc, vdev_id);

	return 0;
}

/**
 * cdp_get_vdev_id() - get vdev id
 * @soc - data path soc handle
 * @vdev - virtual interface instance
 *
 * get virtual interface id
 *
 * return interface id
 *        0 unknown interface
 */
static inline uint16_t
cdp_get_vdev_id(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->misc_ops->get_vdev_id)
		return soc->ops->misc_ops->get_vdev_id(vdev);
	return 0;
}

/**
 * cdp_get_tx_ack_stats() - get tx ack count for vdev
 * @soc - data path soc handle
 * @vdev_id - vdev id
 *
 * return tx ack count
 *          0 invalid count
 */
static inline uint32_t
cdp_get_tx_ack_stats(ol_txrx_soc_handle soc, uint8_t vdev_id)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->misc_ops->get_tx_ack_stats)
		return soc->ops->misc_ops->get_tx_ack_stats(soc, vdev_id);

	return 0;
}

/**
 * cdp_bad_peer_txctl_set_setting() - Set peer timer balance parameters
 * @soc: data path soc handle
 * @pdev_id: id of datapath pdev handle
 * @enable: enable/disable peer balance state
 * @period: balance timer period for peer
 * @txq_limit: txp limit for peer
 *
 * Return: none
 */
static inline void
cdp_bad_peer_txctl_set_setting(ol_txrx_soc_handle soc, uint8_t pdev_id,
			       int enable, int period, int txq_limit)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->bad_peer_txctl_set_setting)
		return soc->ops->misc_ops->bad_peer_txctl_set_setting(
					soc, pdev_id, enable, period,
					txq_limit);
	return;
}

/**
 * cdp_bad_peer_txctl_update_threshold() - TBD
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 * @level: index of the threshold configuration
 * @tput_thresh: peer balance throughput threshold
 * @tx_limit: peer balance tx limit threshold
 *
 * TBD
 *
 * Return: none
 */
static inline void
cdp_bad_peer_txctl_update_threshold(ol_txrx_soc_handle soc,
				    uint8_t pdev_id, int level,
				    int tput_thresh, int tx_limit)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->bad_peer_txctl_update_threshold)
		return soc->ops->misc_ops->bad_peer_txctl_update_threshold(
			soc, pdev_id, level, tput_thresh, tx_limit);
	return;
}

/**
 * cdp_mark_first_wakeup_packet() - set flag to indicate that
 *    fw is compatible for marking first packet after wow wakeup
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 * @value: 1 for enabled/ 0 for disabled
 *
 * Return: None
 */
static inline void cdp_mark_first_wakeup_packet(ol_txrx_soc_handle soc,
						uint8_t pdev_id, uint8_t value)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->mark_first_wakeup_packet)
		return soc->ops->misc_ops->mark_first_wakeup_packet(
							soc, pdev_id, value);
	return;
}


/**
 * cds_update_mac_id() - update mac_id for vdev
 * @psoc: data path soc handle
 * @vdev_id: vdev id
 * @mac_id: mac id
 *
 * Return: none
 */
static inline void cdp_update_mac_id(void *psoc, uint8_t vdev_id,
				     uint8_t mac_id)
{
	ol_txrx_soc_handle soc = psoc;

	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->update_mac_id)
		return soc->ops->misc_ops->update_mac_id(soc, vdev_id, mac_id);
	return;
}

/**
 * cdp_flush_rx_frames() - flush cached rx frames
 * @soc: data path soc handle
 * @pdev_id: datapath pdev identifier
 * @peer_mac: peer mac address
 * @drop: set flag to drop frames
 *
 * Return: None
 */
static inline void cdp_flush_rx_frames(ol_txrx_soc_handle soc, uint8_t pdev_id,
				       uint8_t *peer_mac, bool drop)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->flush_rx_frames)
		return soc->ops->misc_ops->flush_rx_frames(soc, pdev_id,
							   peer_mac, drop);
	return;
}

/*
 * cdp_get_intra_bss_fwd_pkts_count() - to get the total tx and rx packets
 *   that has been forwarded from txrx layer without going to upper layers.
 * @soc: Datapath soc handle
 * @vdev_id: vdev id
 * @fwd_tx_packets: pointer to forwarded tx packets count parameter
 * @fwd_rx_packets: pointer to forwarded rx packets count parameter
 *
 * Return: status -> A_OK - success, A_ERROR - failure
 */
static inline A_STATUS cdp_get_intra_bss_fwd_pkts_count(
		ol_txrx_soc_handle soc, uint8_t vdev_id,
		uint64_t *fwd_tx_packets, uint64_t *fwd_rx_packets)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->misc_ops->get_intra_bss_fwd_pkts_count)
		return soc->ops->misc_ops->get_intra_bss_fwd_pkts_count(
			soc, vdev_id, fwd_tx_packets, fwd_rx_packets);

	return 0;
}

/**
 * cdp_pkt_log_init() - API to initialize packet log
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 * @scn: HIF context
 *
 * Return: void
 */
static inline void cdp_pkt_log_init(ol_txrx_soc_handle soc,
				    uint8_t pdev_id, void *scn)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->pkt_log_init)
		return soc->ops->misc_ops->pkt_log_init(soc, pdev_id, scn);

	return;
}

/**
 * cdp_pkt_log_con_service() - API to connect packet log service
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 * @scn: HIF context
 *
 * Return: void
 */
static inline void cdp_pkt_log_con_service(ol_txrx_soc_handle soc,
					   uint8_t pdev_id, void *scn)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->pkt_log_con_service)
		return soc->ops->misc_ops->pkt_log_con_service(
						soc, pdev_id, scn);

	return;
}

/**
 * cdp_pkt_log_exit() - API to cleanup packet log info
 * @soc: data path soc handle
 * @pdev_id: id of data path pdev handle
 *
 * Return: void
 */
static inline void cdp_pkt_log_exit(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->pkt_log_exit)
		return soc->ops->misc_ops->pkt_log_exit(soc, pdev_id);

	return;
}

/**
 * cdp_get_num_rx_contexts() - API to get the number of RX contexts
 * @soc: soc handle
 *
 * Return: number of RX contexts
 */
static inline int cdp_get_num_rx_contexts(ol_txrx_soc_handle soc)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->misc_ops->get_num_rx_contexts)
		return soc->ops->misc_ops->get_num_rx_contexts(soc);

	return 0;
}

/**
 * cdp_register_packetdump_cb() - API to register packetdump callback
 *
 * Register TX/RX callback for data packets, during connection. And per packet
 * stats will be passed to user-space by @tx_cb/@rx_cb.
 *
 * @soc: soc handle
 * @pdev_id: id of data path pdev handle
 * @tx_cb: tx packet callback
 * @rx_cb: rx packet callback
 *
 * Return: void
 */
static inline void cdp_register_packetdump_cb(ol_txrx_soc_handle soc,
					      uint8_t pdev_id,
					      ol_txrx_pktdump_cb tx_cb,
					      ol_txrx_pktdump_cb rx_cb)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->register_pktdump_cb)
		return soc->ops->misc_ops->register_pktdump_cb(
						soc, pdev_id, tx_cb, rx_cb);
}

/**
 * cdp_deregister_packetdump_cb() - API to unregister packetdump callback
 *
 * Deregister callback for TX/RX data packets.
 *
 * @soc: soc handle
 * @pdev_id: id of data path pdev handle
 *
 * Return: void
 */
static inline void cdp_deregister_packetdump_cb(ol_txrx_soc_handle soc,
						uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->unregister_pktdump_cb)
		return soc->ops->misc_ops->unregister_pktdump_cb(soc, pdev_id);
}

typedef void (*rx_mic_error_callback)(struct cdp_ctrl_objmgr_psoc *psoc,
				uint8_t pdev_id,
				struct cdp_rx_mic_err_info *info);

/**
 * cdp_register_rx_mic_error_ind_handler() - API to register mic error
 *                                           indication handler
 *
 * @soc: soc handle
 * @rx_mic_cb: rx mic error indication callback
 *
 * Return: void
 */
static inline void
cdp_register_rx_mic_error_ind_handler(ol_txrx_soc_handle soc,
				      rx_mic_error_callback rx_mic_cb)
{
	if (!soc || !soc->ol_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	soc->ol_ops->rx_mic_error = rx_mic_cb;
}

typedef void (*rx_refill_thread_sched_cb)(ol_txrx_soc_handle soc);

/**
 * cdp_register_rx_refill_thread_sched_handler() - API to register RX refill
 *                                                 thread schedule handler
 *
 * @soc: soc handle
 *
 * Return: void
 */
static inline void
cdp_register_rx_refill_thread_sched_handler(ol_txrx_soc_handle soc,
					    rx_refill_thread_sched_cb rx_sched_cb)
{
	if (!soc || !soc->ol_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	soc->ol_ops->dp_rx_sched_refill_thread = rx_sched_cb;
}

/**
 * cdp_pdev_reset_driver_del_ack() - reset driver TCP delayed ack flag
 * @soc: data path soc handle
 * @pdev_id: pdev id
 *
 * Return: none
 */
static inline void cdp_pdev_reset_driver_del_ack(void *psoc,
						 uint8_t pdev_id)
{
	ol_txrx_soc_handle soc = psoc;

	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->pdev_reset_driver_del_ack)
		return soc->ops->misc_ops->pdev_reset_driver_del_ack(soc,
								     pdev_id);
}

/*
 * cdp_vdev_set_driver_del_ack_enable() - set driver delayed ack enabled flag
 * @soc: data path soc handle
 * @vdev_id: vdev id
 * @rx_packets: number of rx packets
 * @time_in_ms: time in ms
 * @high_th: high threshold
 * @low_th: low threshold
 *
 * Return: none
 */
static inline void cdp_vdev_set_driver_del_ack_enable(ol_txrx_soc_handle soc,
						      uint8_t vdev_id,
						      unsigned long rx_packets,
						      uint32_t time_in_ms,
						      uint32_t high_th,
						      uint32_t low_th)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->vdev_set_driver_del_ack_enable)
		return soc->ops->misc_ops->vdev_set_driver_del_ack_enable(
			soc, vdev_id, rx_packets, time_in_ms, high_th, low_th);
}

static inline void cdp_vdev_set_bundle_require_flag(ol_txrx_soc_handle soc,
						    uint8_t vdev_id,
						    unsigned long tx_bytes,
						    uint32_t time_in_ms,
						    uint32_t high_th,
						    uint32_t low_th)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->vdev_set_bundle_require_flag)
		return soc->ops->misc_ops->vdev_set_bundle_require_flag(
			vdev_id, tx_bytes, time_in_ms, high_th, low_th);
}

static inline void cdp_pdev_reset_bundle_require_flag(ol_txrx_soc_handle soc,
						      uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->pdev_reset_bundle_require_flag)
		return soc->ops->misc_ops->pdev_reset_bundle_require_flag(
								soc, pdev_id);
}

/**
 * cdp_txrx_ext_stats_request(): request dp tx and rx extended stats
 * @soc: soc handle
 * @pdev_id: pdev id
 * @req: stats request structure to fill
 *
 * return: status
 */
static inline QDF_STATUS
cdp_txrx_ext_stats_request(ol_txrx_soc_handle soc, uint8_t pdev_id,
			   struct cdp_txrx_ext_stats *req)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops || !req) {
		dp_cdp_debug("Invalid Instance:");
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->txrx_ext_stats_request)
		return soc->ops->misc_ops->txrx_ext_stats_request(soc, pdev_id,
								  req);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_request_rx_hw_stats(): request rx hw stats
 * @soc: soc handle
 * @vdev_id: vdev id
 *
 * return: none
 */
static inline QDF_STATUS
cdp_request_rx_hw_stats(ol_txrx_soc_handle soc, uint8_t vdev_id)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		dp_cdp_debug("Invalid Instance:");
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->request_rx_hw_stats)
		return soc->ops->misc_ops->request_rx_hw_stats(soc, vdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_reset_rx_hw_ext_stats(): reset rx hw ext stats
 * @soc: soc handle
 *
 * Return: none
 */
static inline void
cdp_reset_rx_hw_ext_stats(ol_txrx_soc_handle soc)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		dp_cdp_debug("Invalid Instance");
		return;
	}

	if (soc->ops->misc_ops->reset_rx_hw_ext_stats)
		soc->ops->misc_ops->reset_rx_hw_ext_stats(soc);
}

/**
 * cdp_vdev_inform_ll_conn() - Inform DP about the low latency connection
 * @soc: soc handle
 * @vdev_id: vdev id
 * @action: Action to be performed (Add/Delete)
 *
 * Returns: QDF_STATUS
 */
static inline QDF_STATUS
cdp_vdev_inform_ll_conn(ol_txrx_soc_handle soc, uint8_t vdev_id,
			enum vdev_ll_conn_actions action)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		dp_cdp_debug("Invalid Instance:");
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->vdev_inform_ll_conn)
		return soc->ops->misc_ops->vdev_inform_ll_conn(soc, vdev_id,
							       action);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_soc_set_swlm_enable() - Enable or disable software latency manager
 * @soc: soc handle
 * @value: value (enable/disable)
 *
 * Returns: QDF_STATUS
 */
static inline QDF_STATUS
cdp_soc_set_swlm_enable(ol_txrx_soc_handle soc, uint8_t value)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		dp_cdp_debug("Invalid Instance:");
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->set_swlm_enable)
		return soc->ops->misc_ops->set_swlm_enable(soc, value);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_soc_is_swlm_enabled() - Check if the software latency manager is
 *			       enabled or not
 * @soc: soc handle
 *
 * Returns: 1 if enabled, 0 if disabled
 */
static inline uint8_t
cdp_soc_is_swlm_enabled(ol_txrx_soc_handle soc)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		dp_cdp_debug("Invalid Instance:");
		return 0;
	}

	if (soc->ops->misc_ops->is_swlm_enabled)
		return soc->ops->misc_ops->is_swlm_enabled(soc);

	return 0;
}

/**
 * cdp_display_txrx_hw_info() - Dump the DP rings info
 * @soc: soc handle
 *
 * Return: none
 */
static inline void
cdp_display_txrx_hw_info(ol_txrx_soc_handle soc)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		dp_cdp_debug("Invalid Instance:");
		return;
	}

	if (soc->ops->misc_ops->display_txrx_hw_info)
		return soc->ops->misc_ops->display_txrx_hw_info(soc);
}

/**
 * cdp_get_tx_rings_grp_bitmap() - Get tx rings grp bitmap
 * @soc: soc handle
 *
 * Return: tx rings bitmap
 */
static inline uint32_t
cdp_get_tx_rings_grp_bitmap(ol_txrx_soc_handle soc)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		dp_cdp_debug("Invalid Instance:");
		return 0;
	}

	if (soc->ops->misc_ops->get_tx_rings_grp_bitmap)
		return soc->ops->misc_ops->get_tx_rings_grp_bitmap(soc);

	return 0;
}

#ifdef WLAN_FEATURE_PEER_TXQ_FLUSH_CONF
/**
 * cdp_set_peer_txq_flush_config() - Set the peer txq flush configuration
 * @soc: Opaque handle to the DP soc object
 * @vdev_id: VDEV identifier
 * @mac: MAC address of the peer
 * @ac: access category mask
 * @tid: TID mask
 * @policy: Flush policy
 *
 * Return: 0 on success, errno on failure
 */
static inline int
cdp_set_peer_txq_flush_config(ol_txrx_soc_handle soc, uint8_t vdev_id,
			      uint8_t *mac, uint8_t ac, uint32_t tid,
			      enum cdp_peer_txq_flush_policy policy)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops || !mac) {
		dp_cdp_debug("Invalid parameters");
		return 0;
	}

	if (soc->ops->misc_ops->set_peer_txq_flush_config) {
		return soc->ops->misc_ops->set_peer_txq_flush_config(soc,
								     vdev_id,
								     mac, ac,
								     tid,
								     policy);
	}

	return 0;
}
#endif /* WLAN_FEATURE_PEER_TXQ_FLUSH_CONF */
#endif /* _CDP_TXRX_MISC_H_ */
