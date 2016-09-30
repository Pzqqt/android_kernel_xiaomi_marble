/*
 * Copyright (c) 2013-2016 The Linux Foundation. All rights reserved.
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

#ifndef _HTC_INTERNAL_H_
#define _HTC_INTERNAL_H_

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "htc_api.h"
#include "htc_packet.h"
#include <hif.h>
#include <htc.h>
#include <qdf_atomic.h>
#include <qdf_event.h>
#include <qdf_lock.h>
#include <qdf_nbuf.h>
#include <qdf_timer.h>
#include <qdf_types.h>

/* HTC operational parameters */
#define HTC_TARGET_RESPONSE_TIMEOUT         2000        /* in ms */
#define HTC_TARGET_DEBUG_INTR_MASK          0x01
#define HTC_TARGET_CREDIT_INTR_MASK         0xF0
#define HTC_MIN_MSG_PER_BUNDLE              2
#if defined(HIF_USB)
#define HTC_MAX_MSG_PER_BUNDLE_RX           11
#define HTC_MAX_MSG_PER_BUNDLE_TX           8
#else
#define HTC_MAX_MSG_PER_BUNDLE_RX           64
#define HTC_MAX_MSG_PER_BUNDLE              16
#define HTC_MAX_MSG_PER_BUNDLE_TX           32
#endif

/*
 * HTC_MAX_TX_BUNDLE_SEND_LIMIT -
 * This value is in units of tx frame fragments.
 * It needs to be at least as large as the maximum number of tx frames in a
 * HTC download bundle times the average number of fragments in each such frame
 * (In certain operating systems, such as Linux, we expect to only have
 * a single fragment per frame anyway.)
 */
#define HTC_MAX_TX_BUNDLE_SEND_LIMIT        255

#define HTC_PACKET_CONTAINER_ALLOCATION     32
#define NUM_CONTROL_TX_BUFFERS              2
#define HTC_CONTROL_BUFFER_SIZE             (HTC_MAX_CONTROL_MESSAGE_LENGTH + HTC_HDR_LENGTH)
#define HTC_CONTROL_BUFFER_ALIGN            32
#define HTC_TARGET_RESPONSE_POLL_MS         10
#if !defined(A_SIMOS_DEVHOST)
#define HTC_TARGET_MAX_RESPONSE_POLL        200 /* actual HW */
#else
#define HTC_TARGET_MAX_RESPONSE_POLL        600 /* host + target simulation */
#endif

#define HTC_SERVICE_TX_PACKET_TAG  HTC_TX_PACKET_TAG_INTERNAL

#define HTC_CREDIT_HISTORY_MAX              1024

#define HTC_IS_EPPING_ENABLED(_x)           ((_x) == QDF_GLOBAL_EPPING_MODE)

typedef enum {
	HTC_REQUEST_CREDIT,
	HTC_PROCESS_CREDIT_REPORT,
	HTC_SUSPEND_ACK,
	HTC_SUSPEND_NACK,
	HTC_INITIAL_WAKE_UP,
} htc_credit_exchange_type;

typedef struct {
	htc_credit_exchange_type type;
	uint64_t time;
	uint32_t tx_credit;
	uint32_t htc_tx_queue_depth;
} HTC_CREDIT_HISTORY;

