/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
 *
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

#include <target_if_son.h>
#include <target_if.h>
#include <wlan_lmac_if_def.h>
#include <wmi_unified_api.h>
#include <cdp_txrx_ctrl.h>

#if QCA_SUPPORT_SON

bool son_ol_is_peer_inact(struct wlan_objmgr_peer *peer)
{
	struct wlan_objmgr_vdev *vdev;
	struct wlan_objmgr_psoc *psoc;

	vdev = wlan_peer_get_vdev(peer);
	if (!vdev)
		return false;

	psoc = wlan_vdev_get_psoc(vdev);
	if (!psoc)
		return false;

	return cdp_peer_is_inact(wlan_psoc_get_dp_handle(psoc),
				 (void *)(wlan_peer_get_dp_handle(peer)));
}

u_int32_t son_ol_get_peer_rate(struct wlan_objmgr_peer *peer, u_int8_t type)
{
	return ol_if_peer_get_rate(peer, type);
}


bool son_ol_enable(struct wlan_objmgr_pdev *pdev, bool enable)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc)
		return false;

	return cdp_start_inact_timer(wlan_psoc_get_dp_handle(psoc),
				(void *)(wlan_pdev_get_dp_handle(pdev)),
				enable);
}

/* Function pointer to set overload status */
void son_ol_set_overload(struct wlan_objmgr_pdev *pdev, bool overload)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);

	return cdp_set_overload(wlan_psoc_get_dp_handle(psoc),
				(void *)(wlan_pdev_get_dp_handle(pdev)),
				overload);
}
/* Function pointer to set band steering parameters */
bool son_ol_set_params(struct wlan_objmgr_pdev *pdev,
			    u_int32_t inactivity_check_period,
			    u_int32_t inactivity_threshold_normal,
			    u_int32_t inactivity_threshold_overload)
{
	struct wlan_objmgr_psoc *psoc;

	psoc = wlan_pdev_get_psoc(pdev);
	return cdp_set_inact_params(wlan_psoc_get_dp_handle(psoc),
				(void *)wlan_pdev_get_dp_handle(pdev),
				inactivity_check_period,
				inactivity_threshold_normal,
				inactivity_threshold_overload);
}

int8_t son_ol_sanitize_util_invtl(struct wlan_objmgr_pdev *pdev,
				  u_int32_t *sample_period,
				  u_int32_t *num_of_sample)
{
	return EOK;
}

QDF_STATUS son_ol_send_null(struct wlan_objmgr_pdev *pdev,
			 u_int8_t *macaddr,
			 struct wlan_objmgr_vdev *vdev)
{
	struct stats_request_params param = {0};
	struct wlan_objmgr_psoc *psoc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);

	if (!psoc)
		return QDF_STATUS_E_FAILURE;

	param.vdev_id = wlan_vdev_get_id(vdev);
	param.stats_id = WMI_HOST_REQUEST_INST_STAT;

	return wmi_unified_stats_request_send(GET_WMI_HDL_FROM_PSOC(psoc),
					macaddr, &param);
}

int son_ol_lmac_create(struct wlan_objmgr_pdev *pdev)
{
	return EOK;
}

int son_ol_lmac_destroy(struct wlan_objmgr_pdev *pdev)
{
	return EOK;

}

void  son_ol_rx_rssi_update(struct wlan_objmgr_pdev *pdev, u_int8_t *macaddres,
			    u_int8_t status, int8_t rssi, u_int8_t subtype)
{
	return;

}

void son_ol_rx_rate_update(struct wlan_objmgr_pdev *pdev, u_int8_t *macaddres,
			   u_int8_t status, u_int32_t rateKbps)
{
	return;
}

void target_if_son_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	/* wlan son related function handler */
	tx_ops->son_tx_ops.son_enable = son_ol_enable;
	tx_ops->son_tx_ops.set_overload = son_ol_set_overload;
	tx_ops->son_tx_ops.set_params = son_ol_set_params;
	tx_ops->son_tx_ops.lmac_create = son_ol_lmac_create;
	tx_ops->son_tx_ops.lmac_destroy = son_ol_lmac_destroy;
	tx_ops->son_tx_ops.son_send_null = son_ol_send_null;
	tx_ops->son_tx_ops.son_rssi_update = son_ol_rx_rssi_update;
	tx_ops->son_tx_ops.son_rate_update = son_ol_rx_rate_update;
	tx_ops->son_tx_ops.son_sanity_util_intvl = son_ol_sanitize_util_invtl;
	tx_ops->son_tx_ops.get_peer_rate = son_ol_get_peer_rate;
	tx_ops->son_tx_ops.son_node_isinact = son_ol_is_peer_inact;
	return;
}
#else
void target_if_son_register_tx_ops(struct wlan_lmac_if_tx_ops *tx_ops)
{
	return;
}
int8_t son_ol_sanitize_util_intvl(struct wlan_objmgr_pdev *pdev,
				  u_int32_t *sample_period,
				  u_int32_t *num_of_sample)
{
	return -EINVAL;

}

u_int32_t son_ol_get_peer_rate(struct wlan_objmgr_peer *peer, u_int8_t type)
{
	return 0;
}


bool son_ol_enable(struct wlan_objmgr_pdev *pdev, bool enable)
{
	return -EINVAL;

}


/* Function pointer to set overload status */

void son_ol_set_overload(struct wlan_objmgr_pdev *pdev, bool overload)
{
	return;
}


/* Function pointer to set band steering parameters */

bool son_ol_set_params(struct wlan_objmgr_pdev *dev,
			     u_int32_t inactivity_check_period,
			     u_int32_t inactivity_threshold_normal,
			     u_int32_t inactivity_threshold_overload)
{
	return -EINVAL;
}



QDF_STATUS son_ol_send_null(struct wlan_objmgr_pdev *pdev,
			    u_int8_t *macaddr,
			    struct wlan_objmgr_vdev *vdev)
{
	return EOK;
}
int8_t son_ol_sanitize_util_invtl(struct wlan_objmgr_pdev *pdev,
				  u_int32_t *sample_period,
				  u_int32_t *num_of_sample)
{
	return EOK;
}

int son_ol_lmac_create(struct wlan_objmgr_pdev *pdev)
{
	return EOK;
}


int son_ol_lmac_destroy(struct wlan_objmgr_pdev *pdev)
{
	return EOK;

}


void  son_ol_rx_rssi_update(struct wlan_objmgr_pdev *pdev, u_int8_t *macaddres,
			    u_int8_t status, int8_t rssi, u_int8_t subtype)
{
	return;

}

void son_ol_rx_rate_update(struct wlan_objmgr_pdev *pdev, u_int8_t *macaddres,
			   u_int8_t status, u_int32_t rateKbps)
{
	return;
}

bool son_ol_is_peer_inact(struct wlan_objmgr_peer *peer)
{
	return false;
}
#endif
