/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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

#define DP_PEER_WDS_COUNT_INVALID UINT_MAX

/* Alignment for consistent memory for DP rings*/
#define DP_RING_BASE_ALIGN 32

#define DP_RSSI_INVAL 0x80
#define DP_RSSI_AVG_WEIGHT 2
/*
 * Formula to derive avg_rssi is taken from wifi2.o firmware
 */
#define DP_GET_AVG_RSSI(avg_rssi, last_rssi) \
	(((avg_rssi) - (((uint8_t)(avg_rssi)) >> DP_RSSI_AVG_WEIGHT)) \
	+ ((((uint8_t)(last_rssi)) >> DP_RSSI_AVG_WEIGHT)))

/* Macro For NYSM value received in VHT TLV */
#define VHT_SGI_NYSM 3

/* struct htt_dbgfs_cfg - structure to maintain required htt data
 * @msg_word: htt msg sent to upper layer
 * @m: qdf debugfs file pointer
 */
struct htt_dbgfs_cfg {
	uint32_t *msg_word;
	qdf_debugfs_file_t m;
};

/* Cookie MSB bits assigned for different use case.
 * Note: User can't use last 3 bits, as it is reserved for pdev_id.
 * If in future number of pdev are more than 3.
 */
/* Reserve for default case */
#define DBG_STATS_COOKIE_DEFAULT 0x0

/* Reserve for DP Stats: 3rd bit */
#define DBG_STATS_COOKIE_DP_STATS BIT(3)

/* Reserve for HTT Stats debugfs support: 4th bit */
#define DBG_STATS_COOKIE_HTT_DBGFS BIT(4)

/*Reserve for HTT Stats debugfs support: 5th bit */
#define DBG_SYSFS_STATS_COOKIE BIT(5)

/**
 * Bitmap of HTT PPDU TLV types for Default mode
 */
#define HTT_PPDU_DEFAULT_TLV_BITMAP \
	(1 << HTT_PPDU_STATS_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_RATE_TLV) | \
	(1 << HTT_PPDU_STATS_SCH_CMD_STATUS_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV)

/* PPDU STATS CFG */
#define DP_PPDU_STATS_CFG_ALL 0xFFFF

/* PPDU stats mask sent to FW to enable enhanced stats */
#define DP_PPDU_STATS_CFG_ENH_STATS \
	(HTT_PPDU_DEFAULT_TLV_BITMAP) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_FLUSH_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMMON_ARRAY_TLV) | \
	(1 << HTT_PPDU_STATS_USERS_INFO_TLV)

/* PPDU stats mask sent to FW to support debug sniffer feature */
#define DP_PPDU_STATS_CFG_SNIFFER \
	(HTT_PPDU_DEFAULT_TLV_BITMAP) | \
	(1 << HTT_PPDU_STATS_USR_MPDU_ENQ_BITMAP_64_TLV) | \
	(1 << HTT_PPDU_STATS_USR_MPDU_ENQ_BITMAP_256_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_64_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_256_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_FLUSH_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_256_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_FLUSH_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMMON_ARRAY_TLV) | \
	(1 << HTT_PPDU_STATS_TX_MGMTCTRL_PAYLOAD_TLV) | \
	(1 << HTT_PPDU_STATS_USERS_INFO_TLV)

/* PPDU stats mask sent to FW to support BPR feature*/
#define DP_PPDU_STATS_CFG_BPR \
	(1 << HTT_PPDU_STATS_TX_MGMTCTRL_PAYLOAD_TLV) | \
	(1 << HTT_PPDU_STATS_USERS_INFO_TLV)

/* PPDU stats mask sent to FW to support BPR and enhanced stats feature */
#define DP_PPDU_STATS_CFG_BPR_ENH (DP_PPDU_STATS_CFG_BPR | \
				   DP_PPDU_STATS_CFG_ENH_STATS)
/* PPDU stats mask sent to FW to support BPR and pcktlog stats feature */
#define DP_PPDU_STATS_CFG_BPR_PKTLOG (DP_PPDU_STATS_CFG_BPR | \
				      DP_PPDU_TXLITE_STATS_BITMASK_CFG)

/**
 * Bitmap of HTT PPDU delayed ba TLV types for Default mode
 */
#define HTT_PPDU_DELAYED_BA_TLV_BITMAP \
	(1 << HTT_PPDU_STATS_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_RATE_TLV)

/**
 * Bitmap of HTT PPDU TLV types for Delayed BA
 */
#define HTT_PPDU_STATUS_TLV_BITMAP \
	(1 << HTT_PPDU_STATS_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV)

/**
 * Bitmap of HTT PPDU TLV types for Sniffer mode bitmap 64
 */
#define HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP_64 \
	((1 << HTT_PPDU_STATS_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_RATE_TLV) | \
	(1 << HTT_PPDU_STATS_SCH_CMD_STATUS_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_64_TLV) | \
	(1 << HTT_PPDU_STATS_USR_MPDU_ENQ_BITMAP_64_TLV))

/**
 * Bitmap of HTT PPDU TLV types for Sniffer mode bitmap 256
 */
#define HTT_PPDU_SNIFFER_AMPDU_TLV_BITMAP_256 \
	((1 << HTT_PPDU_STATS_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_RATE_TLV) | \
	(1 << HTT_PPDU_STATS_SCH_CMD_STATUS_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_COMMON_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_ACK_BA_STATUS_TLV) | \
	(1 << HTT_PPDU_STATS_USR_COMPLTN_BA_BITMAP_256_TLV) | \
	(1 << HTT_PPDU_STATS_USR_MPDU_ENQ_BITMAP_256_TLV))

QDF_STATUS dp_mon_soc_attach(struct dp_soc *soc);
QDF_STATUS dp_mon_soc_detach(struct dp_soc *soc);

#ifdef MONITOR_MODULARIZED_ENABLE
static inline bool dp_monitor_modularized_enable(void)
{
	return TRUE;
}

static inline QDF_STATUS
dp_mon_soc_attach_wrapper(struct dp_soc *soc) { return QDF_STATUS_SUCCESS; }

static inline QDF_STATUS
dp_mon_soc_detach_wrapper(struct dp_soc *soc) { return QDF_STATUS_SUCCESS; }
#else
static inline bool dp_monitor_modularized_enable(void)
{
	return FALSE;
}

static inline QDF_STATUS dp_mon_soc_attach_wrapper(struct dp_soc *soc)
{
	return dp_mon_soc_attach(soc);
}

static inline QDF_STATUS dp_mon_soc_detach_wrapper(struct dp_soc *soc)
{
	return dp_mon_soc_detach(soc);
}
#endif

#ifndef WIFI_MONITOR_SUPPORT
#define MON_BUF_MIN_ENTRIES 64

