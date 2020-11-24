/*
 * Copyright (c) 2016-2020 The Linux Foundation. All rights reserved.
 * Copyright (c) 2007-2008 Sam Leffler, Errno Consulting
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * DOC: This file has Agile State machine functions.
 *
 */

#include "../dfs_precac_forest.h"
#include "wlan_reg_services_api.h"

#ifdef QCA_SUPPORT_AGILE_DFS
/* dfs_start_agile_engine() - Prepare ADFS params and program the agile
 *                            engine sending agile config cmd to FW.
 * @dfs: Pointer to struct wlan_dfs.
 */
void dfs_start_agile_engine(struct wlan_dfs *dfs)
{
	struct dfs_agile_cac_params adfs_param;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;

	/* Fill the RCAC ADFS params and send it to FW.
	 * FW does not use RCAC timeout values for RCAC feature.
	 * FW runs an infinite timer.
	 */
	dfs_fill_adfs_chan_params(dfs, &adfs_param);
	adfs_param.min_precac_timeout = MIN_RCAC_DURATION;
	adfs_param.max_precac_timeout = MAX_RCAC_DURATION;
	adfs_param.ocac_mode = QUICK_RCAC_MODE;

	qdf_info("%s : %d RCAC channel request sent for pdev: %pK ch_freq: %d",
		 __func__, __LINE__, dfs->dfs_pdev_obj,
		 dfs->dfs_agile_precac_freq_mhz);

	dfs_tx_ops = wlan_psoc_get_dfs_txops(dfs_soc_obj->psoc);

	if (dfs_tx_ops && dfs_tx_ops->dfs_agile_ch_cfg_cmd)
		dfs_tx_ops->dfs_agile_ch_cfg_cmd(dfs->dfs_pdev_obj,
						 &adfs_param);
	else
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"dfs_tx_ops=%pK", dfs_tx_ops);
}

