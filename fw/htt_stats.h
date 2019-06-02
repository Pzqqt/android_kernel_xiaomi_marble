/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

/**
 * @file htt_stats.h
 *
 * @details the public header file of HTT STATS
 */
#ifndef __HTT_STATS_H__
#define __HTT_STATS_H__

#include <htt.h>

/*
 * htt_dbg_ext_stats_type -
 * The base structure for each of the stats_type is only for reference
 * Host should use this information to know the type of TLVs to expect
 * for a particular stats type.
 *
 *    Max supported stats :- 256.
 */
enum htt_dbg_ext_stats_type {
    /* HTT_DBG_EXT_STATS_RESET
     * PARAM:
     *   - config_param0 : start_offset (stats type)
     *   - config_param1 : stats bmask from start offset
     *   - config_param2 : stats bmask from start offset + 32
     *   - config_param3 : stats bmask from start offset + 64
     * RESP MSG:
     *   - No response sent.
     */
    HTT_DBG_EXT_STATS_RESET              = 0,

    /* HTT_DBG_EXT_STATS_PDEV_TX
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_pdev_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_TX            = 1,

    /* HTT_DBG_EXT_STATS_PDEV_RX
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_rx_pdev_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_RX            = 2,

    /* HTT_DBG_EXT_STATS_PDEV_TX_HWQ
     * PARAMS:
     *   - config_param0: [Bit31: Bit0] HWQ mask
     * RESP MSG:
     *   - htt_tx_hwq_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_TX_HWQ        = 3,

    /* HTT_DBG_EXT_STATS_PDEV_TX_SCHED
     * PARAMS:
     *   - config_param0: [Bit31: Bit0] TXQ mask
     * RESP MSG:
     *   - htt_stats_tx_sched_t
     */
    HTT_DBG_EXT_STATS_PDEV_TX_SCHED      = 4,

    /* HTT_DBG_EXT_STATS_PDEV_ERROR
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_hw_err_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_ERROR         = 5,

    /* HTT_DBG_EXT_STATS_PDEV_TQM
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_tqm_pdev_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_TQM           = 6,

    /* HTT_DBG_EXT_STATS_TQM_CMDQ
     * PARAMS:
     *   - config_param0:
     *      [Bit15: Bit0 ] cmdq id :if 0xFFFF print all cmdq's
     *      [Bit31: Bit16] reserved
     * RESP MSG:
     *   - htt_tx_tqm_cmdq_stats_t
     */
    HTT_DBG_EXT_STATS_TQM_CMDQ           = 7,

    /* HTT_DBG_EXT_STATS_TX_DE_INFO
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_de_stats_t
     */
    HTT_DBG_EXT_STATS_TX_DE_INFO         = 8,

    /* HTT_DBG_EXT_STATS_PDEV_TX_RATE
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_pdev_rate_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_TX_RATE       = 9,

    /* HTT_DBG_EXT_STATS_PDEV_RX_RATE
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_rx_pdev_rate_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_RX_RATE       = 10,

    /* HTT_DBG_EXT_STATS_PEER_INFO
     * PARAMS:
     *   - config_param0:
     *     [Bit0] - [0] for sw_peer_id, [1] for mac_addr based request
     *     [Bit15 : Bit 1] htt_peer_stats_req_mode_t
     *     [Bit31 : Bit16] sw_peer_id
     *     config_param1:
     *     peer_stats_req_type_mask:32 (enum htt_peer_stats_tlv_enum)
     *           0 bit htt_peer_stats_cmn_tlv
     *           1 bit htt_peer_details_tlv
     *           2 bit htt_tx_peer_rate_stats_tlv
     *           3 bit htt_rx_peer_rate_stats_tlv
     *           4 bit htt_tx_tid_stats_tlv/htt_tx_tid_stats_v1_tlv
     *           5 bit htt_rx_tid_stats_tlv
     *           6 bit htt_msdu_flow_stats_tlv
     *   - config_param2: [Bit31 : Bit0] mac_addr31to0
     *   - config_param3: [Bit15 : Bit0] mac_addr47to32
     *                    [Bit31 : Bit16] reserved
     * RESP MSG:
     *   - htt_peer_stats_t
     */
    HTT_DBG_EXT_STATS_PEER_INFO          = 11,

    /* HTT_DBG_EXT_STATS_TX_SELFGEN_INFO
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_pdev_selfgen_stats_t
     */
    HTT_DBG_EXT_STATS_TX_SELFGEN_INFO    = 12,

    /* HTT_DBG_EXT_STATS_TX_MU_HWQ
     * PARAMS:
     *   - config_param0: [Bit31: Bit0] HWQ mask
     * RESP MSG:
     *   - htt_tx_hwq_mu_mimo_stats_t
     */
    HTT_DBG_EXT_STATS_TX_MU_HWQ          = 13,

    /* HTT_DBG_EXT_STATS_RING_IF_INFO
     * PARAMS:
     *   - config_param0:
     *      [Bit15: Bit0 ] ring id :if 0xFFFF print all rings
     *      [Bit31: Bit16] reserved
     * RESP MSG:
     *   - htt_ring_if_stats_t
     */
    HTT_DBG_EXT_STATS_RING_IF_INFO       = 14,

    /* HTT_DBG_EXT_STATS_SRNG_INFO
     * PARAMS:
     *   - config_param0:
     *      [Bit15: Bit0 ] ring id :if 0xFFFF print all rings
     *      [Bit31: Bit16] reserved
     *   - No Params
     * RESP MSG:
     *   - htt_sring_stats_t
     */
    HTT_DBG_EXT_STATS_SRNG_INFO          = 15,

    /* HTT_DBG_EXT_STATS_SFM_INFO
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_sfm_stats_t
     */
    HTT_DBG_EXT_STATS_SFM_INFO           = 16,

    /* HTT_DBG_EXT_STATS_PDEV_TX_MU
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_pdev_mu_mimo_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_TX_MU         = 17,

    /* HTT_DBG_EXT_STATS_ACTIVE_PEERS_LIST
     * PARAMS:
     *   - config_param0:
     *      [Bit7 : Bit0]   vdev_id:8
     *                      note:0xFF to get all active peers based on pdev_mask.
     *      [Bit31 : Bit8]  rsvd:24
     * RESP MSG:
     *   - htt_active_peer_details_list_t
     */
    HTT_DBG_EXT_STATS_ACTIVE_PEERS_LIST  = 18,

    /* HTT_DBG_EXT_STATS_PDEV_CCA_STATS
     * PARAMS:
     *   - config_param0:
     *      [Bit0] - 1 sec interval histogram
     *      [Bit1] - 100ms interval histogram
     *      [Bit3] - Cumulative CCA stats
     * RESP MSG:
     *   - htt_pdev_cca_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_CCA_STATS     = 19,

    /* HTT_DBG_EXT_STATS_TWT_SESSIONS
     * PARAMS:
     *   - config_param0:
     *      No params
     * RESP MSG:
     *   - htt_pdev_twt_sessions_stats_t
     */
    HTT_DBG_EXT_STATS_TWT_SESSIONS       = 20,

    /* HTT_DBG_EXT_STATS_REO_CNTS
     * PARAMS:
     *   - config_param0:
     *      No params
     * RESP MSG:
     *   - htt_soc_reo_resource_stats_t
     */
    HTT_DBG_EXT_STATS_REO_RESOURCE_STATS = 21,

    /* HTT_DBG_EXT_STATS_TX_SOUNDING_INFO
     * PARAMS:
     *   - config_param0:
     *      [Bit0]          vdev_id_set:1
     *          set to 1 if vdev_id is set and vdev stats are requested
     *      [Bit8 : Bit1]   vdev_id:8
     *          note:0xFF to get all active vdevs based on pdev_mask.
     *      [Bit31 : Bit9]  rsvd:22
     *
     * RESP MSG:
     *   - htt_tx_sounding_stats_t
     */
    HTT_DBG_EXT_STATS_TX_SOUNDING_INFO   = 22,

    /* HTT_DBG_EXT_STATS_PDEV_OBSS_PD_STATS
     * PARAMS:
     *   - config_param0:
     *      No params
     * RESP MSG:
     *   - htt_pdev_obss_pd_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_OBSS_PD_STATS = 23,

    /* HTT_DBG_EXT_STATS_RING_BACKPRESSURE_STATS
     * PARAMS:
     *   - config_param0:
     *      No params
     * RESP MSG:
     *   - htt_stats_ring_backpressure_stats_t
     */
    HTT_DBG_EXT_STATS_RING_BACKPRESSURE_STATS = 24,

    /* HTT_DBG_EXT_STATS_LATENCY_PROF_STATS
     *  PARAMS:
     *
     *  RESP MSG:
     *    - htt_soc_latency_prof_t
     */
    HTT_DBG_EXT_STATS_LATENCY_PROF_STATS = 25,


    /* keep this last */
    HTT_DBG_NUM_EXT_STATS = 256,
};

typedef enum {
    HTT_STATS_TX_PDEV_CMN_TAG                      = 0,  /* htt_tx_pdev_stats_cmn_tlv */
    HTT_STATS_TX_PDEV_UNDERRUN_TAG                 = 1,  /* htt_tx_pdev_stats_urrn_tlv_v */
    HTT_STATS_TX_PDEV_SIFS_TAG                     = 2,  /* htt_tx_pdev_stats_sifs_tlv_v */
    HTT_STATS_TX_PDEV_FLUSH_TAG                    = 3,  /* htt_tx_pdev_stats_flush_tlv_v */
    HTT_STATS_TX_PDEV_PHY_ERR_TAG                  = 4,  /* htt_tx_pdev_stats_phy_err_tlv_v */
    HTT_STATS_STRING_TAG = 5,                            /* htt_stats_string_tlv */
    HTT_STATS_TX_HWQ_CMN_TAG = 6,                        /* htt_tx_hwq_stats_cmn_tlv */
    HTT_STATS_TX_HWQ_DIFS_LATENCY_TAG              = 7,  /* htt_tx_hwq_difs_latency_stats_tlv_v */
    HTT_STATS_TX_HWQ_CMD_RESULT_TAG                = 8,  /* htt_tx_hwq_cmd_result_stats_tlv_v */
    HTT_STATS_TX_HWQ_CMD_STALL_TAG                 = 9,  /* htt_tx_hwq_cmd_stall_stats_tlv_v */
    HTT_STATS_TX_HWQ_FES_STATUS_TAG                = 10, /* htt_tx_hwq_fes_result_stats_tlv_v */
    HTT_STATS_TX_TQM_GEN_MPDU_TAG                  = 11, /* htt_tx_tqm_gen_mpdu_stats_tlv_v */
    HTT_STATS_TX_TQM_LIST_MPDU_TAG                 = 12, /* htt_tx_tqm_list_mpdu_stats_tlv_v */
    HTT_STATS_TX_TQM_LIST_MPDU_CNT_TAG             = 13, /* htt_tx_tqm_list_mpdu_cnt_tlv_v */
    HTT_STATS_TX_TQM_CMN_TAG = 14,                       /* htt_tx_tqm_cmn_stats_tlv */
    HTT_STATS_TX_TQM_PDEV_TAG                      = 15, /* htt_tx_tqm_pdev_stats_tlv_v */
    HTT_STATS_TX_TQM_CMDQ_STATUS_TAG               = 16, /* htt_tx_tqm_cmdq_status_tlv */
    HTT_STATS_TX_DE_EAPOL_PACKETS_TAG              = 17, /* htt_tx_de_eapol_packets_stats_tlv */
    HTT_STATS_TX_DE_CLASSIFY_FAILED_TAG            = 18, /* htt_tx_de_classify_failed_stats_tlv */
    HTT_STATS_TX_DE_CLASSIFY_STATS_TAG             = 19, /* htt_tx_de_classify_stats_tlv */
    HTT_STATS_TX_DE_CLASSIFY_STATUS_TAG            = 20, /* htt_tx_de_classify_status_stats_tlv */
    HTT_STATS_TX_DE_ENQUEUE_PACKETS_TAG            = 21, /* htt_tx_de_enqueue_packets_stats_tlv */
    HTT_STATS_TX_DE_ENQUEUE_DISCARD_TAG            = 22, /* htt_tx_de_enqueue_discard_stats_tlv */
    HTT_STATS_TX_DE_CMN_TAG = 23,                        /* htt_tx_de_cmn_stats_tlv */
    HTT_STATS_RING_IF_TAG = 24,                          /* htt_ring_if_stats_tlv */
    HTT_STATS_TX_PDEV_MU_MIMO_STATS_TAG            = 25, /* htt_tx_pdev_mu_mimo_sch_stats_tlv */
    HTT_STATS_SFM_CMN_TAG = 26,                          /* htt_sfm_cmn_tlv */
    HTT_STATS_SRING_STATS_TAG                      = 27, /* htt_sring_stats_tlv */
    HTT_STATS_RX_PDEV_FW_STATS_TAG                 = 28, /* htt_rx_pdev_fw_stats_tlv */
    HTT_STATS_RX_PDEV_FW_RING_MPDU_ERR_TAG         = 29, /* htt_rx_pdev_fw_ring_mpdu_err_tlv_v */
    HTT_STATS_RX_PDEV_FW_MPDU_DROP_TAG             = 30, /* htt_rx_pdev_fw_mpdu_drop_tlv_v */
    HTT_STATS_RX_SOC_FW_STATS_TAG                  = 31, /* htt_rx_soc_fw_stats_tlv */
    HTT_STATS_RX_SOC_FW_REFILL_RING_EMPTY_TAG      = 32, /* htt_rx_soc_fw_refill_ring_empty_tlv_v */
    HTT_STATS_RX_SOC_FW_REFILL_RING_NUM_REFILL_TAG = 33, /* htt_rx_soc_fw_refill_ring_num_refill_tlv_v */
    HTT_STATS_TX_PDEV_RATE_STATS_TAG               = 34, /* htt_tx_pdev_rate_stats_tlv */
    HTT_STATS_RX_PDEV_RATE_STATS_TAG               = 35, /* htt_rx_pdev_rate_stats_tlv */
    HTT_STATS_TX_PDEV_SCHEDULER_TXQ_STATS_TAG      = 36, /* htt_tx_pdev_stats_sched_per_txq_tlv */
    HTT_STATS_TX_SCHED_CMN_TAG                     = 37, /* htt_stats_tx_sched_cmn_tlv */
    HTT_STATS_TX_PDEV_MUMIMO_MPDU_STATS_TAG        = 38, /* htt_tx_pdev_mu_mimo_mpdu_stats_tlv */
    HTT_STATS_SCHED_TXQ_CMD_POSTED_TAG             = 39, /* htt_sched_txq_cmd_posted_tlv_v */
    HTT_STATS_RING_IF_CMN_TAG                      = 40, /* htt_ring_if_cmn_tlv */
    HTT_STATS_SFM_CLIENT_USER_TAG                  = 41, /* htt_sfm_client_user_tlv_v */
    HTT_STATS_SFM_CLIENT_TAG = 42,                       /* htt_sfm_client_tlv */
    HTT_STATS_TX_TQM_ERROR_STATS_TAG               = 43, /* htt_tx_tqm_error_stats_tlv */
    HTT_STATS_SCHED_TXQ_CMD_REAPED_TAG             = 44, /* htt_sched_txq_cmd_reaped_tlv_v */
    HTT_STATS_SRING_CMN_TAG = 45,                        /* htt_sring_cmn_tlv */
    HTT_STATS_TX_SELFGEN_AC_ERR_STATS_TAG          = 46, /* htt_tx_selfgen_ac_err_stats_tlv */
    HTT_STATS_TX_SELFGEN_CMN_STATS_TAG             = 47, /* htt_tx_selfgen_cmn_stats_tlv */
    HTT_STATS_TX_SELFGEN_AC_STATS_TAG              = 48, /* htt_tx_selfgen_ac_stats_tlv */
    HTT_STATS_TX_SELFGEN_AX_STATS_TAG              = 49, /* htt_tx_selfgen_ax_stats_tlv */
    HTT_STATS_TX_SELFGEN_AX_ERR_STATS_TAG          = 50, /* htt_tx_selfgen_ax_err_stats_tlv */
    HTT_STATS_TX_HWQ_MUMIMO_SCH_STATS_TAG          = 51, /* htt_tx_hwq_mu_mimo_sch_stats_tlv */
    HTT_STATS_TX_HWQ_MUMIMO_MPDU_STATS_TAG         = 52, /* htt_tx_hwq_mu_mimo_mpdu_stats_tlv */
    HTT_STATS_TX_HWQ_MUMIMO_CMN_STATS_TAG          = 53, /* htt_tx_hwq_mu_mimo_cmn_stats_tlv */
    HTT_STATS_HW_INTR_MISC_TAG                     = 54, /* htt_hw_stats_intr_misc_tlv */
    HTT_STATS_HW_WD_TIMEOUT_TAG                    = 55, /* htt_hw_stats_wd_timeout_tlv */
    HTT_STATS_HW_PDEV_ERRS_TAG                     = 56, /* htt_hw_stats_pdev_errs_tlv */
    HTT_STATS_COUNTER_NAME_TAG                     = 57, /* htt_counter_tlv */
    HTT_STATS_TX_TID_DETAILS_TAG                   = 58, /* htt_tx_tid_stats_tlv */
    HTT_STATS_RX_TID_DETAILS_TAG                   = 59, /* htt_rx_tid_stats_tlv */
    HTT_STATS_PEER_STATS_CMN_TAG                   = 60, /* htt_peer_stats_cmn_tlv */
    HTT_STATS_PEER_DETAILS_TAG                     = 61, /* htt_peer_details_tlv */
    HTT_STATS_PEER_TX_RATE_STATS_TAG               = 62, /* htt_tx_peer_rate_stats_tlv */
    HTT_STATS_PEER_RX_RATE_STATS_TAG               = 63, /* htt_rx_peer_rate_stats_tlv */
    HTT_STATS_PEER_MSDU_FLOWQ_TAG                  = 64, /* htt_msdu_flow_stats_tlv */
    HTT_STATS_TX_DE_COMPL_STATS_TAG                = 65, /* htt_tx_de_compl_stats_tlv */
    HTT_STATS_WHAL_TX_TAG = 66,                          /* htt_hw_stats_whal_tx_tlv */
    HTT_STATS_TX_PDEV_SIFS_HIST_TAG                = 67, /* htt_tx_pdev_stats_sifs_hist_tlv_v */
    HTT_STATS_RX_PDEV_FW_STATS_PHY_ERR_TAG         = 68, /* htt_rx_pdev_fw_stats_phy_err_tlv */
    HTT_STATS_TX_TID_DETAILS_V1_TAG                = 69, /* htt_tx_tid_stats_v1_tlv */
    HTT_STATS_PDEV_CCA_1SEC_HIST_TAG               = 70, /* htt_pdev_cca_stats_hist_tlv (for 1 sec interval stats) */
    HTT_STATS_PDEV_CCA_100MSEC_HIST_TAG            = 71, /* htt_pdev_cca_stats_hist_tlv (for 100 msec interval stats) */
    HTT_STATS_PDEV_CCA_STAT_CUMULATIVE_TAG         = 72, /* htt_pdev_stats_cca_stats_tlv */
    HTT_STATS_PDEV_CCA_COUNTERS_TAG                = 73, /* htt_pdev_stats_cca_counters_tlv */
    HTT_STATS_TX_PDEV_MPDU_STATS_TAG               = 74, /* htt_tx_pdev_mpdu_stats_tlv */
    HTT_STATS_PDEV_TWT_SESSIONS_TAG                = 75, /* htt_pdev_stats_twt_sessions_tlv */
    HTT_STATS_PDEV_TWT_SESSION_TAG                 = 76, /* htt_pdev_stats_twt_session_tlv */
    HTT_STATS_RX_REFILL_RXDMA_ERR_TAG              = 77, /* htt_rx_soc_fw_refill_ring_num_rxdma_err_tlv_v */
    HTT_STATS_RX_REFILL_REO_ERR_TAG                = 78, /* htt_rx_soc_fw_refill_ring_num_reo_err_tlv_v */
    HTT_STATS_RX_REO_RESOURCE_STATS_TAG            = 79, /* htt_rx_reo_debug_stats_tlv_v */
    HTT_STATS_TX_SOUNDING_STATS_TAG                = 80, /* htt_tx_sounding_stats_tlv */
    HTT_STATS_TX_PDEV_TX_PPDU_STATS_TAG            = 81, /* htt_tx_pdev_stats_tx_ppdu_stats_tlv_v */
    HTT_STATS_TX_PDEV_TRIED_MPDU_CNT_HIST_TAG      = 82, /* htt_tx_pdev_stats_tried_mpdu_cnt_hist_tlv_v */
    HTT_STATS_TX_HWQ_TRIED_MPDU_CNT_HIST_TAG       = 83, /* htt_tx_hwq_tried_mpdu_cnt_hist_tlv_v */
    HTT_STATS_TX_HWQ_TXOP_USED_CNT_HIST_TAG        = 84, /* htt_tx_hwq_txop_used_cnt_hist_tlv_v */
    HTT_STATS_TX_DE_FW2WBM_RING_FULL_HIST_TAG      = 85, /* htt_tx_de_fw2wbm_ring_full_hist_tlv */
    HTT_STATS_SCHED_TXQ_SCHED_ORDER_SU_TAG         = 86, /* htt_sched_txq_sched_order_su_tlv */
    HTT_STATS_SCHED_TXQ_SCHED_INELIGIBILITY_TAG    = 87, /* htt_sched_txq_sched_eligibility_tlv */
    HTT_STATS_PDEV_OBSS_PD_TAG                     = 88, /* htt_pdev_obss_pd_stats_tlv */
    HTT_STATS_HW_WAR_TAG                           = 89, /* htt_hw_war_stats_tlv */
    HTT_STATS_RING_BACKPRESSURE_STATS_TAG          = 90, /* htt_ring_backpressure_stats_tlv */
    HTT_STATS_LATENCY_PROF_STATS_TAG               = 91, /* htt_latency_prof_stats_tlv */
    HTT_STATS_LATENCY_CTX_TAG                      = 92, /* htt_latency_prof_ctx_tlv */
    HTT_STATS_LATENCY_CNT_TAG                      = 93, /* htt_latency_prof_cnt_tlv */

    HTT_STATS_MAX_TAG,
} htt_tlv_tag_t;

