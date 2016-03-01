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

#include <ol_osif_api.h>        /* ol_osif_vdev_handle */
#include <ol_txrx_api.h>        /* ol_txrx_pdev_handle, etc. */
#include <ol_ctrl_api.h>        /* ol_pdev_handle, ol_vdev_handle */

#include <wlan_defs.h>          /* MAX_SPATIAL_STREAM */

#define OL_ATH_TX_DRAIN_WAIT_DELAY 50

/* Maximum number of station supported by data path, including BC. */
#define WLAN_MAX_STA_COUNT  (HAL_NUM_STA)

/* The symbolic station ID return to HDD to specify the packet is bc/mc */
#define WLAN_RX_BCMC_STA_ID (WLAN_MAX_STA_COUNT + 1)

/* The symbolic station ID return to HDD to specify the packet is
       to soft-AP itself */
#define WLAN_RX_SAP_SELF_STA_ID (WLAN_MAX_STA_COUNT + 2)

/**
 * enum wlan_op_mode - Virtual device operation mode
 *
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
};

#define OL_TXQ_PAUSE_REASON_FW                (1 << 0)
#define OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED (1 << 1)
#define OL_TXQ_PAUSE_REASON_TX_ABORT          (1 << 2)
#define OL_TXQ_PAUSE_REASON_VDEV_STOP         (1 << 3)
#define OL_TXQ_PAUSE_REASON_THERMAL_MITIGATION (1 << 4)


/**
 * enum netif_action_type - Type of actions on netif queues
 * @WLAN_STOP_ALL_NETIF_QUEUE: stop all netif queues
 * @WLAN_START_ALL_NETIF_QUEUE: start all netif queues
 * @WLAN_WAKE_ALL_NETIF_QUEUE: wake all netif queues
 * @WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER: stop all queues and off carrier
 * @WLAN_START_ALL_NETIF_QUEUE_N_CARRIER: start all queues and on carrier
 * @WLAN_NETIF_TX_DISABLE: disable tx
 * @WLAN_NETIF_TX_DISABLE_N_CARRIER: disable tx and off carrier
 * @WLAN_NETIF_CARRIER_ON: on carrier
 * @WLAN_NETIF_CARRIER_OFF: off carrier
 */
enum netif_action_type {
	WLAN_STOP_ALL_NETIF_QUEUE,
	WLAN_START_ALL_NETIF_QUEUE,
	WLAN_WAKE_ALL_NETIF_QUEUE,
	WLAN_STOP_ALL_NETIF_QUEUE_N_CARRIER,
	WLAN_START_ALL_NETIF_QUEUE_N_CARRIER,
	WLAN_NETIF_TX_DISABLE,
	WLAN_NETIF_TX_DISABLE_N_CARRIER,
	WLAN_NETIF_CARRIER_ON,
	WLAN_NETIF_CARRIER_OFF,
	WLAN_NETIF_ACTION_TYPE_MAX,
};

/**
 * enum netif_reason_type - reason for netif queue action
 * @WLAN_CONTROL_PATH: action from control path
 * @WLAN_DATA_FLOW_CONTROL: because of flow control
 * @WLAN_FW_PAUSE: because of firmware pause
 * @WLAN_TX_ABORT: because of tx abort
 * @WLAN_VDEV_STOP: because of vdev stop
 * @WLAN_PEER_UNAUTHORISED: because of peer is unauthorised
 * @WLAN_THERMAL_MITIGATION: because of thermal mitigation
 */
enum netif_reason_type {
	WLAN_CONTROL_PATH,
	WLAN_DATA_FLOW_CONTROL,
	WLAN_FW_PAUSE,
	WLAN_TX_ABORT,
	WLAN_VDEV_STOP,
	WLAN_PEER_UNAUTHORISED,
	WLAN_THERMAL_MITIGATION,
	WLAN_REASON_TYPE_MAX,
};


/* command options for dumpStats*/
#define WLAN_HDD_STATS        0
#define WLAN_TXRX_STATS       1
#define WLAN_TXRX_HIST_STATS  2
#define WLAN_TXRX_TSO_STATS   3
#define WLAN_HDD_NETIF_OPER_HISTORY 4
#define WLAN_DUMP_TX_FLOW_POOL_INFO 5
#define WLAN_TXRX_DESC_STATS  6

ol_txrx_pdev_handle
ol_txrx_pdev_alloc(ol_pdev_handle ctrl_pdev,
		   HTC_HANDLE htc_pdev, qdf_device_t osdev);

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
ol_txrx_pdev_attach(ol_txrx_pdev_handle pdev);

