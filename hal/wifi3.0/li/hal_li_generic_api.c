/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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

#include "hal_li_api.h"
#include "hal_li_hw_headers.h"
#include "hal_li_reo.h"

#include "hal_rx.h"
#include "hal_li_rx.h"
#include "hal_tx.h"
#include <hal_api_mon.h>

static uint32_t hal_get_reo_qdesc_size_li(uint32_t ba_window_size, int tid)
{
	/* Return descriptor size corresponding to window size of 2 since
	 * we set ba_window_size to 2 while setting up REO descriptors as
	 * a WAR to get 2k jump exception aggregates are received without
	 * a BA session.
	 */
	if (ba_window_size <= 1) {
		if (tid != HAL_NON_QOS_TID)
			return sizeof(struct rx_reo_queue) +
				sizeof(struct rx_reo_queue_ext);
		else
			return sizeof(struct rx_reo_queue);
	}

	if (ba_window_size <= 105)
		return sizeof(struct rx_reo_queue) +
			sizeof(struct rx_reo_queue_ext);

	if (ba_window_size <= 210)
		return sizeof(struct rx_reo_queue) +
			(2 * sizeof(struct rx_reo_queue_ext));

	return sizeof(struct rx_reo_queue) +
		(3 * sizeof(struct rx_reo_queue_ext));
}

void hal_set_link_desc_addr_li(void *desc, uint32_t cookie,
			       qdf_dma_addr_t link_desc_paddr)
{
	uint32_t *buf_addr = (uint32_t *)desc;

	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_0, BUFFER_ADDR_31_0,
			   link_desc_paddr & 0xffffffff);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_1, BUFFER_ADDR_39_32,
			   (uint64_t)link_desc_paddr >> 32);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_1, RETURN_BUFFER_MANAGER,
			   WBM_IDLE_DESC_LIST);
	HAL_DESC_SET_FIELD(buf_addr, BUFFER_ADDR_INFO_1, SW_BUFFER_COOKIE,
			   cookie);
}

void hal_tx_init_data_ring_li(hal_soc_handle_t hal_soc_hdl,
			      hal_ring_handle_t hal_ring_hdl)
{
	uint8_t *desc_addr;
	struct hal_srng_params srng_params;
	uint32_t desc_size;
	uint32_t num_desc;

	hal_get_srng_params(hal_soc_hdl, hal_ring_hdl, &srng_params);

	desc_addr = (uint8_t *)srng_params.ring_base_vaddr;
	desc_size = sizeof(struct tcl_data_cmd);
	num_desc = srng_params.num_entries;

	while (num_desc) {
		HAL_TX_DESC_SET_TLV_HDR(desc_addr, HAL_TX_TCL_DATA_TAG,
					desc_size);
		desc_addr += (desc_size + sizeof(struct tlv_32_hdr));
		num_desc--;
	}
}

/*
 * hal_rx_msdu_is_wlan_mcast_generic_li(): Check if the buffer is for multicast
 *					address
 * @nbuf: Network buffer
 *
 * Returns: flag to indicate whether the nbuf has MC/BC address
 */
static uint32_t hal_rx_msdu_is_wlan_mcast_generic_li(qdf_nbuf_t nbuf)
{
	uint8_t *buf = qdf_nbuf_data(nbuf);

	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;

	return rx_attn->mcast_bcast;
}

/**
 * hal_rx_tlv_decap_format_get_li() - Get packet decap format from the TLV
 * @hw_desc_addr: rx tlv desc
 *
 * Return: pkt decap format
 */
static uint32_t hal_rx_tlv_decap_format_get_li(void *hw_desc_addr)
{
	struct rx_msdu_start *rx_msdu_start;
	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_msdu_start = &rx_desc->msdu_start_tlv.rx_msdu_start;

	return HAL_RX_GET(rx_msdu_start, RX_MSDU_START_2, DECAP_FORMAT);
}

/**
 * hal_rx_dump_pkt_tlvs_li(): API to print all member elements of
 *			 RX TLVs
 * @ buf: pointer the pkt buffer.
 * @ dbg_level: log level.
 *
 * Return: void
 */
static void hal_rx_dump_pkt_tlvs_li(hal_soc_handle_t hal_soc_hdl,
				uint8_t *buf, uint8_t dbg_level)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	struct rx_mpdu_start *mpdu_start =
				&pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;
	struct rx_msdu_start *msdu_start =
				&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	struct rx_mpdu_end *mpdu_end = &pkt_tlvs->mpdu_end_tlv.rx_mpdu_end;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;

	hal_rx_dump_rx_attention_tlv(rx_attn, dbg_level);
	hal_rx_dump_mpdu_start_tlv(mpdu_start, dbg_level, hal_soc);
	hal_rx_dump_msdu_start_tlv(hal_soc, msdu_start, dbg_level);
	hal_rx_dump_mpdu_end_tlv(mpdu_end, dbg_level);
	hal_rx_dump_msdu_end_tlv(hal_soc, msdu_end, dbg_level);
	hal_rx_dump_pkt_hdr_tlv(pkt_tlvs, dbg_level);
}

/**
 * hal_rx_tlv_get_offload_info_li() - Get the offload info from TLV
 * @rx_tlv: RX tlv start address in buffer
 * @offload_info: Buffer to store the offload info
 *
 * Return: 0 on success, -EINVAL on failure.
 */
static int
hal_rx_tlv_get_offload_info_li(uint8_t *rx_tlv,
			       struct hal_offload_info *offload_info)
{
	offload_info->flow_id = HAL_RX_TLV_GET_FLOW_ID_TOEPLITZ(rx_tlv);
	offload_info->ipv6_proto = HAL_RX_TLV_GET_IPV6(rx_tlv);
	offload_info->lro_eligible = HAL_RX_TLV_GET_LRO_ELIGIBLE(rx_tlv);
	offload_info->tcp_proto = HAL_RX_TLV_GET_TCP_PROTO(rx_tlv);

