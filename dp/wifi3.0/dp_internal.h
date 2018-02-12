/*
 * Copyright (c) 2016-2018 The Linux Foundation. All rights reserved.
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

#ifndef _DP_INTERNAL_H_
#define _DP_INTERNAL_H_

#include "dp_types.h"

#define RX_BUFFER_SIZE_PKTLOG_LITE 1024

#if DP_PRINT_ENABLE
#include <stdarg.h>       /* va_list */
#include <qdf_types.h> /* qdf_vprint */
#include <cdp_txrx_handle.h>

enum {
	/* FATAL_ERR - print only irrecoverable error messages */
	DP_PRINT_LEVEL_FATAL_ERR,

	/* ERR - include non-fatal err messages */
	DP_PRINT_LEVEL_ERR,

	/* WARN - include warnings */
	DP_PRINT_LEVEL_WARN,

	/* INFO1 - include fundamental, infrequent events */
	DP_PRINT_LEVEL_INFO1,

	/* INFO2 - include non-fundamental but infrequent events */
	DP_PRINT_LEVEL_INFO2,
};


#define dp_print(level, fmt, ...) do { \
	if (level <= g_txrx_print_level) \
		qdf_print(fmt, ## __VA_ARGS__); \
while (0)
#define DP_PRINT(level, fmt, ...) do { \
	dp_print(level, "DP: " fmt, ## __VA_ARGS__); \
while (0)
#else
#define DP_PRINT(level, fmt, ...)
#endif /* DP_PRINT_ENABLE */

#define DP_TRACE(LVL, fmt, args ...)                             \
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_##LVL,       \
		fmt, ## args)

#define DP_TRACE_STATS(LVL, fmt, args ...)                             \
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_##LVL,       \
		fmt, ## args)

#define DP_PRINT_STATS(fmt, args ...)	\
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,       \
		fmt, ## args)

#define DP_STATS_INIT(_handle) \
	qdf_mem_set(&((_handle)->stats), sizeof((_handle)->stats), 0x0)

#define DP_STATS_CLR(_handle) \
	qdf_mem_set(&((_handle)->stats), sizeof((_handle)->stats), 0x0)

#ifndef DISABLE_DP_STATS
#define DP_STATS_INC(_handle, _field, _delta) \
{ \
	if (likely(_handle)) \
		_handle->stats._field += _delta; \
}

#define DP_STATS_INCC(_handle, _field, _delta, _cond) \
{ \
	if (_cond && likely(_handle)) \
		_handle->stats._field += _delta; \
}

#define DP_STATS_DEC(_handle, _field, _delta) \
{ \
	if (likely(_handle)) \
		_handle->stats._field -= _delta; \
}

#define DP_STATS_UPD(_handle, _field, _delta) \
{ \
	if (likely(_handle)) \
		_handle->stats._field = _delta; \
}

#define DP_STATS_INC_PKT(_handle, _field, _count, _bytes) \
{ \
	DP_STATS_INC(_handle, _field.num, _count); \
	DP_STATS_INC(_handle, _field.bytes, _bytes) \
}

#define DP_STATS_INCC_PKT(_handle, _field, _count, _bytes, _cond) \
{ \
	DP_STATS_INCC(_handle, _field.num, _count, _cond); \
	DP_STATS_INCC(_handle, _field.bytes, _bytes, _cond) \
}

#define DP_STATS_AGGR(_handle_a, _handle_b, _field) \
{ \
	_handle_a->stats._field += _handle_b->stats._field; \
}

#define DP_STATS_AGGR_PKT(_handle_a, _handle_b, _field) \
{ \
	DP_STATS_AGGR(_handle_a, _handle_b, _field.num); \
	DP_STATS_AGGR(_handle_a, _handle_b, _field.bytes);\
}


#define DP_HIST_INIT() \
	uint32_t num_of_packets[MAX_PDEV_CNT] = {0};

#define DP_HIST_PACKET_COUNT_INC(_pdev_id) \
{ \
		++num_of_packets[_pdev_id]; \
}

#define DP_TX_HISTOGRAM_UPDATE(_pdev, _p_cntrs) \
	do {                                                              \
		if (_p_cntrs == 1) {                                      \
			DP_STATS_INC(_pdev,                               \
				tx_comp_histogram.pkts_1, 1);             \
		} else if (_p_cntrs > 1 && _p_cntrs <= 20) {              \
			DP_STATS_INC(_pdev,                               \
				tx_comp_histogram.pkts_2_20, 1);          \
		} else if (_p_cntrs > 20 && _p_cntrs <= 40) {             \
			DP_STATS_INC(_pdev,                               \
				tx_comp_histogram.pkts_21_40, 1);         \
		} else if (_p_cntrs > 40 && _p_cntrs <= 60) {             \
			DP_STATS_INC(_pdev,                               \
				tx_comp_histogram.pkts_41_60, 1);         \
		} else if (_p_cntrs > 60 && _p_cntrs <= 80) {             \
			DP_STATS_INC(_pdev,                               \
				tx_comp_histogram.pkts_61_80, 1);         \
		} else if (_p_cntrs > 80 && _p_cntrs <= 100) {            \
			DP_STATS_INC(_pdev,                               \
				tx_comp_histogram.pkts_81_100, 1);        \
		} else if (_p_cntrs > 100 && _p_cntrs <= 200) {           \
			DP_STATS_INC(_pdev,                               \
				tx_comp_histogram.pkts_101_200, 1);       \
		} else if (_p_cntrs > 200) {                              \
			DP_STATS_INC(_pdev,                               \
				tx_comp_histogram.pkts_201_plus, 1);      \
		}                                                         \
	} while (0)

#define DP_RX_HISTOGRAM_UPDATE(_pdev, _p_cntrs) \
	do {                                                              \
		if (_p_cntrs == 1) {                                      \
			DP_STATS_INC(_pdev,                               \
				rx_ind_histogram.pkts_1, 1);              \
		} else if (_p_cntrs > 1 && _p_cntrs <= 20) {              \
			DP_STATS_INC(_pdev,                               \
				rx_ind_histogram.pkts_2_20, 1);           \
		} else if (_p_cntrs > 20 && _p_cntrs <= 40) {             \
			DP_STATS_INC(_pdev,                               \
				rx_ind_histogram.pkts_21_40, 1);          \
		} else if (_p_cntrs > 40 && _p_cntrs <= 60) {             \
			DP_STATS_INC(_pdev,                               \
				rx_ind_histogram.pkts_41_60, 1);          \
		} else if (_p_cntrs > 60 && _p_cntrs <= 80) {             \
			DP_STATS_INC(_pdev,                               \
				rx_ind_histogram.pkts_61_80, 1);          \
		} else if (_p_cntrs > 80 && _p_cntrs <= 100) {            \
			DP_STATS_INC(_pdev,                               \
				rx_ind_histogram.pkts_81_100, 1);         \
		} else if (_p_cntrs > 100 && _p_cntrs <= 200) {           \
			DP_STATS_INC(_pdev,                               \
				rx_ind_histogram.pkts_101_200, 1);        \
		} else if (_p_cntrs > 200) {                              \
			DP_STATS_INC(_pdev,                               \
				rx_ind_histogram.pkts_201_plus, 1);       \
		}                                                         \
	} while (0)

#define DP_TX_HIST_STATS_PER_PDEV() \
	do { \
		uint8_t hist_stats = 0; \
		for (hist_stats = 0; hist_stats < soc->pdev_count; \
				hist_stats++) { \
			DP_TX_HISTOGRAM_UPDATE(soc->pdev_list[hist_stats], \
					num_of_packets[hist_stats]); \
		} \
	}  while (0)


#define DP_RX_HIST_STATS_PER_PDEV() \
	do { \
		uint8_t hist_stats = 0; \
		for (hist_stats = 0; hist_stats < soc->pdev_count; \
				hist_stats++) { \
			DP_RX_HISTOGRAM_UPDATE(soc->pdev_list[hist_stats], \
					num_of_packets[hist_stats]); \
		} \
	}  while (0)


#else
#define DP_STATS_INC(_handle, _field, _delta)
#define DP_STATS_INCC(_handle, _field, _delta, _cond)
#define DP_STATS_DEC(_handle, _field, _delta)
#define DP_STATS_UPD(_handle, _field, _delta)
#define DP_STATS_INC_PKT(_handle, _field, _count, _bytes)
#define DP_STATS_INCC_PKT(_handle, _field, _count, _bytes, _cond)
#define DP_STATS_AGGR(_handle_a, _handle_b, _field)
#define DP_STATS_AGGR_PKT(_handle_a, _handle_b, _field)
#define DP_HIST_INIT()
#define DP_HIST_PACKET_COUNT_INC(_pdev_id)
#define DP_TX_HISTOGRAM_UPDATE(_pdev, _p_cntrs)
#define DP_RX_HISTOGRAM_UPDATE(_pdev, _p_cntrs)
#define DP_RX_HIST_STATS_PER_PDEV()
#define DP_TX_HIST_STATS_PER_PDEV()
#endif

#define DP_HTT_T2H_HP_PIPE 5

#define DP_UPDATE_STATS(_tgtobj, _srcobj)	\
	do {				\
		uint8_t i;		\
		uint8_t pream_type;	\
		for (pream_type = 0; pream_type < DOT11_MAX; pream_type++) { \
			for (i = 0; i < MAX_MCS; i++) { \
				DP_STATS_AGGR(_tgtobj, _srcobj, \
					tx.pkt_type[pream_type].mcs_count[i]); \
				DP_STATS_AGGR(_tgtobj, _srcobj, \
					rx.pkt_type[pream_type].mcs_count[i]); \
			} \
		} \
		  \
		for (i = 0; i < MAX_BW; i++) { \
			DP_STATS_AGGR(_tgtobj, _srcobj, tx.bw[i]); \
			DP_STATS_AGGR(_tgtobj, _srcobj, rx.bw[i]); \
		} \
		  \
		for (i = 0; i < SS_COUNT; i++) \
			DP_STATS_AGGR(_tgtobj, _srcobj, rx.nss[i]); \
		\
		for (i = 0; i < WME_AC_MAX; i++) { \
			DP_STATS_AGGR(_tgtobj, _srcobj, tx.wme_ac_type[i]); \
			DP_STATS_AGGR(_tgtobj, _srcobj, rx.wme_ac_type[i]); \
			DP_STATS_AGGR(_tgtobj, _srcobj, tx.excess_retries_per_ac[i]); \
		\
		} \
		\
		for (i = 0; i < MAX_GI; i++) { \
			DP_STATS_AGGR(_tgtobj, _srcobj, tx.sgi_count[i]); \
			DP_STATS_AGGR(_tgtobj, _srcobj, rx.sgi_count[i]); \
		} \
		\
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.comp_pkt); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.ucast); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.mcast); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.bcast); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.tx_success); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.tx_failed); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.ofdma); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.stbc); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.ldpc); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.retries); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.non_amsdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.amsdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.fw_rem); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.fw_rem_tx); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.fw_rem_notx); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.fw_reason1); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.fw_reason2); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.fw_reason3); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.age_out); \
								\
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.err.mic_err); \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.err.decrypt_err); \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.non_ampdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.ampdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.non_amsdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.amsdu_cnt); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.to_stack); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.bcast);\
								\
		for (i = 0; i <  CDP_MAX_RX_RINGS; i++)	\
			DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.rcvd_reo[i]); \
									\
		_srcobj->stats.rx.unicast.num = \
			_srcobj->stats.rx.to_stack.num - \
					_srcobj->stats.rx.multicast.num; \
		_srcobj->stats.rx.unicast.bytes = \
			_srcobj->stats.rx.to_stack.bytes - \
					_srcobj->stats.rx.multicast.bytes; \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.unicast); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.multicast); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.raw); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.intra_bss.pkts); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.intra_bss.fail); \
								  \
		_tgtobj->stats.tx.last_ack_rssi =	\
			_srcobj->stats.tx.last_ack_rssi; \
	}  while (0)

