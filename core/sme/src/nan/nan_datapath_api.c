/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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
 * DOC: nan_datapath_api.c
 *
 * SME NAN Data path API implementation
 */
#include "sms_debug.h"
#include "sme_api.h"
#include "sme_inside.h"
#include "csr_internal.h"
#include "sme_nan_datapath.h"

/**
 * csr_free_ndp_initiator_req() - free resouces from sme command for ndp
 * initiator request
 * @cmd: sme command msg
 *
 * Return: None
 */
static void csr_free_ndp_initiator_req(tSmeCmd *cmd)
{
	qdf_mem_free(cmd->u.initiator_req.ndp_config.ndp_cfg);
	cmd->u.initiator_req.ndp_config.ndp_cfg = NULL;
	cmd->u.initiator_req.ndp_config.ndp_cfg_len = 0;
	qdf_mem_free(cmd->u.initiator_req.ndp_info.ndp_app_info);
	cmd->u.initiator_req.ndp_info.ndp_app_info = NULL;
	cmd->u.initiator_req.ndp_info.ndp_app_info_len = 0;
	qdf_mem_free(cmd->u.initiator_req.pmk.pmk);
	cmd->u.initiator_req.pmk.pmk = NULL;
	cmd->u.initiator_req.pmk.pmk_len = 0;
}

/**
 * csr_free_ndp_responder_req() - free resouces from sme command for ndp
 * responder request
 * @cmd: sme command msg
 *
 * Return: None
 */
static void csr_free_ndp_responder_req(tSmeCmd *cmd)
{
	qdf_mem_free(cmd->u.responder_req.ndp_config.ndp_cfg);
	cmd->u.responder_req.ndp_config.ndp_cfg = NULL;
	cmd->u.responder_req.ndp_config.ndp_cfg_len = 0;
	qdf_mem_free(cmd->u.responder_req.ndp_info.ndp_app_info);
	cmd->u.responder_req.ndp_info.ndp_app_info = NULL;
	cmd->u.responder_req.ndp_info.ndp_app_info_len = 0;
	qdf_mem_free(cmd->u.responder_req.pmk.pmk);
	cmd->u.responder_req.pmk.pmk = NULL;
	cmd->u.responder_req.pmk.pmk_len = 0;
}

