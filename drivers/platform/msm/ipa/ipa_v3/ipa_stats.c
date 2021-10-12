/* SPDX-License-Identifier: GPL-2.0-only */
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "ipa_stats.h"
#include <linux/fs.h>
#include "ipa_i.h"
#include "ipahal.h"
#include "ipa_odl.h"
#include "ipa_common_i.h"
#include <linux/msm_ipa.h>
#include "gsi.h"

#define DRIVER_NAME "ipa_lnx_stats_ioctl"
#define DEV_NAME_IPA_LNX_STATS "ipa-lnx-stats"

#define IPA_STATS_DBG(fmt, args...) \
	do { \
		pr_debug(DEV_NAME_IPA_LNX_STATS " %s:%d " fmt, __func__,\
				__LINE__, ## args); \
		IPA_IPC_LOGGING(ipa3_get_ipc_logbuf(), \
				DEV_NAME_IPA_LNX_STATS " %s:%d " fmt, ## args); \
		IPA_IPC_LOGGING(ipa3_get_ipc_logbuf_low(), \
				DEV_NAME_IPA_LNX_STATS " %s:%d " fmt, ## args); \
	} while (0)

#define IPA_STATS_ERR(fmt, args...) \
	do { \
		pr_err(DEV_NAME_IPA_LNX_STATS " %s:%d " fmt, __func__,\
				__LINE__, ## args); \
		IPA_IPC_LOGGING(ipa3_get_ipc_logbuf(), \
				DEV_NAME_IPA_LNX_STATS " %s:%d " fmt, ## args); \
		IPA_IPC_LOGGING(ipa3_get_ipc_logbuf_low(), \
				DEV_NAME_IPA_LNX_STATS " %s:%d " fmt, ## args); \
	} while (0)

static unsigned int dev_num = 1;
static struct cdev ipa_lnx_stats_ioctl_cdev;
static struct class *class;
static dev_t device;

struct ipa_lnx_stats_spearhead_ctx ipa_lnx_agent_ctx;

struct wlan_intf_mode_cnt {
	u8 ap_cnt;
	u8 sta_cnt;
};

enum wlan_intf_mode {
	AP,
	AP_AP,
	AP_STA,
	AP_AP_STA,
	AP_AP_AP,
	AP_AP_AP_STA,
	AP_AP_AP_AP_,
	WLAN_INTF_MODE_MAX
};

union ipa_gsi_ring_prev_poll_info {
	struct {
		uint32_t num_tx_ring_100_perc_with_cred;
		uint32_t num_tx_ring_0_perc_with_cred;
		uint32_t num_tx_ring_above_75_perc_cred;
		uint32_t num_tx_ring_above_25_perc_cred;
		uint32_t num_tx_ring_stats_polled;
	} tx_cred_info;
	struct {
		uint32_t num_rx_ring_100_perc_with_pack;
		uint32_t num_rx_ring_0_perc_with_pack;
		uint32_t num_rx_ring_above_75_perc_pack;
		uint32_t num_rx_ring_above_25_perc_pack;
		uint32_t num_rx_ring_stats_polled;
	} rx_pack_info;
};

union ipa_gsi_ring_prev_poll_info poll_pack_and_cred_info[IPA_CLIENT_MAX];

static enum wlan_intf_mode ipa_get_wlan_intf_mode(void)
{
	struct wlan_intf_mode_cnt mode_cnt;
	mode_cnt.ap_cnt = ipa3_ctx->stats.msg_w[WLAN_AP_CONNECT] -
		ipa3_ctx->stats.msg_w[WLAN_AP_DISCONNECT];
	mode_cnt.sta_cnt = ipa3_ctx->stats.msg_w[WLAN_STA_CONNECT] -
		ipa3_ctx->stats.msg_w[WLAN_STA_DISCONNECT];
	if ((mode_cnt.ap_cnt < 0) || (mode_cnt.sta_cnt < 0))
		return WLAN_INTF_MODE_MAX;

	switch (mode_cnt.ap_cnt) {
	case 1:
		if (mode_cnt.sta_cnt == 1)
			return AP_STA;
		return AP;
	case 2:
		if (mode_cnt.sta_cnt == 1)
			return AP_AP_STA;
		return AP_AP;
	case 3:
		if (mode_cnt.sta_cnt == 1)
			return AP_AP_AP_STA;
		return AP_AP_AP;
	case 4:
		return AP_AP_AP_AP_;
	default:
		if (mode_cnt.ap_cnt > 4)
			return AP_AP_AP_AP_;
		return WLAN_INTF_MODE_MAX;
	}
}

static int ipa_stats_ioctl_open(struct inode *inode, struct file *filp)
{
	return 0;
}

static bool ipa_stats_struct_mismatch(enum ipa_lnx_stats_ioc_cmd_type type)
{
	switch (type) {
	case IPA_LNX_CMD_GET_ALLOC_INFO:
		if (IPA_LNX_EACH_INST_ALLOC_INFO_STRUCT_LEN_INT !=
				IPA_LNX_EACH_INST_ALLOC_INFO_STRUCT_LEN ||
			IPA_LNX_STATS_ALL_INFO_STRUCT_LEN_INT !=
				IPA_LNX_STATS_ALL_INFO_STRUCT_LEN ||
			IPA_LNX_STATS_SPEARHEAD_CTX_STRUCT_LEN_INT !=
				IPA_LNX_STATS_SPEARHEAD_CTX_STRUCT_LEN) {
				IPA_STATS_ERR("IPA_LNX_CMD_GET_ALLOC_INFO size mismatch");
				return true;
		} else return false;
	case IPA_LNX_CMD_GENERIC_STATS:
		if (IPA_LNX_PG_RECYCLE_STATS_STRUCT_LEN_INT !=
				IPA_LNX_PG_RECYCLE_STATS_STRUCT_LEN ||
			IPA_LNX_EXCEPTION_STATS_STRUCT_LEN_INT !=
				IPA_LNX_EXCEPTION_STATS_STRUCT_LEN ||
			IPA_LNX_ODL_EP_STATS_STRUCT_LEN_INT !=
				IPA_LNX_ODL_EP_STATS_STRUCT_LEN ||
			IPA_LNX_HOLB_DISCARD_STATS_STRUCT_LEN_INT !=
				IPA_LNX_HOLB_DISCARD_STATS_STRUCT_LEN ||
			IPA_LNX_HOLB_MONITOR_STATS_STRUCT_LEN_INT !=
				IPA_LNX_HOLB_MONITOR_STATS_STRUCT_LEN ||
			IPA_LNX_HOLB_DROP_AND_MON_STATS_STRUCT_LEN_INT !=
				IPA_LNX_HOLB_DROP_AND_MON_STATS_STRUCT_LEN ||
			IPA_LNX_GENERIC_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GENERIC_STATS_STRUCT_LEN) {
				IPA_STATS_ERR("IPA_LNX_CMD_GENERIC_STATS size mismatch");
				return true;
		} else return false;
	case IPA_LNX_CMD_CLOCK_STATS:
		if (IPA_LNX_PM_CLIENT_STATS_STRUCT_LEN_INT !=
				IPA_LNX_PM_CLIENT_STATS_STRUCT_LEN ||
			IPA_LNX_CLOCK_STATS_STRUCT_LEN_INT !=
				IPA_LNX_CLOCK_STATS_STRUCT_LEN) {
				IPA_STATS_ERR("IPA_LNX_CMD_CLOCK_STATS size mismatch");
				return true;
		} else return false;
	case IPA_LNX_CMD_WLAN_INST_STATS:
		if (IPA_LNX_GSI_RX_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_RX_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_GSI_TX_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_TX_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_GSI_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_PIPE_INFO_STATS_STRUCT_LEN_INT !=
				IPA_LNX_PIPE_INFO_STATS_STRUCT_LEN ||
			IPA_LNX_WLAN_INSTANCE_INFO_STRUCT_LEN_INT !=
				IPA_LNX_WLAN_INSTANCE_INFO_STRUCT_LEN ||
			IPA_LNX_WLAN_INST_STATS_STRUCT_LEN_INT !=
				IPA_LNX_WLAN_INST_STATS_STRUCT_LEN) {
				IPA_STATS_ERR("IPA_LNX_CMD_WLAN_INST_STATS size mismatch");
				return true;
		} else return false;
	case IPA_LNX_CMD_ETH_INST_STATS:
		if (IPA_LNX_GSI_RX_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_RX_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_GSI_TX_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_TX_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_GSI_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_PIPE_INFO_STATS_STRUCT_LEN_INT !=
				IPA_LNX_PIPE_INFO_STATS_STRUCT_LEN ||
			IPA_LNX_ETH_INSTANCE_INFO_STRUCT_LEN_INT !=
				IPA_LNX_ETH_INSTANCE_INFO_STRUCT_LEN ||
			IPA_LNX_ETH_INST_STATS_STRUCT_LEN_INT !=
				IPA_LNX_ETH_INST_STATS_STRUCT_LEN) {
				IPA_STATS_ERR("IPA_LNX_CMD_ETH_INST_STATS size mismatch");
				return true;
		} else return false;
	case IPA_LNX_CMD_USB_INST_STATS:
		if (IPA_LNX_GSI_RX_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_RX_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_GSI_TX_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_TX_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_GSI_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_PIPE_INFO_STATS_STRUCT_LEN_INT !=
				IPA_LNX_PIPE_INFO_STATS_STRUCT_LEN ||
			IPA_LNX_USB_INSTANCE_INFO_STRUCT_LEN_INT !=
				IPA_LNX_USB_INSTANCE_INFO_STRUCT_LEN ||
			IPA_LNX_USB_INST_STATS_STRUCT_LEN_INT !=
				IPA_LNX_USB_INST_STATS_STRUCT_LEN) {
				IPA_STATS_ERR("IPA_LNX_CMD_USB_INST_STATS ize mismatch");
				return true;
		} else return false;
	case IPA_LNX_CMD_MHIP_INST_STATS:
		if (IPA_LNX_GSI_RX_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_RX_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_GSI_TX_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_TX_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_GSI_DEBUG_STATS_STRUCT_LEN_INT !=
				IPA_LNX_GSI_DEBUG_STATS_STRUCT_LEN ||
			IPA_LNX_PIPE_INFO_STATS_STRUCT_LEN_INT !=
				IPA_LNX_PIPE_INFO_STATS_STRUCT_LEN ||
			IPA_LNX_MHIP_INSTANCE_INFO_STRUCT_LEN_INT !=
				IPA_LNX_MHIP_INSTANCE_INFO_STRUCT_LEN ||
			IPA_LNX_MHIP_INST_STATS_STRUCT_LEN_INT !=
				IPA_LNX_MHIP_INST_STATS_STRUCT_LEN) {
				IPA_STATS_ERR("IPA_LNX_CMD_MHIP_INST_STATS size mismatch");
				return true;
		} else return false;
	default:
		return true;
	}
}

static int ipa_get_generic_stats(unsigned long arg)
{
	int res;
	int i, j;
	struct ipa_lnx_generic_stats *generic_stats;
	struct ipa_drop_stats_all *out;
	int alloc_size;
	int reg_idx;
	struct ipa_uc_holb_client_info *holb_client;
	struct holb_discard_stats *holb_disc_stats_ptr;
	struct holb_monitor_stats *holb_mon_stats_ptr;

	alloc_size = sizeof(struct ipa_lnx_generic_stats) +
		(sizeof(struct holb_discard_stats) *
			ipa_lnx_agent_ctx.alloc_info.num_holb_drop_stats_clients) +
		(sizeof(struct holb_monitor_stats) *
			ipa_lnx_agent_ctx.alloc_info.num_holb_mon_stats_clients);

	generic_stats = (struct ipa_lnx_generic_stats *) memdup_user((
		const void __user *)arg, alloc_size);
	if (IS_ERR(generic_stats)) {
		IPA_STATS_ERR("copy from user failed");
		return -ENOMEM;
	}

	generic_stats->tx_dma_pkts = ipa3_ctx->stats.tx_sw_pkts;
	generic_stats->tx_hw_pkts = ipa3_ctx->stats.tx_hw_pkts;
	generic_stats->tx_non_linear = ipa3_ctx->stats.tx_non_linear;
	generic_stats->tx_pkts_compl = ipa3_ctx->stats.tx_pkts_compl;
	generic_stats->stats_compl = ipa3_ctx->stats.stat_compl;
	generic_stats->active_eps =
		atomic_read(&ipa3_ctx->ipa3_active_clients.cnt);
	generic_stats->wan_rx_empty = ipa3_ctx->stats.wan_rx_empty;
	generic_stats->wan_repl_rx_empty = ipa3_ctx->stats.wan_repl_rx_empty;
	generic_stats->lan_rx_empty = ipa3_ctx->stats.lan_rx_empty;
	generic_stats->lan_repl_rx_empty = ipa3_ctx->stats.lan_repl_rx_empty;
	/* Page recycle stats */
	generic_stats->pg_rec_stats.coal_total_repl_buff =
		ipa3_ctx->stats.page_recycle_stats[0].total_replenished;
	generic_stats->pg_rec_stats.coal_temp_repl_buff =
		ipa3_ctx->stats.page_recycle_stats[0].tmp_alloc;
	generic_stats->pg_rec_stats.def_total_repl_buff =
		ipa3_ctx->stats.page_recycle_stats[1].total_replenished;
	generic_stats->pg_rec_stats.def_temp_repl_buff =
		ipa3_ctx->stats.page_recycle_stats[1].tmp_alloc;
	/* Exception stats */
	generic_stats->excep_stats.excptn_type_none =
		ipa3_ctx->stats.rx_excp_pkts[IPAHAL_PKT_STATUS_EXCEPTION_NONE];
	generic_stats->excep_stats.excptn_type_deaggr =
		ipa3_ctx->stats.rx_excp_pkts[IPAHAL_PKT_STATUS_EXCEPTION_DEAGGR];
	generic_stats->excep_stats.excptn_type_iptype =
		ipa3_ctx->stats.rx_excp_pkts[IPAHAL_PKT_STATUS_EXCEPTION_IPTYPE];
	generic_stats->excep_stats.excptn_type_pkt_len =
		ipa3_ctx->stats.rx_excp_pkts[
			IPAHAL_PKT_STATUS_EXCEPTION_PACKET_LENGTH];
	generic_stats->excep_stats.excptn_type_pkt_thrshld =
		ipa3_ctx->stats.rx_excp_pkts[
			IPAHAL_PKT_STATUS_EXCEPTION_PACKET_THRESHOLD];
	generic_stats->excep_stats.excptn_type_frag_rule_miss =
		ipa3_ctx->stats.rx_excp_pkts[
			IPAHAL_PKT_STATUS_EXCEPTION_FRAG_RULE_MISS];
	generic_stats->excep_stats.excptn_type_sw_flt =
		ipa3_ctx->stats.rx_excp_pkts[IPAHAL_PKT_STATUS_EXCEPTION_SW_FILT];
	generic_stats->excep_stats.excptn_type_nat =
		ipa3_ctx->stats.rx_excp_pkts[IPAHAL_PKT_STATUS_EXCEPTION_NAT];
	generic_stats->excep_stats.excptn_type_ipv6_ct =
		ipa3_ctx->stats.rx_excp_pkts[IPAHAL_PKT_STATUS_EXCEPTION_IPV6CT];
	generic_stats->excep_stats.excptn_type_csum =
		ipa3_ctx->stats.rx_excp_pkts[IPAHAL_PKT_STATUS_EXCEPTION_CSUM];
	/* ODL EP stats */
	if (ipa3_odl_ctx) {
		generic_stats->odl_stats.rx_pkt = ipa3_odl_ctx->stats.odl_rx_pkt;
		generic_stats->odl_stats.processed_pkt =
			ipa3_odl_ctx->stats.odl_tx_diag_pkt;
		generic_stats->odl_stats.dropped_pkt =
			ipa3_odl_ctx->stats.odl_drop_pkt;
		generic_stats->odl_stats.num_queue_pkt =
			atomic_read(&ipa3_odl_ctx->stats.numer_in_queue);
	}
	/* HOLB discard stats */
	if (!(ipa3_ctx->hw_stats && ipa3_ctx->hw_stats->enabled)) {
		generic_stats->holb_stats.num_holb_disc_pipes = 0;
		generic_stats->holb_stats.num_holb_mon_clients = 0;
	}

	generic_stats->holb_stats.num_holb_disc_pipes =
		ipa_lnx_agent_ctx.alloc_info.num_holb_drop_stats_clients;
	generic_stats->holb_stats.num_holb_mon_clients =
		ipa_lnx_agent_ctx.alloc_info.num_holb_mon_stats_clients;

	out = kzalloc(sizeof(*out), GFP_KERNEL);
	if (!out) {
		kfree(generic_stats);
		return -ENOMEM;
	}

	res = ipa_get_drop_stats(out);
	if (res) {
		kfree(out);
		kfree(generic_stats);
		return res;
	}

	/* HOLB Discard stats */
	holb_disc_stats_ptr = &generic_stats->holb_stats.holb_disc_stats[0];
	for (i = 0; i < IPA_CLIENT_MAX; i++) {
		int ep_idx = ipa3_get_ep_mapping(i);

		if ((ep_idx == -1) || (!IPA_CLIENT_IS_CONS(i)) ||
			(IPA_CLIENT_IS_TEST(i)))
			continue;

		reg_idx = ipahal_get_ep_reg_idx(ep_idx);
		if (!(ipa3_ctx->hw_stats &&
			(ipa3_ctx->hw_stats->drop.init.enabled_bitmask[reg_idx] &
			ipahal_get_ep_bit(ep_idx))))
			continue;

		holb_disc_stats_ptr->client_type = i;
		holb_disc_stats_ptr->num_drp_cnt = out->client[i].drop_packet_cnt;
		holb_disc_stats_ptr->num_drp_bytes = out->client[i].drop_byte_cnt;
		holb_disc_stats_ptr = (struct holb_discard_stats *)((
			uint64_t)holb_disc_stats_ptr + sizeof(struct holb_discard_stats));
	}

	/* HOLB Monitor stats */
	holb_mon_stats_ptr = (struct holb_monitor_stats *)(
		&generic_stats->holb_stats.holb_disc_stats[0] +
		(ipa_lnx_agent_ctx.alloc_info.num_holb_drop_stats_clients *
		sizeof(struct holb_discard_stats)));
	for (i = 0; i < generic_stats->holb_stats.num_holb_mon_clients; i++) {
		holb_client = &(ipa3_ctx->uc_ctx.holb_monitor.client[i]);
		/* Get the client type from gsi_hdl */
		for (j = 0; j < IPA5_MAX_NUM_PIPES; j++) {
			if (ipa3_ctx->ep[j].gsi_chan_hdl == holb_client->gsi_chan_hdl) {
				holb_mon_stats_ptr->client_type = ipa3_ctx->ep[j].client;
				break;
			}
		}
		holb_mon_stats_ptr->curr_index = holb_client->current_idx;
		holb_mon_stats_ptr->num_en_cnt = holb_client->enable_cnt;
		holb_mon_stats_ptr->num_dis_cnt = holb_client->disable_cnt;
		holb_mon_stats_ptr = (struct holb_monitor_stats *)((
			uint64_t)holb_mon_stats_ptr + sizeof(struct holb_monitor_stats));
	}

	if(copy_to_user((void __user *)arg,
		(u8 *)generic_stats,
		alloc_size)) {
		kfree(generic_stats);
		kfree(out);
		IPA_STATS_ERR("copy to user failed");
		return -EFAULT;
	}

	kfree(out);
	kfree(generic_stats);
	return 0;
}

static int ipa_get_clock_stats(unsigned long arg)
{
	struct ipa_lnx_clock_stats *clock_stats;
	int i;
	int alloc_size;
	struct pm_client_stats *pm_stats_ptr;

	alloc_size = sizeof(struct ipa_lnx_clock_stats) +
		(sizeof(struct pm_client_stats) *
		ipa_lnx_agent_ctx.alloc_info.num_pm_clients);

	clock_stats = (struct ipa_lnx_clock_stats *) memdup_user((
		const void __user *)arg, alloc_size);
	if (IS_ERR(clock_stats)) {
		IPA_STATS_ERR("copy from user failed\n");
		return -ENOMEM;
	}

	if(ipa_pm_get_scaling_bw_levels(clock_stats))
		IPA_STATS_ERR("Couldn't get scaling bw levels\n");
	clock_stats->aggr_bw =
		ipa_pm_get_aggregated_throughput();
	clock_stats->curr_clk_vote = ipa_pm_get_current_clk_vote();
	clock_stats->active_clients = 0;

	pm_stats_ptr = &clock_stats->pm_clnt_stats[0];
	for (i = 1; i < ipa_lnx_agent_ctx.alloc_info.num_pm_clients; i++) {
		if (ipa_get_pm_client_stats_filled(pm_stats_ptr, i)) {
			clock_stats->active_clients++;
			pm_stats_ptr = (struct pm_client_stats *)((uint64_t)pm_stats_ptr +
				sizeof(struct pm_client_stats));
		}
	}

	if(copy_to_user((void __user *)arg,
		(u8 *)clock_stats,
		alloc_size)) {
		kfree(clock_stats);
		IPA_STATS_ERR("copy to user failed");
		return -EFAULT;
	}

	kfree(clock_stats);
	return 0;
}

/**
 * ipa_get_gsi_pipe_info - API to fill gsi pipe info
 */
static void ipa_get_gsi_pipe_info(
	struct ipa_lnx_pipe_info *pipe_info_ptr_local, struct ipa3_ep_context *ep)
{
	const struct ipa_gsi_ep_config *gsi_ep_info;

	pipe_info_ptr_local->client_type = ep->client;
	if (ep->sys) {
		pipe_info_ptr_local->buff_size = ep->sys->buff_size;
		pipe_info_ptr_local->is_common_evt_ring =
			ep->sys->use_comm_evt_ring;
	}
	pipe_info_ptr_local->direction = IPA_CLIENT_IS_CONS(ep->client);
	pipe_info_ptr_local->num_free_buff = 0;
	pipe_info_ptr_local->gsi_chan_num = ep->gsi_chan_hdl;
	pipe_info_ptr_local->gsi_evt_num = ep->gsi_evt_ring_hdl;

	pipe_info_ptr_local->gsi_prot_type =
		gsi_get_chan_prot_type(ep->gsi_chan_hdl);
	pipe_info_ptr_local->gsi_chan_state =
		gsi_get_chan_state(ep->gsi_chan_hdl);
	pipe_info_ptr_local->gsi_chan_stop_stm =
		gsi_get_chan_stop_stm(ep->gsi_chan_hdl, gsi_get_peripheral_ee());
	pipe_info_ptr_local->gsi_poll_mode =
		gsi_get_chan_poll_mode(ep->gsi_chan_hdl);
	pipe_info_ptr_local->gsi_chan_ring_len =
		gsi_get_ring_len(ep->gsi_chan_hdl);
	pipe_info_ptr_local->gsi_db_in_bytes =
		gsi_get_chan_props_db_in_bytes(ep->gsi_chan_hdl);
	pipe_info_ptr_local->gsi_chan_ring_bp =
		gsi_read_chan_ring_bp(ep->gsi_chan_hdl);
	pipe_info_ptr_local->gsi_chan_ring_rp =
		gsi_read_chan_ring_rp(ep->gsi_chan_hdl, gsi_get_peripheral_ee());
	pipe_info_ptr_local->gsi_chan_ring_wp =
		gsi_read_chan_ring_wp(ep->gsi_chan_hdl, gsi_get_peripheral_ee());

	gsi_ep_info = ipa3_get_gsi_ep_info(ep->client);
	pipe_info_ptr_local->gsi_ipa_if_tlv =
		gsi_ep_info ? gsi_ep_info->ipa_if_tlv : 0;
	pipe_info_ptr_local->gsi_ipa_if_aos =
		gsi_ep_info ? gsi_ep_info->ipa_if_aos : 0;

	pipe_info_ptr_local->gsi_desc_size =
		gsi_get_evt_ring_re_size(ep->gsi_evt_ring_hdl);
	pipe_info_ptr_local->gsi_evt_ring_len =
		gsi_get_evt_ring_len(ep->gsi_evt_ring_hdl);
	pipe_info_ptr_local->gsi_evt_ring_bp =
		gsi_read_event_ring_bp(ep->gsi_evt_ring_hdl);
	pipe_info_ptr_local->gsi_evt_ring_rp =
		gsi_get_evt_ring_rp(ep->gsi_evt_ring_hdl);
	pipe_info_ptr_local->gsi_evt_ring_wp =
		gsi_read_event_ring_wp(ep->gsi_evt_ring_hdl, gsi_get_peripheral_ee());
}

/**
 * ipa_lnx_calculate_gsi_ring_summay - API to calculate gsi ring summary
 * GSI tx_summary and rx_summary are calculated based on the difference between
 * the previous poll and the current poll. Both summaries are on a scale of 100
 * and will be rated based upon number of credits left(tx) or number or packets
 * filled(rx). 100 value being efficient and 0 being non efficient/stall/IPA idle
 */
static void ipa_lnx_calculate_gsi_ring_summay(
	struct ipa_lnx_gsi_tx_debug_stats *tx_instance_ptr_local,
	struct ipa_lnx_gsi_rx_debug_stats *rx_instance_ptr_local,
	int client_type)
{
	uint32_t diff_100_perc_cred;
	uint32_t diff_0_perc_cred;
	uint32_t diff_75_perc_cred;
	uint32_t diff_50_perc_cred;
	uint32_t diff_25_perc_cred;
	uint32_t diff_tx_polled;
	uint32_t diff_100_perc_pack;
	uint32_t diff_0_perc_pack;
	uint32_t diff_75_perc_pack;
	uint32_t diff_50_perc_pack;
	uint32_t diff_25_perc_pack;
	uint32_t diff_rx_polled;

	if (IPA_CLIENT_IS_CONS(client_type) && tx_instance_ptr_local) {
		if (tx_instance_ptr_local->num_tx_ring_100_perc_with_cred >=
			poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_100_perc_with_cred)
			diff_100_perc_cred =
				tx_instance_ptr_local->num_tx_ring_100_perc_with_cred -
				poll_pack_and_cred_info[
				client_type].tx_cred_info.num_tx_ring_100_perc_with_cred;
		else diff_100_perc_cred = (0xFFFFFFFF - poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_100_perc_with_cred) +
			tx_instance_ptr_local->num_tx_ring_100_perc_with_cred;
		if (tx_instance_ptr_local->num_tx_ring_above_75_perc_cred >=
			poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_above_75_perc_cred)
			diff_75_perc_cred =
				tx_instance_ptr_local->num_tx_ring_above_75_perc_cred -
				poll_pack_and_cred_info[
					client_type].tx_cred_info.num_tx_ring_above_75_perc_cred;
		else diff_75_perc_cred = (0xFFFFFFFF - poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_above_75_perc_cred) +
			tx_instance_ptr_local->num_tx_ring_above_75_perc_cred;
		if (tx_instance_ptr_local->num_tx_ring_above_25_perc_cred >=
			poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_above_25_perc_cred)
			diff_25_perc_cred =
				tx_instance_ptr_local->num_tx_ring_above_25_perc_cred -
				poll_pack_and_cred_info[
					client_type].tx_cred_info.num_tx_ring_above_25_perc_cred;
		else diff_25_perc_cred = (0xFFFFFFFF - poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_above_25_perc_cred) +
			tx_instance_ptr_local->num_tx_ring_above_25_perc_cred;
		if (tx_instance_ptr_local->num_tx_ring_0_perc_with_cred >=
			poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_0_perc_with_cred)
			diff_0_perc_cred =
				tx_instance_ptr_local->num_tx_ring_0_perc_with_cred -
				poll_pack_and_cred_info[
					client_type].tx_cred_info.num_tx_ring_0_perc_with_cred;
		else diff_0_perc_cred = (0xFFFFFFFF - poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_0_perc_with_cred) +
			tx_instance_ptr_local->num_tx_ring_0_perc_with_cred;
		if (tx_instance_ptr_local->num_tx_ring_stats_polled >=
			poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_stats_polled)
			diff_tx_polled =
				tx_instance_ptr_local->num_tx_ring_stats_polled -
				poll_pack_and_cred_info[
					client_type].tx_cred_info.num_tx_ring_stats_polled;
		else diff_tx_polled = (0xFFFFFFFF - poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_stats_polled) +
			tx_instance_ptr_local->num_tx_ring_stats_polled;

		poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_100_perc_with_cred =
			tx_instance_ptr_local->num_tx_ring_100_perc_with_cred;
		poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_0_perc_with_cred =
			tx_instance_ptr_local->num_tx_ring_0_perc_with_cred;
		poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_above_75_perc_cred =
			tx_instance_ptr_local->num_tx_ring_above_75_perc_cred;
		poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_above_25_perc_cred =
			tx_instance_ptr_local->num_tx_ring_above_25_perc_cred;
		poll_pack_and_cred_info[
			client_type].tx_cred_info.num_tx_ring_stats_polled =
			tx_instance_ptr_local->num_tx_ring_stats_polled;

		diff_50_perc_cred = diff_tx_polled - (diff_100_perc_cred +
			diff_75_perc_cred + diff_25_perc_cred + diff_0_perc_cred);
		/**
		 * TX ring scale(summary) - Varies between 0 to 100
		 * If the value tends towards 0, we can assume following things
		 *	1. DL throughput increasing or
		 *	2. Peripheral not pulling data fast enough.
		 *
		 * If the value tends towards 100, we can assume following things
		 *	1. Client processing data speed increasing or
		 *	2. Equal to 100 when no DL data transfer
		 */
		tx_instance_ptr_local->tx_summary = ((diff_100_perc_cred * 100) +
			(diff_75_perc_cred * 75) +
			(diff_50_perc_cred * 50) +
			(diff_25_perc_cred * 25) +
			(diff_0_perc_cred * 0))/diff_tx_polled;
	} else if(!IPA_CLIENT_IS_CONS(client_type) && rx_instance_ptr_local) {
		if (rx_instance_ptr_local->num_rx_ring_100_perc_with_pack >=
			poll_pack_and_cred_info[
				client_type].rx_pack_info.num_rx_ring_100_perc_with_pack)
			diff_100_perc_pack =
				rx_instance_ptr_local->num_rx_ring_100_perc_with_pack -
				poll_pack_and_cred_info[
					client_type].rx_pack_info.num_rx_ring_100_perc_with_pack;
		else diff_100_perc_pack = (0xFFFFFFFF - poll_pack_and_cred_info[
				client_type].rx_pack_info.num_rx_ring_100_perc_with_pack) +
				rx_instance_ptr_local->num_rx_ring_100_perc_with_pack;
		if (rx_instance_ptr_local->num_rx_ring_above_75_perc_pack >=
			poll_pack_and_cred_info[
				client_type].rx_pack_info.num_rx_ring_above_75_perc_pack)
			diff_75_perc_pack =
				rx_instance_ptr_local->num_rx_ring_above_75_perc_pack -
				poll_pack_and_cred_info[
					client_type].rx_pack_info.num_rx_ring_above_75_perc_pack;
		else diff_75_perc_pack = (0xFFFFFFFF - poll_pack_and_cred_info[
			client_type].rx_pack_info.num_rx_ring_above_75_perc_pack) +
			rx_instance_ptr_local->num_rx_ring_above_75_perc_pack;
		if (rx_instance_ptr_local->num_rx_ring_above_25_perc_pack >=
			poll_pack_and_cred_info[
				client_type].rx_pack_info.num_rx_ring_above_25_perc_pack)
			diff_25_perc_pack =
			rx_instance_ptr_local->num_rx_ring_above_25_perc_pack -
			poll_pack_and_cred_info[
				client_type].rx_pack_info.num_rx_ring_above_25_perc_pack;
		else diff_25_perc_pack = (0xFFFFFFFF - poll_pack_and_cred_info[
			client_type].rx_pack_info.num_rx_ring_above_25_perc_pack) +
			rx_instance_ptr_local->num_rx_ring_above_25_perc_pack;
		if (rx_instance_ptr_local->num_rx_ring_0_perc_with_pack >=
			poll_pack_and_cred_info[
				client_type].rx_pack_info.num_rx_ring_0_perc_with_pack)
			diff_0_perc_pack =
				rx_instance_ptr_local->num_rx_ring_0_perc_with_pack -
				poll_pack_and_cred_info[
					client_type].rx_pack_info.num_rx_ring_0_perc_with_pack;
		else diff_0_perc_pack = (0xFFFFFFFF - poll_pack_and_cred_info[
			client_type].rx_pack_info.num_rx_ring_0_perc_with_pack) +
			rx_instance_ptr_local->num_rx_ring_0_perc_with_pack;
		if (rx_instance_ptr_local->num_rx_ring_stats_polled >=
			poll_pack_and_cred_info[
				client_type].rx_pack_info.num_rx_ring_stats_polled)
			diff_rx_polled =
				rx_instance_ptr_local->num_rx_ring_stats_polled -
				poll_pack_and_cred_info[
					client_type].rx_pack_info.num_rx_ring_stats_polled;
		else diff_rx_polled = (0xFFFFFFFF - poll_pack_and_cred_info[
			client_type].rx_pack_info.num_rx_ring_stats_polled) +
			rx_instance_ptr_local->num_rx_ring_stats_polled;

		poll_pack_and_cred_info[
			client_type].rx_pack_info.num_rx_ring_100_perc_with_pack =
				rx_instance_ptr_local->num_rx_ring_100_perc_with_pack;
		poll_pack_and_cred_info[
			client_type].rx_pack_info.num_rx_ring_0_perc_with_pack =
				rx_instance_ptr_local->num_rx_ring_0_perc_with_pack;
		poll_pack_and_cred_info[
			client_type].rx_pack_info.num_rx_ring_above_75_perc_pack =
				rx_instance_ptr_local->num_rx_ring_above_75_perc_pack;
		poll_pack_and_cred_info[
			client_type].rx_pack_info.num_rx_ring_above_25_perc_pack =
				rx_instance_ptr_local->num_rx_ring_above_25_perc_pack;
		poll_pack_and_cred_info[
			client_type].rx_pack_info.num_rx_ring_stats_polled =
				rx_instance_ptr_local->num_rx_ring_stats_polled;

		diff_50_perc_pack = diff_rx_polled - (diff_100_perc_pack +
			diff_75_perc_pack + diff_25_perc_pack + diff_0_perc_pack);
		/**
		 * RX ring scale(summary) - Varies between 0 to 100
		 * If the value tends towards 0, we can assume following things
		 *	1. UL throughput is increasing or
		 *	2. IPA packet processing speed decreasing or
		 * 	3. Q6 packet pulling speed decreasing or
		 *	4. A7 packet pulling speed decreasing (Lan2Lan)
		 *
		 * If the value tends towards 100, we can assume following things
		 *	1. IPA processing data speed increasing or
		 *	2. Peripheral data pushing speed decreasing or
		 *	3. Equal to 100 during no UL data transfer
		 */
		rx_instance_ptr_local->rx_summary = ((diff_100_perc_pack * 0) +
			(diff_75_perc_pack * 25) +
			(diff_50_perc_pack * 50) +
			(diff_25_perc_pack * 75) +
			(diff_0_perc_pack * 100))/diff_rx_polled;
	}
}

static int ipa_get_wlan_inst_stats(unsigned long arg)
{
	struct ipa_lnx_wlan_inst_stats *wlan_stats;
	int i, j;
	int alloc_size;
	int ep_idx;
	int client_type;
	struct ipa_lnx_pipe_info *pipe_info_ptr = NULL;
	struct ipa_lnx_gsi_tx_debug_stats *tx_instance_ptr = NULL;
	struct ipa_lnx_gsi_rx_debug_stats *rx_instance_ptr = NULL;
	struct ipa_lnx_pipe_info *pipe_info_ptr_local = NULL;
	struct ipa_lnx_gsi_tx_debug_stats *tx_instance_ptr_local = NULL;
	struct ipa_lnx_gsi_rx_debug_stats *rx_instance_ptr_local = NULL;
	struct wlan_instance_info *instance_ptr = NULL;
	struct ipa_uc_dbg_ring_stats stats;

	alloc_size = sizeof(struct ipa_lnx_wlan_inst_stats) +
			(ipa_lnx_agent_ctx.alloc_info.num_wlan_instances *
			sizeof(struct wlan_instance_info));
	for (i = 0; i < ipa_lnx_agent_ctx.alloc_info.num_wlan_instances; i++) {
		alloc_size = alloc_size +
			(ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[i].num_tx_instances *
			sizeof(struct ipa_lnx_gsi_tx_debug_stats)) +
			(ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[i].num_rx_instances *
			sizeof(struct ipa_lnx_gsi_rx_debug_stats)) +
			(ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[i].num_pipes *
			sizeof(struct ipa_lnx_pipe_info));
	}

	wlan_stats = (struct ipa_lnx_wlan_inst_stats *) memdup_user((
		const void __user *)arg, alloc_size);
	if (IS_ERR(wlan_stats)) {
		IPA_STATS_ERR("copy from user failed");
		return -ENOMEM;
	}

	if (!ipa_lnx_agent_ctx.alloc_info.num_wlan_instances)
		goto success;
	wlan_stats->num_wlan_instance =
		ipa_lnx_agent_ctx.alloc_info.num_wlan_instances;

	instance_ptr = &wlan_stats->instance_info[0];
	for (i = 0; i < wlan_stats->num_wlan_instance; i++) {
		instance_ptr->num_pipes =
			ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[i].num_pipes;
		instance_ptr->gsi_debug_stats.num_tx_instances =
			ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[i].num_tx_instances;
		instance_ptr->gsi_debug_stats.num_rx_instances =
			ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[i].num_rx_instances;
		if(ipa3_get_wdi3_gsi_stats(&stats)) {
			instance_ptr = (struct wlan_instance_info *)((
				uint64_t)instance_ptr + sizeof(struct wlan_instance_info) +
				(instance_ptr->gsi_debug_stats.num_tx_instances *
				sizeof(struct ipa_lnx_gsi_tx_debug_stats)) +
				(instance_ptr->gsi_debug_stats.num_rx_instances *
				sizeof(struct ipa_lnx_gsi_rx_debug_stats)) +
				(instance_ptr->num_pipes * sizeof(struct ipa_lnx_pipe_info)));
			continue;
		}
		instance_ptr->instance_id = i;
		instance_ptr->wdi_ver = ipa_get_wdi_version();
		instance_ptr->wlan_mode = ipa_get_wlan_intf_mode();
		instance_ptr->wdi_over_gsi = ipa3_ctx->ipa_wdi3_over_gsi;
		instance_ptr->dbs_mode = ipa_wdi_is_tx1_used();
		instance_ptr->pm_bandwidth =
			ipa_pm_get_pm_clnt_throughput(
				ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[
				i].tx_inst_client_type[0]);

		tx_instance_ptr = (struct ipa_lnx_gsi_tx_debug_stats *)((
			uint64_t)instance_ptr + sizeof(struct wlan_instance_info));
		for (j = 0; j < ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[
			i].num_tx_instances; j++) {
			tx_instance_ptr_local = (struct ipa_lnx_gsi_tx_debug_stats *)((
				uint64_t)tx_instance_ptr + (j *
				sizeof(struct ipa_lnx_gsi_tx_debug_stats)));

			client_type = ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[
				i].tx_inst_client_type[j];
			tx_instance_ptr_local->tx_client = client_type;
			tx_instance_ptr_local->num_tx_ring_100_perc_with_cred =
				stats.u.ring[1 + j].ringFull;
			tx_instance_ptr_local->num_tx_ring_0_perc_with_cred =
				stats.u.ring[1 + j].ringEmpty;
			tx_instance_ptr_local->num_tx_ring_above_75_perc_cred =
				stats.u.ring[1 + j].ringUsageHigh;
			tx_instance_ptr_local->num_tx_ring_above_25_perc_cred =
				stats.u.ring[1 + j].ringUsageLow;
			tx_instance_ptr_local->num_tx_ring_stats_polled =
				stats.u.ring[1 + j].RingUtilCount;
			ipa_lnx_calculate_gsi_ring_summay(
				tx_instance_ptr_local, NULL, client_type);

			/* Currently reserved until GSI needs anything in future */
			tx_instance_ptr_local->num_tx_oob = 0;
			tx_instance_ptr_local->num_tx_oob_time = 0;
			tx_instance_ptr_local->gsi_debug1 = 0;
			tx_instance_ptr_local->gsi_debug2 = 0;
			tx_instance_ptr_local->gsi_debug3 = 0;
			tx_instance_ptr_local->gsi_debug4 = 0;
		}

		rx_instance_ptr = (struct ipa_lnx_gsi_rx_debug_stats *)((
			uint64_t)instance_ptr + sizeof(struct wlan_instance_info)
			+ (sizeof(struct ipa_lnx_gsi_tx_debug_stats) * (
			ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[
				i].num_tx_instances)));
		for (j = 0; j < ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[
			i].num_rx_instances; j++) {
			rx_instance_ptr_local = (struct ipa_lnx_gsi_rx_debug_stats *)((
				uint64_t)rx_instance_ptr + (j *
				sizeof(struct ipa_lnx_gsi_rx_debug_stats)));

			client_type =
				ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[
					i].rx_inst_client_type[j];
			rx_instance_ptr_local->rx_client =
				client_type;
			rx_instance_ptr_local->num_rx_ring_100_perc_with_pack =
				stats.u.ring[j].ringFull;
			rx_instance_ptr_local->num_rx_ring_0_perc_with_pack =
				stats.u.ring[j].ringEmpty;
			rx_instance_ptr_local->num_rx_ring_above_75_perc_pack =
				stats.u.ring[j].ringUsageHigh;
			rx_instance_ptr_local->num_rx_ring_above_25_perc_pack =
				stats.u.ring[j].ringUsageLow;
			rx_instance_ptr_local->num_rx_ring_stats_polled =
				stats.u.ring[j].RingUtilCount;
			rx_instance_ptr_local->num_rx_drop_stats = 0;
			ipa_lnx_calculate_gsi_ring_summay(
				NULL, rx_instance_ptr_local, client_type);

			/* Currently reserved until GSI needs anything in future */
			rx_instance_ptr_local->gsi_debug1 = 0;
			rx_instance_ptr_local->gsi_debug2 = 0;
			rx_instance_ptr_local->gsi_debug3 = 0;
			rx_instance_ptr_local->gsi_debug4 = 0;
		}

		pipe_info_ptr = (struct ipa_lnx_pipe_info *)((uint64_t)instance_ptr +
			sizeof(struct wlan_instance_info)
			+ (sizeof(struct ipa_lnx_gsi_tx_debug_stats) *
			(ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[i].num_tx_instances))
			+ (sizeof(struct ipa_lnx_gsi_rx_debug_stats) *
			(ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[
				i].num_rx_instances)));
		for (j = 0; j < instance_ptr->num_pipes; j++) {
			pipe_info_ptr_local = (struct ipa_lnx_pipe_info *)((
				uint64_t)pipe_info_ptr +
				(j * sizeof(struct ipa_lnx_pipe_info)));

			ep_idx = ipa3_get_ep_mapping(
				ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[
				i].pipes_client_type[j]);
			if (ep_idx == -1) {
				kfree(wlan_stats);
				return -EFAULT;
			}
			pipe_info_ptr_local->pipe_num = ep_idx;
			ipa_get_gsi_pipe_info(
				pipe_info_ptr_local, &ipa3_ctx->ep[ep_idx]);
		}

		instance_ptr = (struct wlan_instance_info *)((uint64_t)pipe_info_ptr +
			(sizeof(struct ipa_lnx_pipe_info) * (instance_ptr->num_pipes)));
	}

success:
	if(copy_to_user((void __user *)arg,
		(u8 *)wlan_stats,
		alloc_size)) {
		IPA_STATS_ERR("copy to user failed");
		kfree(wlan_stats);
		return -EFAULT;
	}

	kfree(wlan_stats);
	return 0;
}

static int ipa_get_eth_inst_stats(unsigned long arg)
{
	struct ipa_lnx_eth_inst_stats *eth_stats;
	int i, j;
	int alloc_size;
	int ep_idx;
	int client_type;
	struct ipa_lnx_pipe_info *pipe_info_ptr = NULL;
	struct ipa_lnx_gsi_tx_debug_stats *tx_instance_ptr = NULL;
	struct ipa_lnx_gsi_rx_debug_stats *rx_instance_ptr = NULL;
	struct ipa_lnx_pipe_info *pipe_info_ptr_local = NULL;
	struct ipa_lnx_gsi_tx_debug_stats *tx_instance_ptr_local = NULL;
	struct ipa_lnx_gsi_rx_debug_stats *rx_instance_ptr_local = NULL;
	struct eth_instance_info *instance_ptr = NULL;
	struct ipa_uc_dbg_ring_stats stats;

	alloc_size = sizeof(struct ipa_lnx_eth_inst_stats) +
			(ipa_lnx_agent_ctx.alloc_info.num_eth_instances *
				sizeof(struct eth_instance_info));
	for (i = 0; i < ipa_lnx_agent_ctx.alloc_info.num_eth_instances; i++) {
		alloc_size = alloc_size +
			(ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_tx_instances
			* sizeof(struct ipa_lnx_gsi_tx_debug_stats)) +
			(ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_rx_instances
			* sizeof(struct ipa_lnx_gsi_rx_debug_stats)) +
			(ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_pipes
			* sizeof(struct ipa_lnx_pipe_info));
	}

	eth_stats = (struct ipa_lnx_eth_inst_stats *) memdup_user((
		const void __user *)arg, alloc_size);
	if (IS_ERR(eth_stats)) {
		IPA_STATS_ERR("copy from user failed");
		return -ENOMEM;
	}

	eth_stats->num_eth_instance =
		ipa_lnx_agent_ctx.alloc_info.num_eth_instances;
	if (!ipa_lnx_agent_ctx.alloc_info.num_eth_instances)
		goto success;

	instance_ptr = &eth_stats->instance_info[0];
	for (i = 0; i < eth_stats->num_eth_instance; i++) {
		instance_ptr->instance_id = i;
		instance_ptr->num_pipes =
			ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_pipes;
		instance_ptr->gsi_debug_stats.num_tx_instances =
			ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_tx_instances;
		instance_ptr->gsi_debug_stats.num_rx_instances =
			ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_rx_instances;

		tx_instance_ptr = (struct ipa_lnx_gsi_tx_debug_stats *)((
			uint64_t)instance_ptr + sizeof(struct eth_instance_info));
		for (j = 0; j < ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
			i].num_tx_instances; j++) {
			tx_instance_ptr_local = (struct ipa_lnx_gsi_tx_debug_stats *)((
				uint64_t)tx_instance_ptr + (j *
				sizeof(struct ipa_lnx_gsi_tx_debug_stats)));

			/* Eth mode is sent in the tx_inst_client_type variable only */
			instance_ptr->eth_mode =
				ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
					i].tx_inst_client_type[j];
			if (instance_ptr->eth_mode == IPA_ETH_CLIENT_AQC107 ||
				instance_ptr->eth_mode == IPA_ETH_CLIENT_AQC113 ||
				instance_ptr->eth_mode == IPA_ETH_CLIENT_NTN ||
				instance_ptr->eth_mode == IPA_ETH_CLIENT_EMAC) {

				if(instance_ptr->eth_mode == IPA_ETH_CLIENT_NTN) {
					if(ipa3_get_ntn_gsi_stats(&stats)) {
						instance_ptr = (struct eth_instance_info *)((
							uint64_t)instance_ptr +
							sizeof(struct eth_instance_info) +
							(instance_ptr->gsi_debug_stats.num_tx_instances *
							sizeof(struct ipa_lnx_gsi_tx_debug_stats)) +
							(instance_ptr->gsi_debug_stats.num_rx_instances *
							sizeof(struct ipa_lnx_gsi_rx_debug_stats)) +
							(instance_ptr->num_pipes *
							sizeof(struct ipa_lnx_pipe_info)));
						continue;
					}
				} else {
					if(ipa3_get_aqc_gsi_stats(&stats)) {
						instance_ptr = (struct eth_instance_info *)((
							uint64_t)instance_ptr +
							sizeof(struct eth_instance_info) +
							(instance_ptr->gsi_debug_stats.num_tx_instances *
							sizeof(struct ipa_lnx_gsi_tx_debug_stats)) +
							(instance_ptr->gsi_debug_stats.num_rx_instances *
							sizeof(struct ipa_lnx_gsi_rx_debug_stats)) +
							(instance_ptr->num_pipes *
							sizeof(struct ipa_lnx_pipe_info)));
						continue;
					}
				}

				if (instance_ptr->eth_mode == IPA_ETH_CLIENT_NTN ||
					instance_ptr->eth_mode == IPA_ETH_CLIENT_EMAC)
					tx_instance_ptr_local->tx_client =
						IPA_CLIENT_ETHERNET_CONS;
				else tx_instance_ptr_local->tx_client =
						IPA_CLIENT_AQC_ETHERNET_CONS;
				client_type = tx_instance_ptr_local->tx_client;
				instance_ptr->pm_bandwidth =
					ipa_pm_get_pm_clnt_throughput(client_type);
				tx_instance_ptr_local->num_tx_ring_100_perc_with_cred =
					stats.u.ring[1].ringFull;
				tx_instance_ptr_local->num_tx_ring_0_perc_with_cred =
					stats.u.ring[1].ringEmpty;
				tx_instance_ptr_local->num_tx_ring_above_75_perc_cred =
					stats.u.ring[1].ringUsageHigh;
				tx_instance_ptr_local->num_tx_ring_above_25_perc_cred =
					stats.u.ring[1].ringUsageLow;
				tx_instance_ptr_local->num_tx_ring_stats_polled =
					stats.u.ring[1].RingUtilCount;
				ipa_lnx_calculate_gsi_ring_summay(
					tx_instance_ptr_local, NULL, client_type);

				/* Currently reserved until GSI needs anything in future */
				tx_instance_ptr_local->num_tx_oob = 0;
				tx_instance_ptr_local->num_tx_oob_time = 0;
				tx_instance_ptr_local->gsi_debug1 = 0;
				tx_instance_ptr_local->gsi_debug2 = 0;
				tx_instance_ptr_local->gsi_debug3 = 0;
				tx_instance_ptr_local->gsi_debug4 = 0;
			} else if (instance_ptr->eth_mode == IPA_ETH_CLIENT_RTK8111K ||
				instance_ptr->eth_mode == IPA_ETH_CLIENT_RTK8125B) {

				if(ipa3_get_rtk_gsi_stats(&stats)) {
					instance_ptr = (struct eth_instance_info *)((
						uint64_t)instance_ptr +
						sizeof(struct eth_instance_info) +
						(instance_ptr->gsi_debug_stats.num_tx_instances *
						sizeof(struct ipa_lnx_gsi_tx_debug_stats)) +
						(instance_ptr->gsi_debug_stats.num_rx_instances *
						sizeof(struct ipa_lnx_gsi_rx_debug_stats)) +
						(instance_ptr->num_pipes *
						sizeof(struct ipa_lnx_pipe_info)));
					continue;
				}
				client_type = IPA_CLIENT_RTK_ETHERNET_CONS;
				instance_ptr->pm_bandwidth =
					ipa_pm_get_pm_clnt_throughput(client_type);
				tx_instance_ptr_local->tx_client = client_type;
				tx_instance_ptr_local->num_tx_ring_100_perc_with_cred =
					stats.u.rtk[1].commStats.ringFull;
				tx_instance_ptr_local->num_tx_ring_0_perc_with_cred =
					stats.u.rtk[1].commStats.ringEmpty;
				tx_instance_ptr_local->num_tx_ring_above_75_perc_cred =
					stats.u.rtk[1].commStats.ringUsageHigh;
				tx_instance_ptr_local->num_tx_ring_above_25_perc_cred =
					stats.u.rtk[1].commStats.ringUsageLow;
				tx_instance_ptr_local->num_tx_ring_stats_polled =
					stats.u.rtk[1].commStats.RingUtilCount;
				ipa_lnx_calculate_gsi_ring_summay(
					tx_instance_ptr_local, NULL, client_type);

				/* Currently reserved until GSI needs anything in future */
				tx_instance_ptr_local->num_tx_oob = 0;
				tx_instance_ptr_local->num_tx_oob_time = 0;
				tx_instance_ptr_local->gsi_debug1 = 0;
				tx_instance_ptr_local->gsi_debug2 = 0;
				tx_instance_ptr_local->gsi_debug3 = 0;
				tx_instance_ptr_local->gsi_debug4 = 0;
			} else IPA_STATS_ERR("Eth tx client type not found");
		}

		rx_instance_ptr = (struct ipa_lnx_gsi_rx_debug_stats *)((
			uint64_t)instance_ptr + sizeof(struct eth_instance_info)
			+ (sizeof(struct ipa_lnx_gsi_tx_debug_stats) *
			(ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
			i].num_tx_instances)));
		for (j = 0; j < ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
			i].num_rx_instances; j++) {
			rx_instance_ptr_local = (struct ipa_lnx_gsi_rx_debug_stats *)((
				uint64_t)rx_instance_ptr + (j *
				sizeof(struct ipa_lnx_gsi_rx_debug_stats)));

			if ((instance_ptr->eth_mode == IPA_ETH_CLIENT_AQC107 ||
				instance_ptr->eth_mode == IPA_ETH_CLIENT_AQC113 ||
				instance_ptr->eth_mode == IPA_ETH_CLIENT_NTN ||
				instance_ptr->eth_mode == IPA_ETH_CLIENT_EMAC)) {

				if (instance_ptr->eth_mode == IPA_ETH_CLIENT_NTN ||
					instance_ptr->eth_mode == IPA_ETH_CLIENT_EMAC)
					rx_instance_ptr_local->rx_client =
					IPA_CLIENT_ETHERNET_PROD;
				else rx_instance_ptr_local->rx_client =
						IPA_CLIENT_AQC_ETHERNET_PROD;
				client_type = rx_instance_ptr_local->rx_client;
				rx_instance_ptr_local->num_rx_ring_100_perc_with_pack =
					stats.u.ring[0].ringFull;
				rx_instance_ptr_local->num_rx_ring_0_perc_with_pack =
					stats.u.ring[0].ringEmpty;
				rx_instance_ptr_local->num_rx_ring_above_75_perc_pack =
					stats.u.ring[0].ringUsageHigh;
				rx_instance_ptr_local->num_rx_ring_above_25_perc_pack =
					stats.u.ring[0].ringUsageLow;
				rx_instance_ptr_local->num_rx_ring_stats_polled =
					stats.u.ring[0].RingUtilCount;
				rx_instance_ptr_local->num_rx_drop_stats = 0;
				ipa_lnx_calculate_gsi_ring_summay(
					NULL, rx_instance_ptr_local, client_type);

				/* Currently reserved until GSI needs anything in future */
				rx_instance_ptr_local->gsi_debug1 = 0;
				rx_instance_ptr_local->gsi_debug2 = 0;
				rx_instance_ptr_local->gsi_debug3 = 0;
				rx_instance_ptr_local->gsi_debug4 = 0;
			} else if (instance_ptr->eth_mode == IPA_ETH_CLIENT_RTK8111K ||
				instance_ptr->eth_mode == IPA_ETH_CLIENT_RTK8125B) {

				client_type = IPA_CLIENT_RTK_ETHERNET_PROD;
				rx_instance_ptr_local->rx_client = client_type;
				rx_instance_ptr_local->num_rx_ring_100_perc_with_pack =
					stats.u.rtk[0].commStats.ringFull;
				rx_instance_ptr_local->num_rx_ring_0_perc_with_pack =
					stats.u.rtk[0].commStats.ringEmpty;
				rx_instance_ptr_local->num_rx_ring_above_75_perc_pack =
					stats.u.rtk[0].commStats.ringUsageHigh;
				rx_instance_ptr_local->num_rx_ring_above_25_perc_pack =
					stats.u.rtk[0].commStats.ringUsageLow;
				rx_instance_ptr_local->num_rx_ring_stats_polled =
					stats.u.rtk[0].commStats.RingUtilCount;
				rx_instance_ptr_local->num_rx_drop_stats = 0;
				ipa_lnx_calculate_gsi_ring_summay(
					NULL, rx_instance_ptr_local, client_type);

				/* Currently reserved until GSI needs anything in future */
				rx_instance_ptr_local->gsi_debug1 = 0;
				rx_instance_ptr_local->gsi_debug2 = 0;
				rx_instance_ptr_local->gsi_debug3 = 0;
				rx_instance_ptr_local->gsi_debug4 = 0;
			} else IPA_STATS_ERR("Eth rx client type not found");
		}

		pipe_info_ptr = (struct ipa_lnx_pipe_info *)((uint64_t)instance_ptr +
			sizeof(struct eth_instance_info)
			+ (sizeof(struct ipa_lnx_gsi_tx_debug_stats) *
			(ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_tx_instances))
			+ (sizeof(struct ipa_lnx_gsi_rx_debug_stats) *
			(ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
				i].num_rx_instances)));
		for (j = 0; j < instance_ptr->num_pipes; j++) {
			pipe_info_ptr_local = (struct ipa_lnx_pipe_info *)((
				uint64_t)pipe_info_ptr + (j *
				sizeof(struct ipa_lnx_pipe_info)));

			ep_idx = ipa3_get_ep_mapping(
				ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
					i].pipes_client_type[j]);
			if (ep_idx == -1) {
				kfree(eth_stats);
				return -EFAULT;
			}

			pipe_info_ptr_local->pipe_num = ep_idx;
			ipa_get_gsi_pipe_info(
				pipe_info_ptr_local, &ipa3_ctx->ep[ep_idx]);
		}

		instance_ptr = (struct eth_instance_info *)((
			uint64_t)pipe_info_ptr + (sizeof(struct ipa_lnx_pipe_info)
			* (instance_ptr->num_pipes)));
	}

success:
	if(copy_to_user((void __user *)arg,
		(u8 *)eth_stats,
		alloc_size)) {
		IPA_STATS_ERR("copy to user failed");
		kfree(eth_stats);
		return -EFAULT;
	}

	kfree(eth_stats);
	return 0;
}

