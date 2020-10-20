/*
 *
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
/*
 * DOC: Public APIs to perform debug operations on object manager
 */

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include <wlan_objmgr_debug.h>
#include "wlan_objmgr_global_obj_i.h"
#include <qdf_mem.h>
#include <qdf_platform.h>
#include <qdf_str.h>

/*
 * Default TTL (of FW) for mgmt frames is 5 sec, by considering all the other
 * delays, arrived with this value
 */
#define LOG_DEL_OBJ_TIMEOUT_VALUE_MSEC   8000
#define LOG_DEL_OBJ_DESTROY_DURATION_SEC 8
/*
 * The max duration for which a obj can be allowed to remain in L-state
 * The duration  should be higher than the psoc idle timeout.
 */
#define LOG_DEL_OBJ_DESTROY_ASSERT_DURATION_SEC 32
#define LOG_DEL_OBJ_LIST_MAX_COUNT       (3 + 5 + 48 + 4096)

union wlan_objmgr_del_obj {
	struct wlan_objmgr_psoc *obj_psoc;
	struct wlan_objmgr_pdev *obj_pdev;
	struct wlan_objmgr_vdev *obj_vdev;
	struct wlan_objmgr_peer *obj_peer;
};

/**
 * struct log_del_obj    - Logically deleted Object
 * @obj:            Represents peer/vdev/pdev/psoc
 * @node:           List node from Logically deleted list
 * @obj_type:       Object type for peer/vdev/pdev/psoc
 * @tstamp:         Timestamp when node entered logically
 *                  deleted state
 */
struct log_del_obj {
	union wlan_objmgr_del_obj obj;
	qdf_list_node_t node;
	enum wlan_objmgr_obj_type obj_type;
	qdf_time_t tstamp;
};

/**
 * struct wlan_objmgr_debug_info     - Objmgr debug info
 * for Logically deleted object
 * @obj_timer:          Timer object
 * @obj_list:           list object having linking logically
 *                       deleted nodes
 * @list_lock:          lock to protect list
 */
struct wlan_objmgr_debug_info {
	qdf_timer_t obj_timer;
	qdf_list_t obj_list;
	qdf_spinlock_t list_lock;
};

static const char *
wlan_obj_type_get_obj_name(enum wlan_objmgr_obj_type obj_type)
{
	static const struct wlan_obj_type_to_name {
		enum wlan_objmgr_obj_type obj_type;
		const char *name;
	} obj_type_name[WLAN_OBJ_TYPE_MAX] = {
		{WLAN_PSOC_OP, "psoc"},
		{WLAN_PDEV_OP, "pdev"},
		{WLAN_VDEV_OP, "vdev"},
		{WLAN_PEER_OP, "peer"}
	};
	uint8_t idx;

	for (idx = 0; idx < WLAN_OBJ_TYPE_MAX; idx++) {
		if (obj_type == obj_type_name[idx].obj_type)
			return obj_type_name[idx].name;
	}

	return NULL;
}

static uint8_t*
wlan_objmgr_debug_get_macaddr(union wlan_objmgr_del_obj *obj,
			      enum wlan_objmgr_obj_type obj_type)
{
	switch (obj_type) {
	case WLAN_PSOC_OP:
		return wlan_psoc_get_hw_macaddr(obj->obj_psoc);
	case WLAN_PDEV_OP:
		return wlan_pdev_get_hw_macaddr(obj->obj_pdev);
	case WLAN_VDEV_OP:
		return wlan_vdev_mlme_get_macaddr(obj->obj_vdev);
	case WLAN_PEER_OP:
		return wlan_peer_get_macaddr(obj->obj_peer);
	default:
		obj_mgr_err("invalid obj_type");
		return NULL;
	}
}

static void
wlan_objmgr_insert_ld_obj_to_list(struct wlan_objmgr_debug_info *debug_info,
				  qdf_list_node_t *node)
{
	/* Insert object to list with lock being held*/
	qdf_spin_lock_bh(&debug_info->list_lock);

	/* Start timer only when list is empty */
	if (qdf_list_empty(&debug_info->obj_list))
		qdf_timer_start(&debug_info->obj_timer,
				LOG_DEL_OBJ_TIMEOUT_VALUE_MSEC);

	qdf_list_insert_back(&debug_info->obj_list, node);
	qdf_spin_unlock_bh(&debug_info->list_lock);
}