	if (offload_info->tcp_proto) {
		offload_info->tcp_pure_ack =
					HAL_RX_TLV_GET_TCP_PURE_ACK(rx_tlv);
		offload_info->tcp_offset = HAL_RX_TLV_GET_TCP_OFFSET(rx_tlv);
		offload_info->tcp_win = HAL_RX_TLV_GET_TCP_WIN(rx_tlv);
		offload_info->tcp_seq_num = HAL_RX_TLV_GET_TCP_SEQ(rx_tlv);
		offload_info->tcp_ack_num = HAL_RX_TLV_GET_TCP_ACK(rx_tlv);
	}
	return 0;
}

/*
 * hal_rx_attn_phy_ppdu_id_get(): get phy_ppdu_id value
 * from rx attention
 * @buf: pointer to rx_pkt_tlvs
 *
 * Return: phy_ppdu_id
 */
static uint16_t hal_rx_attn_phy_ppdu_id_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	uint16_t phy_ppdu_id;

	phy_ppdu_id = HAL_RX_ATTN_PHY_PPDU_ID_GET(rx_attn);

	return phy_ppdu_id;
}

/**
 * hal_rx_msdu_start_msdu_len_get(): API to get the MSDU length
 * from rx_msdu_start TLV
 *
 * @ buf: pointer to the start of RX PKT TLV headers
 * Return: msdu length
 */
static uint32_t hal_rx_msdu_start_msdu_len_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
			&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t msdu_len;

	msdu_len = HAL_RX_MSDU_START_MSDU_LEN_GET(msdu_start);

	return msdu_len;
}

/**
 * hal_rx_get_frame_ctrl_field(): Function to retrieve frame control field
 *
 * @nbuf: Network buffer
 * Returns: rx more fragment bit
 *
 */
static uint16_t hal_rx_get_frame_ctrl_field_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = hal_rx_get_pkt_tlvs(buf);
	struct rx_mpdu_info *rx_mpdu_info = hal_rx_get_mpdu_info(pkt_tlvs);
	uint16_t frame_ctrl = 0;

	frame_ctrl = HAL_RX_MPDU_GET_FRAME_CONTROL_FIELD(rx_mpdu_info);

	return frame_ctrl;
}

/**
 * hal_rx_get_proto_params_li() - Get l4 proto values from TLV
 * @buf: rx tlv address
 * @proto_params: Buffer to store proto parameters
 *
 * Return: 0 on success.
 */
static int hal_rx_get_proto_params_li(uint8_t *buf, void *proto_params)
{
	struct hal_proto_params *param =
				(struct hal_proto_params *)proto_params;

	param->tcp_proto = HAL_RX_TLV_GET_IP_OFFSET(buf);
	param->udp_proto = HAL_RX_TLV_GET_UDP_PROTO(buf);
	param->ipv6_proto = HAL_RX_TLV_GET_IPV6(buf);

	return 0;
}

/**
 * hal_rx_get_l3_l4_offsets_li() - Get l3/l4 header offset from TLV
 * @buf: rx tlv start address
 * @l3_hdr_offset: buffer to store l3 offset
 * @l4_hdr_offset: buffer to store l4 offset
 *
 * Return: 0 on success.
 */
static int hal_rx_get_l3_l4_offsets_li(uint8_t *buf, uint32_t *l3_hdr_offset,
				       uint32_t *l4_hdr_offset)
{
	*l3_hdr_offset = HAL_RX_TLV_GET_IP_OFFSET(buf);
	*l4_hdr_offset = HAL_RX_TLV_GET_TCP_OFFSET(buf);

	return 0;
}

/**
 * hal_rx_tlv_get_pn_num_li() - Get packet number from RX TLV
 * @buf: rx tlv address
 * @pn_num: buffer to store packet number
 *
 * Return: None
 */
static inline void hal_rx_tlv_get_pn_num_li(uint8_t *buf, uint64_t *pn_num)
{
	struct rx_pkt_tlvs *rx_pkt_tlv =
			(struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_info *rx_mpdu_info_details =
	 &rx_pkt_tlv->mpdu_start_tlv.rx_mpdu_start.rx_mpdu_info_details;

	pn_num[0] = rx_mpdu_info_details->pn_31_0;
	pn_num[0] |=
		((uint64_t)rx_mpdu_info_details->pn_63_32 << 32);
	pn_num[1] = rx_mpdu_info_details->pn_95_64;
	pn_num[1] |=
		((uint64_t)rx_mpdu_info_details->pn_127_96 << 32);
}

#ifdef NO_RX_PKT_HDR_TLV
/**
 * hal_rx_pkt_hdr_get_li() - Get rx packet header start address.
 * @buf: packet start address
 *
 * Return: packet data start address.
 */
static inline uint8_t *hal_rx_pkt_hdr_get_li(uint8_t *buf)
{
	return buf + RX_PKT_TLVS_LEN;
}
#else
static inline uint8_t *hal_rx_pkt_hdr_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;

	return pkt_tlvs->pkt_hdr_tlv.rx_pkt_hdr;
}
#endif

/**
 * hal_rx_priv_info_set_in_tlv_li(): Save the private info to
 *				the reserved bytes of rx_tlv_hdr
 * @buf: start of rx_tlv_hdr
 * @priv_data: hal_wbm_err_desc_info structure
 * @len: length of the private data
 * Return: void
 */
static inline void
hal_rx_priv_info_set_in_tlv_li(uint8_t *buf, uint8_t *priv_data,
			       uint32_t len)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t copy_len = (len > RX_PADDING0_BYTES) ?
			    RX_PADDING0_BYTES : len;

	qdf_mem_copy(pkt_tlvs->rx_padding0, priv_data, copy_len);
}

/**
 * hal_rx_priv_info_get_from_tlv_li(): retrieve the private data from
 *				the reserved bytes of rx_tlv_hdr.
 * @buf: start of rx_tlv_hdr
 * @priv_data: hal_wbm_err_desc_info structure
 * @len: length of the private data
 * Return: void
 */
static inline void
hal_rx_priv_info_get_from_tlv_li(uint8_t *buf, uint8_t *priv_data,
			       uint32_t len)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	uint32_t copy_len = (len > RX_PADDING0_BYTES) ?
			    RX_PADDING0_BYTES : len;

	qdf_mem_copy(priv_data, pkt_tlvs->rx_padding0, copy_len);
}

