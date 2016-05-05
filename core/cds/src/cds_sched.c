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

/**
 *  File: cds_sched.c
 *
 *  DOC: CDS Scheduler Implementation
 */

 /* Include Files */
#include <cds_mq.h>
#include <cds_api.h>
#include <ani_global.h>
#include <sir_types.h>
#include <qdf_types.h>
#include <lim_api.h>
#include <sme_api.h>
#include <wlan_qct_sys.h>
#include "cds_sched.h"
#include <wlan_hdd_power.h>
#include "wma_types.h"
#include <linux/spinlock.h>
#include <linux/kthread.h>
#include <linux/cpu.h>
/* Preprocessor Definitions and Constants */
#define CDS_SCHED_THREAD_HEART_BEAT    INFINITE
/* Milli seconds to delay SSR thread when an Entry point is Active */
#define SSR_WAIT_SLEEP_TIME 200
/* MAX iteration count to wait for Entry point to exit before
 * we proceed with SSR in WD Thread
 */
#define MAX_SSR_WAIT_ITERATIONS 200
#define MAX_SSR_PROTECT_LOG (16)

static atomic_t ssr_protect_entry_count;

/**
 * struct ssr_protect - sub system restart(ssr) protection tracking table
 * @func: Function which needs ssr protection
 * @free: Flag to tell whether entry is free in table or not
 * @pid: Process id which needs ssr protection
 */
struct ssr_protect {
	const char *func;
	bool  free;
	uint32_t pid;
};

static spinlock_t ssr_protect_lock;
static struct ssr_protect ssr_protect_log[MAX_SSR_PROTECT_LOG];

static p_cds_sched_context gp_cds_sched_context;

static int cds_mc_thread(void *Arg);
#ifdef QCA_CONFIG_SMP
static int cds_ol_rx_thread(void *arg);
static unsigned long affine_cpu;
static QDF_STATUS cds_alloc_ol_rx_pkt_freeq(p_cds_sched_context pSchedContext);
#endif

#ifdef QCA_CONFIG_SMP
#define CDS_CORE_PER_CLUSTER (4)
static int cds_set_cpus_allowed_ptr(struct task_struct *task, unsigned long cpu)
{
	return set_cpus_allowed_ptr(task, cpumask_of(cpu));
}

/**
 * cds_cpu_hotplug_notify() - hot plug notify
 * @block: Pointer to block
 * @state: State
 * @hcpu: Pointer to hotplug cpu
 *
 * Return: NOTIFY_OK
 */
static int
cds_cpu_hotplug_notify(struct notifier_block *block,
		       unsigned long state, void *hcpu)
{
	unsigned long cpu = (unsigned long)hcpu;
	unsigned long pref_cpu = 0;
	p_cds_sched_context pSchedContext = get_cds_sched_ctxt();
	int i;
	unsigned int multi_cluster;
	unsigned int num_cpus;

	if ((NULL == pSchedContext) || (NULL == pSchedContext->ol_rx_thread))
		return NOTIFY_OK;

	if (cds_is_load_or_unload_in_progress())
		return NOTIFY_OK;

	num_cpus = num_possible_cpus();
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_LOW,
		  "%s: RX CORE %d, STATE %d, NUM CPUS %d",
		  __func__, (int)affine_cpu, (int)state, num_cpus);
	multi_cluster = (num_cpus > CDS_CORE_PER_CLUSTER) ? 1 : 0;

	switch (state) {
	case CPU_ONLINE:
		if ((!multi_cluster) && (affine_cpu != 0))
			return NOTIFY_OK;

		for_each_online_cpu(i) {
			if (i == 0)
				continue;
			pref_cpu = i;
			if (!multi_cluster)
				break;
		}
		break;
	case CPU_DEAD:
		if (cpu != affine_cpu)
			return NOTIFY_OK;

		affine_cpu = 0;
		for_each_online_cpu(i) {
			if (i == 0)
				continue;
			pref_cpu = i;
			if (!multi_cluster)
				break;
		}
	}

	if (pref_cpu == 0)
		return NOTIFY_OK;

	if (!cds_set_cpus_allowed_ptr(pSchedContext->ol_rx_thread, pref_cpu))
		affine_cpu = pref_cpu;

	return NOTIFY_OK;
}

static struct notifier_block cds_cpu_hotplug_notifier = {
	.notifier_call = cds_cpu_hotplug_notify,
};
#endif

/**
 * cds_sched_open() - initialize the CDS Scheduler
 * @p_cds_context: Pointer to the global CDS Context
 * @pSchedContext: Pointer to a previously allocated buffer big
 *	enough to hold a scheduler context.
 * @SchedCtxSize: CDS scheduler context size
 *
 * This function initializes the CDS Scheduler
 * Upon successful initialization:
 *	- All the message queues are initialized
 *	- The Main Controller thread is created and ready to receive and
 *	dispatch messages.
 *
 *
 * Return: QDF status
 */
