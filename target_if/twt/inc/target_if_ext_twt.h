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
 *  DOC: target_if_ext_twt.h
 *  This file contains twt component's target related APIs
 */

#ifndef _TARGET_IF_EXT_TWT_H_
#define _TARGET_IF_EXT_TWT_H_

#include <wlan_lmac_if_def.h>

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)

/**
 * target_if_twt_register_ext_events() - Register twt ext wmi events
 * @psoc: psoc handle
 *
 * Register TWT ext (outside of qcacmn) WMI events
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_register_ext_events(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_twt_deregister_ext_events() - De-register twt ext wmi events
 * @psoc: psoc handle
 *
 * Register TWT ext (outside of qcacmn) WMI events
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_deregister_ext_events(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_twt_register_ext_tx_ops() - Register twt tx ops
 * @twt_tx_ops: twt_tx_ops structure
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_register_ext_tx_ops(struct wlan_lmac_if_twt_tx_ops *twt_tx_ops);

/**
 * target_if_twt_setup_req() - target if twt setup request
 * @psoc: psoc handle
 * @req: TWT add dialog request structure
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_setup_req(struct wlan_objmgr_psoc *psoc,
			struct twt_add_dialog_param *req);

/**
 * target_if_twt_teardown_req() - target if twt teardown request
 * @psoc: psoc handle
 * @req: TWT del dialog request structure
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_teardown_req(struct wlan_objmgr_psoc *psoc,
			   struct twt_del_dialog_param *req);

/**
 * target_if_twt_pause_req() - target if twt pause request
 * @psoc: psoc handle
 * @req: TWT pause dialog request structure
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_pause_req(struct wlan_objmgr_psoc *psoc,
			struct twt_pause_dialog_cmd_param *req);

/**
 * target_if_twt_resume_req() - target if twt resume request
 * @psoc: psoc handle
 * @req: TWT resume dialog request structure
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_resume_req(struct wlan_objmgr_psoc *psoc,
			 struct twt_resume_dialog_cmd_param *req);

/**
 * target_if_twt_nudge_req() - target if twt nudge request
 * @psoc: psoc handle
 * @req: TWT nudge dialog request structure
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_nudge_req(struct wlan_objmgr_psoc *psoc,
			struct twt_nudge_dialog_cmd_param *req);
#else
static inline QDF_STATUS
target_if_twt_register_ext_tx_ops(struct wlan_lmac_if_twt_tx_ops *twt_tx_ops)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_if_twt_register_ext_events(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_if_twt_deregister_ext_events(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_if_twt_setup_req(struct wlan_objmgr_psoc *psoc,
			struct twt_add_dialog_param *req)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_if_twt_teardown_req(struct wlan_objmgr_psoc *psoc,
			   struct twt_del_dialog_param *req)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_if_twt_pause_req(struct wlan_objmgr_psoc *psoc,
			struct twt_pause_dialog_cmd_param *req)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_if_twt_resume_req(struct wlan_objmgr_psoc *psoc,
			 struct twt_resume_dialog_cmd_param *req)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS
target_if_twt_nudge_req(struct wlan_objmgr_psoc *psoc,
			struct twt_nudge_dialog_cmd_param *req)
{
	return QDF_STATUS_SUCCESS;
}

#endif
#endif /*_TARGET_IF_EXT_TWT_H_ */

