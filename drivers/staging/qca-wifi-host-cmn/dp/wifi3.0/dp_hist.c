/*
 * Copyright (c) 2020 The Linux Foundation. All rights reserved.
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

#include <qdf_util.h>
#include <qdf_mem.h>
#include <cdp_txrx_hist_struct.h>
#include "dp_hist.h"

/*
 * dp_hist_sw_enq_dbucket: Sofware enqueue delay bucket in ms
 * @index_0 = 0_1 ms
 * @index_1 = 1_2 ms
 * @index_2 = 2_3 ms
 * @index_3 = 3_4 ms
 * @index_4 = 4_5 ms
 * @index_5 = 5_6 ms
 * @index_6 = 6_7 ms
 * @index_7 = 7_8 ms
 * @index_8 = 8_9 ms
 * @index_8 = 9+ ms
 */
static uint16_t dp_hist_sw_enq_dbucket[CDP_HIST_BUCKET_MAX] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

/*
 * cdp_hist_fw2hw_dbucket: HW enqueue to Completion Delay
 * @index_0 = 0_10 ms
 * @index_1 = 10_20 ms
 * @index_2 = 20_30ms
 * @index_3 = 30_40 ms
 * @index_4 = 40_50 ms
 * @index_5 = 50_60 ms
 * @index_6 = 60_70 ms
 * @index_7 = 70_80 ms
 * @index_8 = 80_90 ms
 * @index_9 = 90+ ms
 */
static uint16_t dp_hist_fw2hw_dbucket[CDP_HIST_BUCKET_MAX] = {
	0, 10, 20, 30, 40, 50, 60, 70, 80, 90};

/*
 * dp_hist_reap2stack_bucket: Reap to stack bucket
 * @index_0 = 0_5 ms
 * @index_1 = 5_10 ms
 * @index_2 = 10_15 ms
 * @index_3 = 15_20 ms
 * @index_4 = 20_25 ms
 * @index_5 = 25_30 ms
 * @index_6 = 30_35 ms
 * @index_7 = 35_40 ms
 * @index_8 = 40_45 ms
 * @index_9 = 45+ ms
 */
static uint16_t dp_hist_reap2stack_bucket[CDP_HIST_BUCKET_MAX] = {
	0, 5, 10, 15, 20, 25, 30, 35, 40, 45};

/*
 * dp_hist_find_bucket_idx: Find the bucket index
 * @bucket_array: Bucket array
 * @value: Frequency value
 *
 * Return: The bucket index
 */
static int dp_hist_find_bucket_idx(int16_t *bucket_array, int value)
{
	uint8_t idx = CDP_HIST_BUCKET_0;

	for (; idx < (CDP_HIST_BUCKET_MAX - 1); idx++) {
		if (value < bucket_array[idx + 1])
			break;
	}

	return idx;
}

/*
 * dp_hist_fill_buckets: Fill the histogram frequency buckets
 * @hist_bucket: Histogram bukcets
 * @value: Frequency value
 *
 * Return: void
 */
static void dp_hist_fill_buckets(struct cdp_hist_bucket *hist_bucket, int value)
{
	enum cdp_hist_types hist_type;
	int idx = CDP_HIST_BUCKET_MAX;

	if (qdf_unlikely(!hist_bucket))
		return;

	hist_type = hist_bucket->hist_type;

	/* Identify the bucket the bucket and update. */
	switch (hist_type) {
	case CDP_HIST_TYPE_SW_ENQEUE_DELAY:
		idx =  dp_hist_find_bucket_idx(&dp_hist_sw_enq_dbucket[0],
					       value);
		break;
	case CDP_HIST_TYPE_HW_COMP_DELAY:
		idx =  dp_hist_find_bucket_idx(&dp_hist_fw2hw_dbucket[0],
					       value);
		break;
	case CDP_HIST_TYPE_REAP_STACK:
		idx =  dp_hist_find_bucket_idx(
				&dp_hist_reap2stack_bucket[0], value);
		break;
	default:
		break;
	}

	if (idx == CDP_HIST_BUCKET_MAX)
		return;

	hist_bucket->freq[idx]++;
}

/*
 * dp_hist_update_stats: Update histogram stats
 * @hist_stats: Hist stats object
 * @value: Delay value
 *
 * Return: void
 */
void dp_hist_update_stats(struct cdp_hist_stats *hist_stats, int value)
{
	if (qdf_unlikely(!hist_stats))
		return;

	/*
	 * Fill the histogram buckets according to the delay
	 */
	dp_hist_fill_buckets(&hist_stats->hist, value);

	/*
	 * Compute the min, max and average. Average computed is weighted
	 * average
	 */
	if (value < hist_stats->min)
		hist_stats->min = value;

	if (value > hist_stats->max)
		hist_stats->max = value;

	if (qdf_unlikely(!hist_stats->avg))
		hist_stats->avg = value;
	else
		hist_stats->avg = hist_stats->avg +
			((value - hist_stats->avg) >> HIST_AVG_WEIGHT_DENOM);
}

/*
 * dp_copy_hist_stats(): Copy the histogram stats
 * @src_hist_stats: Source histogram stats
 * @dst_hist_stats: Destination histogram stats
 *
 * Return: void
 */
void dp_copy_hist_stats(struct cdp_hist_stats *src_hist_stats,
			struct cdp_hist_stats *dst_hist_stats)
{
	uint8_t index;

	for (index = 0; index < CDP_HIST_BUCKET_MAX; index++)
		dst_hist_stats->hist.freq[index] =
			src_hist_stats->hist.freq[index];
	dst_hist_stats->min = src_hist_stats->min;
	dst_hist_stats->max = src_hist_stats->max;
	dst_hist_stats->avg = src_hist_stats->avg;
}

/*
 * dp_accumulate_hist_stats(): Accumulate the hist src to dst
 * @src_hist_stats: Source histogram stats
 * @dst_hist_stats: Destination histogram stats
 *
 * Return: void
 */
void dp_accumulate_hist_stats(struct cdp_hist_stats *src_hist_stats,
			      struct cdp_hist_stats *dst_hist_stats)
{
	uint8_t index;

	for (index = 0; index < CDP_HIST_BUCKET_MAX; index++)
		dst_hist_stats->hist.freq[index] +=
			src_hist_stats->hist.freq[index];
	dst_hist_stats->min = QDF_MIN(src_hist_stats->min, dst_hist_stats->min);
	dst_hist_stats->max = QDF_MAX(src_hist_stats->max, dst_hist_stats->max);
	dst_hist_stats->avg = (src_hist_stats->avg + dst_hist_stats->avg) >> 1;
}

/*
 * dp_hist_init(): Initialize the histogram object
 * @hist_stats: Hist stats object
 * @hist_type: Histogram type
 */
void dp_hist_init(struct cdp_hist_stats *hist_stats,
		  enum cdp_hist_types hist_type)
{
	qdf_mem_zero(hist_stats, sizeof(*hist_stats));
	hist_stats->hist.hist_type = hist_type;
}
