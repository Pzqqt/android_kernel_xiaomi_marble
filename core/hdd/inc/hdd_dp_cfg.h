/*
 * Copyright (c) 2012-2019 The Linux Foundation. All rights reserved.
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

/**
 * DOC: This file contains centralized definitions of converged configuration.
 */

#ifndef __HDD_DP_CONFIG_H
#define __HDD_DP_CONFIG_H

#define CFG_ENABLE_RX_THREAD		BIT(0)
#define CFG_ENABLE_RPS			BIT(1)
#define CFG_ENABLE_NAPI			BIT(2)
#define CFG_ENABLE_DYNAMIC_RPS		BIT(3)
#define CFG_ENABLE_DP_RX_THREADS	BIT(4)
#define CFG_RX_MODE_MAX (CFG_ENABLE_RX_THREAD | \
					  CFG_ENABLE_RPS | \
					  CFG_ENABLE_NAPI | \
					  CFG_ENABLE_DYNAMIC_RPS | \
					  CFG_ENABLE_DP_RX_THREADS)
#ifdef MDM_PLATFORM
#define CFG_RX_MODE_DEFAULT 0
#elif defined(HELIUMPLUS)
#define CFG_RX_MODE_DEFAULT CFG_ENABLE_NAPI
#elif defined(QCA_WIFI_QCA6290_11AX)
#define CFG_RX_MODE_DEFAULT (CFG_ENABLE_DP_RX_THREADS | CFG_ENABLE_NAPI)
#else
#define CFG_RX_MODE_DEFAULT (CFG_ENABLE_RX_THREAD | CFG_ENABLE_NAPI)
#endif

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL

/*
 * <ini>
 * TxFlowLowWaterMark - Low watermark for pausing network queues
 *
 * @Min: 0
 * @Max: 1000
 * @Default: 300
 *
 * This ini specifies the low watermark of data packets transmitted
 * before pausing netif queues in tx flow path. It is only applicable
 * where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowHighWaterMarkOffset, TxFlowMaxQueueDepth,
 *          TxLbwFlowLowWaterMark, TxLbwFlowHighWaterMarkOffset,
 *          TxLbwFlowMaxQueueDepth, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_LL_TX_FLOW_LWM \
		CFG_INI_UINT( \
		"TxFlowLowWaterMark", \
		0, \
		1000, \
		300, \
		CFG_VALUE_OR_DEFAULT, \
		"Low watermark for pausing network queues")

/*
 * <ini>
 * TxFlowHighWaterMarkOffset - High Watermark offset to unpause Netif queues
 * @Min: 0
 * @Max: 300
 * @Default: 94
 *
 * This ini specifies the offset to upause the netif queues
 * when they are paused due to insufficient descriptors as guided by
 * ini TxFlowLowWaterMark. It is only applicable where legacy flow control
 * is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowMaxQueueDepth,
 *          TxLbwFlowLowWaterMark, TxLbwFlowHighWaterMarkOffset,
 *          TxLbwFlowMaxQueueDepth, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_LL_TX_FLOW_HWM_OFFSET \
		CFG_INI_UINT( \
		"TxFlowHighWaterMarkOffset", \
		0, \
		300, \
		94, \
		CFG_VALUE_OR_DEFAULT, \
		"High Watermark offset to unpause Netif queues")

/*
 * <ini>
 * TxFlowMaxQueueDepth - Max pause queue depth.
 *
 * @Min: 400
 * @Max: 3500
 * @Default: 1500
 *
 * This ini specifies the max queue pause depth.It is only applicable
 * where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxLbwFlowLowWaterMark, TxLbwFlowHighWaterMarkOffset,
 *          TxLbwFlowMaxQueueDepth, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_LL_TX_FLOW_MAX_Q_DEPTH \
		CFG_INI_UINT( \
		"TxFlowMaxQueueDepth", \
		400, \
		3500, \
		1500, \
		CFG_VALUE_OR_DEFAULT, \
		"Max pause queue depth")

/*
 * <ini>
 * TxLbwFlowLowWaterMark - Low watermark for pausing network queues
 *                         in low bandwidth band
 * @Min: 0
 * @Max: 1000
 * @Default: 450
 *
 * This ini specifies the low watermark of data packets transmitted
 * before pausing netif queues in tx flow path in low bandwidth band.
 * It is only applicable where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowHighWaterMarkOffset,
 *          TxLbwFlowMaxQueueDepth, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_LL_TX_LBW_FLOW_LWM \
		CFG_INI_UINT( \
		"TxLbwFlowLowWaterMark", \
		0, \
		1000, \
		450, \
		CFG_VALUE_OR_DEFAULT, \
		"Low watermark for pausing network queues")

/*
 * <ini>
 * TxLbwFlowHighWaterMarkOffset - High Watermark offset to unpause Netif queues
 *                                in low bandwidth band.
 * @Min: 0
 * @Max: 300
 * @Default: 50
 *
 * This ini specifies the offset to upause the netif queues
 * when they are paused due to insufficient descriptors as guided by
 * ini TxLbwFlowLowWaterMark in low bandwidth band. It is only applicable
 * where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowLowWaterMark,
 *          TxLbwFlowMaxQueueDepth, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_LL_TX_LBW_FLOW_HWM_OFFSET \
		CFG_INI_UINT( \
		"TxLbwFlowHighWaterMarkOffset", \
		0, \
		300, \
		50, \
		CFG_VALUE_OR_DEFAULT, \
		"High Watermark offset to unpause Netif queues")

/*
 * <ini>
 * TxLbwFlowMaxQueueDepth - Max pause queue depth in low bandwidth band
 *
 * @Min: 400
 * @Max: 3500
 * @Default: 750
 *
 * This ini specifies the max queue pause depth in low bandwidth band.
 * It is only applicable where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowLowWaterMark,
 *          TxLbwFlowHighWaterMarkOffset, TxHbwFlowLowWaterMark,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_LL_TX_LBW_FLOW_MAX_Q_DEPTH \
		CFG_INI_UINT( \
		"TxLbwFlowMaxQueueDepth", \
		400, \
		3500, \
		750, \
		CFG_VALUE_OR_DEFAULT, \
		"Max pause queue depth in low bandwidth band")

/*
 * <ini>
 * TxHbwFlowLowWaterMark - Low watermark for pausing network queues
 *                         in high bandwidth band
 * @Min: 0
 * @Max: 1000
 * @Default: 406
 *
 * This ini specifies the threshold of data packets transmitted
 * before pausing netif queues.It is only applicable where
 * legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowLowWaterMark,
 *          TxLbwFlowHighWaterMarkOffset, TxLbwFlowMaxQueueDepth,
 *          TxHbwFlowHighWaterMarkOffset, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_LL_TX_HBW_FLOW_LWM \
		CFG_INI_UINT( \
		"TxHbwFlowLowWaterMark", \
		0, \
		1000, \
		406, \
		CFG_VALUE_OR_DEFAULT, \
		"Low watermark for pausing network queues")

/*
 * <ini>
 * TxHbwFlowHighWaterMarkOffset - High Watermark offset to unpause Netif queues
 *                                in high bandwidth band.
 * @Min: 0
 * @Max: 300
 * @Default: 94
 *
 * This ini specifies the offset to upause the netif queues
 * when they are paused due to insufficient descriptors as guided by
 * ini TxHbwFlowLowWaterMark in high bandwidth band. It is only applicable
 * where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowLowWaterMark,
 *          TxLbwFlowHighWaterMarkOffset, TxLbwFlowMaxQueueDepth,
 *          TxHbwFlowLowWaterMark, TxHbwFlowMaxQueueDepth
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_LL_TX_HBW_FLOW_HWM_OFFSET \
		CFG_INI_UINT( \
		"TxHbwFlowHighWaterMarkOffset", \
		0, \
		300, \
		94, \
		CFG_VALUE_OR_DEFAULT, \
		"High Watermark offset to unpause Netif queues")

/*
 * <ini>
 * TxHbwFlowMaxQueueDepth - Max pause queue depth in high bandwidth band
 * @Min: 4000
 * @Max: 3500
 * @Default: 1500
 *
 * This ini specifies the max queue pause depth in high bandwidth band.
 * It is only applicable where legacy flow control is used i.e.for Rome.
 *
 * Related: TxFlowLowWaterMark, TxFlowHighWaterMarkOffset,
 *          TxFlowMaxQueueDepth, TxLbwFlowLowWaterMark,
 *          TxLbwFlowHighWaterMarkOffset, TxLbwFlowMaxQueueDepth,
 *          TxHbwFlowLowWaterMark, TxHbwFlowHighWaterMarkOffset
 *
 * Supported Feature: Dynamic Flow Control
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_LL_TX_HBW_FLOW_MAX_Q_DEPTH \
		CFG_INI_UINT( \
		"TxHbwFlowMaxQueueDepth", \
		400, \
		3500, \
		1500, \
		CFG_VALUE_OR_DEFAULT, \
		"Max pause queue depth in high bandwidth band")

#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */
#ifdef MSM_PLATFORM
/*
 * <ini>
 * gBusBandwidthHighThreshold - bus bandwidth high threshold
 *
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 2000
 *
 * This ini specifies thebus bandwidth high threshold
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_BUS_BANDWIDTH_HIGH_THRESHOLD \
		CFG_INI_UINT( \
		"gBusBandwidthHighThreshold", \
		0, \
		4294967295UL, \
		2000, \
		CFG_VALUE_OR_DEFAULT, \
		"Bus bandwidth high threshold")

/*
 * <ini>
 * gBusBandwidthMediumThreshold - bus bandwidth medium threshold
 *
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 500
 *
 * This ini specifies thebus bandwidth medium threshold
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_BUS_BANDWIDTH_MEDIUM_THRESHOLD \
		CFG_INI_UINT( \
		"gBusBandwidthMediumThreshold", \
		0, \
		4294967295UL, \
		500, \
		CFG_VALUE_OR_DEFAULT, \
		"Bus bandwidth medium threshold")

/*
 * <ini>
 * gBusBandwidthLowThreshold - bus bandwidth low threshold
 *
 * @Min: 0
 * @Max: 4294967295UL
 * @Default: 150
 *
 * This ini specifies thebus bandwidth low threshold
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_BUS_BANDWIDTH_LOW_THRESHOLD \
		CFG_INI_UINT( \
		"gBusBandwidthLowThreshold", \
		0, \
		4294967295UL, \
		150, \
		CFG_VALUE_OR_DEFAULT, \
		"Bus bandwidth low threshold")

/*
 * <ini>
 * gBusBandwidthComputeInterval - bus bandwidth compute interval
 *
 * @Min: 0
 * @Max: 10000
 * @Default: 100
 *
 * This ini specifies thebus bandwidth compute interval
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_BUS_BANDWIDTH_COMPUTE_INTERVAL \
		CFG_INI_UINT( \
		"gBusBandwidthComputeInterval", \
		0, \
		10000, \
		100, \
		CFG_VALUE_OR_DEFAULT, \
		"Bus bandwidth compute interval")

/*
 * <ini>
 * gTcpLimitOutputEnable - Control to enable TCP limit output byte
 * @Default: true
 *
 * This ini is used to enable dynamic configuration of TCP limit output bytes
 * tcp_limit_output_bytes param. Enabling this will let driver post message to
 * cnss-daemon, accordingly cnss-daemon will modify the tcp_limit_output_bytes.
 *
 * Supported Feature: Tcp limit output bytes
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_ENABLE_TCP_LIMIT_OUTPUT \
		CFG_INI_BOOL( \
		"gTcpLimitOutputEnable", \
		true, \
		"Control to enable TCP limit output byte")

/*
 * <ini>
 * gTcpAdvWinScaleEnable - Control to enable  TCP adv window scaling
 * @Default: true
 *
 * This ini is used to enable dynamic configuration of TCP adv window scaling
 * system parameter.
 *
 * Supported Feature: Tcp Advance Window Scaling
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_ENABLE_TCP_ADV_WIN_SCALE \
		CFG_INI_BOOL( \
		"gTcpAdvWinScaleEnable", \
		true, \
		"Control to enable  TCP adv window scaling")

/*
 * <ini>
 * gTcpDelAckEnable - Control to enable Dynamic Configuration of Tcp Delayed Ack
 * @Default: true
 *
 * This ini is used to enable Dynamic Configuration of Tcp Delayed Ack
 *
 * Related: gTcpDelAckThresholdHigh, gTcpDelAckThresholdLow,
 *          gTcpDelAckTimerCount
 *
 * Supported Feature: Tcp Delayed Ack
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_ENABLE_TCP_DELACK \
		CFG_INI_BOOL( \
		"gTcpDelAckEnable", \
		true, \
		"Control to enable Dynamic Config of Tcp Delayed Ack")

/*
 * <ini>
 * gTcpDelAckThresholdHigh - High Threshold inorder to trigger TCP Del Ack
 *                                          indication
 * @Min: 0
 * @Max: 16000
 * @Default: 500
 *
 * This ini is used to mention the High Threshold inorder to trigger TCP Del Ack
 * indication i.e the threshold of packets received over a period of 100 ms.
 * i.e to have a low RX throughput requirement
 * Related: gTcpDelAckEnable, gTcpDelAckThresholdLow, gTcpDelAckTimerCount
 *
 * Supported Feature: Tcp Delayed Ack
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_TCP_DELACK_THRESHOLD_HIGH \
		CFG_INI_UINT( \
		"gTcpDelAckThresholdHigh", \
		0, \
		16000, \
		500, \
		CFG_VALUE_OR_DEFAULT, \
		"High Threshold inorder to trigger TCP Del Ack")

/*
 * <ini>
 * gTcpDelAckThresholdLow - Low Threshold inorder to trigger TCP Del Ack
 *                                          indication
 * @Min: 0
 * @Max: 10000
 * @Default: 1000
 *
 * This ini is used to mention the Low Threshold inorder to trigger TCP Del Ack
 * indication i.e the threshold of packets received over a period of 100 ms.
 * i.e to have a low RX throughput requirement
 *
 * Related: gTcpDelAckEnable, gTcpDelAckThresholdHigh, gTcpDelAckTimerCount
 *
 * Supported Feature: Tcp Delayed Ack
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_TCP_DELACK_THRESHOLD_LOW \
		CFG_INI_UINT( \
		"gTcpDelAckThresholdLow", \
		0, \
		10000, \
		1000, \
		CFG_VALUE_OR_DEFAULT, \
		"Low Threshold inorder to trigger TCP Del Ack")

/*
 * <ini>
 * gTcpDelAckTimerCount - Del Ack Timer Count inorder to trigger TCP Del Ack
 *                                      indication
 * @Min: 1
 * @Max: 1000
 * @Default: 30
 *
 * This ini is used to mention the Del Ack Timer Count inorder to
 * trigger TCP Del Ack indication i.e number of 100 ms periods
 *
 * Related: gTcpDelAckEnable, gTcpDelAckThresholdHigh, gTcpDelAckThresholdLow
 *
 * Supported Feature: Tcp Delayed Ack
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_TCP_DELACK_TIMER_COUNT \
		CFG_INI_UINT( \
		"gTcpDelAckTimerCount", \
		1, \
		1000, \
		30, \
		CFG_VALUE_OR_DEFAULT, \
		"Del Ack Timer Count inorder to trigger TCP Del Ack")

/*
 * <ini>
 * gTcpTxHighTputThreshold - High Threshold inorder to trigger High
 *                                          Tx Throughput requirement.
 * @Min: 0
 * @Max: 16000
 * @Default: 500
 *
 * This ini specifies the threshold of packets transmitted
 * over a period of 100 ms beyond which TCP can be considered to have a high
 * TX throughput requirement. The driver uses this condition to tweak TCP TX
 * specific parameters (via cnss-daemon)
 *
 * Supported Feature: To tweak TCP TX n/w parameters
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_TCP_TX_HIGH_TPUT_THRESHOLD \
		CFG_INI_UINT( \
		"gTcpTxHighTputThreshold", \
		0, \
		16000, \
		500, \
		CFG_VALUE_OR_DEFAULT, \
		"High Threshold inorder to trigger High Tx Tp")

#endif /* MSM_PLATFORM */