static int ipa_get_usb_inst_stats(unsigned long arg)
{
	struct ipa_lnx_usb_inst_stats *usb_stats;
	int i, j;
	int alloc_size;
	int ep_idx;
	int client_type;
	struct ipa_lnx_pipe_info *pipe_info_ptr = NULL;
	struct ipa_lnx_gsi_tx_debug_stats *tx_instance_ptr = NULL;
	struct ipa_lnx_gsi_rx_debug_stats *rx_instance_ptr = NULL;
	struct ipa_lnx_pipe_info *pipe_info_ptr_local = NULL;
	struct ipa_lnx_gsi_tx_debug_stats *tx_instance_ptr_local = NULL;
	struct ipa_lnx_gsi_rx_debug_stats *rx_instance_ptr_local = NULL;
	struct usb_instance_info *instance_ptr = NULL;
	struct ipa_uc_dbg_ring_stats stats;

	alloc_size = sizeof(struct ipa_lnx_usb_inst_stats) +
			(ipa_lnx_agent_ctx.alloc_info.num_usb_instances *
				sizeof(struct usb_instance_info));
	for (i = 0; i < ipa_lnx_agent_ctx.alloc_info.num_usb_instances; i++) {
		alloc_size = alloc_size +
			(ipa_lnx_agent_ctx.alloc_info.usb_inst_info[i].num_tx_instances *
				sizeof(struct ipa_lnx_gsi_tx_debug_stats)) +
			(ipa_lnx_agent_ctx.alloc_info.usb_inst_info[i].num_rx_instances *
				sizeof(struct ipa_lnx_gsi_rx_debug_stats)) +
			(ipa_lnx_agent_ctx.alloc_info.usb_inst_info[i].num_pipes *
				sizeof(struct ipa_lnx_pipe_info));
	}

	usb_stats = (struct ipa_lnx_usb_inst_stats *) memdup_user((
		const void __user *)arg, alloc_size);
	if (IS_ERR(usb_stats)) {
		IPA_STATS_ERR("copy from user failed");
		return -ENOMEM;
	}

	usb_stats->num_usb_instance =
		ipa_lnx_agent_ctx.alloc_info.num_usb_instances;
	if (!ipa_lnx_agent_ctx.alloc_info.num_usb_instances)
		goto success;

	instance_ptr = &usb_stats->instance_info[0];
	for (i = 0; i < usb_stats->num_usb_instance; i++) {
		instance_ptr->instance_id = i;
		instance_ptr->usb_mode = ipa_lnx_agent_ctx.usb_teth_prot[i];
		instance_ptr->pm_bandwidth =
			ipa_pm_get_pm_clnt_throughput(ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
				i].tx_inst_client_type[0]);
		instance_ptr->num_pipes =
			ipa_lnx_agent_ctx.alloc_info.usb_inst_info[i].num_pipes;
		instance_ptr->gsi_debug_stats.num_tx_instances =
			ipa_lnx_agent_ctx.alloc_info.usb_inst_info[i].num_tx_instances;
		instance_ptr->gsi_debug_stats.num_rx_instances =
			ipa_lnx_agent_ctx.alloc_info.usb_inst_info[i].num_rx_instances;
		if(ipa3_get_usb_gsi_stats(&stats)) {
			instance_ptr = (struct usb_instance_info *)((uint64_t)instance_ptr
				+ sizeof(struct usb_instance_info) +
				(instance_ptr->gsi_debug_stats.num_tx_instances *
				sizeof(struct ipa_lnx_gsi_tx_debug_stats)) +
				(instance_ptr->gsi_debug_stats.num_rx_instances *
				sizeof(struct ipa_lnx_gsi_rx_debug_stats)) +
				(instance_ptr->num_pipes * sizeof(struct ipa_lnx_pipe_info)));
			continue;
		}

		tx_instance_ptr = (struct ipa_lnx_gsi_tx_debug_stats *)((
			uint64_t)instance_ptr + sizeof(struct usb_instance_info));
		for (j = 0; j < ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
			i].num_tx_instances; j++) {
			tx_instance_ptr_local = (struct ipa_lnx_gsi_tx_debug_stats *)((
				uint64_t)tx_instance_ptr + (j *
				sizeof(struct ipa_lnx_gsi_tx_debug_stats)));

			client_type = ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
				i].tx_inst_client_type[j];
			tx_instance_ptr_local->tx_client = client_type;
			tx_instance_ptr_local->num_tx_ring_100_perc_with_cred =
				stats.u.ring[1 + j].ringFull;
			tx_instance_ptr_local->num_tx_ring_0_perc_with_cred =
				stats.u.ring[1 + j].ringEmpty;
			tx_instance_ptr_local->num_tx_ring_above_75_perc_cred =
				stats.u.ring[1 + j].ringUsageHigh;
			tx_instance_ptr_local->num_tx_ring_above_25_perc_cred =
				stats.u.ring[1 + j].ringUsageLow;
			tx_instance_ptr_local->num_tx_ring_stats_polled =
				stats.u.ring[1 + j].RingUtilCount;
			ipa_lnx_calculate_gsi_ring_summay(
				tx_instance_ptr_local, NULL, client_type);

			/* Currently reserved until GSI needs anything in future */
			tx_instance_ptr_local->num_tx_oob = 0;
			tx_instance_ptr_local->num_tx_oob_time = 0;
			tx_instance_ptr_local->gsi_debug1 = 0;
			tx_instance_ptr_local->gsi_debug2 = 0;
			tx_instance_ptr_local->gsi_debug3 = 0;
			tx_instance_ptr_local->gsi_debug4 = 0;
		}

		rx_instance_ptr = (struct ipa_lnx_gsi_rx_debug_stats *) ((
			uint64_t)instance_ptr + sizeof(struct usb_instance_info)
			+ (sizeof(struct ipa_lnx_gsi_tx_debug_stats) *
			(ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
			i].num_tx_instances)));
		for (j = 0; j < ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
			i].num_rx_instances; j++) {
			rx_instance_ptr_local = (struct ipa_lnx_gsi_rx_debug_stats *)((
				uint64_t)rx_instance_ptr +
				(j * sizeof(struct ipa_lnx_gsi_rx_debug_stats)));

			client_type = ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
				i].rx_inst_client_type[j];
			rx_instance_ptr_local->rx_client = client_type;
			rx_instance_ptr_local->num_rx_ring_100_perc_with_pack =
				stats.u.ring[j].ringFull;
			rx_instance_ptr_local->num_rx_ring_0_perc_with_pack =
				stats.u.ring[j].ringEmpty;
			rx_instance_ptr_local->num_rx_ring_above_75_perc_pack =
				stats.u.ring[j].ringUsageHigh;
			rx_instance_ptr_local->num_rx_ring_above_25_perc_pack =
				stats.u.ring[j].ringUsageLow;
			rx_instance_ptr_local->num_rx_ring_stats_polled =
				stats.u.ring[j].RingUtilCount;
			rx_instance_ptr_local->num_rx_drop_stats = 0;
			ipa_lnx_calculate_gsi_ring_summay(
				NULL, rx_instance_ptr_local, client_type);

			/* Currently reserved until GSI needs anything in future */
			rx_instance_ptr_local->gsi_debug1 = 0;
			rx_instance_ptr_local->gsi_debug2 = 0;
			rx_instance_ptr_local->gsi_debug3 = 0;
			rx_instance_ptr_local->gsi_debug4 = 0;
		}

		pipe_info_ptr = (struct ipa_lnx_pipe_info *)((uint64_t)instance_ptr +
			sizeof(struct usb_instance_info)
			+ (sizeof(struct ipa_lnx_gsi_tx_debug_stats) *
			(ipa_lnx_agent_ctx.alloc_info.usb_inst_info[i].num_tx_instances))
			+ (sizeof(struct ipa_lnx_gsi_rx_debug_stats) *
			(ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
				i].num_rx_instances)));
		for (j = 0; j < instance_ptr->num_pipes; j++) {
			pipe_info_ptr_local = (struct ipa_lnx_pipe_info *)((
				uint64_t)pipe_info_ptr + (j *
				sizeof(struct ipa_lnx_pipe_info)));

			ep_idx = ipa3_get_ep_mapping(
				ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
					i].pipes_client_type[j]);
			if (ep_idx == -1) {
				kfree(usb_stats);
				return -EFAULT;
			}
			pipe_info_ptr_local->pipe_num = ep_idx;
			ipa_get_gsi_pipe_info(
				pipe_info_ptr_local, &ipa3_ctx->ep[ep_idx]);
		}

		instance_ptr = (struct usb_instance_info *) ((uint64_t)pipe_info_ptr +
			(sizeof(struct ipa_lnx_pipe_info) * (instance_ptr->num_pipes)));
	}

