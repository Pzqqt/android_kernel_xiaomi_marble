/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

/*
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

#ifndef REMOVE_PKT_LOG
#include "ol_txrx_types.h"
#include "ol_htt_tx_api.h"
#include "ol_tx_desc.h"
#include "qdf_mem.h"
#include "htt.h"
#include "htt_internal.h"
#include "pktlog_ac_i.h"
#include "wma_api.h"
#include "wlan_logging_sock_svc.h"

#define TX_DESC_ID_LOW_MASK     0xffff
#define TX_DESC_ID_LOW_SHIFT    0
#define TX_DESC_ID_HIGH_MASK    0xffff0000
#define TX_DESC_ID_HIGH_SHIFT   16

#define PER_PACKET_STATS_THRESHOLD 4096

void pktlog_getbuf_intsafe(struct ath_pktlog_arg *plarg)
{
	struct ath_pktlog_buf *log_buf;
	int32_t buf_size;
	struct ath_pktlog_hdr *log_hdr;
	int32_t cur_wr_offset;
	char *log_ptr;
	struct ath_pktlog_info *pl_info;
	uint16_t log_type;
	size_t log_size;
	uint32_t flags;
#ifdef HELIUMPLUS
	uint8_t mac_id;
#endif

	if (!plarg) {
		printk("Invalid parg in %s\n", __func__);
		return;
	}

	pl_info = plarg->pl_info;
#ifdef HELIUMPLUS
	mac_id = plarg->macId;
	log_type = plarg->log_type;
#else
	log_type = plarg->log_type;
#endif
	log_size = plarg->log_size;
	log_buf = pl_info->buf;
	flags = plarg->flags;

	if (!log_buf) {
		printk("Invalid log_buf in %s\n", __func__);
		return;
	}
	buf_size = pl_info->buf_size;
	cur_wr_offset = log_buf->wr_offset;
	/* Move read offset to the next entry if there is a buffer overlap */
	if (log_buf->rd_offset >= 0) {
		if ((cur_wr_offset <= log_buf->rd_offset)
		    && (cur_wr_offset + sizeof(struct ath_pktlog_hdr)) >
		    log_buf->rd_offset) {
			PKTLOG_MOV_RD_IDX(log_buf->rd_offset, log_buf,
					  buf_size);
		}
	} else {
		log_buf->rd_offset = cur_wr_offset;
	}

	log_hdr = (struct ath_pktlog_hdr *)(log_buf->log_data + cur_wr_offset);

	log_hdr->flags = flags;
#ifdef HELIUMPLUS
	log_hdr->macId = mac_id;
	log_hdr->log_type = log_type;
#else
	log_hdr->log_type = log_type;
#endif
	log_hdr->size = (uint16_t) log_size;
	log_hdr->missed_cnt = plarg->missed_cnt;
	log_hdr->timestamp = plarg->timestamp;
#ifdef HELIUMPLUS
	log_hdr->type_specific_data = plarg->type_specific_data;
#endif
	cur_wr_offset += sizeof(*log_hdr);

	if ((buf_size - cur_wr_offset) < log_size) {
		while ((cur_wr_offset <= log_buf->rd_offset)
		       && (log_buf->rd_offset < buf_size)) {
			PKTLOG_MOV_RD_IDX(log_buf->rd_offset, log_buf,
					  buf_size);
		}
		cur_wr_offset = 0;
	}

	while ((cur_wr_offset <= log_buf->rd_offset)
	       && (cur_wr_offset + log_size) > log_buf->rd_offset) {
		PKTLOG_MOV_RD_IDX(log_buf->rd_offset, log_buf, buf_size);
	}

	log_ptr = &(log_buf->log_data[cur_wr_offset]);
	cur_wr_offset += log_hdr->size;

	log_buf->wr_offset = ((buf_size - cur_wr_offset) >=
			      sizeof(struct ath_pktlog_hdr)) ? cur_wr_offset :
			     0;

	plarg->buf = log_ptr;
}