static void wlan_obj_type_get_obj(union wlan_objmgr_del_obj *obj,
				  union wlan_objmgr_del_obj *del_obj,
				  enum wlan_objmgr_obj_type obj_type)
{
	switch (obj_type) {
	case WLAN_PSOC_OP:
		del_obj->obj_psoc = obj->obj_psoc;
		return;
	case WLAN_PDEV_OP:
		del_obj->obj_pdev = obj->obj_pdev;
		return;
	case WLAN_VDEV_OP:
		del_obj->obj_vdev = obj->obj_vdev;
		return;
	case WLAN_PEER_OP:
		del_obj->obj_peer = obj->obj_peer;
		return;
	default:
		obj_mgr_err("invalid obj_type");
		return;
	}
}

void wlan_objmgr_notify_log_delete(void *obj,
				   enum wlan_objmgr_obj_type obj_type)
{
	struct wlan_objmgr_debug_info *debug_info;
	const char *obj_name;
	uint8_t *macaddr;
	qdf_time_t tstamp;
	struct log_del_obj *node;
	union wlan_objmgr_del_obj *del_obj = (union wlan_objmgr_del_obj *)&obj;

	if (!obj) {
		obj_mgr_err("object is null");
		return;
	}

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	debug_info = g_umac_glb_obj->debug_info;
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);

	if (!debug_info) {
		obj_mgr_err("debug_info is null");
		return;
	}

	macaddr = wlan_objmgr_debug_get_macaddr(del_obj, obj_type);
	if (!macaddr) {
		obj_mgr_err("macaddr is null");
		return;
	}

	obj_name = wlan_obj_type_get_obj_name(obj_type);
	if (!obj_name) {
		obj_mgr_err("obj_name is null");
		return;
	}

	tstamp = qdf_system_ticks_to_msecs(qdf_system_ticks()) / 1000;
	node = qdf_mem_malloc(sizeof(*node));
	if (!node)
		return;

	wlan_obj_type_get_obj(del_obj, &node->obj, obj_type);
	node->obj_type = obj_type;
	node->tstamp = tstamp;
	obj_mgr_debug("#%s : mac_addr: "QDF_MAC_ADDR_FMT" entered L-state",
		      obj_name, QDF_MAC_ADDR_REF(macaddr));
	wlan_objmgr_insert_ld_obj_to_list(debug_info, &node->node);
}

static bool wlan_objmgr_del_obj_match(union wlan_objmgr_del_obj *obj,
				      union wlan_objmgr_del_obj *del_obj,
				      enum wlan_objmgr_obj_type obj_type)
{
	switch (obj_type) {
	case WLAN_PSOC_OP:
		return (del_obj->obj_psoc == obj->obj_psoc);
	case WLAN_PDEV_OP:
		return (del_obj->obj_pdev == obj->obj_pdev);
	case WLAN_VDEV_OP:
		return (del_obj->obj_vdev == obj->obj_vdev);
	case WLAN_PEER_OP:
		return (del_obj->obj_peer == obj->obj_peer);
	default:
		return false;
	}
}

static void
wlan_objmgr_rem_ld_obj_from_list(union wlan_objmgr_del_obj *obj,
				 struct wlan_objmgr_debug_info *debug_info,
				 enum wlan_objmgr_obj_type obj_type)
{
	qdf_list_node_t *node = NULL;
	struct log_del_obj *obj_to_remove = NULL;
	qdf_list_t *list;
	QDF_STATUS status;

	list = &debug_info->obj_list;
	qdf_spin_lock_bh(&debug_info->list_lock);
	status = qdf_list_peek_front(list, &node);

	while (QDF_IS_STATUS_SUCCESS(status)) {
		obj_to_remove = qdf_container_of(node,
						 struct log_del_obj, node);
		if (wlan_objmgr_del_obj_match(obj, &obj_to_remove->obj,
					      obj_type) &&
		    obj_to_remove->obj_type == obj_type) {
			status = qdf_list_remove_node(list,
						      &obj_to_remove->node);
			/* Stop timer if list is empty */
			if (QDF_IS_STATUS_SUCCESS(status)) {
				if (qdf_list_empty(&debug_info->obj_list))
					qdf_timer_stop(&debug_info->obj_timer);
				qdf_mem_free(obj_to_remove);
			}
			break;
		}
		status = qdf_list_peek_next(list, node, &node);
	};
	qdf_spin_unlock_bh(&debug_info->list_lock);
}

