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
 * DOC: osif_twt_rsp.c
 *
 */
#include <qdf_status.h>
#include <wlan_cfg80211.h>
#include <osif_twt_rsp.h>
#include <osif_twt_util.h>
#include <wlan_osif_request_manager.h>

/**
 * osif_twt_enable_complete_cb() - callback for twt enable complete event
 * @psoc: Pointer to global psoc
 * @event: Pointer to TWT enable dialog complete event structure
 * @context: TWT enable context set during TWT enable request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
osif_twt_enable_complete_cb(struct wlan_objmgr_psoc *psoc,
			    struct twt_enable_complete_event_param *event,
			    void *context)
{
	struct twt_en_dis_priv *twt_en_priv;
	struct osif_request *request = NULL;

	osif_debug("osif_twt_enable_complete_cb");
	request = osif_request_get(context);
	if (!request) {
		osif_err("obsolete request");
		return QDF_STATUS_E_FAILURE;
	}

	twt_en_priv = osif_request_priv(request);
	if (!twt_en_priv) {
		osif_err("obsolete twt_en_priv");
		return QDF_STATUS_E_FAILURE;
	}

	twt_en_priv->pdev_id = event->pdev_id;
	twt_en_priv->status = event->status;

	osif_request_complete(request);
	osif_request_put(request);
	return QDF_STATUS_SUCCESS;
}

/**
 * osif_twt_disable_complete_cb() - callback for twt disable complete event
 * @psoc: Pointer to global psoc
 * @event: Pointer to TWT disable dialog complete event structure
 * @context: TWT disable context set during TWT disable request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
osif_twt_disable_complete_cb(struct wlan_objmgr_psoc *psoc,
			     struct twt_disable_complete_event_param *event,
			     void *context)
{
	struct twt_en_dis_priv *twt_en_priv;
	struct osif_request *request = NULL;

	osif_debug("osif_handle_twt_disable_complete");
	request = osif_request_get(context);
	if (!request) {
		osif_err("obsolete request");
		return QDF_STATUS_E_FAILURE;
	}

	twt_en_priv = osif_request_priv(request);
	if (!twt_en_priv) {
		osif_err("obsolete twt_en_priv");
		return QDF_STATUS_E_FAILURE;
	}

	twt_en_priv->pdev_id = event->pdev_id;
	twt_en_priv->status = event->status;

	osif_request_complete(request);
	osif_request_put(request);
	return QDF_STATUS_SUCCESS;
}

