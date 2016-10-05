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
 * @file ol_txrx_ctrl_api.h
 * @brief Define the host data API functions called by the host control SW.
 */
#ifndef _OL_TXRX_CTRL_API__H_
#define _OL_TXRX_CTRL_API__H_

#include <athdefs.h>            /* A_STATUS */
#include <qdf_nbuf.h>           /* qdf_nbuf_t */
#include <qdf_types.h>          /* qdf_device_t */
#include <htc_api.h>            /* HTC_HANDLE */

#include <ol_txrx_api.h>        /* ol_sec_type */
#include <wlan_defs.h>          /* MAX_SPATIAL_STREAM */
#include <cdp_txrx_cmn.h>       /* ol_pdev_handle, ol_vdev_handle, etc */
#include <cdp_txrx_cfg.h>

#define OL_ATH_TX_DRAIN_WAIT_DELAY 50

/* Maximum number of station supported by data path, including BC. */
#define WLAN_MAX_STA_COUNT  (HAL_NUM_STA)

/* The symbolic station ID return to HDD to specify the packet is bc/mc */
#define WLAN_RX_BCMC_STA_ID (WLAN_MAX_STA_COUNT + 1)

/* The symbolic station ID return to HDD to specify the packet is
       to soft-AP itself */
#define WLAN_RX_SAP_SELF_STA_ID (WLAN_MAX_STA_COUNT + 2)

#define OL_TXQ_PAUSE_REASON_FW                (1 << 0)
#define OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED (1 << 1)
#define OL_TXQ_PAUSE_REASON_TX_ABORT          (1 << 2)
#define OL_TXQ_PAUSE_REASON_VDEV_STOP         (1 << 3)
#define OL_TXQ_PAUSE_REASON_THERMAL_MITIGATION (1 << 4)

/* command options for dumpStats*/
#define WLAN_HDD_STATS        0
#define WLAN_TXRX_STATS       1
#define WLAN_TXRX_HIST_STATS  2
#define WLAN_TXRX_TSO_STATS   3
#define WLAN_HDD_NETIF_OPER_HISTORY 4
#define WLAN_DUMP_TX_FLOW_POOL_INFO 5
#define WLAN_TXRX_DESC_STATS  6
#define WLAN_HIF_STATS  7
#define WLAN_SCHEDULER_STATS        21
#define WLAN_TX_QUEUE_STATS         22
#define WLAN_BUNDLE_STATS           23
#define WLAN_CREDIT_STATS           24

/**
 * @brief Set up the data SW subsystem.
 * @details
 *  As part of the WLAN device attach, the data SW subsystem has
 *  to be attached as a component within the WLAN device.
 *  This attach allocates and initializes the physical device object
 *  used by the data SW.
 *  The data SW subsystem attach needs to happen after the target has
 *  be started, and host / target parameter negotiation has completed,
 *  since the host data SW uses some of these host/target negotiated
 *  parameters (e.g. peer ID range) during the initializations within
 *  its attach function.
 *  However, the host data SW is not allowed to send HTC messages to the
 *  target within this pdev_attach function call, since the HTC setup
 *  has not complete at this stage of initializations.  Any messaging
 *  to the target has to be done in the separate pdev_attach_target call
 *  that is invoked after HTC setup is complete.
 *
 * @param pdev - txrx_pdev handle
 * @return 0 for success or error code
 */
int
ol_txrx_pdev_post_attach(ol_txrx_pdev_handle pdev);

/**
 * @brief Parameter type to be input to ol_txrx_peer_update
 * @details
 *  This struct is union,to be used to specify various informations to update
 *   txrx peer object.
 */
union ol_txrx_peer_update_param_t {
	uint8_t qos_capable;
	uint8_t uapsd_mask;
	enum ol_sec_type sec_type;
};

/**
 * @brief Parameter type to be input to ol_txrx_peer_update
 * @details
 *   This enum is used to specify what exact information in
 *   ol_txrx_peer_update_param_t
 *   is used to update the txrx peer object.
 */