/**
 * @brief Do final steps of data SW setup that send messages to the target.
 * @details
 *  The majority of the data SW setup are done by the pdev_attach function,
 *  but this function completes the data SW setup by sending datapath
 *  configuration messages to the target.
 *
 * @param data_pdev - the physical device being initialized
 */
A_STATUS ol_txrx_pdev_attach_target(ol_txrx_pdev_handle data_pdev);

/**
 * @brief Allocate and initialize the data object for a new virtual device.
 * @param data_pdev - the physical device the virtual device belongs to
 * @param vdev_mac_addr - the MAC address of the virtual device
 * @param vdev_id - the ID used to identify the virtual device to the target
 * @param op_mode - whether this virtual device is operating as an AP,
 *      an IBSS, or a STA
 * @return
 *      success: handle to new data vdev object, -OR-
 *      failure: NULL
 */
ol_txrx_vdev_handle
ol_txrx_vdev_attach(ol_txrx_pdev_handle data_pdev,
		    uint8_t *vdev_mac_addr,
		    uint8_t vdev_id, enum wlan_op_mode op_mode);

/**
 * @brief Allocate and set up references for a data peer object.
 * @details
 *  When an association with a peer starts, the host's control SW
 *  uses this function to inform the host data SW.
 *  The host data SW allocates its own peer object, and stores a
 *  reference to the control peer object within the data peer object.
 *  The host data SW also stores a reference to the virtual device
 *  that the peer is associated with.  This virtual device handle is
 *  used when the data SW delivers rx data frames to the OS shim layer.
 *  The host data SW returns a handle to the new peer data object,
 *  so a reference within the control peer object can be set to the
 *  data peer object.
 *
 * @param data_pdev - data physical device object that will indirectly
 *      own the data_peer object
 * @param data_vdev - data virtual device object that will directly
 *      own the data_peer object
 * @param peer_mac_addr - MAC address of the new peer
 * @return handle to new data peer object, or NULL if the attach fails
 */
ol_txrx_peer_handle
ol_txrx_peer_attach(ol_txrx_pdev_handle data_pdev,
		    ol_txrx_vdev_handle data_vdev, uint8_t *peer_mac_addr);

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

/**
 * @brief Set paramters of WMM scheduler per AC settings.  .
 * @details
 *  This function applies only to HL systems.
 *
 * @param data_pdev - the physical device being paused
 * @param wmm_param - the wmm parameters
 */
#define ol_txrx_set_wmm_param(data_pdev, wmm_param)     /* no-op */

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
#define ol_txrx_peer_pause(data_peer)   /* no-op */

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
#define ol_txrx_peer_tid_unpause(data_peer, tid)        /* no-op */

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
#define ol_txrx_tx_release(peer, tid_mask, max_frms)    /* no-op */

/**
 * @brief Suspend all tx data for the specified virtual device.
 * @details
 *  This function applies primarily to HL systems, but also applies to
 *  LL systems that use per-vdev tx queues for MCC or thermal throttling.
 *  As an example, this function could be used when a single-channel physical
 *  device supports multiple channels by jumping back and forth between the
 *  channels in a time-shared manner.  As the device is switched from channel
 *  A to channel B, the virtual devices that operate on channel A will be
 *  paused.
 *
 * @param data_vdev - the virtual device being paused
 * @param reason - the reason for which vdev queue is getting paused
 */
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
void ol_txrx_vdev_pause(ol_txrx_vdev_handle vdev, uint32_t reason);
#else
static inline
void ol_txrx_vdev_pause(ol_txrx_vdev_handle vdev, uint32_t reason)
{
	return;
}
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

/**
 * @brief Drop all tx data for the specified virtual device.
 * @details
 *  This function applies primarily to HL systems, but also applies to
 *  LL systems that use per-vdev tx queues for MCC or thermal throttling.
 *  This function would typically be used by the ctrl SW after it parks
 *  a STA vdev and then resumes it, but to a new AP.  In this case, though
 *  the same vdev can be used, any old tx frames queued inside it would be
 *  stale, and would need to be discarded.
 *
 * @param data_vdev - the virtual device being flushed
 */
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL)
void ol_txrx_vdev_flush(ol_txrx_vdev_handle data_vdev);
#else
#define ol_txrx_vdev_flush(data_vdev)   /* no-op */
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