/**
 * sme_ndp_initiator_req_handler() - ndp initiator req handler
 * @hal: hal handle
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS sme_ndp_initiator_req_handler(tHalHandle hal,
			struct ndp_initiator_req *req_params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tSmeCmd *cmd;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	if (NULL == req_params) {
		sms_log(mac_ctx, LOGE, FL("Invalid req_params"));
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac_ctx, LOGE,
		       FL("SME lock failed, status:%d"), status);
		return status;
	}
	cmd = csr_get_command_buffer(mac_ctx);
	if (NULL == cmd) {
		sme_release_global_lock(&mac_ctx->sme);
		return QDF_STATUS_E_RESOURCES;
	}

	cmd->command = eSmeCommandNdpInitiatorRequest;
	cmd->sessionId = (uint8_t)req_params->vdev_id;
	qdf_mem_copy(&cmd->u.initiator_req, req_params,
		     sizeof(struct ndp_initiator_req));
	/* pointers copied as part of above operation are to be overwritten */
	cmd->u.initiator_req.ndp_info.ndp_app_info = NULL;
	cmd->u.initiator_req.ndp_config.ndp_cfg = NULL;
	cmd->u.initiator_req.pmk.pmk = NULL;

	if (req_params->ndp_info.ndp_app_info_len) {
		cmd->u.initiator_req.ndp_info.ndp_app_info =
			qdf_mem_malloc(req_params->ndp_info.ndp_app_info_len);
		if (NULL == cmd->u.initiator_req.ndp_info.ndp_app_info) {
			csr_release_ndp_initiator_req(mac_ctx, cmd);
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(cmd->u.initiator_req.ndp_info.ndp_app_info,
			req_params->ndp_info.ndp_app_info,
			req_params->ndp_info.ndp_app_info_len);
	}

	if (req_params->ndp_config.ndp_cfg_len) {
		cmd->u.initiator_req.ndp_config.ndp_cfg =
			qdf_mem_malloc(req_params->ndp_config.ndp_cfg_len);
		if (NULL == cmd->u.initiator_req.ndp_config.ndp_cfg) {
			csr_release_ndp_initiator_req(mac_ctx, cmd);
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(cmd->u.initiator_req.ndp_config.ndp_cfg,
			req_params->ndp_config.ndp_cfg,
			req_params->ndp_config.ndp_cfg_len);
	}

	if (req_params->pmk.pmk_len) {
		cmd->u.initiator_req.pmk.pmk =
			qdf_mem_malloc(req_params->pmk.pmk_len);
		if (NULL == cmd->u.initiator_req.pmk.pmk) {
			csr_release_ndp_initiator_req(mac_ctx, cmd);
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(cmd->u.initiator_req.pmk.pmk,
			     req_params->pmk.pmk, req_params->pmk.pmk_len);
	}

	status = csr_queue_sme_command(mac_ctx, cmd, true);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac_ctx, LOGE, FL("SME enqueue failed, status:%d"),
			status);
		csr_release_ndp_initiator_req(mac_ctx, cmd);
	}

	sme_release_global_lock(&mac_ctx->sme);
	return status;
}
/**
 * sme_ndp_responder_req_handler() - ndp responder request handler
 * @hal: hal handle
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS sme_ndp_responder_req_handler(tHalHandle hal,
	struct ndp_responder_req *req_params)
{
	QDF_STATUS status;
	tSmeCmd *cmd;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	if (NULL == req_params) {
		sms_log(mac_ctx, LOGE, FL("Invalid req_params"));
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac_ctx, LOGE,
			FL("SME lock failed, status:%d"), status);
		return status;
	}
	cmd = csr_get_command_buffer(mac_ctx);
	if (NULL == cmd) {
		sme_release_global_lock(&mac_ctx->sme);
		return QDF_STATUS_E_RESOURCES;
	}

	cmd->command = eSmeCommandNdpResponderRequest;
	cmd->sessionId = (uint8_t)req_params->vdev_id;
	qdf_mem_copy(&cmd->u.responder_req, req_params,
			sizeof(*req_params));

	/*
	 * Pointers copied as part of above operation are
	 * to be overwritten
	 */
	cmd->u.responder_req.ndp_info.ndp_app_info = NULL;
	cmd->u.responder_req.ndp_config.ndp_cfg = NULL;
	cmd->u.responder_req.pmk.pmk = NULL;

	if (req_params->ndp_info.ndp_app_info_len) {
		cmd->u.responder_req.ndp_info.ndp_app_info =
			qdf_mem_malloc(req_params->ndp_info.ndp_app_info_len);
		if (NULL == cmd->u.responder_req.ndp_info.ndp_app_info) {
			csr_release_ndp_responder_req(mac_ctx, cmd);
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(cmd->u.responder_req.ndp_info.ndp_app_info,
			req_params->ndp_info.ndp_app_info,
			req_params->ndp_info.ndp_app_info_len);
	}

	if (req_params->ndp_config.ndp_cfg_len) {
		cmd->u.responder_req.ndp_config.ndp_cfg =
			qdf_mem_malloc(req_params->ndp_config.ndp_cfg_len);
		if (NULL == cmd->u.responder_req.ndp_config.ndp_cfg) {
			csr_release_ndp_responder_req(mac_ctx, cmd);
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(cmd->u.responder_req.ndp_config.ndp_cfg,
			req_params->ndp_config.ndp_cfg,
			req_params->ndp_config.ndp_cfg_len);
	}

	if (req_params->pmk.pmk_len) {
		cmd->u.responder_req.pmk.pmk =
			qdf_mem_malloc(req_params->pmk.pmk_len);
		if (NULL == cmd->u.responder_req.pmk.pmk) {
			csr_release_ndp_responder_req(mac_ctx, cmd);
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_NOMEM;
		}
		qdf_mem_copy(cmd->u.responder_req.pmk.pmk,
			     req_params->pmk.pmk, req_params->pmk.pmk_len);
	}

	status = csr_queue_sme_command(mac_ctx, cmd, true);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac_ctx, LOGE,
			FL("SME enqueue failed, status:%d"), status);
		csr_release_ndp_responder_req(mac_ctx, cmd);
	}

	sme_release_global_lock(&mac_ctx->sme);
	return status;
}