static inline QDF_STATUS dp_monitor_pdev_attach(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_monitor_pdev_detach(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_monitor_vdev_attach(struct dp_vdev *vdev)
{
	return QDF_STATUS_E_FAILURE;
}

static inline QDF_STATUS dp_monitor_vdev_detach(struct dp_vdev *vdev)
{
	return QDF_STATUS_E_FAILURE;
}

static inline QDF_STATUS dp_monitor_peer_attach(struct dp_soc *soc,
						struct dp_peer *peer)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_monitor_peer_detach(struct dp_soc *soc,
						struct dp_peer *peer)
{
	return QDF_STATUS_E_FAILURE;
}

static inline QDF_STATUS dp_monitor_pdev_init(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_monitor_pdev_deinit(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_monitor_soc_cfg_init(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

static inline QDF_STATUS dp_monitor_config_debug_sniffer(struct dp_pdev *pdev,
							 int val)
{
	return QDF_STATUS_E_FAILURE;
}

static inline void dp_monitor_flush_rings(struct dp_soc *soc)
{
}

static inline QDF_STATUS dp_monitor_htt_srng_setup(struct dp_soc *soc,
						   struct dp_pdev *pdev,
						   int mac_id,
						   int mac_for_pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline void dp_monitor_service_mon_rings(struct dp_soc *soc,
						uint32_t quota)
{
}

static inline
uint32_t dp_monitor_process(struct dp_soc *soc, struct dp_intr *int_ctx,
			    uint32_t mac_id, uint32_t quota)
{
	return 0;
}

static inline
uint32_t dp_monitor_drop_packets_for_mac(struct dp_pdev *pdev,
					 uint32_t mac_id, uint32_t quota)
{
	return 0;
}

static inline void dp_monitor_peer_tx_init(struct dp_pdev *pdev,
					   struct dp_peer *peer)
{
}

static inline void dp_monitor_peer_tx_cleanup(struct dp_vdev *vdev,
					      struct dp_peer *peer)
{
}

static inline
void dp_monitor_peer_tid_peer_id_update(struct dp_soc *soc,
					struct dp_peer *peer,
					uint16_t peer_id)
{
}

static inline void dp_monitor_tx_ppdu_stats_attach(struct dp_pdev *pdev)
{
}

static inline void dp_monitor_tx_ppdu_stats_detach(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_monitor_tx_capture_debugfs_init(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline void dp_monitor_peer_tx_capture_filter_check(struct dp_pdev *pdev,
							   struct dp_peer *peer)
{
}

static inline
QDF_STATUS dp_monitor_tx_add_to_comp_queue(struct dp_soc *soc,
					   struct dp_tx_desc_s *desc,
					   struct hal_tx_completion_status *ts,
					   struct dp_peer *peer)
{
	return QDF_STATUS_E_FAILURE;
}

static inline
QDF_STATUS monitor_update_msdu_to_list(struct dp_soc *soc,
				       struct dp_pdev *pdev,
				       struct dp_peer *peer,
				       struct hal_tx_completion_status *ts,
				       qdf_nbuf_t netbuf)
{
	return QDF_STATUS_E_FAILURE;
}

static inline bool dp_monitor_ppdu_stats_ind_handler(struct htt_soc *soc,
						     uint32_t *msg_word,
						     qdf_nbuf_t htt_t2h_msg)
{
	return true;
}

static inline QDF_STATUS dp_monitor_htt_ppdu_stats_attach(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

static inline void dp_monitor_htt_ppdu_stats_detach(struct dp_pdev *pdev)
{
}

static inline void dp_monitor_print_pdev_rx_mon_stats(struct dp_pdev *pdev)
{
}

static inline QDF_STATUS dp_monitor_config_enh_tx_capture(struct dp_pdev *pdev,
							  uint32_t val)
{
	return QDF_STATUS_E_INVAL;
}

static inline QDF_STATUS dp_monitor_config_enh_rx_capture(struct dp_pdev *pdev,
							  uint32_t val)
{
	return QDF_STATUS_E_INVAL;
}

static inline
QDF_STATUS dp_monitor_set_bpr_enable(struct dp_pdev *pdev, uint32_t val)
{
	return QDF_STATUS_E_FAILURE;
}

static inline
int dp_monitor_set_filter_neigh_peers(struct dp_pdev *pdev, bool val)
{
	return 0;
}

static inline
void dp_monitor_set_atf_stats_enable(struct dp_pdev *pdev, bool value)
{
}

static inline
void dp_monitor_set_bsscolor(struct dp_pdev *pdev, uint8_t bsscolor)
{
}

static inline
bool dp_monitor_pdev_get_filter_mcast_data(struct cdp_pdev *pdev_handle)
{
	return false;
}

static inline
bool dp_monitor_pdev_get_filter_non_data(struct cdp_pdev *pdev_handle)
{
	return false;
}

static inline
bool dp_monitor_pdev_get_filter_ucast_data(struct cdp_pdev *pdev_handle)
{
	return false;
}

static inline
int dp_monitor_set_pktlog_wifi3(struct dp_pdev *pdev, uint32_t event,
				bool enable)
{
	return 0;
}

static inline void dp_monitor_pktlogmod_exit(struct dp_pdev *pdev)
{
}

static inline
void dp_monitor_vdev_set_monitor_mode_buf_rings(struct dp_pdev *pdev)
{
}

static inline
void dp_monitor_neighbour_peers_detach(struct dp_pdev *pdev)
{
}

static inline QDF_STATUS dp_monitor_filter_neighbour_peer(struct dp_pdev *pdev,
							  uint8_t *rx_pkt_hdr)
{
	return QDF_STATUS_E_FAILURE;
}

static inline void dp_monitor_print_pdev_tx_capture_stats(struct dp_pdev *pdev)
{
}

static inline
void dp_monitor_reap_timer_init(struct dp_soc *soc)
{
}

static inline
void dp_monitor_reap_timer_deinit(struct dp_soc *soc)
{
}

static inline
void dp_monitor_reap_timer_start(struct dp_soc *soc)
{
}

static inline
bool dp_monitor_reap_timer_stop(struct dp_soc *soc)
{
	return false;
}

static inline
void dp_monitor_vdev_timer_init(struct dp_soc *soc)
{
}

static inline
void dp_monitor_vdev_timer_deinit(struct dp_soc *soc)
{
}

static inline
void dp_monitor_vdev_timer_start(struct dp_soc *soc)
{
}

static inline
bool dp_monitor_vdev_timer_stop(struct dp_soc *soc)
{
	return false;
}

static inline struct qdf_mem_multi_page_t*
dp_monitor_get_link_desc_pages(struct dp_soc *soc, uint32_t mac_id)
{
	return NULL;
}

static inline uint32_t *
dp_monitor_get_total_link_descs(struct dp_soc *soc, uint32_t mac_id)
{
	return NULL;
}

static inline QDF_STATUS dp_monitor_drop_inv_peer_pkts(struct dp_vdev *vdev)
{
	return QDF_STATUS_E_FAILURE;
}

static inline bool dp_is_enable_reap_timer_non_pkt(struct dp_pdev *pdev)
{
	return false;
}

static inline void dp_monitor_vdev_register_osif(struct dp_vdev *vdev,
						 struct ol_txrx_ops *txrx_ops)
{
}

static inline bool dp_monitor_is_vdev_timer_running(struct dp_soc *soc)
{
	return false;
}

static inline
void dp_monitor_pdev_set_mon_vdev(struct dp_pdev *pdev)
{
}

static inline void dp_monitor_vdev_delete(struct dp_soc *soc,
					  struct dp_vdev *vdev)
{
}

static inline void dp_peer_ppdu_delayed_ba_init(struct dp_peer *peer)
{
}

static inline void dp_monitor_neighbour_peer_add_ast(struct dp_pdev *pdev,
						     struct dp_peer *ta_peer,
						     uint8_t *mac_addr,
						     qdf_nbuf_t nbuf,
						     uint32_t flags)
{
}

static inline void
dp_monitor_set_chan_band(struct dp_pdev *pdev, enum reg_wifi_band chan_band)
{
}

static inline void
dp_monitor_set_chan_freq(struct dp_pdev *pdev, qdf_freq_t chan_freq)
{
}

static inline void dp_monitor_set_chan_num(struct dp_pdev *pdev, int chan_num)
{
}

static inline bool dp_monitor_is_enable_mcopy_mode(struct dp_pdev *pdev)
{
	return false;
}

static inline
void dp_monitor_neighbour_peer_list_remove(struct dp_pdev *pdev,
					   struct dp_vdev *vdev,
					   struct dp_neighbour_peer *peer)
{
}

static inline bool dp_monitor_is_chan_band_known(struct dp_pdev *pdev)
{
	return false;
}

static inline enum reg_wifi_band
dp_monitor_get_chan_band(struct dp_pdev *pdev)
{
	return 0;
}

static inline void dp_monitor_get_mpdu_status(struct dp_pdev *pdev,
					      struct dp_soc *soc,
					      uint8_t *rx_tlv_hdr)
{
}

static inline void dp_monitor_print_tx_stats(struct dp_pdev *pdev)
{
}

static inline
QDF_STATUS dp_monitor_mcopy_check_deliver(struct dp_pdev *pdev,
					  uint16_t peer_id, uint32_t ppdu_id,
					  uint8_t first_msdu)
{
	return QDF_STATUS_SUCCESS;
}

static inline bool dp_monitor_is_enable_tx_sniffer(struct dp_pdev *pdev)
{
	return false;
}

static inline struct dp_vdev*
dp_monitor_get_monitor_vdev_from_pdev(struct dp_pdev *pdev)
{
	return NULL;
}

static inline QDF_STATUS dp_monitor_check_com_info_ppdu_id(struct dp_pdev *pdev,
							   void *rx_desc)
{
	return QDF_STATUS_E_FAILURE;
}

static inline struct mon_rx_status*
dp_monitor_get_rx_status(struct dp_pdev *pdev)
{
	return NULL;
}

static inline
void dp_monitor_pdev_config_scan_spcl_vap(struct dp_pdev *pdev)
{
}

static inline
void dp_monitor_pdev_reset_scan_spcl_vap_stats_enable(struct dp_pdev *pdev,
						      bool val)
{
}
#endif

/**
 * cdp_soc_t_to_dp_soc() - typecast cdp_soc_t to
 * dp soc handle
 * @psoc: CDP psoc handle
 *
 * Return: struct dp_soc pointer
 */
static inline
struct dp_soc *cdp_soc_t_to_dp_soc(struct cdp_soc_t *psoc)
{
	return (struct dp_soc *)psoc;
}

#define DP_MAX_TIMER_EXEC_TIME_TICKS \
		(QDF_LOG_TIMESTAMP_CYCLES_PER_10_US * 100 * 20)

/**
 * enum timer_yield_status - yield status code used in monitor mode timer.
 * @DP_TIMER_NO_YIELD: do not yield
 * @DP_TIMER_WORK_DONE: yield because work is done
 * @DP_TIMER_WORK_EXHAUST: yield because work quota is exhausted
 * @DP_TIMER_TIME_EXHAUST: yield due to time slot exhausted
 */
enum timer_yield_status {
	DP_TIMER_NO_YIELD,
	DP_TIMER_WORK_DONE,
	DP_TIMER_WORK_EXHAUST,
	DP_TIMER_TIME_EXHAUST,
};

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

#ifdef WLAN_SYSFS_DP_STATS
void DP_PRINT_STATS(const char *fmt, ...);
#else /* WLAN_SYSFS_DP_STATS */
#ifdef DP_PRINT_NO_CONSOLE
/* Stat prints should not go to console or kernel logs.*/
#define DP_PRINT_STATS(fmt, args ...)\
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,       \
		  fmt, ## args)
#else
#define DP_PRINT_STATS(fmt, args ...)\
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_FATAL,\
		  fmt, ## args)
#endif
#endif /* WLAN_SYSFS_DP_STATS */

#define DP_STATS_INIT(_handle) \
	qdf_mem_zero(&((_handle)->stats), sizeof((_handle)->stats))

#define DP_STATS_CLR(_handle) \
	qdf_mem_zero(&((_handle)->stats), sizeof((_handle)->stats))

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

#define DP_STATS_UPD_STRUCT(_handle_a, _handle_b, _field) \
{ \
	_handle_a->stats._field = _handle_b->stats._field; \
}

#else
#define DP_STATS_INC(_handle, _field, _delta)
#define DP_STATS_INCC(_handle, _field, _delta, _cond)
#define DP_STATS_DEC(_handle, _field, _delta)
#define DP_STATS_UPD(_handle, _field, _delta)
#define DP_STATS_INC_PKT(_handle, _field, _count, _bytes)
#define DP_STATS_INCC_PKT(_handle, _field, _count, _bytes, _cond)
#define DP_STATS_AGGR(_handle_a, _handle_b, _field)
#define DP_STATS_AGGR_PKT(_handle_a, _handle_b, _field)
#endif

#ifdef ENABLE_DP_HIST_STATS
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
#define DP_HIST_INIT()
#define DP_HIST_PACKET_COUNT_INC(_pdev_id)
#define DP_TX_HISTOGRAM_UPDATE(_pdev, _p_cntrs)
#define DP_RX_HISTOGRAM_UPDATE(_pdev, _p_cntrs)
#define DP_RX_HIST_STATS_PER_PDEV()
#define DP_TX_HIST_STATS_PER_PDEV()
#endif /* DISABLE_DP_STATS */

#define FRAME_MASK_IPV4_ARP   1
#define FRAME_MASK_IPV4_DHCP  2
#define FRAME_MASK_IPV4_EAPOL 4
#define FRAME_MASK_IPV6_DHCP  8

static inline int dp_log2_ceil(unsigned int value)
{
	unsigned int tmp = value;
	int log2 = -1;

	while (tmp) {
		log2++;
		tmp >>= 1;
	}
	if (1 << log2 != value)
		log2++;
	return log2;
}

#ifdef QCA_SUPPORT_PEER_ISOLATION
#define dp_get_peer_isolation(_peer) ((_peer)->isolation)

static inline void dp_set_peer_isolation(struct dp_peer *peer, bool val)
{
	peer->isolation = val;
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		  "peer:"QDF_MAC_ADDR_FMT" isolation:%d",
		  QDF_MAC_ADDR_REF(peer->mac_addr.raw), peer->isolation);
}

#else
#define dp_get_peer_isolation(_peer) (0)

static inline void dp_set_peer_isolation(struct dp_peer *peer, bool val)
{
}
#endif /* QCA_SUPPORT_PEER_ISOLATION */

#ifdef QCA_SUPPORT_WDS_EXTENDED
static inline void dp_wds_ext_peer_init(struct dp_peer *peer)
{
	peer->wds_ext.init = 0;
}
#else
static inline void dp_wds_ext_peer_init(struct dp_peer *peer)
{
}
#endif /* QCA_SUPPORT_WDS_EXTENDED */

#ifdef QCA_HOST2FW_RXBUF_RING
static inline
struct dp_srng *dp_get_rxdma_ring(struct dp_pdev *pdev, int lmac_id)
{
	return &pdev->rx_mac_buf_ring[lmac_id];
}
#else
static inline
struct dp_srng *dp_get_rxdma_ring(struct dp_pdev *pdev, int lmac_id)
{
	return &pdev->soc->rx_refill_buf_ring[lmac_id];
}
#endif

/**
 * The lmac ID for a particular channel band is fixed.
 * 2.4GHz band uses lmac_id = 1
 * 5GHz/6GHz band uses lmac_id=0
 */
#define DP_INVALID_LMAC_ID	(-1)
#define DP_MON_INVALID_LMAC_ID	(-1)
#define DP_MON_2G_LMAC_ID	1
#define DP_MON_5G_LMAC_ID	0
#define DP_MON_6G_LMAC_ID	0

#ifdef FEATURE_TSO_STATS
/**
 * dp_init_tso_stats() - Clear tso stats
 * @pdev: pdev handle
 *
 * Return: None
 */
static inline
void dp_init_tso_stats(struct dp_pdev *pdev)
{
	if (pdev) {
		qdf_mem_zero(&((pdev)->stats.tso_stats),
			     sizeof((pdev)->stats.tso_stats));
		qdf_atomic_init(&pdev->tso_idx);
	}
}

/**
 * dp_stats_tso_segment_histogram_update() - TSO Segment Histogram
 * @pdev: pdev handle
 * @_p_cntrs: number of tso segments for a tso packet
 *
 * Return: None
 */
void dp_stats_tso_segment_histogram_update(struct dp_pdev *pdev,
					   uint8_t _p_cntrs);

/**
 * dp_tso_segment_update() - Collect tso segment information
 * @pdev: pdev handle
 * @stats_idx: tso packet number
 * @idx: tso segment number
 * @seg: tso segment
 *
 * Return: None
 */
void dp_tso_segment_update(struct dp_pdev *pdev,
			   uint32_t stats_idx,
			   uint8_t idx,
			   struct qdf_tso_seg_t seg);

/**
 * dp_tso_packet_update() - TSO Packet information
 * @pdev: pdev handle
 * @stats_idx: tso packet number
 * @msdu: nbuf handle
 * @num_segs: tso segments
 *
 * Return: None
 */
void dp_tso_packet_update(struct dp_pdev *pdev, uint32_t stats_idx,
			  qdf_nbuf_t msdu, uint16_t num_segs);

/**
 * dp_tso_segment_stats_update() - TSO Segment stats
 * @pdev: pdev handle
 * @stats_seg: tso segment list
 * @stats_idx: tso packet number
 *
 * Return: None
 */
void dp_tso_segment_stats_update(struct dp_pdev *pdev,
				 struct qdf_tso_seg_elem_t *stats_seg,
				 uint32_t stats_idx);

/**
 * dp_print_tso_stats() - dump tso statistics
 * @soc:soc handle
 * @level: verbosity level
 *
 * Return: None
 */
void dp_print_tso_stats(struct dp_soc *soc,
			enum qdf_stats_verbosity_level level);

/**
 * dp_txrx_clear_tso_stats() - clear tso stats
 * @soc: soc handle
 *
 * Return: None
 */
void dp_txrx_clear_tso_stats(struct dp_soc *soc);
#else
static inline
void dp_init_tso_stats(struct dp_pdev *pdev)
{
}

static inline
void dp_stats_tso_segment_histogram_update(struct dp_pdev *pdev,
					   uint8_t _p_cntrs)
{
}

static inline
void dp_tso_segment_update(struct dp_pdev *pdev,
			   uint32_t stats_idx,
			   uint32_t idx,
			   struct qdf_tso_seg_t seg)
{
}

static inline
void dp_tso_packet_update(struct dp_pdev *pdev, uint32_t stats_idx,
			  qdf_nbuf_t msdu, uint16_t num_segs)
{
}

static inline
void dp_tso_segment_stats_update(struct dp_pdev *pdev,
				 struct qdf_tso_seg_elem_t *stats_seg,
				 uint32_t stats_idx)
{
}

static inline
void dp_print_tso_stats(struct dp_soc *soc,
			enum qdf_stats_verbosity_level level)
{
}

static inline
void dp_txrx_clear_tso_stats(struct dp_soc *soc)
{
}
#endif /* FEATURE_TSO_STATS */

#define DP_HTT_T2H_HP_PIPE 5
static inline void dp_update_pdev_stats(struct dp_pdev *tgtobj,
					struct cdp_vdev_stats *srcobj)
{
	uint8_t i;
	uint8_t pream_type;

	for (pream_type = 0; pream_type < DOT11_MAX; pream_type++) {
		for (i = 0; i < MAX_MCS; i++) {
			tgtobj->stats.tx.pkt_type[pream_type].
				mcs_count[i] +=
			srcobj->tx.pkt_type[pream_type].
				mcs_count[i];
			tgtobj->stats.rx.pkt_type[pream_type].
				mcs_count[i] +=
			srcobj->rx.pkt_type[pream_type].
				mcs_count[i];
		}
	}

	for (i = 0; i < MAX_BW; i++) {
		tgtobj->stats.tx.bw[i] += srcobj->tx.bw[i];
		tgtobj->stats.rx.bw[i] += srcobj->rx.bw[i];
	}

	for (i = 0; i < SS_COUNT; i++) {
		tgtobj->stats.tx.nss[i] += srcobj->tx.nss[i];
		tgtobj->stats.rx.nss[i] += srcobj->rx.nss[i];
	}

	for (i = 0; i < WME_AC_MAX; i++) {
		tgtobj->stats.tx.wme_ac_type[i] +=
			srcobj->tx.wme_ac_type[i];
		tgtobj->stats.rx.wme_ac_type[i] +=
			srcobj->rx.wme_ac_type[i];
		tgtobj->stats.tx.excess_retries_per_ac[i] +=
			srcobj->tx.excess_retries_per_ac[i];
	}

	for (i = 0; i < MAX_GI; i++) {
		tgtobj->stats.tx.sgi_count[i] +=
			srcobj->tx.sgi_count[i];
		tgtobj->stats.rx.sgi_count[i] +=
			srcobj->rx.sgi_count[i];
	}

	for (i = 0; i < MAX_RECEPTION_TYPES; i++)
		tgtobj->stats.rx.reception_type[i] +=
			srcobj->rx.reception_type[i];

	tgtobj->stats.tx.comp_pkt.bytes += srcobj->tx.comp_pkt.bytes;
	tgtobj->stats.tx.comp_pkt.num += srcobj->tx.comp_pkt.num;
	tgtobj->stats.tx.ucast.num += srcobj->tx.ucast.num;
	tgtobj->stats.tx.ucast.bytes += srcobj->tx.ucast.bytes;
	tgtobj->stats.tx.mcast.num += srcobj->tx.mcast.num;
	tgtobj->stats.tx.mcast.bytes += srcobj->tx.mcast.bytes;
	tgtobj->stats.tx.bcast.num += srcobj->tx.bcast.num;
	tgtobj->stats.tx.bcast.bytes += srcobj->tx.bcast.bytes;
	tgtobj->stats.tx.tx_success.num += srcobj->tx.tx_success.num;
	tgtobj->stats.tx.tx_success.bytes +=
		srcobj->tx.tx_success.bytes;
	tgtobj->stats.tx.nawds_mcast.num +=
		srcobj->tx.nawds_mcast.num;
	tgtobj->stats.tx.nawds_mcast.bytes +=
		srcobj->tx.nawds_mcast.bytes;
	tgtobj->stats.tx.nawds_mcast_drop +=
		srcobj->tx.nawds_mcast_drop;
	tgtobj->stats.tx.num_ppdu_cookie_valid +=
		srcobj->tx.num_ppdu_cookie_valid;
	tgtobj->stats.tx.tx_failed += srcobj->tx.tx_failed;
	tgtobj->stats.tx.ofdma += srcobj->tx.ofdma;
	tgtobj->stats.tx.stbc += srcobj->tx.stbc;
	tgtobj->stats.tx.ldpc += srcobj->tx.ldpc;
	tgtobj->stats.tx.pream_punct_cnt += srcobj->tx.pream_punct_cnt;
	tgtobj->stats.tx.retries += srcobj->tx.retries;
	tgtobj->stats.tx.non_amsdu_cnt += srcobj->tx.non_amsdu_cnt;
	tgtobj->stats.tx.amsdu_cnt += srcobj->tx.amsdu_cnt;
	tgtobj->stats.tx.non_ampdu_cnt += srcobj->tx.non_ampdu_cnt;
	tgtobj->stats.tx.ampdu_cnt += srcobj->tx.ampdu_cnt;
	tgtobj->stats.tx.dropped.fw_rem.num += srcobj->tx.dropped.fw_rem.num;
	tgtobj->stats.tx.dropped.fw_rem.bytes +=
			srcobj->tx.dropped.fw_rem.bytes;
	tgtobj->stats.tx.dropped.fw_rem_tx +=
			srcobj->tx.dropped.fw_rem_tx;
	tgtobj->stats.tx.dropped.fw_rem_notx +=
			srcobj->tx.dropped.fw_rem_notx;
	tgtobj->stats.tx.dropped.fw_reason1 +=
			srcobj->tx.dropped.fw_reason1;
	tgtobj->stats.tx.dropped.fw_reason2 +=
			srcobj->tx.dropped.fw_reason2;
	tgtobj->stats.tx.dropped.fw_reason3 +=
			srcobj->tx.dropped.fw_reason3;
	tgtobj->stats.tx.dropped.age_out += srcobj->tx.dropped.age_out;
	tgtobj->stats.rx.err.mic_err += srcobj->rx.err.mic_err;
	if (srcobj->rx.snr != 0)
		tgtobj->stats.rx.snr = srcobj->rx.snr;
	tgtobj->stats.rx.rx_rate = srcobj->rx.rx_rate;
	tgtobj->stats.rx.err.decrypt_err += srcobj->rx.err.decrypt_err;
	tgtobj->stats.rx.non_ampdu_cnt += srcobj->rx.non_ampdu_cnt;
	tgtobj->stats.rx.amsdu_cnt += srcobj->rx.ampdu_cnt;
	tgtobj->stats.rx.non_amsdu_cnt += srcobj->rx.non_amsdu_cnt;
	tgtobj->stats.rx.amsdu_cnt += srcobj->rx.amsdu_cnt;
	tgtobj->stats.rx.nawds_mcast_drop += srcobj->rx.nawds_mcast_drop;
	tgtobj->stats.rx.to_stack.num += srcobj->rx.to_stack.num;
	tgtobj->stats.rx.to_stack.bytes += srcobj->rx.to_stack.bytes;

	for (i = 0; i <  CDP_MAX_RX_RINGS; i++) {
		tgtobj->stats.rx.rcvd_reo[i].num +=
			srcobj->rx.rcvd_reo[i].num;
		tgtobj->stats.rx.rcvd_reo[i].bytes +=
			srcobj->rx.rcvd_reo[i].bytes;
	}

	srcobj->rx.unicast.num =
		srcobj->rx.to_stack.num -
				(srcobj->rx.multicast.num);
	srcobj->rx.unicast.bytes =
		srcobj->rx.to_stack.bytes -
				(srcobj->rx.multicast.bytes);

	tgtobj->stats.rx.unicast.num += srcobj->rx.unicast.num;
	tgtobj->stats.rx.unicast.bytes += srcobj->rx.unicast.bytes;
	tgtobj->stats.rx.multicast.num += srcobj->rx.multicast.num;
	tgtobj->stats.rx.multicast.bytes += srcobj->rx.multicast.bytes;
	tgtobj->stats.rx.bcast.num += srcobj->rx.bcast.num;
	tgtobj->stats.rx.bcast.bytes += srcobj->rx.bcast.bytes;
	tgtobj->stats.rx.raw.num += srcobj->rx.raw.num;
	tgtobj->stats.rx.raw.bytes += srcobj->rx.raw.bytes;
	tgtobj->stats.rx.intra_bss.pkts.num +=
			srcobj->rx.intra_bss.pkts.num;
	tgtobj->stats.rx.intra_bss.pkts.bytes +=
			srcobj->rx.intra_bss.pkts.bytes;
	tgtobj->stats.rx.intra_bss.fail.num +=
			srcobj->rx.intra_bss.fail.num;
	tgtobj->stats.rx.intra_bss.fail.bytes +=
			srcobj->rx.intra_bss.fail.bytes;

	tgtobj->stats.tx.last_ack_rssi =
		srcobj->tx.last_ack_rssi;
	tgtobj->stats.rx.mec_drop.num += srcobj->rx.mec_drop.num;
	tgtobj->stats.rx.mec_drop.bytes += srcobj->rx.mec_drop.bytes;
	tgtobj->stats.rx.multipass_rx_pkt_drop +=
		srcobj->rx.multipass_rx_pkt_drop;
}

static inline void dp_update_pdev_ingress_stats(struct dp_pdev *tgtobj,
						struct dp_vdev *srcobj)
{
	DP_STATS_AGGR_PKT(tgtobj, srcobj, tx_i.nawds_mcast);

	DP_STATS_AGGR_PKT(tgtobj, srcobj, tx_i.rcvd);
	DP_STATS_AGGR_PKT(tgtobj, srcobj, tx_i.processed);
	DP_STATS_AGGR_PKT(tgtobj, srcobj, tx_i.reinject_pkts);
	DP_STATS_AGGR_PKT(tgtobj, srcobj, tx_i.inspect_pkts);
	DP_STATS_AGGR_PKT(tgtobj, srcobj, tx_i.raw.raw_pkt);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.raw.dma_map_error);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.raw.num_frags_overflow_err);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.sg.dropped_host.num);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.sg.dropped_target);
	DP_STATS_AGGR_PKT(tgtobj, srcobj, tx_i.sg.sg_pkt);
	DP_STATS_AGGR_PKT(tgtobj, srcobj, tx_i.mcast_en.mcast_pkt);
	DP_STATS_AGGR(tgtobj, srcobj,
		      tx_i.mcast_en.dropped_map_error);
	DP_STATS_AGGR(tgtobj, srcobj,
		      tx_i.mcast_en.dropped_self_mac);
	DP_STATS_AGGR(tgtobj, srcobj,
		      tx_i.mcast_en.dropped_send_fail);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.mcast_en.ucast);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.igmp_mcast_en.igmp_rcvd);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.igmp_mcast_en.igmp_ucast_converted);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.dropped.dma_error);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.dropped.ring_full);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.dropped.enqueue_fail);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.dropped.fail_per_pkt_vdev_id_check);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.dropped.desc_na.num);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.dropped.res_full);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.dropped.headroom_insufficient);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.cce_classified);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.cce_classified_raw);
	DP_STATS_AGGR_PKT(tgtobj, srcobj, tx_i.sniffer_rcvd);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.mesh.exception_fw);
	DP_STATS_AGGR(tgtobj, srcobj, tx_i.mesh.completion_fw);

	tgtobj->stats.tx_i.dropped.dropped_pkt.num =
		tgtobj->stats.tx_i.dropped.dma_error +
		tgtobj->stats.tx_i.dropped.ring_full +
		tgtobj->stats.tx_i.dropped.enqueue_fail +
		tgtobj->stats.tx_i.dropped.fail_per_pkt_vdev_id_check +
		tgtobj->stats.tx_i.dropped.desc_na.num +
		tgtobj->stats.tx_i.dropped.res_full;

}

