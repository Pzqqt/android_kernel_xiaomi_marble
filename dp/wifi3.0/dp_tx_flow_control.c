/*
 * Copyright (c) 2015-2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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

#include <cds_api.h>

/* OS abstraction libraries */
#include <qdf_nbuf.h>           /* qdf_nbuf_t, etc. */
#include <qdf_atomic.h>         /* qdf_atomic_read, etc. */
#include <qdf_util.h>           /* qdf_unlikely */
#include "dp_types.h"
#include "dp_tx_desc.h"

#include <cdp_txrx_handle.h>
#include "dp_internal.h"
#define INVALID_FLOW_ID 0xFF
#define MAX_INVALID_BIN 3
#define GLOBAL_FLOW_POOL_STATS_LEN 25
#define FLOW_POOL_LOG_LEN 50

#ifdef QCA_AC_BASED_FLOW_CONTROL
/**
 * dp_tx_initialize_threshold() - Threshold of flow Pool initialization
 * @pool: flow_pool
 * @stop_threshold: stop threshold of certian AC
 * @start_threshold: start threshold of certian AC
 * @flow_pool_size: flow pool size
 *
 * Return: none
 */
static inline void
dp_tx_initialize_threshold(struct dp_tx_desc_pool_s *pool,
			   uint32_t start_threshold,
			   uint32_t stop_threshold,
			   uint16_t flow_pool_size)
{
	/* BE_BK threshold is same as previous threahold */
	pool->start_th[DP_TH_BE_BK] = (start_threshold
					* flow_pool_size) / 100;
	pool->stop_th[DP_TH_BE_BK] = (stop_threshold
					* flow_pool_size) / 100;

	/* Update VI threshold based on BE_BK threashold */
	pool->start_th[DP_TH_VI] = (pool->start_th[DP_TH_BE_BK]
					* FL_TH_VI_PERCENTAGE) / 100;
	pool->stop_th[DP_TH_VI] = (pool->stop_th[DP_TH_BE_BK]
					* FL_TH_VI_PERCENTAGE) / 100;

	/* Update VO threshold based on BE_BK threashold */
	pool->start_th[DP_TH_VO] = (pool->start_th[DP_TH_BE_BK]
					* FL_TH_VO_PERCENTAGE) / 100;
	pool->stop_th[DP_TH_VO] = (pool->stop_th[DP_TH_BE_BK]
					* FL_TH_VO_PERCENTAGE) / 100;

	/* Update High Priority threshold based on BE_BK threashold */
	pool->start_th[DP_TH_HI] = (pool->start_th[DP_TH_BE_BK]
					* FL_TH_HI_PERCENTAGE) / 100;
	pool->stop_th[DP_TH_HI] = (pool->stop_th[DP_TH_BE_BK]
					* FL_TH_HI_PERCENTAGE) / 100;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		  "%s: tx flow control threshold is set, pool size is %d",
		  __func__, flow_pool_size);
}

/**
 * dp_tx_flow_pool_reattach() - Reattach flow_pool
 * @pool: flow_pool
 *
 * Return: none
 */
static inline void
dp_tx_flow_pool_reattach(struct dp_tx_desc_pool_s *pool)
{
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		  "%s: flow pool already allocated, attached %d times",
		  __func__, pool->pool_create_cnt);

	pool->status = FLOW_POOL_ACTIVE_UNPAUSED_REATTACH;
	pool->pool_create_cnt++;
}

/**
 * dp_tx_flow_pool_dump_threshold() - Dump threshold of the flow_pool
 * @pool: flow_pool
 *
 * Return: none
 */
static inline void
dp_tx_flow_pool_dump_threshold(struct dp_tx_desc_pool_s *pool)
{
	int i;

	for (i = 0; i < FL_TH_MAX; i++) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Level %d :: Start threshold %d :: Stop threshold %d",
			  i, pool->start_th[i], pool->stop_th[i]);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Level %d :: Maximun pause time %lu ms",
			  i, pool->max_pause_time[i]);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			  "Level %d :: Latest pause timestamp %lu",
			  i, pool->latest_pause_time[i]);
	}
}

