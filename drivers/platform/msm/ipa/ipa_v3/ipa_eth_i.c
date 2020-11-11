// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 */
#include "ipa_i.h"
#include <linux/if_vlan.h>
#include <linux/ipa_eth.h>
#include <linux/log2.h>

#define IPA_ETH_RTK_MODT (32)
#define IPA_ETH_RTK_MODC (128)

#define IPA_ETH_AQC_MODT (32)
#define IPA_ETH_AQC_MODC (128)


#define IPA_ETH_AGGR_PKT_LIMIT 1
#define IPA_ETH_AGGR_BYTE_LIMIT 2 /*2 Kbytes Agger hard byte limit*/

#define IPA_ETH_MBOX_M (1)
#define IPA_ETH_RX_MBOX_N (20)
#define IPA_ETH_TX_MBOX_N (21)

#define IPA_ETH_RX_MBOX_VAL (1)
#define IPA_ETH_TX_MBOX_VAL (2)

#define IPA_ETH_PCIE_MASK BIT_ULL(40)
#define IPA_ETH_PCIE_SET(val) (val | IPA_ETH_PCIE_MASK)

enum ipa_eth_dir {
	IPA_ETH_RX = 0,
	IPA_ETH_TX = 1,
};

static void ipa3_eth_save_client_mapping(
	struct ipa_eth_client_pipe_info *pipe,
	enum ipa_client_type type, int id,
	int pipe_id, int ch_id)
{
	struct ipa_eth_client *client_info;
	enum ipa_eth_client_type client_type;
	u8 inst_id, pipe_hdl;
	struct ipa3_eth_info *eth_info;

	client_info = pipe->client_info;
	client_type = client_info->client_type;
	inst_id = client_info->inst_id;
	pipe_hdl = pipe->pipe_hdl;
	eth_info = &ipa3_ctx->eth_info[client_type][inst_id];
	if (!eth_info->map[id].valid) {
		eth_info->num_ch++;
		eth_info->map[id].type = type;
		eth_info->map[id].pipe_id = pipe_id;
		eth_info->map[id].ch_id = ch_id;
		eth_info->map[id].valid = true;
		eth_info->map[id].pipe_hdl = pipe_hdl;
	}
}

static void ipa3_eth_release_client_mapping(
	struct ipa_eth_client_pipe_info *pipe,
	int id)
{
	struct ipa_eth_client *client_info;
	enum ipa_eth_client_type client_type;
	u8 inst_id, pipe_hdl;
	struct ipa3_eth_info *eth_info;

	client_info = pipe->client_info;
	client_type = client_info->client_type;
	inst_id = client_info->inst_id;
	pipe_hdl = pipe->pipe_hdl;
	eth_info = &ipa3_ctx->eth_info[client_type][inst_id];
	if (eth_info->map[id].valid) {
		eth_info->num_ch--;
		eth_info->map[id].type = 0;
		eth_info->map[id].pipe_id = 0;
		eth_info->map[id].ch_id = 0;
		eth_info->map[id].valid = false;
		eth_info->map[id].pipe_hdl = 0;
	}
}

static int ipa3_eth_uc_init_peripheral(bool init, u64 per_base)
{
	struct ipa_mem_buffer cmd;
	enum ipa_cpu_2_hw_offload_commands command;
	int result;

	if (init) {
		struct IpaHwAQCInitCmdData_t *cmd_data;

		cmd.size = sizeof(*cmd_data);
		cmd.base = dma_alloc_coherent(ipa3_ctx->uc_pdev, cmd.size,
			&cmd.phys_base, GFP_KERNEL);
		if (cmd.base == NULL) {
			IPAERR("fail to get DMA memory.\n");
			return -ENOMEM;
		}
		cmd_data =
			(struct IpaHwAQCInitCmdData_t *)cmd.base;
		cmd_data->periph_baddr_lsb = lower_32_bits(per_base);
		cmd_data->periph_baddr_msb = upper_32_bits(per_base);
		command = IPA_CPU_2_HW_CMD_PERIPHERAL_INIT;
	} else {
		struct IpaHwAQCDeinitCmdData_t *cmd_data;

		cmd.size = sizeof(*cmd_data);
		cmd.base = dma_alloc_coherent(ipa3_ctx->uc_pdev, cmd.size,
			&cmd.phys_base, GFP_KERNEL);
		if (cmd.base == NULL) {
			IPAERR("fail to get DMA memory.\n");
			return -ENOMEM;
		}
		cmd_data =
			(struct IpaHwAQCDeinitCmdData_t *)cmd.base;
		cmd_data->reserved = 0;
		command = IPA_CPU_2_HW_CMD_PERIPHERAL_DEINIT;
	}
	IPA_ACTIVE_CLIENTS_INC_SIMPLE();

	result = ipa3_uc_send_cmd((u32)(cmd.phys_base),
		command,
		IPA_HW_2_CPU_OFFLOAD_CMD_STATUS_SUCCESS,
		false, 10 * HZ);
	if (result) {
		IPAERR("fail to %s uc\n",
			init ? "init" : "deinit");
	}

	dma_free_coherent(ipa3_ctx->uc_pdev,
		cmd.size, cmd.base, cmd.phys_base);
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();

	IPADBG("exit\n");
	return result;
}

