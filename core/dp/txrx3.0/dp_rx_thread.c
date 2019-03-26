/*
 * Copyright (c) 2014-2019 The Linux Foundation. All rights reserved.
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

#include <dp_txrx.h>
#include <cdp_txrx_cmn_struct.h>
#include <cdp_txrx_peer_ops.h>
#include <cds_sched.h>

/* Timeout in ms to wait for a DP rx thread */
#define DP_RX_THREAD_WAIT_TIMEOUT 200

#define DP_RX_TM_DEBUG 0
#if DP_RX_TM_DEBUG
/**
 * dp_rx_tm_walk_skb_list() - Walk skb list and print members
 * @nbuf_list - nbuf list to print
 *
 * Returns: None
 */
static inline void dp_rx_tm_walk_skb_list(qdf_nbuf_t nbuf_list)
{
	qdf_nbuf_t nbuf;
	int i = 0;

	nbuf = nbuf_list;
	while (nbuf) {
		dp_debug("%d nbuf:%pk nbuf->next:%pK nbuf->data:%pk ", i,
			 nbuf, qdf_nbuf_next(nbuf), qdf_nbuf_data(nbuf));
		nbuf = qdf_nbuf_next(nbuf);
		i++;
	}
}
#else
static inline void dp_rx_tm_walk_skb_list(qdf_nbuf_t nbuf_list)
{ }
#endif /* DP_RX_TM_DEBUG */

/**
 * dp_rx_tm_get_soc_handle() - get soc handle from struct dp_rx_tm_handle_cmn
 * @rx_tm_handle_cmn - rx thread manager cmn handle
 *
 * Returns: ol_txrx_soc_handle on success, NULL on failure.
 */
static inline
ol_txrx_soc_handle dp_rx_tm_get_soc_handle(struct dp_rx_tm_handle_cmn *rx_tm_handle_cmn)
{
	struct dp_txrx_handle_cmn *txrx_handle_cmn;
	ol_txrx_soc_handle soc;

	txrx_handle_cmn =
		dp_rx_thread_get_txrx_handle(rx_tm_handle_cmn);

	soc = dp_txrx_get_soc_from_ext_handle(txrx_handle_cmn);
	return soc;
}

/**
 * dp_rx_tm_thread_dump_stats() - display stats for a rx_thread
 * @rx_thread - rx_thread pointer for which the stats need to be
 *            displayed
 *
 * Returns: None
 */
static void dp_rx_tm_thread_dump_stats(struct dp_rx_thread *rx_thread)
{
	uint8_t reo_ring_num;
	uint32_t off = 0;
	char nbuf_queued_string[100];
	uint32_t total_queued = 0;
	uint32_t temp = 0;

	qdf_mem_zero(nbuf_queued_string, sizeof(nbuf_queued_string));

	for (reo_ring_num = 0; reo_ring_num < DP_RX_TM_MAX_REO_RINGS;
	     reo_ring_num++) {
		temp = rx_thread->stats.nbuf_queued[reo_ring_num];
		if (!temp)
			continue;
		total_queued += temp;
		if (off >= sizeof(nbuf_queued_string))
			continue;
		off += qdf_scnprintf(&nbuf_queued_string[off],
				     sizeof(nbuf_queued_string) - off,
				     "reo[%u]:%u ", reo_ring_num, temp);
	}

	if (!total_queued)
		return;

	dp_info("thread:%u - qlen:%u queued:(total:%u %s) dequeued:%u stack:%u max_len:%u invalid(peer:%u vdev:%u rx-handle:%u others:%u)",
		rx_thread->id,
		qdf_nbuf_queue_head_qlen(&rx_thread->nbuf_queue),
		total_queued,
		nbuf_queued_string,
		rx_thread->stats.nbuf_dequeued,
		rx_thread->stats.nbuf_sent_to_stack,
		rx_thread->stats.nbufq_max_len,
		rx_thread->stats.dropped_invalid_peer,
		rx_thread->stats.dropped_invalid_vdev,
		rx_thread->stats.dropped_invalid_os_rx_handles,
		rx_thread->stats.dropped_others);
}

