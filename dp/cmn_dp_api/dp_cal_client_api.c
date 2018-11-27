/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
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

#include "dp_cal_client_api.h"
#include "qdf_module.h"

/* dp_cal_client_attach - function to attach cal client timer
 * @cal_client_ctx: cal client timer context
 * @pdev: pdev handle
 * @osdev: device pointer
 * @dp_iterate_peer_list : function pointer to iterate and update peer stats
 *
 * return: void
 */
void dp_cal_client_attach(void **cal_client_ctx, void *pdev,
			  qdf_device_t osdev,
			  void (*dp_iterate_peer_list)(void *))
{
	struct cal_client *cal_cl;

	*cal_client_ctx = qdf_mem_malloc(sizeof(struct cal_client));

	if (!(*cal_client_ctx))
		return;

	cal_cl = (struct cal_client *)(*cal_client_ctx);
	cal_cl->iterate_update_peer_list = dp_iterate_peer_list;
	cal_cl->pdev_hdl = pdev;

	qdf_timer_init(osdev, &cal_cl->cal_client_timer,
		       dp_cal_client_stats_timer_fn, *cal_client_ctx,
		       QDF_TIMER_TYPE_WAKE_APPS);
}

qdf_export_symbol(dp_cal_client_attach);

/* dp_cal_client_detach - detach cal client timer
 * @cal_client_ctx: cal client timer context
 *
 * return: void
 */
void dp_cal_client_detach(void **cal_client_ctx)
{
	struct cal_client *cal_cl;

	if (*cal_client_ctx) {
		cal_cl = (struct cal_client *)*cal_client_ctx;

		qdf_timer_stop(&cal_cl->cal_client_timer);
		qdf_timer_free(&cal_cl->cal_client_timer);
		qdf_mem_free(cal_cl);
		*cal_client_ctx = NULL;
	}
}

qdf_export_symbol(dp_cal_client_detach);

/* dp_cal_client_timer_start- api to start cal client timer
 * @ctx: cal client timer ctx
 *
 * return: void
 */
void dp_cal_client_timer_start(void *ctx)
{
	struct cal_client *cal_cl;

	if (ctx) {
		cal_cl  = (struct cal_client *)ctx;
		qdf_timer_start(&cal_cl->cal_client_timer, DP_CAL_CLIENT_TIME);
	}
}

qdf_export_symbol(dp_cal_client_timer_start);

/* dp_cal_client_timer_stop- api to stop cal client timer
 * @ctx: cal client timer ctx
 *
 * return: void
 */
void dp_cal_client_timer_stop(void *ctx)
{
	struct cal_client *cal_cl;

	if (ctx) {
		cal_cl = (struct cal_client *)ctx;
		qdf_timer_sync_cancel(&cal_cl->cal_client_timer);
		qdf_timer_stop(&cal_cl->cal_client_timer);
	}
}

qdf_export_symbol(dp_cal_client_timer_stop);

/* dp_cal_client_stats_timer_fn- function called on timer interval
 * @ctx: cal client timer ctx
 *
 * return: void
 */
void dp_cal_client_stats_timer_fn(void *ctx)
{
	struct cal_client *cal_cl = (struct cal_client *)ctx;

	if (!cal_cl)
		return;

	cal_cl->iterate_update_peer_list(cal_cl->pdev_hdl);
	qdf_timer_mod(&cal_cl->cal_client_timer, DP_CAL_CLIENT_TIME);
}

qdf_export_symbol(dp_cal_client_stats_timer_fn);

/*dp_cal_client_update_peer_stats - update peer stats in peer
 * @peer_stats: cdp peer stats pointer
 *
 * return: void
 */
void dp_cal_client_update_peer_stats(struct cdp_peer_stats *peer_stats)
{
	uint32_t temp_rx_bytes = peer_stats->rx.to_stack.bytes;
	uint32_t temp_rx_data = peer_stats->rx.to_stack.num;
	uint32_t temp_tx_bytes = peer_stats->tx.tx_success.bytes;
	uint32_t temp_tx_data = peer_stats->tx.tx_success.num;
	uint32_t temp_tx_ucast_pkts = peer_stats->tx.ucast.num;

	peer_stats->rx.rx_byte_rate = temp_rx_bytes -
					peer_stats->rx.rx_bytes_success_last;
	peer_stats->rx.rx_data_rate  = temp_rx_data -
					peer_stats->rx.rx_data_success_last;
	peer_stats->tx.tx_byte_rate = temp_tx_bytes -
					peer_stats->tx.tx_bytes_success_last;
	peer_stats->tx.tx_data_rate  = temp_tx_data -
					peer_stats->tx.tx_data_success_last;
	peer_stats->tx.tx_data_ucast_rate = temp_tx_ucast_pkts -
					peer_stats->tx.tx_data_ucast_last;

	peer_stats->rx.rx_bytes_success_last = temp_rx_bytes;
	peer_stats->rx.rx_data_success_last = temp_rx_data;
	peer_stats->tx.tx_bytes_success_last = temp_tx_bytes;
	peer_stats->tx.tx_data_success_last = temp_tx_data;
	peer_stats->tx.tx_data_ucast_last = temp_tx_ucast_pkts;

	if (peer_stats->tx.tx_data_ucast_rate) {
		if (peer_stats->tx.tx_data_ucast_rate >
				peer_stats->tx.tx_data_rate)
			peer_stats->tx.last_per =
				((peer_stats->tx.tx_data_ucast_rate -
					peer_stats->tx.tx_data_rate) * 100) /
				peer_stats->tx.tx_data_ucast_rate;
		else
			peer_stats->tx.last_per = 0;
	}

}

qdf_export_symbol(dp_cal_client_update_peer_stats);

