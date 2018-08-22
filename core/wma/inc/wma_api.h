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

#ifndef WMA_API_H
#define WMA_API_H

#include "osdep.h"
#include "ani_global.h"
#include "a_types.h"
#include "osapi_linux.h"
#include "wmi_unified.h"
#ifdef NOT_YET
#include "htc_api.h"
#endif
#include "lim_global.h"
#include "cds_utils.h"
#include "scheduler_api.h"
#include "wlan_policy_mgr_api.h"
#include "wma_sar_public_structs.h"
#include <cdp_txrx_ops.h>

typedef void *WMA_HANDLE;

/**
 * enum GEN_PARAM - general parameters
 * @GEN_VDEV_PARAM_AMPDU: Set ampdu size
 * @GEN_VDEV_PARAM_AMSDU: Set amsdu size
 * @GEN_PARAM_CRASH_INJECT: inject crash
 * @GEN_PARAM_MODULATED_DTIM: moduled dtim
 * @GEN_PARAM_CAPTURE_TSF: read tsf
 * @GEN_PARAM_RESET_TSF_GPIO: reset tsf gpio
 * @GEN_VDEV_ROAM_SYNCH_DELAY: roam sync delay
 * @GEN_PARAM_LISTEN_INTERVAL: listen interval
 */
enum GEN_PARAM {
	GEN_VDEV_PARAM_AMPDU = 0x1,
	GEN_VDEV_PARAM_AMSDU,
	GEN_PARAM_CRASH_INJECT,
	GEN_PARAM_MODULATED_DTIM,
	GEN_PARAM_CAPTURE_TSF,
	GEN_PARAM_RESET_TSF_GPIO,
	GEN_VDEV_ROAM_SYNCH_DELAY,
	GEN_PARAM_LISTEN_INTERVAL,
};

/**
 * struct wma_caps_per_phy - various caps per phy
 * @ht_2g: entire HT cap for 2G band in terms of 32 bit flag
 * @ht_5g: entire HT cap for 5G band in terms of 32 bit flag
 * @vht_2g: entire VHT cap for 2G band in terms of 32 bit flag
 * @vht_5g: entire VHT cap for 5G band in terms of 32 bit flag
 * @he_2g: entire HE cap for 2G band in terms of 32 bit flag
 * @he_5g: entire HE cap for 5G band in terms of 32 bit flag
 * @tx_chain_mask_2G: tx chain mask for 2g
 * @rx_chain_mask_2G: rx chain mask for 2g
 * @tx_chain_mask_5G: tx chain mask for 5g
 * @rx_chain_mask_5G: rx chain mask for 5g
 */
struct wma_caps_per_phy {
	uint32_t ht_2g;
	uint32_t ht_5g;
	uint32_t vht_2g;
	uint32_t vht_5g;
	uint32_t he_2g[PSOC_HOST_MAX_MAC_SIZE];
	uint32_t he_5g[PSOC_HOST_MAX_MAC_SIZE];
	uint32_t tx_chain_mask_2G;
	uint32_t rx_chain_mask_2G;
	uint32_t tx_chain_mask_5G;
	uint32_t rx_chain_mask_5G;
};


#define VDEV_CMD 1
#define PDEV_CMD 2
#define GEN_CMD  3
#define DBG_CMD  4
#define PPS_CMD  5
#define QPOWER_CMD 6
#define GTX_CMD  7

typedef void (*wma_peer_authorized_fp) (uint32_t vdev_id);


QDF_STATUS wma_pre_start(void);

QDF_STATUS wma_mc_process_handler(struct scheduler_msg *msg);

QDF_STATUS wma_start(void);

/**
 * wma_stop() - wma stop function.
 *
 * Performs all of the operations required to stop the WMA layer
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF Error on failure
 */
QDF_STATUS wma_stop(void);

QDF_STATUS wma_close(void);

QDF_STATUS wma_wmi_service_close(void);

QDF_STATUS wma_wmi_work_close(void);

