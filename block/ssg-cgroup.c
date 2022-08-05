// SPDX-License-Identifier: GPL-2.0
/*
 *  Control Group of SamSung Generic I/O scheduler
 *
 *  Copyright (C) 2021 Changheun Lee <nanich.lee@samsung.com>
 */

#include <linux/blkdev.h>
#include <linux/blk-mq.h>

#include "blk-mq.h"
#include "blk-mq-tag.h"
#include "ssg-cgroup.h"



static struct blkcg_policy ssg_blkcg_policy;



#define CPD_TO_SSG_BLKCG(_cpd) \
	container_of_safe((_cpd), struct ssg_blkcg, cpd)
#define BLKCG_TO_SSG_BLKCG(_blkcg) \
	CPD_TO_SSG_BLKCG(blkcg_to_cpd((_blkcg), &ssg_blkcg_policy))

#define PD_TO_SSG_BLKG(_pd) \
	container_of_safe((_pd), struct ssg_blkg, pd)
#define BLKG_TO_SSG_BLKG(_blkg) \
	PD_TO_SSG_BLKG(blkg_to_pd((_blkg), &ssg_blkcg_policy))

#define CSS_TO_SSG_BLKCG(css) BLKCG_TO_SSG_BLKCG(css_to_blkcg(css))



static struct blkcg_policy_data *ssg_blkcg_cpd_alloc(gfp_t gfp)
{
	struct ssg_blkcg *ssg_blkcg;

	ssg_blkcg = kzalloc(sizeof(struct ssg_blkcg), gfp);
	if (ZERO_OR_NULL_PTR(ssg_blkcg))
		return NULL;

	return &ssg_blkcg->cpd;
}

static void ssg_blkcg_cpd_init(struct blkcg_policy_data *cpd)
{
	struct ssg_blkcg *ssg_blkcg = CPD_TO_SSG_BLKCG(cpd);

	if (IS_ERR_OR_NULL(ssg_blkcg))
		return;

	ssg_blkcg->max_available_ratio = 100;
}

static void ssg_blkcg_cpd_free(struct blkcg_policy_data *cpd)
{
	struct ssg_blkcg *ssg_blkcg = CPD_TO_SSG_BLKCG(cpd);

	if (IS_ERR_OR_NULL(ssg_blkcg))
		return;

	kfree(ssg_blkcg);
}

static void ssg_blkcg_set_shallow_depth(struct ssg_blkcg *ssg_blkcg,
		struct ssg_blkg *ssg_blkg, struct blk_mq_tags *tags)
{
	unsigned int depth = tags->bitmap_tags->sb.depth;
	unsigned int map_nr = tags->bitmap_tags->sb.map_nr;

	ssg_blkg->max_available_rqs =
		depth * ssg_blkcg->max_available_ratio / 100U;
	ssg_blkg->shallow_depth =
		max_t(unsigned int, 1, ssg_blkg->max_available_rqs / map_nr);
}

static struct blkg_policy_data *ssg_blkcg_pd_alloc(gfp_t gfp,
		struct request_queue *q, struct blkcg *blkcg)
{
	struct ssg_blkg *ssg_blkg;

	ssg_blkg = kzalloc_node(sizeof(struct ssg_blkg), gfp, q->node);
	if (ZERO_OR_NULL_PTR(ssg_blkg))
		return NULL;

	return &ssg_blkg->pd;
}

static void ssg_blkcg_pd_init(struct blkg_policy_data *pd)
{
	struct ssg_blkg *ssg_blkg;
	struct ssg_blkcg *ssg_blkcg;

	ssg_blkg = PD_TO_SSG_BLKG(pd);
	if (IS_ERR_OR_NULL(ssg_blkg))
		return;

	ssg_blkcg = BLKCG_TO_SSG_BLKCG(pd->blkg->blkcg);
	if (IS_ERR_OR_NULL(ssg_blkcg))
		return;

	atomic_set(&ssg_blkg->current_rqs, 0);
	ssg_blkcg_set_shallow_depth(ssg_blkcg, ssg_blkg,
			pd->blkg->q->queue_hw_ctx[0]->sched_tags);
}

static void ssg_blkcg_pd_free(struct blkg_policy_data *pd)
{
	struct ssg_blkg *ssg_blkg = PD_TO_SSG_BLKG(pd);

	if (IS_ERR_OR_NULL(ssg_blkg))
		return;

	kfree(ssg_blkg);
}

unsigned int ssg_blkcg_shallow_depth(struct request_queue *q)
{
	struct blkcg_gq *blkg;
	struct ssg_blkg *ssg_blkg;

	rcu_read_lock();
	blkg = blkg_lookup(css_to_blkcg(blkcg_css()), q);
	ssg_blkg = BLKG_TO_SSG_BLKG(blkg);
	rcu_read_unlock();

	if (IS_ERR_OR_NULL(ssg_blkg))
		return 0;

	if (atomic_read(&ssg_blkg->current_rqs) < ssg_blkg->max_available_rqs)
		return 0;

	return ssg_blkg->shallow_depth;
}

