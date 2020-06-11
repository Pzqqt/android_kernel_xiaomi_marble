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

/*
 * DOC: wlan_cm_tgt_if_tx_api.c
 *
 * Implementation for the Common Roaming interfaces.
 */

#include "wlan_objmgr_psoc_obj.h"
#include "wlan_policy_mgr_api.h"
#include "wlan_mlme_ucfg_api.h"
#include "wlan_reg_services_api.h"
#include "wlan_cm_tgt_if_tx_api.h"
#include "wlan_mlme_public_struct.h"
#include "wma.h"

#ifdef WLAN_FEATURE_ROAM_OFFLOAD
QDF_STATUS
wlan_cm_roam_send_set_vdev_pcl(struct wlan_objmgr_psoc *psoc,
			       struct set_pcl_req *pcl_req)
{
	struct wlan_cm_roam_tx_ops roam_tx_ops;
	struct saved_channels *freq_list;
	struct wlan_objmgr_vdev *vdev;
	struct wmi_pcl_chan_weights *weights;
	QDF_STATUS status = QDF_STATUS_E_FAILURE;
	uint8_t band_capability;
	uint16_t i;

	/*
	 * If vdev_id is WLAN_UMAC_VDEV_ID_MAX, then PDEV pcl command
	 * needs to be sent
	 */
	if (pcl_req && pcl_req->vdev_id == WLAN_UMAC_VDEV_ID_MAX)
		return QDF_STATUS_E_FAILURE;

	status = ucfg_mlme_get_band_capability(psoc, &band_capability);
	if (QDF_IS_STATUS_ERROR(status))
		return QDF_STATUS_E_FAILURE;

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc, pcl_req->vdev_id,
						    WLAN_MLME_SB_ID);
	if (!vdev) {
		mlme_err("vdev object is NULL");
		status = QDF_STATUS_E_FAILURE;
		return status;
	}

	roam_tx_ops = GET_CM_ROAM_TX_OPS_FROM_VDEV(vdev);
	if (!roam_tx_ops.send_vdev_set_pcl_cmd) {
		mlme_err("send_vdev_set_pcl_cmd is NULL");
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	freq_list = qdf_mem_malloc(sizeof(*freq_list));
	if (!freq_list) {
		status = QDF_STATUS_E_FAILURE;
		goto end;
	}

	mlme_roam_fill_saved_channels(psoc, freq_list->freq,
				      &freq_list->num_channels);

	weights = &pcl_req->chan_weights;
	for (i = 0; i < freq_list->num_channels; i++)
		weights->saved_chan_list[i] = freq_list->freq[i];

	weights->saved_num_chan = freq_list->num_channels;
	status = policy_mgr_get_valid_chan_weights(
			psoc, (struct policy_mgr_pcl_chan_weights *)weights);

	qdf_mem_free(freq_list);

	for (i = 0; i < chan_weights->saved_num_chan; i++) {
		weights->weighed_valid_list[i] =
			wma_map_pcl_weights(weights->weighed_valid_list[i]);

		/* Dont allow roaming on 2G when 5G_ONLY configured */
		if ((band_capability == BAND_5G ||
		     pcl_req->band_mask == BIT(REG_BAND_5G)) &&
		     WLAN_REG_IS_24GHZ_CH_FREQ(weights->saved_chan_list[i]))
			weights->weighed_valid_list[i] =
				WEIGHT_OF_DISALLOWED_CHANNELS;

		if ((band_capability == BAND_2G ||
		     pcl_req->band_mask == BIT(REG_BAND_2G)) &&
		    !WLAN_REG_IS_24GHZ_CH_FREQ(weights->saved_chan_list[i]))
			weights->weighed_valid_list[i] =
				WEIGHT_OF_DISALLOWED_CHANNELS;
	}

	if (QDF_IS_STATUS_ERROR(status)) {
		mlme_err("Error in creating weighed pcl");
		goto end;
	}

	mlme_debug("LFR3: Dump Vdev PCL weights");
	policy_mgr_dump_channel_list(weights->saved_num_chan,
				     weights->saved_chan_list,
				     weights->weighed_valid_list);

	status = roam_tx_ops.send_vdev_set_pcl_cmd(vdev, pcl_req);
	if (QDF_IS_STATUS_ERROR(status))
		mlme_err("set vdev PCL failed");

end:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_MLME_SB_ID);

	return status;
}
#endif