extern int dp_peer_find_attach(struct dp_soc *soc);
extern void dp_peer_find_detach(struct dp_soc *soc);
extern void dp_peer_find_hash_add(struct dp_soc *soc, struct dp_peer *peer);
extern void dp_peer_find_hash_remove(struct dp_soc *soc, struct dp_peer *peer);
extern void dp_peer_find_hash_erase(struct dp_soc *soc);
extern void dp_peer_rx_init(struct dp_pdev *pdev, struct dp_peer *peer);
extern void dp_peer_cleanup(struct dp_vdev *vdev, struct dp_peer *peer);
extern void dp_peer_rx_cleanup(struct dp_vdev *vdev, struct dp_peer *peer);
extern void dp_peer_unref_delete(void *peer_handle);
extern void dp_rx_discard(struct dp_vdev *vdev, struct dp_peer *peer,
	unsigned tid, qdf_nbuf_t msdu_list);
extern void *dp_find_peer_by_addr(struct cdp_pdev *dev,
	uint8_t *peer_mac_addr, uint8_t *peer_id);
#ifndef CONFIG_WIN
QDF_STATUS dp_register_peer(struct cdp_pdev *pdev_handle,
		struct ol_txrx_desc_type *sta_desc);
QDF_STATUS dp_clear_peer(struct cdp_pdev *pdev_handle, uint8_t local_id);
void *dp_find_peer_by_addr_and_vdev(struct cdp_pdev *pdev_handle,
		struct cdp_vdev *vdev,
		uint8_t *peer_addr, uint8_t *local_id);
