/*
 * Copyright (c) 2013-2014, 2016-2020 The Linux Foundation. All rights reserved.
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

#ifndef _HTC_API_H_
#define _HTC_API_H_

#include <htc.h>
#include <htc_services.h>
#include <qdf_types.h>          /* qdf_device_t */
#include "htc_packet.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* TODO.. for BMI */
#define ENDPOINT1 0
/* TODO -remove me, but we have to fix BMI first */
#define HTC_MAILBOX_NUM_MAX    4

/* this is the amount of header room required by users of HTC */
#define HTC_HEADER_LEN         HTC_HDR_LENGTH

#define HTC_HTT_TRANSFER_HDRSIZE 24

/*
 * NOTE WELL: struct opaque_htc_handle is not defined anywhere. This
 * reference is used to help ensure that a HTC_HANDLE is never used
 * where a different handle type is expected
 */
struct opaque_htc_handle;
typedef struct opaque_htc_handle *HTC_HANDLE;

typedef uint16_t HTC_SERVICE_ID;

typedef void (*HTC_TARGET_FAILURE)(void *Instance, QDF_STATUS Status);

struct htc_init_info {
	void *pContext;         /* context for target notifications */
	void (*TargetFailure)(void *Instance, QDF_STATUS Status);
	void (*TargetSendSuspendComplete)(void *ctx, bool is_nack);
	void (*target_initial_wakeup_cb)(void *cb_ctx);
	void *target_psoc;
	uint32_t cfg_wmi_credit_cnt;
};

/* Struct for HTC layer packet stats*/
struct ol_ath_htc_stats {
	int htc_get_pkt_q_fail_count;
	int htc_pkt_q_empty_count;
	int htc_send_q_empty_count;
};

/* To resume HTT Tx queue during runtime resume */
typedef void (*HTC_EP_RESUME_TX_QUEUE)(void *);

typedef int (*HTC_EP_PADDING_CREDIT_UPDATE) (void *, int);

/* per service connection send completion */
typedef void (*HTC_EP_SEND_PKT_COMPLETE)(void *, HTC_PACKET *);
/* per service connection callback when a plurality of packets have been sent
 * The HTC_PACKET_QUEUE is a temporary queue object (e.g. freed on return from
 * the callback) to hold a list of completed send packets.
 * If the handler cannot fully traverse the packet queue before returning, it
 * should transfer the items of the queue into the caller's private queue using:
 * HTC_PACKET_ENQUEUE()
 */
typedef void (*HTC_EP_SEND_PKT_COMP_MULTIPLE)(void *,
					      HTC_PACKET_QUEUE *);
/* per service connection pkt received */
typedef void (*HTC_EP_RECV_PKT)(void *, HTC_PACKET *);
/* per service connection callback when a plurality of packets are received
 * The HTC_PACKET_QUEUE is a temporary queue object (e.g. freed on return from
 * the callback) to hold a list of recv packets.
 * If the handler cannot fully traverse the packet queue before returning, it
 * should transfer the items of the queue into the caller's private queue using:
 * HTC_PACKET_ENQUEUE()
 */
typedef void (*HTC_EP_RECV_PKT_MULTIPLE)(void *, HTC_PACKET_QUEUE *);

/* Optional per service connection receive buffer re-fill callback,
 * On some OSes (like Linux) packets are allocated from a global pool and
 * indicated up to the network stack.  The driver never gets the packets back
 * from the OS. For these OSes a refill callback can be used to allocate and
 * re-queue buffers into HTC.
 *
 * On other OSes, the network stack can call into the driver's OS-specific
 * "return_packet" handler and the driver can re-queue these buffers into HTC.
 * In this regard a refill callback is unnecessary
 */
typedef void (*HTC_EP_RECV_REFILL)(void *, HTC_ENDPOINT_ID Endpoint);

/* Optional per service connection receive buffer allocation callback.
 * On some systems packet buffers are an extremely limited resource. Rather than
 * queue largest-possible-sized buffers to HTC, some systems would rather
 * allocate a specific size as the packet is received.  The trade off is
 * slightly more processing (callback invoked for each RX packet)
 * for the benefit of committing fewer buffer resources into HTC.
 *
 * The callback is provided the length of the pending packet to fetch. This
 * includes the HTC header length plus the length of payload.  The callback can
 * return a pointer to the allocated HTC packet for immediate use.
 *
 * Alternatively a variant of this handler can be used to allocate large receive
 * packets as needed. For example an application can use the refill mechanism
 * for normal packets and the recv-alloc mechanism to handle the case where a
 * large packet buffer is required.  This can significantly reduce the
 * amount of "committed" memory used to receive packets.
 */