/**
 * --------------------- ROLLING CAC STATE MACHINE ----------------------
 *
 * Rolling CAC is a feature where in, a separate hardware (Agile detector)
 * will be brought up in a channel that is not the current operating channel
 * and will continue to monitor the channel non-stop, until the next
 * channel change or radar in this RCAC channel.
 *
 * Now if the Rolling CAC channel was radar free for a minimum duration
 * (1 min.) and the device is now switching to this channel, no CAC is required.
 *
 * I.e. let's say the current operating channel is 64 HT80 and we are starting
 * the agile detector in 100 HT80. After a minute of being up in 100 HT80, we
 * switch the radio to 100 HT80. This operating channel change will not
 * require CAC now since the channel was radar free for the last 1 minute,
 * as determined by the agile detector.
 *
 * Introduction of a rolling CAC state machine:
 *
 * To acheive the rolling CAC feature using the agile detector, a trivial
 * state machine is implemented, as represented below:
 *
 *                           _________________
 *                          |                 |
 *            |------------>|       INIT      |<-----------|
 *            |             |_________________|            |
 *            |                      |                     |
 *            |                      |                     |
 *            | [EV_RCAC_STOP]       | [EV_RCAC_START]     | [EV_RCAC_STOP]
 *            | [EV_ADFS_RADAR]      |                     | [EV_ADFS_RADAR]
 *            |                      |                     |
 *            |                      |                     |
 *    ________|________              |             ________|________
 *   |                 |             |----------->|                 |
 *   |    COMPLETE     |                          |     RUNNING     |
 *   |_________________|<-------------------------|_________________|
 *                             [EV_RCAC_DONE]
 *
 *
 *
 * Legend:
 *     _________________
 *    |                 |
 * 1. |   RCAC STATES   |
 *    |_________________|
 *
 * 2. [RCAC_EVENTS]
 *
 *
 * Event triggers and handlers description:
 *
 * EV_RCAC_START:
 *   Posted from vdev response and is handled by all three states.
 *   1. INIT handler:
 *        a. Check if RCAC is already running,
 *           - If yes, do not transition.
 *           - If no, go to step b.
 *        b. Check if a new RCAC channel can be found,
 *           - If no, do not transition.
 *           - If yes, transition to RUNNING.
 *
 * EV_RCAC_STOP:
 *   Posted from last vap down or config disable, handled by RUNNING
 *   and COMPLETE.
 *   1. RUNNING handler:
 *        a. Stop the HOST RCAC timer.
 *        b. Send wmi_adfs_abort_cmd to FW and transition to INIT.
 *   2. COMPLETE handler:
 *        a. Send wmi_adfs_abort_cmd to FW and transition to INIT.
 *
 * EV_ADFS_RADAR:
 *   Posted from radar detection and is handled in RUNNING and COMPLETE.
 *   1. RUNNING handler (same as EV_RCAC_START):
 *        a. Check if RCAC was running for this pdev,
 *           - If yes, transition to INIT and post EV_RCAC_START event.
 *           - If no, ignore.
 *   2. COMPLETE handler (same as EV_RCAC_START):
 *        a. Check if RCAC was running for this pdev,
 *           - If yes, transition to INIT and post EV_RCAC_START event.
 *           - If no, ignore.
 *
 *   Note: EV_ADFS_RADAR works same as EV_RCAC_START event right now, but
 *         will change in future, where, based on user preference, either
 *         a new RCAC channel will be picked (requiring the transition to
 *         INIT like present), or RCAC will be restarted on the same channel.
 *
 * EV_RCAC_DONE:
 *   Posted from host RCAC timer completion and is handled in RUNNING.
 *   1. RUNNING handler:
 *      a. mark RCAC done and transition to COMPLETE.
 *
 * Epilogue:
 *   Rolling CAC state machine is for the entire psoc and since the
 *   agile detector can run for one pdev at a time, sharing of resource is
 *   required.
 *   In case of ETSI preCAC, sharing was done in a round robin fashion where
 *   each pdev runs ADFS for it's channels alternatively. However, in RCAC, the
 *   CAC period is not defined is continuous till the next channel change.
 *
 *   Hence ADFS detector is shared as follows:
 *   1. First come first serve: the pdev that is brought up first, i.e, for
 *      the first vdev response, an RCAC_START is posted and this pdev will
 *      hold the agile detector and run RCAC till it is stopped.
 *   2. Stopping the RCAC can be either by disabling user config "rcac_en 0"
 *      or by bringing down all vaps, or if no channel is available.
 *   3. Once RCAC is stopped for a pdev, it can be started in the other pdev
 *      by restarting it's vap (i.e. a vdev response).
 *
 *   A working sequence of RCAC is as follows:
 *     - Consider that the channel configured during bring up is 52HT80.
 *       1. The First VAP's vdev_start_resp posts an event EV_RCAC_START to the
 *          RCAC state machine.
 *       2. The RCAC state machine which is in INIT state (default) receives the
 *          event, picks a channel to do rolling CAC on, e.g. channel 100HT80.
 *          The SM is then transitioned to RUNNING state.
 *       3. In the entry of RUNNING state, a host timer is started and agile
 *          cfg cmd to FW is sent.
 *       4. When the HOST timer expires, it posts the EV_RCAC_DONE event to
 *          the state machine.
 *       5. EV_RCAC_DONE event received in RUNNING state, transitions the SM
 *          to COMPLETE.
 *       6. In the entry of COMPLETE, the RCAC channel is marked as CAC done
 *          in the precac tree.
 *       7. If radar is detected on primary channel, the new channel is the
 *          RCAC channel (100HT80) which does not require CAC if the preCAC
 *          tree is marked as CAC done.
 *          Before sending vdev_start, an EV_RCAC_STOP is posted
 *          which moves the SM to INIT state clearing all the params and
 *          bringing down the agile detector.
 *          (CAC decisions are taken before).
 *       8. After vdev_resp, another EV_RCAC_START is sent to restart the
 *          RCAC SM with a new RCAC channel if available.
 *
 *   A future enhancement will be triggering RCAC_START at user level.
 */

/**
 * dfs_agile_set_curr_state() - API to set the current state of Agile SM.
 * @dfs_soc_obj: Pointer to DFS soc private object.
 * @state: value of current state.
 *
 * Return: void.
 */
static void dfs_agile_set_curr_state(struct dfs_soc_priv_obj *dfs_soc_obj,
				     enum dfs_agile_sm_state state)
{
	if (state < DFS_AGILE_S_MAX) {
		dfs_soc_obj->dfs_agile_sm_cur_state = state;
	} else {
		dfs_err(NULL, WLAN_DEBUG_DFS_ALWAYS,
			"DFS RCAC state (%d) is invalid", state);
		QDF_BUG(0);
	}
}