/**
 * hal_rx_get_tlv_size_generic_li() - Get rx packet tlv size
 * @rx_pkt_tlv_size: TLV size for regular RX packets
 * @rx_mon_pkt_tlv_size: TLV size for monitor mode packets
 *
 * Return: size of rx pkt tlv before the actual data
 */
static void hal_rx_get_tlv_size_generic_li(uint16_t *rx_pkt_tlv_size,
					   uint16_t *rx_mon_pkt_tlv_size)
{
	*rx_pkt_tlv_size = RX_PKT_TLVS_LEN;
	*rx_mon_pkt_tlv_size = SIZE_OF_MONITOR_TLV;
}

/**
 * hal_rx_wbm_err_src_get_li() - Get WBM error source from descriptor
 * @ring_desc: ring descriptor
 *
 * Return: wbm error source
 */
uint32_t hal_rx_wbm_err_src_get_li(hal_ring_desc_t ring_desc)
{
	return HAL_WBM2SW_RELEASE_SRC_GET(ring_desc);
}

/**
 * hal_rx_ret_buf_manager_get_li() - Get return buffer manager from ring desc
 * @ring_desc: ring descriptor
 *
 * Return: rbm
 */
uint8_t hal_rx_ret_buf_manager_get_li(hal_ring_desc_t ring_desc)
{
	/*
	 * The following macro takes buf_addr_info as argument,
	 * but since buf_addr_info is the first field in ring_desc
	 * Hence the following call is OK
	 */
	return HAL_RX_BUF_RBM_GET(ring_desc);
}

/**
 * hal_rx_reo_buf_paddr_get_li: Gets the physical address and
 * cookie from the REO destination ring element
 *
 * @ rx_desc: Opaque cookie pointer used by HAL to get to
 * the current descriptor
 * @ buf_info: structure to return the buffer information
 * Return: void
 */
static void hal_rx_reo_buf_paddr_get_li(hal_ring_desc_t rx_desc,
					struct hal_buf_info *buf_info)
{
	struct reo_destination_ring *reo_ring =
		 (struct reo_destination_ring *)rx_desc;

	buf_info->paddr =
	 (HAL_RX_REO_BUFFER_ADDR_31_0_GET(reo_ring) |
	  ((uint64_t)(HAL_RX_REO_BUFFER_ADDR_39_32_GET(reo_ring)) << 32));
	buf_info->sw_cookie = HAL_RX_REO_BUF_COOKIE_GET(reo_ring);
}

/**
 * hal_rx_msdu_link_desc_set_li: Retrieves MSDU Link Descriptor to WBM
 *
 * @ hal_soc_hdl	: HAL version of the SOC pointer
 * @ src_srng_desc	: void pointer to the WBM Release Ring descriptor
 * @ buf_addr_info	: void pointer to the buffer_addr_info
 * @ bm_action		: put in IDLE list or release to MSDU_LIST
 *
 * Return: void
 */
/* look at implementation at dp_hw_link_desc_pool_setup()*/
static void hal_rx_msdu_link_desc_set_li(hal_soc_handle_t hal_soc_hdl,
					 void *src_srng_desc,
					 hal_buff_addrinfo_t buf_addr_info,
					 uint8_t bm_action)
{
	/*
	 * The offsets for fields used in this function are same in
	 * wbm_release_ring for Lithium and wbm_release_ring_tx
	 * for Beryllium. hence we can use wbm_release_ring directly.
	 */
	struct wbm_release_ring *wbm_rel_srng =
			(struct wbm_release_ring *)src_srng_desc;
	uint32_t addr_31_0;
	uint8_t addr_39_32;

	/* Structure copy !!! */
	wbm_rel_srng->released_buff_or_desc_addr_info =
			*(struct buffer_addr_info *)buf_addr_info;

	addr_31_0 =
	wbm_rel_srng->released_buff_or_desc_addr_info.buffer_addr_31_0;
	addr_39_32 =
	wbm_rel_srng->released_buff_or_desc_addr_info.buffer_addr_39_32;

	HAL_DESC_SET_FIELD(src_srng_desc, HAL_SW2WBM_RELEASE_RING,
			   RELEASE_SOURCE_MODULE, HAL_RX_WBM_ERR_SRC_SW);
	HAL_DESC_SET_FIELD(src_srng_desc, HAL_SW2WBM_RELEASE_RING, BM_ACTION,
			   bm_action);
	HAL_DESC_SET_FIELD(src_srng_desc, HAL_SW2WBM_RELEASE_RING,
			   BUFFER_OR_DESC_TYPE,
			   HAL_RX_WBM_BUF_TYPE_MSDU_LINK_DESC);

	/* WBM error is indicated when any of the link descriptors given to
	 * WBM has a NULL address, and one those paths is the link descriptors
	 * released from host after processing RXDMA errors,
	 * or from Rx defrag path, and we want to add an assert here to ensure
	 * host is not releasing descriptors with NULL address.
	 */

	if (qdf_unlikely(!addr_31_0 && !addr_39_32)) {
		hal_dump_wbm_rel_desc(src_srng_desc);
		qdf_assert_always(0);
	}
}

static
void hal_rx_buf_cookie_rbm_get_li(uint32_t *buf_addr_info_hdl,
				  hal_buf_info_t buf_info_hdl)
{
	struct hal_buf_info *buf_info =
		(struct hal_buf_info *)buf_info_hdl;
	struct buffer_addr_info *buf_addr_info =
		(struct buffer_addr_info *)buf_addr_info_hdl;

	buf_info->sw_cookie = HAL_RX_BUF_COOKIE_GET(buf_addr_info);
	/*
	 * buffer addr info is the first member of ring desc, so the typecast
	 * can be done.
	 */
	buf_info->rbm = hal_rx_ret_buf_manager_get_li
				((hal_ring_desc_t)buf_addr_info);
}

