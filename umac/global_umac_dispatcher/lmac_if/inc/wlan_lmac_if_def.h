/*
 * Copyright (c) 2016-2017 The Linux Foundation. All rights reserved.
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
#include "wlan_mgmt_txrx_utils_api.h"
#include "wlan_scan_public_structs.h"
#ifdef WLAN_ATF_ENABLE
#include "wlan_atf_utils_defs.h"
#endif

#if WLAN_CRYPTO_SUPPORTED
#include "wlan_crypto_global_def.h"
#endif

/* Number of dev type: Direct attach and Offload */
#define MAX_DEV_TYPE 2

#ifdef WIFI_POS_CONVERGED
/* forward declarations */
struct oem_data_req;
struct oem_data_rsp;
#endif /* WIFI_POS_CONVERGED */

/**
 * struct wlan_lmac_if_mgmt_txrx_tx_ops - structure of tx function
 *                  pointers for mgmt txrx component
 * @mgmt_tx_send: function pointer to transmit mgmt tx frame
 * @beacon_send:  function pointer to transmit beacon frame
 */
struct wlan_lmac_if_mgmt_txrx_tx_ops {
	QDF_STATUS (*mgmt_tx_send)(struct wlan_objmgr_vdev *vdev,
			qdf_nbuf_t nbuf, u_int32_t desc_id,
			void *mgmt_tx_params);
	QDF_STATUS (*beacon_send)(struct wlan_objmgr_vdev *vdev,
			qdf_nbuf_t nbuf);
};

/**
 * struct wlan_lmac_if_scan_tx_ops - south bound tx function pointers for scan
 * @scan_start: function to start scan
 * @scan_cancel: function to cancel scan
 * @scan_reg_ev_handler: function to register for scan events
 * @scan_unreg_ev_handler: function to unregister for scan events
 *
 * scan module uses these functions to avail ol/da lmac services
 */
struct wlan_lmac_if_scan_tx_ops {
	QDF_STATUS (*scan_start)(struct wlan_objmgr_psoc *psoc,
			struct scan_start_request *req);
	QDF_STATUS (*scan_cancel)(struct wlan_objmgr_psoc *psoc,
			struct scan_cancel_param *req);
	QDF_STATUS (*scan_reg_ev_handler)(struct wlan_objmgr_psoc *psoc,
			void *arg);
	QDF_STATUS (*scan_unreg_ev_handler)(struct wlan_objmgr_psoc *psoc,
			void *arg);
	QDF_STATUS (*set_chan_list)(struct wlan_objmgr_pdev *pdev, void *arg);
};


/**
 * struct wlan_lmac_if_scan_rx_ops  - south bound rx function pointers for scan
 * @scan_ev_handler: scan event handler
 *
 * lmac modules uses this API to post scan events to scan module
 */
struct wlan_lmac_if_scan_rx_ops {
	QDF_STATUS (*scan_ev_handler)(struct wlan_objmgr_psoc *psoc,
		struct scan_event_info *event_info);
};

#ifdef WLAN_PMO_ENABLE

/* fwd declarations for pmo tx ops */
struct pmo_arp_offload_params;
struct pmo_ns_offload_params;
struct pmo_gtk_req;
struct pmo_action_wakeup_set_params;
struct pmo_lphb_enable_req;
struct pmo_lphb_tcp_params;
struct pmo_lphb_tcp_filter_req;
struct pmo_lphb_udp_params;
struct pmo_lphb_udp_filter_req;
struct pmo_wow_cmd_params;
struct pmo_suspend_params;

