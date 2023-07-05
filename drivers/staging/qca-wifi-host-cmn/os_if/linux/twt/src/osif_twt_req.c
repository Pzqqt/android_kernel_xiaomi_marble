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
  *  DOC: osif_twt_req.c
  *  This file contains twt request related osif APIs
  */
#include <wlan_cfg80211.h>
#include <osif_twt_req.h>
#include <osif_twt_util.h>
#include <wlan_osif_request_manager.h>
#include <wlan_twt_ucfg_api.h>
#include <wlan_twt_ucfg_ext_api.h>

#define TWT_DISABLE_COMPLETE_TIMEOUT 1000
#define TWT_ENABLE_COMPLETE_TIMEOUT  1000

int osif_twt_requestor_enable(struct wlan_objmgr_psoc *psoc,
			      struct twt_enable_param *req)
{
	struct osif_request *request;
	int ret;
	QDF_STATUS status;
	struct twt_en_dis_priv *twt_en_priv;
	void *context;
	static const struct osif_request_params params = {
				.priv_size = sizeof(*twt_en_priv),
				.timeout_ms = TWT_ENABLE_COMPLETE_TIMEOUT,
	};

	request = osif_request_alloc(&params);
	if (!request) {
		osif_err("Request allocation failure");
		return -ENOMEM;
	}
	context = osif_request_cookie(request);

	status = ucfg_twt_requestor_enable(psoc, req, context);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_warn("Failed to send TWT requestor enable command");
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		osif_warn("TWT Requestor Enable timedout ret:%d", ret);
		ret = -ETIMEDOUT;
		goto cleanup;
	}

cleanup:
	osif_request_put(request);
	return ret;
}

int osif_twt_responder_enable(struct wlan_objmgr_psoc *psoc,
			      struct twt_enable_param *req)
{
	struct osif_request *request;
	int ret;
	QDF_STATUS status;
	struct twt_en_dis_priv *twt_en_priv;
	void *context;
	static const struct osif_request_params params = {
				.priv_size = sizeof(*twt_en_priv),
				.timeout_ms = TWT_ENABLE_COMPLETE_TIMEOUT,
	};

	request = osif_request_alloc(&params);
	if (!request) {
		osif_err("Request allocation failure");
		return -ENOMEM;
	}
	context = osif_request_cookie(request);

	status = ucfg_twt_responder_enable(psoc, req, context);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_warn("Failed to send TWT responder enable command");
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		osif_warn("TWT Responder Enable timedout ret:%d", ret);
		ret = -ETIMEDOUT;
		goto cleanup;
	}

cleanup:
	osif_request_put(request);
	return ret;
}

int osif_twt_requestor_disable(struct wlan_objmgr_psoc *psoc,
			       struct twt_disable_param *req)
{
	struct osif_request *request;
	int ret;
	QDF_STATUS status;
	struct twt_en_dis_priv *twt_en_priv;
	void *context;
	static const struct osif_request_params params = {
				.priv_size = sizeof(*twt_en_priv),
				.timeout_ms = TWT_DISABLE_COMPLETE_TIMEOUT,
	};

	request = osif_request_alloc(&params);
	if (!request) {
		osif_err("Request allocation failure");
		return -ENOMEM;
	}
	context = osif_request_cookie(request);

	status = ucfg_twt_requestor_disable(psoc, req, context);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_warn("Failed to send TWT requestor disable command");
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		osif_warn("TWT Requestor disable timedout ret:%d", ret);
		ret = -ETIMEDOUT;
		goto cleanup;
	}

cleanup:
	osif_request_put(request);
	return ret;
}

int osif_twt_responder_disable(struct wlan_objmgr_psoc *psoc,
			       struct twt_disable_param *req)
{
	struct osif_request *request;
	int ret;
	QDF_STATUS status;
	struct twt_en_dis_priv *twt_en_priv;
	void *context;
	static const struct osif_request_params params = {
				.priv_size = sizeof(*twt_en_priv),
				.timeout_ms = TWT_DISABLE_COMPLETE_TIMEOUT,
	};

	request = osif_request_alloc(&params);
	if (!request) {
		osif_err("Request allocation failure");
		return -ENOMEM;
	}
	context = osif_request_cookie(request);

	status = ucfg_twt_responder_disable(psoc, req, context);
	if (QDF_IS_STATUS_ERROR(status)) {
		osif_warn("Failed to send TWT responder disable command");
		ret = qdf_status_to_os_return(status);
		goto cleanup;
	}

	ret = osif_request_wait_for_response(request);
	if (ret) {
		osif_warn("TWT Responder disable timedout ret:%d", ret);
		ret = -ETIMEDOUT;
		goto cleanup;
	}

cleanup:
	osif_request_put(request);
	return ret;
}

