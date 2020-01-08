/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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

#include <dp_types.h>
#include <dp_fisa_rx.h>
#include "hal_rx_flow.h"
#include "dp_htt.h"
#include "dp_internal.h"
#include <enet.h>
#include <linux/skbuff.h>

#if defined(FISA_DEBUG_ENABLE)
/**
 * hex_dump_skb_data() - Helper function to dump skb while debugging
 * @nbuf: Nbuf to be dumped
 * @dump: dump enable/disable dumping
 *
 * Return: NONE
 */
static void hex_dump_skb_data(qdf_nbuf_t nbuf, bool dump)
{
	qdf_nbuf_t next_nbuf;
	int i = 0;

	if (!dump)
		return;

	if (!nbuf)
		return;

	dp_fisa_debug("%ps: skb: %pk skb->next:%pk frag_list %pk skb->data:%pk len %d data_len%d",
	       (void *)_RET_IP_, nbuf, qdf_nbuf_next(nbuf),
	       skb_shinfo(nbuf)->frag_list, qdf_nbuf_data(nbuf), nbuf->len,
	       nbuf->data_len);
	QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR, nbuf->data,
			   64);

	next_nbuf = skb_shinfo(nbuf)->frag_list;
	while (next_nbuf) {
		dp_fisa_debug("%d nbuf:%pk nbuf->next:%pK nbuf->data:%pk len %d", i,
		       next_nbuf, qdf_nbuf_next(next_nbuf),
		       qdf_nbuf_data(next_nbuf), qdf_nbuf_len(next_nbuf));
		QDF_TRACE_HEX_DUMP(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
				   qdf_nbuf_data(next_nbuf), 64);
		next_nbuf = qdf_nbuf_next(next_nbuf);
		i++;
	}
}

/**
 * dump_tlvs() - Helper function to dump TLVs of msdu
 * @hal_soc_hdl: Handle to TLV functions
 * @buf: Pointer to TLV header
 * @dbg_level: level control output of TLV dump
 *
 * Return: NONE
 */
static void dump_tlvs(hal_soc_handle_t hal_soc_hdl, uint8_t *buf,
		      uint8_t dbg_level)
{
	uint32_t fisa_aggr_count, fisa_timeout, cumulat_l4_csum, cumulat_ip_len;
	int flow_aggr_cont;

	hal_rx_dump_pkt_tlvs(hal_soc_hdl, buf, dbg_level);

	flow_aggr_cont = hal_rx_get_fisa_flow_agg_continuation(hal_soc_hdl,
							       buf);
	fisa_aggr_count = hal_rx_get_fisa_flow_agg_count(hal_soc_hdl, buf);
	fisa_timeout = hal_rx_get_fisa_timeout(hal_soc_hdl, buf);
	cumulat_l4_csum = hal_rx_get_fisa_cumulative_l4_checksum(hal_soc_hdl,
								 buf);
	cumulat_ip_len = hal_rx_get_fisa_cumulative_ip_length(hal_soc_hdl, buf);

	dp_fisa_debug("flow_aggr_cont %d, fisa_timeout %d, fisa_aggr_count %d, cumulat_l4_csum %d, cumulat_ip_len %d",
	       flow_aggr_cont, fisa_timeout, fisa_aggr_count, cumulat_l4_csum,
	       cumulat_ip_len);
}
#else
static void hex_dump_skb_data(qdf_nbuf_t nbuf, bool dump)
{
}

static void dump_tlvs(hal_soc_handle_t hal_soc_hdl, uint8_t *buf,
		      uint8_t dbg_level)
{
}
#endif

/**
 * nbuf_skip_rx_pkt_tlv() - Function to skip the TLVs and mac header from msdu
 * @hal_soc_hdl: Handle to hal_soc to get the TLV info
 * @nbuf: msdu for which TLVs has to be skipped
 *
 * Return: None
 */
static void nbuf_skip_rx_pkt_tlv(hal_soc_handle_t hal_soc_hdl, qdf_nbuf_t nbuf)
{
	uint8_t *rx_tlv_hdr;
	uint32_t l2_hdr_offset;

	rx_tlv_hdr = qdf_nbuf_data(nbuf);
	l2_hdr_offset = hal_rx_msdu_end_l3_hdr_padding_get(hal_soc_hdl,
							   rx_tlv_hdr);
	qdf_nbuf_pull_head(nbuf, RX_PKT_TLVS_LEN + l2_hdr_offset);
}

