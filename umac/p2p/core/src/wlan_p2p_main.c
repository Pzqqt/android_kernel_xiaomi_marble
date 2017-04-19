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

/**
 * DOC: This file contains main P2P function definitions
 */

#include <scheduler_api.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_scan_ucfg_api.h>
#include "wlan_p2p_public_struct.h"
#include "wlan_p2p_ucfg_api.h"
#include "wlan_p2p_tgt_api.h"
#include "wlan_p2p_main.h"
#include "wlan_p2p_roc.h"
#include "wlan_p2p_off_chan_tx.h"

/**
 * p2p_get_cmd_type_str() - parse cmd to string
 * @cmd_type: P2P cmd type
 *
 * This function parse P2P cmd to string.
 *
 * Return: command string
 */
static char *p2p_get_cmd_type_str(enum p2p_cmd_type cmd_type)
{
	switch (cmd_type) {
	case P2P_ROC_REQ:
		return "P2P roc request";
	case P2P_CANCEL_ROC_REQ:
		return "P2P cancel roc request";
	case P2P_MGMT_TX:
		return "P2P mgmt tx request";
	case P2P_MGMT_TX_CANCEL:
		return "P2P cancel mgmt tx request";
	default:
		return "Invalid P2P command";
	}
}

/**
 * p2p_get_event_type_str() - parase event to string
 * @event_type: P2P event type
 *
 * This function parse P2P event to string.
 *
 * Return: event string
 */
static char *p2p_get_event_type_str(enum p2p_event_type event_type)
{
	switch (event_type) {
	case P2P_EVENT_SCAN_EVENT:
		return "P2P scan event";
	case P2P_EVENT_MGMT_TX_ACK_CNF:
		return "P2P mgmt tx ack event";
	case P2P_EVENT_RX_MGMT:
		return "P2P mgmt rx event";
	case P2P_EVENT_LO_STOPPED:
		return "P2P lo stop event";
	case P2P_EVENT_NOA:
		return "P2P noa event";
	default:
		return "Invalid P2P event";
	}
}

