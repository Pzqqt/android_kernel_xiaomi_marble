// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2021, The Linux Foundation. All rights reserved.
 */

#include "ipa_ut_framework.h"
#include <linux/ipa_eth.h>
#include <linux/ipa.h>
#include <linux/delay.h>
#include "ipa_i.h"

#define NUM_TX_BUFS 10
#define NUM_RX_BUFS 10
#define NUM_RX_TR_ELE NUM_RX_BUFS
#define NUM_TX_TR_ELE NUM_TX_BUFS

#define PACKET_HEADER_SIZE 220
#define ETH_PACKET_SIZE 4
#define PACKET_CONTENT 0x12345678

#define BUFFER_SIZE 2048 /* 2K */

#define DB_REGISTER_SIZE 4
#define RX_TAIL_PTR_OFF 0
#define TX_TAIL_PTR_OFF 8

#define IPA_TEST_NTN_NUM_PIPES 2

struct ipa_test_ntn_context {
	struct completion init_completion_obj;
	bool ready;
	int wait_cnt;
	struct ipa_eth_client client;
	struct ipa_eth_client_pipe_info rx_pipe_info;
	struct ipa_eth_client_pipe_info tx_pipe_info;
	struct ipa_mem_buffer tx_transfer_ring_addr;
	struct sg_table *tx_transfer_ring_sgt;
	struct ipa_mem_buffer rx_transfer_ring_addr;
	struct sg_table *rx_transfer_ring_sgt;
	struct ipa_mem_buffer tx_buf;
	struct sg_table *tx_buff_sgt;
	struct ipa_eth_buff_smmu_map tx_data_buff_list[NUM_TX_BUFS];
	struct ipa_mem_buffer rx_buf;
	struct sg_table *rx_buff_sgt;
	struct ipa_mem_buffer bar_addr;
	int rx_db_local;
	int tx_db_local;
	int rx_idx;
	int tx_idx;
	enum ipa_client_type cons_client_type;
	enum ipa_client_type prod_client_type;
	int eth_client_inst_id;
};

static struct ipa_test_ntn_context *test_ntn_ctx;

/* TRE written by NTN (SW) */
struct tx_transfer_ring_ele {
	uint32_t res1;
	uint32_t res2;
	uint32_t res3;
	uint32_t res4: 27;
	uint32_t desc_status : 4;
	uint32_t own : 1;
}__packed;

/* event written by GSI */
struct tx_event_ring_ele {
	uint32_t buff_addr_LSB;
	uint32_t buff_addr_MSB;
	uint32_t buffer_length : 14;
	uint32_t reserved1 : 17;
	uint32_t ioc : 1;
	uint32_t reserved2 : 28;
	uint32_t ld : 1;
	uint32_t fd : 1;
	uint32_t reserved3 : 1;
	uint32_t own : 1;

}__packed;

/* TRE written by NTN (SW) */
struct rx_transfer_ring_ele
{
	uint32_t reserved1;
	uint32_t reserved2;
	uint32_t reserved3;
	uint32_t packet_length : 14;
	uint32_t reserverd4 : 14;
	uint32_t ld : 1;
	uint32_t fd : 1;
	uint32_t reserved5 : 1;
	uint32_t own : 1;
}__packed;

/* event written by GSI */
struct rx_event_ring_ele
{
	uint32_t buff_addr1;
	uint32_t res_or_buff_addr1;
	uint32_t buff_addr2;
	uint32_t res_or_buff_addr2 : 30;
	uint32_t ioc : 1;
	uint32_t own : 1;
}__packed;

static inline void ipa_test_ntn_set_client_params(enum ipa_client_type cons_type,
	enum ipa_client_type prod_type, int inst_id)
{
	test_ntn_ctx->cons_client_type = cons_type;
	test_ntn_ctx->prod_client_type = prod_type;
	test_ntn_ctx->eth_client_inst_id = inst_id;
}

static void ipa_test_ntn_free_dma_buff(struct ipa_mem_buffer *mem)
{
	struct ipa_smmu_cb_ctx *cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_WLAN);

	if (!mem) {
		IPA_UT_ERR("empty pointer\n");
		return;
	}

	dma_free_coherent(cb->dev, mem->size, mem->base,
		mem->phys_base);
}

