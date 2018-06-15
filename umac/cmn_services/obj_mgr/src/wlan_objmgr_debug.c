/*
 *
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
/*
 * DOC: Public APIs to perform debug operations on object manager
 */

#include <wlan_objmgr_psoc_obj.h>
#include <wlan_objmgr_pdev_obj.h>
#include <wlan_objmgr_vdev_obj.h>
#include <wlan_objmgr_peer_obj.h>
#include "wlan_objmgr_global_obj_i.h"
#include <qdf_mem.h>

#define LOG_DEL_OBJ_TIMEOUT_VALUE_MSEC   5000
#define LOG_DEL_OBJ_DESTROY_DURATION_SEC 5
#define LOG_DEL_OBJ_LIST_MAX_COUNT       (3 + 5 + 48 + 4096)

/**
 * struct log_del_obj    - Logically deleted Object
 * @obj:            Represents peer/vdev/pdev/psoc
 * @node:           List node from Logically deleted list
 * @obj_type:       Object type for peer/vdev/pdev/psoc
 * @tstamp:         Timestamp when node entered logically
 *                  deleted state
 */
struct log_del_obj {
	void *obj;
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
wlan_objmgr_debug_get_macaddr(void *obj,
			      enum wlan_objmgr_obj_type obj_type)
{
	switch (obj_type) {
	case WLAN_PSOC_OP:
		return wlan_psoc_get_hw_macaddr(obj);
	case WLAN_PDEV_OP:
		return wlan_pdev_get_hw_macaddr(obj);
	case WLAN_VDEV_OP:
		return wlan_vdev_mlme_get_macaddr(obj);
	case WLAN_PEER_OP:
		return wlan_peer_get_macaddr(obj);
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

void wlan_objmgr_notify_log_delete(void *obj,
				   enum wlan_objmgr_obj_type obj_type)
{
	struct wlan_objmgr_debug_info *debug_info;
	const char *obj_name;
	uint8_t *macaddr;
	qdf_time_t tstamp;
	struct log_del_obj *node;

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

	macaddr = wlan_objmgr_debug_get_macaddr(obj, obj_type);
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
	if (!node) {
		obj_mgr_err("Object node creation failed");
		return;
	}
	node->obj = obj;
	node->obj_type = obj_type;
	node->tstamp = tstamp;
	obj_mgr_debug("#%s : mac_addr :" QDF_MAC_ADDR_STR" entered L-state",
		      obj_name, QDF_MAC_ADDR_ARRAY(macaddr));
	wlan_objmgr_insert_ld_obj_to_list(debug_info, &node->node);
}

static void
wlan_objmgr_rem_ld_obj_from_list(void *obj,
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
		if (obj_to_remove->obj == obj &&
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

	qdf_spin_lock_bh(&g_umac_glb_obj->global_lock);
	debug_info = g_umac_glb_obj->debug_info;
	qdf_spin_unlock_bh(&g_umac_glb_obj->global_lock);

	if (!debug_info) {
		obj_mgr_err("debug_info is null");
		return;
	}
	macaddr = wlan_objmgr_debug_get_macaddr(obj, obj_type);
	if (!macaddr) {
		obj_mgr_err("macaddr is null");
		return;
	}
	obj_name = wlan_obj_type_get_obj_name(obj_type);
	if (!obj_name) {
		obj_mgr_err("obj_name is null");
		return;
	}
	obj_mgr_debug("#%s, macaddr: " QDF_MAC_ADDR_STR" exited L-state",
		      obj_name, QDF_MAC_ADDR_ARRAY(macaddr));

	wlan_objmgr_rem_ld_obj_from_list(obj, debug_info, obj_type);
}

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
		macaddr = wlan_objmgr_debug_get_macaddr(del_obj->obj, obj_type);
		obj_name = wlan_obj_type_get_obj_name(obj_type);

		/* If object is in logically deleted state for time more than
		 * destroy duration, print the object type and MAC
		 */
		if (cur_tstamp  < (del_obj->tstamp +
					LOG_DEL_OBJ_DESTROY_DURATION_SEC)) {
			break;
		}
		if (!macaddr) {
			qdf_spin_unlock_bh(&debug_info->list_lock);
			obj_mgr_err("macaddr is null");
			QDF_BUG(0);
			goto modify_timer;
		}
		if (!obj_name) {
			qdf_spin_unlock_bh(&debug_info->list_lock);
			obj_mgr_err("obj_name is null");
			QDF_BUG(0);
			goto modify_timer;
		}

		obj_mgr_alert("#%s in L-state,MAC: " QDF_MAC_ADDR_STR,
			      obj_name, QDF_MAC_ADDR_ARRAY(macaddr));

		status = qdf_list_peek_next(log_del_obj_list, node, &node);

	} while (QDF_IS_STATUS_SUCCESS(status));

	qdf_spin_unlock_bh(&debug_info->list_lock);

modify_timer:
	/* modify timer timeout value */
	qdf_timer_mod(&debug_info->obj_timer, LOG_DEL_OBJ_TIMEOUT_VALUE_MSEC);
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
		obj_mgr_err("debug_info allocation failed");
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