/**
 * hal_rx_msdu_link_desc_get(): API to get the MSDU information
 * from the MSDU link descriptor
 *
 * @ hal_soc_hdl	: HAL version of the SOC pointer
 * @msdu_link_desc: Opaque pointer used by HAL to get to the
 * MSDU link descriptor (struct rx_msdu_link)
 *
 * @msdu_list: Return the list of MSDUs contained in this link descriptor
 *
 * @num_msdus: Number of MSDUs in the MPDU
 *
 * Return: void
 */
static inline void hal_rx_msdu_list_get_li(hal_soc_handle_t hal_soc_hdl,
					   void *msdu_link_desc,
					   void *hal_msdu_list,
					   uint16_t *num_msdus)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	struct rx_msdu_details *msdu_details;
	struct rx_msdu_desc_info *msdu_desc_info;
	struct hal_rx_msdu_list *msdu_list = hal_msdu_list;
	struct rx_msdu_link *msdu_link = (struct rx_msdu_link *)msdu_link_desc;
	int i;
	struct hal_buf_info buf_info;

	msdu_details = hal_rx_link_desc_msdu0_ptr(msdu_link, hal_soc);

	hal_debug("msdu_link=%pK msdu_details=%pK", msdu_link, msdu_details);
	for (i = 0; i < HAL_RX_NUM_MSDU_DESC; i++) {
		/* num_msdus received in mpdu descriptor may be incorrect
		 * sometimes due to HW issue. Check msdu buffer address also
		 */
		if (!i && (HAL_RX_BUFFER_ADDR_31_0_GET(
			&msdu_details[i].buffer_addr_info_details) == 0))
			break;
		if (HAL_RX_BUFFER_ADDR_31_0_GET(
			&msdu_details[i].buffer_addr_info_details) == 0) {
			/* set the last msdu bit in the prev msdu_desc_info */
			msdu_desc_info =
				hal_rx_msdu_desc_info_get_ptr
					(&msdu_details[i - 1], hal_soc);
			HAL_RX_LAST_MSDU_IN_MPDU_FLAG_SET(msdu_desc_info, 1);
			break;
		}
		msdu_desc_info = hal_rx_msdu_desc_info_get_ptr(&msdu_details[i],
							       hal_soc);

		/* set first MSDU bit or the last MSDU bit */
		if (!i)
			HAL_RX_FIRST_MSDU_IN_MPDU_FLAG_SET(msdu_desc_info, 1);
		else if (i == (HAL_RX_NUM_MSDU_DESC - 1))
			HAL_RX_LAST_MSDU_IN_MPDU_FLAG_SET(msdu_desc_info, 1);

		msdu_list->msdu_info[i].msdu_flags =
			 hal_rx_msdu_flags_get(hal_soc_hdl, msdu_desc_info);
		msdu_list->msdu_info[i].msdu_len =
			 HAL_RX_MSDU_PKT_LENGTH_GET(msdu_desc_info);

		/* addr field in buf_info will not be valid */
		hal_rx_buf_cookie_rbm_get_li(
				(uint32_t *)
				&msdu_details[i].buffer_addr_info_details,
				&buf_info);
		msdu_list->sw_cookie[i] = buf_info.sw_cookie;
		msdu_list->rbm[i] = buf_info.rbm;
		msdu_list->paddr[i] = HAL_RX_BUFFER_ADDR_31_0_GET(
			   &msdu_details[i].buffer_addr_info_details) |
			   (uint64_t)HAL_RX_BUFFER_ADDR_39_32_GET(
			   &msdu_details[i].buffer_addr_info_details) << 32;
		hal_debug("i=%d sw_cookie=%d", i, msdu_list->sw_cookie[i]);
	}
	*num_msdus = i;
}

/*
 * hal_rxdma_buff_addr_info_set() - set the buffer_addr_info of the
 *				    rxdma ring entry.
 * @rxdma_entry: descriptor entry
 * @paddr: physical address of nbuf data pointer.
 * @cookie: SW cookie used as a index to SW rx desc.
 * @manager: who owns the nbuf (host, NSS, etc...).
 *
 */
static void hal_rxdma_buff_addr_info_set_li(void *rxdma_entry,
			qdf_dma_addr_t paddr, uint32_t cookie, uint8_t manager)
{
	uint32_t paddr_lo = ((u64)paddr & 0x00000000ffffffff);
	uint32_t paddr_hi = ((u64)paddr & 0xffffffff00000000) >> 32;

	HAL_RXDMA_PADDR_LO_SET(rxdma_entry, paddr_lo);
	HAL_RXDMA_PADDR_HI_SET(rxdma_entry, paddr_hi);
	HAL_RXDMA_COOKIE_SET(rxdma_entry, cookie);
	HAL_RXDMA_MANAGER_SET(rxdma_entry, manager);
}

/**
 * hal_rx_get_reo_error_code_li() - Get REO error code from ring desc
 * @rx_desc: rx descriptor
 *
 * Return: REO error code
 */
static uint32_t hal_rx_get_reo_error_code_li(hal_ring_desc_t rx_desc)
{
	struct reo_destination_ring *reo_desc =
			(struct reo_destination_ring *)rx_desc;

	return HAL_RX_REO_ERROR_GET(reo_desc);
}

/**
 * hal_gen_reo_remap_val_generic_li() - Generate the reo map value
 * @ix0_map: mapping values for reo
 *
 * Return: IX0 reo remap register value to be written
 */
static uint32_t
hal_gen_reo_remap_val_generic_li(enum hal_reo_remap_reg remap_reg,
				 uint8_t *ix0_map)
{
	uint32_t ix_val = 0;

	switch (remap_reg) {
	case HAL_REO_REMAP_REG_IX0:
		ix_val = HAL_REO_REMAP_IX0(ix0_map[0], 0) |
			HAL_REO_REMAP_IX0(ix0_map[1], 1) |
			HAL_REO_REMAP_IX0(ix0_map[2], 2) |
			HAL_REO_REMAP_IX0(ix0_map[3], 3) |
			HAL_REO_REMAP_IX0(ix0_map[4], 4) |
			HAL_REO_REMAP_IX0(ix0_map[5], 5) |
			HAL_REO_REMAP_IX0(ix0_map[6], 6) |
			HAL_REO_REMAP_IX0(ix0_map[7], 7);
		break;
	case HAL_REO_REMAP_REG_IX2:
		ix_val = HAL_REO_REMAP_IX2(ix0_map[0], 16) |
			HAL_REO_REMAP_IX2(ix0_map[1], 17) |
			HAL_REO_REMAP_IX2(ix0_map[2], 18) |
			HAL_REO_REMAP_IX2(ix0_map[3], 19) |
			HAL_REO_REMAP_IX2(ix0_map[4], 20) |
			HAL_REO_REMAP_IX2(ix0_map[5], 21) |
			HAL_REO_REMAP_IX2(ix0_map[6], 22) |
			HAL_REO_REMAP_IX2(ix0_map[7], 23);
		break;
	default:
		break;
	}

	return ix_val;
}

