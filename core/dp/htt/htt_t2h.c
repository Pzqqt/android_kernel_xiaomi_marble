/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
 * @file htt_t2h.c
 * @brief Provide functions to process target->host HTT messages.
 * @details
 *  This file contains functions related to target->host HTT messages.
 *  There are two categories of functions:
 *  1.  A function that receives a HTT message from HTC, and dispatches it
 *      based on the HTT message type.
 *  2.  functions that provide the info elements from specific HTT messages.
 */
#include <wma.h>
#include <htc_api.h>            /* HTC_PACKET */
#include <htt.h>                /* HTT_T2H_MSG_TYPE, etc. */
#include <qdf_nbuf.h>           /* qdf_nbuf_t */

#include <ol_htt_rx_api.h>
#include <ol_htt_tx_api.h>
#include <ol_txrx_htt_api.h>    /* htt_tx_status */

#include <htt_internal.h>       /* HTT_TX_SCHED, etc. */
#include <pktlog_ac_fmt.h>
#include <wdi_event.h>
#include <ol_htt_tx_api.h>
#include <ol_txrx_peer_find.h>
#include <cdp_txrx_ipa.h>
#include "pktlog_ac.h"

/*--- target->host HTT message dispatch function ----------------------------*/

#ifndef DEBUG_CREDIT
#define DEBUG_CREDIT 0
#endif

#if defined(CONFIG_HL_SUPPORT)



/**
 * htt_rx_frag_set_last_msdu() - set last msdu bit in rx descriptor
 *				 for recieved frames
 * @pdev: Handle (pointer) to HTT pdev.
 * @msg: htt recieved msg
 *
 * Return: None
 */
static inline
void htt_rx_frag_set_last_msdu(struct htt_pdev_t *pdev, qdf_nbuf_t msg)
{
	return;
}
#else

static void htt_rx_frag_set_last_msdu(struct htt_pdev_t *pdev, qdf_nbuf_t msg)
{
	uint32_t *msg_word;
	unsigned num_msdu_bytes;
	qdf_nbuf_t msdu;
	struct htt_host_rx_desc_base *rx_desc;
	int start_idx;
	uint8_t *p_fw_msdu_rx_desc = 0;

	msg_word = (uint32_t *) qdf_nbuf_data(msg);
	num_msdu_bytes = HTT_RX_FRAG_IND_FW_RX_DESC_BYTES_GET(
		*(msg_word + HTT_RX_FRAG_IND_HDR_PREFIX_SIZE32));
	/*
	 * 1 word for the message header,
	 * 1 word to specify the number of MSDU bytes,
	 * 1 word for every 4 MSDU bytes (round up),
	 * 1 word for the MPDU range header
	 */
	pdev->rx_mpdu_range_offset_words = 3 + ((num_msdu_bytes + 3) >> 2);
	pdev->rx_ind_msdu_byte_idx = 0;

	p_fw_msdu_rx_desc = ((uint8_t *) (msg_word) +
			     HTT_ENDIAN_BYTE_IDX_SWAP
				     (HTT_RX_FRAG_IND_FW_DESC_BYTE_OFFSET));

	/*
	 * Fix for EV126710, in which BSOD occurs due to last_msdu bit
	 * not set while the next pointer is deliberately set to NULL
	 * before calling ol_rx_pn_check_base()
	 *
	 * For fragment frames, the HW may not have set the last_msdu bit
	 * in the rx descriptor, but the SW expects this flag to be set,
	 * since each fragment is in a separate MPDU. Thus, set the flag here,
	 * just in case the HW didn't.
	 */
	start_idx = pdev->rx_ring.sw_rd_idx.msdu_payld;
	msdu = pdev->rx_ring.buf.netbufs_ring[start_idx];
	qdf_nbuf_set_pktlen(msdu, HTT_RX_BUF_SIZE);
	qdf_nbuf_unmap(pdev->osdev, msdu, QDF_DMA_FROM_DEVICE);
	rx_desc = htt_rx_desc(msdu);
	*((uint8_t *) &rx_desc->fw_desc.u.val) = *p_fw_msdu_rx_desc;
	rx_desc->msdu_end.last_msdu = 1;
	qdf_nbuf_map(pdev->osdev, msdu, QDF_DMA_FROM_DEVICE);
}
#endif