QDF_STATUS cds_sched_open(void *p_cds_context,
		p_cds_sched_context pSchedContext,
		uint32_t SchedCtxSize)
{
	QDF_STATUS vStatus = QDF_STATUS_SUCCESS;
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Opening the CDS Scheduler", __func__);
	/* Sanity checks */
	if ((p_cds_context == NULL) || (pSchedContext == NULL)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Null params being passed", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	if (sizeof(cds_sched_context) != SchedCtxSize) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
			  "%s: Incorrect CDS Sched Context size passed",
			  __func__);
		return QDF_STATUS_E_INVAL;
	}
	qdf_mem_zero(pSchedContext, sizeof(cds_sched_context));
	pSchedContext->pVContext = p_cds_context;
	vStatus = cds_sched_init_mqs(pSchedContext);
	if (!QDF_IS_STATUS_SUCCESS(vStatus)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to initialize CDS Scheduler MQs",
			  __func__);
		return vStatus;
	}
	/* Initialize the helper events and event queues */
	init_completion(&pSchedContext->McStartEvent);
	init_completion(&pSchedContext->McShutdown);
	init_completion(&pSchedContext->ResumeMcEvent);

	spin_lock_init(&pSchedContext->McThreadLock);
#ifdef QCA_CONFIG_SMP
	spin_lock_init(&pSchedContext->ol_rx_thread_lock);
#endif

	init_waitqueue_head(&pSchedContext->mcWaitQueue);
	pSchedContext->mcEventFlag = 0;

#ifdef QCA_CONFIG_SMP
	init_waitqueue_head(&pSchedContext->ol_rx_wait_queue);
	init_completion(&pSchedContext->ol_rx_start_event);
	init_completion(&pSchedContext->ol_suspend_rx_event);
	init_completion(&pSchedContext->ol_resume_rx_event);
	init_completion(&pSchedContext->ol_rx_shutdown);
	pSchedContext->ol_rx_event_flag = 0;
	spin_lock_init(&pSchedContext->ol_rx_queue_lock);
	spin_lock_init(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	INIT_LIST_HEAD(&pSchedContext->ol_rx_thread_queue);
	spin_lock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	INIT_LIST_HEAD(&pSchedContext->cds_ol_rx_pkt_freeq);
	spin_unlock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	if (cds_alloc_ol_rx_pkt_freeq(pSchedContext) != QDF_STATUS_SUCCESS) {
		return QDF_STATUS_E_FAILURE;
	}
	register_hotcpu_notifier(&cds_cpu_hotplug_notifier);
	pSchedContext->cpu_hot_plug_notifier = &cds_cpu_hotplug_notifier;
#endif
	gp_cds_sched_context = pSchedContext;

	/* Create the CDS Main Controller thread */
	pSchedContext->McThread = kthread_create(cds_mc_thread, pSchedContext,
						 "cds_mc_thread");
	if (IS_ERR(pSchedContext->McThread)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "%s: Could not Create CDS Main Thread Controller",
			  __func__);
		goto MC_THREAD_START_FAILURE;
	}
	wake_up_process(pSchedContext->McThread);
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: CDS Main Controller thread Created", __func__);

#ifdef QCA_CONFIG_SMP
	pSchedContext->ol_rx_thread = kthread_create(cds_ol_rx_thread,
						       pSchedContext,
						       "cds_ol_rx_thread");
	if (IS_ERR(pSchedContext->ol_rx_thread)) {

		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "%s: Could not Create CDS OL RX Thread",
			  __func__);
		goto OL_RX_THREAD_START_FAILURE;

	}
	wake_up_process(pSchedContext->ol_rx_thread);
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  ("CDS OL RX thread Created"));
#endif
	/*
	 * Now make sure all threads have started before we exit.
	 * Each thread should normally ACK back when it starts.
	 */
	wait_for_completion_interruptible(&pSchedContext->McStartEvent);
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: CDS MC Thread has started", __func__);
#ifdef QCA_CONFIG_SMP
	wait_for_completion_interruptible(&pSchedContext->ol_rx_start_event);
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: CDS OL Rx Thread has started", __func__);
#endif
	/* We're good now: Let's get the ball rolling!!! */
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: CDS Scheduler successfully Opened", __func__);
	return QDF_STATUS_SUCCESS;

#ifdef QCA_CONFIG_SMP
OL_RX_THREAD_START_FAILURE:
	/* Try and force the Main thread controller to exit */
	set_bit(MC_SHUTDOWN_EVENT_MASK, &pSchedContext->mcEventFlag);
	set_bit(MC_POST_EVENT_MASK, &pSchedContext->mcEventFlag);
	wake_up_interruptible(&pSchedContext->mcWaitQueue);
	/* Wait for MC to exit */
	wait_for_completion_interruptible(&pSchedContext->McShutdown);
#endif

MC_THREAD_START_FAILURE:
	/* De-initialize all the message queues */
	cds_sched_deinit_mqs(pSchedContext);

#ifdef QCA_CONFIG_SMP
	unregister_hotcpu_notifier(&cds_cpu_hotplug_notifier);
	cds_free_ol_rx_pkt_freeq(gp_cds_sched_context);
#endif

	return QDF_STATUS_E_RESOURCES;

} /* cds_sched_open() */

/**
 * cds_mc_thread() - cds main controller thread execution handler
 * @Arg: Pointer to the global CDS Sched Context
 *
 * Return: thread exit code
 */