static int ipa3_eth_config_uc(bool init,
	u8 protocol,
	u8 dir,
	u8 gsi_ch,
	u8 peripheral_ch)
{
	struct ipa_mem_buffer cmd;
	enum ipa_cpu_2_hw_offload_commands command;
	int result;

	IPADBG("config uc %s\n", init ? "init" : "Deinit");
	if (init) {
		struct IpaHwOffloadSetUpCmdData_t_v4_0 *cmd_data;

		cmd.size = sizeof(*cmd_data);
		cmd.base = dma_alloc_coherent(ipa3_ctx->uc_pdev, cmd.size,
			&cmd.phys_base, GFP_KERNEL);
		if (cmd.base == NULL) {
			IPAERR("fail to get DMA memory.\n");
			return -ENOMEM;
		}
		cmd_data =
			(struct IpaHwOffloadSetUpCmdData_t_v4_0 *)cmd.base;
		cmd_data->protocol = protocol;
		switch (protocol) {
		case IPA_HW_PROTOCOL_AQC:
			cmd_data->SetupCh_params.AqcSetupCh_params.dir =
				dir;
			cmd_data->SetupCh_params.AqcSetupCh_params.gsi_ch =
				gsi_ch;
			cmd_data->SetupCh_params.AqcSetupCh_params.aqc_ch =
				peripheral_ch;
			break;
		case IPA_HW_PROTOCOL_RTK:
			cmd_data->SetupCh_params.RtkSetupCh_params.dir =
				dir;
			cmd_data->SetupCh_params.RtkSetupCh_params.gsi_ch =
				gsi_ch;
			break;
		default:
			IPAERR("invalid protocol%d\n", protocol);
		}
		command = IPA_CPU_2_HW_CMD_OFFLOAD_CHANNEL_SET_UP;

	} else {
		struct IpaHwOffloadCommonChCmdData_t_v4_0 *cmd_data;

		cmd.size = sizeof(*cmd_data);
		cmd.base = dma_alloc_coherent(ipa3_ctx->uc_pdev, cmd.size,
			&cmd.phys_base, GFP_KERNEL);
		if (cmd.base == NULL) {
			IPAERR("fail to get DMA memory.\n");
			return -ENOMEM;
		}

		cmd_data =
			(struct IpaHwOffloadCommonChCmdData_t_v4_0 *)cmd.base;

		cmd_data->protocol = protocol;
		switch (protocol) {
		case IPA_HW_PROTOCOL_AQC:
			cmd_data->CommonCh_params.AqcCommonCh_params.gsi_ch =
				gsi_ch;
			break;
		case IPA_HW_PROTOCOL_RTK:
			cmd_data->CommonCh_params.RtkCommonCh_params.gsi_ch =
				gsi_ch;
			break;
		default:
			IPAERR("invalid protocol%d\n", protocol);
		}
		cmd_data->CommonCh_params.RtkCommonCh_params.gsi_ch = gsi_ch;
		command = IPA_CPU_2_HW_CMD_OFFLOAD_TEAR_DOWN;
	}

	IPA_ACTIVE_CLIENTS_INC_SIMPLE();

	result = ipa3_uc_send_cmd((u32)(cmd.phys_base),
		command,
		IPA_HW_2_CPU_OFFLOAD_CMD_STATUS_SUCCESS,
		false, 10 * HZ);
	if (result) {
		IPAERR("fail to %s uc for %s gsi channel %d\n",
			init ? "init" : "deinit",
			dir == IPA_ETH_RX ? "Rx" : "Tx", gsi_ch);
	}

	dma_free_coherent(ipa3_ctx->uc_pdev,
		cmd.size, cmd.base, cmd.phys_base);
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();

	IPADBG("exit\n");
	return result;
}


static void ipa_eth_gsi_evt_ring_err_cb(struct gsi_evt_err_notify *notify)
{
	switch (notify->evt_id) {
	case GSI_EVT_OUT_OF_BUFFERS_ERR:
		IPAERR("Got GSI_EVT_OUT_OF_BUFFERS_ERR\n");
		break;
	case GSI_EVT_OUT_OF_RESOURCES_ERR:
		IPAERR("Got GSI_EVT_OUT_OF_RESOURCES_ERR\n");
		break;
	case GSI_EVT_UNSUPPORTED_INTER_EE_OP_ERR:
		IPAERR("Got GSI_EVT_UNSUPPORTED_INTER_EE_OP_ERR\n");
		break;
	case GSI_EVT_EVT_RING_EMPTY_ERR:
		IPAERR("Got GSI_EVT_EVT_RING_EMPTY_ERR\n");
		break;
	default:
		IPAERR("Unexpected err evt: %d\n", notify->evt_id);
	}
	ipa_assert();
}

static void ipa_eth_gsi_chan_err_cb(struct gsi_chan_err_notify *notify)
{
	switch (notify->evt_id) {
	case GSI_CHAN_INVALID_TRE_ERR:
		IPAERR("Got GSI_CHAN_INVALID_TRE_ERR\n");
		break;
	case GSI_CHAN_NON_ALLOCATED_EVT_ACCESS_ERR:
		IPAERR("Got GSI_CHAN_NON_ALLOCATED_EVT_ACCESS_ERR\n");
		break;
	case GSI_CHAN_OUT_OF_BUFFERS_ERR:
		IPAERR("Got GSI_CHAN_OUT_OF_BUFFERS_ERR\n");
		break;
	case GSI_CHAN_OUT_OF_RESOURCES_ERR:
		IPAERR("Got GSI_CHAN_OUT_OF_RESOURCES_ERR\n");
		break;
	case GSI_CHAN_UNSUPPORTED_INTER_EE_OP_ERR:
		IPAERR("Got GSI_CHAN_UNSUPPORTED_INTER_EE_OP_ERR\n");
		break;
	case GSI_CHAN_HWO_1_ERR:
		IPAERR("Got GSI_CHAN_HWO_1_ERR\n");
		break;
	default:
		IPAERR("Unexpected err evt: %d\n", notify->evt_id);
	}
	ipa_assert();
}


