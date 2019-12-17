/*
 * Copyright (c) 2017-2020, The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#ifndef _QDF_IPA_H
#define _QDF_IPA_H

#ifdef IPA_OFFLOAD

#include <i_qdf_ipa.h>

/**
 * enum qdf_ipa_wlan_event - QDF IPA events
 * @QDF_IPA_CLIENT_CONNECT: Client Connects
 * @QDF_IPA_CLIENT_DISCONNECT: Client Disconnects
 * @QDF_IPA_AP_CONNECT: SoftAP is started
 * @QDF_IPA_AP_DISCONNECT: SoftAP is stopped
 * @QDF_IPA_STA_CONNECT: STA associates to AP
 * @QDF_IPA_STA_DISCONNECT: STA dissociates from AP
 * @QDF_IPA_CLIENT_CONNECT_EX: Peer associates/re-associates to softap
 * @QDF_SWITCH_TO_SCC: WLAN interfaces in scc mode
 * @QDF_SWITCH_TO_MCC: WLAN interfaces in mcc mode
 * @QDF_WDI_ENABLE: WDI enable complete
 * @QDF_WDI_DISABLE: WDI teardown
 * @QDF_FWR_SSR_BEFORE_SHUTDOWN: WLAN FW recovery
 * @QDF_IPA_WLAN_EVENT_MAX: Max value for the enum
 */
typedef enum {
	QDF_IPA_CLIENT_CONNECT,
	QDF_IPA_CLIENT_DISCONNECT,
	QDF_IPA_AP_CONNECT,
	QDF_IPA_AP_DISCONNECT,
	QDF_IPA_STA_CONNECT,
	QDF_IPA_STA_DISCONNECT,
	QDF_IPA_CLIENT_CONNECT_EX,
	QDF_SWITCH_TO_SCC,
	QDF_SWITCH_TO_MCC,
	QDF_WDI_ENABLE,
	QDF_WDI_DISABLE,
	QDF_FWR_SSR_BEFORE_SHUTDOWN,
	QDF_IPA_WLAN_EVENT_MAX
} qdf_ipa_wlan_event;

/**
 * qdf_ipa_wdi_meter_evt_type_t - type of event client callback is
 * for AP+STA mode metering
 * @IPA_GET_WDI_SAP_STATS: get IPA_stats betwen SAP and STA -
 *			use ipa_get_wdi_sap_stats structure
 * @IPA_SET_WIFI_QUOTA: set quota limit on STA -
 *			use ipa_set_wifi_quota structure
 */
typedef __qdf_ipa_wdi_meter_evt_type_t qdf_ipa_wdi_meter_evt_type_t;

typedef __qdf_ipa_get_wdi_sap_stats_t qdf_ipa_get_wdi_sap_stats_t;

/**
 * qdf_ipa_set_wifi_quota_t - structure used for
 *                                   IPA_SET_WIFI_QUOTA.
 */
typedef __qdf_ipa_set_wifi_quota_t qdf_ipa_set_wifi_quota_t;

/**
 * qdf_ipa_connect_params_t - low-level client connect input parameters. Either
 * client allocates the data and desc FIFO and specifies that in data+desc OR
 * specifies sizes and pipe_mem pref and IPA does the allocation.
 */
typedef __qdf_ipa_connect_params_t qdf_ipa_connect_params_t;

/**
 * qdf_ipa_tx_meta_t - meta-data for the TX packet
 */
typedef __qdf_ipa_tx_meta_t qdf_ipa_tx_meta_t;

/**
 *  __qdf_ipa_sps_params_t - SPS related output parameters resulting from
 */
typedef __qdf_ipa_sps_params_t qdf_ipa_sps_params_t;

/**
 * qdf_ipa_tx_intf_t - interface tx properties
 */
typedef __qdf_ipa_tx_intf_t qdf_ipa_tx_intf_t;

/**
 * qdf_ipa_rx_intf_t - interface rx properties
 */
typedef __qdf_ipa_rx_intf_t qdf_ipa_rx_intf_t;

/**
 * qdf_ipa_ext_intf_t - interface ext properties
 */
typedef __qdf_ipa_ext_intf_t qdf_ipa_ext_intf_t;

/**
 * qdf_ipa_sys_connect_params_t - information needed to setup an IPA end-point
 * in system-BAM mode
 */
typedef __qdf_ipa_sys_connect_params_t qdf_ipa_sys_connect_params_t;