/**
 * print_flow_tuple() - Debug function to dump flow tuple
 * @flow_tuple: flow tuple containing tuple info
 *
 * Return: NONE
 */
static void print_flow_tuple(struct cdp_rx_flow_tuple_info *flow_tuple)
{
	dp_info("dest_ip_127_96 0x%x", flow_tuple->dest_ip_127_96);
	dp_info("dest_ip_95_64 0x%x", flow_tuple->dest_ip_95_64);
	dp_info("dest_ip_63_32 0x%x", flow_tuple->dest_ip_63_32);
	dp_info("dest_ip_31_0 0x%x", flow_tuple->dest_ip_31_0);
	dp_info("src_ip_127_96 0x%x", flow_tuple->src_ip_127_96);
	dp_info("src_ip_95_64 0x%x", flow_tuple->src_ip_95_64);
	dp_info("src_ip_63_32 0x%x", flow_tuple->src_ip_63_32);
	dp_info("src_ip_31_0 0x%x", flow_tuple->src_ip_31_0);
	dp_info("dest_port 0x%x", flow_tuple->dest_port);
	dp_info("src_port 0x%x", flow_tuple->src_port);
	dp_info("l4_protocol 0x%x", flow_tuple->l4_protocol);
}

/**
 * get_flow_tuple_from_nbuf() - Get the flow tuple from msdu
 * @hal_soc_hdl: Handle to hal soc
 * @flow_tuple_info: return argument where the flow is populated
 * @nbuf: msdu from which flow tuple is extracted.
 * @rx_tlv_hdr: Pointer to msdu TLVs
 *
 * Return: None
 */
static void
get_flow_tuple_from_nbuf(hal_soc_handle_t hal_soc_hdl,
			 struct cdp_rx_flow_tuple_info *flow_tuple_info,
			 qdf_nbuf_t nbuf, uint8_t *rx_tlv_hdr)
{
	struct iphdr *iph;
	struct tcphdr *tcph;
	uint32_t ip_hdr_offset = HAL_RX_TLV_GET_IP_OFFSET(rx_tlv_hdr);
	uint32_t tcp_hdr_offset = HAL_RX_TLV_GET_TCP_OFFSET(rx_tlv_hdr);
	uint32_t l2_hdr_offset = hal_rx_msdu_end_l3_hdr_padding_get(hal_soc_hdl,
								    rx_tlv_hdr);

	flow_tuple_info->tuple_populated = true;

	qdf_nbuf_pull_head(nbuf, RX_PKT_TLVS_LEN + l2_hdr_offset);

	iph = (struct iphdr *)(qdf_nbuf_data(nbuf) + ip_hdr_offset);
	tcph = (struct tcphdr *)(qdf_nbuf_data(nbuf) + ip_hdr_offset +
						tcp_hdr_offset);

	flow_tuple_info->dest_ip_31_0 = qdf_ntohl(iph->daddr);
	flow_tuple_info->dest_ip_63_32 = 0;
	flow_tuple_info->dest_ip_95_64 = 0;
	flow_tuple_info->dest_ip_127_96 =
		HAL_IP_DA_SA_PREFIX_IPV4_COMPATIBLE_IPV6;

	flow_tuple_info->src_ip_31_0 = qdf_ntohl(iph->saddr);
	flow_tuple_info->src_ip_63_32 = 0;
	flow_tuple_info->src_ip_95_64 = 0;
	flow_tuple_info->src_ip_127_96 =
		HAL_IP_DA_SA_PREFIX_IPV4_COMPATIBLE_IPV6;

	flow_tuple_info->dest_port = qdf_ntohs(tcph->dest);
	flow_tuple_info->src_port = qdf_ntohs(tcph->source);
	flow_tuple_info->l4_protocol = iph->protocol;
	dp_fisa_debug("l4_protocol %d", flow_tuple_info->l4_protocol);

	qdf_nbuf_push_head(nbuf, RX_PKT_TLVS_LEN + l2_hdr_offset);

	dp_fisa_debug("head_skb: %pk head_skb->next:%pk head_skb->data:%pk len %d data_len",
		      nbuf, qdf_nbuf_next(nbuf), qdf_nbuf_data(nbuf), nbuf->len,
		      nbuf->data_len);
}