/**
 * dp_tx_flow_ctrl_reset_subqueues() - Reset subqueues to orginal state
 * @soc: dp soc
 * @pool: flow pool
 * @pool_status: flow pool status
 *
 * Return: none
 */
static inline void
dp_tx_flow_ctrl_reset_subqueues(struct dp_soc *soc,
				struct dp_tx_desc_pool_s *pool,
				enum flow_pool_status pool_status)
{
	switch (pool_status) {
	case FLOW_POOL_ACTIVE_PAUSED:
		soc->pause_cb(pool->flow_pool_id,
			      WLAN_NETIF_PRIORITY_QUEUE_ON,
			      WLAN_DATA_FLOW_CTRL_PRI);

	case FLOW_POOL_VO_PAUSED:
		soc->pause_cb(pool->flow_pool_id,
			      WLAN_NETIF_VO_QUEUE_ON,
			      WLAN_DATA_FLOW_CTRL_VO);

	case FLOW_POOL_VI_PAUSED:
		soc->pause_cb(pool->flow_pool_id,
			      WLAN_NETIF_VI_QUEUE_ON,
			      WLAN_DATA_FLOW_CTRL_VI);

	case FLOW_POOL_BE_BK_PAUSED:
		soc->pause_cb(pool->flow_pool_id,
			      WLAN_NETIF_BE_BK_QUEUE_ON,
			      WLAN_DATA_FLOW_CTRL_BE_BK);
	default:
		break;
	}
}

#else
static inline void
dp_tx_initialize_threshold(struct dp_tx_desc_pool_s *pool,
			   uint32_t start_threshold,
			   uint32_t stop_threshold,
			   uint16_t flow_pool_size)

{
	/* INI is in percentage so divide by 100 */
	pool->start_th = (start_threshold * flow_pool_size) / 100;
	pool->stop_th = (stop_threshold * flow_pool_size) / 100;
}

static inline void
dp_tx_flow_pool_reattach(struct dp_tx_desc_pool_s *pool)
{
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		  "%s: flow pool already allocated, attached %d times",
		  __func__, pool->pool_create_cnt);
	if (pool->avail_desc > pool->start_th)
		pool->status = FLOW_POOL_ACTIVE_UNPAUSED;
	else
		pool->status = FLOW_POOL_ACTIVE_PAUSED;

	pool->pool_create_cnt++;
}

static inline void
dp_tx_flow_pool_dump_threshold(struct dp_tx_desc_pool_s *pool)
{
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		  "Start threshold %d :: Stop threshold %d",
	pool->start_th, pool->stop_th);
}

static inline void
dp_tx_flow_ctrl_reset_subqueues(struct dp_soc *soc,
				struct dp_tx_desc_pool_s *pool,
				enum flow_pool_status pool_status)
{
}

#endif

/**
 * dp_tx_dump_flow_pool_info() - dump global_pool and flow_pool info
 *
 * @ctx: Handle to struct dp_soc.
 *
 * Return: none
 */
void dp_tx_dump_flow_pool_info(struct cdp_soc_t *soc_hdl)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(soc_hdl);
	struct dp_txrx_pool_stats *pool_stats = &soc->pool_stats;
	struct dp_tx_desc_pool_s *pool = NULL;
	struct dp_tx_desc_pool_s tmp_pool;
	int i;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"No of pool map received %d", pool_stats->pool_map_count);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"No of pool unmap received %d",	pool_stats->pool_unmap_count);
	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		"Pkt dropped due to unavailablity of pool %d",
		pool_stats->pkt_drop_no_pool);

	/*
	 * Nested spin lock.
	 * Always take in below order.
	 * flow_pool_array_lock -> flow_pool_lock
	 */
	qdf_spin_lock_bh(&soc->flow_pool_array_lock);
	for (i = 0; i < MAX_TXDESC_POOLS; i++) {
		pool = &soc->tx_desc[i];
		if (pool->status > FLOW_POOL_INVALID)
			continue;
		qdf_spin_lock_bh(&pool->flow_pool_lock);
		qdf_mem_copy(&tmp_pool, pool, sizeof(tmp_pool));
		qdf_spin_unlock_bh(&pool->flow_pool_lock);
		qdf_spin_unlock_bh(&soc->flow_pool_array_lock);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR, "\n");
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Flow_pool_id %d :: status %d",
			tmp_pool.flow_pool_id, tmp_pool.status);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Total %d :: Available %d",
			tmp_pool.pool_size, tmp_pool.avail_desc);
		dp_tx_flow_pool_dump_threshold(&tmp_pool);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Member flow_id  %d :: flow_type %d",
			tmp_pool.flow_pool_id, tmp_pool.flow_type);
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Pkt dropped due to unavailablity of descriptors %d",
			tmp_pool.pkt_drop_no_desc);
		qdf_spin_lock_bh(&soc->flow_pool_array_lock);
	}
	qdf_spin_unlock_bh(&soc->flow_pool_array_lock);
}

