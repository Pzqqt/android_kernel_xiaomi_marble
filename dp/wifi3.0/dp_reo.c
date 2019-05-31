/*
 * Copyright (c) 2016-2019 The Linux Foundation. All rights reserved.
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

#include "dp_types.h"
#include "hal_reo.h"
#include "dp_internal.h"

QDF_STATUS dp_reo_send_cmd(struct dp_soc *soc, enum hal_reo_cmd_type type,
		     struct hal_reo_cmd_params *params,
		     void (*callback_fn), void *data)
{
	struct dp_reo_cmd_info *reo_cmd;
	int num;

	switch (type) {
	case CMD_GET_QUEUE_STATS:
		num = hal_reo_cmd_queue_stats(soc->reo_cmd_ring.hal_srng,
					      soc->hal_soc, params);
		break;
	case CMD_FLUSH_QUEUE:
		num = hal_reo_cmd_flush_queue(soc->reo_cmd_ring.hal_srng,
					      soc->hal_soc, params);
		break;
	case CMD_FLUSH_CACHE:
		num = hal_reo_cmd_flush_cache(soc->reo_cmd_ring.hal_srng,
					      soc->hal_soc, params);
		break;
	case CMD_UNBLOCK_CACHE:
		num = hal_reo_cmd_unblock_cache(soc->reo_cmd_ring.hal_srng,
						soc->hal_soc, params);
		break;
	case CMD_FLUSH_TIMEOUT_LIST:
		num = hal_reo_cmd_flush_timeout_list(soc->reo_cmd_ring.hal_srng,
						     soc->hal_soc, params);
		break;
	case CMD_UPDATE_RX_REO_QUEUE:
		num = hal_reo_cmd_update_rx_queue(soc->reo_cmd_ring.hal_srng,
						  soc->hal_soc, params);
		break;
	default:
		QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
			"%s: Invalid REO command type", __func__);
		return QDF_STATUS_E_FAILURE;
	};

	if (num < 0) {
		qdf_print("%s: Error with sending REO command", __func__);
		return QDF_STATUS_E_FAILURE;
	}

	if (callback_fn) {
		reo_cmd = qdf_mem_malloc(sizeof(*reo_cmd));
		if (!reo_cmd) {
			QDF_TRACE(QDF_MODULE_ID_TXRX, QDF_TRACE_LEVEL_ERROR,
				"%s: alloc failed for REO cmd:%d!!",
				__func__, type);
			return QDF_STATUS_E_NOMEM;
		}

		reo_cmd->cmd = num;
		reo_cmd->cmd_type = type;
		reo_cmd->handler = callback_fn;
		reo_cmd->data = data;
		qdf_spin_lock_bh(&soc->rx.reo_cmd_lock);
		TAILQ_INSERT_TAIL(&soc->rx.reo_cmd_list, reo_cmd,
				  reo_cmd_list_elem);
		qdf_spin_unlock_bh(&soc->rx.reo_cmd_lock);
	}

	return QDF_STATUS_SUCCESS;
}

uint32_t dp_reo_status_ring_handler(struct dp_intr *int_ctx, struct dp_soc *soc)
{
	uint32_t *reo_desc;
	struct dp_reo_cmd_info *reo_cmd = NULL;
	union hal_reo_status reo_status;
	int num;
	int processed_count = 0;

	if (dp_srng_access_start(int_ctx, soc, soc->reo_status_ring.hal_srng)) {
		return processed_count;
	}
	reo_desc = hal_srng_dst_get_next(soc->hal_soc,
					soc->reo_status_ring.hal_srng);

	while (reo_desc) {
		uint16_t tlv = HAL_GET_TLV(reo_desc);
		processed_count++;

		switch (tlv) {
		case HAL_REO_QUEUE_STATS_STATUS_TLV:
			hal_reo_queue_stats_status(reo_desc,
					   &reo_status.queue_status,
					   soc->hal_soc);
			num = reo_status.queue_status.header.cmd_num;
			break;
		case HAL_REO_FLUSH_QUEUE_STATUS_TLV:
			hal_reo_flush_queue_status(reo_desc,
						   &reo_status.fl_queue_status,
						   soc->hal_soc);
			num = reo_status.fl_queue_status.header.cmd_num;
			break;
		case HAL_REO_FLUSH_CACHE_STATUS_TLV:
			hal_reo_flush_cache_status(reo_desc, soc->hal_soc,
						   &reo_status.fl_cache_status,
						   soc->hal_soc);
			num = reo_status.fl_cache_status.header.cmd_num;
			break;
		case HAL_REO_UNBLK_CACHE_STATUS_TLV:
			hal_reo_unblock_cache_status(reo_desc, soc->hal_soc,
						&reo_status.unblk_cache_status);
			num = reo_status.unblk_cache_status.header.cmd_num;
			break;
		case HAL_REO_TIMOUT_LIST_STATUS_TLV:
			hal_reo_flush_timeout_list_status(reo_desc,
						&reo_status.fl_timeout_status,
						soc->hal_soc);
			num = reo_status.fl_timeout_status.header.cmd_num;
			break;
		case HAL_REO_DESC_THRES_STATUS_TLV:
			hal_reo_desc_thres_reached_status(reo_desc,
						&reo_status.thres_status,
						soc->hal_soc);
			num = reo_status.thres_status.header.cmd_num;
			break;
		case HAL_REO_UPDATE_RX_QUEUE_STATUS_TLV:
			hal_reo_rx_update_queue_status(reo_desc,
						&reo_status.rx_queue_status,
						soc->hal_soc);
			num = reo_status.rx_queue_status.header.cmd_num;
			break;
		default:
			QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_WARN,
				"%s, no handler for TLV:%d", __func__, tlv);
			goto next;
		} /* switch */

		qdf_spin_lock_bh(&soc->rx.reo_cmd_lock);
		TAILQ_FOREACH(reo_cmd, &soc->rx.reo_cmd_list,
			reo_cmd_list_elem) {
			if (reo_cmd->cmd == num) {
				TAILQ_REMOVE(&soc->rx.reo_cmd_list, reo_cmd,
				reo_cmd_list_elem);
				break;
			}
		}
		qdf_spin_unlock_bh(&soc->rx.reo_cmd_lock);

		if (reo_cmd) {
			reo_cmd->handler(soc, reo_cmd->data,
					&reo_status);
			qdf_mem_free(reo_cmd);
		}