/*
 * <ini>
 * NAPI_CPU_AFFINITY_MASK - CPU mask to affine NAPIs
 *
 * @Min: 0
 * @Max: 0xFF
 * @Default: 0
 *
 * This ini is used to set NAPI IRQ CPU affinity
 *
 * Supported Feature: NAPI
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_NAPI_CE_CPU_MASK \
		CFG_INI_UINT( \
		"NAPI_CPU_AFFINITY_MASK", \
		0, \
		0xFF, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"CPU mask to affine NAPIs")

/*
 * <ini>
 * RX_THREAD_CPU_AFFINITY_MASK - CPU mask to affine Rx_thread
 *
 * @Min: 0
 * @Max: 0xFF
 * @Default: 0x02
 *
 * This ini is used to set Rx_thread CPU affinity
 *
 * Supported Feature: Rx_thread
 *
 * Usage: Internal
 *
 * </ini>
 */
#ifdef RX_PERFORMANCE
#define CFG_DP_RX_THREAD_CPU_MASK \
		CFG_INI_UINT( \
		"RX_THREAD_CPU_AFFINITY_MASK", \
		0, \
		0xFF, \
		0xFE, \
		CFG_VALUE_OR_DEFAULT, \
		"CPU mask to affine Rx_thread")
#else
#define CFG_DP_RX_THREAD_CPU_MASK \
		CFG_INI_UINT( \
		"RX_THREAD_CPU_AFFINITY_MASK", \
		0, \
		0xFF, \
		0, \
		CFG_VALUE_OR_DEFAULT, \
		"CPU mask to affine Rx_thread")