void dp_tx_dump_flow_pool_info_compact(struct dp_soc *soc)
{
	struct dp_txrx_pool_stats *pool_stats = &soc->pool_stats;
	struct dp_tx_desc_pool_s *pool = NULL;
	char *comb_log_str;
	uint32_t comb_log_str_size;
	int bytes_written = 0;
	int i;

	comb_log_str_size = GLOBAL_FLOW_POOL_STATS_LEN +
				(FLOW_POOL_LOG_LEN * MAX_TXDESC_POOLS) + 1;
	comb_log_str = qdf_mem_malloc(comb_log_str_size);
	if (!comb_log_str)
		return;

	bytes_written = qdf_snprintf(&comb_log_str[bytes_written],
				     comb_log_str_size, "G:(%d,%d,%d) ",
				     pool_stats->pool_map_count,
				     pool_stats->pool_unmap_count,
				     pool_stats->pkt_drop_no_pool);

	for (i = 0; i < MAX_TXDESC_POOLS; i++) {
		pool = &soc->tx_desc[i];
		if (pool->status > FLOW_POOL_INVALID)
			continue;
		bytes_written += qdf_snprintf(&comb_log_str[bytes_written],
				      (bytes_written >= comb_log_str_size) ? 0 :
				      comb_log_str_size - bytes_written,
				      "| %d %d: (%d,%d,%d)",
				      pool->flow_pool_id, pool->status,
				      pool->pool_size, pool->avail_desc,
				      pool->pkt_drop_no_desc);
	}

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO_HIGH,
		  "FLOW_POOL_STATS %s", comb_log_str);

	qdf_mem_free(comb_log_str);
}

/**
 * dp_tx_clear_flow_pool_stats() - clear flow pool statistics
 *
 * @soc: Handle to struct dp_soc.
 *
 * Return: None
 */
void dp_tx_clear_flow_pool_stats(struct dp_soc *soc)
{

	if (!soc) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s: soc is null", __func__);
		return;
	}
	qdf_mem_zero(&soc->pool_stats, sizeof(soc->pool_stats));
}

/**
 * dp_tx_create_flow_pool() - create flow pool
 * @soc: Handle to struct dp_soc
 * @flow_pool_id: flow pool id
 * @flow_pool_size: flow pool size
 *
 * Return: flow_pool pointer / NULL for error
 */
