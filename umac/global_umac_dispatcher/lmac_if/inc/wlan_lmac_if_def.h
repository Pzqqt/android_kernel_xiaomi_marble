/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#ifndef _WLAN_LMAC_IF_DEF_H_
#define _WLAN_LMAC_IF_DEF_H_

#include "qdf_status.h"
#include "wlan_objmgr_cmn.h"
#ifdef DFS_COMPONENT_ENABLE
#include <wlan_dfs_public_struct.h>
#endif
#include "wlan_mgmt_txrx_utils_api.h"
#include "wlan_scan_public_structs.h"

#ifdef WLAN_ATF_ENABLE
#include "wlan_atf_utils_defs.h"
#endif
#ifdef QCA_SUPPORT_SON
#include <wlan_son_tgt_api.h>
#endif
#ifdef WLAN_SA_API_ENABLE
#include "wlan_sa_api_utils_defs.h"
#endif
#ifdef WLAN_CONV_SPECTRAL_ENABLE
#include "wlan_spectral_public_structs.h"
#endif
#include <reg_services_public_struct.h>

#ifdef WLAN_CONV_CRYPTO_SUPPORTED
#include "wlan_crypto_global_def.h"
#endif

#ifdef WLAN_CFR_ENABLE
#include "wlan_cfr_utils_api.h"
#endif

#include <wlan_dfs_tgt_api.h>
#include <wlan_dfs_ioctl.h>

/* Number of dev type: Direct attach and Offload */
#define MAX_DEV_TYPE 2

#ifdef WIFI_POS_CONVERGED
/* forward declarations */
struct oem_data_req;
struct oem_data_rsp;
#endif /* WIFI_POS_CONVERGED */

#ifdef DIRECT_BUF_RX_ENABLE
/* forward declarations for direct buf rx */
struct direct_buf_rx_data;
/* Forward declaration for module_ring_params */
struct module_ring_params;
/*Forward declaration for dbr_module_config */
struct dbr_module_config;
#endif

#ifdef FEATURE_WLAN_TDLS
#include "wlan_tdls_public_structs.h"
#endif

#ifdef QCA_SUPPORT_CP_STATS
#include <wlan_cp_stats_mc_defs.h>
#endif /* QCA_SUPPORT_CP_STATS */

#include <wlan_vdev_mgr_tgt_if_tx_defs.h>
#include <wlan_vdev_mgr_tgt_if_rx_defs.h>

#ifdef QCA_SUPPORT_CP_STATS
/**
 * struct wlan_lmac_if_cp_stats_tx_ops - defines southbound tx callbacks for
 * control plane statistics component
 * @cp_stats_attach:	function pointer to register events from FW
 * @cp_stats_detach:	function pointer to unregister events from FW
 */
struct wlan_lmac_if_cp_stats_tx_ops {
	QDF_STATUS (*cp_stats_attach)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*cp_stats_detach)(struct wlan_objmgr_psoc *posc);
#ifdef CONFIG_MCL
	void (*inc_wake_lock_stats)(uint32_t reason,
				    struct wake_lock_stats *stats,
				    uint32_t *unspecified_wake_count);
	QDF_STATUS (*send_req_stats)(struct wlan_objmgr_psoc *psoc,
				     enum stats_req_type type,
				     struct request_info *req);
#endif
};

/**
 * struct wlan_lmac_if_cp_stats_rx_ops - defines southbound rx callbacks for
 * control plane statistics component
 * @cp_stats_rx_event_handler:	function pointer to rx FW events
 */
struct wlan_lmac_if_cp_stats_rx_ops {
	QDF_STATUS (*cp_stats_rx_event_handler)(struct wlan_objmgr_vdev *vdev);
#ifdef CONFIG_MCL
	QDF_STATUS (*process_stats_event)(struct wlan_objmgr_psoc *psoc,
					  struct stats_event *ev);
#endif
};
#endif

/**
 * struct wlan_lmac_if_mgmt_txrx_tx_ops - structure of tx function
 *                  pointers for mgmt txrx component
 * @mgmt_tx_send: function pointer to transmit mgmt tx frame
 * @beacon_send:  function pointer to transmit beacon frame
 * @fd_action_frame_send: function pointer to transmit FD action frame
 * @tx_drain_nbuf_op: function pointer for any umac nbuf realted ops for
 *                    pending mgmt frames cleanup
 */
struct wlan_lmac_if_mgmt_txrx_tx_ops {
	QDF_STATUS (*mgmt_tx_send)(struct wlan_objmgr_vdev *vdev,
			qdf_nbuf_t nbuf, u_int32_t desc_id,
			void *mgmt_tx_params);
	QDF_STATUS (*beacon_send)(struct wlan_objmgr_vdev *vdev,
			qdf_nbuf_t nbuf);
	QDF_STATUS (*fd_action_frame_send)(struct wlan_objmgr_vdev *vdev,
					   qdf_nbuf_t nbuf);
	void (*tx_drain_nbuf_op)(struct wlan_objmgr_pdev *pdev,
				 qdf_nbuf_t nbuf);
};

/**
 * struct wlan_lmac_if_scan_tx_ops - south bound tx function pointers for scan
 * @scan_start: function to start scan
 * @scan_cancel: function to cancel scan
 * @pno_start: start pno scan
 * @pno_stop: stop pno scan
 * @scan_reg_ev_handler: function to register for scan events
 * @scan_unreg_ev_handler: function to unregister for scan events
 *
 * scan module uses these functions to avail ol/da lmac services
 */
struct wlan_lmac_if_scan_tx_ops {
	QDF_STATUS (*scan_start)(struct wlan_objmgr_pdev *pdev,
			struct scan_start_request *req);
	QDF_STATUS (*scan_cancel)(struct wlan_objmgr_pdev *pdev,
			struct scan_cancel_param *req);
	QDF_STATUS (*pno_start)(struct wlan_objmgr_psoc *psoc,
			struct pno_scan_req_params *req);
	QDF_STATUS (*pno_stop)(struct wlan_objmgr_psoc *psoc,
			uint8_t vdev_id);
	QDF_STATUS (*scan_reg_ev_handler)(struct wlan_objmgr_psoc *psoc,
			void *arg);
	QDF_STATUS (*scan_unreg_ev_handler)(struct wlan_objmgr_psoc *psoc,
			void *arg);
	QDF_STATUS (*set_chan_list)(struct wlan_objmgr_pdev *pdev, void *arg);
};

/**
 * struct wlan_lmac_if_ftm_tx_ops - south bound tx function pointers for ftm
 * @ftm_attach: function to register event handlers with FW
 * @ftm_detach: function to de-register event handlers with FW
 * @ftm_cmd_send: function to send FTM commands to FW
 *
 * ftm module uses these functions to avail ol/da lmac services
 */
struct wlan_lmac_if_ftm_tx_ops {
	QDF_STATUS (*ftm_attach)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*ftm_detach)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*ftm_cmd_send)(struct wlan_objmgr_pdev *pdev,
				uint8_t *buf, uint32_t len, uint8_t mac_id);
};

enum wlan_mlme_cfg_id;
/**
 * struct wlan_lmac_if_mlme_tx_ops - south bound tx function pointers for mlme
 * @get_wifi_iface_id: function to get wifi interface id
 * @vdev_mlme_attach: function to register events
 * @vdev_mlme_detach: function to unregister events
 * @vdev_create_send: function to send vdev create
 * @vdev_start_send: function to send vdev start
 * @vdev_up_send: function to send vdev up
 * @vdev_delete_send: function to send vdev delete
 * @vdev_stop_send: function to send vdev stop
 * @vdev_down_send: function to send vdev down
 * @vdev_set_param_send: function to send vdev parameter
 * @vdev_set_tx_rx_decap_type: function to send vdev tx rx cap/decap type
 * @vdev_set_nac_rssi_send: function to send nac rssi
 * @vdev_set_neighbour_rx_cmd_send: function to send vdev neighbour rx cmd
 * @vdev_sifs_trigger_send: function to send vdev sifs trigger
 * @vdev_set_custom_aggr_size_cmd_send: function to send custom aggr size
 * @vdev_config_ratemask_cmd_send: function to send ratemask
 * @peer_flush_tids_send: function to flush peer tids
 * @multiple_vdev_restart_req_cmd: function to send multiple vdev restart
 * @beacon_send_cmd: function to send beacon
 * @beacon_tmpl_send: function to send beacon template
 * @vdev_bcn_miss_offload_send: function to send beacon miss offload
 * @vdev_sta_ps_param_send: function to sent STA power save config
 * @peer_delete_all_send: function to send vdev delete all peer request
 * @psoc_vdev_rsp_timer_init: function to initialize psoc vdev response timer
 * @psoc_vdev_rsp_timer_deinit: function to deinitialize psoc vdev rsp timer
 * @psoc_vdev_rsp_timer_inuse: function to determine whether the vdev rsp
 * timer is inuse or not
 * @psoc_vdev_rsp_timer_mod: function to modify the time of vdev rsp timer
 * @psoc_wake_lock_init: Initialize psoc wake lock for vdev response timer
 * @psoc_wake_lock_deinit: De-Initialize psoc wake lock for vdev response timer
 */
struct wlan_lmac_if_mlme_tx_ops {
	uint32_t (*get_wifi_iface_id) (struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*vdev_mlme_attach)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*vdev_mlme_detach)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*vdev_create_send)(struct wlan_objmgr_vdev *vdev,
				       struct vdev_create_params *param);
	QDF_STATUS (*vdev_start_send)(struct wlan_objmgr_vdev *vdev,
				      struct vdev_start_params *param);
	QDF_STATUS (*vdev_up_send)(struct wlan_objmgr_vdev *vdev,
				   struct vdev_up_params *param);
	QDF_STATUS (*vdev_delete_send)(struct wlan_objmgr_vdev *vdev,
				       struct vdev_delete_params *param);
	QDF_STATUS (*vdev_stop_send)(struct wlan_objmgr_vdev *vdev,
				     struct vdev_stop_params *param);
	QDF_STATUS (*vdev_down_send)(struct wlan_objmgr_vdev *vdev,
				     struct vdev_down_params *param);
	QDF_STATUS (*vdev_set_param_send)(struct wlan_objmgr_vdev *vdev,
					  struct vdev_set_params *param);
	QDF_STATUS (*vdev_set_tx_rx_decap_type)(struct wlan_objmgr_vdev *vdev,
						enum wlan_mlme_cfg_id param_id,
						uint32_t value);
	QDF_STATUS (*vdev_set_nac_rssi_send)(
				struct wlan_objmgr_vdev *vdev,
				struct vdev_scan_nac_rssi_params *param);
	QDF_STATUS (*vdev_set_neighbour_rx_cmd_send)(
					struct wlan_objmgr_vdev *vdev,
					struct set_neighbour_rx_params *param,
					uint8_t *mac);
	QDF_STATUS (*vdev_sifs_trigger_send)(
					struct wlan_objmgr_vdev *vdev,
					struct sifs_trigger_param *param);
	QDF_STATUS (*vdev_set_custom_aggr_size_cmd_send)(
				struct wlan_objmgr_vdev *vdev,
				struct set_custom_aggr_size_params *param);
	QDF_STATUS (*vdev_config_ratemask_cmd_send)(
					struct wlan_objmgr_vdev *vdev,
					struct config_ratemask_params *param);
	QDF_STATUS (*peer_flush_tids_send)(
					struct wlan_objmgr_vdev *vdev,
					struct peer_flush_params *param);
	QDF_STATUS (*multiple_vdev_restart_req_cmd)(
				struct wlan_objmgr_pdev *pdev,
				struct multiple_vdev_restart_params *param);
	QDF_STATUS (*beacon_cmd_send)(struct wlan_objmgr_vdev *vdev,
				      struct beacon_params *param);
	QDF_STATUS (*beacon_tmpl_send)(struct wlan_objmgr_vdev *vdev,
				       struct beacon_tmpl_params *param);