static int ipa_test_ntn_alloc_mmio(void)
{
	int ret = 0;
	u32 size;
	struct ipa_smmu_cb_ctx *cb = ipa3_get_smmu_ctx(IPA_SMMU_CB_WLAN);

	if (!test_ntn_ctx) {
		IPA_UT_ERR("test_ntn_ctx is not initialized.\n");
		return -EFAULT;
	}

	/* allocate tx transfer ring memory */
	size = NUM_TX_TR_ELE * sizeof(struct tx_transfer_ring_ele);
	test_ntn_ctx->tx_transfer_ring_addr.size = size;
	test_ntn_ctx->tx_transfer_ring_addr.base =
		dma_alloc_coherent(cb->dev, size,
			&test_ntn_ctx->tx_transfer_ring_addr.phys_base,
			GFP_KERNEL);
	if (!test_ntn_ctx->tx_transfer_ring_addr.phys_base) {
		IPA_UT_ERR("fail to alloc memory.\n");
		return -ENOMEM;
	}

	test_ntn_ctx->tx_transfer_ring_sgt = kzalloc(
		sizeof(test_ntn_ctx->tx_transfer_ring_sgt), GFP_KERNEL);
	if (!test_ntn_ctx->tx_transfer_ring_sgt) {
		IPA_UT_ERR("fail to alloc memory.\n");
		ret = -ENOMEM;
		goto fail_alloc_tx_sgt;
	}

	ret = dma_get_sgtable(cb->dev,
		test_ntn_ctx->tx_transfer_ring_sgt,
		test_ntn_ctx->tx_transfer_ring_addr.base,
		test_ntn_ctx->tx_transfer_ring_addr.phys_base,
		size);
	if (ret) {
		IPA_UT_ERR("failed to get sgtable\n");
		ret = -ENOMEM;
		goto fail_get_tx_sgtable;
	}

	/* allocate rx transfer ring memory */
	size = NUM_RX_TR_ELE * sizeof(struct rx_transfer_ring_ele);
	test_ntn_ctx->rx_transfer_ring_addr.size = size;
	test_ntn_ctx->rx_transfer_ring_addr.base =
		dma_alloc_coherent(cb->dev, size,
			&test_ntn_ctx->rx_transfer_ring_addr.phys_base,
			GFP_KERNEL);
	if (!test_ntn_ctx->rx_transfer_ring_addr.phys_base) {
		IPA_UT_ERR("fail to alloc memory.\n");
		ret = -ENOMEM;
		goto fail_rx_transfer_ring;
	}

	test_ntn_ctx->rx_transfer_ring_sgt = kzalloc(
		sizeof(test_ntn_ctx->rx_transfer_ring_sgt), GFP_KERNEL);
	if (!test_ntn_ctx->rx_transfer_ring_sgt) {
		IPA_UT_ERR("fail to alloc memory.\n");
		ret = -ENOMEM;
		goto fail_alloc_rx_sgt;
	}

	ret = dma_get_sgtable(cb->dev,
		test_ntn_ctx->rx_transfer_ring_sgt,
		test_ntn_ctx->rx_transfer_ring_addr.base,
		test_ntn_ctx->rx_transfer_ring_addr.phys_base,
		size);
	if (ret) {
		IPA_UT_ERR("failed to get sgtable\n");
		ret = -ENOMEM;
		goto fail_get_rx_sgtable;
	}

	/* allocate tx buffers */
	size = BUFFER_SIZE * NUM_TX_BUFS;
	test_ntn_ctx->tx_buf.size = size;
	test_ntn_ctx->tx_buf.base =
		dma_alloc_coherent(cb->dev, size,
			&test_ntn_ctx->tx_buf.phys_base,
			GFP_KERNEL);
	if (!test_ntn_ctx->tx_buf.phys_base) {
		IPA_UT_ERR("fail to alloc memory.\n");
		ret = -ENOMEM;
		goto fail_tx_buf;
	}

	test_ntn_ctx->tx_buff_sgt = kzalloc(
		sizeof(test_ntn_ctx->tx_buff_sgt), GFP_KERNEL);
	if (!test_ntn_ctx->tx_buff_sgt) {
		IPA_UT_ERR("fail to alloc memory.\n");
		ret = -ENOMEM;
		goto fail_alloc_tx_buff_sgt;
	}

	ret = dma_get_sgtable(cb->dev,
		test_ntn_ctx->tx_buff_sgt,
		test_ntn_ctx->tx_buf.base,
		test_ntn_ctx->tx_buf.phys_base,
		size);
	if (ret) {
		IPA_UT_ERR("failed to get sgtable\n");
		ret = -ENOMEM;
		goto fail_get_tx_buf_sgtable;
	}

	/* allocate rx buffers */
	size = BUFFER_SIZE * NUM_RX_BUFS;
	test_ntn_ctx->rx_buf.size = size;
	test_ntn_ctx->rx_buf.base =
		dma_alloc_coherent(cb->dev, size,
			&test_ntn_ctx->rx_buf.phys_base,
			GFP_KERNEL);
	if (!test_ntn_ctx->rx_buf.phys_base) {
		IPA_UT_ERR("fail to alloc memory.\n");
		ret = -ENOMEM;
		goto fail_rx_bufs;
	}

	test_ntn_ctx->rx_buff_sgt = kzalloc(
		sizeof(test_ntn_ctx->rx_buff_sgt), GFP_KERNEL);
	if (!test_ntn_ctx->rx_buff_sgt) {
		IPA_UT_ERR("fail to alloc memory.\n");
		ret = -ENOMEM;
		goto fail_alloc_rx_buff_sgt;
	}

	ret = dma_get_sgtable(cb->dev,
		test_ntn_ctx->rx_buff_sgt,
		test_ntn_ctx->rx_buf.base,
		test_ntn_ctx->rx_buf.phys_base,
		size);
	if (ret) {
		IPA_UT_ERR("failed to get sgtable\n");
		ret = -ENOMEM;
		goto fail_get_rx_buf_sgtable;
	}

	/*
	 * allocate PCI bar with two tail pointers -
	 * addresses need to be 8B aligned
	 */
	test_ntn_ctx->bar_addr.size = 2 * DB_REGISTER_SIZE + 8;
	test_ntn_ctx->bar_addr.base =
		dma_alloc_coherent(ipa3_ctx->pdev,
			test_ntn_ctx->bar_addr.size,
			&test_ntn_ctx->bar_addr.phys_base,
			GFP_KERNEL);
	if (!test_ntn_ctx->bar_addr.base) {
		IPA_UT_ERR("fail to alloc memory\n");
		ret = -ENOMEM;
		goto fail_alloc_bar;
	}

	return ret;

fail_alloc_bar:
	sg_free_table(test_ntn_ctx->rx_buff_sgt);
fail_get_rx_buf_sgtable:
	kfree(test_ntn_ctx->rx_buff_sgt);
	test_ntn_ctx->rx_buff_sgt = NULL;
fail_alloc_rx_buff_sgt:
	ipa_test_ntn_free_dma_buff(&test_ntn_ctx->rx_buf);
fail_rx_bufs:
	sg_free_table(test_ntn_ctx->tx_buff_sgt);
fail_get_tx_buf_sgtable:
	kfree(test_ntn_ctx->tx_buff_sgt);
	test_ntn_ctx->tx_buff_sgt = NULL;
fail_alloc_tx_buff_sgt:
	ipa_test_ntn_free_dma_buff(&test_ntn_ctx->tx_buf);
fail_tx_buf:
	sg_free_table(test_ntn_ctx->rx_transfer_ring_sgt);

fail_get_rx_sgtable:
	kfree(test_ntn_ctx->rx_transfer_ring_sgt);
	test_ntn_ctx->rx_transfer_ring_sgt = NULL;
fail_alloc_rx_sgt:
	ipa_test_ntn_free_dma_buff(&test_ntn_ctx->rx_transfer_ring_addr);
fail_rx_transfer_ring:
	sg_free_table(test_ntn_ctx->tx_transfer_ring_sgt);
fail_get_tx_sgtable:
	kfree(test_ntn_ctx->tx_transfer_ring_sgt);
	test_ntn_ctx->tx_transfer_ring_sgt = NULL;
fail_alloc_tx_sgt:
	ipa_test_ntn_free_dma_buff(&test_ntn_ctx->tx_transfer_ring_addr);
	return ret;
}

static int ipa_test_ntn_free_mmio(void)
{
	if (!test_ntn_ctx) {
		IPA_UT_ERR("test_ntn_ctx is not initialized.\n");
		return -EFAULT;
	}

	/* rx buff */
	sg_free_table(test_ntn_ctx->rx_buff_sgt);
	kfree(test_ntn_ctx->rx_buff_sgt);
	test_ntn_ctx->rx_buff_sgt = NULL;
	ipa_test_ntn_free_dma_buff(&test_ntn_ctx->rx_buf);

	/* tx buff */
	sg_free_table(test_ntn_ctx->tx_buff_sgt);
	kfree(test_ntn_ctx->tx_buff_sgt);
	test_ntn_ctx->tx_buff_sgt = NULL;
	ipa_test_ntn_free_dma_buff(&test_ntn_ctx->tx_buf);

	/* rx transfer ring */
	sg_free_table(test_ntn_ctx->rx_transfer_ring_sgt);
	kfree(test_ntn_ctx->rx_transfer_ring_sgt);
	test_ntn_ctx->rx_transfer_ring_sgt = NULL;
	ipa_test_ntn_free_dma_buff(&test_ntn_ctx->rx_transfer_ring_addr);

	/* tx transfer ring */
	sg_free_table(test_ntn_ctx->tx_transfer_ring_sgt);
	kfree(test_ntn_ctx->tx_transfer_ring_sgt);
	test_ntn_ctx->tx_transfer_ring_sgt = NULL;
	ipa_test_ntn_free_dma_buff(&test_ntn_ctx->tx_transfer_ring_addr);

	return 0;
}

static void ipa_test_ntn_ready_cb(void *user_data)
{
	IPA_UT_DBG("ready CB entry\n");
	test_ntn_ctx->ready = true;
	complete(&test_ntn_ctx->init_completion_obj);
}

static struct ipa_eth_ready eth_ready = {
	.notify = ipa_test_ntn_ready_cb,
	.userdata = NULL
};

static int ipa_test_ntn_init_rings(void)
{
	struct tx_transfer_ring_ele *tx_ele;
	struct rx_transfer_ring_ele *rx_ele;
	int i;

	IPA_UT_DBG("filling the rings\n");

	rx_ele =
		(struct rx_transfer_ring_ele *)
		(test_ntn_ctx->rx_transfer_ring_addr.base);

	tx_ele =
		(struct tx_transfer_ring_ele *)
		(test_ntn_ctx->tx_transfer_ring_addr.base);

	memset(rx_ele, 0, sizeof(*rx_ele) * NUM_RX_TR_ELE);

	for (i = 0; i < NUM_RX_TR_ELE; i++) {
		rx_ele->fd = 1;
		rx_ele->ld = 1;
		rx_ele++;
	}

	/* all fields should be zero */
	memset(tx_ele, 0, sizeof(*tx_ele) * NUM_TX_TR_ELE);

	return 0;
}