static int cds_mc_thread(void *Arg)
{
	p_cds_sched_context pSchedContext = (p_cds_sched_context) Arg;
	p_cds_msg_wrapper pMsgWrapper = NULL;
	tpAniSirGlobal pMacContext = NULL;
	tSirRetStatus macStatus = eSIR_SUCCESS;
	QDF_STATUS vStatus = QDF_STATUS_SUCCESS;
	int retWaitStatus = 0;
	bool shutdown = false;
	hdd_context_t *pHddCtx = NULL;
	v_CONTEXT_t p_cds_context = NULL;

	if (Arg == NULL) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Bad Args passed", __func__);
		return 0;
	}
	set_user_nice(current, -2);

	/* Ack back to the context from which the main controller thread
	 * has been created
	 */
	complete(&pSchedContext->McStartEvent);
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "%s: MC Thread %d (%s) starting up", __func__, current->pid,
		  current->comm);

	/* Get the Global CDS Context */
	p_cds_context = cds_get_global_context();
	if (!p_cds_context) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "%s: Global CDS context is Null", __func__);
		return 0;
	}

	pHddCtx = cds_get_context(QDF_MODULE_ID_HDD);
	if (!pHddCtx) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_FATAL,
			  "%s: HDD context is Null", __func__);
		return 0;
	}

	while (!shutdown) {
		/* This implements the execution model algorithm */
		retWaitStatus =
			wait_event_interruptible(pSchedContext->mcWaitQueue,
						 test_bit(MC_POST_EVENT_MASK,
							  &pSchedContext->mcEventFlag)
						 || test_bit(MC_SUSPEND_EVENT_MASK,
							     &pSchedContext->mcEventFlag));

		if (retWaitStatus == -ERESTARTSYS) {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				  "%s: wait_event_interruptible returned -ERESTARTSYS",
				  __func__);
			QDF_BUG(0);
		}
		clear_bit(MC_POST_EVENT_MASK, &pSchedContext->mcEventFlag);

		while (1) {
			/* Check if MC needs to shutdown */
			if (test_bit
				    (MC_SHUTDOWN_EVENT_MASK,
				    &pSchedContext->mcEventFlag)) {
				QDF_TRACE(QDF_MODULE_ID_QDF,
					  QDF_TRACE_LEVEL_INFO,
					  "%s: MC thread signaled to shutdown",
					  __func__);
				shutdown = true;
				/* Check for any Suspend Indication */
				if (test_bit
					    (MC_SUSPEND_EVENT_MASK,
					    &pSchedContext->mcEventFlag)) {
					clear_bit(MC_SUSPEND_EVENT_MASK,
						  &pSchedContext->mcEventFlag);

					/* Unblock anyone waiting on suspend */
					complete(&pHddCtx->mc_sus_event_var);
				}
				break;
			}
			/* Check the SYS queue first */
			if (!cds_is_mq_empty(&pSchedContext->sysMcMq)) {
				/* Service the SYS message queue */
				pMsgWrapper =
					cds_mq_get(&pSchedContext->sysMcMq);
				if (pMsgWrapper == NULL) {
					QDF_TRACE(QDF_MODULE_ID_QDF,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: pMsgWrapper is NULL",
						  __func__);
					QDF_ASSERT(0);
					break;
				}
				vStatus =
					sys_mc_process_msg(pSchedContext->pVContext,
							   pMsgWrapper->pVosMsg);
				if (!QDF_IS_STATUS_SUCCESS(vStatus)) {
					QDF_TRACE(QDF_MODULE_ID_QDF,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: Issue Processing SYS message",
						  __func__);
				}
				/* return message to the Core */
				cds_core_return_msg(pSchedContext->pVContext,
						    pMsgWrapper);
				continue;
			}
			/* Check the WMA queue */
			if (!cds_is_mq_empty(&pSchedContext->wmaMcMq)) {
				/* Service the WMA message queue */
				pMsgWrapper =
					cds_mq_get(&pSchedContext->wmaMcMq);
				if (pMsgWrapper == NULL) {
					QDF_TRACE(QDF_MODULE_ID_QDF,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: pMsgWrapper is NULL",
						  __func__);
					QDF_ASSERT(0);
					break;
				}
				vStatus =
					wma_mc_process_msg(pSchedContext->pVContext,
							 pMsgWrapper->pVosMsg);
				if (!QDF_IS_STATUS_SUCCESS(vStatus)) {
					QDF_TRACE(QDF_MODULE_ID_QDF,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: Issue Processing WMA message",
						  __func__);
				}
				/* return message to the Core */
				cds_core_return_msg(pSchedContext->pVContext,
						    pMsgWrapper);
				continue;
			}
			/* Check the PE queue */
			if (!cds_is_mq_empty(&pSchedContext->peMcMq)) {
				/* Service the PE message queue */
				pMsgWrapper =
					cds_mq_get(&pSchedContext->peMcMq);
				if (NULL == pMsgWrapper) {
					QDF_TRACE(QDF_MODULE_ID_QDF,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: pMsgWrapper is NULL",
						  __func__);
					QDF_ASSERT(0);
					break;
				}

				/* Need some optimization */
				pMacContext =
					cds_get_context(QDF_MODULE_ID_PE);
				if (NULL == pMacContext) {
					QDF_TRACE(QDF_MODULE_ID_QDF,
						  QDF_TRACE_LEVEL_INFO,
						  "MAC Context not ready yet");
					cds_core_return_msg
						(pSchedContext->pVContext,
						pMsgWrapper);
					continue;
				}

				macStatus =
					pe_process_messages(pMacContext,
							    (tSirMsgQ *)
							    pMsgWrapper->pVosMsg);
				if (eSIR_SUCCESS != macStatus) {
					QDF_TRACE(QDF_MODULE_ID_QDF,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: Issue Processing PE message",
						  __func__);
				}
				/* return message to the Core */
				cds_core_return_msg(pSchedContext->pVContext,
						    pMsgWrapper);
				continue;
			}
			/** Check the SME queue **/
			if (!cds_is_mq_empty(&pSchedContext->smeMcMq)) {
				/* Service the SME message queue */
				pMsgWrapper =
					cds_mq_get(&pSchedContext->smeMcMq);
				if (NULL == pMsgWrapper) {
					QDF_TRACE(QDF_MODULE_ID_QDF,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: pMsgWrapper is NULL",
						  __func__);
					QDF_ASSERT(0);
					break;
				}

				/* Need some optimization */
				pMacContext =
					cds_get_context(QDF_MODULE_ID_SME);
				if (NULL == pMacContext) {
					QDF_TRACE(QDF_MODULE_ID_QDF,
						  QDF_TRACE_LEVEL_INFO,
						  "MAC Context not ready yet");
					cds_core_return_msg
						(pSchedContext->pVContext,
						pMsgWrapper);
					continue;
				}

				vStatus =
					sme_process_msg((tHalHandle) pMacContext,
							pMsgWrapper->pVosMsg);
				if (!QDF_IS_STATUS_SUCCESS(vStatus)) {
					QDF_TRACE(QDF_MODULE_ID_QDF,
						  QDF_TRACE_LEVEL_ERROR,
						  "%s: Issue Processing SME message",
						  __func__);
				}
				/* return message to the Core */
				cds_core_return_msg(pSchedContext->pVContext,
						    pMsgWrapper);
				continue;
			}
			/* Check for any Suspend Indication */
			if (test_bit
				    (MC_SUSPEND_EVENT_MASK,
				    &pSchedContext->mcEventFlag)) {
				clear_bit(MC_SUSPEND_EVENT_MASK,
					  &pSchedContext->mcEventFlag);
				spin_lock(&pSchedContext->McThreadLock);
				INIT_COMPLETION(pSchedContext->ResumeMcEvent);
				/* Mc Thread Suspended */
				complete(&pHddCtx->mc_sus_event_var);

				spin_unlock(&pSchedContext->McThreadLock);

				/* Wait foe Resume Indication */
				wait_for_completion_interruptible
					(&pSchedContext->ResumeMcEvent);
			}
			break;  /* All queues are empty now */
		} /* while message loop processing */
	} /* while true */
	/* If we get here the MC thread must exit */
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "%s: MC Thread exiting!!!!", __func__);
	complete_and_exit(&pSchedContext->McShutdown, 0);
} /* cds_mc_thread() */

