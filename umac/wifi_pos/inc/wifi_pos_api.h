/*
 * Copyright (c) 2012-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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
 * DOC: wifi_pos_api.h
 * This file declares public APIs of wifi positioning component
 */
#ifndef _WIFI_POS_API_H_
#define _WIFI_POS_API_H_

/* Include files */
#include "wifi_pos_utils_pub.h"
#include "../src/wifi_pos_utils_i.h"

/* forward reference */
struct wlan_objmgr_psoc;
struct wifi_pos_driver_caps;

/**
 * enum RTT_FIELD_ID - identifies which field is being specified
 * @META_DATA_SUB_TYPE: oem data req sub type
 * @META_DATA_CHANNEL_MHZ: channel mhz info
 * @META_DATA_PDEV: pdev info
 */
enum RTT_FIELD_ID {
	META_DATA_SUB_TYPE,
	META_DATA_CHANNEL_MHZ,
	META_DATA_PDEV,
};

/**
 * struct wifi_pos_field - wifi positioning field element
 * @id: RTT field id
 * @offset: data offset in field info buffer
 * @length: length of related data in field info buffer
 */
struct wifi_pos_field {
	uint32_t id;
	uint32_t offset;
	uint32_t length;
};

/**
 * struct wifi_pos_field_info - wifi positioning field info buffer
 * @count: number of @wifi_pos_field elements
 * @fields: buffer to hold @wifi_pos_field elements
 */
struct wifi_pos_field_info {
	uint32_t count;
	QDF_FLEX_ARRAY(struct wifi_pos_field, fields);
};

/* Length of interface name */
#define INTERFACE_LEN 16
/**
 * struct wifi_pos_interface - wifi positioning interface structure
 * @length: interface length
 * @dev_name: device name
 */
struct wifi_pos_interface {
	uint8_t length;
	char dev_name[INTERFACE_LEN];
};

#ifdef WIFI_POS_CONVERGED
/**
 * enum oem_err_msg - err msg returned to user space
 * @OEM_ERR_NULL_CONTEXT: NULL context
 * @OEM_ERR_APP_NOT_REGISTERED: OEM App is not registered
 * @OEM_ERR_INVALID_SIGNATURE: Invalid signature
 * @OEM_ERR_NULL_MESSAGE_HEADER: Invalid message header
 * @OEM_ERR_INVALID_MESSAGE_TYPE: Invalid message type
 * @OEM_ERR_INVALID_MESSAGE_LENGTH: Invalid length in message body
 * @OEM_ERR_REQUEST_REJECTED: Request is rejected by the driver
 */
enum oem_err_msg {
	OEM_ERR_NULL_CONTEXT = 1,
	OEM_ERR_APP_NOT_REGISTERED,
	OEM_ERR_INVALID_SIGNATURE,
	OEM_ERR_NULL_MESSAGE_HEADER,
	OEM_ERR_INVALID_MESSAGE_TYPE,
	OEM_ERR_INVALID_MESSAGE_LENGTH,
	OEM_ERR_REQUEST_REJECTED
};

/* this struct is needed since MLME is not converged yet */
struct wifi_pos_ch_info {
	uint8_t chan_id;
	uint32_t mhz;
	uint32_t band_center_freq1;
	uint32_t band_center_freq2;
	uint32_t info;
	uint32_t reg_info_1;
	uint32_t reg_info_2;
	uint8_t nss;
	uint32_t rate_flags;
	uint8_t sec_ch_offset;
	uint32_t ch_width;
};

/**
 * typedef wifi_pos_ch_info_rsp - Channel information
 * @chan_id: channel id
 * @reserved0: reserved for padding and future use
 * @mhz: primary 20 MHz channel frequency in mhz
 * @band_center_freq1: Center frequency 1 in MHz
 * @band_center_freq2: Center frequency 2 in MHz, valid only for 11ac
 *      VHT 80+80 mode
 * @info: channel info
 * @reg_info_1: regulatory information field 1 which contains min power,
 *      max power, reg power and reg class id
 * @reg_info_2: regulatory information field 2 which contains antennamax
 */
struct qdf_packed wifi_pos_ch_info_rsp {
	uint32_t chan_id;
	uint32_t reserved0;
	uint32_t mhz;
	uint32_t band_center_freq1;
	uint32_t band_center_freq2;
	uint32_t info;
	uint32_t reg_info_1;
	uint32_t reg_info_2;
};

/**
 * struct wifi_pos_peer_status_info - Status information for a given peer
 * @peer_mac_addr: peer mac address
 * @peer_status: peer status: 1: CONNECTED, 2: DISCONNECTED
 * @vdev_id: vdev_id for the peer mac
 * @peer_capability: peer capability: 0: RTT/RTT2, 1: RTT3. Default is 0
 * @reserved0: reserved0
 * @peer_chan_info: channel info on which peer is connected
 */
