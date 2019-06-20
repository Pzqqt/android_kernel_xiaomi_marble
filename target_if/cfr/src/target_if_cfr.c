/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
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

#include <target_if_cfr.h>
#include <wlan_tgt_def_config.h>
#include <target_type.h>
#include <hif_hw_version.h>
#include <ol_if_athvar.h>
#include <target_if.h>
#include <wlan_lmac_if_def.h>
#include <wlan_osif_priv.h>
#include <wlan_mlme_dispatcher.h>
#include <init_deinit_lmac.h>
#include <wlan_cfr_utils_api.h>
#include <wlan_objmgr_pdev_obj.h>
#include <target_if_cfr_8074v2.h>

int target_if_cfr_stop_capture(struct wlan_objmgr_pdev *pdev,
			       struct wlan_objmgr_peer *peer)
{
	struct peer_cfr *pe;
	struct peer_cfr_params param = {0};
	struct wmi_unified *pdev_wmi_handle = NULL;
	struct wlan_objmgr_vdev *vdev = {0};
	struct pdev_cfr *pdev_cfrobj;
	int retv = 0;

	pe = wlan_objmgr_peer_get_comp_private_obj(peer, WLAN_UMAC_COMP_CFR);
	if (pe == NULL)
		return -EINVAL;

	pdev_wmi_handle = lmac_get_pdev_wmi_handle(pdev);
	vdev = wlan_peer_get_vdev(peer);

	qdf_mem_set(&param, sizeof(param), 0);

	param.request = PEER_CFR_CAPTURE_DISABLE;
	param.macaddr = wlan_peer_get_macaddr(peer);
	param.vdev_id = wlan_vdev_get_id(vdev);

	param.periodicity = pe->period;
	param.bandwidth = pe->bandwidth;
	param.capture_method = pe->capture_method;

	retv = wmi_unified_send_peer_cfr_capture_cmd(pdev_wmi_handle, &param);

	pdev_cfrobj = wlan_objmgr_pdev_get_comp_private_obj(pdev,
							    WLAN_UMAC_COMP_CFR);
	if (!pdev_cfrobj) {
		cfr_err("pdev object for CFR is null");
		return -EINVAL;
	}
	cfr_err("CFR capture stats for this capture:");
	cfr_err("DBR event count = %u, Tx event count = %u Release count = %u",
		pdev_cfrobj->dbr_evt_cnt, pdev_cfrobj->tx_evt_cnt,
		pdev_cfrobj->release_cnt);

	pdev_cfrobj->dbr_evt_cnt = 0;
	pdev_cfrobj->tx_evt_cnt  = 0;
	pdev_cfrobj->release_cnt = 0;

	return retv;
}

int target_if_cfr_start_capture(struct wlan_objmgr_pdev *pdev,
				struct wlan_objmgr_peer *peer,
				struct cfr_capture_params *cfr_params)
{
	struct peer_cfr_params param = {0};
	struct wmi_unified *pdev_wmi_handle = NULL;
	struct wlan_objmgr_vdev *vdev;
	int retv = 0;

	pdev_wmi_handle = lmac_get_pdev_wmi_handle(pdev);
	vdev = wlan_peer_get_vdev(peer);
	qdf_mem_set(&param, sizeof(param), 0);

	param.request = PEER_CFR_CAPTURE_ENABLE;
	param.macaddr = wlan_peer_get_macaddr(peer);
	param.vdev_id = wlan_vdev_get_id(vdev);

	param.periodicity = cfr_params->period;
	param.bandwidth = cfr_params->bandwidth;
	param.capture_method = cfr_params->method;

	retv = wmi_unified_send_peer_cfr_capture_cmd(pdev_wmi_handle, &param);
	return retv;
}

int target_if_cfr_pdev_set_param(struct wlan_objmgr_pdev *pdev,
				 uint32_t param_id, uint32_t param_value)
{
	struct pdev_params pparam;
	uint32_t pdev_id;

	pdev_id = wlan_objmgr_pdev_get_pdev_id(pdev);
	if (pdev_id < 0)
		return -EINVAL;

	qdf_mem_set(&pparam, sizeof(pparam), 0);
	pparam.param_id = param_id;
	pparam.param_value = param_value;

	return wmi_unified_pdev_param_send(lmac_get_pdev_wmi_handle(pdev),
					   &pparam, pdev_id);
}