/**
 * dp_is_wds_extended(): Check if wds ext is enabled
 * @vdev: DP VDEV handle
 *
 * return: true if enabled, false if not
 */
#ifdef QCA_SUPPORT_WDS_EXTENDED
static bool dp_is_wds_extended(struct dp_peer *peer)
{
	if (qdf_atomic_test_bit(WDS_EXT_PEER_INIT_BIT,
				&peer->wds_ext.init))
		return true;

	return false;
}
#else
static bool dp_is_wds_extended(struct dp_peer *peer)
{
	return false;
}
#endif /* QCA_SUPPORT_WDS_EXTENDED */

static inline void dp_update_vdev_stats(struct dp_soc *soc,
					struct dp_peer *srcobj,
					void *arg)
{
	struct cdp_vdev_stats *tgtobj = (struct cdp_vdev_stats *)arg;
	uint8_t i;
	uint8_t pream_type;

	if (qdf_unlikely(dp_is_wds_extended(srcobj)))
		return;

	for (pream_type = 0; pream_type < DOT11_MAX; pream_type++) {
		for (i = 0; i < MAX_MCS; i++) {
			tgtobj->tx.pkt_type[pream_type].
				mcs_count[i] +=
			srcobj->stats.tx.pkt_type[pream_type].
				mcs_count[i];
			tgtobj->rx.pkt_type[pream_type].
				mcs_count[i] +=
			srcobj->stats.rx.pkt_type[pream_type].
				mcs_count[i];
		}
	}

	for (i = 0; i < MAX_BW; i++) {
		tgtobj->tx.bw[i] += srcobj->stats.tx.bw[i];
		tgtobj->rx.bw[i] += srcobj->stats.rx.bw[i];
	}

	for (i = 0; i < SS_COUNT; i++) {
		tgtobj->tx.nss[i] += srcobj->stats.tx.nss[i];
		tgtobj->rx.nss[i] += srcobj->stats.rx.nss[i];
	}

	for (i = 0; i < WME_AC_MAX; i++) {
		tgtobj->tx.wme_ac_type[i] +=
			srcobj->stats.tx.wme_ac_type[i];
		tgtobj->rx.wme_ac_type[i] +=
			srcobj->stats.rx.wme_ac_type[i];
		tgtobj->tx.excess_retries_per_ac[i] +=
			srcobj->stats.tx.excess_retries_per_ac[i];
	}

	for (i = 0; i < MAX_GI; i++) {
		tgtobj->tx.sgi_count[i] +=
			srcobj->stats.tx.sgi_count[i];
		tgtobj->rx.sgi_count[i] +=
			srcobj->stats.rx.sgi_count[i];
	}

	for (i = 0; i < MAX_RECEPTION_TYPES; i++)
		tgtobj->rx.reception_type[i] +=
			srcobj->stats.rx.reception_type[i];

	tgtobj->tx.comp_pkt.bytes += srcobj->stats.tx.comp_pkt.bytes;
	tgtobj->tx.comp_pkt.num += srcobj->stats.tx.comp_pkt.num;
	tgtobj->tx.ucast.num += srcobj->stats.tx.ucast.num;
	tgtobj->tx.ucast.bytes += srcobj->stats.tx.ucast.bytes;
	tgtobj->tx.mcast.num += srcobj->stats.tx.mcast.num;
	tgtobj->tx.mcast.bytes += srcobj->stats.tx.mcast.bytes;
	tgtobj->tx.bcast.num += srcobj->stats.tx.bcast.num;
	tgtobj->tx.bcast.bytes += srcobj->stats.tx.bcast.bytes;
	tgtobj->tx.tx_success.num += srcobj->stats.tx.tx_success.num;
	tgtobj->tx.tx_success.bytes +=
		srcobj->stats.tx.tx_success.bytes;
	tgtobj->tx.nawds_mcast.num +=
		srcobj->stats.tx.nawds_mcast.num;
	tgtobj->tx.nawds_mcast.bytes +=
		srcobj->stats.tx.nawds_mcast.bytes;
	tgtobj->tx.nawds_mcast_drop +=
		srcobj->stats.tx.nawds_mcast_drop;
	tgtobj->tx.num_ppdu_cookie_valid +=
		srcobj->stats.tx.num_ppdu_cookie_valid;
	tgtobj->tx.tx_failed += srcobj->stats.tx.tx_failed;
	tgtobj->tx.ofdma += srcobj->stats.tx.ofdma;
	tgtobj->tx.stbc += srcobj->stats.tx.stbc;
	tgtobj->tx.ldpc += srcobj->stats.tx.ldpc;
	tgtobj->tx.pream_punct_cnt += srcobj->stats.tx.pream_punct_cnt;
	tgtobj->tx.retries += srcobj->stats.tx.retries;
	tgtobj->tx.non_amsdu_cnt += srcobj->stats.tx.non_amsdu_cnt;
	tgtobj->tx.amsdu_cnt += srcobj->stats.tx.amsdu_cnt;
	tgtobj->tx.non_ampdu_cnt += srcobj->stats.tx.non_ampdu_cnt;
	tgtobj->tx.ampdu_cnt += srcobj->stats.tx.ampdu_cnt;
	tgtobj->tx.dropped.fw_rem.num += srcobj->stats.tx.dropped.fw_rem.num;
	tgtobj->tx.dropped.fw_rem.bytes +=
			srcobj->stats.tx.dropped.fw_rem.bytes;
	tgtobj->tx.dropped.fw_rem_tx +=
			srcobj->stats.tx.dropped.fw_rem_tx;
	tgtobj->tx.dropped.fw_rem_notx +=
			srcobj->stats.tx.dropped.fw_rem_notx;
	tgtobj->tx.dropped.fw_reason1 +=
			srcobj->stats.tx.dropped.fw_reason1;
	tgtobj->tx.dropped.fw_reason2 +=
			srcobj->stats.tx.dropped.fw_reason2;
	tgtobj->tx.dropped.fw_reason3 +=
			srcobj->stats.tx.dropped.fw_reason3;
	tgtobj->tx.dropped.age_out += srcobj->stats.tx.dropped.age_out;
	tgtobj->tx.mpdu_success_with_retries +=
			srcobj->stats.tx.mpdu_success_with_retries;
	tgtobj->rx.err.mic_err += srcobj->stats.rx.err.mic_err;
	if (srcobj->stats.rx.snr != 0)
		tgtobj->rx.snr = srcobj->stats.rx.snr;
	tgtobj->rx.rx_rate = srcobj->stats.rx.rx_rate;
	tgtobj->rx.err.decrypt_err += srcobj->stats.rx.err.decrypt_err;
	tgtobj->rx.non_ampdu_cnt += srcobj->stats.rx.non_ampdu_cnt;
	tgtobj->rx.amsdu_cnt += srcobj->stats.rx.ampdu_cnt;
	tgtobj->rx.non_amsdu_cnt += srcobj->stats.rx.non_amsdu_cnt;
	tgtobj->rx.amsdu_cnt += srcobj->stats.rx.amsdu_cnt;
	tgtobj->rx.nawds_mcast_drop += srcobj->stats.rx.nawds_mcast_drop;
	tgtobj->rx.to_stack.num += srcobj->stats.rx.to_stack.num;
	tgtobj->rx.to_stack.bytes += srcobj->stats.rx.to_stack.bytes;

	for (i = 0; i <  CDP_MAX_RX_RINGS; i++) {
		tgtobj->rx.rcvd_reo[i].num +=
			srcobj->stats.rx.rcvd_reo[i].num;
		tgtobj->rx.rcvd_reo[i].bytes +=
			srcobj->stats.rx.rcvd_reo[i].bytes;
	}

	srcobj->stats.rx.unicast.num =
		srcobj->stats.rx.to_stack.num -
				srcobj->stats.rx.multicast.num;
	srcobj->stats.rx.unicast.bytes =
		srcobj->stats.rx.to_stack.bytes -
				srcobj->stats.rx.multicast.bytes;

	tgtobj->rx.unicast.num += srcobj->stats.rx.unicast.num;
	tgtobj->rx.unicast.bytes += srcobj->stats.rx.unicast.bytes;
	tgtobj->rx.multicast.num += srcobj->stats.rx.multicast.num;
	tgtobj->rx.multicast.bytes += srcobj->stats.rx.multicast.bytes;
	tgtobj->rx.bcast.num += srcobj->stats.rx.bcast.num;
	tgtobj->rx.bcast.bytes += srcobj->stats.rx.bcast.bytes;
	tgtobj->rx.raw.num += srcobj->stats.rx.raw.num;
	tgtobj->rx.raw.bytes += srcobj->stats.rx.raw.bytes;
	tgtobj->rx.intra_bss.pkts.num +=
			srcobj->stats.rx.intra_bss.pkts.num;
	tgtobj->rx.intra_bss.pkts.bytes +=
			srcobj->stats.rx.intra_bss.pkts.bytes;
	tgtobj->rx.intra_bss.fail.num +=
			srcobj->stats.rx.intra_bss.fail.num;
	tgtobj->rx.intra_bss.fail.bytes +=
			srcobj->stats.rx.intra_bss.fail.bytes;
	tgtobj->tx.last_ack_rssi =
		srcobj->stats.tx.last_ack_rssi;
	tgtobj->rx.mec_drop.num += srcobj->stats.rx.mec_drop.num;
	tgtobj->rx.mec_drop.bytes += srcobj->stats.rx.mec_drop.bytes;
	tgtobj->rx.multipass_rx_pkt_drop +=
		srcobj->stats.rx.multipass_rx_pkt_drop;
}

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
		for (i = 0; i < SS_COUNT; i++) { \
			DP_STATS_AGGR(_tgtobj, _srcobj, rx.nss[i]); \
			DP_STATS_AGGR(_tgtobj, _srcobj, tx.nss[i]); \
		} \
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
		for (i = 0; i < MAX_RECEPTION_TYPES; i++) \
			DP_STATS_AGGR(_tgtobj, _srcobj, rx.reception_type[i]); \
		\
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.comp_pkt); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.ucast); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.mcast); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.bcast); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.tx_success); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.nawds_mcast); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.nawds_mcast_drop); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.tx_failed); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.ofdma); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.stbc); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.ldpc); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.retries); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.non_amsdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.amsdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.non_ampdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.ampdu_cnt); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, tx.dropped.fw_rem); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.fw_rem_tx); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.fw_rem_notx); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.fw_reason1); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.fw_reason2); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.fw_reason3); \
		DP_STATS_AGGR(_tgtobj, _srcobj, tx.dropped.age_out); \
								\
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.err.mic_err); \
		if (_srcobj->stats.rx.snr != 0) \
			DP_STATS_UPD_STRUCT(_tgtobj, _srcobj, rx.snr); \
		DP_STATS_UPD_STRUCT(_tgtobj, _srcobj, rx.rx_rate); \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.err.decrypt_err); \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.non_ampdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.ampdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.non_amsdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.amsdu_cnt); \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.nawds_mcast_drop); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.to_stack); \
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
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.bcast); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.raw); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.intra_bss.pkts); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.intra_bss.fail); \
		DP_STATS_AGGR_PKT(_tgtobj, _srcobj, rx.mec_drop); \
								  \
		_tgtobj->stats.tx.last_ack_rssi =	\
			_srcobj->stats.tx.last_ack_rssi; \
		DP_STATS_AGGR(_tgtobj, _srcobj, rx.multipass_rx_pkt_drop); \
	}  while (0)

