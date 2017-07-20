/*
 * Copyright (c) 2017 The Linux Foundation. All rights reserved.
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
 * DOC: Implements arp offload feature API's
 */

#include "qdf_lock.h"
#include "wlan_pmo_hw_filter.h"
#include "wlan_pmo_tgt_api.h"
#include "wlan_pmo_main.h"
#include "wlan_pmo_obj_mgmt_public_struct.h"

static QDF_STATUS pmo_core_conf_hw_filter(struct wlan_objmgr_vdev *vdev,
					  enum pmo_hw_filter_mode mode)
{
	QDF_STATUS status;
	struct pmo_hw_filter_params req = { .mode = mode, };

	PMO_ENTER();

	if (!pmo_core_is_vdev_connected(vdev)) {
		status = QDF_STATUS_E_NOSUPPORT;
		goto exit_with_status;
	}

	req.vdev_id = pmo_vdev_get_id(vdev);
	status = pmo_tgt_conf_hw_filter(pmo_vdev_get_psoc(vdev), &req);

exit_with_status:
	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_core_enable_hw_filter_in_fwr(struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS status;
	struct pmo_psoc_priv_obj *psoc_priv;
	enum pmo_hw_filter_mode mode;

	PMO_ENTER();

	status = pmo_vdev_get_ref(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		goto exit_with_status;

	psoc_priv = pmo_vdev_get_psoc_priv(vdev);
	qdf_spin_lock_bh(&psoc_priv->lock);
	mode = psoc_priv->psoc_cfg.hw_filter_mode;
	qdf_spin_unlock_bh(&psoc_priv->lock);

	status = pmo_core_conf_hw_filter(vdev, mode);

	pmo_vdev_put_ref(vdev);

exit_with_status:
	PMO_EXIT();

	return status;
}

QDF_STATUS pmo_core_disable_hw_filter_in_fwr(struct wlan_objmgr_vdev *vdev)
{
	QDF_STATUS status;

	PMO_ENTER();

	status = pmo_vdev_get_ref(vdev);
	if (QDF_IS_STATUS_ERROR(status))
		goto exit_with_status;

	status = pmo_core_conf_hw_filter(vdev, PMO_HW_FILTER_DISABLED);

	pmo_vdev_put_ref(vdev);

exit_with_status:
	PMO_EXIT();

	return status;
}
