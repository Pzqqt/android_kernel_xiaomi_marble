/*
 * Copyright (c) 2011, 2014-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * @file htt.c
 * @brief Provide functions to create+init and destroy a HTT instance.
 * @details
 *  This file contains functions for creating a HTT instance; initializing
 *  the HTT instance, e.g. by allocating a pool of HTT tx descriptors and
 *  connecting the HTT service with HTC; and deleting a HTT instance.
 */

#include <qdf_mem.h>         /* qdf_mem_malloc */
#include <qdf_types.h>          /* qdf_device_t, qdf_print */

#include <htt.h>                /* htt_tx_msdu_desc_t */
#include <ol_cfg.h>
#include <ol_txrx_htt_api.h>    /* ol_tx_dowload_done_ll, etc. */
#include <ol_htt_api.h>

#include <htt_internal.h>
#include <ol_htt_tx_api.h>
#include <cds_api.h>
#include "hif.h"

#define HTT_HTC_PKT_POOL_INIT_SIZE 100  /* enough for a large A-MPDU */

QDF_STATUS(*htt_h2t_rx_ring_cfg_msg)(struct htt_pdev_t *pdev);
QDF_STATUS(*htt_h2t_rx_ring_rfs_cfg_msg)(struct htt_pdev_t *pdev);

#ifdef IPA_OFFLOAD
A_STATUS htt_ipa_config(htt_pdev_handle pdev, A_STATUS status)
{
	if ((A_OK == status) &&
	    ol_cfg_ipa_uc_offload_enabled(pdev->ctrl_pdev))
		status = htt_h2t_ipa_uc_rsc_cfg_msg(pdev);
	return status;
}

#define HTT_IPA_CONFIG htt_ipa_config
#else
#define HTT_IPA_CONFIG(pdev, status) status     /* no-op */
#endif /* IPA_OFFLOAD */

struct htt_htc_pkt *htt_htc_pkt_alloc(struct htt_pdev_t *pdev)
{
	struct htt_htc_pkt_union *pkt = NULL;

	HTT_TX_MUTEX_ACQUIRE(&pdev->htt_tx_mutex);
	if (pdev->htt_htc_pkt_freelist) {
		pkt = pdev->htt_htc_pkt_freelist;
		pdev->htt_htc_pkt_freelist = pdev->htt_htc_pkt_freelist->u.next;
	}
	HTT_TX_MUTEX_RELEASE(&pdev->htt_tx_mutex);

	if (pkt == NULL)
		pkt = qdf_mem_malloc(sizeof(*pkt));

	return &pkt->u.pkt;     /* not actually a dereference */
}

void htt_htc_pkt_free(struct htt_pdev_t *pdev, struct htt_htc_pkt *pkt)
{
	struct htt_htc_pkt_union *u_pkt = (struct htt_htc_pkt_union *)pkt;

	HTT_TX_MUTEX_ACQUIRE(&pdev->htt_tx_mutex);
	u_pkt->u.next = pdev->htt_htc_pkt_freelist;
	pdev->htt_htc_pkt_freelist = u_pkt;
	HTT_TX_MUTEX_RELEASE(&pdev->htt_tx_mutex);
}

void htt_htc_pkt_pool_free(struct htt_pdev_t *pdev)
{
	struct htt_htc_pkt_union *pkt, *next;
	pkt = pdev->htt_htc_pkt_freelist;
	while (pkt) {
		next = pkt->u.next;
		qdf_mem_free(pkt);
		pkt = next;
	}
	pdev->htt_htc_pkt_freelist = NULL;
}

#ifdef ATH_11AC_TXCOMPACT
void htt_htc_misc_pkt_list_add(struct htt_pdev_t *pdev, struct htt_htc_pkt *pkt)
{
	struct htt_htc_pkt_union *u_pkt = (struct htt_htc_pkt_union *)pkt;

	HTT_TX_MUTEX_ACQUIRE(&pdev->htt_tx_mutex);
	if (pdev->htt_htc_pkt_misclist) {
		u_pkt->u.next = pdev->htt_htc_pkt_misclist;
		pdev->htt_htc_pkt_misclist = u_pkt;
	} else {
		pdev->htt_htc_pkt_misclist = u_pkt;
	}
	HTT_TX_MUTEX_RELEASE(&pdev->htt_tx_mutex);
}

