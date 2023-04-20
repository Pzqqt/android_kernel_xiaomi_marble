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
 * DOC: contains CoAP north bound interface declarations
 */

#ifndef _WLAN_COAP_UCFG_API_H_
#define _WLAN_COAP_UCFG_API_H_

#include "qdf_status.h"
#include <wlan_objmgr_vdev_obj.h>
#include "wlan_coap_public_structs.h"

#ifdef WLAN_FEATURE_COAP
/**
 * ucfg_coap_offload_reply_enable() - API to enable CoAP offload reply
 * @vdev: pointer to vdev object
 * @param: parameters of CoAP offload reply
 *
 * Return: status of operation
 */
QDF_STATUS
ucfg_coap_offload_reply_enable(struct wlan_objmgr_vdev *vdev,
			       struct coap_offload_reply_param *param);

/**
 * ucfg_coap_offload_reply_disable() - API to disable CoAP offload reply
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: status of operation
 */
QDF_STATUS
ucfg_coap_offload_reply_disable(struct wlan_objmgr_vdev *vdev, uint32_t req_id,
				coap_cache_get_callback cbk, void *context);

/**
 * ucfg_coap_offload_periodic_tx_enable() - API to enable CoAP offload
 * periodic transmit
 * @vdev: pointer to vdev object
 * @param: parameters of CoAP periodic transmit
 *
 * Return: status of operation
 */
QDF_STATUS
ucfg_coap_offload_periodic_tx_enable(struct wlan_objmgr_vdev *vdev,
			struct coap_offload_periodic_tx_param *param);

/**
 * ucfg_coap_offload_periodic_tx_disable() - API to disable CoAP offload
 * periodic transmit
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: status of operation
 */
QDF_STATUS
ucfg_coap_offload_periodic_tx_disable(struct wlan_objmgr_vdev *vdev,
				      uint32_t req_id);

/**
 * ucfg_coap_offload_cache_get() - API to get CoAP offload cache
 * @vdev: pointer to vdev object
 * @req_id: request id
 * @cbk: callback function to be called with the cache get result
 * @context: context to be used by the caller to associate the get
 * cache request with the response
 *
 * Return: status of operation
 */
QDF_STATUS
ucfg_coap_offload_cache_get(struct wlan_objmgr_vdev *vdev, uint32_t req_id,
			    coap_cache_get_callback cbk, void *context);
#endif
#endif
