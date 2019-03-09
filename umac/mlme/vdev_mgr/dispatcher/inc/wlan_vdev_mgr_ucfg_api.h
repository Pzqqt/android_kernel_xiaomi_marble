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
 * DOC: wlan_vdev_mgr_ucfg_api.h
 *
 * This header file provides definitions to data structures required
 * for mlme ucfg and declarations for ucfg public APIs
 */

#ifndef __WLAN_VDEV_MGR_UCFG_API_H__
#define __WLAN_VDEV_MGR_UCFG_API_H__

#include <wlan_objmgr_vdev_obj.h>
#include <wlan_vdev_mgr_tgt_if_tx_defs.h>
#include <qdf_nbuf.h>
#include <include/wlan_vdev_mlme.h>

enum wlan_mlme_cfg_id {
	WLAN_MLME_CFG_DTIM_PERIOD,
	WLAN_MLME_CFG_SLOT_TIME,
	WLAN_MLME_CFG_PROTECTION_MODE,
	WLAN_MLME_CFG_BEACON_INTERVAL,
	WLAN_MLME_CFG_LDPC,
	WLAN_MLME_CFG_NSS,
	WLAN_MLME_CFG_TSF_ADJUST,
	WLAN_MLME_CFG_ASSOC_ID,
	WLAN_MLME_CFG_VHT_CAPS,
	WLAN_MLME_CFG_SUBFER,
	WLAN_MLME_CFG_MUBFER,
	WLAN_MLME_CFG_SUBFEE,
	WLAN_MLME_CFG_MUBFEE,
	WLAN_MLME_CFG_IMLICIT_BF,
	WLAN_MLME_CFG_SOUNDING_DIM,
	WLAN_MLME_CFG_BFEE_STS_CAP,
	WLAN_MLME_CFG_TXBF_CAPS,
	WLAN_MLME_CFG_HT_CAPS,
	WLAN_MLME_CFG_HE_OPS,
	WLAN_MLME_CFG_RTS_THRESHOLD,
	WLAN_MLME_CFG_FRAG_THRESHOLD,
	WLAN_MLME_CFG_PROBE_DELAY,
	WLAN_MLME_CFG_REPEAT_PROBE_TIME,
	WLAN_MLME_CFG_DROP_UNENCRY,
	WLAN_MLME_CFG_TX_PWR_LIMIT,
	WLAN_MLME_CFG_TX_POWER,
	WLAN_MLME_CFG_AMPDU,
	WLAN_MLME_CFG_AMSDU,
	WLAN_MLME_CFG_SSID,
	WLAN_MLME_CFG_SSID_LEN,
	WLAN_MLME_CFG_OP_MODE,
	WLAN_MLME_CFG_BMISS_FIRST_BCNT,
	WLAN_MLME_CFG_BMISS_FINAL_BCNT,
	WLAN_MLME_CFG_MIN_IDLE_INACTIVE_TIME,
	WLAN_MLME_CFG_MAX_IDLE_INACTIVE_TIME,
	WLAN_MLME_CFG_MAX_UNRESPONSIVE_INACTIVE_TIME,
	WLAN_MLME_CFG_RATE_FLAGS,
	WLAN_MLME_CFG_PER_BAND_TX_MGMT_RATE,
	WLAN_MLME_CFG_MAX_RATE,
	WLAN_MLME_CFG_TX_MGMT_RATE,
	WLAN_MLME_CFG_TX_CHAINMASK,
	WLAN_MLME_CFG_RX_CHAINMASK,
	WLAN_MLME_CFG_PKT_POWERSAVE,
	WLAN_MLME_CFG_MAX_LI_OF_MODDTIM,
	WLAN_MLME_CFG_DYNDTIM_CNT,
	WLAN_MLME_CFG_LISTEN_INTERVAL,
	WLAN_MLME_CFG_MODDTIM_CNT,
	WLAN_MLME_CFG_BEACON_BUFFER,
	WLAN_MLME_CFG_BEACON_OFFSETS,
	WLAN_MLME_CFG_PROILE_IDX,
	WLAN_MLME_CFG_PROFILE_NUM,
	WLAN_MLME_CFG_MBSSID_FLAGS,
	WLAN_MLME_CFG_VDEVID_TRANS,
	WLAN_MLME_CFG_TRANS_BSSID,
	WLAN_MLME_CFG_TYPE,
	WLAN_MLME_CFG_SUBTYPE,
	WLAN_MLME_CFG_UAPSD,
	WLAN_MLME_CFG_TX_DECAP_TYPE,
	WLAN_MLME_CFG_RX_DECAP_TYPE,
	WLAN_MLME_CFG_RATEMASK_TYPE,
	WLAN_MLME_CFG_RATEMASK_LOWER32,
	WLAN_MLME_CFG_RATEMASK_HIGHER32,
	WLAN_MLME_CFG_RATEMASK_LOWER32_2,
	WLAN_MLME_CFG_BCN_TX_RATE,
	WLAN_MLME_CFG_2G_VHT,
	WLAN_MLME_CFG_11AX_STUB,
	WLAN_MLME_CFG_RATEMASK_CAPS,
	WLAN_MLME_CFG_MAX
};

