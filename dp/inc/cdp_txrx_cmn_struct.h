/*
 * Copyright (c) 2011-2020 The Linux Foundation. All rights reserved.
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
 * @file cdp_txrx_cmn_struct.h
 * @brief Define the host data path converged API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_CMN_STRUCT_H_
#define _CDP_TXRX_CMN_STRUCT_H_

/**
 * For WIN legacy header compilation
 * Temporary add dummy definitions
 * should be removed properly WIN legacy code handle
 */

#include "htc_api.h"
#include "qdf_types.h"
#include "qdf_nbuf.h"
#include "qdf_atomic.h"
#ifdef DP_MOB_DEFS
#include <cdp_txrx_mob_def.h>
#endif
#include <cdp_txrx_handle.h>
#include <cdp_txrx_stats_struct.h>
#ifdef WLAN_RX_PKT_CAPTURE_ENH
#include "cdp_txrx_extd_struct.h"
#endif

#ifndef OL_TXRX_NUM_LOCAL_PEER_IDS
/*
 * Each AP will occupy one ID, so it will occupy two IDs for AP-AP mode.
 * Clients will be assigned max 32 IDs.
 * STA(associated)/P2P DEV(self-PEER) will get one ID.
 */
#define OL_TXRX_NUM_LOCAL_PEER_IDS (32 + 1 + 1 + 1)
#endif

#define CDP_BA_256_BIT_MAP_SIZE_DWORDS 8
#define CDP_BA_64_BIT_MAP_SIZE_DWORDS 2
#define CDP_RSSI_CHAIN_LEN 8

#define OL_TXRX_INVALID_PDEV_ID 0xff
#define OL_TXRX_INVALID_LOCAL_PEER_ID 0xffff
#define CDP_INVALID_VDEV_ID 0xff
/* Options for Dump Statistics */
#define CDP_HDD_STATS               0
#define CDP_TXRX_PATH_STATS         1
#define CDP_TXRX_HIST_STATS         2
#define CDP_TXRX_TSO_STATS          3
#define CDP_HDD_NETIF_OPER_HISTORY  4
#define CDP_DUMP_TX_FLOW_POOL_INFO  5
#define CDP_TXRX_DESC_STATS         6
#define CDP_HIF_STATS               7
#define CDP_LRO_STATS               8
#define CDP_NAPI_STATS              9
#define CDP_WLAN_RX_BUF_DEBUG_STATS 10
#define CDP_RX_RING_STATS          11
#define CDP_DP_NAPI_STATS          12
#define CDP_DP_RX_THREAD_STATS     13
#define CDP_SCHEDULER_STATS        21
#define CDP_TX_QUEUE_STATS         22
#define CDP_BUNDLE_STATS           23
#define CDP_CREDIT_STATS           24
#define CDP_DISCONNECT_STATS       25

#define WME_AC_TO_TID(_ac) (       \
		((_ac) == WME_AC_VO) ? 6 : \
		((_ac) == WME_AC_VI) ? 5 : \
		((_ac) == WME_AC_BK) ? 1 : \
		0)

#define TID_TO_WME_AC(_tid) (      \
		(((_tid) == 0) || ((_tid) == 3)) ? WME_AC_BE : \
		(((_tid) == 1) || ((_tid) == 2)) ? WME_AC_BK : \
		(((_tid) == 4) || ((_tid) == 5)) ? WME_AC_VI : \
		WME_AC_VO)

#define CDP_MU_MAX_USERS 37
#define CDP_MU_MAX_USER_INDEX (CDP_MU_MAX_USERS - 1)
#define CDP_INVALID_PEER 0xffff
#define CDP_INVALID_TID	 31
#define CDP_INVALID_TX_ENCAP_TYPE	 6
#define CDP_INVALID_SEC_TYPE		12

#define CDP_DATA_TID_MAX 8
#define CDP_DATA_NON_QOS_TID 16

#define CDP_NUM_SA_BW 4
#define CDP_PERCENT_MACRO 100
#define CDP_NUM_KB_IN_MB 1000
/*
 * advance rx monitor filter
 * */
#define MON_FILTER_PASS			0x0001
#define MON_FILTER_OTHER		0x0002
#define MON_FILTER_ALL			0x0003

#define FILTER_MGMT_ALL			0xFFFF
#define FILTER_MGMT_ASSOC_REQ		0x0001
#define FILTER_MGMT_ASSOC_RES		0x0002
#define FILTER_MGMT_REASSOC_REQ		0x0004
#define FILTER_MGMT_REASSOC_RES		0x0008
#define FILTER_MGMT_PROBE_REQ		0x0010
#define FILTER_MGMT_PROBE_RES		0x0020
#define FILTER_MGMT_TIM_ADVT		0x0040
#define FILTER_MGMT_RESERVED_7		0x0080
#define FILTER_MGMT_BEACON		0x0100
#define FILTER_MGMT_ATIM		0x0200
#define FILTER_MGMT_DISASSOC		0x0400
#define FILTER_MGMT_AUTH		0x0800
#define FILTER_MGMT_DEAUTH		0x1000
#define FILTER_MGMT_ACTION		0x2000
#define FILTER_MGMT_ACT_NO_ACK		0x4000
#define FILTER_MGMT_RESERVED_15		0x8000

#define FILTER_CTRL_ALL			0xFFFF
#define FILTER_CTRL_RESERVED_1		0x0001
#define FILTER_CTRL_RESERVED_2		0x0002
#define FILTER_CTRL_TRIGGER		0x0004
#define FILTER_CTRL_RESERVED_4		0x0008
#define FILTER_CTRL_BF_REP_POLL		0x0010
#define FILTER_CTRL_VHT_NDP		0x0020
#define FILTER_CTRL_FRAME_EXT		0x0040
#define FILTER_CTRL_CTRLWRAP		0x0080
#define FILTER_CTRL_BA_REQ		0x0100
#define FILTER_CTRL_BA			0x0200
#define FILTER_CTRL_PSPOLL		0x0400
#define FILTER_CTRL_RTS			0x0800
#define FILTER_CTRL_CTS			0x1000
#define FILTER_CTRL_ACK			0x2000
#define FILTER_CTRL_CFEND		0x4000
#define FILTER_CTRL_CFEND_CFACK		0x8000

#define FILTER_DATA_ALL			0xFFFF
#define FILTER_DATA_MCAST		0x4000
#define FILTER_DATA_UCAST		0x8000
#define FILTER_DATA_DATA		0x0001
#define FILTER_DATA_NULL		0x0008

/*
 * Multiply rate by 2 to avoid float point
 * and get rate in units of 500kbps
 */
#define CDP_11B_RATE_0MCS (11 * 2)
#define CDP_11B_RATE_1MCS (5.5 * 2)
#define CDP_11B_RATE_2MCS (2 * 2)
#define CDP_11B_RATE_3MCS (1 * 2)
#define CDP_11B_RATE_4MCS (11 * 2)
#define CDP_11B_RATE_5MCS (5.5 * 2)
#define CDP_11B_RATE_6MCS (2 * 2)

#define CDP_11A_RATE_0MCS (48 * 2)
#define CDP_11A_RATE_1MCS (24 * 2)
#define CDP_11A_RATE_2MCS (12 * 2)
#define CDP_11A_RATE_3MCS (6 * 2)
#define CDP_11A_RATE_4MCS (54 * 2)
#define CDP_11A_RATE_5MCS (36 * 2)
#define CDP_11A_RATE_6MCS (18 * 2)
#define CDP_11A_RATE_7MCS (9 * 2)

#define CDP_LEGACY_MCS0  0
#define CDP_LEGACY_MCS1  1
#define CDP_LEGACY_MCS2  2
#define CDP_LEGACY_MCS3  3
#define CDP_LEGACY_MCS4  4
#define CDP_LEGACY_MCS5  5
#define CDP_LEGACY_MCS6  6
#define CDP_LEGACY_MCS7  7

QDF_DECLARE_EWMA(tx_lag, 1024, 8)
struct cdp_stats_cookie;

/*
 * DP configuration parameters
 */
enum cdp_cfg_param_type {
	CDP_CFG_MAX_PEER_ID,
	CDP_CFG_CCE_DISABLE,
	CDP_CFG_NUM_PARAMS
};

/*
 * PPDU TYPE from FW -
 * @CDP_PPDU_STATS_PPDU_TYPE_SU: single user type
 * @CDP_PPDU_STATS_PPDU_TYPE_MU_MIMO: multi user mu-mimo
 * @CDP_PPDU_STATS_PPDU_TYPE_MU_OFDMA: multi user ofdma
 * @CDP_PPDU_STATS_PPDU_TYPE_MU_MIMO_OFDMA: multi user mu-mimo ofdma
 * @CDP_PPDU_STATS_PPDU_TYPE_UL_TRIG: ul trigger ppdu
 * @CDP_PPDU_STATS_PPDU_TYPE_BURST_BCN: burst beacon
 * @CDP_PPDU_STATS_PPDU_TYPE_UL_BSR_RESP: bsr respond
 * @CDP_PPDU_STATS_PPDU_TYPE_UL_BSR_TRIG: bsr trigger
 * @CDP_PPDU_STATS_PPDU_TYPE_UL_RESP: ul response
 * @CDP_PPDU_STATS_PPDU_TYPE_UNKNOWN
 */
enum CDP_PPDU_STATS_PPDU_TYPE {
	CDP_PPDU_STATS_PPDU_TYPE_SU = 0,
	CDP_PPDU_STATS_PPDU_TYPE_MU_MIMO = 1,
	CDP_PPDU_STATS_PPDU_TYPE_MU_OFDMA = 2,
	CDP_PPDU_STATS_PPDU_TYPE_MU_MIMO_OFDMA = 4,
	CDP_PPDU_STATS_PPDU_TYPE_UL_TRIG = 5,
	CDP_PPDU_STATS_PPDU_TYPE_BURST_BCN = 6,
	CDP_PPDU_STATS_PPDU_TYPE_UL_BSR_RESP = 7,
	CDP_PPDU_STATS_PPDU_TYPE_UL_BSR_TRIG = 8,
	CDP_PPDU_STATS_PPDU_TYPE_UL_RESP = 9,
	CDP_PPDU_STATS_PPDU_TYPE_UNKNOWN = 0x1F,
};

/*
 * htt_dbg_stats_type -
 * bit positions for each stats type within a stats type bitmask
 * The bitmask contains 24 bits.
 */
