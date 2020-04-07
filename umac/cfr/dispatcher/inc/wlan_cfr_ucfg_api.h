/*
 * Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_CFR_UCFG_API_H_
#define _WLAN_CFR_UCFG_API_H_

#include <wlan_objmgr_peer_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_cfr_public_structs.h>

#define MAX_CFR_PRD        (10*60*1000)        /* 10 minutes */

/**
 * ucfg_cfr_start_capture() - function to start cfr capture for connected client
 * @pdev: pointer to pdev object
 * @peer: pointer to peer object
 * @cfr_params: config params to cfr capture
 *
 * Return: status of start capture.
 */
int ucfg_cfr_start_capture(struct wlan_objmgr_pdev *pdev,
			   struct wlan_objmgr_peer *peer,
			   struct cfr_capture_params *cfr_params);

/**
 * ucfg_cfr_stop_capture() - function to stop cfr capture for connected client
 * @pdev: pointer to pdev object
 * @peer: pointer to peer object
 *
 * Return: status of stop capture.
 */
int ucfg_cfr_stop_capture(struct wlan_objmgr_pdev *pdev,
			  struct wlan_objmgr_peer *peer);

/**
 * ucfg_cfr_start_capture_probe_req() - function to start cfr capture for
 *					unassociated clients
 * @pdev: pointer to pdev object
 * @unassoc_mac: mac address of un-associated client
 * @cfr_params: config params to cfr capture
 *
 * Return: status of start capture.
 */
int ucfg_cfr_start_capture_probe_req(struct wlan_objmgr_pdev *pdev,
				     struct qdf_mac_addr *unassoc_mac,
				     struct cfr_capture_params *params);

/**
 * ucfg_cfr_stop_capture_probe_req() - function to stop cfr capture for
 *				       unassociated cleints
 * @pdev: pointer to pdev object
 * @unassoc_mac: mac address of un-associated client
 *
 * Return: status of stop capture.
 */
int ucfg_cfr_stop_capture_probe_req(struct wlan_objmgr_pdev *pdev,
				    struct qdf_mac_addr *unassoc_mac);

/**
 * ucfg_cfr_list_peers() - Lists total number of peers with cfr capture enabled
 * @pdev: pointer to pdev object
 *
 * Return: number of peers with cfr capture enabled
 */