/**
 * struct wlan_vdev_mgr_cfg - vdev mgr configuration
 * @value: configuration value
 * @tsf: tsf adjust value
 * @trans_bssid: transmission bssid address
 * @ssid_cfg: ssid configuration
 */
struct wlan_vdev_mgr_cfg {
	union {
		uint32_t value;
		uint64_t tsf;
		uint8_t trans_bssid[QDF_MAC_ADDR_SIZE];
		struct mlme_mac_ssid ssid_cfg;
	};
};

/**
 * ucfg_wlan_vdev_mgr_set_param() – ucfg MLME API to
 * set value into mlme vdev mgr component
 * @vdev: pointer to vdev object
 * @param_id: param of type wlan_mlme_cfg_id
 * @mlme_cfg: value to set into mlme vdev mgr
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ucfg_wlan_vdev_mgr_set_param(struct wlan_objmgr_vdev *vdev,
					enum wlan_mlme_cfg_id param_id,
					struct wlan_vdev_mgr_cfg mlme_cfg);

/**
 * ucfg_wlan_vdev_mgr_get_param() – ucfg MLME API to
 * get value from mlme vdev mgr component
 * @vdev: pointer to vdev object
 * @param_id: param of type wlan_mlme_cfg_id
 * @param_value: pointer to store the value of mlme vdev mgr
 *
 * Return: void
 */
void ucfg_wlan_vdev_mgr_get_param(struct wlan_objmgr_vdev *vdev,
				  enum wlan_mlme_cfg_id param_id,
				  uint32_t *param_value);

/**
 * ucfg_wlan_vdev_mgr_get_param_ssid() – ucfg MLME API to
 * get ssid from mlme vdev mgr component
 * @vdev: pointer to vdev object
 * @ssid: pointer to store the ssid
 * @ssid_len: pointer to store the ssid length value
 *
 * Return: void
 */
void ucfg_wlan_vdev_mgr_get_param_ssid(struct wlan_objmgr_vdev *vdev,
				       uint8_t *ssid,
				       uint8_t *ssid_len);

/**
 * ucfg_wlan_vdev_mgr_get_param_bssid() – ucfg MLME API to
 * get bssid from mlme vdev mgr component
 * @vdev: pointer to vdev object
 * @bssid: pointer to store the bssid
 *
 */
void ucfg_wlan_vdev_mgr_get_param_bssid(
				struct wlan_objmgr_vdev *vdev,
				uint8_t *bssid);

/**
 * ucfg_wlan_vdev_mgr_get_beacon_buffer() – ucfg MLME API to
 * get beacon buffer from mlme vdev mgr component
 * @vdev: pointer to vdev object
 * @buf: pointer to store the beacon buffer
 *
 * Return: void
 */
void ucfg_wlan_vdev_mgr_get_beacon_buffer(struct wlan_objmgr_vdev *vdev,
					  qdf_nbuf_t buf);

/**
 * ucfg_wlan_vdev_mgr_get_trans_bssid() – ucfg MLME API to
 * get transmission bssid from mlme vdev mgr component
 * @vdev: pointer to vdev object
 * @addr: pointer to store the transmission bssid
 *
 * Return: void
 */
void ucfg_wlan_vdev_mgr_get_trans_bssid(struct wlan_objmgr_vdev *vdev,
					uint8_t *addr);

/**
 * ucfg_wlan_vdev_mgr_get_tsf_adjust() – ucfg MLME API to
 * get tsf_adjust from mlme vdev mgr component
 * @vdev: pointer to vdev object
 * @tsf_adjust: pointer to store the tsf adjust value
 *
 * Return: void
 */
void ucfg_wlan_vdev_mgr_get_tsf_adjust(struct wlan_objmgr_vdev *vdev,
				       uint64_t *tsf_adjust);

#endif /* __WLAN_VDEV_MLME_UCFG_H__ */