static int ipa_eth_setup_rtk_gsi_channel(
	struct ipa_eth_client_pipe_info *pipe,
	struct ipa3_ep_context *ep)
{
	struct gsi_evt_ring_props gsi_evt_ring_props;
	struct gsi_chan_props gsi_channel_props;
	union __packed gsi_channel_scratch ch_scratch;
	union __packed gsi_evt_scratch evt_scratch;
	const struct ipa_gsi_ep_config *gsi_ep_info;
	int result, len;
	int queue_number;
	u64 bar_addr;

	if (unlikely(!pipe->info.is_transfer_ring_valid)) {
		IPAERR("RTK transfer ring invalid\n");
		ipa_assert();
		return -EFAULT;
	}

	/* setup event ring */
	bar_addr =
		IPA_ETH_PCIE_SET(pipe->info.client_info.rtk.bar_addr);
	memset(&gsi_evt_ring_props, 0, sizeof(gsi_evt_ring_props));
	gsi_evt_ring_props.intf = GSI_EVT_CHTYPE_RTK_EV;
	gsi_evt_ring_props.intr = GSI_INTR_MSI;
	gsi_evt_ring_props.re_size = GSI_EVT_RING_RE_SIZE_16B;
	if (pipe->dir == IPA_ETH_PIPE_DIR_TX) {
		gsi_evt_ring_props.int_modt = IPA_ETH_RTK_MODT;
		gsi_evt_ring_props.int_modc = IPA_ETH_RTK_MODC;
	}
	gsi_evt_ring_props.exclusive = true;
	gsi_evt_ring_props.err_cb = ipa_eth_gsi_evt_ring_err_cb;
	gsi_evt_ring_props.user_data = NULL;
	gsi_evt_ring_props.msi_addr =
		bar_addr +
		pipe->info.client_info.rtk.dest_tail_ptr_offs;
	len = pipe->info.transfer_ring_size;
	gsi_evt_ring_props.ring_len = len;
	gsi_evt_ring_props.ring_base_addr =
		(u64)pipe->info.transfer_ring_base;
	result = gsi_alloc_evt_ring(&gsi_evt_ring_props,
		ipa3_ctx->gsi_dev_hdl,
		&ep->gsi_evt_ring_hdl);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("fail to alloc RX event ring\n");
		return -EFAULT;
	}
	ep->gsi_mem_info.evt_ring_len =
		gsi_evt_ring_props.ring_len;
	ep->gsi_mem_info.evt_ring_base_addr =
		gsi_evt_ring_props.ring_base_addr;

	/* setup channel ring */
	memset(&gsi_channel_props, 0, sizeof(gsi_channel_props));
	gsi_channel_props.prot = GSI_CHAN_PROT_RTK;
	if (pipe->dir == IPA_ETH_PIPE_DIR_TX)
		gsi_channel_props.dir = GSI_CHAN_DIR_FROM_GSI;
	else
		gsi_channel_props.dir = GSI_CHAN_DIR_TO_GSI;
		gsi_ep_info = ipa3_get_gsi_ep_info(ep->client);
	if (!gsi_ep_info) {
		IPAERR("Failed getting GSI EP info for client=%d\n",
		       ep->client);
		result = -EINVAL;
		goto fail_get_gsi_ep_info;
	} else
		gsi_channel_props.ch_id = gsi_ep_info->ipa_gsi_chan_num;
	gsi_channel_props.evt_ring_hdl = ep->gsi_evt_ring_hdl;
	gsi_channel_props.re_size = GSI_CHAN_RE_SIZE_16B;
	gsi_channel_props.use_db_eng = GSI_CHAN_DB_MODE;
	gsi_channel_props.max_prefetch = GSI_ONE_PREFETCH_SEG;
	gsi_channel_props.prefetch_mode =
		gsi_ep_info->prefetch_mode;
	gsi_channel_props.empty_lvl_threshold =
		gsi_ep_info->prefetch_threshold;
	gsi_channel_props.low_weight = 1;
	gsi_channel_props.err_cb = ipa_eth_gsi_chan_err_cb;
	gsi_channel_props.ring_len = len;
	gsi_channel_props.ring_base_addr =
		(u64)pipe->info.transfer_ring_base;
	result = gsi_alloc_channel(&gsi_channel_props, ipa3_ctx->gsi_dev_hdl,
		&ep->gsi_chan_hdl);
	if (result != GSI_STATUS_SUCCESS)
		goto fail_get_gsi_ep_info;
	ep->gsi_mem_info.chan_ring_len = gsi_channel_props.ring_len;
	ep->gsi_mem_info.chan_ring_base_addr =
		gsi_channel_props.ring_base_addr;

	/* write event scratch */
	memset(&evt_scratch, 0, sizeof(evt_scratch));
	/* nothing is needed for RTK event scratch */

	/* write ch scratch */
	queue_number = pipe->info.client_info.rtk.queue_number;
	memset(&ch_scratch, 0, sizeof(ch_scratch));
	ch_scratch.rtk.rtk_bar_low =
		(u32)bar_addr;
	ch_scratch.rtk.rtk_bar_high =
		(u32)((u64)(bar_addr) >> 32);
	/*
	 * RX: Queue Number will be as is received from RTK
	 * (Range 0 - 15).
	 * TX: Queue Number will be configured to be
	 * either 16 or 18.
	 * (For TX Queue 0: Configure 16)
	 * (For TX Queue 1: Configure 18)
	 */
	ch_scratch.rtk.queue_number =
		(pipe->dir == IPA_ETH_PIPE_DIR_RX) ?
		pipe->info.client_info.rtk.queue_number :
		(queue_number == 0) ? 16 : 18;
	ch_scratch.rtk.fix_buff_size =
		ilog2(pipe->info.fix_buffer_size);
	ch_scratch.rtk.rtk_buff_addr_low =
		(u32)pipe->info.data_buff_list[0].iova;
	ch_scratch.rtk.rtk_buff_addr_high =
		(u32)((u64)(pipe->info.data_buff_list[0].iova) >> 32);
	result = gsi_write_channel_scratch(ep->gsi_chan_hdl, ch_scratch);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("failed to write evt ring scratch\n");
		goto fail_write_scratch;
	}
	return 0;