/**
 * dp_rx_fisa_setup_hw_fse() - Populate flow so as to update DDR flow table
 * @fisa_hdl: Handle fisa context
 * @hashed_flow_idx: Index to flow table
 * @rx_flow_info: tuple to be populated in flow table
 * @flow_steer_info: REO index to which flow to be steered
 *
 * Return: Pointer to DDR flow table entry
 */
static void *
dp_rx_fisa_setup_hw_fse(struct dp_rx_fst *fisa_hdl,
			uint32_t hashed_flow_idx,
			struct cdp_rx_flow_tuple_info *rx_flow_info,
			uint32_t flow_steer_info)
{
	struct hal_rx_flow flow;
	void *hw_fse;

	/* REO destination index starts from 1 */
	flow.reo_destination_indication = flow_steer_info + 1;
	flow.fse_metadata = 0xDEADBEEF;
	flow.tuple_info.dest_ip_127_96 = rx_flow_info->dest_ip_127_96;
	flow.tuple_info.dest_ip_95_64 = rx_flow_info->dest_ip_95_64;
	flow.tuple_info.dest_ip_63_32 =	rx_flow_info->dest_ip_63_32;
	flow.tuple_info.dest_ip_31_0 = rx_flow_info->dest_ip_31_0;
	flow.tuple_info.src_ip_127_96 =	rx_flow_info->src_ip_127_96;
	flow.tuple_info.src_ip_95_64 = rx_flow_info->src_ip_95_64;
	flow.tuple_info.src_ip_63_32 = rx_flow_info->src_ip_63_32;
	flow.tuple_info.src_ip_31_0 = rx_flow_info->src_ip_31_0;
	flow.tuple_info.dest_port = rx_flow_info->dest_port;
	flow.tuple_info.src_port = rx_flow_info->src_port;
	flow.tuple_info.l4_protocol = rx_flow_info->l4_protocol;
	flow.reo_destination_handler = HAL_RX_FSE_REO_DEST_FT;
	hw_fse = hal_rx_flow_setup_fse(fisa_hdl->hal_rx_fst, hashed_flow_idx,
				       &flow);
	dp_rx_dump_fisa_table(fisa_hdl->soc_hdl);

	return hw_fse;
}

/**
 * dp_rx_fisa_update_sw_ft_entry() - Helper function to update few SW FT entry
 * @sw_ft_entry: Pointer to softerware flow tabel entry
 * @flow_hash: flow_hash for the flow
 * @vdev: Saving dp_vdev in FT later used in the flushing the flow
 * @flow_id: Flow ID of the flow
 *
 * Return: NONE
 */
static void dp_rx_fisa_update_sw_ft_entry(struct dp_fisa_rx_sw_ft *sw_ft_entry,
					  uint32_t flow_hash,
					  struct dp_vdev *vdev,
					  uint32_t flow_id)
{
	sw_ft_entry->flow_hash = flow_hash;
	sw_ft_entry->flow_id = flow_id;
	sw_ft_entry->vdev = vdev;
}

/**
 * is_same_flow() - Function to compare flow tuple to decide if they match
 * @tuple1: flow tuple 1
 * @tuple2: flow tuple 2
 *
 * Return: true if they match, false if they differ
 */
static bool is_same_flow(struct cdp_rx_flow_tuple_info *tuple1,
			 struct cdp_rx_flow_tuple_info *tuple2)
{
	if ((tuple1->src_port ^ tuple2->src_port) |
	    (tuple1->dest_port ^ tuple2->dest_port) |
	    (tuple1->src_ip_31_0 ^ tuple2->src_ip_31_0) |
	    (tuple1->src_ip_63_32 ^ tuple2->src_ip_63_32) |
	    (tuple1->src_ip_95_64 ^ tuple2->src_ip_95_64) |
	    (tuple1->src_ip_127_96 ^ tuple2->src_ip_127_96) |
	    (tuple1->dest_ip_31_0 ^ tuple2->dest_ip_31_0) |
	    /* DST IP check not required? */
	    (tuple1->dest_ip_63_32 ^ tuple2->dest_ip_63_32) |
	    (tuple1->dest_ip_95_64 ^ tuple2->dest_ip_95_64) |
	    (tuple1->dest_ip_127_96 ^ tuple2->dest_ip_127_96) |
	    (tuple1->l4_protocol ^ tuple2->l4_protocol))
		return false;
	else
		return true;
}

