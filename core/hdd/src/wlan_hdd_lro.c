/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

/**
 * DOC: wlan_hdd_lro.c
 *
 * WLAN HDD LRO interface implementation
 */

/* denote that this file does not allow legacy hddLog */
#define HDD_DISALLOW_LEGACY_HDDLOG 1

#include <wlan_hdd_includes.h>
#include <qdf_types.h>
#include <wlan_hdd_lro.h>
#include <wlan_hdd_napi.h>
#include <wma_api.h>
#include <ol_txrx_types.h>
#include <ol_cfg.h>
#include <cdp_txrx_lro.h>

#include <linux/inet_lro.h>
#include <linux/list.h>
#include <linux/random.h>
#include <net/tcp.h>

#define LRO_MAX_AGGR_SIZE 100

#define LRO_VALID_FIELDS \
	(LRO_DESC | LRO_ELIGIBILITY_CHECKED | LRO_TCP_ACK_NUM | \
	 LRO_TCP_DATA_CSUM | LRO_TCP_SEQ_NUM | LRO_TCP_WIN)

/**
 * hdd_lro_get_skb_header() - LRO callback function
 * @skb: network buffer
 * @ip_hdr: contains a pointer to the IP header
 * @tcpudp_hdr: contains a pointer to the TCP header
 * @hdr_flags: indicates if this is a TCP, IPV4 frame
 * @priv: private driver specific opaque pointer
 *
 * Get the IP and TCP headers from the skb
 *
 * Return: 0 - success, < 0 - failure
 */
static int hdd_lro_get_skb_header(struct sk_buff *skb, void **ip_hdr,
	void **tcpudp_hdr, u64 *hdr_flags, void *priv)
{
	if (QDF_NBUF_CB_RX_IPV6_PROTO(skb)) {
		hdr_flags = 0;
		return -EINVAL;
	}

	*hdr_flags |= (LRO_IPV4 | LRO_TCP);
	(*ip_hdr) = skb->data;
	(*tcpudp_hdr) = skb->data + QDF_NBUF_CB_RX_TCP_OFFSET(skb);
	return 0;
}

/**
 * hdd_lro_desc_pool_init() - Initialize the free pool of LRO
 * descriptors
 * @lro_desc_pool: free pool of the LRO descriptors
 * @lro_mgr: LRO manager
 *
 * Initialize a list that holds the free LRO descriptors
 *
 * Return: none
 */
static void hdd_lro_desc_pool_init(struct hdd_lro_desc_pool *lro_desc_pool,
	 struct net_lro_mgr *lro_mgr)
{
	int i;

	INIT_LIST_HEAD(&lro_desc_pool->lro_free_list_head);

	for (i = 0; i < LRO_DESC_POOL_SZ; i++) {
		lro_desc_pool->lro_desc_array[i].lro_desc =
			 &lro_mgr->lro_arr[i];
		list_add_tail(&lro_desc_pool->lro_desc_array[i].lro_node,
			 &lro_desc_pool->lro_free_list_head);
	}
	qdf_spinlock_create(&lro_desc_pool->lro_pool_lock);
}

/**
 * hdd_lro_desc_info_init() - Initialize the LRO descriptors
 * @hdd_info: HDD LRO data structure
 *
 * Initialize the free pool of LRO descriptors and the entries
 * of the hash table
 *
 * Return: none
 */
static void hdd_lro_desc_info_init(struct hdd_lro_s *hdd_info)
{
	int i;

	/* Initialize pool of free LRO desc.*/
	hdd_lro_desc_pool_init(&hdd_info->lro_desc_info.lro_desc_pool,
		 hdd_info->lro_mgr);

	/* Initialize the hash table of LRO desc.*/
	for (i = 0; i < LRO_DESC_TABLE_SZ; i++) {
		/* initialize the flows in the hash table */
		INIT_LIST_HEAD(&hdd_info->lro_desc_info.
			 lro_hash_table[i].lro_desc_list);
	}

	qdf_spinlock_create(&hdd_info->lro_desc_info.lro_hash_lock);
	qdf_spinlock_create(&hdd_info->lro_mgr_arr_access_lock);
}

