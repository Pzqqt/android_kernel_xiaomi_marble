/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2015-2020, The Linux Foundation. All rights reserved.
 */

#include <linux/ipa_mhi.h>
#include <linux/ipa_uc_offload.h>
#include <linux/ipa_wdi3.h>
#include "ipa_common_i.h"

#ifndef _IPA_API_H_
#define _IPA_API_H_

struct ipa_api_controller {
	int (*ipa_cfg_ep)(u32 clnt_hdl, const struct ipa_ep_cfg *ipa_ep_cfg);

	int (*ipa_cfg_ep_nat)(u32 clnt_hdl,
		const struct ipa_ep_cfg_nat *ipa_ep_cfg);

	int (*ipa_cfg_ep_conn_track)(u32 clnt_hdl,
		const struct ipa_ep_cfg_conn_track *ipa_ep_cfg);

	int (*ipa_cfg_ep_hdr)(u32 clnt_hdl,
		const struct ipa_ep_cfg_hdr *ipa_ep_cfg);

	int (*ipa_cfg_ep_hdr_ext)(u32 clnt_hdl,
			const struct ipa_ep_cfg_hdr_ext *ipa_ep_cfg);

	int (*ipa_cfg_ep_mode)(u32 clnt_hdl,
		const struct ipa_ep_cfg_mode *ipa_ep_cfg);

	int (*ipa_cfg_ep_aggr)(u32 clnt_hdl,
		const struct ipa_ep_cfg_aggr *ipa_ep_cfg);

	int (*ipa_cfg_ep_deaggr)(u32 clnt_hdl,
		const struct ipa_ep_cfg_deaggr *ipa_ep_cfg);

	int (*ipa_cfg_ep_route)(u32 clnt_hdl,
		const struct ipa_ep_cfg_route *ipa_ep_cfg);

	int (*ipa_cfg_ep_holb)(u32 clnt_hdl,
		const struct ipa_ep_cfg_holb *ipa_ep_cfg);

	int (*ipa_cfg_ep_cfg)(u32 clnt_hdl,
		const struct ipa_ep_cfg_cfg *ipa_ep_cfg);

	int (*ipa_cfg_ep_metadata_mask)(u32 clnt_hdl,
		const struct ipa_ep_cfg_metadata_mask *ipa_ep_cfg);

	int (*ipa_cfg_ep_holb_by_client)(enum ipa_client_type client,
		const struct ipa_ep_cfg_holb *ipa_ep_cfg);

	int (*ipa_add_hdr_usr)(struct ipa_ioc_add_hdr *hdrs, bool user_only);

	int (*ipa_reset_hdr)(bool user_only);

	int (*ipa_add_hdr_proc_ctx)(struct ipa_ioc_add_hdr_proc_ctx *proc_ctxs,
								bool user_only);

	int (*ipa_del_hdr_proc_ctx)(struct ipa_ioc_del_hdr_proc_ctx *hdls);

	int (*ipa_add_rt_rule_v2)(struct ipa_ioc_add_rt_rule_v2 *rules);

	int (*ipa_add_rt_rule_usr)(struct ipa_ioc_add_rt_rule *rules,
							bool user_only);

	int (*ipa_add_rt_rule_usr_v2)(struct ipa_ioc_add_rt_rule_v2 *rules,
							bool user_only);

	int (*ipa_del_rt_rule)(struct ipa_ioc_del_rt_rule *hdls);

	int (*ipa_commit_rt)(enum ipa_ip_type ip);

	int (*ipa_reset_rt)(enum ipa_ip_type ip, bool user_only);

	int (*ipa_get_rt_tbl)(struct ipa_ioc_get_rt_tbl *lookup);

	int (*ipa_query_rt_index)(struct ipa_ioc_get_rt_tbl_indx *in);

	int (*ipa_mdfy_rt_rule)(struct ipa_ioc_mdfy_rt_rule *rules);

