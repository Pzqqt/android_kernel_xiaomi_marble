/*
 * Copyright (c) 2016-2019, 2021 The Linux Foundation. All rights reserved.
 * Copyright (c) 2021 Qualcomm Innovation Center, Inc. All rights reserved.
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
  * DOC: Public APIs to perform operations on Global objects
  */
#ifndef _WLAN_OBJMGR_GLOBAL_OBJ_H_
#define _WLAN_OBJMGR_GLOBAL_OBJ_H_

/**
 * wlan_objmgr_global_obj_init() - global object initialization
 *
 * Creates global object, intializes with default values
 *
 * Return: SUCCESS  on successful creation,
 *         FAILURE  on Mem alloc failure or allocated already
 *
 */
QDF_STATUS wlan_objmgr_global_obj_init(void);

/**
 * wlan_objmgr_global_obj_deinit() - global object deinitialization
 *
 * Deletes global object
 *
 * Return: SUCCESS  on successful deletion,
 *         FAILURE  on object is not found
 *
 */
QDF_STATUS wlan_objmgr_global_obj_deinit(void);

/**
 * wlan_objmgr_global_obj_can_destroyed() - Checks whether global object
 *					  can be destroyed
 *
 * Checks the psoc table of global object, if psoc table is empty
 * returns the SUCCESS
 *
 * Return: SUCCESS  on can be destroyed,
 *         FAILURE  on can't be destroyed
 *
 */
QDF_STATUS wlan_objmgr_global_obj_can_destroyed(void);

/**
 * wlan_objmgr_register_psoc_create_handler() - register psoc create handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PSOC creation
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on PSOC creation
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_psoc_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_create_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_psoc_create_handler() - unregister psoc create handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PSOC creation
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_psoc_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_create_handler handler,
		void *args);

/**
 * wlan_objmgr_register_psoc_destroy_handler() - register destroy handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PSOC deletion
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on PSOC deletion
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_psoc_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_destroy_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_psoc_destroy_handler() - unregister destroy handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PSOC deletion
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_psoc_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_destroy_handler handler,
		void *args);

/**
 * wlan_objmgr_register_psoc_status_handler() - register status handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PSOC status
 *         change
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on PSOC object status change
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_psoc_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_status_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_psoc_status_handler() - unregister status handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PSOC status
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_psoc_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_status_handler handler,
		void *args);

/**
 * wlan_objmgr_register_pdev_create_handler() - register pdev create handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PDEV creation
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on PDEV creation
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_pdev_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_create_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_pdev_create_handler() - unregister pdev create handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PDEV creation
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_pdev_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_create_handler handler,
		void *args);

/**
 * wlan_objmgr_register_pdev_destroy_handler() - register pdev destroy handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PDEV deletion
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on PDEV deletion
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_pdev_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_destroy_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_pdev_destroy_handler() - unregister pdev destroy handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PDEV deletion
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_pdev_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_destroy_handler handler,
		void *args);

/**
 * wlan_objmgr_register_pdev_status_handler() - register pdev status handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PDEV status
 *         change
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on PDEV object status change
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_pdev_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_status_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_pdev_status_handler() - unregister pdev status handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PDEV status
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_pdev_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_status_handler handler,
		void *args);

/**
 * wlan_objmgr_register_vdev_create_handler() - register vdev create handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on VDEV creation
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on VDEV creation
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_vdev_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_create_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_vdev_create_handler() - unregister vdev create handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on VDEV creation
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_vdev_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_create_handler handler,
		void *args);

/**
 * wlan_objmgr_register_vdev_destroy_handler() - register vdev destroy handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on VDEV deletion
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on VDEV deletion
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_vdev_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_destroy_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_vdev_destroy_handler() - unregister vdev destroy handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on VDEV deletion
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_vdev_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_destroy_handler handler,
		void *args);

/**
 * wlan_objmgr_register_vdev_status_handler() - register vdev status handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on VDEV status
 *         change
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on VDEV object status change
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_vdev_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_status_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_vdev_status_handler() - unregister vdev status handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on VDEV status
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_vdev_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_status_handler handler,
		void *args);

/**
 * wlan_objmgr_register_vdev_peer_free_notify_handler() - register vdev peer
 *                                                        free handler
 * @id: component id
 * @handler: function pointer of the component
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on VDEV Peer gets freed
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_vdev_peer_free_notify_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_peer_free_notify_handler handler);

/**
 * wlan_objmgr_unregister_vdev_peer_free_notify_handler() - unregister vdev
 *                                                          peer free handler
 * @id: component id
 * @handler: function pointer of the component
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_vdev_peer_free_notify_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_peer_free_notify_handler handler);

/**
 * wlan_objmgr_register_peer_create_handler() - register peer create handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PEER creation
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on PEER creation
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_peer_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_create_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_peer_create_handler() - unregister peer create handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PEER creation
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_peer_create_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_create_handler handler,
		void *args);

/**
 * wlan_objmgr_register_peer_destroy_handler() - register peer destroy handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PEER deletion
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on PEER deletion
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_peer_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_destroy_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_peer_destroy_handler() - unregister peer destroy handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PEER deletion
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_peer_destroy_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_destroy_handler handler,
		void *args);

/**
 * wlan_objmgr_register_peer_status_handler() - register peer status handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PEER status
 *         change
 *
 * API, allows other UMAC components to register handler
 * The registered handler would be invoked on PEER object status change
 *
 * Return: SUCCESS,
 *         Failure (if registration fails, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_register_peer_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_status_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_peer_status_handler() - unregister peer status handler
 * @id: component id
 * @handler: function pointer of the component
 * @args: args, if component wants certain args to be passed on PEER status
 *
 * API, allows other UMAC components to unregister handler
 *
 * Return: SUCCESS,
 *         Failure (if handler is not present, each failure has different error
 *         code)
 */
