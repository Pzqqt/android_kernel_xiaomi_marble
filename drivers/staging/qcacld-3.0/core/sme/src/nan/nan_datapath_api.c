/*
 * Copyright (c) 2016-2019, 2021 The Linux Foundation. All rights reserved.
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
#include <sir_common.h>
#include <ani_global.h>
#include "sme_api.h"
#include "sme_inside.h"
#include "csr_internal.h"
#include "sme_nan_datapath.h"

/**
 * csr_roam_start_ndi() - Start connection for NAN datapath
 * @mac_ctx: Global MAC context
 * @session: SME session ID
 * @profile: Configuration profile
 *
 * Return: Success or failure code
 */
QDF_STATUS csr_roam_start_ndi(struct mac_context *mac_ctx, uint32_t session,
			struct csr_roam_profile *profile)
{
	QDF_STATUS status;
	struct bss_config_param bss_cfg;

	qdf_mem_zero(&bss_cfg, sizeof(struct bss_config_param));
	/* Build BSS configuration from profile */
	status = csr_roam_prepare_bss_config_from_profile(mac_ctx, profile,
							  session,
							  &bss_cfg);
	if (QDF_IS_STATUS_SUCCESS(status)) {
		mac_ctx->roam.roamSession[session].bssParams.uCfgDot11Mode
			= bss_cfg.uCfgDot11Mode;
		/* Copy profile parameters to PE session */
		csr_roam_prepare_bss_params(mac_ctx, session, profile,
					    &bss_cfg);
		/*
		 * Following routine will eventually call
		 * csrRoamIssueStartBss through csrRoamCcmCfgSetCallback
		 */
		status = csr_roam_set_bss_config_cfg(mac_ctx, session, profile,
						     &bss_cfg);
	}

	sme_debug("profile config validity: %d", status);

	return status;
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
void csr_roam_update_ndp_return_params(struct mac_context *mac_ctx,
					uint32_t result,
					uint32_t *roam_status,
					uint32_t *roam_result,
					struct csr_roam_info *roam_info)
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
		sme_err("Invalid CSR Roam result code: %d", result);
		break;
	}
}