#define HTT_STATS_TLV_TAG_M 0x00000fff
#define HTT_STATS_TLV_TAG_S 0
#define HTT_STATS_TLV_LENGTH_M 0x00fff000
#define HTT_STATS_TLV_LENGTH_S 12

#define HTT_STATS_TLV_TAG_GET(_var) \
    (((_var) & HTT_STATS_TLV_TAG_M) >> \
     HTT_STATS_TLV_TAG_S)

#define HTT_STATS_TLV_TAG_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_STATS_TLV_TAG, _val); \
        ((_var) |= ((_val) << HTT_STATS_TLV_TAG_S)); \
    } while (0)

#define HTT_STATS_TLV_LENGTH_GET(_var) \
    (((_var) & HTT_STATS_TLV_LENGTH_M) >> \
     HTT_STATS_TLV_LENGTH_S)
#define HTT_STATS_TLV_LENGTH_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_STATS_TLV_LENGTH, _val); \
        ((_var) |= ((_val) << HTT_STATS_TLV_LENGTH_S)); \
    } while (0)

typedef struct {
    union {
        /* BIT [11 :  0]   :- tag
         * BIT [23 : 12]   :- length
         * BIT [31 : 24]   :- reserved
         */
        A_UINT32 tag__length;
        /*
         * The following struct is not endian-portable.
         * It is suitable for use within the target, which is known to be
         * little-endian.
         * The host should use the above endian-portable macros to access
         * the tag and length bitfields in an endian-neutral manner.
         */
        struct {
            A_UINT32 tag      :      12, /* BIT [11 :  0] */
                     length   :   12,    /* BIT [23 : 12] */
                     reserved :  8;      /* BIT [31 : 24] */
        };
    };
} htt_tlv_hdr_t;

#define HTT_STATS_MAX_STRING_SZ32 4
#define HTT_STATS_MACID_INVALID 0xff
#define HTT_TX_HWQ_MAX_DIFS_LATENCY_BINS 10
#define HTT_TX_HWQ_MAX_CMD_RESULT_STATS 13
#define HTT_TX_HWQ_MAX_CMD_STALL_STATS 5
#define HTT_TX_HWQ_MAX_FES_RESULT_STATS 10

typedef enum {
    HTT_STATS_TX_PDEV_NO_DATA_UNDERRUN = 0,
    HTT_STATS_TX_PDEV_DATA_UNDERRUN_BETWEEN_MPDU = 1,
    HTT_STATS_TX_PDEV_DATA_UNDERRUN_WITHIN_MPDU = 2,
    HTT_TX_PDEV_MAX_URRN_STATS = 3,
} htt_tx_pdev_underrun_enum;

#define HTT_TX_PDEV_MAX_FLUSH_REASON_STATS 71
#define HTT_TX_PDEV_MAX_SIFS_BURST_STATS 9
#define HTT_TX_PDEV_MAX_SIFS_BURST_HIST_STATS 10
#define HTT_TX_PDEV_MAX_PHY_ERR_STATS 18
#define HTT_TX_PDEV_SCHED_TX_MODE_MAX 4
#define HTT_TX_PDEV_NUM_SCHED_ORDER_LOG 20

#define HTT_RX_STATS_REFILL_MAX_RING 4
#define HTT_RX_STATS_RXDMA_MAX_ERR 16
#define HTT_RX_STATS_FW_DROP_REASON_MAX 16

/* Bytes stored in little endian order */
/* Length should be multiple of DWORD */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      data[1]; /* Can be variable length */
} htt_stats_string_tlv;

#define HTT_TX_PDEV_STATS_CMN_MAC_ID_M 0x000000ff
#define HTT_TX_PDEV_STATS_CMN_MAC_ID_S 0

#define HTT_TX_PDEV_STATS_CMN_MAC_ID_GET(_var) \
    (((_var) & HTT_TX_PDEV_STATS_CMN_MAC_ID_M) >> \
     HTT_TX_PDEV_STATS_CMN_MAC_ID_S)

#define HTT_TX_PDEV_STATS_CMN_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_PDEV_STATS_CMN_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_PDEV_STATS_CMN_MAC_ID_S)); \
    } while (0)

/* == TX PDEV STATS == */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /* Num queued to HW */
    A_UINT32 hw_queued;
    /* Num PPDU reaped from HW */
    A_UINT32 hw_reaped;
    /* Num underruns */
    A_UINT32 underrun;
    /* Num HW Paused counter. */
    A_UINT32 hw_paused;
    /* Num HW flush counter. */
    A_UINT32 hw_flush;
    /* Num HW filtered counter. */
    A_UINT32 hw_filt;
    /* Num PPDUs cleaned up in TX abort */
    A_UINT32 tx_abort;
    /* Num MPDUs requed by SW */
    A_UINT32 mpdu_requed;
    /* excessive retries */
    A_UINT32 tx_xretry;
    /* Last used data hw rate code */
    A_UINT32 data_rc;
    /* frames dropped due to excessive sw retries */
    A_UINT32 mpdu_dropped_xretry;
    /* illegal rate phy errors  */
    A_UINT32 illgl_rate_phy_err;
    /* wal pdev continous xretry */
    A_UINT32 cont_xretry;
    /* wal pdev tx timeout */
    A_UINT32 tx_timeout;
    /* wal pdev resets  */
    A_UINT32 pdev_resets;
    /* PhY/BB underrun */
    A_UINT32 phy_underrun;
    /* MPDU is more than txop limit */
    A_UINT32 txop_ovf;
    /* Number of Sequences posted */
    A_UINT32 seq_posted;
    /* Number of Sequences failed queueing */
    A_UINT32 seq_failed_queueing;
    /* Number of Sequences completed */
    A_UINT32 seq_completed;
    /* Number of Sequences restarted */
    A_UINT32 seq_restarted;
    /* Number of MU Sequences posted */
    A_UINT32 mu_seq_posted;
    /* Number of time HW ring is paused between seq switch within ISR */
    A_UINT32 seq_switch_hw_paused;
    /* Number of times seq continuation in DSR */
    A_UINT32 next_seq_posted_dsr;
    /* Number of times seq continuation in ISR */
    A_UINT32 seq_posted_isr;
    /* Number of seq_ctrl cached. */
    A_UINT32 seq_ctrl_cached;
    /* Number of MPDUs successfully transmitted */
    A_UINT32 mpdu_count_tqm;
    /* Number of MSDUs successfully transmitted */
    A_UINT32 msdu_count_tqm;
    /* Number of MPDUs dropped */
    A_UINT32 mpdu_removed_tqm;
    /* Number of MSDUs dropped */
    A_UINT32 msdu_removed_tqm;
    /* Num MPDUs flushed by SW, HWPAUSED, SW TXABORT (Reset,channel change) */
    A_UINT32 mpdus_sw_flush;
    /* Num MPDUs filtered by HW, all filter condition (TTL expired) */
    A_UINT32 mpdus_hw_filter;
    /* Num MPDUs truncated by PDG (TXOP, TBTT, PPDU_duration based on rate, dyn_bw) */
    A_UINT32 mpdus_truncated;
    /* Num MPDUs that was tried but didn't receive ACK or BA */
    A_UINT32 mpdus_ack_failed;
    /* Num MPDUs that was dropped due to expiry (MSDU TTL). */
    A_UINT32 mpdus_expired;
    /* Num MPDUs that was retried within seq_ctrl (MGMT/LEGACY) */
    A_UINT32 mpdus_seq_hw_retry;
    /* Num of TQM acked cmds processed */
    A_UINT32 ack_tlv_proc;
    /* coex_abort_mpdu_cnt valid. */
    A_UINT32 coex_abort_mpdu_cnt_valid;
    /* coex_abort_mpdu_cnt from TX FES stats. */
    A_UINT32 coex_abort_mpdu_cnt;
    /* Number of total PPDUs(DATA, MGMT, excludes selfgen) tried over the air (OTA) */
    A_UINT32 num_total_ppdus_tried_ota;
    /* Number of data PPDUs tried over the air (OTA) */
    A_UINT32 num_data_ppdus_tried_ota;
    /* Num Local control/mgmt frames (MSDUs) queued */
    A_UINT32 local_ctrl_mgmt_enqued;
    /* local_ctrl_mgmt_freed:
     * Num Local control/mgmt frames (MSDUs) done
     * It includes all local ctrl/mgmt completions
     * (acked, no ack, flush, TTL, etc)
     */
    A_UINT32 local_ctrl_mgmt_freed;
    /* Num Local data frames (MSDUs) queued */
    A_UINT32 local_data_enqued;
    /* local_data_freed:
     * Num Local data frames (MSDUs) done
     * It includes all local data completions
     * (acked, no ack, flush, TTL, etc)
     */
    A_UINT32 local_data_freed;

    /* Num MPDUs tried by SW */
    A_UINT32 mpdu_tried;
    /* Num of waiting seq posted in isr completion handler */
    A_UINT32 isr_wait_seq_posted;

    A_UINT32 tx_active_dur_us_low;
    A_UINT32 tx_active_dur_us_high;
    /* Number of MPDUs dropped after max retries */
    A_UINT32 remove_mpdus_max_retries;
    /* Num HTT cookies dispatched */
    A_UINT32 comp_delivered;
    /* successful ppdu transmissions */
    A_UINT32 ppdu_ok;
    /* Scheduler self triggers */
    A_UINT32 self_triggers;
    /* FES duration of last tx data PPDU in us (sch_eval_end - ppdu_start) */
    A_UINT32 tx_time_dur_data;
    /* Num of times sequence terminated due to ppdu duration < burst limit */
    A_UINT32 seq_qdepth_repost_stop;
    /* Num of times MU sequence terminated due to MSDUs reaching threshold */
    A_UINT32 mu_seq_min_msdu_repost_stop;
    /* Num of times SU sequence terminated due to MSDUs reaching threshold */
    A_UINT32 seq_min_msdu_repost_stop;
    /* Num of times sequence terminated due to no TXOP available */
    A_UINT32 seq_txop_repost_stop;
    /* Num of times the next sequence got cancelled */
    A_UINT32 next_seq_cancel;
    /* Num of times fes offset was misaligned */
    A_UINT32 fes_offsets_err_cnt;
} htt_tx_pdev_stats_cmn_tlv;

#define HTT_TX_PDEV_STATS_URRN_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      urrn_stats[1]; /* HTT_TX_PDEV_MAX_URRN_STATS */
} htt_tx_pdev_stats_urrn_tlv_v;

#define HTT_TX_PDEV_STATS_FLUSH_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      flush_errs[1]; /* HTT_TX_PDEV_MAX_FLUSH_REASON_STATS */
} htt_tx_pdev_stats_flush_tlv_v;

#define HTT_TX_PDEV_STATS_SIFS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      sifs_status[1]; /* HTT_TX_PDEV_MAX_SIFS_BURST_STATS */
} htt_tx_pdev_stats_sifs_tlv_v;

#define HTT_TX_PDEV_STATS_PHY_ERR_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      phy_errs[1]; /* HTT_TX_PDEV_MAX_PHY_ERR_STATS */
} htt_tx_pdev_stats_phy_err_tlv_v;

#define HTT_TX_PDEV_STATS_SIFS_HIST_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      sifs_hist_status[1]; /* HTT_TX_PDEV_SIFS_BURST_HIST_STATS */
} htt_tx_pdev_stats_sifs_hist_tlv_v;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      num_data_ppdus_legacy_su;
    A_UINT32      num_data_ppdus_ac_su;
    A_UINT32      num_data_ppdus_ax_su;
    A_UINT32      num_data_ppdus_ac_su_txbf;
    A_UINT32      num_data_ppdus_ax_su_txbf;
} htt_tx_pdev_stats_tx_ppdu_stats_tlv_v;

#define HTT_TX_PDEV_STATS_TRIED_MPDU_CNT_HIST_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size .
 *
 *  Tried_mpdu_cnt_hist is the histogram of MPDUs tries per HWQ.
 *  The tries here is the count of the  MPDUS within a PPDU that the
 *  HW had attempted to transmit on  air, for the HWSCH Schedule
 *  command submitted by FW.It is not the retry attempts.
 *  The histogram bins are  0-29, 30-59, 60-89 and so on. The are
 *   10 bins in this histogram. They are defined in FW using the
 *  following macros
 *  #define WAL_MAX_TRIED_MPDU_CNT_HISTOGRAM 9
 *  #define WAL_TRIED_MPDU_CNT_HISTOGRAM_INTERVAL 30
 *
 */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      hist_bin_size;
    A_UINT32      tried_mpdu_cnt_hist[1]; /* HTT_TX_PDEV_TRIED_MPDU_CNT_HIST */
} htt_tx_pdev_stats_tried_mpdu_cnt_hist_tlv_v;

/* STATS_TYPE: HTT_DBG_EXT_STATS_PDEV_TX
 * TLV_TAGS:
 *      - HTT_STATS_TX_PDEV_CMN_TAG
 *      - HTT_STATS_TX_PDEV_URRN_TAG
 *      - HTT_STATS_TX_PDEV_SIFS_TAG
 *      - HTT_STATS_TX_PDEV_FLUSH_TAG
 *      - HTT_STATS_TX_PDEV_PHY_ERR_TAG
 *      - HTT_STATS_TX_PDEV_SIFS_HIST_TAG
 *      - HTT_STATS_TX_PDEV_TX_PPDU_STATS_TAG
 *      - HTT_STATS_TX_PDEV_TRIED_MPDU_CNT_HIST_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct _htt_tx_pdev_stats {
    htt_tx_pdev_stats_cmn_tlv                   cmn_tlv;
    htt_tx_pdev_stats_urrn_tlv_v                underrun_tlv;
    htt_tx_pdev_stats_sifs_tlv_v                sifs_tlv;
    htt_tx_pdev_stats_flush_tlv_v               flush_tlv;
    htt_tx_pdev_stats_phy_err_tlv_v             phy_err_tlv;
    htt_tx_pdev_stats_sifs_hist_tlv_v           sifs_hist_tlv;
    htt_tx_pdev_stats_tx_ppdu_stats_tlv_v       tx_su_tlv;
    htt_tx_pdev_stats_tried_mpdu_cnt_hist_tlv_v tried_mpdu_cnt_hist_tlv;
} htt_tx_pdev_stats_t;

/* == SOC ERROR STATS == */

/* =============== PDEV ERROR STATS ============== */
#define HTT_STATS_MAX_HW_INTR_NAME_LEN 8
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Stored as little endian */
    A_UINT8  hw_intr_name[HTT_STATS_MAX_HW_INTR_NAME_LEN];
    A_UINT32 mask;
    A_UINT32 count;
} htt_hw_stats_intr_misc_tlv;

#define HTT_STATS_MAX_HW_MODULE_NAME_LEN 8
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Stored as little endian */
    A_UINT8  hw_module_name[HTT_STATS_MAX_HW_MODULE_NAME_LEN];
    A_UINT32 count;
} htt_hw_stats_wd_timeout_tlv;

#define HTT_HW_STATS_PDEV_ERRS_MAC_ID_M 0x000000ff
#define HTT_HW_STATS_PDEV_ERRS_MAC_ID_S 0

#define HTT_HW_STATS_PDEV_ERRS_MAC_ID_GET(_var) \
    (((_var) & HTT_HW_STATS_PDEV_ERRS_MAC_ID_M) >> \
     HTT_HW_STATS_PDEV_ERRS_MAC_ID_S)

#define HTT_HW_STATS_PDEV_ERRS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_HW_STATS_PDEV_ERRS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_HW_STATS_PDEV_ERRS_MAC_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    A_UINT32 tx_abort;
    A_UINT32 tx_abort_fail_count;
    A_UINT32 rx_abort;
    A_UINT32 rx_abort_fail_count;
    A_UINT32 warm_reset;
    A_UINT32 cold_reset;
    A_UINT32 tx_flush;
    A_UINT32 tx_glb_reset;
    A_UINT32 tx_txq_reset;
    A_UINT32 rx_timeout_reset;
    A_UINT32 mac_cold_reset_restore_cal;
    A_UINT32 mac_cold_reset;
    A_UINT32 mac_warm_reset;
    A_UINT32 mac_only_reset;
    A_UINT32 phy_warm_reset;
    A_UINT32 phy_warm_reset_ucode_trig;
    A_UINT32 mac_warm_reset_restore_cal;
    A_UINT32 mac_sfm_reset;
    A_UINT32 phy_warm_reset_m3_ssr;
    A_UINT32 phy_warm_reset_reason_phy_m3;
    A_UINT32 phy_warm_reset_reason_tx_hw_stuck;
    A_UINT32 phy_warm_reset_reason_num_cca_rx_frame_stuck;
    A_UINT32 phy_warm_reset_reason_wal_rx_recovery_rst_rx_busy;
    A_UINT32 phy_warm_reset_reason_wal_rx_recovery_rst_mac_hang;
    A_UINT32 phy_warm_reset_reason_mac_reset_converted_phy_reset;

    A_UINT32 wal_rx_recovery_rst_mac_hang_count;
    A_UINT32 wal_rx_recovery_rst_known_sig_count;
    A_UINT32 wal_rx_recovery_rst_no_rx_count;
    A_UINT32 wal_rx_recovery_rst_no_rx_consecutive_count;
    A_UINT32 wal_rx_recovery_rst_rx_busy_count;
    A_UINT32 wal_rx_recovery_rst_phy_mac_hang_count;
    A_UINT32 rx_flush_cnt; /* Num rx flush issued */
} htt_hw_stats_pdev_errs_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    A_UINT32 last_unpause_ppdu_id;
    A_UINT32 hwsch_unpause_wait_tqm_write;
    A_UINT32 hwsch_dummy_tlv_skipped;
    A_UINT32 hwsch_misaligned_offset_received;
    A_UINT32 hwsch_reset_count;
    A_UINT32 hwsch_dev_reset_war;
    A_UINT32 hwsch_delayed_pause;
    A_UINT32 hwsch_long_delayed_pause;
    A_UINT32 sch_rx_ppdu_no_response;
    A_UINT32 sch_selfgen_response;
    A_UINT32 sch_rx_sifs_resp_trigger;
} htt_hw_stats_whal_tx_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    union {
        struct {
            A_UINT32 mac_id:    8,
                     reserved: 24;
        };
        A_UINT32 mac_id__word;
    };

    /*
     * hw_wars is a variable-length array, with each element counting
     * the number of occurrences of the corresponding type of HW WAR.
     * That is, hw_wars[0] indicates how many times HW WAR 0 occurred,
     * hw_wars[1] indicates how many times HW WAR 1 occurred, etc.
     * The target has an internal HW WAR mapping that it uses to keep
     * track of which HW WAR is WAR 0, which HW WAR is WAR 1, etc.
     */
    A_UINT32 hw_wars[1/*or more*/];
} htt_hw_war_stats_tlv;

/* STATS_TYPE: HTT_DBG_EXT_STATS_PDEV_ERROR
 * TLV_TAGS:
 *     - HTT_STATS_HW_PDEV_ERRS_TAG
 *     - HTT_STATS_HW_INTR_MISC_TAG (multiple)
 *     - HTT_STATS_HW_WD_TIMEOUT_TAG (multiple)
 *     - HTT_STATS_WHAL_TX_TAG
 *     - HTT_STATS_HW_WAR_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct _htt_pdev_err_stats {
    htt_hw_stats_pdev_errs_tlv  pdev_errs;
    htt_hw_stats_intr_misc_tlv  misc_stats[1];
    htt_hw_stats_wd_timeout_tlv wd_timeout[1];
    htt_hw_stats_whal_tx_tlv    whal_tx_stats;
    htt_hw_war_stats_tlv        hw_war;
} htt_hw_err_stats_t;

/* ============ PEER STATS ============ */

#define HTT_MSDU_FLOW_STATS_TX_FLOW_NUM_M 0x0000ffff
#define HTT_MSDU_FLOW_STATS_TX_FLOW_NUM_S 0
#define HTT_MSDU_FLOW_STATS_TID_NUM_M 0x000f0000
#define HTT_MSDU_FLOW_STATS_TID_NUM_S 16
#define HTT_MSDU_FLOW_STATS_DROP_M 0x00100000
#define HTT_MSDU_FLOW_STATS_DROP_S 20

#define HTT_MSDU_FLOW_STATS_TX_FLOW_NUM_GET(_var) \
    (((_var) & HTT_MSDU_FLOW_STATS_TX_FLOW_NUM_M) >> \
     HTT_MSDU_FLOW_STATS_TX_FLOW_NUM_S)

#define HTT_MSDU_FLOW_STATS_TX_FLOW_NUM_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_MSDU_FLOW_STATS_TX_FLOW_NUM, _val); \
        ((_var) |= ((_val) << HTT_MSDU_FLOW_STATS_TX_FLOW_NUM_S)); \
    } while (0)

#define HTT_MSDU_FLOW_STATS_TID_NUM_GET(_var) \
    (((_var) & HTT_MSDU_FLOW_STATS_TID_NUM_M) >> \
     HTT_MSDU_FLOW_STATS_TID_NUM_S)

#define HTT_MSDU_FLOW_STATS_TID_NUM_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_MSDU_FLOW_STATS_TID_NUM, _val); \
        ((_var) |= ((_val) << HTT_MSDU_FLOW_STATS_TID_NUM_S)); \
    } while (0)

#define HTT_MSDU_FLOW_STATS_DROP_GET(_var) \
    (((_var) & HTT_MSDU_FLOW_STATS_DROP_M) >> \
     HTT_MSDU_FLOW_STATS_DROP_S)

#define HTT_MSDU_FLOW_STATS_DROP_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_MSDU_FLOW_STATS_DROP, _val); \
        ((_var) |= ((_val) << HTT_MSDU_FLOW_STATS_DROP_S)); \
    } while (0)