/**
 * @brief Resume tx for the specified virtual device.
 * @details
 *  This function applies primarily to HL systems, but also applies to
 *  LL systems that use per-vdev tx queues for MCC or thermal throttling.
 *
 * @param data_vdev - the virtual device being unpaused
 * @param reason - the reason for which vdev queue is getting unpaused
 */
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
void ol_txrx_vdev_unpause(ol_txrx_vdev_handle data_vdev, uint32_t reason);
#else
static inline
void ol_txrx_vdev_unpause(ol_txrx_vdev_handle data_vdev, uint32_t reason)
{
	return;
}
#endif /* QCA_LL_LEGACY_TX_FLOW_CONTROL */

/**
 * @brief Suspend all tx data per thermal event/timer for the
 *  specified physical device
 * @details
 *  This function applies only to HL systerms, and it makes pause and
 * unpause operations happen in pairs.
 */
#define ol_txrx_throttle_pause(data_pdev)       /* no-op */

/**
 * @brief Resume all tx data per thermal event/timer for the
 * specified physical device
 * @details
 *  This function applies only to HL systerms, and it makes pause and
 * unpause operations happen in pairs.
 */
#define ol_txrx_throttle_unpause(data_pdev)     /* no-op */

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
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
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
#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) || defined(QCA_LL_TX_FLOW_CONTROL_V2)
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

/**
 * @brief Delete a peer's data object.
 * @details
 *  When the host's control SW disassociates a peer, it calls this
 *  function to delete the peer's data object.
 *  The reference stored in the control peer object to the data peer
 *  object (set up by a call to ol_peer_store()) is provided.
 *
 * @param data_peer - the object to delete
 */
void ol_txrx_peer_detach(ol_txrx_peer_handle data_peer);

typedef void (*ol_txrx_vdev_delete_cb)(void *context);

/**
 * @brief Deallocate the specified data virtual device object.
 * @details
 *  All peers associated with the virtual device need to be deleted
 *  (ol_txrx_peer_detach) before the virtual device itself is deleted.
 *  However, for the peers to be fully deleted, the peer deletion has to
 *  percolate through the target data FW and back up to the host data SW.
 *  Thus, even though the host control SW may have issued a peer_detach
 *  call for each of the vdev's peers, the peer objects may still be
 *  allocated, pending removal of all references to them by the target FW.
 *  In this case, though the vdev_detach function call will still return
 *  immediately, the vdev itself won't actually be deleted, until the
 *  deletions of all its peers complete.
 *  The caller can provide a callback function pointer to be notified when
 *  the vdev deletion actually happens - whether it's directly within the
 *  vdev_detach call, or if it's deferred until all in-progress peer
 *  deletions have completed.
 *
 * @param data_vdev - data object for the virtual device in question
 * @param callback - function to call (if non-NULL) once the vdev has
 *      been wholly deleted
 * @param callback_context - context to provide in the callback
 */
void
ol_txrx_vdev_detach(ol_txrx_vdev_handle data_vdev,
		    ol_txrx_vdev_delete_cb callback, void *callback_context);

/**
 * @brief Delete the data SW state.
 * @details
 *  This function is used when the WLAN driver is being removed to
 *  remove the host data component within the driver.
 *  All virtual devices within the physical device need to be deleted
 *  (ol_txrx_vdev_detach) before the physical device itself is deleted.
 *
 * @param data_pdev - the data physical device object being removed
 * @param force - delete the pdev (and its vdevs and peers) even if there
 *      are outstanding references by the target to the vdevs and peers
 *      within the pdev
 */
void ol_txrx_pdev_detach(ol_txrx_pdev_handle data_pdev, int force);

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

/**
 * @brief Allow the control-path SW to send data frames.
 * @details
 *  Generally, all tx data frames come from the OS shim into the txrx layer.
 *  However, there are rare cases such as TDLS messaging where the UMAC
 *  control-path SW creates tx data frames.
 *  This UMAC SW can call this function to provide the tx data frames to
 *  the txrx layer.
 *  The UMAC SW can request a callback for these data frames after their
 *  transmission completes, by using the ol_txrx_data_tx_cb_set function
 *  to register a tx completion callback, and by specifying
 *  ol_tx_spec_no_free as the tx_spec arg when giving the frames to
 *  ol_tx_non_std.
 *  The MSDUs need to have the appropriate L2 header type (802.3 vs. 802.11),
 *  as specified by ol_cfg_frame_type().
 *
 * @param data_vdev - which vdev should transmit the tx data frames
 * @param tx_spec - what non-standard handling to apply to the tx data frames
 * @param msdu_list - NULL-terminated list of tx MSDUs
 */
