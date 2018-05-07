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


#include "dp_internal.h"
#include "qdf_mem.h"   /* qdf_mem_malloc,free */

#ifdef WDI_EVENT_ENABLE
void *dp_get_pldev(struct cdp_pdev *txrx_pdev)
{
	struct dp_pdev *pdev = (struct dp_pdev *)txrx_pdev;
	return pdev->pl_dev;
}
/*
 * dp_wdi_event_next_sub() - Return handle for Next WDI event
 * @wdi_sub: WDI Event handle
 *
 * Return handle for next WDI event in list
 *
 * Return: Next WDI event to be subscribe
 */
static inline wdi_event_subscribe *
dp_wdi_event_next_sub(wdi_event_subscribe *wdi_sub)
{
	if (!wdi_sub) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid subscriber in %s\n", __func__);
		return NULL;
	}
	return wdi_sub->priv.next;
}


/*
 * dp_wdi_event_del_subs() -Delete Event subscription
 * @wdi_sub: WDI Event handle
 * @event_index: Event index from list
 *
 * This API will delete subscribed event from list
 * Return: None
 */
static inline void
dp_wdi_event_del_subs(wdi_event_subscribe *wdi_sub, int event_index)
{
	/* Subscribers should take care of deletion */
}


/*
 * dp_wdi_event_iter_sub() - Iterate through all WDI event in the list
 * and pass WDI event to callback function
 * @pdev: DP pdev handle
 * @event_index: Event index in list
 * @wdi_event: WDI event handle
 * @data: pointer to data
 * @peer_id: peer id number
 * @status: HTT rx status
 *
 *
 * Return: None
 */
static inline void
dp_wdi_event_iter_sub(
	struct dp_pdev *pdev,
	uint32_t event_index,
	wdi_event_subscribe *wdi_sub,
	void *data,
	uint16_t peer_id,
	int status)
{
	enum WDI_EVENT event = event_index + WDI_EVENT_BASE;

	if (wdi_sub) {
		do {
			wdi_sub->callback(wdi_sub->context, event, data,
					peer_id, status);
		} while ((wdi_sub = dp_wdi_event_next_sub(wdi_sub)));
	}
}


/*
 * dp_wdi_event_handler() - Event handler for WDI event
 * @event: wdi event number
 * @soc: soc handle
 * @data: pointer to data
 * @peer_id: peer id number
 * @status: HTT rx status
 * @pdev_id: id of pdev
 *
 * It will be called to register WDI event
 *
 * Return: None
 */
void
dp_wdi_event_handler(
	enum WDI_EVENT event,
	void *soc,
	void *data,
	uint16_t peer_id,
	int status, uint8_t pdev_id)
{
	uint32_t event_index;
	wdi_event_subscribe *wdi_sub;
	struct dp_pdev *txrx_pdev;
	struct dp_soc *soc_t = (struct dp_soc *)soc;
	txrx_pdev = dp_get_pdev_for_mac_id(soc_t, pdev_id);

	if (!event) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid WDI event in %s\n", __func__);
		return;
	}
	if (!txrx_pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid pdev in WDI event handler\n");
		return;
	}

	/*
	 *  There can be NULL data, so no validation for the data
	 *  Subscribers must do the sanity based on the requirements
	 */
	event_index = event - WDI_EVENT_BASE;
	if (!(txrx_pdev->wdi_event_list[event_index]) &&
		(event == WDI_EVENT_RX_DESC)) {
		/* WDI_EVEN_RX_DESC is indicated for RX_LITE also */
		event_index = WDI_EVENT_LITE_RX - WDI_EVENT_BASE;
	}
	wdi_sub = txrx_pdev->wdi_event_list[event_index];

	/* Find the subscriber */
	dp_wdi_event_iter_sub(txrx_pdev, event_index, wdi_sub, data,
			peer_id, status);
}


/*
 * dp_wdi_event_sub() - Subscribe WDI event
 * @txrx_pdev_handle: cdp_pdev handle
 * @event_cb_sub_handle: subcribe evnet handle
 * @event: Event to be subscribe
 *
 * Return: 0 for success. nonzero for failure.
 */