typedef struct _htt_msdu_flow_stats_tlv {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 last_update_timestamp;
    A_UINT32 last_add_timestamp;
    A_UINT32 last_remove_timestamp;
    A_UINT32 total_processed_msdu_count;
    A_UINT32 cur_msdu_count_in_flowq;
    A_UINT32 sw_peer_id; /* This will help to find which peer_id is stuck state */
    /* BIT [15 :  0]   :- tx_flow_number
     * BIT [19 : 16]   :- tid_num
     * BIT [20 : 20]   :- drop_rule
     * BIT [31 : 21]   :- reserved
     */
    A_UINT32 tx_flow_no__tid_num__drop_rule;
    A_UINT32 last_cycle_enqueue_count;
    A_UINT32 last_cycle_dequeue_count;
    A_UINT32 last_cycle_drop_count;
    /* BIT [15 :  0]   :- current_drop_th
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 current_drop_th;
} htt_msdu_flow_stats_tlv;

#define MAX_HTT_TID_NAME 8

/* DWORD sw_peer_id__tid_num */
#define HTT_TX_TID_STATS_SW_PEER_ID_M 0x0000ffff
#define HTT_TX_TID_STATS_SW_PEER_ID_S 0
#define HTT_TX_TID_STATS_TID_NUM_M 0xffff0000
#define HTT_TX_TID_STATS_TID_NUM_S 16

#define HTT_TX_TID_STATS_SW_PEER_ID_GET(_var) \
    (((_var) & HTT_TX_TID_STATS_SW_PEER_ID_M) >> \
     HTT_TX_TID_STATS_SW_PEER_ID_S)

#define HTT_TX_TID_STATS_SW_PEER_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_TID_STATS_SW_PEER_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_TID_STATS_SW_PEER_ID_S)); \
    } while (0)

#define HTT_TX_TID_STATS_TID_NUM_GET(_var) \
    (((_var) & HTT_TX_TID_STATS_TID_NUM_M) >> \
     HTT_TX_TID_STATS_TID_NUM_S)

#define HTT_TX_TID_STATS_TID_NUM_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_TID_STATS_TID_NUM, _val); \
        ((_var) |= ((_val) << HTT_TX_TID_STATS_TID_NUM_S)); \
    } while (0)

/* DWORD num_sched_pending__num_ppdu_in_hwq */
#define HTT_TX_TID_STATS_NUM_SCHED_PENDING_M 0x000000ff
#define HTT_TX_TID_STATS_NUM_SCHED_PENDING_S 0
#define HTT_TX_TID_STATS_NUM_PPDU_IN_HWQ_M 0x0000ff00
#define HTT_TX_TID_STATS_NUM_PPDU_IN_HWQ_S 8

#define HTT_TX_TID_STATS_NUM_SCHED_PENDING_GET(_var) \
    (((_var) & HTT_TX_TID_STATS_NUM_SCHED_PENDING_M) >> \
     HTT_TX_TID_STATS_NUM_SCHED_PENDING_S)

#define HTT_TX_TID_STATS_NUM_SCHED_PENDING_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_TID_STATS_NUM_SCHED_PENDING, _val); \
        ((_var) |= ((_val) << HTT_TX_TID_STATS_NUM_SCHED_PENDING_S)); \
    } while (0)

#define HTT_TX_TID_STATS_NUM_PPDU_IN_HWQ_GET(_var) \
    (((_var) & HTT_TX_TID_STATS_NUM_PPDU_IN_HWQ_M) >> \
     HTT_TX_TID_STATS_NUM_PPDU_IN_HWQ_S)

#define HTT_TX_TID_STATS_NUM_PPDU_IN_HWQ_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_TID_STATS_NUM_PPDU_IN_HWQ, _val); \
        ((_var) |= ((_val) << HTT_TX_TID_STATS_NUM_PPDU_IN_HWQ_S)); \
    } while (0)

/* Tidq stats */
typedef struct _htt_tx_tid_stats_tlv {
    htt_tlv_hdr_t tlv_hdr;

    /* Stored as little endian */
    A_UINT8 tid_name[MAX_HTT_TID_NAME];
    /* BIT [15 :  0]   :- sw_peer_id
     * BIT [31 : 16]   :- tid_num
     */
    A_UINT32 sw_peer_id__tid_num;
    /* BIT [ 7 :  0]   :- num_sched_pending
     * BIT [15 :  8]   :- num_ppdu_in_hwq
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 num_sched_pending__num_ppdu_in_hwq;
    A_UINT32 tid_flags;
    /* per tid # of hw_queued ppdu.*/
    A_UINT32 hw_queued;
    /* number of per tid successful PPDU. */
    A_UINT32 hw_reaped;
    /* per tid Num MPDUs filtered by HW */
    A_UINT32 mpdus_hw_filter;

    A_UINT32 qdepth_bytes;
    A_UINT32 qdepth_num_msdu;
    A_UINT32 qdepth_num_mpdu;
    A_UINT32 last_scheduled_tsmp;
    A_UINT32 pause_module_id;
    A_UINT32 block_module_id;
    /* tid tx airtime in sec */
    A_UINT32 tid_tx_airtime;
} htt_tx_tid_stats_tlv;

/* Tidq stats */
typedef struct _htt_tx_tid_stats_v1_tlv {
    htt_tlv_hdr_t tlv_hdr;
    /* Stored as little endian */
    A_UINT8 tid_name[MAX_HTT_TID_NAME];
    /* BIT [15 :  0]   :- sw_peer_id
     * BIT [31 : 16]   :- tid_num
     */
    A_UINT32 sw_peer_id__tid_num;
    /* BIT [ 7 :  0]   :- num_sched_pending
     * BIT [15 :  8]   :- num_ppdu_in_hwq
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 num_sched_pending__num_ppdu_in_hwq;
    A_UINT32 tid_flags;
    /* Max qdepth in bytes reached by this tid*/
    A_UINT32 max_qdepth_bytes;
    /* number of msdus qdepth reached max */
    A_UINT32 max_qdepth_n_msdus;
    /* Made reserved this field */
    A_UINT32 rsvd;

    A_UINT32 qdepth_bytes;
    A_UINT32 qdepth_num_msdu;
    A_UINT32 qdepth_num_mpdu;
    A_UINT32 last_scheduled_tsmp;
    A_UINT32 pause_module_id;
    A_UINT32 block_module_id;
    /* tid tx airtime in sec */
    A_UINT32 tid_tx_airtime;
    A_UINT32 allow_n_flags;
    /* BIT [15 :  0]   :- sendn_frms_allowed
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 sendn_frms_allowed;
} htt_tx_tid_stats_v1_tlv;

#define HTT_RX_TID_STATS_SW_PEER_ID_M 0x0000ffff
#define HTT_RX_TID_STATS_SW_PEER_ID_S 0
#define HTT_RX_TID_STATS_TID_NUM_M 0xffff0000
#define HTT_RX_TID_STATS_TID_NUM_S 16

#define HTT_RX_TID_STATS_SW_PEER_ID_GET(_var) \
    (((_var) & HTT_RX_TID_STATS_SW_PEER_ID_M) >> \
     HTT_RX_TID_STATS_SW_PEER_ID_S)

#define HTT_RX_TID_STATS_SW_PEER_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RX_TID_STATS_SW_PEER_ID, _val); \
        ((_var) |= ((_val) << HTT_RX_TID_STATS_SW_PEER_ID_S)); \
    } while (0)

#define HTT_RX_TID_STATS_TID_NUM_GET(_var) \
    (((_var) & HTT_RX_TID_STATS_TID_NUM_M) >> \
     HTT_RX_TID_STATS_TID_NUM_S)

#define HTT_RX_TID_STATS_TID_NUM_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RX_TID_STATS_TID_NUM, _val); \
        ((_var) |= ((_val) << HTT_RX_TID_STATS_TID_NUM_S)); \
    } while (0)

typedef struct _htt_rx_tid_stats_tlv {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [15 : 0] : sw_peer_id
     * BIT [31 : 16] : tid_num
     */
    A_UINT32 sw_peer_id__tid_num;
    /* Stored as little endian */
    A_UINT8 tid_name[MAX_HTT_TID_NAME];
    /* dup_in_reorder not collected per tid for now,
       as there is no wal_peer back ptr in data rx peer. */
    A_UINT32 dup_in_reorder;
    A_UINT32 dup_past_outside_window;
    A_UINT32 dup_past_within_window;
    /* Number of per tid MSDUs with flag of decrypt_err */
    A_UINT32 rxdesc_err_decrypt;
    /* tid rx airtime in sec */
    A_UINT32 tid_rx_airtime;
} htt_rx_tid_stats_tlv;

#define HTT_MAX_COUNTER_NAME 8
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Stored as little endian */
    A_UINT8  counter_name[HTT_MAX_COUNTER_NAME];
    A_UINT32 count;
} htt_counter_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Number of rx ppdu. */
    A_UINT32 ppdu_cnt;
    /* Number of rx mpdu. */
    A_UINT32 mpdu_cnt;
    /* Number of rx msdu */
    A_UINT32 msdu_cnt;
    /* Pause bitmap */
    A_UINT32 pause_bitmap;
    /* Block bitmap */
    A_UINT32 block_bitmap;
    /* Current timestamp */
    A_UINT32 current_timestamp;
    /* Peer cumulative tx airtime in sec */
    A_UINT32 peer_tx_airtime;
    /* Peer cumulative rx airtime in sec */
    A_UINT32 peer_rx_airtime;
    /* Peer current rssi in dBm */
    A_INT32 rssi;
    /* Total enqueued, dequeued and dropped msdu's for peer */
    A_UINT32 peer_enqueued_count_low;
    A_UINT32 peer_enqueued_count_high;
    A_UINT32 peer_dequeued_count_low;
    A_UINT32 peer_dequeued_count_high;
    A_UINT32 peer_dropped_count_low;
    A_UINT32 peer_dropped_count_high;
    /* Total ppdu transmitted bytes for peer: includes MAC header overhead */
    A_UINT32 ppdu_transmitted_bytes_low;
    A_UINT32 ppdu_transmitted_bytes_high;
    A_UINT32 peer_ttl_removed_count;
    /* inactive_time
     * Running duration of the time since last tx/rx activity by this peer,
     * units = seconds.
     * If the peer is currently active, this inactive_time will be 0x0.
     */
    A_UINT32 inactive_time;
    /* Number of MPDUs dropped after max retries */
    A_UINT32 remove_mpdus_max_retries;
} htt_peer_stats_cmn_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* This enum type of HTT_PEER_TYPE */
    A_UINT32 peer_type;
    A_UINT32 sw_peer_id;
    /* BIT [7 : 0]   :- vdev_id
     * BIT [15 : 8]  :- pdev_id
     * BIT [31 : 16] :- ast_indx
     */
    A_UINT32     vdev_pdev_ast_idx;
    htt_mac_addr mac_addr;
    A_UINT32     peer_flags;
    A_UINT32     qpeer_flags;
} htt_peer_details_tlv;

typedef enum {
    HTT_STATS_PREAM_OFDM,
    HTT_STATS_PREAM_CCK,
    HTT_STATS_PREAM_HT,
    HTT_STATS_PREAM_VHT,
    HTT_STATS_PREAM_HE,
    HTT_STATS_PREAM_RSVD,
    HTT_STATS_PREAM_RSVD1,

    HTT_STATS_PREAM_COUNT,
} HTT_STATS_PREAM_TYPE;

#define HTT_TX_PEER_STATS_NUM_MCS_COUNTERS 12
#define HTT_TX_PEER_STATS_NUM_GI_COUNTERS 4
#define HTT_TX_PEER_STATS_NUM_DCM_COUNTERS 5
#define HTT_TX_PEER_STATS_NUM_BW_COUNTERS 4
#define HTT_TX_PEER_STATS_NUM_SPATIAL_STREAMS 8
#define HTT_TX_PEER_STATS_NUM_PREAMBLE_TYPES HTT_STATS_PREAM_COUNT

typedef struct _htt_tx_peer_rate_stats_tlv {
    htt_tlv_hdr_t tlv_hdr;

    /* Number of tx ldpc packets */
    A_UINT32 tx_ldpc;
    /* Number of tx rts packets */
    A_UINT32 rts_cnt;
    /* RSSI value of last ack packet (units = dB above noise floor) */
    A_UINT32 ack_rssi;

    A_UINT32 tx_mcs[HTT_TX_PEER_STATS_NUM_MCS_COUNTERS];
    A_UINT32 tx_su_mcs[HTT_TX_PEER_STATS_NUM_MCS_COUNTERS];
    A_UINT32 tx_mu_mcs[HTT_TX_PEER_STATS_NUM_MCS_COUNTERS];
    A_UINT32 tx_nss[HTT_TX_PEER_STATS_NUM_SPATIAL_STREAMS]; /* element 0,1, ...7 -> NSS 1,2, ...8 */
    A_UINT32 tx_bw[HTT_TX_PEER_STATS_NUM_BW_COUNTERS];      /* element 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160 and 80+80 MHz */
    A_UINT32 tx_stbc[HTT_TX_PEER_STATS_NUM_MCS_COUNTERS];
    A_UINT32 tx_pream[HTT_TX_PEER_STATS_NUM_PREAMBLE_TYPES];

    /* Counters to track number of tx packets in each GI (400us, 800us, 1600us & 3200us) in each mcs (0-11) */
    A_UINT32 tx_gi[HTT_TX_PEER_STATS_NUM_GI_COUNTERS][HTT_TX_PEER_STATS_NUM_MCS_COUNTERS];

    /* Counters to track packets in dcm mcs (MCS 0, 1, 3, 4) */
    A_UINT32 tx_dcm[HTT_TX_PEER_STATS_NUM_DCM_COUNTERS];
} htt_tx_peer_rate_stats_tlv;

#define HTT_RX_PEER_STATS_NUM_MCS_COUNTERS 12
#define HTT_RX_PEER_STATS_NUM_GI_COUNTERS 4
#define HTT_RX_PEER_STATS_NUM_DCM_COUNTERS 5
#define HTT_RX_PEER_STATS_NUM_BW_COUNTERS 4
#define HTT_RX_PEER_STATS_NUM_SPATIAL_STREAMS 8
#define HTT_RX_PEER_STATS_NUM_PREAMBLE_TYPES HTT_STATS_PREAM_COUNT

typedef struct _htt_rx_peer_rate_stats_tlv {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      nsts;

    /* Number of rx ldpc packets */
    A_UINT32 rx_ldpc;
    /* Number of rx rts packets */
    A_UINT32 rts_cnt;

    A_UINT32 rssi_mgmt; /* units = dB above noise floor */
    A_UINT32 rssi_data; /* units = dB above noise floor */
    A_UINT32 rssi_comb; /* units = dB above noise floor */
    A_UINT32 rx_mcs[HTT_RX_PEER_STATS_NUM_MCS_COUNTERS];
    A_UINT32 rx_nss[HTT_RX_PEER_STATS_NUM_SPATIAL_STREAMS]; /* element 0,1, ...7 -> NSS 1,2, ...8 */
    A_UINT32 rx_dcm[HTT_RX_PEER_STATS_NUM_DCM_COUNTERS];
    A_UINT32 rx_stbc[HTT_RX_PEER_STATS_NUM_MCS_COUNTERS];
    A_UINT32 rx_bw[HTT_RX_PEER_STATS_NUM_BW_COUNTERS]; /* element 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160 and 80+80 MHz */
    A_UINT32 rx_pream[HTT_RX_PEER_STATS_NUM_PREAMBLE_TYPES];
    A_UINT8  rssi_chain[HTT_RX_PEER_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PEER_STATS_NUM_BW_COUNTERS]; /* units = dB above noise floor */

    /* Counters to track number of rx packets in each GI in each mcs (0-11) */
    A_UINT32 rx_gi[HTT_RX_PEER_STATS_NUM_GI_COUNTERS][HTT_RX_PEER_STATS_NUM_MCS_COUNTERS];

    A_UINT32 rx_ulofdma_non_data_ppdu;                             /* ppdu level */
    A_UINT32 rx_ulofdma_data_ppdu;                                 /* ppdu level */
    A_UINT32 rx_ulofdma_mpdu_ok;                                   /* mpdu level */
    A_UINT32 rx_ulofdma_mpdu_fail;                                 /* mpdu level */
    A_INT8   rx_ul_fd_rssi[HTT_RX_PEER_STATS_NUM_SPATIAL_STREAMS]; /* dBm unit */
    /* per_chain_rssi_pkt_type:
     * This field shows what type of rx frame the per-chain RSSI was computed
     * on, by recording the frame type and sub-type as bit-fields within this
     * field:
     * BIT [3 : 0]    :- IEEE80211_FC0_TYPE
     * BIT [7 : 4]    :- IEEE80211_FC0_SUBTYPE
     * BIT [31 : 8]   :- Reserved
     */
    A_UINT32 per_chain_rssi_pkt_type;
    A_INT8   rx_per_chain_rssi_in_dbm[HTT_RX_PEER_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PEER_STATS_NUM_BW_COUNTERS];

    A_UINT32 rx_ulmumimo_non_data_ppdu;   /* ppdu level */
    A_UINT32 rx_ulmumimo_data_ppdu;       /* ppdu level */
    A_UINT32 rx_ulmumimo_mpdu_ok;         /* mpdu level */
    A_UINT32 rx_ulmumimo_mpdu_fail;       /* mpdu level */
} htt_rx_peer_rate_stats_tlv;

typedef enum {
    HTT_PEER_STATS_REQ_MODE_NO_QUERY,
    HTT_PEER_STATS_REQ_MODE_QUERY_TQM,
    HTT_PEER_STATS_REQ_MODE_FLUSH_TQM,
} htt_peer_stats_req_mode_t;

typedef enum {
    HTT_PEER_STATS_CMN_TLV     = 0,
    HTT_PEER_DETAILS_TLV       = 1,
    HTT_TX_PEER_RATE_STATS_TLV = 2,
    HTT_RX_PEER_RATE_STATS_TLV = 3,
    HTT_TX_TID_STATS_TLV       = 4,
    HTT_RX_TID_STATS_TLV       = 5,
    HTT_MSDU_FLOW_STATS_TLV    = 6,

    HTT_PEER_STATS_MAX_TLV     = 31,
} htt_peer_stats_tlv_enum;

/* config_param0 */
#define HTT_DBG_EXT_STATS_PEER_INFO_IS_MAC_ADDR_M 0x00000001
#define HTT_DBG_EXT_STATS_PEER_INFO_IS_MAC_ADDR_S 0

#define HTT_DBG_EXT_STATS_PEER_REQ_MODE_M 0x0000FFFE
#define HTT_DBG_EXT_STATS_PEER_REQ_MODE_S 1

#define HTT_DBG_EXT_STATS_PEER_INFO_SW_PEER_ID_M 0xFFFF0000
#define HTT_DBG_EXT_STATS_PEER_INFO_SW_PEER_ID_S 16

#define HTT_DBG_EXT_STATS_PEER_INFO_IS_MAC_ADDR_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_DBG_EXT_STATS_PEER_INFO_IS_MAC_ADDR, _val); \
        ((_var) |= ((_val) << HTT_DBG_EXT_STATS_PEER_INFO_IS_MAC_ADDR_S)); \
    } while (0)

#define HTT_DBG_EXT_STATS_PEER_INFO_IS_MAC_ADDR_GET(_var) \
    (((_var) & HTT_DBG_EXT_STATS_PEER_INFO_IS_MAC_ADDR_M) >> \
     HTT_DBG_EXT_STATS_PEER_INFO_IS_MAC_ADDR_S)

#define HTT_DBG_EXT_STATS_PEER_REQ_MODE_GET(_var) \
    (((_var) & HTT_DBG_EXT_STATS_PEER_REQ_MODE_M) >> \
     HTT_DBG_EXT_STATS_PEER_REQ_MODE_S)

#define HTT_DBG_EXT_STATS_PEER_REQ_MODE_SET(_var, _val) \
    do { \
        ((_var) |= ((_val) << HTT_DBG_EXT_STATS_PEER_REQ_MODE_S)); \
    } while (0)

#define HTT_DBG_EXT_STATS_PEER_INFO_SW_PEER_ID_GET(_var) \
    (((_var) & HTT_DBG_EXT_STATS_PEER_INFO_SW_PEER_ID_M) >> \
     HTT_DBG_EXT_STATS_PEER_INFO_SW_PEER_ID_S)

#define HTT_DBG_EXT_STATS_PEER_INFO_SW_PEER_ID_SET(_var, _val) \
    do { \
        ((_var) |= ((_val) << HTT_DBG_EXT_STATS_PEER_INFO_SW_PEER_ID_S)); \
    } while (0)

/* STATS_TYPE : HTT_DBG_EXT_STATS_PEER_INFO
 * TLV_TAGS:
 *   - HTT_STATS_PEER_STATS_CMN_TAG
 *   - HTT_STATS_PEER_DETAILS_TAG
 *   - HTT_STATS_PEER_TX_RATE_STATS_TAG
 *   - HTT_STATS_PEER_RX_RATE_STATS_TAG
 *   - HTT_STATS_TX_TID_DETAILS_TAG (multiple) (deprecated, so 0 elements in updated systems)
 *   - HTT_STATS_RX_TID_DETAILS_TAG (multiple)
 *   - HTT_STATS_PEER_MSDU_FLOWQ_TAG (multiple)
 *   - HTT_STATS_TX_TID_DETAILS_V1_TAG (multiple)
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct _htt_peer_stats {
    htt_peer_stats_cmn_tlv cmn_tlv;

    htt_peer_details_tlv peer_details;
    /* from g_rate_info_stats */
    htt_tx_peer_rate_stats_tlv tx_rate;
    htt_rx_peer_rate_stats_tlv rx_rate;
    htt_tx_tid_stats_tlv       tx_tid_stats[1];
    htt_rx_tid_stats_tlv       rx_tid_stats[1];
    htt_msdu_flow_stats_tlv    msdu_flowq[1];
    htt_tx_tid_stats_v1_tlv    tx_tid_stats_v1[1];
} htt_peer_stats_t;

/* =========== ACTIVE PEER LIST ========== */

/* STATS_TYPE: HTT_DBG_EXT_STATS_ACTIVE_PEERS_LIST
 * TLV_TAGS:
 *     - HTT_STATS_PEER_DETAILS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_peer_details_tlv peer_details[1];
} htt_active_peer_details_list_t;

/* =========== MUMIMO HWQ stats =========== */

/* MU MIMO stats per hwQ */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      mu_mimo_sch_posted;
    A_UINT32      mu_mimo_sch_failed;
    A_UINT32      mu_mimo_ppdu_posted;
} htt_tx_hwq_mu_mimo_sch_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      mu_mimo_mpdus_queued_usr;   /* Number of mpdus queued per user */
    A_UINT32      mu_mimo_mpdus_tried_usr;    /* Number of mpdus actually transmitted by TxPCU per user */
    A_UINT32      mu_mimo_mpdus_failed_usr;   /* Number of mpdus failed per user */
    A_UINT32      mu_mimo_mpdus_requeued_usr; /* Number of mpdus requeued per user */
    A_UINT32      mu_mimo_err_no_ba_usr;      /* Number of times BA is not received for a user in MU PPDU */
    A_UINT32      mu_mimo_mpdu_underrun_usr;
    A_UINT32      mu_mimo_ampdu_underrun_usr;
} htt_tx_hwq_mu_mimo_mpdu_stats_tlv;