	int (*ipa_mdfy_rt_rule_v2)(struct ipa_ioc_mdfy_rt_rule_v2 *rules);

	int (*ipa_add_flt_rule)(struct ipa_ioc_add_flt_rule *rules);

	int (*ipa_add_flt_rule_v2)(struct ipa_ioc_add_flt_rule_v2 *rules);

	int (*ipa_add_flt_rule_usr)(struct ipa_ioc_add_flt_rule *rules,
								bool user_only);

	int (*ipa_add_flt_rule_usr_v2)
		(struct ipa_ioc_add_flt_rule_v2 *rules, bool user_only);

	int (*ipa_del_flt_rule)(struct ipa_ioc_del_flt_rule *hdls);

	int (*ipa_mdfy_flt_rule)(struct ipa_ioc_mdfy_flt_rule *rules);

	int (*ipa_mdfy_flt_rule_v2)(struct ipa_ioc_mdfy_flt_rule_v2 *rules);

	int (*ipa_commit_flt)(enum ipa_ip_type ip);

	int (*ipa_reset_flt)(enum ipa_ip_type ip, bool user_only);

	int (*ipa_allocate_nat_device)(struct ipa_ioc_nat_alloc_mem *mem);

	int (*ipa_allocate_nat_table)(
		struct ipa_ioc_nat_ipv6ct_table_alloc *table_alloc);

	int (*ipa_allocate_ipv6ct_table)(
		struct ipa_ioc_nat_ipv6ct_table_alloc *table_alloc);

	int (*ipa_nat_init_cmd)(struct ipa_ioc_v4_nat_init *init);

	int (*ipa_ipv6ct_init_cmd)(struct ipa_ioc_ipv6ct_init *init);

	int (*ipa_nat_dma_cmd)(struct ipa_ioc_nat_dma_cmd *dma);

	int (*ipa_table_dma_cmd)(struct ipa_ioc_nat_dma_cmd *dma);

	int (*ipa_nat_del_cmd)(struct ipa_ioc_v4_nat_del *del);

	int (*ipa_del_nat_table)(struct ipa_ioc_nat_ipv6ct_table_del *del);

	int (*ipa_del_ipv6ct_table)(struct ipa_ioc_nat_ipv6ct_table_del *del);

	int (*ipa_nat_mdfy_pdn)(struct ipa_ioc_nat_pdn_entry *mdfy_pdn);

	int (*ipa_sys_setup)(struct ipa_sys_connect_params *sys_in,
		unsigned long *ipa_bam_hdl,
		u32 *ipa_pipe_num, u32 *clnt_hdl, bool en_status);

	int (*ipa_sys_teardown)(u32 clnt_hdl);

	int (*ipa_sys_update_gsi_hdls)(u32 clnt_hdl, unsigned long gsi_ch_hdl,
		unsigned long gsi_ev_hdl);

	int (*ipa_set_wlan_tx_info)(struct ipa_wdi_tx_info *info);

	int (*teth_bridge_init)(struct teth_bridge_init_params *params);

	int (*teth_bridge_disconnect)(enum ipa_client_type client);

	int (*teth_bridge_connect)(
		struct teth_bridge_connect_params *connect_params);

	void (*ipa_set_client)(
		int index, enum ipacm_client_enum client, bool uplink);

	enum ipacm_client_enum (*ipa_get_client)(int pipe_idx);

	bool (*ipa_get_client_uplink)(int pipe_idx);

	bool (*ipa_has_open_aggr_frame)(enum ipa_client_type client);

	int (*ipa_generate_tag_process)(void);

	int (*ipa_disable_sps_pipe)(enum ipa_client_type client);

	void (*ipa_set_tag_process_before_gating)(bool val);

	int (*ipa_mhi_init_engine)(struct ipa_mhi_init_engine *params);

	int (*ipa_connect_mhi_pipe)(struct ipa_mhi_connect_params_internal *in,
		u32 *clnt_hdl);

	int (*ipa_disconnect_mhi_pipe)(u32 clnt_hdl);

