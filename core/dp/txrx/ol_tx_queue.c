/*
 * Copyright (c) 2012-2016 The Linux Foundation. All rights reserved.
 *
 * Previously licensed under the ISC license by Qualcomm Atheros, Inc.
 *
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

/*
 * This file was originally distributed by Qualcomm Atheros, Inc.
 * under proprietary terms before Copyright ownership was assigned
 * to the Linux Foundation.
 */

#include <cdf_nbuf.h>           /* cdf_nbuf_t, etc. */
#include <qdf_atomic.h>         /* qdf_atomic_read, etc. */
#include <ol_cfg.h>             /* ol_cfg_addba_retry */
#include <htt.h>                /* HTT_TX_EXT_TID_MGMT */
#include <ol_htt_tx_api.h>      /* htt_tx_desc_tid */
#include <ol_txrx_api.h>        /* ol_txrx_vdev_handle */
#include <ol_txrx_ctrl_api.h>   /* ol_txrx_sync, ol_tx_addba_conf */
#include <ol_ctrl_txrx_api.h>   /* ol_ctrl_addba_req */
#include <ol_txrx_internal.h>   /* TXRX_ASSERT1, etc. */
#include <ol_txrx_types.h>      /* pdev stats */
#include <ol_tx_desc.h>         /* ol_tx_desc, ol_tx_desc_frame_list_free */
#include <ol_tx.h>              /* ol_tx_vdev_ll_pause_queue_send */
#include <ol_tx_queue.h>
#include <ol_txrx_dbg.h>        /* ENABLE_TX_QUEUE_LOG */
#include <qdf_types.h>          /* bool */

#if defined(QCA_LL_LEGACY_TX_FLOW_CONTROL)

void ol_txrx_vdev_pause(ol_txrx_vdev_handle vdev, uint32_t reason)
{
	/* TO DO: log the queue pause */
	/* acquire the mutex lock, since we'll be modifying the queues */
	TX_SCHED_DEBUG_PRINT("Enter %s\n", __func__);

	qdf_spin_lock_bh(&vdev->ll_pause.mutex);
	vdev->ll_pause.paused_reason |= reason;
	vdev->ll_pause.q_pause_cnt++;
	vdev->ll_pause.is_q_paused = true;
	qdf_spin_unlock_bh(&vdev->ll_pause.mutex);

	DPTRACE(qdf_dp_trace(NULL, QDF_DP_TRACE_VDEV_PAUSE,
				NULL, 0));
	TX_SCHED_DEBUG_PRINT("Leave %s\n", __func__);
}

void ol_txrx_vdev_unpause(ol_txrx_vdev_handle vdev, uint32_t reason)
{
	/* TO DO: log the queue unpause */
	/* acquire the mutex lock, since we'll be modifying the queues */
	TX_SCHED_DEBUG_PRINT("Enter %s\n", __func__);

	qdf_spin_lock_bh(&vdev->ll_pause.mutex);
	if (vdev->ll_pause.paused_reason & reason) {
		vdev->ll_pause.paused_reason &= ~reason;
		if (!vdev->ll_pause.paused_reason) {
			vdev->ll_pause.is_q_paused = false;
			vdev->ll_pause.q_unpause_cnt++;
			qdf_spin_unlock_bh(&vdev->ll_pause.mutex);
			ol_tx_vdev_ll_pause_queue_send(vdev);
		} else {
			qdf_spin_unlock_bh(&vdev->ll_pause.mutex);
		}
	} else {
		qdf_spin_unlock_bh(&vdev->ll_pause.mutex);
	}
	DPTRACE(qdf_dp_trace(NULL, QDF_DP_TRACE_VDEV_UNPAUSE,
				NULL, 0));
	TX_SCHED_DEBUG_PRINT("Leave %s\n", __func__);
}