/**
 * struct wlan_lmac_if_pmo_tx_ops - structure of tx function
 *					pointers for pmo component
 * @send_arp_offload_req: fp to send arp offload request
 * @send_ns_offload_req: fp to send ns offload request
 * @send_enable_wakeup_event_req: fp to send enable wow wakeup events req
 * @send_disable_wakeup_event_req: fp to send disable wow wakeup events req
 * @send_add_wow_pattern: fp to send wow pattern request
 * @send_enhance_mc_offload_req: fp to send enhanced multicast offload request
 * @send_set_mc_filter_req: fp to send set mc filter request
 * @send_clear_mc_filter_req: fp to send clear mc filter request
 * @send_ra_filter_req: fp to send ra filter request
 * @send_gtk_offload_req: fp to send gtk offload request command
 * @send_get_gtk_rsp_cmd: fp to send get gtk response request cmd to firmware
 * @send_action_frame_pattern_req: fp to send wow action frame patterns request
 * @send_lphb_enable: fp to send lphb enable request command
 * @send_lphb_tcp_params: fp to send lphb tcp params request command
 * @send_lphb_tcp_filter_req: fp to send lphb tcp packet filter request command
 * @send_lphb_upd_params: fp to send lphb udp params request command
 * @send_lphb_udp_filter_req: fp to send lphb udp packet filter request command
 * @send_vdev_param_update_req: fp to send vdev param request
 * @send_vdev_set_sta_ps_param: fp to send sta vdev ps power set req
 * @psoc_update_wow_bus_suspend: fp to update bus suspend req flag at wmi
 * @psoc_get_host_credits: fp to get the host credits
 * @psoc_get_pending_cmnds: fp to get the host pending wmi commands
 * @update_target_suspend_flag: fp to update target suspend flag at wmi
 * @psoc_send_wow_enable_req: fp to send wow enable request
 * @psoc_send_supend_req: fp to send target suspend request
 * @psoc_get_runtime_pm_in_progress: fp to get runtime pm is in progress status
 * @psoc_send_host_wakeup_ind: fp tp send host wake indication to fwr
 * @psoc_send_target_resume_req: fp to send target resume request
 */
