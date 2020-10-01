/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_ipa.h
 * @brief Define the host data path IP Acceleraor API functions
 */
#ifndef _CDP_TXRX_IPA_H_
#define _CDP_TXRX_IPA_H_

#ifdef IPA_OFFLOAD
#ifdef CONFIG_IPA_WDI_UNIFIED_API
#include <qdf_ipa_wdi3.h>
#else
#include <qdf_ipa.h>
#endif
#include <cdp_txrx_cmn.h>
#include "cdp_txrx_handle.h"

/**
 * cdp_ipa_get_resource() - Get allocated WLAN resources for IPA data path
 * @soc - data path soc handle
 * @pdev_id - device instance id
 *
 * Get allocated WLAN resources for IPA data path
 *
 * return QDF_STATUS_SUCCESS
 */
static inline QDF_STATUS
cdp_ipa_get_resource(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_get_resource)
		return soc->ops->ipa_ops->ipa_get_resource(soc, pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_set_doorbell_paddr() - give IPA db paddr to FW
 * @soc - data path soc handle
 * @pdev_id - device instance id
 *
 * give IPA db paddr to FW
 *
 * return QDF_STATUS_SUCCESS
 */
static inline QDF_STATUS
cdp_ipa_set_doorbell_paddr(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_set_doorbell_paddr)
		return soc->ops->ipa_ops->ipa_set_doorbell_paddr(soc, pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_set_active() - activate/de-ctivate IPA offload path
 * @soc - data path soc handle
 * @pdev_id - device instance id
 * @uc_active - activate or de-activate
 * @is_tx - toggle tx or rx data path
 *
 * activate/de-ctivate IPA offload path
 *
 * return QDF_STATUS_SUCCESS
 */
static inline QDF_STATUS
cdp_ipa_set_active(ol_txrx_soc_handle soc, uint8_t pdev_id, bool uc_active,
		   bool is_tx)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_set_active)
		return soc->ops->ipa_ops->ipa_set_active(soc, pdev_id,
				uc_active, is_tx);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_op_response() - event handler from FW
 * @soc - data path soc handle
 * @pdev_id - device instance id
 * @op_msg - event contents from firmware
 *
 * event handler from FW
 *
 * return QDF_STATUS_SUCCESS
 */
static inline QDF_STATUS
cdp_ipa_op_response(ol_txrx_soc_handle soc, uint8_t pdev_id, uint8_t *op_msg)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_op_response)
		return soc->ops->ipa_ops->ipa_op_response(soc, pdev_id, op_msg);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_register_op_cb() - register event handler function pointer
 * @soc - data path soc handle
 * @pdev_id - device instance id
 * @op_cb - event handler callback function pointer
 * @usr_ctxt - user context to registered
 *
 * register event handler function pointer
 *
 * return QDF_STATUS_SUCCESS
 */
static inline QDF_STATUS
cdp_ipa_register_op_cb(ol_txrx_soc_handle soc, uint8_t pdev_id,
		       ipa_uc_op_cb_type op_cb, void *usr_ctxt)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_register_op_cb)
		return soc->ops->ipa_ops->ipa_register_op_cb(soc, pdev_id,
							     op_cb, usr_ctxt);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_get_stat() - get IPA data path stats from FW
 * @soc - data path soc handle
 * @pdev_id - device instance id
 *
 * get IPA data path stats from FW async
 *
 * return QDF_STATUS_SUCCESS
 */
static inline QDF_STATUS
cdp_ipa_get_stat(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_get_stat)
		return soc->ops->ipa_ops->ipa_get_stat(soc, pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_tx_send_ipa_data_frame() - send IPA data frame
 * @soc - data path soc handle
 * @vdev_id: vdev id
 * @skb: skb
 *
 * Return: skb/ NULL is for success
 */
static inline qdf_nbuf_t cdp_ipa_tx_send_data_frame(ol_txrx_soc_handle soc,
						    uint8_t vdev_id,
						    qdf_nbuf_t skb)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return skb;
	}

	if (soc->ops->ipa_ops->ipa_tx_data_frame)
		return soc->ops->ipa_ops->ipa_tx_data_frame(soc, vdev_id, skb);

	return skb;
}

