/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

/**
 * @file cdp_txrx_ops.h
 * @brief Define the host data path converged API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_CMN_OPS_H_
#define _CDP_TXRX_CMN_OPS_H_

#include <cdp_txrx_cmn_struct.h>
#include <cdp_txrx_stats_struct.h>
#include "cdp_txrx_handle.h"
#include <cdp_txrx_mon_struct.h>
#include "wlan_objmgr_psoc_obj.h"

#ifdef IPA_OFFLOAD
#ifdef CONFIG_IPA_WDI_UNIFIED_API
#include <qdf_ipa_wdi3.h>
#else
#include <qdf_ipa.h>
#endif
#endif

/**
 * bitmap values to indicate special handling of peer_delete
 */
#define CDP_PEER_DELETE_NO_SPECIAL             0
#define CDP_PEER_DO_NOT_START_UNMAP_TIMER      1

/* same as ieee80211_nac_param */
enum cdp_nac_param_cmd {
	/* IEEE80211_NAC_PARAM_ADD */
	CDP_NAC_PARAM_ADD = 1,
	/* IEEE80211_NAC_PARAM_DEL */
	CDP_NAC_PARAM_DEL,
	/* IEEE80211_NAC_PARAM_LIST */
	CDP_NAC_PARAM_LIST,
};
/******************************************************************************
 *
 * Control Interface (A Interface)
 *
 *****************************************************************************/

struct cdp_cmn_ops {

	int (*txrx_soc_attach_target)(ol_txrx_soc_handle soc);

	int (*txrx_pdev_attach_target)(struct cdp_pdev *pdev);

	struct cdp_vdev *(*txrx_vdev_attach)
		(struct cdp_pdev *pdev, uint8_t *vdev_mac_addr,
		 uint8_t vdev_id, enum wlan_op_mode op_mode);

	void (*txrx_vdev_detach)
		(struct cdp_vdev *vdev, ol_txrx_vdev_delete_cb callback,
		 void *cb_context);

	struct cdp_pdev *(*txrx_pdev_attach)
		(ol_txrx_soc_handle soc, struct cdp_cfg *ctrl_pdev,
		HTC_HANDLE htc_pdev, qdf_device_t osdev, uint8_t pdev_id);

	int (*txrx_pdev_post_attach)(struct cdp_pdev *pdev);

	void (*txrx_pdev_pre_detach)(struct cdp_pdev *pdev, int force);

	void (*txrx_pdev_detach)(struct cdp_pdev *pdev, int force);

	void *(*txrx_peer_create)
		(struct cdp_vdev *vdev, uint8_t *peer_mac_addr);

	void (*txrx_peer_setup)
		(struct cdp_vdev *vdev_hdl, void *peer_hdl);

	void (*txrx_peer_teardown)
		(struct cdp_vdev *vdev_hdl, void *peer_hdl);

	int (*txrx_peer_add_ast)
		(ol_txrx_soc_handle soc, struct cdp_peer *peer_hdl,
		uint8_t *mac_addr, enum  cdp_txrx_ast_entry_type type,
		uint32_t flags);

	void (*txrx_peer_del_ast)
		(ol_txrx_soc_handle soc, void *ast_hdl);

	int (*txrx_peer_update_ast)
		(ol_txrx_soc_handle soc, struct cdp_peer *peer_hdl,
		void *ast_hdl, uint32_t flags);

	void *(*txrx_peer_ast_hash_find)
		(ol_txrx_soc_handle soc, uint8_t *ast_mac_addr);

	uint8_t (*txrx_peer_ast_get_pdev_id)
		(ol_txrx_soc_handle soc, void *ast_hdl);

	uint8_t (*txrx_peer_ast_get_next_hop)
		(ol_txrx_soc_handle soc, void *ast_hdl);

	void (*txrx_peer_ast_set_type)
		(ol_txrx_soc_handle soc, void *ast_hdl,
		enum cdp_txrx_ast_entry_type type);

	void (*txrx_peer_delete)(void *peer, uint32_t bitmap);

	int (*txrx_set_monitor_mode)(struct cdp_vdev *vdev,
			uint8_t smart_monitor);

	uint8_t (*txrx_get_pdev_id_frm_pdev)(struct cdp_pdev *pdev);

	void (*txrx_set_nac)(struct cdp_peer *peer);

	void (*txrx_set_pdev_tx_capture)(struct cdp_pdev *pdev, int val);

	void (*txrx_get_peer_mac_from_peer_id)
		(struct cdp_pdev *pdev_handle,
		 uint32_t peer_id, uint8_t *peer_mac);

	void (*txrx_vdev_tx_lock)(struct cdp_vdev *vdev);

	void (*txrx_vdev_tx_unlock)(struct cdp_vdev *vdev);

	void (*txrx_ath_getstats)(struct cdp_pdev *pdev,
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2, 6, 36)
			struct rtnl_link_stats64 *stats);
#else
			struct net_device_stats *stats);
