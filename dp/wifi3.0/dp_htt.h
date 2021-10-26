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

#ifndef _DP_HTT_H_
#define _DP_HTT_H_

#include <qdf_types.h>
#include <qdf_lock.h>
#include <qdf_nbuf.h>
#include <htc_api.h>

#include "cdp_txrx_cmn_struct.h"
#include "dp_types.h"
#ifdef HTT_LOGGER
#include "dp_htt_logger.h"
#else
struct htt_logger;
static inline
void htt_interface_logging_init(struct htt_logger **htt_logger_handle,
				struct cdp_ctrl_objmgr_psoc *ctrl_psoc)
{
}

static inline
void htt_interface_logging_deinit(struct htt_logger *htt_logger_handle)
{
}

static inline
int htt_command_record(struct htt_logger *h, uint8_t msg_type,
		       uint8_t *msg_data)
{
	return 0;
}

static inline
int htt_event_record(struct htt_logger *h, uint8_t msg_type,
		     uint8_t *msg_data)
{
	return 0;
}

static inline
int htt_wbm_event_record(struct htt_logger *h, uint8_t tx_status,
			 uint8_t *msg_data)
{
	return 0;
}

#endif

void htt_htc_pkt_pool_free(struct htt_soc *soc);

#define HTT_TX_MUTEX_TYPE qdf_spinlock_t

#define HTT_TX_MUTEX_INIT(_mutex)				\
	qdf_spinlock_create(_mutex)

#define HTT_TX_MUTEX_ACQUIRE(_mutex)			\
	qdf_spin_lock_bh(_mutex)

#define HTT_TX_MUTEX_RELEASE(_mutex)			\
	qdf_spin_unlock_bh(_mutex)

#define HTT_TX_MUTEX_DESTROY(_mutex)			\
	qdf_spinlock_destroy(_mutex)

#define DP_HTT_MAX_SEND_QUEUE_DEPTH 64

#ifndef HTT_MAC_ADDR_LEN
#define HTT_MAC_ADDR_LEN 6
#endif

#define HTT_FRAMECTRL_TYPE_MASK 0x0C
#define HTT_GET_FRAME_CTRL_TYPE(_val)	\
		(((_val) & HTT_FRAMECTRL_TYPE_MASK) >> 2)
#define FRAME_CTRL_TYPE_MGMT	0x0
#define FRAME_CTRL_TYPE_CTRL	0x1
#define FRAME_CTRL_TYPE_DATA	0x2
#define FRAME_CTRL_TYPE_RESV	0x3

#define HTT_FRAMECTRL_DATATYPE 0x08
#define HTT_PPDU_DESC_MAX_DEPTH 16
#define DP_SCAN_PEER_ID 0xFFFF

#define HTT_RX_DELBA_WIN_SIZE_M    0x0000FC00
#define HTT_RX_DELBA_WIN_SIZE_S    10

#define HTT_RX_DELBA_WIN_SIZE_GET(word)		\
	(((word) & HTT_RX_DELBA_WIN_SIZE_M) >> HTT_RX_DELBA_WIN_SIZE_S)

/*
 * Set the base misclist size to HTT copy engine source ring size
 * to guarantee that a packet on the misclist wont be freed while it
 * is sitting in the copy engine.
 */
#define DP_HTT_HTC_PKT_MISCLIST_SIZE          2048
#define HTT_T2H_MAX_MSG_SIZE 2048

#define HTT_T2H_EXT_STATS_TLV_START_OFFSET    3

/*
 * Below offset are based on htt_ppdu_stats_common_tlv
 * defined in htt_ppdu_stats.h
 */
#define HTT_PPDU_STATS_COMMON_TLV_TLV_HDR_OFFSET 0
#define HTT_PPDU_STATS_COMMON_TLV_PPDU_ID_OFFSET 1
#define HTT_PPDU_STATS_COMMON_TLV_RING_ID_SCH_CMD_ID_OFFSET 2
#define HTT_PPDU_STATS_COMMON_TLV_QTYPE_FRM_TYPE_OFFSET 3
#define HTT_PPDU_STATS_COMMON_TLV_CHAIN_MASK_OFFSET 4
#define HTT_PPDU_STATS_COMMON_TLV_FES_DUR_US_OFFSET 5
#define HTT_PPDU_STATS_COMMON_TLV_SCH_EVAL_START_TSTMP_L32_US_OFFSET 6
#define HTT_PPDU_STATS_COMMON_TLV_SCH_END_TSTMP_US_OFFSET 7
#define HTT_PPDU_STATS_COMMON_TLV_START_TSTMP_L32_US_OFFSET 8
#define HTT_PPDU_STATS_COMMON_TLV_CHAN_MHZ_PHY_MODE_OFFSET 9
#define HTT_PPDU_STATS_COMMON_TLV_CCA_DELTA_TIME_US_OFFSET 10
#define HTT_PPDU_STATS_COMMON_TLV_RXFRM_DELTA_TIME_US_OFFSET 11
#define HTT_PPDU_STATS_COMMON_TLV_TXFRM_DELTA_TIME_US_OFFSET 12
#define HTT_PPDU_STATS_COMMON_TLV_RESV_NUM_UL_BEAM_OFFSET 13
#define HTT_PPDU_STATS_COMMON_TLV_START_TSTMP_U32_US_OFFSET 14
#define HTT_PPDU_STATS_COMMON_TLV_BSSCOLOR_OBSS_PSR_OFFSET 15