void htt_htc_misc_pkt_pool_free(struct htt_pdev_t *pdev)
{
	struct htt_htc_pkt_union *pkt, *next;
	qdf_nbuf_t netbuf;
	pkt = pdev->htt_htc_pkt_misclist;

	while (pkt) {
		next = pkt->u.next;
		netbuf = (qdf_nbuf_t) (pkt->u.pkt.htc_pkt.pNetBufContext);
		qdf_nbuf_unmap(pdev->osdev, netbuf, QDF_DMA_TO_DEVICE);
		qdf_nbuf_free(netbuf);
		qdf_mem_free(pkt);
		pkt = next;
	}
	pdev->htt_htc_pkt_misclist = NULL;
}
#endif


/* AR6004 don't need HTT layer. */
#ifdef AR6004_HW
#define NO_HTT_NEEDED true
#else
#define NO_HTT_NEEDED false
#endif

#if defined(QCA_TX_HTT2_SUPPORT) && defined(CONFIG_HL_SUPPORT)

/**
 * htt_htc_tx_htt2_service_start() - Start TX HTT2 service
 *
 * @pdev: pointer to htt device.
 * @connect_req: pointer to service connection request information
 * @connect_resp: pointer to service connection response information
 *
 *
 * Return: None
 */
static void
htt_htc_tx_htt2_service_start(struct htt_pdev_t *pdev,
			      HTC_SERVICE_CONNECT_REQ *connect_req,
			      HTC_SERVICE_CONNECT_RESP *connect_resp)
{
	A_STATUS status;

	qdf_mem_set(connect_req, 0, sizeof(HTC_SERVICE_CONNECT_REQ));
	qdf_mem_set(connect_resp, 0, sizeof(HTC_SERVICE_CONNECT_RESP));

	/* The same as HTT service but no RX. */
	connect_req->EpCallbacks.pContext = pdev;
	connect_req->EpCallbacks.EpTxComplete = htt_h2t_send_complete;
	connect_req->EpCallbacks.EpSendFull = htt_h2t_full;
	connect_req->MaxSendQueueDepth = HTT_MAX_SEND_QUEUE_DEPTH;
	/* Should NOT support credit flow control. */
	connect_req->ConnectionFlags |=
				HTC_CONNECT_FLAGS_DISABLE_CREDIT_FLOW_CTRL;
	/* Enable HTC schedule mechanism for TX HTT2 service. */
	connect_req->ConnectionFlags |= HTC_CONNECT_FLAGS_ENABLE_HTC_SCHEDULE;

	connect_req->service_id = HTT_DATA2_MSG_SVC;

	status = htc_connect_service(pdev->htc_pdev, connect_req, connect_resp);

	if (status != A_OK) {
		pdev->htc_tx_htt2_endpoint = ENDPOINT_UNUSED;
		pdev->htc_tx_htt2_max_size = 0;
	} else {
		pdev->htc_tx_htt2_endpoint = connect_resp->Endpoint;
		pdev->htc_tx_htt2_max_size = HTC_TX_HTT2_MAX_SIZE;
	}

	qdf_print("TX HTT %s, ep %d size %d\n",
		  (status == A_OK ? "ON" : "OFF"),
		  pdev->htc_tx_htt2_endpoint,
		  pdev->htc_tx_htt2_max_size);
}
#else

static inline void
htt_htc_tx_htt2_service_start(struct htt_pdev_t *pdev,
			      HTC_SERVICE_CONNECT_REQ *connect_req,
			      HTC_SERVICE_CONNECT_RESP *connect_resp)
{
	return;
}
#endif

/**
 * htt_htc_credit_flow_disable() - disable flow control for
 *				   HTT data message service
 *
 * @pdev: pointer to htt device.
 * @connect_req: pointer to service connection request information
 *
 * HTC Credit mechanism is disabled based on
 * default_tx_comp_req as throughput will be lower
 * if we disable htc credit mechanism with default_tx_comp_req
 * set since txrx download packet will be limited by ota
 * completion.
 *
 * Return: None
 */
