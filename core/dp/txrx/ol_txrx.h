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

#ifndef _OL_TXRX__H_
#define _OL_TXRX__H_

#include <qdf_nbuf.h>           /* qdf_nbuf_t */
#include <cdp_txrx_cmn.h>       /* ol_txrx_vdev_t, etc. */
#include "cds_sched.h"
#include <cdp_txrx_handle.h>
#include <ol_txrx_types.h>
#include <ol_txrx_internal.h>

/*
 * Pool of tx descriptors reserved for
 * high-priority traffic, such as ARP/EAPOL etc
 * only for forwarding path.
 */
#define OL_TX_NON_FWD_RESERVE	100

/**
 * enum ol_txrx_fc_limit_id - Flow control identifier for
 * vdev limits based on band, channel bw and number of spatial streams
 * @TXRX_FC_5GH_80M_2x2: Limit for 5GHz, 80MHz BW, 2x2 NSS
 * @TXRX_FC_5GH_40M_2x2:
 * @TXRX_FC_5GH_20M_2x2:
 * @TXRX_FC_5GH_80M_1x1:
 * @TXRX_FC_5GH_40M_1x1:
 * @TXRX_FC_5GH_20M_1x1:
 * @TXRX_FC_2GH_40M_2x2:
 * @TXRX_FC_2GH_20M_2x2:
 * @TXRX_FC_2GH_40M_1x1:
 * @TXRX_FC_2GH_20M_1x1:
 */
enum ol_txrx_fc_limit_id {
	TXRX_FC_5GH_80M_2x2,
	TXRX_FC_5GH_40M_2x2,
	TXRX_FC_5GH_20M_2x2,
	TXRX_FC_5GH_80M_1x1,
	TXRX_FC_5GH_40M_1x1,
	TXRX_FC_5GH_20M_1x1,
	TXRX_FC_2GH_40M_2x2,
	TXRX_FC_2GH_20M_2x2,
	TXRX_FC_2GH_40M_1x1,
	TXRX_FC_2GH_20M_1x1,
	TXRX_FC_MAX
};

#define TXRX_RFS_ENABLE_PEER_ID_UNMAP_COUNT    3
#define TXRX_RFS_DISABLE_PEER_ID_UNMAP_COUNT   1

ol_txrx_peer_handle ol_txrx_peer_get_ref_by_addr(ol_txrx_pdev_handle pdev,
						 u8 *peer_addr,
						 u8 *peer_id,
						 enum peer_debug_id_type
									dbg_id);

int  ol_txrx_peer_release_ref(ol_txrx_peer_handle peer,
			      enum peer_debug_id_type dbg_id);

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

#ifndef FW_STATS_DESC_POOL_SIZE
#define FW_STATS_DESC_POOL_SIZE 10
#endif

#ifdef CONFIG_PER_VDEV_TX_DESC_POOL
#define TXRX_HL_TX_FLOW_CTRL_VDEV_LOW_WATER_MARK 400
#define TXRX_HL_TX_FLOW_CTRL_MGMT_RESERVED 100
#endif

#define TXRX_HL_TX_DESC_HI_PRIO_RESERVED 20
#define TXRX_HL_TX_DESC_QUEUE_RESTART_TH \
		(TXRX_HL_TX_DESC_HI_PRIO_RESERVED + 100)

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

/**
 * ol_txrx_get_mon_vdev_from_pdev() - get monitor mode vdev from pdev
 * @ppdev: the physical device the virtual device belongs to
 *
 * Return: vdev handle
 *         NULL if not found.
 */
struct cdp_vdev *ol_txrx_get_mon_vdev_from_pdev(struct cdp_pdev *ppdev);

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

void ol_tx_init_pdev(ol_txrx_pdev_handle pdev);

#ifdef CONFIG_HL_SUPPORT
void ol_txrx_vdev_txqs_init(struct ol_txrx_vdev_t *vdev);
void ol_txrx_vdev_tx_queue_free(struct ol_txrx_vdev_t *vdev);
void ol_txrx_peer_txqs_init(struct ol_txrx_pdev_t *pdev,
			    struct ol_txrx_peer_t *peer);