static int ipa_test_ntn_suite_setup(void **priv)
{
	int ret = 0;

	IPA_UT_DBG("Start NTN Setup\n");

	/* init ipa ntn ctx */
	if (!ipa3_ctx) {
		IPA_UT_ERR("No IPA ctx\n");
		return -EINVAL;
	}

	test_ntn_ctx = kzalloc(sizeof(struct ipa_test_ntn_context),
		GFP_KERNEL);
	if (!test_ntn_ctx) {
		IPA_UT_ERR("failed to allocate ctx\n");
		return -ENOMEM;
	}

	ipa_test_ntn_set_client_params(IPA_CLIENT_ETHERNET_CONS, IPA_CLIENT_ETHERNET_PROD, 0);

	init_completion(&test_ntn_ctx->init_completion_obj);

	/*
	 * registering ready callback mandatory for init. CB shall be launched
	 * anyway so connect the pipe from there.
	 * our driver expects struct memory to be static as it uses it when CB
	 * is launched.
	 */
	ret = ipa_eth_register_ready_cb(&eth_ready);
	if (ret) {
		IPA_UT_ERR("failed to register CB\n");
		goto fail_alloc_mmio;
	}

	IPA_UT_DBG("IPA %s ready\n", eth_ready.is_eth_ready ? "is" : "is not");

	ret = ipa_test_ntn_alloc_mmio();
	if (ret) {
		IPA_UT_ERR("failed to alloc mmio\n");
		goto fail_alloc_mmio;
	}

	*priv = test_ntn_ctx;
	return 0;

fail_alloc_mmio:
	kfree(test_ntn_ctx);
	test_ntn_ctx = NULL;
	return ret;
}

static void ipa_ntn_test_print_stats()
{
	struct ipa_uc_dbg_ring_stats stats;
	int ret;
	int tx_ep, rx_ep;
	struct ipa3_eth_error_stats tx_stats;
	struct ipa3_eth_error_stats rx_stats;

	/* first get uC stats */
	ret = ipa3_get_ntn_gsi_stats(&stats);
	if (ret) {
		IPA_UT_ERR("failed to get stats\n");
		return;
	}
	IPA_UT_INFO("\nuC stats:\n");
	IPA_UT_INFO(
	"NTN_tx_ringFull=%u\n"
	"NTN_tx_ringEmpty=%u\n"
	"NTN_tx_ringUsageHigh=%u\n"
	"NTN_tx_ringUsageLow=%u\n"
	"NTN_tx_RingUtilCount=%u\n",
	stats.u.ring[1].ringFull,
	stats.u.ring[1].ringEmpty,
	stats.u.ring[1].ringUsageHigh,
	stats.u.ring[1].ringUsageLow,
	stats.u.ring[1].RingUtilCount);

	IPA_UT_INFO(
	"NTN_rx_ringFull=%u\n"
	"NTN_rx_ringEmpty=%u\n"
	"NTN_rx_ringUsageHigh=%u\n"
	"NTN_rx_ringUsageLow=%u\n"
	"NTN_rx_RingUtilCount=%u\n",
	stats.u.ring[0].ringFull,
	stats.u.ring[0].ringEmpty,
	stats.u.ring[0].ringUsageHigh,
	stats.u.ring[0].ringUsageLow,
	stats.u.ring[0].RingUtilCount);

	/* now get gsi stats */
	tx_ep = test_ntn_ctx->cons_client_type;
	rx_ep = test_ntn_ctx->prod_client_type;
	ipa3_eth_get_status(tx_ep, 6, &tx_stats);
	ipa3_eth_get_status(rx_ep, 6, &rx_stats);

	IPA_UT_INFO("\nGSI stats:\n");
	IPA_UT_INFO(
		"NTN_TX_RP=0x%x\n"
		"NTN_TX_WP=0x%x\n"
		"NTN_TX_err=%u\n",
		tx_stats.rp,
		tx_stats.wp,
		tx_stats.err);

	IPA_UT_INFO(
		"NTN_RX_RP=0x%x\n"
		"NTN_RX_WP=0x%x\n"
		"NTN_RX_err:%u\n",
		rx_stats.rp,
		rx_stats.wp,
		rx_stats.err);
}

static int ipa_test_ntn_suite_teardown(void *priv)
{
	if (!test_ntn_ctx)
		return  0;

	ipa_test_ntn_free_mmio();
	kfree(test_ntn_ctx);
	test_ntn_ctx = NULL;

	return 0;
}

static int ipa_ntn_test_ready_cb(void *priv)
{
	int ret;

	test_ntn_ctx->wait_cnt++;
	ret = wait_for_completion_timeout(
		&test_ntn_ctx->init_completion_obj,
		msecs_to_jiffies(1000));
	if (!ret) {
		IPA_UT_ERR("ipa ready timeout, don't run\n");
		return -EFAULT;
	}

	return 0;
}

static void ipa_ntn_test_del_client_list()
{
	struct ipa_eth_client *eth_client = &test_ntn_ctx->client;
	struct ipa_eth_client_pipe_info *pipe_info, *tmp;

	list_for_each_entry_safe(pipe_info, tmp, &eth_client->pipe_list, link)
		list_del(&pipe_info->link);
}