/**
 * sme_ndp_end_req_handler() - ndp end request handler
 * @hal: hal handle
 * @req: ndp end request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS sme_ndp_end_req_handler(tHalHandle hal, struct ndp_end_req *req)
{
	tSmeCmd *cmd;
	QDF_STATUS ret = QDF_STATUS_SUCCESS;
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac_ctx = PMAC_STRUCT(hal);

	if (NULL == req) {
		sms_log(mac_ctx, LOGE, FL("Invalid ndp end req"));
		return QDF_STATUS_E_INVAL;
	}

	status = sme_acquire_global_lock(&mac_ctx->sme);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac_ctx, LOGE,
		       FL("SME lock failed, status:%d"), status);
		return QDF_STATUS_E_RESOURCES;
	}
	cmd = csr_get_command_buffer(mac_ctx);
	if (NULL == cmd) {
		sme_release_global_lock(&mac_ctx->sme);
		return QDF_STATUS_E_RESOURCES;
	}

	cmd->command = eSmeCommandNdpDataEndInitiatorRequest;
	cmd->u.data_end_req = qdf_mem_malloc(sizeof(*req) +
				(req->num_ndp_instances * sizeof(uint32_t)));
	if (NULL == cmd->u.data_end_req) {
			csr_release_command_roam(mac_ctx, cmd);
			sme_release_global_lock(&mac_ctx->sme);
			return QDF_STATUS_E_NOMEM;
	}

	qdf_mem_copy(cmd->u.data_end_req, req, sizeof(*req));
	cmd->u.data_end_req->ndp_ids =
		(uint32_t *)((uint8_t *)&cmd->u.data_end_req[1]);
	qdf_mem_copy(cmd->u.data_end_req->ndp_ids, req->ndp_ids,
		 sizeof(uint32_t) * req->num_ndp_instances);

	status = csr_queue_sme_command(mac_ctx, cmd, true);
	if (QDF_STATUS_SUCCESS != status) {
		sms_log(mac_ctx, LOGE, FL("SME enqueue failed, status:%d"),
			status);
		ret = QDF_STATUS_E_FAILURE;
		csr_release_ndp_data_end_req(mac_ctx, cmd);
	}

	sme_release_global_lock(&mac_ctx->sme);
	return ret;
}

/**
 * csr_roam_start_ndi() - Start connection for NAN datapath
 * @mac_ctx: Global MAC context
 * @session: SME session ID
 * @profile: Configuration profile
 *
 * Return: Success or failure code
 */
QDF_STATUS csr_roam_start_ndi(tpAniSirGlobal mac_ctx, uint32_t session,
			tCsrRoamProfile *profile)
{
	QDF_STATUS status;
	tBssConfigParam bss_cfg = {0};

	/* Build BSS configuration from profile */
	status = csr_roam_prepare_bss_config_from_profile(mac_ctx, profile,
						    &bss_cfg, NULL);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac_ctx->roam.roamSession[session].bssParams.uCfgDot11Mode
			= bss_cfg.uCfgDot11Mode;
		/* Copy profile parameters to PE session */
		csr_roam_prepare_bss_params(mac_ctx, session, profile, NULL,
			&bss_cfg, NULL);
		/*
		 * Following routine will eventually call
		 * csrRoamIssueStartBss through csrRoamCcmCfgSetCallback
		 */
		status = csr_roam_set_bss_config_cfg(mac_ctx, session, profile,
						NULL, &bss_cfg, NULL, false);
	}

	if (QDF_IS_STATUS_SUCCESS(status))
		sms_log(mac_ctx, LOG1, FL("Profile config is valid"));
	else
		sms_log(mac_ctx, LOGE,
			FL("Profile config is invalid. status = 0x%x"), status);

	return status;
}