success:
	if(copy_to_user((void __user *)arg,
		(u8 *)usb_stats,
		alloc_size)) {
		IPA_STATS_ERR("copy to user failed");
		kfree(usb_stats);
		return -EFAULT;
	}

	kfree(usb_stats);
	return 0;
}

static int ipa_get_mhip_inst_stats(unsigned long arg)
{
	struct ipa_lnx_mhip_inst_stats *mhip_stats;
	int i, j;
	int alloc_size;
	int ep_idx;
	int client_type;
	struct ipa_lnx_pipe_info *pipe_info_ptr = NULL;
	struct ipa_lnx_gsi_tx_debug_stats *tx_instance_ptr = NULL;
	struct ipa_lnx_gsi_rx_debug_stats *rx_instance_ptr = NULL;
	struct ipa_lnx_pipe_info *pipe_info_ptr_local = NULL;
	struct ipa_lnx_gsi_tx_debug_stats *tx_instance_ptr_local = NULL;
	struct ipa_lnx_gsi_rx_debug_stats *rx_instance_ptr_local = NULL;
	struct mhip_instance_info *instance_ptr = NULL;
	struct ipa_uc_dbg_ring_stats stats;

	alloc_size = sizeof(struct ipa_lnx_mhip_inst_stats) +
			(ipa_lnx_agent_ctx.alloc_info.num_mhip_instances *
				sizeof(struct mhip_instance_info));
	for (i = 0; i < ipa_lnx_agent_ctx.alloc_info.num_mhip_instances; i++) {
		alloc_size = alloc_size +
			(ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[i].num_tx_instances *
				sizeof(struct ipa_lnx_gsi_tx_debug_stats)) +
			(ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[i].num_rx_instances *
				sizeof(struct ipa_lnx_gsi_rx_debug_stats)) +
			(ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[i].num_pipes *
				sizeof(struct ipa_lnx_pipe_info));
	}

	mhip_stats = (struct ipa_lnx_mhip_inst_stats *) memdup_user((
		const void __user *)arg, alloc_size);
	if (IS_ERR(mhip_stats)) {
		IPA_STATS_ERR("copy from user failed");
		return -ENOMEM;
	}

	if (!ipa_lnx_agent_ctx.alloc_info.num_mhip_instances)
		goto success;
	mhip_stats->num_mhip_instance =
		ipa_lnx_agent_ctx.alloc_info.num_mhip_instances;

	instance_ptr = &mhip_stats->instance_info[0];
	for (i = 0; i < mhip_stats->num_mhip_instance; i++) {
		instance_ptr->instance_id = i;
		instance_ptr->mhip_mode =
			ipa_lnx_agent_ctx.usb_teth_prot[i];
		instance_ptr->pm_bandwidth =
			ipa_pm_get_pm_clnt_throughput(
				ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[
				i].tx_inst_client_type[0]);
		instance_ptr->num_pipes =
			ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[i].num_pipes;
		instance_ptr->gsi_debug_stats.num_tx_instances =
			ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[i].num_tx_instances;
		instance_ptr->gsi_debug_stats.num_rx_instances =
			ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[i].num_rx_instances;
		if(ipa3_get_mhip_gsi_stats(&stats)) {
			instance_ptr = (struct mhip_instance_info *)((
				uint64_t)instance_ptr + sizeof(struct mhip_instance_info) +
				(instance_ptr->gsi_debug_stats.num_tx_instances *
					sizeof(struct ipa_lnx_gsi_tx_debug_stats)) +
				(instance_ptr->gsi_debug_stats.num_rx_instances *
					sizeof(struct ipa_lnx_gsi_rx_debug_stats)) +
				(instance_ptr->num_pipes * sizeof(struct ipa_lnx_pipe_info)));
			continue;
		}

		tx_instance_ptr = (struct ipa_lnx_gsi_tx_debug_stats *)((
			uint64_t)instance_ptr + sizeof(struct mhip_instance_info));
		for (j = 0; j < ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[
			i].num_tx_instances; j++) {
			tx_instance_ptr_local = (struct ipa_lnx_gsi_tx_debug_stats *)((
				uint64_t)tx_instance_ptr + (j *
				sizeof(struct ipa_lnx_gsi_tx_debug_stats)));

			client_type = ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[
				i].tx_inst_client_type[j];
			tx_instance_ptr_local->tx_client = client_type;
			tx_instance_ptr_local->num_tx_ring_100_perc_with_cred =
				stats.u.ring[1 + (j*2)].ringFull;
			tx_instance_ptr_local->num_tx_ring_0_perc_with_cred =
				stats.u.ring[1 + (j*2)].ringEmpty;
			tx_instance_ptr_local->num_tx_ring_above_75_perc_cred =
				stats.u.ring[1 + (j*2)].ringUsageHigh;
			tx_instance_ptr_local->num_tx_ring_above_25_perc_cred =
				stats.u.ring[1 + (j*2)].ringUsageLow;
			tx_instance_ptr_local->num_tx_ring_stats_polled =
				stats.u.ring[1 + (j*2)].RingUtilCount;
			ipa_lnx_calculate_gsi_ring_summay(
				tx_instance_ptr_local, NULL, client_type);

			/* Currently reserved until GSI needs anything in future */
			tx_instance_ptr_local->num_tx_oob = 0;
			tx_instance_ptr_local->num_tx_oob_time = 0;
			tx_instance_ptr_local->gsi_debug1 = 0;
			tx_instance_ptr_local->gsi_debug2 = 0;
			tx_instance_ptr_local->gsi_debug3 = 0;
			tx_instance_ptr_local->gsi_debug4 = 0;
		}

		rx_instance_ptr = (struct ipa_lnx_gsi_rx_debug_stats *)((
			uint64_t)instance_ptr + sizeof(struct mhip_instance_info)
			+ (sizeof(struct ipa_lnx_gsi_tx_debug_stats) * (
			ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[
			i].num_tx_instances)));
		for (j = 0; j < ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[
			i].num_rx_instances; j++) {
			rx_instance_ptr_local = (struct ipa_lnx_gsi_rx_debug_stats *)((
				uint64_t)rx_instance_ptr +
				(j * sizeof(struct ipa_lnx_gsi_rx_debug_stats)));

			client_type = ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[
				i].rx_inst_client_type[j];
			rx_instance_ptr_local->rx_client = client_type;
			rx_instance_ptr_local->num_rx_ring_100_perc_with_pack =
				stats.u.ring[2*j].ringFull;
			rx_instance_ptr_local->num_rx_ring_0_perc_with_pack =
				stats.u.ring[2*j].ringEmpty;
			rx_instance_ptr_local->num_rx_ring_above_75_perc_pack =
				stats.u.ring[2*j].ringUsageHigh;
			rx_instance_ptr_local->num_rx_ring_above_25_perc_pack =
				stats.u.ring[2*j].ringUsageLow;
			rx_instance_ptr_local->num_rx_ring_stats_polled =
				stats.u.ring[2*j].RingUtilCount;
			rx_instance_ptr_local->num_rx_drop_stats = 0;
			ipa_lnx_calculate_gsi_ring_summay(NULL,
				rx_instance_ptr_local, client_type);

			/* Currently reserved until GSI needs anything in future */
			rx_instance_ptr_local->gsi_debug1 = 0;
			rx_instance_ptr_local->gsi_debug2 = 0;
			rx_instance_ptr_local->gsi_debug3 = 0;
			rx_instance_ptr_local->gsi_debug4 = 0;
		}

		pipe_info_ptr = (struct ipa_lnx_pipe_info *)((uint64_t)instance_ptr +
			sizeof(struct mhip_instance_info)
			+ (sizeof(struct ipa_lnx_gsi_tx_debug_stats) * (
				ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[
					i].num_tx_instances))
			+ (sizeof(struct ipa_lnx_gsi_rx_debug_stats) * (
				ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[
					i].num_rx_instances)));
		for (j = 0; j < instance_ptr->num_pipes; j++) {
			pipe_info_ptr_local = (struct ipa_lnx_pipe_info *)((uint64_t)
				pipe_info_ptr + (j * sizeof(struct ipa_lnx_pipe_info)));

			ep_idx = ipa3_get_ep_mapping(
				ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[
					i].pipes_client_type[j]);
			if (ep_idx == -1) {
				kfree(mhip_stats);
				return -EFAULT;
			}
			pipe_info_ptr_local->pipe_num = ep_idx;
			ipa_get_gsi_pipe_info(
				pipe_info_ptr_local, &ipa3_ctx->ep[ep_idx]);
		}
		instance_ptr = (struct mhip_instance_info *)((uint64_t)pipe_info_ptr +
			(sizeof(struct ipa_lnx_pipe_info) * (instance_ptr->num_pipes)));
	}

success:
	if(copy_to_user((void __user *)arg,
		(u8 *)mhip_stats,
		alloc_size)) {
		IPA_STATS_ERR("copy to user failed");
		kfree(mhip_stats);
		return -EFAULT;
	}

	kfree(mhip_stats);
	return 0;
}

