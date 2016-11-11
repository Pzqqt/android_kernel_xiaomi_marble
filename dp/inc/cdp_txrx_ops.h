/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
#ifdef CONFIG_WIN
#include <cdp_txrx_stats_struct.h>
#endif

/******************************************************************************
 *
 * Control Interface (A Interface)
 *
 *****************************************************************************/

struct cdp_cmn_ops {

	int(*txrx_soc_attach_target)(ol_txrx_soc_handle soc);

	int(*txrx_pdev_attach_target)(void *pdev);

	void *(*txrx_vdev_attach)
		(void *pdev, uint8_t *vdev_mac_addr,
		 uint8_t vdev_id, enum wlan_op_mode op_mode);

	void(*txrx_vdev_detach)
		(void *vdev, ol_txrx_vdev_delete_cb callback,
		 void *cb_context);

	void *(*txrx_pdev_attach)
		(ol_txrx_soc_handle soc, void *ctrl_pdev,
		HTC_HANDLE htc_pdev, qdf_device_t osdev, uint8_t pdev_id);

	int (*txrx_pdev_post_attach)(void *pdev);

	void(*txrx_pdev_detach)(void *pdev, int force);

	void *(*txrx_peer_create)
		(void *vdev, uint8_t *peer_mac_addr);

	void (*txrx_peer_setup)
		(void *vdev_hdl, void *peer_hdl);

	void (*txrx_peer_teardown)
		(void *vdev_hdl, void *peer_hdl);

	void(*txrx_peer_delete)(void *peer);

	int(*txrx_set_monitor_mode)(void *vdev);

	void(*txrx_set_curchan)(void *pdev, uint32_t chan_mhz);

	void (*txrx_set_privacy_filters)
		(void *vdev, void *filter, uint32_t num);

	/********************************************************************
	 * Data Interface (B Interface)
	 ********************************************************************/

	void(*txrx_vdev_register)(void *vdev,
			void *osif_vdev, struct ol_txrx_ops *txrx_ops);

	int(*txrx_mgmt_send)(void *vdev,
			qdf_nbuf_t tx_mgmt_frm, uint8_t type);

	int(*txrx_mgmt_send_ext)(void *vdev,
			qdf_nbuf_t tx_mgmt_frm,	uint8_t type, uint8_t use_6mbps,
			uint16_t chanfreq);

	/**
	 * ol_txrx_mgmt_tx_cb - tx management delivery notification
	 * callback function
	 */

	void(*txrx_mgmt_tx_cb_set)
		(void *pdev, uint8_t type,
		 ol_txrx_mgmt_tx_cb download_cb, ol_txrx_mgmt_tx_cb ota_ack_cb,
		 void *ctxt);

	int (*txrx_get_tx_pending)(void *pdev);

	/**
	 * ol_txrx_data_tx_cb - Function registered with the data path
	 * that is called when tx frames marked as "no free" are
	 * done being transmitted
	 */

	void(*txrx_data_tx_cb_set)(void *data_vdev,
			ol_txrx_data_tx_cb callback, void *ctxt);

	/*******************************************************************
	 * Statistics and Debugging Interface (C Inteface)
	 ********************************************************************/

	int(*txrx_aggr_cfg)(void *vdev, int max_subfrms_ampdu,
			int max_subfrms_amsdu);

	A_STATUS(*txrx_fw_stats_get)(void *vdev, struct ol_txrx_stats_req *req,
			bool per_vdev, bool response_expected);

	int(*txrx_debug)(void *vdev, int debug_specs);

	void(*txrx_fw_stats_cfg)(void *vdev,
			uint8_t cfg_stats_type, uint32_t cfg_val);

	void(*txrx_print_level_set)(unsigned level);

	/**
	 * ol_txrx_get_vdev_mac_addr() - Return mac addr of vdev
	 * @vdev: vdev handle
	 *
	 * Return: vdev mac address
	 */
	uint8_t *(*txrx_get_vdev_mac_addr)(void *vdev);

