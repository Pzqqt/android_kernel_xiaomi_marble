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
#include "wlan_p2p_public_struct.h"
#include "wlan_p2p_ucfg_api.h"
#include "wlan_p2p_tgt_api.h"
#include "../inc/wlan_p2p_main.h"
#include "../inc/wlan_p2p_roc.h"
#include "../inc/wlan_p2p_off_chan_tx.h"

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

	if (soc == NULL) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = qdf_mem_malloc(sizeof(*p2p_soc_obj));
	if (p2p_soc_obj == NULL) {
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

	if (soc == NULL) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (p2p_soc_obj == NULL) {
		p2p_err("p2p soc private object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_psoc_component_obj_detach(soc,
				WLAN_UMAC_COMP_P2P, p2p_soc_obj);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to detach p2p component, %d", status);
		return status;
	}
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

	if (vdev == NULL) {
		p2p_err("vdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_vdev_obj = qdf_mem_malloc(sizeof(*p2p_vdev_obj));
	if (p2p_vdev_obj == NULL) {
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

	if (vdev == NULL) {
		p2p_err("vdev context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_vdev_obj = wlan_objmgr_vdev_get_comp_private_obj(vdev,
			WLAN_UMAC_COMP_P2P);
	if (p2p_vdev_obj == NULL) {
		p2p_err("p2p vdev object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_vdev_component_obj_detach(vdev,
				WLAN_UMAC_COMP_P2P, p2p_vdev_obj);
	if (status != QDF_STATUS_SUCCESS) {
		p2p_err("Failed to detach p2p component, %d", status);
		return status;
	}

	qdf_mem_free(p2p_vdev_obj);

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

	status = wlan_objmgr_register_psoc_delete_handler(
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

	status = wlan_objmgr_register_vdev_delete_handler(
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
	wlan_objmgr_unregister_psoc_delete_handler(WLAN_UMAC_COMP_P2P,
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

	status = wlan_objmgr_unregister_vdev_delete_handler(
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

	status = wlan_objmgr_unregister_psoc_delete_handler(
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

QDF_STATUS p2p_psoc_open(struct wlan_objmgr_psoc *soc)
{
	struct p2p_soc_priv_obj *p2p_soc_obj;

	if (soc == NULL) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (p2p_soc_obj == NULL) {
		p2p_err("p2p soc priviate object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_list_create(&p2p_soc_obj->roc_q, MAX_QUEUE_LENGTH);
	qdf_list_create(&p2p_soc_obj->tx_q_roc, MAX_QUEUE_LENGTH);
	qdf_list_create(&p2p_soc_obj->tx_q_ack, MAX_QUEUE_LENGTH);
	qdf_event_create(&p2p_soc_obj->cancel_roc_done);
	/*TODO, register scan event, wmi lo and noa event */

	p2p_debug("p2p psoc object open successful");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_psoc_close(struct wlan_objmgr_psoc *soc)
{
	struct p2p_soc_priv_obj *p2p_soc_obj;

	if (soc == NULL) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (p2p_soc_obj == NULL) {
		p2p_err("p2p soc object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_list_destroy(&p2p_soc_obj->roc_q);
	qdf_list_destroy(&p2p_soc_obj->tx_q_roc);
	qdf_list_destroy(&p2p_soc_obj->tx_q_ack);
	/*TODO, ucfg_scan_clear_requestor_id(soc, p2p_soc_obj->scan_req_id);*/

	p2p_debug("p2p psoc object close successful");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_psoc_start(struct wlan_objmgr_psoc *soc,
	struct p2p_start_param *req)
{
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct p2p_start_param *start_param;

	if (soc == NULL) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (p2p_soc_obj == NULL) {
		p2p_err("P2P soc object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	start_param = qdf_mem_malloc(sizeof(*start_param));
	if (start_param == NULL) {
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

	p2p_debug("p2p psoc start successful");

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_psoc_stop(struct wlan_objmgr_psoc *soc)
{
	struct p2p_soc_priv_obj *p2p_soc_obj;
	struct p2p_start_param *start_param;

	if (soc == NULL) {
		p2p_err("psoc context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}

	p2p_soc_obj = wlan_objmgr_psoc_get_comp_private_obj(soc,
			WLAN_UMAC_COMP_P2P);
	if (p2p_soc_obj == NULL) {
		p2p_err("P2P soc object is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	start_param = p2p_soc_obj->start_param;
	p2p_soc_obj->start_param = NULL;
	if (start_param == NULL) {
		p2p_err("start parameters is NULL");
		return QDF_STATUS_E_FAILURE;
	}

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

	p2p_info("msg type %d", msg->type);
	if (msg->bodyptr == NULL) {
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
		break;
	default:
		p2p_err("drop unexpected message received %d",
			msg->type);
		status = QDF_STATUS_E_INVAL;
		break;
	}

	qdf_mem_free(msg->bodyptr);
	msg->bodyptr = NULL;

	return status;
}

QDF_STATUS p2p_process_evt(struct scheduler_msg *msg)
{
	QDF_STATUS status;

	p2p_info("msg type %d", msg->type);
	if (msg->bodyptr == NULL) {
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
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS p2p_process_noa(struct p2p_noa_event *noa_event)
{
	return QDF_STATUS_SUCCESS;
}
