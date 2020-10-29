/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains definitions of Data Path configuration.
 */

#ifndef _CFG_DP_H_
#define _CFG_DP_H_

#include "cfg_define.h"

#define WLAN_CFG_MAX_CLIENTS 64
#define WLAN_CFG_MAX_CLIENTS_MIN 8
#define WLAN_CFG_MAX_CLIENTS_MAX 64

/* Change this to a lower value to enforce scattered idle list mode */
#define WLAN_CFG_MAX_ALLOC_SIZE 0x200000
#define WLAN_CFG_MAX_ALLOC_SIZE_MIN 0x80000
#define WLAN_CFG_MAX_ALLOC_SIZE_MAX 0x200000

#if defined(QCA_LL_TX_FLOW_CONTROL_V2) || \
	defined(QCA_LL_PDEV_TX_FLOW_CONTROL)
#define WLAN_CFG_TX_FLOW_START_QUEUE_OFFSET 10
#define WLAN_CFG_TX_FLOW_STOP_QUEUE_TH 15
#else
#define WLAN_CFG_TX_FLOW_START_QUEUE_OFFSET 0
#define WLAN_CFG_TX_FLOW_STOP_QUEUE_TH 0
#endif

#define WLAN_CFG_PER_PDEV_TX_RING_MIN 0
#define WLAN_CFG_PER_PDEV_TX_RING_MAX 1

#if defined(WLAN_MAX_PDEVS) && (WLAN_MAX_PDEVS == 1)
#define WLAN_CFG_PER_PDEV_RX_RING 0
#define WLAN_CFG_PER_PDEV_LMAC_RING 0
#define WLAN_LRO_ENABLE 0
#ifdef QCA_WIFI_QCA6750
#define WLAN_CFG_MAC_PER_TARGET 1
#else
#define WLAN_CFG_MAC_PER_TARGET 2
#endif
#ifdef IPA_OFFLOAD
/* Size of TCL TX Ring */
#if defined(TX_TO_NPEERS_INC_TX_DESCS)
#define WLAN_CFG_TX_RING_SIZE 2048
#else
#define WLAN_CFG_TX_RING_SIZE 1024
#endif

#define WLAN_CFG_IPA_TX_RING_SIZE 1024
#define WLAN_CFG_IPA_TX_COMP_RING_SIZE 1024

#define WLAN_CFG_PER_PDEV_TX_RING 0
#define WLAN_CFG_IPA_UC_TX_BUF_SIZE 2048
#define WLAN_CFG_IPA_UC_TX_PARTITION_BASE 3000
#define WLAN_CFG_IPA_UC_RX_IND_RING_COUNT 1024
#else
#define WLAN_CFG_TX_RING_SIZE 512
#define WLAN_CFG_PER_PDEV_TX_RING 1
#define WLAN_CFG_IPA_UC_TX_BUF_SIZE 0
#define WLAN_CFG_IPA_UC_TX_PARTITION_BASE 0
#define WLAN_CFG_IPA_UC_RX_IND_RING_COUNT 0
#endif

#if defined(TX_TO_NPEERS_INC_TX_DESCS)
#define WLAN_CFG_TX_COMP_RING_SIZE 4096

/* Tx Descriptor and Tx Extension Descriptor pool sizes */
#define WLAN_CFG_NUM_TX_DESC  4096
#define WLAN_CFG_NUM_TX_EXT_DESC 4096
#else
#define WLAN_CFG_TX_COMP_RING_SIZE 1024

/* Tx Descriptor and Tx Extension Descriptor pool sizes */
#define WLAN_CFG_NUM_TX_DESC  1024
#define WLAN_CFG_NUM_TX_EXT_DESC 1024
#endif

/* Interrupt Mitigation - Batch threshold in terms of number of frames */
#define WLAN_CFG_INT_BATCH_THRESHOLD_TX 1
#define WLAN_CFG_INT_BATCH_THRESHOLD_OTHER 1

/* Interrupt Mitigation - Timer threshold in us */
#define WLAN_CFG_INT_TIMER_THRESHOLD_TX 8
#define WLAN_CFG_INT_TIMER_THRESHOLD_OTHER 8

#ifdef WLAN_DP_PER_RING_TYPE_CONFIG
#define WLAN_CFG_INT_BATCH_THRESHOLD_RX \
		WLAN_CFG_INT_BATCH_THRESHOLD_REO_RING
#define WLAN_CFG_INT_TIMER_THRESHOLD_RX \
		WLAN_CFG_INT_TIMER_THRESHOLD_REO_RING
#else
#define WLAN_CFG_INT_BATCH_THRESHOLD_RX 1
#define WLAN_CFG_INT_TIMER_THRESHOLD_RX 8
#endif
#endif

#define WLAN_CFG_RX_PENDING_HL_THRESHOLD 0x60000
#define WLAN_CFG_RX_PENDING_HL_THRESHOLD_MIN 0
#define WLAN_CFG_RX_PENDING_HL_THRESHOLD_MAX 0x80000

#define WLAN_CFG_RX_PENDING_LO_THRESHOLD 0x60000
#define WLAN_CFG_RX_PENDING_LO_THRESHOLD_MIN 100
#define WLAN_CFG_RX_PENDING_LO_THRESHOLD_MAX 0x80000

#define WLAN_CFG_INT_TIMER_THRESHOLD_WBM_RELEASE_RING 256
#define WLAN_CFG_INT_TIMER_THRESHOLD_REO_RING 512
#define WLAN_CFG_INT_BATCH_THRESHOLD_REO_RING 0

#define WLAN_CFG_PER_PDEV_RX_RING_MIN 0
#define WLAN_CFG_PER_PDEV_RX_RING_MAX 0

#define WLAN_CFG_PER_PDEV_LMAC_RING_MIN 0
#define WLAN_CFG_PER_PDEV_LMAC_RING_MAX 1

#define WLAN_CFG_TX_RING_SIZE_MIN 512
#define WLAN_CFG_TX_RING_SIZE_MAX 0x80000

#define WLAN_CFG_TX_COMP_RING_SIZE_MIN 512
#define WLAN_CFG_TX_COMP_RING_SIZE_MAX 0x80000

#define WLAN_CFG_NUM_TX_DESC_MIN  16
#define WLAN_CFG_NUM_TX_DESC_MAX  32768

#define WLAN_CFG_NUM_TX_EXT_DESC_MIN  16
#define WLAN_CFG_NUM_TX_EXT_DESC_MAX  0x80000

#define WLAN_CFG_INT_BATCH_THRESHOLD_TX_MIN 1
#define WLAN_CFG_INT_BATCH_THRESHOLD_TX_MAX 256

#define WLAN_CFG_INT_BATCH_THRESHOLD_RX_MIN 0
#define WLAN_CFG_INT_BATCH_THRESHOLD_RX_MAX 128

#define WLAN_CFG_INT_BATCH_THRESHOLD_REO_RING_MIN 1
#define WLAN_CFG_INT_BATCH_THRESHOLD_REO_RING_MAX 128

#define WLAN_CFG_INT_BATCH_THRESHOLD_WBM_RELEASE_RING_MIN 1
#define WLAN_CFG_INT_BATCH_THRESHOLD_WBM_RELEASE_RING_MAX 128

#define WLAN_CFG_INT_BATCH_THRESHOLD_OTHER_MIN 1
#define WLAN_CFG_INT_BATCH_THRESHOLD_OTHER_MAX 1

#define WLAN_CFG_INT_TIMER_THRESHOLD_TX_MIN 8
#define WLAN_CFG_INT_TIMER_THRESHOLD_TX_MAX 1000

#define WLAN_CFG_INT_TIMER_THRESHOLD_RX_MIN 8
#define WLAN_CFG_INT_TIMER_THRESHOLD_RX_MAX 500

#define WLAN_CFG_INT_TIMER_THRESHOLD_OTHER_MIN 8
#define WLAN_CFG_INT_TIMER_THRESHOLD_OTHER_MAX 1000