typedef struct _HTC_ENDPOINT {
	HTC_ENDPOINT_ID Id;

	/* service ID this endpoint is bound to
	 * non-zero value means this endpoint is in use
	 */
	HTC_SERVICE_ID service_id;

	HTC_EP_CALLBACKS EpCallBacks;           /* callbacks associated with this endpoint */
	HTC_PACKET_QUEUE TxQueue;               /* HTC frame buffer TX queue */
	int MaxTxQueueDepth;            /* max depth of the TX queue before we need to
					call driver's full handler */
	int MaxMsgLength;               /* max length of endpoint message */
	uint8_t UL_PipeID;
	uint8_t DL_PipeID;
	int ul_is_polled;               /* Need to call HIF to get tx completion callbacks? */
	qdf_timer_t ul_poll_timer;
	int ul_poll_timer_active;
	int ul_outstanding_cnt;
	int dl_is_polled;               /* Need to call HIF to fetch rx?  (Not currently supported.) */
#if 0                           /* not currently supported */
	qdf_timer_t dl_poll_timer;
#endif

	HTC_PACKET_QUEUE TxLookupQueue;         /* lookup queue to match netbufs to htc packets */
	HTC_PACKET_QUEUE RxBufferHoldQueue;             /* temporary hold queue for back compatibility */
	uint8_t SeqNo;          /* TX seq no (helpful) for debugging */
	qdf_atomic_t TxProcessCount;            /* serialization */
	struct _HTC_TARGET *target;
	int TxCredits;          /* TX credits available on this endpoint */
	int TxCreditSize;               /* size in bytes of each credit (set by HTC) */
	int TxCreditsPerMaxMsg;         /* credits required per max message (precalculated) */
#ifdef HTC_EP_STAT_PROFILING
	HTC_ENDPOINT_STATS endpoint_stats;     /* endpoint statistics */
#endif
	bool TxCreditFlowEnabled;
} HTC_ENDPOINT;

#ifdef HTC_EP_STAT_PROFILING
#define INC_HTC_EP_STAT(p, stat, count) ((p)->endpoint_stats.stat += (count))
#else
#define INC_HTC_EP_STAT(p, stat, count)
#endif

typedef struct {
	uint16_t service_id;
	uint8_t CreditAllocation;
} HTC_SERVICE_TX_CREDIT_ALLOCATION;

#define HTC_MAX_SERVICE_ALLOC_ENTRIES 8

/* Error codes for HTC layer packet stats*/
enum ol_ath_htc_pkt_ecodes {
	GET_HTC_PKT_Q_FAIL = 0,         /* error- get packet at head of HTC_PACKET_Q */
	HTC_PKT_Q_EMPTY,
	HTC_SEND_Q_EMPTY
};
/* our HTC target state */
typedef struct _HTC_TARGET {
	struct hif_opaque_softc *hif_dev;
	HTC_ENDPOINT endpoint[ENDPOINT_MAX];
	qdf_spinlock_t HTCLock;
	qdf_spinlock_t HTCRxLock;
	qdf_spinlock_t HTCTxLock;
	qdf_spinlock_t HTCCreditLock;
	uint32_t HTCStateFlags;
	void *host_handle;
	HTC_INIT_INFO HTCInitInfo;
	HTC_PACKET *pHTCPacketStructPool;               /* pool of HTC packets */
	HTC_PACKET_QUEUE ControlBufferTXFreeList;
	uint8_t CtrlResponseBuffer[HTC_MAX_CONTROL_MESSAGE_LENGTH];
	int CtrlResponseLength;
	qdf_event_t ctrl_response_valid;
	bool CtrlResponseProcessing;
	int TotalTransmitCredits;
	HTC_SERVICE_TX_CREDIT_ALLOCATION
		ServiceTxAllocTable[HTC_MAX_SERVICE_ALLOC_ENTRIES];
	int TargetCreditSize;
#ifdef RX_SG_SUPPORT
	qdf_nbuf_queue_t RxSgQueue;
	bool IsRxSgInprogress;
	uint32_t CurRxSgTotalLen;               /* current total length */
	uint32_t ExpRxSgTotalLen;               /* expected total length */
#endif
	qdf_device_t osdev;
	struct ol_ath_htc_stats htc_pkt_stats;
	HTC_PACKET *pBundleFreeList;
	uint32_t ce_send_cnt;
	uint32_t TX_comp_cnt;
	uint8_t MaxMsgsPerHTCBundle;
	qdf_work_t queue_kicker;

#ifdef HIF_SDIO
	A_UINT16 AltDataCreditSize;
#endif
	A_UINT32 avail_tx_credits;
#if defined(DEBUG_HL_LOGGING) && defined(CONFIG_HL_SUPPORT)
	A_UINT32 rx_bundle_stats[HTC_MAX_MSG_PER_BUNDLE_RX];
	A_UINT32 tx_bundle_stats[HTC_MAX_MSG_PER_BUNDLE_TX];
#endif

	uint32_t con_mode;

	/*
	 * This flag is from the mboxping tool. It indicates that we cannot
	 * drop it. Besides, nodrop pkts have higher priority than normal pkts.
	 */
	A_BOOL is_nodrop_pkt;
} HTC_TARGET;