/**
 * hdd_lro_desc_pool_deinit() - Free the LRO descriptor list
 * @hdd_info: HDD LRO data structure
 *
 * Free the pool of LRO descriptors
 *
 * Return: none
 */
static void hdd_lro_desc_pool_deinit(struct hdd_lro_desc_pool *lro_desc_pool)
{
	INIT_LIST_HEAD(&lro_desc_pool->lro_free_list_head);
	qdf_spinlock_destroy(&lro_desc_pool->lro_pool_lock);
}

/**
 * hdd_lro_desc_info_deinit() - Deinitialize the LRO descriptors
 *
 * @hdd_info: HDD LRO data structure
 *
 * Deinitialize the free pool of LRO descriptors and the entries
 * of the hash table
 *
 * Return: none
 */
static void hdd_lro_desc_info_deinit(struct hdd_lro_s *hdd_info)
{
	struct hdd_lro_desc_info *desc_info = &hdd_info->lro_desc_info;

	hdd_lro_desc_pool_deinit(&desc_info->lro_desc_pool);
	qdf_spinlock_destroy(&desc_info->lro_hash_lock);
	qdf_spinlock_destroy(&hdd_info->lro_mgr_arr_access_lock);
}

/**
 * hdd_lro_tcp_flow_match() - function to check for a flow match
 * @iph: IP header
 * @tcph: TCP header
 * @lro_desc: LRO decriptor
 *
 * Checks if the descriptor belongs to the same flow as the one
 * indicated by the TCP and IP header.
 *
 * Return: true - flow match, false - flow does not match
 */
static inline bool hdd_lro_tcp_flow_match(struct net_lro_desc *lro_desc,
	 struct iphdr *iph,
	 struct tcphdr *tcph)
{
	if ((lro_desc->tcph->source != tcph->source) ||
		 (lro_desc->tcph->dest != tcph->dest) ||
		 (lro_desc->iph->saddr != iph->saddr) ||
		 (lro_desc->iph->daddr != iph->daddr))
		return false;

	return true;

}

/**
 * hdd_lro_desc_find() - LRO descriptor look-up function
 *
 * @adapter: HDD adaptor
 * @skb: network buffer
 * @iph: IP header
 * @tcph: TCP header
 * @lro_desc: contains a pointer to the LRO decriptor
 *
 * Look-up the LRO descriptor in the hash table based on the
 * flow ID toeplitz. If the flow is not found, allocates a new
 * LRO descriptor and places it in the hash table
 *
 * Return: 0 - success, < 0 - failure
 */
static int hdd_lro_desc_find(hdd_adapter_t *adapter,
	 struct sk_buff *skb, struct iphdr *iph, struct tcphdr *tcph,
	 struct net_lro_desc **lro_desc)
{
	uint32_t i;
	struct hdd_lro_desc_table *lro_hash_table;
	struct list_head *ptr;
	struct hdd_lro_desc_entry *entry;
	struct hdd_lro_desc_pool free_pool;
	struct hdd_lro_desc_info *desc_info = &adapter->lro_info.lro_desc_info;

	*lro_desc = NULL;
	i = QDF_NBUF_CB_RX_FLOW_ID_TOEPLITZ(skb) & LRO_DESC_TABLE_SZ_MASK;

	lro_hash_table = &desc_info->lro_hash_table[i];

	if (!lro_hash_table) {
		hdd_err("Invalid hash entry");
		QDF_ASSERT(0);
		return -EINVAL;
	}

	qdf_spin_lock_bh(&desc_info->lro_hash_lock);
	/* Check if this flow exists in the descriptor list */
	list_for_each(ptr, &lro_hash_table->lro_desc_list) {
		struct net_lro_desc *tmp_lro_desc = NULL;
		entry = list_entry(ptr, struct hdd_lro_desc_entry, lro_node);
		tmp_lro_desc = entry->lro_desc;
		if (tmp_lro_desc->active) {
			if (hdd_lro_tcp_flow_match(tmp_lro_desc, iph, tcph)) {
				*lro_desc = entry->lro_desc;
				qdf_spin_unlock_bh(&desc_info->lro_hash_lock);
				return 0;
			}
		}
	}
	qdf_spin_unlock_bh(&desc_info->lro_hash_lock);

