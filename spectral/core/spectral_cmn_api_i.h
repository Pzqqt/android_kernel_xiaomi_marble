/*
 * Copyright (c) 2017-2018 The Linux Foundation. All rights reserved.
 *
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

#ifndef _SPECTRAL_CMN_API_I_H_
#define _SPECTRAL_CMN_API_I_H_

#include "spectral_defs_i.h"

/**
 * wlan_spectral_psoc_obj_create_handler() - handler for psoc object create
 * @psoc: reference to global psoc object
 * @arg:  reference to argument provided during registration of handler
 *
 * This is a handler to indicate psoc object created. Hence spectral_context
 * object can be created and attached to psoc component list.
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_FAILURE if psoc is null
 *         QDF_STATUS_E_NOMEM on failure of spectral object allocation
 */
QDF_STATUS wlan_spectral_psoc_obj_create_handler(struct wlan_objmgr_psoc *psoc,
						 void *arg);

/**
 * wlan_spectral_psoc_obj_destroy_handler() - handler for psoc object delete
 * @psoc: reference to global psoc object
 * @arg:  reference to argument provided during registration of handler
 *
 * This is a handler to indicate psoc object going to be deleted.
 * Hence spectral_context object can be detached from psoc component list.
 * Then spectral_context object can be deleted.
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS wlan_spectral_psoc_obj_destroy_handler(struct wlan_objmgr_psoc *psoc,
						  void *arg);

/**
 * wlan_spectral_pdev_obj_create_handler() - handler for pdev object create
 * @pdev: reference to global pdev object
 * @arg:  reference to argument provided during registration of handler
 *
 * This is a handler to indicate pdev object created. Hence pdev specific
 * spectral object can be created and attached to pdev component list.
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_FAILURE if pdev is null
 *         QDF_STATUS_E_NOMEM on failure of spectral object allocation
 */
QDF_STATUS wlan_spectral_pdev_obj_create_handler(struct wlan_objmgr_pdev *pdev,
						 void *arg);

/**
 * wlan_spectral_pdev_obj_destroy_handler() - handler for pdev object delete
 * @pdev: reference to global pdev object
 * @arg:  reference to argument provided during registration of handler
 *
 * This is a handler to indicate pdev object going to be deleted.
 * Hence pdev specific spectral object can be detached from pdev component list.
 * Then pdev_spectral object can be deleted.
 *
 * Return: QDF_STATUS_SUCCESS on success
 *         QDF_STATUS_E_FAILURE on failure
 */
QDF_STATUS wlan_spectral_pdev_obj_destroy_handler(struct wlan_objmgr_pdev *pdev,
						  void *arg);

/**
 * spectral_control_cmn()- common handler for demultiplexing requests from
 *                         higher layer
 * @pdev:    reference to global pdev object
 * @id:      spectral config command id
 * @indata:  reference to input data
 * @insize:  input data size
 * @outdata: reference to output data
 * @outsize: reference to output data size
 *
 * This function processes the spectral config command
 * and appropriate handlers are invoked.
 *
 * Return: 0 success else failure
 */
int spectral_control_cmn(struct wlan_objmgr_pdev *pdev,
			 u_int id,
			 void *indata,
			 uint32_t insize, void *outdata, uint32_t *outsize);

/**
 * spectral_control_ol(): Offload handler for demultiplexing requests from
 *                         higher layer
 * @pdev:    reference to global pdev object
 * @id:      spectral config command id
 * @indata:  reference to input data
 * @insize:  input data size
 * @outdata: reference to output data
 * @outsize: reference to output data size
 *
 * This function processes the spectral config command
 * and appropriate handlers are invoked.
 *
 * Return: 0 success else failure
 */
int spectral_control_ol(
	struct wlan_objmgr_pdev *pdev, u_int id,
	void *indata, uint32_t insize, void *outdata, uint32_t *outsize);

/**
 * spectral_get_spectral_ctx_from_pdev() - API to get spectral context object
 *                                         from pdev
 * @pdev : Reference to pdev global object
 *
 * This API used to get spectral context object from global pdev reference.
 * Null check should be done before invoking this inline function.
 *
 * Return : Reference to spectral_context object
 */
static inline struct spectral_context *
spectral_get_spectral_ctx_from_pdev(struct wlan_objmgr_pdev *pdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct spectral_context *sc = NULL;

	psoc = wlan_pdev_get_psoc(pdev);
	if (psoc) {
		sc = wlan_objmgr_psoc_get_comp_private_obj(
			psoc,
			WLAN_UMAC_COMP_SPECTRAL);
	}

	return sc;
}

/**
 * spectral_get_spectral_ctx_from_pscoc() - API to get spectral context object
 *                                         from psoc
 * @psoc : Reference to psoc global object
 *
 * This API used to get spectral context object from global psoc reference.
 * Null check should be done before invoking this inline function.
 *
 * Return : Reference to spectral_context object
 */
static inline struct spectral_context *
spectral_get_spectral_ctx_from_psoc(struct wlan_objmgr_psoc *psoc)
{
	struct spectral_context *sc = NULL;

	if (psoc) {
		sc = wlan_objmgr_psoc_get_comp_private_obj(
			psoc,
			WLAN_UMAC_COMP_SPECTRAL);
	}

	return sc;
}

/**
 * spectral_get_spectral_ctx_from_vdev() - API to get spectral context object
 *                                         from vdev
 * @vdev : Reference to vdev global object
 *
 * This API used to get spectral context object from global vdev reference.
 * Null check should be done before invoking this inline function.
 *
 * Return : Reference to spectral_context object
 */
static inline struct spectral_context *
spectral_get_spectral_ctx_from_vdev(struct wlan_objmgr_vdev *vdev)
{
	struct wlan_objmgr_psoc *psoc = NULL;
	struct spectral_context *sc = NULL;

	psoc = wlan_vdev_get_psoc(vdev);
	if (psoc) {
		sc = wlan_objmgr_psoc_get_comp_private_obj(
			psoc,
			WLAN_UMAC_COMP_SPECTRAL);
	}

	return sc;
}
#endif				/* _SPECTRAL_CMN_API_I_H_ */
