/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

#if !defined( __CDS_SCHED_H )
#define __CDS_SCHED_H

/**=========================================================================

   \file  cds_sched.h

   \brief Connectivity driver services scheduler

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <cdf_event.h>
#include "i_cdf_types.h"
#include <linux/wait.h>
#if defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#endif
#include <cds_mq.h>
#include <cdf_types.h>
#include "cdf_lock.h"

#define TX_POST_EVENT_MASK               0x001
#define TX_SUSPEND_EVENT_MASK            0x002
#define MC_POST_EVENT_MASK               0x001
#define MC_SUSPEND_EVENT_MASK            0x002
#define RX_POST_EVENT_MASK               0x001
#define RX_SUSPEND_EVENT_MASK            0x002
#define TX_SHUTDOWN_EVENT_MASK           0x010
#define MC_SHUTDOWN_EVENT_MASK           0x010
#define RX_SHUTDOWN_EVENT_MASK           0x010
#define WD_POST_EVENT_MASK               0x001
#define WD_SHUTDOWN_EVENT_MASK           0x002
#define WD_CHIP_RESET_EVENT_MASK         0x004
#define WD_WLAN_SHUTDOWN_EVENT_MASK      0x008
#define WD_WLAN_REINIT_EVENT_MASK        0x010

/*
 * Maximum number of messages in the system
 * These are buffers to account for all current messages
 * with some accounting of what we think is a
 * worst-case scenario.  Must be able to handle all
 * incoming frames, as well as overhead for internal
 * messaging
 *
 * Increased to 8000 to handle more RX frames
 */
#define CDS_CORE_MAX_MESSAGES 8000

#ifdef QCA_CONFIG_SMP
/*
** Maximum number of cds messages to be allocated for
** OL Rx thread.
*/
#define CDS_MAX_OL_RX_PKT 4000

typedef void (*cds_ol_rx_thread_cb)(void *context, void *rxpkt, uint16_t staid);
#endif

/*
** CDF Message queue definition.
*/
typedef struct _cds_mq_type {
	/* Lock use to synchronize access to this message queue */
	spinlock_t mqLock;

	/* List of vOS Messages waiting on this queue */
	struct list_head mqList;

} cds_mq_type, *p_cds_mq_type;

#ifdef QCA_CONFIG_SMP
/*
** CDS message wrapper for data rx from TXRX
*/
struct cds_ol_rx_pkt {
	struct list_head list;
	void *context;

	/* Rx skb */
	void *Rxpkt;

	/* Station id to which this packet is destined */
	uint16_t staId;

	/* Call back to further send this packet to txrx layer */
	cds_ol_rx_thread_cb callback;

};
#endif

/*
** CDS Scheduler context
** The scheduler context contains the following:
**   ** the messages queues
**   ** the handle to the tread
**   ** pointer to the events that gracefully shutdown the MC and Tx threads
**
*/
typedef struct _cds_sched_context {
	/* Place holder to the CDS Context */
	void *pVContext;
	/* WMA Message queue on the Main thread */
	cds_mq_type wmaMcMq;

	/* PE Message queue on the Main thread */
	cds_mq_type peMcMq;

	/* SME Message queue on the Main thread */
	cds_mq_type smeMcMq;

	/* SYS Message queue on the Main thread */
	cds_mq_type sysMcMq;

	/* Handle of Event for MC thread to signal startup */
	struct completion McStartEvent;

	struct task_struct *McThread;

	/* completion object for MC thread shutdown */
	struct completion McShutdown;

	/* Wait queue for MC thread */
	wait_queue_head_t mcWaitQueue;

	unsigned long mcEventFlag;

	/* Completion object to resume Mc thread */
	struct completion ResumeMcEvent;

	/* lock to make sure that McThread suspend/resume mechanism is in sync */
	spinlock_t McThreadLock;
#ifdef QCA_CONFIG_SMP
	spinlock_t ol_rx_thread_lock;

	/* OL Rx thread handle */
	struct task_struct *ol_rx_thread;

	/* Handle of Event for Rx thread to signal startup */
	struct completion ol_rx_start_event;

	/* Completion object to suspend OL rx thread */
	struct completion ol_suspend_rx_event;

	/* Completion objext to resume OL rx thread */
	struct completion ol_resume_rx_event;

	/* Completion object for OL Rxthread shutdown */
	struct completion ol_rx_shutdown;

	/* Waitq for OL Rx thread */
	wait_queue_head_t ol_rx_wait_queue;

	unsigned long ol_rx_event_flag;

	/* Rx buffer queue */
	struct list_head ol_rx_thread_queue;

	/* Spinlock to synchronize between tasklet and thread */
	spinlock_t ol_rx_queue_lock;

	/* Rx queue length */
	unsigned int ol_rx_queue_len;

	/* Lock to synchronize free buffer queue access */
	spinlock_t cds_ol_rx_pkt_freeq_lock;

	/* Free message queue for OL Rx processing */
	struct list_head cds_ol_rx_pkt_freeq;

	/* cpu hotplug notifier */
	struct notifier_block *cpu_hot_plug_notifier;
#endif
} cds_sched_context, *p_cds_sched_context;