/**
 * __qdf_pa_rm_event_t - IPA RM events
 *
 * Indicate the resource state change
 */
typedef __qdf_ipa_rm_event_t qdf_ipa_rm_event_t;

/**
 * struct qdf_ipa_rm_register_params_t - information needed to
 *      register IPA RM client with IPA RM
 */
typedef __qdf_ipa_rm_register_params_t qdf_ipa_rm_register_params_t;

/**
 * struct qdf_ipa_rm_create_params_t - information needed to initialize
 *				the resource
 *
 * IPA RM client is expected to perform non blocking operations only
 * in request_resource and release_resource functions and
 * release notification context as soon as possible.
 */
typedef __qdf_ipa_rm_create_params_t qdf_ipa_rm_create_params_t;

/**
 * qdf_ipa_rm_perf_profile_t - information regarding IPA RM client performance
 * profile
 */
typedef __qdf_ipa_rm_perf_profile_t qdf_ipa_rm_perf_profile_t;

/**
 * qdf_ipa_tx_data_desc_t - information needed
 * to send data packet to HW link: link to data descriptors
 * priv: client specific private data
 */
typedef __qdf_ipa_tx_data_desc_t qdf_ipa_tx_data_desc_t;

/**
 * qdf_ipa_rx_data_t - information needed
 * to send to wlan driver on receiving data from ipa hw
 */
typedef __qdf_ipa_rx_data_t qdf_ipa_rx_data_t;

/**
 * qdf_ipa_wdi_ul_params_t - WDI_RX configuration
 */
typedef __qdf_ipa_wdi_ul_params_t qdf_ipa_wdi_ul_params_t;

/**
 * qdf_ipa_wdi_ul_params_smmu_t - WDI_RX configuration (with WLAN SMMU)
 */
typedef __qdf_ipa_wdi_ul_params_smmu_t qdf_ipa_wdi_ul_params_smmu_t;

/**
 * qdf_ipa_wdi_dl_params_t - WDI_TX configuration
 */
typedef __qdf_ipa_wdi_dl_params_t qdf_ipa_wdi_dl_params_t;

/**
 * qdf_ipa_wdi_dl_params_smmu_t - WDI_TX configuration (with WLAN SMMU)
 */
typedef __qdf_ipa_wdi_dl_params_smmu_t qdf_ipa_wdi_dl_params_smmu_t;

/**
 * qdf_ipa_wdi_in_params_t - information provided by WDI client
 */
typedef __qdf_ipa_wdi_in_params_t qdf_ipa_wdi_in_params_t;

/**
 * qdf_ipa_wdi_out_params_t - information provided to WDI client
 */
typedef __qdf_ipa_wdi_out_params_t qdf_ipa_wdi_out_params_t;

/**
 * qdf_ipa_wdi_db_params_t - information provided to retrieve
 *       physical address of uC doorbell
 */
typedef __qdf_ipa_wdi_db_params_t qdf_ipa_wdi_db_params_t;

/**
 * qdf_ipa_wdi_uc_ready_params_t - uC ready CB parameters
 */
typedef void (*qdf_ipa_uc_ready_cb)(void *priv);
typedef __qdf_ipa_wdi_uc_ready_params_t qdf_ipa_wdi_uc_ready_params_t;

/**
 * qdf_ipa_wdi_buffer_info_t - address info of a WLAN allocated buffer
 *
 * IPA driver will create/release IOMMU mapping in IPA SMMU from iova->pa
 */
typedef __qdf_ipa_wdi_buffer_info_t qdf_ipa_wdi_buffer_info_t;

/**
 * qdf_ipa_gsi_ep_config_t - IPA GSI endpoint configurations
 */
typedef __qdf_ipa_gsi_ep_config_t qdf_ipa_gsi_ep_config_t;

/**
 * qdf_ipa_dp_evt_type_t - type of event client callback is
 * invoked for on data path
 * @IPA_RECEIVE: data is struct sk_buff
 * @IPA_WRITE_DONE: data is struct sk_buff
 */
typedef __qdf_ipa_dp_evt_type_t qdf_ipa_dp_evt_type_t;

#ifdef WDI3_STATS_UPDATE
/**
 * qdf_ipa_wdi_tx_info_t - WLAN embedded TX bytes information
 *
 * WLAN host fills this structure to update IPA driver about
 * embedded TX information.
 */
typedef __qdf_ipa_wdi_tx_info_t qdf_ipa_wdi_tx_info_t;

/**
 * qdf_ipa_wdi_bw_info_t - BW threshold levels to be monitored
 * by IPA uC
 */
