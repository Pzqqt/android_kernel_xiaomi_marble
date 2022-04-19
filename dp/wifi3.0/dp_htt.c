/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2022 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <htt.h>
#include <hal_hw_headers.h>
#include <hal_api.h>
#include "dp_peer.h"
#include "dp_types.h"
#include "dp_internal.h"
#include "dp_rx.h"
#include "htt_stats.h"
#include "htt_ppdu_stats.h"
#include "dp_htt.h"
#ifdef WIFI_MONITOR_SUPPORT
#include <dp_mon.h>
#endif
#include "qdf_mem.h"   /* qdf_mem_malloc,free */
#include "cdp_txrx_cmn_struct.h"

#ifdef FEATURE_PERPKT_INFO
#include "dp_ratetable.h"
#endif
#include <qdf_module.h>

#define HTT_TLV_HDR_LEN HTT_T2H_EXT_STATS_CONF_TLV_HDR_SIZE

#define HTT_HTC_PKT_POOL_INIT_SIZE 64

#define HTT_MSG_BUF_SIZE(msg_bytes) \
	((msg_bytes) + HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING)

#define HTT_PID_BIT_MASK 0x3

#define DP_EXT_MSG_LENGTH 2048
#define HTT_HEADER_LEN 16
#define HTT_MGMT_CTRL_TLV_HDR_RESERVERD_LEN 16

#define HTT_SHIFT_UPPER_TIMESTAMP 32
#define HTT_MASK_UPPER_TIMESTAMP 0xFFFFFFFF00000000
#define HTT_BKP_STATS_MAX_QUEUE_DEPTH 16

struct dp_htt_htc_pkt *
htt_htc_pkt_alloc(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt_union *pkt = NULL;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	if (soc->htt_htc_pkt_freelist) {
		pkt = soc->htt_htc_pkt_freelist;
		soc->htt_htc_pkt_freelist = soc->htt_htc_pkt_freelist->u.next;
	}
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);

	if (!pkt)
		pkt = qdf_mem_malloc(sizeof(*pkt));

	if (!pkt)
		return NULL;

	htc_packet_set_magic_cookie(&(pkt->u.pkt.htc_pkt), 0);

	return &pkt->u.pkt; /* not actually a dereference */
}

qdf_export_symbol(htt_htc_pkt_alloc);

void
htt_htc_pkt_free(struct htt_soc *soc, struct dp_htt_htc_pkt *pkt)
{
	struct dp_htt_htc_pkt_union *u_pkt =
		(struct dp_htt_htc_pkt_union *)pkt;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	htc_packet_set_magic_cookie(&(u_pkt->u.pkt.htc_pkt), 0);
	u_pkt->u.next = soc->htt_htc_pkt_freelist;
	soc->htt_htc_pkt_freelist = u_pkt;
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);
}

qdf_export_symbol(htt_htc_pkt_free);

/*
 * htt_htc_pkt_pool_free() - Free HTC packet pool
 * @htt_soc:	HTT SOC handle
 */
void
htt_htc_pkt_pool_free(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt_union *pkt, *next;
	pkt = soc->htt_htc_pkt_freelist;
	while (pkt) {
		next = pkt->u.next;
		qdf_mem_free(pkt);
		pkt = next;
	}
	soc->htt_htc_pkt_freelist = NULL;
}


#ifndef ENABLE_CE4_COMP_DISABLE_HTT_HTC_MISC_LIST

/*
 * htt_htc_misc_pkt_list_trim() - trim misc list
 * @htt_soc: HTT SOC handle
 * @level: max no. of pkts in list
 */
static void
htt_htc_misc_pkt_list_trim(struct htt_soc *soc, int level)
{
	struct dp_htt_htc_pkt_union *pkt, *next, *prev = NULL;
	int i = 0;
	qdf_nbuf_t netbuf;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	pkt = soc->htt_htc_pkt_misclist;
	while (pkt) {
		next = pkt->u.next;
		/* trim the out grown list*/
		if (++i > level) {
			netbuf =
				(qdf_nbuf_t)(pkt->u.pkt.htc_pkt.pNetBufContext);
			qdf_nbuf_unmap(soc->osdev, netbuf, QDF_DMA_TO_DEVICE);
			qdf_nbuf_free(netbuf);
			qdf_mem_free(pkt);
			pkt = NULL;
			if (prev)
				prev->u.next = NULL;
		}
		prev = pkt;
		pkt = next;
	}
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);
}

/*
 * htt_htc_misc_pkt_list_add() - Add pkt to misc list
 * @htt_soc:	HTT SOC handle
 * @dp_htt_htc_pkt: pkt to be added to list
 */
void
htt_htc_misc_pkt_list_add(struct htt_soc *soc, struct dp_htt_htc_pkt *pkt)
{
	struct dp_htt_htc_pkt_union *u_pkt =
				(struct dp_htt_htc_pkt_union *)pkt;
	int misclist_trim_level = htc_get_tx_queue_depth(soc->htc_soc,
							pkt->htc_pkt.Endpoint)
				+ DP_HTT_HTC_PKT_MISCLIST_SIZE;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	if (soc->htt_htc_pkt_misclist) {
		u_pkt->u.next = soc->htt_htc_pkt_misclist;
		soc->htt_htc_pkt_misclist = u_pkt;
	} else {
		soc->htt_htc_pkt_misclist = u_pkt;
	}
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);

	/* only ce pipe size + tx_queue_depth could possibly be in use
	 * free older packets in the misclist
	 */
	htt_htc_misc_pkt_list_trim(soc, misclist_trim_level);
}

qdf_export_symbol(htt_htc_misc_pkt_list_add);
#endif  /* ENABLE_CE4_COMP_DISABLE_HTT_HTC_MISC_LIST */

/*
 * htt_htc_misc_pkt_pool_free() - free pkts in misc list
 * @htt_soc:	HTT SOC handle
 */
static void
htt_htc_misc_pkt_pool_free(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt_union *pkt, *next;
	qdf_nbuf_t netbuf;

	HTT_TX_MUTEX_ACQUIRE(&soc->htt_tx_mutex);
	pkt = soc->htt_htc_pkt_misclist;

	while (pkt) {
		next = pkt->u.next;
		if (htc_packet_get_magic_cookie(&(pkt->u.pkt.htc_pkt)) !=
		    HTC_PACKET_MAGIC_COOKIE) {
			pkt = next;
			soc->stats.skip_count++;
			continue;
		}
		netbuf = (qdf_nbuf_t) (pkt->u.pkt.htc_pkt.pNetBufContext);
		qdf_nbuf_unmap(soc->osdev, netbuf, QDF_DMA_TO_DEVICE);

		soc->stats.htc_pkt_free++;
		dp_htt_info("%pK: Pkt free count %d",
			    soc->dp_soc, soc->stats.htc_pkt_free);

		qdf_nbuf_free(netbuf);
		qdf_mem_free(pkt);
		pkt = next;
	}
	soc->htt_htc_pkt_misclist = NULL;
	HTT_TX_MUTEX_RELEASE(&soc->htt_tx_mutex);
	dp_info("HTC Packets, fail count = %d, skip count = %d",
		soc->stats.fail_count, soc->stats.skip_count);
}

/*
 * htt_t2h_mac_addr_deswizzle() - Swap MAC addr bytes if FW endianness differ
 * @tgt_mac_addr:	Target MAC
 * @buffer:		Output buffer
 */
static u_int8_t *
htt_t2h_mac_addr_deswizzle(u_int8_t *tgt_mac_addr, u_int8_t *buffer)
{
#ifdef BIG_ENDIAN_HOST
	/*
	 * The host endianness is opposite of the target endianness.
	 * To make u_int32_t elements come out correctly, the target->host
	 * upload has swizzled the bytes in each u_int32_t element of the
	 * message.
	 * For byte-array message fields like the MAC address, this
	 * upload swizzling puts the bytes in the wrong order, and needs
	 * to be undone.
	 */
	buffer[0] = tgt_mac_addr[3];
	buffer[1] = tgt_mac_addr[2];
	buffer[2] = tgt_mac_addr[1];
	buffer[3] = tgt_mac_addr[0];
	buffer[4] = tgt_mac_addr[7];
	buffer[5] = tgt_mac_addr[6];
	return buffer;
#else
	/*
	 * The host endianness matches the target endianness -
	 * we can use the mac addr directly from the message buffer.
	 */
	return tgt_mac_addr;
#endif
}

/*
 * dp_htt_h2t_send_complete_free_netbuf() - Free completed buffer
 * @soc:	SOC handle
 * @status:	Completion status
 * @netbuf:	HTT buffer
 */
static void
dp_htt_h2t_send_complete_free_netbuf(
	void *soc, A_STATUS status, qdf_nbuf_t netbuf)
{
	qdf_nbuf_free(netbuf);
}

#ifdef ENABLE_CE4_COMP_DISABLE_HTT_HTC_MISC_LIST
/*
 * dp_htt_h2t_send_complete() - H2T completion handler
 * @context:	Opaque context (HTT SOC handle)
 * @htc_pkt:	HTC packet
 */
static void
dp_htt_h2t_send_complete(void *context, HTC_PACKET *htc_pkt)
{
	struct htt_soc *soc =  (struct htt_soc *) context;
	struct dp_htt_htc_pkt *htt_pkt;
	qdf_nbuf_t netbuf;

	htt_pkt = container_of(htc_pkt, struct dp_htt_htc_pkt, htc_pkt);

	/* process (free or keep) the netbuf that held the message */
	netbuf = (qdf_nbuf_t) htc_pkt->pNetBufContext;
	/*
	 * adf sendcomplete is required for windows only
	 */
	/* qdf_nbuf_set_sendcompleteflag(netbuf, TRUE); */
	/* free the htt_htc_pkt / HTC_PACKET object */
	qdf_nbuf_free(netbuf);
	htt_htc_pkt_free(soc, htt_pkt);
}

#else /* ENABLE_CE4_COMP_DISABLE_HTT_HTC_MISC_LIST */

/*
 *  * dp_htt_h2t_send_complete() - H2T completion handler
 *   * @context:    Opaque context (HTT SOC handle)
 *    * @htc_pkt:    HTC packet
 *     */
static void
dp_htt_h2t_send_complete(void *context, HTC_PACKET *htc_pkt)
{
	void (*send_complete_part2)(
	     void *soc, QDF_STATUS status, qdf_nbuf_t msdu);
	struct htt_soc *soc =  (struct htt_soc *) context;
	struct dp_htt_htc_pkt *htt_pkt;
	qdf_nbuf_t netbuf;

	send_complete_part2 = htc_pkt->pPktContext;

	htt_pkt = container_of(htc_pkt, struct dp_htt_htc_pkt, htc_pkt);

	/* process (free or keep) the netbuf that held the message */
	netbuf = (qdf_nbuf_t) htc_pkt->pNetBufContext;
	/*
	 * adf sendcomplete is required for windows only
	*/
	/* qdf_nbuf_set_sendcompleteflag(netbuf, TRUE); */
	if (send_complete_part2){
		send_complete_part2(
		    htt_pkt->soc_ctxt, htc_pkt->Status, netbuf);
	}
	/* free the htt_htc_pkt / HTC_PACKET object */
	htt_htc_pkt_free(soc, htt_pkt);
}

#endif /* ENABLE_CE4_COMP_DISABLE_HTT_HTC_MISC_LIST */

/*
 * htt_h2t_ver_req_msg() - Send HTT version request message to target
 * @htt_soc:	HTT SOC handle
 *
 * Return: 0 on success; error code on failure
 */
static int htt_h2t_ver_req_msg(struct htt_soc *soc)
{
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	uint32_t *msg_word;
	QDF_STATUS status;

	msg = qdf_nbuf_alloc(
		soc->osdev,
		HTT_MSG_BUF_SIZE(HTT_VER_REQ_BYTES),
		/* reserve room for the HTC header */
		HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);
	if (!msg)
		return QDF_STATUS_E_NOMEM;

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(msg, HTT_VER_REQ_BYTES) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to expand head for HTT_H2T_MSG_TYPE_VERSION_REQ msg",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}

	/* fill in the message contents */
	msg_word = (u_int32_t *) qdf_nbuf_data(msg);

	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);

	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_VERSION_REQ);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}
	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf, qdf_nbuf_data(msg),
		qdf_nbuf_len(msg), soc->htc_endpoint,
		HTC_TX_PACKET_TAG_RTPM_PUT_RC);

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);
	status = DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_VERSION_REQ,
				     NULL);

	if (status != QDF_STATUS_SUCCESS) {
		qdf_nbuf_free(msg);
		htt_htc_pkt_free(soc, pkt);
	}

	return status;
}

/*
 * htt_srng_setup() - Send SRNG setup message to target
 * @htt_soc:	HTT SOC handle
 * @mac_id:	MAC Id
 * @hal_srng:	Opaque HAL SRNG pointer
 * @hal_ring_type:	SRNG ring type
 *
 * Return: 0 on success; error code on failure
 */
int htt_srng_setup(struct htt_soc *soc, int mac_id,
		   hal_ring_handle_t hal_ring_hdl,
		   int hal_ring_type)
{
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t htt_msg;
	uint32_t *msg_word;
	struct hal_srng_params srng_params;
	qdf_dma_addr_t hp_addr, tp_addr;
	uint32_t ring_entry_size =
		hal_srng_get_entrysize(soc->hal_soc, hal_ring_type);
	int htt_ring_type, htt_ring_id;
	uint8_t *htt_logger_bufp;
	int target_pdev_id;
	int lmac_id = dp_get_lmac_id_for_pdev_id(soc->dp_soc, 0, mac_id);
	QDF_STATUS status;

	/* Sizes should be set in 4-byte words */
	ring_entry_size = ring_entry_size >> 2;

	htt_msg = qdf_nbuf_alloc(soc->osdev,
		HTT_MSG_BUF_SIZE(HTT_SRING_SETUP_SZ),
		/* reserve room for the HTC header */
		HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);
	if (!htt_msg) {
		dp_err("htt_msg alloc failed ring type %d", hal_ring_type);
		goto fail0;
	}

	hal_get_srng_params(soc->hal_soc, hal_ring_hdl, &srng_params);
	hp_addr = hal_srng_get_hp_addr(soc->hal_soc, hal_ring_hdl);
	tp_addr = hal_srng_get_tp_addr(soc->hal_soc, hal_ring_hdl);

	switch (hal_ring_type) {
	case RXDMA_BUF:
#ifdef QCA_HOST2FW_RXBUF_RING
		if (srng_params.ring_id ==
		    (HAL_SRNG_WMAC1_SW2RXDMA0_BUF0 +
		    (lmac_id * HAL_MAX_RINGS_PER_LMAC))) {
			htt_ring_id = HTT_HOST1_TO_FW_RXBUF_RING;
			htt_ring_type = HTT_SW_TO_SW_RING;
#ifdef IPA_OFFLOAD
		} else if (srng_params.ring_id ==
		    (HAL_SRNG_WMAC1_SW2RXDMA0_BUF2 +
		    (lmac_id * HAL_MAX_RINGS_PER_LMAC))) {
			htt_ring_id = HTT_HOST2_TO_FW_RXBUF_RING;
			htt_ring_type = HTT_SW_TO_SW_RING;
#endif
#else
		if (srng_params.ring_id ==
			(HAL_SRNG_WMAC1_SW2RXDMA0_BUF0 +
			(lmac_id * HAL_MAX_RINGS_PER_LMAC))) {
			htt_ring_id = HTT_RXDMA_HOST_BUF_RING;
			htt_ring_type = HTT_SW_TO_HW_RING;
#endif
		} else if (srng_params.ring_id ==
#ifdef IPA_OFFLOAD
			 (HAL_SRNG_WMAC1_SW2RXDMA0_BUF1 +
#else
			 (HAL_SRNG_WMAC1_SW2RXDMA1_BUF +
#endif
			(lmac_id * HAL_MAX_RINGS_PER_LMAC))) {
			htt_ring_id = HTT_RXDMA_HOST_BUF_RING;
			htt_ring_type = HTT_SW_TO_HW_RING;
		} else {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				   "%s: Ring %d currently not supported",
				   __func__, srng_params.ring_id);
			goto fail1;
		}

		break;
	case RXDMA_MONITOR_BUF:
		htt_ring_id = HTT_RXDMA_MONITOR_BUF_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_STATUS:
		htt_ring_id = HTT_RXDMA_MONITOR_STATUS_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_DST:
		htt_ring_id = HTT_RXDMA_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
		break;
	case RXDMA_MONITOR_DESC:
		htt_ring_id = HTT_RXDMA_MONITOR_DESC_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_DST:
		htt_ring_id = HTT_RXDMA_NON_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
		break;
#ifdef QCA_MONITOR_2_0_SUPPORT_WAR
	// WAR till fw htt.h changes are merged
	case TX_MONITOR_BUF:
		htt_ring_id = HTT_TX_MON_HOST2MON_BUF_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case TX_MONITOR_DST:
		htt_ring_id = HTT_TX_MON_MON2HOST_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
		break;
#endif

	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Ring currently not supported", __func__);
			goto fail1;
	}

	dp_info("ring_type %d ring_id %d htt_ring_id %d hp_addr 0x%llx tp_addr 0x%llx",
		hal_ring_type, srng_params.ring_id, htt_ring_id,
		(uint64_t)hp_addr,
		(uint64_t)tp_addr);
	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(htt_msg, HTT_SRING_SETUP_SZ) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to expand head for SRING_SETUP msg",
			__func__);
		return QDF_STATUS_E_FAILURE;
	}

	msg_word = (uint32_t *)qdf_nbuf_data(htt_msg);

	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(htt_msg, HTC_HDR_ALIGNMENT_PADDING);

	/* word 0 */
	*msg_word = 0;
	htt_logger_bufp = (uint8_t *)msg_word;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_SRING_SETUP);
	target_pdev_id =
	dp_get_target_pdev_id_for_host_pdev_id(soc->dp_soc, mac_id);

	if ((htt_ring_type == HTT_SW_TO_HW_RING) ||
			(htt_ring_type == HTT_HW_TO_SW_RING))
		HTT_SRING_SETUP_PDEV_ID_SET(*msg_word, target_pdev_id);
	else
		HTT_SRING_SETUP_PDEV_ID_SET(*msg_word, mac_id);

	dp_info("mac_id %d", mac_id);
	HTT_SRING_SETUP_RING_TYPE_SET(*msg_word, htt_ring_type);
	/* TODO: Discuss with FW on changing this to unique ID and using
	 * htt_ring_type to send the type of ring
	 */
	HTT_SRING_SETUP_RING_ID_SET(*msg_word, htt_ring_id);

	/* word 1 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_BASE_ADDR_LO_SET(*msg_word,
		srng_params.ring_base_paddr & 0xffffffff);

	/* word 2 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_BASE_ADDR_HI_SET(*msg_word,
		(uint64_t)srng_params.ring_base_paddr >> 32);

	/* word 3 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_ENTRY_SIZE_SET(*msg_word, ring_entry_size);
	HTT_SRING_SETUP_RING_SIZE_SET(*msg_word,
		(ring_entry_size * srng_params.num_entries));
	dp_info("entry_size %d", ring_entry_size);
	dp_info("num_entries %d", srng_params.num_entries);
	dp_info("ring_size %d", (ring_entry_size * srng_params.num_entries));
	if (htt_ring_type == HTT_SW_TO_HW_RING)
		HTT_SRING_SETUP_RING_MISC_CFG_FLAG_LOOPCOUNT_DISABLE_SET(
						*msg_word, 1);
	HTT_SRING_SETUP_RING_MISC_CFG_FLAG_MSI_SWAP_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_MSI_SWAP));
	HTT_SRING_SETUP_RING_MISC_CFG_FLAG_TLV_SWAP_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_DATA_TLV_SWAP));
	HTT_SRING_SETUP_RING_MISC_CFG_FLAG_HOST_FW_SWAP_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_RING_PTR_SWAP));

	/* word 4 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_HEAD_OFFSET32_REMOTE_BASE_ADDR_LO_SET(*msg_word,
		hp_addr & 0xffffffff);

	/* word 5 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_HEAD_OFFSET32_REMOTE_BASE_ADDR_HI_SET(*msg_word,
		(uint64_t)hp_addr >> 32);

	/* word 6 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_TAIL_OFFSET32_REMOTE_BASE_ADDR_LO_SET(*msg_word,
		tp_addr & 0xffffffff);

	/* word 7 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_TAIL_OFFSET32_REMOTE_BASE_ADDR_HI_SET(*msg_word,
		(uint64_t)tp_addr >> 32);

	/* word 8 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_MSI_ADDR_LO_SET(*msg_word,
		srng_params.msi_addr & 0xffffffff);

	/* word 9 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_MSI_ADDR_HI_SET(*msg_word,
		(uint64_t)(srng_params.msi_addr) >> 32);

	/* word 10 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_RING_MSI_DATA_SET(*msg_word,
		qdf_cpu_to_le32(srng_params.msi_data));

	/* word 11 */
	msg_word++;
	*msg_word = 0;
	HTT_SRING_SETUP_INTR_BATCH_COUNTER_TH_SET(*msg_word,
		srng_params.intr_batch_cntr_thres_entries *
		ring_entry_size);
	HTT_SRING_SETUP_INTR_TIMER_TH_SET(*msg_word,
		srng_params.intr_timer_thres_us >> 3);

	/* word 12 */
	msg_word++;
	*msg_word = 0;
	if (srng_params.flags & HAL_SRNG_LOW_THRES_INTR_ENABLE) {
		/* TODO: Setting low threshold to 1/8th of ring size - see
		 * if this needs to be configurable
		 */
		HTT_SRING_SETUP_INTR_LOW_TH_SET(*msg_word,
			srng_params.low_threshold);
	}
	/* "response_required" field should be set if a HTT response message is
	 * required after setting up the ring.
	 */
	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		dp_err("pkt alloc failed, ring_type %d ring_id %d htt_ring_id %d",
		       hal_ring_type, srng_params.ring_id, htt_ring_id);
		goto fail1;
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
		&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf,
		qdf_nbuf_data(htt_msg),
		qdf_nbuf_len(htt_msg),
		soc->htc_endpoint,
		HTC_TX_PACKET_TAG_RUNTIME_PUT); /* tag for no FW response msg */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, htt_msg);
	status = DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_SRING_SETUP,
				     htt_logger_bufp);

	if (status != QDF_STATUS_SUCCESS) {
		qdf_nbuf_free(htt_msg);
		htt_htc_pkt_free(soc, pkt);
	}

	return status;