#define HTT_TX_HWQ_MU_MIMO_CMN_STATS_MAC_ID_M 0x000000ff
#define HTT_TX_HWQ_MU_MIMO_CMN_STATS_MAC_ID_S 0

#define HTT_TX_HWQ_MU_MIMO_CMN_STATS_HWQ_ID_M 0x0000ff00
#define HTT_TX_HWQ_MU_MIMO_CMN_STATS_HWQ_ID_S 8

#define HTT_TX_HWQ_MU_MIMO_CMN_STATS_MAC_ID_GET(_var) \
    (((_var) & HTT_TX_HWQ_MU_MIMO_CMN_STATS_MAC_ID_M) >> \
     HTT_TX_HWQ_MU_MIMO_CMN_STATS_MAC_ID_S)

#define HTT_TX_HWQ_MU_MIMO_CMN_STATS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_HWQ_MU_MIMO_CMN_STATS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_HWQ_MU_MIMO_CMN_STATS_MAC_ID_S)); \
    } while (0)

#define HTT_TX_HWQ_MU_MIMO_CMN_STATS_HWQ_ID_GET(_var) \
    (((_var) & HTT_TX_HWQ_MU_MIMO_CMN_STATS_HWQ_ID_M) >> \
     HTT_TX_HWQ_MU_MIMO_CMN_STATS_HWQ_ID_S)

#define HTT_TX_HWQ_MU_MIMO_CMN_STATS_HWQ_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_HWQ_MU_MIMO_CMN_STATS_HWQ_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_HWQ_MU_MIMO_CMN_STATS_HWQ_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [15 :  8]   :- hwq_id
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 mac_id__hwq_id__word;
} htt_tx_hwq_mu_mimo_cmn_stats_tlv;

/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    struct _hwq_mu_mimo_stats {
        htt_tx_hwq_mu_mimo_cmn_stats_tlv  cmn_tlv;
        htt_tx_hwq_mu_mimo_sch_stats_tlv  mu_mimo_sch_stats_tlv[1];  /* WAL_TX_STATS_MAX_GROUP_SIZE */
        htt_tx_hwq_mu_mimo_mpdu_stats_tlv mu_mimo_mpdu_stats_tlv[1]; /* WAL_TX_STATS_TX_MAX_NUM_USERS */
    } hwq[1];
} htt_tx_hwq_mu_mimo_stats_t;

/* == TX HWQ STATS == */
#define HTT_TX_HWQ_STATS_CMN_MAC_ID_M 0x000000ff
#define HTT_TX_HWQ_STATS_CMN_MAC_ID_S 0

#define HTT_TX_HWQ_STATS_CMN_HWQ_ID_M 0x0000ff00
#define HTT_TX_HWQ_STATS_CMN_HWQ_ID_S 8

#define HTT_TX_HWQ_STATS_CMN_MAC_ID_GET(_var) \
    (((_var) & HTT_TX_HWQ_STATS_CMN_MAC_ID_M) >> \
     HTT_TX_HWQ_STATS_CMN_MAC_ID_S)

#define HTT_TX_HWQ_STATS_CMN_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_HWQ_STATS_CMN_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_HWQ_STATS_CMN_MAC_ID_S)); \
    } while (0)

#define HTT_TX_HWQ_STATS_CMN_HWQ_ID_GET(_var) \
    (((_var) & HTT_TX_HWQ_STATS_CMN_HWQ_ID_M) >> \
     HTT_TX_HWQ_STATS_CMN_HWQ_ID_S)

#define HTT_TX_HWQ_STATS_CMN_HWQ_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_HWQ_STATS_CMN_HWQ_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_HWQ_STATS_CMN_HWQ_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [15 :  8]   :- hwq_id
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 mac_id__hwq_id__word;

    /* PPDU level stats */
    A_UINT32 xretry;              /* Number of times ack is failed for the PPDU scheduled on this txQ */
    A_UINT32 underrun_cnt;        /* Number of times sched cmd status reported mpdu underrun */
    A_UINT32 flush_cnt;           /* Number of times sched cmd is flushed */
    A_UINT32 filt_cnt;            /* Number of times sched cmd is filtered */
    A_UINT32 null_mpdu_bmap;      /* Number of times HWSCH uploaded null mpdu bitmap */
    A_UINT32 user_ack_failure;    /* Number of time user ack or ba tlv is not seen on FES ring where it is expected to be */
    A_UINT32 ack_tlv_proc;        /* Number of times TQM processed ack tlv received from HWSCH */
    A_UINT32 sched_id_proc;       /* Cache latest processed scheduler ID received from ack ba tlv */
    A_UINT32 null_mpdu_tx_count;  /* Number of times TxPCU reported mpdus transmitted for a user is zero */
    A_UINT32 mpdu_bmap_not_recvd; /* Number of times SW did not see any mpdu info bitmap tlv on FES status ring */

    /* Selfgen stats per hwQ */
    A_UINT32 num_bar;  /* Number of SU/MU BAR frames posted to hwQ */
    A_UINT32 rts;      /* Number of RTS frames posted to hwQ */
    A_UINT32 cts2self; /* Number of cts2self frames posted to hwQ */
    A_UINT32 qos_null; /* Number of qos null frames posted to hwQ */

    /* MPDU level stats */
    A_UINT32 mpdu_tried_cnt;       /* mpdus tried Tx by HWSCH/TQM */
    A_UINT32 mpdu_queued_cnt;      /* mpdus queued to HWSCH */
    A_UINT32 mpdu_ack_fail_cnt;    /* mpdus tried but ack was not received */
    A_UINT32 mpdu_filt_cnt;        /* This will include sched cmd flush and time based discard */
    A_UINT32 false_mpdu_ack_count; /* Number of MPDUs for which ACK was sucessful but no Tx happened */

    A_UINT32 txq_timeout; /* Number of times txq timeout happened */
} htt_tx_hwq_stats_cmn_tlv;

#define HTT_TX_HWQ_DIFS_LATENCY_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) + /* hist_intvl */ \
                                                          (sizeof(A_UINT32) * (_num_elems)))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      hist_intvl;
    /* histogram of ppdu post to hwsch - > cmd status received */
    A_UINT32 difs_latency_hist[1]; /* HTT_TX_HWQ_MAX_DIFS_LATENCY_BINS */
} htt_tx_hwq_difs_latency_stats_tlv_v;

#define HTT_TX_HWQ_CMD_RESULT_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Histogram of sched cmd result */
    A_UINT32 cmd_result[1]; /* HTT_TX_HWQ_MAX_CMD_RESULT_STATS */
} htt_tx_hwq_cmd_result_stats_tlv_v;

#define HTT_TX_HWQ_CMD_STALL_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Histogram of various pause conitions */
    A_UINT32 cmd_stall_status[1]; /* HTT_TX_HWQ_MAX_CMD_STALL_STATS */
} htt_tx_hwq_cmd_stall_stats_tlv_v;

#define HTT_TX_HWQ_FES_RESULT_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Histogram of number of user fes result */
    A_UINT32 fes_result[1]; /* HTT_TX_HWQ_MAX_FES_RESULT_STATS */
} htt_tx_hwq_fes_result_stats_tlv_v;

#define HTT_TX_HWQ_TRIED_MPDU_CNT_HIST_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size
 *
 *  The hwq_tried_mpdu_cnt_hist is a  histogram of MPDUs tries per HWQ.
 *  The tries here is the count of the  MPDUS within a PPDU that the HW
 *  had attempted to transmit on  air, for the HWSCH Schedule command
 *  submitted by FW in this HWQ .It is not the retry attempts. The
 *  histogram bins are  0-29, 30-59, 60-89 and so on. The are 10 bins
 *  in this histogram.
 *  they are defined in FW using the following macros
 *  #define WAL_MAX_TRIED_MPDU_CNT_HISTOGRAM 9
 *  #define WAL_TRIED_MPDU_CNT_HISTOGRAM_INTERVAL 30
 *
 * */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      hist_bin_size;
    /* Histogram of number of mpdus on tried mpdu */
    A_UINT32 tried_mpdu_cnt_hist[1]; /* HTT_TX_HWQ_TRIED_MPDU_CNT_HIST */
} htt_tx_hwq_tried_mpdu_cnt_hist_tlv_v;

#define HTT_TX_HWQ_TXOP_USED_CNT_HIST_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size
 *
 * The txop_used_cnt_hist is the histogram of txop per burst. After
 * completing the burst, we identify the txop used in the burst and
 * incr the corresponding bin.
 * Each bin represents 1ms & we have 10 bins in this histogram.
 * they are deined in FW using the following macros
 * #define WAL_MAX_TXOP_USED_CNT_HISTOGRAM 10
 * #define WAL_TXOP_USED_HISTOGRAM_INTERVAL 1000 ( 1 ms )
 *
 * */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Histogram of txop used cnt */
    A_UINT32 txop_used_cnt_hist[1]; /* HTT_TX_HWQ_TXOP_USED_CNT_HIST */
} htt_tx_hwq_txop_used_cnt_hist_tlv_v;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_TX_HWQ
 * TLV_TAGS:
 *    - HTT_STATS_STRING_TAG
 *    - HTT_STATS_TX_HWQ_CMN_TAG
 *    - HTT_STATS_TX_HWQ_DIFS_LATENCY_TAG
 *    - HTT_STATS_TX_HWQ_CMD_RESULT_TAG
 *    - HTT_STATS_TX_HWQ_CMD_STALL_TAG
 *    - HTT_STATS_TX_HWQ_FES_STATUS_TAG
 *    - HTT_STATS_TX_HWQ_TRIED_MPDU_CNT_HIST_TAG
 *    - HTT_STATS_TX_HWQ_TXOP_USED_CNT_HIST_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 * General  HWQ stats Mechanism:
 * Once the host request for the stats, FW fill all the HWQ TAGS in a buffer
 * for all the HWQ requested. & the FW send the  buffer to  host. In the
 * buffer the HWQ ID  is filled in mac_id__hwq_id, thus identifying each
 * HWQ distinctly.
 */
typedef struct _htt_tx_hwq_stats {
    htt_stats_string_tlv                 hwq_str_tlv;
    htt_tx_hwq_stats_cmn_tlv             cmn_tlv;
    htt_tx_hwq_difs_latency_stats_tlv_v  difs_tlv;
    htt_tx_hwq_cmd_result_stats_tlv_v    cmd_result_tlv;
    htt_tx_hwq_cmd_stall_stats_tlv_v     cmd_stall_tlv;
    htt_tx_hwq_fes_result_stats_tlv_v    fes_stats_tlv;
    htt_tx_hwq_tried_mpdu_cnt_hist_tlv_v tried_mpdu_tlv;
    htt_tx_hwq_txop_used_cnt_hist_tlv_v  txop_used_tlv;
} htt_tx_hwq_stats_t;

/* == TX SELFGEN STATS == */

#define HTT_TX_SELFGEN_CMN_STATS_MAC_ID_M 0x000000ff
#define HTT_TX_SELFGEN_CMN_STATS_MAC_ID_S 0

#define HTT_TX_SELFGEN_CMN_STATS_MAC_ID_GET(_var) \
    (((_var) & HTT_TX_SELFGEN_CMN_STATS_MAC_ID_M) >> \
     HTT_TX_SELFGEN_CMN_STATS_MAC_ID_S)

#define HTT_TX_SELFGEN_CMN_STATS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_SELFGEN_CMN_STATS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_SELFGEN_CMN_STATS_MAC_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    A_UINT32 su_bar;
    A_UINT32 rts;
    A_UINT32 cts2self;
    A_UINT32 qos_null;
    A_UINT32 delayed_bar_1; /* MU user 1 */
    A_UINT32 delayed_bar_2; /* MU user 2 */
    A_UINT32 delayed_bar_3; /* MU user 3 */
    A_UINT32 delayed_bar_4; /* MU user 4 */
    A_UINT32 delayed_bar_5; /* MU user 5 */
    A_UINT32 delayed_bar_6; /* MU user 6 */
    A_UINT32 delayed_bar_7; /* MU user 7 */
} htt_tx_selfgen_cmn_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* 11AC */
    A_UINT32 ac_su_ndpa;
    A_UINT32 ac_su_ndp;
    A_UINT32 ac_mu_mimo_ndpa;
    A_UINT32 ac_mu_mimo_ndp;
    A_UINT32 ac_mu_mimo_brpoll_1; /* MU user 1 */
    A_UINT32 ac_mu_mimo_brpoll_2; /* MU user 2 */
    A_UINT32 ac_mu_mimo_brpoll_3; /* MU user 3 */
} htt_tx_selfgen_ac_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* 11AX */
    A_UINT32 ax_su_ndpa;
    A_UINT32 ax_su_ndp;
    A_UINT32 ax_mu_mimo_ndpa;
    A_UINT32 ax_mu_mimo_ndp;
    A_UINT32 ax_mu_mimo_brpoll_1; /* MU user 1 */
    A_UINT32 ax_mu_mimo_brpoll_2; /* MU user 2 */
    A_UINT32 ax_mu_mimo_brpoll_3; /* MU user 3 */
    A_UINT32 ax_mu_mimo_brpoll_4; /* MU user 4 */
    A_UINT32 ax_mu_mimo_brpoll_5; /* MU user 5 */
    A_UINT32 ax_mu_mimo_brpoll_6; /* MU user 6 */
    A_UINT32 ax_mu_mimo_brpoll_7; /* MU user 7 */
    A_UINT32 ax_basic_trigger;
    A_UINT32 ax_bsr_trigger;
    A_UINT32 ax_mu_bar_trigger;
    A_UINT32 ax_mu_rts_trigger;
    A_UINT32 ax_ulmumimo_trigger;
} htt_tx_selfgen_ax_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* 11AC error stats */
    A_UINT32 ac_su_ndp_err;
    A_UINT32 ac_su_ndpa_err;
    A_UINT32 ac_mu_mimo_ndpa_err;
    A_UINT32 ac_mu_mimo_ndp_err;
    A_UINT32 ac_mu_mimo_brp1_err;
    A_UINT32 ac_mu_mimo_brp2_err;
    A_UINT32 ac_mu_mimo_brp3_err;
} htt_tx_selfgen_ac_err_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* 11AX error stats */
    A_UINT32 ax_su_ndp_err;
    A_UINT32 ax_su_ndpa_err;
    A_UINT32 ax_mu_mimo_ndpa_err;
    A_UINT32 ax_mu_mimo_ndp_err;
    A_UINT32 ax_mu_mimo_brp1_err;
    A_UINT32 ax_mu_mimo_brp2_err;
    A_UINT32 ax_mu_mimo_brp3_err;
    A_UINT32 ax_mu_mimo_brp4_err;
    A_UINT32 ax_mu_mimo_brp5_err;
    A_UINT32 ax_mu_mimo_brp6_err;
    A_UINT32 ax_mu_mimo_brp7_err;
    A_UINT32 ax_basic_trigger_err;
    A_UINT32 ax_bsr_trigger_err;
    A_UINT32 ax_mu_bar_trigger_err;
    A_UINT32 ax_mu_rts_trigger_err;
    A_UINT32 ax_ulmumimo_trigger_err;
} htt_tx_selfgen_ax_err_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_TX_SELFGEN_INFO
 * TLV_TAGS:
 *      - HTT_STATS_TX_SELFGEN_CMN_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_AC_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_AX_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_AC_ERR_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_AX_ERR_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_tx_selfgen_cmn_stats_tlv cmn_tlv;
    /* 11AC */
    htt_tx_selfgen_ac_stats_tlv ac_tlv;
    /* 11AX */
    htt_tx_selfgen_ax_stats_tlv ax_tlv;
    /* 11AC error stats */
    htt_tx_selfgen_ac_err_stats_tlv ac_err_tlv;
    /* 11AX error stats */
    htt_tx_selfgen_ax_err_stats_tlv ax_err_tlv;
} htt_tx_pdev_selfgen_stats_t;

/* == TX MU STATS == */

#define HTT_TX_PDEV_STATS_NUM_AC_MUMIMO_USER_STATS 4
#define HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS 8
#define HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS 74

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* mu-mimo sw sched cmd stats */
    A_UINT32 mu_mimo_sch_posted;
    A_UINT32 mu_mimo_sch_failed;
    /* MU PPDU stats per hwQ */
    A_UINT32 mu_mimo_ppdu_posted;
    /*
     * Counts the number of users in each transmission of
     * the given TX mode.
     *
     * Index is the number of users - 1.
     */
    A_UINT32 ac_mu_mimo_sch_nusers[HTT_TX_PDEV_STATS_NUM_AC_MUMIMO_USER_STATS];
    A_UINT32 ax_mu_mimo_sch_nusers[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS];
    A_UINT32 ax_ofdma_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
} htt_tx_pdev_mu_mimo_sch_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* mu-mimo mpdu level stats */
    /*
     * This first block of stats is limited to 11ac
     * MU-MIMO transmission.
     */
    A_UINT32 mu_mimo_mpdus_queued_usr;
    A_UINT32 mu_mimo_mpdus_tried_usr;
    A_UINT32 mu_mimo_mpdus_failed_usr;
    A_UINT32 mu_mimo_mpdus_requeued_usr;
    A_UINT32 mu_mimo_err_no_ba_usr;
    A_UINT32 mu_mimo_mpdu_underrun_usr;
    A_UINT32 mu_mimo_ampdu_underrun_usr;

    A_UINT32 ax_mu_mimo_mpdus_queued_usr;
    A_UINT32 ax_mu_mimo_mpdus_tried_usr;
    A_UINT32 ax_mu_mimo_mpdus_failed_usr;
    A_UINT32 ax_mu_mimo_mpdus_requeued_usr;
    A_UINT32 ax_mu_mimo_err_no_ba_usr;
    A_UINT32 ax_mu_mimo_mpdu_underrun_usr;
    A_UINT32 ax_mu_mimo_ampdu_underrun_usr;

    A_UINT32 ax_ofdma_mpdus_queued_usr;
    A_UINT32 ax_ofdma_mpdus_tried_usr;
    A_UINT32 ax_ofdma_mpdus_failed_usr;
    A_UINT32 ax_ofdma_mpdus_requeued_usr;
    A_UINT32 ax_ofdma_err_no_ba_usr;
    A_UINT32 ax_ofdma_mpdu_underrun_usr;
    A_UINT32 ax_ofdma_ampdu_underrun_usr;
} htt_tx_pdev_mu_mimo_mpdu_stats_tlv;

#define HTT_STATS_TX_SCHED_MODE_MU_MIMO_AC 1  /* SCHED_TX_MODE_MU_MIMO_AC */
#define HTT_STATS_TX_SCHED_MODE_MU_MIMO_AX 2  /* SCHED_TX_MODE_MU_MIMO_AX */
#define HTT_STATS_TX_SCHED_MODE_MU_OFDMA_AX 3 /* SCHED_TX_MODE_MU_OFDMA_AX */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* mpdu level stats */
    A_UINT32 mpdus_queued_usr;
    A_UINT32 mpdus_tried_usr;
    A_UINT32 mpdus_failed_usr;
    A_UINT32 mpdus_requeued_usr;
    A_UINT32 err_no_ba_usr;
    A_UINT32 mpdu_underrun_usr;
    A_UINT32 ampdu_underrun_usr;
    A_UINT32 user_index;
    A_UINT32 tx_sched_mode; /* HTT_STATS_TX_SCHED_MODE_xxx */
} htt_tx_pdev_mpdu_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_TX_MU
 * TLV_TAGS:
 *      - HTT_STATS_TX_PDEV_MU_MIMO_STATS_TAG (multiple)
 *      - HTT_STATS_TX_PDEV_MPDU_STATS_TAG (multiple)
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_tx_pdev_mu_mimo_sch_stats_tlv mu_mimo_sch_stats_tlv[1]; /* WAL_TX_STATS_MAX_GROUP_SIZE */
    /*
     * Note that though mu_mimo_mpdu_stats_tlv is named MU-MIMO,
     * it can also hold MU-OFDMA stats.
     */
    htt_tx_pdev_mpdu_stats_tlv mu_mimo_mpdu_stats_tlv[1]; /* WAL_TX_STATS_MAX_NUM_USERS */
} htt_tx_pdev_mu_mimo_stats_t;

/* == TX SCHED STATS == */

#define HTT_SCHED_TXQ_CMD_POSTED_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Scheduler command posted per tx_mode su / mu mimo 11ac / mu mimo 11ax / mu ofdma */
    A_UINT32 sched_cmd_posted[1]; /* HTT_TX_PDEV_SCHED_TX_MODE_MAX */
} htt_sched_txq_cmd_posted_tlv_v;

#define HTT_SCHED_TXQ_CMD_REAPED_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Scheduler command reaped per tx_mode su / mu mimo 11ac / mu mimo 11ax / mu ofdma */
    A_UINT32 sched_cmd_reaped[1]; /* HTT_TX_PDEV_SCHED_TX_MODE_MAX */
} htt_sched_txq_cmd_reaped_tlv_v;

#define HTT_SCHED_TXQ_SCHED_ORDER_SU_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /*
     * sched_order_su contains the peer IDs of peers chosen in the last
     * NUM_SCHED_ORDER_LOG scheduler instances.
     * The array is circular; it's unspecified which array element corresponds
     * to the most recent scheduler invocation, and which corresponds to
     * the (NUM_SCHED_ORDER_LOG-1) most recent scheduler invocation.
     */
    A_UINT32 sched_order_su[1]; /* HTT_TX_PDEV_NUM_SCHED_ORDER_LOG */
} htt_sched_txq_sched_order_su_tlv_v;