void ol_txrx_vdev_flush(ol_txrx_vdev_handle vdev)
{
	qdf_spin_lock_bh(&vdev->ll_pause.mutex);
	qdf_timer_stop(&vdev->ll_pause.timer);
	vdev->ll_pause.is_q_timer_on = false;
	while (vdev->ll_pause.txq.head) {
		cdf_nbuf_t next =
			cdf_nbuf_next(vdev->ll_pause.txq.head);
		cdf_nbuf_set_next(vdev->ll_pause.txq.head, NULL);
		cdf_nbuf_unmap(vdev->pdev->osdev,
			       vdev->ll_pause.txq.head,
			       QDF_DMA_TO_DEVICE);
		cdf_nbuf_tx_free(vdev->ll_pause.txq.head,
				 NBUF_PKT_ERROR);
		vdev->ll_pause.txq.head = next;
	}
	vdev->ll_pause.txq.tail = NULL;
	vdev->ll_pause.txq.depth = 0;
	qdf_spin_unlock_bh(&vdev->ll_pause.mutex);
}

#endif /* defined(QCA_LL_LEGACY_TX_FLOW_CONTROL) */

#ifdef QCA_LL_TX_FLOW_CONTROL_V2

/**
 * ol_txrx_map_to_netif_reason_type() - map to netif_reason_type
 * @reason: reason
 *
 * Return: netif_reason_type
 */
enum netif_reason_type
ol_txrx_map_to_netif_reason_type(uint32_t reason)
{
	switch (reason) {
	case OL_TXQ_PAUSE_REASON_FW:
		return WLAN_FW_PAUSE;
	case OL_TXQ_PAUSE_REASON_PEER_UNAUTHORIZED:
		return WLAN_PEER_UNAUTHORISED;
	case OL_TXQ_PAUSE_REASON_TX_ABORT:
		return WLAN_TX_ABORT;
	case OL_TXQ_PAUSE_REASON_VDEV_STOP:
		return WLAN_VDEV_STOP;
	case OL_TXQ_PAUSE_REASON_THERMAL_MITIGATION:
		return WLAN_THERMAL_MITIGATION;
	default:
		TXRX_PRINT(TXRX_PRINT_LEVEL_ERR,
			   "%s: reason not supported %d\n",
			   __func__, reason);
		return WLAN_REASON_TYPE_MAX;
	}
}

/**
 * ol_txrx_vdev_pause() - pause vdev network queues
 * @vdev: vdev handle
 * @reason: reason
 *
 * Return: none
 */
void ol_txrx_vdev_pause(ol_txrx_vdev_handle vdev, uint32_t reason)
{
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	enum netif_reason_type netif_reason;

	if (qdf_unlikely((!pdev) || (!pdev->pause_cb))) {
		TXRX_PRINT(TXRX_PRINT_LEVEL_ERR,
				   "%s: invalid pdev\n", __func__);
		return;
	}

	netif_reason = ol_txrx_map_to_netif_reason_type(reason);
	if (netif_reason == WLAN_REASON_TYPE_MAX)
		return;

	pdev->pause_cb(vdev->vdev_id, WLAN_NETIF_TX_DISABLE, netif_reason);
}

/**
 * ol_txrx_vdev_unpause() - unpause vdev network queues
 * @vdev: vdev handle
 * @reason: reason
 *
 * Return: none
 */
void ol_txrx_vdev_unpause(ol_txrx_vdev_handle vdev, uint32_t reason)
{
	struct ol_txrx_pdev_t *pdev = vdev->pdev;
	enum netif_reason_type netif_reason;

	if (qdf_unlikely((!pdev) || (!pdev->pause_cb))) {
		TXRX_PRINT(TXRX_PRINT_LEVEL_ERR,
				   "%s: invalid pdev\n", __func__);
		return;
	}

	netif_reason = ol_txrx_map_to_netif_reason_type(reason);
	if (netif_reason == WLAN_REASON_TYPE_MAX)
		return;

	pdev->pause_cb(vdev->vdev_id, WLAN_WAKE_ALL_NETIF_QUEUE,
			netif_reason);

}

/**
 * ol_txrx_pdev_pause() - pause network queues for each vdev
 * @pdev: pdev handle
 * @reason: reason
 *
 * Return: none
 */
void ol_txrx_pdev_pause(struct ol_txrx_pdev_t *pdev, uint32_t reason)
{
	struct ol_txrx_vdev_t *vdev = NULL, *tmp;

	TAILQ_FOREACH_SAFE(vdev, &pdev->vdev_list, vdev_list_elem, tmp) {
		ol_txrx_vdev_pause(vdev, reason);
	}

}

/**
 * ol_txrx_pdev_unpause() - unpause network queues for each vdev
 * @pdev: pdev handle
 * @reason: reason
 *
 * Return: none
 */