/**
 * hal_rx_tlv_csum_err_get_li() - Get IP and tcp-udp checksum fail flag
 * @rx_tlv_hdr: start address of rx_tlv_hdr
 * @ip_csum_err: buffer to return ip_csum_fail flag
 * @tcp_udp_csum_fail: placeholder to return tcp-udp checksum fail flag
 *
 * Return: None
 */
static inline void
hal_rx_tlv_csum_err_get_li(uint8_t *rx_tlv_hdr, uint32_t *ip_csum_err,
			   uint32_t *tcp_udp_csum_err)
{
	*ip_csum_err = hal_rx_attn_ip_cksum_fail_get(rx_tlv_hdr);
	*tcp_udp_csum_err = hal_rx_attn_tcp_udp_cksum_fail_get(rx_tlv_hdr);
}

static
void hal_rx_tlv_get_pkt_capture_flags_li(uint8_t *rx_tlv_pkt_hdr,
					    struct hal_rx_pkt_capture_flags *flags)
{
	struct rx_pkt_tlvs *rx_tlv_hdr = (struct rx_pkt_tlvs *)rx_tlv_pkt_hdr;
	struct rx_attention *rx_attn = &rx_tlv_hdr->attn_tlv.rx_attn;
	struct rx_mpdu_start *mpdu_start =
				&rx_tlv_hdr->mpdu_start_tlv.rx_mpdu_start;
	struct rx_mpdu_end *mpdu_end = &rx_tlv_hdr->mpdu_end_tlv.rx_mpdu_end;
	struct rx_msdu_start *msdu_start =
				&rx_tlv_hdr->msdu_start_tlv.rx_msdu_start;

	flags->encrypt_type = mpdu_start->rx_mpdu_info_details.encrypt_type;
	flags->fcs_err = mpdu_end->fcs_err;
	flags->fragment_flag = rx_attn->fragment_flag;
	flags->chan_freq = HAL_RX_MSDU_START_FREQ_GET(msdu_start);
	flags->rssi_comb = HAL_RX_MSDU_START_RSSI_GET(msdu_start);
	flags->tsft = msdu_start->ppdu_start_timestamp;
}

static uint8_t hal_rx_err_status_get_li(hal_ring_desc_t rx_desc)
{
	return HAL_RX_ERROR_STATUS_GET(rx_desc);
}

static uint8_t hal_rx_reo_buf_type_get_li(hal_ring_desc_t rx_desc)
{
	return HAL_RX_REO_BUF_TYPE_GET(rx_desc);
}

static inline bool
hal_rx_mpdu_info_ampdu_flag_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_start *mpdu_start =
				 &pkt_tlvs->mpdu_start_tlv.rx_mpdu_start;

	struct rx_mpdu_info *mpdu_info = &mpdu_start->rx_mpdu_info_details;
	bool ampdu_flag;

	ampdu_flag = HAL_RX_MPDU_INFO_AMPDU_FLAG_GET(mpdu_info);

	return ampdu_flag;
}

static
uint32_t hal_rx_tlv_mpdu_len_err_get_li(void *hw_desc_addr)
{
	struct rx_attention *rx_attn;
	struct rx_mon_pkt_tlvs *rx_desc =
		(struct rx_mon_pkt_tlvs *)hw_desc_addr;

	rx_attn = &rx_desc->attn_tlv.rx_attn;

	return HAL_RX_GET(rx_attn, RX_ATTENTION_1, MPDU_LENGTH_ERR);
}

static
uint32_t hal_rx_tlv_mpdu_fcs_err_get_li(void *hw_desc_addr)
{
	struct rx_attention *rx_attn;
	struct rx_mon_pkt_tlvs *rx_desc =
		(struct rx_mon_pkt_tlvs *)hw_desc_addr;

	rx_attn = &rx_desc->attn_tlv.rx_attn;

	return HAL_RX_GET(rx_attn, RX_ATTENTION_1, FCS_ERR);
}

#ifdef NO_RX_PKT_HDR_TLV
static uint8_t *hal_rx_desc_get_80211_hdr_li(void *hw_desc_addr)
{
	uint8_t *rx_pkt_hdr;
	struct rx_mon_pkt_tlvs *rx_desc =
		(struct rx_mon_pkt_tlvs *)hw_desc_addr;

	rx_pkt_hdr = &rx_desc->pkt_hdr_tlv.rx_pkt_hdr[0];

	return rx_pkt_hdr;
}
#else
static uint8_t *hal_rx_desc_get_80211_hdr_li(void *hw_desc_addr)
{
	uint8_t *rx_pkt_hdr;

	struct rx_pkt_tlvs *rx_desc = (struct rx_pkt_tlvs *)hw_desc_addr;

	rx_pkt_hdr = &rx_desc->pkt_hdr_tlv.rx_pkt_hdr[0];

	return rx_pkt_hdr;
}
#endif

static uint32_t hal_rx_hw_desc_mpdu_user_id_li(void *hw_desc_addr)
{
	struct rx_mon_pkt_tlvs *rx_desc =
		(struct rx_mon_pkt_tlvs *)hw_desc_addr;
	uint32_t user_id;

	user_id = HAL_RX_GET_USER_TLV32_USERID(
		&rx_desc->mpdu_start_tlv);

	return user_id;
}

/**
 * hal_rx_msdu_start_msdu_len_set_li(): API to set the MSDU length
 * from rx_msdu_start TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * @len: msdu length
 *
 * Return: none
 */
