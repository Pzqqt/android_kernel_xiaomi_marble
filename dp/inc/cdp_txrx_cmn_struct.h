/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
#ifndef CONFIG_WIN
#include <cdp_txrx_mob_def.h>
#endif /* CONFIG_WIN */

#ifndef OL_TXRX_NUM_LOCAL_PEER_IDS
#define OL_TXRX_NUM_LOCAL_PEER_IDS 33   /* default */
#endif

#define OL_TXRX_INVALID_LOCAL_PEER_ID 0xffff

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
    HTT_DBG_CMN_NUM_STATS
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

    /* keep this last */
    htt_cmn_pkt_num_types
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

typedef struct cdp_soc_t *ol_txrx_soc_handle;

/**
 * ol_txrx_vdev_delete_cb - callback registered during vdev
 * detach
 */
typedef void (*ol_txrx_vdev_delete_cb)(void *context);

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
 * cdp_mgmt_tx_cb - tx management delivery notification
 * callback function
 */
typedef void
(*ol_txrx_mgmt_tx_cb)(void *ctxt, qdf_nbuf_t tx_mgmt_frm, int had_error);

/**
 * ol_rxrx_data_tx_cb - Function registered with the data path
 * that is called when tx frames marked as "no free" are
 * done being transmitted
 */
typedef void
(*ol_txrx_data_tx_cb)(void *ctxt, qdf_nbuf_t tx_frm, int had_error);

/**
 * ol_txrx_tx_fp - top-level transmit function
 * @data_vdev - handle to the virtual device object
 * @msdu_list - list of network buffers
 */
typedef qdf_nbuf_t (*ol_txrx_tx_fp)(void *data_vdev,
				    qdf_nbuf_t msdu_list);
/**
 * ol_txrx_tx_flow_control_fp - tx flow control notification
 * function from txrx to OS shim
 * @osif_dev - the virtual device's OS shim object
 * @tx_resume - tx os q should be resumed or not
 */
typedef void (*ol_txrx_tx_flow_control_fp)(void *osif_dev,
					    bool tx_resume);

/**
 * ol_txrx_rx_fp - receive function to hand batches of data
 * frames from txrx to OS shim
 * @data_vdev - handle to the OSIF virtual device object
 * @msdu_list - list of network buffers
 */
typedef QDF_STATUS(*ol_txrx_rx_fp)(void *osif_dev, qdf_nbuf_t msdu_list);

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

/**
 * ol_txrx_stats_callback - statistics notify callback
 */
typedef void (*ol_txrx_stats_callback)(void *ctxt,
				       enum htt_cmn_dbg_stats_type type,
				       uint8_t *buf, int bytes);

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
 * @rx.std - the OS shim rx function to deliver rx data
 * frames to. This can have different values for
 * different virtual devices, e.g. so one virtual
 * device's OS shim directly hands rx frames to the OS,
 * but another virtual device's OS shim filters out P2P
 * messages before sending the rx frames to the OS. The
 * netbufs delivered to the osif_rx function are in the
 * format specified by the OS to use for tx and rx
 * frames (either 802.3 or native WiFi)
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
 * @proxy_arp - proxy arp function pointer - specified by
 * OS shim, stored by txrx
 */
struct ol_txrx_ops {
	/* tx function pointers - specified by txrx, stored by OS shim */
	struct {
		ol_txrx_tx_fp         tx;
	} tx;

	/* rx function pointers - specified by OS shim, stored by txrx */
	struct {
		ol_txrx_rx_fp           rx;
		ol_txrx_rx_check_wai_fp wai_check;
		ol_txrx_rx_mon_fp       mon;
	} rx;

	/* proxy arp function pointer - specified by OS shim, stored by txrx */
	ol_txrx_proxy_arp_fp      proxy_arp;
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

#define PER_RADIO_FW_STATS_REQUEST 0
#define PER_VDEV_FW_STATS_REQUEST 1


#endif