#define WLAN_CFG_INT_TIMER_THRESHOLD_REO_RING_MIN 8
#define WLAN_CFG_INT_TIMER_THRESHOLD_REO_RING_MAX 512

#define WLAN_CFG_INT_TIMER_THRESHOLD_WBM_RELEASE_RING_MIN 8
#define WLAN_CFG_INT_TIMER_THRESHOLD_WBM_RELEASE_RING_MAX 500

#define WLAN_CFG_NSS_TX_COMP_RING_SIZE 0x2000
#define WLAN_CFG_NSS_TX_COMP_RING_SIZE_MIN 0x2000
#define WLAN_CFG_NSS_TX_COMP_RING_SIZE_MAX 0xc000

#ifdef QCA_LL_TX_FLOW_CONTROL_V2

/* Per vdev pools */
#define WLAN_CFG_NUM_TX_DESC_POOL	3
#define WLAN_CFG_NUM_TXEXT_DESC_POOL	3

#else /* QCA_LL_TX_FLOW_CONTROL_V2 */

#ifdef TX_PER_PDEV_DESC_POOL
#define WLAN_CFG_NUM_TX_DESC_POOL	MAX_PDEV_CNT
#define WLAN_CFG_NUM_TXEXT_DESC_POOL	MAX_PDEV_CNT

#else /* TX_PER_PDEV_DESC_POOL */

#define WLAN_CFG_NUM_TX_DESC_POOL 3
#define WLAN_CFG_NUM_TXEXT_DESC_POOL 3

#endif /* TX_PER_PDEV_DESC_POOL */
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */

#define WLAN_CFG_NUM_TXEXT_DESC_POOL_MIN 1
#define WLAN_CFG_NUM_TXEXT_DESC_POOL_MAX 4

#define WLAN_CFG_HTT_PKT_TYPE 2
#define WLAN_CFG_HTT_PKT_TYPE_MIN 2
#define WLAN_CFG_HTT_PKT_TYPE_MAX 2

#define WLAN_CFG_MAX_PEER_ID 64
#define WLAN_CFG_MAX_PEER_ID_MIN 64
#define WLAN_CFG_MAX_PEER_ID_MAX 64

#define WLAN_CFG_RX_DEFRAG_TIMEOUT 100
#define WLAN_CFG_RX_DEFRAG_TIMEOUT_MIN 100
#define WLAN_CFG_RX_DEFRAG_TIMEOUT_MAX 100

#define WLAN_CFG_NUM_TCL_DATA_RINGS 3
#define WLAN_CFG_NUM_TCL_DATA_RINGS_MIN 3
#define WLAN_CFG_NUM_TCL_DATA_RINGS_MAX 3

#define WLAN_CFG_NUM_REO_DEST_RING 4
#define WLAN_CFG_NUM_REO_DEST_RING_MIN 4
#define WLAN_CFG_NUM_REO_DEST_RING_MAX 4

#define WLAN_CFG_NSS_NUM_TCL_DATA_RINGS 2
#define WLAN_CFG_NSS_NUM_TCL_DATA_RINGS_MIN 1
#define WLAN_CFG_NSS_NUM_TCL_DATA_RINGS_MAX 3

#define WLAN_CFG_NSS_NUM_REO_DEST_RING 2
#define WLAN_CFG_NSS_NUM_REO_DEST_RING_MIN 1
#define WLAN_CFG_NSS_NUM_REO_DEST_RING_MAX 3

#define WLAN_CFG_WBM_RELEASE_RING_SIZE 1024
#define WLAN_CFG_WBM_RELEASE_RING_SIZE_MIN 64
#define WLAN_CFG_WBM_RELEASE_RING_SIZE_MAX 1024

#define WLAN_CFG_TCL_CMD_CREDIT_RING_SIZE 32
#define WLAN_CFG_TCL_CMD_CREDIT_RING_SIZE_MIN 32
#define WLAN_CFG_TCL_CMD_CREDIT_RING_SIZE_MAX 32

#define WLAN_CFG_TCL_STATUS_RING_SIZE 32
#define WLAN_CFG_TCL_STATUS_RING_SIZE_MIN 32
#define WLAN_CFG_TCL_STATUS_RING_SIZE_MAX 32

#if defined(QCA_WIFI_QCA6290)
#define WLAN_CFG_REO_DST_RING_SIZE 1024
#else
#define WLAN_CFG_REO_DST_RING_SIZE 2048
#endif

#define WLAN_CFG_REO_DST_RING_SIZE_MIN 1024
#define WLAN_CFG_REO_DST_RING_SIZE_MAX 2048

#define WLAN_CFG_REO_REINJECT_RING_SIZE 128
#define WLAN_CFG_REO_REINJECT_RING_SIZE_MIN 32
#define WLAN_CFG_REO_REINJECT_RING_SIZE_MAX 128

#define WLAN_CFG_RX_RELEASE_RING_SIZE 1024
#define WLAN_CFG_RX_RELEASE_RING_SIZE_MIN 8
#if defined(QCA_WIFI_QCA6390) || defined(QCA_WIFI_QCA6490) || \
    defined(QCA_WIFI_QCA6750)
#define WLAN_CFG_RX_RELEASE_RING_SIZE_MAX 1024
#else
#define WLAN_CFG_RX_RELEASE_RING_SIZE_MAX 8192
#endif

#define WLAN_CFG_REO_EXCEPTION_RING_SIZE 256
#define WLAN_CFG_REO_EXCEPTION_RING_SIZE_MIN 128
#define WLAN_CFG_REO_EXCEPTION_RING_SIZE_MAX 512

#define WLAN_CFG_REO_CMD_RING_SIZE 128
#define WLAN_CFG_REO_CMD_RING_SIZE_MIN 64
#define WLAN_CFG_REO_CMD_RING_SIZE_MAX 128

#define WLAN_CFG_REO_STATUS_RING_SIZE 256
#define WLAN_CFG_REO_STATUS_RING_SIZE_MIN 128
#define WLAN_CFG_REO_STATUS_RING_SIZE_MAX 2048

#define WLAN_CFG_RXDMA_BUF_RING_SIZE 1024
#define WLAN_CFG_RXDMA_BUF_RING_SIZE_MIN 1024
#define WLAN_CFG_RXDMA_BUF_RING_SIZE_MAX 1024

#define WLAN_CFG_RXDMA_REFILL_RING_SIZE 4096
#define WLAN_CFG_RXDMA_REFILL_RING_SIZE_MIN 16
#define WLAN_CFG_RXDMA_REFILL_RING_SIZE_MAX 4096

#define WLAN_CFG_TX_DESC_LIMIT_0 0
#define WLAN_CFG_TX_DESC_LIMIT_0_MIN 4096
#define WLAN_CFG_TX_DESC_LIMIT_0_MAX 32768

#define WLAN_CFG_TX_DESC_LIMIT_1 0
#define WLAN_CFG_TX_DESC_LIMIT_1_MIN 4096
#define WLAN_CFG_TX_DESC_LIMIT_1_MAX 32768

#define WLAN_CFG_TX_DESC_LIMIT_2 0
#define WLAN_CFG_TX_DESC_LIMIT_2_MIN 4096
#define WLAN_CFG_TX_DESC_LIMIT_2_MAX 32768

#define WLAN_CFG_TX_DEVICE_LIMIT 65536
#define WLAN_CFG_TX_DEVICE_LIMIT_MIN 16384
#define WLAN_CFG_TX_DEVICE_LIMIT_MAX 65536

#define WLAN_CFG_TX_SW_INTERNODE_QUEUE 1024
#define WLAN_CFG_TX_SW_INTERNODE_QUEUE_MIN 128
#define WLAN_CFG_TX_SW_INTERNODE_QUEUE_MAX 1024

#define WLAN_CFG_RXDMA_MONITOR_BUF_RING_SIZE 4096
#define WLAN_CFG_RXDMA_MONITOR_BUF_RING_SIZE_MIN 16
#define WLAN_CFG_RXDMA_MONITOR_BUF_RING_SIZE_MAX 8192

