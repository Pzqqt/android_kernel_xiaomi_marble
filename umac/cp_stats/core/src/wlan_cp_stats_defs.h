/*
 * Copyright (c) 2018-2019, 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_cp_stats_defs.h
 *
 * This header file maintains core definitions of control plane statistics
 * component
 */

#ifndef __WLAN_CP_STATS_DEFS_H__
#define __WLAN_CP_STATS_DEFS_H__

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include "wlan_cp_stats_cmn_defs.h"
#include <wlan_cp_stats_utils_api.h>
#include <wlan_cp_stats_ext_type.h>
#include <wlan_cp_stats_public_structs.h>
#ifdef WLAN_FEATURE_MIB_STATS
#include <wlan_cp_stats_mc_defs.h>
#endif

/* noise floor */
#define CP_STATS_TGT_NOISE_FLOOR_DBM (-96)

#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
typedef void (*get_infra_cp_stats_cb)(struct infra_cp_stats_event *ev,
				      void *cookie);
#endif

/**
 * struct psoc_cp_stats - defines cp stats at psoc object
 * @psoc_obj: pointer to psoc
 * @psoc_comp_priv_obj[]: component's private object pointers
 * @psoc_cp_stats_lock: lock to protect object
 * @cmn_stats: stats common for AP and STA devices
 * @obj_stats: stats specific to AP or STA devices
 * @legacy_stats_cb: callback to update the stats received from FW through
 * asynchronous events.
 */
struct psoc_cp_stats {
	struct wlan_objmgr_psoc *psoc_obj;
	void *psoc_comp_priv_obj[WLAN_CP_STATS_MAX_COMPONENTS];
	qdf_spinlock_t psoc_cp_stats_lock;
	struct psoc_cmn_cp_stats *cmn_stats;
	psoc_ext_cp_stats_t *obj_stats;
	void (*legacy_stats_cb)(void *stats);
#ifdef WLAN_SUPPORT_INFRA_CTRL_PATH_STATS
	void (*get_infra_cp_stats)(struct infra_cp_stats_event *ev,
				   void *cookie);
	void *infra_cp_stats_req_context;
#endif
};

/**
 * struct pdev_cp_stats - defines cp stats at pdev object
 * @pdev_obj: pointer to pdev
 * @pdev_stats: pointer to ic/mc specific stats
 * @pdev_comp_priv_obj[]: component's private object pointers
 * @pdev_cp_stats_lock:	lock to protect object
 */
struct pdev_cp_stats {
	struct wlan_objmgr_pdev  *pdev_obj;
	pdev_ext_cp_stats_t *pdev_stats;
	void *pdev_comp_priv_obj[WLAN_CP_STATS_MAX_COMPONENTS];
	qdf_spinlock_t pdev_cp_stats_lock;
};

/**
 * struct vdev_cp_stats - defines cp stats at vdev object
 * @vdev_obj: pointer to vdev
 * @vdev_stats: pointer to ic/mc specific stats
 * @vdev_comp_priv_obj[]: component's private object pointers
 * @vdev_cp_stats_lock:	lock to protect object
 */
struct vdev_cp_stats {
	struct wlan_objmgr_vdev *vdev_obj;
	vdev_ext_cp_stats_t *vdev_stats;
	void *vdev_comp_priv_obj[WLAN_CP_STATS_MAX_COMPONENTS];
	qdf_spinlock_t vdev_cp_stats_lock;
};

/**
 * struct peer_cp_stats - defines cp stats at peer object
 * @peer_obj: pointer to peer
 * @peer_stats: pointer to ic/mc specific stats
 * @peer_comp_priv_obj[]: component's private object pointers
 * @peer_cp_stats_lock:	lock to protect object
 * @twt_param: Pointer to peer twt session parameters
 */
struct peer_cp_stats {
	struct wlan_objmgr_peer *peer_obj;
	peer_ext_cp_stats_t *peer_stats;
	void *peer_comp_priv_obj[WLAN_CP_STATS_MAX_COMPONENTS];
	qdf_spinlock_t peer_cp_stats_lock;
#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
	struct twt_session_stats_info twt_param[TWT_PEER_MAX_SESSIONS];
#endif
};