#endif

	void (*txrx_set_gid_flag)(struct cdp_pdev *pdev, u_int8_t *mem_status,
			u_int8_t *user_position);

	uint32_t (*txrx_fw_supported_enh_stats_version)(struct cdp_pdev *pdev);

	void (*txrx_if_mgmt_drain)(void *ni, int force);

	void (*txrx_set_curchan)(struct cdp_pdev *pdev, uint32_t chan_mhz);

	void (*txrx_set_privacy_filters)
		(struct cdp_vdev *vdev, void *filter, uint32_t num);

	/********************************************************************
	 * Data Interface (B Interface)
	 ********************************************************************/

	void (*txrx_vdev_register)(struct cdp_vdev *vdev,
			void *osif_vdev, struct ol_txrx_ops *txrx_ops);

	int (*txrx_mgmt_send)(struct cdp_vdev *vdev,
			qdf_nbuf_t tx_mgmt_frm, uint8_t type);

	int (*txrx_mgmt_send_ext)(struct cdp_vdev *vdev,
			qdf_nbuf_t tx_mgmt_frm,	uint8_t type, uint8_t use_6mbps,
			uint16_t chanfreq);

	/**
	 * ol_txrx_mgmt_tx_cb - tx management delivery notification
	 * callback function
	 */

	void (*txrx_mgmt_tx_cb_set)(struct cdp_pdev *pdev, uint8_t type,
				    ol_txrx_mgmt_tx_cb download_cb,
				    ol_txrx_mgmt_tx_cb ota_ack_cb,
				    void *ctxt);

	int (*txrx_get_tx_pending)(struct cdp_pdev *pdev);

	/**
	 * ol_txrx_data_tx_cb - Function registered with the data path
	 * that is called when tx frames marked as "no free" are
	 * done being transmitted
	 */

	void (*txrx_data_tx_cb_set)(struct cdp_vdev *data_vdev,
			ol_txrx_data_tx_cb callback, void *ctxt);

	/*******************************************************************
	 * Statistics and Debugging Interface (C Inteface)
	 ********************************************************************/

	int (*txrx_aggr_cfg)(struct cdp_vdev *vdev, int max_subfrms_ampdu,
			int max_subfrms_amsdu);

	A_STATUS (*txrx_fw_stats_get)(struct cdp_vdev *vdev,
			struct ol_txrx_stats_req *req,
			bool per_vdev, bool response_expected);

	int (*txrx_debug)(struct cdp_vdev *vdev, int debug_specs);

	void (*txrx_fw_stats_cfg)(struct cdp_vdev *vdev,
			uint8_t cfg_stats_type, uint32_t cfg_val);

	void (*txrx_print_level_set)(unsigned level);

	/**
	 * ol_txrx_get_vdev_mac_addr() - Return mac addr of vdev
	 * @vdev: vdev handle
	 *
	 * Return: vdev mac address
	 */
	uint8_t * (*txrx_get_vdev_mac_addr)(struct cdp_vdev *vdev);

	/**
	 * ol_txrx_get_vdev_struct_mac_addr() - Return handle to struct qdf_mac_addr of
	 * vdev
	 * @vdev: vdev handle
	 *
	 * Return: Handle to struct qdf_mac_addr
	 */
	struct qdf_mac_addr *
		(*txrx_get_vdev_struct_mac_addr)(struct cdp_vdev *vdev);

	/**
	 * ol_txrx_get_pdev_from_vdev() - Return handle to pdev of vdev
	 * @vdev: vdev handle
	 *
	 * Return: Handle to pdev
	 */
	struct cdp_pdev *(*txrx_get_pdev_from_vdev)
		(struct cdp_vdev *vdev);

	/**
	 * ol_txrx_get_ctrl_pdev_from_vdev() - Return control pdev of vdev
	 * @vdev: vdev handle
	 *
	 * Return: Handle to control pdev
	 */
	struct cdp_cfg *
		(*txrx_get_ctrl_pdev_from_vdev)(struct cdp_vdev *vdev);

	struct cdp_vdev *
		(*txrx_get_vdev_from_vdev_id)(struct cdp_pdev *pdev,
				uint8_t vdev_id);

	void (*txrx_soc_detach)(void *soc);

	int (*addba_requestprocess)(void *peer_handle, uint8_t dialogtoken,
		uint16_t tid, uint16_t batimeout, uint16_t buffersize,
		uint16_t startseqnum);

	void (*addba_responsesetup)(void *peer_handle, uint8_t tid,
		uint8_t *dialogtoken, uint16_t *statuscode,
		uint16_t *buffersize, uint16_t *batimeout);

	int (*delba_process)(void *peer_handle,
		int tid, uint16_t reasoncode);

	void (*set_addba_response)(void *peer_handle,
		uint8_t tid, uint16_t statuscode);

	uint8_t (*get_peer_mac_addr_frm_id)(struct cdp_soc_t *soc_handle,
			uint16_t peer_id, uint8_t *mac_addr);

	void (*set_vdev_dscp_tid_map)(struct cdp_vdev *vdev_handle,
			uint8_t map_id);

	void (*flush_cache_rx_queue)(void);
	void (*set_pdev_dscp_tid_map)(struct cdp_pdev *pdev, uint8_t map_id,
			uint8_t tos, uint8_t tid);

	int (*txrx_stats_request)(struct cdp_vdev *vdev,
			struct cdp_txrx_stats_req *req);

	QDF_STATUS (*display_stats)(void *psoc, uint16_t value,
				    enum qdf_stats_verbosity_level level);
	void (*txrx_soc_set_nss_cfg)(ol_txrx_soc_handle soc, int config);

	int(*txrx_soc_get_nss_cfg)(ol_txrx_soc_handle soc);
	QDF_STATUS (*txrx_intr_attach)(void *soc);
	void (*txrx_intr_detach)(void *soc);
	void  (*set_pn_check)(struct cdp_vdev *vdev,
		struct cdp_peer *peer_handle, enum cdp_sec_type sec_type,
		 uint32_t *rx_pn);
	QDF_STATUS (*update_config_parameters)(struct cdp_soc *psoc,
			struct cdp_config_params *params);

	void *(*get_dp_txrx_handle)(struct cdp_pdev *pdev_hdl);
	void (*set_dp_txrx_handle)(struct cdp_pdev *pdev_hdl,
			void *dp_txrx_hdl);

	void *(*get_soc_dp_txrx_handle)(struct cdp_soc *soc_handle);
	void (*set_soc_dp_txrx_handle)(struct cdp_soc *soc_handle,
			void *dp_txrx_handle);

	ol_txrx_tx_fp tx_send;
};

struct cdp_ctrl_ops {

