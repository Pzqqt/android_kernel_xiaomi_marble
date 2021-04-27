/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * DOC: contains MLO manager public file containing common functionality
 */
#ifndef _WLAN_MLO_MGR_CMN_H_
#define _WLAN_MLO_MGR_CMN_H_

#include <qdf_types.h>
#include <qdf_trace.h>
#include "wlan_mlo_mgr_public_structs.h"

#define mlo_alert(format, args...) \
		QDF_TRACE_FATAL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_err(format, args...) \
		QDF_TRACE_ERROR(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_warn(format, args...) \
		QDF_TRACE_WARN(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_info(format, args...) \
		QDF_TRACE_INFO(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_debug(format, args...) \
		QDF_TRACE_DEBUG(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_alert(format, args...) \
		QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_err(format, args...) \
		QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_warn(format, args...) \
		QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_info(format, args...) \
		QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_debug(format, args...) \
		QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_alert_rl(format, args...) \
		QDF_TRACE_FATAL_RL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_err_rl(format, args...) \
		QDF_TRACE_ERROR_RL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_warn_rl(format, args...) \
		QDF_TRACE_WARN_RL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_info_rl(format, args...) \
		QDF_TRACE_INFO_RL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_debug_rl(format, args...) \
		QDF_TRACE_DEBUG_RL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_alert_rl(format, args...) \
		QDF_TRACE_FATAL_RL_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_err_rl(format, args...) \
		QDF_TRACE_ERROR_RL_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_warn_rl(format, args...) \
		QDF_TRACE_WARN_RL_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_info_rl(format, args...) \
		QDF_TRACE_INFO_RL_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

#define mlo_nofl_debug_rl(format, args...) \
		QDF_TRACE_DEBUG_RL_NO_FL(QDF_MODULE_ID_MLO, format, ## args)

/**
 * mlo_get_link_information() - get partner link information
 * @mld_addr : MLD address
 * @info: partner link information
 *
 * Return: QDF_STATUS
 */
void mlo_get_link_information(struct qdf_mac_addr *mld_addr,
			      struct mlo_link_info *info);
/**
 * is_mlo_all_links_up() - check all the link status in a MLO device
 * @ml_dev: ML device context
 *
 * Return: QDF_STATUS
 */
void is_mlo_all_links_up(struct wlan_mlo_dev_context *ml_dev);

/**
 * mlo_get_vdev_by_link_id() - get vdev by link id
 * @vdev: vdev pointer
 * @link_id: link id
 *
 * Caller should make sure to release the reference of thus obtained vdev
 * by calling mlo_release_vdev_ref() after usage of vdev.
 *
 * Return: vdev object pointer to link id
 */
struct wlan_objmgr_vdev *mlo_get_vdev_by_link_id(
			struct wlan_objmgr_vdev *vdev,
			uint8_t link_id);

/**
 * mlo_release_vdev_ref() - release vdev reference
 * @vdev: vdev pointer
 *
 * Return: void
 */
void mlo_release_vdev_ref(struct wlan_objmgr_vdev *vdev);

/**
 * mlo_reg_mlme_ext_cb() - Function to register mlme extended callbacks
 * @ctx: Pointer to mlo manager global context
 * @ops: Pointer to the struct containing the callbacks
 *
 * Return: QDF_STATUS_SUCCESS on successful registration else failure
 */
QDF_STATUS mlo_reg_mlme_ext_cb(struct mlo_mgr_context *ctx,
			       struct mlo_mlme_ext_ops *ops);

/**
 * mlo_unreg_mlme_ext_cb() - Function to unregister mlme extended callbacks
 * @ctx: Pointer to mlo manager global context
 *
 * Return: QDF_STATUS_SUCCESS on success else failure
 */
QDF_STATUS mlo_unreg_mlme_ext_cb(struct mlo_mgr_context *ctx);

/**
 * mlo_mlme_validate_conn_req() - Validate connect request
 * @vdev: Object manager vdev
 * @ext_data: Data object to be passed to callback
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlo_mlme_validate_conn_req(struct wlan_objmgr_vdev *vdev,
				      void *ext_data);

/**
 * mlo_mlme_ext_create_link_vdev() - Create link vdev for ML STA
 * @vdev: Object manager vdev
 * @ext_data: Data object to be passed to callback
 *
 * Return: QDF_STATUS
 */
QDF_STATUS mlo_mlme_create_link_vdev(struct wlan_objmgr_vdev *vdev,
				     void *ext_data);

/**
 * mlo_mlme_peer_create() - Create mlo peer
 * @vdev: Object manager vdev
 * @ml_peer: MLO peer context
 * @addr: Peer addr
 * @frm_buf: Frame buffer for IE processing
 *
 * Return: void
 */
void mlo_mlme_peer_create(struct wlan_objmgr_vdev *vdev,
			  struct wlan_mlo_peer_context *ml_peer,
			  struct qdf_mac_addr *addr,
			  qdf_nbuf_t frm_buf);

/**
 * mlo_mlme_peer_assoc() - Send ML Peer assoc
 * @peer: Object manager peer
 *
 * Return: void
 */
void mlo_mlme_peer_assoc(struct wlan_objmgr_peer *peer);

/**
 * mlo_mlme_peer_assoc_fail() - Send ML Peer assoc fail
 * @peer: Object manager peer
 *
 * Return: void
 */
void mlo_mlme_peer_assoc_fail(struct wlan_objmgr_peer *peer);

/**
 * mlo_mlme_peer_delete() - Send ML Peer delete
 * @peer: Object manager peer
 *
 * Return: void
 */
void mlo_mlme_peer_delete(struct wlan_objmgr_peer *peer);

/**
 * mlo_mlme_peer_assoc_resp() - Initiate sending of Assoc response
 * @peer: Object manager peer
 *
 * Return: void
 */
void mlo_mlme_peer_assoc_resp(struct wlan_objmgr_peer *peer);

/**
 * mlo_get_link_vdev_ix() - Get index of link VDEV in MLD
 * @ml_dev: ML device context
 * @vdev: VDEV object
 *
 * Return: link index
 */

uint8_t mlo_get_link_vdev_ix(struct wlan_mlo_dev_context *mldev,
			     struct wlan_objmgr_vdev *vdev);
#endif