int target_if_cfr_enable_cfr_timer(struct wlan_objmgr_pdev *pdev,
				   uint32_t cfr_timer)
{
	struct pdev_cfr *pa;
	int retval;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (pa == NULL)
		return QDF_STATUS_E_FAILURE;

	if (!cfr_timer) {
	     /* disable periodic cfr capture */
		retval =
	target_if_cfr_pdev_set_param(pdev,
				     wmi_pdev_param_per_peer_prd_cfr_enable,
				     WMI_HOST_PEER_CFR_TIMER_DISABLE);

		if (retval == QDF_STATUS_SUCCESS)
			pa->cfr_timer_enable = 0;
	} else {
	    /* enable periodic cfr capture (default base timer is 10ms ) */
		retval =
	target_if_cfr_pdev_set_param(pdev,
				     wmi_pdev_param_per_peer_prd_cfr_enable,
				     WMI_HOST_PEER_CFR_TIMER_ENABLE);

		if (retval == QDF_STATUS_SUCCESS)
			pa->cfr_timer_enable = 1;
	}

	return retval;
}

int target_if_cfr_get_target_type(struct wlan_objmgr_psoc *psoc)
{
	uint32_t target_type = 0;
	struct wlan_lmac_if_target_tx_ops *target_type_tx_ops;

	target_type_tx_ops = &psoc->soc_cb.tx_ops.target_tx_ops;

	if (target_type_tx_ops->tgt_get_tgt_type)
		target_type = target_type_tx_ops->tgt_get_tgt_type(psoc);

	return target_type;
}

int target_if_cfr_init_pdev(struct wlan_objmgr_psoc *psoc,
			    struct wlan_objmgr_pdev *pdev)
{
	uint32_t target_type;
	struct pdev_cfr *pa;
	struct psoc_cfr *cfr_sc;

	pa = wlan_objmgr_pdev_get_comp_private_obj(pdev, WLAN_UMAC_COMP_CFR);
	if (pa == NULL)
		return QDF_STATUS_E_FAILURE;

	cfr_sc = wlan_objmgr_psoc_get_comp_private_obj(psoc,
						       WLAN_UMAC_COMP_CFR);

	if (cfr_sc == NULL)
		return QDF_STATUS_E_FAILURE;

	target_type = target_if_cfr_get_target_type(psoc);

	if (target_type == TARGET_TYPE_QCA8074V2) {
		pa->is_cfr_capable = cfr_sc->is_cfr_capable;
		return cfr_8074v2_init_pdev(psoc, pdev);
	} else if ((target_type == TARGET_TYPE_IPQ4019) ||
		   (target_type == TARGET_TYPE_QCA9984) ||
		   (target_type == TARGET_TYPE_QCA9888)) {

		pa->is_cfr_capable = cfr_sc->is_cfr_capable;

		return cfr_wifi2_0_init_pdev(psoc, pdev);
	} else
		return QDF_STATUS_E_NOSUPPORT;
}

int target_if_cfr_deinit_pdev(struct wlan_objmgr_psoc *psoc,
			      struct wlan_objmgr_pdev *pdev)
{
	uint32_t target_type;

	target_type = target_if_cfr_get_target_type(psoc);

	if (target_type == TARGET_TYPE_QCA8074V2) {
		return cfr_8074v2_deinit_pdev(psoc, pdev);
	} else if ((target_type == TARGET_TYPE_IPQ4019) ||
		   (target_type == TARGET_TYPE_QCA9984) ||
		   (target_type == TARGET_TYPE_QCA9888)) {

		return cfr_wifi2_0_deinit_pdev(psoc, pdev);
	} else
		return QDF_STATUS_E_NOSUPPORT;
}

void target_if_cfr_tx_ops_register(struct wlan_lmac_if_tx_ops *tx_ops)
{
	tx_ops->cfr_tx_ops.cfr_init_pdev =
		target_if_cfr_init_pdev;
	tx_ops->cfr_tx_ops.cfr_deinit_pdev =
		target_if_cfr_deinit_pdev;
	tx_ops->cfr_tx_ops.cfr_enable_cfr_timer =
		target_if_cfr_enable_cfr_timer;
	tx_ops->cfr_tx_ops.cfr_start_capture =
		target_if_cfr_start_capture;
	tx_ops->cfr_tx_ops.cfr_stop_capture =
		target_if_cfr_stop_capture;
}

void target_if_cfr_set_cfr_support(struct wlan_objmgr_psoc *psoc,
				   uint8_t value)
{
	if (psoc->soc_cb.rx_ops.cfr_rx_ops.cfr_support_set)
		psoc->soc_cb.rx_ops.cfr_rx_ops.cfr_support_set(psoc, value);
}

void target_if_cfr_info_send(struct wlan_objmgr_pdev *pdev, void *head,
			     size_t hlen, void *data, size_t dlen, void *tail,
			     size_t tlen)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);

	if (psoc->soc_cb.rx_ops.cfr_rx_ops.cfr_info_send)
		psoc->soc_cb.rx_ops.cfr_rx_ops.cfr_info_send(pdev, head, hlen,
							     data, dlen, tail,
							     tlen);
}