#if defined(WLAN_SUPPORT_FILS) || defined(CONFIG_BAND_6GHZ)
	QDF_STATUS (*vdev_fils_enable_send)(struct wlan_objmgr_vdev *vdev,
					    struct config_fils_params *param);
#endif
	QDF_STATUS (*vdev_bcn_miss_offload_send)(struct wlan_objmgr_vdev *vdev);
	QDF_STATUS (*vdev_sta_ps_param_send)(struct wlan_objmgr_vdev *vdev,
					     struct sta_ps_params *param);
	QDF_STATUS (*peer_delete_all_send)(
					struct wlan_objmgr_vdev *vdev,
					struct peer_delete_all_params *param);
	QDF_STATUS (*psoc_vdev_rsp_timer_init)(
				struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id);
	void (*psoc_vdev_rsp_timer_deinit)(
				struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id);
	QDF_STATUS (*psoc_vdev_rsp_timer_inuse)(
				struct wlan_objmgr_psoc *psoc,
				uint8_t vdev_id);
	QDF_STATUS (*psoc_vdev_rsp_timer_mod)(
					struct wlan_objmgr_psoc *psoc,
					uint8_t vdev_id,
					int mseconds);
	void (*psoc_wake_lock_init)(
				struct wlan_objmgr_psoc *psoc);
	void (*psoc_wake_lock_deinit)(
				struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*vdev_mgr_rsp_timer_stop)(
				struct wlan_objmgr_psoc *psoc,
				struct vdev_response_timer *vdev_rsp,
				enum wlan_vdev_mgr_tgt_if_rsp_bit clear_bit);
};

/**
 * struct wlan_lmac_if_scan_rx_ops  - south bound rx function pointers for scan
 * @scan_ev_handler: scan event handler
 * @scan_set_max_active_scans: set max active scans allowed
 *
 * lmac modules uses this API to post scan events to scan module
 */
struct wlan_lmac_if_scan_rx_ops {
	QDF_STATUS (*scan_ev_handler)(struct wlan_objmgr_psoc *psoc,
		struct scan_event_info *event_info);
	QDF_STATUS (*scan_set_max_active_scans)(struct wlan_objmgr_psoc *psoc,
			uint32_t max_active_scans);
};

#ifdef CONVERGED_P2P_ENABLE

/* forward declarations for p2p tx ops */
struct p2p_ps_config;
struct p2p_lo_start;
struct p2p_set_mac_filter;

/**
 * struct wlan_lmac_if_p2p_tx_ops - structure of tx function pointers
 * for P2P component
 * @set_ps:      function pointer to set power save
 * @lo_start:    function pointer to start listen offload
 * @lo_stop:     function pointer to stop listen offload
 * @set_noa:     function pointer to disable/enable NOA
 * @reg_lo_ev_handler:   function pointer to register lo event handler
 * @reg_noa_ev_handler:  function pointer to register noa event handler
 * @unreg_lo_ev_handler: function pointer to unregister lo event handler
 * @unreg_noa_ev_handler:function pointer to unregister noa event handler
 * @reg_mac_addr_rx_filter_handler: function pointer to register/unregister
 *    set mac addr status event callback.
 * @set_mac_addr_rx_filter_cmd: function pointer to set mac addr rx filter
 */
struct wlan_lmac_if_p2p_tx_ops {
	QDF_STATUS (*set_ps)(struct wlan_objmgr_psoc *psoc,
		struct p2p_ps_config *ps_config);
#ifdef FEATURE_P2P_LISTEN_OFFLOAD
	QDF_STATUS (*lo_start)(struct wlan_objmgr_psoc *psoc,
		struct p2p_lo_start *lo_start);
	QDF_STATUS (*lo_stop)(struct wlan_objmgr_psoc *psoc,
		uint32_t vdev_id);
	QDF_STATUS (*reg_lo_ev_handler)(struct wlan_objmgr_psoc *psoc,
			void *arg);
	QDF_STATUS (*unreg_lo_ev_handler)(struct wlan_objmgr_psoc *psoc,
			void *arg);
#endif
	QDF_STATUS (*set_noa)(struct wlan_objmgr_psoc *psoc,
			      uint32_t vdev_id, bool disable_noa);
	QDF_STATUS (*reg_noa_ev_handler)(struct wlan_objmgr_psoc *psoc,
					 void *arg);
	QDF_STATUS (*unreg_noa_ev_handler)(struct wlan_objmgr_psoc *psoc,
			void *arg);
	QDF_STATUS (*reg_mac_addr_rx_filter_handler)(
			struct wlan_objmgr_psoc *psoc, bool reg);
	QDF_STATUS (*set_mac_addr_rx_filter_cmd)(
			struct wlan_objmgr_psoc *psoc,
			struct p2p_set_mac_filter *param);
};
#endif

#ifdef WLAN_ATF_ENABLE

/**
 * struct wlan_lmac_if_atf_tx_ops - ATF specific tx function pointers
 * @atf_node_unblock:             Resume node
 * @atf_set_enable_disable:       Set atf enable/disable
 * @atf_tokens_used:              Get used atf tokens
 * @atf_get_unused_txtoken:       Get unused atf tokens
 * @atf_peer_resume:              Resume peer
 * @atf_tokens_unassigned:        Set unassigned atf tockens
 * @atf_capable_peer:             Set atf state change
 * @atf_airtime_estimate:         Get estimated airtime
 * @atf_debug_peerstate:          Get peer state
 * @atf_enable_disable:           Set atf peer stats enable/disable
 * @atf_ssid_sched_policy:        Set ssid schedule policy
 * @atf_set:                      Set atf
 * @atf_set_grouping:             Set atf grouping
 * @atf_set_group_ac:             Set atf Group AC
 * @atf_send_peer_request:        Send peer requests
 * @atf_set_bwf:                  Set bandwidth fairness
 * @atf_peer_buf_held:            Get buffer held
 * @atf_get_peer_airtime:         Get peer airtime
 * @atf_get_chbusyper:            Get channel busy
 * @atf_open:                     ATF open
 * @atf_register_event_handler    ATF register wmi event handlers
 * @atf_unregister_event_handler  ATF unregister wmi event handlers
 */
struct wlan_lmac_if_atf_tx_ops {
	void (*atf_node_unblock)(struct wlan_objmgr_pdev *pdev,
				 struct wlan_objmgr_peer *peer);
	void (*atf_set_enable_disable)(struct wlan_objmgr_pdev *pdev,
				       uint8_t value);
	uint8_t (*atf_tokens_used)(struct wlan_objmgr_pdev *pdev,
				   struct wlan_objmgr_peer *peer);
	void (*atf_get_unused_txtoken)(struct wlan_objmgr_pdev *pdev,
				       struct wlan_objmgr_peer *peer,
				       int *unused_token);
	void (*atf_peer_resume)(struct wlan_objmgr_pdev *pdev,
				struct wlan_objmgr_peer *peer);
	void (*atf_tokens_unassigned)(struct wlan_objmgr_pdev *pdev,
				      uint32_t tokens_unassigned);
	void (*atf_capable_peer)(struct wlan_objmgr_pdev *pdev,
				 struct wlan_objmgr_peer *peer,
				 uint8_t val, uint8_t atfstate_change);
	uint32_t (*atf_airtime_estimate)(struct wlan_objmgr_pdev *pdev,
					 struct wlan_objmgr_peer *peer,
					 uint32_t tput,
					 uint32_t *possible_tput);
	uint32_t (*atf_debug_peerstate)(struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_peer *peer,
					struct atf_peerstate *peerstate);
	int32_t (*atf_enable_disable)(struct wlan_objmgr_vdev *vdev,
				      uint8_t value);
	int32_t (*atf_ssid_sched_policy)(struct wlan_objmgr_vdev *vdev,
					 uint8_t value);
	int32_t (*atf_set)(struct wlan_objmgr_pdev *pdev,
			   struct pdev_atf_req *atf_req,
			   uint8_t atf_tput_based);
	int32_t (*atf_set_grouping)(struct wlan_objmgr_pdev *pdev,
				    struct pdev_atf_ssid_group_req *atf_grp_req,
				    uint8_t atf_tput_based);
	int32_t (*atf_set_group_ac)(struct wlan_objmgr_pdev *pdev,
				    struct pdev_atf_group_wmm_ac_req *atf_acreq,
				    uint8_t atf_tput_based);
	int32_t (*atf_send_peer_request)(struct wlan_objmgr_pdev *pdev,
					 struct pdev_atf_peer_ext_request *atfr,
					 uint8_t atf_tput_based);
	int32_t (*atf_set_bwf)(struct wlan_objmgr_pdev *pdev,
			       struct pdev_bwf_req *bwf_req);
	uint32_t (*atf_peer_buf_held)(struct wlan_objmgr_peer *peer);
	uint32_t (*atf_get_peer_airtime)(struct wlan_objmgr_peer *peer);
	uint32_t (*atf_get_chbusyper)(struct wlan_objmgr_pdev *pdev);
	void (*atf_open)(struct wlan_objmgr_psoc *psoc);
	void (*atf_register_event_handler)(struct wlan_objmgr_psoc *psoc);
	void (*atf_unregister_event_handler)(struct wlan_objmgr_psoc *psoc);
};
#endif