/**
 * csr_roam_save_ndi_connected_info() - Save connected profile parameters
 * @mac_ctx: Global MAC context
 * @session_id: Session ID
 * @roam_profile: Profile given for starting BSS
 * @bssdesc: BSS description from tSirSmeStartBssRsp response
 *
 * Saves NDI profile parameters into session's connected profile.
 *
 * Return: None
 */
void csr_roam_save_ndi_connected_info(tpAniSirGlobal mac_ctx,
				      uint32_t session_id,
				      tCsrRoamProfile *roam_profile,
				      tSirBssDescription *bssdesc)
{
	tCsrRoamSession *roam_session = NULL;
	tCsrRoamConnectedProfile *connect_profile = NULL;

	roam_session = CSR_GET_SESSION(mac_ctx, session_id);
	if (NULL == roam_session) {
		QDF_TRACE(QDF_MODULE_ID_SME, QDF_TRACE_LEVEL_ERROR,
			FL("session %d not found"), session_id);
		return;
	}

	connect_profile = &roam_session->connectedProfile;
	qdf_mem_set(&roam_session->connectedProfile,
		sizeof(connect_profile), 0);
	connect_profile->AuthType = roam_profile->negotiatedAuthType;
		connect_profile->AuthInfo = roam_profile->AuthType;
	connect_profile->CBMode = roam_profile->CBMode;
	connect_profile->EncryptionType =
		roam_profile->negotiatedUCEncryptionType;
		connect_profile->EncryptionInfo = roam_profile->EncryptionType;
	connect_profile->mcEncryptionType =
		roam_profile->negotiatedMCEncryptionType;
		connect_profile->mcEncryptionInfo =
			roam_profile->mcEncryptionType;
	connect_profile->BSSType = roam_profile->BSSType;
	connect_profile->modifyProfileFields.uapsd_mask =
		roam_profile->uapsd_mask;
	connect_profile->operationChannel = bssdesc->channelId;
	connect_profile->beaconInterval = 0;
	qdf_mem_copy(&connect_profile->Keys, &roam_profile->Keys,
		sizeof(roam_profile->Keys));
	csr_get_bss_id_bss_desc(mac_ctx, bssdesc, &connect_profile->bssid);
	connect_profile->SSID.length = 0;
	csr_free_connect_bss_desc(mac_ctx, session_id);
	connect_profile->qap = false;
	connect_profile->qosConnection = false;
}

/**
 * csr_roam_update_ndp_return_params() - updates ndp return parameters
 * @mac_ctx: MAC context handle
 * @result: result of the roaming command
 * @roam_status: roam status returned to the roam command initiator
 * @roam_result: roam result returned to the roam command initiator
 * @roam_info: Roam info data structure to be updated
 *
 * Results: None
 */
