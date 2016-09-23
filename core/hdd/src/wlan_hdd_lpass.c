/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 *  DOC: wlan_hdd_lpass.c
 *
 *  WLAN Host Device Driver LPASS feature implementation
 *
 */

/* denote that this file does not allow legacy hddLog */
#define HDD_DISALLOW_LEGACY_HDDLOG 1

/* Include Files */
#include "wlan_hdd_main.h"
#include "wlan_hdd_lpass.h"
#include <cds_utils.h>
#include "qwlan_version.h"

/**
 * wlan_hdd_gen_wlan_status_pack() - Create lpass adapter status package
 * @data: Status data record to be created
 * @adapter: Adapter whose status is to being packaged
 * @sta_ctx: Station-specific context of @adapter
 * @is_on: Is wlan driver loaded?
 * @is_connected: Is @adapater connected to an AP?
 *
 * Generate a wlan vdev status package. The status info includes wlan
 * on/off status, vdev ID, vdev mode, supported channels, etc.
 *
 * Return: 0 if package was created, otherwise a negative errno
 */
static int wlan_hdd_gen_wlan_status_pack(struct wlan_status_data *data,
					 hdd_adapter_t *adapter,
					 hdd_station_ctx_t *sta_ctx,
					 uint8_t is_on, uint8_t is_connected)
{
	hdd_context_t *hdd_ctx = NULL;
	uint8_t buflen = WLAN_SVC_COUNTRY_CODE_LEN;

	if (!data) {
		hdd_err("invalid data pointer");
		return -EINVAL;
	}
	if (!adapter) {
		if (is_on) {
			/* no active interface */
			data->lpss_support = 0;
			data->is_on = is_on;
			return 0;
		}
		hdd_err("invalid adapter pointer");
		return -EINVAL;
	}

	if (adapter->sessionId == HDD_SESSION_ID_INVALID)
		return -EINVAL;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	if (hdd_ctx->lpss_support && hdd_ctx->config->enable_lpass_support)
		data->lpss_support = 1;
	else
		data->lpss_support = 0;
	data->numChannels = WLAN_SVC_MAX_NUM_CHAN;
	sme_get_cfg_valid_channels(hdd_ctx->hHal, data->channel_list,
				   &data->numChannels);
	sme_get_country_code(hdd_ctx->hHal, data->country_code, &buflen);
	data->is_on = is_on;
	data->vdev_id = adapter->sessionId;
	data->vdev_mode = adapter->device_mode;
	if (sta_ctx) {
		data->is_connected = is_connected;
		data->rssi = adapter->rssi;
		data->freq =
			cds_chan_to_freq(sta_ctx->conn_info.operationChannel);
		if (WLAN_SVC_MAX_SSID_LEN >=
		    sta_ctx->conn_info.SSID.SSID.length) {
			data->ssid_len = sta_ctx->conn_info.SSID.SSID.length;
			memcpy(data->ssid,
			       sta_ctx->conn_info.SSID.SSID.ssId,
			       sta_ctx->conn_info.SSID.SSID.length);
		}
		if (QDF_MAC_ADDR_SIZE >= sizeof(sta_ctx->conn_info.bssId))
			memcpy(data->bssid, sta_ctx->conn_info.bssId.bytes,
			       QDF_MAC_ADDR_SIZE);
	}
	return 0;
}

/**
 * wlan_hdd_gen_wlan_version_pack() - Create lpass version package
 * @data: Version data record to be created
 * @fw_version: Version code from firmware
 * @chip_id: WLAN chip ID
 * @chip_name: WLAN chip name
 *
 * Generate a wlan software/hw version info package. The version info
 * includes wlan host driver version, wlan fw driver version, wlan hw
 * chip id & wlan hw chip name.
 *
 * Return: 0 if package was created, otherwise a negative errno
 */
static int wlan_hdd_gen_wlan_version_pack(struct wlan_version_data *data,
					  uint32_t fw_version,
					  uint32_t chip_id,
					  const char *chip_name)
{
	if (!data) {
		hdd_err("invalid data pointer");
		return -EINVAL;
	}

	data->chip_id = chip_id;
	strlcpy(data->chip_name, chip_name, WLAN_SVC_MAX_STR_LEN);
	if (strncmp(chip_name, "Unknown", 7))
		strlcpy(data->chip_from, "Qualcomm", WLAN_SVC_MAX_STR_LEN);
	else
		strlcpy(data->chip_from, "Unknown", WLAN_SVC_MAX_STR_LEN);
	strlcpy(data->host_version, QWLAN_VERSIONSTR, WLAN_SVC_MAX_STR_LEN);
	scnprintf(data->fw_version, WLAN_SVC_MAX_STR_LEN, "%d.%d.%d.%d",
		  (fw_version & 0xf0000000) >> 28,
		  (fw_version & 0xf000000) >> 24,
		  (fw_version & 0xf00000) >> 20, (fw_version & 0x7fff));
	return 0;
}

/**
 * wlan_hdd_send_status_pkg() - Send adapter status to lpass
 * @adapter: Adapter whose status is to be sent to lpass
 * @sta_ctx: Station-specific context of @adapter
 * @is_on: Is @adapter enabled
 * @is_connected: Is @adapater connected
 *
 * Generate wlan vdev status pacakge and send it to a user space
 * daemon through netlink.
 *
 * Return: none
 */
