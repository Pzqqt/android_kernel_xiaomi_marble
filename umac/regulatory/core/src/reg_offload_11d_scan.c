/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
 * DOC: Add 11d utility functions
 */

#include <wlan_cmn.h>
#include <reg_services_public_struct.h>
#include <wlan_scan_public_structs.h>
#include <wlan_scan_ucfg_api.h>
#include <wlan_objmgr_psoc_obj.h>
#include "reg_priv_objs.h"
#include "reg_utils.h"
#include "reg_services_common.h"
#include "reg_offload_11d_scan.h"
#include "reg_host_11d.h"

#ifdef TARGET_11D_SCAN

QDF_STATUS reg_set_11d_country(struct wlan_objmgr_pdev *pdev,
			       uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct set_country country_code;
	struct wlan_objmgr_psoc *psoc;
	struct cc_regdmn_s rd;
	QDF_STATUS status;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;
	uint8_t pdev_id;

	if (!country) {
		reg_err("Null country code");
		return QDF_STATUS_E_INVAL;
	}

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);

	psoc = wlan_pdev_get_psoc(pdev);
	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("Null psoc reg component");
		return QDF_STATUS_E_INVAL;
	}

	if (!qdf_mem_cmp(psoc_priv_obj->cur_country, country, REG_ALPHA2_LEN)) {
		if (psoc_priv_obj->cc_src == SOURCE_11D) {
			reg_debug("same country");
			return QDF_STATUS_SUCCESS;
		}
	}

	reg_info("set new 11d country:%c%c to fW",
		 country[0], country[1]);

	qdf_mem_copy(country_code.country, country, REG_ALPHA2_LEN + 1);
	country_code.pdev_id = pdev_id;

	psoc_priv_obj->new_11d_ctry_pending[pdev_id] = true;

	if (psoc_priv_obj->offload_enabled) {
		tx_ops = reg_get_psoc_tx_ops(psoc);
		if (tx_ops->set_country_code) {
			tx_ops->set_country_code(psoc, &country_code);
		} else {
			reg_err("country set fw handler not present");
			psoc_priv_obj->new_11d_ctry_pending[pdev_id] = false;
			return QDF_STATUS_E_FAULT;
		}
		status = QDF_STATUS_SUCCESS;
	} else {
		qdf_mem_copy(rd.cc.alpha, country, REG_ALPHA2_LEN + 1);
		rd.flags = ALPHA_IS_SET;
		reg_program_chan_list(pdev, &rd);
		status = QDF_STATUS_SUCCESS;
	}

	return status;
}

/**
 * reg_send_11d_flush_cbk() - release 11d psoc reference
 * @msg: Pointer to scheduler message.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS reg_send_11d_flush_cbk(struct scheduler_msg *msg)
{
	struct reg_11d_scan_msg *scan_msg_11d = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = scan_msg_11d->psoc;

	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
	qdf_mem_free(scan_msg_11d);

	return QDF_STATUS_SUCCESS;
}

/**
 * reg_send_11d_msg_cbk() - Send start/stop 11d scan message.
 * @msg: Pointer to scheduler message.
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS reg_send_11d_msg_cbk(struct scheduler_msg *msg)
{
	struct reg_11d_scan_msg *scan_msg_11d = msg->bodyptr;
	struct wlan_objmgr_psoc *psoc = scan_msg_11d->psoc;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;
	struct reg_start_11d_scan_req start_req;
	struct reg_stop_11d_scan_req stop_req;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	tx_ops = reg_get_psoc_tx_ops(psoc);

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("Null psoc priv obj");
		goto end;
	}

	if (psoc_priv_obj->vdev_id_for_11d_scan == INVALID_VDEV_ID) {
		psoc_priv_obj->enable_11d_supp = false;
		reg_err("Invalid vdev");
		goto end;
	}

	if (scan_msg_11d->enable_11d_supp) {
		start_req.vdev_id = psoc_priv_obj->vdev_id_for_11d_scan;
		start_req.scan_period_msec = psoc_priv_obj->scan_11d_interval;
		start_req.start_interval_msec = 0;
		reg_debug("sending start msg");
		tx_ops->start_11d_scan(psoc, &start_req);
	} else {
		stop_req.vdev_id = psoc_priv_obj->vdev_id_for_11d_scan;
		reg_debug("sending stop msg");
		tx_ops->stop_11d_scan(psoc, &stop_req);
	}

end:
	qdf_mem_free(scan_msg_11d);
	wlan_objmgr_psoc_release_ref(psoc, WLAN_REGULATORY_SB_ID);
	return QDF_STATUS_SUCCESS;
}

/**
 * reg_sched_11d_msg() - Schedules 11d scan message.
 * @scan_msg_11d: 11d scan message
 */