#ifdef WLAN_SUPPORT_FILS
/**
 * struct wlan_lmac_if_fd_tx_ops - FILS Discovery specific Tx function pointers
 * @fd_vdev_config_fils:         Enable and configure FILS Discovery
 * @fd_register_event_handler:   Register swfda WMI event handler
 * @fd_unregister_event_handler: Un-register swfda WMI event handler
 * @fd_offload_tmpl_send:        Send FD template to FW
 */
struct wlan_lmac_if_fd_tx_ops {
	QDF_STATUS (*fd_vdev_config_fils)(struct wlan_objmgr_vdev *vdev,
					  uint32_t fd_period);
	void (*fd_register_event_handler)(struct wlan_objmgr_psoc *psoc);
	void (*fd_unregister_event_handler)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*fd_offload_tmpl_send)(struct wlan_objmgr_pdev *pdev,
			struct fils_discovery_tmpl_params *fd_tmpl_param);
};
#endif

#ifdef WLAN_SA_API_ENABLE

/**
 * struct wlan_lmac_if_sa_api_tx_ops - SA API specific tx function pointers
 */

struct wlan_lmac_if_sa_api_tx_ops {
	void (*sa_api_register_event_handler)(struct wlan_objmgr_psoc *psoc);
	void (*sa_api_unregister_event_handler)(struct wlan_objmgr_psoc *posc);
	void (*sa_api_enable_sa) (struct wlan_objmgr_pdev *pdev,
			uint32_t enable, uint32_t mode, uint32_t rx_antenna);
	void (*sa_api_set_rx_antenna) (struct wlan_objmgr_pdev *pdev,
			uint32_t antenna);
	void (*sa_api_set_tx_antenna) (struct wlan_objmgr_peer *peer,
			uint32_t *antenna_array);
	void (*sa_api_set_tx_default_antenna) (struct wlan_objmgr_pdev *pdev,
			u_int32_t antenna);
	void (*sa_api_set_training_info) (struct wlan_objmgr_peer *peer,
			uint32_t *rate_array,
			uint32_t *antenna_array,
			uint32_t numpkts);
	void (*sa_api_prepare_rateset)(struct wlan_objmgr_pdev *pdev,
			struct wlan_objmgr_peer *peer,
			struct sa_rate_info *rate_info);
	void (*sa_api_set_node_config_ops) (struct wlan_objmgr_peer *peer,
			uint32_t cmd_id, uint16_t args_count,
			u_int32_t args_arr[]);
};

#endif

#ifdef WLAN_CFR_ENABLE
/**
 * struct wlan_lmac_if_cfr_tx_ops - CFR specific tx function pointers
 * @cfr_init_pdev: Initialize CFR
 * @cfr_deinit_pdev: De-initialize CFR
 * @cfr_enable_cfr_timer: Function to enable CFR timer
 * @cfr_start_capture: Function to start CFR capture
 * @cfr_stop_capture: Function to stop CFR capture
 * @cfr_config_rcc: Function to set the Repetitive channel capture params
 * @cfr_start_lut_timer: Function to start timer to flush aged-out LUT entries
 * @cfr_stop_lut_timer: Function to stop timer to flush aged-out LUT entries
 * @cfr_default_ta_ra_cfg: Function to configure default values for TA_RA mode
 * @cfr_dump_lut_enh: Function to dump LUT entries
 * @cfr_rx_tlv_process: Function to process PPDU status TLVs
 */
struct wlan_lmac_if_cfr_tx_ops {
	int (*cfr_init_pdev)(struct wlan_objmgr_psoc *psoc,
			     struct wlan_objmgr_pdev *pdev);
	int (*cfr_deinit_pdev)(struct wlan_objmgr_psoc *psoc,
			       struct wlan_objmgr_pdev *pdev);
	int (*cfr_enable_cfr_timer)(struct wlan_objmgr_pdev *pdev,
				    uint32_t cfr_timer);
	int (*cfr_start_capture)(struct wlan_objmgr_pdev *pdev,
				 struct wlan_objmgr_peer *peer,
				 struct cfr_capture_params *params);
	int (*cfr_stop_capture)(struct wlan_objmgr_pdev *pdev,
				struct wlan_objmgr_peer *peer);
#ifdef WLAN_ENH_CFR_ENABLE
	QDF_STATUS (*cfr_config_rcc)(struct wlan_objmgr_pdev *pdev,
				     struct cfr_rcc_param *params);
	QDF_STATUS (*cfr_start_lut_timer)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*cfr_stop_lut_timer)(struct wlan_objmgr_pdev *pdev);
	void (*cfr_default_ta_ra_cfg)(struct cfr_rcc_param *params,
				      bool allvalid, uint16_t reset_cfg);
	void (*cfr_dump_lut_enh)(struct wlan_objmgr_pdev *pdev);
	void (*cfr_rx_tlv_process)(struct wlan_objmgr_pdev *pdev, void *nbuf);
#endif
};
#endif /* WLAN_CFR_ENABLE */

#ifdef WLAN_CONV_SPECTRAL_ENABLE
struct wmi_spectral_cmd_ops;
/**
 * struct wlan_lmac_if_sptrl_tx_ops - Spectral south bound Tx operations
 * @sptrlto_spectral_init:          Initialize LMAC/target_if Spectral
 * @sptrlto_spectral_deinit:        De-initialize LMAC/target_if Spectral
 * @sptrlto_set_spectral_config:    Set Spectral configuration
 * @sptrlto_get_spectral_config:    Get Spectral configuration
 * @sptrlto_start_spectral_scan:    Start Spectral Scan
 * @sptrlto_stop_spectral_scan:     Stop Spectral Scan
 * @sptrlto_is_spectral_active:     Get whether Spectral is active
 * @sptrlto_is_spectral_enabled:    Get whether Spectral is enabled
 * @sptrlto_set_icm_active:         Set whether ICM is active or inactive
 * @sptrlto_get_icm_active:         Get whether ICM is active or inactive
 * @sptrlto_get_nominal_nf:         Get Nominal Noise Floor for the current
 *                                  frequency band
 * @sptrlto_set_debug_level:        Set Spectral debug level
 * @sptrlto_get_debug_level:        Get Spectral debug level
 * @sptrlto_get_chaninfo:           Get channel information
 * @sptrlto_clear_chaninfo:         Clear channel information
 * @sptrlto_get_spectral_capinfo:   Get Spectral capability information
 * @sptrlto_get_spectral_diagstats: Get Spectral diagnostic statistics
 * @sptrlto_register_netlink_cb: Register Spectral Netlink callbacks
 * @sptrlto_use_nl_bcast: Get whether to use Netlink broadcast/unicast
 * @sptrlto_deregister_netlink_cb: De-register Spectral Netlink callbacks
 * @sptrlto_process_spectral_report: Process spectral report
 * @sptrlto_set_dma_debug: Set DMA debug for Spectral
 * @sptrlto_direct_dma_support: Whether Direct-DMA is supported on this radio
 * @sptrlto_check_and_do_dbr_ring_debug: Start/Stop Spectral ring debug based
 *                                       on the previous state
 * @sptrlto_check_and_do_dbr_buff_debug: Start/Stop Spectral buffer debug based
 *                                       on the previous state
 **/
struct wlan_lmac_if_sptrl_tx_ops {
	void *(*sptrlto_pdev_spectral_init)(struct wlan_objmgr_pdev *pdev);
	void (*sptrlto_pdev_spectral_deinit)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*sptrlto_set_spectral_config)
					(struct wlan_objmgr_pdev *pdev,
					 const u_int32_t threshtype,
					 const u_int32_t value,
					 const enum spectral_scan_mode smode,
					 enum spectral_cp_error_code *err);
	QDF_STATUS (*sptrlto_get_spectral_config)
					(struct wlan_objmgr_pdev *pdev,
					 struct spectral_config *sptrl_config,
					 enum spectral_scan_mode smode);
	QDF_STATUS (*sptrlto_start_spectral_scan)
					(struct wlan_objmgr_pdev *pdev,
					 const enum spectral_scan_mode smode,
					 enum spectral_cp_error_code *err);
	QDF_STATUS (*sptrlto_stop_spectral_scan)
					(struct wlan_objmgr_pdev *pdev,
					 const enum spectral_scan_mode smode,
					 enum spectral_cp_error_code *err);
	bool (*sptrlto_is_spectral_active)(struct wlan_objmgr_pdev *pdev,
					   const enum spectral_scan_mode smode);
	bool (*sptrlto_is_spectral_enabled)(struct wlan_objmgr_pdev *pdev,
					    enum spectral_scan_mode smode);
	QDF_STATUS (*sptrlto_set_debug_level)(struct wlan_objmgr_pdev *pdev,
					      u_int32_t debug_level);
	u_int32_t (*sptrlto_get_debug_level)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*sptrlto_get_spectral_capinfo)
						(struct wlan_objmgr_pdev *pdev,
						 struct spectral_caps *scaps);
	QDF_STATUS (*sptrlto_get_spectral_diagstats)
					(struct wlan_objmgr_pdev *pdev,
					 struct spectral_diag_stats *stats);
	void (*sptrlto_register_wmi_spectral_cmd_ops)(
		struct wlan_objmgr_pdev *pdev,
		struct wmi_spectral_cmd_ops *cmd_ops);
	void (*sptrlto_register_netlink_cb)(
		struct wlan_objmgr_pdev *pdev,
		struct spectral_nl_cb *nl_cb);
	bool (*sptrlto_use_nl_bcast)(struct wlan_objmgr_pdev *pdev);
	void (*sptrlto_deregister_netlink_cb)(struct wlan_objmgr_pdev *pdev);
	int (*sptrlto_process_spectral_report)(
		struct wlan_objmgr_pdev *pdev,
		void *payload);
	QDF_STATUS (*sptrlto_set_dma_debug)(
		struct wlan_objmgr_pdev *pdev,
		enum spectral_dma_debug dma_debug_type,
		bool dma_debug_enable);
	bool (*sptrlto_direct_dma_support)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*sptrlto_check_and_do_dbr_ring_debug)(
		struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*sptrlto_check_and_do_dbr_buff_debug)(
		struct wlan_objmgr_pdev *pdev);

};
#endif /* WLAN_CONV_SPECTRAL_ENABLE */

#ifdef WIFI_POS_CONVERGED
/*
 * struct wlan_lmac_if_wifi_pos_tx_ops - structure of firmware tx function
 * pointers for wifi_pos component
 * @data_req_tx: function pointer to send wifi_pos req to firmware
 * @wifi_pos_register_events: function pointer to register wifi_pos events
 * @wifi_pos_deregister_events: function pointer to deregister wifi_pos events
 */
struct wlan_lmac_if_wifi_pos_tx_ops {
	QDF_STATUS (*data_req_tx)(struct wlan_objmgr_pdev *pdev,
				  struct oem_data_req *req);
	QDF_STATUS (*wifi_pos_register_events)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*wifi_pos_deregister_events)(struct wlan_objmgr_psoc *psoc);
};
#endif

