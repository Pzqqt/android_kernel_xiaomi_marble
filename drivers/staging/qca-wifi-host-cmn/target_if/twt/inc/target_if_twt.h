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
 * DOC: target_if_twt.h
 */

#ifndef _TARGET_IF_TWT_H_
#define _TARGET_IF_TWT_H_

#include <wmi_unified_param.h>
#include <wlan_lmac_if_def.h>
#include <target_if.h>
#include <wlan_twt_public_structs.h>

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
/**
 * target_if_twt_set_twt_ack_support() - set ack support
 * @psoc: psoc handle
 * @val: value
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_set_twt_ack_support(struct wlan_objmgr_psoc *psoc,
				  bool val);

/**
 * target_if_twt_fill_tgt_caps() - fill twt target caps
 * @psoc: psoc handle
 * @wmi_handle: wmi handle
 *
 * This function populates twt target capability info in twt private psoc object
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_fill_tgt_caps(struct wlan_objmgr_psoc *psoc,
			    wmi_unified_t wmi_handle);

/**
 * target_if_twt_register_tx_ops() - Register twt tx ops
 * @tx_ops: tx_ops structure
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops);

/**
 * target_if_twt_register_events() - Register twt wmi events
 * @psoc: psoc handle
 *
 * Register TWT WMI events
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_register_events(struct wlan_objmgr_psoc *psoc);

/**
 * target_if_twt_deregister_events() - De-register twt wmi events
 * @psoc: psoc handle
 *
 * De-register TWT WMI events
 *
 * return: QDF_STATUS
 */
QDF_STATUS
target_if_twt_deregister_events(struct wlan_objmgr_psoc *psoc);

#else
static inline
QDF_STATUS target_if_twt_set_twt_ack_support(struct wlan_objmgr_psoc *psoc,
					     bool val)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS target_if_twt_fill_tgt_caps(struct wlan_objmgr_psoc *psoc,
				       wmi_unified_t wmi_handle)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS target_if_twt_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS target_if_twt_register_events(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS target_if_twt_deregister_events(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

#endif
#endif /* _TARGET_IF_TWT_H_ */
