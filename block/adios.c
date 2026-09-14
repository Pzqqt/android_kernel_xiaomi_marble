// SPDX-License-Identifier: GPL-2.0
/*
 * Adaptive Deadline I/O Scheduler (ADIOS)
 * Copyright (C) 2025 Masahito Suzuki
 */
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/compiler.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/rbtree.h>
#include <linux/sbitmap.h>
#include <linux/mempool.h>
#include <linux/slab.h>
#include <linux/timekeeping.h>
#include <linux/percpu.h>
#include <linux/string.h>
#include <linux/list_sort.h>
#include <linux/rcupdate.h>

#include "blk.h"
#include "blk-mq.h"
#include "blk-mq-sched.h"

#define ADIOS_VERSION "3.3.0"

/* Request Types:
 *
 * Tier 0 (Highest Priority): Emergency & System Integrity Requests
 * -----------------------------------------------------------------
 * - Target: Requests with the at_head flag.
 * - Purpose: For critical, non-negotiable operations such as device error
 *   recovery or flush sequences that must bypass all other scheduling logic.
 * - Implementation: Placed in a dedicated, high-priority FIFO queue
 *   (`prio_queue[0]`) for immediate dispatch.
 *
 * Tier 1 (Medium Priority): Application Responsiveness
 * ----------------------------------------------------
 * - Target: Normal synchronous requests (e.g., from standard file reads).
 * - Purpose: To ensure correct application behavior for operations that
 *   depend on sequential I/O completion (e.g., file system mounts) and to
 *   provide low latency for interactive applications.
 * - Implementation: The deadline for these requests is set to their start
 *   time (`rq->start_time_ns`). This effectively enforces FIFO-like behavior
 *   within the deadline-sorted red-black tree, preventing out-of-order
 *   execution of dependent synchronous operations.
 *
 * Tier 2 (Normal Priority): Background Throughput
 * -----------------------------------------------
 * - Target: Asynchronous requests.
 * - Purpose: To maximize disk throughput for background tasks where latency
 *   is not critical.
 * - Implementation: These are the only requests where ADIOS's adaptive
 *   latency prediction model is used. A dynamic deadline is calculated based
 *   on the predicted I/O latency, allowing for aggressive reordering to
 *   optimize I/O efficiency.
 *
 * I/O barrier guarantees (REQ_PREFLUSH / REQ_FUA) are provided entirely by
 * the block layer's flush state machine (block/blk-flush.c), which is the
 * mechanism that replaced whole-queue barriers upstream in 2.6.37. No
 * queue-wide barrier is implemented or needed here:
 * - blk_insert_flush() always strips REQ_PREFLUSH (and REQ_FUA unless the
 *   device supports it in hardware) before any request ever reaches this
 *   elevator's ->insert_requests(), and always sets REQ_SYNC on it, so such
 *   a request lands in Tier 1 (immediate deadline) like any other
 *   synchronous request -- never delayed behind Tier 2 reordering.
 * - The data portion of a request that needed an actual PREFLUSH is
 *   resubmitted via BLK_MQ_INSERT_AT_HEAD once the flush completes, which
 *   lands in Tier 0 here, ahead of everything else.
 * - REQ_PREFLUSH/REQ_FUA and RQF_FLUSH_SEQ requests are already excluded
 *   from bio/request merging by the generic block layer (rq_mergeable(),
 *   blk_rq_merge_ok()) before this elevator's merge callbacks ever run.
 * - The synthesized pure REQ_OP_FLUSH command itself always bypasses this
 *   elevator entirely (blk_mq_insert_request() routes it straight to
 *   hctx->dispatch), so it never needs to be recognized or handled here.
 *
 * Dispatch Logic:
 * The scheduler always dispatches requests in strict priority order:
 * 1. prio_queue[0] (Tier 0)
 * 2. The deadline-sorted batch queue (which naturally prioritizes Tier 1
 *    over Tier 2 due to their calculated deadlines).
 */

// Global variable to control the latency
static u64 default_global_latency_window            = 16000000ULL;
static u64 default_global_latency_window_rotational = 22000000ULL;
// Ratio below which batch queues should be refilled
static u8  default_bq_refill_below_ratio = 20;
// Maximum latency sample to input
static u64 default_lat_model_latency_limit = 500 * NSEC_PER_MSEC;
// Batch ordering strategy
static u64 default_batch_order = 0;

/* Compliance Flags:
 * 0x1: Async requests will not be reordered based on the predicted latency
 */
enum adios_compliance_flags {
	ADIOS_CF_FIXORDER  = 1U << 0,
};

// Flags to control compliance with block layer constraints
static u64 default_compliance_flags = 0x0;

// Dynamic thresholds for shrinkage
static u32 default_lm_shrink_at_kreqs  =  5000;
static u32 default_lm_shrink_at_gbytes =    50;
static u32 default_lm_shrink_resist    =     2;

enum adios_optype {
	ADIOS_READ    = 0,
	ADIOS_WRITE   = 1,
	ADIOS_DISCARD = 2,
	ADIOS_OTHER   = 3,
	ADIOS_OPTYPES = 4,
};

// Latency targets for each operation type
static u64 default_latency_target[ADIOS_OPTYPES] = {
	[ADIOS_READ]    =     2ULL * NSEC_PER_MSEC,
	[ADIOS_WRITE]   =  2000ULL * NSEC_PER_MSEC,
	[ADIOS_DISCARD] =  8000ULL * NSEC_PER_MSEC,
	[ADIOS_OTHER]   =     0ULL * NSEC_PER_MSEC,
};

// Maximum batch size limits for each operation type
static u32 default_batch_limit[ADIOS_OPTYPES] = {
	[ADIOS_READ]    = 36,
	[ADIOS_WRITE]   = 72,
	[ADIOS_DISCARD] =  1,
	[ADIOS_OTHER]   =  1,
};

enum adios_batch_order {
	ADIOS_BO_OPTYPE   = 0,
	ADIOS_BO_ELEVATOR = 1,
};

// Thresholds for latency model control
#define LM_BLOCK_SIZE_THRESHOLD 4096
#define LM_SAMPLES_THRESHOLD    1024
#define LM_INTERVAL_THRESHOLD   1500
#define LM_OUTLIER_PERCENTILE     99
#define LM_LAT_BUCKET_COUNT       64

#define ADIOS_PQ_LEVELS 2
#define ADIOS_DL_TYPES  2
#define ADIOS_BQ_PAGES  2

static u32 default_dl_prio[ADIOS_DL_TYPES] = {8, 0};

// Bit flags for the atomic state variable, indicating which queues have requests.
enum adios_state_flags {
	ADIOS_STATE_PQ_0      = 1U << 0,
	ADIOS_STATE_PQ_1      = 1U << 1,
	ADIOS_STATE_DL_0      = 1U << 2,
	ADIOS_STATE_DL_1      = 1U << 3,
	ADIOS_STATE_BQ_PAGE_0 = 1U << 4,
	ADIOS_STATE_BQ_PAGE_1 = 1U << 5,
};
#define ADIOS_STATE_PQ 0
#define ADIOS_STATE_DL 2
#define ADIOS_STATE_BQ 4

// Temporal granularity of the deadline tree node (dl_group)
#define ADIOS_QUANTUM_SHIFT 20

#define ADIOS_MAX_INSERTS_PER_LOCK 72
#define ADIOS_MAX_DELETES_PER_LOCK 24

// Structure to hold latency bucket data for small requests
struct latency_bucket_small {
	u64 weighted_sum_latency;
	u64 sum_of_weights;
};

// Structure to hold latency bucket data for large requests
struct latency_bucket_large {
	u64 weighted_sum_latency;
	u64 weighted_sum_block_size;
	u64 sum_of_weights;
};

// Structure to hold per-cpu buckets, improving data locality and code clarity.
struct lm_buckets {
	struct latency_bucket_small small_bucket[LM_LAT_BUCKET_COUNT];
	struct latency_bucket_large large_bucket[LM_LAT_BUCKET_COUNT];
};

// Structure to hold RCU-protected latency model parameters
struct latency_model_params {
	u64 base;
	u64 slope;
	u64 small_sum_delay;
	u64 small_count;
	u64 large_sum_delay;
	u64 large_sum_bsize;
	u64 last_update_jiffies;
	struct rcu_head rcu;
};

// Structure to hold the latency model context data
struct latency_model {
	spinlock_t update_lock;
	struct latency_model_params __rcu *params;

	// Per-CPU buckets to avoid lock contention on the completion path
	struct lm_buckets __percpu *pcpu_buckets;
	// Per-CPU snapshots for delta-based aggregation (accessed under update_lock)
	struct lm_buckets __percpu *pcpu_snapshot;

	u32 lm_shrink_at_kreqs;
	u32 lm_shrink_at_gbytes;
	u8  lm_shrink_resist;
};

struct adios_pcpu_completion {
	u64      last_completed_time;
	sector_t last_completed_pos;
};

union adios_in_flight_rqs {
	atomic64_t	atomic;
	u64			scalar;
	struct {
		u64 	count:          16;
		u64 	total_pred_lat: 48;
	};
};

// Adios scheduler data
struct adios_data {
	spinlock_t pq_lock;
	struct list_head prio_queue[2];

	struct rb_root_cached dl_tree[2];
	spinlock_t lock;
	s64 dl_bias;
	s32 dl_prio[2];

	atomic_t state;
	u8  bq_state[ADIOS_BQ_PAGES];

	bool models_stable;

	u64 global_latency_window;
	u64 compliance_flags;
	u64 latency_target[ADIOS_OPTYPES];
	u32 batch_limit[ADIOS_OPTYPES];
	u32 batch_actual_max_size[ADIOS_OPTYPES];
	u32 batch_actual_max_total;
	u32 async_depth;
	u32 lat_model_latency_limit;
	u8  bq_refill_below_ratio;
	u8  is_rotational;
	u8  batch_order;
	u8  elv_direction;
	sector_t head_pos;

	bool bq_page;
	struct list_head batch_queue[ADIOS_BQ_PAGES][ADIOS_OPTYPES];
	u32 batch_count[ADIOS_BQ_PAGES][ADIOS_OPTYPES];
	u8  bq_batch_order[ADIOS_BQ_PAGES];
	spinlock_t bq_lock;

	struct lm_buckets *aggr_buckets;

	struct latency_model latency_model[ADIOS_OPTYPES];
	struct timer_list update_timer;

	union adios_in_flight_rqs in_flight_rqs;
	atomic64_t total_pred_lat;

	struct adios_pcpu_completion __percpu *pcpu_completion;

	struct kmem_cache *rq_data_cache;
	mempool_t *rq_data_pool;
	struct kmem_cache *dl_group_pool;

	struct request_queue *queue;
};

// List of requests with the same deadline in the deadline-sorted tree
struct dl_group {
	struct rb_node node;
	struct list_head rqs;
	u64 deadline;
} __attribute__((aligned(64)));

// Structure to hold scheduler-specific data for each request
struct adios_rq_data {
	struct list_head *dl_group;
	struct list_head dl_node;

	struct request *rq;
	u64 deadline;
	u64 pred_lat;
	u32 block_size;
	bool managed;
} __attribute__((aligned(64)));

static const int adios_prio_to_wmult[40] = {
 /* -20 */     88761,     71755,     56483,     46273,     36291,
 /* -15 */     29154,     23254,     18705,     14949,     11916,
 /* -10 */      9548,      7620,      6100,      4904,      3906,
 /*  -5 */      3121,      2501,      1991,      1586,      1277,
 /*   0 */      1024,       820,       655,       526,       423,
 /*   5 */       335,       272,       215,       172,       137,
 /*  10 */       110,        87,        70,        56,        45,
 /*  15 */        36,        29,        23,        18,        15,
};

static inline bool compliant(struct adios_data *ad, u32 flag) {
	return ad->compliance_flags & flag;
}

// Count the number of entries in aggregated small buckets
static u64 lm_count_small_entries(struct latency_bucket_small *buckets) {
	u64 total_weight = 0;
	for (u8 i = 0; i < LM_LAT_BUCKET_COUNT; i++)
		total_weight += buckets[i].sum_of_weights;
	return total_weight;
}

// Update the small buckets in the latency model from aggregated data
static bool lm_update_small_buckets(struct latency_model *model,
		struct latency_model_params *params,
		struct latency_bucket_small *buckets,
		u64 total_weight, bool count_all) {
	u64 sum_latency = 0;
	u64 sum_weight = 0;
	u64 cumulative_weight = 0, threshold_weight = 0;
	u8  outlier_threshold_bucket = 0;
	u8  outlier_percentile = LM_OUTLIER_PERCENTILE;
	u8  reduction;
	u8  i;

	if (count_all)
		outlier_percentile = 100;

	// Calculate the threshold weight for outlier detection
	threshold_weight = (total_weight * outlier_percentile) / 100;

	// Identify the bucket that corresponds to the outlier threshold
	for (i = 0; i < LM_LAT_BUCKET_COUNT; i++) {
		cumulative_weight += buckets[i].sum_of_weights;
		if (cumulative_weight >= threshold_weight) {
			outlier_threshold_bucket = i;
			break;
		}
	}

	// Calculate the average latency, excluding outliers
	for (i = 0; i <= outlier_threshold_bucket; i++) {
		struct latency_bucket_small *bucket = &buckets[i];
		if (i < outlier_threshold_bucket) {
			sum_latency += bucket->weighted_sum_latency;
			sum_weight += bucket->sum_of_weights;
		} else {
			// The threshold bucket's contribution is proportional
			u64 remaining_weight =
				threshold_weight - (cumulative_weight - bucket->sum_of_weights);
			if (bucket->sum_of_weights > 0) {
				sum_latency += div_u64(bucket->weighted_sum_latency *
					remaining_weight, bucket->sum_of_weights);
				sum_weight += remaining_weight;
			}
		}
	}

	// Shrink the model if it reaches at the readjustment threshold
	if (params->small_count >= 1000ULL * model->lm_shrink_at_kreqs) {
		reduction = model->lm_shrink_resist;
		if (params->small_count >> reduction) {
			params->small_sum_delay -= params->small_sum_delay >> reduction;
			params->small_count     -= params->small_count     >> reduction;
		}
	}

	if (!sum_weight)
		return false;

	// Accumulate the average latency into the statistics
	params->small_sum_delay += sum_latency;
	params->small_count     += sum_weight;

	return true;
}

// Count the number of entries in aggregated large buckets
static u64 lm_count_large_entries(struct latency_bucket_large *buckets) {
	u64 total_weight = 0;
	for (u8 i = 0; i < LM_LAT_BUCKET_COUNT; i++)
		total_weight += buckets[i].sum_of_weights;
	return total_weight;
}

// Update the large buckets in the latency model from aggregated data
static bool lm_update_large_buckets(struct latency_model *model,
		struct latency_model_params *params,
		struct latency_bucket_large *buckets,
		u64 total_weight, bool count_all) {
	s64 sum_latency = 0;
	u64 sum_block_size = 0, intercept;
	u64 cumulative_weight = 0, threshold_weight = 0;
	u64 sum_weight = 0;
	u8  outlier_threshold_bucket = 0;
	u8  outlier_percentile = LM_OUTLIER_PERCENTILE;
	u8  reduction;
	u8  i;

	if (count_all)
		outlier_percentile = 100;

	// Calculate the threshold weight for outlier detection
	threshold_weight = (total_weight * outlier_percentile) / 100;

	// Identify the bucket that corresponds to the outlier threshold
	for (i = 0; i < LM_LAT_BUCKET_COUNT; i++) {
		cumulative_weight += buckets[i].sum_of_weights;
		if (cumulative_weight >= threshold_weight) {
			outlier_threshold_bucket = i;
			break;
		}
	}

	// Calculate the average latency and block size, excluding outliers
	for (i = 0; i <= outlier_threshold_bucket; i++) {
		struct latency_bucket_large *bucket = &buckets[i];
		if (i < outlier_threshold_bucket) {
			sum_latency += bucket->weighted_sum_latency;
			sum_block_size += bucket->weighted_sum_block_size;
			sum_weight += bucket->sum_of_weights;
		} else {
			// The threshold bucket's contribution is proportional
			u64 remaining_weight =
				threshold_weight - (cumulative_weight - bucket->sum_of_weights);
			if (bucket->sum_of_weights > 0) {
				sum_latency += div_u64(bucket->weighted_sum_latency *
					remaining_weight, bucket->sum_of_weights);
				sum_block_size += div_u64(bucket->weighted_sum_block_size *
					remaining_weight, bucket->sum_of_weights);
				sum_weight += remaining_weight;
			}
		}
	}

	if (!sum_weight)
		return false;

	// Shrink the model if it reaches at the readjustment threshold
	if (params->large_sum_bsize >= 0x40000000ULL * model->lm_shrink_at_gbytes) {
		reduction = model->lm_shrink_resist;
		if (params->large_sum_bsize >> reduction) {
			params->large_sum_delay -= params->large_sum_delay >> reduction;
			params->large_sum_bsize -= params->large_sum_bsize >> reduction;
		}
	}

	// Accumulate the average delay into the statistics
	intercept = params->base;
	if (sum_latency > intercept)
		sum_latency -= intercept;

	params->large_sum_delay += sum_latency;
	params->large_sum_bsize += sum_block_size;

	return true;
}

static void reset_buckets(struct lm_buckets *buckets)
{ memset(buckets, 0, sizeof(*buckets)); }

static void lm_reset_pcpu_buckets(struct latency_model *model) {
	int cpu;
	for_each_possible_cpu(cpu) {
		reset_buckets(per_cpu_ptr(model->pcpu_buckets, cpu));
		reset_buckets(per_cpu_ptr(model->pcpu_snapshot, cpu));
	}
}

// Update the latency model parameters and statistics
static void latency_model_update(
		struct adios_data *ad, struct latency_model *model) {
	u64 now;
	u64 small_weight, large_weight;
	bool time_elapsed;
	bool small_processed = false, large_processed = false;
	struct lm_buckets *aggr = ad->aggr_buckets;
	struct latency_bucket_small *asb;
	struct latency_bucket_large *alb;
	struct lm_buckets *pcpu_b, *snap;
	unsigned long flags;
	int cpu;
	struct latency_model_params *old_params, *new_params;

	spin_lock_irqsave(&model->update_lock, flags);

	old_params = rcu_dereference_protected(model->params,
				lockdep_is_held(&model->update_lock));
	new_params = kmemdup(old_params, sizeof(*new_params), GFP_ATOMIC);
	if (!new_params) {
		spin_unlock_irqrestore(&model->update_lock, flags);
		return;
	}

	// Aggregate deltas from all CPUs using snapshot-delta method.
	// Per-CPU counters increase monotonically; we compute delta = current - snapshot.
	for_each_possible_cpu(cpu) {
		pcpu_b = per_cpu_ptr(model->pcpu_buckets, cpu);
		snap   = per_cpu_ptr(model->pcpu_snapshot, cpu);

		for (u8 i = 0; i < LM_LAT_BUCKET_COUNT; i++) {
			u64 sw  = pcpu_b->small_bucket[i].sum_of_weights;
			u64 sl  = pcpu_b->small_bucket[i].weighted_sum_latency;
			u64 dsw = sw - snap->small_bucket[i].sum_of_weights;
			u64 dsl = sl - snap->small_bucket[i].weighted_sum_latency;
			snap->small_bucket[i].sum_of_weights = sw;
			snap->small_bucket[i].weighted_sum_latency = sl;
			if (dsw) {
				asb = &aggr->small_bucket[i];
				asb->sum_of_weights += dsw;
				asb->weighted_sum_latency += dsl;
			}

			u64 lw  = pcpu_b->large_bucket[i].sum_of_weights;
			u64 ll  = pcpu_b->large_bucket[i].weighted_sum_latency;
			u64 lb  = pcpu_b->large_bucket[i].weighted_sum_block_size;
			u64 dlw = lw - snap->large_bucket[i].sum_of_weights;
			u64 dll = ll - snap->large_bucket[i].weighted_sum_latency;
			u64 dlb = lb - snap->large_bucket[i].weighted_sum_block_size;
			snap->large_bucket[i].sum_of_weights = lw;
			snap->large_bucket[i].weighted_sum_latency = ll;
			snap->large_bucket[i].weighted_sum_block_size = lb;
			if (dlw) {
				alb = &aggr->large_bucket[i];
				alb->sum_of_weights += dlw;
				alb->weighted_sum_latency += dll;
				alb->weighted_sum_block_size += dlb;
			}
		}
	}

	// Count the number of entries in aggregated buckets
	small_weight = lm_count_small_entries(aggr->small_bucket);
	large_weight = lm_count_large_entries(aggr->large_bucket);

	// Whether enough time has elapsed since the last update
	now = jiffies;
	time_elapsed = unlikely(!new_params->base) ||
		new_params->last_update_jiffies +
		msecs_to_jiffies(LM_INTERVAL_THRESHOLD) <= now;

	// Update small buckets
	if (small_weight && (time_elapsed ||
			LM_SAMPLES_THRESHOLD <= small_weight || !new_params->base)) {
		small_processed = lm_update_small_buckets(model, new_params,
			aggr->small_bucket, small_weight, !new_params->base);
		memset(&aggr->small_bucket[0], 0, sizeof(aggr->small_bucket));
	}
	// Update large buckets
	if (large_weight && (time_elapsed ||
			LM_SAMPLES_THRESHOLD <= large_weight || !new_params->slope)) {
		large_processed = lm_update_large_buckets(model, new_params,
			aggr->large_bucket, large_weight, !new_params->slope);
		memset(&aggr->large_bucket[0], 0, sizeof(aggr->large_bucket));
	}

	// Update the base parameter if small bucket was processed
	if (small_processed && likely(new_params->small_count))
		new_params->base = div_u64(new_params->small_sum_delay,
			new_params->small_count);

	// Update the slope parameter if large bucket was processed
	if (large_processed && likely(new_params->large_sum_bsize))
		new_params->slope = div_u64(new_params->large_sum_delay,
			DIV_ROUND_UP_ULL(new_params->large_sum_bsize, 1024));

	// Update last updated jiffies if update happened or time has elapsed
	if (small_processed || large_processed || time_elapsed)
		new_params->last_update_jiffies = now;

	rcu_assign_pointer(model->params, new_params);
	spin_unlock_irqrestore(&model->update_lock, flags);

	kfree_rcu(old_params, rcu);
}

// Determine the bucket index for a given measured and predicted latency
static u8 lm_input_bucket_index(u64 measured, u64 predicted) {
	u32 bucket_index;

	if (measured < predicted * 2)
		bucket_index = div_u64((measured * 20), predicted);
	else if (measured < predicted * 5)
		bucket_index = div_u64((measured * 10), predicted) + 20;
	else
		bucket_index = div_u64((measured * 3), predicted) + 40;

	if (bucket_index >= LM_LAT_BUCKET_COUNT)
		bucket_index = LM_LAT_BUCKET_COUNT - 1;

	return (u8)bucket_index;
}

// Input latency data into the latency model
static void latency_model_input(struct adios_data *ad,
		struct latency_model *model,
		u32 block_size, u64 latency, u64 pred_lat, u32 weight) {
	unsigned long flags;
	u8 bucket_index;
	struct lm_buckets *buckets;
	u64 current_base;
	struct latency_model_params *params;

	local_irq_save(flags);
	buckets = per_cpu_ptr(model->pcpu_buckets, __smp_processor_id());

	rcu_read_lock();
	params = rcu_dereference(model->params);
	current_base = params->base;
	rcu_read_unlock();

	if (block_size <= LM_BLOCK_SIZE_THRESHOLD) {
		// Handle small requests
		bucket_index = lm_input_bucket_index(latency, current_base ?: 1);

		buckets->small_bucket[bucket_index].sum_of_weights += weight;
		buckets->small_bucket[bucket_index].weighted_sum_latency +=
			latency * weight;

		local_irq_restore(flags);

		if (unlikely(!current_base)) {
			latency_model_update(ad, model);
			return;
		}
	} else {
		// Handle large requests
		if (!current_base || !pred_lat) {
			local_irq_restore(flags);
			return;
		}

		bucket_index = lm_input_bucket_index(latency, pred_lat);

		buckets->large_bucket[bucket_index].sum_of_weights += weight;
		buckets->large_bucket[bucket_index].weighted_sum_latency +=
			latency * weight;
		buckets->large_bucket[bucket_index].weighted_sum_block_size +=
			block_size * weight;

		local_irq_restore(flags);
	}
}

// Predict the latency for a given block size using the latency model
static u64 latency_model_predict(struct latency_model *model, u32 block_size) {
	u64 result;
	struct latency_model_params *params;

	rcu_read_lock();
	params = rcu_dereference(model->params);

	result = params->base;
	if (block_size > LM_BLOCK_SIZE_THRESHOLD)
		result += params->slope *
			DIV_ROUND_UP_ULL(block_size - LM_BLOCK_SIZE_THRESHOLD, 1024);

	rcu_read_unlock();

	return result;
}

// Determine the type of operation based on request flags
static u8 adios_optype(struct request *rq) {
	switch (rq->cmd_flags & REQ_OP_MASK) {
	case REQ_OP_READ:
		return ADIOS_READ;
	case REQ_OP_WRITE:
		return ADIOS_WRITE;
	case REQ_OP_DISCARD:
		return ADIOS_DISCARD;
	default:
		return ADIOS_OTHER;
	}
}

static inline u8 adios_optype_not_read(struct request *rq) {
	return (rq->cmd_flags & REQ_OP_MASK) != REQ_OP_READ;
}

// Helper function to retrieve adios_rq_data from a request
static inline struct adios_rq_data *get_rq_data(struct request *rq) {
	return rq->elv.priv[0];
}

static inline
void set_adios_state(struct adios_data *ad, u32 shift, u32 idx, bool flag) {
	if (flag)
		atomic_or(1U << (idx + shift), &ad->state);
	else
		atomic_andnot(1U << (idx + shift), &ad->state);
}

static inline u32 get_adios_state(struct adios_data *ad)
{ return atomic_read(&ad->state); }

static inline u32 eval_this_adios_state(u32 state, u32 shift)
{ return (state >> shift) & 0x3; }

static inline u32 eval_adios_state(struct adios_data *ad, u32 shift)
{ return eval_this_adios_state(get_adios_state(ad), shift); }

// Add a request to the deadline-sorted red-black tree
static void add_to_dl_tree(
		struct adios_data *ad, bool dl_idx, struct request *rq) {
	struct rb_root_cached *root = &ad->dl_tree[dl_idx];
	struct rb_node **link = &(root->rb_root.rb_node), *parent = NULL;
	bool leftmost = true;
	struct adios_rq_data *rd = get_rq_data(rq);
	struct dl_group *dlg;
	u64 deadline;
	bool was_empty = RB_EMPTY_ROOT(&root->rb_root);

	/* Tier-2: Synchronous Requests
	 * - Needs to be FIFO within a same optype
	 * - Relaxed order between different optypes
	 * - basically needs to be processed in early time */
	rd->deadline = rq->start_time_ns;

	/* Tier-3: Aynchronous Requests
	 * - Can be reordered and delayed freely */
	if (!(rq->cmd_flags & REQ_SYNC)) {
		rd->deadline += ad->latency_target[adios_optype(rq)];
		if (!compliant(ad, ADIOS_CF_FIXORDER))
			rd->deadline += rd->pred_lat;
	}

	// Now quantize the deadline (-> dlg->deadline == RB-Tree key)
	deadline = rd->deadline & ~((1ULL << ADIOS_QUANTUM_SHIFT) - 1);

	while (*link) {
		dlg = rb_entry(*link, struct dl_group, node);
		s64 diff = deadline - dlg->deadline;

		parent = *link;
		if (diff < 0) {
			link = &((*link)->rb_left);
		} else if (diff > 0) {
			link = &((*link)->rb_right);
			leftmost = false;
		} else { // diff == 0
			goto found;
		}
	}

	dlg = rb_entry_safe(parent, struct dl_group, node);
	if (!dlg || dlg->deadline != deadline) {
		dlg = kmem_cache_zalloc(ad->dl_group_pool, GFP_ATOMIC);
		if (!dlg)
			return;
		dlg->deadline = deadline;
		INIT_LIST_HEAD(&dlg->rqs);
		rb_link_node(&dlg->node, parent, link);
		rb_insert_color_cached(&dlg->node, root, leftmost);
	}
found:
	list_add_tail(&rd->dl_node, &dlg->rqs);
	rd->dl_group = &dlg->rqs;

	if (was_empty)
		set_adios_state(ad, ADIOS_STATE_DL, dl_idx, true);
}

// Remove a request from the deadline-sorted red-black tree
static void del_from_dl_tree(
		struct adios_data *ad, bool dl_idx, struct request *rq) {
	struct rb_root_cached *root = &ad->dl_tree[dl_idx];
	struct adios_rq_data *rd = get_rq_data(rq);
	struct dl_group *dlg = container_of(rd->dl_group, struct dl_group, rqs);

	list_del_init(&rd->dl_node);
	if (list_empty(&dlg->rqs)) {
		rb_erase_cached(&dlg->node, root);
		kmem_cache_free(ad->dl_group_pool, dlg);
	}
	rd->dl_group = NULL;

	if (RB_EMPTY_ROOT(&ad->dl_tree[dl_idx].rb_root))
		set_adios_state(ad, ADIOS_STATE_DL, dl_idx, false);
}

// Remove a request from the scheduler
static void remove_request(struct adios_data *ad, struct request *rq) {
	bool dl_idx = adios_optype_not_read(rq);
	struct request_queue *q = rq->q;
	struct adios_rq_data *rd = get_rq_data(rq);

	list_del_init(&rq->queuelist);

	// We might not be on the rbtree, if we are doing an insert merge
	if (rd->dl_group)
		del_from_dl_tree(ad, dl_idx, rq);

	elv_rqhash_del(q, rq);
	if (q->last_merge == rq)
		q->last_merge = NULL;
}

// Convert a queue depth to the corresponding word depth for shallow allocation
static int to_word_depth(struct blk_mq_hw_ctx *hctx, unsigned int qdepth) {
	struct sbitmap_queue *bt = hctx->sched_tags->bitmap_tags;
	const unsigned int nrr = hctx->queue->nr_requests;

	return ((qdepth << bt->sb.shift) + nrr - 1) / nrr;
}

// We limit the depth of request allocation for asynchronous and write requests
static void adios_limit_depth(unsigned int opf, struct blk_mq_alloc_data *data) {
	struct adios_data *ad = data->q->elevator->elevator_data;

	// Do not throttle synchronous reads
	if (op_is_sync(opf) && !op_is_write(opf))
		return;

	data->shallow_depth = to_word_depth(data->hctx, ad->async_depth);
}

// The number of requests in the queue was notified from the block layer
static void adios_depth_updated(struct blk_mq_hw_ctx *hctx) {
	struct request_queue *q = hctx->queue;
	struct adios_data *ad = q->elevator->elevator_data;
	struct blk_mq_tags *tags = hctx->sched_tags;

	ad->async_depth = q->nr_requests;

	sbitmap_queue_min_shallow_depth(tags->bitmap_tags, 1);
}

// Handle request merging after a merge operation
static void adios_request_merged(struct request_queue *q, struct request *req,
				  enum elv_merge type) {
	bool dl_idx = adios_optype_not_read(req);
	struct adios_data *ad = q->elevator->elevator_data;

	// Reposition request in the deadline-sorted tree
	del_from_dl_tree(ad, dl_idx, req);
	add_to_dl_tree(ad, dl_idx, req);
}

// Handle merging of requests after one has been merged into another
static void adios_merged_requests(struct request_queue *q, struct request *req,
				   struct request *next) {
	struct adios_data *ad = q->elevator->elevator_data;

	lockdep_assert_held(&ad->lock);

	// kill knowledge of next, this one is a goner
	remove_request(ad, next);
}

// Try to merge a bio into an existing rq before associating it with an rq
static bool adios_bio_merge(struct request_queue *q, struct bio *bio,
		unsigned int nr_segs) {
	unsigned long flags;
	struct adios_data *ad = q->elevator->elevator_data;
	struct request *free = NULL;
	bool ret;

	if (!spin_trylock_irqsave(&ad->lock, flags))
		return false;

	ret = blk_mq_sched_try_merge(q, bio, nr_segs, &free);
	spin_unlock_irqrestore(&ad->lock, flags);

	if (free)
		blk_mq_free_request(free);

	return ret;
}

static bool merge_or_insert_to_dl_tree(struct adios_data *ad,
		struct request *rq, struct request_queue *q) {
	if (blk_mq_sched_try_insert_merge(q, rq))
		return true;

	bool dl_idx = adios_optype_not_read(rq);
	add_to_dl_tree(ad, dl_idx, rq);

	if (rq_mergeable(rq)) {
		elv_rqhash_add(q, rq);
		if (!q->last_merge)
			q->last_merge = rq;
	}

	return false;
}

static void insert_to_prio_queue(struct adios_data *ad,
		struct request *rq, bool pq_idx) {
	struct adios_rq_data *rd = get_rq_data(rq);

	/* We're sure that rd->managed == true */
	union adios_in_flight_rqs ifr = {
		.count          = 1,
		.total_pred_lat = rd->pred_lat,
	};
	atomic64_add(ifr.scalar, &ad->in_flight_rqs.atomic);

	scoped_guard(spinlock_irqsave, &ad->pq_lock) {
		bool was_empty = list_empty(&ad->prio_queue[pq_idx]);
		list_add_tail(&rq->queuelist, &ad->prio_queue[pq_idx]);
		if (was_empty)
			set_adios_state(ad, ADIOS_STATE_PQ, pq_idx, true);
	}
}

// Insert a request into the scheduler (after Read & Write models stabilized)
static void insert_request_post_stability(struct blk_mq_hw_ctx *hctx,
		struct request *rq, bool at_head) {
	struct request_queue *q = hctx->queue;
	struct adios_data *ad = q->elevator->elevator_data;
	struct adios_rq_data *rd = get_rq_data(rq);
	u8 optype = adios_optype(rq);

	rd->managed = true;
	rd->block_size = blk_rq_bytes(rq);
	rd->pred_lat =
		latency_model_predict(&ad->latency_model[optype], rd->block_size);
	if (unlikely(rd->pred_lat > ad->lat_model_latency_limit))
		rd->pred_lat = ad->lat_model_latency_limit;

	/* Tier-0: at_head Requests */
	if (at_head) {
		insert_to_prio_queue(ad, rq, 0);
		return;
	}

	if (merge_or_insert_to_dl_tree(ad, rq, q))
		return;
}

// Insert a request into the scheduler (before Read & Write models stabilizes)
static void insert_request_pre_stability(struct blk_mq_hw_ctx *hctx,
		struct request *rq, bool at_head) {
	struct adios_data *ad = hctx->queue->elevator->elevator_data;
	struct adios_rq_data *rd = get_rq_data(rq);
	u8 optype = adios_optype(rq);
	u8 pq_idx = at_head ? 0 : 1;
	bool stable = false;

	rd->managed = true;
	rd->block_size = blk_rq_bytes(rq);
	rd->pred_lat =
		latency_model_predict(&ad->latency_model[optype], rd->block_size);
	if (unlikely(rd->pred_lat > ad->lat_model_latency_limit))
		rd->pred_lat = ad->lat_model_latency_limit;

	insert_to_prio_queue(ad, rq, pq_idx);

	rcu_read_lock();
	if (rcu_dereference(ad->latency_model[ADIOS_READ].params)->base > 0 &&
		rcu_dereference(ad->latency_model[ADIOS_WRITE].params)->base > 0)
			stable = true;
	rcu_read_unlock();

	if (stable)
		ad->models_stable = true;
}

// Insert multiple requests into the scheduler
static void adios_insert_requests(struct blk_mq_hw_ctx *hctx,
				   struct list_head *list,
				   bool at_head) {
	struct request_queue *q = hctx->queue;
	struct adios_data *ad = q->elevator->elevator_data;
	struct request *rq;
	bool stop = false;

	do {
	scoped_guard(spinlock_irqsave, &ad->lock)
	for (int i = 0; i < ADIOS_MAX_INSERTS_PER_LOCK; i++) {
		if (list_empty(list)) {
			stop = true;
			break;
		}
		rq = list_first_entry(list, struct request, queuelist);
		list_del_init(&rq->queuelist);
		if (likely(ad->models_stable))
			insert_request_post_stability(hctx, rq, at_head);
		else
			insert_request_pre_stability(hctx, rq, at_head);
	}} while (!stop);
}