#if defined ENABLE_BUNDLE_TX
#define HTC_TX_BUNDLE_ENABLED(target) (target->MaxMsgsPerHTCBundle > 1)
#else
#define HTC_TX_BUNDLE_ENABLED(target) 0
#endif

#if defined ENABLE_BUNDLE_RX
#define HTC_RX_BUNDLE_ENABLED(target) (target->MaxMsgsPerHTCBundle > 1)
#else
#define HTC_RX_BUNDLE_ENABLED(target) 0
#endif

#define HTC_ENABLE_BUNDLE(target) (target->MaxMsgsPerHTCBundle > 1)

#ifdef RX_SG_SUPPORT
#define RESET_RX_SG_CONFIG(_target) \
	_target->ExpRxSgTotalLen = 0; \
	_target->CurRxSgTotalLen = 0; \
	_target->IsRxSgInprogress = false;
#endif

#define HTC_STATE_STOPPING      (1 << 0)
#define HTC_STOPPING(t)         ((t)->HTCStateFlags & HTC_STATE_STOPPING)
#define LOCK_HTC(t)             qdf_spin_lock_bh(&(t)->HTCLock);
#define UNLOCK_HTC(t)           qdf_spin_unlock_bh(&(t)->HTCLock);
#define LOCK_HTC_RX(t)          qdf_spin_lock_bh(&(t)->HTCRxLock);
#define UNLOCK_HTC_RX(t)        qdf_spin_unlock_bh(&(t)->HTCRxLock);
#define LOCK_HTC_TX(t)          qdf_spin_lock_bh(&(t)->HTCTxLock);
#define UNLOCK_HTC_TX(t)        qdf_spin_unlock_bh(&(t)->HTCTxLock);
#define LOCK_HTC_CREDIT(t)      qdf_spin_lock_bh(&(t)->HTCCreditLock);
#define UNLOCK_HTC_CREDIT(t)    qdf_spin_unlock_bh(&(t)->HTCCreditLock);

#define GET_HTC_TARGET_FROM_HANDLE(hnd) ((HTC_TARGET *)(hnd))

#define IS_TX_CREDIT_FLOW_ENABLED(ep)  ((ep)->TxCreditFlowEnabled)

#define HTC_POLL_CLEANUP_PERIOD_MS 10   /* milliseconds */

/* Macro to Increment the  HTC_PACKET_ERRORS for Tx.*/
#define OL_ATH_HTC_PKT_ERROR_COUNT_INCR(_target, _ecode)	\
	do { \
		if (_ecode == GET_HTC_PKT_Q_FAIL) \
		(_target->htc_pkt_stats.htc_get_pkt_q_fail_count) += 1 \
		; \
		if (_ecode == HTC_PKT_Q_EMPTY) \
		(_target->htc_pkt_stats.htc_pkt_q_empty_count) += 1 \
		; \
		if (_ecode == HTC_SEND_Q_EMPTY) \
		(_target->htc_pkt_stats.htc_send_q_empty_count) += 1 \
		; \
	} while (0);
/* internal HTC functions */

QDF_STATUS htc_rx_completion_handler(void *Context, qdf_nbuf_t netbuf,
				   uint8_t pipeID);
QDF_STATUS htc_tx_completion_handler(void *Context, qdf_nbuf_t netbuf,
				   unsigned int transferID, uint32_t toeplitz_hash_result);

HTC_PACKET *allocate_htc_bundle_packet(HTC_TARGET *target);
void free_htc_bundle_packet(HTC_TARGET *target, HTC_PACKET *pPacket);

