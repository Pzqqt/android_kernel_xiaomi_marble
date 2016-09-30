/*
 * Copyright (c) 2013-2014, 2016 The Linux Foundation. All rights reserved.
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

typedef void *HTC_HANDLE;

typedef uint16_t HTC_SERVICE_ID;

typedef void (*HTC_TARGET_FAILURE)(void *Instance, QDF_STATUS Status);

typedef struct _HTC_INIT_INFO {
	void *pContext;         /* context for target notifications */
	void (*TargetFailure)(void *Instance, QDF_STATUS Status);
	void (*TargetSendSuspendComplete)(void *ctx, bool is_nack);
	void (*target_initial_wakeup_cb)(void);
} HTC_INIT_INFO;

/* Struct for HTC layer packet stats*/
struct ol_ath_htc_stats {
	int htc_get_pkt_q_fail_count;
	int htc_pkt_q_empty_count;
	int htc_send_q_empty_count;
};

/* To resume HTT Tx queue during runtime resume */
typedef void (*HTC_EP_RESUME_TX_QUEUE)(void *);

/* per service connection send completion */
typedef void (*HTC_EP_SEND_PKT_COMPLETE)(void *, HTC_PACKET *);
/* per service connection callback when a plurality of packets have been sent
 * The HTC_PACKET_QUEUE is a temporary queue object (e.g. freed on return from the callback)
 * to hold a list of completed send packets.
 * If the handler cannot fully traverse the packet queue before returning, it should
 * transfer the items of the queue into the caller's private queue using:
 *   HTC_PACKET_ENQUEUE() */
typedef void (*HTC_EP_SEND_PKT_COMP_MULTIPLE)(void *,
					      HTC_PACKET_QUEUE *);
/* per service connection pkt received */
typedef void (*HTC_EP_RECV_PKT)(void *, HTC_PACKET *);
/* per service connection callback when a plurality of packets are received
 * The HTC_PACKET_QUEUE is a temporary queue object (e.g. freed on return from the callback)
 * to hold a list of recv packets.
 * If the handler cannot fully traverse the packet queue before returning, it should
 * transfer the items of the queue into the caller's private queue using:
 *   HTC_PACKET_ENQUEUE() */
typedef void (*HTC_EP_RECV_PKT_MULTIPLE)(void *, HTC_PACKET_QUEUE *);

/* Optional per service connection receive buffer re-fill callback,
 * On some OSes (like Linux) packets are allocated from a global pool and indicated up
 * to the network stack.  The driver never gets the packets back from the OS.  For these OSes
 * a refill callback can be used to allocate and re-queue buffers into HTC.
 *
 * On other OSes, the network stack can call into the driver's OS-specifc "return_packet" handler and
 * the driver can re-queue these buffers into HTC. In this regard a refill callback is
 * unnecessary */
typedef void (*HTC_EP_RECV_REFILL)(void *, HTC_ENDPOINT_ID Endpoint);

/* Optional per service connection receive buffer allocation callback.
 * On some systems packet buffers are an extremely limited resource.  Rather than
 * queue largest-possible-sized buffers to HTC, some systems would rather
 * allocate a specific size as the packet is received.  The trade off is
 * slightly more processing (callback invoked for each RX packet)
 * for the benefit of committing fewer buffer resources into HTC.
 *
 * The callback is provided the length of the pending packet to fetch. This includes the
 * HTC header length plus the length of payload.  The callback can return a pointer to
 * the allocated HTC packet for immediate use.
 *
 * Alternatively a variant of this handler can be used to allocate large receive packets as needed.
 * For example an application can use the refill mechanism for normal packets and the recv-alloc mechanism to
 * handle the case where a large packet buffer is required.  This can significantly reduce the
 * amount of "committed" memory used to receive packets.
 *
 * */
typedef HTC_PACKET *(*HTC_EP_RECV_ALLOC)(void *,
					 HTC_ENDPOINT_ID Endpoint,
					 int Length);

typedef enum _HTC_SEND_FULL_ACTION {
	HTC_SEND_FULL_KEEP = 0,         /* packet that overflowed should be kept in the queue */
	HTC_SEND_FULL_DROP = 1,         /* packet that overflowed should be dropped */
} HTC_SEND_FULL_ACTION;