struct dp_tx_desc_pool_s *dp_tx_create_flow_pool(struct dp_soc *soc,
	uint8_t flow_pool_id, uint16_t flow_pool_size)
{
	struct dp_tx_desc_pool_s *pool;
	uint32_t stop_threshold;
	uint32_t start_threshold;

	if (flow_pool_id >= MAX_TXDESC_POOLS) {
		dp_err("invalid flow_pool_id %d", flow_pool_id);
		return NULL;
	}
	pool = &soc->tx_desc[flow_pool_id];
	qdf_spin_lock_bh(&pool->flow_pool_lock);
	if ((pool->status != FLOW_POOL_INACTIVE) || pool->pool_create_cnt) {
		dp_tx_flow_pool_reattach(pool);
		qdf_spin_unlock_bh(&pool->flow_pool_lock);
		dp_err("cannot alloc desc, status=%d, create_cnt=%d",
		       pool->status, pool->pool_create_cnt);
		return pool;
	}

	if (dp_tx_desc_pool_alloc(soc, flow_pool_id, flow_pool_size)) {
		qdf_spin_unlock_bh(&pool->flow_pool_lock);
		return NULL;
	}

	if (dp_tx_desc_pool_init(soc, flow_pool_id, flow_pool_size)) {
		dp_tx_desc_pool_free(soc, flow_pool_id);
		qdf_spin_unlock_bh(&pool->flow_pool_lock);
		return NULL;
	}

	stop_threshold = wlan_cfg_get_tx_flow_stop_queue_th(soc->wlan_cfg_ctx);
	start_threshold = stop_threshold +
		wlan_cfg_get_tx_flow_start_queue_offset(soc->wlan_cfg_ctx);

	pool->flow_pool_id = flow_pool_id;
	pool->pool_size = flow_pool_size;
	pool->avail_desc = flow_pool_size;
	pool->status = FLOW_POOL_ACTIVE_UNPAUSED;
	dp_tx_initialize_threshold(pool, start_threshold, stop_threshold,
				   flow_pool_size);
	pool->pool_create_cnt++;

	qdf_spin_unlock_bh(&pool->flow_pool_lock);

	return pool;
}

/**
 * dp_tx_delete_flow_pool() - delete flow pool
 * @soc: Handle to struct dp_soc
 * @pool: flow pool pointer
 * @force: free pool forcefully
 *
 * Delete flow_pool if all tx descriptors are available.
 * Otherwise put it in FLOW_POOL_INVALID state.
 * If force is set then pull all available descriptors to
 * global pool.
 *
 * Return: 0 for success or error
 */
int dp_tx_delete_flow_pool(struct dp_soc *soc, struct dp_tx_desc_pool_s *pool,
	bool force)
{
	struct dp_vdev *vdev;
	enum flow_pool_status pool_status;

	if (!soc || !pool) {
		dp_err("pool or soc is NULL");
		QDF_ASSERT(0);
		return ENOMEM;
	}

	dp_info("pool create_cnt=%d, avail_desc=%d, size=%d, status=%d",
		pool->pool_create_cnt, pool->avail_desc,
		pool->pool_size, pool->status);
	qdf_spin_lock_bh(&pool->flow_pool_lock);
	if (!pool->pool_create_cnt) {
		qdf_spin_unlock_bh(&pool->flow_pool_lock);
		dp_err("flow pool either not created or alread deleted");
		return -ENOENT;
	}
	pool->pool_create_cnt--;
	if (pool->pool_create_cnt) {
		qdf_spin_unlock_bh(&pool->flow_pool_lock);
		dp_err("pool is still attached, pending detach %d",
		       pool->pool_create_cnt);
		return -EAGAIN;
	}

	if (pool->avail_desc < pool->pool_size) {
		pool_status = pool->status;
		pool->status = FLOW_POOL_INVALID;
		dp_tx_flow_ctrl_reset_subqueues(soc, pool, pool_status);

		qdf_spin_unlock_bh(&pool->flow_pool_lock);
		/* Reset TX desc associated to this Vdev as NULL */
		vdev = dp_vdev_get_ref_by_id(soc, pool->flow_pool_id,
					     DP_MOD_ID_MISC);
		if (vdev) {
			dp_tx_desc_flush(vdev->pdev, vdev, false);
			dp_vdev_unref_delete(soc, vdev,
					     DP_MOD_ID_MISC);
		}
		dp_err("avail desc less than pool size");
		return -EAGAIN;
	}

	/* We have all the descriptors for the pool, we can delete the pool */
	dp_tx_desc_pool_deinit(soc, pool->flow_pool_id);
	dp_tx_desc_pool_free(soc, pool->flow_pool_id);
	qdf_spin_unlock_bh(&pool->flow_pool_lock);
	return 0;
}