/**
 * struct cds_log_complete - Log completion internal structure
 * @is_fatal: Type is fatal or not
 * @indicator: Source of bug report
 * @reason_code: Reason code for bug report
 * @is_report_in_progress: If bug report is in progress
 *
 * This structure internally stores the log related params
 */
struct cds_log_complete {
	uint32_t is_fatal;
	uint32_t indicator;
	uint32_t reason_code;
	bool is_report_in_progress;
};

/*
** CDS Sched Msg Wrapper
** Wrapper messages so that they can be chained to their respective queue
** in the scheduler.
*/
typedef struct _cds_msg_wrapper {
	/* Message node */
	struct list_head msgNode;

	/* the Vos message it is associated to */
	cds_msg_t *pVosMsg;

} cds_msg_wrapper, *p_cds_msg_wrapper;

typedef struct _cds_context_type {
	/* Messages buffers */
	cds_msg_t aMsgBuffers[CDS_CORE_MAX_MESSAGES];

	cds_msg_wrapper aMsgWrappers[CDS_CORE_MAX_MESSAGES];

	/* Free Message queue */
	cds_mq_type freeVosMq;

	/* Scheduler Context */
	cds_sched_context cdf_sched;

	/* HDD Module Context  */
	void *pHDDContext;

	/* MAC Module Context  */
	void *pMACContext;

#ifndef WLAN_FEATURE_MBSSID
	/* SAP Context */
	void *pSAPContext;
#endif

	cdf_event_t ProbeEvent;

	uint32_t driver_state;

	cdf_event_t wmaCompleteEvent;

	/* WMA Context */
	void *pWMAContext;

	void *pHIFContext;

	void *htc_ctx;

	void *epping_ctx;
	/*
	 * cdf_ctx will be used by cdf
	 * while allocating dma memory
	 * to access dev information.
	 */
	cdf_device_t cdf_ctx;

	void *pdev_txrx_ctx;

	/* Configuration handle used to get system configuration */
	void *cfg_ctx;

	bool is_wakelock_log_enabled;
	uint32_t wakelock_log_level;
	uint32_t connectivity_log_level;
	uint32_t packet_stats_log_level;
	uint32_t driver_debug_log_level;
	uint32_t fw_debug_log_level;
	struct cds_log_complete log_complete;
	cdf_spinlock_t bug_report_lock;
	cdf_event_t connection_update_done_evt;

} cds_context_type, *p_cds_contextType;

/*---------------------------------------------------------------------------
   Function declarations and documenation
   ---------------------------------------------------------------------------*/

#ifdef QCA_CONFIG_SMP
/*---------------------------------------------------------------------------
   \brief cds_drop_rxpkt_by_staid() - API to drop pending Rx packets for a sta
   The \a cds_drop_rxpkt_by_staid() drops queued packets for a station, to drop
   all the pending packets the caller has to send WLAN_MAX_STA_COUNT as staId.
   \param  pSchedContext - pointer to the global CDS Sched Context
   \param staId - Station Id

   \return Nothing
   \sa cds_drop_rxpkt_by_staid()
   -------------------------------------------------------------------------*/
void cds_drop_rxpkt_by_staid(p_cds_sched_context pSchedContext, uint16_t staId);

/*---------------------------------------------------------------------------
   \brief cds_indicate_rxpkt() - API to Indicate rx data packet
   The \a cds_indicate_rxpkt() enqueues the rx packet onto ol_rx_thread_queue
   and notifies cds_ol_rx_thread().
   \param  Arg - pointer to the global CDS Sched Context
   \param pkt - Vos data message buffer

   \return Nothing
   \sa cds_indicate_rxpkt()
   -------------------------------------------------------------------------*/
void cds_indicate_rxpkt(p_cds_sched_context pSchedContext,
			struct cds_ol_rx_pkt *pkt);

/*---------------------------------------------------------------------------
   \brief cds_alloc_ol_rx_pkt() - API to return next available cds message
   The \a cds_alloc_ol_rx_pkt() returns next available cds message buffer
   used for Rx Data processing.
   \param pSchedContext - pointer to the global CDS Sched Context

   \return pointer to cds message buffer
   \sa cds_alloc_ol_rx_pkt()
   -------------------------------------------------------------------------*/
