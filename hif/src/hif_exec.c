/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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

#include <hif_exec.h>
#include <ce_main.h>
#include "qdf_module.h"
#include "qdf_net_if.h"
/* mapping NAPI budget 0 to internal budget 0
 * NAPI budget 1 to internal budget [1,scaler -1]
 * NAPI budget 2 to internal budget [scaler, 2 * scaler - 1], etc
 */
#define NAPI_BUDGET_TO_INTERNAL_BUDGET(n, s) \
	(((n) << (s)) - 1)
#define INTERNAL_BUDGET_TO_NAPI_BUDGET(n, s) \
	(((n) + 1) >> (s))

static struct hif_exec_context *hif_exec_tasklet_create(void);

#ifdef WLAN_FEATURE_DP_EVENT_HISTORY
struct hif_event_history hif_event_desc_history[HIF_NUM_INT_CONTEXTS];

static inline
int hif_get_next_record_index(qdf_atomic_t *table_index,
			      int array_size)
{
	int record_index = qdf_atomic_inc_return(table_index);

	return record_index & (array_size - 1);
}

void hif_hist_record_event(struct hif_opaque_softc *hif_ctx,
			   struct hif_event_record *event, uint8_t intr_grp_id)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct hif_exec_context *hif_ext_group;
	struct hif_event_history *hist_ev;
	struct hif_event_record *record;
	int record_index;

	if (!hif_state->hif_num_extgroup)
		return;

	if (scn->event_disable_mask & BIT(event->type))
		return;

	if (intr_grp_id >= HIF_NUM_INT_CONTEXTS) {
		hif_err("Invalid interrupt group id %d", intr_grp_id);
		return;
	}

	hif_ext_group = hif_state->hif_ext_group[intr_grp_id];
	hist_ev = hif_ext_group->evt_hist;

	record_index = hif_get_next_record_index(
			&hist_ev->index, HIF_EVENT_HIST_MAX);

	record = &hist_ev->event[record_index];

	record->hal_ring_id = event->hal_ring_id;
	record->hp = event->hp;
	record->tp = event->tp;
	record->cpu_id = qdf_get_cpu();
	record->timestamp = qdf_get_log_timestamp();
	record->type = event->type;
}

static void hif_event_history_init(struct hif_exec_context *hif_ext_grp)
{
	hif_ext_grp->evt_hist = &hif_event_desc_history[hif_ext_grp->grp_id];
	qdf_atomic_set(&hif_ext_grp->evt_hist->index, -1);
}
#else
static inline void hif_event_history_init(struct hif_exec_context *hif_ext_grp)
{
}
#endif /* WLAN_FEATURE_DP_EVENT_HISTORY */

/**
 * hif_print_napi_latency_stats() - print NAPI scheduling latency stats
 * @hif_state: hif context
 *
 * return: void
 */
#ifdef HIF_LATENCY_PROFILE_ENABLE
static void hif_print_napi_latency_stats(struct HIF_CE_state *hif_state)
{
	struct hif_exec_context *hif_ext_group;
	int i, j;
	int64_t cur_tstamp;

	const char time_str[HIF_SCHED_LATENCY_BUCKETS][15] =  {
		"0-2   ms",
		"3-10  ms",
		"11-20 ms",
		"21-50 ms",
		"51-100 ms",
		"101-250 ms",
		"251-500 ms",
		"> 500 ms"
	};

	cur_tstamp = qdf_ktime_to_ms(qdf_ktime_get());

	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_FATAL,
		  "Current timestamp: %lld", cur_tstamp);

	for (i = 0; i < hif_state->hif_num_extgroup; i++) {
		if (hif_state->hif_ext_group[i]) {
			hif_ext_group = hif_state->hif_ext_group[i];

			QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_FATAL,
				  "Interrupts in the HIF Group");

			for (j = 0; j < hif_ext_group->numirq; j++) {
				QDF_TRACE(QDF_MODULE_ID_HIF,
					  QDF_TRACE_LEVEL_FATAL,
					  "  %s",
					  hif_ext_group->irq_name
					  (hif_ext_group->irq[j]));
			}

			QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_FATAL,
				  "Last serviced timestamp: %lld",
				  hif_ext_group->tstamp);

			QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_FATAL,
				  "Latency Bucket     | Time elapsed");

			for (j = 0; j < HIF_SCHED_LATENCY_BUCKETS; j++) {
				QDF_TRACE(QDF_MODULE_ID_HIF,
					  QDF_TRACE_LEVEL_FATAL,
					  "%s     |    %lld", time_str[j],
					  hif_ext_group->
					  sched_latency_stats[j]);
			}
		}
	}
}
#else
static void hif_print_napi_latency_stats(struct HIF_CE_state *hif_state)
{
}
#endif