void csr_roam_update_ndp_return_params(tpAniSirGlobal mac_ctx,
					uint32_t result,
					uint32_t *roam_status,
					uint32_t *roam_result,
					tCsrRoamInfo *roam_info)
{

	switch (result) {
	case eCsrStartBssSuccess:
		roam_info->ndp.ndi_create_params.reason = 0;
		roam_info->ndp.ndi_create_params.status =
					NDP_RSP_STATUS_SUCCESS;
		roam_info->ndp.ndi_create_params.sta_id = roam_info->staId;
		*roam_status = eCSR_ROAM_NDP_STATUS_UPDATE;
		*roam_result = eCSR_ROAM_RESULT_NDI_CREATE_RSP;
		break;
	case eCsrStartBssFailure:
		roam_info->ndp.ndi_create_params.status = NDP_RSP_STATUS_ERROR;
		roam_info->ndp.ndi_create_params.reason =
					NDP_NAN_DATA_IFACE_CREATE_FAILED;
		*roam_status = eCSR_ROAM_NDP_STATUS_UPDATE;
		*roam_result = eCSR_ROAM_RESULT_NDI_CREATE_RSP;
		break;
	case eCsrStopBssSuccess:
		roam_info->ndp.ndi_delete_params.reason = 0;
		roam_info->ndp.ndi_delete_params.status =
						NDP_RSP_STATUS_SUCCESS;
		*roam_status = eCSR_ROAM_NDP_STATUS_UPDATE;
		*roam_result = eCSR_ROAM_RESULT_NDI_DELETE_RSP;
		break;
	case eCsrStopBssFailure:
		roam_info->ndp.ndi_delete_params.status = NDP_RSP_STATUS_ERROR;
		roam_info->ndp.ndi_delete_params.reason =
					NDP_NAN_DATA_IFACE_DELETE_FAILED;
		*roam_status = eCSR_ROAM_NDP_STATUS_UPDATE;
		*roam_result = eCSR_ROAM_RESULT_NDI_DELETE_RSP;
		break;
	default:
		sms_log(mac_ctx, LOGE,
			FL("Invalid CSR Roam result code: %d"), result);
		break;
	}
}

/**
 * csr_process_ndp_initiator_request() - process ndp initiator request
 * @mac_ctx: Global MAC context
 * @cmd: ndp initiator sme cmd
 *
 * Return: status of operation
 */
QDF_STATUS csr_process_ndp_initiator_request(tpAniSirGlobal mac_ctx,
					     tSmeCmd *cmd)
{
	QDF_STATUS status;
	struct sir_sme_ndp_initiator_req *lim_msg;
	uint16_t msg_len;
	uint8_t *self_mac_addr = NULL;
	struct ndp_initiator_req *req;

	if (NULL == cmd) {
		sms_log(mac_ctx, LOGE, FL("Invalid req_params"));
		return QDF_STATUS_E_INVAL;
	}
	req = &cmd->u.initiator_req;

	msg_len = sizeof(*lim_msg);
	lim_msg = qdf_mem_malloc(msg_len);
	if (NULL == lim_msg) {
		status = QDF_STATUS_E_NOMEM;
		goto sme_initiator_req_failed;
	}

	lim_msg->msg_type = eWNI_SME_NDP_INITIATOR_REQ;
	lim_msg->msg_len = msg_len;
	/*
	 * following is being copied from p_cmd->u.initiator_req,
	 * no need to perform deep copy, as we are going to use memory
	 * allocated at SME in p_cmd->u.initiator_req and pass it all the way
	 * to WMA.
	 */
	qdf_mem_copy(&lim_msg->req, req, sizeof(struct ndp_initiator_req));

	self_mac_addr = lim_msg->req.self_ndi_mac_addr.bytes;
	sms_log(mac_ctx, LOG1, FL("selfMac = "MAC_ADDRESS_STR),
		MAC_ADDR_ARRAY(self_mac_addr));

	status = cds_send_mb_message_to_mac(lim_msg);
	if (status != QDF_STATUS_SUCCESS)
		csr_free_ndp_initiator_req(cmd);

sme_initiator_req_failed:
	/* If fail, free up resources allocated in sme. */
	if (status != QDF_STATUS_SUCCESS)
		csr_free_ndp_initiator_req(cmd);
	return status;
}

/**
 * csr_process_ndp_responder_request() - ndp responder req
 * @mac_ctx: Global MAC context
 * @cmd: Cmd sent to SME
 *
 * Return: Success or failure code
 */