	/* no existing flow found, a new LRO desc needs to be allocated */
	free_pool = adapter->lro_info.lro_desc_info.lro_desc_pool;
	qdf_spin_lock_bh(&free_pool.lro_pool_lock);
	entry = list_first_entry_or_null(
		 &free_pool.lro_free_list_head,
		 struct hdd_lro_desc_entry, lro_node);
	if (NULL == entry) {
		hdd_err("Could not allocate LRO desc!");
		qdf_spin_unlock_bh(&free_pool.lro_pool_lock);
		return -ENOMEM;
	}

	list_del_init(&entry->lro_node);
	qdf_spin_unlock_bh(&free_pool.lro_pool_lock);

	if (NULL == entry->lro_desc) {
		hdd_err("entry->lro_desc is NULL!\n");
		return -EINVAL;
	}

	qdf_mem_zero((void *)entry->lro_desc, sizeof(struct net_lro_desc));

	/*
	 * lro_desc->active should be 0 and lro_desc->tcp_rcv_tsval
	 * should be 0 for newly allocated lro descriptors
	 */
	qdf_spin_lock_bh(&desc_info->lro_hash_lock);
	list_add_tail(&entry->lro_node,
		 &lro_hash_table->lro_desc_list);
	qdf_spin_unlock_bh(&desc_info->lro_hash_lock);
	*lro_desc = entry->lro_desc;

	return 0;
}

/**
 * hdd_lro_get_desc() - LRO descriptor look-up function
 * @iph: IP header
 * @tcph: TCP header
 * @lro_arr: Array of LRO decriptors
 * @lro_mgr: LRO manager
 *
 * Looks-up the LRO descriptor for a given flow
 *
 * Return: LRO descriptor
 */
static struct net_lro_desc *hdd_lro_get_desc(struct net_lro_mgr *lro_mgr,
	 struct net_lro_desc *lro_arr,
	 struct iphdr *iph,
	 struct tcphdr *tcph)
{
	int i;

	for (i = 0; i < lro_mgr->max_desc; i++) {
		if (lro_arr[i].active)
			if (hdd_lro_tcp_flow_match(&lro_arr[i], iph, tcph))
				return &lro_arr[i];
	}

	return NULL;
}

/**
 * hdd_lro_eligible() - LRO eligibilty check
 * @iph: IP header
 * @tcph: TCP header
 * @adapter: HDD adaptor
 * @desc: LRO descriptor
 * @skb: network buffer
 *
 * Determines if the frame is LRO eligible
 *
 * Return: true - LRO eligible frame, false - frame is not LRO
 * eligible
 */
static bool hdd_lro_eligible(hdd_adapter_t *adapter, struct sk_buff *skb,
	 struct iphdr *iph, struct tcphdr *tcph, struct net_lro_desc **desc)
{
	struct net_lro_desc *lro_desc = NULL;
	int hw_lro_eligible =
		 QDF_NBUF_CB_RX_LRO_ELIGIBLE(skb) &&
		 (!QDF_NBUF_CB_RX_TCP_PURE_ACK(skb));

	if (!hw_lro_eligible)
		return false;

	if (0 != hdd_lro_desc_find(adapter, skb, iph, tcph, desc)) {
		hdd_err("finding the LRO desc failed");
		return false;
	}

	lro_desc = *desc;
	if (!lro_desc)
		return false;

	/* if this is not the first skb, check the timestamp option */
	if (lro_desc->tcp_rcv_tsval) {
		if (tcph->doff == 8) {
			__be32 *topt = (__be32 *)(tcph + 1);

			if (*topt != htonl((TCPOPT_NOP << 24)
				 |(TCPOPT_NOP << 16)
				 | (TCPOPT_TIMESTAMP << 8)
				 | TCPOLEN_TIMESTAMP))
				return true;

			/* timestamp should be in right order */
			topt++;
			if (after(ntohl(lro_desc->tcp_rcv_tsval),
					 ntohl(*topt)))
				return false;

			/* timestamp reply should not be zero */
			topt++;
			if (*topt == 0)
				return false;
		}
	}