static
void htt_htc_credit_flow_disable(struct htt_pdev_t *pdev,
				 HTC_SERVICE_CONNECT_REQ *connect_req)
{
	if (pdev->osdev->bus_type == QDF_BUS_TYPE_SDIO) {
		/*
		 * TODO:Conditional disabling will be removed once firmware
		 * with reduced tx completion is pushed into release builds.
		 */
		if (!pdev->cfg.default_tx_comp_req)
			connect_req->ConnectionFlags |=
			HTC_CONNECT_FLAGS_DISABLE_CREDIT_FLOW_CTRL;
	} else {
		connect_req->ConnectionFlags |=
			HTC_CONNECT_FLAGS_DISABLE_CREDIT_FLOW_CTRL;
	}
}

#if defined(DEBUG_HL_LOGGING) && defined(CONFIG_HL_SUPPORT)

/**
 * htt_dump_bundle_stats() - dump wlan stats
 * @pdev: handle to the HTT instance
 *
 * Return: None
 */
void htt_dump_bundle_stats(htt_pdev_handle pdev)
{
	htc_dump_bundle_stats(pdev->htc_pdev);
}

/**
 * htt_clear_bundle_stats() - clear wlan stats
 * @pdev: handle to the HTT instance
 *
 * Return: None
 */
void htt_clear_bundle_stats(htt_pdev_handle pdev)
{
	htc_clear_bundle_stats(pdev->htc_pdev);
}
#endif

/**
 * htt_pdev_alloc() - allocate HTT pdev
 * @txrx_pdev: txrx pdev
 * @ctrl_pdev: cfg pdev
 * @htc_pdev: HTC pdev
 * @osdev: os device
 *
 * Return: HTT pdev handle
 */
htt_pdev_handle
htt_pdev_alloc(ol_txrx_pdev_handle txrx_pdev,
	   ol_pdev_handle ctrl_pdev,
	   HTC_HANDLE htc_pdev, qdf_device_t osdev)
{
	struct htt_pdev_t *pdev;
	struct hif_opaque_softc *osc =  cds_get_context(QDF_MODULE_ID_HIF);

	if (!osc)
		goto fail1;

	pdev = qdf_mem_malloc(sizeof(*pdev));
	if (!pdev)
		goto fail1;

	pdev->osdev = osdev;
	pdev->ctrl_pdev = ctrl_pdev;
	pdev->txrx_pdev = txrx_pdev;
	pdev->htc_pdev = htc_pdev;

	qdf_mem_set(&pdev->stats, sizeof(pdev->stats), 0);
	pdev->htt_htc_pkt_freelist = NULL;
#ifdef ATH_11AC_TXCOMPACT
	pdev->htt_htc_pkt_misclist = NULL;
#endif

	/* for efficiency, store a local copy of the is_high_latency flag */
	pdev->cfg.is_high_latency = ol_cfg_is_high_latency(pdev->ctrl_pdev);
	pdev->cfg.default_tx_comp_req =
			!ol_cfg_tx_free_at_download(pdev->ctrl_pdev);

	pdev->cfg.is_full_reorder_offload =
			ol_cfg_is_full_reorder_offload(pdev->ctrl_pdev);
	qdf_print("is_full_reorder_offloaded? %d\n",
		  (int)pdev->cfg.is_full_reorder_offload);

	pdev->cfg.ce_classify_enabled =
		ol_cfg_is_ce_classify_enabled(ctrl_pdev);
	qdf_print("ce_classify_enabled %d\n",
		  pdev->cfg.ce_classify_enabled);

	if (pdev->cfg.is_high_latency) {
		qdf_atomic_init(&pdev->htt_tx_credit.target_delta);
		qdf_atomic_init(&pdev->htt_tx_credit.bus_delta);
		qdf_atomic_add(HTT_MAX_BUS_CREDIT,
			       &pdev->htt_tx_credit.bus_delta);
	}

	pdev->targetdef = htc_get_targetdef(htc_pdev);
#if defined(HELIUMPLUS_PADDR64)
	/* TODO: OKA: Remove hard-coding */
	HTT_SET_WIFI_IP(pdev, 2, 0);
#endif /* defined(HELIUMPLUS_PADDR64) */

	if (NO_HTT_NEEDED)
		goto success;
	/*
	 * Connect to HTC service.
	 * This has to be done before calling htt_rx_attach,
	 * since htt_rx_attach involves sending a rx ring configure
	 * message to the target.
	 */
	if (htt_htc_attach(pdev, HTT_DATA_MSG_SVC))
		goto fail2;
	if (htt_htc_attach(pdev, HTT_DATA2_MSG_SVC))
		;
	/* TODO: enable the following line once FW is ready */
	/* goto fail2; */
	if (htt_htc_attach(pdev, HTT_DATA3_MSG_SVC))
		;
	/* TODO: enable the following line once FW is ready */
	/* goto fail2; */
	if (hif_ce_fastpath_cb_register(osc, htt_t2h_msg_handler_fast, pdev))
		qdf_print("failed to register fastpath callback\n");

success:
	return pdev;

fail2:
	qdf_mem_free(pdev);

fail1:
	return NULL;

}