/* get index for field in htt_ppdu_stats_common_tlv */
#define HTT_GET_STATS_CMN_INDEX(index) \
	HTT_PPDU_STATS_COMMON_TLV_##index##_OFFSET

#define MAX_SCHED_STARVE 100000
#define WRAP_DROP_TSF_DELTA 10000
#define MAX_TSF_32 0xFFFFFFFF

#define dp_htt_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_DP_HTT, params)
#define dp_htt_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_HTT, params)
#define dp_htt_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_HTT, params)
#define dp_htt_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_HTT, ## params)
#define dp_htt_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_DP_HTT, params)

#define dp_htt_tx_stats_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_DP_HTT_TX_STATS, params)
#define dp_htt_tx_stats_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_HTT_TX_STATS, params)
#define dp_htt_tx_stats_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_HTT_TX_STATS, params)
#define dp_htt_tx_stats_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_HTT_TX_STATS, ## params)
#define dp_htt_tx_stats_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_DP_HTT_TX_STATS, params)

/**
 * enum dp_full_mon_config - enum to enable/disable full monitor mode
 *
 * @DP_FULL_MON_DISABLE: Disable full monitor mode
 * @DP_FULL_MON_ENABLE: Enable full monitor mode
 */
enum dp_full_mon_config {
	DP_FULL_MON_DISABLE,
	DP_FULL_MON_ENABLE,
};

struct dp_htt_htc_pkt {
	void *soc_ctxt;
	qdf_dma_addr_t nbuf_paddr;
	HTC_PACKET htc_pkt;
};

struct dp_htt_htc_pkt_union {
	union {
		struct dp_htt_htc_pkt pkt;
		struct dp_htt_htc_pkt_union *next;
	} u;
};

struct dp_htt_timestamp {
	long *umac_ttt;
	long *lmac_ttt;
};

struct htt_soc {
	struct cdp_ctrl_objmgr_psoc *ctrl_psoc;
	struct dp_soc *dp_soc;
	hal_soc_handle_t hal_soc;
	struct dp_htt_timestamp pdevid_tt[MAX_PDEV_CNT];
	/* htt_logger handle */
	struct htt_logger *htt_logger_handle;
	HTC_HANDLE htc_soc;
	qdf_device_t osdev;
	HTC_ENDPOINT_ID htc_endpoint;
	struct dp_htt_htc_pkt_union *htt_htc_pkt_freelist;
	struct dp_htt_htc_pkt_union *htt_htc_pkt_misclist;
	struct {
		u_int8_t major;
		u_int8_t minor;
	} tgt_ver;
	struct {
		u_int8_t major;
		u_int8_t minor;
	} wifi_ip_ver;

	struct {
		int htc_err_cnt;
		int htc_pkt_free;
		int skip_count;
		int fail_count;
		/* rtpm put skip count for ver req msg */
		int htt_ver_req_put_skip;
	} stats;

	HTT_TX_MUTEX_TYPE htt_tx_mutex;
};

#ifdef QCA_MONITOR_2_0_SUPPORT
/**
 * struct dp_tx_mon_downstream_tlv_config - Enable/Disable TxMon
 * downstream TLVs
 * tx_fes_setup: TX_FES_SETUP TLV
 * tx_peer_entry: TX_PEER_ENTRY TLV
 * tx_queue_extension: TX_QUEUE_EXTENSION TLV
 * tx_last_mpdu_end: TX_LAST_MPDU_END TLV
 * tx_last_mpdu_fetched: TX_LAST_MPDU_FETCHED TLV
 * tx_data_sync: TX_DATA_SYNC TLV
 * pcu_ppdu_setup_init: PCU_PPDU_SETUP_INIT TLV
 * fw2s_mon: FW2S_MON TLV
 * tx_loopback_setup: TX_LOOPBACK_SETUP TLV
 * sch_critical_tlv_ref: SCH_CRITICAL_TLV_REF TLV
 * ndp_preamble_done: NDP_PREAMBLE_DONE TLV
 * tx_raw_frame_setup: TX_RAW_OR_NATIVE_FRAME_SETUP TLV
 * txpcu_user_setup: TXPCU_USER_SETUP TLV
 * rxpcu_setup: RXPCU_SETUP TLV
 * rxpcu_setup_complete: RXPCU_SETUP_COMPLETE TLV
 * coex_tx_req: COEX_TX_REQ TLV
 * rxpcu_user_setup: RXPCU_USER_SETUP TLV
 * rxpcu_user_setup_ext: RXPCU_USER_SETUP_EXT TLV
 * wur_data: WUR_DATA TLV
 * tqm_mpdu_global_start: TQM_MPDU_GLOBAL_START
 * tx_fes_setup_complete: TX_FES_SETUP_COMPLETE TLV
 * scheduler_end: SCHEDULER_END TLV
 * sch_wait_instr_tx_path: SCH_WAIT_INSTR_TX_PATH TLV
 *
 */
