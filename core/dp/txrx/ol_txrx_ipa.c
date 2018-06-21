/*
 * Copyright (c) 2017-2018, The Linux Foundation. All rights reserved.
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

/*=== includes ===*/
/* header files for OS primitives */
#include <osdep.h>              /* uint32_t, etc. */
#include <qdf_mem.h>         /* qdf_mem_malloc,free */
#include <qdf_types.h>          /* qdf_device_t, qdf_print */
#include <qdf_lock.h>           /* qdf_spinlock */
#include <qdf_atomic.h>         /* qdf_atomic_read */

/* header files for utilities */
#include <cds_queue.h>          /* TAILQ */

/* header files for configuration API */
#include <ol_cfg.h>             /* ol_cfg_is_high_latency */
#include <ol_if_athvar.h>

/* header files for HTT API */
#include <ol_htt_api.h>
#include <ol_htt_tx_api.h>

/* header files for our own APIs */
#include <ol_txrx_api.h>
#include <ol_txrx_dbg.h>
#include <cdp_txrx_ocb.h>
#include <ol_txrx_ctrl_api.h>
#include <cdp_txrx_stats.h>
#include <ol_txrx_osif_api.h>
/* header files for our internal definitions */
#include <ol_txrx_internal.h>   /* TXRX_ASSERT, etc. */
#include <wdi_event.h>          /* WDI events */
#include <ol_tx.h>              /* ol_tx_ll */
#include <ol_rx.h>              /* ol_rx_deliver */
#include <ol_txrx_peer_find.h>  /* ol_txrx_peer_find_attach, etc. */
#include <ol_rx_pn.h>           /* ol_rx_pn_check, etc. */
#include <ol_rx_fwd.h>          /* ol_rx_fwd_check, etc. */
#include <ol_rx_reorder_timeout.h>      /* OL_RX_REORDER_TIMEOUT_INIT, etc. */
#include <ol_rx_reorder.h>
#include <ol_tx_send.h>         /* ol_tx_discard_target_frms */
#include <ol_tx_desc.h>         /* ol_tx_desc_frame_free */
#include <ol_tx_queue.h>
#include <ol_tx_sched.h>           /* ol_tx_sched_attach, etc. */
#include <ol_txrx.h>
#include <ol_txrx_types.h>
#include <cdp_txrx_flow_ctrl_legacy.h>
#include <cdp_txrx_bus.h>
#include <cdp_txrx_ipa.h>
#include <cdp_txrx_pmf.h>
#include "wma.h"
#include "hif.h"
#include <cdp_txrx_peer_ops.h>
#ifndef REMOVE_PKT_LOG
#include "pktlog_ac.h"
#endif
#include "epping_main.h"
#include <a_types.h>

#ifdef IPA_OFFLOAD
#include <ol_txrx_ipa.h>

/* For Tx pipes, use Ethernet-II Header format */
#ifdef QCA_WIFI_3_0
struct ol_txrx_ipa_uc_tx_hdr ipa_uc_tx_hdr = {
	{
		0x0000,
		0x00000000,
		0x00000000
	},
	{
		0x00000000
	},
	{
		{0x00, 0x03, 0x7f, 0xaa, 0xbb, 0xcc},
		{0x00, 0x03, 0x7f, 0xdd, 0xee, 0xff},
		0x0008
	}
};
#else
struct ol_txrx_ipa_uc_tx_hdr ipa_uc_tx_hdr = {
	{
		0x00000000,
		0x00000000
	},
	{
		0x00000000
	},
	{
		{0x00, 0x03, 0x7f, 0xaa, 0xbb, 0xcc},
		{0x00, 0x03, 0x7f, 0xdd, 0xee, 0xff},
		0x0008
	}
};
#endif

/**
 * ol_txrx_ipa_uc_get_resource() - Client request resource information
 * @pdev: handle to the HTT instance
 *
 *  OL client will request IPA UC related resource information
 *  Resource information will be distributted to IPA module
 *  All of the required resources should be pre-allocated
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_uc_get_resource(struct cdp_pdev *ppdev)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;
	struct ol_txrx_ipa_resources *ipa_res = &pdev->ipa_resource;
	qdf_device_t osdev = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!osdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: qdf device is null!", __func__);
		return QDF_STATUS_E_NOENT;
	}

	htt_ipa_uc_get_resource(pdev->htt_pdev,
				&ipa_res->ce_sr,
				&ipa_res->tx_comp_ring,
				&ipa_res->rx_rdy_ring,
				&ipa_res->rx2_rdy_ring,
				&ipa_res->rx_proc_done_idx,
				&ipa_res->rx2_proc_done_idx,
				&ipa_res->ce_sr_ring_size,
				&ipa_res->ce_reg_paddr,
				&ipa_res->tx_num_alloc_buffer);

	if ((0 == qdf_mem_get_dma_addr(osdev,
				&ipa_res->ce_sr->mem_info)) ||
	    (0 == qdf_mem_get_dma_addr(osdev,
				&ipa_res->tx_comp_ring->mem_info)) ||
	    (0 == qdf_mem_get_dma_addr(osdev,
				&ipa_res->rx_rdy_ring->mem_info))
#if defined(QCA_WIFI_3_0) && defined(CONFIG_IPA3)
	    || (0 == qdf_mem_get_dma_addr(osdev,
				&ipa_res->rx2_rdy_ring->mem_info))
#endif
	   )
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_uc_set_doorbell_paddr() - Client set IPA UC doorbell register
 * @pdev: handle to the HTT instance
 *
 *  IPA UC let know doorbell register physical address
 *  WLAN firmware will use this physical address to notify IPA UC
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_uc_set_doorbell_paddr(struct cdp_pdev *ppdev)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;
	struct ol_txrx_ipa_resources *ipa_res = &pdev->ipa_resource;
	int ret;

	ret = htt_ipa_uc_set_doorbell_paddr(pdev->htt_pdev,
				      ipa_res->tx_comp_doorbell_dmaaddr,
				      ipa_res->rx_ready_doorbell_dmaaddr);

	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "htt_ipa_uc_set_doorbell_dmaaddr fail: %d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_uc_set_active() - Client notify IPA UC data path active or not
 * @pdev: handle to the HTT instance
 * @uc_active: WDI UC path enable or not
 * @is_tx: TX path or RX path
 *
 *  IPA UC let know doorbell register physical address
 *  WLAN firmware will use this physical address to notify IPA UC
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_uc_set_active(struct cdp_pdev *ppdev, bool uc_active,
			       bool is_tx)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;
	int ret;

	ret = htt_h2t_ipa_uc_set_active(pdev->htt_pdev, uc_active, is_tx);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "htt_h2t_ipa_uc_set_active fail: %d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_uc_op_response() - Handle OP command response from firmware
 * @pdev: handle to the device instance
 * @op_msg: op response message from firmware
 *
 * Return: none
 */
QDF_STATUS ol_txrx_ipa_uc_op_response(struct cdp_pdev *ppdev, uint8_t *op_msg)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;

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
 * ol_txrx_ipa_uc_register_op_cb() - Register OP handler function
 * @pdev: handle to the device instance
 * @op_cb: handler function pointer
 *
 * Return: none
 */
QDF_STATUS ol_txrx_ipa_uc_register_op_cb(struct cdp_pdev *ppdev,
				   ipa_uc_op_cb_type op_cb, void *usr_ctxt)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;

	pdev->ipa_uc_op_cb = op_cb;
	pdev->usr_ctxt = usr_ctxt;

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_uc_get_stat() - Get firmware wdi status
 * @pdev: handle to the HTT instance
 *
 * Return: none
 */
QDF_STATUS ol_txrx_ipa_uc_get_stat(struct cdp_pdev *ppdev)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;
	int ret;

	ret = htt_h2t_ipa_uc_get_stats(pdev->htt_pdev);

	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "htt_h2t_ipa_uc_get_stats fail: %d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_enable_autonomy() - Enable autonomy RX path
 * @pdev: handle to the device instance
 *
 * Set all RX packet route to IPA
 * Return: none
 */