void wlan_objmgr_notify_destroy(void *obj,
				enum wlan_objmgr_obj_type obj_type)
{
	struct wlan_objmgr_debug_info *debug_info;
	uint8_t *macaddr;
	const char *obj_name;
	union wlan_objmgr_del_obj *del_obj = (union wlan_objmgr_del_obj *)&obj;

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	debug_info = g_umac_glb_obj->debug_info;
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);

	if (!debug_info) {
		obj_mgr_err("debug_info is null");
		return;
	}
	macaddr = wlan_objmgr_debug_get_macaddr(del_obj, obj_type);
	if (!macaddr) {
		obj_mgr_err("macaddr is null");
		return;
	}
	obj_name = wlan_obj_type_get_obj_name(obj_type);
	if (!obj_name) {
		obj_mgr_err("obj_name is null");
		return;
	}
	obj_mgr_debug("#%s : macaddr: "QDF_MAC_ADDR_FMT" exited L-state",
		      obj_name, QDF_MAC_ADDR_REF(macaddr));

	wlan_objmgr_rem_ld_obj_from_list(del_obj,
					 debug_info, obj_type);
}

/**
 * wlan_objmgr_debug_obj_destroyed_panic() - Panic in case obj is in L-state
 * for long
 * @obj_name: The name of the module ID
 *
 * This will invoke panic in the case that the obj is in logically destroyed
 * state for a long time. The panic is invoked only in case feature flag
 * WLAN_OBJMGR_PANIC_ON_BUG is enabled
 *
 * Return: None
 */
#ifdef CONFIG_LEAK_DETECTION
static inline void wlan_objmgr_debug_obj_destroyed_panic(const char *obj_name)
{
	obj_mgr_alert("#%s in L-state for too long!", obj_name);
	QDF_BUG(0);
}
#else
static inline void wlan_objmgr_debug_obj_destroyed_panic(const char *obj_name)
{
}
#endif

/*
 * wlan_objmgr_print_pending_refs() - Print pending refs according to the obj
 * @obj:	Represents peer/vdev/pdev/psoc
 * @obj_type:	Object type for peer/vdev/pdev/psoc
 *
 * Return: None
 */
static void wlan_objmgr_print_pending_refs(union wlan_objmgr_del_obj *obj,
					   enum wlan_objmgr_obj_type obj_type)
{
	switch (obj_type) {
	case WLAN_PSOC_OP:
		wlan_objmgr_print_ref_ids(obj->obj_psoc->soc_objmgr.ref_id_dbg,
					  QDF_TRACE_LEVEL_DEBUG);
		break;
	case WLAN_PDEV_OP:
		wlan_objmgr_print_ref_ids(obj->obj_pdev->pdev_objmgr.ref_id_dbg,
					  QDF_TRACE_LEVEL_DEBUG);
		break;
	case WLAN_VDEV_OP:
		wlan_objmgr_print_ref_ids(obj->obj_vdev->vdev_objmgr.ref_id_dbg,
					  QDF_TRACE_LEVEL_DEBUG);
		break;
	case WLAN_PEER_OP:
		wlan_objmgr_print_ref_ids(obj->obj_peer->peer_objmgr.ref_id_dbg,
					  QDF_TRACE_LEVEL_DEBUG);
		break;
	default:
		obj_mgr_debug("invalid obj_type");
	}
}

#ifdef WLAN_OBJMGR_REF_ID_TRACE
static void
wlan_objmgr_print_ref_func_line(struct wlan_objmgr_trace_func *func_head,
				uint32_t id)
{
	uint32_t ref_cnt;
	struct wlan_objmgr_line_ref_node *tmp_ln_node;

	obj_mgr_debug("ID: %s", string_from_dbgid(id));
	while (func_head) {
		obj_mgr_debug("Func: %s", func_head->func);
		tmp_ln_node = func_head->line_head;
		while (tmp_ln_node) {
			ref_cnt = qdf_atomic_read(&tmp_ln_node->line_ref.cnt);
			obj_mgr_debug("line: %d cnt: %d",
				      tmp_ln_node->line_ref.line,
				      ref_cnt);
			tmp_ln_node = tmp_ln_node->next;
		}
		func_head = func_head->next;
	}
}