enum htt_cmn_dbg_stats_type {
	HTT_DBG_CMN_STATS_WAL_PDEV_TXRX      = 0,  /* bit 0 -> 0x1 */
	HTT_DBG_CMN_STATS_RX_REORDER         = 1,  /* bit 1 -> 0x2 */
	HTT_DBG_CMN_STATS_RX_RATE_INFO       = 2,  /* bit 2 -> 0x4 */
	HTT_DBG_CMN_STATS_TX_PPDU_LOG        = 3,  /* bit 3 -> 0x8 */
	HTT_DBG_CMN_STATS_TX_RATE_INFO       = 4,  /* bit 4 -> 0x10 */
	HTT_DBG_CMN_STATS_TIDQ               = 5,  /* bit 5 -> 0x20 */
	HTT_DBG_CMN_STATS_TXBF_INFO          = 6,  /* bit 6 -> 0x40 */
	HTT_DBG_CMN_STATS_SND_INFO           = 7,  /* bit 7 -> 0x80 */
	HTT_DBG_CMN_STATS_ERROR_INFO         = 8,  /* bit 8  -> 0x100 */
	HTT_DBG_CMN_STATS_TX_SELFGEN_INFO    = 9,  /* bit 9  -> 0x200 */
	HTT_DBG_CMN_STATS_TX_MU_INFO         = 10, /* bit 10 -> 0x400 */
	HTT_DBG_CMN_STATS_SIFS_RESP_INFO     = 11, /* bit 11 -> 0x800 */
	HTT_DBG_CMN_STATS_RESET_INFO         = 12, /* bit 12 -> 0x1000 */
	HTT_DBG_CMN_STATS_MAC_WDOG_INFO      = 13, /* bit 13 -> 0x2000 */
	HTT_DBG_CMN_STATS_TX_DESC_INFO       = 14, /* bit 14 -> 0x4000 */
	HTT_DBG_CMN_STATS_TX_FETCH_MGR_INFO  = 15, /* bit 15 -> 0x8000 */
	HTT_DBG_CMN_STATS_TX_PFSCHED_INFO    = 16, /* bit 16 -> 0x10000 */
	HTT_DBG_CMN_STATS_TX_PATH_STATS_INFO = 17, /* bit 17 -> 0x20000 */
	/* bits 18-23 currently reserved */

	/* keep this last */
	HTT_DBG_CMN_NUM_STATS,
	HTT_DBG_CMN_NUM_STATS_INVALID        = 31, /* bit 31 -> 0x80000000 */
};

/*
 * cdp_host_txrx_stats: Different types of host stats
 * @TXRX_HOST_STATS_INVALID: Invalid option
 * @TXRX_RX_RATE_STATS: Rx rate info
 * @TXRX_TX_RATE_STATS: Tx rate info
 * @TXRX_TX_HOST_STATS: Print Tx stats
 * @TXRX_RX_HOST_STATS: Print host Rx stats
 * @TXRX_CLEAR_STATS: clear all host stats
 * @TXRX_SRNG_PTR_STATS: Print SRNG pointer stats
 * @TXRX_RX_MON_STATS: Print monitor mode stats
 * @TXRX_REO_QUEUE_STATS: Print Per peer REO Queue Stats
 * @TXRX_SOC_CFG_PARAMS: Print soc cfg params info
 * @TXRX_PDEV_CFG_PARAMS: Print pdev cfg params info
 * @TXRX_NAPI_STATS: Print NAPI scheduling statistics
 * @TXRX_SOC_INTERRUPT_STATS: Print soc interrupt stats
 */
enum cdp_host_txrx_stats {
	TXRX_HOST_STATS_INVALID  = -1,
	TXRX_CLEAR_STATS     = 0,
	TXRX_RX_RATE_STATS   = 1,
	TXRX_TX_RATE_STATS   = 2,
	TXRX_TX_HOST_STATS   = 3,
	TXRX_RX_HOST_STATS   = 4,
	TXRX_AST_STATS       = 5,
	TXRX_SRNG_PTR_STATS  = 6,
	TXRX_RX_MON_STATS    = 7,
	TXRX_REO_QUEUE_STATS = 8,
	TXRX_SOC_CFG_PARAMS   = 9,
	TXRX_PDEV_CFG_PARAMS  = 10,
	TXRX_NAPI_STATS       = 11,
	TXRX_SOC_INTERRUPT_STATS = 12,
	TXRX_HOST_STATS_MAX,
};

/*
 * cdp_ppdu_ftype: PPDU Frame Type
 * @CDP_PPDU_FTYPE_DATA: SU or MU Data Frame
 * @CDP_PPDU_FTYPE_CTRL: Control/Management Frames
 * @CDP_PPDU_FTYPE_BAR: SU or MU BAR frames
*/
enum cdp_ppdu_ftype {
	CDP_PPDU_FTYPE_CTRL,
	CDP_PPDU_FTYPE_DATA,
	CDP_PPDU_FTYPE_BAR,
	CDP_PPDU_FTYPE_MAX
};


/**
 * @brief General specification of the tx frame contents
 *
 * @details
 * for efficiency, the HTT packet type values correspond
 * to the bit positions of the WAL packet type values, so the
 * translation is a simple shift operation.
 */
enum htt_cmn_pkt_type {
	htt_cmn_pkt_type_raw = 0,
	htt_cmn_pkt_type_native_wifi = 1,
	htt_cmn_pkt_type_ethernet = 2,
	htt_cmn_pkt_type_mgmt = 3,
	htt_cmn_pkt_type_eth2 = 4,

	/* keep this last */
	htt_cmn_pkt_num_types
};

/**
 * @General description of HTT received packets status
 * It is similar to htt_rx_status enum
 * but is added as a cdp enum can be freely used in OL_IF layer
 */
enum htt_cmn_rx_status {
	htt_cmn_rx_status_unknown = 0x0,
	htt_cmn_rx_status_ok,
	htt_cmn_rx_status_err_fcs,
	htt_cmn_rx_status_err_dup,
	htt_cmn_rx_status_err_replay,
	htt_cmn_rx_status_inv_peer,
	htt_cmn_rx_status_ctrl_mgmt_null = 0x08,
	htt_cmn_rx_status_tkip_mic_err = 0x09,
	htt_cmn_rx_status_decrypt_err = 0x0A,
	htt_cmn_rx_status_mpdu_length_err = 0x0B,
	htt_cmn_rx_status_err_misc = 0xFF
};


enum cdp_host_reo_dest_ring {
    cdp_host_reo_dest_ring_unknown = 0,
    cdp_host_reo_dest_ring_1 = 1,
    cdp_host_reo_dest_ring_2 = 2,
    cdp_host_reo_dest_ring_3 = 3,
    cdp_host_reo_dest_ring_4 = 4,
};

enum htt_cmn_t2h_en_stats_type {
    /* keep this alwyas first */
    HTT_CMN_T2H_EN_STATS_TYPE_START     = 0,

    /** ppdu_common_stats is the payload */
    HTT_CMN_T2H_EN_STATS_TYPE_COMMON    = 1,
    /** ppdu_sant_stats is the payload */
    HTT_CMN_T2H_EN_STATS_TYPE_SANT      = 2,
    /** ppdu_common_stats_v2 is the payload */
    HTT_CMN_T2H_EN_STATS_TYPE_COMMON_V2 = 3,

    /* Keep this last */
    HTT_CMN_T2H_EN_STATS_TYPE_END       = 0x1f,
};

enum htt_cmn_t2h_en_stats_status {
    /* Keep this first always */
    HTT_CMN_T2H_EN_STATS_STATUS_PARTIAL     = 0,
    HTT_CMN_T2H_EN_STATS_STATUS_PRESENT     = 1,
    HTT_CMN_T2H_EN_STATS_STATUS_ERROR       = 2,
    HTT_CMN_T2H_EN_STATS_STATUS_INVALID     = 3,


    /* keep this always last */
    HTT_CMN_T2H_EN_STATS_STATUS_SERIES_DONE         = 7,
};

/**
 * struct ol_txrx_peer_state - Peer state information
 */
enum ol_txrx_peer_state {
	OL_TXRX_PEER_STATE_INVALID,
	OL_TXRX_PEER_STATE_DISC,    /* initial state */
	OL_TXRX_PEER_STATE_CONN,    /* authentication in progress */
	OL_TXRX_PEER_STATE_AUTH,    /* authentication successful */
};

/**
 * struct ol_txrx_ast_type - AST entry type information
 */
enum cdp_txrx_ast_entry_type {
	CDP_TXRX_AST_TYPE_NONE,	/* static ast entry for connected peer */
	CDP_TXRX_AST_TYPE_STATIC, /* static ast entry for connected peer */
	CDP_TXRX_AST_TYPE_SELF, /* static ast entry for self peer (STA mode) */
	CDP_TXRX_AST_TYPE_WDS,	/* WDS peer ast entry type*/
	CDP_TXRX_AST_TYPE_MEC,	/* Multicast echo ast entry type */
	CDP_TXRX_AST_TYPE_WDS_HM, /* HM WDS entry */
	CDP_TXRX_AST_TYPE_STA_BSS,	 /* BSS entry(STA mode) */
	CDP_TXRX_AST_TYPE_DA,	/* AST entry based on Destination address */
	CDP_TXRX_AST_TYPE_WDS_HM_SEC, /* HM WDS entry for secondary radio */
	CDP_TXRX_AST_TYPE_MAX
};

/*
 * cdp_ast_free_status: status passed to callback function before freeing ast
 * @CDP_TXRX_AST_DELETED - AST is deleted from FW and delete response received
 * @CDP_TXRX_AST_DELETE_IN_PROGRESS - AST delete command sent to FW and host
 *                                    is waiting for FW response
 */
enum cdp_ast_free_status {
	CDP_TXRX_AST_DELETED,
	CDP_TXRX_AST_DELETE_IN_PROGRESS,
};

/**
 * txrx_ast_free_cb - callback registered for ast free
 * @ctrl_soc: control path soc context
 * @cdp_soc: DP soc context
 * @cookie: cookie
 * @cdp_ast_free_status: ast free status
 */
typedef void (*txrx_ast_free_cb)(struct cdp_ctrl_objmgr_psoc *ctrl_soc,
				 struct cdp_soc *cdp_soc,
				 void *cookie,
				 enum cdp_ast_free_status);

/**
 *  struct cdp_ast_entry_info - AST entry information
 *  @peer_mac_addr: mac address of peer on which AST entry is added
 *  @type: ast entry type
 *  @vdev_id: vdev_id
 *  @pdev_id: pdev_id
 *  @peer_id: peer_id
 *
 *  This structure holds the ast entry information
 *
 */
struct cdp_ast_entry_info {
	uint8_t peer_mac_addr[QDF_MAC_ADDR_SIZE];
	enum cdp_txrx_ast_entry_type type;
	uint8_t vdev_id;
	uint8_t pdev_id;
	uint16_t peer_id;
};

#define MIC_SEQ_CTR_SIZE 6

enum cdp_rx_frame_type {
	cdp_rx_frame_type_802_11,
	cdp_rx_frame_type_802_3,
};

/**
 *  struct cdp_rx_mic_err_info - rx mic error information
 *  @frame_type: frame type - 0 - 802.11 frame
 *                          - 1 - 802.3 frame
 *  @data: 802.11 frame
 *  @ta_mac_addr: transmitter mac address
 *  @da_mac_addr: destination mac address
 *  @tsc: sequence number
 *  @key_id: Key ID
 *  @multicast: flag for multicast
 *  @vdev_id: vdev ID
 *
 *  This structure holds rx mic error information
 *
 */
struct cdp_rx_mic_err_info {
	uint8_t frame_type;
	uint8_t *data;
	struct qdf_mac_addr ta_mac_addr;
	struct qdf_mac_addr da_mac_addr;
	uint8_t tsc[MIC_SEQ_CTR_SIZE];
	uint8_t key_id;
	bool multicast;
	uint16_t vdev_id;
};

/**
 * struct cdp_sec_type - security type information
 */
enum cdp_sec_type {
    cdp_sec_type_none,
    cdp_sec_type_wep128,
    cdp_sec_type_wep104,
    cdp_sec_type_wep40,
    cdp_sec_type_tkip,
    cdp_sec_type_tkip_nomic,
    cdp_sec_type_aes_ccmp,
    cdp_sec_type_wapi,
    cdp_sec_type_aes_ccmp_256,
    cdp_sec_type_aes_gcmp,
    cdp_sec_type_aes_gcmp_256,