enum ol_txrx_peer_update_select_t {
	ol_txrx_peer_update_qos_capable = 1,
	ol_txrx_peer_update_uapsdMask,
	ol_txrx_peer_update_peer_security,
};

/**
 * @brief Update the data peer object as some informaiton changed in node.
 * @details
 *  Only a single prarameter can be changed for each call to this func.
 *
 * @param peer - pointer to the node's object
 * @param param - new param to be upated in peer object.
 * @param select - specify what's parameter needed to be update
 */
void
ol_txrx_peer_update(ol_txrx_vdev_handle data_vdev, uint8_t *peer_mac,
		    union ol_txrx_peer_update_param_t *param,
		    enum ol_txrx_peer_update_select_t select);

enum {
	OL_TX_WMM_AC_BE,
	OL_TX_WMM_AC_BK,
	OL_TX_WMM_AC_VI,
	OL_TX_WMM_AC_VO,

	OL_TX_NUM_WMM_AC
};

/**
 * @brief Parameter type to pass WMM setting to ol_txrx_set_wmm_param
 * @details
 *   The struct is used to specify informaiton to update TX WMM scheduler.
 */
struct ol_tx_ac_param_t {
	uint32_t aifs;
	uint32_t cwmin;
	uint32_t cwmax;
};

struct ol_tx_wmm_param_t {
	struct ol_tx_ac_param_t ac[OL_TX_NUM_WMM_AC];
};

#if defined(CONFIG_HL_SUPPORT)
/**
 * @brief Set paramters of WMM scheduler per AC settings.  .
 * @details
 *  This function applies only to HL systems.
 *
 * @param data_pdev - the physical device being paused
 * @param wmm_param - the wmm parameters
 */
void
ol_txrx_set_wmm_param(ol_txrx_pdev_handle data_pdev,
		      struct ol_tx_wmm_param_t wmm_param);

/**
 * @brief notify tx data SW that a peer-TID is ready to transmit to.
 * @details
 *  This function applies only to HL systems - in LL systems, tx flow control
 *  is handled entirely within the target FW.
 *  If a peer-TID has tx paused, then the tx datapath will end up queuing
 *  any tx frames that arrive from the OS shim for that peer-TID.
 *  In a HL system, the host tx data SW itself will classify the tx frame,
 *  and determine that it needs to be queued rather than downloaded to the
 *  target for transmission.
 *  Once the peer-TID is ready to accept data, the host control SW will call
 *  this function to notify the host data SW that the queued frames can be
 *  enabled for transmission, or specifically to download the tx frames
 *  to the target to transmit.
 *  The TID parameter is an extended version of the QoS TID.  Values 0-15
 *  indicate a regular QoS TID, and the value 16 indicates either non-QoS
 *  data, multicast data, or broadcast data.
 *
 * @param data_peer - which peer is being unpaused
 * @param tid - which TID within the peer is being unpaused, or -1 as a
 *      wildcard to unpause all TIDs within the peer
 */
void
ol_txrx_peer_tid_unpause(ol_txrx_peer_handle data_peer, int tid);


/**
 * @brief Tell a paused peer to release a specified number of tx frames.
 * @details
 *  This function applies only to HL systems - in LL systems, tx flow control
 *  is handled entirely within the target FW.
 *  Download up to a specified maximum number of tx frames from the tx
 *  queues of the specified TIDs within the specified paused peer, usually
 *  in response to a U-APSD trigger from the peer.
 *  It is up to the host data SW to determine how to choose frames from the
 *  tx queues of the specified TIDs.  However, the host data SW does need to
 *  provide long-term fairness across the U-APSD enabled TIDs.
 *  The host data SW will notify the target data FW when it is done downloading
 *  the batch of U-APSD triggered tx frames, so the target data FW can
 *  differentiate between an in-progress download versus a case when there are
 *  fewer tx frames available than the specified limit.
 *  This function is relevant primarily to HL U-APSD, where the frames are
 *  held in the host.
 *
 * @param peer - which peer sent the U-APSD trigger
 * @param tid_mask - bitmask of U-APSD enabled TIDs from whose tx queues
 *      tx frames can be released
 * @param max_frms - limit on the number of tx frames to release from the
 *      specified TID's queues within the specified peer
 */
