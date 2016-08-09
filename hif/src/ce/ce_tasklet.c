/*
 * Copyright (c) 2015-2016 The Linux Foundation. All rights reserved.
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

#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/if_arp.h>
#include "qdf_lock.h"
#include "qdf_types.h"
#include "qdf_status.h"
#include "regtable.h"
#include "hif.h"
#include "hif_io32.h"
#include "ce_main.h"
#include "ce_api.h"
#include "ce_reg.h"
#include "ce_internal.h"
#include "ce_tasklet.h"
#include "pld_common.h"
#include "hif_debug.h"
#include "hif_napi.h"


/**
 * struct tasklet_work
 *
 * @id: ce_id
 * @work: work
 */
struct tasklet_work {
	enum ce_id_type id;
	void *data;
	struct work_struct work;
};


/**
 * reschedule_ce_tasklet_work_handler() - reschedule work
 * @work: struct work_struct
 *
 * Return: N/A
 */
static void reschedule_ce_tasklet_work_handler(struct work_struct *work)
{
	struct tasklet_work *ce_work = container_of(work, struct tasklet_work,
						    work);
	struct hif_softc *scn = ce_work->data;
	struct HIF_CE_state *hif_ce_state;

	if (NULL == scn) {
		HIF_ERROR("%s: tasklet scn is null", __func__);
		return;
	}

	hif_ce_state = HIF_GET_CE_STATE(scn);

	if (scn->hif_init_done == false) {
		HIF_ERROR("%s: wlan driver is unloaded", __func__);
		return;
	}
	tasklet_schedule(&hif_ce_state->tasklets[ce_work->id].intr_tq);
	return;
}

static struct tasklet_work tasklet_workers[CE_ID_MAX];
static bool work_initialized;

/**
 * init_tasklet_work() - init_tasklet_work
 * @work: struct work_struct
 * @work_handler: work_handler
 *
 * Return: N/A
 */
static void init_tasklet_work(struct work_struct *work,
			      work_func_t work_handler)
{
	INIT_WORK(work, work_handler);
}

/**
 * init_tasklet_workers() - init_tasklet_workers
 * @scn: HIF Context
 *
 * Return: N/A
 */
void init_tasklet_workers(struct hif_opaque_softc *scn)
{
	uint32_t id;

	for (id = 0; id < CE_ID_MAX; id++) {
		tasklet_workers[id].id = id;
		tasklet_workers[id].data = scn;
		init_tasklet_work(&tasklet_workers[id].work,
				  reschedule_ce_tasklet_work_handler);
	}
	work_initialized = true;
}

#ifdef CONFIG_SLUB_DEBUG_ON
/**
 * ce_schedule_tasklet() - schedule ce tasklet
 * @tasklet_entry: struct ce_tasklet_entry
 *
 * Return: N/A
 */
static inline void ce_schedule_tasklet(struct ce_tasklet_entry *tasklet_entry)
{
	if (work_initialized && (tasklet_entry->ce_id < CE_ID_MAX))
		schedule_work(&tasklet_workers[tasklet_entry->ce_id].work);
	else
		HIF_ERROR("%s: work_initialized = %d, ce_id = %d",
			__func__, work_initialized, tasklet_entry->ce_id);
}
#else
/**
 * ce_schedule_tasklet() - schedule ce tasklet
 * @tasklet_entry: struct ce_tasklet_entry
 *
 * Return: N/A
 */
static inline void ce_schedule_tasklet(struct ce_tasklet_entry *tasklet_entry)
{
	tasklet_schedule(&tasklet_entry->intr_tq);
}
#endif

/**
 * ce_tasklet() - ce_tasklet
 * @data: data
 *
 * Return: N/A
 */