/**
 * dfs_agile_get_curr_state() - API to get current state of Agile SM.
 * @dfs_soc_obj: Pointer to DFS soc private object.
 *
 * Return: current state enum of type, dfs_rcac_sm_state.
 */
static enum dfs_agile_sm_state
dfs_agile_get_curr_state(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	return dfs_soc_obj->dfs_agile_sm_cur_state;
}

/**
 * dfs_rcac_sm_transition_to() - Wrapper API to transition the Agile SM state.
 * @dfs_soc_obj: Pointer to dfs soc private object that hold the SM handle.
 * @state: State to which the SM is transitioning to.
 *
 * Return: void.
 */
static void dfs_agile_sm_transition_to(struct dfs_soc_priv_obj *dfs_soc_obj,
				       enum dfs_agile_sm_state state)
{
	wlan_sm_transition_to(dfs_soc_obj->dfs_agile_sm_hdl, state);
}

/**
 * dfs_agile_sm_deliver_event() - API to post events to Agile SM.
 * @dfs_soc_obj: Pointer to dfs soc private object.
 * @event: Event to be posted to the RCAC SM.
 * @event_data_len: Length of event data.
 * @event_data: Pointer to event data.
 *
 * Return: QDF_STATUS_SUCCESS on handling the event, else failure.
 *
 * Note: This version of event posting API is not under lock and hence
 * should only be called for posting events within the SM and not be
 * under a dispatcher API without a lock.
 */
static
QDF_STATUS dfs_agile_sm_deliver_event(struct dfs_soc_priv_obj *dfs_soc_obj,
				      enum dfs_agile_sm_evt event,
				      uint16_t event_data_len,
				      void *event_data)
{
	return wlan_sm_dispatch(dfs_soc_obj->dfs_agile_sm_hdl,
				event,
				event_data_len,
				event_data);
}

#ifdef QCA_SUPPORT_ADFS_RCAC
/* dfs_start_agile_rcac_timer() - Start host agile RCAC timer.
 *
 * @dfs: Pointer to struct wlan_dfs.
 */
void dfs_start_agile_rcac_timer(struct wlan_dfs *dfs)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = dfs->dfs_soc_obj;
	uint32_t rcac_timeout = MIN_RCAC_DURATION;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
		 "Host RCAC timeout = %d ms", rcac_timeout);

	qdf_timer_mod(&dfs_soc_obj->dfs_rcac_timer,
		      rcac_timeout);
}


/* dfs_stop_agile_rcac_timer() - Cancel the RCAC timer.
 *
 * @dfs: Pointer to struct wlan_dfs.
 */
void dfs_stop_agile_rcac_timer(struct wlan_dfs *dfs)
{
	struct dfs_soc_priv_obj *dfs_soc_obj;

	dfs_soc_obj = dfs->dfs_soc_obj;
	qdf_timer_sync_cancel(&dfs_soc_obj->dfs_rcac_timer);
}


/**
 * dfs_abort_agile_rcac() - Send abort Agile RCAC to F/W.
 * @dfs: Pointer to struct wlan_dfs.
 */
static void dfs_abort_agile_rcac(struct wlan_dfs *dfs)
{

	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	dfs_stop_agile_rcac_timer(dfs);
	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);
	if (dfs_tx_ops && dfs_tx_ops->dfs_ocac_abort_cmd)
		dfs_tx_ops->dfs_ocac_abort_cmd(dfs->dfs_pdev_obj);

	qdf_mem_zero(&dfs->dfs_rcac_param, sizeof(struct dfs_rcac_params));
	dfs->dfs_agile_precac_freq_mhz = 0;
	dfs->dfs_precac_chwidth = CH_WIDTH_INVALID;
	dfs->dfs_soc_obj->cur_agile_dfs_index = DFS_PSOC_NO_IDX;
}
#else
static inline void dfs_abort_agile_rcac(struct wlan_dfs *dfs)
{
}
#endif

/* dfs_abort_agile_precac() - Reset parameters of wlan_dfs and send abort
 * to F/W.
 * @dfs: Pointer to struct wlan_dfs.
 */