/**
 * struct cp_stats_context - defines cp stats global context object
 * @csc_lock: lock to protect object
 * @psoc_obj: pointer to psoc
 * @psoc_cs: pointer to cp stats at psoc
 * @cp_stats_ctx_init: callback pointer to init cp stats global ctx
 * @cp_stats_ctx_deinit: callback pointer to deinit cp stats global ctx
 * @cp_stats_psoc_obj_init:callback pointer to init cp stats obj on psoc create
 * @cp_stats_psoc_obj_deinit:callback pointer to deinit cp stats obj on psoc
 * destroy
 * @cp_stats_pdev_obj_init:callback pointer to init cp stats obj on pdev create
 * @cp_stats_pdev_obj_deinit:callback pointer to deinit cp stats obj on pdev
 * destroy
 * @cp_stats_vdev_obj_init:callback pointer to init cp stats obj on vdev create
 * @cp_stats_vdev_obj_deinit:callback pointer to deinit cp stats obj on vdev
 * destroy
 * @cp_stats_peer_obj_init:callback pointer to init cp stats obj on peer create
 * @cp_stats_peer_obj_deinit:callback pointer to deinit cp stats obj on peer
 * destroy
 * @cp_stats_comp_obj_config:callback pointer to attach/detach other umac comp
 * @cp_stats_open: callback pointer for cp stats on psoc open
 * @cp_stats_close: callback pointer for cp stats on psoc close
 * @cp_stats_enable: callback pointer for cp stats on psoc enable
 * @cp_stats_disable: callback pointer for cp stats on psoc disable
 */
struct cp_stats_context {
	qdf_spinlock_t csc_lock;
	struct wlan_objmgr_psoc *psoc_obj;
	struct psoc_cp_stats    *psoc_cs;
	QDF_STATUS (*cp_stats_ctx_init)(struct cp_stats_context *ctx);
	QDF_STATUS (*cp_stats_ctx_deinit)(struct cp_stats_context *ctx);
	QDF_STATUS (*cp_stats_psoc_obj_init)(struct psoc_cp_stats *psoc_cs);
	QDF_STATUS (*cp_stats_psoc_obj_deinit)(struct psoc_cp_stats *psoc_cs);
	QDF_STATUS (*cp_stats_pdev_obj_init)(struct pdev_cp_stats *pdev_cs);
	QDF_STATUS (*cp_stats_pdev_obj_deinit)(struct pdev_cp_stats *pdev_cs);
	QDF_STATUS (*cp_stats_vdev_obj_init)(struct vdev_cp_stats *vdev_cs);
	QDF_STATUS (*cp_stats_vdev_obj_deinit)(struct vdev_cp_stats *vdev_cs);
	QDF_STATUS (*cp_stats_peer_obj_init)(struct peer_cp_stats *peer_cs);
	QDF_STATUS (*cp_stats_peer_obj_deinit)(struct peer_cp_stats *peer_cs);
	QDF_STATUS (*cp_stats_comp_obj_config)(
			enum wlan_objmgr_obj_type obj_type,
			enum wlan_cp_stats_cfg_state cfg_state,
			enum wlan_cp_stats_comp_id comp_id,
			void *cmn_obj,
			void *data);
	QDF_STATUS (*cp_stats_open)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*cp_stats_close)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*cp_stats_enable)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*cp_stats_disable)(struct wlan_objmgr_psoc *psoc);
};

/**
 * wlan_cp_stats_psoc_obj_lock() - private API to acquire spinlock at psoc
 * @psoc: pointer to psoc cp stats object
 *
 * Return: void
 */
static inline void wlan_cp_stats_psoc_obj_lock(struct psoc_cp_stats *psoc)
{
	qdf_spin_lock_bh(&psoc->psoc_cp_stats_lock);
}

/**
 * wlan_cp_stats_psoc_obj_unlock() - private API to release spinlock at psoc
 * @psoc: pointer to psoc cp stats object
 *
 * Return: void
 */
static inline void wlan_cp_stats_psoc_obj_unlock(struct psoc_cp_stats *psoc)
{
	qdf_spin_unlock_bh(&psoc->psoc_cp_stats_lock);
}

/**
 * wlan_cp_stats_pdev_obj_lock() - private API to acquire spinlock at pdev
 * @pdev: pointer to pdev cp stats object
 *
 * Return: void
 */
static inline void wlan_cp_stats_pdev_obj_lock(struct pdev_cp_stats *pdev)
{
	qdf_spin_lock_bh(&pdev->pdev_cp_stats_lock);
}