    /* keep this last! */
    cdp_num_sec_types
};

/**
 *  struct cdp_tx_exception_metadata - Exception path parameters
 *  @peer_id: Peer id of the peer
 *  @tid: Transmit Identifier
 *  @tx_encap_type: Transmit encap type (i.e. Raw, Native Wi-Fi, Ethernet)
 *  @sec_type: sec_type to be passed to HAL
 *  @is_tx_sniffer: Indicates if the packet has to be sniffed
 *  @ppdu_cookie: 16-bit ppdu cookie that has to be replayed back in completions
 *
 *  This structure holds the parameters needed in the exception path of tx
 *
 */
struct cdp_tx_exception_metadata {
	uint16_t peer_id;
	uint8_t tid;
	uint16_t tx_encap_type;
	enum cdp_sec_type sec_type;
	uint8_t is_tx_sniffer;
	uint16_t ppdu_cookie;
};

typedef struct cdp_soc_t *ol_txrx_soc_handle;

/**
 * ol_txrx_vdev_delete_cb - callback registered during vdev
 * detach
 */
typedef void (*ol_txrx_vdev_delete_cb)(void *context);

/**
 * ol_txrx_peer_unmap_sync_cb - callback registered during peer detach sync
 */
typedef QDF_STATUS(*ol_txrx_peer_unmap_sync_cb)(uint8_t vdev_id,
						 uint32_t peer_id_cnt,
						 uint16_t *peer_id_list);

/**
 * ol_txrx_pkt_direction - Packet Direction
 * @rx_direction: rx path packet
 * @tx_direction: tx path packet
 */
enum txrx_direction {
	rx_direction = 1,
	tx_direction = 0,
};

/**
 * cdp_capabilities- DP capabilities
 */
enum cdp_capabilities {
	CDP_CFG_DP_TSO,
	CDP_CFG_DP_LRO,
	CDP_CFG_DP_SG,
	CDP_CFG_DP_GRO,
	CDP_CFG_DP_OL_TX_CSUM,
	CDP_CFG_DP_OL_RX_CSUM,
	CDP_CFG_DP_RAWMODE,
	CDP_CFG_DP_PEER_FLOW_CTRL,
};

/**
 * ol_txrx_nbuf_classify - Packet classification object
 * @peer_id: unique peer identifier from fw
 * @tid: traffic identifier(could be overridden)
 * @pkt_tid: traffic identifier(cannot be overridden)
 * @pkt_tos: ip header tos value
 * @pkt_dscp: ip header dscp value
 * @tos: index value in map
 * @dscp: DSCP_TID map index
 * @is_mcast: multicast pkt check
 * @is_eap: eapol pkt check
 * @is_arp: arp pkt check
 * @is_tcp: tcp pkt check
 * @is_dhcp: dhcp pkt check
 * @is_igmp: igmp pkt check
 * @is_ipv4: ip version 4 pkt check
 * @is_ipv6: ip version 6 pkt check
 */
struct ol_txrx_nbuf_classify {
	uint16_t peer_id;
	uint8_t tid;
	uint8_t pkt_tid;
	uint8_t pkt_tos;
	uint8_t pkt_dscp;
	uint8_t tos;
	uint8_t dscp;
	uint8_t is_mcast;
	uint8_t is_eap;
	uint8_t is_arp;
	uint8_t is_tcp;
	uint8_t is_dhcp;
	uint8_t is_igmp;
	uint8_t is_ipv4;
	uint8_t is_ipv6;
};

/**
 * ol_osif_vdev_handle - paque handle for OS shim virtual device
 * object
 */
struct ol_osif_vdev_t;
typedef struct ol_osif_vdev_t *ol_osif_vdev_handle;

/**
 * wlan_op_mode - Virtual device operation mode
 * @wlan_op_mode_unknown: Unknown mode
 * @wlan_op_mode_ap: AP mode
 * @wlan_op_mode_ibss: IBSS mode
 * @wlan_op_mode_sta: STA (client) mode
 * @wlan_op_mode_monitor: Monitor mode
 * @wlan_op_mode_ocb: OCB mode
 */
enum wlan_op_mode {
	wlan_op_mode_unknown,
	wlan_op_mode_ap,
	wlan_op_mode_ibss,
	wlan_op_mode_sta,
	wlan_op_mode_monitor,
	wlan_op_mode_ocb,
	wlan_op_mode_ndi,
};

/**
 * enum wlan_op_subtype - Virtual device subtype
 * @wlan_op_subtype_none: Subtype not applicable
 * @wlan_op_subtype_p2p_device: P2P device
 * @wlan_op_subtye_p2p_cli: P2P Client
 * @wlan_op_subtype_p2p_go: P2P GO
 *
 * This enum lists the subtypes of a particular virtual
 * device.
 */
enum wlan_op_subtype {
	wlan_op_subtype_none,
	wlan_op_subtype_p2p_device,
	wlan_op_subtype_p2p_cli,
	wlan_op_subtype_p2p_go,
};

/**
 * connectivity_stats_pkt_status - data pkt type
 * @PKT_TYPE_REQ: Request packet
 * @PKT_TYPE_RSP: Response packet
 * @PKT_TYPE_TX_DROPPED: TX packet dropped
 * @PKT_TYPE_RX_DROPPED: RX packet dropped
 * @PKT_TYPE_RX_DELIVERED: RX packet delivered
 * @PKT_TYPE_RX_REFUSED: RX packet refused
 * @PKT_TYPE_TX_HOST_FW_SENT: TX packet FW sent
 * @PKT_TYPE_TX_ACK_CNT:TC packet acked
 * @PKT_TYPE_NONE: Invalid packet
 */
enum connectivity_stats_pkt_status {
	PKT_TYPE_REQ,
	PKT_TYPE_RSP,
	PKT_TYPE_TX_DROPPED,
	PKT_TYPE_RX_DROPPED,
	PKT_TYPE_RX_DELIVERED,
	PKT_TYPE_RX_REFUSED,
	PKT_TYPE_TX_HOST_FW_SENT,
	PKT_TYPE_TX_ACK_CNT,
	PKT_TYPE_NONE,
};

/**
 * ol_txrx_mgmt_tx_cb - tx management delivery notification
 * callback function
 */
typedef void
(*ol_txrx_mgmt_tx_cb)(void *ctxt, qdf_nbuf_t tx_mgmt_frm, int had_error);

/**
 * ol_txrx_data_tx_cb - Function registered with the data path
 * that is called when tx frames marked as "no free" are
 * done being transmitted
 */
typedef void
(*ol_txrx_data_tx_cb)(void *ctxt, qdf_nbuf_t tx_frm, int had_error);

/**
 * ol_txrx_tx_fp - top-level transmit function
 * @soc - dp soc handle
 * @vdev_id - handle to the virtual device object
 * @msdu_list - list of network buffers
 */
typedef qdf_nbuf_t (*ol_txrx_tx_fp)(struct cdp_soc_t *soc, uint8_t vdev_id,
				    qdf_nbuf_t msdu_list);

/**
 * ol_txrx_tx_exc_fp - top-level transmit function on exception path
 * @data_vdev - handle to the virtual device object
 * @msdu_list - list of network buffers
 * @tx_exc_metadata - structure that holds parameters to exception path
 */
typedef qdf_nbuf_t (*ol_txrx_tx_exc_fp)(struct cdp_vdev *data_vdev,
					qdf_nbuf_t msdu_list,
					struct cdp_tx_exception_metadata
						*tx_exc_metadata);

/**
 * ol_txrx_completion_fp - top-level transmit function
 * for tx completion
 * @skb: skb data
 * @osif_dev: the virtual device's OS shim object
 */
typedef void (*ol_txrx_completion_fp)(qdf_nbuf_t skb,
				      void *osif_dev);
/**
 * ol_txrx_tx_flow_control_fp - tx flow control notification
 * function from txrx to OS shim
 * @osif_dev - the virtual device's OS shim object
 * @tx_resume - tx os q should be resumed or not
 */
typedef void (*ol_txrx_tx_flow_control_fp)(void *osif_dev,
					    bool tx_resume);

/**
 * ol_txrx_tx_flow_control_is_pause_fp - is tx paused by flow control
 * function from txrx to OS shim
 * @osif_dev - the virtual device's OS shim object
 *
 * Return: true if tx is paused by flow control
 */
typedef bool (*ol_txrx_tx_flow_control_is_pause_fp)(void *osif_dev);

/**
 * ol_txrx_rx_fp - receive function to hand batches of data
 * frames from txrx to OS shim
 * @data_vdev - handle to the OSIF virtual device object
 * @msdu_list - list of network buffers
 */
typedef QDF_STATUS(*ol_txrx_rx_fp)(void *osif_dev, qdf_nbuf_t msdu_list);

/**
 * ol_txrx_rx_flush_fp - receive function to hand batches of data
 * frames from txrx to OS shim
 * @osif_dev: handle to the OSIF virtual device object
 * @vdev_id: vdev_if of the packets to be flushed
 */
typedef QDF_STATUS(*ol_txrx_rx_flush_fp)(void *osif_dev, uint8_t vdev_id);

/**
 * ol_txrx_rx_gro_flush_ind - function to send GRO flush indication to stack
 * for a given RX Context Id.
 * @osif_dev - handle to the OSIF virtual device object
 * @rx_ctx_id - Rx context Id for which gro flush should happen
 */
typedef QDF_STATUS(*ol_txrx_rx_gro_flush_ind_fp)(void *osif_dev,
						 int rx_ctx_id);

/**
 * ol_txrx_stats_rx_fp - receive function to hand batches of data
 * frames from txrx to OS shim
 * @skb: skb data
 * @osif_dev: the virtual device's OS shim object
 * @action: data packet type
 * @pkt_type: packet data type
 */
typedef void (*ol_txrx_stats_rx_fp)(struct sk_buff *skb,
		void *osif_dev, enum connectivity_stats_pkt_status action,
		uint8_t *pkt_type);

/**
 * ol_txrx_get_key_fp - function to gey key based on keyix and peer
 * mac address
 * @osif_dev - the virtual device's OS shim object
 * @key_buf - pointer to store key
 * @mac_addr - pointer to mac address
 * @keyix - key id
 */
typedef QDF_STATUS(*ol_txrx_get_key_fp)(void *osif_dev, uint8_t *key_buf, uint8_t *mac_addr, uint8_t keyix);

/**
 * ol_txrx_rsim_rx_decap_fp - raw mode simulation function to decap the
 * packets in receive path.
 * @osif_dev  - the virtual device's OS shim object
 * @list_head - poniter to head of receive packet queue to decap
 * @list_tail - poniter to tail of receive packet queue to decap
 * @peer      - Peer handler
 */
typedef QDF_STATUS(*ol_txrx_rsim_rx_decap_fp)(void *osif_dev,
						qdf_nbuf_t *list_head,
						qdf_nbuf_t *list_tail,
						struct cdp_peer *peer);

/* ol_txrx_rx_fp - external tx free function to read per packet stats and
 *                            free tx buffer externally
 * @netbuf - tx network buffer
 */
typedef void (*ol_txrx_tx_free_ext_fp)(qdf_nbuf_t netbuf);

