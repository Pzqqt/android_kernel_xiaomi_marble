/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
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

#ifdef IPA_OFFLOAD

#include <ipa_wdi3.h>
#include <qdf_types.h>
#include <qdf_lock.h>
#include <hal_api.h>
#include <hif.h>
#include <htt.h>
#include <wdi_event.h>
#include <queue.h>
#include "dp_types.h"
#include "dp_tx.h"
#include "dp_ipa.h"

/**
 * dp_ipa_uc_get_resource() - Client request resource information
 * @ppdev - handle to the device instance
 *
 *  IPA client will request IPA UC related resource information
 *  Resource information will be distributed to IPA module
 *  All of the required resources should be pre-allocated
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_get_resource(struct cdp_pdev *ppdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_ipa_resources *ipa_res = &pdev->ipa_resource;

	ipa_res->tx_ring_base_paddr =
		soc->ipa_uc_tx_rsc.ipa_tcl_ring_base_paddr;
	ipa_res->tx_ring_size =
		soc->ipa_uc_tx_rsc.ipa_tcl_ring_size;
	ipa_res->tx_num_alloc_buffer =
		(uint32_t)soc->ipa_uc_tx_rsc.alloc_tx_buf_cnt;

	ipa_res->tx_comp_ring_base_paddr =
		soc->ipa_uc_tx_rsc.ipa_wbm_ring_base_paddr;
	ipa_res->tx_comp_ring_size =
		soc->ipa_uc_tx_rsc.ipa_wbm_ring_size;

	ipa_res->rx_rdy_ring_base_paddr =
		soc->ipa_uc_rx_rsc.ipa_reo_ring_base_paddr;
	ipa_res->rx_rdy_ring_size =
		soc->ipa_uc_rx_rsc.ipa_reo_ring_size;

	ipa_res->rx_refill_ring_base_paddr =
		soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_base_paddr;
	ipa_res->rx_refill_ring_size =
		soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_ring_size;

	if ((0 == ipa_res->tx_comp_ring_base_paddr) ||
			(0 == ipa_res->rx_rdy_ring_base_paddr))
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_set_doorbell_paddr () - Set doorbell register physical address to SRNG
 * @ppdev - handle to the device instance
 *
 * Set TX_COMP_DOORBELL register physical address to WBM Head_Ptr_MemAddr_LSB
 * Set RX_READ_DOORBELL register physical address to REO Head_Ptr_MemAddr_LSB
 *
 * Return: none
 */
QDF_STATUS dp_ipa_set_doorbell_paddr(struct cdp_pdev *ppdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_ipa_resources *ipa_res = &pdev->ipa_resource;

	hal_srng_set_hp_paddr(soc->tx_comp_ring[IPA_TX_COMP_RING_IDX].
			      hal_srng, ipa_res->tx_comp_doorbell_paddr);
	hal_srng_set_hp_paddr(soc->reo_dest_ring[IPA_REO_DEST_RING_IDX].
			      hal_srng, ipa_res->rx_ready_doorbell_paddr);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_op_response() - Handle OP command response from firmware
 * @ppdev - handle to the device instance
 * @op_msg: op response message from firmware
 *
 * Return: none
 */
QDF_STATUS dp_ipa_op_response(struct cdp_pdev *ppdev, uint8_t *op_msg)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;

	if (pdev->ipa_uc_op_cb) {
		pdev->ipa_uc_op_cb(op_msg, pdev->usr_ctxt);
	} else {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "%s: IPA callback function is not registered", __func__);
		qdf_mem_free(op_msg);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_register_op_cb() - Register OP handler function
 * @ppdev - handle to the device instance
 * @op_cb: handler function pointer
 *
 * Return: none
 */
QDF_STATUS dp_ipa_register_op_cb(struct cdp_pdev *ppdev,
				 ipa_uc_op_cb_type op_cb,
				 void *usr_ctxt)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;

	pdev->ipa_uc_op_cb = op_cb;
	pdev->usr_ctxt = usr_ctxt;

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_get_stat() - Get firmware wdi status
 * @ppdev - handle to the device instance
 *
 * Return: none
 */
QDF_STATUS dp_ipa_get_stat(struct cdp_pdev *ppdev)
{
	/* TBD */
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_send_ipa_data_frame() - send IPA data frame
 * @vdev: vdev
 * @skb: skb
 *
 * Return: skb/ NULL is for success
 */
qdf_nbuf_t dp_tx_send_ipa_data_frame(struct cdp_vdev *vdev, qdf_nbuf_t skb)
{
	qdf_nbuf_t ret;

	/* Terminate the (single-element) list of tx frames */
	qdf_nbuf_set_next(skb, NULL);
	ret = dp_tx_send((struct dp_vdev_t *)vdev, skb);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Failed to tx");
		return ret;
	}

	return NULL;
}

/**
 * dp_ipa_enable_autonomy() – Enable autonomy RX path
 * @pdev - handle to the device instance
 *
 * Set all RX packet route to IPA REO ring
 * Program Destination_Ring_Ctrl_IX_0 REO register to point IPA REO ring
 * Return: none
 */
QDF_STATUS dp_ipa_enable_autonomy(struct cdp_pdev *ppdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;
	struct dp_soc *soc = pdev->soc;
	uint32_t remap_val;

	/* Call HAL API to remap REO rings to REO2IPA ring */
	remap_val = HAL_REO_REMAP_VAL(REO_REMAP_TCL, REO_REMAP_TCL) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW1, REO_REMAP_SW4) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW2, REO_REMAP_SW4) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW3, REO_REMAP_SW4) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW4, REO_REMAP_SW4) |
		    HAL_REO_REMAP_VAL(REO_REMAP_RELEASE, REO_REMAP_RELEASE) |
		    HAL_REO_REMAP_VAL(REO_REMAP_FW, REO_REMAP_FW) |
		    HAL_REO_REMAP_VAL(REO_REMAP_UNUSED, REO_REMAP_FW);
	hal_reo_remap_IX0(soc->hal_soc, remap_val);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_disable_autonomy() – Disable autonomy RX path
 * @ppdev - handle to the device instance
 *
 * Disable RX packet routing to IPA REO
 * Program Destination_Ring_Ctrl_IX_0 REO register to disable
 * Return: none
 */