/**
 * pktlog_check_threshold() - This function checks threshold for triggering
 * packet stats
 * @pl_info: Packet log information pointer
 * @log_size: Size of current packet log information
 *
 * This function internally triggers logging of per packet stats when the
 * incoming data crosses threshold limit
 *
 * Return: None
 *
 */
void pktlog_check_threshold(struct ath_pktlog_info *pl_info,
		size_t log_size)
{
	PKTLOG_LOCK(pl_info);
	pl_info->buf->bytes_written += log_size + sizeof(struct ath_pktlog_hdr);

	if (pl_info->buf->bytes_written >= PER_PACKET_STATS_THRESHOLD) {
		wlan_logging_set_per_pkt_stats();
		pl_info->buf->bytes_written = 0;
	}
	PKTLOG_UNLOCK(pl_info);
}

char *pktlog_getbuf(struct ol_pktlog_dev_t *pl_dev,
		    struct ath_pktlog_info *pl_info,
		    size_t log_size, struct ath_pktlog_hdr *pl_hdr)
{
	struct ath_pktlog_arg plarg = { 0, };
	uint8_t flags = 0;

	plarg.pl_info = pl_info;
#ifdef HELIUMPLUS
	plarg.macId = pl_hdr->macId;
	plarg.log_type = pl_hdr->log_type;
#else
	plarg.log_type = pl_hdr->log_type;
#endif
	plarg.log_size = log_size;
	plarg.flags = pl_hdr->flags;
	plarg.missed_cnt = pl_hdr->missed_cnt;
	plarg.timestamp = pl_hdr->timestamp;
#ifdef HELIUMPLUS
	plarg.type_specific_data = pl_hdr->type_specific_data;
#endif
	if (flags & PHFLAGS_INTERRUPT_CONTEXT) {
		/*
		 * We are already in interupt context, no need to make it
		 * intsafe. call the function directly.
		 */
		pktlog_getbuf_intsafe(&plarg);
	} else {
		PKTLOG_LOCK(pl_info);
		pktlog_getbuf_intsafe(&plarg);
		PKTLOG_UNLOCK(pl_info);
	}

	/*
	 * We do not want to do this packet stats related processing when
	 * packet log tool is run. i.e., we want this processing to be
	 * done only when start logging command of packet stats is initiated.
	 */
	if (cds_get_ring_log_level(RING_ID_PER_PACKET_STATS) ==
			WLAN_LOG_LEVEL_ACTIVE)
		pktlog_check_threshold(pl_info, log_size);

	return plarg.buf;
}

static struct txctl_frm_hdr frm_hdr;

static void process_ieee_hdr(void *data)
{
	uint8_t dir;
	struct ieee80211_frame *wh = (struct ieee80211_frame *)(data);

	frm_hdr.framectrl = *(uint16_t *) (wh->i_fc);
	frm_hdr.seqctrl = *(uint16_t *) (wh->i_seq);
	dir = (wh->i_fc[1] & IEEE80211_FC1_DIR_MASK);

	if (dir == IEEE80211_FC1_DIR_TODS) {
		frm_hdr.bssid_tail =
			(wh->i_addr1[IEEE80211_ADDR_LEN - 2] << 8) | (wh->
								      i_addr1
								      [IEEE80211_ADDR_LEN
								       - 1]);
		frm_hdr.sa_tail =
			(wh->i_addr2[IEEE80211_ADDR_LEN - 2] << 8) | (wh->
								      i_addr2
								      [IEEE80211_ADDR_LEN
								       - 1]);
		frm_hdr.da_tail =
			(wh->i_addr3[IEEE80211_ADDR_LEN - 2] << 8) | (wh->
								      i_addr3
								      [IEEE80211_ADDR_LEN
								       - 1]);
	} else if (dir == IEEE80211_FC1_DIR_FROMDS) {
		frm_hdr.bssid_tail =
			(wh->i_addr2[IEEE80211_ADDR_LEN - 2] << 8) | (wh->
								      i_addr2
								      [IEEE80211_ADDR_LEN
								       - 1]);
		frm_hdr.sa_tail =
			(wh->i_addr3[IEEE80211_ADDR_LEN - 2] << 8) | (wh->
								      i_addr3
								      [IEEE80211_ADDR_LEN
								       - 1]);
		frm_hdr.da_tail =
			(wh->i_addr1[IEEE80211_ADDR_LEN - 2] << 8) | (wh->
								      i_addr1
								      [IEEE80211_ADDR_LEN
								       - 1]);
	} else {
		frm_hdr.bssid_tail =
			(wh->i_addr3[IEEE80211_ADDR_LEN - 2] << 8) | (wh->
								      i_addr3
								      [IEEE80211_ADDR_LEN
								       - 1]);
		frm_hdr.sa_tail =
			(wh->i_addr2[IEEE80211_ADDR_LEN - 2] << 8) | (wh->
								      i_addr2
								      [IEEE80211_ADDR_LEN
								       - 1]);
		frm_hdr.da_tail =
			(wh->i_addr1[IEEE80211_ADDR_LEN - 2] << 8) | (wh->
								      i_addr1
								      [IEEE80211_ADDR_LEN
								       - 1]);
	}
}

/**
 * fill_ieee80211_hdr_data() - fill ieee802.11 data header
 * @txrx_pdev: txrx pdev
 * @pl_msdu_info: msdu info
 * @data: data received from event
 *
 * Return: none
 */
void
fill_ieee80211_hdr_data(struct ol_txrx_pdev_t *txrx_pdev,
	struct ath_pktlog_msdu_info *pl_msdu_info, void *data)
{
	uint32_t i;
	uint32_t *htt_tx_desc;
	struct ol_tx_desc_t *tx_desc;
	uint8_t msdu_id_offset = MSDU_ID_INFO_ID_OFFSET;
	uint16_t tx_desc_id;
	uint32_t *msdu_id_info = (uint32_t *)
				 ((void *)data + sizeof(struct ath_pktlog_hdr));
	uint32_t *msdu_id = (uint32_t *) ((char *)msdu_id_info +
					  msdu_id_offset);
	uint8_t *addr, *vap_addr;
	uint8_t vdev_id;
	qdf_nbuf_t netbuf;
	uint32_t len;


	pl_msdu_info->num_msdu = *msdu_id_info;
	pl_msdu_info->priv_size = sizeof(uint32_t) *
				 pl_msdu_info->num_msdu + sizeof(uint32_t);

	for (i = 0; i < pl_msdu_info->num_msdu; i++) {
		/*
		 * Handle big endianness
		 * Increment msdu_id once after retrieving
		 * lower 16 bits and uppper 16 bits
		 */
		if (!(i % 2)) {
			tx_desc_id = ((*msdu_id & TX_DESC_ID_LOW_MASK)
				      >> TX_DESC_ID_LOW_SHIFT);
		} else {
			tx_desc_id = ((*msdu_id & TX_DESC_ID_HIGH_MASK)
				      >> TX_DESC_ID_HIGH_SHIFT);
			msdu_id += 1;
		}
		tx_desc = ol_tx_desc_find(txrx_pdev, tx_desc_id);
		qdf_assert(tx_desc);
		netbuf = tx_desc->netbuf;
		htt_tx_desc = (uint32_t *) tx_desc->htt_tx_desc;
		qdf_assert(htt_tx_desc);

		qdf_nbuf_peek_header(netbuf, &addr, &len);

		if (len < (2 * IEEE80211_ADDR_LEN)) {
			qdf_print("TX frame does not have a valid address\n");
			return;
		}
		/* Adding header information for the TX data frames */
		vdev_id = (uint8_t) (*(htt_tx_desc +
				       HTT_TX_VDEV_ID_WORD) >>
				     HTT_TX_VDEV_ID_SHIFT) &
			  HTT_TX_VDEV_ID_MASK;

		vap_addr = wma_get_vdev_address_by_vdev_id(vdev_id);

		frm_hdr.da_tail = (addr[IEEE80211_ADDR_LEN - 2] << 8) |
				  (addr[IEEE80211_ADDR_LEN - 1]);
		frm_hdr.sa_tail =
			(addr[2 * IEEE80211_ADDR_LEN - 2] << 8) |
			(addr[2 * IEEE80211_ADDR_LEN - 1]);
		if (vap_addr) {
			frm_hdr.bssid_tail =
				(vap_addr[IEEE80211_ADDR_LEN - 2] << 8) |
				(vap_addr[IEEE80211_ADDR_LEN - 1]);
		} else {
			frm_hdr.bssid_tail = 0x0000;
		}
		pl_msdu_info->priv.msdu_len[i] = *(htt_tx_desc +
						  HTT_TX_MSDU_LEN_DWORD)
						& HTT_TX_MSDU_LEN_MASK;
		/*
		 * Add more information per MSDU
		 * e.g., protocol information
		 */
	}

}

