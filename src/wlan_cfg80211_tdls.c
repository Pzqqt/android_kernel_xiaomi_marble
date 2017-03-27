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
 * DOC: defines driver functions interfacing with linux kernel
 */

#include <qdf_list.h>
#include <qdf_status.h>
#include <linux/wireless.h>
#include <linux/netdevice.h>
#include <net/cfg80211.h>
#include <wlan_cfg80211.h>
#include <wlan_cfg80211_tdls.h>
#include <wlan_osif_priv.h>
#include <wlan_tdls_public_structs.h>
#include <wlan_tdls_ucfg_api.h>
#include <qdf_mem.h>
#include <wlan_utility.h>
#include <wlan_reg_services_api.h>

#define MAX_CHANNEL (NUM_24GHZ_CHANNELS + NUM_5GHZ_CHANNELS)


#define TDLS_MAX_NO_OF_2_4_CHANNELS 14

QDF_STATUS wlan_cfg80211_tdls_priv_init(struct vdev_osif_priv *osif_priv)
{
	struct osif_tdls_vdev *tdls_priv;

	cfg80211_debug("initialize tdls os if layer private structure");
	tdls_priv = qdf_mem_malloc(sizeof(*tdls_priv));
	if (!tdls_priv) {
		cfg80211_err("failed to allocate memory for tdls_priv");
		return QDF_STATUS_E_NOMEM;
	}
	init_completion(&tdls_priv->tdls_add_peer_comp);
	init_completion(&tdls_priv->tdls_del_peer_comp);
	init_completion(&tdls_priv->tdls_mgmt_comp);
	init_completion(&tdls_priv->tdls_link_establish_req_comp);

	osif_priv->osif_tdls = tdls_priv;

	return QDF_STATUS_SUCCESS;
}

void wlan_cfg80211_tdls_priv_deinit(struct vdev_osif_priv *osif_priv)
{
	cfg80211_debug("deinitialize tdls os if layer private structure");
	if (osif_priv->osif_tdls)
		qdf_mem_free(osif_priv->osif_tdls);
	osif_priv->osif_tdls = NULL;
}

int wlan_cfg80211_tdls_add_peer(struct wlan_objmgr_pdev *pdev,
				struct net_device *dev, const uint8_t *mac)
{
	struct wlan_objmgr_vdev *vdev;
	struct tdls_add_peer_params *add_peer_req;
	int status;
	struct vdev_osif_priv *osif_priv;
	struct osif_tdls_vdev *tdls_priv;
	unsigned long rc;

	cfg80211_debug("Add TDLS peer " QDF_MAC_ADDRESS_STR,
		       QDF_MAC_ADDR_ARRAY(mac));
	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev(pdev, dev->dev_addr,
							 WLAN_OSIF_ID);
	if (vdev == NULL) {
		cfg80211_err("vdev object is NULL");
		return -EIO;
	}

	add_peer_req = qdf_mem_malloc(sizeof(*add_peer_req));
	if (!add_peer_req) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
		cfg80211_err("Failed to allocate tdls add peer request mem");
		return -EINVAL;
	}

	wlan_vdev_obj_lock(vdev);
	osif_priv = wlan_vdev_get_ospriv(vdev);
	tdls_priv = osif_priv->osif_tdls;

	add_peer_req->vdev_id = wlan_vdev_get_id(vdev);
	wlan_vdev_obj_unlock(vdev);

	qdf_mem_copy(add_peer_req->peer_addr, mac, QDF_MAC_ADDR_SIZE);

	reinit_completion(&tdls_priv->tdls_add_peer_comp);
	status = ucfg_tdls_add_peer(vdev, add_peer_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("ucfg_tdls_add_peer returned err %d", status);
		status = -EIO;
		goto error;
	}

	rc = wait_for_completion_timeout(
	    &tdls_priv->tdls_add_peer_comp,
	    msecs_to_jiffies(WAIT_TIME_TDLS_ADD_STA));
	if (!rc) {
		cfg80211_err("timeout for tdls add peer indication %ld", rc);
		status = -EPERM;
		goto error;
	}

	if (QDF_IS_STATUS_ERROR(tdls_priv->tdls_add_peer_status)) {
		cfg80211_err("tdls add peer failed, status:%d",
			     tdls_priv->tdls_add_peer_status);
		status = -EPERM;
	}
