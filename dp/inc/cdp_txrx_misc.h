/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
/**
 * cdp_tx_non_std() - Allow the control-path SW to send data frames
 *
 * @soc - data path soc handle
 * @data_vdev - which vdev should transmit the tx data frames
 * @tx_spec - what non-standard handling to apply to the tx data frames
 * @msdu_list - NULL-terminated list of tx MSDUs
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
cdp_tx_non_std(ol_txrx_soc_handle soc, struct cdp_vdev *vdev,
		enum ol_tx_spec tx_spec, qdf_nbuf_t msdu_list)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		return NULL;
	}

	if (soc->ops->misc_ops->tx_non_std)
		return soc->ops->misc_ops->tx_non_std(
			vdev, tx_spec, msdu_list);
	return NULL;
}

/**
 * cdp_set_ibss_vdev_heart_beat_timer() - Update ibss vdev heart
 * beat timer
 * @soc - data path soc handle
 * @vdev - vdev handle
 * @timer_value_sec - new heart beat timer value
 *
 * Return: Old timer value set in vdev.
 */
static inline uint16_t
cdp_set_ibss_vdev_heart_beat_timer(ol_txrx_soc_handle soc,
		struct cdp_vdev *vdev, uint16_t timer_value_sec)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->misc_ops->set_ibss_vdev_heart_beat_timer)
		return soc->ops->misc_ops->set_ibss_vdev_heart_beat_timer(
			vdev, timer_value_sec);

	return 0;
}

/**
 * cdp_set_wisa_mode() - set wisa mode
 * @soc - data path soc handle
 * @vdev - vdev handle
 * @enable - enable or disable
 *
 * Return: QDF_STATUS_SUCCESS mode enable success
 */
static inline QDF_STATUS
cdp_set_wisa_mode(ol_txrx_soc_handle soc, struct cdp_vdev *vdev, bool enable)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->set_wisa_mode)
		return soc->ops->misc_ops->set_wisa_mode(vdev, enable);
	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_data_stall_cb_register() - register data stall callback
 * @soc - data path soc handle
 * @cb - callback function
 *
 * Return: QDF_STATUS_SUCCESS register success
 */
static inline QDF_STATUS cdp_data_stall_cb_register(ol_txrx_soc_handle soc,
						    data_stall_detect_cb cb)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->txrx_data_stall_cb_register)
		return soc->ops->misc_ops->txrx_data_stall_cb_register(cb);
	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_data_stall_cb_deregister() - de-register data stall callback
 * @soc - data path soc handle
 * @cb - callback function
 *
 * Return: QDF_STATUS_SUCCESS de-register success
 */
static inline QDF_STATUS cdp_data_stall_cb_deregister(ol_txrx_soc_handle soc,
						      data_stall_detect_cb cb)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->txrx_data_stall_cb_deregister)
		return soc->ops->misc_ops->txrx_data_stall_cb_deregister(cb);
	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_post_data_stall_event() - post data stall event
 * @soc - data path soc handle
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
				indicator, data_stall_type, pdev_id,
				vdev_id_bitmap, recovery_type);
}

/**
 * cdp_set_wmm_param() - set wmm parameter
 * @soc - data path soc handle
 * @pdev - device instance pointer
 * @wmm_param - wmm parameter
 *
 * Return: none
 */
static inline void
cdp_set_wmm_param(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		      struct ol_tx_wmm_param_t wmm_param)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->set_wmm_param)
		return soc->ops->misc_ops->set_wmm_param(
			pdev, wmm_param);

	return;
}

/**
 * cdp_runtime_suspend() - suspend
 * @soc - data path soc handle
 * @pdev - device instance pointer
 *
 * Return: QDF_STATUS_SUCCESS suspend success
 */
static inline QDF_STATUS cdp_runtime_suspend(ol_txrx_soc_handle soc,
		struct cdp_pdev *pdev)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->runtime_suspend)
		return soc->ops->misc_ops->runtime_suspend(pdev);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_runtime_resume() - resume
 * @soc - data path soc handle
 * @pdev - device instance pointer
 *
 * Return: QDF_STATUS_SUCCESS suspend success
 */
static inline QDF_STATUS cdp_runtime_resume(ol_txrx_soc_handle soc,
		struct cdp_pdev *pdev)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_INVAL;
	}

	if (soc->ops->misc_ops->runtime_resume)
		return soc->ops->misc_ops->runtime_resume(pdev);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_hl_tdls_flag_reset() - tdls flag reset
 * @soc - data path soc handle
 * @vdev - virtual interface handle pointer
 * @flag
 *
 * Return: none
 */