/**
 * hif_clear_napi_stats() - reset NAPI stats
 * @hif_ctx: hif context
 *
 * return: void
 */
void hif_clear_napi_stats(struct hif_opaque_softc *hif_ctx)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);
	struct hif_exec_context *hif_ext_group;
	size_t i;

	for (i = 0; i < hif_state->hif_num_extgroup; i++) {
		hif_ext_group = hif_state->hif_ext_group[i];

		if (!hif_ext_group)
			return;

		qdf_mem_set(hif_ext_group->sched_latency_stats,
			    sizeof(hif_ext_group->sched_latency_stats),
			    0x0);
	}
}

qdf_export_symbol(hif_clear_napi_stats);

#ifdef WLAN_FEATURE_RX_SOFTIRQ_TIME_LIMIT
/**
 * hif_get_poll_times_hist_str() - Get HIF poll times histogram string
 * @stats: NAPI stats to get poll time buckets
 * @buf: buffer to fill histogram string
 * @buf_len: length of the buffer
 *
 * Return: void
 */
static void hif_get_poll_times_hist_str(struct qca_napi_stat *stats, char *buf,
					uint8_t buf_len)
{
	int i;
	int str_index = 0;

	for (i = 0; i < QCA_NAPI_NUM_BUCKETS; i++)
		str_index += qdf_scnprintf(buf + str_index, buf_len - str_index,
					   "%u|", stats->poll_time_buckets[i]);
}

/**
 * hif_exec_fill_poll_time_histogram() - fills poll time histogram for a NAPI
 * @hif_ext_group: hif_ext_group of type NAPI
 *
 * The function is called at the end of a NAPI poll to calculate poll time
 * buckets.
 *
 * Return: void
 */
static
void hif_exec_fill_poll_time_histogram(struct hif_exec_context *hif_ext_group)
{
	struct qca_napi_stat *napi_stat;
	unsigned long long poll_time_ns;
	uint32_t poll_time_us;
	uint32_t bucket_size_us = 500;
	uint32_t bucket;
	uint32_t cpu_id = qdf_get_cpu();

	poll_time_ns = sched_clock() - hif_ext_group->poll_start_time;
	poll_time_us = qdf_do_div(poll_time_ns, 1000);

	napi_stat = &hif_ext_group->stats[cpu_id];
	if (poll_time_ns > hif_ext_group->stats[cpu_id].napi_max_poll_time)
		hif_ext_group->stats[cpu_id].napi_max_poll_time = poll_time_ns;

	bucket = poll_time_us / bucket_size_us;
	if (bucket >= QCA_NAPI_NUM_BUCKETS)
		bucket = QCA_NAPI_NUM_BUCKETS - 1;
	++napi_stat->poll_time_buckets[bucket];
}

/**
 * hif_exec_poll_should_yield() - Local function deciding if NAPI should yield
 * @hif_ext_group: hif_ext_group of type NAPI
 *
 * Return: true if NAPI needs to yield, else false
 */
