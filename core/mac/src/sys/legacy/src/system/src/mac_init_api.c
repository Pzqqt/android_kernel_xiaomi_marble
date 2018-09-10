/*
 * Copyright (c) 2011-2018 The Linux Foundation. All rights reserved.
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
 *
 * mac_init_api.c - This file has all the mac level init functions
 *                   for all the defined threads at system level.
 * Author:    Dinesh Upadhyay
 * Date:      04/23/2007
 * History:-
 * Date: 04/08/2008       Modified by: Santosh Mandiganal
 * Modification Information: Code to allocate and free the  memory for DumpTable entry.
 * --------------------------------------------------------------------------
 *
 */
/* Standard include files */
#include "cfg_api.h"             /* cfg_cleanup */
#include "lim_api.h"             /* lim_cleanup */
#include "sir_types.h"
#include "sys_entry_func.h"
#include "mac_init_api.h"
#include "wlan_mlme_main.h"

#ifdef TRACE_RECORD
#include "mac_trace.h"
#endif

#ifdef WLAN_ALLOCATE_GLOBAL_BUFFERS_DYNAMICALLY
static tAniSirGlobal *global_mac_context;

static inline tpAniSirGlobal mac_allocate_context_buffer(void)
{
	global_mac_context = qdf_mem_malloc(sizeof(tAniSirGlobal));

	return global_mac_context;
}

static inline void mac_free_context_buffer(void)
{
	qdf_mem_free(global_mac_context);
	global_mac_context = NULL;
}
#else /* WLAN_ALLOCATE_GLOBAL_BUFFERS_DYNAMICALLY */
static tAniSirGlobal global_mac_context;

static inline tpAniSirGlobal mac_allocate_context_buffer(void)
{
	return &global_mac_context;
}

static inline void mac_free_context_buffer(void)
{
}
#endif /* WLAN_ALLOCATE_GLOBAL_BUFFERS_DYNAMICALLY */

QDF_STATUS mac_start(mac_handle_t mac_handle,
		     struct mac_start_params *params)
{
	QDF_STATUS status = QDF_STATUS_SUCCESS;
	tpAniSirGlobal mac = MAC_CONTEXT(mac_handle);

	if (!mac || !params) {
		QDF_ASSERT(0);
		status = QDF_STATUS_E_FAILURE;
		return status;
	}

	mac->gDriverType = params->driver_type;

	if (ANI_DRIVER_TYPE(mac) != QDF_DRIVER_TYPE_MFG)
		status = pe_start(mac);

	return status;
}

QDF_STATUS mac_stop(mac_handle_t mac_handle)
{
	tpAniSirGlobal mac = MAC_CONTEXT(mac_handle);

	pe_stop(mac);
	cfg_cleanup(mac);

	return QDF_STATUS_SUCCESS;
}

/** -------------------------------------------------------------
   \fn mac_open
   \brief this function will be called during init. This function is suppose to allocate all the
 \       memory with the global context will be allocated here.
   \param   tHalHandle pHalHandle
   \param   hdd_handle_t hdd_handle
   \param   tHalOpenParameters* pHalOpenParams
   \return QDF_STATUS
   -------------------------------------------------------------*/

QDF_STATUS mac_open(struct wlan_objmgr_psoc *psoc, tHalHandle *pHalHandle,
		    hdd_handle_t hdd_handle, struct cds_config_info *cds_cfg)
{
	tpAniSirGlobal p_mac;
	QDF_STATUS status;
	struct wlan_mlme_psoc_obj *mlme_obj;

	QDF_BUG(pHalHandle);
	if (!pHalHandle)
		return QDF_STATUS_E_FAILURE;

	p_mac = mac_allocate_context_buffer();
	if (!p_mac)
		return QDF_STATUS_E_NOMEM;

	/*
	 * Set various global fields of p_mac here
	 * (Could be platform dependent as some variables in p_mac are platform
	 * dependent)
	 */
	p_mac->hdd_handle = hdd_handle;

	status = wlan_objmgr_psoc_try_get_ref(psoc, WLAN_LEGACY_MAC_ID);
	if (QDF_IS_STATUS_ERROR(status)) {
		pe_err("PSOC get ref failure");
		goto free_mac_context;
	}

	p_mac->psoc = psoc;
	mlme_obj = mlme_get_psoc_obj(psoc);
	if (!mlme_obj) {
		pe_err("Failed to get MLME Obj");
		status = QDF_STATUS_E_FAILURE;
		goto release_psoc_ref;
	}
	p_mac->mlme_cfg = &mlme_obj->cfg;

	*pHalHandle = (tHalHandle)p_mac;

	/* For Non-FTM cases this value will be reset during mac_start */
	if (cds_cfg->driver_type)
		p_mac->gDriverType = QDF_DRIVER_TYPE_MFG;

	status = cfg_init(p_mac);
	if (QDF_IS_STATUS_ERROR(status)) {
		pe_err("failed to init legacy CFG; status:%u", status);
		goto release_psoc_ref;
	}

	sys_init_globals(p_mac);

	/* FW: 0 to 2047 and Host: 2048 to 4095 */
	p_mac->mgmtSeqNum = WLAN_HOST_SEQ_NUM_MIN - 1;
	p_mac->he_sgi_ltf_cfg_bit_mask = DEF_HE_AUTO_SGI_LTF;
	p_mac->is_usr_cfg_amsdu_enabled = true;

	status = pe_open(p_mac, cds_cfg);
	if (QDF_IS_STATUS_ERROR(status)) {
		pe_err("failed to open PE; status:%u", status);
		goto deinit_cfg;
	}

	return QDF_STATUS_SUCCESS;

deinit_cfg:
	cfg_de_init(p_mac);

release_psoc_ref:
	wlan_objmgr_psoc_release_ref(psoc, WLAN_LEGACY_MAC_ID);

free_mac_context:
	mac_free_context_buffer();

	return status;
}

/** -------------------------------------------------------------
   \fn mac_close
   \brief this function will be called in shutdown sequence from HDD. All the
 \       allocated memory with global context will be freed here.
   \param   tpAniSirGlobal pMac
   \return none
   -------------------------------------------------------------*/

QDF_STATUS mac_close(tHalHandle hHal)
{

	tpAniSirGlobal pMac = (tpAniSirGlobal) hHal;

	if (!pMac)
		return QDF_STATUS_E_FAILURE;

	pe_close(pMac);

	/* Call routine to free-up all CFG data structures */
	cfg_de_init(pMac);

	if (pMac->pdev) {
		wlan_objmgr_pdev_release_ref(pMac->pdev, WLAN_LEGACY_MAC_ID);
		pMac->pdev = NULL;
	}
	wlan_objmgr_psoc_release_ref(pMac->psoc, WLAN_LEGACY_MAC_ID);
	pMac->mlme_cfg = NULL;
	pMac->psoc = NULL;
	mac_free_context_buffer();

	return QDF_STATUS_SUCCESS;
}
