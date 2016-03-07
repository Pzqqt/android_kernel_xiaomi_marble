/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_cfg.h
 * @brief Define the host data path configuration API functions
 */
#ifndef _CDP_TXRX_CFG_H_
#define _CDP_TXRX_CFG_H_

/**
 * struct txrx_pdev_cfg_param_t - configuration information
 * passed to the data path
 */
struct txrx_pdev_cfg_param_t {
	uint8_t is_full_reorder_offload;
	/* IPA Micro controller data path offload enable flag */
	uint8_t is_uc_offload_enabled;
	/* IPA Micro controller data path offload TX buffer count */
	uint32_t uc_tx_buffer_count;
	/* IPA Micro controller data path offload TX buffer size */
	uint32_t uc_tx_buffer_size;
	/* IPA Micro controller data path offload RX indication ring count */
	uint32_t uc_rx_indication_ring_count;
	/* IPA Micro controller data path offload TX partition base */
	uint32_t uc_tx_partition_base;
	/* IP, TCP and UDP checksum offload */
	bool ip_tcp_udp_checksum_offload;
	/* Rx processing in thread from TXRX */
	bool enable_rxthread;
	/* CE classification enabled through INI */
	bool ce_classify_enabled;
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	/* Threshold to stop queue in percentage */
	uint32_t tx_flow_stop_queue_th;
	/* Start queue offset in percentage */
	uint32_t tx_flow_start_queue_offset;
#endif
};

void ol_set_cfg_rx_fwd_disabled(ol_pdev_handle pdev, uint8_t disable_rx_fwd);

void ol_set_cfg_packet_log_enabled(ol_pdev_handle pdev, uint8_t val);

ol_pdev_handle ol_pdev_cfg_attach(qdf_device_t osdev,
			 struct txrx_pdev_cfg_param_t cfg_param);

void ol_vdev_rx_set_intrabss_fwd(ol_txrx_vdev_handle vdev, bool val);

/**
 * ol_txrx_get_opmode() - Return operation mode of vdev
 * @vdev: vdev handle
 *
 * Return: operation mode.
 */
int ol_txrx_get_opmode(ol_txrx_vdev_handle vdev);

/**
 * ol_txrx_is_rx_fwd_disabled() - returns the rx_fwd_disabled status on vdev
 * @vdev: vdev handle
 *
 * Return: Rx Fwd disabled status
 */
uint8_t
ol_txrx_is_rx_fwd_disabled(ol_txrx_vdev_handle vdev);
#endif /* _CDP_TXRX_CFG_H_ */