fail1:
	qdf_nbuf_free(htt_msg);
fail0:
	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(htt_srng_setup);

#ifdef QCA_SUPPORT_FULL_MON
/**
 * htt_h2t_full_mon_cfg() - Send full monitor configuarion msg to FW
 *
 * @htt_soc: HTT Soc handle
 * @pdev_id: Radio id
 * @dp_full_mon_config: enabled/disable configuration
 *
 * Return: Success when HTT message is sent, error on failure
 */
int htt_h2t_full_mon_cfg(struct htt_soc *htt_soc,
			 uint8_t pdev_id,
			 enum dp_full_mon_config config)
{
	struct htt_soc *soc = (struct htt_soc *)htt_soc;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t htt_msg;
	uint32_t *msg_word;
	uint8_t *htt_logger_bufp;

	htt_msg = qdf_nbuf_alloc(soc->osdev,
				 HTT_MSG_BUF_SIZE(
				 HTT_RX_FULL_MONITOR_MODE_SETUP_SZ),
				 /* reserve room for the HTC header */
				 HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING,
				 4,
				 TRUE);
	if (!htt_msg)
		return QDF_STATUS_E_FAILURE;

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (!qdf_nbuf_put_tail(htt_msg, HTT_RX_FULL_MONITOR_MODE_SETUP_SZ)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to expand head for RX Ring Cfg msg",
			  __func__);
		goto fail1;
	}

	msg_word = (uint32_t *)qdf_nbuf_data(htt_msg);

	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(htt_msg, HTC_HDR_ALIGNMENT_PADDING);

	/* word 0 */
	*msg_word = 0;
	htt_logger_bufp = (uint8_t *)msg_word;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_RX_FULL_MONITOR_MODE);
	HTT_RX_FULL_MONITOR_MODE_OPERATION_PDEV_ID_SET(
			*msg_word, DP_SW2HW_MACID(pdev_id));

	msg_word++;
	*msg_word = 0;
	/* word 1 */
	if (config == DP_FULL_MON_ENABLE) {
		HTT_RX_FULL_MONITOR_MODE_ENABLE_SET(*msg_word, true);
		HTT_RX_FULL_MONITOR_MODE_ZERO_MPDU_SET(*msg_word, true);
		HTT_RX_FULL_MONITOR_MODE_NON_ZERO_MPDU_SET(*msg_word, true);
		HTT_RX_FULL_MONITOR_MODE_RELEASE_RINGS_SET(*msg_word, 0x2);
	} else if (config == DP_FULL_MON_DISABLE) {
		/* As per MAC team's suggestion, While disbaling full monitor
		 * mode, Set 'en' bit to true in full monitor mode register.
		 */
		HTT_RX_FULL_MONITOR_MODE_ENABLE_SET(*msg_word, true);
		HTT_RX_FULL_MONITOR_MODE_ZERO_MPDU_SET(*msg_word, false);
		HTT_RX_FULL_MONITOR_MODE_NON_ZERO_MPDU_SET(*msg_word, false);
		HTT_RX_FULL_MONITOR_MODE_RELEASE_RINGS_SET(*msg_word, 0x2);
	}

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_err("HTC packet allocation failed");
		goto fail1;
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
		&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf,
		qdf_nbuf_data(htt_msg),
		qdf_nbuf_len(htt_msg),
		soc->htc_endpoint,
		HTC_TX_PACKET_TAG_RUNTIME_PUT); /* tag for no FW response msg */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, htt_msg);
	qdf_debug("config: %d", config);
	DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_RX_FULL_MONITOR_MODE,
			    htt_logger_bufp);
	return QDF_STATUS_SUCCESS;
fail1:
	qdf_nbuf_free(htt_msg);
	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(htt_h2t_full_mon_cfg);
#else
int htt_h2t_full_mon_cfg(struct htt_soc *htt_soc,
			 uint8_t pdev_id,
			 enum dp_full_mon_config config)
{
	return 0;
}

qdf_export_symbol(htt_h2t_full_mon_cfg);
#endif

/*
 * htt_h2t_rx_ring_cfg() - Send SRNG packet and TLV filter
 * config message to target
 * @htt_soc:	HTT SOC handle
 * @pdev_id:	WIN- PDEV Id, MCL- mac id
 * @hal_srng:	Opaque HAL SRNG pointer
 * @hal_ring_type:	SRNG ring type
 * @ring_buf_size:	SRNG buffer size
 * @htt_tlv_filter:	Rx SRNG TLV and filter setting
 * Return: 0 on success; error code on failure
 */
int htt_h2t_rx_ring_cfg(struct htt_soc *htt_soc, int pdev_id,
			hal_ring_handle_t hal_ring_hdl,
			int hal_ring_type, int ring_buf_size,
			struct htt_rx_ring_tlv_filter *htt_tlv_filter)
{
	struct htt_soc *soc = (struct htt_soc *)htt_soc;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t htt_msg;
	uint32_t *msg_word;
	uint32_t *msg_word_data;
	struct hal_srng_params srng_params;
	uint32_t htt_ring_type, htt_ring_id;
	uint32_t tlv_filter;
	uint8_t *htt_logger_bufp;
	struct wlan_cfg_dp_soc_ctxt *wlan_cfg_ctx = soc->dp_soc->wlan_cfg_ctx;
	uint32_t mon_drop_th = wlan_cfg_get_mon_drop_thresh(wlan_cfg_ctx);
	int target_pdev_id;
	QDF_STATUS status;

	htt_msg = qdf_nbuf_alloc(soc->osdev,
		HTT_MSG_BUF_SIZE(HTT_RX_RING_SELECTION_CFG_SZ),
	/* reserve room for the HTC header */
	HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);
	if (!htt_msg) {
		dp_err("htt_msg alloc failed ring type %d", hal_ring_type);
		goto fail0;
	}

	hal_get_srng_params(soc->hal_soc, hal_ring_hdl, &srng_params);

	switch (hal_ring_type) {
	case RXDMA_BUF:
		htt_ring_id = HTT_RXDMA_HOST_BUF_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_BUF:
		htt_ring_id = HTT_RXDMA_MONITOR_BUF_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_STATUS:
		htt_ring_id = HTT_RXDMA_MONITOR_STATUS_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_MONITOR_DST:
		htt_ring_id = HTT_RXDMA_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
		break;
	case RXDMA_MONITOR_DESC:
		htt_ring_id = HTT_RXDMA_MONITOR_DESC_RING;
		htt_ring_type = HTT_SW_TO_HW_RING;
		break;
	case RXDMA_DST:
		htt_ring_id = HTT_RXDMA_NON_MONITOR_DEST_RING;
		htt_ring_type = HTT_HW_TO_SW_RING;
		break;

	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Ring currently not supported", __func__);
		goto fail1;
	}

	dp_info("ring_type %d ring_id %d htt_ring_id %d",
		hal_ring_type, srng_params.ring_id, htt_ring_id);

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(htt_msg, HTT_RX_RING_SELECTION_CFG_SZ) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Failed to expand head for RX Ring Cfg msg",
			__func__);
		goto fail1; /* failure */
	}

	msg_word = (uint32_t *)qdf_nbuf_data(htt_msg);

	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(htt_msg, HTC_HDR_ALIGNMENT_PADDING);

	/* word 0 */
	htt_logger_bufp = (uint8_t *)msg_word;
	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_RX_RING_SELECTION_CFG);

	/*
	 * pdev_id is indexed from 0 whereas mac_id is indexed from 1
	 * SW_TO_SW and SW_TO_HW rings are unaffected by this
	 */
	target_pdev_id =
	dp_get_target_pdev_id_for_host_pdev_id(soc->dp_soc, pdev_id);

	if (htt_ring_type == HTT_SW_TO_SW_RING ||
			htt_ring_type == HTT_SW_TO_HW_RING)
		HTT_RX_RING_SELECTION_CFG_PDEV_ID_SET(*msg_word,
						      target_pdev_id);

	/* TODO: Discuss with FW on changing this to unique ID and using
	 * htt_ring_type to send the type of ring
	 */
	HTT_RX_RING_SELECTION_CFG_RING_ID_SET(*msg_word, htt_ring_id);

	HTT_RX_RING_SELECTION_CFG_STATUS_TLV_SET(*msg_word,
		!!(srng_params.flags & HAL_SRNG_MSI_SWAP));

	HTT_RX_RING_SELECTION_CFG_RX_OFFSETS_VALID_SET(*msg_word,
						htt_tlv_filter->offset_valid);

	if (mon_drop_th > 0)
		HTT_RX_RING_SELECTION_CFG_DROP_THRESHOLD_VALID_SET(*msg_word,
								   1);
	else
		HTT_RX_RING_SELECTION_CFG_DROP_THRESHOLD_VALID_SET(*msg_word,
								   0);

	/* word 1 */
	msg_word++;
	*msg_word = 0;
	HTT_RX_RING_SELECTION_CFG_RING_BUFFER_SIZE_SET(*msg_word,
		ring_buf_size);

	dp_mon_rx_packet_length_set(soc->dp_soc, msg_word, htt_tlv_filter);

	/* word 2 */
	msg_word++;
	*msg_word = 0;

	if (htt_tlv_filter->enable_fp) {
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0000,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0001,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0010,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_REASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0011,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_REASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0100,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_PROBE_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0101,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_PROBE_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 0110,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_TIM_ADVT) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, FP,
			MGMT, 0111,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_RESERVED_7) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 1000,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_BEACON) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			FP, MGMT, 1001,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ATIM) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_md) {
			/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0000,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_ASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0001,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_ASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0010,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_REASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0011,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_REASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0100,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_PROBE_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0101,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_PROBE_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 0110,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_TIM_ADVT) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MD,
			MGMT, 0111,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_RESERVED_7) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 1000,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_BEACON) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MD, MGMT, 1001,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_ATIM) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_mo) {
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0000,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0001,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0010,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_REASSOC_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0011,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_REASSOC_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0100,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_PROBE_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0101,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_PROBE_RES) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 0110,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_TIM_ADVT) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0, MO,
			MGMT, 0111,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_RESERVED_7) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 1000,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_BEACON) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG0,
			MO, MGMT, 1001,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ATIM) ? 1 : 0);
	}

	/* word 3 */
	msg_word++;
	*msg_word = 0;

	if (htt_tlv_filter->enable_fp) {
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1010,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_DISASSOC) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1011,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_AUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1100,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_DEAUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1101,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ACTION) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			FP, MGMT, 1110,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_ACT_NO_ACK) ? 1 : 0);
		/* reserved*/
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, FP,
			MGMT, 1111,
			(htt_tlv_filter->fp_mgmt_filter &
			FILTER_MGMT_RESERVED_15) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_md) {
			/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MD, MGMT, 1010,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_DISASSOC) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MD, MGMT, 1011,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_AUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MD, MGMT, 1100,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_DEAUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MD, MGMT, 1101,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_ACTION) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MD, MGMT, 1110,
			(htt_tlv_filter->md_mgmt_filter &
			FILTER_MGMT_ACT_NO_ACK) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_mo) {
		/* TYPE: MGMT */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1010,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_DISASSOC) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1011,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_AUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1100,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_DEAUTH) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1101,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ACTION) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1,
			MO, MGMT, 1110,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_ACT_NO_ACK) ? 1 : 0);
		/* reserved*/
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG1, MO,
			MGMT, 1111,
			(htt_tlv_filter->mo_mgmt_filter &
			FILTER_MGMT_RESERVED_15) ? 1 : 0);
	}

	/* word 4 */
	msg_word++;
	*msg_word = 0;

	if (htt_tlv_filter->enable_fp) {
		/* TYPE: CTRL */
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0000,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RESERVED_1) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0001,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RESERVED_2) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0010,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_TRIGGER) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0011,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RESERVED_4) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0100,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_BF_REP_POLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0101,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_VHT_NDP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0110,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_FRAME_EXT) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 0111,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CTRLWRAP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 1000,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_BA_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, FP,
			CTRL, 1001,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_BA) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_md) {
		/* TYPE: CTRL */
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0000,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_RESERVED_1) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0001,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_RESERVED_2) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0010,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_TRIGGER) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0011,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_RESERVED_4) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0100,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_BF_REP_POLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0101,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_VHT_NDP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0110,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_FRAME_EXT) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 0111,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_CTRLWRAP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 1000,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_BA_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MD,
			CTRL, 1001,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_BA) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_mo) {
		/* TYPE: CTRL */
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0000,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RESERVED_1) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0001,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RESERVED_2) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0010,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_TRIGGER) ? 1 : 0);
		/* reserved */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0011,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RESERVED_4) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0100,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_BF_REP_POLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0101,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_VHT_NDP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0110,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_FRAME_EXT) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 0111,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CTRLWRAP) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 1000,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_BA_REQ) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG2, MO,
			CTRL, 1001,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_BA) ? 1 : 0);
	}

	/* word 5 */
	msg_word++;
	*msg_word = 0;
	if (htt_tlv_filter->enable_fp) {
		/* TYPE: CTRL */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1010,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_PSPOLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1011,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_RTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1100,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1101,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_ACK) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1110,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CFEND) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			CTRL, 1111,
			(htt_tlv_filter->fp_ctrl_filter &
			FILTER_CTRL_CFEND_CFACK) ? 1 : 0);
		/* TYPE: DATA */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, MCAST,
			(htt_tlv_filter->fp_data_filter &
			FILTER_DATA_MCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, UCAST,
			(htt_tlv_filter->fp_data_filter &
			FILTER_DATA_UCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, FP,
			DATA, NULL,
			(htt_tlv_filter->fp_data_filter &
			FILTER_DATA_NULL) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_md) {
		/* TYPE: CTRL */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1010,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_PSPOLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1011,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_RTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1100,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_CTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1101,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_ACK) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1110,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_CFEND) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			CTRL, 1111,
			(htt_tlv_filter->md_ctrl_filter &
			FILTER_CTRL_CFEND_CFACK) ? 1 : 0);
		/* TYPE: DATA */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			DATA, MCAST,
			(htt_tlv_filter->md_data_filter &
			FILTER_DATA_MCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			DATA, UCAST,
			(htt_tlv_filter->md_data_filter &
			FILTER_DATA_UCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MD,
			DATA, NULL,
			(htt_tlv_filter->md_data_filter &
			FILTER_DATA_NULL) ? 1 : 0);
	}

	if (htt_tlv_filter->enable_mo) {
		/* TYPE: CTRL */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1010,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_PSPOLL) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1011,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_RTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1100,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CTS) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1101,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_ACK) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1110,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CFEND) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			CTRL, 1111,
			(htt_tlv_filter->mo_ctrl_filter &
			FILTER_CTRL_CFEND_CFACK) ? 1 : 0);
		/* TYPE: DATA */
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, MCAST,
			(htt_tlv_filter->mo_data_filter &
			FILTER_DATA_MCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, UCAST,
			(htt_tlv_filter->mo_data_filter &
			FILTER_DATA_UCAST) ? 1 : 0);
		htt_rx_ring_pkt_enable_subtype_set(*msg_word, FLAG3, MO,
			DATA, NULL,
			(htt_tlv_filter->mo_data_filter &
			FILTER_DATA_NULL) ? 1 : 0);
	}

	/* word 6 */
	msg_word++;
	*msg_word = 0;
	tlv_filter = 0;
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MPDU_START,
		htt_tlv_filter->mpdu_start);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MSDU_START,
		htt_tlv_filter->msdu_start);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PACKET,
		htt_tlv_filter->packet);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MSDU_END,
		htt_tlv_filter->msdu_end);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, MPDU_END,
		htt_tlv_filter->mpdu_end);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PACKET_HEADER,
		htt_tlv_filter->packet_header);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, ATTENTION,
		htt_tlv_filter->attention);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_START,
		htt_tlv_filter->ppdu_start);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_END,
		htt_tlv_filter->ppdu_end);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_END_USER_STATS,
		htt_tlv_filter->ppdu_end_user_stats);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter,
		PPDU_END_USER_STATS_EXT,
		htt_tlv_filter->ppdu_end_user_stats_ext);
	htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, PPDU_END_STATUS_DONE,
		htt_tlv_filter->ppdu_end_status_done);
	/* RESERVED bit maps to header_per_msdu in htt_tlv_filter*/
	 htt_rx_ring_tlv_filter_in_enable_set(tlv_filter, RESERVED,
		 htt_tlv_filter->header_per_msdu);

	HTT_RX_RING_SELECTION_CFG_TLV_FILTER_IN_FLAG_SET(*msg_word, tlv_filter);

	msg_word_data = (uint32_t *)qdf_nbuf_data(htt_msg);
	dp_info("config_data: [0x%x][0x%x][0x%x][0x%x][0x%x][0x%x][0x%x]",
		msg_word_data[0], msg_word_data[1], msg_word_data[2],
		msg_word_data[3], msg_word_data[4], msg_word_data[5],
		msg_word_data[6]);

	msg_word++;
	*msg_word = 0;
	if (htt_tlv_filter->offset_valid) {
		HTT_RX_RING_SELECTION_CFG_RX_PACKET_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_packet_offset);
		HTT_RX_RING_SELECTION_CFG_RX_HEADER_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_header_offset);

		msg_word++;
		*msg_word = 0;
		HTT_RX_RING_SELECTION_CFG_RX_MPDU_END_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_mpdu_end_offset);
		HTT_RX_RING_SELECTION_CFG_RX_MPDU_START_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_mpdu_start_offset);

		msg_word++;
		*msg_word = 0;
		HTT_RX_RING_SELECTION_CFG_RX_MSDU_END_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_msdu_end_offset);
		HTT_RX_RING_SELECTION_CFG_RX_MSDU_START_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_msdu_start_offset);

		msg_word++;
		*msg_word = 0;
		HTT_RX_RING_SELECTION_CFG_RX_ATTENTION_OFFSET_SET(*msg_word,
					htt_tlv_filter->rx_attn_offset);
		msg_word++;
		*msg_word = 0;
	} else {
		msg_word += 4;
		*msg_word = 0;
	}

	if (mon_drop_th > 0)
		HTT_RX_RING_SELECTION_CFG_RX_DROP_THRESHOLD_SET(*msg_word,
								mon_drop_th);
	dp_mon_rx_enable_mpdu_logging(soc->dp_soc, msg_word, htt_tlv_filter);

	msg_word++;
	*msg_word = 0;
	dp_mon_rx_wmask_subscribe(soc->dp_soc, msg_word, htt_tlv_filter);

	/* "response_required" field should be set if a HTT response message is
	 * required after setting up the ring.
	 */
	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		dp_err("pkt alloc failed, ring_type %d ring_id %d htt_ring_id %d",
		       hal_ring_type, srng_params.ring_id, htt_ring_id);
		goto fail1;
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
		&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf,
		qdf_nbuf_data(htt_msg),
		qdf_nbuf_len(htt_msg),
		soc->htc_endpoint,
		HTC_TX_PACKET_TAG_RUNTIME_PUT); /* tag for no FW response msg */

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, htt_msg);
	status = DP_HTT_SEND_HTC_PKT(soc, pkt,
				     HTT_H2T_MSG_TYPE_RX_RING_SELECTION_CFG,
				     htt_logger_bufp);

	if (status != QDF_STATUS_SUCCESS) {
		qdf_nbuf_free(htt_msg);
		htt_htc_pkt_free(soc, pkt);
	}

	return status;