#ifdef QCA_CONFIG_SMP
/**
 * cds_free_ol_rx_pkt_freeq() - free cds buffer free queue
 * @pSchedContext - pointer to the global CDS Sched Context
 *
 * This API does mem free of the buffers available in free cds buffer
 * queue which is used for Data rx processing.
 *
 * Return: none
 */
void cds_free_ol_rx_pkt_freeq(p_cds_sched_context pSchedContext)
{
	struct cds_ol_rx_pkt *pkt;

	spin_lock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	while (!list_empty(&pSchedContext->cds_ol_rx_pkt_freeq)) {
		pkt = list_entry((&pSchedContext->cds_ol_rx_pkt_freeq)->next,
			typeof(*pkt), list);
		list_del(&pkt->list);
		spin_unlock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
		qdf_mem_free(pkt);
		spin_lock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	}
	spin_unlock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
}

/**
 * cds_alloc_ol_rx_pkt_freeq() - Function to allocate free buffer queue
 * @pSchedContext - pointer to the global CDS Sched Context
 *
 * This API allocates CDS_MAX_OL_RX_PKT number of cds message buffers
 * which are used for Rx data processing.
 *
 * Return: status of memory allocation
 */
static QDF_STATUS cds_alloc_ol_rx_pkt_freeq(p_cds_sched_context pSchedContext)
{
	struct cds_ol_rx_pkt *pkt, *tmp;
	int i;

	for (i = 0; i < CDS_MAX_OL_RX_PKT; i++) {
		pkt = qdf_mem_malloc(sizeof(*pkt));
		if (!pkt) {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				  "%s Vos packet allocation for ol rx thread failed",
				  __func__);
			goto free;
		}
		memset(pkt, 0, sizeof(*pkt));
		spin_lock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
		list_add_tail(&pkt->list, &pSchedContext->cds_ol_rx_pkt_freeq);
		spin_unlock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	}

	return QDF_STATUS_SUCCESS;

free:
	spin_lock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	list_for_each_entry_safe(pkt, tmp, &pSchedContext->cds_ol_rx_pkt_freeq,
				 list) {
		list_del(&pkt->list);
		spin_unlock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
		qdf_mem_free(pkt);
		spin_lock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	}
	spin_unlock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	return QDF_STATUS_E_NOMEM;
}

/**
 * cds_free_ol_rx_pkt() - api to release cds message to the freeq
 * This api returns the cds message used for Rx data to the free queue
 * @pSchedContext: Pointer to the global CDS Sched Context
 * @pkt: CDS message buffer to be returned to free queue.
 *
 * Return: none
 */