/**
 * p2p_psoc_obj_create_notification() - Function to allocate per P2P
 * soc private object
 * @soc: soc context
 * @data: Pointer to data
 *
 * This function gets called from object manager when psoc is being
 * created and creates p2p soc context.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS p2p_psoc_obj_create_notification(
	struct wlan_objmgr_psoc *soc, void *data)
{
	struct p2p_soc_priv_obj *p2p_soc_obj;
	QDF_STATUS status;

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = qdf_mem_malloc(sizeof(*p2p_soc_obj));
	if (!p2p_soc_obj) {
		p2p_err("Failed to allocate p2p soc private object");
		return QDF_STATUS_E_NOMEM;
	}

	p2p_soc_obj->soc = soc;

	status = wlan_objmgr_psoc_component_obj_attach(soc,
				WLAN_UMAC_COMP_P2P, p2p_soc_obj,
				QDF_STATUS_SUCCESS);
	if (status != QDF_STATUS_SUCCESS) {
		qdf_mem_free(p2p_soc_obj);
		p2p_err("Failed to attach p2p component, %d", status);
		return status;
	}

	p2p_debug("p2p soc object create successful, %p", p2p_soc_obj);

	return QDF_STATUS_SUCCESS;
}

/**
 * p2p_psoc_obj_destroy_notification() - Free soc private object
 * @soc: soc context
 * @data: Pointer to data
 *
 * This function gets called from object manager when psoc is being
 * deleted and delete p2p soc context.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS p2p_psoc_obj_destroy_notification(
	struct wlan_objmgr_psoc *soc, void *data)
{
	struct p2p_soc_priv_obj *p2p_soc_obj;
	QDF_STATUS status;

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("p2p soc private object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	p2p_soc_obj->soc = NULL;

	status = wlan_objmgr_psoc_component_obj_detach(soc,
				WLAN_UMAC_COMP_P2P, p2p_soc_obj);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to detach p2p component, %d", status);
		return status;
	}

	p2p_debug("destroy p2p soc object, %p", p2p_soc_obj);

	qdf_mem_free(p2p_soc_obj);

	return QDF_STATUS_SUCCESS;
}

/**
 * p2p_vdev_obj_create_notification() - Allocate per p2p vdev object
 * @vdev: vdev context
 * @data: Pointer to data
 *
 * This function gets called from object manager when vdev is being
 * created and creates p2p vdev context.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS p2p_vdev_obj_create_notification(
	struct wlan_objmgr_vdev *vdev, void *data)
{
	struct p2p_vdev_priv_obj *p2p_vdev_obj;
	QDF_STATUS status;
	enum tQDF_ADAPTER_MODE mode;

	if (!vdev) {
		p2p_err("vdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	wlan_vdev_obj_lock(vdev);
	mode = wlan_vdev_mlme_get_opmode(vdev);
	wlan_vdev_obj_unlock(vdev);
	p2p_debug("vdev mode:%d", mode);
	if (mode != QDF_P2P_GO_MODE) {
		p2p_debug("won't create p2p vdev private object if it is not GO");
		return QDF_STATUS_SUCCESS;
	}

	p2p_vdev_obj =
		qdf_mem_malloc(sizeof(*p2p_vdev_obj));
	if (!p2p_vdev_obj) {
		p2p_err("Failed to allocate p2p vdev object");
		return QDF_STATUS_E_NOMEM;
	}

	p2p_vdev_obj->vdev = vdev;

	status = wlan_objmgr_vdev_component_obj_attach(vdev,
				WLAN_UMAC_COMP_P2P, p2p_vdev_obj,
				QDF_STATUS_SUCCESS);
	if (status != QDF_STATUS_SUCCESS) {
		qdf_mem_free(p2p_vdev_obj);
		p2p_err("Failed to attach p2p component to vdev, %d",
			status);
		return status;
	}

	p2p_debug("p2p vdev object create successful, %p", p2p_vdev_obj);

	return QDF_STATUS_SUCCESS;
}

/**
 * p2p_vdev_obj_destroy_notification() - Free per P2P vdev object
 * @vdev: vdev context
 * @data: Pointer to data
 *
 * This function gets called from object manager when vdev is being
 * deleted and delete p2p vdev context.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS p2p_vdev_obj_destroy_notification(
	struct wlan_objmgr_vdev *vdev, void *data)
{
	struct p2p_vdev_priv_obj *p2p_vdev_obj;
	QDF_STATUS status;
	enum tQDF_ADAPTER_MODE mode;

	if (!vdev) {
		p2p_err("vdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	wlan_vdev_obj_lock(vdev);
	mode = wlan_vdev_mlme_get_opmode(vdev);
	wlan_vdev_obj_unlock(vdev);
	p2p_debug("vdev mode:%d", mode);
	if (mode != QDF_P2P_GO_MODE) {
		p2p_debug("no p2p vdev private object if it is not GO");
		return QDF_STATUS_SUCCESS;
	}

	p2p_vdev_obj = wlan_objmgr_vdev_get_comp_private_obj(vdev,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_vdev_obj) {
		p2p_debug("p2p vdev object is NULL");
		return QDF_STATUS_SUCCESS;
	}

	p2p_vdev_obj->vdev = NULL;

	status = wlan_objmgr_vdev_component_obj_detach(vdev,
				WLAN_UMAC_COMP_P2P, p2p_vdev_obj);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to detach p2p component, %d", status);
		return status;
	}

	p2p_debug("destroy p2p vdev object, p2p vdev obj:%p, noa info:%p",
		p2p_vdev_obj, p2p_vdev_obj->noa_info);

	if (p2p_vdev_obj->noa_info)
		qdf_mem_free(p2p_vdev_obj->noa_info);

	qdf_mem_free(p2p_vdev_obj);

	return QDF_STATUS_SUCCESS;
}

/**
 * p2p_send_noa_to_pe() - send noa information to pe
 * @noa_info: vdev context
 *
 * This function sends noa information to pe since MCL layer need noa
 * event.
 *
 * Return: QDF_STATUS_SUCCESS - in case of success
 */
