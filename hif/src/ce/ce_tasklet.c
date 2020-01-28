/*
 * Copyright (c) 2015-2020 The Linux Foundation. All rights reserved.
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

	if (!scn) {
		HIF_ERROR("%s: tasklet scn is null", __func__);
		return;
	}

	hif_ce_state = HIF_GET_CE_STATE(scn);

	if (scn->hif_init_done == false) {
		HIF_ERROR("%s: wlan driver is unloaded", __func__);
		return;
	}
	if (hif_ce_state->tasklets[ce_work->id].inited)
		tasklet_schedule(&hif_ce_state->tasklets[ce_work->id].intr_tq);
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

/**
 * deinit_tasklet_workers() - deinit_tasklet_workers
 * @scn: HIF Context
 *
 * Return: N/A
 */
void deinit_tasklet_workers(struct hif_opaque_softc *scn)
{
	u32 id;

	for (id = 0; id < CE_ID_MAX; id++)
		cancel_work_sync(&tasklet_workers[id].work);

	work_initialized = false;
}

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

#ifdef CE_TASKLET_DEBUG_ENABLE
/**
 * hif_record_tasklet_exec_entry_ts() - Record ce tasklet execution
 *                                      entry time
 * @scn: hif_softc
 * @ce_id: ce_id
 *
 * Return: None
 */
static inline void
hif_record_tasklet_exec_entry_ts(struct hif_softc *scn, uint8_t ce_id)
{
	struct HIF_CE_state *hif_ce_state = HIF_GET_CE_STATE(scn);

	hif_ce_state->stats.tasklet_exec_entry_ts[ce_id] =
					qdf_get_log_timestamp_usecs();
}

/**
 * hif_record_tasklet_sched_entry_ts() - Record ce tasklet scheduled
 *                                       entry time
 * @scn: hif_softc
 * @ce_id: ce_id
 *
 * Return: None
 */
static inline void
hif_record_tasklet_sched_entry_ts(struct hif_softc *scn, uint8_t ce_id)
{
	struct HIF_CE_state *hif_ce_state = HIF_GET_CE_STATE(scn);

	hif_ce_state->stats.tasklet_sched_entry_ts[ce_id] =
					qdf_get_log_timestamp_usecs();
}

/**
 * hif_ce_latency_stats() - Display ce latency information
 * @hif_ctx: hif_softc struct
 *
 * Return: None
 */
static void
hif_ce_latency_stats(struct hif_softc *hif_ctx)
{
	uint8_t i, j;
	uint32_t index, start_index;
	static const char * const buck_str[] = {"0 - 0.5", "0.5 - 1", "1  -  2",
					       "2  -  5", "5  - 10", "  >  10"};
	struct HIF_CE_state *hif_ce_state = HIF_GET_CE_STATE(hif_ctx);
	struct ce_stats *stats = &hif_ce_state->stats;

	hif_err("\tCE TASKLET ARRIVAL AND EXECUTION STATS");
	for (i = 0; i < CE_COUNT_MAX; i++) {
		hif_nofl_err("\n\t\tCE Ring %d Tasklet Execution Bucket", i);
		for (j = 0; j < CE_BUCKET_MAX; j++) {
			hif_nofl_err("\t Bucket %sms :%llu\t last update:%llu",
				     buck_str[j],
				     stats->ce_tasklet_exec_bucket[i][j],
				     stats->ce_tasklet_exec_last_update[i][j]);
		}

		hif_nofl_err("\n\t\tCE Ring %d Tasklet Scheduled Bucket", i);
		for (j = 0; j < CE_BUCKET_MAX; j++) {
			hif_nofl_err("\t Bucket %sms :%llu\t last update :%lld",
				     buck_str[j],
				     stats->ce_tasklet_sched_bucket[i][j],
				     stats->
					   ce_tasklet_sched_last_update[i][j]);
		}

		hif_nofl_err("\n\t\t CE RING %d Last %d time records",
			     i, HIF_REQUESTED_EVENTS);
		index = stats->record_index[i];
		start_index = stats->record_index[i];

		for (j = 0; j < HIF_REQUESTED_EVENTS; j++) {
			hif_nofl_err("\t Execuiton time:  %luus Total Scheduled time: %luus",
				     stats->tasklet_exec_time_record[i][index],
				     stats->
					   tasklet_sched_time_record[i][index]);
			index = (index - 1) % HIF_REQUESTED_EVENTS;
			if (index == start_index)
				break;
		}
	}
}

