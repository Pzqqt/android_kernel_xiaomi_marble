/*
 * Copyright (c) 2016-2018, 2020, The Linux Foundation. All rights reserved.
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

#include <osdep.h>
#include "wmi.h"
#include "wmi_unified_priv.h"

/**
 * send_reset_passpoint_network_list_cmd_tlv() - reset passpoint network list
 * @wmi_handle: wmi handle
 * @req: passpoint network request structure
 *
 * This function sends down WMI command with network id set to wildcard id.
 * firmware shall clear all the config entries
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS send_reset_passpoint_network_list_cmd_tlv
					(wmi_unified_t wmi_handle,
					struct wifi_passpoint_req_param *req)
{
	wmi_passpoint_config_cmd_fixed_param *cmd;
	wmi_buf_t buf;
	uint32_t len;
	int ret;

	len = sizeof(*cmd);
	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("Failed allocate wmi buffer");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_passpoint_config_cmd_fixed_param *) wmi_buf_data(buf);

	WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_passpoint_config_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
			wmi_passpoint_config_cmd_fixed_param));
	cmd->id = WMI_PASSPOINT_NETWORK_ID_WILDCARD;

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
				   WMI_PASSPOINT_LIST_CONFIG_CMDID);
	if (ret) {
		wmi_err("Failed to send reset passpoint network list wmi cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * send_set_passpoint_network_list_cmd_tlv() - set passpoint network list
 * @wmi_handle: wmi handle
 * @req: passpoint network request structure
 *
 * This function reads the incoming @req and fill in the destination
 * WMI structure and send down the passpoint configs down to the firmware
 *
 * Return: QDF_STATUS enumeration
 */
static QDF_STATUS send_set_passpoint_network_list_cmd_tlv
					(wmi_unified_t wmi_handle,
					struct wifi_passpoint_req_param *req)
{
	wmi_passpoint_config_cmd_fixed_param *cmd;
	u_int8_t i, j, *bytes;
	wmi_buf_t buf;
	uint32_t len;
	int ret;

	len = sizeof(*cmd);
	for (i = 0; i < req->num_networks; i++) {
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			wmi_err("Failed allocate wmi buffer");
			return QDF_STATUS_E_NOMEM;
		}

		cmd = (wmi_passpoint_config_cmd_fixed_param *)
				wmi_buf_data(buf);

		WMITLV_SET_HDR(&cmd->tlv_header,
			WMITLV_TAG_STRUC_wmi_passpoint_config_cmd_fixed_param,
			WMITLV_GET_STRUCT_TLVLEN(
			wmi_passpoint_config_cmd_fixed_param));
		cmd->id = req->networks[i].id;
		wmi_debug("network id: %u", cmd->id);
		qdf_mem_copy(cmd->realm, req->networks[i].realm,
			strlen(req->networks[i].realm) + 1);
		wmi_debug("realm: %s", cmd->realm);
		for (j = 0; j < PASSPOINT_ROAMING_CONSORTIUM_ID_NUM; j++) {
			bytes = (uint8_t *) &req->networks[i].roaming_consortium_ids[j];
			wmi_debug("index: %d rcids: %02x %02x %02x %02x %02x %02x %02x %02x",
				j, bytes[0], bytes[1], bytes[2], bytes[3],
				bytes[4], bytes[5], bytes[6], bytes[7]);

			qdf_mem_copy(&cmd->roaming_consortium_ids[j],
				&req->networks[i].roaming_consortium_ids[j],
				PASSPOINT_ROAMING_CONSORTIUM_ID_LEN);
		}
		qdf_mem_copy(cmd->plmn, req->networks[i].plmn,
				PASSPOINT_PLMN_ID_LEN);
		wmi_debug("plmn: %02x:%02x:%02x",
			 cmd->plmn[0], cmd->plmn[1], cmd->plmn[2]);

		ret = wmi_unified_cmd_send(wmi_handle, buf, len,
					   WMI_PASSPOINT_LIST_CONFIG_CMDID);
		if (ret) {
			wmi_err("Failed to send set passpoint network list wmi cmd");
			wmi_buf_free(buf);
			return QDF_STATUS_E_FAILURE;
		}
	}

	return QDF_STATUS_SUCCESS;
}

/** send_set_epno_network_list_cmd_tlv() - set epno network list
 * @wmi_handle: wmi handle
 * @req: epno config params request structure
 *
 * This function reads the incoming epno config request structure
 * and constructs the WMI message to the firmware.
 *
 * Returns: 0 on success, error number otherwise
 */
static QDF_STATUS send_set_epno_network_list_cmd_tlv(wmi_unified_t wmi_handle,
		struct wifi_enhanced_pno_params *req)
{
	wmi_nlo_config_cmd_fixed_param *cmd;
	nlo_configured_parameters *nlo_list;
	enlo_candidate_score_params *cand_score_params;
	u_int8_t i, *buf_ptr;
	wmi_buf_t buf;
	uint32_t len;
	QDF_STATUS ret;

