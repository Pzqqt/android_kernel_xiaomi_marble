/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
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

#ifndef _DP_MON_FILTER_2_0_H_
#define _DP_MON_FILTER_2_0_H_

#include <dp_htt.h>

/**
 * struct dp_mon_filter_be - Monitor TLV filter
 * @rx_tlv_filter: Rx MON TLV filter
 * @tx_tlv_filter: Tx MON TLV filter
 * @tx_valid: enable/disable Tx Mon TLV filter
 */
struct dp_mon_filter_be {
	struct dp_mon_filter rx_tlv_filter;
	struct htt_tx_ring_tlv_filter tx_tlv_filter;
	int tx_valid;
};

/**
 * dp_rx_mon_packet_length_set() - Setup rx monitor per packet type length
 * @msg_word: msg word
 * @htt_tlv_filter: rx ring filter configuration
 */
void
dp_rx_mon_packet_length_set(uint32_t *msg_word,
			    struct htt_rx_ring_tlv_filter *tlvi_filter);

/**
 * dp_rx_mon_word_mask_subscribe() - Setup rx monitor word mask subscription
 * @msg_word: msg word
 * @htt_tlv_filter: rx ring filter configuration
 */
void
dp_rx_mon_word_mask_subscribe(uint32_t *msg_word,
			      struct htt_rx_ring_tlv_filter *tlv_filter);

/**
 * dp_rx_mon_enable_mpdu_logging() - Setup rx monitor per packet mpdu logging
 * @msg_word: msg word
 * @htt_tlv_filter: rx ring filter configuration
 */
void
dp_rx_mon_enable_mpdu_logging(uint32_t *msg_word,
			      struct htt_rx_ring_tlv_filter *tlv_filter);

#ifdef QCA_ENHANCED_STATS_SUPPORT
/**
 * dp_mon_filter_setup_enhanced_stats() - Setup the enhanced stats filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_enhanced_stats_2_0(struct dp_pdev *pdev);

/***
 * dp_mon_filter_reset_enhanced_stats() - Reset the enhanced stats filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_enhanced_stats_2_0(struct dp_pdev *pdev);
#else
static inline void
dp_mon_filter_setup_enhanced_stats_2_0(struct dp_pdev *pdev)
{
}

static inline void
dp_mon_filter_reset_enhanced_stats_2_0(struct dp_pdev *pdev)
{
}
#endif

#ifdef QCA_MCOPY_SUPPORT
/**
 * dp_mon_filter_setup_mcopy_mode() - Setup the m_copy mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_mcopy_mode_2_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_mcopy_mode() - Reset the m_copy mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_mcopy_mode_2_0(struct dp_pdev *pdev);
#else
static inline void
dp_mon_filter_setup_mcopy_mode_2_0(struct dp_pdev *pdev)
{
}

static inline void
dp_mon_filter_reset_mcopy_mode_2_0(struct dp_pdev *pdev)
{
}
#endif

#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
/**
 * dp_mon_filter_setup_smart_monitor() - Setup the smart monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_smart_monitor_2_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_smart_monitor() - Reset the smart monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_smart_monitor_2_0(struct dp_pdev *pdev);
#else
static inline void
dp_mon_filter_setup_smart_monitor_2_0(struct dp_pdev *pdev)
{
}

static inline void
dp_mon_filter_reset_smart_monitor_2_0(struct dp_pdev *pdev)
{
}
#endif

#ifdef WLAN_RX_PKT_CAPTURE_ENH
/**
 * dp_mon_filter_setup_rx_enh_capture() - Setup the Rx capture mode filters
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_rx_enh_capture_2_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_rx_enh_capture() - Reset the Rx capture mode filters
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_rx_enh_capture_2_0(struct dp_pdev *pdev);
#else
static inline void
dp_mon_filter_setup_rx_enh_capture_2_0(struct dp_pdev *pdev)
{
}

static inline void
dp_mon_filter_reset_rx_enh_capture_2_0(struct dp_pdev *pdev)
{
}
#endif

/**
 * dp_mon_filter_setup_mon_mode() - Setup the Rx monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_mon_mode_2_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_mon_mode() - Reset the Rx monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_mon_mode_2_0(struct dp_pdev *pdev);

#ifdef WDI_EVENT_ENABLE
/**
 * dp_mon_filter_setup_rx_pkt_log_full() - Setup the Rx pktlog full mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_rx_pkt_log_full_2_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_rx_pkt_log_full_2_0() - Reset pktlog full mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_rx_pkt_log_full_2_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_setup_rx_pkt_log_lite() - Setup the Rx pktlog lite mode filter
 * in the radio object.
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_rx_pkt_log_lite_2_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_rx_pkt_log_lite() - Reset the Rx pktlog lite mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_rx_pkt_log_lite_2_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_setup_rx_pkt_log_cbf() - Setup the Rx pktlog cbf mode filter
 * in the radio object.
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_rx_pkt_log_cbf_2_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_rx_pktlog_cbf() - Reset the Rx pktlog cbf mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_rx_pktlog_cbf_2_0(struct dp_pdev *pdev);
#else
static inline void
dp_mon_filter_setup_rx_pkt_log_full_2_0(struct dp_pdev *pdev)
{
}

static inline void
dp_mon_filter_reset_rx_pkt_log_full_2_0(struct dp_pdev *pdev)
{
}

static inline void
dp_mon_filter_setup_rx_pkt_log_lite_2_0(struct dp_pdev *pdev)
{
}

static inline void
dp_mon_filter_reset_rx_pkt_log_lite_2_0(struct dp_pdev *pdev)
{
}

static inline void
dp_mon_filter_setup_rx_pkt_log_cbf_2_0(struct dp_pdev *pdev)
{
}

static inline void
dp_mon_filter_reset_rx_pktlog_cbf_2_0(struct dp_pdev *pdev)
{
}
#endif

/**
 * dp_mon_filter_update_2_0() - Update monitor filter configuration
 * @pdev: physical device handle
 *
 * Return: non-zero for failure, zero for success
 */
QDF_STATUS dp_mon_filter_update_2_0(struct dp_pdev *pdev);

#endif /* _DP_MON_FILTER_2_0_H_ */