#endif

/*
 * <ini>
 * rpsRxQueueCpuMapList - RPS map for different RX queues
 *
 * @Default: e
 *
 * This ini is used to set RPS map for different RX queues.
 *
 * List of RPS CPU maps for different rx queues registered by WLAN driver
 * Ref - Kernel/Documentation/networking/scaling.txt
 * RPS CPU map for a particular RX queue, selects CPU(s) for bottom half
 * processing of RX packets. For example, for a system with 4 CPUs,
 * 0xe: Use CPU1 - CPU3 and donot use CPU0.
 * 0x0: RPS is disabled, packets are processed on the interrupting CPU.
.*
 * WLAN driver registers NUM_TX_QUEUES queues for tx and rx each during
 * alloc_netdev_mq. Hence, we need to have a cpu mask for each of the rx queues.
 *
 * For example, if the NUM_TX_QUEUES is 4, a sample WLAN ini entry may look like
 * rpsRxQueueCpuMapList=a b c d
 * For a 4 CPU system (CPU0 - CPU3), this implies:
 * 0xa - (1010) use CPU1, CPU3 for rx queue 0
 * 0xb - (1011) use CPU0, CPU1 and CPU3 for rx queue 1
 * 0xc - (1100) use CPU2, CPU3 for rx queue 2
 * 0xd - (1101) use CPU0, CPU2 and CPU3 for rx queue 3

 * In practice, we may want to avoid the cores which are heavily loaded.
 *
 * Default value of rpsRxQueueCpuMapList. Different platforms may have
 * different configurations for NUM_TX_QUEUES and # of cpus, and will need to
 * configure an appropriate value via ini file. Setting default value to 'e' to
 * avoid use of CPU0 (since its heavily used by other system processes) by rx
 * queue 0, which is currently being used for rx packet processing.
 *
 * Maximum length of string used to hold a list of cpu maps for various rx
 * queues. Considering a 16 core system with 5 rx queues, a RPS CPU map
 * list may look like -
 * rpsRxQueueCpuMapList = ffff ffff ffff ffff ffff
 * (all 5 rx queues can be processed on all 16 cores)
 * max string len = 24 + 1(for '\0'). Considering 30 to be on safe side.
 *
 * Supported Feature: Rx_thread
 *
 * Usage: Internal
 * </ini>
 */