/**
 * dp_peer_find_attach() - Allocates memory for peer objects
 * @soc: SoC handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_peer_find_attach(struct dp_soc *soc);
extern void dp_peer_find_detach(struct dp_soc *soc);
extern void dp_peer_find_hash_add(struct dp_soc *soc, struct dp_peer *peer);
extern void dp_peer_find_hash_remove(struct dp_soc *soc, struct dp_peer *peer);
extern void dp_peer_find_hash_erase(struct dp_soc *soc);
void dp_peer_vdev_list_add(struct dp_soc *soc, struct dp_vdev *vdev,
			   struct dp_peer *peer);
void dp_peer_vdev_list_remove(struct dp_soc *soc, struct dp_vdev *vdev,
			      struct dp_peer *peer);
void dp_peer_find_id_to_obj_add(struct dp_soc *soc,
				struct dp_peer *peer,
				uint16_t peer_id);
void dp_peer_find_id_to_obj_remove(struct dp_soc *soc,
				   uint16_t peer_id);
void dp_vdev_unref_delete(struct dp_soc *soc, struct dp_vdev *vdev,
			  enum dp_mod_id mod_id);

/*
 * dp_peer_ppdu_delayed_ba_cleanup() free ppdu allocated in peer
 * @peer: Datapath peer
 *
 * return: void
 */
void dp_peer_ppdu_delayed_ba_cleanup(struct dp_peer *peer);

extern void dp_peer_rx_init(struct dp_pdev *pdev, struct dp_peer *peer);
void dp_peer_cleanup(struct dp_vdev *vdev, struct dp_peer *peer);
void dp_peer_rx_cleanup(struct dp_vdev *vdev, struct dp_peer *peer);

#ifdef DP_PEER_EXTENDED_API
/**
 * dp_register_peer() - Register peer into physical device
 * @soc_hdl - data path soc handle
 * @pdev_id - device instance id
 * @sta_desc - peer description
 *
 * Register peer into physical device
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_FAULT peer not found
 */
QDF_STATUS dp_register_peer(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			    struct ol_txrx_desc_type *sta_desc);

/**
 * dp_clear_peer() - remove peer from physical device
 * @soc_hdl - data path soc handle
 * @pdev_id - device instance id
 * @peer_addr - peer mac address
 *
 * remove peer from physical device
 *
 * Return: QDF_STATUS_SUCCESS registration success
 *         QDF_STATUS_E_FAULT peer not found
 */
QDF_STATUS dp_clear_peer(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			 struct qdf_mac_addr peer_addr);

/*
 * dp_find_peer_exist - find peer if already exists
 * @soc: datapath soc handle
 * @pdev_id: physical device instance id
 * @peer_mac_addr: peer mac address
 *
 * Return: true or false
 */
bool dp_find_peer_exist(struct cdp_soc_t *soc_hdl, uint8_t pdev_id,
			uint8_t *peer_addr);

/*
 * dp_find_peer_exist_on_vdev - find if peer exists on the given vdev
 * @soc: datapath soc handle
 * @vdev_id: vdev instance id
 * @peer_mac_addr: peer mac address
 *
 * Return: true or false
 */