// Prepare a request before it is inserted into the scheduler
static void adios_prepare_request(struct request *rq) {
	struct adios_data *ad = rq->q->elevator->elevator_data;
	struct adios_rq_data *rd;

	rd = mempool_alloc(ad->rq_data_pool, GFP_ATOMIC);
	memset(rd, 0, sizeof(*rd));
	rd->rq = rq;
	rq->elv.priv[0] = rd;
}

static struct adios_rq_data *get_dl_first_rd(struct adios_data *ad, bool idx) {
	struct rb_root_cached *root = &ad->dl_tree[idx];
	struct rb_node *first = rb_first_cached(root);
	struct dl_group *dl_group = rb_entry(first, struct dl_group, node);

	return list_first_entry(&dl_group->rqs, struct adios_rq_data, dl_node);
}

// Comparison function for sorting requests by block address
static int cmp_rq_pos(void *priv, struct list_head *a, struct list_head *b) {
	struct request *rq_a = list_entry(a, struct request, queuelist);
	struct request *rq_b = list_entry(b, struct request, queuelist);
	u64 pos_a = blk_rq_pos(rq_a);
	u64 pos_b = blk_rq_pos(rq_b);

	return (int)(pos_a > pos_b) - (int)(pos_a < pos_b);
}

#ifndef list_last_entry_or_null
#define list_last_entry_or_null(ptr, type, member) \
	(!list_empty(ptr) ? list_last_entry(ptr, type, member) : NULL)
#endif

// Update the elevator direction
static void update_elv_direction(struct adios_data *ad) {
	if (!ad->is_rotational)
		return;

	bool page = ad->bq_page;
	struct list_head *q = &ad->batch_queue[page][1];
	if (ad->bq_batch_order[page] < ADIOS_BO_ELEVATOR || list_empty(q)) {
		ad->elv_direction = 0;
		return;
	}

	// Get first and last request positions in the queue
	struct request *rq_a = list_first_entry(q, struct request, queuelist);
	struct request *rq_b = list_last_entry (q, struct request, queuelist);
	u64 pos_a = blk_rq_pos(rq_a);
	u64 pos_b = blk_rq_pos(rq_b);
	u64 avg_rq_pos = (pos_a + pos_b) >> 1;

	ad->elv_direction = !!(ad->head_pos > avg_rq_pos);
}

// Fill the batch queues with requests from the deadline-sorted red-black tree
static bool fill_batch_queues(struct adios_data *ad, u64 tpl) {
	struct adios_rq_data *rd;
	struct request *rq;
	struct list_head *dest_q;
	u8  dest_idx;
	u64 added_lat = 0;
	u32 optype_count[ADIOS_OPTYPES] = {0};
	u32 count = 0;
	u8 optype;
	bool page = !ad->bq_page, dl_idx, bias_idx, update_bias;
	u32 dl_queued;
	u8 bq_batch_order;
	bool stop = false;

	// Reset batch queue counts for the back page
	memset(&ad->batch_count[page], 0, sizeof(ad->batch_count[page]));

	ad->bq_batch_order[page] =
		bq_batch_order = ad->batch_order;

	do {
	scoped_guard(spinlock_irqsave, &ad->lock)
	for (int i = 0; i < ADIOS_MAX_DELETES_PER_LOCK; i++) {
		bool has_base = false;

		dl_queued = eval_adios_state(ad, ADIOS_STATE_DL);
		// Check if there are any requests queued in the deadline tree
		if (!dl_queued) {
			stop = true;
			break;
		}

		// Reads if both queues have requests, otherwise pick the non-empty.
		dl_idx = dl_queued >> 1;

		// Get the first request from the deadline-sorted tree
		rd = get_dl_first_rd(ad, dl_idx);

		bias_idx = ad->dl_bias < 0;
		// If read and write requests are queued, choose one based on bias
		if (dl_queued == 0x3) {
			struct adios_rq_data *trd[2] = {get_dl_first_rd(ad, 0), rd};
			rd = trd[bias_idx];

			update_bias = (trd[bias_idx]->deadline > trd[!bias_idx]->deadline);
		} else
			update_bias = (bias_idx == dl_idx);

		rq = rd->rq;
		optype = adios_optype(rq);

		rcu_read_lock();
		has_base =
			!!rcu_dereference(ad->latency_model[optype].params)->base;
		rcu_read_unlock();

		// Check batch size and total predicted latency
		if (count && (!has_base ||
				ad->batch_count[page][optype] >= ad->batch_limit[optype] ||
				(tpl + added_lat + rd->pred_lat) > ad->global_latency_window)) {
			stop = true;
			break;
		}

		if (update_bias) {
			s64 sign = ((s64)bias_idx << 1) - 1;
			if (unlikely(!rd->pred_lat))
				ad->dl_bias = sign;
			else
				// Adjust the bias based on the predicted latency
				ad->dl_bias += sign * (s64)((rd->pred_lat *
					adios_prio_to_wmult[ad->dl_prio[bias_idx] + 20]) >> 10);
		}

		remove_request(ad, rq);

		// Add request to the corresponding batch queue
		dest_idx = (bq_batch_order == ADIOS_BO_OPTYPE || optype == ADIOS_OTHER)?
			optype : !!(rd->deadline != rq->start_time_ns);
		dest_q = &ad->batch_queue[page][dest_idx];
		list_add_tail(&rq->queuelist, dest_q);
		ad->bq_state[page] |= 1U << dest_idx;
		ad->batch_count[page][optype]++;
		optype_count[optype]++;
		added_lat += rd->pred_lat;
		count++;
	}} while (!stop);

	if (bq_batch_order == ADIOS_BO_ELEVATOR && ad->batch_count[page][1] > 1)
			list_sort(NULL, &ad->batch_queue[page][1], cmp_rq_pos);

	if (count) {
		/* We're sure that every request's rd->managed == true */
		union adios_in_flight_rqs ifr = {
			.count          = count,
			.total_pred_lat = added_lat,
		};
		atomic64_add(ifr.scalar, &ad->in_flight_rqs.atomic);

		set_adios_state(ad, ADIOS_STATE_BQ, page, true);

		for (optype = 0; optype < ADIOS_OPTYPES; optype++)
			if (ad->batch_actual_max_size[optype] < optype_count[optype])
				ad->batch_actual_max_size[optype] = optype_count[optype];
		if (ad->batch_actual_max_total < count)
			ad->batch_actual_max_total = count;
	}
	return count;
}

// Flip to the next batch queue page
static void flip_bq_page(struct adios_data *ad) {
	ad->bq_page = !ad->bq_page;
	update_elv_direction(ad);
}

// Pop a request from the specified index (optype or elevator tier)
static inline struct request *pop_bq_request(
		struct adios_data *ad, u8 idx, bool direction) {
	bool page = ad->bq_page;
	struct list_head *q = &ad->batch_queue[page][idx];
	struct request *rq = direction ?
		list_last_entry_or_null (q, struct request, queuelist):
		list_first_entry_or_null(q, struct request, queuelist);
	if (rq) {
		list_del_init(&rq->queuelist);
		if (list_empty(q))
			ad->bq_state[page] &= ~(1U << idx);
	}
	return rq;
}

static struct request *pop_next_bq_request_optype(struct adios_data *ad) {
	u32 bq_state = ad->bq_state[ad->bq_page];
	if (!bq_state) return NULL;

	struct request *rq;
	u32 bq_idx = __builtin_ctz(bq_state);

	// Dispatch based on optype (FIFO within each) or single-queue elevator
	rq = pop_bq_request(ad, bq_idx, false);
	return rq;
}

static struct request *pop_next_bq_request_elevator(struct adios_data *ad) {
	u32 bq_state = ad->bq_state[ad->bq_page];
	if (!bq_state) return NULL;

	struct request *rq;
	u32 bq_idx = __builtin_ctz(bq_state);
	bool direction = (bq_idx == 1) & ad->elv_direction;

	// Tier-2 (sync) is always high priority
	// Tier-3 (async) uses the pre-calculated elevator direction
	rq = pop_bq_request(ad, bq_idx, direction);

	/* If batch queue for the sync requests just became empty */
	if (bq_idx == 0 && rq && !(bq_state & 0x1))
		update_elv_direction(ad);

	return rq;
}

// Returns the state of the batch queue page
static inline bool bq_page_has_rq(u32 bq_state, bool page) {
	return bq_state & (1U << page);
}

// Dispatch a request from the batch queues
static struct request *dispatch_from_bq(struct adios_data *ad) {
	struct request *rq;

	guard(spinlock_irqsave)(&ad->bq_lock);

	u32 state = get_adios_state(ad);
	u32 bq_state = eval_this_adios_state(state, ADIOS_STATE_BQ);
	u32 bq_curr_page_has_rq = bq_page_has_rq(bq_state, ad->bq_page);
	union adios_in_flight_rqs ifr;
	ifr.scalar = atomic64_read(&ad->in_flight_rqs.atomic);
	u64 tpl = ifr.total_pred_lat;