#define CFG_DP_RPS_RX_QUEUE_CPU_MAP_LIST \
		CFG_INI_STRING( \
		"rpsRxQueueCpuMapList", \
		1, \
		30, \
		"e", \
		"specify RPS map for different RX queus")

/*
 * <ini>
 * gEnableTxOrphan- Enable/Disable orphaning of Tx packets
 * @Default: false
 *
 * This ini is used to enable/disable orphaning of Tx packets.
 *
 * Related: None
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DP_TX_ORPHAN_ENABLE \
		CFG_INI_BOOL( \
		"gEnableTxOrphan", \
		false, \
		"orphaning of Tx packets")

/*
 * <ini>
 * rx_mode - Control to decide rx mode for packet procesing
 *
 * @Min: 0
 * @Max: (CFG_ENABLE_RX_THREAD | CFG_ENABLE_RPS | CFG_ENABLE_NAPI | \
 *	 CFG_ENABLE_DYNAMIC_RPS)
 *
 * Some possible configurations:
 * rx_mode=0 - Uses tasklets for bottom half
 * CFG_ENABLE_NAPI (rx_mode=4) - Uses NAPI for bottom half
 * CFG_ENABLE_RX_THREAD | CFG_ENABLE_NAPI (rx_mode=5) - NAPI for bottom half,
 * rx_thread for stack. Single threaded.
 * CFG_ENABLE_DP_RX_THREAD | CFG_ENABLE_NAPI (rx_mode=10) - NAPI for bottom
 * half, dp_rx_thread for stack processing. Supports multiple rx threads.
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_RX_MODE \
		CFG_INI_UINT("rx_mode", \
		0, CFG_RX_MODE_MAX, CFG_RX_MODE_DEFAULT, \
		CFG_VALUE_OR_DEFAULT, \
		"Control to decide rx mode for packet procesing")

/*
 * <ini>
 * enable_multicast_replay_filter - Enable filtering of replayed multicast
 * packets
 *
 * In a typical infrastructure setup, it is quite normal to receive
 * replayed multicast packets. These packets may cause more harm than
 * help if not handled properly. Providing a configuration option
 * to enable filtering of such packets
 *
 * </ini>
 */
