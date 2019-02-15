/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: wlan_hdd_debugfs_connect.c
 *
 * WLAN Host Device Driver implementation to update
 * debugfs with connect information
 */

#include <wlan_hdd_debugfs_csr.h>
#include <wlan_hdd_main.h>
#include <cds_sched.h>
#include <wma_api.h>
#include "qwlan_version.h"
#include "wmi_unified_param.h"

/**
 * wlan_hdd_version_info_debugfs() - Populate driver, FW and HW version
 * @hdd_ctx: pointer to hdd context
 * @buf: output buffer to hold version info
 * @buf_avail_len: available buffer length
 *
 * Return: No.of bytes populated by this function in buffer
 */
static ssize_t
wlan_hdd_version_info_debugfs(struct hdd_context *hdd_ctx, uint8_t *buf,
			      ssize_t buf_avail_len)
{
	uint32_t major_spid = 0, minor_spid = 0, siid = 0, crmid = 0, sub_id;
	ssize_t length = 0;
	int ret_val;

	hdd_get_fw_version(hdd_ctx, &major_spid, &minor_spid, &siid, &crmid);
	sub_id = (hdd_ctx->target_fw_vers_ext & 0xf0000000) >> 28;

	ret_val = scnprintf(buf, buf_avail_len,
			    "\nVERSION DETAILS\n");
	if (ret_val <= 0)
		return length;
	length += ret_val;

	if (length >= buf_avail_len) {
		hdd_err("No sufficient buf_avail_len");
		return buf_avail_len;
	}

	ret_val = scnprintf(buf + length, buf_avail_len - length,
			    "Host Driver Version: %s\n"
			    "Firmware Version: %d.%d.%d.%d.%d\n"
			    "Hardware Version: %s\n",
			    QWLAN_VERSIONSTR,
			    major_spid, minor_spid, siid, crmid, sub_id,
			    hdd_ctx->target_hw_name);
	if (ret_val <= 0)
		return length;

	length += ret_val;

	return length;
}

/**
 * wlan_hdd_add_nss_info() - Populate NSS info
 * @conn_info: station connection information
 * @buf: output buffer to hold version info
 * @buf_avail_len: available buffer length
 *
 * Return: No.of bytes populated by this function in buffer
 */
static ssize_t
wlan_hdd_add_nss_info(struct hdd_connection_info *conn_info,
		      uint8_t *buf, ssize_t buf_avail_len)
{
	ssize_t length = 0;
	int ret_val;

	if (!conn_info->conn_flag.ht_present &&
	    !conn_info->conn_flag.vht_present)
		return length;

	ret_val = scnprintf(buf, buf_avail_len,
			    "nss = %u\n",
			    conn_info->txrate.nss);
	if (ret_val <= 0)
		return length;

	length = ret_val;
	return length;
}

/**
 * wlan_hdd_add_ht_cap_info() - Populate HT info
 * @conn_info: station connection information
 * @buf: output buffer to hold version info
 * @buf_avail_len: available buffer length
 *
 * Return: No.of bytes populated by this function in buffer
 */
static ssize_t
wlan_hdd_add_ht_cap_info(struct hdd_connection_info *conn_info,
			 uint8_t *buf, ssize_t buf_avail_len)
{
	struct ieee80211_ht_cap *ht_caps;
	ssize_t length = 0;
	int ret;

	if (!conn_info->conn_flag.ht_present)
		return length;

	ht_caps = &conn_info->ht_caps;
	ret = scnprintf(buf, buf_avail_len,
			"ht_cap_info = %x\n"
			"ampdu_params_info = %x\n"
			"extended_ht_cap_info = %x\n"
			"tx_BF_cap_info = %x\n"
			"antenna_selection_info = %x\n"
			"ht_rx_higest = %x\n"
			"ht_tx_params = %x\n",
			ht_caps->cap_info,
			ht_caps->ampdu_params_info,
			ht_caps->extended_ht_cap_info,
			ht_caps->tx_BF_cap_info,
			ht_caps->antenna_selection_info,
			ht_caps->mcs.rx_highest,
			ht_caps->mcs.tx_params);
	if (ret <= 0)
		return length;

	length = ret;
	return length;
}

/**
 * wlan_hdd_add_vht_cap_info() - Populate VHT info
 * @conn_info: station connection information
 * @buf: output buffer to hold version info
 * @buf_avail_len: available buffer length
 *
 * Return: No.of bytes populated by this function in buffer
 */
