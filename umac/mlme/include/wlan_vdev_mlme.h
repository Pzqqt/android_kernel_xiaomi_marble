/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: Define VDEV MLME structure and APIs
 */
#ifndef _WLAN_VDEV_MLME_H_
#define _WLAN_VDEV_MLME_H_

struct vdev_mlme_obj;

/**
 * struct vdev_mlme_proto - VDEV protocol strucutre
 */
struct vdev_mlme_proto {
};

/**
 * enum beacon_update_op - Beacon update op type
 * @BEACON_INIT:      Initialize beacon
 * @BEACON_REINIT:    Re-initialize beacon
 * @BEACON_UPDATE:    Update dynamic fields of beacon
 * @BEACON_CSA:       Enable CSA IE
 * @BEACON_FREE:      Beacon buffer free
 */
enum beacon_update_op {
	BEACON_INIT,
	BEACON_REINIT,
	BEACON_UPDATE,
	BEACON_CSA,
	BEACON_FREE,
};

/**
 * struct vdev_mlme_ops - VDEV MLME operation callbacks strucutre
 * @mlme_vdev_validate_basic_params:   callback to validate basic params of VDEV
 * @mlme_vdev_reset_proto_params:      callback to Reset protocol params
 * @mlme_vdev_start_send:              callback to initiate actions of VDEV MLME
 *                                     start operation
 * @mlme_vdev_restart_send:            callback to initiate actions of VDEV MLME
 *                                     restart operation
 * @mlme_vdev_start_continue:          callback to initiate operations on
 *                                     LMAC/FW start response
 * @mlme_vdev_up_send:                 callback to initiate actions of VDEV MLME
 *                                     up operation
 * @mlme_vdev_notify_up_complete:      callback to notify VDEV MLME on moving to
 *                                     UP state
 * @mlme_vdev_update_beacon:           callback to initiate beacon update
 * @mlme_vdev_disconnect_peers:        callback to initiate disconnection of
 *                                     peers
 * @mlme_vdev_dfs_cac_timer_stop:      callback to stop the DFS CAC timer
 * @mlme_vdev_stop_send:               callback to initiate actions of VDEV MLME
 *                                     stop operation
 * @mlme_vdev_stop_continue:           callback to initiate operations on
 *                                     LMAC/FW stop response
 * @mlme_vdev_bss_peer_delete_continue:callback to initiate operations on BSS
 *                                     peer delete completion
 * @mlme_vdev_down_send:               callback to initiate actions of VDEV MLME
 *                                     down operation
 * @mlme_vdev_legacy_hdl_create:       callback to invoke creation of legacy
 *                                     vdev object
 * @mlme_vdev_legacy_hdl_post_create:  callback to invoke post creation actions
 *                                     of legacy vdev object
 * @mlme_vdev_legacy_hdl_destroy:      callback to invoke destroy of legacy
 *                                     vdev object
 */
struct vdev_mlme_ops {
	QDF_STATUS (*mlme_vdev_validate_basic_params)(
					struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_reset_proto_params)(
					struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_start_send)(struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_restart_send)(struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_start_continue)(struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_up_send)(struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_notify_up_complete)(
					struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_update_beacon)(struct vdev_mlme_obj *vdev_mlme,
					      enum beacon_update_op op);
	QDF_STATUS (*mlme_vdev_disconnect_peers)(
					struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_dfs_cac_timer_stop)(
					struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_stop_send)(struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_stop_continue)(struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_bss_peer_delete_continue)(
					struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_down_send)(struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_notify_down_complete)(
					struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_legacy_hdl_create)(
					struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_legacy_hdl_post_create)(
					struct vdev_mlme_obj *vdev_mlme);
	QDF_STATUS (*mlme_vdev_legacy_hdl_destroy)(
					struct vdev_mlme_obj *vdev_mlme);

};

/**
 * struct vdev_mlme_obj - VDEV MLME component object
 * @vdev_proto:           VDEV MLME proto substructure
 * @sm_lock:              VDEV SM lock
 * @sm_hdl:               VDEV SM handle
 * @ops:                  VDEV MLME callback table
 * @legacy_vdev_ptr:      VDEV MLME legacy pointer
 */
struct vdev_mlme_obj {
	struct vdev_mlme_proto vdev_proto;
#ifdef VDEV_SM_LOCK_SUPPORT
	qdf_spinlock_t sm_lock;
#endif
	struct wlan_sm *sm_hdl;
	struct wlan_objmgr_vdev *vdev;
	struct vdev_mlme_ops *ops;
	void *legacy_vdev_ptr;
};

/**
 * mlme_vdev_validate_basic_params - Validate basic params
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API validate MLME VDEV basic parameters
 *
 * Return: SUCCESS on successful validation
 *         FAILURE, if any parameter is not initialized
 */