struct qdf_packed wifi_pos_peer_status_info {
	uint8_t peer_mac_addr[ETH_ALEN];
	uint8_t peer_status;
	uint8_t vdev_id;
	uint32_t peer_capability;
	uint32_t reserved0;
	struct wifi_pos_ch_info_rsp peer_chan_info;
};

/**
 * struct wifi_pos_req_msg - wifi pos request struct
 * @msg_type: message type
 * @pid: process id
 * @buf: request buffer
 * @buf_len: request buffer length
 * @field_info_buf: buffer containing field info
 * @field_info_buf_len: length of field info buffer
 * @rsp_version: nl type or ani type
 * @interface: contains interface name and length
 */
struct wifi_pos_req_msg {
	enum wifi_pos_cmd_ids msg_type;
	uint32_t pid;
	uint8_t *buf;
	uint32_t buf_len;
	struct wifi_pos_field_info *field_info_buf;
	uint32_t field_info_buf_len;
	uint32_t rsp_version;
	struct wifi_pos_interface interface;
};

/**
 * ucfg_wifi_pos_process_req: ucfg API to be called from HDD/OS_IF to process a
 * wifi_pos request from userspace
 * @psoc: pointer to psoc object
 * @req: wifi_pos request msg
 * @send_rsp_cb: callback pointer required to send msg to userspace
 *
 * Return: status of operation
 */
QDF_STATUS ucfg_wifi_pos_process_req(struct wlan_objmgr_psoc *psoc,
				     struct wifi_pos_req_msg *req,
				     wifi_pos_send_rsp_handler send_rsp_cb);

/**
 * wifi_pos_init: initializes WIFI POS component, called by dispatcher init
 *
 * Return: status of operation
 */
QDF_STATUS wifi_pos_init(void);

/**
 * wifi_pos_deinit: de-initializes WIFI POS component, called by dispatcher init
 *
 * Return: status of operation
 */
QDF_STATUS wifi_pos_deinit(void);

/**
 * wifi_pos_psoc_enable: psoc enable API for wifi positioning component
 * @psoc: pointer to PSOC
 *
 * Return: status of operation
 */
QDF_STATUS wifi_pos_psoc_enable(struct wlan_objmgr_psoc *psoc);

/**
 * wifi_pos_psoc_disable: psoc disable API for wifi positioning component
 * @psoc: pointer to PSOC
 *
 * Return: status of operation
 */
QDF_STATUS wifi_pos_psoc_disable(struct wlan_objmgr_psoc *psoc);

/**
 * wifi_pos_set_oem_target_type: public API to set param in wifi_pos private
 * object
 * @psoc: pointer to PSOC
 * @val: value to set
 *
 * Return: None
 */
void wifi_pos_set_oem_target_type(struct wlan_objmgr_psoc *psoc, uint32_t val);

/**
 * wifi_pos_set_oem_fw_version: public API to set param in wifi_pos private
 * object
 * @psoc: pointer to PSOC
 * @val: value to set
 *
 * Return: None
 */
void wifi_pos_set_oem_fw_version(struct wlan_objmgr_psoc *psoc, uint32_t val);

/**
 * wifi_pos_set_drv_ver_major: public API to set param in wifi_pos private
 * object
 * @psoc: pointer to PSOC
 * @val: value to set
 *
 * Return: None
 */
void wifi_pos_set_drv_ver_major(struct wlan_objmgr_psoc *psoc, uint8_t val);

/**
 * wifi_pos_set_drv_ver_minor: public API to set param in wifi_pos private
 * object
 * @psoc: pointer to PSOC
 * @val: value to set
 *
 * Return: None
 */
void wifi_pos_set_drv_ver_minor(struct wlan_objmgr_psoc *psoc, uint8_t val);

/**
 * wifi_pos_set_drv_ver_patch: public API to set param in wifi_pos private
 * object
 * @psoc: pointer to PSOC
 * @val: value to set
 *
 * Return: None
 */
void wifi_pos_set_drv_ver_patch(struct wlan_objmgr_psoc *psoc, uint8_t val);

/**
 * wifi_pos_set_drv_ver_build: public API to set param in wifi_pos private
 * object
 * @psoc: pointer to PSOC
 * @val: value to set
 *
 * Return: None
 */
void wifi_pos_set_drv_ver_build(struct wlan_objmgr_psoc *psoc, uint8_t val);

/**
 * wifi_pos_set_dwell_time_min: public API to set param in wifi_pos private
 * object
 * @psoc: pointer to PSOC
 * @val: value to set
 *
 * Return: None
 */
void wifi_pos_set_dwell_time_min(struct wlan_objmgr_psoc *psoc, uint16_t val);

/**
 * wifi_pos_set_dwell_time_max: public API to set param in wifi_pos private
 * object
 * @psoc: pointer to PSOC
 * @val: value to set
 *
 * Return: None
 */