#define CFG_DP_FILTER_MULTICAST_REPLAY \
	CFG_INI_BOOL("enable_multicast_replay_filter", \
	true, "Enable filtering of replayed multicast packets")

/*
 * <ini>
 * rx_wakelock_timeout - Amount of time to hold wakelock for RX unicast packets
 * @Min: 0
 * @Max: 100
 * @Default: 50
 *
 * This ini item configures the amount of time, in milliseconds, that the driver
 * should prevent system power collapse after receiving an RX unicast packet.
 * A conigured value of 0 disables the RX Wakelock feature completely.
 *
 * Related: None.
 *
 * Supported Feature: RX Wakelock
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_DP_RX_WAKELOCK_TIMEOUT \
	CFG_INI_UINT("rx_wakelock_timeout", \
	0, 100, 50, CFG_VALUE_OR_DEFAULT, \
	"Amount of time to hold wakelock for RX unicast packets")

/*
 * <ini>
 * num_dp_rx_threads - Control to set the number of dp rx threads
 *
 * @Min: 1
 * @Max: 4
 * @Default: 1
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_NUM_DP_RX_THREADS \
	CFG_INI_UINT("num_dp_rx_threads", \
	1, 4, 1, CFG_VALUE_OR_DEFAULT, \
	"Control to set the number of dp rx threads")

#define CFG_DP_CE_SERVICE_MAX_RX_IND_FLUSH \
		CFG_INI_UINT("ce_service_max_rx_ind_flush", \
		1, 32, 32, \
		CFG_VALUE_OR_DEFAULT, "Ctrl to set ce service max rx ind flsh")

#define CFG_DP_CE_SERVICE_MAX_YIELD_TIME \
		CFG_INI_UINT("ce_service_max_yield_time", \
		500, 10000, 10000, \
		CFG_VALUE_OR_DEFAULT, "Ctrl to set ce service max yield time")

#ifdef WLAN_FEATURE_FASTPATH
#define CFG_DP_ENABLE_FASTPATH \
		CFG_INI_BOOL("gEnableFastPath", \
		false, "Ctrl to enable fastpath feature")

#define CFG_DP_ENABLE_FASTPATH_ALL \
	CFG(CFG_DP_ENABLE_FASTPATH)
#else
#define CFG_DP_ENABLE_FASTPATH_ALL
#endif

#define CFG_DP_ENABLE_TCP_PARAM_UPDATE \
		CFG_INI_BOOL("enable_tcp_param_update", \
		false, "configure TCP param through Wi-Fi HAL")
/*
 * <ini>
 *
 * Enable/disable DPTRACE
 * Enabling this might have performace impact.
 *
 * Config DPTRACE
 * The sequence of params is important. If some param is missing, defaults are
 * considered.
 * Param 1: Enable/Disable DP Trace live mode (uint8_t)
 * Param 2: DP Trace live mode high bandwidth thresh.(uint8_t)
 *         (packets/second) beyond which DP Trace is disabled. Decimal Val.
 *          MGMT, DHCP, EAPOL, ARP pkts are not counted. ICMP and Data are.
 * Param 3: Default Verbosity (0-4)
 * Param 4: Proto Bitmap (uint8_t). Decimal Value.
 *          (decimal 62 = 0x3e)
 * e.g., to disable live mode, use the following param in the ini file.
 * gDptraceConfig = 0
 * e.g., to enable dptrace live mode and set the thresh as 6,
 * use the following param in the ini file.
 * gDptraceConfig = 1, 6
 *
 * </ini>
 */