next:
		reo_desc = hal_srng_dst_get_next(soc,
						soc->reo_status_ring.hal_srng);
	} /* while */

	dp_srng_access_end(int_ctx, soc, soc->reo_status_ring.hal_srng);
	return processed_count;
}

/**
 * dp_reo_cmdlist_destroy - Free REO commands in the queue
 * @soc: DP SoC hanle
 *
 */
void dp_reo_cmdlist_destroy(struct dp_soc *soc)
{
	struct dp_reo_cmd_info *reo_cmd = NULL;
	struct dp_reo_cmd_info *tmp_cmd = NULL;
	union hal_reo_status reo_status;

	reo_status.queue_status.header.status =
		HAL_REO_CMD_DRAIN;

	qdf_spin_lock_bh(&soc->rx.reo_cmd_lock);
	TAILQ_FOREACH_SAFE(reo_cmd, &soc->rx.reo_cmd_list,
			reo_cmd_list_elem, tmp_cmd) {
		TAILQ_REMOVE(&soc->rx.reo_cmd_list, reo_cmd,
			reo_cmd_list_elem);
		reo_cmd->handler(soc, reo_cmd->data, &reo_status);
		qdf_mem_free(reo_cmd);
	}
	qdf_spin_unlock_bh(&soc->rx.reo_cmd_lock);
}