QDF_STATUS ol_txrx_ipa_enable_autonomy(struct cdp_pdev *ppdev)
{
	/* TBD */
	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_disable_autonomy() - Disable autonomy RX path
 * @pdev: handle to the device instance
 *
 * Disable RX packet route to host
 * Return: none
 */
QDF_STATUS ol_txrx_ipa_disable_autonomy(struct cdp_pdev *ppdev)
{
	/* TBD */
	return QDF_STATUS_SUCCESS;
}

#ifdef CONFIG_IPA_WDI_UNIFIED_API

#ifndef QCA_LL_TX_FLOW_CONTROL_V2
static inline void ol_txrx_setup_mcc_sys_pipes(
		qdf_ipa_sys_connect_params_t *sys_in,
		qdf_ipa_wdi_conn_in_params_t *pipe_in)
{
	/* Setup MCC sys pipe */
	QDF_IPA_WDI_CONN_IN_PARAMS_NUM_SYS_PIPE_NEEDED(pipe_in) =
			OL_TXRX_IPA_MAX_IFACE;
	for (int i = 0; i < OL_TXRX_IPA_MAX_IFACE; i++)
		memcpy(&QDF_IPA_WDI_CONN_IN_PARAMS_SYS_IN(pipe_in)[i],
		       &sys_in[i], sizeof(qdf_ipa_sys_connect_params_t));
}
#else
static inline void ol_txrx_setup_mcc_sys_pipes(
		qdf_ipa_sys_connect_params_t *sys_in,
		qdf_ipa_wdi_conn_in_params_t *pipe_in)
{
	QDF_IPA_WDI_CONN_IN_PARAMS_NUM_SYS_PIPE_NEEDED(pipe_in) = 0;
}
#endif

#ifdef ENABLE_SMMU_S1_TRANSLATION
/**
 * ol_txrx_ipa_wdi_tx_smmu_params() - Config IPA TX params
 * @ipa_res: IPA resources
 * @tx_smmu: IPA WDI pipe setup info
 *
 * Return: None
 */
static inline void ol_txrx_ipa_wdi_tx_smmu_params(
				struct ol_txrx_ipa_resources *ipa_res,
				qdf_ipa_wdi_pipe_setup_info_smmu_t *tx_smmu)
{
	QDF_IPA_WDI_SETUP_INFO_SMMU_CLIENT(tx_smmu) =
		IPA_CLIENT_WLAN1_CONS;
	qdf_mem_copy(&QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_BASE(
				tx_smmu),
		     &ipa_res->tx_comp_ring->sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_SIZE(tx_smmu) =
		ipa_res->tx_comp_ring->mem_info.size;
	qdf_mem_copy(&QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_BASE(
				tx_smmu),
		     &ipa_res->ce_sr->sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_SIZE(tx_smmu) =
		ipa_res->ce_sr_ring_size;
	QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_DOORBELL_PA(tx_smmu) =
		ipa_res->ce_reg_paddr;
	QDF_IPA_WDI_SETUP_INFO_SMMU_NUM_PKT_BUFFERS(tx_smmu) =
		ipa_res->tx_num_alloc_buffer;
	QDF_IPA_WDI_SETUP_INFO_SMMU_PKT_OFFSET(tx_smmu) = 0;
}

/**
 * ol_txrx_ipa_wdi_rx_smmu_params() - Config IPA RX params
 * @ipa_res: IPA resources
 * @rx_smmu: IPA WDI pipe setup info
 *
 * Return: None
 */
static inline void ol_txrx_ipa_wdi_rx_smmu_params(
				struct ol_txrx_ipa_resources *ipa_res,
				qdf_ipa_wdi_pipe_setup_info_smmu_t *rx_smmu)
{
	QDF_IPA_WDI_SETUP_INFO_SMMU_CLIENT(rx_smmu) =
		IPA_CLIENT_WLAN1_PROD;
	qdf_mem_copy(&QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_BASE(
				rx_smmu),
		     &ipa_res->rx_rdy_ring->sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_SIZE(rx_smmu) =
		ipa_res->rx_rdy_ring->mem_info.size;
	QDF_IPA_WDI_SETUP_INFO_SMMU_TRANSFER_RING_DOORBELL_PA(rx_smmu) =
		ipa_res->rx_proc_done_idx->mem_info.pa;
	qdf_mem_copy(&QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_BASE(
				rx_smmu),
		     &ipa_res->rx2_rdy_ring->sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_SIZE(rx_smmu) =
		ipa_res->rx2_rdy_ring->mem_info.size;
	QDF_IPA_WDI_SETUP_INFO_SMMU_EVENT_RING_DOORBELL_PA(rx_smmu) =
		ipa_res->rx2_proc_done_idx->mem_info.pa;
	QDF_IPA_WDI_SETUP_INFO_SMMU_PKT_OFFSET(rx_smmu) = 0;

}

#else

static inline void ol_txrx_ipa_wdi_tx_smmu_params(
				struct ol_txrx_ipa_resources *ipa_res,
				qdf_ipa_wdi_pipe_setup_info_smmu_t *tx_smmu)
{
}

static inline void ol_txrx_ipa_wdi_rx_smmu_params(
				struct ol_txrx_ipa_resources *ipa_res,
				qdf_ipa_wdi_pipe_setup_info_smmu_t *rx_smmu)
{
}
#endif

/**
 * ol_txrx_ipa_wdi_tx_params() - Config IPA TX params
 * @ipa_res: IPA resources
 * @tx: IPA WDI pipe setup info
 *
 * Return: None
 */
static inline void ol_txrx_ipa_wdi_tx_params(
				struct ol_txrx_ipa_resources *ipa_res,
				qdf_ipa_wdi_pipe_setup_info_t *tx)
{
	qdf_device_t osdev = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	if (!osdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: qdf device is null!", __func__);
		return;
	}

	QDF_IPA_WDI_SETUP_INFO_CLIENT(tx) = IPA_CLIENT_WLAN1_CONS;
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(tx) =
		qdf_mem_get_dma_addr(osdev,
				&ipa_res->tx_comp_ring->mem_info);
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(tx) =
		ipa_res->tx_comp_ring->mem_info.size;
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(tx) =
		qdf_mem_get_dma_addr(osdev,
				&ipa_res->ce_sr->mem_info);
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(tx) =
		ipa_res->ce_sr_ring_size;
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(tx) =
		ipa_res->ce_reg_paddr;
	QDF_IPA_WDI_SETUP_INFO_NUM_PKT_BUFFERS(tx) =
		ipa_res->tx_num_alloc_buffer;
	QDF_IPA_WDI_SETUP_INFO_PKT_OFFSET(tx) = 0;
}

/**
 * ol_txrx_ipa_wdi_rx_params() - Config IPA RX params
 * @ipa_res: IPA resources
 * @rx: IPA WDI pipe setup info
 *
 * Return: None
 */
static inline void ol_txrx_ipa_wdi_rx_params(
				struct ol_txrx_ipa_resources *ipa_res,
				qdf_ipa_wdi_pipe_setup_info_t *rx)
{
	QDF_IPA_WDI_SETUP_INFO_CLIENT(rx) = IPA_CLIENT_WLAN1_PROD;
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_BASE_PA(rx) =
		ipa_res->rx_rdy_ring->mem_info.pa;
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_SIZE(rx) =
		ipa_res->rx_rdy_ring->mem_info.size;
	QDF_IPA_WDI_SETUP_INFO_TRANSFER_RING_DOORBELL_PA(rx) =
		ipa_res->rx_proc_done_idx->mem_info.pa;
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_BASE_PA(rx) =
		ipa_res->rx2_rdy_ring->mem_info.pa;
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_SIZE(rx) =
		ipa_res->rx2_rdy_ring->mem_info.size;
	QDF_IPA_WDI_SETUP_INFO_EVENT_RING_DOORBELL_PA(rx) =
		ipa_res->rx2_proc_done_idx->mem_info.pa;
	QDF_IPA_WDI_SETUP_INFO_PKT_OFFSET(rx) = 0;
}

/**
 * ol_txrx_ipa_setup() - Setup and connect IPA pipes
 * @pdev: handle to the device instance
 * @ipa_i2w_cb: IPA to WLAN callback
 * @ipa_w2i_cb: WLAN to IPA callback
 * @ipa_wdi_meter_notifier_cb: IPA WDI metering callback
 * @ipa_desc_size: IPA descriptor size
 * @ipa_priv: handle to the HTT instance
 * @is_rm_enabled: Is IPA RM enabled or not
 * @p_tx_pipe_handle: pointer to Tx pipe handle
 * @p_rx_pipe_handle: pointer to Rx pipe handle
 * @is_smmu_enabled: Is SMMU enabled or not
 * @sys_in: parameters to setup sys pipe in mcc mode
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_setup(struct cdp_pdev *ppdev, void *ipa_i2w_cb,
			     void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
			     uint32_t ipa_desc_size, void *ipa_priv,
			     bool is_rm_enabled, uint32_t *p_tx_pipe_handle,
			     uint32_t *p_rx_pipe_handle, bool is_smmu_enabled,
			     qdf_ipa_sys_connect_params_t *sys_in)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;
	struct ol_txrx_ipa_resources *ipa_res = &pdev->ipa_resource;
	qdf_ipa_ep_cfg_t *tx_cfg;
	qdf_ipa_ep_cfg_t *rx_cfg;
	qdf_ipa_wdi_pipe_setup_info_t *tx;
	qdf_ipa_wdi_pipe_setup_info_t *rx;
	qdf_ipa_wdi_pipe_setup_info_smmu_t *tx_smmu;
	qdf_ipa_wdi_pipe_setup_info_smmu_t *rx_smmu;
	qdf_ipa_wdi_conn_in_params_t pipe_in;
	qdf_ipa_wdi_conn_out_params_t pipe_out;
	qdf_device_t osdev = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	uint32_t tx_comp_db_dmaaddr = 0, rx_rdy_db_dmaaddr = 0;
	int ret;

	if (!osdev) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: qdf device is null!", __func__);
		return QDF_STATUS_E_NOENT;
	}

	qdf_mem_zero(&pipe_in, sizeof(pipe_in));
	qdf_mem_zero(&pipe_out, sizeof(pipe_out));

	ol_txrx_setup_mcc_sys_pipes(sys_in, &pipe_in);

	/* TX PIPE */
	if (is_smmu_enabled) {
		QDF_IPA_WDI_CONN_IN_PARAMS_SMMU_ENABLED(&pipe_in) = true;
		tx_smmu = &QDF_IPA_WDI_CONN_IN_PARAMS_TX_SMMU(&pipe_in);
		tx_cfg = &QDF_IPA_WDI_SETUP_INFO_SMMU_EP_CFG(tx_smmu);
	} else {
		QDF_IPA_WDI_CONN_IN_PARAMS_SMMU_ENABLED(&pipe_in) = false;
		tx = &QDF_IPA_WDI_CONN_IN_PARAMS_TX(&pipe_in);
		tx_cfg = &QDF_IPA_WDI_SETUP_INFO_EP_CFG(tx);
	}

	QDF_IPA_EP_CFG_NAT_EN(tx_cfg) = IPA_BYPASS_NAT;
	QDF_IPA_EP_CFG_HDR_LEN(tx_cfg) = OL_TXRX_IPA_UC_WLAN_TX_HDR_LEN;
	QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE_VALID(tx_cfg) = 1;
	QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE(tx_cfg) = 0;
	QDF_IPA_EP_CFG_HDR_ADDITIONAL_CONST_LEN(tx_cfg) =
		OL_TXRX_IPA_UC_WLAN_8023_HDR_SIZE;
	QDF_IPA_EP_CFG_MODE(tx_cfg) = IPA_BASIC;
	QDF_IPA_EP_CFG_HDR_LITTLE_ENDIAN(tx_cfg) = true;

	if (is_smmu_enabled)
		ol_txrx_ipa_wdi_tx_smmu_params(ipa_res, tx_smmu);
	else
		ol_txrx_ipa_wdi_tx_params(ipa_res, tx);


	/* RX PIPE */
	if (is_smmu_enabled) {
		rx_smmu = &QDF_IPA_WDI_CONN_IN_PARAMS_RX_SMMU(&pipe_in);
		rx_cfg = &QDF_IPA_WDI_SETUP_INFO_EP_CFG(rx_smmu);
	} else {
		rx = &QDF_IPA_WDI_CONN_IN_PARAMS_RX(&pipe_in);
		rx_cfg = &QDF_IPA_WDI_SETUP_INFO_EP_CFG(rx);
	}

	QDF_IPA_EP_CFG_NAT_EN(rx_cfg) = IPA_BYPASS_NAT;
	QDF_IPA_EP_CFG_HDR_LEN(rx_cfg) = OL_TXRX_IPA_UC_WLAN_RX_HDR_LEN;
	QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE_VALID(rx_cfg) = 1;
	QDF_IPA_EP_CFG_HDR_OFST_PKT_SIZE(rx_cfg) = 0;
	QDF_IPA_EP_CFG_HDR_ADDITIONAL_CONST_LEN(rx_cfg) =
		OL_TXRX_IPA_UC_WLAN_8023_HDR_SIZE;
	QDF_IPA_EP_CFG_HDR_OFST_METADATA_VALID(rx_cfg) = 0;
	QDF_IPA_EP_CFG_HDR_METADATA_REG_VALID(rx_cfg) = 1;
	QDF_IPA_EP_CFG_MODE(rx_cfg) = IPA_BASIC;
	QDF_IPA_EP_CFG_HDR_LITTLE_ENDIAN(rx_cfg) = true;

	if (is_smmu_enabled)
		ol_txrx_ipa_wdi_rx_smmu_params(ipa_res, rx_smmu);
	else
		ol_txrx_ipa_wdi_rx_params(ipa_res, rx);

	QDF_IPA_WDI_CONN_IN_PARAMS_NOTIFY(&pipe_in) = ipa_w2i_cb;
	QDF_IPA_WDI_CONN_IN_PARAMS_PRIV(&pipe_in) = ipa_priv;

	/* Connect WDI IPA PIPE */
	ret = qdf_ipa_wdi_conn_pipes(&pipe_in, &pipe_out);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: ipa_wdi_conn_pipes: IPA pipe setup failed: ret=%d",
			  __func__, ret);
		return QDF_STATUS_E_FAILURE;
	}

	/* IPA uC Doorbell registers */
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Tx DB PA=0x%x, Rx DB PA=0x%x", __func__,
		  (unsigned int)
			QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_PA(&pipe_out),
		  (unsigned int)
			QDF_IPA_WDI_CONN_OUT_PARAMS_RX_UC_DB_PA(&pipe_out));

	ipa_res->tx_comp_doorbell_dmaaddr =
		QDF_IPA_WDI_CONN_OUT_PARAMS_TX_UC_DB_PA(&pipe_out);
	ipa_res->rx_ready_doorbell_dmaaddr =
		QDF_IPA_WDI_CONN_OUT_PARAMS_RX_UC_DB_PA(&pipe_out);

	if (is_smmu_enabled) {
		pld_smmu_map(osdev->dev, ipa_res->tx_comp_doorbell_dmaaddr,
			     &tx_comp_db_dmaaddr, sizeof(uint32_t));
		ipa_res->tx_comp_doorbell_dmaaddr = tx_comp_db_dmaaddr;

		pld_smmu_map(osdev->dev, ipa_res->rx_ready_doorbell_dmaaddr,
			     &rx_rdy_db_dmaaddr, sizeof(uint32_t));
		ipa_res->rx_ready_doorbell_dmaaddr = rx_rdy_db_dmaaddr;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_cleanup() - Disconnect IPA pipes
 * @tx_pipe_handle: Tx pipe handle
 * @rx_pipe_handle: Rx pipe handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_cleanup(uint32_t tx_pipe_handle, uint32_t rx_pipe_handle)
{
	int ret;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Disconnect IPA pipe", __func__);
	ret = qdf_ipa_wdi_disconn_pipes();
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "ipa_wdi_disconn_pipes failed: ret=%d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_setup_iface() - Setup IPA header and register interface
 * @ifname: Interface name
 * @mac_addr: Interface MAC address
 * @prod_client: IPA prod client type
 * @cons_client: IPA cons client type
 * @session_id: Session ID
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_setup_iface(char *ifname, uint8_t *mac_addr,
				   qdf_ipa_client_type_t prod_client,
				   qdf_ipa_client_type_t cons_client,
				   uint8_t session_id, bool is_ipv6_enabled)
{
	qdf_ipa_wdi_reg_intf_in_params_t in;
	qdf_ipa_wdi_hdr_info_t hdr_info;
	struct ol_txrx_ipa_uc_tx_hdr uc_tx_hdr;
	struct ol_txrx_ipa_uc_tx_hdr uc_tx_hdr_v6;
	int ret = -EINVAL;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Add Partial hdr: %s, %pM",
		  __func__, ifname, mac_addr);

	qdf_mem_zero(&hdr_info, sizeof(qdf_ipa_wdi_hdr_info_t));
	memcpy(&uc_tx_hdr, &ipa_uc_tx_hdr, OL_TXRX_IPA_UC_WLAN_TX_HDR_LEN);
	qdf_ether_addr_copy(uc_tx_hdr.eth.h_source, mac_addr);
	uc_tx_hdr.ipa_hd.vdev_id = session_id;

	/* IPV4 header */
	uc_tx_hdr.eth.h_proto = qdf_htons(ETH_P_IP);

	QDF_IPA_WDI_HDR_INFO_HDR(&hdr_info) = (uint8_t *)&uc_tx_hdr;
	QDF_IPA_WDI_HDR_INFO_HDR_LEN(&hdr_info) =
		OL_TXRX_IPA_UC_WLAN_TX_HDR_LEN;
	QDF_IPA_WDI_HDR_INFO_HDR_TYPE(&hdr_info) = IPA_HDR_L2_ETHERNET_II;
	QDF_IPA_WDI_HDR_INFO_DST_MAC_ADDR_OFFSET(&hdr_info) =
		OL_TXRX_IPA_UC_WLAN_HDR_DES_MAC_OFFSET;

	QDF_IPA_WDI_REG_INTF_IN_PARAMS_NETDEV_NAME(&in) = ifname;
	memcpy(&(QDF_IPA_WDI_REG_INTF_IN_PARAMS_HDR_INFO(&in)[IPA_IP_v4]),
	       &hdr_info, sizeof(qdf_ipa_wdi_hdr_info_t));
	QDF_IPA_WDI_REG_INTF_IN_PARAMS_ALT_DST_PIPE(&in) = cons_client;
	QDF_IPA_WDI_REG_INTF_IN_PARAMS_IS_META_DATA_VALID(&in) = 1;
	QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA(&in) =
		htonl(session_id << 16);
	QDF_IPA_WDI_REG_INTF_IN_PARAMS_META_DATA_MASK(&in) = htonl(0x00FF0000);

	/* IPV6 header */
	if (is_ipv6_enabled) {
		memcpy(&uc_tx_hdr_v6, &uc_tx_hdr,
		       OL_TXRX_IPA_UC_WLAN_TX_HDR_LEN);
		uc_tx_hdr_v6.eth.h_proto = qdf_htons(ETH_P_IPV6);
		QDF_IPA_WDI_HDR_INFO_HDR(&hdr_info) = (uint8_t *)&uc_tx_hdr_v6;
		memcpy(&(QDF_IPA_WDI_REG_INTF_IN_PARAMS_HDR_INFO(&in)[IPA_IP_v6]),
		       &hdr_info, sizeof(qdf_ipa_wdi_hdr_info_t));
	}

	ret = qdf_ipa_wdi_reg_intf(&in);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: ipa_wdi_reg_intf falied: ret=%d", __func__, ret);
	}

	return ret;
}

/**
 * ol_txrx_ipa_cleanup_iface() - Cleanup IPA header and deregister interface
 * @ifname: Interface name
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_cleanup_iface(char *ifname, bool is_ipv6_enabled)
{
	int ret;

	/* unregister the interface with IPA */
	ret = qdf_ipa_wdi_dereg_intf(ifname);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "%s: ipa_wdi_dereg_intf failed: devname=%s, ret=%d",
			  __func__, ifname, ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_uc_enable_pipes() - Enable and resume traffic on Tx/Rx pipes
 * @pdev: handle to the device instance
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_enable_pipes(struct cdp_pdev *ppdev)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;
	QDF_STATUS status;
	int ret;

	status = htt_rx_update_smmu_map(pdev->htt_pdev, true);
	if (status != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "IPA SMMU map failed status:%d", status);
		return status;
	}

	/* ACTIVATE TX PIPE */
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Enable IPA pipes", __func__);
	ret = qdf_ipa_wdi_enable_pipes();
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: ipa_wdi_enable_pipes failed: ret=%d",
				__func__, ret);
		status = htt_rx_update_smmu_map(pdev->htt_pdev, false);
		if (status != QDF_STATUS_SUCCESS)
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				  "IPA SMMU unmap failed");
		return QDF_STATUS_E_FAILURE;
	}

	ol_txrx_ipa_uc_set_active((struct cdp_pdev *)pdev, true, true);
	ol_txrx_ipa_uc_set_active((struct cdp_pdev *)pdev, true, false);

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_uc_disable_pipes() – Suspend traffic and disable Tx/Rx pipes
 * @pdev: handle to the device instance
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_disable_pipes(struct cdp_pdev *ppdev)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;
	int ret;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: Disable IPA pipes", __func__);
	ret = qdf_ipa_wdi_disable_pipes();
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: ipa_wdi_disable_pipes failed: ret=%d",
			  __func__, ret);
	}

	if (htt_rx_update_smmu_map(pdev->htt_pdev, false) !=
	    QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "IPA SMMU unmap failed");
	}

	return ret ? QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_set_perf_level() - Set IPA clock bandwidth based on data rates
 * @client: Client type
 * @max_supported_bw_mbps: Maximum bandwidth needed (in Mbps)
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_set_perf_level(int client,
				      uint32_t max_supported_bw_mbps)
{
	qdf_ipa_wdi_perf_profile_t profile;
	int result;

	QDF_IPA_WDI_PERF_PROFILE_CLIENT(&profile) = client;
	QDF_IPA_WDI_PERF_PROFILE_MAX_SUPPORTED_BW_MBPS(&profile) =
		max_supported_bw_mbps;
	result = qdf_ipa_wdi_set_perf_profile(&profile);

	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"Set perf profile failed, code %d", result);

		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#else /* CONFIG_IPA_WDI_UNIFIED_API */