#ifdef DIRECT_BUF_RX_ENABLE
/**
 * struct wlan_lmac_if_direct_buf_rx_tx_ops - structire of direct buf rx txops
 * @direct_buf_rx_module_register: Registration API callback for modules
 *                                 to register with direct buf rx framework
 * @direct_buf_rx_module_unregister: Unregistration API to clean up module
 *                                   specific resources in DBR
 * @direct_buf_rx_register_events: Registration of WMI events for direct
 *                                 buffer rx framework
 * @direct_buf_rx_unregister_events: Unregistraton of WMI events for direct
 *                                   buffer rx framework
 * @direct_buf_rx_print_ring_stat: Print ring status per module per pdev
 *
 * @direct_buf_rx_get_ring_params: Get ring parameters for module_id
 * @direct_buf_rx_start_ring_debug: Start DBR ring debug
 * @direct_buf_rx_stop_ring_debug: Stop DBR ring debug
 * @direct_buf_rx_start_buffer_poisoning: Start DBR buffer poisoning
 * @direct_buf_rx_stop_buffer_poisoning: Stop DBR buffer poisoning
 */
struct wlan_lmac_if_direct_buf_rx_tx_ops {
	QDF_STATUS (*direct_buf_rx_module_register)(
			struct wlan_objmgr_pdev *pdev, uint8_t mod_id,
			struct dbr_module_config *dbr_config,
			bool (*dbr_rsp_handler)
			     (struct wlan_objmgr_pdev *pdev,
			      struct direct_buf_rx_data *dbr_data));
	QDF_STATUS (*direct_buf_rx_module_unregister)(
			struct wlan_objmgr_pdev *pdev, uint8_t mod_id);
	QDF_STATUS (*direct_buf_rx_register_events)(
			struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*direct_buf_rx_unregister_events)(
			struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*direct_buf_rx_print_ring_stat)(
			struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*direct_buf_rx_get_ring_params)
		(struct wlan_objmgr_pdev *pdev,
		 struct module_ring_params *param,
		 uint8_t module_id, uint8_t srng_id);
	QDF_STATUS (*direct_buf_rx_start_ring_debug)(
		struct wlan_objmgr_pdev *pdev, uint8_t mod_id,
		uint32_t num_ring_debug_entries);
	QDF_STATUS (*direct_buf_rx_stop_ring_debug)(
		struct wlan_objmgr_pdev *pdev, uint8_t mod_id);
	QDF_STATUS (*direct_buf_rx_start_buffer_poisoning)(
		struct wlan_objmgr_pdev *pdev, uint8_t mod_id, uint32_t value);
	QDF_STATUS (*direct_buf_rx_stop_buffer_poisoning)(
		struct wlan_objmgr_pdev *pdev, uint8_t mod_id);
};
#endif /* DIRECT_BUF_RX_ENABLE */

#ifdef FEATURE_WLAN_TDLS
/* fwd declarations for tdls tx ops */
struct tdls_info;
struct tdls_peer_update_state;
struct tdls_channel_switch_params;
struct sta_uapsd_trig_params;
/**
 * struct wlan_lmac_if_tdls_tx_ops - south bound tx function pointers for tdls
 * @update_fw_state: function to update tdls firmware state
 * @update_peer_state: function to update tdls peer state
 * @set_offchan_mode: function to set tdls offchannel mode
 * @tdls_reg_ev_handler: function to register for tdls events
 * @tdls_unreg_ev_handler: function to unregister for tdls events
 *
 * tdls module uses these functions to avail ol/da lmac services
 */
struct wlan_lmac_if_tdls_tx_ops {
	QDF_STATUS (*update_fw_state)(struct wlan_objmgr_psoc *psoc,
				     struct tdls_info *req);
	QDF_STATUS (*update_peer_state)(struct wlan_objmgr_psoc *psoc,
				       struct tdls_peer_update_state *param);
	QDF_STATUS (*set_offchan_mode)(struct wlan_objmgr_psoc *psoc,
				      struct tdls_channel_switch_params *param);
	QDF_STATUS (*tdls_reg_ev_handler)(struct wlan_objmgr_psoc *psoc,
					 void *arg);
	QDF_STATUS (*tdls_unreg_ev_handler) (struct wlan_objmgr_psoc *psoc,
					    void *arg);
};

/* fwd declarations for tdls rx ops */
struct tdls_event_info;
/**
 * struct wlan_lmac_if_tdls_rx_ops  - south bound rx function pointers for tdls
 * @tdls_ev_handler: function to handler tdls event
 *
 * lmac modules uses this API to post scan events to tdls module
 */
struct wlan_lmac_if_tdls_rx_ops {
	QDF_STATUS (*tdls_ev_handler)(struct wlan_objmgr_psoc *psoc,
				     struct tdls_event_info *info);
};
#endif

/**
 * struct wlan_lmac_if_ftm_rx_ops  - south bound rx function pointers for FTM
 * @ftm_ev_handler: function to handle FTM event
 *
 * lmac modules uses this API to post FTM events to FTM module
 */
struct wlan_lmac_if_ftm_rx_ops {
	QDF_STATUS (*ftm_ev_handler)(struct wlan_objmgr_pdev *pdev,
					uint8_t *event_buf, uint32_t len);
};

/**
 * struct wlan_lmac_reg_if_tx_ops - structure of tx function
 *                  pointers for regulatory component
 * @register_master_handler: pointer to register event handler
 * @unregister_master_handler:  pointer to unregister event handler
 * @register_11d_new_cc_handler: pointer to register 11d cc event handler
 * @unregister_11d_new_cc_handler:  pointer to unregister 11d cc event handler
 * @send_ctl_info: call-back function to send CTL info to firmware
 */
struct wlan_lmac_if_reg_tx_ops {
	QDF_STATUS (*register_master_handler)(struct wlan_objmgr_psoc *psoc,
					      void *arg);
	QDF_STATUS (*unregister_master_handler)(struct wlan_objmgr_psoc *psoc,
						void *arg);

	QDF_STATUS (*set_country_code)(struct wlan_objmgr_psoc *psoc,
						void *arg);
	QDF_STATUS (*fill_umac_legacy_chanlist)(struct wlan_objmgr_pdev *pdev,
			struct regulatory_channel *cur_chan_list);
	QDF_STATUS (*register_11d_new_cc_handler)(
			struct wlan_objmgr_psoc *psoc, void *arg);
	QDF_STATUS (*unregister_11d_new_cc_handler)(
			struct wlan_objmgr_psoc *psoc, void *arg);
	QDF_STATUS (*start_11d_scan)(struct wlan_objmgr_psoc *psoc,
			struct reg_start_11d_scan_req *reg_start_11d_scan_req);
	QDF_STATUS (*stop_11d_scan)(struct wlan_objmgr_psoc *psoc,
			struct reg_stop_11d_scan_req *reg_stop_11d_scan_req);
	bool (*is_there_serv_ready_extn)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*set_user_country_code)(struct wlan_objmgr_psoc *psoc,
					    uint8_t pdev_id,
					    struct cc_regdmn_s *rd);
	QDF_STATUS (*set_country_failed)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*register_ch_avoid_event_handler)(
			struct wlan_objmgr_psoc *psoc, void *arg);
	QDF_STATUS (*unregister_ch_avoid_event_handler)(
			struct wlan_objmgr_psoc *psoc, void *arg);
	QDF_STATUS (*send_ctl_info)(struct wlan_objmgr_psoc *psoc,
				    struct reg_ctl_params *params);
};

/**
 * struct wlan_lmac_if_dfs_tx_ops - Function pointer to call offload/lmac
 *                                  functions from DFS module.
 * @dfs_enable:                         Enable DFS.
 * @dfs_get_caps:                       Get DFS capabilities.
 * @dfs_disable:                        Disable DFS
 * @dfs_gettsf64:                       Get tsf64 value.
 * @dfs_set_use_cac_prssi:              Set use_cac_prssi value.
 * @dfs_get_dfsdomain:                  Get DFS domain.
 * @dfs_is_countryCode_CHINA:           Check is country code CHINA.
 * @dfs_get_thresholds:                 Get thresholds.
 * @dfs_get_ext_busy:                   Get ext_busy.
 * @dfs_get_target_type:                Get target type.
 * @dfs_is_countryCode_KOREA_ROC3:      Check is county code Korea.
 * @dfs_get_ah_devid:                   Get ah devid.
 * @dfs_get_phymode_info:               Get phymode info.
 * @dfs_reg_ev_handler:                 Register dfs event handler.
 * @dfs_process_emulate_bang_radar_cmd: Process emulate bang radar test command.
 * @dfs_agile_ch_cfg_cmd:               Send Agile Channel Configuration command
 * @dfs_ocac_abort_cmd:                 Send Off-Channel CAC abort command.
 * @dfs_is_pdev_5ghz:                   Check if the given pdev is 5GHz.
 * @dfs_set_phyerr_filter_offload:      Config phyerr filter offload.
 * @dfs_send_offload_enable_cmd:        Send dfs offload enable command to fw.
 * @dfs_host_dfs_check_support:         To check Host DFS confirmation feature
 *                                      support.
 * @dfs_send_avg_radar_params_to_fw:    Send average radar parameters to FW.
 * @dfs_send_usenol_pdev_param:         Send usenol pdev param to FW.
 * @dfs_send_subchan_marking_pdev_param: Send subchan marking pdev param to FW.
 */