typedef HTC_PACKET *(*HTC_EP_RECV_ALLOC)(void *,
					 HTC_ENDPOINT_ID Endpoint,
					 int Length);

/* Optional per service connection callback to log packet information.
 */
typedef void (*HTC_EP_LOG_PKT)(void *, HTC_PACKET *);

enum htc_send_full_action {
	/* packet that overflowed should be kept in the queue */
	HTC_SEND_FULL_KEEP = 0,
	/* packet that overflowed should be dropped */
	HTC_SEND_FULL_DROP = 1,
};

/* Optional per service connection callback when a send queue is full. This can
 * occur if host continues queueing up TX packets faster than credits can arrive
 * To prevent the host (on some Oses like Linux) from continuously queueing pkts
 * and consuming resources, this callback is provided so that that the host
 * can disable TX in the subsystem (i.e. network stack).
 * This callback is invoked for each packet that "overflows" the HTC queue. The
 * callback can determine whether the new packet that overflowed the queue can
 * be kept (HTC_SEND_FULL_KEEP) or dropped (HTC_SEND_FULL_DROP). If a packet is
 * dropped, the EpTxComplete handler will be called and the packet's status
 * field will be set to A_NO_RESOURCE.
 * Other OSes require a "per-packet" indication for each completed TX packet,
 * this closed loop mechanism will prevent the network stack from overunning the
 * NIC. The packet to keep or drop is passed for inspection to the registered
 * handler the handler must ONLY inspect the packet, it may not free or reclaim
 * the packet.
 */
typedef enum htc_send_full_action (*HTC_EP_SEND_QUEUE_FULL)(void *,
					       HTC_PACKET *pPacket);

struct htc_ep_callbacks {
	/* context for each callback */
	void *pContext;
	/* tx completion callback for connected endpoint */
	HTC_EP_SEND_PKT_COMPLETE EpTxComplete;
	/* receive callback for connected endpoint */
	HTC_EP_RECV_PKT EpRecv;
	/* OPTIONAL receive re-fill callback for connected endpoint */
	HTC_EP_RECV_REFILL EpRecvRefill;
	/* OPTIONAL send full callback */
	HTC_EP_SEND_QUEUE_FULL EpSendFull;
	/* OPTIONAL recv allocation callback */
	HTC_EP_RECV_ALLOC EpRecvAlloc;
	/* OPTIONAL recv allocation callback based on a threshold */
	HTC_EP_RECV_ALLOC EpRecvAllocThresh;
	/* OPTIONAL completion handler for multiple complete
	 * indications (EpTxComplete must be NULL)
	 */
	HTC_EP_SEND_PKT_COMP_MULTIPLE EpTxCompleteMultiple;

	HTC_EP_RESUME_TX_QUEUE ep_resume_tx_queue;

	HTC_EP_PADDING_CREDIT_UPDATE ep_padding_credit_update;
	/* if EpRecvAllocThresh is non-NULL, HTC will compare the
	 * threshold value to the current recv packet length and invoke
	 * the EpRecvAllocThresh callback to acquire a packet buffer
	 */
	int RecvAllocThreshold;
	/* if a EpRecvRefill handler is provided, this value
	 * can be used to set a trigger refill callback
	 * when the recv queue drops below this value
	 * if set to 0, the refill is only called when packets
	 * are empty
	 */
	int RecvRefillWaterMark;
	/* OPTIONAL callback to log packet information */
	HTC_EP_LOG_PKT ep_log_pkt;
};

/* service connection information */
struct htc_service_connect_req {
	/* service ID to connect to */
	HTC_SERVICE_ID service_id;
	/* connection flags, see htc protocol definition */
	uint16_t ConnectionFlags;
	/* ptr to optional service-specific meta-data */
	uint8_t *pMetaData;
	/* optional meta data length */
	uint8_t MetaDataLength;
	/* endpoint callbacks */
	struct htc_ep_callbacks EpCallbacks;
	/* maximum depth of any send queue */
	int MaxSendQueueDepth;
	/* HTC flags for the host-side (local) connection */
	uint32_t LocalConnectionFlags;
	/* override max message size in send direction */
	unsigned int MaxSendMsgSize;
};