typedef enum {
    HTT_SCHED_TID_SKIP_SCHED_MASK_DISABLED = 0, /* Skip the tid when WAL_TID_DISABLE_TX_SCHED_MASK is true                                       */
    HTT_SCHED_TID_SKIP_NOTIFY_MPDU,             /* Skip the tid's 2nd sched_cmd when 1st cmd is ongoing                                          */
    HTT_SCHED_TID_SKIP_MPDU_STATE_INVALID,      /* Skip the tid when MPDU state is invalid                                                       */
    HTT_SCHED_TID_SKIP_SCHED_DISABLED,          /* Skip the tid when scheduling is disabled for that tid                                         */
    HTT_SCHED_TID_SKIP_TQM_BYPASS_CMD_PENDING,  /* Skip the TQM bypass tid when it has pending sched_cmd                                         */
    HTT_SCHED_TID_SKIP_SECOND_SU_SCHEDULE,      /* Skip tid from 2nd SU schedule when any of the following flag is set
                                                   WAL_TX_TID(SEND_BAR | TQM_MPDU_STATE_VALID | SEND_QOS_NULL | TQM_NOTIFY_MPDU | SENDN_PENDING) */
    HTT_SCHED_TID_SKIP_CMD_SLOT_NOT_AVAIL,      /* Skip the tid when command slot is not available                                               */
    HTT_SCHED_TID_SKIP_NO_ENQ,                  /* Skip the tid when num_frames is zero with g_disable_remove_tid as true                        */
    HTT_SCHED_TID_SKIP_LOW_ENQ,                 /* Skip the tid when enqueue is low                                                              */
    HTT_SCHED_TID_SKIP_PAUSED,                  /* Skipping the paused tid(sendn-frames)                                                         */
    HTT_SCHED_TID_SKIP_UL,                      /* UL tid skip                                                                                   */
    HTT_SCHED_TID_REMOVE_PAUSED,                /* Removing the paused tid when number of sendn frames is zero                                   */
    HTT_SCHED_TID_REMOVE_NO_ENQ,                /* Remove tid with zero queue depth                                                              */
    HTT_SCHED_TID_REMOVE_UL,                    /* UL tid remove                                                                                 */
    HTT_SCHED_TID_QUERY,                        /* Moving to next user and adding tid in prepend list when qstats update is pending              */
    HTT_SCHED_TID_SU_ONLY,                      /* Tid is eligible and TX_SCHED_SU_ONLY is true                                                  */
    HTT_SCHED_TID_ELIGIBLE,                     /* Tid is eligible for scheduling                                                                */
    HTT_SCHED_INELIGIBILITY_MAX,
} htt_sched_txq_sched_ineligibility_tlv_enum;

#define HTT_SCHED_TXQ_SCHED_INELIGIBILITY_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* sched_ineligibility counts the number of occurrences of different reasons for tid ineligibility during eligibility checks per txq in scheduling */
    A_UINT32 sched_ineligibility[1]; /* indexed by htt_sched_txq_sched_ineligibility_tlv_enum */
} htt_sched_txq_sched_ineligibility_tlv_v;

#define HTT_TX_PDEV_STATS_SCHED_PER_TXQ_MAC_ID_M 0x000000ff
#define HTT_TX_PDEV_STATS_SCHED_PER_TXQ_MAC_ID_S 0

#define HTT_TX_PDEV_STATS_SCHED_PER_TXQ_TXQUEUE_ID_M 0x0000ff00
#define HTT_TX_PDEV_STATS_SCHED_PER_TXQ_TXQUEUE_ID_S 8

#define HTT_TX_PDEV_STATS_SCHED_PER_TXQ_MAC_ID_GET(_var) \
    (((_var) & HTT_TX_PDEV_STATS_SCHED_PER_TXQ_MAC_ID_M) >> \
     HTT_TX_PDEV_STATS_SCHED_PER_TXQ_MAC_ID_S)

#define HTT_TX_PDEV_STATS_SCHED_PER_TXQ_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_PDEV_STATS_SCHED_PER_TXQ_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_PDEV_STATS_SCHED_PER_TXQ_MAC_ID_S)); \
    } while (0)

#define HTT_TX_PDEV_STATS_SCHED_PER_TXQ_TXQUEUE_ID_GET(_var) \
    (((_var) & HTT_TX_PDEV_STATS_SCHED_PER_TXQ_TXQUEUE_ID_M) >> \
     HTT_TX_PDEV_STATS_SCHED_PER_TXQ_TXQUEUE_ID_S)

#define HTT_TX_PDEV_STATS_SCHED_PER_TXQ_TXQUEUE_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_PDEV_STATS_SCHED_PER_TXQ_TXQUEUE_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_PDEV_STATS_SCHED_PER_TXQ_TXQUEUE_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [15 :  8]   :- txq_id
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 mac_id__txq_id__word;
    /* Scheduler policy ised for this TxQ */
    A_UINT32 sched_policy;
    /* Timestamp of last scheduler command posted */
    A_UINT32 last_sched_cmd_posted_timestamp;
    /* Timestamp of last scheduler command completed */
    A_UINT32 last_sched_cmd_compl_timestamp;
    /* Num of Sched2TAC ring hit Low Water Mark condition */
    A_UINT32 sched_2_tac_lwm_count;
    /* Num of Sched2TAC ring full condition */
    A_UINT32 sched_2_tac_ring_full;
    /* Num of scheduler command post failures that includes su/mu mimo/mu ofdma sequence type */
    A_UINT32 sched_cmd_post_failure;
    /* Num of active tids for this TxQ at current instance */
    A_UINT32 num_active_tids;
    /* Num of powersave schedules */
    A_UINT32 num_ps_schedules;
    /* Num of scheduler commands pending for this TxQ */
    A_UINT32 sched_cmds_pending;
    /* Num of tidq registration for this TxQ */
    A_UINT32 num_tid_register;
    /* Num of tidq de-registration for this TxQ */
    A_UINT32 num_tid_unregister;
    /* Num of iterations msduq stats was updated */
    A_UINT32 num_qstats_queried;
    /* qstats query update status */
    A_UINT32 qstats_update_pending;
    /* Timestamp of Last query stats made */
    A_UINT32 last_qstats_query_timestamp;
    /* Num of sched2tqm command queue full condition */
    A_UINT32 num_tqm_cmdq_full;
    /* Num of scheduler trigger from DE Module */
    A_UINT32 num_de_sched_algo_trigger;
    /* Num of scheduler trigger from RT Module */
    A_UINT32 num_rt_sched_algo_trigger;
    /* Num of scheduler trigger from TQM Module */
    A_UINT32 num_tqm_sched_algo_trigger;
    /* Num of schedules for notify frame */
    A_UINT32 notify_sched;
    /* Duration based sendn termination */
    A_UINT32 dur_based_sendn_term;
    /* scheduled via NOTIFY2 */
    A_UINT32 su_notify2_sched;
    /* schedule if queued packets are greater than avg MSDUs in PPDU */
    A_UINT32 su_optimal_queued_msdus_sched;
    /* schedule due to timeout */
    A_UINT32 su_delay_timeout_sched;
    /* delay if txtime is less than 500us */
    A_UINT32 su_min_txtime_sched_delay;
    /* scheduled via no delay */
    A_UINT32 su_no_delay;
} htt_tx_pdev_stats_sched_per_txq_tlv;

#define HTT_STATS_TX_SCHED_CMN_MAC_ID_M 0x000000ff
#define HTT_STATS_TX_SCHED_CMN_MAC_ID_S 0

#define HTT_STATS_TX_SCHED_CMN_MAC_ID_GET(_var) \
    (((_var) & HTT_STATS_TX_SCHED_CMN_MAC_ID_M) >> \
     HTT_STATS_TX_SCHED_CMN_MAC_ID_S)

#define HTT_STATS_TX_SCHED_CMN_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_STATS_TX_SCHED_CMN_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_STATS_TX_SCHED_CMN_MAC_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /* Current timestamp */
    A_UINT32 current_timestamp;
} htt_stats_tx_sched_cmn_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_TX_SCHED
 * TLV_TAGS:
 *     - HTT_STATS_TX_SCHED_CMN_TAG
 *     - HTT_STATS_TX_PDEV_SCHEDULER_TXQ_STATS_TAG
 *     - HTT_STATS_SCHED_TXQ_CMD_POSTED_TAG
 *     - HTT_STATS_SCHED_TXQ_CMD_REAPED_TAG
 *     - HTT_STATS_SCHED_TXQ_SCHED_ORDER_SU_TAG
 *     - HTT_STATS_SCHED_TXQ_SCHED_INELIGIBILITY_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_stats_tx_sched_cmn_tlv cmn_tlv;
    struct _txq_tx_sched_stats {
        htt_tx_pdev_stats_sched_per_txq_tlv     txq_tlv;
        htt_sched_txq_cmd_posted_tlv_v          cmd_posted_tlv;
        htt_sched_txq_cmd_reaped_tlv_v          cmd_reaped_tlv;
        htt_sched_txq_sched_order_su_tlv_v      sched_order_su_tlv;
        htt_sched_txq_sched_ineligibility_tlv_v sched_ineligibility_tlv;
    } txq[1];
} htt_stats_tx_sched_t;

/* == TQM STATS == */

#define HTT_TX_TQM_MAX_GEN_MPDU_END_REASON 16
#define HTT_TX_TQM_MAX_LIST_MPDU_END_REASON 16
#define HTT_TX_TQM_MAX_LIST_MPDU_CNT_HISTOGRAM_BINS 16

#define HTT_TX_TQM_GEN_MPDU_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      gen_mpdu_end_reason[1]; /* HTT_TX_TQM_MAX_GEN_MPDU_END_REASON */
} htt_tx_tqm_gen_mpdu_stats_tlv_v;

#define HTT_TX_TQM_LIST_MPDU_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      list_mpdu_end_reason[1]; /* HTT_TX_TQM_MAX_LIST_MPDU_END_REASON */
} htt_tx_tqm_list_mpdu_stats_tlv_v;

#define HTT_TX_TQM_LIST_MPDU_CNT_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      list_mpdu_cnt_hist[1]; /* HTT_TX_TQM_MAX_LIST_MPDU_CNT_HISTOGRAM_BINS */
} htt_tx_tqm_list_mpdu_cnt_tlv_v;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      msdu_count;
    A_UINT32      mpdu_count;
    A_UINT32      remove_msdu;
    A_UINT32      remove_mpdu;
    A_UINT32      remove_msdu_ttl;
    A_UINT32      send_bar;
    A_UINT32      bar_sync;
    A_UINT32      notify_mpdu;
    A_UINT32      sync_cmd;
    A_UINT32      write_cmd;
    A_UINT32      hwsch_trigger;
    A_UINT32      ack_tlv_proc;
    A_UINT32      gen_mpdu_cmd;
    A_UINT32      gen_list_cmd;
    A_UINT32      remove_mpdu_cmd;
    A_UINT32      remove_mpdu_tried_cmd;
    A_UINT32      mpdu_queue_stats_cmd;
    A_UINT32      mpdu_head_info_cmd;
    A_UINT32      msdu_flow_stats_cmd;
    A_UINT32      remove_msdu_cmd;
    A_UINT32      remove_msdu_ttl_cmd;
    A_UINT32      flush_cache_cmd;
    A_UINT32      update_mpduq_cmd;
    A_UINT32      enqueue;
    A_UINT32      enqueue_notify;
    A_UINT32      notify_mpdu_at_head;
    A_UINT32      notify_mpdu_state_valid;
/*
 * On receiving TQM_FLOW_NOT_EMPTY_STATUS from TQM, (on MSDUs being enqueued
 * the flow is non empty), if the number of MSDUs is greater than the threshold,
 * notify is incremented. UDP_THRESH counters are for UDP MSDUs, and NONUDP are
 * for non-UDP MSDUs.
 * MSDUQ_SWNOTIFY_UDP_THRESH1 threshold    - sched_udp_notify1 is incremented
 * MSDUQ_SWNOTIFY_UDP_THRESH2 threshold    - sched_udp_notify2 is incremented
 * MSDUQ_SWNOTIFY_NONUDP_THRESH1 threshold - sched_nonudp_notify1 is incremented
 * MSDUQ_SWNOTIFY_NONUDP_THRESH2 threshold - sched_nonudp_notify2 is incremented
 *
 * Notify signifies that we trigger the scheduler.
 */
    A_UINT32 sched_udp_notify1;
    A_UINT32 sched_udp_notify2;
    A_UINT32 sched_nonudp_notify1;
    A_UINT32 sched_nonudp_notify2;
} htt_tx_tqm_pdev_stats_tlv_v;

#define HTT_TX_TQM_CMN_STATS_MAC_ID_M 0x000000ff
#define HTT_TX_TQM_CMN_STATS_MAC_ID_S 0

#define HTT_TX_TQM_CMN_STATS_MAC_ID_GET(_var) \
    (((_var) & HTT_TX_TQM_CMN_STATS_MAC_ID_M) >> \
     HTT_TX_TQM_CMN_STATS_MAC_ID_S)

#define HTT_TX_TQM_CMN_STATS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_TQM_CMN_STATS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_TQM_CMN_STATS_MAC_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    A_UINT32 max_cmdq_id;
    A_UINT32 list_mpdu_cnt_hist_intvl;

    /* Global stats */
    A_UINT32 add_msdu;
    A_UINT32 q_empty;
    A_UINT32 q_not_empty;
    A_UINT32 drop_notification;
    A_UINT32 desc_threshold;
    A_UINT32 hwsch_tqm_invalid_status;
    A_UINT32 missed_tqm_gen_mpdus;
} htt_tx_tqm_cmn_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Error stats */
    A_UINT32 q_empty_failure;
    A_UINT32 q_not_empty_failure;
    A_UINT32 add_msdu_failure;
} htt_tx_tqm_error_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_TQM
 * TLV_TAGS:
 *     - HTT_STATS_TX_TQM_CMN_TAG
 *     - HTT_STATS_TX_TQM_ERROR_STATS_TAG
 *     - HTT_STATS_TX_TQM_GEN_MPDU_TAG
 *     - HTT_STATS_TX_TQM_LIST_MPDU_TAG
 *     - HTT_STATS_TX_TQM_LIST_MPDU_CNT_TAG
 *     - HTT_STATS_TX_TQM_PDEV_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_tx_tqm_cmn_stats_tlv         cmn_tlv;
    htt_tx_tqm_error_stats_tlv       err_tlv;
    htt_tx_tqm_gen_mpdu_stats_tlv_v  gen_mpdu_stats_tlv;
    htt_tx_tqm_list_mpdu_stats_tlv_v list_mpdu_stats_tlv;
    htt_tx_tqm_list_mpdu_cnt_tlv_v   list_mpdu_cnt_tlv;
    htt_tx_tqm_pdev_stats_tlv_v      tqm_pdev_stats_tlv;
} htt_tx_tqm_pdev_stats_t;

/* == TQM CMDQ stats == */
#define HTT_TX_TQM_CMDQ_STATUS_MAC_ID_M 0x000000ff
#define HTT_TX_TQM_CMDQ_STATUS_MAC_ID_S 0

#define HTT_TX_TQM_CMDQ_STATUS_CMDQ_ID_M 0x0000ff00
#define HTT_TX_TQM_CMDQ_STATUS_CMDQ_ID_S 8

#define HTT_TX_TQM_CMDQ_STATUS_MAC_ID_GET(_var) \
    (((_var) & HTT_TX_TQM_CMDQ_STATUS_MAC_ID_M) >> \
     HTT_TX_TQM_CMDQ_STATUS_MAC_ID_S)

#define HTT_TX_TQM_CMDQ_STATUS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_TQM_CMDQ_STATUS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_TQM_CMDQ_STATUS_MAC_ID_S)); \
    } while (0)

#define HTT_TX_TQM_CMDQ_STATUS_CMDQ_ID_GET(_var) \
    (((_var) & HTT_TX_TQM_CMDQ_STATUS_CMDQ_ID_M) >> \
     HTT_TX_TQM_CMDQ_STATUS_CMDQ_ID_S)

#define HTT_TX_TQM_CMDQ_STATUS_CMDQ_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_TQM_CMDQ_STATUS_CMDQ_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_TQM_CMDQ_STATUS_CMDQ_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [15 :  8]   :- cmdq_id
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 mac_id__cmdq_id__word;
    A_UINT32 sync_cmd;
    A_UINT32 write_cmd;
    A_UINT32 gen_mpdu_cmd;
    A_UINT32 mpdu_queue_stats_cmd;
    A_UINT32 mpdu_head_info_cmd;
    A_UINT32 msdu_flow_stats_cmd;
    A_UINT32 remove_mpdu_cmd;
    A_UINT32 remove_msdu_cmd;
    A_UINT32 flush_cache_cmd;
    A_UINT32 update_mpduq_cmd;
    A_UINT32 update_msduq_cmd;
} htt_tx_tqm_cmdq_status_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_TQM_CMDQ
 * TLV_TAGS:
 *     - HTT_STATS_STRING_TAG
 *     - HTT_STATS_TX_TQM_CMDQ_STATUS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    struct _cmdq_stats {
        htt_stats_string_tlv       cmdq_str_tlv;
        htt_tx_tqm_cmdq_status_tlv status_tlv;
    } q[1];
} htt_tx_tqm_cmdq_stats_t;

/* == TX-DE STATS == */

/* Structures for tx de stats */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      m1_packets;
    A_UINT32      m2_packets;
    A_UINT32      m3_packets;
    A_UINT32      m4_packets;
    A_UINT32      g1_packets;
    A_UINT32      g2_packets;
    A_UINT32      rc4_packets;
    A_UINT32      eap_packets;
    A_UINT32      eapol_start_packets;
    A_UINT32      eapol_logoff_packets;
    A_UINT32      eapol_encap_asf_packets;
} htt_tx_de_eapol_packets_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      ap_bss_peer_not_found;
    A_UINT32      ap_bcast_mcast_no_peer;
    A_UINT32      sta_delete_in_progress;
    A_UINT32      ibss_no_bss_peer;
    A_UINT32      invaild_vdev_type;
    A_UINT32      invalid_ast_peer_entry;
    A_UINT32      peer_entry_invalid;
    A_UINT32      ethertype_not_ip;
    A_UINT32      eapol_lookup_failed;
    A_UINT32      qpeer_not_allow_data;
    A_UINT32      fse_tid_override;
    A_UINT32      ipv6_jumbogram_zero_length;
    A_UINT32      qos_to_non_qos_in_prog;
    A_UINT32      ap_bcast_mcast_eapol;
    A_UINT32      unicast_on_ap_bss_peer;
    A_UINT32      ap_vdev_invalid;
    A_UINT32      incomplete_llc;
    A_UINT32      eapol_duplicate_m3;
    A_UINT32      eapol_duplicate_m4;
} htt_tx_de_classify_failed_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      arp_packets;
    A_UINT32      igmp_packets;
    A_UINT32      dhcp_packets;
    A_UINT32      host_inspected;
    A_UINT32      htt_included;
    A_UINT32      htt_valid_mcs;
    A_UINT32      htt_valid_nss;
    A_UINT32      htt_valid_preamble_type;
    A_UINT32      htt_valid_chainmask;
    A_UINT32      htt_valid_guard_interval;
    A_UINT32      htt_valid_retries;
    A_UINT32      htt_valid_bw_info;
    A_UINT32      htt_valid_power;
    A_UINT32      htt_valid_key_flags;
    A_UINT32      htt_valid_no_encryption;
    A_UINT32      fse_entry_count;
    A_UINT32      fse_priority_be;
    A_UINT32      fse_priority_high;
    A_UINT32      fse_priority_low;
    A_UINT32      fse_traffic_ptrn_be;
    A_UINT32      fse_traffic_ptrn_over_sub;
    A_UINT32      fse_traffic_ptrn_bursty;
    A_UINT32      fse_traffic_ptrn_interactive;
    A_UINT32      fse_traffic_ptrn_periodic;
    A_UINT32      fse_hwqueue_alloc;
    A_UINT32      fse_hwqueue_created;
    A_UINT32      fse_hwqueue_send_to_host;
    A_UINT32      mcast_entry;
    A_UINT32      bcast_entry;
    A_UINT32      htt_update_peer_cache;
    A_UINT32      htt_learning_frame;
    A_UINT32      fse_invalid_peer;
    /*
     * mec_notify is HTT TX WBM multicast echo check notification
     * from firmware to host.  FW sends SA addresses to host for all
     * multicast/broadcast packets received on STA side.
     */
    A_UINT32 mec_notify;
} htt_tx_de_classify_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      eok;
    A_UINT32      classify_done;
    A_UINT32      lookup_failed;
    A_UINT32      send_host_dhcp;
    A_UINT32      send_host_mcast;
    A_UINT32      send_host_unknown_dest;
    A_UINT32      send_host;
    A_UINT32      status_invalid;
} htt_tx_de_classify_status_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      enqueued_pkts;
    A_UINT32      to_tqm;
    A_UINT32      to_tqm_bypass;
} htt_tx_de_enqueue_packets_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      discarded_pkts;
    A_UINT32      local_frames;
    A_UINT32      is_ext_msdu;
} htt_tx_de_enqueue_discard_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      tcl_dummy_frame;
    A_UINT32      tqm_dummy_frame;
    A_UINT32      tqm_notify_frame;
    A_UINT32      fw2wbm_enq;
    A_UINT32      tqm_bypass_frame;
} htt_tx_de_compl_stats_tlv;

#define HTT_TX_DE_CMN_STATS_MAC_ID_M 0x000000ff
#define HTT_TX_DE_CMN_STATS_MAC_ID_S 0

#define HTT_TX_DE_CMN_STATS_MAC_ID_GET(_var) \
    (((_var) & HTT_TX_DE_CMN_STATS_MAC_ID_M) >> \
     HTT_TX_DE_CMN_STATS_MAC_ID_S)

#define HTT_TX_DE_CMN_STATS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_DE_CMN_STATS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_DE_CMN_STATS_MAC_ID_S)); \
    } while (0)