int ucfg_cfr_list_peers(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_cfr_set_timer() - function to enable cfr timer
 * @pdev: pointer to pdev object
 * @value: value to be set
 *
 * Return: status of timer enable
 */
int ucfg_cfr_set_timer(struct wlan_objmgr_pdev *pdev, uint32_t value);

/**
 * ucfg_cfr_get_timer() - function to get cfr_timer_enable
 * @pdev: pointer to pdev object
 *
 * Return: value of cfr_timer_enable
 */
int ucfg_cfr_get_timer(struct wlan_objmgr_pdev *pdev);

/**
 * ucfg_cfr_stop_indication() - User space API to write cfr stop string
 * @vdev - pointer to vdev object
 *
 * Write stop string and indicate to up layer.
 *
 * Return: status of write CFR stop string
 */
QDF_STATUS ucfg_cfr_stop_indication(struct wlan_objmgr_vdev *vdev);

#ifdef WLAN_ENH_CFR_ENABLE
/* Channel capture recipe filters */
enum capture_type {
	RCC_DIRECTED_FTM_FILTER,
	RCC_ALL_FTM_ACK_FILTER,
	RCC_DIRECTED_NDPA_NDP_FILTER,
	RCC_NDPA_NDP_ALL_FILTER,
	RCC_TA_RA_FILTER,
	RCC_ALL_PACKET_FILTER,
	RCC_DIS_ALL_MODE,
};

/**
 * ucfg_cfr_set_rcc_mode() - function to set RCC mode
 * @vdev: pointer to vdev object
 * @mode: capture type passed by user
 * @value: Enable/Disable capture mode
 *
 * Return: status if the mode is set or not
 */
QDF_STATUS ucfg_cfr_set_rcc_mode(struct wlan_objmgr_vdev *vdev,
				 enum capture_type mode, uint8_t value);

/**
 * ucfg_cfr_get_rcc_enabled() - function to get RCC mode
 * @vdev: pointer to vdev object
 *
 * Return: if the rcc is enabled or not
 */
bool ucfg_cfr_get_rcc_enabled(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cfr_set_tara_config() - function to configure TA/RA address and mask
 * @vdev: pointer to vdev object
 * @params: user config
 *
 * Return: status
 */
QDF_STATUS ucfg_cfr_set_tara_config(struct wlan_objmgr_vdev *vdev,
				    struct cfr_wlanconfig_param *params);

/**
 * ucfg_cfr_set_bw_nss() - function to configure nss and bandwidth
 * @vdev: pointer to vdev object
 * @params: user config
 *
 * Return: status
 */
QDF_STATUS ucfg_cfr_set_bw_nss(struct wlan_objmgr_vdev *vdev,
			       struct cfr_wlanconfig_param *params);

/**
 * ucfg_cfr_set_frame_type_subtype() - function to configure frame type/subtype
 * @vdev: pointer to vdev object
 * @params: user config
 *
 * Return: status
 */
QDF_STATUS
ucfg_cfr_set_frame_type_subtype(struct wlan_objmgr_vdev *vdev,
				struct cfr_wlanconfig_param *params);

/**
 * ucfg_cfr_set_capture_duration() - function to configure capture duration
 * @vdev: pointer to vdev object
 * @params: user config
 *
 * Return: status
 */
QDF_STATUS
ucfg_cfr_set_capture_duration(struct wlan_objmgr_vdev *vdev,
			      struct cfr_wlanconfig_param *params);

/**
 * ucfg_cfr_set_capture_interval() - function to configure capture interval
 * @vdev: pointer to vdev object
 * @params: user config
 *
 * Return: status
 */
QDF_STATUS
ucfg_cfr_set_capture_interval(struct wlan_objmgr_vdev *vdev,
			      struct cfr_wlanconfig_param *params);

/**
 * ucfg_cfr_set_en_bitmap() - function to configure 16-bit bitmap in TA_RA mode
 * @vdev: pointer to vdev object
 * @params: user config
 *
 * Return: status
 */
QDF_STATUS ucfg_cfr_set_en_bitmap(struct wlan_objmgr_vdev *vdev,
				  struct cfr_wlanconfig_param *params);

/**
 * ucfg_cfr_set_reset_bitmap() - function to clear all 9 params for all 16
 * groups in TA_RA mode
 * @vdev: pointer to vdev object
 * @params: user config
 *
 * Return: status
 */
QDF_STATUS ucfg_cfr_set_reset_bitmap(struct wlan_objmgr_vdev *vdev,
				     struct cfr_wlanconfig_param *params);

/**
 * ucfg_cfr_set_ul_mu_user_mask() - function to configure UL MU user mask
 * @vdev: pointer to vdev object
 * @params: user config
 *
 * Return: status
 */
QDF_STATUS
ucfg_cfr_set_ul_mu_user_mask(struct wlan_objmgr_vdev *vdev,
			     struct cfr_wlanconfig_param *params);

/**
 * ucfg_cfr_set_freeze_tlv_delay_cnt() - function to configure freeze TLV delay
 * count threshold
 * @vdev: pointer to vdev object
 * @params: user config
 *
 * Return: status
 */
QDF_STATUS
ucfg_cfr_set_freeze_tlv_delay_cnt(struct wlan_objmgr_vdev *vdev,
				  struct cfr_wlanconfig_param *params);

/**
 * ucfg_cfr_committed_rcc_config() - function to commit user config
 * @vdev: pointer to vdev object
 *
 * Return: status
 */
QDF_STATUS ucfg_cfr_committed_rcc_config(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cfr_get_cfg() - function to display user config
 * @vdev: pointer to vdev object
 *
 * Return: status
 */
QDF_STATUS ucfg_cfr_get_cfg(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cfr_rcc_dump_dbg_counters() - function to display PPDU counters
 * @vdev: pointer to vdev object
 *
 * Return: status
 */
QDF_STATUS ucfg_cfr_rcc_dump_dbg_counters(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cfr_rcc_clr_dbg_counters() - function to clear CFR PPDU counters
 * @vdev: pointer to vdev object
 *
 * Return: status
 */
QDF_STATUS ucfg_cfr_rcc_clr_dbg_counters(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cfr_rcc_dump_lut() - function to display lookup table
 * @vdev: pointer to vdev object
 *
 * Return: status
 */
QDF_STATUS ucfg_cfr_rcc_dump_lut(struct wlan_objmgr_vdev *vdev);

/**
 * ucfg_cfr_subscribe_ppdu_desc() - User space interface to
 * subscribe/unsubscribe WDI PPDU desc event
 * @pdev: pointer to pdev_object
 * @is_subscribe: subscribe or unsubscribei
 *
 * return QDF status
 */
QDF_STATUS ucfg_cfr_subscribe_ppdu_desc(struct wlan_objmgr_pdev *pdev,
					bool is_subscribe);
#endif
#endif
