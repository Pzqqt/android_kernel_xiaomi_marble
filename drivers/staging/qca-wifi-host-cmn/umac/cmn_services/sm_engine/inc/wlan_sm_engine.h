/*
 * Copyright (c) 2018 The Linux Foundation. All rights reserved.
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
 * DOC: Define general SM framework, The modules can implement state machine
 * using framework, it provides framework for state, event, state transition,
 * event handling
 *
 * The module, whoever implement SM using this framework need to define an array
 * of structures (of type struct wlan_sm_state_info) as below,
 * for example, sample state array definition
 *
 * struct wlan_sm_state_info sm_info[] = {
 *     {
 *         (uint8_t) WLAN_VDEV_S_INIT,
 *         (uint8_t) WLAN_SM_ENGINE_STATE_NONE,
 *         (uint8_t) WLAN_SM_ENGINE_STATE_NONE,
 *         true,
 *         "INIT",
 *         mlme_vdev_state_init_entry, --> gets invoked on entering the state
 *         mlme_vdev_state_init_exit, --> gets invoked before exiting the state
 *         mlme_vdev_state_init_event  --> gets invoked on event dispatch
 *     },
 *     {
 *         (uint8_t) WLAN_VDEV_S_START,
 *         (uint8_t) WLAN_SM_ENGINE_STATE_NONE,
 *         (uint8_t) WLAN_SM_ENGINE_STATE_NONE,
 *         true,
 *         "START",
 *         mlme_vdev_state_start_entry,
 *         mlme_vdev_state_start_exit,
 *         mlme_vdev_state_start_event
 *     },
 *     {
 *        (uint8_t) WLAN_VDEV_S_DFS_CAC_WAIT,
 *        (uint8_t) WLAN_SM_ENGINE_STATE_NONE,
 *        (uint8_t) WLAN_SM_ENGINE_STATE_NONE,
 *        true,
 *        "DFS_CAC_WAIT",
 *        mlme_vdev_state_dfs_cac_wait_entry,
 *        mlme_vdev_state_dfs_cac_wait_exit,
 *        mlme_vdev_state_dfs_cac_wait_event
 *     },
 *     ...
 *  };
 *
 *  Invoke wlan_sm_create() with this state array as below
 *
 *  sm = wlan_sm_create("VDEV_SM", vdev_obj, WLAN_VDEV_S_INIT, sm_info, 3,
 *                       event_names[], num_events);
 *
 *  on successful creation, invoke below functions to dispatch events and state
 *  transition
 *
 *  Event dispatch:
 *    wlan_sm_dispatch(sm, start_event_id, 0, NULL);
 *
 *  State transition:
 *     wlan_sm_transition_to(sm, WLAN_VDEV_S_INIT);
 *
 *
 */
#ifndef _WLAN_SM_ENGINE_H_
#define _WLAN_SM_ENGINE_H_

#include <qdf_types.h>
#include <qdf_atomic.h>
#include <wlan_sm_engine_dbg.h>

/* invalid state */
#define WLAN_SM_ENGINE_STATE_NONE 255
/* invalid event */
#define WLAN_SM_ENGINE_EVENT_NONE 255

#define WLAN_SM_ENGINE_MAX_STATE_NAME 128
#define WLAN_SM_ENGINE_MAX_NAME   64
#define WLAN_SM_ENGINE_MAX_STATES 200
#define WLAN_SM_ENGINE_MAX_EVENTS 200

/**
 * struct wlan_sm_state_info - state structure definition
 * @state:             State id
 * @parent_state:      Parent state id (optional)
 * @initial_substate:  Initial sub state of this state
 * @has_substates:     flag to specify, whether it has sub states
 * @name:              Name of the state
 * @wlan_sm_entry:    State entry callback poitner
 * @wlan_sm_exit:     State exit callback poitner
 * @wlan_sm_event:    State event callback poitner
 */
struct wlan_sm_state_info {
	uint8_t state;
	uint8_t parent_state;
	uint8_t initial_substate;
	uint8_t has_substates;
	const char *name;
	void (*wlan_sm_entry) (void *ctx);
	void (*wlan_sm_exit) (void *ctx);
	bool (*wlan_sm_event) (void *ctx, uint16_t event,
			       uint16_t event_data_len, void *event_data);
};