/* enable send bundle padding for this endpoint */
#define HTC_LOCAL_CONN_FLAGS_ENABLE_SEND_BUNDLE_PADDING (1 << 0)

/* service connection response information */
struct htc_service_connect_resp {
	/* caller supplied buffer to optional meta-data */
	uint8_t *pMetaData;
	/* length of caller supplied buffer */
	uint8_t BufferLength;
	/* actual length of meta data */
	uint8_t ActualLength;
	/* endpoint to communicate over */
	HTC_ENDPOINT_ID Endpoint;
	/* max length of all messages over this endpoint */
	unsigned int MaxMsgLength;
	/* connect response code from target */
	uint8_t ConnectRespCode;
};

/* endpoint distribution structure */
struct htc_endpoint_credit_dist {
	struct _htc_endpoint_credit_dist *pNext;
	struct _htc_endpoint_credit_dist *pPrev;
	/* Service ID (set by HTC) */
	HTC_SERVICE_ID service_id;
	/* endpoint for this distribution struct (set by HTC) */
	HTC_ENDPOINT_ID Endpoint;
	/* distribution flags, distribution function can
	 * set default activity using SET_EP_ACTIVE() macro
	 */
	uint32_t DistFlags;
	/* credits for normal operation, anything above this
	 * indicates the endpoint is over-subscribed, this field
	 * is only relevant to the credit distribution function
	 */
	int TxCreditsNorm;
	/* floor for credit distribution, this field is
	 * only relevant to the credit distribution function
	 */
	int TxCreditsMin;
	/* number of credits assigned to this EP, this field
	 * is only relevant to the credit dist function
	 */
	int TxCreditsAssigned;
	/* current credits available, this field is used by
	 * HTC to determine whether a message can be sent or
	 * must be queued
	 */
	int TxCredits;
	/* pending credits to distribute on this endpoint, this
	 * is set by HTC when credit reports arrive.
	 * The credit distribution functions sets this to zero
	 * when it distributes the credits
	 */
	int TxCreditsToDist;
	/* this is the number of credits that the current pending TX
	 * packet needs to transmit.  This is set by HTC when
	 * and endpoint needs credits in order to transmit
	 */
	int TxCreditsSeek;
	/* size in bytes of each credit (set by HTC) */
	int TxCreditSize;
	/* credits required for a maximum sized messages (set by HTC) */
	int TxCreditsPerMaxMsg;
	/* reserved for HTC use */
	void *pHTCReserved;
	/* current depth of TX queue , i.e. messages waiting for credits
	 * This field is valid only when HTC_CREDIT_DIST_ACTIVITY_CHANGE
	 * or HTC_CREDIT_DIST_SEND_COMPLETE is indicated on an endpoint
	 * that has non-zero credits to recover
	 */
	int TxQueueDepth;
};

#define HTC_EP_ACTIVE                            ((uint32_t) (1u << 31))

/* macro to check if an endpoint has gone active, useful for credit
 * distributions */
#define IS_EP_ACTIVE(epDist)  ((epDist)->DistFlags & HTC_EP_ACTIVE)
#define SET_EP_ACTIVE(epDist) (epDist)->DistFlags |= HTC_EP_ACTIVE

/* credit distibution code that is passed into the distrbution function,
 * there are mandatory and optional codes that must be handled
 */
enum htc_credit_dist_reason {
	/* credits available as a result of completed
	 * send operations (MANDATORY) resulting in credit reports
	 */
	HTC_CREDIT_DIST_SEND_COMPLETE = 0,
	/* a change in endpoint activity occurred (OPTIONAL) */
	HTC_CREDIT_DIST_ACTIVITY_CHANGE = 1,
	/* an endpoint needs to "seek" credits (OPTIONAL) */
	HTC_CREDIT_DIST_SEEK_CREDITS,
	/* for debugging, dump any state information that is kept by
	 * the distribution function
	 */
	HTC_DUMP_CREDIT_STATE
};