void
cds_free_ol_rx_pkt(p_cds_sched_context pSchedContext,
		    struct cds_ol_rx_pkt *pkt)
{
	memset(pkt, 0, sizeof(*pkt));
	spin_lock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	list_add_tail(&pkt->list, &pSchedContext->cds_ol_rx_pkt_freeq);
	spin_unlock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
}

/**
 * cds_alloc_ol_rx_pkt() - API to return next available cds message
 * @pSchedContext: Pointer to the global CDS Sched Context
 *
 * This api returns next available cds message buffer used for rx data
 * processing
 *
 * Return: Pointer to cds message buffer
 */
struct cds_ol_rx_pkt *cds_alloc_ol_rx_pkt(p_cds_sched_context pSchedContext)
{
	struct cds_ol_rx_pkt *pkt;

	spin_lock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	if (list_empty(&pSchedContext->cds_ol_rx_pkt_freeq)) {
		spin_unlock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
		return NULL;
	}
	pkt = list_first_entry(&pSchedContext->cds_ol_rx_pkt_freeq,
			       struct cds_ol_rx_pkt, list);
	list_del(&pkt->list);
	spin_unlock_bh(&pSchedContext->cds_ol_rx_pkt_freeq_lock);
	return pkt;
}

/**
 * cds_indicate_rxpkt() - indicate rx data packet
 * @Arg: Pointer to the global CDS Sched Context
 * @pkt: CDS data message buffer
 *
 * This api enqueues the rx packet into ol_rx_thread_queue and notifies
 * cds_ol_rx_thread()
 *
 * Return: none
 */
void
cds_indicate_rxpkt(p_cds_sched_context pSchedContext,
		   struct cds_ol_rx_pkt *pkt)
{
	spin_lock_bh(&pSchedContext->ol_rx_queue_lock);
	list_add_tail(&pkt->list, &pSchedContext->ol_rx_thread_queue);
	spin_unlock_bh(&pSchedContext->ol_rx_queue_lock);
	set_bit(RX_POST_EVENT_MASK, &pSchedContext->ol_rx_event_flag);
	wake_up_interruptible(&pSchedContext->ol_rx_wait_queue);
}

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
void cds_drop_rxpkt_by_staid(p_cds_sched_context pSchedContext, uint16_t staId)
{
	struct list_head local_list;
	struct cds_ol_rx_pkt *pkt, *tmp;
	qdf_nbuf_t buf, next_buf;

	INIT_LIST_HEAD(&local_list);
	spin_lock_bh(&pSchedContext->ol_rx_queue_lock);
	if (list_empty(&pSchedContext->ol_rx_thread_queue)) {
		spin_unlock_bh(&pSchedContext->ol_rx_queue_lock);
		return;
	}
	list_for_each_entry_safe(pkt, tmp, &pSchedContext->ol_rx_thread_queue,
								list) {
		if (pkt->staId == staId || staId == WLAN_MAX_STA_COUNT)
			list_move_tail(&pkt->list, &local_list);
	}
	spin_unlock_bh(&pSchedContext->ol_rx_queue_lock);

	list_for_each_entry_safe(pkt, tmp, &local_list, list) {
		list_del(&pkt->list);
		buf = pkt->Rxpkt;
		while (buf) {
			next_buf = qdf_nbuf_queue_next(buf);
			qdf_nbuf_free(buf);
			buf = next_buf;
		}
		cds_free_ol_rx_pkt(pSchedContext, pkt);
	}
}

/**
 * cds_rx_from_queue() - function to process pending Rx packets
 * @pSchedContext: Pointer to the global CDS Sched Context
 *
 * This api traverses the pending buffer list and calling the callback.
 * This callback would essentially send the packet to HDD.
 *
 * Return: none
 */
static void cds_rx_from_queue(p_cds_sched_context pSchedContext)
{
	struct cds_ol_rx_pkt *pkt;
	uint16_t sta_id;

	spin_lock_bh(&pSchedContext->ol_rx_queue_lock);
	while (!list_empty(&pSchedContext->ol_rx_thread_queue)) {
		pkt = list_first_entry(&pSchedContext->ol_rx_thread_queue,
				       struct cds_ol_rx_pkt, list);
		list_del(&pkt->list);
		spin_unlock_bh(&pSchedContext->ol_rx_queue_lock);
		sta_id = pkt->staId;
		pkt->callback(pkt->context, pkt->Rxpkt, sta_id);
		cds_free_ol_rx_pkt(pSchedContext, pkt);
		spin_lock_bh(&pSchedContext->ol_rx_queue_lock);
	}
	spin_unlock_bh(&pSchedContext->ol_rx_queue_lock);
}

/**
 * cds_ol_rx_thread() - cds main tlshim rx thread
 * @Arg: pointer to the global CDS Sched Context
 *
 * This api is the thread handler for Tlshim Data packet processing.
 *
 * Return: thread exit code
 */
