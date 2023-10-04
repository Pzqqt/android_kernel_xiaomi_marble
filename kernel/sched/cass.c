// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023 Sultan Alsawaf <sultan@kerneltoast.com>.
 */

/**
 * DOC: Capacity Aware Superset Scheduler (CASS) description
 *
 * The Capacity Aware Superset Scheduler (CASS) optimizes runqueue selection of
 * CFS tasks. By using CPU capacity as a basis for comparing the relative
 * utilization between different CPUs, CASS fairly balances load across CPUs of
 * varying capacities. This results in improved multi-core performance,
 * especially when CPUs are overutilized because CASS doesn't clip a CPU's
 * utilization when it eclipses the CPU's capacity.
 *
 * As a superset of capacity aware scheduling, CASS implements a hierarchy of
 * criteria to determine the better CPU to wake a task upon between CPUs that
 * have the same relative utilization. This way, single-core performance,
 * latency, and cache affinity are all optimized where possible.
 *
 * CASS doesn't feature explicit energy awareness but its basic load balancing
 * principle results in decreased overall energy, often better than what is
 * possible with explicit energy awareness. By fairly balancing load based on
 * relative utilization, all CPUs are kept at their lowest P-state necessary to
 * satisfy the overall load at any given moment.
 */

struct cass_cpu_cand {
	int cpu;
	unsigned int exit_lat;
	unsigned long cap;
	unsigned long util;
};

static __always_inline
unsigned long cass_cpu_util(int cpu, bool sync)
{
	struct cfs_rq *cfs_rq = &cpu_rq(cpu)->cfs;
	unsigned long util = READ_ONCE(cfs_rq->avg.util_avg);

	/* Deduct @current's util from this CPU if this is a sync wake */
	if (sync && cpu == smp_processor_id())
		lsub_positive(&util, task_util(current));

	if (sched_feat(UTIL_EST))
		util = max_t(unsigned long, util,
			     READ_ONCE(cfs_rq->avg.util_est.enqueued));

	return util;
}

/* Returns true if @a is a better CPU than @b */
static __always_inline
bool cass_cpu_better(const struct cass_cpu_cand *a,
		     const struct cass_cpu_cand *b,
		     int prev_cpu, bool sync)
{
#define cass_cmp(a, b) ({ res = (a) - (b); })
#define cass_eq(a, b) ({ res = (a) == (b); })
	long res;

	/* Prefer the CPU with lower relative utilization */
	if (cass_cmp(b->util, a->util))
		goto done;

	/* Prefer the current CPU for sync wakes */
	if (sync && (cass_eq(a->cpu, smp_processor_id()) ||
		     !cass_cmp(b->cpu, smp_processor_id())))
		goto done;

	/* Prefer the CPU with higher capacity */
	if (cass_cmp(a->cap, b->cap))
		goto done;

	/* Prefer the CPU with lower idle exit latency */
	if (cass_cmp(b->exit_lat, a->exit_lat))
		goto done;

	/* Prefer the previous CPU */
	if (cass_eq(a->cpu, prev_cpu) || !cass_cmp(b->cpu, prev_cpu))
		goto done;

	/* Prefer the CPU that shares a cache with the previous CPU */
	if (cass_cmp(cpus_share_cache(a->cpu, prev_cpu),
		     cpus_share_cache(b->cpu, prev_cpu)))
		goto done;

	/* @a isn't a better CPU than @b. @res must be <=0 to indicate such. */
done:
	/* @a is a better CPU than @b if @res is positive */
	return res > 0;
}

static int cass_best_cpu(struct task_struct *p, int prev_cpu, bool sync)
{
	/* Initialize @best such that @best always has a valid CPU at the end */
	struct cass_cpu_cand cands[2], *best = cands, *curr;
	struct cpuidle_state *idle_state;
	bool has_idle = false;
	unsigned long p_util;
	int cidx = 0, cpu;

	/* Get the utilization for this task */
	p_util = clamp(task_util_est(p),
		       uclamp_eff_value(p, UCLAMP_MIN),
		       uclamp_eff_value(p, UCLAMP_MAX));

	/*
	 * Find the best CPU to wake @p on. Although idle_get_state() requires
	 * an RCU read lock, an RCU read lock isn't needed because we're not
	 * preemptible and RCU-sched is unified with normal RCU. Therefore,
	 * non-preemptible contexts are implicitly RCU-safe.
	 */
	for_each_cpu_and(cpu, p->cpus_ptr, cpu_active_mask) {
		/* Use the free candidate slot */
		curr = &cands[cidx];
		curr->cpu = cpu;

		/*
		 * Check if this CPU is idle or only has SCHED_IDLE tasks. For
		 * sync wakes, always treat the current CPU as idle.
		 */
		if ((sync && cpu == smp_processor_id()) ||
		    available_idle_cpu(cpu) || sched_idle_cpu(cpu)) {
			/* Discard any previous non-idle candidate */
			if (!has_idle) {
				best = curr;
				cidx ^= 1;
			}
			has_idle = true;

			/* Nonzero exit latency indicates this CPU is idle */
			curr->exit_lat = 1;

			/* Add on the actual idle exit latency, if any */
			idle_state = idle_get_state(cpu_rq(cpu));
			if (idle_state)
				curr->exit_lat += idle_state->exit_latency;
		} else {
			/* Skip non-idle CPUs if there's an idle candidate */
			if (has_idle)
				continue;

			/* Zero exit latency indicates this CPU isn't idle */
			curr->exit_lat = 0;
		}

		/* Get this CPU's utilization, possibly without @current */
		curr->util = cass_cpu_util(cpu, sync);

		/*
		 * Add @p's utilization to this CPU if it's not @p's CPU, to
		 * find what this CPU's relative utilization would look like
		 * if @p were on it.
		 */
		if (cpu != task_cpu(p))
			curr->util += p_util;

		/*
		 * Get the current capacity of this CPU adjusted for thermal
		 * pressure as well as IRQ and RT-task time.
		 */
		curr->cap = capacity_of(cpu);

		/* Calculate the relative utilization for this CPU candidate */
		curr->util = curr->util * SCHED_CAPACITY_SCALE / curr->cap;

		/* If @best == @curr then there's no need to compare them */
		if (best == curr)
			continue;

		/* Check if this CPU is better than the best CPU found */
		if (cass_cpu_better(curr, best, prev_cpu, sync)) {
			best = curr;
			cidx ^= 1;
		}
	}

	return best->cpu;
}

static int cass_select_task_rq_fair(struct task_struct *p, int prev_cpu,
				    int sd_flag, int wake_flags)
{
	bool sync;

	/* Don't balance on exec since we don't know what @p will look like */
	if (sd_flag & SD_BALANCE_EXEC)
		return prev_cpu;

	/*
	 * If there aren't any valid CPUs which are active, then just return the
	 * first valid CPU since it's possible for certain types of tasks to run
	 * on inactive CPUs.
	 */
	if (unlikely(!cpumask_intersects(p->cpus_ptr, cpu_active_mask)))
		return cpumask_first(p->cpus_ptr);

	/* cass_best_cpu() needs the task's utilization, so sync it up */
	if (!(sd_flag & SD_BALANCE_FORK))
		sync_entity_load_avg(&p->se);

	sync = (wake_flags & WF_SYNC) && !(current->flags & PF_EXITING);
	return cass_best_cpu(p, prev_cpu, sync);
}
