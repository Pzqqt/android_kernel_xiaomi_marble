/*
 * Copyright (c) 2017-2020 The Linux Foundation. All rights reserved.
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
 * DOC: This file contains main green ap function definitions
 */

#include "wlan_green_ap_main_i.h"

/*
 * wlan_green_ap_ant_ps_reset() - Reset function
 * @green_ap - green ap context
 *
 * Reset fiunction, so that Antenna Mask can come into effect.
 *                This applies for only few of the hardware chips
 *
 * Return: QDF_STATUS
 */
static QDF_STATUS wlan_green_ap_ant_ps_reset
		(struct wlan_pdev_green_ap_ctx *green_ap_ctx)
{
	struct wlan_lmac_if_green_ap_tx_ops *green_ap_tx_ops;
	struct wlan_objmgr_pdev *pdev;

	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	pdev = green_ap_ctx->pdev;

	green_ap_tx_ops = wlan_psoc_get_green_ap_tx_ops(green_ap_ctx);
	if (!green_ap_tx_ops) {
		green_ap_err("green ap tx ops obtained are NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!green_ap_tx_ops->reset_dev)
		return QDF_STATUS_SUCCESS;

	/*
	 * Add protection against green AP enabling interrupts
	 * when not valid or no VAPs exist
	 */
	if (wlan_util_is_vdev_active(pdev, WLAN_GREEN_AP_ID) ==
						QDF_STATUS_SUCCESS)
		green_ap_tx_ops->reset_dev(pdev);
	else
		green_ap_err("Green AP tried to enable IRQs when invalid");

	return QDF_STATUS_SUCCESS;
}

struct wlan_lmac_if_green_ap_tx_ops *
wlan_psoc_get_green_ap_tx_ops(struct wlan_pdev_green_ap_ctx *green_ap_ctx)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_objmgr_pdev *pdev = green_ap_ctx->pdev;
	struct wlan_lmac_if_tx_ops *tx_ops;

	if (!pdev) {
		green_ap_err("pdev context obtained is NULL");
		return NULL;
	}

	psoc = wlan_pdev_get_psoc(pdev);
	if (!psoc) {
		green_ap_err("pdev context obtained is NULL");
		return NULL;
	}

	tx_ops = wlan_psoc_get_lmac_if_txops(psoc);
	if (!tx_ops) {
		green_ap_err("tx_ops is NULL");
		return NULL;
	}

	return &tx_ops->green_ap_tx_ops;
}

bool wlan_is_egap_enabled(struct wlan_pdev_green_ap_ctx *green_ap_ctx)
{
	struct wlan_green_ap_egap_params *egap_params;

	if (!green_ap_ctx) {
		green_ap_err("green ap context passed is NULL");
		return QDF_STATUS_E_INVAL;
	}
	egap_params = &green_ap_ctx->egap_params;

	if (egap_params->fw_egap_support &&
	    egap_params->host_enable_egap &&
	    egap_params->egap_feature_flags)
		return true;
	return false;
}
qdf_export_symbol(wlan_is_egap_enabled);

/**
 * wlan_green_ap_ps_event_state_update() - Update PS state and event
 * @pdev: pdev pointer
 * @state: ps state
 * @event: ps event
 *
 * @Return: Success or Failure
 */
static QDF_STATUS wlan_green_ap_ps_event_state_update(
			struct wlan_pdev_green_ap_ctx *green_ap_ctx,
			enum wlan_green_ap_ps_state state,
			enum wlan_green_ap_ps_event event)
{
	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	green_ap_ctx->ps_state = state;
	green_ap_ctx->ps_event = event;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS wlan_green_ap_state_mc(struct wlan_pdev_green_ap_ctx *green_ap_ctx,
				  enum wlan_green_ap_ps_event event)
{
	struct wlan_lmac_if_green_ap_tx_ops *green_ap_tx_ops;
	uint8_t pdev_id;

	/*
	 * Remove the assignments once channel info is available for
	 * converged component.
	 */
	uint16_t channel = 1;
	uint32_t channel_flags = 1;

	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!green_ap_ctx->pdev) {
		green_ap_err("pdev obtained is NULL");
		return QDF_STATUS_E_FAILURE;
	}
	pdev_id = wlan_objmgr_pdev_get_pdev_id(green_ap_ctx->pdev);

	green_ap_tx_ops = wlan_psoc_get_green_ap_tx_ops(green_ap_ctx);
	if (!green_ap_tx_ops) {
		green_ap_err("green ap tx ops obtained are NULL");
		return QDF_STATUS_E_FAILURE;
	}

	if (!green_ap_tx_ops->ps_on_off_send) {
		green_ap_err("tx op for sending enbale/disable green ap is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	qdf_spin_lock_bh(&green_ap_ctx->lock);

	if (green_ap_tx_ops->get_current_channel)
		channel = green_ap_tx_ops->get_current_channel(
						green_ap_ctx->pdev);

	if (green_ap_tx_ops->get_current_channel_flags)
		channel_flags = green_ap_tx_ops->get_current_channel_flags(
							green_ap_ctx->pdev);

	/* handle the green ap ps event */
	switch (event) {
	case WLAN_GREEN_AP_ADD_STA_EVENT:
		green_ap_ctx->num_nodes++;
		break;

	case WLAN_GREEN_AP_DEL_STA_EVENT:
		if (green_ap_ctx->num_nodes)
			green_ap_ctx->num_nodes--;
		break;

	case WLAN_GREEN_AP_ADD_MULTISTREAM_STA_EVENT:
		green_ap_ctx->num_nodes_multistream++;
		break;

	case WLAN_GREEN_AP_DEL_MULTISTREAM_STA_EVENT:
		if (green_ap_ctx->num_nodes_multistream)
			green_ap_ctx->num_nodes_multistream--;
		break;

	case WLAN_GREEN_AP_PS_START_EVENT:
	case WLAN_GREEN_AP_PS_STOP_EVENT:
	case WLAN_GREEN_AP_PS_ON_EVENT:
	case WLAN_GREEN_AP_PS_WAIT_EVENT:
		break;

	default:
		green_ap_err("Invalid event: %d", event);
		break;
	}

	green_ap_debug("Green-AP event: %d, state: %d, num_nodes: %d",
		       event, green_ap_ctx->ps_state, green_ap_ctx->num_nodes);

	/* Confirm that power save is enabled before doing state transitions */
	if (!green_ap_ctx->ps_enable) {
		green_ap_debug("Green-AP is disabled");
		if (green_ap_ctx->ps_state == WLAN_GREEN_AP_PS_ON_STATE) {
			if (green_ap_tx_ops->ps_on_off_send(green_ap_ctx->pdev,
								false, pdev_id))
				green_ap_err("failed to set green ap mode");
			wlan_green_ap_ant_ps_reset(green_ap_ctx);
		}
		wlan_green_ap_ps_event_state_update(
				green_ap_ctx,
				WLAN_GREEN_AP_PS_IDLE_STATE,
				WLAN_GREEN_AP_PS_WAIT_EVENT);
		goto done;
	}

	/* handle the green ap ps state */
	switch (green_ap_ctx->ps_state) {
	case WLAN_GREEN_AP_PS_IDLE_STATE:
		if ((green_ap_ctx->num_nodes &&
		     green_ap_ctx->ps_mode == WLAN_GREEN_AP_MODE_NO_STA) ||
		    (green_ap_ctx->num_nodes_multistream &&
		     green_ap_ctx->ps_mode == WLAN_GREEN_AP_MODE_NUM_STREAM)) {
			/*
			 * Multistream nodes present, switchoff the power save
			 */
			green_ap_info("Transition to OFF from IDLE");
			wlan_green_ap_ps_event_state_update(
					green_ap_ctx,
					WLAN_GREEN_AP_PS_OFF_STATE,
					WLAN_GREEN_AP_PS_WAIT_EVENT);
		} else {
			/* No Active nodes, get into power save */
			green_ap_info("Transition to WAIT from IDLE");
			wlan_green_ap_ps_event_state_update(
					green_ap_ctx,
					WLAN_GREEN_AP_PS_WAIT_STATE,
					WLAN_GREEN_AP_PS_WAIT_EVENT);
			qdf_timer_start(&green_ap_ctx->ps_timer,
					green_ap_ctx->ps_trans_time * 1000);
		}
		break;

	case WLAN_GREEN_AP_PS_OFF_STATE:
		if ((!green_ap_ctx->num_nodes &&
		     green_ap_ctx->ps_mode == WLAN_GREEN_AP_MODE_NO_STA) ||
		    (!green_ap_ctx->num_nodes_multistream &&
		     green_ap_ctx->ps_mode == WLAN_GREEN_AP_MODE_NUM_STREAM)) {
			green_ap_info("Transition to WAIT from OFF");
			wlan_green_ap_ps_event_state_update(
						green_ap_ctx,
						WLAN_GREEN_AP_PS_WAIT_STATE,
						WLAN_GREEN_AP_PS_WAIT_EVENT);
			qdf_timer_start(&green_ap_ctx->ps_timer,
					green_ap_ctx->ps_trans_time * 1000);
		}
		break;

	case WLAN_GREEN_AP_PS_WAIT_STATE:
		if ((!green_ap_ctx->num_nodes &&
		     green_ap_ctx->ps_mode == WLAN_GREEN_AP_MODE_NO_STA) ||
		    (!green_ap_ctx->num_nodes_multistream &&
		     green_ap_ctx->ps_mode == WLAN_GREEN_AP_MODE_NUM_STREAM)) {
			if (event == WLAN_GREEN_AP_DEL_MULTISTREAM_STA_EVENT)
				break;
			if ((channel == 0) || (channel_flags == 0)) {
				/*
				 * Stay in the current state and restart the
				 * timer to check later.
				 */
				qdf_timer_start(&green_ap_ctx->ps_timer,
					green_ap_ctx->ps_on_time * 1000);
			} else {
				wlan_green_ap_ps_event_state_update(
						green_ap_ctx,
						WLAN_GREEN_AP_PS_ON_STATE,
						WLAN_GREEN_AP_PS_WAIT_EVENT);

				green_ap_info("Transition to ON from WAIT");
				green_ap_tx_ops->ps_on_off_send(
					green_ap_ctx->pdev, true, pdev_id);
				wlan_green_ap_ant_ps_reset(green_ap_ctx);

				if (green_ap_ctx->ps_on_time)
					qdf_timer_start(&green_ap_ctx->ps_timer,
						green_ap_ctx->ps_on_time * 1000);
			}
		} else {
			green_ap_info("Transition to OFF from WAIT");
			qdf_timer_stop(&green_ap_ctx->ps_timer);
			wlan_green_ap_ps_event_state_update(
						green_ap_ctx,
						WLAN_GREEN_AP_PS_OFF_STATE,
						WLAN_GREEN_AP_PS_WAIT_EVENT);
		}
		break;

	case WLAN_GREEN_AP_PS_ON_STATE:
		if ((green_ap_ctx->num_nodes &&
		     green_ap_ctx->ps_mode == WLAN_GREEN_AP_MODE_NO_STA) ||
		    (green_ap_ctx->num_nodes_multistream &&
		     green_ap_ctx->ps_mode == WLAN_GREEN_AP_MODE_NUM_STREAM)) {
			qdf_timer_stop(&green_ap_ctx->ps_timer);
			if (green_ap_tx_ops->ps_on_off_send(
					green_ap_ctx->pdev, false, pdev_id)) {
				green_ap_err("Failed to set Green AP mode");
				goto done;
			}
			wlan_green_ap_ant_ps_reset(green_ap_ctx);
			green_ap_info("Transition to OFF from ON\n");
			wlan_green_ap_ps_event_state_update(
						green_ap_ctx,
						WLAN_GREEN_AP_PS_OFF_STATE,
						WLAN_GREEN_AP_PS_WAIT_EVENT);
		} else if ((green_ap_ctx->ps_event ==
					WLAN_GREEN_AP_PS_WAIT_EVENT) &&
			   (green_ap_ctx->ps_on_time)) {
			/* ps_on_time timeout, switch to ps wait */
			wlan_green_ap_ps_event_state_update(
						green_ap_ctx,
						WLAN_GREEN_AP_PS_WAIT_STATE,
						WLAN_GREEN_AP_PS_ON_EVENT);

			if (green_ap_tx_ops->ps_on_off_send(
					green_ap_ctx->pdev, false, pdev_id)) {
				green_ap_err("Failed to set Green AP mode");
				goto done;
			}

			wlan_green_ap_ant_ps_reset(green_ap_ctx);
			green_ap_info("Transition to WAIT from ON\n");
			qdf_timer_start(&green_ap_ctx->ps_timer,
					green_ap_ctx->ps_trans_time * 1000);
		}
		break;

	default:
		green_ap_err("invalid state %d", green_ap_ctx->ps_state);
		wlan_green_ap_ps_event_state_update(
						green_ap_ctx,
						WLAN_GREEN_AP_PS_OFF_STATE,
						WLAN_GREEN_AP_PS_WAIT_EVENT);
		break;
	}

done:
	qdf_spin_unlock_bh(&green_ap_ctx->lock);
	return QDF_STATUS_SUCCESS;
}

void wlan_green_ap_timer_fn(void *pdev)
{
	struct wlan_pdev_green_ap_ctx *green_ap_ctx;
	struct wlan_objmgr_pdev *pdev_ctx = (struct wlan_objmgr_pdev *)pdev;

	if (!pdev_ctx) {
		green_ap_err("pdev context passed is NULL");
		return;
	}

	green_ap_ctx = wlan_objmgr_pdev_get_comp_private_obj(
			pdev_ctx, WLAN_UMAC_COMP_GREEN_AP);
	if (!green_ap_ctx) {
		green_ap_err("green ap context obtained is NULL");
		return;
	}
	wlan_green_ap_state_mc(green_ap_ctx, green_ap_ctx->ps_event);
}

void wlan_green_ap_check_mode(struct wlan_objmgr_pdev *pdev,
		void *object,
		void *arg)
{
	struct wlan_objmgr_vdev *vdev = (struct wlan_objmgr_vdev *)object;
	uint8_t *flag = (uint8_t *)arg;

	wlan_vdev_obj_lock(vdev);
	if (wlan_vdev_mlme_get_opmode(vdev) != QDF_SAP_MODE)
		*flag = 1;

	wlan_vdev_obj_unlock(vdev);
}