static void dfs_abort_agile_precac(struct wlan_dfs *dfs)
{
	struct wlan_objmgr_psoc *psoc;
	struct wlan_lmac_if_dfs_tx_ops *dfs_tx_ops;

	psoc = wlan_pdev_get_psoc(dfs->dfs_pdev_obj);
	dfs_tx_ops = wlan_psoc_get_dfs_txops(psoc);

	dfs_agile_precac_cleanup(dfs);
	/*Send the abort to F/W as well */
	if (dfs_tx_ops && dfs_tx_ops->dfs_ocac_abort_cmd)
		dfs_tx_ops->dfs_ocac_abort_cmd(dfs->dfs_pdev_obj);
}

/**
 * dfs_agile_state_init_entry() - Entry API for INIT state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving to INIT state
 *
 * Return: void
 */
static void dfs_agile_state_init_entry(void *ctx)
{
	struct dfs_soc_priv_obj *dfs_soc = (struct dfs_soc_priv_obj *)ctx;

	dfs_agile_set_curr_state(dfs_soc, DFS_AGILE_S_INIT);
}

/**
 * dfs_agile_state_init_exit() - Exit API for INIT state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving out of INIT state
 *
 * Return: void
 */
static void dfs_agile_state_init_exit(void *ctx)
{
	/* NO OPS */
}

/**
 * dfs_init_agile_start_evt_handler() - Init state start event handler.
 * @dfs: Instance of wlan_dfs structure.
 * @dfs_soc: DFS SoC private object
 *
 * Return : True if PreCAC/RCAC chan is found.
 */
static bool  dfs_init_agile_start_evt_handler(struct wlan_dfs *dfs,
					      struct dfs_soc_priv_obj *dfs_soc)
{
	bool is_chan_found = false;

	/*For RCAC */
	if (dfs_is_agile_rcac_enabled(dfs)) {
		/* Check if feature is enabled for this DFS and if RCAC channel
		 * is valid, if those are true, send appropriate WMIs to FW
		 * and only then transition to the state as follows.
		 */
		dfs_prepare_agile_rcac_channel(dfs, &is_chan_found);
	}
	/*For PreCAC */
	else if (dfs_is_agile_precac_enabled(dfs)) {
		dfs_soc->dfs_priv[dfs->dfs_psoc_idx].agile_precac_active
			= true;
		if (!dfs_soc->precac_state_started &&
		    !dfs_soc->dfs_precac_timer_running) {
			dfs_soc->precac_state_started = true;
			dfs_prepare_agile_precac_chan(dfs, &is_chan_found);
		}
	}

	return is_chan_found;
}

/**
 * dfs_agile_state_init_event() - INIT State event handler
 * @ctx: DFS SoC private object
 * @event: Event posted to the SM.
 * @event_data_len: Length of event data.
 * @event_data: Pointer to event data.
 *
 * API to handle events in INIT state
 *
 * Return: TRUE:  on handling event
 *         FALSE: on ignoring the event
 */
static bool dfs_agile_state_init_event(void *ctx,
				      uint16_t event,
				      uint16_t event_data_len,
				      void *event_data)
{
	struct dfs_soc_priv_obj *dfs_soc = (struct dfs_soc_priv_obj *)ctx;
	bool status;
	struct wlan_dfs *dfs;
	bool is_chan_found;

	if (!event_data)
		return false;

	dfs = (struct wlan_dfs *)event_data;

	switch (event) {
	case DFS_AGILE_SM_EV_AGILE_START:

		if (dfs_soc->cur_agile_dfs_index != DFS_PSOC_NO_IDX)
			return true;

		is_chan_found = dfs_init_agile_start_evt_handler(dfs,
								 dfs_soc);
		if (is_chan_found) {
			dfs_soc->cur_agile_dfs_index = dfs->dfs_psoc_idx;
			dfs_agile_sm_transition_to(dfs_soc,
						   DFS_AGILE_S_RUNNING);
		} else {
			/*
			 * This happens when there is no preCAC chan
			 * in any of the radios
			 */
			dfs_agile_precac_cleanup(dfs);
			/* Cleanup and wait */
		}

		status = true;
		break;
	default:
		status = false;
		break;
	}

	return status;
}

/**
 * dfs_agile_state_running_entry() - Entry API for running state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving to running state
 *
 * Return: void
 */