error:
	qdf_mem_free(add_peer_req);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
	return status;
}

static bool
is_duplicate_channel(uint8_t *arr, int index, uint8_t match)
{
	int i;

	for (i = 0; i < index; i++) {
		if (arr[i] == match)
			return true;
	}
	return false;
}

static void
tdls_calc_channels_from_staparams(struct tdls_update_peer_params *req_info,
				  struct station_parameters *params)
{
	int i = 0, j = 0, k = 0, no_of_channels = 0;
	int num_unique_channels;
	int next;
	uint8_t *dest_chans;
	const uint8_t *src_chans;

	dest_chans = req_info->supported_channels;
	src_chans = params->supported_channels;

	/* Convert (first channel , number of channels) tuple to
	 * the total list of channels. This goes with the assumption
	 * that if the first channel is < 14, then the next channels
	 * are an incremental of 1 else an incremental of 4 till the number
	 * of channels.
	 */
	for (i = 0; i < params->supported_channels_len &&
	     j < WLAN_MAC_MAX_SUPP_CHANNELS; i += 2) {
		int wifi_chan_index;

		if (!is_duplicate_channel(dest_chans, j, src_chans[i]))
			dest_chans[j] = src_chans[i];
		else
			continue;

		wifi_chan_index = ((dest_chans[j] <= WLAN_CHANNEL_14) ? 1 : 4);
		no_of_channels = src_chans[i + 1];

		cfg80211_debug("i:%d,j:%d,k:%d,[%d]:%d,index:%d,chans_num: %d",
			       i, j, k, j,
			       dest_chans[j],
			       wifi_chan_index,
			       no_of_channels);

		for (k = 1; k <= no_of_channels &&
		     j < WLAN_MAC_MAX_SUPP_CHANNELS - 1; k++) {
			next = dest_chans[j] + wifi_chan_index;

			if (!is_duplicate_channel(dest_chans, j + 1, next))
				dest_chans[j + 1] = next;
			else
				continue;

			cfg80211_debug("i: %d, j: %d, k: %d, [%d]: %d",
				       i, j, k, j + 1, dest_chans[j + 1]);
			j += 1;
		}
	}
	num_unique_channels = j + 1;
	cfg80211_debug("Unique Channel List: supported_channels ");
	for (i = 0; i < num_unique_channels; i++)
		cfg80211_debug("[%d]: %d,", i, dest_chans[i]);

	if (MAX_CHANNEL < num_unique_channels)
		num_unique_channels = MAX_CHANNEL;
	req_info->supported_channels_len = num_unique_channels;
	cfg80211_debug("After removing duplcates supported_channels_len: %d",
		       req_info->supported_channels_len);
}

static void
wlan_cfg80211_tdls_extract_params(struct tdls_update_peer_params *req_info,
				  struct station_parameters *params)
{
	int i;

	cfg80211_debug("sta cap %d, uapsd_queue %d, max_sp %d",
		       params->capability,
		       params->uapsd_queues, params->max_sp);

	if (!req_info) {
		cfg80211_err("reg_info is NULL");
		return;
	}
	req_info->capability = params->capability;
	req_info->uapsd_queues = params->uapsd_queues;
	req_info->max_sp = params->max_sp;

	if (params->supported_channels_len)
		tdls_calc_channels_from_staparams(req_info, params);

	qdf_mem_copy(req_info->supported_oper_classes,
		     params->supported_oper_classes,
		     params->supported_oper_classes_len);
	req_info->supported_oper_classes_len =
		params->supported_oper_classes_len;

	if (params->ext_capab_len)
		qdf_mem_copy(req_info->extn_capability, params->ext_capab,
			     sizeof(req_info->extn_capability));

	if (params->ht_capa) {
		req_info->htcap_present = 1;
		qdf_mem_copy(&req_info->ht_cap, params->ht_capa,
			     sizeof(struct htcap_cmn_ie));
	}

