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
 * @file cdp_txrx_host_stats.h
 * @brief Define the host data path stats API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_HOST_STATS_H_
#define _CDP_TXRX_HOST_STATS_H_

#include <cdp_txrx_stats_struct.h>
/* WIN */
/* Need to remove the "req" parameter */
/* Need to rename the function to reflect the functionality "show" / "display"
 * WIN -- to figure out whether to change OSIF to converge (not an immediate AI)
 * */
#if WLAN_FEATURE_FASTPATH
int ol_txrx_host_stats_get(
	ol_txrx_vdev_handle vdev,
	struct ol_txrx_stats_req *req);


void
ol_txrx_host_stats_clr(ol_txrx_vdev_handle vdev);

void
ol_txrx_host_ce_stats(ol_txrx_vdev_handle vdev);

int
ol_txrx_stats_publish(ol_txrx_pdev_handle pdev, struct ol_txrx_stats *buf);
/**
 * @brief Enable enhanced stats functionality.
 *
 * @param pdev - the physical device object
 * @return - void
 */
void
ol_txrx_enable_enhanced_stats(ol_txrx_pdev_handle pdev);

/**
 * @brief Disable enhanced stats functionality.
 *
 * @param pdev - the physical device object
 * @return - void
 */
void
ol_txrx_disable_enhanced_stats(ol_txrx_pdev_handle pdev);

#if ENHANCED_STATS
/**
 * @brief Get the desired stats from the message.
 *
 * @param pdev - the physical device object
 * @param stats_base - stats buffer recieved from FW
 * @param type - stats type.
 * @return - pointer to requested stat identified by type
 */
uint32_t *ol_txrx_get_stats_base(ol_txrx_pdev_handle pdev,
	uint32_t *stats_base, uint32_t msg_len, uint8_t type);
#endif
#endif /* WLAN_FEATURE_FASTPATH*/
#if (HOST_SW_TSO_ENABLE || HOST_SW_TSO_SG_ENABLE)
void
ol_tx_print_tso_stats(
	ol_txrx_vdev_handle vdev);

void
ol_tx_rst_tso_stats(ol_txrx_vdev_handle vdev);
#endif /* HOST_SW_TSO_ENABLE || HOST_SW_TSO_SG_ENABLE */

#if HOST_SW_SG_ENABLE
void
ol_tx_print_sg_stats(
	ol_txrx_vdev_handle vdev);

void
ol_tx_rst_sg_stats(ol_txrx_vdev_handle vdev);
#endif /* HOST_SW_SG_ENABLE */

#if RX_CHECKSUM_OFFLOAD
void
ol_print_rx_cksum_stats(
	ol_txrx_vdev_handle vdev);

void
ol_rst_rx_cksum_stats(ol_txrx_vdev_handle vdev);
#endif /* RX_CHECKSUM_OFFLOAD */

#if (ATH_SUPPORT_IQUE && WLAN_FEATURE_FASTPATH)
A_STATUS
ol_txrx_host_me_stats(ol_txrx_vdev_handle vdev);
#endif /* WLAN_FEATURE_FASTPATH */
#if PEER_FLOW_CONTROL
extern void
ol_txrx_per_peer_stats(struct ol_txrx_pdev_t *pdev, char *addr);
#endif
#if WLAN_FEATURE_FASTPATH && PEER_FLOW_CONTROL
int ol_txrx_host_msdu_ttl_stats(
	ol_txrx_vdev_handle vdev,
	struct ol_txrx_stats_req *req);
#endif

#define BSS_CHAN_INFO_READ                        1
#define BSS_CHAN_INFO_READ_AND_CLEAR              2

#define TX_FRAME_TYPE_DATA 0
#define TX_FRAME_TYPE_MGMT 1
#define TX_FRAME_TYPE_BEACON 2

#if HOST_SW_LRO_ENABLE
void
ol_print_lro_stats(ol_txrx_vdev_handle vdev);

void
ol_reset_lro_stats(ol_txrx_vdev_handle vdev);
#endif /* HOST_SW_LRO_ENABLE */

#endif