#define WLAN_CFG_RXDMA_MONITOR_DST_RING_SIZE 2048
#define WLAN_CFG_RXDMA_MONITOR_DST_RING_SIZE_MIN 48
#define WLAN_CFG_RXDMA_MONITOR_DST_RING_SIZE_MAX 8192

#define WLAN_CFG_RXDMA_MONITOR_STATUS_RING_SIZE 1024
#define WLAN_CFG_RXDMA_MONITOR_STATUS_RING_SIZE_MIN 16
#define WLAN_CFG_RXDMA_MONITOR_STATUS_RING_SIZE_MAX 8192

#define WLAN_CFG_RXDMA_MONITOR_DESC_RING_SIZE 4096
#define WLAN_CFG_RXDMA_MONITOR_DESC_RING_SIZE_MIN 4096
#define WLAN_CFG_RXDMA_MONITOR_DESC_RING_SIZE_MAX 16384

#define WLAN_CFG_RXDMA_ERR_DST_RING_SIZE 1024
#define WLAN_CFG_RXDMA_ERR_DST_RING_SIZE_MIN 1024
#define WLAN_CFG_RXDMA_ERR_DST_RING_SIZE_MAX 8192

#define WLAN_CFG_RXDMA_MONITOR_RX_DROP_THRESH_SIZE 32
#define WLAN_CFG_RXDMA_MONITOR_RX_DROP_THRESH_SIZE_MIN 0
#define WLAN_CFG_RXDMA_MONITOR_RX_DROP_THRESH_SIZE_MAX 256

/**
 * Allocate as many RX descriptors as buffers in the SW2RXDMA
 * ring. This value may need to be tuned later.
 */
#if defined(QCA_HOST2FW_RXBUF_RING)
#define WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE 1
#define WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE_MIN 1
#define WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE_MAX 1
#define WLAN_CFG_RX_SW_DESC_NUM_SIZE 4096
#define WLAN_CFG_RX_SW_DESC_NUM_SIZE_MIN 4096
#define WLAN_CFG_RX_SW_DESC_NUM_SIZE_MAX 4096

/**
 * For low memory AP cases using 1 will reduce the rx descriptors memory req
 */
#elif defined(QCA_LOWMEM_CONFIG) || defined(QCA_512M_CONFIG)
#define WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE 1
#define WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE_MIN 1
#define WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE_MAX 3
#define WLAN_CFG_RX_SW_DESC_NUM_SIZE 4096
#define WLAN_CFG_RX_SW_DESC_NUM_SIZE_MIN 1024
#define WLAN_CFG_RX_SW_DESC_NUM_SIZE_MAX 12288

/**
 * AP use cases need to allocate more RX Descriptors than the number of
 * entries avaialable in the SW2RXDMA buffer replenish ring. This is to account
 * for frames sitting in REO queues, HW-HW DMA rings etc. Hence using a
 * multiplication factor of 3, to allocate three times as many RX descriptors
 * as RX buffers.
 */
#else
#define WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE 3
#define WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE_MIN 1
#define WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE_MAX 3
#define WLAN_CFG_RX_SW_DESC_NUM_SIZE 12288
#define WLAN_CFG_RX_SW_DESC_NUM_SIZE_MIN 4096
#define WLAN_CFG_RX_SW_DESC_NUM_SIZE_MAX 12288
#endif //QCA_HOST2FW_RXBUF_RING

#define WLAN_CFG_RX_FLOW_SEARCH_TABLE_SIZE 16384
#define WLAN_CFG_RX_FLOW_SEARCH_TABLE_SIZE_MIN 1
#define WLAN_CFG_RX_FLOW_SEARCH_TABLE_SIZE_MAX 16384

#define WLAN_CFG_PKTLOG_BUFFER_SIZE 10
#define WLAN_CFG_PKTLOG_MIN_BUFFER_SIZE 1
#define WLAN_CFG_PKTLOG_MAX_BUFFER_SIZE 10

#define WLAN_CFG_NUM_REO_RINGS_MAP 0xF
#define WLAN_CFG_NUM_REO_RINGS_MAP_MIN 0x1
#define WLAN_CFG_NUM_REO_RINGS_MAP_MAX 0xF

#define WLAN_CFG_RADIO_0_DEFAULT_REO 0x1
#define WLAN_CFG_RADIO_1_DEFAULT_REO 0x2
#define WLAN_CFG_RADIO_2_DEFAULT_REO 0x3

#define WLAN_CFG_RADIO_DEFAULT_REO_MIN 0x1
#define WLAN_CFG_RADIO_DEFAULT_REO_MAX 0x4

/* DP INI Declerations */
#define CFG_DP_HTT_PACKET_TYPE \
		CFG_INI_UINT("dp_htt_packet_type", \
		WLAN_CFG_HTT_PKT_TYPE_MIN, \
		WLAN_CFG_HTT_PKT_TYPE_MAX, \
		WLAN_CFG_HTT_PKT_TYPE, \
		CFG_VALUE_OR_DEFAULT, "DP HTT packet type")

#define CFG_DP_INT_BATCH_THRESHOLD_OTHER \
		CFG_INI_UINT("dp_int_batch_threshold_other", \
		WLAN_CFG_INT_BATCH_THRESHOLD_OTHER_MIN, \
		WLAN_CFG_INT_BATCH_THRESHOLD_OTHER_MAX, \
		WLAN_CFG_INT_BATCH_THRESHOLD_OTHER, \
		CFG_VALUE_OR_DEFAULT, "DP INT batch threshold Other")

#define CFG_DP_INT_BATCH_THRESHOLD_RX \
		CFG_INI_UINT("dp_int_batch_threshold_rx", \
		WLAN_CFG_INT_BATCH_THRESHOLD_RX_MIN, \
		WLAN_CFG_INT_BATCH_THRESHOLD_RX_MAX, \
		WLAN_CFG_INT_BATCH_THRESHOLD_RX, \
		CFG_VALUE_OR_DEFAULT, "DP INT batch threshold Rx")

#define CFG_DP_INT_BATCH_THRESHOLD_TX \
		CFG_INI_UINT("dp_int_batch_threshold_tx", \
		WLAN_CFG_INT_BATCH_THRESHOLD_TX_MIN, \
		WLAN_CFG_INT_BATCH_THRESHOLD_TX_MAX, \
		WLAN_CFG_INT_BATCH_THRESHOLD_TX, \
		CFG_VALUE_OR_DEFAULT, "DP INT threshold Tx")

#define CFG_DP_INT_TIMER_THRESHOLD_OTHER \
		CFG_INI_UINT("dp_int_timer_threshold_other", \
		WLAN_CFG_INT_TIMER_THRESHOLD_OTHER_MIN, \
		WLAN_CFG_INT_TIMER_THRESHOLD_OTHER_MAX, \
		WLAN_CFG_INT_TIMER_THRESHOLD_OTHER, \
		CFG_VALUE_OR_DEFAULT, "DP INT timer threshold Other")

#define CFG_DP_INT_TIMER_THRESHOLD_RX \
		CFG_INI_UINT("dp_int_timer_threshold_rx", \
		WLAN_CFG_INT_TIMER_THRESHOLD_RX_MIN, \
		WLAN_CFG_INT_TIMER_THRESHOLD_RX_MAX, \
		WLAN_CFG_INT_TIMER_THRESHOLD_RX, \
		CFG_VALUE_OR_DEFAULT, "DP INT timer threshold Rx")

#define CFG_DP_INT_TIMER_THRESHOLD_REO_RING \
		CFG_INI_UINT("dp_int_timer_threshold_reo_ring", \
		WLAN_CFG_INT_TIMER_THRESHOLD_REO_RING_MIN, \
		WLAN_CFG_INT_TIMER_THRESHOLD_REO_RING_MAX, \
		WLAN_CFG_INT_TIMER_THRESHOLD_REO_RING, \
		CFG_VALUE_OR_DEFAULT, "DP INT timer threshold Reo ring")