void ol_txrx_pdev_unpause(struct ol_txrx_pdev_t *pdev, uint32_t reason)
{
	struct ol_txrx_vdev_t *vdev = NULL, *tmp;

	TAILQ_FOREACH_SAFE(vdev, &pdev->vdev_list, vdev_list_elem, tmp) {
		ol_txrx_vdev_unpause(vdev, reason);
	}

}
#endif

/*--- LL tx throttle queue code --------------------------------------------*/
#if defined(QCA_SUPPORT_TX_THROTTLE)
uint8_t ol_tx_pdev_is_target_empty(void)
{
	/* TM TODO */
	return 1;
}

#ifdef QCA_LL_TX_FLOW_CONTROL_V2
/**
 * ol_txrx_thermal_pause() - pause due to thermal mitigation
 * @pdev: pdev handle
 *
 * Return: none
 */
static inline
void ol_txrx_thermal_pause(struct ol_txrx_pdev_t *pdev)
{
	ol_txrx_pdev_pause(pdev, OL_TXQ_PAUSE_REASON_THERMAL_MITIGATION);
	return;
}
/**
 * ol_txrx_thermal_unpause() - unpause due to thermal mitigation
 * @pdev: pdev handle
 *
 * Return: none
 */
static inline
void ol_txrx_thermal_unpause(struct ol_txrx_pdev_t *pdev)
{
	ol_txrx_pdev_unpause(pdev, OL_TXQ_PAUSE_REASON_THERMAL_MITIGATION);
	return;
}
#else
/**
 * ol_txrx_thermal_pause() - pause due to thermal mitigation
 * @pdev: pdev handle
 *
 * Return: none
 */
static inline
void ol_txrx_thermal_pause(struct ol_txrx_pdev_t *pdev)
{
	return;
}

/**
 * ol_txrx_thermal_unpause() - unpause due to thermal mitigation
 * @pdev: pdev handle
 *
 * Return: none
 */
static inline
void ol_txrx_thermal_unpause(struct ol_txrx_pdev_t *pdev)
{
	ol_tx_pdev_ll_pause_queue_send_all(pdev);
	return;
}
#endif

void ol_tx_pdev_throttle_phase_timer(void *context)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)context;
	int ms;
	enum throttle_level cur_level;
	enum throttle_phase cur_phase;

	/* update the phase */
	pdev->tx_throttle.current_throttle_phase++;

	if (pdev->tx_throttle.current_throttle_phase == THROTTLE_PHASE_MAX)
		pdev->tx_throttle.current_throttle_phase = THROTTLE_PHASE_OFF;

	if (pdev->tx_throttle.current_throttle_phase == THROTTLE_PHASE_OFF) {
		/* Traffic is stopped */
		TXRX_PRINT(TXRX_PRINT_LEVEL_WARN,
				   "throttle phase --> OFF\n");
		ol_txrx_thermal_pause(pdev);
		cur_level = pdev->tx_throttle.current_throttle_level;
		cur_phase = pdev->tx_throttle.current_throttle_phase;
		ms = pdev->tx_throttle.throttle_time_ms[cur_level][cur_phase];
		if (pdev->tx_throttle.current_throttle_level !=
				THROTTLE_LEVEL_0) {
			TXRX_PRINT(TXRX_PRINT_LEVEL_WARN,
					   "start timer %d ms\n", ms);
			qdf_timer_start(&pdev->tx_throttle.
							phase_timer, ms);
		}
	} else {
		/* Traffic can go */
		TXRX_PRINT(TXRX_PRINT_LEVEL_WARN,
					"throttle phase --> ON\n");
		ol_txrx_thermal_unpause(pdev);
		cur_level = pdev->tx_throttle.current_throttle_level;
		cur_phase = pdev->tx_throttle.current_throttle_phase;
		ms = pdev->tx_throttle.throttle_time_ms[cur_level][cur_phase];
		if (pdev->tx_throttle.current_throttle_level !=
		    THROTTLE_LEVEL_0) {
			TXRX_PRINT(TXRX_PRINT_LEVEL_WARN, "start timer %d ms\n",
				   ms);
			qdf_timer_start(&pdev->tx_throttle.phase_timer,
						ms);
		}
	}
}

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
void ol_tx_pdev_throttle_tx_timer(void *context)
{
	struct ol_txrx_pdev_t *pdev = (struct ol_txrx_pdev_t *)context;
	ol_tx_pdev_ll_pause_queue_send_all(pdev);
}
#endif