HTC_PACKET *allocate_htc_packet_container(HTC_TARGET *target);
void free_htc_packet_container(HTC_TARGET *target, HTC_PACKET *pPacket);
void htc_flush_rx_hold_queue(HTC_TARGET *target, HTC_ENDPOINT *pEndpoint);
void htc_flush_endpoint_tx(HTC_TARGET *target, HTC_ENDPOINT *pEndpoint,
			   HTC_TX_TAG Tag);
void htc_recv_init(HTC_TARGET *target);
A_STATUS htc_wait_recv_ctrl_message(HTC_TARGET *target);
void htc_free_control_tx_packet(HTC_TARGET *target, HTC_PACKET *pPacket);
HTC_PACKET *htc_alloc_control_tx_packet(HTC_TARGET *target);
uint8_t htc_get_credit_allocation(HTC_TARGET *target, uint16_t service_id);
void htc_tx_resource_avail_handler(void *context, uint8_t pipeID);
void htc_control_rx_complete(void *Context, HTC_PACKET *pPacket);
void htc_process_credit_rpt(HTC_TARGET *target,
			    HTC_CREDIT_REPORT *pRpt,
			    int NumEntries, HTC_ENDPOINT_ID FromEndpoint);
void htc_fw_event_handler(void *context, QDF_STATUS status);
void htc_send_complete_check_cleanup(void *context);
#ifdef FEATURE_RUNTIME_PM
void htc_kick_queues(void *context);
#endif

void htc_credit_record(htc_credit_exchange_type type, uint32_t tx_credit,
		       uint32_t htc_tx_queue_depth);

static inline void htc_send_complete_poll_timer_stop(HTC_ENDPOINT *
						     pEndpoint) {
	LOCK_HTC_TX(pEndpoint->target);
	if (pEndpoint->ul_poll_timer_active) {
		/* qdf_timer_stop(&pEndpoint->ul_poll_timer); */
		pEndpoint->ul_poll_timer_active = 0;
	}
	UNLOCK_HTC_TX(pEndpoint->target);
}

static inline void htc_send_complete_poll_timer_start(HTC_ENDPOINT *
						      pEndpoint) {
	LOCK_HTC_TX(pEndpoint->target);
	if (pEndpoint->ul_outstanding_cnt
	    && !pEndpoint->ul_poll_timer_active) {
		/*
		   qdf_timer_start(
		   &pEndpoint->ul_poll_timer, HTC_POLL_CLEANUP_PERIOD_MS);
		 */
		pEndpoint->ul_poll_timer_active = 1;
	}
	UNLOCK_HTC_TX(pEndpoint->target);
}

static inline void
htc_send_complete_check(HTC_ENDPOINT *pEndpoint, int force) {
	/*
	 * Stop the polling-cleanup timer that will result in a later call to
	 * this function.  It may get started again below, if there are still
	 * outsending sends.
	 */
	htc_send_complete_poll_timer_stop(pEndpoint);
	/*
	 * Check whether HIF has any prior sends that have finished,
	 * have not had the post-processing done.
	 */
	hif_send_complete_check(pEndpoint->target->hif_dev,
				pEndpoint->UL_PipeID, force);
	/*
	 * If there are still outstanding sends after polling, start a timer
	 * to check again a little later.
	 */
	htc_send_complete_poll_timer_start(pEndpoint);
}

#ifdef __cplusplus
}
#endif

#ifndef DEBUG_BUNDLE
#define DEBUG_BUNDLE 0
#endif

#if defined(HIF_SDIO) || defined(HIF_USB)
#ifndef ENABLE_BUNDLE_TX
#define ENABLE_BUNDLE_TX 1
#endif

#ifndef ENABLE_BUNDLE_RX
#define ENABLE_BUNDLE_RX 1
#endif
#endif /*defined(HIF_SDIO) || defined(HIF_USB)*/
#endif /* !_HTC_HOST_INTERNAL_H_ */