/* Optional per service connection callback when a send queue is full. This can occur if the
 * host continues queueing up TX packets faster than credits can arrive
 * To prevent the host (on some Oses like Linux) from continuously queueing packets
 * and consuming resources, this callback is provided so that that the host
 * can disable TX in the subsystem (i.e. network stack).
 * This callback is invoked for each packet that "overflows" the HTC queue. The callback can
 * determine whether the new packet that overflowed the queue can be kept (HTC_SEND_FULL_KEEP) or
 * dropped (HTC_SEND_FULL_DROP).  If a packet is dropped, the EpTxComplete handler will be called
 * and the packet's status field will be set to A_NO_RESOURCE.
 * Other OSes require a "per-packet" indication for each completed TX packet, this
 * closed loop mechanism will prevent the network stack from overunning the NIC
 * The packet to keep or drop is passed for inspection to the registered handler the handler
 * must ONLY inspect the packet, it may not free or reclaim the packet. */
typedef HTC_SEND_FULL_ACTION (*HTC_EP_SEND_QUEUE_FULL)(void *,
						       HTC_PACKET *
						       pPacket);

typedef struct _HTC_EP_CALLBACKS {
	void *pContext;         /* context for each callback */
	HTC_EP_SEND_PKT_COMPLETE EpTxComplete;          /* tx completion callback for connected endpoint */
	HTC_EP_RECV_PKT EpRecv;         /* receive callback for connected endpoint */
	HTC_EP_RECV_REFILL EpRecvRefill;                /* OPTIONAL receive re-fill callback for connected endpoint */
	HTC_EP_SEND_QUEUE_FULL EpSendFull;              /* OPTIONAL send full callback */
	HTC_EP_RECV_ALLOC EpRecvAlloc;          /* OPTIONAL recv allocation callback */
	HTC_EP_RECV_ALLOC EpRecvAllocThresh;            /* OPTIONAL recv allocation callback based on a threshold */
	HTC_EP_SEND_PKT_COMP_MULTIPLE EpTxCompleteMultiple;             /* OPTIONAL completion handler for multiple complete
									indications (EpTxComplete must be NULL) */
	HTC_EP_RECV_PKT_MULTIPLE EpRecvPktMultiple;             /* OPTIONAL completion handler for multiple
									recv packet indications (EpRecv must be NULL) */
	HTC_EP_RESUME_TX_QUEUE ep_resume_tx_queue;
	int RecvAllocThreshold;         /* if EpRecvAllocThresh is non-NULL, HTC will compare the
					threshold value to the current recv packet length and invoke
					the EpRecvAllocThresh callback to acquire a packet buffer */
	int RecvRefillWaterMark;                /* if a EpRecvRefill handler is provided, this value
						can be used to set a trigger refill callback
						when the recv queue drops below this value
						if set to 0, the refill is only called when packets
						are empty */
} HTC_EP_CALLBACKS;

/* service connection information */
typedef struct _HTC_SERVICE_CONNECT_REQ {
	HTC_SERVICE_ID service_id;               /* service ID to connect to */
	uint16_t ConnectionFlags;               /* connection flags, see htc protocol definition */
	uint8_t *pMetaData;             /* ptr to optional service-specific meta-data */
	uint8_t MetaDataLength;         /* optional meta data length */
	HTC_EP_CALLBACKS EpCallbacks;           /* endpoint callbacks */
	int MaxSendQueueDepth;          /* maximum depth of any send queue */
	uint32_t LocalConnectionFlags;          /* HTC flags for the host-side (local) connection */
	unsigned int MaxSendMsgSize;            /* override max message size in send direction */
} HTC_SERVICE_CONNECT_REQ;

#define HTC_LOCAL_CONN_FLAGS_ENABLE_SEND_BUNDLE_PADDING (1 << 0)        /* enable send bundle padding for this endpoint */

/* service connection response information */
typedef struct _HTC_SERVICE_CONNECT_RESP {
	uint8_t *pMetaData;             /* caller supplied buffer to optional meta-data */
	uint8_t BufferLength;           /* length of caller supplied buffer */
	uint8_t ActualLength;           /* actual length of meta data */
	HTC_ENDPOINT_ID Endpoint;               /* endpoint to communicate over */
	unsigned int MaxMsgLength;              /* max length of all messages over this endpoint */
	uint8_t ConnectRespCode;                /* connect response code from target */
} HTC_SERVICE_CONNECT_RESP;