	return true;
}

/**
 * hdd_lro_desc_free() - Free the LRO descriptor
 * @adapter: HDD adaptor
 * @desc: LRO descriptor
 *
 * Return the LRO descriptor to the free pool
 *
 * Return: none
 */
static void hdd_lro_desc_free(struct net_lro_desc *desc,
	 hdd_adapter_t *adapter)
{
	struct hdd_lro_desc_entry *entry;
	struct net_lro_desc *arr_base = adapter->lro_info.lro_mgr->lro_arr;
	struct hdd_lro_desc_info *desc_info = &adapter->lro_info.lro_desc_info;
	int i = desc - arr_base;

	if (i >= LRO_DESC_POOL_SZ) {
		hdd_err("invalid index %d", i);
		return;
	}

	entry = &desc_info->lro_desc_pool.lro_desc_array[i];

	qdf_spin_lock_bh(&desc_info->lro_hash_lock);
	list_del_init(&entry->lro_node);
	qdf_spin_unlock_bh(&desc_info->lro_hash_lock);

	qdf_spin_lock_bh(&desc_info->lro_desc_pool.lro_pool_lock);
	list_add_tail(&entry->lro_node, &desc_info->
		 lro_desc_pool.lro_free_list_head);
	qdf_spin_unlock_bh(&desc_info->lro_desc_pool.lro_pool_lock);
}

/**
 * hdd_lro_flush_pkt() - function to flush the LRO flow
 * @iph: IP header
 * @tcph: TCP header
 * @adapter: HDD adaptor
 * @lro_mgr: LRO manager
 *
 * Flush all the packets aggregated in the LRO manager for the
 * flow indicated by the TCP and IP header
 *
 * Return: none
 */
void hdd_lro_flush_pkt(struct net_lro_mgr *lro_mgr,
	 struct iphdr *iph, struct tcphdr *tcph, hdd_adapter_t *adapter)
{
	struct net_lro_desc *lro_desc;

	lro_desc = hdd_lro_get_desc(lro_mgr, lro_mgr->lro_arr, iph, tcph);

	if (!lro_desc)
		return;

	hdd_lro_desc_free(lro_desc, adapter);
	lro_flush_desc(lro_mgr, lro_desc);
}

/**
 * hdd_lro_flush() - LRO flush callback
 * @data: opaque pointer containing HDD specific information
 *
 * Callback registered to flush all the packets aggregated in
 * the LRO manager for all the flows
 *
 * Return: none
 */
void hdd_lro_flush(void *data)
{
	hdd_adapter_t *adapter = (hdd_adapter_t *)data;
	struct hdd_lro_s *hdd_info = &adapter->lro_info;
	int i;

	qdf_spin_lock_bh(&hdd_info->lro_mgr_arr_access_lock);
	for (i = 0; i < adapter->lro_info.lro_mgr->max_desc; i++) {
		if (adapter->lro_info.lro_mgr->lro_arr[i].active) {
			hdd_lro_desc_free(
				 &adapter->lro_info.lro_mgr->lro_arr[i],
				 (void *)adapter);
			lro_flush_desc(adapter->lro_info.lro_mgr,
				 &adapter->lro_info.lro_mgr->lro_arr[i]);
		}
	}
	qdf_spin_unlock_bh(&hdd_info->lro_mgr_arr_access_lock);
}

/**
 * hdd_lro_init() - initialization for LRO
 * @hdd_ctx: HDD context
 *
 * This function sends the LRO configuration to the firmware
 * via WMA
 *
 * Return: 0 - success, < 0 - failure
 */
