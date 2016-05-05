/*
 * Copyright (c) 2014-2016 The Linux Foundation. All rights reserved.
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

/**
 *  DOC: wlan_hdd_green_ap.c
 *
 *  WLAN Host Device Driver Green AP implementation
 *
 */

/* denote that this file does not allow legacy hddLog */
#define HDD_DISALLOW_LEGACY_HDDLOG 1

/* Include Files */
#include <wlan_hdd_main.h>
#include <wlan_hdd_misc.h>
#include "wlan_hdd_green_ap.h"
#include "wma_api.h"
#include "cds_concurrency.h"

#define GREEN_AP_PS_ON_TIME        (0)
#define GREEN_AP_PS_DELAY_TIME     (20)

/**
 * enum hdd_green_ap_ps_state - Green-AP power save states
 * @GREEN_AP_PS_IDLE_STATE: the Green_AP is not enabled
 * @GREEN_AP_PS_OFF_STATE: in Power Saving OFF state
 * @GREEN_AP_PS_WAIT_STATE: in transition to Power Saving ON state
 * @GREEN_AP_PS_ON_STATE: in Power Saving ON state
 */
enum hdd_green_ap_ps_state {
	GREEN_AP_PS_IDLE_STATE = 1,
	GREEN_AP_PS_OFF_STATE,
	GREEN_AP_PS_WAIT_STATE,
	GREEN_AP_PS_ON_STATE,
};

/**
 * enum hdd_green_ap_event - Green-AP power save events
 * @GREEN_AP_PS_START_EVENT: event to indicate to enable Green_AP
 * @GREEN_AP_PS_START_EVENT: event to indicate to disable Green_AP
 * @GREEN_AP_ADD_STA_EVENT: event to indicate a new STA connected
 * @GREEN_AP_DEL_STA_EVENT: event to indicate a STA disconnected
 * @GREEN_AP_PS_ON_EVENT: event to indicate to enter Power Saving state
 * @GREEN_AP_PS_WAIT_EVENT: event to indicate in the transition to Power Saving
 */
enum hdd_green_ap_event {
	GREEN_AP_PS_START_EVENT = 1,
	GREEN_AP_PS_STOP_EVENT,
	GREEN_AP_ADD_STA_EVENT,
	GREEN_AP_DEL_STA_EVENT,
	GREEN_AP_PS_ON_EVENT,
	GREEN_AP_PS_WAIT_EVENT,
};

/**
 * struct hdd_green_ap_ctx - Green-AP context
 * @ps_enable: Whether or not Green AP is enabled
 * @ps_on_time: Amount of time to stay in Green AP power saving state
 * @ps_delay_time: Amount of time to delay when changing states
 * @num_nodes: Number of connected clients
 * @ps_state: Current state
 * @ps_event: Event to trigger when timer expires
 * @ps_timer: Event timer
 * @egap_support: Enhanced Green AP support flag
 */
struct hdd_green_ap_ctx {
	uint8_t ps_enable;
	uint32_t ps_on_time;
	uint32_t ps_delay_time;
	uint32_t num_nodes;

	enum hdd_green_ap_ps_state ps_state;
	enum hdd_green_ap_event ps_event;

	qdf_mc_timer_t ps_timer;

	bool egap_support;
};

/**
 * hdd_wlan_green_ap_update() - update the current State and Event
 * @hdd_ctx: Global HDD context
 * @state: New state
 * @event: New event
 *
 * Return: none
 */
static void hdd_wlan_green_ap_update(struct hdd_context_s *hdd_ctx,
				     enum hdd_green_ap_ps_state state,
				     enum hdd_green_ap_event event)
{
	struct hdd_green_ap_ctx *green_ap = hdd_ctx->green_ap_ctx;

	green_ap->ps_state = state;
	green_ap->ps_event = event;
}

/**
 * hdd_wlan_green_ap_enable() - Send Green AP configuration to firmware
 * @adapter: Adapter upon which Green AP is being configured
 * @enable: Flag which indicates if Green AP is being enabled or disabled
 *
 * Return: 0 upon success, non-zero upon failure
 */