static int ipa_ntn_test_setup_pipes(void)
{
	struct ipa_eth_client *client;
	int ret, i;
#if IPA_ETH_API_VER >= 2
	struct net_device dummy_net_dev;
	unsigned char dummy_dev_addr = 1;

	memset(dummy_net_dev.name, 0, sizeof(dummy_net_dev.name));
	dummy_net_dev.dev_addr = &dummy_dev_addr;

	test_ntn_ctx->client.client_type = IPA_ETH_CLIENT_NTN3;
	test_ntn_ctx->client.inst_id = test_ntn_ctx->eth_client_inst_id;
#else
	test_ntn_ctx->client.client_type = IPA_ETH_CLIENT_NTN;
	test_ntn_ctx->client.inst_id = 0;
#endif
	test_ntn_ctx->client.traffic_type = IPA_ETH_PIPE_BEST_EFFORT;
#if IPA_ETH_API_VER >= 2
	test_ntn_ctx->client.net_dev = &dummy_net_dev;
#endif

	/* RX pipe */
	/* ring */
	test_ntn_ctx->rx_pipe_info.dir = IPA_ETH_PIPE_DIR_RX;
	test_ntn_ctx->rx_pipe_info.client_info = &test_ntn_ctx->client;
	test_ntn_ctx->rx_pipe_info.info.is_transfer_ring_valid = true;
	test_ntn_ctx->rx_pipe_info.info.transfer_ring_base =
		test_ntn_ctx->rx_transfer_ring_addr.phys_base;
	test_ntn_ctx->rx_pipe_info.info.transfer_ring_size =
		test_ntn_ctx->rx_transfer_ring_addr.size;
	test_ntn_ctx->rx_pipe_info.info.transfer_ring_sgt =
		test_ntn_ctx->rx_transfer_ring_sgt;

	IPA_UT_DBG("rx TR phys 0x%X, cpu 0x%X, size %d, sgt 0x%X\n",
		test_ntn_ctx->rx_transfer_ring_addr.phys_base,
		test_ntn_ctx->rx_transfer_ring_addr.base,
		test_ntn_ctx->rx_transfer_ring_addr.size,
		test_ntn_ctx->rx_transfer_ring_sgt);

	/* buff */
	test_ntn_ctx->rx_pipe_info.info.is_buffer_pool_valid = true;
	test_ntn_ctx->rx_pipe_info.info.fix_buffer_size = BUFFER_SIZE;
	test_ntn_ctx->rx_pipe_info.info.buffer_pool_base_addr =
		test_ntn_ctx->rx_buf.phys_base;
	test_ntn_ctx->rx_pipe_info.info.buffer_pool_base_sgt =
		test_ntn_ctx->rx_buff_sgt;

	IPA_UT_DBG("rx buff phys 0x%X, cpu 0x%X, size %d, fix size %d sgt 0x%X\n"
		, test_ntn_ctx->rx_buf.phys_base,
		test_ntn_ctx->rx_buf.base,
		test_ntn_ctx->rx_buf.size,
		test_ntn_ctx->rx_pipe_info.info.fix_buffer_size,
		test_ntn_ctx->rx_buff_sgt);

	/* we don't plan to recieve skb on RX CB */
	test_ntn_ctx->rx_pipe_info.info.notify = NULL;
	test_ntn_ctx->rx_pipe_info.info.priv = NULL;

	/* gsi info */
	test_ntn_ctx->rx_pipe_info.info.client_info.ntn.bar_addr =
		test_ntn_ctx->bar_addr.phys_base;

	/*
	* use the first 4 bytes as the RX tail_ptr and the next 4 for TX,
	* make sure 8B alignment
	*/
	test_ntn_ctx->rx_pipe_info.info.client_info.ntn.tail_ptr_offs =
		RX_TAIL_PTR_OFF;

	IPA_UT_DBG("tail registers bar: phys 0x%X virt 0x%X\n",
		test_ntn_ctx->bar_addr.phys_base, test_ntn_ctx->bar_addr.base);

	/* TX pipe */
	/* ring */
	test_ntn_ctx->tx_pipe_info.dir = IPA_ETH_PIPE_DIR_TX;
	test_ntn_ctx->tx_pipe_info.client_info = &test_ntn_ctx->client;
	test_ntn_ctx->tx_pipe_info.info.is_transfer_ring_valid = true;
	test_ntn_ctx->tx_pipe_info.info.transfer_ring_base =
		test_ntn_ctx->tx_transfer_ring_addr.phys_base;
	test_ntn_ctx->tx_pipe_info.info.transfer_ring_size =
		test_ntn_ctx->tx_transfer_ring_addr.size;
	test_ntn_ctx->tx_pipe_info.info.transfer_ring_sgt =
		test_ntn_ctx->tx_transfer_ring_sgt;

	IPA_UT_DBG("tx TR phys 0x%X, cpu 0x%X, size %d, sgt 0x%X\n",
		test_ntn_ctx->tx_transfer_ring_addr.phys_base,
		test_ntn_ctx->tx_transfer_ring_addr.base,
		test_ntn_ctx->tx_transfer_ring_addr.size,
		test_ntn_ctx->tx_transfer_ring_sgt);

	/* buff - for tx let's use the buffer list method (test both methods) */
	test_ntn_ctx->tx_pipe_info.info.is_buffer_pool_valid = false;
	test_ntn_ctx->tx_pipe_info.info.fix_buffer_size = BUFFER_SIZE;
	test_ntn_ctx->tx_pipe_info.info.data_buff_list =
		test_ntn_ctx->tx_data_buff_list;
	for (i = 0; i < NUM_TX_BUFS; i++) {
		test_ntn_ctx->tx_pipe_info.info.data_buff_list[i].iova =
			(phys_addr_t)((u8 *)test_ntn_ctx->tx_buf.phys_base +
				i * BUFFER_SIZE);
		test_ntn_ctx->tx_pipe_info.info.data_buff_list[i].pa =
			page_to_phys(vmalloc_to_page(test_ntn_ctx->tx_buf.base
				+ (BUFFER_SIZE * i))) |
			((phys_addr_t)(test_ntn_ctx->tx_buf.base +
			(BUFFER_SIZE * i)) & ~PAGE_MASK);

		IPA_UT_DBG("tx_pipe_info.info.data_buff_list[%d].iova = 0x%lx",
			i,
			test_ntn_ctx->tx_pipe_info.info.data_buff_list[i].iova);
		IPA_UT_DBG("tx_pipe_info.info.data_buff_list[%d].pa = 0x%lx",
			i,
			test_ntn_ctx->tx_pipe_info.info.data_buff_list[i].pa);
	}
	test_ntn_ctx->tx_pipe_info.info.data_buff_list_size = NUM_TX_BUFS;

	IPA_UT_DBG("tx buff phys 0x%X, cpu 0x%X, size %d, fix size %d sgt 0x%X\n"
		, test_ntn_ctx->tx_buf.phys_base,
		test_ntn_ctx->tx_buf.base,
		test_ntn_ctx->tx_buf.size,
		test_ntn_ctx->tx_pipe_info.info.fix_buffer_size,
		test_ntn_ctx->tx_buff_sgt);

	test_ntn_ctx->tx_pipe_info.info.notify = NULL;
	test_ntn_ctx->tx_pipe_info.info.priv = NULL;

	test_ntn_ctx->tx_pipe_info.info.client_info.ntn.bar_addr =
		test_ntn_ctx->bar_addr.phys_base;

	/*
	 * use the first 4 bytes as the RX tail_ptr and the next 4 for TX,
	 * make sure 8B alignment
	 */
	test_ntn_ctx->tx_pipe_info.info.client_info.ntn.tail_ptr_offs =
		TX_TAIL_PTR_OFF;

	/* add pipes to list */
	INIT_LIST_HEAD(&test_ntn_ctx->client.pipe_list);
	list_add(&test_ntn_ctx->rx_pipe_info.link,
		&test_ntn_ctx->client.pipe_list);
	list_add(&test_ntn_ctx->tx_pipe_info.link,
		&test_ntn_ctx->client.pipe_list);

	test_ntn_ctx->client.test = true;
	client = &test_ntn_ctx->client;
	ret = ipa_eth_client_conn_pipes(client);
	if(ret) {
		IPA_UT_ERR("ipa_eth_client_conn_pipes failed ret %d\n", ret);
		goto conn_failed;
	}

	return 0;

conn_failed:
	ipa_ntn_test_del_client_list();
	return ret;
}