QDF_STATUS dp_ipa_disable_autonomy(struct cdp_pdev *ppdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;
	struct dp_soc *soc = pdev->soc;
	uint32_t remap_val;

	/* Call HAL API to remap REO rings to REO2IPA ring */
	remap_val = HAL_REO_REMAP_VAL(REO_REMAP_TCL, REO_REMAP_TCL) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW1, REO_REMAP_SW1) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW2, REO_REMAP_SW2) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW3, REO_REMAP_SW3) |
		    HAL_REO_REMAP_VAL(REO_REMAP_SW4, REO_REMAP_SW2) |
		    HAL_REO_REMAP_VAL(REO_REMAP_RELEASE, REO_REMAP_RELEASE) |
		    HAL_REO_REMAP_VAL(REO_REMAP_FW, REO_REMAP_FW) |
		    HAL_REO_REMAP_VAL(REO_REMAP_UNUSED, REO_REMAP_FW);
	hal_reo_remap_IX0(soc->hal_soc, remap_val);

	return QDF_STATUS_SUCCESS;
}

/* This should be configurable per H/W configuration enable status */
#define L3_HEADER_PADDING	2

/**
 * dp_ipa_setup() - Setup and connect IPA pipes
 * @ppdev - handle to the device instance
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
QDF_STATUS dp_ipa_setup(struct cdp_pdev *ppdev, void *ipa_i2w_cb,
			void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
			uint32_t ipa_desc_size, void *ipa_priv,
			bool is_rm_enabled, uint32_t *tx_pipe_handle,
			uint32_t *rx_pipe_handle)
{
	struct dp_pdev *pdev = (struct dp_pdev *)ppdev;
	struct dp_soc *soc = pdev->soc;
	struct dp_ipa_resources *ipa_res = &pdev->ipa_resource;
	struct ipa_wdi3_setup_info tx;
	struct ipa_wdi3_setup_info rx;
	struct ipa_wdi3_conn_in_params pipe_in;
	struct ipa_wdi3_conn_out_params pipe_out;
	int ret;

	qdf_mem_zero(&pipe_in, sizeof(struct ipa_wdi3_conn_in_params));
	qdf_mem_zero(&pipe_out, sizeof(struct ipa_wdi3_conn_out_params));

	/* TX PIPE */
	/**
	 * Transfer Ring: WBM Ring
	 * Transfer Ring Doorbell PA: WBM Tail Pointer Address
	 * Event Ring: TCL ring
	 * Event Ring Doorbell PA: TCL Head Pointer Address
	 */
	tx.ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
	tx.ipa_ep_cfg.hdr.hdr_len = DP_IPA_UC_WLAN_TX_HDR_LEN;
	tx.ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid = 0;
	tx.ipa_ep_cfg.hdr.hdr_ofst_pkt_size = 0;
	tx.ipa_ep_cfg.hdr.hdr_additional_const_len = 0;
	tx.ipa_ep_cfg.mode.mode = IPA_BASIC;
	tx.ipa_ep_cfg.hdr_ext.hdr_little_endian = true;
	tx.client = IPA_CLIENT_WLAN1_CONS;
	tx.transfer_ring_base_pa = ipa_res->tx_comp_ring_base_paddr;
	tx.transfer_ring_size = ipa_res->tx_comp_ring_size;
	tx.transfer_ring_doorbell_pa = /* WBM Tail Pointer Address */
		soc->ipa_uc_tx_rsc.ipa_wbm_tp_paddr;
	tx.event_ring_base_pa = ipa_res->tx_ring_base_paddr;
	tx.event_ring_size = ipa_res->tx_ring_size;
	tx.event_ring_doorbell_pa = /* TCL Head Pointer Address */
		soc->ipa_uc_tx_rsc.ipa_tcl_hp_paddr;
	tx.num_pkt_buffers = ipa_res->tx_num_alloc_buffer;
	tx.pkt_offset = 0;

	/* RX PIPE */
	/**
	 * Transfer Ring: REO Ring
	 * Transfer Ring Doorbell PA: REO Tail Pointer Address
	 * Event Ring: FW ring
	 * Event Ring Doorbell PA: FW Head Pointer Address
	 */
	rx.ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
	rx.ipa_ep_cfg.hdr.hdr_len = DP_IPA_UC_WLAN_RX_HDR_LEN;
	rx.ipa_ep_cfg.hdr.hdr_ofst_metadata_valid = 0;
	rx.ipa_ep_cfg.hdr.hdr_metadata_reg_valid = 1;
	rx.ipa_ep_cfg.mode.mode = IPA_BASIC;
	rx.client = IPA_CLIENT_WLAN1_PROD;
	rx.transfer_ring_base_pa = ipa_res->rx_rdy_ring_base_paddr;
	rx.transfer_ring_size = ipa_res->rx_rdy_ring_size;
	rx.transfer_ring_doorbell_pa = /* REO Tail Pointer Address */
		soc->ipa_uc_rx_rsc.ipa_reo_tp_paddr;
	rx.event_ring_base_pa = ipa_res->rx_refill_ring_base_paddr;
	rx.event_ring_size = ipa_res->rx_refill_ring_size;
	rx.event_ring_doorbell_pa = /* FW Head Pointer Address */
		soc->ipa_uc_rx_rsc.ipa_rx_refill_buf_hp_paddr;
	rx.pkt_offset = RX_PKT_TLVS_LEN + L3_HEADER_PADDING;

	pipe_in.notify = ipa_w2i_cb;
	pipe_in.priv = ipa_priv;
	memcpy(&pipe_in.tx, &tx, sizeof(struct ipa_wdi3_setup_info));
	memcpy(&pipe_in.rx, &rx, sizeof(struct ipa_wdi3_setup_info));

	/* Connect WDI IPA PIPE */
	ret = ipa_wdi3_conn_pipes(&pipe_in, &pipe_out);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "ipa_wdi3_conn_pipes: IPA pipe setup failed: ret=%d",
			  ret);
		return QDF_STATUS_E_FAILURE;
	}

	/* IPA uC Doorbell registers */
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		"Tx DB PA=0x%x, Rx DB PA=0x%x",
		(unsigned int)pipe_out.tx_uc_db_pa,
		(unsigned int)pipe_out.rx_uc_db_pa);

	ipa_res->tx_comp_doorbell_paddr = pipe_out.tx_uc_db_pa;
	ipa_res->rx_ready_doorbell_paddr = pipe_out.rx_uc_db_pa;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "Tx: %s=%pK, %s=%d, %s=%pK, %s=%pK, %s=%d, %s=%pK, %s=%d, %s=%pK",
		  "transfer_ring_base_pa",
		  (void *)pipe_in.tx.transfer_ring_base_pa,
		  "transfer_ring_size",
		  pipe_in.tx.transfer_ring_size,
		  "transfer_ring_doorbell_pa",
		  (void *)pipe_in.tx.transfer_ring_doorbell_pa,
		  "event_ring_base_pa",
		  (void *)pipe_in.tx.event_ring_base_pa,
		  "event_ring_size",
		  pipe_in.tx.event_ring_size,
		  "event_ring_doorbell_pa",
		  (void *)pipe_in.tx.event_ring_doorbell_pa,
		  "num_pkt_buffers",
		  pipe_in.tx.num_pkt_buffers,
		  "tx_comp_doorbell_paddr",
		  (void *)ipa_res->tx_comp_doorbell_paddr);

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "Rx: %s=%pK, %s=%d, %s=%pK, %s=%pK, %s=%d, %s=%pK, %s=%d, %s=%pK",
		  "transfer_ring_base_pa",
		  (void *)pipe_in.rx.transfer_ring_base_pa,
		  "transfer_ring_size",
		  pipe_in.rx.transfer_ring_size,
		  "transfer_ring_doorbell_pa",
		  (void *)pipe_in.rx.transfer_ring_doorbell_pa,
		  "event_ring_base_pa",
		  (void *)pipe_in.rx.event_ring_base_pa,
		  "event_ring_size",
		  pipe_in.rx.event_ring_size,
		  "event_ring_doorbell_pa",
		  (void *)pipe_in.rx.event_ring_doorbell_pa,
		  "num_pkt_buffers",
		  pipe_in.rx.num_pkt_buffers,
		  "tx_comp_doorbell_paddr",
		  (void *)ipa_res->rx_ready_doorbell_paddr);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_cleanup() - Disconnect IPA pipes
 * @tx_pipe_handle: Tx pipe handle
 * @rx_pipe_handle: Rx pipe handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_cleanup(uint32_t tx_pipe_handle, uint32_t rx_pipe_handle)
{
	int ret;

	ret = ipa_wdi3_disconn_pipes();
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "ipa_wdi3_disconn_pipes: IPA pipe cleanup failed: ret=%d",
		    ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_setup_iface() - Setup IPA header and register interface
 * @ifname: Interface name
 * @mac_addr: Interface MAC address
 * @prod_client: IPA prod client type
 * @cons_client: IPA cons client type
 * @session_id: Session ID
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_setup_iface(char *ifname, uint8_t *mac_addr,
			      enum ipa_client_type prod_client,
			      enum ipa_client_type cons_client,
			      uint8_t session_id, bool is_ipv6_enabled)
{
	struct ipa_wdi3_reg_intf_in_params in;
	struct ipa_wdi3_hdr_info hdr_info;
	struct dp_ipa_uc_tx_hdr uc_tx_hdr;
	int ret = -EINVAL;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "Add Partial hdr: %s, %pM",
		  ifname, mac_addr);

	qdf_mem_zero(&hdr_info, sizeof(struct ipa_wdi3_hdr_info));
	qdf_ether_addr_copy(uc_tx_hdr.eth.h_source, mac_addr);

	/* IPV4 header */
	uc_tx_hdr.eth.h_proto = qdf_htons(ETH_P_IP);

	hdr_info.hdr = (uint8_t *)&uc_tx_hdr;
	hdr_info.hdr_len = DP_IPA_UC_WLAN_TX_HDR_LEN;
	hdr_info.hdr_type = IPA_HDR_L2_ETHERNET_II;
	hdr_info.dst_mac_addr_offset = DP_IPA_UC_WLAN_HDR_DES_MAC_OFFSET;

	in.netdev_name = ifname;
	memcpy(&(in.hdr_info[0]), &hdr_info, sizeof(struct ipa_wdi3_hdr_info));
	in.is_meta_data_valid = 1;
	in.meta_data = htonl(session_id << 16);
	in.meta_data_mask = htonl(0x00FF0000);

	/* IPV6 header */
	if (is_ipv6_enabled) {
		uc_tx_hdr.eth.h_proto = qdf_htons(ETH_P_IPV6);
		memcpy(&(in.hdr_info[1]), &hdr_info,
				sizeof(struct ipa_wdi3_hdr_info));
	}

	ret = ipa_wdi3_reg_intf(&in);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "ipa_wdi3_reg_intf: register IPA interface falied: ret=%d",
		    ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_cleanup_iface() - Cleanup IPA header and deregister interface
 * @ifname: Interface name
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_cleanup_iface(char *ifname, bool is_ipv6_enabled)
{
	int ret;

	ret = ipa_wdi3_dereg_intf(ifname);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		  "ipa_wdi3_dereg_intf: IPA pipe deregistration failed: ret=%d",
		  ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

 /**
 * dp_ipa_uc_enable_pipes() - Enable and resume traffic on Tx/Rx pipes
 * @ppdev - handle to the device instance
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_enable_pipes(struct cdp_pdev *ppdev)
{
	QDF_STATUS result;

	result = ipa_wdi3_enable_pipes();
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Enable WDI PIPE fail, code %d",
				__func__, result);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_uc_disable_pipes() – Suspend traffic and disable Tx/Rx pipes
 * @ppdev - handle to the device instance
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_disable_pipes(struct cdp_pdev *ppdev)
{
	QDF_STATUS result;

	result = ipa_wdi3_disable_pipes();
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Disable WDI PIPE fail, code %d",
				__func__, result);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_ipa_set_perf_level() - Set IPA clock bandwidth based on data rates
 * @client: Client type
 * @max_supported_bw_mbps: Maximum bandwidth needed (in Mbps)
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_ipa_set_perf_level(int client, uint32_t max_supported_bw_mbps)
{
	struct ipa_wdi3_perf_profile profile;
	QDF_STATUS result;

	profile.client = client;
	profile.max_supported_bw_mbps = max_supported_bw_mbps;

	result = ipa_wdi3_set_perf_profile(&profile);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: ipa_wdi3_set_perf_profile fail, code %d",
				__func__, result);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif
