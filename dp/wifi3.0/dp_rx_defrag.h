/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#ifndef _DP_RX_DEFRAG_H
#define _DP_RX_DEFRAG_H

#include "hal_rx.h"

#ifdef CONFIG_MCL
#include <cds_ieee80211_common.h>
#else
#include <linux/ieee80211.h>
#endif

#define DEFRAG_IEEE80211_ADDR_LEN	6
#define DEFRAG_IEEE80211_KEY_LEN	8
#define DEFRAG_IEEE80211_FCS_LEN	4

#define DP_RX_DEFRAG_IEEE80211_ADDR_COPY(dst, src) \
	qdf_mem_copy(dst, src, IEEE80211_ADDR_LEN)

#define DP_RX_DEFRAG_IEEE80211_QOS_HAS_SEQ(wh) \
	(((wh) & \
	(IEEE80211_FC0_TYPE_MASK | IEEE80211_FC0_SUBTYPE_QOS)) == \
	(IEEE80211_FC0_TYPE_DATA | IEEE80211_FC0_SUBTYPE_QOS))

#define UNI_DESC_OWNER_SW 0x1
#define UNI_DESC_BUF_TYPE_RX_MSDU_LINK 0x6
/**
 * struct dp_rx_defrag_cipher: structure to indicate cipher header
 * @ic_name: Name
 * @ic_header: header length
 * @ic_trailer: trail length
 * @ic_miclen: MIC length
 */
struct dp_rx_defrag_cipher {
	const char *ic_name;
	uint16_t ic_header;
	uint8_t ic_trailer;
	uint8_t ic_miclen;
};

uint32_t dp_rx_frag_handle(struct dp_soc *soc, void *ring_desc,
		struct hal_rx_mpdu_desc_info *mpdu_desc_info,
		union dp_rx_desc_list_elem_t **head,
		union dp_rx_desc_list_elem_t **tail,
		uint32_t quota);

/*
 * dp_rx_frag_get_mac_hdr() - Return pointer to the mac hdr
 * @rx_desc_info: Pointer to the pkt_tlvs in the
 * nbuf (pkt_tlvs->mac_hdr->data)
 *
 * It is inefficient to peek into the packet for received
 * frames but these APIs are required to get to some of
 * 802.11 fields that hardware does not populate in the
 * rx meta data.
 *
 * Returns: pointer to ieee80211_frame
 */
static inline
struct ieee80211_frame *dp_rx_frag_get_mac_hdr(uint8_t *rx_desc_info)
{
	int rx_desc_len = hal_rx_get_desc_len();
	return (struct ieee80211_frame *)(rx_desc_info + rx_desc_len);
}

/*
 * dp_rx_frag_get_mpdu_seq_number() - Get mpdu sequence number
 * @rx_desc_info: Pointer to the pkt_tlvs in the
 * nbuf (pkt_tlvs->mac_hdr->data)
 *
 * Returns: uint16_t, rx sequence number
 */
static inline
uint16_t dp_rx_frag_get_mpdu_seq_number(uint8_t *rx_desc_info)
{
	struct ieee80211_frame *mac_hdr;
	mac_hdr = dp_rx_frag_get_mac_hdr(rx_desc_info);

	return qdf_le16_to_cpu(*(uint16_t *) mac_hdr->i_seq) >>
		IEEE80211_SEQ_SEQ_SHIFT;
}

/*
 * dp_rx_frag_get_mpdu_frag_number() - Get mpdu fragment number
 * @rx_desc_info: Pointer to the pkt_tlvs in the
 * nbuf (pkt_tlvs->mac_hdr->data)
 *
 * Returns: uint8_t, receive fragment number
 */
static inline
uint8_t dp_rx_frag_get_mpdu_frag_number(uint8_t *rx_desc_info)
{
	struct ieee80211_frame *mac_hdr;
	mac_hdr = dp_rx_frag_get_mac_hdr(rx_desc_info);

	return qdf_le16_to_cpu(*(uint16_t *) mac_hdr->i_seq) &
		IEEE80211_SEQ_FRAG_MASK;
}

/*
 * dp_rx_frag_get_more_frag_bit() - Get more fragment bit
 * @rx_desc_info: Pointer to the pkt_tlvs in the
 * nbuf (pkt_tlvs->mac_hdr->data)
 *
 * Returns: uint8_t, get more fragment bit
 */
static inline
uint8_t dp_rx_frag_get_more_frag_bit(uint8_t *rx_desc_info)
{
	struct ieee80211_frame *mac_hdr;
	mac_hdr = dp_rx_frag_get_mac_hdr(rx_desc_info);

	return (mac_hdr->i_fc[1] & IEEE80211_FC1_MORE_FRAG) >> 2;
}

static inline
uint8_t dp_rx_get_pkt_dir(uint8_t *rx_desc_info)
{
	struct ieee80211_frame *mac_hdr;
	mac_hdr = dp_rx_frag_get_mac_hdr(rx_desc_info);

	return mac_hdr->i_fc[1] & IEEE80211_FC1_DIR_MASK;
}

void dp_rx_defrag_waitlist_flush(struct dp_soc *soc);
void dp_rx_reorder_flush_frag(struct dp_peer *peer,
			 unsigned int tid);
void dp_rx_defrag_waitlist_remove(struct dp_peer *peer, unsigned tid);
void dp_rx_defrag_cleanup(struct dp_peer *peer, unsigned tid);
#endif /* _DP_RX_DEFRAG_H */
