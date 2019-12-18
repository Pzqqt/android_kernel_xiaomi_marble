/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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
#ifndef _DP_PEER_H_
#define _DP_PEER_H_

#include <qdf_types.h>
#include <qdf_lock.h>
#include "dp_types.h"

#define DP_INVALID_PEER_ID 0xffff

#define DP_FW_PEER_STATS_CMP_TIMEOUT_MSEC 5000
/**
 * __dp_peer_find_by_id() - Returns peer object given the peer id
 *
 * @soc		: core DP soc context
 * @peer_id	: peer id from peer object can be retrieved
 *
 * Return: struct dp_peer*: Pointer to DP peer object
 */
static inline struct dp_peer *
__dp_peer_find_by_id(struct dp_soc *soc,
		   uint16_t peer_id)
{
	struct dp_peer *peer;

	/* TODO: Hold lock */
	peer = (peer_id >= soc->max_peers) ? NULL :
				soc->peer_id_to_obj_map[peer_id];

	return peer;
}

#ifdef PEER_PROTECTED_ACCESS
/**
 * dp_peer_find_by_id() - Returns peer object given the peer id
 *                        if delete_in_progress in not set for peer
 *
 * @soc		: core DP soc context
 * @peer_id	: peer id from peer object can be retrieved
 *
 * Return: struct dp_peer*: Pointer to DP peer object
 */
static inline
struct dp_peer *dp_peer_find_by_id(struct dp_soc *soc,
				   uint16_t peer_id)
{
	struct dp_peer *peer;

	qdf_spin_lock_bh(&soc->peer_ref_mutex);
	peer = __dp_peer_find_by_id(soc, peer_id);
	if (!peer || (peer && peer->delete_in_progress)) {
		qdf_spin_unlock_bh(&soc->peer_ref_mutex);
		return NULL;
	}
	qdf_atomic_inc(&peer->ref_cnt);
	qdf_spin_unlock_bh(&soc->peer_ref_mutex);

	return peer;
}
#else
static inline struct dp_peer *
dp_peer_find_by_id(struct dp_soc *soc,
		   uint16_t peer_id)
{
	struct dp_peer *peer;

	peer = __dp_peer_find_by_id (soc, peer_id);
	if (peer && peer->delete_in_progress) {
		return NULL;
	}

	return peer;
}
#endif /* PEER_LOCK_REF_PROTECT */

void dp_print_ast_stats(struct dp_soc *soc);
void dp_rx_peer_map_handler(struct dp_soc *soc, uint16_t peer_id,
			    uint16_t hw_peer_id, uint8_t vdev_id,
			    uint8_t *peer_mac_addr, uint16_t ast_hash,
			    uint8_t is_wds);
void dp_rx_peer_unmap_handler(struct dp_soc *soc, uint16_t peer_id,
			      uint8_t vdev_id, uint8_t *peer_mac_addr,
			      uint8_t is_wds);
void dp_rx_sec_ind_handler(struct dp_soc *soc, uint16_t peer_id,
			   enum cdp_sec_type sec_type, int is_unicast,
			   u_int32_t *michael_key, u_int32_t *rx_pn);
uint8_t dp_get_peer_mac_addr_frm_id(struct cdp_soc_t *soc_handle,
		uint16_t peer_id, uint8_t *peer_mac);

int dp_peer_add_ast(struct dp_soc *soc, struct dp_peer *peer,
		uint8_t *mac_addr, enum cdp_txrx_ast_entry_type type,
		uint32_t flags);

void dp_peer_del_ast(struct dp_soc *soc, struct dp_ast_entry *ast_entry);

void dp_peer_ast_unmap_handler(struct dp_soc *soc,
			       struct dp_ast_entry *ast_entry);

int dp_peer_update_ast(struct dp_soc *soc, struct dp_peer *peer,
			struct dp_ast_entry *ast_entry,	uint32_t flags);

struct dp_ast_entry *dp_peer_ast_hash_find_by_pdevid(struct dp_soc *soc,
						     uint8_t *ast_mac_addr,
						     uint8_t pdev_id);

struct dp_ast_entry *dp_peer_ast_hash_find_soc(struct dp_soc *soc,
					       uint8_t *ast_mac_addr);

struct dp_ast_entry *dp_peer_ast_list_find(struct dp_soc *soc,
					   struct dp_peer *peer,
					   uint8_t *ast_mac_addr);

uint8_t dp_peer_ast_get_pdev_id(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry);