struct wlan_lmac_if_pmo_tx_ops {
	QDF_STATUS(*send_arp_offload_req)(struct wlan_objmgr_vdev *vdev,
			struct pmo_arp_offload_params *arp_offload_req,
			struct pmo_ns_offload_params *ns_offload_req);
	QDF_STATUS(*send_ns_offload_req)(struct wlan_objmgr_vdev *vdev,
			struct pmo_arp_offload_params *arp_offload_req,
			struct pmo_ns_offload_params *ns_offload_req);
	QDF_STATUS(*send_enable_wow_wakeup_event_req)(
			struct wlan_objmgr_vdev *vdev,
			uint32_t bitmap);
	QDF_STATUS(*send_disable_wow_wakeup_event_req)(
			struct wlan_objmgr_vdev *vdev,
			uint32_t bitmap);
	QDF_STATUS(*send_add_wow_pattern)(
			struct wlan_objmgr_vdev *vdev,
			uint8_t ptrn_id, const uint8_t *ptrn, uint8_t ptrn_len,
			uint8_t ptrn_offset, const uint8_t *mask,
			uint8_t mask_len, bool user);
	QDF_STATUS(*send_enhance_mc_offload_req)(
			struct wlan_objmgr_vdev *vdev, bool enable);
	QDF_STATUS(*send_set_mc_filter_req)(
			struct wlan_objmgr_vdev *vdev,
			struct qdf_mac_addr multicast_addr);
	QDF_STATUS(*send_clear_mc_filter_req)(
			struct wlan_objmgr_vdev *vdev,
			struct qdf_mac_addr multicast_addr);
	QDF_STATUS(*send_ra_filter_req)(
			struct wlan_objmgr_vdev *vdev,
			uint8_t default_pattern, uint16_t rate_limit_interval);
	QDF_STATUS(*send_gtk_offload_req)(
			struct wlan_objmgr_vdev *vdev,
			struct pmo_gtk_req *gtk_offload_req);
	QDF_STATUS(*send_get_gtk_rsp_cmd)(struct wlan_objmgr_vdev *vdev);
	QDF_STATUS(*send_action_frame_pattern_req)(
			struct wlan_objmgr_vdev *vdev,
			struct pmo_action_wakeup_set_params *ip_cmd);
	QDF_STATUS(*send_lphb_enable)(
			struct wlan_objmgr_psoc *psoc,
			struct pmo_lphb_enable_req *ts_lphb_enable);
	QDF_STATUS(*send_lphb_tcp_params)(
			struct wlan_objmgr_psoc *psoc,
			struct pmo_lphb_tcp_params *ts_lphb_tcp_param);
	QDF_STATUS(*send_lphb_tcp_filter_req)(
			struct wlan_objmgr_psoc *psoc,
			struct pmo_lphb_tcp_filter_req *ts_lphb_tcp_filter);
	QDF_STATUS(*send_lphb_upd_params)(
			struct wlan_objmgr_psoc *psoc,
			struct pmo_lphb_udp_params *ts_lphb_udp_param);
	QDF_STATUS(*send_lphb_udp_filter_req)(
			struct wlan_objmgr_psoc *psoc,
			struct pmo_lphb_udp_filter_req *ts_lphb_udp_filter);
	QDF_STATUS(*send_vdev_param_update_req)(
			struct wlan_objmgr_vdev *vdev,
			uint32_t param_id, uint32_t param_value);
	QDF_STATUS(*send_vdev_sta_ps_param_req)(
			struct wlan_objmgr_vdev *vdev,
			uint32_t ps_mode, uint32_t value);
	void(*psoc_update_wow_bus_suspend)(
			struct wlan_objmgr_psoc *psoc, uint8_t value);
	int(*psoc_get_host_credits)(
			struct wlan_objmgr_psoc *psoc);
	int(*psoc_get_pending_cmnds)(
			struct wlan_objmgr_psoc *psoc);
	void(*update_target_suspend_flag)(
		struct wlan_objmgr_psoc *psoc, uint8_t value);
	QDF_STATUS(*psoc_send_wow_enable_req)(struct wlan_objmgr_psoc *psoc,
		struct pmo_wow_cmd_params *param);
	QDF_STATUS(*psoc_send_supend_req)(struct wlan_objmgr_psoc *psoc,
		struct pmo_suspend_params *param);
	bool(*psoc_get_runtime_pm_in_progress)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS(*psoc_send_host_wakeup_ind)(struct wlan_objmgr_psoc *psoc);
	QDF_STATUS (*psoc_send_target_resume_req)(
			struct wlan_objmgr_psoc *psoc);

};
#endif

#ifdef CONVERGED_P2P_ENABLE

/* forward declarations for p2p tx ops */
struct p2p_ps_config;
struct p2p_lo_start;

/**
 * struct wlan_lmac_if_p2p_tx_ops - structure of tx function pointers
 * for P2P component
 * @set_ps:      function pointer to set power save
 * @lo_start:    function pointer to start listen offload
 * @lo_stop:     function pointer to stop listen offload
 * @reg_lo_ev_handler:   function pointer to register lo event handler
 * @reg_noa_ev_handler:  function pointer to register noa event handler
 * @unreg_lo_ev_handler: function pointer to unregister lo event handler
 * @unreg_noa_ev_handler:function pointer to unregister noa event handler
 */
struct wlan_lmac_if_p2p_tx_ops {
	QDF_STATUS (*set_ps)(struct wlan_objmgr_psoc *psoc,
		struct p2p_ps_config *ps_config);
	QDF_STATUS (*lo_start)(struct wlan_objmgr_psoc *psoc,
		struct p2p_lo_start *lo_start);
	QDF_STATUS (*lo_stop)(struct wlan_objmgr_psoc *psoc,
		uint32_t vdev_id);
	QDF_STATUS (*reg_lo_ev_handler)(struct wlan_objmgr_psoc *psoc,
			void *arg);
	QDF_STATUS (*reg_noa_ev_handler)(struct wlan_objmgr_psoc *psoc,
			void *arg);
	QDF_STATUS (*unreg_lo_ev_handler)(struct wlan_objmgr_psoc *psoc,
			void *arg);
	QDF_STATUS (*unreg_noa_ev_handler)(struct wlan_objmgr_psoc *psoc,
			void *arg);
};
#endif