struct wlan_lmac_if_dfs_tx_ops {
	QDF_STATUS (*dfs_enable)(struct wlan_objmgr_pdev *pdev,
			int *is_fastclk,
			struct wlan_dfs_phyerr_param *param,
			uint32_t dfsdomain);
	QDF_STATUS (*dfs_get_caps)(struct wlan_objmgr_pdev *pdev,
			struct wlan_dfs_caps *dfs_caps);
	QDF_STATUS (*dfs_disable)(struct wlan_objmgr_pdev *pdev,
			int no_cac);
	QDF_STATUS (*dfs_gettsf64)(struct wlan_objmgr_pdev *pdev,
			uint64_t *tsf64);
	QDF_STATUS (*dfs_set_use_cac_prssi)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*dfs_get_thresholds)(struct wlan_objmgr_pdev *pdev,
			struct wlan_dfs_phyerr_param *param);
	QDF_STATUS (*dfs_get_ext_busy)(struct wlan_objmgr_pdev *pdev,
			int *dfs_ext_chan_busy);
	QDF_STATUS (*dfs_get_target_type)(struct wlan_objmgr_pdev *pdev,
			uint32_t *target_type);
	QDF_STATUS (*dfs_get_ah_devid)(struct wlan_objmgr_pdev *pdev,
			uint16_t *devid);
	QDF_STATUS (*dfs_get_phymode_info)(struct wlan_objmgr_pdev *pdev,
			uint32_t chan_mode,
			uint32_t *mode_info,
			bool is_2gvht_en);
	QDF_STATUS (*dfs_reg_ev_handler)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*dfs_process_emulate_bang_radar_cmd)(
			struct wlan_objmgr_pdev *pdev,
			struct dfs_emulate_bang_radar_test_cmd *dfs_unit_test);
	QDF_STATUS (*dfs_agile_ch_cfg_cmd)(
			struct wlan_objmgr_pdev *pdev,
			struct dfs_agile_cac_params *adfs_params);
	QDF_STATUS (*dfs_ocac_abort_cmd)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*dfs_is_pdev_5ghz)(struct wlan_objmgr_pdev *pdev,
			bool *is_5ghz);
	QDF_STATUS (*dfs_set_phyerr_filter_offload)(
			struct wlan_objmgr_pdev *pdev,
			bool dfs_phyerr_filter_offload);
	bool (*dfs_is_tgt_offload)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*dfs_send_offload_enable_cmd)(
			struct wlan_objmgr_pdev *pdev,
			bool enable);
	QDF_STATUS (*dfs_host_dfs_check_support)(struct wlan_objmgr_pdev *pdev,
						 bool *enabled);
	QDF_STATUS (*dfs_send_avg_radar_params_to_fw)(
			struct wlan_objmgr_pdev *pdev,
			struct dfs_radar_found_params *params);
	QDF_STATUS (*dfs_send_usenol_pdev_param)(struct wlan_objmgr_pdev *pdev,
						 bool usenol);
	QDF_STATUS (*dfs_send_subchan_marking_pdev_param)(
			struct wlan_objmgr_pdev *pdev,
			bool subchanmark);
};

/**
 * struct wlan_lmac_if_target_tx_ops - Function pointers to call target
 *                                     functions from other modules.
 * @tgt_is_tgt_type_ar900b:  To check AR900B target type.
 * @tgt_is_tgt_type_ipq4019: To check IPQ4019 target type.
 * @tgt_is_tgt_type_qca9984: To check QCA9984 target type.
 * @tgt_is_tgt_type_qca9888: To check QCA9888 target type.
 * @tgt_is_tgt_type_adrastea: To check QCS40X target type.
 * @tgt_get_tgt_type:        Get target type
 * @tgt_get_tgt_version:     Get target version
 * @tgt_get_tgt_revision:    Get target revision
 */
struct wlan_lmac_if_target_tx_ops {
	bool (*tgt_is_tgt_type_ar900b)(uint32_t);
	bool (*tgt_is_tgt_type_ipq4019)(uint32_t);
	bool (*tgt_is_tgt_type_qca9984)(uint32_t);
	bool (*tgt_is_tgt_type_qca9888)(uint32_t);
	bool (*tgt_is_tgt_type_adrastea)(uint32_t);
	uint32_t (*tgt_get_tgt_type)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*tgt_get_tgt_version)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*tgt_get_tgt_revision)(struct wlan_objmgr_psoc *psoc);
};

#ifdef WLAN_OFFCHAN_TXRX_ENABLE
/**
 * struct wlan_lmac_if_offchan_txrx_ops - Function pointers to check target
 *                                     capabilities related to offchan txrx.
 * @offchan_data_tid_support: To check if target supports separate tid for
 *                                     offchan data tx.
 */
struct wlan_lmac_if_offchan_txrx_ops {
	bool (*offchan_data_tid_support)(struct wlan_objmgr_pdev *pdev);
};
#endif

#ifdef WLAN_SUPPORT_GREEN_AP
struct wlan_green_ap_egap_params;
/**
 * struct wlan_lmac_if_green_ap_tx_ops - structure of tx function
 *                  pointers for green ap component
 * @enable_egap: function pointer to send enable egap indication to fw
 * @ps_on_off_send:  function pointer to send enable/disable green ap ps to fw
 */
struct wlan_lmac_if_green_ap_tx_ops {
	QDF_STATUS (*enable_egap)(struct wlan_objmgr_pdev *pdev,
				struct wlan_green_ap_egap_params *egap_params);
	QDF_STATUS (*ps_on_off_send)(struct wlan_objmgr_pdev *pdev,
				     bool value, uint8_t pdev_id);
	QDF_STATUS (*reset_dev)(struct wlan_objmgr_pdev *pdev);
	uint16_t (*get_current_channel)(struct wlan_objmgr_pdev *pdev);
	uint64_t (*get_current_channel_flags)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*get_capab)(struct  wlan_objmgr_pdev *pdev);
};
#endif

/**
 * struct wlan_lmac_if_tx_ops - south bound tx function pointers
 * @mgmt_txrx_tx_ops: mgmt txrx tx ops
 * @scan: scan tx ops
 * @dfs_tx_ops: dfs tx ops.
 * @green_ap_tx_ops: green_ap tx_ops
 * @cp_stats_tx_ops: cp stats tx_ops
 *
 * Callback function tabled to be registered with umac.
 * umac will use the functional table to send events/frames to lmac/wmi
 */

struct wlan_lmac_if_tx_ops {
	/* Components to declare function pointers required by the module
	 * in component specific structure.
	 * The component specific ops structure can be declared in this file
	 * only
	 */
	 struct wlan_lmac_if_mgmt_txrx_tx_ops mgmt_txrx_tx_ops;
	 struct wlan_lmac_if_scan_tx_ops scan;
#ifdef CONVERGED_P2P_ENABLE
	struct wlan_lmac_if_p2p_tx_ops p2p;
#endif
#ifdef QCA_SUPPORT_SON
	struct wlan_lmac_if_son_tx_ops son_tx_ops;
#endif

#ifdef WLAN_ATF_ENABLE
	struct wlan_lmac_if_atf_tx_ops atf_tx_ops;
#endif
#ifdef QCA_SUPPORT_CP_STATS
	struct wlan_lmac_if_cp_stats_tx_ops cp_stats_tx_ops;
#endif
#ifdef WLAN_SA_API_ENABLE
	struct wlan_lmac_if_sa_api_tx_ops sa_api_tx_ops;
#endif

#ifdef WLAN_CFR_ENABLE
	struct wlan_lmac_if_cfr_tx_ops cfr_tx_ops;
#endif

#ifdef WLAN_CONV_SPECTRAL_ENABLE
	struct wlan_lmac_if_sptrl_tx_ops sptrl_tx_ops;
#endif

#ifdef WLAN_CONV_CRYPTO_SUPPORTED
	struct wlan_lmac_if_crypto_tx_ops crypto_tx_ops;
#endif

#ifdef WIFI_POS_CONVERGED
	struct wlan_lmac_if_wifi_pos_tx_ops wifi_pos_tx_ops;
#endif
	struct wlan_lmac_if_reg_tx_ops reg_ops;
	struct wlan_lmac_if_dfs_tx_ops dfs_tx_ops;

#ifdef FEATURE_WLAN_TDLS
	struct wlan_lmac_if_tdls_tx_ops tdls_tx_ops;
#endif

#ifdef WLAN_SUPPORT_FILS
	struct wlan_lmac_if_fd_tx_ops fd_tx_ops;
#endif
	 struct wlan_lmac_if_mlme_tx_ops mops;
	 struct wlan_lmac_if_target_tx_ops target_tx_ops;

#ifdef WLAN_OFFCHAN_TXRX_ENABLE
	struct wlan_lmac_if_offchan_txrx_ops offchan_txrx_ops;
#endif

#ifdef DIRECT_BUF_RX_ENABLE
	struct wlan_lmac_if_direct_buf_rx_tx_ops dbr_tx_ops;
#endif

#ifdef WLAN_SUPPORT_GREEN_AP
	 struct wlan_lmac_if_green_ap_tx_ops green_ap_tx_ops;
#endif

	struct wlan_lmac_if_ftm_tx_ops ftm_tx_ops;
};

/**
 * struct wlan_lmac_if_mgmt_txrx_rx_ops - structure of rx function
 *                  pointers for mgmt txrx component
 * @mgmt_tx_completion_handler: function pointer to give tx completions
 *                              to mgmt txrx comp.
 * @mgmt_rx_frame_handler: function pointer to give rx frame to mgmt txrx comp.
 * @mgmt_txrx_get_nbuf_from_desc_id: function pointer to get nbuf from desc id
 * @mgmt_txrx_get_peer_from_desc_id: function pointer to get peer from desc id
 * @mgmt_txrx_get_vdev_id_from_desc_id: function pointer to get vdev id from
 *                                      desc id
 */
struct wlan_lmac_if_mgmt_txrx_rx_ops {
	QDF_STATUS (*mgmt_tx_completion_handler)(
			struct wlan_objmgr_pdev *pdev,
			uint32_t desc_id, uint32_t status,
			void *tx_compl_params);
	QDF_STATUS (*mgmt_rx_frame_handler)(
			struct wlan_objmgr_psoc *psoc,
			qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params);
	qdf_nbuf_t (*mgmt_txrx_get_nbuf_from_desc_id)(
			struct wlan_objmgr_pdev *pdev,
			uint32_t desc_id);
	struct wlan_objmgr_peer * (*mgmt_txrx_get_peer_from_desc_id)(
			struct wlan_objmgr_pdev *pdev, uint32_t desc_id);
	uint8_t (*mgmt_txrx_get_vdev_id_from_desc_id)(
			struct wlan_objmgr_pdev *pdev,
			uint32_t desc_id);
	uint32_t (*mgmt_txrx_get_free_desc_pool_count)(
			struct wlan_objmgr_pdev *pdev);
};