typedef void (*HTC_CREDIT_DIST_CALLBACK)(void *Context,
					 struct htc_endpoint_credit_dist *
					 pEPList,
					 enum htc_credit_dist_reason
					 Reason);

typedef void (*HTC_CREDIT_INIT_CALLBACK)(void *Context,
					 struct htc_endpoint_credit_dist *
					 pEPList, int TotalCredits);

/* endpoint statistics action */
enum htc_endpoint_stat_action {
	/* only read statistics */
	HTC_EP_STAT_SAMPLE = 0,
	/* sample and immediately clear statistics */
	HTC_EP_STAT_SAMPLE_AND_CLEAR = 1,
	/* clear only */
	HTC_EP_STAT_CLEAR
};

/* endpoint statistics */
struct htc_endpoint_stats {
	/* number of TX packets posted to the endpoint */
	uint32_t TxPosted;
	/* number of times the host set the credit-low flag in a send message on
	 * this endpoint
	 */
	uint32_t TxCreditLowIndications;
	/* running count of total TX packets issued */
	uint32_t TxIssued;
	/* running count of TX packets that were issued in bundles */
	uint32_t TxPacketsBundled;
	/* running count of TX bundles that were issued */
	uint32_t TxBundles;
	/* tx packets that were dropped */
	uint32_t TxDropped;
	/* running count of total credit reports received for this endpoint */
	uint32_t TxCreditRpts;
	/* credit reports received from this endpoint's RX packets */
	uint32_t TxCreditRptsFromRx;
	/* credit reports received from RX packets of other endpoints */
	uint32_t TxCreditRptsFromOther;
	/* credit reports received from endpoint 0 RX packets */
	uint32_t TxCreditRptsFromEp0;
	/* count of credits received via Rx packets on this endpoint */
	uint32_t TxCreditsFromRx;
	/* count of credits received via another endpoint */
	uint32_t TxCreditsFromOther;
	/* count of credits received via another endpoint */
	uint32_t TxCreditsFromEp0;
	/* count of consummed credits */
	uint32_t TxCreditsConsummed;
	/* count of credits returned */
	uint32_t TxCreditsReturned;
	/* count of RX packets received */
	uint32_t RxReceived;
	/* count of lookahead records
	 * found in messages received on this endpoint
	 */
	uint32_t RxLookAheads;
	/* count of recv packets received in a bundle */
	uint32_t RxPacketsBundled;
	/* count of number of bundled lookaheads */
	uint32_t RxBundleLookAheads;
	/* count of the number of bundle indications from the HTC header */
	uint32_t RxBundleIndFromHdr;
	/* number of times the recv allocation threshold was hit */
	uint32_t RxAllocThreshHit;
	/* total number of bytes */
	uint32_t RxAllocThreshBytes;
};

/* ------ Function Prototypes ------ */
/**
 * htc_create - Create an instance of HTC over the underlying HIF device
 * @HifDevice: hif device handle,
 * @pInfo: initialization information
 * @osdev: QDF device structure
 * @con_mode: driver connection mode
 *
 * Return: HTC_HANDLE on success, NULL on failure
 */
HTC_HANDLE htc_create(void *HifDevice, struct htc_init_info *pInfo,
			qdf_device_t osdev, uint32_t con_mode);

/**
 * htc_get_hif_device - Get the underlying HIF device handle
 * @HTCHandle: handle passed into the AddInstance callback
 *
 * Return: opaque HIF device handle usable in HIF API calls.
 */
void *htc_get_hif_device(HTC_HANDLE HTCHandle);

/**
 * htc_set_credit_distribution - Set credit distribution parameters
 * @HTCHandle: HTC handle
 * @pCreditDistCont: caller supplied context to pass into distribution functions
 * @CreditDistFunc: Distribution function callback
 * @CreditDistInit: Credit Distribution initialization callback
 * @ServicePriorityOrder: Array containing list of service IDs, lowest index
 * @is highestpriority: ListLength - number of elements in ServicePriorityOrder
 *
 * The user can set a custom credit distribution function to handle
 * special requirementsfor each endpoint.  A default credit distribution
 * routine can be used by setting CreditInitFunc to NULL. The default
 * credit distribution is only provided for simple "fair" credit distribution
 * without regard to any prioritization.
 * Return: None
 */