#ifdef WLAN_ATF_ENABLE

/**
 * struct wlan_lmac_if_atf_tx_ops - ATF specific tx function pointers
 * @atf_update_peer_txtoken:      Update peer Tx token
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
	void (*atf_update_peer_txtoken)(struct wlan_objmgr_pdev *pdev,
					struct wlan_objmgr_peer *peer,
					struct atf_stats *stats);
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
					struct wlan_objmgr_peer *peer);

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

#ifdef WIFI_POS_CONVERGED
/*
 * struct wlan_lmac_if_wifi_pos_tx_ops - structure of firmware tx function
 * pointers for wifi_pos component
 * @data_req_tx: function pointer to send wifi_pos req to firmware
 */
struct wlan_lmac_if_wifi_pos_tx_ops {
	QDF_STATUS (*data_req_tx)(struct wlan_objmgr_psoc *psoc,
				  struct oem_data_req *req);
};
#endif

#ifdef WLAN_FEATURE_NAN_CONVERGENCE
/**
 * struct wlan_lmac_if_nan_tx_ops - structure of firwware tx function
 * pointers for nan component
 * @data_req_tx: function pointer to send nan req to firmware
 */
struct wlan_lmac_if_nan_tx_ops {
	QDF_STATUS (*nan_req_tx)(struct wlan_objmgr_vdev *vdev, void *req);
};
#endif

/**
 * struct wlan_lmac_if_tx_ops - south bound tx function pointers
 * @mgmt_txrx_tx_ops: mgmt txrx tx ops
 * @scan: scan tx ops
 * @set_chan_list: tx func for configuring scan channel
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
#ifdef WLAN_PMO_ENABLE
	 struct wlan_lmac_if_pmo_tx_ops pmo_tx_ops;
#endif
#ifdef CONVERGED_P2P_ENABLE
	struct wlan_lmac_if_p2p_tx_ops p2p;
#endif
#ifdef WLAN_ATF_ENABLE
	struct wlan_lmac_if_atf_tx_ops atf_tx_ops;
#endif
#if WLAN_CRYPTO_SUPPORTED
	struct wlan_lmac_if_crypto_tx_ops crypto_tx_ops;
#endif
#ifdef WIFI_POS_CONVERGED
	struct wlan_lmac_if_wifi_pos_tx_ops wifi_pos_tx_ops;
#endif
#ifdef WLAN_FEATURE_NAN_CONVERGENCE
	struct wlan_lmac_if_nan_tx_ops nan_tx_ops;
#endif
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
			struct wlan_objmgr_psoc *psoc,
			uint32_t desc_id, uint32_t status,
			void *tx_compl_params);
	QDF_STATUS (*mgmt_rx_frame_handler)(
			struct wlan_objmgr_psoc *psoc,
			qdf_nbuf_t buf,
			struct mgmt_rx_event_params *mgmt_rx_params);
	qdf_nbuf_t (*mgmt_txrx_get_nbuf_from_desc_id)(
			struct wlan_objmgr_psoc *psoc,
			uint32_t desc_id);
	struct wlan_objmgr_peer * (*mgmt_txrx_get_peer_from_desc_id)(
			struct wlan_objmgr_psoc *psoc, uint32_t desc_id);
	uint8_t (*mgmt_txrx_get_vdev_id_from_desc_id)(
			struct wlan_objmgr_psoc *psoc,
			uint32_t desc_id);
};

#ifdef WLAN_PMO_ENABLE

/* fwd declarations for pmo rx ops */
struct pmo_gtk_rsp_params;
struct pmo_lphb_rsp;

/**
 * struct wlan_lmac_if_pmo_rx_ops - structure of rx function
 *                  pointers for pmo component
 * @gtk_rsp_event: function pointer to handle gtk rsp event from fwr
 * @lphb_rsp_event: function pointer to handle lphb rsp event from fwr
 */
