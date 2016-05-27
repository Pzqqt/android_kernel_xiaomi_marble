/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
#include "csr_inside_api.h"
#include "sme_inside.h"
#include "csr_internal.h"
#include "sme_nan_datapath.h"

/**
 * sme_ndp_initiator_req_handler() - ndp initiator req handler
 * @session_id: session id over which the ndp is being created
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS sme_ndp_initiator_req_handler(uint32_t session_id,
	struct ndp_initiator_req *req_params)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_ndp_responder_req_handler() - ndp responder request handler
 * @session_id: session id over which the ndp is being created
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS sme_ndp_responder_req_handler(uint32_t session_id,
	struct ndp_responder_req *req_params)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * sme_ndp_end_req_handler() - ndp end request handler
 * @session_id: session id over which the ndp is being created
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS sme_ndp_end_req_handler(uint32_t session_id,
	struct ndp_end_req *req_params)
{
	return QDF_STATUS_SUCCESS;
}


/**
 * sme_ndp_sched_req_handler() - ndp schedule request handler
 * @session_id: session id over which the ndp is being created
 * @req_params: request parameters
 *
 * Return: QDF_STATUS_SUCCESS on success; error number otherwise
 */
QDF_STATUS sme_ndp_sched_req_handler(uint32_t session_id,
	struct ndp_schedule_update_req *req_params)
{
	return QDF_STATUS_SUCCESS;
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
 * csr_roam_fill_roaminfo_ndp() - fill the ndi create struct inside roam info
 * @mac_ctx: Global MAC context
 * @roam_info: roam info to be updated with ndi create params
 * @roam_result: roam result to update
 * @status_code: status code to update
 * @reason_code: reason code to update
 * @transaction_id: transcation id to update
 *
 * Return: None
 */
void csr_roam_fill_roaminfo_ndp(tpAniSirGlobal mac_ctx,
		tCsrRoamInfo *roam_info,
		eCsrRoamResult roam_result,
		tSirResultCodes status_code,
		uint32_t reason_code,
		uint32_t transaction_id)
{
	struct ndi_create_rsp *rsp_params;

	sms_log(mac_ctx, LOG1,
		FL("reason 0x%x, status 0x%x, transaction_id %d"),
		reason_code, status_code, transaction_id);
	rsp_params = (struct ndi_create_rsp *)
			&roam_info->ndp.ndi_create_params;
	rsp_params->reason = reason_code;
	rsp_params->status = status_code;
	rsp_params->transaction_id = transaction_id;
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
 *
 * Results: None
 */
void csr_roam_update_ndp_return_params(tpAniSirGlobal mac_ctx,
					uint32_t result,
					uint32_t *roam_status,
					uint32_t *roam_result)
{
	switch (result) {
	case eCsrStartBssSuccess:
	case eCsrStartBssFailure:
		*roam_status = eCSR_ROAM_NDP_STATUS_UPDATE;
		*roam_result = eCSR_ROAM_RESULT_NDP_CREATE_RSP;
		break;
	default:
		sms_log(mac_ctx, LOGE,
			FL("Invalid CSR Roam result code: %d"), result);
		break;
	}
}