uint16_t dp_local_peer_id(void *peer);
void *dp_peer_find_by_local_id(struct cdp_pdev *pdev_handle, uint8_t local_id);
QDF_STATUS dp_peer_state_update(struct cdp_pdev *pdev_handle, uint8_t *peer_mac,
		enum ol_txrx_peer_state state);
QDF_STATUS dp_get_vdevid(void *peer_handle, uint8_t *vdev_id);
struct cdp_vdev *dp_get_vdev_by_sta_id(struct cdp_pdev *pdev_handle,
		uint8_t sta_id);
struct cdp_vdev *dp_get_vdev_for_peer(void *peer);
uint8_t *dp_peer_get_peer_mac_addr(void *peer);
int dp_get_peer_state(void *peer_handle);
void dp_local_peer_id_pool_init(struct dp_pdev *pdev);
void dp_local_peer_id_alloc(struct dp_pdev *pdev, struct dp_peer *peer);
void dp_local_peer_id_free(struct dp_pdev *pdev, struct dp_peer *peer);
qdf_time_t *dp_get_last_assoc_received(void *peer_handle);
qdf_time_t *dp_get_last_disassoc_received(void *peer_handle);
qdf_time_t *dp_get_last_deauth_received(void *peer_handle);
#endif
extern int dp_addba_requestprocess_wifi3(void *peer_handle,
	uint8_t dialogtoken, uint16_t tid, uint16_t batimeout,
	uint16_t buffersize, uint16_t startseqnum);