	int
		(*txrx_mempools_attach)(void *ctrl_pdev);
	int
		(*txrx_set_filter_neighbour_peers)(
				struct cdp_pdev *pdev,
				uint32_t val);
	int
		(*txrx_update_filter_neighbour_peers)(
				struct cdp_pdev *pdev,
				uint32_t cmd, uint8_t *macaddr);
	/**
	 * @brief set the safemode of the device
	 * @details
	 * This flag is used to bypass the encrypt and decrypt processes when
	 * send and receive packets. It works like open AUTH mode, HW will
	 * ctreate all packets as non-encrypt frames because no key installed.
	 * For rx fragmented frames,it bypasses all the rx defragmentaion.
	 *
	 * @param vdev - the data virtual device object
	 * @param val - the safemode state
	 * @return - void
	 */

	void
		(*txrx_set_safemode)(
				struct cdp_vdev *vdev,
				u_int32_t val);
	/**
	 * @brief configure the drop unencrypted frame flag
	 * @details
	 * Rx related. When set this flag, all the unencrypted frames
	 * received over a secure connection will be discarded
	 *
	 * @param vdev - the data virtual device object
	 * @param val - flag
	 * @return - void
	 */
	void
		(*txrx_set_drop_unenc)(
				struct cdp_vdev *vdev,
				u_int32_t val);


	/**
	 * @brief set the Tx encapsulation type of the VDEV
	 * @details
	 * This will be used to populate the HTT desc packet type field
	 * during Tx
	 * @param vdev - the data virtual device object
	 * @param val - the Tx encap type
	 * @return - void
	 */
	void
		(*txrx_set_tx_encap_type)(
				struct cdp_vdev *vdev,
				enum htt_cmn_pkt_type val);
	/**
	 * @brief set the Rx decapsulation type of the VDEV
	 * @details
	 * This will be used to configure into firmware and hardware
	 * which format to decap all Rx packets into, for all peers under
	 * the VDEV.
	 * @param vdev - the data virtual device object
	 * @param val - the Rx decap mode
	 * @return - void
	 */
	void
		(*txrx_set_vdev_rx_decap_type)(
				struct cdp_vdev *vdev,
				enum htt_cmn_pkt_type val);

	/**
	 * @brief get the Rx decapsulation type of the VDEV
	 *
	 * @param vdev - the data virtual device object
	 * @return - the Rx decap type
	 */
	enum htt_cmn_pkt_type
		(*txrx_get_vdev_rx_decap_type)(struct cdp_vdev *vdev);

	/* Is this similar to ol_txrx_peer_state_update() in MCL */
	/**
	 * @brief Update the authorize peer object at association time
	 * @details
	 * For the host-based implementation of rate-control, it
	 * updates the peer/node-related parameters within rate-control
	 * context of the peer at association.
	 *
	 * @param peer - pointer to the node's object
	 * @authorize - either to authorize or unauthorize peer
	 *
	 * @return none
	 */
	void
		(*txrx_peer_authorize)(struct cdp_peer *peer,
				u_int32_t authorize);

	bool
		(*txrx_set_inact_params)(struct cdp_pdev *pdev,
				u_int16_t inact_check_interval,
				u_int16_t inact_normal,
				u_int16_t inact_overload);
	bool
		(*txrx_start_inact_timer)(
				struct cdp_pdev *pdev,
				bool enable);


	/**
	 * @brief Set the overload status of the radio
	 * @details
	 *  Set the overload status of the radio, updating the inactivity
	 *  threshold and inactivity count for each node.
	 *
	 * @param pdev - the data physical device object
	 * @param overload - whether the radio is overloaded or not
	 */
	void (*txrx_set_overload)(
			struct cdp_pdev *pdev,
			bool overload);
	/**
	 * @brief Check the inactivity status of the peer/node
	 *
	 * @param peer - pointer to the node's object
	 * @return true if the node is inactive; otherwise return false
	 */
	bool
		(*txrx_peer_is_inact)(void *peer);

	/**
	 * @brief Mark inactivity status of the peer/node
	 * @details
	 *  If it becomes active, reset inactivity count to reload value;
	 *  if the inactivity status changed, notify umac band steering.
	 *
	 * @param peer - pointer to the node's object
	 * @param inactive - whether the node is inactive or not
	 */
	void (*txrx_mark_peer_inact)(
			void *peer,
			bool inactive);


	/* Should be ol_txrx_ctrl_api.h */
	void (*txrx_set_mesh_mode)(struct cdp_vdev *vdev, u_int32_t val);

	/**
	 * @brief setting mesh rx filter
	 * @details
	 *  based on the bits enabled in the filter packets has to be dropped.
	 *
	 * @param vdev - the data virtual device object
	 * @param val - value to set
	 */
	void (*txrx_set_mesh_rx_filter)(struct cdp_vdev *vdev, uint32_t val);

	void (*tx_flush_buffers)(struct cdp_vdev *vdev);

	int (*txrx_is_target_ar900b)(struct cdp_vdev *vdev);

	void (*txrx_set_vdev_param)(struct cdp_vdev *vdev,
			enum cdp_vdev_param_type param, uint32_t val);

	void (*txrx_peer_set_nawds)(struct cdp_peer *peer, uint8_t value);
	/**
	 * @brief Set the reo dest ring num of the radio
	 * @details
	 *  Set the reo destination ring no on which we will receive
	 *  pkts for this radio.
	 *
	 * @param pdev - the data physical device object
	 * @param reo_dest_ring_num - value ranges between 1 - 4
	 */
	void (*txrx_set_pdev_reo_dest)(
			struct cdp_pdev *pdev,
			enum cdp_host_reo_dest_ring reo_dest_ring_num);

	/**
	 * @brief Get the reo dest ring num of the radio
	 * @details
	 *  Get the reo destination ring no on which we will receive
	 *  pkts for this radio.
	 *
	 * @param pdev - the data physical device object
	 * @return the reo destination ring number
	 */
	enum cdp_host_reo_dest_ring (*txrx_get_pdev_reo_dest)(
						struct cdp_pdev *pdev);