static int ipa_ntn_test_reg_intf(void)
{
	struct ipa_eth_intf_info intf;
#if IPA_ETH_API_VER >= 2
	struct net_device dummy_net_dev;
	unsigned char dummy_dev_addr[ETH_ALEN] = { 0 };
#else
	char netdev_name[IPA_RESOURCE_NAME_MAX] = { 0 };
	u8 hdr_content = 1;
#endif
	int ret = 0;

	memset(&intf, 0, sizeof(intf));
#if IPA_ETH_API_VER >= 2
	memset(dummy_net_dev.name, 0, sizeof(dummy_net_dev.name));

	intf.net_dev = &dummy_net_dev;
	intf.net_dev->dev_addr = (unsigned char *)dummy_dev_addr;
	intf.is_conn_evt = true;

	snprintf(intf.net_dev->name, sizeof(intf.net_dev->name), "ntn_test");
	IPA_UT_INFO("netdev name: %s strlen: %lu\n", intf.net_dev->name, strlen(intf.net_dev->name));
#else
	snprintf(netdev_name, sizeof(netdev_name), "ntn_test");
	intf.netdev_name = netdev_name;
	IPA_UT_INFO("netdev name: %s strlen: %lu\n", intf.netdev_name,
		strlen(intf.netdev_name));

	intf.hdr[0].hdr = &hdr_content;
	intf.hdr[0].hdr_len = 1;
	intf.hdr[0].dst_mac_addr_offset = 0;
	intf.hdr[0].hdr_type = IPA_HDR_L2_ETHERNET_II;

	intf.hdr[1].hdr = &hdr_content;
	intf.hdr[1].hdr_len = 1;
	intf.hdr[1].dst_mac_addr_offset = 0;
	intf.hdr[1].hdr_type = IPA_HDR_L2_ETHERNET_II;

	intf.pipe_hdl_list =
		kcalloc(IPA_TEST_NTN_NUM_PIPES,
			sizeof(*intf.pipe_hdl_list),
			GFP_KERNEL);
	if (!intf.pipe_hdl_list) {
		IPA_UT_ERR("Failed to alloc pipe handle list");
		return -ENOMEM;
	}

	intf.pipe_hdl_list[0] = test_ntn_ctx->rx_pipe_info.pipe_hdl;
	intf.pipe_hdl_list[1] = test_ntn_ctx->tx_pipe_info.pipe_hdl;
	intf.pipe_hdl_list_size = IPA_TEST_NTN_NUM_PIPES;
#endif

	ret = ipa_eth_client_reg_intf(&intf);
	if (ret) {
		IPA_UT_ERR("Failed to register IPA interface");
	}

#if IPA_ETH_API_VER >= 2
#else
	kfree(intf.pipe_hdl_list);
#endif

	return ret;
}

static int ipa_ntn_test_unreg_intf(void)
{
	struct ipa_eth_intf_info intf;
#if IPA_ETH_API_VER >= 2
	struct net_device dummy_net_dev;
#else
	char netdev_name[IPA_RESOURCE_NAME_MAX] = { 0 };
#endif

	memset(&intf, 0, sizeof(intf));
#if IPA_ETH_API_VER >= 2
	memset(dummy_net_dev.name, 0, sizeof(dummy_net_dev.name));

	intf.net_dev = &dummy_net_dev;

	snprintf(intf.net_dev->name, sizeof(intf.net_dev->name), "ntn_test");
	IPA_UT_INFO("netdev name: %s strlen: %lu\n", intf.net_dev->name, strlen(intf.net_dev->name));
#else
	snprintf(netdev_name, sizeof(netdev_name), "ntn_test");
	intf.netdev_name = netdev_name;
	IPA_UT_INFO("netdev name: %s strlen: %lu\n", intf.netdev_name,
		strlen(intf.netdev_name));
#endif

	return (ipa_eth_client_unreg_intf(&intf));
}

static void ipa_ntn_test_advance_db(u32 *db, int steps,
	int num_words, int ring_size)
{
	*db = (*db + steps * num_words) % ring_size;
}

static int ipa_ntn_send_one_packet(void)
{
	u32 *packet;
	u32 *packet_recv;

	void __iomem *rx_db;
	void __iomem *tx_db;
	struct rx_event_ring_ele *rx_event;
	u32 *tx_ring_tail;
	u32 orig_tx_tail;
	u32 *rx_ring_tail;
	u32 orig_rx_tail;
	int loop_cnt;
	u64 evt_addr;
	u64 pkt_addr;
	struct rx_transfer_ring_ele *rx_ele;

	int ret = 0;

	rx_db = ioremap(
		test_ntn_ctx->rx_pipe_info.info.db_pa, DB_REGISTER_SIZE);
	if (!rx_db) {
		IPA_UT_ERR("ioremap failed");
		return ret;
	}

	tx_db = ioremap(
		test_ntn_ctx->tx_pipe_info.info.db_pa, DB_REGISTER_SIZE);
	if (!tx_db) {
		IPA_UT_ERR("ioremap failed");
		return ret;
	}

	/* initialize packet */
	packet = (u32 *)((u8 *)test_ntn_ctx->rx_buf.base +
		(test_ntn_ctx->rx_idx * BUFFER_SIZE));
	pkt_addr = (u64)((u8 *)test_ntn_ctx->rx_buf.phys_base +
		(test_ntn_ctx->rx_idx * BUFFER_SIZE));
	*packet = PACKET_CONTENT;

	/* update length in TRE */
	rx_ele = (struct rx_transfer_ring_ele *)
		test_ntn_ctx->rx_transfer_ring_addr.base + test_ntn_ctx->rx_idx;
	rx_ele->packet_length = ETH_PACKET_SIZE;

	/* point to base + 1 */
	ipa_ntn_test_advance_db(&test_ntn_ctx->rx_db_local, 1,
		sizeof(struct rx_transfer_ring_ele),
		test_ntn_ctx->rx_transfer_ring_addr.size);

	tx_ring_tail = (u32 *)((char *)test_ntn_ctx->bar_addr.base +
		TX_TAIL_PTR_OFF);
	orig_tx_tail = *tx_ring_tail;
	rx_ring_tail = (u32 *)((char *)test_ntn_ctx->bar_addr.base +
		RX_TAIL_PTR_OFF);
	orig_rx_tail = *rx_ring_tail;

	IPA_UT_DBG("orig tx tail 0x%X\n", orig_tx_tail);
	IPA_UT_DBG("orig rx tail 0x%X\n", orig_rx_tail);

	/* ring db and send packet */
	iowrite32(test_ntn_ctx->rx_db_local +
		lower_32_bits(test_ntn_ctx->rx_transfer_ring_addr.phys_base),
		rx_db);
	IPA_UT_DBG("rx_db_local increased to 0x%X\n",
		test_ntn_ctx->rx_db_local +
		lower_32_bits(test_ntn_ctx->rx_transfer_ring_addr.phys_base));

	loop_cnt = 0;
	while ((orig_rx_tail == *rx_ring_tail) ||
		(orig_tx_tail == *tx_ring_tail)) {
		loop_cnt++;
		if (loop_cnt == 1000) {
			IPA_UT_ERR("transfer timeout!\n");
			IPA_UT_ERR("orig_tx_tail: %X tx_ring_db: %X\n",
				orig_tx_tail, *tx_ring_tail);
			IPA_UT_ERR("orig_rx_tail: %X rx_ring_db: %X\n",
				orig_rx_tail, *rx_ring_tail);
			IPA_UT_ERR("rx db local: %u\n",
				test_ntn_ctx->rx_db_local +
				lower_32_bits(
				test_ntn_ctx->rx_transfer_ring_addr.phys_base));
			BUG();
			ret = -EFAULT;
			goto err;
		}
		usleep_range(1000, 1001);
	}
	IPA_UT_DBG("loop_cnt %d\n", loop_cnt);
	IPA_UT_DBG("rx ring tail 0x%X\n", *rx_ring_tail);
	IPA_UT_DBG("tx ring tail 0x%X\n", *tx_ring_tail);

	/* verify RX event */
	rx_event = (struct rx_event_ring_ele *)rx_ele;

	IPA_UT_DBG("address written by GSI is 0x[%X][%X]\n",
		rx_event->buff_addr2, rx_event->buff_addr1);
	IPA_UT_DBG("own bit is now %u", rx_event->own);

	if (!rx_event->own) {
		IPA_UT_ERR("own bit not modified by gsi - failed\n");
		ret = -EFAULT;
	}

	evt_addr = ((u64)rx_event->buff_addr2 << 32) |
		(u64)(rx_event->buff_addr1);
	IPA_UT_DBG("RX: addr from event 0x%llx, address from buff %llx\n",
		evt_addr, pkt_addr);
	if (evt_addr != pkt_addr) {
		IPA_UT_ERR("addresses are different - fail\n");
		ret = -EFAULT;
	}

	/* read received packet */
	packet_recv = (u32 *)((u8 *)test_ntn_ctx->tx_buf.base +
		(test_ntn_ctx->tx_idx * BUFFER_SIZE));
	IPA_UT_DBG("received packet 0x%X\n", *packet_recv);

	if (*packet_recv != *packet) {
		IPA_UT_ERR("packet content mismatch\n");
		ret = -EFAULT;
	}

	/* recycle buffer */
	*packet_recv = 0;

	/* recycle TRE */
	/* TX */
	memset((struct tx_transfer_ring_ele *)
		test_ntn_ctx->tx_transfer_ring_addr.base + test_ntn_ctx->tx_idx,
		0, sizeof(struct rx_transfer_ring_ele));

	/* RX */
	memset(rx_ele, 0, sizeof(struct rx_transfer_ring_ele));
	rx_ele->fd = 1;
	rx_ele->ld = 1;

	test_ntn_ctx->rx_idx = (test_ntn_ctx->rx_idx + 1) % NUM_RX_TR_ELE;
	test_ntn_ctx->tx_idx = (test_ntn_ctx->tx_idx + 1) % NUM_TX_TR_ELE;
	IPA_UT_DBG("now indexes are: rx %d, tx %d\n", test_ntn_ctx->rx_idx,
		test_ntn_ctx->tx_idx);

	ipa_ntn_test_advance_db(&test_ntn_ctx->tx_db_local, 1,
		sizeof(struct tx_transfer_ring_ele),
		test_ntn_ctx->tx_transfer_ring_addr.size);
	iowrite32(test_ntn_ctx->tx_db_local +
		lower_32_bits(test_ntn_ctx->tx_transfer_ring_addr.phys_base),
		tx_db);
	IPA_UT_DBG("tx_db_local advanced to 0x%X\n",
		test_ntn_ctx->tx_db_local +
		lower_32_bits(test_ntn_ctx->tx_transfer_ring_addr.phys_base));
err:
	iounmap(rx_db);
	iounmap(tx_db);
	return ret;
}