qdf_nbuf_t
ol_tx_non_std(ol_txrx_vdev_handle data_vdev,
	      enum ol_tx_spec tx_spec, qdf_nbuf_t msdu_list);

typedef void
(*ol_txrx_mgmt_tx_cb)(void *ctxt, qdf_nbuf_t tx_mgmt_frm, int had_error);

/**
 * @brief Store a callback for delivery notifications for management frames.
 * @details
 *  When the txrx SW receives notifications from the target that a tx frame
 *  has been delivered to its recipient, it will check if the tx frame
 *  is a management frame.  If so, the txrx SW will check the management
 *  frame type specified when the frame was submitted for transmission.
 *  If there is a callback function registered for the type of managment
 *  frame in question, the txrx code will invoke the callback to inform
 *  the management + control SW that the mgmt frame was delivered.
 *  This function is used by the control SW to store a callback pointer
 *  for a given type of management frame.
 *
 * @param pdev - the data physical device object
 * @param type - the type of mgmt frame the callback is used for
 * @param download_cb - the callback for notification of delivery to the target
 * @param ota_ack_cb - the callback for notification of delivery to the peer
 * @param ctxt - context to use with the callback
 */
void
ol_txrx_mgmt_tx_cb_set(ol_txrx_pdev_handle pdev,
		       uint8_t type,
		       ol_txrx_mgmt_tx_cb download_cb,
		       ol_txrx_mgmt_tx_cb ota_ack_cb, void *ctxt);

/**
 * @brief Transmit a management frame.
 * @details
 *  Send the specified management frame from the specified virtual device.
 *  The type is used for determining whether to invoke a callback to inform
 *  the sender that the tx mgmt frame was delivered, and if so, which
 *  callback to use.
 *
 * @param vdev - virtual device transmitting the frame
 * @param tx_mgmt_frm - management frame to transmit
 * @param type - the type of managment frame (determines what callback to use)
 * @param use_6mbps - specify whether management frame to transmit should use 6 Mbps
 *                    rather than 1 Mbps min rate(for 5GHz band or P2P)
 * @return
 *      0 -> the frame is accepted for transmission, -OR-
 *      1 -> the frame was not accepted
 */
int
ol_txrx_mgmt_send(ol_txrx_vdev_handle vdev,
		  qdf_nbuf_t tx_mgmt_frm,
		  uint8_t type, uint8_t use_6mbps, uint16_t chanfreq);

/**
 * @brief Setup the monitor mode vap (vdev) for this pdev
 * @details
 *  When a non-NULL vdev handle is registered as the monitor mode vdev, all
 *  packets received by the system are delivered to the OS stack on this
 *  interface in 802.11 MPDU format. Only a single monitor mode interface
 *  can be up at any timer. When the vdev handle is set to NULL the monitor
 *  mode delivery is stopped. This handle may either be a unique vdev
 *  object that only receives monitor mode packets OR a point to a a vdev
 *  object that also receives non-monitor traffic. In the second case the
 *  OS stack is responsible for delivering the two streams using approprate
 *  OS APIs
 *
 * @param pdev - the data physical device object
 * @param vdev - the data virtual device object to deliver monitor mode
 *                  packets on
 * @return
 *       0 -> the monitor mode vap was sucessfully setup
 *      -1 -> Unable to setup monitor mode
 */
int
ol_txrx_set_monitor_mode_vap(ol_txrx_pdev_handle pdev,
			     ol_txrx_vdev_handle vdev);

/**
 * @brief Setup the current operating channel of the device
 * @details
 *  Mainly used when populating monitor mode status that requires the
 *  current operating channel
 *
 * @param pdev - the data physical device object
 * @param chan_mhz - the channel frequency (mhz)
 *                  packets on
 * @return - void
 */
void ol_txrx_set_curchan(ol_txrx_pdev_handle pdev, uint32_t chan_mhz);

QDF_STATUS ol_txrx_bus_suspend(void);
QDF_STATUS ol_txrx_bus_resume(void);
QDF_STATUS ol_txrx_wait_for_pending_tx(int timeout);

/**
 * @brief Get the number of pending transmit frames that are awaiting completion.
 * @details
 *  Mainly used in clean up path to make sure all buffers have been free'ed
 *
 * @param pdev - the data physical device object
 * @return - count of pending frames
 */