/**
 * htt_attach() - Allocate and setup HTT TX/RX descriptors
 * @pdev: pdev ptr
 * @desc_pool_size: size of tx descriptors
 *
 * Return: 0 for success or error code.
 */
int
htt_attach(struct htt_pdev_t *pdev, int desc_pool_size)
{
	int i;
	int ret = 0;

	ret = htt_tx_attach(pdev, desc_pool_size);
	if (ret)
		goto fail1;

	ret = htt_rx_attach(pdev);
	if (ret)
		goto fail2;

	HTT_TX_MUTEX_INIT(&pdev->htt_tx_mutex);
	HTT_TX_NBUF_QUEUE_MUTEX_INIT(pdev);

	/* pre-allocate some HTC_PACKET objects */
	for (i = 0; i < HTT_HTC_PKT_POOL_INIT_SIZE; i++) {
		struct htt_htc_pkt_union *pkt;
		pkt = qdf_mem_malloc(sizeof(*pkt));
		if (!pkt)
			break;
		htt_htc_pkt_free(pdev, &pkt->u.pkt);
	}

	if (pdev->cfg.is_high_latency) {
		/*
		 * HL - download the whole frame.
		 * Specify a download length greater than the max MSDU size,
		 * so the downloads will be limited by the actual frame sizes.
		 */
		pdev->download_len = 5000;

		if (ol_cfg_tx_free_at_download(pdev->ctrl_pdev))
			pdev->tx_send_complete_part2 =
						ol_tx_download_done_hl_free;
		else
			pdev->tx_send_complete_part2 =
						ol_tx_download_done_hl_retain;

		/*
		 * CHECK THIS LATER: does the HL HTT version of
		 * htt_rx_mpdu_desc_list_next
		 * (which is not currently implemented) present the
		 * adf_nbuf_data(rx_ind_msg)
		 * as the abstract rx descriptor?
		 * If not, the rx_fw_desc_offset initialization
		 * here will have to be adjusted accordingly.
		 * NOTE: for HL, because fw rx desc is in ind msg,
		 * not in rx desc, so the
		 * offset should be negtive value
		 */
		pdev->rx_fw_desc_offset =
			HTT_ENDIAN_BYTE_IDX_SWAP(
					HTT_RX_IND_FW_RX_DESC_BYTE_OFFSET
					- HTT_RX_IND_HL_BYTES);

		htt_h2t_rx_ring_cfg_msg = htt_h2t_rx_ring_cfg_msg_hl;
		htt_h2t_rx_ring_rfs_cfg_msg = htt_h2t_rx_ring_rfs_cfg_msg_hl;

		/* initialize the txrx credit count */
		ol_tx_target_credit_update(
				pdev->txrx_pdev, ol_cfg_target_tx_credit(
					pdev->ctrl_pdev));
	} else {
		enum wlan_frm_fmt frm_type;

		/*
		 * LL - download just the initial portion of the frame.
		 * Download enough to cover the encapsulation headers checked
		 * by the target's tx classification descriptor engine.
		 *
		 * For LL, the FW rx desc directly referenced at its location
		 * inside the rx indication message.
		 */

		/* account for the 802.3 or 802.11 header */
		frm_type = ol_cfg_frame_type(pdev->ctrl_pdev);

		if (frm_type == wlan_frm_fmt_native_wifi) {
			pdev->download_len = HTT_TX_HDR_SIZE_NATIVE_WIFI;
		} else if (frm_type == wlan_frm_fmt_802_3) {
			pdev->download_len = HTT_TX_HDR_SIZE_ETHERNET;
		} else {
			qdf_print("Unexpected frame type spec: %d\n", frm_type);
			HTT_ASSERT0(0);
		}

		/*
		 * Account for the optional L2 / ethernet header fields:
		 * 802.1Q, LLC/SNAP
		 */
		pdev->download_len +=
			HTT_TX_HDR_SIZE_802_1Q + HTT_TX_HDR_SIZE_LLC_SNAP;

		/*
		 * Account for the portion of the L3 (IP) payload that the
		 * target needs for its tx classification.
		 */
		pdev->download_len += ol_cfg_tx_download_size(pdev->ctrl_pdev);

		/*
		 * Account for the HTT tx descriptor, including the
		 * HTC header + alignment padding.
		 */
		pdev->download_len += sizeof(struct htt_host_tx_desc_t);

		/*
		 * The TXCOMPACT htt_tx_sched function uses pdev->download_len
		 * to apply for all requeued tx frames.  Thus,
		 * pdev->download_len has to be the largest download length of
		 * any tx frame that will be downloaded.
		 * This maximum download length is for management tx frames,
		 * which have an 802.11 header.
		 */
#ifdef ATH_11AC_TXCOMPACT
		pdev->download_len = sizeof(struct htt_host_tx_desc_t)
			+ HTT_TX_HDR_SIZE_OUTER_HDR_MAX /* worst case */
			+ HTT_TX_HDR_SIZE_802_1Q
			+ HTT_TX_HDR_SIZE_LLC_SNAP
			+ ol_cfg_tx_download_size(pdev->ctrl_pdev);
#endif
		pdev->tx_send_complete_part2 = ol_tx_download_done_ll;

		/*
		 * For LL, the FW rx desc is alongside the HW rx desc fields in
		 * the htt_host_rx_desc_base struct/.
		 */
		pdev->rx_fw_desc_offset = RX_STD_DESC_FW_MSDU_OFFSET;

		htt_h2t_rx_ring_cfg_msg = htt_h2t_rx_ring_cfg_msg_ll;
		htt_h2t_rx_ring_rfs_cfg_msg = htt_h2t_rx_ring_rfs_cfg_msg_ll;
	}

	return 0;

fail2:
	htt_tx_detach(pdev);

fail1:
	return ret;
}

