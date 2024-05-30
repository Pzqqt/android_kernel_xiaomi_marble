/*
 * Copyright (c) 2017-2019 The Linux Foundation. All rights reserved.
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

/**
 * @file cal_client_api.h
 * @brief: define timer to update DP stats
 */
#ifndef _DP_CAL_CLIENT_H_
#define _DP_CAL_CLIENT_H_

#include<cdp_txrx_stats_struct.h>
#include <qdf_timer.h>
#include <qdf_mem.h>
#include <cdp_txrx_handle.h>

/*timer will run every 1 sec*/
#define DP_CAL_CLIENT_TIME 1000

struct cal_client {
	qdf_timer_t cal_client_timer;
	void (*iterate_update_peer_list)(struct cdp_pdev *ctx);
	struct cdp_pdev *pdev_hdl;
};

void dp_cal_client_attach(struct cdp_cal_client **cal_client_ctx,
			  struct cdp_pdev *pdev, qdf_device_t osdev,
			  void (*iterate_peer_list)(struct cdp_pdev *));
void dp_cal_client_detach(struct cdp_cal_client **cal_client_ctx);
void dp_cal_client_timer_start(void *ctx);
void dp_cal_client_timer_stop(void *ctx);
void dp_cal_client_stats_timer_fn(void *pdev_hdl);
void dp_cal_client_update_peer_stats(struct cdp_peer_stats *peer_stats);

#ifndef ATH_SUPPORT_EXT_STAT
void dp_cal_client_attach(struct cdp_cal_client **cal_client_ctx,
			  struct cdp_pdev *pdev, qdf_device_t osdev,
			  void (*iterate_peer_list)(struct cdp_pdev *))
{
}

void dp_cal_client_detach(struct cdp_cal_client **cal_client_ctx)
{
}

void dp_cal_client_timer_start(void *ctx)
{
}

void dp_cal_client_timer_stop(void *ctx)
{
}

void dp_cal_client_stats_timer_fn(void *pdev_hdl)
{
}

void dp_cal_client_update_peer_stats(struct cdp_peer_stats *peer_stats)
{
}
#endif

#endif /*_DP_CAL_CLIENT_H_*/