extern void dp_addba_responsesetup_wifi3(void *peer_handle, uint8_t tid,
	uint8_t *dialogtoken, uint16_t *statuscode,
	uint16_t *buffersize, uint16_t *batimeout);
extern void dp_set_addba_response(void *peer_handle, uint8_t tid,
	uint16_t statuscode);
extern int dp_delba_process_wifi3(void *peer_handle,
	int tid, uint16_t reasoncode);

extern int dp_rx_tid_setup_wifi3(struct dp_peer *peer, int tid,
	uint32_t ba_window_size, uint32_t start_seq);

extern QDF_STATUS dp_reo_send_cmd(struct dp_soc *soc,
	enum hal_reo_cmd_type type, struct hal_reo_cmd_params *params,
	void (*callback_fn), void *data);

extern void dp_reo_cmdlist_destroy(struct dp_soc *soc);
extern void dp_reo_status_ring_handler(struct dp_soc *soc);
void dp_aggregate_vdev_stats(struct dp_vdev *vdev);
void dp_rx_tid_stats_cb(struct dp_soc *soc, void *cb_ctxt,
	union hal_reo_status *reo_status);
void dp_rx_bar_stats_cb(struct dp_soc *soc, void *cb_ctxt,
		union hal_reo_status *reo_status);
uint16_t dp_tx_me_send_convert_ucast(struct cdp_vdev *vdev_handle,
		qdf_nbuf_t nbuf, uint8_t newmac[][DP_MAC_ADDR_LEN],
		uint8_t new_mac_cnt);
void dp_tx_me_alloc_descriptor(struct cdp_pdev *pdev);

void dp_tx_me_free_descriptor(struct cdp_pdev *pdev);
QDF_STATUS dp_h2t_ext_stats_msg_send(struct dp_pdev *pdev,
		uint32_t stats_type_upload_mask, uint32_t config_param_0,
		uint32_t config_param_1, uint32_t config_param_2,
		uint32_t config_param_3, int cookie, int cookie_msb);
void dp_htt_stats_print_tag(uint8_t tag_type, uint32_t *tag_buf);
void dp_htt_stats_copy_tag(struct dp_pdev *pdev, uint8_t tag_type, uint32_t *tag_buf);
void dp_peer_rxtid_stats(struct dp_peer *peer, void (*callback_fn),
		void *cb_ctxt);
void dp_set_pn_check_wifi3(struct cdp_vdev *vdev_handle,
	struct cdp_peer *peer_handle, enum cdp_sec_type sec_type,
	 uint32_t *rx_pn);