/**
 * dp_rx_flow_send_htt_operation_cmd() - Invalidate FSE cache on FT change
 * @pdev: handle to DP pdev
 * @fse_op: Cache operation code
 * @rx_flow_tuple: flow tuple whose entry has to be invalidated
 *
 * Return: Success if we successfully send FW HTT command
 */
static QDF_STATUS
dp_rx_flow_send_htt_operation_cmd(struct dp_pdev *pdev,
				  enum dp_htt_flow_fst_operation fse_op,
				  struct cdp_rx_flow_tuple_info *rx_flow_tuple)
{
	struct dp_htt_rx_flow_fst_operation fse_op_cmd;
	struct cdp_rx_flow_info rx_flow_info;

	rx_flow_info.is_addr_ipv4 = true;
	rx_flow_info.op_code = CDP_FLOW_FST_ENTRY_ADD;
	qdf_mem_copy(&rx_flow_info.flow_tuple_info, rx_flow_tuple,
		     sizeof(struct cdp_rx_flow_tuple_info));
	rx_flow_info.fse_metadata = 0xDADA;
	fse_op_cmd.pdev_id = pdev->pdev_id;
	fse_op_cmd.op_code = fse_op;
	fse_op_cmd.rx_flow = &rx_flow_info;

	return dp_htt_rx_flow_fse_operation(pdev, &fse_op_cmd);
}

/**
 * dp_rx_fisa_add_ft_entry() - Add new flow to HW and SW FT if it is not added
 * @fisa_hdl: handle to FISA context
 * @flow_idx_hash: Hashed flow index
 * @nbuf: nbuf belonging to new flow
 * @vdev: Handle DP vdev to save in SW flow table
 * @rx_tlv_hdr: Pointer to TLV header
 *
 * Return: pointer to sw FT entry on success, NULL otherwise
 */