int ol_txrx_get_tx_pending(ol_txrx_pdev_handle pdev);

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
 * @brief set the privacy filter
 * @details
 *  Rx related. Set the privacy filters. When rx packets, check
 *  the ether type, filter type and packet type
 *  to decide whether discard these packets.
 *
 * @param vdev - the data virtual device object
 * @param filter - filters to be set
 * @param num - the number of filters
 * @return - void
 */
void
ol_txrx_set_privacy_filters(ol_txrx_vdev_handle vdev,
			    void *filter, uint32_t num);

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

enum ol_txrx_peer_state {
	ol_txrx_peer_state_invalid,
	ol_txrx_peer_state_disc,        /* initial state */
	ol_txrx_peer_state_conn,        /* authentication in progress */
	ol_txrx_peer_state_auth,        /* authentication successful */
};

/**
 * @brief specify the peer's authentication state
 * @details
 *  Specify the peer's authentication state (none, connected, authenticated)
 *  to allow the data SW to determine whether to filter out invalid data frames.
 *  (In the "connected" state, where security is enabled, but authentication
 *  has not completed, tx and rx data frames other than EAPOL or WAPI should
 *  be discarded.)
 *  This function is only relevant for systems in which the tx and rx filtering
 *  are done in the host rather than in the target.
 *
 * @param data_peer - which peer has changed its state
 * @param state - the new state of the peer
 *
 * Return: CDF Status
 */
QDF_STATUS
ol_txrx_peer_state_update(ol_txrx_pdev_handle pdev, uint8_t *peer_addr,
			  enum ol_txrx_peer_state state);

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

/**
 * @brief Find a txrx peer handle from a peer's local ID
 * @details
 *  The control SW typically uses the txrx peer handle to refer to the peer.
 *  In unusual circumstances, if it is infeasible for the control SW maintain
 *  the txrx peer handle but it can maintain a small integer local peer ID,
 *  this function allows the peer handled to be retrieved, based on the local
 *  peer ID.
 *
 * @param pdev - the data physical device object
 * @param local_peer_id - the ID txrx assigned locally to the peer in question
 * @return handle to the txrx peer object
 */
#if QCA_SUPPORT_TXRX_LOCAL_PEER_ID
ol_txrx_peer_handle
ol_txrx_peer_find_by_local_id(ol_txrx_pdev_handle pdev, uint8_t local_peer_id);
#else
#define ol_txrx_peer_find_by_local_id(pdev, local_peer_id) NULL
#endif

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

/* Config parameters for txrx_pdev */
struct txrx_pdev_cfg_param_t {
	uint8_t is_full_reorder_offload;
	/* IPA Micro controller data path offload enable flag */
	uint8_t is_uc_offload_enabled;
	/* IPA Micro controller data path offload TX buffer count */
	uint32_t uc_tx_buffer_count;
	/* IPA Micro controller data path offload TX buffer size */
	uint32_t uc_tx_buffer_size;
	/* IPA Micro controller data path offload RX indication ring count */
	uint32_t uc_rx_indication_ring_count;
	/* IPA Micro controller data path offload TX partition base */
	uint32_t uc_tx_partition_base;
	/* IP, TCP and UDP checksum offload */
	bool ip_tcp_udp_checksum_offload;
	/* Rx processing in thread from TXRX */
	bool enable_rxthread;
	/* CE classification enabled through INI */
	bool ce_classify_enabled;
#ifdef QCA_LL_TX_FLOW_CONTROL_V2
	/* Threshold to stop queue in percentage */
	uint32_t tx_flow_stop_queue_th;
	/* Start queue offset in percentage */
	uint32_t tx_flow_start_queue_offset;
#endif
};

/**
 * @brief Setup configuration parameters
 * @details
 *  Allocation configuration context that will be used across data path
 *
 * @param osdev - OS handle needed as an argument for some OS primitives
 * @return the control device object
 */
ol_pdev_handle ol_pdev_cfg_attach(qdf_device_t osdev,
				  struct txrx_pdev_cfg_param_t cfg_param);

QDF_STATUS ol_txrx_get_vdevid(struct ol_txrx_peer_t *peer, uint8_t *vdev_id);
void *ol_txrx_get_vdev_by_sta_id(uint8_t sta_id);


#define OL_TXRX_INVALID_LOCAL_PEER_ID 0xffff
#ifdef QCA_SUPPORT_TXRX_LOCAL_PEER_ID
uint16_t ol_txrx_local_peer_id(ol_txrx_peer_handle peer);
ol_txrx_peer_handle ol_txrx_find_peer_by_addr(ol_txrx_pdev_handle pdev,
					      uint8_t *peer_addr,
					      uint8_t *peer_id);