void ol_txrx_tx_release(ol_txrx_peer_handle peer,
			u_int32_t tid_mask,
			int max_frms);

/**
 * @brief Suspend all tx data per thermal event/timer for the
 *  specified physical device
 * @details
 *  This function applies only to HL systerms, and it makes pause and
 * unpause operations happen in pairs.
 */
void
ol_txrx_throttle_pause(ol_txrx_pdev_handle data_pdev);


/**
 * @brief Resume all tx data per thermal event/timer for the
 * specified physical device
 * @details
 *  This function applies only to HL systerms, and it makes pause and
 * unpause operations happen in pairs.
 */
void
ol_txrx_throttle_unpause(ol_txrx_pdev_handle data_pdev);

#else

static inline
void ol_txrx_set_wmm_param(ol_txrx_pdev_handle data_pdev,
		      struct ol_tx_wmm_param_t wmm_param)
{
	return;
}

static inline void
ol_txrx_peer_tid_unpause(ol_txrx_peer_handle data_peer, int tid)
{
	return;
}

static inline void
ol_txrx_tx_release(ol_txrx_peer_handle peer,
		   u_int32_t tid_mask,
		   int max_frms)
{
	return;
}

static inline void
ol_txrx_throttle_pause(ol_txrx_pdev_handle data_pdev)
{
	return;
}

static inline void
ol_txrx_throttle_unpause(ol_txrx_pdev_handle data_pdev)
{
	return;
}

#endif /* CONFIG_HL_SUPPORT */

/**
 * @brief notify tx data SW that a peer's transmissions are suspended.
 * @details
 *  This function applies only to HL systems - in LL systems, tx flow control
 *  is handled entirely within the target FW.
 *  The HL host tx data SW is doing tx classification and tx download
 *  scheduling, and therefore also needs to actively participate in tx
 *  flow control.  Specifically, the HL tx data SW needs to check whether a
 *  given peer is available to transmit to, or is paused.
 *  This function is used to tell the HL tx data SW when a peer is paused,
 *  so the host tx data SW can hold the tx frames for that SW.
 *
 * @param data_peer - which peer is being paused
 */
static inline void ol_txrx_peer_pause(struct ol_txrx_peer_t *data_peer)
{
	return;
}

/**
 * @brief Suspend all tx data for the specified physical device.
 * @details
 *  This function applies only to HL systems - in LL systems, tx flow control
 *  is handled entirely within the target FW.
 *  In some systems it is necessary to be able to temporarily
 *  suspend all WLAN traffic, e.g. to allow another device such as bluetooth
 *  to temporarily have exclusive access to shared RF chain resources.
 *  This function suspends tx traffic within the specified physical device.
 *
 * @param data_pdev - the physical device being paused
 */
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || \
		defined(QCA_LL_TX_FLOW_CONTROL_V2) || defined(CONFIG_HL_SUPPORT)

void ol_txrx_pdev_pause(struct ol_txrx_pdev_t *data_pdev, uint32_t reason);
#else
static inline
void ol_txrx_pdev_pause(struct ol_txrx_pdev_t *data_pdev, uint32_t reason)
{
	return;
}
#endif

/**
 * @brief Resume tx for the specified physical device.
 * @details
 *  This function applies only to HL systems - in LL systems, tx flow control
 *  is handled entirely within the target FW.
 *
 * @param data_pdev - the physical device being unpaused
 */
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || \
		defined(QCA_LL_TX_FLOW_CONTROL_V2) || defined(CONFIG_HL_SUPPORT)

void ol_txrx_pdev_unpause(struct ol_txrx_pdev_t *pdev, uint32_t reason);
#else
static inline
void ol_txrx_pdev_unpause(struct ol_txrx_pdev_t *pdev, uint32_t reason)
{
	return;
}
#endif

