/*
 * Copyright (c) 2015-2017 The Linux Foundation. All rights reserved.
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

#include <wlan_hdd_includes.h>
#include <qdf_types.h>
#include <qdf_lro.h>
#include <wlan_hdd_lro.h>
#include <wlan_hdd_napi.h>
#include <wma_api.h>

#include <linux/inet_lro.h>
#include <linux/list.h>
#include <linux/random.h>
#include <net/tcp.h>

#define LRO_VALID_FIELDS \
	(LRO_DESC | LRO_ELIGIBILITY_CHECKED | LRO_TCP_ACK_NUM | \
	 LRO_TCP_DATA_CSUM | LRO_TCP_SEQ_NUM | LRO_TCP_WIN)

#if defined(QCA_WIFI_QCA6290)
/**
 * hdd_lro_init() - initialization for LRO
 * @hdd_ctx: HDD context
 *
 * This function sends the LRO configuration to the firmware
 * via WMA
 * Make sure that this function gets called after NAPI
 * instances have been created.
 *
 * Return: 0 - success, < 0 - failure
 */
int hdd_lro_init(hdd_context_t *hdd_ctx)
{
	return 0;
}

static qdf_lro_ctx_t wlan_hdd_get_lro_ctx(struct sk_buff *skb)
{
	return (qdf_lro_ctx_t)QDF_NBUF_CB_RX_LRO_CTX(skb);
}
#else
/**
 * hdd_lro_init() - initialization for LRO
 * @hdd_ctx: HDD context
 *
 * This function sends the LRO configuration to the firmware
 * via WMA
 * Make sure that this function gets called after NAPI
 * instances have been created.
 *
 * Return: 0 - success, < 0 - failure
 */
int hdd_lro_init(hdd_context_t *hdd_ctx)
{
	struct cdp_lro_hash_config lro_config;

	if ((!hdd_ctx->config->lro_enable) &&
	    (hdd_napi_enabled(HDD_NAPI_ANY) == 0)) {
		hdd_warn("LRO and NAPI are both disabled");
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

static qdf_lro_ctx_t wlan_hdd_get_lro_ctx(struct sk_buff *skb)
{
	struct hif_opaque_softc *hif_hdl =
		(struct hif_opaque_softc *)cds_get_context(QDF_MODULE_ID_HIF);
	if (hif_hdl == NULL) {
		hdd_err("hif_hdl is NULL");
	}

	return hif_get_lro_info(QDF_NBUF_CB_RX_CTX_ID(skb), hif_hdl);
}
#endif

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
	qdf_lro_ctx_t ctx;
	enum hdd_lro_rx_status status = HDD_LRO_NO_RX;

	if ((adapter->dev->features & NETIF_F_LRO) &&
		 QDF_NBUF_CB_RX_TCP_PROTO(skb) &&
		 !QDF_NBUF_CB_RX_PEER_CACHED_FRM(skb)) {
		struct qdf_lro_info info;
		struct net_lro_desc *lro_desc = NULL;
		struct hif_opaque_softc *hif_hdl =
			(struct hif_opaque_softc *)cds_get_context(
							QDF_MODULE_ID_HIF);
		if (hif_hdl == NULL) {
			hdd_err("hif_hdl is NULL");
			return status;
		}

		ctx = wlan_hdd_get_lro_ctx(skb);
		if (ctx == NULL) {
			hdd_err("LRO mgr is NULL, vdev could be going down");
			return status;
		}

		info.iph = skb->data;
		info.tcph = skb->data + QDF_NBUF_CB_RX_TCP_OFFSET(skb);
		ctx->lro_mgr->dev = adapter->dev;
		if (qdf_lro_get_info(ctx, skb, &info, (void **)&lro_desc)) {
			struct net_lro_info hdd_lro_info;

			hdd_lro_info.valid_fields = LRO_VALID_FIELDS;

			hdd_lro_info.lro_desc = lro_desc;
			hdd_lro_info.lro_eligible = 1;
			hdd_lro_info.tcp_ack_num = QDF_NBUF_CB_RX_TCP_ACK_NUM(skb);
			hdd_lro_info.tcp_data_csum =
				 csum_unfold(htons(QDF_NBUF_CB_RX_TCP_CHKSUM(skb)));
			hdd_lro_info.tcp_seq_num = QDF_NBUF_CB_RX_TCP_SEQ_NUM(skb);
			hdd_lro_info.tcp_win = QDF_NBUF_CB_RX_TCP_WIN(skb);

			lro_receive_skb_ext(ctx->lro_mgr, skb,
				 (void *)adapter, &hdd_lro_info);

			if (!hdd_lro_info.lro_desc->active)
				qdf_lro_desc_free(ctx, lro_desc);

			status = HDD_LRO_RX;
		} else {
			qdf_lro_flush_pkt(ctx, &info);
		}
	}
	return status;
}

/**
 * hdd_lro_display_stats() - display LRO statistics
 * @hdd_ctx: hdd context
 *
 * Return: none
 */
void hdd_lro_display_stats(hdd_context_t *hdd_ctx)
{
	hdd_debug("LRO stats is broken, will fix it");
}