struct dp_tx_mon_downstream_tlv_config {
	uint32_t tx_fes_setup:1,
		 tx_peer_entry:1,
		 tx_queue_extension:1,
		 tx_last_mpdu_end:1,
		 tx_last_mpdu_fetched:1,
		 tx_data_sync:1,
		 pcu_ppdu_setup_init:1,
		 fw2s_mon:1,
		 tx_loopback_setup:1,
		 sch_critical_tlv_ref:1,
		 ndp_preamble_done:1,
		 tx_raw_frame_setup:1,
		 txpcu_user_setup:1,
		 rxpcu_setup:1,
		 rxpcu_setup_complete:1,
		 coex_tx_req:1,
		 rxpcu_user_setup:1,
		 rxpcu_user_setup_ext:1,
		 wur_data:1,
		 tqm_mpdu_global_start:1,
		 tx_fes_setup_complete:1,
		 scheduler_end:1,
		 sch_wait_instr_tx_path:1;
};

/**
 * struct dp_tx_mon_upstream_tlv_config - Enable/Disable TxMon
 * upstream TLVs
 * rx_response_required_info: RX_RESPONSE_REQUIRED_INFO
 * TLV
 * response_start_status: RESPONSE_START_STATUS TLV
 * response_end_status: RESPONSE_END_STATUS TLV
 * tx_fes_status_start: TX_FES_STATUS_START TLV
 * tx_fes_status_start_ppdu: TX_FES_STATUS_START_PPDU TLV
 * tx_fes_status_user_ppdu: TX_FES_STATUS_USER_PPDU TLV
 * tx_fes_status_ack_or_ba: TX_FES_STATUS_ACK_OR_BA TLV
 * tx_fes_status_1k_ba: TX_FES_STATUS_1K_BA TLV
 * tx_fes_status_start_prot: TX_FES_STATUS_START_PROTO TLV
 * tx_fes_status_user_response: TX_FES_STATUS_USER_RESPONSE TLV
 * rx_frame_bitmap_ack: RX_FRAME_BITMAP_ACK TLV
 * rx_frame_1k_bitmap_ack: RX_FRAME_1K_BITMAP_ACK TLV
 * coex_tx_status: COEX_TX_STATUS TLV
 * recevied_response_info: RECEIVED_RESPONSE_INFO TLV
 * recevied_response_info_p2: RECEIVED_RESPONSE_INFO_PART2 TLV
 * ofdma_trigger_details: OFDMA_TRIGGER_DETAILS
 * recevied_trigger_info: RECEIVED_TRIGGER_INFO
 * pdg_tx_request: PDG_TX_REQUEST
 * pdg_response: PDG_RESPONSE
 * pdg_trig_response: PDG_TRIG_RESPONSE
 * trigger_response_tx_done: TRIGGER_RESPONSE_TX_DONE
 * prot_tx_end: PROT_TX_END
 * ppdu_tx_end: PPDU_TX_END
 * r2r_status_end: R2R_STATUS_END
 * flush_req: FLUSH_REQ
 * mactx_phy_desc: MACTX_PHY_DESC
 * mactx_user_desc_cmn: MACTX_USER_DESC_COMMON
 * mactx_user_desc_per_usr: MACTX_USER_DESC_PER_USER
 * tqm_acked_1k_mpdu: TQM_ACKED_1K_MPDU
 * tqm_acked_mpdu: TQM_ACKED_MPDU
 * tqm_update_tx_mpdu_count: TQM_UPDATE_TX_MPDU_COUNT
 * phytx_ppdu_header_info_request: PHYTX_PPDU_HEADER_INFO_REQUEST
 * u_sig_eht_su_mu: U_SIG_EHT_SU_MU
 * u_sig_eht_su: U_SIG_EHT_SU
 * eht_sig_usr_su: EHT_SIG_USR_SU
 * eht_sig_usr_mu_mimo: EHT_SIG_USR_MU_MIMO
 * eht_sig_usr_ofdma: EHT_SIG_USR_MU_MIMO
 * he_sig_a_su: HE_SIG_A_SU
 * he_sig_a_mu_dl: HE_SIG_A_MU_DL
 * he_sig_a_mu_ul: HE_SIG_A_MU_UL
 * he_sig_b1_mu: HE_SIG_B1_MU
 * he_sig_b2_mu: HE_SIG_B2_MU
 * he_sig_b2_ofdma: HE_SIG_B2_OFDMA
 * vht_sig_b_mu160: VHT_SIG_B_MU160
 * vht_sig_b_mu80: VHT_SIG_B_MU80
 * vht_sig_b_mu40: VHT_SIG_B_MU40
 * vht_sig_b_mu20: VHT_SIG_B_MU20
 * vht_sig_b_su160: VHT_SIG_B_SU160
 * vht_sig_b_su80: VHT_SIG_B_SU80
 * vht_sig_b_su40: VHT_SIG_B_SU40
 * vht_sig_b_su20: VHT_SIG_B_SU20
 * vht_sig_a: VHT_SIG_A
 * ht_sig: HT_SIG
 * l_sig_b: L_SIG_B
 * l_sig_a: L_SIG_A
 * tx_service: TX_SERVICE
 * txpcu_buf_status: TXPCU_BUFFER_STATUS
 * txpcu_user_buf_status: TXPCU_USER_BUFFER_STATUS
 * txdma_stop_request: TXDMA_STOP_REQUEST
 * expected_response: EXPECTED_RESPONSE
 * tx_mpdu_count_transfer_end: TX_MPDU_COUNT_TRANSFER_END
 * rx_trig_info: RX_TRIG_INFO
 * rxpcu_tx_setup_clear: RXPCU_TX_SETUP_CLEAR
 * rx_frame_bitmap_req: RX_FRAME_BITMAP_REQ
 * rx_phy_sleep: RX_PHY_SLEEP
 * txpcu_preamble_done: TXPCU_PREAMBLE_DONE
 * txpcu_phytx_debug32: TXPCU_PHYTX_DEBUG32
 * txpcu_phytx_other_transmit_info32: TXPCU_PHYTX_OTHER_TRANSMIT_INFO32
 * rx_ppdu_noack_report: RX_PPDU_NO_ACK_REPORT
 * rx_ppdu_ack_report: RX_PPDU_ACK_REPORT
 * coex_rx_status: COEX_RX_STATUS
 * rx_start_param: RX_START_PARAM
 * tx_cbf_info: TX_CBF_INFO
 * rxpcu_early_rx_indication: RXPCU_EARLY_RX_INDICATION
 * received_response_user_7_0: RECEIVED_RESPONSE_USER_7_0
 * received_response_user_15_8: RECEIVED_RESPONSE_USER_15_8
 * received_response_user_23_16: RECEIVED_RESPONSE_USER_23_16
 * received_response_user_31_24: RECEIVED_RESPONSE_USER_31_24
 * received_response_user_36_32: RECEIVED_RESPONSE_USER_36_32
 * rx_pm_info: RX_PM_INFO
 * rx_preamble: RX_PREAMBLE
 * others: OTHERS
 * mactx_pre_phy_desc: MACTX_PRE_PHY_DESC
 *
 */