/**
 * @brief Synchronize the data-path tx with a control-path target download
 * @dtails
 * @param data_pdev - the data-path physical device object
 * @param sync_cnt - after the host data-path SW downloads this sync request
 *      to the target data-path FW, the target tx data-path will hold itself
 *      in suspension until it is given an out-of-band sync counter value that
 *      is equal to or greater than this counter value
 */
void ol_txrx_tx_sync(ol_txrx_pdev_handle data_pdev, uint8_t sync_cnt);

typedef void (*ol_txrx_vdev_delete_cb)(void *context);


typedef void
(*ol_txrx_data_tx_cb)(void *ctxt, qdf_nbuf_t tx_frm, int had_error);

/**
 * @brief Store a delivery notification callback for specific data frames.
 * @details
 *  Through a non-std tx function, the txrx SW can be given tx data frames
 *  that are specially marked to not be unmapped and freed by the tx SW
 *  when transmission completes.  Rather, these specially-marked frames
 *  are provided to the callback registered with this function.
 *
 * @param data_vdev - which vdev the callback is being registered with
 *      (Currently the callback is stored in the pdev rather than the vdev.)
 * @param callback - the function to call when tx frames marked as "no free"
 *      are done being transmitted
 * @param ctxt - the context argument provided to the callback function
 */
void
ol_txrx_data_tx_cb_set(ol_txrx_vdev_handle data_vdev,
		       ol_txrx_data_tx_cb callback, void *ctxt);

#ifdef FEATURE_RUNTIME_PM
QDF_STATUS ol_txrx_runtime_suspend(ol_txrx_pdev_handle txrx_pdev);
QDF_STATUS ol_txrx_runtime_resume(ol_txrx_pdev_handle txrx_pdev);
#endif

QDF_STATUS ol_txrx_wait_for_pending_tx(int timeout);

/**
 * @brief Discard all tx frames that are pending in txrx.
 * @details
 *  Mainly used in clean up path to make sure all pending tx packets
 *  held by txrx are returned back to OS shim immediately.
 *
 * @param pdev - the data physical device object
 * @return - void
 */
void ol_txrx_discard_tx_pending(ol_txrx_pdev_handle pdev);

/**
 * @brief set the safemode of the device
 * @details
 *  This flag is used to bypass the encrypt and decrypt processes when send and
 *  receive packets. It works like open AUTH mode, HW will treate all packets
 *  as non-encrypt frames because no key installed. For rx fragmented frames,
 *  it bypasses all the rx defragmentaion.
 *
 * @param vdev - the data virtual device object
 * @param val - the safemode state
 * @return - void
 */
void ol_txrx_set_safemode(ol_txrx_vdev_handle vdev, uint32_t val);

/**
 * @brief configure the drop unencrypted frame flag
 * @details
 *  Rx related. When set this flag, all the unencrypted frames
 *  received over a secure connection will be discarded
 *
 * @param vdev - the data virtual device object
 * @param val - flag
 * @return - void
 */
void ol_txrx_set_drop_unenc(ol_txrx_vdev_handle vdev, uint32_t val);

void
ol_txrx_peer_keyinstalled_state_update(ol_txrx_peer_handle data_peer,
				       uint8_t val);

#define ol_tx_addba_conf(data_peer, tid, status)        /* no-op */

/**
 * @brief Find a txrx peer handle from the peer's MAC address
 * @details
 *  The control SW typically uses the txrx peer handle to refer to the peer.
 *  In unusual circumstances, if it is infeasible for the control SW maintain
 *  the txrx peer handle but it can maintain the peer's MAC address,
 *  this function allows the peer handled to be retrieved, based on the peer's
 *  MAC address.
 *  In cases where there are multiple peer objects with the same MAC address,
 *  it is undefined which such object is returned.
 *  This function does not increment the peer's reference count.  Thus, it is
 *  only suitable for use as long as the control SW has assurance that it has
 *  not deleted the peer object, by calling ol_txrx_peer_detach.
 *
 * @param pdev - the data physical device object
 * @param peer_mac_addr - MAC address of the peer in question
 * @return handle to the txrx peer object
 */