/**
 * dp_tx_flow_pool_vdev_map() - Map flow_pool with vdev
 * @pdev: Handle to struct dp_pdev
 * @pool: flow_pool
 * @vdev_id: flow_id /vdev_id
 *
 * Return: none
 */
static void dp_tx_flow_pool_vdev_map(struct dp_pdev *pdev,
	struct dp_tx_desc_pool_s *pool, uint8_t vdev_id)
{
	struct dp_vdev *vdev;
	struct dp_soc *soc = pdev->soc;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);
	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		   "%s: invalid vdev_id %d",
		   __func__, vdev_id);
		return;
	}

	vdev->pool = pool;
	qdf_spin_lock_bh(&pool->flow_pool_lock);
	pool->pool_owner_ctx = soc;
	pool->flow_pool_id = vdev_id;
	qdf_spin_unlock_bh(&pool->flow_pool_lock);
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
}

/**
 * dp_tx_flow_pool_vdev_unmap() - Unmap flow_pool from vdev
 * @pdev: Handle to struct dp_pdev
 * @pool: flow_pool
 * @vdev_id: flow_id /vdev_id
 *
 * Return: none
 */
static void dp_tx_flow_pool_vdev_unmap(struct dp_pdev *pdev,
		struct dp_tx_desc_pool_s *pool, uint8_t vdev_id)
{
	struct dp_vdev *vdev;
	struct dp_soc *soc = pdev->soc;

	vdev = dp_vdev_get_ref_by_id(soc, vdev_id, DP_MOD_ID_CDP);
	if (!vdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		   "%s: invalid vdev_id %d",
		   __func__, vdev_id);
		return;
	}

	vdev->pool = NULL;
	dp_vdev_unref_delete(soc, vdev, DP_MOD_ID_CDP);
}

/**
 * dp_tx_flow_pool_map_handler() - Map flow_id with pool of descriptors
 * @pdev: Handle to struct dp_pdev
 * @flow_id: flow id
 * @flow_type: flow type
 * @flow_pool_id: pool id
 * @flow_pool_size: pool size
 *
 * Process below target to host message
 * HTT_T2H_MSG_TYPE_FLOW_POOL_MAP
 *
 * Return: none
 */
QDF_STATUS dp_tx_flow_pool_map_handler(struct dp_pdev *pdev, uint8_t flow_id,
	uint8_t flow_type, uint8_t flow_pool_id, uint16_t flow_pool_size)
{
	struct dp_soc *soc = pdev->soc;
	struct dp_tx_desc_pool_s *pool;
	enum htt_flow_type type = flow_type;


	dp_info("flow_id %d flow_type %d flow_pool_id %d flow_pool_size %d",
		flow_id, flow_type, flow_pool_id, flow_pool_size);

	if (qdf_unlikely(!soc)) {
		dp_err("soc is NULL");
		return QDF_STATUS_E_FAULT;
	}
	soc->pool_stats.pool_map_count++;

	pool = dp_tx_create_flow_pool(soc, flow_pool_id,
			flow_pool_size);
	if (!pool) {
		dp_err("creation of flow_pool %d size %d failed",
		       flow_pool_id, flow_pool_size);
		return QDF_STATUS_E_RESOURCES;
	}

	switch (type) {

	case FLOW_TYPE_VDEV:
		dp_tx_flow_pool_vdev_map(pdev, pool, flow_id);
		break;
	default:
		dp_err("flow type %d not supported", type);
		break;
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_tx_flow_pool_unmap_handler() - Unmap flow_id from pool of descriptors
 * @pdev: Handle to struct dp_pdev
 * @flow_id: flow id
 * @flow_type: flow type
 * @flow_pool_id: pool id
 *
 * Process below target to host message
 * HTT_T2H_MSG_TYPE_FLOW_POOL_UNMAP
 *
 * Return: none
 */
void dp_tx_flow_pool_unmap_handler(struct dp_pdev *pdev, uint8_t flow_id,
	uint8_t flow_type, uint8_t flow_pool_id)
{
	struct dp_soc *soc = pdev->soc;
	struct dp_tx_desc_pool_s *pool;
	enum htt_flow_type type = flow_type;

	QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_INFO,
		"%s: flow_id %d flow_type %d flow_pool_id %d",
		__func__, flow_id, flow_type, flow_pool_id);

	if (qdf_unlikely(!pdev)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"%s: pdev is NULL", __func__);
		return;
	}
	soc->pool_stats.pool_unmap_count++;

	pool = &soc->tx_desc[flow_pool_id];
	if (!pool) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		   "%s: flow_pool not available flow_pool_id %d",
		   __func__, type);
		return;
	}

	switch (type) {

	case FLOW_TYPE_VDEV:
		dp_tx_flow_pool_vdev_unmap(pdev, pool, flow_id);
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
		   "%s: flow type %d not supported !!!",
		   __func__, type);
		return;
	}

	/* only delete if all descriptors are available */
	dp_tx_delete_flow_pool(soc, pool, false);
}

