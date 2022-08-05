/* SPDX-License-Identifier: GPL-2.0 */
#ifndef SSG_CGROUP_H
#define SSG_CGROUP_H
#include <linux/blk-cgroup.h>

#if IS_ENABLED(CONFIG_MQ_IOSCHED_SSG_CGROUP)
struct ssg_blkcg {
	struct blkcg_policy_data cpd; /* must be the first member */

	int max_available_ratio;
};

struct ssg_blkg {
	struct blkg_policy_data pd; /* must be the first member */

	atomic_t current_rqs;
	int max_available_rqs;
	unsigned int shallow_depth; /* shallow depth for each tag map to get sched tag */
};

extern int ssg_blkcg_init(void);
extern void ssg_blkcg_exit(void);
extern int ssg_blkcg_activate(struct request_queue *q);
extern void ssg_blkcg_deactivate(struct request_queue *q);
extern unsigned int ssg_blkcg_shallow_depth(struct request_queue *q);
extern void ssg_blkcg_depth_updated(struct blk_mq_hw_ctx *hctx);
extern void ssg_blkcg_inc_rq(struct blkcg_gq *blkg);
extern void ssg_blkcg_dec_rq(struct blkcg_gq *blkg);
#else
int ssg_blkcg_init(void)
{
	return 0;
}
void ssg_blkcg_exit(void)
{
}

int ssg_blkcg_activate(struct request_queue *q)
{
	return 0;
}

void ssg_blkcg_deactivate(struct request_queue *q)
{
}

unsigned int ssg_blkcg_shallow_depth(struct request_queue *q)
{
	return 0;
}

void ssg_blkcg_depth_updated(struct blk_mq_hw_ctx *hctx)
{
}

void ssg_blkcg_inc_rq(struct blkcg_gq *blkg)
{
}

void ssg_blkcg_dec_rq(struct blkcg_gq *blkg)
{
}
#endif

#endif