#ifdef ENABLE_SMMU_S1_TRANSLATION
/**
 * ol_txrx_ipa_tx_smmu_params() - Config IPA TX params
 * @ipa_res: IPA resources
 * @pipe_in: IPA WDI TX pipe params
 *
 * Return: None
 */
static inline void ol_txrx_ipa_tx_smmu_params(
					struct ol_txrx_ipa_resources *ipa_res,
					qdf_ipa_wdi_in_params_t *pipe_in)
{
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		    "%s: SMMU Enabled", __func__);

	QDF_IPA_PIPE_IN_SMMU_ENABLED(pipe_in) = true;
	qdf_mem_copy(&QDF_IPA_PIPE_IN_DL_SMMU_COMP_RING(pipe_in),
		     &ipa_res->tx_comp_ring->sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_PIPE_IN_DL_SMMU_COMP_RING_SIZE(pipe_in) =
					ipa_res->tx_comp_ring->mem_info.size;
	qdf_mem_copy(&QDF_IPA_PIPE_IN_DL_SMMU_CE_RING(pipe_in),
		     &ipa_res->ce_sr->sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_PIPE_IN_DL_SMMU_CE_DOOR_BELL_PA(pipe_in) =
					ipa_res->ce_reg_paddr;
	QDF_IPA_PIPE_IN_DL_SMMU_CE_RING_SIZE(pipe_in) =
					ipa_res->ce_sr_ring_size;
	QDF_IPA_PIPE_IN_DL_SMMU_NUM_TX_BUFFERS(pipe_in) =
					ipa_res->tx_num_alloc_buffer;
}

/**
 * ol_txrx_ipa_rx_smmu_params() - Config IPA TX params
 * @ipa_res: IPA resources
 * @pipe_in: IPA WDI TX pipe params
 *
 * Return: None
 */
static inline void ol_txrx_ipa_rx_smmu_params(
					struct ol_txrx_ipa_resources *ipa_res,
					qdf_ipa_wdi_in_params_t *pipe_in)
{
	qdf_mem_copy(&QDF_IPA_PIPE_IN_UL_SMMU_RDY_RING(pipe_in),
		     &ipa_res->rx_rdy_ring->sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_PIPE_IN_UL_SMMU_RDY_RING_SIZE(pipe_in) =
				ipa_res->rx_rdy_ring->mem_info.size;
	QDF_IPA_PIPE_IN_UL_SMMU_RDY_RING_RP_PA(pipe_in) =
				ipa_res->rx_proc_done_idx->mem_info.pa;

	QDF_IPA_PIPE_IN_UL_SMMU_RDY_RING_RP_VA(pipe_in) =
				ipa_res->rx_proc_done_idx->vaddr;
	qdf_mem_copy(&QDF_IPA_PIPE_IN_UL_SMMU_RDY_COMP_RING(pipe_in),
		     &ipa_res->rx2_rdy_ring->sgtable,
		     sizeof(sgtable_t));
	QDF_IPA_PIPE_IN_UL_SMMU_RDY_COMP_RING_SIZE(pipe_in) =
				ipa_res->rx2_rdy_ring->mem_info.size;
	QDF_IPA_PIPE_IN_UL_SMMU_RDY_COMP_RING_WP_PA(pipe_in) =
				ipa_res->rx2_proc_done_idx->mem_info.pa;
	QDF_IPA_PIPE_IN_UL_SMMU_RDY_COMP_RING_WP_VA(pipe_in) =
				ipa_res->rx2_proc_done_idx->vaddr;
}
#else
static inline void ol_txrx_ipa_tx_smmu_params(
				struct ol_txrx_ipa_resources *ipa_res,
				qdf_ipa_wdi_in_params_t *pipe_in)
{
}

static inline void ol_txrx_ipa_rx_smmu_params(
				struct ol_txrx_ipa_resources *ipa_res,
				qdf_ipa_wdi_in_params_t *pipe_in)
{
}
#endif

/**
 * ol_txrx_ipa_tx_params() - Config IPA TX params
 * @ipa_res: IPA resources
 * @pipe_in: IPA WDI TX pipe params
 *
 * Return: None
 */
static inline void ol_txrx_ipa_tx_params(
					struct ol_txrx_ipa_resources *ipa_res,
					qdf_ipa_wdi_in_params_t *pipe_in)
{
	qdf_device_t osdev = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);

	QDF_IPA_PIPE_IN_DL_COMP_RING_BASE_PA(pipe_in) =
		qdf_mem_get_dma_addr(osdev,
				&ipa_res->tx_comp_ring->mem_info);
	QDF_IPA_PIPE_IN_DL_COMP_RING_SIZE(pipe_in) =
		ipa_res->tx_comp_ring->mem_info.size;
	QDF_IPA_PIPE_IN_DL_CE_RING_BASE_PA(pipe_in) =
		qdf_mem_get_dma_addr(osdev,
				&ipa_res->ce_sr->mem_info);
	QDF_IPA_PIPE_IN_DL_CE_DOOR_BELL_PA(pipe_in) =
		ipa_res->ce_reg_paddr;
	QDF_IPA_PIPE_IN_DL_CE_RING_SIZE(pipe_in) =
		ipa_res->ce_sr_ring_size;
	QDF_IPA_PIPE_IN_DL_NUM_TX_BUFFERS(pipe_in) =
		ipa_res->tx_num_alloc_buffer;
}

/**
 * ol_txrx_ipa_rx_params() - Config IPA RX params
 * @ipa_res: IPA resources
 * @pipe_in: IPA WDI RX pipe params
 *
 * Return: None
 */
static inline void ol_txrx_ipa_rx_params(
					struct ol_txrx_ipa_resources *ipa_res,
					qdf_ipa_wdi_in_params_t *pipe_in)
{
	QDF_IPA_PIPE_IN_UL_RDY_RING_BASE_PA(pipe_in) =
		ipa_res->rx_rdy_ring->mem_info.pa;
	QDF_IPA_PIPE_IN_UL_RDY_RING_SIZE(pipe_in) =
		ipa_res->rx_rdy_ring->mem_info.size;
	QDF_IPA_PIPE_IN_UL_RDY_RING_RP_PA(pipe_in) =
		ipa_res->rx_proc_done_idx->mem_info.pa;
	OL_TXRX_IPA_WDI2_SET(pipe_in, ipa_res,
			     cds_get_context(QDF_MODULE_ID_QDF_DEVICE));
}

/**
 * ol_txrx_ipa_setup() - Setup and connect IPA pipes
 * @pdev: handle to the device instance
 * @ipa_i2w_cb: IPA to WLAN callback
 * @ipa_w2i_cb: WLAN to IPA callback
 * @ipa_wdi_meter_notifier_cb: IPA WDI metering callback
 * @ipa_desc_size: IPA descriptor size
 * @ipa_priv: handle to the HTT instance
 * @is_rm_enabled: Is IPA RM enabled or not
 * @p_tx_pipe_handle: pointer to Tx pipe handle
 * @p_rx_pipe_handle: pointer to Rx pipe handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_setup(struct cdp_pdev *ppdev, void *ipa_i2w_cb,
			     void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
			     uint32_t ipa_desc_size, void *ipa_priv,
			     bool is_rm_enabled, uint32_t *p_tx_pipe_handle,
			     uint32_t *p_rx_pipe_handle)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;
	qdf_device_t osdev = cds_get_context(QDF_MODULE_ID_QDF_DEVICE);
	struct ol_txrx_ipa_resources *ipa_res = &pdev->ipa_resource;
	qdf_ipa_wdi_in_params_t pipe_in;
	qdf_ipa_wdi_out_params_t pipe_out;
	uint32_t tx_comp_db_dmaaddr = 0, rx_rdy_db_dmaaddr = 0;

	int ret;

	qdf_mem_zero(&pipe_in, sizeof(pipe_in));
	qdf_mem_zero(&pipe_out, sizeof(pipe_out));

	/* TX PIPE */
	QDF_IPA_PIPE_IN_NAT_EN(&pipe_in) = IPA_BYPASS_NAT;
	QDF_IPA_PIPE_IN_HDR_LEN(&pipe_in) = OL_TXRX_IPA_UC_WLAN_TX_HDR_LEN;
	QDF_IPA_PIPE_IN_HDR_OFST_PKT_SIZE_VALID(&pipe_in) = 1;
	QDF_IPA_PIPE_IN_HDR_OFST_PKT_SIZE(&pipe_in) = 0;
	QDF_IPA_PIPE_IN_HDR_ADDITIONAL_CONST_LEN(&pipe_in) =
		OL_TXRX_IPA_UC_WLAN_8023_HDR_SIZE;
	QDF_IPA_PIPE_IN_MODE(&pipe_in) = IPA_BASIC;
	QDF_IPA_PIPE_IN_CLIENT(&pipe_in) = IPA_CLIENT_WLAN1_CONS;
	QDF_IPA_PIPE_IN_DESC_FIFO_SZ(&pipe_in) = ipa_desc_size;
	QDF_IPA_PIPE_IN_PRIV(&pipe_in) = ipa_priv;
	QDF_IPA_PIPE_IN_HDR_LITTLE_ENDIAN(&pipe_in) = true;
	QDF_IPA_PIPE_IN_NOTIFY(&pipe_in) = ipa_i2w_cb;

	if (!is_rm_enabled) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			    "%s: IPA RM DISABLED, IPA AWAKE", __func__);
		QDF_IPA_PIPE_IN_KEEP_IPA_AWAKE(&pipe_in) = true;
	}

	if (qdf_mem_smmu_s1_enabled(osdev))
		ol_txrx_ipa_tx_smmu_params(ipa_res, &pipe_in);
	else
		ol_txrx_ipa_tx_params(ipa_res, &pipe_in);

	/* Connect WDI IPA PIPE */
	ret = qdf_ipa_connect_wdi_pipe(&pipe_in, &pipe_out);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "ipa_connect_wdi_pipe: Tx pipe setup failed: ret=%d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	/* Micro Controller Doorbell register */
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		"%s CONS DB pipe out 0x%x TX PIPE Handle 0x%x", __func__,
		(unsigned int)QDF_IPA_PIPE_OUT_UC_DOOR_BELL_PA(&pipe_out),
		pipe_out.clnt_hdl);
	ipa_res->tx_comp_doorbell_dmaaddr =
		QDF_IPA_PIPE_OUT_UC_DOOR_BELL_PA(&pipe_out);
	/* WLAN TX PIPE Handle */
	ipa_res->tx_pipe_handle = QDF_IPA_PIPE_OUT_CLNT_HDL(&pipe_out);
	*p_tx_pipe_handle = QDF_IPA_PIPE_OUT_CLNT_HDL(&pipe_out);

	/* RX PIPE */
	QDF_IPA_PIPE_IN_NAT_EN(&pipe_in) = IPA_BYPASS_NAT;
	QDF_IPA_PIPE_IN_HDR_LEN(&pipe_in) = OL_TXRX_IPA_UC_WLAN_RX_HDR_LEN;
	QDF_IPA_PIPE_IN_HDR_OFST_METADATA_VALID(&pipe_in) = 0;
	QDF_IPA_PIPE_IN_HDR_METADATA_REG_VALID(&pipe_in) = 1;
	QDF_IPA_PIPE_IN_MODE(&pipe_in) = IPA_BASIC;
	QDF_IPA_PIPE_IN_CLIENT(&pipe_in) = IPA_CLIENT_WLAN1_PROD;
	QDF_IPA_PIPE_IN_DESC_FIFO_SZ(&pipe_in) =
		ipa_desc_size + SPS_DESC_SIZE;
	QDF_IPA_PIPE_IN_NOTIFY(&pipe_in) = ipa_w2i_cb;
	if (!is_rm_enabled) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			    "%s: IPA RM DISABLED, IPA AWAKE", __func__);
		QDF_IPA_PIPE_IN_KEEP_IPA_AWAKE(&pipe_in) = true;
	}

	if (qdf_mem_smmu_s1_enabled(osdev))
		ol_txrx_ipa_rx_smmu_params(ipa_res, &pipe_in);
	else
		ol_txrx_ipa_rx_params(ipa_res, &pipe_in);