static bool hif_exec_poll_should_yield(struct hif_exec_context *hif_ext_group)
{
	bool time_limit_reached = false;
	unsigned long long poll_time_ns;
	int cpu_id = qdf_get_cpu();
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ext_group->hif);
	struct hif_config_info *cfg = &scn->hif_config;

	poll_time_ns = sched_clock() - hif_ext_group->poll_start_time;
	time_limit_reached =
		poll_time_ns > cfg->rx_softirq_max_yield_duration_ns ? 1 : 0;

	if (time_limit_reached) {
		hif_ext_group->stats[cpu_id].time_limit_reached++;
		hif_ext_group->force_break = true;
	}

	return time_limit_reached;
}

bool hif_exec_should_yield(struct hif_opaque_softc *hif_ctx, uint grp_id)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct hif_exec_context *hif_ext_group;
	bool ret_val = false;

	if (!(grp_id < hif_state->hif_num_extgroup) ||
	    !(grp_id < HIF_MAX_GROUP))
		return false;

	hif_ext_group = hif_state->hif_ext_group[grp_id];

	if (hif_ext_group->type == HIF_EXEC_NAPI_TYPE)
		ret_val = hif_exec_poll_should_yield(hif_ext_group);

	return ret_val;
}

/**
 * hif_exec_update_service_start_time() - Update NAPI poll start time
 * @hif_ext_group: hif_ext_group of type NAPI
 *
 * The function is called at the beginning of a NAPI poll to record the poll
 * start time.
 *
 * Return: None
 */
static inline
void hif_exec_update_service_start_time(struct hif_exec_context *hif_ext_group)
{
	hif_ext_group->poll_start_time = sched_clock();
}

void hif_print_napi_stats(struct hif_opaque_softc *hif_ctx)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);
	struct hif_exec_context *hif_ext_group;
	struct qca_napi_stat *napi_stats;
	int i, j;

	/*
	 * Max value of uint_32 (poll_time_bucket) = 4294967295
	 * Thus we need 10 chars + 1 space =11 chars for each bucket value.
	 * +1 space for '\0'.
	 */
	char hist_str[(QCA_NAPI_NUM_BUCKETS * 11) + 1] = {'\0'};

	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_ERROR,
		  "NAPI[#]CPU[#] |scheds |polls  |comps  |dones  |t-lim  |max(us)|hist(500us buckets)");

	for (i = 0;
	     (i < hif_state->hif_num_extgroup && hif_state->hif_ext_group[i]);
	     i++) {
		hif_ext_group = hif_state->hif_ext_group[i];
		for (j = 0; j < num_possible_cpus(); j++) {
			napi_stats = &hif_ext_group->stats[j];
			if (!napi_stats->napi_schedules)
				continue;

			hif_get_poll_times_hist_str(napi_stats,
						    hist_str,
						    sizeof(hist_str));
			QDF_TRACE(QDF_MODULE_ID_HIF,
				  QDF_TRACE_LEVEL_ERROR,
				  "NAPI[%d]CPU[%d]: %7u %7u %7u %7u %7u %7llu %s",
				  i, j,
				  napi_stats->napi_schedules,
				  napi_stats->napi_polls,
				  napi_stats->napi_completes,
				  napi_stats->napi_workdone,
				  napi_stats->time_limit_reached,
				  qdf_do_div(napi_stats->napi_max_poll_time,
					     1000),
				  hist_str);
		}
	}

	hif_print_napi_latency_stats(hif_state);
}

qdf_export_symbol(hif_print_napi_stats);

#else

static inline
void hif_get_poll_times_hist_str(struct qca_napi_stat *stats, char *buf,
				 uint8_t buf_len)
{
}

static inline
void hif_exec_update_service_start_time(struct hif_exec_context *hif_ext_group)
{
}

static inline
void hif_exec_fill_poll_time_histogram(struct hif_exec_context *hif_ext_group)
{
}