static int ipa_stats_get_alloc_info(unsigned long arg)
{
	int i = 0;
	int j, k;
	int holb_drop_stats_num_pipes = 0;
	int ipa_ep_idx_tx, ipa_ep_idx_rx;
	int ipa_client_type;
	int reg_idx;
	int index;

	if (copy_from_user(&ipa_lnx_agent_ctx, u64_to_user_ptr((u64) arg),
		sizeof(struct ipa_lnx_stats_spearhead_ctx))) {
		IPA_STATS_ERR("copy from user failed");
		return -EFAULT;
	}

	/* For generic stats */
	if (ipa_lnx_agent_ctx.log_type_mask &
		SPRHD_IPA_LOG_TYPE_GENERIC_STATS) {
		for (i = 0; i < IPA_CLIENT_MAX; i++) {
			int ep_idx = ipa3_get_ep_mapping(i);

			if ((ep_idx == -1) || (!IPA_CLIENT_IS_CONS(i)) ||
				(IPA_CLIENT_IS_TEST(i)))
				continue;

			reg_idx = ipahal_get_ep_reg_idx(ep_idx);
			if (!(ipa3_ctx->hw_stats &&
				(ipa3_ctx->hw_stats->drop.init.enabled_bitmask[reg_idx] &
				ipahal_get_ep_bit(ep_idx))))
				continue;

			holb_drop_stats_num_pipes++;
		}
		ipa_lnx_agent_ctx.alloc_info.num_holb_drop_stats_clients =
			holb_drop_stats_num_pipes;
		ipa_lnx_agent_ctx.alloc_info.num_holb_mon_stats_clients =
			ipa3_ctx->uc_ctx.holb_monitor.num_holb_clients;
	}

	/* For clock stats */
	if (ipa_lnx_agent_ctx.log_type_mask & SPRHD_IPA_LOG_TYPE_CLOCK_STATS)
		ipa_lnx_agent_ctx.alloc_info.num_pm_clients =
			ipa3_get_max_num_pipes();

	/* For WLAN instance */
	if (ipa_lnx_agent_ctx.log_type_mask & SPRHD_IPA_LOG_TYPE_WLAN_STATS) {
		ipa_ep_idx_tx = ipa3_get_ep_mapping(IPA_CLIENT_WLAN2_CONS);
		ipa_ep_idx_rx = ipa3_get_ep_mapping(IPA_CLIENT_WLAN2_PROD);
		if ((ipa_ep_idx_tx == -1) || (ipa_ep_idx_rx == -1) ||
			!ipa3_ctx->ep[ipa_ep_idx_tx].valid ||
			!ipa3_ctx->ep[ipa_ep_idx_rx].valid) {
			ipa_lnx_agent_ctx.alloc_info.num_wlan_instances = 0;
		} else {
			ipa_lnx_agent_ctx.alloc_info.num_wlan_instances = 1;
			ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[0].num_pipes = 2;
			ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[0].num_tx_instances = 1;
			ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[0].num_rx_instances = 1;
			ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[0].pipes_client_type[0]
				= IPA_CLIENT_WLAN2_CONS;
			ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[0].pipes_client_type[1]
				= IPA_CLIENT_WLAN2_PROD;
			ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[0].tx_inst_client_type[0]
				= IPA_CLIENT_WLAN2_CONS;
			ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[0].rx_inst_client_type[0]
				= IPA_CLIENT_WLAN2_PROD;
			if(ipa_wdi_is_tx1_used() == 1) {
				ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[0].num_tx_instances++;
				ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[0].num_pipes++;
				ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[
					0].pipes_client_type[2] = IPA_CLIENT_WLAN2_CONS1;
				ipa_lnx_agent_ctx.alloc_info.wlan_inst_info[
					0].tx_inst_client_type[1] = IPA_CLIENT_WLAN2_CONS1;
		}
		}
	}

	/* For ETH instance */
	if (ipa_lnx_agent_ctx.log_type_mask & SPRHD_IPA_LOG_TYPE_ETH_STATS) {
		ipa_lnx_agent_ctx.alloc_info.num_eth_instances = 0;
		for (i = 0; i < IPA_ETH_INST_ID_MAX; i++) {
			ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_pipes = 0;
			ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_pipes = 0;
			ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_tx_instances
				= 0;
			ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_rx_instances
				= 0;
			k = 0;
			for (j = 0; (j < IPA_ETH_CLIENT_MAX) &&
				(k < SPEARHEAD_NUM_MAX_TX_INSTANCES); j++) {
				if (ipa_eth_client_exist(j, i)) {
					ipa_lnx_agent_ctx.alloc_info.eth_inst_info[i].num_pipes =
						ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
							i].num_pipes + 2;
					ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
						i].num_tx_instances++;
					ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
						i].num_rx_instances++;
					ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
						i].tx_inst_client_type[k] = j;
					ipa_client_type =
						ipa_eth_get_ipa_client_type_from_eth_type(
							j, IPA_ETH_PIPE_DIR_TX);
					if (ipa_client_type >= IPA_CLIENT_MAX) {
						IPA_STATS_ERR("Eth tx client type not found");
						ipa_assert();
					}
					ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
						i].pipes_client_type[k*2] = ipa_client_type;
					ipa_client_type =
						ipa_eth_get_ipa_client_type_from_eth_type(
							j, IPA_ETH_PIPE_DIR_RX);
					if (ipa_client_type >= IPA_CLIENT_MAX) {
						IPA_STATS_ERR("Eth rx client type not found");
						ipa_assert();
					}
					ipa_lnx_agent_ctx.alloc_info.eth_inst_info[
						i].pipes_client_type[(k*2) + 1] = ipa_client_type;
					ipa_lnx_agent_ctx.alloc_info.num_eth_instances++;
					k++;
				}
			}
		}
	}

	/* For USB instance */
	if (ipa_lnx_agent_ctx.log_type_mask & SPRHD_IPA_LOG_TYPE_USB_STATS) {
		ipa_lnx_agent_ctx.alloc_info.num_usb_instances = 0;
		index = 0;
		for (i = 0; (i < IPA_USB_MAX_TETH_PROT_SIZE) &&
			(index < SPEARHEAD_NUM_MAX_INSTANCES); i++) {
			if(ipa_usb_is_teth_prot_connected(i)) {
				if (index == SPEARHEAD_NUM_MAX_INSTANCES) {
					IPA_STATS_ERR("USB alloc info max size reached\n");
					break;
				}
				ipa_lnx_agent_ctx.usb_teth_prot[index] = i;
				if (ipa_lnx_agent_ctx.usb_teth_prot[index] ==
					IPA_USB_RMNET_CV2X) {
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].num_pipes = 2;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].num_tx_instances = 1;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].num_rx_instances = 1;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].pipes_client_type[0] = IPA_CLIENT_USB2_PROD;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].pipes_client_type[1] = IPA_CLIENT_USB2_CONS;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].tx_inst_client_type[0] = IPA_CLIENT_USB2_CONS;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].rx_inst_client_type[0] = IPA_CLIENT_USB2_PROD;
				} else if (ipa_lnx_agent_ctx.usb_teth_prot[index] ==
					IPA_USB_DIAG) {
					/* USB DIAG stats not supported, can be added in future */
					continue;
				} else {
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].num_pipes = 2;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].num_tx_instances = 1;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].num_rx_instances = 1;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].pipes_client_type[0] = IPA_CLIENT_USB_PROD;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].pipes_client_type[1] = IPA_CLIENT_USB_CONS;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].tx_inst_client_type[0] = IPA_CLIENT_USB_CONS;
					ipa_lnx_agent_ctx.alloc_info.usb_inst_info[
						index].rx_inst_client_type[0] = IPA_CLIENT_USB_PROD;
				}
				ipa_lnx_agent_ctx.alloc_info.num_usb_instances++;
				index++;
			}
		}
	}

	/* For MHIP instance */
	if (ipa_lnx_agent_ctx.log_type_mask & SPRHD_IPA_LOG_TYPE_MHIP_STATS) {
#if IS_ENABLED(CONFIG_IPA3_MHI_PRIME_MANAGER)
		if (ipa_usb_is_teth_prot_connected(IPA_USB_RNDIS))
			ipa_lnx_agent_ctx.usb_teth_prot[0] = IPA_USB_RNDIS;
		else if(ipa_usb_is_teth_prot_connected(IPA_USB_RMNET))
			ipa_lnx_agent_ctx.usb_teth_prot[0] = IPA_USB_RMNET;
		else ipa_lnx_agent_ctx.usb_teth_prot[0] = IPA_USB_MAX_TETH_PROT_SIZE;
		ipa_lnx_agent_ctx.alloc_info.num_mhip_instances = 1;
		ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[0].num_pipes = 4;
		ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[0].num_tx_instances = 2;
		ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[0].num_rx_instances = 2;
		ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[0].pipes_client_type[0] =
			IPA_CLIENT_MHI_PRIME_TETH_CONS;
		ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[0].pipes_client_type[1] =
			IPA_CLIENT_MHI_PRIME_TETH_PROD;
		ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[0].pipes_client_type[2] =
			IPA_CLIENT_MHI_PRIME_RMNET_CONS;
		ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[0].pipes_client_type[3] =
			IPA_CLIENT_MHI_PRIME_RMNET_PROD;
		ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[0].tx_inst_client_type[0]
			= IPA_CLIENT_MHI_PRIME_TETH_CONS;
		ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[0].tx_inst_client_type[1]
			= IPA_CLIENT_MHI_PRIME_RMNET_CONS;
		ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[0].rx_inst_client_type[0]
			= IPA_CLIENT_MHI_PRIME_TETH_PROD;
		ipa_lnx_agent_ctx.alloc_info.mhip_inst_info[0].rx_inst_client_type[1]
			= IPA_CLIENT_MHI_PRIME_RMNET_PROD;
#else
		/* MHI Prime is not enabled */
		ipa_lnx_agent_ctx.alloc_info.num_mhip_instances = 0;
#endif
	}

	if(copy_to_user((u8 *)arg,
		&ipa_lnx_agent_ctx,
		sizeof(struct ipa_lnx_stats_spearhead_ctx))) {
		IPA_STATS_ERR("copy to user failed");
		return -EFAULT;
	}
	return 0;
}