A_STATUS process_tx_info(struct ol_txrx_pdev_t *txrx_pdev, void *data)
{
	/*
	 * Must include to process different types
	 * TX_CTL, TX_STATUS, TX_MSDU_ID, TX_FRM_HDR
	 */
	struct ol_pktlog_dev_t *pl_dev;
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_info *pl_info;
	uint32_t *pl_tgt_hdr;

	if (!txrx_pdev) {
		printk("Invalid pdev in %s\n", __func__);
		return A_ERROR;
	}
	qdf_assert(txrx_pdev->pl_dev);
	qdf_assert(data);
	pl_dev = txrx_pdev->pl_dev;

	pl_tgt_hdr = (uint32_t *) data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
#ifdef HELIUMPLUS
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
		   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
		  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.macId = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MAC_ID_OFFSET) &
		   ATH_PKTLOG_HDR_MAC_ID_MASK) >>
		  ATH_PKTLOG_HDR_MAC_ID_SHIFT;
#else
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
#endif
	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);
#ifdef HELIUMPLUS
	pl_hdr.type_specific_data =
		*(pl_tgt_hdr + ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET);
#endif
	pl_info = pl_dev->pl_info;

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_FRM_HDR) {
		/* Valid only for the TX CTL */
		process_ieee_hdr(data + sizeof(pl_hdr));
	}

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_VIRT_ADDR) {
		A_UINT32 desc_id = (A_UINT32)
				   *((A_UINT32 *) (data + sizeof(pl_hdr)));
		A_UINT32 vdev_id = desc_id;

		/* if the pkt log msg is for the bcn frame the vdev id
		 * is piggybacked in desc_id and the MSB of the desc ID
		 * would be set to FF
		 */
#define BCN_DESC_ID 0xFF
		if ((desc_id >> 24) == BCN_DESC_ID) {
			void *data;
			A_UINT32 buf_size;

			vdev_id &= 0x00FFFFFF;
			data = wma_get_beacon_buffer_by_vdev_id(vdev_id,
								&buf_size);
			if (data) {
				process_ieee_hdr(data);
				qdf_mem_free(data);
			}
		} else {
			/*
			 * TODO: get the hdr content for mgmt frames from
			 * Tx mgmt desc pool
			 */
		}
	}

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_CTRL) {
#if !defined(HELIUMPLUS)
		struct ath_pktlog_txctl txctl_log;
		size_t log_size = sizeof(txctl_log.priv);

		txctl_log.txdesc_hdr_ctl = (void *)pktlog_getbuf(pl_dev,
								 pl_info,
								 log_size,
								 &pl_hdr);

		if (!txctl_log.txdesc_hdr_ctl) {
			printk
				("failed to get buf for txctl_log.txdesc_hdr_ctl\n");
			return A_ERROR;
		}

		/*
		 * frm hdr is currently Valid only for local frames
		 * Add capability to include the fmr hdr for remote frames
		 */
		txctl_log.priv.frm_hdr = frm_hdr;
		qdf_assert(txctl_log.priv.txdesc_ctl);
		qdf_mem_copy((void *)&txctl_log.priv.txdesc_ctl,
			     ((void *)data + sizeof(struct ath_pktlog_hdr)),
			     pl_hdr.size);
		qdf_assert(txctl_log.txdesc_hdr_ctl);
		qdf_mem_copy(txctl_log.txdesc_hdr_ctl, &txctl_log.priv,
			     sizeof(txctl_log.priv));
		/* Add Protocol information and HT specific information */
#else
		size_t log_size = sizeof(frm_hdr) + pl_hdr.size;
		void *txdesc_hdr_ctl = (void *)
		pktlog_getbuf(pl_dev, pl_info, log_size, &pl_hdr);
		qdf_assert(txdesc_hdr_ctl);
		qdf_assert(pl_hdr.size < (370 * sizeof(u_int32_t)));

		qdf_mem_copy(txdesc_hdr_ctl, &frm_hdr, sizeof(frm_hdr));
		qdf_mem_copy((char *)txdesc_hdr_ctl + sizeof(frm_hdr),
					((void *)data + sizeof(struct ath_pktlog_hdr)),
					 pl_hdr.size);
#endif /* !defined(HELIUMPLUS) */
	}

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_STAT) {
		struct ath_pktlog_tx_status txstat_log;
		size_t log_size = pl_hdr.size;

		txstat_log.ds_status = (void *)
				       pktlog_getbuf(pl_dev, pl_info, log_size, &pl_hdr);
		qdf_assert(txstat_log.ds_status);
		qdf_mem_copy(txstat_log.ds_status,
			     ((void *)data + sizeof(struct ath_pktlog_hdr)),
			     pl_hdr.size);
	}

	if (pl_hdr.log_type == PKTLOG_TYPE_TX_MSDU_ID) {
		struct ath_pktlog_msdu_info pl_msdu_info;
		size_t log_size;

		qdf_mem_set(&pl_msdu_info, sizeof(pl_msdu_info), 0);
		log_size = sizeof(pl_msdu_info.priv);

		if (pl_dev->mt_pktlog_enabled == false)
			fill_ieee80211_hdr_data(txrx_pdev, &pl_msdu_info, data);

		pl_msdu_info.ath_msdu_info = pktlog_getbuf(pl_dev, pl_info,
							   log_size, &pl_hdr);
		qdf_mem_copy((void *)&pl_msdu_info.priv.msdu_id_info,
			     ((void *)data + sizeof(struct ath_pktlog_hdr)),
			     sizeof(pl_msdu_info.priv.msdu_id_info));
		qdf_mem_copy(pl_msdu_info.ath_msdu_info, &pl_msdu_info.priv,
			     sizeof(pl_msdu_info.priv));
	}
	return A_OK;
}