	// Refill the batch queues if the back page is empty, dl_tree has work, and
	// current page is empty or the total ongoing latency is below the threshold
	if (!bq_page_has_rq(bq_state, !ad->bq_page) &&
			(!bq_curr_page_has_rq || (!tpl || tpl < div_u64(
			ad->global_latency_window * ad->bq_refill_below_ratio, 100))) &&
			eval_this_adios_state(state, ADIOS_STATE_DL))
		fill_batch_queues(ad, tpl);

	// If current batch queue page is empty, and the other page has work, flip
	if (!bq_curr_page_has_rq &&
			bq_page_has_rq(eval_adios_state(ad, ADIOS_STATE_BQ), !ad->bq_page))
		flip_bq_page(ad);

	// Use the per-page state to decide the dispatch logic, ensuring correctness
	rq = (ad->bq_batch_order[ad->bq_page] == ADIOS_BO_ELEVATOR) ?
		pop_next_bq_request_elevator(ad):
		pop_next_bq_request_optype(ad);

	if (rq) {
		bool page = ad->bq_page;
		bool is_empty = !ad->bq_state[page];
		if (is_empty)
			set_adios_state(ad, ADIOS_STATE_BQ, page, false);
		return rq;
	}

	return NULL;
}

// Dispatch a request from the priority queue
static struct request *dispatch_from_pq(struct adios_data *ad) {
	struct request *rq = NULL;

	guard(spinlock_irqsave)(&ad->pq_lock);
	u32 pq_state = eval_adios_state(ad, ADIOS_STATE_PQ);
	u8  pq_idx = pq_state >> 1;
	struct list_head *q = &ad->prio_queue[pq_idx];

	if (unlikely(list_empty(q))) return NULL;

	rq = list_first_entry(q, struct request, queuelist);
	list_del_init(&rq->queuelist);
	if (list_empty(q)) {
		set_adios_state(ad, ADIOS_STATE_PQ, pq_idx, false);
		update_elv_direction(ad);
	}
	return rq;
}


// Dispatch a request to the hardware queue
static struct request *adios_dispatch_request(struct blk_mq_hw_ctx *hctx) {
	struct adios_data *ad = hctx->queue->elevator->elevator_data;
	struct request *rq;

	rq = dispatch_from_pq(ad);
	if (rq)
		goto found;

	rq = dispatch_from_bq(ad);
	if (rq)
		goto found;

	return NULL;
found:
	if (ad->is_rotational)
		ad->head_pos = blk_rq_pos(rq) + blk_rq_sectors(rq);

	rq->rq_flags |= RQF_STARTED;
	return rq;
}

// Timer callback function to periodically update latency models
static void update_timer_callback(struct timer_list *t) {
	struct adios_data *ad = from_timer(ad, t, update_timer);

	for (u8 optype = 0; optype < ADIOS_OPTYPES; optype++)
		latency_model_update(ad, &ad->latency_model[optype]);
}

// Handle the completion of a request
static void adios_completed_request(struct request *rq, u64 now) {
	struct adios_data *ad = rq->q->elevator->elevator_data;
	struct adios_rq_data *rd = get_rq_data(rq);
	union adios_in_flight_rqs ifr = { .scalar = 0 };

	if (op_is_flush(rq->cmd_flags) || !rd)
		return;

	if (rd->managed) {
		union adios_in_flight_rqs ifr_to_sub = {
			.count          = 1,
			.total_pred_lat = rd->pred_lat,
		};
		ifr.scalar = atomic64_sub_return(
			ifr_to_sub.scalar, &ad->in_flight_rqs.atomic);
	}
	u8 optype = adios_optype(rq);

	unsigned long flags;
	struct adios_pcpu_completion *pc;

	local_irq_save(flags);
	pc = this_cpu_ptr(ad->pcpu_completion);

	if (optype == ADIOS_OTHER) {
		// Non-positional commands make the head position unpredictable.
		// Invalidate our knowledge of the last completed position.
		if (ad->is_rotational)
			pc->last_completed_pos = 0;
		local_irq_restore(flags);
		return;
	}

	u64 lct = pc->last_completed_time ?: rq->io_start_time_ns;
	pc->last_completed_time = (ifr.count) ? now : 0;

	if (!rq->io_start_time_ns || !rd->block_size || unlikely(now < lct)) {
		local_irq_restore(flags);
		return;
	}

	u64 latency = now - lct;

	u32 weight = 1;
	if (ad->is_rotational) {
		sector_t current_pos = blk_rq_pos(rq);
		// Only calculate seek distance if we have a valid last position.
		if (pc->last_completed_pos > 0) {
			u64 seek_distance = abs(
				(s64)current_pos - (s64)pc->last_completed_pos);
			if (seek_distance)
				weight = 65 - __builtin_clzll(seek_distance);
		}
		// Update (or re-synchronize) our knowledge of the head position.
		pc->last_completed_pos = current_pos + blk_rq_sectors(rq);
	}

	local_irq_restore(flags);

	if (latency > ad->lat_model_latency_limit)
		return;

	latency_model_input(ad, &ad->latency_model[optype],
		rd->block_size, latency, rd->pred_lat, weight);
	timer_reduce(&ad->update_timer, jiffies + msecs_to_jiffies(100));
}

// Clean up after a request is finished
static void adios_finish_request(struct request *rq) {
	struct adios_data *ad = rq->q->elevator->elevator_data;

	if (rq->elv.priv[0]) {
		// Free adios_rq_data back to the memory pool
		mempool_free(get_rq_data(rq), ad->rq_data_pool);
		rq->elv.priv[0] = NULL;
	}
}

// Check if there are any requests available for dispatch
static bool adios_has_work(struct blk_mq_hw_ctx *hctx) {
	struct adios_data *ad = hctx->queue->elevator->elevator_data;

	return atomic_read(&ad->state) != 0;
}

// Initialize the scheduler-specific data for a hardware queue
static int adios_init_hctx(struct blk_mq_hw_ctx *hctx, unsigned int hctx_idx) {
	adios_depth_updated(hctx);
	return 0;
}

// Initialize the scheduler-specific data when initializing the request queue
static int adios_init_sched(struct request_queue *q, struct elevator_type *e) {
	struct adios_data *ad;
	struct elevator_queue *eq;
	int ret = -ENOMEM;
	u8 optype = 0;
	u8 i;

	eq = elevator_alloc(q, e);
	if (!eq) {
		pr_err("adios: Failed to allocate the elevator\n");
		return ret;
	}

	ad = kzalloc_node(sizeof(*ad), GFP_KERNEL, q->node);
	if (!ad) {
		pr_err("adios: Failed to create adios_data\n");
		goto put_eq;
	}

	// Create a memory pool for adios_rq_data
	ad->rq_data_cache = kmem_cache_create("adios_rq_data",
						sizeof(struct adios_rq_data),
						0, SLAB_HWCACHE_ALIGN, NULL);
	if (!ad->rq_data_cache) {
		pr_err("adios: Failed to create rq_data_cache\n");
		goto free_ad;
	}

	ad->rq_data_pool = mempool_create_slab_pool(
						q->nr_requests, ad->rq_data_cache);
	if (!ad->rq_data_pool) {
		pr_err("adios: Failed to create rq_data_pool\n");
		goto destroy_rq_data_cache;
	}

	/* Create a memory pool for dl_group */
	ad->dl_group_pool = kmem_cache_create("dl_group_pool",
						sizeof(struct dl_group),
						0, SLAB_HWCACHE_ALIGN, NULL);
	if (!ad->dl_group_pool) {
		pr_err("adios: Failed to create dl_group_pool\n");
		goto destroy_rq_data_pool;
	}

	for (i = 0; i < ADIOS_PQ_LEVELS; i++)
		INIT_LIST_HEAD(&ad->prio_queue[i]);

	for (i = 0; i < ADIOS_DL_TYPES; i++) {
		ad->dl_tree[i] = RB_ROOT_CACHED;
		ad->dl_prio[i] = default_dl_prio[i];
	}
	ad->dl_bias = 0;

	for (u8 page = 0; page < ADIOS_BQ_PAGES; page++)
		for (optype = 0; optype < ADIOS_OPTYPES; optype++)
			INIT_LIST_HEAD(&ad->batch_queue[page][optype]);

	ad->aggr_buckets = kzalloc(sizeof(*ad->aggr_buckets), GFP_KERNEL);
	if (!ad->aggr_buckets) {
		pr_err("adios: Failed to allocate aggregation buckets\n");
		goto destroy_dl_group_pool;
	}

	ad->pcpu_completion = alloc_percpu(struct adios_pcpu_completion);
	if (!ad->pcpu_completion) {
		pr_err("adios: Failed to allocate per-CPU completion data\n");
		goto free_aggr_buckets;
	}

	for (optype = 0; optype < ADIOS_OPTYPES; optype++) {
		struct latency_model *model = &ad->latency_model[optype];
		struct latency_model_params *params;

		spin_lock_init(&model->update_lock);
		params = kzalloc(sizeof(*params), GFP_KERNEL);
		if (!params) {
			pr_err("adios: Failed to allocate latency_model_params\n");
			goto free_buckets;
		}
		params->last_update_jiffies = jiffies;
		RCU_INIT_POINTER(model->params, params);

		model->pcpu_buckets = alloc_percpu(struct lm_buckets);
		if (!model->pcpu_buckets) {
			pr_err("adios: Failed to allocate per-CPU buckets\n");
			kfree(params);
			goto free_buckets;
		}

		model->pcpu_snapshot = alloc_percpu(struct lm_buckets);
		if (!model->pcpu_snapshot) {
			pr_err("adios: Failed to allocate per-CPU snapshot\n");
			free_percpu(model->pcpu_buckets);
			kfree(params);
			goto free_buckets;
		}

		model->lm_shrink_at_kreqs  = default_lm_shrink_at_kreqs;
		model->lm_shrink_at_gbytes = default_lm_shrink_at_gbytes;
		model->lm_shrink_resist    = default_lm_shrink_resist;
	}

	for (optype = 0; optype < ADIOS_OPTYPES; optype++) {
		ad->latency_target[optype] = default_latency_target[optype];
		ad->batch_limit[optype] = default_batch_limit[optype];
	}

	eq->elevator_data = ad;

	ad->is_rotational = !blk_queue_nonrot(q);
	ad->global_latency_window = (ad->is_rotational)?
		default_global_latency_window_rotational:
		default_global_latency_window;
	ad->bq_refill_below_ratio = default_bq_refill_below_ratio;
	ad->lat_model_latency_limit = default_lat_model_latency_limit;
	ad->batch_order = default_batch_order;
	ad->compliance_flags = default_compliance_flags;

	ad->models_stable = false;

	atomic_set(&ad->state, 0);

	spin_lock_init(&ad->lock);
	spin_lock_init(&ad->pq_lock);
	spin_lock_init(&ad->bq_lock);

	timer_setup(&ad->update_timer, update_timer_callback, 0);

	ad->queue = q;
	blk_stat_enable_accounting(q);

	q->elevator = eq;
	return 0;

free_buckets:
	pr_err("adios: Failed to allocate per-cpu buckets\n");
	while (optype-- > 0) {
		struct latency_model *prev_model = &ad->latency_model[optype];
		kfree(rcu_access_pointer(prev_model->params));
		free_percpu(prev_model->pcpu_snapshot);
		free_percpu(prev_model->pcpu_buckets);
	}
	free_percpu(ad->pcpu_completion);
free_aggr_buckets:
	kfree(ad->aggr_buckets);
destroy_dl_group_pool:
	kmem_cache_destroy(ad->dl_group_pool);
destroy_rq_data_pool:
	mempool_destroy(ad->rq_data_pool);
destroy_rq_data_cache:
	kmem_cache_destroy(ad->rq_data_cache);
free_ad:
	kfree(ad);
put_eq:
	kobject_put(&eq->kobj);
	return ret;
}