static QDF_STATUS p2p_send_noa_to_pe(struct p2p_noa_info *noa_info)
{
	struct p2p_noa_attr *noa_attr;
	struct scheduler_msg msg = {0};

	if (!noa_info) {
		p2p_err("noa info is null");
		return QDF_STATUS_E_INVAL;
	}

	noa_attr = qdf_mem_malloc(sizeof(*noa_attr));
	if (!noa_attr) {
		p2p_err("Failed to allocate memory for tSirP2PNoaAttr");
		return QDF_STATUS_E_NOMEM;
	}

	noa_attr->index = noa_info->index;
	noa_attr->opps_ps = noa_info->opps_ps;
	noa_attr->ct_win = noa_info->ct_window;
	if (!noa_info->num_desc) {
		p2p_debug("Zero noa descriptors");
	} else {
		p2p_debug("%d noa descriptors", noa_info->num_desc);

		noa_attr->noa1_count =
			noa_info->noa_desc[0].type_count;
		noa_attr->noa1_duration =
			noa_info->noa_desc[0].duration;
		noa_attr->noa1_interval =
			noa_info->noa_desc[0].interval;
		noa_attr->noa1_start_time =
			noa_info->noa_desc[0].start_time;
		if (noa_info->num_desc > 1) {
			noa_attr->noa2_count =
				noa_info->noa_desc[1].type_count;
			noa_attr->noa2_duration =
				noa_info->noa_desc[1].duration;
			noa_attr->noa2_interval =
				noa_info->noa_desc[1].interval;
			noa_attr->noa2_start_time =
				noa_info->noa_desc[1].start_time;
		}
	}

	p2p_debug("Sending P2P_NOA_ATTR_IND to pe");

	msg.type = P2P_NOA_ATTR_IND;
	msg.bodyval = 0;
	msg.bodyptr = noa_attr;
	scheduler_post_msg(QDF_MODULE_ID_PE,  &msg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_component_init(void)
{
	QDF_STATUS status;

	status = wlan_objmgr_register_psoc_create_handler(
				WLAN_UMAC_COMP_P2P,
				p2p_psoc_obj_create_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to register p2p obj create handler");
		goto err_reg_psoc_create;
	}

	status = wlan_objmgr_register_psoc_destroy_handler(
				WLAN_UMAC_COMP_P2P,
				p2p_psoc_obj_destroy_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to register p2p obj delete handler");
		goto err_reg_psoc_delete;
	}

	status = wlan_objmgr_register_vdev_create_handler(
				WLAN_UMAC_COMP_P2P,
				p2p_vdev_obj_create_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to register p2p vdev create handler");
		goto err_reg_vdev_create;
	}

	status = wlan_objmgr_register_vdev_destroy_handler(
				WLAN_UMAC_COMP_P2P,
				p2p_vdev_obj_destroy_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to register p2p vdev delete handler");
		goto err_reg_vdev_delete;
	}

	p2p_debug("Register p2p obj handler successful");

	return QDF_STATUS_SUCCESS;
err_reg_vdev_delete:
	wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_P2P,
			p2p_vdev_obj_create_notification, NULL);
err_reg_vdev_create:
	wlan_objmgr_unregister_psoc_destroy_handler(WLAN_UMAC_COMP_P2P,
			p2p_psoc_obj_destroy_notification, NULL);
err_reg_psoc_delete:
	wlan_objmgr_unregister_psoc_create_handler(WLAN_UMAC_COMP_P2P,
			p2p_psoc_obj_create_notification, NULL);
err_reg_psoc_create:
	return status;
}

QDF_STATUS p2p_component_deinit(void)
{
	QDF_STATUS status;
	QDF_STATUS ret_status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_unregister_vdev_create_handler(
				WLAN_UMAC_COMP_P2P,
				p2p_vdev_obj_create_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to unregister p2p vdev create handler, %d",
			status);
		ret_status = status;
	}

	status = wlan_objmgr_unregister_vdev_destroy_handler(
				WLAN_UMAC_COMP_P2P,
				p2p_vdev_obj_destroy_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to unregister p2p vdev delete handler, %d",
			status);
		ret_status = status;
	}

	status = wlan_objmgr_unregister_psoc_create_handler(
				WLAN_UMAC_COMP_P2P,
				p2p_psoc_obj_create_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to unregister p2p obj create handler, %d",
			status);
		ret_status = status;
	}

	status = wlan_objmgr_unregister_psoc_destroy_handler(
				WLAN_UMAC_COMP_P2P,
				p2p_psoc_obj_destroy_notification,
				NULL);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to unregister p2p obj delete handler, %d",
			status);
		ret_status = status;
	}

	p2p_debug("Unregister p2p obj handler complete");

	return ret_status;
}

QDF_STATUS p2p_psoc_object_open(struct wlan_objmgr_psoc *soc)
{
	QDF_STATUS status;
	struct p2p_soc_priv_obj *p2p_soc_obj;

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("p2p soc priviate object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_list_create(&p2p_soc_obj->roc_q, MAX_QUEUE_LENGTH);
	qdf_list_create(&p2p_soc_obj->tx_q_roc, MAX_QUEUE_LENGTH);
	qdf_list_create(&p2p_soc_obj->tx_q_ack, MAX_QUEUE_LENGTH);

	status = qdf_event_create(&p2p_soc_obj->cancel_roc_done);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("failed to create cancel roc done event");
		goto fail_event;
	}
	p2p_soc_obj->roc_runtime_lock = qdf_runtime_lock_init(
						P2P_MODULE_NAME);

	p2p_debug("p2p psoc object open successful");

	return QDF_STATUS_SUCCESS;

fail_event:
	qdf_list_destroy(&p2p_soc_obj->tx_q_ack);
	qdf_list_destroy(&p2p_soc_obj->tx_q_roc);
	qdf_list_destroy(&p2p_soc_obj->roc_q);

	return status;
}