/**
 * wlan_cp_stats_pdev_obj_unlock() - private api to release spinlock at pdev
 * @pdev: pointer to pdev cp stats object
 *
 * Return: void
 */
static inline void wlan_cp_stats_pdev_obj_unlock(struct pdev_cp_stats *pdev)
{
	qdf_spin_unlock_bh(&pdev->pdev_cp_stats_lock);
}

/**
 * wlan_cp_stats_vdev_obj_lock() - private api to acquire spinlock at vdev
 * @vdev: pointer to vdev cp stats object
 *
 * Return: void
 */
static inline void wlan_cp_stats_vdev_obj_lock(struct vdev_cp_stats *vdev)
{
	qdf_spin_lock_bh(&vdev->vdev_cp_stats_lock);
}

/**
 * wlan_cp_stats_vdev_obj_unlock() - private api to release spinlock at vdev
 * @vdev: pointer to vdev cp stats object
 *
 * Return: void
 */
static inline void wlan_cp_stats_vdev_obj_unlock(struct vdev_cp_stats *vdev)
{
	qdf_spin_unlock_bh(&vdev->vdev_cp_stats_lock);
}

/**
 * wlan_cp_stats_peer_obj_lock() - private api to acquire spinlock at peer
 * @peer: pointer to peer cp stats object
 *
 * Return: void
 */
static inline void wlan_cp_stats_peer_obj_lock(struct peer_cp_stats *peer)
{
	qdf_spin_lock_bh(&peer->peer_cp_stats_lock);
}

/**
 * wlan_cp_stats_peer_obj_unlock() - private api to release spinlock at peer
 * @peer: pointer to peer cp stats object
 *
 * Return: void
 */
static inline void wlan_cp_stats_peer_obj_unlock(struct peer_cp_stats *peer)
{
	qdf_spin_unlock_bh(&peer->peer_cp_stats_lock);
}

/**
 * wlan_cp_stats_get_psoc_stats_obj() - API to get psoc_cp_stats from psoc
 * @psoc: Reference to psoc global object
 *
 * This API used to get psoc specific cp_stats object from global psoc
 * reference.
 *
 * Return : Reference to psoc_cp_stats object on success or NULL on failure
 */
static inline
struct psoc_cp_stats *wlan_cp_stats_get_psoc_stats_obj(struct wlan_objmgr_psoc
							*psoc)
{
	struct cp_stats_context *csc;

	if (!psoc)
		return NULL;

	csc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						    WLAN_UMAC_COMP_CP_STATS);

	if (!csc)
		return NULL;

	return csc->psoc_cs;
}

/**
 * wlan_cp_stats_get_pdev_stats_obj() - API to get pdev_cp_stats from pdev
 * @pdev: Reference to pdev global object
 *
 * This API used to get pdev specific cp_stats object from global pdev
 * reference.
 *
 * Return : Reference to pdev_cp_stats object on success or NULL on failure
 */
static inline
struct pdev_cp_stats *wlan_cp_stats_get_pdev_stats_obj(struct wlan_objmgr_pdev
							*pdev)
{
	struct pdev_cp_stats *pdev_cs = NULL;

	if (pdev) {
		pdev_cs = wlan_objmgr_pdev_get_comp_private_obj
				(pdev, WLAN_UMAC_COMP_CP_STATS);
	}

	return pdev_cs;
}

/**
 * wlan_cp_stats_get_vdev_stats_obj() - API to get vdev_cp_stats from vdev
 * @vdev : Reference to vdev global object
 *
 * This API used to get vdev specific cp_stats object from global vdev
 * reference.
 *
 * Return : Reference to vdev_cp_stats object on success or NULL on failure
 */
static inline
struct vdev_cp_stats *wlan_cp_stats_get_vdev_stats_obj(struct wlan_objmgr_vdev
							*vdev)
{
	struct vdev_cp_stats *vdev_cs = NULL;

	if (vdev) {
		vdev_cs = wlan_objmgr_vdev_get_comp_private_obj
				(vdev, WLAN_UMAC_COMP_CP_STATS);
	}

	return vdev_cs;
}