static inline void
hal_rx_msdu_start_msdu_len_set_li(uint8_t *buf, uint32_t len)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
			&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	void *wrd1;

	wrd1 = (uint8_t *)msdu_start + RX_MSDU_START_1_MSDU_LENGTH_OFFSET;
	*(uint32_t *)wrd1 &= (~RX_MSDU_START_1_MSDU_LENGTH_MASK);
	*(uint32_t *)wrd1 |= len;
}

/*
 * hal_rx_tlv_bw_get_li(): API to get the Bandwidth
 * Interval from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(bw)
 */
static inline uint32_t hal_rx_tlv_bw_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t bw;

	bw = HAL_RX_MSDU_START_BW_GET(msdu_start);

	return bw;
}

/*
 * hal_rx_tlv_get_freq_li(): API to get the frequency of operating channel
 * from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(frequency)
 */
static inline uint32_t
hal_rx_tlv_get_freq_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t freq;

	freq = HAL_RX_MSDU_START_FREQ_GET(msdu_start);

	return freq;
}

/**
 * hal_rx_tlv_sgi_get_li(): API to get the Short Gaurd
 * Interval from rx_msdu_start TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(sgi)
 */
static inline uint32_t
hal_rx_tlv_sgi_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t sgi;

	sgi = HAL_RX_MSDU_START_SGI_GET(msdu_start);

	return sgi;
}

/**
 * hal_rx_tlv_rate_mcs_get_li(): API to get the MCS rate
 * from rx_msdu_start TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(rate_mcs)
 */
static inline uint32_t
hal_rx_tlv_rate_mcs_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t rate_mcs;

	rate_mcs = HAL_RX_MSDU_START_RATE_MCS_GET(msdu_start);

	return rate_mcs;
}

/*
 * hal_rx_tlv_get_pkt_type_li(): API to get the pkt type
 * from rx_msdu_start
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(pkt type)
 */

static inline uint32_t hal_rx_tlv_get_pkt_type_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_start *msdu_start =
		&pkt_tlvs->msdu_start_tlv.rx_msdu_start;
	uint32_t pkt_type;

	pkt_type = HAL_RX_MSDU_START_PKT_TYPE_GET(msdu_start);

	return pkt_type;
}

/**
 * hal_rx_tlv_mic_err_get_li(): API to get the MIC ERR
 * from rx_mpdu_end TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(mic_err)
 */
static inline uint32_t
hal_rx_tlv_mic_err_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_end *mpdu_end =
		&pkt_tlvs->mpdu_end_tlv.rx_mpdu_end;
	uint32_t mic_err;

	mic_err = HAL_RX_MPDU_END_MIC_ERR_GET(mpdu_end);

	return mic_err;
}

/**
 * hal_rx_tlv_decrypt_err_get_li(): API to get the Decrypt ERR
 * from rx_mpdu_end TLV
 *
 * @buf: pointer to the start of RX PKT TLV headers
 * Return: uint32_t(decrypt_err)
 */
static inline uint32_t
hal_rx_tlv_decrypt_err_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_mpdu_end *mpdu_end =
		&pkt_tlvs->mpdu_end_tlv.rx_mpdu_end;
	uint32_t decrypt_err;

	decrypt_err = HAL_RX_MPDU_END_DECRYPT_ERR_GET(mpdu_end);

	return decrypt_err;
}

/*
 * hal_rx_tlv_first_mpdu_get_li(): get fist_mpdu bit from rx attention
 * @buf: pointer to rx_pkt_tlvs
 *
 * reutm: uint32_t(first_msdu)
 */
static inline uint32_t
hal_rx_tlv_first_mpdu_get_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	uint32_t first_mpdu;

	first_mpdu = HAL_RX_ATTN_FIRST_MPDU_GET(rx_attn);

	return first_mpdu;
}

/*
 * hal_rx_msdu_get_keyid_li(): API to get the key id if the decrypted packet
 * from rx_msdu_end
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(key id)
 */
static inline uint8_t
hal_rx_msdu_get_keyid_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_msdu_end *msdu_end = &pkt_tlvs->msdu_end_tlv.rx_msdu_end;
	uint32_t keyid_octet;

	keyid_octet = HAL_RX_MSDU_END_KEYID_OCTET_GET(msdu_end);

	return keyid_octet & 0x3;
}

/*
 * hal_rx_tlv_get_is_decrypted_li(): API to get the decrypt status of the
 *  packet from rx_attention
 *
 * @buf: pointer to the start of RX PKT TLV header
 * Return: uint32_t(decryt status)
 */
static inline uint32_t
hal_rx_tlv_get_is_decrypted_li(uint8_t *buf)
{
	struct rx_pkt_tlvs *pkt_tlvs = (struct rx_pkt_tlvs *)buf;
	struct rx_attention *rx_attn = &pkt_tlvs->attn_tlv.rx_attn;
	uint32_t is_decrypt = 0;
	uint32_t decrypt_status;

	decrypt_status = HAL_RX_ATTN_DECRYPT_STATUS_GET(rx_attn);

	if (!decrypt_status)
		is_decrypt = 1;

	return is_decrypt;
}

/**
 * hal_rx_msdu_reo_dst_ind_get_li: Gets the REO
 * destination ring ID from the msdu desc info
 *
 * @ hal_soc_hdl	: HAL version of the SOC pointer
 * @msdu_link_desc : Opaque cookie pointer used by HAL to get to
 * the current descriptor
 *
 * Return: dst_ind (REO destination ring ID)
 */
static inline uint32_t
hal_rx_msdu_reo_dst_ind_get_li(hal_soc_handle_t hal_soc_hdl,
			       void *msdu_link_desc)
{
	struct hal_soc *hal_soc = (struct hal_soc *)hal_soc_hdl;
	struct rx_msdu_details *msdu_details;
	struct rx_msdu_desc_info *msdu_desc_info;
	struct rx_msdu_link *msdu_link = (struct rx_msdu_link *)msdu_link_desc;
	uint32_t dst_ind;

	msdu_details = hal_rx_link_desc_msdu0_ptr(msdu_link, hal_soc);

	/* The first msdu in the link should exsist */
	msdu_desc_info = hal_rx_msdu_desc_info_get_ptr(&msdu_details[0],
						       hal_soc);
	dst_ind = HAL_RX_MSDU_REO_DST_IND_GET(msdu_desc_info);
	return dst_ind;
}