struct dp_tx_mon_upstream_tlv_config {
	uint32_t rx_response_required_info:1,
		 response_start_status:1,
		 response_end_status:1,
		 tx_fes_status_start:1,
		 tx_fes_status_end:1,
		 tx_fes_status_start_ppdu:1,
		 tx_fes_status_user_ppdu:1,
		 tx_fes_status_ack_or_ba:1,
		 tx_fes_status_1k_ba:1,
		 tx_fes_status_start_prot:1,
		 tx_fes_status_prot:1,
		 tx_fes_status_user_response:1,
		 rx_frame_bitmap_ack:1,
		 rx_frame_1k_bitmap_ack:1,
		 coex_tx_status:1,
		 recevied_response_info:1,
		 recevied_response_info_p2:1,
		 ofdma_trigger_details:1,
		 recevied_trigger_info:1,
		 pdg_tx_request:1,
		 pdg_response:1,
		 pdg_trig_response:1,
		 trigger_response_tx_done:1,
		 prot_tx_end:1,
		 ppdu_tx_end:1,
		 r2r_status_end:1,
		 flush_req:1,
		 mactx_phy_desc:1,
		 mactx_user_desc_cmn:1,
		 mactx_user_desc_per_usr:1;
	uint32_t tqm_acked_1k_mpdu:1,
		 tqm_acked_mpdu:1,
		 tqm_update_tx_mpdu_count:1,
		 phytx_ppdu_header_info_request:1,
		 u_sig_eht_su_mu:1,
		 u_sig_eht_su:1,
		 u_sig_eht_tb:1,
		 eht_sig_usr_su:1,
		 eht_sig_usr_mu_mimo:1,
		 eht_sig_usr_ofdma:1,
		 he_sig_a_su:1,
		 he_sig_a_mu_dl:1,
		 he_sig_a_mu_ul:1,
		 he_sig_b1_mu:1,
		 he_sig_b2_mu:1,
		 he_sig_b2_ofdma:1,
		 vht_sig_b_mu160:1,
		 vht_sig_b_mu80:1,
		 vht_sig_b_mu40:1,
		 vht_sig_b_mu20:1,
		 vht_sig_b_su160:1,
		 vht_sig_b_su80:1,
		 vht_sig_b_su40:1,
		 vht_sig_b_su20:1,
		 vht_sig_a:1,
		 ht_sig:1,
		 l_sig_b:1,
		 l_sig_a:1,
		 tx_service:1;
	uint32_t txpcu_buf_status:1,
		 txpcu_user_buf_status:1,
		 txdma_stop_request:1,
		 expected_response:1,
		 tx_mpdu_count_transfer_end:1,
		 rx_trig_info:1,
		 rxpcu_tx_setup_clear:1,
		 rx_frame_bitmap_req:1,
		 rx_phy_sleep:1,
		 txpcu_preamble_done:1,
		 txpcu_phytx_debug32:1,
		 txpcu_phytx_other_transmit_info32:1,
		 rx_ppdu_noack_report:1,
		 rx_ppdu_ack_report:1,
		 coex_rx_status:1,
		 rx_start_param:1,
		 tx_cbf_info:1,
		 rxpcu_early_rx_indication:1,
		 received_response_user_7_0:1,
		 received_response_user_15_8:1,
		 received_response_user_23_16:1,
		 received_response_user_31_24:1,
		 received_response_user_36_32:1,
		 rx_pm_info:1,
		 rx_preamble:1,
		 others:1,
		 mactx_pre_phy_desc:1;
};