	/**
	 * ol_txrx_get_vdev_struct_mac_addr() - Return handle to struct qdf_mac_addr of
	 * vdev
	 * @vdev: vdev handle
	 *
	 * Return: Handle to struct qdf_mac_addr
	 */
	struct qdf_mac_addr *
		(*txrx_get_vdev_struct_mac_addr)(void *vdev);

	/**
	 * ol_txrx_get_pdev_from_vdev() - Return handle to pdev of vdev
	 * @vdev: vdev handle
	 *
	 * Return: Handle to pdev
	 */
	void *(*txrx_get_pdev_from_vdev)
		(void *vdev);

	/**
	 * ol_txrx_get_ctrl_pdev_from_vdev() - Return control pdev of vdev
	 * @vdev: vdev handle
	 *
	 * Return: Handle to control pdev
	 */
	void *
		(*txrx_get_ctrl_pdev_from_vdev)(void *vdev);

	void *
		(*txrx_get_vdev_from_vdev_id)(void *pdev, uint8_t vdev_id);

	void (*txrx_soc_detach)(void *soc);

};

struct cdp_ctrl_ops {

	int
		(*txrx_mempools_attach)(void *ctrl_pdev);
	int
		(*txrx_set_filter_neighbour_peers)(
				void *pdev,
				u_int32_t val);
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
				void *vdev,
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
				void *vdev,
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
				void *vdev,
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
				void *vdev,
				enum htt_cmn_pkt_type val);

	/**
	 * @brief get the Rx decapsulation type of the VDEV
	 *
	 * @param vdev - the data virtual device object
	 * @return - the Rx decap type
	 */
	enum htt_pkt_type
		(*txrx_get_vdev_rx_decap_type)(void *vdev);

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
		(*txrx_peer_authorize)(void *peer,
				u_int32_t authorize);

	bool
		(*txrx_set_inact_params)(void *pdev,
				u_int16_t inact_check_interval,
				u_int16_t inact_normal,
				u_int16_t inact_overload);
	bool
		(*txrx_start_inact_timer)(
				void *pdev,
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
			void *pdev,
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
	void (*txrx_set_mesh_mode)(void *vdev, u_int32_t val);

	void (*tx_flush_buffers)(void *vdev);

	int (*txrx_is_target_ar900b)(void *vdev);

};

struct cdp_me_ops {

	u_int16_t (*tx_desc_alloc_and_mark_for_mcast_clone)
		(void *pdev, u_int16_t buf_count);

		u_int16_t (*tx_desc_free_and_unmark_for_mcast_clone)(
				void *pdev,
				u_int16_t buf_count);

	u_int16_t
		(*tx_get_mcast_buf_allocated_marked)
			(void *pdev);
	void
		(*tx_me_alloc_descriptor)(void *pdev);

	void
		(*tx_me_free_descriptor)(void *pdev);

	uint16_t
		(*tx_me_convert_ucast)(void *vdev,
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

	int (*mcast_notify)(void *pdev,
			u_int8_t vdev_id, qdf_nbuf_t msdu);
};

struct cdp_mon_ops {

	void (*txrx_monitor_set_filter_ucast_data)
		(void *, u_int8_t val);
	void (*txrx_monitor_set_filter_mcast_data)
		(void *, u_int8_t val);
	void (*txrx_monitor_set_filter_non_data)
		(void *, u_int8_t val);

	u_int8_t (*txrx_monitor_get_filter_ucast_data)
		(void *vdev_txrx_handle);
	u_int8_t (*txrx_monitor_get_filter_mcast_data)
		(void *vdev_txrx_handle);
	u_int8_t (*txrx_monitor_get_filter_non_data)
		(void *vdev_txrx_handle);
	int (*txrx_reset_monitor_mode)(void *pdev);

};

struct cdp_host_stats_ops {
	int (*txrx_host_stats_get)(void *vdev,
			struct ol_txrx_stats_req *req);

	void (*txrx_host_stats_clr)(void *vdev);

	void (*txrx_host_ce_stats)(void *vdev);

	int (*txrx_stats_publish)(void *pdev,
			void *buf);
	/**
	 * @brief Enable enhanced stats functionality.
	 *
	 * @param pdev - the physical device object
	 * @return - void
	 */
	void (*txrx_enable_enhanced_stats)(void *pdev);