	req_info->supported_rates_len = params->supported_rates_len;

	/* Note : The Maximum sizeof supported_rates sent by the Supplicant is
	 * 32. The supported_rates array , for all the structures propogating
	 * till Add Sta to the firmware has to be modified , if the supplicant
	 * (ieee80211) is modified to send more rates.
	 */

	/* To avoid Data Currption , set to max length to SIR_MAC_MAX_SUPP_RATES
	 */
	if (req_info->supported_rates_len > WLAN_MAC_MAX_SUPP_RATES)
		req_info->supported_rates_len = WLAN_MAC_MAX_SUPP_RATES;

	if (req_info->supported_rates_len) {
		qdf_mem_copy(req_info->supported_rates,
			     params->supported_rates,
			     req_info->supported_rates_len);
		cfg80211_debug("Supported Rates with Length %d",
			       req_info->supported_rates_len);

		for (i = 0; i < req_info->supported_rates_len; i++)
			cfg80211_debug("[%d]: %0x", i,
				       req_info->supported_rates[i]);
	}

	if (params->vht_capa) {
		req_info->vhtcap_present = 1;
		qdf_mem_copy(&req_info->vht_cap, params->vht_capa,
			     sizeof(struct vhtcap));
	}

	if (params->ht_capa || params->vht_capa ||
	    (params->sta_flags_set & BIT(NL80211_STA_FLAG_WME)))
		req_info->is_qos_wmm_sta = true;
}

int wlan_cfg80211_tdls_update_peer(struct wlan_objmgr_pdev *pdev,
				   struct net_device *dev,
				   const uint8_t *mac,
				   struct station_parameters *params)
{
	struct wlan_objmgr_vdev *vdev;
	struct tdls_update_peer_params *req_info;
	int status;
	struct vdev_osif_priv *osif_priv;
	struct osif_tdls_vdev *tdls_priv;
	unsigned long rc;

	cfg80211_debug("Update TDLS peer " QDF_MAC_ADDRESS_STR,
		       QDF_MAC_ADDR_ARRAY(mac));
	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev(pdev, dev->dev_addr,
							 WLAN_OSIF_ID);
	if (vdev == NULL) {
		cfg80211_err("vdev object is NULL");
		return -EIO;
	}

	req_info = qdf_mem_malloc(sizeof(*req_info));
	if (!req_info) {
		wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
		cfg80211_err("Failed to allocate tdls add peer request mem");
		return -EINVAL;
	}
	wlan_cfg80211_tdls_extract_params(req_info, params);

	wlan_vdev_obj_lock(vdev);
	osif_priv = wlan_vdev_get_ospriv(vdev);
	tdls_priv = osif_priv->osif_tdls;

	req_info->vdev_id = wlan_vdev_get_id(vdev);
	wlan_vdev_obj_unlock(vdev);
	qdf_mem_copy(req_info->peer_addr, mac, QDF_MAC_ADDR_SIZE);

	reinit_completion(&tdls_priv->tdls_add_peer_comp);
	status = ucfg_tdls_update_peer(vdev, req_info);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("ucfg_tdls_update_peer returned err %d", status);
		status = -EIO;
		goto error;
	}

	rc = wait_for_completion_timeout(
		&tdls_priv->tdls_add_peer_comp,
		msecs_to_jiffies(WAIT_TIME_TDLS_ADD_STA));
	if (!rc) {
		cfg80211_err("timeout for tdls update peer indication %ld", rc);
		status = -EPERM;
		goto error;
	}

	if (QDF_IS_STATUS_ERROR(tdls_priv->tdls_add_peer_status)) {
		cfg80211_err("tdls update peer failed, status:%d",
			     tdls_priv->tdls_add_peer_status);
		status = -EPERM;
	}
error:
	qdf_mem_free(req_info);
	wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
	return status;
}