/**
 * ce_tasklet_update_bucket() - update ce execution and scehduled time latency
 *                              in corresponding time buckets
 * @stats: struct ce_stats
 * @ce_id: ce_id_type
 * @entry_us: timestamp when tasklet is started to execute
 * @exit_us: timestamp when tasklet is completed execution
 *
 * Return: N/A
 */
static void ce_tasklet_update_bucket(struct HIF_CE_state *hif_ce_state,
				     uint8_t ce_id)
{
	uint32_t index;
	uint64_t exec_time, exec_ms;
	uint64_t sched_time, sched_ms;
	uint64_t curr_time = qdf_get_log_timestamp_usecs();
	struct ce_stats *stats = &hif_ce_state->stats;

	exec_time = curr_time - (stats->tasklet_exec_entry_ts[ce_id]);
	sched_time = (stats->tasklet_exec_entry_ts[ce_id]) -
		      (stats->tasklet_sched_entry_ts[ce_id]);

	index = stats->record_index[ce_id];
	index = (index + 1) % HIF_REQUESTED_EVENTS;

	stats->tasklet_exec_time_record[ce_id][index] = exec_time;
	stats->tasklet_sched_time_record[ce_id][index] = sched_time;
	stats->record_index[ce_id] = index;

	exec_ms = qdf_do_div(exec_time, 1000);
	sched_ms = qdf_do_div(sched_time, 1000);

	if (exec_ms > 10) {
		stats->ce_tasklet_exec_bucket[ce_id][CE_BUCKET_BEYOND]++;
		stats->ce_tasklet_exec_last_update[ce_id][CE_BUCKET_BEYOND]
								= curr_time;
	} else if (exec_ms > 5) {
		stats->ce_tasklet_exec_bucket[ce_id][CE_BUCKET_10_MS]++;
		stats->ce_tasklet_exec_last_update[ce_id][CE_BUCKET_10_MS]
								= curr_time;
	} else if (exec_ms > 2) {
		stats->ce_tasklet_exec_bucket[ce_id][CE_BUCKET_5_MS]++;
		stats->ce_tasklet_exec_last_update[ce_id][CE_BUCKET_5_MS]
								= curr_time;
	} else if (exec_ms > 1) {
		stats->ce_tasklet_exec_bucket[ce_id][CE_BUCKET_2_MS]++;
		stats->ce_tasklet_exec_last_update[ce_id][CE_BUCKET_2_MS]
								= curr_time;
	} else if (exec_time > 500) {
		stats->ce_tasklet_exec_bucket[ce_id][CE_BUCKET_1_MS]++;
		stats->ce_tasklet_exec_last_update[ce_id][CE_BUCKET_1_MS]
								= curr_time;
	} else {
		stats->ce_tasklet_exec_bucket[ce_id][CE_BUCKET_500_US]++;
		stats->ce_tasklet_exec_last_update[ce_id][CE_BUCKET_500_US]
								= curr_time;
	}

	if (sched_ms > 10) {
		stats->ce_tasklet_sched_bucket[ce_id][CE_BUCKET_BEYOND]++;
		stats->ce_tasklet_sched_last_update[ce_id][CE_BUCKET_BEYOND]
								= curr_time;
	} else if (sched_ms > 5) {
		stats->ce_tasklet_sched_bucket[ce_id][CE_BUCKET_10_MS]++;
		stats->ce_tasklet_sched_last_update[ce_id][CE_BUCKET_10_MS]
								= curr_time;
	} else if (sched_ms > 2) {
		stats->ce_tasklet_sched_bucket[ce_id][CE_BUCKET_5_MS]++;
		stats->ce_tasklet_sched_last_update[ce_id][CE_BUCKET_5_MS]
								= curr_time;
	} else if (sched_ms > 1) {
		stats->ce_tasklet_sched_bucket[ce_id][CE_BUCKET_2_MS]++;
		stats->ce_tasklet_sched_last_update[ce_id][CE_BUCKET_2_MS]
								= curr_time;
	} else if (sched_time > 500) {
		stats->ce_tasklet_sched_bucket[ce_id][CE_BUCKET_1_MS]++;
		stats->ce_tasklet_sched_last_update[ce_id][CE_BUCKET_1_MS]
								= curr_time;
	} else {
		stats->ce_tasklet_sched_bucket[ce_id][CE_BUCKET_500_US]++;
		stats->ce_tasklet_sched_last_update[ce_id][CE_BUCKET_500_US]
								= curr_time;
	}
}
#else
static inline void
hif_record_tasklet_exec_entry_ts(struct hif_softc *scn, uint8_t ce_id)
{
}