void *dp_get_pdev_for_mac_id(struct dp_soc *soc, uint32_t mac_id);
int dp_get_ring_id_for_mac_id(struct dp_soc *soc, uint32_t mac_id);
void dp_mark_peer_inact(void *peer_handle, bool inactive);
bool dp_set_inact_params(struct cdp_pdev *pdev_handle,
		 u_int16_t inact_check_interval,
		 u_int16_t inact_normal, u_int16_t inact_overload);
bool dp_start_inact_timer(struct cdp_pdev *pdev_handle, bool enable);
void dp_set_overload(struct cdp_pdev *pdev_handle, bool overload);
bool dp_peer_is_inact(void *peer_handle);
void dp_init_inact_timer(struct dp_soc *soc);
void dp_free_inact_timer(struct dp_soc *soc);

#ifdef WDI_EVENT_ENABLE
QDF_STATUS dp_h2t_cfg_stats_msg_send(struct dp_pdev *pdev,
				uint32_t stats_type_upload_mask,
				uint8_t mac_id);

int dp_wdi_event_unsub(struct cdp_pdev *txrx_pdev_handle,
	void *event_cb_sub_handle,
	uint32_t event);

int dp_wdi_event_sub(struct cdp_pdev *txrx_pdev_handle,
	void *event_cb_sub_handle,
	uint32_t event);

void dp_wdi_event_handler(enum WDI_EVENT event, void *soc,
		void *data, u_int16_t peer_id,
		int status, u_int8_t pdev_id);

int dp_wdi_event_attach(struct dp_pdev *txrx_pdev);
int dp_wdi_event_detach(struct dp_pdev *txrx_pdev);
int dp_set_pktlog_wifi3(struct dp_pdev *pdev, uint32_t event,
	bool enable);
void *dp_get_pldev(struct cdp_pdev *txrx_pdev);
void dp_pkt_log_init(struct cdp_pdev *ppdev, void *scn);

static inline void dp_hif_update_pipe_callback(void *soc, void *cb_context,
	QDF_STATUS (*callback)(void *, qdf_nbuf_t, uint8_t), uint8_t pipe_id)
{
	struct hif_msg_callbacks hif_pipe_callbacks;
	struct dp_soc *dp_soc = (struct dp_soc *)soc;

	/* TODO: Temporary change to bypass HTC connection for this new
	 * HIF pipe, which will be used for packet log and other high-
	 * priority HTT messsages. Proper HTC connection to be added
	 * later once required FW changes are available
	 */
	hif_pipe_callbacks.rxCompletionHandler = callback;
	hif_pipe_callbacks.Context = cb_context;
	hif_update_pipe_callback(dp_soc->hif_handle,
		DP_HTT_T2H_HP_PIPE, &hif_pipe_callbacks);
}

#else
static inline int dp_wdi_event_unsub(struct cdp_pdev *txrx_pdev_handle,
	void *event_cb_sub_handle,
	uint32_t event)
{
	return 0;
}

static inline int dp_wdi_event_sub(struct cdp_pdev *txrx_pdev_handle,
	void *event_cb_sub_handle,
	uint32_t event)
{
	return 0;
}

static inline void dp_wdi_event_handler(enum WDI_EVENT event, void *soc,
		void *data, u_int16_t peer_id,
		int status, u_int8_t pdev_id)
{
}

static inline int dp_wdi_event_attach(struct dp_pdev *txrx_pdev)
{
	return 0;
}

static inline int dp_wdi_event_detach(struct dp_pdev *txrx_pdev)
{
	return 0;
}

static inline int dp_set_pktlog_wifi3(struct dp_pdev *pdev, uint32_t event,
	bool enable)
{
	return 0;
}
static inline QDF_STATUS dp_h2t_cfg_stats_msg_send(struct dp_pdev *pdev,
		uint32_t stats_type_upload_mask, uint8_t mac_id);
{
	return 0;
}
static inline void dp_hif_update_pipe_callback(void *soc, void *cb_context,
	QDF_STATUS (*callback)(void *, qdf_nbuf_t, uint8_t), uint8_t pipe_id)
{
}
#endif /* CONFIG_WIN */
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
void dp_tx_dump_flow_pool_info(void *soc);
int dp_tx_delete_flow_pool(struct dp_soc *soc, struct dp_tx_desc_pool_s *pool,
	bool force);
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */
#endif /* #ifndef _DP_INTERNAL_H_ */