QDF_STATUS p2p_psoc_object_close(struct wlan_objmgr_psoc *soc)
{
	struct p2p_soc_priv_obj *p2p_soc_obj;

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("p2p soc object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_runtime_lock_deinit(p2p_soc_obj->roc_runtime_lock);
	qdf_event_destroy(&p2p_soc_obj->cancel_roc_done);
	qdf_list_destroy(&p2p_soc_obj->tx_q_ack);
	qdf_list_destroy(&p2p_soc_obj->tx_q_roc);
	qdf_list_destroy(&p2p_soc_obj->roc_q);

	p2p_debug("p2p psoc object close successful");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_psoc_start(struct wlan_objmgr_psoc *soc,
	struct p2p_start_param *req)
{
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct p2p_start_param *start_param;

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("P2P soc object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	start_param = qdf_mem_malloc(sizeof(*start_param));
	if (!start_param) {
		p2p_err("Failed to allocate start params");
		return QDF_STATUS_E_NOMEM;
	}
	start_param->rx_cb = req->rx_cb;
	start_param->rx_cb_data = req->rx_cb_data;
	start_param->event_cb = req->event_cb;
	start_param->event_cb_data = req->event_cb_data;
	start_param->tx_cnf_cb = req->tx_cnf_cb;
	start_param->tx_cnf_cb_data = req->tx_cnf_cb_data;
	start_param->lo_event_cb = req->lo_event_cb;
	start_param->lo_event_cb_data = req->lo_event_cb_data;
	p2p_soc_obj->start_param = start_param;

	/* register p2p lo stop and noa event */
	tgt_p2p_register_lo_ev_handler(soc);
	tgt_p2p_register_noa_ev_handler(soc);

	/* register scan request id */
	p2p_soc_obj->scan_req_id = ucfg_scan_register_requester(
		soc, P2P_MODULE_NAME, tgt_p2p_scan_event_cb,
		p2p_soc_obj);

	p2p_debug("p2p psoc start successful, scan request id:%d",
		p2p_soc_obj->scan_req_id);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_psoc_stop(struct wlan_objmgr_psoc *soc)
{
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct p2p_start_param *start_param;

	if (!soc) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (!p2p_soc_obj) {
		p2p_err("P2P soc object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	start_param = p2p_soc_obj->start_param;
	p2p_soc_obj->start_param = NULL;
	if (!start_param) {
		p2p_err("start parameters is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	/* clean up queue of p2p psoc private object */
	p2p_cleanup_tx_queue(p2p_soc_obj);
	p2p_cleanup_roc_queue(p2p_soc_obj);

	/* unrgister scan request id*/
	ucfg_scan_unregister_requester(soc, p2p_soc_obj->scan_req_id);

	/* unregister p2p lo stop and noa event */
	tgt_p2p_unregister_lo_ev_handler(soc);
	tgt_p2p_unregister_noa_ev_handler(soc);

	start_param->rx_cb = NULL;
	start_param->rx_cb_data = NULL;
	start_param->event_cb = NULL;
	start_param->event_cb_data = NULL;
	start_param->tx_cnf_cb = NULL;
	start_param->tx_cnf_cb_data = NULL;
	qdf_mem_free(start_param);

	p2p_debug("p2p psoc stop successful");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_process_cmd(struct scheduler_msg *msg)
{
	QDF_STATUS status;

	p2p_debug("msg type %d, %s", msg->type,
		p2p_get_cmd_type_str(msg->type));

	if (!(msg->bodyptr)) {
		p2p_err("Invalid message body");
		return QDF_STATUS_E_INVAL;
	}
	switch (msg->type) {
	case P2P_ROC_REQ:
		status = p2p_process_roc_req(
				(struct p2p_roc_context *)
				msg->bodyptr);
		break;
	case P2P_CANCEL_ROC_REQ:
		status = p2p_process_cancel_roc_req(
				(struct cancel_roc_context *)
				msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	case P2P_MGMT_TX:
		status = p2p_process_mgmt_tx(
				(struct tx_action_context *)
				msg->bodyptr);
		break;
	case P2P_MGMT_TX_CANCEL:
		status = p2p_process_mgmt_tx_cancel(
				(struct cancel_roc_context *)
				msg->bodyptr);
		qdf_mem_free(msg->bodyptr);
		break;
	default:
		p2p_err("drop unexpected message received %d",
			msg->type);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	return status;
}

QDF_STATUS p2p_process_evt(struct scheduler_msg *msg)
{
	QDF_STATUS status;

	p2p_debug("msg type %d, %s", msg->type,
		p2p_get_event_type_str(msg->type));

	if (!(msg->bodyptr)) {
		p2p_err("Invalid message body");
		return QDF_STATUS_E_INVAL;
	}

	switch (msg->type) {
	case P2P_EVENT_MGMT_TX_ACK_CNF:
		status = p2p_process_mgmt_tx_ack_cnf(
				(struct p2p_tx_conf_event *)
				msg->bodyptr);
		break;
	case P2P_EVENT_RX_MGMT:
		status  = p2p_process_rx_mgmt(
				(struct p2p_rx_mgmt_event *)
				msg->bodyptr);
		break;
	case P2P_EVENT_LO_STOPPED:
		status = p2p_process_lo_stop(
				(struct p2p_lo_stop_event *)
				msg->bodyptr);
		break;
	case P2P_EVENT_NOA:
		status = p2p_process_noa(
				(struct p2p_noa_event *)
				msg->bodyptr);
		break;
	default:
		p2p_err("Drop unexpected message received %d",
			msg->type);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	qdf_mem_free(msg->bodyptr);
	msg->bodyptr = NULL;

	return status;
}

QDF_STATUS p2p_process_lo_stop(
	struct p2p_lo_stop_event *lo_stop_event)
{
	struct p2p_lo_event *lo_evt;
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct p2p_start_param *start_param;

	if (!lo_stop_event) {
		p2p_err("invalid lo stop event");
		return QDF_STATUS_E_INVAL;
	}
	lo_evt = lo_stop_event->lo_event;
	p2p_soc_obj = lo_stop_event->p2p_soc_obj;

	p2p_debug("vdev_id %d, reason %d",
		lo_evt->vdev_id, lo_evt->reason_code);

	if (!p2p_soc_obj || !(p2p_soc_obj->start_param)) {
		p2p_err("Invalid p2p soc object or start parameters");
		return QDF_STATUS_E_INVAL;
	}
	start_param = p2p_soc_obj->start_param;
	if (start_param->lo_event_cb)
		start_param->lo_event_cb(
			start_param->lo_event_cb_data, lo_evt);
	else
		p2p_err("Invalid p2p soc obj or hdd lo event callback");

	qdf_mem_free(lo_evt);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_process_noa(struct p2p_noa_event *noa_event)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct p2p_noa_info *noa_info;
	struct p2p_vdev_priv_obj *p2p_vdev_obj;
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;
	enum tQDF_ADAPTER_MODE mode;

	if (!noa_event) {
		p2p_err("invalid noa event");
		return QDF_STATUS_E_INVAL;
	}
	noa_info = noa_event->noa_info;
	p2p_soc_obj = noa_event->p2p_soc_obj;
	psoc = p2p_soc_obj->soc;

	p2p_debug("psoc:%p, index:%d, opps_ps:%d, ct_window:%d, num_desc:%d, vdev_id:%d",
		psoc, noa_info->index, noa_info->opps_ps,
		noa_info->ct_window, noa_info->num_desc,
		noa_info->vdev_id);

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
			noa_info->vdev_id, WLAN_P2P_ID);
	if (!vdev) {
		p2p_err("vdev obj is NULL");
		qdf_mem_free(noa_event->noa_info);
		return QDF_STATUS_E_INVAL;
	}

	wlan_vdev_obj_lock(vdev);
	mode = wlan_vdev_mlme_get_opmode(vdev);
	wlan_vdev_obj_unlock(vdev);
	p2p_debug("vdev mode:%d", mode);
	if (mode != QDF_P2P_GO_MODE) {
		p2p_err("invalid p2p vdev mode:%d", mode);
		status = QDF_STATUS_E_INVAL;
		goto fail;
	}

	/* must send noa to pe since of limitation*/
	p2p_send_noa_to_pe(noa_info);

	p2p_vdev_obj = wlan_objmgr_vdev_get_comp_private_obj(vdev,
			WLAN_UMAC_COMP_P2P);
	if (!(p2p_vdev_obj->noa_info)) {
		p2p_vdev_obj->noa_info =
			qdf_mem_malloc(sizeof(struct p2p_noa_info));
		if (!(p2p_vdev_obj->noa_info)) {
			p2p_err("Failed to allocate p2p noa info");
			status = QDF_STATUS_E_NOMEM;
			goto fail;
		}
	}
	qdf_mem_copy(p2p_vdev_obj->noa_info, noa_info,
		sizeof(struct p2p_noa_info));
fail:
	qdf_mem_free(noa_event->noa_info);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_P2P_ID);

	return status;
}