struct cds_ol_rx_pkt *cds_alloc_ol_rx_pkt(p_cds_sched_context pSchedContext);

/*---------------------------------------------------------------------------
   \brief cds_free_ol_rx_pkt() - API to release cds message to the freeq
   The \a cds_free_ol_rx_pkt() returns the cds message used for Rx data
   to the free queue.
   \param  pSchedContext - pointer to the global CDS Sched Context
   \param  pkt - Vos message buffer to be returned to free queue.

   \return Nothing
   \sa cds_free_ol_rx_pkt()
   -------------------------------------------------------------------------*/
void cds_free_ol_rx_pkt(p_cds_sched_context pSchedContext,
			 struct cds_ol_rx_pkt *pkt);
/*---------------------------------------------------------------------------
   \brief cds_free_ol_rx_pkt_freeq() - Free cdss buffer free queue
   The \a cds_free_ol_rx_pkt_freeq() does mem free of the buffers
   available in free cds buffer queue which is used for Data rx processing
   from Tlshim.
   \param pSchedContext - pointer to the global CDS Sched Context

   \return Nothing
   \sa cds_free_ol_rx_pkt_freeq()
   -------------------------------------------------------------------------*/
void cds_free_ol_rx_pkt_freeq(p_cds_sched_context pSchedContext);
#endif

/*---------------------------------------------------------------------------

   \brief cds_sched_open() - initialize the CDS Scheduler

   The \a cds_sched_open() function initializes the CDS Scheduler
   Upon successful initialization:

     - All the message queues are initialized

     - The Main Controller thread is created and ready to receive and
       dispatch messages.

     - The Tx thread is created and ready to receive and dispatch messages

   \param  p_cds_context - pointer to the global CDF Context

   \param  p_cds_sched_context - pointer to a previously allocated buffer big
          enough to hold a scheduler context.
 \

   \return CDF_STATUS_SUCCESS - Scheduler was successfully initialized and
          is ready to be used.

          CDF_STATUS_E_RESOURCES - System resources (other than memory)
          are unavailable to initilize the scheduler

          CDF_STATUS_E_NOMEM - insufficient memory exists to initialize
          the scheduler

          CDF_STATUS_E_INVAL - Invalid parameter passed to the scheduler Open
          function

          CDF_STATUS_E_FAILURE - Failure to initialize the scheduler/

   \sa cds_sched_open()

   -------------------------------------------------------------------------*/
CDF_STATUS cds_sched_open(void *p_cds_context,
			  p_cds_sched_context pSchedCxt, uint32_t SchedCtxSize);

/*---------------------------------------------------------------------------

   \brief cds_sched_close() - Close the CDS Scheduler

   The \a cds_sched_closes() function closes the CDS Scheduler
   Upon successful closing:

     - All the message queues are flushed

     - The Main Controller thread is closed

     - The Tx thread is closed

   \param  p_cds_context - pointer to the global CDF Context

   \return CDF_STATUS_SUCCESS - Scheduler was successfully initialized and
          is ready to be used.

          CDF_STATUS_E_INVAL - Invalid parameter passed to the scheduler Open
          function

          CDF_STATUS_E_FAILURE - Failure to initialize the scheduler/

   \sa cds_sched_close()

   ---------------------------------------------------------------------------*/
CDF_STATUS cds_sched_close(void *p_cds_context);

/* Helper routines provided to other CDS API's */
CDF_STATUS cds_mq_init(p_cds_mq_type pMq);
void cds_mq_deinit(p_cds_mq_type pMq);
void cds_mq_put(p_cds_mq_type pMq, p_cds_msg_wrapper pMsgWrapper);
p_cds_msg_wrapper cds_mq_get(p_cds_mq_type pMq);
bool cds_is_mq_empty(p_cds_mq_type pMq);
p_cds_sched_context get_cds_sched_ctxt(void);
CDF_STATUS cds_sched_init_mqs(p_cds_sched_context pSchedContext);
void cds_sched_deinit_mqs(p_cds_sched_context pSchedContext);
void cds_sched_flush_mc_mqs(p_cds_sched_context pSchedContext);

void cdf_timer_module_init(void);
void cds_ssr_protect_init(void);
void cds_ssr_protect(const char *caller_func);
void cds_ssr_unprotect(const char *caller_func);
bool cds_is_ssr_ready(const char *caller_func);
int cds_get_gfp_flags(void);

#define cds_wait_for_work_thread_completion(func) cds_is_ssr_ready(func)

#endif /* #if !defined __CDS_SCHED_H */