static inline void
cdp_hl_tdls_flag_reset(ol_txrx_soc_handle soc, struct cdp_vdev *vdev, bool flag)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->hl_tdls_flag_reset)
		return soc->ops->misc_ops->hl_tdls_flag_reset(vdev, flag);

	return;
}

/**
 * cdp_get_opmode() - get vdev operation mode
 * @soc - data path soc handle
 * @vdev - virtual interface instance
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
cdp_get_opmode(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return 0;
	}

	if (soc->ops->misc_ops->get_opmode)
		return soc->ops->misc_ops->get_opmode(vdev);
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
		return soc->ops->misc_ops->get_tx_ack_stats(vdev_id);
	return 0;
}

/**
 * cdp_bad_peer_txctl_set_setting() - TBD
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @enable -
 * @period -
 * @txq_limit -
 *
 * TBD
 *
 * Return: none
 */
static inline void
cdp_bad_peer_txctl_set_setting(ol_txrx_soc_handle soc, struct cdp_pdev *pdev,
		int enable, int period, int txq_limit)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->bad_peer_txctl_set_setting)
		return soc->ops->misc_ops->bad_peer_txctl_set_setting(pdev,
			enable, period, txq_limit);
	return;
}

/**
 * cdp_bad_peer_txctl_update_threshold() - TBD
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @level -
 * @tput_thresh -
 * @tx_limit -
 *
 * TBD
 *
 * Return: none
 */
static inline void
cdp_bad_peer_txctl_update_threshold(ol_txrx_soc_handle soc,
		struct cdp_pdev *pdev,
		int level, int tput_thresh, int tx_limit)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->bad_peer_txctl_update_threshold)
		return soc->ops->misc_ops->bad_peer_txctl_update_threshold(
			pdev, level, tput_thresh, tx_limit);
	return;
}

/**
 * cdp_mark_first_wakeup_packet() - set flag to indicate that
 *    fw is compatible for marking first packet after wow wakeup
 * @soc - data path soc handle
 * @value: 1 for enabled/ 0 for disabled
 *
 * Return: None
 */
static inline void cdp_mark_first_wakeup_packet(ol_txrx_soc_handle soc,
		uint8_t value)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->mark_first_wakeup_packet)
		return soc->ops->misc_ops->mark_first_wakeup_packet(value);
	return;
}


/**
 * cds_update_mac_id() - update mac_id for vdev
 * @soc - data path soc handle
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
		return soc->ops->misc_ops->update_mac_id(vdev_id, mac_id);
	return;
}

/**
 * cdp_flush_rx_frames() - flush cached rx frames
 * @soc - data path soc handle
 * @peer: peer
 * @drop: set flag to drop frames
 *
 * Return: None
 */
static inline void cdp_flush_rx_frames(ol_txrx_soc_handle soc, void *peer,
		bool drop)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->flush_rx_frames)
		return soc->ops->misc_ops->flush_rx_frames(peer, drop);
	return;
}

/*
 * cdp_get_intra_bss_fwd_pkts_count() - to get the total tx and rx packets
 *   that has been forwarded from txrx layer without going to upper layers.
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
			vdev_id, fwd_tx_packets, fwd_rx_packets);

	return 0;
}

/**
 * cdp_pkt_log_init() - API to initialize packet log
 * @handle: pdev handle
 * @scn: HIF context
 *
 * Return: void
 */
static inline void cdp_pkt_log_init(ol_txrx_soc_handle soc,
		struct cdp_pdev *pdev, void *scn)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->pkt_log_init)
		return soc->ops->misc_ops->pkt_log_init(pdev, scn);

	return;
}

/**
 * cdp_pkt_log_con_service() - API to connect packet log service
 * @handle: pdev handle
 * @scn: HIF context
 *
 * Return: void
 */
static inline void cdp_pkt_log_con_service(ol_txrx_soc_handle soc,
		struct cdp_pdev *pdev, void *scn)
{
	if (!soc || !soc->ops || !soc->ops->misc_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return;
	}

	if (soc->ops->misc_ops->pkt_log_con_service)
		return soc->ops->misc_ops->pkt_log_con_service(pdev, scn);

	return;
}
#endif /* _CDP_TXRX_MISC_H_ */
