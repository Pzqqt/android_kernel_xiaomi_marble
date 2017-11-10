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
int hdd_lro_init(struct hdd_context *hdd_ctx)
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
int hdd_lro_init(struct hdd_context *hdd_ctx)
{
	struct cdp_lro_hash_config lro_config;

	if ((!hdd_ctx->config->lro_enable) &&
	    (hdd_napi_enabled(HDD_NAPI_ANY) == 0)) {
		hdd_warn("LRO and NAPI are both disabled");
		return QDF_STATUS_E_FAILURE;
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

	if (hdd_ctx->config->enable_tcp_delack) {
		hdd_ctx->en_tcp_delack_no_lro = 0;
		hdd_reset_tcp_delack(hdd_ctx);
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
enum hdd_lro_rx_status hdd_lro_rx(struct hdd_context *hdd_ctx,
	 struct hdd_adapter *adapter, struct sk_buff *skb)
{
	qdf_lro_ctx_t ctx;
	enum hdd_lro_rx_status status = HDD_LRO_NO_RX;

	if (((adapter->dev->features & NETIF_F_LRO) != NETIF_F_LRO) ||
		!QDF_NBUF_CB_RX_TCP_PROTO(skb) ||
		QDF_NBUF_CB_RX_PEER_CACHED_FRM(skb) ||
		qdf_atomic_read(&hdd_ctx->disable_lro_in_concurrency) ||
		qdf_atomic_read(&hdd_ctx->disable_lro_in_low_tput))
		return HDD_LRO_NO_RX;

	{
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
void hdd_lro_display_stats(struct hdd_context *hdd_ctx)
{
	hdd_debug("LRO stats is broken, will fix it");
}

QDF_STATUS
hdd_lro_set_reset(struct hdd_context *hdd_ctx, struct hdd_adapter *adapter,
			       uint8_t enable_flag)
{
	if (!hdd_ctx->config->lro_enable ||
		 QDF_STA_MODE != adapter->device_mode) {
		hdd_debug("LRO is already Disabled");
		return 0;
	}

	if (enable_flag) {
		qdf_atomic_set(&hdd_ctx->vendor_disable_lro_flag, 0);
		adapter->dev->features |= NETIF_F_LRO;
	} else {
		/* Disable LRO, Enable tcpdelack*/
		qdf_atomic_set(&hdd_ctx->vendor_disable_lro_flag, 1);
		adapter->dev->features &= ~NETIF_F_LRO;
		hdd_debug("LRO Disabled");

		if (hdd_ctx->en_tcp_delack_no_lro) {
			struct wlan_rx_tp_data rx_tp_data;

			hdd_debug("Enable TCP delack as LRO is disabled");
			rx_tp_data.rx_tp_flags = TCP_DEL_ACK_IND;
			rx_tp_data.level = GET_CUR_RX_LVL(hdd_ctx);
			wlan_hdd_send_svc_nlink_msg(hdd_ctx->radio_index,
						WLAN_SVC_WLAN_TP_IND,
						&rx_tp_data,
						sizeof(rx_tp_data));
			hdd_ctx->en_tcp_delack_no_lro = 1;
		}
	}
	return 0;
}

/**
 * hdd_disable_lro_in_concurrency() - Disable LRO due to concurrency
 * @disable: bool value
 *
 * Return: none
 */
void hdd_disable_lro_in_concurrency(bool disable)
{
	struct hdd_context *hdd_ctx = cds_get_context(QDF_MODULE_ID_HDD);

	if (disable) {
		if (hdd_ctx->en_tcp_delack_no_lro) {
			struct wlan_rx_tp_data rx_tp_data;

			hdd_info("Enable TCP delack as LRO disabled in concurrency");
			rx_tp_data.rx_tp_flags = TCP_DEL_ACK_IND;
			rx_tp_data.level = GET_CUR_RX_LVL(hdd_ctx);
			wlan_hdd_send_svc_nlink_msg(hdd_ctx->radio_index,
						WLAN_SVC_WLAN_TP_IND,
						&rx_tp_data,
						sizeof(rx_tp_data));
			hdd_ctx->en_tcp_delack_no_lro = 1;
		}
		qdf_atomic_set(&hdd_ctx->disable_lro_in_concurrency, 1);
	} else {
		if (hdd_ctx->en_tcp_delack_no_lro) {
			hdd_info("Disable TCP delack as LRO is enabled");
			hdd_ctx->en_tcp_delack_no_lro = 0;
			hdd_reset_tcp_delack(hdd_ctx);
		}
		qdf_atomic_set(&hdd_ctx->disable_lro_in_concurrency, 0);
	}
}

void hdd_disable_lro_for_low_tput(struct hdd_context *hdd_ctx, bool disable)
{
	if (disable)
		qdf_atomic_set(&hdd_ctx->disable_lro_in_low_tput, 1);
	else
		qdf_atomic_set(&hdd_ctx->disable_lro_in_low_tput, 0);
}