/* endpoint distribution structure */
typedef struct _HTC_ENDPOINT_CREDIT_DIST {
	struct _HTC_ENDPOINT_CREDIT_DIST *pNext;
	struct _HTC_ENDPOINT_CREDIT_DIST *pPrev;
	HTC_SERVICE_ID service_id;               /* Service ID (set by HTC) */
	HTC_ENDPOINT_ID Endpoint;               /* endpoint for this distribution struct (set by HTC) */
	uint32_t DistFlags;             /* distribution flags, distribution function can
					set default activity using SET_EP_ACTIVE() macro */
	int TxCreditsNorm;              /* credits for normal operation, anything above this
					indicates the endpoint is over-subscribed, this field
					is only relevant to the credit distribution function */
	int TxCreditsMin;               /* floor for credit distribution, this field is
					only relevant to the credit distribution function */
	int TxCreditsAssigned;          /* number of credits assigned to this EP, this field
					is only relevant to the credit dist function */
	int TxCredits;          /* current credits available, this field is used by
				HTC to determine whether a message can be sent or
				must be queued */
	int TxCreditsToDist;            /* pending credits to distribute on this endpoint, this
					is set by HTC when credit reports arrive.
					The credit distribution functions sets this to zero
					when it distributes the credits */
	int TxCreditsSeek;              /* this is the number of credits that the current pending TX
					packet needs to transmit.  This is set by HTC when
					and endpoint needs credits in order to transmit */
	int TxCreditSize;               /* size in bytes of each credit (set by HTC) */
	int TxCreditsPerMaxMsg;         /* credits required for a maximum sized messages (set by HTC) */
	void *pHTCReserved;             /* reserved for HTC use */
	int TxQueueDepth;               /* current depth of TX queue , i.e. messages waiting for credits
					This field is valid only when HTC_CREDIT_DIST_ACTIVITY_CHANGE
					or HTC_CREDIT_DIST_SEND_COMPLETE is indicated on an endpoint
					that has non-zero credits to recover
					*/
} HTC_ENDPOINT_CREDIT_DIST;

#define HTC_EP_ACTIVE                            ((uint32_t) (1u << 31))

/* macro to check if an endpoint has gone active, useful for credit
 * distributions */
#define IS_EP_ACTIVE(epDist)  ((epDist)->DistFlags & HTC_EP_ACTIVE)
#define SET_EP_ACTIVE(epDist) (epDist)->DistFlags |= HTC_EP_ACTIVE

/* credit distibution code that is passed into the distrbution function,
 * there are mandatory and optional codes that must be handled */
typedef enum _HTC_CREDIT_DIST_REASON {
	HTC_CREDIT_DIST_SEND_COMPLETE = 0,              /* credits available as a result of completed
							send operations (MANDATORY) resulting in credit reports */
	HTC_CREDIT_DIST_ACTIVITY_CHANGE = 1,            /* a change in endpoint activity occured (OPTIONAL) */
	HTC_CREDIT_DIST_SEEK_CREDITS,           /* an endpoint needs to "seek" credits (OPTIONAL) */
	HTC_DUMP_CREDIT_STATE           /* for debugging, dump any state information that is kept by
					the distribution function */
} HTC_CREDIT_DIST_REASON;

typedef void (*HTC_CREDIT_DIST_CALLBACK)(void *Context,
					 HTC_ENDPOINT_CREDIT_DIST *
					 pEPList,
					 HTC_CREDIT_DIST_REASON
					 Reason);

typedef void (*HTC_CREDIT_INIT_CALLBACK)(void *Context,
					 HTC_ENDPOINT_CREDIT_DIST *
					 pEPList, int TotalCredits);

/* endpoint statistics action */
typedef enum _HTC_ENDPOINT_STAT_ACTION {
	HTC_EP_STAT_SAMPLE = 0,         /* only read statistics */
	HTC_EP_STAT_SAMPLE_AND_CLEAR = 1,               /* sample and immediately clear statistics */
	HTC_EP_STAT_CLEAR               /* clear only */
} HTC_ENDPOINT_STAT_ACTION;