/*
 *  The htt_tx_de_fw2wbm_ring_full_hist_tlv is a histogram of time we waited
 *  for the fw2wbm ring buffer.  we are requesting a buffer in FW2WBM release
 *  ring,which may fail, due to non availability of buffer. Hence we sleep for
 *  200us & again request for it. This is a histogram of time we wait, with
 *  bin of 200ms & there are 10 bin (2 seconds max)
 *  They are defined by the following macros in FW
 *  #define ENTRIES_PER_BIN_COUNT 1000  // per bin 1000 * 200us = 200ms
 *  #define RING_FULL_BIN_ENTRIES (WAL_TX_DE_FW2WBM_ALLOC_TIMEOUT_COUNT /
 *                               ENTRIES_PER_BIN_COUNT)
 */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 fw2wbm_ring_full_hist[1];
} htt_tx_de_fw2wbm_ring_full_hist_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;

    /* Global Stats */
    A_UINT32 tcl2fw_entry_count;
    A_UINT32 not_to_fw;
    A_UINT32 invalid_pdev_vdev_peer;
    A_UINT32 tcl_res_invalid_addrx;
    A_UINT32 wbm2fw_entry_count;
    A_UINT32 invalid_pdev;
    A_UINT32 tcl_res_addrx_timeout;
    A_UINT32 invalid_vdev;
    A_UINT32 invalid_tcl_exp_frame_desc;
} htt_tx_de_cmn_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_TX_DE_INFO
 * TLV_TAGS:
 *     - HTT_STATS_TX_DE_CMN_TAG
 *     - HTT_STATS_TX_DE_FW2WBM_RING_FULL_HIST_TAG
 *     - HTT_STATS_TX_DE_EAPOL_PACKETS_TAG
 *     - HTT_STATS_TX_DE_CLASSIFY_STATS_TAG
 *     - HTT_STATS_TX_DE_CLASSIFY_FAILED_TAG
 *     - HTT_STATS_TX_DE_CLASSIFY_STATUS_TAG
 *     - HTT_STATS_TX_DE_ENQUEUE_PACKETS_TAG
 *     - HTT_STATS_TX_DE_ENQUEUE_DISCARD_TAG
 *     - HTT_STATS_TX_DE_COMPL_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_tx_de_cmn_stats_tlv             cmn_tlv;
    htt_tx_de_fw2wbm_ring_full_hist_tlv fw2wbm_hist_tlv;
    htt_tx_de_eapol_packets_stats_tlv   eapol_stats_tlv;
    htt_tx_de_classify_stats_tlv        classify_stats_tlv;
    htt_tx_de_classify_failed_stats_tlv classify_failed_tlv;
    htt_tx_de_classify_status_stats_tlv classify_status_rlv;
    htt_tx_de_enqueue_packets_stats_tlv enqueue_packets_tlv;
    htt_tx_de_enqueue_discard_stats_tlv enqueue_discard_tlv;
    htt_tx_de_compl_stats_tlv           comp_status_tlv;
} htt_tx_de_stats_t;

/* == RING-IF STATS == */
/* DWORD num_elems__prefetch_tail_idx */
#define HTT_RING_IF_STATS_NUM_ELEMS_M 0x0000ffff
#define HTT_RING_IF_STATS_NUM_ELEMS_S 0

#define HTT_RING_IF_STATS_PREFETCH_TAIL_IDX_M 0xffff0000
#define HTT_RING_IF_STATS_PREFETCH_TAIL_IDX_S 16

#define HTT_RING_IF_STATS_NUM_ELEMS_GET(_var) \
    (((_var) & HTT_RING_IF_STATS_NUM_ELEMS_M) >> \
     HTT_RING_IF_STATS_NUM_ELEMS_S)

#define HTT_RING_IF_STATS_NUM_ELEMS_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RING_IF_STATS_NUM_ELEMS, _val); \
        ((_var) |= ((_val) << HTT_RING_IF_STATS_NUM_ELEMS_S)); \
    } while (0)

#define HTT_RING_IF_STATS_PREFETCH_TAIL_IDX_GET(_var) \
    (((_var) & HTT_RING_IF_STATS_PREFETCH_TAIL_IDX_M) >> \
     HTT_RING_IF_STATS_PREFETCH_TAIL_IDX_S)

#define HTT_RING_IF_STATS_PREFETCH_TAIL_IDX_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RING_IF_STATS_PREFETCH_TAIL_IDX, _val); \
        ((_var) |= ((_val) << HTT_RING_IF_STATS_PREFETCH_TAIL_IDX_S)); \
    } while (0)

/* DWORD head_idx__tail_idx */
#define HTT_RING_IF_STATS_HEAD_IDX_M 0x0000ffff
#define HTT_RING_IF_STATS_HEAD_IDX_S 0

#define HTT_RING_IF_STATS_TAIL_IDX_M 0xffff0000
#define HTT_RING_IF_STATS_TAIL_IDX_S 16

#define HTT_RING_IF_STATS_HEAD_IDX_GET(_var) \
    (((_var) & HTT_RING_IF_STATS_HEAD_IDX_M) >> \
     HTT_RING_IF_STATS_HEAD_IDX_S)

#define HTT_RING_IF_STATS_HEAD_IDX_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RING_IF_STATS_HEAD_IDX, _val); \
        ((_var) |= ((_val) << HTT_RING_IF_STATS_HEAD_IDX_S)); \
    } while (0)

#define HTT_RING_IF_STATS_TAIL_IDX_GET(_var) \
    (((_var) & HTT_RING_IF_STATS_TAIL_IDX_M) >> \
     HTT_RING_IF_STATS_TAIL_IDX_S)

#define HTT_RING_IF_STATS_TAIL_IDX_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RING_IF_STATS_TAIL_IDX, _val); \
        ((_var) |= ((_val) << HTT_RING_IF_STATS_TAIL_IDX_S)); \
    } while (0)

/* DWORD shadow_head_idx__shadow_tail_idx */
#define HTT_RING_IF_STATS_SHADOW_HEAD_IDX_M 0x0000ffff
#define HTT_RING_IF_STATS_SHADOW_HEAD_IDX_S 0

#define HTT_RING_IF_STATS_SHADOW_TAIL_IDX_M 0xffff0000
#define HTT_RING_IF_STATS_SHADOW_TAIL_IDX_S 16

#define HTT_RING_IF_STATS_SHADOW_HEAD_IDX_GET(_var) \
    (((_var) & HTT_RING_IF_STATS_SHADOW_HEAD_IDX_M) >> \
     HTT_RING_IF_STATS_SHADOW_HEAD_IDX_S)

#define HTT_RING_IF_STATS_SHADOW_HEAD_IDX_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RING_IF_STATS_SHADOW_HEAD_IDX, _val); \
        ((_var) |= ((_val) << HTT_RING_IF_STATS_SHADOW_HEAD_IDX_S)); \
    } while (0)

#define HTT_RING_IF_STATS_SHADOW_TAIL_IDX_GET(_var) \
    (((_var) & HTT_RING_IF_STATS_SHADOW_TAIL_IDX_M) >> \
     HTT_RING_IF_STATS_SHADOW_TAIL_IDX_S)

#define HTT_RING_IF_STATS_SHADOW_TAIL_IDX_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RING_IF_STATS_SHADOW_TAIL_IDX, _val); \
        ((_var) |= ((_val) << HTT_RING_IF_STATS_SHADOW_TAIL_IDX_S)); \
    } while (0)

/* DWORD lwm_thresh__hwm_thresh */
#define HTT_RING_IF_STATS_LWM_THRESHOLD_M 0x0000ffff
#define HTT_RING_IF_STATS_LWM_THRESHOLD_S 0

#define HTT_RING_IF_STATS_HWM_THRESHOLD_M 0xffff0000
#define HTT_RING_IF_STATS_HWM_THRESHOLD_S 16

#define HTT_RING_IF_STATS_LWM_THRESHOLD_GET(_var) \
    (((_var) & HTT_RING_IF_STATS_LWM_THRESHOLD_M) >> \
     HTT_RING_IF_STATS_LWM_THRESHOLD_S)

#define HTT_RING_IF_STATS_LWM_THRESHOLD_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RING_IF_STATS_LWM_THRESHOLD, _val); \
        ((_var) |= ((_val) << HTT_RING_IF_STATS_LWM_THRESHOLD_S)); \
    } while (0)

#define HTT_RING_IF_STATS_HWM_THRESHOLD_GET(_var) \
    (((_var) & HTT_RING_IF_STATS_HWM_THRESHOLD_M) >> \
     HTT_RING_IF_STATS_HWM_THRESHOLD_S)

#define HTT_RING_IF_STATS_HWM_THRESHOLD_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RING_IF_STATS_HWM_THRESHOLD, _val); \
        ((_var) |= ((_val) << HTT_RING_IF_STATS_HWM_THRESHOLD_S)); \
    } while (0)

#define HTT_STATS_LOW_WM_BINS 5
#define HTT_STATS_HIGH_WM_BINS 5

typedef struct {
    A_UINT32 base_addr; /* DWORD aligned base memory address of the ring */

    A_UINT32 elem_size; /* size of each ring element */

    /* BIT [15 :  0]   :- num_elems
     * BIT [31 : 16]   :- prefetch_tail_idx
     */
    A_UINT32 num_elems__prefetch_tail_idx;
    /* BIT [15 :  0]   :- head_idx
     * BIT [31 : 16]   :- tail_idx
     */
    A_UINT32 head_idx__tail_idx;
    /* BIT [15 :  0]   :- shadow_head_idx
     * BIT [31 : 16]   :- shadow_tail_idx
     */
    A_UINT32 shadow_head_idx__shadow_tail_idx;
    A_UINT32 num_tail_incr;
    /* BIT [15 :  0]   :- lwm_thresh
     * BIT [31 : 16]   :- hwm_thresh
     */
    A_UINT32 lwm_thresh__hwm_thresh;
    A_UINT32 overrun_hit_count;
    A_UINT32 underrun_hit_count;
    A_UINT32 prod_blockwait_count;
    A_UINT32 cons_blockwait_count;
    A_UINT32 low_wm_hit_count[HTT_STATS_LOW_WM_BINS];   /* FIX THIS: explain what each array element is for */
    A_UINT32 high_wm_hit_count[HTT_STATS_HIGH_WM_BINS]; /* FIX THIS: explain what each array element is for */
} htt_ring_if_stats_tlv;

#define HTT_RING_IF_CMN_MAC_ID_M 0x000000ff
#define HTT_RING_IF_CMN_MAC_ID_S 0

#define HTT_RING_IF_CMN_MAC_ID_GET(_var) \
    (((_var) & HTT_RING_IF_CMN_MAC_ID_M) >> \
     HTT_RING_IF_CMN_MAC_ID_S)

#define HTT_RING_IF_CMN_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RING_IF_CMN_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_RING_IF_CMN_MAC_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    A_UINT32 num_records;
} htt_ring_if_cmn_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_RING_IF_INFO
 * TLV_TAGS:
 *     - HTT_STATS_RING_IF_CMN_TAG
 *     - HTT_STATS_STRING_TAG
 *     - HTT_STATS_RING_IF_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_ring_if_cmn_tlv cmn_tlv;
    /* Variable based on the Number of records. */
    struct _ring_if {
        htt_stats_string_tlv  ring_str_tlv;
        htt_ring_if_stats_tlv ring_tlv;
    } r[1];
} htt_ring_if_stats_t;

/* == SFM STATS == */

#define HTT_SFM_CLIENT_USER_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Number of DWORDS used per user and per client */
    A_UINT32 dwords_used_by_user_n[1];
} htt_sfm_client_user_tlv_v;

typedef struct  {
    htt_tlv_hdr_t tlv_hdr;
    /* Client ID */
    A_UINT32 client_id;
    /* Minimum number of buffers */
    A_UINT32 buf_min;
    /* Maximum number of buffers */
    A_UINT32 buf_max;
    /* Number of Busy buffers */
    A_UINT32 buf_busy;
    /* Number of Allocated buffers */
    A_UINT32 buf_alloc;
    /* Number of Available/Usable buffers */
    A_UINT32 buf_avail;
    /* Number of users */
    A_UINT32 num_users;
} htt_sfm_client_tlv;

#define HTT_SFM_CMN_MAC_ID_M 0x000000ff
#define HTT_SFM_CMN_MAC_ID_S 0

#define HTT_SFM_CMN_MAC_ID_GET(_var) \
    (((_var) & HTT_SFM_CMN_MAC_ID_M) >> \
     HTT_SFM_CMN_MAC_ID_S)

#define HTT_SFM_CMN_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SFM_CMN_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_SFM_CMN_MAC_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /* Indicates the total number of 128 byte buffers in the CMEM that are available for buffer sharing */
    A_UINT32 buf_total;
    /* Indicates for certain client or all the clients there is no dowrd saved in SFM, refer to SFM_R1_MEM_EMPTY */
    A_UINT32 mem_empty;
    /* DEALLOCATE_BUFFERS, refer to register SFM_R0_DEALLOCATE_BUFFERS */
    A_UINT32 deallocate_bufs;
    /* Number of Records */
    A_UINT32 num_records;
} htt_sfm_cmn_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_RING_IF_INFO
 * TLV_TAGS:
 *    - HTT_STATS_SFM_CMN_TAG
 *    - HTT_STATS_STRING_TAG
 *    - HTT_STATS_SFM_CLIENT_TAG
 *    - HTT_STATS_SFM_CLIENT_USER_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_sfm_cmn_tlv cmn_tlv;
    /* Variable based on the Number of records. */
    struct _sfm_client {
        htt_stats_string_tlv      client_str_tlv;
        htt_sfm_client_tlv        client_tlv;
        htt_sfm_client_user_tlv_v user_tlv;
    } r[1];
} htt_sfm_stats_t;

/* == SRNG STATS == */
/* DWORD mac_id__ring_id__arena__ep */
#define HTT_SRING_STATS_MAC_ID_M 0x000000ff
#define HTT_SRING_STATS_MAC_ID_S 0

#define HTT_SRING_STATS_RING_ID_M 0x0000ff00
#define HTT_SRING_STATS_RING_ID_S 8

#define HTT_SRING_STATS_ARENA_M 0x00ff0000
#define HTT_SRING_STATS_ARENA_S 16

#define HTT_SRING_STATS_EP_TYPE_M 0x01000000
#define HTT_SRING_STATS_EP_TYPE_S 24

#define HTT_SRING_STATS_MAC_ID_GET(_var) \
    (((_var) & HTT_SRING_STATS_MAC_ID_M) >> \
     HTT_SRING_STATS_MAC_ID_S)

#define HTT_SRING_STATS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_MAC_ID_S)); \
    } while (0)

#define HTT_SRING_STATS_RING_ID_GET(_var) \
    (((_var) & HTT_SRING_STATS_RING_ID_M) >> \
     HTT_SRING_STATS_RING_ID_S)

#define HTT_SRING_STATS_RING_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_RING_ID, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_RING_ID_S)); \
    } while (0)

#define HTT_SRING_STATS_ARENA_GET(_var) \
    (((_var) & HTT_SRING_STATS_ARENA_M) >> \
     HTT_SRING_STATS_ARENA_S)

#define HTT_SRING_STATS_ARENA_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_ARENA, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_ARENA_S)); \
    } while (0)

#define HTT_SRING_STATS_EP_TYPE_GET(_var) \
    (((_var) & HTT_SRING_STATS_EP_TYPE_M) >> \
     HTT_SRING_STATS_EP_TYPE_S)

#define HTT_SRING_STATS_EP_TYPE_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_EP_TYPE, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_EP_TYPE_S)); \
    } while (0)

/* DWORD num_avail_words__num_valid_words */
#define HTT_SRING_STATS_NUM_AVAIL_WORDS_M 0x0000ffff
#define HTT_SRING_STATS_NUM_AVAIL_WORDS_S 0

#define HTT_SRING_STATS_NUM_VALID_WORDS_M 0xffff0000
#define HTT_SRING_STATS_NUM_VALID_WORDS_S 16

#define HTT_SRING_STATS_NUM_AVAIL_WORDS_GET(_var) \
    (((_var) & HTT_SRING_STATS_NUM_AVAIL_WORDS_M) >> \
     HTT_SRING_STATS_NUM_AVAIL_WORDS_S)

#define HTT_SRING_STATS_NUM_AVAIL_WORDS_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_NUM_AVAIL_WORDS, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_NUM_AVAIL_WORDS_S)); \
    } while (0)

#define HTT_SRING_STATS_NUM_VALID_WORDS_GET(_var) \
    (((_var) & HTT_SRING_STATS_NUM_VALID_WORDS_M) >> \
     HTT_SRING_STATS_NUM_VALID_WORDS_S)

#define HTT_SRING_STATS_NUM_VALID_WORDS_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_NUM_VALID_WORDS, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_NUM_VALID_WORDS_S)); \
    } while (0)

/* DWORD head_ptr__tail_ptr */
#define HTT_SRING_STATS_HEAD_PTR_M 0x0000ffff
#define HTT_SRING_STATS_HEAD_PTR_S 0

#define HTT_SRING_STATS_TAIL_PTR_M 0xffff0000
#define HTT_SRING_STATS_TAIL_PTR_S 16

#define HTT_SRING_STATS_HEAD_PTR_GET(_var) \
    (((_var) & HTT_SRING_STATS_HEAD_PTR_M) >> \
     HTT_SRING_STATS_HEAD_PTR_S)

#define HTT_SRING_STATS_HEAD_PTR_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_HEAD_PTR, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_HEAD_PTR_S)); \
    } while (0)

#define HTT_SRING_STATS_TAIL_PTR_GET(_var) \
    (((_var) & HTT_SRING_STATS_TAIL_PTR_M) >> \
     HTT_SRING_STATS_TAIL_PTR_S)

#define HTT_SRING_STATS_TAIL_PTR_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_TAIL_PTR, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_TAIL_PTR_S)); \
    } while (0)

/* DWORD consumer_empty__producer_full */
#define HTT_SRING_STATS_CONSUMER_EMPTY_M 0x0000ffff
#define HTT_SRING_STATS_CONSUMER_EMPTY_S 0

#define HTT_SRING_STATS_PRODUCER_FULL_M 0xffff0000
#define HTT_SRING_STATS_PRODUCER_FULL_S 16

#define HTT_SRING_STATS_CONSUMER_EMPTY_GET(_var) \
    (((_var) & HTT_SRING_STATS_CONSUMER_EMPTY_M) >> \
     HTT_SRING_STATS_CONSUMER_EMPTY_S)

#define HTT_SRING_STATS_CONSUMER_EMPTY_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_CONSUMER_EMPTY, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_CONSUMER_EMPTY_S)); \
    } while (0)

#define HTT_SRING_STATS_PRODUCER_FULL_GET(_var) \
    (((_var) & HTT_SRING_STATS_PRODUCER_FULL_M) >> \
     HTT_SRING_STATS_PRODUCER_FULL_S)

#define HTT_SRING_STATS_PRODUCER_FULL_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_PRODUCER_FULL, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_PRODUCER_FULL_S)); \
    } while (0)

/* DWORD prefetch_count__internal_tail_ptr */
#define HTT_SRING_STATS_PREFETCH_COUNT_M 0x0000ffff
#define HTT_SRING_STATS_PREFETCH_COUNT_S 0

#define HTT_SRING_STATS_INTERNAL_TP_M 0xffff0000
#define HTT_SRING_STATS_INTERNAL_TP_S 16

#define HTT_SRING_STATS_PREFETCH_COUNT_GET(_var) \
    (((_var) & HTT_SRING_STATS_PREFETCH_COUNT_M) >> \
     HTT_SRING_STATS_PREFETCH_COUNT_S)

#define HTT_SRING_STATS_PREFETCH_COUNT_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_PREFETCH_COUNT, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_PREFETCH_COUNT_S)); \
    } while (0)

#define HTT_SRING_STATS_INTERNAL_TP_GET(_var) \
    (((_var) & HTT_SRING_STATS_INTERNAL_TP_M) >> \
     HTT_SRING_STATS_INTERNAL_TP_S)

#define HTT_SRING_STATS_INTERNAL_TP_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_SRING_STATS_INTERNAL_TP, _val); \
        ((_var) |= ((_val) << HTT_SRING_STATS_INTERNAL_TP_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [15 :  8]   :- ring_id
     * BIT [23 : 16]   :- arena 0 -SRING_HRAM, 1 - SRING_HCRAM, 2 - SRING_HW2HW.
     * BIT [24 : 24]   :- EP 0 -consumer, 1 - producer
     * BIT [31 : 25]   :- reserved
     */
    A_UINT32 mac_id__ring_id__arena__ep;
    A_UINT32 base_addr_lsb; /* DWORD aligned base memory address of the ring */
    A_UINT32 base_addr_msb;
    A_UINT32 ring_size; /* size of ring */
    A_UINT32 elem_size; /* size of each ring element */

    /* Ring status */
    /* BIT [15 :  0]   :- num_avail_words
     * BIT [31 : 16]   :- num_valid_words
     */
    A_UINT32 num_avail_words__num_valid_words;

    /* Index of head and tail */
    /* BIT [15 :  0]   :- head_ptr
     * BIT [31 : 16]   :- tail_ptr
     */
    A_UINT32 head_ptr__tail_ptr;

    /* Empty or full counter of rings */
    /* BIT [15 :  0]   :- consumer_empty
     * BIT [31 : 16]   :- producer_full
     */
    A_UINT32 consumer_empty__producer_full;

    /* Prefetch status of consumer ring */
    /* BIT [15 :  0]   :- prefetch_count
     * BIT [31 : 16]   :- internal_tail_ptr
     */
    A_UINT32 prefetch_count__internal_tail_ptr;
} htt_sring_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      num_records;
} htt_sring_cmn_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_SRNG_INFO
 * TLV_TAGS:
 *     - HTT_STATS_SRING_CMN_TAG
 *     - HTT_STATS_STRING_TAG
 *     - HTT_STATS_SRING_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_sring_cmn_tlv cmn_tlv;
    /* Variable based on the Number of records. */
    struct _sring_stats {
        htt_stats_string_tlv sring_str_tlv;
        htt_sring_stats_tlv  sring_stats_tlv;
    } r[1];
} htt_sring_stats_t;

/* == PDEV TX RATE CTRL STATS == */

#define HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS 12
#define HTT_TX_PDEV_STATS_NUM_GI_COUNTERS 4
#define HTT_TX_PDEV_STATS_NUM_DCM_COUNTERS 5
#define HTT_TX_PDEV_STATS_NUM_BW_COUNTERS 4
#define HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS 8
#define HTT_TX_PDEV_STATS_NUM_PREAMBLE_TYPES HTT_STATS_PREAM_COUNT
#define HTT_TX_PDEV_STATS_NUM_LEGACY_CCK_STATS 4
#define HTT_TX_PDEV_STATS_NUM_LEGACY_OFDM_STATS 8
#define HTT_TX_PDEV_STATS_NUM_LTF 4
#define HTT_TX_PDEV_STATS_NUM_11AX_TRIGGER_TYPES 6
#define HTT_TX_NUM_OF_SOUNDING_STATS_WORDS \
    (HTT_TX_PDEV_STATS_NUM_BW_COUNTERS * \
     HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS)