// Clean up and free resources when exiting the scheduler
static void adios_exit_sched(struct elevator_queue *e) {
	struct adios_data *ad = e->elevator_data;
	u8 i;

	del_timer_sync(&ad->update_timer);

	for (i = 0; i < 2; i++)
		WARN_ON_ONCE(!list_empty(&ad->prio_queue[i]));

	for (i = 0; i < ADIOS_OPTYPES; i++) {
		struct latency_model *model = &ad->latency_model[i];
		struct latency_model_params *params = rcu_access_pointer(model->params);

		RCU_INIT_POINTER(model->params, NULL);
		kfree_rcu(params, rcu);

		free_percpu(model->pcpu_snapshot);
		free_percpu(model->pcpu_buckets);
	}

	synchronize_rcu();

	free_percpu(ad->pcpu_completion);
	kfree(ad->aggr_buckets);

	mempool_destroy(ad->rq_data_pool);
	kmem_cache_destroy(ad->rq_data_cache);

	if (ad->dl_group_pool)
		kmem_cache_destroy(ad->dl_group_pool);

	kfree(ad);
}

static void sideload_latency_model(
		struct latency_model *model, u64 base, u64 slope) {
	struct latency_model_params *old_params, *new_params;
	unsigned long flags;

	new_params = kzalloc(sizeof(*new_params), GFP_KERNEL);
	if (!new_params)
		return;

	spin_lock_irqsave(&model->update_lock, flags);

	old_params = rcu_dereference_protected(model->params,
			lockdep_is_held(&model->update_lock));

	new_params->last_update_jiffies = jiffies;

	// Initialize base and its statistics as a single sample.
	new_params->base = base;
	new_params->small_sum_delay = base;
	new_params->small_count = 1;

	// Initialize slope and its statistics as a single sample.
	new_params->slope = slope;
	new_params->large_sum_delay = slope;
	new_params->large_sum_bsize = 1024; /* Corresponds to 1 KiB */

	lm_reset_pcpu_buckets(model);

	rcu_assign_pointer(model->params, new_params);
	spin_unlock_irqrestore(&model->update_lock, flags);

	kfree_rcu(old_params, rcu);
}

// Define sysfs attributes for operation types
#define SYSFS_OPTYPE_DECL(name, optype) \
static ssize_t adios_lat_model_##name##_show( \
		struct elevator_queue *e, char *page) { \
	struct adios_data *ad = e->elevator_data; \
	struct latency_model *model = &ad->latency_model[optype]; \
	struct latency_model_params *params; \
	ssize_t len = 0; \
	u64 base, slope; \
	rcu_read_lock(); \
	params = rcu_dereference(model->params); \
	base = params->base; \
	slope = params->slope; \
	rcu_read_unlock(); \
	len += sprintf(page,       "base : %llu ns\n", base); \
	len += sprintf(page + len, "slope: %llu ns/KiB\n", slope); \
	return len; \
} \
static ssize_t adios_lat_model_##name##_store( \
		struct elevator_queue *e, const char *page, size_t count) { \
	struct adios_data *ad = e->elevator_data; \
	struct latency_model *model = &ad->latency_model[optype]; \
	u64 base, slope; \
	int ret; \
	ret = sscanf(page, "%llu %llu", &base, &slope); \
	if (ret != 2) \
		return -EINVAL; \
	sideload_latency_model(model, base, slope); \
	reset_buckets(ad->aggr_buckets); \
	return count; \
} \
static ssize_t adios_lat_target_##name##_show( \
		struct elevator_queue *e, char *page) { \
	struct adios_data *ad = e->elevator_data; \
	return sprintf(page, "%llu\n", ad->latency_target[optype]); \
} \
static ssize_t adios_lat_target_##name##_store( \
		struct elevator_queue *e, const char *page, size_t count) { \
	struct adios_data *ad = e->elevator_data; \
	unsigned long nsec; \
	int ret; \
	ret = kstrtoul(page, 10, &nsec); \
	if (ret) \
		return ret; \
	sideload_latency_model(&ad->latency_model[optype], 0, 0); \
	ad->latency_target[optype] = nsec; \
	return count; \
} \
static ssize_t adios_batch_limit_##name##_show( \
		struct elevator_queue *e, char *page) { \
	struct adios_data *ad = e->elevator_data; \
	return sprintf(page, "%u\n", ad->batch_limit[optype]); \
} \
static ssize_t adios_batch_limit_##name##_store( \
		struct elevator_queue *e, const char *page, size_t count) { \
	unsigned long max_batch; \
	int ret; \
	ret = kstrtoul(page, 10, &max_batch); \
	if (ret || max_batch == 0) \
		return -EINVAL; \
	struct adios_data *ad = e->elevator_data; \
	ad->batch_limit[optype] = max_batch; \
	return count; \
}

SYSFS_OPTYPE_DECL(read, ADIOS_READ);
SYSFS_OPTYPE_DECL(write, ADIOS_WRITE);
SYSFS_OPTYPE_DECL(discard, ADIOS_DISCARD);

// Show the maximum batch size actually achieved for each operation type
static ssize_t adios_batch_actual_max_show(
		struct elevator_queue *e, char *page) {
	struct adios_data *ad = e->elevator_data;
	u32 total_count, read_count, write_count, discard_count;

	total_count = ad->batch_actual_max_total;
	read_count = ad->batch_actual_max_size[ADIOS_READ];
	write_count = ad->batch_actual_max_size[ADIOS_WRITE];
	discard_count = ad->batch_actual_max_size[ADIOS_DISCARD];

	return sprintf(page,
		"Total  : %u\nDiscard: %u\nRead   : %u\nWrite  : %u\n",
		total_count, discard_count, read_count, write_count);
}

#define SYSFS_ULL_DECL(field, min_val, max_val) \
static ssize_t adios_##field##_show( \
		struct elevator_queue *e, char *page) { \
	struct adios_data *ad = e->elevator_data; \
	return sprintf(page, "%llu\n", ad->field); \
} \
static ssize_t adios_##field##_store( \
		struct elevator_queue *e, const char *page, size_t count) { \
	struct adios_data *ad = e->elevator_data; \
	unsigned long val; \
	int ret; \
	ret = kstrtoul(page, 10, &val); \
	if (ret || val < (min_val) || val > (max_val)) \
		return -EINVAL; \
	ad->field = val; \
	return count; \
}

SYSFS_ULL_DECL(global_latency_window, 0, ULLONG_MAX)
SYSFS_ULL_DECL(compliance_flags, 0, ULLONG_MAX)