static void
wlan_objmgr_trace_print_ref(union wlan_objmgr_del_obj *obj,
			    enum wlan_objmgr_obj_type obj_type)
{
	uint32_t id;
	struct wlan_objmgr_trace_func *func_head;
	struct wlan_objmgr_trace *trace;
	struct wlan_objmgr_vdev_objmgr *vdev_obj;
	struct wlan_objmgr_peer_objmgr *peer_obj;

	switch (obj_type) {
	case WLAN_VDEV_OP:
		vdev_obj = &obj->obj_vdev->vdev_objmgr;
		trace = &vdev_obj->trace;
		for (id = 0; id < WLAN_REF_ID_MAX; id++) {
			if (qdf_atomic_read(&vdev_obj->ref_id_dbg[id])) {
				obj_mgr_debug("Reference:");

				func_head = trace->references[id].head;
				wlan_objmgr_print_ref_func_line(func_head, id);

				obj_mgr_debug("Dereference:");
				func_head = trace->dereferences[id].head;
				wlan_objmgr_print_ref_func_line(func_head, id);
			}
		}
		break;
	case WLAN_PEER_OP:
		peer_obj = &obj->obj_peer->peer_objmgr;
		trace = &peer_obj->trace;
		for (id = 0; id < WLAN_REF_ID_MAX; id++) {
			if (qdf_atomic_read(&peer_obj->ref_id_dbg[id])) {
				obj_mgr_debug("Reference:");

				func_head = trace->references[id].head;
				wlan_objmgr_print_ref_func_line(func_head, id);

				obj_mgr_debug("Dereference:");
				func_head = trace->dereferences[id].head;
				wlan_objmgr_print_ref_func_line(func_head, id);
			}
		}
		break;
	default:
		break;
	}
}
#else
static void
wlan_objmgr_trace_print_ref(union wlan_objmgr_del_obj *obj,
			    enum wlan_objmgr_obj_type obj_type)
{
}
#endif

/* timeout handler for iterating logically deleted object */

static void wlan_objmgr_iterate_log_del_obj_handler(void *timer_arg)
{
	enum wlan_objmgr_obj_type obj_type;
	uint8_t *macaddr;
	const char *obj_name;
	struct wlan_objmgr_debug_info *debug_info;
	qdf_list_node_t *node;
	qdf_list_t *log_del_obj_list = NULL;
	struct log_del_obj *del_obj = NULL;
	qdf_time_t cur_tstamp;
	QDF_STATUS status;

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	debug_info = g_umac_glb_obj->debug_info;
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);

	if (!debug_info) {
		obj_mgr_err("debug_info is not initialized");
		return;
	}

	log_del_obj_list = &debug_info->obj_list;
	qdf_spin_lock_bh(&debug_info->list_lock);

	status = qdf_list_peek_front(log_del_obj_list, &node);
	if (QDF_IS_STATUS_ERROR(status)) {
		qdf_spin_unlock_bh(&debug_info->list_lock);
		return;
	}

	/* compute the current timestamp in seconds
	 * need to compare with destroy duration of object
	 */
	cur_tstamp = (qdf_system_ticks_to_msecs(qdf_system_ticks()) / 1000);

	do {
		del_obj = qdf_container_of(node, struct log_del_obj, node);
		obj_type = del_obj->obj_type;
		macaddr = wlan_objmgr_debug_get_macaddr(&del_obj->obj,
							obj_type);
		obj_name = wlan_obj_type_get_obj_name(obj_type);

		/* If object is in logically deleted state for time more than
		 * destroy duration, print the object type and MAC
		 */
		if (cur_tstamp  < (del_obj->tstamp +
					LOG_DEL_OBJ_DESTROY_DURATION_SEC)) {
			break;
		}
		if (!macaddr) {
			obj_mgr_err("macaddr is null");
			QDF_BUG(0);
			break;
		}
		if (!obj_name) {
			obj_mgr_err("obj_name is null");
			QDF_BUG(0);
			break;
		}

		obj_mgr_alert("#%s in L-state,MAC: " QDF_MAC_ADDR_FMT,
			      obj_name, QDF_MAC_ADDR_REF(macaddr));
		wlan_objmgr_print_pending_refs(&del_obj->obj, obj_type);

		wlan_objmgr_trace_print_ref(&del_obj->obj, obj_type);
		if (cur_tstamp > del_obj->tstamp +
		    LOG_DEL_OBJ_DESTROY_ASSERT_DURATION_SEC) {
			if (!qdf_is_recovering() && !qdf_is_fw_down())
				wlan_objmgr_debug_obj_destroyed_panic(obj_name);
		}

		status = qdf_list_peek_next(log_del_obj_list, node, &node);

	} while (QDF_IS_STATUS_SUCCESS(status));

	qdf_timer_mod(&debug_info->obj_timer, LOG_DEL_OBJ_TIMEOUT_VALUE_MSEC);
	qdf_spin_unlock_bh(&debug_info->list_lock);
}