void wifi_pos_set_dwell_time_max(struct wlan_objmgr_psoc *psoc, uint16_t val);

/**
 * wifi_pos_set_current_dwell_time_min: public API to set param in wifi_pos
 * private object
 * @psoc: pointer to PSOC
 * @val: value to set
 *
 * Return: None
 */
void wifi_pos_set_current_dwell_time_min(struct wlan_objmgr_psoc *psoc,
					 uint16_t val);

/**
 * wifi_pos_set_current_dwell_time_max: public API to set param in wifi_pos
 * private object
 * @psoc: pointer to PSOC
 * @val: value to set
 *
 * Return: None
 */
void wifi_pos_set_current_dwell_time_max(struct wlan_objmgr_psoc *psoc,
					 uint16_t val);

/**
 * wifi_pos_populate_caps() - populate oem capabilities
 * @psoc: psoc object
 * @caps: pointer to populate the capabilities
 *
 * Return: error code
 */
QDF_STATUS wifi_pos_populate_caps(struct wlan_objmgr_psoc *psoc,
			   struct wifi_pos_driver_caps *caps);

struct wlan_lmac_if_rx_ops;
/**
 * wifi_pos_register_rx_ops: function to register with lmac rx ops
 * @rx_ops: lmac rx ops struct object
 *
 * Return: None
 */
void wifi_pos_register_rx_ops(struct wlan_lmac_if_rx_ops *rx_ops);

/**
 * ucfg_wifi_pos_get_ftm_cap: API to get fine timing measurement caps
 * @psoc: psoc object
 *
 * Return: FTM value
 */
uint32_t ucfg_wifi_pos_get_ftm_cap(struct wlan_objmgr_psoc *psoc);

/**
 * ucfg_wifi_pos_set_ftm_cap: API to set fine timing measurement caps
 * @psoc: psoc object
 * @val: value to set
 *
 * Return: None
 */
void ucfg_wifi_pos_set_ftm_cap(struct wlan_objmgr_psoc *psoc, uint32_t val);

/**
 * ucfg_wifi_pos_set_oem_6g_supported: API to set oem target 6g enabled/disabled
 * @psoc: psoc object
 * @val: value to set
 *
 * Return: None
 */
void ucfg_wifi_pos_set_oem_6g_supported(struct wlan_objmgr_psoc *psoc,
					bool val);

/**
 * ucfg_wifi_pos_is_nl_rsp: API to check if response is nl or ani type
 * @psoc: psoc object
 *
 * Return: true if response is nl type
 */
bool ucfg_wifi_pos_is_nl_rsp(struct wlan_objmgr_psoc *psoc);

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

/**
 * wifi_pos_get_psoc: API to get global PSOC object
 *
 * Since request from userspace is not associated with any vdev/pdev/psoc, this
 * API is used to get global psoc object.
 * Return: global psoc object.
 */
struct wlan_objmgr_psoc *wifi_pos_get_psoc(void);