/* endpoint statistics */
typedef struct _HTC_ENDPOINT_STATS {
	uint32_t TxPosted;              /* number of TX packets posted to the endpoint */
	uint32_t TxCreditLowIndications;                /* number of times the host set the credit-low flag in a send message on
							this endpoint */
	uint32_t TxIssued;              /* running count of total TX packets issued */
	uint32_t TxPacketsBundled;              /* running count of TX packets that were issued in bundles */
	uint32_t TxBundles;             /* running count of TX bundles that were issued */
	uint32_t TxDropped;             /* tx packets that were dropped */
	uint32_t TxCreditRpts;          /* running count of total credit reports received for this endpoint */
	uint32_t TxCreditRptsFromRx;            /* credit reports received from this endpoint's RX packets */
	uint32_t TxCreditRptsFromOther;         /* credit reports received from RX packets of other endpoints */
	uint32_t TxCreditRptsFromEp0;           /* credit reports received from endpoint 0 RX packets */
	uint32_t TxCreditsFromRx;               /* count of credits received via Rx packets on this endpoint */
	uint32_t TxCreditsFromOther;            /* count of credits received via another endpoint */
	uint32_t TxCreditsFromEp0;              /* count of credits received via another endpoint */
	uint32_t TxCreditsConsummed;            /* count of consummed credits */
	uint32_t TxCreditsReturned;             /* count of credits returned */
	uint32_t RxReceived;            /* count of RX packets received */
	uint32_t RxLookAheads;          /* count of lookahead records
					found in messages received on this endpoint */
	uint32_t RxPacketsBundled;              /* count of recv packets received in a bundle */
	uint32_t RxBundleLookAheads;            /* count of number of bundled lookaheads */
	uint32_t RxBundleIndFromHdr;            /* count of the number of bundle indications from the HTC header */
	uint32_t RxAllocThreshHit;              /* count of the number of times the recv allocation threshhold was hit */
	uint32_t RxAllocThreshBytes;            /* total number of bytes */
} HTC_ENDPOINT_STATS;

/* ------ Function Prototypes ------ */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Create an instance of HTC over the underlying HIF device
   @function name: htc_create
   @input: HifDevice - hif device handle,
	   pInfo - initialization information
	   osdev - QDF device structure
	   con_mode - driver connection mode
   @output:
   @return: HTC_HANDLE on success, NULL on failure
   @notes:
   @example:
   @see also: htc_destroy
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
HTC_HANDLE htc_create(void *HifDevice, HTC_INIT_INFO *pInfo, qdf_device_t osdev,
		      uint32_t con_mode);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Get the underlying HIF device handle
   @function name: htc_get_hif_device
   @input:  HTCHandle - handle passed into the AddInstance callback
   @output:
   @return: opaque HIF device handle usable in HIF API calls.
   @notes:
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void *htc_get_hif_device(HTC_HANDLE HTCHandle);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Set credit distribution parameters
   @function name: htc_set_credit_distribution
   @input:  HTCHandle - HTC handle
   pCreditDistCont - caller supplied context to pass into distribution functions
   CreditDistFunc - Distribution function callback
   CreditDistInit - Credit Distribution initialization callback
   ServicePriorityOrder - Array containing list of service IDs, lowest index
   is highestpriority
	ListLength - number of elements in ServicePriorityOrder
   @output:
   @return:
   @notes :  The user can set a custom credit distribution function to handle
   special requirementsfor each endpoint.  A default credit distribution
   routine can be used by setting CreditInitFunc to NULL. The default
   credit distribution is only provided for simple "fair" credit distribution
   without regard to any prioritization.

   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void htc_set_credit_distribution(HTC_HANDLE HTCHandle,
				 void *pCreditDistContext,
				 HTC_CREDIT_DIST_CALLBACK CreditDistFunc,
				 HTC_CREDIT_INIT_CALLBACK CreditInitFunc,
				 HTC_SERVICE_ID ServicePriorityOrder[],
				 int ListLength);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Wait for the target to indicate the HTC layer is ready
   @function name: htc_wait_target
   @input:  HTCHandle - HTC handle
   @output:
   @return:
   @notes:  This API blocks until the target responds with an HTC ready message.
   The caller should not connect services until the target has indicated it is
   ready.
   @example:
   @see also: htc_connect_service
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
A_STATUS htc_wait_target(HTC_HANDLE HTCHandle);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Start target service communications
   @function name: htc_start
   @input:  HTCHandle - HTC handle
   @output:
   @return:
   @notes: This API indicates to the target that the service connection phase
   is completeand the target can freely start all connected services.  This
   API should only be called AFTER all service connections have been made.
   TCStart will issue a SETUP_COMPLETE message to the target to indicate that
   all service connections have been made and the target can start
   communicating over the endpoints.
   @example:
   @see also: htc_connect_service
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
A_STATUS htc_start(HTC_HANDLE HTCHandle);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Add receive packet to HTC
   @function name: htc_add_receive_pkt
   @input:  HTCHandle - HTC handle
   pPacket - HTC receive packet to add
   @output:
   @return: A_OK on success
   @notes:  user must supply HTC packets for capturing incomming HTC frames.
   The caller must initialize each HTC packet using the
   SET_HTC_PACKET_INFO_RX_REFILL() macro.
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
A_STATUS htc_add_receive_pkt(HTC_HANDLE HTCHandle, HTC_PACKET *pPacket);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Connect to an HTC service
   @function name: htc_connect_service
   @input:  HTCHandle - HTC handle
   pReq - connection details
   @output: pResp - connection response
   @return:
   @notes:  Service connections must be performed before htc_start.
   User provides callback handlersfor various endpoint events.
   @example:
   @see also: htc_start
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
A_STATUS htc_connect_service(HTC_HANDLE HTCHandle,
			     HTC_SERVICE_CONNECT_REQ *pReq,
			     HTC_SERVICE_CONNECT_RESP *pResp);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: HTC register log dump
   @function name: htc_dump
   @input:  HTCHandle - HTC handle
   CmdId - Log command
   start - start/print logs
   @output:
   @return:
   @notes: Register logs will be started/printed.
   be flushed.
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