static int cds_ol_rx_thread(void *arg)
{
	p_cds_sched_context pSchedContext = (p_cds_sched_context) arg;
	unsigned long pref_cpu = 0;
	bool shutdown = false;
	int status, i;
	unsigned int num_cpus;

	set_user_nice(current, -1);
#ifdef MSM_PLATFORM
	set_wake_up_idle(true);
#endif

	num_cpus = num_possible_cpus();
	/* Find the available cpu core other than cpu 0 and
	 * bind the thread
	 */
	for_each_online_cpu(i) {
		if (i == 0)
			continue;
		pref_cpu = i;
		if (num_cpus <= CDS_CORE_PER_CLUSTER)
			break;
	}
	if (pref_cpu != 0 && (!cds_set_cpus_allowed_ptr(current, pref_cpu)))
		affine_cpu = pref_cpu;

	if (!arg) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Bad Args passed", __func__);
		return 0;
	}

	complete(&pSchedContext->ol_rx_start_event);

	while (!shutdown) {
		status =
			wait_event_interruptible(pSchedContext->ol_rx_wait_queue,
						 test_bit(RX_POST_EVENT_MASK,
							  &pSchedContext->ol_rx_event_flag)
						 || test_bit(RX_SUSPEND_EVENT_MASK,
							     &pSchedContext->ol_rx_event_flag));
		if (status == -ERESTARTSYS)
			break;

		clear_bit(RX_POST_EVENT_MASK, &pSchedContext->ol_rx_event_flag);
		while (true) {
			if (test_bit(RX_SHUTDOWN_EVENT_MASK,
				     &pSchedContext->ol_rx_event_flag)) {
				clear_bit(RX_SHUTDOWN_EVENT_MASK,
					  &pSchedContext->ol_rx_event_flag);
				if (test_bit(RX_SUSPEND_EVENT_MASK,
					     &pSchedContext->ol_rx_event_flag)) {
					clear_bit(RX_SUSPEND_EVENT_MASK,
						  &pSchedContext->ol_rx_event_flag);
					complete
						(&pSchedContext->ol_suspend_rx_event);
				}
				QDF_TRACE(QDF_MODULE_ID_QDF,
					  QDF_TRACE_LEVEL_INFO,
					  "%s: Shutting down OL RX Thread",
					  __func__);
				shutdown = true;
				break;
			}
			cds_rx_from_queue(pSchedContext);

			if (test_bit(RX_SUSPEND_EVENT_MASK,
				     &pSchedContext->ol_rx_event_flag)) {
				clear_bit(RX_SUSPEND_EVENT_MASK,
					  &pSchedContext->ol_rx_event_flag);
				spin_lock(&pSchedContext->ol_rx_thread_lock);
				INIT_COMPLETION
					(pSchedContext->ol_resume_rx_event);
				complete(&pSchedContext->ol_suspend_rx_event);
				spin_unlock(&pSchedContext->ol_rx_thread_lock);
				wait_for_completion_interruptible
					(&pSchedContext->ol_resume_rx_event);
			}
			break;
		}
	}

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "%s: Exiting CDS OL rx thread", __func__);
	complete_and_exit(&pSchedContext->ol_rx_shutdown, 0);
}
#endif

/**
 * cds_sched_close() - close the cds scheduler
 * @p_cds_context: Pointer to the global CDS Context
 *
 * This api closes the CDS Scheduler upon successful closing:
 *	- All the message queues are flushed
 *	- The Main Controller thread is closed
 *	- The Tx thread is closed
 *
 *
 * Return: qdf status
 */
QDF_STATUS cds_sched_close(void *p_cds_context)
{
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: invoked", __func__);
	if (gp_cds_sched_context == NULL) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: gp_cds_sched_context == NULL", __func__);
		return QDF_STATUS_E_FAILURE;
	}
	/* shut down MC Thread */
	set_bit(MC_SHUTDOWN_EVENT_MASK, &gp_cds_sched_context->mcEventFlag);
	set_bit(MC_POST_EVENT_MASK, &gp_cds_sched_context->mcEventFlag);
	wake_up_interruptible(&gp_cds_sched_context->mcWaitQueue);
	/* Wait for MC to exit */
	wait_for_completion(&gp_cds_sched_context->McShutdown);
	gp_cds_sched_context->McThread = 0;

	/* Clean up message queues of MC thread */
	cds_sched_flush_mc_mqs(gp_cds_sched_context);

	/* Deinit all the queues */
	cds_sched_deinit_mqs(gp_cds_sched_context);

#ifdef QCA_CONFIG_SMP
	/* Shut down Tlshim Rx thread */
	set_bit(RX_SHUTDOWN_EVENT_MASK, &gp_cds_sched_context->ol_rx_event_flag);
	set_bit(RX_POST_EVENT_MASK, &gp_cds_sched_context->ol_rx_event_flag);
	wake_up_interruptible(&gp_cds_sched_context->ol_rx_wait_queue);
	wait_for_completion(&gp_cds_sched_context->ol_rx_shutdown);
	gp_cds_sched_context->ol_rx_thread = NULL;
	cds_drop_rxpkt_by_staid(gp_cds_sched_context, WLAN_MAX_STA_COUNT);
	cds_free_ol_rx_pkt_freeq(gp_cds_sched_context);
	unregister_hotcpu_notifier(&cds_cpu_hotplug_notifier);
#endif
	return QDF_STATUS_SUCCESS;
} /* cds_sched_close() */

/**
 * cds_sched_init_mqs() - initialize the cds scheduler message queues
 * @p_cds_sched_context: Pointer to the Scheduler Context.
 *
 * This api initializes the cds scheduler message queues.
 *
 * Return: QDF status
 */