bool dp_find_peer_exist_on_vdev(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
				uint8_t *peer_addr);

/*
 * dp_find_peer_exist_on_other_vdev - find if peer exists
 * on other than the given vdev
 * @soc: datapath soc handle
 * @vdev_id: vdev instance id
 * @peer_mac_addr: peer mac address
 * @max_bssid: max number of bssids
 *
 * Return: true or false
 */
bool dp_find_peer_exist_on_other_vdev(struct cdp_soc_t *soc_hdl,
				      uint8_t vdev_id, uint8_t *peer_addr,
				      uint16_t max_bssid);

/**
 * dp_peer_state_update() - update peer local state
 * @pdev - data path device instance
 * @peer_addr - peer mac address
 * @state - new peer local state
 *
 * update peer local state
 *
 * Return: QDF_STATUS_SUCCESS registration success
 */
QDF_STATUS dp_peer_state_update(struct cdp_soc_t *soc, uint8_t *peer_mac,
				enum ol_txrx_peer_state state);

/**
 * dp_get_vdevid() - Get virtual interface id which peer registered
 * @soc - datapath soc handle
 * @peer_mac - peer mac address
 * @vdev_id - virtual interface id which peer registered
 *
 * Get virtual interface id which peer registered
 *
 * Return: QDF_STATUS_SUCCESS registration success
 */
QDF_STATUS dp_get_vdevid(struct cdp_soc_t *soc_hdl, uint8_t *peer_mac,
			 uint8_t *vdev_id);
struct cdp_vdev *dp_get_vdev_by_peer_addr(struct cdp_pdev *pdev_handle,
		struct qdf_mac_addr peer_addr);
struct cdp_vdev *dp_get_vdev_for_peer(void *peer);
uint8_t *dp_peer_get_peer_mac_addr(void *peer);

/**
 * dp_get_peer_state() - Get local peer state
 * @soc - datapath soc handle
 * @vdev_id - vdev id
 * @peer_mac - peer mac addr
 *
 * Get local peer state
 *
 * Return: peer status
 */
int dp_get_peer_state(struct cdp_soc_t *soc, uint8_t vdev_id,
		      uint8_t *peer_mac);
void dp_local_peer_id_pool_init(struct dp_pdev *pdev);
void dp_local_peer_id_alloc(struct dp_pdev *pdev, struct dp_peer *peer);
void dp_local_peer_id_free(struct dp_pdev *pdev, struct dp_peer *peer);
#else
/**
 * dp_get_vdevid() - Get virtual interface id which peer registered
 * @soc - datapath soc handle
 * @peer_mac - peer mac address
 * @vdev_id - virtual interface id which peer registered
 *
 * Get virtual interface id which peer registered
 *
 * Return: QDF_STATUS_SUCCESS registration success
 */
static inline
QDF_STATUS dp_get_vdevid(struct cdp_soc_t *soc_hdl, uint8_t *peer_mac,
			 uint8_t *vdev_id)
{
	return QDF_STATUS_E_NOSUPPORT;
}

static inline void dp_local_peer_id_pool_init(struct dp_pdev *pdev)
{
}

static inline
void dp_local_peer_id_alloc(struct dp_pdev *pdev, struct dp_peer *peer)
{
}

static inline
void dp_local_peer_id_free(struct dp_pdev *pdev, struct dp_peer *peer)
{
}
#endif
int dp_addba_resp_tx_completion_wifi3(struct cdp_soc_t *cdp_soc,
				      uint8_t *peer_mac, uint16_t vdev_id,
				      uint8_t tid,
				      int status);
int dp_addba_requestprocess_wifi3(struct cdp_soc_t *cdp_soc,
				  uint8_t *peer_mac, uint16_t vdev_id,
				  uint8_t dialogtoken, uint16_t tid,
				  uint16_t batimeout,
				  uint16_t buffersize,
				  uint16_t startseqnum);
QDF_STATUS dp_addba_responsesetup_wifi3(struct cdp_soc_t *cdp_soc,
					uint8_t *peer_mac, uint16_t vdev_id,
					uint8_t tid, uint8_t *dialogtoken,
					uint16_t *statuscode,
					uint16_t *buffersize,
					uint16_t *batimeout);
QDF_STATUS dp_set_addba_response(struct cdp_soc_t *cdp_soc,
				 uint8_t *peer_mac,
				 uint16_t vdev_id, uint8_t tid,
				 uint16_t statuscode);
int dp_delba_process_wifi3(struct cdp_soc_t *cdp_soc, uint8_t *peer_mac,
			   uint16_t vdev_id, int tid,
			   uint16_t reasoncode);
/*
 * dp_delba_tx_completion_wifi3() -  Handle delba tx completion
 *
 * @cdp_soc: soc handle
 * @vdev_id: id of the vdev handle
 * @peer_mac: peer mac address
 * @tid: Tid number
 * @status: Tx completion status
 * Indicate status of delba Tx to DP for stats update and retry
 * delba if tx failed.
 *
 */
int dp_delba_tx_completion_wifi3(struct cdp_soc_t *cdp_soc, uint8_t *peer_mac,
				 uint16_t vdev_id, uint8_t tid,
				 int status);
extern QDF_STATUS dp_rx_tid_setup_wifi3(struct dp_peer *peer, int tid,
					uint32_t ba_window_size,
					uint32_t start_seq);

extern QDF_STATUS dp_reo_send_cmd(struct dp_soc *soc,
	enum hal_reo_cmd_type type, struct hal_reo_cmd_params *params,
	void (*callback_fn), void *data);

extern void dp_reo_cmdlist_destroy(struct dp_soc *soc);

/**
 * dp_reo_status_ring_handler - Handler for REO Status ring
 * @int_ctx: pointer to DP interrupt context
 * @soc: DP Soc handle
 *
 * Returns: Number of descriptors reaped
 */
uint32_t dp_reo_status_ring_handler(struct dp_intr *int_ctx,
				    struct dp_soc *soc);
void dp_aggregate_vdev_stats(struct dp_vdev *vdev,
			     struct cdp_vdev_stats *vdev_stats);
void dp_rx_tid_stats_cb(struct dp_soc *soc, void *cb_ctxt,
	union hal_reo_status *reo_status);
void dp_rx_bar_stats_cb(struct dp_soc *soc, void *cb_ctxt,
		union hal_reo_status *reo_status);
uint16_t dp_tx_me_send_convert_ucast(struct cdp_soc_t *soc, uint8_t vdev_id,
				     qdf_nbuf_t nbuf,
				     uint8_t newmac[][QDF_MAC_ADDR_SIZE],
				     uint8_t new_mac_cnt, uint8_t tid,
				     bool is_igmp);
void dp_tx_me_alloc_descriptor(struct cdp_soc_t *soc, uint8_t pdev_id);

void dp_tx_me_free_descriptor(struct cdp_soc_t *soc, uint8_t pdev_id);
QDF_STATUS dp_h2t_ext_stats_msg_send(struct dp_pdev *pdev,
		uint32_t stats_type_upload_mask, uint32_t config_param_0,
		uint32_t config_param_1, uint32_t config_param_2,
		uint32_t config_param_3, int cookie, int cookie_msb,
		uint8_t mac_id);
void dp_htt_stats_print_tag(struct dp_pdev *pdev,
			    uint8_t tag_type, uint32_t *tag_buf);
void dp_htt_stats_copy_tag(struct dp_pdev *pdev, uint8_t tag_type, uint32_t *tag_buf);
QDF_STATUS dp_h2t_3tuple_config_send(struct dp_pdev *pdev, uint32_t tuple_mask,
				     uint8_t mac_id);
/**
 * dp_rxtid_stats_cmd_cb - function pointer for peer
 *			   rx tid stats cmd call_back
 */
typedef void (*dp_rxtid_stats_cmd_cb)(struct dp_soc *soc, void *cb_ctxt,
				      union hal_reo_status *reo_status);
int dp_peer_rxtid_stats(struct dp_peer *peer,
			dp_rxtid_stats_cmd_cb dp_stats_cmd_cb,
			void *cb_ctxt);
QDF_STATUS
dp_set_pn_check_wifi3(struct cdp_soc_t *soc, uint8_t vdev_id,
		      uint8_t *peer_mac, enum cdp_sec_type sec_type,
		      uint32_t *rx_pn);

QDF_STATUS
dp_set_key_sec_type_wifi3(struct cdp_soc_t *soc, uint8_t vdev_id,
			  uint8_t *peer_mac, enum cdp_sec_type sec_type,
			  bool is_unicast);

void *dp_get_pdev_for_mac_id(struct dp_soc *soc, uint32_t mac_id);

QDF_STATUS
dp_set_michael_key(struct cdp_soc_t *soc, uint8_t vdev_id,
		   uint8_t *peer_mac,
		   bool is_unicast, uint32_t *key);

/**
 * dp_check_pdev_exists() - Validate pdev before use
 * @soc - dp soc handle
 * @data - pdev handle
 *
 * Return: 0 - success/invalid - failure
 */
bool dp_check_pdev_exists(struct dp_soc *soc, struct dp_pdev *data);

/**
 * dp_update_delay_stats() - Update delay statistics in structure
 *                              and fill min, max and avg delay
 * @pdev: pdev handle
 * @delay: delay in ms
 * @tid: tid value
 * @mode: type of tx delay mode
 * @ring id: ring number
 *
 * Return: none
 */
void dp_update_delay_stats(struct dp_pdev *pdev, uint32_t delay,
			   uint8_t tid, uint8_t mode, uint8_t ring_id);

/**
 * dp_print_ring_stats(): Print tail and head pointer
 * @pdev: DP_PDEV handle
 *
 * Return:void
 */
void dp_print_ring_stats(struct dp_pdev *pdev);

/**
 * dp_print_pdev_cfg_params() - Print the pdev cfg parameters
 * @pdev_handle: DP pdev handle
 *
 * Return - void
 */
void dp_print_pdev_cfg_params(struct dp_pdev *pdev);

/**
 * dp_print_soc_cfg_params()- Dump soc wlan config parameters
 * @soc_handle: Soc handle
 *
 * Return: void
 */
void dp_print_soc_cfg_params(struct dp_soc *soc);

/**
 * dp_srng_get_str_from_ring_type() - Return string name for a ring
 * @ring_type: Ring
 *
 * Return: char const pointer
 */
const
char *dp_srng_get_str_from_hal_ring_type(enum hal_ring_type ring_type);

/*
 * dp_txrx_path_stats() - Function to display dump stats
 * @soc - soc handle
 *
 * return: none
 */
void dp_txrx_path_stats(struct dp_soc *soc);

/*
 * dp_print_per_ring_stats(): Packet count per ring
 * @soc - soc handle
 *
 * Return - None
 */
void dp_print_per_ring_stats(struct dp_soc *soc);

/**
 * dp_aggregate_pdev_stats(): Consolidate stats at PDEV level
 * @pdev: DP PDEV handle
 *
 * return: void
 */
void dp_aggregate_pdev_stats(struct dp_pdev *pdev);

/**
 * dp_print_rx_rates(): Print Rx rate stats
 * @vdev: DP_VDEV handle
 *
 * Return:void
 */
void dp_print_rx_rates(struct dp_vdev *vdev);

/**
 * dp_print_tx_rates(): Print tx rates
 * @vdev: DP_VDEV handle
 *
 * Return:void
 */
void dp_print_tx_rates(struct dp_vdev *vdev);

/**
 * dp_print_peer_stats():print peer stats
 * @peer: DP_PEER handle
 *
 * return void
 */
void dp_print_peer_stats(struct dp_peer *peer);

/**
 * dp_print_pdev_tx_stats(): Print Pdev level TX stats
 * @pdev: DP_PDEV Handle
 *
 * Return:void
 */
void
dp_print_pdev_tx_stats(struct dp_pdev *pdev);

/**
 * dp_print_pdev_rx_stats(): Print Pdev level RX stats
 * @pdev: DP_PDEV Handle
 *
 * Return: void
 */
void
dp_print_pdev_rx_stats(struct dp_pdev *pdev);

/**
 * dp_print_soc_tx_stats(): Print SOC level  stats
 * @soc DP_SOC Handle
 *
 * Return: void
 */
void dp_print_soc_tx_stats(struct dp_soc *soc);

/**
 * dp_print_soc_interrupt_stats() - Print interrupt stats for the soc
 * @soc: dp_soc handle
 *
 * Return: None
 */
void dp_print_soc_interrupt_stats(struct dp_soc *soc);

/**
 * dp_print_soc_rx_stats: Print SOC level Rx stats
 * @soc: DP_SOC Handle
 *
 * Return:void
 */
void dp_print_soc_rx_stats(struct dp_soc *soc);

/**
 * dp_get_mac_id_for_pdev() -  Return mac corresponding to pdev for mac
 *
 * @mac_id: MAC id
 * @pdev_id: pdev_id corresponding to pdev, 0 for MCL
 *
 * Single pdev using both MACs will operate on both MAC rings,
 * which is the case for MCL.
 * For WIN each PDEV will operate one ring, so index is zero.
 *
 */
