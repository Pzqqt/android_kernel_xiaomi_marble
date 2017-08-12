/*
 * Copyright (c) 2013-2017 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#ifndef HDD_IPA_H__
#define HDD_IPA_H__

/**
 * DOC: wlan_hdd_ipa.h
 *
 * WLAN IPA interface module headers
 * Originally written by Qualcomm Atheros, Inc
 */

struct hdd_context;

/**
 * enum hdd_ipa_wlan_event - HDD IPA events
 * @HDD_IPA_CLIENT_CONNECT: Client Connects
 * @HDD_IPA_CLIENT_DISCONNECT: Client Disconnects
 * @HDD_IPA_AP_CONNECT: SoftAP is started
 * @HDD_IPA_AP_DISCONNECT: SoftAP is stopped
 * @HDD_IPA_STA_CONNECT: STA associates to AP
 * @HDD_IPA_STA_DISCONNECT: STA dissociates from AP
 * @HDD_IPA_CLIENT_CONNECT_EX: Peer associates/re-associates to softap
 * @HDD_IPA_WLAN_EVENT_MAX: Max value for the enum
 */
enum hdd_ipa_wlan_event {
	HDD_IPA_CLIENT_CONNECT,
	HDD_IPA_CLIENT_DISCONNECT,
	HDD_IPA_AP_CONNECT,
	HDD_IPA_AP_DISCONNECT,
	HDD_IPA_STA_CONNECT,
	HDD_IPA_STA_DISCONNECT,
	HDD_IPA_CLIENT_CONNECT_EX,
	HDD_IPA_WLAN_EVENT_MAX
};

#ifdef IPA_OFFLOAD

/**
 * enum hdd_ipa_forward_type: Type of forward packet received from IPA
 * @HDD_IPA_FORWARD_PKT_NONE: No forward packet
 * @HDD_IPA_FORWARD_PKT_LOCAL_STACK: Packet forwarded to kernel network stack
 * @HDD_IPA_FORWARD_PKT_DISCARD: Discarded packet before sending to kernel stack
 */
enum hdd_ipa_forward_type {
	HDD_IPA_FORWARD_PKT_NONE = 0,
	HDD_IPA_FORWARD_PKT_LOCAL_STACK = 1,
	HDD_IPA_FORWARD_PKT_DISCARD = 2
};

/**
 * FIXME: Temporary hack - until IPA functionality gets restored
 *
 */
typedef void (*hdd_ipa_nbuf_cb_fn)(qdf_nbuf_t);
void hdd_ipa_nbuf_cb(qdf_nbuf_t skb);  /* Fwd declare */
static inline hdd_ipa_nbuf_cb_fn wlan_hdd_stub_ipa_fn(void)
{
	return hdd_ipa_nbuf_cb;
};