fail1:
	qdf_nbuf_free(htt_msg);
fail0:
	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(htt_h2t_rx_ring_cfg);

#if defined(HTT_STATS_ENABLE)
static inline QDF_STATUS dp_send_htt_stat_resp(struct htt_stats_context *htt_stats,
					struct dp_soc *soc, qdf_nbuf_t htt_msg)

{
	uint32_t pdev_id;
	uint32_t *msg_word = NULL;
	uint32_t msg_remain_len = 0;

	msg_word = (uint32_t *) qdf_nbuf_data(htt_msg);

	/*COOKIE MSB*/
	pdev_id = *(msg_word + 2) & HTT_PID_BIT_MASK;

	/* stats message length + 16 size of HTT header*/
	msg_remain_len = qdf_min(htt_stats->msg_len + 16,
				(uint32_t)DP_EXT_MSG_LENGTH);

	dp_wdi_event_handler(WDI_EVENT_HTT_STATS, soc,
			msg_word,  msg_remain_len,
			WDI_NO_VAL, pdev_id);

	if (htt_stats->msg_len >= DP_EXT_MSG_LENGTH) {
		htt_stats->msg_len -= DP_EXT_MSG_LENGTH;
	}
	/* Need to be freed here as WDI handler will
	 * make a copy of pkt to send data to application
	 */
	qdf_nbuf_free(htt_msg);
	return QDF_STATUS_SUCCESS;
}
#else
static inline QDF_STATUS
dp_send_htt_stat_resp(struct htt_stats_context *htt_stats,
		      struct dp_soc *soc, qdf_nbuf_t htt_msg)
{
	return QDF_STATUS_E_NOSUPPORT;
}
#endif

#ifdef HTT_STATS_DEBUGFS_SUPPORT
/* dp_send_htt_stats_dbgfs_msg() - Function to send htt data to upper layer.
 * @pdev: dp pdev handle
 * @msg_word: HTT msg
 * @msg_len: Length of HTT msg sent
 *
 * Return: none
 */
static inline void
dp_htt_stats_dbgfs_send_msg(struct dp_pdev *pdev, uint32_t *msg_word,
			    uint32_t msg_len)
{
	struct htt_dbgfs_cfg dbgfs_cfg;
	int done = 0;

	/* send 5th word of HTT msg to upper layer */
	dbgfs_cfg.msg_word = (msg_word + 4);
	dbgfs_cfg.m = pdev->dbgfs_cfg->m;

	/* stats message length + 16 size of HTT header*/
	msg_len = qdf_min(msg_len + HTT_HEADER_LEN, (uint32_t)DP_EXT_MSG_LENGTH);

	if (pdev->dbgfs_cfg->htt_stats_dbgfs_msg_process)
		pdev->dbgfs_cfg->htt_stats_dbgfs_msg_process(&dbgfs_cfg,
							     (msg_len - HTT_HEADER_LEN));

	/* Get TLV Done bit from 4th msg word */
	done = HTT_T2H_EXT_STATS_CONF_TLV_DONE_GET(*(msg_word + 3));
	if (done) {
		if (qdf_event_set(&pdev->dbgfs_cfg->htt_stats_dbgfs_event))
			dp_htt_err("%pK: Failed to set event for debugfs htt stats"
				   , pdev->soc);
	}
}
#else
static inline void
dp_htt_stats_dbgfs_send_msg(struct dp_pdev *pdev, uint32_t *msg_word,
			    uint32_t msg_len)
{
}
#endif /* HTT_STATS_DEBUGFS_SUPPORT */

#ifdef WLAN_SYSFS_DP_STATS
/* dp_htt_stats_sysfs_update_config() - Function to send htt data to upper layer.
 * @pdev: dp pdev handle
 *
 * This function sets the process id and printing mode within the sysfs config
 * struct. which enables DP_PRINT statements within this process to write to the
 * console buffer provided by the user space.
 *
 * Return: None
 */
static inline void
dp_htt_stats_sysfs_update_config(struct dp_pdev *pdev)
{
	struct dp_soc *soc = pdev->soc;

	if (!soc) {
		dp_htt_err("soc is null");
		return;
	}

	if (!soc->sysfs_config) {
		dp_htt_err("soc->sysfs_config is NULL");
		return;
	}

	/* set sysfs config parameters */
	soc->sysfs_config->process_id = qdf_get_current_pid();
	soc->sysfs_config->printing_mode = PRINTING_MODE_ENABLED;
}

/*
 * dp_htt_stats_sysfs_set_event() - Set sysfs stats event.
 * @soc: soc handle.
 * @msg_word: Pointer to htt msg word.
 *
 * @return: void
 */
static inline void
dp_htt_stats_sysfs_set_event(struct dp_soc *soc, uint32_t *msg_word)
{
	int done = 0;

	done = HTT_T2H_EXT_STATS_CONF_TLV_DONE_GET(*(msg_word + 3));
	if (done) {
		if (qdf_event_set(&soc->sysfs_config->sysfs_txrx_fw_request_done))
			dp_htt_err("%pK:event compl Fail to set event ",
				   soc);
	}
}
#else /* WLAN_SYSFS_DP_STATS */
static inline void
dp_htt_stats_sysfs_update_config(struct dp_pdev *pdev)
{
}

static inline void
dp_htt_stats_sysfs_set_event(struct dp_soc *dp_soc, uint32_t *msg_word)
{
}
#endif /* WLAN_SYSFS_DP_STATS */

/**
 * dp_process_htt_stat_msg(): Process the list of buffers of HTT EXT stats
 * @htt_stats: htt stats info
 *
 * The FW sends the HTT EXT STATS as a stream of T2H messages. Each T2H message
 * contains sub messages which are identified by a TLV header.
 * In this function we will process the stream of T2H messages and read all the
 * TLV contained in the message.
 *
 * THe following cases have been taken care of
 * Case 1: When the tlv_remain_length <= msg_remain_length of HTT MSG buffer
 *		In this case the buffer will contain multiple tlvs.
 * Case 2: When the tlv_remain_length > msg_remain_length of HTT MSG buffer.
 *		Only one tlv will be contained in the HTT message and this tag
 *		will extend onto the next buffer.
 * Case 3: When the buffer is the continuation of the previous message
 * Case 4: tlv length is 0. which will indicate the end of message
 *
 * return: void
 */
static inline void dp_process_htt_stat_msg(struct htt_stats_context *htt_stats,
					struct dp_soc *soc)
{
	htt_tlv_tag_t tlv_type = 0xff;
	qdf_nbuf_t htt_msg = NULL;
	uint32_t *msg_word;
	uint8_t *tlv_buf_head = NULL;
	uint8_t *tlv_buf_tail = NULL;
	uint32_t msg_remain_len = 0;
	uint32_t tlv_remain_len = 0;
	uint32_t *tlv_start;
	int cookie_val = 0;
	int cookie_msb = 0;
	int pdev_id;
	bool copy_stats = false;
	struct dp_pdev *pdev;

	/* Process node in the HTT message queue */
	while ((htt_msg = qdf_nbuf_queue_remove(&htt_stats->msg))
		!= NULL) {
		msg_word = (uint32_t *) qdf_nbuf_data(htt_msg);
		cookie_val = *(msg_word + 1);
		htt_stats->msg_len = HTT_T2H_EXT_STATS_CONF_TLV_LENGTH_GET(
					*(msg_word +
					HTT_T2H_EXT_STATS_TLV_START_OFFSET));

		if (cookie_val) {
			if (dp_send_htt_stat_resp(htt_stats, soc, htt_msg)
					== QDF_STATUS_SUCCESS) {
				continue;
			}
		}

		cookie_msb = *(msg_word + 2);
		pdev_id = *(msg_word + 2) & HTT_PID_BIT_MASK;
		pdev = soc->pdev_list[pdev_id];

		if (!cookie_val && (cookie_msb & DBG_STATS_COOKIE_HTT_DBGFS)) {
			dp_htt_stats_dbgfs_send_msg(pdev, msg_word,
						    htt_stats->msg_len);
			qdf_nbuf_free(htt_msg);
			continue;
		}

		if (!cookie_val && (cookie_msb & DBG_SYSFS_STATS_COOKIE))
			dp_htt_stats_sysfs_update_config(pdev);

		if (cookie_msb & DBG_STATS_COOKIE_DP_STATS)
			copy_stats = true;

		/* read 5th word */
		msg_word = msg_word + 4;
		msg_remain_len = qdf_min(htt_stats->msg_len,
				(uint32_t) DP_EXT_MSG_LENGTH);
		/* Keep processing the node till node length is 0 */
		while (msg_remain_len) {
			/*
			 * if message is not a continuation of previous message
			 * read the tlv type and tlv length
			 */
			if (!tlv_buf_head) {
				tlv_type = HTT_STATS_TLV_TAG_GET(
						*msg_word);
				tlv_remain_len = HTT_STATS_TLV_LENGTH_GET(
						*msg_word);
			}

			if (tlv_remain_len == 0) {
				msg_remain_len = 0;

				if (tlv_buf_head) {
					qdf_mem_free(tlv_buf_head);
					tlv_buf_head = NULL;
					tlv_buf_tail = NULL;
				}

				goto error;
			}

			if (!tlv_buf_head)
				tlv_remain_len += HTT_TLV_HDR_LEN;

			if ((tlv_remain_len <= msg_remain_len)) {
				/* Case 3 */
				if (tlv_buf_head) {
					qdf_mem_copy(tlv_buf_tail,
							(uint8_t *)msg_word,
							tlv_remain_len);
					tlv_start = (uint32_t *)tlv_buf_head;
				} else {
					/* Case 1 */
					tlv_start = msg_word;
				}

				if (copy_stats)
					dp_htt_stats_copy_tag(pdev,
							      tlv_type,
							      tlv_start);
				else
					dp_htt_stats_print_tag(pdev,
							       tlv_type,
							       tlv_start);

				if (tlv_type == HTT_STATS_PEER_DETAILS_TAG ||
				    tlv_type == HTT_STATS_PEER_STATS_CMN_TAG)
					dp_peer_update_inactive_time(pdev,
								     tlv_type,
								     tlv_start);

				msg_remain_len -= tlv_remain_len;

				msg_word = (uint32_t *)
					(((uint8_t *)msg_word) +
					tlv_remain_len);

				tlv_remain_len = 0;

				if (tlv_buf_head) {
					qdf_mem_free(tlv_buf_head);
					tlv_buf_head = NULL;
					tlv_buf_tail = NULL;
				}

			} else { /* tlv_remain_len > msg_remain_len */
				/* Case 2 & 3 */
				if (!tlv_buf_head) {
					tlv_buf_head = qdf_mem_malloc(
							tlv_remain_len);

					if (!tlv_buf_head) {
						QDF_TRACE(QDF_MODULE_ID_TXRX,
								QDF_TRACE_LEVEL_ERROR,
								"Alloc failed");
						goto error;
					}

					tlv_buf_tail = tlv_buf_head;
				}

				qdf_mem_copy(tlv_buf_tail, (uint8_t *)msg_word,
						msg_remain_len);
				tlv_remain_len -= msg_remain_len;
				tlv_buf_tail += msg_remain_len;
			}
		}

		if (htt_stats->msg_len >= DP_EXT_MSG_LENGTH) {
			htt_stats->msg_len -= DP_EXT_MSG_LENGTH;
		}

		/* indicate event completion in case the event is done */
		if (!cookie_val && (cookie_msb & DBG_SYSFS_STATS_COOKIE))
			dp_htt_stats_sysfs_set_event(soc, msg_word);

		qdf_nbuf_free(htt_msg);
	}
	return;

error:
	qdf_nbuf_free(htt_msg);
	while ((htt_msg = qdf_nbuf_queue_remove(&htt_stats->msg))
			!= NULL)
		qdf_nbuf_free(htt_msg);
}

void htt_t2h_stats_handler(void *context)
{
	struct dp_soc *soc = (struct dp_soc *)context;
	struct htt_stats_context htt_stats;
	uint32_t *msg_word;
	qdf_nbuf_t htt_msg = NULL;
	uint8_t done;
	uint32_t rem_stats;

	if (!soc) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "soc is NULL");
		return;
	}

	if (!qdf_atomic_read(&soc->cmn_init_done)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "soc: 0x%pK, init_done: %d", soc,
			  qdf_atomic_read(&soc->cmn_init_done));
		return;
	}

	qdf_mem_zero(&htt_stats, sizeof(htt_stats));
	qdf_nbuf_queue_init(&htt_stats.msg);

	/* pull one completed stats from soc->htt_stats_msg and process */
	qdf_spin_lock_bh(&soc->htt_stats.lock);
	if (!soc->htt_stats.num_stats) {
		qdf_spin_unlock_bh(&soc->htt_stats.lock);
		return;
	}
	while ((htt_msg = qdf_nbuf_queue_remove(&soc->htt_stats.msg)) != NULL) {
		msg_word = (uint32_t *) qdf_nbuf_data(htt_msg);
		msg_word = msg_word + HTT_T2H_EXT_STATS_TLV_START_OFFSET;
		done = HTT_T2H_EXT_STATS_CONF_TLV_DONE_GET(*msg_word);
		qdf_nbuf_queue_add(&htt_stats.msg, htt_msg);
		/*
		 * Done bit signifies that this is the last T2H buffer in the
		 * stream of HTT EXT STATS message
		 */
		if (done)
			break;
	}
	rem_stats = --soc->htt_stats.num_stats;
	qdf_spin_unlock_bh(&soc->htt_stats.lock);

	/* If there are more stats to process, schedule stats work again.
	 * Scheduling prior to processing ht_stats to queue with early
	 * index
	 */
	if (rem_stats)
		qdf_sched_work(0, &soc->htt_stats.work);

	dp_process_htt_stat_msg(&htt_stats, soc);
}

/**
 * dp_txrx_fw_stats_handler() - Function to process HTT EXT stats
 * @soc: DP SOC handle
 * @htt_t2h_msg: HTT message nbuf
 *
 * return:void
 */