/**
 * struct wlan_sm - state machine structure
 * @name:                 Name of the statemachine
 * @cur_state:            Current state (state/sub-state)
 * @num_states:           Number of states
 * @last_event:           Holds the last handled event of SM
 * @state_info:           Initialized States' table
 * @ctx:                  Holds the caller's context
 * @in_state_transition:  Flag to check whether state transition is in progress
 * @event_names:          Array of event names
 * @num_event_names:      Number of event names
 * @history:              Holds the SM history pointer
 */
struct wlan_sm {
	uint8_t name[WLAN_SM_ENGINE_MAX_NAME];
	uint8_t cur_state;
	uint8_t num_states;
	uint8_t last_event;
	struct wlan_sm_state_info *state_info;
	void *ctx;
	qdf_atomic_t in_state_transition;
	const char **event_names;
	uint32_t num_event_names;
#ifdef SM_ENG_HIST_ENABLE
	struct wlan_sm_history history;
#endif
};

#define WLAN_SM_ENGINE_ENTRY(name, state, parent, initsubstate, has_substates) \
	{ state, parent, initsubstate, has_substates, \
	"##name", wlan_sm_##name_entry, wlan_sm_##name_exit,\
	wlan_sm_##name_event }

/*
 * flag definitions
 */
#define WLAN_SM_ENGINE_ASYNCHRONOUS  0x0  /* run SM asynchronously */
#define WLAN_SM_ENGINE_SYNCHRONOUS   0x1  /* run SM synchronously */

/**
 * wlan_sm_create() - SM create
 * @name: Name of SM owner module
 * @ctx: caller pointer, used on invoking callbacks
 * @init_state: Default state of the SM
 * @state_info: States' definitions
 * @num_state: Number of states
 * @event_names: Event name table
 * @num_event_names: Number of events
 *
 * Creates SM object, intializes with init_state, stores the name and owner
 * module pointer, states definition table, and event name table
 *
 * Return: Handle to struct wlan_sm on successful creation,
 *         NULL on Failure
 */
struct wlan_sm *wlan_sm_create(const char *name, void *ctx,
			       uint8_t init_state,
			       struct wlan_sm_state_info *state_info,
			       uint8_t num_states,
			       const char **event_names,
			       uint32_t num_event_names);

/**
 * wlan_sm_delete() - SM delete
 * @sm: state machine handle
 *
 * Delete SM object
 *
 * Return: void
 */
void wlan_sm_delete(struct wlan_sm *sm);

/**
 * wlan_sm_dispatch() - API to notify event to SM
 * @sm: state machine handle
 * @event: event id
 * @event_data_len: Size of event data
 * @event_data: Event data
 *
 * Notifies event to SM, it invokes event callback of the current state of SM
 *
 * Return: QDF_STATUS_SUCCESS for handling
 *         QDF_STATUS_E_INVAL for not handling
 */
QDF_STATUS wlan_sm_dispatch(struct wlan_sm *sm, uint16_t event,
			    uint16_t event_data_len, void *event_data);

/**
 * wlan_sm_transition_to() - API to move the state of SM
 * @sm: state machine handle
 * @state: State id
 *
 * Moves the SM's state
 *
 * Return: void
 */
void wlan_sm_transition_to(struct wlan_sm *sm, uint8_t state);

/**
 * wlan_sm_get_lastevent() - API to get last dispatched event
 * @sm: state machine handle
 *
 * Gets the last dispatched event
 *
 * Return: event id
 */
uint8_t wlan_sm_get_lastevent(struct wlan_sm *sm);

/**
 * wlan_sm_get_current_state() - API to get current state of SM
 * @sm: state machine handle
 *
 * Gets the current state of SM
 *
 * Return: state id
 */
uint8_t wlan_sm_get_current_state(struct wlan_sm *sm);

/**
 * wlan_sm_get_current_state_name() - API to get current state's name of SM
 * @sm: state machine handle
 *
 * Gets the current state name of SM
 *
 * Return: name of the state
 */
const char *wlan_sm_get_current_state_name(struct wlan_sm *sm);

/**
 * wlan_sm_get_state_name() - API to get state's name
 * @sm: state machine handle
 * @state: state id
 *
 * Gets the given state name of SM
 *
 * Return: name of the state
 */
const char *wlan_sm_get_state_name(struct wlan_sm *sm, uint8_t state);

/**
 * wlan_sm_reset() - API to reset SM state
 * @sm: state machine handle
 * @init_state: state to reset SM
 *
 * Resets the SM to given state
 *
 * Return: void
 */
void wlan_sm_reset(struct wlan_sm *sm, uint8_t init_state);

#endif