static void ce_tasklet(unsigned long data)
{
	struct ce_tasklet_entry *tasklet_entry =
		(struct ce_tasklet_entry *)data;
	struct HIF_CE_state *hif_ce_state = tasklet_entry->hif_ce_state;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ce_state);
	struct CE_state *CE_state = scn->ce_id_to_state[tasklet_entry->ce_id];

	hif_record_ce_desc_event(scn, tasklet_entry->ce_id,
			HIF_CE_TASKLET_ENTRY, NULL, NULL, 0);

	if (qdf_atomic_read(&scn->link_suspended)) {
		HIF_ERROR("%s: ce %d tasklet fired after link suspend.",
				__func__, tasklet_entry->ce_id);
		QDF_BUG(0);
	}

	ce_per_engine_service(scn, tasklet_entry->ce_id);

	if (CE_state->lro_flush_cb != NULL) {
		CE_state->lro_flush_cb(CE_state->lro_data);
	}

	if (ce_check_rx_pending(CE_state)) {
		/*
		 * There are frames pending, schedule tasklet to process them.
		 * Enable the interrupt only when there is no pending frames in
		 * any of the Copy Engine pipes.
		 */
		hif_record_ce_desc_event(scn, tasklet_entry->ce_id,
				HIF_CE_TASKLET_RESCHEDULE, NULL, NULL, 0);
		ce_schedule_tasklet(tasklet_entry);
		return;
	}

	if (scn->target_status != TARGET_STATUS_RESET)
		hif_irq_enable(scn, tasklet_entry->ce_id);

	hif_record_ce_desc_event(scn, tasklet_entry->ce_id, HIF_CE_TASKLET_EXIT,
				 NULL, NULL, 0);

	qdf_atomic_dec(&scn->active_tasklet_cnt);
}

/**
 * ce_tasklet_init() - ce_tasklet_init
 * @hif_ce_state: hif_ce_state
 * @mask: mask
 *
 * Return: N/A
 */
void ce_tasklet_init(struct HIF_CE_state *hif_ce_state, uint32_t mask)
{
	int i;

	for (i = 0; i < CE_COUNT_MAX; i++) {
		if (mask & (1 << i)) {
			hif_ce_state->tasklets[i].ce_id = i;
			hif_ce_state->tasklets[i].inited = true;
			hif_ce_state->tasklets[i].hif_ce_state = hif_ce_state;
			tasklet_init(&hif_ce_state->tasklets[i].intr_tq,
				ce_tasklet,
				(unsigned long)&hif_ce_state->tasklets[i]);
		}
	}
}
/**
 * ce_tasklet_kill() - ce_tasklet_kill
 * @hif_ce_state: hif_ce_state
 *
 * Return: N/A
 */
void ce_tasklet_kill(struct hif_softc *scn)
{
	int i;
	struct HIF_CE_state *hif_ce_state = HIF_GET_CE_STATE(scn);

	for (i = 0; i < CE_COUNT_MAX; i++)
		if (hif_ce_state->tasklets[i].inited) {
			tasklet_kill(&hif_ce_state->tasklets[i].intr_tq);
			hif_ce_state->tasklets[i].inited = false;
		}
	qdf_atomic_set(&scn->active_tasklet_cnt, 0);
}

#define HIF_CE_DRAIN_WAIT_CNT          20
/**
 * hif_drain_tasklets(): wait untill no tasklet is pending
 * @scn: hif context
 *
 * Let running tasklets clear pending trafic.
 *
 * Return: 0 if no bottom half is in progress when it returns.
 *   -EFAULT if it times out.
 */
int hif_drain_tasklets(struct hif_softc *scn)
{
	uint32_t ce_drain_wait_cnt = 0;

	while (qdf_atomic_read(&scn->active_tasklet_cnt)) {
		if (++ce_drain_wait_cnt > HIF_CE_DRAIN_WAIT_CNT) {
			HIF_ERROR("%s: CE still not done with access",
			       __func__);

			return -EFAULT;
		}
		HIF_INFO("%s: Waiting for CE to finish access", __func__);
		msleep(10);
	}
	return 0;
}



#ifdef WLAN_SUSPEND_RESUME_TEST
static bool g_hif_apps_fake_suspended;
static hdd_fake_resume_callback hdd_fake_aps_resume;

static void hif_wlan_resume_work_handler(struct work_struct *work)
{
	hdd_fake_aps_resume(0);
}

static DECLARE_WORK(hif_resume_work, hif_wlan_resume_work_handler);