static void ce_tasklet_update_bucket(struct HIF_CE_state *hif_ce_state,
				     uint8_t ce_id)
{
}

static inline void
hif_record_tasklet_sched_entry_ts(struct hif_softc *scn, uint8_t ce_id)
{
}

static void
hif_ce_latency_stats(struct hif_softc *hif_ctx)
{
}
#endif /*CE_TASKLET_DEBUG_ENABLE*/

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

	if (scn->ce_latency_stats)
		hif_record_tasklet_exec_entry_ts(scn, tasklet_entry->ce_id);

	hif_record_ce_desc_event(scn, tasklet_entry->ce_id,
				 HIF_CE_TASKLET_ENTRY, NULL, NULL, -1, 0);

	if (qdf_atomic_read(&scn->link_suspended)) {
		HIF_ERROR("%s: ce %d tasklet fired after link suspend.",
				__func__, tasklet_entry->ce_id);
		QDF_BUG(0);
	}

	ce_per_engine_service(scn, tasklet_entry->ce_id);

	if (ce_check_rx_pending(CE_state) && tasklet_entry->inited) {
		/*
		 * There are frames pending, schedule tasklet to process them.
		 * Enable the interrupt only when there is no pending frames in
		 * any of the Copy Engine pipes.
		 */
		hif_record_ce_desc_event(scn, tasklet_entry->ce_id,
				HIF_CE_TASKLET_RESCHEDULE, NULL, NULL, -1, 0);

		ce_schedule_tasklet(tasklet_entry);
		return;
	}

	if (scn->target_status != TARGET_STATUS_RESET)
		hif_irq_enable(scn, tasklet_entry->ce_id);

	hif_record_ce_desc_event(scn, tasklet_entry->ce_id, HIF_CE_TASKLET_EXIT,
				NULL, NULL, -1, 0);

	if (scn->ce_latency_stats)
		ce_tasklet_update_bucket(hif_ce_state, tasklet_entry->ce_id);

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
 * Context: Non-Atomic context
 * Return: N/A
 */
void ce_tasklet_kill(struct hif_softc *scn)
{
	int i;
	struct HIF_CE_state *hif_ce_state = HIF_GET_CE_STATE(scn);

	work_initialized = false;

	for (i = 0; i < CE_COUNT_MAX; i++) {
		if (hif_ce_state->tasklets[i].inited) {
			hif_ce_state->tasklets[i].inited = false;
			/*
			 * Cancel the tasklet work before tasklet_disable
			 * to avoid race between tasklet_schedule and
			 * tasklet_kill. Here cancel_work_sync() won't
			 * return before reschedule_ce_tasklet_work_handler()
			 * completes. Even if tasklet_schedule() happens
			 * tasklet_disable() will take care of that.
			 */
			cancel_work_sync(&tasklet_workers[i].work);
			tasklet_kill(&hif_ce_state->tasklets[i].intr_tq);
		}
	}
	qdf_atomic_set(&scn->active_tasklet_cnt, 0);
}

#define HIF_CE_DRAIN_WAIT_CNT          20
/**
 * hif_drain_tasklets(): wait until no tasklet is pending
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
	int32_t tasklet_cnt;

	while ((tasklet_cnt = qdf_atomic_read(&scn->active_tasklet_cnt))) {
		if (++ce_drain_wait_cnt > HIF_CE_DRAIN_WAIT_CNT) {
			HIF_ERROR("%s: CE still not done with access: %d",
				  __func__, tasklet_cnt);

			return -EFAULT;
		}
		HIF_INFO("%s: Waiting for CE to finish access", __func__);
		msleep(10);
	}
	return 0;
}

#ifdef WLAN_SUSPEND_RESUME_TEST
/**
 * hif_interrupt_is_ut_resume(): Tests if an irq on the given copy engine should
 *	trigger a unit-test resume.
 * @scn: The HIF context to operate on
 * @ce_id: The copy engine Id from the originating interrupt
 *
 * Return: true if the raised irq should trigger a unit-test resume
 */
