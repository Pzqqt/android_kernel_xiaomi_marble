/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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
 * DOC: wma_nan_datapath.c
 *
 * WMA NAN Data path API implementation
 */

#include "wma.h"
#include "wma_api.h"
#include "wmi_unified_api.h"
#include "wmi_unified.h"
#include "wma_nan_datapath.h"
#include "wma_internal.h"
#include "cds_utils.h"
#include "cdp_txrx_peer_ops.h"
#include "cdp_txrx_tx_delay.h"
#include "cdp_txrx_misc.h"
#include <cdp_txrx_handle.h>

/**
 * wma_add_bss_ndi_mode() - Process BSS creation request while adding NaN
 * Data interface
 * @wma: wma handle
 * @add_bss: Parameters for ADD_BSS command
 *
 * Sends VDEV_START command to firmware
 * Return: None
 */
void wma_add_bss_ndi_mode(tp_wma_handle wma, tpAddBssParams add_bss)
{
	struct cdp_pdev *pdev;
	struct cdp_vdev *vdev;
	struct wma_vdev_start_req req;
	void *peer = NULL;
	struct wma_target_req *msg;
	uint8_t vdev_id, peer_id;
	QDF_STATUS status;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	WMA_LOGD("%s: enter", __func__);
	vdev = wma_find_vdev_by_addr(wma, add_bss->bssId, &vdev_id);
	if (!vdev) {
		WMA_LOGE("%s: Failed to find vdev", __func__);
		goto send_fail_resp;
	}
	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (!pdev) {
		WMA_LOGE("%s: Failed to get pdev", __func__);
		goto send_fail_resp;
	}

	wma_set_bss_rate_flags(wma, vdev_id, add_bss);

	status = wma_create_peer(wma, pdev, vdev, add_bss->selfMacAddr,
				 WMI_PEER_TYPE_DEFAULT, vdev_id, false);
	if (QDF_IS_STATUS_ERROR(status)) {
		WMA_LOGE("%s: Failed to create peer", __func__);
		goto send_fail_resp;
	}

	peer = cdp_peer_find_by_addr(soc, pdev, add_bss->selfMacAddr, &peer_id);
	if (!peer) {
		WMA_LOGE("%s Failed to find peer %pM", __func__,
			 add_bss->selfMacAddr);
		goto send_fail_resp;
	}

	msg = wma_fill_vdev_req(wma, vdev_id, WMA_ADD_BSS_REQ,
			WMA_TARGET_REQ_TYPE_VDEV_START, add_bss,
			WMA_VDEV_START_REQUEST_TIMEOUT);
	if (!msg) {
		WMA_LOGE("%s Failed to allocate vdev request vdev_id %d",
			 __func__, vdev_id);
		goto send_fail_resp;
	}

	add_bss->staContext.staIdx = cdp_peer_get_local_peer_id(soc, peer);

	/*
	 * beacon_intval, dtim_period, hidden_ssid, is_dfs, ssid
	 * will be ignored for NDI device.
	 */
	qdf_mem_zero(&req, sizeof(req));
	req.vdev_id = vdev_id;
	req.chan = add_bss->currentOperChannel;
	req.ch_center_freq_seg0 = add_bss->ch_center_freq_seg0;
	req.ch_center_freq_seg1 = add_bss->ch_center_freq_seg1;
	req.vht_capable = add_bss->vhtCapable;
	req.max_txpow = add_bss->maxTxPower;
	req.oper_mode = add_bss->operMode;

	status = wma_vdev_start(wma, &req, false);
	if (status != QDF_STATUS_SUCCESS) {
		wma_remove_vdev_req(wma, vdev_id,
			WMA_TARGET_REQ_TYPE_VDEV_START);
		goto send_fail_resp;
	}
	WMA_LOGD("%s: vdev start request for NDI sent to target", __func__);

	/* Initialize protection mode to no protection */
	wma_vdev_set_param(wma->wmi_handle, vdev_id,
		WMI_VDEV_PARAM_PROTECTION_MODE, IEEE80211_PROT_NONE);

	return;

send_fail_resp:
	add_bss->status = QDF_STATUS_E_FAILURE;
	wma_send_msg_high_priority(wma, WMA_ADD_BSS_RSP, (void *)add_bss, 0);
}

/**
 * wma_add_sta_ndi_mode() - Process ADD_STA for NaN Data path
 * @wma: wma handle
 * @add_sta: Parameters of ADD_STA command
 *
 * Sends CREATE_PEER command to firmware
 * Return: void
 */