static char *tdls_oper_to_str(enum nl80211_tdls_operation oper)
{
	switch (oper) {
	case NL80211_TDLS_ENABLE_LINK:
		return "TDLS_ENABLE_LINK";
	case NL80211_TDLS_DISABLE_LINK:
		return "TDLS_DISABLE_LINK";
	case NL80211_TDLS_TEARDOWN:
		return "TDLS_TEARDOWN";
	case NL80211_TDLS_SETUP:
		return "TDLS_SETUP";
	default:
		return "UNKNOWN:ERR";
	}
}

static enum tdls_command_type tdls_oper_to_cmd(enum nl80211_tdls_operation oper)
{
	if (oper == NL80211_TDLS_ENABLE_LINK)
		return TDLS_CMD_ENABLE_LINK;
	else if (oper == NL80211_TDLS_DISABLE_LINK)
		return TDLS_CMD_DISABLE_LINK;
	else if (oper == NL80211_TDLS_TEARDOWN)
		return TDLS_CMD_REMOVE_FORCE_PEER;
	else if (oper == NL80211_TDLS_SETUP)
		return TDLS_CMD_CONFIG_FORCE_PEER;
	else
		return 0;
}

int wlan_cfg80211_tdls_configure_mode(struct wlan_objmgr_vdev *vdev,
						uint32_t trigger_mode)
{
	enum tdls_feature_mode tdls_mode;
	struct tdls_set_mode_params set_mode_params;
	int status;

	if (!vdev)
		return -EINVAL;

	switch (trigger_mode) {
	case WLAN_VENDOR_TDLS_TRIGGER_MODE_EXPLICIT:
		tdls_mode = TDLS_SUPPORT_IMP_MODE;
		break;
	case WLAN_VENDOR_TDLS_TRIGGER_MODE_EXTERNAL:
		tdls_mode = TDLS_SUPPORT_EXT_CONTROL;
		break;
	case WLAN_VENDOR_TDLS_TRIGGER_MODE_IMPLICIT:
		tdls_mode = TDLS_SUPPORT_IMP_MODE;
		break;
	default:
		cfg80211_err("Invalid TDLS trigger mode");
		return -EINVAL;
	}

	cfg80211_notice("cfg80211 tdls trigger mode %d", trigger_mode);
	set_mode_params.source = TDLS_SET_MODE_SOURCE_USER;
	set_mode_params.tdls_mode = tdls_mode;
	set_mode_params.update_last = false;
	set_mode_params.vdev = vdev;

	status = ucfg_tdls_set_operating_mode(&set_mode_params);
	return status;
}

int wlan_cfg80211_tdls_oper(struct wlan_objmgr_pdev *pdev,
			    struct net_device *dev,
			    const uint8_t *peer,
			    enum nl80211_tdls_operation oper)
{
	struct wlan_objmgr_vdev *vdev;
	struct vdev_osif_priv *osif_priv;
	struct osif_tdls_vdev *tdls_priv;
	int status;
	unsigned long rc;
	enum tdls_command_type cmd;

	if (NL80211_TDLS_DISCOVERY_REQ == oper) {
		cfg80211_warn(
			"We don't support in-driver setup/teardown/discovery");
		return -ENOTSUPP;
	}

	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev(pdev,
							 dev->dev_addr,
							 WLAN_OSIF_ID);
	if (vdev == NULL) {
		cfg80211_err("vdev object is NULL");
		return -EIO;
	}

	cfg80211_debug("%s start", tdls_oper_to_str(oper));
	cmd = tdls_oper_to_cmd(oper);
	switch (oper) {
	case NL80211_TDLS_ENABLE_LINK:
	case NL80211_TDLS_TEARDOWN:
	case NL80211_TDLS_SETUP:
		status = ucfg_tdls_oper(vdev, peer, cmd);
		if (QDF_IS_STATUS_ERROR(status)) {
			cfg80211_err("%s fail %d",
				     tdls_oper_to_str(oper), status);
			status = -EIO;
			goto error;
		}
		break;
	case NL80211_TDLS_DISABLE_LINK:
		wlan_vdev_obj_lock(vdev);
		osif_priv = wlan_vdev_get_ospriv(vdev);
		tdls_priv = osif_priv->osif_tdls;
		wlan_vdev_obj_unlock(vdev);
		reinit_completion(&tdls_priv->tdls_del_peer_comp);
		status = ucfg_tdls_oper(vdev, peer, cmd);
		if (QDF_IS_STATUS_ERROR(status)) {
			cfg80211_err("ucfg_tdls_disable_link fail %d", status);
			status = -EIO;
			goto error;
		}

		rc = wait_for_completion_timeout(
			&tdls_priv->tdls_del_peer_comp,
			msecs_to_jiffies(WAIT_TIME_TDLS_DEL_STA));
		if (!rc) {
			cfg80211_err("timeout for tdls disable link %ld", rc);
			status = -EPERM;
		}
		break;
	default:
		cfg80211_err("unsupported event %d", oper);
		status = -ENOTSUPP;
	}

error:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);

	return status;
}