ol_txrx_peer_handle
ol_txrx_peer_find_by_addr(ol_txrx_pdev_handle pdev, uint8_t *peer_mac_addr);

struct ol_txrx_peer_stats_t {
	struct {
		struct {
			uint32_t ucast;
			uint32_t mcast;
			uint32_t bcast;
		} frms;
		struct {
			uint32_t ucast;
			uint32_t mcast;
			uint32_t bcast;
		} bytes;
	} tx;
	struct {
		struct {
			uint32_t ucast;
			uint32_t mcast;
			uint32_t bcast;
		} frms;
		struct {
			uint32_t ucast;
			uint32_t mcast;
			uint32_t bcast;
		} bytes;
	} rx;
};

/**
 * @brief Provide a snapshot of the txrx counters for the specified peer
 * @details
 *  The txrx layer optionally maintains per-peer stats counters.
 *  This function provides the caller with a consistent snapshot of the
 *  txrx stats counters for the specified peer.
 *
 * @param pdev - the data physical device object
 * @param peer - which peer's stats counters are requested
 * @param stats - buffer for holding the stats counters snapshot
 * @return success / failure status
 */
#ifdef QCA_ENABLE_OL_TXRX_PEER_STATS
A_STATUS
ol_txrx_peer_stats_copy(ol_txrx_pdev_handle pdev,
			ol_txrx_peer_handle peer, ol_txrx_peer_stats_t *stats);
#else
#define ol_txrx_peer_stats_copy(pdev, peer, stats) A_ERROR      /* failure */
#endif /* QCA_ENABLE_OL_TXRX_PEER_STATS */

QDF_STATUS ol_txrx_get_vdevid(struct ol_txrx_peer_t *peer, uint8_t *vdev_id);

void *ol_txrx_get_vdev_by_sta_id(uint8_t sta_id);

#define OL_TXRX_INVALID_LOCAL_PEER_ID 0xffff

#define OL_TXRX_RSSI_INVALID 0xffff
/**
 * @brief Provide the current RSSI average from data frames sent by a peer.
 * @details
 *  If a peer has sent data frames, the data SW will optionally keep
 *  a running average of the RSSI observed for those data frames.
 *  This function returns that time-average RSSI if is it available,
 *  or OL_TXRX_RSSI_INVALID if either RSSI tracking is disabled or if
 *  no data frame indications with valid RSSI meta-data have been received.
 *  The RSSI is in approximate dBm units, and is normalized with respect
 *  to a 20 MHz channel.  For example, if a data frame is received on a
 *  40 MHz channel, wherein both the primary 20 MHz channel and the
 *  secondary 20 MHz channel have an RSSI of -77 dBm, the reported RSSI
 *  will be -77 dBm, rather than the actual -74 dBm RSSI from the
 *  combination of the primary + extension 20 MHz channels.
 *  Alternatively, the RSSI may be evaluated only on the primary 20 MHz
 *  channel.
 *
 * @param peer - which peer's RSSI is desired
 * @return RSSI evaluted from frames sent by the specified peer
 */
#ifdef QCA_SUPPORT_PEER_DATA_RX_RSSI
int16_t ol_txrx_peer_rssi(ol_txrx_peer_handle peer);
#else
#define ol_txrx_peer_rssi(peer) OL_TXRX_RSSI_INVALID
#endif /* QCA_SUPPORT_PEER_DATA_RX_RSSI */

/*
 * Bins used for reporting delay histogram:
 * bin 0:  0 - 10  ms delay
 * bin 1: 10 - 20  ms delay
 * bin 2: 20 - 40  ms delay
 * bin 3: 40 - 80  ms delay
 * bin 4: 80 - 160 ms delay
 * bin 5: > 160 ms delay
 */
#define QCA_TX_DELAY_HIST_REPORT_BINS 6

#if defined(CONFIG_HL_SUPPORT) && defined(QCA_BAD_PEER_TX_FLOW_CL)