/**
 * ol_txrx_rx_check_wai_fp - OSIF WAPI receive function
*/
typedef bool (*ol_txrx_rx_check_wai_fp)(ol_osif_vdev_handle vdev,
					    qdf_nbuf_t mpdu_head,
					    qdf_nbuf_t mpdu_tail);
/**
 * ol_txrx_rx_mon_fp - OSIF monitor mode receive function for single
 * MPDU (802.11 format)
 */
typedef void (*ol_txrx_rx_mon_fp)(ol_osif_vdev_handle vdev,
					    qdf_nbuf_t mpdu,
					    void *rx_status);

/**
 * ol_txrx_proxy_arp_fp - proxy arp function pointer
*/
typedef int (*ol_txrx_proxy_arp_fp)(ol_osif_vdev_handle vdev,
					    qdf_nbuf_t netbuf);

/*
 * ol_txrx_mcast_me_fp - function pointer for multicast enhancement
 */
typedef int (*ol_txrx_mcast_me_fp)(ol_osif_vdev_handle vdev,
						qdf_nbuf_t netbuf);

/**
 * ol_txrx_stats_callback - statistics notify callback
 */
typedef void (*ol_txrx_stats_callback)(void *ctxt,
				       enum htt_cmn_dbg_stats_type type,
				       uint8_t *buf, int bytes);

/**
 * ol_txrx_pktdump_cb - callback for packet dump feature
 */
typedef void (*ol_txrx_pktdump_cb)(ol_txrx_soc_handle soc,
				   uint8_t pdev_id,
				   uint8_t vdev_id,
				   qdf_nbuf_t netbuf,
				   uint8_t status,
				   uint8_t type);

/**
 * ol_txrx_ops - (pointers to) the functions used for tx and rx
 * data xfer
 *
 * There are two portions of these txrx operations.
 * The rx portion is filled in by OSIF SW before calling
 * ol_txrx_osif_vdev_register; inside the ol_txrx_osif_vdev_register
 * the txrx SW stores a copy of these rx function pointers, to use
 * as it delivers rx data frames to the OSIF SW.
 * The tx portion is filled in by the txrx SW inside
 * ol_txrx_osif_vdev_register; when the function call returns,
 * the OSIF SW stores a copy of these tx functions to use as it
 * delivers tx data frames to the txrx SW.
 *
 * @tx.std -  the tx function pointer for standard data
 * frames This function pointer is set by the txrx SW
 * perform host-side transmit operations based on
 * whether a HL or LL host/target interface is in use.
 * @tx.flow_control_cb - the transmit flow control
 * function that is registered by the
 * OSIF which is called from txrx to
 * indicate whether the transmit OS
 * queues should be paused/resumed
 * @rx.rx - the OS shim rx function to deliver rx data
 * frames to. This can have different values for
 * different virtual devices, e.g. so one virtual
 * device's OS shim directly hands rx frames to the OS,
 * but another virtual device's OS shim filters out P2P
 * messages before sending the rx frames to the OS. The
 * netbufs delivered to the osif_rx function are in the
 * format specified by the OS to use for tx and rx
 * frames (either 802.3 or native WiFi). In case RX Threads are enabled, pkts
 * are given to the thread, instead of the stack via this pointer.
 * @rx.stack - function to give packets to the stack. Differs from @rx.rx.
 * In case RX Threads are enabled, this pointer holds the callback to give
 * packets to the stack.
 * @rx.rx_gro_flush - GRO flush indication to stack for a given RX CTX ID
 * @rx.wai_check - the tx function pointer for WAPI frames
 * @rx.mon - the OS shim rx monitor function to deliver
 * monitor data to Though in practice, it is probable
 * that the same function will be used for delivering
 * rx monitor data for all virtual devices, in theory
 * each different virtual device can have a different
 * OS shim function for accepting rx monitor data. The
 * netbufs delivered to the osif_rx_mon function are in
 * 802.11 format.  Each netbuf holds a 802.11 MPDU, not
 * an 802.11 MSDU. Depending on compile-time
 * configuration, each netbuf may also have a
 * monitor-mode encapsulation header such as a radiotap
 * header added before the MPDU contents.
 * @rx.std - the OS shim rx function to deliver rx data
 * @proxy_arp - proxy arp function pointer - specified by
 * OS shim, stored by txrx
 * @get_key - function pointer to get key of the peer with
 * specific key index
 */
struct ol_txrx_ops {
	/* tx function pointers - specified by txrx, stored by OS shim */
	struct {
		ol_txrx_tx_fp         tx;
		ol_txrx_tx_exc_fp     tx_exception;
		ol_txrx_tx_free_ext_fp tx_free_ext;
		ol_txrx_completion_fp tx_comp;
	} tx;

	/* rx function pointers - specified by OS shim, stored by txrx */
	struct {
		ol_txrx_rx_fp           rx;
		ol_txrx_rx_fp           rx_stack;
		ol_txrx_rx_flush_fp     rx_flush;
		ol_txrx_rx_gro_flush_ind_fp           rx_gro_flush;
		ol_txrx_rx_check_wai_fp wai_check;
		ol_txrx_rx_mon_fp       mon;
		ol_txrx_stats_rx_fp           stats_rx;
		ol_txrx_rsim_rx_decap_fp rsim_rx_decap;
	} rx;
	/* proxy arp function pointer - specified by OS shim, stored by txrx */
	ol_txrx_proxy_arp_fp      proxy_arp;
	ol_txrx_mcast_me_fp          me_convert;

	ol_txrx_get_key_fp  get_key;
};

/**
 * ol_txrx_stats_req - specifications of the requested
 * statistics
 */
struct ol_txrx_stats_req {
	uint32_t stats_type_upload_mask;        /* which stats to upload */
	uint32_t stats_type_reset_mask; /* which stats to reset */

	/* stats will be printed if either print element is set */
	struct {
		int verbose;    /* verbose stats printout */
		int concise;    /* concise stats printout (takes precedence) */
	} print;                /* print uploaded stats */

	/* stats notify callback will be invoked if fp is non-NULL */
	struct {
		ol_txrx_stats_callback fp;
		void *ctxt;
	} callback;

	/* stats will be copied into the specified buffer if buf is non-NULL */
	struct {
		uint8_t *buf;
		int byte_limit; /* don't copy more than this */
	} copy;

	/*
	 * If blocking is true, the caller will take the specified semaphore
	 * to wait for the stats to be uploaded, and the driver will release
	 * the semaphore when the stats are done being uploaded.
	 */
	struct {
		int blocking;
		/*Note: this needs to change to some qdf_* type */
		qdf_semaphore_t *sem_ptr;
	} wait;
};


/* DP soc struct definition */
struct cdp_soc_t {
	struct cdp_ops *ops;
	struct ol_if_ops *ol_ops;
};

/*
 * cdp_pdev_param_type: different types of parameters
 *			to set values in pdev
 * @CDP_CONFIG_DEBUG_SNIFFER: Enable debug sniffer feature
 * @CDP_CONFIG_BPR_ENABLE: Enable bcast probe feature
 * @CDP_CONFIG_PRIMARY_RADIO: Configure radio as primary
 * @CDP_CONFIG_ENABLE_PERPKT_TXSTATS: Enable per packet statistics
 * @CDP_CONFIG_IGMPMLD_OVERRIDE: Override IGMP/MLD
 * @CDP_CONFIG_IGMPMLD_TID: Configurable TID value when igmmld_override is set
 * @CDP_CONFIG_ARP_DBG_CONF: Enable ARP debug
 * @CDP_CONFIG_CAPTURE_LATENCY: Capture time latency
 * @CDP_INGRESS_STATS: Accumulate ingress statistics
 * @CDP_OSIF_DROP: Accumulate drops in OSIF layer
 * @CDP_CONFIG_ENH_RX_CAPTURE: Enable enhanced RX capture
 */
enum cdp_pdev_param_type {
	CDP_CONFIG_DEBUG_SNIFFER,
	CDP_CONFIG_BPR_ENABLE,
	CDP_CONFIG_PRIMARY_RADIO,
	CDP_CONFIG_ENABLE_PERPKT_TXSTATS,
	CDP_CONFIG_IGMPMLD_OVERRIDE,
	CDP_CONFIG_IGMPMLD_TID,
	CDP_CONFIG_ARP_DBG_CONF,
	CDP_CONFIG_CAPTURE_LATENCY,
	CDP_INGRESS_STATS,
	CDP_OSIF_DROP,
	CDP_CONFIG_ENH_RX_CAPTURE,
	CDP_CONFIG_TX_CAPTURE,
};

/**
 * cdp_rx_enh_capture_mode - Rx enhanced capture modes
 * @CDP_RX_ENH_CAPTURE_DISABLED: Disable Rx enhance capture
 * @CDP_RX_ENH_CAPTURE_MPDU: Enable capture of 128 bytes of each MPDU
 * @CDP_RX_ENH_CAPTURE_MPDU_MSDU: Enable capture of 128 bytes of each MSDU
 */
enum cdp_rx_enh_capture_mode {
	CDP_RX_ENH_CAPTURE_DISABLED = 0,
	CDP_RX_ENH_CAPTURE_MPDU,
	CDP_RX_ENH_CAPTURE_MPDU_MSDU,
};

/**
 * cdp_rx_enh_capture_peer - Rx enhanced capture peer filtering
 * @CDP_RX_ENH_CAPTURE_PEER_DISABLED: Disable Rx ENH capture peer filtering
 * @CDP_RX_ENH_CAPTURE_PEER_ENABLED: Enable Rx ENH capture peer filtering
 */
enum cdp_rx_enh_capture_peer {
	CDP_RX_ENH_CAPTURE_PEER_DISABLED = 0,
	CDP_RX_ENH_CAPTURE_PEER_ENABLED,
};

/**
 * cdp_tx_enh_capture_mode - Tx enhanced capture modes
 * @CDP_TX_ENH_CAPTURE_DISABLED: Disable Tx enhance capture for all peers
 * @CDP_TX_ENH_CAPTURE_ENABLE_ALL_PEERS: Enable tx capture for all peers
 * @CDP_TX_ENH_CAPTURE_ENDIS_PER_PEER: Enable/disable per peer as necessary
 */
enum cdp_tx_enh_capture_mode {
	CDP_TX_ENH_CAPTURE_DISABLED = 0,
	CDP_TX_ENH_CAPTURE_ENABLE_ALL_PEERS,
	CDP_TX_ENH_CAPTURE_ENDIS_PER_PEER,
};

/*
 * enum cdp_pdev_bpr_param - different types of parameters
 *			     to set value in pdev
 * @CDP_BPR_DISABLE: Set bpr to disable state
 * @CDP_BPR_ENABLE: set bpr to enable state
 *
 * Enum indicating bpr state to enable/disable.
 */
enum cdp_pdev_bpr_param {
	CDP_BPR_DISABLE,
	CDP_BPR_ENABLE,
};

/*
 * cdp_vdev_param_type: different types of parameters
 *			to set values in vdev
 * @CDP_ENABLE_NAWDS: set nawds enable/disable
 * @CDP_ENABLE_MCAST_EN: enable/disable multicast enhancement
 * @CDP_ENABLE_WDS: wds sta
 * @CDP_ENABLE_MEC: MEC enable flags
 * @CDP_ENABLE_PROXYSTA: proxy sta
 * @CDP_UPDATE_TDLS_FLAGS: tdls link flags
 * @CDP_ENABLE_AP_BRIDGE: set ap_bridging enable/disable
 * @CDP_ENABLE_CIPHER : set cipher type based on security
 * @CDP_ENABLE_QWRAP_ISOLATION: qwrap isolation mode
 */
