// SPDX-License-Identifier: GPL-2.0
/*
 *  SamSung Generic I/O scheduler
 *  for the blk-mq scheduling framework
 *
 *  Copyright (C) 2021 Jisoo Oh <jisoo2146.oh@samsung.com>
 *  Copyright (C) 2021 Manjong Lee <mj0123.lee@samsung.com>
 *  Copyright (C) 2021 Changheun Lee <nanich.lee@samsung.com>
 */
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/blkdev.h>
#include <linux/blk-mq.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>
#include <linux/compiler.h>
#include <linux/rbtree.h>
#include <linux/sbitmap.h>

#include "blk.h"
#include "blk-mq.h"
#include "blk-mq-debugfs.h"
#include "blk-mq-tag.h"
#include "blk-mq-sched.h"
#include "ssg-cgroup.h"

#if IS_ENABLED(CONFIG_BLK_SEC_STATS)
extern void blk_sec_stats_account_init(struct request_queue *q);
extern void blk_sec_stats_account_exit(struct elevator_queue *eq);
extern void blk_sec_stats_account_io_done(
		struct request *rq, unsigned int data_size,
		pid_t tgid, const char *tg_name, u64 tg_start_time);
#else
#define blk_sec_stats_account_init(q)	do {} while(0)
#define blk_sec_stats_account_exit(eq)	do {} while(0)
#define blk_sec_stats_account_io_done(rq, size, tgid, name, time) do {} while(0)
#endif

#define MAX_ASYNC_WRITE_RQS	8

static const int read_expire = HZ / 2;		/* max time before a read is submitted. */
static const int write_expire = 5 * HZ;		/* ditto for writes, these limits are SOFT! */
static const int max_write_starvation = 2;	/* max times reads can starve a write */
static const int congestion_threshold = 90;	/* percentage of congestion threshold */
static const int max_tgroup_io_ratio = 50;	/* maximum service ratio for each thread group */
static const int max_async_write_ratio = 25;	/* maximum service ratio for async write */

struct ssg_request_info {
	pid_t tgid;
	char tg_name[TASK_COMM_LEN];
	u64 tg_start_time;

	struct blkcg_gq *blkg;

	unsigned int data_size;
};

struct ssg_data {
	struct request_queue *queue;

	/*
	 * requests are present on both sort_list and fifo_list
	 */
	struct rb_root sort_list[2];
	struct list_head fifo_list[2];

	/*
	 * next in sort order. read, write or both are NULL
	 */
	struct request *next_rq[2];
	unsigned int starved_writes;	/* times reads have starved writes */

	/*
	 * settings that change how the i/o scheduler behaves
	 */
	int fifo_expire[2];
	int max_write_starvation;
	int front_merges;

	/*
	 * to control request allocation
	 */
	atomic_t allocated_rqs;
	atomic_t async_write_rqs;
	int congestion_threshold_rqs;
	int max_tgroup_rqs;
	int max_async_write_rqs;
	unsigned int tgroup_shallow_depth;	/* thread group shallow depth for each tag map */
	unsigned int async_write_shallow_depth;	/* async write shallow depth for each tag map */

	/*
	 * I/O context information for each request
	 */
	struct ssg_request_info *rq_info;

	spinlock_t lock;
	spinlock_t zone_lock;
	struct list_head dispatch;
};

static inline struct rb_root *ssg_rb_root(struct ssg_data *ssg, struct request *rq)
{
	return &ssg->sort_list[rq_data_dir(rq)];
}

/*
 * get the request after `rq' in sector-sorted order
 */
static inline struct request *ssg_latter_request(struct request *rq)
{
	struct rb_node *node = rb_next(&rq->rb_node);

	if (node)
		return rb_entry_rq(node);

	return NULL;
}

static void ssg_add_rq_rb(struct ssg_data *ssg, struct request *rq)
{
	struct rb_root *root = ssg_rb_root(ssg, rq);

	elv_rb_add(root, rq);
}

static inline void ssg_del_rq_rb(struct ssg_data *ssg, struct request *rq)
{
	const int data_dir = rq_data_dir(rq);

	if (ssg->next_rq[data_dir] == rq)
		ssg->next_rq[data_dir] = ssg_latter_request(rq);

	elv_rb_del(ssg_rb_root(ssg, rq), rq);
}

static inline struct ssg_request_info *ssg_rq_info(struct ssg_data *ssg,
		struct request *rq)
{
	if (unlikely(!ssg->rq_info))
		return NULL;

