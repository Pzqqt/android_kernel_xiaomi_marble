// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2023 Sultan Alsawaf <sultan@kerneltoast.com>.
 */

/**
 * DOC: SBalance description
 *
 * This is a simple IRQ balancer that polls every X number of milliseconds and
 * moves IRQs from the most interrupt-heavy CPU to the least interrupt-heavy
 * CPUs until the heaviest CPU is no longer the heaviest. IRQs are only moved
 * from one source CPU to any number of destination CPUs per balance run.
 * Balancing is skipped if the gap between the most interrupt-heavy CPU and the
 * least interrupt-heavy CPU is below the configured threshold of interrupts.
 *
 * The heaviest IRQs are targeted for migration in order to reduce the number of
 * IRQs to migrate. If moving an IRQ would reduce overall balance, then it won't
 * be migrated.
 *
 * The most interrupt-heavy CPU is calculated by scaling the number of new
 * interrupts on that CPU to the CPU's current capacity. This way, interrupt
 * heaviness takes into account factors such as thermal pressure and time spent
 * processing interrupts rather than just the sheer number of them. This also
 * makes SBalance aware of CPU asymmetry, where different CPUs can have
 * different performance capacities and be proportionally balanced.
 */

#define pr_fmt(fmt) "sbalance: " fmt

#include <linux/freezer.h>
#include <linux/irq.h>
#include <linux/list_sort.h>
#include "../sched/sched.h"
#include "internals.h"

/* Perform IRQ balancing every POLL_MS milliseconds */
#define POLL_MS CONFIG_IRQ_SBALANCE_POLL_MSEC

/*
 * There needs to be a difference of at least this many new interrupts between
 * the heaviest and least-heavy CPUs during the last polling window in order for
 * balancing to occur. This is to avoid balancing when the system is quiet.
 *
 * This threshold is compared to the _scaled_ interrupt counts per CPU; i.e.,
 * the number of interrupts scaled to the CPU's capacity.
 */
#define IRQ_SCALED_THRESH CONFIG_IRQ_SBALANCE_THRESH

struct bal_irq {
	struct list_head node;
	struct list_head move_node;
	struct rcu_head rcu;
	struct irq_desc *desc;
	unsigned int delta_nr;
	unsigned int old_nr;
	int prev_cpu;
};

struct bal_domain {
	struct list_head movable_irqs;
	unsigned int intrs;
	int cpu;
};

static LIST_HEAD(bal_irq_list);
static DEFINE_SPINLOCK(bal_irq_lock);
static DEFINE_PER_CPU(struct bal_domain, balance_data);
static DEFINE_PER_CPU(unsigned long, cpu_cap);
static cpumask_t cpu_exclude_mask __read_mostly;

void sbalance_desc_add(struct irq_desc *desc)
{
	struct bal_irq *bi;

	bi = kmalloc(sizeof(*bi), GFP_KERNEL);
	if (WARN_ON(!bi))
		return;

	*bi = (typeof(*bi)){ .desc = desc };
	spin_lock(&bal_irq_lock);
	list_add_tail_rcu(&bi->node, &bal_irq_list);
	spin_unlock(&bal_irq_lock);
}

void sbalance_desc_del(struct irq_desc *desc)
{
	struct bal_irq *bi;

	spin_lock(&bal_irq_lock);
	list_for_each_entry(bi, &bal_irq_list, node) {
		if (bi->desc == desc) {
			list_del_rcu(&bi->node);
			kfree_rcu(bi, rcu);
			break;
		}
	}
	spin_unlock(&bal_irq_lock);
}

static int bal_irq_move_node_cmp(void *priv, const struct list_head *lhs_p,
				 const struct list_head *rhs_p)
{
	const struct bal_irq *lhs = list_entry(lhs_p, typeof(*lhs), move_node);
	const struct bal_irq *rhs = list_entry(rhs_p, typeof(*rhs), move_node);

	return rhs->delta_nr - lhs->delta_nr;
}