void hif_print_napi_stats(struct hif_opaque_softc *hif_ctx)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);
	struct hif_exec_context *hif_ext_group;
	struct qca_napi_stat *napi_stats;
	int i, j;

	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_FATAL,
		"NAPI[#ctx]CPU[#] |schedules |polls |completes |workdone");

	for (i = 0; i < hif_state->hif_num_extgroup; i++) {
		if (hif_state->hif_ext_group[i]) {
			hif_ext_group = hif_state->hif_ext_group[i];
			for (j = 0; j < num_possible_cpus(); j++) {
				napi_stats = &(hif_ext_group->stats[j]);
				if (napi_stats->napi_schedules != 0)
					QDF_TRACE(QDF_MODULE_ID_HIF,
						QDF_TRACE_LEVEL_FATAL,
						"NAPI[%2d]CPU[%d]: "
						"%7d %7d %7d %7d ",
						i, j,
						napi_stats->napi_schedules,
						napi_stats->napi_polls,
						napi_stats->napi_completes,
						napi_stats->napi_workdone);
			}
		}
	}

	hif_print_napi_latency_stats(hif_state);
}
qdf_export_symbol(hif_print_napi_stats);
#endif /* WLAN_FEATURE_RX_SOFTIRQ_TIME_LIMIT */

static void hif_exec_tasklet_schedule(struct hif_exec_context *ctx)
{
	struct hif_tasklet_exec_context *t_ctx = hif_exec_get_tasklet(ctx);

	tasklet_schedule(&t_ctx->tasklet);
}

/**
 * hif_exec_tasklet() - grp tasklet
 * data: context
 *
 * return: void
 */
static void hif_exec_tasklet_fn(unsigned long data)
{
	struct hif_exec_context *hif_ext_group =
			(struct hif_exec_context *)data;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ext_group->hif);
	unsigned int work_done;

	work_done =
		hif_ext_group->handler(hif_ext_group->context, HIF_MAX_BUDGET);

	if (hif_ext_group->work_complete(hif_ext_group, work_done)) {
		qdf_atomic_dec(&(scn->active_grp_tasklet_cnt));
		hif_ext_group->irq_enable(hif_ext_group);
	} else {
		hif_exec_tasklet_schedule(hif_ext_group);
	}
}

/**
 * hif_latency_profile_measure() - calculate latency and update histogram
 * hif_ext_group: hif exec context
 *
 * return: None
 */
#ifdef HIF_LATENCY_PROFILE_ENABLE
static void hif_latency_profile_measure(struct hif_exec_context *hif_ext_group)
{
	int64_t cur_tstamp;
	int64_t time_elapsed;

	cur_tstamp = qdf_ktime_to_ms(qdf_ktime_get());

	if (cur_tstamp > hif_ext_group->tstamp)
		time_elapsed = (cur_tstamp - hif_ext_group->tstamp);
	else
		time_elapsed = ~0x0 - (hif_ext_group->tstamp - cur_tstamp);

	hif_ext_group->tstamp = cur_tstamp;

	if (time_elapsed <= HIF_SCHED_LATENCY_BUCKET_0_2)
		hif_ext_group->sched_latency_stats[0]++;
	else if (time_elapsed <= HIF_SCHED_LATENCY_BUCKET_3_10)
		hif_ext_group->sched_latency_stats[1]++;
	else if (time_elapsed <= HIF_SCHED_LATENCY_BUCKET_11_20)
		hif_ext_group->sched_latency_stats[2]++;
	else if (time_elapsed <= HIF_SCHED_LATENCY_BUCKET_21_50)
		hif_ext_group->sched_latency_stats[3]++;
	else if (time_elapsed <= HIF_SCHED_LATENCY_BUCKET_51_100)
		hif_ext_group->sched_latency_stats[4]++;
	else if (time_elapsed <= HIF_SCHED_LATENCY_BUCKET_101_250)
		hif_ext_group->sched_latency_stats[5]++;
	else if (time_elapsed <= HIF_SCHED_LATENCY_BUCKET_251_500)
		hif_ext_group->sched_latency_stats[6]++;
	else
		hif_ext_group->sched_latency_stats[7]++;
}
#else
static inline
void hif_latency_profile_measure(struct hif_exec_context *hif_ext_group)
{
}
#endif