void ol_txrx_peer_tx_queue_free(struct ol_txrx_pdev_t *pdev,
				struct ol_txrx_peer_t *peer);
#else
static inline void
ol_txrx_vdev_txqs_init(struct ol_txrx_vdev_t *vdev) {}

static inline void
ol_txrx_vdev_tx_queue_free(struct ol_txrx_vdev_t *vdev) {}

static inline void
ol_txrx_peer_txqs_init(struct ol_txrx_pdev_t *pdev,
		       struct ol_txrx_peer_t *peer) {}

static inline void
ol_txrx_peer_tx_queue_free(struct ol_txrx_pdev_t *pdev,
			   struct ol_txrx_peer_t *peer) {}
#endif

#if defined(CONFIG_HL_SUPPORT) && defined(DEBUG_HL_LOGGING)
void ol_txrx_pdev_txq_log_init(struct ol_txrx_pdev_t *pdev);
void ol_txrx_pdev_txq_log_destroy(struct ol_txrx_pdev_t *pdev);
void ol_txrx_pdev_grp_stats_init(struct ol_txrx_pdev_t *pdev);
void ol_txrx_pdev_grp_stat_destroy(struct ol_txrx_pdev_t *pdev);
#else
static inline void
ol_txrx_pdev_txq_log_init(struct ol_txrx_pdev_t *pdev) {}

static inline void
ol_txrx_pdev_txq_log_destroy(struct ol_txrx_pdev_t *pdev) {}

static inline void
ol_txrx_pdev_grp_stats_init(struct ol_txrx_pdev_t *pdev) {}

static inline void
ol_txrx_pdev_grp_stat_destroy(struct ol_txrx_pdev_t *pdev) {}
#endif

#if defined(CONFIG_HL_SUPPORT) && defined(FEATURE_WLAN_TDLS)
void ol_txrx_copy_mac_addr_raw(struct cdp_vdev *pvdev, uint8_t *bss_addr);
void ol_txrx_add_last_real_peer(struct cdp_pdev *ppdev,
				struct cdp_vdev *pvdev, uint8_t *peer_id);
bool is_vdev_restore_last_peer(void *ppeer);
void ol_txrx_update_last_real_peer(struct cdp_pdev *ppdev, void *pvdev,
				   uint8_t *peer_id, bool restore_last_peer);
#endif

#if defined(FEATURE_TSO) && defined(FEATURE_TSO_DEBUG)
void ol_txrx_stats_display_tso(ol_txrx_pdev_handle pdev);
void ol_txrx_tso_stats_init(ol_txrx_pdev_handle pdev);
void ol_txrx_tso_stats_deinit(ol_txrx_pdev_handle pdev);
void ol_txrx_tso_stats_clear(ol_txrx_pdev_handle pdev);
#else
static inline
void ol_txrx_stats_display_tso(ol_txrx_pdev_handle pdev)
{
	ol_txrx_err("TSO is not supported\n");
}

static inline
void ol_txrx_tso_stats_init(ol_txrx_pdev_handle pdev) {}

static inline
void ol_txrx_tso_stats_deinit(ol_txrx_pdev_handle pdev) {}

static inline
void ol_txrx_tso_stats_clear(ol_txrx_pdev_handle pdev) {}
#endif

struct ol_tx_desc_t *
ol_txrx_mgmt_tx_desc_alloc(struct ol_txrx_pdev_t *pdev,
			   struct ol_txrx_vdev_t *vdev,
			   qdf_nbuf_t tx_mgmt_frm,
			   struct ol_txrx_msdu_info_t *tx_msdu_info);

int ol_txrx_mgmt_send_frame(struct ol_txrx_vdev_t *vdev,
			    struct ol_tx_desc_t *tx_desc,
			    qdf_nbuf_t tx_mgmt_frm,
			    struct ol_txrx_msdu_info_t *tx_msdu_info,
			    uint16_t chanfreq);

