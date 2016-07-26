/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
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

#if !defined(__CDS_SCHED_H)
#define __CDS_SCHED_H

/**=========================================================================

   \file  cds_sched.h

   \brief Connectivity driver services scheduler

   ========================================================================*/

/*--------------------------------------------------------------------------
   Include Files
   ------------------------------------------------------------------------*/
#include <qdf_event.h>
#include <i_qdf_types.h>
#include <linux/wait.h>
#if defined(WLAN_OPEN_SOURCE) && defined(CONFIG_HAS_WAKELOCK)
#include <linux/wakelock.h>
#endif
#include <cds_mq.h>
#include <qdf_types.h>
#include "qdf_lock.h"
#include "qdf_mc_timer.h"
#include "cds_config.h"

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
** QDF Message queue definition.
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

	/* affinity lock */
	struct mutex affinity_lock;

	/* rx thread affinity cpu */
	unsigned long rx_thread_cpu;

	/* high throughput required */
	bool high_throughput_required;
#endif
} cds_sched_context, *p_cds_sched_context;

/**
 * struct cds_log_complete - Log completion internal structure
 * @is_fatal: Type is fatal or not
 * @indicator: Source of bug report
 * @reason_code: Reason code for bug report
 * @is_report_in_progress: If bug report is in progress
 * @recovery_needed: if recovery is needed after report completion
 *
 * This structure internally stores the log related params
 */
struct cds_log_complete {
	uint32_t is_fatal;
	uint32_t indicator;
	uint32_t reason_code;
	bool is_report_in_progress;
	bool recovery_needed;
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
	cds_sched_context qdf_sched;

	/* HDD Module Context  */
	void *pHDDContext;

	/* MAC Module Context  */
	void *pMACContext;

	qdf_event_t ProbeEvent;

	uint32_t driver_state;

	qdf_event_t wmaCompleteEvent;

	/* WMA Context */
	void *pWMAContext;

	void *pHIFContext;

	void *htc_ctx;

	void *g_ol_context;
	/*
	 * qdf_ctx will be used by qdf
	 * while allocating dma memory
	 * to access dev information.
	 */
	qdf_device_t qdf_ctx;

	void *pdev_txrx_ctx;

	/* Configuration handle used to get system configuration */
	void *cfg_ctx;

	/* radio index per driver */
	int radio_index;

	bool is_wakelock_log_enabled;
	uint32_t wakelock_log_level;
	uint32_t connectivity_log_level;
	uint32_t packet_stats_log_level;
	uint32_t driver_debug_log_level;
	uint32_t fw_debug_log_level;
	struct cds_log_complete log_complete;
	qdf_spinlock_t bug_report_lock;
	qdf_event_t connection_update_done_evt;
	qdf_mutex_t qdf_conc_list_lock;
	qdf_mc_timer_t dbs_opportunistic_timer;
#ifdef FEATURE_WLAN_MCC_TO_SCC_SWITCH
	void (*sap_restart_chan_switch_cb)(void *, uint32_t, uint32_t);
#endif
	QDF_STATUS (*sme_get_valid_channels)(void*, uint8_t *, uint32_t *);
	void (*sme_get_nss_for_vdev)(void*, enum tQDF_ADAPTER_MODE,
		uint8_t *, uint8_t *);

	void (*ol_txrx_update_mac_id)(uint8_t , uint8_t);

	/* This list is not sessionized. This mandatory channel list would be
	 * as per OEMs preference as per the regulatory/other considerations.
	 * So, this would remain same for all the interfaces.
	 */
	uint8_t sap_mandatory_channels[QDF_MAX_NUM_CHAN];
	uint32_t sap_mandatory_channels_len;
	bool do_hw_mode_change;
	bool enable_fatal_event;
	struct cds_config_info *cds_cfg;
} cds_context_type, *p_cds_contextType;

/*---------------------------------------------------------------------------
   Function declarations and documenation
   ---------------------------------------------------------------------------*/
#ifdef QCA_CONFIG_SMP
int cds_sched_handle_cpu_hot_plug(void);
int cds_sched_handle_throughput_req(bool high_tput_required);

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
#else
/**
 * cds_drop_rxpkt_by_staid() - api to drop pending rx packets for a sta
 * @pSchedContext: Pointer to the global CDS Sched Context
 * @staId: Station Id
 *
 * This api drops queued packets for a station, to drop all the pending
 * packets the caller has to send WLAN_MAX_STA_COUNT as staId.
 *
 * Return: none
 */
static inline
void cds_drop_rxpkt_by_staid(p_cds_sched_context pSchedContext, uint16_t staId)
{
}

