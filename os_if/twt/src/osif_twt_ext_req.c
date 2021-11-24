/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 *  DOC: osif_twt_ext_req.c
 *  This file contains twt component's osif API implementation
 */
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_twt_ucfg_api.h>
#include <wlan_twt_ucfg_ext_api.h>
#include <osif_twt_req.h>
#include <osif_twt_ext_req.h>
#include <wlan_policy_mgr_api.h>
#include <wlan_cm_api.h>
#include <wlan_cfg80211.h>
#include <wlan_cm_roam_api.h>
#include <wlan_serialization_api.h>
#include <osif_twt_internal.h>
#include <wlan_osif_request_manager.h>

#define TWT_ACK_COMPLETE_TIMEOUT 1000

#define TWT_FLOW_TYPE_ANNOUNCED 0
#define TWT_FLOW_TYPE_UNANNOUNCED 1

#define TWT_SETUP_WAKE_INTVL_MANTISSA_MAX       0xFFFF
#define TWT_SETUP_WAKE_DURATION_MAX             0xFFFF
#define TWT_SETUP_WAKE_INTVL_EXP_MAX            31
#define TWT_MAX_NEXT_TWT_SIZE                   3

int osif_twt_send_requestor_enable_cmd(struct wlan_objmgr_psoc *psoc,
				       uint8_t pdev_id)
{
	struct twt_enable_param req = {0};

	req.pdev_id = pdev_id;
	req.ext_conf_present = true;

	return osif_twt_requestor_enable(psoc, &req);
}

int osif_twt_send_responder_enable_cmd(struct wlan_objmgr_psoc *psoc,
				       uint8_t pdev_id)
{
	struct twt_enable_param req = {0};

	req.pdev_id = pdev_id;
	req.ext_conf_present = true;

	return osif_twt_responder_enable(psoc, &req);
}

int osif_twt_send_requestor_disable_cmd(struct wlan_objmgr_psoc *psoc,
					uint8_t pdev_id)
{
	struct twt_disable_param req = {0};

	req.pdev_id = pdev_id;
	req.ext_conf_present = true;

	return osif_twt_requestor_disable(psoc, &req);
}

int osif_twt_send_responder_disable_cmd(struct wlan_objmgr_psoc *psoc,
					uint8_t pdev_id)
{
	struct twt_disable_param req = {0};

	req.pdev_id = pdev_id;
	req.ext_conf_present = true;

	return osif_twt_responder_disable(psoc, &req);
}

int osif_twt_setup_req(struct wlan_objmgr_vdev *vdev,
		       struct nlattr *twt_param_attr)
{
	return 0;
}

int osif_twt_sap_teardown_req(struct wlan_objmgr_vdev *vdev,
			      struct nlattr *twt_param_attr)
{
	return 0;
}

int osif_twt_sta_teardown_req(struct wlan_objmgr_vdev *vdev,
			      struct nlattr *twt_param_attr)
{
	return 0;
}

