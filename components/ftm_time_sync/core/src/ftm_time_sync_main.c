/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: Implement various notification handlers which are accessed
 * internally in ftm_timesync component only.
 */

#include "ftm_time_sync_main.h"
#include "target_if_ftm_time_sync.h"
#include <pld_common.h>

static void ftm_time_sync_work_handler(void *arg)
{
	struct ftm_timesync_vdev_priv *vdev_priv = arg;
	struct wlan_objmgr_psoc *psoc;
	qdf_device_t qdf_dev;
	QDF_STATUS status;
	uint8_t vdev_id;
	uint64_t lpass_ts;

	if (!vdev_priv) {
		ftm_time_sync_err("ftm vdev priv is Null");
		return;
	}

	psoc = wlan_vdev_get_psoc(vdev_priv->vdev);
	if (!psoc) {
		ftm_time_sync_err("Failed to get psoc");
		return;
	}

	vdev_id = wlan_vdev_get_id(vdev_priv->vdev);

	qdf_dev = wlan_psoc_get_qdf_dev(psoc);
	pld_get_audio_wlan_timestamp(qdf_dev->dev, PLD_TRIGGER_NEGATIVE_EDGE,
				     &lpass_ts);

	qdf_mutex_acquire(&vdev_priv->ftm_time_sync_mutex);

	if (vdev_priv->num_reads) {
		vdev_priv->num_reads--;
		qdf_mutex_release(&vdev_priv->ftm_time_sync_mutex);
		qdf_delayed_work_start(&vdev_priv->ftm_time_sync_work,
				       vdev_priv->time_sync_interval);
	} else {
		qdf_mutex_release(&vdev_priv->ftm_time_sync_mutex);
	}

	if (vdev_priv->valid) {
		status = vdev_priv->tx_ops.ftm_time_sync_send_qtime(
						psoc, vdev_id, lpass_ts);
		if (status != QDF_STATUS_SUCCESS)
			ftm_time_sync_err("send_ftm_time_sync_qtime failed %d",
					  status);
		vdev_priv->valid = false;
	} else {
		vdev_priv->valid = true;
	}
}

QDF_STATUS
ftm_timesync_vdev_create_notification(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct ftm_timesync_vdev_priv *vdev_priv;
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	vdev_priv = qdf_mem_malloc(sizeof(*vdev_priv));
	if (!vdev_priv) {
		status = QDF_STATUS_E_NOMEM;
		goto exit;
	}

	status = wlan_objmgr_vdev_component_obj_attach(
				vdev, WLAN_UMAC_COMP_FTM_TIME_SYNC,
				(void *)vdev_priv, QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		ftm_time_sync_err("Failed to attach priv with vdev");
		goto free_vdev_priv;
	}

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		ftm_time_sync_err("Failed to get psoc");
		return QDF_STATUS_E_INVAL;
	}

	vdev_priv->vdev = vdev;
	status = qdf_delayed_work_create(&vdev_priv->ftm_time_sync_work,
					 ftm_time_sync_work_handler, vdev_priv);
	if (QDF_IS_STATUS_ERROR(status)) {
		ftm_time_sync_err("Failed to create ftm time sync work\n");
		goto free_vdev_priv;
	}

	qdf_mutex_create(&vdev_priv->ftm_time_sync_mutex);
	target_if_ftm_time_sync_register_tx_ops(&vdev_priv->tx_ops);
	target_if_ftm_time_sync_register_rx_ops(&vdev_priv->rx_ops);

	vdev_priv->rx_ops.ftm_timesync_register_start_stop(psoc);
	vdev_priv->rx_ops.ftm_timesync_regiser_master_slave_offset(psoc);

	vdev_priv->valid = true;

	goto exit;

free_vdev_priv:
	qdf_mem_free(vdev_priv);
	status = QDF_STATUS_E_INVAL;
exit:
	return status;
}

static QDF_STATUS
ftm_time_sync_deregister_wmi_events(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc;
	QDF_STATUS status;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc) {
		ftm_time_sync_err("Failed to get psoc");
		return QDF_STATUS_E_INVAL;
	}

	status = target_if_ftm_time_sync_unregister_ev_handlers(psoc);
	return status;
}

QDF_STATUS
ftm_timesync_vdev_destroy_notification(struct wlan_objmgr_vdev *vdev, void *arg)
{
	struct ftm_timesync_vdev_priv *vdev_priv = NULL;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;

	vdev_priv = ftm_timesync_vdev_get_priv(vdev);
	if (!vdev_priv) {
		ftm_time_sync_err("vdev priv is NULL");
		goto exit;
	}

	qdf_mutex_destroy(&vdev_priv->ftm_time_sync_mutex);
	qdf_delayed_work_destroy(&vdev_priv->ftm_time_sync_work);

	ftm_time_sync_deregister_wmi_events(vdev);

	status = wlan_objmgr_vdev_component_obj_detach(
					vdev, WLAN_UMAC_COMP_FTM_TIME_SYNC,
					(void *)vdev_priv);
	if (QDF_IS_STATUS_ERROR(status))
		ftm_time_sync_err("Failed to detach priv with vdev");

	qdf_mem_free(vdev_priv);
	vdev_priv = NULL;

exit:
	return status;
}