/**
 * wlan_cp_stats_get_peer_stats_obj() - API to get peer_cp_stats from peer
 * @peer: Reference to peer global object
 *
 * This API used to get peer specific cp_stats object from global peer
 * reference.
 *
 * Return : Reference to peer_cp_stats object on success or NULL on failure
 */
static inline
struct peer_cp_stats *wlan_cp_stats_get_peer_stats_obj(struct wlan_objmgr_peer
							*peer)
{
	struct peer_cp_stats *peer_cs = NULL;

	if (peer) {
		peer_cs = wlan_objmgr_peer_get_comp_private_obj
				(peer, WLAN_UMAC_COMP_CP_STATS);
	}

	return peer_cs;
}

/**
 * wlan_cp_stats_get_pdev_from_vdev() - API to get pdev_cp_stats obj from vdev
 * @vdev: Reference to vdev global object
 *
 * This API used to get pdev specific cp_stats object from global vdev
 * reference.
 *
 * Return: Reference to pdev_cp_stats object on success or NULL on failure
 */
static inline
struct pdev_cp_stats *wlan_cp_stats_get_pdev_from_vdev(struct wlan_objmgr_vdev
							*vdev)
{
	struct wlan_objmgr_pdev *pdev;
	struct pdev_cp_stats *pdev_cs = NULL;

	pdev = wlan_vdev_get_pdev(vdev);
	if (pdev) {
		pdev_cs = wlan_objmgr_pdev_get_comp_private_obj
				(pdev, WLAN_UMAC_COMP_CP_STATS);
	}

	return pdev_cs;
}

/**
 * wlan_cp_stats_ctx_get_from_pdev() - API to get cp_stats ctx obj from pdev
 * @pdev: Reference to pdev global object
 *
 * This API used to get cp_stats context object from global pdev reference.
 *
 * Return: Reference to cp_stats_context object on success or NULL on failure
 */
static inline
struct cp_stats_context *wlan_cp_stats_ctx_get_from_pdev(struct wlan_objmgr_pdev
							*pdev)
{
	struct wlan_objmgr_psoc *psoc;
	struct cp_stats_context *csc = NULL;

	if (!pdev)
		return NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	if (psoc) {
		csc = wlan_objmgr_psoc_get_comp_private_obj
				(psoc, WLAN_UMAC_COMP_CP_STATS);
	}
	return csc;
}

/**
 * wlan_cp_stats_ctx_get_from_vdev() - API to get cp_stats ctx obj from vdev
 * @vdev: Reference to vdev global object
 *
 * This API used to get cp_stats context object from global vdev reference.
 *
 * Return: Reference to cp_stats_context object on success or NULL on failure
 */
static inline
struct cp_stats_context *wlan_cp_stats_ctx_get_from_vdev(struct wlan_objmgr_vdev
							*vdev)
{
	struct wlan_objmgr_pdev *pdev;

	if (!vdev)
		return NULL;

	pdev = wlan_vdev_get_pdev(vdev);
	return wlan_cp_stats_ctx_get_from_pdev(pdev);
}

/**
 * wlan_cp_stats_ctx_get_from_peer() - API to get cp_stats ctx object from peer
 * @peer: Reference to peer object
 *
 * This API used to get cp_stats context object from global peer reference.
 *
 * Return: Reference to cp_stats_context object on success or NULL on failure
 */
static inline
struct cp_stats_context *wlan_cp_stats_ctx_get_from_peer(struct wlan_objmgr_peer
							*peer)
{
	struct wlan_objmgr_vdev *vdev;

	vdev = wlan_peer_get_vdev(peer);
	return wlan_cp_stats_ctx_get_from_vdev(vdev);
}

/**
 * wlan_cp_stats_get_comp_id() - API to get cp_stats component id from umac
 * component id
 * @comp_id: umac comp id
 *
 * Return: wlan_cp_stats_comp_id
 */
static inline enum wlan_cp_stats_comp_id
wlan_cp_stats_get_comp_id(enum wlan_umac_comp_id comp_id)
{
	enum wlan_cp_stats_comp_id cp_stats_comp_id =
		WLAN_CP_STATS_MAX_COMPONENTS;

	if (comp_id == WLAN_UMAC_COMP_ATF)
		cp_stats_comp_id = WLAN_CP_STATS_ATF;

	return cp_stats_comp_id;
}

#endif /* QCA_SUPPORT_CP_STATS */
#endif /* __WLAN_CP_STATS_DEFS_H__ */
