/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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
#include <hif_irq_affinity.h>
#include "qdf_module.h"

/* mapping NAPI budget 0 to internal budget 0
 * NAPI budget 1 to internal budget [1,scaler -1]
 * NAPI budget 2 to internal budget [scaler, 2 * scaler - 1], etc
 */
#define NAPI_BUDGET_TO_INTERNAL_BUDGET(n, s) \
	(((n) << (s)) - 1)
#define INTERNAL_BUDGET_TO_NAPI_BUDGET(n, s) \
	(((n) + 1) >> (s))

static struct hif_exec_context *hif_exec_tasklet_create(void);

/**
 * hif_print_napi_stats() - print NAPI stats
 * @hif_ctx: hif context
 *
 * return: void
 */
void hif_print_napi_stats(struct hif_opaque_softc *hif_ctx)
{
	struct HIF_CE_state *hif_state = HIF_GET_CE_STATE(hif_ctx);
	struct hif_exec_context *hif_ext_group;
	struct qca_napi_stat *napi_stats;
	int i, j;

	QDF_TRACE(QDF_MODULE_ID_HIF, QDF_TRACE_LEVEL_FATAL,
		"NAPI[#ctx]CPU[#] |schedules |polls |completes |workdone\n");

	for (i = 0; i < hif_state->hif_num_extgroup; i++) {
		if (hif_state->hif_ext_group[i]) {
			hif_ext_group = hif_state->hif_ext_group[i];
			for (j = 0; j < num_possible_cpus(); j++) {
				napi_stats = &(hif_ext_group->stats[j]);
				if (napi_stats->napi_schedules != 0)
					QDF_TRACE(QDF_MODULE_ID_HIF,
						QDF_TRACE_LEVEL_FATAL,
						"NAPI[%2d]CPU[%d]: "
						"%7d %7d %7d %7d \n",
						i, j,
						napi_stats->napi_schedules,
						napi_stats->napi_polls,
						napi_stats->napi_completes,
						napi_stats->napi_workdone);
			}
		}
	}
}
qdf_export_symbol(hif_print_napi_stats);

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
 * hif_exec_poll() - grp tasklet
 * data: context
 *
 * return: void
 */
static int hif_exec_poll(struct napi_struct *napi, int budget)
{
	struct hif_napi_exec_context *exec_ctx =
		    qdf_container_of(napi, struct hif_napi_exec_context, napi);
	struct hif_exec_context *hif_ext_group = &exec_ctx->exec_ctx;
	struct hif_softc *scn = HIF_GET_SOFTC(hif_ext_group->hif);
	int work_done;
	int normalized_budget = 0;
	int shift = hif_ext_group->scale_bin_shift;
	int cpu = smp_processor_id();

	if (budget)
		normalized_budget = NAPI_BUDGET_TO_INTERNAL_BUDGET(budget, shift);
	work_done = hif_ext_group->handler(hif_ext_group->context,
							normalized_budget);

	if (work_done < normalized_budget) {
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
	hif_ext_group->stats[cpu].napi_workdone += work_done;

	/* map internal budget to NAPI budget */
	if (work_done)
		work_done = INTERNAL_BUDGET_TO_NAPI_BUDGET(work_done, shift);

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

	netif_napi_del(&(n_ctx->napi));
}

struct hif_execution_ops napi_sched_ops = {
	.schedule = &hif_exec_napi_schedule,
	.kill = &hif_exec_napi_kill,
};

#ifdef FEATURE_NAPI
/**
 * hif_exec_napi_create() - allocate and initialize a napi exec context
 * @scale: a binary shift factor to map NAPI budget from\to internal
 *         budget
 */
static struct hif_exec_context *hif_exec_napi_create(uint32_t scale)
{
	struct hif_napi_exec_context *ctx;

	ctx = qdf_mem_malloc(sizeof(struct hif_napi_exec_context));
	if (ctx == NULL)
		return NULL;

	ctx->exec_ctx.sched_ops = &napi_sched_ops;
	ctx->exec_ctx.inited = true;
	ctx->exec_ctx.scale_bin_shift = scale;
	init_dummy_netdev(&(ctx->netdev));
	netif_napi_add(&(ctx->netdev), &(ctx->napi), hif_exec_poll,
		       QCA_NAPI_BUDGET);
	napi_enable(&ctx->napi);

	return &ctx->exec_ctx;
}
#else
static struct hif_exec_context *hif_exec_napi_create(uint32_t scale)
{
	HIF_WARN("%s: FEATURE_NAPI not defined, making tasklet");
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
	if (ctx == NULL)
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

/**
 * hif_configure_ext_group_interrupts() - API to configure external group
 * interrpts
 * @hif_ctx : HIF Context
 *
 * Return: status
 */
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

	hif_ext_group->irq_disable(hif_ext_group);
	qdf_atomic_inc(&scn->active_grp_tasklet_cnt);

	hif_ext_group->sched_ops->schedule(hif_ext_group);

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
	if (hif_ext_group == NULL)
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
	HIF_INFO("%s: create exec_type %d budget %d\n",
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

		HIF_INFO("%s: Deregistering grp id %d name %s\n",
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