void wlan_cfg80211_tdls_rx_callback(void *user_data,
	struct tdls_rx_mgmt_frame *rx_frame)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_vdev *vdev;
	struct vdev_osif_priv *osif_priv;
	struct wireless_dev *wdev;
	uint16_t freq;

	cfg80211_debug("user data:%p, vdev id:%d, rssi:%d, buf:%p, len:%d",
		user_data, rx_frame->vdev_id, rx_frame->rx_rssi,
		rx_frame->buf, rx_frame->frame_len);

	psoc = user_data;
	if (!psoc) {
		cfg80211_err("psoc is null");
		return;
	}

	vdev = wlan_objmgr_get_vdev_by_id_from_psoc(psoc,
		rx_frame->vdev_id, WLAN_TDLS_NB_ID);
	if (!vdev) {
		cfg80211_err("vdev is null");
		return;
	}

	wlan_vdev_obj_lock(vdev);
	osif_priv = wlan_vdev_get_ospriv(vdev);
	wlan_vdev_obj_unlock(vdev);
	if (!osif_priv) {
		cfg80211_err("osif_priv is null");
		goto fail;
	}

	wdev = osif_priv->wdev;
	if (!wdev) {
		cfg80211_err("wdev is null");
		goto fail;
	}

	if (rx_frame->rx_chan <= TDLS_MAX_NO_OF_2_4_CHANNELS)
		freq = ieee80211_channel_to_frequency(
			rx_frame->rx_chan, NL80211_BAND_2GHZ);
	else
		freq = ieee80211_channel_to_frequency(
			rx_frame->rx_chan, NL80211_BAND_5GHZ);

	cfg80211_notice("Indicate frame over nl80211, vdev id:%d, idx:%d",
		   rx_frame->vdev_id, wdev->netdev->ifindex);

#if (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 18, 0))
	cfg80211_rx_mgmt(wdev, freq, rx_frame->rx_rssi * 100,
		rx_frame->buf, rx_frame->frame_len,
		NL80211_RXMGMT_FLAG_ANSWERED);
#elif (LINUX_VERSION_CODE >= KERNEL_VERSION(3, 12, 0))
	cfg80211_rx_mgmt(wdev, freq, rx_frame->rx_rssi * 100,
		rx_frame->buf, rx_frame->frame_len,
		NL80211_RXMGMT_FLAG_ANSWERED, GFP_ATOMIC);
#else
	cfg80211_rx_mgmt(wdev, freq, rx_frame->rx_rssi * 100,
		rx_frame->buf, rx_frame->frame_len, GFP_ATOMIC);
#endif /* LINUX_VERSION_CODE */
fail:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_TDLS_NB_ID);
}