QDF_STATUS cds_sched_init_mqs(p_cds_sched_context pSchedContext)
{
	QDF_STATUS vStatus = QDF_STATUS_SUCCESS;
	/* Now intialize all the message queues */
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Initializing the WMA MC Message queue", __func__);
	vStatus = cds_mq_init(&pSchedContext->wmaMcMq);
	if (!QDF_IS_STATUS_SUCCESS(vStatus)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to init WMA MC Message queue", __func__);
		QDF_ASSERT(0);
		return vStatus;
	}
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Initializing the PE MC Message queue", __func__);
	vStatus = cds_mq_init(&pSchedContext->peMcMq);
	if (!QDF_IS_STATUS_SUCCESS(vStatus)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to init PE MC Message queue", __func__);
		QDF_ASSERT(0);
		return vStatus;
	}
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Initializing the SME MC Message queue", __func__);
	vStatus = cds_mq_init(&pSchedContext->smeMcMq);
	if (!QDF_IS_STATUS_SUCCESS(vStatus)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to init SME MC Message queue", __func__);
		QDF_ASSERT(0);
		return vStatus;
	}
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s: Initializing the SYS MC Message queue", __func__);
	vStatus = cds_mq_init(&pSchedContext->sysMcMq);
	if (!QDF_IS_STATUS_SUCCESS(vStatus)) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: Failed to init SYS MC Message queue", __func__);
		QDF_ASSERT(0);
		return vStatus;
	}

	return QDF_STATUS_SUCCESS;
} /* cds_sched_init_mqs() */

/**
 * cds_sched_deinit_mqs() - Deinitialize the cds scheduler message queues
 * @p_cds_sched_context: Pointer to the Scheduler Context.
 *
 * Return: none
 */
void cds_sched_deinit_mqs(p_cds_sched_context pSchedContext)
{
	/* Now de-intialize all message queues */

	/* MC WMA */
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s De-Initializing the WMA MC Message queue", __func__);
	cds_mq_deinit(&pSchedContext->wmaMcMq);
	/* MC PE */
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s De-Initializing the PE MC Message queue", __func__);
	cds_mq_deinit(&pSchedContext->peMcMq);
	/* MC SME */
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s De-Initializing the SME MC Message queue", __func__);
	cds_mq_deinit(&pSchedContext->smeMcMq);
	/* MC SYS */
	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO_HIGH,
		  "%s De-Initializing the SYS MC Message queue", __func__);
	cds_mq_deinit(&pSchedContext->sysMcMq);

} /* cds_sched_deinit_mqs() */

/**
 * cds_sched_flush_mc_mqs() - flush all the MC thread message queues
 * @pSchedContext: Pointer to global cds context
 *
 * Return: none
 */
void cds_sched_flush_mc_mqs(p_cds_sched_context pSchedContext)
{
	p_cds_msg_wrapper pMsgWrapper = NULL;
	p_cds_contextType cds_ctx;

	/* Here each of the MC thread MQ shall be drained and returned to the
	 * Core. Before returning a wrapper to the Core, the CDS message shall
	 * be freed first
	 */
	QDF_TRACE(QDF_MODULE_ID_QDF,
		  QDF_TRACE_LEVEL_INFO,
		  ("Flushing the MC Thread message queue"));

	if (NULL == pSchedContext) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: pSchedContext is NULL", __func__);
		return;
	}

	cds_ctx = (p_cds_contextType) (pSchedContext->pVContext);
	if (NULL == cds_ctx) {
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: cds_ctx is NULL", __func__);
		return;
	}

	/* Flush the SYS Mq */
	while (NULL != (pMsgWrapper = cds_mq_get(&pSchedContext->sysMcMq))) {
		QDF_TRACE(QDF_MODULE_ID_QDF,
			  QDF_TRACE_LEVEL_INFO,
			  "%s: Freeing MC SYS message type %d ", __func__,
			  pMsgWrapper->pVosMsg->type);
		cds_core_return_msg(pSchedContext->pVContext, pMsgWrapper);
	}
	/* Flush the WMA Mq */
	while (NULL != (pMsgWrapper = cds_mq_get(&pSchedContext->wmaMcMq))) {
		if (pMsgWrapper->pVosMsg != NULL) {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
				  "%s: Freeing MC WMA MSG message type %d",
				  __func__, pMsgWrapper->pVosMsg->type);
			if (pMsgWrapper->pVosMsg->bodyptr) {
				qdf_mem_free((void *)pMsgWrapper->
					     pVosMsg->bodyptr);
			}

			pMsgWrapper->pVosMsg->bodyptr = NULL;
			pMsgWrapper->pVosMsg->bodyval = 0;
			pMsgWrapper->pVosMsg->type = 0;
		}
		cds_core_return_msg(pSchedContext->pVContext, pMsgWrapper);
	}

	/* Flush the PE Mq */
	while (NULL != (pMsgWrapper = cds_mq_get(&pSchedContext->peMcMq))) {
		QDF_TRACE(QDF_MODULE_ID_QDF,
			  QDF_TRACE_LEVEL_INFO,
			  "%s: Freeing MC PE MSG message type %d", __func__,
			  pMsgWrapper->pVosMsg->type);
		pe_free_msg(cds_ctx->pMACContext,
			    (tSirMsgQ *) pMsgWrapper->pVosMsg);
		cds_core_return_msg(pSchedContext->pVContext, pMsgWrapper);
	}
	/* Flush the SME Mq */
	while (NULL != (pMsgWrapper = cds_mq_get(&pSchedContext->smeMcMq))) {
		QDF_TRACE(QDF_MODULE_ID_QDF,
			  QDF_TRACE_LEVEL_INFO,
			  "%s: Freeing MC SME MSG message type %d", __func__,
			  pMsgWrapper->pVosMsg->type);
		sme_free_msg(cds_ctx->pMACContext, pMsgWrapper->pVosMsg);
		cds_core_return_msg(pSchedContext->pVContext, pMsgWrapper);
	}
} /* cds_sched_flush_mc_mqs() */