void wlan_hdd_send_status_pkg(hdd_adapter_t *adapter,
			      hdd_station_ctx_t *sta_ctx,
			      uint8_t is_on, uint8_t is_connected)
{
	int ret = 0;
	struct wlan_status_data data;
	hdd_context_t *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	if (!hdd_ctx)
		return;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam())
		return;

	memset(&data, 0, sizeof(struct wlan_status_data));
	if (is_on)
		ret = wlan_hdd_gen_wlan_status_pack(&data, adapter, sta_ctx,
						    is_on, is_connected);

	if (!ret)
		wlan_hdd_send_svc_nlink_msg(hdd_ctx->radio_index,
					WLAN_SVC_WLAN_STATUS_IND,
					    &data, sizeof(data));
}

/**
 * wlan_hdd_send_version_pkg() - report version information to lpass
 * @fw_version: Version code from firmware
 * @chip_id: WLAN chip ID
 * @chip_name: WLAN chip name
 *
 * Generate a wlan sw/hw version info package and send it to a user
 * space daemon through netlink.
 *
 * Return: none
 */
static void wlan_hdd_send_version_pkg(uint32_t fw_version,
				      uint32_t chip_id,
				      const char *chip_name)
{
	int ret = 0;
	struct wlan_version_data data;
	hdd_context_t *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	if (!hdd_ctx)
		return;

	if (QDF_GLOBAL_FTM_MODE == hdd_get_conparam())
		return;

	memset(&data, 0, sizeof(struct wlan_version_data));
	ret = wlan_hdd_gen_wlan_version_pack(&data, fw_version, chip_id,
					     chip_name);
	if (!ret)
		wlan_hdd_send_svc_nlink_msg(hdd_ctx->radio_index,
					WLAN_SVC_WLAN_VERSION_IND,
					    &data, sizeof(data));
}

/**
 * wlan_hdd_send_all_scan_intf_info() - report scan interfaces to lpass
 * @hdd_ctx: The global HDD context
 *
 * This function iterates through all of the interfaces registered
 * with HDD and indicates to lpass all that support scanning.
 * If no interfaces support scanning then that fact is also indicated.
 *
 * Return: none
 */
static void wlan_hdd_send_all_scan_intf_info(struct hdd_context_s *hdd_ctx)
{
	hdd_adapter_t *adapter = NULL;
	hdd_adapter_list_node_t *node = NULL, *next = NULL;
	bool scan_intf_found = false;
	QDF_STATUS status;

	if (!hdd_ctx) {
		hdd_err("NULL pointer for hdd_ctx");
		return;
	}

	status = hdd_get_front_adapter(hdd_ctx, &node);
	while (NULL != node && QDF_STATUS_SUCCESS == status) {
		adapter = node->pAdapter;
		if (adapter) {
			if (adapter->device_mode == QDF_STA_MODE
			    || adapter->device_mode == QDF_P2P_CLIENT_MODE
			    || adapter->device_mode ==
			    QDF_P2P_DEVICE_MODE) {
				scan_intf_found = true;
				wlan_hdd_send_status_pkg(adapter, NULL, 1, 0);
			}
		}
		status = hdd_get_next_adapter(hdd_ctx, node, &next);
		node = next;
	}

	if (!scan_intf_found)
		wlan_hdd_send_status_pkg(adapter, NULL, 1, 0);
}

/**
 * hdd_lpass_notify_mode_change() - Notify LPASS of interface mode change
 * @adapter: The adapter whose mode was changed
 *
 * This function is used to notify the LPASS feature that an adapter
 * had its mode changed.
 *
 * Return: none
 */
/* implementation note: when one changes we notify them all */
void hdd_lpass_notify_mode_change(struct hdd_adapter_s *adapter)
{
	struct hdd_context_s *hdd_ctx;

	hdd_ctx = WLAN_HDD_GET_CTX(adapter);
	wlan_hdd_send_all_scan_intf_info(hdd_ctx);
}

/**
 * hdd_lpass_notify_start() - Notify LPASS of driver start
 * @hdd_ctx: The global HDD context
 *
 * This function is used to notify the LPASS feature that the wlan
 * driver has (re-)started.
 *
 * Return: none
 */
void hdd_lpass_notify_start(struct hdd_context_s *hdd_ctx)
{
	wlan_hdd_send_all_scan_intf_info(hdd_ctx);
	wlan_hdd_send_version_pkg(hdd_ctx->target_fw_version,
				  hdd_ctx->target_hw_version,
				  hdd_ctx->target_hw_name);
}

/**
 * hdd_lpass_notify_stop() - Notify LPASS of driver stop
 * @hdd_ctx: The global HDD context
 *
 * This function is used to notify the LPASS feature that the wlan
 * driver has stopped
 *
 * Return: none
 */
void hdd_lpass_notify_stop(struct hdd_context_s *hdd_ctx)
{
	wlan_hdd_send_status_pkg(NULL, NULL, 0, 0);
}
