/*
 * Copyright (c) 2018-2019, 2021 The Linux Foundation. All rights reserved.
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

/**
 * DOC: Implements VDEV MLME public APIs
 */

#include <wlan_objmgr_vdev_obj.h>
#include <wlan_mlme_dbg.h>
#include "include/wlan_vdev_mlme.h"
#include "../../core/src/vdev_mlme_sm.h"
#include <wlan_vdev_mlme_api.h>
#include <qdf_module.h>

struct vdev_mlme_obj *wlan_vdev_mlme_get_cmpt_obj(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	if (!vdev) {
		mlme_err("vdev is NULL");
		return NULL;
	}

	vdev_mlme = wlan_objmgr_vdev_get_comp_private_obj(vdev,
							  WLAN_UMAC_COMP_MLME);
	if (!vdev_mlme) {
		mlme_err(" MLME component object is NULL");
		return NULL;
	}

	return vdev_mlme;
}

qdf_export_symbol(wlan_vdev_mlme_get_cmpt_obj);

void wlan_vdev_mlme_set_ext_hdl(struct wlan_objmgr_vdev *vdev,
				mlme_vdev_ext_t *ext_hdl)
{
	struct vdev_mlme_obj *vdev_mlme;

	if (!ext_hdl) {
		mlme_err("Invalid input");
		return;
	}

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (vdev_mlme)
		vdev_mlme->ext_vdev_ptr = ext_hdl;
}

qdf_export_symbol(wlan_vdev_mlme_set_ext_hdl);

mlme_vdev_ext_t *wlan_vdev_mlme_get_ext_hdl(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (vdev_mlme)
		return vdev_mlme->ext_vdev_ptr;

	return NULL;
}

qdf_export_symbol(wlan_vdev_mlme_get_ext_hdl);

QDF_STATUS wlan_vdev_mlme_sm_deliver_evt(struct wlan_objmgr_vdev *vdev,
					 enum wlan_vdev_sm_evt event,
					 uint16_t event_data_len,
					 void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme;
	QDF_STATUS status;
	enum wlan_vdev_state state_entry, state_exit;
	enum wlan_vdev_state substate_entry, substate_exit;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	mlme_vdev_sm_spin_lock(vdev_mlme);

	/* store entry state and sub state for prints */
	state_entry = wlan_vdev_mlme_get_state(vdev);
	substate_entry = wlan_vdev_mlme_get_substate(vdev);
	mlme_vdev_sm_print_state_event(vdev_mlme, event);

	status = mlme_vdev_sm_deliver_event(vdev_mlme, event, event_data_len,
					    event_data);
	/* Take exit state, exit substate for prints */
	state_exit = wlan_vdev_mlme_get_state(vdev);
	substate_exit = wlan_vdev_mlme_get_substate(vdev);
	/* If no state and substate change, don't print */
	if (!((state_entry == state_exit) && (substate_entry == substate_exit)))
		mlme_vdev_sm_print_state(vdev_mlme);
	mlme_vdev_sm_spin_unlock(vdev_mlme);

	return status;
}

qdf_export_symbol(wlan_vdev_mlme_sm_deliver_evt);

QDF_STATUS wlan_vdev_mlme_sm_deliver_evt_sync(struct wlan_objmgr_vdev *vdev,
					      enum wlan_vdev_sm_evt event,
					      uint16_t event_data_len,
					      void *event_data)
{
	struct vdev_mlme_obj *vdev_mlme;
	QDF_STATUS status;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev component object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	status = mlme_vdev_sm_deliver_event(vdev_mlme, event, event_data_len,
					    event_data);

	return status;
}

qdf_export_symbol(wlan_vdev_mlme_sm_deliver_evt_sync);

#ifdef SM_ENG_HIST_ENABLE
void wlan_vdev_mlme_sm_history_print(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev component object is NULL");
		return;
	}

	mlme_vdev_sm_history_print(vdev_mlme);
}
#endif

QDF_STATUS wlan_vdev_allow_connect_n_tx(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);
	if ((state == WLAN_VDEV_S_UP && substate == WLAN_VDEV_SS_UP_ACTIVE) ||
	    ((state == WLAN_VDEV_S_SUSPEND) &&
	     (substate == WLAN_VDEV_SS_SUSPEND_CSA_RESTART)))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_vdev_mlme_is_active(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;

	state = wlan_vdev_mlme_get_state(vdev);
	if ((state == WLAN_VDEV_S_UP) ||  (state == WLAN_VDEV_S_DFS_CAC_WAIT) ||
	    (state == WLAN_VDEV_S_SUSPEND))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(wlan_vdev_mlme_is_active);

QDF_STATUS wlan_vdev_chan_config_valid(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);
	if (!((state == WLAN_VDEV_S_INIT) || (state == WLAN_VDEV_S_STOP)))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

qdf_export_symbol(wlan_vdev_chan_config_valid);

QDF_STATUS wlan_vdev_mlme_is_csa_restart(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);
	if ((state == WLAN_VDEV_S_SUSPEND) &&
	    (substate == WLAN_VDEV_SS_SUSPEND_CSA_RESTART))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_vdev_is_going_down(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);
	if ((state == WLAN_VDEV_S_STOP) ||
	    ((state == WLAN_VDEV_S_SUSPEND) &&
	     (substate == WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN)))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_vdev_is_peer_create_allowed(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;

	if (!vdev) {
		mlme_err("vdev is null");
		return QDF_STATUS_E_FAILURE;
	}

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);
	if (!((state == WLAN_VDEV_S_INIT) ||
	     (state == WLAN_VDEV_S_STOP) ||
	     ((state == WLAN_VDEV_S_SUSPEND) &&
	      (substate == WLAN_VDEV_SS_SUSPEND_SUSPEND_DOWN))))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_vdev_is_restart_progress(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);
	if ((state == WLAN_VDEV_S_START) &&
	    (substate == WLAN_VDEV_SS_START_RESTART_PROGRESS))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_vdev_is_dfs_cac_wait(struct wlan_objmgr_vdev *vdev)
{
	if (wlan_vdev_mlme_get_state(vdev) == WLAN_VDEV_S_DFS_CAC_WAIT)
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

void wlan_vdev_mlme_cmd_lock(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev component object is NULL");
		return;
	}

	mlme_vdev_cmd_mutex_acquire(vdev_mlme);
}

void wlan_vdev_mlme_cmd_unlock(struct wlan_objmgr_vdev *vdev)
{
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme) {
		mlme_err("vdev component object is NULL");
		return;
	}

	mlme_vdev_cmd_mutex_release(vdev_mlme);
}

QDF_STATUS wlan_vdev_mlme_is_scan_allowed(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);
	if ((state == WLAN_VDEV_S_INIT) ||
	    (state == WLAN_VDEV_S_UP && substate == WLAN_VDEV_SS_UP_ACTIVE) ||
	    (state == WLAN_VDEV_S_STOP))
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_vdev_mlme_is_init_state(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;

	state = wlan_vdev_mlme_get_state(vdev);
	if (state == WLAN_VDEV_S_INIT)
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}

QDF_STATUS wlan_vdev_is_up_active_state(struct wlan_objmgr_vdev *vdev)
{
	enum wlan_vdev_state state;
	enum wlan_vdev_state substate;

	state = wlan_vdev_mlme_get_state(vdev);
	substate = wlan_vdev_mlme_get_substate(vdev);
	if (state == WLAN_VDEV_S_UP && substate == WLAN_VDEV_SS_UP_ACTIVE)
		return QDF_STATUS_SUCCESS;

	return QDF_STATUS_E_FAILURE;
}
