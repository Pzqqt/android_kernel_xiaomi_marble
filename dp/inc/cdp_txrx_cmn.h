/*
 * Copyright (c) 2011-2016 The Linux Foundation. All rights reserved.
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
 /**
 * @file cdp_txrx_api_common.h
 * @brief Define the host data path converged API functions
 * called by the host control SW and the OS interface module
 */
#ifndef _CDP_TXRX_CMN_H_
#define _CDP_TXRX_CMN_H_

#include "htc_api.h"
#include "qdf_types.h"
#include "qdf_nbuf.h"

/******************************************************************************
 *
 * Common Data Path Header File
 *
 *****************************************************************************/

/******************************************************************************
 *
 * Structure definitions
 *
 *****************************************************************************/

 /**
 * ol_txrx_pdev_handle - opaque handle for txrx physical device
 * object
 */
struct ol_txrx_pdev_t;
typedef struct ol_txrx_pdev_t *ol_txrx_pdev_handle;

/**
 * ol_txrx_vdev_handle - opaque handle for txrx virtual device
 * object
 */
struct ol_txrx_vdev_t;
typedef struct ol_txrx_vdev_t *ol_txrx_vdev_handle;

/**
 * ol_pdev_handle - opaque handle for the configuration
 * associated with the physical device
 */
struct ol_pdev_t;
typedef struct ol_pdev_t *ol_pdev_handle;

/**
 * ol_txrx_peer_handle - opaque handle for txrx peer object
 */
struct ol_txrx_peer_t;
typedef struct ol_txrx_peer_t *ol_txrx_peer_handle;

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
 * @wlan_op_mode_ndi: NAN datapath mode
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
 * ol_txrx_tx_fp - top-level transmit function
 * @data_vdev - handle to the virtual device object
 * @msdu_list - list of network buffers
 */
typedef qdf_nbuf_t (*ol_txrx_tx_fp)(ol_txrx_vdev_handle data_vdev,
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
typedef QDF_STATUS (*ol_txrx_rx_fp)(void *osif_dev, qdf_nbuf_t msdu_list);

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
				       uint32_t type,
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

/******************************************************************************
 *
 * Control Interface (A Interface)
 *
 *****************************************************************************/

int
ol_txrx_pdev_attach_target(ol_txrx_pdev_handle pdev);

ol_txrx_vdev_handle
ol_txrx_vdev_attach(ol_txrx_pdev_handle pdev, uint8_t *vdev_mac_addr,
			 uint8_t vdev_id, enum wlan_op_mode op_mode);

void
ol_txrx_vdev_detach(ol_txrx_vdev_handle vdev,
			 ol_txrx_vdev_delete_cb callback, void *cb_context);

ol_txrx_pdev_handle
ol_txrx_pdev_attach(
	ol_pdev_handle ctrl_pdev,
	HTC_HANDLE htc_pdev,
	qdf_device_t osdev);

void
ol_txrx_pdev_detach(ol_txrx_pdev_handle pdev, int force);

ol_txrx_peer_handle
ol_txrx_peer_attach(ol_txrx_vdev_handle vdev, uint8_t *peer_mac_addr);

void
ol_txrx_peer_detach(ol_txrx_peer_handle peer);

int
ol_txrx_set_monitor_mode(ol_txrx_vdev_handle vdev);

void
ol_txrx_set_curchan(
	ol_txrx_pdev_handle pdev,
	uint32_t chan_mhz);

void
ol_txrx_set_privacy_filters(ol_txrx_vdev_handle vdev,
			 void *filter, uint32_t num);

/******************************************************************************
 * Data Interface (B Interface)
 *****************************************************************************/
void
ol_txrx_vdev_register(ol_txrx_vdev_handle vdev,
			 void *osif_vdev, struct ol_txrx_ops *txrx_ops);

int
ol_txrx_mgmt_send(
	ol_txrx_vdev_handle vdev,
	qdf_nbuf_t tx_mgmt_frm,
	uint8_t type);

int
ol_txrx_mgmt_send_ext(ol_txrx_vdev_handle vdev,
			 qdf_nbuf_t tx_mgmt_frm,
			 uint8_t type, uint8_t use_6mbps, uint16_t chanfreq);

/**
 * ol_txrx_mgmt_tx_cb - tx management delivery notification
 * callback function
 */
typedef void
(*ol_txrx_mgmt_tx_cb)(void *ctxt, qdf_nbuf_t tx_mgmt_frm, int had_error);

void
ol_txrx_mgmt_tx_cb_set(ol_txrx_pdev_handle pdev,
			 uint8_t type,
			 ol_txrx_mgmt_tx_cb download_cb,
			 ol_txrx_mgmt_tx_cb ota_ack_cb, void *ctxt);

int ol_txrx_get_tx_pending(ol_txrx_pdev_handle pdev);

/**
 * ol_txrx_data_tx_cb - Function registered with the data path
 * that is called when tx frames marked as "no free" are
 * done being transmitted
 */
typedef void
(*ol_txrx_data_tx_cb)(void *ctxt, qdf_nbuf_t tx_frm, int had_error);

void
ol_txrx_data_tx_cb_set(ol_txrx_vdev_handle data_vdev,
		 ol_txrx_data_tx_cb callback, void *ctxt);

/******************************************************************************
 * Statistics and Debugging Interface (C Inteface)
 *****************************************************************************/

int
ol_txrx_aggr_cfg(ol_txrx_vdev_handle vdev,
			 int max_subfrms_ampdu,
			 int max_subfrms_amsdu);

int
ol_txrx_fw_stats_get(
	 ol_txrx_vdev_handle vdev,
	 struct ol_txrx_stats_req *req,
	 bool per_vdev,
	 bool response_expected);

int
ol_txrx_debug(ol_txrx_vdev_handle vdev, int debug_specs);

void ol_txrx_fw_stats_cfg(
	 ol_txrx_vdev_handle vdev,
	 uint8_t cfg_stats_type,
	 uint32_t cfg_val);

void ol_txrx_print_level_set(unsigned level);

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
/**
 * ol_txrx_get_vdev_mac_addr() - Return mac addr of vdev
 * @vdev: vdev handle
 *
 * Return: vdev mac address
 */
uint8_t *
ol_txrx_get_vdev_mac_addr(ol_txrx_vdev_handle vdev);

/**
 * ol_txrx_get_vdev_struct_mac_addr() - Return handle to struct qdf_mac_addr of
 * vdev
 * @vdev: vdev handle
 *
 * Return: Handle to struct qdf_mac_addr
 */
struct qdf_mac_addr *
ol_txrx_get_vdev_struct_mac_addr(ol_txrx_vdev_handle vdev);

/**
 * ol_txrx_get_pdev_from_vdev() - Return handle to pdev of vdev
 * @vdev: vdev handle
 *
 * Return: Handle to pdev
 */
ol_txrx_pdev_handle ol_txrx_get_pdev_from_vdev(ol_txrx_vdev_handle vdev);

/**
 * ol_txrx_get_ctrl_pdev_from_vdev() - Return control pdev of vdev
 * @vdev: vdev handle
 *
 * Return: Handle to control pdev
 */
ol_pdev_handle
ol_txrx_get_ctrl_pdev_from_vdev(ol_txrx_vdev_handle vdev);

#endif /* _CDP_TXRX_CMN_H_ */