A_STATUS process_rx_info_remote(void *pdev, void *data)
{
	struct ol_pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	struct htt_host_rx_desc_base *rx_desc;
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_rx_info rxstat_log;
	size_t log_size;
	struct ol_rx_remote_data *r_data = (struct ol_rx_remote_data *)data;
	qdf_nbuf_t msdu;

	if (!pdev) {
		printk("Invalid pdev in %s\n", __func__);
		return A_ERROR;
	}
	if (!r_data) {
		printk("Invalid data in %s\n", __func__);
		return A_ERROR;
	}
	pl_dev = ((struct ol_txrx_pdev_t *)pdev)->pl_dev;
	pl_info = pl_dev->pl_info;
	msdu = r_data->msdu;

	while (msdu) {
		rx_desc =
		   (struct htt_host_rx_desc_base *)(qdf_nbuf_data(msdu)) - 1;
		log_size =
			sizeof(*rx_desc) - sizeof(struct htt_host_fw_desc_base);

		/*
		 * Construct the pktlog header pl_hdr
		 * Because desc is DMA'd to the host memory
		 */
		pl_hdr.flags = (1 << PKTLOG_FLG_FRM_TYPE_REMOTE_S);
		pl_hdr.missed_cnt = 0;
#if defined(HELIUMPLUS)
		pl_hdr.macId = r_data->mac_id;
		pl_hdr.log_type = PKTLOG_TYPE_RX_STAT;
#else
		pl_hdr.log_type = PKTLOG_TYPE_RX_STAT;
#endif
		pl_hdr.size = sizeof(*rx_desc) -
			      sizeof(struct htt_host_fw_desc_base);
#if defined(HELIUMPLUS)
		pl_hdr.timestamp =
			rx_desc->ppdu_end.rx_pkt_end.phy_timestamp_1_lower_32;
		pl_hdr.type_specific_data = 0xDEADAA;
#else
		pl_hdr.timestamp = rx_desc->ppdu_end.tsf_timestamp;
#endif /* !defined(HELIUMPLUS) */
		rxstat_log.rx_desc = (void *)pktlog_getbuf(pl_dev, pl_info,
							   log_size, &pl_hdr);
		qdf_mem_copy(rxstat_log.rx_desc, (void *)rx_desc +
			     sizeof(struct htt_host_fw_desc_base), pl_hdr.size);
		msdu = qdf_nbuf_next(msdu);
	}
	return A_OK;
}