void htc_dump(HTC_HANDLE HTCHandle, uint8_t CmdId, bool start);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Send an HTC packet
   @function name: htc_send_pkt
   @input:  HTCHandle - HTC handle
   pPacket - packet to send
   @output:
   @return: A_OK
   @notes:  Caller must initialize packet using SET_HTC_PACKET_INFO_TX() macro.
   This interface is fully asynchronous.  On error, HTC SendPkt will
   call the registered Endpoint callback to cleanup the packet.
   @example:
   @see also: htc_flush_endpoint
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
A_STATUS htc_send_pkt(HTC_HANDLE HTCHandle, HTC_PACKET *pPacket);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Send an HTC packet containing a tx descriptor and data
   @function name: htc_send_data_pkt
   @input:  HTCHandle - HTC handle
   pPacket - packet to send
   @output:
   @return: A_OK
   @notes:  Caller must initialize packet using SET_HTC_PACKET_INFO_TX() macro.
   Caller must provide headroom in an initial fragment added to the
   network buffer to store a HTC_FRAME_HDR.
   This interface is fully asynchronous.  On error, htc_send_data_pkt will
   call the registered Endpoint EpDataTxComplete callback to cleanup
   the packet.
   @example:
   @see also: htc_send_pkt
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifdef ATH_11AC_TXCOMPACT
A_STATUS htc_send_data_pkt(HTC_HANDLE HTCHandle, qdf_nbuf_t netbuf,
			   int Epid, int ActualLength);
#else                           /*ATH_11AC_TXCOMPACT */
A_STATUS htc_send_data_pkt(HTC_HANDLE HTCHandle, HTC_PACKET *pPacket,
			   uint8_t more_data);