void wlan_objmgr_debug_info_deinit(void)
{
	struct log_del_obj *obj_to_remove;
	struct wlan_objmgr_debug_info *debug_info;
	qdf_list_node_t *node = NULL;
	qdf_list_t *list;
	bool is_child_alive = false;

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	debug_info = g_umac_glb_obj->debug_info;
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);

	if (!debug_info) {
		obj_mgr_err("debug_info is not initialized");
		return;
	}
	list = &debug_info->obj_list;

	qdf_spin_lock_bh(&debug_info->list_lock);

	/* Check if any child of global object is in L-state and remove it,
	 * ideally it shouldn't be
	 */
	while (qdf_list_remove_front(list, &node) == QDF_STATUS_SUCCESS) {
		is_child_alive = true;
		obj_to_remove = qdf_container_of(node,
						 struct log_del_obj, node);
		if (qdf_list_empty(&debug_info->obj_list))
			qdf_timer_stop(&debug_info->obj_timer);
		/* free the object */
		qdf_mem_free(obj_to_remove);
	}
	qdf_spin_unlock_bh(&debug_info->list_lock);

	if (is_child_alive) {
		obj_mgr_alert("This shouldn't happen!!, No child of global"
			       "object should be in L-state, as global obj"
				"is going to destroy");
		QDF_BUG(0);
	}

	/* free timer, destroy spinlock, list and debug_info object as
	 * global object is going to free
	 */
	qdf_list_destroy(list);
	qdf_timer_free(&debug_info->obj_timer);
	qdf_spinlock_destroy(&debug_info->list_lock);
	qdf_mem_free(debug_info);

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	g_umac_glb_obj->debug_info = NULL;
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
}

void wlan_objmgr_debug_info_init(void)
{
	struct wlan_objmgr_debug_info *debug_info;

	debug_info = qdf_mem_malloc(sizeof(*debug_info));
	if (!debug_info) {
		g_umac_glb_obj->debug_info = NULL;
		return;
	}

	/* Initialize timer with timeout handler */
	qdf_timer_init(NULL, &debug_info->obj_timer,
		       wlan_objmgr_iterate_log_del_obj_handler,
		       NULL, QDF_TIMER_TYPE_WAKE_APPS);

	/* Initialze the node_count to 0 and create list*/
	qdf_list_create(&debug_info->obj_list,
			LOG_DEL_OBJ_LIST_MAX_COUNT);

	/* Initialize the spin_lock to protect list */
	qdf_spinlock_create(&debug_info->list_lock);

	/* attach debug_info object to global object */
	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	g_umac_glb_obj->debug_info = debug_info;
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);
}

#ifdef WLAN_OBJMGR_REF_ID_TRACE
void
wlan_objmgr_trace_init_lock(struct wlan_objmgr_trace *trace)
{
	qdf_spinlock_create(&trace->trace_lock);
}

void
wlan_objmgr_trace_deinit_lock(struct wlan_objmgr_trace *trace)
{
	qdf_spinlock_destroy(&trace->trace_lock);
}
#endif

#ifdef WLAN_OBJMGR_REF_ID_TRACE
static inline struct wlan_objmgr_line_ref_node*
wlan_objmgr_trace_line_node_alloc(int line)
{
	struct wlan_objmgr_line_ref_node *line_node;

	line_node = qdf_mem_malloc_atomic(sizeof(*line_node));
	if (!line_node)
		return NULL;

	line_node->line_ref.line = line;
	qdf_atomic_set(&line_node->line_ref.cnt, 1);
	line_node->next = NULL;

	return line_node;
}

