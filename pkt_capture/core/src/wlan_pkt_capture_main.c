/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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
 * DOC: Implement various notification handlers which are accessed
 * internally in pkt_capture component only.
 */

#include "wlan_pkt_capture_main.h"
#include "cfg_ucfg_api.h"

enum pkt_capture_mode pkt_capture_get_mode(struct wlan_objmgr_psoc *psoc)
{
	struct pkt_psoc_priv *psoc_priv;

	if (!psoc) {
		pkt_capture_err("psoc is NULL");
		return PACKET_CAPTURE_MODE_DISABLE;
	}

	psoc_priv = pkt_capture_psoc_get_priv(psoc);
	if (!psoc_priv) {
		pkt_capture_err("psoc_priv is NULL");
		return PACKET_CAPTURE_MODE_DISABLE;
	}

	return psoc_priv->cfg_param.pkt_capture_mode;
}

/**
 * pkt_capture_cfg_init() - Initialize packet capture cfg ini params
 * @psoc_priv: psoc private object
 *
 * Return: None
 */
static void
pkt_capture_cfg_init(struct pkt_psoc_priv *psoc_priv)
{
	struct pkt_capture_cfg *cfg_param;

	cfg_param = &psoc_priv->cfg_param;

	cfg_param->pkt_capture_mode = cfg_get(psoc_priv->psoc,
					      CFG_PKT_CAPTURE_MODE);
}

QDF_STATUS
pkt_capture_vdev_create_notification(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct pkt_capture_vdev_priv *vdev_priv;
	QDF_STATUS status;

	vdev_priv = qdf_mem_malloc(sizeof(*vdev_priv));
	if (!vdev_priv)
		return QDF_STATUS_E_NOMEM;

	status = wlan_objmgr_vdev_component_obj_attach(
					vdev,
					WLAN_UMAC_COMP_PKT_CAPTURE,
					vdev_priv, QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		pkt_capture_err("Failed to attach vdev component obj");
		goto free_vdev_priv;
	}

	vdev_priv->vdev = vdev;
	return status;

free_vdev_priv:
	qdf_mem_free(vdev_priv);
	return status;
}

QDF_STATUS
pkt_capture_vdev_destroy_notification(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct pkt_capture_vdev_priv *vdev_priv;
	QDF_STATUS status;

	vdev_priv = pkt_capture_vdev_get_priv(vdev);
	if (!vdev_priv) {
		pkt_capture_err("vdev priv is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_vdev_component_obj_detach(
					vdev,
					WLAN_UMAC_COMP_PKT_CAPTURE,
					vdev_priv);
	if (QDF_IS_STATUS_ERROR(status))
		pkt_capture_err("Failed to detach vdev component obj");

	qdf_mem_free(vdev_priv);
	return status;
}

QDF_STATUS
pkt_capture_psoc_create_notification(struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct pkt_psoc_priv *psoc_priv;
	QDF_STATUS status;

	psoc_priv = qdf_mem_malloc(sizeof(*psoc_priv));
	if (!psoc_priv)
		return QDF_STATUS_E_NOMEM;

	status = wlan_objmgr_psoc_component_obj_attach(psoc,
				WLAN_UMAC_COMP_PKT_CAPTURE,
				psoc_priv, QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		pkt_capture_err("Failed to attach psoc component obj");
		goto free_psoc_priv;
	}

	psoc_priv->psoc = psoc;
	pkt_capture_cfg_init(psoc_priv);

	return status;

free_psoc_priv:
	qdf_mem_free(psoc_priv);
	return status;
}

QDF_STATUS
pkt_capture_psoc_destroy_notification(struct wlan_objmgr_psoc *psoc, void *arg)
{
	struct pkt_psoc_priv *psoc_priv;
	QDF_STATUS status;

	psoc_priv = pkt_capture_psoc_get_priv(psoc);
	if (!psoc_priv) {
		pkt_capture_err("psoc priv is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
					WLAN_UMAC_COMP_PKT_CAPTURE,
					psoc_priv);
	if (QDF_IS_STATUS_ERROR(status)) {
		pkt_capture_err("Failed to detach psoc component obj");
		return status;
	}

	qdf_mem_free(psoc_priv);
	return status;
}