void ol_tx_throttle_set_level(struct ol_txrx_pdev_t *pdev, int level)
{
	int ms = 0;

	if (level >= THROTTLE_LEVEL_MAX) {
		TXRX_PRINT(TXRX_PRINT_LEVEL_WARN,
			   "%s invalid throttle level set %d, ignoring\n",
			   __func__, level);
		return;
	}

	TXRX_PRINT(TXRX_PRINT_LEVEL_ERR, "Setting throttle level %d\n", level);

	/* Set the current throttle level */
	pdev->tx_throttle.current_throttle_level = (enum throttle_level) level;

	/* Reset the phase */
	pdev->tx_throttle.current_throttle_phase = THROTTLE_PHASE_OFF;
	ol_txrx_thermal_unpause(pdev);

	/* Start with the new time */
	ms = pdev->tx_throttle.
	     throttle_time_ms[level][THROTTLE_PHASE_OFF];

	qdf_timer_stop(&pdev->tx_throttle.phase_timer);

	if (level != THROTTLE_LEVEL_0)
		qdf_timer_start(&pdev->tx_throttle.phase_timer, ms);
}

/* This table stores the duty cycle for each level.
   Example "on" time for level 2 with duty period 100ms is:
   "on" time = duty_period_ms >> throttle_duty_cycle_table[2]
   "on" time = 100 ms >> 2 = 25ms */
static uint8_t g_throttle_duty_cycle_table[THROTTLE_LEVEL_MAX] = { 0, 1, 2, 4 };

void ol_tx_throttle_init_period(struct ol_txrx_pdev_t *pdev, int period)
{
	int i;

	/* Set the current throttle level */
	pdev->tx_throttle.throttle_period_ms = period;

	TXRX_PRINT(TXRX_PRINT_LEVEL_WARN, "level  OFF  ON\n");
	for (i = 0; i < THROTTLE_LEVEL_MAX; i++) {
		pdev->tx_throttle.throttle_time_ms[i][THROTTLE_PHASE_ON] =
			pdev->tx_throttle.throttle_period_ms >>
			g_throttle_duty_cycle_table[i];
		pdev->tx_throttle.throttle_time_ms[i][THROTTLE_PHASE_OFF] =
			pdev->tx_throttle.throttle_period_ms -
			pdev->tx_throttle.throttle_time_ms[
				i][THROTTLE_PHASE_ON];
		TXRX_PRINT(TXRX_PRINT_LEVEL_WARN, "%d      %d    %d\n", i,
			   pdev->tx_throttle.
			   throttle_time_ms[i][THROTTLE_PHASE_OFF],
			   pdev->tx_throttle.
			   throttle_time_ms[i][THROTTLE_PHASE_ON]);
	}
}

void ol_tx_throttle_init(struct ol_txrx_pdev_t *pdev)
{
	uint32_t throttle_period;

	pdev->tx_throttle.current_throttle_level = THROTTLE_LEVEL_0;
	pdev->tx_throttle.current_throttle_phase = THROTTLE_PHASE_OFF;
	qdf_spinlock_create(&pdev->tx_throttle.mutex);

	throttle_period = ol_cfg_throttle_period_ms(pdev->ctrl_pdev);

	ol_tx_throttle_init_period(pdev, throttle_period);

	qdf_timer_init(pdev->osdev,
			       &pdev->tx_throttle.phase_timer,
			       ol_tx_pdev_throttle_phase_timer, pdev,
			       QDF_TIMER_TYPE_SW);

#ifdef QCA_LL_LEGACY_TX_FLOW_CONTROL
	qdf_timer_init(pdev->osdev,
			       &pdev->tx_throttle.tx_timer,
			       ol_tx_pdev_throttle_tx_timer, pdev,
			       QDF_TIMER_TYPE_SW);
#endif

	pdev->tx_throttle.tx_threshold = THROTTLE_TX_THRESHOLD;
}
#endif /* QCA_SUPPORT_TX_THROTTLE */
/*--- End of LL tx throttle queue code ---------------------------------------*/