struct wlan_lmac_if_reg_rx_ops {
	QDF_STATUS (*master_list_handler)(struct cur_regulatory_info
					  *reg_info);
	QDF_STATUS (*reg_11d_new_cc_handler)(struct wlan_objmgr_psoc *psoc,
			struct reg_11d_new_country *reg_11d_new_cc);
	QDF_STATUS (*reg_set_regdb_offloaded)(struct wlan_objmgr_psoc *psoc,
			bool val);
	QDF_STATUS (*reg_set_11d_offloaded)(struct wlan_objmgr_psoc *psoc,
			bool val);
	QDF_STATUS (*reg_set_6ghz_supported)(struct wlan_objmgr_psoc *psoc,
					     bool val);
	QDF_STATUS (*get_dfs_region)(struct wlan_objmgr_pdev *pdev,
			enum dfs_reg *dfs_reg);
	QDF_STATUS (*reg_ch_avoid_event_handler)(struct wlan_objmgr_psoc *psoc,
			struct ch_avoid_ind_type *ch_avoid_ind);
	uint8_t (*reg_freq_to_chan)(struct wlan_objmgr_pdev *pdev,
				    qdf_freq_t freq);
	QDF_STATUS (*reg_set_chan_144)(struct wlan_objmgr_pdev *pdev,
			bool enable_ch_144);
	bool (*reg_get_chan_144)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*reg_program_default_cc)(struct wlan_objmgr_pdev *pdev,
			uint16_t regdmn);
	QDF_STATUS (*reg_get_current_regdomain)(struct wlan_objmgr_pdev *pdev,
			struct cur_regdmn_info *cur_regdmn);
	QDF_STATUS (*reg_enable_dfs_channels)(struct wlan_objmgr_pdev *pdev,
					      bool dfs_enable);
	QDF_STATUS (*reg_modify_pdev_chan_range)(struct
						 wlan_objmgr_pdev *pdev);
	QDF_STATUS (*reg_disable_chan_coex)(struct wlan_objmgr_pdev *pdev,
					    uint8_t unii_5g_bitmap);
	bool (*reg_ignore_fw_reg_offload_ind)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*reg_get_unii_5g_bitmap)(struct wlan_objmgr_pdev *pdev,
					     uint8_t *bitmap);
};

#ifdef CONVERGED_P2P_ENABLE

/* forward declarations for p2p rx ops */
struct p2p_noa_info;
struct p2p_lo_event;
struct p2p_set_mac_filter_evt;

/**
 * struct wlan_lmac_if_p2p_rx_ops - structure of rx function pointers
 * for P2P component
 * @lo_ev_handler:    function pointer to give listen offload event
 * @noa_ev_handler:   function pointer to give noa event
 * @add_mac_addr_filter_evt_handler: function pointer to process add mac addr
 *    rx filter event
 */
struct wlan_lmac_if_p2p_rx_ops {
#ifdef FEATURE_P2P_LISTEN_OFFLOAD
	QDF_STATUS (*lo_ev_handler)(struct wlan_objmgr_psoc *psoc,
		struct p2p_lo_event *event_info);
#endif
	QDF_STATUS (*noa_ev_handler)(struct wlan_objmgr_psoc *psoc,
		struct p2p_noa_info *event_info);
	QDF_STATUS (*add_mac_addr_filter_evt_handler)(
		struct wlan_objmgr_psoc *psoc,
		struct p2p_set_mac_filter_evt *event_info);

};
#endif

#ifdef WLAN_ATF_ENABLE

/**
 * struct wlan_lmac_if_atf_rx_ops - ATF south bound rx function pointers
 * @atf_get_atf_commit:                Get ATF commit state
 * @atf_get_fmcap:                     Get firmware capability for ATF
 * @atf_get_obss_scale:                Get OBSS scale
 * @atf_get_mode:                      Get mode of ATF
 * @atf_get_msdu_desc:                 Get msdu desc for ATF
 * @atf_get_max_vdevs:                 Get maximum vdevs for a Radio
 * @atf_get_peers:                     Get number of peers for a radio
 * @atf_get_tput_based:                Get throughput based enabled/disabled
 * @atf_get_logging:                   Get logging enabled/disabled
 * @atf_update_buf_held:               Set Num buf held by subgroup
 * @atf_get_ssidgroup:                 Get ssid group state
 * @atf_get_vdev_ac_blk_cnt:           Get AC block count for vdev
 * @atf_get_peer_blk_txbitmap:         Get peer tx traffic AC bitmap
 * @atf_get_vdev_blk_txtraffic:        Get vdev tx traffic block state
 * @atf_get_sched:                     Get ATF scheduled policy
 * @atf_get_tx_tokens:                 Get Tx tokens
 * @atf_buf_distribute:                Distribute Buffers
 * @atf_get_tx_tokens_common:          Get common tx tokens
 * @atf_get_shadow_alloted_tx_tokens:  Get shadow alloted tx tokens
 * @atf_get_peer_stats:                Get atf peer stats
 * @atf_adjust_subgroup_txtokens:      Adjust tokens based on actual duration
 * @atf_account_subgroup_txtokens:     Estimate tx time & update subgroup tokens
 * @atf_subgroup_free_buf:             On tx completion, update num buf held
 * @atf_update_subgroup_tidstate:      TID state (Paused/unpaused) of node
 * @atf_get_subgroup_airtime:          Get subgroup airtime
 * @atf_get_token_allocated:           Get atf token allocated
 * @atf_get_token_utilized:            Get atf token utilized
 * @atf_set_sched:                     Set ATF schedule policy
 * @atf_set_fmcap:                     Set firmware capability for ATF
 * @atf_set_obss_scale:                Set ATF obss scale
 * @atf_set_msdu_desc:                 Set msdu desc
 * @atf_set_max_vdevs:                 Set maximum vdevs number
 * @atf_set_peers:                     Set peers number
 * @atf_set_peer_stats:                Set peer stats
 * @atf_set_vdev_blk_txtraffic:        Set Block/unblock vdev tx traffic
 * @atf_peer_blk_txtraffic:            Block peer tx traffic
 * @atf_peer_unblk_txtraffic:          Unblock peer tx traffic
 * @atf_set_token_allocated:           Set atf token allocated
 * @atf_set_token_utilized:            Set atf token utilized
 */
struct wlan_lmac_if_atf_rx_ops {
	uint8_t (*atf_get_atf_commit)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_fmcap)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*atf_get_obss_scale)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_mode)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*atf_get_msdu_desc)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*atf_get_max_vdevs)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*atf_get_peers)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*atf_get_tput_based)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_logging)(struct wlan_objmgr_pdev *pdev);
	void*   (*atf_update_buf_held)(struct wlan_objmgr_peer *peer,
				       int8_t ac);
	uint32_t (*atf_get_ssidgroup)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_vdev_ac_blk_cnt)(struct wlan_objmgr_vdev *vdev);
	uint8_t (*atf_get_peer_blk_txbitmap)(struct wlan_objmgr_peer *peer);
	uint8_t (*atf_get_vdev_blk_txtraffic)(struct wlan_objmgr_vdev *vdev);
	uint32_t (*atf_get_sched)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_tx_tokens)(struct wlan_objmgr_peer *peer);
	uint32_t (*atf_buf_distribute)(struct wlan_objmgr_pdev *pdev,
				       struct wlan_objmgr_peer *peer,
				       int8_t ac);
	uint32_t (*atf_get_txtokens_common)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_shadow_alloted_tx_tokens)(
						struct wlan_objmgr_pdev *pdev);
	void (*atf_get_peer_stats)(struct wlan_objmgr_peer *peer,
				   struct atf_stats *stats);
	QDF_STATUS
	(*atf_adjust_subgroup_txtokens)(struct wlan_objmgr_peer *pr,
					uint8_t ac, uint32_t actual_duration,
					uint32_t est_duration);
	QDF_STATUS
	(*atf_account_subgroup_txtokens)(struct wlan_objmgr_peer *pr,
					 uint8_t ac,
					 uint32_t duration);
	QDF_STATUS
	(*atf_subgroup_free_buf)(uint16_t buf_acc_size, void *bf_atf_sg);
	QDF_STATUS
	(*atf_update_subgroup_tidstate)(struct wlan_objmgr_peer *peer,
					uint8_t atf_nodepaused);
	uint8_t (*atf_get_subgroup_airtime)(struct wlan_objmgr_peer *peer,
					    uint8_t ac);
	uint16_t (*atf_get_token_allocated)(struct wlan_objmgr_peer *peer);
	uint16_t (*atf_get_token_utilized)(struct wlan_objmgr_peer *peer);
	void (*atf_set_sched)(struct wlan_objmgr_pdev *pdev, uint32_t value);
	void (*atf_set_fmcap)(struct wlan_objmgr_psoc *psoc, uint32_t value);
	void (*atf_set_obss_scale)(struct wlan_objmgr_pdev *pdev,
				   uint32_t value);
	void (*atf_set_msdu_desc)(struct wlan_objmgr_psoc *psoc,
				  uint32_t value);
	void (*atf_set_max_vdevs)(struct wlan_objmgr_psoc *psoc,
				  uint32_t value);
	void (*atf_set_peers)(struct wlan_objmgr_psoc *psoc, uint32_t value);
	void (*atf_set_peer_stats)(struct wlan_objmgr_peer *peer,
				   struct atf_stats *stats);
	void (*atf_set_vdev_blk_txtraffic)(struct wlan_objmgr_vdev *vdev,
					   uint8_t value);
	void (*atf_peer_blk_txtraffic)(struct wlan_objmgr_peer *peer,
				       int8_t ac_id);
	void (*atf_peer_unblk_txtraffic)(struct wlan_objmgr_peer *peer,
					 int8_t ac_id);
	void (*atf_set_token_allocated)(struct wlan_objmgr_peer *peer,
					uint16_t value);
	void (*atf_set_token_utilized)(struct wlan_objmgr_peer *peer,
					uint16_t value);
};
#endif

#ifdef WLAN_SUPPORT_FILS
/**
 * struct wlan_lmac_if_fd_rx_ops - FILS Discovery specific Rx function pointers
 * @fd_is_fils_enable:      FILS enabled or not
 * @fd_alloc:               Allocate FD buffer
 * @fd_stop:                Stop and free deferred FD buffer
 * @fd_free:                Free FD frame buffer
 * @fd_get_valid_fd_period: Get valid FD period
 * @fd_swfda_handler:       SWFDA event handler
 * @fd_offload:             Offload FD frame
 */
struct wlan_lmac_if_fd_rx_ops {
	uint8_t (*fd_is_fils_enable)(struct wlan_objmgr_vdev *vdev);
	void (*fd_alloc)(struct wlan_objmgr_vdev *vdev);
	void (*fd_stop)(struct wlan_objmgr_vdev *vdev);
	void (*fd_free)(struct wlan_objmgr_vdev *vdev);
	uint32_t (*fd_get_valid_fd_period)(struct wlan_objmgr_vdev *vdev,
					   uint8_t *is_modified);
	QDF_STATUS (*fd_swfda_handler)(struct wlan_objmgr_vdev *vdev);
	QDF_STATUS (*fd_offload)(struct wlan_objmgr_vdev *vdev,
				 uint32_t vdev_id);
};
#endif

#ifdef WLAN_SA_API_ENABLE

/**
 * struct wlan_lmac_if_sa_api_rx_ops - SA API south bound rx function pointers
 */