static QDF_STATUS reg_sched_11d_msg(struct reg_11d_scan_msg *scan_msg_11d)
{
	struct scheduler_msg msg = {0};
	QDF_STATUS status;

	status = wlan_objmgr_psoc_try_get_ref(scan_msg_11d->psoc,
					      WLAN_REGULATORY_SB_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		reg_err("error taking psoc ref cnt");
		return status;
	}

	msg.bodyptr = scan_msg_11d;
	msg.callback = reg_send_11d_msg_cbk;
	msg.flush_callback = reg_send_11d_flush_cbk;

	status = scheduler_post_message(QDF_MODULE_ID_REGULATORY,
					QDF_MODULE_ID_REGULATORY,
					QDF_MODULE_ID_TARGET_IF, &msg);
	if (QDF_IS_STATUS_ERROR(status))
		wlan_objmgr_psoc_release_ref(scan_msg_11d->psoc,
					     WLAN_REGULATORY_SB_ID);

	return status;
}

void reg_run_11d_state_machine(struct wlan_objmgr_psoc *psoc)
{
	bool temp_11d_support;
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	bool world_mode;
	struct reg_11d_scan_msg *scan_msg_11d;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("Null reg psoc private obj");
		return;
	}

	if (psoc_priv_obj->vdev_id_for_11d_scan == INVALID_VDEV_ID) {
		psoc_priv_obj->enable_11d_supp = false;
		reg_err("Invalid vdev");
		return;
	}

	world_mode = reg_is_world_alpha2(psoc_priv_obj->cur_country);

	temp_11d_support = psoc_priv_obj->enable_11d_supp;
	if ((psoc_priv_obj->enable_11d_in_world_mode) && (world_mode))
		psoc_priv_obj->enable_11d_supp = true;
	else if (((psoc_priv_obj->user_ctry_set) &&
		  (psoc_priv_obj->user_ctry_priority)) ||
		 (psoc_priv_obj->master_vdev_cnt))
		psoc_priv_obj->enable_11d_supp = false;
	else
		psoc_priv_obj->enable_11d_supp =
			psoc_priv_obj->enable_11d_supp_original;

	reg_debug("inside 11d state machine:tmp %d 11d_supp %d org %d set %d pri %d cnt %d vdev %d",
		  temp_11d_support,
		  psoc_priv_obj->enable_11d_supp,
		  psoc_priv_obj->enable_11d_supp_original,
		  psoc_priv_obj->user_ctry_set,
		  psoc_priv_obj->user_ctry_priority,
		  psoc_priv_obj->master_vdev_cnt,
		  psoc_priv_obj->vdev_id_for_11d_scan);

	if (temp_11d_support != psoc_priv_obj->enable_11d_supp) {
		if (psoc_priv_obj->is_11d_offloaded) {
			scan_msg_11d = qdf_mem_malloc(sizeof(*scan_msg_11d));
			if (!scan_msg_11d)
				return;
			scan_msg_11d->psoc = psoc;
			scan_msg_11d->enable_11d_supp =
						psoc_priv_obj->enable_11d_supp;
			reg_sched_11d_msg(scan_msg_11d);
		} else {
			reg_11d_host_scan(psoc_priv_obj);
		}
	}
}

