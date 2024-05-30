/*
 * Copyright (c) 2016-2021 The Linux Foundation. All rights reserved.
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
#include <qdf_time.h>

#define dp_reo_alert(params...) QDF_TRACE_FATAL(QDF_MODULE_ID_DP_REO, params)
#define dp_reo_err(params...) QDF_TRACE_ERROR(QDF_MODULE_ID_DP_REO, params)
#define dp_reo_warn(params...) QDF_TRACE_WARN(QDF_MODULE_ID_DP_REO, params)
#define dp_reo_info(params...) \
	__QDF_TRACE_FL(QDF_TRACE_LEVEL_INFO_HIGH, QDF_MODULE_ID_DP_REO, ## params)
#define dp_reo_debug(params...) QDF_TRACE_DEBUG(QDF_MODULE_ID_DP_REO, params)

#ifdef WLAN_FEATURE_DP_EVENT_HISTORY
/**
 * dp_reo_cmd_srng_event_record() - Record reo cmds posted
 * to the reo cmd ring
 * @soc: dp soc handle
 * @type: reo cmd type
 * @post_status: command error status
 *
 * Return: None
 */
static
void dp_reo_cmd_srng_event_record(struct dp_soc *soc,
				  enum hal_reo_cmd_type type,
				  int post_status)
{
	struct reo_cmd_event_history *cmd_event_history =
					&soc->stats.cmd_event_history;
	struct reo_cmd_event_record *record = cmd_event_history->cmd_record;
	int record_index;

	record_index = (qdf_atomic_inc_return(&cmd_event_history->index)) &
				(REO_CMD_EVENT_HIST_MAX - 1);

	record[record_index].cmd_type = type;
	record[record_index].cmd_return_status = post_status;
	record[record_index].timestamp  = qdf_get_log_timestamp();
}
#else
static inline
void dp_reo_cmd_srng_event_record(struct dp_soc *soc,
				  enum hal_reo_cmd_type type,
				  int post_status)
{
}
#endif /*WLAN_FEATURE_DP_EVENT_HISTORY */

QDF_STATUS dp_reo_send_cmd(struct dp_soc *soc, enum hal_reo_cmd_type type,
		     struct hal_reo_cmd_params *params,
		     void (*callback_fn), void *data)
{
	struct dp_reo_cmd_info *reo_cmd;
	int num;

	num = hal_reo_send_cmd(soc->hal_soc, soc->reo_cmd_ring.hal_srng, type,
			       params);
	if (num < 0)
		return QDF_STATUS_E_INVAL;

	dp_reo_cmd_srng_event_record(soc, type, num);

	if (num < 0) {
		return QDF_STATUS_E_FAILURE;
	}

	if (callback_fn) {
		reo_cmd = qdf_mem_malloc(sizeof(*reo_cmd));
		if (!reo_cmd) {
			dp_err_log("alloc failed for REO cmd:%d!!",
				   type);
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
	hal_ring_desc_t reo_desc;
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
		QDF_STATUS status;

		processed_count++;

		status = hal_reo_status_update(soc->hal_soc,
					       reo_desc,
					       &reo_status, tlv, &num);
		if (status != QDF_STATUS_SUCCESS)
			goto next;

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