	bool (*ipa_mhi_stop_gsi_channel)(enum ipa_client_type client);

	int (*ipa_qmi_disable_force_clear)(u32 request_id);

	int (*ipa_qmi_enable_force_clear_datapath_send)(
		struct ipa_enable_force_clear_datapath_req_msg_v01 *req);

	int (*ipa_qmi_disable_force_clear_datapath_send)(
		struct ipa_disable_force_clear_datapath_req_msg_v01 *req);

	bool (*ipa_mhi_sps_channel_empty)(enum ipa_client_type client);

	int (*ipa_mhi_reset_channel_internal)(enum ipa_client_type client);

	int (*ipa_mhi_start_channel_internal)(enum ipa_client_type client);

	void (*ipa_get_holb)(int ep_idx, struct ipa_ep_cfg_holb *holb);

	int (*ipa_mhi_query_ch_info)(enum ipa_client_type client,
			struct gsi_chan_info *ch_info);

	int (*ipa_mhi_resume_channels_internal)(
			enum ipa_client_type client,
			bool LPTransitionRejected,
			bool brstmode_enabled,
			union __packed gsi_channel_scratch ch_scratch,
			u8 index);

	int  (*ipa_mhi_destroy_channel)(enum ipa_client_type client);

	int (*ipa_uc_mhi_send_dl_ul_sync_info)
		(union IpaHwMhiDlUlSyncCmdData_t *cmd);

	int (*ipa_uc_mhi_init)
		(void (*ready_cb)(void), void (*wakeup_request_cb)(void));

	void (*ipa_uc_mhi_cleanup)(void);

	int (*ipa_uc_mhi_print_stats)(char *dbg_buff, int size);

	int (*ipa_uc_mhi_reset_channel)(int channelHandle);

	int (*ipa_uc_mhi_suspend_channel)(int channelHandle);

	int (*ipa_uc_mhi_stop_event_update_channel)(int channelHandle);

	int (*ipa_uc_state_check)(void);

	int (*ipa_write_qmap_id)(struct ipa_ioc_write_qmapid *param_in);

	int (*ipa_remove_interrupt_handler)(enum ipa_irq_type interrupt);

	int (*ipa_ap_suspend)(struct device *dev);

	int (*ipa_ap_resume)(struct device *dev);

	int (*ipa_start_gsi_channel)(u32 clnt_hdl);

	void (*ipa_inc_client_enable_clks)(
		struct ipa_active_client_logging_info *id);

	void (*ipa_dec_client_disable_clks)(
		struct ipa_active_client_logging_info *id);

	int (*ipa_inc_client_enable_clks_no_block)(
		struct ipa_active_client_logging_info *id);

	int (*ipa_suspend_resource_no_block)(
		enum ipa_rm_resource_name resource);

	int (*ipa_resume_resource)(enum ipa_rm_resource_name name);

	int (*ipa_suspend_resource_sync)(enum ipa_rm_resource_name resource);

	int (*ipa_set_required_perf_profile)(
		enum ipa_voltage_level floor_voltage, u32 bandwidth_mbps);

	void *(*ipa_get_ipc_logbuf)(void);

	void *(*ipa_get_ipc_logbuf_low)(void);

	int (*ipa_rx_poll)(u32 clnt_hdl, int budget);

	void (*ipa_recycle_wan_skb)(struct sk_buff *skb);

	int (*ipa_setup_uc_ntn_pipes)(struct ipa_ntn_conn_in_params *in,
		ipa_notify_cb notify, void *priv, u8 hdr_len,
		struct ipa_ntn_conn_out_params *outp);

	int (*ipa_tear_down_uc_offload_pipes)(int ipa_ep_idx_ul,
		int ipa_ep_idx_dl, struct ipa_ntn_conn_in_params *params);

	struct device *(*ipa_get_pdev)(void);

	int (*ipa_ntn_uc_reg_rdyCB)(void (*ipauc_ready_cb)(void *user_data),
		void *user_data);