#ifdef FEATURE_METERING
	QDF_IPA_PIPE_IN_WDI_NOTIFY(&pipe_in) = ipa_wdi_meter_notifier_cb;
#endif

	ret = qdf_ipa_connect_wdi_pipe(&pipe_in, &pipe_out);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "ipa_connect_wdi_pipe: Rx pipe setup failed: ret=%d", ret);
		return QDF_STATUS_E_FAILURE;
	}
	ipa_res->rx_ready_doorbell_dmaaddr =
		QDF_IPA_PIPE_OUT_UC_DOOR_BELL_PA(&pipe_out);
	ipa_res->rx_pipe_handle = QDF_IPA_PIPE_OUT_CLNT_HDL(&pipe_out);
	*p_rx_pipe_handle = QDF_IPA_PIPE_OUT_CLNT_HDL(&pipe_out);

	if (qdf_mem_smmu_s1_enabled(osdev)) {
		pld_smmu_map(osdev->dev, ipa_res->tx_comp_doorbell_dmaaddr,
			     &tx_comp_db_dmaaddr, sizeof(uint32_t));
		ipa_res->tx_comp_doorbell_dmaaddr = tx_comp_db_dmaaddr;

		pld_smmu_map(osdev->dev, ipa_res->rx_ready_doorbell_dmaaddr,
			     &rx_rdy_db_dmaaddr, sizeof(uint32_t));
		ipa_res->rx_ready_doorbell_dmaaddr = rx_rdy_db_dmaaddr;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_cleanup() - Disconnect IPA pipes
 * @tx_pipe_handle: Tx pipe handle
 * @rx_pipe_handle: Rx pipe handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_cleanup(uint32_t tx_pipe_handle, uint32_t rx_pipe_handle)
{
	int ret;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		    "%s: Disconnect TX PIPE tx_pipe_handle=0x%x",
		    __func__, tx_pipe_handle);
	ret = qdf_ipa_disconnect_wdi_pipe(tx_pipe_handle);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "ipa_disconnect_wdi_pipe: Tx pipe cleanup failed: ret=%d",
		    ret);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		    "%s: Disconnect RX PIPE rx_pipe_handle=0x%x",
		    __func__, rx_pipe_handle);
	ret = qdf_ipa_disconnect_wdi_pipe(rx_pipe_handle);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "ipa_disconnect_wdi_pipe: Rx pipe cleanup failed: ret=%d",
		    ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_remove_ipa_header() - Remove a specific header from IPA
 * @name: Name of the header to be removed
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS ol_txrx_ipa_remove_header(char *name)
{
	qdf_ipa_ioc_get_hdr_t hdrlookup;
	int ret = 0, len;
	qdf_ipa_ioc_del_hdr_t *ipa_hdr;

	qdf_mem_zero(&hdrlookup, sizeof(hdrlookup));
	strlcpy(hdrlookup.name, name, sizeof(hdrlookup.name));
	ret = qdf_ipa_get_hdr(&hdrlookup);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "Hdr deleted already %s, %d", name, ret);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG, "hdl: 0x%x",
		  hdrlookup.hdl);
	len = sizeof(qdf_ipa_ioc_del_hdr_t) + sizeof(qdf_ipa_hdr_del_t) * 1;
	ipa_hdr = (qdf_ipa_ioc_del_hdr_t *)qdf_mem_malloc(len);
	if (ipa_hdr == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "ipa_hdr allocation failed");
		return QDF_STATUS_E_FAILURE;
	}
	QDF_IPA_IOC_DEL_HDR_NUM_HDRS(ipa_hdr) = 1;
	QDF_IPA_IOC_DEL_HDR_COMMIT(ipa_hdr) = 0;
	QDF_IPA_IOC_DEL_HDR_HDL(ipa_hdr) = QDF_IPA_IOC_GET_HDR_HDL(&hdrlookup);
	QDF_IPA_IOC_DEL_HDR_STATUS(ipa_hdr) = -1;
	ret = qdf_ipa_del_hdr(ipa_hdr);
	if (ret != 0) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Delete header failed: %d", ret);
		qdf_mem_free(ipa_hdr);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_free(ipa_hdr);
	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_add_header_info() - Add IPA header for a given interface
 * @ifname: Interface name
 * @mac_addr: Interface MAC address
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: 0 on success, negativer errno value on error
 */