static void dfs_agile_state_running_entry(void *ctx)
{
	struct dfs_soc_priv_obj *dfs_soc = (struct dfs_soc_priv_obj *)ctx;
	struct wlan_dfs *dfs =
		dfs_soc->dfs_priv[dfs_soc->cur_agile_dfs_index].dfs;

	dfs_agile_set_curr_state(dfs_soc, DFS_AGILE_S_RUNNING);

	/* RCAC */
	if (dfs_is_agile_rcac_enabled(dfs)) {
		dfs_start_agile_rcac_timer(dfs);
		dfs_start_agile_engine(dfs);
	}
}

/**
 * dfs_agile_state_running_exit() - Exit API for RUNNING state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving out of RUNNING state
 *
 * Return: void
 */
static void dfs_agile_state_running_exit(void *ctx)
{
	/* NO OPS */
}

/**
 * dfs_agile_state_running_event() - RUNNING State event handler
 * @ctx: DFS SoC private object
 * @event: Event posted to the SM.
 * @event_data_len: Length of event data.
 * @event_data: Pointer to event data.
 *
 * API to handle events in RUNNING state
 *
 * Return: TRUE:  on handling event
 *         FALSE: on ignoring the event
 */
static bool dfs_agile_state_running_event(void *ctx,
					 uint16_t event,
					 uint16_t event_data_len,
					 void *event_data)
{
	struct dfs_soc_priv_obj *dfs_soc = (struct dfs_soc_priv_obj *)ctx;
	bool status;
	struct wlan_dfs *dfs;
	bool is_cac_done_on_des_chan;

	if (!event_data)
		return false;

	dfs = (struct wlan_dfs *)event_data;

	if (dfs->dfs_psoc_idx != dfs_soc->cur_agile_dfs_index)
		return false;

	switch (event) {
	case DFS_AGILE_SM_EV_ADFS_RADAR:
		/* After radar is found on the Agile channel we need to find
		 * a new channel and then start Agile CAC on that.
		 * On receiving the "DFS_AGILE_SM_EV_ADFS_RADAR_FOUND" if
		 * we change the state from [RUNNING] -> [RUNNING] then
		 * [RUNNING] should handle case in which a channel is not found
		 * and bring the state machine back to INIT.
		 * Instead we move the state to INIT and post the event
		 * "DFS_AGILE_SM_EV_AGILE_START" so INIT handles the case of
		 * channel not found and stay in that state.
		 * Abort the existing RCAC and restart from INIT state.
		 */
		if (dfs_is_agile_rcac_enabled(dfs))
			dfs_abort_agile_rcac(dfs);
		else if (dfs_is_agile_precac_enabled(dfs))
			dfs_abort_agile_precac(dfs);

		dfs_agile_sm_transition_to(dfs_soc, DFS_AGILE_S_INIT);
		dfs_agile_sm_deliver_event(dfs_soc,
					   DFS_AGILE_SM_EV_AGILE_START,
					   event_data_len,
					   event_data);

		status = true;
		break;
	case DFS_AGILE_SM_EV_AGILE_STOP:
		if (dfs_is_agile_rcac_enabled(dfs))
			dfs_abort_agile_rcac(dfs);
		else if (dfs_is_agile_precac_enabled(dfs))
			dfs_abort_agile_precac(dfs);

		dfs_agile_sm_transition_to(dfs_soc, DFS_AGILE_S_INIT);
		status = true;
		break;
	case DFS_AGILE_SM_EV_AGILE_DONE:
		if (dfs_is_agile_precac_enabled(dfs)) {
			if (dfs_soc->ocac_status == OCAC_SUCCESS) {
				dfs_soc->ocac_status = OCAC_RESET;
				dfs_mark_adfs_chan_as_cac_done(dfs);
			}
			dfs_agile_sm_transition_to(dfs_soc, DFS_AGILE_S_INIT);
			dfs_agile_precac_cleanup(dfs);
			is_cac_done_on_des_chan =
				dfs_precac_check_home_chan_change(dfs);
			if (!is_cac_done_on_des_chan) {
				dfs_agile_sm_deliver_event(dfs_soc,
					DFS_AGILE_SM_EV_AGILE_START,
					event_data_len,
					event_data);
			}
		} else if (dfs_is_agile_rcac_enabled(dfs)) {
			dfs_agile_sm_transition_to(dfs_soc,
						   DFS_AGILE_S_COMPLETE);
		}
		status = true;
		break;
	default:
		status = false;
		break;
	}

	return status;
}

/**
 * dfs_agile_state_complete_entry() - Entry API for complete state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving to complete state
 *
 * Return: void
 */
