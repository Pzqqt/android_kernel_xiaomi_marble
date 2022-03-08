/*
 * Copyright (c) 2022 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wlan_twt_tgt_if_ext_rx_api.c
 *
 * This file provide definition for APIs registered for LMAC TWT Rx Ops
 */
#include <qdf_types.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_twt_public_structs.h>
#include <wlan_twt_tgt_if_ext_rx_ops.h>
#include "twt/core/src/wlan_twt_main.h"

static QDF_STATUS
tgt_twt_setup_complete_resp_handler(struct wlan_objmgr_psoc *psoc,
			     struct twt_add_dialog_complete_event *event)
{
	return wlan_twt_setup_complete_event_handler(psoc, event);
}

static QDF_STATUS
tgt_twt_teardown_complete_resp_handler(struct wlan_objmgr_psoc *psoc,
			     struct twt_del_dialog_complete_event_param *event)
{
	return wlan_twt_teardown_complete_event_handler(psoc, event);
}

static QDF_STATUS
tgt_twt_pause_complete_resp_handler(struct wlan_objmgr_psoc *psoc,
			    struct twt_pause_dialog_complete_event_param *event)
{
	return wlan_twt_pause_complete_event_handler(psoc, event);
}

static QDF_STATUS
tgt_twt_resume_complete_resp_handler(struct wlan_objmgr_psoc *psoc,
			   struct twt_resume_dialog_complete_event_param *event)
{
	return wlan_twt_resume_complete_event_handler(psoc, event);
}

static QDF_STATUS
tgt_twt_nudge_complete_resp_handler(struct wlan_objmgr_psoc *psoc,
			    struct twt_nudge_dialog_complete_event_param *event)
{
	return wlan_twt_nudge_complete_event_handler(psoc, event);
}

static QDF_STATUS
tgt_twt_notify_complete_resp_handler(struct wlan_objmgr_psoc *psoc,
			    struct twt_notify_event_param *event)
{
	return wlan_twt_notify_event_handler(psoc, event);
}

static QDF_STATUS
tgt_twt_ack_complete_resp_handler(struct wlan_objmgr_psoc *psoc,
			    struct twt_ack_complete_event_param *event)
{
	return wlan_twt_ack_event_handler(psoc, event);
}

void tgt_twt_register_ext_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops)
{
	struct wlan_lmac_if_twt_rx_ops *twt_rx_ops = &rx_ops->twt_rx_ops;

	twt_rx_ops->twt_setup_comp_cb = tgt_twt_setup_complete_resp_handler;
	twt_rx_ops->twt_teardown_comp_cb =
					tgt_twt_teardown_complete_resp_handler;
	twt_rx_ops->twt_pause_comp_cb = tgt_twt_pause_complete_resp_handler;
	twt_rx_ops->twt_resume_comp_cb = tgt_twt_resume_complete_resp_handler;
	twt_rx_ops->twt_nudge_comp_cb = tgt_twt_nudge_complete_resp_handler;
	twt_rx_ops->twt_notify_comp_cb = tgt_twt_notify_complete_resp_handler;
	twt_rx_ops->twt_ack_comp_cb = tgt_twt_ack_complete_resp_handler;
}

