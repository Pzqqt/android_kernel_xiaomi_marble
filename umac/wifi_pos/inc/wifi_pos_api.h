/*
 * Copyright (c) 2012-2018 The Linux Foundation. All rights reserved.
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
#include "qdf_types.h"
#include "qdf_status.h"
#include "qdf_trace.h"

/* forward reference */
struct wlan_objmgr_psoc;
struct wifi_pos_driver_caps;

#ifdef WIFI_POS_CONVERGED
/**
 * enum oem_err_msg - err msg returned to user space
 * @OEM_ERR_NULL_CONTEXT: NULL context
 * @OEM_ERR_APP_NOT_REGISTERED: OEM App is not registered
 * @OEM_ERR_INVALID_SIGNATURE: Invalid signature
 * @OEM_ERR_NULL_MESSAGE_HEADER: Invalid message header
 * @OEM_ERR_INVALID_MESSAGE_TYPE: Invalid message type
 * @OEM_ERR_INVALID_MESSAGE_LENGTH: Invalid length in message body
 */
enum oem_err_msg {
	OEM_ERR_NULL_CONTEXT = 1,
	OEM_ERR_APP_NOT_REGISTERED,
	OEM_ERR_INVALID_SIGNATURE,
	OEM_ERR_NULL_MESSAGE_HEADER,
	OEM_ERR_INVALID_MESSAGE_TYPE,
	OEM_ERR_INVALID_MESSAGE_LENGTH
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
 * struct wmi_pos_peer_status_info - Status information for a given peer
 * @peer_mac_addr: peer mac address
 * @peer_status: peer status: 1: CONNECTED, 2: DISCONNECTED
 * @vdev_id: vdev_id for the peer mac
 * @peer_capability: peer capability: 0: RTT/RTT2, 1: RTT3. Default is 0
 * @reserved0: reserved0
 * @peer_chan_info: channel info on which peer is connected
 */
struct qdf_packed wmi_pos_peer_status_info {
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
 *
 */
struct wifi_pos_req_msg {
	uint32_t msg_type;
	uint32_t pid;
	uint8_t *buf;
	uint32_t buf_len;
	struct wifi_pos_field_info *field_info_buf;
	uint32_t field_info_buf_len;
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
		void (*send_rsp_cb)(uint32_t, uint32_t, uint32_t, uint8_t *));

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
			void (*handler)(uint8_t, uint32_t, uint32_t *));

#endif
