/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
 *
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

#include <wlan_spectral_ucfg_api.h>
#include "../../core/spectral_cmn_api_i.h"
#include <wlan_spectral_utils_api.h>
#include <qdf_module.h>
#include <cfg_ucfg_api.h>

QDF_STATUS
ucfg_spectral_control(struct wlan_objmgr_pdev *pdev,
		      struct spectral_cp_request *sscan_req)
{
	struct spectral_context *sc;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		return -EPERM;
	}

	if (wlan_spectral_is_feature_disabled(wlan_pdev_get_psoc(pdev))) {
		spectral_info("Spectral is disabled");
		return -EPERM;
	}

	sc = spectral_get_spectral_ctx_from_pdev(pdev);
	if (!sc) {
		spectral_err("spectral context is NULL!");
		return -EPERM;
	}

	return sc->sptrlc_spectral_control(pdev, sscan_req);
}
qdf_export_symbol(ucfg_spectral_control);

void ucfg_spectral_scan_set_ppid(struct wlan_objmgr_pdev *pdev, uint32_t ppid)
{
	struct pdev_spectral *ps = NULL;

	if (!pdev) {
		spectral_err("PDEV is NULL!");
		return;
	}
	ps = wlan_objmgr_pdev_get_comp_private_obj(pdev,
						   WLAN_UMAC_COMP_SPECTRAL);
	if (!ps) {
		spectral_err("spectral context is NULL!");
		return;
	}
	ps->spectral_pid = ppid;
	spectral_debug("spectral ppid: %d", ppid);

	return;
}

QDF_STATUS ucfg_spectral_create_cp_req(struct spectral_cp_request *sscan_req,
				       void *indata, u_int32_t insize)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	switch (sscan_req->req_id) {
	case SPECTRAL_SET_CONFIG:
		{
			if (insize < sizeof(struct spectral_config) ||
			    !indata) {
				status = QDF_STATUS_E_FAILURE;
				goto bad;
			}
			qdf_mem_copy(&sscan_req->config_req.sscan_config,
				     indata,
				     sizeof(struct spectral_config));
		}
		break;

	case SPECTRAL_SET_DEBUG_LEVEL:
		{
			if (insize < sizeof(uint32_t) || !indata) {
				status = QDF_STATUS_E_FAILURE;
				goto bad;
			}
			sscan_req->debug_req.spectral_dbg_level =
							*(uint32_t *)indata;
		}
		break;

	default:
		break;
	}

bad:
	return status;
}

qdf_export_symbol(ucfg_spectral_create_cp_req);

QDF_STATUS ucfg_spectral_extract_response(struct spectral_cp_request *sscan_req,
					  void *outdata, u_int32_t *outsize)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	switch (sscan_req->req_id) {
	case SPECTRAL_GET_CONFIG:
		{
			if (!outdata || !outsize ||
			    (*outsize < sizeof(struct spectral_config))) {
				status = QDF_STATUS_E_FAILURE;
				goto bad;
			}
			*outsize = sizeof(struct spectral_config);
			qdf_mem_copy(outdata,
				     &sscan_req->config_req.sscan_config,
				     sizeof(struct spectral_config));
		}
		break;

	case SPECTRAL_IS_ACTIVE:
		{
			if (!outdata || !outsize ||
			    *outsize < sizeof(uint32_t)) {
				status = QDF_STATUS_E_FAILURE;
				goto bad;
			}
			*outsize = sizeof(uint32_t);
			*((uint32_t *)outdata) =
				sscan_req->status_req.is_active;
		}
		break;

	case SPECTRAL_IS_ENABLED:
		{
			if (!outdata || !outsize ||
			    *outsize < sizeof(uint32_t)) {
				status = QDF_STATUS_E_FAILURE;
				goto bad;
			}
			*outsize = sizeof(uint32_t);
			*((uint32_t *)outdata) =
				sscan_req->status_req.is_enabled;
		}
		break;

	case SPECTRAL_GET_DEBUG_LEVEL:
		{
			if (!outdata || !outsize ||
			    *outsize < sizeof(uint32_t)) {
				status = QDF_STATUS_E_FAILURE;
				goto bad;
			}
			*outsize = sizeof(uint32_t);
			*((uint32_t *)outdata) =
				sscan_req->debug_req.spectral_dbg_level;
		}
		break;

	case SPECTRAL_GET_CAPABILITY_INFO:
		{
			if (!outdata || !outsize ||
			    *outsize < sizeof(struct spectral_caps)) {
				status = QDF_STATUS_E_FAILURE;
				goto bad;
			}
			*outsize = sizeof(struct spectral_caps);
			qdf_mem_copy(outdata, &sscan_req->caps_req.sscan_caps,
				     sizeof(struct spectral_caps));
		}
		break;

	case SPECTRAL_GET_DIAG_STATS:
		{
			if (!outdata || !outsize ||
			    (*outsize < sizeof(struct spectral_diag_stats))) {
				status = QDF_STATUS_E_FAILURE;
				goto bad;
			}
			*outsize = sizeof(struct spectral_diag_stats);
			qdf_mem_copy(outdata, &sscan_req->diag_req.sscan_diag,
				     sizeof(struct spectral_diag_stats));
		}
		break;

	case SPECTRAL_GET_CHAN_WIDTH:
		{
			if (!outdata || !outsize ||
			    *outsize < sizeof(uint32_t)) {
				status = QDF_STATUS_E_FAILURE;
				goto bad;
			}
			*outsize = sizeof(uint32_t);
			*((uint32_t *)outdata) =
				sscan_req->chan_width_req.chan_width;
		}
		break;

	default:
		break;
	}

bad:
	return status;
}

qdf_export_symbol(ucfg_spectral_extract_response);

QDF_STATUS ucfg_spectral_register_to_dbr(struct wlan_objmgr_pdev *pdev)
{
	return spectral_pdev_open(pdev);
}