/**
 * cds_indicate_rxpkt() - API to Indicate rx data packet
 * @pSchedContext: pointer to  CDS Sched Context
 * @pkt: CDS OL RX pkt pointer containing to RX data message buffer
 *
 * Return: none
 */
static inline
void cds_indicate_rxpkt(p_cds_sched_context pSchedContext,
			struct cds_ol_rx_pkt *pkt)
{
}

/**
 * cds_alloc_ol_rx_pkt() - API to return next available cds message
 * @pSchedContext: pointer to  CDS Sched Context
 *
 * Return: none
 */
static inline
struct cds_ol_rx_pkt *cds_alloc_ol_rx_pkt(p_cds_sched_context pSchedContext)
{
}

/**
 * cds_free_ol_rx_pkt() - API to release cds message to the freeq
 * @pSchedContext: pointer to  CDS Sched Context
 * @pkt: CDS message buffer to be returned to free queue
 *
 * Return: none
 */
static inline
void cds_free_ol_rx_pkt(p_cds_sched_context pSchedContext,
			 struct cds_ol_rx_pkt *pkt)
{
}

/**
 * cds_free_ol_rx_pkt_freeq() - Free cds buffer free queue
 * @pSchedContext: pointer to  CDS Sched Context
 * @pkt: CDS message buffer to be returned to free queue
 *
 * Return: none
 */
static inline
void cds_free_ol_rx_pkt_freeq(p_cds_sched_context pSchedContext)
{
}

static inline int cds_sched_handle_throughput_req(
	bool high_tput_required)
{
	return 0;
}

#endif

/*---------------------------------------------------------------------------

   \brief cds_sched_open() - initialize the CDS Scheduler

   The \a cds_sched_open() function initializes the CDS Scheduler
   Upon successful initialization:

     - All the message queues are initialized

     - The Main Controller thread is created and ready to receive and
       dispatch messages.

     - The Tx thread is created and ready to receive and dispatch messages

   \param  p_cds_context - pointer to the global QDF Context

   \param  p_cds_sched_context - pointer to a previously allocated buffer big
   enough to hold a scheduler context.

   \return QDF_STATUS_SUCCESS - Scheduler was successfully initialized and
   is ready to be used.

   QDF_STATUS_E_RESOURCES - System resources (other than memory)
   are unavailable to initilize the scheduler

   QDF_STATUS_E_NOMEM - insufficient memory exists to initialize
   the scheduler

   QDF_STATUS_E_INVAL - Invalid parameter passed to the scheduler Open
   function

   QDF_STATUS_E_FAILURE - Failure to initialize the scheduler/

   \sa cds_sched_open()

   -------------------------------------------------------------------------*/
QDF_STATUS cds_sched_open(void *p_cds_context,
			  p_cds_sched_context pSchedCxt, uint32_t SchedCtxSize);

/*---------------------------------------------------------------------------

   \brief cds_sched_close() - Close the CDS Scheduler

   The \a cds_sched_closes() function closes the CDS Scheduler
   Upon successful closing:

     - All the message queues are flushed

     - The Main Controller thread is closed

     - The Tx thread is closed

   \param  p_cds_context - pointer to the global QDF Context

   \return QDF_STATUS_SUCCESS - Scheduler was successfully initialized and
   is ready to be used.

   QDF_STATUS_E_INVAL - Invalid parameter passed to the scheduler Open
   function

   QDF_STATUS_E_FAILURE - Failure to initialize the scheduler/

   \sa cds_sched_close()

   ---------------------------------------------------------------------------*/
QDF_STATUS cds_sched_close(void *p_cds_context);

/* Helper routines provided to other CDS API's */
QDF_STATUS cds_mq_init(p_cds_mq_type pMq);
void cds_mq_deinit(p_cds_mq_type pMq);
void cds_mq_put(p_cds_mq_type pMq, p_cds_msg_wrapper pMsgWrapper);
p_cds_msg_wrapper cds_mq_get(p_cds_mq_type pMq);
bool cds_is_mq_empty(p_cds_mq_type pMq);
p_cds_sched_context get_cds_sched_ctxt(void);
QDF_STATUS cds_sched_init_mqs(p_cds_sched_context pSchedContext);
void cds_sched_deinit_mqs(p_cds_sched_context pSchedContext);
void cds_sched_flush_mc_mqs(p_cds_sched_context pSchedContext);

void qdf_timer_module_init(void);
void qdf_timer_module_deinit(void);
void cds_ssr_protect_init(void);
void cds_ssr_protect(const char *caller_func);
void cds_ssr_unprotect(const char *caller_func);
bool cds_wait_for_external_threads_completion(const char *caller_func);
int cds_get_gfp_flags(void);

#endif /* #if !defined __CDS_SCHED_H */