/**
 * cdp_ipa_set_uc_tx_partition_base() - set tx packet partition base
 * @soc - data path soc handle
 * @cfg_pdev: physical device instance config
 * @value: partition base value
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_set_uc_tx_partition_base(ol_txrx_soc_handle soc,
				struct cdp_cfg *cfg_pdev, uint32_t value)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops || !cfg_pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_set_uc_tx_partition_base)
		soc->ops->ipa_ops->ipa_set_uc_tx_partition_base(cfg_pdev,
								value);

	return QDF_STATUS_SUCCESS;
}

#ifdef FEATURE_METERING
/**
 * cdp_ipa_uc_get_share_stats() - get Tx/Rx byte stats from FW
 * @soc - data path soc handle
 * @pdev_id: physical device instance number
 * @value: reset stats
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_uc_get_share_stats(ol_txrx_soc_handle soc, uint8_t pdev_id,
			   uint8_t value)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_uc_get_share_stats)
		return soc->ops->ipa_ops->ipa_uc_get_share_stats(soc, pdev_id,
								 value);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_uc_set_quota() - set quota limit to FW
 * @soc - data path soc handle
 * @pdev_id: physical device instance number
 * @value: quota limit bytes
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_uc_set_quota(ol_txrx_soc_handle soc, uint8_t pdev_id, uint64_t value)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_uc_set_quota)
		return soc->ops->ipa_ops->ipa_uc_set_quota(soc, pdev_id, value);

	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * cdp_ipa_enable_autonomy() - Enable autonomy RX data path
 * @soc: data path soc handle
 * @pdev_id: physical device instance number
 *
 * IPA Data path is enabled and resumed.
 * All autonomy data path elements are ready to deliver packet
 * All RX packet should routed to IPA_REO ring, then IPA can receive packet
 * from WLAN
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_enable_autonomy(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_enable_autonomy)
		return soc->ops->ipa_ops->ipa_enable_autonomy(soc, pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_disable_autonomy() - Disable autonomy RX data path
 * @soc: data path soc handle
 * @pdev_id: physical device instance number
 *
 * IPA Data path is enabled and resumed.
 * All autonomy datapath elements are ready to deliver packet
 * All RX packet should routed to IPA_REO ring, then IPA can receive packet
 * from WLAN
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_disable_autonomy(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (soc->ops->ipa_ops->ipa_disable_autonomy)
		return soc->ops->ipa_ops->ipa_disable_autonomy(soc, pdev_id);

	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_IPA_WDI_UNIFIED_API
/**
 * cdp_ipa_setup() - Setup and connect IPA pipes
 * @soc: data path soc handle
 * @pdev_id: handle to the device instance number
 * @ipa_i2w_cb: IPA to WLAN callback
 * @ipa_w2i_cb: WLAN to IPA callback
 * @ipa_wdi_meter_notifier_cb: IPA WDI metering callback
 * @ipa_desc_size: IPA descriptor size
 * @ipa_priv: handle to the HTT instance
 * @is_rm_enabled: Is IPA RM enabled or not
 * @tx_pipe_handle: pointer to Tx pipe handle
 * @rx_pipe_handle: pointer to Rx pipe handle
 * @is_smmu_enabled: Is SMMU enabled or not
 * @sys_in: parameters to setup sys pipe in mcc mode
 * @over_gsi: Is IPA using GSI
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_setup(ol_txrx_soc_handle soc, uint8_t pdev_id, void *ipa_i2w_cb,
	      void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
	      uint32_t ipa_desc_size, void *ipa_priv, bool is_rm_enabled,
	      uint32_t *tx_pipe_handle, uint32_t *rx_pipe_handle,
	      bool is_smmu_enabled, qdf_ipa_sys_connect_params_t *sys_in,
	      bool over_gsi)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_setup)
		return soc->ops->ipa_ops->ipa_setup(soc, pdev_id, ipa_i2w_cb,
						    ipa_w2i_cb,
						    ipa_wdi_meter_notifier_cb,
						    ipa_desc_size, ipa_priv,
						    is_rm_enabled,
						    tx_pipe_handle,
						    rx_pipe_handle,
						    is_smmu_enabled,
						    sys_in, over_gsi);

	return QDF_STATUS_SUCCESS;
}
#else /* CONFIG_IPA_WDI_UNIFIED_API */
/**
 * cdp_ipa_setup() - Setup and connect IPA pipes
 * @soc: data path soc handle
 * @pdev_id: handle to the device instance number
 * @ipa_i2w_cb: IPA to WLAN callback
 * @ipa_w2i_cb: WLAN to IPA callback
 * @ipa_wdi_meter_notifier_cb: IPA WDI metering callback
 * @ipa_desc_size: IPA descriptor size
 * @ipa_priv: handle to the HTT instance
 * @is_rm_enabled: Is IPA RM enabled or not
 * @tx_pipe_handle: pointer to Tx pipe handle
 * @rx_pipe_handle: pointer to Rx pipe handle
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_setup(ol_txrx_soc_handle soc, uint8_t pdev_id, void *ipa_i2w_cb,
	      void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
	      uint32_t ipa_desc_size, void *ipa_priv, bool is_rm_enabled,
	      uint32_t *tx_pipe_handle, uint32_t *rx_pipe_handle)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_setup)
		return soc->ops->ipa_ops->ipa_setup(soc, pdev_id, ipa_i2w_cb,
						    ipa_w2i_cb,
						    ipa_wdi_meter_notifier_cb,
						    ipa_desc_size, ipa_priv,
						    is_rm_enabled,
						    tx_pipe_handle,
						    rx_pipe_handle);

	return QDF_STATUS_SUCCESS;
}
#endif /* CONFIG_IPA_WDI_UNIFIED_API */