enum cdp_vdev_param_type {
	CDP_ENABLE_NAWDS,
	CDP_ENABLE_MCAST_EN,
	CDP_ENABLE_WDS,
	CDP_ENABLE_MEC,
	CDP_ENABLE_DA_WAR,
	CDP_ENABLE_PROXYSTA,
	CDP_UPDATE_TDLS_FLAGS,
	CDP_CFG_WDS_AGING_TIMER,
	CDP_ENABLE_AP_BRIDGE,
	CDP_ENABLE_CIPHER,
	CDP_ENABLE_QWRAP_ISOLATION,
	CDP_UPDATE_MULTIPASS
};

#define TXRX_FW_STATS_TXSTATS                     1
#define TXRX_FW_STATS_RXSTATS                     2
#define TXRX_FW_STATS_RX_RATE_INFO                3
#define TXRX_FW_STATS_PHYSTATS                    4
#define TXRX_FW_STATS_PHYSTATS_CONCISE            5
#define TXRX_FW_STATS_TX_RATE_INFO                6
#define TXRX_FW_STATS_TID_STATE                   7
#define TXRX_FW_STATS_HOST_STATS                  8
#define TXRX_FW_STATS_CLEAR_HOST_STATS            9
#define TXRX_FW_STATS_CE_STATS                   10
#define TXRX_FW_STATS_VOW_UMAC_COUNTER           11
#define TXRX_FW_STATS_ME_STATS                   12
#define TXRX_FW_STATS_TXBF_INFO                  13
#define TXRX_FW_STATS_SND_INFO                   14
#define TXRX_FW_STATS_ERROR_INFO                 15
#define TXRX_FW_STATS_TX_SELFGEN_INFO            16
#define TXRX_FW_STATS_TX_MU_INFO                 17
#define TXRX_FW_SIFS_RESP_INFO                   18
#define TXRX_FW_RESET_STATS                      19
#define TXRX_FW_MAC_WDOG_STATS                   20
#define TXRX_FW_MAC_DESC_STATS                   21
#define TXRX_FW_MAC_FETCH_MGR_STATS              22
#define TXRX_FW_MAC_PREFETCH_MGR_STATS           23
#define TXRX_FW_STATS_DURATION_INFO              24
#define TXRX_FW_STATS_DURATION_INFO_RESET        25
#define TXRX_FW_HALPHY_STATS                     26
#define TXRX_FW_COEX_STATS                       27

#define PER_RADIO_FW_STATS_REQUEST 0
#define PER_VDEV_FW_STATS_REQUEST 1
/**
 * enum data_stall_log_event_indicator - Module triggering data stall
 * @DATA_STALL_LOG_INDICATOR_UNUSED: Unused
 * @DATA_STALL_LOG_INDICATOR_HOST_DRIVER: Host driver indicates data stall
 * @DATA_STALL_LOG_INDICATOR_FIRMWARE: FW indicates data stall
 * @DATA_STALL_LOG_INDICATOR_FRAMEWORK: Framework indicates data stall
 *
 * Enum indicating the module that indicates data stall event
 */
enum data_stall_log_event_indicator {
	DATA_STALL_LOG_INDICATOR_UNUSED,
	DATA_STALL_LOG_INDICATOR_HOST_DRIVER,
	DATA_STALL_LOG_INDICATOR_FIRMWARE,
	DATA_STALL_LOG_INDICATOR_FRAMEWORK,
};

/**
 * enum data_stall_log_event_type - data stall event type
 * @DATA_STALL_LOG_NONE
 * @DATA_STALL_LOG_FW_VDEV_PAUSE
 * @DATA_STALL_LOG_HWSCHED_CMD_FILTER
 * @DATA_STALL_LOG_HWSCHED_CMD_FLUSH
 * @DATA_STALL_LOG_FW_RX_REFILL_FAILED
 * @DATA_STALL_LOG_FW_RX_FCS_LEN_ERROR
 * @DATA_STALL_LOG_FW_WDOG_ERRORS
 * @DATA_STALL_LOG_BB_WDOG_ERROR
 * @DATA_STALL_LOG_POST_TIM_NO_TXRX_ERROR
 * @DATA_STALL_LOG_HOST_STA_TX_TIMEOUT
 * @DATA_STALL_LOG_HOST_SOFTAP_TX_TIMEOUT
 * @DATA_STALL_LOG_NUD_FAILURE
 *
 * Enum indicating data stall event type
 */
enum data_stall_log_event_type {
	DATA_STALL_LOG_NONE,
	DATA_STALL_LOG_FW_VDEV_PAUSE,
	DATA_STALL_LOG_HWSCHED_CMD_FILTER,
	DATA_STALL_LOG_HWSCHED_CMD_FLUSH,
	DATA_STALL_LOG_FW_RX_REFILL_FAILED,
	DATA_STALL_LOG_FW_RX_FCS_LEN_ERROR,
	DATA_STALL_LOG_FW_WDOG_ERRORS,
	DATA_STALL_LOG_BB_WDOG_ERROR,
	DATA_STALL_LOG_POST_TIM_NO_TXRX_ERROR,
	/* Stall events triggered by host/framework start from 0x100 onwards. */
	DATA_STALL_LOG_HOST_STA_TX_TIMEOUT = 0x100,
	DATA_STALL_LOG_HOST_SOFTAP_TX_TIMEOUT,
	DATA_STALL_LOG_NUD_FAILURE,
};

/**
 * enum data_stall_log_recovery_type - data stall recovery type
 * @DATA_STALL_LOG_RECOVERY_NONE,
 * @DATA_STALL_LOG_RECOVERY_CONNECT_DISCONNECT,
 * @DATA_STALL_LOG_RECOVERY_TRIGGER_PDR
 *
 * Enum indicating data stall recovery type
 */
enum data_stall_log_recovery_type {
	DATA_STALL_LOG_RECOVERY_NONE = 0,
	DATA_STALL_LOG_RECOVERY_CONNECT_DISCONNECT,
	DATA_STALL_LOG_RECOVERY_TRIGGER_PDR,
};

/**
 * struct data_stall_event_info - data stall info
 * @indicator: Module triggering data stall
 * @data_stall_type: data stall event type
 * @vdev_id_bitmap: vdev_id_bitmap
 * @pdev_id: pdev id
 * @recovery_type: data stall recovery type
 */
struct data_stall_event_info {
	uint32_t indicator;
	uint32_t data_stall_type;
	uint32_t vdev_id_bitmap;
	uint32_t pdev_id;
	uint32_t recovery_type;
};

typedef void (*data_stall_detect_cb)(struct data_stall_event_info *);

/*
 * enum cdp_stats - options for host and firmware
 * statistics
 * @CDP_TXRX_STATS_1: HTT Pdev tx stats
 * @CDP_TXRX_STATS_2: HTT Pdev rx stats
 * @CDP_TXRX_STATS_3: HTT Pdev Tx HW Queue stats
 * @CDP_TXRX_STATS_4: HTT Pdev Tx HW Sched stats
 * @CDP_TXRX_STATS_5: HTT Pdev error stats
 * @CDP_TXRX_STATS_6: HTT TQM stats
 * @CDP_TXRX_STATS_7: HTT TQM CMDQ stats
 * @CDP_TXRX_STATS_8: HTT Tx_de_cmn thread stats
 * @CDP_TXRX_STATS_9: HTT Pdev Tx rate stats
 * @CDP_TXRX_STATS_10: HTT Pdev Rx rate stats
 * @CDP_TXRX_STATS_11: HTT Peer stats
 * @CDP_TXRX_STATS_12: HTT Tx Self Gen Info
 * @CDP_TXRX_STATS_13: HTT Tx MU HWQ stats
 * @CDP_TXRX_STATS_14: HTT Ring interface info stats
 * @CDP_TXRX_STATS_15: HTT SRNG info stats
 * @CDP_TXRX_STATS_16: HTT SFM info stats
 * @CDP_TXRX_STATS_17: HTT Pdev tx mu mimo sched info
 * @CDP_TXRX_STATS_18: HTT Peer list details
 * @CDP_TXRX_STATS_19: Reserved
 * @CDP_TXRX_STATS_20: Reset Host stats
 * @CDP_TXRX_STATS_21: Host Rx rate stats
 * @CDP_TXRX_STATS_22: Host Tx rate stats
 * @CDP_TXRX_STATS_23: Host Tx stats
 * @CDP_TXRX_STATS_24: Host Rx stats
 * @CDP_TXRX_STATS_25: Host Ast stats
 * @CDP_TXRX_STATS_26: Host Head/Tail Pointer stats
 * @CDP_TXRX_STATS_27: Host Monitor mode stats
 * @CDP_TXRX_STATS_28: Host Peer entry stats
 * @CDP_TXRX_STATS_29: Host Soc config params info
 * @CDP_TXRX_STATS_30: Host Pdev config params info
 * @CDP_TXRX_STATS_31: Host DP Interrupt Stats
 */
enum cdp_stats {
	CDP_TXRX_STATS_0  = 0,
	CDP_TXRX_STATS_1,
	CDP_TXRX_STATS_2,
	CDP_TXRX_STATS_3,
	CDP_TXRX_STATS_4,
	CDP_TXRX_STATS_5,
	CDP_TXRX_STATS_6,
	CDP_TXRX_STATS_7,
	CDP_TXRX_STATS_8,
	CDP_TXRX_STATS_9,
	CDP_TXRX_STATS_10,
	CDP_TXRX_STATS_11,
	CDP_TXRX_STATS_12,
	CDP_TXRX_STATS_13,
	CDP_TXRX_STATS_14,
	CDP_TXRX_STATS_15,
	CDP_TXRX_STATS_16,
	CDP_TXRX_STATS_17,
	CDP_TXRX_STATS_18,
	CDP_TXRX_STATS_19,
	CDP_TXRX_STATS_20,
	CDP_TXRX_STATS_21,
	CDP_TXRX_STATS_22,
	CDP_TXRX_STATS_23,
	CDP_TXRX_STATS_24,
	CDP_TXRX_STATS_25,
	CDP_TXRX_STATS_26,
	CDP_TXRX_STATS_27,
	CDP_TXRX_STATS_28,
	CDP_TXRX_STATS_29,
	CDP_TXRX_STATS_30,
	CDP_TXRX_STATS_31,
	CDP_TXRX_STATS_HTT_MAX = 256,
	CDP_TXRX_MAX_STATS = 265,
};

/*
 * Different Stat update types sent to OL_IF
 * @UPDATE_PEER_STATS: update peer stats
 * @UPDATE_VDEV_STATS: update vdev stats
 * @UPDATE_PDE_STATS: Update pdev stats
 */
enum cdp_stat_update_type {
	UPDATE_PEER_STATS = 0,
	UPDATE_VDEV_STATS = 1,
	UPDATE_PDEV_STATS = 2,
};

/*
 * struct cdp_tx_sojourn_stats - Tx sojourn stats
 * @ppdu_seq_id: ppdu_seq_id from tx completion
 * @avg_sojourn_msdu: average sojourn msdu time
 * @sum_sojourn_msdu: sum sojourn msdu time
 * @num_msdu: number of msdus per ppdu
 * @cookie: cookie to be used by upper layer
 */
