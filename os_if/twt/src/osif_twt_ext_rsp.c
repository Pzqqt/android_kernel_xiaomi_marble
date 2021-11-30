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
 * DOC: osif_twt_ext_rsp.c
 *
 */
#include <wlan_cfg80211.h>
#include <osif_twt_ext_req.h>
#include <osif_twt_ext_rsp.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_osif_priv.h>

QDF_STATUS
osif_twt_setup_complete_cb(struct wlan_objmgr_psoc *psoc,
			   struct twt_add_dialog_complete_event *event,
			   bool renego_fail)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_teardown_complete_cb(struct wlan_objmgr_psoc *psoc,
			      struct twt_del_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_pause_complete_cb(struct wlan_objmgr_psoc *psoc,
			   struct twt_pause_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_resume_complete_cb(struct wlan_objmgr_psoc *psoc,
			   struct twt_resume_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_nudge_complete_cb(struct wlan_objmgr_psoc *psoc,
			   struct twt_nudge_dialog_complete_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_notify_complete_cb(struct wlan_objmgr_psoc *psoc,
			    struct twt_notify_event_param *event)
{
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
osif_twt_ack_complete_cb(struct wlan_objmgr_psoc *psoc,
			 struct twt_ack_complete_event_param *params,
			 void *context)
{
	return QDF_STATUS_SUCCESS;
}