/**
 * cdp_ipa_cleanup() - Disconnect IPA pipes
 * @soc: data path soc handle
 * @pdev_id: handle to the device instance number
 * @tx_pipe_handle: Tx pipe handle
 * @rx_pipe_handle: Rx pipe handle
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_cleanup(ol_txrx_soc_handle soc, uint8_t pdev_id,
		uint32_t tx_pipe_handle, uint32_t rx_pipe_handle)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_cleanup)
		return soc->ops->ipa_ops->ipa_cleanup(soc, pdev_id,
						      tx_pipe_handle,
						      rx_pipe_handle);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_setup_iface() - Setup IPA header and register interface
 * @soc: data path soc handle
 * @ifname: Interface name
 * @mac_addr: Interface MAC address
 * @prod_client: IPA prod client type
 * @cons_client: IPA cons client type
 * @session_id: Session ID
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_setup_iface(ol_txrx_soc_handle soc, char *ifname, uint8_t *mac_addr,
		    qdf_ipa_client_type_t prod_client,
		    qdf_ipa_client_type_t cons_client,
		    uint8_t session_id, bool is_ipv6_enabled)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_setup_iface)
		return soc->ops->ipa_ops->ipa_setup_iface(ifname, mac_addr,
							  prod_client,
							  cons_client,
							  session_id,
							  is_ipv6_enabled);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_cleanup_iface() - Cleanup IPA header and deregister interface
 * @soc: data path soc handle
 * @ifname: Interface name
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_cleanup_iface(ol_txrx_soc_handle soc, char *ifname,
		      bool is_ipv6_enabled)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_cleanup_iface)
		return soc->ops->ipa_ops->ipa_cleanup_iface(ifname,
							    is_ipv6_enabled);

	return QDF_STATUS_SUCCESS;
}

 /**
 * cdp_ipa_uc_enable_pipes() - Enable and resume traffic on Tx/Rx pipes
 * @soc - data path soc handle
 * @pdev_id - device instance id
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_enable_pipes(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_enable_pipes)
		return soc->ops->ipa_ops->ipa_enable_pipes(soc, pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_uc_disable_pipes() - Suspend traffic and disable Tx/Rx pipes
 * @soc: data path soc handle
 * @pdev_id - device instance id
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
cdp_ipa_disable_pipes(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_disable_pipes)
		return soc->ops->ipa_ops->ipa_disable_pipes(soc, pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_set_perf_level() - Set IPA clock bandwidth based on data rates
 * @soc: data path soc handle
 * @client: WLAN Client ID
 * @max_supported_bw_mbps: Maximum bandwidth needed (in Mbps)
 *
 * Return: 0 on success, negative errno on error
 */