ol_txrx_peer_handle
ol_txrx_find_peer_by_addr_and_vdev(ol_txrx_pdev_handle pdev,
				   ol_txrx_vdev_handle vdev,
				   uint8_t *peer_addr, uint8_t *peer_id);
#else
#define ol_txrx_local_peer_id(peer) OL_TXRX_INVALID_LOCAL_PEER_ID
#define ol_txrx_find_peer_by_addr(pdev, peer_addr, peer_id) NULL
#define ol_txrx_find_peer_by_addr_and_vdev(pdev, vdev, peer_addr, peer_id) NULL
#endif

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

#define OL_TXRX_INVALID_LOCAL_PEER_ID 0xffff
#if QCA_SUPPORT_TXRX_LOCAL_PEER_ID
uint16_t ol_txrx_local_peer_id(ol_txrx_peer_handle peer);
#else
#define ol_txrx_local_peer_id(peer) OL_TXRX_INVALID_LOCAL_PEER_ID
#endif

#ifdef QCA_COMPUTE_TX_DELAY
/**
 * @brief updates the compute interval period for TSM stats.
 * @details
 * @param interval - interval for stats computation
 */
void ol_tx_set_compute_interval(ol_txrx_pdev_handle pdev, uint32_t interval);

/**
 * @brief Return the uplink (transmitted) packet count and loss count.
 * @details
 *  This function will be called for getting uplink packet count and
 *  loss count for given stream (access category) a regular interval.
 *  This also resets the counters hence, the value returned is packets
 *  counted in last 5(default) second interval. These counter are
 *  incremented per access category in ol_tx_completion_handler()
 *
 * @param category - access category of interest
 * @param out_packet_count - number of packets transmitted
 * @param out_packet_loss_count - number of packets lost
 */
void
ol_tx_packet_count(ol_txrx_pdev_handle pdev,
		   uint16_t *out_packet_count,
		   uint16_t *out_packet_loss_count, int category);
#endif

/**
 * @brief Return the average delays for tx frames.
 * @details
 *  Return the average of the total time tx frames spend within the driver
 *  and the average time tx frames take to be transmitted.
 *  These averages are computed over a 5 second time interval.
 *  These averages are computed separately for separate access categories,
 *  if the QCA_COMPUTE_TX_DELAY_PER_AC flag is set.
 *
 * @param pdev - the data physical device instance
 * @param queue_delay_microsec - average time tx frms spend in the WLAN driver
 * @param tx_delay_microsec - average time for frames to be transmitted
 * @param category - category (TID) of interest
 */
#ifdef QCA_COMPUTE_TX_DELAY
void
ol_tx_delay(ol_txrx_pdev_handle pdev,
	    uint32_t *queue_delay_microsec,
	    uint32_t *tx_delay_microsec, int category);
#else
static inline void
ol_tx_delay(ol_txrx_pdev_handle pdev,
	    uint32_t *queue_delay_microsec,
	    uint32_t *tx_delay_microsec, int category)
{
	/* no-op version if QCA_COMPUTE_TX_DELAY is not set */
	*queue_delay_microsec = *tx_delay_microsec = 0;
}
#endif

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
/**
 * @brief Provide a histogram of tx queuing delays.
 * @details
 *  Return a histogram showing the number of tx frames of the specified
 *  category for each of the delay levels in the histogram bin spacings
 *  listed above.
 *  These histograms are computed over a 5 second time interval.
 *  These histograms are computed separately for separate access categories,
 *  if the QCA_COMPUTE_TX_DELAY_PER_AC flag is set.
 *
 * @param pdev - the data physical device instance
 * @param bin_values - an array of QCA_TX_DELAY_HIST_REPORT_BINS elements
 *      This array gets filled in with the histogram bin counts.
 * @param category - category (TID) of interest
 */
#ifdef QCA_COMPUTE_TX_DELAY
void
ol_tx_delay_hist(ol_txrx_pdev_handle pdev, uint16_t *bin_values, int category);
#else
static inline void
ol_tx_delay_hist(ol_txrx_pdev_handle pdev, uint16_t *bin_values, int category)
{
	/* no-op version if QCA_COMPUTE_TX_DELAY is not set */
	qdf_assert(bin_values);
	qdf_mem_zero(bin_values,
		     QCA_TX_DELAY_HIST_REPORT_BINS * sizeof(*bin_values));
}
#endif