static int ol_txrx_ipa_add_header_info(char *ifname, uint8_t *mac_addr,
				       uint8_t session_id, bool is_ipv6_enabled)
{
	qdf_ipa_ioc_add_hdr_t *ipa_hdr = NULL;
	int ret = -EINVAL;
	struct ol_txrx_ipa_uc_tx_hdr *uc_tx_hdr = NULL;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "Add Partial hdr: %s, %pM", ifname, mac_addr);

	/* dynamically allocate the memory to add the hdrs */
	ipa_hdr = qdf_mem_malloc(sizeof(qdf_ipa_ioc_add_hdr_t)
				 + sizeof(qdf_ipa_hdr_add_t));
	if (!ipa_hdr) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			    "%s: ipa_hdr allocation failed", ifname);
		ret = -ENOMEM;
		goto end;
	}

	QDF_IPA_IOC_ADD_HDR_COMMIT(ipa_hdr) = 0;
	QDF_IPA_IOC_ADD_HDR_NUM_HDRS(ipa_hdr) = 1;

	uc_tx_hdr = (struct ol_txrx_ipa_uc_tx_hdr *)
		QDF_IPA_IOC_ADD_HDR_HDR(ipa_hdr);
	memcpy(uc_tx_hdr, &ipa_uc_tx_hdr, OL_TXRX_IPA_UC_WLAN_TX_HDR_LEN);
	memcpy(uc_tx_hdr->eth.h_source, mac_addr, ETH_ALEN);
	uc_tx_hdr->ipa_hd.vdev_id = session_id;
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "ifname=%s, vdev_id=%d",
		  ifname, uc_tx_hdr->ipa_hd.vdev_id);
	snprintf(QDF_IPA_IOC_ADD_HDR_NAME(ipa_hdr), IPA_RESOURCE_NAME_MAX,
		 "%s%s", ifname, OL_TXRX_IPA_IPV4_NAME_EXT);
	QDF_IPA_IOC_ADD_HDR_HDR_LEN(ipa_hdr) = OL_TXRX_IPA_UC_WLAN_TX_HDR_LEN;
	QDF_IPA_IOC_ADD_HDR_TYPE(ipa_hdr) = IPA_HDR_L2_ETHERNET_II;
	QDF_IPA_IOC_ADD_HDR_IS_PARTIAL(ipa_hdr) = 1;
	QDF_IPA_IOC_ADD_HDR_HDR_HDL(ipa_hdr) = 0;
	QDF_IPA_IOC_ADD_HDR_IS_ETH2_OFST_VALID(ipa_hdr) = 1;
	QDF_IPA_IOC_ADD_HDR_ETH2_OFST(ipa_hdr) =
		OL_TXRX_IPA_UC_WLAN_HDR_DES_MAC_OFFSET;

	ret = qdf_ipa_add_hdr(ipa_hdr);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s IPv4 add hdr failed: %d", ifname, ret);
		goto end;
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		"%s: IPv4 hdr_hdl: 0x%x",
		QDF_IPA_IOC_ADD_HDR_NAME(ipa_hdr),
		QDF_IPA_IOC_ADD_HDR_HDR_HDL(ipa_hdr));

	if (is_ipv6_enabled) {
		snprintf(QDF_IPA_IOC_ADD_HDR_NAME(ipa_hdr),
			 IPA_RESOURCE_NAME_MAX, "%s%s",
			 ifname, OL_TXRX_IPA_IPV6_NAME_EXT);

		uc_tx_hdr = (struct ol_txrx_ipa_uc_tx_hdr *)
			QDF_IPA_IOC_ADD_HDR_HDR(ipa_hdr);
		uc_tx_hdr->eth.h_proto = cpu_to_be16(ETH_P_IPV6);

		ret = qdf_ipa_add_hdr(ipa_hdr);
		if (ret) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				    "%s: IPv6 add hdr failed: %d", ifname, ret);
			goto clean_ipv4_hdr;
		}

		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: IPv6 hdr_hdl: 0x%x",
			QDF_IPA_IOC_ADD_HDR_NAME(ipa_hdr),
			QDF_IPA_IOC_ADD_HDR_HDR_HDL(ipa_hdr));
	}

	qdf_mem_free(ipa_hdr);

	return ret;