static inline void
hal_mpdu_desc_info_set_li(hal_soc_handle_t hal_soc_hdl,
			  void *mpdu_desc, uint32_t seq_no)
{
	struct rx_mpdu_desc_info *mpdu_desc_info =
			(struct rx_mpdu_desc_info *)mpdu_desc;

	HAL_RX_MPDU_DESC_INFO_SET(mpdu_desc_info,
				  MSDU_COUNT, 0x1);
	HAL_RX_MPDU_DESC_INFO_SET(mpdu_desc_info,
				  MPDU_SEQUENCE_NUMBER, seq_no);
	/* unset frag bit */
	HAL_RX_MPDU_DESC_INFO_SET(mpdu_desc_info,
				  FRAGMENT_FLAG, 0x0);
	/* set sa/da valid bits */
	HAL_RX_MPDU_DESC_INFO_SET(mpdu_desc_info,
				  SA_IS_VALID, 0x1);
	HAL_RX_MPDU_DESC_INFO_SET(mpdu_desc_info,
				  DA_IS_VALID, 0x1);
	HAL_RX_MPDU_DESC_INFO_SET(mpdu_desc_info,
				  RAW_MPDU, 0x0);
}

static inline void
hal_msdu_desc_info_set_li(hal_soc_handle_t hal_soc_hdl,
			  void *msdu_desc, uint32_t dst_ind,
			  uint32_t nbuf_len)
{
	struct rx_msdu_desc_info *msdu_desc_info =
		(struct rx_msdu_desc_info *)msdu_desc;

	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
				  FIRST_MSDU_IN_MPDU_FLAG, 1);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
				  LAST_MSDU_IN_MPDU_FLAG, 1);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
				  MSDU_CONTINUATION, 0x0);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
				  REO_DESTINATION_INDICATION,
				  dst_ind);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
				  MSDU_LENGTH, nbuf_len);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
				  SA_IS_VALID, 1);
	HAL_RX_MSDU_DESC_INFO_SET(msdu_desc_info,
				  DA_IS_VALID, 1);
}

static QDF_STATUS hal_reo_status_update_li(hal_soc_handle_t hal_soc_hdl,
					   hal_ring_desc_t reo_desc,
					   void *st_handle,
					   uint32_t tlv, int *num_ref)
{
	union hal_reo_status *reo_status_ref;

	reo_status_ref = (union hal_reo_status *)st_handle;

	switch (tlv) {
	case HAL_REO_QUEUE_STATS_STATUS_TLV:
		hal_reo_queue_stats_status_li(reo_desc,
					      &reo_status_ref->queue_status,
					      hal_soc_hdl);
		*num_ref = reo_status_ref->queue_status.header.cmd_num;
		break;
	case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
		hal_reo_flush_queue_status_li(reo_desc,
					      &reo_status_ref->fl_queue_status,
					      hal_soc_hdl);
		*num_ref = reo_status_ref->fl_queue_status.header.cmd_num;
		break;
	case HAL_REO_FLUSH_CACHE_STATUS_TLV:
		hal_reo_flush_cache_status_li(reo_desc,
					      &reo_status_ref->fl_cache_status,
					      hal_soc_hdl);
		*num_ref = reo_status_ref->fl_cache_status.header.cmd_num;
		break;
	case HAL_REO_UNBLK_CACHE_STATUS_TLV:
		hal_reo_unblock_cache_status_li(
					reo_desc, hal_soc_hdl,
					&reo_status_ref->unblk_cache_status);
		*num_ref = reo_status_ref->unblk_cache_status.header.cmd_num;
		break;
	case HAL_REO_TIMOUT_LIST_STATUS_TLV:
		hal_reo_flush_timeout_list_status_li(
					reo_desc,
					&reo_status_ref->fl_timeout_status,
					hal_soc_hdl);
		*num_ref = reo_status_ref->fl_timeout_status.header.cmd_num;
		break;
	case HAL_REO_DESC_THRES_STATUS_TLV:
		hal_reo_desc_thres_reached_status_li(
						reo_desc,
						&reo_status_ref->thres_status,
						hal_soc_hdl);
		*num_ref = reo_status_ref->thres_status.header.cmd_num;
		break;
	case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
		hal_reo_rx_update_queue_status_li(
					reo_desc,
					&reo_status_ref->rx_queue_status,
					hal_soc_hdl);
		*num_ref = reo_status_ref->rx_queue_status.header.cmd_num;
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_DP_REO, QDF_TRACE_LEVEL_WARN,
			  "hal_soc %pK: no handler for TLV:%d",
			   hal_soc_hdl, tlv);
		return QDF_STATUS_E_FAILURE;
	} /* switch */

	return QDF_STATUS_SUCCESS;
}

/**
 * hal_hw_txrx_default_ops_attach_li() - Attach the default hal ops for
 *		lithium chipsets.
 * @hal_soc_hdl: HAL soc handle
 *
 * Return: None
 */