#if defined(QCA_SUPPORT_TX_THROTTLE)
/**
 * @brief Set the thermal mitgation throttling level.
 * @details
 *  This function applies only to LL systems. This function is used set the
 *  tx throttle level used for thermal mitigation
 *
 * @param pdev - the physics device being throttled
 */
void ol_tx_throttle_set_level(struct ol_txrx_pdev_t *pdev, int level);
#else
static inline void ol_tx_throttle_set_level(struct ol_txrx_pdev_t *pdev,
					    int level)
{
	/* no-op */
}
#endif /* QCA_SUPPORT_TX_THROTTLE */

#if defined(QCA_SUPPORT_TX_THROTTLE)
/**
 * @brief Configure the thermal mitgation throttling period.
 * @details
 *  This function applies only to LL systems. This function is used set the
 *  period over which data will be throttled
 *
 * @param pdev - the physics device being throttled
 */
void ol_tx_throttle_init_period(struct ol_txrx_pdev_t *pdev, int period);
#else
static inline void ol_tx_throttle_init_period(struct ol_txrx_pdev_t *pdev,
					      int period)
{
	/* no-op */
}
#endif /* QCA_SUPPORT_TX_THROTTLE */

void ol_vdev_rx_set_intrabss_fwd(ol_txrx_vdev_handle vdev, bool val);


#ifdef IPA_OFFLOAD
void
ol_txrx_ipa_uc_get_resource(ol_txrx_pdev_handle pdev,
			    qdf_dma_addr_t *ce_sr_base_paddr,
			    uint32_t *ce_sr_ring_size,
			    qdf_dma_addr_t *ce_reg_paddr,
			    qdf_dma_addr_t *tx_comp_ring_base_paddr,
			    uint32_t *tx_comp_ring_size,
			    uint32_t *tx_num_alloc_buffer,
			    qdf_dma_addr_t *rx_rdy_ring_base_paddr,
			    uint32_t *rx_rdy_ring_size,
			    qdf_dma_addr_t *rx_proc_done_idx_paddr,
			    void **rx_proc_done_idx_vaddr,
			    qdf_dma_addr_t *rx2_rdy_ring_base_paddr,
			    uint32_t *rx2_rdy_ring_size,
			    qdf_dma_addr_t *rx2_proc_done_idx_paddr,
			    void **rx2_proc_done_idx_vaddr);


void
ol_txrx_ipa_uc_set_doorbell_paddr(ol_txrx_pdev_handle pdev,
				  qdf_dma_addr_t ipa_tx_uc_doorbell_paddr,
				  qdf_dma_addr_t ipa_rx_uc_doorbell_paddr);

void
ol_txrx_ipa_uc_set_active(ol_txrx_pdev_handle pdev, bool uc_active, bool is_tx);

void ol_txrx_ipa_uc_op_response(ol_txrx_pdev_handle pdev, uint8_t *op_msg);

void ol_txrx_ipa_uc_register_op_cb(ol_txrx_pdev_handle pdev,
				   void (*ipa_uc_op_cb_type)(uint8_t *op_msg,
							     void *osif_ctxt),
				   void *osif_dev);

void ol_txrx_ipa_uc_get_stat(ol_txrx_pdev_handle pdev);
#else
/**
 * ol_txrx_ipa_uc_get_resource() - Client request resource information
 * @pdev: handle to the HTT instance
 * @ce_sr_base_paddr: copy engine source ring base physical address
 * @ce_sr_ring_size: copy engine source ring size
 * @ce_reg_paddr: copy engine register physical address
 * @tx_comp_ring_base_paddr: tx comp ring base physical address
 * @tx_comp_ring_size: tx comp ring size
 * @tx_num_alloc_buffer: number of allocated tx buffer
 * @rx_rdy_ring_base_paddr: rx ready ring base physical address
 * @rx_rdy_ring_size: rx ready ring size
 * @rx_proc_done_idx_paddr: rx process done index physical address
 * @rx_proc_done_idx_vaddr: rx process done index virtual address
 * @rx2_rdy_ring_base_paddr: rx done ring base physical address
 * @rx2_rdy_ring_size: rx done ring size
 * @rx2_proc_done_idx_paddr: rx done index physical address
 * @rx2_proc_done_idx_vaddr: rx done index virtual address
 *
 *  OL client will reuqest IPA UC related resource information
 *  Resource information will be distributted to IPA module
 *  All of the required resources should be pre-allocated
 *
 * Return: none
 */