QDF_STATUS wlan_objmgr_unregister_peer_status_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_status_handler handler,
		void *args);

/**
 * APIs to operations on psoc
 */
typedef void (*wlan_objmgr_psoc_handler)(struct wlan_objmgr_psoc *psoc,
					void *arg,
					uint8_t index);

/**
 * wlan_objmgr_iterate_psoc_list() - iterate through all psocs
 *
 * @handler: the handler will be called for each psoc
 *           the handler should be implemented to perform required operation
 * @arg:     agruments passed by caller
 * @dbg_id: id of the caller
 *
 * API to be used for performing the operations on all psoc
 * The "handler" here shouldn't take g_umac_glb_obj->global_lock lock when
 * processing
 *
 * Return: SUCCESS/FAILURE
 */
QDF_STATUS wlan_objmgr_iterate_psoc_list(
		wlan_objmgr_psoc_handler handler,
		void *arg, wlan_objmgr_ref_dbgid dbg_id);

/**
 * wlan_objmgr_get_psoc_by_id() - find psoc by psoc id
 *
 * @psoc_id: Id of PSOC to be retrieved
 * @dbg_id: id of the caller
 *
 * API to find psoc object pointer by psoc id
 *
 * This API increments the ref count of the psoc object internally, the
 * caller has to invoke the wlan_objmgr_psoc_release_ref() to decrement
 * ref count
 *
 * Return: psoc pointer
 *         NULL on FAILURE
 */
struct wlan_objmgr_psoc
*wlan_objmgr_get_psoc_by_id(uint8_t psoc_id, wlan_objmgr_ref_dbgid dbg_id);

#ifdef WLAN_FEATURE_11BE_MLO
/**
 * wlan_objmgr_get_mlo_ctx() - Get MLO context from global umac object
 *
 * This API is used to get MLO context object from the global umac object
 *
 * Return: Pointer to the mlo context
 */
struct mlo_mgr_context *wlan_objmgr_get_mlo_ctx(void);

/**
 * wlan_objmgr_set_mlo_ctx() - Set MLO context at global umac object
 *
 * This API is used to set MLO context object at the global umac object
 *
 * Return: None
 */
void wlan_objmgr_set_mlo_ctx(struct mlo_mgr_context *ctx);

/**
 * wlan_objmgr_set_dp_mlo_ctx() - set dp handle in mlo context
 * @dp_handle: Data path module handle
 *
 * Return: void
 */
void wlan_objmgr_set_dp_mlo_ctx(void *dp_handle);

/**
 * wlan_objmgr_get_dp_mlo_ctx() - get dp handle from mlo_context
 *
 * Return: dp handle
 */
void *wlan_objmgr_get_dp_mlo_ctx(void);
#else
static inline struct mlo_mgr_context *wlan_objmgr_get_mlo_ctx(void)
{
	return NULL;
}

static inline void wlan_objmgr_set_mlo_ctx(struct mlo_mgr_context *ctx)
{}

static inline void *wlan_objmgr_get_dp_mlo_ctx(void)
{
	return NULL;
}

static inline void wlan_objmgr_set_dp_mlo_ctx(void *dp_handle)
{
}
#endif /* WLAN_FEATURE_11BE_MLO */

#endif /* _WLAN_OBJMGR_GLOBAL_OBJ_H_*/