typedef __qdf_ipa_wdi_bw_info_t qdf_ipa_wdi_bw_info_t;

/**
 * qdf_ipa_inform_wlan_bw_t - BW information given by IPA driver
 * whenever uC detects threshold level reached
 */
typedef __qdf_ipa_inform_wlan_bw_t qdf_ipa_inform_wlan_bw_t;
#endif

typedef __qdf_ipa_hdr_add_t qdf_ipa_hdr_add_t;
typedef __qdf_ipa_hdr_del_t qdf_ipa_hdr_del_t;
typedef __qdf_ipa_ioc_add_hdr_t qdf_ipa_ioc_add_hdr_t;
typedef __qdf_ipa_ioc_del_hdr_t qdf_ipa_ioc_del_hdr_t;
typedef __qdf_ipa_ioc_get_hdr_t qdf_ipa_ioc_get_hdr_t;
typedef __qdf_ipa_ioc_copy_hdr_t qdf_ipa_ioc_copy_hdr_t;
typedef __qdf_ipa_ioc_add_hdr_proc_ctx_t qdf_ipa_ioc_add_hdr_proc_ctx_t;
typedef __qdf_ipa_ioc_del_hdr_proc_ctx_t qdf_ipa_ioc_del_hdr_proc_ctx_t;
typedef __qdf_ipa_msg_meta_t qdf_ipa_msg_meta_t;
typedef __qdf_ipa_client_type_t qdf_ipa_client_type_t;
typedef __qdf_ipa_hw_stats_wdi_info_data_t qdf_ipa_hw_stats_wdi_info_data_t;
typedef __qdf_ipa_rm_resource_name_t  qdf_ipa_rm_resource_name_t;
typedef __qdf_ipa_wlan_event_t qdf_ipa_wlan_event_t;
typedef __qdf_ipa_wlan_msg_t qdf_ipa_wlan_msg_t;
typedef __qdf_ipa_wlan_msg_ex_t qdf_ipa_wlan_msg_ex_t;
typedef __qdf_ipa_ioc_tx_intf_prop_t qdf_ipa_ioc_tx_intf_prop_t;
typedef __qdf_ipa_ioc_rx_intf_prop_t qdf_ipa_ioc_rx_intf_prop_t;
typedef __qdf_ipa_wlan_hdr_attrib_val_t qdf_ipa_wlan_hdr_attrib_val_t;
typedef int (*qdf_ipa_msg_pull_fn)(void *buff, u32 len, u32 type);
typedef void (*qdf_ipa_ready_cb)(void *user_data);

#define QDF_IPA_SET_META_MSG_TYPE(meta, msg_type) \
	__QDF_IPA_SET_META_MSG_TYPE(meta, msg_type)

#define QDF_IPA_RM_RESOURCE_GRANTED __QDF_IPA_RM_RESOURCE_GRANTED
#define QDF_IPA_RM_RESOURCE_RELEASED __QDF_IPA_RM_RESOURCE_RELEASED

#define QDF_IPA_VOLTAGE_LEVEL __QDF_IPA_VOLTAGE_LEVEL

#define QDF_IPA_RM_RESOURCE_WLAN_PROD __QDF_IPA_RM_RESOURCE_WLAN_PROD
#define QDF_IPA_RM_RESOURCE_WLAN_CONS __QDF_IPA_RM_RESOURCE_WLAN_CONS
#define QDF_IPA_RM_RESOURCE_APPS_CONS __QDF_IPA_RM_RESOURCE_APPS_CONS

#define QDF_IPA_CLIENT_WLAN1_PROD __QDF_IPA_CLIENT_WLAN1_PROD
#define QDF_IPA_CLIENT_WLAN1_CONS __QDF_IPA_CLIENT_WLAN1_CONS
#define QDF_IPA_CLIENT_WLAN2_CONS __QDF_IPA_CLIENT_WLAN2_CONS
#define QDF_IPA_CLIENT_WLAN3_CONS __QDF_IPA_CLIENT_WLAN3_CONS
#define QDF_IPA_CLIENT_WLAN4_CONS __QDF_IPA_CLIENT_WLAN4_CONS

/*
 * Resume / Suspend
 */
static inline int qdf_ipa_reset_endpoint(u32 clnt_hdl)
{
	return __qdf_ipa_reset_endpoint(clnt_hdl);
}

/*
 * Remove ep delay
 */