QDF_STATUS reg_11d_vdev_created_update(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_objmgr_pdev *parent_pdev;
	struct wlan_objmgr_psoc *parent_psoc;
	uint32_t vdev_id;
	enum QDF_OPMODE op_mode;
	uint8_t i;

	op_mode = wlan_vdev_mlme_get_opmode(vdev);

	parent_pdev = wlan_vdev_get_pdev(vdev);
	parent_psoc = wlan_pdev_get_psoc(parent_pdev);

	psoc_priv_obj = reg_get_psoc_obj(parent_psoc);
	if (!psoc_priv_obj) {
		reg_err("reg psoc private obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	if ((op_mode == QDF_STA_MODE) ||
	    (op_mode == QDF_P2P_DEVICE_MODE) ||
	    (op_mode == QDF_P2P_CLIENT_MODE)) {
		vdev_id = wlan_vdev_get_id(vdev);
		if (!psoc_priv_obj->vdev_cnt_11d) {
			psoc_priv_obj->vdev_id_for_11d_scan = vdev_id;
			reg_debug("running 11d state machine, opmode %d",
				  op_mode);
			reg_run_11d_state_machine(parent_psoc);
		}

		for (i = 0; i < MAX_STA_VDEV_CNT; i++) {
			if (psoc_priv_obj->vdev_ids_11d[i] == INVALID_VDEV_ID) {
				psoc_priv_obj->vdev_ids_11d[i] = vdev_id;
				break;
			}
		}
		psoc_priv_obj->vdev_cnt_11d++;
	}

	if ((op_mode == QDF_P2P_GO_MODE) || (op_mode == QDF_SAP_MODE)) {
		reg_debug("running 11d state machine, opmode %d", op_mode);
		psoc_priv_obj->master_vdev_cnt++;
		reg_run_11d_state_machine(parent_psoc);
	}

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS reg_11d_vdev_delete_update(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_objmgr_pdev *parent_pdev;
	struct wlan_objmgr_psoc *parent_psoc;
	enum QDF_OPMODE op_mode;
	uint32_t vdev_id;
	uint8_t i;

	if (!vdev) {
		reg_err("NULL vdev");
		return QDF_STATUS_E_INVAL;
	}
	op_mode = wlan_vdev_mlme_get_opmode(vdev);

	parent_pdev = wlan_vdev_get_pdev(vdev);
	parent_psoc = wlan_pdev_get_psoc(parent_pdev);

	psoc_priv_obj = reg_get_psoc_obj(parent_psoc);
	if (!psoc_priv_obj) {
		reg_err("NULL reg psoc private obj");
		return QDF_STATUS_E_FAULT;
	}

	if ((op_mode == QDF_P2P_GO_MODE) || (op_mode == QDF_SAP_MODE)) {
		psoc_priv_obj->master_vdev_cnt--;
		reg_debug("run 11d state machine, deleted opmode %d",
			  op_mode);
		reg_run_11d_state_machine(parent_psoc);
		return QDF_STATUS_SUCCESS;
	}

	if ((op_mode == QDF_STA_MODE) || (op_mode == QDF_P2P_DEVICE_MODE) ||
	    (op_mode == QDF_P2P_CLIENT_MODE)) {
		vdev_id = wlan_vdev_get_id(vdev);
		for (i = 0; i < MAX_STA_VDEV_CNT; i++) {
			if (psoc_priv_obj->vdev_ids_11d[i] == vdev_id) {
				psoc_priv_obj->vdev_ids_11d[i] =
					INVALID_VDEV_ID;
				psoc_priv_obj->vdev_cnt_11d--;
				break;
			}
		}

		if (psoc_priv_obj->vdev_id_for_11d_scan != vdev_id)
			return QDF_STATUS_SUCCESS;

		if (!psoc_priv_obj->vdev_cnt_11d) {
			psoc_priv_obj->vdev_id_for_11d_scan = INVALID_VDEV_ID;
			psoc_priv_obj->enable_11d_supp = false;
			return QDF_STATUS_SUCCESS;
		}

		for (i = 0; i < MAX_STA_VDEV_CNT; i++) {
			if (psoc_priv_obj->vdev_ids_11d[i] == INVALID_VDEV_ID)
				continue;
			psoc_priv_obj->vdev_id_for_11d_scan =
				psoc_priv_obj->vdev_ids_11d[i];
			psoc_priv_obj->enable_11d_supp = false;
			reg_debug("running 11d state machine, vdev %d",
				  psoc_priv_obj->vdev_id_for_11d_scan);
			reg_run_11d_state_machine(parent_psoc);
			break;
		}
	}

	return QDF_STATUS_SUCCESS;
}

bool reg_is_11d_scan_inprogress(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("NULL reg psoc private obj");
		return false;
	}

	return psoc_priv_obj->enable_11d_supp;
}

QDF_STATUS reg_save_new_11d_country(struct wlan_objmgr_psoc *psoc,
				    uint8_t *country)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;
	struct wlan_lmac_if_reg_tx_ops *tx_ops;
	struct set_country country_code;
	uint8_t pdev_id;
	uint8_t ctr;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("NULL reg psoc private obj");

		return QDF_STATUS_E_FAILURE;
	}

	/*
	 * Need firmware to send channel list event
	 * for all phys. Therefore set pdev_id to 0xFF
	 */
	pdev_id = 0xFF;
	for (ctr = 0; ctr < psoc_priv_obj->num_phy; ctr++)
		psoc_priv_obj->new_11d_ctry_pending[ctr] = true;

	qdf_mem_copy(country_code.country, country, REG_ALPHA2_LEN + 1);
	country_code.pdev_id = pdev_id;

	if (psoc_priv_obj->offload_enabled) {
		tx_ops = reg_get_psoc_tx_ops(psoc);
		if (tx_ops->set_country_code) {
			tx_ops->set_country_code(psoc, &country_code);
		} else {
			reg_err("NULL country set handler");
			for (ctr = 0; ctr < psoc_priv_obj->num_phy; ctr++)
				psoc_priv_obj->new_11d_ctry_pending[ctr] =
					false;
			return QDF_STATUS_E_FAULT;
		}
	}

	return QDF_STATUS_SUCCESS;
}

bool reg_11d_enabled_on_host(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("NULL reg psoc private obj");
		return QDF_STATUS_E_FAILURE;
	}

	return (psoc_priv_obj->enable_11d_supp &&
		!psoc_priv_obj->is_11d_offloaded);
}

QDF_STATUS reg_set_11d_offloaded(struct wlan_objmgr_psoc *psoc, bool val)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("NULL psoc reg component");
		return QDF_STATUS_E_FAILURE;
	}

	psoc_priv_obj->is_11d_offloaded = val;
	reg_debug("set is_11d_offloaded %d", val);
	return QDF_STATUS_SUCCESS;
}

bool reg_is_11d_offloaded(struct wlan_objmgr_psoc *psoc)
{
	struct wlan_regulatory_psoc_priv_obj *psoc_priv_obj;

	psoc_priv_obj = reg_get_psoc_obj(psoc);
	if (!psoc_priv_obj) {
		reg_err("NULL reg psoc private obj");
		return false;
	}

	return psoc_priv_obj->is_11d_offloaded;
}
#endif
