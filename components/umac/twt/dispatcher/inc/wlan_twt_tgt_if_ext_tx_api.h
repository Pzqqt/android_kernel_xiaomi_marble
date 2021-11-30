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
 * DOC: wlan_twt_tgt_if_ext_tx_api.h
 *
 * API declarations to send WMI command using Tx Ops
 */
#ifndef _WLAN_TWT_TGT_IF_EXT_TX_API_H_
#define _WLAN_TWT_TGT_IF_EXT_TX_API_H_

/**
 * tgt_twt_setup_req_send() - Send twt setup request
 * @psoc: Pointer to psoc object
 * @req: TWT setup request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
tgt_twt_setup_req_send(struct wlan_objmgr_psoc *psoc,
		       struct twt_add_dialog_param *req);

/**
 * tgt_twt_teardown_req_send() - Send twt teardown request
 * @psoc: Pointer to psoc object
 * @req: TWT setup request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
tgt_twt_teardown_req_send(struct wlan_objmgr_psoc *psoc,
			  struct twt_del_dialog_param *req);

/**
 * tgt_twt_pause_req_send() - Send twt pause request
 * @psoc: Pointer to psoc object
 * @req: TWT pause request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
tgt_twt_pause_req_send(struct wlan_objmgr_psoc *psoc,
		       struct twt_pause_dialog_cmd_param *req);

/**
 * tgt_twt_resume_req_send() - Send twt resume request
 * @psoc: Pointer to psoc object
 * @req: TWT resume request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
tgt_twt_resume_req_send(struct wlan_objmgr_psoc *psoc,
			struct twt_resume_dialog_cmd_param *req);

/**
 * tgt_twt_nudge_req_send() - Send twt nudge request
 * @psoc: Pointer to psoc object
 * @req: TWT nudge request
 *
 * Return: QDF_STATUS
 */
QDF_STATUS
tgt_twt_nudge_req_send(struct wlan_objmgr_psoc *psoc,
		       struct twt_nudge_dialog_cmd_param *req);

#endif