	if (unlikely(!rq))
		return NULL;

	if (unlikely(rq->internal_tag < 0))
		return NULL;

	if (unlikely(rq->internal_tag >= rq->q->nr_requests))
		return NULL;

	return &ssg->rq_info[rq->internal_tag];
}

static inline void set_thread_group_info(struct ssg_request_info *rqi)
{
	struct task_struct *gleader = current->group_leader;

	rqi->tgid = task_tgid_nr(gleader);
	strncpy(rqi->tg_name, gleader->comm, TASK_COMM_LEN - 1);
	rqi->tg_name[TASK_COMM_LEN - 1] = '\0';
	rqi->tg_start_time = gleader->start_time;
}

static inline void clear_thread_group_info(struct ssg_request_info *rqi)
{
	rqi->tgid = 0;
	rqi->tg_name[0] = '\0';
	rqi->tg_start_time = 0;
}

/*
 * remove rq from rbtree and fifo.
 */
static void ssg_remove_request(struct request_queue *q, struct request *rq)
{
	struct ssg_data *ssg = q->elevator->elevator_data;

	list_del_init(&rq->queuelist);

	/*
	 * We might not be on the rbtree, if we are doing an insert merge
	 */
	if (!RB_EMPTY_NODE(&rq->rb_node))
		ssg_del_rq_rb(ssg, rq);

	elv_rqhash_del(q, rq);
	if (q->last_merge == rq)
		q->last_merge = NULL;
}

static void ssg_request_merged(struct request_queue *q, struct request *req,
			      enum elv_merge type)
{
	struct ssg_data *ssg = q->elevator->elevator_data;

	/*
	 * if the merge was a front merge, we need to reposition request
	 */
	if (type == ELEVATOR_FRONT_MERGE) {
		elv_rb_del(ssg_rb_root(ssg, req), req);
		ssg_add_rq_rb(ssg, req);
	}
}

static void ssg_merged_requests(struct request_queue *q, struct request *req,
			       struct request *next)
{
	/*
	 * if next expires before rq, assign its expire time to rq
	 * and move into next position (next will be deleted) in fifo
	 */
	if (!list_empty(&req->queuelist) && !list_empty(&next->queuelist)) {
		if (time_before((unsigned long)next->fifo_time,
				(unsigned long)req->fifo_time)) {
			list_move(&req->queuelist, &next->queuelist);
			req->fifo_time = next->fifo_time;
		}
	}

	/*
	 * kill knowledge of next, this one is a goner
	 */
	ssg_remove_request(q, next);
}

/*
 * move an entry to dispatch queue
 */
static void ssg_move_request(struct ssg_data *ssg, struct request *rq)
{
	const int data_dir = rq_data_dir(rq);

	ssg->next_rq[READ] = NULL;
	ssg->next_rq[WRITE] = NULL;
	ssg->next_rq[data_dir] = ssg_latter_request(rq);

	/*
	 * take it off the sort and fifo list
	 */
	ssg_remove_request(rq->q, rq);
}

/*
 * ssg_check_fifo returns 0 if there are no expired requests on the fifo,
 * 1 otherwise. Requires !list_empty(&ssg->fifo_list[data_dir])
 */
static inline int ssg_check_fifo(struct ssg_data *ssg, int ddir)
{
	struct request *rq = rq_entry_fifo(ssg->fifo_list[ddir].next);

	/*
	 * rq is expired!
	 */
	if (time_after_eq(jiffies, (unsigned long)rq->fifo_time))
		return 1;

	return 0;
}

/*
 * For the specified data direction, return the next request to
 * dispatch using arrival ordered lists.
 */
static struct request *ssg_fifo_request(struct ssg_data *ssg, int data_dir)
{
	struct request *rq;
	unsigned long flags;

	if (WARN_ON_ONCE(data_dir != READ && data_dir != WRITE))
		return NULL;

	if (list_empty(&ssg->fifo_list[data_dir]))
		return NULL;

	rq = rq_entry_fifo(ssg->fifo_list[data_dir].next);
	if (data_dir == READ || !blk_queue_is_zoned(rq->q))
		return rq;

	/*
	 * Look for a write request that can be dispatched, that is one with
	 * an unlocked target zone.
	 */
	spin_lock_irqsave(&ssg->zone_lock, flags);
	list_for_each_entry(rq, &ssg->fifo_list[WRITE], queuelist) {
		if (blk_req_can_dispatch_to_zone(rq))
			goto out;
	}
	rq = NULL;
out:
	spin_unlock_irqrestore(&ssg->zone_lock, flags);