static inline int dp_get_mac_id_for_pdev(uint32_t mac_id, uint32_t pdev_id)
{
	if (mac_id && pdev_id) {
		qdf_print("Both mac_id and pdev_id cannot be non zero");
		QDF_BUG(0);
		return 0;
	}
	return (mac_id + pdev_id);
}

/**
 * dp_get_lmac_id_for_pdev_id() -  Return lmac id corresponding to host pdev id
 * @soc: soc pointer
 * @mac_id: MAC id
 * @pdev_id: pdev_id corresponding to pdev, 0 for MCL
 *
 * For MCL, Single pdev using both MACs will operate on both MAC rings.
 *
 * For WIN, each PDEV will operate one ring.
 *
 */
static inline int
dp_get_lmac_id_for_pdev_id
	(struct dp_soc *soc, uint32_t mac_id, uint32_t pdev_id)
{
	if (!wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx)) {
		if (mac_id && pdev_id) {
			qdf_print("Both mac_id and pdev_id cannot be non zero");
			QDF_BUG(0);
			return 0;
		}
		return (mac_id + pdev_id);
	}

	return soc->pdev_list[pdev_id]->lmac_id;
}

/**
 * dp_get_pdev_for_lmac_id() -  Return pdev pointer corresponding to lmac id
 * @soc: soc pointer
 * @lmac_id: LMAC id
 *
 * For MCL, Single pdev exists
 *
 * For WIN, each PDEV will operate one ring.
 *
 */
static inline struct dp_pdev *
	dp_get_pdev_for_lmac_id(struct dp_soc *soc, uint32_t lmac_id)
{
	uint8_t i = 0;

	if (wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx)) {
		i = wlan_cfg_get_pdev_idx(soc->wlan_cfg_ctx, lmac_id);
		return ((i < MAX_PDEV_CNT) ? soc->pdev_list[i] : NULL);
	}

	/* Typically for MCL as there only 1 PDEV*/
	return soc->pdev_list[0];
}

/**
 * dp_calculate_target_pdev_id_from_host_pdev_id() - Return target pdev
 *                                          corresponding to host pdev id
 * @soc: soc pointer
 * @mac_for_pdev: pdev_id corresponding to host pdev for WIN, mac id for MCL
 *
 * returns target pdev_id for host pdev id. For WIN, this is derived through
 * a two step process:
 * 1. Get lmac_id corresponding to host pdev_id (lmac_id can change
 *    during mode switch)
 * 2. Get target pdev_id (set up during WMI ready) from lmac_id
 *
 * For MCL, return the offset-1 translated mac_id
 */
static inline int
dp_calculate_target_pdev_id_from_host_pdev_id
	(struct dp_soc *soc, uint32_t mac_for_pdev)
{
	struct dp_pdev *pdev;

	if (!wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx))
		return DP_SW2HW_MACID(mac_for_pdev);

	pdev = soc->pdev_list[mac_for_pdev];

	/*non-MCL case, get original target_pdev mapping*/
	return wlan_cfg_get_target_pdev_id(soc->wlan_cfg_ctx, pdev->lmac_id);
}

/**
 * dp_get_target_pdev_id_for_host_pdev_id() - Return target pdev corresponding
 *                                         to host pdev id
 * @soc: soc pointer
 * @mac_for_pdev: pdev_id corresponding to host pdev for WIN, mac id for MCL
 *
 * returns target pdev_id for host pdev id.
 * For WIN, return the value stored in pdev object.
 * For MCL, return the offset-1 translated mac_id.
 */
static inline int
dp_get_target_pdev_id_for_host_pdev_id
	(struct dp_soc *soc, uint32_t mac_for_pdev)
{
	struct dp_pdev *pdev;

	if (!wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx))
		return DP_SW2HW_MACID(mac_for_pdev);

	pdev = soc->pdev_list[mac_for_pdev];

	return pdev->target_pdev_id;
}

/**
 * dp_get_host_pdev_id_for_target_pdev_id() - Return host pdev corresponding
 *                                         to target pdev id
 * @soc: soc pointer
 * @pdev_id: pdev_id corresponding to target pdev
 *
 * returns host pdev_id for target pdev id. For WIN, this is derived through
 * a two step process:
 * 1. Get lmac_id corresponding to target pdev_id
 * 2. Get host pdev_id (set up during WMI ready) from lmac_id
 *
 * For MCL, return the 0-offset pdev_id
 */
static inline int
dp_get_host_pdev_id_for_target_pdev_id
	(struct dp_soc *soc, uint32_t pdev_id)
{
	struct dp_pdev *pdev;
	int lmac_id;

	if (!wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx))
		return DP_HW2SW_MACID(pdev_id);

	/*non-MCL case, get original target_lmac mapping from target pdev*/
	lmac_id = wlan_cfg_get_hw_mac_idx(soc->wlan_cfg_ctx,
					  DP_HW2SW_MACID(pdev_id));

	/*Get host pdev from lmac*/
	pdev = dp_get_pdev_for_lmac_id(soc, lmac_id);

	return pdev ? pdev->pdev_id : INVALID_PDEV_ID;
}

/*
 * dp_get_mac_id_for_mac() -  Return mac corresponding WIN and MCL mac_ids
 *
 * @soc: handle to DP soc
 * @mac_id: MAC id
 *
 * Single pdev using both MACs will operate on both MAC rings,
 * which is the case for MCL.
 * For WIN each PDEV will operate one ring, so index is zero.
 *
 */
static inline int dp_get_mac_id_for_mac(struct dp_soc *soc, uint32_t mac_id)
{
	/*
	 * Single pdev using both MACs will operate on both MAC rings,
	 * which is the case for MCL.
	 */
	if (!wlan_cfg_per_pdev_lmac_ring(soc->wlan_cfg_ctx))
		return mac_id;

	/* For WIN each PDEV will operate one ring, so index is zero. */
	return 0;
}

/*
 * dp_is_subtype_data() - check if the frame subtype is data
 *
 * @frame_ctrl: Frame control field
 *
 * check the frame control field and verify if the packet
 * is a data packet.
 *
 * Return: true or false
 */
static inline bool dp_is_subtype_data(uint16_t frame_ctrl)
{
	if (((qdf_cpu_to_le16(frame_ctrl) & QDF_IEEE80211_FC0_TYPE_MASK) ==
	    QDF_IEEE80211_FC0_TYPE_DATA) &&
	    (((qdf_cpu_to_le16(frame_ctrl) & QDF_IEEE80211_FC0_SUBTYPE_MASK) ==
	    QDF_IEEE80211_FC0_SUBTYPE_DATA) ||
	    ((qdf_cpu_to_le16(frame_ctrl) & QDF_IEEE80211_FC0_SUBTYPE_MASK) ==
	    QDF_IEEE80211_FC0_SUBTYPE_QOS))) {
		return true;
	}

	return false;
}

#ifdef WDI_EVENT_ENABLE
QDF_STATUS dp_h2t_cfg_stats_msg_send(struct dp_pdev *pdev,
				uint32_t stats_type_upload_mask,
				uint8_t mac_id);

int dp_wdi_event_unsub(struct cdp_soc_t *soc, uint8_t pdev_id,
		       wdi_event_subscribe *event_cb_sub_handle,
		       uint32_t event);

int dp_wdi_event_sub(struct cdp_soc_t *soc, uint8_t pdev_id,
		     wdi_event_subscribe *event_cb_sub_handle,
		     uint32_t event);

void dp_wdi_event_handler(enum WDI_EVENT event, struct dp_soc *soc,
			  void *data, u_int16_t peer_id,
			  int status, u_int8_t pdev_id);

int dp_wdi_event_attach(struct dp_pdev *txrx_pdev);
int dp_wdi_event_detach(struct dp_pdev *txrx_pdev);

static inline void
dp_hif_update_pipe_callback(struct dp_soc *dp_soc,
			    void *cb_context,
			    QDF_STATUS (*callback)(void *, qdf_nbuf_t, uint8_t),
			    uint8_t pipe_id)
{
	struct hif_msg_callbacks hif_pipe_callbacks;

	/* TODO: Temporary change to bypass HTC connection for this new
	 * HIF pipe, which will be used for packet log and other high-
	 * priority HTT messages. Proper HTC connection to be added
	 * later once required FW changes are available
	 */
	hif_pipe_callbacks.rxCompletionHandler = callback;
	hif_pipe_callbacks.Context = cb_context;
	hif_update_pipe_callback(dp_soc->hif_handle,
		DP_HTT_T2H_HP_PIPE, &hif_pipe_callbacks);
}
#else
static inline int dp_wdi_event_unsub(struct cdp_soc_t *soc, uint8_t pdev_id,
				     wdi_event_subscribe *event_cb_sub_handle,
				     uint32_t event)
{
	return 0;
}

static inline int dp_wdi_event_sub(struct cdp_soc_t *soc, uint8_t pdev_id,
				   wdi_event_subscribe *event_cb_sub_handle,
				   uint32_t event)
{
	return 0;
}

static inline
void dp_wdi_event_handler(enum WDI_EVENT event,
			  struct dp_soc *soc,
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

static inline QDF_STATUS dp_h2t_cfg_stats_msg_send(struct dp_pdev *pdev,
		uint32_t stats_type_upload_mask, uint8_t mac_id)
{
	return 0;
}

static inline void
dp_hif_update_pipe_callback(struct dp_soc *dp_soc, void *cb_context,
			    QDF_STATUS (*callback)(void *, qdf_nbuf_t, uint8_t),
			    uint8_t pipe_id)
{
}
#endif /* CONFIG_WIN */

#ifdef VDEV_PEER_PROTOCOL_COUNT
/**
 * dp_vdev_peer_stats_update_protocol_cnt() - update per-peer protocol counters
 * @vdev: VDEV DP object
 * @nbuf: data packet
 * @peer: Peer DP object
 * @is_egress: whether egress or ingress
 * @is_rx: whether rx or tx
 *
 * This function updates the per-peer protocol counters
 * Return: void
 */
void dp_vdev_peer_stats_update_protocol_cnt(struct dp_vdev *vdev,
					    qdf_nbuf_t nbuf,
					    struct dp_peer *peer,
					    bool is_egress,
					    bool is_rx);

/**
 * dp_vdev_peer_stats_update_protocol_cnt() - update per-peer protocol counters
 * @soc: SOC DP object
 * @vdev_id: vdev_id
 * @nbuf: data packet
 * @is_egress: whether egress or ingress
 * @is_rx: whether rx or tx
 *
 * This function updates the per-peer protocol counters
 * Return: void
 */

void dp_peer_stats_update_protocol_cnt(struct cdp_soc_t *soc,
				       int8_t vdev_id,
				       qdf_nbuf_t nbuf,
				       bool is_egress,
				       bool is_rx);

void dp_vdev_peer_stats_update_protocol_cnt_tx(struct dp_vdev *vdev_hdl,
					       qdf_nbuf_t nbuf);

#else
#define dp_vdev_peer_stats_update_protocol_cnt(vdev, nbuf, peer, \
					       is_egress, is_rx)

static inline
void dp_vdev_peer_stats_update_protocol_cnt_tx(struct dp_vdev *vdev_hdl,
					       qdf_nbuf_t nbuf)
{
}

#endif

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
void dp_tx_dump_flow_pool_info(struct cdp_soc_t *soc_hdl);

/**
 * dp_tx_dump_flow_pool_info_compact() - dump flow pool info
 * @soc: DP soc context
 *
 * Return: none
 */
void dp_tx_dump_flow_pool_info_compact(struct dp_soc *soc);
int dp_tx_delete_flow_pool(struct dp_soc *soc, struct dp_tx_desc_pool_s *pool,
	bool force);
#else
static inline void dp_tx_dump_flow_pool_info_compact(struct dp_soc *soc)
{
}
#endif /* QCA_LL_TX_FLOW_CONTROL_V2 */

#ifdef QCA_OL_DP_SRNG_LOCK_LESS_ACCESS
static inline int
dp_hal_srng_access_start(hal_soc_handle_t soc, hal_ring_handle_t hal_ring_hdl)
{
	return hal_srng_access_start_unlocked(soc, hal_ring_hdl);
}

static inline void
dp_hal_srng_access_end(hal_soc_handle_t soc, hal_ring_handle_t hal_ring_hdl)
{
	hal_srng_access_end_unlocked(soc, hal_ring_hdl);
}

#else
static inline int
dp_hal_srng_access_start(hal_soc_handle_t soc, hal_ring_handle_t hal_ring_hdl)
{
	return hal_srng_access_start(soc, hal_ring_hdl);
}

static inline void
dp_hal_srng_access_end(hal_soc_handle_t soc, hal_ring_handle_t hal_ring_hdl)
{
	hal_srng_access_end(soc, hal_ring_hdl);
}
#endif

