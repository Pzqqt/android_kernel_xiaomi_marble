/*
 * Copyright (c) 2016 The Linux Foundation. All rights reserved.
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
 * wlan_objmgr_global_obj_create() - Creates global object
 *
 * Creates global object, intializes with default values
 *
 * Return: SUCCESS  on successful creation,
 *         FAILURE  on Mem alloc failure or allocated already
 *
 */
QDF_STATUS wlan_objmgr_global_obj_create(void);

/**
 * wlan_objmgr_global_obj_delete() - Deletes global object
 *
 * Deletes global object
 *
 * Return: SUCCESS  on successful deletion,
 *         FAILURE  on object is not found
 *
 */
QDF_STATUS wlan_objmgr_global_obj_delete(void);

/**
 * wlan_objmgr_global_obj_can_deleted() - Checks whether global object
 *					  can be deleted
 *
 * Checks the psoc table of global object, if psoc table is empty
 * returns the SUCCESS
 *
 * Return: SUCCESS  on can be deleted,
 *         FAILURE  on can't be deleted
 *
 */
QDF_STATUS wlan_objmgr_global_obj_can_deleted(void);

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
 * wlan_objmgr_register_psoc_delete_handler() - register delete handler
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
QDF_STATUS wlan_objmgr_register_psoc_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_delete_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_psoc_delete_handler() - unregister delete handler
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
QDF_STATUS wlan_objmgr_unregister_psoc_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_psoc_delete_handler handler,
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
 * wlan_objmgr_register_pdev_delete_handler() - register pdev delete handler
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
QDF_STATUS wlan_objmgr_register_pdev_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_delete_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_pdev_delete_handler() - unregister pdev delete handler
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
QDF_STATUS wlan_objmgr_unregister_pdev_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_pdev_delete_handler handler,
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
 * wlan_objmgr_register_vdev_delete_handler() - register vdev delete handler
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
QDF_STATUS wlan_objmgr_register_vdev_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_delete_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_vdev_delete_handler() - unregister vdev delete handler
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
QDF_STATUS wlan_objmgr_unregister_vdev_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_vdev_delete_handler handler,
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
 * wlan_objmgr_register_peer_delete_handler() - register peer delete handler
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
QDF_STATUS wlan_objmgr_register_peer_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_delete_handler handler,
		void *args);

/**
 * wlan_objmgr_unregister_peer_delete_handler() - unregister peer delete handler
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
QDF_STATUS wlan_objmgr_unregister_peer_delete_handler(
		enum wlan_umac_comp_id id,
		wlan_objmgr_peer_delete_handler handler,
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

#endif /* _WLAN_OBJMGR_GLOBAL_OBJ_H_*/