	int (*txrx_wdi_event_sub)(struct cdp_pdev *pdev, void *event_cb_sub,
			uint32_t event);

	int (*txrx_wdi_event_unsub)(struct cdp_pdev *pdev, void *event_cb_sub,
			uint32_t event);
	int (*txrx_get_sec_type)(struct cdp_peer *peer, uint8_t sec_idx);

	void (*txrx_update_mgmt_txpow_vdev)(struct cdp_vdev *vdev,
			uint8_t subtype, uint8_t tx_power);

	void (*txrx_set_pdev_param)(struct cdp_pdev *pdev,
			enum cdp_pdev_param_type type, uint8_t val);
	void * (*txrx_get_pldev)(struct cdp_pdev *pdev);

#ifdef ATH_SUPPORT_NAC_RSSI
	QDF_STATUS (*txrx_vdev_config_for_nac_rssi)(struct cdp_vdev *vdev,
		enum cdp_nac_param_cmd cmd, char *bssid, char *client_macaddr,
		uint8_t chan_num);
#endif
};

struct cdp_me_ops {

	u_int16_t (*tx_desc_alloc_and_mark_for_mcast_clone)
		(struct cdp_pdev *pdev, u_int16_t buf_count);

		u_int16_t (*tx_desc_free_and_unmark_for_mcast_clone)(
				struct cdp_pdev *pdev,
				u_int16_t buf_count);

	u_int16_t
		(*tx_get_mcast_buf_allocated_marked)
			(struct cdp_pdev *pdev);
	void
		(*tx_me_alloc_descriptor)(struct cdp_pdev *pdev);

	void
		(*tx_me_free_descriptor)(struct cdp_pdev *pdev);

	uint16_t
		(*tx_me_convert_ucast)(struct cdp_vdev *vdev,
			qdf_nbuf_t wbuf, u_int8_t newmac[][6],
			uint8_t newmaccnt);
	/* Should be a function pointer in ol_txrx_osif_ops{} */
	/**
	 * @brief notify mcast frame indication from FW.
	 * @details
	 *     This notification will be used to convert
	 *     multicast frame to unicast.
	 *
	 * @param pdev - handle to the ctrl SW's physical device object
	 * @param vdev_id - ID of the virtual device received the special data
	 * @param msdu - the multicast msdu returned by FW for host inspect
	 */

	int (*mcast_notify)(struct cdp_pdev *pdev,
			u_int8_t vdev_id, qdf_nbuf_t msdu);
};

struct cdp_mon_ops {

	void (*txrx_monitor_set_filter_ucast_data)
		(struct cdp_pdev *, u_int8_t val);
	void (*txrx_monitor_set_filter_mcast_data)
		(struct cdp_pdev *, u_int8_t val);
	void (*txrx_monitor_set_filter_non_data)
		(struct cdp_pdev *, u_int8_t val);

	bool (*txrx_monitor_get_filter_ucast_data)
		(struct cdp_vdev *vdev_txrx_handle);
	bool (*txrx_monitor_get_filter_mcast_data)
		(struct cdp_vdev *vdev_txrx_handle);
	bool (*txrx_monitor_get_filter_non_data)
		(struct cdp_vdev *vdev_txrx_handle);
	int (*txrx_reset_monitor_mode)(struct cdp_pdev *pdev);

	/* HK advance monitor filter support */
	int (*txrx_set_advance_monitor_filter)
		(struct cdp_pdev *pdev, struct cdp_monitor_filter *filter_val);
};

struct cdp_host_stats_ops {
	int (*txrx_host_stats_get)(struct cdp_vdev *vdev,
			struct ol_txrx_stats_req *req);

	void (*txrx_host_stats_clr)(struct cdp_vdev *vdev);

	void (*txrx_host_ce_stats)(struct cdp_vdev *vdev);

	int (*txrx_stats_publish)(struct cdp_pdev *pdev,
			void *buf);
	/**
	 * @brief Enable enhanced stats functionality.
	 *
	 * @param pdev - the physical device object
	 * @return - void
	 */
	void (*txrx_enable_enhanced_stats)(struct cdp_pdev *pdev);

	/**
	 * @brief Disable enhanced stats functionality.
	 *
	 * @param pdev - the physical device object
	 * @return - void
	 */
	void (*txrx_disable_enhanced_stats)(struct cdp_pdev *pdev);

	/**
	 * @brief Get the desired stats from the message.
	 *
	 * @param pdev - the physical device object
	 * @param stats_base - stats buffer recieved from FW
	 * @param type - stats type.
	 * @return - pointer to requested stat identified by type
	 */
	uint32_t * (*txrx_get_stats_base)(struct cdp_pdev *pdev,
			uint32_t *stats_base, uint32_t msg_len, uint8_t type);
	void
		(*tx_print_tso_stats)(struct cdp_vdev *vdev);

	void
		(*tx_rst_tso_stats)(struct cdp_vdev *vdev);

	void
		(*tx_print_sg_stats)(struct cdp_vdev *vdev);

	void
		(*tx_rst_sg_stats)(struct cdp_vdev *vdev);

	void
		(*print_rx_cksum_stats)(struct cdp_vdev *vdev);

	void
		(*rst_rx_cksum_stats)(struct cdp_vdev *vdev);

	A_STATUS
		(*txrx_host_me_stats)(struct cdp_vdev *vdev);
	void
		(*txrx_per_peer_stats)(struct cdp_pdev *pdev, char *addr);
	int (*txrx_host_msdu_ttl_stats)(struct cdp_vdev *vdev,
			struct ol_txrx_stats_req *req);

	void
		(*print_lro_stats)(struct cdp_vdev *vdev);

	void
		(*reset_lro_stats)(struct cdp_vdev *vdev);