fail_write_scratch:
	gsi_dealloc_channel(ep->gsi_chan_hdl);
	ep->gsi_chan_hdl = ~0;
fail_get_gsi_ep_info:
	gsi_dealloc_evt_ring(ep->gsi_evt_ring_hdl);
	ep->gsi_evt_ring_hdl = ~0;
	return result;
}

static int ipa3_smmu_map_eth_pipes(struct ipa_eth_client_pipe_info *pipe,
	bool map)
{
	struct iommu_domain *smmu_domain;
	int result;
	int i;
	u64 iova;
	phys_addr_t pa;
	u64 iova_p;
	phys_addr_t pa_p;
	u32 size_p;

	if (pipe->info.fix_buffer_size > PAGE_SIZE) {
		IPAERR("invalid data buff size\n");
		return -EINVAL;
	}

	if (ipa3_ctx->s1_bypass_arr[IPA_SMMU_CB_AP]) {
		IPADBG("AP SMMU is set to s1 bypass\n");
		return 0;
	}

	smmu_domain = ipa3_get_smmu_domain();
	if (!smmu_domain) {
		IPAERR("invalid smmu domain\n");
		return -EINVAL;
	}

	result = ipa3_smmu_map_peer_buff(
		(u64)pipe->info.transfer_ring_base,
		pipe->info.transfer_ring_size,
		map,
		pipe->info.transfer_ring_sgt,
		IPA_SMMU_CB_AP);
	if (result) {
		IPAERR("failed to %s ntn ring %d\n",
			map ? "map" : "unmap", result);
		return -EINVAL;
	}

	for (i = 0; i < pipe->info.data_buff_list_size; i++) {
		iova = (u64)pipe->info.data_buff_list[i].iova;
		pa = (phys_addr_t)pipe->info.data_buff_list[i].pa;
		IPA_SMMU_ROUND_TO_PAGE(iova, pa, pipe->info.fix_buffer_size,
			iova_p, pa_p, size_p);
		IPADBG_LOW("%s 0x%llx to 0x%pa size %d\n", map ? "mapping" :
			"unmapping", iova_p, &pa_p, size_p);
		if (map) {
			result = ipa3_iommu_map(smmu_domain, iova_p, pa_p,
				size_p, IOMMU_READ | IOMMU_WRITE);
			if (result)
				IPAERR("Fail to map 0x%llx\n", iova);
		} else {
			result = iommu_unmap(smmu_domain, iova_p, size_p);
			if (result != size_p) {
				IPAERR("Fail to unmap 0x%llx\n", iova);
				goto fail_map_buffer_smmu_enabled;
			}
		}
	}
	return 0;

fail_map_buffer_smmu_enabled:
	ipa3_smmu_map_peer_buff(
		(u64)pipe->info.transfer_ring_base,
		pipe->info.transfer_ring_size,
		!map,
		pipe->info.transfer_ring_sgt,
		IPA_SMMU_CB_AP);
	return result;
}

static int ipa_eth_setup_aqc_gsi_channel(
	struct ipa_eth_client_pipe_info *pipe,
	struct ipa3_ep_context *ep)
{
	struct gsi_evt_ring_props gsi_evt_ring_props;
	struct gsi_chan_props gsi_channel_props;
	union __packed gsi_channel_scratch ch_scratch;
	union __packed gsi_evt_scratch evt_scratch;
	const struct ipa_gsi_ep_config *gsi_ep_info;
	int result, len;
	u64 bar_addr;
	u64 head_ptr;