	/* Fixed Params */
	len = sizeof(*cmd);
	if (req->num_networks) {
		/* TLV place holder for array of structures
		 * then each nlo_configured_parameters(nlo_list) TLV.
		 */
		len += WMI_TLV_HDR_SIZE;
		len += (sizeof(nlo_configured_parameters)
			    * QDF_MIN(req->num_networks, WMI_NLO_MAX_SSIDS));
		/* TLV for array of uint32 channel_list */
		len += WMI_TLV_HDR_SIZE;
		/* TLV for nlo_channel_prediction_cfg */
		len += WMI_TLV_HDR_SIZE;
		/* TLV for candidate score params */
		len += sizeof(enlo_candidate_score_params);
	}

	buf = wmi_buf_alloc(wmi_handle, len);
	if (!buf) {
		wmi_err("Failed allocate wmi buffer");
		return QDF_STATUS_E_NOMEM;
	}

	cmd = (wmi_nlo_config_cmd_fixed_param *) wmi_buf_data(buf);

	buf_ptr = (u_int8_t *) cmd;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_nlo_config_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN(
			       wmi_nlo_config_cmd_fixed_param));
	cmd->vdev_id = req->vdev_id;

	/* set flag to reset if num of networks are 0 */
	cmd->flags = (req->num_networks == 0 ?
		WMI_NLO_CONFIG_ENLO_RESET : WMI_NLO_CONFIG_ENLO);

	buf_ptr += sizeof(wmi_nlo_config_cmd_fixed_param);

	cmd->no_of_ssids = QDF_MIN(req->num_networks, WMI_NLO_MAX_SSIDS);
	wmi_debug("SSID count: %d flags: %d",
		 cmd->no_of_ssids, cmd->flags);

	/* Fill nlo_config only when num_networks are non zero */
	if (cmd->no_of_ssids) {
		/* Fill networks */
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC,
			cmd->no_of_ssids * sizeof(nlo_configured_parameters));
		buf_ptr += WMI_TLV_HDR_SIZE;

		nlo_list = (nlo_configured_parameters *) buf_ptr;
		for (i = 0; i < cmd->no_of_ssids; i++) {
			WMITLV_SET_HDR(&nlo_list[i].tlv_header,
				WMITLV_TAG_ARRAY_BYTE,
				WMITLV_GET_STRUCT_TLVLEN(
				nlo_configured_parameters));
			/* Copy ssid and it's length */
			nlo_list[i].ssid.valid = true;
			nlo_list[i].ssid.ssid.ssid_len =
				req->networks[i].ssid.length;
			qdf_mem_copy(nlo_list[i].ssid.ssid.ssid,
				     req->networks[i].ssid.ssid,
				     nlo_list[i].ssid.ssid.ssid_len);
			wmi_debug("index: %d ssid: %.*s len: %d", i,
				 nlo_list[i].ssid.ssid.ssid_len,
				 (char *) nlo_list[i].ssid.ssid.ssid,
				 nlo_list[i].ssid.ssid.ssid_len);

			/* Copy pno flags */
			nlo_list[i].bcast_nw_type.valid = true;
			nlo_list[i].bcast_nw_type.bcast_nw_type =
					req->networks[i].flags;
			wmi_debug("PNO flags: %u",
				 nlo_list[i].bcast_nw_type.bcast_nw_type);

			/* Copy auth bit field */
			nlo_list[i].auth_type.valid = true;
			nlo_list[i].auth_type.auth_type =
					req->networks[i].auth_bit_field;
			wmi_debug("Auth bit field: %u",
				 nlo_list[i].auth_type.auth_type);
		}

		buf_ptr += cmd->no_of_ssids * sizeof(nlo_configured_parameters);
		/* Fill the channel list */
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_UINT32, 0);
		buf_ptr += WMI_TLV_HDR_SIZE;

		/* Fill prediction_param */
		WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_STRUC, 0);
		buf_ptr += WMI_TLV_HDR_SIZE;

		/* Fill epno candidate score params */
		cand_score_params = (enlo_candidate_score_params *) buf_ptr;
		WMITLV_SET_HDR(buf_ptr,
			WMITLV_TAG_STRUC_enlo_candidate_score_param,
			WMITLV_GET_STRUCT_TLVLEN(enlo_candidate_score_params));
		cand_score_params->min5GHz_rssi =
			req->min_5ghz_rssi;
		cand_score_params->min24GHz_rssi =
			req->min_24ghz_rssi;
		cand_score_params->initial_score_max =
			req->initial_score_max;
		cand_score_params->current_connection_bonus =
			req->current_connection_bonus;
		cand_score_params->same_network_bonus =
			req->same_network_bonus;
		cand_score_params->secure_bonus =
			req->secure_bonus;
		cand_score_params->band5GHz_bonus =
			req->band_5ghz_bonus;
		buf_ptr += sizeof(enlo_candidate_score_params);
	}

	ret = wmi_unified_cmd_send(wmi_handle, buf, len,
			WMI_NETWORK_LIST_OFFLOAD_CONFIG_CMDID);
	if (QDF_IS_STATUS_ERROR(ret)) {
		wmi_err("Failed to send nlo wmi cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_INVAL;
	}

	wmi_debug("set ePNO list request sent successfully for vdev %d",
		 req->vdev_id);

	return ret;
}