/**
 * hif_fake_apps_suspend(): Suspend WLAN
 *
 * Set the fake suspend flag such that hif knows that it will need
 * to fake the aps resume process using the hdd_fake_aps_resume
 *
 * Return: none
 */
void hif_fake_apps_suspend(hdd_fake_resume_callback callback)
{
	hdd_fake_aps_resume = callback;
	g_hif_apps_fake_suspended = true;
}

/**
 * hif_fake_aps_resume(): check if WLAN resume is needed
 *
 * Return: true if a fake apps resume has been been triggered
 *         returns false if regular interrupt processing is needed
 */
static bool hif_fake_aps_resume(void)
{
	if (g_hif_apps_fake_suspended) {
		g_hif_apps_fake_suspended = false;
		schedule_work(&hif_resume_work);
		return true;
	} else {
		return false;
	}
}

#else

/**
 * hif_fake_aps_resume(): check if WLAN resume is needed
 *
 * Return: always false if WLAN_SUSPEND_RESUME_TEST is not defined
 */
static bool hif_fake_aps_resume(void)
{
	return false;
}
#endif /* End of WLAN_SUSPEND_RESUME_TEST */

/**
 * hif_snoc_interrupt_handler() - hif_snoc_interrupt_handler
 * @irq: irq coming from kernel
 * @context: context
 *
 * Return: N/A
 */
static irqreturn_t hif_snoc_interrupt_handler(int irq, void *context)
{
	struct ce_tasklet_entry *tasklet_entry = context;
	struct hif_softc *scn = HIF_GET_SOFTC(tasklet_entry->hif_ce_state);

	return ce_dispatch_interrupt(pld_get_ce_id(scn->qdf_dev->dev, irq),
				     tasklet_entry);
}

/**
 * hif_ce_increment_interrupt_count() - update ce stats
 * @hif_ce_state: ce state
 * @ce_id: ce id
 *
 * Return: none
 */
static inline void
hif_ce_increment_interrupt_count(struct HIF_CE_state *hif_ce_state, int ce_id)
{
	int cpu_id = qdf_get_cpu();

	hif_ce_state->stats.ce_per_cpu[ce_id][cpu_id]++;
}

/**
 * hif_display_ce_stats() - display ce stats
 * @hif_ce_state: ce state
 *
 * Return: none
 */
void hif_display_ce_stats(struct HIF_CE_state *hif_ce_state)
{
#define STR_SIZE 128
	uint8_t i, j, pos;
	char str_buffer[STR_SIZE];
	int size, ret;

	qdf_print("CE interrupt statistics:");
	for (i = 0; i < CE_COUNT_MAX; i++) {
		size = STR_SIZE;
		pos = 0;
		qdf_print("CE id: %d", i);
		for (j = 0; j < QDF_MAX_AVAILABLE_CPU; j++) {
			ret = snprintf(str_buffer + pos, size, "[%d]: %d",
				j, hif_ce_state->stats.ce_per_cpu[i][j]);
			if (ret <= 0 || ret >= size)
				break;
			size -= ret;
			pos += ret;
		}
		qdf_print("%s", str_buffer);
	}
#undef STR_SIZE
}

/**
 * hif_clear_ce_stats() - clear ce stats
 * @hif_ce_state: ce state
 *
 * Return: none
 */
void hif_clear_ce_stats(struct HIF_CE_state *hif_ce_state)
{
	qdf_mem_zero(&hif_ce_state->stats, sizeof(struct ce_intr_stats));
}

/**
 * ce_dispatch_interrupt() - dispatch an interrupt to a processing context
 * @ce_id: ce_id
 * @tasklet_entry: context
 *
 * Return: N/A
 */
