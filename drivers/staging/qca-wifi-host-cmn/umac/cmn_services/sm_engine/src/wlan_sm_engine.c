/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
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
 * DOC: Implements general SM framework
 */

#include "wlan_sm_engine.h"
#include "wlan_sm_engine_dbg.h"
#include <qdf_module.h>
#include <qdf_mem.h>
#include <qdf_str.h>

QDF_STATUS wlan_sm_dispatch(struct wlan_sm *sm, uint16_t event,
			    uint16_t event_data_len, void *event_data)
{
	bool event_handled = false;
	uint8_t state;
	const char *event_name = NULL;

	if (!sm) {
		sm_engine_err("SM is NULL");
		return QDF_STATUS_E_FAILURE;
	}

	state = sm->cur_state;

	if (event == WLAN_SM_ENGINE_EVENT_NONE) {
		sm_engine_err("%s: invalid event %d", sm->name, event);
		return QDF_STATUS_E_FAILURE;
	}
	sm->last_event = event;

	wlan_sm_save_history(sm, SM_EVENT_MSG_PROCESSING, sm->cur_state,
			     sm->cur_state, event);

	if (sm->event_names) {
		if (event < sm->num_event_names)
			event_name = sm->event_names[event];

		sm_engine_nofl_debug("%s: %s, %s", sm->name,
				     sm->state_info[state].name,
				     event_name ? event_name : "UNKNOWN_EVENT");
	} else {
		sm_engine_nofl_debug("%s: %s ev [%d]", sm->name,
				     sm->state_info[state].name, event);
	}

	if (state != WLAN_SM_ENGINE_STATE_NONE) {
		event_handled = (*sm->state_info[state].wlan_sm_event) (
				 sm->ctx, event, event_data_len, event_data);
		if (!event_handled) {
			sm_engine_nofl_info("%s: event %d not handled in state %s",
					    sm->name, event,
					    sm->state_info[sm->cur_state].name);
			return QDF_STATUS_E_INVAL;
		}
	}

	return QDF_STATUS_SUCCESS;
}

qdf_export_symbol(wlan_sm_dispatch);

void wlan_sm_transition_to(struct wlan_sm *sm, uint8_t state)
{
	struct wlan_sm_state_info *state_info;
	uint8_t new_state;
	uint8_t old_state;
	uint8_t new_sub_st;
	uint8_t ol_sub_st;
	uint8_t cur_state;

	if (!sm) {
		sm_engine_err("SM is NULL");
		return;
	}

	state_info = sm->state_info;
	cur_state = sm->cur_state;

	/* cannot change state from state entry/exit routines */
	if (qdf_atomic_read(&sm->in_state_transition)) {
		sm_engine_alert(
			"%s: can not call state transition from entry/exit routines",
					sm->name);
		QDF_BUG(0);
		return;
	}

	qdf_atomic_set(&sm->in_state_transition, 1);

	wlan_sm_save_history(sm, SM_EVENT_STATE_TRANSITION, sm->cur_state,
			     state, 0xFF);

	if ((state == WLAN_SM_ENGINE_STATE_NONE) ||
	    (state >= WLAN_SM_ENGINE_MAX_STATES) ||
	    (state >= sm->num_states)) {
		sm_engine_err(
			"%s: to state %d needs to be a valid state current_state=%d",
					sm->name, cur_state, state);
		return;
	}

	/*
	 * Here state and sub state are derived for debug printing only
	 * as SME keeps state and sub state as flat, to differentiate between
	 * state and substate, checks current state if it has parent state,
	 * the parent state is printed along with the sub state
	 */
	if (state_info[cur_state].parent_state != WLAN_SM_ENGINE_STATE_NONE)
		old_state = state_info[cur_state].parent_state;
	else
		old_state = cur_state;

	if (state_info[state].parent_state != WLAN_SM_ENGINE_STATE_NONE)
		new_state = state_info[state].parent_state;
	else
		new_state = state;

	if (state_info[cur_state].parent_state != WLAN_SM_ENGINE_STATE_NONE)
		ol_sub_st = cur_state;
	else
		ol_sub_st = 0;

	if (state_info[state].parent_state != WLAN_SM_ENGINE_STATE_NONE)
		new_sub_st = state;
	else
		new_sub_st = 0;

	sm_engine_nofl_debug("%s: %s > %s, %s > %s", sm->name,
			     state_info[old_state].name,
			     state_info[new_state].name,
			     ol_sub_st ? state_info[ol_sub_st].name : "IDLE",
			     new_sub_st ? state_info[new_sub_st].name : "IDLE");

	/*
	 * call the exit function(s) of the current state hierarchy
	 * starting from substate.
	 */
	while (cur_state != WLAN_SM_ENGINE_STATE_NONE) {
		if (state_info[cur_state].wlan_sm_exit)
			state_info[cur_state].wlan_sm_exit(sm->ctx);

		cur_state = state_info[cur_state].parent_state;
	}

	/*
	 * call the entry function(s) of the current state hierarchy
	 * starting from superstate.
	 */
	cur_state = state;
	while (cur_state != WLAN_SM_ENGINE_STATE_NONE) {
		if (state_info[cur_state].wlan_sm_entry)
			state_info[cur_state].wlan_sm_entry(sm->ctx);

		sm->cur_state = cur_state;
		cur_state = state_info[cur_state].initial_substate;

		if (cur_state != WLAN_SM_ENGINE_STATE_NONE)
			sm_engine_nofl_debug("%s: Initial sub state %s",
					     sm->name,
					     state_info[cur_state].name);
	}
	qdf_atomic_set(&sm->in_state_transition, 0);
}