clean_ipv4_hdr:
	snprintf(ipa_hdr->hdr[0].name, IPA_RESOURCE_NAME_MAX, "%s%s",
		 ifname, OL_TXRX_IPA_IPV4_NAME_EXT);
	ol_txrx_ipa_remove_header(ipa_hdr->hdr[0].name);
end:
	if (ipa_hdr)
		qdf_mem_free(ipa_hdr);

	return ret;
}

/**
 * ol_txrx_ipa_register_interface() - register IPA interface
 * @ifname: Interface name
 * @prod_client: IPA prod client type
 * @cons_client: IPA cons client type
 * @session_id: Session ID
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: 0 on success, negative errno on error
 */
static int ol_txrx_ipa_register_interface(char *ifname,
					  qdf_ipa_client_type_t prod_client,
					  qdf_ipa_client_type_t cons_client,
					  uint8_t session_id,
					  bool is_ipv6_enabled)
{
	qdf_ipa_tx_intf_t tx_intf;
	qdf_ipa_rx_intf_t rx_intf;
	qdf_ipa_ioc_tx_intf_prop_t *tx_prop = NULL;
	qdf_ipa_ioc_rx_intf_prop_t *rx_prop = NULL;

	char ipv4_hdr_name[IPA_RESOURCE_NAME_MAX];
	char ipv6_hdr_name[IPA_RESOURCE_NAME_MAX];

	int num_prop = 1;
	int ret = 0;

	if (is_ipv6_enabled)
		num_prop++;

	/* Allocate TX properties for TOS categories, 1 each for IPv4 & IPv6 */
	tx_prop =
		qdf_mem_malloc(sizeof(qdf_ipa_ioc_tx_intf_prop_t) * num_prop);
	if (!tx_prop) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "tx_prop allocation failed");
		goto register_interface_fail;
	}

	/* Allocate RX properties, 1 each for IPv4 & IPv6 */
	rx_prop =
		qdf_mem_malloc(sizeof(qdf_ipa_ioc_rx_intf_prop_t) * num_prop);
	if (!rx_prop) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "rx_prop allocation failed");
		goto register_interface_fail;
	}

	qdf_mem_zero(&tx_intf, sizeof(tx_intf));
	qdf_mem_zero(&rx_intf, sizeof(rx_intf));

	snprintf(ipv4_hdr_name, IPA_RESOURCE_NAME_MAX, "%s%s",
		 ifname, OL_TXRX_IPA_IPV4_NAME_EXT);
	snprintf(ipv6_hdr_name, IPA_RESOURCE_NAME_MAX, "%s%s",
		 ifname, OL_TXRX_IPA_IPV6_NAME_EXT);

	QDF_IPA_IOC_RX_INTF_PROP_IP(&rx_prop[IPA_IP_v4]) = IPA_IP_v4;
	QDF_IPA_IOC_RX_INTF_PROP_SRC_PIPE(&rx_prop[IPA_IP_v4]) = prod_client;
	QDF_IPA_IOC_RX_INTF_PROP_HDR_L2_TYPE(&rx_prop[IPA_IP_v4]) =
		IPA_HDR_L2_ETHERNET_II;
	QDF_IPA_IOC_RX_INTF_PROP_ATTRIB_MASK(&rx_prop[IPA_IP_v4]) =
		IPA_FLT_META_DATA;

	/*
	 * Interface ID is 3rd byte in the CLD header. Add the meta data and
	 * mask to identify the interface in IPA hardware
	 */
	QDF_IPA_IOC_RX_INTF_PROP_META_DATA(&rx_prop[IPA_IP_v4]) =
		htonl(session_id << 16);
	QDF_IPA_IOC_RX_INTF_PROP_META_DATA_MASK(&rx_prop[IPA_IP_v4]) =
		htonl(0x00FF0000);

	rx_intf.num_props++;
	if (is_ipv6_enabled) {
		QDF_IPA_IOC_RX_INTF_PROP_IP(&rx_prop[IPA_IP_v6]) = IPA_IP_v6;
		QDF_IPA_IOC_RX_INTF_PROP_SRC_PIPE(&rx_prop[IPA_IP_v6]) =
			prod_client;
		QDF_IPA_IOC_RX_INTF_PROP_HDR_L2_TYPE(&rx_prop[IPA_IP_v6]) =
			IPA_HDR_L2_ETHERNET_II;
		QDF_IPA_IOC_RX_INTF_PROP_ATTRIB_MASK(&rx_prop[IPA_IP_v6]) =
			IPA_FLT_META_DATA;
		QDF_IPA_IOC_RX_INTF_PROP_META_DATA(&rx_prop[IPA_IP_v6]) =
			htonl(session_id << 16);
		QDF_IPA_IOC_RX_INTF_PROP_META_DATA_MASK(&rx_prop[IPA_IP_v6]) =
			htonl(0x00FF0000);

		rx_intf.num_props++;
	}

	QDF_IPA_IOC_TX_INTF_PROP_IP(&tx_prop[IPA_IP_v4]) = IPA_IP_v4;
	QDF_IPA_IOC_TX_INTF_PROP_HDR_L2_TYPE(&tx_prop[IPA_IP_v4]) =
			IPA_HDR_L2_ETHERNET_II;
	QDF_IPA_IOC_TX_INTF_PROP_DST_PIPE(&tx_prop[IPA_IP_v4]) =
			IPA_CLIENT_WLAN1_CONS;
	QDF_IPA_IOC_TX_INTF_PROP_ALT_DST_PIPE(&tx_prop[IPA_IP_v4]) =
			cons_client;
	strlcpy(QDF_IPA_IOC_TX_INTF_PROP_HDR_NAME(&tx_prop[IPA_IP_v4]),
		ipv4_hdr_name, IPA_RESOURCE_NAME_MAX);
	tx_intf.num_props++;

	if (is_ipv6_enabled) {
		QDF_IPA_IOC_TX_INTF_PROP_IP(&tx_prop[IPA_IP_v6]) = IPA_IP_v6;
		QDF_IPA_IOC_TX_INTF_PROP_HDR_L2_TYPE(&tx_prop[IPA_IP_v6]) =
			IPA_HDR_L2_ETHERNET_II;
		QDF_IPA_IOC_TX_INTF_PROP_DST_PIPE(&tx_prop[IPA_IP_v6]) =
			IPA_CLIENT_WLAN1_CONS;
		QDF_IPA_IOC_TX_INTF_PROP_ALT_DST_PIPE(&tx_prop[IPA_IP_v6]) =
			cons_client;
		strlcpy(QDF_IPA_IOC_TX_INTF_PROP_HDR_NAME(&tx_prop[IPA_IP_v6]),
			ipv6_hdr_name, IPA_RESOURCE_NAME_MAX);
		tx_intf.num_props++;
	}

	QDF_IPA_TX_INTF_PROP(&tx_intf) = tx_prop;
	QDF_IPA_RX_INTF_PROP(&rx_intf) = rx_prop;

	/* Call the ipa api to register interface */
	ret = qdf_ipa_register_intf(ifname, &tx_intf, &rx_intf);

