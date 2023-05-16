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
 * DOC: contains CoAP south bound interface definitions
 */

#ifndef _WLAN_COAP_TGT_API_H_
#define _WLAN_COAP_TGT_API_H_

#ifdef WLAN_FEATURE_COAP
#include <qdf_types.h>
#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_vdev_obj.h>

/**
 * tgt_coap_attach(): attach CoAP component
 * @psoc: pointer to psoc
 *
 * This API will register CoAP related WMI event handlers.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS tgt_coap_attach(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_coap_detach(): detach CoAP component
 * @psoc: pointer to psoc
 *
 * This API will unregister CoAP related WMI event handlers.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
QDF_STATUS tgt_coap_detach(struct wlan_objmgr_psoc *psoc);

/**
 * tgt_send_coap_offload_reply_enable() - enable CoAP offload reply
 * @vdev: pointer to vdev object
 * @param: parameters for CoAP offload reply
 *
 * Return: status of operation
 */
QDF_STATUS
tgt_send_coap_offload_reply_enable(struct wlan_objmgr_vdev *vdev,
				   struct coap_offload_reply_param *param);

/**
 * tgt_send_coap_offload_reply_disable() - disable CoAP offload reply
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: status of operation
 */
QDF_STATUS
tgt_send_coap_offload_reply_disable(struct wlan_objmgr_vdev *vdev,
				    uint32_t req_id);

/**
 * tgt_send_coap_offload_periodic_tx_enable() - enable CoAP offload
 * periodic transmitting
 * @vdev: pointer to vdev object
 * @param: parameters for CoAP periodic transmitting
 *
 * Return: status of operation
 */
QDF_STATUS
tgt_send_coap_offload_periodic_tx_enable(struct wlan_objmgr_vdev *vdev,
			struct coap_offload_periodic_tx_param *param);

/**
 * tgt_send_coap_offload_periodic_tx_disable() - disable CoAP offload
 * periodic transmitting
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: status of operation
 */
QDF_STATUS
tgt_send_coap_offload_periodic_tx_disable(struct wlan_objmgr_vdev *vdev,
					  uint32_t req_id);

/**
 * wlan_coap_offload_cache_get() - get cached CoAP messages
 * @vdev: pointer to vdev object
 * @req_id: request id
 *
 * Return: status of operation
 */
QDF_STATUS
tgt_send_coap_offload_cache_get(struct wlan_objmgr_vdev *vdev,
				uint32_t req_id);
#endif
#endif