static ssize_t
wlan_hdd_add_vht_cap_info(struct hdd_connection_info *conn_info,
			  uint8_t *buf, ssize_t buf_avail_len)
{
	struct ieee80211_vht_cap *vht_caps;
	ssize_t length = 0;
	int ret;

	if (!conn_info->conn_flag.vht_present)
		return length;

	vht_caps = &conn_info->vht_caps;
	ret = scnprintf(buf, buf_avail_len,
			"vht_cap_info = %x\n"
			"rx_mcs_map = %x\n"
			"rx_highest = %x\n"
			"tx_mcs_map = %x\n"
			"tx_highest = %x\n",
			vht_caps->vht_cap_info,
			vht_caps->supp_mcs.rx_mcs_map,
			vht_caps->supp_mcs.rx_highest,
			vht_caps->supp_mcs.tx_mcs_map,
			vht_caps->supp_mcs.tx_highest);
	if (ret <= 0)
		return length;

	length = ret;
	return length;
}

/**
 * hdd_auth_type_str() - Get string for enum csr auth type
 * @auth_type: authentication id
 *
 * Return: Meaningful string for enum csr auth type
 */
static
uint8_t *hdd_auth_type_str(uint32_t auth_type)
{
	switch (auth_type) {
	case eCSR_AUTH_TYPE_OPEN_SYSTEM:
		return "OPEN SYSTEM";
	case eCSR_AUTH_TYPE_SHARED_KEY:
		return "SHARED KEY";
	case eCSR_AUTH_TYPE_WPA:
		return "WPA";
	case eCSR_AUTH_TYPE_WPA_PSK:
		return "WPA PSK";
	case eCSR_AUTH_TYPE_AUTOSWITCH:
		return "AUTO SWITCH";
	case eCSR_AUTH_TYPE_WPA_NONE:
		return "WPA NONE";
	case eCSR_AUTH_TYPE_RSN:
		return "RSN";
	case eCSR_AUTH_TYPE_RSN_PSK:
		return "RSN PSK";
	case eCSR_AUTH_TYPE_FT_RSN:
		return "FT RSN";
	case eCSR_AUTH_TYPE_FT_RSN_PSK:
		return "FT RSN PSK";
	case eCSR_AUTH_TYPE_WAPI_WAI_CERTIFICATE:
		return "WAPI WAI CERTIFICATE";
	case eCSR_AUTH_TYPE_WAPI_WAI_PSK:
		return "WAPI WAI PSK";
	case eCSR_AUTH_TYPE_CCKM_WPA:
		return "CCKM WPA";
	case eCSR_AUTH_TYPE_CCKM_RSN:
		return "CCKM RSN";
	case eCSR_AUTH_TYPE_RSN_PSK_SHA256:
		return "RSN PSK SHA256";
	case eCSR_AUTH_TYPE_RSN_8021X_SHA256:
		return "RSN 8021X SHA256";
	case eCSR_NUM_OF_SUPPORT_AUTH_TYPE:
		return "NUM OF SUPPORT AUTH TYPE";
	case eCSR_AUTH_TYPE_FAILED:
		return "FAILED";
	case eCSR_AUTH_TYPE_NONE:
		return "NONE";
	}

	return "UNKNOWN";
}

/**
 * hdd_dot11_mode_str() - Get string for enum csr dot11 mode
 * @dot11mode: dot11 mode ID
 *
 * Return: Meaningful string for enum csr dot11 mode
 */
static
uint8_t *hdd_dot11_mode_str(uint32_t dot11mode)
{
	switch (dot11mode) {
	case eCSR_CFG_DOT11_MODE_11A:
		return "DOT11 MODE 11A";
	case eCSR_CFG_DOT11_MODE_11B:
		return "DOT11 MODE 11B";
	case eCSR_CFG_DOT11_MODE_11G:
		return "DOT11 MODE 11G";
	case eCSR_CFG_DOT11_MODE_11N:
		return "DOT11 MODE 11N";
	case eCSR_CFG_DOT11_MODE_11AC:
		return "DOT11 MODE 11AC";
	case eCSR_CFG_DOT11_MODE_AUTO:
		return "DOT11 MODE AUTO";
	case eCSR_CFG_DOT11_MODE_ABG:
		return "DOT11 MODE 11ABG";
	}

	return "UNKNOWN";
}

/**
 * hdd_ch_width_str() - Get string for channel width
 * @ch_width: channel width from connect info
 *
 * Return: User readable string for channel width
 */
static
uint8_t *hdd_ch_width_str(enum phy_ch_width ch_width)
{
	switch (ch_width) {
	case CH_WIDTH_20MHZ:
		return "20MHz";
	case CH_WIDTH_40MHZ:
		return "40MHz";
	case CH_WIDTH_80MHZ:
		return "80MHz";
	case CH_WIDTH_160MHZ:
		return "160MHz";
	case CH_WIDTH_80P80MHZ:
		return "(80 + 80)MHz";
	case CH_WIDTH_5MHZ:
		return "5MHz";
	case CH_WIDTH_10MHZ:
		return "10MHz";
	case CH_WIDTH_INVALID:
		/* Fallthrough */
	case CH_WIDTH_MAX:
		/* Fallthrough */
	default:
		return "UNKNOWN";
	}
}