#ifdef WLAN_FEATURE_DP_EVENT_HISTORY
/**
 * dp_srng_access_start() - Wrapper function to log access start of a hal ring
 * @int_ctx: pointer to DP interrupt context. This should not be NULL
 * @soc: DP Soc handle
 * @hal_ring: opaque pointer to the HAL Rx Error Ring, which will be serviced
 *
 * Return: 0 on success; error on failure
 */
int dp_srng_access_start(struct dp_intr *int_ctx, struct dp_soc *dp_soc,
			 hal_ring_handle_t hal_ring_hdl);

/**
 * dp_srng_access_end() - Wrapper function to log access end of a hal ring
 * @int_ctx: pointer to DP interrupt context. This should not be NULL
 * @soc: DP Soc handle
 * @hal_ring: opaque pointer to the HAL Rx Error Ring, which will be serviced
 *
 * Return: void
 */
void dp_srng_access_end(struct dp_intr *int_ctx, struct dp_soc *dp_soc,
			hal_ring_handle_t hal_ring_hdl);

#else
static inline int dp_srng_access_start(struct dp_intr *int_ctx,
				       struct dp_soc *dp_soc,
				       hal_ring_handle_t hal_ring_hdl)
{
	hal_soc_handle_t hal_soc = dp_soc->hal_soc;

	return dp_hal_srng_access_start(hal_soc, hal_ring_hdl);
}

static inline void dp_srng_access_end(struct dp_intr *int_ctx,
				      struct dp_soc *dp_soc,
				      hal_ring_handle_t hal_ring_hdl)
{
	hal_soc_handle_t hal_soc = dp_soc->hal_soc;

	return dp_hal_srng_access_end(hal_soc, hal_ring_hdl);
}
#endif /* WLAN_FEATURE_DP_EVENT_HISTORY */

#ifdef QCA_CACHED_RING_DESC
/**
 * dp_srng_dst_get_next() - Wrapper function to get next ring desc
 * @dp_socsoc: DP Soc handle
 * @hal_ring: opaque pointer to the HAL Destination Ring
 *
 * Return: HAL ring descriptor
 */
static inline void *dp_srng_dst_get_next(struct dp_soc *dp_soc,
					 hal_ring_handle_t hal_ring_hdl)
{
	hal_soc_handle_t hal_soc = dp_soc->hal_soc;

	return hal_srng_dst_get_next_cached(hal_soc, hal_ring_hdl);
}

/**
 * dp_srng_dst_inv_cached_descs() - Wrapper function to invalidate cached
 * descriptors
 * @dp_socsoc: DP Soc handle
 * @hal_ring: opaque pointer to the HAL Rx Destination ring
 * @num_entries: Entry count
 *
 * Return: None
 */
static inline void dp_srng_dst_inv_cached_descs(struct dp_soc *dp_soc,
						hal_ring_handle_t hal_ring_hdl,
						uint32_t num_entries)
{
	hal_soc_handle_t hal_soc = dp_soc->hal_soc;

	hal_srng_dst_inv_cached_descs(hal_soc, hal_ring_hdl, num_entries);
}
#else
static inline void *dp_srng_dst_get_next(struct dp_soc *dp_soc,
					 hal_ring_handle_t hal_ring_hdl)
{
	hal_soc_handle_t hal_soc = dp_soc->hal_soc;

	return hal_srng_dst_get_next(hal_soc, hal_ring_hdl);
}

static inline void dp_srng_dst_inv_cached_descs(struct dp_soc *dp_soc,
						hal_ring_handle_t hal_ring_hdl,
						uint32_t num_entries)
{
}
#endif /* QCA_CACHED_RING_DESC */

#ifdef QCA_ENH_V3_STATS_SUPPORT
/**
 * dp_pdev_print_delay_stats(): Print pdev level delay stats
 * @pdev: DP_PDEV handle
 *
 * Return:void
 */
void dp_pdev_print_delay_stats(struct dp_pdev *pdev);

/**
 * dp_pdev_print_tid_stats(): Print pdev level tid stats
 * @pdev: DP_PDEV handle
 *
 * Return:void
 */
void dp_pdev_print_tid_stats(struct dp_pdev *pdev);

/**
 * dp_pdev_print_rx_error_stats(): Print pdev level rx error stats
 * @pdev: DP_PDEV handle
 *
 * Return:void
 */
void dp_pdev_print_rx_error_stats(struct dp_pdev *pdev);
#endif /* CONFIG_WIN */

void dp_soc_set_txrx_ring_map(struct dp_soc *soc);

/**
 * dp_vdev_to_cdp_vdev() - typecast dp vdev to cdp vdev
 * @vdev: DP vdev handle
 *
 * Return: struct cdp_vdev pointer
 */
static inline
struct cdp_vdev *dp_vdev_to_cdp_vdev(struct dp_vdev *vdev)
{
	return (struct cdp_vdev *)vdev;
}

/**
 * dp_pdev_to_cdp_pdev() - typecast dp pdev to cdp pdev
 * @pdev: DP pdev handle
 *
 * Return: struct cdp_pdev pointer
 */
static inline
struct cdp_pdev *dp_pdev_to_cdp_pdev(struct dp_pdev *pdev)
{
	return (struct cdp_pdev *)pdev;
}

/**
 * dp_soc_to_cdp_soc() - typecast dp psoc to cdp psoc
 * @psoc: DP psoc handle
 *
 * Return: struct cdp_soc pointer
 */
static inline
struct cdp_soc *dp_soc_to_cdp_soc(struct dp_soc *psoc)
{
	return (struct cdp_soc *)psoc;
}

/**
 * dp_soc_to_cdp_soc_t() - typecast dp psoc to
 * ol txrx soc handle
 * @psoc: DP psoc handle
 *
 * Return: struct cdp_soc_t pointer
 */
static inline
struct cdp_soc_t *dp_soc_to_cdp_soc_t(struct dp_soc *psoc)
{
	return (struct cdp_soc_t *)psoc;
}

#if defined(WLAN_SUPPORT_RX_FLOW_TAG) || defined(WLAN_SUPPORT_RX_FISA)
/**
 * dp_rx_flow_update_fse_stats() - Update a flow's statistics
 * @pdev: pdev handle
 * @flow_id: flow index (truncated hash) in the Rx FST
 *
 * Return: Success when flow statistcs is updated, error on failure
 */
QDF_STATUS dp_rx_flow_get_fse_stats(struct dp_pdev *pdev,
				    struct cdp_rx_flow_info *rx_flow_info,
				    struct cdp_flow_stats *stats);

/**
 * dp_rx_flow_delete_entry() - Delete a flow entry from flow search table
 * @pdev: pdev handle
 * @rx_flow_info: DP flow parameters
 *
 * Return: Success when flow is deleted, error on failure
 */
QDF_STATUS dp_rx_flow_delete_entry(struct dp_pdev *pdev,
				   struct cdp_rx_flow_info *rx_flow_info);

/**
 * dp_rx_flow_add_entry() - Add a flow entry to flow search table
 * @pdev: DP pdev instance
 * @rx_flow_info: DP flow paramaters
 *
 * Return: Success when flow is added, no-memory or already exists on error
 */
QDF_STATUS dp_rx_flow_add_entry(struct dp_pdev *pdev,
				struct cdp_rx_flow_info *rx_flow_info);

/**
 * dp_rx_fst_attach() - Initialize Rx FST and setup necessary parameters
 * @soc: SoC handle
 * @pdev: Pdev handle
 *
 * Return: Handle to flow search table entry
 */
QDF_STATUS dp_rx_fst_attach(struct dp_soc *soc, struct dp_pdev *pdev);

/**
 * dp_rx_fst_detach() - De-initialize Rx FST
 * @soc: SoC handle
 * @pdev: Pdev handle
 *
 * Return: None
 */
void dp_rx_fst_detach(struct dp_soc *soc, struct dp_pdev *pdev);

/**
 * dp_rx_flow_send_fst_fw_setup() - Program FST parameters in FW/HW post-attach
 * @soc: SoC handle
 * @pdev: Pdev handle
 *
 * Return: Success when fst parameters are programmed in FW, error otherwise
 */
QDF_STATUS dp_rx_flow_send_fst_fw_setup(struct dp_soc *soc,
					struct dp_pdev *pdev);
#else /* !((WLAN_SUPPORT_RX_FLOW_TAG) || defined(WLAN_SUPPORT_RX_FISA)) */

/**
 * dp_rx_fst_attach() - Initialize Rx FST and setup necessary parameters
 * @soc: SoC handle
 * @pdev: Pdev handle
 *
 * Return: Handle to flow search table entry
 */
