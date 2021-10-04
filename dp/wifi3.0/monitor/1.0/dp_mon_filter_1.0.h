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
#ifndef _DP_MON_FILTER_1_0_H_
#define _DP_MON_FILTER_1_0_H_

/**
 * Accessor Macros to access the software
 * defined HTT filter htt_rx_ring_tlv_filter.
 */
#define DP_MON_FILTER_TLV_OFFSET                      0x00000000
#define DP_MON_FILTER_TLV_MASK                        0xffffffff
#define DP_MON_FILTER_TLV_LSB                         0

#define DP_MON_FILTER_FP_MGMT_OFFSET                  0x00000004
#define DP_MON_FILTER_FP_MGMT_MASK                    0x0000ffff
#define DP_MON_FILTER_FP_MGMT_LSB                     0

#define DP_MON_FILTER_MO_MGMT_OFFSET                  0x00000004
#define DP_MON_FILTER_MO_MGMT_MASK                    0xffff0000
#define DP_MON_FILTER_MO_MGMT_LSB                     16

#define DP_MON_FILTER_FP_CTRL_OFFSET                  0x00000008
#define DP_MON_FILTER_FP_CTRL_MASK                    0x0000ffff
#define DP_MON_FILTER_FP_CTRL_LSB                     0

#define DP_MON_FILTER_MO_CTRL_OFFSET                  0x00000008
#define DP_MON_FILTER_MO_CTRL_MASK                    0xffff0000
#define DP_MON_FILTER_MO_CTRL_LSB                     16

#define DP_MON_FILTER_FP_DATA_OFFSET                  0x0000000c
#define DP_MON_FILTER_FP_DATA_MASK                    0x0000ffff
#define DP_MON_FILTER_FP_DATA_LSB                     0

#define DP_MON_FILTER_MO_DATA_OFFSET                  0x0000000c
#define DP_MON_FILTER_MO_DATA_MASK                    0xffff0000
#define DP_MON_FILTER_MO_DATA_LSB                     16

#define DP_MON_FILTER_MD_DATA_OFFSET                  0x00000010
#define DP_MON_FILTER_MD_DATA_MASK                    0x0000ffff
#define DP_MON_FILTER_MD_DATA_LSB                     0

#define DP_MON_FILTER_MD_MGMT_OFFSET                  0x00000010
#define DP_MON_FILTER_MD_MGMT_MASK                    0xffff0000
#define DP_MON_FILTER_MD_MGMT_LSB                     16

#define DP_MON_FILTER_MD_CTRL_OFFSET                  0x00000014
#define DP_MON_FILTER_MD_CTRL_MASK                    0x0000ffff
#define DP_MON_FILTER_MD_CTRL_LSB                     0

#ifdef QCA_ENHANCED_STATS_SUPPORT
/**
 * dp_mon_filter_setup_enhanced_stats() - Setup the enhanced stats filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_enhanced_stats_1_0(struct dp_pdev *pdev);

/***
 * dp_mon_filter_reset_enhanced_stats() - Reset the enhanced stats filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_enhanced_stats_1_0(struct dp_pdev *pdev);
#else
static inline void dp_mon_filter_setup_enhanced_stats_1_0(struct dp_pdev *pdev)
{
}

static inline void dp_mon_filter_reset_enhanced_stats_1_0(struct dp_pdev *pdev)
{
}
#endif

#ifdef QCA_MCOPY_SUPPORT
/**
 * dp_mon_filter_setup_mcopy_mode() - Setup the m_copy mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_mcopy_mode_1_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_mcopy_mode() - Reset the m_copy mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_mcopy_mode_1_0(struct dp_pdev *pdev);
#else
static inline void dp_mon_filter_setup_mcopy_mode_1_0(struct dp_pdev *pdev)
{
}

static inline void dp_mon_filter_reset_mcopy_mode_1_0(struct dp_pdev *pdev)
{
}
#endif

#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
/**
 * dp_mon_filter_setup_smart_monitor() - Setup the smart monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_smart_monitor_1_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_smart_monitor() - Reset the smart monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_smart_monitor_1_0(struct dp_pdev *pdev);
#else
static inline void dp_mon_filter_setup_smart_monitor_1_0(struct dp_pdev *pdev)
{
}

static inline void dp_mon_filter_reset_smart_monitor_1_0(struct dp_pdev *pdev)
{
}
#endif

#ifdef WLAN_RX_PKT_CAPTURE_ENH
/**
 * dp_mon_filter_setup_rx_enh_capture() - Setup the Rx capture mode filters
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_rx_enh_capture_1_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_rx_enh_capture() - Reset the Rx capture mode filters
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_rx_enh_capture_1_0(struct dp_pdev *pdev);
#else
static inline void dp_mon_filter_setup_rx_enh_capture_1_0(struct dp_pdev *pdev)
{
}

static inline void dp_mon_filter_reset_rx_enh_capture_1_0(struct dp_pdev *pdev)
{
}
#endif

/**
 * dp_mon_filter_setup_mon_mode() - Setup the Rx monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_mon_mode_1_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_mon_mode() - Reset the Rx monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_mon_mode_1_0(struct dp_pdev *pdev);

#ifdef WDI_EVENT_ENABLE
/**
 * dp_mon_filter_setup_rx_pkt_log_full() - Setup the Rx pktlog full mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_rx_pkt_log_full_1_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_rx_pkt_log_full_1_0() - Reset the Rx pktlog full mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_rx_pkt_log_full_1_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_setup_rx_pkt_log_lite() - Setup the Rx pktlog lite mode filter
 * in the radio object.
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_rx_pkt_log_lite_1_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_rx_pkt_log_lite() - Reset the Rx pktlog lite mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_rx_pkt_log_lite_1_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_setup_rx_pkt_log_cbf() - Setup the Rx pktlog cbf mode filter
 * in the radio object.
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_rx_pkt_log_cbf_1_0(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_rx_pktlog_cbf() - Reset the Rx pktlog cbf mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_rx_pktlog_cbf_1_0(struct dp_pdev *pdev);
#else
static inline void dp_mon_filter_setup_rx_pkt_log_full_1_0(struct dp_pdev *pdev)
{
}

static inline void dp_mon_filter_reset_rx_pkt_log_full_1_0(struct dp_pdev *pdev)
{
}

static inline void dp_mon_filter_setup_rx_pkt_log_lite_1_0(struct dp_pdev *pdev)
{
}

static inline void dp_mon_filter_reset_rx_pkt_log_lite_1_0(struct dp_pdev *pdev)
{
}

static inline void dp_mon_filter_setup_rx_pkt_log_cbf_1_0(struct dp_pdev *pdev)
{
}

static inline void dp_mon_filter_reset_rx_pktlog_cbf_1_0(struct dp_pdev *pdev)
{
}
#endif

QDF_STATUS dp_mon_filter_update_1_0(struct dp_pdev *pdev);

#endif /* _DP_MON_FILTER_1_0_H_ */