/**
 * get_cds_sched_ctxt() - get cds scheduler context
 *
 * Return: none
 */
p_cds_sched_context get_cds_sched_ctxt(void)
{
	/* Make sure that Vos Scheduler context has been initialized */
	if (gp_cds_sched_context == NULL)
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			  "%s: gp_cds_sched_context == NULL", __func__);

	return gp_cds_sched_context;
}

/**
 * cds_ssr_protect_init() - initialize ssr protection debug functionality
 *
 * Return:
 *        void
 */
void cds_ssr_protect_init(void)
{
	int i = 0;

	spin_lock_init(&ssr_protect_lock);

	while (i < MAX_SSR_PROTECT_LOG) {
		ssr_protect_log[i].func = NULL;
		ssr_protect_log[i].free = true;
		ssr_protect_log[i].pid =  0;
		i++;
	}
}

/**
 * cds_print_external_threads() - print external threads stuck in driver
 *
 * Return:
 *        void
 */

static void cds_print_external_threads(void)
{
	int i = 0;
	unsigned long irq_flags;

	spin_lock_irqsave(&ssr_protect_lock, irq_flags);

	while (i < MAX_SSR_PROTECT_LOG) {
		if (!ssr_protect_log[i].free) {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			"PID %d is stuck at %s", ssr_protect_log[i].pid,
			ssr_protect_log[i].func);
		}
		i++;
	}

	spin_unlock_irqrestore(&ssr_protect_lock, irq_flags);
}

/**
 * cds_ssr_protect() - start ssr protection
 * @caller_func: name of calling function.
 *
 * This function is called to keep track of active driver entry points
 *
 * Return: none
 */
void cds_ssr_protect(const char *caller_func)
{
	int count;
	int i = 0;
	bool status = false;
	unsigned long irq_flags;

	count = atomic_inc_return(&ssr_protect_entry_count);

	spin_lock_irqsave(&ssr_protect_lock, irq_flags);

	while (i < MAX_SSR_PROTECT_LOG) {
		if (ssr_protect_log[i].free) {
			ssr_protect_log[i].func = caller_func;
			ssr_protect_log[i].free = false;
			ssr_protect_log[i].pid = current->pid;
			status = true;
			break;
		}
		i++;
	}

	spin_unlock_irqrestore(&ssr_protect_lock, irq_flags);

	if (!status)
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
		"Could not track PID %d call %s: log is full",
		current->pid, caller_func);
}

/**
 * cds_ssr_unprotect() - stop ssr protection
 * @caller_func: name of calling function.
 *
 * Return: none
 */
void cds_ssr_unprotect(const char *caller_func)
{
	int count;
	int i = 0;
	bool status = false;
	unsigned long irq_flags;

	count = atomic_dec_return(&ssr_protect_entry_count);

	spin_lock_irqsave(&ssr_protect_lock, irq_flags);

	while (i < MAX_SSR_PROTECT_LOG) {
		if (!ssr_protect_log[i].free) {
			if ((ssr_protect_log[i].pid == current->pid) &&
			     !strcmp(ssr_protect_log[i].func, caller_func)) {
				ssr_protect_log[i].func = NULL;
				ssr_protect_log[i].free = true;
				ssr_protect_log[i].pid =  0;
				status = true;
				break;
			}
		}
		i++;
	}

	spin_unlock_irqrestore(&ssr_protect_lock, irq_flags);

	if (!status)
		QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
			"Untracked call %s", caller_func);
}

/**
 * cds_wait_for_external_threads_completion() - wait for external threads
 *					completion before proceeding further
 * @caller_func: name of calling function.
 *
 * Return: true if there is no active entry points in driver
 *	   false if there is at least one active entry in driver
 */
bool cds_wait_for_external_threads_completion(const char *caller_func)
{
	int count = MAX_SSR_WAIT_ITERATIONS;

	while (count) {

		if (!atomic_read(&ssr_protect_entry_count))
			break;

		if (--count) {
			QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_ERROR,
				  "%s: Waiting for active entry points to exit",
				  __func__);
			msleep(SSR_WAIT_SLEEP_TIME);
		}
	}
	/* at least one external thread is executing */
	if (!count) {
		cds_print_external_threads();
		return false;
	}

	QDF_TRACE(QDF_MODULE_ID_QDF, QDF_TRACE_LEVEL_INFO,
		  "Allowing SSR/Driver unload for %s", caller_func);

	return true;
}

/**
 * cds_get_gfp_flags(): get GFP flags
 *
 * Based on the scheduled context, return GFP flags
 * Return: gfp flags
 */
int cds_get_gfp_flags(void)
{
	int flags = GFP_KERNEL;

	if (in_interrupt() || in_atomic() || irqs_disabled())
		flags = GFP_ATOMIC;

	return flags;
}
