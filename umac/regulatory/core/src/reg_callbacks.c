/*
 * Copyright (c) 2014-2020 The Linux Foundation. All rights reserved.
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
 * DOC: reg_callbacks.c
 * This file defines regulatory callback functions
 */

#include <wlan_cmn.h>
#include <reg_services_public_struct.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include "reg_priv_objs.h"
#include "reg_utils.h"
#include <scheduler_api.h>
#include "reg_callbacks.h"
#include "reg_services_common.h"
#include "reg_build_chan_list.h"

/**
 * reg_call_chan_change_cbks() - Call registered callback functions on channel
 * change.
 * @psoc: Pointer to global psoc structure.
 * @pdev: Pointer to global pdev structure.
 */
static void reg_call_chan_change_cbks(struct wlan_objmgr_psoc *psoc,
				      struct wlan_objmgr_pdev *pdev)
{
	struct chan_change_cbk_entry *cbk_list;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct regulatory_channel *cur_chan_list;
	uint32_t ctr;
	struct avoid_freq_ind_data *avoid_freq_ind = NULL;
	reg_chan_change_callback callback;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_alert("psoc reg component is NULL");
		return;
	}

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_alert("pdev reg component is NULL");
		return;
	}

	cur_chan_list = qdf_mem_malloc(NUM_CHANNELS * sizeof(*cur_chan_list));
	if (!cur_chan_list)
		return;

	qdf_mem_copy(cur_chan_list,
		     pdev_priv_obj->cur_chan_list,
		     NUM_CHANNELS *
		     sizeof(struct regulatory_channel));

	if (psoc_priv_obj->ch_avoid_ind) {
		avoid_freq_ind = qdf_mem_malloc(sizeof(*avoid_freq_ind));
		if (!avoid_freq_ind)
			goto skip_ch_avoid_ind;

		qdf_mem_copy(&avoid_freq_ind->freq_list,
			     &psoc_priv_obj->avoid_freq_list,
				sizeof(struct ch_avoid_ind_type));
		qdf_mem_copy(&avoid_freq_ind->chan_list,
			     &psoc_priv_obj->unsafe_chan_list,
				sizeof(struct unsafe_ch_list));
		psoc_priv_obj->ch_avoid_ind = false;
	}

skip_ch_avoid_ind:
	cbk_list = psoc_priv_obj->cbk_list;

	for (ctr = 0; ctr < REG_MAX_CHAN_CHANGE_CBKS; ctr++) {
		callback  = NULL;
		qdf_spin_lock_bh(&psoc_priv_obj->cbk_list_lock);
		if (cbk_list[ctr].cbk)
			callback = cbk_list[ctr].cbk;
		qdf_spin_unlock_bh(&psoc_priv_obj->cbk_list_lock);
		if (callback)
			callback(psoc, pdev, cur_chan_list, avoid_freq_ind,
				 cbk_list[ctr].arg);
	}
	qdf_mem_free(cur_chan_list);
	if (avoid_freq_ind)
		qdf_mem_free(avoid_freq_ind);
}

/**
 * reg_alloc_and_fill_payload() - Alloc and fill payload structure.
 * @psoc: Pointer to global psoc structure.
 * @pdev: Pointer to global pdev structure.
 */
static void reg_alloc_and_fill_payload(struct wlan_objmgr_psoc *psoc,
				       struct wlan_objmgr_pdev *pdev,
				       struct reg_sched_payload **payload)
{
	*payload = qdf_mem_malloc(sizeof(**payload));
	if (*payload) {
		(*payload)->psoc = psoc;
		(*payload)->pdev = pdev;
	}
}

/**
 * reg_chan_change_flush_cbk_sb() - Flush south bound channel change callbacks.
 * @msg: Pointer to scheduler msg structure.
 */
static QDF_STATUS reg_chan_change_flush_cbk_sb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_sched_chan_change_cbks_sb() - Schedule south bound channel change
 * callbacks.
 * @msg: Pointer to scheduler msg structure.
 */
static QDF_STATUS reg_sched_chan_change_cbks_sb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	reg_call_chan_change_cbks(psoc, pdev);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_chan_change_flush_cbk_nb() - Flush north bound channel change callbacks.
 * @msg: Pointer to scheduler msg structure.
 */
static QDF_STATUS reg_chan_change_flush_cbk_nb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_sched_chan_change_cbks_nb() - Schedule north bound channel change
 * callbacks.
 * @msg: Pointer to scheduler msg structure.
 */