uint8_t dp_peer_ast_get_next_hop(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry);

void dp_peer_ast_set_type(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry,
				enum cdp_txrx_ast_entry_type type);

void dp_peer_ast_send_wds_del(struct dp_soc *soc,
			      struct dp_ast_entry *ast_entry);

void dp_peer_free_hmwds_cb(struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
			   struct cdp_soc *dp_soc,
			   void *cookie,
			   enum cdp_ast_free_status status);

void dp_peer_ast_hash_remove(struct dp_soc *soc,
			     struct dp_ast_entry *ase);

/*
 * dp_peer_find_by_id_exist - check if peer exists for given id
 * @soc: core DP soc context
 * @peer_id: peer id from peer object can be retrieved
 *
 * Return: true if peer exists of false otherwise
 */
bool dp_peer_find_by_id_valid(struct dp_soc *soc, uint16_t peer_id);

#define DP_AST_ASSERT(_condition) \
	do { \
		if (!(_condition)) { \
			dp_print_ast_stats(soc);\
			QDF_BUG(_condition); \
		} \
	} while (0)

/**
 * dp_peer_update_inactive_time - Update inactive time for peer
 * @pdev: pdev object
 * @tag_type: htt_tlv_tag type
 * #tag_buf: buf message
 */
void
dp_peer_update_inactive_time(struct dp_pdev *pdev, uint32_t tag_type,
			     uint32_t *tag_buf);

#ifndef QCA_MULTIPASS_SUPPORT
/**
 * dp_peer_set_vlan_id: set vlan_id for this peer
 * @cdp_soc: soc handle
 * @peer_mac: mac address
 * @vlan_id: vlan id for peer
 *
 * return: void
 */
static inline
void dp_peer_set_vlan_id(struct cdp_soc_t *cdp_soc,
			 struct cdp_vdev *vdev_handle, uint8_t *peer_mac,
			 uint16_t vlan_id)
{
}

/**
 * dp_set_vlan_groupkey: set vlan map for vdev
 * @cdp_soc: soc handle
 * @vdev_id: vdev_id
 * @vlan_id: vlan_id of peer
 * @group_key: group key for vlan
 *
 * return: set success/failure
 */
static inline
QDF_STATUS dp_set_vlan_groupkey(struct cdp_soc_t *soc, uint8_t vdev_id,
				uint16_t vlan_id, uint16_t group_key)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_peer_multipass_list_init: initialize multipass peer list
 * @vdev: pointer to vdev
 *
 * return: void
 */
static inline
void dp_peer_multipass_list_init(struct dp_vdev *vdev)
{
}

/**
 * dp_peer_multipass_list_remove: remove peer from special peer list
 * @peer: peer handle
 *
 * return: void
 */
static inline
void dp_peer_multipass_list_remove(struct dp_peer *peer)
{
}
#else
void dp_peer_set_vlan_id(struct cdp_soc_t *cdp_soc,
			 struct cdp_vdev *vdev_handle, uint8_t *peer_mac,
			 uint16_t vlan_id);
QDF_STATUS dp_set_vlan_groupkey(struct cdp_soc_t *soc, uint8_t vdev_id,
				uint16_t vlan_id, uint16_t group_key);
void dp_peer_multipass_list_init(struct dp_vdev *vdev);
void dp_peer_multipass_list_remove(struct dp_peer *peer);
#endif

/**
 * dp_peer_update_pkt_capture_params: Set Rx & Tx Capture flags for a peer
 * @is_rx_pkt_cap_enable: enable/disable Rx packet capture in monitor mode
 * @is_tx_pkt_cap_enable: enable/disable Tx packet capture in monitor mode
 * @peer_mac: MAC address for which the above need to be enabled/disabled
 *
 * Return: Success if Rx & Tx capture is enabled for peer, false otherwise
 */
QDF_STATUS
dp_peer_update_pkt_capture_params(struct cdp_pdev *pdev,
				  bool is_rx_pkt_cap_enable,
				  bool is_tx_pkt_cap_enable,
				  uint8_t *peer_mac);

/*
 * dp_rx_tid_delete_cb() - Callback to flush reo descriptor HW cache
 * after deleting the entries (ie., setting valid=0)
 *
 * @soc: DP SOC handle
 * @cb_ctxt: Callback context
 * @reo_status: REO command status
 */
void dp_rx_tid_delete_cb(struct dp_soc *soc,
			 void *cb_ctxt,
			 union hal_reo_status *reo_status);
#endif /* _DP_PEER_H_ */
