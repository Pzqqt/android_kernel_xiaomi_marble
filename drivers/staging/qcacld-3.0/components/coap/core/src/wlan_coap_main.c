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
 * DOC: contains definitions for CoAP core functions
 */

#include <wlan_coap_tgt_api.h>
#include <wlan_coap_main.h>
#include <wlan_objmgr_global_obj.h>

QDF_STATUS wlan_coap_enable(struct wlan_objmgr_psoc *psoc)
{
	return tgt_coap_attach(psoc);
}

QDF_STATUS wlan_coap_disable(struct wlan_objmgr_psoc *psoc)
{
	return tgt_coap_detach(psoc);
}

static QDF_STATUS
wlan_coap_vdev_obj_create_handler(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct wlan_coap_comp_priv *coap_priv;
	QDF_STATUS status;

	if (!vdev)
		return QDF_STATUS_E_INVAL;

	coap_priv = qdf_mem_malloc(sizeof(struct wlan_coap_comp_priv));
	if (!coap_priv)
		return QDF_STATUS_E_NOMEM;

	status = wlan_objmgr_vdev_component_obj_attach(vdev,
						       WLAN_UMAC_COMP_COAP,
						       (void *)coap_priv,
						       QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status))
		qdf_mem_free(coap_priv);

	return status;
}

static QDF_STATUS
wlan_coap_vdev_obj_destroy_handler(struct wlan_objmgr_vdev *vdev, void *arg)
{
	void *coap_priv;

	if (!vdev) {
		coap_err("Vdev NULL");
		return QDF_STATUS_E_INVAL;
	}

	coap_priv = wlan_get_vdev_coap_obj(vdev);
	if (!coap_priv) {
		coap_err("coap_priv NULL");
		return QDF_STATUS_E_INVAL;
	}

	wlan_objmgr_vdev_component_obj_detach(vdev, WLAN_UMAC_COMP_COAP,
					      coap_priv);
	qdf_mem_free(coap_priv);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_coap_init(void)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	status = wlan_objmgr_register_vdev_create_handler(WLAN_UMAC_COMP_COAP,
				wlan_coap_vdev_obj_create_handler, NULL);
	if (status != QDF_STATUS_SUCCESS)
		return status;

	status = wlan_objmgr_register_vdev_destroy_handler(WLAN_UMAC_COMP_COAP,
				wlan_coap_vdev_obj_destroy_handler, NULL);
	if (QDF_IS_STATUS_SUCCESS(status))
		return status;

	wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_COAP,
				wlan_coap_vdev_obj_create_handler, NULL);
	return status;
}

QDF_STATUS wlan_coap_deinit(void)
{
	wlan_objmgr_unregister_vdev_create_handler(WLAN_UMAC_COMP_COAP,
				wlan_coap_vdev_obj_create_handler, NULL);
	wlan_objmgr_unregister_vdev_destroy_handler(WLAN_UMAC_COMP_COAP,
				wlan_coap_vdev_obj_destroy_handler, NULL);
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS
wlan_coap_offload_reply_enable(struct wlan_objmgr_vdev *vdev,
			       struct coap_offload_reply_param *params)
{
	return tgt_send_coap_offload_reply_enable(vdev, params);
}

QDF_STATUS
wlan_coap_offload_reply_disable(struct wlan_objmgr_vdev *vdev, uint32_t req_id,
				coap_cache_get_callback cbk, void *context)
{
	struct wlan_coap_comp_priv *coap_priv;

	if (!vdev) {
		coap_err("Vdev NULL");
		return QDF_STATUS_E_INVAL;
	}

	coap_priv = wlan_get_vdev_coap_obj(vdev);
	coap_priv->req_id = req_id;
	coap_priv->cache_get_context = context;
	coap_priv->cache_get_cbk = cbk;
	return tgt_send_coap_offload_reply_disable(vdev, req_id);
}

QDF_STATUS
wlan_coap_offload_periodic_tx_enable(struct wlan_objmgr_vdev *vdev,
			struct coap_offload_periodic_tx_param *params)
{
	return tgt_send_coap_offload_periodic_tx_enable(vdev, params);
}

QDF_STATUS
wlan_coap_offload_periodic_tx_disable(struct wlan_objmgr_vdev *vdev,
				      uint32_t req_id)
{
	return tgt_send_coap_offload_periodic_tx_disable(vdev, req_id);
}

QDF_STATUS
wlan_coap_offload_cache_get(struct wlan_objmgr_vdev *vdev, uint32_t req_id,
			    coap_cache_get_callback cbk, void *context)
{
	struct wlan_coap_comp_priv *coap_priv;

	if (!vdev) {
		coap_err("Vdev NULL");
		return QDF_STATUS_E_INVAL;
	}

	coap_priv = wlan_get_vdev_coap_obj(vdev);
	coap_priv->req_id = req_id;
	coap_priv->cache_get_context = context;
	coap_priv->cache_get_cbk = cbk;
	return tgt_send_coap_offload_cache_get(vdev, req_id);
}