#endif /*ATH_11AC_TXCOMPACT */
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Flush HTC when target is removed surprisely service communications
   @function name: htc_flush_surprise_remove
   @input:  HTCHandle - HTC handle
   @output:
   @return:
   @notes: All receive and pending TX packets will
   be flushed.
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void htc_flush_surprise_remove(HTC_HANDLE HTCHandle);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Stop HTC service communications
   @function name: htc_stop
   @input:  HTCHandle - HTC handle
   @output:
   @return:
   @notes: HTC communications is halted.  All receive and pending TX packets will
   be flushed.
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void htc_stop(HTC_HANDLE HTCHandle);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Destory HTC service
   @function name: htc_destroy
   @input: HTCHandle
   @output:
   @return:
   @notes:  This cleans up all resources allocated by htc_create().
   @example:
   @see also: htc_create
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void htc_destroy(HTC_HANDLE HTCHandle);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Flush pending TX packets
   @function name: htc_flush_endpoint
   @input:  HTCHandle - HTC handle
   Endpoint - Endpoint to flush
   Tag - flush tag
   @output:
   @return:
   @notes:  The Tag parameter is used to selectively flush packets with matching tags.
   The value of 0 forces all packets to be flush regardless of tag.
   @example:
   @see also: htc_send_pkt
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void htc_flush_endpoint(HTC_HANDLE HTCHandle, HTC_ENDPOINT_ID Endpoint,
			HTC_TX_TAG Tag);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Dump credit distribution state
   @function name: htc_dump_credit_states
   @input:  HTCHandle - HTC handle
   @output:
   @return:
   @notes:  This dumps all credit distribution information to the debugger
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void htc_dump_credit_states(HTC_HANDLE HTCHandle);
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Indicate a traffic activity change on an endpoint
   @function name: htc_indicate_activity_change
   @input:  HTCHandle - HTC handle
   Endpoint - endpoint in which activity has changed
   Active - true if active, false if it has become inactive
   @output:
   @return:
   @notes:  This triggers the registered credit distribution function to
   re-adjust credits for active/inactive endpoints.
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void htc_indicate_activity_change(HTC_HANDLE HTCHandle,
				  HTC_ENDPOINT_ID Endpoint, bool Active);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Get endpoint statistics
   @function name: htc_get_endpoint_statistics
   @input:  HTCHandle - HTC handle
   Endpoint - Endpoint identifier
   Action - action to take with statistics
   @output:
   pStats - statistics that were sampled (can be NULL if Action is HTC_EP_STAT_CLEAR)

   @return: true if statistics profiling is enabled, otherwise false.

   @notes :  Statistics is a compile-time option and this function may return
	false if HTC is not compiled with profiling.

	The caller can specify the statistic "action" to take when sampling
	the statistics.  This includes :

	HTC_EP_STAT_SAMPLE : The pStats structure is filled with the current
	values.
	HTC_EP_STAT_SAMPLE_AND_CLEAR : The structure is filled and the current
	statisticsare cleared.

	HTC_EP_STAT_CLEA : the statistics are cleared, the called can pass a NULL
	value forpStats

   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
