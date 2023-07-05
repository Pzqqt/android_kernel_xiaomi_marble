/*
 * Copyright (c) 2022-2023 Qualcomm Innovation Center, Inc. All rights reserved.
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

/*
 * DOC: contains declarations for CoAP core functions
 */

#ifndef _WLAN_COAP_MAIN_H_
#define _WLAN_COAP_MAIN_H_

#ifdef WLAN_FEATURE_COAP
#include "wlan_objmgr_vdev_obj.h"

#define coap_err(params...) \
	QDF_TRACE_ERROR(QDF_MODULE_ID_COAP, params)
#define coap_info(params...) \
	QDF_TRACE_INFO(QDF_MODULE_ID_COAP, params)
#define coap_debug(params...) \
	QDF_TRACE_DEBUG(QDF_MODULE_ID_COAP, params)

/**
 * struct wlan_coap_comp_priv - CoAP component private structure
 * @req_id: cache get request id
 * @cache_get_cbk: Callback function to be called with the cache get result
 * @cache_get_context: context to be used by the caller to associate the get
 * cache request with the response
 */
struct wlan_coap_comp_priv {
	uint32_t req_id;
	coap_cache_get_callback cache_get_cbk;
	void *cache_get_context;
};

static inline struct wlan_coap_comp_priv *
wlan_get_vdev_coap_obj(struct wlan_objmgr_vdev *vdev)
{
	return wlan_objmgr_vdev_get_comp_private_obj(vdev,
						     WLAN_UMAC_COMP_COAP);
}

/*
 * wlan_coap_init() - CoAP module initialization API
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_coap_init(void);

/*
 * wlan_coap_init() - CoAP module deinitialization API
 *
 * Return: QDF_STATUS
 */
QDF_STATUS wlan_coap_deinit(void);

/**
 * wlan_coap_enable(): API to enable CoAP component
 * @psoc: pointer to psoc
 *
 * This API is invoked from dispatcher psoc enable.
 * This API will register CoAP related WMI event handlers.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_coap_enable(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_coap_disable(): API to disable CoAP component
 * @psoc: pointer to psoc
 *
 * This API is invoked from dispatcher psoc disable.
 * This API will unregister CoAP related WMI event handlers.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS wlan_coap_disable(struct wlan_objmgr_psoc *psoc);

/**
 * wlan_coap_offload_reply_enable() - private API to enable CoAP offload reply
 * @vdev: pointer to vdev object
 * @param: parameters of CoAP offload reply
 *
 * Return: status of operation
 */
QDF_STATUS
wlan_coap_offload_reply_enable(struct wlan_objmgr_vdev *vdev,
			       struct coap_offload_reply_param *param);

/**
 * wlan_coap_offload_reply_disable() - private API to disable CoAP offload reply
 * @vdev: pointer to vdev object
 * @req_id: request id
 * @cbk: callback function to be called with the cache info
 * @context: context to be used by the caller to associate the disable request
 *
 * Return: status of operation
 */
QDF_STATUS
wlan_coap_offload_reply_disable(struct wlan_objmgr_vdev *vdev, uint32_t req_id,
				coap_cache_get_callback cbk, void *context);

/**
 * wlan_coap_offload_periodic_tx_enable() - private API to enable CoAP offload
 * periodic transmitting
 * @vdev: pointer to vdev object
 * @param: parameters of CoAP periodic transmit
 *
 * Return: status of operation
 */
QDF_STATUS
wlan_coap_offload_periodic_tx_enable(struct wlan_objmgr_vdev *vdev,
			struct coap_offload_periodic_tx_param *param);

/**
 * wlan_coap_offload_periodic_tx_disable() - private API to disable CoAP
 * offload periodic transmitting
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: status of operation
 */
QDF_STATUS
wlan_coap_offload_periodic_tx_disable(struct wlan_objmgr_vdev *vdev,
				      uint32_t req_id);

/**
 * wlan_coap_offload_cache_get() - private API to get CoAP offload cache
 * @vdev: pointer to vdev object
 * @req_id: request id
 * @cbk: callback function to be called with the cache get result
 * @context: context to be used by the caller to associate the get
 * cache request with the response
 *
 * Return: status of operation
 */
QDF_STATUS
wlan_coap_offload_cache_get(struct wlan_objmgr_vdev *vdev, uint32_t req_id,
			    coap_cache_get_callback cbk, void *context);
#endif
#endif