int hdd_lro_init(hdd_context_t *hdd_ctx)
{
	struct wma_lro_config_cmd_t lro_config;

	if (!hdd_ctx->config->lro_enable) {
		hdd_err("LRO Disabled");
		return 0;
	}

	lro_config.lro_enable = 1;
	lro_config.tcp_flag = TCPHDR_ACK;
	lro_config.tcp_flag_mask = TCPHDR_FIN | TCPHDR_SYN | TCPHDR_RST |
		TCPHDR_ACK | TCPHDR_URG | TCPHDR_ECE | TCPHDR_CWR;

	get_random_bytes(lro_config.toeplitz_hash_ipv4,
		 (sizeof(lro_config.toeplitz_hash_ipv4[0]) *
		 LRO_IPV4_SEED_ARR_SZ));

	get_random_bytes(lro_config.toeplitz_hash_ipv6,
		 (sizeof(lro_config.toeplitz_hash_ipv6[0]) *
		 LRO_IPV6_SEED_ARR_SZ));

	hdd_debug("sending the LRO configuration to the fw");
	if (0 != wma_lro_init(&lro_config)) {
		hdd_err("Failed to send LRO configuration!");
		hdd_ctx->config->lro_enable = 0;
		return -EAGAIN;
	}

	return 0;
}

/**
 * hdd_lro_enable() - enable LRO
 * @hdd_ctx: HDD context
 * @adapter: HDD adapter
 *
 * This function enables LRO in the network device attached to
 * the HDD adapter. It also allocates the HDD LRO instance for
 * that network device
 *
 * Return: 0 - success, < 0 - failure
 */
int hdd_lro_enable(hdd_context_t *hdd_ctx,
	 hdd_adapter_t *adapter)
{
	struct hdd_lro_s *hdd_lro;
	size_t lro_mgr_sz, desc_arr_sz, desc_pool_sz, hash_table_sz;
	uint8_t *lro_mem_ptr;

	if (!hdd_ctx->config->lro_enable ||
		 NL80211_IFTYPE_STATION != adapter->wdev.iftype) {
		hdd_info("LRO Disabled");
		return 0;
	}

	hdd_info("LRO Enabled");

	hdd_lro = &adapter->lro_info;
	qdf_mem_zero((void *)hdd_lro, sizeof(struct hdd_lro_s));
	/*
	* Allocate all the LRO data structures at once and then carve
	* them up as needed
	*/
	lro_mgr_sz = sizeof(struct net_lro_mgr);
	desc_arr_sz = (LRO_DESC_POOL_SZ * sizeof(struct net_lro_desc));
	desc_pool_sz = (LRO_DESC_POOL_SZ * sizeof(struct hdd_lro_desc_entry));
	hash_table_sz = (sizeof(struct hdd_lro_desc_table) * LRO_DESC_TABLE_SZ);

	lro_mem_ptr = qdf_mem_malloc(lro_mgr_sz + desc_arr_sz + desc_pool_sz +
		 hash_table_sz);

	if (NULL == lro_mem_ptr) {
		hdd_err("Unable to allocate memory for LRO");
		hdd_ctx->config->lro_enable = 0;
		return -ENOMEM;
	}

	/* LRO manager */
	hdd_lro->lro_mgr = (struct net_lro_mgr *)lro_mem_ptr;
	lro_mem_ptr += lro_mgr_sz;

	/* LRO decriptor array */
	hdd_lro->lro_mgr->lro_arr = (struct net_lro_desc *)lro_mem_ptr;
	lro_mem_ptr += desc_arr_sz;

	/* LRO descriptor pool */
	hdd_lro->lro_desc_info.lro_desc_pool.lro_desc_array =
		 (struct hdd_lro_desc_entry *)lro_mem_ptr;
	lro_mem_ptr += desc_pool_sz;

	/* hash table to store the LRO descriptors */
	hdd_lro->lro_desc_info.lro_hash_table =
		 (struct hdd_lro_desc_table *)lro_mem_ptr;

	/* Initialize the LRO descriptors */
	 hdd_lro_desc_info_init(hdd_lro);

	hdd_lro->lro_mgr->dev = adapter->dev;
	if (hdd_ctx->enableRxThread)
		hdd_lro->lro_mgr->features = LRO_F_NI;

	if (hdd_napi_enabled(HDD_NAPI_ANY))
		hdd_lro->lro_mgr->features |= LRO_F_NAPI;

	hdd_lro->lro_mgr->ip_summed_aggr = CHECKSUM_UNNECESSARY;
	hdd_lro->lro_mgr->max_aggr = LRO_MAX_AGGR_SIZE;
	hdd_lro->lro_mgr->get_skb_header = hdd_lro_get_skb_header;
	hdd_lro->lro_mgr->ip_summed = CHECKSUM_UNNECESSARY;
	hdd_lro->lro_mgr->max_desc = LRO_DESC_POOL_SZ;

	adapter->dev->features |= NETIF_F_LRO;

	/* Register the flush callback */
	ol_register_lro_flush_cb(hdd_lro_flush, adapter);

	return 0;
}

