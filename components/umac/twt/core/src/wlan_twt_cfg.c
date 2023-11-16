/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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
#include <wlan_twt_ext_type.h>
#include <wlan_twt_api.h>
#include <cfg_ucfg_api.h>
#include <cfg_twt.h>
#include "wlan_twt_cfg.h"
#include "wlan_twt_priv.h"

QDF_STATUS wlan_twt_cfg_init(struct wlan_objmgr_psoc *psoc)
{
	struct twt_psoc_priv_obj *twt_psoc;
	psoc_twt_ext_cfg_params_t *twt_cfg;
	uint32_t bcast_conf;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	twt_cfg = &twt_psoc->cfg_params;
	bcast_conf = cfg_get(psoc, CFG_BCAST_TWT_REQ_RESP);

	twt_cfg->enable_twt = cfg_get(psoc, CFG_ENABLE_TWT);
	twt_cfg->twt_requestor = cfg_get(psoc, CFG_TWT_REQUESTOR);
	twt_cfg->twt_responder = cfg_get(psoc, CFG_TWT_RESPONDER);
	twt_cfg->twt_congestion_timeout =
				cfg_get(psoc, CFG_TWT_CONGESTION_TIMEOUT);
	twt_cfg->bcast_requestor_enabled = CFG_TWT_GET_BCAST_REQ(bcast_conf);
	twt_cfg->bcast_responder_enabled = CFG_TWT_GET_BCAST_RES(bcast_conf);
	twt_cfg->enable_twt_24ghz = cfg_get(psoc, CFG_ENABLE_TWT_24GHZ);
	twt_cfg->flex_twt_sched = cfg_default(CFG_HE_FLEX_TWT_SCHED);
	twt_cfg->req_flag = false;
	twt_cfg->res_flag = false;

	twt_debug("req: %d resp: %d", twt_cfg->twt_requestor,
		  twt_cfg->twt_responder);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_twt_cfg_deinit(struct wlan_objmgr_psoc *psoc)
{
	struct twt_psoc_priv_obj *twt_psoc;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_mem_zero(&twt_psoc->cfg_params, sizeof(twt_psoc->cfg_params));

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_twt_cfg_update(struct wlan_objmgr_psoc *psoc)
{
	struct twt_psoc_priv_obj *twt_psoc;
	psoc_twt_ext_cfg_params_t *twt_cfg;
	struct twt_tgt_caps *tgt_caps;
	bool enable_twt;

	if (!psoc) {
		twt_err("null psoc");
		return QDF_STATUS_E_FAILURE;
	}

	twt_psoc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
							 WLAN_UMAC_COMP_TWT);
	if (!twt_psoc) {
		twt_err("null twt psoc priv obj");
		return QDF_STATUS_E_FAILURE;
	}

	twt_cfg = &twt_psoc->cfg_params;
	tgt_caps = &twt_psoc->twt_caps;
	enable_twt = twt_cfg->enable_twt;

	twt_cfg->twt_requestor = QDF_MIN(tgt_caps->twt_requestor,
					(enable_twt && twt_cfg->twt_requestor));
	twt_cfg->twt_responder = QDF_MIN(tgt_caps->twt_responder,
					(enable_twt && twt_cfg->twt_responder));
	twt_cfg->bcast_requestor_enabled =
			QDF_MIN((tgt_caps->twt_bcast_req_support ||
				tgt_caps->legacy_bcast_twt_support),
				(enable_twt &&
					twt_cfg->bcast_requestor_enabled));
	twt_cfg->bcast_responder_enabled =
			QDF_MIN((tgt_caps->twt_bcast_res_support ||
				tgt_caps->legacy_bcast_twt_support),
				(enable_twt &&
					twt_cfg->bcast_responder_enabled));
	twt_debug("req: %d resp: %d bcast_req: %d bcast_resp: %d",
		  twt_cfg->twt_requestor, twt_cfg->twt_responder,
		  twt_cfg->bcast_requestor_enabled,
		  twt_cfg->bcast_responder_enabled);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj) {
		*val = cfg_default(CFG_TWT_REQUESTOR);
		return QDF_STATUS_E_INVAL;
	}

	*val = twt_psoc_obj->cfg_params.twt_requestor;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_responder(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj) {
		*val = cfg_default(CFG_TWT_RESPONDER);
		return QDF_STATUS_E_INVAL;
	}

	*val = twt_psoc_obj->cfg_params.twt_responder;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_congestion_timeout(struct wlan_objmgr_psoc *psoc,
				    uint32_t *val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj) {
		*val = cfg_default(CFG_TWT_CONGESTION_TIMEOUT);
		return QDF_STATUS_E_INVAL;
	}

	*val = twt_psoc_obj->cfg_params.twt_congestion_timeout;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_set_congestion_timeout(struct wlan_objmgr_psoc *psoc, uint32_t val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj)
		return QDF_STATUS_E_INVAL;

	twt_psoc_obj->cfg_params.twt_congestion_timeout = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_requestor_flag(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj) {
		*val = false;
		return QDF_STATUS_E_INVAL;
	}

	*val = twt_psoc_obj->cfg_params.req_flag;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_set_requestor_flag(struct wlan_objmgr_psoc *psoc, bool val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj)
		return QDF_STATUS_E_INVAL;

	twt_psoc_obj->cfg_params.req_flag = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_responder_flag(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj)
		return QDF_STATUS_E_INVAL;

	*val = twt_psoc_obj->cfg_params.res_flag;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_set_responder_flag(struct wlan_objmgr_psoc *psoc, bool val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj)
		return QDF_STATUS_E_INVAL;

	twt_psoc_obj->cfg_params.res_flag = val;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_flex_sched(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj) {
		*val = cfg_default(CFG_HE_FLEX_TWT_SCHED);
		return QDF_STATUS_E_INVAL;
	}

	*val = twt_psoc_obj->cfg_params.flex_twt_sched;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_24ghz_enabled(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj) {
		*val = cfg_default(CFG_ENABLE_TWT_24GHZ);
		return QDF_STATUS_E_INVAL;
	}

	*val = twt_psoc_obj->cfg_params.enable_twt_24ghz;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_bcast_requestor(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj) {
		uint32_t b_req_res;

		b_req_res = cfg_default(CFG_BCAST_TWT_REQ_RESP);
		*val = CFG_TWT_GET_BCAST_REQ(b_req_res);
		return QDF_STATUS_E_INVAL;
	}

	*val = twt_psoc_obj->cfg_params.bcast_requestor_enabled;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_twt_cfg_get_bcast_responder(struct wlan_objmgr_psoc *psoc, bool *val)
{
	struct twt_psoc_priv_obj *twt_psoc_obj;

	twt_psoc_obj = wlan_twt_psoc_get_comp_private_obj(psoc);
	if (!twt_psoc_obj) {
		uint32_t b_req_res;

		b_req_res = cfg_default(CFG_BCAST_TWT_REQ_RESP);
		*val = CFG_TWT_GET_BCAST_RES(b_req_res);
		return QDF_STATUS_E_INVAL;
	}

	*val = twt_psoc_obj->cfg_params.bcast_responder_enabled;

	return QDF_STATUS_SUCCESS;
}