	if (unlikely(!pipe->info.is_transfer_ring_valid)) {
		IPAERR("RTK transfer ring invalid\n");
		ipa_assert();
		return -EFAULT;
	}
	/* setup event ring */
	bar_addr =
		IPA_ETH_PCIE_SET(pipe->info.client_info.aqc.bar_addr);
	memset(&gsi_evt_ring_props, 0, sizeof(gsi_evt_ring_props));
	gsi_evt_ring_props.intf = GSI_EVT_CHTYPE_AQC_EV;
	gsi_evt_ring_props.intr = GSI_INTR_MSI;
	gsi_evt_ring_props.re_size = GSI_EVT_RING_RE_SIZE_16B;
	gsi_evt_ring_props.int_modt = IPA_ETH_AQC_MODT;
	gsi_evt_ring_props.int_modc = IPA_ETH_AQC_MODC;
	gsi_evt_ring_props.exclusive = true;
	gsi_evt_ring_props.err_cb = ipa_eth_gsi_evt_ring_err_cb;
	gsi_evt_ring_props.user_data = NULL;
	gsi_evt_ring_props.msi_addr =
		bar_addr +
		pipe->info.client_info.aqc.dest_tail_ptr_offs;
	len = pipe->info.transfer_ring_size;
	gsi_evt_ring_props.ring_len = len;
	gsi_evt_ring_props.ring_base_addr =
		(u64)pipe->info.transfer_ring_base;
	result = gsi_alloc_evt_ring(&gsi_evt_ring_props,
		ipa3_ctx->gsi_dev_hdl,
		&ep->gsi_evt_ring_hdl);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("fail to alloc RX event ring\n");
		result = -EFAULT;
	}
	ep->gsi_mem_info.evt_ring_len =
		gsi_evt_ring_props.ring_len;
	ep->gsi_mem_info.evt_ring_base_addr =
		gsi_evt_ring_props.ring_base_addr;

	/* setup channel ring */
	memset(&gsi_channel_props, 0, sizeof(gsi_channel_props));
	gsi_channel_props.prot = GSI_CHAN_PROT_AQC;
	if (pipe->dir == IPA_ETH_PIPE_DIR_TX)
		gsi_channel_props.dir = GSI_CHAN_DIR_FROM_GSI;
	else
		gsi_channel_props.dir = GSI_CHAN_DIR_TO_GSI;
		gsi_ep_info = ipa3_get_gsi_ep_info(ep->client);
	if (!gsi_ep_info) {
		IPAERR("Failed getting GSI EP info for client=%d\n",
		       ep->client);
		result = -EINVAL;
	} else
		gsi_channel_props.ch_id = gsi_ep_info->ipa_gsi_chan_num;
	gsi_channel_props.evt_ring_hdl = ep->gsi_evt_ring_hdl;
	gsi_channel_props.re_size = GSI_CHAN_RE_SIZE_16B;
	gsi_channel_props.use_db_eng = GSI_CHAN_DB_MODE;
	gsi_channel_props.db_in_bytes = 0;
	gsi_channel_props.max_prefetch = GSI_ONE_PREFETCH_SEG;
	gsi_channel_props.prefetch_mode =
		gsi_ep_info->prefetch_mode;
	gsi_channel_props.empty_lvl_threshold =
		gsi_ep_info->prefetch_threshold;
	gsi_channel_props.low_weight = 1;
	gsi_channel_props.err_cb = ipa_eth_gsi_chan_err_cb;
	gsi_channel_props.ring_len = len;
	gsi_channel_props.ring_base_addr =
		(u64)pipe->info.transfer_ring_base;
	result = gsi_alloc_channel(&gsi_channel_props, ipa3_ctx->gsi_dev_hdl,
		&ep->gsi_chan_hdl);
	if (result != GSI_STATUS_SUCCESS)
		goto fail_get_gsi_ep_info;
	ep->gsi_mem_info.chan_ring_len = gsi_channel_props.ring_len;
	ep->gsi_mem_info.chan_ring_base_addr =
		gsi_channel_props.ring_base_addr;

	/* write event scratch */
	memset(&evt_scratch, 0, sizeof(evt_scratch));
	/* nothing is needed for AQC event scratch */

	/* write ch scratch */
	memset(&ch_scratch, 0, sizeof(ch_scratch));
	ch_scratch.aqc.fix_buff_size =
		ilog2(pipe->info.fix_buffer_size);
	head_ptr = pipe->info.client_info.aqc.head_ptr_offs;
	if (pipe->dir == IPA_ETH_PIPE_DIR_RX) {
		ch_scratch.aqc.buff_addr_lsb =
			(u32)pipe->info.data_buff_list[0].iova;
		ch_scratch.aqc.buff_addr_msb =
			(u32)((u64)(pipe->info.data_buff_list[0].iova) >> 32);
		ch_scratch.aqc.head_ptr_lsb = (u32)(bar_addr + head_ptr);
		ch_scratch.aqc.head_ptr_msb = (u32)((bar_addr +
			head_ptr) >> 32);
	}

	result = gsi_write_channel_scratch(ep->gsi_chan_hdl, ch_scratch);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("failed to write evt ring scratch\n");
		goto fail_write_scratch;
	}
	return 0;
fail_write_scratch:
	gsi_dealloc_channel(ep->gsi_chan_hdl);
	ep->gsi_chan_hdl = ~0;
fail_get_gsi_ep_info:
	gsi_dealloc_evt_ring(ep->gsi_evt_ring_hdl);
	ep->gsi_evt_ring_hdl = ~0;
	return result;
}

static int ipa3_eth_get_prot(struct ipa_eth_client_pipe_info *pipe,
	enum ipa4_hw_protocol *prot)
{
	int ret = 0;