void ssg_blkcg_depth_updated(struct blk_mq_hw_ctx *hctx)
{
	struct request_queue *q = hctx->queue;
	struct cgroup_subsys_state *pos_css;
	struct blkcg_gq *blkg;
	struct ssg_blkg *ssg_blkg;
	struct ssg_blkcg *ssg_blkcg;

	rcu_read_lock();
	blkg_for_each_descendant_pre(blkg, pos_css, q->root_blkg) {
		ssg_blkg = BLKG_TO_SSG_BLKG(blkg);
		if (IS_ERR_OR_NULL(ssg_blkg))
			continue;

		ssg_blkcg = BLKCG_TO_SSG_BLKCG(blkg->blkcg);
		if (IS_ERR_OR_NULL(ssg_blkcg))
			continue;

		atomic_set(&ssg_blkg->current_rqs, 0);
		ssg_blkcg_set_shallow_depth(ssg_blkcg, ssg_blkg, hctx->sched_tags);
	}
	rcu_read_unlock();
}

void ssg_blkcg_inc_rq(struct blkcg_gq *blkg)
{
	struct ssg_blkg *ssg_blkg = BLKG_TO_SSG_BLKG(blkg);

	if (IS_ERR_OR_NULL(ssg_blkg))
		return;

	atomic_inc(&ssg_blkg->current_rqs);
}

void ssg_blkcg_dec_rq(struct blkcg_gq *blkg)
{
	struct ssg_blkg *ssg_blkg = BLKG_TO_SSG_BLKG(blkg);

	if (IS_ERR_OR_NULL(ssg_blkg))
		return;

	atomic_dec(&ssg_blkg->current_rqs);
}

static int ssg_blkcg_show_max_available_ratio(struct seq_file *sf, void *v)
{
	struct ssg_blkcg *ssg_blkcg = CSS_TO_SSG_BLKCG(seq_css(sf));

	if (IS_ERR_OR_NULL(ssg_blkcg))
		return -EINVAL;

	seq_printf(sf, "%d\n", ssg_blkcg->max_available_ratio);

	return 0;
}

static int ssg_blkcg_set_max_available_ratio(struct cgroup_subsys_state *css,
		struct cftype *cftype, u64 ratio)
{
	struct blkcg *blkcg = css_to_blkcg(css);
	struct ssg_blkcg *ssg_blkcg = CSS_TO_SSG_BLKCG(css);
	struct blkcg_gq *blkg;
	struct ssg_blkg *ssg_blkg;

	if (IS_ERR_OR_NULL(ssg_blkcg))
		return -EINVAL;

	if (ratio > 100)
		return -EINVAL;

	spin_lock_irq(&blkcg->lock);
	ssg_blkcg->max_available_ratio = ratio;
	hlist_for_each_entry(blkg, &blkcg->blkg_list, blkcg_node) {
		ssg_blkg = BLKG_TO_SSG_BLKG(blkg);
		if (IS_ERR_OR_NULL(ssg_blkg))
			continue;

		ssg_blkcg_set_shallow_depth(ssg_blkcg, ssg_blkg,
				blkg->q->queue_hw_ctx[0]->sched_tags);
	}
	spin_unlock_irq(&blkcg->lock);

	return 0;
}

struct cftype ssg_blkg_files[] = {
	{
		.name = "ssg.max_available_ratio",
		.flags = CFTYPE_NOT_ON_ROOT,
		.seq_show = ssg_blkcg_show_max_available_ratio,
		.write_u64 = ssg_blkcg_set_max_available_ratio,
	},

	{} /* terminate */
};

static struct blkcg_policy ssg_blkcg_policy = {
	.legacy_cftypes = ssg_blkg_files,

	.cpd_alloc_fn = ssg_blkcg_cpd_alloc,
	.cpd_init_fn = ssg_blkcg_cpd_init,
	.cpd_free_fn = ssg_blkcg_cpd_free,

	.pd_alloc_fn = ssg_blkcg_pd_alloc,
	.pd_init_fn = ssg_blkcg_pd_init,
	.pd_free_fn = ssg_blkcg_pd_free,
};

int ssg_blkcg_activate(struct request_queue *q)
{
	return blkcg_activate_policy(q, &ssg_blkcg_policy);
}

void ssg_blkcg_deactivate(struct request_queue *q)
{
	blkcg_deactivate_policy(q, &ssg_blkcg_policy);
}

int ssg_blkcg_init(void)
{
	return blkcg_policy_register(&ssg_blkcg_policy);
}

void ssg_blkcg_exit(void)
{
	blkcg_policy_unregister(&ssg_blkcg_policy);
}