/**
 * hif_latency_profile_start() - Update the start timestamp for HIF ext group
 * hif_ext_group: hif exec context
 *
 * return: None
 */
#ifdef HIF_LATENCY_PROFILE_ENABLE
static void hif_latency_profile_start(struct hif_exec_context *hif_ext_group)
{
	hif_ext_group->tstamp = qdf_ktime_to_ms(qdf_ktime_get());
}
#else
static inline
void hif_latency_profile_start(struct hif_exec_context *hif_ext_group)
{
}
#endif

#ifdef FEATURE_NAPI
/**
 * hif_exec_poll() - napi poll
 * napi: napi struct
 * budget: budget for napi
 *
 * Return: mapping of internal budget to napi
 */
static int hif_exec_poll(struct napi_struct *napi, int budget)
{
	struct hif_napi_exec_context *napi_exec_ctx =
		    qdf_container_of(napi, struct hif_napi_exec_context, napi);
	struct hif_exec_context *hif_ext_group = &napi_exec_ctx->exec_ctx;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ext_group->hif);
	int work_done;
	int normalized_budget = 0;
	int actual_dones;
	int shift = hif_ext_group->scale_bin_shift;
	int cpu = smp_processor_id();

	hif_record_event(hif_ext_group->hif, hif_ext_group->grp_id,
			 0, 0, 0, HIF_EVENT_BH_SCHED);

	hif_ext_group->force_break = false;
	hif_exec_update_service_start_time(hif_ext_group);

	if (budget)
		normalized_budget = NAPI_BUDGET_TO_INTERNAL_BUDGET(budget, shift);

	hif_latency_profile_measure(hif_ext_group);

	work_done = hif_ext_group->handler(hif_ext_group->context,
					   normalized_budget);

	actual_dones = work_done;

	if (!hif_ext_group->force_break && work_done < normalized_budget) {
		napi_complete(napi);
		qdf_atomic_dec(&scn->active_grp_tasklet_cnt);
		hif_ext_group->irq_enable(hif_ext_group);
		hif_ext_group->stats[cpu].napi_completes++;
	} else {
		/* if the ext_group supports time based yield, claim full work
		 * done anyways */
		work_done = normalized_budget;
	}

	hif_ext_group->stats[cpu].napi_polls++;
	hif_ext_group->stats[cpu].napi_workdone += actual_dones;

	/* map internal budget to NAPI budget */
	if (work_done)
		work_done = INTERNAL_BUDGET_TO_NAPI_BUDGET(work_done, shift);

	hif_exec_fill_poll_time_histogram(hif_ext_group);

	return work_done;
}

/**
 * hif_exec_napi_schedule() - schedule the napi exec instance
 * @ctx: a hif_exec_context known to be of napi type
 */
static void hif_exec_napi_schedule(struct hif_exec_context *ctx)
{
	struct hif_napi_exec_context *n_ctx = hif_exec_get_napi(ctx);
	ctx->stats[smp_processor_id()].napi_schedules++;

	napi_schedule(&n_ctx->napi);
}

/**
 * hif_exec_napi_kill() - stop a napi exec context from being rescheduled
 * @ctx: a hif_exec_context known to be of napi type
 */
static void hif_exec_napi_kill(struct hif_exec_context *ctx)
{
	struct hif_napi_exec_context *n_ctx = hif_exec_get_napi(ctx);
	int irq_ind;

	if (ctx->inited) {
		napi_disable(&n_ctx->napi);
		ctx->inited = 0;
	}

	for (irq_ind = 0; irq_ind < ctx->numirq; irq_ind++)
		hif_irq_affinity_remove(ctx->os_irq[irq_ind]);

	hif_core_ctl_set_boost(false);
	netif_napi_del(&(n_ctx->napi));
}

struct hif_execution_ops napi_sched_ops = {
	.schedule = &hif_exec_napi_schedule,
	.kill = &hif_exec_napi_kill,
};