	void
		(*get_fw_peer_stats)(struct cdp_pdev *pdev, uint8_t *addr,
				uint32_t cap);
	void
		(*get_htt_stats)(struct cdp_pdev *pdev, void *data,
				uint32_t data_len);
};

struct cdp_wds_ops {
	void
		(*txrx_set_wds_rx_policy)(struct cdp_vdev *vdev,
				u_int32_t val);
	void
		(*txrx_wds_peer_tx_policy_update)(struct cdp_peer *peer,
				int wds_tx_ucast, int wds_tx_mcast);
	int (*vdev_set_wds)(void *vdev, uint32_t val);
};

struct cdp_raw_ops {
	int (*txrx_get_nwifi_mode)(struct cdp_vdev *vdev);

	void (*rsim_get_astentry)(struct cdp_vdev *vdev,
				qdf_nbuf_t *pnbuf,
				struct cdp_raw_ast *raw_ast);
};

#ifdef CONFIG_WIN
struct cdp_pflow_ops {
	uint32_t(*pflow_update_pdev_params)(void *,
			enum _ol_ath_param_t, uint32_t, void *);
};
#endif /* CONFIG_WIN */

#define LRO_IPV4_SEED_ARR_SZ 5
#define LRO_IPV6_SEED_ARR_SZ 11

/**
 * struct cdp_lro_config - set LRO init parameters
 * @lro_enable: indicates whether lro is enabled
 * @tcp_flag: If the TCP flags from the packet do not match
 * the values in this field after masking with TCP flags mask
 * below, packet is not LRO eligible
 * @tcp_flag_mask: field for comparing the TCP values provided
 * above with the TCP flags field in the received packet
 * @toeplitz_hash_ipv4: contains seed needed to compute the flow id
 * 5-tuple toeplitz hash for ipv4 packets
 * @toeplitz_hash_ipv6: contains seed needed to compute the flow id
 * 5-tuple toeplitz hash for ipv6 packets
 */
struct cdp_lro_hash_config {
	uint32_t lro_enable;
	uint32_t tcp_flag:9,
		tcp_flag_mask:9;
	uint32_t toeplitz_hash_ipv4[LRO_IPV4_SEED_ARR_SZ];
	uint32_t toeplitz_hash_ipv6[LRO_IPV6_SEED_ARR_SZ];
};

struct ol_if_ops {
	void (*peer_set_default_routing)(void *scn_handle,
			uint8_t *peer_macaddr, uint8_t vdev_id,
			bool hash_based, uint8_t ring_num);
	int (*peer_rx_reorder_queue_setup)(void *scn_handle,
			uint8_t vdev_id, uint8_t *peer_mac,
			qdf_dma_addr_t hw_qdesc, int tid, uint16_t queue_num);
	int (*peer_rx_reorder_queue_remove)(void *scn_handle,
			uint8_t vdev_id, uint8_t *peer_macaddr,
			uint32_t tid_mask);
	int (*peer_unref_delete)(void *scn_handle, uint8_t vdev_id,
			uint8_t *peer_macaddr);
	bool (*is_hw_dbs_2x2_capable)(struct wlan_objmgr_psoc *psoc);
	int (*peer_add_wds_entry)(void *ol_soc_handle,
			const uint8_t *dest_macaddr, uint8_t *peer_macaddr,
			uint32_t flags);
	int (*peer_update_wds_entry)(void *ol_soc_handle,
			uint8_t *dest_macaddr, uint8_t *peer_macaddr,
			uint32_t flags);
	void (*peer_del_wds_entry)(void *ol_soc_handle,
			uint8_t *wds_macaddr);
	QDF_STATUS (*lro_hash_config)(void *scn_handle,
			struct cdp_lro_hash_config *lro_hash);
	void (*update_dp_stats)(void *soc, void *stats, uint16_t id,
			uint8_t type);
	uint8_t (*rx_invalid_peer)(void *osif_pdev, void *msg);

	int  (*peer_map_event)(void *ol_soc_handle, uint16_t peer_id, uint16_t hw_peer_id,
			uint8_t vdev_id, uint8_t *peer_mac_addr,
			enum cdp_txrx_ast_entry_type peer_type);
	int (*peer_unmap_event)(void *ol_soc_handle, uint16_t peer_id);

	int (*get_dp_cfg_param)(void *ol_soc_handle, enum cdp_cfg_param_type param_num);

	void (*rx_mic_error)(void *ol_soc_handle,
			 uint16_t vdev_id, void *wh);
	uint8_t (*freq_to_channel)(void *ol_soc_handle,  uint16_t vdev_id);

	void (*record_act_change)(struct wlan_objmgr_pdev *pdev,
				  u_int8_t *dstmac, bool active);
#ifdef ATH_SUPPORT_NAC_RSSI
	int (*config_fw_for_nac_rssi)(struct wlan_objmgr_pdev *pdev,
		u_int8_t vdev_id, enum cdp_nac_param_cmd cmd, char *bssid,
		char *client_macaddr, uint8_t chan_num);
	int (*config_bssid_in_fw_for_nac_rssi)(struct wlan_objmgr_pdev *pdev,
		u_int8_t vdev_id, enum cdp_nac_param_cmd cmd, char *bssid);
#endif
	/* TODO: Add any other control path calls required to OL_IF/WMA layer */
};

#ifndef CONFIG_WIN
/* From here MCL specific OPs */
/**
 * struct cdp_misc_ops - mcl ops not classified
 * @set_ibss_vdev_heart_beat_timer:
 * @bad_peer_txctl_set_setting:
 * @bad_peer_txctl_update_threshold:
 * @hl_tdls_flag_reset:
 * @tx_non_std:
 * @get_vdev_id:
 * @set_wisa_mode:
 * @txrx_data_stall_cb_register:
 * @txrx_data_stall_cb_deregister:
 * @txrx_post_data_stall_event
 * @runtime_suspend:
 * @runtime_resume:
 */