#define HTT_TX_PDEV_RATE_STATS_MAC_ID_M 0x000000ff
#define HTT_TX_PDEV_RATE_STATS_MAC_ID_S 0

#define HTT_TX_PDEV_RATE_STATS_MAC_ID_GET(_var) \
    (((_var) & HTT_TX_PDEV_RATE_STATS_MAC_ID_M) >> \
     HTT_TX_PDEV_RATE_STATS_MAC_ID_S)

#define HTT_TX_PDEV_RATE_STATS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TX_PDEV_RATE_STATS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_TX_PDEV_RATE_STATS_MAC_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /* Number of tx ldpc packets */
    A_UINT32 tx_ldpc;
    /* Number of tx rts packets */
    A_UINT32 rts_cnt;
    /* RSSI value of last ack packet (units = dB above noise floor) */
    A_UINT32 ack_rssi;

    A_UINT32 tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];

    /* tx_xx_mcs: currently unused */
    A_UINT32 tx_su_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 tx_mu_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];

    A_UINT32 tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS]; /* element 0,1, ...7 -> NSS 1,2, ...8 */
    A_UINT32 tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];      /* element 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160 and 80+80 MHz */
    A_UINT32 tx_stbc[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 tx_pream[HTT_TX_PDEV_STATS_NUM_PREAMBLE_TYPES];

    /* Counters to track number of tx packets in each GI (400us, 800us, 1600us & 3200us) in each mcs (0-11) */
    A_UINT32 tx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];

    /* Counters to track packets in dcm mcs (MCS 0, 1, 3, 4) */
    A_UINT32 tx_dcm[HTT_TX_PDEV_STATS_NUM_DCM_COUNTERS];
    /* Number of CTS-acknowledged RTS packets */
    A_UINT32 rts_success;

    /*
     * Counters for legacy 11a and 11b transmissions.
     *
     * The index corresponds to:
     *
     * CCK: 0: 1 Mbps, 1: 2 Mbps, 2: 5.5 Mbps, 3: 11 Mbps
     *
     * OFDM: 0: 6 Mbps, 1: 9 Mbps, 2: 12 Mbps, 3: 18 Mbps,
     *       4: 24 Mbps, 5: 36 Mbps, 6: 48 Mbps, 7: 54 Mbps
     */
    A_UINT32 tx_legacy_cck_rate[HTT_TX_PDEV_STATS_NUM_LEGACY_CCK_STATS];
    A_UINT32 tx_legacy_ofdm_rate[HTT_TX_PDEV_STATS_NUM_LEGACY_OFDM_STATS];

    A_UINT32 ac_mu_mimo_tx_ldpc;
    A_UINT32 ax_mu_mimo_tx_ldpc;
    A_UINT32 ofdma_tx_ldpc;

    /*
     * Counters for 11ax HE LTF selection during TX.
     *
     * The index corresponds to:
     *
     * 0: unused, 1: 1x LTF, 2: 2x LTF, 3: 4x LTF
     */
    A_UINT32 tx_he_ltf[HTT_TX_PDEV_STATS_NUM_LTF];

    A_UINT32 ac_mu_mimo_tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 ax_mu_mimo_tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 ofdma_tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];

    A_UINT32 ac_mu_mimo_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 ax_mu_mimo_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 ofdma_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];

    A_UINT32 ac_mu_mimo_tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 ax_mu_mimo_tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 ofdma_tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];

    A_UINT32 ac_mu_mimo_tx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 ax_mu_mimo_tx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 ofdma_tx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 trigger_type_11ax[HTT_TX_PDEV_STATS_NUM_11AX_TRIGGER_TYPES];
    A_UINT32 tx_11ax_su_ext;
} htt_tx_pdev_rate_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_TX_RATE
 * TLV_TAGS:
 *      - HTT_STATS_TX_PDEV_RATE_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_tx_pdev_rate_stats_tlv rate_tlv;
} htt_tx_pdev_rate_stats_t;

/* == PDEV RX RATE CTRL STATS == */

#define HTT_RX_PDEV_STATS_NUM_LEGACY_CCK_STATS 4
#define HTT_RX_PDEV_STATS_NUM_LEGACY_OFDM_STATS 8
#define HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS 12
#define HTT_RX_PDEV_STATS_NUM_GI_COUNTERS 4
#define HTT_RX_PDEV_STATS_NUM_DCM_COUNTERS 5
#define HTT_RX_PDEV_STATS_NUM_BW_COUNTERS 4
#define HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS 8
#define HTT_RX_PDEV_STATS_NUM_PREAMBLE_TYPES HTT_STATS_PREAM_COUNT
#define HTT_RX_PDEV_MAX_OFDMA_NUM_USER 8
#define HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER 8
#define HTT_RX_PDEV_STATS_RXEVM_MAX_PILOTS_PER_NSS 16
#define HTT_RX_PDEV_STATS_NUM_RU_SIZE_COUNTERS 6

#define HTT_RX_PDEV_RATE_STATS_MAC_ID_M 0x000000ff
#define HTT_RX_PDEV_RATE_STATS_MAC_ID_S 0

#define HTT_RX_PDEV_RATE_STATS_MAC_ID_GET(_var) \
    (((_var) & HTT_RX_PDEV_RATE_STATS_MAC_ID_M) >> \
     HTT_RX_PDEV_RATE_STATS_MAC_ID_S)

#define HTT_RX_PDEV_RATE_STATS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RX_PDEV_RATE_STATS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_RX_PDEV_RATE_STATS_MAC_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    A_UINT32 nsts;

    /* Number of rx ldpc packets */
    A_UINT32 rx_ldpc;
    /* Number of rx rts packets */
    A_UINT32 rts_cnt;

    A_UINT32 rssi_mgmt; /* units = dB above noise floor */
    A_UINT32 rssi_data; /* units = dB above noise floor */
    A_UINT32 rssi_comb; /* units = dB above noise floor */
    A_UINT32 rx_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 rx_nss[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS]; /* element 0,1, ...7 -> NSS 1,2, ...8 */
    A_UINT32 rx_dcm[HTT_RX_PDEV_STATS_NUM_DCM_COUNTERS];
    A_UINT32 rx_stbc[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 rx_bw[HTT_RX_PDEV_STATS_NUM_BW_COUNTERS]; /* element 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160 and 80+80 MHz */
    A_UINT32 rx_pream[HTT_RX_PDEV_STATS_NUM_PREAMBLE_TYPES];
    A_UINT8  rssi_chain[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_STATS_NUM_BW_COUNTERS]; /* units = dB above noise floor */

    /* Counters to track number of rx packets in each GI in each mcs (0-11) */
    A_UINT32 rx_gi[HTT_RX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_INT32  rssi_in_dbm; /* rx Signal Strength value in dBm unit */

    A_UINT32 rx_11ax_su_ext;
    A_UINT32 rx_11ac_mumimo;
    A_UINT32 rx_11ax_mumimo;
    A_UINT32 rx_11ax_ofdma;
    A_UINT32 txbf;
    A_UINT32 rx_legacy_cck_rate[HTT_RX_PDEV_STATS_NUM_LEGACY_CCK_STATS];
    A_UINT32 rx_legacy_ofdm_rate[HTT_RX_PDEV_STATS_NUM_LEGACY_OFDM_STATS];
    A_UINT32 rx_active_dur_us_low;
    A_UINT32 rx_active_dur_us_high;

    A_UINT32 rx_11ax_ul_ofdma;

    A_UINT32 ul_ofdma_rx_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 ul_ofdma_rx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 ul_ofdma_rx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 ul_ofdma_rx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 ul_ofdma_rx_stbc;
    A_UINT32 ul_ofdma_rx_ldpc;

    /* record the stats for each user index */
    A_UINT32 rx_ulofdma_non_data_ppdu[HTT_RX_PDEV_MAX_OFDMA_NUM_USER]; /* ppdu level */
    A_UINT32 rx_ulofdma_data_ppdu[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];     /* ppdu level */
    A_UINT32 rx_ulofdma_mpdu_ok[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];       /* mpdu level */
    A_UINT32 rx_ulofdma_mpdu_fail[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];     /* mpdu level */

    A_UINT32 nss_count;
    A_UINT32 pilot_count;
    /* RxEVM stats in dB */
    A_INT32 rx_pilot_evm_dB[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_STATS_RXEVM_MAX_PILOTS_PER_NSS];
    /* rx_pilot_evm_dB_mean:
     * EVM mean across pilots, computed as
     *     mean(10*log10(rx_pilot_evm_linear)) = mean(rx_pilot_evm_dB)
     */
    A_INT32 rx_pilot_evm_dB_mean[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_INT8  rx_ul_fd_rssi[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_MAX_OFDMA_NUM_USER]; /* dBm units */
    /* per_chain_rssi_pkt_type:
     * This field shows what type of rx frame the per-chain RSSI was computed
     * on, by recording the frame type and sub-type as bit-fields within this
     * field:
     * BIT [3 : 0]    :- IEEE80211_FC0_TYPE
     * BIT [7 : 4]    :- IEEE80211_FC0_SUBTYPE
     * BIT [31 : 8]   :- Reserved
     */
    A_UINT32 per_chain_rssi_pkt_type;
    A_INT8   rx_per_chain_rssi_in_dbm[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 rx_su_ndpa;
    A_UINT32 rx_11ax_su_txbf_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 rx_mu_ndpa;
    A_UINT32 rx_11ax_mu_txbf_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 rx_br_poll;
    A_UINT32 rx_11ax_dl_ofdma_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 rx_11ax_dl_ofdma_ru[HTT_RX_PDEV_STATS_NUM_RU_SIZE_COUNTERS];

    A_UINT32 rx_ulmumimo_non_data_ppdu[HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER]; /* ppdu level */
    A_UINT32 rx_ulmumimo_data_ppdu[HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER];     /* ppdu level */
    A_UINT32 rx_ulmumimo_mpdu_ok[HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER];       /* mpdu level */
    A_UINT32 rx_ulmumimo_mpdu_fail[HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER];     /* mpdu level */
} htt_rx_pdev_rate_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_RX_RATE
 * TLV_TAGS:
 *      - HTT_STATS_RX_PDEV_RATE_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_rx_pdev_rate_stats_tlv rate_tlv;
} htt_rx_pdev_rate_stats_t;

/* == RX PDEV/SOC STATS == */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Num Packets received on REO FW ring */
    A_UINT32 fw_reo_ring_data_msdu;
    /* Num bc/mc packets indicated from fw to host */
    A_UINT32 fw_to_host_data_msdu_bcmc;
    /* Num unicast packets indicated from fw to host */
    A_UINT32 fw_to_host_data_msdu_uc;
    /* Num remote buf recycle from offload  */
    A_UINT32 ofld_remote_data_buf_recycle_cnt;
    /* Num remote free buf given to offload */
    A_UINT32 ofld_remote_free_buf_indication_cnt;

    /* Num unicast packets from local path indicated to host */
    A_UINT32 ofld_buf_to_host_data_msdu_uc;
    /* Num unicast packets from REO indicated to host */
    A_UINT32 reo_fw_ring_to_host_data_msdu_uc;

    /* Num Packets received from WBM SW1 ring */
    A_UINT32 wbm_sw_ring_reap;
    /* Num packets from WBM forwarded from fw to host via WBM */
    A_UINT32 wbm_forward_to_host_cnt;
    /* Num packets from WBM recycled to target refill ring */
    A_UINT32 wbm_target_recycle_cnt;

    /* Total Num of recycled to refill ring, including packets from WBM and REO */
    A_UINT32 target_refill_ring_recycle_cnt;
} htt_rx_soc_fw_stats_tlv;

#define HTT_RX_SOC_FW_REFILL_RING_EMPTY_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Num ring empty encountered */
    A_UINT32 refill_ring_empty_cnt[1]; /* HTT_RX_STATS_REFILL_MAX_RING */
} htt_rx_soc_fw_refill_ring_empty_tlv_v;

#define HTT_RX_SOC_FW_REFILL_RING_EMPTY_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Num total buf refilled from refill ring */
    A_UINT32 refill_ring_num_refill[1]; /* HTT_RX_STATS_REFILL_MAX_RING */
} htt_rx_soc_fw_refill_ring_num_refill_tlv_v;

/* RXDMA error code from WBM released packets */
typedef enum {
    HTT_RX_RXDMA_OVERFLOW_ERR     = 0,
    HTT_RX_RXDMA_MPDU_LENGTH_ERR  = 1,
    HTT_RX_RXDMA_FCS_ERR = 2,
    HTT_RX_RXDMA_DECRYPT_ERR      = 3,
    HTT_RX_RXDMA_TKIP_MIC_ERR     = 4,
    HTT_RX_RXDMA_UNECRYPTED_ERR   = 5,
    HTT_RX_RXDMA_MSDU_LEN_ERR     = 6,
    HTT_RX_RXDMA_MSDU_LIMIT_ERR   = 7,
    HTT_RX_RXDMA_WIFI_PARSE_ERR   = 8,
    HTT_RX_RXDMA_AMSDU_PARSE_ERR  = 9,
    HTT_RX_RXDMA_SA_TIMEOUT_ERR   = 10,
    HTT_RX_RXDMA_DA_TIMEOUT_ERR   = 11,
    HTT_RX_RXDMA_FLOW_TIMEOUT_ERR = 12,
    HTT_RX_RXDMA_FLUSH_REQUEST    = 13,
    HTT_RX_RXDMA_ERR_CODE_RVSD0   = 14,
    HTT_RX_RXDMA_ERR_CODE_RVSD1   = 15,

    /*
     * This MAX_ERR_CODE should not be used in any host/target messages,
     * so that even though it is defined within a host/target interface
     * definition header file, it isn't actually part of the host/target
     * interface, and thus can be modified.
     */
    HTT_RX_RXDMA_MAX_ERR_CODE
} htt_rx_rxdma_error_code_enum;

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* NOTE:
     * The mapping of RXDMA error types to rxdma_err array elements is HW dependent.
     * It is expected but not required that the target will provide a rxdma_err element
     * for each of the htt_rx_rxdma_error_code_enum values, up to but not including
     * MAX_ERR_CODE.  The host should ignore any array elements whose
     * indices are >= the MAX_ERR_CODE value the host was compiled with.
     */
    A_UINT32 rxdma_err[1]; /* HTT_RX_RXDMA_MAX_ERR_CODE */
} htt_rx_soc_fw_refill_ring_num_rxdma_err_tlv_v;

/* REO error code from WBM released packets */
typedef enum {
    HTT_RX_REO_QUEUE_DESC_ADDR_ZERO     = 0,
    HTT_RX_REO_QUEUE_DESC_NOT_VALID     = 1,
    HTT_RX_AMPDU_IN_NON_BA = 2,
    HTT_RX_NON_BA_DUPLICATE = 3,
    HTT_RX_BA_DUPLICATE = 4,
    HTT_RX_REGULAR_FRAME_2K_JUMP        = 5,
    HTT_RX_BAR_FRAME_2K_JUMP = 6,
    HTT_RX_REGULAR_FRAME_OOR = 7,
    HTT_RX_BAR_FRAME_OOR = 8,
    HTT_RX_BAR_FRAME_NO_BA_SESSION      = 9,
    HTT_RX_BAR_FRAME_SN_EQUALS_SSN      = 10,
    HTT_RX_PN_CHECK_FAILED = 11,
    HTT_RX_2K_ERROR_HANDLING_FLAG_SET   = 12,
    HTT_RX_PN_ERROR_HANDLING_FLAG_SET   = 13,
    HTT_RX_QUEUE_DESCRIPTOR_BLOCKED_SET = 14,
    HTT_RX_REO_ERR_CODE_RVSD = 15,

    /*
     * This MAX_ERR_CODE should not be used in any host/target messages,
     * so that even though it is defined within a host/target interface
     * definition header file, it isn't actually part of the host/target
     * interface, and thus can be modified.
     */
    HTT_RX_REO_MAX_ERR_CODE
} htt_rx_reo_error_code_enum;

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* NOTE:
     * The mapping of REO error types to reo_err array elements is HW dependent.
     * It is expected but not required that the target will provide a rxdma_err element
     * for each of the htt_rx_reo_error_code_enum values, up to but not including
     * MAX_ERR_CODE.  The host should ignore any array elements whose
     * indices are >= the MAX_ERR_CODE value the host was compiled with.
     */
    A_UINT32 reo_err[1]; /* HTT_RX_REO_MAX_ERR_CODE */
} htt_rx_soc_fw_refill_ring_num_reo_err_tlv_v;

/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_rx_soc_fw_stats_tlv                       fw_tlv;
    htt_rx_soc_fw_refill_ring_empty_tlv_v         fw_refill_ring_empty_tlv;
    htt_rx_soc_fw_refill_ring_num_refill_tlv_v    fw_refill_ring_num_refill_tlv;
    htt_rx_soc_fw_refill_ring_num_rxdma_err_tlv_v fw_refill_ring_num_rxdma_err_tlv;
    htt_rx_soc_fw_refill_ring_num_reo_err_tlv_v   fw_refill_ring_num_reo_err_tlv;
} htt_rx_soc_stats_t;

/* == RX PDEV STATS == */
#define HTT_RX_PDEV_FW_STATS_MAC_ID_M 0x000000ff
#define HTT_RX_PDEV_FW_STATS_MAC_ID_S 0

#define HTT_RX_PDEV_FW_STATS_MAC_ID_GET(_var) \
    (((_var) & HTT_RX_PDEV_FW_STATS_MAC_ID_M) >> \
     HTT_RX_PDEV_FW_STATS_MAC_ID_S)

#define HTT_RX_PDEV_FW_STATS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_RX_PDEV_FW_STATS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_RX_PDEV_FW_STATS_MAC_ID_S)); \
    } while (0)

#define HTT_STATS_SUBTYPE_MAX 16

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /* Num PPDU status processed from HW */
    A_UINT32 ppdu_recvd;
    /* Num MPDU across PPDUs with FCS ok */
    A_UINT32 mpdu_cnt_fcs_ok;
    /* Num MPDU across PPDUs with FCS err */
    A_UINT32 mpdu_cnt_fcs_err;
    /* Num MSDU across PPDUs */
    A_UINT32 tcp_msdu_cnt;
    /* Num MSDU across PPDUs */
    A_UINT32 tcp_ack_msdu_cnt;
    /* Num MSDU across PPDUs */
    A_UINT32 udp_msdu_cnt;
    /* Num MSDU across PPDUs */
    A_UINT32 other_msdu_cnt;
    /* Num MPDU on FW ring indicated */
    A_UINT32 fw_ring_mpdu_ind;
    /* Num MGMT MPDU given to protocol */
    A_UINT32 fw_ring_mgmt_subtype[HTT_STATS_SUBTYPE_MAX];
    /* Num ctrl MPDU given to protocol */
    A_UINT32 fw_ring_ctrl_subtype[HTT_STATS_SUBTYPE_MAX];
    /* Num mcast data packet received */
    A_UINT32 fw_ring_mcast_data_msdu;
    /* Num broadcast data packet received */
    A_UINT32 fw_ring_bcast_data_msdu;
    /* Num unicat data packet received */
    A_UINT32 fw_ring_ucast_data_msdu;
    /* Num null data packet received  */
    A_UINT32 fw_ring_null_data_msdu;
    /* Num MPDU on FW ring dropped */
    A_UINT32 fw_ring_mpdu_drop;

    /* Num buf indication to offload */
    A_UINT32 ofld_local_data_ind_cnt;
    /* Num buf recycle from offload */
    A_UINT32 ofld_local_data_buf_recycle_cnt;
    /* Num buf indication to data_rx */
    A_UINT32 drx_local_data_ind_cnt;
    /* Num buf recycle from data_rx */
    A_UINT32 drx_local_data_buf_recycle_cnt;
    /* Num buf indication to protocol */
    A_UINT32 local_nondata_ind_cnt;
    /* Num buf recycle from protocol */
    A_UINT32 local_nondata_buf_recycle_cnt;

    /* Num buf fed */
    A_UINT32 fw_status_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    A_UINT32 fw_status_buf_ring_empty_cnt;
    /* Num buf fed  */
    A_UINT32 fw_pkt_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    A_UINT32 fw_pkt_buf_ring_empty_cnt;
    /* Num buf fed  */
    A_UINT32 fw_link_buf_ring_refill_cnt;
    /* Num ring empty encountered  */
    A_UINT32 fw_link_buf_ring_empty_cnt;

    /* Num buf fed */
    A_UINT32 host_pkt_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    A_UINT32 host_pkt_buf_ring_empty_cnt;
    /* Num buf fed */
    A_UINT32 mon_pkt_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    A_UINT32 mon_pkt_buf_ring_empty_cnt;
    /* Num buf fed */
    A_UINT32 mon_status_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    A_UINT32 mon_status_buf_ring_empty_cnt;
    /* Num buf fed */
    A_UINT32 mon_desc_buf_ring_refill_cnt;
    /* Num ring empty encountered */
    A_UINT32 mon_desc_buf_ring_empty_cnt;
    /* Num buf fed */
    A_UINT32 mon_dest_ring_update_cnt;
    /* Num ring full encountered */
    A_UINT32 mon_dest_ring_full_cnt;

    /* Num rx suspend is attempted */
    A_UINT32 rx_suspend_cnt;
    /* Num rx suspend failed */
    A_UINT32 rx_suspend_fail_cnt;
    /* Num rx resume attempted */
    A_UINT32 rx_resume_cnt;
    /* Num rx resume failed */
    A_UINT32 rx_resume_fail_cnt;
    /* Num rx ring switch */
    A_UINT32 rx_ring_switch_cnt;
    /* Num rx ring restore */
    A_UINT32 rx_ring_restore_cnt;
    /* Num rx flush issued */
    A_UINT32 rx_flush_cnt;
    /* Num rx recovery */
    A_UINT32 rx_recovery_reset_cnt;
} htt_rx_pdev_fw_stats_tlv;