/**
 * hif_exec_napi_create() - allocate and initialize a napi exec context
 * @scale: a binary shift factor to map NAPI budget from\to internal
 *         budget
 */
static struct hif_exec_context *hif_exec_napi_create(uint32_t scale)
{
	struct hif_napi_exec_context *ctx;

	ctx = qdf_mem_malloc(sizeof(struct hif_napi_exec_context));
	if (!ctx)
		return NULL;

	ctx->exec_ctx.sched_ops = &napi_sched_ops;
	ctx->exec_ctx.inited = true;
	ctx->exec_ctx.scale_bin_shift = scale;
	qdf_net_if_create_dummy_if((struct qdf_net_if *)&ctx->netdev);
	netif_napi_add(&(ctx->netdev), &(ctx->napi), hif_exec_poll,
		       QCA_NAPI_BUDGET);
	napi_enable(&ctx->napi);

	return &ctx->exec_ctx;
}
#else
static struct hif_exec_context *hif_exec_napi_create(uint32_t scale)
{
	HIF_WARN("%s: FEATURE_NAPI not defined, making tasklet", __func__);
	return hif_exec_tasklet_create();
}
#endif


/**
 * hif_exec_tasklet_kill() - stop a tasklet exec context from being rescheduled
 * @ctx: a hif_exec_context known to be of tasklet type
 */
static void hif_exec_tasklet_kill(struct hif_exec_context *ctx)
{
	struct hif_tasklet_exec_context *t_ctx = hif_exec_get_tasklet(ctx);
	int irq_ind;

	if (ctx->inited) {
		tasklet_disable(&t_ctx->tasklet);
		tasklet_kill(&t_ctx->tasklet);
	}
	ctx->inited = false;

	for (irq_ind = 0; irq_ind < ctx->numirq; irq_ind++)
		hif_irq_affinity_remove(ctx->os_irq[irq_ind]);
}

struct hif_execution_ops tasklet_sched_ops = {
	.schedule = &hif_exec_tasklet_schedule,
	.kill = &hif_exec_tasklet_kill,
};

/**
 * hif_exec_tasklet_schedule() -  allocate and initialize a tasklet exec context
 */
static struct hif_exec_context *hif_exec_tasklet_create(void)
{
	struct hif_tasklet_exec_context *ctx;

	ctx = qdf_mem_malloc(sizeof(struct hif_tasklet_exec_context));
	if (!ctx)
		return NULL;

	ctx->exec_ctx.sched_ops = &tasklet_sched_ops;
	tasklet_init(&ctx->tasklet, hif_exec_tasklet_fn,
		     (unsigned long)ctx);

	ctx->exec_ctx.inited = true;

	return &ctx->exec_ctx;
}

/**
 * hif_exec_get_ctx() - retrieve an exec context based on an id
 * @softc: the hif context owning the exec context
 * @id: the id of the exec context
 *
 * mostly added to make it easier to rename or move the context array
 */
struct hif_exec_context *hif_exec_get_ctx(struct hif_opaque_softc *softc,
					  uint8_t id)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(softc);

	if (id < hif_state->hif_num_extgroup)
		return hif_state->hif_ext_group[id];

	return NULL;
}

int32_t hif_get_int_ctx_irq_num(struct hif_opaque_softc *softc,
				uint8_t id)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(softc);

	if (id < hif_state->hif_num_extgroup)
		return hif_state->hif_ext_group[id]->os_irq[0];
	return -EINVAL;
}

qdf_export_symbol(hif_get_int_ctx_irq_num);

#ifdef HIF_CPU_PERF_AFFINE_MASK
void hif_config_irq_set_perf_affinity_hint(
	struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);

	hif_config_irq_affinity(scn);
}

qdf_export_symbol(hif_config_irq_set_perf_affinity_hint);
#endif