static int hdd_wlan_green_ap_enable(hdd_adapter_t *adapter,
				    uint8_t enable)
{
	int ret;

	hdd_notice("Set Green-AP val: %d", enable);

	ret = wma_cli_set_command(adapter->sessionId,
				  WMI_PDEV_GREEN_AP_PS_ENABLE_CMDID,
				  enable, DBG_CMD);

	return ret;
}

/**
 * hdd_wlan_green_ap_mc() - Green AP state machine
 * @hdd_ctx: HDD global context
 * @event: New event being processed
 *
 * Return: none
 */
static void hdd_wlan_green_ap_mc(struct hdd_context_s *hdd_ctx,
				 enum hdd_green_ap_event event)
{
	struct hdd_green_ap_ctx *green_ap;
	hdd_adapter_t *adapter;

	green_ap = hdd_ctx->green_ap_ctx;
	if (green_ap == NULL)
		return;

	hdd_notice("Green-AP event: %d, state: %d, num_nodes: %d",
		   event, green_ap->ps_state, green_ap->num_nodes);

	/* handle the green ap ps event */
	switch (event) {
	case GREEN_AP_PS_START_EVENT:
		green_ap->ps_enable = 1;
		break;

	case GREEN_AP_PS_STOP_EVENT:
		if (!(cds_get_concurrency_mode() & QDF_SAP_MASK))
			green_ap->ps_enable = 0;
		break;

	case GREEN_AP_ADD_STA_EVENT:
		green_ap->num_nodes++;
		break;

	case GREEN_AP_DEL_STA_EVENT:
		if (green_ap->num_nodes)
			green_ap->num_nodes--;
		break;

	case GREEN_AP_PS_ON_EVENT:
	case GREEN_AP_PS_WAIT_EVENT:
		break;

	default:
		hdd_err("invalid event %d", event);
		break;
	}

	/* Confirm that power save is enabled before doing state transitions */
	if (!green_ap->ps_enable) {
		hdd_notice("Green-AP is disabled");
		hdd_wlan_green_ap_update(hdd_ctx,
					 GREEN_AP_PS_IDLE_STATE,
					 GREEN_AP_PS_WAIT_EVENT);
		goto done;
	}

	adapter = hdd_get_adapter(hdd_ctx, QDF_SAP_MODE);
	if (adapter == NULL) {
		hdd_err("Green-AP no SAP adapter");
		goto done;
	}

	/* handle the green ap ps state */
	switch (green_ap->ps_state) {
	case GREEN_AP_PS_IDLE_STATE:
		hdd_wlan_green_ap_update(hdd_ctx,
					 GREEN_AP_PS_OFF_STATE,
					 GREEN_AP_PS_WAIT_EVENT);
		break;

	case GREEN_AP_PS_OFF_STATE:
		if (!green_ap->num_nodes) {
			hdd_wlan_green_ap_update(hdd_ctx,
						 GREEN_AP_PS_WAIT_STATE,
						 GREEN_AP_PS_WAIT_EVENT);
			qdf_mc_timer_start(&green_ap->ps_timer,
					   green_ap->ps_delay_time);
		}
		break;

	case GREEN_AP_PS_WAIT_STATE:
		if (!green_ap->num_nodes) {
			hdd_wlan_green_ap_update(hdd_ctx,
						 GREEN_AP_PS_ON_STATE,
						 GREEN_AP_PS_WAIT_EVENT);

			hdd_wlan_green_ap_enable(adapter, 1);

			if (green_ap->ps_on_time) {
				hdd_wlan_green_ap_update(hdd_ctx,
							 0,
							 GREEN_AP_PS_WAIT_EVENT);
				qdf_mc_timer_start(&green_ap->ps_timer,
						   green_ap->ps_on_time);
			}
		} else {
			hdd_wlan_green_ap_update(hdd_ctx,
						 GREEN_AP_PS_OFF_STATE,
						 GREEN_AP_PS_WAIT_EVENT);
		}
		break;

	case GREEN_AP_PS_ON_STATE:
		if (green_ap->num_nodes) {
			if (hdd_wlan_green_ap_enable(adapter, 0)) {
				hdd_err("FAILED TO SET GREEN-AP mode");
				goto done;
			}
			hdd_wlan_green_ap_update(hdd_ctx,
						 GREEN_AP_PS_OFF_STATE,
						 GREEN_AP_PS_WAIT_EVENT);
		} else if ((green_ap->ps_event == GREEN_AP_PS_WAIT_EVENT)
			   && (green_ap->ps_on_time)) {

			/* ps_on_time timeout, switch to ps off */
			hdd_wlan_green_ap_update(hdd_ctx,
						 GREEN_AP_PS_WAIT_STATE,
						 GREEN_AP_PS_ON_EVENT);

			if (hdd_wlan_green_ap_enable(adapter, 0)) {
				hdd_err("FAILED TO SET GREEN-AP mode");
				goto done;
			}

			qdf_mc_timer_start(&green_ap->ps_timer,
					   green_ap->ps_delay_time);
		}
		break;

	default:
		hdd_err("invalid state %d", green_ap->ps_state);
		hdd_wlan_green_ap_update(hdd_ctx, GREEN_AP_PS_OFF_STATE,
					 GREEN_AP_PS_WAIT_EVENT);
		break;
	}

done:
	return;
}