static struct dp_fisa_rx_sw_ft *
dp_rx_fisa_add_ft_entry(struct dp_rx_fst *fisa_hdl,
			uint32_t flow_idx_hash,
			qdf_nbuf_t nbuf, struct dp_vdev *vdev,
			uint8_t *rx_tlv_hdr)
{
	struct dp_fisa_rx_sw_ft *sw_ft_entry;
	uint32_t flow_hash;
	uint32_t hashed_flow_idx;
	uint32_t skid_count = 0, max_skid_length;
	struct cdp_rx_flow_tuple_info rx_flow_tuple_info;
	QDF_STATUS status;
	bool is_fst_updated = false;
	bool is_flow_tcp, is_flow_udp, is_flow_ipv6;
	hal_soc_handle_t hal_soc_hdl = fisa_hdl->soc_hdl->hal_soc;
	uint32_t reo_id = QDF_NBUF_CB_RX_CTX_ID(nbuf);

	is_flow_tcp = HAL_RX_TLV_GET_TCP_PROTO(rx_tlv_hdr);
	is_flow_udp = HAL_RX_TLV_GET_UDP_PROTO(rx_tlv_hdr);
	is_flow_ipv6 = HAL_RX_TLV_GET_IPV6(rx_tlv_hdr);

	if (is_flow_ipv6 || !(is_flow_tcp || is_flow_udp)) {
		dp_fisa_debug("Not UDP or TCP IPV4 flow");
		return NULL;
	}

	/* Get the hash from TLV
	 * FSE FT Toeplitz hash is same Common parser hash available in TLV
	 * common parser toeplitz hash is same as FSE toeplitz hash as
	 * toeplitz key is same.
	 */
	rx_flow_tuple_info.tuple_populated = false;
	flow_hash = flow_idx_hash;
	hashed_flow_idx = flow_hash & fisa_hdl->hash_mask;
	max_skid_length = fisa_hdl->max_skid_length;

	dp_fisa_debug("flow_hash 0x%x hashed_flow_idx 0x%x", flow_hash,
		      hashed_flow_idx);
	dp_fisa_debug("max_skid_length 0x%x", max_skid_length);
	qdf_spin_lock_bh(&fisa_hdl->dp_rx_fst_lock);
	do {
		sw_ft_entry = &(((struct dp_fisa_rx_sw_ft *)
					fisa_hdl->base)[hashed_flow_idx]);
		if (!sw_ft_entry->is_populated) {
			/* Add SW FT entry */
			dp_rx_fisa_update_sw_ft_entry(sw_ft_entry,
						      flow_hash, vdev,
						      hashed_flow_idx);
			if (!rx_flow_tuple_info.tuple_populated)
				get_flow_tuple_from_nbuf(hal_soc_hdl,
							 &rx_flow_tuple_info,
							 nbuf, rx_tlv_hdr);

			/* Add HW FT entry */
			sw_ft_entry->hw_fse =
				dp_rx_fisa_setup_hw_fse(fisa_hdl,
							hashed_flow_idx,
							&rx_flow_tuple_info,
							reo_id);
			sw_ft_entry->is_populated = true;
			sw_ft_entry->napi_id = reo_id;
			qdf_mem_copy(&sw_ft_entry->rx_flow_tuple_info,
				     &rx_flow_tuple_info,
				     sizeof(struct cdp_rx_flow_tuple_info));

			sw_ft_entry->is_flow_tcp = is_flow_tcp;
			sw_ft_entry->is_flow_udp = is_flow_udp;

			is_fst_updated = true;
			fisa_hdl->add_flow_count++;
			break;
		}
		/* else */
		if (!rx_flow_tuple_info.tuple_populated)
			get_flow_tuple_from_nbuf(hal_soc_hdl,
						 &rx_flow_tuple_info,
						 nbuf, rx_tlv_hdr);

		if (is_same_flow(&sw_ft_entry->rx_flow_tuple_info,
				 &rx_flow_tuple_info)) {
			dp_fisa_debug("It is same flow fse entry idx %d",
				      hashed_flow_idx);
			/* Incoming flow tuple matches with existing
			 * entry. This is subsequent skbs of the same
			 * flow. Earlier entry made is not reflected
			 * yet in FSE cache
			 */
			break;
		}
		/* else */
		/* hash collision move to the next FT entry */
		dp_fisa_debug("Hash collision %d", fisa_hdl->hash_collision_cnt);
		fisa_hdl->hash_collision_cnt++;
#ifdef NOT_YET /* assist Flow eviction algorithm */
	/* uint32_t lru_ft_entry_time = 0xffffffff, lru_ft_entry_idx = 0; */
		if (fisa_hdl->hw_ft_entry->timestamp < lru_ft_entry_time) {
			lru_ft_entry_time = fisa_hdl->hw_ft_entry->timestamp;
			lru_ft_entry_idx = hashed_flow_idx;
		}
#endif
		skid_count++;
		hashed_flow_idx++;
		hashed_flow_idx &= fisa_hdl->hash_mask;
	} while (skid_count <= max_skid_length);

	/*
	 * fisa_hdl->flow_eviction_cnt++;
	 * if (skid_count > max_skid_length)
	 * Remove LRU flow from HW FT
	 * Remove LRU flow from SW FT
	 */
	qdf_spin_unlock_bh(&fisa_hdl->dp_rx_fst_lock);

	if (skid_count > max_skid_length) {
		dp_fisa_debug("Max skid length reached flow cannot be added, evict exiting flow");
		return NULL;
	}

	/**
	 * Send HTT cache invalidation command to firmware to
	 * reflect the flow update
	 */
	if (is_fst_updated) {
		status = dp_rx_flow_send_htt_operation_cmd(vdev->pdev,
					DP_HTT_FST_CACHE_INVALIDATE_FULL,
					&rx_flow_tuple_info);
		if (QDF_STATUS_SUCCESS != status) {
			dp_err("Failed to send the cache invalidation\n");
			/* TBD: remove flow from SW and HW flow table
			 * Not big impact cache entry gets updated later
			 */
		}
	}
	dp_fisa_debug("sw_ft_entry %pk", sw_ft_entry);
	return sw_ft_entry;
}