#else
static inline QDF_STATUS wifi_pos_init(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wifi_pos_deinit(void)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wifi_pos_psoc_enable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wifi_pos_psoc_disable(struct wlan_objmgr_psoc *psoc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

#if defined(WLAN_FEATURE_CIF_CFR) && defined(WIFI_POS_CONVERGED)
/**
 * wifi_pos_init_cir_cfr_rings: API to set DMA ring cap in wifi pos psoc private
 * object
 * @psoc: pointer to psoc object
 * @hal_soc: hal soc pointer
 * @num_mac: number of macs
 * @buf: buffer containing dma ring cap
 *
 * Return: status of operation.
 */
QDF_STATUS wifi_pos_init_cir_cfr_rings(struct wlan_objmgr_psoc *psoc,
				   void *hal_soc, uint8_t num_mac, void *buf);
#else
static inline QDF_STATUS wifi_pos_init_cir_cfr_rings(
				struct wlan_objmgr_psoc *psoc,
				void *hal_soc, uint8_t num_mac, void *buf)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/**
 * wifi_pos_register_get_fw_phy_mode_for_freq_cb: API to register callback
 * to get current PHY mode
 * @psoc: pointer to psoc object
 * @handler: callback to be registered
 *
 * Return: QDF_STATUS_SUCCESS in case of success, error codes in
 * case of failure
 */
QDF_STATUS wifi_pos_register_get_fw_phy_mode_for_freq_cb(
			struct wlan_objmgr_psoc *psoc,
			void (*handler)(uint32_t, uint32_t, uint32_t *));

/**
 * wifi_pos_register_get_phy_mode_cb: API to register callback to get
 * current PHY mode
 * @psoc: pointer to psoc object
 * @handler: callback to be registered
 *
 * Return: QDF_STATUS_SUCCESS in case of success, error codes in
 * case of failure
 */
QDF_STATUS wifi_pos_register_get_phy_mode_cb(
			struct wlan_objmgr_psoc *psoc,
			void (*handler)(qdf_freq_t, uint32_t, uint32_t *));

/**
 * wifi_pos_register_send_action: API to register callback to send
 * action frames
 * @psoc: pointer to psoc object
 * @handler: callback to be registered
 *
 * Return: QDF_STATUS_SUCCESS in case of success, error codes in
 * case of failure
 */
QDF_STATUS wifi_pos_register_send_action(
				struct wlan_objmgr_psoc *psoc,
				void (*handler)(struct wlan_objmgr_psoc *psoc,
						uint32_t sub_type,
						uint8_t *buf,
						uint32_t buf_len));

#ifndef CNSS_GENL
/**
 * ucfg_wifi_psoc_get_pdev_id_by_dev_name: ucfg API to get pdev_id and psoc from
 *                                         devname.
 * @dev_name: dev name received from LOWI application
 * @pdev_id: get pdev_id from dev_name
 * @psoc: get psoc corresponding psoc from dev_name
 */
QDF_STATUS ucfg_wifi_psoc_get_pdev_id_by_dev_name(
		char *dev_name, uint8_t *pdev_id,
		struct wlan_objmgr_psoc **psoc);

/**
 * wifi_pos_register_get_pdev_id_by_dev_name: API to register callback to get
 * pdev_id from dev name
 * @psoc: pointer to global psoc object
 * @handler: callback to be registered
 *
 * Return: QDF_STATUS_SUCCESS in case of success, error codes in
 * case of failure
 */
QDF_STATUS wifi_pos_register_get_pdev_id_by_dev_name(
		struct wlan_objmgr_psoc *psoc,
		QDF_STATUS (*handler)(char *dev_name, uint8_t *pdev_id,
				      struct wlan_objmgr_psoc **psoc));
#endif /* CNSS_GENL */

#if !defined(CNSS_GENL) && defined(WLAN_RTT_MEASUREMENT_NOTIFICATION)
/**
 * ucfg_wifi_pos_measurement_request_notification: ucfg API to notify
 * measurement request received from the LOWI application
 * @pdev: Pointer to pdev structure
 * @req: Pointer to wifi_pos_req_msg structure
 *
 * Return: QDF_STATUS_SUCCESS in case of success, error codes in
 * case of failure
 */
QDF_STATUS ucfg_wifi_pos_measurement_request_notification(
		struct wlan_objmgr_pdev *pdev,
		struct wifi_pos_req_msg *req);

/**
 * wifi_pos_register_measurement_request_notification: API to register a
 * callback that needs to be called when the driver receives a measurement
 * request from the LOWI application.
 * @psoc: pointer to global psoc object
 * @handler: callback to be registered
 *
 * Return: QDF_STATUS_SUCCESS in case of success, error codes in case of
 * failure.
 */
QDF_STATUS wifi_pos_register_measurement_request_notification(
		struct wlan_objmgr_psoc *psoc,
		QDF_STATUS (*handler)(struct wlan_objmgr_pdev *pdev,
				      struct rtt_channel_info *chinfo));
#else
static inline QDF_STATUS ucfg_wifi_pos_measurement_request_notification(
		struct wlan_objmgr_pdev *pdev,
		struct wifi_pos_req_msg *req)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS wifi_pos_register_measurement_request_notification(
		struct wlan_objmgr_psoc *psoc,
		QDF_STATUS (*handler)(struct wlan_objmgr_pdev *pdev,
				      struct rtt_channel_info *chinfo))
{
	return QDF_STATUS_SUCCESS;
}
#endif /*!defined(CNSS_GENL) && defined(WLAN_RTT_MEASUREMENT_NOTIFICATION)*/

/**
 * wifi_pos_send_report_resp: Send report to osif
 * @psoc: pointer to psoc object
 * @req_id: Request id
 * @dest_mac: destination mac address
 * @err_code: Error code to be sent
 *
 * Return: QDF_STATUS_SUCCESS in case of success, error codes in
 * case of failure
 */
QDF_STATUS wifi_pos_send_report_resp(struct wlan_objmgr_psoc *psoc,
				     int req_id, uint8_t *dest_mac,
				     int err_code);

/**
 * wifi_pos_convert_host_pdev_id_to_target: convert host pdev_id to target
 * pdev_id
 * @psoc: pointer to psoc object
 * @host_pdev_id: host pdev id
 * @target_pdev_id: target pdev id
 *
 * Return: QDF_STATUS_SUCCESS in case of success, error codes in
 * case of failure
 */
QDF_STATUS wifi_pos_convert_host_pdev_id_to_target(
	struct wlan_objmgr_psoc *psoc, uint32_t host_pdev_id,
	uint32_t *target_pdev_id);
#endif