static void dfs_agile_state_complete_entry(void *ctx)
{
	struct dfs_soc_priv_obj *dfs_soc_obj = (struct dfs_soc_priv_obj *)ctx;
	struct wlan_dfs *dfs;

	dfs_agile_set_curr_state(dfs_soc_obj, DFS_AGILE_S_COMPLETE);

	if (!(dfs_soc_obj->cur_agile_dfs_index < WLAN_UMAC_MAX_PDEVS))
		return;

	dfs = dfs_soc_obj->dfs_priv[dfs_soc_obj->cur_agile_dfs_index].dfs;

	/* Mark the RCAC channel as CAC done. */
	dfs_mark_adfs_chan_as_cac_done(dfs);
	/*
	 * Check if rcac is done on preffered channel.
	 * If so, change channel from intermediate channel to preffered chan.
	 */
	dfs_precac_check_home_chan_change(dfs);
	/*
	 * In case of preCAC interCAC, if the above home channel change fails,
	 * we explicitly do agile start (DFS_AGILE_SM_EV_AGILE_START) as we need
	 * to pick the next agile channel. However, in case of RCAC, if the
	 * above home channel change fails, the agile continues in the current
	 * RCAC channel therefore explicit agile start
	 * (DFS_AGILE_SM_EV_AGILE_START) is not required. Refer to function
	 * "dfs_agile_state_running_event"
	 */
}

/**
 * dfs_agile_state_complete_exit() - Exit API for complete state
 * @ctx: DFS SoC private object
 *
 * API to perform operations on moving out of complete state
 *
 * Return: void
 */
static void dfs_agile_state_complete_exit(void *ctx)
{
	/* NO OPs. */
}

/**
 * dfs_agile_state_complete_event() - COMPLETE State event handler
 * @ctx: DFS SoC private object
 * @event: Event posted to the SM.
 * @event_data_len: Length of event data.
 * @event_data: Pointer to event data.
 *
 * API to handle events in COMPLETE state
 *
 * Return: TRUE:  on handling event
 *         FALSE: on ignoring the event
 */
static bool dfs_agile_state_complete_event(void *ctx,
					  uint16_t event,
					  uint16_t event_data_len,
					  void *event_data)
{
	struct dfs_soc_priv_obj *dfs_soc = (struct dfs_soc_priv_obj *)ctx;
	bool status;
	struct wlan_dfs *dfs;

	if (!event_data)
		return false;

	dfs = (struct wlan_dfs *)event_data;

	if (dfs->dfs_psoc_idx != dfs_soc->cur_agile_dfs_index)
		return false;

	switch (event) {
	case DFS_AGILE_SM_EV_ADFS_RADAR:
		/* Reset the RCAC done state for this RCAC chan of this dfs.
		 * Unmark the channels for RCAC done before calling abort API as
		 * the abort API invalidates the cur_agile_dfs_index.
		 */
		dfs_unmark_rcac_done(dfs);
		/* Abort the existing RCAC and restart from INIT state. */
		dfs_abort_agile_rcac(dfs);
		dfs_agile_sm_transition_to(dfs_soc, DFS_AGILE_S_INIT);
		dfs_agile_sm_deliver_event(dfs_soc,
					  DFS_AGILE_SM_EV_AGILE_START,
					  event_data_len,
					  event_data);
		status = true;
		break;
	case DFS_AGILE_SM_EV_AGILE_STOP:
		/* Reset the RCAC done state for this RCAC chan of this dfs.
		 * Unmark the channels for RCAC done before calling abort API as
		 * the abort API invalidates the cur_agile_dfs_index.
		 */
		dfs_unmark_rcac_done(dfs);
		dfs_abort_agile_rcac(dfs);
		dfs_agile_sm_transition_to(dfs_soc, DFS_AGILE_S_INIT);
		status = true;
		break;
	default:
		status = false;
		break;
	}

	return status;
}

static struct wlan_sm_state_info dfs_agile_sm_info[] = {
	{
		(uint8_t)DFS_AGILE_S_INIT,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"INIT",
		dfs_agile_state_init_entry,
		dfs_agile_state_init_exit,
		dfs_agile_state_init_event
	},
	{
		(uint8_t)DFS_AGILE_S_RUNNING,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"RUNNING",
		dfs_agile_state_running_entry,
		dfs_agile_state_running_exit,
		dfs_agile_state_running_event
	},
	{
		(uint8_t)DFS_AGILE_S_COMPLETE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		(uint8_t)WLAN_SM_ENGINE_STATE_NONE,
		false,
		"COMPLETE",
		dfs_agile_state_complete_entry,
		dfs_agile_state_complete_exit,
		dfs_agile_state_complete_event
	},
};