static QDF_STATUS reg_sched_chan_change_cbks_nb(struct scheduler_msg *msg)
{
	struct reg_sched_payload *load = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = load->psoc;
	struct wlan_objmgr_pdev *pdev = load->pdev;

	reg_call_chan_change_cbks(psoc, pdev);

	wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
	qdf_mem_free(load);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_send_scheduler_msg_sb(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_pdev *pdev)
{
	struct scheduler_msg msg = {0};
	struct reg_sched_payload *payload;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_alert("pdev reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!pdev_priv_obj->pdev_opened) {
		reg_err("hlos not initialized");
		return QDF_STATUS_E_FAILURE;
	}

	if (!pdev_priv_obj->chan_list_recvd) {
		reg_err("Empty channel list");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}

	status = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
		reg_err("error taking pdev ref cnt");
		return status;
	}

	reg_alloc_and_fill_payload(psoc, pdev, &payload);
	if (!payload) {
		reg_err("malloc failed");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
		return QDF_STATUS_E_NOMEM;
	}

	msg.bodyptr = payload;
	msg.callback = reg_sched_chan_change_cbks_sb;
	msg.flush_callback = reg_chan_change_flush_cbk_sb;

	status = scheduler_post_message(QDF_MODULE_ID_REGULATORY,
					QDF_MODULE_ID_REGULATORY,
					QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_SB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
		qdf_mem_free(payload);
	}

	return status;
}

QDF_STATUS reg_send_scheduler_msg_nb(struct wlan_objmgr_psoc *psoc,
				     struct wlan_objmgr_pdev *pdev)
{
	struct scheduler_msg msg = {0};
	struct reg_sched_payload *payload;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);
	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_alert("pdev reg component is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!pdev_priv_obj->pdev_opened) {
		reg_err("hlos not initialized");
		return QDF_STATUS_E_FAILURE;
	}

	if (!pdev_priv_obj->chan_list_recvd) {
		reg_err("Empty channel list");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_REGULATORY_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}

	status = wlan_objmgr_pdev_try_get_ref(pdev, WLAN_REGULATORY_NB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
		reg_err("error taking pdev ref cnt");
		return status;
	}

	reg_alloc_and_fill_payload(psoc, pdev, &payload);
	if (!payload) {
		reg_err("malloc failed");
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
		return QDF_STATUS_E_NOMEM;
	}
	msg.bodyptr = payload;
	msg.callback = reg_sched_chan_change_cbks_nb;
	msg.flush_callback = reg_chan_change_flush_cbk_nb;

	status = scheduler_post_message(QDF_MODULE_ID_REGULATORY,
					QDF_MODULE_ID_REGULATORY,
					QDF_MODULE_ID_OS_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status)) {
		wlan_objmgr_pdev_release_ref(pdev, WLAN_REGULATORY_NB_ID);
		wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_NB_ID);
		qdf_mem_free(payload);
	}

	return status;
}

QDF_STATUS reg_notify_sap_event(struct wlan_objmgr_pdev *pdev,
				bool sap_state)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_regulatory_pdev_priv_obj *pdev_priv_obj;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	pdev_priv_obj = reg_get_pdev_obj(pdev);

	if (!IS_VALID_PDEV_REG_OBJ(pdev_priv_obj)) {
		reg_err("pdev reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		reg_err("psoc is NULL");
		return QDF_STATUS_E_INVAL;
	}

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!IS_VALID_PSOC_REG_OBJ(psoc_priv_obj)) {
		reg_err("psoc reg component is NULL");
		return QDF_STATUS_E_INVAL;
	}

	reg_info("sap_state: %d", sap_state);

	if (pdev_priv_obj->sap_state == sap_state)
		return QDF_STATUS_SUCCESS;

	pdev_priv_obj->sap_state = sap_state;
	set_disable_channel_state(pdev_priv_obj);

	reg_compute_pdev_current_chan_list(pdev_priv_obj);
	status = reg_send_scheduler_msg_sb(psoc, pdev);

	return status;
}

void reg_register_chan_change_callback(struct wlan_objmgr_psoc *psoc,
				       reg_chan_change_callback cbk, void *arg)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	uint32_t count;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	qdf_spin_lock_bh(&psoc_priv_obj->cbk_list_lock);
	for (count = 0; count < REG_MAX_CHAN_CHANGE_CBKS; count++)
		if (!psoc_priv_obj->cbk_list[count].cbk) {
			psoc_priv_obj->cbk_list[count].cbk = cbk;
			psoc_priv_obj->cbk_list[count].arg = arg;
			psoc_priv_obj->num_chan_change_cbks++;
			break;
		}
	qdf_spin_unlock_bh(&psoc_priv_obj->cbk_list_lock);

	if (count == REG_MAX_CHAN_CHANGE_CBKS)
		reg_err("callback list is full");
}

void reg_unregister_chan_change_callback(struct wlan_objmgr_psoc *psoc,
					 reg_chan_change_callback cbk)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	uint32_t count;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return;
	}

	qdf_spin_lock_bh(&psoc_priv_obj->cbk_list_lock);
	for (count = 0; count < REG_MAX_CHAN_CHANGE_CBKS; count++)
		if (psoc_priv_obj->cbk_list[count].cbk == cbk) {
			psoc_priv_obj->cbk_list[count].cbk = NULL;
			psoc_priv_obj->num_chan_change_cbks--;
			break;
		}
	qdf_spin_unlock_bh(&psoc_priv_obj->cbk_list_lock);

	if (count == REG_MAX_CHAN_CHANGE_CBKS)
		reg_err("callback not found in the list");
}