	switch (pipe->client_info->client_type) {
	case IPA_ETH_CLIENT_AQC107:
	case IPA_ETH_CLIENT_AQC113:
		*prot = IPA_HW_PROTOCOL_AQC;
		break;
	case IPA_ETH_CLIENT_RTK8111K:
	case IPA_ETH_CLIENT_RTK8125B:
		*prot = IPA_HW_PROTOCOL_RTK;
		break;
	case IPA_ETH_CLIENT_NTN:
	case IPA_ETH_CLIENT_EMAC:
		*prot = IPA_HW_PROTOCOL_ETH;
		break;
	default:
		IPAERR("invalid client type%d\n",
			pipe->client_info->client_type);
		*prot = IPA_HW_PROTOCOL_MAX;
		ret = -EFAULT;
	}

	return ret;
}

int ipa3_eth_connect(
	struct ipa_eth_client_pipe_info *pipe,
	enum ipa_client_type client_type)
{
	struct ipa3_ep_context *ep;
	int ep_idx;
	bool vlan_mode;
	int result = 0;
	u32 gsi_db_addr_low, gsi_db_addr_high;
	void __iomem *db_addr;
	u32 evt_ring_db_addr_low, evt_ring_db_addr_high, db_val = 0;
	int id;
	int ch;
	u64 bar_addr;
	enum ipa4_hw_protocol prot;

	ep_idx = ipa_get_ep_mapping(client_type);
	if (ep_idx == -1 || ep_idx >= IPA3_MAX_NUM_PIPES) {
		IPAERR("undefined client_type\n");
		return -EFAULT;
	}
	/* need enhancement for vlan support on multiple attach */
	result = ipa3_is_vlan_mode(IPA_VLAN_IF_ETH, &vlan_mode);
	if (result) {
		IPAERR("Could not determine IPA VLAN mode\n");
		return result;
	}

	result = ipa3_eth_get_prot(pipe, &prot);
	if (result) {
		IPAERR("Could not determine protocol\n");
		return result;
	}

	if (prot == IPA_HW_PROTOCOL_ETH) {
		IPAERR("EMAC\\NTN still not supported using this framework\n");
		return -EFAULT;
	}

	result = ipa3_smmu_map_eth_pipes(pipe, true);
	if (result) {
		IPAERR("failed to map SMMU %d\n", result);
		return result;
	}

	ep = &ipa3_ctx->ep[ep_idx];
	memset(ep, 0, offsetof(struct ipa3_ep_context, sys));
	IPA_ACTIVE_CLIENTS_INC_SIMPLE();
	ep->valid = 1;
	ep->client = client_type;
	result = ipa3_disable_data_path(ep_idx);
	if (result) {
		IPAERR("disable data path failed res=%d clnt=%d.\n", result,
			ep_idx);
		goto disable_data_path_fail;
	}

	ep->cfg.nat.nat_en = IPA_CLIENT_IS_PROD(client_type) ?
		IPA_SRC_NAT : IPA_BYPASS_NAT;
	ep->cfg.hdr.hdr_len = vlan_mode ? VLAN_ETH_HLEN : ETH_HLEN;
	ep->cfg.mode.mode = IPA_BASIC;
	if (IPA_CLIENT_IS_CONS(client_type)) {
		ep->cfg.aggr.aggr_en = IPA_ENABLE_AGGR;
		ep->cfg.aggr.aggr = IPA_GENERIC;
		ep->cfg.aggr.aggr_byte_limit = IPA_ETH_AGGR_BYTE_LIMIT;
		ep->cfg.aggr.aggr_pkt_limit = IPA_ETH_AGGR_PKT_LIMIT;
		ep->cfg.aggr.aggr_hard_byte_limit_en = IPA_ENABLE_AGGR;
	} else {
		ep->client_notify = pipe->info.notify;
		ep->priv = pipe->info.priv;
	}
	if (ipa3_cfg_ep(ep_idx, &ep->cfg)) {
		IPAERR("fail to setup rx pipe cfg\n");
		goto cfg_ep_fail;
	}
	if (IPA_CLIENT_IS_PROD(client_type))
		ipa3_install_dflt_flt_rules(ep_idx);
	IPADBG("client %d (ep: %d) connected\n", client_type,
		ep_idx);

	if (prot == IPA_HW_PROTOCOL_RTK) {
		if (ipa_eth_setup_rtk_gsi_channel(pipe, ep)) {
			IPAERR("fail to setup eth gsi rx channel\n");
			result = -EFAULT;
			goto setup_gsi_ch_fail;
		}
	} else if (prot == IPA_HW_PROTOCOL_AQC) {
		if (ipa_eth_setup_aqc_gsi_channel(pipe, ep)) {
			IPAERR("fail to setup eth gsi rx channel\n");
			result = -EFAULT;
			goto setup_gsi_ch_fail;
		}
	}