static inline void dp_txrx_fw_stats_handler(struct dp_soc *soc,
					    qdf_nbuf_t htt_t2h_msg)
{
	uint8_t done;
	qdf_nbuf_t msg_copy;
	uint32_t *msg_word;

	msg_word = (uint32_t *)qdf_nbuf_data(htt_t2h_msg);
	msg_word = msg_word + 3;
	done = HTT_T2H_EXT_STATS_CONF_TLV_DONE_GET(*msg_word);

	/*
	 * HTT EXT stats response comes as stream of TLVs which span over
	 * multiple T2H messages.
	 * The first message will carry length of the response.
	 * For rest of the messages length will be zero.
	 *
	 * Clone the T2H message buffer and store it in a list to process
	 * it later.
	 *
	 * The original T2H message buffers gets freed in the T2H HTT event
	 * handler
	 */
	msg_copy = qdf_nbuf_clone(htt_t2h_msg);

	if (!msg_copy) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
			  "T2H messge clone failed for HTT EXT STATS");
		goto error;
	}

	qdf_spin_lock_bh(&soc->htt_stats.lock);
	qdf_nbuf_queue_add(&soc->htt_stats.msg, msg_copy);
	/*
	 * Done bit signifies that this is the last T2H buffer in the stream of
	 * HTT EXT STATS message
	 */
	if (done) {
		soc->htt_stats.num_stats++;
		qdf_sched_work(0, &soc->htt_stats.work);
	}
	qdf_spin_unlock_bh(&soc->htt_stats.lock);

	return;

error:
	qdf_spin_lock_bh(&soc->htt_stats.lock);
	while ((msg_copy = qdf_nbuf_queue_remove(&soc->htt_stats.msg))
			!= NULL) {
		qdf_nbuf_free(msg_copy);
	}
	soc->htt_stats.num_stats = 0;
	qdf_spin_unlock_bh(&soc->htt_stats.lock);
	return;
}

/*
 * htt_soc_attach_target() - SOC level HTT setup
 * @htt_soc:	HTT SOC handle
 *
 * Return: 0 on success; error code on failure
 */
int htt_soc_attach_target(struct htt_soc *htt_soc)
{
	struct htt_soc *soc = (struct htt_soc *)htt_soc;

	return htt_h2t_ver_req_msg(soc);
}

void htt_set_htc_handle(struct htt_soc *htt_soc, HTC_HANDLE htc_soc)
{
	htt_soc->htc_soc = htc_soc;
}

HTC_HANDLE htt_get_htc_handle(struct htt_soc *htt_soc)
{
	return htt_soc->htc_soc;
}

struct htt_soc *htt_soc_attach(struct dp_soc *soc, HTC_HANDLE htc_handle)
{
	int i;
	int j;
	int alloc_size = HTT_SW_UMAC_RING_IDX_MAX * sizeof(unsigned long);
	struct htt_soc *htt_soc = NULL;

	htt_soc = qdf_mem_malloc(sizeof(*htt_soc));
	if (!htt_soc) {
		dp_err("HTT attach failed");
		return NULL;
	}

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		htt_soc->pdevid_tt[i].umac_ttt = qdf_mem_malloc(alloc_size);
		if (!htt_soc->pdevid_tt[i].umac_ttt)
			break;
		qdf_mem_set(htt_soc->pdevid_tt[i].umac_ttt, alloc_size, -1);
		htt_soc->pdevid_tt[i].lmac_ttt = qdf_mem_malloc(alloc_size);
		if (!htt_soc->pdevid_tt[i].lmac_ttt) {
			qdf_mem_free(htt_soc->pdevid_tt[i].umac_ttt);
			break;
		}
		qdf_mem_set(htt_soc->pdevid_tt[i].lmac_ttt, alloc_size, -1);
	}
	if (i != MAX_PDEV_CNT) {
		for (j = 0; j < i; j++) {
			qdf_mem_free(htt_soc->pdevid_tt[j].umac_ttt);
			qdf_mem_free(htt_soc->pdevid_tt[j].lmac_ttt);
		}
		qdf_mem_free(htt_soc);
		return NULL;
	}

	htt_soc->dp_soc = soc;
	htt_soc->htc_soc = htc_handle;
	HTT_TX_MUTEX_INIT(&htt_soc->htt_tx_mutex);

	return htt_soc;
}

#if defined(WDI_EVENT_ENABLE) && \
	!defined(REMOVE_PKT_LOG)
/*
 * dp_pktlog_msg_handler() - Pktlog msg handler
 * @htt_soc:	 HTT SOC handle
 * @msg_word:    Pointer to payload
 *
 * Return: None
 */
static void
dp_pktlog_msg_handler(struct htt_soc *soc,
		      uint32_t *msg_word)
{
	uint8_t pdev_id;
	uint8_t target_pdev_id;
	uint32_t *pl_hdr;

	target_pdev_id = HTT_T2H_PKTLOG_PDEV_ID_GET(*msg_word);
	pdev_id = dp_get_host_pdev_id_for_target_pdev_id(soc->dp_soc,
							 target_pdev_id);
	pl_hdr = (msg_word + 1);
	dp_wdi_event_handler(WDI_EVENT_OFFLOAD_ALL, soc->dp_soc,
		pl_hdr, HTT_INVALID_PEER, WDI_NO_VAL,
		pdev_id);
}
#else
static void
dp_pktlog_msg_handler(struct htt_soc *soc,
		      uint32_t *msg_word)
{
}
#endif

#ifdef QCA_VDEV_STATS_HW_OFFLOAD_SUPPORT
/*
 * dp_vdev_txrx_hw_stats_handler - Handle vdev stats received from FW
 * @soc - htt soc handle
 * @ msg_word - buffer containing stats
 *
 * Return: void
 */
static void dp_vdev_txrx_hw_stats_handler(struct htt_soc *soc,
					  uint32_t *msg_word)
{
	struct dp_soc *dpsoc = (struct dp_soc *)soc->dp_soc;
	uint8_t pdev_id;
	uint8_t vdev_id;
	uint8_t target_pdev_id;
	uint16_t payload_size;
	struct dp_pdev *pdev;
	struct dp_vdev *vdev;
	uint8_t *tlv_buf;
	uint32_t *tlv_buf_temp;
	uint32_t *tag_buf;
	htt_tlv_tag_t tlv_type;
	uint16_t tlv_length;
	uint64_t pkt_count = 0;
	uint64_t byte_count = 0;
	uint64_t soc_drop_cnt = 0;
	struct cdp_pkt_info tx_comp = { 0 };
	struct cdp_pkt_info tx_failed =  { 0 };

	target_pdev_id =
		HTT_T2H_VDEVS_TXRX_STATS_PERIODIC_IND_PDEV_ID_GET(*msg_word);
	pdev_id = dp_get_host_pdev_id_for_target_pdev_id(dpsoc,
							 target_pdev_id);

	if (pdev_id >= MAX_PDEV_CNT)
		return;

	pdev = dpsoc->pdev_list[pdev_id];
	if (!pdev) {
		dp_err("PDEV is NULL for pdev_id:%d", pdev_id);
		return;
	}

	payload_size =
	HTT_T2H_VDEVS_TXRX_STATS_PERIODIC_IND_PAYLOAD_SIZE_GET(*msg_word);

	qdf_trace_hex_dump(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
			   (void *)msg_word, payload_size + 16);

	/* Adjust msg_word to point to the first TLV in buffer */
	msg_word = msg_word + 4;

	/* Parse the received buffer till payload size reaches 0 */
	while (payload_size > 0) {
		tlv_buf = (uint8_t *)msg_word;
		tlv_buf_temp = msg_word;
		tlv_type = HTT_STATS_TLV_TAG_GET(*msg_word);
		tlv_length = HTT_STATS_TLV_LENGTH_GET(*msg_word);

		/* Add header size to tlv length*/
		tlv_length += 4;

		switch (tlv_type) {
		case HTT_STATS_SOC_TXRX_STATS_COMMON_TAG:
		{
			tag_buf = tlv_buf_temp +
					HTT_VDEV_STATS_GET_INDEX(SOC_DROP_CNT);
			soc_drop_cnt = HTT_VDEV_GET_STATS_U64(tag_buf);
			DP_STATS_UPD(dpsoc, tx.tqm_drop_no_peer, soc_drop_cnt);
			break;
		}
		case HTT_STATS_VDEV_TXRX_STATS_HW_STATS_TAG:
		{
			tag_buf = tlv_buf_temp +
					HTT_VDEV_STATS_GET_INDEX(VDEV_ID);
			vdev_id = (uint8_t)(*tag_buf);
			vdev = dp_vdev_get_ref_by_id(dpsoc, vdev_id,
						     DP_MOD_ID_HTT);

			if (!vdev)
				goto invalid_vdev;

			/* Extract received packet count from buffer */
			tag_buf = tlv_buf_temp +
					HTT_VDEV_STATS_GET_INDEX(RX_PKT_CNT);
			pkt_count = HTT_VDEV_GET_STATS_U64(tag_buf);
			DP_STATS_UPD(vdev, rx_i.reo_rcvd_pkt.num, pkt_count);

			/* Extract received packet byte count from buffer */
			tag_buf = tlv_buf_temp +
					HTT_VDEV_STATS_GET_INDEX(RX_BYTE_CNT);
			byte_count = HTT_VDEV_GET_STATS_U64(tag_buf);
			DP_STATS_UPD(vdev, rx_i.reo_rcvd_pkt.bytes, byte_count);

			/* Extract tx success packet count from buffer */
			tag_buf = tlv_buf_temp +
				HTT_VDEV_STATS_GET_INDEX(TX_SUCCESS_PKT_CNT);
			pkt_count = HTT_VDEV_GET_STATS_U64(tag_buf);
			tx_comp.num = pkt_count;

			/* Extract tx success packet byte count from buffer */
			tag_buf = tlv_buf_temp +
				HTT_VDEV_STATS_GET_INDEX(TX_SUCCESS_BYTE_CNT);
			byte_count = HTT_VDEV_GET_STATS_U64(tag_buf);
			tx_comp.bytes = byte_count;

			/* Extract tx retry packet count from buffer */
			tag_buf = tlv_buf_temp +
				HTT_VDEV_STATS_GET_INDEX(TX_RETRY_PKT_CNT);
			pkt_count = HTT_VDEV_GET_STATS_U64(tag_buf);
			tx_comp.num += pkt_count;
			tx_failed.num = pkt_count;

			/* Extract tx retry packet byte count from buffer */
			tag_buf = tlv_buf_temp +
				HTT_VDEV_STATS_GET_INDEX(TX_RETRY_BYTE_CNT);
			byte_count = HTT_VDEV_GET_STATS_U64(tag_buf);
			tx_comp.bytes += byte_count;
			tx_failed.bytes = byte_count;

			/* Extract tx drop packet count from buffer */
			tag_buf = tlv_buf_temp +
				HTT_VDEV_STATS_GET_INDEX(TX_DROP_PKT_CNT);
			pkt_count = HTT_VDEV_GET_STATS_U64(tag_buf);
			tx_comp.num += pkt_count;
			tx_failed.num += pkt_count;

			/* Extract tx drop packet byte count from buffer */
			tag_buf = tlv_buf_temp +
				HTT_VDEV_STATS_GET_INDEX(TX_DROP_BYTE_CNT);
			byte_count = HTT_VDEV_GET_STATS_U64(tag_buf);
			tx_comp.bytes += byte_count;
			tx_failed.bytes += byte_count;

			/* Extract tx age-out packet count from buffer */
			tag_buf = tlv_buf_temp +
				HTT_VDEV_STATS_GET_INDEX(TX_AGE_OUT_PKT_CNT);
			pkt_count = HTT_VDEV_GET_STATS_U64(tag_buf);
			tx_comp.num += pkt_count;
			tx_failed.num += pkt_count;

			/* Extract tx age-out packet byte count from buffer */
			tag_buf = tlv_buf_temp +
				HTT_VDEV_STATS_GET_INDEX(TX_AGE_OUT_BYTE_CNT);
			byte_count = HTT_VDEV_GET_STATS_U64(tag_buf);
			tx_comp.bytes += byte_count;
			tx_failed.bytes += byte_count;

			DP_STATS_UPD(vdev, tx.comp_pkt.num, tx_comp.num);
			DP_STATS_UPD(vdev, tx.comp_pkt.bytes, tx_comp.bytes);

			DP_STATS_UPD(vdev, tx.tx_failed, tx_failed.num);

			dp_vdev_unref_delete(dpsoc, vdev, DP_MOD_ID_HTT);
			break;
		}
		default:
			qdf_assert(0);
		}
invalid_vdev:
		msg_word = (uint32_t *)((uint8_t *)tlv_buf + tlv_length);
		payload_size -= tlv_length;
	}
}
#else
static void dp_vdev_txrx_hw_stats_handler(struct htt_soc *soc,
					  uint32_t *msg_word)
{}
#endif

/*
 * time_allow_print() - time allow print
 * @htt_ring_tt:	ringi_id array of timestamps
 * @ring_id:		ring_id (index)
 *
 * Return: 1 for successfully saving timestamp in array
 *	and 0 for timestamp falling within 2 seconds after last one
 */
static bool time_allow_print(unsigned long *htt_ring_tt, u_int8_t ring_id)
{
	unsigned long tstamp;
	unsigned long delta;

	tstamp = qdf_get_system_timestamp();

	if (!htt_ring_tt)
		return 0; //unable to print backpressure messages

	if (htt_ring_tt[ring_id] == -1) {
		htt_ring_tt[ring_id] = tstamp;
		return 1;
	}
	delta = tstamp - htt_ring_tt[ring_id];
	if (delta >= 2000) {
		htt_ring_tt[ring_id] = tstamp;
		return 1;
	}

	return 0;
}

static void dp_htt_alert_print(enum htt_t2h_msg_type msg_type,
			       struct dp_pdev *pdev, u_int8_t ring_id,
			       u_int16_t hp_idx, u_int16_t tp_idx,
			       u_int32_t bkp_time, char *ring_stype)
{
	dp_alert("seq_num %u msg_type: %d pdev_id: %d ring_type: %s ",
		 pdev->bkp_stats.seq_num, msg_type, pdev->pdev_id, ring_stype);
	dp_alert("ring_id: %d hp_idx: %d tp_idx: %d bkpressure_time_ms: %d ",
		 ring_id, hp_idx, tp_idx, bkp_time);
}

/**
 * dp_get_srng_ring_state_from_hal(): Get hal level ring stats
 * @soc: DP_SOC handle
 * @srng: DP_SRNG handle
 * @ring_type: srng src/dst ring
 *
 * Return: void
 */
static QDF_STATUS
dp_get_srng_ring_state_from_hal(struct dp_soc *soc,
				struct dp_pdev *pdev,
				struct dp_srng *srng,
				enum hal_ring_type ring_type,
				struct dp_srng_ring_state *state)
{
	struct hal_soc *hal_soc;

	if (!soc || !srng || !srng->hal_srng || !state)
		return QDF_STATUS_E_INVAL;

	hal_soc = (struct hal_soc *)soc->hal_soc;

	hal_get_sw_hptp(soc->hal_soc, srng->hal_srng, &state->sw_tail,
			&state->sw_head);

	hal_get_hw_hptp(soc->hal_soc, srng->hal_srng, &state->hw_head,
			&state->hw_tail, ring_type);

	state->ring_type = ring_type;

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_MONITOR_PKT_SUPPORT
static void
dp_queue_mon_ring_stats(struct dp_pdev *pdev,
			int lmac_id, uint32_t *num_srng,
			struct dp_soc_srngs_state *soc_srngs_state)
{
	QDF_STATUS status;

	if (pdev->soc->wlan_cfg_ctx->rxdma1_enable) {
		status = dp_get_srng_ring_state_from_hal
			(pdev->soc, pdev,
			 &pdev->soc->rxdma_mon_buf_ring[lmac_id],
			 RXDMA_MONITOR_BUF,
			 &soc_srngs_state->ring_state[*num_srng]);

		if (status == QDF_STATUS_SUCCESS)
			qdf_assert_always(++(*num_srng) < DP_MAX_SRNGS);

		status = dp_get_srng_ring_state_from_hal
			(pdev->soc, pdev,
			 &pdev->soc->rxdma_mon_dst_ring[lmac_id],
			 RXDMA_MONITOR_DST,
			 &soc_srngs_state->ring_state[*num_srng]);

		if (status == QDF_STATUS_SUCCESS)
			qdf_assert_always(++(*num_srng) < DP_MAX_SRNGS);

		status = dp_get_srng_ring_state_from_hal
			(pdev->soc, pdev,
			 &pdev->soc->rxdma_mon_desc_ring[lmac_id],
			 RXDMA_MONITOR_DESC,
			 &soc_srngs_state->ring_state[*num_srng]);

		if (status == QDF_STATUS_SUCCESS)
			qdf_assert_always(++(*num_srng) < DP_MAX_SRNGS);
	}
}
#else
static void
dp_queue_mon_ring_stats(struct dp_pdev *pdev,
			int lmac_id, uint32_t *num_srng,
			struct dp_soc_srngs_state *soc_srngs_state)
{
}
#endif

#ifndef WLAN_DP_DISABLE_TCL_CMD_CRED_SRNG
static inline QDF_STATUS
dp_get_tcl_cmd_cred_ring_state_from_hal(struct dp_pdev *pdev,
					struct dp_srng_ring_state *ring_state)
{
	return dp_get_srng_ring_state_from_hal(pdev->soc, pdev,
					       &pdev->soc->tcl_cmd_credit_ring,
					       TCL_CMD_CREDIT, ring_state);
}
#else
static inline QDF_STATUS
dp_get_tcl_cmd_cred_ring_state_from_hal(struct dp_pdev *pdev,
					struct dp_srng_ring_state *ring_state)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#ifndef WLAN_DP_DISABLE_TCL_STATUS_SRNG
static inline QDF_STATUS
dp_get_tcl_status_ring_state_from_hal(struct dp_pdev *pdev,
				      struct dp_srng_ring_state *ring_state)
{
	return dp_get_srng_ring_state_from_hal(pdev->soc, pdev,
					       &pdev->soc->tcl_status_ring,
					       TCL_STATUS, ring_state);
}
#else
static inline QDF_STATUS
dp_get_tcl_status_ring_state_from_hal(struct dp_pdev *pdev,
				      struct dp_srng_ring_state *ring_state)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * dp_queue_srng_ring_stats(): Print pdev hal level ring stats
 * @pdev: DP_pdev handle
 *
 * Return: void
 */
static void dp_queue_ring_stats(struct dp_pdev *pdev)
{
	uint32_t i;
	int mac_id;
	int lmac_id;
	uint32_t j = 0;
	struct dp_soc *soc = pdev->soc;
	struct dp_soc_srngs_state * soc_srngs_state = NULL;
	struct dp_soc_srngs_state *drop_srngs_state = NULL;
	QDF_STATUS status;

	soc_srngs_state = qdf_mem_malloc(sizeof(struct dp_soc_srngs_state));
	if (!soc_srngs_state) {
		dp_htt_alert("Memory alloc failed for back pressure event");
		return;
	}

	status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->soc->reo_exception_ring,
				 REO_EXCEPTION,
				 &soc_srngs_state->ring_state[j]);

	if (status == QDF_STATUS_SUCCESS)
		qdf_assert_always(++j < DP_MAX_SRNGS);