register_interface_fail:
	qdf_mem_free(tx_prop);
	qdf_mem_free(rx_prop);
	return ret;
}

/**
 * ol_txrx_ipa_setup_iface() - Setup IPA header and register interface
 * @ifname: Interface name
 * @mac_addr: Interface MAC address
 * @prod_client: IPA prod client type
 * @cons_client: IPA cons client type
 * @session_id: Session ID
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_setup_iface(char *ifname, uint8_t *mac_addr,
				   qdf_ipa_client_type_t prod_client,
				   qdf_ipa_client_type_t cons_client,
				   uint8_t session_id, bool is_ipv6_enabled)
{
	int ret;

	ret = ol_txrx_ipa_add_header_info(ifname, mac_addr, session_id,
					  is_ipv6_enabled);
	if (ret)
		return QDF_STATUS_E_FAILURE;

	/* Configure the TX and RX pipes filter rules */
	ret = ol_txrx_ipa_register_interface(ifname,
					     prod_client,
					     cons_client,
					     session_id, is_ipv6_enabled);
	if (ret)
		return QDF_STATUS_E_FAILURE;

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_cleanup_iface() - Cleanup IPA header and deregister interface
 * @ifname: Interface name
 * @is_ipv6_enabled: Is IPV6 enabled or not
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_cleanup_iface(char *ifname, bool is_ipv6_enabled)
{
	char name_ipa[IPA_RESOURCE_NAME_MAX];
	int ret;

	/* Remove the headers */
	snprintf(name_ipa, IPA_RESOURCE_NAME_MAX, "%s%s",
			ifname, OL_TXRX_IPA_IPV4_NAME_EXT);
	ol_txrx_ipa_remove_header(name_ipa);

	if (is_ipv6_enabled) {
		snprintf(name_ipa, IPA_RESOURCE_NAME_MAX, "%s%s",
				ifname, OL_TXRX_IPA_IPV6_NAME_EXT);
		ol_txrx_ipa_remove_header(name_ipa);
	}
	/* unregister the interface with IPA */
	ret = qdf_ipa_deregister_intf(ifname);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
				"%s: ipa_deregister_intf fail: %d",
				ifname, ret);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_uc_enable_pipes() - Enable and resume traffic on Tx/Rx pipes
 * @pdev: handle to the device instance
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_enable_pipes(struct cdp_pdev *ppdev)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;
	struct ol_txrx_ipa_resources *ipa_res = &pdev->ipa_resource;
	int result;
	QDF_STATUS status;

	status = htt_rx_update_smmu_map(pdev->htt_pdev, true);
	if (status != QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "IPA SMMU map failed status:%d", status);
		return status;
	}

	/* ACTIVATE TX PIPE */
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Enable TX PIPE(tx_pipe_handle=%d)",
			__func__, ipa_res->tx_pipe_handle);
	result = qdf_ipa_enable_wdi_pipe(ipa_res->tx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Enable TX PIPE fail, code %d",
				__func__, result);
		goto smmu_unmap;
	}
	result = qdf_ipa_resume_wdi_pipe(ipa_res->tx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Resume TX PIPE fail, code %d",
				__func__, result);
		goto smmu_unmap;
	}
	ol_txrx_ipa_uc_set_active((struct cdp_pdev *)pdev, true, true);

	/* ACTIVATE RX PIPE */
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Enable RX PIPE(rx_pipe_handle=%d)",
			__func__, ipa_res->rx_pipe_handle);
	result = qdf_ipa_enable_wdi_pipe(ipa_res->rx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Enable RX PIPE fail, code %d",
				__func__, result);
		goto smmu_unmap;
	}
	result = qdf_ipa_resume_wdi_pipe(ipa_res->rx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Resume RX PIPE fail, code %d",
				__func__, result);
		goto smmu_unmap;
	}
	ol_txrx_ipa_uc_set_active((struct cdp_pdev *)pdev, true, false);

	return QDF_STATUS_SUCCESS;