QDF_STATUS dp_rx_tm_dump_stats(struct dp_rx_tm_handle *rx_tm_hdl)
{
	int i;

	for (i = 0; i < rx_tm_hdl->num_dp_rx_threads; i++) {
		if (!rx_tm_hdl->rx_thread[i])
			continue;
		dp_rx_tm_thread_dump_stats(rx_tm_hdl->rx_thread[i]);
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_tm_thread_enqueue() - enqueue nbuf list into rx_thread
 * @rx_thread - rx_thread in which the nbuf needs to be queued
 * @nbuf_list - list of packets to be queued into the thread
 *
 * Enqueue packet into rx_thread and wake it up. The function
 * moves the next pointer of the nbuf_list into the ext list of
 * the first nbuf for storage into the thread. Only the first
 * nbuf is queued into the thread nbuf queue. The reverse is
 * done at the time of dequeue.
 *
 * Returns: QDF_STATUS_SUCCESS on success or qdf error code on
 * failure
 */
static QDF_STATUS dp_rx_tm_thread_enqueue(struct dp_rx_thread *rx_thread,
					  qdf_nbuf_t nbuf_list)
{
	qdf_nbuf_t head_ptr, next_ptr_list;
	uint32_t temp_qlen;
	uint32_t num_elements_in_nbuf;
	struct dp_rx_tm_handle_cmn *tm_handle_cmn;
	uint8_t reo_ring_num = QDF_NBUF_CB_RX_CTX_ID(nbuf_list);
	qdf_wait_queue_head_t *wait_q_ptr;

	tm_handle_cmn = rx_thread->rtm_handle_cmn;

	if (!tm_handle_cmn) {
		dp_alert("tm_handle_cmn is null!");
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}

	wait_q_ptr = dp_rx_thread_get_wait_queue(tm_handle_cmn);

	if (reo_ring_num >= DP_RX_TM_MAX_REO_RINGS) {
		dp_alert("incorrect ring %u", reo_ring_num);
		QDF_BUG(0);
		return QDF_STATUS_E_FAILURE;
	}

	num_elements_in_nbuf = QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(nbuf_list);

	dp_rx_tm_walk_skb_list(nbuf_list);

	head_ptr = nbuf_list;

	/* Ensure head doesn't have an ext list */
	while (qdf_unlikely(head_ptr && qdf_nbuf_get_ext_list(head_ptr))) {
		QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(head_ptr) = 1;
		num_elements_in_nbuf--;
		next_ptr_list = head_ptr->next;
		qdf_nbuf_set_next(head_ptr, NULL);
		qdf_nbuf_queue_head_enqueue_tail(&rx_thread->nbuf_queue,
						 head_ptr);
		head_ptr = next_ptr_list;
	}

	if (!head_ptr)
		goto enq_done;

	next_ptr_list = head_ptr->next;

	if (next_ptr_list) {
		/* move ->next pointer to ext list */
		qdf_nbuf_append_ext_list(head_ptr, next_ptr_list, 0);
		dp_debug("appended next_ptr_list %pK to nbuf %pK ext list %pK",
			 qdf_nbuf_next(nbuf_list), nbuf_list,
			 qdf_nbuf_get_ext_list(nbuf_list));
	}
	qdf_nbuf_set_next(head_ptr, NULL);

	qdf_nbuf_queue_head_enqueue_tail(&rx_thread->nbuf_queue, head_ptr);

enq_done:
	temp_qlen = qdf_nbuf_queue_head_qlen(&rx_thread->nbuf_queue);

	rx_thread->stats.nbuf_queued[reo_ring_num] += num_elements_in_nbuf;

	if (temp_qlen > rx_thread->stats.nbufq_max_len)
		rx_thread->stats.nbufq_max_len = temp_qlen;

	qdf_set_bit(RX_POST_EVENT, &rx_thread->event_flag);
	qdf_wake_up_interruptible(wait_q_ptr);

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_tm_thread_dequeue() - dequeue nbuf list from rx_thread
 * @rx_thread - rx_thread from which the nbuf needs to be dequeued
 *
 * Returns: nbuf or nbuf_list dequeued from rx_thread
 */
static qdf_nbuf_t dp_rx_tm_thread_dequeue(struct dp_rx_thread *rx_thread)
{
	qdf_nbuf_t head, next_ptr_list, nbuf_list;

	head = qdf_nbuf_queue_head_dequeue(&rx_thread->nbuf_queue);
	nbuf_list = head;
	if (head && QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(head) > 1) {
		/* move ext list to ->next pointer */
		next_ptr_list = qdf_nbuf_get_ext_list(head);
		qdf_nbuf_append_ext_list(head, NULL, 0);
		qdf_nbuf_set_next(nbuf_list, next_ptr_list);
		dp_rx_tm_walk_skb_list(nbuf_list);
	}
	return nbuf_list;
}

/**
 * dp_rx_thread_process_nbufq() - process nbuf queue of a thread
 * @rx_thread - rx_thread whose nbuf queue needs to be processed
 *
 * Returns: 0 on success, error code on failure
 */
static int dp_rx_thread_process_nbufq(struct dp_rx_thread *rx_thread)
{
	qdf_nbuf_t nbuf_list;
	uint32_t peer_local_id;
	void *peer;
	struct cdp_vdev *vdev;
	ol_txrx_rx_fp stack_fn;
	ol_osif_vdev_handle osif_vdev;
	ol_txrx_soc_handle soc;
	uint32_t num_list_elements = 0;
	struct cdp_pdev *pdev;

	struct dp_txrx_handle_cmn *txrx_handle_cmn;

	txrx_handle_cmn =
		dp_rx_thread_get_txrx_handle(rx_thread->rtm_handle_cmn);

	soc = dp_txrx_get_soc_from_ext_handle(txrx_handle_cmn);
	pdev = dp_txrx_get_pdev_from_ext_handle(txrx_handle_cmn);

	if (!soc || !pdev) {
		dp_err("invalid soc or pdev!");
		QDF_BUG(0);
		return -EFAULT;
	}

	nbuf_list = dp_rx_tm_thread_dequeue(rx_thread);
	while (nbuf_list) {
		num_list_elements =
			QDF_NBUF_CB_RX_NUM_ELEMENTS_IN_LIST(nbuf_list);
		rx_thread->stats.nbuf_dequeued += num_list_elements;

		peer_local_id = QDF_NBUF_CB_RX_PEER_LOCAL_ID(nbuf_list);
		peer = cdp_peer_find_by_local_id(soc, pdev, peer_local_id);

		if (!peer) {
			rx_thread->stats.dropped_invalid_peer +=
							num_list_elements;
			dp_err("peer not found for local_id %u!",
			       peer_local_id);
			qdf_nbuf_list_free(nbuf_list);
			goto dequeue_rx_thread;
		}

		vdev = cdp_peer_get_vdev(soc, peer);
		if (!vdev) {
			rx_thread->stats.dropped_invalid_vdev +=
							num_list_elements;
			dp_err("vdev not found for local_id %u!, pkt dropped",
			       peer_local_id);
			qdf_nbuf_list_free(nbuf_list);
			goto dequeue_rx_thread;
		}

		cdp_get_os_rx_handles_from_vdev(soc, vdev, &stack_fn,
						&osif_vdev);
		if (!stack_fn || !osif_vdev) {
			rx_thread->stats.dropped_invalid_os_rx_handles +=
							num_list_elements;
			qdf_nbuf_list_free(nbuf_list);
			goto dequeue_rx_thread;
		}
		stack_fn(osif_vdev, nbuf_list);
		rx_thread->stats.nbuf_sent_to_stack += num_list_elements;

dequeue_rx_thread:
		nbuf_list = dp_rx_tm_thread_dequeue(rx_thread);
	}

	return 0;
}

/**
 * dp_rx_thread_sub_loop() - rx thread subloop
 * @rx_thread - rx_thread to be processed
 * @shutdown - pointer to shutdown variable
 *
 * The function handles shutdown and suspend events from other
 * threads and processes nbuf queue of a rx thread. In case a
 * shutdown event is received from some other wlan thread, the
 * function sets the shutdown pointer to true and returns
 *
 * Returns: 0 on success, error code on failure
 */
static int dp_rx_thread_sub_loop(struct dp_rx_thread *rx_thread, bool *shutdown)
{
	while (true) {
		if (qdf_atomic_test_and_clear_bit(RX_SHUTDOWN_EVENT,
						  &rx_thread->event_flag)) {
			if (qdf_atomic_test_and_clear_bit(RX_SUSPEND_EVENT,
							  &rx_thread->event_flag)) {
				qdf_event_set(&rx_thread->suspend_event);
			}
			dp_debug("shutting down (%s) id %d pid %d",
				 qdf_get_current_comm(), rx_thread->id,
				 qdf_get_current_pid());
			*shutdown = true;
			break;
		}

		dp_rx_thread_process_nbufq(rx_thread);

		if (qdf_atomic_test_and_clear_bit(RX_SUSPEND_EVENT,
						  &rx_thread->event_flag)) {
			dp_debug("received suspend ind (%s) id %d pid %d",
				 qdf_get_current_comm(), rx_thread->id,
				 qdf_get_current_pid());
			qdf_event_reset(&rx_thread->resume_event);
			qdf_event_set(&rx_thread->suspend_event);
			dp_debug("waiting for resume (%s) id %d pid %d",
				 qdf_get_current_comm(), rx_thread->id,
				 qdf_get_current_pid());
			qdf_wait_single_event(&rx_thread->resume_event, 0);
		}
		break;
	}
	return 0;
}

/**
 * dp_rx_thread_loop() - main dp rx thread loop
 * @arg: pointer to dp_rx_thread structure for the rx thread
 *
 * Return: thread exit code
 */
static int dp_rx_thread_loop(void *arg)
{
	struct dp_rx_thread *rx_thread = arg;
	bool shutdown = false;
	int status;
	struct dp_rx_tm_handle_cmn *tm_handle_cmn;

	tm_handle_cmn = rx_thread->rtm_handle_cmn;

	if (!arg) {
		dp_err("bad Args passed");
		return 0;
	}

	qdf_set_user_nice(qdf_get_current_task(), -1);
	qdf_set_wake_up_idle(true);

	qdf_event_set(&rx_thread->start_event);
	dp_info("starting rx_thread (%s) id %d pid %d", qdf_get_current_comm(),
		rx_thread->id, qdf_get_current_pid());
	while (!shutdown) {
		/* This implements the execution model algorithm */
		dp_debug("sleeping");
		status =
		    qdf_wait_queue_interruptible
				(DP_RX_THREAD_GET_WAIT_QUEUE_OBJ(tm_handle_cmn),
				 qdf_atomic_test_bit(RX_POST_EVENT,
						     &rx_thread->event_flag) ||
				 qdf_atomic_test_bit(RX_SUSPEND_EVENT,
						     &rx_thread->event_flag));
		dp_debug("woken up");

		if (status == -ERESTARTSYS) {
			QDF_DEBUG_PANIC("wait_event_interruptible returned -ERESTARTSYS");
			break;
		}
		qdf_atomic_clear_bit(RX_POST_EVENT, &rx_thread->event_flag);
		dp_rx_thread_sub_loop(rx_thread, &shutdown);
	}

	/* If we get here the scheduler thread must exit */
	dp_info("exiting (%s) id %d pid %d", qdf_get_current_comm(),
		rx_thread->id, qdf_get_current_pid());
	qdf_event_set(&rx_thread->shutdown_event);
	qdf_exit_thread(QDF_STATUS_SUCCESS);

	return 0;
}

/**
 * dp_rx_tm_thread_napi_poll() - dummy napi poll for rx_thread NAPI
 * @napi: pointer to DP rx_thread NAPI
 * @budget: NAPI BUDGET
 *
 * Return: 0 as it is not supposed to be polled at all as it is not scheduled.
 */
static int dp_rx_tm_thread_napi_poll(struct napi_struct *napi, int budget)
{
	QDF_DEBUG_PANIC("this napi_poll should not be polled as we don't schedule it");

	return 0;
}

/**
 * dp_rx_tm_thread_napi_init() - Initialize dummy rx_thread NAPI
 * @rx_thread: dp_rx_thread structure containing dummy napi and netdev
 *
 * Return: None
 */
static void dp_rx_tm_thread_napi_init(struct dp_rx_thread *rx_thread)
{
	/* Todo - optimize to use only one dummy netdev for all thread napis */
	init_dummy_netdev(&rx_thread->netdev);
	netif_napi_add(&rx_thread->netdev, &rx_thread->napi,
		       dp_rx_tm_thread_napi_poll, 64);
	napi_enable(&rx_thread->napi);
}

/**
 * dp_rx_tm_thread_napi_deinit() - De-initialize dummy rx_thread NAPI
 * @rx_thread: dp_rx_thread handle containing dummy napi and netdev
 *
 * Return: None
 */
static void dp_rx_tm_thread_napi_deinit(struct dp_rx_thread *rx_thread)
{
	netif_napi_del(&rx_thread->napi);
}

/*
 * dp_rx_tm_thread_init() - Initialize dp_rx_thread structure and thread
 *
 * @rx_thread: dp_rx_thread structure to be initialized
 * @id: id of the thread to be initialized
 *
 * Return: QDF_STATUS on success, QDF error code on failure
 */
static QDF_STATUS dp_rx_tm_thread_init(struct dp_rx_thread *rx_thread,
				       uint8_t id)
{
	char thread_name[15];
	QDF_STATUS qdf_status;

	qdf_mem_zero(thread_name, sizeof(thread_name));

	if (!rx_thread) {
		dp_err("rx_thread is null!");
		return QDF_STATUS_E_FAULT;
	}
	rx_thread->id = id;
	rx_thread->event_flag = 0;
	qdf_nbuf_queue_head_init(&rx_thread->nbuf_queue);
	qdf_event_create(&rx_thread->start_event);
	qdf_event_create(&rx_thread->suspend_event);
	qdf_event_create(&rx_thread->resume_event);
	qdf_event_create(&rx_thread->shutdown_event);
	qdf_scnprintf(thread_name, sizeof(thread_name), "dp_rx_thread_%u", id);
	dp_info("%s %u", thread_name, id);

	if (cdp_cfg_get(dp_rx_tm_get_soc_handle(rx_thread->rtm_handle_cmn),
			cfg_dp_gro_enable))
		dp_rx_tm_thread_napi_init(rx_thread);

	rx_thread->task = qdf_create_thread(dp_rx_thread_loop,
					    rx_thread, thread_name);
	if (!rx_thread->task) {
		dp_err("could not create dp_rx_thread %d", id);
		return QDF_STATUS_E_FAILURE;
	}

	qdf_wake_up_process(rx_thread->task);
	qdf_status = qdf_wait_single_event(&rx_thread->start_event, 0);

	if (!QDF_IS_STATUS_SUCCESS(qdf_status)) {
		dp_err("failed waiting for thread creation id %d", id);
		return QDF_STATUS_E_FAILURE;
	}
	return QDF_STATUS_SUCCESS;
}

/*
 * dp_rx_tm_thread_deinit() - De-Initialize dp_rx_thread structure and thread
 * @rx_thread: dp_rx_thread structure to be de-initialized
 * @id: id of the thread to be initialized
 *
 * Return: QDF_STATUS_SUCCESS
 */
static QDF_STATUS dp_rx_tm_thread_deinit(struct dp_rx_thread *rx_thread)
{
	qdf_event_destroy(&rx_thread->start_event);
	qdf_event_destroy(&rx_thread->suspend_event);
	qdf_event_destroy(&rx_thread->resume_event);
	qdf_event_destroy(&rx_thread->shutdown_event);

	if (cdp_cfg_get(dp_rx_tm_get_soc_handle(rx_thread->rtm_handle_cmn),
			cfg_dp_gro_enable))
		dp_rx_tm_thread_napi_deinit(rx_thread);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dp_rx_tm_init(struct dp_rx_tm_handle *rx_tm_hdl,
			 uint8_t num_dp_rx_threads)
{
	int i;
	QDF_STATUS qdf_status = QDF_STATUS_SUCCESS;

	if (num_dp_rx_threads > DP_MAX_RX_THREADS) {
		dp_err("unable to initialize %u number of threads. MAX %u",
		       num_dp_rx_threads, DP_MAX_RX_THREADS);
		return QDF_STATUS_E_INVAL;
	}

	rx_tm_hdl->num_dp_rx_threads = num_dp_rx_threads;

	dp_info("initializing %u threads", num_dp_rx_threads);

	/* allocate an array to contain the DP RX thread pointers */
	rx_tm_hdl->rx_thread = qdf_mem_malloc(num_dp_rx_threads *
					      sizeof(struct dp_rx_thread *));

	if (qdf_unlikely(!rx_tm_hdl->rx_thread)) {
		qdf_status = QDF_STATUS_E_NOMEM;
		goto ret;
	}

	qdf_init_waitqueue_head(&rx_tm_hdl->wait_q);

	for (i = 0; i < rx_tm_hdl->num_dp_rx_threads; i++) {
		rx_tm_hdl->rx_thread[i] =
			(struct dp_rx_thread *)
			qdf_mem_malloc(sizeof(struct dp_rx_thread));
		if (qdf_unlikely(!rx_tm_hdl->rx_thread[i])) {
			QDF_ASSERT(0);
			qdf_status = QDF_STATUS_E_NOMEM;
			goto ret;
		}
		rx_tm_hdl->rx_thread[i]->rtm_handle_cmn =
				(struct dp_rx_tm_handle_cmn *)rx_tm_hdl;
		qdf_status =
			dp_rx_tm_thread_init(rx_tm_hdl->rx_thread[i], i);
		if (!QDF_IS_STATUS_SUCCESS(qdf_status))
			break;
	}
ret:
	if (!QDF_IS_STATUS_SUCCESS(qdf_status))
		dp_rx_tm_deinit(rx_tm_hdl);

	return qdf_status;
}

/**
 * dp_rx_tm_resume() - suspend DP RX threads
 * @rx_tm_hdl: dp_rx_tm_handle containing the overall thread
 *            infrastructure
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS dp_rx_tm_suspend(struct dp_rx_tm_handle *rx_tm_hdl)
{
	int i;
	QDF_STATUS qdf_status;
	struct dp_rx_thread *rx_thread;

	for (i = 0; i < rx_tm_hdl->num_dp_rx_threads; i++) {
		if (!rx_tm_hdl->rx_thread[i])
			continue;
		qdf_set_bit(RX_SUSPEND_EVENT,
			    &rx_tm_hdl->rx_thread[i]->event_flag);
	}

	qdf_wake_up_interruptible(&rx_tm_hdl->wait_q);

	for (i = 0; i < rx_tm_hdl->num_dp_rx_threads; i++) {
		rx_thread = rx_tm_hdl->rx_thread[i];
		if (!rx_thread)
			continue;
		dp_debug("thread %d", i);
		qdf_status = qdf_wait_single_event(&rx_thread->suspend_event,
						   DP_RX_THREAD_WAIT_TIMEOUT);
		if (QDF_IS_STATUS_SUCCESS(qdf_status))
			dp_debug("thread:%d suspended", rx_thread->id);
		else if (qdf_status == QDF_STATUS_E_TIMEOUT)
			dp_err("thread:%d timed out waiting for suspend",
			       rx_thread->id);
		else
			dp_err("thread:%d failed while waiting for suspend",
			       rx_thread->id);
	}
	rx_tm_hdl->state = DP_RX_THREAD_SUSPENDED;

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_tm_resume() - resume DP RX threads
 * @rx_tm_hdl: dp_rx_tm_handle containing the overall thread
 *            infrastructure
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS dp_rx_tm_resume(struct dp_rx_tm_handle *rx_tm_hdl)
{
	int i;

	if (rx_tm_hdl->state != DP_RX_THREAD_SUSPENDED) {
		dp_err("resume callback received without suspend");
		return QDF_STATUS_E_FAULT;
	}

	for (i = 0; i < rx_tm_hdl->num_dp_rx_threads; i++) {
		if (!rx_tm_hdl->rx_thread[i])
			continue;
		dp_debug("calling thread %d to resume", i);
		qdf_event_set(&rx_tm_hdl->rx_thread[i]->resume_event);
	}

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_tm_shutdown() - shutdown all DP RX threads
 * @rx_tm_hdl: dp_rx_tm_handle containing the overall thread infrastructure
 *
 * Return: QDF_STATUS_SUCCESS
 */
static QDF_STATUS dp_rx_tm_shutdown(struct dp_rx_tm_handle *rx_tm_hdl)
{
	int i;

	for (i = 0; i < rx_tm_hdl->num_dp_rx_threads; i++) {
		if (!rx_tm_hdl->rx_thread[i])
			continue;
		qdf_set_bit(RX_SHUTDOWN_EVENT,
			    &rx_tm_hdl->rx_thread[i]->event_flag);
		qdf_set_bit(RX_POST_EVENT,
			    &rx_tm_hdl->rx_thread[i]->event_flag);
	}

	qdf_wake_up_interruptible(&rx_tm_hdl->wait_q);

	for (i = 0; i < rx_tm_hdl->num_dp_rx_threads; i++) {
		if (!rx_tm_hdl->rx_thread[i])
			continue;
		dp_debug("waiting for shutdown of thread %d", i);
		qdf_wait_single_event(&rx_tm_hdl->rx_thread[i]->shutdown_event,
				      0);
	}
	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_tm_deinit() - de-initialize RX thread infrastructure
 * @rx_tm_hdl: dp_rx_tm_handle containing the overall thread
 *            infrastructure
 *
 * Return: QDF_STATUS_SUCCESS
 */
QDF_STATUS dp_rx_tm_deinit(struct dp_rx_tm_handle *rx_tm_hdl)
{
	int i = 0;
	if (!rx_tm_hdl->rx_thread) {
		dp_err("rx_tm_hdl->rx_thread not initialized!");
		return QDF_STATUS_SUCCESS;
	}

	dp_rx_tm_shutdown(rx_tm_hdl);

	for (i = 0; i < rx_tm_hdl->num_dp_rx_threads; i++) {
		if (!rx_tm_hdl->rx_thread[i])
			continue;
		dp_rx_tm_thread_deinit(rx_tm_hdl->rx_thread[i]);
		qdf_mem_free(rx_tm_hdl->rx_thread[i]);
	}

	/* free the array of RX thread pointers*/
	qdf_mem_free(rx_tm_hdl->rx_thread);
	rx_tm_hdl->rx_thread = NULL;

	return QDF_STATUS_SUCCESS;
}

/**
 * dp_rx_tm_select_thread() - select a DP RX thread for a nbuf
 * @rx_tm_hdl: dp_rx_tm_handle containing the overall thread
 *            infrastructure
 * @nbuf_list: list of nbufs to be enqueued in to the thread
 *
 * The function relies on the presence of QDF_NBUF_CB_RX_CTX_ID
 * in the nbuf list. Depending on the RX_CTX (copy engine or reo
 * ring) on which the packet was received, the function selects
 * a corresponding rx_thread.
 *
 * Return: rx thread ID selected for the nbuf
 */
static uint8_t dp_rx_tm_select_thread(struct dp_rx_tm_handle *rx_tm_hdl,
				      qdf_nbuf_t nbuf_list)
{
	uint8_t selected_rx_thread;
	uint8_t reo_ring_num = QDF_NBUF_CB_RX_CTX_ID(nbuf_list);

	if (reo_ring_num >= rx_tm_hdl->num_dp_rx_threads) {
		dp_err_rl("unexpected ring number");
		QDF_BUG(0);
		return 0;
	}

	selected_rx_thread = reo_ring_num;
	return selected_rx_thread;
}

QDF_STATUS dp_rx_tm_enqueue_pkt(struct dp_rx_tm_handle *rx_tm_hdl,
				qdf_nbuf_t nbuf_list)
{
	uint8_t selected_thread_id;

	selected_thread_id = dp_rx_tm_select_thread(rx_tm_hdl, nbuf_list);
	dp_rx_tm_thread_enqueue(rx_tm_hdl->rx_thread[selected_thread_id],
				nbuf_list);
	return QDF_STATUS_SUCCESS;
}

struct napi_struct *dp_rx_tm_get_napi_context(struct dp_rx_tm_handle *rx_tm_hdl,
					      uint8_t rx_ctx_id)
{
	if (rx_ctx_id >= rx_tm_hdl->num_dp_rx_threads) {
		dp_err_rl("unexpected rx_ctx_id %u", rx_ctx_id);
		QDF_BUG(0);
		return NULL;
	}

	return &rx_tm_hdl->rx_thread[rx_ctx_id]->napi;
}