#ifdef CONFIG_HL_SUPPORT
static inline
uint32_t ol_tx_get_desc_global_pool_size(struct ol_txrx_pdev_t *pdev)
{
	return ol_tx_desc_pool_size_hl(pdev->ctrl_pdev);
}
#else
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
static inline
uint32_t ol_tx_get_desc_global_pool_size(struct ol_txrx_pdev_t *pdev)
{
	return pdev->num_msdu_desc;
}
#else
static inline
uint32_t ol_tx_get_desc_global_pool_size(struct ol_txrx_pdev_t *pdev)
{
	return ol_cfg_target_tx_credit(pdev->ctrl_pdev);
}
#endif
#endif

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
void ol_tx_set_desc_global_pool_size(uint32_t num_msdu_desc);
uint32_t ol_tx_get_total_free_desc(struct ol_txrx_pdev_t *pdev);
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
bool ol_txrx_fwd_desc_thresh_check(struct cdp_vdev *vdev);

/**
 * ol_tx_desc_thresh_reached() - is tx desc threshold reached
 * @vdev: vdev handle
 *
 * Return: true if tx desc available reached threshold or false otherwise
 */
static inline bool ol_tx_desc_thresh_reached(struct cdp_vdev *vdev)
{
	return !(ol_txrx_fwd_desc_thresh_check(vdev));
}

#else
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
bool ol_txrx_fwd_desc_thresh_check(struct cdp_vdev *vdev)
{
	return true;
}

#endif

#if defined(FEATURE_HL_GROUP_CREDIT_FLOW_CONTROL) && \
	defined(FEATURE_HL_DBS_GROUP_CREDIT_SHARING)
static inline void
ol_txrx_init_txq_group_limit_lend(struct ol_txrx_pdev_t *pdev)
{
	BUILD_BUG_ON(OL_TX_MAX_GROUPS_PER_QUEUE > 1);
	BUILD_BUG_ON(OL_TX_MAX_TXQ_GROUPS > 2);
	pdev->limit_lend = 0;
	pdev->min_reserve = 0;
}
#else
static inline void
ol_txrx_init_txq_group_limit_lend(struct ol_txrx_pdev_t *pdev)
{}
#endif

int ol_txrx_fw_stats_desc_pool_init(struct ol_txrx_pdev_t *pdev,
				    uint8_t pool_size);
void ol_txrx_fw_stats_desc_pool_deinit(struct ol_txrx_pdev_t *pdev);
struct ol_txrx_fw_stats_desc_t
	*ol_txrx_fw_stats_desc_alloc(struct ol_txrx_pdev_t *pdev);
struct ol_txrx_stats_req_internal
	*ol_txrx_fw_stats_desc_get_req(struct ol_txrx_pdev_t *pdev,
				       uint8_t desc_id);

#ifdef QCA_HL_NETDEV_FLOW_CONTROL
int ol_txrx_register_hl_flow_control(struct cdp_soc_t *soc,
				     tx_pause_callback flowcontrol);
int ol_txrx_set_vdev_os_queue_status(u8 vdev_id, enum netif_action_type action);
int ol_txrx_set_vdev_tx_desc_limit(u8 vdev_id, u8 chan);
#endif

/**
 * ol_txrx_get_new_htt_msg_format() - check htt h2t msg feature
 * @pdev - datapath device instance
 *
 * Check if h2t message length includes htc header length
 *
 * return if new htt h2t msg feature enabled
 */
bool ol_txrx_get_new_htt_msg_format(struct ol_txrx_pdev_t *pdev);

/**
 * ol_txrx_set_new_htt_msg_format() - set htt h2t msg feature
 * @val - enable or disable new htt h2t msg feature
 *
 * Set if h2t message length includes htc header length
 *
 * return NONE
 */
void ol_txrx_set_new_htt_msg_format(uint8_t val);

/**
 * ol_txrx_set_peer_unmap_conf_support() - set peer unmap conf feature
 * @val - enable or disable peer unmap conf feature
 *
 * Set if peer unamp conf feature is supported by both FW and in INI
 *
 * return NONE
 */
void ol_txrx_set_peer_unmap_conf_support(bool val);

/**
 * ol_txrx_get_peer_unmap_conf_support() - check peer unmap conf feature
 *
 * Check if peer unmap conf feature is enabled
 *
 * return true is peer unmap conf feature is enabled else false
 */
bool ol_txrx_get_peer_unmap_conf_support(void);
#endif /* _OL_TXRX__H_ */