void htc_set_credit_distribution(HTC_HANDLE HTCHandle,
				 void *pCreditDistContext,
				 HTC_CREDIT_DIST_CALLBACK CreditDistFunc,
				 HTC_CREDIT_INIT_CALLBACK CreditInitFunc,
				 HTC_SERVICE_ID ServicePriorityOrder[],
				 int ListLength);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 * Wait for the target to indicate the HTC layer is ready
 * htc_wait_target
 * @HTCHandle - HTC handle
 *
 * This API blocks until the target responds with an HTC ready message.
 * The caller should not connect services until the target has indicated it is
 * ready.
 * Return: None
 */
QDF_STATUS htc_wait_target(HTC_HANDLE HTCHandle);

/**
 * htc_start - Start target service communications
 * @HTCHandle - HTC handle
 *
 * This API indicates to the target that the service connection phase
 * is completeand the target can freely start all connected services.  This
 * API should only be called AFTER all service connections have been made.
 * TCStart will issue a SETUP_COMPLETE message to the target to indicate that
 * all service connections have been made and the target can start
 * communicating over the endpoints.
 * Return: None
 */
QDF_STATUS htc_start(HTC_HANDLE HTCHandle);

/**
 * htc_connect_service - Connect to an HTC service
 * @HTCHandle - HTC handle
 * @pReq - connection details
 * @pResp - connection response
 *
 * Service connections must be performed before htc_start.
 * User provides callback handlersfor various endpoint events.
 * Return: None
 */
QDF_STATUS htc_connect_service(HTC_HANDLE HTCHandle,
			     struct htc_service_connect_req *pReq,
			     struct htc_service_connect_resp *pResp);

/**
 * htc_dump - HTC register log dump
 * @HTCHandle - HTC handle
 * @CmdId - Log command
 * @start - start/print logs
 *
 * Register logs will be started/printed/ be flushed.
 * Return: None
 */
void htc_dump(HTC_HANDLE HTCHandle, uint8_t CmdId, bool start);

/**
 * htc_ce_taklet_debug_dump - Dump ce tasklet rings debug data
 * @HTCHandle - HTC handle
 *
 * Debug logs will be printed.
 * Return: None
 */
void htc_ce_tasklet_debug_dump(HTC_HANDLE htc_handle);

/**
 * htc_send_pkt - Send an HTC packet
 * @HTCHandle - HTC handle
 * @pPacket - packet to send
 *
 * Caller must initialize packet using SET_HTC_PACKET_INFO_TX() macro.
 * This interface is fully asynchronous.  On error, HTC SendPkt will
 * call the registered Endpoint callback to cleanup the packet.
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS htc_send_pkt(HTC_HANDLE HTCHandle, HTC_PACKET *pPacket);

/**
 * htc_send_data_pkt - Send an HTC packet containing a tx descriptor and data
 * @HTCHandle - HTC handle
 * @pPacket - packet to send
 *
 * Caller must initialize packet using SET_HTC_PACKET_INFO_TX() macro.
 * Caller must provide headroom in an initial fragment added to the
 * network buffer to store a HTC_FRAME_HDR.
 * This interface is fully asynchronous.  On error, htc_send_data_pkt will
 * call the registered Endpoint EpDataTxComplete callback to cleanup
 * the packet.
 * Return: A_OK
 */
#ifdef ATH_11AC_TXCOMPACT
QDF_STATUS htc_send_data_pkt(HTC_HANDLE HTCHandle, qdf_nbuf_t netbuf,
			   int Epid, int ActualLength);
#else                           /*ATH_11AC_TXCOMPACT */
QDF_STATUS htc_send_data_pkt(HTC_HANDLE HTCHandle, HTC_PACKET *pPacket,
			   uint8_t more_data);
#endif /*ATH_11AC_TXCOMPACT */

/**
 * htc_flush_surprise_remove - Flush HTC when target is removed surprisely
 *                             service communications
 * @HTCHandle - HTC handle
 *
 * All receive and pending TX packets will be flushed.
 * Return: None
 */
void htc_flush_surprise_remove(HTC_HANDLE HTCHandle);