static int ipa_ntn_teardown_pipes(void)
{
	int ret = 0;

	if (ipa_eth_client_disconn_pipes(&test_ntn_ctx->client)) {
		IPA_UT_ERR("fail to teardown ntn pipes.\n");
		ret = -EFAULT;
	}

	test_ntn_ctx->rx_idx = 0;
	test_ntn_ctx->tx_idx = 0;
	test_ntn_ctx->tx_db_local = 0;
	test_ntn_ctx->rx_db_local = 0;
	return ret;
}
static int ipa_ntn_test_prepare_test(void)
{
	struct ipa_ep_cfg ep_cfg = { { 0 } };
	int offset = 0;
	int ret = 0;

	if (ipa_test_ntn_init_rings()) {
		IPA_UT_ERR("fail to fill rings.\n");
		return -EFAULT;
	}

	if (ipa_ntn_test_setup_pipes()) {
		IPA_UT_ERR("fail to setup ntn pipes.\n");
		return -EFAULT;
	}

	offset = sizeof(struct rx_transfer_ring_ele) * (NUM_RX_TR_ELE - 1);
	IPA_UT_DBG("ofset 0x%X\n", offset);

	IPA_UT_DBG("writing to RX tail ptr in 0x%X le value of 0x%X",
		(u32 *)((char *)test_ntn_ctx->bar_addr.base + RX_TAIL_PTR_OFF),
		lower_32_bits(test_ntn_ctx->rx_transfer_ring_addr.phys_base +
			offset));

	*((u32 *)((char *)test_ntn_ctx->bar_addr.base + RX_TAIL_PTR_OFF)) =
		cpu_to_le32(lower_32_bits(
			test_ntn_ctx->rx_transfer_ring_addr.phys_base +
			offset));

	/* initialize tx tail to the beginning of the ring */
	IPA_UT_DBG("writing to TX tail ptr in 0x%X le value of 0x%X",
		(u32 *)((char *)test_ntn_ctx->bar_addr.base + TX_TAIL_PTR_OFF),
		lower_32_bits(test_ntn_ctx->tx_transfer_ring_addr.phys_base));

	*((u32 *)((char *)test_ntn_ctx->bar_addr.base + TX_TAIL_PTR_OFF)) =
		cpu_to_le32(lower_32_bits(
			test_ntn_ctx->tx_transfer_ring_addr.phys_base));

	if (ipa_ntn_test_reg_intf()) {
		IPA_UT_ERR("fail to reg ntn interface.\n");
		ret = -EFAULT;
		goto teardown_pipes;
	}

	/* configure NTN RX EP in DMA mode */
	ep_cfg.mode.mode = IPA_DMA;
	ep_cfg.mode.dst = test_ntn_ctx->cons_client_type;

	ep_cfg.seq.set_dynamic = true;

	if (ipa3_cfg_ep(ipa_get_ep_mapping(test_ntn_ctx->prod_client_type),
		&ep_cfg)) {
		IPA_UT_ERR("fail to configure DMA mode.\n");
		ret = -EFAULT;
		goto unreg;
	}

	return 0;

unreg:
	if (ipa_ntn_test_unreg_intf()) {
		IPA_UT_ERR("fail to unregister interface.\n");
		ret = -EFAULT;
		goto teardown_pipes;
	}
teardown_pipes:
	if (ipa_ntn_teardown_pipes())
		ret = -EFAULT;
	return ret;
}
static int ipa_ntn_test_single_transfer(void *priv)
{
	int ret = 0;

	if(!test_ntn_ctx->ready) {
		if (test_ntn_ctx->wait_cnt) {
			IPA_UT_ERR("ipa ready timeout, don't run\n");
			return -EFAULT;
		}
		/* ready cb test hasn't ran yet and we need to wait */
		if (ipa_ntn_test_ready_cb(NULL)) {
			IPA_UT_ERR("ipa ready timeout, don't run\n");
			return -EFAULT;
		}
	}

	if (ipa_ntn_test_prepare_test()) {
		IPA_UT_ERR("failed to prepare test.\n");
		ret = -EFAULT;
		goto fail;
	}

	if (ipa_ntn_send_one_packet()) {
		IPA_UT_ERR("fail to transfer packet.\n");
		ret = -EFAULT;
		goto fail;
	}

	IPA_UT_INFO("one packet sent and received succesfully\n");

	ipa_ntn_test_print_stats();

fail:
	if (ipa_ntn_test_unreg_intf()) {
		IPA_UT_ERR("fail to unregister interface.\n");
		ret = -EFAULT;
	}

	if (ipa_ntn_teardown_pipes())
		ret = -EFAULT;
	return ret;
}

static int ipa_ntn_send_multi_packet_one_by_one(int num)
{
	int i;

	IPA_UT_INFO("about to send %d packets\n", num);
	for (i = 0; i < num; i++) {
		if (ipa_ntn_send_one_packet()) {
			IPA_UT_ERR("failed on %d packet\n", i);
			return -EFAULT;
		}
		IPA_UT_DBG("packet %d sent and recieved succesfully\n\n", i);
	}
	IPA_UT_INFO("all packets were succesfull\n\n");
	return 0;
}