struct cdp_tx_sojourn_stats {
	uint32_t ppdu_seq_id;
	qdf_ewma_tx_lag avg_sojourn_msdu[CDP_DATA_TID_MAX];
	uint32_t sum_sojourn_msdu[CDP_DATA_TID_MAX];
	uint32_t num_msdus[CDP_DATA_TID_MAX];
	struct cdp_stats_cookie *cookie;
};

/**
 * struct cdp_delayed_tx_completion_ppdu_user - Delayed Tx PPDU completion
 * per-user information
 * @frame_ctrl: frame control field in 802.11 header
 * @qos_ctrl: QoS control field in 802.11 header
 * @mpdu_tried: number of mpdus tried
 * @ltf_size: ltf_size
 * @stbc: stbc
 * @he_re: he_re (range extension)
 * @txbf: txbf
 * @bw: Transmission bandwidth
 *       <enum 2 transmit_bw_20_MHz>
 *       <enum 3 transmit_bw_40_MHz>
 *       <enum 4 transmit_bw_80_MHz>
 *       <enum 5 transmit_bw_160_MHz>
 * @nss: NSS 1,2, ...8
 * @mcs: MCS index
 * @preamble: preamble
 * @gi: guard interval 800/400/1600/3200 ns
 * @dcm: dcm
 * @ldpc: ldpc
 * @ru_start: RU start index
 * @ru_tones: RU tones length
 * @is_mcast: MCAST or UCAST
 * @user_pos: user position
 * @mu_group_id: mu group id
 */
struct cdp_delayed_tx_completion_ppdu_user {
	uint32_t frame_ctrl:16,
		 qos_ctrl:16;
	uint32_t mpdu_tried_ucast:16,
		mpdu_tried_mcast:16;
	uint32_t ltf_size:2,
		 stbc:1,
		 he_re:1,
		 txbf:4,
		 bw:4,
		 nss:4,
		 mcs:4,
		 preamble:4,
		 gi:4,
		 dcm:1,
		 ldpc:1,
		 delayed_ba:1;
	uint16_t ru_start;
	uint16_t ru_tones;
	bool is_mcast;
	uint32_t user_pos;
	uint32_t mu_group_id;
};

/**
 * struct cdp_tx_completion_ppdu_user - Tx PPDU completion per-user information
 * @completion_status: completion status - OK/Filter/Abort/Timeout
 * @tid: TID number
 * @peer_id: Peer ID
 * @ba_size: Block-Ack size
 * @frame_ctrl: frame control field in 802.11 header
 * @qos_ctrl: QoS control field in 802.11 header
 * @mpdu_tried: number of mpdus tried
 * @mpdu_success: number of mpdus successfully transmitted
 * @long_retries: long retries
 * @short_retries: short retries
 * @is_ampdu: mpdu aggregate or non-aggregate?
 * @success_bytes: bytes successfully transmitted
 * @retry_bytes: bytes retried
 * @failed_msdus: MSDUs failed transmission
 * @duration: user duration in ppdu
 * @ltf_size: ltf_size
 * @stbc: stbc
 * @he_re: he_re (range extension)
 * @txbf: txbf
 * @bw: Transmission bandwidth
 *       <enum 2 transmit_bw_20_MHz>
 *       <enum 3 transmit_bw_40_MHz>
 *       <enum 4 transmit_bw_80_MHz>
 *       <enum 5 transmit_bw_160_MHz>
 * @nss: NSS 1,2, ...8
 * @mcs: MCS index
 * @preamble: preamble
 * @gi: guard interval 800/400/1600/3200 ns
 * @dcm: dcm
 * @ldpc: ldpc
 * @delayed_ba: delayed ba bit
 * @ppdu_type: SU/MU_MIMO/MU_OFDMA/MU_MIMO_OFDMA/UL_TRIG/BURST_BCN/UL_BSR_RESP/
 * UL_BSR_TRIG/UNKNOWN
 * @ba_seq_no: Block Ack sequence number
 * @ba_bitmap: Block Ack bitmap
 * @start_seqa: Sequence number of first MPDU
 * @enq_bitmap: Enqueue MPDU bitmap
 * @ru_start: RU start index
 * @ru_tones: RU tones length
 * @is_mcast: MCAST or UCAST
 * @tx_rate: Transmission Rate
 * @user_pos: user position
 * @mu_group_id: mu group id
 * @rix: rate index
 * @cookie: cookie to used by upper layer
 * @is_ppdu_cookie_valid : Indicates that ppdu_cookie is valid
 * @ppdu_cookie: 16-bit ppdu_cookie
 * @sa_is_training: smart antenna training packets indication
 * @rssi_chain: rssi chain per bandwidth
 * @sa_tx_antenna: antenna in which packet is transmitted
 * @sa_max_rates: smart antenna tx feedback info max rates
 * @sa_goodput: smart antenna tx feedback info goodput
 * @current_rate_per: Moving average per
 * @last_enq_seq: last equeue sequence number
 */
struct cdp_tx_completion_ppdu_user {
	uint32_t completion_status:8,
		 tid:8,
		 peer_id:16;
	uint8_t mac_addr[6];
	uint16_t ba_size;
	uint32_t frame_ctrl:16,
		 qos_ctrl:16;
	uint32_t mpdu_tried_ucast:16,
		mpdu_tried_mcast:16;
	uint16_t mpdu_success:16;
	uint16_t mpdu_failed:16;
	uint32_t long_retries:4,
		 short_retries:4,
		 tx_ratecode:16,
		 is_ampdu:1,
		 ppdu_type:5;
	uint32_t success_bytes;
	uint32_t retry_bytes;
	uint32_t failed_bytes;
	uint32_t success_msdus:16,
		 retry_msdus:16;
	uint32_t failed_msdus:16,
		 duration:16;
	uint32_t ltf_size:2,
		 stbc:1,
		 he_re:1,
		 txbf:4,
		 bw:4,
		 nss:4,
		 mcs:4,
		 preamble:4,
		 gi:4,
		 dcm:1,
		 ldpc:1,
		 delayed_ba:1;
	uint32_t ba_seq_no;
	uint32_t ba_bitmap[CDP_BA_256_BIT_MAP_SIZE_DWORDS];
	uint32_t start_seq;
	uint32_t enq_bitmap[CDP_BA_256_BIT_MAP_SIZE_DWORDS];
	uint32_t failed_bitmap[CDP_BA_256_BIT_MAP_SIZE_DWORDS];
	uint32_t num_mpdu:9,
		 num_msdu:16;
	uint32_t tx_duration;
	uint16_t ru_start;
	uint16_t ru_tones;
	bool is_mcast;
	uint32_t tx_rate;
	uint32_t tx_ratekbps;
	/*ack rssi for separate chains*/
	uint32_t ack_rssi[CDP_RSSI_CHAIN_LEN];
	bool ack_rssi_valid;
	uint32_t user_pos;
	uint32_t mu_group_id;
	uint32_t rix;
	struct cdp_stats_cookie *cookie;
	uint8_t is_ppdu_cookie_valid;
	uint16_t ppdu_cookie;
	uint8_t sa_is_training;
	uint32_t rssi_chain[CDP_RSSI_CHAIN_LEN];
	uint32_t sa_tx_antenna;
	/*Max rates for BW: 20MHZ, 40MHZ and 80MHZ and 160MHZ
	 * |---------------------------------------|
	 * | 16 bits | 16 bits | 16 bits | 16 bits |
	 * |   BW-1  |   BW-2  |   BW-3  |   BW-4  |
	 * |      /\  \                            |
	 * |     /  \  \                           |
	 * |    /    \  \                          |
	 * |   /      \  \                         |
	 * |  /        \  \                        |
	 * | /          \  \                       |
	 * |/            \  \                      |
	 * |[11|8]     [5|8] \                     |
	 * | BW1      PADDED  \                    |
	 * |---------------------------------------|
	 */
	uint16_t sa_max_rates[CDP_NUM_SA_BW];
	uint32_t sa_goodput;
	/* below field is used to calculate goodput in non-training period
	 * Note: As host is exposing goodput and hence current_rate_per is
	 * of no use. It is just for Host computation.
	 */
	uint32_t current_rate_per;
	uint32_t last_enq_seq;
};

/**
 * struct cdp_tx_indication_mpdu_info - Tx MPDU completion information
 * @ppdu_id: PPDU id
 * @duration: user duration in ppdu
 * @frame_type: frame type MGMT/CTRL/DATA/BAR
 * @frame_ctrl: frame control field in 802.11 header
 * @qos_ctrl: QoS control field in 802.11 header
 * @tid: TID number
 * @num_msdu: number of msdu in MPDU
 * @seq_no: Sequence number of first MPDU
 * @ltf_size: ltf_size
 * @stbc: stbc
 * @he_re: he_re (range extension)
 * @txbf: txbf
 * @bw: Transmission bandwidth
 *       <enum 2 transmit_bw_20_MHz>
 *       <enum 3 transmit_bw_40_MHz>
 *       <enum 4 transmit_bw_80_MHz>
 *       <enum 5 transmit_bw_160_MHz>
 * @nss: NSS 1,2, ...8
 * @mcs: MCS index
 * @preamble: preamble
 * @gi: guard interval 800/400/1600/3200 ns
 * @resp_type: response type
 * @mprot_type: medium protection type
 * @rts_success: rts success
 * @rts failure: rts failure
 * @channel: frequency
 * @channel_num: channel number
 * @ack_rssi: ack rssi
 * @ldpc: ldpc
 * @tx_rate: Transmission Rate
 * @mac_address: peer mac address
 * @bss_mac_address: bss mac address
 * @ppdu_start_timestamp: TSF at PPDU start
 * @ppdu_end_timestamp: TSF at PPDU end
 * @ba_start_seq: Block Ack sequence number
 * @ba_bitmap: Block Ack bitmap
 * @ppdu_cookie: 16-bit ppdu_cookie
 * @long_retries: long retries
 * @short_retries: short retries
 * @completion_status: completion status - OK/Filter/Abort/Timeout
 */
struct cdp_tx_indication_mpdu_info {
	uint32_t ppdu_id;
	uint32_t tx_duration;
	uint16_t frame_type;
	uint16_t frame_ctrl;
	uint16_t qos_ctrl;
	uint8_t tid;
	uint32_t num_msdu;
	uint32_t seq_no;
	uint32_t ltf_size:2,
		 he_re:1,
		 txbf:4,
		 bw:4,
		 nss:4,
		 mcs:4,
		 preamble:4,
		 gi:4;
	uint32_t channel;
	uint8_t channel_num;
	uint32_t ack_rssi;
	uint32_t ldpc;
	uint32_t tx_rate;
	uint8_t mac_address[QDF_MAC_ADDR_SIZE];
	uint8_t bss_mac_address[QDF_MAC_ADDR_SIZE];
	uint64_t ppdu_start_timestamp;
	uint64_t ppdu_end_timestamp;
	uint32_t ba_start_seq;
	uint32_t ba_bitmap[CDP_BA_256_BIT_MAP_SIZE_DWORDS];
	uint16_t ppdu_cookie;
	uint16_t long_retries:4,
		 short_retries:4,
		 completion_status:8;
	uint16_t resp_type:4,
		 mprot_type:3,
		 rts_success:1,
		 rts_failure:1;
};

/**
 * struct cdp_tx_indication_info - Tx capture information
 * @mpdu_info: Tx MPDU completion information
 * @mpdu_nbuf: reconstructed mpdu packet
 * @ppdu_desc: tx completion ppdu
 */