static inline struct wlan_objmgr_trace_func*
wlan_objmgr_trace_ref_node_alloc(const char *func, int line)
{
	struct wlan_objmgr_trace_func *func_node;
	struct wlan_objmgr_line_ref_node *line_node;

	func_node = qdf_mem_malloc_atomic(sizeof(*func_node));
	if (!func_node)
		return NULL;

	line_node = wlan_objmgr_trace_line_node_alloc(line);
	if (!line_node) {
		qdf_mem_free(func_node);
		return NULL;
	}

	func_node->line_head = line_node;
	qdf_str_lcopy(func_node->func, func, WLAN_OBJMGR_TRACE_FUNC_SIZE);
	func_node->next = NULL;

	return func_node;
}

static inline void
wlan_objmgr_trace_check_line(struct wlan_objmgr_trace_func *tmp_func_node,
			     struct wlan_objmgr_trace *trace, int line)
{
	struct wlan_objmgr_line_ref_node *tmp_ln_node;

	tmp_ln_node = tmp_func_node->line_head;
	while (tmp_ln_node) {
		if (tmp_ln_node->line_ref.line == line) {
			qdf_atomic_inc(&tmp_ln_node->line_ref.cnt);
			break;
		}
		tmp_ln_node = tmp_ln_node->next;
	}
	if (!tmp_ln_node) {
		tmp_ln_node = wlan_objmgr_trace_line_node_alloc(line);
		if (tmp_ln_node) {
			tmp_ln_node->next = tmp_func_node->line_head;
			tmp_func_node->line_head = tmp_ln_node;
		}
	}
}

void
wlan_objmgr_trace_ref(struct wlan_objmgr_trace_func **func_head,
		      struct wlan_objmgr_trace *trace,
		      const char *func, int line)
{
	struct wlan_objmgr_trace_func *tmp_func_node;
	struct wlan_objmgr_trace_func *func_node;

	qdf_spin_lock_bh(&trace->trace_lock);
	if (!*func_head) {
		tmp_func_node = wlan_objmgr_trace_ref_node_alloc(func, line);
		if (tmp_func_node)
			*func_head = tmp_func_node;
	} else {
		tmp_func_node = *func_head;
		while (tmp_func_node) {
			func_node = tmp_func_node;
			if (!qdf_str_ncmp(tmp_func_node->func, func,
					  WLAN_OBJMGR_TRACE_FUNC_SIZE - 1)) {
				wlan_objmgr_trace_check_line(tmp_func_node,
							     trace, line);
				break;
			}
			tmp_func_node = tmp_func_node->next;
		}

		if (!tmp_func_node) {
			tmp_func_node = wlan_objmgr_trace_ref_node_alloc(func,
									 line);
			if (tmp_func_node)
				func_node->next = tmp_func_node;
		}
	}
	qdf_spin_unlock_bh(&trace->trace_lock);
}

static void
wlan_objmgr_trace_del_line(struct wlan_objmgr_line_ref_node **line_head)
{
	struct wlan_objmgr_line_ref_node *del_tmp_node;
	struct wlan_objmgr_line_ref_node *line_node;

	line_node = *line_head;
	while (line_node) {
		del_tmp_node = line_node;
		line_node = line_node->next;
		qdf_mem_free(del_tmp_node);
	}
	*line_head = NULL;
}

void
wlan_objmgr_trace_del_ref_list(struct wlan_objmgr_trace *trace)
{
	struct wlan_objmgr_trace_func *func_node;
	struct wlan_objmgr_trace_func *del_tmp_node;
	uint32_t id;

	qdf_spin_lock_bh(&trace->trace_lock);
	for (id = 0; id < WLAN_REF_ID_MAX; id++) {
		func_node = trace->references[id].head;
		while (func_node) {
			del_tmp_node = func_node;
			wlan_objmgr_trace_del_line(&del_tmp_node->line_head);
			func_node = func_node->next;
			qdf_mem_free(del_tmp_node);
		}
		trace->references[id].head = NULL;
	}
	for (id = 0; id < WLAN_REF_ID_MAX; id++) {
		func_node = trace->dereferences[id].head;
		while (func_node) {
			del_tmp_node = func_node;
			wlan_objmgr_trace_del_line(&del_tmp_node->line_head);
			func_node = func_node->next;
			qdf_mem_free(del_tmp_node);
		}
		trace->dereferences[id].head = NULL;
	}
	qdf_spin_unlock_bh(&trace->trace_lock);
}
#endif
