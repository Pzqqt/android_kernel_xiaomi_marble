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

/*=== includes ===*/
/* header files for OS primitives */
#include <osdep.h>              /* uint32_t, etc. */
#include <qdf_mem.h>         /* qdf_mem_malloc,free */
#include <qdf_types.h>          /* qdf_device_t, qdf_print */
#include <qdf_lock.h>           /* qdf_spinlock */
#include <qdf_atomic.h>         /* qdf_atomic_read */

#if defined(HIF_PCI) || defined(HIF_SNOC) || defined(HIF_AHB)
/* Required for WLAN_FEATURE_FASTPATH */
#include <ce_api.h>
#endif
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

/**
 * ol_txrx_ipa_uc_get_resource() - Client request resource information
 * @pdev: handle to the HTT instance
 *
 *  OL client will reuqest IPA UC related resource information
 *  Resource information will be distributted to IPA module
 *  All of the required resources should be pre-allocated
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ol_txrx_ipa_uc_get_resource(struct cdp_pdev *ppdev)
{
	ol_txrx_pdev_handle pdev = (ol_txrx_pdev_handle)ppdev;
	struct ol_txrx_ipa_resources *ipa_res = &pdev->ipa_resource;

	htt_ipa_uc_get_resource(pdev->htt_pdev,
				&ipa_res->ce_sr_base_paddr,
				&ipa_res->ce_sr_ring_size,
				&ipa_res->ce_reg_paddr,
				&ipa_res->tx_comp_ring_base_paddr,
				&ipa_res->tx_comp_ring_size,
				&ipa_res->tx_num_alloc_buffer,
				&ipa_res->rx_rdy_ring_base_paddr,
				&ipa_res->rx_rdy_ring_size,
				&ipa_res->rx_proc_done_idx_paddr,
				&ipa_res->rx_proc_done_idx_vaddr,
				&ipa_res->rx2_rdy_ring_base_paddr,
				&ipa_res->rx2_rdy_ring_size,
				&ipa_res->rx2_proc_done_idx_paddr,
				&ipa_res->rx2_proc_done_idx_vaddr);

	if ((0 == ipa_res->ce_sr_base_paddr) ||
	    (0 == ipa_res->tx_comp_ring_base_paddr) ||
	    (0 == ipa_res->rx_rdy_ring_base_paddr)
#if defined(QCA_WIFI_3_0) && defined(CONFIG_IPA3)
	    || (0 == ipa_res->rx2_rdy_ring_base_paddr)
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
				      ipa_res->tx_comp_doorbell_paddr,
				      ipa_res->rx_ready_doorbell_paddr);

	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "htt_ipa_uc_set_doorbell_paddr fail: %d", ret);
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
	struct ol_txrx_ipa_resources *ipa_res = &pdev->ipa_resource;
	struct ipa_wdi_in_params pipe_in;
	struct ipa_wdi_out_params pipe_out;
	int ret;

	qdf_mem_zero(&pipe_in, sizeof(struct ipa_wdi_in_params));
	qdf_mem_zero(&pipe_out, sizeof(struct ipa_wdi_out_params));

	/* TX PIPE */
	pipe_in.sys.ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_len = OL_TXRX_IPA_UC_WLAN_TX_HDR_LEN;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_ofst_pkt_size_valid = 1;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_ofst_pkt_size = 0;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_additional_const_len =
		OL_TXRX_IPA_UC_WLAN_8023_HDR_SIZE;
	pipe_in.sys.ipa_ep_cfg.mode.mode = IPA_BASIC;
	pipe_in.sys.client = IPA_CLIENT_WLAN1_CONS;
	pipe_in.sys.desc_fifo_sz = ipa_desc_size;
	pipe_in.sys.priv = ipa_priv;
	pipe_in.sys.ipa_ep_cfg.hdr_ext.hdr_little_endian = true;
	pipe_in.sys.notify = ipa_i2w_cb;
	if (!is_rm_enabled) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			    "%s: IPA RM DISABLED, IPA AWAKE", __func__);
		pipe_in.sys.keep_ipa_awake = true;
	}

	pipe_in.u.dl.comp_ring_base_pa = ipa_res->tx_comp_ring_base_paddr;
	pipe_in.u.dl.comp_ring_size = ipa_res->tx_comp_ring_size *
				      sizeof(qdf_dma_addr_t);
	pipe_in.u.dl.ce_ring_base_pa = ipa_res->ce_sr_base_paddr;
	pipe_in.u.dl.ce_door_bell_pa = ipa_res->ce_reg_paddr;
	pipe_in.u.dl.ce_ring_size = ipa_res->ce_sr_ring_size;
	pipe_in.u.dl.num_tx_buffers = ipa_res->tx_num_alloc_buffer;

	/* Connect WDI IPA PIPE */
	ret = ipa_connect_wdi_pipe(&pipe_in, &pipe_out);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "ipa_connect_wdi_pipe: Tx pipe setup failed: ret=%d", ret);
		return QDF_STATUS_E_FAILURE;
	}

	/* Micro Controller Doorbell register */
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		"%s CONS DB pipe out 0x%x TX PIPE Handle 0x%x",
		__func__, (unsigned int)pipe_out.uc_door_bell_pa,
		pipe_out.clnt_hdl);
	ipa_res->tx_comp_doorbell_paddr = pipe_out.uc_door_bell_pa;
	/* WLAN TX PIPE Handle */
	ipa_res->tx_pipe_handle = pipe_out.clnt_hdl;
	*p_tx_pipe_handle = pipe_out.clnt_hdl;
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		"TX: %s 0x%x, %s %d, %s 0x%x, %s 0x%x, %s %d, %sNB %d, %s 0x%x",
		"comp_ring_base_pa",
		(unsigned int)pipe_in.u.dl.comp_ring_base_pa,
		"comp_ring_size",
		pipe_in.u.dl.comp_ring_size,
		"ce_ring_base_pa",
		(unsigned int)pipe_in.u.dl.ce_ring_base_pa,
		"ce_door_bell_pa",
		(unsigned int)pipe_in.u.dl.ce_door_bell_pa,
		"ce_ring_size",
		pipe_in.u.dl.ce_ring_size,
		"num_tx_buffers",
		pipe_in.u.dl.num_tx_buffers,
		"tx_comp_doorbell_paddr",
		(unsigned int)ipa_res->tx_comp_doorbell_paddr);

	/* RX PIPE */
	pipe_in.sys.ipa_ep_cfg.nat.nat_en = IPA_BYPASS_NAT;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_len = OL_TXRX_IPA_UC_WLAN_RX_HDR_LEN;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_ofst_metadata_valid = 0;
	pipe_in.sys.ipa_ep_cfg.hdr.hdr_metadata_reg_valid = 1;
	pipe_in.sys.ipa_ep_cfg.mode.mode = IPA_BASIC;
	pipe_in.sys.client = IPA_CLIENT_WLAN1_PROD;
	pipe_in.sys.desc_fifo_sz = ipa_desc_size + sizeof(struct sps_iovec);
	pipe_in.sys.notify = ipa_w2i_cb;
	if (!is_rm_enabled) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			    "%s: IPA RM DISABLED, IPA AWAKE", __func__);
		pipe_in.sys.keep_ipa_awake = true;
	}

	pipe_in.u.ul.rdy_ring_base_pa = ipa_res->rx_rdy_ring_base_paddr;
	pipe_in.u.ul.rdy_ring_size = ipa_res->rx_rdy_ring_size;
	pipe_in.u.ul.rdy_ring_rp_pa = ipa_res->rx_proc_done_idx_paddr;
	OL_TXRX_IPA_WDI2_SET(pipe_in, ipa_res);