static inline QDF_STATUS mlme_vdev_validate_basic_params(
				struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_validate_basic_params)
		ret = vdev_mlme->ops->mlme_vdev_validate_basic_params(
								vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_reset_proto_params - Reset VDEV protocol params
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API resets the protocol params fo vdev
 *
 * Return: SUCCESS on successful reset
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_reset_proto_params(
				struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_reset_proto_params)
		ret = vdev_mlme->ops->mlme_vdev_reset_proto_params(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_start_send - Invokes VDEV start operation
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API invokes VDEV start operation
 *
 * Return: SUCCESS on successful completion of start operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_start_send(struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_start_send)
		ret = vdev_mlme->ops->mlme_vdev_start_send(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_restart_send - Invokes VDEV restart operation
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API invokes VDEV restart operation
 *
 * Return: SUCCESS on successful completion of restart operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_restart_send(struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_restart_send)
		ret = vdev_mlme->ops->mlme_vdev_restart_send(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_start_continue - VDEV start response handling
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API invokes VDEV start response actions
 *
 * Return: SUCCESS on successful completion of start response operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_start_continue(
				struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_start_continue)
		ret = vdev_mlme->ops->mlme_vdev_start_continue(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_up_send - VDEV up operation
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API invokes VDEV up operations
 *
 * Return: SUCCESS on successful completion of up operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_up_send(struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_up_send)
		ret = vdev_mlme->ops->mlme_vdev_up_send(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_notify_up_complete - VDEV up state transition notification
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API notifies MLME on moving to UP state
 *
 * Return: SUCCESS on successful completion of up notification
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_notify_up_complete(
			      struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_notify_up_complete)
		ret = vdev_mlme->ops->mlme_vdev_notify_up_complete(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_update_beacon - Updates beacon
 * @vdev_mlme_obj:  VDEV MLME comp object
 * @op: beacon update type
 *
 * API updates/allocates/frees the beacon
 *
 * Return: SUCCESS on successful update of beacon
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_update_beacon(
						struct vdev_mlme_obj *vdev_mlme,
						enum beacon_update_op op)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_update_beacon)
		ret = vdev_mlme->ops->mlme_vdev_update_beacon(vdev_mlme, op);

	return ret;
}

/**
 * mlme_vdev_disconnect_peers - Disconnect peers
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API trigger stations disconnection with AP VDEV or AP disconnection with STA
 * VDEV
 *
 * Return: SUCCESS on successful invocation of station disconnection
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_disconnect_peers(
			      struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_disconnect_peers)
		ret = vdev_mlme->ops->mlme_vdev_disconnect_peers(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_dfs_cac_timer_stop - Stop CAC timer
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API stops the CAC timer through DFS API
 *
 * Return: SUCCESS on successful CAC timer stop
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_dfs_cac_timer_stop(
			      struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_dfs_cac_timer_stop)
		ret = vdev_mlme->ops->mlme_vdev_dfs_cac_timer_stop(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_stop_send - Invokes VDEV stop operation
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API invokes VDEV stop operation
 *
 * Return: SUCCESS on successful completion of stop operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_stop_send(struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_stop_send)
		ret = vdev_mlme->ops->mlme_vdev_stop_send(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_stop_continue - VDEV stop response handling
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API invokes VDEV stop response actions
 *
 * Return: SUCCESS on successful completion of stop response operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_stop_continue(
						struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_stop_continue)
		ret = vdev_mlme->ops->mlme_vdev_stop_continue(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_bss_peer_delete_continue - VDEV BSS peer delete complete
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API invokes VDEV BSS peer delete complete operation
 *
 * Return: SUCCESS on successful completion of BSS peer delete handling
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_bss_peer_delete_continue(
			      struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) &&
	    vdev_mlme->ops->mlme_vdev_bss_peer_delete_continue)
		ret = vdev_mlme->ops->mlme_vdev_bss_peer_delete_continue(
								vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_down_send - VDEV down operation
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API invokes VDEV down operation
 *
 * Return: SUCCESS on successful completion of VDEV down operation
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_down_send(struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_down_send)
		ret = vdev_mlme->ops->mlme_vdev_down_send(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_notify_down_complete - VDEV init state transition notification
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API notifies MLME on moving to INIT state
 *
 * Return: SUCCESS on successful completion of down notification
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_notify_down_complete(
					struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_notify_down_complete)
		ret = vdev_mlme->ops->mlme_vdev_notify_down_complete(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_legacy_hdl_create - VDEV legacy pointer allocation
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API invokes legacy pointer allocation and initialization
 *
 * Return: SUCCESS on successful creation of legacy handle
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_legacy_hdl_create(
			      struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_legacy_hdl_create)
		ret = vdev_mlme->ops->mlme_vdev_legacy_hdl_create(vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_legacy_hdl_post_create - VDEV post legacy pointer allocation
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API invokes post legacy pointer allocation operation
 *
 * Return: SUCCESS on successful creation of legacy handle
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_legacy_hdl_post_create(
				struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if (vdev_mlme->ops && vdev_mlme->ops->mlme_vdev_legacy_hdl_post_create)
		ret = vdev_mlme->ops->mlme_vdev_legacy_hdl_post_create(
								vdev_mlme);

	return ret;
}

/**
 * mlme_vdev_legacy_hdl_destroy - VDEV legacy pointer free
 * @vdev_mlme_obj:  VDEV MLME comp object
 *
 * API invokes legacy pointer free
 *
 * Return: SUCCESS on successful free of legacy handle
 *         FAILURE, if it fails due to any
 */
static inline QDF_STATUS mlme_vdev_legacy_hdl_destroy(
			      struct vdev_mlme_obj *vdev_mlme)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS;

	if ((vdev_mlme->ops) && vdev_mlme->ops->mlme_vdev_legacy_hdl_destroy)
		ret = vdev_mlme->ops->mlme_vdev_legacy_hdl_destroy(vdev_mlme);

	return ret;
}

#endif
