/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.

 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC : lim_mlo.h
 *
 * WLAN Host Device Driver file for 802.11be (Extremely High Throughput)
 * support.
 *
 */

#if !defined(LIM_MLO_H)
#define LIM_MLO_H

#include "ani_global.h"
#include "lim_session.h"
#include <wlan_mlo_mgr_public_structs.h>

#ifdef WLAN_FEATURE_11BE_MLO

/**
 * lim_update_partner_link_info - Update partner link information
 *
 * This function is triggered from mlo mgr
 *
 * @vdev: vdev pointer
 *
 * Return: QDF_STATUS_SUCCESS on successful update link info else failure.
 */
QDF_STATUS lim_partner_link_info_change(struct wlan_objmgr_vdev *vdev);

/**
 * lim_mlo_free_vdev_ref() - release vdev reference
 * @vdev: vdev obj
 *
 * Return: void
 */
void lim_mlo_release_vdev_ref(struct wlan_objmgr_vdev *vdev);

/**
 * pe_find_partner_session_by_link_id() - Get partner session by link id
 * @session: pe session
 * @link_id: link id
 *
 * Return: partner session
 */
struct pe_session *pe_find_partner_session_by_link_id(
		struct pe_session *session, uint8_t link_id);

/**
 * lim_get_mlo_vdev_list() - Get mlo vdev list
 * @session: pe session
 * @vdev_count: vdev count
 * @wlan_vdev_list: vdev list
 *
 * Return: void
 */
void lim_get_mlo_vdev_list(struct pe_session *session, uint16_t *vdev_count,
			   struct wlan_objmgr_vdev **wlan_vdev_list);

/**
 * lim_mlo_notify_peer_disconn - trigger mlo to delete partner peer
 * @pe_session: pe session
 * @sta_ds: Pointer to internal STA Datastructure
 *
 * Return: void
 */
void lim_mlo_notify_peer_disconn(struct pe_session *pe_session,
				 tpDphHashNode sta_ds);

/**
 * lim_mlo_sta_notify_peer_disconn - trigger mlo to delete partner peer
 *                                   This API is only for MLO STA.
 * @pe_session: pe session
 *
 * Return: void
 */
void lim_mlo_sta_notify_peer_disconn(struct pe_session *pe_session);

/**
 * lim_mlo_cleanup_partner_peer() - cleanup given peer which is partner peer
 *                                  of mlo connection.
 *
 * This function is triggered from mlo mgr.
 *
 * @peer: pointer to peer to be cleanup
 *
 * Return: void
 */
void lim_mlo_cleanup_partner_peer(struct wlan_objmgr_peer *peer);

/**
 * lim_mlo_set_mld_mac_peer() - set mld mac
 * @sta_ds: Pointer to internal STA Datastructure
 * @peer_mld_addr: peer mld mac addr
 *
 * Return: void
 */
void lim_mlo_set_mld_mac_peer(tpDphHashNode sta_ds,
			      uint8_t peer_mld_addr[QDF_MAC_ADDR_SIZE]);

/**
 * lim_is_mlo_conn() - whether it is mlo connection
 * @session: pe session
 * @sta_ds: Pointer to internal STA Datastructure
 *
 * Return: true if it is mlo connection
 */
bool lim_is_mlo_conn(struct pe_session *session, tpDphHashNode sta_ds);

/**
 * lim_is_mlo_recv_assoc() - whether it received assoc frame or not
 * @sta_ds: Pointer to internal STA Datastructure
 *
 * Return: true if this peer corresponding link received assoc frame
 */
bool lim_is_mlo_recv_assoc(tpDphHashNode sta_ds);

/**
 * lim_set_mlo_recv_assoc() - set received assoc frame flag
 * @sta_ds: Pointer to internal STA Datastructure
 * @mlo_recv_assoc_frm: true if it received assoc frame
 *
 * Return: void
 */
void lim_set_mlo_recv_assoc(tpDphHashNode sta_ds, bool mlo_recv_assoc_frm);

/**
 * lim_mlo_proc_assoc_req_frm() - process assoc frame for mlo partner link
 *
 * This function is triggered by mlo mgr
 *
 * @vdev: pointer to vdev
 * @ml_peer: pointer to ml_peer
 * @link_addr: link addr
 * @frm_buf: assoc req buffer
 *
 * This function is called from mlo mgr.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS lim_mlo_proc_assoc_req_frm(struct wlan_objmgr_vdev *vdev,
				      struct wlan_mlo_peer_context *ml_peer,
				      struct qdf_mac_addr *link_addr,
				      qdf_nbuf_t buf);

/**
 * lim_mlo_ap_sta_assoc_suc() - process add sta rsp for mlo connection
 * @peer: pointer to peer to handle add sta response
 *
 * This function is triggered from mlo mgr.
 *
 * Return: void
 */