static inline int qdf_ipa_clear_endpoint_delay(u32 clnt_hdl)
{
	return __qdf_ipa_clear_endpoint_delay(clnt_hdl);
}

/*
 * Header removal / addition
 */
static inline int qdf_ipa_add_hdr(qdf_ipa_ioc_add_hdr_t *hdrs)
{
	return __qdf_ipa_add_hdr(hdrs);
}

static inline int qdf_ipa_del_hdr(qdf_ipa_ioc_del_hdr_t *hdls)
{
	return __qdf_ipa_del_hdr(hdls);
}

static inline int qdf_ipa_commit_hdr(void)
{
	return __qdf_ipa_commit_hdr();
}

static inline int qdf_ipa_get_hdr(qdf_ipa_ioc_get_hdr_t *lookup)
{
	return __qdf_ipa_get_hdr(lookup);
}

static inline int qdf_ipa_put_hdr(u32 hdr_hdl)
{
	return __qdf_ipa_put_hdr(hdr_hdl);
}

static inline int qdf_ipa_copy_hdr(qdf_ipa_ioc_copy_hdr_t *copy)
{
	return __qdf_ipa_copy_hdr(copy);
}

/*
 * Messaging
 */
static inline int qdf_ipa_send_msg(qdf_ipa_msg_meta_t *meta, void *buff,
		ipa_msg_free_fn callback)
{
	return __qdf_ipa_send_msg(meta, buff, callback);
}

static inline int qdf_ipa_register_pull_msg(qdf_ipa_msg_meta_t *meta,
		qdf_ipa_msg_pull_fn callback)
{
	return __qdf_ipa_register_pull_msg(meta, callback);
}

static inline int qdf_ipa_deregister_pull_msg(qdf_ipa_msg_meta_t *meta)
{
	return __qdf_ipa_deregister_pull_msg(meta);
}

/*
 * Interface
 */
static inline int qdf_ipa_register_intf(const char *name,
				     const qdf_ipa_tx_intf_t *tx,
				     const qdf_ipa_rx_intf_t *rx)
{
	return __qdf_ipa_register_intf(name, tx, rx);
}

static inline int qdf_ipa_register_intf_ext(const char *name,
		const qdf_ipa_tx_intf_t *tx,
		const qdf_ipa_rx_intf_t *rx,
		const qdf_ipa_ext_intf_t *ext)
{
	return __qdf_ipa_register_intf_ext(name, tx, rx, ext);
}

static inline int qdf_ipa_deregister_intf(const char *name)
{
	return __qdf_ipa_deregister_intf(name);
}

/*
 * Data path
 */
static inline int qdf_ipa_tx_dp(qdf_ipa_client_type_t dst, struct sk_buff *skb,
		qdf_ipa_tx_meta_t *metadata)
{
	return __qdf_ipa_tx_dp(dst, skb, metadata);
}

/*
 * To transfer multiple data packets
 */
static inline int qdf_ipa_tx_dp_mul(
	qdf_ipa_client_type_t dst,
	qdf_ipa_tx_data_desc_t *data_desc)
{
	return __qdf_ipa_tx_dp_mul(dst, data_desc);
}

static inline void qdf_ipa_free_skb(qdf_ipa_rx_data_t *rx_in)
{
	return __qdf_ipa_free_skb(rx_in);;
}

/*
 * System pipes
 */
static inline u16 qdf_ipa_get_smem_restr_bytes(void)
{
	return __qdf_ipa_get_smem_restr_bytes();
}

static inline int qdf_ipa_setup_sys_pipe(qdf_ipa_sys_connect_params_t *sys_in,
		u32 *clnt_hdl)
{
	return __qdf_ipa_setup_sys_pipe(sys_in, clnt_hdl);
}

static inline int qdf_ipa_teardown_sys_pipe(u32 clnt_hdl)
{
	return __qdf_ipa_teardown_sys_pipe(clnt_hdl);
}

static inline int qdf_ipa_connect_wdi_pipe(qdf_ipa_wdi_in_params_t *in,
		qdf_ipa_wdi_out_params_t *out)
{
	return __qdf_ipa_connect_wdi_pipe(in, out);
}

static inline int qdf_ipa_disconnect_wdi_pipe(u32 clnt_hdl)
{
	return __qdf_ipa_disconnect_wdi_pipe(clnt_hdl);
}

static inline int qdf_ipa_enable_wdi_pipe(u32 clnt_hdl)
{
	return __qdf_ipa_enable_wdi_pipe(clnt_hdl);
}