/**
 * wlan_hdd_connect_info_debugfs() - Populate connect info
 * @adapter: pointer to sta adapter for which connect info is required
 * @buf: output buffer to hold version info
 * @buf_avail_len: available buffer length
 *
 * Return: No.of bytes populated by this function in buffer
 */
static ssize_t
wlan_hdd_connect_info_debugfs(struct hdd_adapter *adapter, uint8_t *buf,
			      ssize_t buf_avail_len)
{
	ssize_t length = 0;
	struct hdd_station_ctx *hdd_sta_ctx;
	struct hdd_connection_info *conn_info;
	uint32_t bit_rate;
	int ret_val;

	hdd_sta_ctx = WLAN_HDD_GET_STATION_CTX_PTR(adapter);
	if (hdd_sta_ctx->conn_info.connState != eConnectionState_Associated) {
		ret_val = scnprintf(buf, buf_avail_len,
				    "\nSTA is not connected\n");
		if (ret_val >= 0)
			length = ret_val;
		return length;
	}

	ret_val = scnprintf(buf, buf_avail_len,
			    "\nCONNECTION DETAILS\n");
	if (ret_val <= 0)
		return length;
	length += ret_val;

	if (length >= buf_avail_len) {
		hdd_err("No sufficient buf_avail_len");
		return buf_avail_len;
	}

	if (hdd_sta_ctx->hdd_reassoc_scenario) {
		ret_val = scnprintf(buf + length, buf_avail_len - length,
				    "Roaming is in progress");
		if (ret_val <= 0)
			return length;
		length += ret_val;
	}

	conn_info = &hdd_sta_ctx->conn_info;
	bit_rate = cfg80211_calculate_bitrate(&conn_info->txrate);

	if (length >= buf_avail_len) {
		hdd_err("No sufficient buf_avail_len");
		return buf_avail_len;
	}
	ret_val = scnprintf(buf + length, buf_avail_len - length,
			    "ssid = %s\n"
			    "bssid = " MAC_ADDRESS_STR "\n"
			    "connect_time = %s\n"
			    "auth_time = %s\n"
			    "freq = %u\n"
			    "ch_width = %s\n"
			    "signal = %ddBm\n"
			    "bit_rate = %u\n"
			    "last_auth_type = %s\n"
			    "dot11Mode = %s\n",
			    conn_info->last_ssid.SSID.ssId,
			    MAC_ADDR_ARRAY(conn_info->bssId.bytes),
			    conn_info->connect_time,
			    conn_info->auth_time,
			    conn_info->freq,
			    hdd_ch_width_str(conn_info->ch_width),
			    conn_info->signal,
			    bit_rate,
			    hdd_auth_type_str(conn_info->last_auth_type),
			    hdd_dot11_mode_str(conn_info->dot11Mode));

	if (ret_val <= 0)
		return length;
	length += ret_val;

	if (length >= buf_avail_len) {
		hdd_err("No sufficient buf_avail_len");
		return buf_avail_len;
	}
	length += wlan_hdd_add_nss_info(conn_info, buf + length,
					buf_avail_len - length);

	if (length >= buf_avail_len) {
		hdd_err("No sufficient buf_avail_len");
		return buf_avail_len;
	}

	length += wlan_hdd_add_ht_cap_info(conn_info, buf + length,
					   buf_avail_len - length);

	if (length >= buf_avail_len) {
		hdd_err("No sufficient buf_avail_len");
		return buf_avail_len;
	}
	length += wlan_hdd_add_vht_cap_info(conn_info, buf + length,
					    buf_avail_len - length);
	return length;
}

ssize_t
wlan_hdd_debugfs_update_connect_info(struct hdd_context *hdd_ctx,
				     struct hdd_adapter *adapter,
				     uint8_t *buf, ssize_t buf_avail_len)
{
	ssize_t len;
	int ret_val;

	hdd_enter();

	len = wlan_hdd_current_time_info_debugfs(buf, buf_avail_len);
	if (len >= buf_avail_len) {
		hdd_err("No sufficient buf_avail_len");
		return buf_avail_len;
	}

	if (adapter->device_mode != QDF_STA_MODE) {
		ret_val = scnprintf(buf + len, buf_avail_len - len,
				    "Interface is not operating STA Mode\n");
		if (ret_val <= 0)
			return len;

		len += ret_val;
		return len;
	}

	if (len >= buf_avail_len) {
		hdd_err("No sufficient buf_avail_len");
		return buf_avail_len;
	}
	len += wlan_hdd_version_info_debugfs(hdd_ctx, buf + len,
					     buf_avail_len - len);

	if (len >= buf_avail_len) {
		hdd_err("No sufficient buf_avail_len");
		return buf_avail_len;
	}
	len += wlan_hdd_connect_info_debugfs(adapter, buf + len,
					     buf_avail_len - len);

	hdd_exit();

	return len;
}