/**
 * send_extscan_get_capabilities_cmd_tlv() - extscan get capabilities
 * @wmi_handle: wmi handle
 * @pgetcapab: get capabilities params
 *
 * This function send request to fw to get extscan capabilities.
 *
 * Return: CDF status
 */
static QDF_STATUS send_extscan_get_capabilities_cmd_tlv(wmi_unified_t wmi_handle,
		    struct extscan_capabilities_params *pgetcapab)
{
	wmi_extscan_get_capabilities_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_extscan_get_capabilities_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	       WMITLV_TAG_STRUC_wmi_extscan_get_capabilities_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_extscan_get_capabilities_cmd_fixed_param));

	cmd->request_id = pgetcapab->request_id;

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_EXTSCAN_GET_CAPABILITIES_CMDID)) {
		wmi_err("Failed to send extscan get capabilities cmd");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_extscan_get_cached_results_cmd_tlv() - extscan get cached results
 * @wmi_handle: wmi handle
 * @pcached_results: cached results parameters
 *
 * This function send request to fw to get cached results.
 *
 * Return: CDF status
 */
static QDF_STATUS send_extscan_get_cached_results_cmd_tlv(wmi_unified_t wmi_handle,
		  struct extscan_cached_result_params *pcached_results)
{
	wmi_extscan_get_cached_results_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_extscan_get_cached_results_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		WMITLV_TAG_STRUC_wmi_extscan_get_cached_results_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_extscan_get_cached_results_cmd_fixed_param));

	cmd->request_id = pcached_results->request_id;
	cmd->vdev_id = pcached_results->vdev_id;
	cmd->control_flags = pcached_results->flush;

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_EXTSCAN_GET_CACHED_RESULTS_CMDID)) {
		wmi_err("failed to  command", __func__);
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_extscan_stop_change_monitor_cmd_tlv() - send stop change monitor cmd
 * @wmi_handle: wmi handle
 * @reset_req: Reset change request params
 *
 * This function sends stop change monitor request to fw.
 *
 * Return: CDF status
 */
static QDF_STATUS send_extscan_stop_change_monitor_cmd_tlv
			(wmi_unified_t wmi_handle,
			struct extscan_capabilities_reset_params *reset_req)
{
	wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;
	int change_list = 0;

	len = sizeof(*cmd);

	/* reset significant change tlv is set to 0 */
	len += WMI_TLV_HDR_SIZE;
	len += change_list * sizeof(wmi_extscan_wlan_change_bssid_param);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);

	cmd = (wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param *)
		buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param,
		WMITLV_GET_STRUCT_TLVLEN
		(wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param));

	cmd->request_id = reset_req->request_id;
	cmd->vdev_id = reset_req->vdev_id;
	cmd->mode = 0;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       change_list *
		       sizeof(wmi_extscan_wlan_change_bssid_param));
	buf_ptr += WMI_TLV_HDR_SIZE + (change_list *
				       sizeof
				       (wmi_extscan_wlan_change_bssid_param));

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
			 WMI_EXTSCAN_CONFIGURE_WLAN_CHANGE_MONITOR_CMDID)) {
		wmi_err("Failed to send extscan change monitor cmd");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_get_buf_extscan_change_monitor_cmd() - fill change monitor request
 * @wmi_handle: wmi handle
 * @psigchange: change monitor request params
 * @buf: wmi buffer
 * @buf_len: buffer length
 *
 * This function fills elements of change monitor request buffer.
 *
 * Return: QDF status
 */
static QDF_STATUS wmi_get_buf_extscan_change_monitor_cmd
			(wmi_unified_t wmi_handle,
			struct extscan_set_sig_changereq_params
			*psigchange, wmi_buf_t *buf, int *buf_len)
{
	wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param *cmd;
	wmi_extscan_wlan_change_bssid_param *dest_chglist;
	uint8_t *buf_ptr;
	int j;
	int len = sizeof(*cmd);
	uint32_t numap = psigchange->num_ap;
	struct ap_threshold_params *src_ap = psigchange->ap;

	if (!numap || (numap > WMI_WLAN_EXTSCAN_MAX_SIGNIFICANT_CHANGE_APS)) {
		wmi_err("Invalid number of bssid's");
		return QDF_STATUS_E_INVAL;
	}
	len += WMI_TLV_HDR_SIZE;
	len += numap * sizeof(wmi_extscan_wlan_change_bssid_param);

	*buf = wmi_buf_alloc(wmi_handle, len);
	if (!*buf) {
		wmi_err("Failed to allocate memory for change monitor cmd");
		return QDF_STATUS_E_FAILURE;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(*buf);
	cmd =
		(wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param *)
		buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param,
	       WMITLV_GET_STRUCT_TLVLEN
	       (wmi_extscan_configure_wlan_change_monitor_cmd_fixed_param));

	cmd->request_id = psigchange->request_id;
	cmd->vdev_id = psigchange->vdev_id;
	cmd->total_entries = numap;
	cmd->mode = 1;
	cmd->num_entries_in_page = numap;
	cmd->lost_ap_scan_count = psigchange->lostap_sample_size;
	cmd->max_rssi_samples = psigchange->rssi_sample_size;
	cmd->rssi_averaging_samples = psigchange->rssi_sample_size;
	cmd->max_out_of_range_count = psigchange->min_breaching;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       numap * sizeof(wmi_extscan_wlan_change_bssid_param));
	dest_chglist = (wmi_extscan_wlan_change_bssid_param *)
		       (buf_ptr + WMI_TLV_HDR_SIZE);

	for (j = 0; j < numap; j++) {
		WMITLV_SET_HDR(dest_chglist,
		       WMITLV_TAG_STRUC_wmi_extscan_bucket_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
		       (wmi_extscan_wlan_change_bssid_param));

		dest_chglist->lower_rssi_limit = src_ap->low;
		dest_chglist->upper_rssi_limit = src_ap->high;
		WMI_CHAR_ARRAY_TO_MAC_ADDR(src_ap->bssid.bytes,
					   &dest_chglist->bssid);

		wmi_debug("min_rssi: %d", dest_chglist->lower_rssi_limit);
		dest_chglist++;
		src_ap++;
	}
	buf_ptr += WMI_TLV_HDR_SIZE +
		   (numap * sizeof(wmi_extscan_wlan_change_bssid_param));
	*buf_len = len;
	return QDF_STATUS_SUCCESS;
}