	return rq;
}

/*
 * For the specified data direction, return the next request to
 * dispatch using sector position sorted lists.
 */
static struct request *ssg_next_request(struct ssg_data *ssg, int data_dir)
{
	struct request *rq;
	unsigned long flags;

	if (WARN_ON_ONCE(data_dir != READ && data_dir != WRITE))
		return NULL;

	rq = ssg->next_rq[data_dir];
	if (!rq)
		return NULL;

	if (data_dir == READ || !blk_queue_is_zoned(rq->q))
		return rq;

	/*
	 * Look for a write request that can be dispatched, that is one with
	 * an unlocked target zone.
	 */
	spin_lock_irqsave(&ssg->zone_lock, flags);
	while (rq) {
		if (blk_req_can_dispatch_to_zone(rq))
			break;
		rq = ssg_latter_request(rq);
	}
	spin_unlock_irqrestore(&ssg->zone_lock, flags);

	return rq;
}

/*
 * ssg_dispatch_requests selects the best request according to
 * read/write expire, etc
 */
static struct request *__ssg_dispatch_request(struct ssg_data *ssg)
{
	struct request *rq, *next_rq;
	bool reads, writes;
	int data_dir;

	if (!list_empty(&ssg->dispatch)) {
		rq = list_first_entry(&ssg->dispatch, struct request, queuelist);
		list_del_init(&rq->queuelist);
		goto done;
	}

	reads = !list_empty(&ssg->fifo_list[READ]);
	writes = !list_empty(&ssg->fifo_list[WRITE]);

	/*
	 * select the appropriate data direction (read / write)
	 */

	if (reads) {
		BUG_ON(RB_EMPTY_ROOT(&ssg->sort_list[READ]));

		if (ssg_fifo_request(ssg, WRITE) &&
		    (ssg->starved_writes++ >= ssg->max_write_starvation))
			goto dispatch_writes;

		data_dir = READ;

		goto dispatch_find_request;
	}

	/*
	 * there are either no reads or writes have been starved
	 */

	if (writes) {
dispatch_writes:
		BUG_ON(RB_EMPTY_ROOT(&ssg->sort_list[WRITE]));

		ssg->starved_writes = 0;

		data_dir = WRITE;

		goto dispatch_find_request;
	}

	return NULL;

dispatch_find_request:
	/*
	 * we are not running a batch, find best request for selected data_dir
	 */
	next_rq = ssg_next_request(ssg, data_dir);
	if (ssg_check_fifo(ssg, data_dir) || !next_rq) {
		/*
		 * A deadline has expired, the last request was in the other
		 * direction, or we have run out of higher-sectored requests.
		 * Start again from the request with the earliest expiry time.
		 */
		rq = ssg_fifo_request(ssg, data_dir);
	} else {
		/*
		 * The last req was the same dir and we have a next request in
		 * sort order. No expired requests so continue on from here.
		 */
		rq = next_rq;
	}

	/*
	 * For a zoned block device, if we only have writes queued and none of
	 * them can be dispatched, rq will be NULL.
	 */
	if (!rq)
		return NULL;

	/*
	 * rq is the selected appropriate request.
	 */
	ssg_move_request(ssg, rq);
done:
	/*
	 * If the request needs its target zone locked, do it.
	 */
	blk_req_zone_write_lock(rq);
	rq->rq_flags |= RQF_STARTED;
	return rq;
}

/*
 * One confusing aspect here is that we get called for a specific
 * hardware queue, but we may return a request that is for a
 * different hardware queue. This is because ssg-iosched has shared
 * state for all hardware queues, in terms of sorting, FIFOs, etc.
 */
static struct request *ssg_dispatch_request(struct blk_mq_hw_ctx *hctx)
{
	struct ssg_data *ssg = hctx->queue->elevator->elevator_data;
	struct request *rq;
	struct ssg_request_info *rqi;

	spin_lock(&ssg->lock);
	rq = __ssg_dispatch_request(ssg);
	spin_unlock(&ssg->lock);

	rqi = ssg_rq_info(ssg, rq);
	if (likely(rqi))
		rqi->data_size = blk_rq_bytes(rq);

	return rq;
}