/**
 * dp_tx_flow_control_init() - Initialize tx flow control
 * @tx_desc_pool: Handle to flow_pool
 *
 * Return: none
 */
void dp_tx_flow_control_init(struct dp_soc *soc)
{
	qdf_spinlock_create(&soc->flow_pool_array_lock);
}

/**
 * dp_tx_desc_pool_dealloc() - De-allocate tx desc pool
 * @tx_desc_pool: Handle to flow_pool
 *
 * Return: none
 */
static inline void dp_tx_desc_pool_dealloc(struct dp_soc *soc)
{
	struct dp_tx_desc_pool_s *tx_desc_pool;
	int i;

	for (i = 0; i < MAX_TXDESC_POOLS; i++) {
		tx_desc_pool = &((soc)->tx_desc[i]);
		if (!tx_desc_pool->desc_pages.num_pages)
			continue;

		dp_tx_desc_pool_deinit(soc, i);
		dp_tx_desc_pool_free(soc, i);
	}
}

/**
 * dp_tx_flow_control_deinit() - Deregister fw based tx flow control
 * @tx_desc_pool: Handle to flow_pool
 *
 * Return: none
 */
void dp_tx_flow_control_deinit(struct dp_soc *soc)
{
	dp_tx_desc_pool_dealloc(soc);

	qdf_spinlock_destroy(&soc->flow_pool_array_lock);
}

/**
 * dp_txrx_register_pause_cb() - Register pause callback
 * @ctx: Handle to struct dp_soc
 * @pause_cb: Tx pause_cb
 *
 * Return: none
 */
QDF_STATUS dp_txrx_register_pause_cb(struct cdp_soc_t *handle,
	tx_pause_callback pause_cb)
{
	struct dp_soc *soc = (struct dp_soc *)handle;

	if (!soc || !pause_cb) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			FL("soc or pause_cb is NULL"));
		return QDF_STATUS_E_INVAL;
	}
	soc->pause_cb = pause_cb;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_tx_flow_pool_map(struct cdp_soc_t *handle, uint8_t pdev_id,
			       uint8_t vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(handle);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);
	int tx_ring_size = wlan_cfg_get_num_tx_desc(soc->wlan_cfg_ctx);

	if (!pdev) {
		dp_err("pdev is NULL");
		return QDF_STATUS_E_INVAL;
	}

	return dp_tx_flow_pool_map_handler(pdev, vdev_id, FLOW_TYPE_VDEV,
					   vdev_id, tx_ring_size);
}

void dp_tx_flow_pool_unmap(struct cdp_soc_t *handle, uint8_t pdev_id,
			   uint8_t vdev_id)
{
	struct dp_soc *soc = cdp_soc_t_to_dp_soc(handle);
	struct dp_pdev *pdev =
		dp_get_pdev_from_soc_pdev_id_wifi3(soc, pdev_id);

	if (!pdev) {
		dp_err("pdev is NULL");
		return;
	}

	return dp_tx_flow_pool_unmap_handler(pdev, vdev_id,
					     FLOW_TYPE_VDEV, vdev_id);
}