struct cdp_misc_ops {
	uint16_t (*set_ibss_vdev_heart_beat_timer)(struct cdp_vdev *vdev,
			uint16_t timer_value_sec);
	void (*set_wmm_param)(struct cdp_pdev *cfg_pdev,
			struct ol_tx_wmm_param_t wmm_param);
	void (*bad_peer_txctl_set_setting)(struct cdp_pdev *pdev, int enable,
			int period, int txq_limit);
	void (*bad_peer_txctl_update_threshold)(struct cdp_pdev *pdev,
		int level, int tput_thresh, int tx_limit);
	void (*hl_tdls_flag_reset)(struct cdp_vdev *vdev, bool flag);
	qdf_nbuf_t (*tx_non_std)(struct cdp_vdev *vdev,
		enum ol_tx_spec tx_spec, qdf_nbuf_t msdu_list);
	uint16_t (*get_vdev_id)(struct cdp_vdev *vdev);
	QDF_STATUS (*set_wisa_mode)(struct cdp_vdev *vdev, bool enable);
	QDF_STATUS (*txrx_data_stall_cb_register)(data_stall_detect_cb cb);
	QDF_STATUS (*txrx_data_stall_cb_deregister)(data_stall_detect_cb cb);
	void (*txrx_post_data_stall_event)(
			enum data_stall_log_event_indicator indicator,
			enum data_stall_log_event_type data_stall_type,
			uint32_t pdev_id, uint32_t vdev_id_bitmap,
			enum data_stall_log_recovery_type recovery_type);
	QDF_STATUS (*runtime_suspend)(struct cdp_pdev *pdev);
	QDF_STATUS (*runtime_resume)(struct cdp_pdev *pdev);
	int (*get_opmode)(struct cdp_vdev *vdev);
	void (*mark_first_wakeup_packet)(uint8_t value);
	void (*update_mac_id)(uint8_t vdev_id, uint8_t mac_id);
	void (*flush_rx_frames)(void *peer, bool drop);
	A_STATUS (*get_intra_bss_fwd_pkts_count)(uint8_t vdev_id,
		uint64_t *fwd_tx_packets, uint64_t *fwd_rx_packets);
	void (*pkt_log_init)(struct cdp_pdev *handle, void *scn);
	void (*pkt_log_con_service)(struct cdp_pdev *pdev, void *scn);
};

/**
 * struct cdp_tx_delay_ops - mcl tx delay ops
 * @tx_delay:
 * @tx_delay_hist:
 * @tx_packet_count:
 * @tx_set_compute_interval:
 */
struct cdp_tx_delay_ops {
	void (*tx_delay)(struct cdp_pdev *pdev, uint32_t *queue_delay_microsec,
		uint32_t *tx_delay_microsec, int category);
	void (*tx_delay_hist)(struct cdp_pdev *pdev,
		uint16_t *bin_values, int category);
	void (*tx_packet_count)(struct cdp_pdev *pdev,
		uint16_t *out_packet_count,
		uint16_t *out_packet_loss_count, int category);
	void (*tx_set_compute_interval)(struct cdp_pdev *pdev,
		uint32_t interval);
};

/**
 * struct cdp_pmf_ops - mcl protected management frame ops
 * @get_pn_info:
 */
struct cdp_pmf_ops {
	void (*get_pn_info)(void *peer, uint8_t **last_pn_valid,
			uint64_t **last_pn, uint32_t **rmf_pn_replays);
};

/**
 * struct cdp_cfg_ops - mcl configuration ops
 * @set_cfg_rx_fwd_disabled:
 * @set_cfg_packet_log_enabled:
 * @cfg_attach:
 * @vdev_rx_set_intrabss_fwd:
 * @get_opmode:
 * @is_rx_fwd_disabled:
 * @tx_set_is_mgmt_over_wmi_enabled:
 * @is_high_latency:
 * @set_flow_control_parameters:
 */
struct cdp_cfg_ops {
	void (*set_cfg_rx_fwd_disabled)(struct cdp_cfg *cfg_pdev,
		uint8_t disable_rx_fwd);
	void (*set_cfg_packet_log_enabled)(struct cdp_cfg *cfg_pdev,
		uint8_t val);
	struct cdp_cfg * (*cfg_attach)(qdf_device_t osdev, void *cfg_param);
	void (*vdev_rx_set_intrabss_fwd)(struct cdp_vdev *vdev, bool val);
	uint8_t (*is_rx_fwd_disabled)(struct cdp_vdev *vdev);
	void (*tx_set_is_mgmt_over_wmi_enabled)(uint8_t value);
	int (*is_high_latency)(struct cdp_cfg *cfg_pdev);
	void (*set_flow_control_parameters)(struct cdp_cfg *cfg_pdev,
		void *param);
	void (*set_flow_steering)(struct cdp_cfg *cfg_pdev, uint8_t val);
	void (*set_ptp_rx_opt_enabled)(struct cdp_cfg *cfg_pdev, uint8_t val);
};

/**
 * struct cdp_flowctl_ops - mcl flow control
 * @register_pause_cb:
 * @set_desc_global_pool_size:
 * @dump_flow_pool_info:
 */
struct cdp_flowctl_ops {
	QDF_STATUS (*register_pause_cb)(struct cdp_soc_t *soc,
					tx_pause_callback);
	void (*set_desc_global_pool_size)(uint32_t num_msdu_desc);
	void (*dump_flow_pool_info)(void *);
};

/**
 * struct cdp_lflowctl_ops - mcl legacy flow control ops
 * @register_tx_flow_control:
 * @deregister_tx_flow_control_cb:
 * @flow_control_cb:
 * @get_tx_resource:
 * @ll_set_tx_pause_q_depth:
 * @vdev_flush:
 * @vdev_pause:
 * @vdev_unpause:
 */