static void ssg_completed_request(struct request *rq, u64 now)
{
	struct ssg_data *ssg = rq->q->elevator->elevator_data;
	struct ssg_request_info *rqi;

	rqi = ssg_rq_info(ssg, rq);
	if (likely(rqi))
		blk_sec_stats_account_io_done(rq, rqi->data_size,
				rqi->tgid, rqi->tg_name, rqi->tg_start_time);
}

static void ssg_set_shallow_depth(struct ssg_data *ssg, struct blk_mq_tags *tags)
{
	unsigned int depth = tags->bitmap_tags->sb.depth;
	unsigned int map_nr = tags->bitmap_tags->sb.map_nr;

	ssg->max_async_write_rqs = depth * max_async_write_ratio / 100U;
	ssg->max_async_write_rqs =
		min_t(int, ssg->max_async_write_rqs, MAX_ASYNC_WRITE_RQS);
	ssg->async_write_shallow_depth =
		max_t(unsigned int, ssg->max_async_write_rqs / map_nr, 1);

	ssg->max_tgroup_rqs = depth * max_tgroup_io_ratio / 100U;
	ssg->tgroup_shallow_depth =
		max_t(unsigned int, ssg->max_tgroup_rqs / map_nr, 1);
}

static void ssg_depth_updated(struct blk_mq_hw_ctx *hctx)
{
	struct request_queue *q = hctx->queue;
	struct ssg_data *ssg = q->elevator->elevator_data;
	struct blk_mq_tags *tags = hctx->sched_tags;
	unsigned int depth = tags->bitmap_tags->sb.depth;

	ssg->congestion_threshold_rqs = depth * congestion_threshold / 100U;

	kfree(ssg->rq_info);
	ssg->rq_info = kmalloc(depth * sizeof(struct ssg_request_info),
			GFP_KERNEL | __GFP_ZERO);
	if (ZERO_OR_NULL_PTR(ssg->rq_info))
		ssg->rq_info = NULL;

	ssg_set_shallow_depth(ssg, tags);
	sbitmap_queue_min_shallow_depth(tags->bitmap_tags,
			ssg->async_write_shallow_depth);

	ssg_blkcg_depth_updated(hctx);
}

static inline bool ssg_op_is_async_write(unsigned int op)
{
	return (op & REQ_OP_MASK) == REQ_OP_WRITE && !op_is_sync(op);
}

static unsigned int ssg_async_write_shallow_depth(unsigned int op,
		struct blk_mq_alloc_data *data)
{
	struct ssg_data *ssg = data->q->elevator->elevator_data;

	if (!ssg_op_is_async_write(op))
		return 0;

	if (atomic_read(&ssg->async_write_rqs) < ssg->max_async_write_rqs)
		return 0;

	return ssg->async_write_shallow_depth;
}

static unsigned int ssg_tgroup_shallow_depth(struct blk_mq_alloc_data *data)
{
	struct ssg_data *ssg = data->q->elevator->elevator_data;
	pid_t tgid = task_tgid_nr(current->group_leader);
	int nr_requests = data->q->nr_requests;
	int tgroup_rqs = 0;
	int i;

	if (unlikely(!ssg->rq_info))
		return 0;

	for (i = 0; i < nr_requests; i++)
		if (tgid == ssg->rq_info[i].tgid)
			tgroup_rqs++;

	if (tgroup_rqs < ssg->max_tgroup_rqs)
		return 0;

	return ssg->tgroup_shallow_depth;
}

static void ssg_limit_depth(unsigned int op, struct blk_mq_alloc_data *data)
{
	struct ssg_data *ssg = data->q->elevator->elevator_data;
	unsigned int shallow_depth = ssg_blkcg_shallow_depth(data->q);

	shallow_depth = min_not_zero(shallow_depth,
			ssg_async_write_shallow_depth(op, data));

	if (atomic_read(&ssg->allocated_rqs) > ssg->congestion_threshold_rqs)
		shallow_depth = min_not_zero(shallow_depth,
				ssg_tgroup_shallow_depth(data));

	data->shallow_depth = shallow_depth;
}

static int ssg_init_hctx(struct blk_mq_hw_ctx *hctx, unsigned int hctx_idx)
{
	struct ssg_data *ssg = hctx->queue->elevator->elevator_data;
	struct blk_mq_tags *tags = hctx->sched_tags;

	ssg_set_shallow_depth(ssg, tags);
	sbitmap_queue_min_shallow_depth(tags->bitmap_tags,
			ssg->async_write_shallow_depth);

	return 0;
}