uint32_t hif_configure_ext_group_interrupts(struct hif_opaque_softc *hif_ctx)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);
	struct hif_exec_context *hif_ext_group;
	int i, status;

	if (scn->ext_grp_irq_configured) {
		HIF_ERROR("%s Called after ext grp irq configured\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	for (i = 0; i < hif_state->hif_num_extgroup; i++) {
		hif_ext_group = hif_state->hif_ext_group[i];
		status = 0;
		qdf_spinlock_create(&hif_ext_group->irq_lock);
		if (hif_ext_group->configured &&
		    hif_ext_group->irq_requested == false) {
			hif_ext_group->irq_enabled = true;
			status = hif_grp_irq_configure(scn, hif_ext_group);
		}
		if (status != 0) {
			HIF_ERROR("%s: failed for group %d", __func__, i);
			hif_ext_group->irq_enabled = false;
		}
	}

	scn->ext_grp_irq_configured = true;

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(hif_configure_ext_group_interrupts);

#ifdef WLAN_SUSPEND_RESUME_TEST
/**
 * hif_check_and_trigger_ut_resume() - check if unit-test command was used to
 *				       to trigger fake-suspend command, if yes
 *				       then issue resume procedure.
 * @scn: opaque HIF software context
 *
 * This API checks if unit-test command was used to trigger fake-suspend command
 * and if answer is yes then it would trigger resume procedure.
 *
 * Make this API inline to save API-switch overhead and do branch-prediction to
 * optimize performance impact.
 *
 * Return: void
 */
static inline void hif_check_and_trigger_ut_resume(struct hif_softc *scn)
{
	if (qdf_unlikely(hif_irq_trigger_ut_resume(scn)))
		hif_ut_fw_resume(scn);
}
#else
static inline void hif_check_and_trigger_ut_resume(struct hif_softc *scn)
{
}
#endif

/**
 * hif_ext_group_interrupt_handler() - handler for related interrupts
 * @irq: irq number of the interrupt
 * @context: the associated hif_exec_group context
 *
 * This callback function takes care of dissabling the associated interrupts
 * and scheduling the expected bottom half for the exec_context.
 * This callback function also helps keep track of the count running contexts.
 */
irqreturn_t hif_ext_group_interrupt_handler(int irq, void *context)
{
	struct hif_exec_context *hif_ext_group = context;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ext_group->hif);

	if (hif_ext_group->irq_requested) {
		hif_latency_profile_start(hif_ext_group);

		hif_record_event(hif_ext_group->hif, hif_ext_group->grp_id,
				 0, 0, 0, HIF_EVENT_IRQ_TRIGGER);

		hif_ext_group->irq_disable(hif_ext_group);
		/*
		 * if private ioctl has issued fake suspend command to put
		 * FW in D0-WOW state then here is our chance to bring FW out
		 * of WOW mode.
		 *
		 * The reason why you need to explicitly wake-up the FW is here:
		 * APSS should have been in fully awake through-out when
		 * fake APSS suspend command was issued (to put FW in WOW mode)
		 * hence organic way of waking-up the FW
		 * (as part-of APSS-host wake-up) won't happen because
		 * in reality APSS didn't really suspend.
		 */
		hif_check_and_trigger_ut_resume(scn);
		qdf_atomic_inc(&scn->active_grp_tasklet_cnt);

		hif_ext_group->sched_ops->schedule(hif_ext_group);
	}

	return IRQ_HANDLED;
}

/**
 * hif_exec_kill() - grp tasklet kill
 * scn: hif_softc
 *
 * return: void
 */
void hif_exec_kill(struct hif_opaque_softc *hif_ctx)
{
	int i;
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);

	for (i = 0; i < hif_state->hif_num_extgroup; i++)
		hif_state->hif_ext_group[i]->sched_ops->kill(
			hif_state->hif_ext_group[i]);

	qdf_atomic_set(&hif_state->ol_sc.active_grp_tasklet_cnt, 0);
}