void lim_mlo_ap_sta_assoc_suc(struct wlan_objmgr_peer *peer);

/**
 * lim_ap_mlo_sta_peer_ind() - Indicate mlo mgr after receiving sta rsp
 *
 * @mac: pointer to mac_context
 * @pe_session: pe session
 * @sta_ds: Pointer to internal STA Datastructure
 * @add_sta_rsp_status: add sta rsp status
 *
 * Return: void
 */
void lim_ap_mlo_sta_peer_ind(struct mac_context *mac,
			     struct pe_session *pe_session,
			     tpDphHashNode sta,
			     bool add_sta_rsp_status);

/**
 * lim_mlo_partner_auth_type: update auth type from partner
 * @session: pe session
 * @partner_peer_idx: aid
 * @auth_type: auth type to update
 *
 * Return: true if auth type is gotten successfully
 */
bool lim_mlo_partner_auth_type(struct pe_session *session,
			       uint16_t partner_peer_idx,
			       tAniAuthType *auth_type);

/**
 * lim_mlo_ap_sta_assoc_fail() - process add sta rsp fail for mlo connection
 * @peer: pointer to peer to handle add sta response
 *
 * This function is triggered from mlo mgr.
 *
 * Return: void
 */
void lim_mlo_ap_sta_assoc_fail(struct wlan_objmgr_peer *peer);

/**
 * lim_mlo_delete_link_peer() - notify mlo mgr peer obj is deleted
 * @pe_session: pe session
 * @sta_ds: Pointer to internal STA Datastructure
 *
 * Return: void
 */
void lim_mlo_delete_link_peer(struct pe_session *pe_session,
			      tpDphHashNode sta_ds);

/**
 * lim_mlo_assoc_ind_upper_layer() - indicate assoc confirm to upper layer
 *                                   for mlo partner link
 * @mac: pointer to mac_context
 * @pe_session: pe session
 * @mlo_info: mlo partner information
 *
 * Return: QDF_STATUS
 */
QDF_STATUS lim_mlo_assoc_ind_upper_layer(struct mac_context *mac,
					 struct pe_session *pe_session,
					 struct mlo_partner_info *mlo_info);
void lim_mlo_save_mlo_info(tpDphHashNode sta_ds,
			   struct mlo_partner_info *mlo_info);
#else

static inline void lim_mlo_notify_peer_disconn(struct pe_session *pe_session,
					       tpDphHashNode sta_ds)
{
}

static inline void lim_mlo_sta_notify_peer_disconn(
						struct pe_session *pe_session)
{
}

static inline void lim_mlo_set_mld_mac_peer(
				tpDphHashNode sta_ds,
				uint8_t peer_mld_addr[QDF_MAC_ADDR_SIZE])
{
}

static inline void lim_mlo_cleanup_partner_peer(struct vdev_mlme_obj *vdev_mlme,
						struct wlan_objmgr_peer *peer)
{
}

static inline bool lim_is_mlo_conn(struct pe_session *session,
				   tpDphHashNode sta_ds)
{
	return false;
}

static inline bool lim_is_mlo_recv_assoc(tpDphHashNode sta_ds)
{
	return false;
}

static inline void lim_set_mlo_recv_assoc(tpDphHashNode sta_ds,
					  bool mlo_recv_assoc_frm)
{
}

static inline bool lim_mlo_partner_auth_type(struct pe_session *session,
					     uint16_t partner_peer_idx,
					     tAniAuthType *auth_type)
{
	return false;
}

static inline void lim_ap_mlo_sta_peer_ind(struct mac_context *mac,
					   struct pe_session *pe_session,
					   tpDphHashNode sta,
					   bool add_sta_rsp_status)
{
}

static inline void lim_mlo_delete_link_peer(struct pe_session *pe_session,
					    tpDphHashNode sta_ds)
{
}

static inline QDF_STATUS lim_mlo_assoc_ind_upper_layer(
					struct mac_context *mac,
					struct pe_session *pe_session,
					struct mlo_partner_info *mlo_info)
{
	return QDF_STATUS_SUCCESS;
}

static inline void lim_mlo_save_mlo_info(tpDphHashNode sta_ds,
					 struct mlo_partner_info *mlo_info)
{
}
#endif
#endif