/**
 * struct dp_tx_mon_wordmask_config - Tx monitor word mask
 * tx_fes_setup: TX_FES_SETUP TLV word mask
 * tx_peer_entry: TX_PEER_ENTRY TLV word mask
 * tx_queue_ext: TX_QUEUE_EXTENSION TLV word mask
 * tx_msdu_start: TX_MSDU_START TLV word mask
 * tx_mpdu_start: TX_MPDU_START TLV word mask
 * pcu_ppdu_setup_init: PCU_PPDU_SETUP TLV word mask
 * rxpcu_user_setup: RXPCU_USER_SETUP TLV word mask
 */
struct dp_tx_mon_wordmask_config {
	uint16_t tx_fes_setup;
	uint16_t tx_peer_entry;
	uint16_t tx_queue_ext;
	uint16_t tx_msdu_start;
	uint16_t tx_mpdu_start;
	uint16_t pcu_ppdu_setup_init;
	uint16_t rxpcu_user_setup;
};

/**
 * struct htt_tx_ring_tlv_filter - Tx ring TLV filter
 * enable/disable.
 * @dtlvs: enable/disable downstream TLVs
 * @utlvs: enable/disable upstream TLVs
 * @wmask: enable/disbale word mask subscription
 * @mgmt_filter: enable/disable mgmt packets
 * @data_filter: enable/disable data packets
 * @ctrl_filter: enable/disable ctrl packets
 * @mgmt_dma_length: configure length for mgmt packet
 * @ctrl_dma_length: configure length for ctrl packet
 * @data_dma_length: configure length for data packet
 * @mgmt_mpdu_end: enable mpdu end tlv for mgmt
 * @mgmt_msdu_end: enable msdu end tlv for mgmt
 * @mgmt_msdu_start: enable msdu start tlv for mgmt
 * @mgmt_mpdu_start: enable mpdu start tlv for mgmt
 * @ctrl_mpdu_end: enable mpdu end tlv for ctrl
 * @ctrl_msdu_end: enable msdu end tlv for ctrl
 * @ctrl_msdu_start: enable msdu start tlv for ctrl
 * @ctrl_mpdu_start: enable mpdu start tlv for ctrl
 * @data_mpdu_end: enable mpdu end tlv for data
 * @data_msdu_end: enable msdu end tlv for data
 * @data_msdu_start: enable msdu start tlv for data
 * @data_mpdu_start: enable mpdu start tlv for data
 * @mgmt_mpdu_log: enable mgmt mpdu level logging
 * @ctrl_mpdu_log: enable ctrl mpdu level logging
 * @data_mpdu_log: enable data mpdu level logging
 *
 * NOTE: Do not change the layout of this structure
 */
struct htt_tx_ring_tlv_filter {
	struct dp_tx_mon_downstream_tlv_config dtlvs;
	struct dp_tx_mon_upstream_tlv_config utlvs;
	struct dp_tx_mon_wordmask_config wmask;
	uint16_t mgmt_filter;
	uint16_t data_filter;
	uint16_t ctrl_filter;
	uint16_t mgmt_dma_length:3,
		 ctrl_dma_length:3,
		 data_dma_length:3;
	uint16_t mgmt_mpdu_end:1,
		 mgmt_msdu_end:1,
		 mgmt_msdu_start:1,
		 mgmt_mpdu_start:1,
		 ctrl_mpdu_end:1,
		 ctrl_msdu_end:1,
		 ctrl_msdu_start:1,
		 ctrl_mpdu_start:1,
		 data_mpdu_end:1,
		 data_msdu_end:1,
		 data_msdu_start:1,
		 data_mpdu_start:1;
	uint8_t  mgmt_mpdu_log:1,
		 ctrl_mpdu_log:1,
		 data_mpdu_log:1;
};
#endif /* QCA_MONITOR_2_0_SUPPORT */

