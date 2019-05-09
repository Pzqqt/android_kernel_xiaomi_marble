/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_cfg.h
 * @brief Define the host data path configuration API functions
 */
#ifndef _CDP_TXRX_CFG_H_
#define _CDP_TXRX_CFG_H_
#include "cdp_txrx_handle.h"
/**
 * cdp_cfg_set_rx_fwd_disabled() - enable/disable rx forwarding
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @disable_rx_fwd - enable or disable rx forwarding
 *
 * enable/disable rx forwarding
 *
 * return NONE
 */
static inline void
cdp_cfg_set_rx_fwd_disabled(ol_txrx_soc_handle soc, struct cdp_cfg *cfg_pdev,
		uint8_t disable_rx_fwd)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->set_cfg_rx_fwd_disabled)
		return;

	soc->ops->cfg_ops->set_cfg_rx_fwd_disabled(cfg_pdev,
			disable_rx_fwd);
}

/**
 * cdp_cfg_set_packet_log_enabled() - enable/disable packet log
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @val - enable or disable packet log
 *
 * packet log enable or disable
 *
 * return NONE
 */
static inline void
cdp_cfg_set_packet_log_enabled(ol_txrx_soc_handle soc,
		struct cdp_cfg *cfg_pdev, uint8_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->set_cfg_packet_log_enabled)
		return;

	soc->ops->cfg_ops->set_cfg_packet_log_enabled(cfg_pdev,
				val);
}

/**
 * cdp_cfg_attach() - attach config module
 * @soc - data path soc handle
 * @osdev - os instance
 * @cfg_param - configuration parameter should be propagated
 *
 * Allocate configuration module instance, and propagate configuration values
 *
 * return soc configuration module instance
 */
static inline struct cdp_cfg
*cdp_cfg_attach(ol_txrx_soc_handle soc,
		qdf_device_t osdev, void *cfg_param)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return NULL;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->cfg_attach)
		return NULL;

	return soc->ops->cfg_ops->cfg_attach(osdev, cfg_param);
}

/**
 * cdp_cfg_vdev_rx_set_intrabss_fwd() - enable/disable intra bass forwarding
 * @soc - data path soc handle
 * @vdev - virtual interface instance
 * @val - enable or disable intra bss forwarding
 *
 * ap isolate, do not forward intra bss traffic
 *
 * return NONE
 */
static inline void
cdp_cfg_vdev_rx_set_intrabss_fwd(ol_txrx_soc_handle soc,
		struct cdp_vdev *vdev, bool val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->vdev_rx_set_intrabss_fwd)
		return;

	soc->ops->cfg_ops->vdev_rx_set_intrabss_fwd(vdev, val);
}

/**
 * cdp_cfg_is_rx_fwd_disabled() - get vdev rx forward
 * @soc - data path soc handle
 * @vdev - virtual interface instance
 *
 * Return rx forward feature enable status
 *
 * return 1 enabled
 *        0 disabled
 */
static inline uint8_t
cdp_cfg_is_rx_fwd_disabled(ol_txrx_soc_handle soc, struct cdp_vdev *vdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->is_rx_fwd_disabled)
		return 0;

	return soc->ops->cfg_ops->is_rx_fwd_disabled(vdev);

}

/**
 * cdp_cfg_tx_set_is_mgmt_over_wmi_enabled() - mgmt tx over wmi enable/disable
 * @soc - data path soc handle
 * @value - feature enable or disable
 *
 * Enable or disable management packet TX over WMI feature
 *
 * return None
 */
static inline void
cdp_cfg_tx_set_is_mgmt_over_wmi_enabled(ol_txrx_soc_handle soc,
		uint8_t value)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->tx_set_is_mgmt_over_wmi_enabled)
		return;

	soc->ops->cfg_ops->tx_set_is_mgmt_over_wmi_enabled(value);
}

/**
 * cdp_cfg_is_high_latency() - query data path is in high or low latency
 * @soc - data path soc handle
 * @pdev - data path device instance
 *
 * query data path is in high or low latency
 *
 * return 1 high latency data path, usb or sdio
 *        0 low latency data path
 */
static inline int
cdp_cfg_is_high_latency(ol_txrx_soc_handle soc, struct cdp_cfg *cfg_pdev)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return 0;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->is_high_latency)
		return 0;

	return soc->ops->cfg_ops->is_high_latency(cfg_pdev);
}