	if (gsi_query_channel_db_addr(ep->gsi_chan_hdl,
		&gsi_db_addr_low, &gsi_db_addr_high)) {
		IPAERR("failed to query gsi rx db addr\n");
		result = -EFAULT;
		goto query_ch_db_fail;
	}
	/* only 32 bit lsb is used */
	db_addr = ioremap((phys_addr_t)(gsi_db_addr_low), 4);
	if (IPA_CLIENT_IS_PROD(client_type)) {
		/* Rx: Initialize to ring base (i.e point 6) */
		db_val = (u32)ep->gsi_mem_info.chan_ring_base_addr;
	} else {
		/* TX: Initialize to end of ring */
		db_val = (u32)ep->gsi_mem_info.chan_ring_base_addr;
		db_val += (u32)ep->gsi_mem_info.chan_ring_len;
	}
	iowrite32(db_val, db_addr);
	iounmap(db_addr);
	gsi_query_evt_ring_db_addr(ep->gsi_evt_ring_hdl,
		&evt_ring_db_addr_low, &evt_ring_db_addr_high);
	IPADBG("evt_ring_hdl %lu, db_addr_low %u db_addr_high %u\n",
		ep->gsi_evt_ring_hdl, evt_ring_db_addr_low,
		evt_ring_db_addr_high);
	/* only 32 bit lsb is used */
	db_addr = ioremap((phys_addr_t)(evt_ring_db_addr_low), 4);
	/*
	* IPA/GSI driver should ring the event DB once after
	* initialization of the event, with a value that is
	* outside of the ring range. Eg: ring base = 0x1000,
	* ring size = 0x100 => AP can write value > 0x1100
	* into the doorbell address. Eg: 0x 1110.
	* Use event ring base addr + event ring size + 1 element size.
	*/
	db_val = (u32)ep->gsi_mem_info.evt_ring_base_addr;
	db_val += (u32)ep->gsi_mem_info.evt_ring_len;
	db_val += GSI_EVT_RING_RE_SIZE_16B;
	iowrite32(db_val, db_addr);
	iounmap(db_addr);
	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v5_0) {
		if (IPA_CLIENT_IS_PROD(client_type)) {
			if (gsi_query_aqc_msi_addr(ep->gsi_chan_hdl,
				&pipe->info.db_pa)) {
				result = -EFAULT;
				goto query_msi_fail;
			}
		} else {
			pipe->info.db_pa = 0;
		}
	} else {
		if (IPA_CLIENT_IS_PROD(client_type)) {
			/* RX mailbox */
			pipe->info.db_pa = ipa3_ctx->ipa_wrapper_base +
				ipahal_get_reg_base() +
				ipahal_get_reg_mn_ofst(IPA_UC_MAILBOX_m_n,
					IPA_ETH_MBOX_M,
					IPA_ETH_RX_MBOX_N);
			pipe->info.db_val = IPA_ETH_RX_MBOX_VAL;
		} else {
			/* TX mailbox */
			pipe->info.db_pa = ipa3_ctx->ipa_wrapper_base +
				ipahal_get_reg_base() +
				ipahal_get_reg_mn_ofst(IPA_UC_MAILBOX_m_n,
					IPA_ETH_MBOX_M,
					IPA_ETH_TX_MBOX_N);
			pipe->info.db_val = IPA_ETH_TX_MBOX_VAL;
		}
	}


	/* enable data path */
	result = ipa3_enable_data_path(ep_idx);
	if (result) {
		IPAERR("enable data path failed res=%d clnt=%d\n", result,
			ep_idx);
		goto enable_data_path_fail;
	}

	/* start gsi channel */
	result = gsi_start_channel(ep->gsi_chan_hdl);
	if (result) {
		IPAERR("failed to start gsi tx channel\n");
		goto start_channel_fail;
	}

	id = (pipe->dir == IPA_ETH_PIPE_DIR_TX) ? 1 : 0;

	/* start uC gsi dbg stats monitor */
	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_5) {
		ipa3_ctx->gsi_info[prot].ch_id_info[id].ch_id
			= ep->gsi_chan_hdl;
		ipa3_ctx->gsi_info[prot].ch_id_info[id].dir
			= pipe->dir;
		ipa3_uc_debug_stats_alloc(
			ipa3_ctx->gsi_info[prot]);
	}

	ch = 0;
	if ((ipa3_ctx->ipa_hw_type == IPA_HW_v4_5) &&
			(prot == IPA_HW_PROTOCOL_AQC)) {
		enum ipa_eth_client_type type;
		u8 inst_id;
		struct ipa3_eth_info *eth_info;

		type = pipe->client_info->client_type;
		inst_id = pipe->client_info->inst_id;
		eth_info = &ipa3_ctx->eth_info[type][inst_id];
		if (!eth_info->num_ch) {
			bar_addr =
				IPA_ETH_PCIE_SET(pipe->info.client_info.aqc.bar_addr);
			result = ipa3_eth_uc_init_peripheral(true, bar_addr);
			if (result) {
				IPAERR("failed to init peripheral from uc\n");
				goto uc_init_peripheral_fail;
			}
		}
		ch = pipe->info.client_info.aqc.aqc_ch;
	}

	ipa3_eth_save_client_mapping(pipe, client_type,
		id, ep_idx, ep->gsi_chan_hdl);
	if ((ipa3_ctx->ipa_hw_type == IPA_HW_v4_5) ||
		(prot != IPA_HW_PROTOCOL_AQC)) {
		result = ipa3_eth_config_uc(true,
			prot,
			(pipe->dir == IPA_ETH_PIPE_DIR_TX)
			? IPA_ETH_TX : IPA_ETH_RX,
			ep->gsi_chan_hdl, ch);
		if (result) {
			IPAERR("failed to config uc\n");
			goto config_uc_fail;
		}
	}

	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
	return 0;