A_STATUS process_rx_info(void *pdev, void *data)
{
	struct ol_pktlog_dev_t *pl_dev;
	struct ath_pktlog_info *pl_info;
	struct ath_pktlog_rx_info rxstat_log;
	struct ath_pktlog_hdr pl_hdr;
	size_t log_size;
	uint32_t *pl_tgt_hdr;

	if (!pdev) {
		printk("Invalid pdev in %s", __func__);
		return A_ERROR;
	}
	pl_dev = ((struct ol_txrx_pdev_t *)pdev)->pl_dev;
	pl_info = pl_dev->pl_info;
	pl_tgt_hdr = (uint32_t *) data;
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
#ifdef HELIUMPLUS
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.macId = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MAC_ID_OFFSET) &
			   ATH_PKTLOG_HDR_MAC_ID_MASK) >>
			  ATH_PKTLOG_HDR_MAC_ID_SHIFT;
#else
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
				   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
				  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
#endif

	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);
	log_size = pl_hdr.size;
	rxstat_log.rx_desc = (void *)pktlog_getbuf(pl_dev, pl_info,
						   log_size, &pl_hdr);

	qdf_mem_copy(rxstat_log.rx_desc,
		     (void *)data + sizeof(struct ath_pktlog_hdr), pl_hdr.size);

	return A_OK;
}

A_STATUS process_rate_find(void *pdev, void *data)
{
	struct ol_pktlog_dev_t *pl_dev;
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_info *pl_info;
	size_t log_size;

	/*
	 * Will be uncommented when the rate control find
	 * for pktlog is implemented in the firmware.
	 * Currently derived from the TX PPDU status
	 */
	struct ath_pktlog_rc_find rcf_log;
	uint32_t *pl_tgt_hdr;

	if (!pdev) {
		qdf_print("Invalid pdev in %s\n", __func__);
		return A_ERROR;
	}
	if (!data) {
		qdf_print("Invalid data in %s\n", __func__);
		return A_ERROR;
	}

	pl_tgt_hdr = (uint32_t *) data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
#ifdef HELIUMPLUS
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.macId = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MAC_ID_OFFSET) &
			   ATH_PKTLOG_HDR_MAC_ID_MASK) >>
			  ATH_PKTLOG_HDR_MAC_ID_SHIFT;
#else
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
#endif

	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);
	pl_dev = ((struct ol_txrx_pdev_t *)pdev)->pl_dev;
	pl_info = pl_dev->pl_info;
	log_size = pl_hdr.size;
	rcf_log.rcFind = (void *)pktlog_getbuf(pl_dev, pl_info,
					       log_size, &pl_hdr);

	qdf_mem_copy(rcf_log.rcFind,
				 ((char *)data + sizeof(struct ath_pktlog_hdr)),
				 pl_hdr.size);

	return A_OK;
}

A_STATUS process_sw_event(void *pdev, void *data)
{
	struct ol_pktlog_dev_t *pl_dev;
	struct ath_pktlog_hdr pl_hdr;
	struct ath_pktlog_info *pl_info;
	size_t log_size;

	/*
	 * Will be uncommented when the rate control find
	 * for pktlog is implemented in the firmware.
	 * Currently derived from the TX PPDU status
	 */
	struct ath_pktlog_sw_event sw_event;
	uint32_t *pl_tgt_hdr;

	if (!pdev) {
		qdf_print("Invalid pdev in %s\n", __func__);
		return A_ERROR;
	}
	if (!data) {
		qdf_print("Invalid data in %s\n", __func__);
		return A_ERROR;
	}

	pl_tgt_hdr = (uint32_t *) data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
#ifdef HELIUMPLUS
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.macId = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MAC_ID_OFFSET) &
			   ATH_PKTLOG_HDR_MAC_ID_MASK) >>
			  ATH_PKTLOG_HDR_MAC_ID_SHIFT;