void hal_hw_txrx_default_ops_attach_li(struct hal_soc *hal_soc)
{
	hal_soc->ops->hal_get_reo_qdesc_size = hal_get_reo_qdesc_size_li;
	hal_soc->ops->hal_set_link_desc_addr = hal_set_link_desc_addr_li;
	hal_soc->ops->hal_tx_init_data_ring = hal_tx_init_data_ring_li;
	hal_soc->ops->hal_get_ba_aging_timeout = hal_get_ba_aging_timeout_li;
	hal_soc->ops->hal_set_ba_aging_timeout = hal_set_ba_aging_timeout_li;
	hal_soc->ops->hal_get_reo_reg_base_offset =
					hal_get_reo_reg_base_offset_li;
	hal_soc->ops->hal_rx_get_tlv_size = hal_rx_get_tlv_size_generic_li;
	hal_soc->ops->hal_rx_msdu_is_wlan_mcast =
					hal_rx_msdu_is_wlan_mcast_generic_li;
	hal_soc->ops->hal_rx_tlv_decap_format_get =
					hal_rx_tlv_decap_format_get_li;
	hal_soc->ops->hal_rx_dump_pkt_tlvs = hal_rx_dump_pkt_tlvs_li;
	hal_soc->ops->hal_rx_tlv_get_offload_info =
					hal_rx_tlv_get_offload_info_li;
	hal_soc->ops->hal_rx_tlv_phy_ppdu_id_get =
					hal_rx_attn_phy_ppdu_id_get_li;
	hal_soc->ops->hal_rx_tlv_msdu_done_get = hal_rx_attn_msdu_done_get_li;
	hal_soc->ops->hal_rx_tlv_msdu_len_get =
					hal_rx_msdu_start_msdu_len_get_li;
	hal_soc->ops->hal_rx_get_frame_ctrl_field =
						hal_rx_get_frame_ctrl_field_li;
	hal_soc->ops->hal_rx_get_proto_params = hal_rx_get_proto_params_li;
	hal_soc->ops->hal_rx_get_l3_l4_offsets = hal_rx_get_l3_l4_offsets_li;

	hal_soc->ops->hal_rx_reo_buf_paddr_get = hal_rx_reo_buf_paddr_get_li;
	hal_soc->ops->hal_rx_msdu_link_desc_set = hal_rx_msdu_link_desc_set_li;
	hal_soc->ops->hal_rx_buf_cookie_rbm_get = hal_rx_buf_cookie_rbm_get_li;
	hal_soc->ops->hal_rx_ret_buf_manager_get =
						hal_rx_ret_buf_manager_get_li;
	hal_soc->ops->hal_rxdma_buff_addr_info_set =
					hal_rxdma_buff_addr_info_set_li;
	hal_soc->ops->hal_rx_msdu_flags_get = hal_rx_msdu_flags_get_li;
	hal_soc->ops->hal_rx_get_reo_error_code = hal_rx_get_reo_error_code_li;
	hal_soc->ops->hal_gen_reo_remap_val =
				hal_gen_reo_remap_val_generic_li;
	hal_soc->ops->hal_rx_tlv_csum_err_get =
					hal_rx_tlv_csum_err_get_li;
	hal_soc->ops->hal_rx_mpdu_desc_info_get =
					hal_rx_mpdu_desc_info_get_li;
	hal_soc->ops->hal_rx_err_status_get = hal_rx_err_status_get_li;
	hal_soc->ops->hal_rx_reo_buf_type_get = hal_rx_reo_buf_type_get_li;
	hal_soc->ops->hal_rx_pkt_hdr_get = hal_rx_pkt_hdr_get_li;
	hal_soc->ops->hal_rx_wbm_err_src_get = hal_rx_wbm_err_src_get_li;
	hal_soc->ops->hal_rx_priv_info_set_in_tlv =
					hal_rx_priv_info_set_in_tlv_li;
	hal_soc->ops->hal_rx_priv_info_get_from_tlv =
					hal_rx_priv_info_get_from_tlv_li;
	hal_soc->ops->hal_rx_mpdu_info_ampdu_flag_get =
					hal_rx_mpdu_info_ampdu_flag_get_li;
	hal_soc->ops->hal_rx_tlv_mpdu_len_err_get =
					hal_rx_tlv_mpdu_len_err_get_li;
	hal_soc->ops->hal_rx_tlv_mpdu_fcs_err_get =
					hal_rx_tlv_mpdu_fcs_err_get_li;
	hal_soc->ops->hal_reo_send_cmd = hal_reo_send_cmd_li;
	hal_soc->ops->hal_rx_tlv_get_pkt_capture_flags =
					hal_rx_tlv_get_pkt_capture_flags_li;
	hal_soc->ops->hal_rx_desc_get_80211_hdr = hal_rx_desc_get_80211_hdr_li;
	hal_soc->ops->hal_rx_hw_desc_mpdu_user_id =
					hal_rx_hw_desc_mpdu_user_id_li;
	hal_soc->ops->hal_reo_qdesc_setup = hal_reo_qdesc_setup_li;
	hal_soc->ops->hal_rx_tlv_msdu_len_set =
					hal_rx_msdu_start_msdu_len_set_li;
	hal_soc->ops->hal_rx_tlv_bw_get = hal_rx_tlv_bw_get_li;
	hal_soc->ops->hal_rx_tlv_get_freq = hal_rx_tlv_get_freq_li;
	hal_soc->ops->hal_rx_tlv_sgi_get = hal_rx_tlv_sgi_get_li;
	hal_soc->ops->hal_rx_tlv_rate_mcs_get = hal_rx_tlv_rate_mcs_get_li;
	hal_soc->ops->hal_rx_tlv_get_pkt_type = hal_rx_tlv_get_pkt_type_li;
	hal_soc->ops->hal_rx_tlv_get_pn_num = hal_rx_tlv_get_pn_num_li;
	hal_soc->ops->hal_rx_tlv_mic_err_get = hal_rx_tlv_mic_err_get_li;
	hal_soc->ops->hal_rx_tlv_decrypt_err_get =
			hal_rx_tlv_decrypt_err_get_li;
	hal_soc->ops->hal_rx_tlv_first_mpdu_get = hal_rx_tlv_first_mpdu_get_li;
	hal_soc->ops->hal_rx_tlv_get_is_decrypted =
			hal_rx_tlv_get_is_decrypted_li;
	hal_soc->ops->hal_rx_msdu_get_keyid = hal_rx_msdu_get_keyid_li;
	hal_soc->ops->hal_rx_msdu_reo_dst_ind_get =
			hal_rx_msdu_reo_dst_ind_get_li;
	hal_soc->ops->hal_msdu_desc_info_set = hal_msdu_desc_info_set_li;
	hal_soc->ops->hal_mpdu_desc_info_set = hal_mpdu_desc_info_set_li;
	hal_soc->ops->hal_reo_status_update = hal_reo_status_update_li;
	hal_soc->ops->hal_get_tlv_hdr_size = hal_get_tlv_hdr_size_li;
}
