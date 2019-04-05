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

/**
 * DOC: defines DP interaction with FW using WMI
 */

#include <qdf_status.h>
#include "target_if_dp.h"
#include <init_deinit_lmac.h>

void
target_if_peer_set_default_routing(struct cdp_ctrl_objmgr_pdev *pdev,
				   uint8_t *peer_macaddr, uint8_t vdev_id,
				   bool hash_based, uint8_t ring_num)
{
	uint32_t value;
	struct peer_set_params param;
	struct common_wmi_handle *pdev_wmi_handle;

	pdev_wmi_handle =
		lmac_get_pdev_wmi_handle((struct wlan_objmgr_pdev *)pdev);
	if (!pdev_wmi_handle) {
		target_if_err("pdev wmi handle NULL");
		return;
	}

	qdf_mem_zero(&param, sizeof(param));

	/* TODO: Need bit definitions for ring number and hash based routing
	 * fields in common wmi header file
	 */
	value = ((hash_based) ? 1 : 0) | (ring_num << 1);

	param.param_id = WMI_HOST_PEER_SET_DEFAULT_ROUTING;
	param.vdev_id = vdev_id;
	param.param_value = value;

	if (wmi_set_peer_param_send(pdev_wmi_handle, peer_macaddr, &param)) {
		target_if_err("Unable to set default routing for peer "
				QDF_MAC_ADDR_STR,
				QDF_MAC_ADDR_ARRAY(peer_macaddr));
	}
}

#ifdef SERIALIZE_QUEUE_SETUP
static QDF_STATUS
target_if_rx_reorder_queue_setup(struct scheduler_msg *msg)
{
	struct rx_reorder_queue_setup_params param;
	struct common_wmi_handle *pdev_wmi_handle;
	struct reorder_q_setup *q_params;
	struct cdp_ctrl_objmgr_pdev *pdev;
	QDF_STATUS status;

	if (!(msg->bodyptr)) {
		target_if_err("rx_reorder: Invalid message body");
		return QDF_STATUS_E_INVAL;
	}

	q_params = msg->bodyptr;
	pdev = q_params->pdev;
	pdev_wmi_handle =
		lmac_get_pdev_wmi_handle((struct wlan_objmgr_pdev *)pdev);
	if (!pdev_wmi_handle) {
		target_if_err("pdev wmi handle NULL");
		status = QDF_STATUS_E_FAILURE;
		goto out;
	}

	param.tid = q_params->tid;
	param.vdev_id = q_params->vdev_id;
	param.peer_macaddr = q_params->peer_mac;
	param.hw_qdesc_paddr_lo = q_params->hw_qdesc_paddr & 0xffffffff;
	param.hw_qdesc_paddr_hi = (uint64_t)q_params->hw_qdesc_paddr >> 32;
	param.queue_no = q_params->queue_no;
	param.ba_window_size_valid = q_params->ba_window_size_valid;
	param.ba_window_size = q_params->ba_window_size;

	status = wmi_unified_peer_rx_reorder_queue_setup_send(pdev_wmi_handle,
							      &param);
out:
	qdf_mem_free(q_params);

	return status;
}

QDF_STATUS
target_if_peer_rx_reorder_queue_setup(struct cdp_ctrl_objmgr_pdev *pdev,
				      uint8_t vdev_id, uint8_t *peer_macaddr,
				      qdf_dma_addr_t hw_qdesc, int tid,
				      uint16_t queue_no,
				      uint8_t ba_window_size_valid,
				      uint16_t ba_window_size)
{
	struct scheduler_msg msg = {0};
	struct reorder_q_setup *q_params;
	QDF_STATUS status;

	q_params = qdf_mem_malloc(sizeof(*q_params));
	if (!q_params)
		return QDF_STATUS_E_NOMEM;

	q_params->pdev = pdev;
	q_params->vdev_id = vdev_id;
	q_params->hw_qdesc_paddr = hw_qdesc;
	q_params->tid = tid;
	q_params->queue_no = queue_no;
	q_params->ba_window_size_valid = ba_window_size_valid;
	q_params->ba_window_size = ba_window_size;
	qdf_mem_copy(q_params->peer_mac, peer_macaddr, QDF_MAC_ADDR_SIZE);

	msg.bodyptr = q_params;
	msg.callback = target_if_rx_reorder_queue_setup;
	status = scheduler_post_message(QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF,
					QDF_MODULE_ID_TARGET_IF, &msg);

	if (status != QDF_STATUS_SUCCESS)
		qdf_mem_free(q_params);

	return status;
}

