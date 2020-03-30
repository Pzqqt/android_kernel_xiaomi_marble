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

#ifndef __HIF_EXEC_H__
#define __HIF_EXEC_H__

#include <hif.h>
#include <hif_irq_affinity.h>
#include <linux/cpumask.h>
/*Number of buckets for latency*/
#define HIF_SCHED_LATENCY_BUCKETS 8

/*Buckets for latency between 0 to 2 ms*/
#define HIF_SCHED_LATENCY_BUCKET_0_2 2
/*Buckets for latency between 3 to 10 ms*/
#define HIF_SCHED_LATENCY_BUCKET_3_10 10
/*Buckets for latency between 11 to 20 ms*/
#define HIF_SCHED_LATENCY_BUCKET_11_20 20
/*Buckets for latency between 21 to 50 ms*/
#define HIF_SCHED_LATENCY_BUCKET_21_50 50
/*Buckets for latency between 50 to 100 ms*/
#define HIF_SCHED_LATENCY_BUCKET_51_100 100
/*Buckets for latency between 100 to 250 ms*/
#define HIF_SCHED_LATENCY_BUCKET_101_250 250
/*Buckets for latency between 250 to 500 ms*/
#define HIF_SCHED_LATENCY_BUCKET_251_500 500

struct hif_exec_context;

struct hif_execution_ops {
	char *context_type;
	void (*schedule)(struct hif_exec_context *);
	void (*reschedule)(struct hif_exec_context *);
	void (*kill)(struct hif_exec_context *);
};

/**
 * hif_exec_context: only ever allocated as a subtype eg.
 *					hif_tasklet_exec_context
 *
 * @context: context for the handler function to use.
 * @evt_hist: a pointer to the DP event history
 * @context_name: a pointer to a const string for debugging.
 *		this should help whenever there could be ambiguity
 *		in what type of context the void* context points to
 * @irq: irq handle coresponding to hw block
 * @os_irq: irq handle for irq_afinity
 * @cpu: the cpu this context should be affined to
 * @work_complete: Function call called when leaving the execution context to
 *	determine if this context should reschedule or wait for an interrupt.
 *	This function may be used as a hook for post processing.
 *
 * @sched_latency_stats: schdule latency stats for different latency buckets
 * @tstamp: timestamp when napi poll happens
 * @irq_disable: called before scheduling the context.
 * @irq_enable: called when the context leaves polling mode
 * @irq_name: pointer to function to return irq name/string mapped to irq number
 * @irq_lock: spinlock used while enabling/disabling IRQs
 * @type: type of execution context
 * @poll_start_time: hif napi poll start time in nanoseconds
 * @force_break: flag to indicate if HIF execution context was forced to return
 *		 to HIF. This means there is more work to be done. Hence do not
 *		 call napi_complete.
 */
struct hif_exec_context {
	struct hif_execution_ops *sched_ops;
	struct hif_opaque_softc *hif;
	uint32_t numirq;
	uint32_t irq[HIF_MAX_GRP_IRQ];
	uint32_t os_irq[HIF_MAX_GRP_IRQ];
	cpumask_t cpumask;
	uint32_t grp_id;
	uint32_t scale_bin_shift;
	const char *context_name;
	void *context;
	ext_intr_handler handler;
	struct hif_event_history *evt_hist;

	bool (*work_complete)(struct hif_exec_context *, int work_done);
	void (*irq_enable)(struct hif_exec_context *);
	void (*irq_disable)(struct hif_exec_context *);
	const char* (*irq_name)(int irq_no);
	uint64_t sched_latency_stats[HIF_SCHED_LATENCY_BUCKETS];
	uint64_t tstamp;

	uint8_t cpu;
	struct qca_napi_stat stats[NR_CPUS];
	bool inited;
	bool configured;
	bool irq_requested;
	bool irq_enabled;
	qdf_spinlock_t irq_lock;
	enum hif_exec_type type;
	unsigned long long poll_start_time;
	bool force_break;
#ifdef HIF_CPU_PERF_AFFINE_MASK
	/* Stores the affinity hint mask for each WLAN IRQ */
	qdf_cpu_mask new_cpu_mask[HIF_MAX_GRP_IRQ];
#endif
};

/**
 * struct hif_tasklet_exec_context - exec_context for tasklets
 * @exec_ctx: inherited data type
 * @tasklet: tasklet structure for scheduling
 */
struct hif_tasklet_exec_context {
	struct hif_exec_context exec_ctx;
	struct tasklet_struct tasklet;
};

/**
 * struct hif_napi_exec_context - exec_context for NAPI
 * @exec_ctx: inherited data type
 * @netdev: dummy net device associated with the napi context
 * @napi: napi structure used in scheduling
 */
struct hif_napi_exec_context {
	struct hif_exec_context exec_ctx;
	struct net_device    netdev; /* dummy net_dev */
	struct napi_struct   napi;
};

static inline struct hif_napi_exec_context*
	hif_exec_get_napi(struct hif_exec_context *ctx)
{
	return (struct hif_napi_exec_context *) ctx;
}

static inline struct hif_tasklet_exec_context*
	hif_exec_get_tasklet(struct hif_exec_context *ctx)
{
	return (struct hif_tasklet_exec_context *) ctx;
}

struct hif_exec_context *hif_exec_create(enum hif_exec_type type,
						uint32_t scale);

void hif_exec_destroy(struct hif_exec_context *ctx);

int hif_grp_irq_configure(struct hif_softc *scn,
			  struct hif_exec_context *hif_exec);
irqreturn_t hif_ext_group_interrupt_handler(int irq, void *context);

struct hif_exec_context *hif_exec_get_ctx(struct hif_opaque_softc *hif,
					  uint8_t id);
void hif_exec_kill(struct hif_opaque_softc *scn);

#ifdef HIF_CPU_PERF_AFFINE_MASK
/**
 * hif_pci_irq_set_affinity_hint() - API to set IRQ affinity
 * @hif_ext_group: hif_ext_group to extract the irq info
 *
 * This function will set the IRQ affinity to the gold cores
 * only for defconfig builds
 *
 * Return: none
 */
void hif_pci_irq_set_affinity_hint(
	struct hif_exec_context *hif_ext_group);
#else
static inline void hif_pci_irq_set_affinity_hint(
	struct hif_exec_context *hif_ext_group)
{
}
#endif /* ifdef HIF_CPU_PERF_AFFINE_MASK */
#endif