int wma_rx_ready_event(void *handle, uint8_t *ev, uint32_t len);

int  wma_rx_service_ready_event(void *handle, uint8_t *ev, uint32_t len);

int wma_rx_service_ready_ext_event(void *handle, uint8_t *ev, uint32_t len);

void wma_setneedshutdown(void);

bool wma_needshutdown(void);

QDF_STATUS wma_wait_for_ready_event(WMA_HANDLE handle);

uint8_t wma_map_channel(uint8_t mapChannel);

int wma_cli_get_command(int vdev_id, int param_id, int vpdev);
int wma_cli_set_command(int vdev_id, int param_id, int sval, int vpdev);
int wma_cli_set2_command(int vdev_id, int param_id, int sval1,
			 int sval2, int vpdev);

QDF_STATUS wma_set_htconfig(uint8_t vdev_id, uint16_t ht_capab, int value);

/**
 * wma_get_wcnss_software_version() - get wcnss software version
 * @version: version pointer
 * @version_buffer_size: buffer size
 *
 * Return: QDF_STATUS_SUCCESS for success or error code
 */
QDF_STATUS wma_get_wcnss_software_version(uint8_t *version,
					  uint32_t version_buffer_size);

void wma_set_peer_authorized_cb(void *wma_ctx, wma_peer_authorized_fp auth_cb);
QDF_STATUS wma_set_peer_param(void *wma_ctx, uint8_t *peer_addr,
		  uint32_t param_id,
		  uint32_t param_value, uint32_t vdev_id);
QDF_STATUS wma_get_link_speed(WMA_HANDLE handle, tSirLinkSpeedInfo *pLinkSpeed);
#ifdef NOT_YET
QDF_STATUS wma_update_channel_list(WMA_HANDLE handle, void *scan_chan_info);
#endif

uint8_t *wma_get_vdev_address_by_vdev_id(uint8_t vdev_id);
struct wma_txrx_node *wma_get_interface_by_vdev_id(uint8_t vdev_id);
QDF_STATUS wma_get_connection_info(uint8_t vdev_id,
		struct policy_mgr_vdev_entry_info *conn_table_entry);

bool wma_is_vdev_up(uint8_t vdev_id);

void *wma_get_beacon_buffer_by_vdev_id(uint8_t vdev_id, uint32_t *buffer_size);

bool wma_get_fw_wlan_feat_caps(enum cap_bitmap feature);
void wma_set_fw_wlan_feat_caps(enum cap_bitmap feature);

QDF_STATUS wma_post_ctrl_msg(tpAniSirGlobal pMac, struct scheduler_msg *pMsg);

void wma_register_wow_wakeup_events(WMA_HANDLE handle, uint8_t vdev_id,
					uint8_t vdev_type, uint8_t sub_type);
void wma_register_wow_default_patterns(WMA_HANDLE handle, uint8_t vdev_id);
int8_t wma_get_mac_id_of_vdev(uint32_t vdev_id);
void wma_update_intf_hw_mode_params(uint32_t vdev_id, uint32_t mac_id,
				uint32_t cfgd_hw_mode_index);
void wma_set_dbs_capability_ut(uint32_t dbs);
QDF_STATUS wma_get_caps_for_phyidx_hwmode(struct wma_caps_per_phy *caps_per_phy,
		enum hw_mode_dbs_capab hw_mode, enum cds_band_type band);
bool wma_is_rx_ldpc_supported_for_channel(uint32_t channel);

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
int wma_unified_radio_tx_mem_free(void *handle);
#else /* WLAN_FEATURE_LINK_LAYER_STATS */
static inline int wma_unified_radio_tx_mem_free(void *handle)
{
	return 0;
}
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

/**
 * wma_form_unit_test_cmd_and_send() - to form a wma command and send it to FW
 * @session_id: wma session id to be filled while forming the command
 * @module_id: module id given by user to be filled in the command
 * @arg_count: number of argument count
 * @arg: pointer to argument list
 *
 * This API exposed to HDD layer which takes the argument from user and forms
 * the wma unit test command to be sent down to firmware
 *
 * Return: QDF_STATUS based on overall success
 */