static long ipa_lnx_stats_ioctl(struct file *filp,
	unsigned int cmd,
	unsigned long arg)
{
	int retval = 0;
	u8 *param = NULL;

	if (_IOC_TYPE(cmd) != IPA_LNX_STATS_IOC_MAGIC) {
		IPA_STATS_ERR("IOC type mismatch %d\n", cmd);
		return -ENOTTY;
	}

	if(!ipa3_ctx) {
		IPA_STATS_ERR("IPA driver is not up, rejecting the ioctl\n");
		return -EPERM;
	}

	switch (cmd) {
	case IPA_LNX_IOC_GET_ALLOC_INFO:
		if (!ipa_stats_struct_mismatch(IPA_LNX_CMD_GET_ALLOC_INFO)) {
			retval = ipa_stats_get_alloc_info(arg);
			if (retval) {
				IPA_STATS_ERR("ipa get alloc info fail");
				break;
			}
		}
		else retval = -EPERM;
		break;
	case IPA_LNX_IOC_GET_GENERIC_STATS:
		if (!ipa_stats_struct_mismatch(IPA_LNX_CMD_GENERIC_STATS)) {
			retval = ipa_get_generic_stats(arg);
			if (retval) {
				IPA_STATS_ERR("ipa get generic stats fail");
				break;
			}
		}
		else retval = -EPERM;
		break;
	case IPA_LNX_IOC_GET_CLOCK_STATS:
		if (!ipa_stats_struct_mismatch(IPA_LNX_CMD_CLOCK_STATS)) {
			retval = ipa_get_clock_stats(arg);
			if (retval) {
				IPA_STATS_ERR("ipa get clock stats fail");
				break;
			}
		} else retval = -EPERM;
		break;
	case IPA_LNX_IOC_GET_WLAN_INST_STATS:
		if (!ipa_stats_struct_mismatch(IPA_LNX_CMD_WLAN_INST_STATS)) {
			retval = ipa_get_wlan_inst_stats(arg);
			if (retval) {
				IPA_STATS_ERR("ipa get wlan inst stats fail");
				break;
			}
		} else retval = -EPERM;
		break;
	case IPA_LNX_IOC_GET_ETH_INST_STATS:
		if (!ipa_stats_struct_mismatch(IPA_LNX_CMD_ETH_INST_STATS)) {
			retval = ipa_get_eth_inst_stats(arg);
			if (retval) {
				IPA_STATS_ERR("ipa get eth inst stats fail");
				break;
			}
		} else retval = -EPERM;
		break;
	case IPA_LNX_IOC_GET_USB_INST_STATS:
		if (!ipa_stats_struct_mismatch(IPA_LNX_CMD_USB_INST_STATS)) {
			retval = ipa_get_usb_inst_stats(arg);
			if (retval) {
				IPA_STATS_ERR("ipa get usb inst stats fail");
				break;
			}
		} else retval = -EPERM;
		break;
	case IPA_LNX_IOC_GET_MHIP_INST_STATS:
#if IS_ENABLED(CONFIG_IPA3_MHI_PRIME_MANAGER)
		if (!ipa_stats_struct_mismatch(IPA_LNX_CMD_MHIP_INST_STATS)) {
			retval = ipa_get_mhip_inst_stats(arg);
			if (retval) {
				IPA_STATS_ERR("ipa get mhip inst stats fail");
				break;
			}
		} else retval = -EPERM;
#else
		retval = IPA_LNX_STATS_SUCCESS;
#endif
		break;
	default:
		retval = -ENOTTY;
	}
	if (param)
		vfree(param);
	return retval;
}