struct cdp_tx_indication_info {
	struct cdp_tx_indication_mpdu_info mpdu_info;
	qdf_nbuf_t mpdu_nbuf;
	struct cdp_tx_completion_ppdu *ppdu_desc;
};

/**
 * struct cdp_tx_mgmt_comp_info - Tx mgmt comp info
 * @ppdu_id: ppdu_id
 * @is_sgen_pkt: payload recevied from wmi or htt path
 * @retries_count: retries count
 * @tx_tsf: 64 bit timestamp
 */
struct cdp_tx_mgmt_comp_info {
	uint32_t ppdu_id;
	bool is_sgen_pkt;
	uint16_t retries_count;
	uint64_t tx_tsf;
};

/**
 * struct cdp_tx_completion_ppdu - Tx PPDU completion information
 * @completion_status: completion status - OK/Filter/Abort/Timeout
 * @ppdu_id: PPDU Id
 * @ppdu_seq_id: ppdu sequence id for sojourn stats
 * @vdev_id: VAP Id
 * @bar_num_users: BA response user count, based on completion common TLV
 * @num_users: Number of users
 * @pending_retries: pending MPDUs (retries)
 * @drop_reason: drop reason from flush status
 * @is_flush: is_flush is set based on flush tlv
 * @flow_type: tx flow type from flush status
 * @queue_type: queue type from flush status
 * @num_mpdu: Number of MPDUs in PPDU
 * @num_msdu: Number of MSDUs in PPDU
 * @frame_type: frame SU or MU
 * @htt_frame_type: frame type from htt
 * @frame_ctrl: frame control of 80211 header
 * @channel: Channel informartion
 * @resp_type: response type
 * @mprot_type: medium protection type
 * @rts_success: rts success
 * @rts failure: rts failure
 * @phymode: phy mode
 * @ack_rssi: RSSI value of last ack packet (units=dB above noise floor)
 * @tx_duration: PPDU airtime
 * @ppdu_start_timestamp: TSF at PPDU start
 * @ppdu_end_timestamp: TSF at PPDU end
 * @ack_timestamp: TSF at the reception of ACK
 * @delayed_ba: Delayed ba flag
 * @beam_change: beam change bit in ppdu for he-information
 * @bss_color: 6 bit value for full bss color
 * @user: per-User stats (array of per-user structures)
 * @mpdu_q: queue of mpdu in a ppdu
 * @mpdus: MPDU list based on enqueue sequence bitmap
 * @bar_ppdu_id: BAR ppdu_id
 * @bar_tx_duration: BAR tx duration
 * @bar_ppdu_start_timestamp: BAR start timestamp
 * @bar_ppdu_end_timestamp: BAR end timestamp
 */
struct cdp_tx_completion_ppdu {
	uint32_t ppdu_id;
	uint32_t ppdu_seq_id;
	uint16_t vdev_id;
	uint16_t bar_num_users;
	uint32_t num_users;
	uint8_t last_usr_index;
	uint32_t pending_retries;
	uint32_t drop_reason;
	uint32_t is_flush:1,
		 flow_type:8,
		 queue_type:8;
	uint32_t num_mpdu:9,
		 num_msdu:16;
	uint16_t frame_type;
	uint16_t htt_frame_type;
	uint16_t frame_ctrl;
	uint16_t channel;
	uint16_t resp_type:4,
		 mprot_type:3,
		 rts_success:1,
		 rts_failure:1;
	uint16_t phy_mode;
	uint32_t ack_rssi;
	uint32_t tx_duration;
	uint64_t ppdu_start_timestamp;
	uint64_t ppdu_end_timestamp;
	uint64_t ack_timestamp;
	bool delayed_ba;
	uint8_t beam_change;
	uint8_t bss_color;
	struct cdp_tx_completion_ppdu_user user[CDP_MU_MAX_USERS];
	qdf_nbuf_queue_t mpdu_q;
	qdf_nbuf_t *mpdus;
	uint32_t bar_ppdu_id;
	uint32_t bar_tx_duration;
	uint32_t bar_ppdu_start_timestamp;
	uint32_t bar_ppdu_end_timestamp;
};

/**
 * struct cdp_dev_stats - Network device stats structure
 * @tx_packets: Tx total packets transmitted
 * @tx_bytes  : Tx total bytes transmitted
 * @tx_errors : Tx error due to FW tx failure, Ring failure DMA etc
 * @tx_dropped: Tx dropped is same as tx errors as above
 * @rx_packets: Rx total packets transmitted
 * @rx_bytes  : Rx total bytes transmitted
 * @rx_errors : Rx erros
 * @rx_dropped: Rx dropped stats
 */
struct cdp_dev_stats {
	uint32_t tx_packets;
	uint32_t tx_bytes;
	uint32_t tx_errors;
	uint32_t tx_dropped;
	uint32_t rx_packets;
	uint32_t rx_bytes;
	uint32_t rx_errors;
	uint32_t rx_dropped;
};

/**
 * struct cdp_rate_stats - Tx/Rx Rate statistics
 * @bw: Indicates the BW of the upcoming transmission -
 *       <enum 2 transmit_bw_20_MHz>
 *       <enum 3 transmit_bw_40_MHz>
 *       <enum 4 transmit_bw_80_MHz>
 *       <enum 5 transmit_bw_160_MHz>
 * @pkt_type: Transmit Packet Type
 * @stbc: When set, STBC transmission rate was used
 * @ldpc: When set, use LDPC transmission rates
 * @sgi: <enum 0     0_8_us_sgi > Legacy normal GI
 *       <enum 1     0_4_us_sgi > Legacy short GI
 *       <enum 2     1_6_us_sgi > HE related GI
 *       <enum 3     3_2_us_sgi > HE
 * @mcs: Transmit MCS Rate
 * @ofdma: Set when the transmission was an OFDMA transmission
 * @tones_in_ru: The number of tones in the RU used.
 * @tsf: Lower 32 bits of the TSF (timestamp when ppdu transmission finished)
 * @peer_id: Peer ID of the flow or MPDU queue
 * @tid: TID of the flow or MPDU queue
 */
struct cdp_rate_stats {
	uint32_t rate_stats_info_valid:1,
		 bw:2,
		 pkt_type:4,
		 stbc:1,
		 ldpc:1,
		 sgi:2,
		 mcs:4,
		 ofdma:1,
		 tones_in_ru:12,
		 resvd0:4;
	uint32_t tsf;
	uint16_t peer_id;
	uint8_t tid;
};

/**
 * struct cdp_tx_completion_msdu - Tx MSDU completion descriptor
 * @ppdu_id: PPDU to which this MSDU belongs
 * @transmit_cnt: Number of times this frame has been transmitted
 * @ack_frame_rssi: RSSI of the received ACK or BA frame
 * @first_msdu: Indicates this MSDU is the first MSDU in AMSDU
 * @last_msdu: Indicates this MSDU is the last MSDU in AMSDU
 * @msdu_part_of_amsdu : Indicates this MSDU was part of an A-MSDU in MPDU
 * @extd: Extended structure containing rate statistics
 */
struct cdp_tx_completion_msdu {
	uint32_t ppdu_id;
	uint8_t transmit_cnt;
	uint32_t ack_frame_rssi:8,
		 resvd0:1,
		 first_msdu:1,
		 last_msdu:1,
		 msdu_part_of_amsdu:1,
		 resvd1:20;
	struct cdp_rate_stats extd;
};

/**
 * struct cdp_rx_stats_ppdu_user -- per user RX stats
 * @peer_id: Peer ID
 * @vdev_id: VAP ID
 * @is_ampdu: mpdu aggregate or non-aggregate?
 * @mu_ul_info_valid: MU UL info valid
 * @ofdma_ru_start_index: RU index number(0-73)
 * @ofdma_ru_width: size of RU in units of 1(26tone)RU
 * @nss: NSS 1,2, ...8
 * @mcs: MCS index
 * @user_index: user ID in multi-user case
 * @ast_index: ast index in multi-user case
 * @tid: TID number
 * @num_msdu: Number of MSDUs in PPDU
 * @udp_msdu_count: Number of UDP MSDUs in PPDU
 * @tcp_msdu_count: Number of TCP MSDUs in PPDU
 * @other_msdu_count: Number of MSDUs other than UDP and TCP MSDUs in PPDU
 * @frame_control: frame control field
 * @frame_control_info_valid: frame_control valid
 * @data_sequence_control_info_valid: data_sequence_control_info valid
 * @first_data_seq_ctrl: Sequence control field of first data frame
 * @preamble: preamble
 * @ht_flag: ht flag
 * @vht_flag: vht flag
 * @he_re: he_re (range extension)
 * @mpdu_cnt_fcs_ok: Number of MPDUs in PPDU with fcs ok
 * @mpdu_cnt_fcs_err: Number of MPDUs in PPDU with fcs err
 * @mpdu_fcs_ok_bitmap - MPDU with fcs ok bitmap
 * @retried - number of retries
 * @mac_addr: Peer MAC Address
 */
struct cdp_rx_stats_ppdu_user {
	uint16_t peer_id;
	uint8_t vdev_id;
	bool is_ampdu;
	uint32_t mu_ul_info_valid:1,
		 ofdma_ru_start_index:7,
		 ofdma_ru_width:7,
		 nss:4,
		 mcs:4;
	/* user id */
	uint8_t  user_index;
	uint32_t ast_index;
	uint32_t tid;
	uint32_t num_msdu;
	uint16_t  tcp_msdu_count;
	uint16_t  udp_msdu_count;
	uint16_t  other_msdu_count;
	uint16_t frame_control;
	uint8_t  frame_control_info_valid;
	uint8_t data_sequence_control_info_valid;
	uint16_t first_data_seq_ctrl;
	uint32_t preamble_type;
	uint16_t ht_flags;
	uint16_t vht_flags;
	uint16_t he_flags;
	uint32_t mpdu_cnt_fcs_ok;
	uint32_t mpdu_cnt_fcs_err;
	uint32_t mpdu_fcs_ok_bitmap[QDF_MON_STATUS_MPDU_FCS_BMAP_NWORDS];
	uint32_t mpdu_ok_byte_count;
	uint32_t mpdu_err_byte_count;
	uint32_t retries;
	uint8_t  mac_addr[QDF_MAC_ADDR_SIZE];
};