struct cdp_lflowctl_ops {
	int (*register_tx_flow_control)(uint8_t vdev_id,
		ol_txrx_tx_flow_control_fp flowControl, void *osif_fc_ctx,
		ol_txrx_tx_flow_control_is_pause_fp flow_control_is_pause);
	int (*deregister_tx_flow_control_cb)(uint8_t vdev_id);
	void (*flow_control_cb)(struct cdp_vdev *vdev, bool tx_resume);
	bool (*get_tx_resource)(uint8_t sta_id,
			 unsigned int low_watermark,
			 unsigned int high_watermark_offset);
	int (*ll_set_tx_pause_q_depth)(uint8_t vdev_id, int pause_q_depth);
	void (*vdev_flush)(struct cdp_vdev *vdev);
	void (*vdev_pause)(struct cdp_vdev *vdev, uint32_t reason);
	void (*vdev_unpause)(struct cdp_vdev *vdev, uint32_t reason);
};

#ifdef IPA_OFFLOAD
/**
 * struct cdp_ipa_ops - mcl ipa data path ops
 * @ipa_get_resource:
 * @ipa_set_doorbell_paddr:
 * @ipa_set_active:
 * @ipa_op_response:
 * @ipa_register_op_cb:
 * @ipa_get_stat:
 * @ipa_tx_data_frame:
 */
struct cdp_ipa_ops {
	QDF_STATUS (*ipa_get_resource)(struct cdp_pdev *pdev);
	QDF_STATUS (*ipa_set_doorbell_paddr)(struct cdp_pdev *pdev);
	QDF_STATUS (*ipa_set_active)(struct cdp_pdev *pdev, bool uc_active,
		bool is_tx);
	QDF_STATUS (*ipa_op_response)(struct cdp_pdev *pdev, uint8_t *op_msg);
	QDF_STATUS (*ipa_register_op_cb)(struct cdp_pdev *pdev,
		void (*ipa_uc_op_cb_type)(uint8_t *op_msg, void *osif_ctxt),
		void *usr_ctxt);
	QDF_STATUS (*ipa_get_stat)(struct cdp_pdev *pdev);
	qdf_nbuf_t (*ipa_tx_data_frame)(struct cdp_vdev *vdev, qdf_nbuf_t skb);
	void (*ipa_set_uc_tx_partition_base)(struct cdp_cfg *pdev,
		uint32_t value);
#ifdef FEATURE_METERING
	QDF_STATUS (*ipa_uc_get_share_stats)(struct cdp_pdev *pdev,
		uint8_t reset_stats);
	QDF_STATUS (*ipa_uc_set_quota)(struct cdp_pdev *pdev,
		uint64_t quota_bytes);
#endif
	QDF_STATUS (*ipa_enable_autonomy)(struct cdp_pdev *pdev);
	QDF_STATUS (*ipa_disable_autonomy)(struct cdp_pdev *pdev);
#ifdef CONFIG_IPA_WDI_UNIFIED_API
	QDF_STATUS (*ipa_setup)(struct cdp_pdev *pdev, void *ipa_i2w_cb,
		void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
		uint32_t ipa_desc_size, void *ipa_priv, bool is_rm_enabled,
		uint32_t *tx_pipe_handle, uint32_t *rx_pipe_handle,
		bool is_smmu_enabled, qdf_ipa_sys_connect_params_t *sys_in);
#else /* CONFIG_IPA_WDI_UNIFIED_API */
	QDF_STATUS (*ipa_setup)(struct cdp_pdev *pdev, void *ipa_i2w_cb,
		void *ipa_w2i_cb, void *ipa_wdi_meter_notifier_cb,
		uint32_t ipa_desc_size, void *ipa_priv, bool is_rm_enabled,
		uint32_t *tx_pipe_handle, uint32_t *rx_pipe_handle);
#endif /* CONFIG_IPA_WDI_UNIFIED_API */
	QDF_STATUS (*ipa_cleanup)(uint32_t tx_pipe_handle,
		uint32_t rx_pipe_handle);
	QDF_STATUS (*ipa_setup_iface)(char *ifname, uint8_t *mac_addr,
		qdf_ipa_client_type_t prod_client,
		qdf_ipa_client_type_t cons_client,
		uint8_t session_id, bool is_ipv6_enabled);
	QDF_STATUS (*ipa_cleanup_iface)(char *ifname, bool is_ipv6_enabled);
	QDF_STATUS (*ipa_enable_pipes)(struct cdp_pdev *pdev);
	QDF_STATUS (*ipa_disable_pipes)(struct cdp_pdev *pdev);
	QDF_STATUS (*ipa_set_perf_level)(int client,
		uint32_t max_supported_bw_mbps);
};
#endif

/**
 * struct cdp_bus_ops - mcl bus suspend/resume ops
 * @bus_suspend:
 * @bus_resume:
 */
struct cdp_bus_ops {
	QDF_STATUS (*bus_suspend)(struct cdp_pdev *opaque_pdev);
	QDF_STATUS (*bus_resume)(struct cdp_pdev *opaque_pdev);
};

/**
 * struct cdp_ocb_ops - mcl ocb ops
 * @set_ocb_chan_info:
 * @get_ocb_chan_info:
 */
struct cdp_ocb_ops {
	void (*set_ocb_chan_info)(struct cdp_vdev *vdev,
			struct ol_txrx_ocb_set_chan ocb_set_chan);
	struct ol_txrx_ocb_chan_info *
		(*get_ocb_chan_info)(struct cdp_vdev *vdev);
};