#ifdef CONFIG_DP_TRACE
#define CFG_DP_ENABLE_DP_TRACE \
			CFG_INI_BOOL("enable_dp_trace", \
			true, "Ctrl to enable dp trace feature")

#define CFG_DP_DP_TRACE_CONFIG \
		CFG_INI_STRING( \
		"gDptraceConfig", \
		1, \
		20, \
		"1, 6, 2, 126", \
		"dp trace configuration string")
#define CFG_DP_CONFIG_DP_TRACE_ALL \
		CFG(CFG_DP_ENABLE_DP_TRACE) \
		CFG(CFG_DP_DP_TRACE_CONFIG)
#else
#define CFG_DP_CONFIG_DP_TRACE_ALL
#endif

/*
 * <ini>
 * gEnableNUDTracking - Will enable or disable NUD tracking within driver
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini is used to enable or disable NUD tracking within driver
 *
 * Related: None
 *
 * Supported Feature: STA
 *
 * Usage: External
 *
 * <ini>
 */
#ifdef WLAN_NUD_TRACKING
#define CFG_DP_ENABLE_NUD_TRACKING \
		CFG_INI_BOOL("gEnableNUDTracking", \
		true, "Ctrl to enable nud tracking")

#define CFG_DP_ENABLE_NUD_TRACKING_ALL \
			CFG(CFG_DP_ENABLE_NUD_TRACKING)
#else
#define CFG_DP_ENABLE_NUD_TRACKING_ALL
#endif

/*
 * <ini>
 * gWmiCreditCount - Credit count for WMI exchange
 * @0: Not allowed
 * @1: Serialize WMI commands, 1 command at a time
 * @Default: 2: As advertized by FW
 *
 * This ini is used to serialize the WMI commandsif required.
 *
 * Related: None
 *
 * Usage: External
 *
 * <ini>
 */
#define WLAN_CFG_WMI_CREDIT_DEFAULT	0
#define WLAN_CFG_WMI_CREDIT_MIN		1
#define WLAN_CFG_WMI_CREDIT_MAX		2