A_STATUS htt_attach_target(htt_pdev_handle pdev)
{
	A_STATUS status;

	status = htt_h2t_ver_req_msg(pdev);
	if (status != A_OK)
		return status;

#if defined(HELIUMPLUS_PADDR64)
	/*
	 * Send the frag_desc info to target.
	 */
	htt_h2t_frag_desc_bank_cfg_msg(pdev);
#endif /* defined(HELIUMPLUS_PADDR64) */


	/*
	 * If applicable, send the rx ring config message to the target.
	 * The host could wait for the HTT version number confirmation message
	 * from the target before sending any further HTT messages, but it's
	 * reasonable to assume that the host and target HTT version numbers
	 * match, and proceed immediately with the remaining configuration
	 * handshaking.
	 */

	status = htt_h2t_rx_ring_rfs_cfg_msg(pdev);
	status = htt_h2t_rx_ring_cfg_msg(pdev);
	status = HTT_IPA_CONFIG(pdev, status);

	return status;
}

void htt_detach(htt_pdev_handle pdev)
{
	htt_rx_detach(pdev);
	htt_tx_detach(pdev);
	htt_htc_pkt_pool_free(pdev);
#ifdef ATH_11AC_TXCOMPACT
	htt_htc_misc_pkt_pool_free(pdev);
#endif
	HTT_TX_MUTEX_DESTROY(&pdev->htt_tx_mutex);
	HTT_TX_NBUF_QUEUE_MUTEX_DESTROY(pdev);
	htt_rx_dbg_rxbuf_deinit(pdev);
}