	status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->soc->reo_reinject_ring,
				 REO_REINJECT,
				 &soc_srngs_state->ring_state[j]);

	if (status == QDF_STATUS_SUCCESS)
		qdf_assert_always(++j < DP_MAX_SRNGS);

	status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->soc->reo_cmd_ring,
				 REO_CMD,
				 &soc_srngs_state->ring_state[j]);

	if (status == QDF_STATUS_SUCCESS)
		qdf_assert_always(++j < DP_MAX_SRNGS);

	status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->soc->reo_status_ring,
				 REO_STATUS,
				 &soc_srngs_state->ring_state[j]);

	if (status == QDF_STATUS_SUCCESS)
		qdf_assert_always(++j < DP_MAX_SRNGS);

	status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->soc->rx_rel_ring,
				 WBM2SW_RELEASE,
				 &soc_srngs_state->ring_state[j]);

	if (status == QDF_STATUS_SUCCESS)
		qdf_assert_always(++j < DP_MAX_SRNGS);

	status = dp_get_tcl_cmd_cred_ring_state_from_hal
				(pdev, &soc_srngs_state->ring_state[j]);
	if (status == QDF_STATUS_SUCCESS)
		qdf_assert_always(++j < DP_MAX_SRNGS);

	status = dp_get_tcl_status_ring_state_from_hal
				(pdev, &soc_srngs_state->ring_state[j]);
	if (status == QDF_STATUS_SUCCESS)
		qdf_assert_always(++j < DP_MAX_SRNGS);

	status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->soc->wbm_desc_rel_ring,
				 SW2WBM_RELEASE,
				 &soc_srngs_state->ring_state[j]);

	if (status == QDF_STATUS_SUCCESS)
		qdf_assert_always(++j < DP_MAX_SRNGS);

	for (i = 0; i < MAX_REO_DEST_RINGS; i++) {
		status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->soc->reo_dest_ring[i],
				 REO_DST,
				 &soc_srngs_state->ring_state[j]);

		if (status == QDF_STATUS_SUCCESS)
			qdf_assert_always(++j < DP_MAX_SRNGS);
	}

	for (i = 0; i < pdev->soc->num_tcl_data_rings; i++) {
		status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->soc->tcl_data_ring[i],
				 TCL_DATA,
				 &soc_srngs_state->ring_state[j]);

		if (status == QDF_STATUS_SUCCESS)
			qdf_assert_always(++j < DP_MAX_SRNGS);
	}

	for (i = 0; i < MAX_TCL_DATA_RINGS; i++) {
		status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->soc->tx_comp_ring[i],
				 WBM2SW_RELEASE,
				 &soc_srngs_state->ring_state[j]);

		if (status == QDF_STATUS_SUCCESS)
			qdf_assert_always(++j < DP_MAX_SRNGS);
	}

	lmac_id = dp_get_lmac_id_for_pdev_id(pdev->soc, 0, pdev->pdev_id);
	status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->soc->rx_refill_buf_ring
				 [lmac_id],
				 RXDMA_BUF,
				 &soc_srngs_state->ring_state[j]);

	if (status == QDF_STATUS_SUCCESS)
		qdf_assert_always(++j < DP_MAX_SRNGS);

	status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->rx_refill_buf_ring2,
				 RXDMA_BUF,
				 &soc_srngs_state->ring_state[j]);

	if (status == QDF_STATUS_SUCCESS)
		qdf_assert_always(++j < DP_MAX_SRNGS);


	for (i = 0; i < MAX_RX_MAC_RINGS; i++) {
		dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->rx_mac_buf_ring[i],
				 RXDMA_BUF,
				 &soc_srngs_state->ring_state[j]);

		if (status == QDF_STATUS_SUCCESS)
			qdf_assert_always(++j < DP_MAX_SRNGS);
	}

	for (mac_id = 0;
	     mac_id  < soc->wlan_cfg_ctx->num_rxdma_status_rings_per_pdev;
	     mac_id++) {
		lmac_id = dp_get_lmac_id_for_pdev_id(pdev->soc,
						     mac_id, pdev->pdev_id);

		dp_queue_mon_ring_stats(pdev, lmac_id, &j,
					soc_srngs_state);

		status = dp_get_srng_ring_state_from_hal
			(pdev->soc, pdev,
			 &pdev->soc->rxdma_mon_status_ring[lmac_id],
			 RXDMA_MONITOR_STATUS,
			 &soc_srngs_state->ring_state[j]);

		if (status == QDF_STATUS_SUCCESS)
			qdf_assert_always(++j < DP_MAX_SRNGS);
	}

	for (i = 0; i < soc->wlan_cfg_ctx->num_rxdma_dst_rings_per_pdev; i++) {
		lmac_id = dp_get_lmac_id_for_pdev_id(pdev->soc,
						     i, pdev->pdev_id);

		status = dp_get_srng_ring_state_from_hal
				(pdev->soc, pdev,
				 &pdev->soc->rxdma_err_dst_ring
				 [lmac_id],
				 RXDMA_DST,
				 &soc_srngs_state->ring_state[j]);

		if (status == QDF_STATUS_SUCCESS)
			qdf_assert_always(++j < DP_MAX_SRNGS);
	}
	soc_srngs_state->max_ring_id = j;

	qdf_spin_lock_bh(&pdev->bkp_stats.list_lock);

	soc_srngs_state->seq_num = pdev->bkp_stats.seq_num;

	if (pdev->bkp_stats.queue_depth >= HTT_BKP_STATS_MAX_QUEUE_DEPTH) {
		drop_srngs_state = TAILQ_FIRST(&pdev->bkp_stats.list);
		qdf_assert_always(drop_srngs_state);
		TAILQ_REMOVE(&pdev->bkp_stats.list, drop_srngs_state,
			     list_elem);
		qdf_mem_free(drop_srngs_state);
		pdev->bkp_stats.queue_depth--;
	}

	pdev->bkp_stats.queue_depth++;
	TAILQ_INSERT_TAIL(&pdev->bkp_stats.list, soc_srngs_state,
			  list_elem);
	pdev->bkp_stats.seq_num++;
	qdf_spin_unlock_bh(&pdev->bkp_stats.list_lock);

	qdf_queue_work(0, pdev->bkp_stats.work_queue,
		       &pdev->bkp_stats.work);
}

/*
 * dp_htt_bkp_event_alert() - htt backpressure event alert
 * @msg_word:	htt packet context
 * @htt_soc:	HTT SOC handle
 *
 * Return: after attempting to print stats
 */
static void dp_htt_bkp_event_alert(u_int32_t *msg_word, struct htt_soc *soc)
{
	u_int8_t ring_type;
	u_int8_t pdev_id;
	uint8_t target_pdev_id;
	u_int8_t ring_id;
	u_int16_t hp_idx;
	u_int16_t tp_idx;
	u_int32_t bkp_time;
	enum htt_t2h_msg_type msg_type;
	struct dp_soc *dpsoc;
	struct dp_pdev *pdev;
	struct dp_htt_timestamp *radio_tt;

	if (!soc)
		return;

	dpsoc = (struct dp_soc *)soc->dp_soc;
	msg_type = HTT_T2H_MSG_TYPE_GET(*msg_word);
	ring_type = HTT_T2H_RX_BKPRESSURE_RING_TYPE_GET(*msg_word);
	target_pdev_id = HTT_T2H_RX_BKPRESSURE_PDEV_ID_GET(*msg_word);
	pdev_id = dp_get_host_pdev_id_for_target_pdev_id(soc->dp_soc,
							 target_pdev_id);
	if (pdev_id >= MAX_PDEV_CNT) {
		dp_htt_debug("%pK: pdev id %d is invalid", soc, pdev_id);
		return;
	}

	pdev = (struct dp_pdev *)dpsoc->pdev_list[pdev_id];
	ring_id = HTT_T2H_RX_BKPRESSURE_RINGID_GET(*msg_word);
	hp_idx = HTT_T2H_RX_BKPRESSURE_HEAD_IDX_GET(*(msg_word + 1));
	tp_idx = HTT_T2H_RX_BKPRESSURE_TAIL_IDX_GET(*(msg_word + 1));
	bkp_time = HTT_T2H_RX_BKPRESSURE_TIME_MS_GET(*(msg_word + 2));
	radio_tt = &soc->pdevid_tt[pdev_id];

	switch (ring_type) {
	case HTT_SW_RING_TYPE_UMAC:
		if (!time_allow_print(radio_tt->umac_ttt, ring_id))
			return;
		dp_htt_alert_print(msg_type, pdev, ring_id, hp_idx, tp_idx,
				   bkp_time, "HTT_SW_RING_TYPE_UMAC");
	break;
	case HTT_SW_RING_TYPE_LMAC:
		if (!time_allow_print(radio_tt->lmac_ttt, ring_id))
			return;
		dp_htt_alert_print(msg_type, pdev, ring_id, hp_idx, tp_idx,
				   bkp_time, "HTT_SW_RING_TYPE_LMAC");
	break;
	default:
		dp_htt_alert_print(msg_type, pdev, ring_id, hp_idx, tp_idx,
				   bkp_time, "UNKNOWN");
	break;
	}

	dp_queue_ring_stats(pdev);
}

#ifdef WLAN_FEATURE_PKT_CAPTURE_V2
/*
 * dp_offload_ind_handler() - offload msg handler
 * @htt_soc: HTT SOC handle
 * @msg_word: Pointer to payload
 *
 * Return: None
 */
static void
dp_offload_ind_handler(struct htt_soc *soc, uint32_t *msg_word)
{
	u_int8_t pdev_id;
	u_int8_t target_pdev_id;

	target_pdev_id = HTT_T2H_PPDU_STATS_PDEV_ID_GET(*msg_word);
	pdev_id = dp_get_host_pdev_id_for_target_pdev_id(soc->dp_soc,
							 target_pdev_id);
	dp_wdi_event_handler(WDI_EVENT_PKT_CAPTURE_OFFLOAD_TX_DATA, soc->dp_soc,
			     msg_word, HTT_INVALID_VDEV, WDI_NO_VAL,
			     pdev_id);
}
#else
static void
dp_offload_ind_handler(struct htt_soc *soc, uint32_t *msg_word)
{
}
#endif

#ifdef WLAN_FEATURE_11BE_MLO
static void dp_htt_mlo_peer_map_handler(struct htt_soc *soc,
					uint32_t *msg_word)
{
	uint8_t mac_addr_deswizzle_buf[QDF_MAC_ADDR_SIZE];
	uint8_t *mlo_peer_mac_addr;
	uint16_t mlo_peer_id;
	uint8_t num_links;
	struct dp_mlo_flow_override_info mlo_flow_info[DP_MLO_FLOW_INFO_MAX];

	mlo_peer_id = HTT_RX_MLO_PEER_MAP_MLO_PEER_ID_GET(*msg_word);
	num_links =
		HTT_RX_MLO_PEER_MAP_NUM_LOGICAL_LINKS_GET(*msg_word);
	mlo_peer_mac_addr =
	htt_t2h_mac_addr_deswizzle((u_int8_t *)(msg_word + 1),
				   &mac_addr_deswizzle_buf[0]);

	mlo_flow_info[0].ast_idx =
		HTT_RX_MLO_PEER_MAP_PRIMARY_AST_INDEX_GET(*(msg_word + 3));
	mlo_flow_info[0].ast_idx_valid =
		HTT_RX_MLO_PEER_MAP_AST_INDEX_VALID_FLAG_GET(*(msg_word + 3));
	mlo_flow_info[0].chip_id =
		HTT_RX_MLO_PEER_MAP_CHIP_ID_AST_INDEX_GET(*(msg_word + 3));
	mlo_flow_info[0].tidmask =
		HTT_RX_MLO_PEER_MAP_TIDMASK_AST_INDEX_GET(*(msg_word + 3));
	mlo_flow_info[0].cache_set_num =
	HTT_RX_MLO_PEER_MAP_CACHE_SET_NUM_AST_INDEX_GET(*(msg_word + 3));

	mlo_flow_info[1].ast_idx =
		HTT_RX_MLO_PEER_MAP_PRIMARY_AST_INDEX_GET(*(msg_word + 3));
	mlo_flow_info[1].ast_idx_valid =
		HTT_RX_MLO_PEER_MAP_AST_INDEX_VALID_FLAG_GET(*(msg_word + 3));
	mlo_flow_info[1].chip_id =
		HTT_RX_MLO_PEER_MAP_CHIP_ID_AST_INDEX_GET(*(msg_word + 3));
	mlo_flow_info[1].tidmask =
		HTT_RX_MLO_PEER_MAP_TIDMASK_AST_INDEX_GET(*(msg_word + 3));
	mlo_flow_info[1].cache_set_num =
	HTT_RX_MLO_PEER_MAP_CACHE_SET_NUM_AST_INDEX_GET(*(msg_word + 3));

	mlo_flow_info[2].ast_idx =
		HTT_RX_MLO_PEER_MAP_PRIMARY_AST_INDEX_GET(*(msg_word + 3));
	mlo_flow_info[2].ast_idx_valid =
		HTT_RX_MLO_PEER_MAP_AST_INDEX_VALID_FLAG_GET(*(msg_word + 3));
	mlo_flow_info[2].chip_id =
		HTT_RX_MLO_PEER_MAP_CHIP_ID_AST_INDEX_GET(*(msg_word + 3));
	mlo_flow_info[2].tidmask =
		HTT_RX_MLO_PEER_MAP_TIDMASK_AST_INDEX_GET(*(msg_word + 3));
	mlo_flow_info[2].cache_set_num =
	HTT_RX_MLO_PEER_MAP_CACHE_SET_NUM_AST_INDEX_GET(*(msg_word + 3));

	dp_rx_mlo_peer_map_handler(soc->dp_soc, mlo_peer_id,
				   mlo_peer_mac_addr,
				   mlo_flow_info);
}

static void dp_htt_mlo_peer_unmap_handler(struct htt_soc *soc,
					  uint32_t *msg_word)
{
	uint16_t mlo_peer_id;

	mlo_peer_id = HTT_RX_MLO_PEER_UNMAP_MLO_PEER_ID_GET(*msg_word);
	dp_rx_mlo_peer_unmap_handler(soc->dp_soc, mlo_peer_id);
}

static void
dp_rx_mlo_timestamp_ind_handler(struct dp_soc *soc,
				uint32_t *msg_word)
{
	uint8_t pdev_id;
	uint8_t target_pdev_id;
	struct dp_pdev *pdev;

	if (!soc)
		return;

	target_pdev_id = HTT_T2H_MLO_TIMESTAMP_OFFSET_PDEV_ID_GET(*msg_word);
	pdev_id = dp_get_host_pdev_id_for_target_pdev_id(soc,
							 target_pdev_id);

	if (pdev_id >= MAX_PDEV_CNT) {
		dp_htt_debug("%pK: pdev id %d is invalid", soc, pdev_id);
		return;
	}

	pdev = (struct dp_pdev *)soc->pdev_list[pdev_id];

	if (!pdev) {
		dp_err("Invalid pdev");
		return;
	}
	dp_wdi_event_handler(WDI_EVENT_MLO_TSTMP, soc,
			     msg_word, HTT_INVALID_PEER, WDI_NO_VAL,
			     pdev_id);

	qdf_spin_lock_bh(&soc->htt_stats.lock);
	pdev->timestamp.msg_type =
		HTT_T2H_MLO_TIMESTAMP_OFFSET_MSG_TYPE_GET(*msg_word);
	pdev->timestamp.pdev_id = pdev_id;
	pdev->timestamp.chip_id =
		HTT_T2H_MLO_TIMESTAMP_OFFSET_CHIP_ID_GET(*msg_word);
	pdev->timestamp.mac_clk_freq =
		HTT_T2H_MLO_TIMESTAMP_OFFSET_MAC_CLK_FREQ_MHZ_GET(*msg_word);
	pdev->timestamp.sync_tstmp_lo_us = *(msg_word + 1);
	pdev->timestamp.sync_tstmp_hi_us = *(msg_word + 2);
	pdev->timestamp.mlo_offset_lo_us = *(msg_word + 3);
	pdev->timestamp.mlo_offset_hi_us = *(msg_word + 4);
	pdev->timestamp.mlo_offset_clks  = *(msg_word + 5);
	pdev->timestamp.mlo_comp_us =
	HTT_T2H_MLO_TIMESTAMP_OFFSET_MLO_TIMESTAMP_COMP_US_GET(
							*(msg_word + 6));
	pdev->timestamp.mlo_comp_clks =
	HTT_T2H_MLO_TIMESTAMP_OFFSET_MLO_TIMESTAMP_COMP_CLKS_GET(
							*(msg_word + 6));
	pdev->timestamp.mlo_comp_timer =
	HTT_T2H_MLO_TIMESTAMP_OFFSET_MLO_TIMESTAMP_COMP_PERIOD_US_GET(
							*(msg_word + 7));
	qdf_spin_unlock_bh(&soc->htt_stats.lock);
}
#else
static void dp_htt_mlo_peer_map_handler(struct htt_soc *soc,
					uint32_t *msg_word)
{
	qdf_assert_always(0);
}

static void dp_htt_mlo_peer_unmap_handler(struct htt_soc *soc,
					 uint32_t *msg_word)
{
	qdf_assert_always(0);
}

static void
dp_rx_mlo_timestamp_ind_handler(void *soc_handle,
				uint32_t *msg_word)
{
	qdf_assert_always(0);
}
#endif

/*
 * dp_htt_t2h_msg_handler() - Generic Target to host Msg/event handler
 * @context:	Opaque context (HTT SOC handle)
 * @pkt:	HTC packet
 */
