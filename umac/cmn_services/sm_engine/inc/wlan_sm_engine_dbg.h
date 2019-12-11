/*
 * Copyright (c) 2018-2020 The Linux Foundation. All rights reserved.
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
  * DOC: Define the debug data structure of UMAC SM
  */
#ifndef _WLAN_SM_ENGINE_DBG_H_
#define _WLAN_SM_ENGINE_DBG_H_

#include <qdf_types.h>
#include <qdf_trace.h>

#define sm_engine_alert(params...) \
		QDF_TRACE_FATAL(QDF_MODULE_ID_SM_ENGINE, params)

#define sm_engine_err(params...) \
		QDF_TRACE_ERROR(QDF_MODULE_ID_SM_ENGINE, params)

#define sm_engine_warn(params...) \
		QDF_TRACE_WARN(QDF_MODULE_ID_SM_ENGINE, params)

#define sm_engine_info(params...) \
		QDF_TRACE_INFO(QDF_MODULE_ID_SM_ENGINE, params)

#define sm_engine_debug(params...) \
		QDF_TRACE_DEBUG(QDF_MODULE_ID_SM_ENGINE, params)

#define sm_engine_nofl_alert(params...) \
	QDF_TRACE_FATAL_NO_FL(QDF_MODULE_ID_SM_ENGINE, params)
#define sm_engine_nofl_err(params...) \
	QDF_TRACE_ERROR_NO_FL(QDF_MODULE_ID_SM_ENGINE, params)
#define sm_engine_nofl_warn(params...) \
	QDF_TRACE_WARN_NO_FL(QDF_MODULE_ID_SM_ENGINE, params)
#define sm_engine_nofl_info(params...) \
	QDF_TRACE_INFO_NO_FL(QDF_MODULE_ID_SM_ENGINE, params)
#define sm_engine_nofl_debug(params...) \
	QDF_TRACE_DEBUG_NO_FL(QDF_MODULE_ID_SM_ENGINE, params)

#define WLAN_SM_ENGINE_HISTORY_SIZE  50
struct wlan_sm;
/**
 * enum wlan_sm_trace_type - history element type
 * @SM_EVENT_STATE_TRANSITION - Represents state transition
 * @SM_EVENT_MSG_PROCESSING - Represents event processing
 */
enum wlan_sm_trace_type {
	SM_EVENT_STATE_TRANSITION = 1,
	SM_EVENT_MSG_PROCESSING,
};

#ifdef SM_ENG_HIST_ENABLE

/**
 * struct wlan_sm_history_info - history element structure
 * @trace_type:      history element type
 * @event_type:      Type of the event
 * @initial_state:   Current state (state/sub-state)
 * @final_state:     New state
 */
struct wlan_sm_history_info {
	enum wlan_sm_trace_type trace_type;
	uint8_t event_type;
	uint8_t initial_state;
	uint8_t final_state;
};

/**
 * struct wlan_sm_history - history structure
 * @sm_history_lock: SM history lock
 * @index:           Last updated entry index
 * @data:            Histoy elements array
 */
struct wlan_sm_history {
	qdf_spinlock_t sm_history_lock;
	uint8_t index;
	struct wlan_sm_history_info data[WLAN_SM_ENGINE_HISTORY_SIZE];
};

/**
 * wlan_sm_save_history() - API to save SM history
 * @sm: state machine handle
 * @trace_type: type of operation
 * @initial_state: current state
 * @final_state: Resultant state
 * @event_type: Event id
 *
 * Stores the SM state transition and event processing
 *
 * Return: void
 */
void wlan_sm_save_history(struct wlan_sm *sm,
			  enum wlan_sm_trace_type trace_type,
			  uint8_t initial_state, uint8_t final_state,
			  uint16_t event_type);

/**
 * wlan_sm_history_init() - API to initialize SM history module
 * @sm: state machine handle
 *
 * Initializes SM history module
 *
 * Return: void
 */
void wlan_sm_history_init(struct wlan_sm *sm);

/**
 * wlan_sm_history_delete() - API to delete SM history module
 * @sm: state machine handle
 *
 * Deletes SM history module
 *
 * Return: void
 */
void wlan_sm_history_delete(struct wlan_sm *sm);

/**
 * wlan_sm_print_history() - API to print SM history
 * @sm: state machine handle
 *
 * Prints SM history
 *
 * Return: void
 */
void wlan_sm_print_history(struct wlan_sm *sm);

#if SM_HIST_DEBUGFS_SUPPORT
/**
 * wlan_sm_print_fs_history() - API to print SM history in proc
 * @sm: state machine handle
 * @m: debug fs file handle
 *
 * Prints SM history through proc
 *
 * Return: void
 */
void wlan_sm_print_fs_history(struct wlan_sm *sm, qdf_debugfs_file_t m);
#endif
#else /* SM_ENG_HIST_ENABLE */

/**
 * wlan_sm_save_history() - API to save SM history
 * @sm: state machine handle
 * @trace_type: type of operation
 * @initial_state: current state
 * @final_state: Resultant state
 * @event_type: Event id
 *
 * Stores the SM state transition and event processing
 *
 * Return: void
 */
static inline void wlan_sm_save_history(struct wlan_sm *sm,
					enum wlan_sm_trace_type trace_type,
					uint8_t initial_state,
					uint8_t final_state,
					uint16_t event_type)
{
}

/**
 * wlan_sm_history_init() - API to initialize SM history module
 * @sm: state machine handle
 *
 * Initializes SM history module
 *
 * Return: void
 */
static inline void wlan_sm_history_init(struct wlan_sm *sm)
{
}

/**
 * wlan_sm_history_delete() - API to delete SM history module
 * @sm: state machine handle
 *
 * Deletes SM history module
 *
 * Return: void
 */
static inline void wlan_sm_history_delete(struct wlan_sm *sm)
{
}

/**
 * wlan_sm_print_history() - API to print SM history
 * @sm: state machine handle
 *
 * Prints SM history
 *
 * Return: void
 */
static inline void wlan_sm_print_history(struct wlan_sm *sm)
{
}

#endif    /* SM_ENG_HIST_ENABLE */
#endif    /* _WLAN_SM_ENGINE_DBG_H_ */