struct wlan_lmac_if_pmo_rx_ops {
	QDF_STATUS(*rx_gtk_rsp_event)(struct wlan_objmgr_psoc *psoc,
			struct pmo_gtk_rsp_params *rsp_param);
	QDF_STATUS (*lphb_rsp_event)(struct wlan_objmgr_psoc *psoc,
			struct pmo_lphb_rsp *rsp_param);
};
#endif

#ifdef CONVERGED_P2P_ENABLE

/* forward declarations for p2p rx ops */
struct p2p_noa_info;
struct p2p_lo_event;

/**
 * struct wlan_lmac_if_p2p_rx_ops - structure of rx function pointers
 * for P2P component
 * @lo_ev_handler:    function pointer to give listen offload event
 * @noa_ev_handler:   function pointer to give noa event
 */
struct wlan_lmac_if_p2p_rx_ops {
	QDF_STATUS (*lo_ev_handler)(struct wlan_objmgr_psoc *psoc,
		struct p2p_lo_event *event_info);
	QDF_STATUS (*noa_ev_handler)(struct wlan_objmgr_psoc *psoc,
		struct p2p_noa_info *event_info);
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
 * @atf_get_txbuf_share:               Get TxBuff share state
 * @atf_get_txbuf_max:                 Get TxBuff MAX number
 * @atf_get_txbuf_min:                 Get TxBuff MIN number
 * @atf_get_ssidgroup:                 Get ssid group state
 * @atf_get_tx_block_count:            Get tx block count
 * @atf_get_peer_blk_txtraffic:        Get peer tx traffic block state
 * @atf_get_vdev_blk_txtraffic:        Get vdev tx traffic block state
 * @atf_get_sched:                     Get ATF scheduled policy
 * @atf_get_tx_tokens:                 Get Tx tokens
 * @atf_get_shadow_tx_tokens:          Get shadow tx tokens
 * @atf_get_tx_tokens_common:          Get common tx tokens
 * @atf_get_shadow_alloted_tx_tokens:  Get shadow alloted tx tokens
 * @atf_get_peer_stats:                Get atf peer stats
 * @atf_get_token_allocated:           Get atf token allocated
 * @atf_get_token_utilized:            Get atf token utilized
 * @atf_set_sched:                     Set ATF schedule policy
 * @atf_set_fmcap:                     Set firmware capability for ATF
 * @atf_set_obss_scale:                Set ATF obss scale
 * @atf_set_mode:                      Set ATF mode
 * @atf_set_msdu_desc:                 Set msdu desc
 * @atf_set_max_vdevs:                 Set maximum vdevs number
 * @atf_set_peers:                     Set peers number
 * @atf_set_peer_stats:                Set peer stats
 * @atf_set_vdev_blk_txtraffic:        Set Block/unblock vdev tx traffic
 * @atf_set_peer_blk_txtraffic:        Set Block/unblock peer tx traffic
 * @atf_set_tx_block_count:            Set tx block count
 * @atf_set_token_allocated:           Set atf token allocated
 * @atf_set_token_utilized:            Set atf token utilized
 */
struct wlan_lmac_if_atf_rx_ops {
	uint32_t (*atf_get_atf_commit)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_fmcap)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*atf_get_obss_scale)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_mode)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*atf_get_msdu_desc)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*atf_get_max_vdevs)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*atf_get_peers)(struct wlan_objmgr_psoc *psoc);
	uint32_t (*atf_get_tput_based)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_logging)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_txbuf_share)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_txbuf_max)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_txbuf_min)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_ssidgroup)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_tx_block_count)(struct wlan_objmgr_vdev *vdev);
	uint8_t (*atf_get_peer_blk_txtraffic)(struct wlan_objmgr_peer *peer);
	uint8_t (*atf_get_vdev_blk_txtraffic)(struct wlan_objmgr_vdev *vdev);
	uint32_t (*atf_get_sched)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_tx_tokens)(struct wlan_objmgr_peer *peer);
	uint32_t (*atf_get_shadow_tx_tokens)(struct wlan_objmgr_peer *peer);
	uint32_t (*atf_get_txtokens_common)(struct wlan_objmgr_pdev *pdev);
	uint32_t (*atf_get_shadow_alloted_tx_tokens)(
						struct wlan_objmgr_pdev *pdev);
	void (*atf_get_peer_stats)(struct wlan_objmgr_peer *peer,
				   struct atf_stats *stats);
	uint16_t (*atf_get_token_allocated)(struct wlan_objmgr_peer *peer);
	uint16_t (*atf_get_token_utilized)(struct wlan_objmgr_peer *peer);

	void (*atf_set_sched)(struct wlan_objmgr_pdev *pdev, uint32_t value);
	void (*atf_set_fmcap)(struct wlan_objmgr_psoc *psoc, uint32_t value);
	void (*atf_set_obss_scale)(struct wlan_objmgr_pdev *pdev,
				   uint32_t value);
	void (*atf_set_mode)(struct wlan_objmgr_psoc *psoc, uint8_t value);
	void (*atf_set_msdu_desc)(struct wlan_objmgr_psoc *psoc,
				  uint32_t value);
	void (*atf_set_max_vdevs)(struct wlan_objmgr_psoc *psoc,
				  uint32_t value);
	void (*atf_set_peers)(struct wlan_objmgr_psoc *psoc, uint32_t value);
	void (*atf_set_peer_stats)(struct wlan_objmgr_peer *peer,
				   struct atf_stats *stats);
	void (*atf_set_vdev_blk_txtraffic)(struct wlan_objmgr_vdev *vdev,
					   uint8_t value);
	void (*atf_set_peer_blk_txtraffic)(struct wlan_objmgr_peer *peer,
					   uint8_t value);
	void (*atf_set_tx_block_count)(struct wlan_objmgr_vdev *vdev,
					uint32_t count);
	void (*atf_set_token_allocated)(struct wlan_objmgr_peer *peer,
					uint16_t value);
	void (*atf_set_token_utilized)(struct wlan_objmgr_peer *peer,
					uint16_t value);
};
#endif

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