/**
 * struct cdp_peer_ops - mcl peer related ops
 * @register_peer:
 * @clear_peer:
 * @cfg_attach:
 * @find_peer_by_addr:
 * @find_peer_by_addr_and_vdev:
 * @local_peer_id:
 * @peer_find_by_local_id:
 * @peer_state_update:
 * @get_vdevid:
 * @get_vdev_by_sta_id:
 * @register_ocb_peer:
 * @peer_get_peer_mac_addr:
 * @get_peer_state:
 * @get_vdev_for_peer:
 * @update_ibss_add_peer_num_of_vdev:
 * @remove_peers_for_vdev:
 * @remove_peers_for_vdev_no_lock:
 * @copy_mac_addr_raw:
 * @add_last_real_peer:
 * @last_assoc_received:
 * @last_disassoc_received:
 * @last_deauth_received:
 * @is_vdev_restore_last_peer:
 * @update_last_real_peer:
 */
struct cdp_peer_ops {
	QDF_STATUS (*register_peer)(struct cdp_pdev *pdev,
			struct ol_txrx_desc_type *sta_desc);
	QDF_STATUS (*clear_peer)(struct cdp_pdev *pdev, uint8_t sta_id);
	QDF_STATUS (*change_peer_state)(uint8_t sta_id,
			enum ol_txrx_peer_state sta_state,
			bool roam_synch_in_progress);
	void * (*peer_get_ref_by_addr)(struct cdp_pdev *pdev,
				       u8 *peer_addr, uint8_t *peer_id,
				       enum peer_debug_id_type debug_id);
	void (*peer_release_ref)(void *peer, enum peer_debug_id_type debug_id);
	void * (*find_peer_by_addr)(struct cdp_pdev *pdev,
			uint8_t *peer_addr, uint8_t *peer_id);
	void * (*find_peer_by_addr_and_vdev)(struct cdp_pdev *pdev,
			struct cdp_vdev *vdev,
			uint8_t *peer_addr, uint8_t *peer_id);
	uint16_t (*local_peer_id)(void *peer);
	void * (*peer_find_by_local_id)(struct cdp_pdev *pdev,
			uint8_t local_peer_id);
	QDF_STATUS (*peer_state_update)(struct cdp_pdev *pdev,
			uint8_t *peer_addr,
			enum ol_txrx_peer_state state);
	QDF_STATUS (*get_vdevid)(void *peer, uint8_t *vdev_id);
	struct cdp_vdev * (*get_vdev_by_sta_id)(struct cdp_pdev *pdev,
			uint8_t sta_id);
	QDF_STATUS (*register_ocb_peer)(uint8_t *mac_addr, uint8_t *peer_id);
	uint8_t * (*peer_get_peer_mac_addr)(void *peer);
	int (*get_peer_state)(void *peer);
	struct cdp_vdev * (*get_vdev_for_peer)(void *peer);
	int16_t (*update_ibss_add_peer_num_of_vdev)(struct cdp_vdev *vdev,
			int16_t peer_num_delta);
	void (*remove_peers_for_vdev)(struct cdp_vdev *vdev,
			ol_txrx_vdev_peer_remove_cb callback,
			void *callback_context, bool remove_last_peer);
	void (*remove_peers_for_vdev_no_lock)(struct cdp_vdev *vdev,
			ol_txrx_vdev_peer_remove_cb callback,
			void *callback_context);
	void (*copy_mac_addr_raw)(struct cdp_vdev *vdev, uint8_t *bss_addr);
	void (*add_last_real_peer)(struct cdp_pdev *pdev,
		struct cdp_vdev *vdev, uint8_t *peer_id);
	qdf_time_t * (*last_assoc_received)(void *peer);
	qdf_time_t * (*last_disassoc_received)(void *peer);
	qdf_time_t * (*last_deauth_received)(void *peer);
	bool (*is_vdev_restore_last_peer)(void *peer);
	void (*update_last_real_peer)(struct cdp_pdev *pdev, void *peer,
			uint8_t *peer_id, bool restore_last_peer);
	void (*peer_detach_force_delete)(void *peer);
};

/**
 * struct cdp_ocb_ops - mcl ocb ops
 * @throttle_init_period:
 * @throttle_set_level:
 */
struct cdp_throttle_ops {
	void (*throttle_init_period)(struct cdp_pdev *pdev, int period,
			uint8_t *dutycycle_level);
	void (*throttle_set_level)(struct cdp_pdev *pdev, int level);
};

/**
 * struct cdp_ocb_ops - mcl ocb ops
 * @clear_stats:
 * @stats:
 */
struct cdp_mob_stats_ops {
	void (*clear_stats)(uint16_t bitmap);
	int (*stats)(uint8_t vdev_id, char *buffer, unsigned buf_len);
};
#endif /* CONFIG_WIN */

struct cdp_ops {
	struct cdp_cmn_ops          *cmn_drv_ops;
	struct cdp_ctrl_ops         *ctrl_ops;
	struct cdp_me_ops           *me_ops;
	struct cdp_mon_ops          *mon_ops;
	struct cdp_host_stats_ops   *host_stats_ops;
	struct cdp_wds_ops          *wds_ops;
	struct cdp_raw_ops          *raw_ops;
	struct cdp_pflow_ops        *pflow_ops;
#ifndef CONFIG_WIN
	struct cdp_misc_ops         *misc_ops;
	struct cdp_cfg_ops          *cfg_ops;
	struct cdp_flowctl_ops      *flowctl_ops;
	struct cdp_lflowctl_ops     *l_flowctl_ops;
#ifdef IPA_OFFLOAD
	struct cdp_ipa_ops          *ipa_ops;
#endif
	struct cdp_bus_ops          *bus_ops;
	struct cdp_ocb_ops          *ocb_ops;
	struct cdp_peer_ops         *peer_ops;
	struct cdp_throttle_ops     *throttle_ops;
	struct cdp_mob_stats_ops    *mob_stats_ops;
	struct cdp_tx_delay_ops     *delay_ops;
	struct cdp_pmf_ops          *pmf_ops;
#endif /* CONFIG_WIN */
};

#endif