/**
 * struct htt_rx_ring_tlv_filter - Rx ring TLV filter
 * enable/disable.
 * @mpdu_start: enable/disable MPDU start TLV
 * @msdu_start: enable/disable MSDU start TLV
 * @packet: enable/disable PACKET TLV
 * @msdu_end: enable/disable MSDU end TLV
 * @mpdu_end: enable/disable MPDU end TLV
 * @packet_header: enable/disable PACKET header TLV
 * @attention: enable/disable ATTENTION TLV
 * @ppdu_start: enable/disable PPDU start TLV
 * @ppdu_end: enable/disable PPDU end TLV
 * @ppdu_end_user_stats: enable/disable PPDU user stats TLV
 * @ppdu_end_user_stats_ext: enable/disable PPDU user stats ext TLV
 * @ppdu_end_status_done: enable/disable PPDU end status done TLV
 * @enable_fp: enable/disable FP packet
 * @enable_md: enable/disable MD packet
 * @enable_mo: enable/disable MO packet
 * @enable_mgmt: enable/disable MGMT packet
 * @enable_ctrl: enable/disable CTRL packet
 * @enable_data: enable/disable DATA packet
 * @offset_valid: Flag to indicate if below offsets are valid
 * @rx_packet_offset: Offset of packet payload
 * @rx_header_offset: Offset of rx_header tlv
 * @rx_mpdu_end_offset: Offset of rx_mpdu_end tlv
 * @rx_mpdu_start_offset: Offset of rx_mpdu_start tlv
 * @rx_msdu_end_offset: Offset of rx_msdu_end tlv
 * @rx_msdu_start_offset: Offset of rx_msdu_start tlv
 * @rx_attn_offset: Offset of rx_attention tlv
 * @rx_mpdu_start_wmask: word mask for mpdu start tlv
 * @rx_mpdu_end_wmask: word mask for mpdu end tlv
 * @rx_msdu_end_tlv: word mask for msdu end tlv
 * @rx_pkt_tlv_offset: rx pkt tlv offset
 * @mgmt_dma_length: configure length for mgmt packet
 * @ctrl_dma_length: configure length for ctrl packet
 * @data_dma_length: configure length for data packet
 * @mgmt_mpdu_log: enable mgmt mpdu level logging
 * @ctrl_mpdu_log: enable ctrl mpdu level logging
 * @data_mpdu_log: enable data mpdu level logging
 *
 * NOTE: Do not change the layout of this structure
 */
struct htt_rx_ring_tlv_filter {
	u_int32_t mpdu_start:1,
		msdu_start:1,
		packet:1,
		msdu_end:1,
		mpdu_end:1,
		packet_header:1,
		attention:1,
		ppdu_start:1,
		ppdu_end:1,
		ppdu_end_user_stats:1,
		ppdu_end_user_stats_ext:1,
		ppdu_end_status_done:1,
		header_per_msdu:1,
		enable_fp:1,
		enable_md:1,
		enable_mo:1;
	u_int32_t fp_mgmt_filter:16,
		mo_mgmt_filter:16;
	u_int32_t fp_ctrl_filter:16,
		mo_ctrl_filter:16;
	u_int32_t fp_data_filter:16,
		mo_data_filter:16;
	u_int16_t md_data_filter;
	u_int16_t md_mgmt_filter;
	u_int16_t md_ctrl_filter;
	bool offset_valid;
	uint16_t rx_packet_offset;
	uint16_t rx_header_offset;
	uint16_t rx_mpdu_end_offset;
	uint16_t rx_mpdu_start_offset;
	uint16_t rx_msdu_end_offset;
	uint16_t rx_msdu_start_offset;
	uint16_t rx_attn_offset;
#ifdef QCA_MONITOR_2_0_SUPPORT
	uint16_t rx_mpdu_start_wmask;
	uint16_t rx_mpdu_end_wmask;
	uint16_t rx_msdu_end_wmask;
	uint16_t rx_pkt_tlv_offset;
	uint16_t mgmt_dma_length:3,
		 ctrl_dma_length:3,
		 data_dma_lepngth:3,
		 mgmt_mpdu_log:1,
		 ctrl_mpdu_log:1,
		 data_mpdu_log:1;
#endif
};

/**
 * struct dp_htt_rx_flow_fst_setup - Rx FST setup message
 * @pdev_id: DP Pdev identifier
 * @max_entries: Size of Rx FST in number of entries
 * @max_search: Number of collisions allowed
 * @base_addr_lo: lower 32-bit physical address
 * @base_addr_hi: upper 32-bit physical address
 * @ip_da_sa_prefix: IPv4 prefix to map to IPv6 address scheme
 * @hash_key_len: Rx FST hash key size
 * @hash_key: Rx FST Toeplitz hash key
 */