#ifdef FEATURE_METERING
	pipe_in.wdi_notify = ipa_wdi_meter_notifier_cb;
#endif

	ret = ipa_connect_wdi_pipe(&pipe_in, &pipe_out);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "ipa_connect_wdi_pipe: Rx pipe setup failed: ret=%d", ret);
		return QDF_STATUS_E_FAILURE;
	}
	ipa_res->rx_ready_doorbell_paddr = pipe_out.uc_door_bell_pa;
	ipa_res->rx_pipe_handle = pipe_out.clnt_hdl;
	*p_rx_pipe_handle = pipe_out.clnt_hdl;
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "RX: %s 0x%x, %s %d, %s 0x%x, %s 0x%x",
		  "rdy_ring_base_pa",
		  (unsigned int)pipe_in.u.ul.rdy_ring_base_pa,
		  "rdy_ring_size",
		  pipe_in.u.ul.rdy_ring_size,
		  "rdy_ring_rp_pa",
		  (unsigned int)pipe_in.u.ul.rdy_ring_rp_pa,
		  "rx_ready_doorbell_paddr",
		  (unsigned int)ipa_res->rx_ready_doorbell_paddr);

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
	ret = ipa_disconnect_wdi_pipe(tx_pipe_handle);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
		    "ipa_disconnect_wdi_pipe: Tx pipe cleanup failed: ret=%d",
		    ret);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		    "%s: Disconnect RX PIPE rx_pipe_handle=0x%x",
		    __func__, rx_pipe_handle);
	ret = ipa_disconnect_wdi_pipe(rx_pipe_handle);
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
	struct ipa_ioc_get_hdr hdrlookup;
	int ret = 0, len;
	struct ipa_ioc_del_hdr *ipa_hdr;

	qdf_mem_zero(&hdrlookup, sizeof(hdrlookup));
	strlcpy(hdrlookup.name, name, sizeof(hdrlookup.name));
	ret = ipa_get_hdr(&hdrlookup);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "Hdr deleted already %s, %d", name, ret);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG, "hdl: 0x%x",
		  hdrlookup.hdl);
	len = sizeof(struct ipa_ioc_del_hdr) + sizeof(struct ipa_hdr_del) * 1;
	ipa_hdr = (struct ipa_ioc_del_hdr *)qdf_mem_malloc(len);
	if (ipa_hdr == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "ipa_hdr allocation failed");
		return QDF_STATUS_E_FAILURE;
	}
	ipa_hdr->num_hdls = 1;
	ipa_hdr->commit = 0;
	ipa_hdr->hdl[0].hdl = hdrlookup.hdl;
	ipa_hdr->hdl[0].status = -1;
	ret = ipa_del_hdr(ipa_hdr);
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
	struct ipa_ioc_add_hdr *ipa_hdr = NULL;
	int ret = -EINVAL;
	struct ol_txrx_ipa_uc_tx_hdr *uc_tx_hdr = NULL;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "Add Partial hdr: %s, %pM", ifname, mac_addr);

	/* dynamically allocate the memory to add the hdrs */
	ipa_hdr = qdf_mem_malloc(sizeof(struct ipa_ioc_add_hdr)
				 + sizeof(struct ipa_hdr_add));
	if (!ipa_hdr) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			    "%s: ipa_hdr allocation failed", ifname);
		ret = -ENOMEM;
		goto end;
	}

	ipa_hdr->commit = 0;
	ipa_hdr->num_hdrs = 1;

	uc_tx_hdr = (struct ol_txrx_ipa_uc_tx_hdr *)ipa_hdr->hdr[0].hdr;
	memcpy(uc_tx_hdr, &ipa_uc_tx_hdr, OL_TXRX_IPA_UC_WLAN_TX_HDR_LEN);
	memcpy(uc_tx_hdr->eth.h_source, mac_addr, ETH_ALEN);
	uc_tx_hdr->ipa_hd.vdev_id = session_id;
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "ifname=%s, vdev_id=%d",
		  ifname, uc_tx_hdr->ipa_hd.vdev_id);
	snprintf(ipa_hdr->hdr[0].name, IPA_RESOURCE_NAME_MAX, "%s%s",
		 ifname, OL_TXRX_IPA_IPV4_NAME_EXT);
	ipa_hdr->hdr[0].hdr_len = OL_TXRX_IPA_UC_WLAN_TX_HDR_LEN;
	ipa_hdr->hdr[0].type = IPA_HDR_L2_ETHERNET_II;
	ipa_hdr->hdr[0].is_partial = 1;
	ipa_hdr->hdr[0].hdr_hdl = 0;
	ipa_hdr->hdr[0].is_eth2_ofst_valid = 1;
	ipa_hdr->hdr[0].eth2_ofst = OL_TXRX_IPA_UC_WLAN_HDR_DES_MAC_OFFSET;

	ret = ipa_add_hdr(ipa_hdr);
	if (ret) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s IPv4 add hdr failed: %d", ifname, ret);
		goto end;
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
		  "%s: IPv4 hdr_hdl: 0x%x",
		  ipa_hdr->hdr[0].name, ipa_hdr->hdr[0].hdr_hdl);

	if (is_ipv6_enabled) {
		snprintf(ipa_hdr->hdr[0].name, IPA_RESOURCE_NAME_MAX, "%s%s",
			 ifname, OL_TXRX_IPA_IPV6_NAME_EXT);

		uc_tx_hdr = (struct ol_txrx_ipa_uc_tx_hdr *)ipa_hdr->hdr[0].hdr;
		uc_tx_hdr->eth.h_proto = cpu_to_be16(ETH_P_IPV6);

		ret = ipa_add_hdr(ipa_hdr);
		if (ret) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				    "%s: IPv6 add hdr failed: %d", ifname, ret);
			goto clean_ipv4_hdr;
		}

		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			  "%s: IPv6 hdr_hdl: 0x%x",
			  ipa_hdr->hdr[0].name, ipa_hdr->hdr[0].hdr_hdl);
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
					  enum ipa_client_type prod_client,
					  enum ipa_client_type cons_client,
					  uint8_t session_id,
					  bool is_ipv6_enabled)
{
	struct ipa_tx_intf tx_intf;
	struct ipa_rx_intf rx_intf;
	struct ipa_ioc_tx_intf_prop *tx_prop = NULL;
	struct ipa_ioc_rx_intf_prop *rx_prop = NULL;

	char ipv4_hdr_name[IPA_RESOURCE_NAME_MAX];
	char ipv6_hdr_name[IPA_RESOURCE_NAME_MAX];

	int num_prop = 1;
	int ret = 0;

	if (is_ipv6_enabled)
		num_prop++;

	/* Allocate TX properties for TOS categories, 1 each for IPv4 & IPv6 */
	tx_prop =
		qdf_mem_malloc(sizeof(struct ipa_ioc_tx_intf_prop) * num_prop);
	if (!tx_prop) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "tx_prop allocation failed");
		goto register_interface_fail;
	}

	/* Allocate RX properties, 1 each for IPv4 & IPv6 */
	rx_prop =
		qdf_mem_malloc(sizeof(struct ipa_ioc_rx_intf_prop) * num_prop);
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

	rx_prop[IPA_IP_v4].ip = IPA_IP_v4;
	rx_prop[IPA_IP_v4].src_pipe = prod_client;
	rx_prop[IPA_IP_v4].hdr_l2_type = IPA_HDR_L2_ETHERNET_II;
	rx_prop[IPA_IP_v4].attrib.attrib_mask = IPA_FLT_META_DATA;

	/*
	 * Interface ID is 3rd byte in the CLD header. Add the meta data and
	 * mask to identify the interface in IPA hardware
	 */
	rx_prop[IPA_IP_v4].attrib.meta_data =
		htonl(session_id << 16);
	rx_prop[IPA_IP_v4].attrib.meta_data_mask = htonl(0x00FF0000);

	rx_intf.num_props++;
	if (is_ipv6_enabled) {
		rx_prop[IPA_IP_v6].ip = IPA_IP_v6;
		rx_prop[IPA_IP_v6].src_pipe = prod_client;
		rx_prop[IPA_IP_v6].hdr_l2_type = IPA_HDR_L2_ETHERNET_II;
		rx_prop[IPA_IP_v6].attrib.attrib_mask = IPA_FLT_META_DATA;
		rx_prop[IPA_IP_v6].attrib.meta_data =
			htonl(session_id << 16);
		rx_prop[IPA_IP_v4].attrib.meta_data_mask = htonl(0x00FF0000);

		rx_intf.num_props++;
	}

	tx_prop[IPA_IP_v4].ip = IPA_IP_v4;
	tx_prop[IPA_IP_v4].hdr_l2_type = IPA_HDR_L2_ETHERNET_II;
	tx_prop[IPA_IP_v4].dst_pipe = IPA_CLIENT_WLAN1_CONS;
	tx_prop[IPA_IP_v4].alt_dst_pipe = cons_client;
	strlcpy(tx_prop[IPA_IP_v4].hdr_name, ipv4_hdr_name,
			IPA_RESOURCE_NAME_MAX);
	tx_intf.num_props++;

	if (is_ipv6_enabled) {
		tx_prop[IPA_IP_v6].ip = IPA_IP_v6;
		tx_prop[IPA_IP_v6].hdr_l2_type = IPA_HDR_L2_ETHERNET_II;
		tx_prop[IPA_IP_v6].dst_pipe = IPA_CLIENT_WLAN1_CONS;
		tx_prop[IPA_IP_v6].alt_dst_pipe = cons_client;
		strlcpy(tx_prop[IPA_IP_v6].hdr_name, ipv6_hdr_name,
				IPA_RESOURCE_NAME_MAX);
		tx_intf.num_props++;
	}

	tx_intf.prop = tx_prop;
	rx_intf.prop = rx_prop;

	/* Call the ipa api to register interface */
	ret = ipa_register_intf(ifname, &tx_intf, &rx_intf);

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
				   enum ipa_client_type prod_client,
				   enum ipa_client_type cons_client,
				   uint8_t session_id, bool is_ipv6_enabled)
{
	int ret;

	ret = ol_txrx_ipa_add_header_info(ifname, mac_addr, session_id,
					  is_ipv6_enabled);
	if (ret)
		return QDF_STATUS_E_FAILURE;

	/* Configure the TX and RX pipes filter rules */
	ret = ol_txrx_ipa_register_interface(ifname, prod_client, cons_client,
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
	ret = ipa_deregister_intf(ifname);
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

	/* ACTIVATE TX PIPE */
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Enable TX PIPE(tx_pipe_handle=%d)",
			__func__, ipa_res->tx_pipe_handle);
	result = ipa_enable_wdi_pipe(ipa_res->tx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Enable TX PIPE fail, code %d",
				__func__, result);
		return QDF_STATUS_E_FAILURE;
	}
	result = ipa_resume_wdi_pipe(ipa_res->tx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Resume TX PIPE fail, code %d",
				__func__, result);
		return QDF_STATUS_E_FAILURE;
	}
	ol_txrx_ipa_uc_set_active((struct cdp_pdev *)pdev, true, true);

	/* ACTIVATE RX PIPE */
	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Enable RX PIPE(rx_pipe_handle=%d)",
			__func__, ipa_res->rx_pipe_handle);
	result = ipa_enable_wdi_pipe(ipa_res->rx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Enable RX PIPE fail, code %d",
				__func__, result);
		return QDF_STATUS_E_FAILURE;
	}
	result = ipa_resume_wdi_pipe(ipa_res->rx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Resume RX PIPE fail, code %d",
				__func__, result);
		return QDF_STATUS_E_FAILURE;
	}
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
	struct ol_txrx_ipa_resources *ipa_res = &pdev->ipa_resource;
	int result;

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Disable RX PIPE", __func__);
	result = ipa_suspend_wdi_pipe(ipa_res->rx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Suspend RX PIPE fail, code %d",
				__func__, result);
		return QDF_STATUS_E_FAILURE;
	}

	result = ipa_disable_wdi_pipe(ipa_res->rx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Disable RX PIPE fail, code %d",
				__func__, result);
		return QDF_STATUS_E_FAILURE;
	}

	QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_DEBUG,
			"%s: Disable TX PIPE", __func__);
	result = ipa_suspend_wdi_pipe(ipa_res->tx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Suspend TX PIPE fail, code %d",
				__func__, result);
		return QDF_STATUS_E_FAILURE;
	}
	result = ipa_disable_wdi_pipe(ipa_res->tx_pipe_handle);
	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: Disable TX PIPE fail, code %d",
				__func__, result);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
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
	struct ipa_rm_perf_profile profile;
	int result;

	profile.max_supported_bandwidth_mbps = max_supported_bw_mbps;
	result = ipa_rm_set_perf_profile(client, &profile);

	if (result) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"Set perf profile failed, code %d", result);

		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

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
