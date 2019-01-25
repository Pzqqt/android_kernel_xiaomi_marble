/*
 * Copyright (c) 2018-2019 The Linux Foundation. All rights reserved.
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/**
 * DOC: Implements general SM debug framework
 */
#include <wlan_sm_engine.h>
#include <wlan_sm_engine_dbg.h>

void wlan_sm_save_history(struct wlan_sm *sm,
			  enum wlan_sm_trace_type trace_type,
			  uint8_t initial_state, uint8_t final_state,
			  uint16_t event_type)
{
	struct wlan_sm_history *p_sm_history = &sm->history;
	struct wlan_sm_history_info *p_memento;

	/*
	 * History saved in circular buffer.
	 * Save a pointer to next write location and increment pointer.
	 */
	qdf_spin_lock_bh(&p_sm_history->sm_history_lock);
	p_memento = &p_sm_history->data[p_sm_history->index];
	p_sm_history->index++;

	if (p_sm_history->index >= WLAN_SM_ENGINE_HISTORY_SIZE)
		p_sm_history->index = 0;

	qdf_spin_unlock_bh(&p_sm_history->sm_history_lock);

	qdf_mem_zero(p_memento, sizeof(*p_memento));
	p_memento->trace_type = trace_type;
	p_memento->initial_state = initial_state;
	p_memento->final_state = final_state;
	p_memento->event_type = event_type;
}

void wlan_sm_history_init(struct wlan_sm *sm)
{
	qdf_spinlock_create(&sm->history.sm_history_lock);
}

void wlan_sm_history_delete(struct wlan_sm *sm)
{
	qdf_spinlock_destroy(&sm->history.sm_history_lock);
}

static void wlan_sm_print_history_entry(struct wlan_sm *sm,
					struct wlan_sm_history_info *ent,
					uint16_t i)
{
	const char *event_name = NULL;

	if (sm->event_names) {
		if (ent->event_type < sm->num_event_names)
			event_name = sm->event_names[ent->event_type];

		sm_engine_nofl_info(" (%d) trace_type: %d, event:%s[%d]", i,
				    ent->trace_type,
				    event_name ? event_name : "UNKNOWN_EVENT",
				    ent->event_type);
	} else {
		sm_engine_nofl_info(" (%d) trace_type: %d, event: %d", i,
				    ent->trace_type, ent->event_type);
	}

	sm_engine_nofl_info(" (%d) initial state :%s[%d], final state: %s[%d]",
			    i, sm->state_info[ent->initial_state].name,
			    ent->initial_state,
			    sm->state_info[ent->final_state].name,
			    ent->final_state);
}

void wlan_sm_print_history(struct wlan_sm *sm)
{
	struct wlan_sm_history *p_sm_history = &sm->history;
	uint16_t i;

	/*
	 * History saved in circular buffer.
	 * Save a pointer to next write location and increment pointer.
	 */
	qdf_spin_lock_bh(&p_sm_history->sm_history_lock);
	sm_engine_nofl_info("%s: SM History index is %d",
			    sm->name, p_sm_history->index);
	for (i = 0; i < WLAN_SM_ENGINE_HISTORY_SIZE; i++)
		wlan_sm_print_history_entry(sm, &p_sm_history->data[i], i);

	qdf_spin_unlock_bh(&p_sm_history->sm_history_lock);
}