smmu_unmap:
	if (htt_rx_update_smmu_map(pdev->htt_pdev, false) !=
	    QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "IPA SMMU unmap failed");
	}

	return QDF_STATUS_E_FAILURE;
}

/**
 * ol_txrx_ipa_uc_disable_pipes() – Suspend traffic and disable Tx/Rx pipes
 * @pdev: handle to the device instance
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_disable_pipes(struct cdp_pdev *ppdev)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;
	struct ol_txrx_ipa_resources *ipa_res = &pdev->ipa_resource;
	int result;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Disable RX PIPE", __func__);
	result = qdf_ipa_suspend_wdi_pipe(ipa_res->rx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Suspend RX PIPE fail, code %d",
				__func__, result);
		goto smmu_unmap;
	}

	result = qdf_ipa_disable_wdi_pipe(ipa_res->rx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Disable RX PIPE fail, code %d",
				__func__, result);
		goto smmu_unmap;
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Disable TX PIPE", __func__);
	result = qdf_ipa_suspend_wdi_pipe(ipa_res->tx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Suspend TX PIPE fail, code %d",
				__func__, result);
		goto smmu_unmap;
	}
	result = qdf_ipa_disable_wdi_pipe(ipa_res->tx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Disable TX PIPE fail, code %d",
				__func__, result);
		goto smmu_unmap;
	}

smmu_unmap:
	if (htt_rx_update_smmu_map(pdev->htt_pdev, false) !=
	    QDF_STATUS_SUCCESS) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "IPA SMMU unmap failed");
	}

	return result ? QDF_STATUS_E_FAILURE : QDF_STATUS_SUCCESS;
}

/**
 * ol_txrx_ipa_set_perf_level() - Set IPA clock bandwidth based on data rates
 * @client: Client type
 * @max_supported_bw_mbps: Maximum bandwidth needed (in Mbps)
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_set_perf_level(int client,
				      uint32_t max_supported_bw_mbps)
{
	qdf_ipa_rm_resource_name_t resource_name;
	qdf_ipa_rm_perf_profile_t profile;
	int result;

	if (client == QDF_IPA_CLIENT_WLAN1_PROD) {
		resource_name = QDF_IPA_RM_RESOURCE_WLAN_PROD;
	} else if (client == QDF_IPA_CLIENT_WLAN1_CONS) {
		resource_name = QDF_IPA_RM_RESOURCE_WLAN_CONS;
	} else {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"not supported client %d", client);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_IPA_RM_PERF_PROFILE_MAX_SUPPORTED_BANDWIDTH_MBPS(&profile) =
		max_supported_bw_mbps;
	result = qdf_ipa_rm_set_perf_profile(resource_name, &profile);

	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"Set perf profile failed, code %d", result);

		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

#endif /* CONFIG_IPA_WDI_UNIFIED_API */

#ifdef FEATURE_METERING
QDF_STATUS ol_txrx_ipa_uc_get_share_stats(struct cdp_pdev *ppdev,
					   uint8_t reset_stats)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;
	int result;

	result = htt_h2t_ipa_uc_get_share_stats(pdev->htt_pdev, reset_stats);

	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Get IPA sharing stats failed, code %d", result);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS ol_txrx_ipa_uc_set_quota(struct cdp_pdev *ppdev,
				     uint64_t quota_bytes)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)ppdev;
	int result;

	result = htt_h2t_ipa_uc_set_quota(pdev->htt_pdev, quota_bytes);

	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Set IPA quota failed, code %d", result);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}
#endif
#endif /* IPA_UC_OFFLOAD */
