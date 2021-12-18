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
 * DOC: wlan_twt_api.h
 */
#ifndef _WLAN_TWT_API_H_
#define _WLAN_TWT_API_H_

#if defined(WLAN_SUPPORT_TWT) && defined(WLAN_TWT_CONV_SUPPORTED)
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_global_obj.h>
#include <wlan_lmac_if_def.h>

#define twt_alert(params...) \
	QDF_TRACE_FATAL(QDF_MODULE_ID_TWT, params)
#define twt_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_TWT, params)
#define twt_warn(params...) \
	QDF_TRACE_WARN(QDF_MODULE_ID_TWT, params)
#define twt_notice(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_TWT, params)
#define twt_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_TWT, params)

#define twt_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_TWT, params)
#define twt_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_TWT, params)
#define twt_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_TWT, params)
#define twt_nofl_notice(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_TWT, params)
#define twt_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_TWT, params)

/**
 * twt_psoc_enable() - twt psoc enable
 * @psoc: psoc handle
 *
 * return: QDF_STATUS
 */
QDF_STATUS twt_psoc_enable(struct wlan_objmgr_psoc *psoc);

/**
 * twt_psoc_disable() - twt psoc disable
 * @psoc: psoc handle
 *
 * return: QDF_STATUS
 */
QDF_STATUS twt_psoc_disable(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_twt_init() - twt init
 * @psoc: psoc handle
 *
 * return: QDF_STATUS
 */
QDF_STATUS wlan_twt_init(void);

/**
 * wlan_twt_deinit() - twt de-init
 * @psoc: psoc handle
 *
 * return: QDF_STATUS
 */
QDF_STATUS wlan_twt_deinit(void);

/**
 * wlan_twt_get_tx_ops() - get tx ops
 * @psoc: psoc handle
 *
 * return: QDF_STATUS
 */
struct wlan_lmac_if_twt_tx_ops *
wlan_twt_get_tx_ops(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_twt_get_rx_ops() - get rx ops
 * @psoc: psoc handle
 *
 * return: QDF_STATUS
 */
struct wlan_lmac_if_twt_rx_ops *
wlan_twt_get_rx_ops(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_twt_psoc_get_comp_private_obj() - get twt priv obj
 * @psoc: psoc handle
 *
 * return: QDF_STATUS
 */
struct twt_psoc_priv_obj*
wlan_twt_psoc_get_comp_private_obj(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_set_peer_twt_capabilities() - set peer twt capabilities
 * @psoc: psoc handle
 * @peer_mac: peer mac address
 * @peer_cap: TWT peer capability bitmap. Refer enum
 * wlan_twt_capabilities for representation.
 *
 * return: QDF_STATUS
 */
QDF_STATUS
wlan_set_peer_twt_capabilities(struct wlan_objmgr_psoc *psoc,
			       struct qdf_mac_addr *peer_mac,
			       uint8_t peer_cap);

#else
static inline
QDF_STATUS twt_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS twt_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS wlan_twt_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline
QDF_STATUS wlan_twt_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}
#endif
#endif /* _WLAN_TWT_API_H_ */
