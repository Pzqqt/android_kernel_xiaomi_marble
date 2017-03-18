/*
 * Copyright (c) 2012-2017 The Linux Foundation. All rights reserved.
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
 * DOC: wifi_pos_utils_i.h
 * This file defines the prototypes for the utility helper functions
 * for the wifi_pos component.
 */

#ifdef WIFI_POS_CONVERGED
#ifndef _WIFI_POS_UTILS_H_
#define _WIFI_POS_UTILS_H_
/* Include files */
#include "qdf_types.h"
#include "qdf_status.h"
#include "ol_defines.h"
#include "qdf_trace.h"

struct wlan_objmgr_psoc;
struct wifi_pos_req_msg;

#define wifi_pos_log(level, args...) \
	QDF_TRACE(QDF_MODULE_ID_WIFIPOS, level, ## args)
#define wifi_pos_logfl(level, format, args...) \
	wifi_pos_log(level, FL(format), ## args)

#define wifi_pos_alert(format, args...) \
	wifi_pos_logfl(QDF_TRACE_LEVEL_FATAL, format, ## args)
#define wifi_pos_err(format, args...) \
	wifi_pos_logfl(QDF_TRACE_LEVEL_ERROR, format, ## args)
#define wifi_pos_warn(format, args...) \
	wifi_pos_logfl(QDF_TRACE_LEVEL_WARN, format, ## args)
#define wifi_pos_notice(format, args...) \
	wifi_pos_logfl(QDF_TRACE_LEVEL_INFO, format, ## args)
#define wifi_pos_debug(format, args...) \
	wifi_pos_logfl(QDF_TRACE_LEVEL_DEBUG, format, ## args)

#define OEM_APP_SIGNATURE_LEN      16
#define OEM_APP_SIGNATURE_STR      "QUALCOMM-OEM-APP"

#define OEM_TARGET_SIGNATURE_LEN   8
#define OEM_TARGET_SIGNATURE       "QUALCOMM"

#define OEM_CAP_MAX_NUM_CHANNELS   128

#ifndef OEM_DATA_RSP_SIZE
#define OEM_DATA_RSP_SIZE 1724
#endif

/**
 * struct app_reg_rsp_vdev_info - vdev info struct
 * @dev_mode: device mode
 * @vdev_id: vdev id
 *
 */
struct qdf_packed app_reg_rsp_vdev_info {
	uint8_t dev_mode;
	uint8_t vdev_id;
};

/**
 * struct wifi_app_reg_rsp - app registration response struct
 * @num_inf: number of interfaces active
 * @vdevs: array indicating all active vdev's information
 *
 */
struct qdf_packed wifi_app_reg_rsp {
	uint8_t num_inf;
	struct app_reg_rsp_vdev_info vdevs[1];
};

/**
 * struct oem_data_req - data request to be sent to firmware
 * @data_len: len of data
 * @data: buffer containing data
 *
 */
struct oem_data_req {
	uint32_t data_len;
	uint8_t *data;
};

/**
 * struct oem_data_rsp - response from firmware to data request sent earlier
 * @data_len: len of data
 * @data: buffer containing data
 *
 */
struct oem_data_rsp {
	uint32_t rsp_len;
	uint8_t *data;
};

/**
 * struct wifi_pos_driver_version - Driver version identifier (w.x.y.z)
 * @major: Version ID major number
 * @minor: Version ID minor number
 * @patch: Version ID patch number
 * @build: Version ID build number
 */
struct qdf_packed wifi_pos_driver_version {
	uint8_t major;
	uint8_t minor;
	uint8_t patch;
	uint8_t build;
};

/**
 * struct wifi_pos_driver_caps - OEM Data Capabilities
 * @oem_target_signature: Signature of chipset vendor, e.g. QUALCOMM
 * @oem_target_type: Chip type
 * @oem_fw_version: Firmware version
 * @driver_version: Host software version
 * @allowed_dwell_time_min: Channel dwell time - allowed minimum
 * @allowed_dwell_time_max: Channel dwell time - allowed maximum
 * @curr_dwell_time_min: Channel dwell time - current minimim
 * @curr_dwell_time_max: Channel dwell time - current maximum
 * @supported_bands: Supported bands, 2.4G or 5G Hz
 * @num_channels: Num of channels IDs to follow
 * @channel_list: List of channel IDs
 */
struct qdf_packed wifi_pos_driver_caps {
	uint8_t oem_target_signature[OEM_TARGET_SIGNATURE_LEN];
	uint32_t oem_target_type;
	uint32_t oem_fw_version;
	struct wifi_pos_driver_version driver_version;
	uint16_t allowed_dwell_time_min;
	uint16_t allowed_dwell_time_max;
	uint16_t curr_dwell_time_min;
	uint16_t curr_dwell_time_max;
	uint16_t supported_bands;
	uint16_t num_channels;
	uint8_t channel_list[OEM_CAP_MAX_NUM_CHANNELS];
};

/**
 * struct wifi_pos_user_defined_caps - OEM capability to be exchanged between host
 * and userspace
 * @ftm_rr: FTM range report capability bit
 * @lci_capability: LCI capability bit
 * @reserved1: reserved
 * @reserved2: reserved
 */
struct wifi_pos_user_defined_caps {
	uint32_t ftm_rr:1;
	uint32_t lci_capability:1;
	uint32_t reserved1:30;
	uint32_t reserved2;
};

/**
 * struct wifi_pos_oem_get_cap_rsp - capabilites set by userspace and target.
 * @driver_cap: target capabilities
 * @user_defined_cap: capabilities set by userspace via set request
 */
struct qdf_packed wifi_pos_oem_get_cap_rsp {
	struct wifi_pos_driver_caps driver_cap;
	struct wifi_pos_user_defined_caps user_defined_cap;
};

/**
 * struct wifi_pos_psoc_priv_obj - psoc obj data for wifi_pos
 * @app_pid: pid of app registered to host driver
 * @is_app_registered: indicates if app is registered
 * @fine_time_meas_cap: FTM cap for different roles, reflection of ini
 * @ftm_rr: configured value of FTM Ranging Request capability
 * @lci_capability: configured value of LCI capability
 * @rsvd: reserved
 * @wifi_pos_req_handler: function pointer to handle TLV or non-TLV
 * wifi pos request messages
 * <----- fine_time_meas_cap (in bits) ----->
 *+----------+-----+-----+------+------+-------+-------+-----+-----+
 *|   8-31   |  7  |  6  |   5  |   4  |   3   |   2   |  1  |  0  |
 *+----------+-----+-----+------+------+-------+-------+-----+-----+
 *| reserved | SAP | SAP |P2P-GO|P2P-GO|P2P-CLI|P2P-CLI| STA | STA |
 *|          |resp |init |resp  |init  |resp   |init   |resp |init |
 *+----------+-----+-----+------+------+-------+-------+-----+-----+
 * resp - responder role; init- initiator role
 *
 */
struct wifi_pos_psoc_priv_obj {
	uint32_t app_pid;
	bool is_app_registered;
	uint32_t fine_time_meas_cap;
	uint32_t ftm_rr:1;
	uint32_t lci_capability:1;
	uint32_t rsvd:30;

	/* following are populated from HDD */
	uint32_t oem_target_type;
	uint32_t oem_fw_version;
	struct wifi_pos_driver_version driver_version;
	uint16_t allowed_dwell_time_min;
	uint16_t allowed_dwell_time_max;
	uint16_t current_dwell_time_min;
	uint16_t current_dwell_time_max;

	qdf_spinlock_t wifi_pos_lock;

	QDF_STATUS (*wifi_pos_req_handler)(struct wlan_objmgr_psoc *psoc,
				    struct wifi_pos_req_msg *req);
	void (*wifi_pos_send_rsp)(uint32_t, uint32_t, uint32_t, uint8_t *);
};

/**
 * wifi_pos_get_psoc_priv_obj: API to get wifi_psoc private object
 * @psoc: pointer to psoc object
 *
 * Return: psoc private object on success, NULL otherwise
 */
struct wifi_pos_psoc_priv_obj *wifi_pos_get_psoc_priv_obj(
		struct wlan_objmgr_psoc *psoc);

/**
 * wifi_pos_set_psoc: API to set global PSOC object
 * @psoc: pointer to psoc object
 *
 * Since request from userspace is not associated with any vdev/pdev/psoc, this
 * API is used to set global psoc object.
 *
 * Return: none.
 */
void wifi_pos_set_psoc(struct wlan_objmgr_psoc *psoc);

/**
 * wifi_pos_get_psoc: API to get global PSOC object
 *
 * Since request from userspace is not associated with any vdev/pdev/psoc, this
 * API is used to get global psoc object.
 * Return: global psoc object.
 */
struct wlan_objmgr_psoc *wifi_pos_get_psoc(void);

/**
 * wifi_pos_get_psoc: API to clear global PSOC object
 *
 * Return: none.
 */
void wifi_pos_clear_psoc(void);

/**
 * wifi_pos_populate_caps: API to get OEM caps
 * @psoc: psoc object
 * @caps: capabilites buffer to populate
 *
 * Return: status of operation.
 */
QDF_STATUS wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
				  struct wifi_pos_driver_caps *caps);

/**
 * wifi_pos_get_app_pid: returns oem app pid.
 * @psoc: pointer to psoc object
 *
 * Return: oem app pid
 */
uint32_t wifi_pos_get_app_pid(struct wlan_objmgr_psoc *psoc);

/**
 * wifi_pos_is_app_registered: indicates if oem app is registered.
 * @psoc: pointer to psoc object
 *
 * Return: true if app is registered, false otherwise
 */
bool wifi_pos_is_app_registered(struct wlan_objmgr_psoc *psoc);

#endif /* _WIFI_POS_UTILS_H_ */
#endif /* WIFI_POS_CONVERGED */
