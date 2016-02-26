/*
 * Copyright (c) 2015 The Linux Foundation. All rights reserved.
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

#include <osdep.h>
#include <linux/pci.h>
#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/if_arp.h>
#include "a_types.h"
#include "athdefs.h"
#include "cdf_lock.h"
#include "cdf_types.h"
#include "cdf_status.h"
#include "cds_api.h"
#include "regtable.h"
#include "hif.h"
#include "hif_io32.h"
#include "ce_main.h"
#include "ce_api.h"
#include "ce_reg.h"
#include "ce_internal.h"
#ifdef CONFIG_CNSS
#include <net/cnss.h>
#ifdef HIF_PCI
#include "icnss_stub.h"
#else
#include <soc/qcom/icnss.h>
#endif /* HIF_PCI */
#endif
#include "hif_debug.h"
#include "hif_napi.h"


/**
 * ce_irq_status() - read CE IRQ status
 * @scn: struct ol_softc
 * @ce_id: ce_id
 * @host_status: host_status
 *
 * Return: IRQ status
 */
static inline void ce_irq_status(struct ol_softc *scn,
	int ce_id, uint32_t *host_status)
{
	uint32_t offset = HOST_IS_ADDRESS + CE_BASE_ADDRESS(ce_id);

	*host_status = hif_read32_mb(scn->mem + offset);
}

/**
 * reschedule_ce_tasklet_work_handler() - reschedule work
 * @ce_id: ce_id
 *
 * Return: N/A
 */
static void reschedule_ce_tasklet_work_handler(int ce_id)
{
	struct ol_softc *scn = cds_get_context(CDF_MODULE_ID_HIF);
	struct HIF_CE_state *hif_ce_state;

	if (NULL == scn) {
		HIF_ERROR("%s: tasklet scn is null", __func__);
		return;
	}
	hif_ce_state = (struct HIF_CE_state *)scn->hif_hdl;

	if (scn->hif_init_done == false) {
		HIF_ERROR("%s: wlan driver is unloaded", __func__);
		return;
	}
	tasklet_schedule(&hif_ce_state->tasklets[ce_id].intr_tq);
	return;
}

/**
 * struct tasklet_work
 *
 * @id: ce_id
 * @work: work
 */
struct tasklet_work {
	enum ce_id_type id;
	struct work_struct work;
};

static struct tasklet_work tasklet_workers[CE_ID_MAX];
static bool work_initialized;

/**
 * work_handler() - work_handler
 * @work: struct work_struct
 *
 * Return: N/A
 */
static void work_handler(struct work_struct *work)
{
	struct tasklet_work *tmp;

	tmp = container_of(work, struct tasklet_work, work);
	reschedule_ce_tasklet_work_handler(tmp->id);
}

/**
 * init_tasklet_work() - init_tasklet_work
 * @work: struct work_struct
 * @work_handler: work_handler
 *
 * Return: N/A
 */
#ifdef CONFIG_CNSS
static void init_tasklet_work(struct work_struct *work,
	work_func_t work_handler)
{
	cnss_init_work(work, work_handler);
}
#else
static void init_tasklet_work(struct work_struct *work,
	work_func_t work_handler)
{
	INIT_WORK(work, work_handler);
}
#endif

/**
 * init_tasklet_workers() - init_tasklet_workers
 *
 * Return: N/A
 */