bool htc_get_endpoint_statistics(HTC_HANDLE HTCHandle,
				   HTC_ENDPOINT_ID Endpoint,
				   HTC_ENDPOINT_STAT_ACTION Action,
				   HTC_ENDPOINT_STATS *pStats);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Unblock HTC message reception
   @function name: htc_unblock_recv
   @input:  HTCHandle - HTC handle
   @output:
   @return:
   @notes:
   HTC will block the receiver if the EpRecvAlloc callback fails to provide a
   packet. The caller can use this API to indicate to HTC when resources
   (buffers) are available such that the  receiver can be unblocked and HTC
   may re-attempt fetching the pending message.

   This API is not required if the user uses the EpRecvRefill callback or uses
   the HTCAddReceivePacket()API to recycle or provide receive packets to HTC.

   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void htc_unblock_recv(HTC_HANDLE HTCHandle);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: send a series of HTC packets
   @function name: htc_send_pkts_multiple
   @input:  HTCHandle - HTC handle
	pPktQueue - local queue holding packets to send
   @output:
   @return: A_OK
   @notes:  Caller must initialize each packet using SET_HTC_PACKET_INFO_TX() macro.
	The queue must only contain packets directed at the same endpoint.
	Caller supplies a pointer to an HTC_PACKET_QUEUE structure holding the TX packets in FIFO order.
	This API will remove the packets from the pkt queue and place them into the HTC Tx Queue
	and bundle messages where possible.
	The caller may allocate the pkt queue on the stack to hold the packets.
	This interface is fully asynchronous.  On error, htc_send_pkts will
	call the registered Endpoint callback to cleanup the packet.
   @example:
   @see also: htc_flush_endpoint
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
A_STATUS htc_send_pkts_multiple(HTC_HANDLE HTCHandle,
				HTC_PACKET_QUEUE *pPktQueue);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Add multiple receive packets to HTC
   @function name: htc_add_receive_pkt_multiple
   @input:  HTCHandle - HTC handle
	pPktQueue - HTC receive packet queue holding packets to add
   @output:
   @return: A_OK on success
   @notes:  user must supply HTC packets for capturing incomming HTC frames.  The caller
	must initialize each HTC packet using the SET_HTC_PACKET_INFO_RX_REFILL()
	macro. The queue must only contain recv packets for the same endpoint.
	Caller supplies a pointer to an HTC_PACKET_QUEUE structure holding the recv packet.
	This API will remove the packets from the pkt queue and place them into internal
	recv packet list.
	The caller may allocate the pkt queue on the stack to hold the packets.
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
A_STATUS htc_add_receive_pkt_multiple(HTC_HANDLE HTCHandle,
				      HTC_PACKET_QUEUE *pPktQueue);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Check if an endpoint is marked active
   @function name: htc_is_endpoint_active
   @input:  HTCHandle - HTC handle
	Endpoint - endpoint to check for active state
   @output:
   @return: returns true if Endpoint is Active
   @notes:
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
bool htc_is_endpoint_active(HTC_HANDLE HTCHandle,
			      HTC_ENDPOINT_ID Endpoint);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Set up nodrop pkt flag for mboxping nodrop pkt
   @function name: htc_set_nodrop_pkt
   @input:  HTCHandle - HTC handle
	isNodropPkt - indicates whether it is nodrop pkt
   @output:
   @return:
   @notes:
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
void htc_set_nodrop_pkt(HTC_HANDLE HTCHandle, A_BOOL isNodropPkt);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Get the number of recv buffers currently queued into an HTC endpoint
   @function name: htc_get_num_recv_buffers
   @input:  HTCHandle - HTC handle
	Endpoint - endpoint to check
   @output:
   @return: returns number of buffers in queue
   @notes:
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
int htc_get_num_recv_buffers(HTC_HANDLE HTCHandle,
			     HTC_ENDPOINT_ID Endpoint);

/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
   @desc: Set the target failure handling callback in HTC layer
   @function name: htc_set_target_failure_callback
   @input:  HTCHandle - HTC handle
	Callback - target failure handling callback
   @output:
   @return:
   @notes:
   @example:
   @see also:
 +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
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

#ifdef HIF_USB
#define HTCReturnReceivePkt(target, p, osbuf) \
	do { \
		A_NETBUF_FREE(osbuf);  \
		if (p->Status == A_CLONE) {  \
			qdf_mem_free(p);  \
		} \
	} while (0)
#else
#define HTCReturnReceivePkt(target, p, osbuf)   htc_add_receive_pkt(target, p)
#endif

#ifdef WLAN_FEATURE_FASTPATH
void htc_ctrl_msg_cmpl(HTC_HANDLE htc_pdev, HTC_ENDPOINT_ID htc_ep_id);

#define HTC_TX_DESC_FILL(_htc_tx_desc, _download_len, _ep_id, _seq_no)	\
do {									\
		HTC_WRITE32((_htc_tx_desc),				\
			SM((_download_len), HTC_FRAME_HDR_PAYLOADLEN) |	\
			SM((_ep_id), HTC_FRAME_HDR_ENDPOINTID));	\
									\
		HTC_WRITE32((uint32_t *)(_htc_tx_desc) + 1,		\
				SM((_seq_no), HTC_FRAME_HDR_CONTROLBYTES1));\
} while (0)
#endif /* WLAN_FEATURE_FASTPATH */

#ifdef __cplusplus
}
#endif
void htc_get_control_endpoint_tx_host_credits(HTC_HANDLE HTCHandle, int *credit);
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
			     qdf_dma_addr_t *ce_sr_base_paddr,
			     uint32_t *ce_sr_ring_size,
			     qdf_dma_addr_t *ce_reg_paddr);
#else
#define htc_ipa_get_ce_resource(htc_handle,                \
			ce_sr_base_paddr,                  \
			ce_sr_ring_size,                   \
			ce_reg_paddr)                      /* NO-OP */
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
#else
static inline int htc_pm_runtime_get(HTC_HANDLE htc_handle) { return 0; }
static inline int htc_pm_runtime_put(HTC_HANDLE htc_handle) { return 0; }
#endif
#endif /* _HTC_API_H_ */