void wma_add_sta_ndi_mode(tp_wma_handle wma, tpAddStaParams add_sta)
{
	enum ol_txrx_peer_state state = OL_TXRX_PEER_STATE_CONN;
	struct cdp_pdev *pdev;
	struct cdp_vdev *vdev;
	void *peer;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);
	u_int8_t peer_id;
	QDF_STATUS status;
	struct wma_txrx_node *iface;

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);

	if (NULL == pdev) {
		WMA_LOGE(FL("Failed to find pdev"));
		add_sta->status = QDF_STATUS_E_FAILURE;
		goto send_rsp;
	}

	vdev = wma_find_vdev_by_id(wma, add_sta->smesessionId);
	if (!vdev) {
		WMA_LOGE(FL("Failed to find vdev"));
		add_sta->status = QDF_STATUS_E_FAILURE;
		goto send_rsp;
	}

	iface = &wma->interfaces[cdp_get_vdev_id(soc, vdev)];
	WMA_LOGD(FL("vdev: %d, peer_mac_addr: "MAC_ADDRESS_STR),
		add_sta->smesessionId, MAC_ADDR_ARRAY(add_sta->staMac));

	peer = cdp_peer_find_by_addr_and_vdev(soc,
			pdev, vdev,
			add_sta->staMac, &peer_id);
	if (peer) {
		WMA_LOGE(FL("NDI peer already exists, peer_addr %pM"),
			 add_sta->staMac);
		add_sta->status = QDF_STATUS_E_EXISTS;
		goto send_rsp;
	}

	/*
	 * The code above only checks the peer existence on its own vdev.
	 * Need to check whether the peer exists on other vDevs because firmware
	 * can't create the peer if the peer with same MAC address already
	 * exists on the pDev. As this peer belongs to other vDevs, just return
	 * here.
	 */
	peer = cdp_peer_find_by_addr(soc,
			pdev,
			add_sta->staMac, &peer_id);
	if (peer) {
		WMA_LOGE(FL("vdev:%d, peer exists on other vdev with peer_addr %pM and peer_id %d"),
			cdp_get_vdev_id(soc, vdev),
			add_sta->staMac, peer_id);
		add_sta->status = QDF_STATUS_E_EXISTS;
		goto send_rsp;
	}

	status = wma_create_peer(wma, pdev, vdev, add_sta->staMac,
				 WMI_PEER_TYPE_NAN_DATA, add_sta->smesessionId,
				 false);
	if (status != QDF_STATUS_SUCCESS) {
		WMA_LOGE(FL("Failed to create peer for %pM"), add_sta->staMac);
		add_sta->status = status;
		goto send_rsp;
	}

	peer = cdp_peer_find_by_addr_and_vdev(soc,
			pdev, vdev,
			add_sta->staMac, &peer_id);
	if (!peer) {
		WMA_LOGE(FL("Failed to find peer handle using peer mac %pM"),
			 add_sta->staMac);
		add_sta->status = QDF_STATUS_E_FAILURE;
		wma_remove_peer(wma, add_sta->staMac, add_sta->smesessionId,
				peer, false);
		goto send_rsp;
	}

	WMA_LOGD(FL("Moving peer %pM to state %d"), add_sta->staMac, state);
	cdp_peer_state_update(soc, pdev, add_sta->staMac, state);

	add_sta->staIdx = cdp_peer_get_local_peer_id(soc, peer);
	add_sta->nss    = iface->nss;
	add_sta->status = QDF_STATUS_SUCCESS;
send_rsp:
	WMA_LOGD(FL("Sending add sta rsp to umac (mac:%pM, status:%d)"),
		 add_sta->staMac, add_sta->status);
	wma_send_msg_high_priority(wma, WMA_ADD_STA_RSP, (void *)add_sta, 0);
}

/**
 * wma_delete_sta_req_ndi_mode() - Process DEL_STA request for NDI data peer
 * @wma: WMA context
 * @del_sta: DEL_STA parameters from LIM
 *
 * Removes wma/txrx peer entry for the NDI STA
 *
 * Return: None
 */
void wma_delete_sta_req_ndi_mode(tp_wma_handle wma,
					tpDeleteStaParams del_sta)
{
	struct cdp_pdev *pdev;
	void *peer;
	void *soc = cds_get_context(QDF_MODULE_ID_SOC);

	pdev = cds_get_context(QDF_MODULE_ID_TXRX);
	if (!pdev) {
		WMA_LOGE(FL("Failed to get pdev"));
		del_sta->status = QDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}

	peer = cdp_peer_find_by_local_id(cds_get_context(QDF_MODULE_ID_SOC),
			pdev, del_sta->staIdx);
	if (!peer) {
		WMA_LOGE(FL("Failed to get peer handle using peer id %d"),
			 del_sta->staIdx);
		del_sta->status = QDF_STATUS_E_FAILURE;
		goto send_del_rsp;
	}

	wma_remove_peer(wma, cdp_peer_get_peer_mac_addr(soc, peer),
			del_sta->smesessionId, peer, false);
	del_sta->status = QDF_STATUS_SUCCESS;

send_del_rsp:
	if (del_sta->respReqd) {
		WMA_LOGD(FL("Sending del rsp to umac (status: %d)"),
				del_sta->status);
		wma_send_msg_high_priority(wma, WMA_DELETE_STA_RSP, del_sta, 0);
	} else {
		WMA_LOGD(FL("NDI Del Sta resp not needed"));
		qdf_mem_free(del_sta);
	}

}