static void dp_htt_t2h_msg_handler(void *context, HTC_PACKET *pkt)
{
	struct htt_soc *soc = (struct htt_soc *) context;
	qdf_nbuf_t htt_t2h_msg = (qdf_nbuf_t) pkt->pPktContext;
	u_int32_t *msg_word;
	enum htt_t2h_msg_type msg_type;
	bool free_buf = true;

	/* check for successful message reception */
	if (pkt->Status != QDF_STATUS_SUCCESS) {
		if (pkt->Status != QDF_STATUS_E_CANCELED)
			soc->stats.htc_err_cnt++;

		qdf_nbuf_free(htt_t2h_msg);
		return;
	}

	/* TODO: Check if we should pop the HTC/HTT header alignment padding */

	msg_word = (u_int32_t *) qdf_nbuf_data(htt_t2h_msg);
	msg_type = HTT_T2H_MSG_TYPE_GET(*msg_word);
	htt_event_record(soc->htt_logger_handle,
			 msg_type, (uint8_t *)msg_word);
	switch (msg_type) {
	case HTT_T2H_MSG_TYPE_BKPRESSURE_EVENT_IND:
	{
		dp_htt_bkp_event_alert(msg_word, soc);
		break;
	}
	case HTT_T2H_MSG_TYPE_PEER_MAP:
		{
			u_int8_t mac_addr_deswizzle_buf[QDF_MAC_ADDR_SIZE];
			u_int8_t *peer_mac_addr;
			u_int16_t peer_id;
			u_int16_t hw_peer_id;
			u_int8_t vdev_id;
			u_int8_t is_wds;
			struct dp_soc *dpsoc = (struct dp_soc *)soc->dp_soc;

			peer_id = HTT_RX_PEER_MAP_PEER_ID_GET(*msg_word);
			hw_peer_id =
				HTT_RX_PEER_MAP_HW_PEER_ID_GET(*(msg_word+2));
			vdev_id = HTT_RX_PEER_MAP_VDEV_ID_GET(*msg_word);
			peer_mac_addr = htt_t2h_mac_addr_deswizzle(
				(u_int8_t *) (msg_word+1),
				&mac_addr_deswizzle_buf[0]);
			QDF_TRACE(QDF_MODULE_ID_TXRX,
				QDF_TRACE_LEVEL_INFO,
				"HTT_T2H_MSG_TYPE_PEER_MAP msg for peer id %d vdev id %d n",
				peer_id, vdev_id);

			/*
			 * check if peer already exists for this peer_id, if so
			 * this peer map event is in response for a wds peer add
			 * wmi command sent during wds source port learning.
			 * in this case just add the ast entry to the existing
			 * peer ast_list.
			 */
			is_wds = !!(dpsoc->peer_id_to_obj_map[peer_id]);
			dp_rx_peer_map_handler(soc->dp_soc, peer_id, hw_peer_id,
					       vdev_id, peer_mac_addr, 0,
					       is_wds);
			break;
		}
	case HTT_T2H_MSG_TYPE_PEER_UNMAP:
		{
			u_int16_t peer_id;
			u_int8_t vdev_id;
			u_int8_t mac_addr[QDF_MAC_ADDR_SIZE] = {0};
			peer_id = HTT_RX_PEER_UNMAP_PEER_ID_GET(*msg_word);
			vdev_id = HTT_RX_PEER_UNMAP_VDEV_ID_GET(*msg_word);

			dp_rx_peer_unmap_handler(soc->dp_soc, peer_id,
						 vdev_id, mac_addr, 0,
						 DP_PEER_WDS_COUNT_INVALID);
			break;
		}
	case HTT_T2H_MSG_TYPE_SEC_IND:
		{
			u_int16_t peer_id;
			enum cdp_sec_type sec_type;
			int is_unicast;

			peer_id = HTT_SEC_IND_PEER_ID_GET(*msg_word);
			sec_type = HTT_SEC_IND_SEC_TYPE_GET(*msg_word);
			is_unicast = HTT_SEC_IND_UNICAST_GET(*msg_word);
			/* point to the first part of the Michael key */
			msg_word++;
			dp_rx_sec_ind_handler(
				soc->dp_soc, peer_id, sec_type, is_unicast,
				msg_word, msg_word + 2);
			break;
		}

	case HTT_T2H_MSG_TYPE_PPDU_STATS_IND:
		{
			free_buf =
				dp_monitor_ppdu_stats_ind_handler(soc,
								  msg_word,
								  htt_t2h_msg);
			break;
		}

	case HTT_T2H_MSG_TYPE_PKTLOG:
		{
			dp_pktlog_msg_handler(soc, msg_word);
			break;
		}

	case HTT_T2H_MSG_TYPE_VERSION_CONF:
		{
			/*
			 * HTC maintains runtime pm count for H2T messages that
			 * have a response msg from FW. This count ensures that
			 * in the case FW does not sent out the response or host
			 * did not process this indication runtime_put happens
			 * properly in the cleanup path.
			 */
			if (htc_dec_return_runtime_cnt(soc->htc_soc) >= 0)
				htc_pm_runtime_put(soc->htc_soc);
			else
				soc->stats.htt_ver_req_put_skip++;
			soc->tgt_ver.major = HTT_VER_CONF_MAJOR_GET(*msg_word);
			soc->tgt_ver.minor = HTT_VER_CONF_MINOR_GET(*msg_word);
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO_LOW,
				"target uses HTT version %d.%d; host uses %d.%d",
				soc->tgt_ver.major, soc->tgt_ver.minor,
				HTT_CURRENT_VERSION_MAJOR,
				HTT_CURRENT_VERSION_MINOR);
			if (soc->tgt_ver.major != HTT_CURRENT_VERSION_MAJOR) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_WARN,
					"*** Incompatible host/target HTT versions!");
			}
			/* abort if the target is incompatible with the host */
			qdf_assert(soc->tgt_ver.major ==
				HTT_CURRENT_VERSION_MAJOR);
			if (soc->tgt_ver.minor != HTT_CURRENT_VERSION_MINOR) {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_INFO_LOW,
					"*** Warning: host/target HTT versions"
					" are different, though compatible!");
			}
			break;
		}
	case HTT_T2H_MSG_TYPE_RX_ADDBA:
		{
			uint16_t peer_id;
			uint8_t tid;
			uint8_t win_sz;
			uint16_t status;
			struct dp_peer *peer;

			/*
			 * Update REO Queue Desc with new values
			 */
			peer_id = HTT_RX_ADDBA_PEER_ID_GET(*msg_word);
			tid = HTT_RX_ADDBA_TID_GET(*msg_word);
			win_sz = HTT_RX_ADDBA_WIN_SIZE_GET(*msg_word);
			peer = dp_peer_get_ref_by_id(soc->dp_soc, peer_id,
						     DP_MOD_ID_HTT);

			/*
			 * Window size needs to be incremented by 1
			 * since fw needs to represent a value of 256
			 * using just 8 bits
			 */
			if (peer) {
				status = dp_addba_requestprocess_wifi3(
					(struct cdp_soc_t *)soc->dp_soc,
					peer->mac_addr.raw, peer->vdev->vdev_id,
					0, tid, 0, win_sz + 1, 0xffff);

				/*
				 * If PEER_LOCK_REF_PROTECT enbled dec ref
				 * which is inc by dp_peer_get_ref_by_id
				 */
				dp_peer_unref_delete(peer, DP_MOD_ID_HTT);

				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_INFO,
					FL("PeerID %d BAW %d TID %d stat %d"),
					peer_id, win_sz, tid, status);

			} else {
				QDF_TRACE(QDF_MODULE_ID_TXRX,
					QDF_TRACE_LEVEL_ERROR,
					FL("Peer not found peer id %d"),
					peer_id);
			}
			break;
		}
	case HTT_T2H_MSG_TYPE_EXT_STATS_CONF:
		{
			dp_txrx_fw_stats_handler(soc->dp_soc, htt_t2h_msg);
			break;
		}
	case HTT_T2H_MSG_TYPE_PEER_MAP_V2:
		{
			u_int8_t mac_addr_deswizzle_buf[QDF_MAC_ADDR_SIZE];
			u_int8_t *peer_mac_addr;
			u_int16_t peer_id;
			u_int16_t hw_peer_id;
			u_int8_t vdev_id;
			bool is_wds;
			u_int16_t ast_hash;
			struct dp_ast_flow_override_info ast_flow_info;

			qdf_mem_set(&ast_flow_info, 0,
					    sizeof(struct dp_ast_flow_override_info));

			peer_id = HTT_RX_PEER_MAP_V2_SW_PEER_ID_GET(*msg_word);
			hw_peer_id =
			HTT_RX_PEER_MAP_V2_HW_PEER_ID_GET(*(msg_word + 2));
			vdev_id = HTT_RX_PEER_MAP_V2_VDEV_ID_GET(*msg_word);
			peer_mac_addr =
			htt_t2h_mac_addr_deswizzle((u_int8_t *)(msg_word + 1),
						   &mac_addr_deswizzle_buf[0]);
			is_wds =
			HTT_RX_PEER_MAP_V2_NEXT_HOP_GET(*(msg_word + 3));
			ast_hash =
			HTT_RX_PEER_MAP_V2_AST_HASH_VALUE_GET(*(msg_word + 3));
			/*
			 * Update 4 ast_index per peer, ast valid mask
			 * and TID flow valid mask.
			 * AST valid mask is 3 bit field corresponds to
			 * ast_index[3:1]. ast_index 0 is always valid.
			 */
			ast_flow_info.ast_valid_mask =
			HTT_RX_PEER_MAP_V2_AST_VALID_MASK_GET(*(msg_word + 3));
			ast_flow_info.ast_idx[0] = hw_peer_id;
			ast_flow_info.ast_flow_mask[0] =
			HTT_RX_PEER_MAP_V2_AST_0_FLOW_MASK_GET(*(msg_word + 4));
			ast_flow_info.ast_idx[1] =
			HTT_RX_PEER_MAP_V2_AST_INDEX_1_GET(*(msg_word + 4));
			ast_flow_info.ast_flow_mask[1] =
			HTT_RX_PEER_MAP_V2_AST_1_FLOW_MASK_GET(*(msg_word + 4));
			ast_flow_info.ast_idx[2] =
			HTT_RX_PEER_MAP_V2_AST_INDEX_2_GET(*(msg_word + 5));
			ast_flow_info.ast_flow_mask[2] =
			HTT_RX_PEER_MAP_V2_AST_2_FLOW_MASK_GET(*(msg_word + 4));
			ast_flow_info.ast_idx[3] =
			HTT_RX_PEER_MAP_V2_AST_INDEX_3_GET(*(msg_word + 6));
			ast_flow_info.ast_flow_mask[3] =
			HTT_RX_PEER_MAP_V2_AST_3_FLOW_MASK_GET(*(msg_word + 4));
			/*
			 * TID valid mask is applicable only
			 * for HI and LOW priority flows.
			 * tid_valid_mas is 8 bit field corresponds
			 * to TID[7:0]
			 */
			ast_flow_info.tid_valid_low_pri_mask =
			HTT_RX_PEER_MAP_V2_TID_VALID_LOW_PRI_GET(*(msg_word + 5));
			ast_flow_info.tid_valid_hi_pri_mask =
			HTT_RX_PEER_MAP_V2_TID_VALID_HI_PRI_GET(*(msg_word + 5));

			QDF_TRACE(QDF_MODULE_ID_TXRX,
				  QDF_TRACE_LEVEL_INFO,
				  "HTT_T2H_MSG_TYPE_PEER_MAP msg for peer id %d vdev id %d n",
				  peer_id, vdev_id);

			QDF_TRACE(QDF_MODULE_ID_TXRX,
				  QDF_TRACE_LEVEL_INFO,
				  "ast_idx[0] %d ast_idx[1] %d ast_idx[2] %d ast_idx[3] %d n",
				  ast_flow_info.ast_idx[0],
				  ast_flow_info.ast_idx[1],
				  ast_flow_info.ast_idx[2],
				  ast_flow_info.ast_idx[3]);

			dp_rx_peer_map_handler(soc->dp_soc, peer_id,
					       hw_peer_id, vdev_id,
					       peer_mac_addr, ast_hash,
					       is_wds);

			/*
			 * Update ast indexes for flow override support
			 * Applicable only for non wds peers
			 */
			if (!soc->dp_soc->ast_offload_support)
				dp_peer_ast_index_flow_queue_map_create(
						soc->dp_soc, is_wds,
						peer_id, peer_mac_addr,
						&ast_flow_info);

			break;
		}
	case HTT_T2H_MSG_TYPE_PEER_UNMAP_V2:
		{
			u_int8_t mac_addr_deswizzle_buf[QDF_MAC_ADDR_SIZE];
			u_int8_t *mac_addr;
			u_int16_t peer_id;
			u_int8_t vdev_id;
			u_int8_t is_wds;
			u_int32_t free_wds_count;

			peer_id =
			HTT_RX_PEER_UNMAP_V2_SW_PEER_ID_GET(*msg_word);
			vdev_id = HTT_RX_PEER_UNMAP_V2_VDEV_ID_GET(*msg_word);
			mac_addr =
			htt_t2h_mac_addr_deswizzle((u_int8_t *)(msg_word + 1),
						   &mac_addr_deswizzle_buf[0]);
			is_wds =
			HTT_RX_PEER_UNMAP_V2_NEXT_HOP_GET(*(msg_word + 2));
			free_wds_count =
			HTT_RX_PEER_UNMAP_V2_PEER_WDS_FREE_COUNT_GET(*(msg_word + 4));

			QDF_TRACE(QDF_MODULE_ID_TXRX,
				  QDF_TRACE_LEVEL_INFO,
				  "HTT_T2H_MSG_TYPE_PEER_UNMAP msg for peer id %d vdev id %d n",
				  peer_id, vdev_id);

			dp_rx_peer_unmap_handler(soc->dp_soc, peer_id,
						 vdev_id, mac_addr,
						 is_wds, free_wds_count);
			break;
		}
	case HTT_T2H_MSG_TYPE_RX_DELBA:
		{
			uint16_t peer_id;
			uint8_t tid;
			uint8_t win_sz;
			QDF_STATUS status;

			peer_id = HTT_RX_DELBA_PEER_ID_GET(*msg_word);
			tid = HTT_RX_DELBA_TID_GET(*msg_word);
			win_sz = HTT_RX_DELBA_WIN_SIZE_GET(*msg_word);

			status = dp_rx_delba_ind_handler(
				soc->dp_soc,
				peer_id, tid, win_sz);

			QDF_TRACE(QDF_MODULE_ID_TXRX,
				  QDF_TRACE_LEVEL_INFO,
				  FL("DELBA PeerID %d BAW %d TID %d stat %d"),
				  peer_id, win_sz, tid, status);
			break;
		}
	case HTT_T2H_MSG_TYPE_FSE_CMEM_BASE_SEND:
		{
			uint16_t num_entries;
			uint32_t cmem_ba_lo;
			uint32_t cmem_ba_hi;

			num_entries = HTT_CMEM_BASE_SEND_NUM_ENTRIES_GET(*msg_word);
			cmem_ba_lo = *(msg_word + 1);
			cmem_ba_hi = *(msg_word + 2);

			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_INFO,
				  FL("CMEM FSE num_entries %u CMEM BA LO %x HI %x"),
				  num_entries, cmem_ba_lo, cmem_ba_hi);

			dp_rx_fst_update_cmem_params(soc->dp_soc, num_entries,
						     cmem_ba_lo, cmem_ba_hi);
			break;
		}
	case HTT_T2H_MSG_TYPE_TX_OFFLOAD_DELIVER_IND:
		{
			dp_offload_ind_handler(soc, msg_word);
			break;
		}
	case HTT_T2H_MSG_TYPE_PEER_MAP_V3:
	{
		u_int8_t mac_addr_deswizzle_buf[QDF_MAC_ADDR_SIZE];
		u_int8_t *peer_mac_addr;
		u_int16_t peer_id;
		u_int16_t hw_peer_id;
		u_int8_t vdev_id;
		uint8_t is_wds;
		u_int16_t ast_hash = 0;

		peer_id = HTT_RX_PEER_MAP_V3_SW_PEER_ID_GET(*msg_word);
		vdev_id = HTT_RX_PEER_MAP_V3_VDEV_ID_GET(*msg_word);
		peer_mac_addr =
		htt_t2h_mac_addr_deswizzle((u_int8_t *)(msg_word + 1),
					   &mac_addr_deswizzle_buf[0]);
		hw_peer_id = HTT_RX_PEER_MAP_V3_HW_PEER_ID_GET(*(msg_word + 3));
		ast_hash = HTT_RX_PEER_MAP_V3_CACHE_SET_NUM_GET(*(msg_word + 3));
		is_wds = HTT_RX_PEER_MAP_V3_NEXT_HOP_GET(*(msg_word + 4));

		dp_htt_info("HTT_T2H_MSG_TYPE_PEER_MAP_V3 msg for peer id %d vdev id %d n",
			    peer_id, vdev_id);

		dp_rx_peer_map_handler(soc->dp_soc, peer_id,
				       hw_peer_id, vdev_id,
				       peer_mac_addr, ast_hash,
				       is_wds);

		break;
	}
	case HTT_T2H_MSG_TYPE_MLO_RX_PEER_MAP:
	{
		dp_htt_mlo_peer_map_handler(soc, msg_word);
		break;
	}
	case HTT_T2H_MSG_TYPE_MLO_RX_PEER_UNMAP:
	{
		dp_htt_mlo_peer_unmap_handler(soc, msg_word);
		break;
	}
	case HTT_T2H_MSG_TYPE_MLO_TIMESTAMP_OFFSET_IND:
	{
		dp_rx_mlo_timestamp_ind_handler(soc->dp_soc, msg_word);
		break;
	}
	case HTT_T2H_MSG_TYPE_VDEVS_TXRX_STATS_PERIODIC_IND:
	{
		dp_vdev_txrx_hw_stats_handler(soc, msg_word);
		break;
	}
	default:
		break;
	};

	/* Free the indication buffer */
	if (free_buf)
		qdf_nbuf_free(htt_t2h_msg);
}

/*
 * dp_htt_h2t_full() - Send full handler (called from HTC)
 * @context:	Opaque context (HTT SOC handle)
 * @pkt:	HTC packet
 *
 * Return: enum htc_send_full_action
 */
static enum htc_send_full_action
dp_htt_h2t_full(void *context, HTC_PACKET *pkt)
{
	return HTC_SEND_FULL_KEEP;
}

/*
 * dp_htt_hif_t2h_hp_callback() - HIF callback for high priority T2H messages
 * @context:	Opaque context (HTT SOC handle)
 * @nbuf:	nbuf containing T2H message
 * @pipe_id:	HIF pipe ID
 *
 * Return: QDF_STATUS
 *
 * TODO: Temporary change to bypass HTC connection for this new HIF pipe, which
 * will be used for packet log and other high-priority HTT messages. Proper
 * HTC connection to be added later once required FW changes are available
 */
static QDF_STATUS
dp_htt_hif_t2h_hp_callback (void *context, qdf_nbuf_t nbuf, uint8_t pipe_id)
{
	QDF_STATUS rc = QDF_STATUS_SUCCESS;
	HTC_PACKET htc_pkt;

	qdf_assert_always(pipe_id == DP_HTT_T2H_HP_PIPE);
	qdf_mem_zero(&htc_pkt, sizeof(htc_pkt));
	htc_pkt.Status = QDF_STATUS_SUCCESS;
	htc_pkt.pPktContext = (void *)nbuf;
	dp_htt_t2h_msg_handler(context, &htc_pkt);

	return rc;
}