/* Returns false if this IRQ should be totally ignored for this balancing run */
static bool update_irq_data(struct bal_irq *bi, int *cpu)
{
	struct irq_desc *desc = bi->desc;
	unsigned int nr;

	/* Find the CPU which currently has this IRQ affined */
	raw_spin_lock_irq(&desc->lock);
	*cpu = cpumask_first(desc->irq_common_data.affinity);
	raw_spin_unlock_irq(&desc->lock);
	if (*cpu >= nr_cpu_ids)
		return false;

	/*
	 * Calculate the number of new interrupts from this IRQ. It is assumed
	 * that the IRQ has been running on the same CPU since the last
	 * balancing run. This might not hold true if the IRQ was moved by
	 * someone else since the last balancing run, or if the CPU this IRQ was
	 * previously running on has since gone offline.
	 */
	nr = *per_cpu_ptr(desc->kstat_irqs, *cpu);
	if (nr <= bi->old_nr) {
		bi->old_nr = nr;
		return false;
	}

	/* Calculate the number of new interrupts on this CPU from this IRQ */
	bi->delta_nr = nr - bi->old_nr;
	bi->old_nr = nr;
	return true;
}

static int move_irq_to_cpu(struct bal_irq *bi, int cpu)
{
	struct irq_desc *desc = bi->desc;
	int prev_cpu, ret;

	/* Set the affinity if it wasn't changed since we looked at it */
	raw_spin_lock_irq(&desc->lock);
	prev_cpu = cpumask_first(desc->irq_common_data.affinity);
	if (prev_cpu == bi->prev_cpu) {
		ret = irq_set_affinity_locked(&desc->irq_data, cpumask_of(cpu),
					      false);
	} else {
		bi->prev_cpu = prev_cpu;
		ret = -EINVAL;
	}
	raw_spin_unlock_irq(&desc->lock);

	if (!ret) {
		/* Update the old interrupt count using the new CPU */
		bi->old_nr = *per_cpu_ptr(desc->kstat_irqs, cpu);
		pr_debug("Moved IRQ%d (CPU%d -> CPU%d)\n",
			 irq_desc_get_irq(desc), prev_cpu, cpu);
	}
	return ret;
}

static unsigned int scale_intrs(unsigned int intrs, int cpu)
{
	/* Scale the number of interrupts to this CPU's current capacity */
	return intrs * SCHED_CAPACITY_SCALE / per_cpu(cpu_cap, cpu);
}

/* Returns true if IRQ balancing should stop */
static bool find_min_bd(const cpumask_t *mask, unsigned int max_intrs,
			struct bal_domain **min_bd)
{
	unsigned int intrs, min_intrs = UINT_MAX;
	struct bal_domain *bd;
	int cpu;

	for_each_cpu(cpu, mask) {
		bd = per_cpu_ptr(&balance_data, cpu);
		intrs = scale_intrs(bd->intrs, bd->cpu);

		/* Terminate when the formerly-max CPU isn't the max anymore */
		if (intrs > max_intrs)
			return true;

		/* Find the CPU with the lowest relative number of interrupts */
		if (intrs < min_intrs) {
			min_intrs = intrs;
			*min_bd = bd;
		}
	}

	/* Don't balance if IRQs are already balanced evenly enough */
	return max_intrs - min_intrs < IRQ_SCALED_THRESH;
}