QDF_STATUS csr_process_ndp_responder_request(tpAniSirGlobal mac_ctx,
							tSmeCmd *cmd)
{
	struct sir_sme_ndp_responder_req *lim_msg;
	uint16_t msg_len;
	QDF_STATUS status;

	if (!cmd) {
		sms_log(mac_ctx, LOGE, FL("Invalid req_params"));
		return QDF_STATUS_E_INVAL;
	}

	msg_len  = sizeof(*lim_msg);
	lim_msg = qdf_mem_malloc(msg_len);

	if (!lim_msg) {
		sms_log(mac_ctx, LOGE, FL("Mem alloc fail"));
		status = QDF_STATUS_E_NOMEM;
		goto free_config;
	}

	lim_msg->msg_type = eWNI_SME_NDP_RESPONDER_REQ;
	lim_msg->msg_len = msg_len;
	/*
	 * following is being copied from p_cmd->u.responder_req,
	 * no need to perform deep copy, as we are going to use memory
	 * allocated at SME in p_cmd->u.responder_req and pass it all the way
	 * to WMA.
	 */
	qdf_mem_copy(&lim_msg->req, &cmd->u.responder_req,
			sizeof(struct ndp_responder_req));

	sms_log(mac_ctx, LOG1,
		FL("vdev_id %d ndp_rsp = %d Instance id %d"),
		lim_msg->req.vdev_id,
		lim_msg->req.ndp_rsp,
		lim_msg->req.ndp_instance_id);

	status = cds_send_mb_message_to_mac(lim_msg);

free_config:
	/* If fail, free up the ndp_cfg and ndp_app_info allocated in sme. */
	if (status != QDF_STATUS_SUCCESS)
		csr_free_ndp_responder_req(cmd);
	return status;
}

/*
 * csr_process_ndp_data_end_request() - process ndp data end request
 * @mac_ctx: Global MAC context
 * @cmd: sme command containing ndp initiator request
 *
 * Return: status of operation
 */
QDF_STATUS csr_process_ndp_data_end_request(tpAniSirGlobal mac_ctx,
					    tSmeCmd *cmd)
{
	QDF_STATUS status;
	struct sir_sme_ndp_end_req *lim_msg;
	uint16_t msg_len;

	if (NULL == cmd) {
		sms_log(mac_ctx, LOGE, FL("NULL sme cmd"));
		return QDF_STATUS_E_INVAL;
	}

	msg_len  = sizeof(*lim_msg);
	lim_msg = qdf_mem_malloc(msg_len);
	if (NULL == lim_msg) {
		sms_log(mac_ctx, LOGE, FL("Malloc failed"));
		qdf_mem_free(cmd->u.data_end_req);
		cmd->u.data_end_req = NULL;
		return QDF_STATUS_E_NOMEM;
	}

	lim_msg->msg_type = (uint16_t)eWNI_SME_NDP_END_REQ;
	lim_msg->msg_len = msg_len;
	lim_msg->req = cmd->u.data_end_req;

	status = cds_send_mb_message_to_mac(lim_msg);
	if (status != QDF_STATUS_SUCCESS) {
		qdf_mem_free(cmd->u.data_end_req);
		cmd->u.data_end_req = NULL;
	}
	return status;
}

/**
 * sme_ndp_msg_processor() - message processor for ndp/ndi north-bound SME msg.
 * @mac_ctx: Global MAC context
 * @msg: ndp/ndi SME message
 *
 * This function will further call csrRoamCallCallback with appropriate
 * roam_status and roam_result thus allowing hdd to correctly identify NDP/NDI
 * response.
 *
 * Return: nothing
 */