	/**
	 * @brief Disable enhanced stats functionality.
	 *
	 * @param pdev - the physical device object
	 * @return - void
	 */
	void (*txrx_disable_enhanced_stats)(void *pdev);

	/**
	 * @brief Get the desired stats from the message.
	 *
	 * @param pdev - the physical device object
	 * @param stats_base - stats buffer recieved from FW
	 * @param type - stats type.
	 * @return - pointer to requested stat identified by type
	 */
	uint32_t*(*txrx_get_stats_base)(void *pdev,
			uint32_t *stats_base, uint32_t msg_len, uint8_t type);
	void
		(*tx_print_tso_stats)(void *vdev);

	void
		(*tx_rst_tso_stats)(void *vdev);

	void
		(*tx_print_sg_stats)(void *vdev);

	void
		(*tx_rst_sg_stats)(void *vdev);

	void
		(*print_rx_cksum_stats)(void *vdev);

	void
		(*rst_rx_cksum_stats)(void *vdev);

	A_STATUS
		(*txrx_host_me_stats)(void *vdev);
	void
		(*txrx_per_peer_stats)(void *pdev, char *addr);
	int (*txrx_host_msdu_ttl_stats)(void *vdev,
			struct ol_txrx_stats_req *req);

	void
		(*print_lro_stats)(void *vdev);

	void
		(*reset_lro_stats)(void *vdev);

};

struct cdp_wds_ops {
	void
		(*txrx_set_wds_rx_policy)(void *vdev,
				u_int32_t val);

};

struct cdp_raw_ops {
	int (*txrx_get_nwifi_mode)(void *vdev);

	int
		(*rsim_tx_encap)(void *vdev, qdf_nbuf_t *pnbuf);
};

#ifdef CONFIG_WIN
struct cdp_pflow_ops {
	uint32_t(*pflow_update_pdev_params)(void *,
			ol_ath_param_t, uint32_t, void *);
};
#endif /* CONFIG_WIN */

struct ol_if_ops {
	void (*peer_set_default_routing)(void *scn_handle,
			uint8_t *peer_macaddr, uint8_t vdev_id,
			bool hash_based, uint8_t ring_num);
	int (*peer_rx_reorder_queue_setup)(void *ol_soc_handle,
			uint8_t vdev_id, uint8_t *peer_mac,
			qdf_dma_addr_t hw_qdesc, int tid, uint16_t queue_num);
	int (*peer_rx_reorder_queue_remove)(void *ol_soc_handle,
			uint8_t vdev_id, uint8_t *peer_macaddr,
			uint32_t tid_mask);
	int (*peer_unref_delete)(void *ol_soc_handle);

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
 * @runtime_suspend:
 * @runtime_resume:
 */
struct cdp_misc_ops {
	uint16_t (*set_ibss_vdev_heart_beat_timer)(void *vdev,
			uint16_t timer_value_sec);
	void (*set_wmm_param)(void *cfg_pdev,
			struct ol_tx_wmm_param_t wmm_param);
	void (*bad_peer_txctl_set_setting)(void *pdev, int enable,
			int period, int txq_limit);
	void (*bad_peer_txctl_update_threshold)(void *pdev,
		int level, int tput_thresh, int tx_limit);
	void (*hl_tdls_flag_reset)(void *vdev, bool flag);
	qdf_nbuf_t (*tx_non_std)(void *vdev,
		enum ol_tx_spec tx_spec, qdf_nbuf_t msdu_list);
	uint16_t (*get_vdev_id)(void *vdev);
	QDF_STATUS (*set_wisa_mode)(void *vdev, bool enable);
	QDF_STATUS (*runtime_suspend)(void *pdev);
	QDF_STATUS (*runtime_resume)(void *pdev);
	int (*get_opmode)(void *vdev);
	void (*mark_first_wakeup_packet)(uint8_t value);
	void (*update_mac_id)(uint8_t vdev_id, uint8_t mac_id);
	void (*flush_rx_frames)(void *peer, bool drop);
	A_STATUS (*get_intra_bss_fwd_pkts_count)(uint8_t vdev_id,
		uint64_t *fwd_tx_packets, uint64_t *fwd_rx_packets);
	void (*pkt_log_init)(void *handle, void *scn);
	void (*pkt_log_con_service)(void *pdev, void *scn);
};

/**
 * struct cdp_tx_delay_ops - mcl tx delay ops
 * @tx_delay:
 * @tx_delay_hist:
 * @tx_packet_count:
 * @tx_set_compute_interval:
 */
struct cdp_tx_delay_ops {
	void (*tx_delay)(void *pdev, uint32_t *queue_delay_microsec,
		uint32_t *tx_delay_microsec, int category);
	void (*tx_delay_hist)(void *pdev,
		uint16_t *bin_values, int category);
	void (*tx_packet_count)(void *pdev, uint16_t *out_packet_count,
		uint16_t *out_packet_loss_count, int category);
	void (*tx_set_compute_interval)(void *pdev, uint32_t interval);
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
	void (*set_cfg_rx_fwd_disabled)(void *pdev, uint8_t disable_rx_fwd);
	void (*set_cfg_packet_log_enabled)(void *pdev, uint8_t val);
	void * (*cfg_attach)(qdf_device_t osdev, void *cfg_param);
	void (*vdev_rx_set_intrabss_fwd)(void *vdev, bool val);
	uint8_t (*is_rx_fwd_disabled)(void *vdev);
	void (*tx_set_is_mgmt_over_wmi_enabled)(uint8_t value);
	int (*is_high_latency)(void *pdev);
	void (*set_flow_control_parameters)(void *cfg, void *param);
	void (*set_flow_steering)(void *pdev, uint8_t val);
};

/**
 * struct cdp_flowctl_ops - mcl flow control
 * @register_pause_cb:
 * @set_desc_global_pool_size:
 * @dump_flow_pool_info:
 */
struct cdp_flowctl_ops {
	QDF_STATUS (*register_pause_cb)(ol_tx_pause_callback_fp);