#define CFG_DP_HTC_WMI_CREDIT_CNT \
		CFG_INI_UINT("gWmiCreditCount", \
		 WLAN_CFG_WMI_CREDIT_MIN, \
		 WLAN_CFG_WMI_CREDIT_MAX, \
		 WLAN_CFG_WMI_CREDIT_DEFAULT, \
		 CFG_VALUE_OR_DEFAULT, "WMI HTC CREDIT COUNT")

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
#define CFG_HDD_DP_LEGACY_TX_FLOW \
	CFG(CFG_DP_LL_TX_FLOW_LWM) \
	CFG(CFG_DP_LL_TX_FLOW_HWM_OFFSET) \
	CFG(CFG_DP_LL_TX_FLOW_MAX_Q_DEPTH) \
	CFG(CFG_DP_LL_TX_LBW_FLOW_LWM) \
	CFG(CFG_DP_LL_TX_LBW_FLOW_HWM_OFFSET) \
	CFG(CFG_DP_LL_TX_LBW_FLOW_MAX_Q_DEPTH) \
	CFG(CFG_DP_LL_TX_HBW_FLOW_LWM) \
	CFG(CFG_DP_LL_TX_HBW_FLOW_HWM_OFFSET) \
	CFG(CFG_DP_LL_TX_HBW_FLOW_MAX_Q_DEPTH)
#else
#define CFG_HDD_DP_LEGACY_TX_FLOW
#endif

#ifdef MSM_PLATFORM
#define CFG_HDD_DP_MSM_PLATFORM \
	CFG(CFG_DP_BUS_BANDWIDTH_HIGH_THRESHOLD) \
	CFG(CFG_DP_BUS_BANDWIDTH_MEDIUM_THRESHOLD) \
	CFG(CFG_DP_BUS_BANDWIDTH_LOW_THRESHOLD) \
	CFG(CFG_DP_BUS_BANDWIDTH_COMPUTE_INTERVAL) \
	CFG(CFG_DP_ENABLE_TCP_LIMIT_OUTPUT) \
	CFG(CFG_DP_ENABLE_TCP_ADV_WIN_SCALE) \
	CFG(CFG_DP_ENABLE_TCP_DELACK) \
	CFG(CFG_DP_TCP_DELACK_THRESHOLD_HIGH) \
	CFG(CFG_DP_TCP_DELACK_THRESHOLD_LOW) \
	CFG(CFG_DP_TCP_DELACK_TIMER_COUNT) \
	CFG(CFG_DP_TCP_TX_HIGH_TPUT_THRESHOLD)
#else
#define CFG_HDD_DP_MSM_PLATFORM
#endif

#define CFG_HDD_DP_ALL \
	CFG(CFG_DP_NAPI_CE_CPU_MASK) \
	CFG(CFG_DP_RX_THREAD_CPU_MASK) \
	CFG(CFG_DP_RPS_RX_QUEUE_CPU_MAP_LIST) \
	CFG(CFG_DP_TX_ORPHAN_ENABLE) \
	CFG(CFG_DP_RX_MODE) \
	CFG(CFG_DP_CE_SERVICE_MAX_RX_IND_FLUSH) \
	CFG(CFG_DP_CE_SERVICE_MAX_YIELD_TIME) \
	CFG(CFG_DP_ENABLE_TCP_PARAM_UPDATE) \
	CFG(CFG_DP_FILTER_MULTICAST_REPLAY) \
	CFG(CFG_DP_RX_WAKELOCK_TIMEOUT) \
	CFG(CFG_DP_NUM_DP_RX_THREADS) \
	CFG(CFG_DP_HTC_WMI_CREDIT_CNT) \
	CFG_DP_ENABLE_FASTPATH_ALL \
	CFG_HDD_DP_MSM_PLATFORM \
	CFG_HDD_DP_LEGACY_TX_FLOW \
	CFG_DP_ENABLE_NUD_TRACKING_ALL \
	CFG_DP_CONFIG_DP_TRACE_ALL
#endif