static inline int qdf_ipa_disable_wdi_pipe(u32 clnt_hdl)
{
	return __qdf_ipa_disable_wdi_pipe(clnt_hdl);
}

static inline int qdf_ipa_resume_wdi_pipe(u32 clnt_hdl)
{
	return __qdf_ipa_resume_wdi_pipe(clnt_hdl);
}

static inline int qdf_ipa_suspend_wdi_pipe(u32 clnt_hdl)
{
	return __qdf_ipa_suspend_wdi_pipe(clnt_hdl);
}

static inline int qdf_ipa_uc_wdi_get_dbpa(
	qdf_ipa_wdi_db_params_t *out)
{
	return __qdf_ipa_uc_wdi_get_dbpa(out);
}

static inline int qdf_ipa_uc_reg_rdyCB(
	qdf_ipa_wdi_uc_ready_params_t *param)
{
	return __qdf_ipa_uc_reg_rdyCB(param);
}

static inline int qdf_ipa_uc_dereg_rdyCB(void)
{
	return __qdf_ipa_uc_dereg_rdyCB();
}


/*
 * Resource manager
 */
static inline int qdf_ipa_rm_create_resource(
		qdf_ipa_rm_create_params_t *create_params)
{
	return __qdf_ipa_rm_create_resource(create_params);
}

static inline int qdf_ipa_rm_delete_resource(
		qdf_ipa_rm_resource_name_t resource_name)
{
	return __qdf_ipa_rm_delete_resource(resource_name);
}

static inline int qdf_ipa_rm_register(qdf_ipa_rm_resource_name_t resource_name,
			qdf_ipa_rm_register_params_t *reg_params)
{
	return __qdf_ipa_rm_register(resource_name, reg_params);
}

static inline int qdf_ipa_rm_set_perf_profile(
		qdf_ipa_rm_resource_name_t resource_name,
		qdf_ipa_rm_perf_profile_t *profile)
{
	return __qdf_ipa_rm_set_perf_profile(resource_name, profile);
}

static inline int qdf_ipa_rm_deregister(qdf_ipa_rm_resource_name_t resource_name,
			qdf_ipa_rm_register_params_t *reg_params)
{
	return __qdf_ipa_rm_deregister(resource_name, reg_params);
}

static inline int qdf_ipa_rm_add_dependency(
		qdf_ipa_rm_resource_name_t resource_name,
		qdf_ipa_rm_resource_name_t depends_on_name)
{
	return __qdf_ipa_rm_add_dependency(resource_name, depends_on_name);
}

static inline int qdf_ipa_rm_add_dependency_sync(
		qdf_ipa_rm_resource_name_t resource_name,
		qdf_ipa_rm_resource_name_t depends_on_name)
{
	return __qdf_ipa_rm_add_dependency_sync(resource_name, depends_on_name);
}

static inline int qdf_ipa_rm_delete_dependency(
		qdf_ipa_rm_resource_name_t resource_name,
		qdf_ipa_rm_resource_name_t depends_on_name)
{
	return __qdf_ipa_rm_delete_dependency(resource_name, depends_on_name);
}

static inline int qdf_ipa_rm_request_resource(
		qdf_ipa_rm_resource_name_t resource_name)
{
	return __qdf_ipa_rm_request_resource(resource_name);
}

static inline int qdf_ipa_rm_release_resource(
		qdf_ipa_rm_resource_name_t resource_name)
{
	return __qdf_ipa_rm_release_resource(resource_name);
}

static inline int qdf_ipa_rm_notify_completion(qdf_ipa_rm_event_t event,
		qdf_ipa_rm_resource_name_t resource_name)
{
	return __qdf_ipa_rm_notify_completion(event, resource_name);
}

static inline int qdf_ipa_rm_inactivity_timer_init(
		qdf_ipa_rm_resource_name_t resource_name,
			unsigned long msecs)
{
	return __qdf_ipa_rm_inactivity_timer_init(resource_name, msecs);
}

static inline int qdf_ipa_rm_inactivity_timer_destroy(
		qdf_ipa_rm_resource_name_t resource_name)
{
	return __qdf_ipa_rm_inactivity_timer_destroy(resource_name);
}

static inline int qdf_ipa_rm_inactivity_timer_request_resource(
				qdf_ipa_rm_resource_name_t resource_name)
{
	return __qdf_ipa_rm_inactivity_timer_request_resource(resource_name);
}