/*
 * htt_htc_soc_attach() - Register SOC level HTT instance with HTC
 * @htt_soc:	HTT SOC handle
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS
htt_htc_soc_attach(struct htt_soc *soc)
{
	struct htc_service_connect_req connect;
	struct htc_service_connect_resp response;
	QDF_STATUS status;
	struct dp_soc *dpsoc = soc->dp_soc;

	qdf_mem_zero(&connect, sizeof(connect));
	qdf_mem_zero(&response, sizeof(response));

	connect.pMetaData = NULL;
	connect.MetaDataLength = 0;
	connect.EpCallbacks.pContext = soc;
	connect.EpCallbacks.EpTxComplete = dp_htt_h2t_send_complete;
	connect.EpCallbacks.EpTxCompleteMultiple = NULL;
	connect.EpCallbacks.EpRecv = dp_htt_t2h_msg_handler;

	/* rx buffers currently are provided by HIF, not by EpRecvRefill */
	connect.EpCallbacks.EpRecvRefill = NULL;

	/* N/A, fill is done by HIF */
	connect.EpCallbacks.RecvRefillWaterMark = 1;

	connect.EpCallbacks.EpSendFull = dp_htt_h2t_full;
	/*
	 * Specify how deep to let a queue get before htc_send_pkt will
	 * call the EpSendFull function due to excessive send queue depth.
	 */
	connect.MaxSendQueueDepth = DP_HTT_MAX_SEND_QUEUE_DEPTH;

	/* disable flow control for HTT data message service */
	connect.ConnectionFlags |= HTC_CONNECT_FLAGS_DISABLE_CREDIT_FLOW_CTRL;

	/* connect to control service */
	connect.service_id = HTT_DATA_MSG_SVC;

	status = htc_connect_service(soc->htc_soc, &connect, &response);

	if (status != QDF_STATUS_SUCCESS)
		return status;

	soc->htc_endpoint = response.Endpoint;

	hif_save_htc_htt_config_endpoint(dpsoc->hif_handle, soc->htc_endpoint);

	htt_interface_logging_init(&soc->htt_logger_handle, soc->ctrl_psoc);
	dp_hif_update_pipe_callback(soc->dp_soc, (void *)soc,
		dp_htt_hif_t2h_hp_callback, DP_HTT_T2H_HP_PIPE);

	return QDF_STATUS_SUCCESS; /* success */
}

/*
 * htt_soc_initialize() - SOC level HTT initialization
 * @htt_soc: Opaque htt SOC handle
 * @ctrl_psoc: Opaque ctrl SOC handle
 * @htc_soc: SOC level HTC handle
 * @hal_soc: Opaque HAL SOC handle
 * @osdev: QDF device
 *
 * Return: HTT handle on success; NULL on failure
 */
void *
htt_soc_initialize(struct htt_soc *htt_soc,
		   struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
		   HTC_HANDLE htc_soc,
		   hal_soc_handle_t hal_soc_hdl, qdf_device_t osdev)
{
	struct htt_soc *soc = (struct htt_soc *)htt_soc;

	soc->osdev = osdev;
	soc->ctrl_psoc = ctrl_psoc;
	soc->htc_soc = htc_soc;
	soc->hal_soc = hal_soc_hdl;

	if (htt_htc_soc_attach(soc))
		goto fail2;

	return soc;

fail2:
	return NULL;
}

void htt_soc_htc_dealloc(struct htt_soc *htt_handle)
{
	htt_interface_logging_deinit(htt_handle->htt_logger_handle);
	htt_htc_misc_pkt_pool_free(htt_handle);
	htt_htc_pkt_pool_free(htt_handle);
}

/*
 * htt_soc_htc_prealloc() - HTC memory prealloc
 * @htt_soc: SOC level HTT handle
 *
 * Return: QDF_STATUS_SUCCESS on Success or
 * QDF_STATUS_E_NOMEM on allocation failure
 */
QDF_STATUS htt_soc_htc_prealloc(struct htt_soc *soc)
{
	int i;

	soc->htt_htc_pkt_freelist = NULL;
	/* pre-allocate some HTC_PACKET objects */
	for (i = 0; i < HTT_HTC_PKT_POOL_INIT_SIZE; i++) {
		struct dp_htt_htc_pkt_union *pkt;
		pkt = qdf_mem_malloc(sizeof(*pkt));
		if (!pkt)
			return QDF_STATUS_E_NOMEM;

		htt_htc_pkt_free(soc, &pkt->u.pkt);
	}
	return QDF_STATUS_SUCCESS;
}

/*
 * htt_soc_detach() - Free SOC level HTT handle
 * @htt_hdl: HTT SOC handle
 */
void htt_soc_detach(struct htt_soc *htt_hdl)
{
	int i;
	struct htt_soc *htt_handle = (struct htt_soc *)htt_hdl;

	for (i = 0; i < MAX_PDEV_CNT; i++) {
		qdf_mem_free(htt_handle->pdevid_tt[i].umac_ttt);
		qdf_mem_free(htt_handle->pdevid_tt[i].lmac_ttt);
	}

	HTT_TX_MUTEX_DESTROY(&htt_handle->htt_tx_mutex);
	qdf_mem_free(htt_handle);

}

/**
 * dp_h2t_ext_stats_msg_send(): function to contruct HTT message to pass to FW
 * @pdev: DP PDEV handle
 * @stats_type_upload_mask: stats type requested by user
 * @config_param_0: extra configuration parameters
 * @config_param_1: extra configuration parameters
 * @config_param_2: extra configuration parameters
 * @config_param_3: extra configuration parameters
 * @mac_id: mac number
 *
 * return: QDF STATUS
 */
QDF_STATUS dp_h2t_ext_stats_msg_send(struct dp_pdev *pdev,
		uint32_t stats_type_upload_mask, uint32_t config_param_0,
		uint32_t config_param_1, uint32_t config_param_2,
		uint32_t config_param_3, int cookie_val, int cookie_msb,
		uint8_t mac_id)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	uint32_t *msg_word;
	uint8_t pdev_mask = 0;
	uint8_t *htt_logger_bufp;
	int mac_for_pdev;
	int target_pdev_id;
	QDF_STATUS status;

	msg = qdf_nbuf_alloc(
			soc->osdev,
			HTT_MSG_BUF_SIZE(HTT_H2T_EXT_STATS_REQ_MSG_SZ),
			HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);

	if (!msg)
		return QDF_STATUS_E_NOMEM;

	/*TODO:Add support for SOC stats
	 * Bit 0: SOC Stats
	 * Bit 1: Pdev stats for pdev id 0
	 * Bit 2: Pdev stats for pdev id 1
	 * Bit 3: Pdev stats for pdev id 2
	 */
	mac_for_pdev = dp_get_mac_id_for_pdev(mac_id, pdev->pdev_id);
	target_pdev_id =
	dp_get_target_pdev_id_for_host_pdev_id(pdev->soc, mac_for_pdev);

	pdev_mask = 1 << target_pdev_id;

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(msg, HTT_H2T_EXT_STATS_REQ_MSG_SZ) == NULL) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"Failed to expand head for HTT_EXT_STATS");
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}

	dp_htt_tx_stats_info("%pK: cookie <-> %d\n config_param_0 %u\n"
			     "config_param_1 %u\n config_param_2 %u\n"
			     "config_param_4 %u\n -------------",
			     pdev->soc, cookie_val,
			     config_param_0,
			     config_param_1, config_param_2, config_param_3);

	msg_word = (uint32_t *) qdf_nbuf_data(msg);

	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	htt_logger_bufp = (uint8_t *)msg_word;
	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_EXT_STATS_REQ);
	HTT_H2T_EXT_STATS_REQ_PDEV_MASK_SET(*msg_word, pdev_mask);
	HTT_H2T_EXT_STATS_REQ_STATS_TYPE_SET(*msg_word, stats_type_upload_mask);

	/* word 1 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_0);

	/* word 2 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_1);

	/* word 3 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_2);

	/* word 4 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, config_param_3);

	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, 0);

	/* word 5 */
	msg_word++;

	/* word 6 */
	msg_word++;
	*msg_word = 0;
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, cookie_val);

	/* word 7 */
	msg_word++;
	*msg_word = 0;
	/* Currently Using last 2 bits for pdev_id
	 * For future reference, reserving 3 bits in cookie_msb for pdev_id
	 */
	cookie_msb = (cookie_msb | pdev->pdev_id);
	HTT_H2T_EXT_STATS_REQ_CONFIG_PARAM_SET(*msg_word, cookie_msb);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_NOMEM;
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
			dp_htt_h2t_send_complete_free_netbuf,
			qdf_nbuf_data(msg), qdf_nbuf_len(msg),
			soc->htc_endpoint,
			/* tag for FW response msg not guaranteed */
			HTC_TX_PACKET_TAG_RUNTIME_PUT);

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);
	status = DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_EXT_STATS_REQ,
				     htt_logger_bufp);

	if (status != QDF_STATUS_SUCCESS) {
		qdf_nbuf_free(msg);
		htt_htc_pkt_free(soc, pkt);
	}

	return status;
}

#ifdef QCA_VDEV_STATS_HW_OFFLOAD_SUPPORT
#define HTT_VDEV_TXRX_STATS_RESET_BITMASK_L32_MASK 0xFFFFFFFF
#define HTT_VDEV_TXRX_STATS_RESET_BITMASK_U32_MASK 0xFFFFFFFF00000000
#define HTT_VDEV_TXRX_STATS_RESET_BITMASK_U32_SHIFT 32

QDF_STATUS dp_h2t_hw_vdev_stats_config_send(struct dp_soc *dpsoc,
					    uint8_t pdev_id, bool enable,
					    bool reset, uint64_t reset_bitmask)
{
	struct htt_soc *soc = dpsoc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	uint32_t *msg_word;
	uint8_t *htt_logger_bufp;
	QDF_STATUS status;
	int duration;
	uint32_t bitmask;
	int target_pdev_id;

	msg = qdf_nbuf_alloc(
			soc->osdev,
			HTT_MSG_BUF_SIZE(sizeof(struct htt_h2t_vdevs_txrx_stats_cfg)),
			HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, true);

	if (!msg) {
		dp_htt_err("%pK: Fail to allocate "
		"HTT_H2T_HW_VDEV_TXRX_STATS_CFG_MSG_SZ msg buffer", dpsoc);
		return QDF_STATUS_E_NOMEM;
	}

	if (pdev_id != INVALID_PDEV_ID)
		target_pdev_id = DP_SW2HW_MACID(pdev_id);
	else
		target_pdev_id = 0;

	duration =
	wlan_cfg_get_vdev_stats_hw_offload_timer(dpsoc->wlan_cfg_ctx);

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (!qdf_nbuf_put_tail(msg,
			       sizeof(struct htt_h2t_vdevs_txrx_stats_cfg))) {
		dp_htt_err("%pK: Failed to expand head for HTT_HW_VDEV_STATS"
			   , dpsoc);
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}

	msg_word = (uint32_t *)qdf_nbuf_data(msg);

	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	htt_logger_bufp = (uint8_t *)msg_word;
	*msg_word = 0;

	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_VDEVS_TXRX_STATS_CFG);
	HTT_RX_VDEVS_TXRX_STATS_PDEV_ID_SET(*msg_word, target_pdev_id);

	HTT_RX_VDEVS_TXRX_STATS_ENABLE_SET(*msg_word, enable);

	HTT_RX_VDEVS_TXRX_STATS_PERIODIC_INTERVAL_SET(*msg_word,
						      (duration >> 3));

	HTT_RX_VDEVS_TXRX_STATS_RESET_STATS_BITS_SET(*msg_word, reset);

	msg_word++;
	*msg_word = 0;
	bitmask = (reset_bitmask & HTT_VDEV_TXRX_STATS_RESET_BITMASK_L32_MASK);
	*msg_word = bitmask;

	msg_word++;
	*msg_word = 0;
	bitmask =
		((reset_bitmask & HTT_VDEV_TXRX_STATS_RESET_BITMASK_U32_MASK) >>
		 HTT_VDEV_TXRX_STATS_RESET_BITMASK_U32_SHIFT);
	*msg_word = bitmask;

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		dp_htt_err("%pK: Fail to allocate dp_htt_htc_pkt buffer",
			   dpsoc);
		qdf_assert(0);
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_NOMEM;
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
			       dp_htt_h2t_send_complete_free_netbuf,
			       qdf_nbuf_data(msg), qdf_nbuf_len(msg),
			       soc->htc_endpoint,
			       /* tag for no FW response msg */
			       HTC_TX_PACKET_TAG_RUNTIME_PUT);

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);
	status = DP_HTT_SEND_HTC_PKT(soc, pkt,
				     HTT_H2T_MSG_TYPE_VDEVS_TXRX_STATS_CFG,
				     htt_logger_bufp);

	if (status != QDF_STATUS_SUCCESS) {
		qdf_nbuf_free(msg);
		htt_htc_pkt_free(soc, pkt);
	}

	return status;
}
#else
QDF_STATUS dp_h2t_hw_vdev_stats_config_send(struct dp_soc *dpsoc,
					    uint8_t pdev_id, bool enable,
					    bool reset, uint64_t reset_bitmask)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * dp_h2t_3tuple_config_send(): function to contruct 3 tuple configuration
 * HTT message to pass to FW
 * @pdev: DP PDEV handle
 * @tuple_mask: tuple configuration to report 3 tuple hash value in either
 * toeplitz_2_or_4 or flow_id_toeplitz in MSDU START TLV.
 *
 * tuple_mask[1:0]:
 *   00 - Do not report 3 tuple hash value
 *   10 - Report 3 tuple hash value in toeplitz_2_or_4
 *   01 - Report 3 tuple hash value in flow_id_toeplitz
 *   11 - Report 3 tuple hash value in both toeplitz_2_or_4 & flow_id_toeplitz
 *
 * return: QDF STATUS
 */
QDF_STATUS dp_h2t_3tuple_config_send(struct dp_pdev *pdev,
				     uint32_t tuple_mask, uint8_t mac_id)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	uint32_t *msg_word;
	uint8_t *htt_logger_bufp;
	int mac_for_pdev;
	int target_pdev_id;

	msg = qdf_nbuf_alloc(
			soc->osdev,
			HTT_MSG_BUF_SIZE(HTT_3_TUPLE_HASH_CFG_REQ_BYTES),
			HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);

	if (!msg)
		return QDF_STATUS_E_NOMEM;

	mac_for_pdev = dp_get_mac_id_for_pdev(mac_id, pdev->pdev_id);
	target_pdev_id =
	dp_get_target_pdev_id_for_host_pdev_id(pdev->soc, mac_for_pdev);

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (!qdf_nbuf_put_tail(msg, HTT_3_TUPLE_HASH_CFG_REQ_BYTES)) {
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			  "Failed to expand head for HTT_3TUPLE_CONFIG");
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}

	dp_htt_info("%pK: config_param_sent 0x%x for target_pdev %d\n -------------",
		    pdev->soc, tuple_mask, target_pdev_id);

	msg_word = (uint32_t *)qdf_nbuf_data(msg);
	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	htt_logger_bufp = (uint8_t *)msg_word;

	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_3_TUPLE_HASH_CFG);
	HTT_RX_3_TUPLE_HASH_PDEV_ID_SET(*msg_word, target_pdev_id);

	msg_word++;
	*msg_word = 0;
	HTT_H2T_FLOW_ID_TOEPLITZ_FIELD_CONFIG_SET(*msg_word, tuple_mask);
	HTT_H2T_TOEPLITZ_2_OR_4_FIELD_CONFIG_SET(*msg_word, tuple_mask);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_NOMEM;
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
			&pkt->htc_pkt,
			dp_htt_h2t_send_complete_free_netbuf,
			qdf_nbuf_data(msg),
			qdf_nbuf_len(msg),
			soc->htc_endpoint,
			/* tag for no FW response msg */
			HTC_TX_PACKET_TAG_RUNTIME_PUT);

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);
	DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_3_TUPLE_HASH_CFG,
			    htt_logger_bufp);

	return QDF_STATUS_SUCCESS;
}

/* This macro will revert once proper HTT header will define for
 * HTT_H2T_MSG_TYPE_PPDU_STATS_CFG in htt.h file
 * */
#if defined(WDI_EVENT_ENABLE)
/**
 * dp_h2t_cfg_stats_msg_send(): function to construct HTT message to pass to FW
 * @pdev: DP PDEV handle
 * @stats_type_upload_mask: stats type requested by user
 * @mac_id: Mac id number
 *
 * return: QDF STATUS
 */
QDF_STATUS dp_h2t_cfg_stats_msg_send(struct dp_pdev *pdev,
		uint32_t stats_type_upload_mask, uint8_t mac_id)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	uint32_t *msg_word;
	uint8_t pdev_mask;
	QDF_STATUS status;

	msg = qdf_nbuf_alloc(
			soc->osdev,
			HTT_MSG_BUF_SIZE(HTT_H2T_PPDU_STATS_CFG_MSG_SZ),
			HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, true);

	if (!msg) {
		dp_htt_err("%pK: Fail to allocate HTT_H2T_PPDU_STATS_CFG_MSG_SZ msg buffer"
			   , pdev->soc);
		qdf_assert(0);
		return QDF_STATUS_E_NOMEM;
	}

	/*TODO:Add support for SOC stats
	 * Bit 0: SOC Stats
	 * Bit 1: Pdev stats for pdev id 0
	 * Bit 2: Pdev stats for pdev id 1
	 * Bit 3: Pdev stats for pdev id 2
	 */
	pdev_mask = 1 << dp_get_target_pdev_id_for_host_pdev_id(pdev->soc,
								mac_id);

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (qdf_nbuf_put_tail(msg, HTT_H2T_PPDU_STATS_CFG_MSG_SZ) == NULL) {
		dp_htt_err("%pK: Failed to expand head for HTT_CFG_STATS"
			   , pdev->soc);
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}

	msg_word = (uint32_t *) qdf_nbuf_data(msg);

	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_PPDU_STATS_CFG);
	HTT_H2T_PPDU_STATS_CFG_PDEV_MASK_SET(*msg_word, pdev_mask);
	HTT_H2T_PPDU_STATS_CFG_TLV_BITMASK_SET(*msg_word,
			stats_type_upload_mask);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		dp_htt_err("%pK: Fail to allocate dp_htt_htc_pkt buffer", pdev->soc);
		qdf_assert(0);
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_NOMEM;
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
			dp_htt_h2t_send_complete_free_netbuf,
			qdf_nbuf_data(msg), qdf_nbuf_len(msg),
			soc->htc_endpoint,
			/* tag for no FW response msg */
			HTC_TX_PACKET_TAG_RUNTIME_PUT);

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);
	status = DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_PPDU_STATS_CFG,
				     (uint8_t *)msg_word);

	if (status != QDF_STATUS_SUCCESS) {
		qdf_nbuf_free(msg);
		htt_htc_pkt_free(soc, pkt);
	}

	return status;
}

qdf_export_symbol(dp_h2t_cfg_stats_msg_send);
#endif