irqreturn_t ce_dispatch_interrupt(int ce_id,
				  struct ce_tasklet_entry *tasklet_entry)
{
	struct HIF_CE_state *hif_ce_state = tasklet_entry->hif_ce_state;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ce_state);
	struct hif_opaque_softc *hif_hdl = GET_HIF_OPAQUE_HDL(scn);

	if (tasklet_entry->ce_id != ce_id) {
		HIF_ERROR("%s: ce_id (expect %d, received %d) does not match",
			  __func__, tasklet_entry->ce_id, ce_id);
		return IRQ_NONE;
	}
	if (unlikely(ce_id >= CE_COUNT_MAX)) {
		HIF_ERROR("%s: ce_id=%d > CE_COUNT_MAX=%d",
			  __func__, tasklet_entry->ce_id, CE_COUNT_MAX);
		return IRQ_NONE;
	}
	hif_irq_disable(scn, ce_id);
	qdf_atomic_inc(&scn->active_tasklet_cnt);
	hif_record_ce_desc_event(scn, ce_id, HIF_IRQ_EVENT, NULL, NULL, 0);
	hif_ce_increment_interrupt_count(hif_ce_state, ce_id);

	if (unlikely(hif_fake_aps_resume())) {
		HIF_ERROR("received resume interrupt");
		hif_irq_enable(scn, ce_id);
		return IRQ_HANDLED;
	}

	if (hif_napi_enabled(hif_hdl, ce_id))
		hif_napi_schedule(hif_hdl, ce_id);
	else
		tasklet_schedule(&tasklet_entry->intr_tq);

	return IRQ_HANDLED;
}

/**
 * const char *ce_name
 *
 * @ce_name: ce_name
 */
const char *ce_name[] = {
	"WLAN_CE_0",
	"WLAN_CE_1",
	"WLAN_CE_2",
	"WLAN_CE_3",
	"WLAN_CE_4",
	"WLAN_CE_5",
	"WLAN_CE_6",
	"WLAN_CE_7",
	"WLAN_CE_8",
	"WLAN_CE_9",
	"WLAN_CE_10",
	"WLAN_CE_11",
};
/**
 * ce_unregister_irq() - ce_unregister_irq
 * @hif_ce_state: hif_ce_state copy engine device handle
 * @mask: which coppy engines to unregister for.
 *
 * Unregisters copy engine irqs matching mask.  If a 1 is set at bit x,
 * unregister for copy engine x.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ce_unregister_irq(struct HIF_CE_state *hif_ce_state, uint32_t mask)
{
	int id;
	int ce_count;
	int ret;
	struct hif_softc *scn;

	if (hif_ce_state == NULL) {
		HIF_WARN("%s: hif_ce_state = NULL", __func__);
		return QDF_STATUS_SUCCESS;
	}

	scn = HIF_GET_SOFTC(hif_ce_state);
	ce_count = scn->ce_count;

	for (id = 0; id < ce_count; id++) {
		if ((mask & (1 << id)) && hif_ce_state->tasklets[id].inited) {
			ret = pld_ce_free_irq(scn->qdf_dev->dev, id,
					&hif_ce_state->tasklets[id]);
			if (ret < 0)
				HIF_ERROR(
					"%s: pld_unregister_irq error - ce_id = %d, ret = %d",
					__func__, id, ret);
		}
	}
	return QDF_STATUS_SUCCESS;
}
/**
 * ce_register_irq() - ce_register_irq
 * @hif_ce_state: hif_ce_state
 * @mask: which coppy engines to unregister for.
 *
 * Registers copy engine irqs matching mask.  If a 1 is set at bit x,
 * Register for copy engine x.
 *
 * Return: QDF_STATUS
 */
QDF_STATUS ce_register_irq(struct HIF_CE_state *hif_ce_state, uint32_t mask)
{
	int id;
	int ce_count;
	int ret;
	unsigned long irqflags = IRQF_TRIGGER_RISING;
	uint32_t done_mask = 0;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ce_state);

	ce_count = scn->ce_count;

	for (id = 0; id < ce_count; id++) {
		if ((mask & (1 << id)) && hif_ce_state->tasklets[id].inited) {
			ret = pld_ce_request_irq(scn->qdf_dev->dev, id,
				hif_snoc_interrupt_handler,
				irqflags, ce_name[id],
				&hif_ce_state->tasklets[id]);
			if (ret) {
				HIF_ERROR(
					"%s: cannot register CE %d irq handler, ret = %d",
					__func__, id, ret);
				ce_unregister_irq(hif_ce_state, done_mask);
				return QDF_STATUS_E_FAULT;
			} else {
				done_mask |= 1 << id;
			}
		}
	}

	return QDF_STATUS_SUCCESS;
}