#else
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
				   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
				  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
#endif

	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);

#ifdef HELIUMPLUS
	pl_hdr.type_specific_data =
		*(pl_tgt_hdr + ATH_PKTLOG_HDR_TYPE_SPECIFIC_DATA_OFFSET);
#endif

	pl_dev = ((struct ol_txrx_pdev_t *)pdev)->pl_dev;
	pl_info = pl_dev->pl_info;
	log_size = pl_hdr.size;
	sw_event.sw_event = (void *)pktlog_getbuf(pl_dev, pl_info,
					       log_size, &pl_hdr);

	qdf_mem_copy(sw_event.sw_event,
				 ((char *)data + sizeof(struct ath_pktlog_hdr)),
				 pl_hdr.size);

	return A_OK;
}

A_STATUS process_rate_update(void *pdev, void *data)
{
	struct ol_pktlog_dev_t *pl_dev;
	struct ath_pktlog_hdr pl_hdr;
	size_t log_size;
	struct ath_pktlog_info *pl_info;
	struct ath_pktlog_rc_update rcu_log;
	uint32_t *pl_tgt_hdr;

	if (!pdev) {
		printk("Invalid pdev in %s\n", __func__);
		return A_ERROR;
	}
	if (!data) {
		printk("Invalid data in %s\n", __func__);
		return A_ERROR;
	}
	pl_tgt_hdr = (uint32_t *) data;
	/*
	 * Makes the short words (16 bits) portable b/w little endian
	 * and big endian
	 */
	pl_hdr.flags = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_FLAGS_OFFSET) &
			ATH_PKTLOG_HDR_FLAGS_MASK) >>
		       ATH_PKTLOG_HDR_FLAGS_SHIFT;
	pl_hdr.missed_cnt = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MISSED_CNT_OFFSET) &
			     ATH_PKTLOG_HDR_MISSED_CNT_MASK) >>
			    ATH_PKTLOG_HDR_MISSED_CNT_SHIFT;
#ifdef HELIUMPLUS
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
			   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
			  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
	pl_hdr.macId = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_MAC_ID_OFFSET) &
			   ATH_PKTLOG_HDR_MAC_ID_MASK) >>
			  ATH_PKTLOG_HDR_MAC_ID_SHIFT;
#else
	pl_hdr.log_type = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_LOG_TYPE_OFFSET) &
				   ATH_PKTLOG_HDR_LOG_TYPE_MASK) >>
				  ATH_PKTLOG_HDR_LOG_TYPE_SHIFT;
#endif

	pl_hdr.size = (*(pl_tgt_hdr + ATH_PKTLOG_HDR_SIZE_OFFSET) &
		       ATH_PKTLOG_HDR_SIZE_MASK) >> ATH_PKTLOG_HDR_SIZE_SHIFT;
	pl_hdr.timestamp = *(pl_tgt_hdr + ATH_PKTLOG_HDR_TIMESTAMP_OFFSET);
	pl_dev = ((struct ol_txrx_pdev_t *)pdev)->pl_dev;
	log_size = pl_hdr.size;
	pl_info = pl_dev->pl_info;

	/*
	 * Will be uncommented when the rate control update
	 * for pktlog is implemented in the firmware.
	 * Currently derived from the TX PPDU status
	 */
	rcu_log.txRateCtrl = (void *)pktlog_getbuf(pl_dev, pl_info,
						   log_size, &pl_hdr);
	qdf_mem_copy(rcu_log.txRateCtrl,
		     ((char *)data + sizeof(struct ath_pktlog_hdr)),
		     pl_hdr.size);
	return A_OK;
}
#endif /*REMOVE_PKT_LOG */