void sme_ndp_msg_processor(tpAniSirGlobal mac_ctx, cds_msg_t *msg)
{
	tCsrRoamInfo roam_info = {0};
	eCsrRoamResult result;
	uint32_t session_id;
	tListElem *entry = NULL;
	tSmeCmd *cmd = NULL;
	bool release_active_cmd = false;
	eSmeCommandType cmd_to_rel = eSmeNoCommand;
	bool send_to_user = true;

	entry = csr_ll_peek_head(&mac_ctx->sme.smeCmdActiveList,
				 LL_ACCESS_LOCK);
	if (entry != NULL)
		cmd = GET_BASE_ADDR(entry, tSmeCmd, Link);

	switch (msg->type) {
	case eWNI_SME_NDP_CONFIRM_IND: {
		result = eCSR_ROAM_RESULT_NDP_CONFIRM_IND;
		/* copy msg from msg body to roam info passed to callback */
		qdf_mem_copy(&roam_info.ndp.ndp_confirm_params, msg->bodyptr,
			sizeof(roam_info.ndp.ndp_confirm_params));
		session_id = roam_info.ndp.ndp_confirm_params.vdev_id;
		break;
	}
	case eWNI_SME_NDP_INITIATOR_RSP: {
		if (true == msg->bodyval) {
			/* rsp was locally generated, do not send to HDD */
			send_to_user = false;
		} else {
			result = eCSR_ROAM_RESULT_NDP_INITIATOR_RSP;
			qdf_mem_copy(&roam_info.ndp.ndp_init_rsp_params,
				     msg->bodyptr,
				     sizeof(roam_info.ndp.ndp_init_rsp_params));
			session_id = roam_info.ndp.ndp_init_rsp_params.vdev_id;
		}
		release_active_cmd = true;
		cmd_to_rel = eSmeCommandNdpInitiatorRequest;
		break;
	}
	case eWNI_SME_NDP_NEW_PEER_IND: {
		result = eCSR_ROAM_RESULT_NDP_NEW_PEER_IND;
		/* copy msg from msg body to roam info passed to callback */
		qdf_mem_copy(&roam_info.ndp.ndp_peer_ind_params,
			     msg->bodyptr,
			     sizeof(roam_info.ndp.ndp_peer_ind_params));
		session_id = roam_info.ndp.ndp_peer_ind_params.session_id;
		break;
	}
	case eWNI_SME_NDP_INDICATION:
		result = eCSR_ROAM_RESULT_NDP_INDICATION;
		/* copy msg from msg body to roam info passed to callback */
		qdf_mem_copy(&roam_info.ndp.ndp_indication_params,
			msg->bodyptr, sizeof(struct ndp_indication_event));
		session_id = roam_info.ndp.ndp_indication_params.vdev_id;
		break;
	case eWNI_SME_NDP_RESPONDER_RSP:
		if (true == msg->bodyval) {
			/* rsp was locally generated, do not send to HDD */
			send_to_user = false;
		} else {
			result = eCSR_ROAM_RESULT_NDP_RESPONDER_RSP;
			/*
			 * Copy msg from msg body to roam info passed to
			 * callback
			 */
			qdf_mem_copy(&roam_info.ndp.ndp_responder_rsp_params,
				msg->bodyptr,
				sizeof(struct ndp_responder_rsp_event));
			session_id =
				roam_info.ndp.ndp_responder_rsp_params.vdev_id;
		}
		release_active_cmd = true;
		cmd_to_rel = eSmeCommandNdpResponderRequest;
		break;
	case eWNI_SME_NDP_END_RSP: {
		if (true == msg->bodyval) {
			/* rsp was locally generated, do not send to HDD */
			send_to_user = false;
		} else {
			result = eCSR_ROAM_RESULT_NDP_END_RSP;
			roam_info.ndp.ndp_end_rsp_params = msg->bodyptr;
			/*
			 * NDP_END_IND is independent of session, but session_id
			 * is needed for csrRoamCallCallback(). Set it to 0
			 * which is a valid session.
			 */
			session_id = 0;
		}
		release_active_cmd = true;
		cmd_to_rel = eSmeCommandNdpDataEndInitiatorRequest;
		break;
	}
	case eWNI_SME_NDP_END_IND:
		result = eCSR_ROAM_RESULT_NDP_END_IND;
		roam_info.ndp.ndp_end_ind_params = msg->bodyptr;
		/*
		 * NDP_END_IND is independent of session, but session_id is
		 * needed for csr_roam_call_callback(). Set it to vdev_id of
		 * first entry which is a valid session. vdev_id is likely to
		 * be same for all.
		 */
		session_id =
			roam_info.ndp.ndp_end_ind_params->ndp_map[0].vdev_id;
		break;
	case eWNI_SME_NDP_PEER_DEPARTED_IND:
		result = eCSR_ROAM_RESULT_NDP_PEER_DEPARTED_IND;
		/* copy msg from msg body to roam info passed to callback */
		qdf_mem_copy(&roam_info.ndp.ndp_peer_ind_params,
			msg->bodyptr,
			sizeof(roam_info.ndp.ndp_peer_ind_params));
		session_id =
			((struct sme_ndp_peer_ind *)msg->bodyptr)->session_id;
		break;
	default:
		sms_log(mac_ctx, LOGE, FL("Unhandled NDP rsp"));
		qdf_mem_free(msg->bodyptr);
		return;
	}

	if (true == send_to_user)
		csr_roam_call_callback(mac_ctx, session_id, &roam_info, 0,
				    eCSR_ROAM_NDP_STATUS_UPDATE, result);

	/*
	 * free ndp_cfg and ndp_app_info if required
	 * For some commands this info may be needed in HDD
	 * so free them after roam callback.
	 */
	switch (msg->type) {
	case eWNI_SME_NDP_INITIATOR_RSP:
		if (cmd && eSmeCommandNdpInitiatorRequest == cmd->command) {
			qdf_mem_free(cmd->u.initiator_req.ndp_config.ndp_cfg);
			qdf_mem_free(
				cmd->u.initiator_req.ndp_info.ndp_app_info);
		}
		break;
	case eWNI_SME_NDP_INDICATION:
		qdf_mem_free(roam_info.ndp.ndp_indication_params.scid.scid);
		qdf_mem_free(
		    roam_info.ndp.ndp_indication_params.ndp_config.ndp_cfg);
		qdf_mem_free(
		    roam_info.ndp.ndp_indication_params.ndp_info.ndp_app_info);
		break;
	case eWNI_SME_NDP_END_RSP:
		if (cmd &&
			eSmeCommandNdpDataEndInitiatorRequest == cmd->command) {
			qdf_mem_free(cmd->u.data_end_req);
			cmd->u.data_end_req = NULL;
		}
		break;
	case eWNI_SME_NDP_END_IND:
		break;
	default:
		break;
	}
	qdf_mem_free(msg->bodyptr);
	msg->bodyptr = NULL;

	if (release_active_cmd && cmd && cmd_to_rel == cmd->command) {
		/* Now put this cmd back on the avilable command list */
		if (csr_ll_remove_entry(&mac_ctx->sme.smeCmdActiveList,
				     entry, LL_ACCESS_LOCK))
			sme_release_command(mac_ctx, cmd);
		sme_process_pending_queue(mac_ctx);
	}
}