#define HTT_STATS_PHY_ERR_MAX 43

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /* Num of phy err */
    A_UINT32 total_phy_err_cnt;
    /* Counts of different types of phy errs
     * The mapping of PHY error types to phy_err array elements is HW dependent.
     * The only currently-supported mapping is shown below:
     *
     * 0 phyrx_err_phy_off Reception aborted due to receiving a PHY_OFF TLV
     * 1 phyrx_err_synth_off
     * 2 phyrx_err_ofdma_timing
     * 3 phyrx_err_ofdma_signal_parity
     * 4 phyrx_err_ofdma_rate_illegal
     * 5 phyrx_err_ofdma_length_illegal
     * 6 phyrx_err_ofdma_restart
     * 7 phyrx_err_ofdma_service
     * 8 phyrx_err_ppdu_ofdma_power_drop
     * 9 phyrx_err_cck_blokker
     * 10 phyrx_err_cck_timing
     * 11 phyrx_err_cck_header_crc
     * 12 phyrx_err_cck_rate_illegal
     * 13 phyrx_err_cck_length_illegal
     * 14 phyrx_err_cck_restart
     * 15 phyrx_err_cck_service
     * 16 phyrx_err_cck_power_drop
     * 17 phyrx_err_ht_crc_err
     * 18 phyrx_err_ht_length_illegal
     * 19 phyrx_err_ht_rate_illegal
     * 20 phyrx_err_ht_zlf
     * 21 phyrx_err_false_radar_ext
     * 22 phyrx_err_green_field
     * 23 phyrx_err_bw_gt_dyn_bw
     * 24 phyrx_err_leg_ht_mismatch
     * 25 phyrx_err_vht_crc_error
     * 26 phyrx_err_vht_siga_unsupported
     * 27 phyrx_err_vht_lsig_len_invalid
     * 28 phyrx_err_vht_ndp_or_zlf
     * 29 phyrx_err_vht_nsym_lt_zero
     * 30 phyrx_err_vht_rx_extra_symbol_mismatch
     * 31 phyrx_err_vht_rx_skip_group_id0
     * 32 phyrx_err_vht_rx_skip_group_id1to62
     * 33 phyrx_err_vht_rx_skip_group_id63
     * 34 phyrx_err_ofdm_ldpc_decoder_disabled
     * 35 phyrx_err_defer_nap
     * 36 phyrx_err_fdomain_timeout
     * 37 phyrx_err_lsig_rel_check
     * 38 phyrx_err_bt_collision
     * 39 phyrx_err_unsupported_mu_feedback
     * 40 phyrx_err_ppdu_tx_interrupt_rx
     * 41 phyrx_err_unsupported_cbf
     * 42 phyrx_err_other
     */
    A_UINT32 phy_err[HTT_STATS_PHY_ERR_MAX];
} htt_rx_pdev_fw_stats_phy_err_tlv;

#define HTT_RX_PDEV_FW_RING_MPDU_ERR_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Num error MPDU for each RxDMA error type  */
    A_UINT32 fw_ring_mpdu_err[1]; /* HTT_RX_STATS_RXDMA_MAX_ERR */
} htt_rx_pdev_fw_ring_mpdu_err_tlv_v;

#define HTT_RX_PDEV_FW_MPDU_DROP_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Num MPDU dropped  */
    A_UINT32 fw_mpdu_drop[1]; /* HTT_RX_STATS_FW_DROP_REASON_MAX */
} htt_rx_pdev_fw_mpdu_drop_tlv_v;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_RX
 * TLV_TAGS:
 *      - HTT_STATS_RX_SOC_FW_STATS_TAG (head TLV in soc_stats)
 *      - HTT_STATS_RX_SOC_FW_REFILL_RING_EMPTY_TAG (inside soc_stats)
 *      - HTT_STATS_RX_SOC_FW_REFILL_RING_NUM_REFILL_TAG (inside soc_stats)
 *      - HTT_STATS_RX_PDEV_FW_STATS_TAG
 *      - HTT_STATS_RX_PDEV_FW_RING_MPDU_ERR_TAG
 *      - HTT_STATS_RX_PDEV_FW_MPDU_DROP_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_rx_soc_stats_t                 soc_stats;
    htt_rx_pdev_fw_stats_tlv           fw_stats_tlv;
    htt_rx_pdev_fw_ring_mpdu_err_tlv_v fw_ring_mpdu_err_tlv;
    htt_rx_pdev_fw_mpdu_drop_tlv_v     fw_ring_mpdu_drop;
    htt_rx_pdev_fw_stats_phy_err_tlv   fw_stats_phy_err_tlv;
} htt_rx_pdev_stats_t;

#define HTT_PDEV_CCA_STATS_TX_FRAME_INFO_PRESENT (0x1)
#define HTT_PDEV_CCA_STATS_RX_FRAME_INFO_PRESENT (0x2)
#define HTT_PDEV_CCA_STATS_RX_CLEAR_INFO_PRESENT (0x4)
#define HTT_PDEV_CCA_STATS_MY_RX_FRAME_INFO_PRESENT (0x8)
#define HTT_PDEV_CCA_STATS_USEC_CNT_INFO_PRESENT (0x10)
#define HTT_PDEV_CCA_STATS_MED_RX_IDLE_INFO_PRESENT (0x20)
#define HTT_PDEV_CCA_STATS_MED_TX_IDLE_GLOBAL_INFO_PRESENT (0x40)
#define HTT_PDEV_CCA_STATS_CCA_OBBS_USEC_INFO_PRESENT (0x80)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* Below values are obtained from the HW Cycles counter registers */
    A_UINT32 tx_frame_usec;
    A_UINT32 rx_frame_usec;
    A_UINT32 rx_clear_usec;
    A_UINT32 my_rx_frame_usec;
    A_UINT32 usec_cnt;
    A_UINT32 med_rx_idle_usec;
    A_UINT32 med_tx_idle_global_usec;
    A_UINT32 cca_obss_usec;
} htt_pdev_stats_cca_counters_tlv;

/* NOTE: THIS htt_pdev_cca_stats_hist_tlv STRUCTURE IS DEPRECATED,
 * due to lack of support in some host stats infrastructures for
 * TLVs nested within TLVs.
 */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* The channel number on which these stats were collected */
    A_UINT32 chan_num;

    /* num of CCA records (Num of htt_pdev_stats_cca_counters_tlv)*/
    A_UINT32 num_records;

    /*
     * Bit map of valid CCA counters
     * Bit0 - tx_frame_usec
     * Bit1 - rx_frame_usec
     * Bit2 - rx_clear_usec
     * Bit3 - my_rx_frame_usec
     * bit4 - usec_cnt
     * Bit5 - med_rx_idle_usec
     * Bit6 - med_tx_idle_global_usec
     * Bit7 - cca_obss_usec
     *
     * See HTT_PDEV_CCA_STATS_xxx_INFO_PRESENT defs
     */
    A_UINT32 valid_cca_counters_bitmap;

    /* Indicates the stats collection interval
     *  Valid Values:
     *      100        - For the 100ms interval CCA stats histogram
     *      1000       - For 1sec interval CCA histogram
     *      0xFFFFFFFF - For Cumulative CCA Stats
     */
    A_UINT32 collection_interval;

    /**
     * This will be followed by an array which contains the CCA stats
     * collected in the last N intervals,
     * if the indication is for last N intervals CCA stats.
     * Then the pdev_cca_stats[0] element contains the oldest CCA stats
     * and pdev_cca_stats[N-1] will have the most recent CCA stats.
     */
    htt_pdev_stats_cca_counters_tlv cca_hist_tlv[1];
} htt_pdev_cca_stats_hist_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* The channel number on which these stats were collected */
    A_UINT32 chan_num;

    /* num of CCA records (Num of htt_pdev_stats_cca_counters_tlv)*/
    A_UINT32 num_records;

    /*
     * Bit map of valid CCA counters
     * Bit0 - tx_frame_usec
     * Bit1 - rx_frame_usec
     * Bit2 - rx_clear_usec
     * Bit3 - my_rx_frame_usec
     * bit4 - usec_cnt
     * Bit5 - med_rx_idle_usec
     * Bit6 - med_tx_idle_global_usec
     * Bit7 - cca_obss_usec
     *
     * See HTT_PDEV_CCA_STATS_xxx_INFO_PRESENT defs
     */
    A_UINT32 valid_cca_counters_bitmap;

    /* Indicates the stats collection interval
     *  Valid Values:
     *      100        - For the 100ms interval CCA stats histogram
     *      1000       - For 1sec interval CCA histogram
     *      0xFFFFFFFF - For Cumulative CCA Stats
     */
    A_UINT32 collection_interval;

    /**
     * This will be followed by an array which contains the CCA stats
     * collected in the last N intervals,
     * if the indication is for last N intervals CCA stats.
     * Then the pdev_cca_stats[0] element contains the oldest CCA stats
     * and pdev_cca_stats[N-1] will have the most recent CCA stats.
     * htt_pdev_stats_cca_counters_tlv cca_hist_tlv[1];
     */
} htt_pdev_cca_stats_hist_v1_tlv;

#define HTT_TWT_SESSION_FLAG_FLOW_ID_M 0x0000ffff
#define HTT_TWT_SESSION_FLAG_FLOW_ID_S 0

#define HTT_TWT_SESSION_FLAG_BCAST_TWT_M 0x00010000
#define HTT_TWT_SESSION_FLAG_BCAST_TWT_S 16

#define HTT_TWT_SESSION_FLAG_TRIGGER_TWT_M 0x00020000
#define HTT_TWT_SESSION_FLAG_TRIGGER_TWT_S 17

#define HTT_TWT_SESSION_FLAG_ANNOUN_TWT_M 0x00040000
#define HTT_TWT_SESSION_FLAG_ANNOUN_TWT_S 18

#define HTT_TWT_SESSION_FLAG_FLOW_ID_GET(_var) \
    (((_var) & HTT_TWT_SESSION_FLAG_FLOW_ID_M) >> \
     HTT_TWT_SESSION_FLAG_FLOW_ID_S)

#define HTT_TWT_SESSION_FLAG_FLOW_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TWT_SESSION_FLAG_FLOW_ID, _val); \
        ((_var) |= ((_val) << HTT_TWT_SESSION_FLAG_FLOW_ID_S)); \
    } while (0)

#define HTT_TWT_SESSION_FLAG_BCAST_TWT_GET(_var) \
    (((_var) & HTT_TWT_SESSION_FLAG_BCAST_TWT_M) >> \
     HTT_TWT_SESSION_FLAG_BCAST_TWT_S)

#define HTT_TWT_SESSION_FLAG_BCAST_TWT_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TWT_SESSION_FLAG_BCAST_TWT, _val); \
        ((_var) |= ((_val) << HTT_TWT_SESSION_FLAG_BCAST_TWT_S)); \
    } while (0)

#define HTT_TWT_SESSION_FLAG_TRIGGER_TWT_GET(_var) \
    (((_var) & HTT_TWT_SESSION_FLAG_TRIGGER_TWT_M) >> \
     HTT_TWT_SESSION_FLAG_TRIGGER_TWT_S)

#define HTT_TWT_SESSION_FLAG_TRIGGER_TWT_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TWT_SESSION_FLAG_TRIGGER_TWT, _val); \
        ((_var) |= ((_val) << HTT_TWT_SESSION_FLAG_TRIGGER_TWT_S)); \
    } while (0)

#define HTT_TWT_SESSION_FLAG_ANNOUN_TWT_GET(_var) \
    (((_var) & HTT_TWT_SESSION_FLAG_ANNOUN_TWT_M) >> \
     HTT_TWT_SESSION_FLAG_ANNOUN_TWT_S)

#define HTT_TWT_SESSION_FLAG_ANNOUN_TWT_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TWT_SESSION_FLAG_ANNOUN_TWT, _val); \
        ((_var) |= ((_val) << HTT_TWT_SESSION_FLAG_ANNOUN_TWT_S)); \
    } while (0)

#define TWT_DIALOG_ID_UNAVAILABLE 0xFFFFFFFF

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32     vdev_id;
    htt_mac_addr peer_mac;
    A_UINT32     flow_id_flags;
    A_UINT32     dialog_id; /* TWT_DIALOG_ID_UNAVAILABLE is used when TWT session is not initiated by host */
    A_UINT32     wake_dura_us;
    A_UINT32     wake_intvl_us;
    A_UINT32     sp_offset_us;
} htt_pdev_stats_twt_session_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 pdev_id;
    A_UINT32 num_sessions;

    htt_pdev_stats_twt_session_tlv twt_session[1];
} htt_pdev_stats_twt_sessions_tlv;

/* STATS_TYPE: HTT_DBG_EXT_STATS_TWT_SESSIONS
 * TLV_TAGS:
 *     - HTT_STATS_PDEV_TWT_SESSIONS_TAG
 *     - HTT_STATS_PDEV_TWT_SESSION_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_pdev_stats_twt_sessions_tlv twt_sessions[1];
} htt_pdev_twt_sessions_stats_t;

typedef enum {
    /* Global link descriptor queued in REO */
    HTT_RX_REO_RESOURCE_GLOBAL_LINK_DESC_COUNT_0 = 0,
    HTT_RX_REO_RESOURCE_GLOBAL_LINK_DESC_COUNT_1 = 1,
    HTT_RX_REO_RESOURCE_GLOBAL_LINK_DESC_COUNT_2 = 2,
    /*Number of queue descriptors of this aging group */
    HTT_RX_REO_RESOURCE_BUFFERS_USED_AC0     = 3,
    HTT_RX_REO_RESOURCE_BUFFERS_USED_AC1     = 4,
    HTT_RX_REO_RESOURCE_BUFFERS_USED_AC2     = 5,
    HTT_RX_REO_RESOURCE_BUFFERS_USED_AC3     = 6,
    /* Total number of MSDUs buffered in AC */
    HTT_RX_REO_RESOURCE_AGING_NUM_QUEUES_AC0 = 7,
    HTT_RX_REO_RESOURCE_AGING_NUM_QUEUES_AC1 = 8,
    HTT_RX_REO_RESOURCE_AGING_NUM_QUEUES_AC2 = 9,
    HTT_RX_REO_RESOURCE_AGING_NUM_QUEUES_AC3 = 10,

    HTT_RX_REO_RESOURCE_STATS_MAX = 16
} htt_rx_reo_resource_sample_id_enum;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Variable based on the Number of records. HTT_RX_REO_RESOURCE_STATS_MAX */
    /* htt_rx_reo_debug_sample_id_enum */
    A_UINT32 sample_id;
    /* Max value of all samples */
    A_UINT32 total_max;
    /* Average value of total samples */
    A_UINT32 total_avg;
    /* Num of samples including both zeros and non zeros ones*/
    A_UINT32 total_sample;
    /* Average value of all non zeros samples */
    A_UINT32 non_zeros_avg;
    /* Num of non zeros samples */
    A_UINT32 non_zeros_sample;
    /* Max value of last N non zero samples (N = last_non_zeros_sample) */
    A_UINT32 last_non_zeros_max;
    /* Min value of last N non zero samples (N = last_non_zeros_sample) */
    A_UINT32 last_non_zeros_min;
    /* Average value of last N non zero samples (N = last_non_zeros_sample) */
    A_UINT32 last_non_zeros_avg;
    /* Num of last non zero samples */
    A_UINT32 last_non_zeros_sample;
} htt_rx_reo_resource_stats_tlv_v;

/* STATS_TYPE: HTT_DBG_EXT_STATS_REO_RESOURCE_STATS
 * TLV_TAGS:
 *     - HTT_STATS_RX_REO_RESOURCE_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_rx_reo_resource_stats_tlv_v reo_resource_stats;
} htt_soc_reo_resource_stats_t;

/* == TX SOUNDING STATS == */

/* config_param0 */

#define HTT_DBG_EXT_STATS_SET_VDEV_MASK(_var) ((_var << 1) | 0x1)
#define HTT_DBG_EXT_STATS_GET_VDEV_ID_FROM_VDEV_MASK(_var) ((_var >> 1) & 0xFF)
#define HTT_DBG_EXT_STATS_IS_VDEV_ID_SET(_var) ((_var) & 0x1)

typedef enum {
    /* Implicit beamforming stats */
    HTT_IMPLICIT_TXBF_STEER_STATS = 0,
    /* Single user short inter frame sequence steer stats */
    HTT_EXPLICIT_TXBF_SU_SIFS_STEER_STATS = 1,
    /* Single user random back off steer stats */
    HTT_EXPLICIT_TXBF_SU_RBO_STEER_STATS  = 2,
    /* Multi user short inter frame sequence steer stats */
    HTT_EXPLICIT_TXBF_MU_SIFS_STEER_STATS = 3,
    /* Multi user random back off steer stats */
    HTT_EXPLICIT_TXBF_MU_RBO_STEER_STATS  = 4,
    /* For backward compatability new modes cannot be added */
    HTT_TXBF_MAX_NUM_OF_MODES = 5
} htt_txbf_sound_steer_modes;

typedef enum {
    HTT_TX_AC_SOUNDING_MODE = 0,
    HTT_TX_AX_SOUNDING_MODE = 1,
} htt_stats_sounding_tx_mode;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      tx_sounding_mode; /* HTT_TX_XX_SOUNDING_MODE */
    /* Counts number of soundings for all steering modes in each bw */
    A_UINT32 cbf_20[HTT_TXBF_MAX_NUM_OF_MODES];
    A_UINT32 cbf_40[HTT_TXBF_MAX_NUM_OF_MODES];
    A_UINT32 cbf_80[HTT_TXBF_MAX_NUM_OF_MODES];
    A_UINT32 cbf_160[HTT_TXBF_MAX_NUM_OF_MODES];
    /*
     * The sounding array is a 2-D array stored as an 1-D array of
     * A_UINT32. The stats for a particular user/bw combination is
     * referenced with the following:
     *
     *          sounding[(user* max_bw) + bw]
     *
     * ... where max_bw == 4 for 160mhz
     */
    A_UINT32 sounding[HTT_TX_NUM_OF_SOUNDING_STATS_WORDS];
} htt_tx_sounding_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_TX_SOUNDING_INFO
 * TLV_TAGS:
 *      - HTT_STATS_TX_SOUNDING_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_tx_sounding_stats_tlv sounding_tlv;
} htt_tx_sounding_stats_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 num_obss_tx_ppdu_success;
    A_UINT32 num_obss_tx_ppdu_failure;
} htt_pdev_obss_pd_stats_tlv;

/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_pdev_obss_pd_stats_tlv obss_pd_stat;
} htt_pdev_obss_pd_stats_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 pdev_id;
    A_UINT32 current_head_idx;
    A_UINT32 current_tail_idx;
    A_UINT32 num_htt_msgs_sent;
    /*
     * Time in milliseconds for which the ring has been in
     * its current backpressure condition
     */
    A_UINT32 backpressure_time_ms;
    /* backpressure_hist - histogram showing how many times different degrees
     * of backpressure duration occurred:
     * Index 0 indicates the number of times ring was
     * continously in backpressure state for 100 - 200ms.
     * Index 1 indicates the number of times ring was
     * continously in backpressure state for 200 - 300ms.
     * Index 2 indicates the number of times ring was
     * continously in backpressure state for 300 - 400ms.
     * Index 3 indicates the number of times ring was
     * continously in backpressure state for 400 - 500ms.
     * Index 4 indicates the number of times ring was
     * continously in backpressure state beyond 500ms.
     */
    A_UINT32 backpressure_hist[5];
} htt_ring_backpressure_stats_tlv;

/* STATS_TYPE : HTT_STATS_RING_BACKPRESSURE_STATS_INFO
 * TLV_TAGS:
 *      - HTT_STATS_RING_BACKPRESSURE_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_sring_cmn_tlv cmn_tlv;
    struct {
        htt_stats_string_tlv sring_str_tlv;
        htt_ring_backpressure_stats_tlv backpressure_stats_tlv;
    } r[1]; /* variable-length array */
} htt_ring_backpressure_stats_t;

#define HTT_LATENCY_PROFILE_MAX_HIST        3
#define HTT_STATS_MAX_PROF_STATS_NAME_LEN  32
typedef struct {
    htt_tlv_hdr_t   tlv_hdr;
    /* print_header:
     * This field suggests whether the host should print a header when
     * displaying the TLV (because this is the first latency_prof_stats
     * TLV within a series), or if only the TLV contents should be displayed
     * without a header (because this is not the first TLV within the series).
     */
    A_UINT32 print_header;
    A_UINT8 latency_prof_name[HTT_STATS_MAX_PROF_STATS_NAME_LEN];
    A_UINT32 cnt; /* number of data values included in the tot sum */
    A_UINT32 min; /* time in us */
    A_UINT32 max; /* time in us */
    A_UINT32 last;
    A_UINT32 tot; /* time in us */
    A_UINT32 avg; /* time in us */
    /* hist_intvl:
     * Histogram interval, i.e. the latency range covered by each
     * bin of the histogram, in microsecond units.
     * hist[0] counts how many latencies were between 0 to hist_intvl
     * hist[1] counts how many latencies were between hist_intvl to 2*hist_intvl
     * hist[2] counts how many latencies were more than 2*hist_intvl
     */
    A_UINT32 hist_intvl;
    A_UINT32 hist[HTT_LATENCY_PROFILE_MAX_HIST];
} htt_latency_prof_stats_tlv;

typedef struct {
    htt_tlv_hdr_t   tlv_hdr;
    /* duration:
     * Time period over which counts were gathered, units = microseconds.
     */
    A_UINT32 duration;
    A_UINT32 tx_msdu_cnt;
    A_UINT32 tx_mpdu_cnt;
    A_UINT32 tx_ppdu_cnt;
    A_UINT32 rx_msdu_cnt;
    A_UINT32 rx_mpdu_cnt;
} htt_latency_prof_ctx_tlv;

typedef struct {
    htt_tlv_hdr_t   tlv_hdr;
    A_UINT32 prof_enable_cnt; /* count of enabled profiles */
} htt_latency_prof_cnt_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_LATENCY_PROF_STATS
 * TLV_TAGS:
 *      HTT_STATS_LATENCY_PROF_STATS_TAG / htt_latency_prof_stats_tlv
 *      HTT_STATS_LATENCY_CTX_TAG / htt_latency_prof_ctx_tlv
 *      HTT_STATS_LATENCY_CNT_TAG / htt_latency_prof_cnt_tlv
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_latency_prof_stats_tlv latency_prof_stat;
    htt_latency_prof_ctx_tlv latency_ctx_stat;
    htt_latency_prof_cnt_tlv latency_cnt_stat;
} htt_soc_latency_stats_t;


#endif /* __HTT_STATS_H__ */