struct wlan_lmac_if_sa_api_rx_ops {
	uint32_t (*sa_api_get_sa_supported)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*sa_api_get_validate_sw)(struct wlan_objmgr_psoc *psoc);
	void (*sa_api_enable_sa)(struct wlan_objmgr_psoc *psoc, uint32_t value);
	uint32_t (*sa_api_get_sa_enable)(struct wlan_objmgr_psoc *psoc);
	void (*sa_api_peer_assoc_hanldler)(struct wlan_objmgr_pdev *pdev,
			struct wlan_objmgr_peer *peer, struct sa_rate_cap *);
	uint32_t (*sa_api_update_tx_feedback)(struct wlan_objmgr_pdev *pdev,
			struct wlan_objmgr_peer *peer,
			struct sa_tx_feedback *feedback);
	uint32_t (*sa_api_update_rx_feedback)(struct wlan_objmgr_pdev *pdev,
			struct wlan_objmgr_peer *peer,
			struct sa_rx_feedback *feedback);
	uint32_t (*sa_api_ucfg_set_param)(struct wlan_objmgr_pdev *pdev,
			char *val);
	uint32_t (*sa_api_ucfg_get_param)(struct wlan_objmgr_pdev *pdev,
			char *val);
	uint32_t (*sa_api_is_tx_feedback_enabled)
			(struct wlan_objmgr_pdev *pdev);
	uint32_t (*sa_api_is_rx_feedback_enabled)
			(struct wlan_objmgr_pdev *pdev);
	uint32_t (*sa_api_convert_rate_2g)(uint32_t rate);
	uint32_t (*sa_api_convert_rate_5g)(uint32_t rate);
	uint32_t (*sa_api_get_sa_mode)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*sa_api_get_beacon_txantenna)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*sa_api_cwm_action)(struct wlan_objmgr_pdev *pdev);
};
#endif

#ifdef WLAN_CFR_ENABLE

/**
 * struct wlan_lmac_if_cfr_rx_ops - CFR south bound rx function pointers
 * @cfr_support_set: Set the CFR support based on FW advert
 * @cfr_info_send: Send cfr info to upper layers
 */
struct wlan_lmac_if_cfr_rx_ops {
	void (*cfr_support_set)(struct wlan_objmgr_psoc *psoc, uint32_t value);
	uint32_t (*cfr_info_send)(struct wlan_objmgr_pdev *pdev, void *head,
				  size_t hlen, void *data, size_t dlen,
				  void *tail, size_t tlen);
};
#endif

#ifdef WLAN_CONV_SPECTRAL_ENABLE
/**
 * struct wlan_lmac_if_sptrl_rx_ops - Spectral south bound Rx operations
 *
 * @sptrlro_get_target_handle: Get Spectral handle for target/LMAC private data
 * @sptrlro_vdev_get_chan_freq_seg2: Get secondary 80 center frequency
 * @sptrlro_spectral_is_feature_disabled: Check if spectral feature is disabled
 */
struct wlan_lmac_if_sptrl_rx_ops {
	void * (*sptrlro_get_target_handle)(struct wlan_objmgr_pdev *pdev);
	int16_t (*sptrlro_vdev_get_chan_freq)(struct wlan_objmgr_vdev *vdev);
	int16_t (*sptrlro_vdev_get_chan_freq_seg2)
					(struct wlan_objmgr_vdev *vdev);
	enum phy_ch_width (*sptrlro_vdev_get_ch_width)(
			struct wlan_objmgr_vdev *vdev);
	int (*sptrlro_vdev_get_sec20chan_freq_mhz)(
			struct wlan_objmgr_vdev *vdev,
			uint16_t *sec20chan_freq);
	bool (*sptrlro_spectral_is_feature_disabled)(
			struct wlan_objmgr_psoc *psoc);
};
#endif /* WLAN_CONV_SPECTRAL_ENABLE */

#ifdef WIFI_POS_CONVERGED
/**
 * struct wlan_lmac_if_wifi_pos_rx_ops - structure of rx function
 * pointers for wifi_pos component
 * @oem_rsp_event_rx: callback for WMI_OEM_RESPONSE_EVENTID
 */
struct wlan_lmac_if_wifi_pos_rx_ops {
	int (*oem_rsp_event_rx)(struct wlan_objmgr_psoc *psoc,
				struct oem_data_rsp *oem_rsp);
};
#endif

/**
 * struct wlan_lmac_if_dfs_rx_ops - Function pointers to call dfs functions
 *                                  from lmac/offload.
 * @dfs_get_radars:                   Calls init radar table functions.
 * @dfs_process_phyerr:               Process phyerr.
 * @dfs_destroy_object:               Destroys the DFS object.
 * @dfs_radar_enable:                 Enables the radar.
 * @dfs_is_radar_enabled:             Check if the radar is enabled.
 * @dfs_control:                      Used to process ioctls related to DFS.
 * @dfs_is_precac_timer_running:      Check whether precac timer is running.
 * @dfs_find_vht80_chan_for_precac:   Find VHT80 channel for precac.
 * @dfs_cancel_precac_timer:          Cancel the precac timer.
 * @dfs_override_precac_timeout:      Override the default precac timeout.
 * @dfs_set_precac_enable:            Set precac enable flag.
 * @dfs_get_legacy_precac_enable:     Get the precac enable flag for
 *                                    partial offload (legacy) chipsets.
 * @dfs_set_precac_intermediate_chan: Set intermediate channel for precac.
 * @dfs_get_precac_intermediate_chan: Get intermediate channel for precac.
 * @dfs_precac_preferred_chan:        Configure preferred channel during
 *                                    precac.
 * dfs_get_precac_chan_state:         Get precac status for given channel.
 * dfs_start_precac_timer:            Start precac timer.
 * @dfs_get_override_precac_timeout:  Get precac timeout.
 * @dfs_set_current_channel:          Set DFS current channel.
 * @dfs_process_radar_ind:            Process radar found indication.
 * @dfs_dfs_cac_complete_ind:         Process cac complete indication.
 * @dfs_agile_precac_start:           Initiate Agile PreCAC run.
 * @dfs_set_agile_precac_state:       Set agile precac state.
 * @dfs_reset_adfs_config:            Reset agile dfs variables.
 * @dfs_dfs_ocac_complete_ind:        Process offchan cac complete indication.
 * @dfs_stop:                         Clear dfs timers.
 * @dfs_reinit_timers:                Reinitialize DFS timers.
 * @dfs_enable_stadfs:                Enable/Disable STADFS capability.
 * @dfs_is_stadfs_enabled:            Get STADFS capability value.
 * @dfs_process_phyerr_filter_offload:Process radar event.
 * @dfs_is_phyerr_filter_offload:     Check whether phyerr filter is offload.
 * @dfs_action_on_status:             Trigger the action to be taken based on
 *                                    on host dfs status received from fw.
 * @dfs_override_status_timeout:      Override the value of host dfs status
 *                                    wait timeout.
 * @dfs_get_override_status_timeout:  Get the value of host dfs status wait
 *                                    timeout.
 * @dfs_reset_spoof_test:             Checks if radar detection is enabled.
 * @dfs_is_disable_radar_marking_set: Check if dis_radar_marking param is set.
 * @dfs_allow_hw_pulses:              Set or unset dfs_allow_hw_pulses which
 *                                    allow or disallow HW pulses.
 * @dfs_is_hw_pulses_allowed:         Check if HW pulses are allowed or not.
 * @dfs_set_fw_adfs_support:          Set the agile DFS FW support in DFS.
 * @dfs_reset_dfs_prevchan:           Reset DFS previous channel structure.
 */
struct wlan_lmac_if_dfs_rx_ops {
	QDF_STATUS (*dfs_get_radars)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*dfs_process_phyerr)(struct wlan_objmgr_pdev *pdev,
			void *buf,
			uint16_t datalen,
			uint8_t r_rssi,
			uint8_t r_ext_rssi,
			uint32_t r_rs_tstamp,
			uint64_t r_fulltsf);
	QDF_STATUS (*dfs_destroy_object)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*dfs_radar_enable)(
			struct wlan_objmgr_pdev *pdev,
			int no_cac,
			uint32_t opmode,
			bool enable);
	void (*dfs_is_radar_enabled)(struct wlan_objmgr_pdev *pdev,
				     int *ignore_dfs);
	QDF_STATUS (*dfs_control)(struct wlan_objmgr_pdev *pdev,
				  u_int id,
				  void *indata,
				  uint32_t insize,
				  void *outdata,
				  uint32_t *outsize,
				  int *error);
	QDF_STATUS (*dfs_is_precac_timer_running)(struct wlan_objmgr_pdev *pdev,
						  bool *is_precac_timer_running
						  );
#ifdef CONFIG_CHAN_NUM_API
	QDF_STATUS
	    (*dfs_find_vht80_chan_for_precac)(struct wlan_objmgr_pdev *pdev,
					      uint32_t chan_mode,
					      uint8_t ch_freq_seg1,
					      uint32_t *cfreq1,
					      uint32_t *cfreq2,
					      uint32_t *phy_mode,
					      bool *dfs_set_cfreq2,
					      bool *set_agile);
#endif
#ifdef CONFIG_CHAN_FREQ_API
	QDF_STATUS
	    (*dfs_find_vht80_chan_for_precac_for_freq)(struct wlan_objmgr_pdev
						       *pdev,
						       uint32_t chan_mode,
						       uint16_t ch_freq_seg1,
						       uint32_t *cfreq1,
						       uint32_t *cfreq2,
						       uint32_t *phy_mode,
						       bool *dfs_set_cfreq2,
						       bool *set_agile);
#endif
	QDF_STATUS (*dfs_agile_precac_start)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*dfs_set_agile_precac_state)(struct wlan_objmgr_pdev *pdev,
						 int agile_precac_state);
	QDF_STATUS (*dfs_reset_adfs_config)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS
	(*dfs_dfs_ocac_complete_ind)(struct wlan_objmgr_pdev *pdev,
				     struct vdev_adfs_complete_status *ocac_st);
	QDF_STATUS (*dfs_start_precac_timer)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*dfs_cancel_precac_timer)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*dfs_override_precac_timeout)(
			struct wlan_objmgr_pdev *pdev,
			int precac_timeout);
	QDF_STATUS (*dfs_set_precac_enable)(struct wlan_objmgr_pdev *pdev,
			uint32_t value);
	QDF_STATUS
	(*dfs_get_legacy_precac_enable)(struct wlan_objmgr_pdev *pdev,
					bool *buff);
	QDF_STATUS (*dfs_get_agile_precac_enable)(struct wlan_objmgr_pdev *pdev,
						  bool *buff);
#ifdef WLAN_DFS_PRECAC_AUTO_CHAN_SUPPORT
	QDF_STATUS (*dfs_set_precac_intermediate_chan)(struct wlan_objmgr_pdev *pdev,
						       uint32_t value);
	QDF_STATUS (*dfs_get_precac_intermediate_chan)(struct wlan_objmgr_pdev *pdev,
						       int *buff);