#ifdef WLAN_FEATURE_NAN_CONVERGENCE
/**
 * struct wlan_lmac_if_nan_rx_ops - structure of rx function
 * pointers for nan component
 * @oem_rsp_event_rx: callback for WMI_OEM_RESPONSE_EVENTID
 */
struct wlan_lmac_if_nan_rx_ops {
	int (*nan_event_rx)(struct wlan_objmgr_vdev *vdev, void *ev);
};
#endif

/**
 * struct wlan_lmac_if_rx_ops - south bound rx function pointers
 * @arg1
 * @arg2
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
#ifdef WLAN_PMO_ENABLE
	 struct wlan_lmac_if_pmo_rx_ops pmo_rx_ops;
#endif
#ifdef CONVERGED_P2P_ENABLE
	struct wlan_lmac_if_p2p_rx_ops p2p;
#endif
#ifdef WLAN_ATF_ENABLE
	struct wlan_lmac_if_atf_rx_ops atf_rx_ops;
#endif
#if WLAN_CRYPTO_SUPPORTED
	struct wlan_lmac_if_crypto_rx_ops crypto_rx_ops;
#endif
#ifdef WIFI_POS_CONVERGED
	struct wlan_lmac_if_wifi_pos_rx_ops wifi_pos_rx_ops;
#endif
#ifdef WLAN_FEATURE_NAN_CONVERGENCE
	struct wlan_lmac_if_nan_rx_ops nan_rx_ops;
#endif
};

/* Function pointer to call legacy tx_ops registration in OL/WMA.
 */
extern QDF_STATUS (*wlan_lmac_if_umac_tx_ops_register)
				(struct wlan_lmac_if_tx_ops *tx_ops);
#endif /* _WLAN_LMAC_IF_DEF_H_ */