static const char *dfs_agile_sm_event_names[] = {
	"EV_AGILE_START",
	"EV_AGILE_STOP",
	"EV_AGILE_DONE",
	"EV_ADFS_RADAR_FOUND",
};

/**
 * dfs_agile_sm_print_state() - API to log the current state.
 * @dfs_soc_obj: Pointer to dfs soc private object.
 *
 * Return: void.
 */
static void dfs_agile_sm_print_state(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	enum dfs_agile_sm_state state;

	state = dfs_agile_get_curr_state(dfs_soc_obj);
	if (!(state < DFS_AGILE_S_MAX))
		return;

	dfs_debug(NULL, WLAN_DEBUG_DFS_AGILE, "->[%s] %s",
		  dfs_soc_obj->dfs_agile_sm_hdl->name,
		  dfs_agile_sm_info[state].name);
}

/**
 * dfs_agile_sm_print_state_event() - API to log the current state and event
 *                                   received.
 * @dfs_soc_obj: Pointer to dfs soc private object.
 * @event: Event posted to RCAC SM.
 *
 * Return: void.
 */
static void dfs_agile_sm_print_state_event(struct dfs_soc_priv_obj *dfs_soc_obj,
					  enum dfs_agile_sm_evt event)
{
	enum dfs_agile_sm_state state;

	state = dfs_agile_get_curr_state(dfs_soc_obj);
	if (!(state < DFS_AGILE_S_MAX))
		return;

	dfs_debug(NULL, WLAN_DEBUG_DFS_AGILE, "[%s]%s, %s",
		  dfs_soc_obj->dfs_agile_sm_hdl->name,
		  dfs_agile_sm_info[state].name,
		  dfs_agile_sm_event_names[event]);
}

QDF_STATUS dfs_agile_sm_deliver_evt(struct dfs_soc_priv_obj *dfs_soc_obj,
				    enum dfs_agile_sm_evt event,
				    uint16_t event_data_len,
				    void *event_data)
{
	enum dfs_agile_sm_state old_state, new_state;
	QDF_STATUS status;

	DFS_AGILE_SM_SPIN_LOCK(dfs_soc_obj);
	old_state = dfs_agile_get_curr_state(dfs_soc_obj);

	/* Print current state and event received */
	dfs_agile_sm_print_state_event(dfs_soc_obj, event);

	status = dfs_agile_sm_deliver_event(dfs_soc_obj, event,
					   event_data_len, event_data);

	new_state = dfs_agile_get_curr_state(dfs_soc_obj);

	/* Print new state after event if transition happens */
	if (old_state != new_state)
		dfs_agile_sm_print_state(dfs_soc_obj);
	DFS_AGILE_SM_SPIN_UNLOCK(dfs_soc_obj);

	return status;
}