/**
 * htt_pdev_free() - Free HTT pdev
 * @pdev: htt pdev
 *
 * Return: none
 */
void htt_pdev_free(htt_pdev_handle pdev)
{
	qdf_mem_free(pdev);
}

void htt_detach_target(htt_pdev_handle pdev)
{
}

static inline
int htt_update_endpoint(struct htt_pdev_t *pdev,
			uint16_t service_id, HTC_ENDPOINT_ID ep)
{
	struct hif_opaque_softc *hif_ctx;
	uint8_t ul = 0xff, dl = 0xff;
	int     ul_polled, dl_polled;
	int     tx_service = 0;
	int     rc = 0;

	hif_ctx = cds_get_context(QDF_MODULE_ID_HIF);
	if (qdf_unlikely(NULL == hif_ctx)) {
		QDF_ASSERT(NULL != hif_ctx);
		qdf_print("%s:%d: assuming non-tx service.",
			  __func__, __LINE__);
	} else {
		ul = dl = 0xff;
		if (QDF_STATUS_SUCCESS !=
		    hif_map_service_to_pipe(hif_ctx, service_id,
					    &ul, &dl,
					    &ul_polled, &dl_polled))
			qdf_print("%s:%d: assuming non-tx srv.",
				  __func__, __LINE__);
		else
			tx_service = (ul != 0xff);
	}
	if (tx_service) {
		/* currently we have only one OUT htt tx service */
		QDF_BUG(service_id == HTT_DATA_MSG_SVC);

		pdev->htc_tx_endpoint = ep;
		hif_save_htc_htt_config_endpoint(hif_ctx, ep);
		rc = 1;
	}
	return rc;
}

int htt_htc_attach(struct htt_pdev_t *pdev, uint16_t service_id)
{
	HTC_SERVICE_CONNECT_REQ connect;
	HTC_SERVICE_CONNECT_RESP response;
	A_STATUS status;

	qdf_mem_set(&connect, sizeof(connect), 0);
	qdf_mem_set(&response, sizeof(response), 0);

	connect.pMetaData = NULL;
	connect.MetaDataLength = 0;
	connect.EpCallbacks.pContext = pdev;
	connect.EpCallbacks.EpTxComplete = htt_h2t_send_complete;
	connect.EpCallbacks.EpTxCompleteMultiple = NULL;
	connect.EpCallbacks.EpRecv = htt_t2h_msg_handler;
	connect.EpCallbacks.ep_resume_tx_queue = htt_tx_resume_handler;

	/* rx buffers currently are provided by HIF, not by EpRecvRefill */
	connect.EpCallbacks.EpRecvRefill = NULL;
	connect.EpCallbacks.RecvRefillWaterMark = 1;
	/* N/A, fill is done by HIF */

	connect.EpCallbacks.EpSendFull = htt_h2t_full;
	/*
	 * Specify how deep to let a queue get before htc_send_pkt will
	 * call the EpSendFull function due to excessive send queue depth.
	 */
	connect.MaxSendQueueDepth = HTT_MAX_SEND_QUEUE_DEPTH;

	/* disable flow control for HTT data message service */
	htt_htc_credit_flow_disable(pdev, &connect);

	/* connect to control service */
	connect.service_id = service_id;

	status = htc_connect_service(pdev->htc_pdev, &connect, &response);

	if (status != A_OK)
		return -EIO;       /* failure */

	htt_update_endpoint(pdev, service_id, response.Endpoint);

	/* Start TX HTT2 service if the target support it. */
	htt_htc_tx_htt2_service_start(pdev, &connect, &response);

	return 0;               /* success */
}