/**
 * hdd_wlan_green_ap_timer_fn() - Green AP Timer handler
 * @ctx: Global HDD context
 *
 * Return: none
 */
static void hdd_wlan_green_ap_timer_fn(void *ctx)
{
	struct hdd_context_s *hdd_ctx = ctx;
	struct hdd_green_ap_ctx *green_ap;

	if (wlan_hdd_validate_context(hdd_ctx))
		return;

	green_ap = hdd_ctx->green_ap_ctx;
	if (green_ap)
		hdd_wlan_green_ap_mc(hdd_ctx, green_ap->ps_event);
}

/**
 * hdd_wlan_green_ap_attach() - Attach Green AP context to HDD context
 * @hdd_ctx: Global HDD contect
 *
 * Return: QDF_STATUS_SUCCESS on success, otherwise QDF_STATUS_E_** error
 */
static QDF_STATUS hdd_wlan_green_ap_attach(struct hdd_context_s *hdd_ctx)
{
	struct hdd_green_ap_ctx *green_ap;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	ENTER();

	green_ap = qdf_mem_malloc(sizeof(*green_ap));
	if (!green_ap) {
		hdd_alert("Memory allocation for Green-AP failed!");
		status = QDF_STATUS_E_NOMEM;
		goto error;
	}

	qdf_mem_zero(green_ap, sizeof(*green_ap));
	green_ap->ps_state = GREEN_AP_PS_OFF_STATE;
	green_ap->ps_event = 0;
	green_ap->num_nodes = 0;
	green_ap->ps_on_time = GREEN_AP_PS_ON_TIME;
	green_ap->ps_delay_time = GREEN_AP_PS_DELAY_TIME;

	qdf_mc_timer_init(&green_ap->ps_timer,
			  QDF_TIMER_TYPE_SW,
			  hdd_wlan_green_ap_timer_fn, hdd_ctx);

error:
	hdd_ctx->green_ap_ctx = green_ap;

	EXIT();
	return status;
}

/**
 * hdd_wlan_green_ap_deattach() - Detach Green AP context from HDD context
 * @hdd_ctx: Global HDD contect
 *
 * Return: QDF_STATUS_SUCCESS on success, otherwise QDF_STATUS_E_** error
 */
static QDF_STATUS hdd_wlan_green_ap_deattach(struct hdd_context_s *hdd_ctx)
{
	struct hdd_green_ap_ctx *green_ap = hdd_ctx->green_ap_ctx;
	QDF_STATUS status = QDF_STATUS_SUCCESS;

	ENTER();

	if (green_ap == NULL) {
		hdd_notice("Green-AP is not enabled");
		status = QDF_STATUS_E_NOSUPPORT;
		goto done;
	}

	/* check if the timer status is destroyed */
	if (QDF_TIMER_STATE_RUNNING ==
	    qdf_mc_timer_get_current_state(&green_ap->ps_timer))
		qdf_mc_timer_stop(&green_ap->ps_timer);

	/* Destroy the Green AP timer */
	if (!QDF_IS_STATUS_SUCCESS(qdf_mc_timer_destroy(&green_ap->ps_timer)))
		hdd_notice("Cannot deallocate Green-AP's timer");

	/* release memory */
	qdf_mem_zero(green_ap, sizeof(*green_ap));
	qdf_mem_free(green_ap);
	hdd_ctx->green_ap_ctx = NULL;

done:

	EXIT();
	return status;
}