static void ssg_exit_queue(struct elevator_queue *e)
{
	struct ssg_data *ssg = e->elevator_data;

	ssg_blkcg_deactivate(ssg->queue);

	BUG_ON(!list_empty(&ssg->fifo_list[READ]));
	BUG_ON(!list_empty(&ssg->fifo_list[WRITE]));

	kfree(ssg->rq_info);
	kfree(ssg);

	blk_sec_stats_account_exit(e);
}

/*
 * initialize elevator private data (ssg_data).
 */
static int ssg_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct ssg_data *ssg;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	ssg = kzalloc_node(sizeof(*ssg), GFP_KERNEL, q->node);
	if (!ssg) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}
	eq->elevator_data = ssg;

	ssg->queue = q;
	INIT_LIST_HEAD(&ssg->fifo_list[READ]);
	INIT_LIST_HEAD(&ssg->fifo_list[WRITE]);
	ssg->sort_list[READ] = RB_ROOT;
	ssg->sort_list[WRITE] = RB_ROOT;
	ssg->fifo_expire[READ] = read_expire;
	ssg->fifo_expire[WRITE] = write_expire;
	ssg->max_write_starvation = max_write_starvation;
	ssg->front_merges = 1;

	atomic_set(&ssg->allocated_rqs, 0);
	atomic_set(&ssg->async_write_rqs, 0);
	ssg->congestion_threshold_rqs =
		q->nr_requests * congestion_threshold / 100U;
	ssg->rq_info = kmalloc(q->nr_requests * sizeof(struct ssg_request_info),
			GFP_KERNEL | __GFP_ZERO);
	if (ZERO_OR_NULL_PTR(ssg->rq_info))
		ssg->rq_info = NULL;

	spin_lock_init(&ssg->lock);
	spin_lock_init(&ssg->zone_lock);
	INIT_LIST_HEAD(&ssg->dispatch);

	ssg_blkcg_activate(q);

	q->elevator = eq;

	blk_sec_stats_account_init(q);
	return 0;
}

static int ssg_request_merge(struct request_queue *q, struct request **rq,
			    struct bio *bio)
{
	struct ssg_data *ssg = q->elevator->elevator_data;
	sector_t sector = bio_end_sector(bio);
	struct request *__rq;

	if (!ssg->front_merges)
		return ELEVATOR_NO_MERGE;

	__rq = elv_rb_find(&ssg->sort_list[bio_data_dir(bio)], sector);
	if (__rq) {
		BUG_ON(sector != blk_rq_pos(__rq));

		if (elv_bio_merge_ok(__rq, bio)) {
			*rq = __rq;

			if (blk_discard_mergable(__rq))
				return ELEVATOR_DISCARD_MERGE;

			return ELEVATOR_FRONT_MERGE;
		}
	}

	return ELEVATOR_NO_MERGE;
}

static bool ssg_bio_merge(struct request_queue *q, struct bio *bio,
		unsigned int nr_segs)
{
	struct ssg_data *ssg = q->elevator->elevator_data;
	struct request *free = NULL;
	bool ret;

	spin_lock(&ssg->lock);
	ret = blk_mq_sched_try_merge(q, bio, nr_segs, &free);
	spin_unlock(&ssg->lock);

	if (free)
		blk_mq_free_request(free);

	return ret;
}

/*
 * add rq to rbtree and fifo
 */
static void ssg_insert_request(struct blk_mq_hw_ctx *hctx, struct request *rq,
			      bool at_head)
{
	struct request_queue *q = hctx->queue;
	struct ssg_data *ssg = q->elevator->elevator_data;
	const int data_dir = rq_data_dir(rq);


	/*
	 * This may be a requeue of a write request that has locked its
	 * target zone. If it is the case, this releases the zone lock.
	 */
	blk_req_zone_write_unlock(rq);

	if (blk_mq_sched_try_insert_merge(q, rq))
		return;

	blk_mq_sched_request_inserted(rq);

	if (at_head || blk_rq_is_passthrough(rq)) {
		if (at_head)
			list_add(&rq->queuelist, &ssg->dispatch);
		else
			list_add_tail(&rq->queuelist, &ssg->dispatch);
	} else {
		ssg_add_rq_rb(ssg, rq);

		if (rq_mergeable(rq)) {
			elv_rqhash_add(q, rq);
			if (!q->last_merge)
				q->last_merge = rq;
		}

		/*
		 * set expire time and add to fifo list
		 */
		rq->fifo_time = jiffies + ssg->fifo_expire[data_dir];
		list_add_tail(&rq->queuelist, &ssg->fifo_list[data_dir]);
	}
}