int wlan_cfg80211_tdls_mgmt(struct wlan_objmgr_pdev *pdev,
				struct net_device *dev, const uint8_t *peer_mac,
				uint8_t action_code, uint8_t dialog_token,
				uint16_t status_code, uint32_t peer_capability,
				const uint8_t *buf, size_t len)
{
	struct wlan_objmgr_vdev *vdev;
	struct tdls_action_frame_request mgmt_req;
	struct vdev_osif_priv *osif_priv;
	struct osif_tdls_vdev *tdls_priv;
	int status;
	unsigned long rc;
	int max_sta_failed = 0;
	struct tdls_validate_action_req chk_frame;
	struct tdls_set_responder_req set_responder;

	vdev = wlan_objmgr_get_vdev_by_macaddr_from_pdev(pdev,
							 dev->dev_addr,
							 WLAN_OSIF_ID);
	if (vdev == NULL) {
		cfg80211_err("vdev object is NULL");
		return -EIO;
	}

	wlan_vdev_obj_lock(vdev);
	osif_priv = wlan_vdev_get_ospriv(vdev);
	wlan_vdev_obj_unlock(vdev);

	tdls_priv = osif_priv->osif_tdls;

	/* make sure doesn't call send_mgmt() while it is pending */
	if (TDLS_VDEV_MAGIC == tdls_priv->mgmt_tx_completion_status) {
		cfg80211_err(QDF_MAC_ADDRESS_STR " action %d couldn't sent, as one is pending. return EBUSY",
			     QDF_MAC_ADDR_ARRAY(peer_mac), action_code);
		wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
		return -EBUSY;
	}

	/* Reset TDLS VDEV magic */
	tdls_priv->mgmt_tx_completion_status = TDLS_VDEV_MAGIC;


	/*prepare the request */

	/* Validate the management Request */
	chk_frame.vdev = vdev;
	chk_frame.action_code = action_code;
	qdf_mem_copy(chk_frame.peer_mac, peer_mac, QDF_MAC_ADDR_SIZE);
	chk_frame.dialog_token = dialog_token;
	chk_frame.action_code = action_code;
	chk_frame.status_code = status_code;
	chk_frame.len = len;
	chk_frame.max_sta_failed = max_sta_failed;

	mgmt_req.chk_frame = &chk_frame;

	mgmt_req.vdev = vdev;
	mgmt_req.vdev_id = wlan_vdev_get_id(vdev);
	mgmt_req.session_id = mgmt_req.vdev_id;
	/* populate management req params */
	qdf_mem_copy(mgmt_req.tdls_mgmt.peer_mac.bytes,
		     peer_mac, QDF_MAC_ADDR_SIZE);
	mgmt_req.tdls_mgmt.dialog = dialog_token;
	mgmt_req.tdls_mgmt.frame_type = action_code;
	mgmt_req.tdls_mgmt.len = len;
	mgmt_req.tdls_mgmt.peer_capability = peer_capability;
	mgmt_req.tdls_mgmt.status_code = chk_frame.status_code;

	/*populate the additional IE's */
	mgmt_req.cmd_buf = buf;
	mgmt_req.len = len;

	reinit_completion(&tdls_priv->tdls_mgmt_comp);
	status = ucfg_tdls_send_mgmt_frame(&mgmt_req);
	if (QDF_IS_STATUS_ERROR(status)) {
		cfg80211_err("ucfg_tdls_send_mgmt failed err %d", status);
		status = -EIO;
		tdls_priv->mgmt_tx_completion_status = false;
		goto error_mgmt_req;
	}

	cfg80211_info("Wait for tdls_mgmt_comp. Timeout %u ms",
		WAIT_TIME_FOR_TDLS_MGMT);

	rc = wait_for_completion_timeout(
		&tdls_priv->tdls_mgmt_comp,
		msecs_to_jiffies(WAIT_TIME_FOR_TDLS_MGMT));

	if ((0 == rc) || (true != tdls_priv->mgmt_tx_completion_status)) {
		cfg80211_err("%s rc %ld mgmtTxCompletionStatus %u",
			     !rc ? "Mgmt Tx Completion timed out" :
			     "Mgmt Tx Completion failed",
			     rc, tdls_priv->mgmt_tx_completion_status);

		tdls_priv->mgmt_tx_completion_status = false;
		status = -EINVAL;
		goto error_mgmt_req;
	}

	cfg80211_info("Mgmt Tx Completion status %ld TxCompletion %u",
		rc, tdls_priv->mgmt_tx_completion_status);

	if (chk_frame.max_sta_failed) {
		status = max_sta_failed;
		goto error_mgmt_req;
	}

	if (TDLS_SETUP_RESPONSE == action_code ||
	    TDLS_SETUP_CONFIRM == action_code) {
		qdf_mem_copy(set_responder.peer_mac, peer_mac,
			     QDF_MAC_ADDR_SIZE);
		set_responder.vdev = vdev;
		if (TDLS_SETUP_RESPONSE == action_code)
			set_responder.responder = false;
		if (TDLS_SETUP_CONFIRM == action_code)
			set_responder.responder = true;
		ucfg_tdls_responder(&set_responder);
	}

error_mgmt_req:
	wlan_objmgr_vdev_release_ref(vdev, WLAN_OSIF_ID);
	return status;
}