QDF_STATUS wma_form_unit_test_cmd_and_send(uint32_t vdev_id,
		uint32_t module_id, uint32_t arg_count, uint32_t *arg);

/**
 * wma_lro_init() - sends LRO configuration to FW
 * @lro_config:         pointer to the config parameters
 *
 * This function ends LRO configuration to FW.
 *
 * Return: 0 for success or reasons for failure
 */
int wma_lro_init(struct cdp_lro_hash_config *lro_config);

QDF_STATUS wma_remove_beacon_filter(WMA_HANDLE wma,
				struct beacon_filter_param *filter_params);

QDF_STATUS wma_add_beacon_filter(WMA_HANDLE wma,
				struct beacon_filter_param *filter_params);
QDF_STATUS wma_send_adapt_dwelltime_params(WMA_HANDLE handle,
			struct adaptive_dwelltime_params *dwelltime_params);

/**
 * wma_send_dbs_scan_selection_params() - send DBS scan selection configuration
 * params to firmware
 * @handle: wma handler
 * @dbs_scan_params: pointer to wmi_dbs_scan_sel_params
 *
 * Return: QDF_STATUS_SUCCESS on success and QDF failure reason code for failure
 */
QDF_STATUS wma_send_dbs_scan_selection_params(WMA_HANDLE handle,
			struct wmi_dbs_scan_sel_params *dbs_scan_params);
QDF_STATUS wma_set_tx_power_scale(uint8_t vdev_id, int value);
QDF_STATUS wma_set_tx_power_scale_decr_db(uint8_t vdev_id, int value);

bool wma_is_csa_offload_enabled(void);
#ifdef FEATURE_P2P_LISTEN_OFFLOAD
bool wma_is_p2p_lo_capable(void);
QDF_STATUS wma_p2p_lo_start(struct sir_p2p_lo_start *params);
QDF_STATUS wma_p2p_lo_stop(u_int32_t vdev_id);
#else
static inline bool wma_is_p2p_lo_capable(void)
{
	return 0;
}
#endif
bool wma_capability_enhanced_mcast_filter(void);
#ifndef QCA_SUPPORT_CP_STATS
QDF_STATUS wma_get_wakelock_stats(struct sir_wake_lock_stats *wake_lock_stats);
#endif
void wma_process_pdev_hw_mode_trans_ind(void *wma,
	wmi_pdev_hw_mode_transition_event_fixed_param *fixed_param,
	wmi_pdev_set_hw_mode_response_vdev_mac_entry *vdev_mac_entry,
	struct sir_hw_mode_trans_ind *hw_mode_trans_ind);

/**
 * wma_set_cts2self_for_p2p_go() - set CTS2SELF command for P2P GO.
 * @wma_handle:                  pointer to wma handle.
 * @cts2self_for_p2p_go:         value needs to set to firmware.
 *
 * At the time of driver startup, inform about ini parma to FW that
 * if legacy client connects to P2P GO, stop using NOA for P2P GO.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS wma_set_cts2self_for_p2p_go(void *wma_handle,
		uint32_t cts2self_for_p2p_go);
QDF_STATUS wma_set_tx_rx_aggregation_size
	(struct sir_set_tx_rx_aggregation_size *tx_rx_aggregation_size);

/**
 * wma_set_tx_rx_aggregation_size_per_ac() - set aggregation size per ac
 * @tx_rx_aggregation_size: the parameter for aggregation size
 *
 *  This function try to set the aggregation size per AC.
 *
 *  Return: QDF_STATUS enumeration
 */
QDF_STATUS wma_set_tx_rx_aggregation_size_per_ac
	(struct sir_set_tx_rx_aggregation_size *tx_rx_aggregation_size);
