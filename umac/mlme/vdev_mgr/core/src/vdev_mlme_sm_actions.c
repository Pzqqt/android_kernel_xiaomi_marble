/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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
 * DOC: Implements VDEV MLME SM state actions
 */

#include <wlan_objmgr_cmn.h>
#include <wlan_objmgr_vdev_obj.h>
#include <include/wlan_mlme_cmn.h>
#include <include/wlan_pdev_mlme.h>
#include <include/wlan_vdev_mlme.h>
#include <wlan_mlme_dbg.h>
#include <wlan_pdev_mlme_api.h>
#include <wlan_vdev_mlme_api.h>
#include <wlan_utility.h>
#include <ieee80211_mlme_priv.h>
#include <ieee80211_ucfg.h>

static QDF_STATUS
mlme_vdev_validate_basic_params_cb(struct vdev_mlme_obj *vdev_mlme,
				   uint16_t event_data_len, void *event_data)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_channel *des_chan;
	struct wlan_objmgr_pdev *pdev;
	uint8_t ssid[WLAN_SSID_MAX_LEN + 1] = {0};
	uint8_t ssid_len = 0;

	if (!vdev_mlme) {
		mlme_err("VDEV MLME obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = vdev_mlme->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("PDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	des_chan = wlan_vdev_mlme_get_des_chan(vdev);
	if ((des_chan) && (des_chan->ch_freq == 0)) {
		mlme_err("(vdev-id:%d)channel is not configured",
			 wlan_vdev_get_id(vdev));
		return QDF_STATUS_E_FAILURE;
	}

	wlan_vdev_mlme_get_ssid(vdev, ssid, &ssid_len);
	if ((wlan_vdev_mlme_get_opmode(vdev) != QDF_MONITOR_MODE) &&
	    (!ssid_len)) {
		mlme_err("(vdev-id:%d)SSID is not configured",
			 wlan_vdev_get_id(vdev));
		return QDF_STATUS_E_FAILURE;
	}

	/* it checks the channels across pdev, notifies with prints */
	wlan_util_pdev_vdevs_deschan_match(pdev, vdev, WLAN_MLME_SB_ID);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS mlme_vdev_start_is_allowed(struct wlan_objmgr_pdev *pdev,
					     struct wlan_objmgr_vdev *vdev)
{
	struct pdev_mlme_obj *pdev_mlme;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	pdev_mlme = wlan_pdev_mlme_get_cmpt_obj(pdev);
	if (!pdev_mlme) {
		mlme_err(" PDEV MLME is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&pdev_mlme->vdev_restart_lock);
	if (wlan_pdev_mlme_op_get(pdev, WLAN_PDEV_OP_RESTART_INPROGRESS)) {
		wlan_util_change_map_index(pdev_mlme->start_send_vdev_arr,
					   wlan_vdev_get_id(vdev), 1);
		status = QDF_STATUS_E_FAILURE;
	}
	qdf_spin_unlock_bh(&pdev_mlme->vdev_restart_lock);

	return status;
}

static QDF_STATUS mlme_vdev_start_fw_cmd_send(struct wlan_objmgr_pdev *pdev,
					      struct wlan_objmgr_vdev *vdev,
					      uint8_t restart)
{
	QDF_STATUS error = QDF_STATUS_E_FAILURE;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct vdev_mlme_obj *vdev_mlme;

	vdev_mlme = wlan_vdev_mlme_get_cmpt_obj(vdev);
	if (!vdev_mlme)
		return status;

	error = mlme_vdev_ops_start_fw_send(vdev, restart);

	if (error == QDF_STATUS_E_FAILURE) {
		if (!restart)
			status = wlan_vdev_mlme_sm_deliver_evt_sync
					(vdev,
					 WLAN_VDEV_SM_EV_START_REQ_FAIL, 0,
					 NULL);
		else
			status = wlan_vdev_mlme_sm_deliver_evt_sync
					(vdev,
					 WLAN_VDEV_SM_EV_RESTART_REQ_FAIL, 0,
					 NULL);
	}

	return status;
}

QDF_STATUS mlme_vdev_start_send_cb(struct vdev_mlme_obj *vdev_mlme,
				   uint16_t event_data_len, void *event_data)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;

	if (!vdev_mlme) {
		mlme_err("VDEV MLME obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = vdev_mlme->vdev;

	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("(vdev-id:%d) PDEV is NULL", wlan_vdev_get_id(vdev));
		return QDF_STATUS_E_FAILURE;
	}

	if (mlme_vdev_start_is_allowed(pdev, vdev) == QDF_STATUS_SUCCESS)
		mlme_vdev_start_fw_cmd_send(pdev, vdev, 0);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS mlme_stop_pending_start(struct wlan_objmgr_pdev *pdev,
					  struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct pdev_mlme_obj *pdev_mlme;

	pdev_mlme = wlan_pdev_mlme_get_cmpt_obj(pdev);
	if (!pdev_mlme) {
		mlme_err(" PDEV MLME is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&pdev_mlme->vdev_restart_lock);
	if (wlan_pdev_mlme_op_get(pdev, WLAN_PDEV_OP_RESTART_INPROGRESS)) {
		if (wlan_util_map_index_is_set(
				pdev_mlme->start_send_vdev_arr,
				wlan_vdev_get_id(vdev)) == true) {
			wlan_util_change_map_index(
					pdev_mlme->start_send_vdev_arr,
					wlan_vdev_get_id(vdev), 0);
			status = QDF_STATUS_SUCCESS;
		}
	}
	qdf_spin_unlock_bh(&pdev_mlme->vdev_restart_lock);

	return status;
}

static void wlan_vdev_start_fw_send(struct wlan_objmgr_pdev *pdev,
				    void *object,
				    void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	unsigned long *send_array = (unsigned long *)arg;

	if (wlan_util_map_index_is_set(send_array, wlan_vdev_get_id(vdev)) ==
					false)
		return;

	mlme_vdev_start_fw_cmd_send(pdev, vdev, 0);

	 /* Reset the flag */
	wlan_util_change_map_index(send_array, wlan_vdev_get_id(vdev), 0);
}

static QDF_STATUS mlme_stop_pending_restart(struct wlan_objmgr_pdev *pdev,
					    struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	struct pdev_mlme_obj *pdev_mlme;

	pdev_mlme = wlan_pdev_mlme_get_cmpt_obj(pdev);
	if (!pdev_mlme) {
		mlme_err(" PDEV MLME is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&pdev_mlme->vdev_restart_lock);
	if (wlan_pdev_mlme_op_get(pdev, WLAN_PDEV_OP_RESTART_INPROGRESS)) {
		wlan_util_change_map_index(
				pdev_mlme->restart_pend_vdev_bmap,
				wlan_vdev_get_id(vdev), 0);
		if (wlan_util_map_index_is_set(
				pdev_mlme->restart_send_vdev_bmap,
				wlan_vdev_get_id(vdev)) == true) {
			wlan_util_change_map_index(
					pdev_mlme->restart_send_vdev_bmap,
					wlan_vdev_get_id(vdev), 0);
			status = QDF_STATUS_SUCCESS;
		}
		if (!wlan_util_map_is_any_index_set(
				pdev_mlme->restart_pend_vdev_bmap,
				sizeof(pdev_mlme->restart_pend_vdev_bmap)) &&
		    !wlan_util_map_is_any_index_set(
				pdev_mlme->restart_send_vdev_bmap,
				sizeof(pdev_mlme->restart_send_vdev_bmap))) {
			mlme_err("Clear MVR bit for Pdev %d",
				 wlan_objmgr_pdev_get_pdev_id(pdev));
			wlan_pdev_mlme_op_clear
					(pdev,
					 WLAN_PDEV_OP_RESTART_INPROGRESS);
			wlan_pdev_mlme_op_clear(pdev,
						WLAN_PDEV_OP_MBSSID_RESTART);

			if (wlan_util_map_is_any_index_set(
			    pdev_mlme->start_send_vdev_arr,
			    sizeof(pdev_mlme->start_send_vdev_arr))) {
				wlan_objmgr_pdev_iterate_obj_list(
					 pdev, WLAN_VDEV_OP,
					 wlan_vdev_start_fw_send,
					 pdev_mlme->start_send_vdev_arr, 0,
					 WLAN_MLME_NB_ID);
			}
		}

	}
	qdf_spin_unlock_bh(&pdev_mlme->vdev_restart_lock);

	return status;
}

static void wlan_vdev_restart_fw_send(struct wlan_objmgr_pdev *pdev,
				      void *object,
				      void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	unsigned long *send_array = (unsigned long *)arg;

	if (wlan_util_map_index_is_set(send_array, wlan_vdev_get_id(vdev)) ==
				       false)
		return;

	mlme_vdev_start_fw_cmd_send(pdev, vdev, 1);
	/* Reset the flag */
	wlan_util_change_map_index(send_array, wlan_vdev_get_id(vdev), 0);
}

static void wlan_vdev_update_des_chan(struct wlan_objmgr_pdev *pdev,
				      void *object,
				      void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	struct wlan_channel *des_chan = (struct wlan_channel *)arg;
	struct wlan_channel *iter_chan = wlan_vdev_mlme_get_des_chan(vdev);
	struct pdev_mlme_obj *pdev_mlme;

	pdev_mlme = wlan_pdev_mlme_get_cmpt_obj(pdev);
	if (!pdev_mlme) {
		mlme_err(" PDEV MLME is NULL");
		return;
	}

	if (wlan_util_map_index_is_set(pdev_mlme->start_send_vdev_arr,
				       wlan_vdev_get_id(vdev)) == false)
		return;

	if (wlan_chan_eq(iter_chan, des_chan) != QDF_STATUS_SUCCESS) {
		mlme_err("==> vdev id: %d: ieee %d, restart vdev ieee %d, updating",
			 wlan_vdev_get_id(vdev), iter_chan->ch_ieee,
			 des_chan->ch_ieee);
		wlan_chan_copy(iter_chan, des_chan);
	} else {
		mlme_err("is invoked");
	}
}

static void mlme_restart_req_timer_start(struct pdev_mlme_obj *pdev_mlme)
{
	qdf_timer_mod(&pdev_mlme->restart_req_timer, 100);
}

static void mlme_restart_req_timer_stop(struct pdev_mlme_obj *pdev_mlme)
{
	qdf_timer_stop(&pdev_mlme->restart_req_timer);
}

static struct wlan_channel *wlan_pdev_mlme_get_restart_des_chan(
					struct wlan_objmgr_pdev *pdev)
{
	struct wlan_channel *des_chan;
	uint32_t max_vdevs = 0, i;
	uint8_t vdev_id = 0xFF;
	struct wlan_objmgr_vdev *vdev;
	struct pdev_mlme_obj *pdev_mlme;

	pdev_mlme = wlan_pdev_mlme_get_cmpt_obj(pdev);
	if (!pdev_mlme) {
		mlme_err(" PDEV MLME is NULL");
		return NULL;
	}

	max_vdevs = wlan_psoc_get_max_vdev_count(wlan_pdev_get_psoc(pdev));
	for (i = 0; i < max_vdevs; i++) {
		if (wlan_util_map_index_is_set(
			pdev_mlme->restart_send_vdev_bmap, i)
					== false)
			continue;

		vdev_id = i;
		break;
	}

	if (vdev_id == 0xFF)
		return NULL;

	vdev = wlan_objmgr_get_vdev_by_id_from_pdev(pdev, vdev_id,
						    WLAN_MLME_NB_ID);
	if (!vdev)
		return NULL;

	des_chan = wlan_vdev_mlme_get_des_chan(vdev);

	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_NB_ID);

	return des_chan;
}

static void mlme_multivdev_restart(struct pdev_mlme_obj *pdev_mlme)
{
	struct wlan_objmgr_pdev *pdev;
	struct wlan_channel *des_chan = NULL;

	pdev = pdev_mlme->pdev;

	/* On timer expiry, check any pending vdev has gone down, then enable
	 * that vdev bit, if pending vdev is still in SUSPEND (CSA_RESTART or
	 * SUSPEND_RESTART), then restart the timer
	 */
	if (!wlan_util_map_is_any_index_set(
			pdev_mlme->restart_pend_vdev_bmap,
			sizeof(pdev_mlme->restart_pend_vdev_bmap))) {
		mlme_err("Sending MVR for Pdev %d",
			 wlan_objmgr_pdev_get_pdev_id(pdev));
		wlan_pdev_mlme_op_clear(pdev, WLAN_PDEV_OP_MBSSID_RESTART);
		wlan_pdev_mlme_op_clear(pdev, WLAN_PDEV_OP_RESTART_INPROGRESS);

		qdf_mem_copy(&pdev_mlme->pdev_restart.restart_bmap,
			     &pdev_mlme->restart_send_vdev_bmap,
			     sizeof(pdev_mlme->pdev_restart.restart_bmap));

		qdf_atomic_init(&pdev_mlme->multivdev_restart_wait_cnt);
		des_chan = wlan_pdev_mlme_get_restart_des_chan(pdev);
		if (!wlan_pdev_nif_feat_cap_get(pdev,
						WLAN_PDEV_F_MULTIVDEV_RESTART))
			wlan_objmgr_pdev_iterate_obj_list
				(pdev,  WLAN_VDEV_OP,
				 wlan_vdev_restart_fw_send,
				 pdev_mlme->restart_send_vdev_bmap, 0,
				 WLAN_MLME_NB_ID);
		else
			mlme_vdev_ops_multivdev_restart_fw_cmd_send(pdev);

		if (wlan_util_map_is_any_index_set(
				pdev_mlme->start_send_vdev_arr,
				sizeof(pdev_mlme->start_send_vdev_arr))) {
			if (des_chan) {
				wlan_objmgr_pdev_iterate_obj_list(pdev,
						WLAN_VDEV_OP,
						wlan_vdev_update_des_chan,
						des_chan, 0,
						WLAN_MLME_NB_ID);
			}
			wlan_objmgr_pdev_iterate_obj_list
				(pdev, WLAN_VDEV_OP,
				 wlan_vdev_start_fw_send,
				 pdev_mlme->start_send_vdev_arr, 0,
				 WLAN_MLME_NB_ID);
		}
	} else {
		mlme_restart_req_timer_start(pdev_mlme);
	}
}

#define MULTIVDEV_RESTART_MAX_RETRY_CNT 200
static os_timer_func(mlme_restart_req_timeout)
{
	qdf_bitmap(tmp_restart_pend_vdev_bmap, WLAN_UMAC_PSOC_MAX_VDEVS);
	qdf_bitmap(tmp_dest_bmap, WLAN_UMAC_PSOC_MAX_VDEVS);
	struct wlan_objmgr_pdev *pdev;
	struct pdev_mlme_obj *pdev_mlme;

	OS_GET_TIMER_ARG(pdev_mlme, struct pdev_mlme_obj *);

	pdev = pdev_mlme->pdev;

	qdf_mem_zero(tmp_restart_pend_vdev_bmap,
		     sizeof(tmp_restart_pend_vdev_bmap));
	qdf_mem_zero(tmp_dest_bmap, sizeof(tmp_dest_bmap));

	qdf_spin_lock_bh(&pdev_mlme->vdev_restart_lock);
	if (wlan_pdev_mlme_op_get(pdev, WLAN_PDEV_OP_RESTART_INPROGRESS)) {
		wlan_pdev_chan_change_pending_vdevs(pdev,
						    tmp_restart_pend_vdev_bmap,
						    WLAN_MLME_SB_ID);
		qdf_atomic_inc(&pdev_mlme->multivdev_restart_wait_cnt);
		if (qdf_atomic_read(&pdev_mlme->multivdev_restart_wait_cnt) >
				    MULTIVDEV_RESTART_MAX_RETRY_CNT) {
			mlme_err("Multivdev restart_pend_vdev_bmap is");
			qdf_trace_hex_dump(
				QDF_MODULE_ID_CMN_MLME, QDF_TRACE_LEVEL_ERROR,
				pdev_mlme->restart_pend_vdev_bmap,
				sizeof(pdev_mlme->restart_pend_vdev_bmap));
			QDF_BUG(0);
		}

		/* If all the pending vdevs goes down, this would fail,
		 * otherwise start timer
		 */
		qdf_bitmap_and(tmp_dest_bmap, tmp_restart_pend_vdev_bmap,
			       pdev_mlme->restart_pend_vdev_bmap,
			       QDF_CHAR_BIT * sizeof(tmp_dest_bmap));
		if (wlan_util_map_is_any_index_set(tmp_dest_bmap,
						   sizeof(tmp_dest_bmap)))
			mlme_restart_req_timer_start(pdev_mlme);
		else
			mlme_multivdev_restart(pdev_mlme);
	}
	qdf_spin_unlock_bh(&pdev_mlme->vdev_restart_lock);
}

static QDF_STATUS mlme_vdev_restart_is_allowed(struct wlan_objmgr_pdev *pdev,
					       struct wlan_objmgr_vdev *vdev)
{
	uint8_t id;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct pdev_mlme_obj *pdev_mlme;

	pdev_mlme = wlan_pdev_mlme_get_cmpt_obj(pdev);
	if (!pdev_mlme) {
		mlme_err(" PDEV MLME is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!wlan_pdev_mlme_op_get(pdev, WLAN_PDEV_OP_MBSSID_RESTART)) {
		mlme_err(" No multivdev restart");
		return status;
	}

	qdf_spin_lock_bh(&pdev_mlme->vdev_restart_lock);
	if (!wlan_pdev_mlme_op_get(pdev, WLAN_PDEV_OP_RESTART_INPROGRESS)) {
		/* If channel change is not started, Initialize bit map with
		 * current active vdevs	compare channels (if desired channel of
		 * this AP matches with all other vdevs return SUCCESS
		 */
		wlan_pdev_chan_change_pending_vdevs
				(pdev,
				 pdev_mlme->restart_pend_vdev_bmap,
				 WLAN_MLME_SB_ID);
		qdf_mem_zero(pdev_mlme->restart_send_vdev_bmap,
			     sizeof(pdev_mlme->restart_send_vdev_bmap));
		if (!wlan_util_map_is_any_index_set(
				pdev_mlme->restart_pend_vdev_bmap,
				sizeof(pdev_mlme->restart_pend_vdev_bmap)))
			QDF_BUG(0);

		wlan_pdev_mlme_op_set(pdev, WLAN_PDEV_OP_RESTART_INPROGRESS);
	}
	/* if channel change is in progress */
	if (wlan_pdev_mlme_op_get(pdev, WLAN_PDEV_OP_RESTART_INPROGRESS)) {
		/* enable vdev id bit */
		id = wlan_vdev_get_id(vdev);
		/* Invalid vdev id */
		if (id >=
		    wlan_psoc_get_max_vdev_count(wlan_pdev_get_psoc(pdev))) {
			qdf_spin_unlock_bh(&pdev_mlme->vdev_restart_lock);
			return QDF_STATUS_E_FAILURE;
		}
		wlan_util_change_map_index(pdev_mlme->restart_pend_vdev_bmap,
					   wlan_vdev_get_id(vdev), 0);
		wlan_util_change_map_index(pdev_mlme->restart_send_vdev_bmap,
					   wlan_vdev_get_id(vdev), 1);

		/* If all vdev id bits are enabled, start vdev restart for all
		 * vdevs, otherwise, start timer and return
		 */
		if (!wlan_util_map_is_any_index_set(
				pdev_mlme->restart_pend_vdev_bmap,
				sizeof(pdev_mlme->restart_pend_vdev_bmap))) {
			mlme_restart_req_timer_stop(pdev_mlme);
			mlme_multivdev_restart(pdev_mlme);
			status = QDF_STATUS_E_FAILURE;
		} else {
			/* On timer expiry, check any pending vdev has gone
			 * down, then enable thats vdev bit, if pending vdev
			 * is still in valid, then restart the timer
			 */
			mlme_restart_req_timer_start(pdev_mlme);
			status = QDF_STATUS_E_FAILURE;
		}
	}
	qdf_spin_unlock_bh(&pdev_mlme->vdev_restart_lock);

	return status;
}

static QDF_STATUS mlme_vdev_restart_send_cb(struct vdev_mlme_obj *vdev_mlme,
					    uint16_t event_data_len,
					    void *event_data)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;

	if (!vdev_mlme) {
		mlme_err("VDEV MLME obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = vdev_mlme->vdev;

	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("(vdev-id:%d) PDEV is NULL", wlan_vdev_get_id(vdev));
		return QDF_STATUS_E_FAILURE;
	}

	if (mlme_vdev_restart_is_allowed(pdev, vdev) != QDF_STATUS_SUCCESS) {
		mlme_debug
			("(vdev-id:%d) Restart send is deferred for other vaps",
			 wlan_vdev_get_id(vdev));
		return QDF_STATUS_SUCCESS;
	}

	mlme_vdev_start_fw_cmd_send(pdev, vdev, 1);

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS mlme_vdev_stop_start_send_cb(struct vdev_mlme_obj *vdev_mlme,
					       enum vdev_cmd_type restart,
					       uint16_t event_data_len,
					       void *event_data)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;
	QDF_STATUS error;

	if (!vdev_mlme) {
		mlme_err("VDEV MLME obj is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	vdev = vdev_mlme->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("(vdev-id:%d) PDEV is NULL", wlan_vdev_get_id(vdev));
		return QDF_STATUS_E_FAILURE;
	}

	if (restart == RESTART_REQ) {
		error = mlme_stop_pending_restart(pdev, vdev);
		if (error == QDF_STATUS_SUCCESS) {
			error = wlan_vdev_mlme_sm_deliver_evt_sync
					(vdev,
					 WLAN_VDEV_SM_EV_RESTART_REQ_FAIL,
					 0, NULL);
		}
	} else {
		error = mlme_stop_pending_start(pdev, vdev);
		/* no resource manager in place */
		if (error == QDF_STATUS_SUCCESS) {
			error = wlan_vdev_mlme_sm_deliver_evt_sync
					(vdev,
					 WLAN_VDEV_SM_EV_START_REQ_FAIL,
					 0, NULL);
		}
	}

	return QDF_STATUS_SUCCESS;
}

static QDF_STATUS mlme_vdev_stop_send_cb(struct vdev_mlme_obj *vdev_mlme,
					 uint16_t event_data_len,
					 void *event_data)
{
	QDF_STATUS error = QDF_STATUS_E_FAILURE;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_pdev *pdev;

	vdev = vdev_mlme->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	pdev = wlan_vdev_get_pdev(vdev);
	if (!pdev) {
		mlme_err("(vdev-id:%d) PDEV is NULL", wlan_vdev_get_id(vdev));
		return QDF_STATUS_E_FAILURE;
	}

	/* if restart is pending on this VDEV, clear the bitmap */
	mlme_stop_pending_restart(pdev, vdev);

	error = mlme_vdev_ops_stop_fw_send(vdev);

	return error;
}

static QDF_STATUS mlme_vdev_down_send_cb(struct vdev_mlme_obj *vdev_mlme,
					 uint16_t event_data_len,
					 void *event_data)
{
	QDF_STATUS error = QDF_STATUS_E_FAILURE;
	struct wlan_objmgr_vdev *vdev;

	vdev = vdev_mlme->vdev;
	if (!vdev) {
		mlme_err("VDEV is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	error = mlme_vdev_ops_down_fw_send(vdev);

	return error;
}

QDF_STATUS mlme_register_cmn_ops(struct vdev_mlme_obj *vdev_mlme)
{
	vdev_mlme->ops->mlme_vdev_validate_basic_params =
				mlme_vdev_validate_basic_params_cb;
	vdev_mlme->ops->mlme_vdev_start_send = mlme_vdev_start_send_cb;
	vdev_mlme->ops->mlme_vdev_restart_send = mlme_vdev_restart_send_cb;
	vdev_mlme->ops->mlme_vdev_stop_start_send =
						mlme_vdev_stop_start_send_cb;
	vdev_mlme->ops->mlme_vdev_stop_send = mlme_vdev_stop_send_cb;
	vdev_mlme->ops->mlme_vdev_down_send = mlme_vdev_down_send_cb;

	return QDF_STATUS_SUCCESS;
}

void mlme_restart_timer_init(struct pdev_mlme_obj *pdev_mlme)
{
	qdf_atomic_init(&pdev_mlme->multivdev_restart_wait_cnt);
	qdf_timer_init(NULL, &pdev_mlme->restart_req_timer,
		       mlme_restart_req_timeout, (void *)(pdev_mlme),
		       QDF_TIMER_TYPE_WAKE_APPS);

	qdf_spinlock_create(&pdev_mlme->vdev_restart_lock);
}

void mlme_restart_timer_delete(struct pdev_mlme_obj *pdev_mlme)
{
	qdf_timer_stop(&pdev_mlme->restart_req_timer);

	qdf_spinlock_destroy(&pdev_mlme->vdev_restart_lock);
}