/**
 * csr_release_ndp_initiator_req() - free resouces from sme command for ndp
 * and release the cmd
 * initiator request
 * @mac_ctx: Global MAC context
 * @cmd: sme command msg
 *
 * Return: None
 */
void csr_release_ndp_initiator_req(tpAniSirGlobal mac_ctx, tSmeCmd *cmd)
{
	csr_free_ndp_initiator_req(cmd);
	sme_release_command(mac_ctx, cmd);
}

/**
 * csr_release_ndp_responder_req() - free resouces from sme command for ndp
 * responder request and release the command
 * @mac_ctx: Global MAC context
 * @cmd: sme command msg
 *
 * Return: None
 */
void csr_release_ndp_responder_req(tpAniSirGlobal mac_ctx, tSmeCmd *cmd)
{
	csr_free_ndp_responder_req(cmd);
	sme_release_command(mac_ctx, cmd);
}

/**
 * csr_release_ndp_data_end_req() - free resouces from sme command for ndp
 * data end request
 * @mac_ctx: Global MAC context
 * @cmd: sme command msg
 *
 * Return: None
 */
void csr_release_ndp_data_end_req(tpAniSirGlobal mac_ctx, tSmeCmd *cmd)
{
	qdf_mem_free(cmd->u.data_end_req);
	cmd->u.data_end_req = NULL;
	sme_release_command(mac_ctx, cmd);
}