/**
 * wma_set_sw_retry_threshold() - set sw retry threshold per AC for tx
 * @tx_rx_aggregation_size: value needs to set to firmware
 *
 * This function sends WMI command to set the sw retry threshold per AC
 * for Tx.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS wma_set_sw_retry_threshold
	(struct sir_set_tx_aggr_sw_retry_threshold *tx_rx_aggregation_size);
/**
 * wma_get_sar_limit() - get SAR limits from the target
 * @handle: wma handle
 * @callback: Callback function to invoke with the results
 * @context: Opaque context to pass back to caller in the callback
 *
 *  This function sends WMI command to get SAR limits.
 *
 *  Return: QDF_STATUS enumeration
 */
QDF_STATUS wma_get_sar_limit(WMA_HANDLE handle,
			     wma_sar_cb callback, void *context);

/**
 * wma_set_sar_limit() - set sar limits in the target
 * @handle: wma handle
 * @sar_limit_cmd_params: sar limit cmd params
 *
 *  This function sends WMI command to set SAR limits.
 *
 *  Return: QDF_STATUS enumeration
 */
QDF_STATUS wma_set_sar_limit(WMA_HANDLE handle,
		struct sar_limit_cmd_params *sar_limit_params);

/**
 * wma_set_qpower_config() - update qpower config in wma
 * @vdev_id:	the Id of the vdev to configure
 * @qpower:	new qpower value
 *
 * Return: QDF_STATUS_SUCCESS on success, error number otherwise
 */
QDF_STATUS wma_set_qpower_config(uint8_t vdev_id, uint8_t qpower);

#ifdef FEATURE_WLAN_D0WOW
static inline bool wma_d0_wow_is_supported(void)
{
	return true;
}
#else
static inline bool wma_d0_wow_is_supported(void)
{
	return false;
}
#endif

/**
 * wma_store_pdev() - store pdev
 * @wma_ctx:	wma context
 * @pdev:	pdev context
 *
 * Return: void
 */
void wma_store_pdev(void *wma_ctx, struct wlan_objmgr_pdev *pdev);

#ifdef WLAN_FEATURE_LINK_LAYER_STATS
/**
 * wmi_to_sir_peer_type() - convert peer type from WMI to SIR enum
 * @type: enum wmi_peer_type
 *
 * Return: tSirWifiPeerType
 */
tSirWifiPeerType wmi_to_sir_peer_type(enum wmi_peer_type type);
#endif /* WLAN_FEATURE_LINK_LAYER_STATS */

QDF_STATUS wma_crash_inject(WMA_HANDLE wma_handle, uint32_t type,
			    uint32_t delay_time_ms);

/**
 * wma_critical_events_in_flight() - get the number of critical events in flight
 *
 * This API gets the number of events in flight which should prevent power
 * collapse.
 *
 * Return: the number of critical events in flight
 */
uint32_t wma_critical_events_in_flight(void);

#ifdef FEATURE_WLAN_DYNAMIC_CVM
/**
 * wma_set_vc_mode_config() - set voltage corner mode config to FW.
 * @wma_handle:	pointer to wma handle.
 * @vc_bitmap:	value needs to set to firmware.
 *
 * At the time of driver startup, set operating voltage corner mode
 * for differenet phymode and bw configurations.
 *
 * Return: QDF_STATUS.
 */
QDF_STATUS wma_set_vc_mode_config(void *wma_handle,
		uint32_t vc_bitmap);
#endif

QDF_STATUS wma_process_dhcp_ind(WMA_HANDLE wma_handle,
				tAniDHCPInd *ta_dhcp_ind);

/**
 * wma_wmi_stop() - send wmi stop cmd
 *
 *  Return: None
 */
void wma_wmi_stop(void);

/**
 * wma_get_mcs_idx() - get mcs index
 * @max_rate: max rate
 * @rate_flags: rate flags
 * @nss: nss
 * @mcs_rate_flag: mcs rate flags
 *
 *  Return: mcs index
 */
uint8_t wma_get_mcs_idx(uint16_t max_rate, uint8_t rate_flags,
			uint8_t *nss, uint8_t *mcs_rate_flag);
#endif