void init_tasklet_workers(void)
{
	uint32_t id;

	for (id = 0; id < CE_ID_MAX; id++) {
		tasklet_workers[id].id = id;
		init_tasklet_work(&tasklet_workers[id].work, work_handler);
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
	struct ol_softc *scn = hif_ce_state->scn;
	struct CE_state *CE_state = scn->ce_id_to_state[tasklet_entry->ce_id];

	if (cdf_atomic_read(&scn->link_suspended)) {
		HIF_ERROR("%s: ce %d tasklet fired after link suspend.",
				__func__, tasklet_entry->ce_id);
		CDF_BUG(0);
	}

	ce_per_engine_service(scn, tasklet_entry->ce_id);

	if (CE_state->lro_flush_cb != NULL) {
		CE_state->lro_flush_cb(CE_state->lro_data);
	}

	if (ce_check_rx_pending(scn, tasklet_entry->ce_id)) {
		/*
		 * There are frames pending, schedule tasklet to process them.
		 * Enable the interrupt only when there is no pending frames in
		 * any of the Copy Engine pipes.
		 */
		ce_schedule_tasklet(tasklet_entry);
		return;
	}

	if (scn->target_status != OL_TRGET_STATUS_RESET)
		ce_irq_enable(scn, tasklet_entry->ce_id);

	cdf_atomic_dec(&scn->active_tasklet_cnt);
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
void ce_tasklet_kill(struct HIF_CE_state *hif_ce_state)
{
	int i;
	struct ol_softc *scn = hif_ce_state->scn;

	for (i = 0; i < CE_COUNT_MAX; i++)
		if (hif_ce_state->tasklets[i].inited) {
			tasklet_kill(&hif_ce_state->tasklets[i].intr_tq);
			hif_ce_state->tasklets[i].inited = false;
		}
	cdf_atomic_set(&scn->active_tasklet_cnt, 0);
}
/**
 * ce_irq_handler() - ce_irq_handler
 * @ce_id: ce_id
 * @context: context
 *
 * Return: N/A
 */
static irqreturn_t ce_irq_handler(int irq, void *context)
{
	struct ce_tasklet_entry *tasklet_entry = context;
	struct HIF_CE_state *hif_ce_state = tasklet_entry->hif_ce_state;
	struct ol_softc *scn = hif_ce_state->scn;
	uint32_t host_status;
	int ce_id = icnss_get_ce_id(irq);

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
#ifndef HIF_PCI
	disable_irq_nosync(irq);
#endif
	ce_irq_disable(scn, ce_id);
	ce_irq_status(scn, ce_id, &host_status);
	cdf_atomic_inc(&scn->active_tasklet_cnt);
	if (hif_napi_enabled(scn, ce_id))
		hif_napi_schedule(scn, ce_id);
	else
		tasklet_schedule(&tasklet_entry->intr_tq);

	return IRQ_HANDLED;
}

/**
 * const char *ce_name
 *
 * @ce_name: ce_name
 */
const char *ce_name[ICNSS_MAX_IRQ_REGISTRATIONS] = {
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
 * Return: CDF_STATUS
 */
CDF_STATUS ce_unregister_irq(struct HIF_CE_state *hif_ce_state, uint32_t mask)
{
	int id;
	int ret;

	if (hif_ce_state == NULL) {
		HIF_WARN("%s: hif_ce_state = NULL", __func__);
		return CDF_STATUS_SUCCESS;
	}
	for (id = 0; id < CE_COUNT_MAX; id++) {
		if ((mask & (1 << id)) && hif_ce_state->tasklets[id].inited) {
			ret = icnss_ce_free_irq(id,
					&hif_ce_state->tasklets[id]);
			if (ret < 0)
				HIF_ERROR(
					"%s: icnss_unregister_irq error - ce_id = %d, ret = %d",
					__func__, id, ret);
		}
	}
	return CDF_STATUS_SUCCESS;
}
/**
 * ce_register_irq() - ce_register_irq
 * @hif_ce_state: hif_ce_state
 * @mask: which coppy engines to unregister for.
 *
 * Registers copy engine irqs matching mask.  If a 1 is set at bit x,
 * Register for copy engine x.
 *
 * Return: CDF_STATUS
 */
CDF_STATUS ce_register_irq(struct HIF_CE_state *hif_ce_state, uint32_t mask)
{
	int id;
	int ret;
	unsigned long irqflags = IRQF_TRIGGER_RISING;
	uint32_t done_mask = 0;

	for (id = 0; id < CE_COUNT_MAX; id++) {
		if ((mask & (1 << id)) && hif_ce_state->tasklets[id].inited) {
			ret = icnss_ce_request_irq(id, ce_irq_handler,
				irqflags, ce_name[id],
				&hif_ce_state->tasklets[id]);
			if (ret) {
				HIF_ERROR(
					"%s: cannot register CE %d irq handler, ret = %d",
					__func__, id, ret);
				ce_unregister_irq(hif_ce_state, done_mask);
				return CDF_STATUS_E_FAULT;
			} else {
				done_mask |= 1 << id;
			}
		}
	}

#ifndef HIF_PCI
	/* move to hif_configure_irq */
	ce_enable_irq_in_group_reg(hif_ce_state->scn, done_mask);
#endif

	return CDF_STATUS_SUCCESS;
}