static int ipa_ntn_test_multi_transfer(void *priv)
{
	int ret = 0;

	if (!test_ntn_ctx->ready) {
		if (test_ntn_ctx->wait_cnt) {
			IPA_UT_ERR("ipa ready timeout, don't run\n");
			return -EFAULT;
		}
		/* ready cb test hasn't ran yet and we need to wait */
		if (ipa_ntn_test_ready_cb(NULL)) {
			IPA_UT_ERR("ipa ready timeout, don't run\n");
			return -EFAULT;
		}
	}

	if (ipa_ntn_test_prepare_test()) {
		IPA_UT_ERR("failed to prepare test.\n");
		ret = -EFAULT;
		goto fail;
	}

	if (ipa_ntn_send_multi_packet_one_by_one(NUM_RX_BUFS/2)) {
		IPA_UT_ERR("failed to send packets.\n");
		ret = -EFAULT;
		goto fail;
	}

	IPA_UT_INFO("%d packets sent and received succesfully\n",
		NUM_RX_BUFS / 2);

	ipa_ntn_test_print_stats();

fail:
	if (ipa_ntn_test_unreg_intf()) {
		IPA_UT_ERR("fail to unregister interface.\n");
		ret = -EFAULT;
	}

	if (ipa_ntn_teardown_pipes())
		ret = -EFAULT;
	return ret;
}

static int ipa_ntn_test_multi_transfer_wrap_around(void *priv)
{
	int ret = 0;

	if (!test_ntn_ctx->ready) {
		if (test_ntn_ctx->wait_cnt) {
			IPA_UT_ERR("ipa ready timeout, don't run\n");
			return -EFAULT;
		}
		/* ready cb test hasn't ran yet and we need to wait */
		if (ipa_ntn_test_ready_cb(NULL)) {
			IPA_UT_ERR("ipa ready timeout, don't run\n");
			return -EFAULT;
		}
	}

	if (ipa_ntn_test_prepare_test()) {
		IPA_UT_ERR("failed to prepare test.\n");
		ret = -EFAULT;
		goto fail;
	}

	if (ipa_ntn_send_multi_packet_one_by_one(NUM_RX_BUFS * 2)) {
		IPA_UT_ERR("failed to send packets.\n");
		ret = -EFAULT;
		goto fail;
	}

	IPA_UT_INFO("%d packets sent and received succesfully\n",
		NUM_RX_BUFS * 2);

	ipa_ntn_test_print_stats();

fail:
	if (ipa_ntn_test_unreg_intf()) {
		IPA_UT_ERR("fail to unregister interface.\n");
		ret = -EFAULT;
	}

	if (ipa_ntn_teardown_pipes())
		ret = -EFAULT;
	return ret;
}

static int ipa_ntn_send_packet_burst(void)
{
	u32 *packet[NUM_RX_TR_ELE];
	u32 *packet_recv;

	void __iomem *rx_db;
	void __iomem *tx_db;
	struct rx_event_ring_ele *rx_event;
	u32 *tx_ring_tail;
	u32 orig_tx_tail;
	u32 *rx_ring_tail;
	u32 orig_rx_tail;
	int loop_cnt;
	u64 evt_addr;
	u64 pkt_addr[NUM_RX_TR_ELE];
	struct rx_transfer_ring_ele *rx_ele;
	struct ipa_ep_cfg_ctrl ep_cfg_ctrl = { 0 };

	int i, initial_val, ret = 0;

	rx_db = ioremap(
		test_ntn_ctx->rx_pipe_info.info.db_pa, DB_REGISTER_SIZE);
	if (!rx_db) {
		IPA_UT_ERR("ioremap failed");
		return ret;
	}

	tx_db = ioremap(
		test_ntn_ctx->tx_pipe_info.info.db_pa, DB_REGISTER_SIZE);
	if (!tx_db) {
		IPA_UT_ERR("ioremap failed");
		return ret;
	}

	/* initialize packets */
	initial_val = PACKET_CONTENT;
	for (i = 0; i < NUM_RX_TR_ELE - 1; i++, initial_val++) {
		packet[i] = (u32 *)((u8 *)test_ntn_ctx->rx_buf.base +
			(i * BUFFER_SIZE));
		pkt_addr[i] = (u64)((u8 *)test_ntn_ctx->rx_buf.phys_base +
			(i * BUFFER_SIZE));
		IPA_UT_DBG("loading packet %d with val 0x%X\n", i, initial_val);
		*(packet[i]) = initial_val;

		/* update length in TRE */
		rx_ele = (struct rx_transfer_ring_ele *)
			test_ntn_ctx->rx_transfer_ring_addr.base + i;
		rx_ele->packet_length = ETH_PACKET_SIZE;
	}

	/*
	 * set ep delay of 20ms to make sure uC is able to poll and see the
	 * ring full stats for RX
	 */
	ep_cfg_ctrl.ipa_ep_delay = true;
	ret = ipa3_cfg_ep_ctrl(
		ipa_get_ep_mapping(test_ntn_ctx->prod_client_type),
		&ep_cfg_ctrl);
	if (ret) {
		IPA_UT_ERR("couldn't set delay to ETHERNET_PROD\n");
		goto err;
	}
	IPA_UT_DBG("delay set succesfully to ETHERNET_PROD\n");

	/* point db to end of ring */
	ipa_ntn_test_advance_db(&test_ntn_ctx->rx_db_local, NUM_RX_TR_ELE - 1,
		sizeof(struct rx_transfer_ring_ele),
		test_ntn_ctx->rx_transfer_ring_addr.size);

	tx_ring_tail = (u32 *)((char *)test_ntn_ctx->bar_addr.base +
		TX_TAIL_PTR_OFF);
	orig_tx_tail = *tx_ring_tail;
	rx_ring_tail = (u32 *)((char *)test_ntn_ctx->bar_addr.base +
		RX_TAIL_PTR_OFF);
	orig_rx_tail = *rx_ring_tail;

	IPA_UT_DBG("orig tx tail 0x%X\n", orig_tx_tail);
	IPA_UT_DBG("orig rx tail 0x%X\n", orig_rx_tail);

	/* ring db and send packet */
	iowrite32(test_ntn_ctx->rx_db_local +
		lower_32_bits(test_ntn_ctx->rx_transfer_ring_addr.phys_base),
		rx_db);
	IPA_UT_DBG("rx_db_local increased to 0x%X\n",
		test_ntn_ctx->rx_db_local +
		lower_32_bits(test_ntn_ctx->rx_transfer_ring_addr.phys_base));

	IPA_UT_DBG("sleep before removing delay\n");
	msleep(20);
	ep_cfg_ctrl.ipa_ep_delay = false;
	ret = ipa3_cfg_ep_ctrl(
		ipa_get_ep_mapping(test_ntn_ctx->prod_client_type),
		&ep_cfg_ctrl);
	if (ret) {
		IPA_UT_ERR("couldn't unset delay to ETHERNET_PROD\n");
		goto err;
	}
	IPA_UT_DBG("delay unset succesfully from ETHERNET_PROD\n");

	loop_cnt = 0;
	while (((*rx_ring_tail - orig_rx_tail) < NUM_RX_TR_ELE - 1) ||
		((*tx_ring_tail - orig_tx_tail) < NUM_TX_TR_ELE - 1)) {
		loop_cnt++;

		if (loop_cnt == 1000) {
			IPA_UT_ERR("transfer timeout!\n");
			IPA_UT_ERR("orig_tx_tail: %X tx_ring_db: %X\n",
				orig_tx_tail, *tx_ring_tail);
			IPA_UT_ERR("orig_rx_tail: %X rx_ring_db: %X\n",
				orig_rx_tail, *rx_ring_tail);
			IPA_UT_ERR("rx db local: 0x%X\n",
				test_ntn_ctx->rx_db_local +
				lower_32_bits(
				test_ntn_ctx->rx_transfer_ring_addr.phys_base));
			BUG();
			ret = -EFAULT;
			goto err;
		}
		usleep_range(1000, 1001);
	}

	IPA_UT_DBG("loop_cnt %d\n", loop_cnt);
	IPA_UT_DBG("rx ring tail 0x%X\n", *rx_ring_tail);
	IPA_UT_DBG("tx ring tail 0x%X\n", *tx_ring_tail);

	for (i = 0; i < NUM_RX_TR_ELE - 1; i++, initial_val++) {
		/* verify RX event */
		rx_ele = (struct rx_transfer_ring_ele *)
			test_ntn_ctx->rx_transfer_ring_addr.base + i;
		rx_event = (struct rx_event_ring_ele *)rx_ele;

		IPA_UT_DBG("%d: address written by GSI is 0x[%X][%X]\n",
			i, rx_event->buff_addr2, rx_event->buff_addr1);
		IPA_UT_DBG("own bit is now %u", rx_event->own);

		if (!rx_event->own) {
			IPA_UT_ERR("own bit not modified by gsi - failed\n");
			ret = -EFAULT;
		}

		evt_addr = ((u64)rx_event->buff_addr2 << 32) |
			(u64)(rx_event->buff_addr1);
		IPA_UT_DBG(
			"RX: addr from event 0x%llx, address from buff %llx\n",
			evt_addr, pkt_addr[i]);
		if (evt_addr != pkt_addr[i]) {
			IPA_UT_ERR("addresses are different - fail\n");
			ret = -EFAULT;
		}

		/* read received packet */
		packet_recv = (u32 *)((u8 *)test_ntn_ctx->tx_buf.base +
			(i * BUFFER_SIZE));
		IPA_UT_DBG("received packet 0x%X\n", *packet_recv);

		if (*packet_recv != *(packet[i])) {
			IPA_UT_ERR("packet content mismatch 0x%X != 0x%X\n",
				*packet_recv, *(packet[i]));
			ret = -EFAULT;
		}
		IPA_UT_DBG("packet %d content match!\n", i);

		/* recycle buffer */
		*packet_recv = 0;

		/* recycle TRE */
		/* TX */
		memset((struct tx_transfer_ring_ele *)
			test_ntn_ctx->tx_transfer_ring_addr.base + i,
			0, sizeof(struct rx_transfer_ring_ele));

		/* RX */
		memset(rx_ele, 0, sizeof(struct rx_transfer_ring_ele));
		rx_ele->fd = 1;
		rx_ele->ld = 1;
	}

	ipa_ntn_test_advance_db(&test_ntn_ctx->tx_db_local, NUM_TX_TR_ELE - 1,
		sizeof(struct tx_transfer_ring_ele),
		test_ntn_ctx->tx_transfer_ring_addr.size);
	IPA_UT_DBG("advance tx_db_local to 0x%X\n",
		test_ntn_ctx->tx_db_local +
		lower_32_bits(test_ntn_ctx->tx_transfer_ring_addr.phys_base));
	iowrite32(test_ntn_ctx->tx_db_local +
		lower_32_bits(test_ntn_ctx->tx_transfer_ring_addr.phys_base),
		tx_db);

	test_ntn_ctx->rx_idx = NUM_RX_TR_ELE - 1;
	test_ntn_ctx->tx_idx = NUM_TX_TR_ELE - 1;
err:
	iounmap(rx_db);
	iounmap(tx_db);
	return ret;
}