int
dp_wdi_event_sub(
	struct cdp_pdev *txrx_pdev_handle,
	void *event_cb_sub_handle,
	uint32_t event)
{
	uint32_t event_index;
	wdi_event_subscribe *wdi_sub;
	struct dp_pdev *txrx_pdev = (struct dp_pdev *)txrx_pdev_handle;
	wdi_event_subscribe *event_cb_sub =
		(wdi_event_subscribe *) event_cb_sub_handle;

	if (!txrx_pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid txrx_pdev in %s", __func__);
		return -EINVAL;
	}
	if (!event_cb_sub) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid callback in %s", __func__);
		return -EINVAL;
	}
	if ((!event) || (event >= WDI_EVENT_LAST) || (event < WDI_EVENT_BASE)) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid event in %s", __func__);
		return -EINVAL;
	}
	dp_set_pktlog_wifi3(txrx_pdev, event, true);
	event_index = event - WDI_EVENT_BASE;
	wdi_sub = txrx_pdev->wdi_event_list[event_index];

	/*
	 *  Check if it is the first subscriber of the event
	 */
	if (!wdi_sub) {
		wdi_sub = event_cb_sub;
		wdi_sub->priv.next = NULL;
		wdi_sub->priv.prev = NULL;
		txrx_pdev->wdi_event_list[event_index] = wdi_sub;
		return 0;
	}
	event_cb_sub->priv.next = wdi_sub;
	event_cb_sub->priv.prev = NULL;
	wdi_sub->priv.prev = event_cb_sub;
	txrx_pdev->wdi_event_list[event_index] = event_cb_sub;
	return 0;

}

/*
 * dp_wdi_event_unsub() - WDI event unsubscribe
 * @txrx_pdev_handle: cdp_pdev handle
 * @event_cb_sub_handle: subscribed event handle
 * @event: Event to be unsubscribe
 *
 *
 * Return: 0 for success. nonzero for failure.
 */
int
dp_wdi_event_unsub(
	struct cdp_pdev *txrx_pdev_handle,
	void *event_cb_sub_handle,
	uint32_t event)
{
	uint32_t event_index = event - WDI_EVENT_BASE;
	struct dp_pdev *txrx_pdev = (struct dp_pdev *)txrx_pdev_handle;
	wdi_event_subscribe *event_cb_sub =
		(wdi_event_subscribe *) event_cb_sub_handle;

	if (!event_cb_sub) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid callback in %s", __func__);
		return -EINVAL;
	}

	dp_set_pktlog_wifi3(txrx_pdev, event, false);

	if (!event_cb_sub->priv.prev) {
		txrx_pdev->wdi_event_list[event_index] = event_cb_sub->priv.next;
	} else {
		event_cb_sub->priv.prev->priv.next = event_cb_sub->priv.next;
	}
	if (event_cb_sub->priv.next) {
		event_cb_sub->priv.next->priv.prev = event_cb_sub->priv.prev;
	}

	return 0;
}


/*
 * dp_wdi_event_attach() - Attach wdi event
 * @txrx_pdev: DP pdev handle
 *
 * Return: 0 for success. nonzero for failure.
 */
int
dp_wdi_event_attach(struct dp_pdev *txrx_pdev)
{
	if (!txrx_pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid device in %s\nWDI event attach failed\n",
			__func__);
		return -EINVAL;
	}
	/* Separate subscriber list for each event */
	txrx_pdev->wdi_event_list = (wdi_event_subscribe **)
		qdf_mem_malloc(
			sizeof(wdi_event_subscribe *) * WDI_NUM_EVENTS);
	if (!txrx_pdev->wdi_event_list) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Insufficient memory for the WDI event lists\n");
		return -EINVAL;
	}
	return 0;
}


/*
 * dp_wdi_event_detach() - Detach WDI event
 * @txrx_pdev: DP pdev handle
 *
 * Return: 0 for success. nonzero for failure.
 */
int
dp_wdi_event_detach(struct dp_pdev *txrx_pdev)
{
	int i;
	wdi_event_subscribe *wdi_sub;
	if (!txrx_pdev) {
		QDF_TRACE(QDF_MODULE_ID_DP, QDF_TRACE_LEVEL_ERROR,
			"Invalid device in %s\nWDI attach failed", __func__);
		return -EINVAL;
	}
	if (!txrx_pdev->wdi_event_list) {
		return -EINVAL;
	}
	for (i = 0; i < WDI_NUM_EVENTS; i++) {
		wdi_sub = txrx_pdev->wdi_event_list[i];
		/* Delete all the subscribers */
		dp_wdi_event_del_subs(wdi_sub, i);
	}
	if (txrx_pdev->wdi_event_list) {
		qdf_mem_free(txrx_pdev->wdi_event_list);
	}
	return 0;
}
#endif /* CONFIG_WIN */