static void
wlan_cfg80211_tdls_indicate_discovery(struct tdls_osif_indication *ind)
{
	struct vdev_osif_priv *osif_vdev;

	wlan_vdev_obj_lock(ind->vdev);
	osif_vdev = wlan_vdev_get_ospriv(ind->vdev);
	wlan_vdev_obj_unlock(ind->vdev);

	cfg80211_info("Implicit TDLS, request Send Discovery request");
	cfg80211_tdls_oper_request(osif_vdev->wdev->netdev,
				   ind->peer_mac, NL80211_TDLS_DISCOVERY_REQ,
				   false, GFP_KERNEL);
}

static void
wlan_cfg80211_tdls_indicate_setup(struct tdls_osif_indication *ind)
{
	struct vdev_osif_priv *osif_vdev;

	wlan_vdev_obj_lock(ind->vdev);
	osif_vdev = wlan_vdev_get_ospriv(ind->vdev);
	wlan_vdev_obj_unlock(ind->vdev);

	cfg80211_info("Indication to request TDLS setup");
	cfg80211_tdls_oper_request(osif_vdev->wdev->netdev,
				   ind->peer_mac, NL80211_TDLS_SETUP, false,
				   GFP_KERNEL);
}

static void
wlan_cfg80211_tdls_indicate_teardown(struct tdls_osif_indication *ind)
{
	struct vdev_osif_priv *osif_vdev;

	wlan_vdev_obj_lock(ind->vdev);
	osif_vdev = wlan_vdev_get_ospriv(ind->vdev);
	wlan_vdev_obj_unlock(ind->vdev);

	cfg80211_info("Teardown reason %d", ind->reason);
	cfg80211_tdls_oper_request(osif_vdev->wdev->netdev,
				   ind->peer_mac, NL80211_TDLS_TEARDOWN,
				   ind->reason, GFP_KERNEL);
}

void wlan_cfg80211_tdls_event_callback(void *user_data,
				       enum tdls_event_type type,
				       struct tdls_osif_indication *ind)
{
	struct vdev_osif_priv *osif_vdev;
	struct osif_tdls_vdev *tdls_priv;

	if (!ind || !ind->vdev) {
		cfg80211_err("ind: %p", ind);
		return;
	}
	wlan_vdev_obj_lock(ind->vdev);
	osif_vdev = wlan_vdev_get_ospriv(ind->vdev);
	tdls_priv = osif_vdev->osif_tdls;
	wlan_vdev_obj_unlock(ind->vdev);

	switch (type) {
	case TDLS_EVENT_MGMT_TX_ACK_CNF:
		tdls_priv->mgmt_tx_completion_status = ind->status;
		complete(&tdls_priv->tdls_mgmt_comp);
		break;
	case TDLS_EVENT_ADD_PEER:
		tdls_priv->tdls_add_peer_status = ind->status;
		complete(&tdls_priv->tdls_add_peer_comp);
		break;
	case TDLS_EVENT_DEL_PEER:
		complete(&tdls_priv->tdls_del_peer_comp);
		break;
	case TDLS_EVENT_DISCOVERY_REQ:
		wlan_cfg80211_tdls_indicate_discovery(ind);
		break;
	case TDLS_EVENT_TEARDOWN_REQ:
		wlan_cfg80211_tdls_indicate_teardown(ind);
		break;
	case TDLS_EVENT_SETUP_REQ:
		wlan_cfg80211_tdls_indicate_setup(ind);
		break;
	default:
		break;
	}
}
