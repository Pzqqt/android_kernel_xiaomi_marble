/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#ifndef _DP_MON_FILTER_H_
#define _DP_MON_FILTER_H_

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

#define DP_MON_FILTER_GET(src, field) \
	((*((uint32_t *)((uint8_t *)(src) + DP_MON_ ## field ## _OFFSET)) & \
	(DP_MON_ ## field ## _MASK)) >> DP_MON_ ## field ## _LSB) \

#define DP_MON_FILTER_SET(dst, field, value) \
do { \
	uint32_t *val = \
	((uint32_t *)((uint8_t *)(dst) + DP_MON_ ## field ## _OFFSET)); \
	*val &= ~(DP_MON_ ## field ## _MASK); \
	*val |= ((value) << DP_MON_ ## field ## _LSB); \
} while (0)

#define DP_MON_FILTER_PRINT(fmt, args ...) \
	QDF_TRACE(QDF_MODULE_ID_MON_FILTER, QDF_TRACE_LEVEL_DEBUG, \
		  fmt, ## args)
/**
 * struct dp_mon_filter - Monitor TLV filter
 * @valid: enable/disable TLV filter
 * @tlv_filter: Rx ring TLV filter
 */
struct dp_mon_filter {
	bool valid;
	struct htt_rx_ring_tlv_filter tlv_filter;
};

/**
 * enum dp_mon_filter_mode - Different modes for SRNG filters
 * @DP_MON_FILTER_ENHACHED_STATS_MODE: PPDU enhanced stats mode
 * @DP_MON_FILTER_SMART_MONITOR_MODE: Smart monitor mode
 * @DP_MON_FILTER_MCOPY_MODE: AM copy mode
 * @DP_MON_FILTER_MONITOR_MODE: Monitor mode
 * @DP_MON_FILTER_RX_CAPTURE_MODE: Rx Capture mode
 * @DP_MON_FILTER_PKT_LOG_FULL_MODE: Packet log full mode
 * @DP_MON_FILTER_PKT_LOG_LITE_MODE: Packet log lite mode
 */
enum dp_mon_filter_mode {
#ifdef FEATURE_PERPKT_INFO
	DP_MON_FILTER_ENHACHED_STATS_MODE,
	DP_MON_FILTER_MCOPY_MODE,
#endif /* FEATURE_PERPKT_INFO */
#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
	DP_MON_FILTER_SMART_MONITOR_MODE,
#endif /* ATH_SUPPORT_NAC_RSSI || ATH_SUPPORT_NAC */
	DP_MON_FILTER_MONITOR_MODE,
#ifdef	WLAN_RX_PKT_CAPTURE_ENH
	DP_MON_FILTER_RX_CAPTURE_MODE,
#endif /* WLAN_RX_PKT_CAPTURE_ENH */

#ifdef WDI_EVENT_ENABLE
	DP_MON_FILTER_PKT_LOG_FULL_MODE,
	DP_MON_FILTER_PKT_LOG_LITE_MODE,
#endif /* WDI_EVENT_ENABLE */
	DP_MON_FILTER_MAX_MODE
};

/**
 * enum dp_mon_filter_srng_type - Srng types dynamic mode filter
 * settings.
 * @DP_MON_FILTER_SRNG_TYPE_RXDMA_BUF: RXDMA srng type
 * @DP_MON_FILTER_SRNG_TYPE_RXDMA_MONITOR_STATUS: RxDMA monitor status srng
 * @DP_MON_FILTER_SRNG_TYPE_RXDMA_MON_BUF: RxDMA destination srng
 * @DP_MON_FILTER_SRNG_TYPE_MAX: Srng max type
 */
enum dp_mon_filter_srng_type {
	DP_MON_FILTER_SRNG_TYPE_RXDMA_BUF,
	DP_MON_FILTER_SRNG_TYPE_RXDMA_MONITOR_STATUS,
	DP_MON_FILTER_SRNG_TYPE_RXDMA_MON_BUF,
	DP_MON_FILTER_SRNG_TYPE_MAX
};

/**
 * enum dp_mon_filter_action - Action for storing the filters
 * into the radio structure.
 * @DP_MON_FILTER_CLEAR - Clears the filter for a mode
 * @DP_MON_FILTER_SET - Set the filtes for a mode
 */
enum dp_mon_filter_action {
	DP_MON_FILTER_CLEAR,
	DP_MON_FILTER_SET,
};

#ifdef FEATURE_PERPKT_INFO
/**
 * dp_mon_filter_setup_enhanced_stats() - Setup the enhanced stats filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_enhanced_stats(struct dp_pdev *pdev);

/***
 * dp_mon_filter_reset_enhanced_stats() - Reset the enhanced stats filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_enhanced_stats(struct dp_pdev *pdev);

/**
 * dp_mon_filter_setup_mcopy_mode() - Setup the m_copy mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_mcopy_mode(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_mcopy_mode() - Reset the m_copy mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_mcopy_mode(struct dp_pdev *pdev);
#endif /* FEATURE_PERPKT_INFO */

#if defined(ATH_SUPPORT_NAC_RSSI) || defined(ATH_SUPPORT_NAC)
/**
 * dp_mon_filter_setup_smart_monitor() - Setup the smart monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_smart_monitor(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_smart_monitor() - Reset the smart monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_smart_monitor(struct dp_pdev *pdev);
#endif /* ATH_SUPPORT_NAC_RSSI || ATH_SUPPORT_NAC */

#ifdef  WLAN_RX_PKT_CAPTURE_ENH
/**
 * dp_mon_filter_setup_rx_enh_capture() - Setup the Rx capture mode filters
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_rx_enh_capture(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_rx_enh_capture() - Reset the Rx capture mode filters
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_rx_enh_capture(struct dp_pdev *pdev);
#endif /* WLAN_RX_PKT_CAPTURE_ENH */

/**
 * dp_mon_filter_setup_mon_mode() - Setup the Rx monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_mon_mode(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_mon_mode() - Reset the Rx monitor mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_mon_mode(struct dp_pdev *pdev);

#ifdef WDI_EVENT_ENABLE
/**
 * dp_mon_filter_setup_rx_pkt_log_full() - Setup the Rx pktlog full mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_rx_pkt_log_full(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_rx_pkt_log_full() - Reset the Rx pktlog full mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_rx_pkt_log_full(struct dp_pdev *pdev);

/**
 * dp_mon_filter_setup_rx_pkt_log_lite() - Setup the Rx pktlog lite mode filter
 * in the radio object.
 * @pdev: DP pdev handle
 */
void dp_mon_filter_setup_rx_pkt_log_lite(struct dp_pdev *pdev);

/**
 * dp_mon_filter_reset_rx_pkt_log_lite() - Reset the Rx pktlog lite mode filter
 * @pdev: DP pdev handle
 */
void dp_mon_filter_reset_rx_pkt_log_lite(struct dp_pdev *pdev);
#endif /* WDI_EVENT_ENABLE */

/**
 * dp_mon_filter_update() - Setup the monitor filter setting for a srng
 * type
 * @pdev: DP pdev handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_mon_filter_update(struct dp_pdev *pdev);

/**
 * dp_mon_filter_dealloc() - Deallocate the filter objects to be stored in
 * the radio object.
 * @pdev: DP pdev handle
 */
void dp_mon_filter_dealloc(struct dp_pdev *pdev);

/**
 * dp_mon_filter_alloc() - Allocate the filter objects to be stored in
 * the radio object.
 * @pdev: DP pdev handle
 */
struct dp_mon_filter  **dp_mon_filter_alloc(struct dp_pdev *pdev);
#endif /* #ifndef _DP_MON_FILTER_H_ */