static void balance_irqs(void)
{
	static cpumask_t cpus;
	struct bal_domain *bd, *max_bd, *min_bd;
	unsigned int intrs, max_intrs;
	bool moved_irq = false;
	struct bal_irq *bi;
	int cpu;

	rcu_read_lock();

	/* Find the available CPUs for balancing, if there are any */
	cpumask_andnot(&cpus, cpu_active_mask, &cpu_exclude_mask);
	if (unlikely(cpumask_weight(&cpus) <= 1))
		goto unlock;

	/*
	 * Get the current capacity for each CPU. This is adjusted for time
	 * spent processing IRQs, RT-task time, and thermal pressure. We don't
	 * exclude time spent processing IRQs when balancing because balancing
	 * is only done using interrupt counts rather than time spent in
	 * interrupts. That way, time spent processing each interrupt is
	 * considered when balancing.
	 */
	for_each_cpu(cpu, &cpus)
		per_cpu(cpu_cap, cpu) = cpu_rq(cpu)->cpu_capacity;

	list_for_each_entry_rcu(bi, &bal_irq_list, node) {
		if (!update_irq_data(bi, &cpu))
			continue;

		/* Add the number of new interrupts to this CPU's count */
		bd = per_cpu_ptr(&balance_data, cpu);
		bd->intrs += bi->delta_nr;

		/* Consider this IRQ for balancing if it's movable */
		if (!__irq_can_set_affinity(bi->desc))
			continue;

		/* Ignore for this balancing run if something else moved it */
		if (cpu != bi->prev_cpu) {
			bi->prev_cpu = cpu;
			continue;
		}

		list_add_tail(&bi->move_node, &bd->movable_irqs);
	}

	/* Find the most interrupt-heavy CPU with movable IRQs */
	while (1) {
		max_intrs = 0;
		for_each_cpu(cpu, &cpus) {
			bd = per_cpu_ptr(&balance_data, cpu);
			intrs = scale_intrs(bd->intrs, bd->cpu);
			if (intrs > max_intrs) {
				max_intrs = intrs;
				max_bd = bd;
			}
		}

		/* No balancing to do if there aren't any movable IRQs */
		if (unlikely(!max_intrs))
			goto unlock;

		/* Ensure the heaviest CPU has IRQs which can be moved away */
		if (!list_empty(&max_bd->movable_irqs))
			break;

try_next_heaviest:
		/*
		 * If the heaviest CPU has no movable IRQs then it can neither
		 * receive IRQs nor give IRQs. Exclude it from balancing so the
		 * remaining CPUs can be balanced, if there are any.
		 */
		if (cpumask_weight(&cpus) == 2)
			goto unlock;

		cpumask_clear_cpu(max_bd->cpu, &cpus);
	}

	/* Find the CPU with the lowest relative interrupt count */
	if (find_min_bd(&cpus, max_intrs, &min_bd))
		goto unlock;

	/* Sort movable IRQs in descending order of number of new interrupts */
	list_sort(NULL, &max_bd->movable_irqs, bal_irq_move_node_cmp);

	/* Push IRQs away from the heaviest CPU to the least-heavy CPUs */
	list_for_each_entry(bi, &max_bd->movable_irqs, move_node) {
		/* Skip this IRQ if it would just overload the target CPU */
		intrs = scale_intrs(min_bd->intrs + bi->delta_nr, min_bd->cpu);
		if (intrs >= max_intrs)
			continue;

		/* Try to migrate this IRQ, or skip it if migration fails */
		if (move_irq_to_cpu(bi, min_bd->cpu))
			continue;

		/* Keep track of whether or not any IRQs are moved */
		moved_irq = true;

		/* Update the counts and recalculate the max scaled count */
		min_bd->intrs += bi->delta_nr;
		max_bd->intrs -= bi->delta_nr;
		max_intrs = scale_intrs(max_bd->intrs, max_bd->cpu);

		/* Recheck for the least-heavy CPU since it may have changed */
		if (find_min_bd(&cpus, max_intrs, &min_bd))
			break;
	}

	/*
	 * If the heaviest CPU has movable IRQs which can't actually be moved,
	 * then ignore it and try balancing the next heaviest CPU.
	 */
	if (!moved_irq)
		goto try_next_heaviest;
unlock:
	rcu_read_unlock();

	/* Reset each balance domain for the next run */
	for_each_possible_cpu(cpu) {
		bd = per_cpu_ptr(&balance_data, cpu);
		INIT_LIST_HEAD(&bd->movable_irqs);
		bd->intrs = 0;
	}
}

static int __noreturn sbalance_thread(void *data)
{
	long poll_jiffies = msecs_to_jiffies(POLL_MS);
	struct bal_domain *bd;
	int cpu;

	/* Parse the list of CPUs to exclude, if any */
	if (cpulist_parse(CONFIG_SBALANCE_EXCLUDE_CPUS, &cpu_exclude_mask))
		cpu_exclude_mask = CPU_MASK_NONE;

	/* Initialize the data used for balancing */
	for_each_possible_cpu(cpu) {
		bd = per_cpu_ptr(&balance_data, cpu);
		INIT_LIST_HEAD(&bd->movable_irqs);
		bd->cpu = cpu;
	}

	set_freezable();
	while (1) {
		freezable_schedule_timeout_interruptible(poll_jiffies);
		balance_irqs();
	}
}

static int __init sbalance_init(void)
{
	BUG_ON(IS_ERR(kthread_run(sbalance_thread, NULL, "sbalanced")));
	return 0;
}
late_initcall(sbalance_init);