	void (*ipa_ntn_uc_dereg_rdyCB)(void);

	int (*ipa_conn_wdi_pipes)(struct ipa_wdi_conn_in_params *in,
		struct ipa_wdi_conn_out_params *out,
		ipa_wdi_meter_notifier_cb wdi_notify);

	int (*ipa_disconn_wdi_pipes)(int ipa_ep_idx_tx,
		int ipa_ep_idx_rx);

	int (*ipa_enable_wdi_pipes)(int ipa_ep_idx_tx,
		int ipa_ep_idx_rx);

	int (*ipa_disable_wdi_pipes)(int ipa_ep_idx_tx,
		int ipa_ep_idx_rx);

	int (*ipa_tz_unlock_reg)(struct ipa_tz_unlock_reg_info *reg_info,
		u16 num_regs);

	int (*ipa_get_smmu_params)(struct ipa_smmu_in_params *in,
		struct ipa_smmu_out_params *out);
	int (*ipa_is_vlan_mode)(enum ipa_vlan_ifaces iface, bool *res);

	int (*ipa_wigig_internal_init)(
		struct ipa_wdi_uc_ready_params *inout,
		ipa_wigig_misc_int_cb int_notify,
		phys_addr_t *uc_db_pa);

	int (*ipa_conn_wigig_rx_pipe_i)(void *in,
		struct ipa_wigig_conn_out_params *out,
		struct dentry **parent);

	int (*ipa_conn_wigig_client_i)(void *in,
		struct ipa_wigig_conn_out_params *out,
		ipa_notify_cb tx_notify,
		void *priv);

	int (*ipa_disconn_wigig_pipe_i)(enum ipa_client_type client,
		struct ipa_wigig_pipe_setup_info_smmu *pipe_smmu,
		void *dbuff);

	int (*ipa_wigig_uc_msi_init)(bool init,
		phys_addr_t periph_baddr_pa,
		phys_addr_t pseudo_cause_pa,
		phys_addr_t int_gen_tx_pa,
		phys_addr_t int_gen_rx_pa,
		phys_addr_t dma_ep_misc_pa);

	int (*ipa_enable_wigig_pipe_i)(enum ipa_client_type client);

	int (*ipa_disable_wigig_pipe_i)(enum ipa_client_type client);

	void (*ipa_register_client_callback)(
		int (*client_cb)(bool is_lock),
		bool (*teth_port_state)(void), enum ipa_client_type client);

	void (*ipa_deregister_client_callback)(enum ipa_client_type client);

	int (*ipa_uc_debug_stats_alloc)(
		struct IpaHwOffloadStatsAllocCmdData_t cmdinfo);

	int (*ipa_uc_debug_stats_dealloc)(uint32_t prot_id);

	void (*ipa_get_gsi_stats)(int prot_id,
		struct ipa_uc_dbg_ring_stats *stats);

	int (*ipa_get_prot_id)(enum ipa_client_type client);
};

#if IS_ENABLED(CONFIG_IPA3)

int ipa3_plat_drv_probe(struct platform_device *pdev_p,
	struct ipa_api_controller *api_ctrl,
	const struct of_device_id *pdrv_match);
int ipa3_pci_drv_probe(
	struct pci_dev            *pci_dev,
	struct ipa_api_controller *api_ctrl,
	const struct of_device_id *pdrv_match);

#else /* IS_ENABLED(CONFIG_IPA3) */

static inline int ipa3_plat_drv_probe(struct platform_device *pdev_p,
	struct ipa_api_controller *api_ctrl,
	const struct of_device_id *pdrv_match)
{
	return -ENODEV;
}
static inline int ipa3_pci_drv_probe(
	struct pci_dev            *pci_dev,
	struct ipa_api_controller *api_ctrl,
	const struct of_device_id *pdrv_match)
{
	return -ENODEV;
}

#endif /* IS_ENABLED(CONFIG_IPA3) */

#endif /* _IPA_API_H_ */
