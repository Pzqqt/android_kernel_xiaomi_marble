/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

/**
 * dp_peer_find_by_id() - Returns peer object given the peer id
 *                        if delete_in_progress in not set for peer
 *
 * @soc		: core DP soc context
 * @peer_id	: peer id from peer object can be retrieved
 *
 * Return: struct dp_peer*: Pointer to DP peer object
 */
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

void dp_rx_peer_map_handler(void *soc_handle, uint16_t peer_id,
	uint16_t hw_peer_id, uint8_t vdev_id, uint8_t *peer_mac_addr);
void dp_rx_peer_unmap_handler(void *soc_handle, uint16_t peer_id);
void dp_rx_sec_ind_handler(void *soc_handle, uint16_t peer_id,
	enum htt_sec_type sec_type, int is_unicast,
	u_int32_t *michael_key, u_int32_t *rx_pn);
uint8_t dp_get_peer_mac_addr_frm_id(struct cdp_soc_t *soc_handle,
		uint16_t peer_id, uint8_t *peer_mac);

int dp_peer_add_ast(struct dp_soc *soc, struct dp_peer *peer,
		uint8_t *mac_addr, enum cdp_txrx_ast_entry_type type,
		uint32_t flags);

void dp_peer_del_ast(struct dp_soc *soc, struct dp_ast_entry *ast_entry);

int dp_peer_update_ast(struct dp_soc *soc, struct dp_peer *peer,
			struct dp_ast_entry *ast_entry,	uint32_t flags);

struct dp_ast_entry *dp_peer_ast_hash_find(struct dp_soc *soc,
						uint8_t *ast_mac_addr);

uint8_t dp_peer_ast_get_pdev_id(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry);


uint8_t dp_peer_ast_get_next_hop(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry);

void dp_peer_ast_set_type(struct dp_soc *soc,
				struct dp_ast_entry *ast_entry,
				enum cdp_txrx_ast_entry_type type);

/*
 * dp_get_vdev_from_soc_vdev_id_wifi3() -
 * Returns vdev object given the vdev id
 * vdev id is unique across pdev's
 *
 * @soc         : core DP soc context
 * @vdev_id     : vdev id from vdev object can be retrieved
 *
 * Return: struct dp_vdev*: Pointer to DP vdev object
 */
static inline struct dp_vdev *
dp_get_vdev_from_soc_vdev_id_wifi3(struct dp_soc *soc,
					uint8_t vdev_id)
{
	struct dp_pdev *pdev = NULL;
	struct dp_vdev *vdev = NULL;
	int i;

	for (i = 0; i < MAX_PDEV_CNT && soc->pdev_list[i]; i++) {
		pdev = soc->pdev_list[i];
		TAILQ_FOREACH(vdev, &pdev->vdev_list, vdev_list_elem) {
			if (vdev->vdev_id == vdev_id) {
				QDF_TRACE(QDF_MODULE_ID_DP,
					QDF_TRACE_LEVEL_INFO,
					FL("Found vdev 0x%pK on pdev %d\n"),
					vdev, i);
				return vdev;
			}
		}
	}
	return NULL;

}
#endif /* _DP_PEER_H_ */