void
dp_peer_update_inactive_time(struct dp_pdev *pdev, uint32_t tag_type,
			     uint32_t *tag_buf)
{
	struct dp_peer *peer = NULL;
	switch (tag_type) {
	case HTT_STATS_PEER_DETAILS_TAG:
	{
		htt_peer_details_tlv *dp_stats_buf =
			(htt_peer_details_tlv *)tag_buf;

		pdev->fw_stats_peer_id = dp_stats_buf->sw_peer_id;
	}
	break;
	case HTT_STATS_PEER_STATS_CMN_TAG:
	{
		htt_peer_stats_cmn_tlv *dp_stats_buf =
			(htt_peer_stats_cmn_tlv *)tag_buf;

		peer = dp_peer_get_ref_by_id(pdev->soc, pdev->fw_stats_peer_id,
					     DP_MOD_ID_HTT);

		if (peer && !peer->bss_peer) {
			peer->stats.tx.inactive_time =
				dp_stats_buf->inactive_time;
			qdf_event_set(&pdev->fw_peer_stats_event);
		}
		if (peer)
			dp_peer_unref_delete(peer, DP_MOD_ID_HTT);
	}
	break;
	default:
		qdf_err("Invalid tag_type");
	}
}

/**
 * dp_htt_rx_flow_fst_setup(): Send HTT Rx FST setup message to FW
 * @pdev: DP pdev handle
 * @fse_setup_info: FST setup parameters
 *
 * Return: Success when HTT message is sent, error on failure
 */
QDF_STATUS
dp_htt_rx_flow_fst_setup(struct dp_pdev *pdev,
			 struct dp_htt_rx_flow_fst_setup *fse_setup_info)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	u_int32_t *msg_word;
	struct htt_h2t_msg_rx_fse_setup_t *fse_setup;
	uint8_t *htt_logger_bufp;
	u_int32_t *key;
	QDF_STATUS status;

	msg = qdf_nbuf_alloc(
		soc->osdev,
		HTT_MSG_BUF_SIZE(sizeof(struct htt_h2t_msg_rx_fse_setup_t)),
		/* reserve room for the HTC header */
		HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);

	if (!msg)
		return QDF_STATUS_E_NOMEM;

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (!qdf_nbuf_put_tail(msg,
			       sizeof(struct htt_h2t_msg_rx_fse_setup_t))) {
		qdf_err("Failed to expand head for HTT RX_FSE_SETUP msg");
		return QDF_STATUS_E_FAILURE;
	}

	/* fill in the message contents */
	msg_word = (u_int32_t *)qdf_nbuf_data(msg);

	memset(msg_word, 0, sizeof(struct htt_h2t_msg_rx_fse_setup_t));
	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	htt_logger_bufp = (uint8_t *)msg_word;

	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_RX_FSE_SETUP_CFG);

	fse_setup = (struct htt_h2t_msg_rx_fse_setup_t *)msg_word;

	HTT_RX_FSE_SETUP_PDEV_ID_SET(*msg_word, fse_setup_info->pdev_id);

	msg_word++;
	HTT_RX_FSE_SETUP_NUM_REC_SET(*msg_word, fse_setup_info->max_entries);
	HTT_RX_FSE_SETUP_MAX_SEARCH_SET(*msg_word, fse_setup_info->max_search);
	HTT_RX_FSE_SETUP_IP_DA_SA_PREFIX_SET(*msg_word,
					     fse_setup_info->ip_da_sa_prefix);

	msg_word++;
	HTT_RX_FSE_SETUP_BASE_ADDR_LO_SET(*msg_word,
					  fse_setup_info->base_addr_lo);
	msg_word++;
	HTT_RX_FSE_SETUP_BASE_ADDR_HI_SET(*msg_word,
					  fse_setup_info->base_addr_hi);

	key = (u_int32_t *)fse_setup_info->hash_key;
	fse_setup->toeplitz31_0 = *key++;
	fse_setup->toeplitz63_32 = *key++;
	fse_setup->toeplitz95_64 = *key++;
	fse_setup->toeplitz127_96 = *key++;
	fse_setup->toeplitz159_128 = *key++;
	fse_setup->toeplitz191_160 = *key++;
	fse_setup->toeplitz223_192 = *key++;
	fse_setup->toeplitz255_224 = *key++;
	fse_setup->toeplitz287_256 = *key++;
	fse_setup->toeplitz314_288 = *key;

	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz31_0);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz63_32);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz95_64);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz127_96);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz159_128);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz191_160);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz223_192);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz255_224);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_VALUE_SET(*msg_word, fse_setup->toeplitz287_256);
	msg_word++;
	HTT_RX_FSE_SETUP_HASH_314_288_SET(*msg_word,
					  fse_setup->toeplitz314_288);

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_err("Fail to allocate dp_htt_htc_pkt buffer");
		qdf_assert(0);
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_RESOURCES; /* failure */
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
		&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf,
		qdf_nbuf_data(msg),
		qdf_nbuf_len(msg),
		soc->htc_endpoint,
		/* tag for no FW response msg */
		HTC_TX_PACKET_TAG_RUNTIME_PUT);

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);

	status = DP_HTT_SEND_HTC_PKT(soc, pkt,
				     HTT_H2T_MSG_TYPE_RX_FSE_SETUP_CFG,
				     htt_logger_bufp);

	if (status == QDF_STATUS_SUCCESS) {
		dp_info("HTT_H2T RX_FSE_SETUP sent to FW for pdev = %u",
			fse_setup_info->pdev_id);
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_ANY, QDF_TRACE_LEVEL_DEBUG,
				   (void *)fse_setup_info->hash_key,
				   fse_setup_info->hash_key_len);
	} else {
		qdf_nbuf_free(msg);
		htt_htc_pkt_free(soc, pkt);
	}

	return status;
}

/**
 * dp_htt_rx_flow_fse_operation(): Send HTT Flow Search Entry msg to
 * add/del a flow in HW
 * @pdev: DP pdev handle
 * @fse_op_info: Flow entry parameters
 *
 * Return: Success when HTT message is sent, error on failure
 */
QDF_STATUS
dp_htt_rx_flow_fse_operation(struct dp_pdev *pdev,
			     struct dp_htt_rx_flow_fst_operation *fse_op_info)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	u_int32_t *msg_word;
	struct htt_h2t_msg_rx_fse_operation_t *fse_operation;
	uint8_t *htt_logger_bufp;
	QDF_STATUS status;

	msg = qdf_nbuf_alloc(
		soc->osdev,
		HTT_MSG_BUF_SIZE(sizeof(struct htt_h2t_msg_rx_fse_operation_t)),
		/* reserve room for the HTC header */
		HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING, 4, TRUE);
	if (!msg)
		return QDF_STATUS_E_NOMEM;

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (!qdf_nbuf_put_tail(msg,
			       sizeof(struct htt_h2t_msg_rx_fse_operation_t))) {
		qdf_err("Failed to expand head for HTT_RX_FSE_OPERATION msg");
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}

	/* fill in the message contents */
	msg_word = (u_int32_t *)qdf_nbuf_data(msg);

	memset(msg_word, 0, sizeof(struct htt_h2t_msg_rx_fse_operation_t));
	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	htt_logger_bufp = (uint8_t *)msg_word;

	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_RX_FSE_OPERATION_CFG);

	fse_operation = (struct htt_h2t_msg_rx_fse_operation_t *)msg_word;

	HTT_RX_FSE_OPERATION_PDEV_ID_SET(*msg_word, fse_op_info->pdev_id);
	msg_word++;
	HTT_RX_FSE_IPSEC_VALID_SET(*msg_word, false);
	if (fse_op_info->op_code == DP_HTT_FST_CACHE_INVALIDATE_ENTRY) {
		HTT_RX_FSE_OPERATION_SET(*msg_word,
					 HTT_RX_FSE_CACHE_INVALIDATE_ENTRY);
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.src_ip_31_0));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.src_ip_63_32));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.src_ip_95_64));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.src_ip_127_96));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.dest_ip_31_0));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.dest_ip_63_32));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(fse_op_info->rx_flow->flow_tuple_info.dest_ip_95_64));
		msg_word++;
		HTT_RX_FSE_OPERATION_IP_ADDR_SET(
		*msg_word,
		qdf_htonl(
		fse_op_info->rx_flow->flow_tuple_info.dest_ip_127_96));
		msg_word++;
		HTT_RX_FSE_SOURCEPORT_SET(
			*msg_word,
			fse_op_info->rx_flow->flow_tuple_info.src_port);
		HTT_RX_FSE_DESTPORT_SET(
			*msg_word,
			fse_op_info->rx_flow->flow_tuple_info.dest_port);
		msg_word++;
		HTT_RX_FSE_L4_PROTO_SET(
			*msg_word,
			fse_op_info->rx_flow->flow_tuple_info.l4_protocol);
	} else if (fse_op_info->op_code == DP_HTT_FST_CACHE_INVALIDATE_FULL) {
		HTT_RX_FSE_OPERATION_SET(*msg_word,
					 HTT_RX_FSE_CACHE_INVALIDATE_FULL);
	} else if (fse_op_info->op_code == DP_HTT_FST_DISABLE) {
		HTT_RX_FSE_OPERATION_SET(*msg_word, HTT_RX_FSE_DISABLE);
	} else if (fse_op_info->op_code == DP_HTT_FST_ENABLE) {
		HTT_RX_FSE_OPERATION_SET(*msg_word, HTT_RX_FSE_ENABLE);
	}

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_err("Fail to allocate dp_htt_htc_pkt buffer");
		qdf_assert(0);
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_RESOURCES; /* failure */
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(
		&pkt->htc_pkt,
		dp_htt_h2t_send_complete_free_netbuf,
		qdf_nbuf_data(msg),
		qdf_nbuf_len(msg),
		soc->htc_endpoint,
		/* tag for no FW response msg */
		HTC_TX_PACKET_TAG_RUNTIME_PUT);

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);

	status = DP_HTT_SEND_HTC_PKT(soc, pkt,
				     HTT_H2T_MSG_TYPE_RX_FSE_OPERATION_CFG,
				     htt_logger_bufp);

	if (status == QDF_STATUS_SUCCESS) {
		dp_info("HTT_H2T RX_FSE_OPERATION_CFG sent to FW for pdev = %u",
			fse_op_info->pdev_id);
	} else {
		qdf_nbuf_free(msg);
		htt_htc_pkt_free(soc, pkt);
	}

	return status;
}

/**
 * dp_htt_rx_fisa_config(): Send HTT msg to configure FISA
 * @pdev: DP pdev handle
 * @fse_op_info: Flow entry parameters
 *
 * Return: Success when HTT message is sent, error on failure
 */
QDF_STATUS
dp_htt_rx_fisa_config(struct dp_pdev *pdev,
		      struct dp_htt_rx_fisa_cfg *fisa_config)
{
	struct htt_soc *soc = pdev->soc->htt_handle;
	struct dp_htt_htc_pkt *pkt;
	qdf_nbuf_t msg;
	u_int32_t *msg_word;
	struct htt_h2t_msg_type_fisa_config_t *htt_fisa_config;
	uint8_t *htt_logger_bufp;
	uint32_t len;
	QDF_STATUS status;

	len = HTT_MSG_BUF_SIZE(sizeof(struct htt_h2t_msg_type_fisa_config_t));

	msg = qdf_nbuf_alloc(soc->osdev,
			     len,
			     /* reserve room for the HTC header */
			     HTC_HEADER_LEN + HTC_HDR_ALIGNMENT_PADDING,
			     4,
			     TRUE);
	if (!msg)
		return QDF_STATUS_E_NOMEM;

	/*
	 * Set the length of the message.
	 * The contribution from the HTC_HDR_ALIGNMENT_PADDING is added
	 * separately during the below call to qdf_nbuf_push_head.
	 * The contribution from the HTC header is added separately inside HTC.
	 */
	if (!qdf_nbuf_put_tail(msg,
			       sizeof(struct htt_h2t_msg_type_fisa_config_t))) {
		qdf_err("Failed to expand head for HTT_RX_FSE_OPERATION msg");
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_FAILURE;
	}

	/* fill in the message contents */
	msg_word = (u_int32_t *)qdf_nbuf_data(msg);

	memset(msg_word, 0, sizeof(struct htt_h2t_msg_type_fisa_config_t));
	/* rewind beyond alignment pad to get to the HTC header reserved area */
	qdf_nbuf_push_head(msg, HTC_HDR_ALIGNMENT_PADDING);
	htt_logger_bufp = (uint8_t *)msg_word;

	*msg_word = 0;
	HTT_H2T_MSG_TYPE_SET(*msg_word, HTT_H2T_MSG_TYPE_RX_FISA_CFG);

	htt_fisa_config = (struct htt_h2t_msg_type_fisa_config_t *)msg_word;

	HTT_RX_FSE_OPERATION_PDEV_ID_SET(*msg_word, htt_fisa_config->pdev_id);

	msg_word++;
	HTT_RX_FISA_CONFIG_FISA_V2_ENABLE_SET(*msg_word, 1);
	HTT_RX_FISA_CONFIG_FISA_V2_AGGR_LIMIT_SET(*msg_word, 0xf);

	msg_word++;
	htt_fisa_config->fisa_timeout_threshold = fisa_config->fisa_timeout;

	pkt = htt_htc_pkt_alloc(soc);
	if (!pkt) {
		qdf_err("Fail to allocate dp_htt_htc_pkt buffer");
		qdf_assert(0);
		qdf_nbuf_free(msg);
		return QDF_STATUS_E_RESOURCES; /* failure */
	}

	pkt->soc_ctxt = NULL; /* not used during send-done callback */

	SET_HTC_PACKET_INFO_TX(&pkt->htc_pkt,
			       dp_htt_h2t_send_complete_free_netbuf,
			       qdf_nbuf_data(msg),
			       qdf_nbuf_len(msg),
			       soc->htc_endpoint,
			       /* tag for no FW response msg */
			       HTC_TX_PACKET_TAG_RUNTIME_PUT);

	SET_HTC_PACKET_NET_BUF_CONTEXT(&pkt->htc_pkt, msg);

	status = DP_HTT_SEND_HTC_PKT(soc, pkt, HTT_H2T_MSG_TYPE_RX_FISA_CFG,
				     htt_logger_bufp);

	if (status == QDF_STATUS_SUCCESS) {
		dp_info("HTT_H2T_MSG_TYPE_RX_FISA_CFG sent to FW for pdev = %u",
			fisa_config->pdev_id);
	} else {
		qdf_nbuf_free(msg);
		htt_htc_pkt_free(soc, pkt);
	}

	return status;
}

/**
 * dp_bk_pressure_stats_handler(): worker function to print back pressure
 *				   stats
 *
 * @context : argument to work function
 */
static void dp_bk_pressure_stats_handler(void *context)
{
	struct dp_pdev *pdev = (struct dp_pdev *)context;
	struct dp_soc_srngs_state *soc_srngs_state = NULL;
	const char *ring_name;
	int i;
	struct dp_srng_ring_state *ring_state;
	bool empty_flag;

	qdf_spin_lock_bh(&pdev->bkp_stats.list_lock);

	/* Extract only first entry for printing in one work event */
	if (pdev->bkp_stats.queue_depth &&
	    !TAILQ_EMPTY(&pdev->bkp_stats.list)) {
		soc_srngs_state = TAILQ_FIRST(&pdev->bkp_stats.list);
		TAILQ_REMOVE(&pdev->bkp_stats.list, soc_srngs_state,
			     list_elem);
		pdev->bkp_stats.queue_depth--;
	}

	empty_flag = TAILQ_EMPTY(&pdev->bkp_stats.list);
	qdf_spin_unlock_bh(&pdev->bkp_stats.list_lock);

	if (soc_srngs_state) {
		DP_PRINT_STATS("### BKP stats for seq_num %u START ###",
			       soc_srngs_state->seq_num);
		for (i = 0; i < soc_srngs_state->max_ring_id; i++) {
			ring_state = &soc_srngs_state->ring_state[i];
			ring_name = dp_srng_get_str_from_hal_ring_type
						(ring_state->ring_type);
			DP_PRINT_STATS("%s: SW:Head pointer = %d Tail Pointer = %d\n",
				       ring_name,
				       ring_state->sw_head,
				       ring_state->sw_tail);

			DP_PRINT_STATS("%s: HW:Head pointer = %d Tail Pointer = %d\n",
				       ring_name,
				       ring_state->hw_head,
				       ring_state->hw_tail);
		}

		DP_PRINT_STATS("### BKP stats for seq_num %u COMPLETE ###",
			       soc_srngs_state->seq_num);
		qdf_mem_free(soc_srngs_state);
	}
	dp_print_napi_stats(pdev->soc);

	/* Schedule work again if queue is not empty */
	if (!empty_flag)
		qdf_queue_work(0, pdev->bkp_stats.work_queue,
			       &pdev->bkp_stats.work);
}

/*
 * dp_pdev_bkp_stats_detach() - detach resources for back pressure stats
 *				processing
 * @pdev: Datapath PDEV handle
 *
 */
void dp_pdev_bkp_stats_detach(struct dp_pdev *pdev)
{
	struct dp_soc_srngs_state *ring_state, *ring_state_next;

	if (!pdev->bkp_stats.work_queue)
		return;

	qdf_flush_workqueue(0, pdev->bkp_stats.work_queue);
	qdf_destroy_workqueue(0, pdev->bkp_stats.work_queue);
	qdf_flush_work(&pdev->bkp_stats.work);
	qdf_disable_work(&pdev->bkp_stats.work);
	qdf_spin_lock_bh(&pdev->bkp_stats.list_lock);
	TAILQ_FOREACH_SAFE(ring_state, &pdev->bkp_stats.list,
			   list_elem, ring_state_next) {
		TAILQ_REMOVE(&pdev->bkp_stats.list, ring_state,
			     list_elem);
		qdf_mem_free(ring_state);
	}
	qdf_spin_unlock_bh(&pdev->bkp_stats.list_lock);
	qdf_spinlock_destroy(&pdev->bkp_stats.list_lock);
}

/*
 * dp_pdev_bkp_stats_attach() - attach resources for back pressure stats
 *				processing
 * @pdev: Datapath PDEV handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_NOMEM: Error
 */
QDF_STATUS dp_pdev_bkp_stats_attach(struct dp_pdev *pdev)
{
	TAILQ_INIT(&pdev->bkp_stats.list);
	pdev->bkp_stats.seq_num = 0;
	pdev->bkp_stats.queue_depth = 0;

	qdf_create_work(0, &pdev->bkp_stats.work,
			dp_bk_pressure_stats_handler, pdev);

	pdev->bkp_stats.work_queue =
		qdf_alloc_unbound_workqueue("dp_bkp_work_queue");
	if (!pdev->bkp_stats.work_queue)
		goto fail;

	qdf_spinlock_create(&pdev->bkp_stats.list_lock);
	return QDF_STATUS_SUCCESS;

fail:
	dp_htt_alert("BKP stats attach failed");
	qdf_flush_work(&pdev->bkp_stats.work);
	qdf_disable_work(&pdev->bkp_stats.work);
	return QDF_STATUS_E_FAILURE;
}