/**
 * hdd_lro_disable() - disable LRO
 * @hdd_ctx: HDD context
 * @adapter: HDD adapter
 *
 * This function frees the HDD LRO instance for the network
 * device attached to the HDD adapter
 *
 * Return: none
 */
void hdd_lro_disable(hdd_context_t *hdd_ctx, hdd_adapter_t *adapter)
{
	if (!hdd_ctx->config->lro_enable ||
		 NL80211_IFTYPE_STATION != adapter->wdev.iftype)
		return;

	/* Deregister the flush callback */
	ol_deregister_lro_flush_cb();

	if (adapter->lro_info.lro_mgr) {
		hdd_lro_desc_info_deinit(&adapter->lro_info);
		qdf_mem_free(adapter->lro_info.lro_mgr);
		adapter->lro_info.lro_mgr = NULL;
		adapter->lro_info.lro_desc_info.
			lro_desc_pool.lro_desc_array = NULL;
		adapter->lro_info.lro_desc_info.
			lro_hash_table = NULL;
	}
	return;
}

/**
 * hdd_lro_rx() - LRO receive function
 * @hdd_ctx: HDD context
 * @adapter: HDD adapter
 * @skb: network buffer
 *
 * Delivers LRO eligible frames to the LRO manager
 *
 * Return: HDD_LRO_RX - frame delivered to LRO manager
 * HDD_LRO_NO_RX - frame not delivered
 */
enum hdd_lro_rx_status hdd_lro_rx(hdd_context_t *hdd_ctx,
	 hdd_adapter_t *adapter, struct sk_buff *skb)
{
	enum hdd_lro_rx_status status = HDD_LRO_NO_RX;

	if ((adapter->dev->features & NETIF_F_LRO) &&
		 QDF_NBUF_CB_RX_TCP_PROTO(skb)) {
		struct iphdr *iph;
		struct tcphdr *tcph;
		struct net_lro_desc *lro_desc = NULL;
		iph = (struct iphdr *)skb->data;
		tcph = (struct tcphdr *)(skb->data + QDF_NBUF_CB_RX_TCP_OFFSET(skb));
		if (hdd_lro_eligible(adapter, skb, iph, tcph, &lro_desc)) {
			struct net_lro_info hdd_lro_info;

			hdd_lro_info.valid_fields = LRO_VALID_FIELDS;

			hdd_lro_info.lro_desc = lro_desc;
			hdd_lro_info.lro_eligible = 1;
			hdd_lro_info.tcp_ack_num = QDF_NBUF_CB_RX_TCP_ACK_NUM(skb);
			hdd_lro_info.tcp_data_csum =
				 csum_unfold(htons(QDF_NBUF_CB_RX_TCP_CHKSUM(skb)));
			hdd_lro_info.tcp_seq_num = QDF_NBUF_CB_RX_TCP_SEQ_NUM(skb);
			hdd_lro_info.tcp_win = QDF_NBUF_CB_RX_TCP_WIN(skb);

			lro_receive_skb_ext(adapter->lro_info.lro_mgr, skb,
				 (void *)adapter, &hdd_lro_info);

			if (!hdd_lro_info.lro_desc->active)
				hdd_lro_desc_free(lro_desc, adapter);

			status = HDD_LRO_RX;
		} else {
			hdd_lro_flush_pkt(adapter->lro_info.lro_mgr,
				 iph, tcph, adapter);
		}
	}
	return status;
}