/**
 * is_flow_idx_valid() - Function to decide if flow_idx TLV is valid
 * @flow_invalid: flow invalid TLV value
 * @flow_timeout: flow timeout TLV value, set when FSE timedout flow search
 *
 * Return: True if flow_idx value is valid
 */
static bool is_flow_idx_valid(bool flow_invalid, bool flow_timeout)
{
	if (!flow_invalid && !flow_timeout)
		return true;
	else
		return false;
}

/**
 * dp_rx_get_fisa_flow() - Get FT entry corresponding to incoming nbuf
 * @fisa_hdl: handle to FISA context
 * @vdev: handle to DP vdev
 * @nbuf: incoming msdu
 *
 * Return: handle SW FT entry for nbuf flow
 */
static struct dp_fisa_rx_sw_ft *
dp_rx_get_fisa_flow(struct dp_rx_fst *fisa_hdl, struct dp_vdev *vdev,
		    qdf_nbuf_t nbuf)
{
	uint8_t *rx_tlv_hdr;
	uint32_t flow_idx;
	bool flow_invalid, flow_timeout, flow_idx_valid;
	struct dp_fisa_rx_sw_ft *sw_ft_entry = NULL;
	struct dp_fisa_rx_sw_ft *sw_ft_base = (struct dp_fisa_rx_sw_ft *)
								fisa_hdl->base;
	hal_soc_handle_t hal_soc_hdl = fisa_hdl->soc_hdl->hal_soc;

	if (QDF_NBUF_CB_RX_TCP_PROTO(nbuf))
		return sw_ft_entry;

	rx_tlv_hdr = qdf_nbuf_data(nbuf);
	hal_rx_msdu_get_flow_params(hal_soc_hdl, rx_tlv_hdr, &flow_invalid,
				    &flow_timeout, &flow_idx);

	dp_fisa_debug("nbuf %pk fl_idx %d fl_inv %d fl_timeout %d",
		      nbuf, flow_idx, flow_invalid, flow_timeout);

	flow_idx_valid = is_flow_idx_valid(flow_invalid, flow_timeout);
	if (flow_idx_valid) {
		qdf_assert_always(flow_idx < fisa_hdl->max_entries);
		dp_fisa_debug("flow_idx is valid 0x%x", flow_idx);
		return &sw_ft_base[flow_idx];
	}

	/* else new flow, add entry to FT */
	sw_ft_entry = dp_rx_fisa_add_ft_entry(fisa_hdl, flow_idx, nbuf, vdev,
					      rx_tlv_hdr);

	return sw_ft_entry;
}

/**
 * dp_add_nbuf_to_fisa_flow() - Aggregate incoming nbuf
 * @fisa_hdl: handle to fisa context
 * @vdev: handle DP vdev
 * @nbuf: Incoming nbuf
 * @fisa_flow: Handle SW flow entry
 *
 * Return: Success on aggregation
 */
static int dp_add_nbuf_to_fisa_flow(struct dp_rx_fst *fisa_hdl,
				    struct dp_vdev *vdev, qdf_nbuf_t nbuf,
				    struct dp_fisa_rx_sw_ft *fisa_flow)
{
	return FISA_AGGR_NOT_ELIGIBLE;
}

/**
 * dp_fisa_rx() - Entry function to FISA to handle aggregation
 * @soc: core txrx main context
 * @vdev: Handle DP vdev
 * @nbuf_list: List nbufs to be aggregated
 *
 * Return: Success on aggregation
 */