/**
 * htc_stop - Stop HTC service communications
 * @HTCHandle - HTC handle
 *
 * HTC communications is halted.  All receive and pending TX packets
 * will  be flushed.
 * Return: None
 */
void htc_stop(HTC_HANDLE HTCHandle);

/**
 * htc_destroy - Destroy HTC service
 * @HTCHandle - HTC handle
 *
 * This cleans up all resources allocated by htc_create().
 * Return: None
 */
void htc_destroy(HTC_HANDLE HTCHandle);

/**
 * htc_flush_endpoint - Flush pending TX packets
 * @HTCHandle - HTC handle
 * @Endpoint - Endpoint to flush
 * @Tag - flush tag
 *
 * The Tag parameter is used to selectively flush packets with matching
 * tags. The value of 0 forces all packets to be flush regardless of tag
 * Return: None
 */
void htc_flush_endpoint(HTC_HANDLE HTCHandle, HTC_ENDPOINT_ID Endpoint,
			HTC_TX_TAG Tag);
/**
 * htc_dump_credit_states - Dump credit distribution state
 * @HTCHandle - HTC handle
 *
 * This dumps all credit distribution information to the debugger
 * Return: None
 */
void htc_dump_credit_states(HTC_HANDLE HTCHandle);

/**
 * htc_indicate_activity_change - Indicate a traffic activity change on an
 *                                endpoint
 * @HTCHandle - HTC handle
 * @Endpoint - endpoint in which activity has changed
 * @Active - true if active, false if it has become inactive
 *
 * This triggers the registered credit distribution function to
 * re-adjust credits for active/inactive endpoints.
 * Return: None
 */
void htc_indicate_activity_change(HTC_HANDLE HTCHandle,
				  HTC_ENDPOINT_ID Endpoint, bool Active);

/**
 * htc_get_endpoint_statistics - Get endpoint statistics
 * @HTCHandle - HTC handle
 * @Endpoint - Endpoint identifier
 * @Action - action to take with statistics
 * @pStats - statistics that were sampled (can be NULL if Action is
 *           HTC_EP_STAT_CLEAR)
 *
 * Statistics is a compile-time option and this function may return
 * false if HTC is not compiled with profiling.
 * The caller can specify the statistic "action" to take when sampling
 * the statistics.  This includes :
 * HTC_EP_STAT_SAMPLE : The pStats structure is filled with the current
 *                      values.
 * HTC_EP_STAT_SAMPLE_AND_CLEAR : The structure is filled and the current
 *                                statisticsare cleared.
 * HTC_EP_STAT_CLEA : the statistics are cleared, the called can pass
 *                    a NULL value for pStats
 * Return: true if statistics profiling is enabled, otherwise false.
 */
bool htc_get_endpoint_statistics(HTC_HANDLE HTCHandle,
				   HTC_ENDPOINT_ID Endpoint,
				   enum htc_endpoint_stat_action Action,
				   struct htc_endpoint_stats *pStats);

/**
 * htc_unblock_recv - Unblock HTC message reception
 * @HTCHandle - HTC handle
 *
 * HTC will block the receiver if the EpRecvAlloc callback fails to provide a
 * packet. The caller can use this API to indicate to HTC when resources
 * (buffers) are available such that the  receiver can be unblocked and HTC
 * may re-attempt fetching the pending message.
 * This API is not required if the user uses the EpRecvRefill callback or uses
 * the HTCAddReceivePacket()API to recycle or provide receive packets to HTC.
 * Return: None
 */
void htc_unblock_recv(HTC_HANDLE HTCHandle);

/**
 * htc_add_receive_pkt_multiple - Add multiple receive packets to HTC
 * @HTCHandle - HTC handle
 * @pPktQueue - HTC receive packet queue holding packets to add
 *
 * User must supply HTC packets for capturing incoming HTC frames.
 * The caller mmust initialize each HTC packet using the
 * SET_HTC_PACKET_INFO_RX_REFILL() macro. The queue must only contain
 * recv packets for the same endpoint. Caller supplies a pointer to an
 * HTC_PACKET_QUEUE structure holding the recv packet. This API will
 * remove the packets from the pkt queue and place them into internal
 * recv packet list.
 * The caller may allocate the pkt queue on the stack to hold the pkts.
 * Return: A_OK on success
 */