/**
 * hdd_wlan_green_ap_init() - Initialize Green AP feature
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_wlan_green_ap_init(struct hdd_context_s *hdd_ctx)
{
	if (!QDF_IS_STATUS_SUCCESS(hdd_wlan_green_ap_attach(hdd_ctx)))
		hdd_err("Failed to allocate Green-AP resource");
}

/**
 * hdd_wlan_green_ap_deinit() - De-initialize Green AP feature
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_wlan_green_ap_deinit(struct hdd_context_s *hdd_ctx)
{
	if (!QDF_IS_STATUS_SUCCESS(hdd_wlan_green_ap_deattach(hdd_ctx)))
		hdd_err("Cannot deallocate Green-AP resource");
}

/**
 * hdd_wlan_green_ap_start_bss() - Notify Green AP of Start BSS event
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_wlan_green_ap_start_bss(struct hdd_context_s *hdd_ctx)
{
	struct hdd_config *cfg = hdd_ctx->config;

	/* check if the firmware and ini are both enabled the egap,
	 * and also the feature_flag enable, then we enable the egap
	 */
	if (hdd_ctx->green_ap_ctx->egap_support && cfg->enable_egap &&
	    cfg->egap_feature_flag) {
		hdd_notice("Set EGAP - enabled: %d, flag: %x, inact_time: %d, wait_time: %d",
			   cfg->enable_egap, cfg->egap_feature_flag,
			   cfg->egap_inact_time, cfg->egap_wait_time);
		if (!sme_send_egap_conf_params(cfg->enable_egap,
					       cfg->egap_inact_time,
					       cfg->egap_wait_time,
					       cfg->egap_feature_flag)) {
			/* EGAP is enabled, disable host GAP */
			hdd_wlan_green_ap_mc(hdd_ctx, GREEN_AP_PS_STOP_EVENT);
			goto exit;
		}
		/* fall through, if send_egap_conf_params() failed,
		 * then check host GAP and enable it accordingly
		 */
	}

	if (!(QDF_STA_MASK & hdd_ctx->concurrency_mode) &&
	    cfg->enable2x2 && cfg->enableGreenAP) {
		hdd_wlan_green_ap_mc(hdd_ctx, GREEN_AP_PS_START_EVENT);
	} else {
		hdd_wlan_green_ap_mc(hdd_ctx, GREEN_AP_PS_STOP_EVENT);
		hdd_notice("Green-AP: is disabled, due to sta_concurrency: %d, enable2x2: %d, enableGreenAP: %d",
			   QDF_STA_MASK & hdd_ctx->concurrency_mode,
			   cfg->enable2x2, cfg->enableGreenAP);
	}
exit:
	return;
}

/**
 * hdd_wlan_green_ap_stop_bss() - Notify Green AP of Stop BSS event
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_wlan_green_ap_stop_bss(struct hdd_context_s *hdd_ctx)
{
	hdd_wlan_green_ap_mc(hdd_ctx, GREEN_AP_PS_STOP_EVENT);
}

/**
 * hdd_wlan_green_ap_add_sta() - Notify Green AP of Add Station  event
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_wlan_green_ap_add_sta(struct hdd_context_s *hdd_ctx)
{
	hdd_wlan_green_ap_mc(hdd_ctx, GREEN_AP_ADD_STA_EVENT);
}

/**
 * hdd_wlan_green_ap_del_sta() - Notify Green AP of Delete Station event
 * @hdd_ctx: HDD global context
 *
 * Return: none
 */
void hdd_wlan_green_ap_del_sta(struct hdd_context_s *hdd_ctx)
{
	hdd_wlan_green_ap_mc(hdd_ctx, GREEN_AP_DEL_STA_EVENT);
}

/**
 * hdd_wlan_set_egap_support() - helper function to set egap support flag
 * @hdd_ctx:   pointer to hdd context
 * @param:     pointer to target configuration
 *
 * Return:     None
 */
void hdd_wlan_set_egap_support(hdd_context_t *hdd_ctx, void *param)
{
	struct wma_tgt_cfg *cfg = (struct wma_tgt_cfg *) param;

	if (hdd_ctx && cfg)
		hdd_ctx->green_ap_ctx->egap_support = cfg->egap_support;
}