/**
 * @brief Configure the bad peer tx limit setting.
 * @details
 *
 * @param pdev - the physics device
 */
void
ol_txrx_bad_peer_txctl_set_setting(
	struct ol_txrx_pdev_t *pdev,
	int enable,
	int period,
	int txq_limit);

/**
 * @brief Configure the bad peer tx threshold limit
 * @details
 *
 * @param pdev - the physics device
 */
void
ol_txrx_bad_peer_txctl_update_threshold(
	struct ol_txrx_pdev_t *pdev,
	int level,
	int tput_thresh,
	int tx_limit);

#else

static inline void
ol_txrx_bad_peer_txctl_set_setting(
	struct ol_txrx_pdev_t *pdev,
	int enable,
	int period,
	int txq_limit)
{
	return;
}

static inline void
ol_txrx_bad_peer_txctl_update_threshold(
	struct ol_txrx_pdev_t *pdev,
	int level,
	int tput_thresh,
	int tx_limit)
{
	return;
}
#endif /* defined(CONFIG_HL_SUPPORT) && defined(QCA_BAD_PEER_TX_FLOW_CL) */


void ol_txrx_set_ocb_peer(struct ol_txrx_pdev_t *pdev,
			  struct ol_txrx_peer_t *peer);

bool ol_txrx_get_ocb_peer(struct ol_txrx_pdev_t *pdev,
			  struct ol_txrx_peer_t **peer);

void ol_tx_set_is_mgmt_over_wmi_enabled(uint8_t value);
uint8_t ol_tx_get_is_mgmt_over_wmi_enabled(void);

/* TX FLOW Control related functions */
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
#define TX_FLOW_MGMT_POOL_ID	0xEF

#ifdef QCA_LL_TX_FLOW_GLOBAL_MGMT_POOL
#define TX_FLOW_MGMT_POOL_SIZE  32
#else
#define TX_FLOW_MGMT_POOL_SIZE  0
#endif

void ol_tx_register_flow_control(struct ol_txrx_pdev_t *pdev);
void ol_tx_deregister_flow_control(struct ol_txrx_pdev_t *pdev);
void ol_tx_dump_flow_pool_info(void);
void ol_tx_clear_flow_pool_stats(void);
void ol_tx_flow_pool_map_handler(uint8_t flow_id, uint8_t flow_type,
				 uint8_t flow_pool_id, uint16_t flow_pool_size);
void ol_tx_flow_pool_unmap_handler(uint8_t flow_id, uint8_t flow_type,
				   uint8_t flow_pool_id);
struct ol_tx_flow_pool_t *ol_tx_create_flow_pool(uint8_t flow_pool_id,
						 uint16_t flow_pool_size);
int ol_tx_delete_flow_pool(struct ol_tx_flow_pool_t *pool, bool force);
#else

static inline void ol_tx_register_flow_control(struct ol_txrx_pdev_t *pdev)
{
	return;
}
static inline void ol_tx_deregister_flow_control(struct ol_txrx_pdev_t *pdev)
{
	return;
}
static inline void ol_tx_dump_flow_pool_info(void)
{
	return;
}
static inline void ol_tx_clear_flow_pool_stats(void)
{
	return;
}
static inline void ol_tx_flow_pool_map_handler(uint8_t flow_id,
	uint8_t flow_type, uint8_t flow_pool_id, uint16_t flow_pool_size)
{
	return;
}
static inline void ol_tx_flow_pool_unmap_handler(uint8_t flow_id,
	 uint8_t flow_type, uint8_t flow_pool_id)
{
	return;
}
static inline struct ol_tx_flow_pool_t *ol_tx_create_flow_pool(
		uint8_t flow_pool_id, uint16_t flow_pool_size)
{
	return NULL;
}
static inline int ol_tx_delete_flow_pool(struct ol_tx_flow_pool_t *pool,
		bool force)
{
	return 0;
}
#endif

void ol_tx_mark_first_wakeup_packet(uint8_t value);

#endif /* _OL_TXRX_CTRL_API__H_ */
