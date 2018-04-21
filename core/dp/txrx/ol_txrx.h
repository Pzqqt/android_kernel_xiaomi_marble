/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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

#ifndef _OL_TXRX__H_
#define _OL_TXRX__H_

#include <qdf_nbuf.h>           /* qdf_nbuf_t */
#include <cdp_txrx_cmn.h>       /* ol_txrx_vdev_t, etc. */
#include "cds_sched.h"
#include <cdp_txrx_handle.h>
#include <ol_txrx_types.h>
/*
 * Pool of tx descriptors reserved for
 * high-priority traffic, such as ARP/EAPOL etc
 * only for forwarding path.
 */
#define OL_TX_NON_FWD_RESERVE	100

ol_txrx_peer_handle ol_txrx_peer_get_ref_by_addr(ol_txrx_pdev_handle pdev,
						 u8 *peer_addr,
						 u8 *peer_id,
						 enum peer_debug_id_type
									dbg_id);

int  ol_txrx_peer_release_ref(ol_txrx_peer_handle peer,
			      enum peer_debug_id_type dbg_id);
/* ol_txrx_is_peer_eligible_for_deletion() - check if peer to be deleted
 * @peer: peer handler
 * @pdev: pointer to pdev
 *
 * Return: true if eligible for deletion else false
 */
bool ol_txrx_is_peer_eligible_for_deletion(ol_txrx_peer_handle peer,
					   struct ol_txrx_pdev_t *pdev);

/**
 * ol_tx_desc_pool_size_hl() - allocate tx descriptor pool size for HL systems
 * @ctrl_pdev: the control pdev handle
 *
 * Return: allocated pool size
 */
u_int16_t
ol_tx_desc_pool_size_hl(struct cdp_cfg *ctrl_pdev);

#ifndef OL_TX_AVG_FRM_BYTES
#define OL_TX_AVG_FRM_BYTES 1000
#endif

#ifndef OL_TX_DESC_POOL_SIZE_MIN_HL
#define OL_TX_DESC_POOL_SIZE_MIN_HL 500
#endif

#ifndef OL_TX_DESC_POOL_SIZE_MAX_HL
#define OL_TX_DESC_POOL_SIZE_MAX_HL 5000
#endif


#ifdef CONFIG_PER_VDEV_TX_DESC_POOL
#define TXRX_HL_TX_FLOW_CTRL_VDEV_LOW_WATER_MARK 400
#define TXRX_HL_TX_FLOW_CTRL_MGMT_RESERVED 100
#endif

#ifdef CONFIG_TX_DESC_HI_PRIO_RESERVE
#define TXRX_HL_TX_DESC_HI_PRIO_RESERVED 20
#endif

#if defined(CONFIG_HL_SUPPORT) && defined(FEATURE_WLAN_TDLS)

void
ol_txrx_hl_tdls_flag_reset(struct cdp_vdev *vdev, bool flag);
#else

static inline void
ol_txrx_hl_tdls_flag_reset(struct cdp_vdev *vdev, bool flag)
{
}
#endif

#ifdef WDI_EVENT_ENABLE
void *ol_get_pldev(struct cdp_pdev *txrx_pdev);
#else
static inline
void *ol_get_pldev(struct cdp_pdev *txrx_pdev)
{
	return NULL;
}
#endif

#ifdef QCA_SUPPORT_TXRX_LOCAL_PEER_ID
ol_txrx_peer_handle
ol_txrx_peer_find_by_local_id(struct cdp_pdev *pdev,
			      uint8_t local_peer_id);
ol_txrx_peer_handle
ol_txrx_peer_get_ref_by_local_id(struct cdp_pdev *ppdev,
				 uint8_t local_peer_id,
				 enum peer_debug_id_type dbg_id);
#endif /* QCA_SUPPORT_TXRX_LOCAL_PEER_ID */

/*
 * @nbuf: buffer which contains data to be displayed
 * @nbuf_paddr: physical address of the buffer
 * @len: defines the size of the data to be displayed
 *
 * Return: None
 */
void
ol_txrx_dump_pkt(qdf_nbuf_t nbuf, uint32_t nbuf_paddr, int len);

struct cdp_vdev *ol_txrx_get_vdev_from_vdev_id(uint8_t vdev_id);

void *ol_txrx_find_peer_by_addr(struct cdp_pdev *pdev,
				uint8_t *peer_addr,
				uint8_t *peer_id);

void htt_pkt_log_init(struct cdp_pdev *pdev_handle, void *scn);
void peer_unmap_timer_work_function(void *);
void peer_unmap_timer_handler(void *data);

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
int ol_txrx_register_tx_flow_control(uint8_t vdev_id,
				     ol_txrx_tx_flow_control_fp flow_control,
				     void *osif_fc_ctx,
				     ol_txrx_tx_flow_control_is_pause_fp
				     flow_control_is_pause);

int ol_txrx_deregister_tx_flow_control_cb(uint8_t vdev_id);

bool ol_txrx_get_tx_resource(uint8_t sta_id,
			     unsigned int low_watermark,
			     unsigned int high_watermark_offset);

int ol_txrx_ll_set_tx_pause_q_depth(uint8_t vdev_id, int pause_q_depth);
#endif

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
void ol_tx_set_desc_global_pool_size(uint32_t num_msdu_desc);
uint32_t ol_tx_get_total_free_desc(struct ol_txrx_pdev_t *pdev);
static inline
uint32_t ol_tx_get_desc_global_pool_size(struct ol_txrx_pdev_t *pdev)
{
	return pdev->num_msdu_desc;
}

QDF_STATUS ol_txrx_register_pause_cb(struct cdp_soc_t *soc,
				     tx_pause_callback pause_cb);
/**
 * ol_txrx_fwd_desc_thresh_check() - check to forward packet to tx path
 * @vdev: which virtual device the frames were addressed to
 *
 * This API is to check whether enough descriptors are available or not
 * to forward packet to tx path. If not enough descriptors left,
 * start dropping tx-path packets.
 * Do not pause netif queues as still a pool of descriptors is reserved
 * for high-priority traffic such as EAPOL/ARP etc.
 * In case of intra-bss forwarding, it could be possible that tx-path can
 * consume all the tx descriptors and pause netif queues. Due to this,
 * there would be some left for stack triggered packets such as ARP packets
 * which could lead to disconnection of device. To avoid this, reserved
 * a pool of descriptors for high-priority packets, i.e., reduce the
 * threshold of drop in the intra-bss forwarding path.
 *
 * Return: true ; forward the packet, i.e., below threshold
 *         false; not enough descriptors, drop the packet
 */
bool ol_txrx_fwd_desc_thresh_check(struct ol_txrx_vdev_t *vdev);
#else
/**
 * ol_tx_get_desc_global_pool_size() - get global pool size
 * @pdev: pdev handle
 *
 * Return: global pool size
 */
static inline
uint32_t ol_tx_get_desc_global_pool_size(struct ol_txrx_pdev_t *pdev)
{
	return ol_cfg_target_tx_credit(pdev->ctrl_pdev);
}

/**
 * ol_tx_get_total_free_desc() - get total free descriptors
 * @pdev: pdev handle
 *
 * Return: total free descriptors
 */
static inline
uint32_t ol_tx_get_total_free_desc(struct ol_txrx_pdev_t *pdev)
{
	return pdev->tx_desc.num_free;
}

static inline
bool ol_txrx_fwd_desc_thresh_check(struct ol_txrx_vdev_t *vdev)
{
	return true;
}

#endif
#endif /* _OL_TXRX__H_ */