struct dp_htt_rx_flow_fst_setup {
	uint8_t pdev_id;
	uint32_t max_entries;
	uint32_t max_search;
	uint32_t base_addr_lo;
	uint32_t base_addr_hi;
	uint32_t ip_da_sa_prefix;
	uint32_t hash_key_len;
	uint8_t *hash_key;
};

/**
 * enum dp_htt_flow_fst_operation - FST related operations allowed
 * @DP_HTT_FST_CACHE_OP_NONE: Cache no-op
 * @DP_HTT_FST_CACHE_INVALIDATE_ENTRY: Invalidate single cache entry
 * @DP_HTT_FST_CACHE_INVALIDATE_FULL: Invalidate entire cache
 * @DP_HTT_FST_ENABLE: Bypass FST is enabled
 * @DP_HTT_FST_DISABLE: Disable bypass FST
 */
enum dp_htt_flow_fst_operation {
	DP_HTT_FST_CACHE_OP_NONE,
	DP_HTT_FST_CACHE_INVALIDATE_ENTRY,
	DP_HTT_FST_CACHE_INVALIDATE_FULL,
	DP_HTT_FST_ENABLE,
	DP_HTT_FST_DISABLE
};

/**
 * struct dp_htt_rx_flow_fst_setup - Rx FST setup message
 * @pdev_id: DP Pdev identifier
 * @op_code: FST operation to be performed by FW/HW
 * @rx_flow: Rx Flow information on which operation is to be performed
 */
struct dp_htt_rx_flow_fst_operation {
	uint8_t pdev_id;
	enum dp_htt_flow_fst_operation op_code;
	struct cdp_rx_flow_info *rx_flow;
};

/**
 * struct dp_htt_rx_fisa_config - Rx fisa config
 * @pdev_id: DP Pdev identifier
 * @fisa_timeout: fisa aggregation timeout
 */
struct dp_htt_rx_fisa_cfg {
	uint8_t pdev_id;
	uint32_t fisa_timeout;
};

/*
 * htt_htc_pkt_alloc() - Allocate HTC packet buffer
 * @htt_soc:	HTT SOC handle
 *
 * Return: Pointer to htc packet buffer
 */
struct dp_htt_htc_pkt *htt_htc_pkt_alloc(struct htt_soc *soc);

/*
 * htt_htc_pkt_free() - Free HTC packet buffer
 * @htt_soc:	HTT SOC handle
 */
void
htt_htc_pkt_free(struct htt_soc *soc, struct dp_htt_htc_pkt *pkt);

#define HTT_HTC_PKT_STATUS_SUCCESS \
	((pkt->htc_pkt.Status != QDF_STATUS_E_CANCELED) && \
	(pkt->htc_pkt.Status != QDF_STATUS_E_RESOURCES))

#ifdef ENABLE_CE4_COMP_DISABLE_HTT_HTC_MISC_LIST

static void
htt_htc_misc_pkt_list_add(struct htt_soc *soc, struct dp_htt_htc_pkt *pkt)
{
}

#else  /* ENABLE_CE4_COMP_DISABLE_HTT_HTC_MISC_LIST */

/*
 * htt_htc_misc_pkt_list_add() - Add pkt to misc list
 * @htt_soc:	HTT SOC handle
 * @dp_htt_htc_pkt: pkt to be added to list
 */
void
htt_htc_misc_pkt_list_add(struct htt_soc *soc, struct dp_htt_htc_pkt *pkt);

#endif  /* ENABLE_CE4_COMP_DISABLE_HTT_HTC_MISC_LIST */

/**
 * DP_HTT_SEND_HTC_PKT() - Send htt packet from host
 * @soc : HTT SOC handle
 * @pkt: pkt to be send
 * @cmd : command to be recorded in dp htt logger
 * @buf : Pointer to buffer needs to be recored for above cmd
 *
 * Return: None
 */
static inline QDF_STATUS DP_HTT_SEND_HTC_PKT(struct htt_soc *soc,
					     struct dp_htt_htc_pkt *pkt,
					     uint8_t cmd, uint8_t *buf)
{
	QDF_STATUS status;

	htt_command_record(soc->htt_logger_handle, cmd, buf);

	status = htc_send_pkt(soc->htc_soc, &pkt->htc_pkt);
	if (status == QDF_STATUS_SUCCESS && HTT_HTC_PKT_STATUS_SUCCESS)
		htt_htc_misc_pkt_list_add(soc, pkt);
	else
		soc->stats.fail_count++;
	return status;
}

QDF_STATUS dp_htt_rx_fisa_config(struct dp_pdev *pdev,
				 struct dp_htt_rx_fisa_cfg *fisa_config);

/*
 * htt_soc_initialize() - SOC level HTT initialization
 * @htt_soc: Opaque htt SOC handle
 * @ctrl_psoc: Opaque ctrl SOC handle
 * @htc_soc: SOC level HTC handle
 * @hal_soc: Opaque HAL SOC handle
 * @osdev: QDF device
 *
 * Return: HTT handle on success; NULL on failure
 */