	void (*set_desc_global_pool_size)(uint32_t num_msdu_desc);
	void (*dump_flow_pool_info)(void);
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
		ol_txrx_tx_flow_control_fp flowControl, void *osif_fc_ctx);
	int (*deregister_tx_flow_control_cb)(uint8_t vdev_id);
	void (*flow_control_cb)(void *vdev, bool tx_resume);
	bool (*get_tx_resource)(uint8_t sta_id,
			 unsigned int low_watermark,
			 unsigned int high_watermark_offset);
	int (*ll_set_tx_pause_q_depth)(uint8_t vdev_id, int pause_q_depth);
	void (*vdev_flush)(void *vdev);
	void (*vdev_pause)(void *vdev, uint32_t reason);
	void (*vdev_unpause)(void *vdev, uint32_t reason);
};

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
	void (*ipa_get_resource)(void *pdev,
			struct ol_txrx_ipa_resources *ipa_res);
	void (*ipa_set_doorbell_paddr)(void *pdev,
		 qdf_dma_addr_t ipa_tx_uc_doorbell_paddr,
		 qdf_dma_addr_t ipa_rx_uc_doorbell_paddr);
	void (*ipa_set_active)(void *pdev, bool uc_active, bool is_tx);
	void (*ipa_op_response)(void *pdev, uint8_t *op_msg);
	void (*ipa_register_op_cb)(void *pdev,
		 void (*ipa_uc_op_cb_type)(uint8_t *op_msg, void *osif_ctxt),
		 void *osif_dev);
	void (*ipa_get_stat)(void *pdev);
	qdf_nbuf_t (*ipa_tx_data_frame)(void *vdev, qdf_nbuf_t skb);
	void (*ipa_set_uc_tx_partition_base)(void *pdev, uint32_t value);
};

/**
 * struct cdp_lro_ops - mcl large receive offload ops
 * @register_lro_flush_cb:
 * @deregister_lro_flush_cb:
 */
