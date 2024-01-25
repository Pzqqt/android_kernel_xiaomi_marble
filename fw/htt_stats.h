/*
 * Copyright (c) 2017-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021-2024 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <htt_deps.h> /* A_UINT32 */
#include <htt_common.h>
#include <htt.h> /* HTT stats TLV struct def and tag defs */


/* HTT_STATS_VAR_LEN_ARRAY1:
 * This macro is for converting the definition of existing variable-length
 * arrays within TLV structs of the form "type name[1];" to use the form
 * "type name[];" while ensuring that the length of the TLV struct is
 * unmodified by the conversion.
 * In general, any new variable-length structs should simply use
 * "type name[];" directly, rather than using HTT_STATS_VAR_LEN_ARRAY1.
 * However, if there's a legitimate reason to make the new variable-length
 * struct appear to not have a variable length, HTT_STATS_VAR_LEN_ARRAY1
 * can be used for this purpose.
 */

#if defined(ATH_TARGET) || defined(__WINDOWS__)
    #define HTT_STATS_VAR_LEN_ARRAY1(type, name) type name[1]
#else
    /*
     * Certain build settings of the Linux kernel don't allow zero-element
     * arrays, and C++ doesn't allow zero-length empty structs.
     * Confirm that there's no build that combines kernel with C++.
     */
    #ifdef __cplusplus
        #error unsupported combination of kernel and C plus plus
    #endif
    #define HTT_STATS_DUMMY_ZERO_LEN_FIELD struct {} dummy_zero_len_field

    #define HTT_STATS_VAR_LEN_ARRAY1(type, name) \
        union { \
            type name ## __first_elem; \
            struct { \
                HTT_STATS_DUMMY_ZERO_LEN_FIELD; \
                type name[]; \
            };  \
        }
#endif


/**
 * htt_dbg_ext_stats_type -
 * The base structure for each of the stats_type is only for reference
 * Host should use this information to know the type of TLVs to expect
 * for a particular stats type.
 *
 *    Max supported stats :- 256.
 */
enum htt_dbg_ext_stats_type {
    /** HTT_DBG_EXT_STATS_RESET
     * PARAM:
     *   - config_param0 : start_offset (stats type)
     *   - config_param1 : stats bmask from start offset
     *   - config_param2 : stats bmask from start offset + 32
     *   - config_param3 : stats bmask from start offset + 64
     * RESP MSG:
     *   - No response sent.
     */
    HTT_DBG_EXT_STATS_RESET              = 0,

    /** HTT_DBG_EXT_STATS_PDEV_TX
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_pdev_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_TX            = 1,

    /** HTT_DBG_EXT_STATS_PDEV_RX
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_rx_pdev_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_RX            = 2,

    /** HTT_DBG_EXT_STATS_PDEV_TX_HWQ
     * PARAMS:
     *   - config_param0: [Bit31: Bit0] HWQ mask
     * RESP MSG:
     *   - htt_tx_hwq_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_TX_HWQ        = 3,

    /** HTT_DBG_EXT_STATS_PDEV_TX_SCHED
     * PARAMS:
     *   - config_param0: [Bit31: Bit0] TXQ mask
     * RESP MSG:
     *   - htt_stats_tx_sched_t
     */
    HTT_DBG_EXT_STATS_PDEV_TX_SCHED      = 4,

    /** HTT_DBG_EXT_STATS_PDEV_ERROR
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_hw_err_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_ERROR         = 5,

    /** HTT_DBG_EXT_STATS_PDEV_TQM
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_tqm_pdev_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_TQM           = 6,

    /** HTT_DBG_EXT_STATS_TQM_CMDQ
     * PARAMS:
     *   - config_param0:
     *      [Bit15: Bit0 ] cmdq id :if 0xFFFF print all cmdq's
     *      [Bit31: Bit16] reserved
     * RESP MSG:
     *   - htt_tx_tqm_cmdq_stats_t
     */
    HTT_DBG_EXT_STATS_TQM_CMDQ           = 7,

    /** HTT_DBG_EXT_STATS_TX_DE_INFO
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_de_stats_t
     */
    HTT_DBG_EXT_STATS_TX_DE_INFO         = 8,

    /** HTT_DBG_EXT_STATS_PDEV_TX_RATE
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_pdev_rate_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_TX_RATE       = 9,

    /** HTT_DBG_EXT_STATS_PDEV_RX_RATE
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_rx_pdev_rate_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_RX_RATE       = 10,

    /** HTT_DBG_EXT_STATS_PEER_INFO
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
     *           7 bit htt_peer_sched_stats_tlv
     *           8 bit htt_peer_ax_ofdma_stats_tlv
     *           9 bit htt_peer_be_ofdma_stats_tlv
     *   - config_param2: [Bit31 : Bit0] mac_addr31to0
     *   - config_param3: [Bit15 : Bit0] mac_addr47to32
     *                    [Bit 16] If this bit is set, reset per peer stats
     *                             of corresponding tlv indicated by config
     *                             param 1.
     *                             HTT_DBG_EXT_PEER_STATS_RESET_GET will be
     *                             used to get this bit position.
     *                             WMI_SERVICE_PER_PEER_HTT_STATS_RESET
     *                             indicates that FW supports per peer HTT
     *                             stats reset.
     *                    [Bit31 : Bit17] reserved
     * RESP MSG:
     *   - htt_peer_stats_t
     */
    HTT_DBG_EXT_STATS_PEER_INFO          = 11,

    /** HTT_DBG_EXT_STATS_TX_SELFGEN_INFO
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_pdev_selfgen_stats_t
     */
    HTT_DBG_EXT_STATS_TX_SELFGEN_INFO    = 12,

    /** HTT_DBG_EXT_STATS_TX_MU_HWQ
     * PARAMS:
     *   - config_param0: [Bit31: Bit0] HWQ mask
     * RESP MSG:
     *   - htt_tx_hwq_mu_mimo_stats_t
     */
    HTT_DBG_EXT_STATS_TX_MU_HWQ          = 13,

    /** HTT_DBG_EXT_STATS_RING_IF_INFO
     * PARAMS:
     *   - config_param0:
     *      [Bit15: Bit0 ] ring id :if 0xFFFF print all rings
     *      [Bit31: Bit16] reserved
     * RESP MSG:
     *   - htt_ring_if_stats_t
     */
    HTT_DBG_EXT_STATS_RING_IF_INFO       = 14,

    /** HTT_DBG_EXT_STATS_SRNG_INFO
     * PARAMS:
     *   - config_param0:
     *      [Bit15: Bit0 ] ring id :if 0xFFFF print all rings
     *      [Bit31: Bit16] reserved
     *   - No Params
     * RESP MSG:
     *   - htt_sring_stats_t
     */
    HTT_DBG_EXT_STATS_SRNG_INFO          = 15,

    /** HTT_DBG_EXT_STATS_SFM_INFO
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_sfm_stats_t
     */
    HTT_DBG_EXT_STATS_SFM_INFO           = 16,

    /** HTT_DBG_EXT_STATS_PDEV_TX_MU
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_pdev_mu_mimo_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_TX_MU         = 17,

    /** HTT_DBG_EXT_STATS_ACTIVE_PEERS_LIST
     * PARAMS:
     *   - config_param0:
     *      [Bit7 : Bit0]   vdev_id:8
     *                      note:0xFF to get all active peers based on pdev_mask.
     *      [Bit31 : Bit8]  rsvd:24
     * RESP MSG:
     *   - htt_active_peer_details_list_t
     */
    HTT_DBG_EXT_STATS_ACTIVE_PEERS_LIST  = 18,

    /** HTT_DBG_EXT_STATS_PDEV_CCA_STATS
     * PARAMS:
     *   - config_param0:
     *      [Bit0] - Clear bit0 to read 1sec,100ms & cumulative CCA stats.
     *               Set bit0 to 1 to read 1sec interval histogram.
     *      [Bit1] - 100ms interval histogram
     *      [Bit3] - Cumulative CCA stats
     * RESP MSG:
     *   - htt_pdev_cca_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_CCA_STATS     = 19,

    /** HTT_DBG_EXT_STATS_TWT_SESSIONS
     * PARAMS:
     *   - config_param0:
     *      No params
     * RESP MSG:
     *   - htt_pdev_twt_sessions_stats_t
     */
    HTT_DBG_EXT_STATS_TWT_SESSIONS       = 20,

    /** HTT_DBG_EXT_STATS_REO_CNTS
     * PARAMS:
     *   - config_param0:
     *      No params
     * RESP MSG:
     *   - htt_soc_reo_resource_stats_t
     */
    HTT_DBG_EXT_STATS_REO_RESOURCE_STATS = 21,

    /** HTT_DBG_EXT_STATS_TX_SOUNDING_INFO
     * PARAMS:
     *   - config_param0:
     *      [Bit0]          vdev_id_set:1
     *          set to 1 if vdev_id is set and vdev stats are requested.
     *          set to 0 if pdev_stats sounding stats are requested.
     *      [Bit8 : Bit1]   vdev_id:8
     *          note:0xFF to get all active vdevs based on pdev_mask.
     *      [Bit31 : Bit9]  rsvd:22
     *
     * RESP MSG:
     *   - htt_tx_sounding_stats_t
     */
    HTT_DBG_EXT_STATS_TX_SOUNDING_INFO   = 22,

    /** HTT_DBG_EXT_STATS_PDEV_OBSS_PD_STATS
     * PARAMS:
     *   - config_param0:
     *      No params
     * RESP MSG:
     *   - htt_pdev_obss_pd_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_OBSS_PD_STATS = 23,

    /** HTT_DBG_EXT_STATS_RING_BACKPRESSURE_STATS
     * PARAMS:
     *   - config_param0:
     *      No params
     * RESP MSG:
     *   - htt_stats_ring_backpressure_stats_t
     */
    HTT_DBG_EXT_STATS_RING_BACKPRESSURE_STATS = 24,

    /** HTT_DBG_EXT_STATS_LATENCY_PROF_STATS
     *  PARAMS:
     *
     *  RESP MSG:
     *    - htt_soc_latency_prof_t
     */
    HTT_DBG_EXT_STATS_LATENCY_PROF_STATS = 25,

    /** HTT_DBG_EXT_STATS_PDEV_UL_TRIGGER
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_rx_pdev_ul_trig_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_UL_TRIG_STATS = 26,

    /** HTT_DBG_EXT_STATS_PDEV_UL_MUMIMO_TRIG_STATS = 27
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_rx_pdev_ul_mumimo_trig_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_UL_MUMIMO_TRIG_STATS = 27,

    /** HTT_DBG_EXT_STATS_FSE_RX
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_rx_fse_stats_t
     */
    HTT_DBG_EXT_STATS_FSE_RX = 28,

    /** HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS
     * PARAMS:
     *   - config_param0: [Bit0] : [1] for mac_addr based request
     *   - config_param1: [Bit31 : Bit0] mac_addr31to0
     *   - config_param2: [Bit15 : Bit0] mac_addr47to32
     * RESP MSG:
     *   - htt_ctrl_path_txrx_stats_t
     */
    HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS = 29,

    /** HTT_DBG_EXT_STATS_PDEV_RX_RATE_EXT
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_rx_pdev_rate_ext_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_RX_RATE_EXT    = 30,

    /** HTT_DBG_EXT_STATS_PDEV_TX_RATE_TXBF
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_pdev_txbf_rate_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_TX_RATE_TXBF   = 31,

    /** HTT_DBG_EXT_STATS_TXBF_OFDMA
     */
    HTT_DBG_EXT_STATS_TXBF_OFDMA          = 32,

    /** HTT_DBG_EXT_STA_11AX_UL_STATS
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_sta_11ax_ul_stats
     */
    HTT_DBG_EXT_STA_11AX_UL_STATS = 33,

    /** HTT_DBG_EXT_VDEV_RTT_RESP_STATS
     * PARAMS:
     *   - config_param0:
     *      [Bit7 : Bit0]   vdev_id:8
     *      [Bit31 : Bit8]  rsvd:24
     * RESP MSG:
     *   -
     */
    HTT_DBG_EXT_VDEV_RTT_RESP_STATS = 34,

    /** HTT_DBG_EXT_PKTLOG_AND_HTT_RING_STATS
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_pktlog_and_htt_ring_stats_t
     */
    HTT_DBG_EXT_PKTLOG_AND_HTT_RING_STATS = 35,

    /** HTT_DBG_EXT_STATS_DLPAGER_STATS
     * PARAMS:
     *
     * RESP MSG:
     *   - htt_dlpager_stats_t
     */
    HTT_DBG_EXT_STATS_DLPAGER_STATS = 36,

    /** HTT_DBG_EXT_PHY_COUNTERS_AND_PHY_STATS
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_phy_counters_and_phy_stats_t
     */
    HTT_DBG_EXT_PHY_COUNTERS_AND_PHY_STATS = 37,

    /** HTT_DBG_EXT_VDEVS_TXRX_STATS
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_vdevs_txrx_stats_t
     */
    HTT_DBG_EXT_VDEVS_TXRX_STATS = 38,

    HTT_DBG_EXT_VDEV_RTT_INITIATOR_STATS = 39,

    /** HTT_DBG_EXT_PDEV_PER_STATS
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_pdev_per_stats_t
     */
    HTT_DBG_EXT_PDEV_PER_STATS = 40,

    HTT_DBG_EXT_AST_ENTRIES = 41,

    /** HTT_DBG_EXT_RX_RING_STATS
     * PARAMS:
     *    - No Params
     * RESP MSG:
     *    - htt_rx_fw_ring_stats_tlv_v
     */
    HTT_DBG_EXT_RX_RING_STATS = 42,

    /** HTT_STRM_GEN_MPDUS_STATS, HTT_STRM_GEN_MPDUS_DETAILS_STATS
     * PARAMS:
     *   - No params
     * RESP MSG: HTT_T2H STREAMING_STATS_IND (not EXT_STATS_CONF)
     *   - HTT_STRM_GEN_MPDUS_STATS:
     *     htt_stats_strm_gen_mpdus_tlv_t
     *   - HTT_STRM_GEN_MPDUS_DETAILS_STATS:
     *     htt_stats_strm_gen_mpdus_details_tlv_t
     */
    HTT_STRM_GEN_MPDUS_STATS = 43,
    HTT_STRM_GEN_MPDUS_DETAILS_STATS = 44,

    /** HTT_DBG_SOC_ERROR_STATS
     * PARAMS:
     *    - No Params
     * RESP MSG:
     *    - htt_dmac_reset_stats_tlv
     */
    HTT_DBG_SOC_ERROR_STATS = 45,

    /** HTT_DBG_PDEV_PUNCTURE_STATS
     * PARAMS:
     *    - param 0: enum from htt_tx_pdev_puncture_stats_upload_t, indicating
     *      the stats to upload
     * RESP MSG:
     *    - one or more htt_pdev_puncture_stats_tlv, depending on param 0
     */
    HTT_DBG_PDEV_PUNCTURE_STATS = 46,

    /** HTT_DBG_EXT_STATS_ML_PEERS_INFO
     * PARAMS:
     *    - param 0:
     *      Bit 0 -> HTT_ML_PEER_DETAILS_TLV always enabled by default
     *      Bit 1 -> HTT_ML_PEER_EXT_DETAILS_TLV will be uploaded when
     *               this bit is set
     *      Bit 2 -> HTT_ML_LINK_INFO_TLV will be uploaded when this bit is set
     *  RESP MSG:
     *    - htt_ml_peer_stats_t
     */
    HTT_DBG_EXT_STATS_ML_PEERS_INFO = 47,

    /** HTT_DBG_ODD_MANDATORY_STATS
     * params:
     *          None
     * Response MSG:
     *          htt_odd_mandatory_pdev_stats_tlv
     */
    HTT_DBG_ODD_MANDATORY_STATS = 48,

    /** HTT_DBG_PDEV_SCHED_ALGO_STATS
     * PARAMS:
     *      - No Params
     * RESP MSG:
     *   - htt_pdev_sched_algo_ofdma_stats_tlv
     */
    HTT_DBG_PDEV_SCHED_ALGO_STATS = 49,

    /** HTT_DBG_ODD_MANDATORY_MUMIMO_STATS
     * params:
     *          None
     * Response MSG:
     *          htt_odd_mandatory_mumimo_pdev_stats_tlv
     */
    HTT_DBG_ODD_MANDATORY_MUMIMO_STATS = 50,
    /** HTT_DBG_ODD_MANDATORY_MUOFDMA_STATS
     * params:
     *          None
     * Response MSG:
     *          htt_odd_mandatory_muofdma_pdev_stats_tlv
     */
    HTT_DBG_ODD_MANDATORY_MUOFDMA_STATS = 51,

    /** HTT_DBG_EXT_PHY_PROF_CAL_STATS
     * params:
     *          None
     * Response MSG:
     *          htt_stats_latency_prof_cal_data_tlv
     */
    HTT_DBG_EXT_PHY_PROF_CAL_STATS = 52,

    /** HTT_DBG_EXT_STATS_PDEV_BW_MGR
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_pdev_bw_mgr_stats_t
     */
    HTT_DBG_EXT_STATS_PDEV_BW_MGR = 53,

    /** HTT_DBG_PDEV_MBSSID_CTRL_FRAME_STATS
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_pdev_mbssid_ctrl_frame_stats
     */
    HTT_DBG_PDEV_MBSSID_CTRL_FRAME_STATS = 54,

    /** HTT_DBG_SOC_SSR_STATS
     * Used for non-MLO UMAC recovery stats.
     * PARAMS:
     *    - No Params
     * RESP MSG:
     *    - htt_umac_ssr_stats_tlv
     */
    HTT_DBG_SOC_SSR_STATS = 55,

    /** HTT_DBG_MLO_UMAC_SSR_STATS
     * Used for MLO UMAC recovery stats.
     * PARAMS:
     *    - No Params
     * RESP MSG:
     *    - htt_mlo_umac_ssr_stats_tlv
     */
    HTT_DBG_MLO_UMAC_SSR_STATS = 56,

    /** HTT_DBG_PDEV_TDMA_STATS
     * PARAMS:
     *    - No Params
     * RESP MSG:
     *    - htt_pdev_tdma_stats_tlv
     */
    HTT_DBG_PDEV_TDMA_STATS = 57,

    /** HTT_DBG_CODEL_STATS
     * PARAMS:
     *    - No Params
     * RESP MSG:
     *    - htt_codel_svc_class_stats_tlv
     *    - htt_codel_msduq_stats_tlv
     */
    HTT_DBG_CODEL_STATS = 58,

    /** HTT_DBG_ODD_PDEV_BE_TX_MU_OFDMA_STATS
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_tx_pdev_mpdu_stats_tlv
     */
    HTT_DBG_ODD_PDEV_BE_TX_MU_OFDMA_STATS = 59,

    /** HTT_DBG_EXT_STATS_PDEV_UL_TRIGGER
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_rx_pdev_be_ul_ofdma_user_stats_tlv
     */
    HTT_DBG_ODD_UL_BE_OFDMA_STATS = 60,

    /** HTT_DBG_ODD_BE_TXBF_OFDMA_STATS
     */
    HTT_DBG_ODD_BE_TXBF_OFDMA_STATS = 61,

    /** HTT_DBG_ODD_STATS_PDEV_BE_UL_MUMIMO_TRIG_STATS
     * PARAMS:
     *   - No Params
     * RESP MSG:
     *   - htt_rx_pdev_be_ul_ofdma_user_stats_tlv
     */
    HTT_DBG_ODD_STATS_PDEV_BE_UL_MUMIMO_TRIG_STATS = 62,

    /** HTT_DBG_MLO_SCHED_STATS
     * PARAMS:
     *    - No Params
     * RESP MSG:
     *    - htt_pdev_mlo_sched_stats_tlv
     */
    HTT_DBG_MLO_SCHED_STATS = 63,

    /** HTT_DBG_PDEV_MLO_IPC_STATS
     * PARAMS:
     *    - No Params
     * RESP MSG:
     *    - htt_pdev_mlo_ipc_stats_tlv
     */
    HTT_DBG_PDEV_MLO_IPC_STATS = 64,

    /** HTT_DBG_EXT_PDEV_RTT_RESP_STATS
     * PARAMS:
     *    - No Params
     * RESP MSG:
     *    -  htt_stats_pdev_rtt_resp_stats_tlv
     *    -  htt_stats_pdev_rtt_hw_stats_tlv
     *    -  htt_stats_pdev_rtt_tbr_selfgen_queued_stats_tlv
     *    -  htt_stats_pdev_rtt_tbr_cmd_result_stats_tlv
     */
    HTT_DBG_EXT_PDEV_RTT_RESP_STATS = 65,

    /** HTT_DBG_EXT_PDEV_RTT_INITIATOR_STATS
     * PARAMS:
     *    - No Params
     * RESP MSG:
     *    -  htt_stats_pdev_rtt_init_stats_tlv
     *    -  htt_stats_pdev_rtt_hw_stats_tlv
     */
    HTT_DBG_EXT_PDEV_RTT_INITIATOR_STATS = 66,


    /* keep this last */
    HTT_DBG_NUM_EXT_STATS = 256,
};

/*
 * Macros to get/set the bit field in config param[3] that indicates to
 * clear corresponding per peer stats specified by config param 1
 */
#define HTT_DBG_EXT_PEER_STATS_RESET_M 0x00010000
#define HTT_DBG_EXT_PEER_STATS_RESET_S 16

#define HTT_DBG_EXT_PEER_STATS_RESET_GET(_var) \
    (((_var) & HTT_DBG_EXT_PEER_STATS_RESET_M) >> \
     HTT_DBG_EXT_PEER_STATS_RESET_S)

#define HTT_DBG_EXT_PEER_STATS_RESET_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_DBG_EXT_PEER_STATS_RESET, _val); \
        ((_var) |= ((_val) << HTT_DBG_EXT_PEER_STATS_RESET_S)); \
    } while (0)

#define HTT_STATS_SUBTYPE_MAX 16

/* htt_mu_stats_upload_t
 * Enumerations for specifying whether to upload all MU stats in response to
 * HTT_DBG_EXT_STATS_PDEV_TX_MU, or if not all, then which subset.
 */
typedef enum {
    /* HTT_UPLOAD_MU_STATS: upload all MU stats:
     * UL MU-MIMO + DL MU-MIMO + UL MU-OFDMA + DL MU-OFDMA
     * (note: included OFDMA stats are limited to 11ax)
     */
    HTT_UPLOAD_MU_STATS,

    /* HTT_UPLOAD_MU_MIMO_STATS: upload UL MU-MIMO + DL MU-MIMO stats */
    HTT_UPLOAD_MU_MIMO_STATS,

    /* HTT_UPLOAD_MU_OFDMA_STATS:
     * upload UL MU-OFDMA + DL MU-OFDMA stats (note: 11ax only stats)
     */
    HTT_UPLOAD_MU_OFDMA_STATS,

    HTT_UPLOAD_DL_MU_MIMO_STATS,
    HTT_UPLOAD_UL_MU_MIMO_STATS,
    /* HTT_UPLOAD_DL_MU_OFDMA_STATS:
     * upload DL MU-OFDMA stats (note: 11ax only stats)
     */
    HTT_UPLOAD_DL_MU_OFDMA_STATS,
    /* HTT_UPLOAD_UL_MU_OFDMA_STATS:
     * upload UL MU-OFDMA stats (note: 11ax only stats)
     */
    HTT_UPLOAD_UL_MU_OFDMA_STATS,

    /*
     * Upload BE UL MU-OFDMA + BE DL MU-OFDMA stats,
     * TLV: htt_tx_pdev_dl_be_mu_ofdma_sch_stats_tlv and
     * htt_tx_pdev_ul_be_mu_ofdma_sch_stats_tlv
     */
    HTT_UPLOAD_BE_MU_OFDMA_STATS,

    /*
     * Upload BE DL MU-OFDMA
     * TLV: htt_tx_pdev_dl_be_mu_ofdma_sch_stats_tlv
     */
    HTT_UPLOAD_BE_DL_MU_OFDMA_STATS,

    /*
     * Upload BE UL MU-OFDMA
     * TLV: htt_tx_pdev_ul_be_mu_ofdma_sch_stats_tlv
     */
    HTT_UPLOAD_BE_UL_MU_OFDMA_STATS,
} htt_mu_stats_upload_t;

/* htt_tx_rate_stats_upload_t
 * Enumerations for specifying which stats to upload in response to
 * HTT_DBG_EXT_STATS_PDEV_TX_RATE.
 */
typedef enum {
    /* 11abgn, 11ac, and 11ax TX stats, and a few 11be SU stats
     *
     * TLV: htt_tx_pdev_rate_stats_tlv
     */
    HTT_TX_RATE_STATS_DEFAULT,

    /*
     * Upload 11be OFDMA TX stats
     *
     * TLV: htt_tx_pdev_rate_stats_be_ofdma_tlv
     */
    HTT_TX_RATE_STATS_UPLOAD_11BE_OFDMA,
} htt_tx_rate_stats_upload_t;

/* htt_rx_ul_trigger_stats_upload_t
 * Enumerations for specifying which stats to upload in response to
 * HTT_DBG_EXT_STATS_PDEV_TX_RATE.
 */
typedef enum {
    /* Upload 11ax UL OFDMA RX Trigger stats
     *
     * TLV: htt_rx_pdev_ul_trigger_stats_tlv
     */
    HTT_RX_UL_TRIGGER_STATS_UPLOAD_11AX_OFDMA,

    /*
     * Upload 11be UL OFDMA RX Trigger stats
     *
     * TLV: htt_rx_pdev_be_ul_trigger_stats_tlv
     */
    HTT_RX_UL_TRIGGER_STATS_UPLOAD_11BE_OFDMA,
} htt_rx_ul_trigger_stats_upload_t;

/*
 * The htt_rx_ul_mumimo_trigger_stats_upload_t enum values are
 * provided by the host as one of the config param elements in
 * the HTT_H2T EXT_STATS_REQ message, for stats type ==
 * HTT_DBG_EXT_STATS_PDEV_UL_MUMIMO_TRIG_STATS.
 */
typedef enum {
    /*
     * Upload 11ax UL MUMIMO RX Trigger stats
     * TLV: htt_rx_pdev_ul_mumimo_trig_stats_tlv
     */
    HTT_RX_UL_MUMIMO_TRIGGER_STATS_UPLOAD_11AX,

    /*
     * Upload 11be UL MUMIMO RX Trigger stats
     * TLV: htt_rx_pdev_ul_mumimo_trig_be_stats_tlv
     */
    HTT_RX_UL_MUMIMO_TRIGGER_STATS_UPLOAD_11BE,
} htt_rx_ul_mumimo_trigger_stats_upload_t;

/* htt_tx_pdev_txbf_ofdma_stats_upload_t
 * Enumerations for specifying which stats to upload in response to
 * HTT_DBG_EXT_STATS_TXBF_OFDMA.
 */
typedef enum {
    /* upload 11ax TXBF OFDMA stats
     *
     * TLV: htt_tx_pdev_ax_txbf_ofdma_stats_t
     */
    HTT_UPLOAD_AX_TXBF_OFDMA_STATS,

    /*
     * Upload 11be TXBF OFDMA stats
     *
     * TLV: htt_tx_pdev_be_txbf_ofdma_stats_t
     */
    HTT_UPLOAD_BE_TXBF_OFDMA_STATS,
} htt_tx_pdev_txbf_ofdma_stats_upload_t;

/* htt_tx_pdev_puncture_stats_upload_t
 * Enumerations for specifying which stats to upload in response to
 * HTT_DBG_PDEV_PUNCTURE_STATS.
 */
typedef enum {
    /* upload puncture stats for all supported modes, both TX and RX */
    HTT_UPLOAD_PUNCTURE_STATS_ALL,

    /* upload puncture stats for all supported TX modes */
    HTT_UPLOAD_PUNCTURE_STATS_TX,

    /* upload puncture stats for all supported RX modes */
    HTT_UPLOAD_PUNCTURE_STATS_RX,
} htt_tx_pdev_puncture_stats_upload_t;

#define HTT_STATS_MAX_STRING_SZ32 4
#define HTT_STATS_MACID_INVALID 0xff
#define HTT_TX_HWQ_MAX_DIFS_LATENCY_BINS 10
#define HTT_TX_HWQ_MAX_CMD_RESULT_STATS 13
#define HTT_TX_HWQ_MAX_CMD_STALL_STATS 5
#define HTT_TX_HWQ_MAX_FES_RESULT_STATS 10
#define HTT_PDEV_STATS_PPDU_DUR_HIST_BINS 16
#define HTT_PDEV_STATS_PPDU_DUR_HIST_INTERVAL_US 250

typedef enum {
    HTT_STATS_TX_PDEV_NO_DATA_UNDERRUN = 0,
    HTT_STATS_TX_PDEV_DATA_UNDERRUN_BETWEEN_MPDU = 1,
    HTT_STATS_TX_PDEV_DATA_UNDERRUN_WITHIN_MPDU = 2,
    HTT_TX_PDEV_MAX_URRN_STATS = 3,
} htt_tx_pdev_underrun_enum;

#define HTT_TX_PDEV_MAX_FLUSH_REASON_STATS 150
#define HTT_TX_PDEV_MAX_SIFS_BURST_STATS 9
#define HTT_TX_PDEV_MAX_SIFS_BURST_HIST_STATS 10
#define HTT_TX_PDEV_MAX_PHY_ERR_STATS 18
/* HTT_TX_PDEV_SCHED_TX_MODE_MAX:
 * DEPRECATED - num sched tx mode max is 8
 */
#define HTT_TX_PDEV_SCHED_TX_MODE_MAX 4
#define HTT_TX_PDEV_NUM_SCHED_ORDER_LOG 20

#define HTT_RX_STATS_REFILL_MAX_RING 4
#define HTT_RX_STATS_RXDMA_MAX_ERR 16
#define HTT_RX_STATS_FW_DROP_REASON_MAX 16

/* Bytes stored in little endian order */
/* Length should be multiple of DWORD */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, data); /* Can be variable length */
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

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /** Num PPDUs queued to HW */
    A_UINT32 hw_queued;
    /** Num PPDUs reaped from HW */
    A_UINT32 hw_reaped;
    /** Num underruns */
    A_UINT32 underrun;
    /** Num HW Paused counter */
    A_UINT32 hw_paused;
    /** Num HW flush counter */
    A_UINT32 hw_flush;
    /** Num HW filtered counter */
    A_UINT32 hw_filt;
    /** Num PPDUs cleaned up in TX abort */
    A_UINT32 tx_abort;
    /** Num MPDUs requeued by SW */
    A_UINT32 mpdu_requed;
    /** excessive retries */
    A_UINT32 tx_xretry;
    /** Last used data hw rate code */
    A_UINT32 data_rc;
    /** frames dropped due to excessive SW retries */
    A_UINT32 mpdu_dropped_xretry;
    /** illegal rate phy errors */
    A_UINT32 illgl_rate_phy_err;
    /** wal pdev continuous xretry */
    A_UINT32 cont_xretry;
    /** wal pdev tx timeout */
    A_UINT32 tx_timeout;
    /** wal pdev resets  */
    A_UINT32 pdev_resets;
    /** PHY/BB underrun */
    A_UINT32 phy_underrun;
    /** MPDU is more than txop limit */
    A_UINT32 txop_ovf;
    /** Number of Sequences posted */
    A_UINT32 seq_posted;
    /** Number of Sequences failed queueing */
    A_UINT32 seq_failed_queueing;
    /** Number of Sequences completed */
    A_UINT32 seq_completed;
    /** Number of Sequences restarted */
    A_UINT32 seq_restarted;
    /** Number of MU Sequences posted */
    A_UINT32 mu_seq_posted;
    /** Number of time HW ring is paused between seq switch within ISR */
    A_UINT32 seq_switch_hw_paused;
    /** Number of times seq continuation in DSR */
    A_UINT32 next_seq_posted_dsr;
    /** Number of times seq continuation in ISR */
    A_UINT32 seq_posted_isr;
    /** Number of seq_ctrl cached. */
    A_UINT32 seq_ctrl_cached;
    /** Number of MPDUs successfully transmitted */
    A_UINT32 mpdu_count_tqm;
    /** Number of MSDUs successfully transmitted */
    A_UINT32 msdu_count_tqm;
    /** Number of MPDUs dropped */
    A_UINT32 mpdu_removed_tqm;
    /** Number of MSDUs dropped */
    A_UINT32 msdu_removed_tqm;
    /** Num MPDUs flushed by SW, HWPAUSED, SW TXABORT (Reset,channel change) */
    A_UINT32 mpdus_sw_flush;
    /** Num MPDUs filtered by HW, all filter condition (TTL expired) */
    A_UINT32 mpdus_hw_filter;
    /**
     * Num MPDUs truncated by PDG
     * (TXOP, TBTT, PPDU_duration based on rate, dyn_bw)
     */
    A_UINT32 mpdus_truncated;
    /** Num MPDUs that was tried but didn't receive ACK or BA */
    A_UINT32 mpdus_ack_failed;
    /** Num MPDUs that was dropped due to expiry (MSDU TTL) */
    A_UINT32 mpdus_expired;
    /** Num MPDUs that was retried within seq_ctrl (MGMT/LEGACY) */
    A_UINT32 mpdus_seq_hw_retry;
    /** Num of TQM acked cmds processed */
    A_UINT32 ack_tlv_proc;
    /** coex_abort_mpdu_cnt valid */
    A_UINT32 coex_abort_mpdu_cnt_valid;
    /** coex_abort_mpdu_cnt from TX FES stats */
    A_UINT32 coex_abort_mpdu_cnt;
    /**
     * Number of total PPDUs
     * (DATA, MGMT, excludes selfgen) tried over the air (OTA)
     */
    A_UINT32 num_total_ppdus_tried_ota;
    /** Number of data PPDUs tried over the air (OTA) */
    A_UINT32 num_data_ppdus_tried_ota;
    /** Num Local control/mgmt frames (MSDUs) queued */
    A_UINT32 local_ctrl_mgmt_enqued;
    /**
     * Num Local control/mgmt frames (MSDUs) done
     * It includes all local ctrl/mgmt completions
     * (acked, no ack, flush, TTL, etc)
     */
    A_UINT32 local_ctrl_mgmt_freed;
    /** Num Local data frames (MSDUs) queued */
    A_UINT32 local_data_enqued;
    /**
     * Num Local data frames (MSDUs) done
     * It includes all local data completions
     * (acked, no ack, flush, TTL, etc)
     */
    A_UINT32 local_data_freed;

    /** Num MPDUs tried by SW */
    A_UINT32 mpdu_tried;
    /** Num of waiting seq posted in ISR completion handler */
    A_UINT32 isr_wait_seq_posted;

    A_UINT32 tx_active_dur_us_low;
    A_UINT32 tx_active_dur_us_high;
    /** Number of MPDUs dropped after max retries */
    A_UINT32 remove_mpdus_max_retries;
    /** Num HTT cookies dispatched */
    A_UINT32 comp_delivered;
    /** successful ppdu transmissions */
    A_UINT32 ppdu_ok;
    /** Scheduler self triggers */
    A_UINT32 self_triggers;
    /** FES duration of last tx data PPDU in us (sch_eval_end - ppdu_start) */
    A_UINT32 tx_time_dur_data;
    /** Num of times sequence terminated due to ppdu duration < burst limit */
    A_UINT32 seq_qdepth_repost_stop;
    /** Num of times MU sequence terminated due to MSDUs reaching threshold */
    A_UINT32 mu_seq_min_msdu_repost_stop;
    /** Num of times SU sequence terminated due to MSDUs reaching threshold */
    A_UINT32 seq_min_msdu_repost_stop;
    /** Num of times sequence terminated due to no TXOP available */
    A_UINT32 seq_txop_repost_stop;
    /** Num of times the next sequence got cancelled */
    A_UINT32 next_seq_cancel;
    /** Num of times fes offset was misaligned */
    A_UINT32 fes_offsets_err_cnt;
    /** Num of times peer denylisted for MU-MIMO transmission */
    A_UINT32 num_mu_peer_blacklisted;
    /** Num of times mu_ofdma seq posted */
    A_UINT32 mu_ofdma_seq_posted;
    /** Num of times UL MU MIMO seq posted */
    A_UINT32 ul_mumimo_seq_posted;
    /** Num of times UL OFDMA seq posted */
    A_UINT32 ul_ofdma_seq_posted;
    /** Num of times Thermal module suspended scheduler */
    A_UINT32 thermal_suspend_cnt;
    /** Num of times DFS module suspended scheduler */
    A_UINT32 dfs_suspend_cnt;
    /** Num of times TX abort module suspended scheduler */
    A_UINT32 tx_abort_suspend_cnt;
    /**
     * This field is a target-specific bit mask of suspended PPDU tx queues.
     * Since the bit mask definition is different for different targets,
     * this field is not meant for general use, but rather for debugging use.
     */
    A_UINT32 tgt_specific_opaque_txq_suspend_info;
    /**
     * Last SCHEDULER suspend reason
     * 1 -> Thermal Module
     * 2 -> DFS Module
     * 3 -> Tx Abort Module
     */
    A_UINT32 last_suspend_reason;
    /** Num of dynamic mimo ps dlmumimo sequences posted */
    A_UINT32 num_dyn_mimo_ps_dlmumimo_sequences;
    /** Num of times su bf sequences are denylisted */
    A_UINT32 num_su_txbf_denylisted;
    /** pdev uptime in microseconds **/
    A_UINT32 pdev_up_time_us_low;
    A_UINT32 pdev_up_time_us_high;
    /** count of ofdma sequences flushed */
    A_UINT32 ofdma_seq_flush;
} htt_stats_tx_pdev_cmn_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_cmn_tlv htt_tx_pdev_stats_cmn_tlv;

#define HTT_TX_PDEV_STATS_URRN_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* HTT_TX_PDEV_MAX_URRN_STATS */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, urrn_stats);
} htt_stats_tx_pdev_underrun_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_underrun_tlv htt_tx_pdev_stats_urrn_tlv_v;

#define HTT_TX_PDEV_STATS_FLUSH_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* HTT_TX_PDEV_MAX_FLUSH_REASON_STATS */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, flush_errs);
} htt_stats_tx_pdev_flush_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_flush_tlv htt_tx_pdev_stats_flush_tlv_v;

#define HTT_TX_PDEV_STATS_MLO_ABORT_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* HTT_TX_PDEV_MAX_MLO_ABORT_REASON_STATS */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, mlo_abort_cnt);
} htt_stats_tx_pdev_mlo_abort_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_mlo_abort_tlv htt_tx_pdev_stats_mlo_abort_tlv_v;

#define HTT_TX_PDEV_STATS_MLO_TXOP_ABORT_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* HTT_TX_PDEV_MAX_MLO_ABORT_REASON_STATS */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, mlo_txop_abort_cnt);
} htt_stats_tx_pdev_mlo_txop_abort_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_mlo_txop_abort_tlv
    htt_tx_pdev_stats_mlo_txop_abort_tlv_v;

#define HTT_TX_PDEV_STATS_SIFS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* HTT_TX_PDEV_MAX_SIFS_BURST_STATS */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, sifs_status);
} htt_stats_tx_pdev_sifs_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_sifs_tlv htt_tx_pdev_stats_sifs_tlv_v;

#define HTT_TX_PDEV_STATS_PHY_ERR_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* HTT_TX_PDEV_MAX_PHY_ERR_STATS */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, phy_errs);
} htt_stats_tx_pdev_phy_err_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_phy_err_tlv htt_tx_pdev_stats_phy_err_tlv_v;

/*
 * Each array in the below struct has 16 elements, to cover the 16 possible
 * values for the CW and AIFS parameters.  Each element within the array
 * stores the counter indicating how many transmissions have occurred with
 * that particular value for the MU EDCA parameter in question.
 */
#define HTT_STATS_MUEDCA_VALUE_MAX 16
typedef struct { /* DEPRECATED */
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 aifs[HTT_NUM_AC_WMM][HTT_STATS_MUEDCA_VALUE_MAX];
    A_UINT32 cw_min[HTT_NUM_AC_WMM][HTT_STATS_MUEDCA_VALUE_MAX];
    A_UINT32 cw_max[HTT_NUM_AC_WMM][HTT_STATS_MUEDCA_VALUE_MAX];
} htt_stats_tx_pdev_muedca_params_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_muedca_params_stats_tlv
    htt_tx_pdev_muedca_params_stats_tlv_v;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 relaxed_mu_edca[HTT_NUM_AC_WMM];
    A_UINT32 mumimo_aggressive_mu_edca[HTT_NUM_AC_WMM];
    A_UINT32 mumimo_relaxed_mu_edca[HTT_NUM_AC_WMM];
    A_UINT32 muofdma_aggressive_mu_edca[HTT_NUM_AC_WMM];
    A_UINT32 muofdma_relaxed_mu_edca[HTT_NUM_AC_WMM];
    A_UINT32 latency_mu_edca[HTT_NUM_AC_WMM];
    A_UINT32 psd_boost_mu_edca[HTT_NUM_AC_WMM];
} htt_stats_tx_pdev_mu_edca_params_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_mu_edca_params_stats_tlv
    htt_tx_pdev_mu_edca_params_stats_tlv_v;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 ul_mumimo_less_aggressive[HTT_NUM_AC_WMM];
    A_UINT32 ul_mumimo_medium_aggressive[HTT_NUM_AC_WMM];
    A_UINT32 ul_mumimo_highly_aggressive[HTT_NUM_AC_WMM];
    A_UINT32 ul_mumimo_default_relaxed[HTT_NUM_AC_WMM];
    A_UINT32 ul_muofdma_less_aggressive[HTT_NUM_AC_WMM];
    A_UINT32 ul_muofdma_medium_aggressive[HTT_NUM_AC_WMM];
    A_UINT32 ul_muofdma_highly_aggressive[HTT_NUM_AC_WMM];
    A_UINT32 ul_muofdma_default_relaxed[HTT_NUM_AC_WMM];
} htt_stats_tx_pdev_ap_edca_params_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_ap_edca_params_stats_tlv
    htt_tx_pdev_ap_edca_params_stats_tlv_v;

#define HTT_TX_PDEV_SIFS_BURST_HIST_STATS 10
#define HTT_TX_PDEV_STATS_SIFS_HIST_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* HTT_TX_PDEV_SIFS_BURST_HIST_STATS */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, sifs_hist_status);
} htt_stats_tx_pdev_sifs_hist_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_sifs_hist_tlv htt_tx_pdev_stats_sifs_hist_tlv_v;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      num_data_ppdus_legacy_su;
    A_UINT32      num_data_ppdus_ac_su;
    A_UINT32      num_data_ppdus_ax_su;
    A_UINT32      num_data_ppdus_ac_su_txbf;
    A_UINT32      num_data_ppdus_ax_su_txbf;
} htt_stats_tx_pdev_tx_ppdu_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_tx_ppdu_stats_tlv
    htt_tx_pdev_stats_tx_ppdu_stats_tlv_v;

typedef enum {
    HTT_TX_WAL_ISR_SCHED_SUCCESS,
    HTT_TX_WAL_ISR_SCHED_FILTER,
    HTT_TX_WAL_ISR_SCHED_RESP_TIMEOUT,
    HTT_TX_WAL_ISR_SCHED_RATES_EXHAUSTED,
    HTT_TX_WAL_ISR_SCHED_DATA_EXHAUSTED,
    HTT_TX_WAL_ISR_SCHED_SEQ_ABORT,
    HTT_TX_WAL_ISR_SCHED_NOTIFY_FRAME_ENCOUNTERED,
    HTT_TX_WAL_ISR_SCHED_COMPLETION,
    HTT_TX_WAL_ISR_SCHED_IN_PROGRESS,
} htt_tx_wal_tx_isr_sched_status;

/* [0]- nr4 , [1]- nr8 */
#define HTT_STATS_NUM_NR_BINS 2
/* Termination status stated in htt_tx_wal_tx_isr_sched_status */
#define HTT_STATS_MAX_NUM_SCHED_STATUS  9
#define HTT_STATS_MAX_NUM_MU_PPDU_PER_BURST 10
#define HTT_STATS_MAX_NUM_SCHED_STATUS_WORDS \
    (HTT_STATS_NUM_NR_BINS * HTT_STATS_MAX_NUM_SCHED_STATUS)
#define HTT_STATS_MAX_NUM_MU_PPDU_PER_BURST_WORDS \
    (HTT_STATS_NUM_NR_BINS * HTT_STATS_MAX_NUM_MU_PPDU_PER_BURST)

typedef enum {
    HTT_STATS_HWMODE_AC = 0,
    HTT_STATS_HWMODE_AX = 1,
    HTT_STATS_HWMODE_BE = 2,
} htt_stats_hw_mode;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 hw_mode; /* HTT_STATS_HWMODE_xx */

    A_UINT32 mu_mimo_num_seq_term_status[HTT_STATS_MAX_NUM_SCHED_STATUS_WORDS];

    A_UINT32 mu_mimo_num_ppdu_completed_per_burst[HTT_STATS_MAX_NUM_MU_PPDU_PER_BURST_WORDS];

    A_UINT32 mu_mimo_num_seq_posted[HTT_STATS_NUM_NR_BINS];

    A_UINT32 mu_mimo_num_ppdu_posted_per_burst[HTT_STATS_MAX_NUM_MU_PPDU_PER_BURST_WORDS];
} htt_stats_mu_ppdu_dist_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_mu_ppdu_dist_tlv htt_pdev_mu_ppdu_dist_tlv_v;

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

    /* HTT_TX_PDEV_TRIED_MPDU_CNT_HIST */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, tried_mpdu_cnt_hist);
} htt_stats_tx_pdev_tried_mpdu_cnt_hist_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_tried_mpdu_cnt_hist_tlv
    htt_tx_pdev_stats_tried_mpdu_cnt_hist_tlv_v;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Num MGMT MPDU transmitted by the target */
    A_UINT32 fw_tx_mgmt_subtype[HTT_STATS_SUBTYPE_MAX];
} htt_stats_pdev_ctrl_path_tx_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_ctrl_path_tx_stats_tlv htt_pdev_ctrl_path_tx_stats_tlv_v;

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
 *      - HTT_STATS_PDEV_CTRL_PATH_TX_STATS_TAG
 *      - HTT_STATS_MU_PPDU_DIST_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct _htt_tx_pdev_stats {
    htt_stats_tx_pdev_cmn_tlv                   cmn_tlv;
    htt_stats_tx_pdev_underrun_tlv              underrun_tlv;
    htt_stats_tx_pdev_sifs_tlv                  sifs_tlv;
    htt_stats_tx_pdev_flush_tlv                 flush_tlv;
    htt_stats_tx_pdev_phy_err_tlv               phy_err_tlv;
    htt_stats_tx_pdev_sifs_hist_tlv             sifs_hist_tlv;
    htt_stats_tx_pdev_tx_ppdu_stats_tlv         tx_su_tlv;
    htt_stats_tx_pdev_tried_mpdu_cnt_hist_tlv   tried_mpdu_cnt_hist_tlv;
    htt_stats_pdev_ctrl_path_tx_stats_tlv       ctrl_path_tx_tlv;
    htt_stats_mu_ppdu_dist_tlv                  mu_ppdu_dist_tlv;
} htt_tx_pdev_stats_t;
#endif /* ATH_TARGET */

/* == SOC ERROR STATS == */

/* =============== PDEV ERROR STATS ============== */
#define HTT_STATS_MAX_HW_INTR_NAME_LEN 8
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Stored as little endian */
    A_UINT8  hw_intr_name[HTT_STATS_MAX_HW_INTR_NAME_LEN];
    A_UINT32 mask;
    A_UINT32 count;
} htt_stats_hw_intr_misc_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_hw_intr_misc_tlv htt_hw_stats_intr_misc_tlv;

#define HTT_STATS_MAX_HW_MODULE_NAME_LEN 8
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Stored as little endian */
    A_UINT8  hw_module_name[HTT_STATS_MAX_HW_MODULE_NAME_LEN];
    A_UINT32 count;
} htt_stats_hw_wd_timeout_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_hw_wd_timeout_tlv htt_hw_stats_wd_timeout_tlv;

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
    A_UINT32 phy_warm_reset_reason_tx_lifetime_expiry_cca_stuck;
    A_UINT32 phy_warm_reset_reason_tx_consecutive_flush9_war;
    A_UINT32 phy_warm_reset_reason_tx_hwsch_reset_war;
    A_UINT32 phy_warm_reset_reason_hwsch_wdog_or_cca_wdog_war;
    A_UINT32 fw_rx_rings_reset;
    /**
     * Num of iterations rx leak prevention successfully done.
     */
    A_UINT32 rx_dest_drain_rx_descs_leak_prevention_done;
    /**
     * Num of rx descs successfully saved by rx leak prevention.
     */
    A_UINT32 rx_dest_drain_rx_descs_saved_cnt;
    /*
     * Stats to debug reason Rx leak prevention
     * was not required to be kicked in.
     */
    A_UINT32 rx_dest_drain_rxdma2reo_leak_detected;
    A_UINT32 rx_dest_drain_rxdma2fw_leak_detected;
    A_UINT32 rx_dest_drain_rxdma2wbm_leak_detected;
    A_UINT32 rx_dest_drain_rxdma1_2sw_leak_detected;
    A_UINT32 rx_dest_drain_rx_drain_ok_mac_idle;
    A_UINT32 rx_dest_drain_ok_mac_not_idle;
    A_UINT32 rx_dest_drain_prerequisite_invld;
    A_UINT32 rx_dest_drain_skip_for_non_lmac_reset;
    A_UINT32 rx_dest_drain_hw_fifo_not_empty_post_drain_wait;
} htt_stats_hw_pdev_errs_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_hw_pdev_errs_tlv htt_hw_stats_pdev_errs_tlv;

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
} htt_stats_whal_tx_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_whal_tx_tlv htt_hw_stats_whal_tx_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 wsib_event_watchdog_timeout;
    A_UINT32 wsib_event_slave_tlv_length_error;
    A_UINT32 wsib_event_slave_parity_error;
    A_UINT32 wsib_event_slave_direct_message;
    A_UINT32 wsib_event_slave_backpressure_error;
    A_UINT32 wsib_event_master_tlv_length_error;
} htt_stats_whal_wsi_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    union {
        struct {
            A_UINT32 mac_id:    8,
                     reserved: 24;
        };
        A_UINT32 mac_id__word;
    };

    /**
     * hw_wars is a variable-length array, with each element counting
     * the number of occurrences of the corresponding type of HW WAR.
     * That is, hw_wars[0] indicates how many times HW WAR 0 occurred,
     * hw_wars[1] indicates how many times HW WAR 1 occurred, etc.
     * The target has an internal HW WAR mapping that it uses to keep
     * track of which HW WAR is WAR 0, which HW WAR is WAR 1, etc.
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, hw_wars);
} htt_stats_hw_war_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_hw_war_tlv htt_hw_war_stats_tlv;
/* provide properly-named macro */
#define HTT_STATS_HW_WAR_MAC_ID_GET(word) (word & 0xff)

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
#ifdef ATH_TARGET
typedef struct _htt_pdev_err_stats {
    htt_stats_hw_pdev_errs_tlv  pdev_errs;
    htt_stats_hw_intr_misc_tlv  misc_stats[1];
    htt_stats_hw_wd_timeout_tlv wd_timeout[1];
    htt_stats_whal_tx_tlv       whal_tx_stats;
    htt_stats_hw_war_tlv        hw_war;
} htt_hw_err_stats_t;
#endif /* ATH_TARGET */

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
    /** This will help to find which peer_id is stuck state */
    A_UINT32 sw_peer_id;
    /**
     * BIT [15 :  0]   :- tx_flow_number
     * BIT [19 : 16]   :- tid_num
     * BIT [20 : 20]   :- drop_rule
     * BIT [31 : 21]   :- reserved
     */
    A_UINT32 tx_flow_no__tid_num__drop_rule;
    A_UINT32 last_cycle_enqueue_count;
    A_UINT32 last_cycle_dequeue_count;
    A_UINT32 last_cycle_drop_count;
    /**
     * BIT [15 :  0]   :- current_drop_th
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 current_drop_th;
} htt_stats_peer_msdu_flowq_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_peer_msdu_flowq_tlv htt_msdu_flow_stats_tlv;

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

    /** Stored as little endian */
    A_UINT8 tid_name[MAX_HTT_TID_NAME];
    /**
     * BIT [15 :  0]   :- sw_peer_id
     * BIT [31 : 16]   :- tid_num
     */
    A_UINT32 sw_peer_id__tid_num;
    /**
     * BIT [ 7 :  0]   :- num_sched_pending
     * BIT [15 :  8]   :- num_ppdu_in_hwq
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 num_sched_pending__num_ppdu_in_hwq;
    A_UINT32 tid_flags;
    /** per tid # of hw_queued ppdu */
    A_UINT32 hw_queued;
    /** number of per tid successful PPDU */
    A_UINT32 hw_reaped;
    /** per tid Num MPDUs filtered by HW */
    A_UINT32 mpdus_hw_filter;

    A_UINT32 qdepth_bytes;
    A_UINT32 qdepth_num_msdu;
    A_UINT32 qdepth_num_mpdu;
    A_UINT32 last_scheduled_tsmp;
    A_UINT32 pause_module_id;
    A_UINT32 block_module_id;
    /** tid tx airtime in sec */
    A_UINT32 tid_tx_airtime;
} htt_stats_tx_tid_details_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_tid_details_tlv htt_tx_tid_stats_tlv;

/* Tidq stats */
typedef struct _htt_tx_tid_stats_v1_tlv {
    htt_tlv_hdr_t tlv_hdr;
    /** Stored as little endian */
    A_UINT8 tid_name[MAX_HTT_TID_NAME];
    /**
     * BIT [15 :  0]   :- sw_peer_id
     * BIT [31 : 16]   :- tid_num
     */
    A_UINT32 sw_peer_id__tid_num;
    /**
     * BIT [ 7 :  0]   :- num_sched_pending
     * BIT [15 :  8]   :- num_ppdu_in_hwq
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 num_sched_pending__num_ppdu_in_hwq;
    A_UINT32 tid_flags;
    /** Max qdepth in bytes reached by this tid */
    A_UINT32 max_qdepth_bytes;
    /** number of msdus qdepth reached max */
    A_UINT32 max_qdepth_n_msdus;
    A_UINT32 rsvd;

    A_UINT32 qdepth_bytes;
    A_UINT32 qdepth_num_msdu;
    A_UINT32 qdepth_num_mpdu;
    A_UINT32 last_scheduled_tsmp;
    A_UINT32 pause_module_id;
    A_UINT32 block_module_id;
    /** tid tx airtime in sec */
    A_UINT32 tid_tx_airtime;
    A_UINT32 allow_n_flags;
    /**
     * BIT [15 :  0]   :- sendn_frms_allowed
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 sendn_frms_allowed;
    /*
     * tid_ext_flags, tid_ext2_flags, and tid_flush_reason are opaque fields
     * that cannot be interpreted by the host.
     * They are only for off-line debug.
     */
    A_UINT32 tid_ext_flags;
    A_UINT32 tid_ext2_flags;
    A_UINT32 tid_flush_reason;
    A_UINT32 mlo_flush_tqm_status_pending_low;
    A_UINT32 mlo_flush_tqm_status_pending_high;
    A_UINT32 mlo_flush_partner_info_low;
    A_UINT32 mlo_flush_partner_info_high;
    A_UINT32 mlo_flush_initator_info_low;
    A_UINT32 mlo_flush_initator_info_high;
    /*
     * head_msdu_tqm_timestamp_us:
     *     MSDU enqueue timestamp (TQM reference timestamp) for the MSDU
     *     at the head of the MPDU queue
     * head_msdu_tqm_latency_us:
     *     The age of the MSDU that is at the head of the MPDU queue,
     *     i.e. the delta between the current TQM time and the MSDU's
     *     enqueue timestamp.
     */
    A_UINT32 head_msdu_tqm_timestamp_us;
    A_UINT32 head_msdu_tqm_latency_us;
} htt_stats_tx_tid_details_v1_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_tid_details_v1_tlv htt_tx_tid_stats_v1_tlv;

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

    /**
     * BIT [15 : 0] : sw_peer_id
     * BIT [31 : 16] : tid_num
     */
    A_UINT32 sw_peer_id__tid_num;
    /** Stored as little endian */
    A_UINT8 tid_name[MAX_HTT_TID_NAME];
    /**
     * dup_in_reorder not collected per tid for now,
     * as there is no wal_peer back ptr in data rx peer.
     */
    A_UINT32 dup_in_reorder;
    A_UINT32 dup_past_outside_window;
    A_UINT32 dup_past_within_window;
    /** Number of per tid MSDUs with flag of decrypt_err */
    A_UINT32 rxdesc_err_decrypt;
    /** tid rx airtime in sec */
    A_UINT32 tid_rx_airtime;
} htt_stats_rx_tid_details_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_tid_details_tlv htt_rx_tid_stats_tlv;

#define HTT_MAX_COUNTER_NAME 8
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** Stored as little endian */
    A_UINT8  counter_name[HTT_MAX_COUNTER_NAME];
    A_UINT32 count;
} htt_stats_counter_name_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_counter_name_tlv htt_counter_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** Number of rx PPDU */
    A_UINT32 ppdu_cnt;
    /** Number of rx MPDU */
    A_UINT32 mpdu_cnt;
    /** Number of rx MSDU */
    A_UINT32 msdu_cnt;
    /** pause bitmap */
    A_UINT32 pause_bitmap;
    /** block bitmap */
    A_UINT32 block_bitmap;
    /** current timestamp */
    A_UINT32 current_timestamp;
    /** Peer cumulative tx airtime in sec */
    A_UINT32 peer_tx_airtime;
    /** Peer cumulative rx airtime in sec */
    A_UINT32 peer_rx_airtime;
    /** Peer current rssi in dBm */
    A_INT32 rssi;
    /** Total enqueued, dequeued and dropped MSDU's for peer */
    A_UINT32 peer_enqueued_count_low;
    A_UINT32 peer_enqueued_count_high;
    A_UINT32 peer_dequeued_count_low;
    A_UINT32 peer_dequeued_count_high;
    A_UINT32 peer_dropped_count_low;
    A_UINT32 peer_dropped_count_high;
    /** Total ppdu transmitted bytes for peer: includes MAC header overhead */
    A_UINT32 ppdu_transmitted_bytes_low;
    A_UINT32 ppdu_transmitted_bytes_high;
    A_UINT32 peer_ttl_removed_count;
    /**
     * inactive_time
     * Running duration of the time since last tx/rx activity by this peer,
     * units = seconds.
     * If the peer is currently active, this inactive_time will be 0x0.
     */
    A_UINT32 inactive_time;
    /** Number of MPDUs dropped after max retries */
    A_UINT32 remove_mpdus_max_retries;
} htt_stats_peer_stats_cmn_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_peer_stats_cmn_tlv htt_peer_stats_cmn_tlv;

#define HTT_PEER_DETAILS_ML_PEER_OFFSET_BYTES 32
#define HTT_PEER_DETAILS_ML_PEER_OFFSET_DWORD 8
#define HTT_PEER_DETAILS_ML_PEER_ID_VALID_M   0x00000001
#define HTT_PEER_DETAILS_ML_PEER_ID_VALID_S   0
#define HTT_PEER_DETAILS_ML_PEER_ID_M         0x00001ffe
#define HTT_PEER_DETAILS_ML_PEER_ID_S         1
#define HTT_PEER_DETAILS_LINK_IDX_M           0x001fe000
#define HTT_PEER_DETAILS_LINK_IDX_S           13
#define HTT_PEER_DETAILS_USE_PPE_M            0x00200000
#define HTT_PEER_DETAILS_USE_PPE_S            21


#define HTT_PEER_DETAILS_SRC_INFO_M           0x00000fff
#define HTT_PEER_DETAILS_SRC_INFO_S           0


#define HTT_PEER_DETAILS_SET(word, httsym, val)  \
    do {                                         \
        HTT_CHECK_SET_VAL(HTT_PEER_DETAILS_ ## httsym, val);          \
            (word) |= ((val) << HTT_PEER_DETAILS_ ## httsym ## _S);         \
    } while(0)

#define HTT_PEER_DETAILS_GET(word, httsym) \
    (((word) & HTT_PEER_DETAILS_ ## httsym ## _M) >> HTT_PEER_DETAILS_ ## httsym ## _S)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** This enum type of HTT_PEER_TYPE */
    A_UINT32 peer_type;
    A_UINT32 sw_peer_id;
    /**
     * BIT [7 : 0]   :- vdev_id
     * BIT [15 : 8]  :- pdev_id
     * BIT [31 : 16] :- ast_indx
     */
    A_UINT32     vdev_pdev_ast_idx;
    htt_mac_addr mac_addr;
    A_UINT32     peer_flags;
    A_UINT32     qpeer_flags;

    /* Dword 8 */
    union {
        A_UINT32 word__ml_peer_id_valid__ml_peer_id__link_idx__use_ppe;
        struct {
            A_UINT32     ml_peer_id_valid  : 1,   /* [0:0] */
                         ml_peer_id        : 12,  /* [12:1] */
                         link_idx          : 8,   /* [20:13] */
                         use_ppe           : 1,   /* [21:21] */
                         rsvd0             : 10;  /* [31:22] */
        };
    };

    /* Dword 9 */
    union {
        A_UINT32 word__src_info;
        struct {
            A_UINT32     src_info          : 12,  /* [11:0] */
                         rsvd1             : 20;  /* [31:12] */
        };
    };
} htt_stats_peer_details_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_peer_details_tlv htt_peer_details_tlv;

#define HTT_STATS_PEER_DETAILS_ML_PEER_ID_VALID_GET(word) ((word >> 0) & 0x1)
#define HTT_STATS_PEER_DETAILS_ML_PEER_ID_GET(word)       ((word >> 1) & 0xfff)
#define HTT_STATS_PEER_DETAILS_LINK_IDX_GET(word)         ((word >> 13) & 0xff)
#define HTT_STATS_PEER_DETAILS_USE_PPE_GET(word)          ((word >> 21) & 0x1)

#define HTT_STATS_PEER_DETAILS_SRC_INFO_GET(word) ((word >> 0) & 0xfff)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32     sw_peer_id;
    A_UINT32     ast_index;
    htt_mac_addr mac_addr;
    A_UINT32
        pdev_id        : 2,
        vdev_id        : 8,
        next_hop       : 1,
        mcast          : 1,
        monitor_direct : 1,
        mesh_sta       : 1,
        mec            : 1,
        intra_bss      : 1,
        chip_id        : 2,
        ml_peer_id     : 13,
        on_chip        : 1;
    A_UINT32
        tx_monitor_override_sta : 1,
        rx_monitor_override_sta : 1,
        reserved1               : 30;
} htt_stats_ast_entry_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_ast_entry_tlv htt_ast_entry_tlv;

typedef enum {
    HTT_STATS_DIRECTION_TX,
    HTT_STATS_DIRECTION_RX,
} HTT_STATS_DIRECTION;

typedef enum {
    HTT_STATS_PPDU_TYPE_MODE_SU,
    HTT_STATS_PPDU_TYPE_DL_MU_MIMO,
    HTT_STATS_PPDU_TYPE_UL_MU_MIMO,
    HTT_STATS_PPDU_TYPE_DL_MU_OFDMA,
    HTT_STATS_PPDU_TYPE_UL_MU_OFDMA,
} HTT_STATS_PPDU_TYPE;

typedef enum {
    HTT_STATS_PREAM_OFDM,
    HTT_STATS_PREAM_CCK,
    HTT_STATS_PREAM_HT,
    HTT_STATS_PREAM_VHT,
    HTT_STATS_PREAM_HE,
    HTT_STATS_PREAM_EHT,
    HTT_STATS_PREAM_RSVD1,

    HTT_STATS_PREAM_COUNT,
} HTT_STATS_PREAM_TYPE;

#define HTT_TX_PEER_STATS_NUM_MCS_COUNTERS 12 /* 0-11 */
#define HTT_TX_PEER_STATS_NUM_EXTRA_MCS_COUNTERS 2 /* 12, 13 */
/* HTT_TX_PEER_STATS_NUM_GI_COUNTERS:
 * GI Index 0:  WHAL_GI_800
 * GI Index 1:  WHAL_GI_400
 * GI Index 2:  WHAL_GI_1600
 * GI Index 3:  WHAL_GI_3200
 */
#define HTT_TX_PEER_STATS_NUM_GI_COUNTERS 4
#define HTT_TX_PEER_STATS_NUM_DCM_COUNTERS 5
 /* HTT_TX_PEER_STATS_NUM_BW_COUNTERS:
  * bw index 0: rssi_pri20_chain0
  * bw index 1: rssi_ext20_chain0
  * bw index 2: rssi_ext40_low20_chain0
  * bw index 3: rssi_ext40_high20_chain0
  */
#define HTT_TX_PEER_STATS_NUM_BW_COUNTERS 4
/* HTT_RX_PEER_STATS_NUM_BW_EXT_COUNTERS:
 * bw index 4 (bw ext index 0): rssi_ext80_low20_chain0
 * bw index 5 (bw ext index 1): rssi_ext80_low_high20_chain0
 * bw index 6 (bw ext index 2): rssi_ext80_high_low20_chain0
 * bw index 7 (bw ext index 3): rssi_ext80_high20_chain0
 */
#define HTT_RX_PEER_STATS_NUM_BW_EXT_COUNTERS 4
#define HTT_RX_PDEV_STATS_NUM_BW_EXT_COUNTERS 4

/* HTT_RX  STATS_NUM_BW_EXT_2_COUNTERS:
 * bw index 8  (bw ext_2 index 0): rssi_ext160_0_chainX
 * bw index 9  (bw ext_2 index 1): rssi_ext160_1_chainX
 * bw index 10 (bw ext_2 index 2): rssi_ext160_2_chainX
 * bw index 11 (bw ext_2 index 3): rssi_ext160_3_chainX
 * bw index 12 (bw ext_2 index 4): rssi_ext160_4_chainX
 * bw index 13 (bw ext_2 index 5): rssi_ext160_5_chainX
 * bw index 14 (bw ext_2 index 6): rssi_ext160_6_chainX
 * bw index 15 (bw ext_2 index 7): rssi_ext160_7_chainX
 */
#define HTT_RX_PDEV_STATS_NUM_BW_EXT_2_COUNTERS 8

#define HTT_TX_PEER_STATS_NUM_SPATIAL_STREAMS 8
#define HTT_TX_PEER_STATS_NUM_PREAMBLE_TYPES HTT_STATS_PREAM_COUNT
#define HTT_TX_PEER_STATS_NUM_REDUCED_CHAN_TYPES 2 /* 0 - Half, 1 - Quarter */

typedef struct _htt_tx_peer_rate_stats_tlv {
    htt_tlv_hdr_t tlv_hdr;

    /** Number of tx LDPC packets */
    A_UINT32 tx_ldpc;
    /** Number of tx RTS packets */
    A_UINT32 rts_cnt;
    /** RSSI value of last ack packet (units = dB above noise floor) */
    A_UINT32 ack_rssi;

    A_UINT32 tx_mcs[HTT_TX_PEER_STATS_NUM_MCS_COUNTERS];
    A_UINT32 tx_su_mcs[HTT_TX_PEER_STATS_NUM_MCS_COUNTERS];
    A_UINT32 tx_mu_mcs[HTT_TX_PEER_STATS_NUM_MCS_COUNTERS];
    /**
     * element 0,1, ...7 -> NSS 1,2, ...8
     */
    A_UINT32 tx_nss[HTT_TX_PEER_STATS_NUM_SPATIAL_STREAMS];
    /**
     * element 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160 and 80+80 MHz
     */
    A_UINT32 tx_bw[HTT_TX_PEER_STATS_NUM_BW_COUNTERS];
    A_UINT32 tx_stbc[HTT_TX_PEER_STATS_NUM_MCS_COUNTERS];
    A_UINT32 tx_pream[HTT_TX_PEER_STATS_NUM_PREAMBLE_TYPES];

    /**
     * Counters to track number of tx packets in each GI
     * (400us, 800us, 1600us & 3200us) in each mcs (0-11)
     */
    A_UINT32 tx_gi[HTT_TX_PEER_STATS_NUM_GI_COUNTERS][HTT_TX_PEER_STATS_NUM_MCS_COUNTERS];

    /** Counters to track packets in dcm mcs (MCS 0, 1, 3, 4) */
    A_UINT32 tx_dcm[HTT_TX_PEER_STATS_NUM_DCM_COUNTERS];

    /** Stats for MCS 12/13 */
    A_UINT32 tx_mcs_ext[HTT_TX_PEER_STATS_NUM_EXTRA_MCS_COUNTERS];
    A_UINT32 tx_su_mcs_ext[HTT_TX_PEER_STATS_NUM_EXTRA_MCS_COUNTERS];
    A_UINT32 tx_mu_mcs_ext[HTT_TX_PEER_STATS_NUM_EXTRA_MCS_COUNTERS];
    A_UINT32 tx_stbc_ext[HTT_TX_PEER_STATS_NUM_EXTRA_MCS_COUNTERS];
    A_UINT32 tx_gi_ext[HTT_TX_PEER_STATS_NUM_GI_COUNTERS][HTT_TX_PEER_STATS_NUM_EXTRA_MCS_COUNTERS];
    A_UINT32 reduced_tx_bw[HTT_TX_PEER_STATS_NUM_REDUCED_CHAN_TYPES][HTT_TX_PEER_STATS_NUM_BW_COUNTERS];
    A_UINT32 tx_bw_320mhz;
} htt_stats_peer_tx_rate_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_peer_tx_rate_stats_tlv htt_tx_peer_rate_stats_tlv;

#define HTT_RX_PEER_STATS_NUM_MCS_COUNTERS 12 /* 0-11 */
#define HTT_RX_PEER_STATS_NUM_EXTRA_MCS_COUNTERS 2 /* 12, 13 */
#define HTT_RX_PEER_STATS_NUM_GI_COUNTERS 4
#define HTT_RX_PEER_STATS_NUM_DCM_COUNTERS 5
#define HTT_RX_PEER_STATS_NUM_BW_COUNTERS 4
#define HTT_RX_PEER_STATS_NUM_SPATIAL_STREAMS 8
#define HTT_RX_PEER_STATS_NUM_PREAMBLE_TYPES HTT_STATS_PREAM_COUNT
#define HTT_RX_PEER_STATS_NUM_REDUCED_CHAN_TYPES 2 /* 0 - Half, 1 - Quarter */

typedef struct _htt_rx_peer_rate_stats_tlv {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      nsts;

    /** Number of rx LDPC packets */
    A_UINT32 rx_ldpc;
    /** Number of rx RTS packets */
    A_UINT32 rts_cnt;

    /** units = dB above noise floor */
    A_UINT32 rssi_mgmt;
    /** units = dB above noise floor */
    A_UINT32 rssi_data;
    /** units = dB above noise floor */
    A_UINT32 rssi_comb;
    A_UINT32 rx_mcs[HTT_RX_PEER_STATS_NUM_MCS_COUNTERS];
    /**
     * element 0,1, ...7 -> NSS 1,2, ...8
     */
    A_UINT32 rx_nss[HTT_RX_PEER_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 rx_dcm[HTT_RX_PEER_STATS_NUM_DCM_COUNTERS];
    A_UINT32 rx_stbc[HTT_RX_PEER_STATS_NUM_MCS_COUNTERS];
    /**
     * element 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160 and 80+80 MHz
     */
    A_UINT32 rx_bw[HTT_RX_PEER_STATS_NUM_BW_COUNTERS];
    A_UINT32 rx_pream[HTT_RX_PEER_STATS_NUM_PREAMBLE_TYPES];
    /** units = dB above noise floor */
    A_UINT8  rssi_chain[HTT_RX_PEER_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PEER_STATS_NUM_BW_COUNTERS];

    /** Counters to track number of rx packets in each GI in each mcs (0-11) */
    A_UINT32 rx_gi[HTT_RX_PEER_STATS_NUM_GI_COUNTERS][HTT_RX_PEER_STATS_NUM_MCS_COUNTERS];

    A_UINT32 rx_ulofdma_non_data_ppdu; /** PPDU level */
    A_UINT32 rx_ulofdma_data_ppdu;     /** PPDU level */
    A_UINT32 rx_ulofdma_mpdu_ok;       /** MPDU level */
    A_UINT32 rx_ulofdma_mpdu_fail;     /** MPDU level */
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

    /** PPDU level */
    A_UINT32 rx_ulmumimo_non_data_ppdu;
    /** PPDU level */
    A_UINT32 rx_ulmumimo_data_ppdu;
    /** MPDU level */
    A_UINT32 rx_ulmumimo_mpdu_ok;
    /** mpdu level */
    A_UINT32 rx_ulmumimo_mpdu_fail;

    /** units = dB above noise floor */
    A_UINT8  rssi_chain_ext[HTT_RX_PEER_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PEER_STATS_NUM_BW_EXT_COUNTERS];

    /** Stats for MCS 12/13 */
    A_UINT32 rx_mcs_ext[HTT_RX_PEER_STATS_NUM_EXTRA_MCS_COUNTERS];
    A_UINT32 rx_stbc_ext[HTT_RX_PEER_STATS_NUM_EXTRA_MCS_COUNTERS];
    A_UINT32 rx_gi_ext[HTT_RX_PEER_STATS_NUM_GI_COUNTERS][HTT_RX_PEER_STATS_NUM_EXTRA_MCS_COUNTERS];
    A_UINT32 reduced_rx_bw[HTT_RX_PEER_STATS_NUM_REDUCED_CHAN_TYPES][HTT_RX_PEER_STATS_NUM_BW_COUNTERS];
    A_INT8   rx_per_chain_rssi_in_dbm_ext[HTT_RX_PEER_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PEER_STATS_NUM_BW_EXT_COUNTERS];
} htt_stats_peer_rx_rate_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_peer_rx_rate_stats_tlv htt_rx_peer_rate_stats_tlv;

typedef enum {
    HTT_PEER_STATS_REQ_MODE_NO_QUERY,
    HTT_PEER_STATS_REQ_MODE_QUERY_TQM,
    HTT_PEER_STATS_REQ_MODE_FLUSH_TQM,
} htt_peer_stats_req_mode_t;

typedef enum {
    HTT_PEER_STATS_CMN_TLV      = 0,
    HTT_PEER_DETAILS_TLV        = 1,
    HTT_TX_PEER_RATE_STATS_TLV  = 2,
    HTT_RX_PEER_RATE_STATS_TLV  = 3,
    HTT_TX_TID_STATS_TLV        = 4,
    HTT_RX_TID_STATS_TLV        = 5,
    HTT_MSDU_FLOW_STATS_TLV     = 6,
    HTT_PEER_SCHED_STATS_TLV    = 7,
    HTT_PEER_AX_OFDMA_STATS_TLV = 8,
    HTT_PEER_BE_OFDMA_STATS_TLV = 9,

    HTT_PEER_STATS_MAX_TLV      = 31,
} htt_peer_stats_tlv_enum;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 peer_id;
    /** Num of DL schedules for peer */
    A_UINT32 num_sched_dl;
    /** Num od UL schedules for peer */
    A_UINT32 num_sched_ul;
    /** Peer TX time */
    A_UINT32 peer_tx_active_dur_us_low;
    A_UINT32 peer_tx_active_dur_us_high;
    /** Peer RX time */
    A_UINT32 peer_rx_active_dur_us_low;
    A_UINT32 peer_rx_active_dur_us_high;
    A_UINT32 peer_curr_rate_kbps;
} htt_stats_peer_sched_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_peer_sched_stats_tlv htt_peer_sched_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 peer_id;
    A_UINT32 ax_basic_trig_count;
    A_UINT32 ax_basic_trig_err;
    A_UINT32 ax_bsr_trig_count;
    A_UINT32 ax_bsr_trig_err;
    A_UINT32 ax_mu_bar_trig_count;
    A_UINT32 ax_mu_bar_trig_err;
    A_UINT32 ax_basic_trig_with_per;
    A_UINT32 ax_bsr_trig_with_per;
    A_UINT32 ax_mu_bar_trig_with_per;
    /* is_airtime_large_for_dl_ofdma, is_airtime_large_for_ul_ofdma
     * These fields contain 2 counters each.  The first element in each
     * array counts how many times the airtime is short enough to use
     * OFDMA, and the second element in each array counts how many times the
     * airtime is too large to select OFDMA for the PPDUs involving the peer.
     */
    A_UINT32 is_airtime_large_for_dl_ofdma[2];
    A_UINT32 is_airtime_large_for_ul_ofdma[2];
    /* Last updated value of DL and UL queue depths for each peer per AC */
    A_UINT32 last_updated_dl_qdepth[HTT_NUM_AC_WMM];
    A_UINT32 last_updated_ul_qdepth[HTT_NUM_AC_WMM];
    /* Per peer Manual 11ax UL OFDMA trigger and trigger error counts */
    A_UINT32 ax_manual_ulofdma_trig_count;
    A_UINT32 ax_manual_ulofdma_trig_err_count;
} htt_stats_peer_ax_ofdma_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_peer_ax_ofdma_stats_tlv htt_peer_ax_ofdma_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 peer_id;
    /* Per peer Manual 11be UL OFDMA trigger and trigger error counts */
    A_UINT32 be_manual_ulofdma_trig_count;
    A_UINT32 be_manual_ulofdma_trig_err_count;
} htt_stats_peer_be_ofdma_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_peer_be_ofdma_stats_tlv htt_peer_be_ofdma_stats_tlv;


/* config_param0 */

#define HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS_IS_MAC_ADDR_M 0x00000001
#define HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS_IS_MAC_ADDR_S 0

#define HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS_IS_MAC_ADDR_GET(_var) \
    (((_var) & HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS_IS_MAC_ADDR_M) >> \
     HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS_IS_MAC_ADDR_S)

#define HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS_IS_MAC_ADDR_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS_IS_MAC_ADDR, _val); \
        ((_var) |= ((_val) << HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS_IS_MAC_ADDR_S)); \
    } while (0)
/* DEPRECATED
 * The old IS_peer_MAC_ADDR_SET macro name is being retained for now,
 * as an alias for the corrected macro name.
 * If/when all references to the old name are removed, the definition of
 * the old name will also be removed.
 */
#define HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS_IS_peer_MAC_ADDR_SET HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS_IS_MAC_ADDR_SET

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
 *   - HTT_STATS_PEER_SCHED_STATS_TAG
 *   - HTT_STATS_PEER_AX_OFDMA_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct _htt_peer_stats {
    htt_stats_peer_stats_cmn_tlv      cmn_tlv;

    htt_stats_peer_details_tlv        peer_details;
    /* from g_rate_info_stats */
    htt_stats_peer_tx_rate_stats_tlv  tx_rate;
    htt_stats_peer_rx_rate_stats_tlv  rx_rate;
    htt_stats_tx_tid_details_tlv      tx_tid_stats[1];
    htt_stats_rx_tid_details_tlv      rx_tid_stats[1];
    htt_stats_peer_msdu_flowq_tlv     msdu_flowq[1];
    htt_stats_tx_tid_details_v1_tlv   tx_tid_stats_v1[1];
    htt_stats_peer_sched_stats_tlv    peer_sched_stats;
    htt_stats_peer_ax_ofdma_stats_tlv ax_ofdma_stats;
    htt_stats_peer_be_ofdma_stats_tlv be_ofdma_stats;
} htt_peer_stats_t;
#endif /* ATH_TARGET */

/* =========== ACTIVE PEER LIST ========== */

/* STATS_TYPE: HTT_DBG_EXT_STATS_ACTIVE_PEERS_LIST
 * TLV_TAGS:
 *     - HTT_STATS_PEER_DETAILS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_peer_details_tlv peer_details[1];
} htt_active_peer_details_list_t;
#endif /* ATH_TARGET */

/* =========== MUMIMO HWQ stats =========== */

/* MU MIMO stats per hwQ */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** number of MU MIMO schedules posted to HW */
    A_UINT32      mu_mimo_sch_posted;
    /** number of MU MIMO schedules failed to post */
    A_UINT32      mu_mimo_sch_failed;
    /** number of MU MIMO PPDUs posted to HW */
    A_UINT32      mu_mimo_ppdu_posted;
} htt_stats_tx_hwq_mumimo_sch_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_hwq_mumimo_sch_stats_tlv htt_tx_hwq_mu_mimo_sch_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** 11AC DL MU MIMO number of mpdus queued to HW, per user */
    A_UINT32      mu_mimo_mpdus_queued_usr;
    /** 11AC DL MU MIMO number of mpdus tried over the air, per user */
    A_UINT32      mu_mimo_mpdus_tried_usr;
    /** 11AC DL MU MIMO number of mpdus failed acknowledgement, per user */
    A_UINT32      mu_mimo_mpdus_failed_usr;
    /** 11AC DL MU MIMO number of mpdus re-queued to HW, per user */
    A_UINT32      mu_mimo_mpdus_requeued_usr;
    /** 11AC DL MU MIMO BA not received, per user */
    A_UINT32      mu_mimo_err_no_ba_usr;
    /** 11AC DL MU MIMO mpdu underrun encountered, per user */
    A_UINT32      mu_mimo_mpdu_underrun_usr;
    /** 11AC DL MU MIMO ampdu underrun encountered, per user */
    A_UINT32      mu_mimo_ampdu_underrun_usr;
} htt_stats_tx_hwq_mumimo_mpdu_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_hwq_mumimo_mpdu_stats_tlv
    htt_tx_hwq_mu_mimo_mpdu_stats_tlv;

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

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [15 :  8]   :- hwq_id
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 mac_id__hwq_id__word;
} htt_stats_tx_hwq_mumimo_cmn_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_hwq_mumimo_cmn_stats_tlv htt_tx_hwq_mu_mimo_cmn_stats_tlv;

/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    struct {
        htt_stats_tx_hwq_mumimo_cmn_stats_tlv cmn_tlv;
        /** WAL_TX_STATS_MAX_GROUP_SIZE */
        htt_stats_tx_hwq_mumimo_sch_stats_tlv  mu_mimo_sch_stats_tlv[1];
        /** WAL_TX_STATS_TX_MAX_NUM_USERS */
        htt_stats_tx_hwq_mumimo_mpdu_stats_tlv mu_mimo_mpdu_stats_tlv[1];
    } hwq[1];
} htt_tx_hwq_mu_mimo_stats_t;
#endif /* ATH_TARGET */

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

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [15 :  8]   :- hwq_id
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 mac_id__hwq_id__word;

    /*--- PPDU level stats */
    /** Number of times ack is failed for the PPDU scheduled on this txQ */
    A_UINT32 xretry;
    /** Number of times sched cmd status reported mpdu underrun */
    A_UINT32 underrun_cnt;
    /** Number of times sched cmd is flushed */
    A_UINT32 flush_cnt;
    /** Number of times sched cmd is filtered */
    A_UINT32 filt_cnt;
    /** Number of times HWSCH uploaded null mpdu bitmap */
    A_UINT32 null_mpdu_bmap;
    /**
     * Number of times user ack or BA TLV is not seen on FES ring
     * where it is expected to be
     */
    A_UINT32 user_ack_failure;
    /** Number of times TQM processed ack TLV received from HWSCH */
    A_UINT32 ack_tlv_proc;
    /** Cache latest processed scheduler ID received from ack BA TLV */
    A_UINT32 sched_id_proc;
    /** Number of times TxPCU reported MPDUs transmitted for a user is zero */
    A_UINT32 null_mpdu_tx_count;
    /**
     * Number of times SW did not see any MPDU info bitmap TLV
     * on FES status ring
     */
    A_UINT32 mpdu_bmap_not_recvd;

    /*--- Selfgen stats per hwQ */
    /** Number of SU/MU BAR frames posted to hwQ */
    A_UINT32 num_bar;
    /** Number of RTS frames posted to hwQ */
    A_UINT32 rts;
    /** Number of cts2self frames posted to hwQ */
    A_UINT32 cts2self;
    /** Number of qos null frames posted to hwQ */
    A_UINT32 qos_null;

    /*--- MPDU level stats */
    /** mpdus tried Tx by HWSCH/TQM */
    A_UINT32 mpdu_tried_cnt;
    /** mpdus queued to HWSCH */
    A_UINT32 mpdu_queued_cnt;
    /** mpdus tried but ack was not received */
    A_UINT32 mpdu_ack_fail_cnt;
    /** This will include sched cmd flush and time based discard */
    A_UINT32 mpdu_filt_cnt;
    /** Number of MPDUs for which ACK was successful but no Tx happened */
    A_UINT32 false_mpdu_ack_count;

    /** Number of times txq timeout happened */
    A_UINT32 txq_timeout;
} htt_stats_tx_hwq_cmn_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_hwq_cmn_tlv htt_tx_hwq_stats_cmn_tlv;

#define HTT_TX_HWQ_DIFS_LATENCY_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) + /* hist_intvl */ \
                                                          (sizeof(A_UINT32) * (_num_elems)))
/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      hist_intvl;
    /** difs_latency_hist:
     * histogram of ppdu post to hwsch - > cmd status receive,
     * HTT_TX_HWQ_MAX_DIFS_LATENCY_BINS
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, difs_latency_hist);
} htt_stats_tx_hwq_difs_latency_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_hwq_difs_latency_tlv htt_tx_hwq_difs_latency_stats_tlv_v;

#define HTT_TX_HWQ_CMD_RESULT_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** cmd_result:
     * Histogram of sched cmd result,
     * HTT_TX_HWQ_MAX_CMD_RESULT_STATS
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, cmd_result);
} htt_stats_tx_hwq_cmd_result_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_hwq_cmd_result_tlv htt_tx_hwq_cmd_result_stats_tlv_v;

#define HTT_TX_HWQ_CMD_STALL_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** cmd_stall_status:
     * Histogram of various pause conitions
     * HTT_TX_HWQ_MAX_CMD_STALL_STATS
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, cmd_stall_status);
} htt_stats_tx_hwq_cmd_stall_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_hwq_cmd_stall_tlv htt_tx_hwq_cmd_stall_stats_tlv_v;

#define HTT_TX_HWQ_FES_RESULT_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** fes_result:
     * Histogram of number of user fes result,
     * HTT_TX_HWQ_MAX_FES_RESULT_STATS
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, fes_result);
} htt_stats_tx_hwq_fes_status_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_hwq_fes_status_tlv htt_tx_hwq_fes_result_stats_tlv_v;

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
    /** tried_mpdu_cnt_hist:
     * Histogram of number of mpdus on tried mpdu,
     * HTT_TX_HWQ_TRIED_MPDU_CNT_HIST
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, tried_mpdu_cnt_hist);
} htt_stats_tx_hwq_tried_mpdu_cnt_hist_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_hwq_tried_mpdu_cnt_hist_tlv
    htt_tx_hwq_tried_mpdu_cnt_hist_tlv_v;

#define HTT_TX_HWQ_TXOP_USED_CNT_HIST_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))
/* NOTE: Variable length TLV, use length spec to infer array size
 *
 * The txop_used_cnt_hist is the histogram of txop per burst. After
 * completing the burst, we identify the txop used in the burst and
 * incr the corresponding bin.
 * Each bin represents 1ms & we have 10 bins in this histogram.
 * they are defined in FW using the following macros
 * #define WAL_MAX_TXOP_USED_CNT_HISTOGRAM 10
 * #define WAL_TXOP_USED_HISTOGRAM_INTERVAL 1000 ( 1 ms )
 *
 * */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** txop_used_cnt_hist:
     * Histogram of txop used cnt,
     * HTT_TX_HWQ_TXOP_USED_CNT_HIST
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, txop_used_cnt_hist);
} htt_stats_tx_hwq_txop_used_cnt_hist_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_hwq_txop_used_cnt_hist_tlv
    htt_tx_hwq_txop_used_cnt_hist_tlv_v;

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
#ifdef ATH_TARGET
typedef struct _htt_tx_hwq_stats {
    htt_stats_string_tlv                     hwq_str_tlv;
    htt_stats_tx_hwq_cmn_tlv                 cmn_tlv;
    htt_stats_tx_hwq_difs_latency_tlv        difs_tlv;
    htt_stats_tx_hwq_cmd_result_tlv          cmd_result_tlv;
    htt_stats_tx_hwq_cmd_stall_tlv           cmd_stall_tlv;
    htt_stats_tx_hwq_fes_status_tlv          fes_stats_tlv;
    htt_stats_tx_hwq_tried_mpdu_cnt_hist_tlv tried_mpdu_tlv;
    htt_stats_tx_hwq_txop_used_cnt_hist_tlv  txop_used_tlv;
} htt_tx_hwq_stats_t;
#endif /* ATH_TARGET */

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

typedef enum {
    HTT_TXERR_NONE,
    HTT_TXERR_RESP,    /* response timeout, mismatch,
                        * BW mismatch, mimo ctrl mismatch,
                        * CRC error.. */
    HTT_TXERR_FILT,    /* blocked by tx filtering */
    HTT_TXERR_FIFO,    /* fifo, misc errors in HW */
    HTT_TXERR_SWABORT, /* software initialted abort (TX_ABORT) */

    HTT_TXERR_RESERVED1,
    HTT_TXERR_RESERVED2,
    HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS = 7,

    HTT_TXERR_INVALID = 0xff,
} htt_tx_err_status_t;


/* Matching enum for htt_tx_selfgen_sch_tsflag_error_stats */
typedef enum {
    HTT_TX_SELFGEN_SCH_TSFLAG_FLUSH_RCVD_ERR,
    HTT_TX_SELFGEN_SCH_TSFLAG_FILT_SCHED_CMD_ERR,
    HTT_TX_SELFGEN_SCH_TSFLAG_RESP_MISMATCH_ERR,
    HTT_TX_SELFGEN_SCH_TSFLAG_RESP_CBF_MIMO_CTRL_MISMATCH_ERR,
    HTT_TX_SELFGEN_SCH_TSFLAG_RESP_CBF_BW_MISMATCH_ERR,
    HTT_TX_SELFGEN_SCH_TSFLAG_RETRY_COUNT_FAIL_ERR,
    HTT_TX_SELFGEN_SCH_TSFLAG_RESP_TOO_LATE_RECEIVED_ERR,
    HTT_TX_SELFGEN_SCH_TSFLAG_SIFS_STALL_NO_NEXT_CMD_ERR,

    HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS = 8,
    HTT_TX_SELFGEN_SCH_TSFLAG_ERROR_STATS_VALID = 8
} htt_tx_selfgen_sch_tsflag_error_stats;

typedef enum {
    HTT_TX_MUMIMO_GRP_VALID,
    HTT_TX_MUMIMO_GRP_INVALID_NUM_MU_USERS_EXCEEDED_MU_MAX_USERS,
    HTT_TX_MUMIMO_GRP_INVALID_SCHED_ALGO_NOT_MU_COMPATIBLE_GID,
    HTT_TX_MUMIMO_GRP_INVALID_NON_PRIMARY_GRP,
    HTT_TX_MUMIMO_GRP_INVALID_ZERO_CANDIDATES,
    HTT_TX_MUMIMO_GRP_INVALID_MORE_CANDIDATES,
    HTT_TX_MUMIMO_GRP_INVALID_GROUP_SIZE_EXCEED_NSS,
    HTT_TX_MUMIMO_GRP_INVALID_GROUP_INELIGIBLE,
    HTT_TX_MUMIMO_GRP_INVALID,
    HTT_TX_MUMIMO_GRP_INVALID_GROUP_EFF_MU_TPUT_OMBPS,
    HTT_TX_MUMIMO_GRP_INVALID_MAX_REASON_CODE,
} htt_tx_mumimo_grp_invalid_reason_code_stats;

#define HTT_TX_PDEV_STATS_NUM_AC_MUMIMO_USER_STATS 4
#define HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS 8
#define HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS 8
#define HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS 74
#define HTT_TX_PDEV_STATS_NUM_UL_MUMIMO_USER_STATS 8
#define HTT_STATS_MAX_MUMIMO_GRP_SZ 8
/*
 * Each bin represents a 300 mbps throughput
 *  [0] - 0-300mbps;     [1] - 300-600mbps    [2] - 600-900mbps;    [3] - 900-1200mbps;  [4] - 1200-1500mbps
 *  [5] - 1500-1800mbps; [6] - 1800-2100mbps; [7] - 2100-2400mbps;  [8] - 2400-2700mbps; [9] - >=2700mbps
 */
#define HTT_STATS_MUMIMO_TPUT_NUM_BINS 10
#define HTT_STATS_MAX_INVALID_REASON_CODE \
    HTT_TX_MUMIMO_GRP_INVALID_MAX_REASON_CODE
/* Reasons stated in htt_tx_mumimo_grp_invalid_reason_code_stats */
#define HTT_TX_NUM_MUMIMO_GRP_INVALID_WORDS \
    (HTT_STATS_MAX_MUMIMO_GRP_SZ * HTT_STATS_MAX_INVALID_REASON_CODE)

#define HTT_MAX_NUM_SBT_INTR 4

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /*
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /** BAR sent out for SU transmission */
    A_UINT32 su_bar;
    /** SW generated RTS frame sent */
    A_UINT32 rts;
    /** SW generated CTS-to-self frame sent */
    A_UINT32 cts2self;
    /** SW generated QOS NULL frame sent */
    A_UINT32 qos_null;
    /** BAR sent for MU user 1 */
    A_UINT32 delayed_bar_1;
    /** BAR sent for MU user 2 */
    A_UINT32 delayed_bar_2;
    /** BAR sent for MU user 3 */
    A_UINT32 delayed_bar_3;
    /** BAR sent for MU user 4 */
    A_UINT32 delayed_bar_4;
    /** BAR sent for MU user 5 */
    A_UINT32 delayed_bar_5;
    /** BAR sent for MU user 6 */
    A_UINT32 delayed_bar_6;
    /** BAR sent for MU user 7 */
    A_UINT32 delayed_bar_7;
    A_UINT32 bar_with_tqm_head_seq_num;
    A_UINT32 bar_with_tid_seq_num;
    /** SW generated RTS frame queued to the HW */
    A_UINT32 su_sw_rts_queued;
    /** SW generated RTS frame sent over the air */
    A_UINT32 su_sw_rts_tried;
    /** SW generated RTS frame completed with error */
    A_UINT32 su_sw_rts_err;
    /** SW generated RTS frame flushed */
    A_UINT32 su_sw_rts_flushed;
    /** CTS (RTS response) received in different BW */
    A_UINT32 su_sw_rts_rcvd_cts_diff_bw;
/* START DEPRECATED FIELDS */
    /** 11AX HE MU Combined Freq. BSRP Trigger frame sent over the air */
    A_UINT32 combined_ax_bsr_trigger_tried[HTT_NUM_AC_WMM];
    /** 11AX HE MU Combined Freq. BSRP Trigger completed with error(s) */
    A_UINT32 combined_ax_bsr_trigger_err[HTT_NUM_AC_WMM];
    /** 11AX HE MU Standalone Freq. BSRP Trigger frame sent over the air */
    A_UINT32 standalone_ax_bsr_trigger_tried[HTT_NUM_AC_WMM];
    /** 11AX HE MU Standalone Freq. BSRP Trigger completed with error(s) */
    A_UINT32 standalone_ax_bsr_trigger_err[HTT_NUM_AC_WMM];
/* END DEPRECATED FIELDS */
    /** smart_basic_trig_sch_histogram:
     * Count how many times the interval between predictive basic triggers
     * sent to a given STA based on analysis of that STA's traffic patterns
     * is within a given range:
     *
     * smart_basic_trig_sch_histogram[0]: SBT interval <= 10 ms
     * smart_basic_trig_sch_histogram[1]: 10 ms < SBT interval <= 20 ms
     * smart_basic_trig_sch_histogram[2]: 20 ms < SBT interval <= 30 ms
     * smart_basic_trig_sch_histogram[3]: 30 ms < SBT interval <= 40 ms
     *
     * (Smart basic triggers are only used with intervals <= 40 ms.)
     */
    A_UINT32 smart_basic_trig_sch_histogram[HTT_MAX_NUM_SBT_INTR];
} htt_stats_tx_selfgen_cmn_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_selfgen_cmn_stats_tlv htt_tx_selfgen_cmn_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** 11AC VHT SU NDPA frame sent over the air */
    A_UINT32 ac_su_ndpa;
    /** 11AC VHT SU NDP frame sent over the air */
    A_UINT32 ac_su_ndp;
    /** 11AC VHT MU MIMO NDPA frame sent over the air */
    A_UINT32 ac_mu_mimo_ndpa;
    /** 11AC VHT MU MIMO NDP frame sent over the air */
    A_UINT32 ac_mu_mimo_ndp;
    /** 11AC VHT MU MIMO BR-POLL for user 1 sent over the air */
    A_UINT32 ac_mu_mimo_brpoll_1;
    /** 11AC VHT MU MIMO BR-POLL for user 2 sent over the air */
    A_UINT32 ac_mu_mimo_brpoll_2;
    /** 11AC VHT MU MIMO BR-POLL for user 3 sent over the air */
    A_UINT32 ac_mu_mimo_brpoll_3;
    /** 11AC VHT SU NDPA frame queued to the HW */
    A_UINT32 ac_su_ndpa_queued;
    /** 11AC VHT SU NDP frame queued to the HW */
    A_UINT32 ac_su_ndp_queued;
    /** 11AC VHT MU MIMO NDPA frame queued to the HW */
    A_UINT32 ac_mu_mimo_ndpa_queued;
    /** 11AC VHT MU MIMO NDP frame queued to the HW */
    A_UINT32 ac_mu_mimo_ndp_queued;
    /** 11AC VHT MU MIMO BR-POLL for user 1 frame queued to the HW */
    A_UINT32 ac_mu_mimo_brpoll_1_queued;
    /** 11AC VHT MU MIMO BR-POLL for user 2 frame queued to the HW */
    A_UINT32 ac_mu_mimo_brpoll_2_queued;
    /** 11AC VHT MU MIMO BR-POLL for user 3 frame queued to the HW */
    A_UINT32 ac_mu_mimo_brpoll_3_queued;
} htt_stats_tx_selfgen_ac_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_selfgen_ac_stats_tlv htt_tx_selfgen_ac_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** 11AX HE SU NDPA frame sent over the air */
    A_UINT32 ax_su_ndpa;
    /** 11AX HE NDP frame sent over the air */
    A_UINT32 ax_su_ndp;
    /** 11AX HE MU MIMO NDPA frame sent over the air */
    A_UINT32 ax_mu_mimo_ndpa;
    /** 11AX HE MU MIMO NDP frame sent over the air */
    A_UINT32 ax_mu_mimo_ndp;
    union {
        struct {
            /* deprecated old names */
            A_UINT32 ax_mu_mimo_brpoll_1;
            A_UINT32 ax_mu_mimo_brpoll_2;
            A_UINT32 ax_mu_mimo_brpoll_3;
            A_UINT32 ax_mu_mimo_brpoll_4;
            A_UINT32 ax_mu_mimo_brpoll_5;
            A_UINT32 ax_mu_mimo_brpoll_6;
            A_UINT32 ax_mu_mimo_brpoll_7;
        };
        /** 11AX HE MU BR-POLL frame for users 1 - 7 sent over the air */
        A_UINT32 ax_mu_mimo_brpoll[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS - 1];
    };
    /** 11AX HE MU Basic Trigger frame sent over the air */
    A_UINT32 ax_basic_trigger;
    /** 11AX HE MU BSRP Trigger frame sent over the air */
    A_UINT32 ax_bsr_trigger;
    /** 11AX HE MU BAR Trigger frame sent over the air */
    A_UINT32 ax_mu_bar_trigger;
    /** 11AX HE MU RTS Trigger frame sent over the air */
    A_UINT32 ax_mu_rts_trigger;
    /** 11AX HE MU UL-MUMIMO Trigger frame sent over the air */
    A_UINT32 ax_ulmumimo_trigger;
    /** 11AX HE SU NDPA frame queued to the HW */
    A_UINT32 ax_su_ndpa_queued;
    /** 11AX HE SU NDP frame queued to the HW */
    A_UINT32 ax_su_ndp_queued;
    /** 11AX HE MU MIMO NDPA frame queued to the HW */
    A_UINT32 ax_mu_mimo_ndpa_queued;
    /** 11AX HE MU MIMO NDP frame queued to the HW */
    A_UINT32 ax_mu_mimo_ndp_queued;
    /** 11AX HE MU BR-POLL frame for users 1 - 7 queued to the HW */
    A_UINT32 ax_mu_mimo_brpoll_queued[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS - 1];
    /**
     * 11AX HE UL-MUMIMO Trigger frame for users 0 - 7
     * successfully sent over the air
     */
    A_UINT32 ax_ul_mumimo_trigger[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS];
    /** 11AX HE MU Combined Freq. BSRP Trigger frame sent over the air */
    A_UINT32 combined_ax_bsr_trigger_tried[HTT_NUM_AC_WMM];
    /** 11AX HE MU Combined Freq. BSRP Trigger completed with error(s) */
    A_UINT32 combined_ax_bsr_trigger_err[HTT_NUM_AC_WMM];
    /** 11AX HE MU Standalone Freq. BSRP Trigger frame sent over the air */
    A_UINT32 standalone_ax_bsr_trigger_tried[HTT_NUM_AC_WMM];
    /** 11AX HE MU Standalone Freq. BSRP Trigger completed with error(s) */
    A_UINT32 standalone_ax_bsr_trigger_err[HTT_NUM_AC_WMM];
    /** 11AX HE Manual Single-User UL OFDMA Trigger frame sent over the air */
    A_UINT32 manual_ax_su_ulofdma_basic_trigger[HTT_NUM_AC_WMM];
    /** 11AX HE Manual Single-User UL OFDMA Trigger completed with error(s) */
    A_UINT32 manual_ax_su_ulofdma_basic_trigger_err[HTT_NUM_AC_WMM];
    /** 11AX HE Manual Multi-User UL OFDMA Trigger frame sent over the air */
    A_UINT32 manual_ax_mu_ulofdma_basic_trigger[HTT_NUM_AC_WMM];
    /** 11AX HE Manual Multi-User UL OFDMA Trigger completed with error(s) */
    A_UINT32 manual_ax_mu_ulofdma_basic_trigger_err[HTT_NUM_AC_WMM];
    /** 11AX HE UL OFDMA Basic Trigger frames per AC */
    A_UINT32 ax_basic_trigger_per_ac[HTT_NUM_AC_WMM];
    /** 11AX HE UL OFDMA Basic Trigger frames per AC completed with error(s) */
    A_UINT32 ax_basic_trigger_errors_per_ac[HTT_NUM_AC_WMM];
    /** 11AX HE MU-BAR Trigger frames per AC */
    A_UINT32 ax_mu_bar_trigger_per_ac[HTT_NUM_AC_WMM];
    /** 11AX HE MU-BAR Trigger frames per AC completed with error(s) */
    A_UINT32 ax_mu_bar_trigger_errors_per_ac[HTT_NUM_AC_WMM];
} htt_stats_tx_selfgen_ax_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_selfgen_ax_stats_tlv htt_tx_selfgen_ax_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** 11be EHT SU NDPA frame sent over the air */
    A_UINT32 be_su_ndpa;
    /** 11be EHT NDP frame sent over the air */
    A_UINT32 be_su_ndp;
    /** 11be EHT MU MIMO NDPA frame sent over the air */
    A_UINT32 be_mu_mimo_ndpa;
    /** 11be EHT MU MIMO NDP frame sent over theT air */
    A_UINT32 be_mu_mimo_ndp;
    /** 11be EHT MU BR-POLL frame for users 1 - 7 sent over the air */
    A_UINT32 be_mu_mimo_brpoll[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS - 1];
    /** 11be EHT MU Basic Trigger frame sent over the air */
    A_UINT32 be_basic_trigger;
    /** 11be EHT MU BSRP Trigger frame sent over the air */
    A_UINT32 be_bsr_trigger;
    /** 11be EHT MU BAR Trigger frame sent over the air */
    A_UINT32 be_mu_bar_trigger;
    /** 11be EHT MU RTS Trigger frame sent over the air */
    A_UINT32 be_mu_rts_trigger;
    /** 11be EHT MU UL-MUMIMO Trigger frame sent over the air */
    A_UINT32 be_ulmumimo_trigger;
    /** 11be EHT SU NDPA frame queued to the HW */
    A_UINT32 be_su_ndpa_queued;
    /** 11be EHT SU NDP frame queued to the HW */
    A_UINT32 be_su_ndp_queued;
    /** 11be EHT MU MIMO NDPA frame queued to the HW */
    A_UINT32 be_mu_mimo_ndpa_queued;
    /** 11be EHT MU MIMO NDP frame queued to the HW */
    A_UINT32 be_mu_mimo_ndp_queued;
    /** 11be EHT MU BR-POLL frame for users 1 - 7 queued to the HW */
    A_UINT32 be_mu_mimo_brpoll_queued[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS - 1];
    /**
     * 11be EHT UL-MUMIMO Trigger frame for users 0 - 7
     * successfully sent over the air
     */
    A_UINT32 be_ul_mumimo_trigger[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS];
    /** 11BE EHT MU Combined Freq. BSRP Trigger frame sent over the air */
    A_UINT32 combined_be_bsr_trigger_tried[HTT_NUM_AC_WMM];
    /** 11BE EHT MU Combined Freq. BSRP Trigger completed with error(s) */
    A_UINT32 combined_be_bsr_trigger_err[HTT_NUM_AC_WMM];
    /** 11BE EHT MU Standalone Freq. BSRP Trigger frame sent over the air */
    A_UINT32 standalone_be_bsr_trigger_tried[HTT_NUM_AC_WMM];
    /** 11BE EHT MU Standalone Freq. BSRP Trigger completed with error(s) */
    A_UINT32 standalone_be_bsr_trigger_err[HTT_NUM_AC_WMM];
    /** 11BE EHT Manual Single-User UL OFDMA Trigger frame sent over the air */
    A_UINT32 manual_be_su_ulofdma_basic_trigger[HTT_NUM_AC_WMM];
    /** 11BE EHT Manual Single-User UL OFDMA Trigger completed with error(s) */
    A_UINT32 manual_be_su_ulofdma_basic_trigger_err[HTT_NUM_AC_WMM];
    /** 11BE EHT Manual Multi-User UL OFDMA Trigger frame sent over the air */
    A_UINT32 manual_be_mu_ulofdma_basic_trigger[HTT_NUM_AC_WMM];
    /** 11BE EHT Manual Multi-User UL OFDMA Trigger completed with error(s) */
    A_UINT32 manual_be_mu_ulofdma_basic_trigger_err[HTT_NUM_AC_WMM];
    /** 11BE EHT UL OFDMA Basic Trigger frames per AC */
    A_UINT32 be_basic_trigger_per_ac[HTT_NUM_AC_WMM];
    /** 11BE EHT UL OFDMA Basic Trigger frames per AC completed with error(s) */
    A_UINT32 be_basic_trigger_errors_per_ac[HTT_NUM_AC_WMM];
    /** 11BE EHT MU-BAR Trigger frames per AC */
    A_UINT32 be_mu_bar_trigger_per_ac[HTT_NUM_AC_WMM];
    /** 11BE EHT MU-BAR Trigger frames per AC completed with error(s) */
    A_UINT32 be_mu_bar_trigger_errors_per_ac[HTT_NUM_AC_WMM];
} htt_stats_tx_selfgen_be_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_selfgen_be_stats_tlv htt_tx_selfgen_be_stats_tlv;

typedef struct { /* DEPRECATED */
    htt_tlv_hdr_t tlv_hdr;
    /** 11AX HE OFDMA NDPA frame queued to the HW */
    A_UINT32 ax_ofdma_ndpa_queued[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** 11AX HE OFDMA NDPA frame sent over the air */
    A_UINT32 ax_ofdma_ndpa_tried[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** 11AX HE OFDMA NDPA frame flushed by HW */
    A_UINT32 ax_ofdma_ndpa_flushed[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** 11AX HE OFDMA NDPA frame completed with error(s) */
    A_UINT32 ax_ofdma_ndpa_err[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
} htt_stats_txbf_ofdma_ndpa_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_ndpa_stats_tlv htt_txbf_ofdma_ndpa_stats_tlv;

typedef struct { /* DEPRECATED */
    htt_tlv_hdr_t tlv_hdr;
    /** 11AX HE OFDMA NDP frame queued to the HW */
    A_UINT32 ax_ofdma_ndp_queued[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** 11AX HE OFDMA NDPA frame sent over the air */
    A_UINT32 ax_ofdma_ndp_tried[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** 11AX HE OFDMA NDPA frame flushed by HW */
    A_UINT32 ax_ofdma_ndp_flushed[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** 11AX HE OFDMA NDPA frame completed with error(s) */
    A_UINT32 ax_ofdma_ndp_err[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
} htt_stats_txbf_ofdma_ndp_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_ndp_stats_tlv htt_txbf_ofdma_ndp_stats_tlv;

typedef struct { /* DEPRECATED */
    htt_tlv_hdr_t tlv_hdr;
    /** 11AX HE OFDMA MU BRPOLL frame queued to the HW */
    A_UINT32 ax_ofdma_brpoll_queued[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** 11AX HE OFDMA MU BRPOLL frame sent over the air */
    A_UINT32 ax_ofdma_brpoll_tried[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** 11AX HE OFDMA MU BRPOLL frame flushed by HW */
    A_UINT32 ax_ofdma_brpoll_flushed[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** 11AX HE OFDMA MU BRPOLL frame completed with error(s) */
    A_UINT32 ax_ofdma_brp_err[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /**
     * Number of CBF(s) received when 11AX HE OFDMA MU BRPOLL frame
     * completed with error(s)
     */
    A_UINT32 ax_ofdma_brp_err_num_cbf_rcvd[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS+1];
} htt_stats_txbf_ofdma_brp_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_brp_stats_tlv htt_txbf_ofdma_brp_stats_tlv;

typedef struct { /* DEPRECATED */
    htt_tlv_hdr_t tlv_hdr;
    /**
     * 11AX HE OFDMA PPDUs that were sent over the air with steering
     * (TXBF + OFDMA)
     */
    A_UINT32 ax_ofdma_num_ppdu_steer[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** 11AX HE OFDMA PPDUs that were sent over the air in open loop */
    A_UINT32 ax_ofdma_num_ppdu_ol[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /**
     * 11AX HE OFDMA number of users for which CBF prefetch was initiated
     * to PHY HW during TX
     */
    A_UINT32 ax_ofdma_num_usrs_prefetch[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /**
     * 11AX HE OFDMA number of users for which sounding was initiated
     * during TX
     */
    A_UINT32 ax_ofdma_num_usrs_sound[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** 11AX HE OFDMA number of users for which sounding was forced during TX */
    A_UINT32 ax_ofdma_num_usrs_force_sound[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
} htt_stats_txbf_ofdma_steer_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_steer_stats_tlv htt_txbf_ofdma_steer_stats_tlv;

/* Note:
 * This struct htt_tx_pdev_txbf_ofdma_stats_t and all its constituent
 * struct TLVs are deprecated, due to the need for restructuring these
 * stats into a variable length array
 */
#ifdef ATH_TARGET
typedef struct { /* DEPRECATED */
    htt_stats_txbf_ofdma_ndpa_stats_tlv  ofdma_ndpa_tlv;
    htt_stats_txbf_ofdma_ndp_stats_tlv   ofdma_ndp_tlv;
    htt_stats_txbf_ofdma_brp_stats_tlv   ofdma_brp_tlv;
    htt_stats_txbf_ofdma_steer_stats_tlv ofdma_steer_tlv;
} htt_tx_pdev_txbf_ofdma_stats_t;
#endif /* ATH_TARGET */

typedef struct {
    /** 11AX HE OFDMA NDPA frame queued to the HW */
    A_UINT32 ax_ofdma_ndpa_queued;
    /** 11AX HE OFDMA NDPA frame sent over the air */
    A_UINT32 ax_ofdma_ndpa_tried;
    /** 11AX HE OFDMA NDPA frame flushed by HW */
    A_UINT32 ax_ofdma_ndpa_flushed;
    /** 11AX HE OFDMA NDPA frame completed with error(s) */
    A_UINT32 ax_ofdma_ndpa_err;
} htt_txbf_ofdma_ax_ndpa_stats_elem_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * This field is populated with the num of elems in the ax_ndpa[]
     * variable length array.
     */
    A_UINT32 num_elems_ax_ndpa_arr;
    /**
     * This field will be filled by target with value of
     * sizeof(htt_txbf_ofdma_ax_ndpa_stats_elem_t).
     * This is for allowing host to infer how much data target has provided,
     * even if it using different version of the struct def than what target
     * had used.
     */
    A_UINT32 arr_elem_size_ax_ndpa;
    HTT_STATS_VAR_LEN_ARRAY1(htt_txbf_ofdma_ax_ndpa_stats_elem_t, ax_ndpa);
} htt_stats_txbf_ofdma_ax_ndpa_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_ax_ndpa_stats_tlv htt_txbf_ofdma_ax_ndpa_stats_tlv;

typedef struct {
    /** 11AX HE OFDMA NDP frame queued to the HW */
    A_UINT32 ax_ofdma_ndp_queued;
    /** 11AX HE OFDMA NDPA frame sent over the air */
    A_UINT32 ax_ofdma_ndp_tried;
    /** 11AX HE OFDMA NDPA frame flushed by HW */
    A_UINT32 ax_ofdma_ndp_flushed;
    /** 11AX HE OFDMA NDPA frame completed with error(s) */
    A_UINT32 ax_ofdma_ndp_err;
} htt_txbf_ofdma_ax_ndp_stats_elem_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * This field is populated with the num of elems in the the ax_ndp[]
     * variable length array.
     */
    A_UINT32 num_elems_ax_ndp_arr;
    /**
     * This field will be filled by target with value of
     * sizeof(htt_txbf_ofdma_ax_ndp_stats_elem_t).
     * This is for allowing host to infer how much data target has provided,
     * even if it using different version of the struct def than what target
     * had used.
     */
    A_UINT32 arr_elem_size_ax_ndp;
    HTT_STATS_VAR_LEN_ARRAY1(htt_txbf_ofdma_ax_ndp_stats_elem_t, ax_ndp);
} htt_stats_txbf_ofdma_ax_ndp_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_ax_ndp_stats_tlv htt_txbf_ofdma_ax_ndp_stats_tlv;

typedef struct {
    /** 11AX HE OFDMA MU BRPOLL frame queued to the HW */
    A_UINT32 ax_ofdma_brpoll_queued;
    /** 11AX HE OFDMA MU BRPOLL frame sent over the air */
    A_UINT32 ax_ofdma_brpoll_tried;
    /** 11AX HE OFDMA MU BRPOLL frame flushed by HW */
    A_UINT32 ax_ofdma_brpoll_flushed;
    /** 11AX HE OFDMA MU BRPOLL frame completed with error(s) */
    A_UINT32 ax_ofdma_brp_err;
    /**
     * Number of CBF(s) received when 11AX HE OFDMA MU BRPOLL frame
     * completed with error(s)
     */
    A_UINT32 ax_ofdma_brp_err_num_cbf_rcvd;
} htt_txbf_ofdma_ax_brp_stats_elem_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * This field is populated with the num of elems in the the ax_brp[]
     * variable length array.
     */
    A_UINT32 num_elems_ax_brp_arr;
    /**
     * This field will be filled by target with value of
     * sizeof(htt_txbf_ofdma_ax_brp_stats_elem_t).
     * This is for allowing host to infer how much data target has provided,
     * even if it using different version of the struct than what target
     * had used.
     */
    A_UINT32 arr_elem_size_ax_brp;
    HTT_STATS_VAR_LEN_ARRAY1(htt_txbf_ofdma_ax_brp_stats_elem_t, ax_brp);
} htt_stats_txbf_ofdma_ax_brp_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_ax_brp_stats_tlv htt_txbf_ofdma_ax_brp_stats_tlv;

typedef struct {
    /**
     * 11AX HE OFDMA PPDUs that were sent over the air with steering
     * (TXBF + OFDMA)
     */
    A_UINT32 ax_ofdma_num_ppdu_steer;
    /** 11AX HE OFDMA PPDUs that were sent over the air in open loop */
    A_UINT32 ax_ofdma_num_ppdu_ol;
    /**
     * 11AX HE OFDMA number of users for which CBF prefetch was initiated
     * to PHY HW during TX
     */
    A_UINT32 ax_ofdma_num_usrs_prefetch;
    /**
     * 11AX HE OFDMA number of users for which sounding was initiated
     * during TX
     */
    A_UINT32 ax_ofdma_num_usrs_sound;
    /** 11AX HE OFDMA number of users for which sounding was forced during TX */
    A_UINT32 ax_ofdma_num_usrs_force_sound;
} htt_txbf_ofdma_ax_steer_stats_elem_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * This field is populated with the num of elems in the ax_steer[]
     * variable length array.
     */
    A_UINT32 num_elems_ax_steer_arr;
    /**
     * This field will be filled by target with value of
     * sizeof(htt_txbf_ofdma_ax_steer_stats_elem_t).
     * This is for allowing host to infer how much data target has provided,
     * even if it using different version of the struct than what target
     * had used.
     */
    A_UINT32 arr_elem_size_ax_steer;
    HTT_STATS_VAR_LEN_ARRAY1(htt_txbf_ofdma_ax_steer_stats_elem_t, ax_steer);
} htt_stats_txbf_ofdma_ax_steer_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_ax_steer_stats_tlv
    htt_txbf_ofdma_ax_steer_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* 11AX HE OFDMA MPDUs tried in rbo steering */
    A_UINT32 ax_ofdma_rbo_steer_mpdus_tried;
    /* 11AX HE OFDMA MPDUs failed in rbo steering */
    A_UINT32 ax_ofdma_rbo_steer_mpdus_failed;
    /* 11AX HE OFDMA MPDUs tried in sifs steering */
    A_UINT32 ax_ofdma_sifs_steer_mpdus_tried;
    /* 11AX HE OFDMA MPDUs failed in sifs steering */
    A_UINT32 ax_ofdma_sifs_steer_mpdus_failed;
} htt_stats_txbf_ofdma_ax_steer_mpdu_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_ax_steer_mpdu_stats_tlv
    htt_txbf_ofdma_ax_steer_mpdu_stats_tlv;

typedef struct {
    /** 11BE EHT OFDMA NDPA frame queued to the HW */
    A_UINT32 be_ofdma_ndpa_queued;
    /** 11BE EHT OFDMA NDPA frame sent over the air */
    A_UINT32 be_ofdma_ndpa_tried;
    /** 11BE EHT OFDMA NDPA frame flushed by HW */
    A_UINT32 be_ofdma_ndpa_flushed;
    /** 11BE EHT OFDMA NDPA frame completed with error(s) */
    A_UINT32 be_ofdma_ndpa_err;
} htt_txbf_ofdma_be_ndpa_stats_elem_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * This field is populated with the num of elems in the be_ndpa[]
     * variable length array.
     */
    A_UINT32 num_elems_be_ndpa_arr;
    /**
     * This field will be filled by target with value of
     * sizeof(htt_txbf_ofdma_be_ndpa_stats_elem_t).
     * This is for allowing host to infer how much data target has provided,
     * even if it using different version of the struct than what target
     * had used.
     */
    A_UINT32 arr_elem_size_be_ndpa;
    HTT_STATS_VAR_LEN_ARRAY1(htt_txbf_ofdma_be_ndpa_stats_elem_t, be_ndpa);
} htt_stats_txbf_ofdma_be_ndpa_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_be_ndpa_stats_tlv htt_txbf_ofdma_be_ndpa_stats_tlv;

typedef struct {
    /** 11BE EHT OFDMA NDP frame queued to the HW */
    A_UINT32 be_ofdma_ndp_queued;
    /** 11BE EHT OFDMA NDPA frame sent over the air */
    A_UINT32 be_ofdma_ndp_tried;
    /** 11BE EHT OFDMA NDPA frame flushed by HW */
    A_UINT32 be_ofdma_ndp_flushed;
    /** 11BE EHT OFDMA NDPA frame completed with error(s) */
    A_UINT32 be_ofdma_ndp_err;
} htt_txbf_ofdma_be_ndp_stats_elem_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * This field is populated with the num of elems in the be_ndp[]
     * variable length array.
     */
    A_UINT32 num_elems_be_ndp_arr;
    /**
     * This field will be filled by target with value of
     * sizeof(htt_txbf_ofdma_be_ndp_stats_elem_t).
     * This is for allowing host to infer how much data target has provided,
     * even if it using different version of the struct than what target
     * had used.
     */
    A_UINT32 arr_elem_size_be_ndp;
    HTT_STATS_VAR_LEN_ARRAY1(htt_txbf_ofdma_be_ndp_stats_elem_t, be_ndp);
} htt_stats_txbf_ofdma_be_ndp_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_be_ndp_stats_tlv htt_txbf_ofdma_be_ndp_stats_tlv;

typedef struct {
    /** 11BE EHT OFDMA MU BRPOLL frame queued to the HW */
    A_UINT32 be_ofdma_brpoll_queued;
    /** 11BE EHT OFDMA MU BRPOLL frame sent over the air */
    A_UINT32 be_ofdma_brpoll_tried;
    /** 11BE EHT OFDMA MU BRPOLL frame flushed by HW */
    A_UINT32 be_ofdma_brpoll_flushed;
    /** 11BE EHT OFDMA MU BRPOLL frame completed with error(s) */
    A_UINT32 be_ofdma_brp_err;
    /**
     * Number of CBF(s) received when 11BE EHT OFDMA MU BRPOLL frame
     * completed with error(s)
     */
    A_UINT32 be_ofdma_brp_err_num_cbf_rcvd;
} htt_txbf_ofdma_be_brp_stats_elem_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * This field is populated with the num of elems in the be_brp[]
     * variable length array.
     */
    A_UINT32 num_elems_be_brp_arr;
    /**
     * This field will be filled by target with value of
     * sizeof(htt_txbf_ofdma_be_brp_stats_elem_t).
     * This is for allowing host to infer how much data target has provided,
     * even if it using different version of the struct than what target
     * had used
     */
    A_UINT32 arr_elem_size_be_brp;
    HTT_STATS_VAR_LEN_ARRAY1(htt_txbf_ofdma_be_brp_stats_elem_t, be_brp);
} htt_stats_txbf_ofdma_be_brp_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_be_brp_stats_tlv htt_txbf_ofdma_be_brp_stats_tlv;

typedef struct {
    /**
     * 11BE EHT OFDMA PPDUs that were sent over the air with steering
     * (TXBF + OFDMA)
     */
    A_UINT32 be_ofdma_num_ppdu_steer;
    /** 11BE EHT OFDMA PPDUs that were sent over the air in open loop */
    A_UINT32 be_ofdma_num_ppdu_ol;
    /**
     * 11BE EHT OFDMA number of users for which CBF prefetch was initiated
     * to PHY HW during TX
     */
    A_UINT32 be_ofdma_num_usrs_prefetch;
    /**
     * 11BE EHT OFDMA number of users for which sounding was initiated
     * during TX
     */
    A_UINT32 be_ofdma_num_usrs_sound;
    /**
     * 11BE EHT OFDMA number of users for which sounding was forced during TX
     */
    A_UINT32 be_ofdma_num_usrs_force_sound;
} htt_txbf_ofdma_be_steer_stats_elem_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * This field is populated with the num of elems in the be_steer[]
     * variable length array.
     */
    A_UINT32 num_elems_be_steer_arr;
    /**
     * This field will be filled by target with value of
     * sizeof(htt_txbf_ofdma_be_steer_stats_elem_t).
     * This is for allowing host to infer how much data target has provided,
     * even if it using different version of the struct than what target
     * had used.
     */
    A_UINT32 arr_elem_size_be_steer;
    HTT_STATS_VAR_LEN_ARRAY1(htt_txbf_ofdma_be_steer_stats_elem_t, be_steer);
} htt_stats_txbf_ofdma_be_steer_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_be_steer_stats_tlv
    htt_txbf_ofdma_be_steer_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* 11BE EHT OFDMA MPDUs tried in rbo steering */
    A_UINT32 be_ofdma_rbo_steer_mpdus_tried;
    /* 11BE EHT OFDMA MPDUs failed in rbo steering */
    A_UINT32 be_ofdma_rbo_steer_mpdus_failed;
    /* 11BE EHT OFDMA MPDUs tried in sifs steering */
    A_UINT32 be_ofdma_sifs_steer_mpdus_tried;
    /* 11BE EHT OFDMA MPDUs failed in sifs steering */
    A_UINT32 be_ofdma_sifs_steer_mpdus_failed;
} htt_stats_txbf_ofdma_be_steer_mpdu_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_txbf_ofdma_be_steer_mpdu_stats_tlv
    htt_txbf_ofdma_be_steer_mpdu_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_TXBF_OFDMA
 * TLV_TAGS:
 *      - HTT_STATS_TXBF_OFDMA_NDPA_STATS_TAG
 *      - HTT_STATS_TXBF_OFDMA_NDP_STATS_TAG
 *      - HTT_STATS_TXBF_OFDMA_BRP_STATS_TAG
 *      - HTT_STATS_TXBF_OFDMA_STEER_STATS_TAG
 *      - HTT_STATS_TXBF_OFDMA_AX_STEER_MPDU_STATS_TAG
 *      - HTT_STATS_TXBF_OFDMA_BE_NDPA_STATS_TAG
 *      - HTT_STATS_TXBF_OFDMA_BE_NDP_STATS_TAG
 *      - HTT_STATS_TXBF_OFDMA_BE_BRP_STATS_TAG
 *      - HTT_STATS_TXBF_OFDMA_BE_STEER_STATS_TAG
 *      - HTT_STATS_TXBF_OFDMA_BE_STEER_MPDU_STATS_TAG
 */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** 11AC VHT SU NDP frame completed with error(s) */
    A_UINT32 ac_su_ndp_err;
    /** 11AC VHT SU NDPA frame completed with error(s) */
    A_UINT32 ac_su_ndpa_err;
    /** 11AC VHT MU MIMO NDPA frame completed with error(s) */
    A_UINT32 ac_mu_mimo_ndpa_err;
    /** 11AC VHT MU MIMO NDP frame completed with error(s) */
    A_UINT32 ac_mu_mimo_ndp_err;
    /** 11AC VHT MU MIMO BRPOLL for user 1 frame completed with error(s) */
    A_UINT32 ac_mu_mimo_brp1_err;
    /** 11AC VHT MU MIMO BRPOLL for user 2 frame completed with error(s) */
    A_UINT32 ac_mu_mimo_brp2_err;
    /** 11AC VHT MU MIMO BRPOLL for user 3 frame completed with error(s) */
    A_UINT32 ac_mu_mimo_brp3_err;
    /** 11AC VHT SU NDPA frame flushed by HW */
    A_UINT32 ac_su_ndpa_flushed;
    /** 11AC VHT SU NDP frame flushed by HW */
    A_UINT32 ac_su_ndp_flushed;
    /** 11AC VHT MU MIMO NDPA frame flushed by HW */
    A_UINT32 ac_mu_mimo_ndpa_flushed;
    /** 11AC VHT MU MIMO NDP frame flushed by HW */
    A_UINT32 ac_mu_mimo_ndp_flushed;
    /** 11AC VHT MU MIMO BRPOLL for user 1 frame flushed by HW */
    A_UINT32 ac_mu_mimo_brpoll1_flushed;
    /** 11AC VHT MU MIMO BRPOLL for user 2 frame flushed by HW */
    A_UINT32 ac_mu_mimo_brpoll2_flushed;
    /** 11AC VHT MU MIMO BRPOLL for user 3 frame flushed by HW */
    A_UINT32 ac_mu_mimo_brpoll3_flushed;
} htt_stats_tx_selfgen_ac_err_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_selfgen_ac_err_stats_tlv htt_tx_selfgen_ac_err_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** 11AX HE SU NDP frame completed with error(s) */
    A_UINT32 ax_su_ndp_err;
    /** 11AX HE SU NDPA frame completed with error(s) */
    A_UINT32 ax_su_ndpa_err;
    /** 11AX HE MU MIMO NDPA frame completed with error(s) */
    A_UINT32 ax_mu_mimo_ndpa_err;
    /** 11AX HE MU MIMO NDP frame completed with error(s) */
    A_UINT32 ax_mu_mimo_ndp_err;
    union {
        struct {
            /* deprecated old names */
            A_UINT32 ax_mu_mimo_brp1_err;
            A_UINT32 ax_mu_mimo_brp2_err;
            A_UINT32 ax_mu_mimo_brp3_err;
            A_UINT32 ax_mu_mimo_brp4_err;
            A_UINT32 ax_mu_mimo_brp5_err;
            A_UINT32 ax_mu_mimo_brp6_err;
            A_UINT32 ax_mu_mimo_brp7_err;
        };
        /** 11AX HE MU BR-POLL frame for 1 - 7 users completed with error(s) */
        A_UINT32 ax_mu_mimo_brp_err[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS - 1];
    };
    /** 11AX HE MU Basic Trigger frame completed with error(s) */
    A_UINT32 ax_basic_trigger_err;
    /** 11AX HE MU BSRP Trigger frame completed with error(s) */
    A_UINT32 ax_bsr_trigger_err;
    /** 11AX HE MU BAR Trigger frame completed with error(s) */
    A_UINT32 ax_mu_bar_trigger_err;
    /** 11AX HE MU RTS Trigger frame completed with error(s) */
    A_UINT32 ax_mu_rts_trigger_err;
    /** 11AX HE MU ULMUMIMO Trigger frame completed with error(s) */
    A_UINT32 ax_ulmumimo_trigger_err;
    /**
     * Number of CBF(s) received when 11AX HE MU MIMO BRPOLL
     * frame completed with error(s)
     */
    A_UINT32 ax_mu_mimo_brp_err_num_cbf_received[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS];
    /** 11AX HE SU NDPA frame flushed by HW */
    A_UINT32 ax_su_ndpa_flushed;
    /** 11AX HE SU NDP frame flushed by HW */
    A_UINT32 ax_su_ndp_flushed;
    /** 11AX HE MU MIMO NDPA frame flushed by HW */
    A_UINT32 ax_mu_mimo_ndpa_flushed;
    /** 11AX HE MU MIMO NDP frame flushed by HW */
    A_UINT32 ax_mu_mimo_ndp_flushed;
    /** 11AX HE MU BR-POLL frame for users 1 - 7 flushed by HW */
    A_UINT32 ax_mu_mimo_brpoll_flushed[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS - 1];
    /**
     * 11AX HE UL-MUMIMO Trigger frame for users 0 - 7 completed with error(s)
     */
    A_UINT32 ax_ul_mumimo_trigger_err[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS];

    /** 11AX HE MU OFDMA Basic Trigger frame completed with partial user response */
    A_UINT32 ax_basic_trigger_partial_resp;
    /** 11AX HE MU BSRP Trigger frame completed with partial user response */
    A_UINT32 ax_bsr_trigger_partial_resp;
    /** 11AX HE MU BAR Trigger frame completed with partial user response */
    A_UINT32 ax_mu_bar_trigger_partial_resp;
} htt_stats_tx_selfgen_ax_err_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_selfgen_ax_err_stats_tlv htt_tx_selfgen_ax_err_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** 11BE EHT SU NDP frame completed with error(s) */
    A_UINT32 be_su_ndp_err;
    /** 11BE EHT SU NDPA frame completed with error(s) */
    A_UINT32 be_su_ndpa_err;
    /** 11BE EHT MU MIMO NDPA frame completed with error(s) */
    A_UINT32 be_mu_mimo_ndpa_err;
    /** 11BE EHT MU MIMO NDP frame completed with error(s) */
    A_UINT32 be_mu_mimo_ndp_err;
    /** 11BE EHT MU BR-POLL frame for 1 - 7 users completed with error(s) */
    A_UINT32 be_mu_mimo_brp_err[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS - 1];
    /** 11BE EHT MU Basic Trigger frame completed with error(s) */
    A_UINT32 be_basic_trigger_err;
    /** 11BE EHT MU BSRP Trigger frame completed with error(s) */
    A_UINT32 be_bsr_trigger_err;
    /** 11BE EHT MU BAR Trigger frame completed with error(s) */
    A_UINT32 be_mu_bar_trigger_err;
    /** 11BE EHT MU RTS Trigger frame completed with error(s) */
    A_UINT32 be_mu_rts_trigger_err;
    /** 11BE EHT MU ULMUMIMO Trigger frame completed with error(s) */
    A_UINT32 be_ulmumimo_trigger_err;
    /**
     * Number of CBF(s) received when 11BE EHT MU MIMO BRPOLL frame
     * completed with error(s)
     */
    A_UINT32 be_mu_mimo_brp_err_num_cbf_received[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS];
    /** 11BE EHT SU NDPA frame flushed by HW */
    A_UINT32 be_su_ndpa_flushed;
    /** 11BE EHT SU NDP frame flushed by HW */
    A_UINT32 be_su_ndp_flushed;
    /** 11BE EHT MU MIMO NDPA frame flushed by HW */
    A_UINT32 be_mu_mimo_ndpa_flushed;
    /** 11BE HT MU MIMO NDP frame flushed by HW */
    A_UINT32 be_mu_mimo_ndp_flushed;
    /** 11BE EHT MU BR-POLL frame for users 1 - 7 flushed by HW */
    A_UINT32 be_mu_mimo_brpoll_flushed[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS - 1];
    /**
     * 11BE EHT UL-MUMIMO Trigger frame for users 0 - 7 completed with error(s)
     */
    A_UINT32 be_ul_mumimo_trigger_err[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS];

    /** 11BE EHT MU OFDMA Basic Trigger frame completed with partial user response */
    A_UINT32 be_basic_trigger_partial_resp;
    /** 11BE EHT MU BSRP Trigger frame completed with partial user response */
    A_UINT32 be_bsr_trigger_partial_resp;
    /** 11BE EHT MU BAR Trigger frame completed with partial user response */
    A_UINT32 be_mu_bar_trigger_partial_resp;
    /** 11BE EHT MU RTS Trigger frame blocked due to partner link TX/RX(eMLSR) */
    A_UINT32 be_mu_rts_trigger_blocked;
    /** 11BE EHT MU BSR Trigger frame blocked due to partner link TX/RX(eMLSR) */
    A_UINT32 be_bsr_trigger_blocked;
} htt_stats_tx_selfgen_be_err_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_selfgen_be_err_stats_tlv htt_tx_selfgen_be_err_stats_tlv;

/*
 * Scheduler completion status reason code.
 * (0) HTT_TXERR_NONE - No error (Success).
 * (1) HTT_TXERR_RESP - Response timeout, response mismatch, BW mismatch,
 *     MIMO control mismatch, CRC error etc.
 * (2) HTT_TXERR_FILT - Blocked by HW tx filtering.
 * (3) HTT_TXERR_FIFO - FIFO, misc. errors in HW.
 * (4) HTT_TXERR_SWABORT - Software initialted abort (TX_ABORT).
 * (5) HTT_TXERR_RESERVED1 - Currently reserved.
 * (6) HTT_TXERR_RESERVED2 - Currently reserved.
 */

/* Scheduler error code.
 * (0) HTT_TX_SELFGEN_SCH_TSFLAG_FLUSH_RCVD_ERR - Flush received from HW.
 * (1) HTT_TX_SELFGEN_SCH_TSFLAG_FILT_SCHED_CMD_ERR - Scheduler command was
 *     filtered by HW.
 * (2) HTT_TX_SELFGEN_SCH_TSFLAG_RESP_MISMATCH_ERR - Response frame mismatch
 *     error.
 * (3) HTT_TX_SELFGEN_SCH_TSFLAG_RESP_CBF_MIMO_CTRL_MISMATCH_ERR - CBF
 *     received with MIMO control mismatch.
 * (4) HTT_TX_SELFGEN_SCH_TSFLAG_RESP_CBF_BW_MISMATCH_ERR - CBF received with
 *     BW mismatch.
 * (5) HTT_TX_SELFGEN_SCH_TSFLAG_RETRY_COUNT_FAIL_ERR - Error in transmitting
 *     frame even after maximum retries.
 * (6) HTT_TX_SELFGEN_SCH_TSFLAG_RESP_TOO_LATE_RECEIVED_ERR - Response frame
 *     received outside RX window.
 * (7) HTT_TX_SELFGEN_SCH_TSFLAG_SIFS_STALL_NO_NEXT_CMD_ERR - No frame
 *     received by HW for queuing within SIFS interval.
 */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** 11AC VHT SU NDPA scheduler completion status reason code */
    A_UINT32 ac_su_ndpa_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AC VHT SU NDP scheduler completion status reason code */
    A_UINT32 ac_su_ndp_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AC VHT SU NDP scheduler error code */
    A_UINT32 ac_su_ndp_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /** 11AC VHT MU MIMO NDPA scheduler completion status reason code */
    A_UINT32 ac_mu_mimo_ndpa_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AC VHT MU MIMO NDP scheduler completion status reason code */
    A_UINT32 ac_mu_mimo_ndp_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AC VHT MU MIMO NDP scheduler error code */
    A_UINT32 ac_mu_mimo_ndp_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /** 11AC VHT MU MIMO BRPOLL scheduler completion status reason code */
    A_UINT32 ac_mu_mimo_brp_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AC VHT MU MIMO BRPOLL scheduler error code */
    A_UINT32 ac_mu_mimo_brp_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
} htt_stats_tx_selfgen_ac_sched_status_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_selfgen_ac_sched_status_stats_tlv
    htt_tx_selfgen_ac_sched_status_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** 11AX HE SU NDPA scheduler completion status reason code */
    A_UINT32 ax_su_ndpa_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AX SU NDP scheduler completion status reason code */
    A_UINT32 ax_su_ndp_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AX HE SU NDP scheduler error code */
    A_UINT32 ax_su_ndp_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /** 11AX HE MU MIMO NDPA scheduler completion status reason code */
    A_UINT32 ax_mu_mimo_ndpa_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AX HE MU MIMO NDP scheduler completion status reason code */
    A_UINT32 ax_mu_mimo_ndp_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AX HE MU MIMO NDP scheduler error code */
    A_UINT32 ax_mu_mimo_ndp_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /** 11AX HE MU MIMO MU BRPOLL scheduler completion status reason code */
    A_UINT32 ax_mu_brp_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AX HE MU MIMO MU BRPOLL scheduler error code */
    A_UINT32 ax_mu_brp_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /** 11AX HE MU BAR scheduler completion status reason code */
    A_UINT32 ax_mu_bar_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AX HE MU BAR scheduler error code */
    A_UINT32 ax_mu_bar_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /**
     * 11AX HE UL OFDMA Basic Trigger scheduler completion status reason code
     */
    A_UINT32 ax_basic_trig_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AX HE UL OFDMA Basic Trigger scheduler error code */
    A_UINT32 ax_basic_trig_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /**
     * 11AX HE UL MUMIMO Basic Trigger scheduler completion status reason code
     */
    A_UINT32 ax_ulmumimo_trig_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11AX HE UL MUMIMO Basic Trigger scheduler error code */
    A_UINT32 ax_ulmumimo_trig_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
} htt_stats_tx_selfgen_ax_sched_status_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_selfgen_ax_sched_status_stats_tlv
    htt_tx_selfgen_ax_sched_status_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** 11BE EHT SU NDPA scheduler completion status reason code */
    A_UINT32 be_su_ndpa_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11BE SU NDP scheduler completion status reason code */
    A_UINT32 be_su_ndp_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11BE EHT SU NDP scheduler error code */
    A_UINT32 be_su_ndp_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /** 11BE EHT MU MIMO NDPA scheduler completion status reason code */
    A_UINT32 be_mu_mimo_ndpa_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11BE EHT MU MIMO NDP scheduler completion status reason code */
    A_UINT32 be_mu_mimo_ndp_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11BE EHT MU MIMO NDP scheduler error code */
    A_UINT32 be_mu_mimo_ndp_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /** 11BE EHT MU MIMO MU BRPOLL scheduler completion status reason code */
    A_UINT32 be_mu_brp_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11BE EHT MU MIMO MU BRPOLL scheduler error code */
    A_UINT32 be_mu_brp_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /** 11BE EHT MU BAR scheduler completion status reason code */
    A_UINT32 be_mu_bar_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11BE EHT MU BAR scheduler error code */
    A_UINT32 be_mu_bar_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /**
     * 11BE EHT UL OFDMA Basic Trigger scheduler completion status reason code
     */
    A_UINT32 be_basic_trig_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11BE EHT UL OFDMA Basic Trigger scheduler error code */
    A_UINT32 be_basic_trig_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
    /**
     * 11BE EHT UL MUMIMO Basic Trigger scheduler completion status reason code
     */
    A_UINT32 be_ulmumimo_trig_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    /** 11BE EHT UL MUMIMO Basic Trigger scheduler error code */
    A_UINT32 be_ulmumimo_trig_sch_flag_err[HTT_TX_SELFGEN_NUM_SCH_TSFLAG_ERROR_STATS];
} htt_stats_tx_selfgen_be_sched_status_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_selfgen_be_sched_status_stats_tlv
    htt_tx_selfgen_be_sched_status_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_TX_SELFGEN_INFO
 * TLV_TAGS:
 *      - HTT_STATS_TX_SELFGEN_CMN_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_AC_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_AX_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_AC_ERR_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_AX_ERR_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_AC_SCHED_STATUS_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_AX_SCHED_STATUS_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_BE_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_BE_ERR_STATS_TAG
 *      - HTT_STATS_TX_SELFGEN_BE_SCHED_STATUS_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_tx_selfgen_cmn_stats_tlv cmn_tlv;
    htt_stats_tx_selfgen_ac_stats_tlv ac_tlv;
    htt_stats_tx_selfgen_ax_stats_tlv ax_tlv;
    htt_stats_tx_selfgen_ac_err_stats_tlv ac_err_tlv;
    htt_stats_tx_selfgen_ax_err_stats_tlv ax_err_tlv;
    htt_stats_tx_selfgen_ac_sched_status_stats_tlv ac_sched_status_tlv;
    htt_stats_tx_selfgen_ax_sched_status_stats_tlv ax_sched_status_tlv;
    htt_stats_tx_selfgen_be_stats_tlv be_tlv;
    htt_stats_tx_selfgen_be_err_stats_tlv be_err_tlv;
    htt_stats_tx_selfgen_be_sched_status_stats_tlv be_sched_status_tlv;
} htt_tx_pdev_selfgen_stats_t;
#endif /* ATH_TARGET */

/* == TX MU STATS == */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** Number of MU MIMO schedules posted to HW */
    A_UINT32 mu_mimo_sch_posted;
    /** Number of MU MIMO schedules failed to post */
    A_UINT32 mu_mimo_sch_failed;
    /** Number of MU MIMO PPDUs posted to HW */
    A_UINT32 mu_mimo_ppdu_posted;
    /*
     * This is the common description for the below sch stats.
     * Counts the number of transmissions of each number of MU users
     * in each TX mode.
     * The array index is the "number of users - 1".
     * For example, ac_mu_mimo_sch_nusers[1] counts the number of 11AC MU2
     * TX PPDUs, ac_mu_mimo_sch_nusers[2] counts the number of 11AC MU3
     * TX PPDUs and so on.
     * The same is applicable for the other TX mode stats.
     */
    /** Represents the count for 11AC DL MU MIMO sequences */
    A_UINT32 ac_mu_mimo_sch_nusers[HTT_TX_PDEV_STATS_NUM_AC_MUMIMO_USER_STATS];
    /** Represents the count for 11AX DL MU MIMO sequences */
    A_UINT32 ax_mu_mimo_sch_nusers[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS];
    /** Represents the count for 11AX DL MU OFDMA sequences */
    A_UINT32 ax_ofdma_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /**
     * Represents the count for 11AX UL MU OFDMA sequences with Basic Triggers
     */
    A_UINT32 ax_ul_ofdma_basic_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** Represents the count for 11AX UL MU OFDMA sequences with BSRP Triggers */
    A_UINT32 ax_ul_ofdma_bsr_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** Represents the count for 11AX UL MU OFDMA sequences with BAR Triggers */
    A_UINT32 ax_ul_ofdma_bar_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /** Represents the count for 11AX UL MU OFDMA sequences with BRP Triggers */
    A_UINT32 ax_ul_ofdma_brp_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /**
     * Represents the count for 11AX UL MU MIMO sequences with Basic Triggers
     */
    A_UINT32 ax_ul_mumimo_basic_sch_nusers[HTT_TX_PDEV_STATS_NUM_UL_MUMIMO_USER_STATS];
    /** Represents the count for 11AX UL MU MIMO sequences with BRP Triggers */
    A_UINT32 ax_ul_mumimo_brp_sch_nusers[HTT_TX_PDEV_STATS_NUM_UL_MUMIMO_USER_STATS];
    /** Number of 11AC DL MU MIMO schedules posted per group size (0-3) */
    A_UINT32 ac_mu_mimo_sch_posted_per_grp_sz[HTT_TX_PDEV_STATS_NUM_AC_MUMIMO_USER_STATS];
    /** Number of 11AX DL MU MIMO schedules posted per group size */
    A_UINT32 ax_mu_mimo_sch_posted_per_grp_sz[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS];
    /** Represents the count for 11BE DL MU MIMO sequences */
    A_UINT32 be_mu_mimo_sch_nusers[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS];
    /** Number of 11BE DL MU MIMO schedules posted per group size */
    A_UINT32 be_mu_mimo_sch_posted_per_grp_sz[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS];
    /** Number of 11AC DL MU MIMO schedules posted per group size (4-7) */
    A_UINT32 ac_mu_mimo_sch_posted_per_grp_sz_ext[HTT_TX_PDEV_STATS_NUM_AC_MUMIMO_USER_STATS];
} htt_stats_tx_pdev_mu_mimo_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_mu_mimo_stats_tlv htt_tx_pdev_mu_mimo_sch_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 dl_mumimo_grp_best_grp_size[HTT_STATS_MAX_MUMIMO_GRP_SZ];

    A_UINT32 dl_mumimo_grp_best_num_usrs[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS];

    A_UINT32 dl_mumimo_grp_eligible[HTT_STATS_MAX_MUMIMO_GRP_SZ];

    A_UINT32 dl_mumimo_grp_ineligible[HTT_STATS_MAX_MUMIMO_GRP_SZ];

    A_UINT32 dl_mumimo_grp_invalid[HTT_TX_NUM_MUMIMO_GRP_INVALID_WORDS];

    A_UINT32 dl_mumimo_grp_tputs[HTT_STATS_MUMIMO_TPUT_NUM_BINS];

    A_UINT32 ul_mumimo_grp_best_grp_size[HTT_STATS_MAX_MUMIMO_GRP_SZ];

    A_UINT32 ul_mumimo_grp_best_num_usrs[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS];

    A_UINT32 ul_mumimo_grp_tputs[HTT_STATS_MUMIMO_TPUT_NUM_BINS];
} htt_stats_tx_pdev_mumimo_grp_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_mumimo_grp_stats_tlv htt_tx_pdev_mumimo_grp_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** Number of MU MIMO schedules posted to HW */
    A_UINT32 mu_mimo_sch_posted;
    /** Number of MU MIMO schedules failed to post */
    A_UINT32 mu_mimo_sch_failed;
    /** Number of MU MIMO PPDUs posted to HW */
    A_UINT32 mu_mimo_ppdu_posted;
    /*
     * This is the common description for the below sch stats.
     * Counts the number of transmissions of each number of MU users
     * in each TX mode.
     * The array index is the "number of users - 1".
     * For example, ac_mu_mimo_sch_nusers[1] counts the number of 11AC MU2
     * TX PPDUs, ac_mu_mimo_sch_nusers[2] counts the number of 11AC MU3
     * TX PPDUs and so on.
     * The same is applicable for the other TX mode stats.
     */
    /** Represents the count for 11AC DL MU MIMO sequences */
    A_UINT32 ac_mu_mimo_sch_nusers[HTT_TX_PDEV_STATS_NUM_AC_MUMIMO_USER_STATS];
    /** Represents the count for 11AX DL MU MIMO sequences */
    A_UINT32 ax_mu_mimo_sch_nusers[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS];
    /** Number of 11AC DL MU MIMO schedules posted per group size (0-3) */
    A_UINT32 ac_mu_mimo_sch_posted_per_grp_sz[HTT_TX_PDEV_STATS_NUM_AC_MUMIMO_USER_STATS];
    /** Number of 11AX DL MU MIMO schedules posted per group size */
    A_UINT32 ax_mu_mimo_sch_posted_per_grp_sz[HTT_TX_PDEV_STATS_NUM_AX_MUMIMO_USER_STATS];
    /** Represents the count for 11BE DL MU MIMO sequences */
    A_UINT32 be_mu_mimo_sch_nusers[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS];
    /** Number of 11BE DL MU MIMO schedules posted per group size */
    A_UINT32 be_mu_mimo_sch_posted_per_grp_sz[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS];
    /** Number of 11AC DL MU MIMO schedules posted per group size (4 - 7)*/
    A_UINT32 ac_mu_mimo_sch_posted_per_grp_sz_ext[HTT_TX_PDEV_STATS_NUM_AC_MUMIMO_USER_STATS];
} htt_stats_tx_pdev_dl_mu_mimo_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_dl_mu_mimo_stats_tlv
    htt_tx_pdev_dl_mu_mimo_sch_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** Represents the count for 11AX DL MU OFDMA sequences */
    A_UINT32 ax_mu_ofdma_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
} htt_stats_tx_pdev_dl_mu_ofdma_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_dl_mu_ofdma_stats_tlv
    htt_tx_pdev_dl_mu_ofdma_sch_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** Represents the count for 11BE DL MU OFDMA sequences */
    A_UINT32 be_mu_ofdma_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
} htt_stats_tx_pdev_be_dl_mu_ofdma_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_be_dl_mu_ofdma_stats_tlv
    htt_tx_pdev_be_dl_mu_ofdma_sch_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * Represents the count for 11AX UL MU OFDMA sequences with Basic Triggers
     */
    A_UINT32 ax_ul_mu_ofdma_basic_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /**
     * Represents the count for 11AX UL MU OFDMA sequences with BSRP Triggers
     */
    A_UINT32 ax_ul_mu_ofdma_bsr_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /**
     * Represents the count for 11AX UL MU OFDMA sequences with BAR Triggers
     */
    A_UINT32 ax_ul_mu_ofdma_bar_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /**
     * Represents the count for 11AX UL MU OFDMA sequences with BRP Triggers
     */
    A_UINT32 ax_ul_mu_ofdma_brp_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
} htt_stats_tx_pdev_ul_mu_ofdma_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_ul_mu_ofdma_stats_tlv
    htt_tx_pdev_ul_mu_ofdma_sch_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * Represents the count for 11BE UL MU OFDMA sequences with Basic Triggers
     */
    A_UINT32 be_ul_mu_ofdma_basic_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /**
     * Represents the count for 11BE UL MU OFDMA sequences with BSRP Triggers
     */
    A_UINT32 be_ul_mu_ofdma_bsr_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /**
     * Represents the count for 11BE UL MU OFDMA sequences with BAR Triggers
     */
    A_UINT32 be_ul_mu_ofdma_bar_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
    /**
     * Represents the count for 11BE UL MU OFDMA sequences with BRP Triggers
     */
    A_UINT32 be_ul_mu_ofdma_brp_sch_nusers[HTT_TX_PDEV_STATS_NUM_OFDMA_USER_STATS];
} htt_stats_tx_pdev_be_ul_mu_ofdma_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_be_ul_mu_ofdma_stats_tlv
    htt_tx_pdev_be_ul_mu_ofdma_sch_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * Represents the count for 11AX UL MU MIMO sequences with Basic Triggers
     */
    A_UINT32 ax_ul_mu_mimo_basic_sch_nusers[HTT_TX_PDEV_STATS_NUM_UL_MUMIMO_USER_STATS];
    /**
     * Represents the count for 11AX UL MU MIMO sequences with BRP Triggers
     */
    A_UINT32 ax_ul_mu_mimo_brp_sch_nusers[HTT_TX_PDEV_STATS_NUM_UL_MUMIMO_USER_STATS];
} htt_stats_tx_pdev_ul_mu_mimo_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_ul_mu_mimo_stats_tlv
    htt_tx_pdev_ul_mu_mimo_sch_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * Represents the count for 11BE UL MU MIMO sequences with Basic Triggers
     */
    A_UINT32 be_ul_mu_mimo_basic_sch_nusers[HTT_TX_PDEV_STATS_NUM_UL_MUMIMO_USER_STATS];
    /**
     * Represents the count for 11BE UL MU MIMO sequences with BRP Triggers
     */
    A_UINT32 be_ul_mu_mimo_brp_sch_nusers[HTT_TX_PDEV_STATS_NUM_UL_MUMIMO_USER_STATS];
} htt_stats_tx_pdev_be_ul_mu_mimo_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_be_ul_mu_mimo_stats_tlv
    htt_tx_pdev_be_ul_mu_mimo_sch_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** 11AC DL MU MIMO number of mpdus queued to HW, per user */
    A_UINT32 mu_mimo_mpdus_queued_usr;
    /** 11AC DL MU MIMO number of mpdus tried over the air, per user */
    A_UINT32 mu_mimo_mpdus_tried_usr;
    /** 11AC DL MU MIMO number of mpdus failed acknowledgement, per user */
    A_UINT32 mu_mimo_mpdus_failed_usr;
    /** 11AC DL MU MIMO number of mpdus re-queued to HW, per user */
    A_UINT32 mu_mimo_mpdus_requeued_usr;
    /** 11AC DL MU MIMO BA not received, per user */
    A_UINT32 mu_mimo_err_no_ba_usr;
    /** 11AC DL MU MIMO mpdu underrun encountered, per user */
    A_UINT32 mu_mimo_mpdu_underrun_usr;
    /** 11AC DL MU MIMO ampdu underrun encountered, per user */
    A_UINT32 mu_mimo_ampdu_underrun_usr;

    /** 11AX MU MIMO number of mpdus queued to HW, per user */
    A_UINT32 ax_mu_mimo_mpdus_queued_usr;
    /** 11AX MU MIMO number of mpdus tried over the air, per user */
    A_UINT32 ax_mu_mimo_mpdus_tried_usr;
    /** 11AX DL MU MIMO number of mpdus failed acknowledgement, per user */
    A_UINT32 ax_mu_mimo_mpdus_failed_usr;
    /** 11AX DL MU MIMO number of mpdus re-queued to HW, per user */
    A_UINT32 ax_mu_mimo_mpdus_requeued_usr;
    /** 11AX DL MU MIMO BA not received, per user */
    A_UINT32 ax_mu_mimo_err_no_ba_usr;
    /** 11AX DL MU MIMO mpdu underrun encountered, per user */
    A_UINT32 ax_mu_mimo_mpdu_underrun_usr;
    /** 11AX DL MU MIMO ampdu underrun encountered, per user */
    A_UINT32 ax_mu_mimo_ampdu_underrun_usr;

    /** 11AX MU OFDMA number of mpdus queued to HW, per user */
    A_UINT32 ax_ofdma_mpdus_queued_usr;
    /** 11AX MU OFDMA number of mpdus tried over the air, per user */
    A_UINT32 ax_ofdma_mpdus_tried_usr;
    /** 11AX MU OFDMA number of mpdus failed acknowledgement, per user */
    A_UINT32 ax_ofdma_mpdus_failed_usr;
    /** 11AX MU OFDMA number of mpdus re-queued to HW, per user */
    A_UINT32 ax_ofdma_mpdus_requeued_usr;
    /** 11AX MU OFDMA BA not received, per user */
    A_UINT32 ax_ofdma_err_no_ba_usr;
    /** 11AX MU OFDMA mpdu underrun encountered, per user */
    A_UINT32 ax_ofdma_mpdu_underrun_usr;
    /** 11AX MU OFDMA ampdu underrun encountered, per user */
    A_UINT32 ax_ofdma_ampdu_underrun_usr;
} htt_stats_tx_pdev_mumimo_mpdu_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_mumimo_mpdu_stats_tlv
    htt_tx_pdev_mu_mimo_mpdu_stats_tlv;

#define HTT_STATS_TX_SCHED_MODE_MU_MIMO_AC  1 /* SCHED_TX_MODE_MU_MIMO_AC */
#define HTT_STATS_TX_SCHED_MODE_MU_MIMO_AX  2 /* SCHED_TX_MODE_MU_MIMO_AX */
#define HTT_STATS_TX_SCHED_MODE_MU_OFDMA_AX 3 /* SCHED_TX_MODE_MU_OFDMA_AX */
#define HTT_STATS_TX_SCHED_MODE_MU_OFDMA_BE 4 /* SCHED_TX_MODE_MU_OFDMA_BE */
#define HTT_STATS_TX_SCHED_MODE_MU_MIMO_BE  5 /* SCHED_TX_MODE_MU_MIMO_BE */

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
    /** HTT_STATS_TX_SCHED_MODE_xxx */
    A_UINT32 tx_sched_mode;
} htt_stats_tx_pdev_mpdu_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_mpdu_stats_tlv htt_tx_pdev_mpdu_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_TX_MU
 * TLV_TAGS:
 *      - HTT_STATS_TX_PDEV_MU_MIMO_STATS_TAG (multiple)
 *      - HTT_STATS_TX_PDEV_MPDU_STATS_TAG (multiple)
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_tx_pdev_mu_mimo_stats_tlv mu_mimo_sch_stats_tlv[1]; /* WAL_TX_STATS_MAX_GROUP_SIZE */
    htt_stats_tx_pdev_dl_mu_mimo_stats_tlv dl_mu_mimo_sch_stats_tlv[1];
    htt_stats_tx_pdev_ul_mu_mimo_stats_tlv ul_mu_mimo_sch_stats_tlv[1];
    htt_stats_tx_pdev_dl_mu_ofdma_stats_tlv dl_mu_ofdma_sch_stats_tlv[1];
    htt_stats_tx_pdev_ul_mu_ofdma_stats_tlv ul_mu_ofdma_sch_stats_tlv[1];
    /*
     * Note that though mu_mimo_mpdu_stats_tlv is named MU-MIMO,
     * it can also hold MU-OFDMA stats.
     */
    htt_stats_tx_pdev_mpdu_stats_tlv mu_mimo_mpdu_stats_tlv[1]; /* WAL_TX_STATS_MAX_NUM_USERS */
    htt_stats_tx_pdev_mumimo_grp_stats_tlv mumimo_grp_stats_tlv;
} htt_tx_pdev_mu_mimo_stats_t;
#endif /* ATH_TARGET */

/* == TX SCHED STATS == */

#define HTT_SCHED_TXQ_CMD_POSTED_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** Scheduler command posted per tx_mode */
    A_UINT32 sched_cmd_posted[1/* length = num tx modes */];
} htt_stats_sched_txq_cmd_posted_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_sched_txq_cmd_posted_tlv htt_sched_txq_cmd_posted_tlv_v;

#define HTT_SCHED_TXQ_CMD_REAPED_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** Scheduler command reaped per tx_mode */
    A_UINT32 sched_cmd_reaped[1/* length = num tx modes */];
} htt_stats_sched_txq_cmd_reaped_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_sched_txq_cmd_reaped_tlv htt_sched_txq_cmd_reaped_tlv_v;

#define HTT_SCHED_TXQ_SCHED_ORDER_SU_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * sched_order_su contains the peer IDs of peers chosen in the last
     * NUM_SCHED_ORDER_LOG scheduler instances.
     * The array is circular; it's unspecified which array element corresponds
     * to the most recent scheduler invocation, and which corresponds to
     * the (NUM_SCHED_ORDER_LOG-1) most recent scheduler invocation.
     *
     * HTT_TX_PDEV_NUM_SCHED_ORDER_LOG
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, sched_order_su);
} htt_stats_sched_txq_sched_order_su_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_sched_txq_sched_order_su_tlv htt_sched_txq_sched_order_su_tlv_v;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 htt_stats_type;
} htt_stats_error_tlv_v;

typedef enum {
    HTT_SCHED_TID_SKIP_SCHED_MASK_DISABLED = 0, /* Skip the tid when WAL_TID_DISABLE_TX_SCHED_MASK is true                                       */
    HTT_SCHED_TID_SKIP_NOTIFY_MPDU,             /* Skip the tid's 2nd sched_cmd when 1st cmd is ongoing                                          */
    HTT_SCHED_TID_SKIP_MPDU_STATE_INVALID,      /* Skip the tid when MPDU state is invalid                                                       */
    HTT_SCHED_TID_SKIP_SCHED_DISABLED,          /* Skip the tid when scheduling is disabled for that tid                                         */
    HTT_SCHED_TID_SKIP_TQM_BYPASS_CMD_PENDING,  /* Skip the TQM bypass tid when it has pending sched_cmd                                         */
    HTT_SCHED_TID_SKIP_SECOND_SU_SCHEDULE,      /* Skip tid from 2nd SU schedule when any of the following flag is set
                                                   WAL_TX_TID(SEND_BAR | TQM_MPDU_STATE_VALID | SEND_QOS_NULL | TQM_NOTIFY_MPDU | SENDN_PENDING) */
    HTT_SCHED_TID_SKIP_CMD_SLOT_NOT_AVAIL,      /* Skip the tid when command slot is not available                                               */
    HTT_SCHED_TID_SKIP_NO_DATA,                 /* Skip tid without data                                                                         */
    HTT_SCHED_TID_SKIP_NO_ENQ = HTT_SCHED_TID_SKIP_NO_DATA, /* deprecated old name */
    HTT_SCHED_TID_SKIP_LOW_ENQ,                 /* Skip the tid when enqueue is low                                                              */
    HTT_SCHED_TID_SKIP_PAUSED,                  /* Skipping the paused tid(sendn-frames)                                                         */
    HTT_SCHED_TID_SKIP_UL_RESP,                 /* skip UL response tid                                                                          */
    HTT_SCHED_TID_SKIP_UL = HTT_SCHED_TID_SKIP_UL_RESP, /* deprecated old name */
    HTT_SCHED_TID_REMOVE_PAUSED,                /* Removing the paused tid when number of sendn frames is zero                                   */
    HTT_SCHED_TID_REMOVE_NO_ENQ,                /* Remove tid with zero queue depth                                                              */
    HTT_SCHED_TID_REMOVE_UL_RESP,               /* Remove tid UL response                                                                        */
    HTT_SCHED_TID_REMOVE_UL = HTT_SCHED_TID_REMOVE_UL_RESP, /* deprecated old name */
    HTT_SCHED_TID_QUERY,                        /* Moving to next user and adding tid in prepend list when qstats update is pending              */
    HTT_SCHED_TID_SU_ONLY,                      /* Tid is eligible and TX_SCHED_SU_ONLY is true                                                  */
    HTT_SCHED_TID_ELIGIBLE,                     /* Tid is eligible for scheduling                                                                */
    HTT_SCHED_TID_SKIP_EXCEPT_EAPOL,            /* skip tid except eapol                                                                         */
    HTT_SCHED_TID_SU_LOW_PRI_ONLY,              /* su low priority tid only                                                                      */
    HTT_SCHED_TID_SKIP_SOUND_IN_PROGRESS,       /* skip tid  sound in progress                                                                   */
    HTT_SCHED_TID_SKIP_NO_UL_DATA,              /* skip ul tid when no ul data                                                                   */
    HTT_SCHED_TID_REMOVE_UL_NOT_CAPABLE,        /* Remove tid that are not UL capable                                                            */
    HTT_SCHED_TID_UL_ELIGIBLE,                  /* Tid is eligible for UL scheduling                                                             */
    HTT_SCHED_TID_FALLBACK_TO_PREV_DECISION,    /* Fall back to previous decision                                                                */
    HTT_SCHED_TID_SKIP_PEER_ALREADY_IN_TXQ,     /* skip tid, peer is already available in the txq                                                */
    HTT_SCHED_TID_SKIP_DELAY_UL_SCHED,          /* skip tid delay UL schedule                                                                    */
    HTT_SCHED_TID_SKIP_PWR_SAVE_STATE_OFF,      /* Limit UL scheduling to primary link if not in power save state                                */
    HTT_SCHED_TID_SKIP_TWT_SUSPEND,             /* Skip UL trigger for certain cases ex TWT suspend                                              */
    HTT_SCHED_TID_SKIP_DISABLE_160MHZ_OFDMA,    /* Skip ul tid if peer supports 160MHZ                                                           */
    HTT_SCHED_TID_SKIP_ULMU_DISABLE_FROM_OMI,   /* Skip ul tid if sta send omi to indicate to disable UL mu data                                 */
    HTT_SCHED_TID_SKIP_UL_MAX_SCHED_CMD_EXCEEDED,/* skip ul tid if max sched cmd is exceeded                                                     */
    HTT_SCHED_TID_SKIP_UL_SMALL_QDEPTH,         /* Skip ul tid for small qdepth                                                                  */
    HTT_SCHED_TID_SKIP_UL_TWT_PAUSED,           /* Skip ul tid if twt txq is paused                                                              */
    HTT_SCHED_TID_SKIP_PEER_UL_RX_NOT_ACTIVE,   /* Skip ul tid if peer ul rx is not active                                                       */
    HTT_SCHED_TID_SKIP_NO_FORCE_TRIGGER,        /* Skip ul tid if there is no force triggers                                                     */
    HTT_SCHED_TID_SKIP_SMART_BASIC_TRIGGER,     /* Skip ul tid if smart basic trigger doesn't have enough data                                   */


    HTT_SCHED_INELIGIBILITY_MAX,
} htt_sched_txq_sched_ineligibility_tlv_enum;

#define HTT_SCHED_TXQ_SCHED_INELIGIBILITY_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * sched_ineligibility counts the number of occurrences of different
     * reasons for tid ineligibility during eligibility checks per txq
     * in scheduling
     *
     * Indexed by htt_sched_txq_sched_ineligibility_tlv_enum.
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, sched_ineligibility);
} htt_stats_sched_txq_sched_ineligibility_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_sched_txq_sched_ineligibility_tlv
    htt_sched_txq_sched_ineligibility_tlv_v;

typedef enum {
    HTT_SCHED_SUPERCYCLE_TRIGGER_NONE = 0,                 /* Supercycle not triggered */
    HTT_SCHED_SUPERCYCLE_TRIGGER_FORCED,                   /* forced supercycle trigger */
    HTT_SCHED_SUPERCYCLE_TRIGGER_LESS_NUM_TIDQ_ENTRIES,    /* Num tidq entries is less than max_client threshold */
    HTT_SCHED_SUPERCYCLE_TRIGGER_LESS_NUM_ACTIVE_TIDS,     /* Num active tids is less than max_client threshold */
    HTT_SCHED_SUPERCYCLE_TRIGGER_MAX_ITR_REACHED,          /* max sched iteration reached */
    HTT_SCHED_SUPERCYCLE_TRIGGER_DUR_THRESHOLD_REACHED,    /* duration threshold reached */
    HTT_SCHED_SUPERCYCLE_TRIGGER_TWT_TRIGGER,              /* TWT supercycle trigger */
    HTT_SCHED_SUPERCYCLE_TRIGGER_MAX,
} htt_sched_txq_supercycle_triggers_tlv_enum;

#define HTT_SCHED_TXQ_SUPERCYCLE_TRIGGERS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * supercycle_triggers[] is a histogram that counts the number of
     * occurrences of each different reason for a transmit scheduler
     * supercycle to be triggered.
     * The htt_sched_txq_supercycle_triggers_tlv_enum is used to index
     * supercycle_triggers[], e.g. supercycle_triggers[1] holds the number
     * of times a supercycle has been forced.
     * These supercycle trigger counts are not automatically reset, but
     * are reset upon request.
     */
    A_UINT32 supercycle_triggers[1/*HTT_SCHED_SUPERCYCLE_TRIGGER_MAX*/];
} htt_stats_sched_txq_supercycle_trigger_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_sched_txq_supercycle_trigger_tlv
    htt_sched_txq_supercycle_triggers_tlv_v;

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

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [15 :  8]   :- txq_id
     * BIT [31 : 16]   :- reserved
     */
    A_UINT32 mac_id__txq_id__word;
    /** Scheduler policy ised for this TxQ */
    A_UINT32 sched_policy;
    /** Timestamp of last scheduler command posted */
    A_UINT32 last_sched_cmd_posted_timestamp;
    /** Timestamp of last scheduler command completed */
    A_UINT32 last_sched_cmd_compl_timestamp;
    /** Num of Sched2TAC ring hit Low Water Mark condition */
    A_UINT32 sched_2_tac_lwm_count;
    /** Num of Sched2TAC ring full condition */
    A_UINT32 sched_2_tac_ring_full;
    /**
     * Num of scheduler command post failures that includes SU/MU-MIMO/MU-OFDMA
     * sequence type
     */
    A_UINT32 sched_cmd_post_failure;
    /** Num of active tids for this TxQ at current instance */
    A_UINT32 num_active_tids;
    /** Num of powersave schedules */
    A_UINT32 num_ps_schedules;
    /** Num of scheduler commands pending for this TxQ */
    A_UINT32 sched_cmds_pending;
    /** Num of tidq registration for this TxQ */
    A_UINT32 num_tid_register;
    /** Num of tidq de-registration for this TxQ */
    A_UINT32 num_tid_unregister;
    /** Num of iterations msduq stats was updated */
    A_UINT32 num_qstats_queried;
    /** qstats query update status */
    A_UINT32 qstats_update_pending;
    /** Timestamp of Last query stats made */
    A_UINT32 last_qstats_query_timestamp;
    /** Num of sched2tqm command queue full condition */
    A_UINT32 num_tqm_cmdq_full;
    /** Num of scheduler trigger from DE Module */
    A_UINT32 num_de_sched_algo_trigger;
    /** Num of scheduler trigger from RT Module */
    A_UINT32 num_rt_sched_algo_trigger;
    /** Num of scheduler trigger from TQM Module */
    A_UINT32 num_tqm_sched_algo_trigger;
    /** Num of schedules for notify frame */
    A_UINT32 notify_sched;
    /** Duration based sendn termination */
    A_UINT32 dur_based_sendn_term;
    /** scheduled via NOTIFY2 */
    A_UINT32 su_notify2_sched;
    /** schedule if queued packets are greater than avg MSDUs in PPDU */
    A_UINT32 su_optimal_queued_msdus_sched;
    /** schedule due to timeout */
    A_UINT32 su_delay_timeout_sched;
    /** delay if txtime is less than 500us */
    A_UINT32 su_min_txtime_sched_delay;
    /** scheduled via no delay */
    A_UINT32 su_no_delay;
    /** Num of supercycles for this TxQ */
    A_UINT32 num_supercycles;
    /** Num of subcycles with sort for this TxQ */
    A_UINT32 num_subcycles_with_sort;
    /** Num of subcycles without sort for this Txq */
    A_UINT32 num_subcycles_no_sort;
} htt_stats_tx_pdev_scheduler_txq_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_scheduler_txq_stats_tlv
    htt_tx_pdev_stats_sched_per_txq_tlv;

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

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /** Current timestamp */
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
 *     - HTT_STATS_SCHED_TXQ_SUPERCYCLE_TRIGGER_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_stats_tx_sched_cmn_tlv cmn_tlv;
    struct {
        htt_stats_tx_pdev_scheduler_txq_stats_tlv     txq_tlv;
        htt_stats_sched_txq_cmd_posted_tlv      cmd_posted_tlv;
        htt_stats_sched_txq_cmd_reaped_tlv      cmd_reaped_tlv;
        htt_stats_sched_txq_sched_order_su_tlv      sched_order_su_tlv;
        htt_stats_sched_txq_sched_ineligibility_tlv sched_ineligibility_tlv;
        htt_stats_sched_txq_supercycle_trigger_tlv  htt_sched_txq_sched_ineligibility_tlv_esched_supercycle_trigger_tlv;
    } txq[1];
} htt_stats_tx_sched_t;

/* == TQM STATS == */

#define HTT_TX_TQM_MAX_GEN_MPDU_END_REASON 17
#define HTT_TX_TQM_MAX_LIST_MPDU_END_REASON 16
#define HTT_TX_TQM_MAX_LIST_MPDU_CNT_HISTOGRAM_BINS 16

#define HTT_TX_TQM_GEN_MPDU_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* HTT_TX_TQM_MAX_GEN_MPDU_END_REASON */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, gen_mpdu_end_reason);
} htt_stats_tx_tqm_gen_mpdu_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_tqm_gen_mpdu_tlv htt_tx_tqm_gen_mpdu_stats_tlv_v;

#define HTT_TX_TQM_LIST_MPDU_STATS_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* HTT_TX_TQM_MAX_LIST_MPDU_END_REASON */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, list_mpdu_end_reason);
} htt_stats_tx_tqm_list_mpdu_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_tqm_list_mpdu_tlv htt_tx_tqm_list_mpdu_stats_tlv_v;

#define HTT_TX_TQM_LIST_MPDU_CNT_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* HTT_TX_TQM_MAX_LIST_MPDU_CNT_HISTOGRAM_BINS */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, list_mpdu_cnt_hist);
} htt_stats_tx_tqm_list_mpdu_cnt_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_tqm_list_mpdu_cnt_tlv htt_tx_tqm_list_mpdu_cnt_tlv_v;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 msdu_count;
    A_UINT32 mpdu_count;
    A_UINT32 remove_msdu;
    A_UINT32 remove_mpdu;
    A_UINT32 remove_msdu_ttl;
    A_UINT32 send_bar;
    A_UINT32 bar_sync;
    A_UINT32 notify_mpdu;
    A_UINT32 sync_cmd;
    A_UINT32 write_cmd;
    A_UINT32 hwsch_trigger;
    A_UINT32 ack_tlv_proc;
    A_UINT32 gen_mpdu_cmd;
    A_UINT32 gen_list_cmd;
    A_UINT32 remove_mpdu_cmd;
    A_UINT32 remove_mpdu_tried_cmd;
    A_UINT32 mpdu_queue_stats_cmd;
    A_UINT32 mpdu_head_info_cmd;
    A_UINT32 msdu_flow_stats_cmd;
    A_UINT32 remove_msdu_cmd;
    A_UINT32 remove_msdu_ttl_cmd;
    A_UINT32 flush_cache_cmd;
    A_UINT32 update_mpduq_cmd;
    A_UINT32 enqueue;
    A_UINT32 enqueue_notify;
    A_UINT32 notify_mpdu_at_head;
    A_UINT32 notify_mpdu_state_valid;
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
} htt_stats_tx_tqm_pdev_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_tqm_pdev_tlv htt_tx_tqm_pdev_stats_tlv_v;

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

    /**
     * BIT [ 7 :  0]   :- mac_id
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
    A_UINT32 tqm_active_tids;
    A_UINT32 tqm_inactive_tids;
    A_UINT32 tqm_active_msduq_flows;

    /* SAWF system delay reference timestamp updation related stats */
    A_UINT32 total_msduq_timestamp_updates;
    A_UINT32 total_msduq_timestamp_updates_by_get_mpdu_head_info_cmd;
    A_UINT32 total_msduq_timestamp_updates_by_empty_to_nonempty_status;
    A_UINT32 total_get_mpdu_head_info_cmds_by_sched_algo_la_query;
    A_UINT32 total_get_mpdu_head_info_cmds_by_tac;
    A_UINT32 total_gen_mpdu_cmds_by_sched_algo_la_query;
    A_UINT32 high_prio_q_not_empty;
} htt_stats_tx_tqm_cmn_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_tqm_cmn_tlv htt_tx_tqm_cmn_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* Error stats */
    A_UINT32 q_empty_failure;
    A_UINT32 q_not_empty_failure;
    A_UINT32 add_msdu_failure;

    /* TQM reset debug stats */
    A_UINT32 tqm_cache_ctl_err;
    A_UINT32 tqm_soft_reset;
    A_UINT32 tqm_reset_total_num_in_use_link_descs;
    A_UINT32 tqm_reset_worst_case_num_lost_link_descs;
    A_UINT32 tqm_reset_worst_case_num_lost_host_tx_bufs_count;
    A_UINT32 tqm_reset_num_in_use_link_descs_internal_tqm;
    A_UINT32 tqm_reset_num_in_use_link_descs_wbm_idle_link_ring;
    A_UINT32 tqm_reset_time_to_tqm_hang_delta_ms;
    A_UINT32 tqm_reset_recovery_time_ms;
    A_UINT32 tqm_reset_num_peers_hdl;
    A_UINT32 tqm_reset_cumm_dirty_hw_mpduq_proc_cnt;
    A_UINT32 tqm_reset_cumm_dirty_hw_msduq_proc;
    A_UINT32 tqm_reset_flush_cache_cmd_su_cnt;
    A_UINT32 tqm_reset_flush_cache_cmd_other_cnt;
    A_UINT32 tqm_reset_flush_cache_cmd_trig_type;
    A_UINT32 tqm_reset_flush_cache_cmd_trig_cfg;
    A_UINT32 tqm_reset_flush_cache_cmd_skip_cmd_status_null;
} htt_stats_tx_tqm_error_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_tqm_error_stats_tlv htt_tx_tqm_error_stats_tlv;

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
#ifdef ATH_TARGET
typedef struct {
    htt_stats_tx_tqm_cmn_tlv           cmn_tlv;
    htt_stats_tx_tqm_error_stats_tlv   err_tlv;
    htt_stats_tx_tqm_gen_mpdu_tlv      gen_mpdu_stats_tlv;
    htt_stats_tx_tqm_list_mpdu_tlv     list_mpdu_stats_tlv;
    htt_stats_tx_tqm_list_mpdu_cnt_tlv list_mpdu_cnt_tlv;
    htt_stats_tx_tqm_pdev_tlv          tqm_pdev_stats_tlv;
} htt_tx_tqm_pdev_stats_t;
#endif /* ATH_TARGET */

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

    /*
     * BIT [ 7 :  0]   :- mac_id
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
} htt_stats_tx_tqm_cmdq_status_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_tqm_cmdq_status_tlv htt_tx_tqm_cmdq_status_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_TQM_CMDQ
 * TLV_TAGS:
 *     - HTT_STATS_STRING_TAG
 *     - HTT_STATS_TX_TQM_CMDQ_STATUS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    struct {
        htt_stats_string_tlv             cmdq_str_tlv;
        htt_stats_tx_tqm_cmdq_status_tlv status_tlv;
    } q[1];
} htt_tx_tqm_cmdq_stats_t;
#endif /* ATH_TARGET */

/* == TX-DE STATS == */

/* Structures for tx de stats */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 m1_packets;
    A_UINT32 m2_packets;
    A_UINT32 m3_packets;
    A_UINT32 m4_packets;
    A_UINT32 g1_packets;
    A_UINT32 g2_packets;
    A_UINT32 rc4_packets;
    A_UINT32 eap_packets;
    A_UINT32 eapol_start_packets;
    A_UINT32 eapol_logoff_packets;
    A_UINT32 eapol_encap_asf_packets;
} htt_stats_tx_de_eapol_packets_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_de_eapol_packets_tlv htt_tx_de_eapol_packets_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 ap_bss_peer_not_found;
    A_UINT32 ap_bcast_mcast_no_peer;
    A_UINT32 sta_delete_in_progress;
    A_UINT32 ibss_no_bss_peer;
    A_UINT32 invaild_vdev_type;
    A_UINT32 invalid_ast_peer_entry;
    A_UINT32 peer_entry_invalid;
    A_UINT32 ethertype_not_ip;
    A_UINT32 eapol_lookup_failed;
    A_UINT32 qpeer_not_allow_data;
    A_UINT32 fse_tid_override;
    A_UINT32 ipv6_jumbogram_zero_length;
    A_UINT32 qos_to_non_qos_in_prog;
    A_UINT32 ap_bcast_mcast_eapol;
    A_UINT32 unicast_on_ap_bss_peer;
    A_UINT32 ap_vdev_invalid;
    A_UINT32 incomplete_llc;
    A_UINT32 eapol_duplicate_m3;
    A_UINT32 eapol_duplicate_m4;
} htt_stats_tx_de_classify_failed_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_de_classify_failed_tlv htt_tx_de_classify_failed_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 arp_packets;
    A_UINT32 igmp_packets;
    A_UINT32 dhcp_packets;
    A_UINT32 host_inspected;
    A_UINT32 htt_included;
    A_UINT32 htt_valid_mcs;
    A_UINT32 htt_valid_nss;
    A_UINT32 htt_valid_preamble_type;
    A_UINT32 htt_valid_chainmask;
    A_UINT32 htt_valid_guard_interval;
    A_UINT32 htt_valid_retries;
    A_UINT32 htt_valid_bw_info;
    A_UINT32 htt_valid_power;
    A_UINT32 htt_valid_key_flags;
    A_UINT32 htt_valid_no_encryption;
    A_UINT32 fse_entry_count;
    A_UINT32 fse_priority_be;
    A_UINT32 fse_priority_high;
    A_UINT32 fse_priority_low;
    A_UINT32 fse_traffic_ptrn_be;
    A_UINT32 fse_traffic_ptrn_over_sub;
    A_UINT32 fse_traffic_ptrn_bursty;
    A_UINT32 fse_traffic_ptrn_interactive;
    A_UINT32 fse_traffic_ptrn_periodic;
    A_UINT32 fse_hwqueue_alloc;
    A_UINT32 fse_hwqueue_created;
    A_UINT32 fse_hwqueue_send_to_host;
    A_UINT32 mcast_entry;
    A_UINT32 bcast_entry;
    A_UINT32 htt_update_peer_cache;
    A_UINT32 htt_learning_frame;
    A_UINT32 fse_invalid_peer;
    /**
     * mec_notify is HTT TX WBM multicast echo check notification
     * from firmware to host.  FW sends SA addresses to host for all
     * multicast/broadcast packets received on STA side.
     */
    A_UINT32 mec_notify;
    A_UINT32 arp_response;
    A_UINT32 arp_request;
} htt_stats_tx_de_classify_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_de_classify_stats_tlv htt_tx_de_classify_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 eok;
    A_UINT32 classify_done;
    A_UINT32 lookup_failed;
    A_UINT32 send_host_dhcp;
    A_UINT32 send_host_mcast;
    A_UINT32 send_host_unknown_dest;
    A_UINT32 send_host;
    A_UINT32 status_invalid;
} htt_stats_tx_de_classify_status_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_de_classify_status_tlv htt_tx_de_classify_status_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 enqueued_pkts;
    A_UINT32 to_tqm;
    A_UINT32 to_tqm_bypass;
} htt_stats_tx_de_enqueue_packets_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_de_enqueue_packets_tlv htt_tx_de_enqueue_packets_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 discarded_pkts;
    A_UINT32 local_frames;
    A_UINT32 is_ext_msdu;
} htt_stats_tx_de_enqueue_discard_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_de_enqueue_discard_tlv htt_tx_de_enqueue_discard_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 tcl_dummy_frame;
    A_UINT32 tqm_dummy_frame;
    A_UINT32 tqm_notify_frame;
    A_UINT32 fw2wbm_enq;
    A_UINT32 tqm_bypass_frame;
} htt_stats_tx_de_compl_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_de_compl_stats_tlv htt_tx_de_compl_stats_tlv;

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

    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, fw2wbm_ring_full_hist);
} htt_stats_tx_de_fw2wbm_ring_full_hist_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_de_fw2wbm_ring_full_hist_tlv
    htt_tx_de_fw2wbm_ring_full_hist_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * BIT [ 7 :  0]   :- mac_id
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
    A_UINT32 vdev_id_mismatch_cnt;
} htt_stats_tx_de_cmn_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_de_cmn_tlv htt_tx_de_cmn_stats_tlv;

#define HTT_STATS_RX_FW_RING_SIZE_NUM_ENTRIES(dword) ((dword >> 0)  & 0xffff)
#define HTT_STATS_RX_FW_RING_CURR_NUM_ENTRIES(dword) ((dword >> 16) & 0xffff)

/* Rx debug info for status rings */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * BIT [15 :  0] :- max possible number of entries in respective ring
     *                  (size of the ring in terms of entries)
     * BIT [16 : 31] :- current number of entries occupied in respective ring
     */
    A_UINT32 entry_status_sw2rxdma;
    A_UINT32 entry_status_rxdma2reo;
    A_UINT32 entry_status_reo2sw1;
    A_UINT32 entry_status_reo2sw4;
    A_UINT32 entry_status_refillringipa;
    A_UINT32 entry_status_refillringhost;
    /** datarate - Moving Average of Number of Entries */
    A_UINT32 datarate_refillringipa;
    A_UINT32 datarate_refillringhost;
    /**
     * refillringhost_backpress_hist and refillringipa_backpress_hist are
     * deprecated, and will be filled with 0x0 by the target.
     */
    A_UINT32 refillringhost_backpress_hist[3];
    A_UINT32 refillringipa_backpress_hist[3];
    /**
     * Number of times reo2sw4(IPA_DEST_RING) ring is back-pressured
     * in recent time periods
     * element 0: in last 0 to 250ms
     * element 1: 250ms to 500ms
     * element 2: above 500ms
     */
    A_UINT32 reo2sw4ringipa_backpress_hist[3];
} htt_stats_rx_ring_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_ring_stats_tlv htt_rx_fw_ring_stats_tlv_v;

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
#ifdef ATH_TARGET
typedef struct {
    htt_stats_tx_de_cmn_tlv                   cmn_tlv;
    htt_stats_tx_de_fw2wbm_ring_full_hist_tlv fw2wbm_hist_tlv;
    htt_stats_tx_de_eapol_packets_tlv         eapol_stats_tlv;
    htt_stats_tx_de_classify_stats_tlv        classify_stats_tlv;
    htt_stats_tx_de_classify_failed_tlv       classify_failed_tlv;
    htt_stats_tx_de_classify_status_tlv       classify_status_rlv;
    htt_stats_tx_de_enqueue_packets_tlv       enqueue_packets_tlv;
    htt_stats_tx_de_enqueue_discard_tlv       enqueue_discard_tlv;
    htt_stats_tx_de_compl_stats_tlv           comp_status_tlv;
} htt_tx_de_stats_t;
#endif /* ATH_TARGET */

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
    /** DWORD aligned base memory address of the ring */
    A_UINT32 base_addr;

    /** size of each ring element */
    A_UINT32 elem_size;

    /**
     * BIT [15 :  0]   :- num_elems
     * BIT [31 : 16]   :- prefetch_tail_idx
     */
    A_UINT32 num_elems__prefetch_tail_idx;
    /**
     * BIT [15 :  0]   :- head_idx
     * BIT [31 : 16]   :- tail_idx
     */
    A_UINT32 head_idx__tail_idx;
    /**
     * BIT [15 :  0]   :- shadow_head_idx
     * BIT [31 : 16]   :- shadow_tail_idx
     */
    A_UINT32 shadow_head_idx__shadow_tail_idx;
    A_UINT32 num_tail_incr;
    /**
     * BIT [15 :  0]   :- lwm_thresh
     * BIT [31 : 16]   :- hwm_thresh
     */
    A_UINT32 lwm_thresh__hwm_thresh;
    A_UINT32 overrun_hit_count;
    A_UINT32 underrun_hit_count;
    A_UINT32 prod_blockwait_count;
    A_UINT32 cons_blockwait_count;
    A_UINT32 low_wm_hit_count[HTT_STATS_LOW_WM_BINS];
    A_UINT32 high_wm_hit_count[HTT_STATS_HIGH_WM_BINS];
} htt_stats_ring_if_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_ring_if_tlv htt_ring_if_stats_tlv;

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

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    A_UINT32 num_records;
} htt_stats_ring_if_cmn_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_ring_if_cmn_tlv htt_ring_if_cmn_tlv;

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
#ifdef ATH_TARGET
typedef struct {
    htt_stats_ring_if_cmn_tlv cmn_tlv;
    /** Variable based on the Number of records. */
    struct {
        htt_stats_string_tlv  ring_str_tlv;
        htt_stats_ring_if_tlv ring_tlv;
    } r[1];
} htt_ring_if_stats_t;
#endif /* ATH_TARGET */

/* == SFM STATS == */

#define HTT_SFM_CLIENT_USER_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** Number of DWORDS used per user and per client */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, dwords_used_by_user_n);
} htt_stats_sfm_client_user_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_sfm_client_user_tlv htt_sfm_client_user_tlv_v;

typedef struct  {
    htt_tlv_hdr_t tlv_hdr;
    /** Client ID */
    A_UINT32 client_id;
    /** Minimum number of buffers */
    A_UINT32 buf_min;
    /** Maximum number of buffers */
    A_UINT32 buf_max;
    /** Number of Busy buffers */
    A_UINT32 buf_busy;
    /** Number of Allocated buffers */
    A_UINT32 buf_alloc;
    /** Number of Available/Usable buffers */
    A_UINT32 buf_avail;
    /** Number of users */
    A_UINT32 num_users;
} htt_stats_sfm_client_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_sfm_client_tlv htt_sfm_client_tlv;

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

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /**
     * Indicates the total number of 128 byte buffers in the CMEM
     * that are available for buffer sharing
     */
    A_UINT32 buf_total;
    /**
     * Indicates for certain client or all the clients there is no
     * dword saved in SFM, refer to SFM_R1_MEM_EMPTY
     */
    A_UINT32 mem_empty;
    /** DEALLOCATE_BUFFERS, refer to register SFM_R0_DEALLOCATE_BUFFERS */
    A_UINT32 deallocate_bufs;
    /** Number of Records */
    A_UINT32 num_records;
} htt_stats_sfm_cmn_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_sfm_cmn_tlv htt_sfm_cmn_tlv;

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
#ifdef ATH_TARGET
typedef struct {
    htt_stats_sfm_cmn_tlv cmn_tlv;
    /** Variable based on the Number of records. */
    struct {
        htt_stats_string_tlv          client_str_tlv;
        htt_stats_sfm_client_tlv      client_tlv;
        htt_stats_sfm_client_user_tlv user_tlv;
    } r[1];
} htt_sfm_stats_t;
#endif /* ATH_TARGET */

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

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [15 :  8]   :- ring_id
     * BIT [23 : 16]   :- arena 0 -SRING_HRAM, 1 - SRING_HCRAM, 2 - SRING_HW2HW.
     * BIT [24 : 24]   :- EP 0 -consumer, 1 - producer
     * BIT [31 : 25]   :- reserved
     */
    A_UINT32 mac_id__ring_id__arena__ep;
    /** DWORD aligned base memory address of the ring */
    A_UINT32 base_addr_lsb;
    A_UINT32 base_addr_msb;
    /** size of ring */
    A_UINT32 ring_size;
    /** size of each ring element */
    A_UINT32 elem_size;

    /** Ring status
     *
     * BIT [15 :  0]   :- num_avail_words
     * BIT [31 : 16]   :- num_valid_words
     */
    A_UINT32 num_avail_words__num_valid_words;

    /** Index of head and tail
     * BIT [15 :  0]   :- head_ptr
     * BIT [31 : 16]   :- tail_ptr
     */
    A_UINT32 head_ptr__tail_ptr;

    /** Empty or full counter of rings
     * BIT [15 :  0]   :- consumer_empty
     * BIT [31 : 16]   :- producer_full
     */
    A_UINT32 consumer_empty__producer_full;

    /** Prefetch status of consumer ring
     * BIT [15 :  0]   :- prefetch_count
     * BIT [31 : 16]   :- internal_tail_ptr
     */
    A_UINT32 prefetch_count__internal_tail_ptr;
} htt_stats_sring_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_sring_stats_tlv htt_sring_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      num_records;
} htt_stats_sring_cmn_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_sring_cmn_tlv htt_sring_cmn_tlv;

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
#ifdef ATH_TARGET
typedef struct {
    htt_stats_sring_cmn_tlv cmn_tlv;
    /** Variable based on the Number of records */
    struct {
        htt_stats_string_tlv sring_str_tlv;
        htt_stats_sring_stats_tlv sring_stats_tlv;
    } r[1];
} htt_sring_stats_t;
#endif /* ATH_TARGET */

/* == PDEV TX RATE CTRL STATS == */

#define HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS 12 /* 0-11 */
#define HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS 2 /* 12, 13 */
#define HTT_TX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS 2 /* 14, 15 */
#define HTT_TX_PDEV_STATS_NUM_GI_COUNTERS 4
#define HTT_TX_PDEV_STATS_NUM_DCM_COUNTERS 5
#define HTT_TX_PDEV_STATS_NUM_BW_COUNTERS 4
#define HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS 8
#define HTT_TX_PDEV_STATS_NUM_PREAMBLE_TYPES HTT_STATS_PREAM_COUNT
#define HTT_TX_PDEV_STATS_NUM_LEGACY_CCK_STATS 4
#define HTT_TX_PDEV_STATS_NUM_LEGACY_OFDM_STATS 8
#define HTT_TX_PDEV_STATS_NUM_LTF 4
#define HTT_TX_PDEV_STATS_NUM_11AX_TRIGGER_TYPES 6
#define HTT_TX_PDEV_STATS_NUM_11BE_TRIGGER_TYPES 6
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

#define HTT_TX_PDEV_STATS_NUM_MCS_DROP_COUNTERS \
    (HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS + \
     HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS + \
     HTT_TX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS)

#define HTT_TX_PDEV_STATS_NUM_PER_COUNTERS 101

/*
 * Introduce new TX counters to support 320MHz support and punctured modes
 */
typedef enum {
    HTT_TX_PDEV_STATS_PUNCTURED_NONE = 0,
    HTT_TX_PDEV_STATS_PUNCTURED_20 = 1,
    HTT_TX_PDEV_STATS_PUNCTURED_40 = 2,
    HTT_TX_PDEV_STATS_PUNCTURED_80 = 3,
    HTT_TX_PDEV_STATS_PUNCTURED_120 = 4,
    HTT_TX_PDEV_STATS_NUM_PUNCTURED_MODE_COUNTERS = 5
} HTT_TX_PDEV_STATS_NUM_PUNCTURED_MODE_TYPE;

#define HTT_TX_PDEV_STATS_NUM_REDUCED_CHAN_TYPES 2 /* 0 - Half, 1 - Quarter */
/* 11be related updates */
#define HTT_TX_PDEV_STATS_NUM_BE_MCS_COUNTERS 16 /* 0...13,-2,-1 */
#define HTT_TX_PDEV_STATS_NUM_BE_BW_COUNTERS  5  /* 20,40,80,160,320 MHz */

#define HTT_TX_PDEV_STATS_NUM_HE_SIG_B_MCS_COUNTERS 6
#define HTT_TX_PDEV_STATS_NUM_EHT_SIG_MCS_COUNTERS 4

typedef enum {
    HTT_TX_PDEV_STATS_AX_RU_SIZE_26,
    HTT_TX_PDEV_STATS_AX_RU_SIZE_52,
    HTT_TX_PDEV_STATS_AX_RU_SIZE_106,
    HTT_TX_PDEV_STATS_AX_RU_SIZE_242,
    HTT_TX_PDEV_STATS_AX_RU_SIZE_484,
    HTT_TX_PDEV_STATS_AX_RU_SIZE_996,
    HTT_TX_PDEV_STATS_AX_RU_SIZE_996x2,
    HTT_TX_PDEV_STATS_NUM_AX_RU_SIZE_COUNTERS,
} HTT_TX_PDEV_STATS_AX_RU_SIZE;

typedef enum {
    HTT_TX_PDEV_STATS_BE_RU_SIZE_26,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_52,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_52_26,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_106,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_106_26,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_242,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_484,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_484_242,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_996,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_996_484,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_996_484_242,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_996x2,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_996x2_484,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_996x3,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_996x3_484,
    HTT_TX_PDEV_STATS_BE_RU_SIZE_996x4,
    HTT_TX_PDEV_STATS_NUM_BE_RU_SIZE_COUNTERS,
} HTT_TX_PDEV_STATS_BE_RU_SIZE;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /** Number of tx ldpc packets */
    A_UINT32 tx_ldpc;
    /** Number of tx rts packets */
    A_UINT32 rts_cnt;
    /** RSSI value of last ack packet (units = dB above noise floor) */
    A_UINT32 ack_rssi;

    A_UINT32 tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];

    /** tx_xx_mcs: currently unused */
    A_UINT32 tx_su_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 tx_mu_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];

    /* element 0,1, ...7 -> NSS 1,2, ...8 */
    A_UINT32 tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    /* element 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160 and 80+80 MHz */
    A_UINT32 tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 tx_stbc[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 tx_pream[HTT_TX_PDEV_STATS_NUM_PREAMBLE_TYPES];

    /**
     * Counters to track number of tx packets in each GI
     * (400us, 800us, 1600us & 3200us) in each mcs (0-11)
     */
    A_UINT32 tx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];

    /** Counters to track packets in dcm mcs (MCS 0, 1, 3, 4) */
    A_UINT32 tx_dcm[HTT_TX_PDEV_STATS_NUM_DCM_COUNTERS];
    /** Number of CTS-acknowledged RTS packets */
    A_UINT32 rts_success;

    /**
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

    /** 11AC VHT DL MU MIMO LDPC count */
    A_UINT32 ac_mu_mimo_tx_ldpc;
    /** 11AX HE DL MU MIMO LDPC count */
    A_UINT32 ax_mu_mimo_tx_ldpc;
    /** 11AX HE DL MU OFDMA LDPC count */
    A_UINT32 ofdma_tx_ldpc;

    /**
     * Counters for 11ax HE LTF selection during TX.
     *
     * The index corresponds to:
     *
     * 0: unused, 1: 1x LTF, 2: 2x LTF, 3: 4x LTF
     */
    A_UINT32 tx_he_ltf[HTT_TX_PDEV_STATS_NUM_LTF];

    /** 11AC VHT DL MU MIMO TX MCS stats */
    A_UINT32 ac_mu_mimo_tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    /** 11AX HE DL MU MIMO TX MCS stats */
    A_UINT32 ax_mu_mimo_tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    /** 11AX HE DL MU OFDMA TX MCS stats */
    A_UINT32 ofdma_tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];

    /** 11AC VHT DL MU MIMO TX NSS stats (Indicates NSS for individual users) */
    A_UINT32 ac_mu_mimo_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    /** 11AX HE DL MU MIMO TX NSS stats (Indicates NSS for individual users) */
    A_UINT32 ax_mu_mimo_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    /** 11AX HE DL MU OFDMA TX NSS stats (Indicates NSS for individual users) */
    A_UINT32 ofdma_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];

    /** 11AC VHT DL MU MIMO TX BW stats */
    A_UINT32 ac_mu_mimo_tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    /** 11AX HE DL MU MIMO TX BW stats */
    A_UINT32 ax_mu_mimo_tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    /** 11AX HE DL MU OFDMA TX BW stats */
    A_UINT32 ofdma_tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];

    /** 11AC VHT DL MU MIMO TX guard interval stats */
    A_UINT32 ac_mu_mimo_tx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    /** 11AX HE DL MU MIMO TX guard interval stats */
    A_UINT32 ax_mu_mimo_tx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    /** 11AX HE DL MU OFDMA TX guard interval stats */
    A_UINT32 ofdma_tx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 trigger_type_11ax[HTT_TX_PDEV_STATS_NUM_11AX_TRIGGER_TYPES];
    A_UINT32 tx_11ax_su_ext;

    /* Stats for MCS 12/13 */
    A_UINT32 tx_mcs_ext[HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS];
    A_UINT32 tx_stbc_ext[HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS];
    A_UINT32 tx_gi_ext[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS];
    /** 11AX VHT DL MU MIMO extended TX MCS stats for MCS 12/13 */
    A_UINT32 ax_mu_mimo_tx_mcs_ext[HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS];
    /** 11AX VHT DL MU OFDMA extended TX MCS stats for MCS 12/13 */
    A_UINT32 ofdma_tx_mcs_ext[HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS];
    /** 11AX VHT DL MU MIMO extended TX guard interval stats for MCS 12/13 */
    A_UINT32 ax_mu_mimo_tx_gi_ext[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS];
    /** 11AX VHT DL MU OFDMA extended TX guard interval stats for MCS 12/13 */
    A_UINT32 ofdma_tx_gi_ext[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS];
    /* Stats for MCS 14/15 */
    A_UINT32 tx_mcs_ext_2[HTT_TX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS];
    A_UINT32 tx_bw_320mhz;
    A_UINT32 tx_gi_ext_2[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS];
    A_UINT32 tx_su_punctured_mode[HTT_TX_PDEV_STATS_NUM_PUNCTURED_MODE_COUNTERS];
    A_UINT32 reduced_tx_bw[HTT_TX_PDEV_STATS_NUM_REDUCED_CHAN_TYPES][HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    /** 11AC VHT DL MU MIMO TX BW stats at reduced channel config */
    A_UINT32 reduced_ac_mu_mimo_tx_bw[HTT_TX_PDEV_STATS_NUM_REDUCED_CHAN_TYPES][HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    /** 11AX HE DL MU MIMO TX BW stats at reduced channel config */
    A_UINT32 reduced_ax_mu_mimo_tx_bw[HTT_TX_PDEV_STATS_NUM_REDUCED_CHAN_TYPES][HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    /** 11AX HE DL MU OFDMA TX BW stats at reduced channel config */
    A_UINT32 reduced_ax_mu_ofdma_tx_bw[HTT_TX_PDEV_STATS_NUM_REDUCED_CHAN_TYPES][HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    /** 11AX HE DL MU OFDMA TX RU Size stats */
    A_UINT32 ofdma_tx_ru_size[HTT_TX_PDEV_STATS_NUM_AX_RU_SIZE_COUNTERS];
    /** 11AX HE DL MU OFDMA HE-SIG-B MCS stats */
    A_UINT32 ofdma_he_sig_b_mcs[HTT_TX_PDEV_STATS_NUM_HE_SIG_B_MCS_COUNTERS];
    /** 11AX HE SU data + embedded trigger PPDU success stats (stats for HETP ack success PPDU cnt) */
    A_UINT32 ax_su_embedded_trigger_data_ppdu;
    /** 11AX HE SU data + embedded trigger PPDU failure stats (stats for HETP ack failure PPDU cnt) */
    A_UINT32 ax_su_embedded_trigger_data_ppdu_err;
    /** sta side trigger stats */
    A_UINT32 trigger_type_11be[HTT_TX_PDEV_STATS_NUM_11BE_TRIGGER_TYPES];
    /** Stats for Extra EHT LTF */
    A_UINT32 extra_eht_ltf;
    /** Counter for Extra EHT LTFs in OFDMA sequences */
    A_UINT32 extra_eht_ltf_ofdma;
} htt_stats_tx_pdev_rate_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_rate_stats_tlv htt_tx_pdev_rate_stats_tlv;

typedef struct {
     /* 11be mode pdev rate stats; placed in a separate TLV to adhere to size restrictions */
    htt_tlv_hdr_t tlv_hdr;
    /** 11BE EHT DL MU MIMO TX MCS stats */
    A_UINT32 be_mu_mimo_tx_mcs[HTT_TX_PDEV_STATS_NUM_BE_MCS_COUNTERS];
    /** 11BE EHT DL MU MIMO TX NSS stats (Indicates NSS for individual users) */
    A_UINT32 be_mu_mimo_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    /** 11BE EHT DL MU MIMO TX BW stats */
    A_UINT32 be_mu_mimo_tx_bw[HTT_TX_PDEV_STATS_NUM_BE_BW_COUNTERS];
    /** 11BE EHT DL MU MIMO TX guard interval stats */
    A_UINT32 be_mu_mimo_tx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_BE_MCS_COUNTERS];
    /** 11BE DL MU MIMO LDPC count */
    A_UINT32 be_mu_mimo_tx_ldpc;
} htt_stats_tx_pdev_be_rate_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_be_rate_stats_tlv htt_tx_pdev_rate_stats_be_tlv;

typedef struct {
    /*
     * SAWF pdev rate stats;
     * placed in a separate TLV to adhere to size restrictions
     */
    htt_tlv_hdr_t tlv_hdr;

    /**
     * Counter incremented when MCS is dropped due to the successive retries
     * to a peer reaching the configured limit.
     */
    A_UINT32 rate_retry_mcs_drop_cnt;

    /**
     * histogram of MCS rate drop down, indexed by pre-drop MCS
     */
    A_UINT32 mcs_drop_rate[HTT_TX_PDEV_STATS_NUM_MCS_DROP_COUNTERS];

    /**
     * PPDU PER histogram - each PPDU has its PER computed,
     * and the bin corresponding to that PER percentage is incremented.
     */
    A_UINT32 per_histogram_cnt[HTT_TX_PDEV_STATS_NUM_PER_COUNTERS];

    /**
     * When the service class contains delay bound rate parameters which
     * indicate low latency and we enable latency-based RA params then
     * the low_latency_rate_count will be incremented.
     * This counts the number of peer-TIDs that have been categorized as
     * low-latency.
     */
    A_UINT32 low_latency_rate_cnt;

    /** Indicate how many times rate drop happened within SIFS burst */
    A_UINT32 su_burst_rate_drop_cnt;

    /** Indicates how many within SIFS burst failed to deliver any pkt */
    A_UINT32 su_burst_rate_drop_fail_cnt;
} htt_stats_tx_pdev_sawf_rate_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_sawf_rate_stats_tlv htt_tx_pdev_rate_stats_sawf_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;

    /** 11BE EHT DL MU OFDMA LDPC count */
    A_UINT32 be_ofdma_tx_ldpc;
    /** 11BE EHT DL MU OFDMA TX MCS stats */
    A_UINT32 be_ofdma_tx_mcs[HTT_TX_PDEV_STATS_NUM_BE_MCS_COUNTERS];
    /**
     * 11BE EHT DL MU OFDMA TX NSS stats (Indicates NSS for individual users)
     */
    A_UINT32 be_ofdma_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    /** 11BE EHT DL MU OFDMA TX BW stats */
    A_UINT32 be_ofdma_tx_bw[HTT_TX_PDEV_STATS_NUM_BE_BW_COUNTERS];
    /** 11BE EHT DL MU OFDMA TX guard interval stats */
    A_UINT32 be_ofdma_tx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_TX_PDEV_STATS_NUM_BE_MCS_COUNTERS];
    /** 11BE EHT DL MU OFDMA TX RU Size stats */
    A_UINT32 be_ofdma_tx_ru_size[HTT_TX_PDEV_STATS_NUM_BE_RU_SIZE_COUNTERS];
    /** 11BE EHT DL MU OFDMA EHT-SIG MCS stats */
    A_UINT32 be_ofdma_eht_sig_mcs[HTT_TX_PDEV_STATS_NUM_EHT_SIG_MCS_COUNTERS];
} htt_stats_tx_pdev_rate_stats_be_ofdma_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_rate_stats_be_ofdma_tlv
    htt_tx_pdev_rate_stats_be_ofdma_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** tx_ppdu_dur_hist:
     * Tx PPDU duration histogram, which holds the tx duration of PPDUs
     * under histogram bins of interval 250us
     */
    A_UINT32 tx_ppdu_dur_hist[HTT_PDEV_STATS_PPDU_DUR_HIST_BINS];
    A_UINT32 tx_success_time_us_low;
    A_UINT32 tx_success_time_us_high;
    A_UINT32 tx_fail_time_us_low;
    A_UINT32 tx_fail_time_us_high;
    A_UINT32 pdev_up_time_us_low;
    A_UINT32 pdev_up_time_us_high;
    /** tx_ofdma_ppdu_dur_hist:
     * Tx OFDMA PPDU duration histogram, which holds the tx duration of
     * OFDMA PPDUs under histogram bins of interval 250us
     */
    A_UINT32 tx_ofdma_ppdu_dur_hist[HTT_PDEV_STATS_PPDU_DUR_HIST_BINS];
} htt_stats_tx_pdev_ppdu_dur_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_pdev_ppdu_dur_tlv htt_tx_pdev_ppdu_dur_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_TX_RATE
 * TLV_TAGS:
 *      - HTT_STATS_TX_PDEV_RATE_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_tx_pdev_rate_stats_tlv rate_tlv;
    htt_stats_tx_pdev_be_rate_stats_tlv rate_be_tlv;
    htt_stats_tx_pdev_sawf_rate_stats_tlv rate_sawf_tlv;
    htt_stats_tx_pdev_ppdu_dur_tlv tx_ppdu_dur_tlv;
} htt_tx_pdev_rate_stats_t;
#endif /* ATH_TARGET */

/* == PDEV RX RATE CTRL STATS == */

#define HTT_RX_PDEV_STATS_NUM_LEGACY_CCK_STATS 4
#define HTT_RX_PDEV_STATS_NUM_LEGACY_OFDM_STATS 8
#define HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS 12 /* 0-11 */
#define HTT_RX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS 2 /* 12, 13 */
#define HTT_RX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS 2 /* 14, 15 */
#define HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS_EXT 14 /* 0-13 */
#define HTT_RX_PDEV_STATS_NUM_GI_COUNTERS 4
#define HTT_RX_PDEV_STATS_NUM_DCM_COUNTERS 5
#define HTT_RX_PDEV_STATS_NUM_BW_COUNTERS 4
#define HTT_RX_PDEV_STATS_TOTAL_BW_COUNTERS \
    (HTT_RX_PDEV_STATS_NUM_BW_EXT_COUNTERS + HTT_RX_PDEV_STATS_NUM_BW_COUNTERS)
#define HTT_RX_PDEV_STATS_NUM_BW_EXT2_COUNTERS 5 /* 20, 40, 80, 160, 320Mhz */
#define HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS 8
#define HTT_RX_PDEV_STATS_ULMUMIMO_NUM_SPATIAL_STREAMS 8
#define HTT_RX_PDEV_STATS_NUM_PREAMBLE_TYPES HTT_STATS_PREAM_COUNT
#define HTT_RX_PDEV_MAX_OFDMA_NUM_USER 8
#define HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER 8
#define HTT_RX_PDEV_STATS_RXEVM_MAX_PILOTS_PER_NSS 16
#define HTT_RX_PDEV_STATS_NUM_BE_MCS_COUNTERS 16 /* 0-13, -2, -1 */
#define HTT_RX_PDEV_STATS_NUM_BE_BW_COUNTERS  5  /* 20,40,80,160,320 MHz */

/* HTT_RX_PDEV_STATS_NUM_RU_SIZE_COUNTERS:
 * RU size index 0: HTT_UL_OFDMA_V0_RU_SIZE_RU_26
 * RU size index 1: HTT_UL_OFDMA_V0_RU_SIZE_RU_52
 * RU size index 2: HTT_UL_OFDMA_V0_RU_SIZE_RU_106
 * RU size index 3: HTT_UL_OFDMA_V0_RU_SIZE_RU_242
 * RU size index 4: HTT_UL_OFDMA_V0_RU_SIZE_RU_484
 * RU size index 5: HTT_UL_OFDMA_V0_RU_SIZE_RU_996
 */
#define HTT_RX_PDEV_STATS_NUM_RU_SIZE_COUNTERS 6
/* HTT_RX_PDEV_STATS_NUM_RU_SIZE_160MHZ_CNTRS:
 * RU size index 0: HTT_UL_OFDMA_V0_RU_SIZE_RU_26
 * RU size index 1: HTT_UL_OFDMA_V0_RU_SIZE_RU_52
 * RU size index 2: HTT_UL_OFDMA_V0_RU_SIZE_RU_106
 * RU size index 3: HTT_UL_OFDMA_V0_RU_SIZE_RU_242
 * RU size index 4: HTT_UL_OFDMA_V0_RU_SIZE_RU_484
 * RU size index 5: HTT_UL_OFDMA_V0_RU_SIZE_RU_996
 * RU size index 6: HTT_UL_OFDMA_V0_RU_SIZE_RU_996x2
 */
#define HTT_RX_PDEV_STATS_NUM_RU_SIZE_160MHZ_CNTRS 7 /* includes 996x2 */

typedef enum {
    HTT_RX_PDEV_STATS_BE_RU_SIZE_26,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_52,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_52_26,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_106,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_106_26,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_242,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_484,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_484_242,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_996,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_996_484,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_996_484_242,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_996x2,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_996x2_484,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_996x3,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_996x3_484,
    HTT_RX_PDEV_STATS_BE_RU_SIZE_996x4,
    HTT_RX_PDEV_STATS_NUM_BE_RU_SIZE_COUNTERS,
} HTT_RX_PDEV_STATS_BE_RU_SIZE;

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

/* Introduce new RX counters to support 320MHZ support and punctured modes */
typedef enum {
    HTT_RX_PDEV_STATS_PUNCTURED_NONE = 0,
    HTT_RX_PDEV_STATS_PUNCTURED_20 = 1,
    HTT_RX_PDEV_STATS_PUNCTURED_40 = 2,
    HTT_RX_PDEV_STATS_PUNCTURED_80 = 3,
    HTT_RX_PDEV_STATS_PUNCTURED_120 = 4,
    HTT_RX_PDEV_STATS_NUM_PUNCTURED_MODE_COUNTERS = 5
} HTT_RX_PDEV_STATS_NUM_PUNCTURED_MODE_TYPE;

#define HTT_RX_PDEV_STATS_NUM_REDUCED_CHAN_TYPES 2 /* 0 - Half, 1 - Quarter */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    A_UINT32 nsts;

    /** Number of rx ldpc packets */
    A_UINT32 rx_ldpc;
    /** Number of rx rts packets */
    A_UINT32 rts_cnt;

    /** units = dB above noise floor */
    A_UINT32 rssi_mgmt;
    /** units = dB above noise floor */
    A_UINT32 rssi_data;
    /** units = dB above noise floor */
    A_UINT32 rssi_comb;
    A_UINT32 rx_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    /** element 0,1, ...7 -> NSS 1,2, ...8 */
    A_UINT32 rx_nss[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 rx_dcm[HTT_RX_PDEV_STATS_NUM_DCM_COUNTERS];
    A_UINT32 rx_stbc[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    /** element 0: 20 MHz, 1: 40 MHz, 2: 80 MHz, 3: 160 and 80+80 MHz */
    A_UINT32 rx_bw[HTT_RX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 rx_pream[HTT_RX_PDEV_STATS_NUM_PREAMBLE_TYPES];
    /** units = dB above noise floor */
    A_UINT8  rssi_chain[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_STATS_NUM_BW_COUNTERS];

    /** Counters to track number of rx packets in each GI in each mcs (0-11) */
    A_UINT32 rx_gi[HTT_RX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    /** rx Signal Strength value in dBm unit */
    A_INT32  rssi_in_dbm;

    A_UINT32 rx_11ax_su_ext;
    A_UINT32 rx_11ac_mumimo;
    A_UINT32 rx_11ax_mumimo;
    A_UINT32 rx_11ax_ofdma;
    A_UINT32 txbf;
    A_UINT32 rx_legacy_cck_rate[HTT_RX_PDEV_STATS_NUM_LEGACY_CCK_STATS];
    A_UINT32 rx_legacy_ofdm_rate[HTT_RX_PDEV_STATS_NUM_LEGACY_OFDM_STATS];
    A_UINT32 rx_active_dur_us_low;
    A_UINT32 rx_active_dur_us_high;

    /** number of times UL MU MIMO RX packets received */
    A_UINT32 rx_11ax_ul_ofdma;

    /** 11AX HE UL OFDMA RX TB PPDU MCS stats */
    A_UINT32 ul_ofdma_rx_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    /** 11AX HE UL OFDMA RX TB PPDU GI stats */
    A_UINT32 ul_ofdma_rx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    /**
     * 11AX HE UL OFDMA RX TB PPDU NSS stats
     * (Increments the individual user NSS in the OFDMA PPDU received)
     */
    A_UINT32 ul_ofdma_rx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    /** 11AX HE UL OFDMA RX TB PPDU BW stats */
    A_UINT32 ul_ofdma_rx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    /** Number of times UL OFDMA TB PPDUs received with stbc */
    A_UINT32 ul_ofdma_rx_stbc;
    /** Number of times UL OFDMA TB PPDUs received with ldpc */
    A_UINT32 ul_ofdma_rx_ldpc;

    /**
     * Number of non data PPDUs received for each degree (number of users)
     * in UL OFDMA
     */
    A_UINT32 rx_ulofdma_non_data_ppdu[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];
    /**
     * Number of data ppdus received for each degree (number of users)
     * in UL OFDMA
     */
    A_UINT32 rx_ulofdma_data_ppdu[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];
    /**
     * Number of mpdus passed for each degree (number of users)
     * in UL OFDMA TB PPDU
     */
    A_UINT32 rx_ulofdma_mpdu_ok[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];
    /**
     * Number of mpdus failed for each degree (number of users)
     * in UL OFDMA TB PPDU
     */
    A_UINT32 rx_ulofdma_mpdu_fail[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];

    A_UINT32 nss_count;
    A_UINT32 pilot_count;
    /** RxEVM stats in dB */
    A_INT32 rx_pilot_evm_dB[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_STATS_RXEVM_MAX_PILOTS_PER_NSS];
    /**
     * EVM mean across pilots, computed as
     *     mean(10*log10(rx_pilot_evm_linear)) = mean(rx_pilot_evm_dB)
     */
    A_INT32 rx_pilot_evm_dB_mean[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    /** dBm units */
    A_INT8  rx_ul_fd_rssi[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_MAX_OFDMA_NUM_USER];
    /** per_chain_rssi_pkt_type:
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

    /**
     * Number of non data ppdus received for each degree (number of users)
     * with UL MUMIMO
     */
    A_UINT32 rx_ulmumimo_non_data_ppdu[HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER];
    /**
     * Number of data ppdus received for each degree (number of users)
     * with UL MUMIMO
     */
    A_UINT32 rx_ulmumimo_data_ppdu[HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER];
    /**
     * Number of mpdus passed for each degree (number of users)
     * with UL MUMIMO TB PPDU
     */
    A_UINT32 rx_ulmumimo_mpdu_ok[HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER];
    /**
     * Number of mpdus failed for each degree (number of users)
     * with UL MUMIMO TB PPDU
     */
    A_UINT32 rx_ulmumimo_mpdu_fail[HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER];
    /**
     * Number of non data ppdus received for each degree (number of users)
     * in UL OFDMA
     */
    A_UINT32 rx_ulofdma_non_data_nusers[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];
    /**
     *  Number of data ppdus received for each degree (number of users)
     *in UL OFDMA
     */
    A_UINT32 rx_ulofdma_data_nusers[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];

    /* Stats for MCS 12/13 */
    A_UINT32 rx_mcs_ext[HTT_RX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS];
/*
 * NOTE - this TLV is already large enough that it causes the HTT message
 * carrying it to be nearly at the message size limit that applies to
 * many targets/hosts.
 * No further fields should be added to this TLV without very careful
 * review to ensure the size increase is acceptable.
 */
} htt_stats_rx_pdev_rate_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_rate_stats_tlv htt_rx_pdev_rate_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** Tx PPDU duration histogram **/
    A_UINT32 rx_ppdu_dur_hist[HTT_PDEV_STATS_PPDU_DUR_HIST_BINS];
} htt_stats_rx_pdev_ppdu_dur_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_ppdu_dur_tlv htt_rx_pdev_ppdu_dur_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_RX_RATE
 * TLV_TAGS:
 *      - HTT_STATS_RX_PDEV_RATE_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_rx_pdev_rate_stats_tlv rate_tlv;
    htt_stats_rx_pdev_ppdu_dur_tlv rx_ppdu_dur_tlv;
} htt_rx_pdev_rate_stats_t;
#endif /* ATH_TARGET */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** units = dB above noise floor */
    A_UINT8  rssi_chain_ext[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_STATS_NUM_BW_EXT_COUNTERS];
    A_INT8   rx_per_chain_rssi_ext_in_dbm[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_STATS_NUM_BW_EXT_COUNTERS];
    /** rx mcast signal strength value in dBm unit */
    A_INT32  rssi_mcast_in_dbm;
    /** rx mgmt packet signal Strength value in dBm unit */
    A_INT32  rssi_mgmt_in_dbm;
    /*
     * Stats for MCS 0-13 since rx_pdev_rate_stats_tlv cannot be updated,
     * due to message size limitations.
     */
    A_UINT32 rx_mcs_ext[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS_EXT];
    A_UINT32 rx_stbc_ext[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS_EXT];
    A_UINT32 rx_gi_ext[HTT_RX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS_EXT];
    A_UINT32 ul_ofdma_rx_mcs_ext[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS_EXT];
    A_UINT32 ul_ofdma_rx_gi_ext[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS_EXT];
    A_UINT32 rx_11ax_su_txbf_mcs_ext[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS_EXT];
    A_UINT32 rx_11ax_mu_txbf_mcs_ext[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS_EXT];
    A_UINT32 rx_11ax_dl_ofdma_mcs_ext[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS_EXT];
    /* MCS 14,15 */
    A_UINT32 rx_mcs_ext_2[HTT_RX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS];
    A_UINT32 rx_bw_ext[HTT_RX_PDEV_STATS_NUM_BW_EXT2_COUNTERS];
    A_UINT32 rx_gi_ext_2[HTT_RX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS];
    A_UINT32 rx_su_punctured_mode[HTT_RX_PDEV_STATS_NUM_PUNCTURED_MODE_COUNTERS];
    A_UINT32 reduced_rx_bw[HTT_RX_PDEV_STATS_NUM_REDUCED_CHAN_TYPES][HTT_RX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT8  rssi_chain_ext_2[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_STATS_NUM_BW_EXT_2_COUNTERS]; /* units = dB above noise floor */
    A_INT8   rx_per_chain_rssi_ext_2_in_dbm[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_STATS_NUM_BW_EXT_2_COUNTERS];
} htt_stats_rx_pdev_rate_ext_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_rate_ext_stats_tlv htt_rx_pdev_rate_ext_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_RX_RATE_EXT
 * TLV_TAGS:
 *      - HTT_STATS_RX_PDEV_RATE_EXT_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_rx_pdev_rate_ext_stats_tlv rate_tlv;
} htt_rx_pdev_rate_ext_stats_t;
#endif /* ATH_TARGET */

#define HTT_STATS_CMN_MAC_ID_M 0x000000ff
#define HTT_STATS_CMN_MAC_ID_S 0

#define HTT_STATS_CMN_MAC_ID_GET(_var) \
    (((_var) & HTT_STATS_CMN_MAC_ID_M) >> \
     HTT_STATS_CMN_MAC_ID_S)

#define HTT_STATS_CMN_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_STATS_CMN_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_STATS_CMN_MAC_ID_S)); \
    } while (0)

#define HTT_RX_UL_MAX_UPLINK_RSSI_TRACK 5

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;

    A_UINT32 rx_11ax_ul_ofdma;

    A_UINT32 ul_ofdma_rx_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 ul_ofdma_rx_gi[HTT_RX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 ul_ofdma_rx_nss[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 ul_ofdma_rx_bw[HTT_RX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 ul_ofdma_rx_stbc;
    A_UINT32 ul_ofdma_rx_ldpc;

    /*
     * These are arrays to hold the number of PPDUs that we received per RU.
     * E.g. PPDUs (data or non data) received in RU26 will be incremented in
     * array offset 0 and similarly RU52 will be incremented in array offset 1
     */
    A_UINT32 rx_ulofdma_data_ru_size_ppdu[HTT_RX_PDEV_STATS_NUM_RU_SIZE_160MHZ_CNTRS];      /* ppdu level */
    A_UINT32 rx_ulofdma_non_data_ru_size_ppdu[HTT_RX_PDEV_STATS_NUM_RU_SIZE_160MHZ_CNTRS];  /* ppdu level */

    /*
     * These arrays hold Target RSSI (rx power the AP wants),
     * FD RSSI (rx power the AP sees) & Power headroom values of STAs
     * which can be identified by AIDs, during trigger based RX.
     * Array acts a circular buffer and holds values for last 5 STAs
     * in the same order as RX.
     */
    /**
     * STA AID array for identifying which STA the
     * Target-RSSI / FD-RSSI / pwr headroom stats are for
     */
    A_UINT32 uplink_sta_aid[HTT_RX_UL_MAX_UPLINK_RSSI_TRACK];
    /**
     * Trig Target RSSI for STA AID in same index - UNIT(dBm)
     */
    A_INT32 uplink_sta_target_rssi[HTT_RX_UL_MAX_UPLINK_RSSI_TRACK];
    /**
     * Trig FD RSSI from STA AID in same index - UNIT(dBm)
     */
    A_INT32 uplink_sta_fd_rssi[HTT_RX_UL_MAX_UPLINK_RSSI_TRACK];
    /**
     * Trig power headroom for STA AID in same idx - UNIT(dB)
     */
    A_UINT32 uplink_sta_power_headroom[HTT_RX_UL_MAX_UPLINK_RSSI_TRACK];
    A_UINT32 reduced_ul_ofdma_rx_bw[HTT_RX_PDEV_STATS_NUM_REDUCED_CHAN_TYPES][HTT_RX_PDEV_STATS_NUM_BW_COUNTERS];

    /*
     * Number of HE UL OFDMA per-user responses containing only a QoS null in
     * response to basic trigger. Typically a data response is expected.
     */
    A_UINT32 ul_ofdma_basic_trigger_rx_qos_null_only;
} htt_stats_rx_pdev_ul_trig_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_ul_trig_stats_tlv htt_rx_pdev_ul_trigger_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_UL_TRIG_STATS
 * TLV_TAGS:
 *      - HTT_STATS_RX_PDEV_UL_TRIG_STATS_TAG
 * NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_rx_pdev_ul_trig_stats_tlv ul_trigger_tlv;
} htt_rx_pdev_ul_trigger_stats_t;
#endif /* ATH_TARGET */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;

    A_UINT32 rx_11be_ul_ofdma;

    A_UINT32 be_ul_ofdma_rx_mcs[HTT_RX_PDEV_STATS_NUM_BE_MCS_COUNTERS];
    A_UINT32 be_ul_ofdma_rx_gi[HTT_RX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_BE_MCS_COUNTERS];
    A_UINT32 be_ul_ofdma_rx_nss[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 be_ul_ofdma_rx_bw[HTT_RX_PDEV_STATS_NUM_BE_BW_COUNTERS];
    A_UINT32 be_ul_ofdma_rx_stbc;
    A_UINT32 be_ul_ofdma_rx_ldpc;

    /*
     * These are arrays to hold the number of PPDUs that we received per RU.
     * E.g. PPDUs (data or non data) received in RU26 will be incremented in
     * array offset 0 and similarly RU52 will be incremented in array offset 1
     */
    /** PPDU level */
    A_UINT32 be_rx_ulofdma_data_ru_size_ppdu[HTT_RX_PDEV_STATS_NUM_BE_RU_SIZE_COUNTERS];
    /** PPDU level */
    A_UINT32 be_rx_ulofdma_non_data_ru_size_ppdu[HTT_RX_PDEV_STATS_NUM_BE_RU_SIZE_COUNTERS];

    /*
     * These arrays hold Target RSSI (rx power the AP wants),
     * FD RSSI (rx power the AP sees) & Power headroom values of STAs
     * which can be identified by AIDs, during trigger based RX.
     * Array acts a circular buffer and holds values for last 5 STAs
     * in the same order as RX.
     */
    /**
     * STA AID array for identifying which STA the
     * Target-RSSI / FD-RSSI / pwr headroom stats are for
     */
    A_UINT32 be_uplink_sta_aid[HTT_RX_UL_MAX_UPLINK_RSSI_TRACK];
    /**
     * Trig Target RSSI for STA AID in same index - UNIT(dBm)
     */
    A_INT32 be_uplink_sta_target_rssi[HTT_RX_UL_MAX_UPLINK_RSSI_TRACK];
    /**
     * Trig FD RSSI from STA AID in same index - UNIT(dBm)
     */
    A_INT32 be_uplink_sta_fd_rssi[HTT_RX_UL_MAX_UPLINK_RSSI_TRACK];
    /**
     * Trig power headroom for STA AID in same idx - UNIT(dB)
     */
    A_UINT32 be_uplink_sta_power_headroom[HTT_RX_UL_MAX_UPLINK_RSSI_TRACK];

    /*
     * Number of EHT UL OFDMA per-user responses containing only a QoS null in
     * response to basic trigger. Typically a data response is expected.
     */
    A_UINT32 be_ul_ofdma_basic_trigger_rx_qos_null_only;

    /* UL MLO Queue Depth Sharing Stats */
    A_UINT32 ul_mlo_send_qdepth_params_count;
    A_UINT32 ul_mlo_proc_qdepth_params_count;
    A_UINT32 ul_mlo_proc_accepted_qdepth_params_count;
    A_UINT32 ul_mlo_proc_discarded_qdepth_params_count;
} htt_stats_rx_pdev_be_ul_trig_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_be_ul_trig_stats_tlv
    htt_rx_pdev_be_ul_trigger_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_UL_TRIG_STATS
 * TLV_TAGS:
 *      - HTT_STATS_RX_PDEV_BE_UL_TRIG_STATS_TAG
 * NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_rx_pdev_be_ul_trig_stats_tlv ul_trigger_tlv;
} htt_rx_pdev_be_ul_trigger_stats_t;
#endif /* ATH_TARGET */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 user_index;
    /** PPDU level */
    A_UINT32 rx_ulofdma_non_data_ppdu;
    /** PPDU level */
    A_UINT32 rx_ulofdma_data_ppdu;
    /** MPDU level */
    A_UINT32 rx_ulofdma_mpdu_ok;
    /** MPDU level */
    A_UINT32 rx_ulofdma_mpdu_fail;
    A_UINT32 rx_ulofdma_non_data_nusers;
    A_UINT32 rx_ulofdma_data_nusers;
} htt_stats_rx_pdev_ul_ofdma_user_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_ul_ofdma_user_stats_tlv
    htt_rx_pdev_ul_ofdma_user_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 user_index;
    /** PPDU level */
    A_UINT32 be_rx_ulofdma_non_data_ppdu;
    /** PPDU level */
    A_UINT32 be_rx_ulofdma_data_ppdu;
    /** MPDU level */
    A_UINT32 be_rx_ulofdma_mpdu_ok;
    /** MPDU level */
    A_UINT32 be_rx_ulofdma_mpdu_fail;
    A_UINT32 be_rx_ulofdma_non_data_nusers;
    A_UINT32 be_rx_ulofdma_data_nusers;
} htt_stats_rx_pdev_be_ul_ofdma_user_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_be_ul_ofdma_user_stats_tlv
    htt_rx_pdev_be_ul_ofdma_user_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 user_index;
    /** PPDU level */
    A_UINT32 rx_ulmumimo_non_data_ppdu;
    /** PPDU level */
    A_UINT32 rx_ulmumimo_data_ppdu;
    /** MPDU level */
    A_UINT32 rx_ulmumimo_mpdu_ok;
    /** MPDU level */
    A_UINT32 rx_ulmumimo_mpdu_fail;
} htt_stats_rx_pdev_ul_mimo_user_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_ul_mimo_user_stats_tlv
    htt_rx_pdev_ul_mimo_user_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 user_index;
    /** PPDU level */
    A_UINT32 be_rx_ulmumimo_non_data_ppdu;
    /** PPDU level */
    A_UINT32 be_rx_ulmumimo_data_ppdu;
    /** MPDU level */
    A_UINT32 be_rx_ulmumimo_mpdu_ok;
    /** MPDU level */
    A_UINT32 be_rx_ulmumimo_mpdu_fail;
} htt_stats_rx_pdev_be_ul_mimo_user_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_be_ul_mimo_user_stats_tlv
    htt_rx_pdev_be_ul_mimo_user_stats_tlv;

/* == RX PDEV/SOC STATS == */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /**
     * BIT [7:0]  :- mac_id
     * BIT [31:8] :- reserved
     *
     * Refer to HTT_STATS_CMN_MAC_ID_GET/SET macros.
     */
    A_UINT32 mac_id__word;

    /** Number of times UL MUMIMO RX packets received */
    A_UINT32 rx_11ax_ul_mumimo;

    /** 11AX HE UL MU-MIMO RX TB PPDU MCS stats */
    A_UINT32 ul_mumimo_rx_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    /**
     * 11AX HE UL MU-MIMO RX GI & LTF stats.
     * Index 0 indicates 1xLTF + 1.6 msec GI
     * Index 1 indicates 2xLTF + 1.6 msec GI
     * Index 2 indicates 4xLTF + 3.2 msec GI
     */
    A_UINT32 ul_mumimo_rx_gi[HTT_RX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    /**
     * 11AX HE UL MU-MIMO RX TB PPDU NSS stats
     * (Increments the individual user NSS in the UL MU MIMO PPDU received)
     */
    A_UINT32 ul_mumimo_rx_nss[HTT_RX_PDEV_STATS_ULMUMIMO_NUM_SPATIAL_STREAMS];
    /** 11AX HE UL MU-MIMO RX TB PPDU BW stats */
    A_UINT32 ul_mumimo_rx_bw[HTT_RX_PDEV_STATS_NUM_BW_COUNTERS];
    /** Number of times UL MUMIMO TB PPDUs received with STBC */
    A_UINT32 ul_mumimo_rx_stbc;
    /** Number of times UL MUMIMO TB PPDUs received with LDPC */
    A_UINT32 ul_mumimo_rx_ldpc;

    /* Stats for MCS 12/13 */
    A_UINT32 ul_mumimo_rx_mcs_ext[HTT_RX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS];
    A_UINT32 ul_mumimo_rx_gi_ext[HTT_RX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS];

    /** RSSI in dBm for Rx TB PPDUs */
    A_INT8 rx_ul_mumimo_chain_rssi_in_dbm[HTT_RX_PDEV_STATS_ULMUMIMO_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_STATS_TOTAL_BW_COUNTERS];
    /** Target RSSI programmed in UL MUMIMO triggers (units dBm) */
    A_INT8 rx_ul_mumimo_target_rssi[HTT_TX_PDEV_STATS_NUM_UL_MUMIMO_USER_STATS][HTT_RX_PDEV_STATS_NUM_BW_COUNTERS];
    /** FD RSSI measured for Rx UL TB PPDUs (units dBm) */
    A_INT8 rx_ul_mumimo_fd_rssi[HTT_TX_PDEV_STATS_NUM_UL_MUMIMO_USER_STATS][HTT_RX_PDEV_STATS_ULMUMIMO_NUM_SPATIAL_STREAMS];
    /** Average pilot EVM measued for RX UL TB PPDU */
    A_INT8 rx_ulmumimo_pilot_evm_dB_mean[HTT_TX_PDEV_STATS_NUM_UL_MUMIMO_USER_STATS][HTT_RX_PDEV_STATS_ULMUMIMO_NUM_SPATIAL_STREAMS];
    A_UINT32 reduced_ul_mumimo_rx_bw[HTT_RX_PDEV_STATS_NUM_REDUCED_CHAN_TYPES][HTT_RX_PDEV_STATS_NUM_BW_COUNTERS];

    /*
     * Number of HE UL MU-MIMO per-user responses containing only a QoS null in
     * response to basic trigger. Typically a data response is expected.
     */
    A_UINT32 ul_mumimo_basic_trigger_rx_qos_null_only;
} htt_stats_rx_pdev_ul_mumimo_trig_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_ul_mumimo_trig_stats_tlv
    htt_rx_pdev_ul_mumimo_trig_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /**
     * BIT [7:0]  :- mac_id
     * BIT [31:8] :- reserved
     *
     * Refer to HTT_STATS_CMN_MAC_ID_GET/SET macros.
     */
    A_UINT32 mac_id__word;

    /** Number of times UL MUMIMO RX packets received */
    A_UINT32 rx_11be_ul_mumimo;

    /** 11BE EHT UL MU-MIMO RX TB PPDU MCS stats */
    A_UINT32 be_ul_mumimo_rx_mcs[HTT_RX_PDEV_STATS_NUM_BE_MCS_COUNTERS];
    /**
     * 11BE EHT UL MU-MIMO RX GI & LTF stats.
     * Index 0 indicates 1xLTF + 1.6 msec GI
     * Index 1 indicates 2xLTF + 1.6 msec GI
     * Index 2 indicates 4xLTF + 3.2 msec GI
     */
    A_UINT32 be_ul_mumimo_rx_gi[HTT_RX_PDEV_STATS_NUM_GI_COUNTERS][HTT_RX_PDEV_STATS_NUM_BE_MCS_COUNTERS];
    /**
     * 11BE EHT UL MU-MIMO RX TB PPDU NSS stats
     * (Increments the individual user NSS in the UL MU MIMO PPDU received)
     */
    A_UINT32 be_ul_mumimo_rx_nss[HTT_RX_PDEV_STATS_ULMUMIMO_NUM_SPATIAL_STREAMS];
    /** 11BE EHT UL MU-MIMO RX TB PPDU BW stats */
    A_UINT32 be_ul_mumimo_rx_bw[HTT_RX_PDEV_STATS_NUM_BE_BW_COUNTERS];
    /** Number of times UL MUMIMO TB PPDUs received with STBC */
    A_UINT32 be_ul_mumimo_rx_stbc;
    /** Number of times UL MUMIMO TB PPDUs received with LDPC */
    A_UINT32 be_ul_mumimo_rx_ldpc;

    /** RSSI in dBm for Rx TB PPDUs */
    A_INT8 be_rx_ul_mumimo_chain_rssi_in_dbm[HTT_RX_PDEV_STATS_ULMUMIMO_NUM_SPATIAL_STREAMS][HTT_RX_PDEV_STATS_NUM_BE_BW_COUNTERS];
    /** Target RSSI programmed in UL MUMIMO triggers (units dBm) */
    A_INT8 be_rx_ul_mumimo_target_rssi[HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER][HTT_RX_PDEV_STATS_NUM_BE_BW_COUNTERS];
    /** FD RSSI measured for Rx UL TB PPDUs (units dBm) */
    A_INT8 be_rx_ul_mumimo_fd_rssi[HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER][HTT_RX_PDEV_STATS_ULMUMIMO_NUM_SPATIAL_STREAMS];
    /** Average pilot EVM measued for RX UL TB PPDU */
    A_INT8 be_rx_ulmumimo_pilot_evm_dB_mean[HTT_RX_PDEV_MAX_ULMUMIMO_NUM_USER][HTT_RX_PDEV_STATS_ULMUMIMO_NUM_SPATIAL_STREAMS];
    /** Number of times UL MUMIMO TB PPDUs received in a punctured mode */
    A_UINT32 rx_ul_mumimo_punctured_mode[HTT_RX_PDEV_STATS_NUM_PUNCTURED_MODE_COUNTERS];

    /*
     * Number of EHT UL MU-MIMO per-user responses containing only a QoS null
     * in response to basic trigger. Typically a data response is expected.
     */
    A_UINT32 be_ul_mumimo_basic_trigger_rx_qos_null_only;
} htt_stats_rx_pdev_ul_mumimo_trig_be_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_ul_mumimo_trig_be_stats_tlv
    htt_rx_pdev_ul_mumimo_trig_be_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_UL_MUMIMO_TRIG_STATS
 * TLV_TAGS:
 *    - HTT_STATS_RX_PDEV_UL_MUMIMO_TRIG_STATS_TAG
 *    - HTT_STATS_RX_PDEV_UL_MUMIMO_TRIG_BE_STATS_TAG
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_rx_pdev_ul_mumimo_trig_stats_tlv    ul_mumimo_trig_tlv;
    htt_stats_rx_pdev_ul_mumimo_trig_be_stats_tlv ul_mumimo_trig_be_tlv;
} htt_rx_pdev_ul_mumimo_trig_stats_t;
#endif /* ATH_TARGET */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** Num Packets received on REO FW ring */
    A_UINT32 fw_reo_ring_data_msdu;
    /** Num bc/mc packets indicated from fw to host */
    A_UINT32 fw_to_host_data_msdu_bcmc;
    /** Num unicast packets indicated from fw to host */
    A_UINT32 fw_to_host_data_msdu_uc;
    /** Num remote buf recycle from offload  */
    A_UINT32 ofld_remote_data_buf_recycle_cnt;
    /** Num remote free buf given to offload */
    A_UINT32 ofld_remote_free_buf_indication_cnt;

    /** Num unicast packets from local path indicated to host */
    A_UINT32 ofld_buf_to_host_data_msdu_uc;
    /** Num unicast packets from REO indicated to host */
    A_UINT32 reo_fw_ring_to_host_data_msdu_uc;

    /** Num Packets received from WBM SW1 ring */
    A_UINT32 wbm_sw_ring_reap;
    /** Num packets from WBM forwarded from fw to host via WBM */
    A_UINT32 wbm_forward_to_host_cnt;
    /** Num packets from WBM recycled to target refill ring */
    A_UINT32 wbm_target_recycle_cnt;

    /**
     * Total Num of recycled to refill ring,
     * including packets from WBM and REO
     */
    A_UINT32 target_refill_ring_recycle_cnt;
} htt_stats_rx_soc_fw_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_soc_fw_stats_tlv htt_rx_soc_fw_stats_tlv;

#define HTT_RX_SOC_FW_REFILL_RING_EMPTY_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** refill_ring_empty_cnt:
     * Num ring empty encountered,
     * HTT_RX_STATS_REFILL_MAX_RING
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, refill_ring_empty_cnt);
} htt_stats_rx_soc_fw_refill_ring_empty_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_soc_fw_refill_ring_empty_tlv
    htt_rx_soc_fw_refill_ring_empty_tlv_v;

#define HTT_RX_SOC_FW_REFILL_RING_EMPTY_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** refill_ring_num_refill:
     * Num total buf refilled from refill ring,
     * HTT_RX_STATS_REFILL_MAX_RING
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, refill_ring_num_refill);
} htt_stats_rx_soc_fw_refill_ring_num_refill_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_soc_fw_refill_ring_num_refill_tlv
    htt_rx_soc_fw_refill_ring_num_refill_tlv_v;

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

    /** NOTE:
     * The mapping of RXDMA error types to rxdma_err array elements is HW dependent.
     * It is expected but not required that the target will provide a rxdma_err element
     * for each of the htt_rx_rxdma_error_code_enum values, up to but not including
     * MAX_ERR_CODE.  The host should ignore any array elements whose
     * indices are >= the MAX_ERR_CODE value the host was compiled with.
     *
     * HTT_RX_RXDMA_MAX_ERR_CODE
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, rxdma_err);
} htt_stats_rx_refill_rxdma_err_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_refill_rxdma_err_tlv
    htt_rx_soc_fw_refill_ring_num_rxdma_err_tlv_v;

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

    /** NOTE:
     * The mapping of REO error types to reo_err array elements is HW dependent.
     * It is expected but not required that the target will provide a rxdma_err element
     * for each of the htt_rx_reo_error_code_enum values, up to but not including
     * MAX_ERR_CODE.  The host should ignore any array elements whose
     * indices are >= the MAX_ERR_CODE value the host was compiled with.
     *
     * HTT_RX_REO_MAX_ERR_CODE
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, reo_err);
} htt_stats_rx_refill_reo_err_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_refill_reo_err_tlv
    htt_rx_soc_fw_refill_ring_num_reo_err_tlv_v;

/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_rx_soc_fw_stats_tlv              fw_tlv;
    htt_stats_rx_soc_fw_refill_ring_empty_tlv  fw_refill_ring_empty_tlv;
    htt_stats_rx_soc_fw_refill_ring_num_refill_tlv
                                               fw_refill_ring_num_refill_tlv;
    htt_stats_rx_refill_rxdma_err_tlv          fw_refill_ring_num_rxdma_err_tlv;
    htt_stats_rx_refill_reo_err_tlv            fw_refill_ring_num_reo_err_tlv;
} htt_rx_soc_stats_t;
#endif /* ATH_TARGET */

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

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /** Num PPDU status processed from HW */
    A_UINT32 ppdu_recvd;
    /** Num MPDU across PPDUs with FCS ok */
    A_UINT32 mpdu_cnt_fcs_ok;
    /** Num MPDU across PPDUs with FCS err */
    A_UINT32 mpdu_cnt_fcs_err;
    /** Num MSDU across PPDUs */
    A_UINT32 tcp_msdu_cnt;
    /** Num MSDU across PPDUs */
    A_UINT32 tcp_ack_msdu_cnt;
    /** Num MSDU across PPDUs */
    A_UINT32 udp_msdu_cnt;
    /** Num MSDU across PPDUs */
    A_UINT32 other_msdu_cnt;
    /** Num MPDU on FW ring indicated */
    A_UINT32 fw_ring_mpdu_ind;
    /** Num MGMT MPDU given to protocol */
    A_UINT32 fw_ring_mgmt_subtype[HTT_STATS_SUBTYPE_MAX];
    /** Num ctrl MPDU given to protocol */
    A_UINT32 fw_ring_ctrl_subtype[HTT_STATS_SUBTYPE_MAX];
    /** Num mcast data packet received */
    A_UINT32 fw_ring_mcast_data_msdu;
    /** Num broadcast data packet received */
    A_UINT32 fw_ring_bcast_data_msdu;
    /** Num unicast data packet received */
    A_UINT32 fw_ring_ucast_data_msdu;
    /** Num null data packet received  */
    A_UINT32 fw_ring_null_data_msdu;
    /** Num MPDU on FW ring dropped */
    A_UINT32 fw_ring_mpdu_drop;

    /** Num buf indication to offload */
    A_UINT32 ofld_local_data_ind_cnt;
    /** Num buf recycle from offload */
    A_UINT32 ofld_local_data_buf_recycle_cnt;
    /** Num buf indication to data_rx */
    A_UINT32 drx_local_data_ind_cnt;
    /** Num buf recycle from data_rx */
    A_UINT32 drx_local_data_buf_recycle_cnt;
    /** Num buf indication to protocol */
    A_UINT32 local_nondata_ind_cnt;
    /** Num buf recycle from protocol */
    A_UINT32 local_nondata_buf_recycle_cnt;

    /** Num buf fed */
    A_UINT32 fw_status_buf_ring_refill_cnt;
    /** Num ring empty encountered */
    A_UINT32 fw_status_buf_ring_empty_cnt;
    /** Num buf fed  */
    A_UINT32 fw_pkt_buf_ring_refill_cnt;
    /** Num ring empty encountered */
    A_UINT32 fw_pkt_buf_ring_empty_cnt;
    /** Num buf fed  */
    A_UINT32 fw_link_buf_ring_refill_cnt;
    /** Num ring empty encountered  */
    A_UINT32 fw_link_buf_ring_empty_cnt;

    /** Num buf fed */
    A_UINT32 host_pkt_buf_ring_refill_cnt;
    /** Num ring empty encountered */
    A_UINT32 host_pkt_buf_ring_empty_cnt;
    /** Num buf fed */
    A_UINT32 mon_pkt_buf_ring_refill_cnt;
    /** Num ring empty encountered */
    A_UINT32 mon_pkt_buf_ring_empty_cnt;
    /** Num buf fed */
    A_UINT32 mon_status_buf_ring_refill_cnt;
    /** Num ring empty encountered */
    A_UINT32 mon_status_buf_ring_empty_cnt;
    /** Num buf fed */
    A_UINT32 mon_desc_buf_ring_refill_cnt;
    /** Num ring empty encountered */
    A_UINT32 mon_desc_buf_ring_empty_cnt;
    /** Num buf fed */
    A_UINT32 mon_dest_ring_update_cnt;
    /** Num ring full encountered */
    A_UINT32 mon_dest_ring_full_cnt;

    /** Num rx suspend is attempted */
    A_UINT32 rx_suspend_cnt;
    /** Num rx suspend failed */
    A_UINT32 rx_suspend_fail_cnt;
    /** Num rx resume attempted */
    A_UINT32 rx_resume_cnt;
    /** Num rx resume failed */
    A_UINT32 rx_resume_fail_cnt;
    /** Num rx ring switch */
    A_UINT32 rx_ring_switch_cnt;
    /** Num rx ring restore */
    A_UINT32 rx_ring_restore_cnt;
    /** Num rx flush issued */
    A_UINT32 rx_flush_cnt;
    /** Num rx recovery */
    A_UINT32 rx_recovery_reset_cnt;
} htt_stats_rx_pdev_fw_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_fw_stats_tlv htt_rx_pdev_fw_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** peer mac address */
    htt_mac_addr peer_mac_addr;
    /** Num of tx mgmt frames with subtype on peer level */
    A_UINT32 peer_tx_mgmt_subtype[HTT_STATS_SUBTYPE_MAX];
    /** Num of rx mgmt frames with subtype on peer level */
    A_UINT32 peer_rx_mgmt_subtype[HTT_STATS_SUBTYPE_MAX];
} htt_stats_peer_ctrl_path_txrx_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_peer_ctrl_path_txrx_stats_tlv
    htt_peer_ctrl_path_txrx_stats_tlv;

#define HTT_STATS_PHY_ERR_MAX 43

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    /** Num of phy err */
    A_UINT32 total_phy_err_cnt;
    /** Counts of different types of phy errs
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
} htt_stats_rx_pdev_fw_stats_phy_err_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_fw_stats_phy_err_tlv htt_rx_pdev_fw_stats_phy_err_tlv;

#define HTT_RX_PDEV_FW_RING_MPDU_ERR_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** fw_ring_mpdu_err:
     * Num error MPDU for each RxDMA error type,
     * HTT_RX_STATS_RXDMA_MAX_ERR
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, fw_ring_mpdu_err);
} htt_stats_rx_pdev_fw_ring_mpdu_err_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_fw_ring_mpdu_err_tlv
    htt_rx_pdev_fw_ring_mpdu_err_tlv_v;

#define HTT_RX_PDEV_FW_MPDU_DROP_TLV_SZ(_num_elems) (sizeof(A_UINT32) * (_num_elems))

/* NOTE: Variable length TLV, use length spec to infer array size */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** fw_mpdu_drop:
     * Num MPDU dropped,
     * HTT_RX_STATS_FW_DROP_REASON_MAX
     */
    HTT_STATS_VAR_LEN_ARRAY1(A_UINT32, fw_mpdu_drop);
} htt_stats_rx_pdev_fw_mpdu_drop_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_pdev_fw_mpdu_drop_tlv htt_rx_pdev_fw_mpdu_drop_tlv_v;

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
#ifdef ATH_TARGET
typedef struct {
    htt_rx_soc_stats_t                     soc_stats;
    htt_stats_rx_pdev_fw_stats_tlv         fw_stats_tlv;
    htt_stats_rx_pdev_fw_ring_mpdu_err_tlv fw_ring_mpdu_err_tlv;
    htt_stats_rx_pdev_fw_mpdu_drop_tlv     fw_ring_mpdu_drop;
    htt_stats_rx_pdev_fw_stats_phy_err_tlv fw_stats_phy_err_tlv;
} htt_rx_pdev_stats_t;
#endif /* ATH_TARGET */

/* STATS_TYPE : HTT_DBG_EXT_PEER_CTRL_PATH_TXRX_STATS
 * TLV_TAGS:
 *      - HTT_STATS_PEER_CTRL_PATH_TXRX_STATS_TAG
 *
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_peer_ctrl_path_txrx_stats_tlv peer_ctrl_path_txrx_stats_tlv;
} htt_ctrl_path_txrx_stats_t;
#endif /* ATH_TARGET */

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
    A_UINT32 pre_rx_frame_usec;
} htt_stats_pdev_cca_counters_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_cca_counters_tlv htt_pdev_stats_cca_counters_tlv;

/* NOTE: THIS htt_pdev_cca_stats_hist_tlv STRUCTURE IS DEPRECATED,
 * due to lack of support in some host stats infrastructures for
 * TLVs nested within TLVs.
 */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /** The channel number on which these stats were collected */
    A_UINT32 chan_num;

    /** num of CCA records (Num of htt_pdev_stats_cca_counters_tlv)*/
    A_UINT32 num_records;

    /**
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

    /** Indicates the stats collection interval
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
    HTT_STATS_VAR_LEN_ARRAY1(htt_stats_pdev_cca_counters_tlv, cca_hist_tlv);
} htt_pdev_cca_stats_hist_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /** The channel number on which these stats were collected */
    A_UINT32 chan_num;

    /** num of CCA records (Num of htt_pdev_stats_cca_counters_tlv)*/
    A_UINT32 num_records;

    /**
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

    /** Indicates the stats collection interval
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

#define HTT_TWT_SESSION_FLAG_FLOW_ID_M 0x0000000f
#define HTT_TWT_SESSION_FLAG_FLOW_ID_S 0

#define HTT_TWT_SESSION_FLAG_BTWT_PEER_CNT_M 0x0000fff0
#define HTT_TWT_SESSION_FLAG_BTWT_PEER_CNT_S 4

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

#define HTT_TWT_SESSION_FLAG_BTWT_PEER_CNT_GET(_var) \
    (((_var) & HTT_TWT_SESSION_FLAG_BTWT_PEER_CNT_M) >> \
     HTT_TWT_SESSION_FLAG_BTWT_PEER_CNT_S)

#define HTT_TWT_SESSION_FLAG_BTWT_PEER_CNT_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_TWT_SESSION_FLAG_BTWT_PEER_CNT, _val); \
        ((_var) |= ((_val) << HTT_TWT_SESSION_FLAG_BTWT_PEER_CNT_S)); \
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
    /**
     * TWT_DIALOG_ID_UNAVAILABLE is used when TWT session is
     * not initiated by host
     */
    A_UINT32     dialog_id;
    A_UINT32     wake_dura_us;
    A_UINT32     wake_intvl_us;
    A_UINT32     sp_offset_us;
} htt_stats_pdev_twt_session_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_twt_session_tlv htt_pdev_stats_twt_session_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 pdev_id;
    A_UINT32 num_sessions;

    HTT_STATS_VAR_LEN_ARRAY1(htt_stats_pdev_twt_session_tlv, twt_session);
} htt_stats_pdev_twt_sessions_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_twt_sessions_tlv htt_pdev_stats_twt_sessions_tlv;

/* STATS_TYPE: HTT_DBG_EXT_STATS_TWT_SESSIONS
 * TLV_TAGS:
 *     - HTT_STATS_PDEV_TWT_SESSIONS_TAG
 *     - HTT_STATS_PDEV_TWT_SESSION_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_pdev_twt_session_tlv twt_sessions[1];
} htt_pdev_twt_sessions_stats_t;
#endif /* ATH_TARGET */

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
    /** htt_rx_reo_debug_sample_id_enum */
    A_UINT32 sample_id;
    /** Max value of all samples */
    A_UINT32 total_max;
    /** Average value of total samples */
    A_UINT32 total_avg;
    /** Num of samples including both zeros and non zeros ones*/
    A_UINT32 total_sample;
    /** Average value of all non zeros samples */
    A_UINT32 non_zeros_avg;
    /** Num of non zeros samples */
    A_UINT32 non_zeros_sample;
    /** Max value of last N non zero samples (N = last_non_zeros_sample) */
    A_UINT32 last_non_zeros_max;
    /** Min value of last N non zero samples (N = last_non_zeros_sample) */
    A_UINT32 last_non_zeros_min;
    /** Average value of last N non zero samples (N = last_non_zeros_sample) */
    A_UINT32 last_non_zeros_avg;
    /** Num of last non zero samples */
    A_UINT32 last_non_zeros_sample;
} htt_stats_rx_reo_resource_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_reo_resource_stats_tlv htt_rx_reo_resource_stats_tlv_v;

/* STATS_TYPE: HTT_DBG_EXT_STATS_REO_RESOURCE_STATS
 * TLV_TAGS:
 *     - HTT_STATS_RX_REO_RESOURCE_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_rx_reo_resource_stats_tlv reo_resource_stats;
} htt_soc_reo_resource_stats_t;
#endif /* ATH_TARGET */

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
    /* For backward compatibility new modes cannot be added */
    HTT_TXBF_MAX_NUM_OF_MODES = 5
} htt_txbf_sound_steer_modes;

typedef enum {
    HTT_TX_AC_SOUNDING_MODE  = 0,
    HTT_TX_AX_SOUNDING_MODE  = 1,
    HTT_TX_BE_SOUNDING_MODE  = 2,
    HTT_TX_CMN_SOUNDING_MODE = 3,
    HTT_TX_CV_CORR_MODE      = 4,
} htt_stats_sounding_tx_mode;

#define HTT_TX_CV_CORR_MAX_NUM_COLUMNS 8

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32      tx_sounding_mode; /* HTT_TX_XX_SOUNDING_MODE */
    /* Counts number of soundings for all steering modes in each bw */
    A_UINT32 cbf_20[HTT_TXBF_MAX_NUM_OF_MODES];
    A_UINT32 cbf_40[HTT_TXBF_MAX_NUM_OF_MODES];
    A_UINT32 cbf_80[HTT_TXBF_MAX_NUM_OF_MODES];
    A_UINT32 cbf_160[HTT_TXBF_MAX_NUM_OF_MODES];
    /**
     * The sounding array is a 2-D array stored as an 1-D array of
     * A_UINT32. The stats for a particular user/bw combination is
     * referenced with the following:
     *
     *          sounding[(user* max_bw) + bw]
     *
     * ... where max_bw == 4 for 160mhz
     */
    A_UINT32 sounding[HTT_TX_NUM_OF_SOUNDING_STATS_WORDS];

    /* cv upload handler stats */
    /** total times CV nc mismatched */
    A_UINT32 cv_nc_mismatch_err;
    /** total times CV has FCS error */
    A_UINT32 cv_fcs_err;
    /** total times CV has invalid NSS index */
    A_UINT32 cv_frag_idx_mismatch;
    /** total times CV has invalid SW peer ID */
    A_UINT32 cv_invalid_peer_id;
    /** total times CV rejected because TXBF is not setup in peer */
    A_UINT32 cv_no_txbf_setup;
    /** total times CV expired while in updating state */
    A_UINT32 cv_expiry_in_update;
    /** total times Pkt b/w exceeding the cbf_bw */
    A_UINT32 cv_pkt_bw_exceed;
    /** total times CV DMA not completed */
    A_UINT32 cv_dma_not_done_err;
    /** total times CV update to peer failed */
    A_UINT32 cv_update_failed;

    /* cv query stats */
    /** total times CV query happened */
    A_UINT32 cv_total_query;
    /** total pattern based CV query */
    A_UINT32 cv_total_pattern_query;
    /** total BW based CV query */
    A_UINT32 cv_total_bw_query;
    /** incorrect encoding in CV flags */
    A_UINT32 cv_invalid_bw_coding;
    /** forced sounding enabled for the peer */
    A_UINT32 cv_forced_sounding;
    /** standalone sounding sequence on-going */
    A_UINT32 cv_standalone_sounding;
    /** NC of available CV lower than expected */
    A_UINT32 cv_nc_mismatch;
    /** feedback type different from expected */
    A_UINT32 cv_fb_type_mismatch;
    /** CV BW not equal to expected BW for OFDMA */
    A_UINT32 cv_ofdma_bw_mismatch;
    /** CV BW not greater than or equal to expected BW */
    A_UINT32 cv_bw_mismatch;
    /** CV pattern not matching with the expected pattern */
    A_UINT32 cv_pattern_mismatch;
    /** CV available is of different preamble type than expected. */
    A_UINT32 cv_preamble_mismatch;
    /** NR of available CV is lower than expected. */
    A_UINT32 cv_nr_mismatch;
    /** CV in use count has exceeded threshold and cannot be used further. */
    A_UINT32 cv_in_use_cnt_exceeded;
    /** A valid CV has been found. */
    A_UINT32 cv_found;
    /** No valid CV was found. */
    A_UINT32 cv_not_found;
    /** Sounding per user in 320MHz bandwidth */
    A_UINT32 sounding_320[HTT_TX_PDEV_STATS_NUM_BE_MUMIMO_USER_STATS];
    /** Counts number of soundings for all steering modes in 320MHz bandwidth */
    A_UINT32 cbf_320[HTT_TXBF_MAX_NUM_OF_MODES];
    /* This part can be used for new counters added for CV query/upload. */
    /** non-trigger based ranging sequence on-going */
    A_UINT32 cv_ntbr_sounding;
    /** CV found, but upload is in progress. */
    A_UINT32 cv_found_upload_in_progress;
    /** Expired CV found during query. */
    A_UINT32 cv_expired_during_query;
    /** total times CV dma timeout happened */
    A_UINT32 cv_dma_timeout_error;
    /** total times CV bufs uploaded for IBF case */
    A_UINT32 cv_buf_ibf_uploads;
    /** total times CV bufs uploaded for EBF case */
    A_UINT32 cv_buf_ebf_uploads;
    /** total times CV bufs received from IPC ring */
    A_UINT32 cv_buf_received;
    /** total times CV bufs fed back to the IPC ring */
    A_UINT32 cv_buf_fed_back;
    /** Total times CV query happened for IBF case */
    A_UINT32 cv_total_query_ibf;
    /** A valid CV has been found for IBF case */
    A_UINT32 cv_found_ibf;
    /** A valid CV has not been found for IBF case */
    A_UINT32 cv_not_found_ibf;
    /** Expired CV found during query for IBF case */
    A_UINT32 cv_expired_during_query_ibf;
    /** Total number of times adaptive sounding logic has been queried */
    A_UINT32 adaptive_snd_total_query;
    /**
     * Total number of times adaptive sounding mcs drop has been computed
     * and recorded.
     */
    A_UINT32 adaptive_snd_total_mcs_drop[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS + HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS];
    /** Total number of times adaptive sounding logic kicked in */
    A_UINT32 adaptive_snd_kicked_in;
    /** Total number of times we switched back to normal sounding interval */
    A_UINT32 adaptive_snd_back_to_default;

    /**
     * Below are CV correlation feature related stats.
     * This feature is used for DL MU MIMO, but is not available
     * from certain legacy targets.
     */

    /** number of CV Correlation triggers for online mode */
    A_UINT32 cv_corr_trigger_online_mode;
    /** number of CV Correlation triggers for offline mode */
    A_UINT32 cv_corr_trigger_offline_mode;
    /** number of CV Correlation triggers for hybrid mode */
    A_UINT32 cv_corr_trigger_hybrid_mode;
    /** number of CV Correlation triggers with computation level 0 */
    A_UINT32 cv_corr_trigger_computation_level_0;
    /** number of CV Correlation triggers with computation level 1 */
    A_UINT32 cv_corr_trigger_computation_level_1;
    /** number of CV Correlation triggers with computation level 2 */
    A_UINT32 cv_corr_trigger_computation_level_2;
    /** number of users for which CV Correlation was triggered */
    A_UINT32 cv_corr_trigger_num_users[HTT_TX_CV_CORR_MAX_NUM_COLUMNS];
    /** number of streams for which CV Correlation was triggered */
    A_UINT32 cv_corr_trigger_num_streams[HTT_TX_CV_CORR_MAX_NUM_COLUMNS];
    /** number of CV Correlation buffers received through IPC tickle */
    A_UINT32 cv_corr_upload_total_buf_received;
    /** number of CV Correlation buffers fed back to the IPC ring */
    A_UINT32 cv_corr_upload_total_buf_fed_back;
    /** number of CV Correlation buffers for which processing failed */
    A_UINT32 cv_corr_upload_total_processing_failed;
    /**
     * number of CV Correlation buffers for which processing failed,
     * due to no users being present in parsed buffer
     */
    A_UINT32 cv_corr_upload_failed_total_users_zero;
    /**
     * number of CV Correlation buffers for which processing failed,
     * due to number of users present in parsed buffer exceeded
     * CV_CORR_MAX_NUM_COLUMNS
     */
    A_UINT32 cv_corr_upload_failed_total_users_exceeded;
    /**
     * number of CV Correlation buffers for which processing failed,
     * due to peer pointer for parsed peer not available
     */
    A_UINT32 cv_corr_upload_failed_peer_not_found;
    /**
     * number of CV Correlation buffers for which processing encountered,
     * Nss of peer exceeding SCHED_ALGO_MAX_SUPPORTED_MUMIMO_NSS
     */
    A_UINT32 cv_corr_upload_user_nss_exceeded;
    /**
     * number of CV Correlation buffers for which processing encountered,
     * invalid reverse look up index for fetching CV correlation results
     */
    A_UINT32 cv_corr_upload_invalid_lookup_index;
    /** number of users present in uploaded CV Correlation results buffer */
    A_UINT32 cv_corr_upload_total_num_users[HTT_TX_CV_CORR_MAX_NUM_COLUMNS];
    /** number of streams present in uploaded CV Correlation results buffer */
    A_UINT32 cv_corr_upload_total_num_streams[HTT_TX_CV_CORR_MAX_NUM_COLUMNS];
} htt_stats_tx_sounding_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_tx_sounding_stats_tlv htt_tx_sounding_stats_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_TX_SOUNDING_INFO
 * TLV_TAGS:
 *      - HTT_STATS_TX_SOUNDING_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_tx_sounding_stats_tlv sounding_tlv;
} htt_tx_sounding_stats_t;
#endif /* ATH_TARGET */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 num_obss_tx_ppdu_success;
    A_UINT32 num_obss_tx_ppdu_failure;
    /** num_sr_tx_transmissions:
     * Counter of TX done by aborting other BSS RX with spatial reuse
     * (for cases where rx RSSI from other BSS is below the packet-detection
     * threshold for doing spatial reuse)
     */
    union {
        A_UINT32 num_sr_tx_transmissions; /* CORRECTED - use this one */
        A_UINT32 num_sr_tx_tranmissions;  /* DEPRECATED - has typo in name */
    };
    union {
        /**
         * Count the number of times the RSSI from an other-BSS signal
         * is below the spatial reuse power threshold, thus providing an
         * opportunity for spatial reuse since OBSS interference will be
         * inconsequential.
         */
        A_UINT32 num_spatial_reuse_opportunities;

        /* DEPRECATED: num_sr_rx_ge_pd_rssi_thr
         * This old name has been deprecated because it does not
         * clearly and accurately reflect the information stored within
         * this field.
         * Use the new name (num_spatial_reuse_opportunities) instead of
         * the deprecated old name (num_sr_rx_ge_pd_rssi_thr).
         */
        A_UINT32 num_sr_rx_ge_pd_rssi_thr;
    };

    /**
     * Count of number of times OBSS frames were aborted and non-SRG
     * opportunities were created. Non-SRG opportunities are created when
     * incoming OBSS RSSI is lesser than the global configured non-SRG RSSI
     * threshold and non-SRG OBSS color / non-SRG OBSS BSSID registers
     * allow non-SRG TX.
     */
    A_UINT32 num_non_srg_opportunities;
    /**
     * Count of number of times TX PPDU were transmitted using non-SRG
     * opportunities created. Incoming OBSS frame RSSI is compared with per
     * PPDU non-SRG RSSI threshold configured in each PPDU. If incoming OBSS
     * RSSI < non-SRG RSSI threshold configured in each PPDU, then non-SRG
     * transmission happens.
     */
    A_UINT32 num_non_srg_ppdu_tried;
    /**
     * Count of number of times non-SRG based TX transmissions were successful
     */
    A_UINT32 num_non_srg_ppdu_success;
    /**
     * Count of number of times OBSS frames were aborted and SRG opportunities
     * were created. Srg opportunities are created when incoming OBSS RSSI
     * is less than the global configured SRG RSSI threshold and SRC OBSS
     * color / SRG OBSS BSSID / SRG partial bssid / SRG BSS color bitmap
     * registers allow SRG TX.
     */
    A_UINT32 num_srg_opportunities;
    /**
     * Count of number of times TX PPDU were transmitted using SRG
     * opportunities created.
     * Incoming OBSS frame RSSI is compared with per PPDU SRG RSSI
     * threshold configured in each PPDU.
     * If incoming OBSS RSSI < SRG RSSI threshold configured in each PPDU,
     * then SRG transmission happens.
     */
    A_UINT32 num_srg_ppdu_tried;
    /**
     * Count of number of times SRG based TX transmissions were successful
     */
    A_UINT32 num_srg_ppdu_success;
    /**
     * Count of number of times PSR opportunities were created by aborting
     * OBSS UL OFDMA HE-TB PPDU frame. HE-TB ppdu frames are aborted if the
     * spatial reuse info in the OBSS trigger common field is set to allow PSR
     * based spatial reuse.
     */
    A_UINT32 num_psr_opportunities;
    /**
     * Count of number of times TX PPDU were transmitted using PSR
     * opportunities created.
     */
    A_UINT32 num_psr_ppdu_tried;
    /**
     * Count of number of times PSR based TX transmissions were successful.
     */
    A_UINT32 num_psr_ppdu_success;
    /**
     * Count of number of times TX PPDU per access category were transmitted
     * using non-SRG opportunities created.
     */
    A_UINT32 num_non_srg_ppdu_tried_per_ac[HTT_NUM_AC_WMM];
    /**
     * Count of number of times non-SRG based TX transmissions per access
     * category were successful
     */
    A_UINT32 num_non_srg_ppdu_success_per_ac[HTT_NUM_AC_WMM];
    /**
     * Count of number of times TX PPDU per access category were transmitted
     * using SRG opportunities created.
     */
    A_UINT32 num_srg_ppdu_tried_per_ac[HTT_NUM_AC_WMM];
    /**
     * Count of number of times SRG based TX transmissions per access
     * category were successful
     */
    A_UINT32 num_srg_ppdu_success_per_ac[HTT_NUM_AC_WMM];
    /**
     * Count of number of times ppdu was flushed due to ongoing OBSS
     * frame duration value lesser than minimum required frame duration.
     */
    A_UINT32 num_obss_min_duration_check_flush_cnt;
    /**
     * Count of number of times ppdu was flushed due to ppdu duration
     * exceeding aborted OBSS frame duration
     */
    A_UINT32 num_sr_ppdu_abort_flush_cnt;
} htt_stats_pdev_obss_pd_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_obss_pd_tlv htt_pdev_obss_pd_stats_tlv;

/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_pdev_obss_pd_tlv obss_pd_stat;
} htt_pdev_obss_pd_stats_t;
#endif /* ATH_TARGET */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 pdev_id;
    A_UINT32 current_head_idx;
    A_UINT32 current_tail_idx;
    A_UINT32 num_htt_msgs_sent;
    /**
     * Time in milliseconds for which the ring has been in
     * its current backpressure condition
     */
    A_UINT32 backpressure_time_ms;
    /** backpressure_hist -
     * histogram showing how many times different degrees of backpressure
     * duration occurred:
     * Index 0 indicates the number of times ring was
     * continuously in backpressure state for 100 - 200ms.
     * Index 1 indicates the number of times ring was
     * continuously in backpressure state for 200 - 300ms.
     * Index 2 indicates the number of times ring was
     * continuously in backpressure state for 300 - 400ms.
     * Index 3 indicates the number of times ring was
     * continuously in backpressure state for 400 - 500ms.
     * Index 4 indicates the number of times ring was
     * continuously in backpressure state beyond 500ms.
     */
    A_UINT32 backpressure_hist[5];
} htt_stats_ring_backpressure_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_ring_backpressure_stats_tlv htt_ring_backpressure_stats_tlv;

/* STATS_TYPE : HTT_STATS_RING_BACKPRESSURE_STATS_INFO
 * TLV_TAGS:
 *      - HTT_STATS_RING_BACKPRESSURE_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_sring_cmn_tlv cmn_tlv;
    struct {
        htt_stats_string_tlv sring_str_tlv;
        htt_stats_ring_backpressure_stats_tlv backpressure_stats_tlv;
    } r[1]; /* variable-length array */
} htt_ring_backpressure_stats_t;
#endif /* ATH_TARGET */

#define HTT_LATENCY_PROFILE_MAX_HIST        3
#define HTT_STATS_MAX_PROF_STATS_NAME_LEN  32
#define HTT_INTERRUPTS_LATENCY_PROFILE_MAX_HIST 3
typedef struct {
    htt_tlv_hdr_t   tlv_hdr;
    /** print_header:
     * This field suggests whether the host should print a header when
     * displaying the TLV (because this is the first latency_prof_stats
     * TLV within a series), or if only the TLV contents should be displayed
     * without a header (because this is not the first TLV within the series).
     */
    A_UINT32 print_header;
    A_UINT8 latency_prof_name[HTT_STATS_MAX_PROF_STATS_NAME_LEN];
    /** number of data values included in the tot sum */
    A_UINT32 cnt;
    /** time in us */
    A_UINT32 min;
    /** time in us */
    A_UINT32 max;
    A_UINT32 last;
    /** time in us */
    A_UINT32 tot;
    /** time in us */
    A_UINT32 avg;
    /** hist_intvl:
     * Histogram interval, i.e. the latency range covered by each
     * bin of the histogram, in microsecond units.
     * hist[0] counts how many latencies were between 0 to hist_intvl
     * hist[1] counts how many latencies were between hist_intvl to 2*hist_intvl
     * hist[2] counts how many latencies were more than 2*hist_intvl
     */
    A_UINT32 hist_intvl;
    A_UINT32 hist[HTT_LATENCY_PROFILE_MAX_HIST];
    /** max page faults in any 1 sampling window */
    A_UINT32 page_fault_max;
    /** summed over all sampling windows */
    A_UINT32 page_fault_total;
    /** ignored_latency_count:
     * ignore some of profile latency to avoid avg skewing
     */
    A_UINT32 ignored_latency_count;
    /** interrupts_max: max interrupts within any single sampling window */
    A_UINT32 interrupts_max;
    /** interrupts_hist: histogram of interrupt rate
     * bin0 contains the number of sampling windows that had 0 interrupts,
     * bin1 contains the number of sampling windows that had 1-4 interrupts,
     * bin2 contains the number of sampling windows that had > 4 interrupts
     */
    A_UINT32 interrupts_hist[HTT_INTERRUPTS_LATENCY_PROFILE_MAX_HIST];
} htt_stats_latency_prof_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_latency_prof_stats_tlv htt_latency_prof_stats_tlv;

typedef struct {
    htt_tlv_hdr_t   tlv_hdr;
    /** duration:
     * Time period over which counts were gathered, units = microseconds.
     */
    A_UINT32 duration;
    A_UINT32 tx_msdu_cnt;
    A_UINT32 tx_mpdu_cnt;
    A_UINT32 tx_ppdu_cnt;
    A_UINT32 rx_msdu_cnt;
    A_UINT32 rx_mpdu_cnt;
} htt_stats_latency_ctx_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_latency_ctx_tlv htt_latency_prof_ctx_tlv;

typedef struct {
    htt_tlv_hdr_t   tlv_hdr;
    /** count of enabled profiles */
    A_UINT32 prof_enable_cnt;
} htt_stats_latency_cnt_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_latency_cnt_tlv htt_latency_prof_cnt_tlv;

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
#ifdef ATH_TARGET
typedef struct {
    htt_stats_latency_prof_stats_tlv latency_prof_stat;
    htt_stats_latency_ctx_tlv latency_ctx_stat;
    htt_stats_latency_cnt_tlv latency_cnt_stat;
} htt_soc_latency_stats_t;
#endif /* ATH_TARGET */

#define HTT_RX_MAX_PEAK_OCCUPANCY_INDEX 10
#define HTT_RX_MAX_CURRENT_OCCUPANCY_INDEX 10
#define HTT_RX_SQUARE_INDEX 6
#define HTT_RX_MAX_PEAK_SEARCH_INDEX 4
#define HTT_RX_MAX_PENDING_SEARCH_INDEX 4

/* STATS_TYPE : HTT_DBG_EXT_RX_FSE_STATS
 * TLV_TAGS:
 *    - HTT_STATS_RX_FSE_STATS_TAG
 */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /**
     * Number of times host requested for fse enable/disable
     */
    A_UINT32 fse_enable_cnt;
    A_UINT32 fse_disable_cnt;
    /**
     * Number of times host requested for fse cache invalidation
     * individual entries or full cache
     */
    A_UINT32 fse_cache_invalidate_entry_cnt;
    A_UINT32 fse_full_cache_invalidate_cnt;

    /**
     * Cache hits count will increase if there is a matching flow in the cache
     * There is no register for cache miss but the number of cache misses can
     * be calculated as
     *    cache miss = (num_searches - cache_hits)
     * Thus, there is no need to have a separate variable for cache misses.
     * Num searches is flow search times done in the cache.
     */
    A_UINT32 fse_num_cache_hits_cnt;
    A_UINT32 fse_num_searches_cnt;
    /**
     * Cache Occupancy holds 2 types of values: Peak and Current.
     * 10 bins are used to keep track of peak occupancy.
     * 8 of these bins represent ranges of values, while the first and last
     * bins represent the extreme cases of the cache being completely empty
     * or completely full.
     * For the non-extreme bins, the number of cache occupancy values per
     * bin is the maximum cache occupancy (128), divided by the number of
     * non-extreme bins (8), so 128/8 = 16 values per bin.
     * The range of values for each histogram bins is specified below:
     * Bin0 = Counter increments when cache occupancy is empty
     * Bin1 = Counter increments when cache occupancy is within [1 to 16]
     * Bin2 = Counter increments when cache occupancy is within [17 to 32]
     * Bin3 = Counter increments when cache occupancy is within [33 to 48]
     * Bin4 = Counter increments when cache occupancy is within [49 to 64]
     * Bin5 = Counter increments when cache occupancy is within [65 to 80]
     * Bin6 = Counter increments when cache occupancy is within [81 to 96]
     * Bin7 = Counter increments when cache occupancy is within [97 to 112]
     * Bin8 = Counter increments when cache occupancy is within [113 to 127]
     * Bin9 = Counter increments when cache occupancy is equal to 128
     * The above histogram bin definitions apply to both the peak-occupancy
     * histogram and the current-occupancy histogram.
     *
     * @fse_cache_occupancy_peak_cnt:
     * Array records periodically PEAK cache occupancy values.
     * Peak Occupancy will increment only if it is greater than current
     * occupancy value.
     *
     * @fse_cache_occupancy_curr_cnt:
     * Array records periodically current cache occupancy value.
     * Current Cache occupancy always holds instant snapshot of
     * current number of cache entries.
     **/
    A_UINT32 fse_cache_occupancy_peak_cnt[HTT_RX_MAX_PEAK_OCCUPANCY_INDEX];
    A_UINT32 fse_cache_occupancy_curr_cnt[HTT_RX_MAX_CURRENT_OCCUPANCY_INDEX];
    /**
     * Square stat is sum of squares of cache occupancy to better understand
     * any variation/deviation within each cache set, over a given time-window.
     *
     * Square stat is calculated this way:
     *     Square =  SUM(Squares of all Occupancy in a Set) / 8
     * The cache has 16-way set associativity, so the occupancy of a
     * set can vary from 0 to 16.  There are 8 sets within the cache.
     * Therefore, the minimum possible square value is 0, and the maximum
     * possible square value is (8*16^2) / 8 = 256.
     *
     * 6 bins are used to keep track of square stats:
     * Bin0 = increments when square of current cache occupancy is zero
     * Bin1 = increments when square of current cache occupancy is within
     *        [1 to 50]
     * Bin2 = increments when square of current cache occupancy is within
     *        [51 to 100]
     * Bin3 = increments when square of current cache occupancy is within
     *        [101 to 200]
     * Bin4 = increments when square of current cache occupancy is within
     *        [201 to 255]
     * Bin5 = increments when square of current cache occupancy is 256
     */
    A_UINT32 fse_search_stat_square_cnt[HTT_RX_SQUARE_INDEX];
    /**
     * Search stats has 2 types of values: Peak Pending and Number of
     * Search Pending.
     * GSE command ring for FSE can hold maximum of 5 Pending searches
     * at any given time.
     *
     * 4 bins are used to keep track of search stats:
     * Bin0 = Counter increments when there are NO pending searches
     *        (For peak, it will be number of pending searches greater
     *        than GSE command ring FIFO outstanding requests.
     *        For Search Pending, it will be number of pending search
     *        inside GSE command ring FIFO.)
     * Bin1 = Counter increments when number of pending searches are within
     *        [1 to 2]
     * Bin2 = Counter increments when number of pending searches are within
     *        [3 to 4]
     * Bin3 = Counter increments when number of pending searches are
     *        greater/equal to [ >= 5]
     */
    A_UINT32 fse_search_stat_peak_cnt[HTT_RX_MAX_PEAK_SEARCH_INDEX];
    A_UINT32 fse_search_stat_search_pending_cnt[HTT_RX_MAX_PENDING_SEARCH_INDEX];
} htt_stats_rx_fse_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_rx_fse_stats_tlv htt_rx_fse_stats_tlv;

/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_rx_fse_stats_tlv rx_fse_stats;
} htt_rx_fse_stats_t;
#endif /* ATH_TARGET */

#define HTT_TX_TXBF_RATE_STATS_NUM_MCS_COUNTERS 14
#define HTT_TX_TXBF_RATE_STATS_NUM_BW_COUNTERS 5 /* 20, 40, 80, 160, 320 */

#define HTT_TX_TXBF_RATE_STATS_NUM_REDUCED_CHAN_TYPES 2/* 0: Half, 1: Quarter */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** SU TxBF TX MCS stats */
    A_UINT32 tx_su_txbf_mcs[HTT_TX_TXBF_RATE_STATS_NUM_MCS_COUNTERS];
    /** Implicit BF TX MCS stats */
    A_UINT32 tx_su_ibf_mcs[HTT_TX_TXBF_RATE_STATS_NUM_MCS_COUNTERS];
    /** Open loop TX MCS stats */
    A_UINT32 tx_su_ol_mcs[HTT_TX_TXBF_RATE_STATS_NUM_MCS_COUNTERS];
    /** SU TxBF TX NSS stats */
    A_UINT32 tx_su_txbf_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    /** Implicit BF TX NSS stats */
    A_UINT32 tx_su_ibf_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    /** Open loop TX NSS stats */
    A_UINT32 tx_su_ol_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    /** SU TxBF TX BW stats */
    A_UINT32 tx_su_txbf_bw[HTT_TX_TXBF_RATE_STATS_NUM_BW_COUNTERS];
    /** Implicit BF TX BW stats */
    A_UINT32 tx_su_ibf_bw[HTT_TX_TXBF_RATE_STATS_NUM_BW_COUNTERS];
    /** Open loop TX BW stats */
    A_UINT32 tx_su_ol_bw[HTT_TX_TXBF_RATE_STATS_NUM_BW_COUNTERS];
    /** Legacy and OFDM TX rate stats */
    A_UINT32 tx_legacy_ofdm_rate[HTT_TX_PDEV_STATS_NUM_LEGACY_OFDM_STATS];
    /** SU TxBF TX BW stats */
    A_UINT32 reduced_tx_su_txbf_bw[HTT_TX_TXBF_RATE_STATS_NUM_REDUCED_CHAN_TYPES][HTT_TX_TXBF_RATE_STATS_NUM_BW_COUNTERS];
    /** Implicit BF TX BW stats */
    A_UINT32 reduced_tx_su_ibf_bw[HTT_TX_TXBF_RATE_STATS_NUM_REDUCED_CHAN_TYPES][HTT_TX_TXBF_RATE_STATS_NUM_BW_COUNTERS];
    /** Open loop TX BW stats */
    A_UINT32 reduced_tx_su_ol_bw[HTT_TX_TXBF_RATE_STATS_NUM_REDUCED_CHAN_TYPES][HTT_TX_TXBF_RATE_STATS_NUM_BW_COUNTERS];
    /** Txbf flag reason stats */
    A_UINT32 txbf_flag_set_mu_mode;
    A_UINT32 txbf_flag_set_final_status;
    A_UINT32 txbf_flag_not_set_verified_txbf_mode;
    A_UINT32 txbf_flag_not_set_disable_p2p_access;
    A_UINT32 txbf_flag_not_set_max_nss_reached_in_he160;
    A_UINT32 txbf_flag_not_set_disable_ul_dl_ofdma;
    A_UINT32 txbf_flag_not_set_mcs_threshold_value;
    A_UINT32 txbf_flag_not_set_final_status;
} htt_stats_pdev_tx_rate_txbf_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_tx_rate_txbf_stats_tlv htt_tx_pdev_txbf_rate_stats_tlv;

typedef enum {
    HTT_STATS_RC_MODE_DLSU     = 0,
    HTT_STATS_RC_MODE_DLMUMIMO = 1,
    HTT_STATS_RC_MODE_DLOFDMA  = 2,
    HTT_STATS_RC_MODE_ULMUMIMO = 3,
    HTT_STATS_RC_MODE_ULOFDMA  = 4,
} htt_stats_rc_mode;

typedef struct {
    A_UINT32 ppdus_tried;
    A_UINT32 ppdus_ack_failed;
    A_UINT32 mpdus_tried;
    A_UINT32 mpdus_failed;
} htt_tx_rate_stats_t;

typedef enum {
    HTT_RC_MODE_SU_OL,
    HTT_RC_MODE_SU_BF,
    HTT_RC_MODE_MU1_INTF,
    HTT_RC_MODE_MU2_INTF,
    HTT_Rc_MODE_MU3_INTF,
    HTT_RC_MODE_MU4_INTF,
    HTT_RC_MODE_MU5_INTF,
    HTT_RC_MODE_MU6_INTF,
    HTT_RC_MODE_MU7_INTF,
    HTT_RC_MODE_2D_COUNT,
} HTT_RC_MODE;

typedef enum {
    HTT_STATS_RU_TYPE_INVALID             = 0,
    HTT_STATS_RU_TYPE_SINGLE_RU_ONLY      = 1,
    HTT_STATS_RU_TYPE_SINGLE_AND_MULTI_RU = 2,
} htt_stats_ru_type;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /** HTT_STATS_RC_MODE_XX */
    A_UINT32 rc_mode;

    A_UINT32 last_probed_mcs;

    A_UINT32 last_probed_nss;

    A_UINT32 last_probed_bw;

    htt_tx_rate_stats_t per_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];

    htt_tx_rate_stats_t per_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];

    htt_tx_rate_stats_t per_mcs[HTT_TX_TXBF_RATE_STATS_NUM_MCS_COUNTERS];

    /** 320MHz extension for PER */
    htt_tx_rate_stats_t per_bw320;

    A_UINT32 probe_cnt_per_rcmode[HTT_RC_MODE_2D_COUNT];

    A_UINT32 ru_type; /* refer to htt_stats_ru_type enum */
    htt_tx_rate_stats_t per_ru[HTT_TX_PDEV_STATS_NUM_BE_RU_SIZE_COUNTERS];
} htt_stats_per_rate_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_per_rate_stats_tlv htt_tx_rate_stats_per_tlv;

/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_pdev_tx_rate_txbf_stats_tlv txbf_rate_stats;
} htt_pdev_txbf_rate_stats_t;
#endif /* ATH_TARGET */

#ifdef ATH_TARGET
typedef struct {
    htt_stats_per_rate_stats_tlv per_stats;
} htt_tx_pdev_per_stats_t;
#endif /* ATH_TARGET */

typedef enum {
    HTT_ULTRIG_QBOOST_TRIGGER = 0,
    HTT_ULTRIG_PSPOLL_TRIGGER,
    HTT_ULTRIG_UAPSD_TRIGGER,
    HTT_ULTRIG_11AX_TRIGGER,
    HTT_ULTRIG_11AX_WILDCARD_TRIGGER,
    HTT_ULTRIG_11AX_UNASSOC_WILDCARD_TRIGGER,
    HTT_STA_UL_OFDMA_NUM_TRIG_TYPE,
} HTT_STA_UL_OFDMA_RX_TRIG_TYPE;

typedef enum {
    HTT_11AX_TRIGGER_BASIC_E                             = 0,
    HTT_11AX_TRIGGER_BRPOLL_E                            = 1,
    HTT_11AX_TRIGGER_MU_BAR_E                            = 2,
    HTT_11AX_TRIGGER_MU_RTS_E                            = 3,
    HTT_11AX_TRIGGER_BUFFER_SIZE_E                       = 4,
    HTT_11AX_TRIGGER_GCR_MU_BAR_E                        = 5,
    HTT_11AX_TRIGGER_BQRP_E                              = 6,
    HTT_11AX_TRIGGER_NDP_FB_REPORT_POLL_E                = 7,
    HTT_11AX_TRIGGER_RESERVED_8_E                        = 8,
    HTT_11AX_TRIGGER_RESERVED_9_E                        = 9,
    HTT_11AX_TRIGGER_RESERVED_10_E                       = 10,
    HTT_11AX_TRIGGER_RESERVED_11_E                       = 11,
    HTT_11AX_TRIGGER_RESERVED_12_E                       = 12,
    HTT_11AX_TRIGGER_RESERVED_13_E                       = 13,
    HTT_11AX_TRIGGER_RESERVED_14_E                       = 14,
    HTT_11AX_TRIGGER_RESERVED_15_E                       = 15,
    HTT_STA_UL_OFDMA_NUM_11AX_TRIG_TYPE,
} HTT_STA_UL_OFDMA_11AX_TRIG_TYPE;

/* UL RESP Queues 0 - HIPRI, 1 - LOPRI & 2 - BSR */
#define HTT_STA_UL_OFDMA_NUM_UL_QUEUES 3
/* Actual resp type sent by STA for trigger
 * 0 - HE TB PPDU, 1 - NULL Delimiter */
#define HTT_STA_UL_OFDMA_NUM_RESP_END_TYPE 2
/* Counter for MCS 0-13 */
#define HTT_STA_UL_OFDMA_NUM_MCS_COUNTERS 14
/* Counters BW 20,40,80,160,320 */
#define HTT_STA_UL_OFDMA_NUM_BW_COUNTERS 5
#define HTT_STA_UL_OFDMA_NUM_REDUCED_CHAN_TYPES 2 /* 0 - Half, 1 - Quarter */

/* STATS_TYPE : HTT_DBG_EXT_STA_11AX_UL_STATS
 * TLV_TAGS:
 *    - HTT_STATS_STA_UL_OFDMA_STATS_TAG
 */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 pdev_id;

    /**
     * Trigger Type reported by HWSCH on RX reception
     * Each index populate enum HTT_STA_UL_OFDMA_RX_TRIG_TYPE
     */
    A_UINT32 rx_trigger_type[HTT_STA_UL_OFDMA_NUM_TRIG_TYPE];
    /**
     * 11AX Trigger Type on RX reception
     * Each index populate enum HTT_STA_UL_OFDMA_11AX_TRIG_TYPE
     */
    A_UINT32 ax_trigger_type[HTT_STA_UL_OFDMA_NUM_11AX_TRIG_TYPE];

    /** Num data PPDUs/Delims responded to trigs. per HWQ for UL RESP */
    A_UINT32 num_data_ppdu_responded_per_hwq[HTT_STA_UL_OFDMA_NUM_UL_QUEUES];
    A_UINT32 num_null_delimiters_responded_per_hwq[HTT_STA_UL_OFDMA_NUM_UL_QUEUES];
    /**
     * Overall UL STA RESP Status 0 - HE TB PPDU, 1 - NULL Delimiter
     * Super set of num_data_ppdu_responded_per_hwq,
     * num_null_delimiters_responded_per_hwq
     */
    A_UINT32 num_total_trig_responses[HTT_STA_UL_OFDMA_NUM_RESP_END_TYPE];

    /**
     * Time interval between current time ms and last successful trigger RX
     * 0xFFFFFFFF denotes no trig received / timestamp roll back
     */
    A_UINT32 last_trig_rx_time_delta_ms;

    /**
     * Rate Statistics for UL OFDMA
     * UL TB PPDU TX MCS, NSS, GI, BW from STA HWQ
     */
    A_UINT32 ul_ofdma_tx_mcs[HTT_STA_UL_OFDMA_NUM_MCS_COUNTERS];
    A_UINT32 ul_ofdma_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 ul_ofdma_tx_gi[HTT_TX_PDEV_STATS_NUM_GI_COUNTERS][HTT_STA_UL_OFDMA_NUM_MCS_COUNTERS];
    A_UINT32 ul_ofdma_tx_ldpc;
    A_UINT32 ul_ofdma_tx_bw[HTT_STA_UL_OFDMA_NUM_BW_COUNTERS];

    /** Trig based PPDU TX/ RBO based PPDU TX Count */
    A_UINT32 trig_based_ppdu_tx;
    A_UINT32 rbo_based_ppdu_tx;
    /** Switch MU EDCA to SU EDCA Count */
    A_UINT32 mu_edca_to_su_edca_switch_count;
    /** Num MU EDCA applied Count */
    A_UINT32 num_mu_edca_param_apply_count;

    /**
     * Current MU EDCA Parameters for WMM ACs
     * Mode - 0 - SU EDCA, 1- MU EDCA
     */
    A_UINT32 current_edca_hwq_mode[HTT_NUM_AC_WMM];
    /** Contention Window minimum. Range: 1 - 10 */
    A_UINT32 current_cw_min[HTT_NUM_AC_WMM];
    /** Contention Window maximum. Range: 1 - 10 */
    A_UINT32 current_cw_max[HTT_NUM_AC_WMM];
    /** AIFS value - 0 -255 */
    A_UINT32 current_aifs[HTT_NUM_AC_WMM];
    A_UINT32 reduced_ul_ofdma_tx_bw[HTT_STA_UL_OFDMA_NUM_REDUCED_CHAN_TYPES][HTT_STA_UL_OFDMA_NUM_BW_COUNTERS];
} htt_stats_sta_ul_ofdma_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_sta_ul_ofdma_stats_tlv htt_sta_ul_ofdma_stats_tlv;

/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_sta_ul_ofdma_stats_tlv ul_ofdma_sta_stats;
} htt_sta_11ax_ul_stats_t;
#endif /* ATH_TARGET */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** No of Fine Timing Measurement frames transmitted successfully */
    A_UINT32 tx_ftm_suc;
    /**
     * No of Fine Timing Measurement frames transmitted successfully
     * after retry
      */
    A_UINT32 tx_ftm_suc_retry;
    /** No of Fine Timing Measurement frames not transmitted successfully */
    A_UINT32 tx_ftm_fail;
    /**
     * No of Fine Timing Measurement Request frames received,
     * including initial, non-initial, and duplicates
     */
    A_UINT32 rx_ftmr_cnt;
    /**
     * No of duplicate Fine Timing Measurement Request frames received,
     * including both initial and non-initial
      */
    A_UINT32 rx_ftmr_dup_cnt;
    /** No of initial Fine Timing Measurement Request frames received */
    A_UINT32 rx_iftmr_cnt;
    /**
     * No of duplicate initial Fine Timing Measurement Request frames received
     */
    A_UINT32 rx_iftmr_dup_cnt;
    /** No of responder sessions rejected when initiator was active */
    A_UINT32 initiator_active_responder_rejected_cnt;
    /** Responder terminate count */
    A_UINT32 responder_terminate_cnt;
    A_UINT32 vdev_id;
} htt_stats_vdev_rtt_resp_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_vdev_rtt_resp_stats_tlv htt_vdev_rtt_resp_stats_tlv;

#ifdef ATH_TARGET
typedef struct {
    htt_stats_vdev_rtt_resp_stats_tlv vdev_rtt_resp_stats;
} htt_vdev_rtt_resp_stats_t;
#endif /* ATH_TARGET */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    A_UINT32 vdev_id;
    /**
     * No of Fine Timing Measurement request frames transmitted successfully
     */
    A_UINT32 tx_ftmr_cnt;
    /**
     * No of Fine Timing Measurement request frames not transmitted successfully
     */
    A_UINT32 tx_ftmr_fail;
    /**
     * No of Fine Timing Measurement request frames transmitted successfully
     * after retry
     */
    A_UINT32 tx_ftmr_suc_retry;
    /**
     * No of Fine Timing Measurement frames received, including initial,
     * non-initial, and duplicates
     */
    A_UINT32 rx_ftm_cnt;
    /** Initiator Terminate count */
    A_UINT32 initiator_terminate_cnt;
    /** Debug count to check the Measurement request from host */
    A_UINT32 tx_meas_req_count;
} htt_stats_vdev_rtt_init_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_vdev_rtt_init_stats_tlv htt_vdev_rtt_init_stats_tlv;

#ifdef ATH_TARGET
typedef struct {
    htt_stats_vdev_rtt_init_stats_tlv vdev_rtt_init_stats;
} htt_vdev_rtt_init_stats_t;
#endif /* ATH_TARGET */


#define HTT_STATS_MAX_SCH_CMD_RESULT 25

/* TXSEND self generated frames */
typedef enum {
    HTT_TXSEND_FTYPE_SGEN_TF_POLL,
    HTT_TXSEND_FTYPE_SGEN_TF_SOUND,
    HTT_TXSEND_FTYPE_SGEN_TBR_NDPA,
    HTT_TXSEND_FTYPE_SGEN_TBR_NDP,
    HTT_TXSEND_FTYPE_SGEN_TBR_LMR,
    HTT_TXSEND_FTYPE_SGEN_TF_REPORT,

    HTT_TXSEND_FTYPE_MAX
}
htt_stats_txsend_ftype_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /* 11AZ TBR SU Stats */
    A_UINT32 tbr_su_ftype_queued[HTT_TXSEND_FTYPE_MAX];
    /* 11AZ TBR MU Stats */
    A_UINT32 tbr_mu_ftype_queued[HTT_TXSEND_FTYPE_MAX];
} htt_stats_pdev_rtt_tbr_selfgen_queued_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** tbr_num_sch_cmd_result_buckets:
     * Number of sch cmd results buckets in use per chip
     * Each bucket contains the counter of the number of times that bucket
     * index was seen in the sch_cmd_result. The last bucket will capture
     * the count of sch_cmd_result matching the last bucket index and the
     * count of all the sch_cmd_results that exceeded the last bucket index
     * value.
     * tbr_num_sch_cmd_result_buckets must be <= HTT_STATS_MAX_SCH_CMD_RESULT
     */
    A_UINT32 tbr_num_sch_cmd_result_buckets;
    /* cmd result status for SU frames in case of TB ranging */
    A_UINT32 opaque_tbr_su_ftype_cmd_result[HTT_TXSEND_FTYPE_MAX][HTT_STATS_MAX_SCH_CMD_RESULT];
    /* cmd result status for MU frames in case of TB ranging */
    A_UINT32 opaque_tbr_mu_ftype_cmd_result[HTT_TXSEND_FTYPE_MAX][HTT_STATS_MAX_SCH_CMD_RESULT];
} htt_stats_pdev_rtt_tbr_cmd_result_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** ista_ranging_ndpa_cnt:
     * Indicates the number of Ranging NDPA sent successfully.
     */
    A_UINT32 ista_ranging_ndpa_cnt;
    /** ista_ranging_ndp_cnt:
     * Indicates the number of Ranging NDP sent successfully.
     */
    A_UINT32 ista_ranging_ndp_cnt;
    /** ista_ranging_i2r_lmr_cnt:
     * Indicates the number of Ranging I2R LMR sent successfully.
     */
    A_UINT32 ista_ranging_i2r_lmr_cnt;
    /** rtsa_ranging_resp_cnt
     * Indicates the number of times RXPCU initiates a Ranging response
     * as a RSTA.
     */
    A_UINT32 rtsa_ranging_resp_cnt;
    /** rtsa_ranging_ndp_cnt:
     * Indicates the number of Ranging NDP response sent successfully.
     */
    A_UINT32 rtsa_ranging_ndp_cnt;
    /** rsta_ranging_lmr_cnt:
     * Indicates the number of Ranging R2I LMR response sent successfully.
     */
    A_UINT32 rsta_ranging_lmr_cnt;
    /** tb_ranging_cts2s_rcvd_cnt:
     * Indicates the number of expected CTS2S response received for TF Poll
     * sent.
     */
    A_UINT32 tb_ranging_cts2s_rcvd_cnt;
    /** tb_ranging_ndp_rcvd_cnt:
     * Indicates the number of expected NDP response received for TF Sound
     * or Secure Sound sent.
     */
    A_UINT32 tb_ranging_ndp_rcvd_cnt;
    /** tb_ranging_lmr_rcvd_cnt:
     * Indicates the number of expected LMR response received for TF Report
     * sent.
     */
    A_UINT32 tb_ranging_lmr_rcvd_cnt;
    /** tb_ranging_tf_poll_resp_sent_cnt:
     * Indicates the number of successful responses sent for TF Poll
     * received.
     */
    A_UINT32 tb_ranging_tf_poll_resp_sent_cnt;
    /** tb_ranging_tf_sound_resp_sent_cnt:
     * Indicates the number of successful responses sent for TF Sound
     * (or Secure) received.
     */
    A_UINT32 tb_ranging_tf_sound_resp_sent_cnt;
    /** tb_ranging_tf_report_resp_sent_cnt:
     * Indicates the number of successful responses sent for TF Report
     * received.
     */
    A_UINT32 tb_ranging_tf_report_resp_sent_cnt;
} htt_stats_pdev_rtt_hw_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 pdev_id;
    /** tx_11mc_ftm_suc:
     * Number of 11mc Fine Timing Measurement frames transmitted successfully.
     */
    A_UINT32 tx_11mc_ftm_suc;
    /** tx_11mc_ftm_suc_retry:
     * Number of Fine Timing Measurement frames transmitted successfully
     * after retrying.
     */
    A_UINT32 tx_11mc_ftm_suc_retry;
    /** tx_11mc_ftm_fail:
     * Number of Fine Timing Measurement frames not transmitted successfully.
     */
    A_UINT32 tx_11mc_ftm_fail;
    /** rx_11mc_ftmr_cnt:
     * Number of FTMR frames received, including initial, non-initial,
     * and duplicates.
     */
    A_UINT32 rx_11mc_ftmr_cnt;
    /** rx_11mc_ftmr_dup_cnt:
     * Number of duplicate Fine Timing Measurement Request frames received,
     * including both initial and non-initial.
     */
    A_UINT32 rx_11mc_ftmr_dup_cnt;
    /** rx_11mc_iftmr_cnt:
     * Number of initial Fine Timing Measurement Request frames received.
     */
    A_UINT32 rx_11mc_iftmr_cnt;
    /** rx_11mc_iftmr_dup_cnt:
     * Number of duplicate initial Fine Timing Measurement Request frames
     * received.
     */
    A_UINT32 rx_11mc_iftmr_dup_cnt;
    /** ftmr_drop_11mc_resp_role_not_enabled_cnt:
     * Number of FTMR frames dropped as 11mc is not supported for this VAP.
     */
    A_UINT32 ftmr_drop_11mc_resp_role_not_enabled_cnt;
    /** initiator_active_responder_rejected_cnt:
     * Number of responder sessions rejected when initiator was active.
     */
    A_UINT32 initiator_active_responder_rejected_cnt;
    /** responder_terminate_cnt:
     * Number of times Responder session got terminated.
     */
    A_UINT32 responder_terminate_cnt;
    /** active_rsta_open:
     * Number of active responder contexts in open mode.
     */
    A_UINT32 active_rsta_open;
    /** active_rsta_mac:
     * Number of active responder contexts in mac security mode.
     */
    A_UINT32 active_rsta_mac;
    /** active_rsta_mac_phy:
     * Number of active responder contexts in mac_phy security mode.
     */
    A_UINT32 active_rsta_mac_phy;
    /** num_assoc_ranging_peers:
     * Number of active associated ISTA ranging peers.
     */
    A_UINT32 num_assoc_ranging_peers;
    /** num_unassoc_ranging_peers:
     * Number of active un-associated ISTA ranging peers.
     */
    A_UINT32 num_unassoc_ranging_peers;
    /** responder_alloc_cnt:
     * Number of responder contexts allocated.
     */
    A_UINT32 responder_alloc_cnt;
    /** responder_alloc_failure:
     * Number of times responder context failed to be allocated.
     */
    A_UINT32 responder_alloc_failure;
    /** pn_check_failure_cnt:
     * Number of times PN check failed.
     */
    A_UINT32 pn_check_failure_cnt;
    /** pasn_m1_auth_recv_cnt:
     * Num of M1 auth frames received for PASN over the air from iSTA.
     */
    A_UINT32 pasn_m1_auth_recv_cnt;
    /** pasn_m1_auth_drop_cnt:
     * Number of M1 auth frames received for PASN over the air from iSTA
     * but dropped in FW due to any reason (such as unavailability of
     * responder ctxt or any other check).
     */
    A_UINT32 pasn_m1_auth_drop_cnt;
    /** pasn_m2_auth_recv_cnt:
     * Number of M2 auth frames received in FW for PASN from Host driver.
     */
    A_UINT32 pasn_m2_auth_recv_cnt;
    /** pasn_m2_auth_tx_fail_cnt:
     * Number of M2 auth frames received in FW but Tx failed.
     */
    A_UINT32 pasn_m2_auth_tx_fail_cnt;
    /** pasn_m3_auth_recv_cnt:
     * Number of M3 auth frames received for PASN.
     */
    A_UINT32 pasn_m3_auth_recv_cnt;
    /** pasn_m3_auth_drop_cnt:
     * Number of M3 auth frames received for PASN over the air from iSTA but
     * dropped in FW due to any reason.
     */
    A_UINT32 pasn_m3_auth_drop_cnt;
    /** pasn_peer_create_request_cnt:
     * Number of times FW requested PASN peer create request to Host.
     */
    A_UINT32 pasn_peer_create_request_cnt;
    /** pasn_peer_create_timeout_cnt:
     * Number of times PASN peer was not created within timeout period.
     */
    A_UINT32 pasn_peer_create_timeout_cnt;
    /** pasn_peer_created_cnt:
     * Number of times Host sent PASN peer create request to FW.
     */
    A_UINT32 pasn_peer_created_cnt;
    /** sec_ranging_not_supported_mfp_not_setup:
     * management frame protection not setup, drop secure ranging request.
     */
    A_UINT32 sec_ranging_not_supported_mfp_not_setup;
    /** non_sec_ranging_discarded_for_assoc_peer_with_mfpr_set:
     * Non secured ranging request discarded for Assoc peer with MFPR set.
     */
    A_UINT32 non_sec_ranging_discarded_for_assoc_peer_with_mfpr_set;
    /** open_ranging_discarded_with_URNM_MFPR_set_for_pasn_peer:
     * Failure in case non-secured frame is received for PASN peer and
     * URNM_MFPR is set.
     */
    A_UINT32 open_ranging_discarded_with_URNM_MFPR_set_for_pasn_peer;
    /** unassoc_non_pasn_ranging_not_supported_with_URNM_MFPR:
     * Failure in case non-assoc/non-PASN sta is sending open FTMR and
     * RSTA does not support un-secured ranging.
     */
    A_UINT32 unassoc_non_pasn_ranging_not_supported_with_URNM_MFPR;
    /** num_req_bw_20_MHz:
     * Number of requests with BW 20 MHz.
     */
    A_UINT32 num_req_bw_20_MHz;
    /** num_req_bw_40_MHz:
     * Number of requests with BW 40 MHz.
     */
    A_UINT32 num_req_bw_40_MHz;
    /** num_req_bw_80_MHz:
     * Number of requests with BW 80 MHz.
     */
    A_UINT32 num_req_bw_80_MHz;
    /** num_req_bw_160_MHz:
     * Number of requests with BW 160 MHz.
     */
    A_UINT32 num_req_bw_160_MHz;
    /** tx_11az_ftm_successful:
     * Number of 11AZ FTM frames transmitted successfully.
     */
    A_UINT32 tx_11az_ftm_successful;
    /** tx_11az_ftm_failed:
     * Number of 11AZ FTM frames for which Tx failed.
     */
    A_UINT32 tx_11az_ftm_failed;
    /** rx_11az_ftmr_cnt:
     * Number of 11AZ FTM frames received.
     */
    A_UINT32 rx_11az_ftmr_cnt;
    /** rx_11az_ftmr_dup_cnt:
     * Number of duplicate 11az ftmr frames dropped.
     */
    A_UINT32 rx_11az_ftmr_dup_cnt;
    /** rx_11az_iftmr_dup_cnt:
     * Number of duplicate 11az iftmr frames dropped.
     */
    A_UINT32 rx_11az_iftmr_dup_cnt;
    /** malformed_ftmr:
     * Number of malformed FTMR frames received from client leading to
     * frame parse error.
     */
    A_UINT32 malformed_ftmr;
    /** ftmr_drop_ntb_resp_role_not_enabled_cnt:
     * Number of FTMR frames dropped as NTB is not supported for this VAP.
     */
    A_UINT32 ftmr_drop_ntb_resp_role_not_enabled_cnt;
    /** ftmr_drop_tb_resp_role_not_enabled_cnt:
     * Number of FTMR frames dropped as TB is not supported for this VAP.
     */
    A_UINT32 ftmr_drop_tb_resp_role_not_enabled_cnt;
    /** invalid_ftm_request_params:
     * Number of FTMR frames received with invalid params.
     */
    A_UINT32 invalid_ftm_request_params;
    /** requested_bw_format_not_supported:
     * FTMR rejected as requested format is lower or higher than AP's
     * capability, or unknown.
     */
    A_UINT32 requested_bw_format_not_supported;
    /** ntb_unsec_unassoc_mode_ranging_peer_alloc_failed:
     * AST entry creation failed for NTB unsecured mode.
     */
    A_UINT32 ntb_unsec_unassoc_mode_ranging_peer_alloc_failed;
    /** tb_unassoc_unsec_mode_pasn_peer_creation_failed:
     * PASN peer creation failed for unsecured mode TBR.
     */
    A_UINT32 tb_unassoc_unsec_mode_pasn_peer_creation_failed;
    /** num_ranging_sequences_processed:
     * Number of ranging sequences processed for NTB and TB.
     */
    A_UINT32 num_ranging_sequences_processed;
    /** Number of NDPs transmitted for NTBR */
    A_UINT32 ntb_tx_ndp;
    A_UINT32 ndp_rx_cnt;
    /** Number of NDPAs received for 11AZ NTB ranging */
    A_UINT32 num_ntb_ranging_NDPAs_recv;
    /** Number of LMR frames received */
    A_UINT32 recv_lmr;
    /** invalid_ftmr_cnt:
     * Number of invalid FTMR frames received
     * iftmr with null ie element is invalid
     * The Frame is valid if any of the following combination is present:
     * a. LCI sub ie + parameter ie
     * b. LCR sub ie + parameter ie
     * c. parameter ie
     * d. LCI sub ie + LCR sub ie + parameter ie
     */
    A_UINT32 invalid_ftmr_cnt;
    /** Number of times the 'max time b/w measurement' timer got expired */
    A_UINT32 max_time_bw_meas_exp_cnt;
} htt_stats_pdev_rtt_resp_stats_tlv;

/* STATS_TYPE: HTT_DBG_EXT_PDEV_RTT_RESP_STATS
 * TLV_TAGS:
 *  HTT_STATS_PDEV_RTT_RESP_STATS_TAG
 *  HTT_STATS_PDEV_RTT_HW_STATS_TAG
 *  HTT_STATS_PDEV_RTT_TBR_SELFGEN_QUEUED_STATS_TAG
 *  HTT_STATS_PDEV_RTT_TBR_CMD_RESULT_STATS_TAG
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_pdev_rtt_resp_stats_tlv pdev_rtt_resp_stats;
    htt_stats_pdev_rtt_hw_stats_tlv pdev_rtt_hw_stats;
    htt_stats_pdev_rtt_tbr_selfgen_queued_stats_tlv pdev_rtt_tbr_selfgen_queued_stats;
    htt_stats_pdev_rtt_tbr_cmd_result_stats_tlv pdev_rtt_tbr_cmd_result_stats;
} htt_pdev_rtt_resp_stats_t;
#endif /* ATH_TARGET */

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 pdev_id;
    /** tx_11mc_ftmr_cnt:
     * Number of 11mc Fine Timing Measurement request frames transmitted
     * successfully.
     */
    A_UINT32 tx_11mc_ftmr_cnt;
    /** tx_11mc_ftmr_fail:
     * Number of 11mc Fine Timing Measurement request frames not transmitted
     * successfully.
     */
    A_UINT32 tx_11mc_ftmr_fail;
    /** tx_11mc_ftmr_suc_retry:
     * Number of 11mc Fine Timing Measurement request frames transmitted
     * successfully after retrying.
     */
    A_UINT32 tx_11mc_ftmr_suc_retry;
    /** rx_11mc_ftm_cnt:
     * Number of 11mc Fine Timing Measurement frames received, including
     * initial, non-initial, and duplicates.
     */
    A_UINT32 rx_11mc_ftm_cnt;
    /** Count of Ranging Measurement requests received from host */
    A_UINT32 tx_meas_req_count;
    /** Initiator role not supported on the vdev */
    A_UINT32 init_role_not_enabled;
    /** Number of times Initiator context got terminated */
    A_UINT32 initiator_terminate_cnt;
    /** Number of times Tx of FTMR failed */
    A_UINT32 tx_11az_ftmr_fail;
    /** tx_11az_ftmr_start:
     * Number of Fine Timing Measurement start requests transmitted
     * successfully.
     */
    A_UINT32 tx_11az_ftmr_start;
    /** tx_11az_ftmr_stop:
     * Number of Fine Timing Measurement stop requests transmitted
     * successfully.
     */
    A_UINT32 tx_11az_ftmr_stop;
    /** Number of FTM frames received successfully */
    A_UINT32 rx_11az_ftm_cnt;
    /** Number of active ISTA sessions */
    A_UINT32 active_ista;
    /** HE preamble not enabled on Initiator side */
    A_UINT32 invalid_preamble;
    /** Initiator invalid channel bw format */
    A_UINT32 invalid_chan_bw_format;
    /* mgmt_buff_alloc_fail_cnt Management Buffer allocation failure count */
    A_UINT32 mgmt_buff_alloc_fail_cnt;
    /** ftm_parse_failure:
     * Count of FTM frame IE parse failure or RSTA sending measurement
     * negotiation failure.
     */
    A_UINT32 ftm_parse_failure;
    /** Count of NTB/TB ranging negotiation completed successfully */
    A_UINT32 ranging_negotiation_successful_cnt;
    /** incompatible_ftm_params:
     * Number of occurrences of failure due to incompatible parameters
     * suggested by rSTA during negotiation.
     */
    A_UINT32 incompatible_ftm_params;
    /** sec_ranging_req_in_open_mode:
     * Number of occurrences of failure if BSS peer exists in open mode and
     * secured mode RTT ranging is requested.
     */
    A_UINT32 sec_ranging_req_in_open_mode;
    /** ftmr_tx_failed_null_11az_peer:
     * Number of occurrences where FTMR was not transmitted as there was
     * no 11AZ peer.
     */
    A_UINT32 ftmr_tx_failed_null_11az_peer;
    /** Number of times ftmr retry timed out */
    A_UINT32 ftmr_retry_timeout;
    /** Number of times the 'max time b/w measurement' timer got expired */
    A_UINT32 max_time_bw_meas_exp_cnt;
    /** tb_meas_duration_expiry_cnt:
     * Number of times TBR measurement duration expired.
     */
    A_UINT32 tb_meas_duration_expiry_cnt;
    /** num_tb_ranging_requests:
     * Number of TB ranging requests ready for negotiation.
     */
    A_UINT32 num_tb_ranging_requests;
    /** Number of times NTB ranging was triggered successfully */
    A_UINT32 ntbr_triggered_successfully;
    /** Number of times NTB ranging failed to be triggered */
    A_UINT32 ntbr_trigger_failed;
    /** No valid index found for programming vreg settings */
    A_UINT32 invalid_or_no_vreg_idx;
    /** Number of times VREG setting failed */
    A_UINT32 set_vreg_params_failed;
    /** Number of occurrences of SAC mismatch */
    A_UINT32 sac_mismatch;
    /** pasn_m1_auth_recv_cnt:
     * Number of M1 auth frames received for PASN from Host.
     */
    A_UINT32 pasn_m1_auth_recv_cnt;
    /** pasn_m1_auth_tx_fail_cnt:
     * Number of M1 auth frames received in FW but Tx failed.
     */
    A_UINT32 pasn_m1_auth_tx_fail_cnt;
    /** pasn_m2_auth_recv_cnt:
     * Number of M2 auth frames received in FW for PASN over the air from rSTA.
     */
    A_UINT32 pasn_m2_auth_recv_cnt;
    /** pasn_m2_auth_drop_cnt:
     * Number of M2 auth frames received in FW but dropped due to any reason.
     */
    A_UINT32 pasn_m2_auth_drop_cnt;
    /** pasn_m3_auth_recv_cnt:
     * Number of M3 auth frames received for PASN from Host.
     */
    A_UINT32 pasn_m3_auth_recv_cnt;
    /** pasn_m3_auth_tx_fail_cnt:
     * Number of M3 auth frames received in FW but Tx failed.
     */
    A_UINT32 pasn_m3_auth_tx_fail_cnt;
    /** pasn_peer_create_request_cnt:
     * Number of times FW requested PASN peer create request to Host.
     */
    A_UINT32 pasn_peer_create_request_cnt;
    /** pasn_peer_create_timeout_cnt:
     * Number of times PASN peer was not created within timeout period.
     */
    A_UINT32 pasn_peer_create_timeout_cnt;
    /** pasn_peer_created_cnt:
     * Number of times Host sent PASN peer create request to FW.
     */
    A_UINT32 pasn_peer_created_cnt;
    /** Number of occurrences of Tx of NDPA failing */
    A_UINT32 ntbr_ndpa_failed;
    /** ntbr_sequence_successful:
     * The NDPA, NDP and LMR exchanges are successful and sched cmd status
     * is 0.
     */
    A_UINT32 ntbr_sequence_successful;
    /** ntbr_ndp_failed:
     * Number of occurrences of NDPA being transmitted successfully
     * but NDP failing for NTB ranging.
     */
    A_UINT32 ntbr_ndp_failed;
    /** sch_cmd_status_cnts:
     * Elements 0-7 count the number of times the sch_cmd_status was equal to
     * the corresponding value of the index of the array sch_cmd_status_cnts[],
     * and element 8 counts the numbers of times the status was some other
     * value >=8.
     */
    A_UINT32 sch_cmd_status_cnts[9];
    /** Number of times LMR reception timed out */
    A_UINT32 lmr_timeout;
    /** Number of LMR frames received */
    A_UINT32 lmr_recv;
    /** Number of trigger frames received */
    A_UINT32 num_trigger_frames_received;
    /** Number of NDPAs received for TBR */
    A_UINT32 num_tb_ranging_NDPAs_recv;
    /** Number of ranging NDPs received for NTBR/TB */
    A_UINT32 ndp_rx_cnt;
} htt_stats_pdev_rtt_init_stats_tlv;

/* STATS_TYPE: HTT_DBG_EXT_PDEV_RTT_INITIATOR_STATS
 * TLV_TAGS:
 *  HTT_STATS_PDEV_RTT_INIT_STATS_TAG
 *  HTT_STATS_PDEV_RTT_HW_STATS_TAG
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_pdev_rtt_init_stats_tlv pdev_rtt_init_stats;
    htt_stats_pdev_rtt_hw_stats_tlv pdev_rtt_hw_stats;
} htt_pdev_rtt_init_stats_t;
#endif /* ATH_TARGET */


/* STATS_TYPE : HTT_DBG_EXT_PKTLOG_AND_HTT_RING_STATS
 * TLV_TAGS:
 *    - HTT_STATS_PKTLOG_AND_HTT_RING_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
typedef struct {
    htt_tlv_hdr_t   tlv_hdr;

    /** No of pktlog payloads that were dropped in htt_ppdu_stats path */
    A_UINT32 pktlog_lite_drop_cnt;
    /** No of pktlog payloads that were dropped in TQM path */
    A_UINT32 pktlog_tqm_drop_cnt;
    /** No of pktlog ppdu stats payloads that were dropped */
    A_UINT32 pktlog_ppdu_stats_drop_cnt;
    /** No of pktlog ppdu ctrl payloads that were dropped */
    A_UINT32 pktlog_ppdu_ctrl_drop_cnt;
    /** No of pktlog sw events payloads that were dropped */
    A_UINT32 pktlog_sw_events_drop_cnt;
} htt_stats_pktlog_and_htt_ring_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pktlog_and_htt_ring_stats_tlv
    htt_pktlog_and_htt_ring_stats_tlv;

#define HTT_DLPAGER_STATS_MAX_HIST            10
#define HTT_DLPAGER_ASYNC_LOCKED_PAGE_COUNT_M 0x000000FF
#define HTT_DLPAGER_ASYNC_LOCKED_PAGE_COUNT_S 0
#define HTT_DLPAGER_SYNC_LOCKED_PAGE_COUNT_M  0x0000FF00
#define HTT_DLPAGER_SYNC_LOCKED_PAGE_COUNT_S  8
#define HTT_DLPAGER_TOTAL_LOCKED_PAGES_M      0x0000FFFF
#define HTT_DLPAGER_TOTAL_LOCKED_PAGES_S      0
#define HTT_DLPAGER_TOTAL_FREE_PAGES_M        0xFFFF0000
#define HTT_DLPAGER_TOTAL_FREE_PAGES_S        16
#define HTT_DLPAGER_LAST_LOCKED_PAGE_IDX_M    0x0000FFFF
#define HTT_DLPAGER_LAST_LOCKED_PAGE_IDX_S    0
#define HTT_DLPAGER_LAST_UNLOCKED_PAGE_IDX_M  0xFFFF0000
#define HTT_DLPAGER_LAST_UNLOCKED_PAGE_IDX_S  16

#define HTT_DLPAGER_ASYNC_LOCK_PAGE_COUNT_GET(_var) \
    (((_var) & HTT_DLPAGER_ASYNC_LOCKED_PAGE_COUNT_M) >> \
     HTT_DLPAGER_ASYNC_LOCKED_PAGE_COUNT_S)
#define HTT_STATS_DLPAGER_STATS_DL_PAGER_STATS_ASYNC_LOCK_GET(_var) \
    HTT_DLPAGER_ASYNC_LOCK_PAGE_COUNT_GET(_var)


#define HTT_DLPAGER_ASYNC_LOCK_PAGE_COUNT_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_DLPAGER_ASYNC_LOCKED_PAGE_COUNT, _val); \
        ((_var) &= ~(HTT_DLPAGER_ASYNC_LOCKED_PAGE_COUNT_M));\
        ((_var) |= ((_val) << HTT_DLPAGER_ASYNC_LOCKED_PAGE_COUNT_S)); \
    } while (0)

#define HTT_DLPAGER_SYNC_LOCK_PAGE_COUNT_GET(_var) \
    (((_var) & HTT_DLPAGER_SYNC_LOCKED_PAGE_COUNT_M) >> \
     HTT_DLPAGER_SYNC_LOCKED_PAGE_COUNT_S)
#define HTT_STATS_DLPAGER_STATS_DL_PAGER_STATS_SYNC_LOCK_GET(_var) \
    HTT_DLPAGER_SYNC_LOCK_PAGE_COUNT_GET(_var)

#define HTT_DLPAGER_SYNC_LOCK_PAGE_COUNT_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_DLPAGER_SYNC_LOCKED_PAGE_COUNT, _val); \
        ((_var) &= ~(HTT_DLPAGER_SYNC_LOCKED_PAGE_COUNT_M));\
        ((_var) |= ((_val) << HTT_DLPAGER_SYNC_LOCKED_PAGE_COUNT_S)); \
    } while (0)

#define HTT_DLPAGER_TOTAL_LOCKED_PAGES_GET(_var) \
    (((_var) & HTT_DLPAGER_TOTAL_LOCKED_PAGES_M) >> \
     HTT_DLPAGER_TOTAL_LOCKED_PAGES_S)
#define HTT_STATS_DLPAGER_STATS_DL_PAGER_STATS_TOTAL_LOCKED_PAGES_GET(_var) \
    HTT_DLPAGER_TOTAL_LOCKED_PAGES_GET(_var)

#define HTT_DLPAGER_TOTAL_LOCKED_PAGES_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_DLPAGER_TOTAL_LOCKED_PAGES, _val); \
        ((_var) &= ~(HTT_DLPAGER_TOTAL_LOCKED_PAGES_M)); \
        ((_var) |= ((_val) << HTT_DLPAGER_TOTAL_LOCKED_PAGES_S)); \
    } while (0)

#define HTT_DLPAGER_TOTAL_FREE_PAGES_GET(_var) \
    (((_var) & HTT_DLPAGER_TOTAL_FREE_PAGES_M) >> \
     HTT_DLPAGER_TOTAL_FREE_PAGES_S)
#define HTT_STATS_DLPAGER_STATS_DL_PAGER_STATS_TOTAL_FREE_PAGES_GET(_var) \
    HTT_DLPAGER_TOTAL_FREE_PAGES_GET(_var)

#define HTT_DLPAGER_TOTAL_FREE_PAGES_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_DLPAGER_TOTAL_FREE_PAGES, _val); \
        ((_var) &= ~(HTT_DLPAGER_TOTAL_FREE_PAGES_M)); \
        ((_var) |= ((_val) << HTT_DLPAGER_TOTAL_FREE_PAGES_S)); \
    } while (0)

#define HTT_DLPAGER_LAST_LOCKED_PAGE_IDX_GET(_var) \
    (((_var) & HTT_DLPAGER_LAST_LOCKED_PAGE_IDX_M) >> \
     HTT_DLPAGER_LAST_LOCKED_PAGE_IDX_S)
#define HTT_STATS_DLPAGER_STATS_DL_PAGER_STATS_LAST_LOCKED_PAGE_IDX_GET(_var) \
    HTT_DLPAGER_LAST_LOCKED_PAGE_IDX_GET(_var)

#define HTT_DLPAGER_LAST_LOCKED_PAGE_IDX_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_DLPAGER_LAST_LOCKED_PAGE_IDX, _val); \
        ((_var) &= ~(HTT_DLPAGER_LAST_LOCKED_PAGE_IDX_M)); \
        ((_var) |= ((_val) << HTT_DLPAGER_LAST_LOCKED_PAGE_IDX_S)); \
    } while (0)

#define HTT_DLPAGER_LAST_UNLOCKED_PAGE_IDX_GET(_var) \
    (((_var) & HTT_DLPAGER_LAST_UNLOCKED_PAGE_IDX_M) >> \
     HTT_DLPAGER_LAST_UNLOCKED_PAGE_IDX_S)
#define HTT_STATS_DLPAGER_STATS_DL_PAGER_STATS_LAST_UNLOCKED_PAGE_IDX_GET(_var) \
     HTT_DLPAGER_LAST_UNLOCKED_PAGE_IDX_GET(_var)

#define HTT_DLPAGER_LAST_UNLOCKED_PAGE_IDX_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_DLPAGER_LAST_UNLOCKED_PAGE_IDX, _val); \
        ((_var) &= ~(HTT_DLPAGER_LAST_UNLOCKED_PAGE_IDX_M)); \
        ((_var) |= ((_val) << HTT_DLPAGER_LAST_UNLOCKED_PAGE_IDX_S)); \
    } while (0)

enum {
    HTT_STATS_PAGE_LOCKED = 0,
    HTT_STATS_PAGE_UNLOCKED = 1,
    HTT_STATS_NUM_PAGE_LOCK_STATES
};

/* dlPagerStats structure
 * Number of lock/unlock pages with last 10 lock/unlock occurrences are recorded */
typedef struct{
    /** msg_dword_1 bitfields:
     *     async_lock                 : 8,
     *     sync_lock                  : 8,
     *     reserved                   : 16;
     */
    union {
        struct {
            A_UINT32 async_lock: 8,
                     sync_lock: 8,
                     reserved1: 16;

        };
        A_UINT32     msg_dword_1;
    };
    /** mst_dword_2 bitfields:
     *     total_locked_pages         : 16,
     *     total_free_pages           : 16;
     */
    union {
        struct {
            A_UINT32 total_locked_pages: 16,
                     total_free_pages: 16;
        };
        A_UINT32     msg_dword_2;
    };
    /** msg_dword_3 bitfields:
     *     last_locked_page_idx       : 16,
     *     last_unlocked_page_idx     : 16;
     */
    union {
        struct {
            A_UINT32 last_locked_page_idx: 16,
                     last_unlocked_page_idx: 16;
        };
        A_UINT32     msg_dword_3;
    };

    struct {
        A_UINT32 page_num;
        A_UINT32 num_of_pages;
        /** timestamp is in microsecond units, from SoC timer clock */
        A_UINT32 timestamp_lsbs;
        A_UINT32 timestamp_msbs;
    } last_pages_info[HTT_STATS_NUM_PAGE_LOCK_STATES][HTT_DLPAGER_STATS_MAX_HIST];
} htt_dl_pager_stats_tlv;

/* NOTE:
 *  This structure is for documentation, and cannot be safely used directly.
 *  Instead, use the constituent TLV structures to fill/parse.
 *  STATS_TYPE : HTT_DBG_EXT_STATS_DLPAGER_STATS
 *  TLV_TAGS:
 *      - HTT_STATS_DLPAGER_STATS_TAG
 */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    htt_dl_pager_stats_tlv dl_pager_stats;
} htt_stats_dlpager_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_dlpager_stats_tlv htt_dlpager_stats_t;


/*======= PHY STATS ====================*/
/*
 * STATS TYPE : HTT_DBG_EXT_PHY_COUNTERS_AND_PHY_STATS
 * TLV_TAGS:
 *    - HTT_STATS_PHY_COUNTERS_TAG
 *    - HTT_STATS_PHY_STATS_TAG
 */

#define HTT_MAX_RX_PKT_CNT 8
#define HTT_MAX_RX_PKT_CRC_PASS_CNT 8
#define HTT_MAX_PER_BLK_ERR_CNT 20
#define HTT_MAX_RX_OTA_ERR_CNT 14
#define HTT_MAX_RX_PKT_CNT_EXT 4
#define HTT_MAX_RX_PKT_CRC_PASS_CNT_EXT 4
#define HTT_MAX_RX_PKT_MU_CNT 14
#define HTT_MAX_TX_PKT_CNT 10
#define HTT_MAX_PHY_TX_ABORT_CNT 10

typedef enum {
    HTT_STATS_CHANNEL_HALF_RATE          = 0x0001,   /* Half rate */
    HTT_STATS_CHANNEL_QUARTER_RATE       = 0x0002,   /* Quarter rate */
    HTT_STATS_CHANNEL_DFS                = 0x0004,   /* Enable radar event reporting */
    HTT_STATS_CHANNEL_HOME               = 0x0008,   /* Home channel */
    HTT_STATS_CHANNEL_PASSIVE_SCAN       = 0x0010,   /*Passive Scan */
    HTT_STATS_CHANNEL_DFS_SAP_NOT_UP     = 0x0020,   /* set when VDEV_START_REQUEST, clear when VDEV_UP */
    HTT_STATS_CHANNEL_PASSIVE_SCAN_CAL   = 0x0040,   /* need to do passive scan calibration to avoid "spikes" */
    HTT_STATS_CHANNEL_DFS_SAP_UP         = 0x0080,   /* DFS master */
    HTT_STATS_CHANNEL_DFS_CFREQ2         = 0x0100,   /* Enable radar event reporting for sec80 in VHT80p80 */
    HTT_STATS_CHANNEL_DTIM_SYNTH         = 0x0200,   /* Enable DTIM */
    HTT_STATS_CHANNEL_FORCE_GAIN         = 0x0400,   /* Force gain mmode (only used for FTM) */
    HTT_STATS_CHANNEL_PERFORM_NF_CAL     = 0x0800,   /* Perform NF cal in channel change (only used for FTM) */
    HTT_STATS_CHANNEL_165_MODE_0         = 0x1000,   /* 165 MHz mode 0 */
    HTT_STATS_CHANNEL_165_MODE_1         = 0x2000,   /* 165 MHz mode 1 */
    HTT_STATS_CHANNEL_165_MODE_2         = 0x3000,   /* 165 MHz mode 2 */
    HTT_STATS_CHANNEL_165_MODE_MASK      = 0x3000,   /* 165 MHz 2-bit mode mask */
} HTT_STATS_CHANNEL_FLAGS;

typedef enum {
    HTT_STATS_RF_MODE_MIN          = 0,
    HTT_STATS_RF_MODE_PHYA_ONLY    = 0,        // only PHYA is active
    HTT_STATS_RF_MODE_DBS          = 1,        // PHYA/5G and PHYB/2G
    HTT_STATS_RF_MODE_SBS          = 2,        // PHYA/5G and PHYB/5G in HL/NPR; PHYA0/5G and PHYA1/5G in HK
    HTT_STATS_RF_MODE_PHYB_ONLY    = 3,        // only PHYB is active
    HTT_STATS_RF_MODE_DBS_SBS      = 4,        // PHYA0/5G, PHYA1/5G and PHYB/2G in HK (the 2 5G are in different channel)
    HTT_STATS_RF_MODE_DBS_OR_SBS   = 5,        // PHYA0/5G, PHYA1/5G and PHYB/5G or 2G in HK
    HTT_STATS_RF_MODE_INVALID      = 0xff,
} HTT_STATS_RF_MODE;

typedef enum {
    HTT_STATS_RESET_CAUSE_FIRST_RESET      = 0x00000001, /* First reset by application */
    HTT_STATS_RESET_CAUSE_ERROR            = 0x00000002, /* Triggered due to error */
    HTT_STATS_RESET_CAUSE_DEEP_SLEEP       = 0x00000004, /* Reset after deep sleep */
    HTT_STATS_RESET_CAUSE_FULL_RESET       = 0x00000008, /* Full reset without any optimizations */
    HTT_STATS_RESET_CAUSE_CHANNEL_CHANGE   = 0x00000010, /* For normal channel change */
    HTT_STATS_RESET_CAUSE_BAND_CHANGE      = 0x00000020, /* Triggered due to band change */
    HTT_STATS_RESET_CAUSE_DO_CAL           = 0x00000040, /* Triggered due to calibrations */
    HTT_STATS_RESET_CAUSE_MCI_ERROR        = 0x00000080, /* Triggered due to MCI ERROR */
    HTT_STATS_RESET_CAUSE_CHWIDTH_CHANGE   = 0x00000100, /* Triggered due to channel width change */
    HTT_STATS_RESET_CAUSE_WARM_RESTORE_CAL = 0x00000200, /* Triggered due to warm reset we want to just restore calibrations */
    HTT_STATS_RESET_CAUSE_COLD_RESTORE_CAL = 0x00000400, /* Triggered due to cold reset we want to just restore calibrations */
    HTT_STATS_RESET_CAUSE_PHY_WARM_RESET   = 0x00000800, /* Triggered due to phy warm reset we want to just restore calibrations */
    HTT_STATS_RESET_CAUSE_M3_SSR           = 0x00001000, /* Triggered due to SSR Restart */
    HTT_STATS_RESET_CAUSE_FORCE_CAL        = 0x00002000, /* Reset to force the calibration */
    /* 0x00004000, 0x00008000 reserved */
    HTT_STATS_NO_RESET_CHANNEL_CHANGE      = 0x00010000, /* No reset, normal channel change */
    HTT_STATS_NO_RESET_BAND_CHANGE         = 0x00020000, /* No reset, channel change across band */
    HTT_STATS_NO_RESET_CHWIDTH_CHANGE      = 0x00040000, /* No reset, channel change across channel width */
    HTT_STATS_NO_RESET_CHAINMASK_CHANGE    = 0x00080000, /* No reset, chainmask change */
    HTT_STATS_RESET_CAUSE_PHY_WARM_RESET_UCODE_TRIG = 0x00100000, /* Triggered due to phy warm reset we want to just restore calibrations */
    HTT_STATS_RESET_CAUSE_PHY_OFF_TIMEOUT_RESET  = 0x00200000, /* Reset ucode because phy off ack timeout*/
    HTT_STATS_RESET_CAUSE_LMAC_RESET_UMAC_NOC_ERR = 0x00400000, /* LMAC reset triggered due to NOC Address/Slave error originating at LMAC */
    HTT_STATS_NO_RESET_SCAN_BACK_TO_SAME_HOME_CHANNEL_CHANGE = 0x00800000, /* No reset, scan to home channel change */
} HTT_STATS_RESET_CAUSE;

typedef enum {
    HTT_CHANNEL_RATE_FULL,
    HTT_CHANNEL_RATE_HALF,
    HTT_CHANNEL_RATE_QUARTER,

    HTT_CHANNEL_RATE_COUNT
} HTT_CHANNEL_RATE;

typedef enum {
    HTT_PHY_BW_IDX_20MHz    = 0,
    HTT_PHY_BW_IDX_40MHz    = 1,
    HTT_PHY_BW_IDX_80MHz    = 2,
    HTT_PHY_BW_IDX_80Plus80 = 3,
    HTT_PHY_BW_IDX_160MHz   = 4,
    HTT_PHY_BW_IDX_10MHz    = 5,
    HTT_PHY_BW_IDX_5MHz     = 6,
    HTT_PHY_BW_IDX_165MHz   = 7,

} HTT_PHY_BW_IDX;

typedef enum {
    HTT_WHAL_CONFIG_NONE                = 0x00000000,
    HTT_WHAL_CONFIG_NF_WAR              = 0x00000001,
    HTT_WHAL_CONFIG_CAL_WAR             = 0x00000002,
    HTT_WHAL_CONFIG_DO_NF_CAL           = 0x00000004,
    HTT_WHAL_CONFIG_SET_WAIT_FOR_NF_CAL = 0x00000008,
    HTT_WHAL_CONFIG_FORCED_TX_PWR       = 0x00000010,
    HTT_WHAL_CONFIG_FORCED_GAIN_IDX     = 0x00000020,
    HTT_WHAL_CONFIG_FORCED_PER_CHAIN    = 0x00000040,
} HTT_WHAL_CONFIG;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** number of RXTD OFDMA OTA error counts except power surge and drop */
    A_UINT32 rx_ofdma_timing_err_cnt;
    /** rx_cck_fail_cnt:
     * number of cck error counts due to rx reception failure because of
     * timing error in cck
     */
    A_UINT32 rx_cck_fail_cnt;
    /** number of times tx abort initiated by mac */
    A_UINT32 mactx_abort_cnt;
    /** number of times rx abort initiated by mac */
    A_UINT32 macrx_abort_cnt;
    /** number of times tx abort initiated by phy */
    A_UINT32 phytx_abort_cnt;
    /** number of times rx abort initiated by phy */
    A_UINT32 phyrx_abort_cnt;
    /** number of rx deferred count initiated by phy */
    A_UINT32 phyrx_defer_abort_cnt;
    /** number of sizing events generated at LSTF */
    A_UINT32 rx_gain_adj_lstf_event_cnt; /* a.k.a sizing1 */
    /** number of sizing events generated at non-legacy LTF */
    A_UINT32 rx_gain_adj_non_legacy_cnt; /* a.k.a sizing2 */
    /** rx_pkt_cnt -
     * Received EOP (end-of-packet) count per packet type;
     * [0] = 11a; [1] = 11b; [2] = 11n; [3] = 11ac; [4] = 11ax; [5] = GF
     * [6] = EHT; [7]=RSVD; [6] = Applicable only for BE
     */
    A_UINT32 rx_pkt_cnt[HTT_MAX_RX_PKT_CNT];
    /** rx_pkt_crc_pass_cnt -
     * Received EOP (end-of-packet) count per packet type;
     * [0] = 11a; [1] = 11b; [2] = 11n; [3] = 11ac; [4] = 11ax; [5] = GF
     * [6] = EHT; [7]=RSVD; [6] = Applicable only for BE
     */
    A_UINT32 rx_pkt_crc_pass_cnt[HTT_MAX_RX_PKT_CRC_PASS_CNT];
    /** per_blk_err_cnt -
     * Error count per error source;
     * [0] = unknown; [1] = LSIG; [2] = HTSIG; [3] = VHTSIG; [4] = HESIG;
     * [5] = RXTD_OTA; [6] = RXTD_FATAL; [7] = DEMF; [8] = ROBE;
     * [9] = PMI; [10] = TXFD; [11] = TXTD; [12] = PHYRF
     * [13-19]=RSVD
     */
    A_UINT32 per_blk_err_cnt[HTT_MAX_PER_BLK_ERR_CNT];
    /** rx_ota_err_cnt -
     * RXTD OTA (over-the-air) error count per error reason;
     * [0] = voting fail; [1] = weak det fail; [2] = strong sig fail;
     * [3] = cck fail; [4] = power surge; [5] = power drop;
     * [6] = btcf timing timeout error; [7] = btcf packet detect error;
     * [8] = coarse timing timeout error
     * [9-13]=RSVD
     */
    A_UINT32 rx_ota_err_cnt[HTT_MAX_RX_OTA_ERR_CNT];
    /** rx_pkt_cnt_ext -
     * Received EOP (end-of-packet) count per packet type for BE;
     * [0] = WUR; [1] = AZ; [2-3]=RVSD
     */
    A_UINT32 rx_pkt_cnt_ext[HTT_MAX_RX_PKT_CNT_EXT];
    /** rx_pkt_crc_pass_cnt_ext -
     * Received EOP (end-of-packet) count per packet type for BE;
     * [0] = WUR; [1] = AZ; [2-3]=RVSD
     */
    A_UINT32 rx_pkt_crc_pass_cnt_ext[HTT_MAX_RX_PKT_CRC_PASS_CNT_EXT];
    /** rx_pkt_mu_cnt -
     * RX MU MIMO+OFDMA packet count per packet type for BE;
     * [0] = 11ax OFDMA; [1] = 11ax OFDMA+MUMIMO; [2] = 11be OFDMA;
     * [3] = 11be OFDMA+MUMIMO; [4] = 11ax MIMO; [5] = 11be MIMO;
     * [6] = 11ax OFDMA; [7] = 11ax OFDMA+MUMIMO; [8] = 11be OFDMA;
     * [9] = 11be OFDMA+MUMIMO; [10] = 11ax MIMO; [11] = 11be MIMO;
     * [12-13]=RSVD
     */
    A_UINT32 rx_pkt_mu_cnt[HTT_MAX_RX_PKT_MU_CNT];
    /** tx_pkt_cnt -
     * num of transfered packet count per packet type;
     * [0] = 11a; [1] = 11b; [2] = 11n; [3] = 11ac; [4] = 11ax; [5] = GF;
     * [6]= EHT; [7] = WUR; [8] = AZ; [9]=RSVD; [6-8] = Applicable only for BE
     */
    A_UINT32 tx_pkt_cnt[HTT_MAX_TX_PKT_CNT];
    /** phy_tx_abort_cnt -
     * phy tx abort after each tlv;
     * [0] = PRE-PHY desc tlv; [1] = PHY desc tlv; [2] = LSIGA tlv;
     * [3] = LSIGB tlv; [4] = Per User tlv; [5] = HESIGB tlv;
     * [6] = Service tlv; [7] = Tx Packet End tlv; [8-9]=RSVD;
     */
    A_UINT32 phy_tx_abort_cnt[HTT_MAX_PHY_TX_ABORT_CNT];
} htt_stats_phy_counters_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_phy_counters_tlv htt_phy_counters_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** per chain hw noise floor values in dBm */
    A_INT32  nf_chain[HTT_STATS_MAX_CHAINS];
    /** number of false radars detected */
    A_UINT32 false_radar_cnt;
    /** number of channel switches happened due to radar detection */
    A_UINT32 radar_cs_cnt;
    /** ani_level -
     * ANI level (noise interference) corresponds to the channel
     * the desense levels range from -5 to 15 in dB units,
     * higher values indicating more noise interference.
     */
    A_INT32 ani_level;
    /** running time in minutes since FW boot */
    A_UINT32 fw_run_time;
    /** per chain runtime noise floor values in dBm */
    A_INT32 runTime_nf_chain[HTT_STATS_MAX_CHAINS];

    /** DFS SW based progressive stats - start **/

    /* current AP operating bandwidth (refer to WLAN_PHY_MODE) */
    A_UINT32 current_OBW;
    /* current AP device bandwidth (refer to WLAN_PHY_MODE) */
    A_UINT32 current_DBW;
    /* last_radar_type: last detected radar type
     * This last_radar_type field contains a value whose meaning is not
     * exposed to the host; this field is only provided for debug purposes.
     */
    A_UINT32 last_radar_type;
    /* dfs_reg_domain: curent DFS regulatory domain
     * This dfs_reg_domain field contains a value whose meaning is not
     * exposed to the host; this field is only provided for debug purposes.
     */
    A_UINT32 dfs_reg_domain;
    /* radar_mask_bit: Radar mask setting programmed in HW registers.
     * Each bit represents a 20 MHz portion of the channel.
     * Bit 0 represents the highest 20 MHz portion within the channel.
     * For example...
     * For a 80 MHz channel, bit0 = highest 20 MHz, bit3 = lowest 20 MHz
     * For a 320 MHz channel, bit0 = highest 20 MHz, bit15 = lowest 20 MHz
     */
    A_UINT32 radar_mask_bit;
    /* DFS radar rssi threshold (units = dBm) */
    A_INT32 radar_rssi;
    /* DFS global flags (refer to IEEE80211_CHAN_* defines) */
    A_UINT32 radar_dfs_flags;
    /* band center frequency of operating bandwidth (units = MHz) */
    A_UINT32 band_center_frequency_OBW;
    /* band center frequency of device bandwidth (units = MHz) */
    A_UINT32 band_center_frequency_DBW;

    /** DFS SW based progressive stats - end **/
} htt_stats_phy_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_phy_stats_tlv htt_phy_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** current pdev_id */
    A_UINT32 pdev_id;
    /** current channel information */
    A_UINT32 chan_mhz;
    /** center_freq1, center_freq2 in mhz */
    A_UINT32 chan_band_center_freq1;
    A_UINT32 chan_band_center_freq2;
    /** chan_phy_mode - WLAN_PHY_MODE enum type */
    A_UINT32 chan_phy_mode;
    /** chan_flags follows HTT_STATS_CHANNEL_FLAGS enum */
    A_UINT32  chan_flags;
    /** channel Num updated to virtual phybase */
    A_UINT32 chan_num;

    /** Cause for the phy reset - HTT_STATS_RESET_CAUSE */
    A_UINT32 reset_cause;
    /** Cause for the previous phy reset */
    A_UINT32 prev_reset_cause;
    /** source for the phywarm reset - HTT_STATS_RESET_CAUSE */
    A_UINT32 phy_warm_reset_src;
    /** rxGain Table selection mode - register settings
     * 0 - Auto, 1/2 - Forced with and without BT override respectively
     */
    A_UINT32 rx_gain_tbl_mode;
    /** current xbar value - perchain analog to digital idx mapping */
    A_UINT32 xbar_val;
    /** Flag to indicate forced calibration */
    A_UINT32 force_calibration;
    /** current RF mode (e.g. SBS/DBS) - follows HTT_STATS_RF_MODE enum */
    A_UINT32 phyrf_mode;

    /* PDL phyInput stats */
    /** homechannel flag
     * 1- Homechan, 0 - scan channel
     */
    A_UINT32 phy_homechan;
    /** Tx and Rx chainmask */
    A_UINT32 phy_tx_ch_mask;
    A_UINT32 phy_rx_ch_mask;
    /** INI masks - to decide the INI registers to be loaded on a reset */
    A_UINT32 phybb_ini_mask;
    A_UINT32 phyrf_ini_mask;

    /** DFS,ADFS/Spectral scan enable masks */
    A_UINT32 phy_dfs_en_mask;
    A_UINT32 phy_sscan_en_mask;
    A_UINT32 phy_synth_sel_mask;
    A_UINT32 phy_adfs_freq;

    /** CCK FIR settings
     * register settings - filter coefficients for Iqs conversion
     * [31:24] = FIR_COEFF_3_0
     * [23:16] = FIR_COEFF_2_0
     * [15:8]  = FIR_COEFF_1_0
     * [7:0]   = FIR_COEFF_0_0
     */
    A_UINT32 cck_fir_settings;
    /** dynamic primary channel index
     * primary 20MHz channel index on the current channel BW
     */
    A_UINT32  phy_dyn_pri_chan;

    /**
     * Current CCA detection threshold
     * dB above noisefloor req for CCA
     * Register settings for all subbands
     */
    A_UINT32 cca_thresh;
    /**
     * status for dynamic CCA adjustment
     * 0-disabled, 1-enabled
     */
    A_UINT32 dyn_cca_status;
    /** RXDEAF Register value
     * rxdesense_thresh_sw - VREG Register
     * rxdesense_thresh_hw - PHY Register
     */
    A_UINT32 rxdesense_thresh_sw;
    A_UINT32 rxdesense_thresh_hw;
    /** Current PHY Bandwidth -
     * values are specified by the HTT_PHY_BW_IDX enum type
     */
    A_UINT32 phy_bw_code;
    /** Current channel operating rate -
     * values are specified by the HTT_CHANNEL_RATE enum type
     */
    A_UINT32 phy_rate_mode;
    /** current channel operating band
     * 0 - 5G; 1 - 2G; 2 -6G
     */
    A_UINT32 phy_band_code;
    /** microcode processor virtual phy base address -
     * provided only for debug
     */
    A_UINT32 phy_vreg_base;
    /** microcode processor virtual phy base ext address -
     * provided only for debug
     */
    A_UINT32 phy_vreg_base_ext;
    /** HW LUT table configuration for home/scan channel -
     * provided only for debug
     */
    A_UINT32 cur_table_index;
    /** SW configuration flag for PHY reset and Calibrations -
     * values are specified by the HTT_WHAL_CONFIG enum type
     */
    A_UINT32 whal_config_flag;
    /** nfcal_iteration_counts:
     * iteration count for Home/Scan/Periodic Noise Floor calibrations
     * nfcal_iteration_counts[0] - home NF iteration counter
     * nfcal_iteration_counts[1] - scan NF iteration counter
     * nfcal_iteration_counts[2] - periodic NF iteration counter
     * These counters are not reset automatically; they are only reset
     * when explicitly requested by the host.
     */
    A_UINT32 nfcal_iteration_counts[3];
} htt_stats_phy_reset_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_phy_reset_stats_tlv htt_phy_reset_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /** current pdev_id */
    A_UINT32 pdev_id;
    /** ucode PHYOFF pass/failure count */
    A_UINT32 cf_active_low_fail_cnt;
    A_UINT32 cf_active_low_pass_cnt;

    /** PHYOFF count attempted through ucode VREG */
    A_UINT32 phy_off_through_vreg_cnt;

    /** Force calibration count */
    A_UINT32 force_calibration_cnt;

    /** phyoff count during rfmode switch */
    A_UINT32 rf_mode_switch_phy_off_cnt;

    /** Temperature based recalibration count */
    A_UINT32 temperature_recal_cnt;
} htt_stats_phy_reset_counters_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_phy_reset_counters_tlv htt_phy_reset_counters_tlv;

/* Considering 320 MHz maximum 16 power levels */
#define HTT_MAX_CH_PWR_INFO_SIZE    16

#define HTT_PHY_TPC_STATS_CTL_REGION_GRP_M    0x000000ff
#define HTT_PHY_TPC_STATS_CTL_REGION_GRP_S    0

#define HTT_PHY_TPC_STATS_CTL_REGION_GRP_GET(_var) \
    (((_var) & HTT_PHY_TPC_STATS_CTL_REGION_GRP_M) >> \
     HTT_PHY_TPC_STATS_CTL_REGION_GRP_S)
/* provide properly-named macro */
#define HTT_STATS_PHY_TPC_STATS_CTL_REGION_GRP_GET(_var) \
    HTT_PHY_TPC_STATS_CTL_REGION_GRP_GET(_var)
#define HTT_PHY_TPC_STATS_CTL_REGION_GRP_SET(_var, _val) \
  do { \
        HTT_CHECK_SET_VAL(HTT_PHY_TPC_STATS_CTL_REGION_GRP, _val); \
        ((_var) &= ~(HTT_PHY_TPC_STATS_CTL_REGION_GRP_M)); \
        ((_var) |= ((_val) << HTT_PHY_TPC_STATS_CTL_REGION_GRP_S)); \
     } while (0)

#define HTT_PHY_TPC_STATS_SUB_BAND_INDEX_M    0x0000ff00
#define HTT_PHY_TPC_STATS_SUB_BAND_INDEX_S    8

#define HTT_PHY_TPC_STATS_SUB_BAND_INDEX_GET(_var) \
    (((_var) & HTT_PHY_TPC_STATS_SUB_BAND_INDEX_M) >> \
     HTT_PHY_TPC_STATS_SUB_BAND_INDEX_S)
/* provide properly-named macro */
#define HTT_STATS_PHY_TPC_STATS_SUB_BAND_INDEX_GET(_var) \
    HTT_PHY_TPC_STATS_SUB_BAND_INDEX_GET(_var)
#define HTT_PHY_TPC_STATS_SUB_BAND_INDEX_SET(_var, _val) \
  do { \
        HTT_CHECK_SET_VAL(HTT_PHY_TPC_STATS_SUB_BAND_INDEX, _val); \
        ((_var) &= ~(HTT_PHY_TPC_STATS_SUB_BAND_INDEX_M)); \
        ((_var) |= ((_val) << HTT_PHY_TPC_STATS_SUB_BAND_INDEX_S)); \
     } while (0)

#define HTT_PHY_TPC_STATS_AG_CAP_EXT2_ENABLED_M    0x00ff0000
#define HTT_PHY_TPC_STATS_AG_CAP_EXT2_ENABLED_S    16

#define HTT_PHY_TPC_STATS_AG_CAP_EXT2_ENABLED_GET(_var) \
    (((_var) & HTT_PHY_TPC_STATS_AG_CAP_EXT2_ENABLED_M) >> \
     HTT_PHY_TPC_STATS_AG_CAP_EXT2_ENABLED_S)
/* provide properly-named macro */
#define HTT_STATS_PHY_TPC_STATS_ARRAY_GAIN_CAP_EXT2_ENABLED_GET(_var) \
    HTT_PHY_TPC_STATS_AG_CAP_EXT2_ENABLED_GET(_var)
#define HTT_PHY_TPC_STATS_AG_CAP_EXT2_ENABLED_SET(_var, _val) \
 do { \
        HTT_CHECK_SET_VAL(HTT_PHY_TPC_STATS_AG_CAP_EXT2_ENABLED, _val); \
        ((_var) &= ~(HTT_PHY_TPC_STATS_AG_CAP_EXT2_ENABLED_M)); \
        ((_var) |= ((_val) << HTT_PHY_TPC_STATS_AG_CAP_EXT2_ENABLED_S)); \
    } while (0)

#define HTT_PHY_TPC_STATS_CTL_FLAG_M    0xff000000
#define HTT_PHY_TPC_STATS_CTL_FLAG_S    24

#define HTT_PHY_TPC_STATS_CTL_FLAG_GET(_var) \
    (((_var) & HTT_PHY_TPC_STATS_CTL_FLAG_M) >> \
     HTT_PHY_TPC_STATS_CTL_FLAG_S)
/* provide properly-named macro */
#define HTT_STATS_PHY_TPC_STATS_CTL_FLAG_GET(_var) \
    HTT_PHY_TPC_STATS_CTL_FLAG_GET(_var)
#define HTT_PHY_TPC_STATS_CTL_FLAG_SET(_var, _val) \
 do { \
        HTT_CHECK_SET_VAL(HTT_PHY_TPC_STATS_CTL_FLAG, _val); \
        ((_var) &= ~(HTT_PHY_TPC_STATS_CTL_FLAG_M)); \
        ((_var) |= ((_val) << HTT_PHY_TPC_STATS_CTL_FLAG_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /** current pdev_id */
    A_UINT32 pdev_id;

    /** Tranmsit power control scaling related configurations */
    A_UINT32 tx_power_scale;
    A_UINT32 tx_power_scale_db;

    /** Minimum negative tx power supported by the target */
    A_INT32 min_negative_tx_power;

    /** current configured CTL domain */
    A_UINT32 reg_ctl_domain;

    /** Regulatory power information for the current channel */
    A_INT32 max_reg_allowed_power[HTT_STATS_MAX_CHAINS];
    A_INT32 max_reg_allowed_power_6g[HTT_STATS_MAX_CHAINS];
    /** channel max regulatory power in 0.5dB */
    A_UINT32 twice_max_rd_power;

    /** current channel and home channel's maximum possible tx power */
    A_INT32 max_tx_power;
    A_INT32 home_max_tx_power;

    /** channel's Power Spectral Density  */
    A_UINT32 psd_power;
    /** channel's EIRP power */
    A_UINT32 eirp_power;
    /** 6G channel power mode
     * 0-LPI, 1-SP, 2-VLPI and 3-SP_CLIENT power mode
     */
    A_UINT32 power_type_6ghz;

    /** sub-band channels and corresponding Tx-power */
    A_UINT32 sub_band_cfreq[HTT_MAX_CH_PWR_INFO_SIZE];
    A_UINT32 sub_band_txpower[HTT_MAX_CH_PWR_INFO_SIZE];

    /** array_gain_cap:
     * CTL Array Gain cap, units are dB
     * The lower-triangular portion of this square matrix is stored, i.e.
     *     array element 0 stores matrix element (0,0)
     *     array element 1 stores matrix element (1,0)
     *     array element 2 stores matrix element (1,1)
     *     array element 3 stores matrix element (2,0)
     *     ...
     *     array element 35 stores matrix element (7,7)
     */
    A_INT32 array_gain_cap[HTT_STATS_MAX_CHAINS * ((HTT_STATS_MAX_CHAINS/2)+1)];
    union {
        struct {
            A_UINT32
                ctl_region_grp:8, /** Group to which the ctl region belongs */
                sub_band_index:8, /** Frequency subband index */
                /** Array Gain Cap Ext2 feature enablement status */
                array_gain_cap_ext2_enabled:8,
                /** ctl_flag:
                 * 1st bit ULOFDMA supported
                 * 2nd bit DLOFDMA shared Exception supported
                 */
                ctl_flag:8;
        };
        A_UINT32 ctl_args;
    };
} htt_stats_phy_tpc_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_phy_tpc_stats_tlv htt_phy_tpc_stats_tlv;

/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_phy_counters_tlv phy_counters;
    htt_stats_phy_stats_tlv phy_stats;
    htt_stats_phy_reset_counters_tlv phy_reset_counters;
    htt_stats_phy_reset_stats_tlv phy_reset_stats;
    htt_stats_phy_tpc_stats_tlv phy_tpc_stats;
} htt_phy_counters_and_phy_stats_t;
#endif /* ATH_TARGET */

/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_soc_txrx_stats_common_tlv soc_common_stats;
    htt_stats_vdev_txrx_stats_hw_stats_tlv vdev_hw_stats[1/*or more*/];
} htt_vdevs_txrx_stats_t;
#endif /* ATH_TARGET */

typedef struct {
    union {
        A_UINT32 word32;
        struct {
            A_UINT32
                success: 16,
                fail:    16;
        };
    };
} htt_stats_strm_gen_mpdus_cntr_t;

typedef struct {
    /* MSDU queue identification */
    union {
        A_UINT32 word32;
        struct {
            A_UINT32
                peer_id:  16,
                tid:       4, /* only TIDs 0-7 actually expected to be used */
                htt_qtype: 4, /* refer to HTT_MSDUQ_INDEX */
                reserved:  8;
        };
    };
} htt_stats_strm_msdu_queue_id;

#define HTT_STATS_STRM_GEN_MPDUS_QUEUE_ID_PEER_ID_GET(word) \
    ((word >> 0) & 0xffff)
#define HTT_STATS_STRM_GEN_MPDUS_QUEUE_ID_TID_GET(word) \
    ((word >> 16) & 0xf)
#define HTT_STATS_STRM_GEN_MPDUS_QUEUE_ID_HTT_QTYPE_GET(word) \
    ((word >> 20) & 0xf)

#define HTT_STATS_STRM_GEN_MPDUS_SVC_INTERVAL_SUCCESS_GET(word) \
    ((word >> 0) & 0xffff)
#define HTT_STATS_STRM_GEN_MPDUS_SVC_INTERVAL_FAIL_GET(word) \
    ((word >> 16) & 0xffff)

#define HTT_STATS_STRM_GEN_MPDUS_BURST_SIZE_SUCCESS_GET(word) \
    ((word >> 0) & 0xffff)
#define HTT_STATS_STRM_GEN_MPDUS_BURST_SIZE_FAIL_GET(word) \
    ((word >> 16) & 0xffff)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    htt_stats_strm_msdu_queue_id queue_id;
    htt_stats_strm_gen_mpdus_cntr_t svc_interval;
    htt_stats_strm_gen_mpdus_cntr_t burst_size;
} htt_stats_strm_gen_mpdus_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_strm_gen_mpdus_tlv htt_stats_strm_gen_mpdus_tlv_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    htt_stats_strm_msdu_queue_id queue_id;
    struct {
        union {
            A_UINT32 timestamp_prior__timestamp_now__word;
            struct {
                A_UINT32
                    timestamp_prior_ms: 16,
                    timestamp_now_ms:   16;
            };
        };
        union {
            A_UINT32 interval_spec__margin__word;
            struct {
                A_UINT32
                    interval_spec_ms: 16,
                    margin_ms:        16;
            };
        };
    } svc_interval;
    struct {
        union {
            A_UINT32 consumed_bytes_orig__consumed_bytes_final__word;
            struct {
                A_UINT32
                    /* consumed_bytes_orig:
                     * Raw count (actually estimate) of how many bytes were
                     * removed from the MSDU queue by the GEN_MPDUS operation.
                     */
                    consumed_bytes_orig:  16,
                    /* consumed_bytes_final:
                     * Adjusted count of removed bytes that incorporates
                     * normalizing by the actual service interval compared to
                     * the expected service interval.
                     * This allows the burst size computation to be independent
                     * of whether the target is doing GEN_MPDUS at only the
                     * service interval, or substantially more often than the
                     * service interval.
                     *     consumed_bytes_final = consumed_bytes_orig /
                     *         (svc_interval / ref_svc_interval)
                     */
                    consumed_bytes_final: 16;
            };
        };
        union {
            A_UINT32 remaining_bytes__word;
            struct {
                A_UINT32
                    remaining_bytes: 16,
                    reserved:        16;
            };
        };
        union {
            A_UINT32 burst_size_spec__margin_bytes__word;
            struct {
                A_UINT32
                    burst_size_spec: 16,
                    margin_bytes:    16;
            };
        };
    } burst_size;
} htt_stats_strm_gen_mpdus_details_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_strm_gen_mpdus_details_tlv
    htt_stats_strm_gen_mpdus_details_tlv_t;

#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_QUEUE_ID_PEER_ID_GET(word) \
    ((word >> 0) & 0xffff)
#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_QUEUE_ID_TID_GET(word) \
    ((word >> 16) & 0xf)
#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_QUEUE_ID_HTT_QTYPE_GET(word) \
    ((word >> 20) & 0xf)

#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_SVC_INTERVAL_TIMESTAMP_PRIOR_MS_GET(word) \
    ((word >> 0) & 0xffff)
#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_SVC_INTERVAL_TIMESTAMP_NOW_MS_GET(word) \
    ((word >> 16) & 0xffff)

#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_SVC_INTERVAL_INTERVAL_SPEC_MS_GET(word) \
    ((word >> 0) & 0xffff)
#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_SVC_INTERVAL_MARGIN_MS_GET(word) \
    ((word >> 16) & 0xffff)

#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_BURST_SIZE_CONSUMED_BYTES_ORIG_GET(word) \
    ((word >> 0) & 0xffff)
#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_BURST_SIZE_CONSUMED_BYTES_FINAL_GET(word) \
    ((word >> 16) & 0xffff)
#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_BURST_SIZE_REMAINING_BYTES_GET(word) \
    ((word >> 0) & 0xffff)
#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_BURST_SIZE_BURST_SIZE_SPEC_GET(word) \
    ((word >> 0) & 0xffff)
#define HTT_STATS_STRM_GEN_MPDUS_DETAILS_BURST_SIZE_MARGIN_BYTES_GET(word) \
    ((word >> 16) & 0xffff)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32  reset_count;
    /** lower portion (bits 31:0)  of reset time, in milliseconds */
    A_UINT32  reset_time_lo_ms;
    /** upper portion (bits 63:32) of reset time, in milliseconds */
    A_UINT32  reset_time_hi_ms;
    /** lower portion (bits 31:0)  of disengage time, in milliseconds */
    A_UINT32  disengage_time_lo_ms;
    /** upper portion (bits 63:32) of disengage time, in milliseconds */
    A_UINT32  disengage_time_hi_ms;
    /** lower portion (bits 31:0)  of engage time, in milliseconds */
    A_UINT32  engage_time_lo_ms;
    /** upper portion (bits 63:32) of engage time, in milliseconds */
    A_UINT32  engage_time_hi_ms;
    A_UINT32  disengage_count;
    A_UINT32  engage_count;
    A_UINT32  drain_dest_ring_mask;
} htt_stats_dmac_reset_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_dmac_reset_stats_tlv htt_dmac_reset_stats_tlv;


/* Support up to 640 MHz mode for future expansion */
#define HTT_PUNCTURE_STATS_MAX_SUBBAND_COUNT 32

#define HTT_PDEV_PUNCTURE_STATS_MAC_ID_M 0x000000ff
#define HTT_PDEV_PUNCTURE_STATS_MAC_ID_S 0

#define HTT_PDEV_PUNCTURE_STATS_MAC_ID_GET(_var) \
    (((_var) & HTT_PDEV_PUNCTURE_STATS_MAC_ID_M) >> \
     HTT_PDEV_PUNCTURE_STATS_MAC_ID_S)

#define HTT_PDEV_PUNCTURE_STATS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_PDEV_PUNCTURE_STATS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_PDEV_PUNCTURE_STATS_MAC_ID_S)); \
    } while (0)

/*
 * TLV used to provide puncturing related stats for TX/RX and each PPDU type.
 */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /**
     * BIT [ 7 :  0]   :- mac_id
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
     * Stats direction (TX/RX). Enum value from HTT_STATS_DIRECTION.
     */
    A_UINT32 direction;

    /*
     * Preamble type. Enum value from HTT_STATS_PREAM_TYPE.
     *
     * Note that for although OFDM rates don't technically support
     * "puncturing", this TLV can be used to indicate the 20 MHz sub-bands
     * utilized for OFDM legacy duplicate packets, which are also used during
     * puncturing sequences.
     */
    A_UINT32 preamble;

    /*
     * Stats PPDU type. Enum value from HTT_STATS_PPDU_TYPE.
     */
    A_UINT32 ppdu_type;

    /*
     * Indicates the number of valid elements in the
     * "num_subbands_used_cnt" array, and must be <=
     * HTT_PUNCTURE_STATS_MAX_SUBBAND_COUNT.
     *
     * Also indicates how many bits in the last_used_pattern_mask may be
     * non-zero.
     */
    A_UINT32 subband_count;

    /*
     * The last used transmit 20 MHz subband mask. Bit 0 represents the lowest
     * 20 MHz subband mask, bit 1 the second lowest, and so on.
     *
     * All 32 bits are valid and will be used for expansion to higher BW modes.
     */
    A_UINT32 last_used_pattern_mask;


    /*
     * Number of array elements with valid values is equal to "subband_count".
     * If subband_count is < HTT_PUNCTURE_STATS_MAX_SUBBAND_COUNT, the
     * remaining elements will be implicitly set to 0x0.
     *
     * The array index is the number of 20 MHz subbands utilized during TX/RX,
     * and the counter value at that index is the number of times that subband
     * count was used.
     *
     * The count is incremented once for each OTA PPDU transmitted / received.
     */
    A_UINT32 num_subbands_used_cnt[HTT_PUNCTURE_STATS_MAX_SUBBAND_COUNT];
} htt_stats_pdev_puncture_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_puncture_stats_tlv htt_pdev_puncture_stats_tlv;

#define HTT_STATS_PDEV_PUNCTURE_STATS_MAC_ID_GET(word) ((word >> 0) & 0xff)

enum {
    HTT_STATS_CAL_PROF_COLD_BOOT = 0,
    HTT_STATS_CAL_PROF_FULL_CHAN_SWITCH = 1,
    HTT_STATS_CAL_PROF_SCAN_CHAN_SWITCH = 2,
    HTT_STATS_CAL_PROF_DPD_SPLIT_CAL = 3,

    HTT_STATS_MAX_PROF_CAL = 4,
};

#define HTT_STATS_MAX_CAL_IDX_CNT 8
typedef struct { /* DEPRECATED */

    htt_tlv_hdr_t tlv_hdr;

    A_UINT8 latency_prof_name[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_PROF_STATS_NAME_LEN];

    /** To verify whether prof cal is enabled or not */
    A_UINT32 enable;

    /** current pdev_id */
    A_UINT32 pdev_id;

    /** The cnt is incremented when each time the calindex takes place */
    A_UINT32 cnt[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT];

    /** Minimum time taken to complete the calibration - in us */
    A_UINT32 min[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT];

    /** Maximum time taken to complete the calibration -in us */
    A_UINT32 max[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT];

    /** Time taken by the cal for its final time execution - in us */
    A_UINT32 last[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT];

    /** Total time taken - in us */
    A_UINT32 tot[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT];

    /** hist_intvl - by default will be set to 2000 us */
    A_UINT32 hist_intvl[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT];

    /**
     * If last is less than hist_intvl, then hist[0]++,
     * If last is less than hist_intvl << 1, then hist[1]++,
     * otherwise hist[2]++.
     */
    A_UINT32 hist[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT][HTT_INTERRUPTS_LATENCY_PROFILE_MAX_HIST];

    /** Pf_last will log the current no of page faults */
    A_UINT32 pf_last[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT];

    /** Sum of all page faults happened */
    A_UINT32 pf_tot[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT];

    /** If pf_last > pf_max then pf_max = pf_last */
    A_UINT32 pf_max[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT];

    /**
     * For each cal profile, only certain no of cal indices were invoked,
     * this member will store what all the indices got invoked per each
     * cal profile
     */
    A_UINT32 enabledCalIdx[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT];

    /** No of indices invoked per each cal profile */
    A_UINT32 CalCnt[HTT_STATS_MAX_PROF_CAL];
} htt_stats_latency_prof_cal_stats_tlv; /* DEPRECATED */
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_latency_prof_cal_stats_tlv htt_latency_prof_cal_stats_tlv; /* DEPRECATED */

typedef struct {
    /** The cnt is incremented when each time the calindex takes place */
    A_UINT32 cnt;

    /** Minimum time taken to complete the calibration - in us */
    A_UINT32 min;

    /** Maximum time taken to complete the calibration -in us */
    A_UINT32 max;

    /** Time taken by the cal for its final time execution - in us */
    A_UINT32 last;

    /** Total time taken - in us */
    A_UINT32 tot;

    /** hist_intvl - in us, by default will be set to 2000 us */
    A_UINT32 hist_intvl;

    /**
     * If last is less than hist_intvl, then hist[0]++,
     * If last is less than hist_intvl << 1, then hist[1]++,
     * otherwise hist[2]++.
     */
    A_UINT32 hist[HTT_INTERRUPTS_LATENCY_PROFILE_MAX_HIST];

    /** pf_last will log the current no of page faults */
    A_UINT32 pf_last;

    /** Sum of all page faults happened */
    A_UINT32 pf_tot;

    /** If pf_last > pf_max then pf_max = pf_last */
    A_UINT32 pf_max;

    /**
     * For each cal profile, only certain no of cal indices were invoked,
     * this member will store what all the indices got invoked per each
     * cal profile
     */
    A_UINT32 enabled_cal_idx;

/*
 * NOTE: due to backwards-compatibility requirements,
 * no fields can be added to this struct.
 */
} htt_stats_latency_prof_cal_data;

typedef struct {

    htt_tlv_hdr_t tlv_hdr;

    /** To verify whether prof cal is enabled or not */
    A_UINT32 enable;

    /** current pdev_id */
    A_UINT32 pdev_id;

    /** No of indices invoked per each cal profile */
    A_UINT32 cal_cnt[HTT_STATS_MAX_PROF_CAL];

    /** Latency Cal Profile name */
    A_UINT8 latency_prof_name[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_PROF_STATS_NAME_LEN];

    /** Latency Cal data */
    htt_stats_latency_prof_cal_data latency_data[HTT_STATS_MAX_PROF_CAL][HTT_STATS_MAX_CAL_IDX_CNT];
} htt_stats_latency_prof_cal_data_tlv;

#define HTT_ML_PEER_EXT_DETAILS_PEER_ASSOC_IPC_RECVD_M          0x0000003F
#define HTT_ML_PEER_EXT_DETAILS_PEER_ASSOC_IPC_RECVD_S          0
#define HTT_ML_PEER_EXT_DETAILS_SCHED_PEER_DELETE_RECVD_M       0x00000FC0
#define HTT_ML_PEER_EXT_DETAILS_SCHED_PEER_DELETE_RECVD_S       6
#define HTT_ML_PEER_EXT_DETAILS_MLD_AST_INDEX_M                 0x0FFFF000
#define HTT_ML_PEER_EXT_DETAILS_MLD_AST_INDEX_S                 12

#define HTT_ML_PEER_EXT_DETAILS_PEER_ASSOC_IPC_RECVD_GET(_var) \
    (((_var) & HTT_ML_PEER_EXT_DETAILS_PEER_ASSOC_IPC_RECVD_M) >> \
     HTT_ML_PEER_EXT_DETAILS_PEER_ASSOC_IPC_RECVD_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_EXT_DETAILS_PEER_ASSOC_IPC_RECVD_GET(_var) \
    HTT_ML_PEER_EXT_DETAILS_PEER_ASSOC_IPC_RECVD_GET(_var)

#define HTT_ML_PEER_EXT_DETAILS_PEER_ASSOC_IPC_RECVD_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_EXT_DETAILS_PEER_ASSOC_IPC_RECVD, _val); \
        ((_var) &= ~(HTT_ML_PEER_EXT_DETAILS_PEER_ASSOC_IPC_RECVD_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_EXT_DETAILS_PEER_ASSOC_IPC_RECVD_S)); \
    } while (0)

#define HTT_ML_PEER_EXT_DETAILS_SCHED_PEER_DELETE_RECVD_GET(_var) \
    (((_var) & HTT_ML_PEER_EXT_DETAILS_SCHED_PEER_DELETE_RECVD_M) >> \
     HTT_ML_PEER_EXT_DETAILS_SCHED_PEER_DELETE_RECVD_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_EXT_DETAILS_SCHED_PEER_DELETE_RECVD_GET(_var) \
    HTT_ML_PEER_EXT_DETAILS_SCHED_PEER_DELETE_RECVD_GET(_var)

#define HTT_ML_PEER_EXT_DETAILS_SCHED_PEER_DELETE_RECVD_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_EXT_DETAILS_SCHED_PEER_DELETE_RECVD, _val); \
        ((_var) &= ~(HTT_ML_PEER_EXT_DETAILS_SCHED_PEER_DELETE_RECVD_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_EXT_DETAILS_SCHED_PEER_DELETE_RECVD_S)); \
    } while (0)

#define HTT_ML_PEER_EXT_DETAILS_MLD_AST_INDEX_GET(_var) \
    (((_var) & HTT_ML_PEER_EXT_DETAILS_MLD_AST_INDEX_M) >> \
     HTT_ML_PEER_EXT_DETAILS_MLD_AST_INDEX_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_EXT_DETAILS_MLD_AST_INDEX_GET(_var) \
    HTT_ML_PEER_EXT_DETAILS_MLD_AST_INDEX_GET(_var)

#define HTT_ML_PEER_EXT_DETAILS_MLD_AST_INDEX_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_EXT_DETAILS_MLD_AST_INDEX, _val); \
        ((_var) &= ~(HTT_ML_PEER_EXT_DETAILS_MLD_AST_INDEX_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_EXT_DETAILS_MLD_AST_INDEX_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    union {
        struct {
            A_UINT32 peer_assoc_ipc_recvd    : 6,
                     sched_peer_delete_recvd : 6,
                     mld_ast_index           : 16,
                     reserved                : 4;
        };
        A_UINT32 msg_dword_1;
    };
} htt_stats_ml_peer_ext_details_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_ml_peer_ext_details_tlv htt_ml_peer_ext_details_tlv;

#define HTT_ML_LINK_INFO_VALID_M                0x00000001
#define HTT_ML_LINK_INFO_VALID_S                0
#define HTT_ML_LINK_INFO_ACTIVE_M               0x00000002
#define HTT_ML_LINK_INFO_ACTIVE_S               1
#define HTT_ML_LINK_INFO_PRIMARY_M              0x00000004
#define HTT_ML_LINK_INFO_PRIMARY_S              2
#define HTT_ML_LINK_INFO_ASSOC_LINK_M           0x00000008
#define HTT_ML_LINK_INFO_ASSOC_LINK_S           3
#define HTT_ML_LINK_INFO_CHIP_ID_M              0x00000070
#define HTT_ML_LINK_INFO_CHIP_ID_S              4
#define HTT_ML_LINK_INFO_IEEE_LINK_ID_M         0x00007F80
#define HTT_ML_LINK_INFO_IEEE_LINK_ID_S         7
#define HTT_ML_LINK_INFO_HW_LINK_ID_M           0x00038000
#define HTT_ML_LINK_INFO_HW_LINK_ID_S           15
#define HTT_ML_LINK_INFO_LOGICAL_LINK_ID_M      0x000C0000
#define HTT_ML_LINK_INFO_LOGICAL_LINK_ID_S      18
#define HTT_ML_LINK_INFO_MASTER_LINK_M          0x00100000
#define HTT_ML_LINK_INFO_MASTER_LINK_S          20
#define HTT_ML_LINK_INFO_ANCHOR_LINK_M          0x00200000
#define HTT_ML_LINK_INFO_ANCHOR_LINK_S          21
#define HTT_ML_LINK_INFO_INITIALIZED_M          0x00400000
#define HTT_ML_LINK_INFO_INITIALIZED_S          22

#define HTT_ML_LINK_INFO_SW_PEER_ID_M           0x0000ffff
#define HTT_ML_LINK_INFO_SW_PEER_ID_S           0
#define HTT_ML_LINK_INFO_VDEV_ID_M              0x00ff0000
#define HTT_ML_LINK_INFO_VDEV_ID_S              16

#define HTT_ML_LINK_INFO_VALID_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_VALID_M) >> \
     HTT_ML_LINK_INFO_VALID_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_VALID_GET(_var) \
    HTT_ML_LINK_INFO_VALID_GET(_var)

#define HTT_ML_LINK_INFO_VALID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_VALID, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_VALID_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_VALID_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_ACTIVE_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_ACTIVE_M) >> \
     HTT_ML_LINK_INFO_ACTIVE_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_ACTIVE_GET(_var) \
    HTT_ML_LINK_INFO_ACTIVE_GET(_var)

#define HTT_ML_LINK_INFO_ACTIVE_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_ACTIVE, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_ACTIVE_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_ACTIVE_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_PRIMARY_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_PRIMARY_M) >> \
     HTT_ML_LINK_INFO_PRIMARY_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_PRIMARY_GET(_var) \
    HTT_ML_LINK_INFO_PRIMARY_GET(_var)

#define HTT_ML_LINK_INFO_PRIMARY_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_PRIMARY, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_PRIMARY_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_PRIMARY_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_ASSOC_LINK_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_ASSOC_LINK_M) >> \
     HTT_ML_LINK_INFO_ASSOC_LINK_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_ASSOC_LINK_GET(_var) \
    HTT_ML_LINK_INFO_ASSOC_LINK_GET(_var)

#define HTT_ML_LINK_INFO_ASSOC_LINK_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_ASSOC_LINK, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_ASSOC_LINK_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_ASSOC_LINK_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_CHIP_ID_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_CHIP_ID_M) >> \
     HTT_ML_LINK_INFO_CHIP_ID_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_CHIP_ID_GET(_var) \
    HTT_ML_LINK_INFO_CHIP_ID_GET(_var)

#define HTT_ML_LINK_INFO_CHIP_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_CHIP_ID, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_CHIP_ID_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_CHIP_ID_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_IEEE_LINK_ID_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_IEEE_LINK_ID_M) >> \
     HTT_ML_LINK_INFO_IEEE_LINK_ID_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_IEEE_LINK_ID_GET(_var) \
    HTT_ML_LINK_INFO_IEEE_LINK_ID_GET(_var)

#define HTT_ML_LINK_INFO_IEEE_LINK_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_IEEE_LINK_ID, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_IEEE_LINK_ID_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_IEEE_LINK_ID_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_HW_LINK_ID_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_HW_LINK_ID_M) >> \
     HTT_ML_LINK_INFO_HW_LINK_ID_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_HW_LINK_ID_GET(_var) \
    HTT_ML_LINK_INFO_HW_LINK_ID_GET(_var)

#define HTT_ML_LINK_INFO_HW_LINK_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_HW_LINK_ID, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_HW_LINK_ID_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_HW_LINK_ID_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_LOGICAL_LINK_ID_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_LOGICAL_LINK_ID_M) >> \
     HTT_ML_LINK_INFO_LOGICAL_LINK_ID_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_LOGICAL_LINK_ID_GET(_var) \
    HTT_ML_LINK_INFO_LOGICAL_LINK_ID_GET(_var)

#define HTT_ML_LINK_INFO_LOGICAL_LINK_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_LOGICAL_LINK_ID, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_LOGICAL_LINK_ID_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_LOGICAL_LINK_ID_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_MASTER_LINK_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_MASTER_LINK_M) >> \
     HTT_ML_LINK_INFO_MASTER_LINK_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_MASTER_LINK_GET(_var) \
    HTT_ML_LINK_INFO_MASTER_LINK_GET(_var)

#define HTT_ML_LINK_INFO_MASTER_LINK_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_MASTER_LINK, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_MASTER_LINK_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_MASTER_LINK_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_ANCHOR_LINK_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_ANCHOR_LINK_M) >> \
     HTT_ML_LINK_INFO_ANCHOR_LINK_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_ANCHOR_LINK_GET(_var) \
    HTT_ML_LINK_INFO_ANCHOR_LINK_GET(_var)

#define HTT_ML_LINK_INFO_ANCHOR_LINK_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_ANCHOR_LINK, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_ANCHOR_LINK_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_ANCHOR_LINK_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_INITIALIZED_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_INITIALIZED_M) >> \
     HTT_ML_LINK_INFO_INITIALIZED_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_INITIALIZED_GET(_var) \
    HTT_ML_LINK_INFO_INITIALIZED_GET(_var)

#define HTT_ML_LINK_INFO_INITIALIZED_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_INITIALIZED, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_INITIALIZED_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_INITIALIZED_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_SW_PEER_ID_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_SW_PEER_ID_M) >> \
     HTT_ML_LINK_INFO_SW_PEER_ID_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_SW_PEER_ID_GET(_var) \
    HTT_ML_LINK_INFO_SW_PEER_ID_GET(_var)

#define HTT_ML_LINK_INFO_SW_PEER_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_SW_PEER_ID, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_SW_PEER_ID_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_SW_PEER_ID_S)); \
    } while (0)

#define HTT_ML_LINK_INFO_VDEV_ID_GET(_var) \
    (((_var) & HTT_ML_LINK_INFO_VDEV_ID_M) >> \
     HTT_ML_LINK_INFO_VDEV_ID_S)
/* provide properly-named macro */
#define HTT_STATS_ML_LINK_INFO_DETAILS_VDEV_ID_GET(_var) \
    HTT_ML_LINK_INFO_VDEV_ID_GET(_var)

#define HTT_ML_LINK_INFO_VDEV_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_LINK_INFO_VDEV_ID, _val); \
        ((_var) &= ~(HTT_ML_LINK_INFO_VDEV_ID_M)); \
        ((_var) |= ((_val) << HTT_ML_LINK_INFO_VDEV_ID_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    union {
        struct {
            A_UINT32 valid           : 1,
                     active          : 1,
                     primary         : 1,
                     assoc_link      : 1,
                     chip_id         : 3,
                     ieee_link_id    : 8,
                     hw_link_id      : 3,
                     logical_link_id : 2,
                     master_link     : 1,
                     anchor_link     : 1,
                     initialized     : 1,
                     reserved        : 9;
        };
        A_UINT32 msg_dword_1;
    };

    union {
        struct {
            A_UINT32 sw_peer_id      : 16,
                     vdev_id         : 8,
                     reserved1       : 8;
        };
        A_UINT32 msg_dword_2;
    };

    A_UINT32 primary_tid_mask;
} htt_stats_ml_link_info_details_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_ml_link_info_details_tlv htt_ml_link_info_tlv;

#define HTT_ML_PEER_DETAILS_NUM_LINKS_M                     0x00000003
#define HTT_ML_PEER_DETAILS_NUM_LINKS_S                     0
#define HTT_ML_PEER_DETAILS_ML_PEER_ID_M                    0x00003FFC
#define HTT_ML_PEER_DETAILS_ML_PEER_ID_S                    2
#define HTT_ML_PEER_DETAILS_PRIMARY_LINK_IDX_M              0x0001C000
#define HTT_ML_PEER_DETAILS_PRIMARY_LINK_IDX_S              14
#define HTT_ML_PEER_DETAILS_PRIMARY_CHIP_ID_M               0x00060000
#define HTT_ML_PEER_DETAILS_PRIMARY_CHIP_ID_S               17
#define HTT_ML_PEER_DETAILS_LINK_INIT_COUNT_M               0x00380000
#define HTT_ML_PEER_DETAILS_LINK_INIT_COUNT_S               19
#define HTT_ML_PEER_DETAILS_NON_STR_M                       0x00400000
#define HTT_ML_PEER_DETAILS_NON_STR_S                       22
#define HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_M               0x00800000
#define HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_S               23
  /* for backwards compatibility, retain the old EMLSR name of the bitfield */
  #define HTT_ML_PEER_DETAILS_EMLSR_M HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_M
  #define HTT_ML_PEER_DETAILS_EMLSR_S HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_S
#define HTT_ML_PEER_DETAILS_IS_STA_KO_M                     0x01000000
#define HTT_ML_PEER_DETAILS_IS_STA_KO_S                     24
#define HTT_ML_PEER_DETAILS_NUM_LOCAL_LINKS_M               0x06000000
#define HTT_ML_PEER_DETAILS_NUM_LOCAL_LINKS_S               25
#define HTT_ML_PEER_DETAILS_ALLOCATED_M                     0x08000000
#define HTT_ML_PEER_DETAILS_ALLOCATED_S                     27
#define HTT_ML_PEER_DETAILS_EMLSR_SUPPORT_M                 0x10000000
#define HTT_ML_PEER_DETAILS_EMLSR_SUPPORT_S                 28

#define HTT_ML_PEER_DETAILS_PARTICIPATING_CHIPS_BITMAP_M    0x000000ff
#define HTT_ML_PEER_DETAILS_PARTICIPATING_CHIPS_BITMAP_S    0

#define HTT_ML_PEER_DETAILS_NUM_LINKS_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_NUM_LINKS_M) >> \
     HTT_ML_PEER_DETAILS_NUM_LINKS_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_NUM_LINKS_GET(_var) \
    HTT_ML_PEER_DETAILS_NUM_LINKS_GET(_var)

#define HTT_ML_PEER_DETAILS_NUM_LINKS_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_NUM_LINKS, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_NUM_LINKS_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_NUM_LINKS_S)); \
    } while (0)

#define HTT_ML_PEER_DETAILS_ML_PEER_ID_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_ML_PEER_ID_M) >> \
     HTT_ML_PEER_DETAILS_ML_PEER_ID_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_ML_PEER_ID_GET(_var) \
    HTT_ML_PEER_DETAILS_ML_PEER_ID_GET(_var)

#define HTT_ML_PEER_DETAILS_ML_PEER_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_ML_PEER_ID, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_ML_PEER_ID_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_ML_PEER_ID_S)); \
    } while (0)

#define HTT_ML_PEER_DETAILS_PRIMARY_LINK_IDX_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_PRIMARY_LINK_IDX_M) >> \
     HTT_ML_PEER_DETAILS_PRIMARY_LINK_IDX_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_PRIMARY_LINK_IDX_GET(_var) \
    HTT_ML_PEER_DETAILS_PRIMARY_LINK_IDX_GET(_var)

#define HTT_ML_PEER_DETAILS_PRIMARY_LINK_IDX_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_PRIMARY_LINK_IDX, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_PRIMARY_LINK_IDX_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_PRIMARY_LINK_IDX_S)); \
    } while (0)

#define HTT_ML_PEER_DETAILS_PRIMARY_CHIP_ID_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_PRIMARY_CHIP_ID_M) >> \
     HTT_ML_PEER_DETAILS_PRIMARY_CHIP_ID_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_PRIMARY_CHIP_ID_GET(_var) \
    HTT_ML_PEER_DETAILS_PRIMARY_CHIP_ID_GET(_var)

#define HTT_ML_PEER_DETAILS_PRIMARY_CHIP_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_PRIMARY_CHIP_ID, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_PRIMARY_CHIP_ID_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_PRIMARY_CHIP_ID_S)); \
    } while (0)

#define HTT_ML_PEER_DETAILS_LINK_INIT_COUNT_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_LINK_INIT_COUNT_M) >> \
     HTT_ML_PEER_DETAILS_LINK_INIT_COUNT_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_LINK_INIT_COUNT_GET(_var) \
    HTT_ML_PEER_DETAILS_LINK_INIT_COUNT_GET(_var)

#define HTT_ML_PEER_DETAILS_LINK_INIT_COUNT_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_LINK_INIT_COUNT, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_LINK_INIT_COUNT_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_LINK_INIT_COUNT_S)); \
    } while (0)

#define HTT_ML_PEER_DETAILS_NON_STR_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_NON_STR_M) >> \
     HTT_ML_PEER_DETAILS_NON_STR_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_NON_STR_GET(_var) \
    HTT_ML_PEER_DETAILS_NON_STR_GET(_var)

#define HTT_ML_PEER_DETAILS_NON_STR_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_NON_STR, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_NON_STR_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_NON_STR_S)); \
    } while (0)

#define HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_M) >> \
     HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_GET(_var) \
    HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_GET(_var)

#define HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_IS_EMLSR_ACTIVE_S)); \
    } while (0)

    /* start deprecated:
     * For backwards compatibility, retain a macro definition that uses
     * the old EMLSR name of the bitfield
     */
    #define HTT_ML_PEER_DETAILS_EMLSR_GET(_var) \
        (((_var) & HTT_ML_PEER_DETAILS_EMLSR_M) >> \
         HTT_ML_PEER_DETAILS_EMLSR_S)
    #define HTT_ML_PEER_DETAILS_EMLSR_SET(_var, _val) \
        do { \
            HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_EMLSR, _val); \
            ((_var) &= ~(HTT_ML_PEER_DETAILS_EMLSR_M)); \
            ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_EMLSR_S)); \
        } while (0)
    /* end deprecated */

#define HTT_ML_PEER_DETAILS_IS_STA_KO_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_IS_STA_KO_M) >> \
     HTT_ML_PEER_DETAILS_IS_STA_KO_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_IS_STA_KO_GET(_var) \
    HTT_ML_PEER_DETAILS_IS_STA_KO_GET(_var)

#define HTT_ML_PEER_DETAILS_IS_STA_KO_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_IS_STA_KO, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_IS_STA_KO_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_IS_STA_KO_S)); \
    } while (0)

#define HTT_ML_PEER_DETAILS_NUM_LOCAL_LINKS_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_NUM_LOCAL_LINKS_M) >> \
     HTT_ML_PEER_DETAILS_NUM_LOCAL_LINKS_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_NUM_LOCAL_LINKS_GET(_var) \
    HTT_ML_PEER_DETAILS_NUM_LOCAL_LINKS_GET(_var)

#define HTT_ML_PEER_DETAILS_NUM_LOCAL_LINKS_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_NUM_LOCAL_LINKS, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_NUM_LOCAL_LINKS_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_NUM_LOCAL_LINKS_S)); \
    } while (0)

#define HTT_ML_PEER_DETAILS_ALLOCATED_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_ALLOCATED_M) >> \
     HTT_ML_PEER_DETAILS_ALLOCATED_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_ALLOCATED_GET(_var) \
    HTT_ML_PEER_DETAILS_ALLOCATED_GET(_var)

#define HTT_ML_PEER_DETAILS_ALLOCATED_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_ALLOCATED, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_ALLOCATED_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_ALLOCATED_S)); \
    } while (0)

#define HTT_ML_PEER_DETAILS_EMLSR_SUPPORT_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_EMLSR_SUPPORT_M) >> \
     HTT_ML_PEER_DETAILS_EMLSR_SUPPORT_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_EMLSR_SUPPORT_GET(_var) \
    HTT_ML_PEER_DETAILS_EMLSR_SUPPORT_GET(_var)

#define HTT_ML_PEER_DETAILS_EMLSR_SUPPORT_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_EMLSR_SUPPORT, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_EMLSR_SUPPORT_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_EMLSR_SUPPORT_S)); \
    } while (0)


#define HTT_ML_PEER_DETAILS_PARTICIPATING_CHIPS_BITMAP_GET(_var) \
    (((_var) & HTT_ML_PEER_DETAILS_PARTICIPATING_CHIPS_BITMAP_M) >> \
     HTT_ML_PEER_DETAILS_PARTICIPATING_CHIPS_BITMAP_S)
/* provide properly-named macro */
#define HTT_STATS_ML_PEER_DETAILS_PARTICIPATING_CHIPS_BITMAP_GET(_var) \
    HTT_ML_PEER_DETAILS_PARTICIPATING_CHIPS_BITMAP_GET(_var)

#define HTT_ML_PEER_DETAILS_PARTICIPATING_CHIPS_BITMAP_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_ML_PEER_DETAILS_PARTICIPATING_CHIPS_BITMAP, _val); \
        ((_var) &= ~(HTT_ML_PEER_DETAILS_PARTICIPATING_CHIPS_BITMAP_M)); \
        ((_var) |= ((_val) << HTT_ML_PEER_DETAILS_PARTICIPATING_CHIPS_BITMAP_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    htt_mac_addr  remote_mld_mac_addr;
    union {
        struct {
            A_UINT32 num_links         : 2,
                     ml_peer_id        : 12,
                     primary_link_idx  : 3,
                     primary_chip_id   : 2,
                     link_init_count   : 3,
                     non_str           : 1,
                     is_emlsr_active   : 1,
                     is_sta_ko         : 1,
                     num_local_links   : 2,
                     allocated         : 1,
                     emlsr_support     : 1,
                     reserved          : 3;
        };
        struct {
            /*
             * For backwards compatibility, use a dummy union element to
             * retain the old "emlsr" name for the "is_emlsr_active" bitfield.
             */
            A_UINT32 dummy1 : 23,
                     emlsr  : 1,
                     dummy2 : 8;
        };
        A_UINT32 msg_dword_1;
    };

    union {
        struct {
            A_UINT32  participating_chips_bitmap : 8,
                     reserved1                  : 24;
        };
        A_UINT32 msg_dword_2;
    };
    /*
     * ml_peer_flags is an opaque field that cannot be interpreted by
     * the host; it is only for off-line debug.
     */
    A_UINT32  ml_peer_flags;
} htt_stats_ml_peer_details_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_ml_peer_details_tlv htt_ml_peer_details_tlv;

/* STATS_TYPE : HTT_DBG_EXT_STATS_ML_PEERS_INFO
 * TLV_TAGS:
 *   - HTT_STATS_ML_PEER_DETAILS_TAG
 *   - HTT_STATS_ML_LINK_INFO_DETAILS_TAG
 *   - HTT_STATS_ML_PEER_EXT_DETAILS_TAG (multiple)
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct _htt_ml_peer_stats {
    htt_stats_ml_peer_details_tlv      ml_peer_details;
    htt_stats_ml_peer_ext_details_tlv  ml_peer_ext_details;
    htt_stats_ml_link_info_details_tlv ml_link_info[1];
} htt_ml_peer_stats_t;
#endif /* ATH_TARGET */

/*
 * ODD Mandatory Stats are grouped together from all the existing different
 * stats, to form a set of stats that will be used by the ODD application to
 * post the stats to the cloud instead of polling for the individual stats.
 * This is done to avoid non-mandatory stats to be polled as the data will not
 * be required in the recipes derivation.
 * Rather than the host simply printing the ODD stats, the ODD application
 * will take the buffer and map it to the odd_mandatory_stats data structure.
 */
typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 hw_queued;
    A_UINT32 hw_reaped;
    A_UINT32 hw_paused;
    A_UINT32 hw_filt;
    A_UINT32 seq_posted;
    A_UINT32 seq_completed;
    A_UINT32 underrun;
    A_UINT32 hw_flush;
    A_UINT32 next_seq_posted_dsr;
    A_UINT32 seq_posted_isr;
    A_UINT32 mpdu_cnt_fcs_ok;
    A_UINT32 mpdu_cnt_fcs_err;
    A_UINT32 msdu_count_tqm;
    A_UINT32 mpdu_count_tqm;
    A_UINT32 mpdus_ack_failed;
    A_UINT32 num_data_ppdus_tried_ota;
    A_UINT32 ppdu_ok;
    A_UINT32 num_total_ppdus_tried_ota;
    A_UINT32 thermal_suspend_cnt;
    A_UINT32 dfs_suspend_cnt;
    A_UINT32 tx_abort_suspend_cnt;
    A_UINT32 suspended_txq_mask;
    A_UINT32 last_suspend_reason;
    A_UINT32 seq_failed_queueing;
    A_UINT32 seq_restarted;
    A_UINT32 seq_txop_repost_stop;
    A_UINT32 next_seq_cancel;
    A_UINT32 seq_min_msdu_repost_stop;
    A_UINT32 total_phy_err_cnt;
    A_UINT32 ppdu_recvd;
    A_UINT32 tcp_msdu_cnt;
    A_UINT32 tcp_ack_msdu_cnt;
    A_UINT32 udp_msdu_cnt;
    A_UINT32 fw_tx_mgmt_subtype[HTT_STATS_SUBTYPE_MAX];
    A_UINT32 fw_rx_mgmt_subtype[HTT_STATS_SUBTYPE_MAX];
    A_UINT32 fw_ring_mpdu_err[HTT_RX_STATS_RXDMA_MAX_ERR];
    A_UINT32 urrn_stats[HTT_TX_PDEV_MAX_URRN_STATS];
    A_UINT32 sifs_status[HTT_TX_PDEV_MAX_SIFS_BURST_STATS];
    A_UINT32 sifs_hist_status[HTT_TX_PDEV_SIFS_BURST_HIST_STATS];
    A_UINT32 rx_suspend_cnt;
    A_UINT32 rx_suspend_fail_cnt;
    A_UINT32 rx_resume_cnt;
    A_UINT32 rx_resume_fail_cnt;
    A_UINT32 hwq_beacon_cmd_result[HTT_TX_HWQ_MAX_CMD_RESULT_STATS];
    A_UINT32 hwq_voice_cmd_result[HTT_TX_HWQ_MAX_CMD_RESULT_STATS];
    A_UINT32 hwq_video_cmd_result[HTT_TX_HWQ_MAX_CMD_RESULT_STATS];
    A_UINT32 hwq_best_effort_cmd_result[HTT_TX_HWQ_MAX_CMD_RESULT_STATS];
    A_UINT32 hwq_beacon_mpdu_tried_cnt;
    A_UINT32 hwq_voice_mpdu_tried_cnt;
    A_UINT32 hwq_video_mpdu_tried_cnt;
    A_UINT32 hwq_best_effort_mpdu_tried_cnt;
    A_UINT32 hwq_beacon_mpdu_queued_cnt;
    A_UINT32 hwq_voice_mpdu_queued_cnt;
    A_UINT32 hwq_video_mpdu_queued_cnt;
    A_UINT32 hwq_best_effort_mpdu_queued_cnt;
    A_UINT32 hwq_beacon_mpdu_ack_fail_cnt;
    A_UINT32 hwq_voice_mpdu_ack_fail_cnt;
    A_UINT32 hwq_video_mpdu_ack_fail_cnt;
    A_UINT32 hwq_best_effort_mpdu_ack_fail_cnt;
    A_UINT32 pdev_resets;
    A_UINT32 phy_warm_reset;
    A_UINT32 hwsch_reset_count;
    A_UINT32 phy_warm_reset_ucode_trig;
    A_UINT32 mac_cold_reset;
    A_UINT32 mac_warm_reset;
    A_UINT32 mac_warm_reset_restore_cal;
    A_UINT32 phy_warm_reset_m3_ssr;
    A_UINT32 fw_rx_rings_reset;
    A_UINT32 tx_flush;
    A_UINT32 hwsch_dev_reset_war;
    A_UINT32 mac_cold_reset_restore_cal;
    A_UINT32 mac_only_reset;
    A_UINT32 mac_sfm_reset;
    A_UINT32 tx_ldpc; /* Number of tx PPDUs with LDPC coding */
    A_UINT32 rx_ldpc; /* Number of rx PPDUs with LDPC coding */
    A_UINT32 gen_mpdu_end_reason[HTT_TX_TQM_MAX_GEN_MPDU_END_REASON];
    A_UINT32 list_mpdu_end_reason[HTT_TX_TQM_MAX_LIST_MPDU_END_REASON];
    A_UINT32 tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS + HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS + HTT_TX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS];
    A_UINT32 tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 half_tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 quarter_tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 tx_su_punctured_mode[HTT_TX_PDEV_STATS_NUM_PUNCTURED_MODE_COUNTERS];
    A_UINT32 rx_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS + HTT_RX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS + HTT_RX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS];
    A_UINT32 rx_nss[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 rx_bw[HTT_RX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 rx_stbc[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS + HTT_RX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS + HTT_RX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS];
    A_UINT32 rts_cnt;
    A_UINT32 rts_success;
} htt_stats_odd_pdev_mandatory_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_odd_pdev_mandatory_tlv htt_odd_mandatory_pdev_stats_tlv;

typedef struct _htt_odd_mandatory_mumimo_pdev_stats_tlv {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 ac_mu_mimo_tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 ax_mu_mimo_tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 ac_mu_mimo_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 ax_mu_mimo_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 ac_mu_mimo_tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 ax_mu_mimo_tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 ul_mumimo_rx_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS];
    A_UINT32 ul_mumimo_rx_nss[HTT_RX_PDEV_STATS_ULMUMIMO_NUM_SPATIAL_STREAMS];
    A_UINT32 ul_mumimo_rx_bw[HTT_RX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 rx_ulmumimo_mpdu_fail[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];
    A_UINT32 rx_ulmumimo_mpdu_ok[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];
    A_UINT32 rx_ulmumimo_data_ppdu[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];
} htt_dbg_odd_mandatory_mumimo_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_dbg_odd_mandatory_mumimo_tlv
    htt_odd_mandatory_mumimo_pdev_stats_tlv;

typedef struct _htt_odd_mandatory_muofdma_pdev_stats_tlv {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 mu_ofdma_seq_posted;
    A_UINT32 ul_mu_ofdma_seq_posted;
    A_UINT32 ofdma_tx_mcs[HTT_TX_PDEV_STATS_NUM_MCS_COUNTERS + HTT_TX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS + HTT_TX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS];
    A_UINT32 ofdma_tx_nss[HTT_TX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 ofdma_tx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 ofdma_tx_ldpc;
    A_UINT32 ul_ofdma_rx_ldpc;
    A_UINT32 ul_ofdma_rx_mcs[HTT_RX_PDEV_STATS_NUM_MCS_COUNTERS + HTT_RX_PDEV_STATS_NUM_EXTRA_MCS_COUNTERS + HTT_RX_PDEV_STATS_NUM_EXTRA2_MCS_COUNTERS];
    A_UINT32 ul_ofdma_rx_nss[HTT_RX_PDEV_STATS_NUM_SPATIAL_STREAMS];
    A_UINT32 ul_ofdma_rx_bw[HTT_TX_PDEV_STATS_NUM_BW_COUNTERS];
    A_UINT32 rx_ulofdma_data_ppdu[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];
    A_UINT32 rx_ulofdma_mpdu_ok[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];
    A_UINT32 rx_ulofdma_mpdu_fail[HTT_RX_PDEV_MAX_OFDMA_NUM_USER];
    A_UINT32 ax_mu_brp_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    A_UINT32 be_mu_brp_sch_status[HTT_TX_PDEV_STATS_NUM_TX_ERR_STATUS];
    A_UINT32 ofdma_tx_ru_size[HTT_TX_PDEV_STATS_NUM_AX_RU_SIZE_COUNTERS];
} htt_dbg_odd_mandatory_muofdma_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_dbg_odd_mandatory_muofdma_tlv
    htt_odd_mandatory_muofdma_pdev_stats_tlv;


#define HTT_PDEV_SCHED_ALGO_OFDMA_STATS_MAC_ID_M 0x000000ff
#define HTT_PDEV_SCHED_ALGO_OFDMA_STATS_MAC_ID_S 0

#define HTT_PDEV_SCHED_ALGO_OFDMA_STATS_MAC_ID_GET(_var) \
    (((_var) & HTT_PDEV_SCHED_ALGO_OFDMA_STATS_MAC_ID_M) >> \
     HTT_PDEV_SCHED_ALGO_OFDMA_STATS_MAC_ID_S)

#define HTT_PDEV_SCHED_ALGO_OFDMA_STATS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_PDEV_SCHED_ALGO_OFDMA_STATS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_PDEV_SCHED_ALGO_OFDMA_STATS_MAC_ID_S)); \
    } while (0)

typedef enum {
    HTT_STATS_SCHED_OFDMA_TXBF = 0,                                    /* 0 */
    HTT_STATS_SCHED_OFDMA_TXBF_IS_SANITY_FAILED,                       /* 1 */
    HTT_STATS_SCHED_OFDMA_TXBF_IS_EBF_ALLOWED_FAILIED,                 /* 2 */
    HTT_STATS_SCHED_OFDMA_TXBF_RU_ALLOC_BW_DROP_COUNT,                 /* 3 */
    HTT_STATS_SCHED_OFDMA_TXBF_INVALID_CV_QUERY_COUNT,                 /* 4 */
    HTT_STATS_SCHED_OFDMA_TXBF_AVG_TXTIME_LESS_THAN_TXBF_SND_THERHOLD, /* 5 */
    HTT_STATS_SCHED_OFDMA_TXBF_IS_CANDIDATE_KICKED_OUT,                /* 6 */
    HTT_STATS_SCHED_OFDMA_TXBF_CV_IMAGE_BUF_INVALID,                   /* 7 */
    HTT_STATS_SCHED_OFDMA_TXBF_INELIGIBILITY_MAX,
} htt_stats_sched_ofdma_txbf_ineligibility_t;

#define HTT_MAX_NUM_CHAN_ACC_LAT_INTR 9

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * BIT [ 7 :  0]   :- mac_id
     * BIT [31 :  8]   :- reserved
     */
    union {
        struct {
            A_UINT32 mac_id:    8,
                     reserved: 24;
        };
        A_UINT32 mac_id__word;
    };

    /** Num of instances where rate based DL OFDMA status = ENABLED */
    A_UINT32 rate_based_dlofdma_enabled_count[HTT_NUM_AC_WMM];
    /** Num of instances where rate based DL OFDMA status = DISABLED */
    A_UINT32 rate_based_dlofdma_disabled_count[HTT_NUM_AC_WMM];
    /** Num of instances where rate based DL OFDMA status = PROBING */
    A_UINT32 rate_based_dlofdma_probing_count[HTT_NUM_AC_WMM];
    /** Num of instances where rate based DL OFDMA status = MONITORING */
    A_UINT32 rate_based_dlofdma_monitoring_count[HTT_NUM_AC_WMM];
    /** Num of instances where avg. channel access latency based DL OFDMA status = ENABLED */
    A_UINT32 chan_acc_lat_based_dlofdma_enabled_count[HTT_NUM_AC_WMM];
    /** Num of instances where avg. channel access latency based DL OFDMA status = DISABLED */
    A_UINT32 chan_acc_lat_based_dlofdma_disabled_count[HTT_NUM_AC_WMM];
    /** Num of instances where avg. channel access latency based DL OFDMA status = MONITORING */
    A_UINT32 chan_acc_lat_based_dlofdma_monitoring_count[HTT_NUM_AC_WMM];
    /** Num of instances where dl ofdma is disabled due to ru allocation failure */
    A_UINT32 downgrade_to_dl_su_ru_alloc_fail[HTT_NUM_AC_WMM];
    /** Num of instances where dl ofdma is disabled because we have only one user in candidate list */
    A_UINT32 candidate_list_single_user_disable_ofdma[HTT_NUM_AC_WMM];
    /** Num of instances where ul is chosen over dl based on qos weight not specific to OFDMA */
    A_UINT32 dl_cand_list_dropped_high_ul_qos_weight[HTT_NUM_AC_WMM];
    /** Num of instances where dl ofdma is disabled due to pipelining */
    A_UINT32 ax_dlofdma_disabled_due_to_pipelining[HTT_NUM_AC_WMM];
    /** Num of instances where dl ofdma is disabled as the tid is su only eligible */
    A_UINT32 dlofdma_disabled_su_only_eligible[HTT_NUM_AC_WMM];
    /** Num of instances where dl ofdma is disabled because there are no mpdus tried consecutively */
    A_UINT32 dlofdma_disabled_consec_no_mpdus_tried[HTT_NUM_AC_WMM];
    /** Num of instances where dl ofdma is disabled because there are consecutive mpdu failure */
    A_UINT32 dlofdma_disabled_consec_no_mpdus_success[HTT_NUM_AC_WMM];
    A_UINT32 txbf_ofdma_ineligibility_stat[HTT_STATS_SCHED_OFDMA_TXBF_INELIGIBILITY_MAX];
    /** Average channel access latency histogram stats
     *
     *  avg_chan_acc_lat_hist[0]: channel access latency is < 100 us
     *  avg_chan_acc_lat_hist[1]: 100 us <= channel access latency < 200 us
     *  avg_chan_acc_lat_hist[2]: 200 us <= channel access latency < 300 us
     *  avg_chan_acc_lat_hist[3]: 300 us <= channel access latency < 400 us
     *  avg_chan_acc_lat_hist[4]: 400 us <= channel access latency < 500 us
     *  avg_chan_acc_lat_hist[5]: 500 us <= channel access latency < 1000 us
     *  avg_chan_acc_lat_hist[6]: 1000 us <= channel access latency < 1500 us
     *  avg_chan_acc_lat_hist[7]: 1500 us <= channel access latency < 2000 us
     *  avg_chan_acc_lat_hist[8]: channel access latency is >= 2000 us
    */
    A_UINT32 avg_chan_acc_lat_hist[HTT_MAX_NUM_CHAN_ACC_LAT_INTR];
} htt_stats_pdev_sched_algo_ofdma_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_sched_algo_ofdma_stats_tlv
    htt_pdev_sched_algo_ofdma_stats_tlv;

#define HTT_STATS_PDEV_SCHED_ALGO_OFDMA_STATS_MAC_ID_GET(word) ((word >> 0) & 0xff)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /** mac_id__word:
     * BIT [ 7 :  0]   :- mac_id
     *                    Use the HTT_STATS_CMN_MAC_ID_GET,_SET macros to
     *                    read/write this bitfield.
     * BIT [31 :  8]   :- reserved
     */
    A_UINT32 mac_id__word;
    A_UINT32 basic_trigger_across_bss;
    A_UINT32 basic_trigger_within_bss;
    A_UINT32 bsr_trigger_across_bss;
    A_UINT32 bsr_trigger_within_bss;
    A_UINT32 mu_rts_across_bss;
    A_UINT32 mu_rts_within_bss;
    A_UINT32 ul_mumimo_trigger_across_bss;
    A_UINT32 ul_mumimo_trigger_within_bss;
} htt_stats_pdev_mbssid_ctrl_frame_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_mbssid_ctrl_frame_stats_tlv
    htt_pdev_mbssid_ctrl_frame_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    /**
     * BIT [ 7 :  0]   :- mac_id
     *                    Use the HTT_STATS_TDMA_MAC_ID_GET macro to extract
     *                    this bitfield.
     * BIT [31 :  8]   :- reserved
     */
    union {
        struct {
            A_UINT32 mac_id:    8,
                     reserved: 24;
        };
        A_UINT32 mac_id__word;
    };

    /** Num of Active TDMA schedules */
    A_UINT32 num_tdma_active_schedules;
    /** Num of Reserved TDMA schedules */
    A_UINT32 num_tdma_reserved_schedules;
    /** Num of Restricted TDMA schedules */
    A_UINT32 num_tdma_restricted_schedules;
    /** Num of Unconfigured TDMA schedules */
    A_UINT32 num_tdma_unconfigured_schedules;
    /** Num of TDMA slot switches */
    A_UINT32 num_tdma_slot_switches;
    /** Num of TDMA EDCA switches */
    A_UINT32 num_tdma_edca_switches;
} htt_stats_pdev_tdma_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_tdma_tlv htt_pdev_tdma_stats_tlv;

#define HTT_STATS_TDMA_MAC_ID_M 0x000000ff
#define HTT_STATS_TDMA_MAC_ID_S 0

#define HTT_STATS_TDMA_MAC_ID_GET(_var) \
    (((_var) & HTT_STATS_TDMA_MAC_ID_M) >> \
     HTT_STATS_TDMA_MAC_ID_S)
/* provide properly-named macro */
#define HTT_STATS_PDEV_TDMA_MAC_ID_GET(_var) \
    HTT_STATS_TDMA_MAC_ID_GET(_var)

/*======= Bandwidth Manager stats ====================*/

#define HTT_BW_MGR_STATS_MAC_ID_M               0x000000ff
#define HTT_BW_MGR_STATS_MAC_ID_S               0

#define HTT_BW_MGR_STATS_PRI20_IDX_M            0x0000ff00
#define HTT_BW_MGR_STATS_PRI20_IDX_S            8

#define HTT_BW_MGR_STATS_PRI20_FREQ_M           0xffff0000
#define HTT_BW_MGR_STATS_PRI20_FREQ_S           16

#define HTT_BW_MGR_STATS_CENTER_FREQ1_M         0x0000ffff
#define HTT_BW_MGR_STATS_CENTER_FREQ1_S         0

#define HTT_BW_MGR_STATS_CENTER_FREQ2_M         0xffff0000
#define HTT_BW_MGR_STATS_CENTER_FREQ2_S         16

#define HTT_BW_MGR_STATS_CHAN_PHY_MODE_M        0x000000ff
#define HTT_BW_MGR_STATS_CHAN_PHY_MODE_S        0

#define HTT_BW_MGR_STATS_STATIC_PATTERN_M       0x00ffff00
#define HTT_BW_MGR_STATS_STATIC_PATTERN_S       8

#define HTT_BW_MGR_STATS_MAC_ID_GET(_var) \
    (((_var) & HTT_BW_MGR_STATS_MAC_ID_M) >> \
     HTT_BW_MGR_STATS_MAC_ID_S)

#define HTT_BW_MGR_STATS_MAC_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_BW_MGR_STATS_MAC_ID, _val); \
        ((_var) |= ((_val) << HTT_BW_MGR_STATS_MAC_ID_S)); \
    } while (0)


#define HTT_BW_MGR_STATS_PRI20_IDX_GET(_var) \
    (((_var) & HTT_BW_MGR_STATS_PRI20_IDX_M) >> \
     HTT_BW_MGR_STATS_PRI20_IDX_S)

#define HTT_BW_MGR_STATS_PRI20_IDX_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_BW_MGR_STATS_PRI20_IDX, _val); \
        ((_var) |= ((_val) << HTT_BW_MGR_STATS_PRI20_IDX_S)); \
    } while (0)


#define HTT_BW_MGR_STATS_PRI20_FREQ_GET(_var) \
    (((_var) & HTT_BW_MGR_STATS_PRI20_FREQ_M) >> \
     HTT_BW_MGR_STATS_PRI20_FREQ_S)

#define HTT_BW_MGR_STATS_PRI20_FREQ_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_BW_MGR_STATS_PRI20_FREQ, _val); \
        ((_var) |= ((_val) << HTT_BW_MGR_STATS_PRI20_FREQ_S)); \
    } while (0)


#define HTT_BW_MGR_STATS_CENTER_FREQ1_GET(_var) \
    (((_var) & HTT_BW_MGR_STATS_CENTER_FREQ1_M) >> \
     HTT_BW_MGR_STATS_CENTER_FREQ1_S)

#define HTT_BW_MGR_STATS_CENTER_FREQ1_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_BW_MGR_STATS_CENTER_FREQ1, _val); \
        ((_var) |= ((_val) << HTT_BW_MGR_STATS_CENTER_FREQ1_S)); \
    } while (0)


#define HTT_BW_MGR_STATS_CENTER_FREQ2_GET(_var) \
    (((_var) & HTT_BW_MGR_STATS_CENTER_FREQ2_M) >> \
     HTT_BW_MGR_STATS_CENTER_FREQ2_S)

#define HTT_BW_MGR_STATS_CENTER_FREQ2_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_BW_MGR_STATS_CENTER_FREQ2, _val); \
        ((_var) |= ((_val) << HTT_BW_MGR_STATS_CENTER_FREQ2_S)); \
    } while (0)


#define HTT_BW_MGR_STATS_CHAN_PHY_MODE_GET(_var) \
    (((_var) & HTT_BW_MGR_STATS_CHAN_PHY_MODE_M) >> \
     HTT_BW_MGR_STATS_CHAN_PHY_MODE_S)

#define HTT_BW_MGR_STATS_CHAN_PHY_MODE_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_BW_MGR_STATS_CHAN_PHY_MODE, _val); \
        ((_var) |= ((_val) << HTT_BW_MGR_STATS_CHAN_PHY_MODE_S)); \
    } while (0)


#define HTT_BW_MGR_STATS_STATIC_PATTERN_GET(_var) \
    (((_var) & HTT_BW_MGR_STATS_STATIC_PATTERN_M) >> \
     HTT_BW_MGR_STATS_STATIC_PATTERN_S)

#define HTT_BW_MGR_STATS_STATIC_PATTERN_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_BW_MGR_STATS_STATIC_PATTERN, _val); \
        ((_var) |= ((_val) << HTT_BW_MGR_STATS_STATIC_PATTERN_S)); \
    } while (0)


typedef struct {
    htt_tlv_hdr_t tlv_hdr;

    /* BIT [ 7  :  0]  :- mac_id
     * BIT [ 15 :  8]  :- pri20_index
     * BIT [ 31 : 16]  :- pri20_freq in Mhz
     */
    A_UINT32 mac_id__pri20_idx__freq;

    /* BIT [ 15 :  0]  :- centre_freq1
     * BIT [ 31 : 16]  :- centre_freq2
     */
    A_UINT32 centre_freq1__freq2;

    /* BIT [ 7 :  0]  :- channel_phy_mode
     * BIT [ 23 : 8]  :- static_pattern
     */
    A_UINT32 phy_mode__static_pattern;
} htt_stats_pdev_bw_mgr_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_bw_mgr_stats_tlv htt_pdev_bw_mgr_stats_tlv;


/* STATS_TYPE : HTT_DBG_EXT_STATS_PDEV_BW_MGR
 * TLV_TAGS:
 *      - HTT_STATS_PDEV_BW_MGR_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct {
    htt_stats_pdev_bw_mgr_stats_tlv bw_mgr_tlv;
} htt_pdev_bw_mgr_stats_t;
#endif /* ATH_TARGET */


/*============= start MLO UMAC SSR stats ============= { */

typedef enum {
    HTT_MLO_UMAC_SSR_DBG_POINT_INVALID = 0,
    HTT_MLO_UMAC_SSR_DBG_POINT_PRE_RESET_DISABLE_RXDMA_PREFETCH,
    HTT_MLO_UMAC_SSR_DBG_POINT_PRE_RESET_PMACS_HWMLOS,
    HTT_MLO_UMAC_SSR_DBG_POINT_PRE_RESET_GLOBAL_WSI,
    HTT_MLO_UMAC_SSR_DBG_POINT_PRE_RESET_PMACS_DMAC,
    HTT_MLO_UMAC_SSR_DBG_POINT_PRE_RESET_TCL,
    HTT_MLO_UMAC_SSR_DBG_POINT_PRE_RESET_TQM,
    HTT_MLO_UMAC_SSR_DBG_POINT_PRE_RESET_WBM,
    HTT_MLO_UMAC_SSR_DBG_POINT_PRE_RESET_REO,
    HTT_MLO_UMAC_SSR_DBG_POINT_PRE_RESET_HOST,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESET_PREREQUISITES,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESET_PRE_RING_RESET,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESET_APPLY_SOFT_RESET,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESET_POST_RING_RESET,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESET_FW_TQM_CMDQS,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_HOST,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_UMAC_INTERRUPTS,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_WBM,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_REO,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_TQM,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_PMACS_DMAC,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_TQM_SYNC_CMD,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_GLOBAL_WSI,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_PMACS_HWMLOS,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_ENABLE_RXDMA_PREFETCH,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_TCL,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_HOST_ENQ,
    HTT_MLO_UMAC_SSR_DBG_POINT_POST_RESET_VERIFY_UMAC_RECOVERED,
    /* The below debug point values are reserved for future expansion. */
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED28,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED29,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED30,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED31,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED32,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED33,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED34,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED35,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED36,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED37,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED38,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED39,
    HTT_MLO_UMAC_SSR_DBG_POINT_RESERVED40,
    /*
     * Due to backwards compatibility requirements, no futher DBG_POINT values
     * can be added (but the above reserved values can be repurposed).
     */
    HTT_MLO_UMAC_SSR_DBG_POINT_MAX,
} HTT_MLO_UMAC_SSR_DBG_POINTS;

typedef enum {
    HTT_MLO_UMAC_RECOVERY_HANDSHAKE_INVALID = 0,
    HTT_MLO_UMAC_RECOVERY_HANDSHAKE_DO_PRE_RESET,
    HTT_MLO_UMAC_RECOVERY_HANDSHAKE_DO_POST_RESET_START,
    HTT_MLO_UMAC_RECOVERY_HANDSHAKE_DO_POST_RESET_COMPLETE,
    /* The below recovery handshake values are reserved for future expansion. */
    HTT_MLO_UMAC_RECOVERY_HANDSHAKE_RESERVED4,
    HTT_MLO_UMAC_RECOVERY_HANDSHAKE_RESERVED5,
    HTT_MLO_UMAC_RECOVERY_HANDSHAKE_RESERVED6,
    HTT_MLO_UMAC_RECOVERY_HANDSHAKE_RESERVED7,
    HTT_MLO_UMAC_RECOVERY_HANDSHAKE_RESERVED8,
    /*
     * Due to backwards compatibility requirements, no futher
     * RECOVERY_HANDSHAKE values can be added (but the above
     * reserved values can be repurposed).
     */
    HTT_MLO_UMAC_RECOVERY_HANDSHAKE_COUNT,
} HTT_MLO_UMAC_RECOVERY_HANDSHAKES;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 start_ms;
    A_UINT32 end_ms;
    A_UINT32 delta_ms;
    A_UINT32 reserved;
    A_UINT32 footprint; /* holds a HTT_MLO_UMAC_SSR_DBG_POINTS value */
    A_UINT32 tqm_hw_tstamp;
} htt_stats_mlo_umac_ssr_dbg_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_mlo_umac_ssr_dbg_tlv htt_mlo_umac_ssr_dbg_tlv;

typedef struct {
    A_UINT32 last_mlo_htt_handshake_delta_ms;
    A_UINT32 max_mlo_htt_handshake_delta_ms;
    union {
        A_UINT32 umac_recovery_done_mask;
        struct {
            A_UINT32 pre_reset_disable_rxdma_prefetch : 1,
                     pre_reset_pmacs_hwmlos : 1,
                     pre_reset_global_wsi : 1,
                     pre_reset_pmacs_dmac : 1,
                     pre_reset_tcl : 1,
                     pre_reset_tqm : 1,
                     pre_reset_wbm : 1,
                     pre_reset_reo : 1,
                     pre_reset_host : 1,
                     reset_prerequisites : 1,
                     reset_pre_ring_reset : 1,
                     reset_apply_soft_reset : 1,
                     reset_post_ring_reset : 1,
                     reset_fw_tqm_cmdqs : 1,
                     post_reset_host : 1,
                     post_reset_umac_interrupts : 1,
                     post_reset_wbm : 1,
                     post_reset_reo : 1,
                     post_reset_tqm : 1,
                     post_reset_pmacs_dmac : 1,
                     post_reset_tqm_sync_cmd : 1,
                     post_reset_global_wsi : 1,
                     post_reset_pmacs_hwmlos : 1,
                     post_reset_enable_rxdma_prefetch : 1,
                     post_reset_tcl : 1,
                     post_reset_host_enq : 1,
                     post_reset_verify_umac_recovered : 1,
                     reserved : 5;
        } done_mask;
    };
} htt_mlo_umac_ssr_mlo_stats_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    htt_mlo_umac_ssr_mlo_stats_t mlo;
} htt_stats_mlo_umac_ssr_mlo_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_mlo_umac_ssr_mlo_tlv htt_mlo_umac_ssr_mlo_stats_tlv;

/* dword0 - b'0 - PRE_RESET_DISABLE_RXDMA_PREFETCH */
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_DISABLE_RXDMA_PREFETCH_M 0x1
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_DISABLE_RXDMA_PREFETCH_S 0
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_DISABLE_RXDMA_PREFETCH_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_PRE_RESET_DISABLE_RXDMA_PREFETCH_M) >> \
     HTT_UMAC_RECOVERY_DONE_PRE_RESET_DISABLE_RXDMA_PREFETCH_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_PRE_RESET_DISABLE_RXDMA_PREFETCH_GET(word) \
    HTT_UMAC_RECOVERY_DONE_PRE_RESET_DISABLE_RXDMA_PREFETCH_GET(word)
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_DISABLE_RXDMA_PREFETCH_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_PRE_RESET_DISABLE_RXDMA_PREFETCH, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_PRE_RESET_DISABLE_RXDMA_PREFETCH_S));\
    } while (0)

/* dword0 - b'1 - PRE_RESET_PMACS_HWMLOS */
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_HWMLOS_M 0x2
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_HWMLOS_S 1
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_HWMLOS_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_HWMLOS_M) >> \
     HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_HWMLOS_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_PRE_RESET_PMACS_HWMLOS_GET(word) \
    HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_HWMLOS_GET(word)
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_HWMLOS_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_HWMLOS, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_HWMLOS_S));\
    } while (0)

/* dword0 - b'2 - PRE_RESET_GLOBAL_WSI */
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_GLOBAL_WSI_M 0x4
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_GLOBAL_WSI_S 2
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_GLOBAL_WSI_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_PRE_RESET_GLOBAL_WSI_M) >> \
     HTT_UMAC_RECOVERY_DONE_PRE_RESET_GLOBAL_WSI_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_PRE_RESET_GLOBAL_WSI_GET(word) \
    HTT_UMAC_RECOVERY_DONE_PRE_RESET_GLOBAL_WSI_GET(word)
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_GLOBAL_WSI_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_PRE_RESET_GLOBAL_WSI, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_PRE_RESET_GLOBAL_WSI_S));\
    } while (0)

/* dword0 - b'3 - PRE_RESET_PMACS_DMAC */
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_DMAC_M 0x8
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_DMAC_S 3
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_DMAC_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_DMAC_M) >> \
     HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_DMAC_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_PRE_RESET_PMACS_DMAC_GET(word) \
    HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_DMAC_GET(word)
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_DMAC_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_DMAC, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_PRE_RESET_PMACS_DMAC_S));\
    } while (0)

/* dword0 - b'4 - PRE_RESET_TCL */
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_TCL_M 0x10
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_TCL_S 4
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_TCL_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_PRE_RESET_TCL_M) >> \
     HTT_UMAC_RECOVERY_DONE_PRE_RESET_TCL_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_PRE_RESET_TCL_GET(word) \
    HTT_UMAC_RECOVERY_DONE_PRE_RESET_TCL_GET(word)
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_TCL_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_PRE_RESET_TCL, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_PRE_RESET_TCL_S));\
    } while (0)

/* dword0 - b'5 - PRE_RESET_TQM */
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_TQM_M 0x20
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_TQM_S 5
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_TQM_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_PRE_RESET_TQM_M) >> \
     HTT_UMAC_RECOVERY_DONE_PRE_RESET_TQM_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_PRE_RESET_TQM_GET(word) \
    HTT_UMAC_RECOVERY_DONE_PRE_RESET_TQM_GET(word)
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_TQM_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_PRE_RESET_TQM, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_PRE_RESET_TQM_S));\
    } while (0)

/* dword0 - b'6 - PRE_RESET_WBM */
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_WBM_M 0x40
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_WBM_S 6
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_WBM_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_PRE_RESET_WBM_M) >> \
     HTT_UMAC_RECOVERY_DONE_PRE_RESET_WBM_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_PRE_RESET_WBM_GET(word) \
    HTT_UMAC_RECOVERY_DONE_PRE_RESET_WBM_GET(word)
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_WBM_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_PRE_RESET_WBM, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_PRE_RESET_WBM_S));\
    } while (0)

/* dword0 - b'7 - PRE_RESET_REO */
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_REO_M 0x80
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_REO_S 7
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_REO_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_PRE_RESET_REO_M) >> \
     HTT_UMAC_RECOVERY_DONE_PRE_RESET_REO_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_PRE_RESET_REO_GET(word) \
    HTT_UMAC_RECOVERY_DONE_PRE_RESET_REO_GET(word)
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_REO_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_PRE_RESET_REO, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_PRE_RESET_REO_S));\
    } while (0)

/* dword0 - b'8 - PRE_RESET_HOST */
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_HOST_M 0x100
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_HOST_S 8
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_HOST_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_PRE_RESET_HOST_M) >> \
     HTT_UMAC_RECOVERY_DONE_PRE_RESET_HOST_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_PRE_RESET_HOST_GET(word) \
    HTT_UMAC_RECOVERY_DONE_PRE_RESET_HOST_GET(word)
#define HTT_UMAC_RECOVERY_DONE_PRE_RESET_HOST_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_PRE_RESET_HOST, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_PRE_RESET_HOST_S));\
    } while (0)

/* dword0 - b'9 - RESET_PREREQUISITES */
#define HTT_UMAC_RECOVERY_DONE_RESET_PREREQUISITES_M 0x200
#define HTT_UMAC_RECOVERY_DONE_RESET_PREREQUISITES_S 9
#define HTT_UMAC_RECOVERY_DONE_RESET_PREREQUISITES_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_RESET_PREREQUISITES_M) >> \
     HTT_UMAC_RECOVERY_DONE_RESET_PREREQUISITES_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_RESET_PREREQUISITES_GET(word) \
    HTT_UMAC_RECOVERY_DONE_RESET_PREREQUISITES_GET(word)
#define HTT_UMAC_RECOVERY_DONE_RESET_PREREQUISITES_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_RESET_PREREQUISITES, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_RESET_PREREQUISITES_S));\
    } while (0)

/* dword0 - b'10 - RESET_PRE_RING_RESET */
#define HTT_UMAC_RECOVERY_DONE_RESET_PRE_RING_RESET_M 0x400
#define HTT_UMAC_RECOVERY_DONE_RESET_PRE_RING_RESET_S 10
#define HTT_UMAC_RECOVERY_DONE_RESET_PRE_RING_RESET_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_RESET_PRE_RING_RESET_M) >> \
     HTT_UMAC_RECOVERY_DONE_RESET_PRE_RING_RESET_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_RESET_PRE_RING_RESET_GET(word) \
    HTT_UMAC_RECOVERY_DONE_RESET_PRE_RING_RESET_GET(word)
#define HTT_UMAC_RECOVERY_DONE_RESET_PRE_RING_RESET_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_RESET_PRE_RING_RESET, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_RESET_PRE_RING_RESET_S));\
    } while (0)

/* dword0 - b'11 - RESET_APPLY_SOFT_RESET */
#define HTT_UMAC_RECOVERY_DONE_RESET_APPLY_SOFT_RESET_M 0x800
#define HTT_UMAC_RECOVERY_DONE_RESET_APPLY_SOFT_RESET_S 11
#define HTT_UMAC_RECOVERY_DONE_RESET_APPLY_SOFT_RESET_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_RESET_APPLY_SOFT_RESET_M) >> \
     HTT_UMAC_RECOVERY_DONE_RESET_APPLY_SOFT_RESET_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_RESET_APPLY_SOFT_RESET_GET(word) \
    HTT_UMAC_RECOVERY_DONE_RESET_APPLY_SOFT_RESET_GET(word)
#define HTT_UMAC_RECOVERY_DONE_RESET_APPLY_SOFT_RESET_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_RESET_APPLY_SOFT_RESET, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_RESET_APPLY_SOFT_RESET_S));\
    } while (0)

/* dword0 - b'12 - RESET_POST_RING_RESET */
#define HTT_UMAC_RECOVERY_DONE_RESET_POST_RING_RESET_M 0x1000
#define HTT_UMAC_RECOVERY_DONE_RESET_POST_RING_RESET_S 12
#define HTT_UMAC_RECOVERY_DONE_RESET_POST_RING_RESET_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_RESET_POST_RING_RESET_M) >> \
     HTT_UMAC_RECOVERY_DONE_RESET_POST_RING_RESET_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_RESET_POST_RING_RESET_GET(word) \
    HTT_UMAC_RECOVERY_DONE_RESET_POST_RING_RESET_GET(word)
#define HTT_UMAC_RECOVERY_DONE_RESET_POST_RING_RESET_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_RESET_POST_RING_RESET, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_RESET_POST_RING_RESET_S));\
    } while (0)

/* dword0 - b'13 - RESET_FW_TQM_CMDQS */
#define HTT_UMAC_RECOVERY_DONE_RESET_FW_TQM_CMDQS_M 0x2000
#define HTT_UMAC_RECOVERY_DONE_RESET_FW_TQM_CMDQS_S 13
#define HTT_UMAC_RECOVERY_DONE_RESET_FW_TQM_CMDQS_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_RESET_FW_TQM_CMDQS_M) >> \
     HTT_UMAC_RECOVERY_DONE_RESET_FW_TQM_CMDQS_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_RESET_FW_TQM_CMDQS_GET(word) \
    HTT_UMAC_RECOVERY_DONE_RESET_FW_TQM_CMDQS_GET(word)
#define HTT_UMAC_RECOVERY_DONE_RESET_FW_TQM_CMDQS_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_RESET_FW_TQM_CMDQS, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_RESET_FW_TQM_CMDQS_S));\
    } while (0)

/* dword0 - b'14 - POST_RESET_HOST */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_M 0x4000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_S 14
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_HOST_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_S));\
    } while (0)

/* dword0 - b'15 - POST_RESET_UMAC_INTERRUPTS */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_UMAC_INTERRUPTS_M 0x8000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_UMAC_INTERRUPTS_S 15
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_UMAC_INTERRUPTS_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_UMAC_INTERRUPTS_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_UMAC_INTERRUPTS_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_UMAC_INTERRUPTS_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_UMAC_INTERRUPTS_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_UMAC_INTERRUPTS_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_UMAC_INTERRUPTS, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_UMAC_INTERRUPTS_S));\
    } while (0)

/* dword0 - b'16 - POST_RESET_WBM */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_WBM_M 0x10000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_WBM_S 16
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_WBM_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_WBM_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_WBM_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_WBM_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_WBM_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_WBM_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_WBM, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_WBM_S));\
    } while (0)

/* dword0 - b'17 - POST_RESET_REO */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_REO_M 0x20000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_REO_S 17
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_REO_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_REO_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_REO_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_REO_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_REO_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_REO_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_REO, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_REO_S));\
    } while (0)

/* dword0 - b'18 - POST_RESET_TQM */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_M 0x40000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_S 18
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_TQM_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_S));\
    } while (0)

/* dword0 - b'19 - POST_RESET_PMACS_DMAC */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_DMAC_M 0x80000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_DMAC_S 19
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_DMAC_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_DMAC_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_DMAC_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_PMACS_DMAC_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_DMAC_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_DMAC_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_DMAC, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_DMAC_S));\
    } while (0)

/* dword0 - b'20 - POST_RESET_TQM_SYNC_CMD */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_SYNC_CMD_M 0x100000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_SYNC_CMD_S 20
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_SYNC_CMD_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_SYNC_CMD_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_SYNC_CMD_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_TQM_SYNC_CMD_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_SYNC_CMD_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_SYNC_CMD_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_SYNC_CMD, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_TQM_SYNC_CMD_S));\
    } while (0)

/* dword0 - b'21 - POST_RESET_GLOBAL_WSI */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_GLOBAL_WSI_M 0x200000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_GLOBAL_WSI_S 21
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_GLOBAL_WSI_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_GLOBAL_WSI_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_GLOBAL_WSI_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_GLOBAL_WSI_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_GLOBAL_WSI_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_GLOBAL_WSI_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_GLOBAL_WSI, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_GLOBAL_WSI_S));\
    } while (0)

/* dword0 - b'22 - POST_RESET_PMACS_HWMLOS */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_HWMLOS_M 0x400000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_HWMLOS_S 22
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_HWMLOS_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_HWMLOS_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_HWMLOS_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_PMACS_HWMLOS_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_HWMLOS_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_HWMLOS_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_HWMLOS, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_PMACS_HWMLOS_S));\
    } while (0)

/* dword0 - b'23 - POST_RESET_ENABLE_RXDMA_PREFETCH */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_ENABLE_RXDMA_PREFETCH_M 0x800000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_ENABLE_RXDMA_PREFETCH_S 23
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_ENABLE_RXDMA_PREFETCH_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_ENABLE_RXDMA_PREFETCH_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_ENABLE_RXDMA_PREFETCH_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_ENABLE_RXDMA_PREFETCH_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_ENABLE_RXDMA_PREFETCH_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_ENABLE_RXDMA_PREFETCH_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_ENABLE_RXDMA_PREFETCH, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_ENABLE_RXDMA_PREFETCH_S));\
    } while (0)

/* dword0 - b'24 - POST_RESET_TCL */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TCL_M 0x1000000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TCL_S 24
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TCL_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_TCL_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_TCL_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_TCL_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_TCL_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_TCL_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_TCL, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_TCL_S));\
    } while (0)

/* dword0 - b'25 - POST_RESET_HOST_ENQ */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_ENQ_M 0x2000000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_ENQ_S 25
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_ENQ_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_ENQ_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_ENQ_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_HOST_ENQ_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_ENQ_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_ENQ_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_ENQ, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_HOST_ENQ_S));\
    } while (0)

/* dword0 - b'26 - POST_RESET_VERIFY_UMAC_RECOVERED */
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_VERIFY_UMAC_RECOVERED_M 0x4000000
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_VERIFY_UMAC_RECOVERED_S 26
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_VERIFY_UMAC_RECOVERED_GET(word0) \
    (((word0) & HTT_UMAC_RECOVERY_DONE_POST_RESET_VERIFY_UMAC_RECOVERED_M) >> \
     HTT_UMAC_RECOVERY_DONE_POST_RESET_VERIFY_UMAC_RECOVERED_S)
/* provide properly-named macro */
#define HTT_STATS_MLO_UMAC_SSR_MLO_MLO_POST_RESET_VERIFY_UMAC_RECOVERED_GET(word) \
    HTT_UMAC_RECOVERY_DONE_POST_RESET_VERIFY_UMAC_RECOVERED_GET(word)
#define HTT_UMAC_RECOVERY_DONE_POST_RESET_VERIFY_UMAC_RECOVERED_SET(word0, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_UMAC_RECOVERY_DONE_POST_RESET_VERIFY_UMAC_RECOVERED, _val); \
        ((word0) |= ((_val) << HTT_UMAC_RECOVERY_DONE_POST_RESET_VERIFY_UMAC_RECOVERED_S));\
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 last_trigger_request_ms;
    A_UINT32 last_start_ms;
    A_UINT32 last_start_disengage_umac_ms;
    A_UINT32 last_enter_ssr_platform_thread_ms;
    A_UINT32 last_exit_ssr_platform_thread_ms;
    A_UINT32 last_start_engage_umac_ms;
    A_UINT32 last_done_successful_ms;
    A_UINT32 post_reset_tqm_sync_cmd_completion_ms;
    A_UINT32 htt_sync_mlo_initiate_umac_recovery_ms;
    A_UINT32 htt_sync_do_pre_reset_ms;
    A_UINT32 htt_sync_do_post_reset_start_ms;
    A_UINT32 htt_sync_do_post_reset_complete_ms;
} htt_stats_mlo_umac_ssr_kpi_tstmp_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_mlo_umac_ssr_kpi_tstmp_tlv
    htt_mlo_umac_ssr_kpi_tstamp_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 htt_sync_start_ms;
    A_UINT32 htt_sync_delta_ms;
    A_UINT32 post_t2h_start_ms;
    A_UINT32 post_t2h_delta_ms;
    A_UINT32 post_t2h_msg_read_shmem_ms;
    A_UINT32 post_t2h_msg_write_shmem_ms;
    A_UINT32 post_t2h_msg_send_msg_to_host_ms;
} htt_stats_mlo_umac_ssr_handshake_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_mlo_umac_ssr_handshake_tlv
    htt_mlo_umac_htt_handshake_stats_tlv;

#ifdef ATH_TARGET
typedef struct {
    /*
     * Note that the host cannot use this struct directly, but instead needs
     * to use the TLV header within each element of each of the arrays in
     * this struct to determine where the subsequent item resides.
     */
    htt_stats_mlo_umac_ssr_dbg_tlv dbg_point[HTT_MLO_UMAC_SSR_DBG_POINT_MAX];
    htt_stats_mlo_umac_ssr_handshake_tlv htt_handshakes[HTT_MLO_UMAC_RECOVERY_HANDSHAKE_COUNT];
} htt_mlo_umac_ssr_kpi_delta_stats_t;
#endif /* ATH_TARGET */

#ifdef ATH_TARGET
typedef struct {
    /*
     * Since each item within htt_mlo_umac_ssr_kpi_delta_stats_t has its own
     * TLV header, and since no additional fields are added in this struct
     * beyond the htt_mlo_umac_ssr_kpi_delta_stats_t info, no additional
     * TLV header is needed.
     *
     * Note that the host cannot use this struct directly, but instead needs
     * to use the TLV header within each item inside the
     * htt_mlo_umac_ssr_kpi_delta_stats_t to determine where the subsequent
     * item resides.
     */
    htt_mlo_umac_ssr_kpi_delta_stats_t kpi_delta;
} htt_mlo_umac_ssr_kpi_delta_stats_tlv;
#endif /* ATH_TARGET */

typedef struct {
    A_UINT32 last_e2e_delta_ms;
    A_UINT32 max_e2e_delta_ms;
    A_UINT32 per_handshake_max_allowed_delta_ms;
    /* Total done count */
    A_UINT32 total_success_runs_cnt;
    A_UINT32 umac_recovery_in_progress;
    /* Count of Disengaged in Pre reset */
    A_UINT32 umac_disengaged_count;
    /* Count of UMAC Soft/Control Reset */
    A_UINT32 umac_soft_reset_count;
    /* Count of Engaged in Post reset */
    A_UINT32 umac_engaged_count;
} htt_mlo_umac_ssr_common_stats_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    htt_mlo_umac_ssr_common_stats_t cmn;
} htt_stats_mlo_umac_ssr_cmn_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_mlo_umac_ssr_cmn_tlv htt_mlo_umac_ssr_common_stats_tlv;

typedef struct {
    A_UINT32 trigger_requests_count;
    A_UINT32 trigger_count_for_umac_hang;
    A_UINT32 trigger_count_for_mlo_target_recovery_mode1;
    A_UINT32 trigger_count_for_unknown_signature;
    A_UINT32 total_trig_dropped;
    A_UINT32 trigger_count_for_unit_test_direct_trigger;
    A_UINT32 trigger_count_for_tx_de_wdg_dummy_frame_tout;
    A_UINT32 trigger_count_for_peer_delete_wdg_dummy_frame_tout;
    A_UINT32 trigger_count_for_reo_hang;
    A_UINT32 trigger_count_for_tqm_hang;
    A_UINT32 trigger_count_for_tcl_hang;
    A_UINT32 trigger_count_for_wbm_hang;
} htt_mlo_umac_ssr_trigger_stats_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    htt_mlo_umac_ssr_trigger_stats_t trigger;
} htt_stats_mlo_umac_ssr_trigger_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_mlo_umac_ssr_trigger_tlv htt_mlo_umac_ssr_trigger_stats_tlv;

#ifdef ATH_TARGET
typedef struct {
    /*
     * Note that the host cannot use this struct directly, but instead needs
     * to use the TLV header within each element to determine where the
     * subsequent element resides.
     */
    htt_mlo_umac_ssr_kpi_delta_stats_tlv kpi_delta_tlv;
    htt_stats_mlo_umac_ssr_kpi_tstmp_tlv kpi_tstamp_tlv;
} htt_mlo_umac_ssr_kpi_stats_t;
#endif /* ATH_TARGET */

#ifdef ATH_TARGET
typedef struct {
    /*
     * Since the embedded sub-struct within htt_mlo_umac_ssr_kpi_stats_tlv
     * has its own TLV header, and since no additional fields are added in
     * this struct beyond the htt_mlo_umac_ssr_kpi_stats_t info, no additional
     * TLV header is needed.
     *
     * Note that the host cannot use this struct directly, but instead needs
     * to use the TLV header within the htt_mlo_umac_ssr_kpi_stats_t sub-struct
     * to determine how much data is present for this struct.
     */
    htt_mlo_umac_ssr_kpi_stats_t kpi;
} htt_mlo_umac_ssr_kpi_stats_tlv;
#endif /* ATH_TARGET */

#ifdef ATH_TARGET
typedef struct {
    /*
     * Note that the host cannot use this struct directly, but instead needs
     * to use the TLV header within each element to determine where the
     * subsequent element resides.
     */
    htt_stats_mlo_umac_ssr_trigger_tlv trigger_tlv;
    htt_mlo_umac_ssr_kpi_stats_tlv     kpi_tlv;
    htt_stats_mlo_umac_ssr_mlo_tlv mlo_tlv;
    htt_stats_mlo_umac_ssr_cmn_tlv cmn_tlv;
} htt_mlo_umac_ssr_stats_tlv;
#endif /* ATH_TARGET */

/*============= end MLO UMAC SSR stats ============= } */

typedef struct {
    A_UINT32 total_done;
    A_UINT32 trigger_requests_count;
    A_UINT32 total_trig_dropped;
    A_UINT32 umac_disengaged_count;
    A_UINT32 umac_soft_reset_count;
    A_UINT32 umac_engaged_count;
    A_UINT32 last_trigger_request_ms;
    A_UINT32 last_start_ms;
    A_UINT32 last_start_disengage_umac_ms;
    A_UINT32 last_enter_ssr_platform_thread_ms;
    A_UINT32 last_exit_ssr_platform_thread_ms;
    A_UINT32 last_start_engage_umac_ms;
    A_UINT32 last_done_successful_ms;
    A_UINT32 last_e2e_delta_ms;
    A_UINT32 max_e2e_delta_ms;
    A_UINT32 trigger_count_for_umac_hang;
    A_UINT32 trigger_count_for_mlo_quick_ssr;
    A_UINT32 trigger_count_for_unknown_signature;
    A_UINT32 post_reset_tqm_sync_cmd_completion_ms;
    A_UINT32 htt_sync_mlo_initiate_umac_recovery_ms;
    A_UINT32 htt_sync_do_pre_reset_ms;
    A_UINT32 htt_sync_do_post_reset_start_ms;
    A_UINT32 htt_sync_do_post_reset_complete_ms;
} htt_umac_ssr_stats_t;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    htt_umac_ssr_stats_t stats;
} htt_stats_umac_ssr_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_umac_ssr_tlv htt_umac_ssr_stats_tlv;

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 svc_class_id;
    /* codel_drops:
     * How many times have MSDU queues belonging to this service class
     * dropped their head MSDU due to the queue's latency being above
     * the CoDel latency limit specified for the service class throughout
     * the full CoDel latency statistics collection window.
     */
    A_UINT32 codel_drops;
    /* codel_no_drops:
     * How many times have MSDU queues belonging to this service class
     * completed a CoDel latency statistics collection window and
     * concluded that no head MSDU drop is needed, due to the MSDU queue's
     * latency being under the limit specified for the service class at
     * some point during the window.
     */
    A_UINT32 codel_no_drops;
} htt_stats_codel_svc_class_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_codel_svc_class_tlv htt_codel_svc_class_stats_tlv;

#define HTT_CODEL_MSDUQ_STATS_TX_FLOW_NUM_M 0x0000FFFF
#define HTT_CODEL_MSDUQ_STATS_TX_FLOW_NUM_S 0

#define HTT_CODEL_MSDUQ_STATS_TX_FLOW_NUM_GET(_var) \
    (((_var) & HTT_CODEL_MSDUQ_STATS_TX_FLOW_NUM_M) >> \
     HTT_CODEL_MSDUQ_STATS_TX_FLOW_NUM_S)
#define HTT_CODEL_MSDUQ_STATS_TX_FLOW_NUM_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_CODEL_MSDUQ_STATS_TX_FLOW_NUM, _val); \
        ((_var) |= ((_val) << HTT_CODEL_MSDUQ_STATS_TX_FLOW_NUM_S)); \
    } while (0)

#define HTT_CODEL_MSDUQ_STATS_SVC_CLASS_ID_M 0x00FF0000
#define HTT_CODEL_MSDUQ_STATS_SVC_CLASS_ID_S 16

#define HTT_CODEL_MSDUQ_STATS_SVC_CLASS_ID_GET(_var) \
    (((_var) & HTT_CODEL_MSDUQ_STATS_SVC_CLASS_ID_M) >> \
     HTT_CODEL_MSDUQ_STATS_SVC_CLASS_ID_S)
#define HTT_CODEL_MSDUQ_STATS_SVC_CLASS_ID_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_CODEL_MSDUQ_STATS_SVC_CLASS_ID, _val); \
        ((_var) |= ((_val) << HTT_CODEL_MSDUQ_STATS_SVC_CLASS_ID_S)); \
    } while (0)

#define HTT_CODEL_MSDUQ_STATS_DROPS_M 0x0000FFFF
#define HTT_CODEL_MSDUQ_STATS_DROPS_S 0

#define HTT_CODEL_MSDUQ_STATS_DROPS_GET(_var) \
    (((_var) & HTT_CODEL_MSDUQ_STATS_DROPS_M) >> \
     HTT_CODEL_MSDUQ_STATS_DROPS_S)
#define HTT_CODEL_MSDUQ_STATS_DROPS_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_CODEL_MSDUQ_STATS_DROPS, _val); \
        ((_var) |= ((_val) << HTT_CODEL_MSDUQ_STATS_DROPS_S)); \
    } while (0)

#define HTT_CODEL_MSDUQ_STATS_NO_DROPS_M 0xFFFF0000
#define HTT_CODEL_MSDUQ_STATS_NO_DROPS_S 16

#define HTT_CODEL_MSDUQ_STATS_NO_DROPS_GET(_var) \
    (((_var) & HTT_CODEL_MSDUQ_STATS_NO_DROPS_M) >> \
     HTT_CODEL_MSDUQ_STATS_NO_DROPS_S)
#define HTT_CODEL_MSDUQ_STATS_NO_DROPS_SET(_var, _val) \
    do { \
        HTT_CHECK_SET_VAL(HTT_CODEL_MSDUQ_STATS_NO_DROPS, _val); \
        ((_var) |= ((_val) << HTT_CODEL_MSDUQ_STATS_NO_DROPS_S)); \
    } while (0)

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    union {
        A_UINT32 id__word;
        struct {
            A_UINT32 tx_flow_num: 16, /* FW's MSDU queue ID */
                     svc_class_id: 8,
                     reserved: 8;
        };
    };
    union {
        A_UINT32 stats__word;
        struct {
            A_UINT32
                codel_drops: 16,
                codel_no_drops: 16;
        };
    };
} htt_stats_codel_msduq_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_codel_msduq_tlv htt_codel_msduq_stats_tlv;

/*===================== start MLO stats ====================*/

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 pref_link_num_sec_link_sched;
    A_UINT32 pref_link_num_pref_link_timeout;
    A_UINT32 pref_link_num_pref_link_sch_delay_ipc;
    A_UINT32 pref_link_num_pref_link_timeout_ipc;
} htt_stats_mlo_sched_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_mlo_sched_stats_tlv htt_mlo_sched_stats_tlv;

/* STATS_TYPE : HTT_DBG_MLO_SCHED_STATS
 * TLV_TAGS:
 *   - HTT_STATS_MLO_SCHED_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct _htt_mlo_sched_stats {
    htt_stats_mlo_sched_stats_tlv  preferred_link_stats;
} htt_mlo_sched_stats_t;
#endif /* ATH_TARGET */

#define HTT_STATS_HWMLO_MAX_LINKS 6
#define HTT_STATS_MLO_MAX_IPC_RINGS 7

typedef struct {
    htt_tlv_hdr_t tlv_hdr;
    A_UINT32 mlo_ipc_ring_full_cnt[HTT_STATS_HWMLO_MAX_LINKS][HTT_STATS_MLO_MAX_IPC_RINGS];
} htt_stats_pdev_mlo_ipc_stats_tlv;
/* preserve old name alias for new name consistent with the tag name */
typedef htt_stats_pdev_mlo_ipc_stats_tlv htt_pdev_mlo_ipc_stats_tlv;

/* STATS_TYPE : HTT_DBG_MLO_IPC_STATS
 * TLV_TAGS:
 *   - HTT_STATS_PDEV_MLO_IPC_STATS_TAG
 */
/* NOTE:
 * This structure is for documentation, and cannot be safely used directly.
 * Instead, use the constituent TLV structures to fill/parse.
 */
#ifdef ATH_TARGET
typedef struct _htt_mlo_ipc_stats {
    htt_stats_pdev_mlo_ipc_stats_tlv mlo_ipc_stats;
} htt_pdev_mlo_ipc_stats_t;
#endif /* ATH_TARGET */

/*===================== end MLO stats ======================*/

typedef enum {
    HTT_CTRL_PATH_STATS_CAL_TYPE_ADC                     = 0x0,
    HTT_CTRL_PATH_STATS_CAL_TYPE_DAC                     = 0x1,
    HTT_CTRL_PATH_STATS_CAL_TYPE_PROCESS                 = 0x2,
    HTT_CTRL_PATH_STATS_CAL_TYPE_NOISE_FLOOR             = 0x3,
    HTT_CTRL_PATH_STATS_CAL_TYPE_RXDCO                   = 0x4,
    HTT_CTRL_PATH_STATS_CAL_TYPE_COMB_TXLO_TXIQ_RXIQ     = 0x5,
    HTT_CTRL_PATH_STATS_CAL_TYPE_TXLO                    = 0x6,
    HTT_CTRL_PATH_STATS_CAL_TYPE_TXIQ                    = 0x7,
    HTT_CTRL_PATH_STATS_CAL_TYPE_RXIQ                    = 0x8,
    HTT_CTRL_PATH_STATS_CAL_TYPE_IM2                     = 0x9,
    HTT_CTRL_PATH_STATS_CAL_TYPE_LNA                     = 0xa,
    HTT_CTRL_PATH_STATS_CAL_TYPE_DPD_LP_RXDCO            = 0xb,
    HTT_CTRL_PATH_STATS_CAL_TYPE_DPD_LP_RXIQ             = 0xc,
    HTT_CTRL_PATH_STATS_CAL_TYPE_DPD_MEMORYLESS          = 0xd,
    HTT_CTRL_PATH_STATS_CAL_TYPE_DPD_MEMORY              = 0xe,
    HTT_CTRL_PATH_STATS_CAL_TYPE_IBF                     = 0xf,
    HTT_CTRL_PATH_STATS_CAL_TYPE_PDET_AND_PAL            = 0x10,
    HTT_CTRL_PATH_STATS_CAL_TYPE_RXDCO_IQ                = 0x11,
    HTT_CTRL_PATH_STATS_CAL_TYPE_RXDCO_DTIM              = 0x12,
    HTT_CTRL_PATH_STATS_CAL_TYPE_TPC_CAL                 = 0x13,
    HTT_CTRL_PATH_STATS_CAL_TYPE_DPD_TIMEREQ             = 0x14,
    HTT_CTRL_PATH_STATS_CAL_TYPE_BWFILTER                = 0x15,
    HTT_CTRL_PATH_STATS_CAL_TYPE_PEF                     = 0x16,
    HTT_CTRL_PATH_STATS_CAL_TYPE_PADROOP                 = 0x17,
    HTT_CTRL_PATH_STATS_CAL_TYPE_SELFCALTPC              = 0x18,
    HTT_CTRL_PATH_STATS_CAL_TYPE_RXSPUR                  = 0x19,

    /* add new cal types above this line */
    HTT_CTRL_PATH_STATS_CAL_TYPE_INVALID                 = 0xFF
} htt_ctrl_path_stats_cal_type_ids;

#define HTT_RETURN_STRING(str) case ((str)): return (A_UINT8 *)(# str);

#define HTT_GET_BITS(_val, _index, _num_bits) \
    (((_val) >> (_index)) & ((1 << (_num_bits)) - 1))

#define HTT_CTRL_PATH_CALIBRATION_STATS_CAL_TYPE_GET(cal_info) \
    HTT_GET_BITS(cal_info, 0, 8)

/*
 * Used by some hosts to print names of cal type, based on
 * htt_ctrl_path_cal_type_ids values specified in
 * htt_ctrl_path_calibration_stats_struct in ctrl_path_stats event msg.
 */
#ifdef HTT_CTRL_PATH_STATS_CAL_TYPE_STRINGS
static INLINE A_UINT8 *htt_ctrl_path_cal_type_id_to_name(A_UINT32 cal_type_id)
{
    switch (cal_type_id)
    {
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_ADC);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_DAC);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_PROCESS);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_NOISE_FLOOR);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_RXDCO);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_COMB_TXLO_TXIQ_RXIQ);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_TXLO);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_TXIQ);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_RXIQ);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_IM2);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_LNA);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_DPD_LP_RXDCO);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_DPD_LP_RXIQ);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_DPD_MEMORYLESS);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_DPD_MEMORY);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_IBF);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_PDET_AND_PAL);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_RXDCO_IQ);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_RXDCO_DTIM);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_TPC_CAL);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_DPD_TIMEREQ);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_BWFILTER);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_PEF);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_PADROOP);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_SELFCALTPC);
        HTT_RETURN_STRING(HTT_CTRL_PATH_STATS_CAL_TYPE_RXSPUR);
    }

    return (A_UINT8 *) "HTT_CTRL_PATH_STATS_CAL_TYPE_UNKNOWN";
}
#endif /* HTT_CTRL_PATH_STATS_CAL_TYPE_STRINGS */


#endif /* __HTT_STATS_H__ */