static inline QDF_STATUS
cdp_ipa_set_perf_level(ol_txrx_soc_handle soc, int client,
		       uint32_t max_supported_bw_mbps)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			"%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_set_perf_level)
		return soc->ops->ipa_ops->ipa_set_perf_level(client,
				max_supported_bw_mbps);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_rx_intrabss_fwd() - Perform intra-bss fwd for IPA RX path
 *
 * @soc: data path soc handle
 * @vdev_id: vdev id
 * @nbuf: pointer to skb of ethernet packet received from IPA RX path
 * @fwd_success: pointer to indicate if skb succeeded in intra-bss TX
 *
 * This function performs intra-bss forwarding for WDI 3.0 IPA RX path.
 *
 * Return: true if packet is intra-bss fwd-ed and no need to pass to
 *	   network stack. false if packet needs to be passed to network stack.
 */
static inline bool
cdp_ipa_rx_intrabss_fwd(ol_txrx_soc_handle soc, uint8_t vdev_id,
			qdf_nbuf_t nbuf, bool *fwd_success)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops || !fwd_success) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_rx_intrabss_fwd)
		return soc->ops->ipa_ops->ipa_rx_intrabss_fwd(soc, vdev_id,
							      nbuf,
							      fwd_success);

	/* Fall back to pass up to stack */
	return false;
}

/**
 * cdp_ipa_tx_buf_smmu_mapping() - Create SMMU mappings for Tx
 *				   buffers allocated to IPA
 * @soc: data path soc handle
 * @pdev_id: device instance id
 *
 * Create SMMU mappings for Tx buffers allocated to IPA
 *
 * return QDF_STATUS_SUCCESS
 */
static inline QDF_STATUS
cdp_ipa_tx_buf_smmu_mapping(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_tx_buf_smmu_mapping)
		return soc->ops->ipa_ops->ipa_tx_buf_smmu_mapping(soc, pdev_id);

	return QDF_STATUS_SUCCESS;
}

/**
 * cdp_ipa_tx_buf_smmu_unmapping() - Release SMMU mappings for Tx
 *				     buffers allocated to IPA
 * @soc: data path soc handle
 * @pdev_id: device instance id
 *
 * Release SMMU mappings for Tx buffers allocated to IPA
 *
 * return QDF_STATUS_SUCCESS
 */
static inline QDF_STATUS
cdp_ipa_tx_buf_smmu_unmapping(ol_txrx_soc_handle soc, uint8_t pdev_id)
{
	if (!soc || !soc->ops || !soc->ops->ipa_ops) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,
			  "%s invalid instance", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (soc->ops->ipa_ops->ipa_tx_buf_smmu_unmapping)
		return soc->ops->ipa_ops->ipa_tx_buf_smmu_unmapping(soc,
								    pdev_id);

	return QDF_STATUS_SUCCESS;
}
#endif /* IPA_OFFLOAD */

#endif /* _CDP_TXRX_IPA_H_ */