/**
 * cdp_cfg_set_flow_control_parameters() - set flow control params
 * @soc - data path soc handle
 * @cfg - dp config module instance
 * @param - parameters should set
 *
 * set flow control params
 *
 * return None
 */
static inline void
cdp_cfg_set_flow_control_parameters(ol_txrx_soc_handle soc,
		struct cdp_cfg *cfg_pdev, void *param)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->set_flow_control_parameters)
		return;

	soc->ops->cfg_ops->set_flow_control_parameters(cfg_pdev,
						       param);
}

/**
 * cdp_cfg_set_flow_steering - Set Rx flow steering config based on CFG ini
 *			config.
 *
 * @pdev - handle to the physical device
 * @val - 0 - disable, 1 - enable
 *
 * Return: None
 */
static inline void cdp_cfg_set_flow_steering(ol_txrx_soc_handle soc,
		struct cdp_cfg *cfg_pdev, uint8_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			"%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->set_flow_steering)
		return;

	soc->ops->cfg_ops->set_flow_steering(cfg_pdev, val);
}

static inline void cdp_cfg_get_max_peer_id(ol_txrx_soc_handle soc,
		struct cdp_cfg *cfg_pdev)
{
}

/**
 * cdp_cfg_set_ptp_rx_opt_enabled() - enable/disable ptp rx timestamping
 * @soc - data path soc handle
 * @pdev - data path device instance
 * @val - enable or disable packet log
 *
 * ptp rx timestamping enable or disable
 *
 * return NONE
 */
static inline void
cdp_cfg_set_ptp_rx_opt_enabled(ol_txrx_soc_handle soc,
			       struct cdp_cfg *cfg_pdev, uint8_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->set_ptp_rx_opt_enabled)
		return;

	soc->ops->cfg_ops->set_ptp_rx_opt_enabled(cfg_pdev, val);
}

/**
 * cdp_cfg_set_new_htt_msg_format() - set htt h2t msg feature
 * @soc - datapath soc handle
 * @val - enable or disable new htt h2t msg feature
 *
 * Enable whether htt h2t message length includes htc header length
 *
 * return NONE
 */
static inline void
cdp_cfg_set_new_htt_msg_format(ol_txrx_soc_handle soc,
			       uint8_t val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s invalid instance", __func__);
		return;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->set_new_htt_msg_format)
		return;

	soc->ops->cfg_ops->set_new_htt_msg_format(val);
}

/**
 * cdp_cfg_set_peer_unmap_conf_support() - set peer unmap conf feature
 * @soc - datapath soc handle
 * @val - enable or disable peer unmap conf feature
 *
 * Set if peer unmap confirmation feature is supported by both FW and in INI
 *
 * return NONE
 */
static inline void
cdp_cfg_set_peer_unmap_conf_support(ol_txrx_soc_handle soc, bool val)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s invalid instance", __func__);
		QDF_BUG(0);
		return;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->set_peer_unmap_conf_support)
		return;

	soc->ops->cfg_ops->set_peer_unmap_conf_support(val);
}

/**
 * cdp_cfg_get_peer_unmap_conf_support() - check peer unmap conf feature
 * @soc - datapath soc handle
 *
 * Check if peer unmap confirmation feature is enabled
 *
 * return true is peer unmap confirmation feature is enabled else false
 */
static inline bool
cdp_cfg_get_peer_unmap_conf_support(ol_txrx_soc_handle soc)
{
	if (!soc || !soc->ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_DEBUG,
			  "%s invalid instance", __func__);
		QDF_BUG(0);
		return false;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->get_peer_unmap_conf_support)
		return false;

	return soc->ops->cfg_ops->get_peer_unmap_conf_support();
}

static inline void
cdp_cfg_set_tx_compl_tsf64(ol_txrx_soc_handle soc,
			   uint8_t val)
{
	if (!soc || !soc->ops) {
		dp_debug("invalid instance");
		return;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->set_tx_compl_tsf64)
		return;

	soc->ops->cfg_ops->set_tx_compl_tsf64(val);
}

static inline bool
cdp_cfg_get_tx_compl_tsf64(ol_txrx_soc_handle soc)
{
	if (!soc || !soc->ops) {
		dp_debug("invalid instance");
		return false;
	}

	if (!soc->ops->cfg_ops ||
	    !soc->ops->cfg_ops->get_tx_compl_tsf64)
		return false;

	return soc->ops->cfg_ops->get_tx_compl_tsf64();
}

#endif /* _CDP_TXRX_CFG_H_ */