static int ipa_ntn_test_multi_transfer_burst(void *priv)
{
	int ret = 0;

	if (!test_ntn_ctx->ready) {
		if (test_ntn_ctx->wait_cnt) {
			IPA_UT_ERR("ipa ready timeout, don't run\n");
			return -EFAULT;
		}
		/* ready cb test hasn't ran yet and we need to wait */
		if (ipa_ntn_test_ready_cb(NULL)) {
			IPA_UT_ERR("ipa ready timeout, don't run\n");
			return -EFAULT;
		}
	}

	if (ipa_ntn_test_prepare_test()) {
		IPA_UT_ERR("failed to prepare test.\n");
		ret = -EFAULT;
		goto fail;
	}

	if (ipa_ntn_send_packet_burst()) {
		IPA_UT_ERR("failed to send packets.\n");
		ret = -EFAULT;
		goto fail;
	}

	IPA_UT_INFO("sent %d packets in a burst succesfully!\n",
		NUM_TX_TR_ELE - 1);

	if (ipa_ntn_send_one_packet()) {
		IPA_UT_ERR("failed to send last packet.\n");
		ret = -EFAULT;
		goto fail;
	}
	IPA_UT_INFO("sent the last packet succesfully!\n");

	ipa_ntn_test_print_stats();

fail:
	if (ipa_ntn_test_unreg_intf()) {
		IPA_UT_ERR("fail to unregister interface.\n");
		ret = -EFAULT;
	}

	if (ipa_ntn_teardown_pipes())
		ret = -EFAULT;
	return ret;
}

static int ipa_ntn_test_clients2_multi_transfer_burst(void *priv)
{
	int ret;

	ipa_test_ntn_set_client_params(IPA_CLIENT_ETHERNET2_CONS, IPA_CLIENT_ETHERNET2_PROD, 1);
	ret = ipa_ntn_test_multi_transfer_burst(priv);
	ipa_test_ntn_set_client_params(IPA_CLIENT_ETHERNET_CONS, IPA_CLIENT_ETHERNET_PROD, 0);

	return ret;
}

/* Suite definition block */
IPA_UT_DEFINE_SUITE_START(ntn, "NTN3 tests",
	ipa_test_ntn_suite_setup, ipa_test_ntn_suite_teardown)
{
	IPA_UT_ADD_TEST(ready_cb,
		"ready callback test",
		ipa_ntn_test_ready_cb,
		true, IPA_HW_v5_0, IPA_HW_MAX),

	IPA_UT_ADD_TEST(single_transfer,
		"single data transfer",
		ipa_ntn_test_single_transfer,
		true, IPA_HW_v5_0, IPA_HW_MAX),

	IPA_UT_ADD_TEST(multi_transfer,
		"multi data transfer without wrap around",
		ipa_ntn_test_multi_transfer,
		true, IPA_HW_v5_0, IPA_HW_MAX),

	IPA_UT_ADD_TEST(multi_transfer_w_wrap,
		"multi data transfer with wrap around",
		ipa_ntn_test_multi_transfer_wrap_around,
		true, IPA_HW_v5_0, IPA_HW_MAX),

	IPA_UT_ADD_TEST(multi_transfer_burst,
			"send entire ring in one shot",
			ipa_ntn_test_multi_transfer_burst,
			true, IPA_HW_v5_0, IPA_HW_MAX),

	IPA_UT_ADD_TEST(clients2_multi_transfer_burst,
			"Clients pair 2 send entire ring in one shot",
			ipa_ntn_test_clients2_multi_transfer_burst,
			true, IPA_HW_v5_0, IPA_HW_MAX),
} IPA_UT_DEFINE_SUITE_END(ntn);