static void ssg_insert_requests(struct blk_mq_hw_ctx *hctx,
			       struct list_head *list, bool at_head)
{
	struct request_queue *q = hctx->queue;
	struct ssg_data *ssg = q->elevator->elevator_data;

	spin_lock(&ssg->lock);
	while (!list_empty(list)) {
		struct request *rq;

		rq = list_first_entry(list, struct request, queuelist);
		list_del_init(&rq->queuelist);
		ssg_insert_request(hctx, rq, at_head);
	}
	spin_unlock(&ssg->lock);
}

/*
 * Nothing to do here. This is defined only to ensure that .finish_request
 * method is called upon request completion.
 */
static void ssg_prepare_request(struct request *rq)
{
	struct ssg_data *ssg = rq->q->elevator->elevator_data;
	struct ssg_request_info *rqi;

	atomic_inc(&ssg->allocated_rqs);

	rqi = ssg_rq_info(ssg, rq);
	if (likely(rqi)) {
		set_thread_group_info(rqi);

		rcu_read_lock();
		rqi->blkg = blkg_lookup(css_to_blkcg(blkcg_css()), rq->q);
		ssg_blkcg_inc_rq(rqi->blkg);
		rcu_read_unlock();
	}

	if (ssg_op_is_async_write(rq->cmd_flags))
		atomic_inc(&ssg->async_write_rqs);
}

/*
 * For zoned block devices, write unlock the target zone of
 * completed write requests. Do this while holding the zone lock
 * spinlock so that the zone is never unlocked while ssg_fifo_request()
 * or ssg_next_request() are executing. This function is called for
 * all requests, whether or not these requests complete successfully.
 *
 * For a zoned block device, __ssg_dispatch_request() may have stopped
 * dispatching requests if all the queued requests are write requests directed
 * at zones that are already locked due to on-going write requests. To ensure
 * write request dispatch progress in this case, mark the queue as needing a
 * restart to ensure that the queue is run again after completion of the
 * request and zones being unlocked.
 */
static void ssg_finish_request(struct request *rq)
{
	struct request_queue *q = rq->q;
	struct ssg_data *ssg = q->elevator->elevator_data;
	struct ssg_request_info *rqi;

	if (blk_queue_is_zoned(q)) {
		unsigned long flags;

		spin_lock_irqsave(&ssg->zone_lock, flags);
		blk_req_zone_write_unlock(rq);
		if (!list_empty(&ssg->fifo_list[WRITE]))
			blk_mq_sched_mark_restart_hctx(rq->mq_hctx);
		spin_unlock_irqrestore(&ssg->zone_lock, flags);
	}

	if (unlikely(!(rq->rq_flags & RQF_ELVPRIV)))
		return;

	atomic_dec(&ssg->allocated_rqs);

	rqi = ssg_rq_info(ssg, rq);
	if (likely(rqi)) {
		clear_thread_group_info(rqi);
		ssg_blkcg_dec_rq(rqi->blkg);
		rqi->blkg = NULL;
	}

	if (ssg_op_is_async_write(rq->cmd_flags))
		atomic_dec(&ssg->async_write_rqs);
}

static bool ssg_has_work(struct blk_mq_hw_ctx *hctx)
{
	struct ssg_data *ssg = hctx->queue->elevator->elevator_data;

	return !list_empty_careful(&ssg->dispatch) ||
		!list_empty_careful(&ssg->fifo_list[0]) ||
		!list_empty_careful(&ssg->fifo_list[1]);
}

/*
 * sysfs parts below
 */
static ssize_t ssg_var_show(int var, char *page)
{
	return sprintf(page, "%d\n", var);
}

static void ssg_var_store(int *var, const char *page)
{
	char *p = (char *) page;

	*var = simple_strtol(p, &p, 10);
}