config_uc_fail:
	/* stop uC gsi dbg stats monitor */
	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_5) {
		ipa3_ctx->gsi_info[prot].ch_id_info[id].ch_id
			= 0xff;
		ipa3_ctx->gsi_info[prot].ch_id_info[id].dir
			= pipe->dir;
		ipa3_uc_debug_stats_alloc(
			ipa3_ctx->gsi_info[prot]);
	}
uc_init_peripheral_fail:
	ipa3_stop_gsi_channel(ep->gsi_chan_hdl);
start_channel_fail:
	ipa3_disable_data_path(ep_idx);
enable_data_path_fail:
query_msi_fail:
query_ch_db_fail:
setup_gsi_ch_fail:
cfg_ep_fail:
disable_data_path_fail:
	ipa3_smmu_map_eth_pipes(pipe, false);
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
	return result;
}
EXPORT_SYMBOL(ipa3_eth_connect);

int ipa3_eth_disconnect(
	struct ipa_eth_client_pipe_info *pipe,
	enum ipa_client_type client_type)
{
	int result = 0;
	struct ipa3_ep_context *ep;
	int ep_idx;
	int id;
	enum ipa4_hw_protocol prot;

	result = ipa3_eth_get_prot(pipe, &prot);
	if (result) {
		IPAERR("Could not determine protocol\n");
		return result;
	}

	if (prot == IPA_HW_PROTOCOL_ETH) {
		IPAERR("EMAC\\NTN still not supported using this framework\n");
		return -EFAULT;
	}

	IPA_ACTIVE_CLIENTS_INC_SIMPLE();
	ep_idx = ipa_get_ep_mapping(client_type);
	if (ep_idx == -1 || ep_idx >= IPA3_MAX_NUM_PIPES) {
		IPAERR("undefined client_type\n");
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		return -EFAULT;
	}
	ep = &ipa3_ctx->ep[ep_idx];
	/* disable data path */
	result = ipa3_disable_data_path(ep_idx);
	if (result) {
		IPAERR("enable data path failed res=%d clnt=%d.\n", result,
			ep_idx);
		IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
		return -EFAULT;
	}

	id = (pipe->dir == IPA_ETH_PIPE_DIR_TX) ? 1 : 0;
	/* stop uC gsi dbg stats monitor */
	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_5) {
		ipa3_ctx->gsi_info[prot].ch_id_info[id].ch_id
			= 0xff;
		ipa3_ctx->gsi_info[prot].ch_id_info[id].dir
			= pipe->dir;
		ipa3_uc_debug_stats_alloc(
			ipa3_ctx->gsi_info[prot]);
	}
	/* stop gsi channel */
	result = ipa3_stop_gsi_channel(ep_idx);
	if (result) {
		IPAERR("failed to stop gsi channel %d\n", ep_idx);
		result = -EFAULT;
		ipa_assert();
		goto fail;
	}

	if ((ipa3_ctx->ipa_hw_type == IPA_HW_v4_5) ||
		(prot != IPA_HW_PROTOCOL_AQC)) {
		result = ipa3_eth_config_uc(false,
			prot,
			(pipe->dir == IPA_ETH_PIPE_DIR_TX)
			? IPA_ETH_TX : IPA_ETH_RX,
			ep->gsi_chan_hdl, 0);
		if (result)
			IPAERR("failed to config uc\n");
	}

	/* tear down pipe */
	result = ipa3_reset_gsi_channel(ep_idx);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("failed to reset gsi channel: %d.\n", result);
		ipa_assert();
		goto fail;
	}
	result = gsi_reset_evt_ring(ep->gsi_evt_ring_hdl);
	if (result != GSI_STATUS_SUCCESS) {
		IPAERR("failed to reset evt ring: %d.\n", result);
		ipa_assert();
		goto fail;
	}
	result = ipa3_release_gsi_channel(ep_idx);
	if (result) {
		IPAERR("failed to release gsi channel: %d\n", result);
		ipa_assert();
		goto fail;
	}
	memset(ep, 0, sizeof(struct ipa3_ep_context));
	IPADBG("client (ep: %d) disconnected\n", ep_idx);

	if (ipa3_ctx->ipa_hw_type >= IPA_HW_v4_5)
		ipa3_uc_debug_stats_dealloc(prot);
	if (IPA_CLIENT_IS_PROD(client_type))
		ipa3_delete_dflt_flt_rules(ep_idx);
	/* unmap th pipe */
	result = ipa3_smmu_map_eth_pipes(pipe, false);
	if (result)
		IPAERR("failed to unmap SMMU %d\n", result);
	ipa3_eth_release_client_mapping(pipe, id);

	if ((ipa3_ctx->ipa_hw_type == IPA_HW_v4_5) &&
		(prot == IPA_HW_PROTOCOL_AQC)) {
		enum ipa_eth_client_type type;
		u8 inst_id;
		struct ipa3_eth_info *eth_info;

		type = pipe->client_info->client_type;
		inst_id = pipe->client_info->inst_id;
		eth_info = &ipa3_ctx->eth_info[type][inst_id];
		if (!eth_info->num_ch) {
			result = ipa3_eth_uc_init_peripheral(false, 0);
			if (result) {
				IPAERR("failed to de-init peripheral %d\n", result);
				goto fail;
			}
		}
	}
fail:
	IPA_ACTIVE_CLIENTS_DEC_SIMPLE();
	return result;
}
EXPORT_SYMBOL(ipa3_eth_disconnect);