A_STATUS htc_add_receive_pkt_multiple(HTC_HANDLE HTCHandle,
				      HTC_PACKET_QUEUE *pPktQueue);

/**
 * htc_is_endpoint_active - Check if an endpoint is marked active
 * @HTCHandle - HTC handle
 * @Endpoint - endpoint to check for active state
 *
 * Return: returns true if Endpoint is Active
 */
bool htc_is_endpoint_active(HTC_HANDLE HTCHandle,
			      HTC_ENDPOINT_ID Endpoint);

/**
 * htc_set_pkt_dbg - Set up debug flag for HTC packets
 * @HTCHandle - HTC handle
 * @dbg_flag - enable or disable flag
 *
 * Return: none
 */
void htc_set_pkt_dbg(HTC_HANDLE handle, A_BOOL dbg_flag);

/**
 * htc_set_nodrop_pkt - Set up nodrop pkt flag for mboxping nodrop pkt
 * @HTCHandle - HTC handle
 * @isNodropPkt - indicates whether it is nodrop pkt
 *
 * Return: None
 *
 */
void htc_set_nodrop_pkt(HTC_HANDLE HTCHandle, A_BOOL isNodropPkt);

/**
 * htc_enable_hdr_length_check - Set up htc_hdr_length_check flag
 * @HTCHandle - HTC handle
 * @htc_hdr_length_check - flag to indicate whether htc header length check is
 *                         required
 *
 * Return: None
 *
 */
void
htc_enable_hdr_length_check(HTC_HANDLE htc_handle, bool htc_hdr_length_check);

/**
 * htc_get_num_recv_buffers - Get the number of recv buffers currently queued
 *                            into an HTC endpoint
 * @HTCHandle - HTC handle
 * @Endpoint - endpoint to check
 *
 * Return: returns number of buffers in queue
 *
 */
int htc_get_num_recv_buffers(HTC_HANDLE HTCHandle,
			     HTC_ENDPOINT_ID Endpoint);

/**
 * htc_set_target_failure_callback - Set the target failure handling callback
 *                                   in HTC layer
 * @HTCHandle - HTC handle
 * @Callback - target failure handling callback
 *
 * Return: None
 */
void htc_set_target_failure_callback(HTC_HANDLE HTCHandle,
				     HTC_TARGET_FAILURE Callback);

/* internally used functions for testing... */
void htc_enable_recv(HTC_HANDLE HTCHandle);
void htc_disable_recv(HTC_HANDLE HTCHandle);
A_STATUS HTCWaitForPendingRecv(HTC_HANDLE HTCHandle,
			       uint32_t TimeoutInMs,
			       bool *pbIsRecvPending);

/* function to fetch stats from htc layer*/
struct ol_ath_htc_stats *ieee80211_ioctl_get_htc_stats(HTC_HANDLE
						       HTCHandle);
/**
 * htc_get_tx_queue_depth() - get the tx queue depth of an htc endpoint
 * @htc_handle: htc handle
 * @enpoint_id: endpoint to check
 *
 * Return: htc_handle tx queue depth
 */
int htc_get_tx_queue_depth(HTC_HANDLE htc_handle, HTC_ENDPOINT_ID endpoint_id);

#ifdef WLAN_FEATURE_FASTPATH
void htc_ctrl_msg_cmpl(HTC_HANDLE htc_pdev, HTC_ENDPOINT_ID htc_ep_id);

#define HTC_TX_DESC_FILL(_htc_tx_desc, _download_len, _ep_id, _seq_no)	\
do {                                                            \
	HTC_WRITE32((_htc_tx_desc),                             \
		SM((_download_len), HTC_FRAME_HDR_PAYLOADLEN) | \
		SM((_ep_id), HTC_FRAME_HDR_ENDPOINTID));        \
	HTC_WRITE32((uint32_t *)(_htc_tx_desc) + 1,             \
		SM((_seq_no), HTC_FRAME_HDR_CONTROLBYTES1));    \
} while (0)
#endif /* WLAN_FEATURE_FASTPATH */

#ifdef __cplusplus
}
#endif
void htc_get_control_endpoint_tx_host_credits(HTC_HANDLE HTCHandle,
						int *credit);