#define CFG_DP_INT_TIMER_THRESHOLD_WBM_RELEASE_RING \
		CFG_INI_UINT("dp_int_timer_threshold_wbm_release_ring", \
		WLAN_CFG_INT_TIMER_THRESHOLD_WBM_RELEASE_RING_MIN, \
		WLAN_CFG_INT_TIMER_THRESHOLD_WBM_RELEASE_RING_MAX, \
		WLAN_CFG_INT_TIMER_THRESHOLD_WBM_RELEASE_RING, \
		CFG_VALUE_OR_DEFAULT, "DP INT timer threshold wbm release ring")

#define CFG_DP_INT_TIMER_THRESHOLD_TX \
		CFG_INI_UINT("dp_int_timer_threshold_tx", \
		WLAN_CFG_INT_TIMER_THRESHOLD_TX_MIN, \
		WLAN_CFG_INT_TIMER_THRESHOLD_TX_MAX, \
		WLAN_CFG_INT_TIMER_THRESHOLD_TX, \
		CFG_VALUE_OR_DEFAULT, "DP INT timer threshold Tx")

#define CFG_DP_MAX_ALLOC_SIZE \
		CFG_INI_UINT("dp_max_alloc_size", \
		WLAN_CFG_MAX_ALLOC_SIZE_MIN, \
		WLAN_CFG_MAX_ALLOC_SIZE_MAX, \
		WLAN_CFG_MAX_ALLOC_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP Max Alloc Size")

#define CFG_DP_MAX_CLIENTS \
		CFG_INI_UINT("dp_max_clients", \
		WLAN_CFG_MAX_CLIENTS_MIN, \
		WLAN_CFG_MAX_CLIENTS_MAX, \
		WLAN_CFG_MAX_CLIENTS, \
		CFG_VALUE_OR_DEFAULT, "DP Max Clients")

#define CFG_DP_MAX_PEER_ID \
		CFG_INI_UINT("dp_max_peer_id", \
		WLAN_CFG_MAX_PEER_ID_MIN, \
		WLAN_CFG_MAX_PEER_ID_MAX, \
		WLAN_CFG_MAX_PEER_ID, \
		CFG_VALUE_OR_DEFAULT, "DP Max Peer ID")

#define CFG_DP_REO_DEST_RINGS \
		CFG_INI_UINT("dp_reo_dest_rings", \
		WLAN_CFG_NUM_REO_DEST_RING_MIN, \
		WLAN_CFG_NUM_REO_DEST_RING_MAX, \
		WLAN_CFG_NUM_REO_DEST_RING, \
		CFG_VALUE_OR_DEFAULT, "DP REO Destination Rings")

#define CFG_DP_TCL_DATA_RINGS \
		CFG_INI_UINT("dp_tcl_data_rings", \
		WLAN_CFG_NUM_TCL_DATA_RINGS_MIN, \
		WLAN_CFG_NUM_TCL_DATA_RINGS_MAX, \
		WLAN_CFG_NUM_TCL_DATA_RINGS, \
		CFG_VALUE_OR_DEFAULT, "DP TCL Data Rings")

#define CFG_DP_NSS_REO_DEST_RINGS \
		CFG_INI_UINT("dp_nss_reo_dest_rings", \
		WLAN_CFG_NSS_NUM_REO_DEST_RING_MIN, \
		WLAN_CFG_NSS_NUM_REO_DEST_RING_MAX, \
		WLAN_CFG_NSS_NUM_REO_DEST_RING, \
		CFG_VALUE_OR_DEFAULT, "DP NSS REO Destination Rings")

#define CFG_DP_NSS_TCL_DATA_RINGS \
		CFG_INI_UINT("dp_nss_tcl_data_rings", \
		WLAN_CFG_NSS_NUM_TCL_DATA_RINGS_MIN, \
		WLAN_CFG_NSS_NUM_TCL_DATA_RINGS_MAX, \
		WLAN_CFG_NSS_NUM_TCL_DATA_RINGS, \
		CFG_VALUE_OR_DEFAULT, "DP NSS TCL Data Rings")

#define CFG_DP_TX_DESC \
		CFG_INI_UINT("dp_tx_desc", \
		WLAN_CFG_NUM_TX_DESC_MIN, \
		WLAN_CFG_NUM_TX_DESC_MAX, \
		WLAN_CFG_NUM_TX_DESC, \
		CFG_VALUE_OR_DEFAULT, "DP Tx Descriptors")

#define CFG_DP_TX_EXT_DESC \
		CFG_INI_UINT("dp_tx_ext_desc", \
		WLAN_CFG_NUM_TX_EXT_DESC_MIN, \
		WLAN_CFG_NUM_TX_EXT_DESC_MAX, \
		WLAN_CFG_NUM_TX_EXT_DESC, \
		CFG_VALUE_OR_DEFAULT, "DP Tx Ext Descriptors")

#define CFG_DP_TX_EXT_DESC_POOLS \
		CFG_INI_UINT("dp_tx_ext_desc_pool", \
		WLAN_CFG_NUM_TXEXT_DESC_POOL_MIN, \
		WLAN_CFG_NUM_TXEXT_DESC_POOL_MAX, \
		WLAN_CFG_NUM_TXEXT_DESC_POOL, \
		CFG_VALUE_OR_DEFAULT, "DP Tx Ext Descriptors Pool")

#define CFG_DP_PDEV_RX_RING \
		CFG_INI_UINT("dp_pdev_rx_ring", \
		WLAN_CFG_PER_PDEV_RX_RING_MIN, \
		WLAN_CFG_PER_PDEV_RX_RING_MAX, \
		WLAN_CFG_PER_PDEV_RX_RING, \
		CFG_VALUE_OR_DEFAULT, "DP PDEV Rx Ring")

#define CFG_DP_PDEV_TX_RING \
		CFG_INI_UINT("dp_pdev_tx_ring", \
		WLAN_CFG_PER_PDEV_TX_RING_MIN, \
		WLAN_CFG_PER_PDEV_TX_RING_MAX, \
		WLAN_CFG_PER_PDEV_TX_RING, \
		CFG_VALUE_OR_DEFAULT, \
		"DP PDEV Tx Ring")

#define CFG_DP_RX_DEFRAG_TIMEOUT \
		CFG_INI_UINT("dp_rx_defrag_timeout", \
		WLAN_CFG_RX_DEFRAG_TIMEOUT_MIN, \
		WLAN_CFG_RX_DEFRAG_TIMEOUT_MAX, \
		WLAN_CFG_RX_DEFRAG_TIMEOUT, \
		CFG_VALUE_OR_DEFAULT, "DP Rx Defrag Timeout")

#define CFG_DP_TX_COMPL_RING_SIZE \
		CFG_INI_UINT("dp_tx_compl_ring_size", \
		WLAN_CFG_TX_COMP_RING_SIZE_MIN, \
		WLAN_CFG_TX_COMP_RING_SIZE_MAX, \
		WLAN_CFG_TX_COMP_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP Tx Completion Ring Size")

#define CFG_DP_TX_RING_SIZE \
		CFG_INI_UINT("dp_tx_ring_size", \
		WLAN_CFG_TX_RING_SIZE_MIN,\
		WLAN_CFG_TX_RING_SIZE_MAX,\
		WLAN_CFG_TX_RING_SIZE,\
		CFG_VALUE_OR_DEFAULT, "DP Tx Ring Size")

#define CFG_DP_NSS_COMP_RING_SIZE \
		CFG_INI_UINT("dp_nss_comp_ring_size", \
		WLAN_CFG_NSS_TX_COMP_RING_SIZE_MIN, \
		WLAN_CFG_NSS_TX_COMP_RING_SIZE_MAX, \
		WLAN_CFG_NSS_TX_COMP_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP NSS completion Ring Size")

#define CFG_DP_PDEV_LMAC_RING \
		CFG_INI_UINT("dp_pdev_lmac_ring", \
		WLAN_CFG_PER_PDEV_LMAC_RING_MIN, \
		WLAN_CFG_PER_PDEV_LMAC_RING_MAX, \
		WLAN_CFG_PER_PDEV_LMAC_RING, \
		CFG_VALUE_OR_DEFAULT, "DP pdev LMAC ring")
/*
 * <ini>
 * dp_rx_pending_hl_threshold - High threshold of frame number to start
 * frame dropping scheme
 * @Min: 0
 * @Max: 524288
 * @Default: 393216
 *
 * This ini entry is used to set a high limit threshold to start frame
 * dropping scheme
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DP_RX_PENDING_HL_THRESHOLD \
		CFG_INI_UINT("dp_rx_pending_hl_threshold", \
		WLAN_CFG_RX_PENDING_HL_THRESHOLD_MIN, \
		WLAN_CFG_RX_PENDING_HL_THRESHOLD_MAX, \
		WLAN_CFG_RX_PENDING_HL_THRESHOLD, \
		CFG_VALUE_OR_DEFAULT, "DP rx pending hl threshold")

/*
 * <ini>
 * dp_rx_pending_lo_threshold - Low threshold of frame number to stop
 * frame dropping scheme
 * @Min: 100
 * @Max: 524288
 * @Default: 393216
 *
 * This ini entry is used to set a low limit threshold to stop frame
 * dropping scheme
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DP_RX_PENDING_LO_THRESHOLD \
		CFG_INI_UINT("dp_rx_pending_lo_threshold", \
		WLAN_CFG_RX_PENDING_LO_THRESHOLD_MIN, \
		WLAN_CFG_RX_PENDING_LO_THRESHOLD_MAX, \
		WLAN_CFG_RX_PENDING_LO_THRESHOLD, \
		CFG_VALUE_OR_DEFAULT, "DP rx pending lo threshold")

#define CFG_DP_BASE_HW_MAC_ID \
		CFG_INI_UINT("dp_base_hw_macid", \
		0, 1, 1, \
		CFG_VALUE_OR_DEFAULT, "DP Base HW Mac ID")

#define CFG_DP_RX_HASH \
	CFG_INI_BOOL("dp_rx_hash", true, \
	"DP Rx Hash")

#define CFG_DP_TSO \
	CFG_INI_BOOL("TSOEnable", false, \
	"DP TSO Enabled")

#define CFG_DP_LRO \
	CFG_INI_BOOL("LROEnable", WLAN_LRO_ENABLE, \
	"DP LRO Enable")

/*
 * <ini>
 * CFG_DP_SG - Enable the SG feature standalonely
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini entry is used to enable/disable SG feature standalonely.
 * Also does Rome support SG on TX, lithium does not.
 * For example the lithium does not support SG on UDP frames.
 * Which is able to handle SG only for TSO frames(in case TSO is enabled).
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DP_SG \
	CFG_INI_BOOL("dp_sg_support", false, \
	"DP SG Enable")

#define CFG_DP_GRO \
	CFG_INI_BOOL("GROEnable", false, \
	"DP GRO Enable")

#define CFG_DP_OL_TX_CSUM \
	CFG_INI_BOOL("dp_offload_tx_csum_support", false, \
	"DP tx csum Enable")

#define CFG_DP_OL_RX_CSUM \
	CFG_INI_BOOL("dp_offload_rx_csum_support", false, \
	"DP rx csum Enable")

#define CFG_DP_RAWMODE \
	CFG_INI_BOOL("dp_rawmode_support", false, \
	"DP rawmode Enable")

#define CFG_DP_PEER_FLOW_CTRL \
	CFG_INI_BOOL("dp_peer_flow_control_support", false, \
	"DP peer flow ctrl Enable")

#define CFG_DP_NAPI \
	CFG_INI_BOOL("dp_napi_enabled", PLATFORM_VALUE(true, false), \
	"DP Napi Enabled")
/*
 * <ini>
 * gEnableP2pIpTcpUdpChecksumOffload - Enable checksum offload for P2P mode
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * This ini entry is used to enable/disable TX checksum(UDP/TCP) for P2P modes.
 * This includes P2P device mode, P2P client mode and P2P GO mode.
 * The feature is enabled by default. To disable TX checksum for P2P, add the
 * following entry in ini file:
 * gEnableP2pIpTcpUdpChecksumOffload=0
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DP_P2P_TCP_UDP_CKSUM_OFFLOAD \
		CFG_INI_BOOL("gEnableP2pIpTcpUdpChecksumOffload", true, \
		"DP TCP UDP Checksum Offload for P2P mode (device/cli/go)")

/*
 * <ini>
 * gEnableNanIpTcpUdpChecksumOffload - Enable checksum offload for NAN mode
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DP_NAN_TCP_UDP_CKSUM_OFFLOAD \
		CFG_INI_BOOL("gEnableNanIpTcpUdpChecksumOffload", true, \
		"DP TCP UDP Checksum Offload for NAN mode")

/*
 * <ini>
 * gEnableIpTcpUdpChecksumOffload - Enable checksum offload
 * @Min: 0
 * @Max: 1
 * @Default: 1
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DP_TCP_UDP_CKSUM_OFFLOAD \
	CFG_INI_BOOL("gEnableIpTcpUdpChecksumOffload", true, \
	"DP TCP UDP Checksum Offload")

#define CFG_DP_DEFRAG_TIMEOUT_CHECK \
	CFG_INI_BOOL("dp_defrag_timeout_check", true, \
	"DP Defrag Timeout Check")

#define CFG_DP_WBM_RELEASE_RING \
		CFG_INI_UINT("dp_wbm_release_ring", \
		WLAN_CFG_WBM_RELEASE_RING_SIZE_MIN, \
		WLAN_CFG_WBM_RELEASE_RING_SIZE_MAX, \
		WLAN_CFG_WBM_RELEASE_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP WBM Release Ring")

#define CFG_DP_TCL_CMD_CREDIT_RING \
		CFG_INI_UINT("dp_tcl_cmd_credit_ring", \
		WLAN_CFG_TCL_CMD_CREDIT_RING_SIZE_MIN, \
		WLAN_CFG_TCL_CMD_CREDIT_RING_SIZE_MAX, \
		WLAN_CFG_TCL_CMD_CREDIT_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP TCL Cmd_Credit ring")

#define CFG_DP_TCL_STATUS_RING \
		CFG_INI_UINT("dp_tcl_status_ring",\
		WLAN_CFG_TCL_STATUS_RING_SIZE_MIN, \
		WLAN_CFG_TCL_STATUS_RING_SIZE_MAX, \
		WLAN_CFG_TCL_STATUS_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP TCL status ring")

#define CFG_DP_REO_REINJECT_RING \
		CFG_INI_UINT("dp_reo_reinject_ring", \
		WLAN_CFG_REO_REINJECT_RING_SIZE_MIN, \
		WLAN_CFG_REO_REINJECT_RING_SIZE_MAX, \
		WLAN_CFG_REO_REINJECT_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP REO reinject ring")

#define CFG_DP_RX_RELEASE_RING \
		CFG_INI_UINT("dp_rx_release_ring", \
		WLAN_CFG_RX_RELEASE_RING_SIZE_MIN, \
		WLAN_CFG_RX_RELEASE_RING_SIZE_MAX, \
		WLAN_CFG_RX_RELEASE_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP Rx release ring")

#define CFG_DP_REO_EXCEPTION_RING \
		CFG_INI_UINT("dp_reo_exception_ring", \
		WLAN_CFG_REO_EXCEPTION_RING_SIZE_MIN, \
		WLAN_CFG_REO_EXCEPTION_RING_SIZE_MAX, \
		WLAN_CFG_REO_EXCEPTION_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP REO exception ring")

#define CFG_DP_REO_CMD_RING \
		CFG_INI_UINT("dp_reo_cmd_ring", \
		WLAN_CFG_REO_CMD_RING_SIZE_MIN, \
		WLAN_CFG_REO_CMD_RING_SIZE_MAX, \
		WLAN_CFG_REO_CMD_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP REO command ring")

#define CFG_DP_REO_STATUS_RING \
		CFG_INI_UINT("dp_reo_status_ring", \
		WLAN_CFG_REO_STATUS_RING_SIZE_MIN, \
		WLAN_CFG_REO_STATUS_RING_SIZE_MAX, \
		WLAN_CFG_REO_STATUS_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP REO status ring")

#define CFG_DP_RXDMA_BUF_RING \
		CFG_INI_UINT("dp_rxdma_buf_ring", \
		WLAN_CFG_RXDMA_BUF_RING_SIZE_MIN, \
		WLAN_CFG_RXDMA_BUF_RING_SIZE_MAX, \
		WLAN_CFG_RXDMA_BUF_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP RXDMA buffer ring")

#define CFG_DP_RXDMA_REFILL_RING \
		CFG_INI_UINT("dp_rxdma_refill_ring", \
		WLAN_CFG_RXDMA_REFILL_RING_SIZE_MIN, \
		WLAN_CFG_RXDMA_REFILL_RING_SIZE_MAX, \
		WLAN_CFG_RXDMA_REFILL_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP RXDMA refilll ring")

#define CFG_DP_TX_DESC_LIMIT_0 \
		CFG_INI_UINT("dp_tx_desc_limit_0", \
		WLAN_CFG_TX_DESC_LIMIT_0_MIN, \
		WLAN_CFG_TX_DESC_LIMIT_0_MAX, \
		WLAN_CFG_TX_DESC_LIMIT_0, \
		CFG_VALUE_OR_DEFAULT, "DP TX DESC limit 0")

#define CFG_DP_TX_DESC_LIMIT_1 \
		CFG_INI_UINT("dp_tx_desc_limit_1", \
		WLAN_CFG_TX_DESC_LIMIT_1_MIN, \
		WLAN_CFG_TX_DESC_LIMIT_1_MAX, \
		WLAN_CFG_TX_DESC_LIMIT_1, \
		CFG_VALUE_OR_DEFAULT, "DP TX DESC limit 1")

#define CFG_DP_TX_DESC_LIMIT_2 \
		CFG_INI_UINT("dp_tx_desc_limit_2", \
		WLAN_CFG_TX_DESC_LIMIT_2_MIN, \
		WLAN_CFG_TX_DESC_LIMIT_2_MAX, \
		WLAN_CFG_TX_DESC_LIMIT_2, \
		CFG_VALUE_OR_DEFAULT, "DP TX DESC limit 2")

#define CFG_DP_TX_DEVICE_LIMIT \
		CFG_INI_UINT("dp_tx_device_limit", \
		WLAN_CFG_TX_DEVICE_LIMIT_MIN, \
		WLAN_CFG_TX_DEVICE_LIMIT_MAX, \
		WLAN_CFG_TX_DEVICE_LIMIT, \
		CFG_VALUE_OR_DEFAULT, "DP TX DEVICE limit")

#define CFG_DP_TX_SW_INTERNODE_QUEUE \
		CFG_INI_UINT("dp_tx_sw_internode_queue", \
		WLAN_CFG_TX_SW_INTERNODE_QUEUE_MIN, \
		WLAN_CFG_TX_SW_INTERNODE_QUEUE_MAX, \
		WLAN_CFG_TX_SW_INTERNODE_QUEUE, \
		CFG_VALUE_OR_DEFAULT, "DP TX SW internode queue")

#define CFG_DP_RXDMA_MONITOR_BUF_RING \
		CFG_INI_UINT("dp_rxdma_monitor_buf_ring", \
		WLAN_CFG_RXDMA_MONITOR_BUF_RING_SIZE_MIN, \
		WLAN_CFG_RXDMA_MONITOR_BUF_RING_SIZE_MAX, \
		WLAN_CFG_RXDMA_MONITOR_BUF_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP RXDMA monitor buffer ring")

#define CFG_DP_RXDMA_MONITOR_DST_RING \
		CFG_INI_UINT("dp_rxdma_monitor_dst_ring", \
		WLAN_CFG_RXDMA_MONITOR_DST_RING_SIZE_MIN, \
		WLAN_CFG_RXDMA_MONITOR_DST_RING_SIZE_MAX, \
		WLAN_CFG_RXDMA_MONITOR_DST_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP RXDMA monitor destination ring")

#define CFG_DP_RXDMA_MONITOR_STATUS_RING \
		CFG_INI_UINT("dp_rxdma_monitor_status_ring", \
		WLAN_CFG_RXDMA_MONITOR_STATUS_RING_SIZE_MIN, \
		WLAN_CFG_RXDMA_MONITOR_STATUS_RING_SIZE_MAX, \
		WLAN_CFG_RXDMA_MONITOR_STATUS_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP RXDMA monitor status ring")

#define CFG_DP_RXDMA_MONITOR_DESC_RING \
		CFG_INI_UINT("dp_rxdma_monitor_desc_ring", \
		WLAN_CFG_RXDMA_MONITOR_DESC_RING_SIZE_MIN, \
		WLAN_CFG_RXDMA_MONITOR_DESC_RING_SIZE_MAX, \
		WLAN_CFG_RXDMA_MONITOR_DESC_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP RXDMA monitor destination ring")

#define CFG_DP_RXDMA_ERR_DST_RING \
		CFG_INI_UINT("dp_rxdma_err_dst_ring", \
		WLAN_CFG_RXDMA_ERR_DST_RING_SIZE_MIN, \
		WLAN_CFG_RXDMA_ERR_DST_RING_SIZE_MAX, \
		WLAN_CFG_RXDMA_ERR_DST_RING_SIZE, \
		CFG_VALUE_OR_DEFAULT, "RXDMA err destination ring")

#define CFG_DP_PER_PKT_LOGGING \
		CFG_INI_UINT("enable_verbose_debug", \
		0, 0xffff, 0, \
		CFG_VALUE_OR_DEFAULT, "Enable excessive per packet logging")

#define CFG_DP_TX_FLOW_START_QUEUE_OFFSET \
		CFG_INI_UINT("TxFlowStartQueueOffset", \
		0, 30, WLAN_CFG_TX_FLOW_START_QUEUE_OFFSET, \
		CFG_VALUE_OR_DEFAULT, "Start queue offset")

#define CFG_DP_TX_FLOW_STOP_QUEUE_TH \
		CFG_INI_UINT("TxFlowStopQueueThreshold", \
		0, 50, 15, \
		CFG_VALUE_OR_DEFAULT, "Stop queue Threshold")

#define CFG_DP_IPA_UC_TX_BUF_SIZE \
		CFG_INI_UINT("IpaUcTxBufSize", \
		0, 4096, WLAN_CFG_IPA_UC_TX_BUF_SIZE, \
		CFG_VALUE_OR_DEFAULT, "IPA tx buffer size")

#define CFG_DP_IPA_UC_TX_PARTITION_BASE \
		CFG_INI_UINT("IpaUcTxPartitionBase", \
		0, 9000, WLAN_CFG_IPA_UC_TX_PARTITION_BASE, \
		CFG_VALUE_OR_DEFAULT, "IPA tx partition base")

#define CFG_DP_IPA_UC_RX_IND_RING_COUNT \
		CFG_INI_UINT("IpaUcRxIndRingCount", \
		0, 2048, WLAN_CFG_IPA_UC_RX_IND_RING_COUNT, \
		CFG_VALUE_OR_DEFAULT, "IPA rx indication ring count")

#define CFG_DP_REORDER_OFFLOAD_SUPPORT \
		CFG_INI_UINT("gReorderOffloadSupported", \
		0, 1, 1, \
		CFG_VALUE_OR_DEFAULT, "Packet reordering offload to firmware")

#define CFG_DP_AP_STA_SECURITY_SEPERATION \
			CFG_INI_BOOL("gDisableIntraBssFwd", \
			false, "Disable intrs BSS Rx packets")

#define CFG_DP_ENABLE_DATA_STALL_DETECTION \
		CFG_INI_BOOL("gEnableDataStallDetection", \
		true, "Enable/Disable Data stall detection")

#define CFG_DP_RX_SW_DESC_WEIGHT \
		CFG_INI_UINT("dp_rx_sw_desc_weight", \
		WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE_MIN, \
		WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE_MAX, \
		WLAN_CFG_RX_SW_DESC_WEIGHT_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP RX SW DESC weight")

#define CFG_DP_RX_SW_DESC_NUM \
		CFG_INI_UINT("dp_rx_sw_desc_num", \
		WLAN_CFG_RX_SW_DESC_NUM_SIZE_MIN, \
		WLAN_CFG_RX_SW_DESC_NUM_SIZE_MAX, \
		WLAN_CFG_RX_SW_DESC_NUM_SIZE, \
		CFG_VALUE_OR_DEFAULT, "DP RX SW DESC num")

#define CFG_DP_RX_FLOW_SEARCH_TABLE_SIZE \
	CFG_INI_UINT("dp_rx_flow_search_table_size", \
		WLAN_CFG_RX_FLOW_SEARCH_TABLE_SIZE_MIN, \
		WLAN_CFG_RX_FLOW_SEARCH_TABLE_SIZE_MAX, \
		WLAN_CFG_RX_FLOW_SEARCH_TABLE_SIZE, \
		CFG_VALUE_OR_DEFAULT, \
		"DP Rx Flow Search Table Size in number of entries")

#define CFG_DP_RX_FLOW_TAG_ENABLE \
	CFG_INI_BOOL("dp_rx_flow_tag_enable", false, \
		     "Enable/Disable DP Rx Flow Tag")

#define CFG_DP_RX_FLOW_SEARCH_TABLE_PER_PDEV \
	CFG_INI_BOOL("dp_rx_per_pdev_flow_search", false, \
			"DP Rx Flow Search Table Is Per PDev")

#define CFG_DP_RX_MON_PROTOCOL_FLOW_TAG_ENABLE \
	CFG_INI_BOOL("dp_rx_monitor_protocol_flow_tag_enable", true, \
		     "Enable/Disable Rx Protocol & Flow tags in Monitor mode")

#define CFG_DP_TX_PER_PKT_VDEV_ID_CHECK \
	CFG_INI_BOOL("dp_tx_allow_per_pkt_vdev_id_check", false, \
		     "Enable/Disable tx Per Pkt vdev id check")

/*
 * <ini>
 * dp_rx_fisa_enable - Control Rx datapath FISA
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable DP Rx FISA feature
 *
 * Related: dp_rx_flow_search_table_size
 *
 * Supported Feature: STA,P2P and SAP IPA disabled terminating
 *
 * Usage: Internal/External
 *
 * </ini>
 */
#define CFG_DP_RX_FISA_ENABLE \
	CFG_INI_BOOL("dp_rx_fisa_enable", false, \
		     "Enable/Disable DP Rx FISA")

#define CFG_DP_RXDMA_MONITOR_RX_DROP_THRESHOLD \
		CFG_INI_UINT("mon_drop_thresh", \
		WLAN_CFG_RXDMA_MONITOR_RX_DROP_THRESH_SIZE_MIN, \
		WLAN_CFG_RXDMA_MONITOR_RX_DROP_THRESH_SIZE_MAX, \
		WLAN_CFG_RXDMA_MONITOR_RX_DROP_THRESH_SIZE, \
		CFG_VALUE_OR_DEFAULT, "RXDMA monitor rx drop theshold")

#define CFG_DP_PKTLOG_BUFFER_SIZE \
		CFG_INI_UINT("PktlogBufSize", \
		WLAN_CFG_PKTLOG_MIN_BUFFER_SIZE, \
		WLAN_CFG_PKTLOG_MAX_BUFFER_SIZE, \
		WLAN_CFG_PKTLOG_BUFFER_SIZE, \
		CFG_VALUE_OR_DEFAULT, "Packet Log buffer size")

#define CFG_DP_FULL_MON_MODE \
		CFG_INI_BOOL("full_mon_mode", \
		false, "Full Monitor mode support")

#define CFG_DP_REO_RINGS_MAP \
		CFG_INI_UINT("dp_reo_rings_map", \
		WLAN_CFG_NUM_REO_RINGS_MAP_MIN, \
		WLAN_CFG_NUM_REO_RINGS_MAP_MAX, \
		WLAN_CFG_NUM_REO_RINGS_MAP, \
		CFG_VALUE_OR_DEFAULT, "REO Destination Rings Mapping")

#define CFG_DP_RX_RADIO_0_DEFAULT_REO \
		CFG_INI_UINT("dp_rx_radio0_default_reo", \
		WLAN_CFG_RADIO_DEFAULT_REO_MIN, \
		WLAN_CFG_RADIO_DEFAULT_REO_MAX, \
		WLAN_CFG_RADIO_0_DEFAULT_REO, \
		CFG_VALUE_OR_DEFAULT, "Radio0 to REO destination default mapping")

#define CFG_DP_RX_RADIO_1_DEFAULT_REO \
		CFG_INI_UINT("dp_rx_radio1_default_reo", \
		WLAN_CFG_RADIO_DEFAULT_REO_MIN, \
		WLAN_CFG_RADIO_DEFAULT_REO_MAX, \
		WLAN_CFG_RADIO_1_DEFAULT_REO, \
		CFG_VALUE_OR_DEFAULT, "Radio1 to REO destination default mapping")

#define CFG_DP_RX_RADIO_2_DEFAULT_REO \
		CFG_INI_UINT("dp_rx_radio2_default_reo", \
		WLAN_CFG_RADIO_DEFAULT_REO_MIN, \
		WLAN_CFG_RADIO_DEFAULT_REO_MAX, \
		WLAN_CFG_RADIO_2_DEFAULT_REO, \
		CFG_VALUE_OR_DEFAULT, "Radio2 to REO destination default mapping")

#define CFG_DP_PEER_EXT_STATS \
		CFG_INI_BOOL("peer_ext_stats", \
		false, "Peer extended stats")
/*
 * <ini>
 * legacy_mode_csum_disable - Disable csum offload for legacy 802.11abg modes
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to disable HW checksum offload capability for legacy
 * connections
 *
 * Related: gEnableIpTcpUdpChecksumOffload should be enabled
 *
 * Usage: Internal
 *
 * </ini>
 */

#define CFG_DP_LEGACY_MODE_CSUM_DISABLE \
	CFG_INI_BOOL("legacy_mode_csum_disable", false, \
		     "Enable/Disable legacy mode checksum")

#define CFG_DP_RX_BUFF_POOL_ENABLE \
	CFG_INI_BOOL("dp_rx_buff_prealloc_pool", false, \
		     "Enable/Disable DP RX emergency buffer pool support")

#define CFG_DP_POLL_MODE_ENABLE \
		CFG_INI_BOOL("dp_poll_mode_enable", false, \
		"Enable/Disable Polling mode for data path")

#define CFG_DP_RX_FST_IN_CMEM \
	CFG_INI_BOOL("dp_rx_fst_in_cmem", false, \
		     "Enable/Disable flow search table in CMEM")
/*
 * <ini>
 * gEnableSWLM - Control DP Software latency manager
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to enable DP Software latency Manager
 *
 * Supported Feature: STA,P2P and SAP IPA disabled terminating
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_SWLM_ENABLE \
	CFG_INI_BOOL("gEnableSWLM", false, \
		     "Enable/Disable DP SWLM")
/*
 * <ini>
 * wow_check_rx_pending_enable - control to check RX frames pending in Wow
 * @Min: 0
 * @Max: 1
 * @Default: 0
 *
 * This ini is used to control DP Software to perform RX pending check
 * before entering WoW mode
 *
 * Usage: Internal
 *
 * </ini>
 */
#define CFG_DP_WOW_CHECK_RX_PENDING \
		CFG_INI_BOOL("wow_check_rx_pending_enable", \
		false, \
		"enable rx frame pending check in WoW mode")

#define WLAN_CFG_SEND_ALL_ICMP_REQ_TO_FW (-1)
#define WLAN_CFG_SEND_ICMP_REQ_TO_FW 0
#define WLAN_CFG_SEND_ICMP_REQ_TO_FW_MIN (-1)
#define WLAN_CFG_SEND_ICMP_REQ_TO_FW_MAX 100000

/*
 * <ini>
 * send_icmp_pkt_to_fw - Send ICMP Request packet to FW.
 * @Min: -1
 * @Max:  100000
 * @Default: 0
 *
 * This ini is used to control DP Software to send ICMP request packets to FW
 * at certain interval (in milliseconds).
 * The value 0 is used to disable sending the ICMP requests to FW.
 * The value -1 is used to send all ICMP requests to FW.
 * Any value greater than zero indicates the time interval (in milliseconds)
 * at which ICMP requests should be sent to FW.
 *
 * Usage: External
 *
 * </ini>
 */
#define CFG_DP_SEND_ICMP_REQ_TO_FW \
	CFG_INI_INT("send_icmp_req_to_fw", \
		    WLAN_CFG_SEND_ICMP_REQ_TO_FW_MIN, \
		    WLAN_CFG_SEND_ICMP_REQ_TO_FW_MAX, \
		    WLAN_CFG_SEND_ICMP_REQ_TO_FW, \
		    CFG_VALUE_OR_DEFAULT, "Send ICMP Request packets to FW")

#define CFG_DP \
		CFG(CFG_DP_HTT_PACKET_TYPE) \
		CFG(CFG_DP_INT_BATCH_THRESHOLD_OTHER) \
		CFG(CFG_DP_INT_BATCH_THRESHOLD_RX) \
		CFG(CFG_DP_INT_BATCH_THRESHOLD_TX) \
		CFG(CFG_DP_INT_TIMER_THRESHOLD_OTHER) \
		CFG(CFG_DP_INT_TIMER_THRESHOLD_RX) \
		CFG(CFG_DP_INT_TIMER_THRESHOLD_TX) \
		CFG(CFG_DP_MAX_ALLOC_SIZE) \
		CFG(CFG_DP_MAX_CLIENTS) \
		CFG(CFG_DP_MAX_PEER_ID) \
		CFG(CFG_DP_REO_DEST_RINGS) \
		CFG(CFG_DP_TCL_DATA_RINGS) \
		CFG(CFG_DP_NSS_REO_DEST_RINGS) \
		CFG(CFG_DP_NSS_TCL_DATA_RINGS) \
		CFG(CFG_DP_TX_DESC) \
		CFG(CFG_DP_TX_EXT_DESC) \
		CFG(CFG_DP_TX_EXT_DESC_POOLS) \
		CFG(CFG_DP_PDEV_RX_RING) \
		CFG(CFG_DP_PDEV_TX_RING) \
		CFG(CFG_DP_RX_DEFRAG_TIMEOUT) \
		CFG(CFG_DP_TX_COMPL_RING_SIZE) \
		CFG(CFG_DP_TX_RING_SIZE) \
		CFG(CFG_DP_NSS_COMP_RING_SIZE) \
		CFG(CFG_DP_PDEV_LMAC_RING) \
		CFG(CFG_DP_BASE_HW_MAC_ID) \
		CFG(CFG_DP_RX_HASH) \
		CFG(CFG_DP_TSO) \
		CFG(CFG_DP_LRO) \
		CFG(CFG_DP_SG) \
		CFG(CFG_DP_GRO) \
		CFG(CFG_DP_OL_TX_CSUM) \
		CFG(CFG_DP_OL_RX_CSUM) \
		CFG(CFG_DP_RAWMODE) \
		CFG(CFG_DP_PEER_FLOW_CTRL) \
		CFG(CFG_DP_NAPI) \
		CFG(CFG_DP_TCP_UDP_CKSUM_OFFLOAD) \
		CFG(CFG_DP_NAN_TCP_UDP_CKSUM_OFFLOAD) \
		CFG(CFG_DP_P2P_TCP_UDP_CKSUM_OFFLOAD) \
		CFG(CFG_DP_DEFRAG_TIMEOUT_CHECK) \
		CFG(CFG_DP_WBM_RELEASE_RING) \
		CFG(CFG_DP_TCL_CMD_CREDIT_RING) \
		CFG(CFG_DP_TCL_STATUS_RING) \
		CFG(CFG_DP_REO_REINJECT_RING) \
		CFG(CFG_DP_RX_RELEASE_RING) \
		CFG(CFG_DP_REO_EXCEPTION_RING) \
		CFG(CFG_DP_REO_CMD_RING) \
		CFG(CFG_DP_REO_STATUS_RING) \
		CFG(CFG_DP_RXDMA_BUF_RING) \
		CFG(CFG_DP_RXDMA_REFILL_RING) \
		CFG(CFG_DP_TX_DESC_LIMIT_0) \
		CFG(CFG_DP_TX_DESC_LIMIT_1) \
		CFG(CFG_DP_TX_DESC_LIMIT_2) \
		CFG(CFG_DP_TX_DEVICE_LIMIT) \
		CFG(CFG_DP_TX_SW_INTERNODE_QUEUE) \
		CFG(CFG_DP_RXDMA_MONITOR_BUF_RING) \
		CFG(CFG_DP_RXDMA_MONITOR_DST_RING) \
		CFG(CFG_DP_RXDMA_MONITOR_STATUS_RING) \
		CFG(CFG_DP_RXDMA_MONITOR_DESC_RING) \
		CFG(CFG_DP_RXDMA_ERR_DST_RING) \
		CFG(CFG_DP_PER_PKT_LOGGING) \
		CFG(CFG_DP_TX_FLOW_START_QUEUE_OFFSET) \
		CFG(CFG_DP_TX_FLOW_STOP_QUEUE_TH) \
		CFG(CFG_DP_IPA_UC_TX_BUF_SIZE) \
		CFG(CFG_DP_IPA_UC_TX_PARTITION_BASE) \
		CFG(CFG_DP_IPA_UC_RX_IND_RING_COUNT) \
		CFG(CFG_DP_REORDER_OFFLOAD_SUPPORT) \
		CFG(CFG_DP_AP_STA_SECURITY_SEPERATION) \
		CFG(CFG_DP_ENABLE_DATA_STALL_DETECTION) \
		CFG(CFG_DP_RX_SW_DESC_WEIGHT) \
		CFG(CFG_DP_RX_SW_DESC_NUM) \
		CFG(CFG_DP_RX_FLOW_SEARCH_TABLE_SIZE) \
		CFG(CFG_DP_RX_FLOW_TAG_ENABLE) \
		CFG(CFG_DP_RX_FLOW_SEARCH_TABLE_PER_PDEV) \
		CFG(CFG_DP_RX_MON_PROTOCOL_FLOW_TAG_ENABLE) \
		CFG(CFG_DP_RXDMA_MONITOR_RX_DROP_THRESHOLD) \
		CFG(CFG_DP_PKTLOG_BUFFER_SIZE) \
		CFG(CFG_DP_RX_FISA_ENABLE) \
		CFG(CFG_DP_FULL_MON_MODE) \
		CFG(CFG_DP_REO_RINGS_MAP) \
		CFG(CFG_DP_PEER_EXT_STATS) \
		CFG(CFG_DP_RX_BUFF_POOL_ENABLE) \
		CFG(CFG_DP_RX_PENDING_HL_THRESHOLD) \
		CFG(CFG_DP_RX_PENDING_LO_THRESHOLD) \
		CFG(CFG_DP_LEGACY_MODE_CSUM_DISABLE) \
		CFG(CFG_DP_POLL_MODE_ENABLE) \
		CFG(CFG_DP_SWLM_ENABLE) \
		CFG(CFG_DP_TX_PER_PKT_VDEV_ID_CHECK) \
		CFG(CFG_DP_RX_FST_IN_CMEM) \
		CFG(CFG_DP_RX_RADIO_0_DEFAULT_REO) \
		CFG(CFG_DP_RX_RADIO_1_DEFAULT_REO) \
		CFG(CFG_DP_RX_RADIO_2_DEFAULT_REO) \
		CFG(CFG_DP_WOW_CHECK_RX_PENDING) \
		CFG(CFG_DP_SEND_ICMP_REQ_TO_FW)
#endif /* _CFG_DP_H_ */