#ifdef CONFIG_CHAN_NUM_API
	bool (*dfs_decide_precac_preferred_chan)(struct wlan_objmgr_pdev *pdev,
						 uint8_t *pref_chan,
						 enum wlan_phymode mode);
#endif
#ifdef CONFIG_CHAN_FREQ_API
	bool (*dfs_decide_precac_preferred_chan_for_freq)(struct
						    wlan_objmgr_pdev *pdev,
						    uint16_t *pref_chan_freq,
						    enum wlan_phymode mode);
#endif

#ifdef CONFIG_CHAN_NUM_API
	enum precac_chan_state (*dfs_get_precac_chan_state)(struct wlan_objmgr_pdev *pdev,
							    uint8_t precac_chan);
#endif

#ifdef CONFIG_CHAN_FREQ_API
	enum precac_chan_state (*dfs_get_precac_chan_state_for_freq)(struct
						      wlan_objmgr_pdev *pdev,
						      uint16_t pcac_freq);
#endif
#endif
	QDF_STATUS (*dfs_get_override_precac_timeout)(
			struct wlan_objmgr_pdev *pdev,
			int *precac_timeout);
#ifdef CONFIG_CHAN_NUM_API
	QDF_STATUS (*dfs_set_current_channel)(struct wlan_objmgr_pdev *pdev,
			uint16_t ic_freq,
			uint64_t ic_flags,
			uint16_t ic_flagext,
			uint8_t ic_ieee,
			uint8_t ic_vhtop_ch_freq_seg1,
			uint8_t ic_vhtop_ch_freq_seg2);
#endif
#ifdef CONFIG_CHAN_FREQ_API
	QDF_STATUS
	    (*dfs_set_current_channel_for_freq)(struct wlan_objmgr_pdev *pdev,
						uint16_t ic_freq,
						uint64_t ic_flags,
						uint16_t ic_flagext,
						uint8_t ic_ieee,
						uint8_t ic_vhtop_ch_freq_seg1,
						uint8_t ic_vhtop_ch_freq_seg2,
						uint16_t dfs_ch_mhz_freq_seg1,
						uint16_t dfs_ch_mhz_freq_seg2);
#endif
#ifdef DFS_COMPONENT_ENABLE
	QDF_STATUS (*dfs_process_radar_ind)(struct wlan_objmgr_pdev *pdev,
			struct radar_found_info *radar_found);
	QDF_STATUS (*dfs_dfs_cac_complete_ind)(struct wlan_objmgr_pdev *pdev,
			uint32_t vdev_id);
#endif
	QDF_STATUS (*dfs_stop)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*dfs_reinit_timers)(struct wlan_objmgr_pdev *pdev);
	void (*dfs_enable_stadfs)(struct wlan_objmgr_pdev *pdev, bool val);
	bool (*dfs_is_stadfs_enabled)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*dfs_process_phyerr_filter_offload)(
			struct wlan_objmgr_pdev *pdev,
			struct radar_event_info *wlan_radar_info);
	QDF_STATUS (*dfs_is_phyerr_filter_offload)(
			struct wlan_objmgr_psoc *psoc,
			bool *is_phyerr_filter_offload);
	QDF_STATUS (*dfs_action_on_status)(struct wlan_objmgr_pdev *pdev,
			u_int32_t *dfs_status_check);
	QDF_STATUS (*dfs_override_status_timeout)(
			struct wlan_objmgr_pdev *pdev,
			int status_timeout);
	QDF_STATUS (*dfs_get_override_status_timeout)(
			struct wlan_objmgr_pdev *pdev,
			int *status_timeout);
	QDF_STATUS (*dfs_reset_spoof_test)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*dfs_is_disable_radar_marking_set)(struct wlan_objmgr_pdev
						 *pdev,
						 bool *disable_radar_marking);
	QDF_STATUS (*dfs_set_nol_subchannel_marking)(
			struct wlan_objmgr_pdev *pdev,
			bool value);
	QDF_STATUS (*dfs_get_nol_subchannel_marking)(
			struct wlan_objmgr_pdev *pdev,
			bool *value);
	QDF_STATUS (*dfs_set_bw_reduction)(struct wlan_objmgr_pdev *pdev,
			bool value);
	QDF_STATUS (*dfs_is_bw_reduction_needed)(struct wlan_objmgr_pdev *pdev,
			bool *bw_reduce);
	void (*dfs_allow_hw_pulses)(struct wlan_objmgr_pdev *pdev,
				    bool allow_hw_pulses);
	bool (*dfs_is_hw_pulses_allowed)(struct wlan_objmgr_pdev *pdev);
	void (*dfs_set_fw_adfs_support)(struct wlan_objmgr_pdev *pdev,
					bool fw_adfs_support_160,
					bool fw_adfs_support_non_160);
	void (*dfs_reset_dfs_prevchan)(struct wlan_objmgr_pdev *pdev);
};

/**
 * struct wlan_lmac_if_mlme_rx_ops: Function pointer to call MLME functions
 * @vdev_mgr_start_response: function to handle start response
 * @vdev_mgr_stop_response: function to handle stop response
 * @vdev_mgr_delete_response: function to handle delete response
 * @vdev_mgr_offload_bcn_tx_status_event_handle: function to handle offload
 * beacon tx
 * @vdev_mgr_tbttoffset_update_handle: function to handle tbtt offset event
 * @vdev_mgr_peer_delete_all_response: function to handle vdev delete all peer
 * event
 * @psoc_get_wakelock_info: function to get wakelock info
 * @psoc_get_vdev_response_timer_info: function to get vdev response timer
 * structure for a specific vdev id
 */
struct wlan_lmac_if_mlme_rx_ops {
	QDF_STATUS (*vdev_mgr_start_response)(
					struct wlan_objmgr_psoc *psoc,
					struct vdev_start_response *rsp);
	QDF_STATUS (*vdev_mgr_stop_response)(
					struct wlan_objmgr_psoc *psoc,
					struct vdev_stop_response *rsp);
	QDF_STATUS (*vdev_mgr_delete_response)(
					struct wlan_objmgr_psoc *psoc,
					struct vdev_delete_response *rsp);
	QDF_STATUS (*vdev_mgr_offload_bcn_tx_status_event_handle)(
							uint32_t vdev_id,
							uint32_t tx_status);
	QDF_STATUS (*vdev_mgr_tbttoffset_update_handle)(
						uint32_t num_vdevs,
						bool is_ext);
	QDF_STATUS (*vdev_mgr_peer_delete_all_response)(
					struct wlan_objmgr_psoc *psoc,
					struct peer_delete_all_response *rsp);
#ifdef FEATURE_VDEV_RSP_WAKELOCK
	struct vdev_mlme_wakelock *(*psoc_get_wakelock_info)(
				    struct wlan_objmgr_psoc *psoc);
#endif
	struct vdev_response_timer *(*psoc_get_vdev_response_timer_info)(
						struct wlan_objmgr_psoc *psoc,
						uint8_t vdev_id);
};

#ifdef WLAN_SUPPORT_GREEN_AP
struct wlan_lmac_if_green_ap_rx_ops {
	bool (*is_ps_enabled)(struct wlan_objmgr_pdev *pdev);
	bool (*is_dbg_print_enabled)(struct wlan_objmgr_pdev *pdev);
	QDF_STATUS (*ps_get)(struct wlan_objmgr_pdev *pdev, uint8_t *value);
	QDF_STATUS (*ps_set)(struct wlan_objmgr_pdev *pdev, uint8_t value);
	void (*suspend_handle)(struct wlan_objmgr_pdev *pdev);
};
#endif

/**
 * struct wlan_lmac_if_rx_ops - south bound rx function pointers
 * @mgmt_txrx_tx_ops: mgmt txrx rx ops
 * @scan: scan rx ops
 * @dfs_rx_ops: dfs rx ops.
 * @cp_stats_rx_ops: cp stats rx ops
 * @cfr_rx_ops: cfr rx ops
 *
 * Callback function tabled to be registered with lmac/wmi.
 * lmac will use the functional table to send events/frames to umac
 */
struct wlan_lmac_if_rx_ops {
	/* Components to declare function pointers required by the module
	 * in component specific structure.
	 * The component specific ops structure can be declared in this file
	 * only
	 */
	 struct wlan_lmac_if_mgmt_txrx_rx_ops mgmt_txrx_rx_ops;
	 struct wlan_lmac_if_scan_rx_ops scan;
#ifdef CONVERGED_P2P_ENABLE
	struct wlan_lmac_if_p2p_rx_ops p2p;
#endif

#ifdef WLAN_ATF_ENABLE
	struct wlan_lmac_if_atf_rx_ops atf_rx_ops;
#endif
#ifdef QCA_SUPPORT_CP_STATS
	struct wlan_lmac_if_cp_stats_rx_ops cp_stats_rx_ops;
#endif
#ifdef WLAN_SA_API_ENABLE
	struct wlan_lmac_if_sa_api_rx_ops sa_api_rx_ops;
#endif

#ifdef WLAN_CFR_ENABLE
	struct wlan_lmac_if_cfr_rx_ops cfr_rx_ops;
#endif

#ifdef WLAN_CONV_SPECTRAL_ENABLE
	struct wlan_lmac_if_sptrl_rx_ops sptrl_rx_ops;
#endif

#ifdef WLAN_CONV_CRYPTO_SUPPORTED
	struct wlan_lmac_if_crypto_rx_ops crypto_rx_ops;
#endif
#ifdef WIFI_POS_CONVERGED
	struct wlan_lmac_if_wifi_pos_rx_ops wifi_pos_rx_ops;
#endif
	struct wlan_lmac_if_reg_rx_ops reg_rx_ops;
	struct wlan_lmac_if_dfs_rx_ops dfs_rx_ops;
#ifdef FEATURE_WLAN_TDLS
	struct wlan_lmac_if_tdls_rx_ops tdls_rx_ops;
#endif

#ifdef WLAN_SUPPORT_FILS
	struct wlan_lmac_if_fd_rx_ops fd_rx_ops;
#endif

	struct wlan_lmac_if_mlme_rx_ops mops;

#ifdef WLAN_SUPPORT_GREEN_AP
	struct wlan_lmac_if_green_ap_rx_ops green_ap_rx_ops;
#endif

	struct wlan_lmac_if_ftm_rx_ops ftm_rx_ops;
};

/* Function pointer to call legacy tx_ops registration in OL/WMA.
 */
extern QDF_STATUS (*wlan_lmac_if_umac_tx_ops_register)
				(struct wlan_lmac_if_tx_ops *tx_ops);
#endif /* _WLAN_LMAC_IF_DEF_H_ */