QDF_STATUS dfs_agile_sm_create(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	struct wlan_sm *sm;

	sm = wlan_sm_create("DFS_AGILE", dfs_soc_obj,
			    DFS_AGILE_S_INIT,
			    dfs_agile_sm_info,
			    QDF_ARRAY_SIZE(dfs_agile_sm_info),
			    dfs_agile_sm_event_names,
			    QDF_ARRAY_SIZE(dfs_agile_sm_event_names));
	if (!sm) {
		qdf_err("DFS AGILE SM allocation failed");
		return QDF_STATUS_E_FAILURE;
	}
	dfs_soc_obj->dfs_agile_sm_hdl = sm;

	qdf_spinlock_create(&dfs_soc_obj->dfs_agile_sm_lock);

	/* Initialize the RCAC DFS index to default (no index). */
	dfs_soc_obj->cur_agile_dfs_index = DFS_PSOC_NO_IDX;
	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dfs_agile_sm_destroy(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	wlan_sm_delete(dfs_soc_obj->dfs_agile_sm_hdl);
	qdf_spinlock_destroy(&dfs_soc_obj->dfs_agile_sm_lock);

	return QDF_STATUS_SUCCESS;
}

#ifdef QCA_SUPPORT_ADFS_RCAC
QDF_STATUS dfs_set_rcac_enable(struct wlan_dfs *dfs, bool rcac_en)
{
	if (rcac_en == dfs->dfs_agile_rcac_ucfg) {
		dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,
			 "Rolling CAC: %d is already configured", rcac_en);
		return QDF_STATUS_SUCCESS;
	}
	dfs->dfs_agile_rcac_ucfg = rcac_en;

	/* RCAC config is changed. Reset the preCAC tree. */
	dfs_reset_precac_lists(dfs);

	if (!rcac_en) {
		dfs_agile_sm_deliver_evt(dfs->dfs_soc_obj,
					DFS_AGILE_SM_EV_AGILE_STOP,
					0,
					(void *)dfs);
	}
	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS, "rolling cac is %d", rcac_en);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dfs_get_rcac_enable(struct wlan_dfs *dfs, bool *rcacen)
{
	*rcacen = dfs->dfs_agile_rcac_ucfg;

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dfs_set_rcac_freq(struct wlan_dfs *dfs, qdf_freq_t rcac_freq)
{
	if (wlan_reg_is_5ghz_ch_freq(rcac_freq))
		dfs->dfs_agile_rcac_freq_ucfg = rcac_freq;
	else
		dfs->dfs_agile_rcac_freq_ucfg = 0;

	dfs_info(dfs, WLAN_DEBUG_DFS_ALWAYS,  "rolling cac freq %d",
		 dfs->dfs_agile_rcac_freq_ucfg);

	return QDF_STATUS_SUCCESS;
}

QDF_STATUS dfs_get_rcac_freq(struct wlan_dfs *dfs, qdf_freq_t *rcac_freq)
{
	*rcac_freq = dfs->dfs_agile_rcac_freq_ucfg;

	return QDF_STATUS_SUCCESS;
}

/*
 * Rolling CAC Timer timeout function. Following actions are done
 * on timer expiry:
 * Timer running flag is cleared.
 * If the rolling CAC state is completed, the RCAC freq and its sub-channels
 * are marked as 'CAC Done' in the preCAC tree.
 */
static os_timer_func(dfs_rcac_timeout)
{
	struct wlan_dfs *dfs;
	struct dfs_soc_priv_obj *dfs_soc_obj;

	OS_GET_TIMER_ARG(dfs_soc_obj, struct dfs_soc_priv_obj *);

	dfs = dfs_soc_obj->dfs_priv[dfs_soc_obj->cur_agile_dfs_index].dfs;

	dfs_agile_sm_deliver_evt(dfs_soc_obj,
				DFS_AGILE_SM_EV_AGILE_DONE,
				0,
				(void *)dfs);
}

void dfs_rcac_timer_init(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	qdf_timer_init(NULL, &dfs_soc_obj->dfs_rcac_timer,
		       dfs_rcac_timeout,
		       (void *)dfs_soc_obj,
		       QDF_TIMER_TYPE_WAKE_APPS);
}

void dfs_rcac_timer_deinit(struct dfs_soc_priv_obj *dfs_soc_obj)
{
	qdf_timer_free(&dfs_soc_obj->dfs_rcac_timer);
}

/* dfs_prepare_agile_rcac_channel() - Find a valid Rolling CAC channel if
 *                                    available.
 *
 * @dfs: Pointer to struct wlan_dfs.
 * @is_rcac_chan_available: Flag to indicate if a valid RCAC channel is
 *                          available.
 */
void dfs_prepare_agile_rcac_channel(struct wlan_dfs *dfs,
				    bool *is_rcac_chan_available)
{
	qdf_freq_t rcac_ch_freq = 0;

	/* Find out a valid rcac_ch_freq */
	dfs_set_agilecac_chan_for_freq(dfs, &rcac_ch_freq, 0, 0);

	/* If RCAC channel is available, the caller will start the timer and
	 * send RCAC config to FW. If channel not available, the caller takes
	 * care of sending RCAC abort and moving SM to INIT, resetting the RCAC
	 * variables.
	 */
	*is_rcac_chan_available = rcac_ch_freq ? true : false;
	dfs_debug(dfs, WLAN_DEBUG_DFS_AGILE, "Chosen rcac channel: %d",
		  rcac_ch_freq);
}
#endif
#endif
