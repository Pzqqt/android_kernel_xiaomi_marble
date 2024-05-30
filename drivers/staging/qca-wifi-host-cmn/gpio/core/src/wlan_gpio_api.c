/*
 * Copyright (c) 2020, The Linux Foundation. All rights reserved.
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
 * DOC: wlan_gpio_api.c
 */
#include <wlan_gpio_api.h>
#include <wlan_gpio_priv_api.h>
#include <wlan_objmgr_global_obj.h>

/**
 * gpio_psoc_obj_created_notification() - PSOC obj create callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is created.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS
gpio_psoc_obj_created_notification(struct wlan_objmgr_psoc *psoc,
				   void *arg_list)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct gpio_psoc_priv_obj *gpio_obj;

	gpio_obj = qdf_mem_malloc(sizeof(*gpio_obj));
	if (!gpio_obj)
		return QDF_STATUS_E_NOMEM;

	qdf_spinlock_create(&gpio_obj->lock);
	status = wlan_objmgr_psoc_component_obj_attach(psoc,
						       WLAN_UMAC_COMP_GPIO,
						       gpio_obj,
						       QDF_STATUS_SUCCESS);
	if (QDF_IS_STATUS_ERROR(status)) {
		gpio_err("obj attach with psoc failed");
		goto gpio_psoc_attach_failed;
	}

	return QDF_STATUS_SUCCESS;

gpio_psoc_attach_failed:
	qdf_spinlock_destroy(&gpio_obj->lock);
	qdf_mem_free(gpio_obj);
	return status;
}

/**
 * gpio_psoc_obj_destroyed_notification() - obj delete callback
 * @psoc: PSOC object
 * @arg_list: Variable argument list
 *
 * This callback is registered with object manager during initialization to
 * get notified when the object is deleted.
 *
 * Return: QDF_STATUS_SUCCESS on success, QDF_STATUS_E_** on error
 */
static QDF_STATUS
gpio_psoc_obj_destroyed_notification(struct wlan_objmgr_psoc *psoc,
				     void *arg_list)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	struct gpio_psoc_priv_obj *gpio_obj;

	gpio_obj = gpio_get_psoc_priv_obj(psoc);

	if (!gpio_obj) {
		gpio_err("gpio_obj is NULL");
		return QDF_STATUS_E_FAULT;
	}

	status = wlan_objmgr_psoc_component_obj_detach(psoc,
						       WLAN_UMAC_COMP_GPIO,
						       gpio_obj);
	if (QDF_IS_STATUS_ERROR(status))
		gpio_err("gpio_obj detach failed");

	qdf_spinlock_destroy(&gpio_obj->lock);
	qdf_mem_free(gpio_obj);

	return status;
}

QDF_STATUS wlan_gpio_init(void)
{
	QDF_STATUS status;

	/* register psoc create handler functions. */
	status = wlan_objmgr_register_psoc_create_handler(
			WLAN_UMAC_COMP_GPIO,
			gpio_psoc_obj_created_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		gpio_err("register create handler failed");
		return status;
	}

	/* register psoc delete handler functions. */
	status = wlan_objmgr_register_psoc_destroy_handler(
			WLAN_UMAC_COMP_GPIO,
			gpio_psoc_obj_destroyed_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		gpio_err("register destroy handler failed");
		goto fail_delete_psoc;
	}

	return status;

fail_delete_psoc:
	wlan_objmgr_unregister_psoc_create_handler(
				WLAN_UMAC_COMP_GPIO,
				gpio_psoc_obj_created_notification,
				NULL);
	return status;
}

QDF_STATUS wlan_gpio_deinit(void)
{
	QDF_STATUS ret = QDF_STATUS_SUCCESS, status;

	/* unregister psoc delete handler functions. */
	status = wlan_objmgr_unregister_psoc_destroy_handler(
			WLAN_UMAC_COMP_GPIO,
			gpio_psoc_obj_destroyed_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		gpio_err("unregister destroy handler failed");
		ret = status;
	}

	/* unregister psoc create handler functions. */
	status = wlan_objmgr_unregister_psoc_create_handler(
			WLAN_UMAC_COMP_GPIO,
			gpio_psoc_obj_created_notification,
			NULL);
	if (QDF_IS_STATUS_ERROR(status)) {
		gpio_err("unregister create handler failed");
		ret = status;
	}

	return ret;
}