const struct file_operations ipa_stats_fops = {
	.owner = THIS_MODULE,
	.open = ipa_stats_ioctl_open,
	.read = NULL,
	.unlocked_ioctl = ipa_lnx_stats_ioctl,
};

static int ipa_spearhead_stats_ioctl_init(void)
{
	unsigned int ipa_lnx_stats_ioctl_major = 0;
	int ret;
	struct device *dev;

	device = MKDEV(ipa_lnx_stats_ioctl_major, 0);

	ret = alloc_chrdev_region(&device, 0, dev_num, DRIVER_NAME);
	if (ret) {
		IPA_STATS_ERR(":device_alloc err.\n");
		goto dev_alloc_err;
	}
	ipa_lnx_stats_ioctl_major = MAJOR(device);

	class = class_create(THIS_MODULE, DRIVER_NAME);
	if (IS_ERR(class)) {
		IPA_STATS_ERR(":class_create err.\n");
		goto class_err;
	}

	dev = device_create(class, NULL, device,
			NULL, DRIVER_NAME);
	if (IS_ERR(dev)) {
		IPA_STATS_ERR(":device_create err.\n");
		goto device_err;
	}

	cdev_init(&ipa_lnx_stats_ioctl_cdev, &ipa_stats_fops);
	ret = cdev_add(&ipa_lnx_stats_ioctl_cdev, device, dev_num);
	if (ret) {
		IPA_STATS_ERR(":cdev_add err.\n");
		goto cdev_add_err;
	}

	IPA_STATS_ERR("IPA %s major(%d) initial ok :>>>>\n",
		DRIVER_NAME, ipa_lnx_stats_ioctl_major);
	return 0;

cdev_add_err:
	device_destroy(class, device);
device_err:
	class_destroy(class);
class_err:
	unregister_chrdev_region(device, dev_num);
dev_alloc_err:
	return -ENODEV;
}

int ipa_spearhead_stats_init()
{
	int ret;

	ret = ipa_spearhead_stats_ioctl_init();
	if(ret) {
		IPA_STATS_ERR("IPA_LNX_STATS_IOCTL init failure = %d\n", ret);
		return -1;
	}
	memset(&poll_pack_and_cred_info, 0, sizeof(poll_pack_and_cred_info));
	IPA_STATS_ERR("IPA_LNX_STATS_IOCTL init success\n");

	return 0;
}