#if HTT_DEBUG_LEVEL > 5
void htt_display(htt_pdev_handle pdev, int indent)
{
	qdf_print("%*s%s:\n", indent, " ", "HTT");
	qdf_print("%*stx desc pool: %d elems of %d bytes, %d allocated\n",
		  indent + 4, " ",
		  pdev->tx_descs.pool_elems,
		  pdev->tx_descs.size, pdev->tx_descs.alloc_cnt);
	qdf_print("%*srx ring: space for %d elems, filled with %d buffers\n",
		  indent + 4, " ",
		  pdev->rx_ring.size, pdev->rx_ring.fill_level);
	qdf_print("%*sat %p (%llx paddr)\n", indent + 8, " ",
		  pdev->rx_ring.buf.paddrs_ring,
		  (unsigned long long)pdev->rx_ring.base_paddr);
	qdf_print("%*snetbuf ring @ %p\n", indent + 8, " ",
		  pdev->rx_ring.buf.netbufs_ring);
	qdf_print("%*sFW_IDX shadow register: vaddr = %p, paddr = %llx\n",
		  indent + 8, " ",
		  pdev->rx_ring.alloc_idx.vaddr,
		  (unsigned long long)pdev->rx_ring.alloc_idx.paddr);
	qdf_print("%*sSW enqueue idx= %d, SW dequeue idx: desc= %d, buf= %d\n",
		  indent + 8, " ", *pdev->rx_ring.alloc_idx.vaddr,
		  pdev->rx_ring.sw_rd_idx.msdu_desc,
		  pdev->rx_ring.sw_rd_idx.msdu_payld);
}
#endif

#ifdef IPA_OFFLOAD
/**
 * htt_ipa_uc_attach() - Allocate UC data path resources
 * @pdev: handle to the HTT instance
 *
 * Return: 0 success
 *         none 0 fail
 */
int htt_ipa_uc_attach(struct htt_pdev_t *pdev)
{
	int error;

	/* TX resource attach */
	error = htt_tx_ipa_uc_attach(
		pdev,
		ol_cfg_ipa_uc_tx_buf_size(pdev->ctrl_pdev),
		ol_cfg_ipa_uc_tx_max_buf_cnt(pdev->ctrl_pdev),
		ol_cfg_ipa_uc_tx_partition_base(pdev->ctrl_pdev));
	if (error) {
		qdf_print("HTT IPA UC TX attach fail code %d\n", error);
		HTT_ASSERT0(0);
		return error;
	}

	/* RX resource attach */
	error = htt_rx_ipa_uc_attach(
		pdev, qdf_get_pwr2(pdev->rx_ring.fill_level));
	if (error) {
		qdf_print("HTT IPA UC RX attach fail code %d\n", error);
		htt_tx_ipa_uc_detach(pdev);
		HTT_ASSERT0(0);
		return error;
	}

	return 0;               /* success */
}

/**
 * htt_ipa_uc_attach() - Remove UC data path resources
 * @pdev: handle to the HTT instance
 *
 * Return: None
 */
void htt_ipa_uc_detach(struct htt_pdev_t *pdev)
{
	/* TX IPA micro controller detach */
	htt_tx_ipa_uc_detach(pdev);

	/* RX IPA micro controller detach */
	htt_rx_ipa_uc_detach(pdev);
}

/**
 * htt_ipa_uc_get_resource() - Get uc resource from htt and lower layer
 * @pdev: handle to the HTT instance
 * @ce_sr_base_paddr: copy engine source ring base physical address
 * @ce_sr_ring_size: copy engine source ring size
 * @ce_reg_paddr: copy engine register physical address
 * @tx_comp_ring_base_paddr: tx comp ring base physical address
 * @tx_comp_ring_size: tx comp ring size
 * @tx_num_alloc_buffer: number of allocated tx buffer
 * @rx_rdy_ring_base_paddr: rx ready ring base physical address
 * @rx_rdy_ring_size: rx ready ring size
 * @rx_proc_done_idx_paddr: rx process done index physical address
 * @rx_proc_done_idx_vaddr: rx process done index virtual address
 * @rx2_rdy_ring_base_paddr: rx done ring base physical address
 * @rx2_rdy_ring_size: rx done ring size
 * @rx2_proc_done_idx_paddr: rx done index physical address
 * @rx2_proc_done_idx_vaddr: rx done index virtual address
 *
 * Return: 0 success
 */