#define SHOW_FUNCTION(__FUNC, __VAR, __CONV)				\
static ssize_t __FUNC(struct elevator_queue *e, char *page)		\
{									\
	struct ssg_data *ssg = e->elevator_data;			\
	int __data = __VAR;						\
	if (__CONV)							\
		__data = jiffies_to_msecs(__data);			\
	return ssg_var_show(__data, (page));				\
}
SHOW_FUNCTION(ssg_read_expire_show, ssg->fifo_expire[READ], 1);
SHOW_FUNCTION(ssg_write_expire_show, ssg->fifo_expire[WRITE], 1);
SHOW_FUNCTION(ssg_max_write_starvation_show, ssg->max_write_starvation, 0);
SHOW_FUNCTION(ssg_front_merges_show, ssg->front_merges, 0);
SHOW_FUNCTION(ssg_tgroup_shallow_depth_show, ssg->tgroup_shallow_depth, 0);
SHOW_FUNCTION(ssg_async_write_shallow_depth_show, ssg->async_write_shallow_depth, 0);
#undef SHOW_FUNCTION

#define STORE_FUNCTION(__FUNC, __PTR, MIN, MAX, __CONV)			\
static ssize_t __FUNC(struct elevator_queue *e, const char *page, size_t count)	\
{									\
	struct ssg_data *ssg = e->elevator_data;			\
	int __data;							\
	ssg_var_store(&__data, (page));					\
	if (__data < (MIN))						\
		__data = (MIN);						\
	else if (__data > (MAX))					\
		__data = (MAX);						\
	if (__CONV)							\
		*(__PTR) = msecs_to_jiffies(__data);			\
	else								\
		*(__PTR) = __data;					\
	return count;							\
}
STORE_FUNCTION(ssg_read_expire_store, &ssg->fifo_expire[READ], 0, INT_MAX, 1);
STORE_FUNCTION(ssg_write_expire_store, &ssg->fifo_expire[WRITE], 0, INT_MAX, 1);
STORE_FUNCTION(ssg_max_write_starvation_store, &ssg->max_write_starvation, INT_MIN, INT_MAX, 0);
STORE_FUNCTION(ssg_front_merges_store, &ssg->front_merges, 0, 1, 0);
#undef STORE_FUNCTION

#define SSG_ATTR(name) \
	__ATTR(name, 0644, ssg_##name##_show, ssg_##name##_store)

#define SSG_ATTR_RO(name) \
	__ATTR(name, 0444, ssg_##name##_show, NULL)

static struct elv_fs_entry ssg_attrs[] = {
	SSG_ATTR(read_expire),
	SSG_ATTR(write_expire),
	SSG_ATTR(max_write_starvation),
	SSG_ATTR(front_merges),
	SSG_ATTR_RO(tgroup_shallow_depth),
	SSG_ATTR_RO(async_write_shallow_depth),
	__ATTR_NULL
};

#ifdef CONFIG_BLK_DEBUG_FS
#define SSG_DEBUGFS_DDIR_ATTRS(ddir, name)				\
static void *ssg_##name##_fifo_start(struct seq_file *m,		\
					  loff_t *pos)			\
	__acquires(&ssg->lock)						\
{									\
	struct request_queue *q = m->private;				\
	struct ssg_data *ssg = q->elevator->elevator_data;		\
									\
	spin_lock(&ssg->lock);						\
	return seq_list_start(&ssg->fifo_list[ddir], *pos);		\
}									\
									\
static void *ssg_##name##_fifo_next(struct seq_file *m, void *v,	\
					 loff_t *pos)			\
{									\
	struct request_queue *q = m->private;				\
	struct ssg_data *ssg = q->elevator->elevator_data;		\
									\
	return seq_list_next(v, &ssg->fifo_list[ddir], pos);		\
}									\
									\
static void ssg_##name##_fifo_stop(struct seq_file *m, void *v)	\
	__releases(&ssg->lock)						\
{									\
	struct request_queue *q = m->private;				\
	struct ssg_data *ssg = q->elevator->elevator_data;		\
									\
	spin_unlock(&ssg->lock);					\
}									\
									\
static const struct seq_operations ssg_##name##_fifo_seq_ops = {	\
	.start	= ssg_##name##_fifo_start,				\
	.next	= ssg_##name##_fifo_next,				\
	.stop	= ssg_##name##_fifo_stop,				\
	.show	= blk_mq_debugfs_rq_show,				\
};									\
									\