void htc_dump_counter_info(HTC_HANDLE HTCHandle);
void *htc_get_targetdef(HTC_HANDLE htc_handle);
#ifdef FEATURE_RUNTIME_PM
int htc_runtime_suspend(HTC_HANDLE htc_ctx);
int htc_runtime_resume(HTC_HANDLE htc_ctx);
#endif
void htc_global_credit_flow_disable(void);
void htc_global_credit_flow_enable(void);

/* Disable ASPM : Disable PCIe low power */
bool htc_can_suspend_link(HTC_HANDLE HTCHandle);
void htc_vote_link_down(HTC_HANDLE HTCHandle);
void htc_vote_link_up(HTC_HANDLE HTCHandle);
#ifdef IPA_OFFLOAD
void htc_ipa_get_ce_resource(HTC_HANDLE htc_handle,
			     qdf_shared_mem_t **ce_sr,
			     uint32_t *ce_sr_ring_size,
			     qdf_dma_addr_t *ce_reg_paddr);
#else
#define htc_ipa_get_ce_resource(htc_handle,                \
			ce_sr, ce_sr_ring_size, ce_reg_paddr)     /* NO-OP */
#endif /* IPA_OFFLOAD */

#if defined(DEBUG_HL_LOGGING) && defined(CONFIG_HL_SUPPORT)

/**
 * htc_dump_bundle_stats() - dump tx and rx htc message bundle stats
 * @HTCHandle: htc handle
 *
 * Return: None
 */
void htc_dump_bundle_stats(HTC_HANDLE HTCHandle);

/**
 * htc_clear_bundle_stats() - clear tx and rx htc message bundle stats
 * @HTCHandle: htc handle
 *
 * Return: None
 */
void htc_clear_bundle_stats(HTC_HANDLE HTCHandle);
#endif

#ifdef FEATURE_RUNTIME_PM
int htc_pm_runtime_get(HTC_HANDLE htc_handle);
int htc_pm_runtime_put(HTC_HANDLE htc_handle);

/**
 * htc_dec_return_runtime_cnt: Decrement htc runtime count
 * @htc: HTC handle
 *
 * Return: value of runtime count after decrement
 */
int32_t htc_dec_return_runtime_cnt(HTC_HANDLE htc);
#else
static inline int htc_pm_runtime_get(HTC_HANDLE htc_handle) { return 0; }
static inline int htc_pm_runtime_put(HTC_HANDLE htc_handle) { return 0; }

static inline
int32_t htc_dec_return_runtime_cnt(HTC_HANDLE htc)
{
	return -1;
}
#endif

/**
  * htc_set_async_ep() - set async HTC end point
  *           user should call this function after htc_connect_service before
  *           queing any packets to end point
  * @HTCHandle: htc handle
  * @HTC_ENDPOINT_ID: end point id
  * @value: true or false
  *
  * Return: None
  */

void htc_set_async_ep(HTC_HANDLE HTCHandle,
			HTC_ENDPOINT_ID htc_ep_id, bool value);

/**
 * htc_set_wmi_endpoint_count: Set number of WMI endpoint
 * @htc_handle: HTC handle
 * @wmi_ep_count: WMI enpoint count
 *
 * return: None
 */
void htc_set_wmi_endpoint_count(HTC_HANDLE htc_handle, uint8_t wmi_ep_count);

/**
 * htc_get_wmi_endpoint_count: Get number of WMI endpoint
 * @htc_handle: HTC handle
 *
 * return: WMI enpoint count
 */
uint8_t  htc_get_wmi_endpoint_count(HTC_HANDLE htc_handle);

/**
 * htc_print_credit_history: print HTC credit history in buffer
 * @htc:        HTC handle
 * @count:      Number of lines to be copied
 * @print:      Print callback to print in the buffer
 * @print_priv: any data required by the print method, e.g. a file handle
 *
 * return: None
 */
#ifdef FEATURE_HTC_CREDIT_HISTORY
void htc_print_credit_history(HTC_HANDLE htc, uint32_t count,
			      qdf_abstract_print * print, void *print_priv);
#else
static inline
void htc_print_credit_history(HTC_HANDLE htc, uint32_t count,
			      qdf_abstract_print *print, void *print_priv)
{
	print(print_priv, "HTC Credit History Feature is disabled");
}
#endif
#endif /* _HTC_API_H_ */