qdf_export_symbol(wlan_sm_transition_to);

void wlan_sm_reset(struct wlan_sm *sm, uint8_t init_state)
{
	sm->cur_state = init_state;
}

static QDF_STATUS wlan_sm_validate_state_info(const char *name,
				const struct wlan_sm_state_info *state_info,
				uint8_t i)
{
	bool state_visited[WLAN_SM_ENGINE_MAX_STATES] = {false};
	uint8_t state, next_state;
	/*
	 * make sure that the state definitions are in order
	 */
	if ((state_info[i].state >= WLAN_SM_ENGINE_MAX_STATES) ||
	    (state_info[i].state != i)) {
		sm_engine_err("%s: entry %d has invalid state %d",
			      name, i, state_info[i].state);

		return QDF_STATUS_E_FAILURE;
	}
	/* detect if there is any loop in the hierarichy */
	state = state_info[i].state;
	while (state != WLAN_SM_ENGINE_STATE_NONE) {
		if (state_visited[state]) {
			sm_engine_err("%s: detected a loop with entry %d",
				      name, i);
			return QDF_STATUS_E_FAILURE;
		}

		state_visited[state] = true;
		next_state = state_info[state].parent_state;
		if (next_state != WLAN_SM_ENGINE_STATE_NONE) {
			if (!state_info[next_state].has_substates) {
				sm_engine_err(
					"%s: state %d is marked as parent of %d but is not a super state",
						name, next_state, state);
				return QDF_STATUS_E_FAILURE;
			}
		}
		state = next_state;
	}

	return QDF_STATUS_SUCCESS;
}

struct wlan_sm *wlan_sm_create(const char *name, void *ctx,
			       uint8_t init_state,
			       struct wlan_sm_state_info *state_info,
			       uint8_t num_states,
			       const char **event_names,
			       uint32_t num_event_names)
{
	struct wlan_sm *sm;
	u_int32_t i;

	if (num_states > WLAN_SM_ENGINE_MAX_STATES) {
		sm_engine_err("%s: Num states exceeded", name);
		return NULL;
	}

	/*
	 * validate the state_info table.
	 * the entries need to be valid and also
	 * need to be in order.
	 */
	for (i = 0; i < num_states; ++i) {
		if (wlan_sm_validate_state_info(name, state_info, i) !=
				QDF_STATUS_SUCCESS) {
			sm_engine_err("%s: states validation failed", name);
			return NULL;
		}
	}

	sm = qdf_mem_malloc(sizeof(*sm));
	if (!sm)
		return NULL;

	wlan_sm_history_init(sm);

	sm->cur_state = init_state;
	sm->num_states = num_states;
	sm->state_info = state_info;
	sm->ctx = ctx;
	sm->last_event = WLAN_SM_ENGINE_EVENT_NONE;
	qdf_atomic_set(&sm->in_state_transition, 0);
	sm->event_names = event_names;
	sm->num_event_names = num_event_names;

	qdf_str_lcopy(sm->name, name, WLAN_SM_ENGINE_MAX_NAME);

	sm_engine_debug("%s: sm creation successful", name);

	return sm;
}

qdf_export_symbol(wlan_sm_create);

void wlan_sm_delete(struct wlan_sm *sm)
{
	wlan_sm_history_delete(sm);
	qdf_mem_free(sm);
}

qdf_export_symbol(wlan_sm_delete);

uint8_t wlan_sm_get_lastevent(struct wlan_sm *sm)
{
	return sm->last_event;
}

uint8_t wlan_sm_get_current_state(struct wlan_sm *sm)
{
	return sm->cur_state;
}

qdf_export_symbol(wlan_sm_get_current_state);

const char *wlan_sm_get_state_name(struct wlan_sm *sm, uint8_t state)
{
	return sm->state_info[state].name;
}

const char *wlan_sm_get_current_state_name(struct wlan_sm *sm)
{
	return sm->state_info[sm->cur_state].name;
}

qdf_export_symbol(wlan_sm_get_current_state_name);