/**
 * hif_register_ext_group() - API to register external group
 * interrupt handler.
 * @hif_ctx : HIF Context
 * @numirq: number of irq's in the group
 * @irq: array of irq values
 * @handler: callback interrupt handler function
 * @cb_ctx: context to passed in callback
 * @type: napi vs tasklet
 *
 * Return: status
 */
uint32_t hif_register_ext_group(struct hif_opaque_softc *hif_ctx,
		uint32_t numirq, uint32_t irq[], ext_intr_handler handler,
		void *cb_ctx, const char *context_name,
		enum hif_exec_type type, uint32_t scale)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct hif_exec_context *hif_ext_group;

	if (scn->ext_grp_irq_configured) {
		HIF_ERROR("%s Called after ext grp irq configured\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (hif_state->hif_num_extgroup >= HIF_MAX_GROUP) {
		HIF_ERROR("%s Max groups reached\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (numirq >= HIF_MAX_GRP_IRQ) {
		HIF_ERROR("%s invalid numirq\n", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	hif_ext_group = hif_exec_create(type, scale);
	if (!hif_ext_group)
		return QDF_STATUS_E_FAILURE;

	hif_state->hif_ext_group[hif_state->hif_num_extgroup] =
		hif_ext_group;

	hif_ext_group->numirq = numirq;
	qdf_mem_copy(&hif_ext_group->irq[0], irq, numirq * sizeof(irq[0]));
	hif_ext_group->context = cb_ctx;
	hif_ext_group->handler = handler;
	hif_ext_group->configured = true;
	hif_ext_group->grp_id = hif_state->hif_num_extgroup;
	hif_ext_group->hif = hif_ctx;
	hif_ext_group->context_name = context_name;
	hif_ext_group->type = type;
	hif_event_history_init(hif_ext_group);

	hif_state->hif_num_extgroup++;
	return QDF_STATUS_SUCCESS;
}
qdf_export_symbol(hif_register_ext_group);

/**
 * hif_exec_create() - create an execution context
 * @type: the type of execution context to create
 */
struct hif_exec_context *hif_exec_create(enum hif_exec_type type,
						uint32_t scale)
{
	hif_debug("%s: create exec_type %d budget %d\n",
		  __func__, type, QCA_NAPI_BUDGET * scale);

	switch (type) {
	case HIF_EXEC_NAPI_TYPE:
		return hif_exec_napi_create(scale);

	case HIF_EXEC_TASKLET_TYPE:
		return hif_exec_tasklet_create();
	default:
		return NULL;
	}
}

/**
 * hif_exec_destroy() - free the hif_exec context
 * @ctx: context to free
 *
 * please kill the context before freeing it to avoid a use after free.
 */
void hif_exec_destroy(struct hif_exec_context *ctx)
{
	qdf_spinlock_destroy(&ctx->irq_lock);
	qdf_mem_free(ctx);
}

/**
 * hif_deregister_exec_group() - API to free the exec contexts
 * @hif_ctx: HIF context
 * @context_name: name of the module whose contexts need to be deregistered
 *
 * This function deregisters the contexts of the requestor identified
 * based on the context_name & frees the memory.
 *
 * Return: void
 */
void hif_deregister_exec_group(struct hif_opaque_softc *hif_ctx,
				const char *context_name)
{
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ctx);
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(scn);
	struct hif_exec_context *hif_ext_group;
	int i;

	for (i = 0; i < HIF_MAX_GROUP; i++) {
		hif_ext_group = hif_state->hif_ext_group[i];

		if (!hif_ext_group)
			continue;

		hif_debug("%s: Deregistering grp id %d name %s\n",
			  __func__,
			  hif_ext_group->grp_id,
			  hif_ext_group->context_name);

		if (strcmp(hif_ext_group->context_name, context_name) == 0) {
			hif_ext_group->sched_ops->kill(hif_ext_group);
			hif_state->hif_ext_group[i] = NULL;
			hif_exec_destroy(hif_ext_group);
			hif_state->hif_num_extgroup--;
		}

	}
}
qdf_export_symbol(hif_deregister_exec_group);