void *
htt_soc_initialize(struct htt_soc *htt_soc,
		   struct cdp_ctrl_objmgr_psoc *ctrl_psoc,
		   HTC_HANDLE htc_soc,
		   hal_soc_handle_t hal_soc_hdl, qdf_device_t osdev);

/*
 * htt_soc_attach() - attach DP and HTT SOC
 * @soc: DP SOC handle
 * @htc_hdl: HTC handle
 *
 * Return: htt_soc handle on Success, NULL on Failure
 */
struct htt_soc *htt_soc_attach(struct dp_soc *soc, HTC_HANDLE htc_hdl);

/*
 * htt_set_htc_handle_() - set HTC handle
 * @htt_hdl: HTT handle/SOC
 * @htc_soc: HTC handle
 *
 * Return: None
 */
void htt_set_htc_handle(struct htt_soc *htt_hdl, HTC_HANDLE htc_soc);

/*
 * htt_get_htc_handle_() - set HTC handle
 * @htt_hdl: HTT handle/SOC
 *
 * Return: HTC_HANDLE
 */
HTC_HANDLE htt_get_htc_handle(struct htt_soc *htt_hdl);

/*
 * htt_soc_htc_dealloc() - HTC memory de-alloc
 * @htt_soc: SOC level HTT handle
 *
 * Return: None
 */
void htt_soc_htc_dealloc(struct htt_soc *htt_handle);

/*
 * htt_soc_htc_prealloc() - HTC memory prealloc
 * @htt_soc: SOC level HTT handle
 *
 * Return: QDF_STATUS_SUCCESS on success or
 * QDF_STATUS_E_NO_MEM on allocation failure
 */
QDF_STATUS htt_soc_htc_prealloc(struct htt_soc *htt_soc);

void htt_soc_detach(struct htt_soc *soc);

int htt_srng_setup(struct htt_soc *htt_soc, int pdev_id,
		   hal_ring_handle_t hal_ring_hdl,
		   int hal_ring_type);

int htt_soc_attach_target(struct htt_soc *htt_soc);

/*
 * htt_h2t_rx_ring_cfg() - Send SRNG packet and TLV filter
 * config message to target
 * @htt_soc:	HTT SOC handle
 * @pdev_id:	PDEV Id
 * @hal_srng:	Opaque HAL SRNG pointer
 * @hal_ring_type:	SRNG ring type
 * @ring_buf_size:	SRNG buffer size
 * @htt_tlv_filter:	Rx SRNG TLV and filter setting
 *
 * Return: 0 on success; error code on failure
 */
int htt_h2t_rx_ring_cfg(struct htt_soc *htt_soc, int pdev_id,
			hal_ring_handle_t hal_ring_hdl,
			int hal_ring_type, int ring_buf_size,
			struct htt_rx_ring_tlv_filter *htt_tlv_filter);

/*
 * htt_t2h_stats_handler() - target to host stats work handler
 * @context:	context (dp soc context)
 *
 * Return: void
 */
void htt_t2h_stats_handler(void *context);

/**
 * struct htt_stats_context - htt stats information
 * @soc: Size of each descriptor in the pool
 * @msg: T2H Ext stats message queue
 * @msg_len: T2H Ext stats message length
 */
struct htt_stats_context {
	struct dp_soc *soc;
	qdf_nbuf_queue_t msg;
	uint32_t msg_len;
};

/**
 * dp_htt_rx_flow_fst_setup(): Send HTT Rx FST setup message to FW
 * @pdev: DP pdev handle
 * @fse_setup_info: FST setup parameters
 *
 * Return: Success when HTT message is sent, error on failure
 */
QDF_STATUS
dp_htt_rx_flow_fst_setup(struct dp_pdev *pdev,
			 struct dp_htt_rx_flow_fst_setup *setup_info);

/**
 * dp_htt_rx_flow_fse_operation(): Send HTT Flow Search Entry msg to
 * add/del a flow in HW
 * @pdev: DP pdev handle
 * @fse_op_info: Flow entry parameters
 *
 * Return: Success when HTT message is sent, error on failure
 */
QDF_STATUS
dp_htt_rx_flow_fse_operation(struct dp_pdev *pdev,
			     struct dp_htt_rx_flow_fst_operation *op_info);

/**
 * htt_h2t_full_mon_cfg() - Send full monitor configuarion msg to FW
 *
 * @htt_soc: HTT Soc handle
 * @pdev_id: Radio id
 * @dp_full_mon_config: enabled/disable configuration
 *
 * Return: Success when HTT message is sent, error on failure
 */
int htt_h2t_full_mon_cfg(struct htt_soc *htt_soc,
			 uint8_t pdev_id,
			 enum dp_full_mon_config);
#endif /* _DP_HTT_H_ */