static inline int qdf_ipa_rm_inactivity_timer_release_resource(
				qdf_ipa_rm_resource_name_t resource_name)
{
	return __qdf_ipa_rm_inactivity_timer_release_resource(resource_name);
}

/*
 * Miscellaneous
 */
static inline void qdf_ipa_bam_reg_dump(void)
{
	return __qdf_ipa_bam_reg_dump();
}

static inline int qdf_ipa_get_wdi_stats(qdf_ipa_hw_stats_wdi_info_data_t *stats)
{
	return __qdf_ipa_get_wdi_stats(stats);
}

static inline int qdf_ipa_get_ep_mapping(qdf_ipa_client_type_t client)
{
	return __qdf_ipa_get_ep_mapping(client);
}

static inline bool qdf_ipa_is_ready(void)
{
	return __qdf_ipa_is_ready();
}

static inline void qdf_ipa_proxy_clk_vote(void)
{
	return __qdf_ipa_proxy_clk_vote();
}

static inline void qdf_ipa_proxy_clk_unvote(void)
{
	return __qdf_ipa_proxy_clk_unvote();
}

static inline bool qdf_ipa_is_client_handle_valid(u32 clnt_hdl)
{
	return __qdf_ipa_is_client_handle_valid(clnt_hdl);
}

static inline qdf_ipa_client_type_t qdf_ipa_get_client_mapping(int pipe_idx)
{
	return __qdf_ipa_get_client_mapping(pipe_idx);
}

static inline qdf_ipa_rm_resource_name_t qdf_ipa_get_rm_resource_from_ep(
	int pipe_idx)
{
	return __qdf_ipa_get_rm_resource_from_ep(pipe_idx);
}

static inline bool qdf_ipa_get_modem_cfg_emb_pipe_flt(void)
{
	return __qdf_ipa_get_modem_cfg_emb_pipe_flt();
}

static inline int qdf_ipa_create_wdi_mapping(u32 num_buffers,
		__qdf_ipa_wdi_buffer_info_t *info)
{
	return __qdf_ipa_create_wdi_mapping(num_buffers, info);
}

static inline int qdf_ipa_release_wdi_mapping(u32 num_buffers,
		qdf_ipa_wdi_buffer_info_t *info)
{
	return __qdf_ipa_release_wdi_mapping(num_buffers, info);
}

static inline int qdf_ipa_disable_apps_wan_cons_deaggr(uint32_t agg_size,
		uint32_t agg_count)
{
	return __qdf_ipa_disable_apps_wan_cons_deaggr(agg_size, agg_count);
}

static inline const qdf_ipa_gsi_ep_config_t *qdf_ipa_get_gsi_ep_info(qdf_ipa_client_type_t client)
{
	return __qdf_ipa_get_gsi_ep_info(client);
}

static inline int qdf_ipa_stop_gsi_channel(u32 clnt_hdl)
{
	return __qdf_ipa_stop_gsi_channel(clnt_hdl);
}

static inline int qdf_ipa_register_ipa_ready_cb(
	void (*qdf_ipa_ready_cb)(void *user_data),
	void *user_data)
{
	return __qdf_ipa_register_ipa_ready_cb(qdf_ipa_ready_cb, user_data);
}

#ifdef FEATURE_METERING
static inline int qdf_ipa_broadcast_wdi_quota_reach_ind(uint32_t index,
							uint64_t quota_bytes)
{
	return __qdf_ipa_broadcast_wdi_quota_reach_ind(index, quota_bytes);
}
#endif

#ifdef ENABLE_SMMU_S1_TRANSLATION
/**
 * qdf_get_ipa_smmu_enabled() - to get IPA SMMU enable status
 *
 * Return: true when IPA SMMU enabled, otherwise false
 */
static inline bool qdf_get_ipa_smmu_enabled(void)
{
	return __qdf_get_ipa_smmu_enabled();
}
#endif

#ifdef IPA_LAN_RX_NAPI_SUPPORT
/**
 * qdf_ipa_get_lan_rx_napi() - Check if NAPI is enabled in LAN
 * RX DP
 *
 * Returns: true if enabled, false otherwise
 */
static inline bool qdf_ipa_get_lan_rx_napi(void)
{
	return __qdf_ipa_get_lan_rx_napi();
}
#else
static inline bool qdf_ipa_get_lan_rx_napi(void)
{
	return false;
}
#endif /* IPA_LAN_RX_NAPI_SUPPORT */
#endif /* IPA_OFFLOAD */
#endif /* _QDF_IPA_H */
