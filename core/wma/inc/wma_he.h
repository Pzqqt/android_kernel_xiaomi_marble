/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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

#ifndef __WMA_HE_H
#define __WMA_HE_H

#include "wma.h"
#include "sir_api.h"

#ifdef WLAN_FEATURE_11AX
void wma_print_he_cap(tDot11fIEvendor_he_cap *he_cap);
void wma_print_he_ppet(void *ppet);
void wma_print_he_phy_cap(uint32_t *phy_cap);
void wma_print_he_mac_cap(uint32_t mac_cap);
void wma_print_he_op(tDot11fIEvendor_he_op *he_ops);
void wma_update_target_ext_he_cap(tp_wma_handle wma_handle,
	struct wma_tgt_cfg *tgt_cfg);
void wma_he_update_tgt_services(tp_wma_handle wma,
				struct wma_tgt_services *cfg);
void wma_populate_peer_he_cap(struct peer_assoc_params *peer,
			      tpAddStaParams params);
void wma_update_vdev_he_ops(struct wma_vdev_start_req *req,
		tpAddBssParams add_bss);
void wma_copy_txrxnode_he_ops(struct wma_txrx_node *node,
		struct wma_vdev_start_req *req);
void wma_copy_vdev_start_he_ops(struct vdev_start_params *params,
		struct wma_vdev_start_req *req);
void wma_vdev_set_he_bss_params(tp_wma_handle wma, uint8_t vdev_id,
				struct wma_vdev_start_req *req);

static inline bool wma_is_peer_he_capable(tpAddStaParams params)
{
	return params->he_capable;
}

void wma_update_vdev_he_capable(struct wma_vdev_start_req *req,
		tpSwitchChannelParams params);

#else
static inline void wma_print_he_cap(tDot11fIEvendor_he_cap *he_cap)
{
}

static inline void wma_print_he_ppet(void *ppet)
{
}

static inline void wma_print_he_phy_cap(uint32_t *phy_cap)
{
}

static inline void wma_print_he_mac_cap(uint32_t mac_cap)
{
}

static inline void wma_print_he_op(tDot11fIEvendor_he_op *he_ops)
{
}

static inline void wma_update_target_ext_he_cap(tp_wma_handle wma_handle,
						struct wma_tgt_cfg *tgt_cfg)
{
}

static inline void wma_he_update_tgt_services(tp_wma_handle wma,
					      struct wma_tgt_services *cfg)
{
	cfg->en_11ax = false;
	return;
}

static inline void wma_populate_peer_he_cap(struct peer_assoc_params *peer,
					    tpAddStaParams params)
{
}

static inline void wma_update_vdev_he_ops(struct wma_vdev_start_req *req,
			tpAddBssParams add_bss)
{
}
static inline void wma_copy_txrxnode_he_ops(struct wma_txrx_node *intr,
			struct wma_vdev_start_req *req)
{
}

static inline void wma_copy_vdev_start_he_ops(struct vdev_start_params *params,
			struct wma_vdev_start_req *req)
{
}

static inline void wma_vdev_set_he_bss_params(tp_wma_handle wma,
				uint8_t vdev_id, struct wma_vdev_start_req *req)
{
}

static inline bool wma_is_peer_he_capable(tpAddStaParams params)
{
	return false;
}

static inline void wma_update_vdev_he_capable(struct wma_vdev_start_req *req,
					      tpSwitchChannelParams params)
{
}
#endif

#endif /* __WMA_HE_H */