static inline
QDF_STATUS dp_rx_fst_attach(struct dp_soc *soc, struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_fst_detach() - De-initialize Rx FST
 * @soc: SoC handle
 * @pdev: Pdev handle
 *
 * Return: None
 */
static inline
void dp_rx_fst_detach(struct dp_soc *soc, struct dp_pdev *pdev)
{
}
#endif

/**
 * dp_vdev_get_ref() - API to take a reference for VDEV object
 *
 * @soc		: core DP soc context
 * @vdev	: DP vdev
 * @mod_id	: module id
 *
 * Return:	QDF_STATUS_SUCCESS if reference held successfully
 *		else QDF_STATUS_E_INVAL
 */
static inline
QDF_STATUS dp_vdev_get_ref(struct dp_soc *soc, struct dp_vdev *vdev,
			   enum dp_mod_id mod_id)
{
	if (!qdf_atomic_inc_not_zero(&vdev->ref_cnt))
		return QDF_STATUS_E_INVAL;

	qdf_atomic_inc(&vdev->mod_refs[mod_id]);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_vdev_get_ref_by_id() - Returns vdev object given the vdev id
 * @soc: core DP soc context
 * @vdev_id: vdev id from vdev object can be retrieved
 * @mod_id: module id which is requesting the reference
 *
 * Return: struct dp_vdev*: Pointer to DP vdev object
 */
static inline struct dp_vdev *
dp_vdev_get_ref_by_id(struct dp_soc *soc, uint8_t vdev_id,
		      enum dp_mod_id mod_id)
{
	struct dp_vdev *vdev = NULL;
	if (qdf_unlikely(vdev_id >= MAX_VDEV_CNT))
		return NULL;

	qdf_spin_lock_bh(&soc->vdev_map_lock);
	vdev = soc->vdev_id_map[vdev_id];

	if (!vdev || dp_vdev_get_ref(soc, vdev, mod_id) != QDF_STATUS_SUCCESS) {
		qdf_spin_unlock_bh(&soc->vdev_map_lock);
		return NULL;
	}
	qdf_spin_unlock_bh(&soc->vdev_map_lock);

	return vdev;
}

/**
 * dp_get_pdev_from_soc_pdev_id_wifi3() - Returns pdev object given the pdev id
 * @soc: core DP soc context
 * @pdev_id: pdev id from pdev object can be retrieved
 *
 * Return: struct dp_pdev*: Pointer to DP pdev object
 */
static inline struct dp_pdev *
dp_get_pdev_from_soc_pdev_id_wifi3(struct dp_soc *soc,
				   uint8_t pdev_id)
{
	if (qdf_unlikely(pdev_id >= MAX_PDEV_CNT))
		return NULL;

	return soc->pdev_list[pdev_id];
}

/*
 * dp_rx_tid_update_wifi3() – Update receive TID state
 * @peer: Datapath peer handle
 * @tid: TID
 * @ba_window_size: BlockAck window size
 * @start_seq: Starting sequence number
 * @bar_update: BAR update triggered
 *
 * Return: QDF_STATUS code
 */
QDF_STATUS dp_rx_tid_update_wifi3(struct dp_peer *peer, int tid, uint32_t
					 ba_window_size, uint32_t start_seq,
					 bool bar_update);

/**
 * dp_get_peer_mac_list(): function to get peer mac list of vdev
 * @soc: Datapath soc handle
 * @vdev_id: vdev id
 * @newmac: Table of the clients mac
 * @mac_cnt: No. of MACs required
 * @limit: Limit the number of clients
 *
 * return: no of clients
 */
uint16_t dp_get_peer_mac_list(ol_txrx_soc_handle soc, uint8_t vdev_id,
			      u_int8_t newmac[][QDF_MAC_ADDR_SIZE],
			      u_int16_t mac_cnt, bool limit);
/*
 * dp_is_hw_dbs_enable() - Procedure to check if DBS is supported
 * @soc:		DP SoC context
 * @max_mac_rings:	No of MAC rings
 *
 * Return: None
 */
void dp_is_hw_dbs_enable(struct dp_soc *soc,
				int *max_mac_rings);


#if defined(WLAN_SUPPORT_RX_FISA)
void dp_rx_dump_fisa_table(struct dp_soc *soc);

/*
 * dp_rx_fst_update_cmem_params() - Update CMEM FST params
 * @soc:		DP SoC context
 * @num_entries:	Number of flow search entries
 * @cmem_ba_lo:		CMEM base address low
 * @cmem_ba_hi:		CMEM base address high
 *
 * Return: None
 */
void dp_rx_fst_update_cmem_params(struct dp_soc *soc, uint16_t num_entries,
				  uint32_t cmem_ba_lo, uint32_t cmem_ba_hi);

void
dp_rx_fst_update_pm_suspend_status(struct dp_soc *soc, bool suspended);
#else
static inline void
dp_rx_fst_update_cmem_params(struct dp_soc *soc, uint16_t num_entries,
			     uint32_t cmem_ba_lo, uint32_t cmem_ba_hi)
{
}

static inline void
dp_rx_fst_update_pm_suspend_status(struct dp_soc *soc, bool suspended)
{
}
#endif /* WLAN_SUPPORT_RX_FISA */

#ifdef MAX_ALLOC_PAGE_SIZE
/**
 * dp_set_page_size() - Set the max page size for hw link desc.
 * For MCL the page size is set to OS defined value and for WIN
 * the page size is set to the max_alloc_size cfg ini
 * param.
 * This is to ensure that WIN gets contiguous memory allocations
 * as per requirement.
 * @pages: link desc page handle
 * @max_alloc_size: max_alloc_size
 *
 * Return: None
 */
static inline
void dp_set_max_page_size(struct qdf_mem_multi_page_t *pages,
			  uint32_t max_alloc_size)
{
	pages->page_size = qdf_page_size;
}

#else
static inline
void dp_set_max_page_size(struct qdf_mem_multi_page_t *pages,
			  uint32_t max_alloc_size)
{
	pages->page_size = max_alloc_size;
}
#endif /* MAX_ALLOC_PAGE_SIZE */

/**
 * dp_history_get_next_index() - get the next entry to record an entry
 *				 in the history.
 * @curr_idx: Current index where the last entry is written.
 * @max_entries: Max number of entries in the history
 *
 * This function assumes that the max number os entries is a power of 2.
 *
 * Returns: The index where the next entry is to be written.
 */
static inline uint32_t dp_history_get_next_index(qdf_atomic_t *curr_idx,
						 uint32_t max_entries)
{
	uint32_t idx = qdf_atomic_inc_return(curr_idx);

	return idx & (max_entries - 1);
}

/**
 * dp_rx_skip_tlvs() - Skip TLVs len + L2 hdr_offset, save in nbuf->cb
 * @nbuf: nbuf cb to be updated
 * @l2_hdr_offset: l2_hdr_offset
 *
 * Return: None
 */
void dp_rx_skip_tlvs(struct dp_soc *soc, qdf_nbuf_t nbuf, uint32_t l3_padding);

#ifndef FEATURE_WDS
static inline void
dp_hmwds_ast_add_notify(struct dp_peer *peer,
			uint8_t *mac_addr,
			enum cdp_txrx_ast_entry_type type,
			QDF_STATUS err,
			bool is_peer_map)
{
}
#endif

#ifdef HTT_STATS_DEBUGFS_SUPPORT
/* dp_pdev_htt_stats_dbgfs_init() - Function to allocate memory and initialize
 * debugfs for HTT stats
 * @pdev: dp pdev handle
 *
 * Return: QDF_STATUS
 */
QDF_STATUS dp_pdev_htt_stats_dbgfs_init(struct dp_pdev *pdev);

/* dp_pdev_htt_stats_dbgfs_deinit() - Function to remove debugfs entry for
 * HTT stats
 * @pdev: dp pdev handle
 *
 * Return: none
 */
void dp_pdev_htt_stats_dbgfs_deinit(struct dp_pdev *pdev);
#else

/* dp_pdev_htt_stats_dbgfs_init() - Function to allocate memory and initialize
 * debugfs for HTT stats
 * @pdev: dp pdev handle
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS
dp_pdev_htt_stats_dbgfs_init(struct dp_pdev *pdev)
{
	return QDF_STATUS_SUCCESS;
}

/* dp_pdev_htt_stats_dbgfs_deinit() - Function to remove debugfs entry for
 * HTT stats
 * @pdev: dp pdev handle
 *
 * Return: none
 */
static inline void
dp_pdev_htt_stats_dbgfs_deinit(struct dp_pdev *pdev)
{
}
#endif /* HTT_STATS_DEBUGFS_SUPPORT */

#ifndef WLAN_DP_FEATURE_SW_LATENCY_MGR
/**
 * dp_soc_swlm_attach() - attach the software latency manager resources
 * @soc: Datapath global soc handle
 *
 * Returns: QDF_STATUS
 */
static inline QDF_STATUS dp_soc_swlm_attach(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_soc_swlm_detach() - detach the software latency manager resources
 * @soc: Datapath global soc handle
 *
 * Returns: QDF_STATUS
 */
static inline QDF_STATUS dp_soc_swlm_detach(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}
#endif /* !WLAN_DP_FEATURE_SW_LATENCY_MGR */

#ifdef QCA_SUPPORT_WDS_EXTENDED
/**
 * dp_wds_ext_get_peer_id(): function to get peer id by mac
 * This API is called from control path when wds extended
 * device is created, hence it also updates wds extended
 * peer state to up, which will be referred in rx processing.
 * @soc: Datapath soc handle
 * @vdev_id: vdev id
 * @mac: Peer mac address
 *
 * return: valid peer id on success
 *         HTT_INVALID_PEER on failure
 */
uint16_t dp_wds_ext_get_peer_id(ol_txrx_soc_handle soc,
				uint8_t vdev_id,
				uint8_t *mac);

/**
 * dp_wds_ext_set_peer_state(): function to set peer state
 * @soc: Datapath soc handle
 * @vdev_id: vdev id
 * @mac: Peer mac address
 * @rx: rx function pointer
 *
 * return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_INVAL if peer is not found
 *         QDF_STATUS_E_ALREADY if rx is already set/unset
 */
QDF_STATUS dp_wds_ext_set_peer_rx(ol_txrx_soc_handle soc,
				  uint8_t vdev_id,
				  uint8_t *mac,
				  ol_txrx_rx_fp rx,
				  ol_osif_peer_handle osif_peer);
#endif /* QCA_SUPPORT_WDS_EXTENDED */

#ifdef DP_MEM_PRE_ALLOC

/**
 * dp_context_alloc_mem() - allocate memory for DP context
 * @soc: datapath soc handle
 * @ctxt_type: DP context type
 * @ctxt_size: DP context size
 *
 * Return: DP context address
 */
void *dp_context_alloc_mem(struct dp_soc *soc, enum dp_ctxt_type ctxt_type,
			   size_t ctxt_size);

/**
 * dp_context_free_mem() - Free memory of DP context
 * @soc: datapath soc handle
 * @ctxt_type: DP context type
 * @vaddr: Address of context memory
 *
 * Return: None
 */
void dp_context_free_mem(struct dp_soc *soc, enum dp_ctxt_type ctxt_type,
			 void *vaddr);

/**
 * dp_desc_multi_pages_mem_alloc() - alloc memory over multiple pages
 * @soc: datapath soc handle
 * @desc_type: memory request source type
 * @pages: multi page information storage
 * @element_size: each element size
 * @element_num: total number of elements should be allocated
 * @memctxt: memory context
 * @cacheable: coherent memory or cacheable memory
 *
 * This function is a wrapper for memory allocation over multiple
 * pages, if dp prealloc method is registered, then will try prealloc
 * firstly. if prealloc failed, fall back to regular way over
 * qdf_mem_multi_pages_alloc().
 *
 * Return: None
 */
void dp_desc_multi_pages_mem_alloc(struct dp_soc *soc,
				   enum dp_desc_type desc_type,
				   struct qdf_mem_multi_page_t *pages,
				   size_t element_size,
				   uint16_t element_num,
				   qdf_dma_context_t memctxt,
				   bool cacheable);

/**
 * dp_desc_multi_pages_mem_free() - free multiple pages memory
 * @soc: datapath soc handle
 * @desc_type: memory request source type
 * @pages: multi page information storage
 * @memctxt: memory context
 * @cacheable: coherent memory or cacheable memory
 *
 * This function is a wrapper for multiple pages memory free,
 * if memory is got from prealloc pool, put it back to pool.
 * otherwise free by qdf_mem_multi_pages_free().
 *
 * Return: None
 */
void dp_desc_multi_pages_mem_free(struct dp_soc *soc,
				  enum dp_desc_type desc_type,
				  struct qdf_mem_multi_page_t *pages,
				  qdf_dma_context_t memctxt,
				  bool cacheable);

#else
static inline
void *dp_context_alloc_mem(struct dp_soc *soc, enum dp_ctxt_type ctxt_type,
			   size_t ctxt_size)
{
	return qdf_mem_malloc(ctxt_size);
}

static inline
void dp_context_free_mem(struct dp_soc *soc, enum dp_ctxt_type ctxt_type,
			 void *vaddr)
{
	qdf_mem_free(vaddr);
}

static inline
void dp_desc_multi_pages_mem_alloc(struct dp_soc *soc,
				   enum dp_desc_type desc_type,
				   struct qdf_mem_multi_page_t *pages,
				   size_t element_size,
				   uint16_t element_num,
				   qdf_dma_context_t memctxt,
				   bool cacheable)
{
	qdf_mem_multi_pages_alloc(soc->osdev, pages, element_size,
				  element_num, memctxt, cacheable);
}

static inline
void dp_desc_multi_pages_mem_free(struct dp_soc *soc,
				  enum dp_desc_type desc_type,
				  struct qdf_mem_multi_page_t *pages,
				  qdf_dma_context_t memctxt,
				  bool cacheable)
{
	qdf_mem_multi_pages_free(soc->osdev, pages,
				 memctxt, cacheable);
}
#endif

#ifdef FEATURE_RUNTIME_PM
/**
 * dp_runtime_get() - Get dp runtime refcount
 * @soc: Datapath soc handle
 *
 * Get dp runtime refcount by increment of an atomic variable, which can block
 * dp runtime resume to wait to flush pending tx by runtime suspend.
 *
 * Return: Current refcount
 */
static inline int32_t dp_runtime_get(struct dp_soc *soc)
{
	return qdf_atomic_inc_return(&soc->dp_runtime_refcount);
}

/**
 * dp_runtime_put() - Return dp runtime refcount
 * @soc: Datapath soc handle
 *
 * Return dp runtime refcount by decrement of an atomic variable, allow dp
 * runtime resume finish.
 *
 * Return: Current refcount
 */
static inline int32_t dp_runtime_put(struct dp_soc *soc)
{
	return qdf_atomic_dec_return(&soc->dp_runtime_refcount);
}

/**
 * dp_runtime_get_refcount() - Get dp runtime refcount
 * @soc: Datapath soc handle
 *
 * Get dp runtime refcount by returning an atomic variable
 *
 * Return: Current refcount
 */
static inline int32_t dp_runtime_get_refcount(struct dp_soc *soc)
{
	return qdf_atomic_read(&soc->dp_runtime_refcount);
}

/**
 * dp_runtime_init() - Init dp runtime refcount when dp soc init
 * @soc: Datapath soc handle
 *
 * Return: QDF_STATUS
 */
static inline QDF_STATUS dp_runtime_init(struct dp_soc *soc)
{
	return qdf_atomic_init(&soc->dp_runtime_refcount);
}
#else
static inline int32_t dp_runtime_get(struct dp_soc *soc)
{
	return 0;
}

static inline int32_t dp_runtime_put(struct dp_soc *soc)
{
	return 0;
}

static inline QDF_STATUS dp_runtime_init(struct dp_soc *soc)
{
	return QDF_STATUS_SUCCESS;
}
#endif

/*
 * dp_pdev_bkp_stats_detach() - detach resources for back pressure stats
 *				processing
 * @pdev: Datapath PDEV handle
 *
 */
void dp_pdev_bkp_stats_detach(struct dp_pdev *pdev);

/*
 * dp_pdev_bkp_stats_attach() - attach resources for back pressure stats
 *				processing
 * @pdev: Datapath PDEV handle
 *
 * Return: QDF_STATUS_SUCCESS: Success
 *         QDF_STATUS_E_NOMEM: Error
 */

QDF_STATUS dp_pdev_bkp_stats_attach(struct dp_pdev *pdev);

/**
 * dp_peer_flush_frags() - Flush all fragments for a particular
 *  peer
 * @soc_hdl - data path soc handle
 * @vdev_id - vdev id
 * @peer_addr - peer mac address
 *
 * Return: None
 */
void dp_peer_flush_frags(struct cdp_soc_t *soc_hdl, uint8_t vdev_id,
			 uint8_t *peer_mac);

/**
 * dp_soc_reset_mon_intr_mask() - reset mon intr mask
 * @soc: pointer to dp_soc handle
 *
 * Return:
 */
void dp_soc_reset_mon_intr_mask(struct dp_soc *soc);
#endif /* #ifndef _DP_INTERNAL_H_ */