static inline void
ol_txrx_ipa_uc_get_resource(ol_txrx_pdev_handle pdev,
			    qdf_dma_addr_t *ce_sr_base_paddr,
			    uint32_t *ce_sr_ring_size,
			    qdf_dma_addr_t *ce_reg_paddr,
			    qdf_dma_addr_t *tx_comp_ring_base_paddr,
			    uint32_t *tx_comp_ring_size,
			    uint32_t *tx_num_alloc_buffer,
			    qdf_dma_addr_t *rx_rdy_ring_base_paddr,
			    uint32_t *rx_rdy_ring_size,
			    qdf_dma_addr_t *rx_proc_done_idx_paddr,
			    void **rx_proc_done_idx_vaddr,
			    qdf_dma_addr_t *rx2_rdy_ring_base_paddr,
			    uint32_t *rx2_rdy_ring_size,
			    qdf_dma_addr_t *rx2_proc_done_idx_paddr,
			    void **rx2_proc_done_idx_vaddr)
{
	return;
}

/**
 * ol_txrx_ipa_uc_set_doorbell_paddr() - Client set IPA UC doorbell register
 * @pdev: handle to the HTT instance
 * @ipa_uc_tx_doorbell_paddr: tx comp doorbell physical address
 * @ipa_uc_rx_doorbell_paddr: rx ready doorbell physical address
 *
 *  IPA UC let know doorbell register physical address
 *  WLAN firmware will use this physical address to notify IPA UC
 *
 * Return: none
 */
static inline void
ol_txrx_ipa_uc_set_doorbell_paddr(ol_txrx_pdev_handle pdev,
				  qdf_dma_addr_t ipa_tx_uc_doorbell_paddr,
				  qdf_dma_addr_t ipa_rx_uc_doorbell_paddr)
{
	return;
}

/**
 * ol_txrx_ipa_uc_set_active() - Client notify IPA UC data path active or not
 * @pdev: handle to the HTT instance
 * @ipa_uc_tx_doorbell_paddr: tx comp doorbell physical address
 * @ipa_uc_rx_doorbell_paddr: rx ready doorbell physical address
 *
 *  IPA UC let know doorbell register physical address
 *  WLAN firmware will use this physical address to notify IPA UC
 *
 * Return: none
 */
static inline void
ol_txrx_ipa_uc_set_active(ol_txrx_pdev_handle pdev,
	bool uc_active, bool is_tx)
{
	return;
}

/**
 * ol_txrx_ipa_uc_op_response() - Handle OP command response from firmware
 * @pdev: handle to the HTT instance
 * @op_msg: op response message from firmware
 *
 * Return: none
 */
static inline void
ol_txrx_ipa_uc_op_response(ol_txrx_pdev_handle pdev, uint8_t *op_msg)
{
	return;
}

/**
 * ol_txrx_ipa_uc_register_op_cb() - Register OP handler function
 * @pdev: handle to the HTT instance
 * @op_cb: handler function pointer
 * @osif_dev: register client context
 *
 * Return: none
 */
static inline void
ol_txrx_ipa_uc_register_op_cb(ol_txrx_pdev_handle pdev,
				   void (*ipa_uc_op_cb_type)(uint8_t *op_msg,
							     void *osif_ctxt),
				   void *osif_dev)
{
	return;
}

/**
 * ol_txrx_ipa_uc_get_stat() - Get firmware wdi status
 * @pdev: handle to the HTT instance
 *
 * Return: none
 */
static inline void ol_txrx_ipa_uc_get_stat(ol_txrx_pdev_handle pdev)
{
	return;
}
#endif /* IPA_OFFLOAD */

void ol_txrx_display_stats(uint16_t bitmap);
void ol_txrx_clear_stats(uint16_t bitmap);
int ol_txrx_stats(uint8_t vdev_id, char *buffer, unsigned buf_len);

QDF_STATUS ol_txrx_register_ocb_peer(void *cds_ctx, uint8_t *mac_addr,
				     uint8_t *peer_id);

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
int ol_tx_delete_flow_pool(struct ol_tx_flow_pool_t *pool);
void ol_tx_set_desc_global_pool_size(uint32_t num_msdu_desc);
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
static inline int ol_tx_delete_flow_pool(struct ol_tx_flow_pool_t *pool)
{
	return 0;
}
static inline void ol_tx_set_desc_global_pool_size(uint32_t num_msdu_desc)
{
	return;
}
#endif

#endif /* _OL_TXRX_CTRL_API__H_ */