int
htt_ipa_uc_get_resource(htt_pdev_handle pdev,
			qdf_dma_addr_t *ce_sr_base_paddr,
			uint32_t *ce_sr_ring_size,
			qdf_dma_addr_t *ce_reg_paddr,
			qdf_dma_addr_t *tx_comp_ring_base_paddr,
			uint32_t *tx_comp_ring_size,
			uint32_t *tx_num_alloc_buffer,
			qdf_dma_addr_t *rx_rdy_ring_base_paddr,
			uint32_t *rx_rdy_ring_size,
			qdf_dma_addr_t *rx_proc_done_idx_paddr,
			void **rx_proc_done_idx_vaddr,
			qdf_dma_addr_t *rx2_rdy_ring_base_paddr,
			uint32_t *rx2_rdy_ring_size,
			qdf_dma_addr_t *rx2_proc_done_idx_paddr,
			void **rx2_proc_done_idx_vaddr)
{
	/* Release allocated resource to client */
	*tx_comp_ring_base_paddr =
		pdev->ipa_uc_tx_rsc.tx_comp_base.paddr;
	*tx_comp_ring_size =
		(uint32_t) ol_cfg_ipa_uc_tx_max_buf_cnt(pdev->ctrl_pdev);
	*tx_num_alloc_buffer = (uint32_t) pdev->ipa_uc_tx_rsc.alloc_tx_buf_cnt;
	*rx_rdy_ring_base_paddr =
		pdev->ipa_uc_rx_rsc.rx_ind_ring_base.paddr;
	*rx_rdy_ring_size = (uint32_t) pdev->ipa_uc_rx_rsc.rx_ind_ring_size;
	*rx_proc_done_idx_paddr =
		pdev->ipa_uc_rx_rsc.rx_ipa_prc_done_idx.paddr;
	*rx_proc_done_idx_vaddr =
		(void *)pdev->ipa_uc_rx_rsc.rx_ipa_prc_done_idx.vaddr;
	*rx2_rdy_ring_base_paddr =
		pdev->ipa_uc_rx_rsc.rx2_ind_ring_base.paddr;
	*rx2_rdy_ring_size = (uint32_t) pdev->ipa_uc_rx_rsc.rx2_ind_ring_size;
	*rx2_proc_done_idx_paddr =
		pdev->ipa_uc_rx_rsc.rx2_ipa_prc_done_idx.paddr;
	*rx2_proc_done_idx_vaddr =
		(void *)pdev->ipa_uc_rx_rsc.rx2_ipa_prc_done_idx.vaddr;

	/* Get copy engine, bus resource */
	htc_ipa_get_ce_resource(pdev->htc_pdev,
				ce_sr_base_paddr,
				ce_sr_ring_size, ce_reg_paddr);

	return 0;
}

/**
 * htt_ipa_uc_set_doorbell_paddr() - Propagate IPA doorbell address
 * @pdev: handle to the HTT instance
 * @ipa_uc_tx_doorbell_paddr: TX doorbell base physical address
 * @ipa_uc_rx_doorbell_paddr: RX doorbell base physical address
 *
 * Return: 0 success
 */
int
htt_ipa_uc_set_doorbell_paddr(htt_pdev_handle pdev,
			      qdf_dma_addr_t ipa_uc_tx_doorbell_paddr,
			      qdf_dma_addr_t ipa_uc_rx_doorbell_paddr)
{
	pdev->ipa_uc_tx_rsc.tx_comp_idx_paddr = ipa_uc_tx_doorbell_paddr;
	pdev->ipa_uc_rx_rsc.rx_rdy_idx_paddr = ipa_uc_rx_doorbell_paddr;
	return 0;
}
#endif /* IPA_OFFLOAD */

/**
 * htt_mark_first_wakeup_packet() - set flag to indicate that
 *    fw is compatible for marking first packet after wow wakeup
 * @pdev: pointer to htt pdev
 * @value: 1 for enabled/ 0 for disabled
 *
 * Return: None
 */
void htt_mark_first_wakeup_packet(htt_pdev_handle pdev,
			uint8_t value)
{
	if (!pdev) {
		qdf_print("%s: htt pdev is NULL", __func__);
		return;
	}

	pdev->cfg.is_first_wakeup_packet = value;
}