/**
 * send_extscan_start_change_monitor_cmd_tlv() - send start change monitor cmd
 * @wmi_handle: wmi handle
 * @psigchange: change monitor request params
 *
 * This function sends start change monitor request to fw.
 *
 * Return: CDF status
 */
static QDF_STATUS send_extscan_start_change_monitor_cmd_tlv
			(wmi_unified_t wmi_handle,
			struct extscan_set_sig_changereq_params *
			psigchange)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	wmi_buf_t buf;
	int len;


	qdf_status = wmi_get_buf_extscan_change_monitor_cmd(wmi_handle,
			     psigchange, &buf,
			     &len);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		wmi_err("Failed to get buffer for change monitor cmd");
		return QDF_STATUS_E_FAILURE;
	}
	if (!buf) {
		wmi_err("Failed to get buffer");
		return QDF_STATUS_E_FAILURE;
	}
	if (wmi_unified_cmd_send(wmi_handle, buf, len,
		 WMI_EXTSCAN_CONFIGURE_WLAN_CHANGE_MONITOR_CMDID)) {
		wmi_err("Failed to send command");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_extscan_stop_hotlist_monitor_cmd_tlv() - stop hotlist monitor
 * @wmi_handle: wmi handle
 * @photlist_reset: hotlist reset params
 *
 * This function configures hotlist monitor to stop in fw.
 *
 * Return: CDF status
 */
static QDF_STATUS send_extscan_stop_hotlist_monitor_cmd_tlv
		(wmi_unified_t wmi_handle,
		struct extscan_bssid_hotlist_reset_params *photlist_reset)
{
	wmi_extscan_configure_hotlist_monitor_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;
	int hotlist_entries = 0;

	len = sizeof(*cmd);

	/* reset bssid hotlist with tlv set to 0 */
	len += WMI_TLV_HDR_SIZE;
	len += hotlist_entries * sizeof(wmi_extscan_hotlist_entry);

	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_NOMEM;
	}

	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_extscan_configure_hotlist_monitor_cmd_fixed_param *)
	      buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
	WMITLV_TAG_STRUC_wmi_extscan_configure_hotlist_monitor_cmd_fixed_param,
	WMITLV_GET_STRUCT_TLVLEN
	(wmi_extscan_configure_hotlist_monitor_cmd_fixed_param));

	cmd->request_id = photlist_reset->request_id;
	cmd->vdev_id = photlist_reset->vdev_id;
	cmd->mode = 0;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       hotlist_entries * sizeof(wmi_extscan_hotlist_entry));
	buf_ptr += WMI_TLV_HDR_SIZE +
		   (hotlist_entries * sizeof(wmi_extscan_hotlist_entry));

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				WMI_EXTSCAN_CONFIGURE_HOTLIST_MONITOR_CMDID)) {
		wmi_err("Failed to send extscan cfg hotlist cmd");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * send_stop_extscan_cmd_tlv() - stop extscan command to fw.
 * @wmi_handle: wmi handle
 * @pstopcmd: stop scan command request params
 *
 * This function sends stop extscan request to fw.
 *
 * Return: CDF Status.
 */
static QDF_STATUS send_stop_extscan_cmd_tlv(wmi_unified_t wmi_handle,
			  struct extscan_stop_req_params *pstopcmd)
{
	wmi_extscan_stop_cmd_fixed_param *cmd;
	wmi_buf_t wmi_buf;
	uint32_t len;
	uint8_t *buf_ptr;

	len = sizeof(*cmd);
	wmi_buf = wmi_buf_alloc(wmi_handle, len);
	if (!wmi_buf) {
		wmi_err("wmi_buf_alloc failed");
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(wmi_buf);
	cmd = (wmi_extscan_stop_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_extscan_stop_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_extscan_stop_cmd_fixed_param));

	cmd->request_id = pstopcmd->request_id;
	cmd->vdev_id = pstopcmd->vdev_id;

	if (wmi_unified_cmd_send(wmi_handle, wmi_buf, len,
				 WMI_EXTSCAN_STOP_CMDID)) {
		wmi_err("Failed to send extscan stop cmd");
		wmi_buf_free(wmi_buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * wmi_get_buf_extscan_start_cmd() - Fill extscan start request
 * @wmi_handle: wmi handle
 * @pstart: scan command request params
 * @buf: event buffer
 * @buf_len: length of buffer
 *
 * This function fills individual elements of extscan request and
 * TLV for buckets, channel list.
 *
 * Return: CDF Status.
 */
static
QDF_STATUS wmi_get_buf_extscan_start_cmd(wmi_unified_t wmi_handle,
			 struct wifi_scan_cmd_req_params *pstart,
			 wmi_buf_t *buf, int *buf_len)
{
	wmi_extscan_start_cmd_fixed_param *cmd;
	wmi_extscan_bucket *dest_blist;
	wmi_extscan_bucket_channel *dest_clist;
	struct wifi_scan_bucket_params *src_bucket = pstart->buckets;
	struct wifi_scan_channelspec_params *src_channel = src_bucket->channels;
	struct wifi_scan_channelspec_params save_channel[WMI_WLAN_EXTSCAN_MAX_CHANNELS];

	uint8_t *buf_ptr;
	int i, k, count = 0;
	int len = sizeof(*cmd);
	int nbuckets = pstart->num_buckets;
	int nchannels = 0;

	/* These TLV's are are NULL by default */
	uint32_t ie_len_with_pad = 0;
	int num_ssid = 0;
	int num_bssid = 0;
	int ie_len = 0;

	uint32_t base_period = pstart->base_period;

	/* TLV placeholder for ssid_list (NULL) */
	len += WMI_TLV_HDR_SIZE;
	len += num_ssid * sizeof(wmi_ssid);

	/* TLV placeholder for bssid_list (NULL) */
	len += WMI_TLV_HDR_SIZE;
	len += num_bssid * sizeof(wmi_mac_addr);

	/* TLV placeholder for ie_data (NULL) */
	len += WMI_TLV_HDR_SIZE;
	len += ie_len * sizeof(uint32_t);

	/* TLV placeholder for bucket */
	len += WMI_TLV_HDR_SIZE;
	len += nbuckets * sizeof(wmi_extscan_bucket);

	/* TLV channel placeholder */
	len += WMI_TLV_HDR_SIZE;
	for (i = 0; i < nbuckets; i++) {
		nchannels += src_bucket->num_channels;
		src_bucket++;
	}

	wmi_debug("Total buckets: %d total #of channels is %d",
		 nbuckets, nchannels);
	len += nchannels * sizeof(wmi_extscan_bucket_channel);
	/* Allocate the memory */
	*buf = wmi_buf_alloc(wmi_handle, len);
	if (!*buf) {
		wmi_err("Failed to allocate memory for start extscan cmd");
		return QDF_STATUS_E_NOMEM;
	}
	buf_ptr = (uint8_t *) wmi_buf_data(*buf);
	cmd = (wmi_extscan_start_cmd_fixed_param *) buf_ptr;
	WMITLV_SET_HDR(&cmd->tlv_header,
		       WMITLV_TAG_STRUC_wmi_extscan_start_cmd_fixed_param,
		       WMITLV_GET_STRUCT_TLVLEN
			       (wmi_extscan_start_cmd_fixed_param));

	cmd->request_id = pstart->request_id;
	cmd->vdev_id = pstart->vdev_id;
	cmd->base_period = pstart->base_period;
	cmd->num_buckets = nbuckets;
	cmd->configuration_flags = 0;
	if (pstart->configuration_flags & WMI_EXTSCAN_LP_EXTENDED_BATCHING)
		cmd->configuration_flags |= WMI_EXTSCAN_EXTENDED_BATCHING_EN;
	wmi_debug("configuration_flags: 0x%x", cmd->configuration_flags);
#ifdef FEATURE_WLAN_EXTSCAN
	cmd->min_rest_time = WMI_EXTSCAN_REST_TIME;
	cmd->max_rest_time = WMI_EXTSCAN_REST_TIME;
	cmd->max_scan_time = WMI_EXTSCAN_MAX_SCAN_TIME;
	cmd->burst_duration = WMI_EXTSCAN_BURST_DURATION;
#endif

	/* The max dwell time is retrieved from the first channel
	 * of the first bucket and kept common for all channels.
	 */
	cmd->min_dwell_time_active = pstart->min_dwell_time_active;
	cmd->max_dwell_time_active = pstart->max_dwell_time_active;
	cmd->min_dwell_time_passive = pstart->min_dwell_time_passive;
	cmd->max_dwell_time_passive = pstart->max_dwell_time_passive;
	cmd->max_bssids_per_scan_cycle = pstart->max_ap_per_scan;
	cmd->max_table_usage = pstart->report_threshold_percent;
	cmd->report_threshold_num_scans = pstart->report_threshold_num_scans;

	cmd->repeat_probe_time = cmd->max_dwell_time_active /
					WMI_SCAN_NPROBES_DEFAULT;
	cmd->probe_delay = 0;
	cmd->probe_spacing_time = 0;
	cmd->idle_time = 0;
	cmd->scan_ctrl_flags = WMI_SCAN_ADD_BCAST_PROBE_REQ |
			       WMI_SCAN_ADD_CCK_RATES |
			       WMI_SCAN_ADD_OFDM_RATES |
			       WMI_SCAN_ADD_SPOOFED_MAC_IN_PROBE_REQ |
			       WMI_SCAN_ADD_DS_IE_IN_PROBE_REQ;
	WMI_SCAN_SET_DWELL_MODE(cmd->scan_ctrl_flags,
			pstart->extscan_adaptive_dwell_mode);
	cmd->scan_priority = WMI_SCAN_PRIORITY_VERY_LOW;
	cmd->num_ssids = 0;
	cmd->num_bssid = 0;
	cmd->ie_len = 0;
	cmd->n_probes = (cmd->repeat_probe_time > 0) ?
			cmd->max_dwell_time_active / cmd->repeat_probe_time : 0;

	buf_ptr += sizeof(*cmd);
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_FIXED_STRUC,
		       num_ssid * sizeof(wmi_ssid));
	buf_ptr += WMI_TLV_HDR_SIZE + (num_ssid * sizeof(wmi_ssid));

	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_FIXED_STRUC,
		       num_bssid * sizeof(wmi_mac_addr));
	buf_ptr += WMI_TLV_HDR_SIZE + (num_bssid * sizeof(wmi_mac_addr));

	ie_len_with_pad = 0;
	WMITLV_SET_HDR(buf_ptr, WMITLV_TAG_ARRAY_BYTE,
			  ie_len_with_pad);
	buf_ptr += WMI_TLV_HDR_SIZE + ie_len_with_pad;

	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       nbuckets * sizeof(wmi_extscan_bucket));
	dest_blist = (wmi_extscan_bucket *)
		     (buf_ptr + WMI_TLV_HDR_SIZE);
	src_bucket = pstart->buckets;

	/* Retrieve scanning information from each bucket and
	 * channels and send it to the target
	 */
	for (i = 0; i < nbuckets; i++) {
		WMITLV_SET_HDR(dest_blist,
		      WMITLV_TAG_STRUC_wmi_extscan_bucket_cmd_fixed_param,
		      WMITLV_GET_STRUCT_TLVLEN(wmi_extscan_bucket));

		dest_blist->bucket_id = src_bucket->bucket;
		dest_blist->base_period_multiplier =
			src_bucket->period / base_period;
		dest_blist->min_period = src_bucket->period;
		dest_blist->max_period = src_bucket->max_period;
		dest_blist->exp_backoff = src_bucket->exponent;
		dest_blist->exp_max_step_count = src_bucket->step_count;
		dest_blist->channel_band = src_bucket->band;
		dest_blist->num_channels = src_bucket->num_channels;
		dest_blist->notify_extscan_events = 0;

		if (src_bucket->report_events &
					WMI_EXTSCAN_REPORT_EVENTS_EACH_SCAN)
			dest_blist->notify_extscan_events =
					WMI_EXTSCAN_CYCLE_COMPLETED_EVENT |
					WMI_EXTSCAN_CYCLE_STARTED_EVENT;

		if (src_bucket->report_events &
				WMI_EXTSCAN_REPORT_EVENTS_FULL_RESULTS) {
			dest_blist->forwarding_flags =
				WMI_EXTSCAN_FORWARD_FRAME_TO_HOST;
			dest_blist->notify_extscan_events |=
				WMI_EXTSCAN_BUCKET_COMPLETED_EVENT |
				WMI_EXTSCAN_CYCLE_STARTED_EVENT |
				WMI_EXTSCAN_CYCLE_COMPLETED_EVENT;
		} else {
			dest_blist->forwarding_flags =
				WMI_EXTSCAN_NO_FORWARDING;
		}

		if (src_bucket->report_events &
					WMI_EXTSCAN_REPORT_EVENTS_NO_BATCH)
			dest_blist->configuration_flags = 0;
		else
			dest_blist->configuration_flags =
				WMI_EXTSCAN_BUCKET_CACHE_RESULTS;

		wmi_debug("ntfy_extscan_events:%u cfg_flags:%u fwd_flags:%u",
			  dest_blist->notify_extscan_events,
			  dest_blist->configuration_flags,
			  dest_blist->forwarding_flags);

		dest_blist->min_dwell_time_active =
				   src_bucket->min_dwell_time_active;
		dest_blist->max_dwell_time_active =
				   src_bucket->max_dwell_time_active;
		dest_blist->min_dwell_time_passive =
				   src_bucket->min_dwell_time_passive;
		dest_blist->max_dwell_time_passive =
				   src_bucket->max_dwell_time_passive;
		src_channel = src_bucket->channels;

		/* save the channel info to later populate
		 * the  channel TLV
		 */
		for (k = 0; k < src_bucket->num_channels; k++) {
			save_channel[count++].channel = src_channel->channel;
			src_channel++;
		}
		dest_blist++;
		src_bucket++;
	}
	buf_ptr += WMI_TLV_HDR_SIZE + (nbuckets * sizeof(wmi_extscan_bucket));
	WMITLV_SET_HDR(buf_ptr,
		       WMITLV_TAG_ARRAY_STRUC,
		       nchannels * sizeof(wmi_extscan_bucket_channel));
	dest_clist = (wmi_extscan_bucket_channel *)
		     (buf_ptr + WMI_TLV_HDR_SIZE);

	/* Active or passive scan is based on the bucket dwell time
	 * and channel specific active,passive scans are not
	 * supported yet
	 */
	for (i = 0; i < nchannels; i++) {
		WMITLV_SET_HDR(dest_clist,
		WMITLV_TAG_STRUC_wmi_extscan_bucket_channel_event_fixed_param,
			   WMITLV_GET_STRUCT_TLVLEN
			   (wmi_extscan_bucket_channel));
		dest_clist->channel = save_channel[i].channel;
		dest_clist++;
	}
	buf_ptr += WMI_TLV_HDR_SIZE +
		   (nchannels * sizeof(wmi_extscan_bucket_channel));
	*buf_len = len;
	return QDF_STATUS_SUCCESS;
}

/**
 * send_start_extscan_cmd_tlv() - start extscan command to fw.
 * @wmi_handle: wmi handle
 * @pstart: scan command request params
 *
 * This function sends start extscan request to fw.
 *
 * Return: CDF Status.
 */
static QDF_STATUS send_start_extscan_cmd_tlv(wmi_unified_t wmi_handle,
			  struct wifi_scan_cmd_req_params *pstart)
{
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;
	wmi_buf_t buf;
	int len;

	/* Fill individual elements of extscan request and
	 * TLV for buckets, channel list.
	 */
	qdf_status = wmi_get_buf_extscan_start_cmd(wmi_handle,
			     pstart, &buf, &len);
	if (qdf_status != QDF_STATUS_SUCCESS) {
		wmi_err("Failed to get buffer for ext scan cmd");
		return QDF_STATUS_E_FAILURE;
	}
	if (!buf) {
		wmi_err("Failed to get buffer for current extscan info");
		return QDF_STATUS_E_FAILURE;
	}
	if (wmi_unified_cmd_send(wmi_handle, buf,
				 len, WMI_EXTSCAN_START_CMDID)) {
		wmi_err("Failed to send extscan start cmd");
		wmi_buf_free(buf);
		return QDF_STATUS_E_FAILURE;
	}

	return QDF_STATUS_SUCCESS;
}

/** wmi_get_hotlist_entries_per_page() - hotlist entries per page
 * @wmi_handle: wmi handle.
 * @cmd: size of command structure.
 * @per_entry_size: per entry size.
 *
 * This utility function calculates how many hotlist entries can
 * fit in one page.
 *
 * Return: number of entries
 */
static inline int wmi_get_hotlist_entries_per_page
				(wmi_unified_t wmi_handle,
				size_t cmd_size,
				size_t per_entry_size)
{
	uint32_t avail_space = 0;
	int num_entries = 0;
	uint16_t max_msg_len = wmi_get_max_msg_len(wmi_handle);

	/* Calculate number of hotlist entries that can
	 * be passed in wma message request.
	 */
	avail_space = max_msg_len - cmd_size;
	num_entries = avail_space / per_entry_size;
	return num_entries;
}

/**
 * send_extscan_start_hotlist_monitor_cmd_tlv() - start hotlist monitor
 * @wmi_handle: wmi handle
 * @params: hotlist params
 *
 * This function configures hotlist monitor to start in fw.
 *
 * Return: QDF status
 */
static QDF_STATUS send_extscan_start_hotlist_monitor_cmd_tlv
			(wmi_unified_t wmi_handle,
			struct extscan_bssid_hotlist_set_params *params)
{
	wmi_extscan_configure_hotlist_monitor_cmd_fixed_param *cmd = NULL;
	wmi_extscan_hotlist_entry *dest_hotlist;
	struct ap_threshold_params *src_ap = params->ap;
	wmi_buf_t buf;
	uint8_t *buf_ptr;

	int j, index = 0;
	int cmd_len = 0;
	int num_entries;
	int min_entries = 0;
	uint32_t numap = params->num_ap;
	int len = sizeof(*cmd);

	len += WMI_TLV_HDR_SIZE;
	cmd_len = len;

	num_entries = wmi_get_hotlist_entries_per_page(wmi_handle,
							cmd_len,
							sizeof(*dest_hotlist));
	/* setbssid hotlist expects the bssid list
	 * to be non zero value
	 */
	if (!numap || (numap > WMI_WLAN_EXTSCAN_MAX_HOTLIST_APS)) {
		wmi_err("Invalid number of APs: %d", numap);
		return QDF_STATUS_E_INVAL;
	}

	/* Split the hot list entry pages and send multiple command
	 * requests if the buffer reaches the maximum request size
	 */
	while (index < numap) {
		min_entries = QDF_MIN(num_entries, numap);
		len += min_entries * sizeof(wmi_extscan_hotlist_entry);
		buf = wmi_buf_alloc(wmi_handle, len);
		if (!buf) {
			wmi_err("wmi_buf_alloc failed");
			return QDF_STATUS_E_FAILURE;
		}
		buf_ptr = (uint8_t *) wmi_buf_data(buf);
		cmd = (wmi_extscan_configure_hotlist_monitor_cmd_fixed_param *)
		      buf_ptr;
		WMITLV_SET_HDR(&cmd->tlv_header,
			       WMITLV_TAG_STRUC_wmi_extscan_configure_hotlist_monitor_cmd_fixed_param,
			       WMITLV_GET_STRUCT_TLVLEN
				       (wmi_extscan_configure_hotlist_monitor_cmd_fixed_param));

		/* Multiple requests are sent until the num_entries_in_page
		 * matches the total_entries
		 */
		cmd->request_id = params->request_id;
		cmd->vdev_id = params->vdev_id;
		cmd->total_entries = numap;
		cmd->mode = 1;
		cmd->num_entries_in_page = min_entries;
		cmd->lost_ap_scan_count = params->lost_ap_sample_size;
		cmd->first_entry_index = index;

		wmi_debug("vdev id:%d total_entries: %d num_entries: %d lost_ap_sample_size: %d",
			 cmd->vdev_id, cmd->total_entries,
			 cmd->num_entries_in_page,
			 cmd->lost_ap_scan_count);

		buf_ptr += sizeof(*cmd);
		WMITLV_SET_HDR(buf_ptr,
			       WMITLV_TAG_ARRAY_STRUC,
			       min_entries * sizeof(wmi_extscan_hotlist_entry));
		dest_hotlist = (wmi_extscan_hotlist_entry *)
			       (buf_ptr + WMI_TLV_HDR_SIZE);

		/* Populate bssid, channel info and rssi
		 * for the bssid's that are sent as hotlists.
		 */
		for (j = 0; j < min_entries; j++) {
			WMITLV_SET_HDR(dest_hotlist,
				       WMITLV_TAG_STRUC_wmi_extscan_bucket_cmd_fixed_param,
				       WMITLV_GET_STRUCT_TLVLEN
					       (wmi_extscan_hotlist_entry));

			dest_hotlist->min_rssi = src_ap->low;
			WMI_CHAR_ARRAY_TO_MAC_ADDR(src_ap->bssid.bytes,
						   &dest_hotlist->bssid);

			wmi_debug("channel:%d min_rssi %d",
				 dest_hotlist->channel,
				 dest_hotlist->min_rssi);
			wmi_debug("bssid mac_addr31to0: 0x%x, mac_addr47to32: 0x%x",
				dest_hotlist->bssid.mac_addr31to0,
				dest_hotlist->bssid.mac_addr47to32);
			dest_hotlist++;
			src_ap++;
		}
		buf_ptr += WMI_TLV_HDR_SIZE +
			   (min_entries * sizeof(wmi_extscan_hotlist_entry));

		if (wmi_unified_cmd_send(wmi_handle, buf, len,
				WMI_EXTSCAN_CONFIGURE_HOTLIST_MONITOR_CMDID)) {
			wmi_err("Failed to send extscan cfg hotlist monitor cmd");
			wmi_buf_free(buf);
			return QDF_STATUS_E_FAILURE;
		}
		index = index + min_entries;
		num_entries = numap - min_entries;
		len = cmd_len;
	}
	return QDF_STATUS_SUCCESS;
}

void wmi_extscan_attach_tlv(wmi_unified_t wmi_handle)
{
	struct wmi_ops *ops = wmi_handle->ops;
	ops->send_reset_passpoint_network_list_cmd =
				send_reset_passpoint_network_list_cmd_tlv;
	ops->send_set_passpoint_network_list_cmd =
				send_set_passpoint_network_list_cmd_tlv;
	ops->send_set_epno_network_list_cmd =
				send_set_epno_network_list_cmd_tlv;
	ops->send_extscan_get_capabilities_cmd =
				 send_extscan_get_capabilities_cmd_tlv;
	ops->send_extscan_get_cached_results_cmd =
				send_extscan_get_cached_results_cmd_tlv;
	ops->send_extscan_stop_change_monitor_cmd =
				send_extscan_stop_change_monitor_cmd_tlv;
	ops->send_extscan_start_change_monitor_cmd =
				send_extscan_start_change_monitor_cmd_tlv;
	ops->send_extscan_stop_hotlist_monitor_cmd =
				send_extscan_stop_hotlist_monitor_cmd_tlv;
	ops->send_extscan_start_hotlist_monitor_cmd =
				send_extscan_start_hotlist_monitor_cmd_tlv;
	ops->send_stop_extscan_cmd = send_stop_extscan_cmd_tlv;
	ops->send_start_extscan_cmd = send_start_extscan_cmd_tlv;
}