#define SYSFS_INT_DECL(field, min_val, max_val) \
static ssize_t adios_##field##_show( \
		struct elevator_queue *e, char *page) { \
	struct adios_data *ad = e->elevator_data; \
	return sprintf(page, "%d\n", ad->field); \
} \
static ssize_t adios_##field##_store( \
		struct elevator_queue *e, const char *page, size_t count) { \
	struct adios_data *ad = e->elevator_data; \
	int val; \
	int ret; \
	ret = kstrtoint(page, 10, &val); \
	if (ret || val < (min_val) || val > (max_val)) \
		return -EINVAL; \
	ad->field = val; \
	return count; \
}

SYSFS_INT_DECL(bq_refill_below_ratio, 0, 100)
SYSFS_INT_DECL(lat_model_latency_limit, 0, 2*NSEC_PER_SEC)
SYSFS_INT_DECL(batch_order, ADIOS_BO_OPTYPE, !!ad->is_rotational)

// Show the read priority
static ssize_t adios_read_priority_show(
		struct elevator_queue *e, char *page) {
	struct adios_data *ad = e->elevator_data;
	return sprintf(page, "%d\n", ad->dl_prio[0]);
}

// Set the read priority
static ssize_t adios_read_priority_store(
		struct elevator_queue *e, const char *page, size_t count) {
	struct adios_data *ad = e->elevator_data;
	int prio;
	int ret;

	ret = kstrtoint(page, 10, &prio);
	if (ret || prio < -20 || prio > 19)
		return -EINVAL;

	guard(spinlock_irqsave)(&ad->lock);
	ad->dl_prio[0] = prio;
	ad->dl_bias = 0;

	return count;
}

// Reset batch queue statistics
static ssize_t adios_reset_bq_stats_store(
		struct elevator_queue *e, const char *page, size_t count) {
	struct adios_data *ad = e->elevator_data;
	unsigned long val;
	int ret;

	ret = kstrtoul(page, 10, &val);
	if (ret || val != 1)
		return -EINVAL;

	for (u8 i = 0; i < ADIOS_OPTYPES; i++)
		ad->batch_actual_max_size[i] = 0;

	ad->batch_actual_max_total = 0;

	return count;
}

// Reset the latency model parameters or load them from user input
static ssize_t adios_reset_lat_model_store(
		struct elevator_queue *e, const char *page, size_t count)
{
	struct adios_data *ad = e->elevator_data;
	struct latency_model *model;
	int ret;

	/*
	 * Differentiate between two modes based on input format:
	 * 1. "1": Fully reset the model (backward compatibility).
	 * 2. "R_base R_slope W_base W_slope D_base D_slope": Load values.
	 */
	if (!strchr(page, ' ')) {
		// Mode 1: Full reset.
		unsigned long val;

		ret = kstrtoul(page, 10, &val);
		if (ret || val != 1)
			return -EINVAL;

		for (u8 i = 0; i < ADIOS_OPTYPES; i++) {
			model = &ad->latency_model[i];
			sideload_latency_model(model, 0, 0);
		}
	} else {
		// Mode 2: Load initial values for all latency models.
		u64 params[3][2]; /* 0:base, 1:slope for R, W, D */

		ret = sscanf(page, "%llu %llu %llu %llu %llu %llu",
			&params[ADIOS_READ   ][0], &params[ADIOS_READ   ][1],
			&params[ADIOS_WRITE  ][0], &params[ADIOS_WRITE  ][1],
			&params[ADIOS_DISCARD][0], &params[ADIOS_DISCARD][1]);

		if (ret != 6)
			return -EINVAL;

		for (u8 i = ADIOS_READ; i <= ADIOS_DISCARD; i++) {
			model = &ad->latency_model[i];
			sideload_latency_model(model, params[i][0], params[i][1]);
		}
	}
	reset_buckets(ad->aggr_buckets);

	return count;
}

// Show the ADIOS version
static ssize_t adios_version_show(struct elevator_queue *e, char *page) {
	return sprintf(page, "%s\n", ADIOS_VERSION);
}

// Define sysfs attributes for dynamic thresholds
#define SHRINK_THRESHOLD_ATTR_RW(name, model_field, min_value, max_value) \
static ssize_t adios_shrink_##name##_store( \
		struct elevator_queue *e, const char *page, size_t count) { \
	struct adios_data *ad = e->elevator_data; \
	unsigned long val; \
	int ret; \
	ret = kstrtoul(page, 10, &val); \
	if (ret || val < min_value || val > max_value) \
		return -EINVAL; \
	for (u8 i = 0; i < ADIOS_OPTYPES; i++) { \
		struct latency_model *model = &ad->latency_model[i]; \
		unsigned long flags; \
		spin_lock_irqsave(&model->update_lock, flags); \
		model->model_field = val; \
		spin_unlock_irqrestore(&model->update_lock, flags); \
	} \
	return count; \
} \
static ssize_t adios_shrink_##name##_show( \
		struct elevator_queue *e, char *page) { \
	struct adios_data *ad = e->elevator_data; \
	u32 val = 0; \
	unsigned long flags; \
	struct latency_model *model = &ad->latency_model[0]; \
	spin_lock_irqsave(&model->update_lock, flags); \
	val = model->model_field; \
	spin_unlock_irqrestore(&model->update_lock, flags); \
	return sprintf(page, "%u\n", val); \
}

SHRINK_THRESHOLD_ATTR_RW(at_kreqs,  lm_shrink_at_kreqs,  1, 100000)
SHRINK_THRESHOLD_ATTR_RW(at_gbytes, lm_shrink_at_gbytes, 1,   1000)
SHRINK_THRESHOLD_ATTR_RW(resist,    lm_shrink_resist,    1,      3)

// Define sysfs attributes
#define AD_ATTR(name, show_func, store_func) \
	__ATTR(name, 0644, show_func, store_func)
#define AD_ATTR_RW(name) \
	__ATTR(name, 0644, adios_##name##_show, adios_##name##_store)
#define AD_ATTR_RO(name) \
	__ATTR(name, 0444, adios_##name##_show, NULL)
#define AD_ATTR_WO(name) \
	__ATTR(name, 0200, NULL, adios_##name##_store)

// Define sysfs attributes for ADIOS scheduler
static struct elv_fs_entry adios_sched_attrs[] = {
	AD_ATTR_RO(batch_actual_max),
	AD_ATTR_RW(bq_refill_below_ratio),
	AD_ATTR_RW(global_latency_window),
	AD_ATTR_RW(lat_model_latency_limit),
	AD_ATTR_RW(batch_order),
	AD_ATTR_RW(compliance_flags),

	AD_ATTR_RW(batch_limit_read),
	AD_ATTR_RW(batch_limit_write),
	AD_ATTR_RW(batch_limit_discard),

	AD_ATTR_RW(lat_model_read),
	AD_ATTR_RW(lat_model_write),
	AD_ATTR_RW(lat_model_discard),

	AD_ATTR_RW(lat_target_read),
	AD_ATTR_RW(lat_target_write),
	AD_ATTR_RW(lat_target_discard),

	AD_ATTR_RW(shrink_at_kreqs),
	AD_ATTR_RW(shrink_at_gbytes),
	AD_ATTR_RW(shrink_resist),

	AD_ATTR_RW(read_priority),

	AD_ATTR_WO(reset_bq_stats),
	AD_ATTR_WO(reset_lat_model),
	AD_ATTR(adios_version, adios_version_show, NULL),

	__ATTR_NULL
};

// Define the ADIOS scheduler type
static struct elevator_type mq_adios = {
	.ops = {
		.next_request		= elv_rb_latter_request,
		.former_request		= elv_rb_former_request,
		.limit_depth		= adios_limit_depth,
		.depth_updated		= adios_depth_updated,
		.request_merged		= adios_request_merged,
		.requests_merged	= adios_merged_requests,
		.bio_merge			= adios_bio_merge,
		.insert_requests	= adios_insert_requests,
		.prepare_request	= adios_prepare_request,
		.dispatch_request	= adios_dispatch_request,
		.completed_request	= adios_completed_request,
		.finish_request		= adios_finish_request,
		.has_work			= adios_has_work,
		.init_hctx			= adios_init_hctx,
		.init_sched			= adios_init_sched,
		.exit_sched			= adios_exit_sched,
	},
	.elevator_attrs = adios_sched_attrs,
	.elevator_name = "adios",
	.elevator_owner = THIS_MODULE,
};
MODULE_ALIAS("mq-adios-iosched");

#define ADIOS_PROGNAME "Adaptive Deadline I/O Scheduler"
#define ADIOS_AUTHOR   "Masahito Suzuki"

// Initialize the ADIOS scheduler module
static int __init adios_init(void) {
	printk(KERN_INFO "%s %s by %s\n",
		ADIOS_PROGNAME, ADIOS_VERSION, ADIOS_AUTHOR);
	return elv_register(&mq_adios);
}

// Exit the ADIOS scheduler module
static void __exit adios_exit(void) {
	elv_unregister(&mq_adios);
}

module_init(adios_init);
module_exit(adios_exit);

MODULE_AUTHOR(ADIOS_AUTHOR);
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION(ADIOS_PROGNAME);