QDF_STATUS hdd_ipa_init(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_ipa_cleanup(struct hdd_context *hdd_ctx);
void hdd_ipa_flush(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_ipa_process_rxt(void *cds_context, qdf_nbuf_t rxBuf,
	uint8_t sta_id);
int hdd_ipa_wlan_evt(struct hdd_adapter *adapter, uint8_t sta_id,
	enum hdd_ipa_wlan_event type, uint8_t *mac_addr);
int hdd_ipa_set_perf_level(struct hdd_context *hdd_ctx, uint64_t tx_packets,
	uint64_t rx_packets);
int hdd_ipa_suspend(struct hdd_context *hdd_ctx);
int hdd_ipa_resume(struct hdd_context *hdd_ctx);
void hdd_ipa_uc_stat_query(struct hdd_context *hdd_ctx, uint32_t *ipa_tx_diff,
	uint32_t *ipa_rx_diff);
void hdd_ipa_uc_rt_debug_host_dump(struct hdd_context *hdd_ctx);
void hdd_ipa_uc_stat_request(struct hdd_adapter *adapter, uint8_t reason);
void hdd_ipa_uc_sharing_stats_request(struct hdd_adapter *adapter,
				      uint8_t reset_stats);
void hdd_ipa_uc_set_quota(struct hdd_adapter *adapter, uint8_t set_quota,
			  uint64_t quota_bytes);
bool hdd_ipa_is_enabled(struct hdd_context *hdd_ctx);
bool hdd_ipa_uc_is_enabled(struct hdd_context *hdd_ctx);
#ifndef QCA_LL_TX_FLOW_CONTROL_V2
int hdd_ipa_send_mcc_scc_msg(struct hdd_context *hdd_ctx, bool mcc_mode);
#else
static inline int hdd_ipa_send_mcc_scc_msg(struct hdd_context *hdd_ctx,
					   bool mcc_mode)
{
	return 0;
}
#endif
int hdd_ipa_uc_ssr_reinit(struct hdd_context *hdd_ctx);
int hdd_ipa_uc_ssr_deinit(void);
void hdd_ipa_uc_force_pipe_shutdown(struct hdd_context *hdd_ctx);
struct sk_buff *hdd_ipa_tx_packet_ipa(struct hdd_context *hdd_ctx,
	struct sk_buff *skb, uint8_t session_id);
bool hdd_ipa_is_present(struct hdd_context *hdd_ctx);
void hdd_ipa_dump_info(struct hdd_context *hdd_ctx);
QDF_STATUS hdd_ipa_uc_ol_init(struct hdd_context *hdd_ctx);
void hdd_ipa_set_tx_flow_info(void);
int hdd_ipa_uc_ol_deinit(struct hdd_context *hdd_ctx);
#else
static inline QDF_STATUS hdd_ipa_init(struct hdd_context *hdd_ctx)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS hdd_ipa_cleanup(struct hdd_context *hdd_ctx)
{
	return QDF_STATUS_SUCCESS;
}

static inline void hdd_ipa_flush(struct hdd_context *hdd_ctx)
{
}

static inline QDF_STATUS hdd_ipa_process_rxt(void *cds_context,
	qdf_nbuf_t rxBuf, uint8_t sta_id)
{
	return QDF_STATUS_SUCCESS;
}

static inline int hdd_ipa_wlan_evt(struct hdd_adapter *adapter, uint8_t sta_id,
	enum hdd_ipa_wlan_event type, uint8_t *mac_addr)
{
	return 0;
}

static inline int hdd_ipa_send_mcc_scc_msg(struct hdd_context *hdd_ctx,
	bool mcc_mode)
{
	return 0;
}

static inline int hdd_ipa_set_perf_level(struct hdd_context *hdd_ctx,
	uint64_t tx_packets,
	uint64_t rx_packets)
{
	return 0;
}

static inline int hdd_ipa_suspend(struct hdd_context *hdd_ctx)
{
	return 0;
}

static inline int hdd_ipa_resume(struct hdd_context *hdd_ctx)
{
	return 0;
}

static inline void hdd_ipa_uc_stat_query(struct hdd_context *hdd_ctx,
	uint32_t *ipa_tx_diff,
	uint32_t *ipa_rx_diff)
{
	*ipa_tx_diff = 0;
	*ipa_rx_diff = 0;
}

static inline void hdd_ipa_uc_stat_request(struct hdd_adapter *adapter,
	uint8_t reason)
{
}

static inline void hdd_ipa_uc_rt_debug_host_dump(struct hdd_context *hdd_ctx)
{
}

static inline bool hdd_ipa_is_enabled(struct hdd_context *hdd_ctx)
{
	return false;
}

static inline bool hdd_ipa_uc_is_enabled(struct hdd_context *hdd_ctx)
{
	return false;
}

static inline void hdd_ipa_dump_info(struct hdd_context *hdd_ctx)
{
}

static inline int hdd_ipa_uc_ssr_reinit(struct hdd_context *hdd_ctx)
{
	return false;
}

static inline int hdd_ipa_uc_ssr_deinit(void)
{
	return false;
}
static inline void hdd_ipa_uc_force_pipe_shutdown(struct hdd_context *hdd_ctx)
{
}

/**
 * hdd_ipa_tx_packet_ipa() - send packet to IPA
 * @hdd_ctx:    Global HDD context
 * @skb:        skb sent to IPA
 * @session_id: send packet instance session id
 *
 * Send TX packet which generated by system to IPA.
 * This routine only will be used for function verification
 *
 * Return: NULL packet sent to IPA properly
 *         skb packet not sent to IPA. legacy data path should handle
 */
static inline struct sk_buff *hdd_ipa_tx_packet_ipa(struct hdd_context *hdd_ctx,
	struct sk_buff *skb, uint8_t session_id)
{
	return skb;
}

/**
 * hdd_ipa_is_present() - get IPA hw status
 * @hdd_ctx: pointer to hdd context
 *
 * ipa_uc_reg_rdyCB is not directly designed to check
 * ipa hw status. This is an undocumented function which
 * has confirmed with IPA team.
 *
 * Return: true - ipa hw present
 *         false - ipa hw not present
 */
static inline bool hdd_ipa_is_present(struct hdd_context *hdd_ctx)
{
	return false;
}

/**
 * hdd_ipa_uc_ol_init() - Initialize IPA uC offload
 * @hdd_ctx: Global HDD context
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS hdd_ipa_uc_ol_init(struct hdd_context *hdd_ctx)
{
	return QDF_STATUS_SUCCESS;
}

static inline void hdd_ipa_set_tx_flow_info(void)
{
	return;
}

/**
 * hdd_ipa_uc_ol_deinit() - Disconnect IPA TX and RX pipes
 * @hdd_ctx: Global HDD context
 *
 * Return: 0 on success, negativer errno on error
 */
static inline int hdd_ipa_uc_ol_deinit(struct hdd_context *hdd_ctx)
{
	return 0;
}
#endif /* IPA_OFFLOAD */
#endif /* #ifndef HDD_IPA_H__ */