static int ssg_##name##_next_rq_show(void *data,			\
					  struct seq_file *m)		\
{									\
	struct request_queue *q = data;					\
	struct ssg_data *ssg = q->elevator->elevator_data;		\
	struct request *rq = ssg->next_rq[ddir];			\
									\
	if (rq)								\
		__blk_mq_debugfs_rq_show(m, rq);			\
	return 0;							\
}
SSG_DEBUGFS_DDIR_ATTRS(READ, read)
SSG_DEBUGFS_DDIR_ATTRS(WRITE, write)
#undef SSG_DEBUGFS_DDIR_ATTRS

static int ssg_starved_writes_show(void *data, struct seq_file *m)
{
	struct request_queue *q = data;
	struct ssg_data *ssg = q->elevator->elevator_data;

	seq_printf(m, "%u\n", ssg->starved_writes);
	return 0;
}

static void *ssg_dispatch_start(struct seq_file *m, loff_t *pos)
	__acquires(&ssg->lock)
{
	struct request_queue *q = m->private;
	struct ssg_data *ssg = q->elevator->elevator_data;

	spin_lock(&ssg->lock);
	return seq_list_start(&ssg->dispatch, *pos);
}

static void *ssg_dispatch_next(struct seq_file *m, void *v, loff_t *pos)
{
	struct request_queue *q = m->private;
	struct ssg_data *ssg = q->elevator->elevator_data;

	return seq_list_next(v, &ssg->dispatch, pos);
}

static void ssg_dispatch_stop(struct seq_file *m, void *v)
	__releases(&ssg->lock)
{
	struct request_queue *q = m->private;
	struct ssg_data *ssg = q->elevator->elevator_data;

	spin_unlock(&ssg->lock);
}

static const struct seq_operations ssg_dispatch_seq_ops = {
	.start	= ssg_dispatch_start,
	.next	= ssg_dispatch_next,
	.stop	= ssg_dispatch_stop,
	.show	= blk_mq_debugfs_rq_show,
};

#define SSG_IOSCHED_QUEUE_DDIR_ATTRS(name)						\
	{#name "_fifo_list", 0400, .seq_ops = &ssg_##name##_fifo_seq_ops},	\
	{#name "_next_rq", 0400, ssg_##name##_next_rq_show}
static const struct blk_mq_debugfs_attr ssg_queue_debugfs_attrs[] = {
	SSG_IOSCHED_QUEUE_DDIR_ATTRS(read),
	SSG_IOSCHED_QUEUE_DDIR_ATTRS(write),
	{"starved_writes", 0400, ssg_starved_writes_show},
	{"dispatch", 0400, .seq_ops = &ssg_dispatch_seq_ops},
	{},
};
#undef SSG_IOSCHED_QUEUE_DDIR_ATTRS
#endif

static struct elevator_type ssg_iosched = {
	.ops = {
		.insert_requests = ssg_insert_requests,
		.dispatch_request = ssg_dispatch_request,
		.completed_request = ssg_completed_request,
		.prepare_request = ssg_prepare_request,
		.finish_request = ssg_finish_request,
		.next_request = elv_rb_latter_request,
		.former_request = elv_rb_former_request,
		.bio_merge = ssg_bio_merge,
		.request_merge = ssg_request_merge,
		.requests_merged = ssg_merged_requests,
		.request_merged = ssg_request_merged,
		.has_work = ssg_has_work,
		.limit_depth = ssg_limit_depth,
		.depth_updated = ssg_depth_updated,
		.init_hctx = ssg_init_hctx,
		.init_sched = ssg_init_queue,
		.exit_sched = ssg_exit_queue,
	},

#ifdef CONFIG_BLK_DEBUG_FS
	.queue_debugfs_attrs = ssg_queue_debugfs_attrs,
#endif
	.elevator_attrs = ssg_attrs,
	.elevator_name = "ssg",
	.elevator_alias = "ssg",
	.elevator_features = ELEVATOR_F_ZBD_SEQ_WRITE,
	.elevator_owner = THIS_MODULE,
};
MODULE_ALIAS("ssg");

static int __init ssg_iosched_init(void)
{
	int ret;

	ret = elv_register(&ssg_iosched);
	if (ret)
		return ret;

	ret = ssg_blkcg_init();
	if (ret) {
		elv_unregister(&ssg_iosched);
		return ret;
	}

	return ret;
}

static void __exit ssg_iosched_exit(void)
{
	ssg_blkcg_exit();
	elv_unregister(&ssg_iosched);
}

module_init(ssg_iosched_init);
module_exit(ssg_iosched_exit);

MODULE_AUTHOR("Jisoo Oh");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("SSG IO Scheduler");