static bool hif_interrupt_is_ut_resume(struct hif_softc *scn, int ce_id)
{
	int errno;
	uint8_t wake_ce_id;

	if (!hif_is_ut_suspended(scn))
		return false;

	/* ensure passed ce_id matches wake ce_id */
	errno = hif_get_wake_ce_id(scn, &wake_ce_id);
	if (errno) {
		HIF_ERROR("%s: failed to get wake CE Id: %d", __func__, errno);
		return false;
	}

	return ce_id == wake_ce_id;
}
#else
static inline bool
hif_interrupt_is_ut_resume(struct hif_softc *scn, int ce_id)
{
	return false;
}
#endif /* WLAN_SUSPEND_RESUME_TEST */

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
void hif_display_ce_stats(struct hif_softc *hif_ctx)
{
#define STR_SIZE 128
	uint8_t i, j, pos;
	char str_buffer[STR_SIZE];
	int size, ret;
	struct HIF_CE_state *hif_ce_state = HIF_GET_CE_STATE(hif_ctx);

	qdf_debug("CE interrupt statistics:");
	for (i = 0; i < CE_COUNT_MAX; i++) {
		size = STR_SIZE;
		pos = 0;
		for (j = 0; j < QDF_MAX_AVAILABLE_CPU; j++) {
			ret = snprintf(str_buffer + pos, size, "[%d]:%d ",
				       j, hif_ce_state->stats.ce_per_cpu[i][j]);
			if (ret <= 0 || ret >= size)
				break;
			size -= ret;
			pos += ret;
		}
		qdf_debug("CE id[%2d] - %s", i, str_buffer);
	}

	if (hif_ctx->ce_latency_stats)
		hif_ce_latency_stats(hif_ctx);
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
	qdf_mem_zero(&hif_ce_state->stats, sizeof(struct ce_stats));
}

/**
 * hif_tasklet_schedule() - schedule tasklet
 * @hif_ctx: hif context
 * @tasklet_entry: ce tasklet entry
 *
 * Return: false if tasklet already scheduled, otherwise true
 */
static inline bool hif_tasklet_schedule(struct hif_opaque_softc *hif_ctx,
					struct ce_tasklet_entry *tasklet_entry)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	if (test_bit(TASKLET_STATE_SCHED, &tasklet_entry->intr_tq.state)) {
		HIF_DBG("tasklet scheduled, return");
		qdf_atomic_dec(&scn->active_tasklet_cnt);
		return false;
	}

	tasklet_schedule(&tasklet_entry->intr_tq);
	if (scn->ce_latency_stats)
		hif_record_tasklet_sched_entry_ts(scn, tasklet_entry->ce_id);

	return true;
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

	if (!TARGET_REGISTER_ACCESS_ALLOWED(scn))
		return IRQ_HANDLED;

	hif_record_ce_desc_event(scn, ce_id, HIF_IRQ_EVENT,
				NULL, NULL, 0, 0);
	hif_ce_increment_interrupt_count(hif_ce_state, ce_id);

	if (unlikely(hif_interrupt_is_ut_resume(scn, ce_id))) {
		hif_ut_fw_resume(scn);
		hif_irq_enable(scn, ce_id);
		return IRQ_HANDLED;
	}

	qdf_atomic_inc(&scn->active_tasklet_cnt);

	if (hif_napi_enabled(hif_hdl, ce_id))
		hif_napi_schedule(hif_hdl, ce_id);
	else
		hif_tasklet_schedule(hif_hdl, tasklet_entry);

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

	if (!hif_ce_state) {
		HIF_WARN("%s: hif_ce_state = NULL", __func__);
		return QDF_STATUS_SUCCESS;
	}

	scn = HIF_GET_SOFTC(hif_ce_state);
	ce_count = scn->ce_count;
	/* we are removing interrupts, so better stop NAPI */
	ret = hif_napi_event(GET_HIF_OPAQUE_HDL(scn),
			     NAPI_EVT_INT_STATE, (void *)0);
	if (ret != 0)
		HIF_ERROR("%s: napi_event INT_STATE returned %d",
			  __func__, ret);
	/* this is not fatal, continue */

	/* filter mask to free only for ce's with irq registered */
	mask &= hif_ce_state->ce_register_irq_done;
	for (id = 0; id < ce_count; id++) {
		if ((mask & (1 << id)) && hif_ce_state->tasklets[id].inited) {
			ret = pld_ce_free_irq(scn->qdf_dev->dev, id,
					&hif_ce_state->tasklets[id]);
			if (ret < 0)
				HIF_ERROR(
					"%s: pld_unregister_irq error - ce_id = %d, ret = %d",
					__func__, id, ret);
		}
		ce_disable_polling(scn->ce_id_to_state[id]);
	}
	hif_ce_state->ce_register_irq_done &= ~mask;

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
			}
			done_mask |= 1 << id;
		}
	}
	hif_ce_state->ce_register_irq_done |= done_mask;

	return QDF_STATUS_SUCCESS;
}