/**
 * struct cdp_rx_indication_ppdu - Rx PPDU indication structure
 * @ppdu_id: PPDU Id
 * @is_ampdu: mpdu aggregate or non-aggregate?
 * @num_mpdu: Number of MPDUs in PPDU
 * @reserved: Reserved bits for future use
 * @num_msdu: Number of MSDUs in PPDU
 * @udp_msdu_count: Number of UDP MSDUs in PPDU
 * @tcp_msdu_count: Number of TCP MSDUs in PPDU
 * @other_msdu_count: Number of MSDUs other than UDP and TCP MSDUs in PPDU
 * @duration: PPDU duration
 * @tid: TID number
 * @peer_id: Peer ID
 * @vdev_id: VAP ID
 * @mac_addr: Peer MAC Address
 * @first_data_seq_ctrl: Sequence control field of first data frame
 * @ltf_size: ltf_size
 * @stbc: When set, STBC rate was used
 * @he_re: he_re (range extension)
 * @bw: Bandwidth
 *       <enum 0 bw_20_MHz>
 *       <enum 1 bw_40_MHz>
 *       <enum 2 bw_80_MHz>
 *       <enum 3 bw_160_MHz>
 * @nss: NSS 1,2, ...8
 * @mcs: MCS index
 * @preamble: preamble
 * @gi: <enum 0     0_8_us_sgi > Legacy normal GI
 *       <enum 1     0_4_us_sgi > Legacy short GI
 *       <enum 2     1_6_us_sgi > HE related GI
 *       <enum 3     3_2_us_sgi > HE
 * @dcm: dcm
 * @ldpc: ldpc
 * @ppdu_type: SU/MU_MIMO/MU_OFDMA/MU_MIMO_OFDMA/UL_TRIG/BURST_BCN/UL_BSR_RESP/
 * UL_BSR_TRIG/UNKNOWN
 * @rssi: RSSI value (units = dB above noise floor)
 * @timestamp: TSF at the reception of PPDU
 * @length: PPDU length
 * @channel: Channel informartion
 * @lsig_A: L-SIG in 802.11 PHY header
 * @frame_ctrl: frame control field
 * @rix: rate index
 * @rssi_chain: rssi chain per nss per bw
 * @cookie: cookie to used by upper layer
 * @user: per user stats in MU-user case
 * @nf: noise floor
 * @per_chain_rssi: rssi per antenna
 */
struct cdp_rx_indication_ppdu {
	uint32_t ppdu_id;
	uint16_t is_ampdu:1,
		 num_mpdu:9,
		 reserved:6;
	uint32_t num_msdu;
	uint32_t num_bytes;
	uint16_t udp_msdu_count;
	uint16_t tcp_msdu_count;
	uint16_t other_msdu_count;
	uint16_t duration;
	uint32_t tid:8,
		 peer_id:16;
	uint8_t vdev_id;
	uint8_t mac_addr[6];
	uint16_t first_data_seq_ctrl;
	union {
		uint32_t rate_info;
		struct {
			uint32_t ltf_size:2,
				 stbc:1,
				 he_re:1,
				 bw:4,
				 nss:4,
				 mcs:4,
				 preamble:4,
				 gi:4,
				 dcm:1,
				 ldpc:1,
				 ppdu_type:5;
		};
	} u;
	uint32_t rix;
	uint32_t lsig_a;
	uint32_t rssi;
	uint64_t timestamp;
	uint32_t length;
	uint8_t channel;
	uint8_t beamformed;

	uint32_t rx_ratekbps;
	uint32_t ppdu_rx_rate;

	uint32_t retries;
	uint32_t rx_byte_count;
	uint16_t rx_ratecode;
	uint8_t fcs_error_mpdus;
	uint16_t frame_ctrl;
	int8_t rssi_chain[SS_COUNT][MAX_BW];
	struct cdp_stats_cookie *cookie;
	struct cdp_rx_su_evm_info evm_info;
	uint32_t rx_antenna;
	uint8_t num_users;
	struct cdp_rx_stats_ppdu_user user[CDP_MU_MAX_USERS];
	uint32_t nf;
	uint8_t  per_chain_rssi[MAX_CHAIN];
	uint8_t is_mcast_bcast;
#if defined(WLAN_CFR_ENABLE) && defined(WLAN_ENH_CFR_ENABLE)
	struct cdp_rx_ppdu_cfr_info cfr_info;
#endif
};

/**
 * struct cdp_rx_indication_msdu - Rx MSDU info
 * @ppdu_id: PPDU to which the MSDU belongs
 * @msdu_len: Length of MSDU in bytes
 * @ack_frame_rssi: RSSI of the received ACK or BA frame
 * @first_msdu: Indicates this MSDU is the first MSDU in AMSDU
 * @last_msdu: Indicates this MSDU is the last MSDU in AMSDU
 * @msdu_part_of_amsdu : Indicates this MSDU was part of an A-MSDU in MPDU
 * @extd: Extended structure containing rate statistics
 */
struct cdp_rx_indication_msdu {
	uint32_t ppdu_id;
	uint16_t msdu_len;
	uint32_t ack_frame_rssi:8,
		 resvd0:1,
		 first_msdu:1,
		 last_msdu:1,
		 msdu_part_of_amsdu:1,
		 msdu_part_of_ampdu:1,
		 resvd1:19;
	struct cdp_rate_stats extd;
};

/**
 * struct cdp_config_params - Propagate configuration parameters to datapath
 * @tso_enable: Enable/Disable TSO
 * @lro_enable: Enable/Disable LRO
 * @gro_enable: Enable/Disable GRO
 * @flow_steering_enable: Enable/Disable Rx Hash based flow steering
 * @tcp_Udp_ChecksumOffload: Enable/Disable tcp-Udp checksum Offload
 * @napi_enable: Enable/Disable Napi
 * @ipa_enable: Flag indicating if IPA is enabled or not
 * @tx_flow_stop_queue_threshold: Value to Pause tx queues
 * @tx_flow_start_queue_offset: Available Tx descriptors to unpause
 *				tx queue
 * @tx_comp_loop_pkt_limit: Max # of packets to be processed in 1 tx comp loop
 * @rx_reap_loop_pkt_limit: Max # of packets to be processed in 1 rx reap loop
 * @rx_hp_oos_update_limit: Max # of HP OOS (out of sync) updates
 */
struct cdp_config_params {
	unsigned int tso_enable:1;
	unsigned int lro_enable:1;
	unsigned int gro_enable:1;
	unsigned int flow_steering_enable:1;
	unsigned int tcp_udp_checksumoffload:1;
	unsigned int napi_enable:1;
	unsigned int ipa_enable:1;
	/* Set when QCA_LL_TX_FLOW_CONTROL_V2 is enabled */
	uint8_t tx_flow_stop_queue_threshold;
	uint8_t tx_flow_start_queue_offset;
	uint32_t tx_comp_loop_pkt_limit;
	uint32_t rx_reap_loop_pkt_limit;
	uint32_t rx_hp_oos_update_limit;

};

/**
 * cdp_txrx_stats_req: stats request wrapper
 *	used to pass request information to cdp layer
 * @stats: type of stats requested
 * @param0: opaque argument 0 to be passed to htt
 * @param1: opaque argument 1 to be passed to htt
 * @param2: opaque argument 2 to be passed to htt
 * @param3: opaque argument 3 to be passed to htt
 * @mac id: mac_id
 */
struct cdp_txrx_stats_req {
	enum cdp_stats	stats;
	uint32_t	param0;
	uint32_t	param1;
	uint32_t	param2;
	uint32_t	param3;
	uint32_t	cookie_val;
	uint8_t		mac_id;
	char *peer_addr;
};

/**
 * struct cdp_monitor_filter - monitor filter info
 * @mode: set filter mode
 * @fp_mgmt: set Filter Pass MGMT Configuration
 * @fp_ctrl: set Filter Pass CTRL Configuration
 * @fp_data: set Filter Pass DATA Configuration
 * @mo_mgmt: set Monitor Other MGMT Configuration
 * @mo_ctrl: set Monitor Other CTRL Configuration
 * @mo_data: set Monitor other DATA Configuration
 *
 */
struct cdp_monitor_filter {
	uint16_t mode;
	uint16_t fp_mgmt;
	uint16_t fp_ctrl;
	uint16_t fp_data;
	uint16_t mo_mgmt;
	uint16_t mo_ctrl;
	uint16_t mo_data;
};

/**
 * cdp_dp_cfg - dp ini config enum
 */
enum cdp_dp_cfg {
	cfg_dp_enable_data_stall,
	cfg_dp_enable_ip_tcp_udp_checksum_offload,
	cfg_dp_tso_enable,
	cfg_dp_lro_enable,
	cfg_dp_gro_enable,
	cfg_dp_tx_flow_start_queue_offset,
	cfg_dp_tx_flow_stop_queue_threshold,
	cfg_dp_ipa_uc_tx_buf_size,
	cfg_dp_ipa_uc_tx_partition_base,
	cfg_dp_ipa_uc_rx_ind_ring_count,
	cfg_dp_enable_flow_steering,
	cfg_dp_reorder_offload_supported,
	cfg_dp_ce_classify_enable,
	cfg_dp_disable_intra_bss_fwd,
};

/**
 * struct cdp_peer_cookie - cookie used when creating peer
 * @ctx: context passed to be used by consumer
 * @mac_addr: MAC address of peer
 * @peer_id: peer id
 * @pdev_id: pdev_id
 * @cookie: cookie to be used by consumer
 */
struct cdp_peer_cookie {
	struct cdp_stats_cookie *ctx;
	uint8_t mac_addr[QDF_MAC_ADDR_SIZE];
	uint8_t peer_id;
	uint8_t pdev_id;
	uint8_t cookie;
};

/**
 * cdp_flow_stats - Per-Flow (5-tuple) statistics
 * @msdu_count: number of rx msdus matching this flow
 *
 * HW also includes msdu_byte_count and timestamp, which
 * are not currently tracked in SW.
 */
struct cdp_flow_stats {
	uint32_t msdu_count;
};

/**
 * cdp_flow_fst_operation - RX FST operations allowed
 */
enum cdp_flow_fst_operation {
	CDP_FLOW_FST_ENTRY_ADD,
	CDP_FLOW_FST_ENTRY_DEL,
	CDP_FLOW_FST_RX_BYPASS_ENABLE,
	CDP_FLOW_FST_RX_BYPASS_DISABLE
};

/**
 * cdp_flow_protocol_type - RX FST supported protocol types, mapped to HW spec
 */
enum cdp_flow_protocol_type {
	CDP_FLOW_PROTOCOL_TYPE_TCP = 6,
	CDP_FLOW_PROTOCOL_TYPE_UDP = 17,
};

/**
 * cdp_rx_flow_tuple_info - RX flow tuple info used for addition/deletion
 * @dest_ip_127_96: destination IP address bit fields 96-127
 * @dest_ip_95_64: destination IP address bit fields 64-95
 * @dest_ip_63_32: destination IP address bit fields 32-63
 * @dest_ip_31_0: destination IP address bit fields 0-31
 * @src_ip_127_96: source IP address bit fields 96-127
 * @src_ip_95_64: source IP address bit fields 64-95
 * @src_ip_63_32: source IP address bit fields 32-63
 * @src_ip_31_0: source IP address bit fields 0-31
 * @dest_port: destination port of flow
 * @src_port: source port of flow
 * @l4_protocol: protocol type in flow (TCP/UDP)
 */
struct cdp_rx_flow_tuple_info {
	uint32_t dest_ip_127_96;
	uint32_t dest_ip_95_64;
	uint32_t dest_ip_63_32;
	uint32_t dest_ip_31_0;
	uint32_t src_ip_127_96;
	uint32_t src_ip_95_64;
	uint32_t src_ip_63_32;
	uint32_t src_ip_31_0;
	uint16_t dest_port;
	uint16_t src_port;
	uint16_t l4_protocol;
};

/**
 * cdp_rx_flow_info - RX flow info used for addition/deletion
 * @is_addr_ipv4: indicates whether given IP address is IPv4/IPv6
 * @op_code: add/delete/enable/disable operation requested
 * @flow_tupe_info: structure containing tuple info
 * @fse_metadata: metadata to be set in RX flow
 */
struct cdp_rx_flow_info {
	bool is_addr_ipv4;
	enum cdp_flow_fst_operation op_code;
	struct cdp_rx_flow_tuple_info flow_tuple_info;
	uint16_t fse_metadata;
};
#endif