static uint8_t *htt_t2h_mac_addr_deswizzle(uint8_t *tgt_mac_addr,
					   uint8_t *buffer)
{
#ifdef BIG_ENDIAN_HOST
	/*
	 * The host endianness is opposite of the target endianness.
	 * To make uint32_t elements come out correctly, the target->host
	 * upload has swizzled the bytes in each uint32_t element of the
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

/* Target to host Msg/event  handler  for low priority messages*/
void htt_t2h_lp_msg_handler(void *context, qdf_nbuf_t htt_t2h_msg,
			    bool free_msg_buf)
{
	struct htt_pdev_t *pdev = (struct htt_pdev_t *)context;
	uint32_t *msg_word;
	enum htt_t2h_msg_type msg_type;

	msg_word = (uint32_t *) qdf_nbuf_data(htt_t2h_msg);
	msg_type = HTT_T2H_MSG_TYPE_GET(*msg_word);
	switch (msg_type) {
	case HTT_T2H_MSG_TYPE_VERSION_CONF:
	{
		htc_pm_runtime_put(pdev->htc_pdev);
		pdev->tgt_ver.major = HTT_VER_CONF_MAJOR_GET(*msg_word);
		pdev->tgt_ver.minor = HTT_VER_CONF_MINOR_GET(*msg_word);
		qdf_print
			("target uses HTT version %d.%d; host uses %d.%d",
			pdev->tgt_ver.major, pdev->tgt_ver.minor,
			HTT_CURRENT_VERSION_MAJOR,
			HTT_CURRENT_VERSION_MINOR);
		if (pdev->tgt_ver.major != HTT_CURRENT_VERSION_MAJOR)
			qdf_print
			      ("*** Incompatible host/target HTT versions!");
		/* abort if the target is incompatible with the host */
		qdf_assert(pdev->tgt_ver.major ==
			   HTT_CURRENT_VERSION_MAJOR);
		if (pdev->tgt_ver.minor != HTT_CURRENT_VERSION_MINOR) {
			qdf_print("*** Warning: host/target HTT versions are ");
			qdf_print(" different, though compatible!");
		}
		break;
	}
	case HTT_T2H_MSG_TYPE_RX_FLUSH:
	{
		uint16_t peer_id;
		uint8_t tid;
		int seq_num_start, seq_num_end;
		enum htt_rx_flush_action action;

		peer_id = HTT_RX_FLUSH_PEER_ID_GET(*msg_word);
		tid = HTT_RX_FLUSH_TID_GET(*msg_word);
		seq_num_start =
			HTT_RX_FLUSH_SEQ_NUM_START_GET(*(msg_word + 1));
		seq_num_end =
			HTT_RX_FLUSH_SEQ_NUM_END_GET(*(msg_word + 1));
		action =
			HTT_RX_FLUSH_MPDU_STATUS_GET(*(msg_word + 1)) ==
			1 ? htt_rx_flush_release : htt_rx_flush_discard;
		ol_rx_flush_handler(pdev->txrx_pdev, peer_id, tid,
				    seq_num_start, seq_num_end, action);
		break;
	}
	case HTT_T2H_MSG_TYPE_RX_OFFLOAD_DELIVER_IND:
	{
		int msdu_cnt;
		msdu_cnt =
			HTT_RX_OFFLOAD_DELIVER_IND_MSDU_CNT_GET(*msg_word);
		ol_rx_offload_deliver_ind_handler(pdev->txrx_pdev,
						  htt_t2h_msg,
						  msdu_cnt);
		break;
	}
	case HTT_T2H_MSG_TYPE_RX_FRAG_IND:
	{
		uint16_t peer_id;
		uint8_t tid;

		peer_id = HTT_RX_FRAG_IND_PEER_ID_GET(*msg_word);
		tid = HTT_RX_FRAG_IND_EXT_TID_GET(*msg_word);
		htt_rx_frag_set_last_msdu(pdev, htt_t2h_msg);

		ol_rx_frag_indication_handler(pdev->txrx_pdev,
					      htt_t2h_msg,
					      peer_id, tid);
		break;
	}
	case HTT_T2H_MSG_TYPE_RX_ADDBA:
	{
		uint16_t peer_id;
		uint8_t tid;
		uint8_t win_sz;
		uint16_t start_seq_num;

		/*
		 * FOR NOW, the host doesn't need to know the initial
		 * sequence number for rx aggregation.
		 * Thus, any value will do - specify 0.
		 */
		start_seq_num = 0;
		peer_id = HTT_RX_ADDBA_PEER_ID_GET(*msg_word);
		tid = HTT_RX_ADDBA_TID_GET(*msg_word);
		win_sz = HTT_RX_ADDBA_WIN_SIZE_GET(*msg_word);
		ol_rx_addba_handler(pdev->txrx_pdev, peer_id, tid,
				    win_sz, start_seq_num,
				    0 /* success */);
		break;
	}
	case HTT_T2H_MSG_TYPE_RX_DELBA:
	{
		uint16_t peer_id;
		uint8_t tid;

		peer_id = HTT_RX_DELBA_PEER_ID_GET(*msg_word);
		tid = HTT_RX_DELBA_TID_GET(*msg_word);
		ol_rx_delba_handler(pdev->txrx_pdev, peer_id, tid);
		break;
	}
	case HTT_T2H_MSG_TYPE_PEER_MAP:
	{
		uint8_t mac_addr_deswizzle_buf[HTT_MAC_ADDR_LEN];
		uint8_t *peer_mac_addr;
		uint16_t peer_id;
		uint8_t vdev_id;

		peer_id = HTT_RX_PEER_MAP_PEER_ID_GET(*msg_word);
		vdev_id = HTT_RX_PEER_MAP_VDEV_ID_GET(*msg_word);
		peer_mac_addr = htt_t2h_mac_addr_deswizzle(
			(uint8_t *) (msg_word + 1),
			&mac_addr_deswizzle_buf[0]);

		ol_rx_peer_map_handler(pdev->txrx_pdev, peer_id,
				       vdev_id, peer_mac_addr,
				       1 /*can tx */);
		break;
	}
	case HTT_T2H_MSG_TYPE_PEER_UNMAP:
	{
		uint16_t peer_id;
		peer_id = HTT_RX_PEER_UNMAP_PEER_ID_GET(*msg_word);

		ol_rx_peer_unmap_handler(pdev->txrx_pdev, peer_id);
		break;
	}
	case HTT_T2H_MSG_TYPE_SEC_IND:
	{
		uint16_t peer_id;
		enum htt_sec_type sec_type;
		int is_unicast;

		peer_id = HTT_SEC_IND_PEER_ID_GET(*msg_word);
		sec_type = HTT_SEC_IND_SEC_TYPE_GET(*msg_word);
		is_unicast = HTT_SEC_IND_UNICAST_GET(*msg_word);
		msg_word++;   /* point to the first part of the Michael key */
		ol_rx_sec_ind_handler(pdev->txrx_pdev, peer_id,
				      sec_type, is_unicast, msg_word,
				      msg_word + 2);
		break;
	}
	case HTT_T2H_MSG_TYPE_MGMT_TX_COMPL_IND:
	{
		struct htt_mgmt_tx_compl_ind *compl_msg;
		int32_t credit_delta = 1;

		compl_msg =
			(struct htt_mgmt_tx_compl_ind *)(msg_word + 1);

		if (pdev->cfg.is_high_latency) {
			if (!pdev->cfg.default_tx_comp_req) {
				qdf_atomic_add(credit_delta,
					       &pdev->htt_tx_credit.
								target_delta);
				credit_delta = htt_tx_credit_update(pdev);
			}
			if (credit_delta)
				ol_tx_target_credit_update(
						pdev->txrx_pdev, credit_delta);
		}
		ol_tx_desc_update_group_credit(
			pdev->txrx_pdev, compl_msg->desc_id, 1,
			0, compl_msg->status);

		if (!ol_tx_get_is_mgmt_over_wmi_enabled()) {
			ol_tx_single_completion_handler(pdev->txrx_pdev,
							compl_msg->status,
							compl_msg->desc_id);
			htc_pm_runtime_put(pdev->htc_pdev);
			HTT_TX_SCHED(pdev);
		} else {
			qdf_print("Ignoring HTT_T2H_MSG_TYPE_MGMT_TX_COMPL_IND indication");
		}
		break;
	}
	case HTT_T2H_MSG_TYPE_STATS_CONF:
	{
		uint64_t cookie;
		uint8_t *stats_info_list;

		cookie = *(msg_word + 1);
		cookie |= ((uint64_t) (*(msg_word + 2))) << 32;

		stats_info_list = (uint8_t *) (msg_word + 3);
		htc_pm_runtime_put(pdev->htc_pdev);
		ol_txrx_fw_stats_handler(pdev->txrx_pdev, cookie,
					 stats_info_list);
		break;
	}
#ifndef REMOVE_PKT_LOG
	case HTT_T2H_MSG_TYPE_PKTLOG:
	{
		pktlog_process_fw_msg(msg_word + 1);
		break;
	}
#endif
	case HTT_T2H_MSG_TYPE_TX_CREDIT_UPDATE_IND:
	{
		uint32_t htt_credit_delta_abs;
		int32_t htt_credit_delta;
		int sign;

		htt_credit_delta_abs =
			HTT_TX_CREDIT_DELTA_ABS_GET(*msg_word);
		sign = HTT_TX_CREDIT_SIGN_BIT_GET(*msg_word) ? -1 : 1;
		htt_credit_delta = sign * htt_credit_delta_abs;

		if (pdev->cfg.is_high_latency &&
		    !pdev->cfg.default_tx_comp_req) {
			qdf_atomic_add(htt_credit_delta,
				       &pdev->htt_tx_credit.target_delta);
			htt_credit_delta = htt_tx_credit_update(pdev);
		}

		htt_tx_group_credit_process(pdev, msg_word);
		ol_tx_credit_completion_handler(pdev->txrx_pdev,
						htt_credit_delta);
		break;
	}

	case HTT_T2H_MSG_TYPE_WDI_IPA_OP_RESPONSE:
	{
		uint8_t op_code;
		uint16_t len;
		uint8_t *op_msg_buffer;
		uint8_t *msg_start_ptr;

		htc_pm_runtime_put(pdev->htc_pdev);
		msg_start_ptr = (uint8_t *) msg_word;
		op_code =
			HTT_WDI_IPA_OP_RESPONSE_OP_CODE_GET(*msg_word);
		msg_word++;
		len = HTT_WDI_IPA_OP_RESPONSE_RSP_LEN_GET(*msg_word);

		op_msg_buffer =
			qdf_mem_malloc(sizeof
				       (struct htt_wdi_ipa_op_response_t) +
				       len);
		if (!op_msg_buffer) {
			qdf_print("OPCODE messsage buffer alloc fail");
			break;
		}
		qdf_mem_copy(op_msg_buffer,
			     msg_start_ptr,
			     sizeof(struct htt_wdi_ipa_op_response_t) +
			     len);
		ol_txrx_ipa_uc_op_response(pdev->txrx_pdev,
					   op_msg_buffer);
		break;
	}

	case HTT_T2H_MSG_TYPE_FLOW_POOL_MAP:
	{
		uint8_t num_flows;
		struct htt_flow_pool_map_payload_t *pool_map_payoad;

		num_flows = HTT_FLOW_POOL_MAP_NUM_FLOWS_GET(*msg_word);

		msg_word++;
		while (num_flows) {
			pool_map_payoad = (struct htt_flow_pool_map_payload_t *)
								msg_word;
			ol_tx_flow_pool_map_handler(pool_map_payoad->flow_id,
					pool_map_payoad->flow_type,
					pool_map_payoad->flow_pool_id,
					pool_map_payoad->flow_pool_size);

			msg_word += (HTT_FLOW_POOL_MAP_PAYLOAD_SZ /
						 HTT_FLOW_POOL_MAP_HEADER_SZ);
			num_flows--;
		}
		break;
	}

	case HTT_T2H_MSG_TYPE_FLOW_POOL_UNMAP:
	{
		struct htt_flow_pool_unmap_t *pool_numap_payload;

		pool_numap_payload = (struct htt_flow_pool_unmap_t *)msg_word;
		ol_tx_flow_pool_unmap_handler(pool_numap_payload->flow_id,
					pool_numap_payload->flow_type,
					pool_numap_payload->flow_pool_id);
		break;
	}

	case HTT_T2H_MSG_TYPE_RX_OFLD_PKT_ERR:
	{
		switch (HTT_RX_OFLD_PKT_ERR_MSG_SUB_TYPE_GET(*msg_word)) {
		case HTT_RX_OFLD_PKT_ERR_TYPE_MIC_ERR:
		{
			struct ol_error_info err_info;
			struct ol_txrx_vdev_t *vdev;
			struct ol_txrx_peer_t *peer;
			uint16_t peer_id =
				 HTT_RX_OFLD_PKT_ERR_MIC_ERR_PEER_ID_GET
				(*(msg_word + 1));

			peer = ol_txrx_peer_find_by_id(pdev->txrx_pdev,
				 peer_id);
			if (!peer) {
				qdf_print("%s: invalid peer id %d\n",
					 __func__, peer_id);
				qdf_assert(0);
				break;
			}
			vdev = peer->vdev;
			err_info.u.mic_err.vdev_id = vdev->vdev_id;
			err_info.u.mic_err.key_id =
				HTT_RX_OFLD_PKT_ERR_MIC_ERR_KEYID_GET
				(*(msg_word + 1));
			qdf_mem_copy(err_info.u.mic_err.da,
				 (uint8_t *)(msg_word + 2),
				 OL_TXRX_MAC_ADDR_LEN);
			qdf_mem_copy(err_info.u.mic_err.sa,
				 (uint8_t *)(msg_word + 4),
				 OL_TXRX_MAC_ADDR_LEN);
			qdf_mem_copy(&err_info.u.mic_err.pn,
				 (uint8_t *)(msg_word + 6), 6);
			qdf_mem_copy(err_info.u.mic_err.ta,
				 peer->mac_addr.raw, OL_TXRX_MAC_ADDR_LEN);

			wma_indicate_err(OL_RX_ERR_TKIP_MIC, &err_info);
			break;
		}
		default:
		{
			qdf_print("%s: unhandled error type %d\n",
			 __func__,
			 HTT_RX_OFLD_PKT_ERR_MSG_SUB_TYPE_GET(*msg_word));
		break;
		}
		}
	}

	default:
		break;
	};
	/* Free the indication buffer */
	if (free_msg_buf)
		qdf_nbuf_free(htt_t2h_msg);
}

/* Generic Target to host Msg/event  handler  for low priority messages
   Low priority message are handler in a different handler called from
   this function . So that the most likely succes path like Rx and
   Tx comp   has little code   foot print
 */
void htt_t2h_msg_handler(void *context, HTC_PACKET *pkt)
{
	struct htt_pdev_t *pdev = (struct htt_pdev_t *)context;
	qdf_nbuf_t htt_t2h_msg = (qdf_nbuf_t) pkt->pPktContext;
	uint32_t *msg_word;
	enum htt_t2h_msg_type msg_type;

	/* check for successful message reception */
	if (pkt->Status != A_OK) {
		if (pkt->Status != A_ECANCELED)
			pdev->stats.htc_err_cnt++;
		qdf_nbuf_free(htt_t2h_msg);
		return;
	}
#ifdef HTT_RX_RESTORE
	if (qdf_unlikely(pdev->rx_ring.rx_reset)) {
		qdf_print("rx restore ..\n");
		qdf_nbuf_free(htt_t2h_msg);
		return;
	}
#endif

	/* confirm alignment */
	HTT_ASSERT3((((unsigned long)qdf_nbuf_data(htt_t2h_msg)) & 0x3) == 0);

	msg_word = (uint32_t *) qdf_nbuf_data(htt_t2h_msg);
	msg_type = HTT_T2H_MSG_TYPE_GET(*msg_word);

#if defined(HELIUMPLUS_DEBUG)
	qdf_print("%s %d: msg_word 0x%x msg_type %d",
		  __func__, __LINE__, *msg_word, msg_type);
#endif

	switch (msg_type) {
	case HTT_T2H_MSG_TYPE_RX_IND:
	{
		unsigned num_mpdu_ranges;
		unsigned num_msdu_bytes;
		uint16_t peer_id;
		uint8_t tid;

		if (qdf_unlikely(pdev->cfg.is_full_reorder_offload)) {
			qdf_print("HTT_T2H_MSG_TYPE_RX_IND not supported ");
			qdf_print("with full reorder offload\n");
			break;
		}
		peer_id = HTT_RX_IND_PEER_ID_GET(*msg_word);
		tid = HTT_RX_IND_EXT_TID_GET(*msg_word);

		if (tid >= OL_TXRX_NUM_EXT_TIDS) {
			qdf_print("HTT_T2H_MSG_TYPE_RX_IND, invalid tid %d\n",
				tid);
			break;
		}
		num_msdu_bytes =
			HTT_RX_IND_FW_RX_DESC_BYTES_GET(
				*(msg_word + 2 + HTT_RX_PPDU_DESC_SIZE32));
		/*
		 * 1 word for the message header,
		 * HTT_RX_PPDU_DESC_SIZE32 words for the FW rx PPDU desc
		 * 1 word to specify the number of MSDU bytes,
		 * 1 word for every 4 MSDU bytes (round up),
		 * 1 word for the MPDU range header
		 */
		pdev->rx_mpdu_range_offset_words =
			(HTT_RX_IND_HDR_BYTES + num_msdu_bytes + 3) >> 2;
		num_mpdu_ranges =
			HTT_RX_IND_NUM_MPDU_RANGES_GET(*(msg_word + 1));
		pdev->rx_ind_msdu_byte_idx = 0;

		ol_rx_indication_handler(pdev->txrx_pdev,
					 htt_t2h_msg, peer_id,
					 tid, num_mpdu_ranges);

		if (pdev->cfg.is_high_latency)
			return;

		break;
	}
	case HTT_T2H_MSG_TYPE_TX_COMPL_IND:
	{
		int num_msdus;
		enum htt_tx_status status;

		/* status - no enum translation needed */
		status = HTT_TX_COMPL_IND_STATUS_GET(*msg_word);
		num_msdus = HTT_TX_COMPL_IND_NUM_GET(*msg_word);
		if (num_msdus & 0x1) {
			struct htt_tx_compl_ind_base *compl =
				(void *)msg_word;

			/*
			 * Host CPU endianness can be different from FW CPU.
			 * This can result in even and odd MSDU IDs being
			 * switched. If this happens, copy the switched final
			 * odd MSDU ID from location payload[size], to
			 * location payload[size-1], where the message
			 * handler function expects to find it
			 */
			if (compl->payload[num_msdus] !=
			    HTT_TX_COMPL_INV_MSDU_ID) {
				compl->payload[num_msdus - 1] =
					compl->payload[num_msdus];
			}
		}

		if (pdev->cfg.is_high_latency) {
			if (!pdev->cfg.default_tx_comp_req) {
				int credit_delta;

				qdf_atomic_add(num_msdus,
					       &pdev->htt_tx_credit.
								target_delta);
				credit_delta = htt_tx_credit_update(pdev);

				if (credit_delta) {
					ol_tx_target_credit_update(
							pdev->txrx_pdev,
							credit_delta);
				}
			} else {
				ol_tx_target_credit_update(pdev->txrx_pdev,
							   num_msdus);
			}
		}

		ol_tx_completion_handler(pdev->txrx_pdev, num_msdus,
					 status, msg_word + 1);
		HTT_TX_SCHED(pdev);
		break;
	}
	case HTT_T2H_MSG_TYPE_RX_PN_IND:
	{
		uint16_t peer_id;
		uint8_t tid, pn_ie_cnt, *pn_ie = NULL;
		int seq_num_start, seq_num_end;

		/*First dword */
		peer_id = HTT_RX_PN_IND_PEER_ID_GET(*msg_word);
		tid = HTT_RX_PN_IND_EXT_TID_GET(*msg_word);

		msg_word++;
		/*Second dword */
		seq_num_start =
			HTT_RX_PN_IND_SEQ_NUM_START_GET(*msg_word);
		seq_num_end = HTT_RX_PN_IND_SEQ_NUM_END_GET(*msg_word);
		pn_ie_cnt = HTT_RX_PN_IND_PN_IE_CNT_GET(*msg_word);

		msg_word++;
		/*Third dword */
		if (pn_ie_cnt)
			pn_ie = (uint8_t *) msg_word;

		ol_rx_pn_ind_handler(pdev->txrx_pdev, peer_id, tid,
				     seq_num_start, seq_num_end,
				     pn_ie_cnt, pn_ie);

		break;
	}
	case HTT_T2H_MSG_TYPE_TX_INSPECT_IND:
	{
		int num_msdus;

		num_msdus = HTT_TX_COMPL_IND_NUM_GET(*msg_word);
		if (num_msdus & 0x1) {
			struct htt_tx_compl_ind_base *compl =
				(void *)msg_word;

			/*
			 * Host CPU endianness can be different from FW CPU.
			 * This can result in even and odd MSDU IDs being
			 * switched. If this happens, copy the switched final
			 * odd MSDU ID from location payload[size], to
			 * location payload[size-1], where the message handler
			 * function expects to find it
			 */
			if (compl->payload[num_msdus] !=
			    HTT_TX_COMPL_INV_MSDU_ID) {
				compl->payload[num_msdus - 1] =
					compl->payload[num_msdus];
			}
		}
		ol_tx_inspect_handler(pdev->txrx_pdev, num_msdus,
				      msg_word + 1);
		HTT_TX_SCHED(pdev);
		break;
	}
	case HTT_T2H_MSG_TYPE_RX_IN_ORD_PADDR_IND:
	{
		uint16_t peer_id;
		uint8_t tid;
		uint8_t offload_ind, frag_ind;

		if (qdf_unlikely(!pdev->cfg.is_full_reorder_offload)) {
			qdf_print("HTT_T2H_MSG_TYPE_RX_IN_ORD_PADDR_IND not ");
			qdf_print("supported when full reorder offload is ");
			qdf_print("disabled in the configuration.\n");
			break;
		}

		if (qdf_unlikely(pdev->cfg.is_high_latency)) {
			qdf_print("HTT_T2H_MSG_TYPE_RX_IN_ORD_PADDR_IND ");
			qdf_print("not supported on high latency.\n");
			break;
		}

		peer_id = HTT_RX_IN_ORD_PADDR_IND_PEER_ID_GET(*msg_word);
		tid = HTT_RX_IN_ORD_PADDR_IND_EXT_TID_GET(*msg_word);
		offload_ind = HTT_RX_IN_ORD_PADDR_IND_OFFLOAD_GET(*msg_word);
		frag_ind = HTT_RX_IN_ORD_PADDR_IND_FRAG_GET(*msg_word);

#if defined(HELIUMPLUS_DEBUG)
		qdf_print("%s %d: peerid %d tid %d offloadind %d fragind %d\n",
			  __func__, __LINE__, peer_id, tid, offload_ind,
			  frag_ind);
#endif
		if (qdf_unlikely(frag_ind)) {
			ol_rx_frag_indication_handler(pdev->txrx_pdev,
						      htt_t2h_msg,
						      peer_id, tid);
			break;
		}

		ol_rx_in_order_indication_handler(pdev->txrx_pdev,
						  htt_t2h_msg, peer_id,
						  tid, offload_ind);
		break;
	}

	default:
		htt_t2h_lp_msg_handler(context, htt_t2h_msg, true);
		return;

	};

	/* Free the indication buffer */
	qdf_nbuf_free(htt_t2h_msg);
}

#ifdef WLAN_FEATURE_FASTPATH
#define HTT_T2H_MSG_BUF_REINIT(_buf, dev)				\
	do {								\
		QDF_NBUF_CB_PADDR(_buf) -= (HTC_HEADER_LEN +		\
					HTC_HDR_ALIGNMENT_PADDING);	\
		qdf_nbuf_init_fast((_buf));				\
		qdf_mem_dma_sync_single_for_device(dev,			\
					(QDF_NBUF_CB_PADDR(_buf)),	\
					(skb_end_pointer(_buf) -	\
					(_buf)->data) ,			\
					PCI_DMA_FROMDEVICE);		\
	} while (0)

/**
 * htt_t2h_msg_handler_fast() -  Fastpath specific message handler
 * @context: HTT context
 * @cmpl_msdus: netbuf completions
 * @num_cmpls: number of completions to be handled
 *
 * Return: None
 */
void htt_t2h_msg_handler_fast(void *context, qdf_nbuf_t *cmpl_msdus,
			      uint32_t num_cmpls)
{
	struct htt_pdev_t *pdev = (struct htt_pdev_t *)context;
	qdf_nbuf_t htt_t2h_msg;
	uint32_t *msg_word;
	uint32_t i;
	enum htt_t2h_msg_type msg_type;
	uint32_t msg_len;

	for (i = 0; i < num_cmpls; i++) {
		htt_t2h_msg = cmpl_msdus[i];
		msg_len = qdf_nbuf_len(htt_t2h_msg);

		/*
		 * Move the data pointer to point to HTT header
		 * past the HTC header + HTC header alignment padding
		 */
		qdf_nbuf_pull_head(htt_t2h_msg, HTC_HEADER_LEN +
				   HTC_HDR_ALIGNMENT_PADDING);

		/* confirm alignment */
		HTT_ASSERT3((((unsigned long) qdf_nbuf_data(htt_t2h_msg)) & 0x3)
			    == 0);

		msg_word = (u_int32_t *) qdf_nbuf_data(htt_t2h_msg);
		msg_type = HTT_T2H_MSG_TYPE_GET(*msg_word);

		switch (msg_type) {
		case HTT_T2H_MSG_TYPE_RX_IND:
		{
			unsigned int num_mpdu_ranges;
			unsigned int num_msdu_bytes;
			u_int16_t peer_id;
			u_int8_t tid;

			peer_id = HTT_RX_IND_PEER_ID_GET(*msg_word);
			tid = HTT_RX_IND_EXT_TID_GET(*msg_word);

			num_msdu_bytes =
				HTT_RX_IND_FW_RX_DESC_BYTES_GET(
				*(msg_word + 2 +
				  HTT_RX_PPDU_DESC_SIZE32));
			/*
			 * 1 word for the message header,
			 * HTT_RX_PPDU_DESC_SIZE32 words for the FW
			 * rx PPDU desc.
			 * 1 word to specify the number of MSDU bytes,
			 * 1 word for every 4 MSDU bytes (round up),
			 * 1 word for the MPDU range header
			 */
			pdev->rx_mpdu_range_offset_words =
				(HTT_RX_IND_HDR_BYTES + num_msdu_bytes + 3) >>
				2;
			num_mpdu_ranges =
				HTT_RX_IND_NUM_MPDU_RANGES_GET(*(msg_word
								 + 1));
			pdev->rx_ind_msdu_byte_idx = 0;
			ol_rx_indication_handler(pdev->txrx_pdev, htt_t2h_msg,
						 peer_id, tid, num_mpdu_ranges);
			break;
		}
		case HTT_T2H_MSG_TYPE_TX_COMPL_IND:
		{
			int num_msdus;
			enum htt_tx_status status;

			/* status - no enum translation needed */
			status = HTT_TX_COMPL_IND_STATUS_GET(*msg_word);
			num_msdus = HTT_TX_COMPL_IND_NUM_GET(*msg_word);
			if (num_msdus & 0x1) {
				struct htt_tx_compl_ind_base *compl =
					(void *)msg_word;

				/*
				 * Host CPU endianness can be different
				 * from FW CPU. This can result in even
				 * and odd MSDU IDs being switched. If
				 * this happens, copy the switched final
				 * odd MSDU ID from location
				 * payload[size], to location
				 * payload[size-1],where the message
				 * handler function expects to find it
				 */
				if (compl->payload[num_msdus] !=
				    HTT_TX_COMPL_INV_MSDU_ID) {
					compl->payload[num_msdus - 1] =
						compl->payload[num_msdus];
				}
			}
			ol_tx_completion_handler(pdev->txrx_pdev, num_msdus,
						 status, msg_word + 1);

			break;
		}
		case HTT_T2H_MSG_TYPE_RX_PN_IND:
		{
			u_int16_t peer_id;
			u_int8_t tid, pn_ie_cnt, *pn_ie = NULL;
			int seq_num_start, seq_num_end;

			/*First dword */
			peer_id = HTT_RX_PN_IND_PEER_ID_GET(*msg_word);
			tid = HTT_RX_PN_IND_EXT_TID_GET(*msg_word);

			msg_word++;
			/*Second dword */
			seq_num_start =
				HTT_RX_PN_IND_SEQ_NUM_START_GET(*msg_word);
			seq_num_end =
				HTT_RX_PN_IND_SEQ_NUM_END_GET(*msg_word);
			pn_ie_cnt =
				HTT_RX_PN_IND_PN_IE_CNT_GET(*msg_word);

			msg_word++;
			/*Third dword*/
			if (pn_ie_cnt)
				pn_ie = (u_int8_t *)msg_word;

			ol_rx_pn_ind_handler(pdev->txrx_pdev, peer_id, tid,
				seq_num_start, seq_num_end, pn_ie_cnt, pn_ie);

			break;
		}
		case HTT_T2H_MSG_TYPE_TX_INSPECT_IND:
		{
			int num_msdus;

			num_msdus = HTT_TX_COMPL_IND_NUM_GET(*msg_word);
			if (num_msdus & 0x1) {
				struct htt_tx_compl_ind_base *compl =
					(void *)msg_word;

				/*
				 * Host CPU endianness can be different
				 * from FW CPU. This * can result in
				 * even and odd MSDU IDs being switched.
				 * If this happens, copy the switched
				 * final odd MSDU ID from location
				 * payload[size], to location
				 * payload[size-1], where the message
				 * handler function expects to find it
				 */
				if (compl->payload[num_msdus] !=
				    HTT_TX_COMPL_INV_MSDU_ID) {
					compl->payload[num_msdus - 1] =
					compl->payload[num_msdus];
				}
			}
			ol_tx_inspect_handler(pdev->txrx_pdev,
					      num_msdus, msg_word + 1);
			break;
		}
		case HTT_T2H_MSG_TYPE_RX_IN_ORD_PADDR_IND:
		{
			u_int16_t peer_id;
			u_int8_t tid;
			u_int8_t offload_ind, frag_ind;

			if (qdf_unlikely(
				  !pdev->cfg.is_full_reorder_offload)) {
				qdf_print("HTT_T2H_MSG_TYPE_RX_IN_ORD_PADDR_IND not supported when full reorder offload is disabled\n");
				break;
			}

			if (qdf_unlikely(
				pdev->txrx_pdev->cfg.is_high_latency)) {
				qdf_print("HTT_T2H_MSG_TYPE_RX_IN_ORD_PADDR_IND not supported on high latency\n");
				break;
			}

			peer_id = HTT_RX_IN_ORD_PADDR_IND_PEER_ID_GET(
							*msg_word);
			tid = HTT_RX_IN_ORD_PADDR_IND_EXT_TID_GET(
							*msg_word);
			offload_ind =
				HTT_RX_IN_ORD_PADDR_IND_OFFLOAD_GET(
							*msg_word);
			frag_ind = HTT_RX_IN_ORD_PADDR_IND_FRAG_GET(
							*msg_word);

			if (qdf_unlikely(frag_ind)) {
				ol_rx_frag_indication_handler(
				pdev->txrx_pdev, htt_t2h_msg, peer_id,
				tid);
				break;
			}

			ol_rx_in_order_indication_handler(
					pdev->txrx_pdev, htt_t2h_msg,
					peer_id, tid, offload_ind);
			break;
		}
		default:
			htt_t2h_lp_msg_handler(context, htt_t2h_msg, false);
			break;
		};

		/* Re-initialize the indication buffer */
		HTT_T2H_MSG_BUF_REINIT(htt_t2h_msg, pdev->osdev);
		qdf_nbuf_set_pktlen(htt_t2h_msg, 0);
	}
}
#endif /* WLAN_FEATURE_FASTPATH */

/*--- target->host HTT message Info Element access methods ------------------*/

/*--- tx completion message ---*/

uint16_t htt_tx_compl_desc_id(void *iterator, int num)
{
	/*
	 * The MSDU IDs are packed , 2 per 32-bit word.
	 * Iterate on them as an array of 16-bit elements.
	 * This will work fine if the host endianness matches
	 * the target endianness.
	 * If the host endianness is opposite of the target's,
	 * this iterator will produce descriptor IDs in a different
	 * order than the target inserted them into the message -
	 * if the target puts in [0, 1, 2, 3, ...] the host will
	 * put out [1, 0, 3, 2, ...].
	 * This is fine, except for the last ID if there are an
	 * odd number of IDs.  But the TX_COMPL_IND handling code
	 * in the htt_t2h_msg_handler already added a duplicate
	 * of the final ID, if there were an odd number of IDs,
	 * so this function can safely treat the IDs as an array
	 * of 16-bit elements.
	 */
	return *(((uint16_t *) iterator) + num);
}

/*--- rx indication message ---*/

int htt_rx_ind_flush(htt_pdev_handle pdev, qdf_nbuf_t rx_ind_msg)
{
	uint32_t *msg_word;

	msg_word = (uint32_t *) qdf_nbuf_data(rx_ind_msg);
	return HTT_RX_IND_FLUSH_VALID_GET(*msg_word);
}

void
htt_rx_ind_flush_seq_num_range(htt_pdev_handle pdev,
			       qdf_nbuf_t rx_ind_msg,
			       unsigned *seq_num_start, unsigned *seq_num_end)
{
	uint32_t *msg_word;

	msg_word = (uint32_t *) qdf_nbuf_data(rx_ind_msg);
	msg_word++;
	*seq_num_start = HTT_RX_IND_FLUSH_SEQ_NUM_START_GET(*msg_word);
	*seq_num_end = HTT_RX_IND_FLUSH_SEQ_NUM_END_GET(*msg_word);
}

int htt_rx_ind_release(htt_pdev_handle pdev, qdf_nbuf_t rx_ind_msg)
{
	uint32_t *msg_word;

	msg_word = (uint32_t *) qdf_nbuf_data(rx_ind_msg);
	return HTT_RX_IND_REL_VALID_GET(*msg_word);
}

void
htt_rx_ind_release_seq_num_range(htt_pdev_handle pdev,
				 qdf_nbuf_t rx_ind_msg,
				 unsigned *seq_num_start, unsigned *seq_num_end)
{
	uint32_t *msg_word;

	msg_word = (uint32_t *) qdf_nbuf_data(rx_ind_msg);
	msg_word++;
	*seq_num_start = HTT_RX_IND_REL_SEQ_NUM_START_GET(*msg_word);
	*seq_num_end = HTT_RX_IND_REL_SEQ_NUM_END_GET(*msg_word);
}

void
htt_rx_ind_mpdu_range_info(struct htt_pdev_t *pdev,
			   qdf_nbuf_t rx_ind_msg,
			   int mpdu_range_num,
			   enum htt_rx_status *status, int *mpdu_count)
{
	uint32_t *msg_word;

	msg_word = (uint32_t *) qdf_nbuf_data(rx_ind_msg);
	msg_word += pdev->rx_mpdu_range_offset_words + mpdu_range_num;
	*status = HTT_RX_IND_MPDU_STATUS_GET(*msg_word);
	*mpdu_count = HTT_RX_IND_MPDU_COUNT_GET(*msg_word);
}

/**
 * htt_rx_ind_rssi_dbm() - Return the RSSI provided in a rx indication message.
 *
 * @pdev:       the HTT instance the rx data was received on
 * @rx_ind_msg: the netbuf containing the rx indication message
 *
 * Return the RSSI from an rx indication message, in dBm units.
 *
 * Return: RSSI in dBm, or HTT_INVALID_RSSI
 */
int16_t htt_rx_ind_rssi_dbm(htt_pdev_handle pdev, qdf_nbuf_t rx_ind_msg)
{
	int8_t rssi;
	uint32_t *msg_word;

	msg_word = (uint32_t *)
		   (qdf_nbuf_data(rx_ind_msg) +
		    HTT_RX_IND_FW_RX_PPDU_DESC_BYTE_OFFSET);

	/* check if the RX_IND message contains valid rx PPDU start info */
	if (!HTT_RX_IND_START_VALID_GET(*msg_word))
		return HTT_RSSI_INVALID;

	rssi = HTT_RX_IND_RSSI_CMB_GET(*msg_word);
	return (HTT_TGT_RSSI_INVALID == rssi) ?
	       HTT_RSSI_INVALID : rssi;
}

/**
 * htt_rx_ind_rssi_dbm_chain() - Return the RSSI for a chain provided in a rx
 *              indication message.
 * @pdev:       the HTT instance the rx data was received on
 * @rx_ind_msg: the netbuf containing the rx indication message
 * @chain:      the index of the chain (0-4)
 *
 * Return the RSSI for a chain from an rx indication message, in dBm units.
 *
 * Return: RSSI, or HTT_INVALID_RSSI
 */
int16_t
htt_rx_ind_rssi_dbm_chain(htt_pdev_handle pdev, qdf_nbuf_t rx_ind_msg,
		      int8_t chain)
{
	int8_t rssi;
	uint32_t *msg_word;

	if (chain < 0 || chain > 3)
		return HTT_RSSI_INVALID;

	msg_word = (uint32_t *)
		(qdf_nbuf_data(rx_ind_msg) +
		 HTT_RX_IND_FW_RX_PPDU_DESC_BYTE_OFFSET);

	/* check if the RX_IND message contains valid rx PPDU start info */
	if (!HTT_RX_IND_START_VALID_GET(*msg_word))
		return HTT_RSSI_INVALID;

	msg_word += 1 + chain;

	rssi = HTT_RX_IND_RSSI_PRI20_GET(*msg_word);
	return (HTT_TGT_RSSI_INVALID == rssi) ?
		HTT_RSSI_INVALID :
		rssi;
}

/**
 * htt_rx_ind_legacy_rate() - Return the data rate
 * @pdev:        the HTT instance the rx data was received on
 * @rx_ind_msg:  the netbuf containing the rx indication message
 * @legacy_rate: (output) the data rate
 *      The legacy_rate parameter's value depends on the
 *      legacy_rate_sel value.
 *      If legacy_rate_sel is 0:
 *              0x8: OFDM 48 Mbps
 *              0x9: OFDM 24 Mbps
 *              0xA: OFDM 12 Mbps
 *              0xB: OFDM 6 Mbps
 *              0xC: OFDM 54 Mbps
 *              0xD: OFDM 36 Mbps
 *              0xE: OFDM 18 Mbps
 *              0xF: OFDM 9 Mbps
 *      If legacy_rate_sel is 1:
 *              0x8: CCK 11 Mbps long preamble
 *              0x9: CCK 5.5 Mbps long preamble
 *              0xA: CCK 2 Mbps long preamble
 *              0xB: CCK 1 Mbps long preamble
 *              0xC: CCK 11 Mbps short preamble
 *              0xD: CCK 5.5 Mbps short preamble
 *              0xE: CCK 2 Mbps short preamble
 *      -1 on error.
 * @legacy_rate_sel: (output) 0 to indicate OFDM, 1 to indicate CCK.
 *      -1 on error.
 *
 * Return the data rate provided in a rx indication message.
 */
void
htt_rx_ind_legacy_rate(htt_pdev_handle pdev, qdf_nbuf_t rx_ind_msg,
		       uint8_t *legacy_rate, uint8_t *legacy_rate_sel)
{
	uint32_t *msg_word;

	msg_word = (uint32_t *)
		(qdf_nbuf_data(rx_ind_msg) +
		 HTT_RX_IND_FW_RX_PPDU_DESC_BYTE_OFFSET);

	/* check if the RX_IND message contains valid rx PPDU start info */
	if (!HTT_RX_IND_START_VALID_GET(*msg_word)) {
		*legacy_rate = -1;
		*legacy_rate_sel = -1;
		return;
	}

	*legacy_rate = HTT_RX_IND_LEGACY_RATE_GET(*msg_word);
	*legacy_rate_sel = HTT_RX_IND_LEGACY_RATE_SEL_GET(*msg_word);
}

/**
 * htt_rx_ind_timestamp() - Return the timestamp
 * @pdev:                  the HTT instance the rx data was received on
 * @rx_ind_msg:            the netbuf containing the rx indication message
 * @timestamp_microsec:    (output) the timestamp to microsecond resolution.
 *                         -1 on error.
 * @timestamp_submicrosec: the submicrosecond portion of the
 *                         timestamp. -1 on error.
 *
 * Return the timestamp provided in a rx indication message.
 */
void
htt_rx_ind_timestamp(htt_pdev_handle pdev, qdf_nbuf_t rx_ind_msg,
		     uint32_t *timestamp_microsec,
		     uint8_t *timestamp_submicrosec)
{
	uint32_t *msg_word;

	msg_word = (uint32_t *)
		(qdf_nbuf_data(rx_ind_msg) +
		 HTT_RX_IND_FW_RX_PPDU_DESC_BYTE_OFFSET);

	/* check if the RX_IND message contains valid rx PPDU start info */
	if (!HTT_RX_IND_END_VALID_GET(*msg_word)) {
		*timestamp_microsec = -1;
		*timestamp_submicrosec = -1;
		return;
	}

	*timestamp_microsec = *(msg_word + 6);
	*timestamp_submicrosec =
		HTT_RX_IND_TIMESTAMP_SUBMICROSEC_GET(*msg_word);
}

#define INVALID_TSF -1
/**
 * htt_rx_ind_tsf32() - Return the TSF timestamp
 * @pdev:       the HTT instance the rx data was received on
 * @rx_ind_msg: the netbuf containing the rx indication message
 *
 * Return the TSF timestamp provided in a rx indication message.
 *
 * Return: TSF timestamp
 */
uint32_t
htt_rx_ind_tsf32(htt_pdev_handle pdev, qdf_nbuf_t rx_ind_msg)
{
	uint32_t *msg_word;

	msg_word = (uint32_t *)
		(qdf_nbuf_data(rx_ind_msg) +
		 HTT_RX_IND_FW_RX_PPDU_DESC_BYTE_OFFSET);

	/* check if the RX_IND message contains valid rx PPDU start info */
	if (!HTT_RX_IND_END_VALID_GET(*msg_word))
		return INVALID_TSF;

	return *(msg_word + 5);
}

/**
 * htt_rx_ind_ext_tid() - Return the extended traffic ID provided in a rx indication message.
 * @pdev:       the HTT instance the rx data was received on
 * @rx_ind_msg: the netbuf containing the rx indication message
 *
 * Return the extended traffic ID in a rx indication message.
 *
 * Return: Extended TID
 */
uint8_t
htt_rx_ind_ext_tid(htt_pdev_handle pdev, qdf_nbuf_t rx_ind_msg)
{
	uint32_t *msg_word;

	msg_word = (uint32_t *)
		(qdf_nbuf_data(rx_ind_msg));

	return HTT_RX_IND_EXT_TID_GET(*msg_word);
}

/*--- stats confirmation message ---*/

void
htt_t2h_dbg_stats_hdr_parse(uint8_t *stats_info_list,
			    enum htt_dbg_stats_type *type,
			    enum htt_dbg_stats_status *status,
			    int *length, uint8_t **stats_data)
{
	uint32_t *msg_word = (uint32_t *) stats_info_list;
	*type = HTT_T2H_STATS_CONF_TLV_TYPE_GET(*msg_word);
	*status = HTT_T2H_STATS_CONF_TLV_STATUS_GET(*msg_word);
	*length = HTT_T2H_STATS_CONF_TLV_HDR_SIZE +     /* header length */
		HTT_T2H_STATS_CONF_TLV_LENGTH_GET(*msg_word); /* data len */
	*stats_data = stats_info_list + HTT_T2H_STATS_CONF_TLV_HDR_SIZE;
}

void
htt_rx_frag_ind_flush_seq_num_range(htt_pdev_handle pdev,
				    qdf_nbuf_t rx_frag_ind_msg,
				    int *seq_num_start, int *seq_num_end)
{
	uint32_t *msg_word;

	msg_word = (uint32_t *) qdf_nbuf_data(rx_frag_ind_msg);
	msg_word++;
	*seq_num_start = HTT_RX_FRAG_IND_FLUSH_SEQ_NUM_START_GET(*msg_word);
	*seq_num_end = HTT_RX_FRAG_IND_FLUSH_SEQ_NUM_END_GET(*msg_word);
}