struct cdp_lro_ops {
	void (*register_lro_flush_cb)(void (lro_flush_cb)(void *),
			void *(lro_init_cb)(void));
	void (*deregister_lro_flush_cb)(void (lro_deinit_cb)(void *));
};

/**
 * struct cdp_bus_ops - mcl bus suspend/resume ops
 * @bus_suspend:
 * @bus_resume:
 */
struct cdp_bus_ops {
	QDF_STATUS (*bus_suspend)(void);
	QDF_STATUS (*bus_resume)(void);
};

/**
 * struct cdp_ocb_ops - mcl ocb ops
 * @set_ocb_chan_info:
 * @get_ocb_chan_info:
 */
struct cdp_ocb_ops {
	void (*set_ocb_chan_info)(void *vdev,
			  struct ol_txrx_ocb_set_chan ocb_set_chan);
	struct ol_txrx_ocb_chan_info * (*get_ocb_chan_info)(void *vdev);
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
	QDF_STATUS (*register_peer)(void *pdev,
			struct ol_txrx_desc_type *sta_desc);
	QDF_STATUS (*clear_peer)(void *pdev, uint8_t sta_id);
	QDF_STATUS (*change_peer_state)(uint8_t sta_id,
			enum ol_txrx_peer_state sta_state,
			bool roam_synch_in_progress);
	void * (*find_peer_by_addr)(void *pdev,
			uint8_t *peer_addr, uint8_t *peer_id);
	void * (*find_peer_by_addr_and_vdev)(void *pdev, void *vdev,
			uint8_t *peer_addr, uint8_t *peer_id);
	uint16_t (*local_peer_id)(void *peer);
	void * (*peer_find_by_local_id)(void *pdev, uint8_t local_peer_id);
	QDF_STATUS (*peer_state_update)(void *pdev, uint8_t *peer_addr,
			enum ol_txrx_peer_state state);
	QDF_STATUS (*get_vdevid)(void *peer, uint8_t *vdev_id);
	void * (*get_vdev_by_sta_id)(uint8_t sta_id);
	QDF_STATUS (*register_ocb_peer)(void *cds_ctx, uint8_t *mac_addr,
			uint8_t *peer_id);
	uint8_t * (*peer_get_peer_mac_addr)(void *peer);
	int (*get_peer_state)(void *peer);
	void * (*get_vdev_for_peer)(void *peer);
	int16_t (*update_ibss_add_peer_num_of_vdev)(void *vdev,
			int16_t peer_num_delta);
	void (*remove_peers_for_vdev)(void *vdev,
			ol_txrx_vdev_peer_remove_cb callback,
			void *callback_context, bool remove_last_peer);
	void (*remove_peers_for_vdev_no_lock)(void *vdev,
			ol_txrx_vdev_peer_remove_cb callback,
			void *callback_context);
	void (*copy_mac_addr_raw)(void *vdev, uint8_t *bss_addr);
	void (*add_last_real_peer)(void *pdev, void *vdev, uint8_t *peer_id);
	qdf_time_t * (*last_assoc_received)(void *peer);
	qdf_time_t * (*last_disassoc_received)(void *peer);
	qdf_time_t * (*last_deauth_received)(void *peer);
	bool (*is_vdev_restore_last_peer)(void *peer);
	void (*update_last_real_peer)(void *pdev, void *peer,
			uint8_t *peer_id, bool restore_last_peer);
	void (*peer_detach_force_delete)(void *peer);
};

/**
 * struct cdp_ocb_ops - mcl ocb ops
 * @throttle_init_period:
 * @throttle_set_level:
 */
struct cdp_throttle_ops {
	void (*throttle_init_period)(void *pdev, int period,
			uint8_t *dutycycle_level);
	void (*throttle_set_level)(void *pdev, int level);
};

/**
 * struct cdp_ocb_ops - mcl ocb ops
 * @display_stats:
 * @clear_stats:
 * @stats:
 */
struct cdp_mob_stats_ops {
	void (*display_stats)(uint16_t bitmap);
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
	struct cdp_ipa_ops          *ipa_ops;
	struct cdp_lro_ops          *lro_ops;
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