#else

QDF_STATUS
target_if_peer_rx_reorder_queue_setup(struct cdp_ctrl_objmgr_pdev *pdev,
				      uint8_t vdev_id, uint8_t *peer_macaddr,
				      qdf_dma_addr_t hw_qdesc, int tid,
				      uint16_t queue_no,
				      uint8_t ba_window_size_valid,
				      uint16_t ba_window_size)
{
	struct rx_reorder_queue_setup_params param;
	struct common_wmi_handle *pdev_wmi_handle;

	pdev_wmi_handle =
		lmac_get_pdev_wmi_handle((struct wlan_objmgr_pdev *)pdev);
	if (!pdev_wmi_handle) {
		target_if_err("pdev wmi handle NULL");
		return QDF_STATUS_E_FAILURE;
	}
	param.tid = tid;
	param.vdev_id = vdev_id;
	param.peer_macaddr = peer_macaddr;
	param.hw_qdesc_paddr_lo = hw_qdesc & 0xffffffff;
	param.hw_qdesc_paddr_hi = (uint64_t)hw_qdesc >> 32;
	param.queue_no = queue_no;
	param.ba_window_size_valid = ba_window_size_valid;
	param.ba_window_size = ba_window_size;

	return wmi_unified_peer_rx_reorder_queue_setup_send(pdev_wmi_handle,
							    &param);
}
#endif

QDF_STATUS
target_if_peer_rx_reorder_queue_remove(struct cdp_ctrl_objmgr_pdev *pdev,
				       uint8_t vdev_id, uint8_t *peer_macaddr,
				       uint32_t peer_tid_bitmap)
{
	struct rx_reorder_queue_remove_params param;
	struct common_wmi_handle *pdev_wmi_handle;

	pdev_wmi_handle =
		lmac_get_pdev_wmi_handle((struct wlan_objmgr_pdev *)pdev);
	if (!pdev_wmi_handle) {
		target_if_err("pdev wmi handle NULL");
		return QDF_STATUS_E_FAILURE;
	}
	param.vdev_id = vdev_id;
	param.peer_macaddr = peer_macaddr;
	param.peer_tid_bitmap = peer_tid_bitmap;
	return wmi_unified_peer_rx_reorder_queue_remove_send(pdev_wmi_handle,
							     &param);
}

QDF_STATUS
target_if_lro_hash_config(struct cdp_ctrl_objmgr_pdev *pdev,
			  struct cdp_lro_hash_config *lro_hash_cfg)
{
	struct wmi_lro_config_cmd_t wmi_lro_cmd = {0};
	struct common_wmi_handle *pdev_wmi_handle;

	pdev_wmi_handle =
		lmac_get_pdev_wmi_handle((struct wlan_objmgr_pdev *)pdev);
	if (!lro_hash_cfg || !pdev_wmi_handle) {
		target_if_err("wmi_handle: 0x%pK, lro_hash_cfg: 0x%pK",
			      pdev_wmi_handle, lro_hash_cfg);
		return QDF_STATUS_E_FAILURE;
	}

	wmi_lro_cmd.lro_enable = lro_hash_cfg->lro_enable;
	wmi_lro_cmd.tcp_flag = lro_hash_cfg->tcp_flag;
	wmi_lro_cmd.tcp_flag_mask = lro_hash_cfg->tcp_flag_mask;
	wmi_lro_cmd.pdev_id =
		lmac_get_pdev_idx((struct wlan_objmgr_pdev *)pdev);

	qdf_mem_copy(wmi_lro_cmd.toeplitz_hash_ipv4,
		     lro_hash_cfg->toeplitz_hash_ipv4,
		     LRO_IPV4_SEED_ARR_SZ * sizeof(uint32_t));

	qdf_mem_copy(wmi_lro_cmd.toeplitz_hash_ipv6,
		     lro_hash_cfg->toeplitz_hash_ipv6,
		     LRO_IPV6_SEED_ARR_SZ * sizeof(uint32_t));

	return wmi_unified_lro_config_cmd(pdev_wmi_handle,
					  &wmi_lro_cmd);
}