QDF_STATUS dp_fisa_rx(struct dp_soc *soc, struct dp_vdev *vdev,
		      qdf_nbuf_t nbuf_list)
{
	struct dp_rx_fst *dp_fisa_rx_hdl = soc->rx_fst;
	qdf_nbuf_t head_nbuf;
	qdf_nbuf_t next_nbuf;
	struct dp_fisa_rx_sw_ft *fisa_flow;
	int fisa_ret;

	head_nbuf = nbuf_list;

	while (head_nbuf) {
		next_nbuf = head_nbuf->next;
		qdf_nbuf_set_next(head_nbuf, NULL);

		/* Add new flow if the there is no ongoing flow */
		fisa_flow = dp_rx_get_fisa_flow(dp_fisa_rx_hdl, vdev,
						head_nbuf);

		/* Fragmented skb do not handle via fisa
		 * get that flow and deliver that flow to rx_thread
		 */
		if (qdf_unlikely(qdf_nbuf_get_ext_list(head_nbuf))) {
			dp_fisa_debug("Fragmented skb, will not be FISAed");
			if (fisa_flow)
				dp_rx_fisa_flush_flow(vdev, fisa_flow);
			goto deliver_nbuf;
		}

		if (!fisa_flow)
			goto pull_nbuf;

		fisa_ret = dp_add_nbuf_to_fisa_flow(dp_fisa_rx_hdl, vdev,
						    head_nbuf, fisa_flow);
		if (fisa_ret == FISA_AGGR_DONE)
			goto next_msdu;
		else
			qdf_assert(0);

pull_nbuf:
		nbuf_skip_rx_pkt_tlv(dp_fisa_rx_hdl->soc_hdl->hal_soc,
				     head_nbuf);

deliver_nbuf: /* Deliver without FISA */
		qdf_nbuf_set_next(head_nbuf, NULL);
		hex_dump_skb_data(head_nbuf, false);
		vdev->osif_rx(vdev->osif_vdev, head_nbuf);
next_msdu:
		head_nbuf = next_nbuf;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_fisa_flush() - Flush function to end of context flushing of aggregates
 * @soc: core txrx main context
 * @napi_id: REO number to flush the flow Rxed on the REO
 *
 * Return: Success on flushing the flows for the REO
 */
QDF_STATUS dp_rx_fisa_flush(struct dp_soc *soc, int napi_id)
{
	struct dp_rx_fst *fisa_hdl = soc->rx_fst;
	struct dp_fisa_rx_sw_ft *sw_ft_entry =
		(struct dp_fisa_rx_sw_ft *)fisa_hdl->base;
	int ft_size = fisa_hdl->max_entries;
	int i;

	for (i = 0; i < ft_size; i++) {
		if (sw_ft_entry[i].napi_id == napi_id &&
		    sw_ft_entry[i].is_populated) {
			dp_fisa_debug("flushing %d %pk napi_id %d\n", i,
				      &sw_ft_entry[i], napi_id);
			/* Save the ip_len and checksum as hardware assist is
			 * always based on his start of aggregation
			 */
			sw_ft_entry[i].napi_flush_cumulative_l4_checksum =
					sw_ft_entry[i].cumulative_l4_checksum;
			sw_ft_entry[i].napi_flush_cumulative_ip_length =
					sw_ft_entry[i].hal_cumultive_ip_len;
			dp_fisa_debug("napi_flush_cumulative_ip_length 0x%x",
				sw_ft_entry[i].napi_flush_cumulative_ip_length);

			dp_rx_fisa_flush_flow(sw_ft_entry[i].vdev,
					      &sw_ft_entry[i]);
			sw_ft_entry[i].cur_aggr = 0;
		}
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_rx_dump_fisa_stats(struct dp_soc *soc)
{
	struct dp_rx_fst *rx_fst = soc->rx_fst;
	struct dp_fisa_rx_sw_ft *sw_ft_entry =
		&((struct dp_fisa_rx_sw_ft *)rx_fst->base)[0];
	int ft_size = rx_fst->max_entries;
	int i;

	dp_info("Num of flows programmed %d", rx_fst->add_flow_count);
	dp_info("Num of flows evicted %d", rx_fst->del_flow_count);
	dp_info("Hash collision count %d", rx_fst->hash_collision_cnt);

	for (i = 0; i < ft_size; i++, sw_ft_entry++) {
		if (!sw_ft_entry->is_populated)
			continue;

		dp_info("FLOw ID %d is %s on napi/ring %d",
			sw_ft_entry->flow_id,
			sw_ft_entry->is_flow_udp ? "udp" : "tcp",
			sw_ft_entry->napi_id);
		dp_info("num msdu aggr %d", sw_ft_entry->aggr_count);
		dp_info("flush count %d", sw_ft_entry->flush_count);
		dp_info("bytes_aggregated %d", sw_ft_entry->bytes_aggregated);
		dp_info("avg aggregation %d",
			sw_ft_entry->bytes_aggregated / sw_ft_entry->flush_count
			);
		print_flow_tuple(&sw_ft_entry->rx_flow_tuple_info);
	}
	return QDF_STATUS_SUCCESS;
}
